/*
  K32_artnet.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_artnet_h
#define K32_artnet_h

#include <ArtnetWifi.h> //https://github.com/rstephan/ArtnetWifi


struct artnetconf
{
  int universe;
  int address;
  int framesize;
};


class K32_artnet {
  public:
    typedef void (*cbPtr)(uint8_t *data, int length);
    
    K32_artnet();
    void start(artnetconf conf);

    void onDmx( cbPtr callback );


    static artnetconf conf;    
    static cbPtr frameCallback;
    static int _lastSequence;

  private:

    ArtnetWifi* artnet;

    static void check(void * parameter);
    static void _onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t *data);

};

#endif
