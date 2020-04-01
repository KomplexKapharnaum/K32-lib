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

K32_osc::K32_osc(K32_system *system, K32_wifi *wifi, K32_audio *audio, K32_light *light) 
                                : system(system), wifi(wifi), audio(audio), light(light) {}


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
    msg.add(system->id());
    msg.add(system->channel());
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
    if (system->stm32) msg.add(system->stm32->battery());
    else msg.add(0);

    // audio 
    if (audio) {
      msg.add(audio->isSdOK());
      (audio->media() != "") ? msg.add(audio->media().c_str()) : msg.add("stop");
      msg.add(audio->error().c_str());
    }
    else {
      msg.add(false);   /// TODO : SD check without audio engine
      msg.add("stop");
      msg.add("");
    }

    // sampler
    if (audio && audio->sampler) msg.add(audio->sampler->bank());
    else msg.add(0);

    // filesync 
    // msg.add(sync_size());
    // msg.add(sync_getStatus().c_str());
    msg.add(0);   // SYNC count files
    msg.add("");  // SYNC erro
    
    return msg;
}

void K32_osc::send( OSCMessage msg ) {
  if (wifi->isConnected() && this->conf.port_out > 0) { 
    xSemaphoreTake(this->lock, portMAX_DELAY);
    IPAddress dest = (this->linkedIP) ? this->linkedIP : wifi->broadcastIP();
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
      if (that->wifi->isConnected()) { 
        // send
        xSemaphoreTake(that->lock, portMAX_DELAY);
        IPAddress dest = (that->linkedIP) ? that->linkedIP : that->wifi->broadcastIP();
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
   while(!that->wifi->isConnected()) delay( 300 );
   
   MDNS.addService("_osc", "_udp", that->conf.port_in);
   mdns_service_instance_name_set("_osc", "_udp", ("OSC._"+that->system->name()).c_str());

   LOGF("OSC: listening on port %d\n", that->conf.port_in);

   char idpath[8];
   sprintf(idpath, "/%u", that->system->id()); 

   char chpath[8];
   sprintf(chpath, "/c%u", that->system->channel());

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
              that->system->reset();
            }, offset);

            //
            // SHUTDOWN
            //
            msg.dispatch("/shutdown", [](K32_osc* that, K32_oscmsg &msg){
              that->system->shutdown();
            }, offset);

            //
            // CHANNEL
            //
            msg.dispatch("/channel", [](K32_osc* that, K32_oscmsg &msg){
              if (msg.isInt(0)) {
                that->system->channel(msg.getInt(0));
                delay(100);
                that->system->reset();
              }
            }, offset);

            //
            // AUDIO
            //
            if (that->audio)
            msg.route("/audio", [](K32_osc* that, K32_oscmsg &msg, int offset){

              // PLAY
              msg.dispatch("/play", [](K32_osc* that, K32_oscmsg &msg){

                if (!msg.isString(0)) return;
                that->audio->play( msg.getStr(0) );

                if (msg.isInt(1)) {
                  that->audio->volume( msg.getInt(1) );
                  if (msg.isInt(2))
                    that->audio->loop( msg.getInt(2) > 0 );
                }

              }, offset);

              // SAMPLER NOTEON
              if (that->audio->sampler)
              msg.dispatch("/noteon", [](K32_osc* that, K32_oscmsg &msg){

                if (msg.isInt(0) && msg.isInt(1)) {
                  that->audio->sampler->bank( msg.getInt(0) );
                  that->audio->play( that->audio->sampler->path( msg.getInt(1) ) );
                  LOGINL("OSC Sample: "); LOGINL(msg.getInt(0)); LOGINL(msg.getInt(1)); 
                }

                if (msg.isInt(2)) {
                  that->audio->volume( msg.getInt(2) );
                  if (msg.isInt(3))
                    that->audio->loop( msg.getInt(3) > 0 );
                }

              }, offset);

              // LEGACY -> CHANGE IN MAX REGIE !!!!!!
              // SAMPLER NOTEON
              if (that->audio->sampler)
              msg.dispatch("/sample", [](K32_osc* that, K32_oscmsg &msg){

                if (msg.isInt(0) && msg.isInt(1)) {
                  that->audio->sampler->bank( msg.getInt(0) );
                  that->audio->play( that->audio->sampler->path( msg.getInt(1) ) );
                  LOGINL("OSC Sample: "); LOGINL(msg.getInt(0)); LOGINL(msg.getInt(1)); 
                }

                if (msg.isInt(2)) {
                  that->audio->volume( msg.getInt(2) );
                  if (msg.isInt(3))
                    that->audio->loop( msg.getInt(3) > 0 );
                }

              }, offset);


              // SAMPLER NOTEOFF
              if (that->audio->sampler)
              msg.dispatch("/noteoff", [](K32_osc* that, K32_oscmsg &msg){
                
                if (msg.isInt(0)) 
                  if (that->audio->media() == that->audio->sampler->path( msg.getInt(0) ))
                    that->audio->stop();

              }, offset);

              // STOP
              msg.dispatch("/stop", [](K32_osc* that, K32_oscmsg &msg){
                that->audio->stop();
              }, offset);

              // VOLUME
              msg.dispatch("/volume", [](K32_osc* that, K32_oscmsg &msg){
                if (msg.isInt(0)) that->audio->volume( msg.getInt(0) );
              }, offset);

              // LOOP
              msg.dispatch("/loop", [](K32_osc* that, K32_oscmsg &msg){
                if (msg.isInt(0)) that->audio->loop( msg.getInt(0) > 0 );
                else LOG("invalid arg");
              }, offset);

            }, offset);

            //
            // LEDS
            //
            if (that->light)
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

                that->light->strips()->all( red, green, blue, white );
                that->light->show();

              }, offset);

              // SET STRIP
              msg.dispatch("/strip", [](K32_osc* that, K32_oscmsg &msg){

                if (!msg.isInt(0) || !msg.isInt(1)) return;
                int s, red, green, blue, white;
                
                s = msg.getInt(0);
                red = msg.getInt(1);
                if (msg.isInt(2) && msg.isInt(3)) {
                  green = msg.getInt(2);
                  blue = msg.getInt(3);
                  white = msg.isInt(4) ? msg.getInt(4) : 0;
                }
                else { green = red; blue = red; white = red; }

                that->light->strip(s)->all( red, green, blue, white );
                that->light->show();

              }, offset);

              // SET PIXEL
              msg.dispatch("/pixel", [](K32_osc* that, K32_oscmsg &msg){
                
                if (!msg.isInt(0) || !msg.isInt(1) || !msg.isInt(2)) return;
                int s, pixel, red, green, blue, white;
                
                s = msg.getInt(0);
                pixel = msg.getInt(1);
                red = msg.getInt(2);
                if (msg.isInt(3) && msg.isInt(4)) {
                  green = msg.getInt(3);
                  blue = msg.getInt(4);
                  white = msg.isInt(5) ? msg.getInt(5) : 0;
                }
                else { green = red; blue = red; white = red; }

                that->light->strip(s)->pix( pixel, red, green, blue, white );
                that->light->show();

              }, offset);

              // BLACKOUT
              msg.dispatch("/blackout", [](K32_osc* that, K32_oscmsg &msg){
                that->light->stop();
              }, offset);

              // STOP
              msg.dispatch("/stop", [](K32_osc* that, K32_oscmsg &msg){
                that->light->stop();
              }, offset);

              // ANIMATION
              msg.dispatch("/play", [](K32_osc* that, K32_oscmsg &msg){
                
                if (!msg.isString(0)) return;
                K32_anim* anim = that->light->anim( msg.getStr(0) );
                LOGINL("LEDS: play "); LOGINL(msg.getStr(0));

                for (int k=0; k<LEDS_DATA_SLOTS; k++) 
                  if (msg.isInt(k+1)) {
                    anim->setdata(k, msg.getInt(k+1));
                    LOGINL(" "); LOGINL(msg.getInt(k+1));
                  }
                LOG("");

                that->light->play(anim);

              }, offset);


            }, offset);

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
