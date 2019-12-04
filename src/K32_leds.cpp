/*
  K32_leds.cpp
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/

#include "Arduino.h"
#include "K32_leds.h"

K32_leds::K32_leds()
{
  this->dirty = xSemaphoreCreateBinary();
  this->buffer_lock = xSemaphoreCreateMutex();
  this->strands_lock = xSemaphoreCreateBinary();
  xSemaphoreGive(this->strands_lock);
};

void K32_leds::attach(const int PIN, int NPIXEL, led_types LEDTYPE)
{
  if (LEDS_NSTRIPS >= LEDS_MAXSTRIPS)
  {
    LOG("LEDS: no more strip can be attached..");
    return;
  }

  int s = LEDS_NSTRIPS;
  LEDS_NSTRIPS += 1;

  LEDS_NPIXEL[s] = NPIXEL;
  this->STRANDS[s] = {.rmtChannel = s, .gpioNum = PIN, .ledType = LEDTYPE, .brightLimit = 255, .numPixels = NPIXEL, .pixels = nullptr, ._stateVars = nullptr};
}

void K32_leds::start()
{
  digitalLeds_initStrands(this->STRANDS, LEDS_NSTRIPS);

  for (int s = 0; s < LEDS_NSTRIPS; s++)
    digitalLeds_resetPixels(&this->STRANDS[s]);

  // LOOP task
  xTaskCreate(this->draw,       // function
              "leds_show_task", // task name
              1000,             // stack memory
              (void *)this,     // args
              4,                // priority
              NULL);            // handler
}

void K32_leds::show()
{

  // COPY
  xSemaphoreTake(this->strands_lock, portMAX_DELAY);

  xSemaphoreTake(this->buffer_lock, portMAX_DELAY);
  for (int strip = 0; strip < LEDS_NSTRIPS; strip++)
    for (int pixel = 0; pixel < LEDS_NPIXEL[strip]; pixel++)
      this->STRANDS[strip].pixels[pixel] = this->buffer[strip][pixel];

  // LOGINL("\nbuffer copy ");
  // LOGINL(this->buffer[0][0].r); LOGINL(" ");
  // LOGINL(this->buffer[0][0].g); LOGINL(" ");
  // LOGINL(this->buffer[0][0].b); LOGINL(" / ");

  // LOGINL(this->STRANDS[0].pixels[0].r); LOGINL(" ");
  // LOGINL(this->STRANDS[0].pixels[0].g); LOGINL(" ");
  // LOGINL(this->STRANDS[0].pixels[0].b); LOG("");
  xSemaphoreGive(this->buffer_lock);

  xSemaphoreGive(this->dirty);
}

K32_leds *K32_leds::blackout()
{
  this->setAll(0, 0, 0);
  this->show();
  return this;
}

K32_leds *K32_leds::setAll(int red, int green, int blue, int white)
{
  this->setStrip(-1, red, green, blue, white);
  return this;
}

K32_leds *K32_leds::setAll(int red, int green, int blue)
{
  return this->setAll(red, green, blue, 0);
}

K32_leds *K32_leds::setStrip(int strip, int red, int green, int blue, int white)
{
  if (strip == -1)
  {
    for (int s = 0; s < LEDS_NSTRIPS; s++)
      this->setStrip(s, red, green, blue, white);
  }
  else
  {
    for (int i = 0; i < LEDS_NPIXEL[strip]; i++)
      this->setPixel(strip, i, red, green, blue, white);
  }
  return this;
}

K32_leds *K32_leds::setStrip(int strip, int red, int green, int blue)
{
  return this->setStrip(strip, red, green, blue, 0);
}

K32_leds *K32_leds::setPixel(int strip, int pixel, int red, int green, int blue)
{
  return this->setPixel(strip, pixel, red, green, blue, 0);
}

K32_leds *K32_leds::setPixel(int strip, int pixel, int red, int green, int blue, int white)
{
  if (strip == -1)
  {
    for (int s = 0; s < LEDS_NSTRIPS; s++)
      this->setPixel(s, pixel, red, green, blue, white);
  }
  else if (pixel == -1)
    this->setStrip(strip, red, green, blue, white);
  else if ((strip < 0) or (strip >= LEDS_NSTRIPS) or (pixel < 0) or (pixel >= LEDS_NPIXEL[strip]))
    return this;
  else
  {
    if (red > 255)
      red = 255;
    if (red < 0)
      red = 0;
    if (green > 255)
      green = 255;
    if (green < 0)
      green = 0;
    if (blue > 255)
      blue = 255;
    if (blue < 0)
      blue = 0;
    if (white > 255)
      white = 255;
    if (white < 0)
      white = 0;
    xSemaphoreTake(this->buffer_lock, portMAX_DELAY);
    this->buffer[strip][pixel] = pixelFromRGBW(red, green, blue, white);
    xSemaphoreGive(this->buffer_lock);
  }
  return this;
}

/*
 *   PRIVATE
 */

int K32_leds::LEDS_NSTRIPS = 0;

void K32_leds::draw(void *parameter)
{
  K32_leds *that = (K32_leds *)parameter;

  LOG("LEDS: engine started");

  while (true)
  {

    // WAIT show() is called
    xSemaphoreTake(that->dirty, portMAX_DELAY);
    // PUSH LEDS TO RMT
    for (int s = 0; s < LEDS_NSTRIPS; s++)
      digitalLeds_updatePixels(&that->STRANDS[s]);

    //  LOGINL("strands show ");
    //  LOGINL(that->STRANDS[0].pixels[0].r); LOGINL(" ");
    //  LOGINL(that->STRANDS[0].pixels[0].g); LOGINL(" ");
    //  LOGINL(that->STRANDS[0].pixels[0].b); LOG("");

    xSemaphoreGive(that->strands_lock);
    //  yield();  // semaphore take dirty
  }

  vTaskDelete(NULL);
}
