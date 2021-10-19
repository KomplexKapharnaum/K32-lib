/*
  K32_system.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_system_h
#define K32_system_h

#include <Preferences.h>
#include "esp_task_wdt.h"

#include "class/K32_module.h"
#include "K32_version.h"


class K32_system : K32_module {
  public:
    K32_system() : K32_module("system") 
    {
      prefs.begin("k32-app", false);
      
      #if HW_REVISION
          this->hw(HW_REVISION);
          _hw = HW_REVISION;
      #else
          _hw = prefs.getUInt("hw", 0);
      #endif

      #ifdef K32_SET_NODEID
          this->id(K32_SET_NODEID);
          _id = K32_SET_NODEID;
      #else
          _id = prefs.getUInt("id", 0);
      #endif

      #ifdef K32_SET_CHANNEL
          this->channel(K32_SET_CHANNEL);
          _chan = K32_SET_CHANNEL;
      #else
          _chan = prefs.getUInt("channel", 15);
      #endif

    };

    int id() {
      return _id;
    }

    void id(int id) {
      int old = prefs.getUInt("id", 0);
      if (id != old) {
        prefs.putUInt("id", id);
        _id = prefs.getUInt("id", 0);
      }
    }

    int channel() {
      return _chan;
    }

    void channel(int channel) {
      int old = prefs.getUInt("channel", 15);
      if (channel != old) {
        prefs.putUInt("channel", channel);
        _chan = prefs.getUInt("channel", 15);
      }
    }

    int hw() {
      return _hw;
    }

    void hw(int hwrevision) {
      int old = prefs.getUInt("hw", 0);
      if (hwrevision != old) {
        prefs.putUInt("hw", hwrevision);
        _hw = prefs.getUInt("hw", 0);
      }
    }

    String name() {
      String name = "esp-" + String(this->id());
      return name;
    }

    void reset() {
      prefs.end();
      cmd( "stm32/reset" );

      delay(10);
      ESP.restart();

      delay(10);
      LOG("ESP did not reset, going with soft reset");
      // Hard restart
      esp_task_wdt_init(1,true);
      esp_task_wdt_add(NULL);
      while(true);
      //
    }

    void shutdown() {
      prefs.end();
      cmd( "stm32/shutdown" );
    }

    int ledpin(int i) {
      return LEDS_PIN[hw()][i];
    }

    // EXECUTE standardized command
    void command(Orderz* order) 
    {
      // RESET
      if (strcmp(order->action, "reset") == 0) 
          this->reset();

      // SHUTDOWN
      else if (strcmp(order->action, "shutdown") == 0) 
          this->shutdown();

      // SET CHANNEL
      else if (strcmp(order->action, "channel") == 0)
      {
          if (order->count() < 1) return;
          byte chan = order->getData(0)->toInt();
          if (chan > 0) {
            this->channel(chan);
            delay(100);
            this->reset();
          }
      }
    }
    Preferences prefs;
    
  private:
    int _id;
    int _hw;
    int _chan;

};

#endif
