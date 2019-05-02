/*
  K32_osc.cpp
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/

#include "Arduino.h"
#include "K32_osc.h"
#include "K32_leds_rmt.h"

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

K32_osc::K32_osc(oscconf conf, K32* engine) : conf(conf), engine(engine)
{ 
  this->lock = xSemaphoreCreateMutex();
  this->udp = new WiFiUDP();

  // OSC INPUT
  if (this->conf.port_in > 0) {
    this->udp->begin(this->conf.port_in);

    // LOOP server
    xTaskCreate( this->server,          // function
                  "osc_server",         // server name
                  10000,              // stack memory
                  (void*)this,        // args
                  5,                  // priority
                  NULL);              // handler
  }

  // OSC OUTPUT
  if (this->conf.port_out > 0) {
    
    // LOOP beat
    if (this->conf.beatInterval > 0)
      xTaskCreate( this->beat,          // function
                  "osc_beat",         // server name
                  5000,              // stack memory
                  (void*)this,        // args
                  1,                  // priority
                  NULL);              // handler

    // LOOP beacon
    if (this->conf.beaconInterval > 0)
      xTaskCreate( this->beacon,          // function
                  "osc_beacon",         // server name
                  5000,              // stack memory
                  (void*)this,        // args
                  1,                  // priority
                  NULL);              // handler
  }

  
};


OSCMessage K32_osc::status() {

    OSCMessage msg("/status");

    // identity
    msg.add(this->engine->settings->get("id"));
    msg.add(this->engine->settings->get("channel"));
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
    msg.add(this->engine->stm32->battery());

    // audio 
    if (this->engine->audio) {
      msg.add(this->engine->audio->isSdOK());
      (this->engine->audio->media() != "") ? msg.add(this->engine->audio->media().c_str()) : msg.add("stop");
      msg.add(this->engine->audio->error().c_str());
    }
    else {
      msg.add(false);   /// TODO : SD check without audio engine
      msg.add("stop");
      msg.add("");
    }

    // filesync 
    // msg.add(sync_size());
    // msg.add(sync_getStatus().c_str());
    
    return msg;
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
      OSCMessage msg("/beat");

      // send
      xSemaphoreTake(that->lock, portMAX_DELAY);
      IPAddress dest = (that->linkedIP) ? that->linkedIP : that->engine->wifi->broadcastIP();
      sock.beginPacket( dest, that->conf.port_out);
      msg.send(sock);
      sock.endPacket();
      xSemaphoreGive(that->lock);
      // LOG("beat");

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
      // send
      xSemaphoreTake(that->lock, portMAX_DELAY);
      IPAddress dest = (that->linkedIP) ? that->linkedIP : that->engine->wifi->broadcastIP();
      sock.beginPacket( dest, that->conf.port_out);
      that->status().send(sock);
      sock.endPacket();
      xSemaphoreGive(that->lock);
      // LOG("beacon");

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

   LOGF("OSC: listening on port %d\n", that->conf.port_in);

   char idpath[8];
   sprintf(idpath, "/e%u", that->engine->settings->get("id"));

   char chpath[8];
   sprintf(chpath, "/c%u", that->engine->settings->get("channel"));

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

            //
            // RESET
            //
            msg.dispatch("/reset", [](K32_osc* that, K32_oscmsg &msg){
              that->engine->stm32->reset();
            }, offset);

            //
            // SHUTDOWN
            //
            msg.dispatch("/poweroff", [](K32_osc* that, K32_oscmsg &msg){
              that->engine->stm32->shutdown();
            }, offset);

            //
            // AUDIO
            //
            if (that->engine->audio)
            msg.route("/audio", [](K32_osc* that, K32_oscmsg &msg, int offset){

              // PLAY
              msg.dispatch("/play", [](K32_osc* that, K32_oscmsg &msg){

                if (!msg.isString(0)) return;
                that->engine->audio->play( msg.getStr(0) );

                if (msg.isInt(1)) {
                  that->engine->audio->volume( msg.getInt(1) );
                  if (msg.isInt(2))
                    that->engine->audio->loop( msg.getInt(2) > 0 );
                }

              }, offset);

              // PLAY MIDI
              if (that->engine->sampler)
              msg.dispatch("/sample", [](K32_osc* that, K32_oscmsg &msg){

                if (msg.isInt(0) && msg.isInt(1))
                  that->engine->audio->play( that->engine->sampler->path( msg.getInt(0), msg.getInt(1) ) );

                if (msg.isInt(2)) {
                  that->engine->audio->volume( msg.getInt(2) );
                  if (msg.isInt(3))
                    that->engine->audio->loop( msg.getInt(3) > 0 );
                }

              }, offset);

              // STOP
              msg.dispatch("/stop", [](K32_osc* that, K32_oscmsg &msg){
                that->engine->audio->stop();
              }, offset);

              // VOLUME
              msg.dispatch("/volume", [](K32_osc* that, K32_oscmsg &msg){
                if (msg.isInt(0)) that->engine->audio->volume( msg.getInt(0) );
              }, offset);

              // LOOP
              msg.dispatch("/loop", [](K32_osc* that, K32_oscmsg &msg){
                if (msg.isInt(0)) that->engine->audio->loop( msg.getInt(0) > 0 );
              }, offset);

            }, offset);

            //
            // LEDS
            //
            if (that->engine->leds)
            msg.route("/leds", [](K32_osc* that, K32_oscmsg &msg, int offset){

              // SET ALL
              msg.dispatch("/all", [](K32_osc* that, K32_oscmsg &msg){

                if (!msg.isInt(0)) return;
                int red, green, blue, white;
                
                red = msg.getInt(0);
                if (msg.isInt(1) && msg.isInt(2)) {
                  green = msg.getInt(1);
                  blue = msg.getInt(2);
                  white = msg.isInt(3) ? msg.getInt(3) : 0;
                }
                else { green = red; blue = red; white = red; }

                that->engine->leds->leds()->setAll( red, green, blue, white )->show();

              }, offset);

              // SET STRIP
              msg.dispatch("/strip", [](K32_osc* that, K32_oscmsg &msg){

                if (!msg.isInt(0) || !msg.isInt(1)) return;
                int strip, red, green, blue, white;
                
                strip = msg.getInt(0);
                red = msg.getInt(1);
                if (msg.isInt(2) && msg.isInt(3)) {
                  green = msg.getInt(2);
                  blue = msg.getInt(3);
                  white = msg.isInt(4) ? msg.getInt(4) : 0;
                }
                else { green = red; blue = red; white = red; }

                that->engine->leds->leds()->setStrip( strip, red, green, blue, white )->show();

              }, offset);

              // SET PIXEL
              msg.dispatch("/pixel", [](K32_osc* that, K32_oscmsg &msg){
                
                if (!msg.isInt(0) || !msg.isInt(1) || !msg.isInt(2)) return;
                int strip, pixel, red, green, blue, white;
                
                strip = msg.getInt(0);
                pixel = msg.getInt(1);
                red = msg.getInt(2);
                if (msg.isInt(3) && msg.isInt(4)) {
                  green = msg.getInt(3);
                  blue = msg.getInt(4);
                  white = msg.isInt(5) ? msg.getInt(5) : 0;
                }
                else { green = red; blue = red; white = red; }

                that->engine->leds->leds()->setPixel( strip, pixel, red, green, blue, white )->show();

              }, offset);

              // BLACKOUT
              msg.dispatch("/blackout", [](K32_osc* that, K32_oscmsg &msg){
                that->engine->leds->stop();
              }, offset);

              // STOP
              msg.dispatch("/stop", [](K32_osc* that, K32_oscmsg &msg){
                that->engine->leds->stop();
              }, offset);

              // ANIMATION
              msg.dispatch("/play", [](K32_osc* that, K32_oscmsg &msg){
                
                if (!msg.isString(0)) return;
                K32_leds_anim* anim = that->engine->leds->anim( msg.getStr(0) );

                for (int k=0; k<LEDS_PARAM_SLOTS; k++)
                  if (msg.isInt(k+1)) anim->setParam(k, msg.getInt(k+1));

                that->engine->leds->play( anim );

              }, offset);


            }, offset);

          };

          // ROUTE ALL / DEVICE ID / CHANNEL GROUP
          msg.route("/all", router);
          msg.route(idpath, router);
          msg.route(chpath, router);

        } else {
          OSCErrorCode error = msg.getError();
          LOGINL("OSC: error ");
          LOG(error);
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
