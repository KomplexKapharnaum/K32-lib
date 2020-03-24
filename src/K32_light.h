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
#include "K32_light_anims.h"

class K32_light {
  public:
    K32_light();

    void addStrip(const int pin, led_types type, int size = 0);
    void start();

    K32_ledstrip* strip(int s);
    K32_light* strips();

    K32_light* black();
    K32_light* all(pixelColor_t color);
    K32_light* all(int red, int green, int blue, int white = 0);
    K32_light* pix(int pixel, pixelColor_t color);
    K32_light* pix(int pixel, int red, int green, int blue, int white = 0);

    void show();

    K32_light_anim* anim( String animName);

    void play( K32_light_anim* anim );
    void play( String animName );
    void stop();
    void blackout();

    K32_light_anim* getActiveAnim();
    bool isPlaying();

  private:

    static int _nstrips;
    K32_ledstrip* _strips[LEDS_MAXSTRIPS];

    K32_light_animbook* _book;

    TaskHandle_t animateHandle = NULL;
    K32_light_anim* activeAnim;
    SemaphoreHandle_t stop_lock;

    static void animate( void * parameter );
    static void async_stop( void * parameter );
};



#endif
