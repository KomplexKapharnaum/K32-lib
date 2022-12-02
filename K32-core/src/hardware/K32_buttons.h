/*
  K32_buttons.h
  Created by Thomas BOHL, march 2022.
  Released under GPL v3.0
*/
#ifndef K32_buttons_h
#define K32_buttons_h

#include <K32_system.h>
#include "K32_wifi.h"

#define BTNS_SLOTS 16
#define DEBOUNCE_COUNT 4
#define LONGPRESS_COUNT 200

class K32_buttons : K32_plugin 
{
  public:
    K32_buttons(K32* k32);

    void add(int pin, String name);
    void add(int pin);

    void command(Orderz* order);

  private:
    SemaphoreHandle_t lock;
    static void watch( void * parameter );

    int  watchPins[BTNS_SLOTS];
    String  watchNames[BTNS_SLOTS];
    bool watchValues[BTNS_SLOTS];
    int watchDirty[BTNS_SLOTS];  // Debounce
};

#endif
