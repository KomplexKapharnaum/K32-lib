/*
  K32_wifi.cpp
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/

#include "Arduino.h"
#include "K32_wifi.h"

#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <WiFi.h>


/*
 *   PUBLIC
 */

K32_wifi::K32_wifi(String nameDevice = "K32") : nameDevice(nameDevice) {

  this->lock = xSemaphoreCreateMutex();
  ArduinoOTA.setHostname(this->nameDevice.c_str());
  this->_broadcastIP = IPAddress(255, 255, 255, 255);

  // LOOP task
  xTaskCreate( this->task,        // function
                "wifi_task",      // task name
                10000,             // stack memory
                (void*)this,      // args
                1,                // priority
                NULL);            // handler
};


void K32_wifi::ota(bool enable) {
  this->otaEnable = enable;
}


void K32_wifi::staticIP(String ip, String gateway, String mask) {
  IPAddress addrIP;
  addrIP.fromString(ip);
  IPAddress gateIP;
  if (gateway == "auto") {
   gateIP.fromString(ip);
   gateIP[3] = 1;
  }
  else gateIP.fromString(gateway);
  IPAddress maskIP;
  maskIP.fromString(mask);
  WiFi.config(addrIP, gateIP, maskIP);
}

void K32_wifi::staticIP(String ip) {
  this->staticIP(ip, "auto", "255.255.255.0");
}


void K32_wifi::connect(const char* ssid, const char* password) {
  WiFi.mode(WIFI_STA);
  WiFi.onEvent(&K32_wifi::event);
  WiFi.setHostname(this->nameDevice.c_str());
  if (password != NULL) WiFi.begin(ssid, password);
  else WiFi.begin(ssid);
}


bool K32_wifi::wait(int timeout_s) {
  byte retries = 0;
  while(retries < timeout_s*10) {
    delay(100);
    if (this->ok) return true;
    retries += 1;
  }
  LOG("\nWIFI: timeout is over..\n");
  return false;
}


bool K32_wifi::isOK() {
  return this->ok;
}


IPAddress K32_wifi::broadcastIP() {
  IPAddress b;
  xSemaphoreTake(this->lock, portMAX_DELAY);
  b = this->_broadcastIP;
  xSemaphoreGive(this->lock);
  return b;
}




/*
 *   PRIVATE
 */

 bool K32_wifi::ok = false;
 byte K32_wifi::retry = 0;
 bool K32_wifi::didConnect = false;
 bool K32_wifi::didDisconnect = false;


 void K32_wifi::event(WiFiEvent_t event) {
   if (event == SYSTEM_EVENT_STA_DISCONNECTED) {
     if (K32_wifi::ok) {
       K32_wifi::didDisconnect = true;
       K32_wifi::ok = false;
     }
     else LOG("WIFI: can't connect...");
     K32_wifi::retry += 1;
   }
   else if (event == SYSTEM_EVENT_STA_GOT_IP) {
     K32_wifi::retry = 0;
     if (K32_wifi::ok) return;
     K32_wifi::ok = true;
     K32_wifi::didConnect = true;
   }
 }


 void K32_wifi::task( void * parameter ) {
   K32_wifi* that = (K32_wifi*) parameter;
   TickType_t xFrequency = pdMS_TO_TICKS(10);

   while(true) {

     // CONNECTED
     if (that->didConnect) {

        that->didConnect = false;

        // INFO
        LOGINL("WIFI: connected = ");
        LOG(WiFi.localIP());

        // OTA
        if (!that->otaEnable || !that->ok) return;
        ArduinoOTA.begin();
        LOGINL("OTA: started = ");
        LOG(that->nameDevice);

        // BROADCAST
        IPAddress myIP = WiFi.localIP();
        IPAddress mask = WiFi.subnetMask();
        xSemaphoreTake(that->lock, portMAX_DELAY);
        that->_broadcastIP[0] = myIP[0] | (~mask[0]);
        that->_broadcastIP[1] = myIP[1] | (~mask[1]);
        that->_broadcastIP[2] = myIP[2] | (~mask[2]);
        that->_broadcastIP[3] = myIP[3] | (~mask[3]);
        xSemaphoreGive(that->lock);

     }

     // DISCONNECTED
     if (that->didDisconnect) {

       that->didDisconnect = false;

       // INFO
       LOG("WIFI: disconnected");
       LOGF("WIFI: reconnecting.. %i\n", K32_wifi::retry);
       WiFi.reconnect();

     }

     // OTA Loop
     if (that->otaEnable && that->ok) ArduinoOTA.handle();

     vTaskDelay( xFrequency );
   }

   vTaskDelete(NULL);
 }
