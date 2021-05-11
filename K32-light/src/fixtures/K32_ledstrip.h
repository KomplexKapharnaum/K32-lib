/*
  K32_leds.h
  Created by Thomas BOHL, may 2021.
  Released under GPL v3.0
*/
#ifndef K32_leds_h
#define K32_leds_h


#include <Arduino.h>

#include "K32_fixture.h"
#include "_librmt/esp32_digital_led_lib.h"
#include "_libfast/crgbw.h"

class K32_ledstrip : public K32_fixture 
{
  public:
    K32_ledstrip(int chan, int pin, int type, int size) : K32_fixture(size)
    {
      this->_strand = digitalLeds_addStrand(
        {.rmtChannel = chan, .gpioNum = pin, .ledType = type, .brightLimit = 255, .numPixels = this->size(), .pixels = nullptr, ._stateVars = nullptr});
    }


    // COPY Buffers to STRAND
    void show() {
      // LOG("LIGHT: show in");      
      xSemaphoreTake(this->show_lock, portMAX_DELAY);
      xSemaphoreTake(this->buffer_lock, portMAX_DELAY);
      if (this->_dirty) {
        
        // LOGINL("show buffer // ");
        // for(int i=0; i < this->size(); i++) LOGF(" %i", this->_buffer[i].r);
        // LOG(this->_buffer[0].r);
        // LOG();
        
        memcpy(&this->_strand->pixels, &this->_buffer, sizeof(this->_buffer));
        // for(int i=0; i < this->size(); i++) this->_strand->pixels[i] = this->_buffer[i];
        this->_dirty = false;
        // LOG("LIGHT: show _dirty");

        // LOGINL("show strand // ");
        // for(int i=0; i < this->size(); i++) LOGF(" %i", this->_strand->pixels[i].r);
        // LOG();

        xSemaphoreGive(this->draw_lock);
      }
      else xSemaphoreGive(this->show_lock);
      xSemaphoreGive(this->buffer_lock);
      // LOG("LIGHT: show end");
    }

  protected:

    // PUSH strand TO RMT
    void draw() 
    {
      // LOGINL("draw strand // ");
      // for(int i=0; i < this->size(); i++) LOGF(" %i", this->_strand->pixels[i].r);
      // LOG();
      digitalLeds_updatePixels(this->_strand);           // PUSH LEDS TO RMT
      delay(1);
    }

  private:

    strand_t* _strand;

};

#endif
