/*
  K32_leds.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_leds_h
#define K32_leds_h

#define LEDS_MAXSTRIPS 2
#define LEDS_MAXPIXEL 180

#include "Arduino.h"
#include "K32_log.h"
#include "librmt/esp32_digital_led_lib.h"


class K32_leds {
  public:
    K32_leds();
    void attach(int PIN, int NPIXEL, led_types LEDTYPE);
    void start();

    void show();

    K32_leds* blackout();

    K32_leds* setAll(int red, int green, int blue, int white);
    K32_leds* setAll(int red, int green, int blue);

    K32_leds* setStrip(int strip, int red, int green, int blue, int white);
    K32_leds* setStrip(int strip, int red, int green, int blue);

    K32_leds* setPixel(int strip, int pixel, int red, int green, int blue);
    K32_leds* setPixel(int strip, int pixel, int red, int green, int blue, int white);


  private:
    SemaphoreHandle_t buffer_lock;
    SemaphoreHandle_t strands_lock;
    SemaphoreHandle_t dirty;

    pixelColor_t buffer[LEDS_MAXSTRIPS][LEDS_MAXPIXEL];
    strand_t STRANDS[LEDS_MAXSTRIPS];

    static int LEDS_NSTRIPS;
    int LEDS_NPIXEL[LEDS_MAXSTRIPS];

    static void draw( void * parameter );
};



#endif
