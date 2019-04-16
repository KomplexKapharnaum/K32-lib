/*
  K32_osc.cpp
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/

#include "Arduino.h"
#include "K32_osc.h"

#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <WiFi.h>


/*
 *   PUBLIC
 */

K32_osc::K32_osc(int portIN, int portOUT) {
  this->portIN = portIN;
  this->portOUT = portOUT;

  this->osc = new OscWifi();
  this->osc->begin(portIN);

  // LOOP task
  xTaskCreate( this->task,          // function
                "osc_task",         // task name
                10000,              // stack memory
                (void*)this,        // args
                4,                  // priority
                NULL);              // handler

  // SUBSCRIBE
  this->osc->subscribe("/ping", [](OscMessage& m)
    {
        LOG("RECV: /ping");
        this->osc->send(host, send_port, "/pong");
        LOG("SEND: /pong");
    });

};






/*
 *   PRIVATE
 */



 void K32_osc::task( void * parameter ) {
   K32_osc* that = (K32_osc*) parameter;
   TickType_t xFrequency = pdMS_TO_TICKS(1);

   while(true) {
     that->osc->parse();
     vTaskDelay( xFrequency );
   }

   vTaskDelete(NULL);
 }
