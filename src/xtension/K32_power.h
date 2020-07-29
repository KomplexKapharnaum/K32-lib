/*
K32_power.h
Created by Clement GANGNEUX, april 2020.
Released under GPL v3.0
*/
#ifndef K32_power_h
#define K32_power_h

#include "system/K32_log.h"
#include "xtension/K32_mcp.h"
#include "system/K32_stm32.h"
#include "system/K32_stm32_api.h"
#include "Arduino.h"

#define CALIB_BUTTON 7

#define POWER_CHECK 1000        // task loop in ms
#define CURRENT_SENSOR_TYPE 10 // Current sensor type : 0 = no sensor (info given by OSC) ; 
                               // 10 = HO10-P/SP33                                          
                               // 11 = H010                                                 
                               // 25 = HO25-P/SP33

#define DEFAULT_BATTERY_RINT 0.12 // Default internal resistance of battery
#define DEFAULT_MEASURE_OFFSET 2000 // Default current calibration offset

#define CURRENT_ERROR_OFFSET 100    // Offset from measure error (minimal current draw) ~100 mA
#define CURRENT_AVG_SAMPLES  300    // Number of samples to average measure
#define CURRENT_FAKE         12500  // fake current when sensor is unplugged

enum batteryType
{
  LIPO,
  LIFE
};

/* All voltages are given in mV, cf. batteryType for order */
const unsigned int VOLTAGE[2][7] = {
  {3270, 3600, 3700, 3775, 3825, 3925, 4200},     // LIPO one cell 
  {2920, 3140, 3200, 3220, 3240, 3260, 3600}      // LIFE one cell 
};

#define VOLTAGE_DROP_PCT 7                                         // Max Voltage drop in % (when MAX current or ERROR mode)
#define VOLTAGE_CELLS_TOLERANCE 100

class K32_power
{
  public:
    K32_power(K32_stm32 *stm32, K32_mcp *mcp, batteryType type, bool autoGauge, int fakeExtCurrent, const int CURRENT_SENSOR_PIN);

    int measure(int samples=CURRENT_AVG_SAMPLES);

    int current();                                                          // Get current from current sensor / STM32 if Current type = 0
    int power();                                                            // Get instant Load power consumption (W)
    int energy();                                                           // Get Energy consummed since last reset (Wh)
    void reset();                                                           // Reset energy time counter
    
    void calibBtn();                                                        // Check if CALIB btn pressed
    void calibOffset(int offset);                                           // Calibrate current sensor. Call this function when Current flowing through sensor is 0.
    void calibIres();                                                       // Calibrate current sensor. ??
    
    void setAdaptiveGauge(bool adaptiveOn);                                 // Function to activate adaptive gauge visualisation depending on current.
                                                                            // Set adaptiveOn to true to set adaptive gauge algo
                                                                            // type between LIPO and LIFE
    
    bool charge;
    int SOC;

  private:
    SemaphoreHandle_t lock;
    K32_stm32 *_stm32;
    K32_mcp *_mcp;
    int _power = 0;
    int _energy = 0;
    int _current = 0;
    int _fakeExtCurrent = 0;
    TaskHandle_t t_handle = NULL;

    bool _error = false ; 

    /* Current Sensor specs */
    int currentPin;
    int measureOffset = 1800;       // Offset of current measurement
    int currentFactor;              // Factor of current measurement
    int calibVoltage = 0;           // Voltage of the battery during calibration of the offset
    float batteryRint;              // Value of internal resistance of the battery

    /* Adaptive Gauge variables */
    bool firstKick = true;
    bool autoGauge = false; // Enable programm to restart adaptive gauge if the sensor is replugged
    bool adaptiveGaugeOn = false;
    batteryType battType = LIPO;
    uint8_t nbOfCell = 0;
    int currentRecord = 0 ; // Save value of current corresponding to actual gauge value
    unsigned int profile[7]; // Operating voltage profile for SOC calculation
    // voltageProfile profiles[MAX_VOLTAGE_PROFILES];
    // int profileIdx = 0; // Number of profile Idx available
    // int profileOn = -1; // Index of operating profile (-1 stands for default mode)

    void updateCustom(); 

    void _lock();
    void _unlock();

    static void task(void *parameter);
};

#endif
