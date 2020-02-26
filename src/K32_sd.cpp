/*
  K32_sd.cpp
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/

#include "Arduino.h"
#include "Wire.h"
#include "SD.h"
#include "K32_sd.h"


K32_sd::K32_sd(const int SD_PIN[4]) {
  //this->lock = xSemaphoreCreateMutex();

  // Start SD
  if (!SD.exists("/")) {
    SPI.begin(SD_PIN[2], SD_PIN[1], SD_PIN[0]);
    if (SD.begin(SD_PIN[3])) LOG("SD card OK");
    else LOG("SD card ERROR");
  }

};

int K32_sd::readFile(String path, byte *buffer) {

  int maxSize = 0;
  File myFile;
  myFile = SD.open(path);
  if (myFile) {
    
    maxSize = min(int(myFile.size()), 1024*10);
    buffer = (byte*) malloc(maxSize);

    byte buffer[ maxSize ];
    
    int i = 0;
    while (myFile.available() && i < maxSize) {
      buffer[i] = myFile.read();
      i++;
    }

  }
  else LOG("ERROR: Can't open "+path);

  return maxSize;
}

