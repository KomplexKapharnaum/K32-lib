/*
  K32_mqtt.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_mqtt_h
#define K32_mqtt_h

#include "system/K32_system.h"
#include "network/K32_wifi.h"
#include "audio/K32_audio.h"
#include "light/K32_light.h"
#include "xtension/K32_remote.h"

#define CONFIG_ASYNC_TCP_RUNNING_CORE 0
#include <AsyncMqttClient.h>

struct mqttconf
{
  const char *broker;
  int beatInterval;
  int beaconInterval;
};

class K32_mqtt {
  public:
    K32_mqtt(K32_system *system, K32_wifi *wifi, K32_audio *audio, K32_light *light, K32_remote *remote);
    void start(mqttconf conf);
    void publish(const char *topic, const char *payload = (const char *)nullptr, uint8_t qos=1, bool retain=false);

  private:
    SemaphoreHandle_t lock;
    AsyncMqttClient* mqttClient;

    static void check(void * parameter);
    static void beat( void * parameter );
    static void beacon( void * parameter );

    void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
    void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) ;

    void dispatch(char* topic, char* payload, size_t length);

    bool connected;
    bool noteOFF = true;

    mqttconf conf;

    K32_system *system;
    K32_wifi *wifi;
    K32_audio *audio;
    K32_light *light;
    K32_remote *remote;
};

#endif
