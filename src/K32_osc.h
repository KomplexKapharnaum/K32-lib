/*
  K32_osc.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_osc_h
#define K32_osc_h

class K32_osc;      // prevent cicular include error
#include "K32.h"

#include <WiFi.h>
#include <WiFiUdp.h>

#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>

class K32_osc {
  public:
    K32_osc(int port, K32* engine, int beatInterval, int beaconInterval);
    const char* id_path();
    const char* chan_path();

    OSCMessage status();

  private:
    static void server( void * parameter );
    static void beacon( void * parameter );
    static void beat( void * parameter );

    WiFiUDP* udp;
    int port;
    IPAddress linkedIP;
    int  beatInterval = 0;
    int beaconInterval = 0;

    K32* engine;
};

// OSCMessage overload
class K32_oscmsg : public OSCMessage {
  public:
    K32_oscmsg(K32_osc* env);

    bool dispatch(const char * pattern, void (*callback)(K32_osc* env, K32_oscmsg &), int = 0);
    bool route(const char * pattern, void (*callback)(K32_osc* env, K32_oscmsg &, int), int = 0);
    String getStr(int position);

    K32_osc* env;
};

#endif
