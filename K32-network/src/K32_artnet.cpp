/*
  K32_artnet.cpp
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/

#include "K32_artnet.h"

/*
 *   PUBLIC
 */

K32_artnet::K32_artnet(K32* k32, K32_wifi* wifi, String name) : K32_plugin("artnet-strip", k32), wifi(wifi)
{
  this->artnet = new ArtnetWiFiReceiver();
  this->artnet->shortname(name);
  this->artnet->longname(name);
}

void K32_artnet::start()
{
  // LOOP client
  xTaskCreate(this->check,    // function
                          "artnet_check", // name
                          7000,           // stack memory
                          (void *)this,   // args
                          5,              // priority
                          &xHandle       // handler
                          );             // core

}

void K32_artnet::stop()
{
  if (xHandle != NULL) vTaskDelete(xHandle);
  xHandle = NULL;
}

void K32_artnet::onDmx( dmxsub subscription ) 
{
  LOGF("ARTNET: address = %i\n", subscription.address);
  for (int k=0; k< DMX_SUB_SLOTS; k++)
    if (K32_artnet::subscriptions[k].address == 0) {
      K32_artnet::subscriptions[k] = subscription;
      break;
    }
}

void K32_artnet::onFullDmx( cbPtrDmx callback ) 
{
  K32_artnet::fullCallback = callback;
}

void K32_artnet::command(Orderz* order) {
  // TODO: orderz ARTNET
}

dmxsub K32_artnet::subscriptions[DMX_SUB_SLOTS] = {0, 0, nullptr};
cbPtrDmx K32_artnet::fullCallback = nullptr;
int K32_artnet::_lastSequence = 0;

// /*
//  *   PRIVATE
//  */


void K32_artnet::check(void *parameter)
{
  K32_artnet *that = (K32_artnet *)parameter;
  TickType_t xFrequency = pdMS_TO_TICKS(2);

  while(!that->wifi->isConnected()) delay( 300 ); // TODO: use module events wifi/connected

  int uni = that->k32->system->universe();

  that->artnet->begin(0, uni/16);
  that->artnet->subscribe(uni-(uni/16)*16, that->_onArtnet);
  LOGF2("ARTNET: subscribing to subnet=%d universe=%d\n", uni/16, uni-(uni/16)*16);

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
  for (int k=0; k<DMX_SUB_SLOTS; k++) {
    if (K32_artnet::subscriptions[k].address > 0 && length-K32_artnet::subscriptions[k].address > 0) {
      K32_artnet::subscriptions[k].callback( 
        &data[ K32_artnet::subscriptions[k].address - 1 ], 
        min(K32_artnet::subscriptions[k].framesize, length-(K32_artnet::subscriptions[k].address-1)) 
      );
    }
  }

  // Callback Full
  if (K32_artnet::fullCallback && length > 0) K32_artnet::fullCallback(data, length);
}

