/*
 * UrTask.c
 *
 *  Created on: 2018年1月27日
 *      Author: dclab
 */

#include "UrOSInclude.h"
#include "comm.h"
#include "portmacro.h"

/*---------------------------- Variable declare ------------------------------*/
// save tcb ptr that created
volatile P_TASK TaskRunning 	= Ur_NULL;   /*!< A pointer to Task that is running.          				*/
volatile P_TASK TaskWakeUp 		= Ur_NULL;   /*!< A pointer to next Task that waked up by event.          	*/
volatile U8 TaskSchedReq 		= Ur_FALSE;

int __attribute__ ((section (".TASKCODE"))) TaskStack[CFG_MAX_TASK][CFG_STACK_SIZE];

void IdleTask1()
{
	while(1)
	{
		println("Idle 1");
		for(int i=0;i<500000;i++){}
	}
}

void IdleTask2()
{
	while(1)
	{
		println("Idle 2");
		for(int i=0;i<500000;i++){}
	}
}

/*
					< UrTask Architecture >
---------------------------------------------------------------
- Task Array
	Length : CFG_MAX_TASK
	|  0  | , |  1  | , ... , |  CFG_MAX_TASK-1  |
- TaskRunning
- TaskSchedReq
- Ready Queue : |    | -> |    | -> ... -> |    | -> |    | 
- Delay Queue : |    | -> |    | -> ... -> |    | -> |    | 

- Ready Queue Head Array :
	0	| Head of Queue 0 	|
	1	| Head of Queue 1	|
	...
	254 | Head of Queue 254	|
	255 | Head of Queue 255	|

- Ready Queue End Array :
	0	| End of Queue 0 	|
	1	| End of Queue 1	|
	...
	254 | End of Queue 254	|
	255 | End of Queue 255	|
*/

P_TASK UrGetTaskByID(OS_ID id)
{
	U32 *p = SHARED_BASE_TASK;
	P_TASK tempTask;
	p = p + id*SHARED_SIZE_TASK/4;
	tempTask = (P_TASK)p;
	return tempTask;
}

P_TASK getPrioHead(U8 prio)
{
	U32 *p = SHARED_BASE_TASKRDY_PRIO_HEAD;
	P_TASK tempTask;
	p = p + prio;
	tempTask = (P_TASK)*p;
	return tempTask;
}

P_TASK setPrioHead(U8 prio,U32 TaskAddr)
{
	U32 *p = SHARED_BASE_TASKRDY_PRIO_HEAD;
	p = p + prio;
	*p = TaskAddr;
}

P_TASK getPrioEnd(U8 prio)
{
	U32 *p = SHARED_BASE_TASKRDY_PRIO_END;
	P_TASK tempTask;
	p = p + prio;
	tempTask = (P_TASK)*p;
	return tempTask;
}

P_TASK setPrioEnd(U8 prio,U32 TaskAddr)
{
	U32 *p = SHARED_BASE_TASKRDY_PRIO_END;
	p = p + prio;
	*p = TaskAddr;
}

P_TASK getTaskRdy()
{
	U32 *p = SHARED_BASE_TASKRDY;
	P_TASK tempTask;
	tempTask = (P_TASK)*p;
	return tempTask;
}

void setTaskRdy(U32 TaskAddr)
{
	U32 *p = SHARED_BASE_TASKRDY;
	*p = TaskAddr;
}


P_TASK getTaskDly()
{
	U32 *p = SHARED_BASE_TASKDLY;
	P_TASK tempTask;
	tempTask = (P_TASK)*p;
	return tempTask;
}

void setTaskDly(U32 TaskAddr)
{
	U32 *p = SHARED_BASE_TASKDLY;
	*p = TaskAddr;
}


void UrTaskLock()
{
	U32 *p = SHARED_BASE_LOCK;
	while(*p & (1 << LOCK_SHIFT_TASK) ){}
	*p = *p | (1 << LOCK_SHIFT_TASK);
}

void UrTaskUnlock()
{
	U32 *p = SHARED_BASE_LOCK;
	if( *p & (1 << LOCK_SHIFT_TASK)  )
		*p = *p & (~(1 << LOCK_SHIFT_TASK));
}

void UrInitTask()
{
	println(">> UrInitTask");
	P_TASK tempTask;
	OS_ID i;
	
	// Init Task
	for(i=0;i<CFG_MAX_TASK;i++)
	{
		tempTask = UrGetTaskByID(i);		
		tempTask->stkPtr 			= Ur_NULL;              /*!< The current point of task.       						*/
		tempTask->prio 				= INVALID_Prio;         /*!< Task priority.                   						*/
		tempTask->state 			= TASK_DORMANT;         /*!< TaSk status.                     						*/
		tempTask->taskID			= INVALID_ID;           /*!< Task ID.                         						*/
		tempTask->eventID			= INVALID_ID;
		tempTask->eventType			= INVALID_TYPE;
		tempTask->stackStart 		= Ur_NULL;              /*!< The top point of task.           						*/
		tempTask->taskFuc			= Ur_NULL;
		tempTask->stackEnd			= Ur_NULL;
		tempTask->delayTick			= 0;         			/*!< The number of ticks which delay. 						*/
		tempTask->wakeTick			= 0;         			/*!< The number of ticks which delay. 						*/
		tempTask->timeSlice			= CFG_TIME_SLICE;		/*!< When Round Robin , How many tick to Context Switch.	*/
		
#if CFG_FLAG_EN > 0
		tempTask->taskFlagID		= i;
#endif 
		
		tempTask->Tasknext			= Ur_NULL;             	/*!< The pointer to next Task.         						*/
		tempTask->Taskprev			= Ur_NULL;              /*!< The pointer to prev Task.         						*/
		tempTask->nextWaiting		= Ur_NULL;
	}
	//UrCreateTask(IdleTask1,0,INVALID_Prio-1,50);
	//UrCreateTask(IdleTask2,0,INVALID_Prio-1,50);

	println("");
}

OS_ID UrCreateTask(FUNCPtr task,void *argv,U8 prio,U32 timeSlice)
{
	if(UrGetCoreID()!=0)
	{
		return ;
	}
	P_TASK tempTask;
	OS_STK *TopOfStack;
	OS_ID i;
UrSchedLock();
UrTaskLock();
	for(i=0;i<CFG_MAX_TASK;i++)
	{
		tempTask = UrGetTaskByID(i);
		if(tempTask->taskID == INVALID_ID)
		{
			tempTask->taskID 			= i;
			tempTask->prio				= prio;
			tempTask->state				= TASK_READY;
			tempTask->taskFuc			= task;
			tempTask->stackStart 		= TaskStack[i];
			tempTask->stackEnd			= &TaskStack[i][CFG_STACK_SIZE-1];
			
			if(timeSlice)
				tempTask->timeSlice			= timeSlice;
			else 
				tempTask->timeSlice			= CFG_TIME_SLICE;

			TopOfStack = tempTask->stackEnd;

			// Initial the stack and get the real top of stack
			tempTask->stkPtr 	= UrInitTaskContext( task, argv,TopOfStack );

			// Insert To Task RdyList
			UrInsertToTaskRdyList(tempTask);
UrTaskUnlock();
UrSchedUnlock();
			return i;                      /* Return mutex ID                    */
		}
	}
UrTaskUnlock();
UrSchedUnlock();
	return E_CREATE_FAIL;               /* No free mutex control block        */
}

/*
UrDelTask()
------------------------------------------------------------
1. Task state is TASK_READY or TASK_RUNNING or TASK_WAITING.
2. If task wait an Event.
*/
StatusType UrDelTask(OS_ID id)
{
    if(id >= CFG_MAX_TASK)
    {
        return E_INVALID_ID;
    }
UrSchedLock();
UrTaskLock();
    P_TASK tempTask = UrGetTaskByID(id);
	// Remove from Ready Queue
	if(tempTask->state == TASK_READY)
		RemoveFromTaskRdyList(tempTask);
	// Remove from Delay Queue
	else if(tempTask->state == TASK_WAITING)
		RemoveFromTaskDlyList(tempTask);
	// Task wait an event.
	if(tempTask->eventType != INVALID_TYPE)
	{
		switch( tempTask->eventType )   
		{  
			case EVENT_TYPE_SEM:  
				UrRemoveTaskFromSemWaiting(tempTask);
				break;
			case EVENT_TYPE_MUTEX:  
				UrRemoveTaskFromMutexWaiting(tempTask);
				break;
			case EVENT_TYPE_MAILBOX:  
				UrRemoveTaskFromMailboxWaiting(tempTask);
				break;
			case EVENT_TYPE_FLAG:  
				UrRemoveTaskFromFlagWaiting(tempTask);
				break;
			default :  
				break; 
		}  
		
	}
	
	tempTask->stkPtr 			= Ur_NULL;                		/*!< The current point of task.       */
	tempTask->prio 				= INVALID_Prio;                 /*!< Task priority.                   */
	tempTask->state 			= TASK_DORMANT;        			/*!< TaSk status.                     */
	tempTask->taskID			= INVALID_ID;                 	/*!< Task ID.                         */
	tempTask->eventID			= INVALID_ID;
	tempTask->eventType			= INVALID_TYPE;
	tempTask->stackStart 		= Ur_NULL;                 		/*!< The top point of task.           */
	tempTask->stackEnd			= Ur_NULL;
	tempTask->taskFuc			= Ur_NULL;
	tempTask->delayTick			= INVALID_TICK;              	/*!< The number of ticks which delay. */
	tempTask->wakeTick			= INVALID_TICK;
	tempTask->timeSlice			= CFG_TIME_SLICE;				/*!< When Round Robin , How many tick to Context Switch.	*/
	tempTask->Tasknext			= Ur_NULL;               		/*!< The pointer to next Task.         */
	tempTask->Taskprev			= Ur_NULL;               		/*!< The pointer to prev Task.         */
	tempTask->nextWaiting		= Ur_NULL;
	
UrTaskUnlock();
UrSchedUnlock();
	return E_OK;
}

/*
UrInsertToTaskRdyList()
------------------------------------------------------------
1. Ready Queue is NULL.
2. tcbInsert->prio < the prio of head of Ready Queue.
3. The Queue where tcbInsert->prio is not Null.
4. The Queue where tcbInsert->prio is Null. ( Insert to Mid or End )
	- Insert to the End of whole Ready Queue
	- Insert to the Mid of whole Ready Queue
*/
void  UrInsertToTaskRdyList(P_TASK tcbInsert)
{
	if(tcbInsert == Ur_NULL)
		return;
	tcbInsert->state 	= TASK_READY;
	P_TASK tempHead 	= getTaskRdy();
	P_TASK tempEnd;
	// 1. Ready Queue is NULL.
	if(tempHead == Ur_NULL)
	{
		/*print(1);
		println("");*/
		setTaskRdy((U32)tcbInsert);
		setPrioHead(tcbInsert->prio,(U32)tcbInsert);
		setPrioEnd(tcbInsert->prio,(U32)tcbInsert);
		return;
	}
	
	U8 tempHeadPrio = tempHead->prio;
	U8 tempEndPrio 	= tempHead->prio;
	U8 insertPrio 	= tcbInsert->prio;
	// 2. tcbInsert->prio < the prio of head of Ready Queue.
	if( insertPrio < tempHeadPrio )
	{
		/*print(2);
		println("");*/
		tcbInsert->Tasknext = tempHead;
		tempHead->Taskprev 	= tcbInsert;
		
		setTaskRdy((U32)tcbInsert);
		setPrioHead(tcbInsert->prio,(U32)tcbInsert);
		setPrioEnd(tcbInsert->prio,(U32)tcbInsert);
		
		return;
	}
	// 3. The Queue where tcbInsert->prio is not Null.
	if( getPrioHead(insertPrio) != Ur_NULL )
	{
		/*print(3);
		println("");*/
		// Get the end of now queue
		tempEnd 		= getPrioEnd(insertPrio);
		// If tempEnd is not the end of whole Ready Queue. ( Should do before Insert the tcbInsert to the End of now queue. ) 
		if(tempEnd->Tasknext != Ur_NULL)
		{
			tempEnd->Tasknext->Taskprev = tcbInsert;
			tcbInsert->Tasknext 		= tempEnd->Tasknext;
		}
		// Insert to the End of now queue.
		tempEnd->Tasknext 	= tcbInsert;
		tcbInsert->Taskprev = tempEnd;
		
		setPrioEnd(tcbInsert->prio,(U32)tcbInsert);
		
		return;
	}
	// 4. The Queue where tcbInsert->prio is Null.
	P_TASK prevtempEnd;
	tempEnd = getPrioEnd(tempHeadPrio);
	tempEndPrio = tempEnd->prio;
	while(tempEndPrio < insertPrio)
	{
		if(tempEnd->Tasknext==Ur_NULL)
			break;
		prevtempEnd = tempEnd;
		tempEndPrio = tempEnd->Tasknext->prio;
		tempEnd 	= getPrioEnd(tempEndPrio);
	}
	
	// Insert to the End of whole Ready Queue
	if(tempEndPrio < insertPrio)
	{
		/*print(41);
		println("");*/
		tempEnd->Tasknext 	= tcbInsert;
		tcbInsert->Taskprev = tempEnd;
		
		setPrioHead(tcbInsert->prio,(U32)tcbInsert);
		setPrioEnd(tcbInsert->prio,(U32)tcbInsert);
		
		return;
	}
	/*print(42);
	println("");*/
	// prevtempEnd
	prevtempEnd->Tasknext->Taskprev	= tcbInsert;
	tcbInsert->Tasknext 			= prevtempEnd->Tasknext;
	prevtempEnd->Tasknext 			= tcbInsert;
	tcbInsert->Taskprev 			= prevtempEnd;
	
	setPrioHead(tcbInsert->prio,(U32)tcbInsert);
	setPrioEnd(tcbInsert->prio,(U32)tcbInsert);
}

/*
InsertToTaskDlyList()
------------------------------------------------------------
1. Delay Queue is NULL.
2. Insert to Head of Delay Queue.
3. Insert to End of Delay Queue.
4. Insert to Mid of Delay Queue.
*/

void  InsertToTaskDlyList  (U32 timeout)
{
	if(timeout == 0)
		return;
	
	TaskRunning->state 		= TASK_WAITING;
	TaskRunning->delayTick 	= timeout;
	TaskRunning->wakeTick	= UrGetOSTickCnt() + timeout;
	P_TASK tempTask = getTaskDly();
	P_TASK prevTempTask;
	
	// 1. Delay Queue is NULL.
	if( tempTask == Ur_NULL)
	{
		setTaskDly( (U32)TaskRunning );
		return;
	}
	
	// 2. Insert to Head of Delay Queue.
	if(TaskRunning->wakeTick < tempTask->wakeTick)
	{
		TaskRunning->Tasknext 	= tempTask;
		tempTask->Taskprev  	= TaskRunning;
		setTaskDly( (U32)TaskRunning );
		return;
	}
	while(tempTask != Ur_NULL)
	{
		if(TaskRunning->wakeTick < tempTask->wakeTick)
			break;
		prevTempTask = tempTask;
		tempTask = tempTask->Tasknext;
	}
	// 3. Insert to End of Delay Queue.
	prevTempTask->Tasknext 	= TaskRunning;
	TaskRunning->Taskprev  	= prevTempTask;
	// 4. Insert to Mid of Delay Queue.
	if(tempTask != Ur_NULL)
	{
		TaskRunning->Tasknext 	= tempTask;
		tempTask->Taskprev  	= TaskRunning;
	}
}

/*
RemoveFromTaskRdyList()
------------------------------------------------------------
1. Final Task in RdyList.
2. Remove Task is Head of RdyList.
3. Remove Task is End of RdyList.
4. Remove Task between Head and End.
*/
void  RemoveFromTaskRdyList(P_TASK ptcb)
{
	if(ptcb == Ur_NULL)
		return;
	ptcb->state = TASK_WAITING;
	
	if(ptcb->Taskprev == Ur_NULL && ptcb->Tasknext == Ur_NULL)		// Final Task in RdyList.
	{
		setTaskRdy((U32)Ur_NULL);
		setPrioHead(ptcb->prio,(U32)Ur_NULL);
		setPrioEnd(ptcb->prio,(U32)Ur_NULL);
	}
	else if(ptcb->Taskprev == Ur_NULL)								// Remove head of RdyList.
	{
		setTaskRdy((U32)ptcb->Tasknext);
		setPrioHead(ptcb->prio,(U32)ptcb->Tasknext);
		ptcb->Tasknext->Taskprev = Ur_NULL;
	}
	else if(ptcb->Tasknext == Ur_NULL)								// Remove end of RdyList.
	{
		setPrioEnd(ptcb->prio,(U32)ptcb->Taskprev);
		ptcb->Taskprev->Tasknext = Ur_NULL;
	}
	else															// Remove between head and end.
	{
		ptcb->Tasknext->Taskprev = ptcb->Taskprev;
		ptcb->Taskprev->Tasknext = ptcb->Tasknext;
	}
	ptcb->Tasknext	= Ur_NULL;
	ptcb->Taskprev	= Ur_NULL;
}

/*
RemoveFromTaskDlyList()
------------------------------------------------------------
1. Final Task in DlyList.
2. Remove Task is Head of DlyList.
3. Remove Task is End of DlyList.
4. Remove Task between Head and End.
*/
void  RemoveFromTaskDlyList(P_TASK ptcb)
{
	if(ptcb == Ur_NULL)
		return;
	ptcb->state = TASK_READY;
	if(ptcb->Taskprev == Ur_NULL && ptcb->Tasknext == Ur_NULL)		// Final Task in DlyList.
	{
		setTaskDly((U32)Ur_NULL);
	}
	else if(ptcb->Taskprev == Ur_NULL)								// Remove head of DlyList.
	{
		setTaskDly((U32)ptcb->Tasknext);
		ptcb->Tasknext->Taskprev = Ur_NULL;
	}
	else if(ptcb->Tasknext == Ur_NULL)								// Remove end of DlyList.
	{
		ptcb->Taskprev->Tasknext = Ur_NULL;
	}
	else															// Remove between head and end.
	{
		ptcb->Tasknext->Taskprev = ptcb->Taskprev;
		ptcb->Taskprev->Tasknext = ptcb->Tasknext;
	}
	ptcb->Tasknext	= Ur_NULL;
	ptcb->Taskprev	= Ur_NULL;
	ptcb->delayTick	= 0;			// Initial Delay Tick
	ptcb->wakeTick 	= 0;			// Initial Wake Tick
}


/*
EventTaskToWait()
------------------------------------------------------------
1. Waiting List is NULL.
2. 
*/
P_TASK  EventTaskToWait(OS_ID eventID,OS_ID eventType,U8 eventSortType,P_TASK eventTaskList)
{
	if(eventType != EVENT_TYPE_FLAG)
	{
		TaskRunning->eventID 		= eventID;
	}
	TaskRunning->eventType		= eventType;
	TaskRunning->state			= TASK_WAITING;
	if(eventTaskList == Ur_NULL)
	{
		return TaskRunning;
	}
	P_TASK tempTask = eventTaskList;
	// Sort by FIFO
	if( eventSortType == EVENT_SORT_TYPE_FIFO )
	{
		while(tempTask->nextWaiting)
		{
			tempTask = tempTask->nextWaiting;
		}
		tempTask->nextWaiting = TaskRunning;
		
		return eventTaskList;
	}
	// Sort by Priority
	if( eventSortType == EVENT_SORT_TYPE_PRIO )
	{
		if( TaskRunning->prio < eventTaskList->prio )
		{
			TaskRunning->nextWaiting = eventTaskList;
			return TaskRunning;
		}
		while(tempTask->nextWaiting)
		{
			if( TaskRunning->prio < tempTask->nextWaiting->prio )
				break;
		}
		if(tempTask->nextWaiting != Ur_NULL)
		{
			TaskRunning->nextWaiting = tempTask->nextWaiting;
		}
		tempTask->nextWaiting = TaskRunning;
		return eventTaskList;
	}
	
}

P_TASK  EventTaskAwake(P_TASK eventTaskList)
{
	P_TASK newEventTaskList 	= eventTaskList->nextWaiting;
	eventTaskList->eventID 		= INVALID_ID;
	eventTaskList->eventType 	= INVALID_TYPE;
	eventTaskList->state		= TASK_READY;
	eventTaskList->nextWaiting 	= Ur_NULL;
	return newEventTaskList;
}

void UrTaskTimeDispose(void)     /*!< Time dispose function.               */
{
	P_TASK tempTask = getTaskDly();
	P_TASK nextTask;
	while(tempTask != Ur_NULL)
	{
		// Waiting Forever
		if(tempTask->wakeTick == INVALID_TICK)
			break;
		
		if(UrGetOSTickCnt() < tempTask->wakeTick)
			break;
		
		// Delay Task Finish , Resume Task.
		nextTask = tempTask->Tasknext;				// Keep Next Task.
		
		RemoveFromTaskDlyList(tempTask);
		UrInsertToTaskRdyList(tempTask);
		
		tempTask = nextTask;
	}
}

void UrDelayTask(U32 delayTicks)
{
	//println("Delay Task Start ");
UrSchedLock();
UrTaskLock();
	InsertToTaskDlyList(delayTicks);
	// Set Flag to Pending in UrSchedUnlock
	TaskSchedReq = Ur_TRUE;
UrTaskUnlock();
UrSchedUnlock();
	//println("Delay Task End ");
}

void printRdyList()
{
UrSchedLock();
UrTaskLock();
	println(">> printRdyList");
	P_TASK RdyList;
	RdyList = getTaskRdy();
	while(RdyList != Ur_NULL)
	{
		printh((U32)RdyList);
		RdyList = RdyList->Tasknext;
		for(int i=0;i<1000;i++){}
	}
	println("");
UrTaskUnlock();
UrSchedUnlock();
}

void printDlyList()
{
UrSchedLock();
UrTaskLock();
	println(">> printDlyList");
	P_TASK DlyList;
	DlyList = getTaskDly();
	while(DlyList != Ur_NULL)
	{
		printh((U32)DlyList);
		DlyList = DlyList->Tasknext;
		for(int i=0;i<1000;i++){}
	}
	println("");
UrTaskUnlock();
UrSchedUnlock();
}

void printWaitList(P_TASK WaitList)
{
UrSchedLock();
UrTaskLock();
	println(">> printWaitList");
	while(WaitList != Ur_NULL)
	{
		printh((U32)WaitList);
		WaitList = WaitList->nextWaiting;
		for(int i=0;i<1000;i++){}
	}
	println("");
UrTaskUnlock();
UrSchedUnlock();
}