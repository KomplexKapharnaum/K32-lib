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

    K32()
    {
        // LOG
        Serial.begin(115200, SERIAL_8N1);
        Serial.setTimeout(10);
        LOG("\n\n.:: K32 ::.");
        
        // INTERCOM
        intercom = new K32_intercom();

        // SYSTEM
        system = new K32_system();
        attach((K32_module*)system);

        // TIMER 
        timer = new K32_timer();

        // Save NODE_ID in flash
        #ifdef K32_SET_NODEID
            system->id(K32_SET_NODEID);
            system->channel(15);
        #endif

        LOGINL("Node id: ");
        LOG(system->id());

        // Save HW_REVISION in flash
        #ifdef K32_SET_HWREVISION
            system->hw(K32_SET_HWREVISION);
        #elif HW_REVISION
            system->hw(HW_REVISION);
        #endif

        LOGINL("HW rev: ");
        LOG(system->hw());

        LOG("");
        delay(100);

        // RUN Thread
        xTaskCreate( this->run,       // function
                  "run",              // name
                  10000,              // stack memory
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
        return new K32_module("");
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
                module(order->engine)->execute(order);
            }
            else {
                LOGINL(" - ");
                LOG(order->engine_action);
                this->intercom->ee->emit(order->engine, order);
                this->intercom->ee->emit(order->engine_action, order);
            }
        }
        delete(order);            
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
        }
        vTaskDelete(NULL);
    }

    

};


#ifdef ZZZ

#define min(m, n) ((m) < (n) ? (m) : (n))
#define max(m, n) ((m) > (n) ? (m) : (n))

#include <BluetoothSerial.h>

#include "K32_version.h"

#include "xtension/K32_power.h"
#include "system/K32_timer.h"
#include "system/K32_sd.h"
#include "system/K32_pwm.h"
#include "audio/K32_audio.h"
#include "light/K32_light.h"
#include "light/K32_dmx.h"
#include "light/K32_samplerjpeg.h"
#include "xtension/K32_mcp.h"
#include "xtension/K32_remote.h"
#include "network/K32_wifi.h"
#include "network/K32_bluetooth.h"
#include "network/K32_osc.h"
#include "network/K32_mqtt.h"
#include "network/K32_artnet.h"
#include "xtension/K32_power.h"

class K32
{
public:
    K32()
    {
        // SERIAL
        Serial.begin(115200, SERIAL_8N1);
        Serial.setTimeout(10);

        // LOG
        LOG("\n\n.:: K32 ::.");

        // SYSTEM
        system = new K32_system();
        intercom = new K32_intercom();
        timer = new K32_timer();

// Save NODE_ID in flash
#ifdef K32_SET_NODEID
        system->id(K32_SET_NODEID);
        system->channel(15);
#endif
        LOGINL("Node id: ");
        LOG(system->id());

// Save HW_REVISION in flash
#ifdef K32_SET_HWREVISION
        system->hw(K32_SET_HWREVISION);
#elif HW_REVISION
        system->hw(HW_REVISION);
#endif
        LOGINL("HW rev: ");
        LOG(system->hw());

        LOG("");
        delay(100);

        // RUN Thread
        xTaskCreate( this->run,       // function
                  "run",              // name
                  10000,              // stack memory
                  (void*)this,        // args
                  10,                 // priority
                  NULL);
    }


    K32_timer* timer;
    K32_wifi *wifi          = NULL;
    K32_bluetooth *bt       = NULL;
    K32_sd *sd              = NULL;
    K32_audio *audio        = NULL;
    K32_light *light        = NULL;
    K32_pwm *pwm            = NULL;
    K32_mcp *mcp            = NULL;
    K32_remote *remote      = NULL;
    K32_osc *osc            = NULL;
    K32_mqtt *mqtt          = NULL;
    K32_artnet *artnet      = NULL;
    K32_dmx *dmx            = NULL; 
    K32_power *power        = NULL; 

    K32_samplerjpeg *samplerjpeg = NULL;

    void init_stm32()
    {
        system->stm32 = new K32_stm32((system->hw() <= 2));
    }

    void init_sd()
    {
        if (system->hw() >= 0 && system->hw() <= MAX_HW)
            if( SD_PIN[system->hw()][0] > 0 )
                sd = new K32_sd(SD_PIN[system->hw()]);
            else
                LOG("SD: Error Pinout is invalid");
        else
            LOG("SD: Error HWREVISION not valid please define K32_SET_HWREVISION or HW_REVISION");
    }

    void init_audio()
    {
        if (system->hw() >= 0 && system->hw() <= MAX_HW)
            if( AUDIO_PIN[system->hw()][0] > 0 && SD_PIN[system->hw()][0] > 0 )
                audio = new K32_audio(AUDIO_PIN[system->hw()], SD_PIN[system->hw()]);
            else
                LOG("AUDIO: Error Pinout is invalid");
        else
            LOG("AUDIO: Error HWREVISION not valid please define K32_SET_HWREVISION or HW_REVISION");

        if (light)
            LOG("AUDIO: Warning AUDIO should be initialized BEFORE light");
    }

    void init_light(int rubanType, int rubanSize = 0)
    {
        if (system->hw() >= 0 && system->hw() <= MAX_HW)
        {
            light = new K32_light();
            for(int k=0; k<LED_N_STRIPS; k++)
                if(LEDS_PIN[system->hw()][k] > 0)
                    light->addStrip(LEDS_PIN[system->hw()][k], (led_types)rubanType, rubanSize);
            light->start();
        }
        else
            LOG("LIGHT: Error HWREVISION not valid please define K32_SET_HWREVISION or HW_REVISION");
    }

    void init_dmx(DmxDirection dir, const int pinout[3] = NULL)
    {
        if (pinout != NULL) 
        {
            dmx = new K32_dmx(pinout, dir);
        }
        else if (system->hw() >= 0 && system->hw() <= MAX_HW)
        {
            if( DMX_PIN[system->hw()][0] > 0 )
                dmx = new K32_dmx(DMX_PIN[system->hw()], dir);
            else
                LOG("DMX: Error Pinout is invalid");
        }
        else
            LOG("DMX: Error HWREVISION not valid please define K32_SET_HWREVISION or HW_REVISION");
    }

    void init_pwm()
    {
        if (system->hw() >= 0 && system->hw() <= MAX_HW)
        {
            pwm = new K32_pwm();
            for (int k = 0; k < PWM_N_CHAN; k++)
                if (PWM_PIN[system->hw()][k] > 0)
                    pwm->attach(PWM_PIN[system->hw()][k]);
        }
        else
            LOG("LIGHT: Error HWREVISION not valid please define K32_SET_HWREVISION or HW_REVISION");
    }

    void init_mcp()
    {
        if (system->hw() >= 0 && system->hw() <= MAX_HW)
        {
            if( MCP_PIN[system->hw()][0] > 0 )
                mcp = new K32_mcp(MCP_PIN[system->hw()]);
            else
                LOG("MCP: Error Pinout is invalid");
        }
        else LOG("MCP: Error HWREVISION not valid please define K32_SET_HWREVISION or HW_REVISION");
    }

    void init_remote(int nbOfMacro)
    {
        if (system->hw() >= 0 && system->hw() <= MAX_HW)
        {
            remote = new K32_remote(system, mcp);
            if (nbOfMacro > 0)
            {
                remote->setMacroMax(nbOfMacro);
            }
            else LOG("REMOTE: Error Number of Macro must be positive");
        }
        else LOG("REMOTE: Error HWREVISION not valid please define K32_SET_HWREVISION or HW_REVISION");
    }

    void init_power(batteryType type, bool autoGauge, int fakeExtCurrent=0)
    {
        if (system->hw() >= 0 && system->hw() <= MAX_HW)
        {   
            if (mcp == NULL) LOG("POWER: Warning, MCP is not initialized, CALIB button is disabled");

            if (system->stm32 != NULL) {
                if( CURRENT_PIN[system->hw()] > 0 )
                    power = new K32_power(system->stm32, mcp, type, autoGauge, fakeExtCurrent, CURRENT_PIN[system->hw()]);
                else
                    LOG("POWER: Error Pinout is invalid");
            }
            else 
                LOG("POWER: Error Missing STM32 Init. Power module can not be used without STM32");
        }
        else
            LOG("POWER: Error HWREVISION not valid please define K32_SET_HWREVISION or HW_REVISION");
    }

    void init_wifi(String nameAlias = "")
    {
        if (nameAlias != "")
            nameAlias = "-" + nameAlias;
        wifi = new K32_wifi(system->name() + nameAlias, intercom);
    }

    void init_bt(String name = "")
    {
        bt = new K32_bluetooth(name, system, audio, light, remote);
    }

    void init_osc(oscconf conf)
    {
        if (!wifi) {
            LOG("OSC: ERROR wifi should be initialized BEFORE osc..");
            return;
        }

        osc = new K32_osc(intercom, system, wifi);
        osc->start(conf);
    }

    void init_mqtt()
    {
        if (!wifi) {
            LOG("MQTT: ERROR wifi should be initialized BEFORE mqtt..");
            return;
        }

        mqtt = new K32_mqtt(intercom, system, wifi);
    }

    void init_artnet(artnetconf conf) 
    {
        if (!wifi) {
            LOG("ARTNET: ERROR wifi should be initialized BEFORE artnet..");
            return;
        }

        artnet = new K32_artnet();
        artnet->start(conf);
    }

    void init_samplerjpeg()
    {
        samplerjpeg = new K32_samplerjpeg(SD_PIN[system->hw()]);
    }




    // DISPACTH COMMANDS from InterCom
    /////////////////////


    void dispatch(Orderz* order) 
    {
        while (order->consume()) 
        {    
            // SYSTEM
            if (strcmp(order->engine, "system") == 0) 
                this->system->execute(order);

            // AUDIO
            else if (strcmp(order->engine, "audio") == 0 && this->audio) 
                this->audio->execute(order);

            // LEDS
            else if (strcmp(order->engine, "leds") == 0 && this->light)  
                this->light->execute(order);

            // REMOTE
            else if (strcmp(order->engine, "remote") == 0 && this->remote)  
                this->remote->execute(order);

            // K32
            else if (strcmp(order->engine, "k32") == 0 )  {

                // OTA Started -> stop all !
                if (strcmp(order->action, "ota") == 0 )
                {
                    LOG("OTA started, stopping all services");
                    if (this->artnet)   this->artnet->stop();
                    if (this->mqtt)     this->mqtt->stop();
                    if (this->osc)      this->osc->stop();

                    if (this->remote)    this->remote->stop(); 
                    if (this->audio)    this->audio->stop(); 
                    if (this->light)    {
                        this->light->fps(1);
                        this->light->stop();
                    }

                }

            }
        }
        order->clear();
    }

    static void run( void * parameter ) 
    {   
        K32* that = (K32*) parameter;

        while(true) 
        { 
            Orderz nextOrder = that->intercom->next();
            that->dispatch(&nextOrder);
        }
        vTaskDelete(NULL);
    }

    

};

#endif
#endif