/*
  K32_modulator.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_modulator_h
#define K32_modulator_h

#define LEDS_MODDATA_SLOTS  16

#include "K32_anim.h"

//
// BASE ANIM
//
class K32_modulator {
  public:
    K32_modulator() {
      this->paramInUse = xSemaphoreCreateBinary();
      xSemaphoreGive(this->paramInUse);

      this->play();
    }
    
    // get/set name
    String name () {  return this->_name;  }
    void name (String n) {  this->_name = n; }

    void attach(int slot) {
      if (slot < LEDS_DATA_SLOTS) this->dataslot = slot;
    }

    void play() {
      this->isRunning = true;
      this->freezeTime = 0;
    }

    void pause() {
      this->freezeTime = millis();
    }

    void stop() {
      this->isRunning = false;
    }

    uint32_t time() {
      return (this->freezeTime > 0) ? this->freezeTime : millis();
    }

    bool run( int data[LEDS_DATA_SLOTS] ) {
      if (this->isRunning) {
        int value = this->modulate( data );
        if (this->dataslot >= 0) data[this->dataslot] = value;
      }
      return true;
    }

    // set one Params 
    K32_modulator* setparam(int k, int value) {
      if (k<LEDS_MODDATA_SLOTS) {
        xSemaphoreTake(this->paramInUse, portMAX_DELAY);
        this->params[k] = value;
        xSemaphoreGive(this->paramInUse);
      }
      return this;
    }

    // set all Params 
    K32_modulator* set(int* p, int size) {
      size = min(size, LEDS_MODDATA_SLOTS);
      xSemaphoreTake(this->paramInUse, portMAX_DELAY);
      for(int k=0; k<size; k++) this->params[k] = p[k];
      xSemaphoreGive(this->paramInUse);
      return this;
    }

    // Helper to set params
    K32_modulator* set(int d0) { return this->set(new int[1]{d0}, 1); }
    K32_modulator* set(int d0, int d1) { return this->set(new int[2]{d0, d1}, 2); }
    K32_modulator* set(int d0, int d1, int d2) { return this->set(new int[3]{d0, d1, d2}, 3); }
    K32_modulator* set(int d0, int d1, int d2, int d3) { return this->set(new int[4]{d0, d1, d2, d3}, 4); }
    K32_modulator* set(int d0, int d1, int d2, int d3, int d4) { return this->set(new int[5]{d0, d1, d2, d3, d4}, 5); }
    K32_modulator* set(int d0, int d1, int d2, int d3, int d4, int d5) { return this->set(new int[6]{d0, d1, d2, d3, d4, d5}, 6); }
    K32_modulator* set(int d0, int d1, int d2, int d3, int d4, int d5, int d6) { return this->set(new int[7]{d0, d1, d2, d3, d4, d5, d6}, 7); }
    K32_modulator* set(int d0, int d1, int d2, int d3, int d4, int d5, int d6, int d7) { return this->set(new int[8]{d0, d1, d2, d3, d4, d5, d6, d7}, 8); }


  protected:

    // init when anims inits
    virtual void init() { 
    }

    // modulate passed data
    virtual int modulate ( int data[LEDS_DATA_SLOTS] ) { 
      LOG("MOD: full data, doing nothing !");
      return 0;
    };
    
    int params[LEDS_MODDATA_SLOTS];
    SemaphoreHandle_t paramInUse; 
    unsigned long freezeTime = 0;
    String _name = "?"; 
    bool isRunning = false;

    int dataslot = -1;
};


#endif