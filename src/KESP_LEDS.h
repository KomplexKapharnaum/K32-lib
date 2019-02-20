/*
  KESP_LEDS.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef KESP_LEDS_h
#define KESP_LEDS_h

#define LEDS_NUM_STRIPS 2
#define LEDS_NUM_LEDS_PER_STRIP 512          // Note: 1024 leds = 30fps max
#define LEDS_TEST_LEVEL 100

#include "Arduino.h"

class KESP_LEDS {
  public:
    KESP_LEDS();

  private:

};

#endif
