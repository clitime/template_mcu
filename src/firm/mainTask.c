#include "mainTask.h"

#include "inout.h"

#include "FreeRTOS.h"
#include "task.h"

#include "debugTask.h"


void mainTask(void *p) {
    (void)p;

    for (;;) {
        vTaskDelay(1000);
        toggleOutputState(OUT_LED_G);
    }
}
