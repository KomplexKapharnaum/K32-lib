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
  this->_leds = new K32_leds_rmt();
  this->_book = new K32_leds_animbook();

  this->stop_lock = xSemaphoreCreateBinary();
  xSemaphoreGive(this->stop_lock);

}

K32_leds_rmt* K32_leds::leds() {
  return this->_leds;
}

K32_leds_anim* K32_leds::anim( String animName) {
  return this->_book->get(animName);
}


void K32_leds::play( K32_leds_anim* anim ) {
  // ANIM task
  this->stop();
  this->activeAnim = anim;
  xTaskCreate( this->animate,        // function
                "leds_anim_task", // task name
                5000,             // stack memory    // 5000 not enough 
                (void*)this,      // args
                3,                      // priority
                &this->animateHandle ); // handler
  //  LOGINL("created - ");
}

void K32_leds::play( String animName ) {
  this->play( this->anim( animName ) );
}

void K32_leds::stop() {
  xSemaphoreTake(this->stop_lock, portMAX_DELAY);
  xTaskCreate( this->async_stop,              // function
                "leds_stop_task",           // task name
                1000,                      // stack memory
                (void*)this,              // args
                10,                      // priority
                NULL );                 // handler
  xSemaphoreTake(this->stop_lock, portMAX_DELAY);
  xSemaphoreGive(this->stop_lock);
  LOG("LEDS: stop");
}

bool K32_leds::isPlaying() {
  return (this->activeAnim != NULL);
}


/*
 *   PRIVATE
 */

  void K32_leds::animate( void * parameter ) {
    K32_leds* that = (K32_leds*) parameter;
    if (that->activeAnim){
      bool RUN = true;
      while(RUN) {
        RUN = that->activeAnim->loop( that->_leds );
        yield();
      }
    }

    that->animateHandle = NULL;
    that->activeAnim = NULL;
    vTaskDelete(NULL);
  }

  void K32_leds::async_stop( void * parameter ) {
    K32_leds* that = (K32_leds*) parameter;
    if (that->activeAnim) that->activeAnim->lock();
    if (that->animateHandle) {
      vTaskDelete( that->animateHandle );
      that->animateHandle = NULL;
    }
    if (that->activeAnim) that->activeAnim->unlock();
    that->activeAnim = NULL;
    that->_leds->blackout();
    xSemaphoreGive(that->stop_lock);
    vTaskDelete(NULL);
  } 


 /////////////////////////////////////////////
