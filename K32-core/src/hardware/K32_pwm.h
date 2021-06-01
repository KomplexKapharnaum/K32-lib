/*
  K32_pwm.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_pwm_h
#define K32_pwm_h

#define PWM_MAXCHANNELS 16
#define PWM_FREQUENCY 60000 //40000 60000 table lumi
#define PWM_RESOLUTION 16 //8

#include "class/K32_plugin.h"

class K32_pwm : K32_plugin {
  public:
  
    K32_pwm(K32* k32) : K32_plugin("pwm", k32)
    {
      for(int k=0; k<PWM_MAXCHANNELS; k++) this->chanState[k] = 0;
    };


    void attach(const int PIN)
    { 
      if (this->chanNumber >= PWM_MAXCHANNELS) return;
      ledcSetup(this->chanNumber, PWM_FREQUENCY, PWM_RESOLUTION);
      ledcAttachPin(PIN, this->chanNumber);
      ledcWrite(this->chanNumber, 0);
      this->chanNumber += 1;
    }


    K32_pwm* blackout() 
    {
      this->setAll(0);
      return this;
    }


    K32_pwm* setAll(int value) 
    {
      for(int k=0; k<this->chanNumber; k++) this->set(k, value);
      return this;
    }


    K32_pwm* set(int channel, int value)
    { 
      if (channel < this->chanNumber) 
        if (this->chanState[channel] != value)
        {
          this->chanState[channel] = ((value)*256);// value*value/255;
          ledcWrite(channel, this->chanState[channel]);
        }
      return this;
    }


    int get(int channel)
    { 
      return this->chanState[channel];
    }


    void command(Orderz* order)
    {
      // BLACK
      if (strcmp(order->action, "blackout") == 0)  this->blackout();

      // ALL
      else if (strcmp(order->action, "all") == 0)  {
        if (order->count() == 1) 
          this->setAll(order->getData(0)->toInt());
      }

      // SET
      else if (strcmp(order->action, "set") == 0) {
        if (order->count() == 2) 
          this->set( order->getData(0)->toInt(), order->getData(1)->toInt() );
      }
    }

  private:
    int chanState[PWM_MAXCHANNELS];
    byte chanNumber = 0;

};



#endif
