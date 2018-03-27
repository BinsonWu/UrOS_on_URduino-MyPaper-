/*
 * UrMutex.c
 *
 *  Created on: 2018年1月28日
 *      Author: dclab
 */

/*---------------------------- Inlcude --------------------------------------*/
#include "UrOSInclude.h"

#if CFG_MUTEX_EN > 0

P_MUTEX getMutexByID(OS_ID id)
{
	U32 *p = (U32 *)SHARED_BASE_MUTEX;
	P_MUTEX tempMutex;
	p = p + id*SHARED_SIZE_MUTEX;
	tempMutex = (P_MUTEX)p;
	return tempMutex;
}

void UrInitMutex()
{
	// Set Get ID Table
	//getIDTable[EVENT_TYPE_MUTEX] = (FUNCPtr)getMutexByID;

	U32 *p = (U32 *)SHARED_BASE_MUTEX;
	P_MUTEX tempMutex;
	int i;
	for(i=0;i<CFG_MAX_MUTEX;i++)
	{
		p = p + i*SHARED_SIZE_MUTEX;
		tempMutex = (P_MUTEX)p;
		tempMutex->exist					= Ur_FALSE;
		tempMutex->originalPrio		= INVALID_Prio;              	/*!< Mutex priority.                  */
		tempMutex->mutexFlag			= MUTEX_FREE;                 	/*!< Mutex flag.                      */
		tempMutex-> taskID				= INVALID_ID;                    	/*!< Task ID.                         */
		tempMutex->hipriTaskID		= INVALID_ID;               						/*!< Mutex ID.                        */
		tempMutex->waittingList		= Ur_NULL;              			/*!< waitting the Mutex.              */
		tempMutex->mutexLock		= Ur_FALSE;
	}
}

OS_ID UrCreateMutex()
{
	P_MUTEX tempMutex;
	U32 *p = (U32 *)SHARED_BASE_MUTEX;
	UrSchedLock();
	OS_ID i;
	for(i=0;i<CFG_MAX_SEM;i++)
	{
		p = p + i*SHARED_SIZE_MUTEX;
		tempMutex = (P_MUTEX)p;
		if(tempMutex->exist == Ur_FALSE)
		{
			OsSchedUnlock();
			return i;                      /* Return mutex ID                    */
		}
	}
	OsSchedUnlock();
	return E_CREATE_FAIL;               /* No free mutex control block        */
}

StatusType UrEnterMutexSection(OS_ID mutexID)
{
    P_TASK ptcb,pCurTcb;
    P_MUTEX pMutex;

    if(OSSchedLock != 0)                /* Is OS lock?                        */
    {
        return E_OS_IN_LOCK;            /* Yes,error return                   */
    }

    if(mutexID >= CFG_MAX_MUTEX)          /* Invalid 'mutexID'                  */
    {
        return E_INVALID_ID;
    }

    UrSchedLock();
    //pCurTcb = TaskRunning;
    pMutex  = getMutexByID(mutexID);

    pCurTcb->mutexID = mutexID;
    if(pMutex->mutexFlag == MUTEX_FREE)       /* If mutex is available        */
    {
        pMutex->originalPrio = pCurTcb->prio; /* Save priority of owning task */
        pMutex->taskID       = pCurTcb->taskID;   /* Acquire the resource     */
        pMutex->hipriTaskID  = pCurTcb->taskID;
        pMutex->mutexFlag    = MUTEX_OCCUPY;      /* Occupy the mutex resource*/
    }
    /* If the mutex resource had been occupied                                */
    else if(pMutex->mutexFlag == MUTEX_OCCUPY)
    {
		ptcb = getTaskByID(pMutex->taskID);
        if(ptcb->prio > pCurTcb->prio)  /* Need to promote priority of owner? */
        {
#if CFG_ORDER_LIST_SCHEDULE_EN ==0
			//DeleteTaskPri(ptcb->prio);
			//ActiveTaskPri(pCurTcb->prio);
#endif
            ptcb->prio = pCurTcb->prio;	    /* Promote prio of owner          */

            /* Upgarde the highest priority about the mutex                   */
            pMutex->hipriTaskID	= pCurTcb->taskID;
            if(ptcb->state == TASK_READY)   /* If the task is ready to run    */
            {
                //RemoveFromTaskRdyList(ptcb); /* Remove the task from READY list*/
                //InsertToTaskRdyList(ptcb);   /* Insert the task into READY list*/
            }

			pCurTcb->state   = TASK_WAITING;    /* Block current task             */
			TaskSchedReq     = Ur_TRUE;
			pCurTcb->Tasknext = Ur_NULL;
			pCurTcb->Taskprev = Ur_NULL;

			ptcb = pMutex->waittingList;
			if(ptcb == Ur_NULL)               /* If the event waiting list is empty  */
			{
				pMutex->waittingList = pCurTcb; /* Insert the task to head        */
			}
			else                        /* If the event waiting list is not empty */
			{
				while(ptcb->Tasknext != Ur_NULL)    /* Insert the task to tail        */
				{
					ptcb = ptcb->Tasknext;
				}
				ptcb->Tasknext    = pCurTcb;
				pCurTcb->Taskprev = ptcb;
				pCurTcb->Tasknext = Ur_NULL;
			}
		}
		OsSchedUnlock();
		return E_OK;
		}
    return E_OK;
}

void   RemoveMutexList(P_TASK ptcb)
{

}
#endif
