/*
  K32_leds.cpp
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/

#include "Arduino.h"
#include "K32_leds_rmt.h"

K32_leds_rmt::K32_leds_rmt() {
  this->dirty = xSemaphoreCreateBinary();
  this->buffer_lock = xSemaphoreCreateMutex();
  this->strands_lock = xSemaphoreCreateBinary();
  xSemaphoreGive(this->strands_lock);

  int pins[LEDS_NUM_STRIPS] = {21, 22};
  for (int s = 0; s < LEDS_NUM_STRIPS; s++) {
    this->STRANDS[s] = {.rmtChannel = s, .gpioNum = pins[s], .ledType = LED_WS2812B_V1, .brightLimit = 255, .numPixels = LEDS_NUM_PIXEL, .pixels = nullptr, ._stateVars = nullptr};
    pinMode (s, OUTPUT);
    digitalWrite (s, LOW);
  }
  digitalLeds_initStrands( this->STRANDS, LEDS_NUM_STRIPS);

  for (int s = 0; s < LEDS_NUM_STRIPS; s++) {
    digitalLeds_resetPixels( &this->STRANDS[s] );
  }

  // LOOP task
  xTaskCreate( this->draw,        // function
                "leds_show_task", // task name
                1000,             // stack memory
                (void*)this,      // args
                4,                // priority
                NULL);            // handler

};

void K32_leds_rmt::show() {

  // COPY
  xSemaphoreTake(this->strands_lock, portMAX_DELAY);

  xSemaphoreTake(this->buffer_lock, portMAX_DELAY);
  for (int strip = 0; strip < LEDS_NUM_STRIPS; strip++)
    for (int pixel = 0 ; pixel < LEDS_NUM_PIXEL ; pixel++)
      this->STRANDS[strip].pixels[pixel] = this->buffer[strip][pixel];

  // LOGINL("buffer copy ");
  // LOGINL(this->buffer[0][0].r); LOGINL(" ");
  // LOGINL(this->buffer[0][0].g); LOGINL(" ");
  // LOGINL(this->buffer[0][0].b); LOGINL(" / ");
  //
  // LOGINL(this->STRANDS[0].pixels[0].r); LOGINL(" ");
  // LOGINL(this->STRANDS[0].pixels[0].g); LOGINL(" ");
  // LOGINL(this->STRANDS[0].pixels[0].b); LOG("");
  xSemaphoreGive(this->buffer_lock);

  xSemaphoreGive(this->dirty);
}


K32_leds_rmt* K32_leds_rmt::blackout() {
  this->setAll(0, 0, 0);
  this->show();
  return this;
}

K32_leds_rmt* K32_leds_rmt::setAll(int red, int green, int blue, int white) {
  this->setStrip(-1, red, green, blue, white);
  return this;
}

K32_leds_rmt* K32_leds_rmt::setAll(int red, int green, int blue) {
  return this->setAll(red, green, blue, 0);
}

K32_leds_rmt* K32_leds_rmt::setStrip(int strip, int red, int green, int blue, int white) {
  for (int i = 0 ; i < LEDS_NUM_PIXEL ; i++)
    this->setPixel(strip, i, red, green, blue, white);
  return this;
}

K32_leds_rmt* K32_leds_rmt::setStrip(int strip, int red, int green, int blue) {
  return this->setStrip(strip, red, green, blue, 0);
}


K32_leds_rmt* K32_leds_rmt::setPixel(int strip, int pixel, int red, int green, int blue) {
  return this->setPixel(strip, pixel, red, green, blue, 0);
}

K32_leds_rmt* K32_leds_rmt::setPixel(int strip, int pixel, int red, int green, int blue, int white) {
  if (strip == -1) {
    for (int s = 0; s < LEDS_NUM_STRIPS; s++)
      this->setPixel(s, pixel, red, green, blue);
  }
  else if (pixel == -1) this->setStrip(strip, red, green, blue);
  else if ((strip < 0) or (strip >= LEDS_NUM_STRIPS) or (pixel < 0) or (pixel >= LEDS_NUM_PIXEL)) return this;
  else {
    if (red > 255) red = 255;       if (red < 0) red = 0;
    if (green > 255) green = 255;   if (green < 0) green = 0;
    if (blue > 255) blue = 255;     if (blue < 0) blue = 0;
    if (white > 255) white = 255;   if (white < 0) white = 0;
    xSemaphoreTake(this->buffer_lock, portMAX_DELAY);
    this->buffer[strip][pixel] = pixelFromRGBW(red, green, blue, white);
    xSemaphoreGive(this->buffer_lock);
  }
  return this;
}

/*
 *   PRIVATE
 */


 void K32_leds_rmt::draw( void * parameter ) {
   K32_leds_rmt* that = (K32_leds_rmt*) parameter;
   TickType_t xFrequency = pdMS_TO_TICKS(ceil(1000000/(LEDS_NUM_STRIPS*LEDS_NUM_PIXEL*3*8*1.25)));

   while(true) {

     // WAIT show() is called
     xSemaphoreTake(that->dirty, portMAX_DELAY);

     // PUSH LEDS TO RMT
     for (int s = 0; s < LEDS_NUM_STRIPS; s++)
      digitalLeds_updatePixels( &that->STRANDS[s] );

     // LOGINL("strands show ");
     // LOGINL(that->STRANDS[0].pixels[0].r); LOGINL(" ");
     // LOGINL(that->STRANDS[0].pixels[0].g); LOGINL(" ");
     // LOGINL(that->STRANDS[0].pixels[0].b); LOG("");

     xSemaphoreGive(that->strands_lock);

     vTaskDelay( xFrequency );

   }

   vTaskDelete(NULL);
 }

