/*
K32_mcp.h
Created by Clement GANGNEUX, february 2020.
Modified by MGR, July 2020.
Released under GPL v3.0
*/
#ifndef K32_mcp_h
#define K32_mcp_h

#define REMOTE_CHECK 100 // task loop in ms
#define BTN_CHECK 10     // btn reading task loop in ms

#include "utils/K32_log.h"
#include "Wire.h"
#include "Adafruit_MCP23X17.h"

enum iomode { MCPIO_DISABLE, MCPIO_INPUT, MCPIO_OUTPUT };
enum ioflag { MCPIO_NOT, MCPIO_PRESS, MCPIO_PRESS_LONG, MCPIO_RELEASE_LONG, MCPIO_RELEASE_SHORT };

struct mcpio
{
  bool    state;                   // State of IO
  iomode  mode  = MCPIO_DISABLE;   // IO mode
  ioflag  flag  = MCPIO_NOT;       // IO flag
  unsigned long lastPushTime = 0;
};

class K32_mcp
{
public:
  K32_mcp(const int MCP_PIN[2]);

  void input(uint8_t pin);
  void output(uint8_t pin);

  bool state(uint8_t pin);
  ioflag flag(uint8_t pin);
  void consume(uint8_t pin);

  void set(uint8_t pin, bool value);

private:
  SemaphoreHandle_t lock;
  mcpio io[16];

  void _lock();
  void _unlock();

  Adafruit_MCP23X17 mcp;

  static void read_btn_state(void *parameter);


};

#endif
