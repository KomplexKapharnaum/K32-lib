/*
  K32_light.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_light_h
#define K32_light_h

#define LEDS_MAXSTRIPS 8    // There is 8 RMT channels on ESP32


#include "Arduino.h"
#include "K32_ledstrip.h"
#include "generators/K32_gen.h"

class K32_light {
  public:
    K32_light();

    void addStrip(const int pin, led_types type, int size = 0);

    K32_ledstrip* strip(int s);
    K32_light* strips();

    K32_light* black();
    K32_light* all(pixelColor_t color);
    K32_light* all(int red, int green, int blue, int white = 0);
    K32_light* pix(int pixel, pixelColor_t color);
    K32_light* pix(int pixel, int red, int green, int blue, int white = 0);

    void show();

    K32_gen* anim( String animName = "");

    K32_gen* play( K32_gen* anim );
    K32_gen* play( String animName );
    void stop();
    bool wait(int timeout = 0);
    void blackout();

    K32_gen* getActiveAnim();
    bool isPlaying();

  private:

    static int _nstrips;
    K32_ledstrip* _strips[LEDS_MAXSTRIPS];

    K32_genbook* _book;

    TaskHandle_t animateHandle = NULL;
    K32_gen* activeAnim;
    SemaphoreHandle_t stop_lock;
    SemaphoreHandle_t wait_lock;

    static void animate( void * parameter );
    static void async_stop( void * parameter );
};



#endif
