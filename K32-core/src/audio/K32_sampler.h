/*
  K32_sampler.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_sampler_h
#define K32_sampler_h

#include "Arduino.h"

#define MIDI_MAX_BANK 17      //17
#define MIDI_MAX_NOTE 128     //128
#define MIDI_MAX_TITLE 14     // Filename length

class K32_sampler{
  public:
    K32_sampler(const int SD_PIN[4]);

    void scan();

    String path(int note, int bank = -1);
    void bank(int bank); 
    int bank(); 

    int size(byte note, byte bank = -1);
    void remove(byte note, byte bank = -1);

  private:
    SemaphoreHandle_t lock;
    static void task( void * parameter );

    int _bank = 1;
    char samples[MIDI_MAX_BANK][MIDI_MAX_NOTE][MIDI_MAX_TITLE];

    String pad3(int input);
};

#endif