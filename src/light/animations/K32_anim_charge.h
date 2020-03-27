/*
  K32_anim_charge.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_anim_charge_h
#define K32_anim_charge_h

#include "K32_anim.h"


//
// NOTE: to be able to load an animation by name, it must be registered in K32_animbook.h
//


//
// DISCHARGE : Anim for battery gauge display
//
class K32_anim_discharge : public K32_anim {
  public:
    K32_anim_discharge() : K32_anim("discharge") {
      this->data[0] = 0;    // duration (seconds) 0 = infinite
      this->data[1] = 100;  // intensity max
      this->data[2] = 0;    // intensity min
      // Color 1 is for charged part of battery 
      this->data[3] = 0;    // red1
      this->data[4] = 100;  // green1
      this->data[5] = 0;    // blue1
      this->data[6] = 0;    // white1
      // Color 2 is for discharged part of battery 
      this->data[7] = 100;  // red2
      this->data[8] = 75;   // green2
      this->data[9] = 0;    // blue2
      this->data[10] = 0;   // white2
      this->data[11] = 50;  // State of charge
      this->data[12] = 0;   // Power consumption (0 - 300)
      this->data[13] = LEDSTRIP_MAXPIXEL;  // Length of Led strip
    }

    bool loop ( K32_ledstrip* strip ){

      int length = data[13] ;
      pixelColor_t color1 = pixelFromRGBW(data[3],data[4],data[5],data[6]);
      pixelColor_t color2 = pixelFromRGBW(data[7],data[8],data[9],data[10]);

      
      for (int i=0; i<length; i ++)
      {
        /* First color below SOC */
        if(i<((data[11]*length/100)/4))
          strip->pix(i,color1);
  
        else if ((i>=length/2-(data[11]*length/100)/4)&&(i<length/2 + (data[11]*length/100)/4))
          strip->pix(i,color1);
         
        else if (i>=length-(data[11]*length/100)/4)
          strip->pix(i,color1);
        
        else /* Second color */
          strip->pix(i,color2);
        
      }

      /* Fleche Mode */
      strip->pix( (data[11]*length/100)/4, color1);
      strip->pix( length - (data[11]*length/100)/4 - 1, color1);

      strip->show();
      

      vTaskDelay(pdMS_TO_TICKS(max(400 - data[12]*2,50) ));

      /* Blinking */
      for (int i=0; i<=data[12]/100+3; i ++)
      {
        

        /* Normal mode */
        // strip->pix( (data[11]*length/100)/4-1 - i , color2);
        // strip->pix( length - (data[11]*length/100)/4+i, color2);

        /* Fleche mode */
        strip->pix( (data[11]*length/100)/4 - i , color2);
        strip->pix( length - (data[11]*length/100)/4-1+i, color2);
        strip->pix( length/2-(data[11]*length/100)/4 + i, color2);
        strip->pix( length/2 + (data[11]*length/100)/4-1-i, color2);

        strip->show();
        

        vTaskDelay(pdMS_TO_TICKS(max(400 - data[12]*2,50) ));
      }

      /* Leds OFFs */
      if (this->data[0] > 0) {
        if (millis() > (this->startTime+this->data[0]*1000)) {
          strip->black();
          return false;
        }
      }

      return true;    //  loop

    };
};

//
// CHARGE : Anim for battery gauge display
//
class K32_anim_charge : public K32_anim {
  public:
    K32_anim_charge() : K32_anim("charge") {
      this->data[0] = 0;    // Animation Timeout in sec
      this->data[1] = 100;  // intensity max
      this->data[2] = 0;    // intensity min
      /* Color 1 is for charged part of battery */
      this->data[3] = 0;    // red1
      this->data[4] = 100;  // green1
      this->data[5] = 0;    // blue1
      this->data[6] = 0;    // white1
      /* Color 2 is for discharged part of battery */
      this->data[7] = 165;  // red2
      this->data[8] = 110;  // green2
      this->data[9] = 0;    // blue2
      this->data[10] = 0;   // white2
      this->data[11] = 50;  // State of charge
      this->data[12] = 0;   // Power value (0 - 300)
      this->data[13] = LEDSTRIP_MAXPIXEL;  // Length of Led strip
    }

    bool loop ( K32_ledstrip* strip ){

      int length = data[13] ;

      pixelColor_t color1 = pixelFromRGBW(data[3],data[4],data[5],data[6]);
      pixelColor_t color2 = pixelFromRGBW(data[7],data[8],data[9],data[10]);

      
      for (int i=0; i<length; i ++)
      {
        /* First color below SOC */
        if(i<(data[11]*length/100)/4)
          strip->pix(i,color1);
        
        else if ((i>=length/2-(data[11]*length/100)/4)&&(i<length/2 + (data[11]*length/100)/4))
          strip->pix(i,color1);
        
        else if (i>=length-(data[11]*length/100)/4)
          strip->pix(i,color1);
        
        else /* Second color */
          strip->pix(i,color2);
        
      }

      /* Fleche mode */
      strip->pix( (data[11]*length/100)/4 -1, color2);
      strip->pix( length - (data[11]*length/100)/4, color2);

      strip->show();
      

      vTaskDelay(pdMS_TO_TICKS(max(800 - data[12]*2,50) ));

      /* Blinking */
      for (int i=0; i<=data[12]/100+1; i ++)
      {
        /* Normal mode */
        // strip->pix( (data[11]*length/100)/4 + i, color1);
        // strip->pix( length - (data[11]*length/100)/4 -1 - i, color1);

        /* Fleche mode */
        strip->pix( (data[11]*length/100)/4 -1 + i, color1);
        strip->pix( length - (data[11]*length/100)/4  - i, color1);
        strip->pix( length/2-(data[11]*length/100)/4 - 1 - i, color1);
        strip->pix( length/2 + (data[11]*length/100)/4 + i, color1);

        strip->show();
        

        vTaskDelay(pdMS_TO_TICKS(max(800 - data[12]*2,50)));
      }

      /* Leds OFFs */
      if (this->data[0] > 0) {
        if (millis() > (this->startTime+this->data[0]*1000)) {
          strip->black();
          return false;
        }
      }

      return true;    //  loop

    };
};


#endif
