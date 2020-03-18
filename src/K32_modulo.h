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
#define MODULO_TYPE_SLOTS 8

class K32_modulo
{

public:
  K32_modulo();

  void set(int k, int value);
  virtual String type_name() { return ""; };
  virtual int getValue();
  void setParam(int k, int value) { if (k < MODULO_PARAM_SLOTS) this->params[k] = value; }
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
  String type_name() { return "sinus"; }
};

// K32_MODULO_RANDOM

class K32_modulo_random : public K32_modulo
{

public:
  K32_modulo_random(int min, int max);

  int getValue();
  String type_name() { return "random"; }
};

// K32_MODULO_LINPLUS

class K32_modulo_linplus : public K32_modulo
{

public:
  K32_modulo_linplus(int period, int min, int max);

  unsigned long period_last;
  int getValue();
  String type_name() { return "linplus"; }
};

// K32_MODULO_LINMOINS

class K32_modulo_linmoins : public K32_modulo
{

public:
  K32_modulo_linmoins(int period, int min, int max);

  unsigned long period_last;
  int getValue();
  String type_name() { return "linmoins"; }
};

// K32_MODULO_ONOFF

class K32_modulo_onoff : public K32_modulo
{

public:
  K32_modulo_onoff(int period, int min, int max);

  unsigned long period_last;
  int getValue();
  String type_name() { return "onoff"; }

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
  String type_name() { return "triplus"; }

protected:
  bool period_cycle;
};

// K32_MODULO_TRIMOINS

class K32_modulo_trimoins : public K32_modulo
{

public:
  K32_modulo_trimoins(int period, int min, int max);

  unsigned long period_last;
  int getValue();
  String type_name() { return "trimoins"; }

protected:
  bool period_cycle;
};

// K32_MODULO_PHASE

class K32_modulo_phase : public K32_modulo
{

public:
  K32_modulo_phase(int period, int min, int max);

  unsigned long period_last;
  int getValue_1();
  int getValue_2();
  int getValue_3();
  String type_name() { return "phase"; }
};


#endif