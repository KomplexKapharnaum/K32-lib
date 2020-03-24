/*
  K32_pwm.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_pwm_h
#define K32_pwm_h

#define PWM_MAXCHANNELS 16
#define PWM_FREQUENCY 40000
#define PWM_RESOLUTION 8

#include "Arduino.h"
#include "system/K32_log.h"

class K32_pwm {
  public:
    K32_pwm();
    void attach(const int PIN);

    K32_pwm* blackout();
    K32_pwm* setAll(int value);
    K32_pwm* set(int channel, int value);
    int get(int channel);

  private:
    int chanState[PWM_MAXCHANNELS];
    byte chanNumber = 0;

};



#endif
