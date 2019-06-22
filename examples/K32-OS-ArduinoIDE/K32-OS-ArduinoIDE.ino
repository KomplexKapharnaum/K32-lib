// #define NODEID 42

#include "K32.h"

K32* engine;

void setup() {

  engine = new K32({
    .stm32    = true,     // stm32 event listening and battery monitoring
    .leds     = true,     // dual ws2812 
    .audio    = true,     // audio engine with PCM51xx sound card
    .sampler  = true,     // media indexing to midi bank/note-xxx
    .wifi     = {         
      .ssid = "kxkm24",             // ssid (NULL to disable)
      .password = NULL,             // password (NULL if not secured)
      .ip = NULL                    // static ip (NULL to use DHCP)
    },
    .osc  = {
      .port_in  = 1818,             // osc port input (0 = disable)
      .port_out = 1819,             // osc port output (0 = disable)
      .beatInterval     = 0,        // heartbeat interval milliseconds (0 = disable)
      .beaconInterval   = 5000      // full beacon interval milliseconds (0 = disable)
    }
  });

  // Settings SET
  #ifdef NODEID
    engine->settings->set("id", NODEID);
    engine->settings->set("channel", 15);
  #endif
    engine->settings->set("model", 2);   // 0: proto -- 1: big -- 2: small


}

void loop() {

  // if (engine->stm32->clicked()) {
  //   if (engine->audio->isPlaying() || engine->leds->isPlaying()) {
  //     engine->audio->stop();
  //     engine->leds->stop();
  //   }
  //   else {
  //     engine->audio->loop(true);
  //     engine->audio->play( "/test.mp3" );
  //     engine->audio->volume(50);
  //     engine->leds->play("sinus");
  //   }
  // }
  // else if (engine->stm32->dblclicked())
  //   engine->stm32->reset();

  delay(10);

  // LOGF("Heap: %d \n", xPortGetFreeHeapSize());

}
