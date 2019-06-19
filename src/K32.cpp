/*
  K32.cpp
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/

#include "K32.h"

K32::K32(k32conf conf) {
 
  // Settings config
  const char* keys[16] = {"id", "channel", "model"};
  settings = new K32_settings(keys);

  // STM32
  stm32 = new K32_stm32();
  if (conf.stm32) stm32->listen(true, true);
  // LOGSETUP(); // based on #define DEBUG

  // AUDIO  (Note: Audio must be started before LEDS !!)
  if (conf.audio) {
    audio = new K32_audio();
    if(!audio->isEngineOK()) {
      LOG("Audio engine failed to start.. RESET !");
      // stm32->reset();
    }
  }

  // SAMPLER MIDI
  if (conf.sampler) sampler = new K32_samplermidi();
  
  // LEDS
  if (conf.leds) {
    leds = new K32_leds();
    leds->play( "test" );
  }

  // WIFI init
  btStop();
  if (conf.wifi.ssid) {
    wifi = new K32_wifi( "esp-" + String(settings->get("id")) + "-v" + String(K32_VERSION, 2) );
    if (conf.wifi.ip) wifi->staticIP(conf.wifi.ip);
    wifi->connect(conf.wifi.ssid, conf.wifi.password);
    
    // if (!wifi->wait(10)) {
    //   stm32->reset();
    // }
  
    // OSC init
    osc = new K32_osc(conf.osc, this);
  }
  else WiFi.mode(WIFI_OFF);

  

};
