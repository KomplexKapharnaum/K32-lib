/*
  K32_node.h
  Created by Thomas BOHL, octover 2021.
  Released under GPL v3.0
*/

#ifndef K32_presets_h
#define K32_presets_h

#include <Arduino.h>

#define PRESET_MAX 64

typedef const uint8_t mem_t[ANIM_DATA_SLOTS];

class LPreset
{
    public:
        LPreset(mem_t m, size_t size) {
            _size = size;
            for(int j=0; j<_size; j++) _mem[j] = m[j];
        }

        LPreset(const char* p, size_t size) {
            _size = size;
            char value[16];
            for (int i=0; i<_size; i++) {
                splitString(p, ",", i, value);
                _mem[i] = atoi(deblank(value));
            }
        }

        // register new modulator
        K32_modulator* mod(K32_modulator* modulator) 
        { 
            for (int k=0; k<ANIM_MOD_SLOTS; k++)
                if(this->_modulators[k] == nullptr) {
                    this->_modulators[k] = modulator;
                    break;
                }
            
            return modulator;
        }

        K32_modulator** modulators() {
            return _modulators;
        }

        uint8_t* mem() {
            return _mem;
        }

        size_t size() {
            return _size;
        }


    private:
        size_t _size;
        uint8_t _mem[ANIM_DATA_SLOTS];

        // Modulator
        K32_modulator* _modulators[ANIM_MOD_SLOTS] = {nullptr};

        void splitString(const char *data, const char *separator, int index, char *result)
        {
            char input[strlen(data)];
            strcpy(input, data);

            char *command = strtok(input, separator);
            for (int k = 0; k < index; k++)
                if (command != NULL)
                command = strtok(NULL, separator);

            if (command == NULL) strcpy(result, "");
            else strcpy(result, command);
        }

        char * deblank(char *str)
        {
            char *out = str, *put = str;
            for(; *str != '\0'; ++str)
            {
                if(*str != ' ')
                *put++ = *str;
            }
            *put = '\0';
            return out;
        }

};


class LBank
{
    public:
        LBank(size_t presetsize) {
            _presetsize = presetsize;
            _nowifi = new LPreset(mem_t {}, _presetsize);
        }

        void name(String n) { _name = n; }
        String name() { return _name; }

        LPreset* add(mem_t p) 
        {   
            _bank[_size] = new LPreset(p, _presetsize);
            mem = _bank[_size];
            _size++;
            return mem;
        }

        LPreset* add(const char* p) 
        {
            _bank[_size] = new LPreset(p, _presetsize);
            mem = _bank[_size];
            _size++;
            return mem;
        }

        LPreset* nowifi(mem_t m) 
        {
            if (_nowifi) delete _nowifi;
            _nowifi = new LPreset(m, _presetsize);
            return _nowifi;
        }

        LPreset* get(int N) 
        {   
            if (N<0) N += this->_size;
            
            if (N>=0 && N<_size && _bank[N]!=nullptr) {
                mem = _bank[N];
                return _bank[N];
            }

            mem = nullptr;
            return nullptr;
        }

        size_t size() {
            return _size;
        }

        size_t preset_size() {
            return _presetsize;
        }

        LPreset* mem = nullptr;  // !!! point to last added/loaded preset   


    private:
        String _name;
        int _presetsize = 0;
        int _size = 0;

        LPreset*  _bank[PRESET_MAX] = {nullptr};
        LPreset*  _nowifi = nullptr;
};


// class PwmBank : LBank <4> {
//     public:
//         PwmBank() {
//             name("4pwm");

//             add("  255,  255,  255,  255 "); // 00 
//             add("  170,  170,  170,  170 "); // 01 
//             add("  126,  126,  126,  126 "); // 02 
//             add("   82,   82,   82,   82 "); // 03 
//             add("  255,  255,  255,  255 "); // 04 **
//             add("    0,    0,    0,    0 "); // 05 
//             add("  255,  170,  126,   82 "); // 06 
//             add("  170,  126,   82,    0 "); // 07 
//             add("  126,   82,    0,  255 "); // 08 
//             add("   82,    0,  255,  170 "); // 09 
//             add("    0,  255,  170,  126 "); // 10 
//             add("  170,  126,   82,    0 "); // 12 
//             add("  255,  170,  126,   82 "); // 11 
//             add("  126,   82,    0,  255 "); // 13 
//             add("   82,    0,  255,  170 "); // 14 
//             add("    0,    0,    0,    0 "); // 15 BLACK

//             // NO WIFI
//             uint8_t NOWIFI[] = { LULU_MEMNOWIFI_MASTER,  LULU_MEMNOWIFI_MASTER,  LULU_MEMNOWIFI_MASTER,  LULU_MEMNOWIFI_MASTER};
//             nowifi(NOWIFI);    
//         };
// };


#endif