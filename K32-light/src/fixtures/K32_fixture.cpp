/*
  K32_fixture.cpp
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/

#include "K32_fixture.h"
#include <class/K32_module.h>

K32_fixture::K32_fixture(int size) {

  this->buffer_lock = xSemaphoreCreateMutex();
  this->draw_lock = xSemaphoreCreateBinary();
  this->show_lock = xSemaphoreCreateBinary();
  xSemaphoreTake(this->draw_lock, 1);
  xSemaphoreTake(this->show_lock, 1);

  if (size == 0) size = FIXTURE_MAXPIXEL;
  this->_size = size;

  this->_buffer = static_cast<pixelColor_t*>(malloc(this->_size * sizeof(pixelColor_t)));

  // LOOP task
  xTaskCreate(this->task,       // function
              "leds_show_task", // task name
              1000,             // stack memory
              (void *)this,     // args
              4,                // priority
              NULL);            // handler
  
  // LOG("-- init strip");
  this->black();
}

int K32_fixture::size() {
  return this->_size;
}

void K32_fixture::lock() {
  xSemaphoreTake(this->show_lock, portMAX_DELAY);
}

void K32_fixture::unlock() {
  xSemaphoreGive(this->show_lock);
}

bool K32_fixture::dirty() {
  return this->_dirty;
}

K32_fixture *K32_fixture::clear() {

  xSemaphoreTake(this->buffer_lock, portMAX_DELAY);
  memset(this->_buffer, 0, this->size() * sizeof(pixelColor_t));
  // for(int i=0; i < this->size(); i++) this->_buffer[i] = {0,0,0};
  this->_dirty = true;
  xSemaphoreGive(this->buffer_lock);

  return this;
}

K32_fixture *K32_fixture::black() {
  this->clear()->show();
  return this;
}

K32_fixture* K32_fixture::all(pixelColor_t color) {

  xSemaphoreTake(this->buffer_lock, portMAX_DELAY);
  for(int k= 0; k<this->size(); k++) this->_buffer[k] = color;
  this->_dirty = true;
  xSemaphoreGive(this->buffer_lock);

  return this;
}

K32_fixture* K32_fixture::all(int red, int green, int blue, int white) {
  return this->all( pixelFromRGBW(red, green, blue, white) );
}

K32_fixture* K32_fixture::pix(int pixelStart, int count, pixelColor_t color) {
    
  xSemaphoreTake(this->buffer_lock, portMAX_DELAY);
  for(int i = pixelStart; i<pixelStart+count; i++)
    if (i < this->size()) this->_buffer[i] = color;
  this->_dirty = true;
  xSemaphoreGive(this->buffer_lock);
  return this;
}

K32_fixture* K32_fixture::pix(int pixel, pixelColor_t color) {
  if (pixel < this->size()) 
  {
    xSemaphoreTake(this->buffer_lock, portMAX_DELAY);
    this->_buffer[pixel] = color;
    this->_dirty = true;
    xSemaphoreGive(this->buffer_lock);
  }
  return this;
}

K32_fixture* K32_fixture::pix(int pixel, int red, int green, int blue, int white) {
  return this->pix( pixel, pixelFromRGBW(red, green, blue, white) );
}

void K32_fixture::getBuffer(pixelColor_t* buffer, int _size, int offset) {
  xSemaphoreTake(this->buffer_lock, portMAX_DELAY);
  for(int k= 0; (k<this->size() && k<_size); k++) 
    buffer[k] = this->_buffer[k+offset];
  xSemaphoreGive(this->buffer_lock);
}

void K32_fixture::setBuffer(pixelColor_t* buffer, int _size, int offset) {
  xSemaphoreTake(this->buffer_lock, portMAX_DELAY);
  for(int k= 0; (k<this->size() && k<_size); k++) 
    this->_buffer[k+offset] = buffer[k];
  this->_dirty = true;
  xSemaphoreGive(this->buffer_lock);
}

// Virtual !
void K32_fixture::show() 
{
  xSemaphoreTake(this->show_lock, portMAX_DELAY);
  xSemaphoreTake(this->buffer_lock, portMAX_DELAY);
  if (this->_dirty) {
    
    // HERE: COPY BUFFER TO OUTPUT (only executed when _dirty)

    this->_dirty = false;
    xSemaphoreGive(this->draw_lock);
  }
  else xSemaphoreGive(this->show_lock);
  xSemaphoreGive(this->buffer_lock);
}

// Virtual !
void K32_fixture::draw() 
{
  // HERE: PUSH OUTPUT (only executed when _dirty)
}

void K32_fixture::task(void *parameter)
{
  K32_fixture *that = (K32_fixture *)parameter;

  // ready
  xSemaphoreGive(that->show_lock);

  // run
  while (true) {
    xSemaphoreTake(that->draw_lock, portMAX_DELAY);   // WAIT for show()
    that->draw();
    xSemaphoreGive(that->show_lock);                  // READY for next show()
  }
  vTaskDelete(NULL);
}




