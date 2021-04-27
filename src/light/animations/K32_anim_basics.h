/*
  K32_anim_basics.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_anim_basics_h
#define K32_anim_basics_h

//
// NOTE: to be available, add #include to this file in K32_light.h !
//


//
// TEST
//


class K32_anim_test : public K32_anim {
  public:
    
    // Setup
    void init() {
      this->loop(false);
    }

    // Loop
    void draw (int data[ANIM_DATA_SLOTS])
    {
      #ifdef ELP_Start
      int out_r_g_b[size() * 3] = {0};
      #endif   

      int stepMS = data[0];

      // RED
      //

      this->all( CRGBW{255,0,0} );

      #ifdef ELP_Start
      for (int i=0; i< size() * 3; i++) 
        if (i%3 == 0) out_r_g_b[i] = master();
        else out_r_g_b[i] = 0;

      k32->dmx->setMultiple(out_r_g_b, size()*3, ELP_Start);
      #endif 

      this->pause(stepMS);


      // GREEN 
      //

      this->all( CRGBW{0,255,0} );

      #ifdef ELP_Start
      for (int i=0; i< size() * 3; i++) 
        if (i%3 == 1) out_r_g_b[i] = master();
        else out_r_g_b[i] = 0;
        
      k32->dmx->setMultiple(out_r_g_b, size()*3, ELP_Start);
      #endif 

      this->pause(stepMS);

      // BLUE 
      //

      this->all( CRGBW{0,0,255} );

      #ifdef ELP_Start
      for (int i=0; i< size() * 3; i++) 
        if (i%3 == 2) out_r_g_b[i] = master();
        else out_r_g_b[i] = 0;
        
      k32->dmx->setMultiple(out_r_g_b, size()*3, ELP_Start);
      #endif 

      this->pause(stepMS);

      // WHITE 
      //

      this->all( CRGBW{0,0,0,255} );

      #ifdef ELP_Start
      for (int i=0; i< size() * 3; i++) 
        out_r_g_b[i] = master();
        
      k32->dmx->setMultiple(out_r_g_b, size()*3, ELP_Start);
      #endif 

      this->pause(stepMS);

      #ifdef ELP_Start
      for (int i=0; i< size() * 3; i++) 
        out_r_g_b[i] = 0;
        
      k32->dmx->setMultiple(out_r_g_b, size()*3, ELP_Start);
      #endif 

      this->clear();
    };
};


//
// FULLCOLOR
//
class K32_anim_color : public K32_anim {
  public:

    // Setup
    void init() {}

    // Loop
    void draw(int data[ANIM_DATA_SLOTS])
    {
      CRGBW color {data[0], data[1], data[2], data[3]};
      this->all( color );
    };
};




#endif
