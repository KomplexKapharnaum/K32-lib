/*
K32_modulo.cpp
Created by RIRI, Mars 2020.
Released under GPL v3.0
*/

#include "Arduino.h"
#include "K32_modulo.h"

// K32_MODULO

K32_modulo::K32_modulo()
{
    this->critical = xSemaphoreCreateMutex();
    isRunning = false;
    freezeTime = 0;
}

void K32_modulo::set(int k, int value)
{
    this->params[k] = value;
}

int K32_modulo::getValue()
{
    return 0;
}

void K32_modulo::play()
{
    isRunning = true;
    freezeTime = 0;
}

void K32_modulo::pause()
{
    freezeTime = millis();
}

void K32_modulo::stop()
{
    isRunning = false;
}

// K32_MODULO_SINUS

K32_modulo_sinus::K32_modulo_sinus(int period, int min, int max)
{

    this->params[0] = period; //period
    this->params[1] = max;    //value max
    this->params[2] = min;    //value min
}

int K32_modulo_sinus::getValue()
{

    unsigned long time;
    if (freezeTime == 0)
        time = millis();
    else
        time = freezeTime;

    return ((0.5f + 0.5f * sin(2 * PI * time / this->params[0] - 0.5f * PI)) * (this->params[1] - this->params[2]) + this->params[2]);
}

// K32_MODULO_BUMP

K32_modulo_bump::K32_modulo_bump(int period, int min, int max)
{

    this->params[0] = period; //period
    this->params[1] = max;    //value max
    this->params[2] = min;    //value min
}

int K32_modulo_bump::getValue()
{

    unsigned long time;
    if (freezeTime == 0)
        time = millis();
    else
        time = freezeTime;

    if (time - period_last > this->params[0] / 2)
        period_last = time;

    return ((0.5f + 0.5f * sin(2 * PI * (time - period_last) / this->params[0] - 0.5f * PI)) * (this->params[1] - this->params[2]) + this->params[2]);
}

// K32_MODULO_RANDOM

K32_modulo_random::K32_modulo_random(int min, int max)
{

    this->params[0] = min; //value min
    this->params[1] = max; //value max
}

int K32_modulo_random::getValue()
{

    return random(this->params[0], this->params[1]);
}

// K32_MODULO_LINPLUS

K32_modulo_linplus::K32_modulo_linplus(int period, int min, int max)
{

    this->params[0] = period; //period
    this->params[1] = max;    //value max
    this->params[2] = min;    //value min
}

int K32_modulo_linplus::getValue()
{

    unsigned long time;

    if (freezeTime == 0)
        time = millis();
    else
        time = freezeTime;

    if (time - period_last > this->params[0])
        period_last = time;

    return ((((time - period_last) * (this->params[1] - this->params[2])) / this->params[0]) + this->params[2]);
}

// K32_MODULO_LINMOINS

K32_modulo_linmoins::K32_modulo_linmoins(int period, int min, int max)
{

    this->params[0] = period; //period
    this->params[1] = max;    //value max
    this->params[2] = min;    //value min
}

int K32_modulo_linmoins::getValue()
{

    unsigned long time;

    if (freezeTime == 0)
        time = millis();
    else
        time = freezeTime;

    if (time - period_last > this->params[0])
        period_last = time;

    return map(((((time - period_last) * (this->params[1] - this->params[2])) / this->params[0]) + this->params[2]), this->params[2], this->params[1], this->params[1], this->params[2]);
}

// K32_MODULO_ONOFF

K32_modulo_onoff::K32_modulo_onoff(int period, int min, int max)
{

    this->params[0] = period / 2; //period
    this->params[1] = max;        //value max
    this->params[2] = min;        //value min
}

int K32_modulo_onoff::getValue()
{

    unsigned long time;

    if (freezeTime == 0)
        time = millis();
    else
        time = freezeTime;

    if (time - period_last > this->params[0])
    {
        period_last = time;
        if (period_cycle == false)
            period_cycle = true;
        else
            period_cycle = false;
    }
    if (period_cycle == false)
        return this->params[2];
    else
        return this->params[1];
}

// K32_MODULO_TRIPLUS

K32_modulo_triplus::K32_modulo_triplus(int period, int min, int max)
{

    this->params[0] = period / 2; //period
    this->params[1] = max;        //value max
    this->params[2] = min;        //value min
}

int K32_modulo_triplus::getValue()
{

    unsigned long time;

    if (freezeTime == 0)
        time = millis();
    else
        time = freezeTime;

    if (time - period_last > this->params[0])
    {
        period_last = time;
        if (period_cycle == false)
            period_cycle = true;
        else
            period_cycle = false;
    }
    if (period_cycle == false)
        return ((((time - period_last) * (this->params[1] - this->params[2])) / this->params[0]) + this->params[2]);
    else
        return map(((((time - period_last) * (this->params[1] - this->params[2])) / this->params[0]) + this->params[2]), this->params[2], this->params[1], this->params[1], this->params[2]);
}

// K32_MODULO_TRIMOINS

K32_modulo_trimoins::K32_modulo_trimoins(int period, int min, int max)
{

    this->params[0] = period / 2; //period
    this->params[1] = max;        //value max
    this->params[2] = min;        //value min
}

int K32_modulo_trimoins::getValue()
{

    unsigned long time;

    if (freezeTime == 0)
        time = millis();
    else
        time = freezeTime;

    if (time - period_last > this->params[0])
    {
        period_last = time;
        if (period_cycle == false)
            period_cycle = true;
        else
            period_cycle = false;
    }
    if (period_cycle == false)
        return map(((((time - period_last) * (this->params[1] - this->params[2])) / this->params[0]) + this->params[2]), this->params[2], this->params[1], this->params[1], this->params[2]);
    else
        return ((((time - period_last) * (this->params[1] - this->params[2])) / this->params[0]) + this->params[2]);
}

// K32_MODULO_PHASE

K32_modulo_phase::K32_modulo_phase(int period, int min, int max)
{

    this->params[0] = period; //period
    this->params[1] = max;    //value max
    this->params[2] = min;    //value min
}

int K32_modulo_phase::getValue_1()
{

    unsigned long time;
    if (freezeTime == 0)
        time = millis();
    else
        time = freezeTime;

    return ((0.5f + 0.5f * sin(2 * (3 * PI / 4) * time / this->params[0] - 0.5f * (3 * PI / 4))) * (this->params[1] - this->params[2]) + this->params[2]);
}

int K32_modulo_phase::getValue_2()
{

    unsigned long time;
    if (freezeTime == 0)
        time = millis();
    else
        time = freezeTime;

    return ((0.5f + 0.5f * sin(2 * (5 * PI / 4) * time / this->params[0] - 0.5f * (5 * PI / 4))) * (this->params[1] - this->params[2]) + this->params[2]);
}

int K32_modulo_phase::getValue_3()
{

    unsigned long time;
    if (freezeTime == 0)
        time = millis();
    else
        time = freezeTime;

    return ((0.5f + 0.5f * sin(2 * (7 * PI / 4) * time / this->params[0] - 0.5f * (7 * PI / 4))) * (this->params[1] - this->params[2]) + this->params[2]);
}

int K32_modulo_phase::getValue_4()
{

    unsigned long time;
    if (freezeTime == 0)
        time = millis();
    else
        time = freezeTime;

    return ((0.5f + 0.5f * sin(2 * (9 * PI / 4) * time / this->params[0] - 0.5f * (9 * PI / 4))) * (this->params[1] - this->params[2]) + this->params[2]);
}

// K32_MODULO_FADEIN

K32_modulo_fadein::K32_modulo_fadein(int period, int min, int max)
{

    this->params[0] = period; //fade time
    this->params[1] = min;    //value start
    this->params[2] = max;    //value end
}

int K32_modulo_fadein::getValue()
{

    unsigned long time;

    if (freezeTime == 0)
        time = millis();
    else
        time = freezeTime;

    if (time - period_last >= this->params[0])
    {
        // freezeTime = time;
        LOG(" freezzeeee  ");
    }
    if (time - period_last > this->params[0])
    {
        period_last = time;
    }

    if ((this->params[1]) < (this->params[2]))
    {
        LOG(" 1<<<<<<2 ");
        LOGINL(" p1 = ");
        LOG(this->params[1]);
        LOGINL(" p2  = ");
        LOG(this->params[2]);
        LOGINL(" p2 - p1 = ");
        LOG((this->params[2] - this->params[1]));
        LOGINL(" this->params[0] = ");
        LOG(this->params[0]);

        fact = (this->params[2] - this->params[1]) / this->params[0];
        value = this->params[1] + ((time - period_last) * fact);
    }
    else if ((this->params[2]) < (this->params[1]))
    {
        LOG(" 1>>>>>>2  ");
        fact = (this->params[1] - this->params[2]) / (this->params[0] * 1.0);
        value = this->params[1] - ((time - period_last) * fact);
    }
    else if (this->params[1] == this->params[2])
    {
        LOG(" =========  ");
        fact = 0;
        value = this->params[1];
    }

    LOGINL(" time - periode = ");
    LOG(time - period_last);
    LOGINL(" fact = ");
    LOG(fact);
    LOGINL(" value = ");
    LOG(value);

    return value;
}