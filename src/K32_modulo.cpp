/*
K32_modulo.cpp
Created by RIRI, Mars 2020.
Released under GPL v3.0
*/

#include "Arduino.h"
#include "K32_modulo.h"

// K32_MODULO

K32_modulo::K32_modulo() {
    this->critical = xSemaphoreCreateMutex();
    isRunning = false;
    freezeTime = 0;
}

void K32_modulo::set(int k, int value) {
    this->params[k] = value;
}

int K32_modulo::getValue() {
    return 0;
}

void K32_modulo::play() {
    isRunning = true;
    freezeTime = 0;
}

void K32_modulo::pause() {
    freezeTime = millis();
}

void K32_modulo::stop() {
    isRunning = false;
}

// K32_MODULO_SINUS

K32_modulo_sinus::K32_modulo_sinus(int period, int min, int max) {

    this->params[0] = period;    //period
    this->params[1] = max;       //value max
    this->params[2] = min;       //value min
}

int K32_modulo_sinus::getValue() {

    unsigned long time;
    if (freezeTime == 0) time = millis();
    else time = freezeTime;

    return ((0.5f + 0.5f * sin( 2 * PI * time / this->params[0] - 0.5f * PI ) ) * (this->params[1]-this->params[2]) + this->params[2]);

}

K32_modulo_random::K32_modulo_random(int min,int max) {

    this->params[0] = min;      //value min
    this->params[1] = max;      //value max
}

int K32_modulo_random::getValue() {

    return random(this->params[0],this->params[1]);
}

K32_modulo_linplus::K32_modulo_linplus(int period, int min, int max) {

    this->params[0] = period;    //period
    this->params[1] = max;       //value max
    this->params[2] = min;       //value min
}

int K32_modulo_linplus::getValue() {

    unsigned long time;
    
    if (freezeTime == 0) time = millis();
    else time = freezeTime;

    if (time - period_last > this->params[0]) period_last = time ;

    return ((((time - period_last) * (this->params[1] - this->params[2])) / this->params[0])+this->params[2]);
}
