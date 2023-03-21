/*
  K32_web.h
  Created by Thomas BOHL, february 2023.
  Released under GPL v3.0
*/
#ifndef K32_web_h
#define K32_web_h

#include <K32_system.h>
#include <class/K32_plugin.h>

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Arduino_JSON.h>


class K32_web : K32_plugin {
  public:
    K32_web(K32* k32); // TODO: remove wifi dependancy (use intercom)
    void notifyClients(String msg);

    String getSliderValues();
    String getConf();

  private:

    static void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);

    void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);

};

#endif