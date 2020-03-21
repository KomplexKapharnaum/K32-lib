/*
  K32_version.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_version_h
#define K32_version_h

#define MAX_HW 3          // Max value HW_REVISION

// #define K32_VERSION 2.00  // inter modules communication refactoring (modular / lazy includes)
// #define K32_VERSION 2.01  // add hw_3 ATOM
#define K32_VERSION 2.02  // change HW_REVISON and Platformio.ini



// LEDS PINS
// {LED_PIN0, LED_PIN1}
//
const int LEDS_PIN[MAX_HW+1][2] = {
  {21, 22},               // HW_REVISION 0
  {21, 22},               // HW_REVISION 1
  {23, 22},               // HW_REVISION 2
  {26, 32}                // HW_REVISION 3: ATOM
};

// AUDIO PINS
// {I2C_SDA_PIN, I2C_SCL_PIN, I2S_LRCK_PIN, I2S_DATA_PIN, I2S_BCK_PIN}
//
const int AUDIO_PIN[MAX_HW+1][5] = {
  {2, 4, 27, 26, 25},     // HW_REVISION 0
  {2, 4, 27, 26, 25},     // HW_REVISION 1
  {32, 33, 25, 26, 27},   // HW_REVISION 2
  {27, 39, 22, 19, 23}    // HW_REVISION 3 : ATOM
};

// SD PINS
// {SD_DI_PIN, SD_DO_PIN, SD_SCK_PIN, SD_CS_PIN}
//
const int SD_PIN[MAX_HW+1][4] = {
  {23, 19, 18, 5},        // HW_REVISION 0
  {23, 19, 18, 5},        // HW_REVISION 1
  {19, 5, 18, 21},        // HW_REVISION 2
  {22, 19, 23, 33}        // HW_REVISION 3 : ATOM
};

// Buttons PINS
// {I2C_SDA_PIN, I2C_SCL_PIN}
//
const int BTN_PIN[MAX_HW+1][2] = {
  {2, 4},          // HW_REVISION 0
  {2, 4},          // HW_REVISION 1
  {32, 33},        // HW_REVISION 2
  {25, 21}         // HW_REVISION 3 : ATOM
};

#endif
