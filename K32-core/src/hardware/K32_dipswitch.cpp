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

    this->dipswitch_read();
    
    xTaskCreate(this->dipswatch, // function
                "dips_watch",    // server name
                1500,            // stack memory
                (void *)this,    // args
                0,               // priority
                NULL             // handler
    );                           // core
}

/*
 *   PRIVATE
 */

void K32_dipswitch::dipswatch( void * parameter )
{
    K32_dipswitch *that = (K32_dipswitch *)parameter;
    TickType_t xFrequency = pdMS_TO_TICKS(10);

    while (true)
    {
        vTaskDelay(xFrequency);
        that->dipswitch_read();
    }

    vTaskDelete(NULL);
}

void K32_dipswitch::dipswitch_read()
{
    xSemaphoreTake(this->lock, portMAX_DELAY);
    uint8_t pin = DIP_PIN[k32->system->hw()];
    int value = analogRead(pin);
    if ((1650 < value) && (value < 1900))
    {
        this->dip[0] = true;
        this->dip[1] = false;
        this->dip[2] = false;
    }
    else if ((2000 < value) && (value < 2250))
    {
        this->dip[0] = false;
        this->dip[1] = true;
        this->dip[2] = false;
    }
    else if ((2280 < value) && (value < 2500))
    {
        this->dip[0] = false;
        this->dip[1] = false;
        this->dip[2] = true;
    }
    else if ((2600 < value) && (value < 2820))
    {
        this->dip[0] = true;
        this->dip[1] = true;
        this->dip[2] = false;
    }
    else if ((2850 < value) && (value < 2990))
    {
        this->dip[0] = true;
        this->dip[1] = false;
        this->dip[2] = true;
    }
    else if ((2990 < value) && (value < 3100))
    {
        this->dip[0] = false;
        this->dip[1] = true;
        this->dip[2] = true;
    }
    else if ((3200 < value) && (value < 3500))
    {
        this->dip[0] = true;
        this->dip[1] = true;
        this->dip[2] = true;
    }
    else
    {
        this->dip[0] = false;
        this->dip[1] = false;
        this->dip[2] = false;
    }
    if (old_dip[0] != dip[0] || old_dip[1] != dip[1] || old_dip[2] != dip[2])
        {
            old_dip[0] = dip[0];
            old_dip[1] = dip[1];
            old_dip[2] = dip[2];
            
            int v = 0;
            if (this->dip[0]) v += 1;
            if (this->dip[1]) v += 2;
            if (this->dip[2]) v += 4;

            Orderz *order = new Orderz("dipswitch", false);
            order->addData("value", v);
            order->addData("rawvalue", value);
            this->emit(order);
        }
    xSemaphoreGive(this->lock);
}

// mesure de la pin
// dip     = 0
// dip 1   = 1770-1780
// dip 2   = 2135-2142
// dip 3   = 2380-2390
// dip 12  = 2740-2750
// dip 1 3 = 2950-2960
// dip  23 = 2995-3005
// dip 123 = 3369-3376