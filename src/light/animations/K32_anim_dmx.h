/*
  K32_anim_basics.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_anim_dmx_h
#define K32_anim_dmx_h

#define STROB_ON_MS  2000/LEDS_SHOW_FPS

//
// NOTE: to be available, add #include to this file in K32_light.h !
//


// OUTILS
//

enum color_mode { 
  COLOR_BI,
  COLOR_TRI,
  COLOR_QUAD,      
  COLOR_PICKER,    
  COLOR_SD        
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
  {CRGBW::Orange}         // 8
};


// Transform DMX range into simple value, I.E.  0-10 => 0, 11-20 => 1, 21-30 => 2, ....
inline int simplifyDmxRange(int value) {
  if (value > 0) value -= 1;
  return value/10;
}

// Return rounded value after dividing by 255
inline int roundDivInt(int a, int b) {
  return a/b + ((a%b)>(b/2));
}

// Scale a number based on a 8bit value
inline int scale255(int a, uint8_t value) {
  return roundDivInt( a*value, 255 );
}

// Check if int is min <= value <= max
inline bool btw(int value, int min, int max) {
  return (value >= min) && (value <= max);
}

// ANIM DMX
//

class K32_anim_dmx : public K32_anim {
  public:

    // Strobe modulator (pulse)
    K32_mod_pulse* strobe{};

    // Smooth modulator (sinus)
    K32_mod_sinus* smooth{};


    // Loop
    void draw ()
    {
      //
      // ONDMXFRAME PUSH
      //
      
      // Mirror & Zoom -> Segment size
      int mirrorMode  = simplifyDmxRange(data[14]);
      int zoomedSize  = max(1, scale255( size(), data[15]) );

      int segmentSize = zoomedSize;
      if (mirrorMode == 1 || mirrorMode == 4)       segmentSize /= 2;
      else if (mirrorMode == 2 || mirrorMode == 5)  segmentSize /= 3;
      else if (mirrorMode == 3 || mirrorMode == 6)  segmentSize /= 4;

      // Modes
      int pixMode     = simplifyDmxRange(data[5]);

      int colorMode = COLOR_BI;
      if (pixMode == 23) colorMode = COLOR_TRI;
      else if (pixMode == 24) colorMode = COLOR_QUAD;
      else if (btw(pixMode, 6, 11)) colorMode = COLOR_PICKER; 
      else if (btw(pixMode, 12, 17)) colorMode = COLOR_SD;


      // Segment Buffer
      CRGBW segment[segmentSize];


      // PRIMARY COLOR
      /////////////////////////////////////////////////////////////////////////

      // Color mode BI
      if (colorMode == COLOR_BI)
      {
        CRGBW color1 {data[1], data[2], data[3], data[4]};
        for(int i=0; i<segmentSize; i++) segment[i] = color1;
      }

      // Color mode PICKER
      else if (colorMode == COLOR_PICKER) {

        // Hue range
        int hueStart = data[10] + data[7];
        int hueEnd = data[11] + data[7];

        // Channel master
        CRGBW rgbwMaster {data[1], data[2], data[3], data[4]};
        
        // Color wheel
        CRGBW colorWheel;
        for(int i=0; i<segmentSize; i++) {
          segment[i] = colorWheel.setHue( (hueStart + ((hueEnd - hueStart) * i) / segmentSize) );
          segment[i] %= rgbwMaster;
        }

      }

      // Color mode SD
      else if (colorMode == COLOR_SD) {
        // TODO: load image from SD !


      }
      
      // Color mode TRI / QUADRI
      else if (colorMode == COLOR_TRI || colorMode == COLOR_QUAD) {
        
        // Colors
        CRGBW color[5] = {
          {data[10], data[11], data[12], data[13]}, // background     // color[0]
          colorPreset[ simplifyDmxRange(data[1]) ],                   // color[1]
          colorPreset[ simplifyDmxRange(data[2]) ],                   // color[2]
          colorPreset[ simplifyDmxRange(data[3]) ],                   // color[3]
          colorPreset[ simplifyDmxRange(data[4]) ]                    // color[4]
        };

        // Dash Length + Offset + Split 
        int dashLength  = max(3, scale255(2 * segmentSize, data[6]));          
        int dashOffset  = scale255((segmentSize-dashLength), data[7]);
        int dashSplit = (colorMode == COLOR_QUAD) ? 4 : 3;
        int dashPart  = 0;

        // Multi-color dash / color0 backgound
        for(int i=0; i<segmentSize; i++) 
        {
          dashPart = (i-dashOffset)*dashSplit/dashLength + 1;       // find in which part of the dash we are
          if (dashPart < 0 || dashPart > dashSplit) dashPart = 0;   // use 0 if outside of the dash (=> background)

          segment[i] = color[ dashPart ];
        }          

      }   


      // APPLY PIX MOD BACKGROUND
      /////////////////////////////////////////////////////////////////////////

      if (colorMode == COLOR_BI || colorMode == COLOR_PICKER) {

        // Background Color
        CRGBW backColor {CRGBW::Black};
        if (colorMode == COLOR_BI) backColor = CRGBW{data[10], data[11], data[12], data[13]};

        // Dash Length + Offset
        int dashLength  = max(1, scale255(segmentSize, data[6]) );          // pix_start
        int dashOffset  =  scale255(segmentSize, data[7]);                  // pix_pos

        // 2 colors = color1 one-dash / backColor background
        if (pixMode == 1 || pixMode == 7) 
        {
          for(int i=0; i<segmentSize; i++) 
            if (i < dashOffset && i >= dashOffset+dashLength) segment[i] = backColor;
        }

        // 01:02 = color1 + backColor dash 
        if (pixMode == 2 || pixMode == 8) 
        {
          dashLength = data[6];   // on this one, length is absolute and not relative to segementSize

          for(int i=0; i<segmentSize; i++) 
            if ( (i+dashOffset)/dashLength % 2 == 1 ) segment[i] = backColor;
        }

        // rusf>  = color1 one-dash fade R / backColor background
        else if (pixMode == 3 || pixMode == 9) 
        {
          for(int i=0; i<segmentSize; i++) 
          {
            if (i >= dashOffset && i < dashOffset+dashLength) 
            {
              int coef = ((dashOffset+dashLength-1-i) * 255 ) / (dashLength-1);

              segment[i] %= (uint8_t)coef;
              segment[i] += backColor % (uint8_t)(255-coef);
            }
            else segment[i] = backColor;
          }
        }

        // rusf<  = color1 one-dash fade L / color2 background
        else if (pixMode == 4 || pixMode == 10) 
        {
          for(int i=0; i<segmentSize; i++) 
          {
            if (i >= dashOffset && i < dashOffset+dashLength) 
            {
              int coef = ((i-dashOffset) * 255 ) / (dashLength-1);

              segment[i] %= (uint8_t)coef;
              segment[i] += backColor % (uint8_t)(255-coef);
            }
            else segment[i] = backColor;
          }
        }

        // rusf<>  = color1 one-dash fade LR / color2 background
        else if (pixMode == 5 || pixMode == 11) 
        {
          for(int i=0; i<segmentSize; i++) 
          { 
            if (i >= dashOffset && i < dashOffset+dashLength/2) 
            {
              int coef = ((i-dashOffset) * 255 ) / (dashLength/2);

              segment[i] %= (uint8_t)coef;
              segment[i] += backColor % (uint8_t)(255-coef);
            }
            else if (i >= dashOffset+dashLength/2 && i < dashOffset+dashLength) 
            {
              int coef = ((dashOffset+dashLength-1-i) * 255 ) / (dashLength/2);

              segment[i] %= (uint8_t)coef;
              segment[i] += backColor % (uint8_t)(255-coef);
            }
            else segment[i] = backColor;
          }
        }

      }


      // STROBE
      /////////////////////////////////////////////////////////////////////////
    
      int strobeMode  = simplifyDmxRange(data[8]);
      int strobePeriod = map(data[9]*data[9], 0, 255*255, STROB_ON_MS*4, 1000);

      // strobe modulator
      if (strobeMode == 1 || btw(strobeMode, 3, 10)) 
      { 
        this->strobe
                ->period( strobePeriod )
                ->param(0, STROB_ON_MS);

        // OFF
        if ( this->strobe->value() == 0) {
          this->clear();
          return;
        }
      }

      // strobe blink (3xstrobe -> blind 1+s)
      else if (strobeMode == 11 || btw(strobeMode, 12, 19)) 
      {
        // int count = this->strobe->periodCount() % 3;  // ERROR: periodCount is moving.. -> count is not linear !
        // // LOG(count);

        // if (count == 0)       this->strobe->period( strobePeriod*100/225 );
        // else if (count == 1)  this->strobe->period( strobePeriod/4 );
        // else if (count == 2)  this->strobe->period( strobePeriod*116/100 + 1000 );
        
        // // OFF
        // if (this->strobe->value() == 0) {
        //   this->clear();
        //   return;
        // }

        // TODO: make a special "multi-pulse" modulator
      }


      // SMOOTH
      /////////////////////////////////////////////////////////////////////////

      // smooth modulator
      if (strobeMode == 2) 
      {
        this->smooth->period( strobePeriod * 10 );

        for(int i=0; i<segmentSize; i++) segment[i] %= this->smooth->value();
      }

      
      // RANDOM
      /////////////////////////////////////////////////////////////////////////

      // random w/ threshold
      if (btw(strobeMode, 3, 10) || btw(strobeMode, 12, 19) || btw(strobeMode, 20, 25)) 
      {
        // Seuil calculation depends of mode
        int strobeSeuil = 1000; 
        if (btw(strobeMode, 3, 10))       strobeSeuil = (data[8] - 31)*1000/69;         // 0->1000    strobeMode >= 3 && strobeMode <= 10
        else if (btw(strobeMode, 12, 19)) strobeSeuil = (data[8] - 121)*1000/79;        // 0->1000    strobeMode >= 12 && strobeMode <= 19
        else if (btw(strobeMode, 20, 25)) strobeSeuil = (data[8] - 201)*1000/54;        // 0->1000    strobeMode >= 20

        // apply random Seuil
        for(int i=0; i<segmentSize; i++) 
          if (random(1000) > strobeSeuil) segment[i] = {CRGBW::Black};
      }


      // MASTER
      /////////////////////////////////////////////////////////////////////////

      this->master(data[0]);



      // DRAW ON STRIP WITH ZOOM & MIRROR
      /////////////////////////////////////////////////////////////////////////

      // Clear
      this->clear();

      // Mirroring alternate (1 = copy, 2 = alternate)
      int mirrorAlternate = 1 + btw(mirrorMode, 1, 3); 

      // Zoom offset
      int zoomOffset = (size() - zoomedSize)/2;      

      // Copy pixels into strip
      for(int i=0; i<zoomedSize; i++) 
      {
        int pix  = i % segmentSize;               // pix cursor into segment
        int iter = i / segmentSize;               // count of mirror copy 
        
        if (iter && iter % mirrorAlternate)       // alternate: invert pix cursor
          pix = segmentSize - pix - 1;    

        this->pixel(i+zoomOffset, segment[pix]);  // draw on strip
      }      

    }

};


#endif
