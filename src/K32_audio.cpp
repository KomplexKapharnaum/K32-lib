/*
  K32_audio.cpp
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/

#include "Arduino.h"
#include "Wire.h"
#include "SD.h"
#include "K32_audio.h"


K32_audio::K32_audio() {
  this->lock = xSemaphoreCreateMutex();

  pcm = new PCM51xx(Wire); //Using the default I2C address 0x74

};
