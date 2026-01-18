#ifndef INC_MAIN_H_
#define INC_MAIN_H_

// Device header
#include "events.h"
#include "stm32f0xx.h"

// BSP functions
#include "bsp.h"

// FreeRTOS headers
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "semphr.h"
#include "queue.h"
#include "event_groups.h"
#include "stream_buffer.h"
#include "stm32f0xx.h"
#include "bsp.h"
#include "delay.h"
#include "factory_io.h"

// Global functions
int my_printf	(const char *format, ...);
int my_sprintf	(char *out, const char *format, ...);

typedef struct
{
	//xSemaphoreHandle *sem_id; // Semaphore ID to use for publication
	uint8_t sem_id;
	uint32_t sensor_id; // Awaited sensor ID
	uint32_t sensor_state; // Awaited sensor State
} subscribe_message_t;

typedef struct
{
	uint32_t mask; // Awaited mask
	uint32_t state; // Awaited change
} command_message_t;

// variable a definir
#define SIZE 4
#define SENSOR_TABLE_SIZE 14

// Global variables
extern uint8_t sensor[SENSOR_TABLE_SIZE];

// Queue
extern xQueueHandle xComQueue;
extern xQueueHandle xSubcribeQueue;

// Semaphores
extern xSemaphoreHandle xSem_UART_TC;
extern xSemaphoreHandle xSem_DMA_TC;
extern xSemaphoreHandle xSem1;
extern xSemaphoreHandle xSem2;
extern xSemaphoreHandle xSemCarton;
extern xSemaphoreHandle xSemPorte;
extern xSemaphoreHandle xSemAscenseur;

// Mutex
extern xSemaphoreHandle xConsoleMutex;




#endif /* INC_MAIN_H_ */
