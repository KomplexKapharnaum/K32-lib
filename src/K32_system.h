/*
  K32_system.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_system_h
#define K32_system_h

#include <Preferences.h>
#include "K32_stm32.h"

class K32_system {
  public:
    K32_system() {
      this->lock = xSemaphoreCreateMutex();

      xSemaphoreTake(this->lock, portMAX_DELAY);
      preferences.begin("k32-app", false);
      xSemaphoreGive(this->lock);
    };

    int id() {
      int id;
      xSemaphoreTake(this->lock, portMAX_DELAY);
      id = preferences.getUInt("id", 0);
      xSemaphoreGive(this->lock);
      return id; 
    }

    void id(int id) {
      xSemaphoreTake(this->lock, portMAX_DELAY);
      preferences.putUInt("id", id);
      xSemaphoreGive(this->lock);
    }

    int channel() {
      int chan;
      xSemaphoreTake(this->lock, portMAX_DELAY);
      chan = preferences.getUInt("channel", 15);
      xSemaphoreGive(this->lock);
      return chan;
    }

    void channel(int channel) {
      xSemaphoreTake(this->lock, portMAX_DELAY);
      preferences.putUInt("channel", channel);
      xSemaphoreGive(this->lock);
    }

    int hw() {
      int hw;
      xSemaphoreTake(this->lock, portMAX_DELAY);
      hw = preferences.getUInt("hw", 0);
      xSemaphoreGive(this->lock);
      return hw;
    }

    void hw(int hwrevision) {
      xSemaphoreTake(this->lock, portMAX_DELAY);
      preferences.putUInt("hw", hwrevision);
      xSemaphoreGive(this->lock);
    }

    String name() {
      String name = "esp-" + String(this->id());
      return name;
    }

    void reset() {
      xSemaphoreTake(this->lock, portMAX_DELAY);
      preferences.end();
      xSemaphoreGive(this->lock);
      if (stm32) stm32->reset();
      while (true);
    }

    void shutdown() {
      xSemaphoreTake(this->lock, portMAX_DELAY);
      preferences.end();
      xSemaphoreGive(this->lock);
      if (stm32) stm32->shutdown();
    }

    int ledpin(int i) {
      return LEDS_PIN[hw()-1][i];
    }

    K32_stm32 *stm32 = NULL;
  
  private:
    SemaphoreHandle_t lock;
    Preferences preferences;

};

#endif
