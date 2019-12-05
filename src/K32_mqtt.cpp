/*
  K32_mqtt.cpp
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/

#include "Arduino.h"
#include "K32_mqtt.h"
#include <ESPmDNS.h>


/*
 *   PUBLIC
 */

K32_mqtt::K32_mqtt(K32_system *system, K32_wifi *wifi, K32_audio *audio, K32_light *light) 
                                : system(system), wifi(wifi), audio(audio), light(light) {}


void K32_mqtt::start(mqttconf conf)
{
  this->conf = conf;
  lock = xSemaphoreCreateMutex();
  connected = false;
  
  mqttClient = new AsyncMqttClient();
  mqttClient->setServer(conf.broker, 1883);

  mqttClient->onConnect([this](bool sessionPresent){
    this->connected = true;

    String myChan = String(this->system->channel()); 
    String myID =String(this->system->id());

    this->mqttClient->subscribe(("k32/c" + myChan + "/#").c_str(), 1);
    LOG("MQTT: subscribed to " + ("k32/c" + myChan + "/#"));

    this->mqttClient->subscribe(("k32/e" + myID + "/#").c_str(), 1);
    LOG("MQTT: subscribed to " + ("k32/e" + myID + "/#"));

    this->mqttClient->subscribe("k32/all/#", 1);
    LOG("MQTT: subscribed to k32/all/#");

    MDNS.addService("_mqttc", "tcp", 1883);
  });

  mqttClient->onDisconnect([this](AsyncMqttClientDisconnectReason reason){
    this->connected = false;
  });

  mqttClient->onMessage([this](char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total){
    // LOG("Publish received.");
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
    this->dispatch(topic, payload, len);
  });

  // LOOP client
  xTaskCreatePinnedToCore(this->check,    // function
                          "mqtt_check", // name
                          2000,         // stack memory
                          (void *)this,  // args
                          0,             // priority
                          NULL,          // handler
                          0);            // core

  // BEAT
  if (this->conf.beatInterval > 0)
    xTaskCreatePinnedToCore(this->beat,   // function
                            "mqtt_beat",  // server name
                            2000,         // stack memory
                            (void *)this, // args
                            0,            // priority
                            NULL,         // handler
                            0);           // core

  // BEACON
  if (this->conf.beaconInterval > 0)
    xTaskCreatePinnedToCore(this->beacon,   // function
                            "mqtt_beacon",  // server name
                            5000,         // stack memory
                            (void *)this, // args
                            1,            // priority
                            NULL,         // handler
                            0);           // core

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
    if (!that->connected && that->wifi && that->wifi->isConnected())
        that->mqttClient->connect();

    vTaskDelay(xFrequency);
  }
  vTaskDelete(NULL);
}

void K32_mqtt::beat(void *parameter)
{
  K32_mqtt* that = (K32_mqtt*) parameter;
  TickType_t xFrequency = pdMS_TO_TICKS(that->conf.beatInterval);

  while(true) {
    if (that->connected)
      if (that->mqttClient->publish("k32/monitor/beat", 0, false, "1") ) LOG("MQTT: beat published");
      else LOG("MQTT: beat not published");
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
      status += String(that->system->id())+"§";
      status += String(that->system->channel())+"§"+"§";
      status += String(K32_VERSION)+"§"+"§";

      // wifi 
      byte mac[6];
      WiFi.macAddress(mac);
      char shortmac[16];
      sprintf(shortmac, "%02X:%02X:%02X", mac[3], mac[4], mac[5]);
      status += String(shortmac)+"§"+"§";
      status += String(WiFi.localIP().toString().c_str())+"§"+"§";
      status += String(WiFi.RSSI())+"§"+"§";

      // (this->linkedIP) ? status += String(true) : status += String(false);
      status += String(true)+"§"+"§";

      // energy 
      if (that->system->stm32) status += String(that->system->stm32->battery())+"§"+"§";
      else status += String(0)+"§"+"§";

      // audio 
      if (that->audio) {
        status += String(that->audio->isSdOK())+"§";
        (that->audio->media() != "") ? status += String(that->audio->media().c_str()) : status += String("stop")+"§";
        status += String(that->audio->error().c_str())+"§";
      }
      else {
        status += String(false)+"§";   /// TODO : SD check without audio engine
        status += String("stop")+"§";
        status += String("")+"§";
      }

      // sampler
      if (that->audio && that->audio->sampler) status += String(that->audio->sampler->bank())+"§";
      else status += String(0)+"§";

      // filesync 
      // status += String(sync_size());
      // status += String(sync_getStatus().c_str());
      status += String(0)+"§";   // SYNC count files
      status += String("");  // SYNC erro

      that->mqttClient->publish("k32/monitor/status", 1, true, status.c_str()) ; 
    }
    vTaskDelay( xFrequency );
  }
  vTaskDelete(NULL);
}

void splitString(char *data, const char *separator, int index, char *result)
{
  char input[strlen(data)];
  strcpy(input, data);

  char *command = strtok(input, separator);
  for (int k = 0; k < index; k++)
    if (command != NULL)
      command = strtok(NULL, separator);

  if (command == NULL)
    strcpy(result, "");
  else
    strcpy(result, command);
}

void K32_mqtt::dispatch(char *topic, char *payload, size_t length)
{
  payload[length] = 0;

  // TOPIC: k32/all/[motor]   or   k32/c[X]/[motor]   or   k32/e[X]/[motor]

  LOGINL("MQTT: recv ");
  LOGINL(topic);
  LOG(payload);

  // ENGINE
  char motor[16];
  splitString(topic, "/", 2, motor);

  if (strcmp(motor, "reset") == 0)
  {
    this->system->reset();
  }

  else if (strcmp(motor, "shutdown") == 0)
  {
    this->system->shutdown();
  }

  else if (strcmp(motor, "channel") == 0)
  {
    if (strcmp(payload, "") == 0)
      return;
    byte chan = atoi(payload);
    if (chan > 0)
    {
      this->system->channel(chan);
      delay(100);
      this->system->reset();
    }
  }

  // OSC AUDIO
  else if (strcmp(motor, "audio") == 0 && this->audio)
  {

    char action[16];
    splitString(topic, "/", 3, action);

    // PLAY MEDIA
    if (strcmp(action, "play") == 0)
    {

      char path[128];
      splitString(payload, "§", 0, path);
      if (strcmp(path, "") == 0)
        return;
      this->audio->play(path);

      char volume[5];
      splitString(payload, "§", 1, volume);
      if (strcmp(volume, "") != 0)
        this->audio->volume(atoi(volume));

      char loop[5];
      splitString(payload, "§", 2, loop);
      if (strcmp(loop, "") != 0)
        this->audio->volume(atoi(loop) > 0);
    }

    // SAMPLER NOTEON
    else if (strcmp(action, "noteon") == 0)
    {

      char bank[5];
      splitString(payload, "§", 0, bank);
      char note[5];
      splitString(payload, "§", 1, note);

      if (strcmp(bank, "") == 0 || strcmp(note, "") == 0)
        return;

      this->audio->sampler->bank(atoi(bank));
      this->audio->play(this->audio->sampler->path(atoi(note)));

      char velocity[5];
      splitString(payload, "§", 2, velocity);
      if (strcmp(velocity, "") != 0)
        this->audio->volume(atoi(velocity));

      char loop[5];
      splitString(payload, "§", 3, loop);
      if (strcmp(loop, "") != 0)
        this->audio->loop(atoi(loop) > 0);
    }

    // SAMPELR NOTEOFF
    else if (strcmp(action, "noteoff") == 0)
    {
      char note[5];
      splitString(payload, "§", 0, note);
      if (this->audio->media() == this->audio->sampler->path(atoi(note)))
        this->audio->stop();
    }

    // STOP
    else if (strcmp(action, "stop") == 0)
    {
      this->audio->stop();
    }

    // VOLUME
    else if (strcmp(action, "volume") == 0)
    {
      char volume[5];
      splitString(payload, "§", 0, volume);
      this->audio->volume(atoi(volume));
    }

    // LOOP
    else if (strcmp(action, "loop") == 0)
    {
      char loop[5];
      splitString(payload, "§", 0, loop);
      this->audio->loop(atoi(loop) > 0);
    }
  }

  // MIDI RAW
  else if (strcmp(motor, "midi") == 0 && this->audio)
  {

    char val[16];
    splitString(payload, "-", 0, val);
    byte event = atoi(val) / 16;
    splitString(payload, "-", 1, val);
    byte note = atoi(val);
    splitString(payload, "-", 2, val);
    byte velo = atoi(val);

    // NOTE OFF
    if (this->noteOFF && (event == 8 || (event == 9 && velo == 0)))
    {
      if (this->audio->media() == this->audio->sampler->path(note))
        this->audio->stop();
    }

    // NOTE ON
    else if (event == 9)
      this->audio->play(this->audio->sampler->path(note), velo);

    // CC
    else if (event == 11)
    {

      // LOOP
      if (note == 1)
        this->audio->loop((velo > 63));

      // NOTEOFF enable
      else if (note == 2)
        this->noteOFF = (velo < 63);

      // VOLUME
      else if (note == 7)
        this->audio->volume(velo);

      // BANK SELECT
      // else if (note == 32) this->audio->sampler->bank(velo+1);

      // STOP ALL
      else if (note == 119 or note == 120)
        this->audio->stop();
    }
  }

  // OSC LEDS
  else if (strcmp(motor, "leds") == 0 && this->light)
  {

    char action[16];
    splitString(topic, "/", 3, action);

    // PYRAMID
    if (strcmp(action, "pyramid") == 0)
    {
    }

    // ALL
    else if (strcmp(action, "all") == 0)
    {
    }

    // STRIP
    else if (strcmp(action, "strip") == 0)
    {
    }

    // PIXEL
    else if (strcmp(action, "pixel") == 0)
    {
    }

    // PLAY ANIM
    else if (strcmp(action, "play") == 0)
    {

      char anim_name[16];
      splitString(payload, "§", 0, anim_name);
      K32_light_anim *anim = this->light->anim(anim_name);
      LOGINL("MQTT: leds play ");
      LOGINL(anim_name);

      char val[128];
      byte inc = 1;
      splitString(payload, "§", inc, val);
      while (strcmp(val, "") != 0 && (inc - 1) < LEDS_PARAM_SLOTS)
      {
        LOGINL(" ");
        LOGINL(atoi(val));
        anim->setParam(inc - 1, atoi(val));
        ++inc;
        splitString(payload, "§", inc, val);
      }
      LOG("");

      this->light->play(anim);
    }

    // STOP
    else if (strcmp(action, "stop") == 0 || strcmp(action, "blackout") == 0)
    {
      this->light->stop();
    }
  }
}
