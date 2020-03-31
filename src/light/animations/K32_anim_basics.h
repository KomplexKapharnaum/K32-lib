/*
  K32_anim_basics.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_anim_basics_h
#define K32_anim_basics_h

//
// NOTE: to be able to load a generator by name, it must be registered in K32_animbook class
//


//
// TEST
//


class K32_anim_test : public K32_anim {
  public:

    // Set Name & init data
    K32_anim_test() : K32_anim("test") 
    {
      data[0] = 50;       // brightness
      data[1] = 250;      // step duration

      this->loop(false);  // disable loop: it's a one time generator !
    }
    

    // Image generator
    void gener8 ( K32_ledstrip* strip )
    {
      uint8_t master = this->data[0];
      int stepMS = this->data[1];

      strip->all( ( CRGBW(CRGBW::Red) % master) );
      strip->show();
      delay(stepMS);

      strip->all( ( CRGBW(CRGBW::Green) % master) );
      strip->show();
      delay(stepMS);

      strip->all( ( CRGBW(CRGBW::Blue) % master) );
      strip->show();
      delay(stepMS);

      strip->all( ( CRGBW{0,0,0,255} % master) );
      strip->show();
      delay(stepMS);

      strip->black();
    };
};


//
// FULLCOLOR
//
class K32_anim_color : public K32_anim {
  public:

    // Set Name & init data
    K32_anim_color() : K32_anim("color") 
    {
      data[0] = 255;  // master
      data[1] = 255;  // red
      data[2] = 255;  // green
      data[3] = 255;  // blue
      data[4] = 255;  // white
    }


    // Loop
    void gener8( K32_ledstrip* strip )
    {
      CRGBW color {this->data[1], this->data[2], this->data[3], this->data[4]};
      color %= this->data[0];      

      strip->all( color );
      strip->show();
    };
};


/*
//
// SINUS
//
class K32_anim_sinus : public K32_anim {
  public:

    // Set Name
    K32_anim_sinus() : K32_anim("sinus") {
      this->params[0] = 2000; // period
      this->params[1] = 255;  // intensity max
      this->params[2] = 0;    // intensity min
      this->params[3] = 255;  // red
      this->params[4] = 255;  // green
      this->params[5] = 255;  // blue
      this->params[6] = 255;  // white
      this->params[7] = 0;    // duration (seconds) 0 = infinite
    }




    bool loop ( K32_ledstrip* strip ){

      float factor = 0;
      unsigned long start = millis();
      unsigned long progress = 0;
      pixelColor_t color;

      // LOG("LEDS sinus");

      while (progress <= this->params[0]) {

        factor = ((0.5f + 0.5f * sin( 2 * PI * progress / this->params[0] - 0.5f * PI ) ) * (this->params[1]-this->params[2]) + this->params[2]) / 255;
        color = pixelFromRGBW( (int)(this->params[3]*factor),  (int)(this->params[4]*factor),  (int)(this->params[5]*factor),  (int)(this->params[6]*factor) ); 
        
        strip->all( color )->show();

        vTaskDelay(pdMS_TO_TICKS(1));
        progress = millis() - start;
      }

      // ANIMATION Timeout
      if (this->params[7] > 0)
        if (millis() > (this->startTime+this->params[7]*1000)) {
          
          strip->black();
          
          return false;
        }

      return true;    // LOOP !

    };
};


//
// STROBE
//
class K32_anim_strobe : public K32_gen {
  public:
    K32_anim_strobe() : K32_gen("strobe") {
      this->params[0] = 2000; // period
      this->params[1] = 50;   // % ON
    }

    bool loop ( K32_ledstrip* strip ){

      int intensity = 255;
      pixelColor_t color = pixelFromRGBW(intensity, intensity, intensity, intensity);

      TickType_t xOn = max(1, (int)pdMS_TO_TICKS( (this->params[0]*this->params[1]/100) ));
      TickType_t xOff = max(1, (int)pdMS_TO_TICKS( this->params[0] - (this->params[0]*this->params[1]/100) ));

      strip->all( color )->show();
    
      vTaskDelay( xOn );

      strip->black();

      vTaskDelay( xOff );

      return true;    // LOOP !

    };
};


//
// HARDSTROBE
//
class K32_anim_hardstrobe : public K32_gen {
  public:
    K32_anim_hardstrobe() : K32_gen("hardstrobe") {
      this->params[0] = 2000; // period
      this->params[1] = 50;   // % ON

      this->nextEvent = 0;
      state = false;
    }

    bool loop ( K32_ledstrip* strip ){

      int intensity = 255;
      pixelColor_t color = pixelFromRGBW(intensity, intensity, intensity, intensity);

      // LOGINL(millis()); LOGINL(" // "); LOG(this->nextEvent);
      if (millis() > this->nextEvent) {
        // OFF -> ON
        if(!state) {
          this->nextEvent = millis() + (this->params[0]*this->params[1]/100) ;
          
          
          strip->all( color )->show();
          
        }
        // ON -> OFF
        else {
          this->nextEvent = millis() + this->params[0] - (this->params[0]*this->params[1]/100);
          strip->black();
        }
        state = !state;
      }

      return true;    // LOOP !

    };

    long nextEvent;
    bool state;
};

//
// CHASER
//
class K32_anim_chaser : public K32_gen {
  public:
    K32_anim_chaser() : K32_gen("chaser") {
      this->params[0] = 10; // step duration
      this->params[1] = 255;  // intensity max
      this->params[2] = 0;    // intensity min
      this->params[3] = 255;  // red
      this->params[4] = 255;  // green
      this->params[5] = 255;  // blue
      this->params[6] = 255;  // white
      this->params[7] = 10; // length of chaser
      this->params[8] = LEDSTRIP_MAXPIXEL; // length of led strip
      this->params[9] = 0;    // duration (seconds) 0 = infinite
    }

    bool loop ( K32_ledstrip* strip ){

      int progress = 0 ;

      pixelColor_t black = pixelFromRGBW(0,0,0,0);
      pixelColor_t color = pixelFromRGBW(params[3],params[4],params[5],params[6]);

      for ( progress = 0; progress < this-> params[8] + this->params[7]; progress ++)
      {   
          
          for (int i = 0 ; i <= progress ; i ++)
          {
            if (i > progress - this->params[7]) strip->pix(i,color);
            else strip->pix(i, black);
          }
          strip->show();
          
        vTaskDelay(pdMS_TO_TICKS(this->params[0]));
      }
      strip->black();

      // ANIMATION Timeout
      if (this->params[9] > 0)
        if (millis() > (this->startTime+this->params[7]*1000)) {

          strip->black();

          return false;
        }

      return true;    // LOOP !

    };
};

*/

#endif
