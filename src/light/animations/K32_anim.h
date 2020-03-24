/*
  K32_anim.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_anim_h
#define K32_anim_h

#define LEDS_PARAM_SLOTS  16

#include "../K32_ledstrip.h"

//
// BASE ANIM
//
class K32_anim {
  public:
    K32_anim(String name) {
      this->_name = name;
      this->critical = xSemaphoreCreateMutex();
    }

    void init() { this->startTime = millis(); }
    virtual bool loop ( K32_ledstrip* strip ) { return false; };

    String name () { return this->_name; };
  
    void delay(int ms) { vTaskDelay(pdMS_TO_TICKS(ms)); }

    void setParam(int k, int value) { if (k < LEDS_PARAM_SLOTS) this->params[k] = value; }
    int params[LEDS_PARAM_SLOTS];

    void lock() { xSemaphoreTake(this->critical, portMAX_DELAY); }
    void unlock() { xSemaphoreGive(this->critical); }
    SemaphoreHandle_t critical;

  protected:
    unsigned long startTime = 0;
    String _name = "?"; 
};

#endif