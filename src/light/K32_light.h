/*
  K32_light.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_light_h
#define K32_light_h

#define LEDS_MAXSTRIPS 8    // There is 8 RMT channels on ESP32
#define LEDS_ANIM8_FPS 25   // Animator minimum refresh rate (Frames per second)


#include "Arduino.h"
#include "K32_ledstrip.h"
#include "animations/K32_anim.h"

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

    K32_anim* anim( String animName = "");

    K32_anim* load( K32_anim* anim );
    K32_anim* load( String animName );
    
    K32_light* play();
    K32_light* play(K32_anim* anim);

    K32_light* play(int timeout);
    K32_light* play(K32_anim* anim, int timeout);

    void stop();
    bool wait(int timeout = 0);
    void blackout();

    K32_anim* getActiveAnim();
    bool isPlaying();

    void fps(int f = -1);

  private:

    int _fps = LEDS_ANIM8_FPS;
    unsigned long _stopAt = 0;

    static int _nstrips;
    K32_ledstrip* _strips[LEDS_MAXSTRIPS];

    K32_animbook* _book;

    TaskHandle_t animateHandle = NULL;
    K32_anim* activeAnim;
    SemaphoreHandle_t stop_lock;
    SemaphoreHandle_t wait_lock;

    static void animate( void * parameter );
    static void async_stop( void * parameter );
};



#endif
