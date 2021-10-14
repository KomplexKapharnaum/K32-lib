/*
  K32_node.h
  Created by Richard Fontaine, september 2021.
  Released under GPL v3.0
*/
#ifndef K32_node_h
#define K32_node_h

#include <Arduino.h>

#include "K32_dmx.h"
#include "K32_fixture.h"
#include "_librmt/esp32_digital_led_lib.h"
#include "_libfast/crgbw.h"

#define NODE_PATCHSIZE 512   // Must be multiple of 4 to map to CRGBW color

class K32_node : public K32_fixture 
{
  public:
    K32_node(const int DMX_PIN[3]) : K32_fixture(NODE_PATCHSIZE/4)
    {
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
        this->_dmxOut->setMultiple(buffDMX, size()*4);
        /////////////////////////////////////////////////////////////////////////////////

        this->_dirty = false;
        xSemaphoreGive(this->draw_lock);
      }
      else xSemaphoreGive(this->show_lock);
      xSemaphoreGive(this->buffer_lock);
    }


  private:
    K32_dmx* _dmxOut = nullptr;

};

#endif
