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
#define LEDS_FPS 33

#include "Arduino.h"
#include "SmartLeds.h"  // https://github.com/RoboticsBrno/SmartLeds
#include "K32_log.h"


class K32_leds {
  public:
    K32_leds();

    void show();

    void test();
    void blackout();
    void setAll(int red, int green, int blue);
    void setStrip(int strip, int red, int green, int blue);
    void setPixel(int strip, int pixel, int red, int green, int blue);


  private:
    SemaphoreHandle_t buffer_lock;
    SemaphoreHandle_t strands_lock;
    SemaphoreHandle_t dirty;
    static void task( void * parameter );

    Rgb buffer[LEDS_NUM_STRIPS][LEDS_NUM_PIXEL];
    SmartLed* strands[LEDS_NUM_STRIPS];


};


#endif
