/*
  K32_generator.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_generator_h
#define K32_generator_h

#include "K32_ledstrip.h"

enum color_mode { 
  COLOR_NORM, 
  COLOR_PICKER, 
  COLOR_SD 
}


// Generator
class K32_generator {
  public:
    K32_generator(int size) : fullSize(size) {};  

    int fullSize;               // total size: NUM_LEDS_PER_STRIP_max

    int master      = 255;      // master
    pixelColor_t colors[4];     // colors[0]..[3]

    color_mode color_mode  = 0; // color_mod: data[adr + 13] range10

    int pix_mode        = 0;    // pix_mod:   data[adr + 4]  range10
    int pix_length      = 0;    // pix_long:  data[adr + 5]
    int pix_position    = 0;    // pix_pos:   data[adr + 6]

    int zoom            = 255;  // zoom:      data[adr + 15]
    int mirror          = 0;    // mirror:    data[adr + 14] range10


    // MAKE IMAGE
    //
    pixelColor_t* image() {

      //
      // BUFFER
      //
      CRGBW buffer[fullSize];


      // 
      // SIZEs based on zoom & mirror
      //

      int zoomSize = min(1, (fullSize*zoom)/255 );                // zoomSize: NUM_LEDS_PER_STRIP
      
      int segmentSize = zoomSize;                                 // segmentSize: N_L_P_S
      if (mirror == 1 || mirror == 4)       segmentSize /= 2;
      else if (mirror == 2 || mirror == 5)  segmentSize /= 3;
      else if (mirror == 3 || mirror == 6)  segmentSize /= 4;



      //
      // PREPARE pix pos / start / end
      //

      int pStart;
      int pPos;
      int pEnd;

      int rap_tri;

      // COLOR NORM
      if (color_mode == COLOR_NORM) {
        
        if (pix_mode == 1) // 01:02
        {
          pStart = pix_length - 1;
          pEnd = pStart + pStart;
          pPos = ((((3*pStart + N_L_P_S) * pix_position) / 255) - (2*pStart + 1));
        }
        else if (pix_mode == 23 || pix_mode == 24) // tri, quadri
        {
          pStart = (((pix_length * N_L_P_S) / 255) - 1);
          pEnd = pStart + pStart;
          rap_tri = map(pix_length, 0, 255, 0, NUM_LEDS_PER_STRIP * 2);
          pPos = ((NUM_LEDS_PER_STRIP / 2 - rap_tri / 2) + map(pix_position, 0, 255, -NUM_LEDS_PER_STRIP + 1, NUM_LEDS_PER_STRIP + 1));
        }
        else 
        {
          pStart = (((pix_length * N_L_P_S) / 255) - 1);
          pEnd = pStart + pStart;
          pPos = ((((pStart + N_L_P_S + pEnd) * pix_position) / 255) - (pEnd + 1));
        }

      }

      // COLOR PICKER
      else if (color_mode == COLOR_PICKER) 
      { 
        pStart = -1;

        if (pix_mod == 2 || (pix_mod >= 6 && pix_mod <= 8) || (pix_mod >= 12 && pix_mod <= 14))
        {
          pPos = (((pStart + NUM_LEDS_PER_STRIP + pEnd) * pix_position) / 255) - (pEnd + 1);
        }
        else
        {
          pPos = (((pStart + N_L_P_S + pEnd) * pix_position) / 255) - (pEnd + 1);
        }
        
      }








    };             
};




#endif
