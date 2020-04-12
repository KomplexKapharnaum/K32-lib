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

K32_power::K32_power(K32_stm32 *stm32, const int CURRENT_SENSOR_PIN)
{
  LOG("POWER : init");

  this->lock = xSemaphoreCreateMutex();
  this->_stm32 = stm32;
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
#ifdef SET_CURRENT_OFFSET
  this->_lock();
  int old = power_prefs.getUInt("offset", 0);
  if (this->currentOffset != old)
    power_prefs.putUInt("offset", SET_CURRENT_OFFSET);
  this->_unlock();
#else
  this->_lock();
  this->currentOffset = power_prefs.getUInt("offset", 0);
  this->_unlock();
#endif

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

void K32_power::calibrate()
{
  LOG("POWER : Calibration of current sensor");
  this->_lock();
  this->currentOffset = this->currentOffset + this->_current * this->currentFactor / 1000;
  power_prefs.putUInt("offset", this->currentOffset);
  this->_unlock();
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
  this->_lock();
  this->adaptiveGaugeOn = adaptiveOn;
  this->nbOfCell = nbOfCell;
  this->battType = type;
  this->_unlock();
}

void K32_power::addVoltageProfile(unsigned int profile[7], unsigned int minOutputCurrent, unsigned int maxOutputCurrent)
{
  bool currentCheck = true;
  /* Check consistency of output current values */
  for (int i = 0; i < this->profileIdx; i++)
  {
    if ((minOutputCurrent <= this->profiles[i].outputCurrent[1]) && (minOutputCurrent >= this->profiles[i].outputCurrent[0])) // min current is inside interval of another profile
    {
      LOG("POWER: Error min current value for new profile not consistent");
      currentCheck = false;
    }
    else if ((maxOutputCurrent <= this->profiles[i].outputCurrent[1]) && (maxOutputCurrent >= this->profiles[i].outputCurrent[0])) // min current is inside interval of another profile
    {
      LOG("POWER: Error max current value for new profile not consistent");
      currentCheck = false;
    }
    else if ((minOutputCurrent <= this->profiles[i].outputCurrent[0]) && (maxOutputCurrent >= this->profiles[i].outputCurrent[1])) // current values are inside interval of another profile
    {
      LOG("POWER: Error current values for new profile not consistent");
      currentCheck = false;
    }
    else if ((minOutputCurrent >= this->profiles[i].outputCurrent[0]) && (maxOutputCurrent <= this->profiles[i].outputCurrent[1])) // current interval surronds interval of another profile
    {
      LOG("POWER: Error current values for new profile not consistent");
      currentCheck = false;
    }
  }
  /* End of check */
  if (currentCheck)
  {
    for (int i = 0; i < 7; i++)
    {
      this->profiles[this->profileIdx].profile[i] = profile[i];
    }
    this->profiles[this->profileIdx].outputCurrent[0] = minOutputCurrent;
    this->profiles[this->profileIdx].outputCurrent[1] = maxOutputCurrent;
    this->profileIdx++;
    LOGF("POWER: New profile added, now %d profiles available \n", this->profileIdx);
  }
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
}

void K32_power::task(void *parameter)
{

  K32_power *that = (K32_power *)parameter;
  TickType_t xFrequency = pdMS_TO_TICKS(POWER_CHECK);
  int counter = 0;
  int currentMeas = 0;

  while (true)
  {

    /* Check Current Value */
    that->_lock();
    if (CURRENT_SENSOR_TYPE != 0)
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

      currentMeas = analogRead(that->currentPin);
      that->_current = (currentMeas - that->currentOffset) * 1000 / that->currentFactor; // Curent in mA
    }
    else
    {
      /* Do nothing... */
    }

    if (that->_current > 0)
    {
      that->charge = true;
    }
    else if (that->_current < -100)
    {
      that->charge = false;
    }

    /* Adaptive profile */
    if (that->adaptiveGaugeOn)
    {
      bool defaultMode = true; // Will be set to file if a profile corresponds to actual current value
      for (int i = 0; i < that->profileIdx; i++)
      {
        if ((that->_current >= that->profiles[i].outputCurrent[0]) && (that->_current <= that->profiles[i].outputCurrent[1])) // Current in the interval of a profile
        {
          if (i != that->profileOn)
          {
            /* Set new profile according to current value */
            that->_stm32->custom(that->profiles[i].profile[0] * that->nbOfCell,
                                 that->profiles[i].profile[1] * that->nbOfCell,
                                 that->profiles[i].profile[2] * that->nbOfCell,
                                 that->profiles[i].profile[3] * that->nbOfCell,
                                 that->profiles[i].profile[4] * that->nbOfCell,
                                 that->profiles[i].profile[5] * that->nbOfCell,
                                 that->profiles[i].profile[6] * that->nbOfCell);
            /* Update value of operating profile */
            that->profileOn = i;
            i = that->profileIdx; // no need to check other profiles
            LOGF("POWER: Switched to another profile : %d \n", that->profileOn);
          }
          defaultMode = false;
        }
      }
      if ((defaultMode) && (that->profileOn != -1))
      {
        if (that->battType == LIPO)
        {
          that->_stm32->custom(LIPO_VOLTAGE_BREAKS[0] * that->nbOfCell,
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
        that->profileOn = -1;
        LOG("POWER: Switched to default profile");
      }
    }

    that->SOC = that->_stm32->battery();
    that->_unlock();

    vTaskDelay(xFrequency);
  }
}
