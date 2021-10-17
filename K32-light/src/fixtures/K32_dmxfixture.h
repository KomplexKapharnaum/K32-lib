/*
  K32_dmxfixture.h
  Created by Thomas BOHL, october 2021.
  Released under GPL v3.0
*/
#ifndef K32_dmxfixture_h
#define K32_dmxfixture_h

#include <Arduino.h>

#include "K32_dmx.h"
#include "K32_fixture.h"
#include "_libfast/crgbw.h"


class K32_dmxfixture : public K32_fixture 
{
  public:
    K32_dmxfixture(K32_dmx* dmx, int addressStart, int channels) : K32_fixture(channels/4)
    {
      _dmxOut = dmx;
      _addressStart = addressStart;
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
    int _addressStart;
};

#endif
