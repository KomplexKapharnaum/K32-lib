/*
  K32.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_h
#define K32_h

// #define K32_VERSION 1.00
// #define K32_VERSION 1.01  // audio forced kill to avoid deadlock
// #define K32_VERSION 1.02  // audio keep task active
// #define K32_VERSION 1.03  // fixed audio memory leak
// #define K32_VERSION 1.04  // v2 board
// #define K32_VERSION 1.05  // led anim sinus ++
// #define K32_VERSION 1.06  // mqtt (not enabled yet)
// #define K32_VERSION 1.07  // fix various bug (wifi reconnect / audio missing / ...)
// #define K32_VERSION 1.08     // mqtt sampler
// #define K32_VERSION 1.09     // mqtt leds
// #define K32_VERSION 1.10     // mqtt audio
// #define K32_VERSION 1.11     // fix
// #define K32_VERSION 1.12     // mqtt ping before reconnect
// #define K32_VERSION 1.13     // mqtt fix reconnect
// #define K32_VERSION 1.15     // mqtt bank select
// #define K32_VERSION 1.16     // mqtt / osc / esp-regie
// #define K32_VERSION 1.17     // memory osc bug
// #define K32_VERSION 1.18        // disable onclick (buggy !)
// #define K32_VERSION 1.19        // deprecated /audio/sample (keep max compat)
#define K32_VERSION 2.00 // inter modules communication refactoring (modular / lazy includes)

#include <Arduino.h>

#include "K32_log.h"
#include "K32_settings.h"
#include "K32_stm32.h"
#include "K32_wifi.h"
#include "K32_audio.h"
#include "K32_samplermidi.h"
#include "K32_light.h"


byte LEDS_PIN[2][2] = {
  {21, 22},   // HW_REVISION 1
  {23, 22}    // HW_REVISION 2
};

byte AUDIO_PIN[2][2] = {
  {21, 22},   // HW_REVISION 1
  {23, 22}    // HW_REVISION 2
};



class K32
{
public:
  K32()
  {

    // Settings config
    settings = new K32_settings();

    // Settings SET
#ifdef K32_SET_NODEID
    settings->id(K32_SET_NODEID);
    settings->channel(15);
#endif
#ifdef K32_SET_HWREVISION
    settings->hw(K32_SET_HWREVISION);
#endif

#ifdef K32_ENABLE_STM32
    // STM32
    stm32 = new K32_stm32();
#else
    LOGSETUP();
#endif

#ifdef K32_ENABLE_AUDIO
    // AUDIO  (Note: Audio must be started before LEDS !!)
    if (settings->hw() == 1 || settings->hw() == 2) {
      audio = new K32_audio();
      // SAMPLER MIDI
      sampler = new K32_samplermidi();
    }
    else LOG("LIGHT: Error HWREVISION not valid please define K32_SET_HWREVISION");
#endif

#ifdef K32_ENABLE_LIGHT
    // LEDS
    if (settings->hw() == 1 || settings->hw() == 2) {
      light = new K32_light();
      light->leds()->attach(LEDS_PIN[settings->hw()-1][0], 120, LED_SK6812W_V1);
      light->leds()->attach(LEDS_PIN[settings->hw()-1][1], 120, LED_SK6812W_V1);
      light->start();
    }
    else LOG("LIGHT: Error HWREVISION not valid please define K32_SET_HWREVISION");
#endif

#ifdef K32_ENABLE_WIFI
    // WIFI init
    btStop();
    wifi = new K32_wifi(settings->name());

    //   if (conf.wifi.ip)
    //     wifi->staticIP(conf.wifi.ip);
    //   wifi->connect(conf.wifi.ssid, conf.wifi.password);
    // }
#endif
  }

  void reset()
  {
    if (stm32)
      stm32->reset();
    else
      while (true)
        ;
  }

  K32_settings *settings = NULL;
  K32_stm32 *stm32 = NULL;
  K32_wifi *wifi = NULL;
  K32_audio *audio = NULL;
  K32_samplermidi *sampler = NULL;
  K32_light *light = NULL;

private:
};

#endif
