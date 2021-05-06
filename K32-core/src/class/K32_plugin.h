/*
  K32_plugin.h
  Created by Thomas BOHL, may 2021.
  Released under GPL v3.0
*/

#ifndef K32_plugin_h
#define K32_plugin_h

#include "class/K32_module.h"
#include "K32.h"

class K32_plugin : public K32_module {
    public:
        K32_plugin(String name, K32* k = nullptr) : K32_module(name) {
            k32 = k;
            if (k32 != nullptr) k32->attach(this);
        }
        
    protected:
        K32* k32 = nullptr;

};

#endif
