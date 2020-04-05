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

    using K32_modulator::K32_modulator;

    int& period = params[0];
    int& mini    = params[1];
    int& maxi    = params[2];
    int& phase  = params[3];

    // Modulate Data
    int modulate (int data[LEDS_DATA_SLOTS])
    {
      return ((0.5f + 0.5f * sin(2 * PI * (this->time()%period) / period - 0.5f * PI + (phase * PI / 180.0) )) * (maxi - mini) + mini);
    };
  
};

//
// RANDOM
//
class K32_mod_random : public K32_modulator {
  public:  

    using K32_modulator::K32_modulator;

    int& period     = params[0];
    int& mini       = params[1];
    int& maxi       = params[2];

    int lastPeriod;
    int lastValue;

    // Modulate Data
    int modulate (int data[LEDS_DATA_SLOTS])
    {
      int newPeriod = this->time() / max(1, period);
      if (newPeriod != lastPeriod) {
        lastPeriod = newPeriod;
        lastValue = random(mini, maxi);
      }
      return lastValue;
    };
  
};




#endif
