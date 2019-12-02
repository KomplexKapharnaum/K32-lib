/*
  K32_samplermidi.cpp
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/

#include "Arduino.h"
#include "Wire.h"
#include "SD.h"
#include "K32_samplermidi.h"


#if HW_REVISION == 1
  const uint8_t I2C_SDA_PIN = 2;
  const uint8_t I2C_SCL_PIN = 4;
  const uint8_t I2S_LRCK_PIN = 27;
  const uint8_t I2S_DATA_PIN = 26;
  const uint8_t I2S_BCK_PIN = 25;
  const uint8_t SD_DI_PIN = 23;
  const uint8_t SD_DO_PIN = 19;
  const uint8_t SD_SCK_PIN = 18;
  const uint8_t SD_CS_PIN = 5;

#elif HW_REVISION == 2
  const uint8_t I2C_SDA_PIN = 32;
  const uint8_t I2C_SCL_PIN = 33;
  const uint8_t I2S_LRCK_PIN = 25;
  const uint8_t I2S_DATA_PIN = 26;
  const uint8_t I2S_BCK_PIN = 27;
  const uint8_t SD_DI_PIN = 19;
  const uint8_t SD_DO_PIN = 5;
  const uint8_t SD_SCK_PIN = 18;
  const uint8_t SD_CS_PIN = 21;

#else
  #error "HW_REVISION undefined or invalid. Should be 1 or 2"
#endif


K32_samplermidi::K32_samplermidi() {
  this->lock = xSemaphoreCreateMutex();

  // Start SD
  if (!SD.exists("/")) {
    SPI.begin(SD_SCK_PIN, SD_DO_PIN, SD_DI_PIN);
    if (SD.begin(SD_CS_PIN)) LOG("SD card OK");
    else LOG("SD card ERROR");
  }

  // Scan samples
  this->scan();
};


void K32_samplermidi::scan() {
  // Checking task
  xTaskCreate( this->task,
                "sampler_task",
                40000,
                (void*)this,
                1,                // priority
                NULL);
}


String K32_samplermidi::path(int note, int bank) {
  xSemaphoreTake(this->lock, portMAX_DELAY);
  if (bank < 0) bank = this->_bank;
  String path = "/" + this->pad3(bank) + "/" + this->pad3(note);
  if (this->samples[bank][note][0] > 1) path += String(this->samples[bank][note]);
  xSemaphoreGive(this->lock);
  if (SD.exists(path))
    return path;

  LOGF("SAMPLER: %s not found..\n", path.c_str());
  return "";
}

void K32_samplermidi::bank(int bank) {
  xSemaphoreTake(this->lock, portMAX_DELAY);
  this->_bank = bank;
  xSemaphoreGive(this->lock);
}

int K32_samplermidi::bank() {
  int b = 0;
  xSemaphoreTake(this->lock, portMAX_DELAY);
  b = this->_bank;
  xSemaphoreGive(this->lock);
  return b;
}

int K32_samplermidi::size(byte note, byte bank) {
  int sizeN = 0;
  String path = this->path(note, bank);
  if (SD.exists(path)) sizeN = SD.open(path).size();
  return sizeN;
}


void K32_samplermidi::remove(byte note, byte bank) {
  String path = this->path(note, bank);
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



/*
 *   PRIVATE
 */

void K32_samplermidi::task( void * parameter ) {
  K32_samplermidi* that = (K32_samplermidi*) parameter;
  TickType_t xFrequency = pdMS_TO_TICKS(5);

  xSemaphoreTake(that->lock, portMAX_DELAY);

  // Init Alias array
  for (byte bank = 0; bank < MIDI_MAX_BANK; bank++)
    for (byte note = 0; note < MIDI_MAX_NOTE; note++)
      for (byte k = 0; k < MIDI_MAX_TITLE; k++)
        that->samples[bank][note][k] = 0;

  // // Check Bank dirs
  LOG("\nSAMPLER: Scanning...");
  for (byte i = 0; i < MIDI_MAX_BANK; i++) {
    if (SD.exists("/" + that->pad3(i))) {
      LOG("  - scanning bank " + that->pad3(i));
      File dir = SD.open("/" + that->pad3(i));

      // Check Notes files
      while (true) {
        File entry =  dir.openNextFile();
        if (!entry) break;
        if (!entry.isDirectory()) {
          int note = 0;
          note = (entry.name()[5] - '0') * 100 + (entry.name()[6] - '0') * 10 + (entry.name()[7] - '0');
          if (note < MIDI_MAX_NOTE) {
            that->samples[i][note][0] = 1; // put a stamp even if alias is empty
            byte k = 0;
            while ( (k < MIDI_MAX_TITLE) && (entry.name()[8 + k] != 0) ) {
              that->samples[i][note][k] = entry.name()[8 + k];
              k++;
            }
          }
          vTaskDelay( xFrequency );
          // LOG("File found: "+String(entry.name())+" size="+String(entry.size()));
        }
      }
    }
  }
  xSemaphoreGive(that->lock);
  LOG("SAMPLER: ready.");

  vTaskDelete(NULL);
};
