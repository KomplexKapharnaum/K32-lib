/*
  K32_leds_anims.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_leds_anims_h
#define K32_leds_anims_h

#define LEDS_ANIMS_SLOTS  16
#define LEDS_PARAM_SLOTS  16

#include "K32_leds_rmt.h"

//
// NOTE: WHEN ADDING A NEW ANIM, REGISTER IT IN THE ANIMATOR DOWN THERE !
//

class K32_leds_anim {
  public:
    K32_leds_anim() {
      this->critical = xSemaphoreCreateMutex();
    }

    virtual String name () { return ""; };
    virtual bool loop ( K32_leds_rmt* leds ) { return false; };

    void init() { this->startTime = millis(); }
    void setParam(int k, int value) { if (k < LEDS_PARAM_SLOTS) this->params[k] = value; }
    void lock() { xSemaphoreTake(this->critical, portMAX_DELAY); }
    void unlock() { xSemaphoreGive(this->critical); }

    int params[LEDS_PARAM_SLOTS];
    SemaphoreHandle_t critical;

  protected:
    unsigned long startTime = 0;
};


//
// TEST
//
class K32_leds_anim_test : public K32_leds_anim {
  public:
    K32_leds_anim_test() {
      this->params[0] = 150;    // intensity
      this->params[1] = 2000;   // initial delay
      this->params[2] = 200;    // step duration
    }

    String name () { return "test"; }

    void init() {}

    bool loop ( K32_leds_rmt* leds ){

      int wait = this->params[2];

      delay(this->params[1]);
      LOG("LEDS: test");

      this->lock();
      leds->blackout();

      leds->setPixel(-1, 0, this->params[0], 0, 0);
      leds->setPixel(-1, 1, this->params[0], 0, 0);
      leds->setPixel(-1, 2, this->params[0], 0, 0);
      leds->show();
      this->unlock();

      vTaskDelay(pdMS_TO_TICKS(wait));
      this->lock();
      leds->setPixel(-1, 0, 0, this->params[0], 0);
      leds->setPixel(-1, 1, 0, this->params[0], 0);
      leds->setPixel(-1, 2, 0, this->params[0], 0);
      leds->show();
      this->unlock();

      vTaskDelay(pdMS_TO_TICKS(wait));

      this->lock();
      leds->setPixel(-1, 0, 0, 0, this->params[0]);
      leds->setPixel(-1, 1, 0, 0, this->params[0]);
      leds->setPixel(-1, 2, 0, 0, this->params[0]);
      leds->show();
      this->unlock();

      vTaskDelay(pdMS_TO_TICKS(wait));

      this->lock();
      leds->setPixel(-1, 0, 0, 0, 0, this->params[0]);
      leds->setPixel(-1, 1, 0, 0, 0, this->params[0]);
      leds->setPixel(-1, 2, 0, 0, 0, this->params[0]);
      leds->show();
      this->unlock();

      vTaskDelay(pdMS_TO_TICKS(wait));

      this->lock();
      leds->blackout();
      this->unlock();

      return false;    // DON'T LOOP !

    };
};

//
// SINUS
//
class K32_leds_anim_sinus : public K32_leds_anim {
  public:
    K32_leds_anim_sinus() {
      this->params[0] = 1000; // period
      this->params[1] = 255;  // intensity max
      this->params[2] = 0;    // intensity min
      this->params[3] = 255;  // red
      this->params[4] = 255;  // green
      this->params[5] = 255;  // blue
      this->params[6] = 255;  // white
      this->params[7] = 0;    // duration (seconds) 0 = infinite
    }

    String name () { return "sinus"; }

    bool loop ( K32_leds_rmt* leds ){

      float factor = 0;
      unsigned long start = millis();
      unsigned long progress = 0;

      // LOG("LEDS sinus");

      while (progress <= this->params[0]) {

        factor = ((0.5f + 0.5f * sin( 2 * PI * progress / this->params[0] - 0.5f * PI ) ) * (this->params[1]-this->params[2]) + this->params[2]) / 255;

        this->lock();
        leds->setAll( (int)(this->params[3]*factor),  (int)(this->params[4]*factor),  (int)(this->params[5]*factor),  (int)(this->params[6]*factor));
        leds->show();
        this->unlock();

        vTaskDelay(pdMS_TO_TICKS(1));
        progress = millis() - start;
      }

      // ANIMATION Timeout
      if (this->params[7] > 0)
        if (millis() > (this->startTime+this->params[7]*1000)) {
          this->lock();
          leds->blackout();
          this->unlock();
          return false;
        }

      return true;    // LOOP !

    };
};

//
// CHASER
//
class K32_leds_anim_chaser : public K32_leds_anim {
  public:
    K32_leds_anim_chaser() {
      this->params[0] = 10; // step duration
      this->params[1] = 255;  // intensity max
      this->params[2] = 0;    // intensity min
      this->params[3] = 255;  // red
      this->params[4] = 255;  // green
      this->params[5] = 255;  // blue
      this->params[6] = 255;  // white
      this->params[7] = 10; // length of chaser
      this->params[8] = LEDS_NUM_PIXEL; // length of led strip
      this->params[9] = 0;    // duration (seconds) 0 = infinite
    }

    String name () { return "chaser"; }

    bool loop ( K32_leds_rmt* leds ){

      unsigned int progress = 0 ;


      for ( progress = 0; progress < this-> params[8] + this->params[7]; progress ++)
      {
          this->lock();
          for (int i = 0 ; i <= progress ; i ++)
          {
            if (i > progress - this->params[7])
            {
              leds->setPixel(0,i,params[3],params[4],params[5],params[6]);
              leds->setPixel(1,i,params[3],params[4],params[5],params[6]);
            } else
            {
              leds->setPixel( 0,i, 0, 0,0,0);
              leds->setPixel( 1,i, 0,0,0,0);
            }
          }
          leds->show();
          this->unlock();
        vTaskDelay(pdMS_TO_TICKS(this->params[0]));
      }
      leds->blackout();



      // ANIMATION Timeout
      if (this->params[9] > 0)
        if (millis() > (this->startTime+this->params[7]*1000)) {
          this->lock();
          leds->blackout();
          this->unlock();
          return false;
        }


      return true;    // LOOP !

    };
};

//
// DECHARGE
//
class K32_leds_anim_decharge : public K32_leds_anim {
  public:
    K32_leds_anim_decharge() {
      this->params[0] = 2; // Animation timeout in secs
      this->params[1] = 100;  // intensity max
      this->params[2] = 0;    // intensity min
      this->params[3] = 0;  // red1
      this->params[4] = 100;  // green1
      this->params[5] = 0;  // blue1
      this->params[6] = 0;  // white1
      this->params[7] = 100;  // red2
      this->params[8] = 75;  // green2
      this->params[9] = 0;  // blue2
      this->params[10] = 0;  // white2
      this->params[11] = 50;    // State of charge
    }

    String name () { return "decharge"; }

    bool loop ( K32_leds_rmt* leds ){

      unsigned long start = millis();
      unsigned long progress = 0;
      int length = LEDS_NUM_PIXEL ;


        this->lock();
        for (int i=0; i<100; i ++)
        {
          /* First color below SOC */
          if(i<params[11]/4)
          {
            leds->setPixel(0,i,params[3],params[4],params[5],params[6]);
            leds->setPixel(1,i,params[3],params[4],params[5],params[6]);
          } else if ((i>=50-params[11]/4)&&(i<50 + params[11]/4))
          {
            leds->setPixel(0,i,params[3],params[4],params[5],params[6]);
            leds->setPixel(1,i,params[3],params[4],params[5],params[6]);
          } else if (i>=100-params[11]/4)
          {
            leds->setPixel(0,i,params[3],params[4],params[5],params[6]);
            leds->setPixel(1,i,params[3],params[4],params[5],params[6]);
          } else /* Second color */
          {
            leds->setPixel( 0,i, params[7],params[8],params[9],params[10]);
            leds->setPixel( 1,i,params[7],params[8],params[9],params[10]);
          }
        }
        leds->show();
        this->unlock();
        vTaskDelay(pdMS_TO_TICKS(500));

        this->lock();

        /* Blinking */
          leds->setPixel( 0,params[11]/4-1,params[7],params[8],params[9],params[10]);
          leds->setPixel( 1,params[11]/4-1,params[7],params[8],params[9],params[10]);

          leds->setPixel( 0,length/2-params[11]/4 ,params[7],params[8],params[9],params[10]);
          leds->setPixel( 1,length/2-params[11]/4 ,params[7],params[8],params[9],params[10]);

          leds->setPixel( 0,length/2 + params[11]/4-1,params[7],params[8],params[9],params[10]);
          leds->setPixel( 1,length/2 + params[11]/4-1,params[7],params[8],params[9],params[10]);

          leds->setPixel( 0,length - params[11]/4,params[7],params[8],params[9],params[10]);
          leds->setPixel( 1,length - params[11]/4,params[7],params[8],params[9],params[10]);
          leds->show();
          this->unlock();

          vTaskDelay(pdMS_TO_TICKS(500));

          this->lock();
          leds->setPixel( 0,params[11]/4-2,params[7],params[8],params[9],params[10]);
          leds->setPixel( 1,params[11]/4-2,params[7],params[8],params[9],params[10]);

          leds->setPixel( 0,length/2-params[11]/4 + 1,params[7],params[8],params[9],params[10]);
          leds->setPixel( 1,length/2-params[11]/4 + 1,params[7],params[8],params[9],params[10]);

          leds->setPixel( 0,length/2 + params[11]/4-2,params[7],params[8],params[9],params[10]);
          leds->setPixel( 1,length/2 + params[11]/4-2,params[7],params[8],params[9],params[10]);

          leds->setPixel( 0,length - params[11]/4 +1,params[7],params[8],params[9],params[10]);
          leds->setPixel( 1,length - params[11]/4 +1,params[7],params[8],params[9],params[10]);
          leds->show();
          this->unlock();

          vTaskDelay(pdMS_TO_TICKS(500));

          this->lock();
          leds->setPixel( 0,params[11]/4-3,params[7],params[8],params[9],params[10]);
          leds->setPixel( 1,params[11]/4-3,params[7],params[8],params[9],params[10]);

          leds->setPixel( 0,length/2-params[11]/4 + 2,params[7],params[8],params[9],params[10]);
          leds->setPixel( 1,length/2-params[11]/4 + 2,params[7],params[8],params[9],params[10]);

          leds->setPixel( 0,length/2 + params[11]/4-3,params[7],params[8],params[9],params[10]);
          leds->setPixel( 1,length/2 + params[11]/4-3,params[7],params[8],params[9],params[10]);

          leds->setPixel( 0,length - params[11]/4 +2,params[7],params[8],params[9],params[10]);
          leds->setPixel( 1,length - params[11]/4 +2,params[7],params[8],params[9],params[10]);
          leds->show();
          this->unlock();

          vTaskDelay(pdMS_TO_TICKS(500));

          /* Leds OFFs */
          if (this->params[0] > 0)
          {
            if (millis() > (this->startTime+this->params[0]*1000)) {
              this->lock();
              leds->setAll(0,0,0,0);
              leds->show();
              this->unlock();
              return false;
            }
          }

        return true;    //  loop

    };
};


//
// CHARGE
//
class K32_leds_anim_charge : public K32_leds_anim {
  public:
    K32_leds_anim_charge() {
      this->params[0] = 2;  // Animation Timeout in sec
      this->params[1] = 100;  // intensity max
      this->params[2] = 0;    // intensity min
      this->params[3] = 0;  // red1
      this->params[4] = 100;  // green1
      this->params[5] = 0;  // blue1
      this->params[6] = 0;  // white1
      this->params[7] = 165;  // red2
      this->params[8] = 110;  // green2
      this->params[9] = 0;  // blue2
      this->params[10] = 0;  // white2
      this->params[11] = 50;    // State of charge
    }

    String name () { return "charge"; }

    bool loop ( K32_leds_rmt* leds ){

      int length = LEDS_NUM_PIXEL ;


        this->lock();
        for (int i=0; i<length; i ++)
        {
          /* First color below SOC */
          if(i<params[11]/4)
          {
            leds->setPixel(0,i,params[3],params[4],params[5],params[6]);
            leds->setPixel(1,i,params[3],params[4],params[5],params[6]);
          } else if ((i>=length/2-params[11]/4)&&(i<length/2 + params[11]/4))
          {
            leds->setPixel(0,i,params[3],params[4],params[5],params[6]);
            leds->setPixel(1,i,params[3],params[4],params[5],params[6]);
          } else if (i>=length-params[11]/4)
          {
            leds->setPixel(0,i,params[3],params[4],params[5],params[6]);
            leds->setPixel(1,i,params[3],params[4],params[5],params[6]);
          } else /* Second color */
          {
            leds->setPixel( 0,i, params[7],params[8],params[9],params[10]);
            leds->setPixel( 1,i, params[7],params[8],params[9],params[10]);
          }
        }
        leds->show();
        this->unlock();
        vTaskDelay(pdMS_TO_TICKS(500));

        this->lock();
      /* Blinking */
        leds->setPixel( 0,params[11]/4,params[3],params[4],params[5],params[6]);
        leds->setPixel( 1,params[11]/4,params[3],params[4],params[5],params[6]);

        leds->setPixel( 0,length/2-params[11]/4 - 1,params[3],params[4],params[5],params[6]);
        leds->setPixel( 1,length/2-params[11]/4 -1,params[3],params[4],params[5],params[6]);

        leds->setPixel( 0,length/2 + params[11]/4,params[3],params[4],params[5],params[6]);
        leds->setPixel( 1,length/2 + params[11]/4,params[3],params[4],params[5],params[6]);

        leds->setPixel( 0,length - params[11]/4 -1,params[3],params[4],params[5],params[6]);
        leds->setPixel( 1,length - params[11]/4 -1,params[3],params[4],params[5],params[6]);
        leds->show();
        this->unlock();

        vTaskDelay(pdMS_TO_TICKS(500));

        this->lock();
        leds->setPixel( 0,params[11]/4+1,params[3],params[4],params[5],params[6]);
        leds->setPixel( 1,params[11]/4+1,params[3],params[4],params[5],params[6]);

        leds->setPixel( 0,length/2-params[11]/4-2,params[3],params[4],params[5],params[6]);
        leds->setPixel( 1,length/2-params[11]/4-2,params[3],params[4],params[5],params[6]);

        leds->setPixel( 0,length/2 + params[11]/4+1,params[3],params[4],params[5],params[6]);
        leds->setPixel( 1,length/2 + params[11]/4+1,params[3],params[4],params[5],params[6]);

        leds->setPixel( 0,length - params[11]/4-2,params[3],params[4],params[5],params[6]);
        leds->setPixel( 1,length - params[11]/4-2,params[3],params[4],params[5],params[6]);
        leds->show();
        this->unlock();

        vTaskDelay(pdMS_TO_TICKS(500));

        this->lock();
        leds->setPixel( 0,params[11]/4+2,params[3],params[4],params[5],params[6]);
        leds->setPixel( 1,params[11]/4+2,params[3],params[4],params[5],params[6]);

        leds->setPixel( 0,length/2-params[11]/4-3,params[3],params[4],params[5],params[6]);
        leds->setPixel( 1,length/2-params[11]/4-3,params[3],params[4],params[5],params[6]);

        leds->setPixel( 0,length/2 + params[11]/4+2,params[3],params[4],params[5],params[6]);
        leds->setPixel( 1,length/2 + params[11]/4+2,params[3],params[4],params[5],params[6]);

        leds->setPixel( 0,length - params[11]/4-3,params[3],params[4],params[5],params[6]);
        leds->setPixel( 1,length - params[11]/4-3,params[3],params[4],params[5],params[6]);
        leds->show();
        this->unlock();

        vTaskDelay(pdMS_TO_TICKS(500));

        /* Leds OFFs */
        if (this->params[0] > 0)
        {
          if (millis() > (this->startTime+this->params[0]*1000)) {
            this->lock();
            leds->setAll(0,0,0,0);
            leds->show();
            this->unlock();
            return false;
          }
        }

      return true;    //  loop

    };
};

//
// CURRENT VISUALIZATION
//
class K32_leds_anim_current : public K32_leds_anim {
  public:
    K32_leds_anim_current() {
      this->params[0] = 3; // Animation TimeOut in sec
      this->params[1] = 0;  // red1
      this->params[2] = 100;  // green1
      this->params[3] = 0;  // blue1
      this->params[4] = 0;  // white1
      this->params[5] = 165;  // red2
      this->params[6] = 110;  // green2
      this->params[7] = 0;  // blue2
      this->params[8] = 0;  // white2
      this->params[9] = 0;    // Current Value
    }

    String name () { return "current"; }

    bool loop ( K32_leds_rmt* leds ){

      int length = LEDS_NUM_PIXEL ;


        this->lock();
        for (int i=0; i<length; i ++)
        {
            if(this->params[9]>0)
            {
              /* First color below current level */
              if(i<params[9]/4)
              {
                leds->setPixel(0,i,params[1],params[2],params[3],params[4]);
                leds->setPixel(1,i,params[1],params[2],params[3],params[4]);
              } else if ((i>=length/2-params[9]/4)&&(i<length/2 + params[9]/4))
              {
                leds->setPixel(0,i,params[1],params[2],params[3],params[4]);
                leds->setPixel(1,i,params[1],params[2],params[3],params[4]);
              } else if (i>=length-params[9]/4)
              {
                leds->setPixel(0,i,params[1],params[2],params[3],params[4]);
                leds->setPixel(1,i,params[1],params[2],params[3],params[4]);
              } else /* Second color */
              {
                leds->setPixel( 0,i, 0,0,0,0);
                leds->setPixel( 1,i, 0,0,0,0);
              }
            } else
            {
              /* First color below current level */
              if((i>length/4 + params[9]/4)&&(i<length/4 - params[9]/4-1))
              {
                leds->setPixel(0,i,params[5],params[6],params[7],params[8]);
                leds->setPixel(1,i,params[5],params[6],params[7],params[8]);
              } else if ((i>3*length/4 + params[9]/4)&&(i<3*length/4 - params[9]/4-1))
              {
                leds->setPixel(0,i,params[5],params[6],params[7],params[8]);
                leds->setPixel(1,i,params[5],params[6],params[7],params[8]);
              } else /* Second color */
              {
                leds->setPixel( 0,i,0,0,0,0);
                leds->setPixel( 1,i,0,0,0,0);
              }
            }
          }
          leds->show();
          this->unlock();


        /* Leds OFFs */
        this->lock();
        leds->setAll(0,0,0,0);
        leds->show();
        this->unlock();
        //ANIMATION Timeout
        if (this->params[0] > 0)
        {
          if (millis() > (this->startTime+this->params[0]*1000)) {
            this->lock();
            leds->setAll(0,0,0,0);
            leds->show();
            this->unlock();
            return false;
          }
        }

      return true;    //  loop

    };
};


//
// CURRENT VISUALIZATION 2
//
class K32_leds_anim_current2 : public K32_leds_anim {
  public:
    K32_leds_anim_current2() {
      this->params[0] = 3; // Animation TimeOut in sec
      this->params[1] = 0;  // red1
      this->params[2] = 100;  // green1
      this->params[3] = 0;  // blue1
      this->params[4] = 0;  // white1
      this->params[5] = 165;  // red2
      this->params[6] = 110;  // green2
      this->params[7] = 0;  // blue2
      this->params[8] = 0;  // white2
      this->params[9] = 0;    // Current Value
    }

    String name () { return "current2"; }

    bool loop ( K32_leds_rmt* leds ){

      int length = LEDS_NUM_PIXEL ;

        this->lock();
        for (int i=0; i<length; i ++)
        {
            if(this->params[9]>0)
            {
              if ((i < (length + params[9])/8) && (i> length / 8))
              {
                leds->setPixel(0,i,params[1],params[2],params[3],params[4]);
                leds->setPixel(1,i,params[1],params[2],params[3],params[4]);
              } else if( (i< 3*length / 8)&&(i>(3*length-params[9])/8 ))
              {
                leds->setPixel(0,i,params[1],params[2],params[3],params[4]);
                leds->setPixel(1,i,params[1],params[2],params[3],params[4]);
              } else if( (i < (5*length + params[9])/8) && (i> 5*length / 8))
              {
                leds->setPixel(0,i,params[1],params[2],params[3],params[4]);
                leds->setPixel(1,i,params[1],params[2],params[3],params[4]);
              } else if( (i< 7*length / 8)&&(i>(7*length-params[9])/8 ))
              {
                leds->setPixel(0,i,params[1],params[2],params[3],params[4]);
                leds->setPixel(1,i,params[1],params[2],params[3],params[4]);
              } else if ((i==length / 8)||(i==3*length/8) || (i== 5*length / 8) || (i == 7 * length / 8)) /* Zero Point  */
              {
                leds->setPixel(0,i,50,50,50,0);
                leds->setPixel(1,i,50,50,50,0);
              } else /* Second color */
              {
                leds->setPixel( 0,i, 0,0,0,0);
                leds->setPixel( 1,i, 0,0,0,0);
              }
            } else if (this->params[9]<0)
            {
              if ((i > (length + params[9])/8) && (i< length / 8))
              {
                leds->setPixel(0,i,params[5],params[6],params[7],params[8]);
                leds->setPixel(1,i,params[5],params[6],params[7],params[8]);
              } else if( (i> 3*length / 8)&&(i<(3*length-params[9])/8 ))
              {
                leds->setPixel(0,i,params[5],params[6],params[7],params[8]);
                leds->setPixel(1,i,params[5],params[6],params[7],params[8]);
              } else if( (i > (5*length + params[9])/8) && (i < 5*length / 8))
              {
                leds->setPixel(0,i,params[5],params[6],params[7],params[8]);
                leds->setPixel(1,i,params[5],params[6],params[7],params[8]);
              } else if( (i> 7*length / 8)&&(i<(7*length-params[9])/8 ))
              {
                leds->setPixel(0,i,params[5],params[6],params[7],params[8]);
                leds->setPixel(1,i,params[5],params[6],params[7],params[8]);
              } else if ((i==length / 8)||(i==3*length/8) || (i== 5*length / 8) || (i == 7 * length / 8)) /* Zero Point  */
              {
                leds->setPixel(0,i,50,50,50,0);
                leds->setPixel(1,i,50,50,50,0);
              } else /* Second color */
              {
                leds->setPixel( 0,i, 0,0,0,0);
                leds->setPixel( 1,i, 0,0,0,0);
              }

            }
          }
          leds->show();
          this->unlock();

        if (this->params[0] > 0)
        {
          if (millis() > (this->startTime+this->params[0]*1000)) {
            this->lock();
            leds->setAll(0,0,0,0);
            leds->show();
            this->unlock();
            return false;
          }
        }

      return true;    //  loop

    };
};


//
// Double gauge visualization
//
class K32_leds_anim_double : public K32_leds_anim {
  public:
    K32_leds_anim_double() {
      this->params[0] = 3; // Animation TimeOut in sec
      this->params[1] = 0;  // red1
      this->params[2] = 100;  // green1
      this->params[3] = 0;  // blue1
      this->params[4] = 0;  // white1
      this->params[5] = 165;  // red2
      this->params[6] = 110;  // green2
      this->params[7] = 0;  // blue2
      this->params[8] = 0;  // white2
      this->params[9] = 0;    // Current Value
      this->params[10] = 50; // SOC value
    }

    String name () { return "double"; }

    bool loop ( K32_leds_rmt* leds ){

      int length = LEDS_NUM_PIXEL ;

      this->lock();
      for (int i=0; i<length/2; i ++)
      {
        /* First color below SOC */
        if(i<params[10]/4)
        {
          leds->setPixel(0,i,params[1],params[2],params[3],params[4]);
          leds->setPixel(1,i,params[1],params[2],params[3],params[4]);
        } else if ((i>=50-params[10]/4)&&(i<50 + params[10]/4))
        {
          leds->setPixel(0,i,params[1],params[2],params[3],params[4]);
          leds->setPixel(1,i,params[1],params[2],params[3],params[4]);
        } else if (i>=100-params[10]/4)
        {
          leds->setPixel(0,i,params[1],params[2],params[3],params[4]);
          leds->setPixel(1,i,params[1],params[2],params[3],params[4]);
        } else /* Second color */
        {
          leds->setPixel( 0,i, params[5],params[6],params[7],params[8]);
          leds->setPixel( 1,i,params[5],params[6],params[7],params[8]);
        }
      }
      leds->show();
      this->unlock();

        this->lock();
        for (int i=length/2; i<length; i ++)
        {
            if(this->params[9]>0)
            {
              if ((i < (length + params[9])/8) && (i> length / 8))
              {
                leds->setPixel(0,i,params[1],params[2],params[3],params[4]);
                leds->setPixel(1,i,params[1],params[2],params[3],params[4]);
              } else if( (i< 3*length / 8)&&(i>(3*length-params[9])/8 ))
              {
                leds->setPixel(0,i,params[1],params[2],params[3],params[4]);
                leds->setPixel(1,i,params[1],params[2],params[3],params[4]);
              } else if( (i < (5*length + params[9])/8) && (i> 5*length / 8))
              {
                leds->setPixel(0,i,params[1],params[2],params[3],params[4]);
                leds->setPixel(1,i,params[1],params[2],params[3],params[4]);
              } else if( (i< 7*length / 8)&&(i>(7*length-params[9])/8 ))
              {
                leds->setPixel(0,i,params[1],params[2],params[3],params[4]);
                leds->setPixel(1,i,params[1],params[2],params[3],params[4]);
              } else if ((i==length / 8)||(i==3*length/8) || (i== 5*length / 8) || (i == 7 * length / 8)) /* Zero Point  */
              {
                leds->setPixel(0,i,50,50,50,0);
                leds->setPixel(1,i,50,50,50,0);
              } else /* Second color */
              {
                leds->setPixel( 0,i, 0,0,0,0);
                leds->setPixel( 1,i, 0,0,0,0);
              }
            } else if (this->params[9]<0)
            {
              if ((i > (length + params[9])/8) && (i< length / 8))
              {
                leds->setPixel(0,i,params[5],params[6],params[7],params[8]);
                leds->setPixel(1,i,params[5],params[6],params[7],params[8]);
              } else if( (i> 3*length / 8)&&(i<(3*length-params[9])/8 ))
              {
                leds->setPixel(0,i,params[5],params[6],params[7],params[8]);
                leds->setPixel(1,i,params[5],params[6],params[7],params[8]);
              } else if( (i > (5*length + params[9])/8) && (i < 5*length / 8))
              {
                leds->setPixel(0,i,params[5],params[6],params[7],params[8]);
                leds->setPixel(1,i,params[5],params[6],params[7],params[8]);
              } else if( (i> 7*length / 8)&&(i<(7*length-params[9])/8 ))
              {
                leds->setPixel(0,i,params[5],params[6],params[7],params[8]);
                leds->setPixel(1,i,params[5],params[6],params[7],params[8]);
              } else if ((i==length / 8)||(i==3*length/8) || (i== 5*length / 8) || (i == 7 * length / 8)) /* Zero Point  */
              {
                leds->setPixel(0,i,50,50,50,0);
                leds->setPixel(1,i,50,50,50,0);
              } else /* Second color */
              {
                leds->setPixel( 0,i, 0,0,0,0);
                leds->setPixel( 1,i, 0,0,0,0);
              }

            }
          }
          leds->show();
          this->unlock();

        if (this->params[0] > 0)
        {
          if (millis() > (this->startTime+this->params[0]*1000)) {
            this->lock();
            leds->setAll(0,0,0,0);
            leds->show();
            this->unlock();
            return false;
          }
        }

      return true;    //  loop

    };
};


//
// STROBE
//
class K32_leds_anim_strobe : public K32_leds_anim {
  public:
    K32_leds_anim_strobe() {
      this->params[0] = 2000; // period
      this->params[1] = 50;   // % ON
    }

    String name () { return "strobe"; }

    bool loop ( K32_leds_rmt* leds ){

      int intensity = 255;
      TickType_t xOn = max(1, (int)pdMS_TO_TICKS( (this->params[0]*this->params[1]/100) ));
      TickType_t xOff = max(1, (int)pdMS_TO_TICKS( this->params[0] - (this->params[0]*this->params[1]/100) ));

      this->lock();
      leds->setAll(intensity, intensity, intensity, intensity)->show();
      this->unlock();

      vTaskDelay( xOn );

      this->lock();
      leds->blackout();
      this->unlock();

      vTaskDelay( xOff );

      return true;    // LOOP !

    };
};


//
// HARDSTROBE
//
class K32_leds_anim_hardstrobe : public K32_leds_anim {
  public:
    K32_leds_anim_hardstrobe() {
      this->params[0] = 2000; // period
      this->params[1] = 50;   // % ON

      this->nextEvent = 0;
      state = false;
    }

    String name () { return "hardstrobe"; }

    bool loop ( K32_leds_rmt* leds ){

      int intensity = 255;
      // LOGINL(millis()); LOGINL(" // "); LOG(this->nextEvent);
      if (millis() > this->nextEvent) {
        // OFF -> ON
        if(!state) {
          this->nextEvent = millis() + (this->params[0]*this->params[1]/100) ;

          this->lock();
          leds->setAll(intensity, intensity, intensity, intensity)->show();
          this->unlock();
        }
        // ON -> OFF
        else {
          this->nextEvent = millis() + this->params[0] - (this->params[0]*this->params[1]/100);

          this->lock();
          leds->blackout();
          this->unlock();
        }
        state = !state;
      }

      return true;    // LOOP !

    };

    long nextEvent;
    bool state;
};


//
// ANIMATOR BOOK
//
class K32_leds_animbook {
  public:
    K32_leds_animbook() {

      //
      // REGISTER AVAILABLE ANIMS !
      //
      this->add( new K32_leds_anim_test() );
      this->add( new K32_leds_anim_sinus() );
      this->add( new K32_leds_anim_chaser() );
      this->add(new K32_leds_anim_charge());
      this->add(new K32_leds_anim_decharge());
      this->add(new K32_leds_anim_current());
      this->add(new K32_leds_anim_current2());
      this->add(new K32_leds_anim_double());
      this->add( new K32_leds_anim_strobe() );
      this->add( new K32_leds_anim_hardstrobe() );

    }

    K32_leds_anim* get( String name ) {
      for (int k=0; k<this->counter; k++)
        if (this->anims[k]->name() == name) {
          LOGINL("LEDS: "); LOG(name);
          return this->anims[k];
        }
      LOGINL("ANIM: not found "); LOG(name);
      return new K32_leds_anim();
    }


  private:
    K32_leds_anim* anims[LEDS_ANIMS_SLOTS];
    int counter = 0;

    void add(K32_leds_anim* anim) {
      if (this->counter >= LEDS_ANIMS_SLOTS) {
        LOG("ERROR: no more slot available to register new animation");
        return;
      }
      this->anims[ this->counter ] = anim;
      this->counter++;
    };

};


#endif
