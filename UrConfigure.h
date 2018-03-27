/*
 * UrConfigure.h
 *
 *  Created on: 2018年1月27日
 *      Author: dclab
 */

#ifndef URCONFIGURE_H_
#define URCONFIGURE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "board.h"

/*--- Shared Memmory ---*/
// Enable
#define CFG_SEM_EN              			(0)
#define CFG_MUTEX_EN              			(0)
#define CFG_QUEUE_EN              			(0)
#define CFG_MAILBOX_EN              		(0)
#define CFG_FLAG_EN  	            		(0)

// Structure Size
#define SHARED_SIZE_HEAP 					0x08000
#define SHARED_SIZE_TASK					(sizeof(TASK))
#define SHARED_SIZE_PTASK					(sizeof(P_TASK))
#define SHARED_SIZE_SEM						(sizeof(SEM))
#define SHARED_SIZE_MUTEX					(sizeof(MUTEX))
#define SHARED_SIZE_QUEUE					(sizeof(QUEUE))
#define SHARED_SIZE_MAILBOX					(sizeof(MAILBOX))
#define SHARED_SIZE_FLAG					(sizeof(FLAG))

// Structure List Size
#define CFG_MAX_TASK 						(10)

#if CFG_SEM_EN >0
	#define CFG_MAX_SEM 					(5)
#else
	#define CFG_MAX_SEM 					(0)
#endif

#if CFG_MUTEX_EN >0
	#define CFG_MAX_MUTEX 					(5)
#else
	#define CFG_MAX_MUTEX 					(0)
#endif

#if CFG_QUEUE_EN >0
	#define CFG_MAX_QUEUE 					(5)
#else
	#define CFG_MAX_QUEUE 					(0)
#endif

#if CFG_MAILBOX_EN >0
	#define CFG_MAX_MAILBOX 				(15)
#else
	#define CFG_MAX_MAILBOX 				(0)
#endif

#if CFG_FLAG_EN >0
	#define CFG_MAX_FLAG 					(5)
#else
	#define CFG_MAX_FLAG 					(0)
#endif


/*
BASE 				0x38000
CPUFLAG 			0x38000
LOCK 				0x38004
TASK 				0x38008
TASKRDY 			0x381C0
TASKRDY_PRIO_HEAD 	0x381C4
TASKRDY_PRIO_END 	0x385C4
TASKDLY 			0x389C4
SEM 				0x389C8
MUTEX 				0x389F0
QUEUE 				0x389F0
MAILBOX 			0x389F0
FLAG 				0x38BD0
HEAP 				0x48BD0
OS_DEF_END 			0x48BD0

*/

// CPUFLAG
#define CORE_NUM							(2)

// Base
#define SHARED_BASE							(0x38004)	
#define SHARED_BASE_CPUFLAG					SHARED_BASE								// 4  bit 0x38000

#define SHARED_BASE_LOCK 					(SHARED_BASE_CPUFLAG			+ 4	)	// 32 bit 0x38004
#define SHARED_BASE_TASK					(SHARED_BASE_LOCK				+ 4	)	// 0x38008
#define SHARED_BASE_TASKRDY					(SHARED_BASE_TASK 				+ (SHARED_SIZE_TASK		* CFG_MAX_TASK)		)
#define SHARED_BASE_TASKRDY_PRIO_HEAD		(SHARED_BASE_TASKRDY			+ SHARED_SIZE_PTASK							)
#define SHARED_BASE_TASKRDY_PRIO_END		(SHARED_BASE_TASKRDY_PRIO_HEAD	+ (SHARED_SIZE_PTASK	* 256)				)
#define SHARED_BASE_TASKDLY					(SHARED_BASE_TASKRDY_PRIO_END	+ (SHARED_SIZE_PTASK	* 256)				)
#define SHARED_BASE_SEM						(SHARED_BASE_TASKDLY			+ SHARED_SIZE_PTASK							)
#define SHARED_BASE_MUTEX					(SHARED_BASE_SEM 				+ (SHARED_SIZE_SEM		* CFG_MAX_SEM)		)
#define SHARED_BASE_QUEUE					(SHARED_BASE_MUTEX 				+ (SHARED_SIZE_MUTEX	* CFG_MAX_MUTEX)	)
#define SHARED_BASE_MAILBOX					(SHARED_BASE_QUEUE 				+ (SHARED_SIZE_QUEUE	* CFG_MAX_QUEUE)	)
#define SHARED_BASE_FLAG					(SHARED_BASE_MAILBOX 			+ (SHARED_SIZE_MAILBOX	* CFG_MAX_MAILBOX)	)
#define SHARED_BASE_HEAP					(SHARED_BASE_FLAG				+ (SHARED_SIZE_FLAG		* CFG_MAX_FLAG)		)

#define SHARED_OS_DEF_END					(SHARED_BASE_HEAP 				+ SHARED_SIZE_HEAP							)

// Task
#define CFG_STACK_SIZE						(512)
#define CFG_TIME_SLICE						(10)

// Mail Box
#define CFG_MSG_SIZE						(20)

// FreeRTOS Configure
#define configUSE_PREEMPTION				1
#define configUSE_IDLE_HOOK					0
#define configUSE_TICK_HOOK					0
#define configCPU_CLOCK_HZ					( ( unsigned long ) 50000000 )	
#define configTICK_RATE_HZ					( ( portTickType ) 50 )
#define configMINIMAL_STACK_SIZE			( ( unsigned portSHORT ) 2048 )
#define configTOTAL_HEAP_SIZE				( ( size_t ) ( 70 * 1024 ) )
#define configMAX_TASK_NAME_LEN				( 32 )
#define configUSE_TRACE_FACILITY			0
#define configUSE_16_BIT_TICKS				0
#define configIDLE_SHOULD_YIELD				0
#define configUSE_MUTEXES					1
#define configUSE_RECURSIVE_MUTEXES			0
#define configQUEUE_REGISTRY_SIZE			1
#define configUSE_MALLOC_FAILED_HOOK		1
#define configUSE_APPLICATION_TASK_TAG		1
#define configUSE_COUNTING_SEMAPHORES		1
#define configMAX_PRIORITIES				( ( unsigned portBASE_TYPE ) 10 )

#define configGENERATE_RUN_TIME_STATS		0
#define configCHECK_FOR_STACK_OVERFLOW		0

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES 				0
#define configMAX_CO_ROUTINE_PRIORITIES 	( 2 )

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */

#define INCLUDE_vTaskPrioritySet			1
#define INCLUDE_uxTaskPriorityGet			1
#define INCLUDE_vTaskDelete					1
#define INCLUDE_vTaskCleanUpResources		0
#define INCLUDE_vTaskSuspend				1
#define INCLUDE_vTaskDelayUntil				1
#define INCLUDE_vTaskDelay					1
#define INCLUDE_uxTaskGetStackHighWaterMark	1
#define INCLUDE_xTaskGetSchedulerState		1

#ifdef __cplusplus
}
#endif

#endif /* URCONFIGURE_H_ */
