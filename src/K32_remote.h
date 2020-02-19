/*
K32_remote.h
Created by Clement GANGNEUX, february 2020.
Released under GPL v3.0
*/
#ifndef K32_remote_h
#define K32_remote_h

#define REMOTE_CHECK 100           // task loop in ms
#define BTN_CHECK 10           // btn reading task loop in ms
#define NB_BTN 4              // Number of push buttons

class K32_remote;

#include "K32_log.h"
#include "K32.h"
#include "Arduino.h"


struct digitalbtn {
  bool state; // State of button
  uint8_t pin; // GPIO pin
  int flag = 0 ; // 0 if nothing to do; 1 if a short press occured ; 2 if long press occured
  unsigned long lastPushTime = 0;
};

class K32_remote {
  public:
    K32_remote();


  private:
    SemaphoreHandle_t lock;
    digitalbtn buttons[NB_BTN] ;
    int macroIndex = 0;

    static void task(void * parameter);
    static void read_btn_state(void * parameter);

};

#endif
