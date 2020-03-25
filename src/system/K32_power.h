/*
K32_power.h
Created by Clement GANGNEUX, june 2019.
Released under GPL v3.0
*/
#ifndef K32_power_h
#define K32_power_h

#include "system/K32_log.h"
#include "system/K32_stm32.h"
#include "system/K32_stm32_api.h"
#include "Arduino.h"


//#define SET_CURRENT_OFFSET 2890        // Set Current calibration offset ((necessary one time only))


#define POWER_CHECK 100           // task loop in ms
#define CURRENT_SENSOR_TYPE 11    // Current sensor type : 0 = no sensor (info given by OSC) ;
                                  // 10 = HO10-P/SP33
                                  // 11 = H010
                                  // 25 = HO25-P/SP33

class K32_power {
  public:
    K32_power(K32_stm32* stm32, const int CURRENT_SENSOR_PIN);

    void start();
    void stop();

    int power();      // Get instant Load power consumption (W)
    int energy();      // Get Energy consummed since last reset (Wh)
    int current() ;    // Get current from current sensor / STM32 if Current type = 0
    void reset();      // Reset energy time counter
    void calibrate(); // Calibrate current sensor. Call this function when Current flowing through sensor is 0. 


    bool charge ;
    int SOC ;


  private:
    SemaphoreHandle_t lock;
    K32_stm32* _stm32;
    int _power = 0;
    int _energy = 0;
    int _current = 0;
    TaskHandle_t t_handle = NULL;
    int currentPin ; 
    int currentOffset = 1800 ;   // Offset of current measurement
    int currentFactor;         // Factor of current measurement

    void _lock();
    void _unlock();


    static void task(void * parameter);


};

#endif
