/*
 * UrTask.h
 *
 *  Created on: 2018年1月27日
 *      Author: dclab
 */

#ifndef URTASK_H_
#define URTASK_H_

#ifdef __cplusplus
extern "C" {
#endif

// Task
typedef  struct Task
{
    OS_STK      		*stkPtr;                /*!< The current point of task.       						*/
    U8         	 		prio;                   /*!< Task priority.                   						*/
    U8          		state;                  /*!< Task status.                     						*/
    OS_ID      			taskID;                 /*!< Task ID.                         						*/
    OS_ID				eventID;				/*!< Event ID.                         						*/
    OS_ID				eventType;				/*!< Event Type.                    						*/

    OS_STK      		*stackStart;            /*!< The top point of task.           						*/
    OS_STK      		*stackEnd;
	FUNCPtr     		taskFuc;	
    U32         		delayTick;              /*!< The number of ticks which delay. 						*/
	U64         		wakeTick;              	/*!< The number of ticks which delay. 						*/
	U32					timeSlice;				/*!< When Round Robin , How many tick to Context Switch.	*/
	
    struct Task  		*Tasknext;              /*!< The pointer to next Task.        		 				*/
    struct Task  		*Taskprev;              /*!< The pointer to prev Task.         						*/
    struct Task  		*nextWaiting;         	/*!< The pointer to next Task that wait same event.			*/
}TASK,*P_TASK;


/*---------------------------- Variable declare ------------------------------*/
// save tcb ptr that created
extern volatile P_TASK 	TaskRunning;   /*!< A pointer to Task that is running.          */

extern volatile U8 		TaskSchedReq;

extern P_TASK 			getTaskByID(OS_ID id);

extern void 			UrTaskLock();
extern void 			UrTaskUnlock();
extern P_TASK 			getTaskRdy();
extern P_TASK 			getTaskDly();
extern void 			setTaskRdy(U32 TaskAddr);
extern void 			setTaskDly(U32 TaskAddr);
extern void 			UrInitTask();
extern OS_ID 			UrCreateTask(FUNCPtr task,void *argv,U8 prio,U32 timeSlice);
extern StatusType 		UrDelTask(OS_ID id);
extern void    			UrTaskTimeDispose(void);
extern void  			UrInsertToTaskRdyList  (P_TASK tcbInsert);
extern void  			InsertToTaskDlyList  (U32 timeout);
extern void  			RemoveFromTaskRdyList(P_TASK ptcb);
extern void  			RemoveFromTaskDlyList(P_TASK ptcb);
extern P_TASK  			EventTaskToWait(OS_ID eventID,OS_ID eventType,U8 eventSortType,P_TASK eventTaskList);
extern P_TASK  			EventTaskAwake(P_TASK eventTaskList);
extern void 			UrDelayTask(U32 delayTicks);
extern void	 			printRdyList();
extern void 			printDlyList();
extern void 			printWaitList(P_TASK WaitList);

#ifdef __cplusplus
}
#endif

#endif /* URTASK_H_ */

