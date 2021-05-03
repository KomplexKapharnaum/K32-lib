/*
  K32_pwm.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_pwm_h
#define K32_pwm_h

#define PWM_MAXCHANNELS 16
#define PWM_FREQUENCY 60000 //40000
#define PWM_RESOLUTION 16 //8

#include "core/K32_plugin.h"

class K32_pwm : K32_plugin {
  public:
    K32_pwm(K32* k32);
    void attach(const int PIN);

    K32_pwm* blackout();
    K32_pwm* setAll(int value);
    K32_pwm* set(int channel, int value);
    int get(int channel);

    void command(Orderz* order);

  private:
    int chanState[PWM_MAXCHANNELS];
    byte chanNumber = 0;

};



#endif
