/*
  K32_timer_h.h
  Created by Thomas BOHL, february 2019.
  Released under GPL v3.0
*/
#ifndef K32_timer_h
#define K32_timer_h

#include <Arduino.h>
#include <Timer.h>


class K32_timer : public Timer {
    public:
        K32_timer() {
            // Update task
            xTaskCreate(this->task,       // function
              "ktimer",                     // task name
              2000,                         // stack memory
              (void *)this,                 // args
              0,                            // priority
              NULL);                        // handler
        }

        static void task(void *parameter)
        {
            K32_timer *that = (K32_timer *)parameter;
            while (true) {
                that->update();
                delay(1);
            }
            vTaskDelete(NULL);
        }
};

class K32_timeout {
    public:
        K32_timeout( int timeout, void (*callback)() ) 
        {
            this->timeout = timeout;
            this->callback = callback;

            // Update task
            xTaskCreate(this->task,       // function
              "ktimeout",                     // task name
              2000,                         // stack memory
              (void *)this,                 // args
              0,                            // priority
              &task_handle);                        // handler
        }


        void cancel() {
            this->canceled = true;
            if (task_handle) vTaskDelete(task_handle);
            task_handle = NULL;
        }

        TaskHandle_t task_handle = NULL;

    private:
        int timeout;
        void (*callback)();
        bool canceled = false;

        static void task(void *parameter)
        {
            K32_timeout *that = (K32_timeout *)parameter;
            delay(that->timeout);
            if (!that->canceled) that->callback();
            that->task_handle = NULL;
            vTaskDelete(NULL);
        }
};

#endif