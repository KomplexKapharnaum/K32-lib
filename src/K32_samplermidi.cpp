/*
  K32_samplermidi.cpp
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/

#include "Arduino.h"
#include "Wire.h"
#include "SD.h"
#include "K32_samplermidi.h"


K32_samplermidi::K32_samplermidi() {
  this->lock = xSemaphoreCreateMutex();

  // Start SD
  if (!SD.exists("/")) {
    if (SD.begin()) LOG("SD card OK");
    else LOG("SD card ERROR");
  }

  // Scan samples
  this->scan();
};


void K32_samplermidi::scan() {
  xSemaphoreTake(this->lock, portMAX_DELAY);

  // Init Alias array
  for (byte bank = 0; bank < MIDI_MAX_BANK; bank++)
    for (byte note = 0; note < MIDI_MAX_NOTE; note++)
      for (byte k = 0; k < MIDI_MAX_TITLE; k++)
        this->samples[bank][note][k] = 0;

  // Check Bank dirs
  LOG("\nScanning...");
  for (byte i = 0; i < MIDI_MAX_BANK; i++) {
    if (SD.exists("/" + this->pad3(i))) {
      LOG("Scanning bank " + this->pad3(i));
      File dir = SD.open("/" + this->pad3(i));

      // Check Notes files
      while (true) {
        File entry =  dir.openNextFile();
        if (!entry) break;
        if (!entry.isDirectory()) {
          int note = 0;
          note = (entry.name()[5] - '0') * 100 + (entry.name()[6] - '0') * 10 + (entry.name()[7] - '0');
          if (note < MIDI_MAX_NOTE) {
            this->samples[i][note][0] = 1; // put a stamp even if alias is empty
            byte k = 0;
            while ( (k < MIDI_MAX_TITLE) && (entry.name()[8 + k] != 0) ) {
              this->samples[i][note][k] = entry.name()[8 + k];
              k++;
            }
          }
          //LOG("File found: "+String(entry.name())+" size="+String(entry.size()));
        }
      }
    }
  }
  xSemaphoreGive(this->lock);
  LOG("Scan done.");
}


String K32_samplermidi::path(int bank, int note) {
  String path = "/" + this->pad3(bank) + "/" + this->pad3(note);
  xSemaphoreTake(this->lock, portMAX_DELAY);
  if (this->samples[bank][note][0] > 1) path += String(this->samples[bank][note]);
  xSemaphoreGive(this->lock);
  // path += ".mp3";
  //if (SD.exists(path)) return path;
  //else return "";
  return path;
}


int K32_samplermidi::size(byte bank, byte note) {
  int sizeN = 0;
  String path = this->path(bank, note);
  if (SD.exists(path)) sizeN = SD.open(path).size();
  return sizeN;
}


void K32_samplermidi::remove(byte bank, byte note) {
  String path = this->path(bank, note);
  if (SD.exists(path)) {
    SD.remove(path);
    xSemaphoreTake(this->lock, portMAX_DELAY);
    this->samples[bank][note][0] = 0;
    xSemaphoreGive(this->lock);
    LOG("deleted " + path);
  }
}

String K32_samplermidi::pad3(int input) {
  char bank[4];
  bank[3] = 0;
  bank[0] = '0' + input / 100;
  bank[1] = '0' + (input / 10) % 10;
  bank[2] = '0' + input % 10;
  return String(bank);
}
