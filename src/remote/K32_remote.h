/*
K32_remote.h
Created by Clement GANGNEUX, february 2020.
Released under GPL v3.0
*/
#ifndef K32_remote_h
#define K32_remote_h

#define REMOTE_CHECK 100 // task loop in ms
#define BTN_CHECK 10     // btn reading task loop in ms
#define NB_BTN 4         // Number of push buttons

#include "system/K32_log.h"
#include "Arduino.h"
#include "Adafruit_MCP23017.h"

struct digitalbtn
{
  bool state;   // State of button
  uint8_t pin;  // MCP pin
  int flag = 0; // 0 if nothing to do; 1 if a short press occured ; 2 if long press occured
  unsigned long lastPushTime = 0;
};

enum remoteState
{
  REMOTE_AUTO_LOCK,
  REMOTE_AUTO,
  REMOTE_MANU_STM,
  REMOTE_MANU_STM_LOCK,
  REMOTE_MANU_LOCK,
  REMOTE_MANU,
  REMOTE_MANU_LAMP
};

class K32_remote
{
public:
  K32_remote(const int BTN_PIN[2]);
  void setMacroMax(int macroMax);
  void setAuto_Lock();
  void setAuto();
  void setManu_Stm();
  void setManu_Stm_lock();
  void setManu_Lock();
  void setManu();
  void setManu_Lamp();
  
  void setSendMacro();

  remoteState getState();
  int getActiveMacro();
  int getPreviewMacro();
  int getLamp();
  int getSendMacro();

private:
  SemaphoreHandle_t lock;
  digitalbtn buttons[NB_BTN];
  remoteState _state = REMOTE_AUTO;
  remoteState _old_state = REMOTE_MANU_LAMP;
  int _macroMax = 0;
  int _activeMacro = 0;
  int _previewMacro = _macroMax - 1;
  int _lamp = -1;
  int _lamp_grad = 30;
  bool _key_lock = true;
  bool _check_key = true;
  bool _send_active_macro = false;

  void _lock();
  void _unlock();

  Adafruit_MCP23017 mcp;

  static void task(void *parameter);
  static void read_btn_state(void *parameter);
};

#endif
