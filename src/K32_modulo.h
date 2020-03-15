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

// K32_MODULO_SINUS

class K32_modulo_sinus : public K32_modulo
{

public:
  K32_modulo_sinus(int period, int min, int max);

  int getValue();
};

// K32_MODULO_RANDOM

class K32_modulo_random : public K32_modulo
{

public:
  K32_modulo_random(int min, int max);

  int getValue();
};

// K32_MODULO_LINPLUS

class K32_modulo_linplus : public K32_modulo
{

public:
  K32_modulo_linplus(int period, int min, int max);

  unsigned long period_last;
  int getValue();
};

// K32_MODULO_LINMOINS

class K32_modulo_linmoins : public K32_modulo
{

public:
  K32_modulo_linmoins(int period, int min, int max);

  unsigned long period_last;
  int getValue();
};

// K32_MODULO_ONOFF

class K32_modulo_onoff : public K32_modulo
{

public:
  K32_modulo_onoff(int period, int min, int max);

  unsigned long period_last;
  int getValue();

protected:
  bool period_cycle;
};

// K32_MODULO_TRIPLUS

class K32_modulo_triplus : public K32_modulo
{

public:
  K32_modulo_triplus(int period, int min, int max);

  unsigned long period_last;
  int getValue();

protected:
  bool period_cycle;
};

#endif