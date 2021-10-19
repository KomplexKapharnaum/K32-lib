/*
  K32_light.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_light_h
#define K32_light_h

#define LIGHT_MAXFIXTURES   8     // There is 8 RMT channels on ESP32 for leds strips
#define LIGHT_ANIMATE_FPS  100     // Anims modulation FPS
#define LIGHT_SHOW_FPS     100     // Show RMT push FPS
#define LIGHT_ANIMS_SLOTS  16      
#define LIGHT_MAX_COPY     16


#include <class/K32_plugin.h>
#include "fixtures/K32_fixture.h"
#include "K32_anim.h"

//
// NOTE: to be able to use a modulator, it must be included here
//

#include "K32_mods.h"

//
// NOTE: to be able to use an animation, it must be included here
//

#include "animations/K32_anim_basics.h"
#include "animations/K32_anim_charge.h"

struct stripcopy
{
  K32_fixture* srcFixture;
  int srcStart;
  int srcStop;
  K32_fixture* destFixture;
  int destPos;
};



class K32_light : K32_plugin {
  public:
    K32_light(K32* k32);

    // Must be called after new K32_light
    void loadprefs() {
      #ifdef LIGHT_SET_ID
          this->id(LIGHT_SET_ID);
          _id = LIGHT_SET_ID;
      #else
          _id = k32->system->prefs.getUInt("LULU_id", 0);
      #endif
    }
    
    int id();
    void id(int id);

    // FIXTURES
    //
    K32_light* addFixture(K32_fixture* fix);
    K32_light* addFixtures(K32_fixture** fix, int count);
    void cloneFixturesFrom(K32_fixture* masterFixture);
    void copyFixture(stripcopy copy);

    K32_fixture* fixture(int s);
    K32_light* fixtures();

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
    K32_anim* anim( String animName, K32_anim* anim, int size = 0, int offset = 0);
    void stop();

    // Set FPS
    void fps(int f = -1);
    
    void command(Orderz* order);

  private:
    
    int _id = 1;

    static int _nfixtures;
    K32_fixture* _fixtures[LIGHT_MAXFIXTURES];

    K32_anim* _anims[LIGHT_ANIMS_SLOTS];
    int _animcounter = 0;

    
    static void refresh( void * parameter ) ;
    int _fps = LIGHT_SHOW_FPS;

    K32_fixture* _masterClone = nullptr;

    int _copyMax = 0;
    stripcopy _copylist[LIGHT_MAX_COPY];
};



#endif
