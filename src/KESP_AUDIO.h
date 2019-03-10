/*
  KESP_AUDIO.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef KESP_AUDIO_h
#define KESP_AUDIO_h

#include "Arduino.h"
#include "KESP_LOG.h"

//https://github.com/earlephilhower/ESP8266Audio
//https://github.com/Gianbacchio/ESP8266_Spiram
#include "AudioGenerator.h"
#include "AudioFileSourceSD.h"
#include "AudioOutputI2S.h"

//https://github.com/tommag/PCM51xx_Arduino
#include "PCM51xx.h"

// MIDI stuff
#define MIDI_MAX_BANK 17      //17
#define MIDI_MAX_NOTE 128    //128
#define MIDI_MAX_TITLE 10


class KESP_AUDIO {
  public:
    KESP_AUDIO(bool pcmOK);

    bool isEngineOK();
    bool isSdOK();

    void setGainLimits(int min, int max);
    void volume(int vol);
    void loop(bool doLoop);

    bool run();
    bool play(String filePath);
    void stop();

    bool isPlaying();
    String media();
    String error();

    void midiNoteScan();
    String midiNotePath(int bank, int note);
    int midiNoteSize(byte bank, byte note);
    void midiNoteDelete(byte bank, byte note);


  private:
    SemaphoreHandle_t lock;

    PCM51xx* pcm;
    AudioOutputI2S *out;
    AudioGenerator* gen;
    AudioFileSourceSD *file;

    String currentFile = "";
    bool doLoop = false;
    bool sdOK = false;
    bool engineOK = false;
    String errorPlayer = "";

    int gainMin = 140;
    int gainMax = 60;

    char notes[MIDI_MAX_BANK][MIDI_MAX_NOTE][MIDI_MAX_TITLE];

    String pad3(int input);
};

#endif
