/*
  K32_anim.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_anim_h
#define K32_anim_h

#define LEDS_DATA_SLOTS  64

#include "../K32_ledstrip.h"

//
// BASE ANIM
//
class K32_anim {
  public:
    K32_anim(String name) {
      this->_name = name;
      this->newdata = xSemaphoreCreateMutex();
    }
    
    // retrieve name (set by anim class)
    String name () { return this->_name; };

    // init called when anim starts to play (can be overloaded by anim class)
    // waiting for initdata ensure that nothing is really played before first data are pushed
    // setting startTime can be usefull..
    void init() { 
      this->waitData();
      this->refresh();
      this->startTime = millis(); 
    }

    // loop called by dedicated xtask
    // this is a prototype, mus be defined in specific anim class
    // return true/false to loop or not
    // can check or block waiting for new external data, or run on itself
    virtual bool loop ( K32_ledstrip* strip ) { 
      this->waitData();
      LOG("ANIM: newdata pushed !");
      return false; 
    };

    // change one element in data
    K32_anim* set(int k, int value) { 
      if (k < LEDS_DATA_SLOTS) this->data[k] = value; 
      return this;
    }

    // signal that data has been updated
    K32_anim* refresh() {
      xSemaphoreGive(this->newdata);
      return this;
    }

    // block until refresh is called
    K32_anim* waitData() {
      xSemaphoreTake(this->newdata, (TickType_t) 1);  // lock it, if not already token
      xSemaphoreTake(this->newdata, portMAX_DELAY);   // block until next refresh
      return this;
    }

    // set a new data and signal the update
    K32_anim* push(int* frame, int size) {
      for(int k=0; k<size; k++) this->set(k, frame[k]);
      this->refresh();
      return this;
    }


  protected:

    void delay(int ms) { vTaskDelay(pdMS_TO_TICKS(ms)); }
    
    int data[LEDS_DATA_SLOTS];
    SemaphoreHandle_t newdata;  
    unsigned long startTime = 0;
    String _name = "?"; 
};

#endif