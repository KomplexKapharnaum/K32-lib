/*
  K32_bluetooth.cpp
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/

#include "K32_bluetooth.h"
#include <esp_bt.h>


/*
 *   PUBLIC
 */

K32_bluetooth::K32_bluetooth(K32* k32, String nameDevice) : K32_plugin("bt", k32)
{
  esp_bt_controller_mem_release(ESP_BT_MODE_BLE);

  this->lock = xSemaphoreCreateMutex();

  this->serial = new BluetoothSerial();
  this->serial->register_callback((esp_spp_cb_t) this->event);
  this->serial->begin(nameDevice);


  LOGF("BT: started (%s)\n", nameDevice);

  // STATE task
  xTaskCreate(this->state,   // function
              "bt_state",         // task name
              1500,              // stack memory
              (void *)this,      // args
              1,                 // priority
              NULL              // handler
              );               


  // // LISTEN server
  xTaskCreatePinnedToCore( this->server,          // function
                "bt_server",          // server name
                5000,               // stack memory
                (void*)this,        // args
                5,                  // priority
                NULL,              // handler
                0);                // core 
};

cbPtrBTcon K32_bluetooth::conCallback{ nullptr };
cbPtrBTcon K32_bluetooth::disconCallback{ nullptr };

void K32_bluetooth::onConnect( cbPtrBTcon callback ) {
  conCallback = callback;
}

void K32_bluetooth::onDisconnect( cbPtrBTcon callback ) {
  disconCallback = callback;
}

bool K32_bluetooth::isConnected()
{
  return K32_bluetooth::ok;
}

cbPtrBTdata K32_bluetooth::cmdCallback{ nullptr };

void K32_bluetooth::onCmd( cbPtrBTdata callback ) 
{
  this->cmdCallback = callback;
}

long K32_bluetooth::getRSSI()
{ 
  if (!this->isConnected()) return 0;
  return 60;  // TODO: BT Rssi ?
}

void K32_bluetooth::send(String str) {
  this->serial->print(str);
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

      that->emit( "bt/disconnected" );

      // Callback // TODO: NO NEED FOR CALBACK ! use EVents
      if (that->disconCallback != nullptr) that->disconCallback();
    }

    // CONNECTED
    if (K32_bluetooth::didConnect)
    {
      // INFO
      LOG("BT: connected");
      K32_bluetooth::didConnect = false;
      K32_bluetooth::ok = true;

      that->emit( "bt/connected" );

      // Callback // TODO: NO NEED FOR CALBACK ! use EVents
      if (that->conCallback != nullptr) that->conCallback();
    }

    vTaskDelay(xFrequency);
  }

  vTaskDelete(NULL);
}


void K32_bluetooth::server(void *parameter)
{
  K32_bluetooth *that = (K32_bluetooth *)parameter;
  const int bufferSize = 256;
  char buffer[bufferSize];
  int size = 0;

  while (true)
    if (that->isConnected()) {
      
      while(that->serial->available()) {
        buffer[size] = that->serial->read();
        size += 1;

        // buffer is full
        if (size == bufferSize-1) {
          buffer[size] = '\n';
          size += 1;
        }

        // process
        if (buffer[size-1] == '\n') {
          buffer[size-1] = '\0';
          that->dispatch(buffer, size-1);
          size=0;
        }
      }        
      delay(20);
    }
    else delay(300);

  vTaskDelete(NULL);
}


void K32_bluetooth::splitString(char *data, const char *separator, int index, char *result)
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


void K32_bluetooth::dispatch(char* data, size_t length)
{
  LOGINL("BT received: ");
  LOG(data);

  // TOPIC
  char topic[length];
  this->splitString(data, " ", 0, topic);

  // PAYLOAD
  int paySize = length-strlen(topic);
  if (paySize<1) paySize = 1;
  char payload[paySize];
  memcpy( payload, &data[strlen(topic)+1], paySize );
  payload[paySize-1] = '\0';

  LOGINL("BT topic: ");
  LOG(topic);
  LOGINL("BT payload: ");
  LOG(payload);

  Orderz* newOrder = new Orderz(topic);
  char* p = strtok(payload, "§");
  while(p != NULL) {
    newOrder->addData(p);
    p = strtok(NULL, "§");
  }
  this->cmd( newOrder );

  // TODO emit received (for custom callback)
}

/* 
  COMMAND EXECUTION
*/

void K32_bluetooth::command( Orderz* order ) {
  if (strcmp(order->action, "commandZ") == 0) {

  }
}

