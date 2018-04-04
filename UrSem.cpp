/*
 * UrSem.c
 *
 *  Created on: 2018年1月28日
 *      Author: dclab
 */

#include "UrOSInclude.h"
#include "comm.h"
#include "portmacro.h"

#if CFG_SEM_EN > 0

P_SEM getSemByID(OS_ID semID)
{
	if(semID >= CFG_MAX_SEM)
		return Ur_NULL;
	U32 *p = (U32 *)SHARED_BASE_SEM;
	P_SEM tempSem;
	p = p + semID*SHARED_SIZE_SEM/4;
	tempSem = (P_SEM)p;
	return tempSem;
}

void UrInitSem()
{
	P_SEM tempSem;
	OS_ID i;
	for(i=0;i<CFG_MAX_SEM;i++)
	{
		tempSem = getSemByID(i);
		tempSem->id 					= INVALID_ID;        	/*!< Sem id                           */
		tempSem->eventSortType 			= EVENT_SORT_TYPE_FIFO; /*!< 0:FIFO 1: Preemptive by prio     */
		tempSem->eventCounter 			= 0;               		/*!< Counter of semaphore.            */
		tempSem->eventTaskList 			= Ur_NULL;          	/*!< Task waitting list.              */
		UrRelease(&tempSem->lock);
	}
}

OS_ID UrCreateSem(U32 initCount,U8 sortType)
{
	P_SEM tempSem;
	OS_ID i;
UrSchedLock();
	for(i=0;i<CFG_MAX_SEM;i++)
	{
		tempSem = getSemByID(i);
UrAcquire(&tempSem->lock);
		if(tempSem->id == INVALID_ID)
		{
			tempSem->id 			= i;
			tempSem->eventSortType	= sortType;
			tempSem->eventCounter	= initCount;
UrRelease(&tempSem->lock);
UrSchedUnlock();
			return i;                      /* Return mutex ID                    */
		}
UrRelease(&tempSem->lock);
	}
UrSchedUnlock();
	return E_CREATE_FAIL;               /* No free mutex control block        */
}

StatusType UrDelSem(OS_ID semID,U8 opt)
{
    if(semID >= CFG_MAX_SEM)				/* Is Sem ID actualy?                   */
    {
        return E_INVALID_ID;			/* No,error return.						*/
    }
	
UrSchedLock();
    P_SEM tempSem = getSemByID(semID);
UrAcquire(&tempSem->lock);

	if(opt == OPT_DEL_NO_PEND && tempSem->eventTaskList != Ur_NULL )
	{
UrRelease(&tempSem->lock);
UrSchedUnlock();
		return E_TASK_WAITING;
	}
	
    tempSem->id 					= INVALID_ID;			/*!< Sem id                           */
	tempSem->eventSortType 			= 0;            		/*!< 0:FIFO 1: Preemptive by prio     */
	tempSem->eventCounter 			= 0;               		/*!< Counter of semaphore.            */
UrTaskLock();
	P_TASK tempTask = tempSem->eventTaskList;
	while( tempTask != Ur_NULL)
	{
		// Task State : Waiting -> Ready , So only need insert to Delay List.
		if(tempTask->delayTick != OPT_WAIT_FOREVER)
		{
			RemoveFromTaskDlyList(tempTask);						// Waiting
		}
		UrInsertToTaskRdyList(tempTask);						// Waiting -> Ready
		tempTask = EventTaskAwake(tempTask);
	}
UrTaskUnlock();
	tempSem->eventTaskList 			= Ur_NULL;          	/*!< Task waitting list.              */
UrRelease(&tempSem->lock);
UrSchedUnlock();
	return E_OK;
}

/*
UrPostSem()
------------------------------------------------------------
No Change || TASK_WAITING -> TASK_Ready
1. Event Waitting List of Sem is NULL.
2. Event Waitting List of Sem is Not NULL.
*/
StatusType UrPostSem(OS_ID semID)
{
	if(OSSchedLock != 0)                /* Is OS lock?                        */
    {
        return E_OS_IN_LOCK;            /* Yes,error return                   */
    }
	
	if(semID >= CFG_MAX_SEM)				/* Is Sem ID actualy?                   */
    {
        return E_INVALID_ID;				/* No,error return.						*/
    }
UrSchedLock();
UrTaskLock();
	P_SEM tempSem = getSemByID(semID);
UrAcquire(&tempSem->lock);
	if(tempSem->eventTaskList == Ur_NULL)
	{
		tempSem->eventCounter++;
	}
	else
	{
    	// Task State : Waiting -> Ready
		if(tempSem->eventTaskList->delayTick != OPT_WAIT_FOREVER)
		{
			RemoveFromTaskDlyList(tempSem->eventTaskList);						// Waiting -> Ready
		}
		UrInsertToTaskRdyList(tempSem->eventTaskList);							// Add to Ready Queue
		
		tempSem->eventTaskList  = EventTaskAwake(tempSem->eventTaskList);		// Set New EventTaskList
	}
UrRelease(&tempSem->lock);
UrTaskUnlock();
UrSchedUnlock();
	return E_OK;
}

/*
UrPendSem()
------------------------------------------------------------
No Change || TASK_RUNNING -> TASK_WAITING
1. eventCounter of Sem is > 0
2. eventCounter of Sem is = 0
	- Timeout = 0, Don't Wait.
	- Timeout != -1, Add to Delay Queue.
*/
StatusType UrPendSem(OS_ID semID,int timeout)
{
	if(OSSchedLock != 0)                /* Is OS lock?                        	*/
    {
        return E_OS_IN_LOCK;            /* Yes,error return.                   	*/
    }
    if(semID >= CFG_MAX_SEM)			/* Is Sem ID actualy?                   */
    {
        return E_INVALID_ID;			/* No,error return.						*/
    }
UrSchedLock();
    P_SEM tempSem = getSemByID(semID);
UrAcquire(&tempSem->lock);
    if(tempSem->eventCounter > 0) 		/* If semaphore is positive,resource available 	*/
    {
    	tempSem->eventCounter--;		/* Decrement semaphore only if positive 		*/
		return E_OK;
    }
    else                                /* Resource is not available          			*/
    {            						/* Block task until event or timeout occurs 	*/
		if(timeout == 0)
		{
UrRelease(&tempSem->lock);
UrSchedUnlock();
			return E_SEM_EMPTY;
		}
UrTaskLock();
		if(timeout != OPT_WAIT_FOREVER)
		{
			// Task State : Running -> Waiting , So only need insert to Delay List.
			InsertToTaskDlyList(timeout);	// Running -> Waiting
		}
    	// Set New EventTaskList
    	tempSem->eventTaskList = EventTaskToWait(tempSem->id,EVENT_TYPE_SEM,tempSem->eventSortType,tempSem->eventTaskList);
    	// Because of Pending , we need to schedule until it getting a post.
    	TaskSchedReq = Ur_TRUE;
UrTaskUnlock();
    }
UrRelease(&tempSem->lock);
UrSchedUnlock();
    return E_OK;
}

void UrRemoveTaskFromSemWaiting(P_TASK removeTask)
{
	P_SEM tempSem = getSemByID(removeTask->eventID);
UrAcquire(&tempSem->lock);
UrTaskLock();
	P_TASK tempWaitTask = tempSem->eventTaskList;
	// If first Waiting Task is Delete Task, change the head.
	if( tempWaitTask== removeTask)
	{
		tempSem->eventTaskList = tempWaitTask->nextWaiting;
	}
	// Change the link of nextWaiting.
	else
	{
		while(tempWaitTask->nextWaiting != Ur_NULL && tempWaitTask->nextWaiting!=removeTask)
		{
			tempWaitTask = tempWaitTask->nextWaiting;
		}
		if(tempWaitTask->nextWaiting!= Ur_NULL)
			tempWaitTask->nextWaiting = tempWaitTask->nextWaiting->nextWaiting;
	}
UrTaskUnlock();
UrRelease(&tempSem->lock);
}

#endif
