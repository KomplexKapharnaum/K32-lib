/*
  K32_mqtt.cpp
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/

#include "Arduino.h"
#include "K32_mqtt.h"

WiFiClient espClient;
void callback(char* topic, byte* payload, unsigned int length);

/*
 *   PUBLIC
 */

K32_mqtt::K32_mqtt(mqttconf conf, K32* engine) : conf(conf), engine(engine)
{ 
  this->lock = xSemaphoreCreateMutex();

  this->mqttc = new PubSubClient(espClient);
  this->mqttc->setServer(this->conf.broker, 1883);
  this->mqttc->setCallback(callback);

  if (this->mqttc->connected()) LOG("MQTT: init connected");
  else LOG("MQTT: init notconnected");

  // LOOP client
  xTaskCreate( this->loop,          // function
                "mqtt_client",      // name
                10000,               // stack memory
                (void*)this,        // args
                5,                  // priority
                NULL);              // handler

//   // OSC OUTPUT
//   if (this->conf.port_out > 0) {
    
//     // LOOP beat
//     if (this->conf.beatInterval > 0)
//       xTaskCreate( this->beat,          // function
//                   "osc_beat",         // server name
//                   2000,              // stack memory
//                   (void*)this,        // args
//                   1,                  // priority
//                   NULL);              // handler

//     // LOOP beacon
//     if (this->conf.beaconInterval > 0)
//       xTaskCreate( this->beacon,          // function
//                   "osc_beacon",         // server name
//                   2000,              // stack memory
//                   (void*)this,        // args
//                   2,                  // priority
//                   NULL);              // handler
//   }

  
};

void K32_mqtt::reconnect() {
  String myChan = "c3"; 
  
  // Loop until we're reconnected
  while (!this->mqttc->connected() && this->engine->wifi->isOK()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (this->mqttc->connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      this->mqttc->publish("esp", "hello world");
      // ... and resubscribe
      this->mqttc->subscribe( ("midi/"+myChan).c_str(), 1);
      this->mqttc->subscribe( "midi/sys", 1);
      this->mqttc->subscribe( "midi/all", 1);
      this->mqttc->subscribe( ("osc/"+myChan+"/#").c_str(), 1);
      this->mqttc->subscribe( "osc/all", 1);

    } else {
      Serial.print("failed, rc=");
      Serial.print(this->mqttc->state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


// OSCMessage K32_mqtt::status() {

//     OSCMessage msg("/status");

//     // identity
//     msg.add(this->engine->settings->get("id"));
//     msg.add(this->engine->settings->get("channel"));
//     msg.add(K32_VERSION);

//     // wifi 
//     byte mac[6];
//     WiFi.macAddress(mac);
//     char shortmac[16];
//     sprintf(shortmac, "%02X:%02X:%02X", mac[3], mac[4], mac[5]);
//     msg.add(shortmac);
//     msg.add(WiFi.localIP().toString().c_str());
//     msg.add(WiFi.RSSI());
//     (this->linkedIP) ? msg.add(true) : msg.add(false);
    
//     // energy 
//     msg.add(this->engine->stm32->battery());

//     // audio 
//     if (this->engine->audio) {
//       msg.add(this->engine->audio->isSdOK());
//       (this->engine->audio->media() != "") ? msg.add(this->engine->audio->media().c_str()) : msg.add("stop");
//       msg.add(this->engine->audio->error().c_str());
//     }
//     else {
//       msg.add(false);   /// TODO : SD check without audio engine
//       msg.add("stop");
//       msg.add("");
//     }

//     // filesync 
//     // msg.add(sync_size());
//     // msg.add(sync_getStatus().c_str());
    
//     return msg;
// }

void K32_mqtt::send( OSCMessage msg ) {
  if (this->mqttc->connected()) { 
    xSemaphoreTake(this->lock, portMAX_DELAY);
    char path[256];
    msg.getAddress(path);
    this->mqttc->publish(("esp"+String(path)).c_str(), "hello world");
    xSemaphoreGive(this->lock);
  }
}


// /*
//  *   PRIVATE
//  */

void K32_mqtt::loop(void * parameter) {
    K32_mqtt* that = (K32_mqtt*) parameter;
    TickType_t xFrequency = pdMS_TO_TICKS(10);

    while(true) { 
      that->reconnect();
      xSemaphoreTake(that->lock, portMAX_DELAY);
      that->mqttc->loop();
      xSemaphoreGive(that->lock);
      vTaskDelay( xFrequency );
    }
    vTaskDelete(NULL);
}

void splitString(char* data, char* separator, int index, char* result)
{   
    char input[strlen(data)];
    strcpy(input, data);

    char* command = strtok(input, separator);
    for (int k=0; k<index; k++)
      if (command != NULL) command = strtok(NULL, separator);
    
    if (command == NULL) strcpy(result, "");
    else strcpy(result, command);
}

void callback(char* topic, byte* payload, unsigned int length) {
  
  payload[length] = 0;

  // ENGINE
  char engine[16];
  splitString(topic, "/", 0, engine);

  // OSC
  if (strcmp(engine, "osc") == 0) {
    Serial.print("OSC : ");
    Serial.print(topic);
    Serial.print(" ");
       
    char val[128];
    byte inc =0;
    splitString((char*)payload, "§", inc, val);
    while (strcmp(val, "") != 0) {
      Serial.print(val);
      Serial.print(" ");
      ++inc;
      splitString((char*)payload, "§", inc, val);
    }
    Serial.println("");

  }

  // MIDI
  if (strcmp(engine, "midi") == 0) {
    Serial.print("MIDI: ");
    Serial.print(topic);
    Serial.print(" ");

    char val[16];
    splitString((char*)payload, "-", 0, val);
    Serial.print(atoi(val));
    Serial.print(" ");
    splitString((char*)payload, "-", 1, val);
    Serial.print(atoi(val));
    Serial.print(" ");
    splitString((char*)payload, "-", 2, val);
    Serial.print(atoi(val));
    Serial.print(" ");
    Serial.println("");

  }

}



// void K32_mqtt::beat( void * parameter ) {
//     K32_mqtt* that = (K32_mqtt*) parameter;
//     TickType_t xFrequency = pdMS_TO_TICKS(that->conf.beatInterval);
//     WiFiUDP sock;

//     while(true) 
//     { 
//       that->send( OSCMessage("/beat") );
//       vTaskDelay( xFrequency );
//     }

//     vTaskDelete(NULL);
// }



// void K32_mqtt::beacon( void * parameter ) {

//     K32_mqtt* that = (K32_mqtt*) parameter;
//     TickType_t xFrequency = pdMS_TO_TICKS(that->conf.beaconInterval);
//     WiFiUDP sock;

//     while(true) 
//     {
//       if (that->engine->wifi->isOK()) { 
//         // send
//         xSemaphoreTake(that->lock, portMAX_DELAY);
//         IPAddress dest = (that->linkedIP) ? that->linkedIP : that->engine->wifi->broadcastIP();
//         sock.beginPacket( dest, that->conf.port_out);
//         that->status().send(sock);
//         sock.endPacket();
//         xSemaphoreGive(that->lock);
//         // LOG("beacon");
//       }

//       vTaskDelay( xFrequency );
//     }

//     vTaskDelete(NULL);
// }



// void K32_mqtt::server( void * parameter ) {
//    K32_mqtt* that = (K32_mqtt*) parameter;
//    TickType_t xFrequency = pdMS_TO_TICKS(1);
//    int size = 0;
//    K32_mqttmsg msg(that);
//    IPAddress remoteIP;

//    LOGF("OSC: listening on port %d\n", that->conf.port_in);

//    char idpath[8];
//    sprintf(idpath, "/e%u", that->engine->settings->get("id"));

//    char chpath[8];
//    sprintf(chpath, "/c%u", that->engine->settings->get("channel"));

//    while(true) {
     
//       xSemaphoreTake(that->lock, portMAX_DELAY);

//       size = that->udp->parsePacket();
//       if (size > 0) {
//         msg.empty();
//         while (size--) msg.fill(that->udp->read());
//         if (!msg.hasError()) {
//           remoteIP = that->udp->remoteIP();

//           char adr[256];
//           msg.getAddress(adr);
//           LOGINL("OSC: rcv  ");
//           LOG(adr);

//           //
//           // GENERAL PING
//           //
//           msg.dispatch("/ping", [](K32_mqtt* that, K32_mqttmsg &msg){
//             LOGINL("OSC: /ping RECV from " );
//             LOG(that->udp->remoteIP());

//             that->linkedIP = that->udp->remoteIP();

//             if (that->conf.port_out > 0) {
//               OSCMessage response("/pong");
//               that->udp->beginPacket(that->udp->remoteIP(), that->conf.port_out);
//               response.send(*that->udp);
//               that->udp->endPacket();
//             }
//           });

//           //
//           // GENERAL INFO
//           //
//           msg.dispatch("/info", [](K32_mqtt* that, K32_mqttmsg &msg){
//             LOGINL("OSC: /info RECV from " );
//             LOG(that->udp->remoteIP());

//             if (that->conf.port_out > 0) {
//               that->udp->beginPacket(that->udp->remoteIP(), that->conf.port_out);
//               that->status().send(*that->udp);
//               that->udp->endPacket();
//             }
//           });

//           //
//           // IDENTITY ROUTING
//           //
//           auto router = [](K32_mqtt* that, K32_mqttmsg &msg, int offset){

//             //
//             // RESET
//             //
//             msg.dispatch("/reset", [](K32_mqtt* that, K32_mqttmsg &msg){

//               that->engine->stm32->reset();
//             }, offset);

//             //
//             // SHUTDOWN
//             //
//             msg.dispatch("/poweroff", [](K32_mqtt* that, K32_mqttmsg &msg){
//               that->engine->stm32->shutdown();
//             }, offset);

//             //
//             // AUDIO
//             //
//             if (that->engine->audio)
//             msg.route("/audio", [](K32_mqtt* that, K32_mqttmsg &msg, int offset){

//               // PLAY
//               msg.dispatch("/play", [](K32_mqtt* that, K32_mqttmsg &msg){

//                 if (!msg.isString(0)) return;
//                 that->engine->audio->play( msg.getStr(0) );

//                 if (msg.isInt(1)) {
//                   that->engine->audio->volume( msg.getInt(1) );
//                   if (msg.isInt(2))
//                     that->engine->audio->loop( msg.getInt(2) > 0 );
//                 }

//               }, offset);

//               // PLAY MIDI
//               if (that->engine->sampler)
//               msg.dispatch("/sample", [](K32_mqtt* that, K32_mqttmsg &msg){

//                 if (msg.isInt(0) && msg.isInt(1))
//                   that->engine->audio->play( that->engine->sampler->path( msg.getInt(0), msg.getInt(1) ) );

//                 if (msg.isInt(2)) {
//                   that->engine->audio->volume( msg.getInt(2) );
//                   if (msg.isInt(3))
//                     that->engine->audio->loop( msg.getInt(3) > 0 );
//                 }

//               }, offset);

//               // STOP
//               msg.dispatch("/stop", [](K32_mqtt* that, K32_mqttmsg &msg){
//                 that->engine->audio->stop();
//               }, offset);

//               // VOLUME
//               msg.dispatch("/volume", [](K32_mqtt* that, K32_mqttmsg &msg){
//                 if (msg.isInt(0)) that->engine->audio->volume( msg.getInt(0) );
//               }, offset);

//               // LOOP
//               msg.dispatch("/loop", [](K32_mqtt* that, K32_mqttmsg &msg){
//                 if (msg.isInt(0)) that->engine->audio->loop( msg.getInt(0) > 0 );
//               }, offset);

//             }, offset);

//             //
//             // LEDS
//             //
//             if (that->engine->leds)
//             msg.route("/leds", [](K32_mqtt* that, K32_mqttmsg &msg, int offset){

//               // SET ALL
//               msg.dispatch("/all", [](K32_mqtt* that, K32_mqttmsg &msg){

//                 if (!msg.isInt(0)) return;
//                 int red, green, blue, white;
                
//                 red = msg.getInt(0);
//                 if (msg.isInt(1) && msg.isInt(2)) {
//                   green = msg.getInt(1);
//                   blue = msg.getInt(2);
//                   white = msg.isInt(3) ? msg.getInt(3) : 0;
//                 }
//                 else { green = red; blue = red; white = red; }

//                 that->engine->leds->leds()->setAll( red, green, blue, white )->show();

//               }, offset);

//               // SET STRIP
//               msg.dispatch("/strip", [](K32_mqtt* that, K32_mqttmsg &msg){

//                 if (!msg.isInt(0) || !msg.isInt(1)) return;
//                 int strip, red, green, blue, white;
                
//                 strip = msg.getInt(0);
//                 red = msg.getInt(1);
//                 if (msg.isInt(2) && msg.isInt(3)) {
//                   green = msg.getInt(2);
//                   blue = msg.getInt(3);
//                   white = msg.isInt(4) ? msg.getInt(4) : 0;
//                 }
//                 else { green = red; blue = red; white = red; }

//                 that->engine->leds->leds()->setStrip( strip, red, green, blue, white )->show();

//               }, offset);

//               // SET PIXEL
//               msg.dispatch("/pixel", [](K32_mqtt* that, K32_mqttmsg &msg){
                
//                 if (!msg.isInt(0) || !msg.isInt(1) || !msg.isInt(2)) return;
//                 int strip, pixel, red, green, blue, white;
                
//                 strip = msg.getInt(0);
//                 pixel = msg.getInt(1);
//                 red = msg.getInt(2);
//                 if (msg.isInt(3) && msg.isInt(4)) {
//                   green = msg.getInt(3);
//                   blue = msg.getInt(4);
//                   white = msg.isInt(5) ? msg.getInt(5) : 0;
//                 }
//                 else { green = red; blue = red; white = red; }

//                 that->engine->leds->leds()->setPixel( strip, pixel, red, green, blue, white )->show();

//               }, offset);

//               // BLACKOUT
//               msg.dispatch("/blackout", [](K32_mqtt* that, K32_mqttmsg &msg){
//                 that->engine->leds->stop();
//               }, offset);

//               // STOP
//               msg.dispatch("/stop", [](K32_mqtt* that, K32_mqttmsg &msg){
//                 that->engine->leds->stop();
//               }, offset);

//               // ANIMATION
//               msg.dispatch("/play", [](K32_mqtt* that, K32_mqttmsg &msg){
                
//                 if (!msg.isString(0)) return;
//                 K32_leds_anim* anim = that->engine->leds->anim( msg.getStr(0) );

//                 for (int k=0; k<LEDS_PARAM_SLOTS; k++)
//                   if (msg.isInt(k+1)) anim->setParam(k, msg.getInt(k+1));

//                 that->engine->leds->play( anim );

//               }, offset);


//             }, offset);

//           };

//           // ROUTE ALL / DEVICE ID / CHANNEL GROUP
//           msg.route("/all", router);
//           msg.route(idpath, router);
//           msg.route(chpath, router);

//         } else {
//           LOGINL("OSC: error ");
//           LOG(msg.getError());
//         }


//         xSemaphoreGive(that->lock);
//       }
//       else {
//         xSemaphoreGive(that->lock);
//         vTaskDelay( xFrequency );
//       }
      
//    }

//    vTaskDelete(NULL);
// }
