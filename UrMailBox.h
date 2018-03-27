/*
 * UrMailBox.h
 *
 *  Created on: 2018年1月28日
 *      Author: dclab
 */

#ifndef URMAILBOX_H_
#define URMAILBOX_H_

#ifdef __cplusplus
extern "C" {
#endif

// MailBox
typedef struct MailBox
{
	U8   			Message[CFG_MSG_SIZE];                   	/*!< Point to mailbox or queue struct */
    U8      		id;                         									/*!< ECB id                           */
    U8      		eventSortType:4;            							/*!< 0:FIFO 1: Preemptive by prio     */
    U16     		eventCounter;               							/*!< Counter of semaphore.            */
    U16     		initialEventCounter;       						/*!< Initial counter of semaphore.    */
    P_TASK 	eventTaskList;               							/*!< Task waitting list.              */
}MAILBOX,*P_MAILBOX;

/*---------------------------- Function declare ------------------------------*/
extern void UrMailBoxLock();
extern void UrMailBoxUnlock();
extern P_MAILBOX getMailBoxByID(OS_ID id);
extern void UrInitMailBox();
extern OS_ID UrCreateMailBox();
extern StatusType UrDelMailBox(OS_ID id);
extern StatusType setMessage(OS_ID id,U8 *Message);
extern StatusType getMessage(OS_ID id,U8 *Message);

#ifdef __cplusplus
}
#endif

#endif /* URMAILBOX_H_ */
