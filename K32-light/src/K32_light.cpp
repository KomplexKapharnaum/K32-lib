/*
  K32_light.cpp
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/

#include "K32_light.h"


K32_light::K32_light(K32* k32) : K32_plugin("leds", k32)
{
  digitalLeds_init();

  xTaskCreate( this->refresh,           // function
                  "modulate_task",            // task name
                  10000,                   // stack memory
                  (void*)this,            // args
                  4,                      // priority
                  NULL );                 // handler

  pwm = new K32_pwm(k32);
}

void K32_light::addStrip(const int pin, led_types type, int size)
{
  if (size == 0) size = LEDSTRIP_MAXPIXEL;

  if (this->_nstrips >= LEDS_MAXSTRIPS) {
    LOG("LEDS: no more strip can be attached..");
    return;
  }

  int s = this->_nstrips;
  this->_nstrips += 1;
  
  this->_strips[s] = new K32_ledstrip(s, pin, type, size);
}

// link every strip to masterStrip
void K32_light::cloneStrips(int masterStrip) {
  if (masterStrip < this->_nstrips) this->_masterClone = masterStrip;
}

// link every strip to masterStrip
void K32_light::copyStrip(stripcopy copy) {
  if (_copyMax<LEDS_MAX_COPY) 
  {
    _copylist[_copyMax] = copy;
    _copyMax += 1;
  }
}

K32_ledstrip* K32_light::strip(int s) {
  return this->_strips[s];
}

K32_light* K32_light::strips() {
  return this;
}

void K32_light::addDMX(const int DMX_PIN[3], DmxDirection dir) {
  dmx = new K32_dmx(DMX_PIN, dir);
}

void K32_light::addDMX(K32_dmx* d) {
  dmx = d;
}

K32_light* K32_light::black()
{
  for (int s = 0; s < this->_nstrips; s++) this->_strips[s]->black();
  return this;
}

K32_light* K32_light::all(pixelColor_t color)
{
  for (int s = 0; s < this->_nstrips; s++) this->_strips[s]->all(color);
  return this;
}

K32_light* K32_light::all(int red, int green, int blue, int white)
{
  return this->all( pixelFromRGBW(red, green, blue, white) );
}

K32_light* K32_light::pix(int pixel, pixelColor_t color) {
  for (int s = 0; s < this->_nstrips; s++) this->_strips[s]->pix(pixel, color);
  return this;
}

K32_light* K32_light::pix(int pixel, int red, int green, int blue, int white) {
  return this->pix( pixel, pixelFromRGBW(red, green, blue, white) );
}

void K32_light::show() {

  // Clone from master strip (if _masterClone exist)
  pixelColor_t* cloneBuffer = NULL;
  int cloneSize = 0;
  if (this->_masterClone >= 0) {
    cloneSize = this->_strips[this->_masterClone]->size();
    cloneBuffer = static_cast<pixelColor_t*>(malloc(cloneSize * sizeof(pixelColor_t)));
    this->_strips[this->_masterClone]->getBuffer(cloneBuffer, cloneSize);
  }

  for (int s=0; s<this->_nstrips; s++)  
  {
    // Clone from master strip
    if (cloneSize > 0) this->_strips[s]->setBuffer(cloneBuffer, cloneSize); 

    // Copy pixels
    for (int c=0; c<_copyMax; c++) {
      if (_copylist[c].destStrip == s) {
        pixelColor_t* copyBuffer = NULL;
        int copySize = 0;
        copySize = _copylist[c].srcStop-_copylist[c].srcStart+1;
        copyBuffer = static_cast<pixelColor_t*>(malloc(copySize * sizeof(pixelColor_t)));
        this->_strips[_copylist[c].srcStrip]->getBuffer(copyBuffer, copySize, _copylist[c].srcStart);
        this->_strips[s]->setBuffer(copyBuffer, copySize, _copylist[c].destPos); 
        free(copyBuffer);
      }
    }  

    this->_strips[s]->show();
  }

  if (cloneBuffer) free(cloneBuffer);
}


void K32_light::blackout() {
  this->stop();
  this->black();
}



// ANIM
//

// register new anim
K32_anim* K32_light::anim( int stripN, String animName, K32_anim* anim, int size, int offset ) 
{

  if (stripN >= this->_nstrips) {
    LOGF("ERROR: strip %i does not exist\n", stripN);
    return anim;
  }

  if (this->_animcounter >= LEDS_ANIMS_SLOTS) {
    LOG("ERROR: no more slot available to register new animation");
    return anim;
  }
  
  anim->name(animName);
  anim->setup( this->strip(stripN), size, offset );

  this->_anims[ this->_animcounter ] = anim;
  this->_animcounter++;
  // LOGINL("ANIM: register "); LOG(anim->name());

  return anim;
}

// get registered anim
K32_anim* K32_light::anim( String animName) 
{
  for (int k=0; k<this->_animcounter; k++)
    if (this->_anims[k]->name() == animName) {
      // LOGINL("LIGHT: "); LOG(name);
      return this->_anims[k];
    }
  LOGINL("ANIM: not found "); LOG(animName);
  return new K32_anim();

}

// stop all
void K32_light::stop() {
  for (int k=0; k<this->_animcounter; k++) this->_anims[k]->stop();
}

// Set FPS
void K32_light::fps(int f) {
  if (f >= 0) _fps = f;
  else _fps = LEDS_SHOW_FPS;
}


void K32_light::command(Orderz* order) 
{
  // ALL
  if (strcmp(order->action, "all") == 0 || strcmp(order->action, "strip") == 0 || strcmp(order->action, "pixel") == 0)
  {
      int offset = 0;
      if (strcmp(order->action, "strip") == 0) offset = 1;
      if (strcmp(order->action, "pixel") == 0) offset = 2;

      if (order->count() < offset+1) return;
      int red, green, blue, white = 0;
      
      red = order->getData(offset+0)->toInt();
      if (order->count() > offset+2) {
          green = order->getData(offset+1)->toInt();
          blue  = order->getData(offset+2)->toInt();
          if (order->count() > offset+3) 
              white = order->getData(offset+3)->toInt();
      }
      else { green = red; blue = red; white = red; }

      this->blackout();

      if (strcmp(order->action, "all") == 0) 
        this->all( red, green, blue, white );
      else if (strcmp(order->action, "strip") == 0) 
        this->strip(order->getData(0)->toInt())->all( red, green, blue, white );
      else if (strcmp(order->action, "pixel") == 0) 
        this->strip(order->getData(0)->toInt())->pix( order->getData(1)->toInt(), red, green, blue, white );

      this->show();
  }

  // MASTER
  else if (strcmp(order->action, "master") == 0)
  {
      int masterValue = this->anim("manu")->master();

      if (strcmp(order->subaction, "less") == 0)       masterValue -= 2;
      else if (strcmp(order->subaction, "more") == 0)  masterValue += 2;
      else if (strcmp(order->subaction, "full") == 0)  masterValue = 255;
      else if (strcmp(order->subaction, "tenmore") == 0)  masterValue += 10;
      else if (strcmp(order->subaction, "tenless") == 0)  masterValue -= 10;
      else if (strcmp(order->subaction, "fadeout") == 0) {
      if (!this->anim("manu")->hasmod("fadeout"))
          this->anim("manu")->mod(new K32_mod_fadeout)->name("fadeout")->at(0)->period(6000)->play();
      else
          this->anim("manu")->mod("fadeout")->play();
      }
      else if (strcmp(order->subaction, "fadein") == 0) {
      if (!this->anim("manu")->hasmod("fadein"))
          this->anim("manu")->mod(new K32_mod_fadein)->name("fadein")->at(0)->period(6000)->play();
      else
          this->anim("manu")->mod("fadein")->play();
      }
      else if (order->count() > 0) masterValue = order->getData(0)->toInt();

      this->anim("manu")->master( masterValue );
      this->anim("manu")->push();
  }

  // MEM (Manu)
  else if (strcmp(order->action, "mem") == 0)
  {
      LOGF("LIGHT: leds/mem %i\n",  order->getData(0)->toInt());

      if (order->count() > 1) {
        int masterValue = order->getData(1)->toInt();
        this->anim("manu")->master( order->getData(1)->toInt() );
      }

      // reset order
      if (order->count() > 0) {
        int mem = order->getData(0)->toInt();
        order->set("remote/macro")->addData(mem);
      }

  }

  // FRAME
  else if (strcmp(order->action, "frame") == 0)
  {
      LOG("DISPATCH: leds/frame");

      for(int k=0; k<order->count(); k++) {
        int v = order->getData(k)->toInt(); 
        if (v >= 0) this->anim("manu")->set(k, v);
      }
      this->anim("manu")->push();
  }

  // STOP
  else if (strcmp(order->action, "stop") == 0 || strcmp(order->action, "off") == 0 || strcmp(order->action, "blackout") == 0)
  {
      // reset order
      order->set("remote/stop");
  }

  // MODULATORS (Manu)
  else if (strcmp(order->action, "mod") == 0 || strcmp(order->action, "modi") == 0 || strcmp(order->action, "modall") == 0) 
  { 
      int startmod = -1;
      int stopmod = -1;

      // select mod by name
      if (strcmp(order->action, "mod") == 0 && order->count() >= 1) {
        startmod = this->anim("manu")->modindex( String(order->getData(0)->toStr()) );
        stopmod = startmod+1;
      }

      // select mod by index
      else if (strcmp(order->action, "modi") == 0  && order->count() >= 1)  {
        startmod = order->getData(0)->toInt();
        stopmod = startmod+1;
      }

      // select all mod 
      else if (strcmp(order->action, "modall") == 0) {
        startmod = 0;
        stopmod = ANIM_MOD_SLOTS;
      }

      // no mod found selected: exit
      if (startmod < 0) return;

      // apply to mod selection
      for(int k=startmod; k<stopmod; k++) {
        if (!this->anim("manu")->hasmod(k)) 
          continue;
        
        if (strcmp(order->subaction, "faster") == 0) 
          this->anim("manu")->mod(k)->faster();

        else if (strcmp(order->subaction, "slower") == 0) 
          this->anim("manu")->mod(k)->slower();

        else if (strcmp(order->subaction, "bigger") == 0) 
          this->anim("manu")->mod(k)->bigger();

        else if (strcmp(order->subaction, "smaller") == 0) 
          this->anim("manu")->mod(k)->smaller();
      }
  }

  return;
}

/*
 *   PRIVATE
 */


int K32_light::_nstrips = 0;

// thread function
void K32_light::refresh( void * parameter ) 
{
  K32_light* that = (K32_light*) parameter;
  while(true) 
  {
    vTaskDelay( pdMS_TO_TICKS( 1000/that->_fps ) );
    that->show();
  }
  
  vTaskDelete(NULL);
}

 /////////////////////////////////////////////
