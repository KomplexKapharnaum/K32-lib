/*
  K32_leds.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_leds_h
#define K32_leds_h

#define LEDS_TEST_LEVEL 100
#define LEDS_NUM_STRIPS 2
#define LEDS_NUM_PIXEL 512
#define LEDS_FPS 30

#include "Arduino.h"
#include "librmt/esp32_digital_led_lib.h"


class K32_leds {
  public:
    K32_leds();

    void show();

    void blackout();
    void setAll(int red, int green, int blue);
    void setStrip(int strip, int red, int green, int blue);
    void setPixel(int strip, int pixel, int red, int green, int blue);


  private:
    strand_t* strands[LEDS_NUM_STRIPS];
    bool running;
    int refresh;

};

#endif
