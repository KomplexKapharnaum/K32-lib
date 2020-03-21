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
  
  this->listen(true, true);
};

void K32_stm32::listen() {
  // Checking task
  xTaskCreate( this->task,
                "stm32_task",
                1000,
                (void*)this,
                0,              // priority
                NULL);
}

void K32_stm32::listen(bool btn, bool battery) {
  this->_btn_listen = btn;
  this->_batt_listen = battery;
  this->listen();
}

void K32_stm32::leds(uint8_t *values) {
  int arg = 0;
  for (int i = 0; i < 6; i++)
    arg += values[i] * pow(10, i);
  this->send(K32_stm32_api::SET_LEDS, arg);
};

void K32_stm32::gauge(int percent) {
  this->send(K32_stm32_api::SET_LED_GAUGE, percent);
};

void K32_stm32::blink(uint8_t *values, int duration_ms)
{
  this->_blink_leds = values;
  this->_blink_duration = duration_ms;
  xTaskCreate( this->blink_task,
                "blink_task",
                1000,
                (void*)this,
                0,              // priority
                NULL);
}

void K32_stm32::custom(int Ulow, int U1, int U2, int U3, int U4, int U5, int Umax) {
  this->send(K32_stm32_api::SET_BATTERY_VOLTAGE_LOW, Ulow);
  this->send(K32_stm32_api::SET_BATTERY_VOLTAGE_1, U1);
  this->send(K32_stm32_api::SET_BATTERY_VOLTAGE_2, U2);
  this->send(K32_stm32_api::SET_BATTERY_VOLTAGE_3, U3);
  this->send(K32_stm32_api::SET_BATTERY_VOLTAGE_4, U4);
  this->send(K32_stm32_api::SET_BATTERY_VOLTAGE_5, U5);
  this->send(K32_stm32_api::SET_BATTERY_VOLTAGE_6, Umax);
};


int K32_stm32::firmware() {
  return this->get(K32_stm32_api::GET_FW_VERSION);
};

int K32_stm32::current() {
  return this->get(K32_stm32_api::GET_LOAD_CURRENT);
};

int K32_stm32::battery() {
  xSemaphoreTake(this->lock, portMAX_DELAY);
  int battLevel = this->_battery;
  xSemaphoreGive(this->lock);
  return battLevel;
};

int K32_stm32::voltage() {
  return this->get(K32_stm32_api::GET_BATTERY_VOLTAGE);
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
  LOG("STM did not reset, going with soft reset");
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
  TickType_t xFrequency = pdMS_TO_TICKS(STM32_CHECK);
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
    if (that->_btn_listen && STM32_CHECK > 0) {
      event = that->get(K32_stm32_api::GET_BUTTON_EVENT);
      if (event == K32_stm32_api::BUTTON_CLICK_EVENT) {
         that->_btn_click = true;
         LOG("BTN clicked");
      }
      else if (event == K32_stm32_api::BUTTON_DOUBLE_CLICK_EVENT) {
         that->_btn_dblclick = true;
         LOG("BTN dblclicked");
      }
    }

    // check Battery
    if (that->_batt_listen && STM32_CHECK_BATT > 0) {
      tickerBattery -= 1;
      if (tickerBattery <= 0) {
        int batt = that->get(K32_stm32_api::GET_BATTERY_PERCENTAGE);
        that->_battery = batt;
        tickerBattery = STM32_CHECK_BATT/STM32_CHECK;
        LOGINL("Battery "); LOG(batt);
      }
    }

    // Unlock STM32
    xSemaphoreGive(that->lock);

    // sleep
    vTaskDelay( xFrequency );
  }
  vTaskDelete(NULL);
};


void K32_stm32::blink_task( void * parameter ) {
  K32_stm32* that = (K32_stm32*) parameter ;
  long startTime = millis();
  uint8_t leds_off[6] = {0,0,0,0,0,0};
  while ((millis() - startTime) < that->_blink_duration)
  {
    vTaskDelay(100);
    that->leds(that->_blink_leds);
    vTaskDelay(100);
    that->leds(leds_off);
  }
  vTaskDelete(NULL);
}


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
