/*
  K32_bluetooth.h
  Created by Thomas BOHL, july 2020.
  Released under GPL v3.0
*/
#ifndef K32_bluetooth_h
#define K32_bluetooth_h

#define KWIFI_CONNECTION_TIMEOUT 10000

#include <class/K32_plugin.h>
#include <BluetoothSerial.h>


typedef void (*cbPtrBTcon )();
typedef void (*cbPtrBTdata)(uint8_t *data, int length);

class K32_bluetooth : K32_plugin {
  public:
    K32_bluetooth(K32* k32, String nameDevice);

    bool isConnected();
    static void onConnect( cbPtrBTcon callback );
    static void onDisconnect( cbPtrBTcon callback );

    long getRSSI();

    // custom callback
    void onCmd( cbPtrBTdata callback );

    // Send
    void send(String str);


    static cbPtrBTdata cmdCallback;

    BluetoothSerial* serial;
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

    static cbPtrBTcon conCallback;
    static cbPtrBTcon disconCallback;

    void command( Orderz* order );
};


#endif
