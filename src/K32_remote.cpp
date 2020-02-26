/*
  K32_remote.cpp
  Created by Clement GANGNEUX, february 2020.
  Released under GPL v3.0
*/

#include "Arduino.h"
#include "K32_remote.h"
#include <Wire.h>

/*
 *   PUBLIC
 */

K32_remote::K32_remote(const int BTN_PIN[NB_BTN]) {
  LOG("REMOTE: init");

  this->lock = xSemaphoreCreateMutex();

  /* Init I2C and Buttons pins */
  Wire.begin(BTN_PIN[0], BTN_PIN[1]);   // i2c pins
  this->mcp.begin();                  // i2c addr

  for (int i=0; i<NB_BTN; i++) {
    this->buttons[i].pin = i;
    this->buttons[i].state = LOW;

    this->mcp.pinMode(i, INPUT);
    this->mcp.pullUp(i, HIGH); 
  }

  // Start main task
  xTaskCreate( this->task,          // function
      "remote_task",       // task name
      5000,              // stack memory
      (void*)this,        // args
      0,                 // priority
      NULL);              // handler

  // Start read button state task
  xTaskCreate( this->read_btn_state,          // function
      "read_btn_task",       // task name
      5000,              // stack memory
      (void*)this,        // args
      0,                 // priority
      NULL);              // handler
};

void K32_remote::_lock()
{
  //LOG("lock");
  xSemaphoreTake(this->lock, portMAX_DELAY);
}

void K32_remote::_unlock()
{
  //LOG("unlock");
  xSemaphoreGive(this->lock);
}

void K32_remote::setMacroMax(int macroMax)
{
  
  this->_lock();
  this->_macroMax = macroMax;
  this->_unlock();
}

void K32_remote::setAuto()
{
  
  this->_lock();
  this->_state = REMOTE_AUTO;
  this->_unlock();
}

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

/*
 *   PRIVATE
 */




 void K32_remote::task(void * parameter) {
   K32_remote* that = (K32_remote*) parameter;
   TickType_t xFrequency = pdMS_TO_TICKS(REMOTE_CHECK);

   while(true) {
     /* Main loop */

     /* Check flags for each button */

     for (int i=0; i<NB_BTN; i++)
     {
       
       that->_lock();
       if (  that->buttons[i].flag == 1)
       {
         LOGF("REMOTE: Short push on button %d\n",i);

         /* Instructions for the different buttons */
         switch (i)
         {
           case 0 :                           // Button 1 : BlackOut
            that->_activeMacro = 0;
            if (that->_state == REMOTE_AUTO) that->_state = REMOTE_MANU;
            LOG("Blackout");
            break;
           case 1 :                          // Button 2 : Previous
            that->_previewMacro --;
            if (that->_previewMacro < 0 ) that->_previewMacro = that->_macroMax - 1 ;
            LOG("Preview --");
            break;
          case 2 :                          // Button 3 : Forward
            that->_previewMacro ++;
            if (that->_previewMacro >= that->_macroMax ) that->_previewMacro = 0 ;
            LOG("Preview ++");
            break;
          case 3 :                         // Button 4 : Go
            that->_activeMacro = that->_previewMacro;
            if (that->_state == REMOTE_AUTO) that->_state = REMOTE_MANU;
            LOG("Go");
            break;
         }

       } else if ( that->buttons[i].flag == 2) {
         LOGF("REMOTE: Long push on button %d\n",i);

         /* Instructions for the different buttons */
         switch (i)
         {
           case 0 :                           // Button 1 : BlackOut Forced
            that->_activeMacro = 0;
            if (that->_state == REMOTE_MANULOCK) that->_state = REMOTE_MANU;
            else that->_state = REMOTE_MANULOCK;
            break;
           case 1 :                          // Button 2 : lamp on/off
            if (that->_lamp == -1) that->_lamp = 255;
            else that->_lamp = -1;
            break;
          case 2 :                          // Button 3 : lamp on/off
            if (that->_lamp == -1) that->_lamp = 255;
            else that->_lamp = -1;
            break;
          case 3 :                         // Button 4 : Go Forced
            that->_activeMacro = that->_previewMacro;
            if (that->_state == REMOTE_MANULOCK) that->_state = REMOTE_MANU;
            else that->_state = REMOTE_MANULOCK;
            break;
         }
       }
       
       that->_unlock();
       that->buttons[i].flag = 0;
       yield();
     }


     /********/

     vTaskDelay( xFrequency );

   }
 }

void K32_remote::read_btn_state(void * parameter) {
  K32_remote* that = (K32_remote*) parameter;
   TickType_t xFrequency = pdMS_TO_TICKS(BTN_CHECK);
   unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers
   unsigned long longPushDelay = 1000; // Delay for a long push of the button


   while (true)
   {
     for (int i = 0 ; i < NB_BTN ; i++)
     {
       
       that->_lock();
       // read the state of the switch into a local variable:
       int reading = that->mcp.digitalRead(that->buttons[i].pin);

      // pushed
       if (reading == LOW)
       {
         if (that->buttons[i].state == HIGH)  // was released
         {
           that->buttons[i].state = LOW ;
           that->buttons[i].lastPushTime = millis(); // Record time of pushing button
         } 
         else  // was already pushed
         {
           if ((millis()-that->buttons[i].lastPushTime>longPushDelay)&&(that->buttons[i].lastPushTime!=0))
           {
             that->buttons[i].flag = 2; // Long push
             that->buttons[i].lastPushTime = 0; // Reset counter
           }
         }
       } 
       
       // Released
       else
       {
         if (that->buttons[i].state == LOW)  // was pushed
         {
           that->buttons[i].state = HIGH ;
           if ((millis()-that->buttons[i].lastPushTime>debounceDelay)&&(that->buttons[i].lastPushTime!=0))
           {
             that->buttons[i].flag = 1; // short push
           }
         }
       }
       that->_unlock();
     }

     vTaskDelay( xFrequency );
   }
 }
