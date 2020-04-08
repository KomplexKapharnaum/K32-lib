/*
  K32.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_h
#define K32_h

#include <Arduino.h>

#define min(m, n) ((m) < (n) ? (m) : (n))
#define max(m, n) ((m) > (n) ? (m) : (n))

#include "K32_version.h"
#include "system/K32_timer.h"
#include "system/K32_power.h"
#include "system/K32_log.h"
#include "system/K32_system.h"
#include "system/K32_sd.h"
#include "system/K32_pwm.h"
#include "audio/K32_audio.h"
#include "light/K32_light.h"
#include "remote/K32_remote.h"
#include "network/K32_wifi.h"
#include "network/K32_osc.h"
#include "network/K32_mqtt.h"
#include "network/K32_artnet.h"

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
        timer = new K32_timer();

    // Save NODE_ID in flash
    #ifdef K32_SET_NODEID
        system->id(K32_SET_NODEID);
        system->channel(15);
    #endif
        LOGINL("HW id:  ");
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
    K32_system *system = NULL;
    K32_wifi *wifi = NULL;
    K32_sd *sd = NULL;
    K32_audio *audio = NULL;
    K32_light *light = NULL;
    K32_pwm *pwm = NULL;
    K32_remote *remote = NULL;
    K32_power *power = NULL;
    K32_osc *osc = NULL;
    K32_mqtt *mqtt = NULL;
    K32_artnet *artnet = NULL;


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
                    light->addStrip(LEDS_PIN[system->hw()][k], (led_types)rubanType, rubanSize);
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
        remote = new K32_remote(system, BTN_PIN[system->hw()]);
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

    void init_power()
    {
      if (system->hw() >= 0 && system->hw() <= MAX_HW)
      {
        if(system->stm32 != NULL)
        {
          power = new K32_power(system->stm32, CURRENT_PIN[system->hw()]);
        } else
        {
          LOG("POWER: Error Missing STM32 Init. Power module can not be used without STM32");
        }
      } else
      {
        LOG("POWER: Error HWREVISION not valid please define K32_SET_HWREVISION or HW_REVISION");
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

    void init_artnet(artnetconf conf) {
        artnet = new K32_artnet();
        artnet->start(conf);

        if (!wifi)
            LOG("MQTT: Warning WIFI should be initialized BEFORE artnet");
    }

};

#endif
