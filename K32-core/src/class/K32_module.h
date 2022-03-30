/*
  K32_module.h
  Created by Thomas BOHL, may 2021.
  Released under GPL v3.0
*/

#ifndef K32_module_h
#define K32_module_h

#include "K32_intercom.h"

class K32_module {
  public:
    K32_module(String name) {
        _name = name;
    }

    // GET Name
    String name() {
        return _name;
    }

    // COMMAND callback
    virtual void command( Orderz* order ) {
    }

    // EXECUTE specific callback
    void execute(Orderz* order) {
      command(order);
    }

    // LINK intercom
    void link(K32_intercom *i) {
      intercom = i;
    }

    // EVENTS Register
    /////////////////////

    void emit(Orderz* order) {
      order->isCmd = false;
      if (intercom != nullptr) intercom->queue(order);
      else LOG("ERROR: module ot linked to intercom");
    }

    void emit(const char* event) {
      if (intercom != nullptr) intercom->queue(new Orderz(event, false));
      else LOG("ERROR: module ot linked to intercom");
    }

    void emit(String event) {
      this->emit(event.c_str());
    }

    void cmd(Orderz* order) {
      order->isCmd = true;
      if (intercom != nullptr) intercom->queue(order);
      else LOG("ERROR: module ot linked to intercom");
    }

    void cmd(const char* command) {
      if (intercom != nullptr) intercom->queue(new Orderz(command, true));
      else LOG("ERROR: module ot linked to intercom");
    }

    void on(const char *name, void (*cb)(Orderz* order)) {
      if (intercom != nullptr)  intercom->ee->addListener(name, cb);
      else LOG("ERROR: module ot linked to intercom");
    }

  
  protected:
    String _name = "module";

  private:
    K32_intercom *intercom = nullptr;
    
};

#endif
