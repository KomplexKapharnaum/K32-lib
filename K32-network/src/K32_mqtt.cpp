/*
  K32_mqtt.cpp
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/

#include "K32_mqtt.h"
#include <ESPmDNS.h>


/*
 *   PUBLIC
 */


esp_err_t K32_mqtt::mqtt_event(esp_mqtt_event_handle_t event)
{
    K32_mqtt *that = (K32_mqtt *)event->user_context;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;

    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
        {
          that->connected = true;
          LOG("MQTT: connected to broker");

          String myChan = String(that->k32->system->channel()); 
          String myID = String(that->k32->system->id());

          esp_mqtt_client_subscribe(client, ("k32/c" + myChan + "/#").c_str(), 1);
          LOG("MQTT: subscribed to " + ("k32/c" + myChan + "/#"));

          esp_mqtt_client_subscribe(client, ("k32/e" + myID + "/#").c_str(), 1);
          LOG("MQTT: subscribed to " + ("k32/e" + myID + "/#"));

          esp_mqtt_client_subscribe(client, "k32/all/#", 1);
          LOG("MQTT: subscribed to k32/all/#");

          for (int k=0; k<that->subscount; k++) {
            esp_mqtt_client_subscribe(client, that->subscriptions[k].topic, that->subscriptions[k].qos);
            LOGF2("MQTT: subscribed to %s with QOS %i\n", that->subscriptions[k].topic, that->subscriptions[k].qos);
          }

          //   MDNS.addService("_mqttc", "_tcp", 1883);
          //   mdns_service_instance_name_set("_mqttc", "_tcp", ("MQTTc._"+k32->system->name()).c_str());

          // msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 0);
          // LOGF("sent publish successful, msg_id=%d\n", msg_id);
          break;
        } 

        case MQTT_EVENT_DISCONNECTED:
            that->connected = true;
            LOG("MQTT: disconnected");
            break;

        case MQTT_EVENT_DATA:
        {    // FORWARD TO INTERCOM
            
            char topic[127];
            int cmd_size = min(99, event->topic_len);
            strncpy(topic, event->topic, cmd_size);
            topic[cmd_size] = '\0';
            char* command = strchr(topic, '/')+1;
            command = strchr(command, '/')+1;
            
            char data[1024];
            int data_size = min(1023, event->data_len);
            strncpy(data, event->data, data_size);
            data[data_size] = '\0';

            // CUSTOM SUBSCRIBES 
            // TODO: as Event via INTERCOM !
            //
            bool didCustom = false;
            for (int k=0; k<that->subscount; k++)
              if ( strcmp(that->subscriptions[k].topic, topic) == 0 ) {
                that->subscriptions[k].callback(event->data, event->data_len);
                didCustom = true;
              }
            if (didCustom) break;

            // GENERIC SUBSCRIBES (as Command)
            //
            Orderz* newOrder = new Orderz(command);
            char* p = strtok(data, "|");
            while(p != NULL) {
              newOrder->addData(p);
              p = strtok(NULL, "|");
            }
            that->cmd( newOrder );

            // LOG("MQTT_EVENT_DATA");
            // printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            // printf("DATA=%.*s\r\n", event->data_len, event->data);
            break;
        }

        case MQTT_EVENT_ERROR:
            LOG("MQTT: error");
            break;

        default:
            // LOGF("MQTT: Other event id:%d\n", event->event_id);
            break;

    }
    return ESP_OK;
}


K32_mqtt::K32_mqtt(K32* k32, K32_wifi* wifi, mqttconf conf) : K32_plugin("mqtt", k32), wifi(wifi) 
{
  this->conf = conf;
  lock = xSemaphoreCreateMutex();
  connected = false;

  // INIT
  esp_mqtt_client_config_t mqtt_cfg = {0};
  mqtt_cfg.host = conf.broker;
  mqtt_cfg.event_handle = mqtt_event;
  mqtt_cfg.user_context = (void *)this;
  mqttClient = esp_mqtt_client_init(&mqtt_cfg);

  this->start();
}


void K32_mqtt::start()
{
  // LOOP client
  xTaskCreate(this->check,    // function
                          "mqtt_check", // name
                          2000,         // stack memory
                          (void *)this,  // args
                          0,             // priority
                          &xHandle1          // handler
                          );            // core

  // BEAT
  if (this->conf.beatInterval > 0)
    xTaskCreate(this->beat,   // function
                            "mqtt_beat",  // server name
                            2000,         // stack memory
                            (void *)this, // args
                            0,            // priority
                            &xHandle2         // handler
                            );           // core

  // BEACON
  if (this->conf.beaconInterval > 0)
    xTaskCreate(this->beacon,   // function
                            "mqtt_beacon",  // server name
                            5000,         // stack memory
                            (void *)this, // args
                            0,            // priority
                            &xHandle3     // handler
                            );           // core

}

void K32_mqtt::stop() 
{
  if (xHandle1 != NULL) vTaskDelete(xHandle1);
  if (xHandle2 != NULL) vTaskDelete(xHandle2);
  if (xHandle3 != NULL) vTaskDelete(xHandle3);
  xHandle1 = NULL;
  xHandle2 = NULL;
  xHandle3 = NULL;

  // mqttClient->disconnect();
}

void K32_mqtt::publish(const char *topic, const char *payload, uint8_t qos, bool retain) {
  esp_mqtt_client_publish(mqttClient, topic, payload, strlen(payload), qos, retain);
}

void K32_mqtt::subscribe(mqttsub sub) {
  if (subscount >= MQTT_SUBS_SLOTS) return; 
  subscriptions[subscount] = sub;
  subscount += 1;
}

void K32_mqtt::broker(const char *_broker) {
  this->conf.broker = _broker;
  // mqttClient->setServer(conf.broker, 1883);
}

bool K32_mqtt::isConnected() {
  return this->connected;
}


// /*
//  *   PRIVATE
//  */

void K32_mqtt::check(void *parameter)
{
  K32_mqtt *that = (K32_mqtt *)parameter;
  TickType_t xFrequency = pdMS_TO_TICKS(2000);

  // Wait for WIFI to first connect
  while(!that->wifi->isConnected()) delay( 300 );

  esp_mqtt_client_start(that->mqttClient);
  that->emit("mqtt/started");

  while (true)
  {
    if (!that->connected && that->wifi->isConnected()) {
        // that->mqttClient->connect();
        // LOG("MQTT: reconnecting");
    }

    vTaskDelay(xFrequency);
  }
  vTaskDelete(NULL);
}

void K32_mqtt::beat(void *parameter)
{
  K32_mqtt* that = (K32_mqtt*) parameter;
  TickType_t xFrequency = pdMS_TO_TICKS(that->conf.beatInterval);

  String myID = String(that->k32->system->id());

  while(true) {
    if (that->connected) {
      that->publish("k32/monitor/beat", myID.c_str());
      LOG("MQTT: beat published");
    }
    vTaskDelay( xFrequency );
  }

  vTaskDelete(NULL);
}

void K32_mqtt::beacon(void *parameter)
{
  K32_mqtt* that = (K32_mqtt*) parameter;
  TickType_t xFrequency = pdMS_TO_TICKS(that->conf.beaconInterval);
  String status="";

  while(true) {
    if (that->connected) {
      
      status="";

      // identity
      status += String(that->k32->system->id())+"|";
      status += String(that->k32->system->channel())+"|"+"|";
      status += String(K32_VERSION)+"|"+"|";

      // wifi 
      byte mac[6];
      WiFi.macAddress(mac);
      char shortmac[16];
      sprintf(shortmac, "%02X:%02X:%02X", mac[3], mac[4], mac[5]);
      status += String(shortmac)+"|"+"|";
      status += String(WiFi.localIP().toString().c_str())+"|"+"|";
      status += String(WiFi.RSSI())+"|"+"|";

      // (this->linkedIP) ? status += String(true) : status += String(false);
      status += String(true)+"|"+"|";

      // energy 
      // if (that->k32->system->stm32) status += String(that->k32->system->stm32->battery())+"|"+"|";
      // else status += String(0)+"|"+"|";

      // audio 
      // if (that->intercom->audio) {
      //   status += String(that->intercom->audio->isSdOK())+"|";
      //   (that->intercom->audio->media() != "") ? status += String(that->intercom->audio->media().c_str()) : status += String("stop")+"|";
      //   status += String(that->intercom->audio->error().c_str())+"|";
      // }
      // else {
      //   status += String(false)+"|";   /// TODO : SD check without audio engine
      //   status += String("stop")+"|";
      //   status += String("")+"|";
      // }

      // sampler
      // if (that->intercom->audio && that->intercom->audio->sampler) status += String(that->intercom->audio->sampler->bank())+"|";
      // else status += String(0)+"|";

      // filesync 
      // status += String(sync_size());
      // status += String(sync_getStatus().c_str());
      status += String(0)+"|";   // SYNC count files
      status += String("");  // SYNC erro

      // that->mqttClient->publish("k32/monitor/status", status.c_str(), 0, true) ; 
    }
    vTaskDelay( xFrequency );
  }
  vTaskDelete(NULL);
}
