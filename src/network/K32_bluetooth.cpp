/*
  K32_bluetooth.cpp
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/

#include "network/K32_bluetooth.h"


/*
 *   PUBLIC
 */

K32_bluetooth::K32_bluetooth(String nameDevice, K32_system *system, K32_audio *audio, K32_light *light) 
                                                      : nameDevice(nameDevice), system(system), audio(audio), light(light)
{
  this->lock = xSemaphoreCreateMutex();

  this->conCallback = nullptr;
  this->disconCallback = nullptr;

  this->serial = new BluetoothSerial();
  this->serial->register_callback(this->event);
  this->serial->begin(nameDevice);
  LOGF("BT: started (%s)\n", nameDevice);

  // STATE task
  xTaskCreatePinnedToCore(this->state,   // function
              "bt_state",         // task name
              2000,              // stack memory
              (void *)this,      // args
              1,                 // priority
              NULL,              // handler
              0);                // core

  // // LISTEN server
  xTaskCreatePinnedToCore( this->server,          // function
                "bt_server",          // server name
                10000,               // stack memory
                (void*)this,        // args
                5,                  // priority
                NULL,              // handler
                0);                // core 
};

bool K32_bluetooth::isConnected()
{
  return K32_bluetooth::ok;
}

void K32_bluetooth::onConnect( void (*callback)(void) ) {
  this->conCallback = callback;
}

void K32_bluetooth::onDisconnect( void (*callback)(void) ) {
  this->disconCallback = callback;
}


/*
 *   PRIVATE
 */

bool K32_bluetooth::ok = false;
bool K32_bluetooth::didConnect = false;
bool K32_bluetooth::didDisconnect = false;

void K32_bluetooth::event(esp_spp_cb_event_t ev, esp_spp_cb_param_t *param)
{
  if(ev == ESP_SPP_SRV_OPEN_EVT)
  {
    if (K32_bluetooth::ok) return;
    K32_bluetooth::didConnect = true;
  }
  else if(ev == ESP_SPP_CLOSE_EVT )
  {
    if (!K32_bluetooth::ok) return;
    K32_bluetooth::didDisconnect = true;
    K32_bluetooth::ok = false;
  }

}

void K32_bluetooth::state(void *parameter)
{
  K32_bluetooth *that = (K32_bluetooth *)parameter;
  TickType_t xFrequency = pdMS_TO_TICKS(500);

  while (true)
  {

    // DISCONNECTED
    if (K32_bluetooth::didDisconnect)
    {
      LOG("BT: disconnected");
      K32_bluetooth::didDisconnect = false;
      K32_bluetooth::ok = false;

      // Callback
      if (that->disconCallback != nullptr) that->disconCallback();
    }

    // CONNECTED
    if (K32_bluetooth::didConnect)
    {
      // INFO
      LOG("BT: connected");
      K32_bluetooth::didConnect = false;
      K32_bluetooth::ok = true;

      // Callback
      if (that->conCallback != nullptr) that->conCallback();
    }

    vTaskDelay(xFrequency);
  }

  vTaskDelete(NULL);
}


void K32_bluetooth::server(void *parameter)
{
  K32_bluetooth *that = (K32_bluetooth *)parameter;

  while (true)
    if (that->isConnected()) {
      
      while(that->serial->available()) 
      {
        LOGINL("BT received:");
        LOG(that->serial->read());
      }
      delay(20);
    }
    else delay(300);

  vTaskDelete(NULL);
}