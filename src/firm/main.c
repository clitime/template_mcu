#include "stm32f4xx.h"
#include "inout.h"
#include "mainTask.h"
#include "ethernetTestTask.h"

#include "debugTask.h"

#include "FreeRTOS.h"
#include "task.h"


int main(void) __attribute__((noreturn));


int main(void) {
    configInput();
    configOutput();

    initializeDebug();

    xTaskCreate(mainTask, "main", configMINIMAL_STACK_SIZE * 1, NULL, MAIN_TASK_PRIO, NULL);
    xTaskCreate(ethernetTask, "eth", configMINIMAL_STACK_SIZE * 1, NULL, MAIN_TASK_PRIO, NULL);

    vTaskStartScheduler();
    while (1) {
        continue;
    }
}
