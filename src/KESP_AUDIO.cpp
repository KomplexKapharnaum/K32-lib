/*
  KESP_AUDIO.cpp
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/

#include "Arduino.h"
#include "Wire.h"
#include "SD.h"
#include "KESP_AUDIO.h"

#include "AudioGeneratorWAV.h"
#include "AudioGeneratorMP3.h"
#include "AudioGeneratorFLAC.h"
#include "AudioGeneratorAAC.h"


KESP_AUDIO::KESP_AUDIO(bool pcmOK) {
  LOG("AUDIO init");

  this->lock = xSemaphoreCreateMutex();

  // Start SD
  if (SD.exists("/")) this->sdOK = true;
  else this->sdOK = SD.begin();
  if (this->sdOK) LOG("SD card OK");
  else LOG("SD card ERROR");

  // Start PCM engine
  this->engineOK = pcmOK;

  // Start I2S output
  this->out = new AudioOutputI2S(0,AudioOutputI2S::EXTERNAL_I2S, 8, AudioOutputI2S::APLL_DISABLE);
  this->out->SetPinout(25, 27, 26); //HW dependent ! BCK, LRCK, DATA
  this->out->SetGain( 1 );

  // Set Volume
  this->volume(100);
  this->gen = NULL;

};


bool KESP_AUDIO::isEngineOK() {
  return this->engineOK;
}

bool KESP_AUDIO::isSdOK() {
  return this->sdOK;
}


void KESP_AUDIO::setGainLimits(int min, int max) {
  xSemaphoreTake(this->lock, portMAX_DELAY);
  this->gainMin = min;
  this->gainMax = max;
  xSemaphoreGive(this->lock);
}


void KESP_AUDIO::volume(int vol)
{
  if (!this->engineOK) return;

  LOGF("GAIN: %i\n", vol);
  xSemaphoreTake(this->lock, portMAX_DELAY);
  vol = map(vol, 0, 100, this->gainMin, this->gainMax);
  this->pcm->setVolume(vol);
  xSemaphoreGive(this->lock);
}


void KESP_AUDIO::loop(bool doLoop) {
  xSemaphoreTake(this->lock, portMAX_DELAY);
  this->doLoop = doLoop;
  xSemaphoreGive(this->lock);
}


bool KESP_AUDIO::run() {
  if (this->isPlaying()) {
    xSemaphoreTake(this->lock, portMAX_DELAY);
    if (this->gen->loop()) {
      xSemaphoreGive(this->lock);
      return true;
    }
    else if (this->doLoop && this->currentFile != "") {
      //this->play(currentFile);
      this->file->seek(0, SEEK_SET);
      LOG("loop: " + this->currentFile);
      xSemaphoreGive(this->lock);
      return true;
    }
    else {
      xSemaphoreGive(this->lock);
      this->stop();
      return false;
    }
  }
  return false;
}


bool KESP_AUDIO::play(String filePath) {
  xSemaphoreTake(this->lock, portMAX_DELAY);
  this->errorPlayer = "";
  xSemaphoreGive(this->lock);

  if (!this->engineOK) {
    xSemaphoreTake(this->lock, portMAX_DELAY);
    this->errorPlayer = "engine not ready";
    xSemaphoreGive(this->lock);
    return false;
  }

  if (this->isPlaying()) this->stop();
  if (filePath == "") return false;

  xSemaphoreTake(this->lock, portMAX_DELAY);
  if (filePath.endsWith("wav") || filePath.endsWith("WAV")) this->gen = new AudioGeneratorWAV();
  else if (filePath.endsWith("mp3") || filePath.endsWith("MP3")) this->gen = new AudioGeneratorMP3();
  else if (filePath.endsWith("flac") || filePath.endsWith("FLAC")) this->gen = new AudioGeneratorFLAC();
  else if (filePath.endsWith("aac") || filePath.endsWith("AAC")) this->gen = new AudioGeneratorAAC();
  this->file = new AudioFileSourceSD(filePath.c_str());
  bool isStarted = this->gen->begin(file, out);
  xSemaphoreGive(this->lock);

  if (isStarted) {
    xSemaphoreTake(this->lock, portMAX_DELAY);
    this->currentFile = filePath;
    xSemaphoreGive(this->lock);
    LOG("play: " + filePath);
  }
  else {
    LOG("not found: " + filePath);
    xSemaphoreTake(this->lock, portMAX_DELAY);
    this->errorPlayer = "not found (" + filePath + ")";
    xSemaphoreGive(this->lock);
    this->stop();
  }

  return isStarted;
}


void KESP_AUDIO::stop() {
  if (!this->engineOK) return;

  if (this->isPlaying()) {
    xSemaphoreTake(this->lock, portMAX_DELAY);
    this->gen->stop();
    this->out->stop();
    this->file->close();
    this->currentFile = "";
    this->errorPlayer = "";
    LOG("stop");
    xSemaphoreGive(this->lock);
  }
}


bool KESP_AUDIO::isPlaying() {
  xSemaphoreTake(this->lock, portMAX_DELAY);
  bool r = false;
  if (this->gen != NULL) r = this->gen->isRunning();
  xSemaphoreGive(this->lock);
  return r;
}


String KESP_AUDIO::media() {
  xSemaphoreTake(this->lock, portMAX_DELAY);
  String c = this->currentFile;
  xSemaphoreGive(this->lock);
  return c;
}


String KESP_AUDIO::error() {
  xSemaphoreTake(this->lock, portMAX_DELAY);
  String c = this->errorPlayer;
  xSemaphoreGive(this->lock);
  return c;
}


void KESP_AUDIO::midiNoteScan() {
  xSemaphoreTake(this->lock, portMAX_DELAY);

  // Init Alias array
  for (byte bank = 0; bank < MIDI_MAX_BANK; bank++)
    for (byte note = 0; note < MIDI_MAX_NOTE; note++)
      for (byte k = 0; k < MIDI_MAX_TITLE; k++)
        this->notes[bank][note][k] = 0;

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
            this->notes[i][note][0] = 1; // put a stamp even if alias is empty
            byte k = 0;
            while ( (k < MIDI_MAX_TITLE) && (entry.name()[8 + k] != 0) && (entry.name()[8 + k] != '.') ) {
              this->notes[i][note][k] = entry.name()[8 + k];
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


String KESP_AUDIO::midiNotePath(int bank, int note) {
  String path = "/" + this->pad3(bank) + "/" + this->pad3(note);
  xSemaphoreTake(this->lock, portMAX_DELAY);
  if (this->notes[bank][note][0] > 1) path += String(this->notes[bank][note]);
  xSemaphoreGive(this->lock);
  path += ".mp3";
  //if (SD.exists(path)) return path;
  //else return "";
  return path;
}


int KESP_AUDIO::midiNoteSize(byte bank, byte note) {
  int sizeN = 0;
  String path = this->midiNotePath(bank, note);
  if (SD.exists(path)) sizeN = SD.open(path).size();
  return sizeN;
}


void KESP_AUDIO::midiNoteDelete(byte bank, byte note) {
  String path = this->midiNotePath(bank, note);
  if (SD.exists(path)) {
    SD.remove(path);
    xSemaphoreTake(this->lock, portMAX_DELAY);
    this->notes[bank][note][0] = 0;
    xSemaphoreGive(this->lock);
    LOG("deleted " + path);
  }
}

String KESP_AUDIO::pad3(int input) {
  char bank[4];
  bank[3] = 0;
  bank[0] = '0' + input / 100;
  bank[1] = '0' + (input / 10) % 10;
  bank[2] = '0' + input % 10;
  return String(bank);
}
