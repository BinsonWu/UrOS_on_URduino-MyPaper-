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

extern volatile U64 OSTickCnt;          	/*!< Counter for current system ticks.    	*/
extern volatile U64 OSScheduleTime;		/*!< When to Schedule 						*/

/*!< Initial context of task being created	*/
extern void 	UrDisableInterrupts();
extern S32 		UrStartSchedule();
extern OS_STK  *UrInitTaskContext(FUNCPtr task,void *param,OS_STK *pstk);
extern void    	UrSwitchContext(void);         /*!< Switch context                   */
extern void  	UrTimeDispose(void);     /*!< Time dispose function.               */
extern void  	UrSchedule(void);
extern U64 		UrGetSystemTime();
void 			UrSchedule();

#ifdef __cplusplus
}
#endif

#endif