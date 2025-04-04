/*
  K32_dipswitch.cpp
  Created by RIRI, march 2025.
  Released under GPL v3.0
*/

#include "K32_version.h"
#include "K32_dipswitch.h"

/*
 *   PUBLIC
 */

K32_dipswitch::K32_dipswitch(K32 *k32) : K32_plugin("dips", k32)
{

    this->lock = xSemaphoreCreateMutex();

    xTaskCreate(this->dipswatch, // function
                "dips_watch",    // server name
                1500,            // stack memory
                (void *)this,    // args
                0,               // priority
                NULL             // handler
    );                           // core
}

void K32_dipswitch::add(int pin)
{
    this->add(pin, String(pin));
}

/*
 *   PRIVATE
 */

void K32_dipswitch::dipswatch(void *parameter)
{
    // K32_dipswitch *that = (K32_dipswitch *)parameter;
    TickType_t xFrequency = pdMS_TO_TICKS(10);

    while (true)
    {
        // int value = analogRead(DIP_PIN[k32->system->hw()]);
        // that->emit("dips/ value: " + value);
        int value = analogRead(34);
        if ((1650 < value) && (value < 1900))     LOG ("dip 1  "); 
                      else if ( (2000<value) && (value<2250) )  LOG ("dip  2 ");
                      else if ( (2280<value) && (value<2500) )  LOG ("dip   3");
                      else if ( (2600<value) && (value<2820) )  LOG ("dip 12 ");
                      else if ( (2850<value) && (value<2970) )  LOG ("dip 1 3");
                      else if ( (2980<value) && (value<3100) )  LOG ("dip  23");
                      else if ( (3200<value) && (value<3500) )  LOG ("dip 123"); 
      vTaskDelay( xFrequency );
    }

    vTaskDelete(NULL);
}

void K32_dipswitch::dipswitch_read(int pin, int value)
{
    xSemaphoreTake(this->lock, portMAX_DELAY);
    this->emit("dips/ " + String(pin) + ": " + String(value));
    xSemaphoreGive(this->lock);
}
    // this->emit("dips/ " + String(pin) + ": " + String(value));
    // this->k32->timer->every(10000, []()
    // {
    //     int value = analogRead(34);
    //     if ((1650 < value) && (value < 1900))     LOG ("dip 1  "); 
    // else if ( (2000<value) && (value<2250) )  LOG ("dip  2 ");
    // else if ( (2280<value) && (value<2500) )  LOG ("dip   3");
    // else if ( (2600<value) && (value<2820) )  LOG ("dip 12 ");
    // else if ( (2850<value) && (value<2970) )  LOG ("dip 1 3");
    // else if ( (2980<value) && (value<3100) )  LOG ("dip  23");
    // else if ( (3200<value) && (value<3500) )  LOG ("dip 123"); 
    // });
}

// while (true)
// {
//     k32->timer->every(10000, []()
//                       {
//         int value = analogRead(34);
//         if ((1650 < value) && (value < 1900))     LOG ("dip 1  "); 
// else if ( (2000<value) && (value<2250) )  LOG ("dip  2 ");
// else if ( (2280<value) && (value<2500) )  LOG ("dip   3");
// else if ( (2600<value) && (value<2820) )  LOG ("dip 12 ");
// else if ( (2850<value) && (value<2970) )  LOG ("dip 1 3");
// else if ( (2980<value) && (value<3100) )  LOG ("dip  23");
// else if ( (3200<value) && (value<3500) )  LOG ("dip 123"); 
// }

// mesure de la pin
// dip     = 0
// dip 1   = 1770-1780
// dip 2   = 2135-2142
// dip 3   = 2380-2390
// dip 12  = 2740-2750
// dip 1 3 = 2950-2960
// dip  23 = 2995-3005
// dip 123 = 3369-3376