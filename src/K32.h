/*
  K32.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_h
#define K32_h

#include <Arduino.h>

#include "K32_version.h"
#include "K32_log.h"
#include "K32_system.h"
#include "K32_wifi.h"
#include "K32_sd.h"
#include "K32_audio.h"
#include "K32_light.h"
#include "K32_remote.h"
#include "K32_osc.h"
#include "K32_mqtt.h"

class K32
{
public:
    K32()
    {
        // LOG
        LOGSETUP();

        // SYSTEM
        system = new K32_system();

        // System SET
    #ifdef K32_SET_NODEID
        system->id(K32_SET_NODEID);
        system->channel(15);
        LOGINL("Set id: ");
        LOG(K32_SET_NODEID);
    #endif
    #ifdef K32_SET_HWREVISION
        system->hw(K32_SET_HWREVISION);
        LOGINL("Set HW rev: ");
        LOG(K32_SET_HWREVISION);
    #endif

    }

    K32_system *system = NULL;
    K32_wifi *wifi = NULL;
    K32_sd *sd = NULL;
    K32_audio *audio = NULL;
    K32_light *light = NULL;
    K32_remote *remote = NULL;
    K32_osc *osc = NULL;
    K32_mqtt *mqtt = NULL;

    void init_stm32()
    {
        system->stm32 = new K32_stm32();
    }

    void init_sd()
    {
        if (system->hw() >= 0 && system->hw() <= 2)
            sd = new K32_sd(SD_PIN[system->hw()]);
        else
            LOG("SD: Error HWREVISION not valid please define K32_SET_HWREVISION");
    }

    void init_audio()
    {
        if (system->hw() >= 0 && system->hw() <= 2)
            audio = new K32_audio(AUDIO_PIN[system->hw()], SD_PIN[system->hw()]);
        else
            LOG("AUDIO: Error HWREVISION not valid please define K32_SET_HWREVISION");

        if (light)
            LOG("AUDIO: Warning AUDIO should be initialized BEFORE light");
    }

    void init_light()
    {
        if (system->hw() >= 0 && system->hw() <= 2)
        {
            light = new K32_light();
            light->leds()->attach(LEDS_PIN[system->hw()][0], 120, LED_SK6812W_V1);
            light->leds()->attach(LEDS_PIN[system->hw()][1], 120, LED_SK6812W_V1);
            light->start();
        }
        else
            LOG("LIGHT: Error HWREVISION not valid please define K32_SET_HWREVISION");
    }

    void init_remote(int nbOfMacro)
    {
      if (system->hw() >= 0 && system->hw() <= 2)
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
        LOG("REMOTE: Error HWREVISION not valid please define K32_SET_HWREVISION");
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

private:
};

#endif
