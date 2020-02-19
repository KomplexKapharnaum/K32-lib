/*
  K32_remote.cpp
  Created by Clement GANGNEUX, february 2020.
  Released under GPL v3.0
*/

#include "Arduino.h"
#include "K32_remote.h"

/* Definition of GPÏO Ports */
#if HW_REVISION == 1

#elif HW_REVISION == 2

#else
  #error "HW_REVISION undefined or invalid. Should be 1 or 2"
#endif
/**********/

K32_remote::K32_remote() {
  LOG("REMOTE: init");

  this->lock = xSemaphoreCreateMutex();

  // Start task
  xTaskCreate( this->task,          // function
      "remote_task",       // task name
      1000,              // stack memory
      (void*)this,        // args
      3,                 // priority
      NULL);              // handler
};

/*
 *   PRIVATE
 */

 void K32_remote::task(void * parameter) {
   K32_remote* that = (K32_remote*) parameter;
   TickType_t xFrequency = pdMS_TO_TICKS(REMOTE_CHECK);

   while(true) {
     /* Main loop */

     /********/


     vTaskDelay( xFrequency );

   }
 }
