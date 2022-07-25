/*
  K32_mcp.cpp
  Created by Clement GANGNEUX, february 2020.
  Modified by RIRI, Mars 2020.
  Modified by MGR, July 2020.
  Released under GPL v3.0
*/

#include "K32_mcp.h"
#include <Wire.h>

////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////// PUBLIC /////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////

K32_mcp::K32_mcp(K32* k32) : K32_plugin("mcp", k32)
{
  this->lock = xSemaphoreCreateMutex();

  /* Init I2C and Buttons pins */
  Wire.begin(MCP_PIN[k32->system->hw()][0], MCP_PIN[k32->system->hw()][1]);     // i2c pins
  if(!this->mcp.begin_I2C(0x20)){              // i2c addr
    LOG("MCP: not found.. disabled");
    return;
  }

  LOG("MCP: init");
  this->ok = true;

  // Start read button state task
  xTaskCreate(this->read_btn_state, // function
              "read_btn_task",      // task name
              1500,                 // stack memory
              (void *)this,         // args
              0,                    // priority
              NULL);                // handler
};


void K32_mcp::input(uint8_t pin) {
  if (pin>=16) return;
  this->_lock();
  this->mcp.pinMode(pin, INPUT_PULLUP);
  this->io[pin].mode = MCPIO_INPUT;  
  this->io[pin].state = this->mcp.digitalRead(pin);
  if (this->io[pin].state == LOW) this->io[pin].flag = MCPIO_PRESS_LONG;
  // this->mcp.pullUp(pin, HIGH);
  this->_unlock();
}

void K32_mcp::output(uint8_t pin) {
  if (pin>=16) return;
  this->_lock();
  this->io[pin].state = LOW;
  this->io[pin].mode = MCPIO_OUTPUT;
  this->mcp.pinMode(pin, OUTPUT);
  this->_unlock();
}

bool K32_mcp::state(uint8_t pin) {
  if (pin>=16) return true;
  if (this->io[pin].mode == MCPIO_DISABLE) this->input(pin);
  this->_lock();
  bool state = this->io[pin].state;
  this->_unlock();
  return state;
}

ioflag K32_mcp::flag(uint8_t pin) {
  if (pin>=16) return MCPIO_NOT;
  this->_lock();
  ioflag flag = this->io[pin].flag;
  this->_unlock();
  return flag;
}

void K32_mcp::consume(uint8_t pin) {
  this->_lock();
  this->io[pin].flag = MCPIO_NOT;
  this->_unlock();
} 

void K32_mcp::set(uint8_t pin, bool value) {
  if (pin>=16) return;
  this->_lock();
  this->io[pin].state = value;
  this->mcp.digitalWrite(pin, value);
  this->_unlock();
}

void K32_mcp::_lock()
{
  xSemaphoreTake(this->lock, portMAX_DELAY);
}

void K32_mcp::_unlock()
{
  xSemaphoreGive(this->lock);
}


/*
 * /////////////////////////////////////// PRIVATE /////////////////////////////////////
 */


////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////// STATE ////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////

void K32_mcp::read_btn_state(void *parameter)
{
  K32_mcp *that = (K32_mcp *)parameter;
  TickType_t xFrequency = pdMS_TO_TICKS(BTN_CHECK);
  unsigned long debounceDelay = 120;   // the debounce time; increase if the output flickers
  unsigned long longPushDelay = 1000; // Delay for a long push of the button

  while (true)
  {
    // for (int i = 0; i < 16; i++) {
    //   LOGINL(that->mcp.digitalRead(i));
    //   LOGINL(" ");
    // }
    // LOG("");

    for (int i = 0; i < 16; i++)
      if (that->io[i].mode == MCPIO_INPUT)
      {
        // read the state of the switch into a local variable:
        that->_lock();
        int reading = that->mcp.digitalRead(i);
        that->_unlock();

        // Pushed
        if (reading == LOW)
        {
          // was released
          if (that->io[i].state == HIGH) 
          {
            that->_lock();
            that->io[i].state = LOW;
            that->io[i].lastPushTime = millis(); // Record time of pushing button
            that->_unlock();
          }

          // was already pushed
          else 
          {
            
            if (that->io[i].lastPushTime != 0)  // NOT laready LONG PRESS
            {
              // -> PRESS
              if (that->io[i].flag != MCPIO_PRESS && (millis() - that->io[i].lastPushTime > debounceDelay)) {
                that->_lock();
                that->io[i].flag = MCPIO_PRESS;
                that->_unlock();
                
                Orderz* newOrder = new Orderz("mcp/press");
                newOrder->addData(i);
                newOrder->addData("short");
                that->emit( newOrder );

                LOGF("MCP: PRESS %i\n", i);
              }
              
              // -> LONG
              else if (that->io[i].flag == MCPIO_PRESS && (millis() - that->io[i].lastPushTime > longPushDelay)) {
                that->_lock();
                that->io[i].flag = MCPIO_PRESS_LONG;   // Long push
                that->io[i].lastPushTime = 0;          // Reset counter
                that->_unlock();
                
                Orderz* newOrder = new Orderz("mcp/press");
                newOrder->addData(i);
                newOrder->addData("long");
                that->emit( newOrder );

                LOGF("MCP: PRESS LONG %i\n", i);
              }
            }

          }
        }

        // Released
        else
        {
          // was pushed
          if (that->io[i].state == LOW) 
          {
            that->_lock();
            that->io[i].state = HIGH;
            that->io[i].lastPushTime = 0;

            if (that->io[i].flag == MCPIO_PRESS) 
            {
                that->io[i].flag = MCPIO_RELEASE_SHORT;

                Orderz* newOrder = new Orderz( "mcp/release");
                newOrder->addData(i);
                newOrder->addData("short");
                that->emit( newOrder );

                LOGF("MCP: RELEASE SHORT %i\n", i);
            }
            else if (that->io[i].flag == MCPIO_PRESS_LONG)
            {
              that->io[i].flag = MCPIO_RELEASE_LONG;

              Orderz* newOrder = new Orderz( "mcp/release" );
              newOrder->addData(i);
              newOrder->addData("long");
              that->emit( newOrder );

              LOGF("MCP: RELEASE LONG %i\n", i);
            }
            that->_unlock();
          }
        }
      }

    vTaskDelay(xFrequency);
  }
}
