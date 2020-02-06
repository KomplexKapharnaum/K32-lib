/*
  K32_leds.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_leds_rmt_h
#define K32_leds_rmt_h

#define LEDS_NUM_STRIPS 2
#define LEDS_NUM_PIXEL 108

#include "Arduino.h"
#include "K32_log.h"
#include "librmt/esp32_digital_led_lib.h"


class K32_leds_rmt {
  public:
    K32_leds_rmt();

    void show();

    K32_leds_rmt* blackout();

    K32_leds_rmt* setAll(int red, int green, int blue, int white);
    K32_leds_rmt* setAll(int red, int green, int blue);

    K32_leds_rmt* setStrip(int strip, int red, int green, int blue, int white);
    K32_leds_rmt* setStrip(int strip, int red, int green, int blue);

    K32_leds_rmt* setPixel(int strip, int pixel, int red, int green, int blue);
    K32_leds_rmt* setPixel(int strip, int pixel, int red, int green, int blue, int white);


  private:
    SemaphoreHandle_t buffer_lock;
    SemaphoreHandle_t strands_lock;
    SemaphoreHandle_t dirty;

    pixelColor_t buffer[LEDS_NUM_STRIPS][LEDS_NUM_PIXEL];
    strand_t STRANDS[LEDS_NUM_STRIPS];

    static void draw( void * parameter );
};



#endif
