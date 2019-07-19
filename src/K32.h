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
#define K32_VERSION 1.13     // mqtt fix reconnect
#define K32_VERSION 1.15     // mqtt bank select

#include <Arduino.h>
#include <WiFi.h>

// ESP
#include <HTTPClient.h>
#include <SPIFFS.h>
#include <FS.h>

// prevent cicular include error
class K32;

struct mqttconf {
   const char* broker;
   int beatInterval;
};

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
  mqttconf mqtt;
};

// K32 components
#include "K32_log.h"
#include "K32_settings.h"
#include "K32_stm32.h"
#include "K32_wifi.h"
#include "K32_leds.h"
#include "K32_audio.h"
#include "K32_samplermidi.h"
#include "K32_osc.h"
#include "K32_mqtt.h"

class K32 {
  public:
    K32(k32conf conf);

    K32_settings* settings = NULL;
    K32_stm32* stm32 = NULL;
    K32_wifi* wifi = NULL;
    K32_osc* osc = NULL;
    K32_mqtt* mqtt = NULL;
    K32_leds* leds = NULL;
    K32_audio* audio = NULL;
    K32_samplermidi* sampler = NULL;

  private:


};





#endif
