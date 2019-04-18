/*
  K32_osc.cpp
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/

#include "Arduino.h"
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

K32_osc::K32_osc(int port, K32* engine)
  : port(port), engine(engine)
{
  this->udp = new WiFiUDP();
  this->udp->begin(this->port);

  // LOOP task
  xTaskCreate( this->task,          // function
                "osc_task",         // task name
                10000,              // stack memory
                (void*)this,        // args
                4,                  // priority
                NULL);              // handler



};


/*
 *   PRIVATE
 */

 void K32_osc::task( void * parameter ) {
   K32_osc* that = (K32_osc*) parameter;
   TickType_t xFrequency = pdMS_TO_TICKS(1);
   int size = 0;
   K32_oscmsg msg(that);
   IPAddress remoteIP;

   char idpath[8];
   sprintf(idpath, "/e%u", that->engine->settings->get("id"));

   char chpath[8];
   sprintf(chpath, "/c%u", that->engine->settings->get("channel"));

   while(true) {
      size = that->udp->parsePacket();
      if (size > 0) {
        msg.empty();
        while (size--) msg.fill(that->udp->read());
        if (!msg.hasError()) {
          remoteIP = that->udp->remoteIP();

          //
          // GENERAL PING
          //
          msg.dispatch("/ping", [](K32_osc* that, K32_oscmsg &msg){
            LOGINL("OSC: /ping RECV from " );
            LOG(that->udp->remoteIP());

            OSCMessage response("/pong");
            that->udp->beginPacket(that->udp->remoteIP(), that->port);
            response.send(*that->udp);
            that->udp->endPacket();
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
            msg.dispatch("/shutdown", [](K32_osc* that, K32_oscmsg &msg){
              that->engine->stm32->shutdown();
            }, offset);

            //
            // AUDIO
            //
            msg.route("/audio", [](K32_osc* that, K32_oscmsg &msg, int offset){

              // PLAY
              msg.dispatch("/play", [](K32_osc* that, K32_oscmsg &msg){

                if (msg.isString(0))
                  that->engine->audio->play( msg.getStr(0) );

                if (msg.isInt(1)) {
                  that->engine->audio->volume( msg.getInt(1) );
                  if (msg.isBoolean(2))
                    that->engine->audio->loop( msg.getBoolean(2) );
                }
                else if (msg.isBoolean(1))
                  that->engine->audio->loop( msg.getBoolean(1) );

              }, offset);

              // PLAY MIDI
              msg.dispatch("/sampler", [](K32_osc* that, K32_oscmsg &msg){

                if (msg.isInt(0) && msg.isInt(1))
                  that->engine->audio->play( that->engine->sampler->path( msg.getInt(0), msg.getInt(1) ) );

                if (msg.isInt(2)) {
                  that->engine->audio->volume( msg.getInt(2) );
                  if (msg.isBoolean(3))
                    that->engine->audio->loop( msg.getBoolean(3) );
                }
                else if (msg.isBoolean(2))
                  that->engine->audio->loop( msg.getBoolean(2) );

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
                if (msg.isBoolean(0)) that->engine->audio->loop( msg.getBoolean(0) );
              }, offset);

            }, offset);

            //
            // LEDS
            //
            msg.route("/leds", [](K32_osc* that, K32_oscmsg &msg, int offset){

              // ALL LEDS
              msg.dispatch("/all", [](K32_osc* that, K32_oscmsg &msg){

                if (msg.isInt(0) && msg.isInt(1) && msg.isInt(2))
                  if (msg.isInt(3)) that->engine->leds->setAll( msg.getInt(0), msg.getInt(1), msg.getInt(2), msg.getInt(3) )->show();
                  else that->engine->leds->setAll( msg.getInt(0), msg.getInt(1), msg.getInt(2) )->show();

                else if (msg.isInt(0))
                  that->engine->leds->setAll( msg.getInt(0), msg.getInt(0), msg.getInt(0) )->show();

              }, offset);

              // STOP
              msg.dispatch("/stop", [](K32_osc* that, K32_oscmsg &msg){
                that->engine->leds->stop();
              }, offset);

              // SINUS
              msg.dispatch("/sinus", [](K32_osc* that, K32_oscmsg &msg){
                that->engine->leds->play( K32_leds_anims::sinus );
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
      }
      else vTaskDelay( xFrequency );
   }

   vTaskDelete(NULL);
 }
