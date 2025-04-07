/*
  K32_artnet.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_artnet_h
#define K32_artnet_h

#include <ArtnetWiFi.h>         // https://github.com/hideakitai/ArtNet
#include <class/K32_plugin.h>
#include <K32_wifi.h>

#ifndef DMX_SUB_SLOTS
  #define DMX_SUB_SLOTS 16
  typedef void (*cbPtrDmx )(const uint8_t *data, int length);

  struct dmxsub
  {
    int address;
    int framesize;
    cbPtrDmx callback;
  };
#endif

class K32_artnet : K32_plugin {
  public:
    
    K32_artnet(K32* k32, K32_wifi* wifi, String name);

    void start();
    void stop();


    static void onDmx( dmxsub subscription );
    static void onFullDmx( cbPtrDmx callback );
    
    static cbPtrDmx fullCallback;
    static dmxsub subscriptions[DMX_SUB_SLOTS];
    static int _lastSequence;

    void command(Orderz* order);
    
  private:

    K32_wifi *wifi;
    ArtnetWiFiReceiver* artnet;

    static void check(void * parameter);
    static void _onArtnet(const uint8_t* data, const uint16_t length);

    TaskHandle_t xHandle = NULL;

};

#endif
