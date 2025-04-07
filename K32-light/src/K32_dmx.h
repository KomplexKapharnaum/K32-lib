/*
  K32_dmx.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_dmx_h
#define K32_dmx_h

#include <LXESP32DMX.h>
#include "_libfast/pixel.h"
#include "esp_task_wdt.h"

enum DmxDirection { DMX_IN, DMX_OUT };


#ifndef DMX_SUB_SLOTS
  #define DMX_SUB_SLOTS 16
  typedef void (*cbPtrDmx )(const uint8_t *data, int length);

  struct dmxsub
  {
    int address;
    int framesize;
    cbPtrDmx callback;
  };
#endif

class K32_dmx {
  public:
    K32_dmx(const int DMX_PIN[3], DmxDirection dir) {
      
      // DIR pin
      if (DMX_PIN[0] > 0) {
        pinMode(DMX_PIN[0], OUTPUT);
        digitalWrite(DMX_PIN[0], (dir==DMX_OUT) ? HIGH : LOW);
      }
      
      // DMX out
      if (dir == DMX_OUT) 
      {
        if (DMX_PIN[1] > 0) {
          pinMode(DMX_PIN[1], OUTPUT);
          ESP32DMX.startOutput(DMX_PIN[1]);
          outputOK = true;
          LOG("DMX: output STARTED");
        }
        else LOG("DMX: invalid OUTPUT pin, DMXout disabled !");
      }

      // DMX in 
      else 
      {
        if (DMX_PIN[2] > 0) {
          pinMode(DMX_PIN[2], INPUT);
          ESP32DMX.startInput(DMX_PIN[2]);
          inputOK = true;   
          LOG("DMX: input STARTED");

          // Callbacks
          ESP32DMX.setDataReceivedCallback(K32_dmx::_onDmx);

        }
        else LOG("DMX: invalid INPUT pin, DMXin disabled !");
      }
      

    };

    // SET one value
    K32_dmx* set(int index, int value) 
    {
      if (outputOK) {
        xSemaphoreTake(ESP32DMX.lxDataLock, portMAX_DELAY);
        ESP32DMX.setSlot(index, value);
        xSemaphoreGive(ESP32DMX.lxDataLock);
      }
      return this;
    }

    // SET multiple values
    K32_dmx* setMultiple(int* values, int size, int offsetAdr = 1) 
    {
      if (outputOK) {
      // LOGF3("DMX: setMultiple %d %d %d\n",values[0], size, offsetAdr);
        xSemaphoreTake(ESP32DMX.lxDataLock, portMAX_DELAY);
        for (int i = 0; i < size; i++)
          ESP32DMX.setSlot(i+offsetAdr, values[i]);
        xSemaphoreGive(ESP32DMX.lxDataLock);
      }
      return this;
    }

    // SET multiple values
    K32_dmx* setMultiple(const uint8_t* values, int size, int offsetAdr = 1) 
    {
      if (outputOK) {
      // LOGF3("DMX: setMultiple %d %d %d\n",values[0], size, offsetAdr);
        xSemaphoreTake(ESP32DMX.lxDataLock, portMAX_DELAY);
        for (int i = 0; i < size; i++)
          ESP32DMX.setSlot(i+offsetAdr, values[i]);
        xSemaphoreGive(ESP32DMX.lxDataLock);
      }
      return this;
    }

    // Get values
    K32_dmx* get(int index)
    {
      if (inputOK) {
        xSemaphoreTake(ESP32DMX.lxDataLock, portMAX_DELAY);
        ESP32DMX.getSlot(index);
        xSemaphoreGive(ESP32DMX.lxDataLock);
      }
      return this;
    }

    static void onDmx( dmxsub subscription ) 
    {
      LOGF("DMX: address = %i\n", subscription.address);
      for (int k=0; k< DMX_SUB_SLOTS; k++)
        if (K32_dmx::subscriptions[k].address == 0) {
          K32_dmx::subscriptions[k] = subscription;
          break;
        }
    }

    static void onFullDmx( cbPtrDmx callback ) 
    {
      K32_dmx::fullCallback = callback;
    }

    static cbPtrDmx fullCallback;
    static dmxsub subscriptions[DMX_SUB_SLOTS];

  private:
    bool outputOK = false;
    bool inputOK = false;

    static void _onDmx(int length)
    {
      uint8_t* data = ESP32DMX.dmxData();

      // Callback Sub
      for (int k=0; k<DMX_SUB_SLOTS; k++) {
        if (K32_dmx::subscriptions[k].address > 0 && length-K32_dmx::subscriptions[k].address > 0) {
          K32_dmx::subscriptions[k].callback( 
            &data[ K32_dmx::subscriptions[k].address], 
            min(K32_dmx::subscriptions[k].framesize, length-(K32_dmx::subscriptions[k].address)) 
          );
        }
      }

      // Callback Full
      if (K32_dmx::fullCallback && length > 0) K32_dmx::fullCallback(data, length);
    }
};

dmxsub K32_dmx::subscriptions[DMX_SUB_SLOTS] = {0, 0, nullptr};
cbPtrDmx K32_dmx::fullCallback = nullptr;

#endif
