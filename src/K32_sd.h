/*
  K32_sd.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_sd_h
#define K32_sd_h

#include "Arduino.h"
#include "K32_log.h"

class K32_sd {
  public:
    K32_sd(const int SD_PIN[4]);
    
    int readFile(String path, byte *buffer);


  private:
    //SemaphoreHandle_t lock;
    //static void task( void * parameter );

};

#endif
