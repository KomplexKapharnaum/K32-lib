/*
  K32_wifi.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_wifi_h
#define K32_wifi_h

#define KWIFI_CONNECTION_TIMEOUT 10000

#include "system/K32_log.h"
#include <WiFi.h>

struct wificonf
{
  const char *ssid;
  const char *password;
  const char *ip;
};

class K32_wifi {
  public:
    K32_wifi(String nameDevice);

    void ota(bool enable);

    void staticIP(String ip, String gateway, String mask);
    void staticIP(String ip);

    void connect(const char* ssid, const char* password = NULL);
    void connect();

    void add(const char* ssid);
    void add(const char* ssid, const char* password);
    void add(const char* ssid, const char* password, String ip, String mask, String gateway);

    bool wait(int timeout_s);
    bool ping();

    bool isConnected();
    long getRSSI();
    void getWiFiLevel(uint8_t (&led_display)[6]);

    bool find(String ssid);

    IPAddress broadcastIP();

    String nameDevice;

  private:
    SemaphoreHandle_t lock;
    static void task( void * parameter );

    byte otaEnable = true;

    static bool ok;
    static bool didConnect;
    static bool didDisconnect;
    static void event(WiFiEvent_t event);
    
    long engageConnection = 0;
    byte retry = 0;

    IPAddress _broadcastIP;

    String _staticIP;
    String _staticGW;
    String _staticMK;
    
    String _ssid;
    String _password;

};


#endif