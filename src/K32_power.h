/*
K32_power.h
Created by Clement GANGNEUX, june 2019.
Released under GPL v3.0
*/
#ifndef K32_power_h
#define K32_power_h

#define POWER_CHECK 100           // task loop in ms
#define CURRENT_SENSOR_TYPE 25    // Current sensor type : 0 = no sensor (info given by OSC) ; 10 = HO10-P/SP33 ; 11 = H010 ; 25 = HO10-P/SP33
#define CURRENT_SENSOR_PORT 35    // Current Sensor GPIO num
#define CURRENT_CALIB 1819    // Calibration of current sensor
#define MODEL_VERSION true  // code for ESP used in model



#include "K32_log.h"
#include "K32_stm32_api.h"
#include "K32_stm32.h"
#include "esp_task_wdt.h"

#include "K32_epd.h"


class K32_power {
  public:
    K32_power(K32_stm32* stm32);

    void start();
    void stop();

    int power();      // Get instant Load power consumption (W)
    int energy();      // Get Energy consummed since last reset (Wh)
    int current() ;    // Get current from current sensor / STM32 if Current type = 0
    void reset();      // Reset energy time counter
    void set_power(int current, int battery, int power);
    bool charge ;
    int SOC ;
    void set_demo();


  private:
    SemaphoreHandle_t lock;
    K32_stm32* _stm32;
    bool running = false;
    bool demo = false;
    int _power = 0;
    int _energy = 0;
    int _current = 0;
    long previousTime ; // time of previous measurement (ms)
    long actualTime ; // actual time (ms)
    TaskHandle_t t_handle = NULL;


    static void task(void * parameter);


};

#endif
