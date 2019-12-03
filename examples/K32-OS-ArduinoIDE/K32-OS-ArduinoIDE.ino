#include <Arduino.h>

#define K32_SET_NODEID        1   // board unique id
#define K32_SET_HWREVISION    1   // board HW revision 

#define K32_ENABLE_WIFI       // MEM = 45%
#define K32_ENABLE_LIGHT       // MEM = 2%
// #define K32_ENABLE_STM32      // MEM = 2%
// #define K32_ENABLE_AUDIO      // MEM = 25%

#include "K32.h"
K32* k32;


void setup() {
  
  k32 = new K32();

  // k32->wifi->add("ReMoTe");
  // k32->wifi->add("kxkm24lulu", NULL, "2.0.0."+String(k32->settings->id()+100), "255.0.0.0", "2.0.0.1");
  // k32->wifi->add("interweb", "superspeed37");

}


void loop() {

  // if (engine->stm32->clicked()) {

  //   if (engine->audio->isPlaying()) {
  //     engine->audio->stop();
  //   }
  //   else {
  //     engine->audio->loop(true);
  //     engine->audio->play( engine->sampler->path( engine->settings->get("channel"), 0)  );
  //     engine->audio->volume(50);
  //   }

  // //   // if (engine->audio->isPlaying() || engine->leds->isPlaying()) {
  // //   //   engine->audio->stop();
  // //   //   engine->leds->stop();
  // //   // }
  // //   // else {
  // //   //   engine->audio->loop(true);
  // //   //   engine->audio->play( engine->sampler->path(3, 0)  );
  // //   //   engine->audio->volume(50);
  // //   //   engine->leds->play("sinus");
  // //   // }

  // //   // engine->stm32->send(K32_stm32_api::SET_LOAD_SWITCH, 0);
  // //   // delay(2000);
  // //   // engine->stm32->send(K32_stm32_api::SET_LOAD_SWITCH, 1);
  // }

  // if (engine->stm32->dblclicked())
  //   engine->audio->stop();

  delay(500);

  // Serial.println(ESP.getFreeHeap());

}
