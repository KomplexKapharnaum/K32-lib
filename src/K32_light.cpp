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
  this->_book = new K32_light_animbook();

  this->stop_lock = xSemaphoreCreateBinary();
  xSemaphoreGive(this->stop_lock);

  digitalLeds_init();
}

void K32_light::addStrip(const int pin, led_types type, int size)
{
  if (size == 0) size = LEDSTRIP_MAXPIXEL;

  if (this->_nstrips >= LEDS_MAXSTRIPS) {
    LOG("LEDS: no more strip can be attached..");
    return;
  }

  int s = this->_nstrips;
  this->_nstrips += 1;
  
  this->_strips[s] = new K32_ledstrip(s, pin, type, size);
}

void K32_light::start() {
  // test routine
  this->play( "test" );
}

K32_ledstrip* K32_light::strip(int s) {
  return this->_strips[s];
}

K32_light* K32_light::strips() {
  return this;
}

K32_light* K32_light::black()
{
  for (int s = 0; s < this->_nstrips; s++) this->_strips[s]->black();
  return this;
}

K32_light* K32_light::all(pixelColor_t color)
{
  for (int s = 0; s < this->_nstrips; s++) this->_strips[s]->all(color);
  return this;
}

K32_light* K32_light::all(int red, int green, int blue, int white)
{
  return this->all( pixelFromRGBW(red, green, blue, white) );
}

K32_light* K32_light::pix(int pixel, pixelColor_t color) {
  for (int s = 0; s < this->_nstrips; s++) this->_strips[s]->pix(pixel, color);
  return this;
}

K32_light* K32_light::pix(int pixel, int red, int green, int blue, int white) {
  return this->pix( pixel, pixelFromRGBW(red, green, blue, white) );
}

void K32_light::show() {
  for (int s=0; s<this->_nstrips; s++) this->_strips[s]->show();
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

void K32_light::blackout() {
  this->stop();
  this->strips()->black()->show();
}

bool K32_light::isPlaying() {
  return (this->activeAnim != NULL);
}


/*
 *   PRIVATE
 */

int K32_light::_nstrips = 0;

void K32_light::animate( void * parameter ) {
  K32_light* that = (K32_light*) parameter;
  if (that->activeAnim){
    bool RUN = true;
    while(RUN) {
      RUN = that->activeAnim->loop( that->strip(0) );
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
  that->strip(0)->black();
  xSemaphoreGive(that->stop_lock);
  vTaskDelete(NULL);
} 


 /////////////////////////////////////////////
