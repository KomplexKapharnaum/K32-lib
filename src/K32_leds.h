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
#include "K32_leds_rmt.h"
#include "K32_leds_anims.h"



class K32_leds {
  public:
    K32_leds();

    K32_leds_anim* anim( String animName);

    void play( K32_leds_anim* anim );
    void play( String animName );
    void stop();

  private:
    K32_leds_rmt* leds;
    K32_leds_animbook* book;

    TaskHandle_t animateHandle = NULL;
    K32_leds_anim* activeAnim;

    static void animate( void * parameter );
};



#endif
