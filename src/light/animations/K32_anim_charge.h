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
    K32_anim_discharge() : K32_anim() 
    {
      this->setdata(0, 0);    // duration (seconds) 0 = infinite
      this->setdata(1, 100);  // intensity max
      this->setdata(2, 0);    // intensity min

      // Color 1 is for charged part of battery 
      this->setdata(3, 0);    // red1
      this->setdata(4, 100);  // green1
      this->setdata(5, 0);    // blue1
      this->setdata(6, 0);    // white1

      // Color 2 is for discharged part of battery 
      this->setdata(7, 100);  // red2
      this->setdata(8, 75);   // green2
      this->setdata(9, 0);    // blue2
      this->setdata(10, 0);   // white2

      this->setdata(11, 50);  // State of charge
      this->setdata(12, 0);   // Power consumption (0 - 300)

    }

    void frame (int data[LEDS_DATA_SLOTS])
    {

      int length = this->size();
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

        vTaskDelay(pdMS_TO_TICKS(max(400 - data[12]*2,50) ));
      }

      /* Leds OFFs */
      if (data[0] > 0) {
        if (millis() > (this->startTime+data[0]*1000)) {
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
      this->setdata(0, 0);    // Animation Timeout in sec
      this->setdata(1, 100);  // intensity max
      this->setdata(2, 0);    // intensity min

      /* Color 1 is for charged part of battery */
      this->setdata(3, 0);    // red1
      this->setdata(4, 100);  // green1
      this->setdata(5, 0);    // blue1
      this->setdata(6, 0);    // white1

      /* Color 2 is for discharged part of battery */
      this->setdata(7, 165);  // red2
      this->setdata(8, 110);  // green2
      this->setdata(9, 0);    // blue2
      this->setdata(10, 0);   // white2

      this->setdata(11, 50);  // State of charge
      this->setdata(12, 0);   // Power value (0 - 300)
    }

    void frame (int data[LEDS_DATA_SLOTS])
    {

      int length = this->size();

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

        vTaskDelay(pdMS_TO_TICKS(max(800 - data[12]*2,50)));
      }

      /* Leds OFFs */
      if (data[0] > 0) {
        if (millis() > (this->startTime+data[0]*1000)) {
          this->clear();
          this->loop(false);
        }
      }

    };
};


#endif
