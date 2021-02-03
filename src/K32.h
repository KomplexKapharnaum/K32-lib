/*
  K32.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_h
#define K32_h

#include <Arduino.h>
#include <BluetoothSerial.h>


#define min(m, n) ((m) < (n) ? (m) : (n))
#define max(m, n) ((m) > (n) ? (m) : (n))

#include "K32_version.h"
#include "system/K32_timer.h"
#include "xtension/K32_power.h"
#include "system/K32_log.h"
#include "system/K32_system.h"
#include "system/K32_intercom.h"
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

        // LOG
        LOGSETUP();
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
    }


    K32_timer* timer;
    K32_system *system      = NULL;
    K32_intercom *intercom  = NULL;
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

    void init_dmx(DmxDirection dir)
    {
        if (system->hw() >= 0 && system->hw() <= MAX_HW)
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
        wifi = new K32_wifi(system->name() + nameAlias);
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





    void run() 
    {
        Orderz* nextOrder = intercom->next();
        if (nextOrder != NULL)
        {
            LOGINL("COM: dequeue ");
            LOGINL(nextOrder->engine);
            LOGINL(" ");
            LOGINL(nextOrder->dataCount());

            // LOGINL(" =");
            // argX* a = nextOrder->getData(0);
            // LOGINL(a->toInt());
            // LOGINL(" =");
            // LOGINL(nextOrder->getData(1)->toStr());
            // LOGINL(" =");
            // LOGINL(nextOrder->getData(2)->toStr());
            // LOGINL(" =");
            // LOGINL(nextOrder->getData(3)->toStr());

            int v = 0;
            int count = nextOrder->dataCount();
            for(int k=0; k<3; k++ ) {
                LOGINL(" =");
                int v = nextOrder->_data[k];
                LOGINL(v);
                // LOGINL(nextOrder->getData(k)->toInt());
            }
            LOG("ok");
            // delete(nextOrder);
        }
        delay(2);
        return;
        
        if (nextOrder->engine != "") {
            
            // SYSTEM
            //
            if (strcmp(nextOrder->engine, "system") == 0) 
            {

            // RESET
            if (strcmp(nextOrder->action, "reset") == 0) 
                this->system->reset();

            // SHUTDOWN
            else if (strcmp(nextOrder->action, "shutdown") == 0) 
                this->system->shutdown();

            // SET CHANNEL
            else if (strcmp(nextOrder->action, "channel") == 0)
            {
                if (nextOrder->dataCount() < 1) return;
                byte chan = nextOrder->getData(0)->toInt();
                if (chan > 0) {
                this->system->channel(chan);
                delay(100);
                this->system->reset();
                }
            }

            }

            // AUDIO
            //
            else if (strcmp(nextOrder->engine, "audio") == 0 && this->audio) 
            {

            // PLAY MEDIA
            if (strcmp(nextOrder->action, "play") == 0)
            {
                if (nextOrder->dataCount() < 1) return;
                this->audio->play(nextOrder->getData(0)->toStr());

                if (nextOrder->dataCount() < 2) return;
                this->audio->volume(nextOrder->getData(1)->toInt());

                if (nextOrder->dataCount() < 3) return;
                this->audio->loop(nextOrder->getData(2)->toInt() > 0);
            }

            // SAMPLER NOTEON
            else if (strcmp(nextOrder->action, "noteon") == 0)
            {
                if (nextOrder->dataCount() < 2) return;
                this->audio->sampler->bank( nextOrder->getData(0)->toInt() );
                this->audio->play( this->audio->sampler->path( nextOrder->getData(1)->toInt() ) );

                if (nextOrder->dataCount() < 3) return;
                this->audio->volume(nextOrder->getData(2)->toInt());

                if (nextOrder->dataCount() < 4) return;
                this->audio->loop(nextOrder->getData(3)->toInt() > 0);
            }

            // SAMPELR NOTEOFF
            else if (strcmp(nextOrder->action, "noteoff") == 0)
            {
                if (nextOrder->dataCount() < 1) return;
                if (this->audio->media() == this->audio->sampler->path( nextOrder->getData(0)->toInt()) )
                this->audio->stop();
            }

            // STOP
            else if (strcmp(nextOrder->action, "stop") == 0)
            {
                this->audio->stop();
            }

            // VOLUME
            else if (strcmp(nextOrder->action, "volume") == 0)
            {
                if (nextOrder->dataCount() < 1) return;
                this->audio->volume(nextOrder->getData(0)->toInt());
            }

            // LOOP
            else if (strcmp(nextOrder->action, "loop") == 0)
            {
                if (nextOrder->dataCount() == 0) this->audio->loop(true);
                else this->audio->loop(nextOrder->getData(0)->toInt() > 0);
            }

            // UNLOOP
            else if (strcmp(nextOrder->action, "unloop") == 0)
            {
                this->audio->loop(false);
            }

            }

            // RAW MIDI
            //
            else if (strcmp(nextOrder->engine, "midi") == 0 && this->audio) {

            if (nextOrder->dataCount() < 3) return;

            byte event = nextOrder->getData(0)->toInt() / 16;
            byte note  = nextOrder->getData(1)->toInt();
            byte velo  = nextOrder->getData(2)->toInt();

            // NOTE OFF
            if (this->audio->noteOFF && (event == 8 || (event == 9 && velo == 0)))
            {
                if (this->audio->media() == this->audio->sampler->path(note))
                this->audio->stop();
            }

            // NOTE ON
            else if (event == 9)
                this->audio->play(this->audio->sampler->path(note), velo);

            // CC
            else if (event == 11)
            {
                // LOOP
                if (note == 1)
                this->audio->loop((velo > 63));

                // NOTEOFF enable
                else if (note == 2)
                this->audio->noteOFF = (velo < 63);

                // VOLUME
                else if (note == 7)
                this->audio->volume(velo);

                // BANK SELECT
                // else if (note == 32) this->audio->sampler->bank(velo+1);

                // STOP ALL
                else if (note == 119 or note == 120)
                this->audio->stop();
            }

            }

            // LEDS
            //
            else if (strcmp(nextOrder->engine, "leds") == 0 && this->light) 
            {

                // ALL
                if (strcmp(nextOrder->action, "all") == 0 || strcmp(nextOrder->action, "strip") == 0 || strcmp(nextOrder->action, "pixel") == 0)
                {
                    int offset = 0;
                    if (strcmp(nextOrder->action, "strip") == 0) offset = 1;
                    if (strcmp(nextOrder->action, "pixel") == 0) offset = 2;

                    if (nextOrder->dataCount() < offset+1) return;
                    int red, green, blue, white = 0;
                    
                    red = nextOrder->getData(offset+0)->toInt();
                    if (nextOrder->dataCount() > offset+2) {
                        green = nextOrder->getData(offset+1)->toInt();
                        blue  = nextOrder->getData(offset+2)->toInt();
                        if (nextOrder->dataCount() > offset+3) 
                            white = nextOrder->getData(offset+3)->toInt();
                    }
                    else { green = red; blue = red; white = red; }

                    this->light->blackout();

                    if (strcmp(nextOrder->action, "all") == 0) 
                    this->light->all( red, green, blue, white );
                    else if (strcmp(nextOrder->action, "strip") == 0) 
                    this->light->strip(nextOrder->getData(0)->toInt())->all( red, green, blue, white );
                    else if (strcmp(nextOrder->action, "pixel") == 0) 
                    this->light->strip(nextOrder->getData(0)->toInt())->pix( nextOrder->getData(1)->toInt(), red, green, blue, white );

                    this->light->show();
                }

                // MASTER
                else if (strcmp(nextOrder->action, "master") == 0)
                {
                    int masterValue = this->light->anim("manu")->master();

                    if (strcmp(nextOrder->subaction, "less") == 0)       masterValue -= 2;
                    else if (strcmp(nextOrder->subaction, "more") == 0)  masterValue += 2;
                    else if (strcmp(nextOrder->subaction, "full") == 0)  masterValue = 255;
                    else if (strcmp(nextOrder->subaction, "fadeout") == 0) {
                    if (!this->light->anim("manu")->hasmod("fadeout"))
                        this->light->anim("manu")->mod(new K32_mod_fadeout)->name("fadeout")->at(0)->period(6000)->play();
                    else
                        this->light->anim("manu")->mod("fadeout")->play();
                    }
                    else if (strcmp(nextOrder->subaction, "fadein") == 0) {
                    if (!this->light->anim("manu")->hasmod("fadein"))
                        this->light->anim("manu")->mod(new K32_mod_fadein)->name("fadein")->at(0)->period(6000)->play();
                    else
                        this->light->anim("manu")->mod("fadein")->play();
                    }
                    else if (nextOrder->dataCount() > 0) masterValue = nextOrder->getData(0)->toInt();

                    this->light->anim("manu")->master( masterValue );
                    this->light->anim("manu")->push();
                }

                // MEM (Manu)
                else if (strcmp(nextOrder->action, "mem") == 0)
                {
                    LOGF("DISPATCH: leds/mem %i\n",  nextOrder->getData(0)->toInt());

                    if (nextOrder->dataCount() > 0)
                    this->remote->stmSetMacro( nextOrder->getData(0)->toInt() );

                    if (nextOrder->dataCount() > 1) {
                    int masterValue = nextOrder->getData(1)->toInt();
                    this->light->anim("manu")->master( nextOrder->getData(1)->toInt() );
                    }
                }

                // STOP
                else if (strcmp(nextOrder->action, "stop") == 0 || strcmp(nextOrder->action, "off") == 0 || strcmp(nextOrder->action, "blackout") == 0)
                {
                    this->remote->stmBlackout();
                }

                // MODULATORS (Manu)
                else if (strcmp(nextOrder->action, "mod") == 0 || strcmp(nextOrder->action, "modi") == 0)
                { 
                    if (nextOrder->dataCount() < 1) return;

                    // Find MOD
                    K32_modulator* mod;

                    // get MOD by name
                    if (strcmp(nextOrder->action, "mod") == 0)        
                    mod = this->light->anim("manu")->mod( String(nextOrder->getData(0)->toStr()) );

                    // get MOD by id
                    else if (strcmp(nextOrder->action, "modi") == 0) 
                    mod = this->light->anim("manu")->mod( nextOrder->getData(0)->toInt() );

                    if (strcmp(nextOrder->subaction, "faster") == 0) mod->faster();
                    else if (strcmp(nextOrder->subaction, "slower") == 0) mod->slower();
                    else if (strcmp(nextOrder->subaction, "bigger") == 0) mod->bigger();
                    else if (strcmp(nextOrder->subaction, "smaller") == 0) mod->smaller();
                }

            }
        
        }
        else delay(1);
    }

};

#endif
