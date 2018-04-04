/*
 * UrMutex.c
 *
 *  Created on: 2018年1月28日
 *      Author: dclab
 */

/*---------------------------- Inlcude --------------------------------------*/
#include "UrOSInclude.h"
#include "comm.h"
#include "portmacro.h"

#if CFG_MUTEX_EN > 0

P_MUTEX getMutexByID(OS_ID mutexID)
{
	if(mutexID >= CFG_MAX_MUTEX)
		return Ur_NULL;
	U32 *p = (U32 *)SHARED_BASE_MUTEX;
	P_MUTEX tempMutex;
	p = p + mutexID*SHARED_SIZE_MUTEX/4;
	tempMutex = (P_MUTEX)p;
	return tempMutex;
}

void UrInitMutex()
{
	P_MUTEX tempMutex;
	int i;
	for(i=0;i<CFG_MAX_MUTEX;i++)
	{
		tempMutex = getMutexByID(i);
		tempMutex->id 				= INVALID_ID;
		tempMutex->ownTaskID		= INVALID_ID;
		tempMutex->flag				= MUTEX_FREE;                 	/*!< Mutex flag.                      */
		tempMutex->eventTaskList	= Ur_NULL;              		/*!< waitting the Mutex.              */
		UrRelease(&tempMutex->lock);
	}
}

OS_ID UrCreateMutex()
{
	P_MUTEX tempMutex;
UrSchedLock();
	OS_ID i;
	for(i=0;i<CFG_MAX_SEM;i++)
	{
		tempMutex = getMutexByID(i);
UrAcquire(&tempMutex->lock);
		if(tempMutex->id == INVALID_ID)
		{
			tempMutex->id = i;
UrSchedUnlock();
UrRelease(&tempMutex->lock);
			return i;						/* Return mutex ID                    */
		}
UrRelease(&tempMutex->lock);
	}
UrSchedUnlock();
	return E_CREATE_FAIL;               	/* No free mutex control block        */
}


OS_ID UrDeleteMutex(OS_ID mutexID,U8 opt)
{
	if(mutexID >= CFG_MAX_MUTEX)
		return Ur_NULL;
	
UrSchedLock();
    P_MUTEX tempMutex = getMutexByID(mutexID);
UrAcquire(&tempMutex->lock);
	if(opt == OPT_DEL_NO_PEND && tempMutex->eventTaskList != Ur_NULL )
	{
UrRelease(&tempMutex->lock);
UrSchedUnlock();
		return E_TASK_WAITING;
	}
UrTaskLock();
	P_TASK tempTask = tempMutex->eventTaskList;
	while( tempTask != Ur_NULL)
	{
		UrInsertToTaskRdyList(tempTask);						// Add to Ready Queue
		tempTask = EventTaskAwake(tempTask);
	}
UrTaskUnlock();
	tempMutex->id 				= INVALID_ID;
	tempMutex->ownTaskID		= INVALID_ID;
	tempMutex->flag				= MUTEX_FREE;                 	/*!< Mutex flag.                      */
	tempMutex->eventTaskList	= Ur_NULL;              		/*!< waitting the Mutex.              */
UrRelease(&tempMutex->lock);
UrSchedUnlock();
}

/*
UrEnterMutexSection()
------------------------------------------------------------
No Change || TASK_RUNNING -> TASK_WAITING
1. Mutex is MUTEX_FREE
2. Mutex is MUTEX_OCCUPY
	- Add TaskRunning to Event Waitting List
*/
StatusType UrEnterMutexSection(OS_ID mutexID)
{
    if(OSSchedLock != 0)                /* Is OS lock?                        */
    {
        return E_OS_IN_LOCK;            /* Yes,error return                   */
    }

    if(mutexID >= CFG_MAX_MUTEX)          /* Invalid 'mutexID'                  */
    {
        return E_INVALID_ID;
    }

UrSchedLock();
	P_TASK tempTask;
    P_MUTEX tempMutex;
    tempMutex  = getMutexByID(mutexID);
UrAcquire(&tempMutex->lock);
    if(tempMutex->flag == MUTEX_FREE)       					/* If mutex is available        		*/
    {
		tempMutex->ownTaskID = TaskRunning->taskID;
        tempMutex->flag    		= MUTEX_OCCUPY;      			/* Occupy the mutex resource			*/
    }
    /* If the mutex resource had been occupied                                */
    else if(tempMutex->flag == MUTEX_OCCUPY)
    {
UrTaskLock();
		tempMutex->eventTaskList = EventTaskToWait(tempMutex->id,EVENT_TYPE_MUTEX,EVENT_SORT_TYPE_PRIO,tempMutex->eventTaskList);
		TaskSchedReq = Ur_TRUE;
UrTaskUnlock();
	}
UrRelease(&tempMutex->lock);
UrSchedUnlock();
    return E_OK;
}

/*
UrLeaveMutexSection()
------------------------------------------------------------
No Change || TASK_WAITING -> TASK_Ready
1. Event Waitting List is NULL.
2. Event Waitting List is Not NULL.
*/
StatusType UrLeaveMutexSection(OS_ID mutexID)
{
	if(mutexID >= CFG_MAX_MUTEX)          				/* Invalid 'mutexID'                  		*/
    {
        return E_INVALID_ID;
    }
	
	P_TASK tempTask;
    P_MUTEX tempMutex;
UrSchedLock();
    tempMutex  = getMutexByID(mutexID);
UrAcquire(&tempMutex->lock);

	if(tempMutex->ownTaskID != TaskRunning->taskID)		/* Not Exit Critical Section in same Task.	*/
	{
UrRelease(&tempMutex->lock);
UrSchedUnlock();
		return E_MUTEX_DIF_SECTION;
	}
	
    if(tempMutex->eventTaskList == Ur_NULL)
	{
		tempMutex->flag 		= MUTEX_FREE;
		tempMutex->ownTaskID 	= INVALID_ID;
	}
	else
	{
UrTaskLock();
		tempMutex->ownTaskID = tempMutex->eventTaskList->taskID;
		UrInsertToTaskRdyList(tempMutex->eventTaskList);							// Add to Ready Queue
		tempMutex->eventTaskList  = EventTaskAwake(tempMutex->eventTaskList);		// Set New EventTaskList
UrTaskUnlock();
	}
UrRelease(&tempMutex->lock);
UrSchedUnlock();
    return E_OK;			
}

void UrRemoveTaskFromMutexWaiting(P_TASK removeTask)
{
	P_MUTEX tempMutex = getMutexByID(removeTask->eventID);
UrAcquire(&tempMutex->lock);
UrTaskLock();
	P_TASK tempWaitTask = tempMutex->eventTaskList;
	// If first Waiting Task is Delete Task, change the head.
	if( tempWaitTask== removeTask)
	{
		tempMutex->eventTaskList = tempWaitTask->nextWaiting;
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
UrRelease(&tempMutex->lock);
}


#endif

