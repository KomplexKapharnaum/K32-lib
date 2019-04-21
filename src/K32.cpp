/*
  K32.cpp
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/

#include "K32.h"

K32::K32() {

  LOGSETUP(); // based on #define DEBUG

  // Settings config
  const char* keys[16] = {"id", "channel", "model"};
  settings = new K32_settings(keys);

  // Settings SET
  settings->set("id", 0);
  settings->set("channel", 1);
  settings->set("model", 1);   // 0: proto -- 1: big -- 2: small

  // STM32
  stm32 = new K32_stm32();
  // stm32->listen(true, true);

  // AUDIO
  audio = new K32_audio();
  if(!audio->isEngineOK()) {
    LOG("Audio engine failed to start.. RESET !");
    stm32->reset();
  }
  // delay(2000);

  // LEDS
  light = new K32_leds();
  light->play( "test" );

  // SAMPLER MIDI
  sampler = new K32_samplermidi();

  // WIFI init
  wifi = new K32_wifi( "esp-" + String(settings->get("id")) + "-v" + String(K32_VERSION, 2) );
  //wifi->static("192.168.0.237");
  wifi->connect("interweb", "superspeed37");
  // if (!wifi->wait(10)) {
  //   stm32->reset();
  // }

  // OSC init
  osc = new K32_osc(1818, this);

};
