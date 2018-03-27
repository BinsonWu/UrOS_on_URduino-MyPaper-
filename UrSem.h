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
    U8      		id;                         				/*!< Sem id                           */
    U8      		eventSortType:4;            	/*!< 0:FIFO 1: Preemptive by prio     */
    U16    		eventCounter;               	/*!< Counter of semaphore.            */
    P_TASK 	eventTaskList;               /*!< Task waitting list.              */
}SEM,*P_SEM;


/*---------------------------- Function declare ------------------------------*/
extern void UrSemLock();
extern void UrSemUnlock();
extern P_SEM getSemByID(OS_ID id);
extern void UrInitSem();
extern OS_ID UrCreateSem();
extern StatusType UrDelSem(OS_ID id);

#ifdef __cplusplus
}
#endif

#endif /* URSEM_H_ */
