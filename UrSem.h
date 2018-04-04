/*
 * UrSem.h
 *
 *  Created on: 2018年1月28日
 *      Author: dclab
 */

#ifndef URSEM_H_
#define URSEM_H_

#ifdef __cplusplus
extern "C" {
#endif

// Sem
typedef struct Sem
{
    U8      	id;                         	/*!< Sem id                           */
    U8      	eventSortType;            		/*!< 0:FIFO 1: Preemptive by prio     */
    U16    		eventCounter;               	/*!< Counter of semaphore.            */
    P_TASK 		eventTaskList;               	/*!< Task waitting list.              */
	SPINLOCK	lock;
}SEM,*P_SEM;


/*---------------------------- Function declare ------------------------------*/
extern P_SEM 		getSemByID(OS_ID semID);
extern void 		UrInitSem();
extern OS_ID 		UrCreateSem(U32 initCount,U8 sortType);
extern StatusType 	UrDelSem(OS_ID semID,U8 opt);
extern StatusType 	UrPostSem(OS_ID semID);
extern StatusType 	UrPendSem(OS_ID semID,int timeout);
extern void 		UrRemoveTaskFromSemWaiting(P_TASK removeTask);


#ifdef __cplusplus
}
#endif

#endif /* URSEM_H_ */
