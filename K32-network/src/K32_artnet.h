/*
  K32_artnet.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_artnet_h
#define K32_artnet_h

#include <Artnet.h>         // https://github.com/hideakitai/ArtNet
#include <class/K32_plugin.h>

struct artnetconf
{
  int universe;
  int address;
  int framesize;
};


class K32_artnet : K32_plugin {
  public:
    typedef void (*cbPtr)(const uint8_t *data, int length);
    
    K32_artnet(K32* k32, artnetconf conf);
    void start();
    void stop();

    void onDmx( cbPtr callback );
    void onFullDmx( cbPtr callback );


    static artnetconf conf;    
    static cbPtr frameCallback;
    static cbPtr fullCallback;
    static int _lastSequence;

  private:

    ArtnetWiFiReceiver* artnet;

    static void check(void * parameter);
    static void _onArtnet(const uint8_t* data, const uint16_t length);

    TaskHandle_t xHandle = NULL;

};

#endif
