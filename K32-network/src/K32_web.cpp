/*
  K32_web.cpp
  Created by Thomas BOHL, february 2023.
  Released under GPL v3.0
*/
#include "K32_web.h"
// #include "SPIFFS.h"

AsyncWebServer* server  = nullptr;
AsyncWebSocket* ws  = nullptr;
K32_web* that = nullptr;

K32_web::K32_web(K32* k32) : K32_plugin("web", k32)
{ 
  that = this;
  if (server) return;

  if (!SPIFFS.begin()) LOG("WEB: An error has occurred while mounting SPIFFS");
  else LOG("WEB: SPIFFS mounted successfully");

  ws = new AsyncWebSocket("/ws");
  ws->onEvent(K32_web::onEvent);
  
  server = new AsyncWebServer(80);
  server->addHandler(ws);
  
  server->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });
  
  server->serveStatic("/", SPIFFS, "/"); 

  LOG("WEB: server ready, waiting for wifi..");

  // Wait for WIFI to start server
  k32->on("wifi/connected", [](Orderz* order)
  {
      server->begin();
      LOG("WEB: server started on port 80");
  });
}

void K32_web::notifyClients(String msg) 
{
  if (!ws) return;
  ws->textAll(msg);
}


String K32_web::getConf() 
{
  JSONVar conf;

  conf["nodeid"]  = k32->system->id();
  conf["nodeip"]  = WiFi.localIP().toString();
  conf["version"] = k32->version;
  conf["hwrev"]   = k32->system->hw();

  conf["channel"] = k32->system->channel();
  conf["lightid"] = k32->system->lightid();
  conf["dmxuni"]  = k32->system->universe();
  conf["pixsize"] = k32->system->pixsize();

  return JSON.stringify(conf);
}


/*
 *   PRIVATE
 */


void K32_web::onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) 
{
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      that->handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void K32_web::handleWebSocketMessage(void *arg, uint8_t *data, size_t len) 
{
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    String message = (char*)data;
    
    // GET
    //

    if (strcmp((char*)data, "getConf") == 0) {
      this->notifyClients(this->getConf());
    }

    // SET
    //

    if (message.indexOf("conf") >= 0) {

      String conf = message.substring(4);

      JSONVar root = JSON.parse(conf);
      if (JSON.typeof(root) == "undefined") {
        Serial.println("WEB: JSON parseObject() failed");
        return;
      }

      // CHANNEL
      //
      if (JSON.typeof(root["channel"]) == "string") {
        String chan = root["channel"];
        k32->system->channel(chan.toInt());
      }

      // LIGHT ID
      //
      if (JSON.typeof(root["lightid"]) == "string") {
        String id = root["lightid"];
        k32->system->lightid(id.toInt());
      }

      // DMX UNIVERSE
      //
      if (JSON.typeof(root["dmxuni"]) == "string") {
        String uni = root["dmxuni"];
        k32->system->universe(uni.toInt());
      }

      // LEDS STRIP SIZE
      //
      if (JSON.typeof(root["pixsize"]) == "string") {
        String n = root["pixsize"];
        k32->system->pixsize(n.toInt());
      }

      this->notifyClients("reset");
      delay(100);
      k32->system->reset();
    }

  }
}
