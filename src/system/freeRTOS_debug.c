#include "stm32f4xx.h"

#include "FreeRTOS.h"
#include "task.h"


void configureTimerForRunTimeStats( void ) {
	//Timer2 100 us for rtos stats
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	//freq for timers on APB1 84 MHz
	TIM2->PSC = 8400-1;	 					//tic 100 us
	TIM2->ARR = 0xffffffff;
	TIM2->CNT = 0xffffffff;
	TIM2->CR1 = TIM_CR1_CEN;			//timer on
}


uint32_t getRunTimeCounterValue( void ) {
	return TIM2->CNT;
}


void vApplicationMallocFailedHook(void) {
    while(1);
}


void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed portCHAR *pcTaskName) {
    (void)pxTask;
    (void)pcTaskName;
    while(1);
}


#if configSUPPORT_STATIC_ALLOCATION
	void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
										StackType_t **ppxIdleTaskStackBuffer,
										uint32_t *pulIdleTaskStackSize )
	{
		//If the buffers to be provided to the Idle task are declared inside this function then they must be
		//declared static - otherwise
		//they will be allocated on the stack and so not exists after this function exits.
		static StaticTask_t xIdleTaskTCB;
		static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

		//Pass out a pointer to the StaticTask_t structure in which the Idle task's state will be stored.
		*ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
		//Pass out the array that will be used as the Idle task's stack.
		*ppxIdleTaskStackBuffer = uxIdleTaskStack;
		//Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer. Note that, as the array is
		//necessarily of type StackType_t,
		//configMINIMAL_STACK_SIZE is specified in words, not bytes.
		*pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
	}
#endif


#if configSUPPORT_STATIC_ALLOCATION && configUSE_TIMERS
	static StaticTask_t xTimerTaskTCBBuffer;
	static StackType_t xTimerStack[configTIMER_TASK_STACK_DEPTH];

	/* If static allocation is supported then the application must provide the
	following callback function - which enables the application to optionally
	provide the memory that will be used by the timer task as the task's stack
	and TCB. */
	void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize) {
		*ppxTimerTaskTCBBuffer = &xTimerTaskTCBBuffer;
		*ppxTimerTaskStackBuffer = &xTimerStack[0];
		*pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
	}
#endif
