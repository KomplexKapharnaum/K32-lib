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

K32_power::K32_power(K32_stm32 *stm32, K32_mcp *mcp, batteryType type, bool autoGauge, int fakeExtCurrent, const int CURRENT_SENSOR_PIN)
{
  LOG("POWER : init");

  this->lock = xSemaphoreCreateMutex();
  this->battType = type;
  this->currentPin = CURRENT_SENSOR_PIN;
  
  this->_stm32 = stm32;
  this->_mcp = mcp;
  this->_fakeExtCurrent = fakeExtCurrent;

  this->autoGauge = autoGauge;
  this->adaptiveGaugeOn = autoGauge;

  // Calib button
  if (this->_mcp)
    this->_mcp->input(CALIB_BUTTON);

  // Set Current factor depending on HW specifications
  if (CURRENT_SENSOR_TYPE == 10) this->currentFactor = 57;        // original: 60
  else if (CURRENT_SENSOR_TYPE == 25) this->currentFactor = 24;
  else if (CURRENT_SENSOR_TYPE == 11) this->currentFactor = 110;

  // Set Current offset
  power_prefs.begin("k32-power", false);
  this->measureOffset = power_prefs.getUInt("offset", DEFAULT_MEASURE_OFFSET);
  // this->batteryRint = power_prefs.getUInt("rint", DEFAULT_BATTERY_RINT);
  this->batteryRint = DEFAULT_BATTERY_RINT;
  power_prefs.end(); 

  // Start main task
  if (CURRENT_SENSOR_TYPE > 0)
    xTaskCreate(this->task,
                "power_task",
                5000,
                (void *)this,
                0, // priority
                &t_handle); 
};

int K32_power::current()
{
  int a = 0;
  if (CURRENT_SENSOR_TYPE > 0 && !this->_error) {
    this->_lock();
    a = this->_current;
    this->_unlock();
  }
  else a = this->_stm32->current();
  return a;
}

int K32_power::power()
{
  int v = this->_stm32->voltage();
  if (v > 0) return this->current() * v / 1000; // Power in mW
  else return 0;
}

int K32_power::energy()
{
  int e = 0;
  this->_lock();
  e = this->_energy;
  this->_unlock();
  return e; // TODO !
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

void K32_power::setAdaptiveGauge(bool adaptiveOn)
{
  this->adaptiveGaugeOn = adaptiveOn;

  // Set ON
  if (adaptiveOn) {
    LOG("POWER : Switch on adaptive gauge");
    this->firstKick = true;
    return;
  }

  // Set OFF  
  LOG("POWER : Switch off adaptive gauge");

  // OFF -> set FAKE CURRENT
  this->_current = this->_fakeExtCurrent;
  this->firstKick = true;
  this->updateCustom();
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


// int K32_power::voltageBreak(uint8_t i) 
// {
//   int minVoltage;
//   minVoltage = VOLTAGE[this->battType][i] * this->nbOfCell;
//   if (i < 6) minVoltage = minVoltage *(100-VOLTAGE_DROP_PCT)/100;
//   int v = VOLTAGE[this->battType][i] * this->nbOfCell - this->batteryRint * this->current();
//   if (v < minVoltage) v = minVoltage;
//   return v;
// }


void K32_power::updateCustom()
{
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


int K32_power::measure(int samples)
{
  /* Warning this function is blocking for ~100ms ! */
  long meas = 0; 
  for (int i =0; i<samples; i++)
  {
    meas += analogRead(this->currentPin);
    vTaskDelay( pdMS_TO_TICKS(2) );
  }
  meas = meas / samples ; 
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
      if (wait < 5000) wait *= 1.1;
    }
  }
  LOGF2("POWER: Nb cells detected: voltage=%d cells=%d\n", v, that->nbOfCell);


  // CURRENT MEASURE
  //
  TickType_t xFrequency = pdMS_TO_TICKS(POWER_CHECK);
  int currentMeas = 0;
  while (true)
  { 
    // Read sensor
    currentMeas = that->measure();

    /* Check sensor state */ 
    if (currentMeas < 200)
    {
      if (!that->_error) LOG("POWER: Current sensor not plugged. Entering error mode ");
      that->_error = true ; 
    }
    
    /* Sensor in ERROR: check again */
    if (that->_error && currentMeas > 400) 
    {
      LOG("POWER: Sensor is back !");
      that->_error = false ; 
    }

    /* Check CALIB button */
    if (that->_mcp) 
    {
      ioflag calibBtn = that->_mcp->flag(CALIB_BUTTON);

      // Long Push -> Offset
      if (calibBtn == MCPIO_PRESS_LONG || calibBtn == MCPIO_RELEASE_LONG) 
      {  
        that->_stm32->switchLoad(false);
        vTaskDelay(pdMS_TO_TICKS(1000));
        currentMeas = that->measure(500);
        LOGF2("POWER: Long push on calibration button %d %d \n", currentMeas, that->_stm32->current());
        that->calibOffset(currentMeas);
        that->_mcp->consume(CALIB_BUTTON);
        that->_stm32->switchLoad(true);
        vTaskDelay(pdMS_TO_TICKS(1000));
        currentMeas = that->measure();
      }
    }

    /* Calculate EXT current */ 
    int a = CURRENT_ERROR_OFFSET; // Curent in mA
    if (!that->_error) a += (currentMeas - that->measureOffset) * 1000 / that->currentFactor; // external Measure
    else a += that->_stm32->current() + that->_fakeExtCurrent; // internal Measure

    /* Check Current value */ 
    if(abs(a)>20000) 
    {
      LOGF("POWER: Error with current sensor value %d mA", a); 
      that->_error = true;
      that->setAdaptiveGauge(false);
      continue;   /* Loop again */
    }

    /* Set current */
    that->_lock(); 
    that->_current = a;
    // LOGF3("POWER: Current calc=%d stm32=%d pct=%d \n", a, that->_stm32->current(), that->_stm32->battery());
    that->_unlock();
    
    /* Update Adaptative profile */ 
    if(that->adaptiveGaugeOn) that->updateCustom(); 
    
    vTaskDelay(xFrequency);  

  }
}
