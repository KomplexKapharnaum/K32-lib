/*
  K32_leds.cpp
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/

#include "Arduino.h"
#include "K32_ledstrip.h"


K32_ledstrip::K32_ledstrip(int chan, int pin, int type, int size) {

  this->buffer_lock = xSemaphoreCreateMutex();
  this->draw_lock = xSemaphoreCreateBinary();
  this->show_lock = xSemaphoreCreateBinary();
  xSemaphoreGive(this->show_lock);

  this->_strand = digitalLeds_addStrand(
    {.rmtChannel = chan, .gpioNum = pin, .ledType = type, .brightLimit = 255, .numPixels = size, .pixels = nullptr, ._stateVars = nullptr});

  this->_buffer = static_cast<pixelColor_t*>(malloc(this->_strand->numPixels * sizeof(pixelColor_t)));
  this->black();

  // LOOP task
  xTaskCreate(this->draw,       // function
              "leds_show_task", // task name
              1000,             // stack memory
              (void *)this,     // args
              4,                // priority
              NULL);            // handler
}

int K32_ledstrip::size() {
  return this->_strand->numPixels;
}

void K32_ledstrip::lock() {
  xSemaphoreTake(this->show_lock, portMAX_DELAY);
}

void K32_ledstrip::unlock() {
  xSemaphoreGive(this->show_lock);
}

K32_ledstrip *K32_ledstrip::black() {
  xSemaphoreTake(this->buffer_lock, portMAX_DELAY);
  memset(this->_buffer, 0, this->size() * sizeof(pixelColor_t));
  this->dirty = true;
  xSemaphoreGive(this->buffer_lock);
  this->show();
  return this;
}

K32_ledstrip* K32_ledstrip::all(pixelColor_t color) {
  xSemaphoreTake(this->buffer_lock, portMAX_DELAY);
  for(int k= 0; k<this->size(); k++) this->_buffer[k] = color;
  this->dirty = true;
  xSemaphoreGive(this->buffer_lock);
  return this;
}

K32_ledstrip* K32_ledstrip::all(int red, int green, int blue, int white) {
  return this->all( pixelFromRGBW(red, green, blue, white) );
}

K32_ledstrip* K32_ledstrip::pix(int pixel, pixelColor_t color) {
  xSemaphoreTake(this->buffer_lock, portMAX_DELAY);
  this->_buffer[pixel] = color;
  this->dirty = true;
  xSemaphoreGive(this->buffer_lock);
  return this;
}

K32_ledstrip* K32_ledstrip::pix(int pixel, int red, int green, int blue, int white) {
  return this->pix( pixel, pixelFromRGBW(red, green, blue, white) );
}

void K32_ledstrip::show() {
  
  // COPY Buffers to STRAND
  xSemaphoreTake(this->show_lock, portMAX_DELAY);
  xSemaphoreTake(this->buffer_lock, portMAX_DELAY);
  if (this->dirty) {
    memcpy(&this->_strand->pixels, &this->_buffer, sizeof(this->_buffer));
    this->dirty = false;
    xSemaphoreGive(this->draw_lock);
  }
  else xSemaphoreGive(this->show_lock);
  xSemaphoreGive(this->buffer_lock);

}

void K32_ledstrip::draw(void *parameter)
{
  K32_ledstrip *that = (K32_ledstrip *)parameter;

  // run
  while (true) {
    xSemaphoreTake(that->draw_lock, portMAX_DELAY);   // WAIT for show()
    digitalLeds_updatePixels(that->_strand);           // PUSH LEDS TO RMT
    xSemaphoreGive(that->show_lock);                  // READY for next show()
  }
  vTaskDelete(NULL);
}




