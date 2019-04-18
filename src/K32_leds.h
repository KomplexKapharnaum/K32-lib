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
#include "K32_log.h"
#include "librmt/esp32_digital_led_lib.h"


class K32_leds {
  public:
    K32_leds();

    void show();

    K32_leds* blackout();

    K32_leds* setAll(int red, int green, int blue, int white);
    K32_leds* setAll(int red, int green, int blue);

    K32_leds* setStrip(int strip, int red, int green, int blue, int white);
    K32_leds* setStrip(int strip, int red, int green, int blue);

    K32_leds* setPixel(int strip, int pixel, int red, int green, int blue);
    K32_leds* setPixel(int strip, int pixel, int red, int green, int blue, int white);

    void play( bool (*fn)( K32_leds* leds ) );
    void stop();


  private:
    SemaphoreHandle_t buffer_lock;
    SemaphoreHandle_t strands_lock;
    SemaphoreHandle_t dirty;

    pixelColor_t buffer[LEDS_NUM_STRIPS][LEDS_NUM_PIXEL];
    strand_t STRANDS[LEDS_NUM_STRIPS];

    TaskHandle_t animateHandle = NULL;
    bool (*anim)( K32_leds* that );

    static void update( void * parameter );
    static void animate( void * parameter );

};





#endif
