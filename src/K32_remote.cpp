/*
  K32_remote.cpp
  Created by Clement GANGNEUX, february 2020.
  Released under GPL v3.0
*/

#include "Arduino.h"
#include "K32_remote.h"

/* Definition of GPÏO Ports */
#if HW_REVISION == 1
const uint8_t* BTN_PIN[NB_BTN] = {32, 33, 25, 26};


  // const uint8_t BTN1_BLACK_PIN = 32;
  // const uint8_t BTN2_PREV_PIN  = 33;
  // const uint8_t BTN3_FWD_PIN   = 25;
  // const uint8_t BTN4_GO_PIN    = 26;

#elif HW_REVISION == 2
const uint8_t BTN_PIN[NB_BTN] = {32, 33, 25, 26}; // Button1 = BlackOut ; Button2 = Previous ; Button3 = Forward ; Button4 = Go

#else
  #error "HW_REVISION undefined or invalid. Should be 1 or 2"
#endif
/**********/

K32_remote::K32_remote() {
  LOG("REMOTE: init");

  this->lock = xSemaphoreCreateMutex();
  /* Init Buttons pins */
  for (int i = 0 ; i < NB_BTN; i++)
  {
    this->buttons[i].pin = BTN_PIN[i];
    this->buttons[i].state = LOW;
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
     } else if ( that->buttons[0].flag == 2) {

       /* code */
     }
     if (  that->buttons[2].flag == 1)
     {
       that->macroIndex ++;
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

       if (reading == HIGH)
       {
         if (that->buttons[i].state == LOW)
         {
           that->buttons[i].state = HIGH ;
           that->buttons[i].lastPushTime = millis(); // Record time of pushing button
         } else
         {
           if (that->buttons[i].lastPushTime>longPushDelay)
           {
             that->buttons[i].flag = 2; // Long push
             that->buttons[i].lastPushTime = millis(); // Reset counter
           }
         }
       } else
       {
         if (that->buttons[i].state == HIGH)
         {
           that->buttons[i].state = LOW ;
           if (that->buttons[i].lastPushTime>debounceDelay)
           {
             that->buttons[i].flag = 1; // short push
           }
         }
       }
     }

     vTaskDelay( xFrequency );
   }
 }
