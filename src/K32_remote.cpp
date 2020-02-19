/*
  K32_remote.cpp
  Created by Clement GANGNEUX, february 2020.
  Released under GPL v3.0
*/

#include "Arduino.h"
#include "K32_remote.h"



K32_remote::K32_remote(const int BTN_PIN[NB_BTN]) {
  LOG("REMOTE: init");

  this->lock = xSemaphoreCreateMutex();
  /* Init Buttons pins */
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
  this->macroNb = macroNb;
}

/*
 *   PRIVATE
 */




 void K32_remote::task(void * parameter) {
   K32_remote* that = (K32_remote*) parameter;
   TickType_t xFrequency = pdMS_TO_TICKS(REMOTE_CHECK);

   while(true) {
     /* Main loop */

     /* Check Prev and Fwd button */
     if (  that->buttons[0].flag == 1)
     {
       that->macroIndex --;
       LOG("REMOTE: Short push on button 0");
     } else if ( that->buttons[0].flag == 2) {
       LOG("REMOTE: Long push on button 0");

     }
     that->buttons[0].flag = 0;
     if (  that->buttons[1].flag == 1)
     {
       that->macroIndex ++;
       LOG("REMOTE: Short push on button 1");

     } else if ( that->buttons[1].flag == 2)
     {
     LOG("REMOTE: Long push on button 1");
     }
     that->buttons[1].flag = 0;




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
           if (millis()-that->buttons[i].lastPushTime>longPushDelay)
           {
             that->buttons[i].flag = 2; // Long push
             that->buttons[i].lastPushTime = millis(); // Reset counter
           }
         }
       } else
       {
         if (that->buttons[i].state == LOW)
         {
           that->buttons[i].state = HIGH ;
           if (millis()-that->buttons[i].lastPushTime>debounceDelay)
           {
             that->buttons[i].flag = 1; // short push
           }
         }
       }
     }

     vTaskDelay( xFrequency );
   }
 }
