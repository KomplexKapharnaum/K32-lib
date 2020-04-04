/*
  K32_mod_basics.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_mod_basics_h
#define K32_mod_basics_h

#include "../K32_anim.h"

//
// NOTE: to be available, add #include to this file in K32_light.h !
//


//
// SINUS
//


class K32_mod_sinus : public K32_modulator {
  public:  

    K32_mod_sinus(int period, int min, int max) 
    {
      this->params[0] = period; //period
      this->params[1] = max;    //value max
      this->params[2] = min;    //value min
    }

    // Modulate Data
    int modulate (int data[LEDS_DATA_SLOTS])
    {
      return ((0.5f + 0.5f * sin(2 * PI * this->time() / this->params[0] - 0.5f * PI)) * (this->params[1] - this->params[2]) + this->params[2]);
    };

};


#endif
