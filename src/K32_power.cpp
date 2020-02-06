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
   if (CURRENT_SENSOR_TYPE != 0)
   {
     this-> _power = this->_current * this->_stm32->voltage() / 1000;  // Power in mW
   } else
   {
     /* Do nothing */
   }
   return this->_power;
 }

 int K32_power::energy() {
   return this->_energy;
 }

 int K32_power::current() {
   return this->_current;
 }

 void K32_power::set_power(int current, int battery, int power) {
   this->charge = false;  // Gauge inside the installation
   this->SOC = battery ;
   this->_current = current ;
   this->_power = power ;
 }


 void K32_power::set_demo() {
   if (this->demo)
   {
     this->demo = false;
   } else {
     this->demo = true ;
   }
 }



 /*
  *   PRIVATE
  */

  void K32_power::task(void * parameter) {
    K32_power* that = (K32_power*) parameter;
    TickType_t xFrequency = pdMS_TO_TICKS(POWER_CHECK);
    int current_factor = 0 ;
    unsigned long currentTime=millis();


    that->SOC = that->_stm32->battery();
    if (CURRENT_SENSOR_TYPE == 10) {
      current_factor = 60 ;
    } else if (CURRENT_SENSOR_TYPE == 25)
    {
      current_factor = 24;
    } else if (CURRENT_SENSOR_TYPE == 11)
    {
      current_factor = 110 ;
    } else if (CURRENT_SENSOR_TYPE == 1) // Model
    {
      current_factor = 100 ;
    }



    while(true) {

      /* Check Current Value */
      xSemaphoreTake(that->lock, portMAX_DELAY);
      if (CURRENT_SENSOR_TYPE != 0 )
      {
        that->_current = 0 ;
        for (int i = 0; i < 50 ; i ++) // Averaging on 50 values
        {
          that->_current = that->_current + analogRead(CURRENT_SENSOR_PORT) ;
        }
        that->_current = that->_current / 50 ;
        that-> _current = (that->_current - CURRENT_CALIB ) *1000 / current_factor ; // Curent in mA

        //that->_current = analogRead(CURRENT_SENSOR_PORT)-*20 ;


      } else
      {
        /* Do nothing... */
      }

      xSemaphoreGive(that->lock);

      // if (that->_current > 400)
      // {
      //   that->charge = true;
      // } else if (that->_current < -400)
      // {
      //   that->charge = false;
      // }

      if ((!that->demo) && (CURRENT_SENSOR_TYPE != 0))
      {
        if(that->_current > -100)
        {
          that->charge = true;
        } else if (that->_current < -200)
        {
          that->charge = false;
        }

        if (CURRENT_SENSOR_TYPE == 1) {
            if(millis() - currentTime > 50000)
            {

              that->SOC ++;
              if (that->SOC>100)
              {
                that->SOC = 45 ;
              }
              currentTime=millis();
            }
        }

        // if (MODEL_VERSION) {
        //   if(millis() - currentTime > 5000)
        //   {
        //
        //     that->SOC ++;
        //     if (that->SOC>100)
        //     {
        //       that->SOC = 45 ;
        //     }
        //     currentTime=millis();
        //   }
        //
        //
        // }

        //that->charge = true;
        // if (that->SOC<that->_stm32->battery())
        // {
        //   that->charge = true;
        // }else if(that->SOC>that->_stm32->battery())
        // {
        //   that->charge = false;
        // }

        //that->SOC = that->_stm32->battery();
      } else if (CURRENT_SENSOR_TYPE == 0)
      {
        /* Do nothing */
      }else
      {
        if(millis() - currentTime > 500)
        {
          if (that->charge)
          {
            that->SOC ++;
            if (that->SOC >= 100)
            {
              that-> charge = false;
            }
          } else
          {
            that->SOC -- ;
            if (that->SOC <= 0)
            {
              that-> charge = true;
            }
          }
          currentTime=millis();
        }
      }





      vTaskDelay( xFrequency );
    }
  }
