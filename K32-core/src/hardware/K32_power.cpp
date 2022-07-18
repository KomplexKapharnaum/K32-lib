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

K32_power::K32_power(K32_stm32 *stm32, batteryType type, bool autoGauge)
{
  this->_stm32 = stm32;

  if (!this->_stm32) {
    LOG("POWER: no stm32 available.. disabled");
    return;
  }

  LOG("POWER: init");

  this->lock = xSemaphoreCreateMutex();
  this->battType = type;
  
  this->autoGauge = autoGauge;
  if (autoGauge) this->enableAdaptativeGauge();
  else this->disableAdaptativeGauge();

  this->_fakeExternalCurrent = 0;
  this->currentPin = -1;

  // Calib button
  if (this->_mcp && this->_mcp->ok)
    this->_mcp->input(CALIB_BUTTON);

  // Set Current offset
  power_prefs.begin("k32-power", false);
  this->measureOffset = power_prefs.getUInt("offset", DEFAULT_MEASURE_OFFSET);
  // this->batteryRint = power_prefs.getUInt("rint", DEFAULT_BATTERY_RINT);
  this->batteryRint = DEFAULT_BATTERY_RINT;
  power_prefs.end(); 

  // Start main task
  
    xTaskCreate(this->task,
                "power_task",
                2000,
                (void *)this,
                0, // priority
                &t_handle);
};



void K32_power::setExternalCurrentSensor(sensorType sensor, const int pin, int fakeExternalCurrent) 
{
  this->currentFactor = sensor;
  this->currentPin = pin;
  this->_fakeExternalCurrent = fakeExternalCurrent;
}

void K32_power::setMCPcalib(K32_mcp *mcp) {
    this->_mcp = mcp;
}

int K32_power::current()
{
  int a = 0;
  if (!this->_error) {
    this->_lock();
    a = this->_current;
    this->_unlock();
  }
  return a;
}

int K32_power::power()
{
  if (this->_stm32) return this->current() * this->_stm32->voltage() / 1000; // Power in mW
  else  return 0; 
}

int K32_power::energy()
{
  int e = 0;
  this->_lock();
  e = this->_energy;
  this->_unlock();
  return e; 
}

void K32_power::reset()
{
  this->_lock();
  this->_energy = 0;
  this->_unlock();
}


void K32_power::calibOffset(int offset)
{
  LOG("POWER : Calibration of current sensor offset");
  LOG("POWER : Make sure current use is near 0 !!!");

  this->_lock();
  this->measureOffset = offset;
  power_prefs.begin("k32-power", false);
  power_prefs.putUInt("offset", this->measureOffset);
  power_prefs.end() ; 
  this->_unlock();

  LOGF("POWER : Offset at %d\n", offset);
}

void K32_power::calibIres()
{

  if( this->_error) 
    LOG("POWER : Error with current sensor, calibration is impossible"); 
  else 
  {
    /* INTERNAL RES FUNCTION TO REWORK */ 
    // int currentMeas = 0 ; 
    // int voltageMeas = 0; 
    //int rintMeas = 0.14 ; 
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
    //   this->_current = (currentMeas - this->measureOffset) * 1000 / this->currentFactor; // Curent in mA
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

void K32_power::enableAdaptativeGauge()
{
  LOG("POWER : Switch on adaptive gauge");
  this->adaptiveGaugeOn = true;
  this->firstKick = true;
  return;
}

void K32_power::disableAdaptativeGauge(int fake_current)
{
  LOG("POWER : Switch off adaptive gauge");
  this->adaptiveGaugeOn = false;
  this->_current = fake_current;

  // OFF -> set FAKE CURRENT
  this->firstKick = true;
  this->setCustomGauge();
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


void K32_power::setCustomGauge()
{
    if (!this->_stm32) return;

    if (this->_current < 0) return; // Weird value

    if (this->firstKick || abs(this->_current - this->currentRecord) > 500 ) // If current changed significantly
    {
      this->_lock(); 

      /* Update profile according to current value */
      for (int i = 0; i<7; i++)
          this->profile[i] = VOLTAGE[this->battType][i]*this->nbOfCell - this->batteryRint * this->_current ; 

      /* Update custom profile */ 
      this->_stm32->custom( this->profile[0],
                            this->profile[1],
                            this->profile[2],
                            this->profile[3],
                            this->profile[4],
                            this->profile[5], 
                            this->profile[6]);

      vTaskDelay( pdMS_TO_TICKS(100) );

      /* Update value of operating profile */
      this->currentRecord = this->_current ;
      LOGF("POWER : new profile with current : %d mA \n", this->currentRecord ); 
      
      this->firstKick = false;
      this->_unlock(); 
    }
}


int K32_power::rawExtMeasure(int samples)
{
  int meas = 0;
  if (this->currentPin > 0)
  {
    /* Warning this function is blocking for ~100ms ! */
    for (int i =0; i<samples; i++)
    {
      meas += analogRead(this->currentPin);
      vTaskDelay( pdMS_TO_TICKS(2) );
    }
    meas = meas / samples;
  }
  
  return meas;
}

int K32_power::extCurrent()
{
  int meas = this->rawExtMeasure();

  // Valid measure
  if (meas > 200 && meas < 20000) 
  {
    if (this->_error) LOG("POWER: Current sensor is back !");
    this->_error = false; 
    meas = (meas - this->measureOffset) * 1000 / this->currentFactor;
  }

  // Invalid measure -> use fake current value
  else {
    if (!this->_error) LOG("POWER: Current sensor lost.. fallback to STM32 ");
    this->_error = true; 
    meas = this->_fakeExternalCurrent;  // Sensor is down, use fakecurrent..
  }

  return meas;
}



void K32_power::task(void *parameter)
{
  K32_power *that = (K32_power *)parameter;

  // CELLS DETECT
  //
  int wait = 300;
  int vCellMin = VOLTAGE[that->battType][0]-VOLTAGE_CELLS_TOLERANCE;
  int v = 0;

  while (that->nbOfCell == 0)
  {
    vTaskDelay( pdMS_TO_TICKS(wait) );
    
    /* Auto determination of number of Cell*/
    v = that->_stm32->voltage();
    for (int n = 7; n > 0; n--)
      if (n == 6 || n == 5) continue;
      else if (v > n * vCellMin) {
        that->nbOfCell = n;
        break;
      }

    /* Retry */
    if (that->nbOfCell == 0) {
      LOGF2("POWER: Can't detect number of cells. voltage=%d cells=%d\n", v, that->nbOfCell);
      if (wait < 30000) wait *= 1.1;
    }
  }
  LOGF2("POWER: Nb cells detected: voltage=%d cells=%d\n", v, that->nbOfCell);


  // CURRENT MEASURE
  //
  TickType_t xFrequency = pdMS_TO_TICKS(1000);
  int currentMeas = 0;
  while (true)
  { 
    /* Check CALIB button */
    if (that->_mcp && that->_mcp->ok) 
    {
      ioflag calibBtn = that->_mcp->flag(CALIB_BUTTON);

      // Long Push -> Offset
      if (calibBtn == MCPIO_PRESS_LONG || calibBtn == MCPIO_RELEASE_LONG) 
      {  
        LOGF2("POWER: Long push on calibration button %d %d \n", currentMeas, that->_stm32->current());
        that->_stm32->switchLoad(false);
        vTaskDelay(pdMS_TO_TICKS(1000));
        int raw = that->rawExtMeasure(500);
        that->calibOffset(raw);
        that->_mcp->consume(CALIB_BUTTON);
        that->_stm32->switchLoad(true);
        vTaskDelay(pdMS_TO_TICKS(1000));
      }
    }
    

    // Error offset
    currentMeas = CURRENT_ERROR_OFFSET; 

    // STM32 sensor
    if (that->_stm32) currentMeas += that->_stm32->current();
    
    // External Sensor
    currentMeas += that->extCurrent();

    /* Set current */
    that->_lock(); 
    that->_current = currentMeas;
    that->_unlock();
    
    /* Update Adaptative profile */ 
    if(that->adaptiveGaugeOn) that->setCustomGauge(); 
    
    vTaskDelay(xFrequency);  
  }
}
