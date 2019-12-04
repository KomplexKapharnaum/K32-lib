/*
  K32_version.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_version_h
#define K32_version_h

// #define K32_VERSION 1.00
// #define K32_VERSION 1.01  // audio forced kill to avoid deadlock
// #define K32_VERSION 1.02  // audio keep task active
// #define K32_VERSION 1.03  // fixed audio memory leak
// #define K32_VERSION 1.04  // v2 board
// #define K32_VERSION 1.05  // led anim sinus ++
// #define K32_VERSION 1.06  // mqtt (not enabled yet)
// #define K32_VERSION 1.07  // fix various bug (wifi reconnect / audio missing / ...)
// #define K32_VERSION 1.08     // mqtt sampler
// #define K32_VERSION 1.09     // mqtt leds
// #define K32_VERSION 1.10     // mqtt audio
// #define K32_VERSION 1.11     // fix
// #define K32_VERSION 1.12     // mqtt ping before reconnect
// #define K32_VERSION 1.13     // mqtt fix reconnect
// #define K32_VERSION 1.15     // mqtt bank select
// #define K32_VERSION 1.16     // mqtt / osc / esp-regie
// #define K32_VERSION 1.17     // memory osc bug
// #define K32_VERSION 1.18        // disable onclick (buggy !)
// #define K32_VERSION 1.19        // deprecated /audio/sample (keep max compat)
#define K32_VERSION 2.00 // inter modules communication refactoring (modular / lazy includes)

// LEDS PINS ON K32-BOARD
// {LED_PIN0, LED_PIN1}
//
const int LEDS_PIN[3][2] = {
  {21, 22},               // HW_REVISION 0
  {21, 22},               // HW_REVISION 1
  {23, 22}                // HW_REVISION 2
};

// AUDIO PINS ON K32-BOARD
// {I2C_SDA_PIN, I2C_SCL_PIN, I2S_LRCK_PIN, I2S_DATA_PIN, I2S_BCK_PIN}
//
const int AUDIO_PIN[3][5] = {
  {2, 4, 27, 26, 25},     // HW_REVISION 0
  {2, 4, 27, 26, 25},     // HW_REVISION 1
  {32, 33, 25, 26, 27}    // HW_REVISION 2
};

// SD PINS ON K32-BOARD
// {SD_DI_PIN, SD_DO_PIN, SD_SCK_PIN, SD_CS_PIN}
//
const int SD_PIN[3][4] = {
  {23, 19, 18, 5},        // HW_REVISION 0
  {23, 19, 18, 5},        // HW_REVISION 1
  {19, 5, 18, 21}         // HW_REVISION 2
};

#endif
