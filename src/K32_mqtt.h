/*
  K32_mqtt.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_mqtt_h
#define K32_mqtt_h

class K32_mqtt;      // prevent cicular include error
#include "K32.h"
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
    void send(OSCMessage msg);

  private:
    SemaphoreHandle_t lock;
    PubSubClient* mqttc;

    static void loop(void * parameter);

    // void callback(char* topic, byte* payload, unsigned int length);

//     static void server( void * parameter );
//     static void beacon( void * parameter );
//     static void beat( void * parameter );

//     WiFiUDP* udp;         // must be protected with lock 
//     WiFiUDP* sendSock;
//     IPAddress linkedIP;

    mqttconf conf;
    K32* engine;
};

#endif
