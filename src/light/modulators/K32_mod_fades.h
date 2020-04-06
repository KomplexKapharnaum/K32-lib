/*
  K32_mod_fades.h
  Created by Thomas BOHL, March 2020.
  Released under GPL v3.0
*/
#ifndef K32_mod_fades_h
#define K32_mod_fades_h

#include "../K32_anim.h"

//
// NOTE: to be available, add #include to this file in K32_light.h !
//


/*
USAGE: 
Modulators needs to define at least the modulate( int& data ) method,
the data reference corresponds to the animation data slot to which de modulator is attached. 

Modulator has access to this helper methods:

  int period()                    = period length
  int phase()                     = phase value
  int maxi()                      = maximum value
  int mini()                      = minimum value
  int amplitude()                 = maxi-mini

  void useAbsoluteTime()          = time reference boot time
  void useTriggerTime()           = time reference is modulator play() call
  void applyPhase360()            = apply phase shift to time calculation, phase shift is equal to period*phase/360

  uint32_t time()                 = current time if modulator is playing or freezeTime if mod is paused.
  int phaseTime()                 = time for phase as deg angle (360°), relative to period 
  uint32_t timePeriod()           = time ellapsed relative to current period
  float progress()                = % of progress in period between 0.0 and 1.0 
  int periodCount()               = count the number of period iteration since esp start

Modulator can also access / modify those attributes:

  int params[MOD_PARAMS_SLOTS]    = modulator parameters, set by external users, can be renamed for convenience usint local int& attribute
  int anim_data[LEDS_DATA_SLOTS]  = pointer to animation data, can be used for calculation. It can also manipulate this data, use carefully !

*/


//
// FADE IN
//
class K32_mod_fadein : public K32_modulator {
  public:  
    
    bool engage = false;

    void modulate( int& data )
    { 
      useTriggerTime();
      int delay = phase();

      if (time() < delay) return;
  
      else if (time() >= period()+delay) {
        data = maxi();
        stop();
        return;
      }
      
      if (!engage) {        // set initial data value as fadein start
        engage = true; 
        mini(data); 
      }
      
      data = (time()-delay) * amplitude() / period() + mini();
    };
};

//
// FADE OUT
//
class K32_mod_fadeout : public K32_modulator {
  public:  
    
    bool engage = false;

    void modulate( int& data )
    { 
      useTriggerTime();
      int delay = phase();

      if (time() < delay) return;
  
      else if (time() >= period()+delay) {
        data = mini();
        stop();
        return;
      }
      
      if (!engage) {        // set initial data value as fadeout start
        engage = true; 
        maxi(data); 
      }
        
      data = maxi() - (time()-delay) * amplitude() / period();
      
    };
};

#endif