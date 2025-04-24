/*
  K32_modulator.h
  Created by Thomas BOHL, March 2020.
  Released under GPL v3.0
*/
#ifndef K32_modulator_h
#define K32_modulator_h

#define MOD_PARAMS_SLOTS 8

enum ModMode {RELATIVE, ABSOLUTE, ADDITIVE}; 

typedef void (*cbPtrMod )(int value);

#include "K32_anim.h"

/*
  NOTE: This is the modulator BASE class,
  for usable modulator, look into K32_mods.h
*/


//
// BASE MODULATOR
//
class K32_modulator
{
public:
  K32_modulator() {
    this->paramInUse = xSemaphoreCreateBinary();
    xSemaphoreGive(this->paramInUse);

    memset(this->params, 0, sizeof this->params);

    for (int s=0; s<ANIM_DATA_SLOTS; s++) 
      this->dataslot[s] = false;

    
  }

  virtual ~K32_modulator() {
    vQueueDelete(this->paramInUse);
  }

  // get/set name
  String name() { return this->_name; }
  K32_modulator*  name(String n) { 
    this->_name = n; 
    return this;
  }

  K32_modulator* hold() {
    usage++;
    return this;
  }

  K32_modulator* unhold() {
    usage--;
    return this;
  }

  boolean onHold() {
    return usage > 0;
  }

  // mode : absolute/relative
  K32_modulator* absolute(){
    this->_mode = ABSOLUTE;
    return this;
  }

  K32_modulator* relative(){
    this->_mode = RELATIVE;
    return this;
  }

  K32_modulator* additive(){
    this->_mode = ADDITIVE;
    return this;
  }

  K32_modulator* at(int slot)
  {
    if (slot >= 0  && slot < ANIM_DATA_SLOTS) this->dataslot[slot] = true;
    return this;
  }

  K32_modulator* play()
  {
    if (!this->isRunning) {
      this->trigger();
      // if (this->dataslot >= 0) LOGF("ANIM: %s modulate\n", this->name().c_str());
      this->freezeTime = 0;
      this->isRunning = true;
    }
    return this;
  }

  K32_modulator* trigger() 
  {
    this->triggerTime = millis();
    this->_fresh = true;
    return this;
  }

  K32_modulator* pause()
  {
    this->freezeTime = millis();
    return this;
  }

  K32_modulator* stop()
  {
    this->isRunning = false;
    return this;
  }

  K32_modulator* event( cbPtrMod callback )
  {
    this->_event = callback;
    return this;
  }

  // Execute modulation function
  bool run(int *animData)
  { 
    bool didChange = false;

    if (this->isRunning)
    {
      // Get Modulator value
      xSemaphoreTake(this->paramInUse, portMAX_DELAY);
      int val = this->value();
      xSemaphoreGive(this->paramInUse);

      // RELATIVE
      if (_mode == RELATIVE)
      {
        // CLAMP modulator value to 0->255
        val = min(255, val);
        val = max(0, val);

        // Apply modulation to dataslots, value of 255 will not do anything
        if (val < 255) {
          for (int s=0; s<ANIM_DATA_SLOTS; s++)
            if (this->dataslot[s]) animData[s] = scale16by8(animData[s], (uint8_t)val);
        }
      }
      // ABSOLUTE
      else if (_mode == ABSOLUTE) 
      {
        // Apply absolute value to dataslots
        for (int s=0; s<ANIM_DATA_SLOTS; s++)
            if (this->dataslot[s]) animData[s] = val;
      }
      // ADDITIVE
      else if (_mode == ADDITIVE) 
      {
        // Add mod value to animData
        for (int s=0; s<ANIM_DATA_SLOTS; s++)
            if (this->dataslot[s]) animData[s] += val;
      }

      // Did animator produced a different result than last call ?
      didChange = (this->_lastProducedValue != val);
      this->_lastProducedValue = val;

      // custom event callback 
      if (didChange && this->_event != nullptr) {
        this->_event(val);
      }

    }
    return didChange;
  }

  // 8Bit Direct value : Defined in SubClass ! 
  virtual int value()    { return 255; }

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

  // set special params
  K32_modulator *mini(int m) {
    this->_mini = m;
    return this;
  }
  K32_modulator *maxi(int m) {
    this->_maxi = m;
    return this;
  }
  K32_modulator *period(int p) {
    this->_period = p;
    return this;
  }
  K32_modulator *phase(int p) {
    this->_phase = p;
    return this;
  }

  K32_modulator *setMaxValue(int v) {
    this->_MaxValue = v;
    return this;
  }

  // TOOLS
  //

  // get special params
  int mini() { return this->_mini; }
  int maxi() { return this->_maxi; }
  int amplitude() { return this->_maxi - this->_mini; }
  int period() { return max(1, this->_period); }
  int phase() { return this->_phase; }
  int MaxValue() { return this->_MaxValue; }

  virtual int time() { return (this->freezeTime > 0) ? this->freezeTime : millis(); }
  float progress() { return 1.0 * (time() % period()) / period(); }
  int periodCount() { return time() / period(); }

  bool fresh() {
    bool r = this->_fresh;
    this->_fresh = false;
    return r;
  }

  // CONTROLS
  //

  K32_modulator *faster() { 
    this->_period = max(1, (int)(this->_period/1.2)); 
    //LOG(this->_period);
    return this;
  }
  K32_modulator *slower() { 
    this->_period *= 1.2; 
    return this;
  }
  K32_modulator *bigger() { 
    this->_maxi = min(255, (int)(this->_maxi*1.2));
    return this;
  }
  K32_modulator *smaller() { 
    this->_maxi = max(0, (int)(this->_maxi/1.2));
    return this;
  }


protected:

  int params[MOD_PARAMS_SLOTS];

  // common params
  int _period = 1000;
  int _phase = 0;
  int _mini = 0;
  int _maxi = 255;
  int _MaxValue = 255; // max value to be used in modulator (default = 255)

  // time refs
  unsigned long freezeTime = 0;
  unsigned long triggerTime = 0;
  bool _fresh = false; 
  ModMode _mode = ABSOLUTE;

  cbPtrMod _event = nullptr;

private:

  SemaphoreHandle_t paramInUse;
  String _name = "?";
  bool isRunning = false;

  bool dataslot[ANIM_DATA_SLOTS];
  int _lastProducedValue = 0;

  int usage = 0;
};


//  PERIODIC MODULATORS (LFO)
//
class K32_modulator_periodic : public K32_modulator {
  public:

    // Time is shifted with phase (0->360°) * period()
    virtual int time() 
    {
      return K32_modulator::time()- this->triggerTime - ((this->_phase % 360) * this->_period) / 360;
    }

};


//  TRIGGER MODULATORS
//
class K32_modulator_trigger : public K32_modulator {
  public:

    // Time is referenced to triggerTime (play)
    // Time is shifted with phase as fixed delay
    virtual int time() 
    {
      return K32_modulator::time() - this->triggerTime - this->_phase;
    }

};




#endif