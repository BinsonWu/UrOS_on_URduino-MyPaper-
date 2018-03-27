/*
 * UrSharedHeap.h
 *
 *  Created on: 2018年1月27日
 *      Author: dclab
 */

#ifndef URSHAREDHEAP_H_
#define URSHAREDHEAP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "UrOS.h"

typedef struct KernelHeap
{
  U32   startAddr;
  U32   endAddr;
}KHeap,*P_KHeap;


typedef struct UsedMemBlk
{
  void* nextMB;
  void* preMB;
}UMB,*P_UMB;


typedef struct FreeMemBlk
{
  struct FreeMemBlk* nextFMB;
  struct UsedMemBlk* nextUMB;
  struct UsedMemBlk* preUMB;
}FMB,*P_FMB;

/*---------------------------- Function Declare ------------------------------*/
extern void   UrInitKHeap(void);
extern void* UrKMalloc(U32);
extern void   UrKFree(void*);

#ifdef __cplusplus
}
#endif

#endif /* URSHAREDHEAP_H_ */
