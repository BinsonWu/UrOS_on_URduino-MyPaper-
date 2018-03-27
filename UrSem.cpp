/*
 * UrSem.c
 *
 *  Created on: 2018年1月28日
 *      Author: dclab
 */

#include "UrOSInclude.h"

#if CFG_SEM_EN > 0

void UrSemLock()
{
	U32 *p = (U32 *)SHARED_BASE_LOCK;
	while(*p & (1 << LOCK_SHIFT_SEM) ){}
	*p = *p | (1 << LOCK_SHIFT_SEM);
}

void UrSemUnlock()
{
	U32 *p = (U32 *)SHARED_BASE_LOCK;
	if( *p & (1 << LOCK_SHIFT_SEM)  )
		*p = *p & (~(1 << LOCK_SHIFT_SEM));
}

P_SEM getSemByID(OS_ID id)
{
	U32 *p = (U32 *)SHARED_BASE_SEM;
	P_SEM tempSem;
	p = p + id*SHARED_SIZE_SEM/4;
	tempSem = (P_SEM)p;
	return tempSem;
}

void UrInitSem()
{
	// Set Get ID Table
	//getIDTable[EVENT_TYPE_SEM] = (FUNCPtr)getSemByID;

	P_SEM tempSem;
	OS_ID i;
	for(i=0;i<CFG_MAX_SEM;i++)
	{
		tempSem = getSemByID(i);
		tempSem->	id 					= INVALID_ID;         /*!< Sem id                           */
		tempSem->eventSortType 			= 0;            				/*!< 0:FIFO 1: Preemptive by prio     */
		tempSem->eventCounter 			= 0;               			/*!< Counter of semaphore.            */
		tempSem->eventTaskList 			= Ur_NULL;          	/*!< Task waitting list.              */
	}
}

OS_ID UrCreateSem()
{
	P_SEM tempSem;
	OS_ID i;
UrSchedLock();
UrSemLock();
	for(i=0;i<CFG_MAX_SEM;i++)
	{
		tempSem = getSemByID(i);
		if(tempSem->id == INVALID_ID)
		{
			tempSem->id = i;
		UrSemUnlock();
		UrSchedUnlock();
			return i;                      /* Return mutex ID                    */
		}
	}
UrSemUnlock();
UrSchedUnlock();
	return E_CREATE_FAIL;               /* No free mutex control block        */
}

StatusType UrDelSem(OS_ID id)
{
    if(id >= CFG_MAX_SEM)
    {
        return E_INVALID_ID;
    }
UrSchedLock();
UrSemLock();
    P_SEM tempSem = getSemByID(id);
    tempSem->id 								= INVALID_ID;                         /*!< Sem id                           */
	tempSem->eventSortType 		= 0;            				/*!< 0:FIFO 1: Preemptive by prio     */
	tempSem->eventCounter 			= 0;               			/*!< Counter of semaphore.            */
	tempSem->eventTaskList 			= Ur_NULL;          	/*!< Task waitting list.              */
UrSemUnlock();
UrSchedUnlock();
	return E_OK;
}

StatusType UrPostSem(OS_ID id)
{
	if(id >= CFG_MAX_SEM)
	{
		return E_INVALID_ID;
	}
UrSchedLock();
UrSemLock();
UrTaskLock();
	P_SEM tempSem = getSemByID(id);
	if(tempSem->eventTaskList == Ur_NULL)
	{
		tempSem->eventCounter++;
	}
	else
	{
    	// Task State : Waiting -> Ready , So only need insert to Delay List.
		RemoveFromTaskDlyList(tempSem->eventTaskList);										// Waiting
		UrInsertToTaskRdyList(tempSem->eventTaskList);											// Waiting -> Ready
		tempSem->eventTaskList = EventTaskAwake(tempSem->eventTaskList);		// Set New EventTaskList
	}
UrTaskUnlock();
UrSemUnlock();
UrSchedUnlock();
	return E_OK;
}

StatusType UrPendSem(OS_ID id,U32 timeout)
{
    if(id >= CFG_MAX_SEM)
    {
        return E_INVALID_ID;
    }

UrSchedLock();
UrSemLock();
    P_SEM tempSem = getSemByID(id);
    if(tempSem->eventCounter > 0) /* If semaphore is positive,resource available */
    {
    	tempSem->eventCounter--;         /* Decrement semaphore only if positive */
    }
    else                                /* Resource is not available          */
    {            /* Block task until event or timeout occurs                       */
UrTaskLock();
		// Task State : Running -> Waiting , So only need insert to Delay List.
    	InsertToTaskDlyList(timeout);						// Running -> Waiting
    	// Set New EventTaskList
    	tempSem->eventTaskList = EventTaskToWait(tempSem->id,EVENT_TYPE_SEM,tempSem->eventTaskList);
    	// Because of Pending , we need to schedule until it getting a post.
    	TaskSchedReq = Ur_TRUE;
UrTaskUnlock();
    }
UrSemUnlock();
UrSchedUnlock();
    return E_OK;
}

#endif
