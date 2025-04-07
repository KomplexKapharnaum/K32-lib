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

void K32_dipswitch::dipswatch(void *parameter)
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
    if (3400 < value)
    {
        this->dip[0] = true;
        this->dip[1] = true;
        this->dip[2] = true;
    } // dip 123
    else if (2900 < value)
    {
        this->dip[0] = true;
        this->dip[1] = false;
        this->dip[2] = true;
    } // dip 1_3
    else if (2750 < value)
    {
        this->dip[0] = true;
        this->dip[1] = true;
        this->dip[2] = false;
    } // dip 12_
    else if (2450 < value)
    {
        this->dip[0] = false;
        this->dip[1] = false;
        this->dip[2] = true;
    } // dip __3
    else if (2150 < value)
    {
        this->dip[0] = false;
        this->dip[1] = true;
        this->dip[2] = false;
    } // dip _2_
    else if(1650 < value)
    {
        this->dip[0] = true;
        this->dip[1] = false;
        this->dip[2] = false;
    } // dip 1__
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
        if (this->dip[0])
            v += 1;
        if (this->dip[1])
            v += 2;
        if (this->dip[2])
            v += 4;

        LOGF4("DIP: %d %d %d / RAW = %d\n", this->dip[0], this->dip[1], this->dip[2], value);

        Orderz *order = new Orderz("dipswitch", false);
        order->addData(v);
        order->addData(value);
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