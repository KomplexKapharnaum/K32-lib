/*
  K32_remote.cpp
  Created by Clement GANGNEUX, february 2020.
  Modified by RIRI, Mars 2020.
  Released under GPL v3.0
*/

#include "Arduino.h"
#include "K32_remote.h"
#include <Wire.h>

////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////// PUBLIC /////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////

K32_remote::K32_remote(K32_system *system, const int BTN_PIN[NB_BTN]) : system(system)
{
  LOG("REMOTE: init");

  this->lock = xSemaphoreCreateMutex();

  /* Init I2C and Buttons pins */
  Wire.begin(BTN_PIN[0], BTN_PIN[1]); // i2c pins
  this->mcp.begin();                  // i2c addr

  for (int i = 0; i < NB_BTN; i++)
  {
    this->buttons[i].pin = i;
    this->buttons[i].state = LOW;

    this->mcp.pinMode(i, INPUT);
    this->mcp.pullUp(i, HIGH);
  }

  // load LampGrad
  this->_lamp_grad = system->preferences.getUInt("lamp_grad", 30);

  // Start main task
  xTaskCreate(this->task,    // function
              "remote_task", // task name
              5000,          // stack memory
              (void *)this,  // args
              0,             // priority
              NULL);         // handler

  // Start read button state task
  xTaskCreate(this->read_btn_state, // function
              "read_btn_task",      // task name
              5000,                 // stack memory
              (void *)this,         // args
              0,                    // priority
              NULL);                // handler
};

void K32_remote::_lock()
{
  xSemaphoreTake(this->lock, portMAX_DELAY);
}

void K32_remote::_unlock()
{
  xSemaphoreGive(this->lock);
}
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////// SET ///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////

void K32_remote::setMacroMax(int macroMax)
{
  this->_lock();
  this->_macroMax = macroMax;
  this->_unlock();
}

void K32_remote::setAuto_Lock()
{
  this->_lock();
  this->_state = REMOTE_AUTO_LOCK;
  this->_unlock();
}

void K32_remote::setAuto()
{
  this->_lock();
  this->_state = REMOTE_AUTO;
  this->_unlock();
}

void K32_remote::setManu()
{
  this->_lock();
  this->_state = REMOTE_MANU;
  this->_unlock();
}

void K32_remote::setManu_Stm()
{
  this->_lock();
  this->_state = REMOTE_MANU_STM;
  this->_unlock();
}

void K32_remote::setManu_Stm_lock()
{
  this->_lock();
  this->_state = REMOTE_MANU_STM_LOCK;
  this->_unlock();
}

void K32_remote::setManu_Lock()
{
  this->_lock();
  this->_state = REMOTE_MANU_LOCK;
  this->_unlock();
}

void K32_remote::setManu_Lamp()
{
  this->_lock();
  this->_state = REMOTE_MANU_LAMP;
  this->_unlock();
}

void K32_remote::setSendMacro()
{
  this->_lock();
  this->_send_active_macro = false;
  this->_unlock();
}
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////// GET //////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////

remoteState K32_remote::getState()
{
  this->_lock();
  remoteState data = this->_state;
  this->_unlock();
  return data;
}

int K32_remote::getActiveMacro()
{
  this->_lock();
  int data = this->_activeMacro;
  this->_unlock();
  return data;
}

int K32_remote::getPreviewMacro()
{
  this->_lock();
  int data = this->_previewMacro;
  this->_unlock();
  return data;
}

int K32_remote::getLamp()
{
  this->_lock();
  int data = this->_lamp;
  this->_unlock();
  return data;
}

int K32_remote::getLampGrad()
{
  this->_lock();
  int data = this->_lamp_grad;
  this->_unlock();
  return data;
}

int K32_remote::getSendMacro()
{
  this->_lock();
  int data = this->_send_active_macro;
  this->_unlock();
  return data;
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

void K32_remote::task(void *parameter)
{
  K32_remote *that = (K32_remote *)parameter;
  TickType_t xFrequency = pdMS_TO_TICKS(REMOTE_CHECK);

  while (true)
  {
    /* Main loop */

    //////////////////////////////////////////////////////////////////////////////
    /////////////////////////// KEY LOCK & UNLOCK ////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////

    if (that->_key_lock == false && that->_check_key == false) // UNLOCK
    {
      that->_check_key = true;
      that->_send_active_macro = false;
#ifdef DEBUG_lib_btn
      LOG(":REMOTE key UNLOCK");
      LOGF("that->_state %d\n", that->_state);
      LOGF("that->_old_state %d\n", that->_old_state);
#endif
      if (that->_old_state < 3)
      {
        that->_state = REMOTE_MANU;
      }
      else if (that->_old_state == REMOTE_MANU_STM_LOCK)
      {
        that->_state = REMOTE_MANU_STM;
      }
#ifdef DEBUG_lib_btn
      LOGF("NEW that->_old_state %d\n", that->_old_state);
      LOGF("NEW that->_state %d\n", that->_state);
#endif
    }
    else if (that->_key_lock == true && that->_check_key == true) // LOCK
    {
      that->_check_key = false;
#ifdef DEBUG_lib_btn
      LOG(":REMOTE key LOCKED");
      LOGF("that->_state %d\n", that->_state);
      LOGF("that->_old_state %d\n", that->_old_state);
#endif
      if (that->_old_state != that->_state)
      {
        that->_old_state = that->_state;
      }
      if (that->_old_state == REMOTE_MANU)
      {
        that->_state = REMOTE_MANU_LOCK;
      }
      else if (that->_old_state == REMOTE_MANU_LOCK)
      {
        that->_state = REMOTE_MANU_LOCK;
      }
      else if (that->_old_state == REMOTE_MANU_STM)
      {
        that->_state = REMOTE_MANU_STM_LOCK;
      }
      else if (that->_old_state == REMOTE_AUTO)
      {
        that->_state = REMOTE_AUTO_LOCK;
      }
      else
      {
        that->_state = REMOTE_AUTO_LOCK;
      }
#ifdef DEBUG_lib_btn
      LOGF("NEW that->_old_state %d\n", that->_old_state);
      LOGF("NEW that->_state %d\n", that->_state);
#endif
    }

    //////////////////////////////////////////////////////////////////////////////
    ////////////////////* Check flags for each button *///////////////////////////
    //////////////////////////////////////////////////////////////////////////////
    for (int i = 0; i < NB_BTN; i++)
    {
      that->_lock();

      ///////////////////////////////////////////////////////////////////////////
      /////////////////////// Remote control is locked //////////////////////////
      ///////////////////////////////////////////////////////////////////////////

      if (that->_key_lock == true) // Remote control is locked
      {
        if (that->buttons[i].flag == 1) ////////// Short push //////////
        {
#ifdef DEBUG_lib_btn
          LOGF("LOCK REMOTE: Short push on button %d\n", i);
#endif
          if (that->_state == REMOTE_MANU_LAMP)
          {
            switch (i)
            {
            case 0: // Button 1 : Escape
              that->system->preferences.putUInt("lamp_grad", that->_lamp_grad);
              that->_state = that->_old_state;
#ifdef DEBUG_lib_btn
              LOGF("1 Escape STATE =  %d\n", that->_state);
#endif
              break;
            case 1: // Button 2 : Previous
              that->_lamp_grad = max(0, that->_lamp_grad - 1);
              break;
            case 2: // Button 3 : Forward
              that->_lamp_grad = min(255, that->_lamp_grad + 1);
              break;
            case 3: // Button 4 : Go
              that->system->preferences.putUInt("lamp_grad", that->_lamp_grad);
              that->_state = that->_old_state;
#ifdef DEBUG_lib_btn
              LOGF("1 Escape STATE =  %d\n", that->_state);
#endif
              break;
            }
          }
          /* Do nothing */
          that->buttons[i].flag = 0;
        }
        if (that->buttons[i].flag == 2) ////////// Long push //////////
        {
#ifdef DEBUG_lib_btn
          LOGF("LOCK REMOTE: Long push on button %d\n", i);
#endif
          switch (i)
          {
          case 1: // Button 2 : lamp on/off
            if (that->_lamp == -1)
            {
              that->_lamp = that->_lamp_grad;
            }
            else
            {
              that->_lamp = -1;
            }
            break;
          case 2: // Button 3 : lamp on/off
            if (that->_lamp == -1)
            {
              that->_lamp = 255;
            }
            else
            {
              that->_lamp = -1;
            }
            break;
          }
          /* Do nothing */
          that->buttons[i].flag = 0;
        }
        if (that->buttons[i].flag >= 10) ////////// Combined push //////////
        {
#ifdef DEBUG_lib_btn
          LOGF("LOCK REMOTE: Combined short push on button %d\n", that->buttons[i].flag);
#endif
          switch (that->buttons[i].flag)
          {
          case 10: // Button 1 and 2
                   /* */
                   /* Do actions */
                   /* */
            that->buttons[0].flag = 0;
            that->buttons[1].flag = 0; // reset flags after action
            break;
          case 20: // Button 1 and 3
                   /* */
                   /* Do actions */
                   /* */
            that->buttons[0].flag = 0;
            that->buttons[2].flag = 0; // reset flags after action
            break;
          case 30: // Button 1 and 4                                // LOCK & UNLOCK
            /* */
            that->_key_lock = false;
            /* */
            that->buttons[0].flag = 0;
            that->buttons[3].flag = 0; // reset flags after action
            break;
          case 21: // Button 2 and 3                                // LAMP_GRAD
            /* */
            if (that->_state == that->_old_state)
            {
              that->_state = REMOTE_MANU_LAMP;
            }
            else if (that->_state == REMOTE_MANU_LAMP)
            {
              that->_state = that->_old_state;
            }
            /* */
            that->buttons[1].flag = 0;
            that->buttons[2].flag = 0; // reset flags after action
            break;
          case 31: // Button 2 and 4
            /* */
            if (that->_key_lock == true)
            {
              that->_key_lock = false;
            }
            else
            {
              that->_key_lock = true;
            }
#ifdef DEBUG_lib_btn
            LOGF("that->_key_lock %d\n", that->_key_lock);
            LOGF("_check_key %d\n", that->_check_key);
#endif
            /* */
            that->buttons[1].flag = 0;
            that->buttons[3].flag = 0; // reset flags after action
            break;
          case 32: // Button 3 and 4
                   /* */
                   /* Do actions */
                   /* */
            that->buttons[2].flag = 0;
            that->buttons[3].flag = 0; // reset flags after action
            break;
          case 210: // Button 1, 2, 3
                    /* */
                    /* Do actions */
                    /* */
            that->buttons[0].flag = 0;
            that->buttons[1].flag = 0;
            that->buttons[2].flag = 0; // reset flags after action
            break;
          case 310: // Button 1, 2, 4
                    /* */
                    /* Do actions */
                    /* */
            that->buttons[0].flag = 0;
            that->buttons[1].flag = 0;
            that->buttons[3].flag = 0; // reset flags after action
            break;
          case 320: // Button 1, 3, 4
                    /* */
                    /* Do actions */
                    /* */
            that->buttons[0].flag = 0;
            that->buttons[2].flag = 0;
            that->buttons[3].flag = 0; // reset flags after action
            break;
          case 321: // Button 2, 3, 4
                    /* */
                    /* Do actions */
                    /* */
            that->buttons[1].flag = 0;
            that->buttons[2].flag = 0;
            that->buttons[3].flag = 0; // reset flags after action
            break;
          case 3210: // Button 1,2,3,4
                     /* */
                     /* Do actions */
                     /* */
            that->buttons[0].flag = 0;
            that->buttons[1].flag = 0;
            that->buttons[2].flag = 0;
            that->buttons[3].flag = 0; // reset flags after action
            break;
          }
        }
      }
      else

      ///////////////////////////////////////////////////////////////////////////
      ////////////////// Remote unlocked (_key_lock == false) ///////////////////
      ///////////////////////////////////////////////////////////////////////////

      {
        if (that->buttons[i].flag == 1) ////////// Short push //////////
        {
#ifdef DEBUG_lib_btn
          LOGF("UNLOCK REMOTE: Short push on button %d\n", i);
          LOGF("UNLOCK REMOTE: that->_state %d\n", that->_state);
#endif

          /* Instructions for the different buttons */
          switch (i)
          {
          case 0: // Button 1 : Escape
            if (that->_state == REMOTE_AUTO)
            {
              that->_state = REMOTE_AUTO_LOCK;
              that->_key_lock = true;
            }
            else if (that->_state == REMOTE_MANU)
            {
              that->_state = REMOTE_AUTO;
            }
            else if (that->_state == REMOTE_MANU_LAMP)
            {
              that->system->preferences.putUInt("lamp_grad", that->_lamp_grad);
              that->_state = REMOTE_MANU;
              that->_lamp = -1;
            }
            else if (that->_state == REMOTE_MANU_STM)
            {
              that->_state = REMOTE_MANU;
            }
#ifdef DEBUG_lib_btn
            LOGF("Escape STATE =  %d\n", that->_state);
#endif
            break;
          case 1: // Button 2 : Previous
            if (that->_state == REMOTE_MANU)
            {
              that->_previewMacro--;
              if (that->_previewMacro < 0)
              {
                that->_previewMacro = that->_macroMax - 1;
              }
#ifdef DEBUG_lib_btn
              LOGF("Preview -- STATE =  %d\n", that->_state);
#endif
            }
            else if (that->_state == REMOTE_MANU_LAMP)
            {
              that->_lamp_grad = max(0, that->_lamp_grad - 1);
#ifdef DEBUG_lib_btn
              LOGF("LAMP -- STATE =  %d\n", that->_state);
#endif
            }
            else if (that->_state == REMOTE_AUTO)
            {
              that->_state = REMOTE_MANU;
            }
            break;
          case 2: // Button 3 : Forward
            if (that->_state == REMOTE_MANU)
            {
              that->_previewMacro++;
              if (that->_previewMacro >= that->_macroMax)
              {
                that->_previewMacro = 0;
              }
#ifdef DEBUG_lib_btn
              LOGF("Preview ++  STATE =  %d\n", that->_state);
#endif
            }
            else if (that->_state == REMOTE_MANU_LAMP)
            {
              that->_lamp_grad = min(255, that->_lamp_grad + 1);
#ifdef DEBUG_lib_btn
              LOGF("LAMP ++ that->_lamp_grad =  %d\n", that->_lamp_grad);
              LOGF("LAMP ++ STATE =  %d\n", that->_state);
#endif
            }
            else if (that->_state == REMOTE_AUTO)
            {
              that->_state = REMOTE_MANU;
            }
            break;
          case 3: // Button 4 : Go
            if (that->_state == REMOTE_MANU)
            {
              that->_activeMacro = that->_previewMacro;
              that->_send_active_macro = true;
            }
            else if (that->_state == REMOTE_MANU_LAMP)
            {
              that->system->preferences.putUInt("lamp_grad", that->_lamp_grad);
              that->_state = REMOTE_MANU;
              that->_lamp = -1;
            }
#ifdef DEBUG_lib_btn
            LOGF("Go STATE =  %d\n", that->_state);
#endif
            break;
          }
          that->buttons[i].flag = 0;
        }
        else if (that->buttons[i].flag == 2) ////////// Long push //////////
        {
#ifdef DEBUG_lib_btn
          LOGF("UNLOCK REMOTE: Long push on button %d\n", i);
#endif

          /* Instructions for the different buttons */
          switch (i)
          {
          case 0: // Button 1 : BlackOut Forced
            that->_activeMacro = that->_macroMax - 1;
            that->_previewMacro = that->_macroMax - 1;
            that->_send_active_macro = true;
            that->_state = REMOTE_MANU;
            break;
          case 1: // Button 2 : lamp on/off
            if (that->_lamp == -1)
            {
              that->_lamp = that->_lamp_grad;
            }
            else
            {
              that->_lamp = -1;
            }
            break;
          case 2: // Button 3 : lamp on/off
            if (that->_lamp == -1)
            {
              that->_lamp = 255;
            }
            else
            {
              that->_lamp = -1;
            }
            break;
          case 3: // Button 4 : Go Forced
            that->_activeMacro = that->_previewMacro;
            that->_state = REMOTE_MANU_LOCK;
            that->_key_lock = true;
            break;
          }
          that->buttons[i].flag = 0;
        }
        else if (that->buttons[i].flag >= 10) ////////// Combined push //////////
        {
#ifdef DEBUG_lib_btn
          LOGF("UNLOCK REMOTE: Combined short push on button %d\n", that->buttons[i].flag);
#endif
          switch (that->buttons[i].flag)
          {
          case 10: // Button 1 and 2
                   /* */
                   /* Do actions */
                   /* */
            that->buttons[0].flag = 0;
            that->buttons[1].flag = 0; // reset flags after action
            break;
          case 20: // Button 1 and 3
                   /* */
                   /* Do actions */
                   /* */
            that->buttons[0].flag = 0;
            that->buttons[2].flag = 0; // reset flags after action
            break;
          case 30: // Button 1 and 4                                // LOCK & UNLOCK
            /* */
            that->_key_lock = true;
            /* */
            that->buttons[0].flag = 0;
            that->buttons[3].flag = 0; // reset flags after action
            break;
          case 21: // Button 2 and 3                                // LAMP_GRAD
                   /* */
            that->_state = REMOTE_MANU_LAMP;
            /* */
            that->buttons[1].flag = 0;
            that->buttons[2].flag = 0; // reset flags after action
            break;
          case 31: // Button 2 and 4
            /* */
            if (that->_key_lock == true)
            {
              that->_key_lock = false;
            }
            else
            {
              that->_key_lock = true;
            }
#ifdef DEBUG_lib_btn
            LOGF("that->_key_lock %d\n", that->_key_lock);
            LOGF("_check_key %d\n", that->_check_key);
#endif
            /* */
            that->buttons[1].flag = 0;
            that->buttons[3].flag = 0; // reset flags after action
            break;
          case 32: // Button 3 and 4
                   /* */
                   /* Do actions */
                   /* */
            that->buttons[2].flag = 0;
            that->buttons[3].flag = 0; // reset flags after action
            break;
          case 210: // Button 1, 2, 3
                    /* */
                    /* Do actions */
                    /* */
            that->buttons[0].flag = 0;
            that->buttons[1].flag = 0;
            that->buttons[2].flag = 0; // reset flags after action
            break;
          case 310: // Button 1, 2, 4
                    /* */
                    /* Do actions */
                    /* */
            that->buttons[0].flag = 0;
            that->buttons[1].flag = 0;
            that->buttons[3].flag = 0; // reset flags after action
            break;
          case 320: // Button 1, 3, 4
                    /* */
                    /* Do actions */
                    /* */
            that->buttons[0].flag = 0;
            that->buttons[2].flag = 0;
            that->buttons[3].flag = 0; // reset flags after action
            break;
          case 321: // Button 2, 3, 4
                    /* */
                    /* Do actions */
                    /* */
            that->buttons[1].flag = 0;
            that->buttons[2].flag = 0;
            that->buttons[3].flag = 0; // reset flags after action
            break;
          case 3210: // Button 1,2,3,4
                     /* */
                     /* Do actions */
                     /* */
            that->buttons[0].flag = 0;
            that->buttons[1].flag = 0;
            that->buttons[2].flag = 0;
            that->buttons[3].flag = 0; // reset flags after action
            break;
          }
        }
      }

      that->_unlock();
      yield();
    }
    /********/
    vTaskDelay(xFrequency);
  } //while (true)
} //void K32_remote::task(void *parameter)

////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////// STATE ////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////

void K32_remote::read_btn_state(void *parameter)
{
  K32_remote *that = (K32_remote *)parameter;
  TickType_t xFrequency = pdMS_TO_TICKS(BTN_CHECK);
  unsigned long debounceDelay = 50;   // the debounce time; increase if the output flickers
  unsigned long longPushDelay = 1000; // Delay for a long push of the button
  int nbCombined = 0;                 // Variable to check combined push of button

  while (true)
  {
    for (int i = 0; i < NB_BTN; i++)
    {

      that->_lock();
      // read the state of the switch into a local variable:
      int reading = that->mcp.digitalRead(that->buttons[i].pin);

      // pushed
      if (reading == LOW)
      {
        if (that->buttons[i].state == HIGH) // was released
        {
          that->buttons[i].state = LOW;
          that->buttons[i].lastPushTime = millis(); // Record time of pushing button
        }

        else // was already pushed
        {
          if ((millis() - that->buttons[i].lastPushTime > longPushDelay) && (that->buttons[i].lastPushTime != 0))
          {
            that->buttons[i].flag = 2;         // Long push
            that->buttons[i].lastPushTime = 0; // Reset counter
          }
        }
      }

      // Released
      else
      {
        if (that->buttons[i].state == LOW) // was pushed
        {
          that->buttons[i].state = HIGH; // update state
          if ((millis() - that->buttons[i].lastPushTime > debounceDelay) && (that->buttons[i].lastPushTime != 0))
          {
            /* Check for combined push, if another button has been pushed */
            int nbOtherPush = 0;
            for (int j = 0; j < NB_BTN; j++)
            {
              if (j != i)
              {
                if ((that->buttons[j].state == LOW) || (that->buttons[j].flag == 3))
                {
                  nbOtherPush++;
                }
              }
            }
            if (nbOtherPush == 0)
            {
              that->buttons[i].flag = 1; // simple short push
            }
            else // nbOtherPush > 0
            {
              that->buttons[i].flag = 3; // combined short push
              nbCombined++;
              if (nbCombined == nbOtherPush + 1) // all combined buttons have flag set to 3 -> set new combined flag
              {
                int newFlag = 0;
                int power = 0;
                for (int j = 0; j < NB_BTN; j++) // First check to build the new flag
                {
                  if (that->buttons[j].flag == 3)
                  {
                    newFlag += j * pow(10, power);
                    power++;
                  }
                }
                for (int j = 0; j < NB_BTN; j++) // Second check to update flag
                {
                  if (that->buttons[j].flag == 3)
                  {
#ifdef DEBUG_lib_btn
                    LOGF("newflag %d\n", newFlag);
#endif
                    that->buttons[j].flag = newFlag;
                  }
                }
                nbCombined = 0; // Reset combined signals counter
              }
            }
          }
        }
      }
      that->_unlock();
    }

    vTaskDelay(xFrequency);
  }
}
