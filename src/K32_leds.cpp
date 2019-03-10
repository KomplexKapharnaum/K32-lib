/*
  K32_leds.cpp
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/

#include "Arduino.h"
#include "K32_leds.h"

K32_leds::K32_leds() {

  int pins[LEDS_NUM_STRIPS] = {21, 22};

  strand_t STRANDS[LEDS_NUM_STRIPS];
  for (int k = 0; k < LEDS_NUM_STRIPS; k++) {
    this->strands[k] = &STRANDS[k];
    STRANDS[k] = {  .rmtChannel = k, .gpioNum = pins[k], .ledType = LED_WS2812_V1, .brightLimit = 32,
                    .numPixels = LEDS_NUM_PIXEL, .pixels = nullptr, ._stateVars = nullptr
                 };

    // pinMode for ESP
    gpio_num_t gpioNumNative = static_cast<gpio_num_t>(pins[k]);
    gpio_mode_t gpioModeNative = static_cast<gpio_mode_t>(OUTPUT);
    gpio_pad_select_gpio(gpioNumNative);
    gpio_set_direction(gpioNumNative, gpioModeNative);
    gpio_set_level(gpioNumNative, LOW);
  }

  this->running = !digitalLeds_initStrands(STRANDS, LEDS_NUM_STRIPS);
  this->refresh = 1000/LEDS_FPS;

  this->blackout();
  this->show();
};


void K32_leds::show() {
  for (int s = 0; s < LEDS_NUM_STRIPS; s++)
    digitalLeds_updatePixels(this->strands[s]);
}


void K32_leds::blackout() {
  this->setAll(0, 0, 0);
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
    strands[strip]->pixels[pixel] = pixelFromRGB(red, green, blue);
  }
}
