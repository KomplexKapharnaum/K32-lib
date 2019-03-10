/*
  kesp_stm32.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef KESP_STM32_h
#define KESP_STM32_h

#define STM32_CORE 0
#define STM32_CHECK 100           // task loop in ms
#define STM32_CHECK_BATT 5000     // check battery in ms

#include "Arduino.h"
#include "KESP_STM32_API.h"
#include "KESP_LOG.h"
#include "esp_task_wdt.h"
#include <WiFi.h>

class KESP_STM32 {
  public:
    KESP_STM32();

    void leds(uint8_t *values);      // Set Leds

    int battery();      // Get Battery %
    bool clicked();     // Get Btn Click
    bool dblclicked();  // Get Btn DblClick

    void wait();        // Wait for stm32
    void reset();       // Reset board
    void shutdown();    // Shutdown board


  private:
    SemaphoreHandle_t lock;
    bool running = false;
    int _battery = 0;
    bool _btn_click = false;
    bool _btn_dblclick = false;

    static void task( void * parameter );
    void send(KESP_STM32_API::CommandType cmd, int arg);
    void send(KESP_STM32_API::CommandType cmd);
    long get(KESP_STM32_API::CommandType cmd);
    long read();
    void flush();

};

#endif
