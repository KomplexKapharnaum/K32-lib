/*
K32_remote.h
Created by Clement GANGNEUX, february 2020.
Modified by MGR, July 2020.
Released under GPL v3.0
*/
#ifndef K32_remote_h
#define K32_remote_h

#define REMOTE_CHECK 100 // task loop in ms
#define BTN_CHECK 10     // btn reading task loop in ms
#define NB_BTN 4         // Number of push buttons

#define DEBUG_lib_btn 1


#include "system/K32_log.h"
#include "system/K32_system.h"
#include "xtension/K32_mcp.h"


enum remoteState
{
  REMOTE_AUTO,            // 0
  REMOTE_MANU,            // 1
  REMOTE_MANU_STM,        // 2
  REMOTE_MANU_LAMP        // 3
};

class K32_remote
{
public:
  K32_remote(K32_system *system, K32_mcp *mcp);

  void setState(remoteState state);
  void setMacroMax(uint8_t macroMax);

  void stmBlackout();
  void stmSetMacro(uint8_t macro);
  void stmNext();

  void lock();
  void unlock();
  bool isLocked();

  remoteState getState();
  int getActiveMacro();
  int getPreviewMacro();
  int getLamp();
  int getLampGrad();
  int getSendMacro();

  void command(Orderz* order);

private:
  SemaphoreHandle_t semalock;
  ioflag buttons[NB_BTN];

  remoteState _state = REMOTE_AUTO;
  remoteState _old_state = REMOTE_MANU_LAMP;
  int _macroMax = 0;
  int _activeMacro = -1;
  int _previewMacro = 0;
  int _lamp = -1;
  int _lamp_grad;
  bool _key_lock = true;
  bool _send_active_macro = false;

  void _semalock();
  void _semaunlock();

  static void task(void *parameter);

  K32_system *system;
  K32_mcp *mcp;

};

#endif
