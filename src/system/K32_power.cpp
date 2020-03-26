/*
  K32_remote.cpp
  Created by Clement GANGNEUX, june 2019.
  Released under GPL v3.0
*/



#include "Arduino.h"
#include "K32_power.h"
#include <Preferences.h>
Preferences power_prefs; 


/*
 *   PUBLIC
 */

 K32_power::K32_power(K32_stm32* stm32, const int CURRENT_SENSOR_PIN) {
   LOG("POWER : init");

   this->lock = xSemaphoreCreateMutex();
   this->_stm32 = stm32 ;
   this->charge = false;
   this -> currentPin = CURRENT_SENSOR_PIN; 

       // Set Current factor depending on HW specifications
    if (CURRENT_SENSOR_TYPE == 10) {
      this->currentFactor = 60 ;
    } else if (CURRENT_SENSOR_TYPE == 25)
    {
      this->currentFactor = 24;
    } else if (CURRENT_SENSOR_TYPE == 11)
    {
      this->currentFactor = 110 ;
    }

     // Set Current offset
     this->_lock(); 
     power_prefs.begin("k32-power", false);
    this->_unlock(); 
    #ifdef SET_CURRENT_OFFSET
      this->_lock(); 
      int old = power_prefs.getUInt("offset", 0);
      if (this->currentOffset != old) power_prefs.putUInt("offset", SET_CURRENT_OFFSET);
      this->_unlock(); 
    #else 
      this->_lock(); 
      this->currentOffset = power_prefs.getUInt("offset", 0);
      this->_unlock(); 
    #endif




   // Start main task
   xTaskCreate( this->task,
                 "power_task",
                 5000,
                 (void*)this,
                 0,              // priority
                 &t_handle);
 };


 void K32_power::_lock()
 {
   //LOG("lock");
   xSemaphoreTake(this->lock, portMAX_DELAY);
 }

 void K32_power::_unlock()
 {
   //LOG("unlock");
   xSemaphoreGive(this->lock);
 }

 int K32_power::power() {
   if (CURRENT_SENSOR_TYPE != 0)
   {
     this->_lock();
     this-> _power = this->_current * this->_stm32->voltage() / 1000;  // Power in mW
     this->_unlock();
   } else
   {
     this->_lock();
     this-> _power = this->_stm32->current() * this->_stm32->voltage() / 1000;  // Power in mW
     this->_unlock();   }
   return this->_power;
 }

 int K32_power::energy() {
   return this->_energy; // TODO !
 }

 int K32_power::current() {
   if (CURRENT_SENSOR_TYPE != 0)
   {
      return this->_current;
   } else 
   {
     return this->_stm32->current(); 
   }
 }

 void K32_power::calibrate() {
   LOG("POWER : Calibration of current sensor"); 
   this->_lock();  
   this-> currentOffset = this->currentOffset + this->_current * this->currentFactor / 1000 ; 
   power_prefs.putUInt("offset", this-> currentOffset); 
   this->_unlock(); 
 }



 /*
  *   PRIVATE
  */

  void K32_power::task(void * parameter) 
  {

    K32_power* that = (K32_power*) parameter;
    TickType_t xFrequency = pdMS_TO_TICKS(POWER_CHECK);
    int counter = 0;
    int currentMeas = 0; 



    while(true) {

      /* Check Current Value */
      that->_lock(); 
      if (CURRENT_SENSOR_TYPE != 0 )
      {

 /* Averaging*/
        if (counter == 50)
        {
          that-> _current = currentMeas / 50;
          that-> _current = (that->_current - that->currentOffset ) *1000 / that->currentFactor ; // Curent in mA
          counter = 0;
          currentMeas = 0;
        }
        currentMeas = currentMeas + analogRead(that->currentPin) ;
        counter ++;

      } else
      {
        /* Do nothing... */
      }


        if(that->_current > 0)
        {
          that->charge = true;
        } else if (that->_current < -100)
        {
          that->charge = false;
        }


        that->SOC = that->_stm32->battery();
      that->_unlock(); 

      vTaskDelay( xFrequency );
    }
  }
