/*
  K32_audio.cpp
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/

#include "Arduino.h"
#include "Wire.h"
#include "SD.h"
#include "K32_audio.h"

#include "AudioGeneratorWAV.h"
#include "AudioGeneratorMP3.h"
#include "AudioGeneratorFLAC.h"
#include "AudioGeneratorAAC.h"


K32_audio::K32_audio() {
  LOG("AUDIO: init");

  this->lock = xSemaphoreCreateMutex();
  this->runflag = xSemaphoreCreateBinary();
  xSemaphoreGive(this->runflag);

  // Start SD
  if (SD.exists("/")) this->sdOK = true;
  else {
    LOG("AUDIO: attaching SD");
    this->sdOK = SD.begin(SS, SPI, 25000000);
  }
  if (this->sdOK) LOG("AUDIO: sd card OK");
  else LOG("AUDIO: sd card ERROR");

  // Start I2S output
  this->out = new AudioOutputI2S();
  this->out->SetPinout(25, 27, 26); //HW dependent ! BCK, LRCK, DATA
  this->out->SetGain( 1 );

  // Start PCM51xx
  bool pcmOK = true;
  this->pcm = new PCM51xx(Wire);
  Wire.begin(2, 4);
  if (this->pcm->begin(PCM51xx::SAMPLE_RATE_44_1K, PCM51xx::BITS_PER_SAMPLE_16))
      LOG("AUDIO: PCM51xx initialized successfully.");
  else
  {
    LOG("AUDIO: Failed to initialize PCM51xx.");
    uint8_t powerState = this->pcm->getPowerState();
    if (powerState == PCM51xx::POWER_STATE_I2C_FAILURE)
    {
      LOGINL("AUDIO: No answer on I2C bus at address ");
      LOG(this->pcm->getI2CAddr());
    }
    else
    {
      LOGINL("AUDIO: Power state = ");
      LOG(this->pcm->getPowerState());
      LOG("AUDIO: I2S stream must be started before calling begin()");
      LOG("AUDIO: Check that the sample rate / bit depth combination is supported.");
    }
    pcmOK = false;
  }
  if (!pcmOK) LOG("AUDIO: engine failed to start..");
  else LOG("AUDIO: engine started..");
  this->pcm->setVolume(60);

  // Start PCM engine
  this->engineOK = pcmOK;

  // Set Volume
  this->volume(100);
  this->gen = NULL;

};


bool K32_audio::isEngineOK() {
  return this->engineOK;
}

bool K32_audio::isSdOK() {
  return this->sdOK;
}


void K32_audio::setGainLimits(int min, int max) {
  xSemaphoreTake(this->lock, portMAX_DELAY);
  this->gainMin = min;
  this->gainMax = max;
  xSemaphoreGive(this->lock);
}


void K32_audio::volume(int vol)
{
  if (!this->engineOK) return;

  LOGF("AUDIO: gain = %i\n", vol);
  xSemaphoreTake(this->lock, portMAX_DELAY);
  // vol = map(vol, 0, 100, this->gainMin, this->gainMax);
  // this->pcm->setVolume(vol);
  this->out->SetGain( vol/100.0 );
  xSemaphoreGive(this->lock);
}


void K32_audio::loop(bool doLoop) {
  xSemaphoreTake(this->lock, portMAX_DELAY);
  this->doLoop = doLoop;
  xSemaphoreGive(this->lock);
}


bool K32_audio::play(String filePath) {
  xSemaphoreTake(this->lock, portMAX_DELAY);
  this->errorPlayer = "";
  xSemaphoreGive(this->lock);

  if (!this->engineOK) {
    xSemaphoreTake(this->lock, portMAX_DELAY);
    this->errorPlayer = "engine not ready";
    xSemaphoreGive(this->lock);
    return false;
  }

  this->stop();
  if (filePath == "") return false;

  xSemaphoreTake(this->lock, portMAX_DELAY);
  if (filePath.endsWith("wav") || filePath.endsWith("WAV")) this->gen = new AudioGeneratorWAV();
  else if (filePath.endsWith("mp3") || filePath.endsWith("MP3")) this->gen = new AudioGeneratorMP3();
  else if (filePath.endsWith("flac") || filePath.endsWith("FLAC")) this->gen = new AudioGeneratorFLAC();
  else if (filePath.endsWith("aac") || filePath.endsWith("AAC")) this->gen = new AudioGeneratorAAC();
  this->file = new AudioFileSourceSD(filePath.c_str());
  // this->buff = new AudioFileSourceBuffer(file, 4096);
  bool isStarted = this->gen->begin(file, out);
  xSemaphoreGive(this->lock);

  if (isStarted) {
    xSemaphoreTake(this->lock, portMAX_DELAY);
    this->currentFile = filePath;
    xSemaphoreGive(this->lock);
    LOG("AUDIO: play " + filePath);

    xSemaphoreTake(this->runflag, portMAX_DELAY);
    xTaskCreate( this->task,          // function
                  "audio_task",       // task name
                  10000,              // stack memory
                  (void*)this,        // args
                  10,                 // priority
                  NULL);              // handler
  }
  else {
    LOG("AUDIO: file not found = " + filePath);
    xSemaphoreTake(this->lock, portMAX_DELAY);
    this->errorPlayer = "not found (" + filePath + ")";
    xSemaphoreGive(this->lock);
    this->stop();
  }

  return isStarted;
}

bool K32_audio::play() {
  if (this->currentFile != "") 
    return this->play(this->currentFile);
  
  this->stop();
  return false;
}


void K32_audio::stop() {
  if (!this->engineOK) return;

  if (this->isPlaying()) {
    xSemaphoreTake(this->lock, portMAX_DELAY);
    this->gen->stop();
    this->out->stop();
    this->file->close();
    this->currentFile = "";
    this->errorPlayer = "";
    xSemaphoreGive(this->lock);
  }

  xSemaphoreTake(this->runflag, portMAX_DELAY);
  xSemaphoreGive(this->runflag);
}


bool K32_audio::isPlaying() {
  xSemaphoreTake(this->lock, portMAX_DELAY);
  bool r = false;
  if (this->gen != NULL) r = this->gen->isRunning();
  xSemaphoreGive(this->lock);
  return r;
}


String K32_audio::media() {
  xSemaphoreTake(this->lock, portMAX_DELAY);
  String c = this->currentFile;
  xSemaphoreGive(this->lock);
  return c;
}


String K32_audio::error() {
  xSemaphoreTake(this->lock, portMAX_DELAY);
  String c = this->errorPlayer;
  xSemaphoreGive(this->lock);
  return c;
}

/*
 *   PRIVATE
 */

void K32_audio::task( void * parameter ) {
  K32_audio* that = (K32_audio*) parameter;

  // play
  bool RUN = true;
  while (that->isPlaying() && RUN) {
    xSemaphoreTake(that->lock, portMAX_DELAY);
    RUN = that->gen->loop();
    xSemaphoreGive(that->lock);
    yield();
  } 

  xSemaphoreGive(that->runflag);

  if (that->doLoop) {
    LOG("AUDIO: loop");
    that->play();
  }
  else {
    LOG("AUDIO: stop");
    that->stop();
  }

  vTaskDelete(NULL);
};
