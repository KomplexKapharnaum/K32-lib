/*
  K32_pwm.cpp
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/

#include "Arduino.h"
#include "K32_pwm.h"

K32_pwm::K32_pwm()
{
  for(int k=0; k<PWM_MAXCHANNELS; k++) this->chanState[k] = 0;
};

void K32_pwm::attach(const int PIN)
{ 
  if (this->chanNumber >= PWM_MAXCHANNELS) return;
  ledcSetup(this->chanNumber, PWM_FREQUENCY, PWM_RESOLUTION);
  ledcAttachPin(PIN, this->chanNumber);
  ledcWrite(this->chanNumber, 0);
  this->chanNumber += 1;
}

K32_pwm *K32_pwm::blackout()
{
  this->setAll(0);
  return this;
}

K32_pwm *K32_pwm::setAll(int value)
{
  for(int k=0; k<this->chanNumber; k++) this->set(k, value);
  return this;
}

K32_pwm *K32_pwm::set(int channel, int value)
{ 
  if (channel < this->chanNumber) {
    this->chanState[channel] = value;
    ledcWrite(channel, value);
  }
  return this;
}

int K32_pwm::get(int channel)
{ 
  return this->chanState[channel];
}

