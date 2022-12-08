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

class Anim_datathru  : public K32_anim
{

public:
    // Loop
    void draw(int data[ANIM_DATA_SLOTS])
    {
        int pixCount = min(ANIM_DATA_SLOTS/4, size());

        this->clear();

        // Group 4 consecutive data to one RGBW pixel
        for (int k=0; k<pixCount; k++) 
            this->pixel(k, CRGBW(data[k*4], data[k*4+1], data[k*4+2], data[k*4+3]) );

        // Pixel to DMX will be handled by the fixture
    }
};


//
// TEST
//
class Anim_test_strip : public K32_anim {
  public:
    
    // Setup
    void init() {
      this->loop(false);
    }

    // Loop
    void draw (int data[ANIM_DATA_SLOTS])
    {
      int stepMS = data[0];

      // RED
      //

      this->all( CRGBW{255,0,0} );
      this->pause(stepMS);


      // GREEN 
      //

      this->all( CRGBW{0,255,0} );
      this->pause(stepMS);

      // BLUE 
      //

      this->all( CRGBW{0,0,255} );
      this->pause(stepMS);

      // WHITE 
      //
      this->all( CRGBW{0,0,0,255} );
      this->pause(stepMS);

      this->clear();
    };
};


class Anim_test_pwm : public K32_anim {
  public:
    
    // Setup
    void init() {
      this->loop(false);
    }

    // Loop
    void draw (int data[ANIM_DATA_SLOTS])
    {
      int stepMS = data[0];

      this->all( CRGBW(255, 0, 255, 0) );
      this->pause(stepMS);

      this->all( CRGBW(0, 255, 0, 255) );
      this->pause(stepMS);

      this->all( CRGBW(127, 127, 127, 127) );
      this->pause(stepMS);
  
      this->clear();
    };
};

//
// TEST
//
class Anim_flash : public K32_anim {
  public:
    
    // Setup
    void init() {
      this->loop(false);
    }

    // Loop
    void draw (int data[ANIM_DATA_SLOTS])
    {
      // int stepMS = data[1];
      int count = data[0];
      int onTime = data[1];
      int stepMS = 300;

      this->clear();
      
      // WHITE FLASH
      //
      int counter = 0;
      while(counter < count) {
        this->all( CRGBW{255,255,255} );
        this->pause(onTime);
        this->clear();
        this->pause(stepMS);
        counter++;
      }
    };
};


#endif
