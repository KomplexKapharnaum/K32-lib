/*
  K32_audio.cpp
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/

#if HW_ENABLE_AUDIO == 1

#include "Arduino.h"
#include "Wire.h"
#include "SD.h"
#include <SPIFFS.h>
#include "K32_audio.h"

#include "AudioFileSourceSD.h"
#include "AudioFileSourceSPIFFS.h"

#include "AudioGeneratorWAV.h"
#include "AudioGeneratorMP3.h"
#include "AudioGeneratorFLAC.h"
#include "AudioGeneratorAAC.h"


K32_audio::K32_audio(K32* k32, const int AUDIO_PIN[5], const int SD_PIN[4]) : K32_plugin("audio", k32) 
{
  LOG("AUDIO: init");

  this->lock = xSemaphoreCreateMutex();
  this->runflag = xSemaphoreCreateBinary();
  xSemaphoreGive(this->runflag);

  // Start SD
  // LOGF3("AUDIO: attaching SD on pin %d %d %d", SD_PIN[2], SD_PIN[1], SD_PIN[0]);
  LOG("AUDIO: attaching SD");
  SPI.begin(SD_PIN[2], SD_PIN[1], SD_PIN[0]);
  this->sdOK = SD.begin(SD_PIN[3]);
  if (this->sdOK) {
    LOG("AUDIO: sd card OK");
    this->file = new AudioFileSourceSD();
  }
  else {
    LOG("AUDIO: sd card ERROR => Using SPIFFS instead");
    if (!SPIFFS.begin()) LOG("AUDIO: SPIFFS ERROR");
    else LOG("AUDIO: SPIFFS OK");
    this->file = new AudioFileSourceSPIFFS();
  }

  this->initSoundcard(AUDIO_PIN);

  // Set Volume
  this->volume(100);
  this->gen = new AudioGeneratorWAV();

  // Start task
  xTaskCreate( this->task,          // function
      "audio_task",       // task name
      5000,              // stack memory
      (void*)this,        // args
      20,                 // priority
      NULL);              // handler

  if(!this->isEngineOK()) {
    LOG("AUDIO: engine failed to start..");
    // stm32->reset();
  }

  // Init sampler
  // sampler = new K32_sampler(SD_PIN);

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

  if (this->sdOK) this->file = new AudioFileSourceSD(filePath.c_str());
  else this->file = new AudioFileSourceSPIFFS(filePath.c_str());
  
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

void K32_audio::command(Orderz* order) 
{
  if (strcmp(order->action, "play") == 0)
  {
    // PLAY 
    if (order->count() > 0 )
      this->play(   order->getData(0)->toStr(), 
                    (order->count() > 1) ? order->getData(1)->toInt() : 127
                  );

    // LOOP
    if (order->count() > 2)
      this->loop(order->getData(2)->toInt() == 1);

  }
  else if (strcmp(order->action, "stop") == 0)
  {
    this->stop();
  }
  else if (strcmp(order->action, "volume") == 0)
  {
    if (order->count() > 0 )
      this->volume(order->getData(0)->toInt());
  }
  else if (strcmp(order->action, "loop") == 0)
  {
    if (order->count() > 0 )
      this->loop(order->getData(0)->toInt() == 1);
  }
  else if (strcmp(order->action, "noteon") == 0)
  {
    if (sampler);
  }
  else if (strcmp(order->action, "noteoff") == 0)
  {
    if (sampler);
    // if (this->audio->media() == this->audio->sampler->path(atoi(note)))
    //     this->audio->stop();
  }
  
};

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
  // this->pcm->setVolume(vol);           // ERROR: we do loose I2C comm once playback started !!
  this->out->SetGain( vol/100.0 );
  xSemaphoreGive(this->lock);
}

void K32_audio::initSoundcard(const int AUDIO_PIN[5]) {

  // Start I2S output
  this->out = new AudioOutputI2S();
  // this->out = new AudioOutputI2S(0, AudioOutputI2S::EXTERNAL_I2S, 8, AudioOutputI2S::APLL_ENABLE);
  // LOGF3("AUDIO: init I2S on pins %i, %i, %i\n", AUDIO_PIN[2], AUDIO_PIN[3], AUDIO_PIN[4]);
  this->out->SetPinout(AUDIO_PIN[4], AUDIO_PIN[2], AUDIO_PIN[3]);

  this->out->SetBitsPerSample(16);
  this->out->SetRate(44100);
  this->out->SetGain( 1.0 );

  // LOG("AUDIO: Waiting for 5V");

  // Start PCM51xx
  bool pcmOK = true;

  // I2C init
  LOGF2("AUDIO: init I2C on pins %i, %i\n", AUDIO_PIN[0], AUDIO_PIN[1]);
  Wire.begin(AUDIO_PIN[0], AUDIO_PIN[1]);

  // ////////////////////  WIRE SCAN
  // Serial.println("Scanning...");

  // byte error, address;
  // int nDevices = 0;

  // for(address = 1; address < 127; address++ ) 
  // {
  //   Wire.beginTransmission(address);
  //   error = Wire.endTransmission();

  //   if (error == 0)
  //   {
  //     Serial.print("I2C device found at address 0x");
  //     if (address<16) 
  //       Serial.print("0");
  //     Serial.print(address,HEX);
  //     Serial.println("  !");

  //     nDevices++;
  //   }
  //   else if (error==4) 
  //   {
  //     Serial.print("Unknown error at address 0x");
  //     if (address<16) 
  //       Serial.print("0");
  //     Serial.println(address,HEX);
  //   }    
  // }
  // if (nDevices == 0)
  //   Serial.println("No I2C devices found\n");
  // else
  //   Serial.println("done\n");

  // //////////////////////////////////////

  this->pcm = new PCM51xx(Wire);

  // Ping PCM51xx
  Wire.beginTransmission(this->pcm->getI2CAddr());
  if( Wire.endTransmission() == 0) {
    LOG("AUDIO: PCM51xx found.");
  } else {
    LOG("AUDIO: PCM51xx not found.");
    pcmOK = false;
  }

  if (this->pcm->begin(PCM51xx::SAMPLE_RATE_44_1K, PCM51xx::BITS_PER_SAMPLE_16))
      LOG("AUDIO: PCM51xx initialized successfully.");
  else
  {
    uint8_t powerState = this->pcm->getPowerState();
    if (powerState == PCM51xx::POWER_STATE_I2C_FAILURE)
    {
      LOGINL("AUDIO: No answer on I2C bus at address ");
      LOG(this->pcm->getI2CAddr());
      pcmOK = false;
    }
    else
    {
      LOGINL("AUDIO: Power state = ");
      LOG(this->pcm->getPowerState());
      LOG("AUDIO: I2S stream must be started before calling begin()");
      LOG("AUDIO: Check that the sample rate / bit depth combination is supported.");
    }
  }
  if (!pcmOK) LOG("AUDIO: Failed to initialize PCM51xx.");
  else LOG("AUDIO: ready.");
  this->pcm->setVolume(this->gainMax);

  // Start PCM engine
  this->engineOK = pcmOK;

}

void K32_audio::task( void * parameter ) {
  K32_audio* that = (K32_audio*) parameter;
  // LOG("AUDIO: task started");

  bool RUN;
  while(that->engineOK) {

    if (that->isPlaying()) {

      xSemaphoreTake(that->lock, portMAX_DELAY);
      RUN = that->gen->loop();
      xSemaphoreGive(that->lock);

      if (RUN) vTaskDelay(1);
      // end of file
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
    else {
      vTaskDelay(10);
      // LOGF("AUDIO: Power state = %i\n", that->pcm->getPowerState());
    }
  }

  vTaskDelete(NULL);
};

#endif