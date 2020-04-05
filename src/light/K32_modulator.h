/*
  K32_modulator.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_modulator_h
#define K32_modulator_h

#define MOD_PARAMS_SLOTS  8

#include "K32_anim.h"

//
// BASE ANIM
//
class K32_modulator {
  public:
    K32_modulator() {  this->init(NULL, 0); }

    // Helper to set params
    K32_modulator(int d0) { this->init(new int[1]{d0}, 1); }
    K32_modulator(int d0, int d1) { this->init(new int[2]{d0, d1}, 2); }
    K32_modulator(int d0, int d1, int d2) { this->init(new int[3]{d0, d1, d2}, 3); }
    K32_modulator(int d0, int d1, int d2, int d3) { this->init(new int[4]{d0, d1, d2, d3}, 4); }
    K32_modulator(int d0, int d1, int d2, int d3, int d4) { this->init(new int[5]{d0, d1, d2, d3, d4}, 5); }
    K32_modulator(int d0, int d1, int d2, int d3, int d4, int d5) { this->init(new int[6]{d0, d1, d2, d3, d4, d5}, 6); }
    K32_modulator(int d0, int d1, int d2, int d3, int d4, int d5, int d6) { this->init(new int[7]{d0, d1, d2, d3, d4, d5, d6}, 7); }
    K32_modulator(int d0, int d1, int d2, int d3, int d4, int d5, int d6, int d7) { this->init(new int[8]{d0, d1, d2, d3, d4, d5, d6, d7}, 8); }

    
    // get/set name
    String name () {  return this->_name;  }
    void name (String n) {  this->_name = n; }

    void attach(int slot) {
      if (slot < LEDS_DATA_SLOTS) this->dataslot = slot;
    }

    void play() {
      this->isRunning = true;
      this->freezeTime = 0;
      LOGF2("ANIM: %s modulate param %i \n", this->name(), this->dataslot);
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
        xSemaphoreTake(this->paramInUse, portMAX_DELAY);
        int value = this->modulate( data );
        xSemaphoreGive(this->paramInUse);
        if (this->dataslot >= 0) data[this->dataslot] = value;
      }
      return true;
    }

    // change one Params 
    K32_modulator* param(int k, int value) {
      if (k<MOD_PARAMS_SLOTS) {
        xSemaphoreTake(this->paramInUse, portMAX_DELAY);
        this->params[k] = value;
        xSemaphoreGive(this->paramInUse);
      }
      return this;
    }

  protected:

    // modulate passed data
    virtual int modulate ( int data[LEDS_DATA_SLOTS] ) { 
      LOG("MOD: full data, doing nothing !");
      return 0;
    };
    
    int params[MOD_PARAMS_SLOTS];

  private:

    // init on construcion
    void init(int* p, int size) 
    { 
      this->paramInUse = xSemaphoreCreateBinary();
      size = min(size, MOD_PARAMS_SLOTS);
      for(int k=0; k<size; k++) this->params[k] = p[k];
      xSemaphoreGive(this->paramInUse);
    }

    SemaphoreHandle_t paramInUse; 
    unsigned long freezeTime = 0;
    String _name = "?"; 
    bool isRunning = false;

    int dataslot = -1;
};


#endif