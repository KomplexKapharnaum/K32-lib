/*
  K32_light.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_light_h
#define K32_light_h


#include "Arduino.h"
#include "K32_leds.h"
#include "K32_light_anims.h"



class K32_light {
  public:
    K32_light();
    void start();

    K32_leds* leds();
    K32_light_anim* anim( String animName);

    void play( K32_light_anim* anim );
    void play( String animName );
    void stop();
    void blackout();

    K32_light_anim* getActiveAnim();
    bool isPlaying();

  private:
    K32_leds* _leds;
    K32_light_animbook* _book;

    TaskHandle_t animateHandle = NULL;
    K32_light_anim* activeAnim;
    SemaphoreHandle_t stop_lock;

    static void animate( void * parameter );
    static void async_stop( void * parameter );
};



#endif
