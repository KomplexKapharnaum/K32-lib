/*
  K32.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_h
#define K32_h

#include <Arduino.h>

#include "K32_version.h"
#include "K32_system.h"
#include "utils/K32_timer.h"


class K32
{
private:
    K32_module* _modules[32];
    int _moduleCount = 0;

public:

    K32_intercom *intercom;
    K32_system *system;
    K32_timer* timer;

    int version = 0;

    K32(int _version = 0)
    {   
        version = _version;

        // LOG
        Serial.begin(115200, SERIAL_8N1);
        Serial.setTimeout(10);
        LOG("\n\n");

LOG("__/\\\\\\________/\\\\\\_____/\\\\\\\\\\\\\\\\\\\\_____/\\\\\\\\\\\\\\\\\\_____        ");
LOG(" _\\/\\\\\\_____/\\\\\\//____/\\\\\\///////\\\\\\__/\\\\\\///////\\\\\\___       ");
LOG("  _\\/\\\\\\__/\\\\\\//______\\///______/\\\\\\__\\///______\\//\\\\\\__      ");
LOG("   _\\/\\\\\\\\\\\\//\\\\\\_____________/\\\\\\//_____________/\\\\\\/___     ");
LOG("    _\\/\\\\\\//_\\//\\\\\\___________\\////\\\\\\_________/\\\\\\//_____    ");
LOG("     _\\/\\\\\\____\\//\\\\\\_____________\\//\\\\\\_____/\\\\\\//________   ");
LOG("      _\\/\\\\\\_____\\//\\\\\\___/\\\\\\______/\\\\\\____/\\\\\\/___________  ");
LOG("       _\\/\\\\\\______\\//\\\\\\_\\///\\\\\\\\\\\\\\\\\\/____/\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\_ ");
LOG("        _\\///________\\///____\\/////////_____\\///////////////__");
                                            
        LOG("\n\n");

        // INTERCOM
        intercom = new K32_intercom();

        // SYSTEM
        system = new K32_system();
        attach((K32_module*)system);

        // TIMER 
        timer = new K32_timer();

        LOG("");
        delay(100);

        // RUN Thread
        xTaskCreate( this->run,       // function
                  "run",              // name
                  10000,              // stack memory       // 2500 without audio seems ok
                  (void*)this,        // args
                  5,                  // priority
                  NULL);
    }

    void attach(K32_module* module) {
        _modules[_moduleCount] = module;
        _moduleCount += 1;

        module->link(intercom);
    }
    
    K32_module* module(String name) {
        for (int i=0; i<_moduleCount; i++)
            if (_modules[i]->name() == name) return _modules[i];
        LOG("K32: module "+name+" not found..");
        return NULL;
    }

    // EVENTS Register
    /////////////////////

    void on(const char *name, void (*cb)(Orderz* order)) {
        this->intercom->ee->addListener(name, cb);
    }

    void emit(Orderz* order) {
        order->isCmd = false;
      intercom->queue(order);
    }

    void emit(const char* command) {
      intercom->queue(new Orderz(command, false));
    }

    void cmd(Orderz* order) {
        order->isCmd = true;
      intercom->queue(order);
    }

    void cmd(const char* command) {
      intercom->queue(new Orderz(command, true));
    }


    // DISPACTH COMMANDS from InterCom
    /////////////////////


    void dispatch(Orderz* order) 
    {
        while (order->consume()) {
            if (order->isCmd) {
                LOGINL(" * ");
                LOG(order->engine_action);
                K32_module* mod = module(order->engine);
                if (mod) mod->execute(order);
            }
            else {
                LOGINL(" - ");
                LOG(order->engine_action);
                this->intercom->ee->emit(order->engine, order);
                this->intercom->ee->emit(order->engine_action, order);
            }
        }            
    }

    static void run( void * parameter ) 
    {   
        K32* that = (K32*) parameter;

        while(true) 
        { 
            // LOG("K32: waiting order");
            Orderz* nextOrder = that->intercom->next();
            // LOG("K32: order obtained");
            that->dispatch(nextOrder);
            // LOG("K32: order dispatched");
            delete(nextOrder);
            // LOGF("WM k32 run: %d\n", uxTaskGetStackHighWaterMark( NULL ));
        }
        vTaskDelete(NULL);
    }

};


#endif