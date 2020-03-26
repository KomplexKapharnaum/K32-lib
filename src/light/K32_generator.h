/*
  K32_generator.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_generator_h
#define K32_generator_h

#include "K32_ledstrip.h"



// Generator
class K32_generator {
  public:
    K32_generator(int size) : size(size) {};  


    int master      = 255;
    pixelColor_t colors[4];

    int color_mode  = 0;  

    int pix_mode        = 0;
    int pix_position    = 0;
    int pix_length      = 0;

    int size;

    pixelColor_t* image() {
      
      pixelColor_t img[size];   // create buffer

      int pix_start;
      int pix_pos;
      int pix_end;

      int rap_tri;

      // COLOR NORM
      if (color_mode == 0) {
        
        // PIX
        if (pix_mode == 2) {
          pix_start = pix_length - 1;
          pix_end = pix_start + pix_start;
          pix_pos = ((((3*pix_start + N_L_P_S) * pix_position) / 255) - (2*pix_start + 1));
        }
        else if (pix_mode == 23 || pix_mode == 24) {
          pix_start = (((pix_length * N_L_P_S) / 255) - 1);
          pix_end = pix_start + pix_start;
          rap_tri = map(pix_length, 0, 255, 0, NUM_LEDS_PER_STRIP * 2);
          pix_pos = ((NUM_LEDS_PER_STRIP / 2 - rap_tri / 2) + map(pix_position, 0, 255, -NUM_LEDS_PER_STRIP + 1, NUM_LEDS_PER_STRIP + 1));
          // pix_pos = map(pix_pos_v, 0, 255, -NUM_LEDS_PER_STRIP + 1, NUM_LEDS_PER_STRIP + 1);
        }
        else {

        }

      }





    };             
};




#endif
