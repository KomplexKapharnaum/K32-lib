/*
  K32_leds_anims.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_leds_anims_h
#define K32_leds_anims_h

#define LEDS_ANIMS_SLOTS  16
#define LEDS_PARAM_SLOTS  6

#include "K32_leds_rmt.h"

//
// NOTE: WHEN ADDING A NEW ANIM, REGISTER IT IN THE ANIMATOR DOWN THERE !
//

class K32_leds_anim {
  public:
    virtual String name () { return ""; };
    virtual bool loop ( K32_leds_rmt* leds ) { return false; };
    void setParam(int k, int value) {
      if (k >= LEDS_PARAM_SLOTS) return;
      this->params[k] = value;
    } 
    int params[LEDS_PARAM_SLOTS];
};


//
// TEST
//
class K32_leds_anim_test : public K32_leds_anim {
  public:
    K32_leds_anim_test() {
      this->params[0] = 150;    // intensity
      this->params[1] = 2000;   // initial delay
      this->params[2] = 200;    // step duration
    }
    
    String name () { return "test"; }
    
    bool loop ( K32_leds_rmt* leds ){
    
      int wait = this->params[2];

      delay(this->params[1]);
      LOG("LEDS: test");

      leds->blackout();

      leds->setPixel(-1, 0, this->params[0], 0, 0);
      leds->setPixel(-1, 1, this->params[0], 0, 0);
      leds->setPixel(-1, 2, this->params[0], 0, 0);
      leds->show();
      delay(wait);

      leds->setPixel(-1, 0, 0, this->params[0], 0);
      leds->setPixel(-1, 1, 0, this->params[0], 0);
      leds->setPixel(-1, 2, 0, this->params[0], 0);
      leds->show();
      delay(wait);

      leds->setPixel(-1, 0, 0, 0, this->params[0]);
      leds->setPixel(-1, 1, 0, 0, this->params[0]);
      leds->setPixel(-1, 2, 0, 0, this->params[0]);
      leds->show();
      delay(wait);

      leds->setPixel(-1, 0, 0, 0, 0, this->params[0]);
      leds->setPixel(-1, 1, 0, 0, 0, this->params[0]);
      leds->setPixel(-1, 2, 0, 0, 0, this->params[0]);
      leds->show();
      delay(wait);

      leds->blackout();

      return false;    // DON'T LOOP !

    };
};

//
// SINUS
//
class K32_leds_anim_sinus : public K32_leds_anim {
  public:
    K32_leds_anim_sinus() {
      this->params[0] = 2000; // period
    }

    String name () { return "sinus"; }
    
    bool loop ( K32_leds_rmt* leds ){

      int max = 255;
      int period = this->params[0];
      int white = 0;
      long start = millis();
      long progress = millis() - start;

      // LOG("LEDS sinus");

      while (progress <= period) {

        white = (0.5f + 0.5f * sin( 2 * PI * progress / period - 0.5f * PI ) ) * max;
        leds->setAll(white, white, white, white)->show();

        yield();
        progress = millis() - start;
      }

      leds->blackout();
      return true;    // LOOP !

    };
};


//
// STROBE
//
class K32_leds_anim_strobe : public K32_leds_anim {
  public:
    K32_leds_anim_strobe() {
      this->params[0] = 2000; // period
      this->params[1] = 50;   // % ON
    }

    String name () { return "strobe"; }
    
    bool loop ( K32_leds_rmt* leds ){
        
      int intensity = 255;
      TickType_t xOn = max(1, (int)pdMS_TO_TICKS( (this->params[0]*this->params[1]/100) ));
      TickType_t xOff = max(1, (int)pdMS_TO_TICKS( this->params[0] - (this->params[0]*this->params[1]/100) ));
      
      leds->setAll(intensity, intensity, intensity, intensity)->show();
      vTaskDelay( xOn );
      leds->blackout();
      vTaskDelay( xOff );

      return true;    // LOOP !

    };
};


//
// HARDSTROBE
//
class K32_leds_anim_hardstrobe : public K32_leds_anim {
  public:
    K32_leds_anim_hardstrobe() {
      this->params[0] = 2000; // period
      this->params[1] = 50;   // % ON

      this->nextEvent = 0;
      state = false;
    }

    String name () { return "hardstrobe"; }
    
    bool loop ( K32_leds_rmt* leds ){
      
      int intensity = 255;
      // LOGINL(millis()); LOGINL(" // "); LOG(this->nextEvent);  
      if (millis() > this->nextEvent) {
        // OFF -> ON
        if(!state) {
          this->nextEvent = millis() + (this->params[0]*this->params[1]/100) ;
          leds->setAll(intensity, intensity, intensity, intensity)->show();
          // LOGINL(" ON - "); LOG(millis()); 
        }
        // ON -> OFF
        else {
          this->nextEvent = millis() + this->params[0] - (this->params[0]*this->params[1]/100);
          leds->blackout();
          // LOGINL(" OFF - "); LOG(millis());
        }
        state = !state;
      }

      return true;    // LOOP !

    };
    
    long nextEvent;
    bool state;
};


//
// ANIMATOR BOOK
//
class K32_leds_animbook {
  public:
    K32_leds_animbook() {

      //
      // REGISTER AVAILABLE ANIMS !
      //
      this->add( new K32_leds_anim_test() );
      this->add( new K32_leds_anim_sinus() );
      this->add( new K32_leds_anim_strobe() );
      this->add( new K32_leds_anim_hardstrobe() );

    }

    K32_leds_anim* get( String name ) {
      for (int k=0; k<this->counter; k++)
        if (this->anims[k]->name() == name) {
          // LOGINL("ANIM: "); LOG(name);
          return this->anims[k];
        }
      LOGINL("ANIM: not found "); LOG(name);
      return new K32_leds_anim();
    }


  private:
    K32_leds_anim* anims[LEDS_ANIMS_SLOTS];
    int counter = 0;

    void add(K32_leds_anim* anim) {
      if (this->counter >= LEDS_ANIMS_SLOTS) {
        LOG("ERROR: no more slot available to register new animation");
        return;
      }
      this->anims[ this->counter ] = anim;
      this->counter++;
    };
    
};


#endif
