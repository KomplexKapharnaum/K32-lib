/*
  K32_fixture.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_fixture_h
#define K32_fixture_h

#define FIXTURE_MAXPIXEL 512

#include <Arduino.h>


#include "_libfast/crgbw.h"


class K32_fixture {
  public:
    K32_fixture(int size);

    int size();
    
    void lock();
    void unlock();
    bool dirty();
    
    K32_fixture* clear();
    K32_fixture* black();
    K32_fixture* all(pixelColor_t color);
    K32_fixture* all(int red, int green, int blue, int white = 0);
    K32_fixture* pix(int pixel, pixelColor_t color);
    K32_fixture* pix(int pixelStart, int count, pixelColor_t color);
    K32_fixture* pix(int pixel, int red, int green, int blue, int white = 0);

    void getBuffer(pixelColor_t* buffer, int size, int offset=0);
    void setBuffer(pixelColor_t* buffer, int size, int offset=0);

    virtual void show();

  protected:

    virtual void draw();

    bool _dirty;
    pixelColor_t* _buffer;
    SemaphoreHandle_t buffer_lock;
    SemaphoreHandle_t show_lock;
    SemaphoreHandle_t draw_lock;

  private:
    int _size = 0;
    static void task( void * parameter );

};

#endif
