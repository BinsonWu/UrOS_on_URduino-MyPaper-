/*
 * Core.c
 *
 *  Created on: 2018年1月28日
 *      Author: dclab
 */

#include "UrOSInclude.h"
#include "URduino_API.h"
#include "comm.h"
#include "portmacro.h"

volatile int core_id = -1;

/*---------------------------- Variable Define -------------------------------*/
volatile U8 OSSchedLock   = 1;         /*!< Task Switch lock.                      */

void UrSetCoreID(int id)
{
	core_id = id;
}

int  UrGetCoreID()
{
	return core_id;
}

void UrSetCoreFlag(U32 nextCPU)
{
	U32 *p = SHARED_BASE_CPUFLAG;
	*p = nextCPU;
}

U32 UrGetCoreFlag()
{
	U32 *p = SHARED_BASE_CPUFLAG;
	return	*p;
}

void UrSchedLock(void)
{
	while( OSSchedLock > 0){}
	OSSchedLock++;
}

void UrSchedUnlock(void)
{
	if( OSSchedLock > 0)
		OSSchedLock--;
	if(TaskSchedReq == Ur_TRUE)
	{
		TaskSchedReq = Ur_FALSE;
		//while(TaskRunning->state != TASK_RUNNING){}
		//UrSchedule();
	}
}

void printSharedBase(void)
{
	println("< -------------- Share Configure Table -------------- >");
	print("BASE \t\t\t\t");
	printh((U32)SHARED_BASE);
	print("CPUFLAG \t\t\t\t");
	printh((U32)SHARED_BASE_CPUFLAG);
	print("LOCK \t\t\t\t");
	printh((U32)SHARED_BASE_LOCK);
	print("TASK \t\t\t\t");
	printh((U32)SHARED_BASE_TASK);
	print("TASKRDY \t\t\t\t");
	printh((U32)SHARED_BASE_TASKRDY);
	print("TASKRDY_PRIO_HEAD \t");
	printh((U32)SHARED_BASE_TASKRDY_PRIO_HEAD);
	print("TASKRDY_PRIO_END \t\t");
	printh((U32)SHARED_BASE_TASKRDY_PRIO_END);
	print("TASKDLY \t\t\t\t");
	printh((U32)SHARED_BASE_TASKDLY);
	print("SEM \t\t\t\t\t");
	printh((U32)SHARED_BASE_SEM);
	print("MUTEX \t\t\t\t");
	printh((U32)SHARED_BASE_MUTEX);
	print("MAILBOX \t\t\t\t");
	printh((U32)SHARED_BASE_MAILBOX);
	print("FLAG \t\t\t\t");
	printh((U32)SHARED_BASE_FLAG);
	print("HEAP \t\t\t\t");
	printh((U32)SHARED_BASE_HEAP);
	print("OS_DEF_END \t\t\t");
	printh((U32)SHARED_OS_DEF_END);
	println("");
}


void UrConfigureSharedInit(void)
{
	println(">> UrConfigureSharedInit");
	
	println("< -------------- Init Configure -------------- >");
	U32 *p 	= SHARED_BASE_CPUFLAG;
	*p 		= 0;
	println("Init CPU Flag");
	print("\t");
	printh((U32)p);
	
	p 		= SHARED_BASE_LOCK;
	*p 		= 0;
	println("Init Lock");
	print("\t");
	printh((U32)p);
	
	p 		= SHARED_BASE_TASKRDY;
	*p 		= 0;
	println("Init Ready Queue");
	print("\t");
	printh((U32)p);
	
	p 		= SHARED_BASE_TASKDLY;
	*p 		= 0;
	println("Init Delay Queue");
	print("\t");
	printh((U32)p);
	
	println("Init Ready Queue Head and End");
	println("\tReady Queue Head");
	print("\t");
	printh((U32)SHARED_BASE_TASKRDY_PRIO_HEAD);
	println("\tReady Queue End");
	print("\t");
	printh((U32)SHARED_BASE_TASKRDY_PRIO_END);
	for(int i=0;i<256;i++)
	{
		p 		= SHARED_BASE_TASKRDY_PRIO_HEAD;
		*(p+i) 	= 0;
		p 		= SHARED_BASE_TASKRDY_PRIO_END;
		*(p+i) 	= 0;
	}
	
	println("");
}


void UrInitOS(void)
{
	println(">> UrInitOS");
	println("");
	UrConfigureSharedInit();
	UrInitTask();

#if CFG_SEM_EN >0
	UrInitSem();
#endif
#if CFG_MUTEX_EN >0
	UrInitMutex();
#endif
#if CFG_MAILBOX_EN >0
	UrInitMailBox();
#endif
#if CFG_FLAG_EN >0
	UrInitFlag();
#endif

}

void UrStartOS(void)
{
	if(core_id == 0)
	{
		println(">> UrStartOS");
		println("");
	}
	
	// Core 0 do first.
	// When Core Flag is Set to 1,then Core 1 do.
	UrDisableInterrupts();
	
	//while( UrGetCoreFlag() != core_id ){}
	
UrTaskLock();
	P_TASK newTask 	= getTaskRdy();
	if(newTask == Ur_NULL)
	{
UrTaskUnlock();
		return;
	}
	RemoveFromTaskRdyList(newTask);
	newTask->state 	= TASK_RUNNING;
	TaskRunning 	= newTask;
	UrSetOSScheduleTime(TaskRunning->timeSlice);
	/*print("st=");
	print(OSScheduleTime);
	println("");*/
UrTaskUnlock();
	
	if(core_id == 0)
	{
		println(">> StarOther");
	}
	UrSetCoreFlag( (UrGetCoreFlag()+1) % CORE_NUM );
	OSSchedLock = 0;
	
	if(core_id == 0)
	{
		// Wait for other Core Start.
		//while( UrGetCoreFlag() != core_id ){}
		println("\t End");
		println("");
		println(">> UrStartSchedule");
		println("");
		println("< ----------------------------- >");
	}
	if( UrStartSchedule() )
	{
		/* Should not reach here as if the scheduler is running the
		function will not return. */
	}
	else
	{
		/* Should only reach here if a task calls xTaskEndScheduler(). */
	}
}


