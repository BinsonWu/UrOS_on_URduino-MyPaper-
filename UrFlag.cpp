/*
 * UrFlag.c
 *
 *  Created on: 2018年1月28日
 *      Author: dclab
 */

#include "UrOSInclude.h"
#include "comm.h"
#include "portmacro.h"


#if CFG_FLAG_EN >0

/*
					< UrFlag Architecture >
---------------------------------------------------------------
- FlagActive	(32bit)
- FlagReady		(32bit)
- FlagWaitList
	|    | -> |    | -> ... -> |    | -> |    | 
	
EX : 
	U32 GPIO_FlagID, PWM_FlagID, USART_FlagID;
	GPIO_FlagID 	= UrCreateFlag();
	PWM_FlagID 		= UrCreateFlag();
	USART_FlagID 	= UrCreateFlag();
	U32 flags = (1<<GPIO_FlagID)|(1<<PWM_FlagID)|(1<<USART_FlagID);
	UrPendMutipleFlag(flags,OPT_WAIT_FOREVER,OPT_WAIT_ALL);
*/

U32 UrGetFlagActive()
{
	U32 *p = SHARED_BASE_FLAG_ACTIVE;
	return *p;
}

void UrSetFlagActive(U32 newActive)
{
	U32 *p = SHARED_BASE_FLAG_ACTIVE;
	*p = newActive;
}

U32 UrGetFlagReady()
{
	U32 *p = SHARED_BASE_FLAG_READY;
	return *p;
}

void UrSetFlagReady(U32 newReady)
{
	U32 *p = SHARED_BASE_FLAG_READY;
	*p = newReady;
}

P_FLAG_NODE UrGetFlagNodeList()
{
	U32 *p = SHARED_BASE_FLAG_NODELIST;
	P_FLAG_NODE tempFlag;
	tempFlag = (P_FLAG_NODE)*p;
	return tempFlag;
}

void UrSetFlagNodeList(U32 TaskAddr)
{
	U32 *p = SHARED_BASE_FLAG_NODELIST;
	*p = TaskAddr;
}

P_FLAG_NODE UrGetFalgByID(OS_ID flagID)
{
	if(flagID >= CFG_MAX_FLAG)
		return Ur_NULL;
	U32 *p = (U32 *)SHARED_BASE_FLAG;
	P_FLAG_NODE tempFlag;
	p = p + flagID*SHARED_SIZE_FLAG/4;
	tempFlag = (P_FLAG_NODE)p;
	return tempFlag;
}


void UrFlagLock()
{
	U32 *p = SHARED_BASE_LOCK;
	while(*p & (1 << LOCK_SHIFT_FLAG) ){}
	*p = *p | (1 << LOCK_SHIFT_FLAG);
}

void UrFlagUnlock()
{
	U32 *p = SHARED_BASE_LOCK;
	if( *p & (1 << LOCK_SHIFT_FLAG)  )
		*p = *p & (~(1 << LOCK_SHIFT_FLAG));
}


P_FLAG_NODE UrRemoveFlagNode(P_FLAG_NODE rmvNode)
{
	P_FLAG_NODE newNode = rmvNode->nextNode;
	if(rmvNode==UrGetFlagNodeList())
		UrSetFlagNodeList(newNode);
	rmvNode->waitFlags	= 0;
	rmvNode->waitType	= INVALID_TYPE;
	if(rmvNode->prevNode != Ur_NULL)
		rmvNode->prevNode->nextNode	= rmvNode->nextNode;
	if(rmvNode->nextNode != Ur_NULL)
		rmvNode->nextNode->prevNode	= rmvNode->prevNode;
	rmvNode->waitTask  = EventTaskAwake(rmvNode->waitTask);				// Set New rmvNode->waitTask
	rmvNode->prevNode	= Ur_NULL;
	rmvNode->nextNode	= Ur_NULL;
	return newNode;
}


/* ------------------------------------------ User Function ------------------------------------------ */

void UrInitFlag()
{
	UrSetFlagActive(0);
	UrSetFlagReady(0);
	UrSetFlagNodeList(Ur_NULL);
	int i;
	for(i=0;i<CFG_MAX_FLAG;i++)
	{
		P_FLAG_NODE tempFlag 		= UrGetFalgByID(i);
		tempFlag->id				= i;
		tempFlag->waitFlags			= 0;
		tempFlag->waitTask			= Ur_NULL;
		tempFlag->waitType			= INVALID_TYPE;
		tempFlag->nextNode			= Ur_NULL;
		tempFlag->prevNode			= Ur_NULL;
	}
}

OS_ID UrCreateFlag()
{
	OS_ID i;
UrSchedLock();
	for(i=0;i<CFG_MAX_FLAG;i++)
	{
		if( (U32)(UrGetFlagActive()&(1 << i)) != (U32)(1 << i) )
		{
			UrSetFlagActive( UrGetFlagActive()|(1 << i) );
UrSchedUnlock();
			return i;                      	/* Return Flag ID                    */
		}
	}
UrSchedUnlock();
	return E_CREATE_FAIL;               	/* No free Flag        */
}

/*
UrDelFlag()
------------------------------------------------------------
1. Delete No Task Pend on Waitting List.
	- Pend , return error.
2. Set Sem.
3. Remove Event Waitting List.
*/
StatusType UrDelFlag(OS_ID flagID,U8 opt)
{
    if(OSSchedLock != 0)                			/* Is OS lock?                        			*/
    {
        return E_OS_IN_LOCK;            			/* Yes,error return.                   			*/
    }
    if(flagID >= CFG_MAX_FLAG)						/* Is Flag ID actualy?                   		*/
    {
        return E_INVALID_ID;						/* No,error return.								*/
    }
UrFlagLock();
	if( ( (U32)(UrGetFlagActive()&(1<<flagID)) ) == 0)		/* Is Flag ID created?                  */
	{
UrFlagUnlock();
		return E_INVALID_ID;						/* No,error return.						*/
	}
	
UrSchedLock();
	P_FLAG_NODE tempFlag = UrGetFlagNodeList();
	while( tempFlag != Ur_NULL)
	{
		if( (U32)(tempFlag->waitFlags&(1<<flagID)) != 0)
		{
			if(opt == OPT_DEL_NO_PEND)
			{
UrFlagUnlock();
UrSchedUnlock();
				return E_TASK_WAITING;
			}
			if(tempFlag->waitType == OPT_WAIT_ONE)
			{
UrTaskLock();
				// Task State : Waiting -> Ready
				if(tempFlag->waitTask->delayTick != OPT_WAIT_FOREVER)
				{
					RemoveFromTaskDlyList(tempFlag->waitTask);						// Waiting -> Ready
				}
				// Insert to Ready Queue
				UrInsertToTaskRdyList(tempFlag->waitTask);
				// Remove from Flag Node List
				tempFlag = UrRemoveFlagNode(tempFlag);
UrTaskUnlock();
				continue;
			}
			else
			{
				// Update waitFlags
				tempFlag->waitFlags = tempFlag->waitFlags&(~(1<<flagID));
			}
			tempFlag = tempFlag->nextNode;
		}
UrFlagUnlock();
UrSchedUnlock();
	}
	return E_OK;
}

/*
UrSetFlag()
------------------------------------------------------------
1. Detect if Flag Trigger.
	- OPT_WAIT_ANY,OPT_WAIT_ONE
	- OPT_WAIT_ALL
2. If there is not anyone wake up,we set the flagIDth bit to 1.
*/
StatusType UrSetFlag(OS_ID flagID)
{
	if(OSSchedLock != 0)                /* Is OS lock?                        	*/
    {
        return E_OS_IN_LOCK;            /* Yes,error return.                   	*/
    }
	if(flagID >= CFG_MAX_FLAG)						/* Is Flag ID actualy?                  */
    {
        return E_INVALID_ID;						/* No,error return.						*/
    }
UrFlagLock();
	if( ( (U32)(UrGetFlagActive()&(1<<flagID)) ) == 0)		/* Is Flag ID created?                  */
	{
UrFlagUnlock();
		return E_INVALID_ID;						/* No,error return.						*/
	}
	
	if( (U32)( UrGetFlagReady()&(1<<flagID) ) == 1)		/* Already Set 							*/
	{
UrFlagUnlock();
		return E_OK;
	}
	UrSetFlagReady( UrGetFlagReady()|(1<<flagID) );
	
UrSchedLock();
	// 1. Detect if Flag Trigger.
UrTaskLock();
	P_FLAG_NODE tempFlag = UrGetFlagNodeList();
	while(tempFlag != Ur_NULL)
	{
		// OPT_WAIT_ANY,OPT_WAIT_ONE
		if(tempFlag->waitType == OPT_WAIT_ANY || tempFlag->waitType == OPT_WAIT_ONE)
		{
			if( (U32)(tempFlag->waitFlags&UrGetFlagReady()) > 0)
			{
				// Task State : Waiting -> Ready
				if(tempFlag->waitTask->delayTick != OPT_WAIT_FOREVER)
				{
					RemoveFromTaskDlyList(tempFlag->waitTask);						// Waiting -> Ready
				}
				// Isert to Ready Queue
				UrInsertToTaskRdyList(tempFlag->waitTask);
				// Remove from Flag Node List
				tempFlag = UrRemoveFlagNode(tempFlag);
				continue;
			}
		}
		// OPT_WAIT_ALL
		else
		{
			if( (U32)(tempFlag->waitFlags) == (U32)(tempFlag->waitFlags&UrGetFlagReady()) )
			{
				// Task State : Waiting -> Ready
				if(tempFlag->waitTask->delayTick != OPT_WAIT_FOREVER)
				{
					RemoveFromTaskDlyList(tempFlag->waitTask);						// Waiting -> Ready
				}
				// Isert to Ready Queue
				UrInsertToTaskRdyList(tempFlag->waitTask);
				// Remove from Flag Node List
				tempFlag = UrRemoveFlagNode(tempFlag);
				continue;
			}
		}
		tempFlag = tempFlag->nextNode;
	}
UrTaskUnlock();
UrFlagUnlock();
UrSchedUnlock();
}

/*
UrPendFlag()
------------------------------------------------------------
1. Check Flag if already Raady.
2. If not, create a flagnode and pend it by timeout.
	- OPT_WAIT_FOREVER.
	- No Wait.
	- Delay timeout ticks.
*/
StatusType UrPendSingleFlag(OS_ID flagID,int timeout)
{
	if(OSSchedLock != 0)                			/* Is OS lock?                        			*/
    {
        return E_OS_IN_LOCK;            			/* Yes,error return.                   			*/
    }
    if(flagID >= CFG_MAX_FLAG)						/* Is Flag ID actualy?                   		*/
    {
        return E_INVALID_ID;						/* No,error return.								*/
    }
UrFlagLock();
	if( ( (U32)(UrGetFlagActive()&(1<<flagID)) ) == 0)		/* Is Flag ID created?                  */
	{
UrFlagUnlock();
		return E_INVALID_ID;						/* No,error return.						*/
	}
UrSchedLock();
    // 1. Check Flag if already Raady.
	if( (U32)( UrGetFlagReady()&(1<<flagID) ) == 1)
	{
UrFlagUnlock();
UrSchedUnlock();
		return E_OK;
	}
	// Not Wait.
	if(timeout == 0)
	{
UrFlagUnlock();
UrSchedUnlock();
		return E_FLAG_NOT_READY;
	}
	
UrTaskLock();
	if(timeout != OPT_WAIT_FOREVER)
	{
		// Task State : Running -> Waiting , So only need insert to Delay List.
		InsertToTaskDlyList(timeout);	// Running -> Waiting
	}
	P_FLAG_NODE tempTaskFlag = UrGetFalgByID(TaskRunning->taskFlagID);
	// Set Task Flag Node
	tempTaskFlag->waitFlags 	= (1<<flagID);
	tempTaskFlag->waitTask 		= EventTaskToWait(INVALID_ID,EVENT_TYPE_FLAG,EVENT_SORT_TYPE_FIFO,tempTaskFlag->waitTask);
	tempTaskFlag->waitType		= OPT_WAIT_ONE;
	
	if(UrGetFlagNodeList() != Ur_NULL)
		UrGetFlagNodeList()->prevNode	= tempTaskFlag;
	tempTaskFlag->nextNode		= UrGetFlagNodeList();
	UrSetFlagNodeList(tempTaskFlag);
	// Because of Pending , we need to schedule until it getting a post.
	TaskSchedReq = Ur_TRUE;
UrTaskUnlock();
UrFlagUnlock();
UrSchedUnlock();

    return E_OK;
}

StatusType UrPendMutipleFlag(U32 flags,int timeout,U8 waitType)
{
	if(OSSchedLock != 0)                			/* Is OS lock?                        			*/
    {
        return E_OS_IN_LOCK;            			/* Yes,error return.                   			*/
    }
    if(flags >= CFG_MAX_FLAG)						/* Is Flag ID actualy?                   		*/
    {
        return E_INVALID_ID;						/* No,error return.								*/
    }
UrFlagLock();
	if( ( (U32)( UrGetFlagActive()&flags ) ) != flags )		/* Is Flag ID created?                  */
	{
UrFlagUnlock();
		return E_INVALID_ID;						/* No,error return.						*/
	}
UrSchedLock();
    // 1. Check Flag if already Ready.
	if((U32)( UrGetFlagReady()&(flags) ) == flags && waitType == OPT_WAIT_ALL)
	{
UrFlagUnlock();
UrSchedUnlock();
		return E_OK;
	}
	if((U32)( UrGetFlagReady()&(flags) ) > 0 && waitType == OPT_WAIT_ANY)
	{
UrFlagUnlock();
UrSchedUnlock();
		return E_OK;
	}
	// Not Wait.
	if(timeout == 0)
	{
UrFlagUnlock();
UrSchedUnlock();
		return E_FLAG_NOT_READY;
	}
	
UrTaskLock();
	if(timeout != OPT_WAIT_FOREVER)
	{
		// Task State : Running -> Waiting , So only need insert to Delay List.
		InsertToTaskDlyList(timeout);	// Running -> Waiting
	}
	P_FLAG_NODE tempTaskFlag = UrGetFalgByID(TaskRunning->taskFlagID);
	// Set Task Flag Node
	tempTaskFlag->waitFlags 	= flags;
	tempTaskFlag->waitTask 		= EventTaskToWait(INVALID_ID,EVENT_TYPE_FLAG,EVENT_SORT_TYPE_FIFO,tempTaskFlag->waitTask);
	tempTaskFlag->waitType		= waitType;
	if(UrGetFlagNodeList() != Ur_NULL)
		UrGetFlagNodeList()->prevNode	= tempTaskFlag;
	tempTaskFlag->nextNode		= UrGetFlagNodeList();
	UrSetFlagNodeList(tempTaskFlag);
	// Because of Pending , we need to schedule until it getting a post.
	TaskSchedReq = Ur_TRUE;
UrTaskUnlock();
UrFlagUnlock();
UrSchedUnlock();

    return E_OK;
}

StatusType UrClearFlag(OS_ID flagID)
{
	if(OSSchedLock != 0)                			/* Is OS lock?                        			*/
    {
        return E_OS_IN_LOCK;            			/* Yes,error return.                   			*/
    }
    if(flagID >= CFG_MAX_FLAG)						/* Is Flag ID actualy?                   		*/
    {
        return E_INVALID_ID;						/* No,error return.								*/
    }
UrFlagLock();
	if( ( (U32)(UrGetFlagActive()&(1<<flagID)) ) == 0)		/* Is Flag ID created?                  */
	{
UrFlagUnlock();
		return E_INVALID_ID;						/* No,error return.						*/
	}
	UrSetFlagReady( UrGetFlagReady()&( ~(1<<flagID) ) );
UrFlagUnlock();
	return E_OK;
}

void UrRemoveTaskFromFlagWaiting(P_TASK removeTask)
{
UrFlagLock();
	UrRemoveFlagNode( UrGetFalgByID(removeTask->taskFlagID) );
UrFlagUnlock();
}

void printFlagNodeList()
{
	println(">> printFlagNodeList()");
	P_FLAG_NODE tempFalg = UrGetFlagNodeList();
	while(tempFalg!=Ur_NULL)
	{
		print("id\t\t: ");
		print(tempFalg->id);
		println("");
		print("waitFlags\t: ");
		printh(tempFalg->waitFlags);
		print("waitTask\t: ");
		printh(tempFalg->waitTask);
		tempFalg = tempFalg->nextNode;
		for(int i=0;i<1000;i++){}
	}
}

#endif
