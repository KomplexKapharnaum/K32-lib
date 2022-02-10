/*
  K32_mqtt.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_mqtt_h
#define K32_mqtt_h

#include <class/K32_plugin.h>
#include <K32_wifi.h>
#include <K32_light.h>
#include <hardware/K32_stm32.h>

#include <mqtt_client.h>

#define MQTT_SUBS_SLOTS 16

typedef void (*mqttcbPtr)(char *payload, size_t length);

struct mqttconf
{
  const char *broker;
  int beatInterval;
  int statusInterval;

};

struct mqttsub
{
  const char* topic;
  int qos;
  mqttcbPtr callback;
};


class K32_mqtt : K32_plugin {
  public:
    K32_mqtt(K32* k32, K32_wifi* wifi, K32_stm32* stm32, K32_light* light);  // TODO: remove wifi dependancy (use intercom)
    void start(mqttconf conf);
    void stop();
    void broker(const char *_broker);
    void publish(const char *topic, const char *payload = (const char *)nullptr, uint8_t qos=0, bool retain=false);
    void subscribe(mqttsub sub);

    bool isConnected();

    void command(Orderz* order);

  private:
    SemaphoreHandle_t lock;
    esp_mqtt_client_handle_t mqttClient;

    static esp_err_t mqtt_event(esp_mqtt_event_handle_t event);

    static void check(void * parameter);
    static void beat( void * parameter );
    static void beacon( void * parameter );

    // void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
    // void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t length, size_t index, size_t total);

    bool connected;

    mqttconf conf;
    mqttsub subscriptions[MQTT_SUBS_SLOTS];
    int subscount = 0;

    TaskHandle_t xHandle1 = NULL;
    TaskHandle_t xHandle2 = NULL;
    TaskHandle_t xHandle3 = NULL;

    K32_wifi *wifi;
    K32_stm32 *stm32;
    K32_light *light;
};

#endif
