/*
  K32_wifi.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_wifi_h
#define K32_wifi_h

#define KWIFI_CONNECTION_TIMEOUT 10000

#include "Arduino.h"
#include "K32_log.h"

#include <WiFi.h>



class K32_wifi {
  public:
    K32_wifi(String nameDevice);

    void ota(bool enable);

    void staticIP(String ip, String gateway, String mask);
    void staticIP(String ip);

    void connect(const char* ssid, const char* password);
    void reconnect();
    bool wait(int timeout_s);

    bool isOK();
    long getRSSI();
    void getWiFiLevel(uint8_t (&led_display)[6]);

    IPAddress broadcastIP();


  private:
    SemaphoreHandle_t lock;
    static void task( void * parameter );

    byte otaEnable = true;
    String nameDevice;

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
