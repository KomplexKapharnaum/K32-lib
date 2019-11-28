/*
K32_power.h
Created by Clement GANGNEUX, june 2019.
Released under GPL v3.0
*/
#ifndef K32_power_h
#define K32_power_h

#define POWER_CHECK 20000           // task loop in ms


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
    void reset();      // Reset energy time counter
    bool charge ;
    int SOC ;


  private:
    SemaphoreHandle_t lock;
    K32_stm32* _stm32;
    bool running = false;
    int _power = 0;
    int _energy = 0;
    long previousTime ; // time of previous measurement (ms)
    long actualTime ; // actual time (ms)
    TaskHandle_t t_handle = NULL;


    static void task(void * parameter);


};

#endif
