/*
  K32_animbook.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_animbook_h
#define K32_animbook_h

#define LEDS_ANIMS_SLOTS  16

#include "K32_anim_generic.h"
#include "K32_anim_charge.h"

//
// NOTE: to be able to load an animation by name, it must be registered in K32_animbook.h
//

//
// ANIMATOR BOOK
//
class K32_animbook {
  public:
    K32_animbook() {

      //
      // REGISTER AVAILABLE ANIMS !
      //
      this->add( new K32_anim_test() );
      // this->add( new K32_anim_sinus() );
      // this->add( new K32_anim_strobe() );
      // this->add( new K32_anim_hardstrobe() );
      // this->add( new K32_anim_chaser() );
      this->add( new K32_anim_discharge() );
      this->add( new K32_anim_charge() );

    }

    K32_anim* get( String name ) {
      for (int k=0; k<this->counter; k++)
        if (this->anims[k]->name() == name) {
          // LOGINL("LEDS: "); LOG(name);
          return this->anims[k];
        }
      LOGINL("ANIM: not found "); LOG(name);
      return new K32_anim("dummy");
    }


  private:
    K32_anim* anims[LEDS_ANIMS_SLOTS];
    int counter = 0;

    void add(K32_anim* anim) {
      if (this->counter >= LEDS_ANIMS_SLOTS) {
        LOG("ERROR: no more slot available to register new animation");
        return;
      }
      this->anims[ this->counter ] = anim;
      this->counter++;
      LOGINL("ANIM: register "); LOG(anim->name());
    };

};

#endif