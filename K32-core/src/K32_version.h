/*
  K32_version.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_version_h
#define K32_version_h

#define MAX_HW 14             // Max value HW_REVISION

// #define K32_VERSION 2.00  // inter modules communication refactoring (modular / lazy includes)
// #define K32_VERSION 2.01  // add hw_3 ATOM
// #define K32_VERSION 2.02  // change HW_REVISON and Platformio.ini
// #define K32_VERSION 2.03  // add hw_4 ATOM_LITE 2 data led output
// #define K32_VERSION 2.04  // Multiple DMX fixtures
#define K32_VERSION 2.05     // add hw_3 & move atom to 13 ...


// LEDS PINS
// {LED_PIN0, LED_PIN1}
//
#define LED_N_STRIPS 2
const int LEDS_PIN[MAX_HW+1][LED_N_STRIPS] = {
  {-1, -1},               // HW_REVISION 0
  {21, 22},               // HW_REVISION 1 KXKM V1
  {23, 22},               // HW_REVISION 2 KXKM V2
  {23, 22},               // HW_REVISION 3 KXKM V3
  {-1, -1},               // HW_REVISION 4
  {-1, -1},               // HW_REVISION 5
  {-1, -1},               // HW_REVISION 6
  {-1, -1},               // HW_REVISION 7
  {-1, -1},               // HW_REVISION 8
  {-1, -1},               // HW_REVISION 9
  {-1, -1},               // HW_REVISION 10
  {-1, -1},               // HW_REVISION 11
  {-1, -1},               // HW_REVISION 12
  {27, 26},               // HW_REVISION 13: ATOM      (26 & 32 pin out ph2.0 || 27 intern)
  {27, 26}                // HW_REVISION 14: ATOM_LITE (26 & 32 pin out ph2.0 || 27 intern)
};

// PWM PINS
// {PWM_PIN0, PWM_PIN1, PWM_PIN2, PWM_PIN4}
//
#define PWM_N_CHAN 4
const int PWM_PIN[MAX_HW+1][PWM_N_CHAN] = {
  {-1, -1, -1, -1 },               // HW_REVISION 0
  {12, 13, -1, -1 },               // HW_REVISION 1 KXKM V1
  {14, 12, 13, 15 },               // HW_REVISION 2 KXKM V2
  {14, 12, 13, 15 },               // HW_REVISION 3 KXKM V3
  {-1, -1, -1, -1 },               // HW_REVISION 
  {-1, -1, -1, -1 },               // HW_REVISION 
  {-1, -1, -1, -1 },               // HW_REVISION 
  {-1, -1, -1, -1 },               // HW_REVISION 
  {-1, -1, -1, -1 },               // HW_REVISION 
  {-1, -1, -1, -1 },               // HW_REVISION 
  {-1, -1, -1, -1 },               // HW_REVISION 
  {-1, -1, -1, -1 },               // HW_REVISION 
  {-1, -1, -1, -1 },               // HW_REVISION 
  {32, -1, -1, -1 },               // HW_REVISION 13: ATOM        26 & 32 pin out ph2.0
  {32, -1, -1, -1 }                // HW_REVISION 14: ATOM_LITE   26 & 32 pin out ph2.0
};

// DMX PINS
// {DMX_DIRECTION, DMX_OUTPUT, DMX_INPUT}
//
#define DMX_N_FIXTURES 16  // NBR OF FIXTURES ON DMX BUS
const int DMX_PIN[MAX_HW+1][3] = {
  {-1, -1, -1 },               // HW_REVISION 0
  {33, 32, 35 },               // HW_REVISION 1 KXKM V1
  { 4, 16, 17 },               // HW_REVISION 2 KXKM V2
  { 4, 16, 17 },               // HW_REVISION 3 KXKM V3
  {-1, -1, -1 },               // HW_REVISION 4
  {-1, -1, -1 },               // HW_REVISION 5
  {-1, -1, -1 },               // HW_REVISION 6
  {-1, -1, -1 },               // HW_REVISION 7
  {-1, -1, -1 },               // HW_REVISION 8
  {-1, -1, -1 },               // HW_REVISION 9
  {-1, -1, -1 },               // HW_REVISION 10
  {-1, -1, -1 },               // HW_REVISION 11
  {-1, -1, -1 },               // HW_REVISION 12
  {22, 19, 23 },               // HW_REVISION 13: ATOM
  {-1, -1, -1 }                // HW_REVISION 14: ATOM_LITE
};

// AUDIO PINS
// {I2C_SDA_PIN, I2C_SCL_PIN, I2S_LRCK_PIN, I2S_DATA_PIN, I2S_BCK_PIN}
//
const int AUDIO_PIN[MAX_HW+1][5] = {
  {-1, -1, -1, -1, -1},   // HW_REVISION 0
  {2, 4, 27, 26, 25},     // HW_REVISION 1 KXKM V1
  {32, 33, 25, 26, 27},   // HW_REVISION 2 KXKM V2
  {32, 33, 25, 26, 27},   // HW_REVISION 3 KXKM V3
  {-1, -1, -1, -1, -1},   // HW_REVISION 4
  {-1, -1, -1, -1, -1},   // HW_REVISION 5
  {-1, -1, -1, -1, -1},   // HW_REVISION 6
  {-1, -1, -1, -1, -1},   // HW_REVISION 7
  {-1, -1, -1, -1, -1},   // HW_REVISION 8
  {-1, -1, -1, -1, -1},   // HW_REVISION 9
  {-1, -1, -1, -1, -1},   // HW_REVISION 10
  {-1, -1, -1, -1, -1},   // HW_REVISION 11
  {-1, -1, -1, -1, -1},   // HW_REVISION 12
  {-1, -1, -1, -1, -1},   // HW_REVISION 13 : ATOM
  {-1, -1, -1, -1, -1}    // HW_REVISION 14 : ATOM_LITE

};

// SD PINS
// {SD_DI_PIN, SD_DO_PIN, SD_SCK_PIN, SD_CS_PIN}
//
const int SD_PIN[MAX_HW+1][4] = {
  {-1, -1, -1, -1},        // HW_REVISION 0
  {23, 19, 18,  5},        // HW_REVISION 1
  {19,  5, 18, 21},        // HW_REVISION 2
  {-1, -1, -1, -1},        // HW_REVISION 3
  {-1, -1, -1, -1},        // HW_REVISION 4
  {-1, -1, -1, -1},        // HW_REVISION 5
  {-1, -1, -1, -1},        // HW_REVISION 6
  {-1, -1, -1, -1},        // HW_REVISION 7
  {-1, -1, -1, -1},        // HW_REVISION 8
  {-1, -1, -1, -1},        // HW_REVISION 9
  {-1, -1, -1, -1},        // HW_REVISION 10
  {-1, -1, -1, -1},        // HW_REVISION 11
  {-1, -1, -1, -1},        // HW_REVISION 12
  {-1, -1, -1, -1},        // HW_REVISION 13 : ATOM
  {-1, -1, -1, -1}         // HW_REVISION 14 : ATOM_LITE

};

// MCP PINS
// {I2C_SDA_PIN, I2C_SCL_PIN}
//
const int MCP_PIN[MAX_HW+1][2] = {
  {-1, -1},          // HW_REVISION 0
  { 2,  4},          // HW_REVISION 1
  {32, 33},          // HW_REVISION 2
  {32, 33},          // HW_REVISION 3
  {-1, -1},          // HW_REVISION 4
  {-1, -1},          // HW_REVISION 5
  {-1, -1},          // HW_REVISION 6
  {-1, -1},          // HW_REVISION 7
  {-1, -1},          // HW_REVISION 8
  {-1, -1},          // HW_REVISION 9
  {-1, -1},          // HW_REVISION 10
  {-1, -1},          // HW_REVISION 11
  {-1, -1},          // HW_REVISION 12
  {-1, -1},          // HW_REVISION 13 : ATOM
  {21, 25}           // HW_REVISION 14 : ATOM_LITE
};

// Current sensor PIN (ADC)
// {Current Sensor Pin}
//
const int CURRENT_PIN[MAX_HW+1] = {
  -1,           // HW_REVISION 0
  -1,           // HW_REVISION 1
  35,           // HW_REVISION 2
  35,           // HW_REVISION 3
  -1,           // HW_REVISION 4
  -1,           // HW_REVISION 5
  -1,           // HW_REVISION 6
  -1,           // HW_REVISION 7
  -1,           // HW_REVISION 8
  -1,           // HW_REVISION 9
  -1,           // HW_REVISION 10
  -1,           // HW_REVISION 11
  -1,           // HW_REVISION 12
  -1,           // HW_REVISION 13 : ATOM
  -1            // HW_REVISION 14 : ATOM_LITE
};

// DIP_SWITCH PIN 
// {Dip_Switch Pin}
//
const int DIP_PIN[MAX_HW+1] = {
  -1,           // HW_REVISION 0
  -1,           // HW_REVISION 1
  -1,           // HW_REVISION 2
  34,           // HW_REVISION 3
  -1,           // HW_REVISION 4
  -1,           // HW_REVISION 5
  -1,           // HW_REVISION 6
  -1,           // HW_REVISION 7
  -1,           // HW_REVISION 8
  -1,           // HW_REVISION 9
  -1,           // HW_REVISION 10
  -1,           // HW_REVISION 11
  -1,           // HW_REVISION 12
  -1,           // HW_REVISION 13 : ATOM
  -1            // HW_REVISION 14 : ATOM_LITE
};

#endif
