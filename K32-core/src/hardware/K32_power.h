/*
K32_power.h
Created by Clement GANGNEUX, april 2020.
Released under GPL v3.0
*/
#ifndef K32_power_h
#define K32_power_h

#include "utils/K32_log.h"
#include "hardware/K32_mcp.h"
#include "hardware/K32_stm32.h"
#include "hardware/K32_stm32_api.h"
#include "Arduino.h"

#define CALIB_BUTTON 7

#define DEFAULT_MEASURE_OFFSET 2000 // Default current calibration offset
#define DEFAULT_BATTERY_RINT 0.12 // Default internal resistance of battery

#define CURRENT_ERROR_OFFSET 100    // Offset from measure error (minimal current draw) ~100 mA
#define CURRENT_AVG_SAMPLES  300    // Number of samples to average measure

enum batteryType
{
  LIPO,
  LIFE
};

enum sensorType     // Values gives current factor
{
  HO10_P_SP33 = 57,
  H010        = 110,
  HO25_P_SP33 = 24
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
    K32_power(K32_stm32 *stm32, batteryType type, bool autoGauge);

    int measure(int samples=CURRENT_AVG_SAMPLES);

    int current();                                                          // Get current from current sensor / STM32 if Current type = 0
    int power();                                                            // Get instant Load power consumption (W)
    int energy();                                                           // Get Energy consummed since last reset (Wh)
    void reset();                                                           // Reset energy time counter
        
    void setExternalCurrentSensor(sensorType sensor, const int pin, int fakeExternalCurrent=0);        // Set current sensor type and pin
    
    void setAdaptiveGauge(bool adaptiveOn);                                 // Function to activate adaptive gauge visualisation depending on current.
                                                                            // Set adaptiveOn to true to set adaptive gauge algo
                                                                            // type between LIPO and LIFE
    
    void setMCPcalib(K32_mcp *mcp);

    bool charge;
    int SOC;

  private:
    SemaphoreHandle_t lock;
    K32_stm32 *_stm32 = nullptr;
    K32_mcp *_mcp = nullptr;
    int _power = 0;
    int _energy = 0;
    int _current = 0;
    int _fakeExternalCurrent = 0;
    TaskHandle_t t_handle = NULL;

    bool _error = false ; 

    /* Current Sensor specs */
    int currentPin;
    int measureOffset = 1800;       // Offset of current measurement
    sensorType currentFactor;       // Factor of current measurement
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
    int rawExtMeasure(int samples=CURRENT_AVG_SAMPLES);
    int extCurrent();

    void calibOffset(int offset);                                           // Calibrate current sensor. Call this function when Current flowing through sensor is 0.
    void calibIres();                                                       // Calibrate current sensor. ??
    
    void _lock();
    void _unlock();

    static void task(void *parameter);
};

#endif
