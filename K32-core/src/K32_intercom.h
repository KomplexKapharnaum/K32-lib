/*
  K32_intercom.h
  Created by Thomas BOHL, january 2020.
  Released under GPL v3.0
*/
#ifndef K32_intercom_h
#define K32_intercom_h

#include "Arduino.h"
#include "utils/K32_log.h"
#include "freertos/ringbuf.h"
#include <EventEmitter.h>

#define ORDERZ_MAX_ARGS 32

enum argType { INT, STR };
class argX
{ 
  public:
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
    int argInt = 0;
    bool strInited = false;
    
    int toInt() { 
      if (type == STR) return atoi(argStr); 
      return argInt;
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


class Orderz 
{
  public:
    Orderz() {}

    ~Orderz() {
      clear();
    }

    Orderz(const char* command, bool isCmd = false) : isCmd(isCmd) {
      set(command);
    }

    Orderz(String command, bool isCmd = false) : isCmd(isCmd) {
      set(command.c_str());
    }

    Orderz* set(const char* command) {
      clear();
      splitString(command, "/", 0, engine);
      splitString(command, "/", 1, action);
      splitString(command, "/", 2, subaction);

      strcpy (engine_action, engine);
      strcat (engine_action,"/");
      strcat (engine_action, action);

      workable = true;
      return this;
    }

    void addData(int value) {
      if (dataCount < ORDERZ_MAX_ARGS) {
        data[dataCount] = new argX(value);
        dataCount++;
      }
    }

    void addData(const char* value) {
      if (dataCount < ORDERZ_MAX_ARGS) {
        data[dataCount] = new argX(value);
        dataCount++;
      }
    }

    int count() {
      return dataCount;
    }

    argX* getData(int index) {
      if (index < dataCount) return data[index];
      else return new argX("");
    }

    void clear() {
      for (int k=0; k<dataCount; k++) {
        // LOGF("--Intercom: Order deleting arg %d \n", k);
        delete(data[k]);
      }
      dataCount = 0;
      workable = false;
    }

    bool consume() {
      bool w = workable;
      workable = false;
      return w;
    }

    char engine[128];
    char action[128];
    char subaction[128];
    
    char engine_action[257];

    bool isCmd = false; // FALSE = Event, TRUE = Command

  private:

    bool workable = false;
    int dataCount = 0;
    argX* data[ORDERZ_MAX_ARGS];
    
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
};

class K32_intercom 
{
  public:
    K32_intercom() {
        Orderz order;
        orderzQueue = xQueueCreate( 10, sizeof(&order) );
        ee = new EventEmitter<Orderz*>();
      }

    void queue(Orderz* order)
    {
      xQueueSend(orderzQueue, (void*) &order, portMAX_DELAY);
    }

    Orderz* next() {
      Orderz* nextOrder;
      xQueueReceive(orderzQueue, &(nextOrder), portMAX_DELAY);
      return nextOrder;
    }

    EventEmitter<Orderz*>* ee;

  private:
    QueueHandle_t orderzQueue;
    
};



#endif