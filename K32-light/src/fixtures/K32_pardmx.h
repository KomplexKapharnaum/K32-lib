/*
  K32_pardmx.h
  Created by Thomas BOHL, may 2021.
  Released under GPL v3.0
*/
#ifndef K32_pardmx_h
#define K32_pardmx_h

#include <Arduino.h>

#include "K32_dmx.h"
#include "K32_fixture.h"
#include "_librmt/esp32_digital_led_lib.h"
#include "_libfast/crgbw.h"

#define PAR_PATCHSIZE 8   // Must be multiple of 4 to map to CRGBW color

class K32_pardmx : public K32_fixture 
{
  public:
    K32_pardmx(const int DMX_PIN[3], int addressStart) : K32_fixture(PAR_PATCHSIZE/4)
    {
      // ADDR offset
      _addressStart = max(1,addressStart);
      _dmxOut = new K32_dmx(DMX_PIN, DMX_OUT); // TODO make DMX device external (not in fixture so multiple fixture can use a single DMX out)
    }

    // COPY Buffers to STRAND
    void show() {
      xSemaphoreTake(this->show_lock, portMAX_DELAY);
      xSemaphoreTake(this->buffer_lock, portMAX_DELAY);
      if (this->_dirty) 
      {
        // LOGINL("show buffer // ");
        // // for(int i=0; i < this->size(); i++) LOGF(" %i", this->_buffer[i].r);
        // LOG(this->_buffer[0].r);

        /////////////////////////////////////////////////////////////////////////////////
        int buffDMX[size()*4];
        for (int i = 0; i < size(); i++) {
          buffDMX[i*4]    = _buffer[i].r;
          buffDMX[i*4+1]  = _buffer[i].g;
          buffDMX[i*4+2]  = _buffer[i].b;
          buffDMX[i*4+3]  = _buffer[i].w;
        }   
        this->_dmxOut->setMultiple(buffDMX, size()*4, _addressStart);
        /////////////////////////////////////////////////////////////////////////////////

        this->_dirty = false;
        xSemaphoreGive(this->draw_lock);
      }
      else xSemaphoreGive(this->show_lock);
      xSemaphoreGive(this->buffer_lock);
    }


  private:
    K32_dmx* _dmxOut = nullptr;
    int _addressStart = 1;

};

#endif
