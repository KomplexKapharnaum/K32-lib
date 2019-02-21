/*
  kesp_stm32.cpp
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/

#include "Arduino.h"
#include "KESP_STM32.h"

/*
 *   PUBLIC
 */

KESP_STM32::KESP_STM32() {
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


void KESP_STM32::leds(uint8_t *values) {
  int arg = 0;
  for (int i = 0; i < 6; i++)
    arg += values[i] * pow(10, i);
  this->send(KESP_STM32_API::SET_LEDS, arg);
};


int KESP_STM32::battery() {
  return this->_battery;
};


bool KESP_STM32::clicked() {
  xSemaphoreTake(this->lock, portMAX_DELAY);
  bool click = this->_btn_click;
  this->_btn_click = false;
  xSemaphoreGive(this->lock);
  return click;
};


bool KESP_STM32::dblclicked() {
  xSemaphoreTake(this->lock, portMAX_DELAY);
  bool click = this->_btn_dblclick;
  this->_btn_dblclick = false;
  xSemaphoreGive(this->lock);
  return click;
};


void KESP_STM32::wait() {
  xSemaphoreTake(this->lock, portMAX_DELAY);
  xSemaphoreGive(this->lock);
}


void KESP_STM32::reset() {
  xSemaphoreTake(this->lock, portMAX_DELAY);
  this->send(KESP_STM32_API::SET_LOAD_SWITCH, 0);
  this->send(KESP_STM32_API::REQUEST_RESET);
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


void KESP_STM32::shutdown() {
  xSemaphoreTake(this->lock, portMAX_DELAY);
  this->send(KESP_STM32_API::SET_LOAD_SWITCH, 0);
  this->send(KESP_STM32_API::SHUTDOWN);
  xSemaphoreGive(this->lock);
}


/*
 *   PRIVATE
 */

void KESP_STM32::task( void * parameter ) {
  KESP_STM32* that = (KESP_STM32*) parameter;

  int tickerBattery = 0;
  int event;

  xSemaphoreTake(that->lock, portMAX_DELAY);
  that->send(KESP_STM32_API::SET_LOAD_SWITCH, 1);
  xSemaphoreGive(that->lock);

  // loop
  while (true) {
    if (!that->running) break;

    // Lock STM32
    xSemaphoreTake(that->lock, portMAX_DELAY);

    // check Button
    event = that->get(KESP_STM32_API::GET_BUTTON_EVENT);
    if (event == KESP_STM32_API::BUTTON_DOUBLE_CLICK_EVENT) that->_btn_click = true;
    else if (event == KESP_STM32_API::BUTTON_CLICK_EVENT) that->_btn_dblclick = true;

    // check Battery
    tickerBattery -= 1;
    if (tickerBattery <= 0) {
      int batt = that->get(KESP_STM32_API::GET_BATTERY_PERCENTAGE);
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


void KESP_STM32::send(KESP_STM32_API::CommandType cmd, int arg) {
  this->flush();
  Serial.write(KESP_STM32_API::PREAMBLE);
  Serial.write(cmd);
  Serial.write(' ');
  Serial.println(arg);
}


void KESP_STM32::send(KESP_STM32_API::CommandType cmd) {
  this->flush();
  Serial.write(KESP_STM32_API::PREAMBLE);
  Serial.write(cmd);
  Serial.println("");
}


long KESP_STM32::get(KESP_STM32_API::CommandType cmd) {
  this->flush();
  Serial.write(KESP_STM32_API::PREAMBLE);
  Serial.write(cmd);
  Serial.println("");
  long answer = this->read();
  return answer;
}


void KESP_STM32::flush() {
  while (Serial.available()) Serial.read();
}


long KESP_STM32::read() {
  if (Serial.find(KESP_STM32_API::PREAMBLE))
  {
    long arg = Serial.parseInt();
    return arg;
  }
  return 0;
}
