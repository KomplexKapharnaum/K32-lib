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
    void dipswitch_read();
    bool dip[3];
    bool old_dip[3];
    bool dip1() { return this->dip[0]; }
    bool dip2() { return this->dip[1]; }
    bool dip3() { return this->dip[2]; }  

  private:
    SemaphoreHandle_t lock;
    static void dipswatch( void * parameter );
};

#endif
