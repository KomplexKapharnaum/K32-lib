/*
  K32_dipswitch.h
  Created by RIRI, march 2025.
  Released under GPL v3.0
*/
#ifndef K32_dipswitch_h
#define K32_dipswitch_h

// #include <K32_system.h>
// #include "K32_wifi.h"
#include <class/K32_plugin.h>

#define DIP_SLOTS 8

class K32_dipswitch : K32_plugin 
{
  public:
    K32_dipswitch(K32* k32);

    void add(int pin, String name);
    void add(int pin);

    void command(Orderz* order);

  private:
    SemaphoreHandle_t lock;
    static void watch( void * parameter );

    int  watchPins[DIP_SLOTS];
    String  watchNames[DIP_SLOTS];
    bool watchValues[DIP_SLOTS];
};

#endif
