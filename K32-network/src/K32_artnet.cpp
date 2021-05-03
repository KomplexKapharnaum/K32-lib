/*
  K32_artnet.cpp
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/

#include "K32_artnet.h"

/*
 *   PUBLIC
 */

K32_artnet::K32_artnet(K32* k32, artnetconf conf) : K32_plugin("artnet", k32)
{
  this->conf = conf;
  this->artnet = new ArtnetWiFiReceiver();
  this->start();
}


void K32_artnet::start()
{
  // LOOP client
  xTaskCreate(this->check,    // function
                          "artnet_check", // name
                          10000,           // stack memory
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

void K32_artnet::onDmx( cbPtr callback ) 
{
  this->frameCallback = callback;
}

void K32_artnet::onFullDmx( cbPtr callback ) 
{
  this->fullCallback = callback;
}


artnetconf K32_artnet::conf = {0, 0, 0};    
K32_artnet::cbPtr K32_artnet::frameCallback = nullptr;
K32_artnet::cbPtr K32_artnet::fullCallback = nullptr;
int K32_artnet::_lastSequence = 0;

// /*
//  *   PRIVATE
//  */


void K32_artnet::check(void *parameter)
{
  K32_artnet *that = (K32_artnet *)parameter;
  TickType_t xFrequency = pdMS_TO_TICKS(2);

  that->artnet->begin();
  that->artnet->subscribe(conf.universe, that->_onArtnet);

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
  // Callback Frame
  if (K32_artnet::frameCallback) 
  {
    K32_artnet::frameCallback( 
      &data[ K32_artnet::conf.address-1], 
      min(K32_artnet::conf.framesize, length-(K32_artnet::conf.address-1)) 
    );
  }

  // Callback Full
  if (K32_artnet::fullCallback) K32_artnet::fullCallback(data, length);
}

