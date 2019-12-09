/*
  K32_power.cpp
  Created by Clement GANGNEUX, june 2019.
  Released under GPL v3.0
*/

#include "Arduino.h"
#include "K32_power.h"

/*
 *   PUBLIC
 */

 K32_power::K32_power(K32_stm32* stm32) {
   this->lock = xSemaphoreCreateMutex();
   this->_stm32 = stm32 ;
   this->charge = true;
 };


 void K32_power::start() {



   xTaskCreate( this->task,
                 "power_task",
                 1000,
                 (void*)this,
                 1,              // priority
                 &t_handle);
  running = true;
 }

 void K32_power::stop() {
   if (running)
   {
     vTaskDelete(this->t_handle);
     running = false;
   }
 }

 int K32_power::power() {
   return 3;
 }



 /*
  *   PRIVATE
  */

  void K32_power::task(void * parameter) {
    K32_power* that = (K32_power*) parameter;
    TickType_t xFrequency = pdMS_TO_TICKS(POWER_CHECK);
    // bool Cell[40];
    // int randomValue;
    that->SOC = 50;

    /* Init Epd Cells and fill EPD screen */
    // for (int i=0; i<40; i++)
    // {
    //   Cell[i]=1;
    // }
    int numberOfIcon = (int)(that->SOC*4/10);

    /* EPD Init */
    xSemaphoreTake(that->lock, portMAX_DELAY);
    epd_init();
    epd_set_memory(MEM_TF);
    FillScreen(numberOfIcon);
    xSemaphoreTake(that->lock, portMAX_DELAY);
    //epd_enter_stopmode();



    while(true) {

      if (that->charge)
      {
        that->SOC += 5;
        if (that->SOC > 100)
        {
          that->SOC =100 ;
        }





      } else
      {
        that->SOC -= 5 ;
        if (that->SOC < 0) {
          that->SOC = 0;
        }
      }


      /* EPD Routine */
      numberOfIcon = (int)(that->_stm32->battery()*4/10);
      numberOfIcon = (int)(that->SOC*4/10);
      LOGF("Nb of icon : %d\n", numberOfIcon);
      xSemaphoreTake(that->lock, portMAX_DELAY);
      FillScreen(numberOfIcon);
      xSemaphoreTake(that->lock, portMAX_DELAY);
      //epd_enter_stopmode();




      // if ((int)(that->_stm32->battery()*4/10) > numberOfIcon)
      // {
      //   /* Add an icon to the big picture*/
      //   do{randomValue=random(40);} while (Cell[randomValue]);
      //   Cell[randomValue] = 1;
      //    numberOfIcon ++;
      //    DrawPic(random(1,7), randomValue);
      //    LOGF("Add icon %d \n", randomValue);
      //  }
      //  else if ((int)(that->_stm32->battery()*4/10) < numberOfIcon)
      //  {
      //    /* Remove an icon */
      //    do{randomValue=random(40);} while (!Cell[randomValue]);
      //    Cell[randomValue] = 0;
      //     numberOfIcon --;
      //     LOGF("Remove icon %d \n", randomValue);
      //     DrawPic(0, randomValue);
      //  }
      //  else
      //  {
      //    /* Add and remove an icon */
      //    do{randomValue=random(40);} while (!Cell[randomValue]);
      //    Cell[randomValue] = 0;
      //     numberOfIcon --;
      //     //LOGF("Remove icon %d \n", randomValue);
      //     DrawPic(0, randomValue);
      //     do{randomValue=random(40);} while (Cell[randomValue]);
      //     Cell[randomValue] = 1;
      //      numberOfIcon ++;
      //      //LOGF("Add icon %d \n", randomValue);
      //      DrawPic(random(1,7), randomValue);
      //      LOGF("Nb of icon : %d\n", numberOfIcon);
      //      // for (int i=0; i<40; i++)
      //      // {
      //      //   LOG(Cell[i])
      //      /* Verify Nb of icon */
      //
      //      //LOGF("New nb of icon : %d\n", numberOfIcon);
      //
       //}

      vTaskDelay( xFrequency );
    }
  }
