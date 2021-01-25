/*
  K32_intercom.h
  Created by Thomas BOHL, january 2020.
  Released under GPL v3.0
*/
#ifndef K32_intercom_h
#define K32_intercom_h

#include "Arduino.h"
#include "system/K32_log.h"

enum argType { INT, STR };
struct argX
{ 
  argX(int value) {
    type = INT;
    argInt = value;
  }

  argX(const char* value) {
    type = STR;
    argStr = (char *) malloc(strlen(value) + 1); 
    strcpy(argStr, value);
    strInited = true;
  }

  argType type;
  char* argStr;
  int argInt;
  bool strInited = false;
  
  int toInt() { 
    if (type == INT) return argInt; 
    else if (type == STR) return atoi(argStr); 
  }
  const char* toStr() {
    if (type == INT && !strInited) {
      char str[33];
      sprintf(str, "%d", argInt);
      argStr = (char *) malloc(strlen(str) + 1); 
      strcpy(argStr, str);
      strInited = true;
    }
    return argStr; 
  }

  ~argX() {
    if (strInited) free(argStr);
  }
};

#include "system/K32_system.h"
#include "network/K32_wifi.h"
#include "audio/K32_audio.h"
#include "light/K32_light.h"
#include "xtension/K32_remote.h"



class K32_intercom 
{
  public:
    K32_intercom(K32_system *system, K32_wifi *wifi, K32_audio *audio, K32_light *light, K32_remote *remote) 
      : system(system), wifi(wifi), audio(audio), light(light), remote(remote) {}

    void dispatch(char* command, argX** args, int argCount) 
    {
      LOGF("DISPATCH: %s ", command);
      for(int k=0; k<argCount; k++) {
        LOGINL(" "); 
        LOGINL(args[k]->toStr());
      }
      LOG("");

      char engine[8];
      char action[8];
      char subaction[8];
      splitString(command, "/", 0, engine);
      splitString(command, "/", 1, action);
      splitString(command, "/", 2, subaction);

      // SYSTEM
      //
      if (strcmp(engine, "system") == 0) 
      {

        // RESET
        if (strcmp(action, "reset") == 0) 
          this->system->reset();

        // SHUTDOWN
        else if (strcmp(action, "shutdown") == 0) 
          this->system->shutdown();

        // SET CHANNEL
        else if (strcmp(action, "channel") == 0)
        {
          if (argCount < 1) return;
          byte chan = args[0]->toInt();
          if (chan > 0) {
            this->system->channel(chan);
            delay(100);
            this->system->reset();
          }
        }

      }

      // AUDIO
      //
      else if (strcmp(engine, "audio") == 0 && this->audio) 
      {

        // PLAY MEDIA
        if (strcmp(action, "play") == 0)
        {
          if (argCount < 1) return;
          this->audio->play(args[0]->toStr());

          if (argCount < 2) return;
          this->audio->volume(args[1]->toInt());

          if (argCount < 3) return;
          this->audio->loop(args[2]->toInt() > 0);
        }

        // SAMPLER NOTEON
        else if (strcmp(action, "noteon") == 0)
        {
          if (argCount < 2) return;
          this->audio->sampler->bank( args[0]->toInt() );
          this->audio->play( this->audio->sampler->path( args[1]->toInt() ) );

          if (argCount < 3) return;
          this->audio->volume(args[2]->toInt());

          if (argCount < 4) return;
          this->audio->loop(args[3]->toInt() > 0);
        }

        // SAMPELR NOTEOFF
        else if (strcmp(action, "noteoff") == 0)
        {
          if (argCount < 1) return;
          if (this->audio->media() == this->audio->sampler->path( args[0]->toInt()) )
            this->audio->stop();
        }

        // STOP
        else if (strcmp(action, "stop") == 0)
        {
          this->audio->stop();
        }

        // VOLUME
        else if (strcmp(action, "volume") == 0)
        {
          if (argCount < 1) return;
          this->audio->volume(args[0]->toInt());
        }

        // LOOP
        else if (strcmp(action, "loop") == 0)
        {
          if (argCount == 0) this->audio->loop(true);
          else this->audio->loop(args[0]->toInt() > 0);
        }

        // UNLOOP
        else if (strcmp(action, "unloop") == 0)
        {
          this->audio->loop(false);
        }

      }

      // RAW MIDI
      //
      else if (strcmp(engine, "midi") == 0 && this->audio) {
        
        if (argCount < 3) return;

        byte event = args[0]->toInt() / 16;
        byte note  = args[1]->toInt();
        byte velo  = args[2]->toInt();

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
      else if (strcmp(engine, "leds") == 0 && this->light) {

        // ALL
        if (strcmp(action, "all") == 0 || strcmp(action, "strip") == 0 || strcmp(action, "pixel") == 0)
        {
          int offset = 0;
          if (strcmp(action, "strip") == 0) offset = 1;
          if (strcmp(action, "pixel") == 0) offset = 2;

          if (argCount < offset+1) return;
          int red, green, blue, white = 0;
          
          red = args[offset+0]->toInt();
          if (argCount > offset+2) {
            green = args[offset+1]->toInt();
            blue  = args[offset+2]->toInt();
            if (argCount > offset+3) 
              white = args[offset+3]->toInt();
          }
          else { green = red; blue = red; white = red; }

          this->light->blackout();

          if (strcmp(action, "all") == 0) 
            this->light->all( red, green, blue, white );
          else if (strcmp(action, "strip") == 0) 
            this->light->strip(args[0]->toInt())->all( red, green, blue, white );
          else if (strcmp(action, "pixel") == 0) 
            this->light->strip(args[0]->toInt())->pix( args[1]->toInt(), red, green, blue, white );

          this->light->show();
        }

        // MASTER
        else if (strcmp(action, "master") == 0)
        {
          int masterValue = this->light->anim("manu")->master();

          if (strcmp(subaction, "less") == 0)       masterValue -= 2;
          else if (strcmp(subaction, "more") == 0)  masterValue += 2;
          else if (strcmp(subaction, "full") == 0)  masterValue = 255;
          else if (strcmp(subaction, "fadeout") == 0) {
            if (!this->light->anim("manu")->hasmod("fadeout"))
              this->light->anim("manu")->mod(new K32_mod_fadeout)->name("fadeout")->at(0)->period(6000)->play();
            else
              this->light->anim("manu")->mod("fadeout")->play();
          }
          else if (strcmp(subaction, "fadein") == 0) {
            if (!this->light->anim("manu")->hasmod("fadein"))
              this->light->anim("manu")->mod(new K32_mod_fadein)->name("fadein")->at(0)->period(6000)->play();
            else
              this->light->anim("manu")->mod("fadein")->play();
          }
          else if (argCount > 0) masterValue = args[0]->toInt();

          this->light->anim("manu")->master( masterValue );
          this->light->anim("manu")->push();
        }

        // MEM (Manu)
        else if (strcmp(action, "mem") == 0)
        {
          if (argCount > 0)
            this->remote->stmSetMacro( args[0]->toInt() );

          if (argCount > 1) {
            int masterValue = args[1]->toInt();
            this->light->anim("manu")->master( args[1]->toInt() );
          }
        }

        // STOP
        else if (strcmp(action, "stop") == 0 || strcmp(action, "off") == 0 || strcmp(action, "blackout") == 0)
        {
          this->remote->stmBlackout();
        }

        // MODULATORS (Manu)
        else if (strcmp(action, "mod") == 0 || strcmp(action, "modi") == 0)
        { 
          if (argCount < 1) return;

          // Find MOD
          K32_modulator* mod;

          // get MOD by name
          if (strcmp(action, "mod") == 0)        
            mod = this->light->anim("manu")->mod( String(args[0]->toStr()) );

          // get MOD by id
          else if (strcmp(action, "modi") == 0) 
            mod = this->light->anim("manu")->mod( args[0]->toInt() );

          if (strcmp(subaction, "faster") == 0) mod->faster();
          else if (strcmp(subaction, "slower") == 0) mod->slower();
          else if (strcmp(subaction, "bigger") == 0) mod->bigger();
          else if (strcmp(subaction, "smaller") == 0) mod->smaller();
        }

      }

    }

    K32_system *system;
    K32_wifi *wifi;
    K32_audio *audio;
    K32_light *light;
    K32_remote *remote;

    void splitString(char *data, const char *separator, int index, char *result)
    {
      char input[strlen(data)];
      strcpy(input, data);

      char *command = strtok(input, separator);
      for (int k = 0; k < index; k++)
        if (command != NULL)
          command = strtok(NULL, separator);

      if (command == NULL)
        strcpy(result, "");
      else
        strcpy(result, command);
    }

  private:

};



#endif