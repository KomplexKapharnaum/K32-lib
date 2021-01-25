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

K32_osc::K32_osc(K32_intercom *intercom) : intercom(intercom) {}


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
    xTaskCreatePinnedToCore( this->server,          // function
                  "osc_server",         // server name
                  10000,               // stack memory
                  (void*)this,        // args
                  5,                  // priority
                  NULL,              // handler
                  0);                // core 
  }

  // OSC OUTPUT
  if (this->conf.port_out > 0) {
    
    // LOOP beat
    if (this->conf.beatInterval > 0)
      xTaskCreatePinnedToCore( this->beat,          // function
                  "osc_beat",         // server name
                  2000,              // stack memory
                  (void*)this,        // args
                  0,                  // priority
                  NULL,              // handler
                  0);                // core 

    // LOOP beacon
    if (this->conf.beaconInterval > 0)
      xTaskCreatePinnedToCore( this->beacon,          // function
                  "osc_beacon",         // server name
                  2000,              // stack memory
                  (void*)this,        // args
                  1,                  // priority
                  NULL,              // handler
                  0);                // core 
  }

  
};


OSCMessage K32_osc::status() {

    OSCMessage msg("/status");

    // identity
    msg.add(intercom->system->id());
    msg.add(intercom->system->channel());
    msg.add(K32_VERSION);

    // wifi 
    byte mac[6];
    WiFi.macAddress(mac);
    char shortmac[16];
    sprintf(shortmac, "%02X:%02X:%02X", mac[3], mac[4], mac[5]);
    msg.add(shortmac);
    msg.add(WiFi.localIP().toString().c_str());
    msg.add(WiFi.RSSI());
    (this->linkedIP) ? msg.add(true) : msg.add(false);
    
    // energy 
    if (intercom->system->stm32) msg.add(intercom->system->stm32->battery());
    else msg.add(0);

    // audio 
    if (intercom->audio) {
      msg.add(intercom->audio->isSdOK());
      (intercom->audio->media() != "") ? msg.add(intercom->audio->media().c_str()) : msg.add("stop");
      msg.add(intercom->audio->error().c_str());
    }
    else {
      msg.add(false);   /// TODO : SD check without audio engine
      msg.add("stop");
      msg.add("");
    }

    // sampler
    if (intercom->audio && intercom->audio->sampler) msg.add(intercom->audio->sampler->bank());
    else msg.add(0);

    // filesync 
    // msg.add(sync_size());
    // msg.add(sync_getStatus().c_str());
    msg.add(0);   // SYNC count files
    msg.add("");  // SYNC erro
    
    return msg;
}

void K32_osc::send( OSCMessage msg ) {
  if (intercom->wifi->isConnected() && this->conf.port_out > 0) { 
    xSemaphoreTake(this->lock, portMAX_DELAY);
    IPAddress dest = (this->linkedIP) ? this->linkedIP : intercom->wifi->broadcastIP();
    this->sendSock->beginPacket( dest, this->conf.port_out);
    msg.send(*this->sendSock);
    this->sendSock->endPacket();
    xSemaphoreGive(this->lock);
  }
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
      that->send( OSCMessage("/beat") );
      vTaskDelay( xFrequency );
    }

    vTaskDelete(NULL);
}



void K32_osc::beacon( void * parameter ) {

    K32_osc* that = (K32_osc*) parameter;
    TickType_t xFrequency = pdMS_TO_TICKS(that->conf.beaconInterval);
    WiFiUDP sock;

    while(true) 
    {
      if (that->intercom->wifi->isConnected()) { 
        // send
        xSemaphoreTake(that->lock, portMAX_DELAY);
        IPAddress dest = (that->linkedIP) ? that->linkedIP : that->intercom->wifi->broadcastIP();
        sock.beginPacket( dest, that->conf.port_out);
        that->status().send(sock);
        sock.endPacket();
        xSemaphoreGive(that->lock);
        // LOG("beacon");
      }

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
   while(!that->intercom->wifi->isConnected()) delay( 300 );
   
   MDNS.addService("_osc", "_udp", that->conf.port_in);
   mdns_service_instance_name_set("_osc", "_udp", ("OSC._"+that->intercom->system->name()).c_str());

   LOGF("OSC: listening on port %d\n", that->conf.port_in);

   char idpath[8];
   sprintf(idpath, "/e%u", that->intercom->system->id()); 

   char chpath[8];
   sprintf(chpath, "/c%u", that->intercom->system->channel());

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
              that->status().send(*that->udp);
              that->udp->endPacket();
            }
          });

          //
          // IDENTITY ROUTING
          //
          auto router = [](K32_osc* that, K32_oscmsg &msg, int offset){

            char* command;
            char path[32];
            msg.getAddress(path, 1);
            command = strchr(path, '/')+1;

            const int argCount = msg.size();
            argX* args[argCount];
            for(int k=0; k<argCount; k++) {
              if (msg.isInt(k)) args[k] = new argX((int) msg.getInt(k));
              else if (msg.isString(k)) args[k] = new argX(msg.getStr(k).c_str());
              else args[k] = new argX("");
            }
            
            that->intercom->dispatch(command, args, argCount);

          };

          // ROUTE ALL / DEVICE ID / CHANNEL GROUP
          msg.route("/all", router);
          msg.route(idpath, router);
          msg.route(chpath, router);

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
