/*
  K32_leds_anims.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_leds_anims_h
#define K32_leds_anims_h

#include "K32_leds.h"


class K32_leds_anims {
  public:

    static bool test( K32_leds* leds ) {
      int wait = 200;

      delay(2000);
      LOG("LEDS test");

      leds->blackout();

      leds->setPixel(-1, 0, 150, 0, 0);
      leds->setPixel(-1, 1, 150, 0, 0);
      leds->setPixel(-1, 2, 150, 0, 0);
      leds->show();
      delay(wait);

      leds->setPixel(-1, 0, 0, 150, 0);
      leds->setPixel(-1, 1, 0, 150, 0);
      leds->setPixel(-1, 2, 0, 150, 0);
      leds->show();
      delay(wait);

      leds->setPixel(-1, 0, 0, 0, 150);
      leds->setPixel(-1, 1, 0, 0, 150);
      leds->setPixel(-1, 2, 0, 0, 150);
      leds->show();
      delay(wait);

      leds->setPixel(-1, 0, 0, 0, 0, 150);
      leds->setPixel(-1, 1, 0, 0, 0, 150);
      leds->setPixel(-1, 2, 0, 0, 0, 150);
      leds->show();
      delay(wait);

      leds->blackout();

      return false;

    }

    static bool sinus( K32_leds* leds ) {

      int max = 255;
      int period = 2000;
      int white = 0;
      long start = millis();
      long progress = millis() - start;

      // LOG("LEDS sinus");

      while (progress <= period) {

        white = (0.5f + 0.5f * sin( 2 * PI * progress / period - 0.5f * PI ) ) * max;
        leds->setAll(white, white, white, white);
        leds->show();

        delay(1000/LEDS_FPS);
        progress = millis() - start;
      }

      leds->blackout();
      return true;
    }

};



#endif
