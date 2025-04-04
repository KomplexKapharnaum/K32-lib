/*
  K32_dipswitch.h
  Created by RIRI, march 2025.
  Released under GPL v3.0
*/
#ifndef K32_dipswitch_h
#define K32_dipswitch_h

#include <class/K32_plugin.h>


class K32_dipswitch : K32_plugin 
{
  public:
    K32_dipswitch(K32* k32);

    void add(int pin, String name);
    void add(int pin);


  private:
    SemaphoreHandle_t lock;
    static void dipswatch( void * parameter );
    void dipswitch_read(int pin, int value);
    bool dip[3];
};

#endif

// mesure de la pin
// dip     = 0
// dip 1   = 1770-1780
// dip 2   = 2135-2142
// dip 3   = 2380-2390
// dip 12  = 2740-2750
// dip 1 3 = 2950-2960
// dip  23 = 2995-3005
// dip 123 = 3369-3376