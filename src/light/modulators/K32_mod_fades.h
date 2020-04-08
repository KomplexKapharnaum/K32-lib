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

  uint32_t time()                 = current time if modulator is playing or freezeTime if mod is paused. 
                                      if using K32_modulator_trigger: time is based on play() time and corrected with phase as fixed delay (ms)
                                      
  float progress()                = % of progress in period between 0.0 and 1.0 
  int periodCount()               = count the number of period iteration since esp start

Modulator can also access / modify those attributes:

  int params[MOD_PARAMS_SLOTS]    = modulator parameters, set by external users, can be renamed for convenience usint local int& attribute
  int anim_data[LEDS_DATA_SLOTS]  = pointer to animation data, can be used for calculation. It can also manipulate this data, use carefully !

*/


//
// FADE IN
//
class K32_mod_fadein : public K32_modulator_trigger {
  public:  
    
    bool engage = false;

    void modulate( int& data )
    {   
      // not yet ready
      if (time() < 0) return; 

      // end of modulation
      if (time() >= period()) 
      {
        data = maxi();
        stop();
      }
      
      // set initial data value as fadein start
      if (!engage) {        
        engage = true; 
        mini(data); 
      }
      
      data = time() * amplitude() / period() + mini();

    };
};

//
// FADE OUT
//
class K32_mod_fadeout : public K32_modulator_trigger {
  public:  
    
    bool engage = false;

    void modulate( int& data )
    { 
      // not yet ready
      if (time() < 0) return; 

      // end of modulation
      if (time() >= period()) 
      {
        data = mini();
        stop();
      }
      
      // set initial data value as fadeout start
      if (!engage) {        
        engage = true; 
        maxi(data); 
      }
      
      data = maxi() - time() * amplitude() / period();
      
    };
};

#endif