/*
  K32_modulator.h
  Created by Thomas BOHL, March 2020.
  Released under GPL v3.0
*/
#ifndef K32_modulator_h
#define K32_modulator_h

#define MOD_PARAMS_SLOTS 8

#include "K32_anim.h"

//
// BASE ANIM
//
class K32_modulator
{
public:
  K32_modulator() { this->init(NULL, 0); }

  // Helper to set params
  K32_modulator(int d0) { this->init(new int[1]{d0}, 1); }
  K32_modulator(int d0, int d1) { this->init(new int[2]{d0, d1}, 2); }
  K32_modulator(int d0, int d1, int d2) { this->init(new int[3]{d0, d1, d2}, 3); }
  K32_modulator(int d0, int d1, int d2, int d3) { this->init(new int[4]{d0, d1, d2, d3}, 4); }
  K32_modulator(int d0, int d1, int d2, int d3, int d4) { this->init(new int[5]{d0, d1, d2, d3, d4}, 5); }
  K32_modulator(int d0, int d1, int d2, int d3, int d4, int d5) { this->init(new int[6]{d0, d1, d2, d3, d4, d5}, 6); }
  K32_modulator(int d0, int d1, int d2, int d3, int d4, int d5, int d6) { this->init(new int[7]{d0, d1, d2, d3, d4, d5, d6}, 7); }
  K32_modulator(int d0, int d1, int d2, int d3, int d4, int d5, int d6, int d7) { this->init(new int[8]{d0, d1, d2, d3, d4, d5, d6, d7}, 8); }

  // get/set name
  String name() { return this->_name; }
  void name(String n) { this->_name = n; }

  void attach(int slot)
  {
    if (slot < LEDS_DATA_SLOTS)
      this->dataslot = slot;
  }

  void play()
  {
    this->isRunning = true;
    this->freezeTime = 0;
    this->trigTime = this->time();
    LOGF2("ANIM: %s modulate param %i \n", this->name(), this->dataslot);
  }

  void pause()
  {
    this->freezeTime = millis();
  }

  void stop()
  {
    this->isRunning = false;
  }

  // Execute modulation function
  bool run(int *animData)
  {
    if (this->isRunning && this->dataslot >= 0)
    {
      this->anim_data = animData;

      int &mData = animData[this->dataslot];
      int before = mData;
      xSemaphoreTake(this->paramInUse, portMAX_DELAY);
      this->modulate(mData);
      xSemaphoreGive(this->paramInUse);
      return before != mData;
    }
    return false;
  }

  // change one Params
  K32_modulator *param(int k, int value)
  {
    if (k < MOD_PARAMS_SLOTS)
    {
      // xSemaphoreTake(this->paramInUse, portMAX_DELAY);
      this->params[k] = value;
      // xSemaphoreGive(this->paramInUse);
    }
    return this;
  }

  // set common params
  K32_modulator *mini(int m)
  {
    // xSemaphoreTake(this->paramInUse, portMAX_DELAY);
    this->_mini = m;
    // xSemaphoreGive(this->paramInUse);
    return this;
  }
  K32_modulator *maxi(int m)
  {
    // xSemaphoreTake(this->paramInUse, portMAX_DELAY);
    this->_maxi = m;
    // xSemaphoreGive(this->paramInUse);
    return this;
  }
  K32_modulator *period(int p)
  {
    // xSemaphoreTake(this->paramInUse, portMAX_DELAY);
    this->_period = p;
    // xSemaphoreGive(this->paramInUse);
    return this;
  }
  K32_modulator *phase(int p)
  {
    // xSemaphoreTake(this->paramInUse, portMAX_DELAY);
    this->_phase = p;
    // xSemaphoreGive(this->paramInUse);
    return this;
  }

protected:
  virtual void modulate(int &data)
  {
    LOG("MOD: full data, doing nothing !");
  };

  // TOOLS
  //

  int params[MOD_PARAMS_SLOTS];
  int *anim_data;

  int mini() { return this->_mini; }
  int maxi() { return this->_maxi; }
  int amplitude() { return this->_maxi - this->_mini; }
  int period() { return max(1, this->_period); }
  int phase() { return this->_phase; }
  int phaseTime360() { return ((this->_phase % 360) * this->_period) / 360; }

  void useTriggerTime() { this->useRelativeTimeRef = true; }
  void useAbsoluteTime() { this->useRelativeTimeRef = false; }
  void applyPhase360() { this->applyPhaseCorrectionDeg = true; }

  uint32_t time() {  
    uint32_t t = (this->freezeTime > 0) ? this->freezeTime : millis();
    if (this->useRelativeTimeRef) {
      if (t > this->trigTime) t -= this->trigTime;
      else t = 0;
    }
    if (this->applyPhaseCorrectionDeg)  {
      if (t > phaseTime360()) t -= phaseTime360();
      else t = 0;
    }
    return t; 
  }

  int timePeriod() { return time() % period(); }
  float progress() { return 1.0 * timePeriod() / period(); }
  int periodCount() { return time() / period(); }


private:
  // init on construcion
  void init(int *p, int size)
  {
    this->paramInUse = xSemaphoreCreateBinary();
    size = min(size, MOD_PARAMS_SLOTS);
    for (int k = 0; k < size; k++)
      this->params[k] = p[k];
    xSemaphoreGive(this->paramInUse);
  }

  SemaphoreHandle_t paramInUse;
  unsigned long freezeTime = 0;
  unsigned long trigTime = 0;
  bool useRelativeTimeRef = false;
  bool applyPhaseCorrectionDeg = true;
  String _name = "?";
  bool isRunning = false;

  int dataslot = -1;

  // common params
  int _period = 1000;
  int _phase = 0;
  int _mini = 0;
  int _maxi = 255;
};

#endif