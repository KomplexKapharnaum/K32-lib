/*
  K32_audio.cpp
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/

#include "Wire.h"
#include "SD.h"
#include <SPIFFS.h>
#include "audio/K32_audio.h"

#include "AudioGeneratorWAV.h"
#include "AudioGeneratorMP3.h"
#include "AudioGeneratorFLAC.h"
#include "AudioGeneratorAAC.h"


K32_audio::K32_audio(const int AUDIO_PIN[5], const int SD_PIN[4]) {
  LOG("AUDIO: init");

  this->lock = xSemaphoreCreateMutex();
  this->runflag = xSemaphoreCreateBinary();
  xSemaphoreGive(this->runflag);

  // Start SD
  LOG("AUDIO: attaching SD");
  SPI.begin(SD_PIN[2], SD_PIN[1], SD_PIN[0]);
  this->sdOK = SD.begin(SD_PIN[3]);
  if (this->sdOK) LOG("AUDIO: sd card OK");
  else LOG("AUDIO: sd card ERROR");
  
  this->initSoundcard(AUDIO_PIN);

  // Set Volume
  this->volume(100);
  this->gen = new AudioGeneratorWAV();
  this->file = new AudioFileSourceSD();

  // Start task
  xTaskCreate( this->task,          // function
      "audio_task",       // task name
      5000,              // stack memory
      (void*)this,        // args
      20,                 // priority
      NULL);              // handler

  if(!this->isEngineOK()) {
    LOG("Audio engine failed to start..");
    // stm32->reset();
  }

  // Init sampler
  sampler = new K32_samplermidi(SD_PIN);

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
  xSemaphoreTake(this->lock, portMAX_DELAY);
  this->_volume = vol;
  xSemaphoreGive(this->lock);
  this->applyVolume();
}

void K32_audio::loop(bool doLoop) {
  xSemaphoreTake(this->lock, portMAX_DELAY);
  this->doLoop = doLoop;
  xSemaphoreGive(this->lock);
}


bool K32_audio::play(String filePath, int velocity) {
  LOG(filePath);
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
  delete this->gen;
  delete this->file;

  if (filePath.endsWith("wav") || filePath.endsWith("WAV")) this->gen = new AudioGeneratorWAV();
  else if (filePath.endsWith("mp3") || filePath.endsWith("MP3")) this->gen = new AudioGeneratorMP3();
  else if (filePath.endsWith("flac") || filePath.endsWith("FLAC")) this->gen = new AudioGeneratorFLAC();
  else if (filePath.endsWith("aac") || filePath.endsWith("AAC")) this->gen = new AudioGeneratorAAC();
  this->file = new AudioFileSourceSD(filePath.c_str());
  bool isStarted = this->gen->begin(this->file, this->out);
  this->_velocity = velocity;
  xSemaphoreGive(this->lock);

  this->applyVolume();

  // this->engine->stm32->send(K32_stm32_api::SET_LOAD_SWITCH, 1); // TODO: DiRTY HACK !

  if (isStarted) {
    xSemaphoreTake(this->lock, portMAX_DELAY);
    this->currentFile = filePath;
    xSemaphoreGive(this->lock);
    LOG("AUDIO: play " + filePath);
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
    return this->play(this->currentFile, this->_velocity);

  this->stop();
  return false;
}


void K32_audio::stop() {
  if (!this->engineOK) return;
  // this->engine->stm32->send(K32_stm32_api::SET_LOAD_SWITCH, 0); // TODO: DiRTY HACK !

  xSemaphoreTake(this->lock, portMAX_DELAY);
  this->currentFile = "";
  this->errorPlayer = "";
  xSemaphoreGive(this->lock);

  if (this->isPlaying()) {
    xSemaphoreTake(this->lock, portMAX_DELAY);
    this->gen->stop();
    this->out->stop();
    this->file->close();
    xSemaphoreGive(this->lock);
  }

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

// EXECUTE standardized command
void K32_audio::command(Orderz* order) {
  // PLAY MEDIA
  if (strcmp(order->action, "play") == 0)
  {
      if (order->count() < 1) return;
      this->play(order->getData(0)->toStr());

      if (order->count() < 2) return;
      this->volume(order->getData(1)->toInt());

      if (order->count() < 3) return;
      this->loop(order->getData(2)->toInt() > 0);
  }

  // SAMPLER NOTEON
  else if (strcmp(order->action, "noteon") == 0)
  {
      if (order->count() < 2) return;
      this->sampler->bank( order->getData(0)->toInt() );
      this->play( this->sampler->path( order->getData(1)->toInt() ) );

      if (order->count() < 3) return;
      this->volume(order->getData(2)->toInt());

      if (order->count() < 4) return;
      this->loop(order->getData(3)->toInt() > 0);
  }

  // SAMPELR NOTEOFF
  else if (strcmp(order->action, "noteoff") == 0)
  {
      if (order->count() < 1) return;
      if (this->media() == this->sampler->path( order->getData(0)->toInt()) )
      this->stop();
  }

  // STOP
  else if (strcmp(order->action, "stop") == 0)
  {
      this->stop();
  }

  // VOLUME
  else if (strcmp(order->action, "volume") == 0)
  {
      if (order->count() < 1) return;
      this->volume(order->getData(0)->toInt());
  }

  // LOOP
  else if (strcmp(order->action, "loop") == 0)
  {
      if (order->count() == 0) this->loop(true);
      else this->loop(order->getData(0)->toInt() > 0);
  }

  // UNLOOP
  else if (strcmp(order->action, "unloop") == 0)
  {
      this->loop(false);
  }

  // RAW MIDI
  else if (strcmp(order->action, "midi") == 0) {

    if (order->count() < 3) return;

    byte event = order->getData(0)->toInt() / 16;
    byte note  = order->getData(1)->toInt();
    byte velo  = order->getData(2)->toInt();

    // NOTE OFF
    if (this->noteOFF && (event == 8 || (event == 9 && velo == 0)))
    {
        if (this->media() == this->sampler->path(note))
        this->stop();
    }

    // NOTE ON
    else if (event == 9)
        this->play(this->sampler->path(note), velo);

    // CC
    else if (event == 11)
    {
        // LOOP
        if (note == 1)
        this->loop((velo > 63));

        // NOTEOFF enable
        else if (note == 2)
        this->noteOFF = (velo < 63);

        // VOLUME
        else if (note == 7)
        this->volume(velo);

        // BANK SELECT
        // else if (note == 32) this->sampler->bank(velo+1);

        // STOP ALL
        else if (note == 119 or note == 120)
        this->stop();
    }

  }
  return;
}

/*
 *   PRIVATE
 */

void K32_audio::applyVolume()
{
  if (!this->engineOK) return;

  int vol = (this->_velocity * this->_volume) / 127;

  LOGF("AUDIO: gain = %i\n", vol);
  xSemaphoreTake(this->lock, portMAX_DELAY);

  // vol = map(vol, 0, 100, this->gainMin, this->gainMax);
  // this->pcm->setVolume(vol);
  this->out->SetGain( vol/100.0 );
  xSemaphoreGive(this->lock);
}

void K32_audio::initSoundcard(const int AUDIO_PIN[5]) {

  // Start I2S output
  this->out = new AudioOutputI2S(0, AudioOutputI2S::EXTERNAL_I2S, 8, AudioOutputI2S::APLL_ENABLE);
  this->out->SetPinout(AUDIO_PIN[4], AUDIO_PIN[2], AUDIO_PIN[3]);
  this->out->SetBitsPerSample(16);
  this->out->SetRate(44100);
  this->out->SetGain( 1.0 );

  // LOG("AUDIO: Waiting for 5V");
  // delay(2000);

  // Start PCM51xx
  bool pcmOK = true;
  Wire.begin(AUDIO_PIN[0], AUDIO_PIN[1]);
  this->pcm = new PCM51xx(Wire);
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
  this->pcm->setVolume(this->gainMax);

  // Start PCM engine
  this->engineOK = pcmOK;

}

void K32_audio::task( void * parameter ) {
  K32_audio* that = (K32_audio*) parameter;
  // LOG("AUDIO: task started");

  bool RUN;
  while(true) {

    if (that->isPlaying()) {

      xSemaphoreTake(that->lock, portMAX_DELAY);
      RUN = that->gen->loop();
      xSemaphoreGive(that->lock);

      // end of file
      if (RUN) vTaskDelay(1);
      else {
        if (that->doLoop && that->currentFile != "") {
          LOG("AUDIO: loop");
          that->play();
        }
        else {
          LOG("AUDIO: stop");
          that->stop();
        }
      }
    }
    else vTaskDelay(10);

  }

  vTaskDelete(NULL);
};
