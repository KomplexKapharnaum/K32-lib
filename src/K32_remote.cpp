/*
  K32_remote.cpp
  Created by Clement GANGNEUX, february 2020.
  Released under GPL v3.0
*/

#include "Arduino.h"
#include "K32_remote.h"
#include "Adafruit_MCP23017.h"
#include <Wire.h>

/*
 *   PUBLIC
 */

K32_remote::K32_remote(const int BTN_PIN[NB_BTN]) {
  LOG("REMOTE: init");

  this->lock = xSemaphoreCreateMutex();
  /* Init I2C and Buttons pins */
 // Adafruit_MCP23017 mcp;
 //
 //  mcp.begin();

  for (int i = 0 ; i < NB_BTN; i++)
  {
    this->buttons[i].pin = BTN_PIN[i];
    this->buttons[i].state = LOW;
    pinMode(buttons[i].pin, INPUT_PULLUP);
  }

  // Start main task
  xTaskCreate( this->task,          // function
      "remote_task",       // task name
      1000,              // stack memory
      (void*)this,        // args
      3,                 // priority
      NULL);              // handler

// Start read button state task
xTaskCreate( this->read_btn_state,          // function
    "read_btn_task",       // task name
    1000,              // stack memory
    (void*)this,        // args
    3,                 // priority
    NULL);              // handler
};

void K32_remote::setMacroNb(int macroNb)
{
  this->_macroNb = macroNb;
}

void K32_remote::setAuto()
{

}

remoteState K32_remote::getState()
{
  return this->_state;
}


int K32_remote::getActiveMacro()
{
  return this->_activeMacro;
}

int K32_remote::getPreviewMacro()
{
  return this->_previewMacro;
}


/*
 *   PRIVATE
 */




 void K32_remote::task(void * parameter) {
   K32_remote* that = (K32_remote*) parameter;
   TickType_t xFrequency = pdMS_TO_TICKS(REMOTE_CHECK);

   while(true) {
     /* Main loop */

     /* Check flogs for each button */

     for (int i=0; i<NB_BTN; i++)
     {
       if (  that->buttons[i].flag == 1)
       {
         LOGF("REMOTE: Short push on button %d\n",i);

         /* Instructions for the different buttons */
         switch (i)
         {
           case 0 :                           // Button 1 : BlackOut
            break;
           case 1 :                          // Button 2 : Previous
            that->_previewMacro --;
            if (that->_previewMacro < 0 ) that->_previewMacro = that->_macroNb - 1 ;
            LOG("Preview --");
            break;
          case 2 :                          // Button 3 : Forward
            that->_previewMacro ++;
            if (that->_previewMacro >= that->_macroNb ) that->_previewMacro = 0 ;
            LOG("Preview ++");
            break;
          case 3 :                         // Button 4 : Go
            break;

         }
       } else if ( that->buttons[i].flag == 2) {
         LOGF("REMOTE: Long push on button %d\n",i);

         /* Instructions for the different buttons */
         switch (i)
         {
           case 0 :                           // Button 1 : BlackOut
            break;
           case 1 :                          // Button 2 : Previous
            break;
          case 2 :                          // Button 3 : Forward
            break;
          case 3 :                         // Button 4 : Go
            break;

         }

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
   int lastButtonState = LOW ;
   unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
   unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers
   unsigned long longPushDelay = 1000; // Delay for a long push of the button


   while (true)
   {
     for (int i = 0 ; i < NB_BTN ; i++)
     {
             // read the state of the switch into a local variable:
       int reading = digitalRead(that->buttons[i].pin);

       if (reading == LOW)
       {
         if (that->buttons[i].state == HIGH)
         {
           that->buttons[i].state = LOW ;
           that->buttons[i].lastPushTime = millis(); // Record time of pushing button
         } else
         {
           if ((millis()-that->buttons[i].lastPushTime>longPushDelay)&&(that->buttons[i].lastPushTime!=0))
           {
             that->buttons[i].flag = 2; // Long push
             that->buttons[i].lastPushTime = 0; // Reset counter
           }
         }
       } else
       {
         if (that->buttons[i].state == LOW)
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
