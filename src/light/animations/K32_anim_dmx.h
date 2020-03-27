/*
  K32_anim_generic.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_anim_dmx_h
#define K32_anim_dmx_h

#include "K32_anim.h"

//
// NOTE: to be able to load an animation by name, it must be registered in K32_animbook.h
//


// OUTILS
//

enum color_mode { 
  COLOR_NORM    = 0,      
  COLOR_PICKER  = 1,    
  COLOR_SD      = 2        
};

CRGBW colorPreset[25] = {
  {CRGBW::Black},         // 0
  {CRGBW::Red},           // 1
  {CRGBW::Green},         // 2
  {CRGBW::Blue},          // 3
  {CRGBW::White},         // 4
  {CRGBW::Yellow},        // 5
  {CRGBW::Magenta},       // 6
  {CRGBW::Cyan},          // 7
  {CRGBW::Orange}        // 8
};

// Transform DMX range into simple value, I.E.  0-10 => 0, 11-20 => 1, 21-30 => 2, ....
inline int simplifyDmxRange(int value) {
  if (value > 0) value -= 1;
  return value/10;
}


// ANIM DMX
//

class K32_anim_dmx : public K32_anim {
  public:

    // Set Name
    K32_anim_dmx() : K32_anim("dmx") {}

    // Loop
    bool loop ( K32_ledstrip* strip ){
      
      //
      // ONDMXFRAME PUSH
      //
      this->waitData();                   // wait for a data refresh (pushed by ArtNet or manually)
      
      // Master
      int master      = this->data[0];
      
      // Mirror & Zoom -> Segment size
      int mirrorMode  = simplifyDmxRange(this->data[15]);
      int zoomedSize  = min(1, ( strip->size() * this->data[16] )/255 );

      int segmentSize = zoomedSize;
      if (mirrorMode == 1 || mirrorMode == 4)       segmentSize /= 2;
      else if (mirrorMode == 2 || mirrorMode == 5)  segmentSize /= 3;
      else if (mirrorMode == 3 || mirrorMode == 6)  segmentSize /= 4;

      // Create Segment Buffer
      CRGBW segment[segmentSize];

      // Modes
      int colorMode   = simplifyDmxRange(this->data[14]);
      int pixMode     = simplifyDmxRange(this->data[5]);

      
      // Color mode NORM
      //
      if (colorMode == COLOR_NORM) {
        
        // Pix Length
        int pixLength  = min(1, (segmentSize * data[6]) / 255);         // pix_start

        // Bi-color
        //
        if (pixMode < 23) {
          
          // Colors
          CRGBW color1 {this->data[1], this->data[2], this->data[3], this->data[4]};
          CRGBW color2 {this->data[1], this->data[2], this->data[3], this->data[4]};
          
          // Pix Offset 
          int pixOffset  =  (segmentSize * data[7]) / 255;                // pix_pos

          // Fix = only color1
          if (pixMode == 0) 
          {
            for(int i=0; i<segmentSize; i++) segment[i] = color1;
          }

          // Ruban = color1 one-dash / color2 background
          if (pixMode == 1) 
          {
            for(int i=0; i<segmentSize; i++) 
            {
              if (i >= pixOffset && i < pixOffset+pixLength) segment[i] = color1;
              else segment[i] = color2;
            }
          }

          // 01:02 = color1 + color2 dash 
          if (pixMode == 2) 
          {
            pixLength = data[6];   // on this one, length is absolute and not relative to segementSize

            for(int i=0; i<segmentSize; i++) 
            {
              if ( (i+pixOffset)/pixLength % 2 == 0 ) segment[i] = color1;
              else segment[i] = color2;
            }
          }

          // rus> + rusf> = color1 one-dash fade or blend R / color2 background
          if (pixMode == 3 || pixMode == 9) 
          {
            for(int i=0; i<segmentSize; i++) 
            {
              int coef = ((pixOffset+pixLength-i) * 255 ) / pixLength;
              coef = (coef*coef)/255;

              if (i >= pixOffset && i < pixOffset+pixLength) {
                segment[i] = color1 % coef;
                if (pixMode == 9) segment[i] += color2 % (255-coef);
              }
              else segment[i] = color2;
            }
          }

          // rus< + rusf< = color1 one-dash fade or blend L / color2 background
          if (pixMode == 4 || pixMode == 10) 
          {
            for(int i=0; i<segmentSize; i++) 
            {
              int coef = ((i-pixOffset) * 255 ) / pixLength;
              coef = (coef*coef)/255;

              if (i >= pixOffset && i < pixOffset+pixLength) {
                segment[i] = color1 % coef;
                if (pixMode == 10) segment[i] += color2 % (255-coef);
              }
              else segment[i] = color2;
            }
          }

          // rus<> + rusf<> = color1 one-dash fade or blend LR / color2 background
          if (pixMode == 5 || pixMode == 11) 
          {
            for(int i=0; i<segmentSize; i++) 
            {
              int coefL = ((i-pixOffset) * 255 ) / (pixLength/2);
              coefL = (coefL*coefL)/255;

              int coefR = ((pixOffset+pixLength-i) * 255 ) / (pixLength/2);
              coefR = (coefR*coefR)/255;

              if (i >= pixOffset && i < pixOffset+pixLength/2) {
                segment[i] = color1 % coefL;
                if (pixMode == 11) segment[i] += color2 % (255-coefL); 
              }
              else if (i >= pixOffset+pixLength/2 && i < pixOffset+pixLength) {
                segment[i] = color1 % coefR;
                if (pixMode == 11) segment[i] += color2 % (255-coefR); 
              }
              else segment[i] = color2;
            }
          }

        }

        // Tri-color + Quadri-color
        //
        else if (pixMode == 23 || pixMode == 24) {
          
          // Colors
          CRGBW color1 { colorPreset[ simplifyDmxRange(this->data[1]) ] };
          CRGBW color2 { colorPreset[ simplifyDmxRange(this->data[2]) ] };
          CRGBW color3 { colorPreset[ simplifyDmxRange(this->data[3]) ] };
          CRGBW color4 { colorPreset[ simplifyDmxRange(this->data[4]) ] };

          // Pix Offset 
          int rap_tri    =  map(data[adr + 5], 0, 255, 0, NUM_LEDS_PER_STRIP * 2);
          int pixOffset  =  ((NUM_LEDS_PER_STRIP / 2 - rap_tri / 2) + map(pix_pos_v, 0, 255, -NUM_LEDS_PER_STRIP + 1, NUM_LEDS_PER_STRIP + 1));  // pix_pos

          // NOT FINNISHED !

        }       

      }

      // Color mode PICKER
      else if (colorMode == COLOR_PICKER) {


      }

      // Color mode SD
      else if (colorMode == COLOR_SD) {


      }


      
      int strobeMode  = simplifyDmxRange(this->data[8]);
      int strobeSeuil = (this->data[8] - 10*strobeMode)*4;
      int strobeSpeed = this->data[9];
      





      // LOOP AND WAIT NEXT REFRESH
      return true;    
    };

};


#endif
