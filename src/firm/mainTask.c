#include "mainTask.h"

#include "inout.h"

#include "FreeRTOS.h"
#include "task.h"

#include "debugTask.h"

#include "at24_eeprom.h"

static const uint8_t dataSize = 16;
uint8_t data[dataSize] = {0x00};
uint8_t data2[dataSize] = {0xff};

void mainTask(void *p) {
    (void)p;
    I2C_config();

    for (uint32_t i = 0, j = dataSize; i < dataSize; i++) {
        data2[i] = 0xff;
    }
    writeData(128, data2, dataSize);

    readData(128, data, dataSize);

    readData(128 + dataSize - 4, data, 3);

    for (uint8_t i = 0; i < dataSize; i++) {
        data2[i] = i;
    }
    writeData(3, data2, dataSize);

    readData(3, data, dataSize);

    readData(0, data, 3);

    readData(dataSize, data, 3);

    for (;;) {
        vTaskDelay(1000);
        toggleOutputState(OUT_LED_G);
    }
}
