/*
  K32_samplerjpeg.h
  Created by RIRI, april 2020.
  Released under GPL v3.0
*/
#ifndef K32_samplerjpeg_h
#define K32_samplerjpeg_h

#include "Arduino.h"
#include "system/K32_log.h"

#define DMX_MAX_BANK 14  //255
#define DMX_MAX_FICH 255 //255
#define DMX_MAX_TITLE 14 // Filename length

class K32_samplerjpeg
{
public:
    K32_samplerjpeg(const int SD_PIN[4]);

    void scanjpeg();

    String path(int fich, int bank = -1);
    void bank(int bank);
    int bank();

    int size(byte fich, byte bank = -1);
    void remove(byte fich, byte bank = -1);

private:
    SemaphoreHandle_t lock;
    static void taskjpeg(void *parameter);

    int _bank = 1;
    char samplesjpeg[DMX_MAX_BANK][DMX_MAX_FICH][DMX_MAX_TITLE];

    String pad3(int input);
};

#endif