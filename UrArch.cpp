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

void UrSetOSTickCnt(U64 newOSTickCnt)
{
	OSTickCnt = newOSTickCnt;
}

U64 UrGetOSTickCnt()
{
	return OSTickCnt;
}

void UrSetOSScheduleTime(U64 newOSScheduleTime)
{
	OSScheduleTime = newOSScheduleTime;
}

U64 UrGetOSScheduleTime()
{
	return OSScheduleTime;
}

void UrDisableInterrupts()
{
	portDISABLE_INTERRUPTS();
}

S32 UrStartSchedule()
{
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
	//println("UrSwitchContext");
	if(OSSchedLock > 0)
		return;
	//while( UrGetCoreFlag() != core_id ){}
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
	
	// If any Task is waked up by Event.
	if(TaskWakeUp != Ur_NULL)
	{
		//println("TaskWakeUp");
		TaskWakeUp->state 	= TASK_RUNNING;
		UrInsertToTaskRdyList(TaskRunning);
		// Set New Task
		TaskRunning 		= TaskWakeUp;
		// Set New OSScheduleTime
		OSScheduleTime 		= OSTickCnt + TaskRunning->timeSlice;
		TaskWakeUp = Ur_NULL;
	}
	// If TaskRunning is TASK_WAITING ( Not TASK_RUNNING )
	else if(TaskRunning->state != TASK_RUNNING)
	{
		/*print("wt=");
		print(TaskRunning->wakeTick);
		println("");*/
		// Disconnect Next Task From Ready List
		RemoveFromTaskRdyList(newTask);
		newTask->state 	= TASK_RUNNING;
		// Set New Task
		TaskRunning 	= newTask;
		// Set New OSScheduleTime
		OSScheduleTime = OSTickCnt + newTask->timeSlice;
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
		OSScheduleTime = OSTickCnt + newTask->timeSlice;
	}
	// Round Robin
	else if( newTask->prio == TaskRunning->prio && OSTickCnt >= OSScheduleTime )
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
		OSScheduleTime = OSTickCnt + newTask->timeSlice;
	}
UrTaskUnlock();
UrSchedUnlock();
	UrSetCoreFlag( (UrGetCoreFlag()+1) % CORE_NUM );
}

void    UrTimeDispose(void)     /*!< Time dispose function.               */
{
UrSchedLock();
UrTaskLock();
	OSTickCnt++;
	UrTaskTimeDispose();
UrTaskUnlock();
UrSchedUnlock();
}

void  UrSchedule(void)
{
	UrDisableInterrupts();
	asm(
		"l.jal UrSchedule_ASM;"
		"l.nop;"
	);
}

