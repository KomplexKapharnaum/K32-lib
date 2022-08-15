/*
  K32_osc.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_osc_h
#define K32_osc_h

#include <K32_system.h>
#include <K32_wifi.h>
#include <hardware/K32_stm32.h>

#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>

struct oscconf
{
  int port_in;
  int port_out;
  int beatInterval;
  int statusInterval;
};

class K32_osc : K32_plugin {
  public:
    K32_osc(K32* k32, K32_wifi* wifi, K32_stm32* stm32 = nullptr); // TODO: remove wifi dependancy (use intercom)

    void start(oscconf conf);
    void stop();

    const char* id_path();
    const char* chan_path();

    OSCMessage statusMsg();
    OSCMessage beatMsg();

    void send(OSCMessage msg);

    void command(Orderz* order);

  private:
    SemaphoreHandle_t lock;
    static void server( void * parameter );
    static void beacon( void * parameter );
    static void beat( void * parameter );

    WiFiUDP* udp;         // must be protected with lock 
    WiFiUDP* sendSock;
    IPAddress linkedIP;

    oscconf conf;

    K32_system *system;
    K32_wifi *wifi;
    K32_stm32 *stm32;

    TaskHandle_t xHandle1 = NULL;
    TaskHandle_t xHandle2 = NULL;
    TaskHandle_t xHandle3 = NULL;
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
