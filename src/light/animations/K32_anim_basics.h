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

    // Image generator
    void draw ()
    {
      uint8_t master = data[0];
      int stepMS = data[1];

      this->all( ( CRGBW(CRGBW::Red) % master) );
      delay(stepMS);

      this->all( ( CRGBW(CRGBW::Green) % master) );
      delay(stepMS);

      this->all( ( CRGBW(CRGBW::Blue) % master) );
      delay(stepMS);

      this->all( ( CRGBW{0,0,0,255} % master) );
      delay(stepMS);

      this->clear();
    };
};


//
// FULLCOLOR
//
class K32_anim_color : public K32_anim {
  public:
    
    int& master = data[0];
    int& red    = data[1];
    int& green  = data[2];
    int& blue   = data[3];
    int& white  = data[4];

    // Setup
    void init() {}

    // Loop
    void draw()
    {
      CRGBW color {data[1], data[2], data[3], data[4]};
      color %= data[0];      

      this->all( color );
    };
};




#endif
