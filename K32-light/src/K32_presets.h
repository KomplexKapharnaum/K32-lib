/*
  K32_node.h
  Created by Thomas BOHL, octover 2021.
  Released under GPL v3.0
*/

#ifndef K32_presets_h
#define K32_presets_h

#include <Arduino.h>

#define MAX_PRESET 64

typedef void (*cbPtrModulate )(K32_anim *anim);
typedef const uint8_t mem_t[ANIM_DATA_SLOTS];
typedef const uint8_t bank_t[MAX_PRESET][ANIM_DATA_SLOTS];


template <size_t Size>
class LPreset
{
    public:
        LPreset(mem_t m) {
            for(int j=0; j<Size; j++) 
                _mem[j] = m[j];
        }

        LPreset(const char* p) {
            char value[16];
            for (int i=0; i<Size; i++) {
                splitString(p, ",", i, value);
                _mem[i] = atoi(deblank(value));
            }
        }

        LPreset* onload(cbPtrModulate m8clbck) {
            _m8clbck = m8clbck;
        } 

        void load(K32_anim *anim) {
            // remove disposable modulators
            //
            anim->unmod();

            // push new data
            //
            anim->push(_mem, Size);

            if (_m8clbck) _m8clbck(anim);
        }

    private:
        int _size = Size;
        uint8_t _mem[Size];
        cbPtrModulate _m8clbck = nullptr;

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


template <size_t Size>
class LBank
{
    public:
        LBank() {
            _nowifi = new LPreset<Size>(mem_t {});
        }

        void name(String n) { _name = n; }
        String name() { return _name; }

        LPreset<Size>* add(mem_t mem) 
        {   
            _bank[_length] = new LPreset<Size>(mem);
            _length++;
            return _bank[_length-1];
        }

        LPreset<Size>* add(const char* p) 
        {
            _bank[_length] = new LPreset<Size>(p);
            _length++;
            return _bank[_length-1];
        }

        LPreset<Size>* nowifi(mem_t m) 
        {
            if (_nowifi) delete _nowifi;
            _nowifi = new LPreset<Size>(m);
            return _nowifi;
        }


    private:
        String _name;
        int _size = Size;
        int _length = 0;

        LPreset<Size>*  _bank[MAX_PRESET];
        LPreset<Size>*  _nowifi;
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