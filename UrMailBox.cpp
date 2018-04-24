/*
 * UrMailBox.c
 *
 *  Created on: 2018年1月28日
 *      Author: dclab
 */

#include "UrOSInclude.h"
#include "comm.h"
#include "portmacro.h"

#if CFG_MAILBOX_EN >0

/*
					< UrMailBox Architecture >
---------------------------------------------------------------
- MailBox Array
	Length : CFG_MAX_MAILBOX
	|  0  | , |  1  | , ... , |  CFG_MAX_MAILBOX-1  |
*/


P_MAILBOX UrGetMailBoxByID(OS_ID mailboxID)
{
	if(mailboxID >= CFG_MAX_MAILBOX)
    {
        return Ur_NULL;
    }
	U32 *p = (U32 *)SHARED_BASE_MAILBOX;
	P_MAILBOX tempMailBox;
	p = p + mailboxID*SHARED_SIZE_MAILBOX/4;
	tempMailBox = (P_MAILBOX)p;
	return tempMailBox;
}

void UrInitMailBox()
{
	P_MAILBOX tempMailBox;
	int i,j;
	for(i=0;i<CFG_MAX_MAILBOX;i++)
	{
		tempMailBox = UrGetMailBoxByID(i);
		tempMailBox->id 					= INVALID_ID;			/*!< MailBox id							*/
		tempMailBox->eventSortType 			= 0;            		/*!< 1:FIFO 2: Preemptive by prio    	*/
		tempMailBox->eventTaskList 			= Ur_NULL;          	/*!< Task waitting list.              	*/
		UrRelease(&tempMailBox->lock);
		for(j=0;j<CFG_MSG_SIZE;j++)
		{
			tempMailBox->Message[j] = '\0';
		}
	}
}

OS_ID UrCreateMailBox(U8 sortType)
{
	P_MAILBOX tempMailBox;
	int i;
UrSchedLock();
	for(i=0;i<CFG_MAX_MAILBOX;i++)
	{
		tempMailBox = UrGetMailBoxByID(i);
		if(tempMailBox->id == INVALID_ID)
		{
			tempMailBox->id 			= i;
			tempMailBox->eventSortType	= sortType;
UrSchedUnlock();
			return i;                      /* Return mutex ID                    */
		}
	}
UrSchedUnlock();
	return E_CREATE_FAIL;               /* No free mutex control block        */
}

/*
UrDelMailBox()
------------------------------------------------------------
1. Delete No Task Pend on Waitting List.
	- Pend , return error.
2. Set MailBox.
3. Remove Event Waitting List.
*/
StatusType UrDelMailBox(OS_ID mailboxID,U8 opt)
{
    if(mailboxID >= CFG_MAX_MAILBOX)
    {
        return E_INVALID_ID;
    }
	
UrSchedLock();
    P_MAILBOX tempMailBox = UrGetMailBoxByID(mailboxID);
UrAcquire(&tempMailBox->lock);

	// - Pend , return error.
	if(opt == OPT_DEL_NO_PEND && tempMailBox->eventTaskList != Ur_NULL )
	{
UrRelease(&tempMailBox->lock);
UrSchedUnlock();
		return E_TASK_WAITING;
	}
	// 2. Set MailBox.
    tempMailBox->id 					= INVALID_ID;                   /*!< MailBox id                       */
	tempMailBox->eventSortType 			= 0;            				/*!< 0:FIFO 1: Preemptive by prio     */
	int j;
	for(j=0;j<CFG_MSG_SIZE;j++)
	{
		tempMailBox->Message[j] = '\0';
	}
	// 3. Remove Event Waitting List.
UrTaskLock();
	P_TASK tempTask = tempMailBox->eventTaskList;
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
	tempMailBox->eventTaskList 			= Ur_NULL;          			/*!< Task waitting list.              */	
	
UrRelease(&tempMailBox->lock);
UrSchedUnlock();
	return E_OK;
}

/*
UrPostMailBox()
------------------------------------------------------------
No Change || TASK_WAITING -> TASK_Ready
1. Event Waitting List of MailBox is NULL.
2. Event Waitting List of MailBox is Not NULL.
*/
StatusType UrPostMailBox(OS_ID mailboxID,char *Message)
{
	if(OSSchedLock != 0)                /* Is OS lock?                        	*/
    {
        return E_OS_IN_LOCK;            /* Yes,error return.                   	*/
    }
	if(mailboxID >= CFG_MAX_MAILBOX)
    {
        return E_INVALID_ID;
    }
	
UrSchedLock();
	P_MAILBOX tempMailBox = UrGetMailBoxByID(mailboxID);
UrAcquire(&tempMailBox->lock);

	if(tempMailBox->Message[0] != '\0')
	{
UrRelease(&tempMailBox->lock);
UrSchedUnlock();
		return E_MBOX_FULL;
	}
UrTaskLock();
	int count=0;
	while(Message[count]!='\0')
	{
		tempMailBox->Message[count] = Message[count];
		count++;
	}
	
	if(tempMailBox->eventTaskList != Ur_NULL)
	{
    	// Task State : Waiting -> Ready
		if(tempMailBox->eventTaskList->delayTick != OPT_WAIT_FOREVER)
		{
			RemoveFromTaskDlyList(tempMailBox->eventTaskList);						// Waiting -> Ready
		}
		TaskWakeUp = tempMailBox->eventTaskList;
		
		tempMailBox->eventTaskList  = EventTaskAwake(tempMailBox->eventTaskList);		// Set New EventTaskList
		// Because of Post , we need to schedule the Task waked up.
    	TaskSchedReq = Ur_TRUE;
	}
UrTaskUnlock();
UrRelease(&tempMailBox->lock);
UrSchedUnlock();
	return E_OK;
}

/*
No Change || TASK_RUNNING -> TASK_WAITING
1. Message of MailBox is '\0'
2. Message of MailBox is not '\0'
	- Timeout = 0, Don't Wait.
	- Timeout != -1, Add to Delay Queue.
3. When it back.
	- Timeout
	- Post
*/
StatusType UrPendMailBox(OS_ID mailboxID,char *Message,int timeout)
{
	if(OSSchedLock != 0)                /* Is OS lock?                        	*/
    {
        return E_OS_IN_LOCK;            /* Yes,error return.                   	*/
    }
	if(mailboxID >= CFG_MAX_MAILBOX)
    {
        return E_INVALID_ID;
    }
	
	int count=0;
UrSchedLock();
	P_MAILBOX tempMailBox = UrGetMailBoxByID(mailboxID);
UrAcquire(&tempMailBox->lock);
	if(tempMailBox->Message[0] != '\0') 	/* If Message of is not '\0' 				*/
    {
		while(tempMailBox->Message[count]!='\0')
		{
			Message[count] = tempMailBox->Message[count];
			tempMailBox->Message[count] = '\0';
			count++;
		}
UrRelease(&tempMailBox->lock);
UrSchedUnlock();
		return E_OK;
    }
    else                                /* Resource is not available          			*/
    {            						/* Block task until event or timeout occurs 	*/
		if(timeout == 0)
		{
UrRelease(&tempMailBox->lock);
UrSchedUnlock();
			return E_MBOX_EMPTY;
		}
UrTaskLock();
		if(timeout != OPT_WAIT_FOREVER)
		{
			// Task State : Running -> Waiting , So only need insert to Delay List.
			InsertToTaskDlyList(timeout);	// Running -> Waiting
		}
    	// Set New EventTaskList
    	tempMailBox->eventTaskList = EventTaskToWait(tempMailBox->id,EVENT_TYPE_MAILBOX,tempMailBox->eventSortType,tempMailBox->eventTaskList);
    	// Because of Pending , we need to schedule until it getting a post.
    	TaskSchedReq = Ur_TRUE;
UrTaskUnlock();
UrRelease(&tempMailBox->lock);
UrSchedUnlock();
    }
	// 3. When it back.
	// Timeout
UrSchedLock();
tempMailBox = UrGetMailBoxByID(mailboxID);
UrAcquire(&tempMailBox->lock);
	if(tempMailBox->Message[0] == '\0')
	{
UrRelease(&tempMailBox->lock);
UrSchedUnlock();
		return E_TIMEOUT;
	}
	
	// Post
	while(tempMailBox->Message[count]!='\0')
	{
		Message[count] = tempMailBox->Message[count];
		tempMailBox->Message[count] = '\0';
		count++;
	}
UrRelease(&tempMailBox->lock);
UrSchedUnlock();
	return E_OK;
}

void UrRemoveTaskFromMailboxWaiting(P_TASK removeTask)
{
	P_MAILBOX tempMailBox = UrGetMailBoxByID(removeTask->eventID);
UrAcquire(&tempMailBox->lock);
UrTaskLock();
	P_TASK tempWaitTask = tempMailBox->eventTaskList;
	// If first Waiting Task is Delete Task, change the head.
	if( tempWaitTask== removeTask)
	{
		tempMailBox->eventTaskList = tempWaitTask->nextWaiting;
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
UrRelease(&tempMailBox->lock);
}

#endif
