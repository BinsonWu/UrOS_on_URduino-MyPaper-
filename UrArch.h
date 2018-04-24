/*
 * UrArch.h
 *
 *  Created on: 2018年1月28日
 *      Author: dclab
 */
 
#ifndef URARCH_H_
#define URARCH_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "UrOS.h"
/*!< Initial context of task being created	*/
extern void 	UrSetOSTickCnt(U64 newOSTickCnt);
extern U64 		UrGetOSTickCnt();
extern void 	UrSetOSScheduleTime(U64 newOSScheduleTime);
extern U64 		UrGetOSScheduleTime();

extern void 	UrDisableInterrupts();
extern S32 		UrStartSchedule();
extern OS_STK  *UrInitTaskContext(FUNCPtr task,void *param,OS_STK *pstk);
extern void    	UrSwitchContext(void);         /*!< Switch context                   */
extern void  	UrTimeDispose(void);     /*!< Time dispose function.               */
extern void  	UrSchedule(void);
void 			UrSchedule();

#ifdef __cplusplus
}
#endif

#endif