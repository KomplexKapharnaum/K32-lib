/*
  K32_bluetooth.cpp
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/

#include "network/K32_bluetooth.h"

#define min(m, n) ((m) < (n) ? (m) : (n))
#define max(m, n) ((m) > (n) ? (m) : (n))


/*
 *   PUBLIC
 */

K32_bluetooth::K32_bluetooth(String nameDevice, K32_system *system, K32_audio *audio, K32_light *light, K32_remote *remote) 
                                                      : nameDevice(nameDevice), system(system), audio(audio), light(light), remote(remote)
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

void K32_bluetooth::onCmd( cbPtr callback ) 
{
  this->cmdCallback = callback;
}

K32_bluetooth::cbPtr K32_bluetooth::cmdCallback = nullptr;


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
  int paySize = max(1, length-strlen(topic));
  char payload[paySize];
  memcpy( payload, &data[strlen(topic)+1], paySize );
  payload[paySize-1] = '\0';

  // LOGINL("BT topic: ");
  // LOG(topic);
  // LOGINL("BT payload: ");
  // LOG(payload);

  // ENGINE
  char motor[16];
  this->splitString(topic, "/", 0, motor);

  if (strcmp(motor, "leds") == 0 && this->light)
  {

    char action[16];
    this->splitString(topic, "/", 1, action);

    // ALL
    if (strcmp(action, "all") == 0)
    {
      char color[8];
      this->splitString(payload, "§", 0, color);
      // TODO SET COLOR ALL !

    }

    // MEM
    else if (strcmp(action, "mem") == 0)
    {
      char mem[4];
      this->splitString(payload, "§", 0, mem);
      
      if (this->remote)
        this->remote->stmSetMacro( atoi(mem) );
    }

    // STOP
    else if (strcmp(action, "stop") == 0 || strcmp(action, "off") == 0 || strcmp(action, "blackout") == 0)
    {
      // TODO
    }

  }

  // if (K32_bluetooth::cmdCallback)
  //   K32_bluetooth::cmdCallback( 
  //     &data[ K32_artnet::conf.address-1 ], 
  //     min(K32_artnet::conf.framesize, length-(K32_artnet::conf.address-1)) 
  //   );
}