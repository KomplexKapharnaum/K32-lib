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

static  CRGBW colorPreset[25] = {
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

      //
      // GENERATE BASE SEGMENT
      //

      // Color mode NORM
      //
      if (colorMode == COLOR_NORM) {

        // Bi-color
        //
        if (pixMode < 23) {
          
          // Colors
          CRGBW color1 {this->data[1], this->data[2], this->data[3], this->data[4]};
          CRGBW color2 {this->data[10], this->data[11], this->data[12], this->data[13]};

          // Dash Length + Offset
          int dashLength  = min(1, (segmentSize * data[6]) / 255);         // pix_start
          int dashOffset  =  (segmentSize * data[7]) / 255;                // pix_pos
          
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
              if (i >= dashOffset && i < dashOffset+dashLength) segment[i] = color1;
              else segment[i] = color2;
            }
          }

          // 01:02 = color1 + color2 dash 
          if (pixMode == 2) 
          {
            dashLength = data[6];   // on this one, length is absolute and not relative to segementSize

            for(int i=0; i<segmentSize; i++) 
            {
              if ( (i+dashOffset)/dashLength % 2 == 0 ) segment[i] = color1;
              else segment[i] = color2;
            }
          }

          // rus> + rusf> = color1 one-dash fade or blend R / color2 background
          if (pixMode == 3 || pixMode == 9) 
          {
            for(int i=0; i<segmentSize; i++) 
            {
              int coef = ((dashOffset+dashLength-i) * 255 ) / dashLength;
              coef = (coef*coef)/255;

              if (i >= dashOffset && i < dashOffset+dashLength) {
                segment[i] = color1 % (uint8_t)coef;
                if (pixMode == 9) segment[i] += color2 % (uint8_t)(255-coef);
              }
              else segment[i] = color2;
            }
          }

          // rus< + rusf< = color1 one-dash fade or blend L / color2 background
          if (pixMode == 4 || pixMode == 10) 
          {
            for(int i=0; i<segmentSize; i++) 
            {
              int coef = ((i-dashOffset) * 255 ) / dashLength;
              coef = (coef*coef)/255;

              if (i >= dashOffset && i < dashOffset+dashLength) {
                segment[i] = color1 % (uint8_t)coef;
                if (pixMode == 10) segment[i] += color2 % (uint8_t)(255-coef);
              }
              else segment[i] = color2;
            }
          }

          // rus<> + rusf<> = color1 one-dash fade or blend LR / color2 background
          if (pixMode == 5 || pixMode == 11) 
          {
            for(int i=0; i<segmentSize; i++) 
            {
              int coefL = ((i-dashOffset) * 255 ) / (dashLength/2);
              coefL = (coefL*coefL)/255;

              int coefR = ((dashOffset+dashLength-i) * 255 ) / (dashLength/2);
              coefR = (coefR*coefR)/255;

              if (i >= dashOffset && i < dashOffset+dashLength/2) {
                segment[i] = color1 % (uint8_t)coefL;
                if (pixMode == 11) segment[i] += color2 % (uint8_t)(255-coefL); 
              }
              else if (i >= dashOffset+dashLength/2 && i < dashOffset+dashLength) {
                segment[i] = color1 % (uint8_t)coefR;
                if (pixMode == 11) segment[i] += color2 % (uint8_t)(255-coefR); 
              }
              else segment[i] = color2;
            }
          }

        }

        // Tri-color + Quadri-color 
        //
        else if (pixMode == 23 || pixMode == 24) {
          
          // Colors
          CRGBW color[5] = {
            {this->data[10], this->data[11], this->data[12], this->data[13]}, // background
            colorPreset[ simplifyDmxRange(this->data[1]) ],                   // color1
            colorPreset[ simplifyDmxRange(this->data[2]) ],                   // color2
            colorPreset[ simplifyDmxRange(this->data[3]) ],                   // color3
            colorPreset[ simplifyDmxRange(this->data[4]) ]                    // color4
          };

          // Dash Length + Offset + Split 
          int dashLength  = min(3, (3 * segmentSize * data[6]) / 255);          
          int dashOffset  = ((segmentSize-dashLength) * data[7]) / 255;
          int dashSplit = pixMode % 20;
          int dashPart  = 0;

          // Multi-color dash / color0 backgound
          for(int i=0; i<segmentSize; i++) 
          {
            dashPart = (i-dashOffset)*dashSplit/dashLength + 1;       // find in which part of the dash we are
            if (dashPart < 0 || dashPart > dashSplit) dashPart = 0;   // use 0 if outside of the dash

            segment[i] = color[ dashPart ];
          }          

        }       

      }

      // Color mode PICKER
      else if (colorMode == COLOR_PICKER) {

        // Hue range
        uint8_t hueStart = this->data[10];
        uint8_t hueEnd = this->data[11];

        // Channel master
        CRGBW rgbwMaster {this->data[1], this->data[2], this->data[3], this->data[4]};
        
        // Color wheel
        CRGBW colorWheel;
        for(int i=0; i<segmentSize; i++) {
          segment[i] = colorWheel.setHue( hueStart + ((hueEnd - hueStart) * i) / segmentSize );
          segment[i] %= rgbwMaster;
        }

      }

      // Color mode SD
      else if (colorMode == COLOR_SD) {
        // TODO: load image from SD !
      }


      //
      // APPLY STROBE
      //
      // int strobeMode  = simplifyDmxRange(this->data[8]);
      // int strobeSeuil = (this->data[8] - 10*strobeMode)*4;
      // int strobeSpeed = this->data[9];
      
      // TODO: use MODULATOR      


      //
      // APPLY MASTER
      //
      uint8_t master = this->data[0];
      for(int i=0; i<segmentSize; i++) segment[i] %= master;


      //
      // DRAW ON STRIP WITH ZOOM & MIRROR
      //

      // Empty the whole strip
      strip->clear();
      
      // Export color segment into pixels
      pixelColor_t pixels[segmentSize];
      for(int i=0; i<segmentSize; i++) 
        pixels[i] = segment[i].getPixel();

      // Mirroring alternate (1 = copy, 2 = alternate)
      int mirrorAlternate = 1 + (mirrorMode == 1 || mirrorMode == 2 || mirrorMode == 3); 

      // Zoom offset
      int zoomOffset = (strip->size() - zoomedSize);

      // Copy pixels into strip
      for(int i=0; i<zoomedSize; i++) 
      {
        int pix  = i % segmentSize;           // pix cursor into segment
        int iter = i / segmentSize;           // count of mirror copy 
        
        if (iter && iter % mirrorAlternate)   // alternate: invert pix cursor
          pix = segmentSize - pix - 1;    

        strip->pix(i+zoomOffset, pixels[ pix ]);
      }      

      // Show !
      strip->show();

      // Loop and wait for new data
      return true;    

    }

};


#endif
