/*
  K32_stm32.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_stm32_h
#define K32_stm32_h

#define STM32_CORE 0
#define STM32_CHECK 100           // task loop in ms
#define STM32_CHECK_BATT 5000     // check battery in ms

#include "Arduino.h"
#include "K32_log.h"
#include "K32_stm32_api.h"
#include "esp_task_wdt.h"
#include <WiFi.h>

class K32_stm32 {
  public:
    K32_stm32();

    void listen();      // Start monitoring thread
    void listen(bool btn, bool battery);      // Start monitoring thread (configurable)

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

    bool _btn_listen = true;
    bool _batt_listen = true;

    static void task( void * parameter );
    void send(K32_stm32_api::CommandType cmd, int arg);
    void send(K32_stm32_api::CommandType cmd);
    long get(K32_stm32_api::CommandType cmd);
    long read();
    void flush();

};

#endif
