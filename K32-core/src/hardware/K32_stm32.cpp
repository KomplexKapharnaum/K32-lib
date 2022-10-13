/*
  K32_stm32.cpp
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/

#include "K32_stm32.h"

/*
 *   PUBLIC
 */

K32_stm32::K32_stm32(K32* k32, bool startListening) : K32_plugin("stm32", k32) 
{
  this->lock = xSemaphoreCreateMutex();
  if (startListening) this->listen(true, true);
};

void K32_stm32::listen() {
  // Checking task
  this->running = true;
  xTaskCreate( this->task,
                "stm32_task",
                1500,
                (void*)this,
                0,              // priority
                NULL);
  
  uint8_t l[6] = {255,255,255,255,255,255};
  this->blink(l, 200);
}

void K32_stm32::listen(bool btn, bool battery) {
  this->_btn_listen = btn;
  this->_batt_listen = battery;
  this->listen();
}

void K32_stm32::stopListening() {
  this->running = false;
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
                1500,
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
  LOG ("STM: get voltage");
};

bool K32_stm32::clicked() 
{
  xSemaphoreTake(this->lock, portMAX_DELAY);
  bool click = this->_btn_click;
  this->_btn_click = false;
  xSemaphoreGive(this->lock);
  return click;
  LOG ("STM: clicked");
};


bool K32_stm32::dblclicked() {
  xSemaphoreTake(this->lock, portMAX_DELAY);
  bool click = this->_btn_dblclick;
  this->_btn_dblclick = false;
  xSemaphoreGive(this->lock);
  return click;
  LOG ("STM: dblclicked");
};


void K32_stm32::wait() {
  xSemaphoreTake(this->lock, portMAX_DELAY);
  xSemaphoreGive(this->lock);
}

void K32_stm32::switchLoad(bool onoff) {
  this->send(K32_stm32_api::SET_LOAD_SWITCH, onoff);
}


void K32_stm32::reset() {
  xSemaphoreTake(this->lock, portMAX_DELAY);
  this->send(K32_stm32_api::SET_LOAD_SWITCH, 0);
  this->send(K32_stm32_api::REQUEST_RESET);
  xSemaphoreGive(this->lock);
}


void K32_stm32::shutdown() {
  xSemaphoreTake(this->lock, portMAX_DELAY);
  this->send(K32_stm32_api::SET_LOAD_SWITCH, 0);
  this->send(K32_stm32_api::SHUTDOWN);
  xSemaphoreGive(this->lock);
}

void K32_stm32::command(Orderz* order) {
  // RESET
  if (strcmp(order->action, "reset") == 0)  this->reset();

  // SHUTDOWN
  else if (strcmp(order->action, "shutdown") == 0)  this->shutdown();
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
         that->emit("stm32/btn-click");
      }
      else if (event == K32_stm32_api::BUTTON_DOUBLE_CLICK_EVENT) {
         that->_btn_dblclick = true;
         LOG("BTN dblclicked");
         that->emit("stm32/btn-dblclick");
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
