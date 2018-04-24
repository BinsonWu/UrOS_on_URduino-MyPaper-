/*
 * UrFlag.h
 *
 *  Created on: 2018年1月28日
 *      Author: dclab
 */

#ifndef URFLAG_H_
#define URFLAG_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct FlagNode
{
	OS_ID				id;
    U32               	waitFlags;        /*!< Flag value                       */
    P_TASK				waitTask;         /*!< A pointer to task waitting flag  */
    U8                	waitType;         /*!< Wait type                        */
	struct FlagNode*  	nextNode;         /*!< A pointer to next flag node      */
	struct FlagNode*  	prevNode;         /*!< A pointer to prev flag node      */
}FLAG_NODE,*P_FLAG_NODE;

extern U32 UrGetFlagActive();
extern U32 UrGetFlagReady();
extern void UrSetFlagActive(U32 newActive);
extern void UrSetFlagReady(U32 newReady);

extern P_FLAG_NODE UrGetFalgByID(OS_ID flagID);

extern void 		UrInitFlag();
extern OS_ID 		UrCreateFlag();
extern StatusType 	UrDelFlag(OS_ID flagID,U8 opt);
extern StatusType 	UrSetFlag(OS_ID flagID);
extern StatusType 	UrPendSingleFlag(OS_ID flagID,int timeout);
extern StatusType	UrPendMutipleFlag(U32 flags,int timeout,U8 waitType);
extern StatusType 	UrClearFlag(OS_ID flagID);
extern void 		UrRemoveTaskFromFlagWaiting(P_TASK removeTask);
extern void printFlagNodeList();

#ifdef __cplusplus
}
#endif

#endif /* URFLAG_H_ */
