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

/*---------------------------- Resource status -------------------------------*/
#define   MUTEX_FREE        0           /*!< Mutex is free                    */
#define   MUTEX_OCCUPY      1           /*!< Mutex is occupy                  */
#define   WAITING_MUTEX     0x80

// Mutex
typedef struct Mutex
{
	U8		exist;
    U8       originalPrio;              /*!< Mutex priority.                  */
    U8       mutexFlag;                 /*!< Mutex flag.                      */
    OS_ID   taskID;                    /*!< Task ID.                         */
    OS_ID   hipriTaskID;               /*!< Mutex ID.                        */
    P_TASK  waittingList;              /*!< waitting the Mutex.              */
}MUTEX,*P_MUTEX;

/*---------------------------- Function declare ------------------------------*/
extern void   RemoveMutexList(P_TASK ptcb);


#ifdef __cplusplus
}
#endif

#endif /* URMUTEX_H_ */
