/*
  KESP_SETTINGS.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef KESP_SETTINGS_h
#define KESP_SETTINGS_h

#include "Arduino.h"
#include <EEPROM.h>

class KESP_SETTINGS {
  public:
    KESP_SETTINGS(String keys[16]) {
      this->lock = xSemaphoreCreateMutex();

      xSemaphoreTake(this->lock, portMAX_DELAY);
      for (byte k=0; k<16; k++) this->keys[k] = keys[k];
      EEPROM.begin(16);
      for (byte k=0; k<16; k++) this->values[k] = EEPROM.read(k);
      EEPROM.end();
      xSemaphoreGive(this->lock);
    };

    void set(String key, byte value) {
      xSemaphoreTake(this->lock, portMAX_DELAY);
      for (byte k=0; k<16; k++)
        if (this->keys[k] == key) {
          EEPROM.begin(16);
          EEPROM.write(k, value);
          EEPROM.end();
          this->values[k] = value;
          xSemaphoreGive(this->lock);
          return;
        }
       xSemaphoreGive(this->lock);
    }

    byte get(String key) {
      xSemaphoreTake(this->lock, portMAX_DELAY);
      for (byte k=0; k<16; k++)
        if (this->keys[k] == key) {
          byte value = this->values[k];
          xSemaphoreGive(this->lock);
          return value;
        }
      xSemaphoreGive(this->lock);
      return 0;
    }


  private:
    SemaphoreHandle_t lock;
    String keys[16];
    byte values[16];
};

#endif
