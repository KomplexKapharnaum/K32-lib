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

K32_mqtt::K32_mqtt(K32* k32, K32_wifi* wifi) : K32_plugin("mqtt", k32), wifi(wifi) {}


void K32_mqtt::start(mqttconf conf)
{
  this->conf = conf;
  lock = xSemaphoreCreateMutex();
  connected = false;
  
  mqttClient = new AsyncMqttClient();
  mqttClient->setServer(conf.broker, 1883);

  mqttClient->onConnect([this](bool sessionPresent){
    this->connected = true;
    LOG("MQTT: connected");

    String myChan = String(k32->system->channel()); 
    String myID =String(k32->system->id());

    this->mqttClient->subscribe(("k32/c" + myChan + "/#").c_str(), 1);
    LOG("MQTT: subscribed to " + ("k32/c" + myChan + "/#"));

    this->mqttClient->subscribe(("k32/e" + myID + "/#").c_str(), 1);
    LOG("MQTT: subscribed to " + ("k32/e" + myID + "/#"));

    this->mqttClient->subscribe("k32/all/#", 1);
    LOG("MQTT: subscribed to k32/all/#");

    MDNS.addService("_mqttc", "_tcp", 1883);
    mdns_service_instance_name_set("_mqttc", "_tcp", ("MQTTc._"+k32->system->name()).c_str());
    
    // CUSTOM subscriptions
    for (int k=0; k<subscount; k++) {
      this->mqttClient->subscribe(subscriptions[k].topic, subscriptions[k].qos);
      LOGF2("MQTT: subscribed to %s with QOS %i\n", subscriptions[k].topic, subscriptions[k].qos);
    }

  });
  

  mqttClient->onDisconnect([this](AsyncMqttClientDisconnectReason reason){
    this->connected = false;
    mqttClient->disconnect();
    LOG("MQTT: disconnected");
  });

  mqttClient->onMessage([this](char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total){
    // LOG("MQTT received.");
    // LOGINL("  topic: ");
    // LOG(topic);
    // LOGINL("  qos: ");
    // LOG(properties.qos);
    // LOGINL("  dup: ");
    // LOG(properties.dup);
    // LOGINL("  retain: ");
    // LOG(properties.retain);
    // LOGINL("  len: ");
    // LOG(len);
    // LOGINL("  index: ");
    // LOG(index);
    // LOGINL("  total: ");
    // LOG(total);

    char useload[len+1];
    strncpy(useload, payload, len);
    useload[len] = 0;

    // TOPIC: k32/all/[motor]   or   k32/c[X]/[motor]   or   k32/e[X]/[motor]
    LOGF3("MQTT: recv %s with payload(%d) %s\n", topic, len, useload);

    // CUSTOM SUBSCRIBES (takes priority)
    // TODO: move outside mqtt thread (-> use K32::run)
    for (int k=0; k<subscount; k++)
      if ( strcmp(subscriptions[k].topic, topic) == 0 ) {
        subscriptions[k].callback(useload, len);
      }

    // FORWARD TO INTERCOM
    char* command;
    command = strchr(topic, '/')+1;

    // Orderz* newOrder = new Orderz(command);
    
    // char* p = strtok(useload, "|");
    // while(p != NULL) {
    //   newOrder->addData(p);
    //   p = strtok(NULL, "|");
    // }
    
    // this->intercom->queue( newOrder );

    // this->intercom->queue( "super/genial" );

    LOG("emit");
    this->intercom->ee.emit("cool", nullptr);
    LOG("done emit");
  });

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

  mqttClient->disconnect();
  
}

void K32_mqtt::publish(const char *topic, const char *payload, uint8_t qos, bool retain) {
  this->mqttClient->publish(topic, qos, retain, payload); 
}

void K32_mqtt::subscribe(mqttsub sub) {
  if (subscount >= MQTT_SUBS_SLOTS) return; 
  subscriptions[subscount] = sub;
  subscount += 1;
}

void K32_mqtt::broker(const char *_broker) {
  this->conf.broker = _broker;
  mqttClient->setServer(conf.broker, 1883);
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

  while (true)
  {
    if (!that->connected && that->wifi->isConnected()) {
        that->mqttClient->connect();
        LOG("MQTT: reconnecting");
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
      if (that->mqttClient->publish("k32/monitor/beat", 0, false, myID.c_str()) ) LOG("MQTT: beat published");
      else LOG("MQTT: beat not published");
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

      that->mqttClient->publish("k32/monitor/status", 0, true, status.c_str()) ; 
    }
    vTaskDelay( xFrequency );
  }
  vTaskDelete(NULL);
}
