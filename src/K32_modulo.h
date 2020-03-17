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

// MODULO BOOK

class K32_modulo_typebook
{
public:
  K32_modulo_typebook();

  K32_modulo *get(String type_name)
  {

    for (int k = 0; k < this->counter; k++)
      if (this->type[k]->type_name() == type_name)
      {
        // LOGINL("TYPE: "); LOG(type_name);
        return this->type[k];
      }
    LOGINL("TYPE: not found ");
    LOG(type_name);
    return new K32_modulo();
  }

private:
  K32_modulo *type[MODULO_TYPE_SLOTS];
  int counter = 0;

  void add(K32_modulo *type)
  {
    if (this->counter >= MODULO_TYPE_SLOTS)
    {
      LOG("ERROR: no more slot available to register new modulo");
      return;
    }
    this->type[this->counter] = type;
    this->counter++;
  };
};

#endif