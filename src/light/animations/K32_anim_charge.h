/*
  K32_anim_charge.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_anim_charge_h
#define K32_anim_charge_h

//
// NOTE: to be available, add #include to this file in K32_light.h !
//


//
// DISCHARGE : Anim for battery gauge display
//
class K32_anim_discharge : public K32_anim {
  public:
    K32_anim_discharge() : K32_anim() {
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

    void frame (){

      int length = data[13] ;
      CRGBW color1 {data[3],data[4],data[5],data[6]};
      CRGBW color2 {data[7],data[8],data[9],data[10]};

      
      for (int i=0; i<length; i ++)
      {
        /* First color below SOC */
        if(i<((data[11]*length/100)/4))
          this->pixel(i,color1);
  
        else if ((i>=length/2-(data[11]*length/100)/4)&&(i<length/2 + (data[11]*length/100)/4))
          this->pixel(i,color1);
         
        else if (i>=length-(data[11]*length/100)/4)
          this->pixel(i,color1);
        
        else /* Second color */
          this->pixel(i,color2);
        
      }

      /* Fleche Mode */
      this->pixel( (data[11]*length/100)/4, color1);
      this->pixel( length - (data[11]*length/100)/4 - 1, color1);

      this->show();
      

      vTaskDelay(pdMS_TO_TICKS(max(400 - data[12]*2,50) ));

      /* Blinking */
      for (int i=0; i<=data[12]/100+3; i ++)
      {
        

        /* Normal mode */
        // this->pixel( (data[11]*length/100)/4-1 - i , color2);
        // this->pixel( length - (data[11]*length/100)/4+i, color2);

        /* Fleche mode */
        this->pixel( (data[11]*length/100)/4 - i , color2);
        this->pixel( length - (data[11]*length/100)/4-1+i, color2);
        this->pixel( length/2-(data[11]*length/100)/4 + i, color2);
        this->pixel( length/2 + (data[11]*length/100)/4-1-i, color2);

        this->show();

        vTaskDelay(pdMS_TO_TICKS(max(400 - data[12]*2,50) ));
      }

      /* Leds OFFs */
      if (this->data[0] > 0) {
        if (millis() > (this->startTime+this->data[0]*1000)) {
          this->clear();
          this->loop(false);
        }
      }

    };
};

//
// CHARGE : Anim for battery gauge display
//
class K32_anim_charge : public K32_anim {
  public:
    K32_anim_charge() : K32_anim() {
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

    void frame (){

      int length = data[13] ;

      CRGBW color1 {data[3],data[4],data[5],data[6]};
      CRGBW color2 {data[7],data[8],data[9],data[10]};

      
      for (int i=0; i<length; i ++)
      {
        /* First color below SOC */
        if(i<(data[11]*length/100)/4)
          this->pixel(i,color1);
        
        else if ((i>=length/2-(data[11]*length/100)/4)&&(i<length/2 + (data[11]*length/100)/4))
          this->pixel(i,color1);
        
        else if (i>=length-(data[11]*length/100)/4)
          this->pixel(i,color1);
        
        else /* Second color */
          this->pixel(i,color2);
        
      }

      /* Fleche mode */
      this->pixel( (data[11]*length/100)/4 -1, color2);
      this->pixel( length - (data[11]*length/100)/4, color2);

      this->show();
      

      vTaskDelay(pdMS_TO_TICKS(max(800 - data[12]*2,50) ));

      /* Blinking */
      for (int i=0; i<=data[12]/100+1; i ++)
      {
        /* Normal mode */
        // this->pixel( (data[11]*length/100)/4 + i, color1);
        // this->pixel( length - (data[11]*length/100)/4 -1 - i, color1);

        /* Fleche mode */
        this->pixel( (data[11]*length/100)/4 -1 + i, color1);
        this->pixel( length - (data[11]*length/100)/4  - i, color1);
        this->pixel( length/2-(data[11]*length/100)/4 - 1 - i, color1);
        this->pixel( length/2 + (data[11]*length/100)/4 + i, color1);

        this->show();
        

        vTaskDelay(pdMS_TO_TICKS(max(800 - data[12]*2,50)));
      }

      /* Leds OFFs */
      if (this->data[0] > 0) {
        if (millis() > (this->startTime+this->data[0]*1000)) {
          this->clear();
          this->loop(false);
        }
      }

    };
};


#endif
