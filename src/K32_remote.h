/*
K32_remote.h
Created by Clement GANGNEUX, february 2020.
Released under GPL v3.0
*/
#ifndef K32_remote_h
#define K32_remote_h

#define REMOTE_CHECK 100       // task loop in ms
#define BTN_CHECK 10           // btn reading task loop in ms
#define NB_BTN 4               // Number of push buttons


#include "K32_log.h"
#include "Arduino.h"
#include "Adafruit_MCP23017.h"


struct digitalbtn {
  bool state; // State of button
  uint8_t pin; // MCP pin
  int flag = 0 ; // 0 if nothing to do; 1 if a short press occured ; 2 if long press occured
  unsigned long lastPushTime = 0;
};

enum remoteState {
  REMOTE_AUTO,
  REMOTE_MANU,
  REMOTE_MANULOCK
};

class K32_remote {
  public:
    K32_remote(const int BTN_PIN[2]);
    void setMacroMax(int macroMax) ;
    void setAuto();

    remoteState getState();
    int getActiveMacro();
    int getPreviewMacro();

  private:
    SemaphoreHandle_t lock;
    digitalbtn buttons[NB_BTN] ;
    remoteState _state = REMOTE_AUTO;
    int _macroMax = 0;
    int _activeMacro = 0;
    int _previewMacro = 0;

    void _lock();
    void _unlock();

    Adafruit_MCP23017 mcp;

    static void task(void * parameter);
    static void read_btn_state(void * parameter);
};

#endif
