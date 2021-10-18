/*
  K32_pwmfixture.h
  Created by Thomas BOHL, october 2021.
  Released under GPL v3.0
*/
#ifndef K32_pwmfixture_h
#define K32_pwmfixture_h

#include <Arduino.h>

#include <hardware/K32_pwm.h>
#include "K32_fixture.h"
#include "K32_version.h"
#include "_libfast/crgbw.h"


class K32_pwmfixture : public K32_fixture 
{
  public:
    K32_pwmfixture(K32_pwm* pwm) : K32_fixture( ceil(PWM_N_CHAN/4.0) )
    {
      _pwmOut = pwm;
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
        for (int i = 0; i < size(); i++) {
          this->_pwmOut->set(i*4,   _buffer[i].r);
          this->_pwmOut->set(i*4+1, _buffer[i].g);
          this->_pwmOut->set(i*4+2, _buffer[i].b);
          this->_pwmOut->set(i*4+3, _buffer[i].w);
        }   
        /////////////////////////////////////////////////////////////////////////////////

        this->_dirty = false;
        xSemaphoreGive(this->draw_lock);
      }
      else xSemaphoreGive(this->show_lock);
      xSemaphoreGive(this->buffer_lock);
    }


  private:
    K32_pwm* _pwmOut = nullptr;
};

#endif
