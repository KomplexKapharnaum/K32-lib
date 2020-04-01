/*
  K32_artnet.cpp
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/

#include "Arduino.h"
#include "K32_artnet.h"

/*
 *   PUBLIC
 */

K32_artnet::K32_artnet() {
  this->artnet = new ArtnetWifi();
}


void K32_artnet::start(artnetconf conf)
{
  this->conf = conf;
  this->artnet->begin();
  this->artnet->setArtDmxCallback( this->_onDmxFrame );

  // LOOP client
  xTaskCreatePinnedToCore(this->check,    // function
                          "artnet_check", // name
                          2000,           // stack memory
                          (void *)this,   // args
                          0,              // priority
                          NULL,           // handler
                          0);             // core

}


void K32_artnet::onDmx( cbPtr callback ) 
{
  this->frameCallback = callback;
}


artnetconf K32_artnet::conf = {0, 0, 0};    
K32_artnet::cbPtr K32_artnet::frameCallback = nullptr;
int K32_artnet::_lastSequence = 0;

// /*
//  *   PRIVATE
//  */


void K32_artnet::check(void *parameter)
{
  K32_artnet *that = (K32_artnet *)parameter;
  TickType_t xFrequency = pdMS_TO_TICKS(5);

  while (true)
  {
    while(that->artnet->read() > 0) {}
    vTaskDelay(xFrequency);
  }
  vTaskDelete(NULL);
}


void K32_artnet::_onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t *data) 
{
  if (universe != K32_artnet::conf.universe) return;
  // if (sequence <= this->_lastSequence) return;   // TODO: check if sequence is effective !

  K32_artnet::_lastSequence = sequence;
  
  if (K32_artnet::frameCallback)
    K32_artnet::frameCallback( 
      &data[ K32_artnet::conf.address-1 ], 
      min(K32_artnet::conf.framesize, length-(K32_artnet::conf.address-1)) 
    );
}

