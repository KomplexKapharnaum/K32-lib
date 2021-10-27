/*
  K32_remote.cpp
  Created by Clement GANGNEUX, february 2020.
  Modified by RIRI, Mars 2020.
  Modified by MGR, July 2020.
  Released under GPL v3.0
*/

#include "K32_remote.h"

////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////// PUBLIC /////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////

K32_remote::K32_remote(K32* k32, K32_mcp *mcp) : K32_plugin("remote", k32)
{
  this->mcp = mcp;
  LOG("REMOTE: init");

  this->semalock = xSemaphoreCreateMutex();

  this->_state = REMOTE_AUTO;
  this->_old_state = REMOTE_AUTO;

  this->_key_lock = true;

  if (this->mcp && this->mcp->ok)
    for (int i = 0; i < NB_BTN; i++)
      this->mcp->input(i);

  // load LampGrad
  this->_lamp_grad = k32->system->prefs.getUInt("lamp_grad", 127);

  // Start main task
  xTaskCreate(this->task,    // function
              "remote_task", // task name
              5000,          // stack memory
              (void *)this,  // args
              0,             // priority
              &xHandle);         // handler

};

void K32_remote::stop()
{
  if (xHandle != NULL) vTaskDelete(xHandle);
  xHandle = NULL;
}

void K32_remote::_semalock()
{
  xSemaphoreTake(this->semalock, portMAX_DELAY);
}

void K32_remote::_semaunlock()
{
  xSemaphoreGive(this->semalock);
}
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////// SET ///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////

K32_remote* K32_remote::setMacroMax(uint8_t macroMax)
{
  this->_semalock();
  this->_macroMax = macroMax;
  if (_macroMax<1) _macroMax = 1;
  this->_semaunlock();
  return this;
}

K32_remote* K32_remote::setState(remoteState state)
{
  if (this->_state == state) return this;

  this->_semalock();
  
  // Exit from REMOTE_MANU_LAMP: save grad !
  if (this->_state == REMOTE_MANU_LAMP) {
    k32->system->prefs.putUInt("lamp_grad", this->_lamp_grad);
    this->_lamp = -1;
  }

  this->_state = state;
  this->_semaunlock();

  Orderz* newOrder = new Orderz("remote/state");
  newOrder->addData(this->_state);
  this->emit( newOrder );

  return this;
}


K32_remote* K32_remote::stmBlackout() 
{
  this->stmSetMacro(this->_macroMax - 1);
  return this;
}

K32_remote* K32_remote::stmSetMacro(uint8_t macro)
{
  this->changeMacro(macro);
  this->changePreviewMacro(this->_activeMacro);
  this->setState(REMOTE_MANU_STM);

  return this;
}

K32_remote* K32_remote::stmNext()
{
  this->stmSetMacro( this->_activeMacro + 1 );

  // Last macro -> back to AUTO LOCK
  if (this->_activeMacro == this->_macroMax - 1) {
    this->setState(REMOTE_AUTO);
    this->lock();
  }

  return this;
}

K32_remote* K32_remote::lock() {
  this->_semalock();
  this->_key_lock = true;
  this->_semaunlock();  

  this->emit( "remote/lock" );

  return this;
}

K32_remote* K32_remote::unlock() {
  this->_semalock();
  this->_key_lock = false;
  this->_semaunlock();

  this->emit( "remote/unlock" );

  return this;
}


////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////// GET //////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////

bool K32_remote::isLocked() {
  return this->_key_lock;
}

remoteState K32_remote::getState()
{
  this->_semalock();
  remoteState data = this->_state;
  this->_semaunlock();
  return data;
}

int K32_remote::getActiveMacro()
{
  this->_semalock();
  int data = this->_activeMacro;
  this->_semaunlock();
  return data;
}

int K32_remote::getPreviewMacro()
{
  this->_semalock();
  int data = this->_previewMacro;
  this->_semaunlock();
  return data;
}

int K32_remote::getLamp()
{
  this->_semalock();
  int data = this->_lamp;
  this->_semaunlock();
  return data;
}

int K32_remote::getLampGrad()
{
  this->_semalock();
  int data = this->_lamp_grad;
  this->_semaunlock();
  return data;
}

void K32_remote::command(Orderz* order) 
{
  // remote/stop
  if (strcmp(order->action, "stop") == 0 || strcmp(order->action, "off") == 0 || strcmp(order->action, "blackout") == 0)
  {
      this->stmBlackout();
  }

  // remote/macro i
  else if (strcmp(order->action, "macro") == 0)
  {
      this->stmSetMacro( order->getData(0)->toInt() );
  }

}


////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
/*
 * /////////////////////////////////////// PRIVATE /////////////////////////////////////
 */
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////// TASK /////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////


K32_remote* K32_remote::changeMacro(uint8_t macro)
{
  this->_semalock();
  this->_activeMacro = macro % this->_macroMax;
  this->_semaunlock();

  Orderz* newOrder = new Orderz("remote/macro");
  newOrder->addData(this->_activeMacro);
  this->emit( newOrder );

  return this;
}

K32_remote* K32_remote::changePreviewMacro(uint8_t macro)
{
  this->_semalock();
  this->_previewMacro = macro % this->_macroMax;
  this->_semaunlock();

  Orderz* newOrder = new Orderz("remote/preview");
  newOrder->addData(this->_previewMacro);
  this->emit( newOrder );

  return this;
}




void K32_remote::task(void *parameter)
{
  K32_remote *that = (K32_remote *)parameter;
  TickType_t xFrequency = pdMS_TO_TICKS(REMOTE_CHECK);

  while (true)
  {

    //////////////////////////////////////////////////////////////////////////////
    ////////////////////* Check flags for each button *///////////////////////////
    //////////////////////////////////////////////////////////////////////////////
    ioflag flags[NB_BTN];
    uint8_t countPressed = 0; 
    uint8_t countLongPressed = 0; 
    uint8_t countReleased = 0;

    // read and count flags
    for (int i = 0; i < NB_BTN; i++) 
    {
      if (that->mcp && that->mcp->ok) flags[i] = that->mcp->flag(i);                             // read flag but don't consume yet
      else flags[i] = MCPIO_NOT;                                         

      if (flags[i] == MCPIO_PRESS || flags[i] == MCPIO_PRESS_LONG) countPressed++;               // count pressed button
      if (flags[i] == MCPIO_PRESS_LONG) countLongPressed++;                                      // count long pressed button
      if (flags[i] == MCPIO_RELEASE_LONG || flags[i] == MCPIO_RELEASE_SHORT) countReleased++;    // count released button
    }
    uint8_t countEngaged = countPressed + countReleased;
    
    ///////////////////////////////////////////////////////////////////////////
    ////////// STABLE state: all buttons are LONG_PRESSED or RELEASED //////////
    ///////////////////////////////////////////////////////////////////////////

    if (countEngaged > 0)
      if (countEngaged == countLongPressed || countEngaged == countReleased)
      {
          int actionFlag = MCPIO_NOT;
          int actionCode = 0;   

          for (int i = 0; i < NB_BTN; i++) 
          {
            if (that->mcp && that->mcp->ok) that->mcp->consume(i);                               // Stable state -> consume values
            if (flags[i] > actionFlag) actionFlag = flags[i];
            if (flags[i] != MCPIO_NOT) actionCode = actionCode*10+i+1;  
          }

          ///////////////////////////////////////////////////////////////////////////
          /////////////////////// Remote control is locked //////////////////////////
          ///////////////////////////////////////////////////////////////////////////

          // LOCKED
          if (that->isLocked())
          {
            switch(actionCode) 
            {
              //////////// ONE BUTTON

              case 1:
              case 4: 
                // Button 1 SHORT
                if (actionFlag == MCPIO_RELEASE_SHORT) 
                {
                  // Lamp: Save + Escape
                  if (that->_state == REMOTE_MANU_LAMP) 
                  {
                    that->k32->system->prefs.putUInt("lamp_grad", that->_lamp_grad);
                    that->setState(that->_old_state);
                    #ifdef DEBUG_lib_btn
                    LOGF("REMOTE: 1/4 Escape STATE =  %d\n", that->_state);
                    #endif
                  }
                  // STM: Stop
                  else if (that->_state == REMOTE_MANU_STM) 
                  {
                    that->stmBlackout();
                  }
                }
                break;
              
              case 2: 
                // Button 2 SHORT : Lower lamp
                if (actionFlag == MCPIO_RELEASE_SHORT) 
                {
                  that->_lamp_grad = max(0, that->_lamp_grad - 1);
                  that->_lamp = that->_lamp_grad;
                }
                // Button 2 LONG : Lamp ON/OFF
                else
                {
                  if (that->_lamp == -1) that->_lamp = that->_lamp_grad;
                  else that->_lamp = -1;
                }
                break;
              
              case 3: 
                // Button 3 SHORT : Higher lamp
                if (actionFlag == MCPIO_RELEASE_SHORT) 
                {
                  that->_lamp_grad = min(255, that->_lamp_grad + 1);
                  that->_lamp = that->_lamp_grad;
                }
                // Button 3 LONG : Lamp ON/OFF
                else
                {
                  if (that->_lamp == -1) that->_lamp = 255;
                  else that->_lamp = -1;
                } 
                break;

              //////////// MULTIPLE BUTTONS

              case 14:
                // Button 1 + 4 : UNLOCK
                that->unlock();
                #ifdef DEBUG_lib_btn
                  LOG("REMOTE: unlock");
                #endif
                break;

              case 23: 
                // Button 2 + 3 : LAMP_GRAD
                if (that->_state == REMOTE_MANU_LAMP)
                {
                  that->setState(that->_old_state);
                }
                else {
                  that->_old_state = that->_state;
                  that->setState(REMOTE_MANU_LAMP);
                  that->_lamp = that->_lamp_grad;
                }
                break;
              
              case 234:
                // Button 2 + 3 + 4
                break;

              case 1234:
                // Button 1 + 2 + 3 + 4
                break;
              
            }

          }

          ///////////////////////////////////////////////////////////////////////////
          /////////////////////// Remote control is unlocked //////////////////////////
          ///////////////////////////////////////////////////////////////////////////

          // UNLOCKED
          else 
          {
            switch(actionCode) 
            {
              //////////// ONE BUTTON

              case 1: 
                // Button 1 SHORT : ESCAPE
                if (actionFlag == MCPIO_RELEASE_SHORT) 
                {
                  if (that->_state == REMOTE_AUTO)            that->lock();
                  else if (that->_state == REMOTE_MANU)       that->setState(REMOTE_AUTO);
                  else if (that->_state == REMOTE_MANU_STM)   {
                    that->stmBlackout();
                    that->setState(REMOTE_MANU);
                  }
                  else if (that->_state == REMOTE_MANU_LAMP)  that->setState(REMOTE_MANU);

                  #ifdef DEBUG_lib_btn
                              LOGF("REMOTE: Escape STATE =  %d\n", that->_state);
                  #endif
                }

                // Button 1 LONG : BLACKOUT
                else  that->stmBlackout() ;
                
                break;
              
              case 2: 
                // Button 2 SHORT : PREVIOUS
                if (actionFlag == MCPIO_RELEASE_SHORT) 
                {
                  if (that->_state == REMOTE_MANU)
                  {
                    that->_previewMacro--;
                    if (that->_previewMacro < 0) that->_previewMacro = that->_macroMax - 1;
                    #ifdef DEBUG_lib_btn
                      LOGF("REMOTE: Preview -- STATE =  %d\n", that->_state);
                    #endif
                  }
                  else if (that->_state == REMOTE_MANU_LAMP)
                  {
                    that->_lamp_grad = max(0, that->_lamp_grad - 1);
                    that->_lamp = that->_lamp_grad;
                    #ifdef DEBUG_lib_btn
                      LOGF("REMOTE: LAMP -- STATE =  %d\n", that->_state);
                    #endif
                  }
                  else if (that->_state == REMOTE_AUTO) 
                    that->setState(REMOTE_MANU);

                }

                // Button 2 LONG : Lamp ON/OFF
                else
                {
                  if (that->_lamp == -1) that->_lamp = that->_lamp_grad;
                  else that->_lamp = -1;
                }
                break;
              
              case 3: 
                // Button 3 SHORT : NEXT
                if (actionFlag == MCPIO_RELEASE_SHORT) 
                {
                  if (that->_state == REMOTE_MANU)
                  {
                    that->_previewMacro++;
                    if (that->_previewMacro >= that->_macroMax) that->_previewMacro = 0;
                    #ifdef DEBUG_lib_btn
                                  LOGF("REMOTE: Preview ++  STATE =  %d\n", that->_state);
                    #endif
                  }
                  else if (that->_state == REMOTE_MANU_LAMP)
                  {
                    that->_lamp_grad = min(255, that->_lamp_grad + 1);
                    that->_lamp = that->_lamp_grad;
                    #ifdef DEBUG_lib_btn
                                  LOGF("REMOTE: LAMP ++ that->_lamp_grad =  %d\n", that->_lamp_grad);
                                  LOGF("REMOTE: LAMP ++ STATE =  %d\n", that->_state);
                    #endif
                  }
                  else if (that->_state == REMOTE_AUTO) 
                    that->setState(REMOTE_MANU);
                  
                }

                // Button 3 LONG : Lamp ON/OFF
                else
                {
                  if (that->_lamp == -1) that->_lamp = 255;
                  else that->_lamp = -1;
                } 
                break;
              
              case 4: 
                // Button 4 SHORT : 
                if (actionFlag == MCPIO_RELEASE_SHORT) 
                {
                  if (that->_state == REMOTE_MANU)
                  {
                    that->changeMacro(that->_previewMacro);
                  }
                  else if (that->_state == REMOTE_MANU_LAMP)
                  {
                    that->k32->system->prefs.putUInt("lamp_grad", that->_lamp_grad);
                    that->_state = REMOTE_MANU;
                    that->_lamp = -1;
                  }
                  #ifdef DEBUG_lib_btn
                              LOGF("REMOTE: Go STATE =  %d\n", that->_state);
                  #endif
                }

                // Button 4 LONG : GO Force
                else
                {
                  that->changeMacro(that->_previewMacro);
                  that->setState(REMOTE_MANU);
                  that->lock();
                } 
                break;

              //////////// MULTIPLE BUTTONS

              case 14:
                // Button 1 + 4 : LOCK
                that->lock();
                #ifdef DEBUG_lib_btn
                  LOG("REMOTE: LOCKED");
                #endif
                break;

              case 23: 
                // Button 2 + 3 : LAMP MODE
                that->setState(REMOTE_MANU_LAMP);
                break;
              
              case 234:
                // Button 2 + 3 + 4
                break;

              case 1234:
                // Button 1 + 2 + 3 + 4
                break;
              
            }
          }


      }
  
    vTaskDelay(xFrequency);
  } //while (true)

} //void K32_remote::task(void *parameter)
