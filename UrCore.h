/*
 * UrCore.h
 *
 *  Created on: 2018年1月28日
 *      Author: dclab
 */

#ifndef URCORE_H_
#define URCORE_H_

#ifdef __cplusplus
extern "C" {
#endif

extern volatile int core_id;
extern volatile U8 OSSchedLock;

extern void 	UrSetCoreID(int id);
extern int   	UrGetCoreID();
extern void 	UrSetCoreFlag(U32 nextCPU);
extern U32  	UrGetCoreFlag();


extern void 	UrSchedUnlock(void);
extern void 	UrSchedLock(void);
extern void 	printSharedBase(void);
extern void 	UrConfigureSharedInit(void);
extern void 	UrInitOS(void);
extern void 	UrStartOS(void);
extern U8 		UrGetSchedLock(void);

#ifdef __cplusplus
}
#endif

#endif /* URCORE_H_ */
