/*
  K32_anim.h
  Created by Thomas BOHL, March 2020.
  Released under GPL v3.0
*/
#ifndef K32_anim_h
#define K32_anim_h

#define ANIM_DATA_SLOTS  32
#define ANIM_MOD_SLOTS  16

#include "K32_ledstrip.h"
#include "K32_modulator.h"

//
// BASE ANIM
//
class K32_anim {
  public:
    K32_anim()
    {
      this->_strip = NULL;
      memset(this->_dataBuffer, 0, sizeof this->_dataBuffer);

      this->newData = xSemaphoreCreateBinary();
      this->bufferInUse = xSemaphoreCreateBinary();
      xSemaphoreTake(this->newData, 1);
      xSemaphoreGive(this->bufferInUse);

      this->wait_lock = xSemaphoreCreateBinary();
      xSemaphoreGive(this->wait_lock);
    }

    K32_anim* setup(K32_ledstrip* strip, int size = 0, int offset = 0) {
      this->_strip = strip;
      this->_size = (size) ? size : strip->size();
      this->_offset = offset;
      return this;
    }
    
    // ANIM CONTROLS
    //

    // get/set name
    String name () {  return this->_name;  }
    void name (String n) {  this->_name = n; }

    int size() {
      return this->_size;
    }

    // loop get
    bool loop() { 
      return this->_loop;
    }

    // loop set
    K32_anim* loop(bool doLoop) { 
      this->_loop = doLoop;
      return this;
    }

    // is playing ?
    bool isPlaying() {
      return this->animateHandle != NULL;
    }

    // start animate thread
    K32_anim* play(int timeout = 0) 
    {
      if (this->_strip == NULL) {
        LOG("ERROR: animation, you need to call setup before play !");
        return this;
      }
      this->_stopAt = (timeout>0) ? millis() + timeout : 0;

      if (this->isPlaying()) return this;

      xSemaphoreTake(this->wait_lock, portMAX_DELAY);
      xSemaphoreTake(this->bufferInUse, portMAX_DELAY);
      xTaskCreate( this->animate,           // function
                    "anim_task",            // task name
                    5000,                   // stack memory
                    (void*)this,            // args
                    3,                      // priority
                    &this->animateHandle ); // handler
      LOGF("ANIM: %s play \n", this->name().c_str() );
      
      return this;
    }

    // stop animation
    void stop() { 
      this->_stopAt = 1; 
    }

    // wait until anim end (or timeout)
    bool wait(int timeout = 0) 
    {
      TickType_t xTicksToWait = portMAX_DELAY;
      if (timeout > 0) xTicksToWait = pdMS_TO_TICKS(timeout);

      if ( xSemaphoreTake(this->wait_lock, xTicksToWait) == pdTRUE) {
        xSemaphoreGive(this->wait_lock);
        return true;
      }
      return false;
    }

    // register new modulator
    K32_modulator* mod( String modName, K32_modulator* mod, bool playNow = true) 
    {
      if (this->_modcounter >= ANIM_MOD_SLOTS) {
        LOG("ERROR: no more slot available to register new modulator");
        return mod;
      }
      
      mod->name(modName);

      this->_modulators[ this->_modcounter ] = mod;
      this->_modcounter++;
      // LOGINL("ANIM: register "); LOG(mod->name());
      
      if (playNow) mod->play();

      return mod;
    }

    // get registered modulator
    K32_modulator* mod( String modName) 
    {
      for (int k=0; k<this->_modcounter; k++)
        if (this->_modulators[k]->name() == modName) {
          // LOGINL("LIGHT: "); LOG(name);
          return this->_modulators[k];
        }
      LOGINL("MOD: not found "); LOG(modName);
      return new K32_modulator();
    }


    // ANIM DATA
    //
    K32_anim* master(uint8_t m) {
      this->_master = m;
      return this;
    }


    // change one element in data
    K32_anim* set(int k, int value) { 
      xSemaphoreTake(this->bufferInUse, portMAX_DELAY);     // data can be modified only during anim waitData
      if (k < ANIM_DATA_SLOTS) this->_dataBuffer[k] = value; 
      xSemaphoreGive(this->bufferInUse);
      return this;
    }

    // new data push (int[], size)
    K32_anim* push(int* frame, int size) {
      bool didChange = false;
      size = min(size, ANIM_DATA_SLOTS);
      xSemaphoreTake(this->bufferInUse, portMAX_DELAY);     // data can be modified only during anim waitData
      for(int k=0; k<size; k++) 
        if (this->_dataBuffer[k] != frame[k]) {
          this->_dataBuffer[k] = frame[k]; 
          didChange = true;
        }
      xSemaphoreGive(this->bufferInUse);
      if (didChange) xSemaphoreGive(this->newData);
      return this;
    }
    
    // new data push (uint8_t[], size)
    K32_anim* push(uint8_t* frame, int size) {
      int dframe[size];
      for(int i=0; i<size; i++) dframe[i] = frame[i];
      return this->push(dframe, size);
    }

    // refresh data 
    K32_anim* push() {
      xSemaphoreGive(this->newData);
      return this;
    }

    // data push simplified
    K32_anim* push(int d0) { return this->push(new int[1]{d0}, 1); }
    K32_anim* push(int d0, int d1) { return this->push(new int[2]{d0, d1}, 2); }
    K32_anim* push(int d0, int d1, int d2) { return this->push(new int[3]{d0, d1, d2}, 3); }
    K32_anim* push(int d0, int d1, int d2, int d3) { return this->push(new int[4]{d0, d1, d2, d3}, 4); }
    K32_anim* push(int d0, int d1, int d2, int d3, int d4) { return this->push(new int[5]{d0, d1, d2, d3, d4}, 5); }
    K32_anim* push(int d0, int d1, int d2, int d3, int d4, int d5) { return this->push(new int[6]{d0, d1, d2, d3, d4, d5}, 6); }
    K32_anim* push(int d0, int d1, int d2, int d3, int d4, int d5, int d6) { return this->push(new int[7]{d0, d1, d2, d3, d4, d5, d6}, 7); }
    K32_anim* push(int d0, int d1, int d2, int d3, int d4, int d5, int d6, int d7) { return this->push(new int[8]{d0, d1, d2, d3, d4, d5, d6, d7}, 8); }



  // PROTECTED
  //
  protected:

    // ANIM LOGIC
    //

    // init called when anim starts to play (can be overloaded by anim class)
    // this is a prototype, must be defined in specific anim class
    virtual void init() {}

    // generate frame from data, called by update
    // this is a prototype, must be defined in specific anim class
    virtual void draw () { LOG("ANIM: nothing to do.."); };

    // frozen data: can be accessed in draw, do net set anything in it !
    int data[ANIM_DATA_SLOTS];

    // DRAW ON STRIP
    //

    // draw pix
    void pixel(int pix, CRGBW color)  {
      if (pix < this->_size)
        this->_strip->pix( pix + this->_offset, color % this->_master);
    }

    // draw multiple pix
    void pixel(int pixStart, int count, CRGBW color)  {
      this->_strip->pix( pixStart + this->_offset, count, color % this->_master);
    }

    // draw all
    void all(CRGBW color) {
      this->_strip->pix( this->_offset, this->_size, color % this->_master);
    }

    // clear
    void clear() {
      this->all({0,0,0,0});
    }
    
    unsigned long startTime = 0;


  // PRIVATE
  //
  private:

    // input data
    int _dataBuffer[ANIM_DATA_SLOTS];

    // THREAD: draw frame on new data
    static void animate( void * parameter ) 
    {
      K32_anim* that = (K32_anim*) parameter;
      TickType_t timeout;
      
      that->startTime = millis();

      that->init();                                                                 // Subclass init hook

      do 
      {
        xSemaphoreGive(that->bufferInUse);                                          // allow push & pushdata to modify data
        yield();                                                                          
        timeout = pdMS_TO_TICKS( 1000/LEDS_ANIMATE_FPS );                             // push timeout according to FPS
        
        if (xSemaphoreTake(that->newData, timeout) == pdTRUE) {                     // Wait external DATA refresh or FPS timeout
          xSemaphoreGive(that->newData);
          that->_firstDataReceived = true;
        }

        if (!that->_firstDataReceived) continue;                                    // check if data has been set at least one time 
        if (that->_stopAt && millis() >= that->_stopAt) break;                      // check if we should stop now

        xSemaphoreTake(that->bufferInUse, portMAX_DELAY);                           // lock buffer to prevent external change
        
        for (int k=0; k<that->_modcounter; k++)       
          if (that->_modulators[k]->run(that->_dataBuffer))                               // run modulators on data buffer
            xSemaphoreGive(that->newData);;                                         // trigger new data if data has been changed 

        if (xSemaphoreTake(that->newData, 0) == pdTRUE)                             // new data available in buffer
        {
          memcpy(that->data, that->_dataBuffer, ANIM_DATA_SLOTS*sizeof(int));       // copy buffer into data
          xSemaphoreTake(that->newData, 1);                                         // consume newdata
          xSemaphoreGive(that->bufferInUse);                                        // let data be push by others

          that->draw();                                                             // Subclass draw hook
        }

      } 
      while(that->loop());

      
      // stop and clear
      xSemaphoreTake(that->newData, 1);
      xSemaphoreGive(that->bufferInUse);

      that->clear();
      that->animateHandle = NULL;
  
      xSemaphoreGive(that->wait_lock);

      LOGF("ANIM: %s end \n", that->name().c_str());
      vTaskDelete(NULL);
    }
    
    // identity
    String _name = "?"; 
    bool _loop = true;

    // output
    K32_ledstrip* _strip = NULL;
    uint8_t _master = 255;
    int _size = 0;
    int _offset = 0; 

    // internal logic
    SemaphoreHandle_t newData;  
    SemaphoreHandle_t bufferInUse;  
    bool _firstDataReceived = false;

    // Animation
    TaskHandle_t animateHandle = NULL;
    SemaphoreHandle_t wait_lock;
    uint32_t _stopAt = 0;

    // Modulator
    K32_modulator* _modulators[ANIM_MOD_SLOTS];
    int _modcounter = 0;
};



#endif