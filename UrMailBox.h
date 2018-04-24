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
	char   			Message[CFG_MSG_SIZE];                   		/*!< Point to mailbox or queue struct */
    U8      		id;                         					/*!< Event id                         */
    U8      		eventSortType:4;            					/*!< 1:FIFO 2: Preemptive by prio     */
    P_TASK 			eventTaskList;               					/*!< Task waitting list.              */
	SPINLOCK		lock;
}MAILBOX,*P_MAILBOX;

/*---------------------------- Function declare ------------------------------*/

extern P_MAILBOX 	UrGetMailBoxByID(OS_ID mailboxID);
extern void 		UrInitMailBox();
extern OS_ID 		UrCreateMailBox(U8 sortType);
extern StatusType 	UrDelMailBox(OS_ID mailboxID,U8 opt);
extern StatusType 	UrPostMailBox(OS_ID mailboxID,char *Message);
extern StatusType 	UrPendMailBox(OS_ID mailboxID,char *Message,int timeout);
extern void 		UrRemoveTaskFromMailboxWaiting(P_TASK removeTask);

#ifdef __cplusplus
}
#endif

#endif /* URMAILBOX_H_ */
