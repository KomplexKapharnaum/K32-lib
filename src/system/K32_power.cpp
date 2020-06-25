/*
  K32_power.cpp
  Created by Clement GANGNEUX, april 2020.
  Released under GPL v3.0
*/

#include "Arduino.h"
#include "K32_power.h"
#include <Preferences.h>
Preferences power_prefs;

/*
 *   PUBLIC
 */

K32_power::K32_power(K32_stm32 *stm32, bool autoGauge ,const int CURRENT_SENSOR_PIN)
{
  LOG("POWER : init");

  this->lock = xSemaphoreCreateMutex();
  this->_stm32 = stm32;
  this->autoGauge = autoGauge ; 
  this->charge = false;
  this->currentPin = CURRENT_SENSOR_PIN;

  // Set Current factor depending on HW specifications
  if (CURRENT_SENSOR_TYPE == 10)
  {
    this->currentFactor = 60;
  }
  else if (CURRENT_SENSOR_TYPE == 25)
  {
    this->currentFactor = 24;
  }
  else if (CURRENT_SENSOR_TYPE == 11)
  {
    this->currentFactor = 110;
  }

  // Set Current offset
  this->_lock();
  power_prefs.begin("k32-power", false);
  this->_unlock();

  this->_lock();
  this->currentOffset = power_prefs.getUInt("offset", CURRENT_OFFSET);
  //this->batteryRint = power_prefs.getUInt("rint", BATTERY_RINT);
  power_prefs.end(); 
  this->_unlock();

  // Start main task
  xTaskCreate(this->task,
              "power_task",
              5000,
              (void *)this,
              0, // priority
              &t_handle); 
};


int K32_power::power()
{
  if (CURRENT_SENSOR_TYPE != 0)
  {
    this->_lock();
    this->_power = this->_current * this->_stm32->voltage() / 1000; // Power in mW
    this->_unlock();
  }
  else
  {
    this->_lock();
    this->_power = this->_stm32->current() * this->_stm32->voltage() / 1000; // Power in mW
    this->_unlock();
  }
  return this->_power;
}

int K32_power::energy()
{
  return this->_energy; // TODO !
}

int K32_power::current()
{
  if (CURRENT_SENSOR_TYPE != 0)
  {
    return this->_current;
  }
  else
  {
    return this->_stm32->current();
  }
}

void K32_power::calibrate(calibType type)
{
  int currentMeas = 0 ; 
  int voltageMeas = 0; 
  //int rintMeas = 0.14 ; 
  if( this->_error)
  {
    LOG("POWER : Error with current sensor, calibration is impossible"); 
  } else 
  {
    if (type == Offset ) 
    {
      LOG("POWER : Calibration of current sensor offset");
      this->_lock();
      // Reading current value
      currentMeas = analogRead(this->currentPin);
      this->_current = (currentMeas - this->currentOffset) * 1000 / this->currentFactor; // Curent in mA

      this->currentOffset = this->currentOffset + this->_current * this->currentFactor / 1000;
      voltageMeas = this->_stm32->voltage(); 
      this->calibVoltage = voltageMeas; 
      this->_unlock(); 

      this->_lock(); 
      power_prefs.begin("k32-power", false);
      power_prefs.putUInt("offset", this->currentOffset);
      power_prefs.end() ; 
      this->_unlock();
    } else if (type == InternalRes) 
    {
      /* INTERNAL RES FUNCTION TO REWORK */ 
      // if (this->calibVoltage == 0)
      // {
      //   LOG("POWER : Error with calibration voltage value, calibration is impossible");
      //   this->batteryRint = BATTERY_RINT ; 
      //   power_prefs.putUInt("rint", this->batteryRint); 
      // } else
      // {
      //   LOG("POWER : Calibration of battery internal resistance");
      //   this->_lock() ; 
      //   // Reading Current Value
      //   currentMeas = analogRead(this->currentPin);
      //   this->_current = (currentMeas - this->currentOffset) * 1000 / this->currentFactor; // Curent in mA
      //   voltageMeas = this->_stm32->voltage();
      //   if (abs(voltageMeas - this->calibVoltage) < 300) // Check if voltage value is significantly different
      //   {
      //     this->batteryRint = BATTERY_RINT ; 
      //     power_prefs.putUInt("rint", this->batteryRint);
      //     LOG("POWER : Error current is too low, calibration is impossible"); 
      //     return; 
      //   }
      //   rintMeas = (this->calibVoltage - voltageMeas) / this->_current; 
      //   LOG(rintMeas); 
      //   if ((rintMeas<0)||(rintMeas > 1)) 
      //   {
      //     this->batteryRint = BATTERY_RINT ; 
      //     power_prefs.putUInt("rint", this->batteryRint);
      //     LOG("POWER : Error with rint value, calibration is impossible"); 
      //     return;  
      //   }
      //   this->batteryRint = rintMeas ; 
      //   power_prefs.putUInt("rint", this->batteryRint);
      //   LOGF("Rint: %d\n", this->batteryRint); 
      //   this->calibVoltage = 0 ; // Set back calibration voltage
      //   this->_unlock() ; 
      //}
    }
  }
}

void K32_power::setAdaptiveGauge(bool adaptiveOn, batteryType type, int nbOfCell)
{
  if (adaptiveOn)
    LOG("POWER : Switch on adaptive gauge");
  else
    LOG("POWER : Switch off adaptive gauge");

  if (nbOfCell == 0)
  {
    /* Auto determination of number of Cell*/
    int v = this->_stm32->voltage();
    switch (type)
    {
    case LIPO: //LiPo
      v = this->_stm32->voltage();
      nbOfCell = findCellCount(v, LIPO_VOLTAGE_BREAKS[0], LIPO_VOLTAGE_BREAKS[6]);
      break;
    case LIFE: // Life
      v = this->_stm32->voltage();
      nbOfCell = findCellCount(v, LIFE_VOLTAGE_BREAKS[0], LIFE_VOLTAGE_BREAKS[6]);
      break;
    }

    LOGF("POWER : Auto nb of cell : %d", nbOfCell);
  }
  

 // Set to default before switch off adaptive gauge
  if (this->_error)
  {
    this->_lock();
    this->_stm32->custom(LIPO_ERROR_BREAKS[0] * nbOfCell,
                        LIPO_ERROR_BREAKS[1] * nbOfCell,
                        LIPO_ERROR_BREAKS[2] * nbOfCell,
                        LIPO_ERROR_BREAKS[3] * nbOfCell,
                        LIPO_ERROR_BREAKS[4] * nbOfCell,
                        LIPO_ERROR_BREAKS[5] * nbOfCell,
                        LIPO_ERROR_BREAKS[6] * nbOfCell);
    this->_unlock();
  }
  else if (type == LIPO)
  {
    this->_lock();
    this->_stm32->custom(LIPO_ERROR_BREAKS[0] * nbOfCell,
                        LIPO_VOLTAGE_BREAKS[1] * nbOfCell,
                        LIPO_VOLTAGE_BREAKS[2] * nbOfCell,
                        LIPO_VOLTAGE_BREAKS[3] * nbOfCell,
                        LIPO_VOLTAGE_BREAKS[4] * nbOfCell,
                        LIPO_VOLTAGE_BREAKS[5] * nbOfCell,
                        LIPO_VOLTAGE_BREAKS[6] * nbOfCell);
    this->_unlock();
  }
  else if (type == LIFE)
  {
    this->_lock(); 
    this->_stm32->custom(LIFE_VOLTAGE_BREAKS[0] * nbOfCell,
                        LIFE_VOLTAGE_BREAKS[1] * nbOfCell,
                        LIFE_VOLTAGE_BREAKS[2] * nbOfCell,
                        LIFE_VOLTAGE_BREAKS[3] * nbOfCell,
                        LIFE_VOLTAGE_BREAKS[4] * nbOfCell,
                        LIFE_VOLTAGE_BREAKS[5] * nbOfCell,
                        LIFE_VOLTAGE_BREAKS[6] * nbOfCell);
    this->_unlock(); 
  }

           
  this->_lock();
  this->adaptiveGaugeOn = adaptiveOn;
  this->nbOfCell = nbOfCell;
  this->battType = type;
  this->_unlock();
}


/*
  *   PRIVATE
  */

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

/* Determine the number of cells based on the battery voltage and the given min and max cell voltages.
 * Return 0 if the voltage doesn't match any number of cells.
 *
 * 2, 3, 4 and 7 cells are supported.
 */
uint8_t K32_power::findCellCount(unsigned int voltage, unsigned int cellMin, unsigned int cellMax)
{
  for (int i = 2; i <= 7; i++)
  {
    if (i == 5 || i == 6)
      continue;

    if ((voltage > i * cellMin) && (voltage <= i * (cellMax + INITIAL_CELL_VOLTAGE_TOLERANCE)))
    {
      return i;
    }
  }
  return 0;
}

void K32_power::updateCustom(void *parameter)
{
    K32_power *that = (K32_power *)parameter;
    // LOGF("POWER update : Current %d mA", that->_current); 

    if(abs(that->_current)>20000) 
    {
      LOGF("POWER ERROR : problem with current sensor value %d mA", that->_current); 
      that->_lock(); 
      that->_error = true;
      that->_unlock(); 
      that->setAdaptiveGauge(false, that->battType, that->nbOfCell);
      return; 
    }

    if (that->_current > 300) // Discharging batteries
    {
      if (abs(that->_current - that->currentRecord) > 2500 ) // If current changed significantly
      {
        /* Set constant cut off value */ 
        if (that->battType == LIPO)
          {
            that->profile[0] = LIPO_ERROR_BREAKS[0]*that->nbOfCell; 
          }
        else if (that->battType == LIFE)
          {
            that->profile[0] = LIFE_VOLTAGE_BREAKS[0]*that->nbOfCell; 
          }

        /* Update profile according to current value */
        for (int i = 1; i<7; i++)
        {
          if (that->battType == LIPO)
          {
            that->profile[i] = LIPO_VOLTAGE_BREAKS[i]*that->nbOfCell - BATTERY_RINT * that->_current ; 
          }
          else if (that->battType == LIFE)
          {
            that->profile[i] = LIFE_VOLTAGE_BREAKS[i]*that->nbOfCell - BATTERY_RINT * that->_current  ; 
          }
        }
        /* Update custom profile */ 
        that->_stm32->custom(that->profile[0],
                            that->profile[1],
                            that->profile[2],
                            that->profile[3],
                            that->profile[4],
                            that->profile[5], 
                            that->profile[6]);
        /* Update value of operating profile */
        that->currentRecord = that->_current ;
        LOGF("POWER : new profile with current : %d mA \n", that->currentRecord ); 
 

      }
    } else if (that->_current < -300) // Charging batteries
    {
      if(that->currentRecord>0)
      {
          /* Switch profile to default */ 
        if (that->battType == LIPO)
        {
          that->_stm32->custom(LIPO_ERROR_BREAKS[0] * that->nbOfCell,
                              LIPO_VOLTAGE_BREAKS[1] * that->nbOfCell,
                              LIPO_VOLTAGE_BREAKS[2] * that->nbOfCell,
                              LIPO_VOLTAGE_BREAKS[3] * that->nbOfCell,
                              LIPO_VOLTAGE_BREAKS[4] * that->nbOfCell,
                              LIPO_VOLTAGE_BREAKS[5] * that->nbOfCell,
                              LIPO_VOLTAGE_BREAKS[6] * that->nbOfCell);
        }
        else if (that->battType == LIFE)
        {
          that->_stm32->custom(LIFE_VOLTAGE_BREAKS[0] * that->nbOfCell,
                              LIFE_VOLTAGE_BREAKS[1] * that->nbOfCell,
                              LIFE_VOLTAGE_BREAKS[2] * that->nbOfCell,
                              LIFE_VOLTAGE_BREAKS[3] * that->nbOfCell,
                              LIFE_VOLTAGE_BREAKS[4] * that->nbOfCell,
                              LIFE_VOLTAGE_BREAKS[5] * that->nbOfCell,
                              LIFE_VOLTAGE_BREAKS[6] * that->nbOfCell);
        }

        LOG("POWER: Battery charging, Switched to default profile");


      }
      that->currentRecord = -1 ; // Negative value of current

    } else // Current near to zero
    {
      if(that->currentRecord > 0) 
      {
        /* Switch profile to default */ 
        if (that->battType == LIPO)
        {
          that->_stm32->custom(LIPO_ERROR_BREAKS[0] * that->nbOfCell,
                              LIPO_VOLTAGE_BREAKS[1] * that->nbOfCell,
                              LIPO_VOLTAGE_BREAKS[2] * that->nbOfCell,
                              LIPO_VOLTAGE_BREAKS[3] * that->nbOfCell,
                              LIPO_VOLTAGE_BREAKS[4] * that->nbOfCell,
                              LIPO_VOLTAGE_BREAKS[5] * that->nbOfCell,
                              LIPO_VOLTAGE_BREAKS[6] * that->nbOfCell);
        }
        else if (that->battType == LIFE)
        {
          that->_stm32->custom(LIFE_VOLTAGE_BREAKS[0] * that->nbOfCell,
                              LIFE_VOLTAGE_BREAKS[1] * that->nbOfCell,
                              LIFE_VOLTAGE_BREAKS[2] * that->nbOfCell,
                              LIFE_VOLTAGE_BREAKS[3] * that->nbOfCell,
                              LIFE_VOLTAGE_BREAKS[4] * that->nbOfCell,
                              LIFE_VOLTAGE_BREAKS[5] * that->nbOfCell,
                              LIFE_VOLTAGE_BREAKS[6] * that->nbOfCell);
        }

        LOG("POWER: current = 0 ; Switched to default profile");

      }

      that->currentRecord = 0; 


    }

    
    
}

void K32_power::task(void *parameter)
{

  K32_power *that = (K32_power *)parameter;
  TickType_t xFrequency = pdMS_TO_TICKS(POWER_CHECK);
  int currentMeas = 0;

  while (true)
  {

    /* Update State of Charge */ 
    that->_lock(); 
    that->SOC = that->_stm32->battery();
    that->_unlock();

    /* Check Current Value */
    if ((CURRENT_SENSOR_TYPE != 0) && (!that->_error))
    {

      /* Averaging*/
      // if (counter == 50)
      // {
      //   that-> _current = currentMeas / 50;
      //   that-> _current = (that->_current - that->currentOffset ) *1000 / that->currentFactor ; // Curent in mA
      //   counter = 0;
      //   currentMeas = 0;
      // }
      // currentMeas = currentMeas + analogRead(that->currentPin) ;
      // counter ++;


      /* Simple check measurement*/ 
      // that->_lock();
      // currentMeas = analogRead(that->currentPin);
      // that->_unlock(); 

      /* Averaging*/
      that->_lock();
      currentMeas = 0; 
      for (int i =0; i<50; i++)
      {
        currentMeas += analogRead(that->currentPin);
      }
      that->_unlock(); 
      currentMeas = currentMeas / 50 ; 

      //LOG(currentMeas); 

      /* Check for problem with current sensor */ 
      if ((currentMeas == 0))
      {
        LOG("POWER ERROR: Current sensor not plugged in. Entering error mode ");
        that->_lock(); 
        that->_error = true ; 
        that->_unlock(); 
        /* Remove adaptive gauge and set to default */ 
        if(that->adaptiveGaugeOn)
        {
          that->setAdaptiveGauge(false, that->battType, that->nbOfCell); 
        }
        /* stop power task*/ 
        continue; 
      }
      that->_lock(); 
      /* Update Current value */ 
      that->_current = (currentMeas - that->currentOffset) * 1000 / that->currentFactor; // Curent in mA

      /* Update charge state */ 
      if (that->_current > 0)
      {
        that->charge = true;
      }
      else if (that->_current < -100)
      {
        that->charge = false;
      }
      that->_unlock(); 

      /* Update values of gauge */ 
      if(that->adaptiveGaugeOn)  that->updateCustom((void *)that); 
  
      /* Delay */ 
      vTaskDelay(xFrequency);      
    }
    else if (that->_error) // Problem with current sensor
    {
      that->_lock(); 
      currentMeas = analogRead(that->currentPin);
      that->_unlock(); 
      /* Check if current sensor has been replugged */ 
      if (currentMeas > 400)
      {
        /* Double check */ 
        vTaskDelay(pdMS_TO_TICKS(1000));  
        that->_lock(); 
        currentMeas = analogRead(that->currentPin);
        that->_unlock(); 
        if (currentMeas > 400)
        {
          LOG("POWER ERROR: Exitting error mode");
          that->_lock(); 
          that->_error = false ; 
          that->_unlock(); 
          if (that->autoGauge)
          {
            that->setAdaptiveGauge(true, that->battType, that->nbOfCell);
          }
        }
     }
      /* Delay */ 
      vTaskDelay(pdMS_TO_TICKS(2000));  
    } 
    else if (CURRENT_SENSOR_TYPE == 0) // No current sensor 
    {
      /* Do nothing... */
      vTaskDelay(pdMS_TO_TICKS(2000));  
    }
    

  }
}
