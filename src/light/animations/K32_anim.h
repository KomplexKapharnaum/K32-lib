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
// NOTE: to be able to load an generator by name, it must be registered in K32_animbook
//

//
// BASE ANIM
//
class K32_anim {
  public:
    K32_anim(String name) {
      this->_name = name;
      this->newData = xSemaphoreCreateBinary();
      this->dataInUse = xSemaphoreCreateBinary();
      xSemaphoreGive(this->dataInUse);
    }
    
    // get/set name
    String name () { return this->_name; };
    void name (String n) { this->_name = n; };

    // init called when anim starts to play (can be overloaded by anim class)
    // waiting for initdata ensure that nothing is really played before first data are seted
    // setting startTime can be usefull..
    virtual void init() { 
      this->waitData();
      this->refresh();
      this->startTime = millis();
    }


    // gener8 called by dedicated xtask
    // create output image based on data and fill strip
    // this is a prototype, must be defined in specific anim class
    virtual void gener8 ( K32_ledstrip* strip ) { 
      LOG("ANIM: image generated !");
    };


    // modul8 called by dedicated xtask
    // execute all registered modulators to change data (external data) and idata (internal data)
    void modul8 () { 
      // LOG("ANIM: executing modulators !");
    };


    // signal that data has been updated -> unblock waitData
    K32_anim* refresh() {
      xSemaphoreGive(this->newData);
      return this;
    }

    // flush newData flag: cancel programmed refresh
    K32_anim* flush() {
      xSemaphoreTake(this->newData, 1);  // lock it, if not already token
      return this;
    }

    // block until timeout or refresh is called
    bool waitData(int timeout = 0) {
      xSemaphoreGive(this->dataInUse);                                                  // allow set & set to modify data
      yield();                                                                          
      TickType_t xTicksToWait = portMAX_DELAY;                                          
      if (timeout > 0) xTicksToWait = pdMS_TO_TICKS(timeout);                          // set waitData timeout
      bool newDataDetected = xSemaphoreTake(this->newData, xTicksToWait) == pdTRUE;     // try to consume newData until timeout
      xSemaphoreTake(this->dataInUse, portMAX_DELAY);                                   // lock data to prevent new change until next waitData
      
      if (!newDataDetected && xSemaphoreTake(this->newData, 0) == pdTRUE) {             // Poll again to check if newData arrived during dataInUse locking
        this->flush();                                                                  // If new Data: lock it !
        newDataDetected = true;
      }
      return newDataDetected;
    }

    // change one element in data
    K32_anim* setdata(int k, int value) { 
      xSemaphoreTake(this->dataInUse, portMAX_DELAY);     // data can be modified only during anim waitData
      if (k < LEDS_DATA_SLOTS) this->data[k] = value; 
      xSemaphoreGive(this->dataInUse);
      return this;
    }

    // new data set 
    K32_anim* set(int* frame, int size) {
      size = min(size, LEDS_DATA_SLOTS);
      xSemaphoreTake(this->dataInUse, portMAX_DELAY);     // data can be modified only during anim waitData
      for(int k=0; k<size; k++) this->data[k] = frame[k]; 
      xSemaphoreGive(this->dataInUse);
      return this;
    }
    
    // new data set 
    K32_anim* set(uint8_t* frame, int size) {
      size = min(size, LEDS_DATA_SLOTS);
      xSemaphoreTake(this->dataInUse, portMAX_DELAY);     // data can be modified only during anim waitData
      for(int k=0; k<size; k++) this->data[k] = frame[k]; 
      xSemaphoreGive(this->dataInUse);
      return this;
    } 

    // Helper to set and refresh various amount of data
    K32_anim* set() { return this->refresh(); }
    K32_anim* set(int d0) { return this->set(new int[1]{d0}, 1); }
    K32_anim* set(int d0, int d1) { return this->set(new int[2]{d0, d1}, 2); }
    K32_anim* set(int d0, int d1, int d2) { return this->set(new int[3]{d0, d1, d2}, 3); }
    K32_anim* set(int d0, int d1, int d2, int d3) { return this->set(new int[4]{d0, d1, d2, d3}, 4); }
    K32_anim* set(int d0, int d1, int d2, int d3, int d4) { return this->set(new int[5]{d0, d1, d2, d3, d4}, 5); }
    K32_anim* set(int d0, int d1, int d2, int d3, int d4, int d5) { return this->set(new int[6]{d0, d1, d2, d3, d4, d5}, 6); }
    K32_anim* set(int d0, int d1, int d2, int d3, int d4, int d5, int d6) { return this->set(new int[7]{d0, d1, d2, d3, d4, d5, d6}, 7); }
    K32_anim* set(int d0, int d1, int d2, int d3, int d4, int d5, int d6, int d7) { return this->set(new int[8]{d0, d1, d2, d3, d4, d5, d6, d7}, 8); }

    // loop get
    bool loop() { return _loop;}

    // loop set
    K32_anim* loop(bool doLoop) { 
      _loop = doLoop;
      return this;
    }

  protected:

    void delay(int ms) { vTaskDelay(pdMS_TO_TICKS(ms)); }
    
    int data[LEDS_DATA_SLOTS];
    SemaphoreHandle_t newData;  
    SemaphoreHandle_t dataInUse;  
    unsigned long startTime = 0;
    String _name = "?"; 
    bool _loop = true; 
};


//
// ANIMATOR BOOK
//

#define LEDS_ANIMS_SLOTS  16

#include "K32_anim_basics.h"
#include "K32_anim_charge.h"
#include "K32_anim_dmx.h"

//
// NOTE: to be able to load a generator by name, it must be registered in K32_animbook
//

class K32_animbook {
  public:
    K32_animbook() {

      //
      // REGISTER AVAILABLE ANIMS HERE !
      //
      this->add( new K32_anim_test() );
      this->add( new K32_anim_color() );
      // this->add( new K32_anim_strobe() );
      // this->add( new K32_anim_hardstrobe() );
      // this->add( new K32_anim_chaser() );
      this->add( new K32_anim_discharge() );
      this->add( new K32_anim_charge() );
      this->add( new K32_anim_dmx() );

    }

    K32_anim* get( String name ) {
      for (int k=0; k<this->counter; k++)
        if (this->anims[k]->name() == name) {
          // LOGINL("LEDS: "); LOG(name);
          return this->anims[k];
        }
      LOGINL("ANIM: not found "); LOG(name);
      return new K32_anim("dummy");
    }

    void add(K32_anim* anim) {
      if (this->counter >= LEDS_ANIMS_SLOTS) {
        LOG("ERROR: no more slot available to register new animation");
        return;
      }
      this->anims[ this->counter ] = anim;
      this->counter++;
      // LOGINL("ANIM: register "); LOG(anim->name());
    };

  private:
    K32_anim* anims[LEDS_ANIMS_SLOTS];
    int counter = 0;


};


#endif