#include "stm32f4xx.h"
#include "inout.h"

int main(void) __attribute__((noreturn));


int main(void) {
    configInput();
    configOutput();

    while (1) {
        continue;
    }
}
