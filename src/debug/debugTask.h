#ifndef DEBUG_TASK_H_
#define DEBUG_TASK_H_


#include <stdio.h>


#ifdef DEBUG_PORT
    #define assertLog(x)                                                    \
    do {                                                                    \
        if (!(x)) {                                                         \
            printf(__FILE__);                                               \
            printf("\t");                                                   \
            printf("%d", __LINE__);                                         \
            printf("\t");                                                   \
            printf(__func__);                                               \
            printf(" : assertion failed: " #x);                             \
        }                                                                   \
    } while(0)

    #define dPrintf(x)                                                      \
    do {                                                                    \
        printf(x);                                                          \
    } while(0)

    #define dmPrintf(enable, msg)                                           \
    do {                                                                    \
        if (enable) {                                                       \
            printf(__FILE__);                                               \
            printf("\t");                                                   \
            printf("%d", __LINE__);                                         \
            printf("\t");                                                   \
            printf(__func__);                                               \
            printf(" : " msg);                                              \
        }                                                                   \
    } while(0)

    void initializeDebug(void);
#else
    #define assertLog(x)
    #define dPrintf(x)
    #define dmPrintf(en, msg)
    #define initializeDebug()
#endif


#endif
