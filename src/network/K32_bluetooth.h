/*
  K32_bluetooth.h
  Created by Thomas BOHL, july 2020.
  Released under GPL v3.0
*/
#ifndef K32_bluetooth_h
#define K32_bluetooth_h

#include "system/K32_log.h"

#include "system/K32_system.h"
#include "audio/K32_audio.h"
#include "light/K32_light.h"
#include "xtension/K32_remote.h"

#include <BluetoothSerial.h>


class K32_bluetooth {
  public:
    K32_bluetooth(String nameDevice, K32_system *system, K32_audio *audio, K32_light *light, K32_remote *remote);

    BluetoothSerial* serial;

    bool isConnected();
    void onConnect( void (*callback)(void) );
    void onDisconnect( void (*callback)(void) );

    long getRSSI();

    // custom callback
    typedef void (*cbPtr)(uint8_t *data, int length);
    static cbPtr cmdCallback;
    void onCmd( cbPtr callback );

    // Send
    void send(String str);

    String nameDevice;

  private:
    SemaphoreHandle_t lock;
    static void state( void * parameter );
    static void server( void * parameter );

    void dispatch(char* data, size_t length);

    static bool ok;
    static bool didConnect;
    static bool didDisconnect;
    static void event(esp_spp_cb_event_t ev, esp_spp_cb_param_t *param);

    void splitString(char *data, const char *separator, int index, char *result);
    
    void (*conCallback)(void);
    void (*disconCallback)(void);

    K32_system *system;
    K32_audio *audio;
    K32_light *light;
    K32_remote *remote;

};


#endif
