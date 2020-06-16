/*
K32_power.h
Created by Clement GANGNEUX, april 2020.
Released under GPL v3.0
*/
#ifndef K32_power_h
#define K32_power_h

#include "system/K32_log.h"
#include "system/K32_stm32.h"
#include "system/K32_stm32_api.h"
#include "Arduino.h"


#define POWER_CHECK 100        // task loop in ms
#define CURRENT_SENSOR_TYPE 10 // Current sensor type : 0 = no sensor (info given by OSC) ; 
                               // 10 = HO10-P/SP33                                          
                               // 11 = H010                                                 
                               // 25 = HO25-P/SP33

#define BATTERY_RINT 0.14 // Default internal resistance of battery
#define CURRENT_OFFSET 2890 // Default current calibration offset

enum batteryType
{
  LIPO,
  LIFE,
};

enum calibType
{
  Offset, 
  InternalRes,
}; 


/* All voltages are given in mV */
const unsigned int LIPO_VOLTAGE_BREAKS[] = {3500, 3650, 3700, 3750, 3825, 3950, 4200}; //For one cell
const unsigned int LIFE_VOLTAGE_BREAKS[] = {2920, 3140, 3200, 3220, 3240, 3260, 3600}; //For one cell
const unsigned int LIPO_ERROR_BREAKS[] = {3100, 3300, 3357, 3500, 37857, 4000, 4200}; //For one cell
const unsigned int INITIAL_CELL_VOLTAGE_TOLERANCE = 50;                                // Tolerance added to the cell charged voltage

class K32_power
{
public:
  K32_power(K32_stm32 *stm32, bool autoGauge, const int CURRENT_SENSOR_PIN);

  void start();
  void stop();

  int power();                                                            // Get instant Load power consumption (W)
  int energy();                                                           // Get Energy consummed since last reset (Wh)
  int current();                                                          // Get current from current sensor / STM32 if Current type = 0
  void reset();                                                           // Reset energy time counter
  void calibrate(calibType type);                                                       // Calibrate current sensor. Call this function when Current flowing through sensor is 0.
  void setAdaptiveGauge(bool adaptiveOn, batteryType type, int nbOfCell); // Function to activate adaptive gauge visualisation depending on current.
                                                                          // Set adaptiveOn to true to set adaptive gauge algo
                                                                          // type between LIPO and LIFE
                                                                          // Set nbOfCell according to number of Cell ; 0 means auto determining number of Cells

  bool charge;
  int SOC;

private:
  SemaphoreHandle_t lock;
  K32_stm32 *_stm32;
  int _power = 0;
  int _energy = 0;
  int _current = 0;
  TaskHandle_t t_handle = NULL;

  bool _error = false ; 

  /* Current Sensor specs */
  int currentPin;
  int currentOffset = 1800; // Offset of current measurement
  int currentFactor;        // Factor of current measurement
  int calibVoltage = 0; // Voltage of the battery during calibration of the offset
  //int batteryRint;                 // Value of internal resistance of the battery
  /* Adaptive Gauge variables */
  bool autoGauge = false; // Enable programm to restart adaptive gauge if the sensor is replugged
  bool adaptiveGaugeOn = false;
  batteryType battType = LIPO;
  uint8_t nbOfCell = 0;
  int currentRecord = 0 ; // Save value of current corresponding to actual gauge value
  unsigned int profile[7]; // Operating voltage profile for SOC calculation
  // voltageProfile profiles[MAX_VOLTAGE_PROFILES];
  // int profileIdx = 0; // Number of profile Idx available
  // int profileOn = -1; // Index of operating profile (-1 stands for default mode)

  uint8_t findCellCount(unsigned int voltage, unsigned int cellMin, unsigned int cellMax);
  void updateCustom(void *parameter); 
  void _lock();
  void _unlock();

  static void task(void *parameter);
};

#endif
