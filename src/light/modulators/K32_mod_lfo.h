/*
  K32_mod_lfo.h
  Created by Thomas BOHL, March 2020.
  Released under GPL v3.0
*/
#ifndef K32_mod_lfo_h
#define K32_mod_lfo_h

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
                                      if using K32_modulator_periodic: time is corrected with phase (1/360 of period)
                                      
  float progress()                = % of progress in period between 0.0 and 1.0 
  int periodCount()               = count the number of period iteration since esp start

Modulator can also access / modify those attributes:

  int params[MOD_PARAMS_SLOTS]    = modulator parameters, set by external users, can be renamed for convenience usint local int& attribute
  int anim_data[ANIM_DATA_SLOTS]  = pointer to animation data, can be used for calculation. It can also manipulate this data, use carefully !

*/


//
// SINUS
//
class K32_mod_sinus : public K32_modulator_periodic {
  public:  
    
    void modulate( int& data )
    {
      data = (0.5f + 0.5f * sin(2 * PI * progress())) * amplitude() + mini();
    };
  
};

//
// TRIANGLE
//
class K32_mod_triangle : public K32_modulator_periodic {
  public:  

    void modulate ( int& data )
    { 
      float percent = progress();
      if (percent > 0.5) percent = 1 - percent;

      data = 2*percent * amplitude() + mini();
    };
  
};

//
// SAW TOOTH
//
class K32_mod_sawtooth : public K32_modulator_periodic {
  public:  

    void modulate ( int& data )
    {
      data = progress() * amplitude() + mini();
    };
  
};

//
// SAW TOOTH INVERTED
//
class K32_mod_isawtooth : public K32_modulator_periodic {
  public:  

    void modulate ( int& data )
    {
      data = maxi() - progress() * amplitude();
    };

};

//
// PULSE
//
class K32_mod_pulse : public K32_modulator_periodic {
  public:  

    // named link to params 
    int& widthMS  = params[0];  // pulse ON width in milliseconds
    int& widthPCT = params[1];  // pulse ON width in percentage (activated if widthMS = 0)

    void modulate ( int& data )
    {  
      int width = widthMS;
      if (widthMS == 0) width = period()*widthPCT/100;

      if ( time() % period() < width) data = maxi();
      else data = mini();
    };

};


//
// RANDOM
//
class K32_mod_random : public K32_modulator_periodic {
  public:  

    // internal attribute
    int lastPeriod;

    void modulate( int& data )
    {
      int newPeriod = periodCount();
      if (newPeriod != lastPeriod) {
        lastPeriod = newPeriod;
        data = random(mini(), maxi());
      }
    };
  
};

#endif
