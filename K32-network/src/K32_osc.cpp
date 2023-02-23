/*
  K32_osc.cpp
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/

#include "K32_version.h"
#include "K32_osc.h"

#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <WiFi.h>

/*
 *   OSCMessage extension
 */

K32_oscmsg::K32_oscmsg(K32_osc* env){
  this->env = env;
};

bool K32_oscmsg::dispatch(const char * pattern, void (*callback)(K32_osc* env, K32_oscmsg &), int addr_offset){
	if (fullMatch(pattern, addr_offset)){
		callback(this->env, *this);
		return true;
	} else {
		return false;
	}
}

bool K32_oscmsg::route(const char * pattern, void (*callback)(K32_osc* env, K32_oscmsg &, int), int initial_offset){
	int match_offset = match(pattern, initial_offset);
	if (match_offset>0){
		callback(this->env, *this, match_offset + initial_offset);
		return true;
	} else {
		return false;
	}
}

String K32_oscmsg::getStr(int position) {
  int length = this->getDataLength(position);
  char msg[length];
  this->getString(position, msg, length);
  return String(msg);
}


/*
 *   PUBLIC
 */

K32_osc::K32_osc(K32* k32, K32_wifi* wifi, K32_stm32* stm32) : K32_plugin("osc", k32), wifi(wifi), stm32(stm32) {}


void K32_osc::start(oscconf conf)
{ 
  this->conf = conf;

  this->lock = xSemaphoreCreateMutex();
  this->udp = new WiFiUDP();
  this->sendSock = new WiFiUDP();

  // OSC INPUT
  if (this->conf.port_in > 0) {
    this->udp->begin(this->conf.port_in);

    // LOOP server
    xTaskCreate( this->server,          // function
                  "osc_server",         // server name
                  8000,               // stack memory
                  (void*)this,        // args
                  5,                  // priority
                  &xHandle1           // handler
                  );                  // core 
  }

  // OSC OUTPUT
  if (this->conf.port_out > 0) {
    
    // LOOP beat
    if (this->conf.beatInterval > 0)
      xTaskCreate( this->beat,          // function
                  "osc_beat",         // server name
                  2000,              // stack memory
                  (void*)this,        // args
                  0,                  // priority
                  &xHandle2              // handler
                  );                // core 

    // LOOP beacon
    if (this->conf.statusInterval > 0)
      xTaskCreate( this->beacon,          // function
                  "osc_beacon",         // server name
                  2000,              // stack memory
                  (void*)this,        // args
                  0,                  // priority
                  &xHandle3              // handler
                  );                // core 
  }

  
};

void K32_osc::stop() 
{
  if (xHandle1 != NULL) vTaskDelete(xHandle1);
  if (xHandle2 != NULL) vTaskDelete(xHandle2);
  if (xHandle3 != NULL) vTaskDelete(xHandle3);
  xHandle1 = NULL;
  xHandle2 = NULL;
  xHandle3 = NULL;

  this->udp->stop();
  this->sendSock->stop();
}

OSCMessage K32_osc::beatMsg() 
{
    OSCMessage msg("/beat");
    msg.add(k32->system->id());
    return msg;
}


OSCMessage K32_osc::statusMsg() {

    OSCMessage msg("/status");

    // identity
    msg.add(k32->system->id());
    msg.add(k32->system->channel());

    // wifi 
    byte mac[6];
    WiFi.macAddress(mac);
    char shortmac[16];
    sprintf(shortmac, "%02X:%02X:%02X", mac[3], mac[4], mac[5]);
    msg.add(shortmac);
    msg.add(WiFi.localIP().toString().c_str());
    msg.add(WiFi.RSSI());
    
    (this->linkedIP) ? msg.add(1) : msg.add(0);
    
    // battery %
    if (stm32) msg.add(stm32->battery());
    else msg.add(0);

    // voltage 
    if (stm32) msg.add(stm32->voltage());
    else msg.add(0);

    // audio 
    // if (intercom->audio) {
    //   msg.add(intercom->audio->isSdOK());
    //   (intercom->audio->media() != "") ? msg.add(intercom->audio->media().c_str()) : msg.add("stop");
    //   msg.add(intercom->audio->error().c_str());
    // }
    // else {
    //   msg.add(false);   /// TODO : SD check without audio engine
    //   msg.add("stop");
    //   msg.add("");
    // }

    // sampler
    // if (intercom->audio && intercom->audio->sampler) msg.add(intercom->audio->sampler->bank());
    // else msg.add(0);

    // filesync 
    // msg.add(sync_size());
    // msg.add(sync_getStatus().c_str());
    
    return msg;
}

void K32_osc::send( OSCMessage msg ) 
{
  if (wifi->isConnected() && this->conf.port_out > 0) 
  { 
    xSemaphoreTake(this->lock, portMAX_DELAY);
    IPAddress dest = (this->linkedIP) ? this->linkedIP : wifi->broadcastIP();
    this->sendSock->beginPacket( dest, this->conf.port_out);
    msg.send(*this->sendSock);
    this->sendSock->endPacket();
    xSemaphoreGive(this->lock);
  }
}


void K32_osc::command(Orderz* order) {
  // TODO: orderz OSC
}


/*
 *   PRIVATE
 */

void K32_osc::beat( void * parameter ) {
    K32_osc* that = (K32_osc*) parameter;
    TickType_t xFrequency = pdMS_TO_TICKS(that->conf.beatInterval);
    WiFiUDP sock;

    while(true) 
    { 
      that->send( that->beatMsg() );
      vTaskDelay( xFrequency );
    }

    vTaskDelete(NULL);
}



void K32_osc::beacon( void * parameter ) {

    K32_osc* that = (K32_osc*) parameter;
    TickType_t xFrequency = pdMS_TO_TICKS(that->conf.statusInterval);
    WiFiUDP sock;

    while(true) 
    {
      that->send( that->statusMsg() );
      vTaskDelay( xFrequency );
    }

    vTaskDelete(NULL);
}



void K32_osc::server( void * parameter ) {
   K32_osc* that = (K32_osc*) parameter;
   TickType_t xFrequency = pdMS_TO_TICKS(1);
   int size = 0;
   K32_oscmsg msg(that);
   IPAddress remoteIP;

   // Wait for WIFI to first connect
   while(!that->wifi->isConnected()) delay( 300 );
   
   MDNS.addService("_osc", "_udp", that->conf.port_in);
   mdns_service_instance_name_set("_osc", "_udp", ("OSC._"+that->k32->system->name()).c_str());

   LOGF("OSC: listening on port %d\n", that->conf.port_in);

   char idpath[8];
   sprintf(idpath, "/k32/e%u", that->k32->system->id()); 

   char chpath[8];
   sprintf(chpath, "/k32/c%u", that->k32->system->channel());

    that->emit("osc/started");

   while(true) {
     
      xSemaphoreTake(that->lock, portMAX_DELAY);

      size = that->udp->parsePacket();
      if (size > 0) {
        msg.empty();
        while (size--) msg.fill(that->udp->read());
        if (!msg.hasError()) {
          remoteIP = that->udp->remoteIP();

          char adr[256];
          msg.getAddress(adr);
          LOGINL("OSC: rcv  ");
          LOG(adr);

          //
          // GENERAL PING
          //
          msg.dispatch("/ping", [](K32_osc* that, K32_oscmsg &msg){
            LOGINL("OSC: /ping RECV from " );
            LOG(that->udp->remoteIP());

            that->linkedIP = that->udp->remoteIP();

            if (that->conf.port_out > 0) {
              OSCMessage response("/pong");
              that->udp->beginPacket(that->udp->remoteIP(), that->conf.port_out);
              response.send(*that->udp);
              that->udp->endPacket();
            }
          });

          //
          // GENERAL INFO
          //
          msg.dispatch("/info", [](K32_osc* that, K32_oscmsg &msg){
            LOGINL("OSC: /info RECV from " );
            LOG(that->udp->remoteIP());

            if (that->conf.port_out > 0) {
              that->udp->beginPacket(that->udp->remoteIP(), that->conf.port_out);
              that->statusMsg().send(*that->udp);
              that->udp->endPacket();
            }
          });

          //
          // IDENTITY ROUTING
          //
          auto router = [](K32_osc* that, K32_oscmsg &msg, int offset){

            char path[32];
            msg.getAddress(path, 5);  // 5 = /k32/

            Orderz* newOrder = new Orderz( strchr(path, '/')+1 );
            for(int k=0; k<msg.size(); k++) {
              if (msg.isInt(k))         newOrder->addData(msg.getInt(k));
              else if (msg.isString(k)) newOrder->addData(msg.getStr(k).c_str());
              else newOrder->addData("?");
            }

            that->cmd(newOrder);
          };

          // ROUTE ALL / DEVICE ID / CHANNEL GROUP
          msg.route("/k32/all", router);
          msg.route( idpath, router);
          msg.route( chpath, router);

        } else {
          LOGINL("OSC: error ");
          LOG(msg.getError());
        }


        xSemaphoreGive(that->lock);
      }
      else {
        xSemaphoreGive(that->lock);
        vTaskDelay( xFrequency );
      }
      
   }

   vTaskDelete(NULL);
}
