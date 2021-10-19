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

}


int K32_light::id() {
    return _id;
}

void K32_light::id(int id) {
    int old = k32->system->prefs.getUInt("LULU_id", 0);
    if (id != old) {
      k32->system->prefs.putUInt("LULU_id", id);
      _id = k32->system->prefs.getUInt("LULU_id", 0);
    }
}



K32_light* K32_light::addFixture(K32_fixture* fix)
{
  if (!fix) return this;
  if (this->_nfixtures >= LIGHT_MAXFIXTURES) {
    LOG("LIGHT: no more fixture can be attached..");
    return this;
  }
  this->_fixtures[this->_nfixtures] = fix;
  this->_nfixtures += 1;

  return this;
}


K32_light* K32_light::addFixtures(K32_fixture* fixs[], int count)
{
  for (int k=0; k<count; k++) this->addFixture(fixs[k]);
  return this;
}

// link every strip to masterStrip
void K32_light::cloneFixturesFrom(K32_fixture* masterFixture) {
  this->_masterClone = masterFixture;
}

// link every strip to masterStrip
void K32_light::copyFixture(stripcopy copy) {
  if (_copyMax<LIGHT_MAX_COPY) 
  {
    _copylist[_copyMax] = copy;
    _copyMax += 1;
  }
}

K32_fixture* K32_light::fixture(int s) {
  return this->_fixtures[s];
}

K32_light* K32_light::fixtures() {
  return this;
}

K32_light* K32_light::black()
{
  for (int s = 0; s < this->_nfixtures; s++) this->_fixtures[s]->black();
  return this;
}

K32_light* K32_light::all(pixelColor_t color)
{
  for (int s = 0; s < this->_nfixtures; s++) this->_fixtures[s]->all(color);
  return this;
}

K32_light* K32_light::all(int red, int green, int blue, int white)
{
  return this->all( pixelFromRGBW(red, green, blue, white) );
}

K32_light* K32_light::pix(int pixel, pixelColor_t color) {
  for (int s = 0; s < this->_nfixtures; s++) this->_fixtures[s]->pix(pixel, color);
  return this;
}

K32_light* K32_light::pix(int pixel, int red, int green, int blue, int white) {
  return this->pix( pixel, pixelFromRGBW(red, green, blue, white) );
}

void K32_light::show() 
{
  // CLONE ALL from master strip (if _masterClone exist)
  pixelColor_t* cloneBuffer = NULL;
  int cloneSize = 0;
  if (this->_masterClone && this->_masterClone->dirty()) {
    cloneSize = this->_masterClone->size();
    cloneBuffer = static_cast<pixelColor_t*>(malloc(cloneSize * sizeof(pixelColor_t)));
    this->_masterClone->getBuffer(cloneBuffer, cloneSize);

    if (cloneSize > 0)
      for (int s=0; s<this->_nfixtures; s++)
        this->_fixtures[s]->setBuffer(cloneBuffer, cloneSize); 
    
    free(cloneBuffer);
  }

  // COPY Fixtures
  for (int c=0; c<_copyMax; c++) {
    if (_copylist[c].srcFixture && _copylist[c].destFixture) 
      if (_copylist[c].srcFixture->dirty())
      {
        pixelColor_t* copyBuffer = NULL;
        int copySize = 0;
        copySize = _copylist[c].srcStop-_copylist[c].srcStart+1;
        copyBuffer = static_cast<pixelColor_t*>(malloc(copySize * sizeof(pixelColor_t)));
        _copylist[c].srcFixture->getBuffer(copyBuffer, copySize, _copylist[c].srcStart);
        _copylist[c].destFixture->setBuffer(copyBuffer, copySize, _copylist[c].destPos); 
        free(copyBuffer);
        _copylist[c].destFixture;
      }
  }  

  for (int s=0; s<this->_nfixtures; s++)  this->_fixtures[s]->show();

}


void K32_light::blackout() {
  this->stop();
  this->black();
}



// ANIM
//

// register new anim
K32_anim* K32_light::anim( String animName, K32_anim* anim, int size, int offset ) 
{

  if (this->_animcounter >= LIGHT_ANIMS_SLOTS) {
    LOG("ERROR: no more slot available to register new animation");
    return anim;
  }
  
  anim->name(animName);
  anim->setup( size, offset );

  this->_anims[ this->_animcounter ] = anim;
  this->_animcounter++;
  // LOGF2("ANIM: register %s size %d\n", anim->name(), size);

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
  return nullptr;

}

// stop all
void K32_light::stop() {
  for (int k=0; k<this->_animcounter; k++) this->_anims[k]->stop();
}

// Set FPS
void K32_light::fps(int f) {
  if (f >= 0) _fps = f;
  else _fps = LIGHT_SHOW_FPS;
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
        this->fixture(order->getData(0)->toInt())->all( red, green, blue, white );
      else if (strcmp(order->action, "pixel") == 0) 
        this->fixture(order->getData(0)->toInt())->pix( order->getData(1)->toInt(), red, green, blue, white );

      this->show();
  }

  // MASTER
  else if (strcmp(order->action, "master") == 0)
  {
      int masterValue = this->anim("mem-strip")->master();

      if (strcmp(order->subaction, "less") == 0)       masterValue -= 2;
      else if (strcmp(order->subaction, "more") == 0)  masterValue += 2;
      else if (strcmp(order->subaction, "full") == 0)  masterValue = 255;
      else if (strcmp(order->subaction, "tenmore") == 0)  masterValue += 10;
      else if (strcmp(order->subaction, "tenless") == 0)  masterValue -= 10;
      else if (strcmp(order->subaction, "fadeout") == 0) {
      if (!this->anim("mem-strip")->hasmod("fadeout"))
          this->anim("mem-strip")->mod(new K32_mod_fadeout)->name("fadeout")->at(0)->period(6000)->play();
      else
          this->anim("mem-strip")->mod("fadeout")->play();
      }
      else if (strcmp(order->subaction, "fadein") == 0) {
      if (!this->anim("mem-strip")->hasmod("fadein"))
          this->anim("mem-strip")->mod(new K32_mod_fadein)->name("fadein")->at(0)->period(6000)->play();
      else
          this->anim("mem-strip")->mod("fadein")->play();
      }
      else if (order->count() > 0) masterValue = order->getData(0)->toInt();

      this->anim("mem-strip")->master( masterValue );
      this->anim("mem-strip")->push();
      if (this->anim("artnet-dmxfix"))
      {
      this->anim("artnet-dmxfix")->master( masterValue );// to do if dmxthru
      this->anim("artnet-dmxfix")->push();// to do if dmxthru
      }
  }

  // MEM (Manu)
  else if (strcmp(order->action, "mem") == 0)
  {
      LOGF("LIGHT: leds/mem %i\n",  order->getData(0)->toInt());

      if (order->count() > 1) {
        int masterValue = order->getData(1)->toInt();
        this->anim("mem-strip")->master( order->getData(1)->toInt() );
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

        if (v >= 0) this->anim("mem-strip")->set(k, v);           // normal
      //  LOGF("k->%d ", k);                                 // normal
      //  LOGF("=%d ", v);                                   // normal
      }                                                      // normal
      this->anim("mem-strip")->push();                            // normal


      //   if (this->anim("artnet-dmxfix"))                           // dmx_par
      //    if (v >= 0) this->anim("artnet-dmxfix")->set(k, v);       // dmx par
      //   else                                                 // dmx par
      //    if (v >= 0) this->anim("mem-strip")->set(k, v);          // dmx par
      // //  LOGF("k->%d ", k);                                 // dmx par
      // //  LOGF("=%d ", v);                                   // dmx par
      // }                                                      // dmx par
      //  if (this->anim("artnet-dmxfix"))                            // dmx par
      //  {                                                     // dmx par
      //    LOG("DMX THRU");                                    // dmx par
      //    this->anim("artnet-dmxfix")->push();                      // dmx par
      //  }                                                     // dmx par
      //  else                                                  // dmx par
      //  {                                                     // dmx par
      //    LOG("manu ");                                       // dmx par
      //    this->anim("mem-strip")->push();                         // dmx par
      //  }                                                     // dmx par
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
        startmod = this->anim("mem-strip")->modindex( String(order->getData(0)->toStr()) );
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
        if (!this->anim("mem-strip")->hasmod(k)) 
          continue;
        
        if (strcmp(order->subaction, "faster") == 0)
        { 
        this->anim("mem-strip")->mod(k)->faster();
        if (this->anim("artnet-dmxfix"))
        this->anim("artnet-dmxfix")->mod(k)->faster();
        }
        else if (strcmp(order->subaction, "slower") == 0)
        {
          this->anim("mem-strip")->mod(k)->slower();
          if (this->anim("artnet-dmxfix"))
          this->anim("artnet-dmxfix")->mod(k)->slower();
        } 

        else if (strcmp(order->subaction, "bigger") == 0) 
        {
          this->anim("mem-strip")->mod(k)->bigger();
          if (this->anim("artnet-dmxfix"))
          this->anim("artnet-dmxfix")->mod(k)->bigger();
        }

        else if (strcmp(order->subaction, "smaller") == 0) 
         {
          this->anim("mem-strip")->mod(k)->smaller();
          if (this->anim("artnet-dmxfix"))
          this->anim("artnet-dmxfix")->mod(k)->smaller();
         }
      }
  }

  // maree
  else if (strcmp(order->action, "maree") == 0)
  {
    if (strcmp(order->subaction, "go_p") == 0)
    {
      int duration  = order->getData(0)->toInt();
      int high      = order->getData(1)->toInt();
      int low       = order->getData(2)->toInt();
      int position  = order->getData(3)->toInt();


      // haute();
      this->anim("maree")->push(high)
              ->unmod()
              ->mod(new K32_mod_fadein)
                ->absolute()
                ->at(0)
                ->mini(position)
                ->maxi(high)
                ->period(duration*1000)
                ->play();

    } 
    else if (strcmp(order->subaction,"go_m")==0)
    {
      int duration  = order->getData(0)->toInt();
      int high      = order->getData(1)->toInt();
      int low       = order->getData(2)->toInt();
      int position  = order->getData(3)->toInt();


      // basse();
      this->anim("maree")->push(low)
              ->unmod()
              ->mod(new K32_mod_fadeout)
                ->absolute()
                ->at(0)
                ->mini(low)
                ->maxi(position)
                ->period(duration*1000)
                ->play();
    }
    else if (strcmp(order->subaction,"pixel")==0)
    {
      int position  = order->getData(0)->toInt();


      // pixel();
      this->anim("maree")->push(position)
                ->play();
    }
  }

  return;
}

/*
 *   PRIVATE
 */


int K32_light::_nfixtures = 0;

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
