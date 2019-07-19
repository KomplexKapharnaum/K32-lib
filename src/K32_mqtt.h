/*
  K32_mqtt.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_mqtt_h
#define K32_mqtt_h

class K32_mqtt;      // prevent cicular include error
#include "K32.h"

#define MQTT_SOCKET_TIMEOUT 2
#include <PubSubClient.h>

#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>

class K32_mqtt {
  public:
    K32_mqtt(mqttconf conf, K32* engine);
    void reconnect();

//     const char* id_path();
//     const char* chan_path();
//     OSCMessage status();

  private:
    SemaphoreHandle_t lock;
    PubSubClient* mqttc;
    WiFiClient* client;

    static void loop(void * parameter);
    static void beat( void * parameter );
    void dispatch(char* topic, byte* payload, unsigned int length);
    void dispatch2(String &topic, String &payload);

    bool noteOFF = true;

    mqttconf conf;
    K32* engine;
};

#endif
