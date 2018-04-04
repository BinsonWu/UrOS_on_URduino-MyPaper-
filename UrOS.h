/*
 * UrOS.h
 *
 *  Created on: 2018年1月27日
 *      Author: dclab
 */

#ifndef UROS_H_
#define UROS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "UrConfigure.h"

/*---------------------------- Type Define  ----------------------------------*/
typedef signed   char      			S8;
typedef unsigned char      			U8;
typedef short              			S16;
typedef unsigned short     			U16;
typedef long int               		S32;
typedef unsigned long int     		U32;
typedef long long          			S64;
typedef unsigned long long 			U64;
typedef unsigned char      			BYTE;
typedef unsigned char      			BOOL;
typedef U32					       	OS_STK;
typedef U8                	 		OS_ID;
typedef U8                 			StatusType;
typedef U16                			OS_VER;
typedef void               			(*FUNCPtr)(void*);
typedef void               			(*vFUNCPtr)(void);

// From FreeRTOS
#define REDZONE_SIZE		(128)
#define CONTEXT_SIZE		(128)
#define STACKFRAME_SIZE		(CONTEXT_SIZE + REDZONE_SIZE)

/*---------------------------- Constant Define -------------------------------*/
#ifndef 	Ur_NULL
#define 	Ur_NULL          ((void *)0)
#endif

#ifndef 	Ur_FALSE
#define 	Ur_FALSE         (0)
#endif

#ifndef 	Ur_TRUE
#define 	Ur_TRUE          (1)
#endif
/*----------------------------------- Invalid --------------------------------------*/
#define  INVALID_ID     		(U8)0xff
#define  INVALID_Prio     		(U8)0xff
#define  INVALID_VALUE  		0xffffffff
#define  INVALID_TYPE  			(U8)0xff
#define  INVALID_TICK  			(U32)0xffffffff

/*---------------------------- Error Codes   ---------------------------------*/
#define E_CREATE_FAIL         	(StatusType)-1
#define E_OK                  	(StatusType)0
#define E_INVALID_ID          	(StatusType)1
#define E_INVALID_PARAMETER   	(StatusType)2
#define E_CALL                	(StatusType)3
#define E_TASK_WAITING        	(StatusType)4
#define E_TIMEOUT             	(StatusType)5
#define E_SEM_FULL            	(StatusType)6
#define E_MBOX_FULL           	(StatusType)7
#define E_QUEUE_FULL          	(StatusType)8
#define E_SEM_EMPTY           	(StatusType)9
#define E_MBOX_EMPTY          	(StatusType)10
#define E_QUEUE_EMPTY         	(StatusType)11
#define E_FLAG_NOT_READY      	(StatusType)12
#define E_ALREADY_IN_WAITING  	(StatusType)13
#define E_TASK_NOT_WAITING    	(StatusType)14
#define E_TASK_WAIT_OTHER     	(StatusType)15
#define E_EXCEED_MAX_NUM      	(StatusType)16
#define E_NOT_IN_DELAY_LIST   	(StatusType)17
#define E_SEV_REQ_FULL        	(StatusType)18
#define E_NOT_FREE_ALL        	(StatusType)19
#define E_PROTECTED_TASK      	(StatusType)20
#define E_OS_IN_LOCK          	(StatusType)21

#define E_MUTEX_DIF_SECTION	 	(StatusType)22

/*---------------------------- Wait Opreation type  --------------------------*/
// Flag
#define OPT_WAIT_ALL          			0         		/*!< Wait for all flags.              */
#define OPT_WAIT_ANY          			1         		/*!< Wait for any one of flags.       */
#define OPT_WAIT_ONE          			2         		/*!< Waot for one flag.               */

// Sem
#define OPT_WAIT_FOREVER				-1

/*---------------------------- Delete Opreation type  ------------------------*/
#define OPT_DEL_NO_PEND       			0         		/*!< Delete when no task waitting for */
#define OPT_DEL_ANYWAY        			1        		/*!< Delete always.                   */

/*---------------------------- Timer Types  ----------------------------------*/
#define TMR_TYPE_ONE_SHOT     			0         		/*!< Timer counter type: One-shot     */
#define TMR_TYPE_PERIODIC     			1         		/*!< Timer counter type: Periodic     */

/*---------------------------- Event Control ---------------------------------*/
#define EVENT_SORT_TYPE_FIFO  		(U8)0x01 		 	/*!< Insert a event by FIFO           */
#define EVENT_SORT_TYPE_PRIO  		(U8)0x02  			/*!< Insert a event by prio           */


/*---------------------------- Task Status -----------------------------------*/
#define  TASK_READY     		0              	 		/*!< Ready status of task.            */
#define  TASK_RUNNING   		1               		/*!< Running status of task.          */
#define  TASK_WAITING   		2               		/*!< Waitting status of task.         */
#define  TASK_DORMANT   		3               		/*!< Dormant status of task.          */

#define  MAGIC_WORD     		0x5a5aa5a5

#define CFG_IDLE_STACK_SIZE     CFG_STACK_SIZE
#define CFG_MAX_USER_TASKS      5

#define  SYS_TASK_NUM   		2             			/*!< System task number.              */

/*--------------------------- Mutex Status ----------------------------------*/
#define   MUTEX_FREE        0           /*!< Mutex is free                    */
#define   MUTEX_OCCUPY      1           /*!< Mutex is occupy                  */
#define   WAITING_MUTEX     0x80

// Lock Shift
enum {	LOCK_SHIFT_HEAP = 0,
		LOCK_SHIFT_TASK,
		LOCK_SHIFT_SEM,
		LOCK_SHIFT_MUTEX,
		LOCK_SHIFT_QUEUE,
		LOCK_SHIFT_MAILBOX,
		LOCK_SHIFT_FLAG};

// Wait event type
enum {	EVENT_TYPE_SEM = 0,
		EVENT_TYPE_MUTEX,
		EVENT_TYPE_QUEUE,
		EVENT_TYPE_MAILBOX,
		EVENT_TYPE_FLAG};


#ifdef __cplusplus
}
#endif

#endif /* UROS_H_ */
