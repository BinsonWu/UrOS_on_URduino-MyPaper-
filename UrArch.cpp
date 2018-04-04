/*
 * UrArch.c
 *
 *  Created on: 2018年1月28日
 *      Author: dclab
 */

#include "UrOSInclude.h"

#include "URduino_API.h"
#include "comm.h"
#include "portmacro.h"

/*---------------------------- Variable declare ------------------------------*/
volatile U64 OSTickCnt = 0;          	/*!< Counter for current system ticks.    	*/
volatile U64 OSScheduleTime = 0;		/*!< When to Schedule 						*/

U64 getCoreTick()
{
	U64 *p;
	if(core_id == 0)
	{
		p = SHARED_BASE_CORE0TICK;
	}
	if(core_id == 1)
	{
		p = SHARED_BASE_CORE1TICK;
	}
	return *p;
}

void setCoreTick(U64 TickCount)
{
	U64 *p;
	if(core_id == 0)
	{
		p = SHARED_BASE_CORE0TICK;
	}
	if(core_id == 1)
	{
		p = SHARED_BASE_CORE1TICK;
	}
	*p = TickCount;
}

void UrDisableInterrupts()
{
	portDISABLE_INTERRUPTS();
}

S32 UrStartSchedule()
{
	setCoreTick((U64)0);
	OSTickCnt = 0;
	return xPortStartScheduler();
}

/*!< Initial context of task being created	*/
OS_STK  *UrInitTaskContext(FUNCPtr task,void *param,OS_STK *pstk)
{
	return pxPortInitialiseStack( pstk, task, param );
}

void    UrSwitchContext(void)         /*!< Switch context                   */
{
	if(OSSchedLock > 0)
		return;
	while( UrGetCoreFlag() != core_id ){}
UrSchedLock();
UrTaskLock();
	// Get Next Task
	P_TASK newTask 	= getTaskRdy();
	// If has Next Task . if not , return.
	if(newTask == Ur_NULL)
	{
UrTaskUnlock();
UrSchedUnlock();
			return;
	}
	
	/*if(getCoreTick()%100 == 0)
	{
		print("tc=");
		print(getCoreTick());
		println("");
	}*/
	
	// If TaskRunning is TASK_WAITING ( Not TASK_RUNNING )
	if(TaskRunning->state != TASK_RUNNING)
	{
		print("wt=");
		print(TaskRunning->wakeTick);
		println("");
		// Disconnect Next Task From Ready List
		RemoveFromTaskRdyList(newTask);
		newTask->state 	= TASK_RUNNING;
		// Set New Task
		TaskRunning 	= newTask;
		// Set New OSScheduleTime
		OSScheduleTime = getCoreTick() + newTask->timeSlice;
		/*print("st=");
		print(OSScheduleTime);
		println("");*/
	}
	// If newTask's priority < TaskRunning
	else if( newTask->prio < TaskRunning->prio )
	{
		/*println("2");
		print(TaskRunning->taskID);
		print("->");
		print(newTask->taskID);
		println("");*/
		
		// Disconnect Next Task From Ready List
		RemoveFromTaskRdyList(newTask);
		newTask->state 	= TASK_RUNNING;
		UrInsertToTaskRdyList(TaskRunning);
		// Set New Task
		TaskRunning 	= newTask;
		// Set New OSScheduleTime
		OSScheduleTime = getCoreTick() + newTask->timeSlice;
	}
	// Round Robin
	else if( newTask->prio == TaskRunning->prio && getCoreTick() >= OSScheduleTime )
	{
		/*println("3");
		print(TaskRunning->taskID);
		print("->");
		print(newTask->taskID);
		println("");*/
		
		// Disconnect Next Task From Ready List
		RemoveFromTaskRdyList(newTask);
		newTask->state 	= TASK_RUNNING;
		UrInsertToTaskRdyList(TaskRunning);
		// Set New Task
		TaskRunning 	= newTask;
		// Set New OSScheduleTime
		OSScheduleTime = getCoreTick() + newTask->timeSlice;
	}
UrTaskUnlock();
UrSchedUnlock();
	UrSetCoreFlag( (UrGetCoreFlag()+1) % CORE_NUM );
	if(core_id == 0)
	{
		while( UrGetCoreFlag() != core_id ){}
	}
}

void    UrTimeDispose(void)     /*!< Time dispose function.               */
{
UrSchedLock();
UrTaskLock();
	setCoreTick( getCoreTick()+1 );
	OSTickCnt++;
	UrTaskTimeDispose();
UrTaskUnlock();
UrSchedUnlock();
}

void  UrSchedule(void)
{
	// mtspr(SPR_TTMR, mfspr(SPR_TTMR) & ~(SPR_TTMR_IP));
	asm(
		"l.jal UrSchedule_ASM;"
		"l.nop;"
	);
}

