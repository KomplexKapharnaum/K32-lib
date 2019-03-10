/*
  K32_stm32.cpp
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/

#include "Arduino.h"
#include "K32_stm32.h"

/*
 *   PUBLIC
 */

K32_stm32::K32_stm32() {
  this->lock = xSemaphoreCreateMutex();
  Serial.begin(115200, SERIAL_8N1);
  Serial.setTimeout(10);
  this->running = true;

  // Checking task
  xTaskCreatePinnedToCore( this->task, "stm32_task",
                1000,
                (void*)this,
                0,  // priority
                NULL,
                STM32_CORE);
};


void K32_stm32::leds(uint8_t *values) {
  int arg = 0;
  for (int i = 0; i < 6; i++)
    arg += values[i] * pow(10, i);
  this->send(K32_stm32_api::SET_LEDS, arg);
};


int K32_stm32::battery() {
  return this->_battery;
};


bool K32_stm32::clicked() {
  xSemaphoreTake(this->lock, portMAX_DELAY);
  bool click = this->_btn_click;
  this->_btn_click = false;
  xSemaphoreGive(this->lock);
  return click;
};


bool K32_stm32::dblclicked() {
  xSemaphoreTake(this->lock, portMAX_DELAY);
  bool click = this->_btn_dblclick;
  this->_btn_dblclick = false;
  xSemaphoreGive(this->lock);
  return click;
};


void K32_stm32::wait() {
  xSemaphoreTake(this->lock, portMAX_DELAY);
  xSemaphoreGive(this->lock);
}


void K32_stm32::reset() {
  xSemaphoreTake(this->lock, portMAX_DELAY);
  this->send(K32_stm32_api::SET_LOAD_SWITCH, 0);
  this->send(K32_stm32_api::REQUEST_RESET);
  delay(1000);
  Serial.println("STM did not reset, going with soft reset");
  WiFi.disconnect();
  delay(500);
  // Hard restart
  esp_task_wdt_init(1,true);
  esp_task_wdt_add(NULL);
  while(true);
  //
  xSemaphoreGive(this->lock);
}


void K32_stm32::shutdown() {
  xSemaphoreTake(this->lock, portMAX_DELAY);
  this->send(K32_stm32_api::SET_LOAD_SWITCH, 0);
  this->send(K32_stm32_api::SHUTDOWN);
  xSemaphoreGive(this->lock);
}


/*
 *   PRIVATE
 */

void K32_stm32::task( void * parameter ) {
  K32_stm32* that = (K32_stm32*) parameter;

  int tickerBattery = 0;
  int event;

  xSemaphoreTake(that->lock, portMAX_DELAY);
  that->send(K32_stm32_api::SET_LOAD_SWITCH, 1);
  xSemaphoreGive(that->lock);

  // loop
  while (true) {
    if (!that->running) break;

    // Lock STM32
    xSemaphoreTake(that->lock, portMAX_DELAY);

    // check Button
    event = that->get(K32_stm32_api::GET_BUTTON_EVENT);
    if (event == K32_stm32_api::BUTTON_DOUBLE_CLICK_EVENT) that->_btn_click = true;
    else if (event == K32_stm32_api::BUTTON_CLICK_EVENT) that->_btn_dblclick = true;

    // check Battery
    tickerBattery -= 1;
    if (tickerBattery <= 0) {
      int batt = that->get(K32_stm32_api::GET_BATTERY_PERCENTAGE);
      that->_battery = batt;
      tickerBattery = STM32_CHECK_BATT/STM32_CHECK;
    }

    // Unlock STM32
    xSemaphoreGive(that->lock);

    // sleep
    delay(STM32_CHECK);
  }
  vTaskDelete(NULL);
};


void K32_stm32::send(K32_stm32_api::CommandType cmd, int arg) {
  this->flush();
  Serial.write(K32_stm32_api::PREAMBLE);
  Serial.write(cmd);
  Serial.write(' ');
  Serial.println(arg);
}


void K32_stm32::send(K32_stm32_api::CommandType cmd) {
  this->flush();
  Serial.write(K32_stm32_api::PREAMBLE);
  Serial.write(cmd);
  Serial.println("");
}


long K32_stm32::get(K32_stm32_api::CommandType cmd) {
  this->flush();
  Serial.write(K32_stm32_api::PREAMBLE);
  Serial.write(cmd);
  Serial.println("");
  long answer = this->read();
  return answer;
}


void K32_stm32::flush() {
  while (Serial.available()) Serial.read();
}


long K32_stm32::read() {
  if (Serial.find(K32_stm32_api::PREAMBLE))
  {
    long arg = Serial.parseInt();
    return arg;
  }
  return 0;
}
