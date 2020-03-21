/*
  K32_light.cpp
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/

#include "Arduino.h"
#include "K32_light.h"


K32_light::K32_light() {
  // ANIMATOR
  this->activeAnim = NULL;
  this->_leds = new K32_leds();
  this->_book = new K32_light_animbook();

  this->stop_lock = xSemaphoreCreateBinary();
  xSemaphoreGive(this->stop_lock);
}

void K32_light::start() {
  this->_leds->start();
  this->play( "test" );
}

K32_leds* K32_light::leds() {
  return this->_leds;
}

K32_light_anim* K32_light::anim( String animName) {
  return this->_book->get(animName);
}

K32_light_anim* K32_light::getActiveAnim() {
  if (this->activeAnim != NULL) return this->activeAnim;
  else return new K32_light_anim();
}

void K32_light::play( K32_light_anim* anim ) {
  // ANIM task
  this->stop();
  this->activeAnim = anim;
  this->activeAnim->init();
  xTaskCreate( this->animate,        // function
                "leds_anim_task", // task name
                2000,             // stack memory    // 5000 not enough 
                (void*)this,      // args
                3,                      // priority
                &this->animateHandle ); // handler
   LOGINL("LIGHT: play ");
   LOG(anim->name());
}

void K32_light::play( String animName ) {
  this->play( this->anim( animName ) );
}

void K32_light::stop() {
  if (!this->isPlaying()) return;
  xSemaphoreTake(this->stop_lock, portMAX_DELAY);
  xTaskCreate( this->async_stop,              // function
                "leds_stop_task",           // task name
                1000,                      // stack memory
                (void*)this,              // args
                10,                      // priority
                NULL );                 // handler
  xSemaphoreTake(this->stop_lock, portMAX_DELAY);
  xSemaphoreGive(this->stop_lock);
  LOG("LIGHT: stop");
}

bool K32_light::isPlaying() {
  return (this->activeAnim != NULL);
}


/*
 *   PRIVATE
 */

  void K32_light::animate( void * parameter ) {
    K32_light* that = (K32_light*) parameter;
    if (that->activeAnim){
      bool RUN = true;
      while(RUN) {
        RUN = that->activeAnim->loop( that->_leds );
        yield();
      }
    }
    LOG("LIGHT: end");
    that->animateHandle = NULL;
    that->activeAnim = NULL;
    vTaskDelete(NULL);
  }

  void K32_light::async_stop( void * parameter ) {
    K32_light* that = (K32_light*) parameter;
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
