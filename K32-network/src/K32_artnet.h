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

#define ARTNET_SUB_SLOTS 16

typedef void (*cbPtrArtnet )(const uint8_t *data, int length);

struct artnetsub
{
  int address;
  int framesize;
  cbPtrArtnet callback;
};

class K32_artnet : K32_plugin {
  public:
    
    K32_artnet(K32* k32, K32_wifi* wifi, String name);

    void start();
    void stop();


    static void onDmx( artnetsub subscription );
    static void onFullDmx( cbPtrArtnet callback );
    
    static cbPtrArtnet fullCallback;
    static artnetsub subscriptions[ARTNET_SUB_SLOTS];
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
