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

  byte error, address, nDevices;
  nDevices = 0;
  for(address = 1; address < 127; address++ )
  {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0)
    {
      LOGINL("I2C device found at address 0x");
      if (address<16) LOGINL("0");
      LOGINL(address);
      LOG("  !");

      nDevices++;
    }
    else if (error==4)
    {
      LOGINL("Unknown error at address 0x");
      if (address<16)
        LOGINL("0");
      LOG(address);
    }    
  }
  if (nDevices == 0) LOG("No I2C devices found");
  else LOG("I2C scan done");

  this->mcp->begin(0);                  // i2c addr

  for (int i=0; i<NB_BTN; i++) {
    this->buttons[i].pin = i;
    this->buttons[i].state = LOW;

    this->mcp->pinMode(i, INPUT);
    this->mcp->pullUp(i, HIGH); 
  }

  // Start main task
  xTaskCreate( this->task,          // function
      "remote_task",       // task name
      1000,              // stack memory
      (void*)this,        // args
      0,                 // priority
      NULL);              // handler

  // Start read button state task
  xTaskCreate( this->read_btn_state,          // function
      "read_btn_task",       // task name
      1000,              // stack memory
      (void*)this,        // args
      0,                 // priority
      NULL);              // handler
};

void K32_remote::setMacroMax(int macroMax)
{
  xSemaphoreTake(this->lock, portMAX_DELAY);
  this->_macroMax = macroMax;
  xSemaphoreGive(this->lock);
}

void K32_remote::setAuto()
{
  xSemaphoreTake(this->lock, portMAX_DELAY);
  this->_state = REMOTE_AUTO;
  xSemaphoreGive(this->lock);
}

remoteState K32_remote::getState()
{
  xSemaphoreTake(this->lock, portMAX_DELAY);
  remoteState data = this->_state;
  xSemaphoreGive(this->lock);
  return data;
}


int K32_remote::getActiveMacro()
{
  xSemaphoreTake(this->lock, portMAX_DELAY);
  int data = this->_activeMacro;
  xSemaphoreGive(this->lock);
  return data;
}

int K32_remote::getPreviewMacro()
{ 
  xSemaphoreTake(this->lock, portMAX_DELAY);
  int data = this->_previewMacro;
  xSemaphoreGive(this->lock);
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
       if (  that->buttons[i].flag == 1)
       {
         LOGF("REMOTE: Short push on button %d\n",i);

         /* Instructions for the different buttons */
         xSemaphoreTake(that->lock, portMAX_DELAY);
         switch (i)
         {
           case 0 :                           // Button 1 : BlackOut
            that->_activeMacro = 0;
            if (that->_state == REMOTE_AUTO) that->_state = REMOTE_MANU;
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
            break;
         }
         xSemaphoreGive(that->lock);

       } else if ( that->buttons[i].flag == 2) {
         LOGF("REMOTE: Long push on button %d\n",i);

         /* Instructions for the different buttons */
         xSemaphoreTake(that->lock, portMAX_DELAY);
         switch (i)
         {
           case 0 :                           // Button 1 : BlackOut Forced
            that->_activeMacro = 0;
            that->_state = REMOTE_MANULOCK;
            break;
           case 1 :                          // Button 2 : Previous
            break;
          case 2 :                          // Button 3 : Forward
            break;
          case 3 :                         // Button 4 : Go Forced
            that->_activeMacro = that->_previewMacro;
            that->_state = REMOTE_MANULOCK;
            break;
         }
         xSemaphoreGive(that->lock);
       }
       that->buttons[i].flag = 0;

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
       
       // read the state of the switch into a local variable:
       int reading = that->mcp->digitalRead(that->buttons[i].pin);

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

     }

     vTaskDelay( xFrequency );
   }
 }
