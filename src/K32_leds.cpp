/*
  K32_leds.cpp
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/

#include "Arduino.h"
#include "K32_leds.h"

K32_leds::K32_leds() {
  this->dirty = xSemaphoreCreateBinary();
  this->buffer_lock = xSemaphoreCreateMutex();
  this->strands_lock = xSemaphoreCreateBinary();
  xSemaphoreGive(this->strands_lock);

  int pins[LEDS_NUM_STRIPS] = {21, 22};

  for (int s = 0; s < LEDS_NUM_STRIPS; s++)
    this->strands[s] = new SmartLed( LED_WS2812, LEDS_NUM_PIXEL, pins[s], s );

  // this->blackout();

  // LOOP task
  xTaskCreate( this->task,        // function
                "leds_show_task", // task name
                1000,             // stack memory
                (void*)this,      // args
                3,                // priority
                NULL);            // handler
};

void K32_leds::show() {

  // COPY
  xSemaphoreTake(this->strands_lock, portMAX_DELAY);

  xSemaphoreTake(this->buffer_lock, portMAX_DELAY);
  for (int strip = 0; strip < LEDS_NUM_STRIPS; strip++)
    for (int pixel = 0 ; pixel < LEDS_NUM_PIXEL ; pixel++)
      this->strands[strip]->operator[](pixel) = this->buffer[strip][pixel];

  LOGINL("buffer copy ");
  LOGINL(this->buffer[0][0].getGrb(0)); LOGINL(" ");
  LOGINL(this->buffer[0][0].getGrb(1)); LOGINL(" ");
  LOGINL(this->buffer[0][0].getGrb(2)); LOGINL(" / ");

  LOGINL(this->strands[0]->operator[](0).getGrb(0)); LOGINL(" ");
  LOGINL(this->strands[0]->operator[](0).getGrb(1)); LOGINL(" ");
  LOGINL(this->strands[0]->operator[](0).getGrb(2)); LOG("");
  xSemaphoreGive(this->buffer_lock);

  xSemaphoreGive(this->dirty);
}


void K32_leds::test() {
  int wait = 400;

  this->blackout();

  this->setPixel(-1, 0, 100, 0, 0);
  this->setPixel(-1, 1, 100, 0, 0);
  this->setPixel(-1, 2, 100, 0, 0);
  this->show();
  delay(wait);

  this->setPixel(-1, 0, 0, 100, 0);
  this->setPixel(-1, 1, 0, 100, 0);
  this->setPixel(-1, 2, 0, 100, 0);
  this->show();
  delay(wait);

  this->setPixel(-1, 0, 0, 0, 100);
  this->setPixel(-1, 1, 0, 0, 100);
  this->setPixel(-1, 2, 0, 0, 100);
  this->show();
  delay(wait);

  this->blackout();
}


void K32_leds::blackout() {
  this->setAll(0, 0, 0);
  this->show();
}


void K32_leds::setAll(int red, int green, int blue) {
  this->setStrip(-1, red, green, blue);
}


void K32_leds::setStrip(int strip, int red, int green, int blue) {
  for (int i = 0 ; i < LEDS_NUM_PIXEL ; i++)
    this->setPixel(strip, i, red, green, blue);
}


void K32_leds::setPixel(int strip, int pixel, int red, int green, int blue) {
  if (strip == -1) {
    for (int s = 0; s < LEDS_NUM_STRIPS; s++)
      this->setPixel(s, pixel, red, green, blue);
  }
  else if (pixel == -1) this->setStrip(strip, red, green, blue);
  else if ((strip < 0) or (strip >= LEDS_NUM_STRIPS) or (pixel < 0) or (pixel >= LEDS_NUM_PIXEL)) return;
  else {
    red = red * red / 255;
    green = green * green / 255;
    blue = blue * blue / 255;
    if (red > 255) red = 255;     if (red < 0) red = 0;
    if (green > 255) green = 255; if (green < 0) green = 0;
    if (blue > 255) blue = 255;   if (blue < 0) blue = 0;
    xSemaphoreTake(this->buffer_lock, portMAX_DELAY);
    this->buffer[strip][pixel] = Rgb{ red, green, blue };
    xSemaphoreGive(this->buffer_lock);
  }
}


/*
 *   PRIVATE
 */


 void K32_leds::task( void * parameter ) {
   K32_leds* that = (K32_leds*) parameter;
   TickType_t xFrequency = pdMS_TO_TICKS(ceil(1000000/(LEDS_NUM_STRIPS*LEDS_NUM_PIXEL*3*8*1.25)));

   while(true) {

     // WAIT show() is called
     xSemaphoreTake(that->dirty, portMAX_DELAY);

     // PUSH LEDS TO RMT
     for (int s = 0; s < LEDS_NUM_STRIPS; s++)
       that->strands[s]->show();

     LOGINL("strands show ");
     LOGINL(that->strands[0]->operator[](0).getGrb(0)); LOGINL(" ");
     LOGINL(that->strands[0]->operator[](0).getGrb(1)); LOGINL(" ");
     LOGINL(that->strands[0]->operator[](0).getGrb(2)); LOG("");

     // ENSURE TRANSMISSION END
     for (int s = 0; s < LEDS_NUM_STRIPS; s++)
      that->strands[s]->wait();

     LOG("strands set ");

     xSemaphoreGive(that->strands_lock);

     vTaskDelay( xFrequency );

   }

   vTaskDelete(NULL);
 }
