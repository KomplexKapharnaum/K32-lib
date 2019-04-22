/*
  K32_leds.cpp
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/

#include "Arduino.h"
#include "K32_leds.h"


K32_leds::K32_leds() {
  // ANIMATOR
  this->activeAnim = NULL;
  this->running = false;
  this->_leds = new K32_leds_rmt();
  this->_book = new K32_leds_animbook();
}

K32_leds_rmt* K32_leds::leds() {
  return this->_leds;
}

K32_leds_anim* K32_leds::anim( String animName) {
  return this->_book->get(animName);
}


void K32_leds::play( K32_leds_anim* anim ) {
  // ANIM task
  this->activeAnim = anim;
  this->stop();
  this->running = true;
  xTaskCreate( this->animate,        // function
                "leds_anim_task", // task name
                10000,             // stack memory
                (void*)this,      // args
                3,                      // priority
                &this->animateHandle ); // handler
  //  LOGINL("created - ");
}

void K32_leds::play( String animName ) {
  this->play( this->anim( animName ) );
}

void K32_leds::stop() {
  this->running = false;
  while(this->animateHandle) delay(1);
  // LOG("done");
  // if (this->animateHandle) {
  //   vTaskDelete( this->animateHandle );
  //   this->animateHandle = NULL;
  // }
  this->_leds->blackout();
}


/*
 *   PRIVATE
 */

 void K32_leds::animate( void * parameter ) {
   K32_leds* that = (K32_leds*) parameter;
  //  LOG("go");
   if (that->activeAnim)
     while(that->activeAnim->loop( that->_leds ) && that->running) vTaskDelay(1);
  //  LOGINL("exit - ");
   
   that->animateHandle = NULL;
   vTaskDelete(NULL);
 }


 /////////////////////////////////////////////
