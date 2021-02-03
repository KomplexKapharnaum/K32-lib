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
      if (type == STR) return -10; //return atoi(argStr); 
      return argInt;
    }
    const char* toStr() {
      // if (type == INT && !strInited) {
      //   char str[33];
      //   sprintf(str, "%d", argInt);
      //   argStr = (char *) malloc(strlen(str) + 1); 
      //   strcpy(argStr, str);
      //   strInited = true;
      // }
      // return argStr; 
      return "ok";
    }

    // ~argX() {
    //   // if (strInited) free(argStr);
    //   // LOG("ARG destroyed");
    // }
};


class Orderz 
{
  public:
    Orderz(char* command) {
      splitString(command, "/", 0, engine);
      splitString(command, "/", 1, action);
      splitString(command, "/", 2, subaction);
    }

    void addData(int value) {
      // _data[_dataCount] = new argX(value);
      _data[_dataCount] = value;
      _dataCount++;
    }

    void addData(const char* value) {
      // _data[_dataCount] = new argX(value);
      _data[_dataCount] = -10;
      _dataCount++;
    }

    argX* getData(int index) {
      // if (index<_dataCount) return _data[index];
      // else 
      return NULL;
    }

    int dataCount() {
      return _dataCount;
    }

    char engine[8];
    char action[8];
    char subaction[8];

    // ~Orderz() {
    //   // for(int k=0; k<_dataCount; k++) delete(_data[k]);
    //   // LOG("Orderz destroyed");
    // }
    int _dataCount = 0;
    // argX* _data[16];
    int _data[16];

  private:


    void splitString(char *data, const char *separator, int index, char *result)
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
        orderzQueue = xQueueCreate( 10, sizeof( Orderz ) );
      }

    void queue(Orderz* order)
    {
      LOGINL("COM: queue ");
      LOGINL(order->engine);
      LOGINL(" ");
      LOGINL(order->getData(0)->toStr());
      LOGINL(" ");
      LOGINL(order->getData(1)->toStr());
      LOG();

      xQueueSend(orderzQueue, &order, portMAX_DELAY);
    }

    Orderz* next() {
      Orderz* nextOrder;
      if ( xQueueReceive(orderzQueue, &nextOrder, portMAX_DELAY) ) return nextOrder;
      else return NULL;
    }

  private:
    QueueHandle_t orderzQueue;
};



#endif