/*
  K32_audio.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_audio_h
#define K32_audio_h

#include "Arduino.h"

//https://github.com/earlephilhower/ESP8266Audio
//https://github.com/Gianbacchio/ESP8266_Spiram
#include "AudioGenerator.h"
#include "AudioFileSourceSD.h"
#include "AudioOutputI2S.h"

//https://github.com/tommag/PCM51xx_Arduino
#include "PCM51xx.h"


class K32_audio {
  public:
    K32_audio();

    void setGainLimits(int min, int max);

  private:
    SemaphoreHandle_t lock;

    PCM51xx* pcm;
    AudioGenerator* gen;
    AudioFileSourceSD *file;
    AudioOutputI2S *out;

    String currentFile = "";
    bool doLoop = false;
    bool sdOK = false;
    bool engineOK = false;
    String errorPlayer = "";

    int gainMin = 140;
    int gainMax = 60;
};

#endif
