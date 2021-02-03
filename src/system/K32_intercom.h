/*
  K32_intercom.h
  Created by Thomas BOHL, january 2020.
  Released under GPL v3.0
*/
#ifndef K32_intercom_h
#define K32_intercom_h

#include "Arduino.h"
#include "system/K32_log.h"
#include <ArduinoJson.h>
#include "freertos/ringbuf.h"

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
    char* argStr = "";
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

    Orderz(const char* command) {
      reset(command);
    }

    Orderz* reset(const char* command) {
      clear();
      splitString(command, "/", 0, engine);
      splitString(command, "/", 1, action);
      splitString(command, "/", 2, subaction);
      workable = true;
      return this;
    }

    void addData(int value) {
      data[dataCount] = new argX(value);
      dataCount++;
    }

    void addData(const char* value) {
      data[dataCount] = new argX(value);
      dataCount++;
    }

    int count() {
      return dataCount;
    }

    argX* getData(int index) {
      return data[index];
    }

    void clear() {
      for (int k=0; k<dataCount; k++) delete(data[k]);
      dataCount = 0;
      workable = false;
    }

    bool consume() {
      bool w = workable;
      workable = false;
      return w;
    }

    char engine[8];
    char action[8];
    char subaction[8];
    

  private:

    bool workable = false;
    int dataCount = 0;
    argX* data[16];
    
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
        orderzQueue = xQueueCreate( 10, sizeof(Orderz) );
      }

    void queue(Orderz* order)
    {
      xQueueSend(orderzQueue, &(*order), portMAX_DELAY);
      delete(order);
    }


    Orderz next() {
      Orderz nextOrder;
      xQueueReceive(orderzQueue, &nextOrder, portMAX_DELAY);
      return nextOrder;
    }

  private:
    QueueHandle_t orderzQueue;

    
};



#endif