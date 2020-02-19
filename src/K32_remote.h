/*
K32_remote.h
Created by Clement GANGNEUX, february 2020.
Released under GPL v3.0
*/
#ifndef K32_remote_h
#define K32_remote_h

#define REMOTE_CHECK 100           // task loop in ms

class K32_remote;
#include "K32_log.h"
#include "K32.h"
#include "Arduino.h"


class K32_remote {
  public:
    K32_remote();


  private:
    SemaphoreHandle_t lock;


    static void task(void * parameter);
};

#endif
