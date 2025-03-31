/*
  K32_dipswitch.cpp
  Created by RIRI, march 2025.
  Released under GPL v3.0
*/

#include "K32_version.h"
#include "K32_dipswitch.h"

// #include <ESPmDNS.h>


/*
 *   PUBLIC
 */

K32_dipswitch::K32_dipswitch(K32* k32) : K32_plugin("dips", k32) {

    this->lock = xSemaphoreCreateMutex();

    for(int k=0; k<DIP_SLOTS; k++) this->watchPins[k] = 0;

    xTaskCreate( this->watch,          // function
                  "dips_watch",         // server name
                  1500,              // stack memory
                  (void*)this,        // args
                  0,                  // priority
                  NULL              // handler
                  );                // core 
}

void K32_dipswitch::add(int pin, String name) 
{   
    for(int k=0; k<=DIP_SLOTS; k++) {
        if (k==DIP_SLOTS) {LOG("dips: no more slots.."); return;}   // No more slots

        if (this->watchPins[k]==0) 
        {
            pinMode(pin, INPUT_PULLUP);
            this->watchValues[k] = HIGH;
            this->watchNames[k]= name;
            this->watchPins[k] = pin;
            break;
        }
    }
}

void K32_dipswitch::add(int pin) {
    this->add(pin, String(pin));
}


void K32_dipswitch::command(Orderz* order) {
  // TODO: receive commands ?
}


/*
 *   PRIVATE
 */

void K32_dipswitch::watch( void * parameter ) {
    K32_dipswitch* that = (K32_dipswitch*) parameter;
    TickType_t xFrequency = pdMS_TO_TICKS(10);

    while(true) 
    { 
        for(int k=0; k<DIP_SLOTS; k++) 
        {
            if (that->watchPins[k]>0) {
                int value = analogRead(that->watchPins[k]);
                that -> emit( "dips/"+that->watchNames[k]+"value: "+value);
            }
        }

      vTaskDelay( xFrequency );
    }

    vTaskDelete(NULL);
}

