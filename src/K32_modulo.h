/*
K32_modulo.h
Created by RIRI, Mars 2020.
Released under GPL v3.0
*/
#ifndef K32_modulo_h
#define K32_modulo_h

/* #define 
*/

#include "K32_log.h"
#include "Arduino.h"

#define MODULO_PARAM_SLOTS 8

class K32_modulo
{

public:
  K32_modulo();

  void set(int k, int value);
  virtual int getValue();
  void play();
  void pause();
  void stop();

  int params[MODULO_PARAM_SLOTS];
  SemaphoreHandle_t critical;

  bool isRunning;

protected:
  unsigned long freezeTime;
};

class K32_modulo_sinus : public K32_modulo
{

public:
  K32_modulo_sinus(int period, int min, int max);

  int getValue();
};

class K32_modulo_random : public K32_modulo
{

public:
  K32_modulo_random(int min, int max);

  int getValue();

};

class K32_modulo_linplus : public K32_modulo
{

public:
  K32_modulo_linplus(int period, int min, int max);
  
  unsigned long period_last;
  int getValue();
};


#endif