/*
  K32_light.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_light_h
#define K32_light_h

#define LEDS_MAXSTRIPS    8    // There is 8 RMT channels on ESP32
#define LEDS_ANIM8_FPS    100   // Anims modulation FPS
#define LEDS_SHOW_FPS     100   // Show RMT push FPS
#define LEDS_ANIMS_SLOTS  16


#include "K32_ledstrip.h"
#include "K32_anim.h"

//
// NOTE: to be able to use a modulator, it must be included here
//

#include "modulators/K32_mod_waves.h"
#include "modulators/K32_mod_fades.h"

//
// NOTE: to be able to use an animation, it must be included here
//

#include "animations/K32_anim_basics.h"
#include "animations/K32_anim_charge.h"
#include "animations/K32_anim_dmx.h"



class K32_light {
  public:
    K32_light();

    void start();
    
    // STRIPS
    //
    void addStrip(const int pin, led_types type, int size = 0);

    K32_ledstrip* strip(int s);
    K32_light* strips();

    K32_light* black();
    K32_light* all(pixelColor_t color);
    K32_light* all(int red, int green, int blue, int white = 0);
    K32_light* pix(int pixel, pixelColor_t color);
    K32_light* pix(int pixel, int red, int green, int blue, int white = 0);
    void show();
    void blackout();


    //  ANIM
    //
    K32_anim* anim( String animName = "");
    K32_anim* anim( String animName, K32_anim* anim, int stripN = 0, int size = 0, int offset = 0);
    void stop();

    // Set FPS
    void fps(int f = -1);
    

  private:

    static int _nstrips;
    K32_ledstrip* _strips[LEDS_MAXSTRIPS];

    K32_anim* _anims[LEDS_ANIMS_SLOTS];
    int _animcounter = 0;

    static void refresh( void * parameter ) ;
    int _fps = LEDS_SHOW_FPS;

};



#endif
