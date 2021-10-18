/*
  K32_artnet.cpp
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/

#include "K32_artnet.h"

/*
 *   PUBLIC
 */

K32_artnet::K32_artnet(K32* k32, K32_wifi* wifi, String name, int universe) : K32_plugin("artnet-strip", k32), wifi(wifi)
{
  for (int k=0; k<ARTNET_SUB_SLOTS; k++)
    this->subscriptions[k] = {0, 0, nullptr};

  this->_universe = universe;
  this->artnet = new ArtnetWiFiReceiver();
  this->artnet->shortname(name);
  this->artnet->longname(name);
  this->start();
}


void K32_artnet::start()
{
  // LOOP client
  xTaskCreate(this->check,    // function
                          "artnet_check", // name
                          10000,           // stack memory
                          (void *)this,   // args
                          10,              // priority
                          &xHandle       // handler
                          );             // core

}

void K32_artnet::stop()
{
  if (xHandle != NULL) vTaskDelete(xHandle);
  xHandle = NULL;
}

void K32_artnet::onDmx( artnetsub subscription ) 
{
  for (int k=0; k< ARTNET_SUB_SLOTS; k++)
    if (this->subscriptions[k].address == 0) {
      this->subscriptions[k] = subscription;
      break;
    }
}

void K32_artnet::onFullDmx( cbPtrArtnet callback ) 
{
  this->fullCallback = callback;
}

void K32_artnet::command(Orderz* order) {
  // TODO: orderz ARTNET
}

artnetsub K32_artnet::subscriptions[ARTNET_SUB_SLOTS];
cbPtrArtnet K32_artnet::fullCallback = nullptr;
int K32_artnet::_lastSequence = 0;

// /*
//  *   PRIVATE
//  */


void K32_artnet::check(void *parameter)
{
  K32_artnet *that = (K32_artnet *)parameter;
  TickType_t xFrequency = pdMS_TO_TICKS(2);

  while(!that->wifi->isConnected()) delay( 300 ); // TODO: use module events wifi/connected

  that->artnet->begin(0, that->_universe/16);
  that->artnet->subscribe(that->_universe-(that->_universe/16)*16, that->_onArtnet);
  LOGF2("ARTNET: subscribing to subnet=%d universe=%d\n", that->_universe/16, that->_universe-(that->_universe/16)*16);

  that->emit("artnet/started");
  while (true)
  {
    // while(that->artnet->read() > 0) {}
    that->artnet->parse();
    vTaskDelay(xFrequency);
  }
  vTaskDelete(NULL);
}


void K32_artnet::_onArtnet(const uint8_t* data, const uint16_t length)
{
  // Callback Sub
  for (int k=0; k<ARTNET_SUB_SLOTS; k++)
    if (K32_artnet::subscriptions[k].address > 0 && length-K32_artnet::subscriptions[k].address > 0)
      K32_artnet::subscriptions[k].callback( 
        &data[ K32_artnet::subscriptions[k].address - 1 ], 
        min(K32_artnet::subscriptions[k].framesize, length-(K32_artnet::subscriptions[k].address-1)) 
      );

  // Callback Full
  if (K32_artnet::fullCallback && length > 0) K32_artnet::fullCallback(data, length);
}

