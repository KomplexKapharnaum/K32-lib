/*
  K32_wifi.cpp
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/

#include "K32_wifi.h"

#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <WiFi.h>
#include <ESP32Ping.h>

/*
 *   PUBLIC
 */

K32_wifi::K32_wifi(K32* k32) : K32_plugin("wifi", k32)
{
  this->lock = xSemaphoreCreateMutex();

  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  ArduinoOTA.setHostname(this->nameDevice.c_str());
  ArduinoOTA.setTimeout(2000);
  ArduinoOTA
    .onStart([this]() {

      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      LOG("OTA Start updating " + type);

      this->otaProgress = true;
      emit( "wifi/ota/started" );
    })
    .onEnd([this]() {
      this->otaProgress = false;
      cmd( "system/reset" );
    })
    .onError([this](ota_error_t error) {
      this->otaProgress = false;
      cmd( "system/reset" );
    });


  this->_broadcastIP = IPAddress(255, 255, 255, 255);

  this->_staticIP = "";
  this->_staticGW = "";
  this->_staticMK = "";

  this->_ssid = "";
  this->_password = "";

  // LOOP task
  xTaskCreatePinnedToCore(this->task,   // function
              "wifi_task",  // task name
              2000,         // stack memory
              (void *)this, // args
              1,            // priority
              NULL,              // handler
              0);                // core
};

cbPtrWifi K32_wifi::conCallback{ nullptr };
cbPtrWifi K32_wifi::disconCallback{ nullptr };

void K32_wifi::setHostname(String name) {
  this->nameDevice = name;
  ArduinoOTA.setHostname(this->nameDevice.c_str());
}

void K32_wifi::enableOta(bool enable)
{
  this->otaEnable = enable;
}

bool K32_wifi::otaInProgress()
{
  return this->otaProgress;
}

void K32_wifi::staticIP(String ip, String gateway, String mask)
{
  this->_staticIP = ip;
  this->_staticGW = gateway;
  this->_staticMK = mask;
}

void K32_wifi::staticIP(String ip)
{
  this->staticIP(ip, "auto", "255.0.0.0");
}

void K32_wifi::connect(const char *ssid, const char *password)
{

  WiFi.persistent(false); // Don't use FLASH to store wifi..
  WiFi.mode(WIFI_STA);
  WiFi.onEvent(&K32_wifi::wifiEvent);
  WiFi.setHostname(this->nameDevice.c_str());

  // apply static
  if (this->_staticIP != "")
  {
    IPAddress addrIP;
    addrIP.fromString(this->_staticIP);
    IPAddress gateIP;
    if (this->_staticGW == "auto")
    {
      gateIP.fromString(this->_staticIP);
      gateIP[3] = 1;
    }
    else
      gateIP.fromString(this->_staticGW);
    IPAddress maskIP;
    maskIP.fromString(this->_staticMK);
    WiFi.config(addrIP, gateIP, maskIP);
  }

  this->_ssid = String(ssid);
  if (password != NULL)
  {
    WiFi.begin(ssid, password);
    this->_password = String(password);
  }
  else
  {
    WiFi.begin(ssid);
    this->_password = "";
  }

  this->engageConnection = millis();
}

void K32_wifi::connect()
{
  if (this->_ssid != "")
  {
    if (this->_password != "")
      this->connect(this->_ssid.c_str(), this->_password.c_str());
    else
      this->connect(this->_ssid.c_str(), NULL);
  }
}

void K32_wifi::disconnect()
{
  WiFi.disconnect();
}

void K32_wifi::add(const char *ssid) {

}

void K32_wifi::add(const char *ssid, const char *password) {

}

void K32_wifi::add(const char *ssid, const char *password, String ip, String mask, String gateway) {

}


bool K32_wifi::wait(int timeout_s)
{
  byte retries = 0;
  while (retries < timeout_s * 10)
  {
    delay(100);
    if (this->ok)
      return true;
    retries += 1;
  }
  LOG("\nWIFI: timeout is over..\n");
  return false;
}

bool K32_wifi::ping()
{
  if (!WiFi.isConnected())
    return false;
  return Ping.ping(WiFi.gatewayIP());
}

bool K32_wifi::isConnected()
{
  return (K32_wifi::ok && WiFi.isConnected());
}

long K32_wifi::getRSSI()
{ 
  if (!this->isConnected()) return 0;
  return WiFi.RSSI();
}

void K32_wifi::getWiFiLevel(uint8_t (&led_display)[6])
{
  if (this->isConnected())
  {
    led_display[0] = 0;
    led_display[1] = 4;
    long wifiRSSI = this->getRSSI();
    long comp = -80; // Used to compare RSSI value
    LOGF("\nWIFI: Rssi : %ld \n", wifiRSSI);
    for (int l = 2; l < 6; l++)
    {
      if (wifiRSSI > comp)
      {
        led_display[l] = 4;
      }
      else
      {
        led_display[l] = 0;
      }
      comp += 10;
    }
  }
  else
  {
    led_display[0] = 4;
    for (int l = 1; l < 6; l++)
    {
      led_display[l] = 0;
    }

    LOG("\nWifi not connected \n");
  }
}

bool K32_wifi::find(String ssid) {
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; ++i) 
    if (WiFi.SSID(i) == ssid) return true;
  return false;
}

IPAddress K32_wifi::broadcastIP()
{
  IPAddress b;
  xSemaphoreTake(this->lock, portMAX_DELAY);
  b = this->_broadcastIP;
  xSemaphoreGive(this->lock);
  return b;
}

void K32_wifi::onConnect( cbPtrWifi callback ) {
  conCallback = callback;
}

void K32_wifi::onDisconnect( cbPtrWifi callback ) {
  disconCallback = callback;
}


/*
 *   PRIVATE
 */

bool K32_wifi::ok = false;
bool K32_wifi::didConnect = false;
bool K32_wifi::didDisconnect = false;

void K32_wifi::wifiEvent(WiFiEvent_t event)
{
  if (event == SYSTEM_EVENT_STA_DISCONNECTED)
  {
    if (K32_wifi::ok)
    {
      K32_wifi::didDisconnect = true;
      K32_wifi::ok = false;
    }
  }
  else if (event == SYSTEM_EVENT_STA_GOT_IP)
  {
    if (K32_wifi::ok) return;
    K32_wifi::didConnect = true;
  }
}

void K32_wifi::task(void *parameter)
{
  K32_wifi *that = (K32_wifi *)parameter;
  TickType_t xFrequency = pdMS_TO_TICKS(500);

  while (true)
  {

    // PING to confirm
    // if (K32_wifi::ok && !that->ping())
    // {
    //   LOG("WIFI: PING failed.. disconnecting ");
    //   K32_wifi::didDisconnect = true;
    //   K32_wifi::ok = false;
    // }

    // DISCONNECTED
    if (K32_wifi::didDisconnect)
    {
      LOG("WIFI: disconnected");
      that->emit( "wifi/disconnected" );
      ArduinoOTA.end();

      K32_wifi::ok = false;
      K32_wifi::didDisconnect = false;
      that->engageConnection = -1 * KWIFI_CONNECTION_TIMEOUT;

      // Callback // TODO: NO NEED FOR CALBACK ! use EVents
      if (that->disconCallback != nullptr) that->disconCallback();
    }

    // RECONNECT
    if (that->engageConnection != 0) {
      if ((millis() - that->engageConnection) > KWIFI_CONNECTION_TIMEOUT)
      {
        ++that->retry;
        if (that->engageConnection > 0)
          LOG("WIFI: can't establish connection..");

        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
        vTaskDelay(pdMS_TO_TICKS(100));

        LOGF("WIFI: reconnecting.. %i\n", that->retry);
        that->connect();
      }
    }

    // CONNECTED
    if (K32_wifi::didConnect)
    {
      that->engageConnection = 0;
      that->retry = 0;

      // INFO
      LOGINL("WIFI: connected = ");
      LOG(WiFi.localIP());
      that->emit( "wifi/connected" );

      // OTA
      if (that->otaEnable) {
        ArduinoOTA.begin();
        LOGINL("OTA: started = ");
        LOG(that->nameDevice);
      }
      else MDNS.begin(that->nameDevice.c_str());

      // BROADCAST
      IPAddress myIP = WiFi.localIP();
      IPAddress mask = WiFi.subnetMask();
      xSemaphoreTake(that->lock, portMAX_DELAY);
      that->_broadcastIP[0] = myIP[0] | (~mask[0]);
      that->_broadcastIP[1] = myIP[1] | (~mask[1]);
      that->_broadcastIP[2] = myIP[2] | (~mask[2]);
      that->_broadcastIP[3] = myIP[3] | (~mask[3]);
      xSemaphoreGive(that->lock);

      K32_wifi::didConnect = false;
      K32_wifi::ok = true;

      // Callback // TODO: NO NEED FOR CALBACK ! use EVents
      if (that->conCallback != nullptr) that->conCallback();
    }

    // OTA Loop
    ArduinoOTA.handle();

    vTaskDelay(xFrequency);
  }

  vTaskDelete(NULL);
}



/* 
  COMMAND EXECUTION
*/

void K32_wifi::command( Orderz* order ) {
  if (strcmp(order->action, "disconnect") == 0) {
    this->disconnect();
  }
}