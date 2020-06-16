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

//#define DEBUG_lib_btn 1
#define CALIB_BUTTON 7

#include "system/K32_log.h"
#include "Adafruit_MCP23017.h"
#include "system/K32_system.h"

struct digitalbtn
{
  bool state;   // State of button
  uint8_t pin;  // MCP pin
  int flag = 0; // 0 if nothing to do; 1 if a short press occured ; 2 if long press occured
  unsigned long lastPushTime = 0;
};

enum remoteState
{
  REMOTE_AUTO_LOCK,       // 0
  REMOTE_AUTO,            // 1
  REMOTE_MANU_LOCK,       // 2
  REMOTE_MANU_STM_LOCK,   // 3
  REMOTE_MANU,            // 4
  REMOTE_MANU_STM,        // 5
  REMOTE_MANU_LAMP        // 6
};

class K32_remote
{
public:
  K32_remote(K32_system *system, const int BTN_PIN[2]);

  void setMacroMax(int macroMax);
  void setState(remoteState state);
  void stmNext();

  remoteState getState();
  int getActiveMacro();
  int getPreviewMacro();
  int getLamp();
  int getLampGrad();
  int getSendMacro();

private:
  SemaphoreHandle_t lock;
  #ifdef CALIB_BUTTON
  digitalbtn buttons[NB_BTN + 1];
  #else
  digitalbtn buttons[NB_BTN];
  #endif
  remoteState _state = REMOTE_AUTO_LOCK;
  remoteState _old_state = REMOTE_MANU_LAMP;
  int _macroMax = 0;
  int _activeMacro = 0;
  int _previewMacro = 0;
  int _lamp = -1;
  int _lamp_grad;
  bool _key_lock = true;
  bool _check_key = true;
  bool _send_active_macro = false;

  void _lock();
  void _unlock();

  Adafruit_MCP23017 mcp;

  static void task(void *parameter);
  static void read_btn_state(void *parameter);

  K32_system *system;


};

#endif
