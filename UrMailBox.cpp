/*
 * UrMailBox.c
 *
 *  Created on: 2018年1月28日
 *      Author: dclab
 */

#include "UrOSInclude.h"

#if CFG_MAILBOX_EN >0

void UrMailBoxLock()
{
	U32 *p = (U32 *)SHARED_BASE_LOCK;
	while(*p & (1 << LOCK_SHIFT_MAILBOX) ){}
	*p = *p | (1 << LOCK_SHIFT_MAILBOX);
}

void UrMailBoxUnlock()
{
	U32 *p = (U32 *)SHARED_BASE_LOCK;
	if( *p & (1 << LOCK_SHIFT_MAILBOX)  )
		*p = *p & (~(1 << LOCK_SHIFT_MAILBOX));
}

P_MAILBOX getMailBoxByID(OS_ID id)
{
	U32 *p = (U32 *)SHARED_BASE_MAILBOX;
	P_MAILBOX tempMailBox;
	p = p + id*SHARED_SIZE_MAILBOX;
	tempMailBox = (P_MAILBOX)p;
	return tempMailBox;
}

void UrInitMailBox()
{
	// Set Get ID Table
	//getIDTable[EVENT_TYPE_MAILBOX] = (FUNCPtr)getMailBoxByID;

	P_MAILBOX tempMailBox;
	OS_ID i,j;
	for(i=0;i<CFG_MAX_MAILBOX;i++)
	{
		tempMailBox = getMailBoxByID(i);
		tempMailBox->id 								= -1;                         /*!< MailBox id                           */
		tempMailBox->eventSortType 			= 0;            				/*!< 0:FIFO 1: Preemptive by prio     */
		tempMailBox->eventCounter 			= 0;               			/*!< Counter of semaphore.            */
		tempMailBox->initialEventCounter 	= 0;        					/*!< Initial counter of semaphore.    */
		tempMailBox->eventTaskList 			= Ur_NULL;          	/*!< Task waitting list.              */
		for(j=0;j<CFG_MSG_SIZE;j++)
		{
			tempMailBox->Message[j] = '\0';
		}
	}
}

OS_ID UrCreateMailBox()
{
	P_MAILBOX tempMailBox;
	OS_ID i;
UrSchedLock();
UrMailBoxLock();
	for(i=0;i<CFG_MAX_MAILBOX;i++)
	{
		tempMailBox = getMailBoxByID(i);
		if(tempMailBox->id == -1)
		{
			tempMailBox->id = i;
		UrMailBoxUnlock();
		UrSchedUnlock();
			return i;                      /* Return mutex ID                    */
		}
	}
UrMailBoxUnlock();
UrSchedUnlock();
	return E_CREATE_FAIL;               /* No free mutex control block        */
}

StatusType UrDelMailBox(OS_ID id)
{
    if(id >= CFG_MAX_MAILBOX)
    {
        return E_INVALID_ID;
    }
UrSchedLock();
UrMailBoxLock();
    P_MAILBOX tempMailBox = getMailBoxByID(id);
    tempMailBox->	id 							= -1;                         	/*!< MailBox id                           */
	tempMailBox->eventSortType 		= 0;            				/*!< 0:FIFO 1: Preemptive by prio     */
	tempMailBox->eventCounter 			= 0;               			/*!< Counter of semaphore.            */
	tempMailBox->initialEventCounter = 0;        					/*!< Initial counter of semaphore.    */
	tempMailBox->eventTaskList 			= Ur_NULL;          	/*!< Task waitting list.              */
	OS_ID j;
	for(j=0;j<CFG_MSG_SIZE;j++)
	{
		tempMailBox->Message[j] = '\0';
	}
UrMailBoxUnlock();
UrSchedUnlock();
	return E_OK;
}

StatusType setMessage(OS_ID id,U8 *Message)
{
UrSchedLock();
UrMailBoxLock();
	P_MAILBOX tempMailBox = getMailBoxByID(id);
	U8 count=0;
	while(Message[count]!='\0')
	{
		tempMailBox->Message[count] = Message[count];
		count++;
	}
UrMailBoxUnlock();
UrSchedUnlock();
	return E_OK;
}

StatusType getMessage(OS_ID id,U8 *Message)
{
UrSchedLock();
UrMailBoxLock();
	P_MAILBOX tempMailBox = getMailBoxByID(id);
	U8 count=0;
	while(tempMailBox->Message[count]!='\0')
	{
		Message[count] = tempMailBox->Message[count];
		tempMailBox->Message[count] = '\0';
		count++;
	}
UrMailBoxUnlock();
UrSchedUnlock();
	return E_OK;
}

#endif
