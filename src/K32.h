/*
  K32.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_h
#define K32_h

#define K32_VERSION 1.00  
#define K32_VERSION 1.01  // audio forced kill to avoid deadlock
#define K32_VERSION 1.02  // audio keep task active


#include <Arduino.h>
#include <WiFi.h>

// ESP
#include <HTTPClient.h>
#include <SPIFFS.h>
#include <FS.h>

// prevent cicular include error
class K32;

struct oscconf {
   int port_in;
   int port_out;
   int beatInterval;
   int beaconInterval;
};

struct wificonf {
   const char* ssid;
   const char* password;
   const char* ip;
};

struct k32conf {
  bool stm32;
  bool leds;
  bool audio;
  bool sampler;
  wificonf wifi;
  oscconf osc;
};

// K32 components
#include "K32_settings.h"
#include "K32_stm32.h"
#include "K32_wifi.h"
#include "K32_leds.h"
#include "K32_audio.h"
#include "K32_samplermidi.h"
#include "K32_osc.h"

class K32 {
  public:
    K32(k32conf conf);

    K32_settings* settings;
    K32_stm32* stm32;
    K32_wifi* wifi;
    K32_osc* osc;
    K32_leds* leds;
    K32_audio* audio;
    K32_samplermidi* sampler;

  private:


};





#endif
