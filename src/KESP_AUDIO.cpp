/*
  KESP_AUDIO.cpp
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/

#include "Arduino.h"
#include "Wire.h"
#include "SD.h"
#include "KESP_AUDIO.h"


KESP_AUDIO::KESP_AUDIO() {
  this->lock = xSemaphoreCreateMutex();

  pcm = new PCM51xx(Wire); //Using the default I2C address 0x74

};
