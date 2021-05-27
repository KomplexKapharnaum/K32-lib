/*
  K32_elp.h
  Created by Thomas BOHL, may 2021.
  Released under GPL v3.0
*/
#ifndef K32_elp_h
#define K32_elp_h

#define FIXTURE_MAXPIXEL 512

#include <Arduino.h>

#include "K32_dmx.h"
#include "K32_fixture.h"
#include "_librmt/esp32_digital_led_lib.h"
#include "_libfast/crgbw.h"


class K32_elp : public K32_fixture 
{
  public:
    K32_elp(const int DMX_PIN[3], int addressStart, int size) : K32_fixture(size)
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
        int buffDMX[size()*3];
        for (int i = 0; i < size(); i++) {
          buffDMX[i*3]    = _buffer[i].r;
          buffDMX[i*3+1]  = _buffer[i].g;
          buffDMX[i*3+2]  = _buffer[i].b;
        }   
        this->_dmxOut->setMultiple(buffDMX, size()*3, _addressStart);
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
