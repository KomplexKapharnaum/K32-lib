/*
  K32_osc.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_osc_h
#define K32_osc_h

#include "Arduino.h"
#include "K32_log.h"

#include <WiFi.h>
#include <ArduinoOSC.h>


class K32_osc {
  public:
    K32_osc(int portIN, int portOUT);



  private:
    static void task( void * parameter );
    OscWiFi* osc;



};


#endif
