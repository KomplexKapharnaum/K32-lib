/*
  K32_power.cpp
  Created by Clement GANGNEUX, june 2019.
  Released under GPL v3.0
*/

#include "Arduino.h"
#include "K32_power.h"


/*
 *   PUBLIC
 */

 K32_power::K32_power(K32_stm32* stm32) {
   this->lock = xSemaphoreCreateMutex();
   this->_stm32 = stm32 ;
   this->charge = false;
 };


 void K32_power::start() {



   xTaskCreate( this->task,
                 "power_task",
                 1000,
                 (void*)this,
                 1,              // priority
                 &t_handle);
  running = true;
 }

 void K32_power::stop() {
   if (running)
   {
     vTaskDelete(this->t_handle);
     running = false;
   }
 }

 int K32_power::power() {
   return 3;
 }



 /*
  *   PRIVATE
  */

  void K32_power::task(void * parameter) {
    K32_power* that = (K32_power*) parameter;
    TickType_t xFrequency = pdMS_TO_TICKS(POWER_CHECK);

    that->SOC = that->_stm32->battery();



    while(true) {

      if (that->SOC<that->_stm32->battery())
      {
        that->charge = true;
      }else if(that->SOC>that->_stm32->battery())
      {
        that->charge = false;
      }

      that->SOC = that->_stm32->battery();

      LOGF("SOC : %d \n", that->SOC);


      vTaskDelay( xFrequency );
    }
  }
