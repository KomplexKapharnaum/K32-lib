/*
  K32_audio.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_audio_h
#define K32_audio_h

#include "Arduino.h"
#include <HTTPClient.h>
#include "system/K32_log.h"
#include "K32_samplermidi.h"


//https://github.com/earlephilhower/ESP8266Audio
//https://github.com/Gianbacchio/ESP8266_Spiram
#include "AudioGenerator.h"
#include "AudioFileSourceSD.h"
#include "AudioOutputI2S.h"

//https://github.com/tommag/PCM51xx_Arduino
#include "PCM51xx.h"


class K32_audio {
  public:
    K32_audio(const int AUDIO_PIN[5], const int SD_PIN[4]);

    bool isEngineOK();
    bool isSdOK();

    void setGainLimits(int min, int max);
    void volume(int vol);
    void loop(bool doLoop);

    virtual bool play(String filePath, int velocity = 127);
    bool play();
    void stop();

    bool isPlaying();
    String media();
    String error();

    PCM51xx* pcm;

    K32_samplermidi *sampler = NULL;

  private:
    SemaphoreHandle_t lock;
    SemaphoreHandle_t runflag;
    static void task( void * parameter );

    void applyVolume();
    void initSoundcard(const int AUDIO_PIN[5]);

    AudioOutputI2S *out;
    AudioGenerator* gen;
    AudioFileSourceSD *file;
    // AudioFileSourceBuffer *buff;

    String currentFile = "";
    bool doLoop = false;
    bool sdOK = false;
    bool engineOK = false;
    String errorPlayer = "";
    int _volume = 100;
    int _velocity = 100;

    int gainMin = 150;
    int gainMax = 75;

};

#endif
