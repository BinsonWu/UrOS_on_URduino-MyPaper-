/*
 * UrMutex.h
 *
 *  Created on: 2018年1月28日
 *      Author: dclab
 */

#ifndef URMUTEX_H_
#define URMUTEX_H_

#ifdef __cplusplus
extern "C" {
#endif


// Mutex
typedef struct Mutex
{
	U8 			id;
	OS_ID		ownTaskID;
    U8       	flag;               			/*!< Mutex flag.                      */
    P_TASK  	eventTaskList;              	/*!< waitting the Mutex.              */
	SPINLOCK	lock;
}MUTEX,*P_MUTEX;

/*---------------------------- Function declare ------------------------------*/
extern void   RemoveMutexList(P_TASK ptcb);

extern P_MUTEX 		getMutexByID(OS_ID mutexID);
extern void 		UrInitMutex();
extern OS_ID 		UrCreateMutex();
extern StatusType 	UrDeleteMutex(OS_ID mutexID,U8 opt);
extern StatusType 	UrEnterMutexSection(OS_ID mutexID);
extern StatusType 	UrLeaveMutexSection(OS_ID mutexID);
extern void 		UrRemoveTaskFromMutexWaiting(P_TASK removeTask);


#ifdef __cplusplus
}
#endif

#endif /* URMUTEX_H_ */
