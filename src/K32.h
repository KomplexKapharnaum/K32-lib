/*
  K32.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_h
#define K32_h

#include <Arduino.h>

#include "K32_version.h"
#include "system/K32_log.h"
#include "system/K32_system.h"
#include "system/K32_sd.h"
#include "system/K32_pwm.h"
#include "audio/K32_audio.h"
#include "light/K32_light.h"
#include "light/K32_modulo.h"
#include "remote/K32_remote.h"
#include "network/K32_wifi.h"
#include "network/K32_osc.h"
#include "network/K32_mqtt.h"

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

    // Save NODE_ID in flash
    #ifdef K32_SET_NODEID
        system->id(K32_SET_NODEID);
        system->channel(15);
    #endif
        LOGINL("Node ID: ");
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

    K32_system *system = NULL;
    K32_wifi *wifi = NULL;
    K32_sd *sd = NULL;
    K32_audio *audio = NULL;
    K32_light *light = NULL;
    K32_pwm *pwm = NULL;
    K32_remote *remote = NULL;
    K32_osc *osc = NULL;
    K32_mqtt *mqtt = NULL;
    
    K32_modulo_sinus *modulo_sinus = NULL;
    K32_modulo_random *modulo_random = NULL;
    K32_modulo_linplus *modulo_linplus = NULL;
    K32_modulo_linmoins *modulo_linmoins = NULL;
    K32_modulo_onoff *modulo_onoff = NULL;
    K32_modulo_triplus *modulo_triplus = NULL;
    K32_modulo_trimoins *modulo_trimoins = NULL;
    K32_modulo_phase *modulo_phase = NULL;
    K32_modulo_fade *modulo_fade = NULL;

    void init_stm32()
    {
        system->stm32 = new K32_stm32( (system->hw() <= 2) );
    }

    void init_sd()
    {
        if (system->hw() >= 0 && system->hw() <= MAX_HW)
            sd = new K32_sd(SD_PIN[system->hw()]);
        else
            LOG("SD: Error HWREVISION not valid please define K32_SET_HWREVISION or HW_REVISION");
    }

    void init_audio()
    {
        if (system->hw() >= 0 && system->hw() <= MAX_HW)
            audio = new K32_audio(AUDIO_PIN[system->hw()], SD_PIN[system->hw()]);
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
                    light->strips()->addStrip(LEDS_PIN[system->hw()][k], (led_types)rubanType, rubanSize);
            light->start();
        }
        else
            LOG("LIGHT: Error HWREVISION not valid please define K32_SET_HWREVISION or HW_REVISION");
    }

    void init_pwm()
    {
        if (system->hw() >= 0 && system->hw() <= MAX_HW)
        {
            pwm = new K32_pwm();
            for(int k=0; k<PWM_N_CHAN; k++) 
                if(PWM_PIN[system->hw()][k] > 0) 
                    pwm->attach(PWM_PIN[system->hw()][k]);
        }
        else
            LOG("LIGHT: Error HWREVISION not valid please define K32_SET_HWREVISION or HW_REVISION");
    }

    void init_remote(int nbOfMacro)
    {
      if (system->hw() >= 0 && system->hw() <= MAX_HW)
      {
        remote = new K32_remote(BTN_PIN[system->hw()]);
        if(nbOfMacro > 0)
        {
          remote->setMacroMax(nbOfMacro);
        } else
        {
          LOG("REMOTE: Error Number of Macro must be positive");
        }
      } else
      {
        LOG("REMOTE: Error HWREVISION not valid please define K32_SET_HWREVISION or HW_REVISION");
      }
    }

    void init_wifi(String nameAlias = "") {
        btStop();
        if (nameAlias != "") nameAlias = "-"+nameAlias;
        wifi = new K32_wifi(system->name()+nameAlias);
    }

    void init_osc(oscconf conf) {
        osc = new K32_osc(system, wifi, audio, light);
        osc->start(conf);

        if (!wifi)
            LOG("OSC: Warning WIFI should be initialized BEFORE osc");
    }

    void init_mqtt(mqttconf conf) {
        mqtt = new K32_mqtt(system, wifi, audio, light);
        mqtt->start(conf);

        if (!wifi)
            LOG("MQTT: Warning WIFI should be initialized BEFORE mqtt");
    }

    void init_modulo()
    {
        modulo_sinus = new K32_modulo_sinus(0,0,0);
        modulo_random = new K32_modulo_random(0,0);
        modulo_linplus = new K32_modulo_linplus(0,0,0);
        modulo_linmoins = new K32_modulo_linmoins(0,0,0);
        modulo_onoff = new K32_modulo_onoff(0,0,0);
        modulo_triplus = new K32_modulo_triplus(0,0,0);
        modulo_trimoins = new K32_modulo_trimoins(0,0,0);
        modulo_phase = new K32_modulo_phase(0,0,0);
        modulo_fade = new K32_modulo_fade(0,0,0);
    }
    

private:
};

#endif
