/*
  K32_module.h
  Created by Thomas BOHL, may 2021.
  Released under GPL v3.0
*/

#ifndef K32_module_h
#define K32_module_h

#include "core/K32_intercom.h"

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
      LOG("BASE ");
    }

    // EXECUTE specific callback
    void execute(Orderz* order) {
      command(order);
    }

    // LINK intercom
    void link(K32_intercom *i) {
      intercom = i;
    }
  
  protected:

    String _name = "module";

    K32_intercom *intercom;
};

#endif
