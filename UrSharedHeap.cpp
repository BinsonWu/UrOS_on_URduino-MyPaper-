/*
 * UrSharedHeap.c
 *
 *  Created on: 2018年1月27日
 *      Author: dclab
 */

# include "UrSharedHeap.h"
#include "URduino_API.h"
#include "comm.h"
#include "portmacro.h"

P_FMB   	FMBlist = Ur_NULL;					/*!< Free memory block list	*/
KHeap   	Kheap   = {0};                  	/*!< Kernel heap control		*/

void   UrInitKHeap(void)
{
	println(">> UrInitKHeap");
	Kheap.startAddr = (U32)SHARED_BASE_HEAP;
	Kheap.endAddr 	= (U32)SHARED_BASE_HEAP+(U32)SHARED_SIZE_HEAP;
	FMBlist = (P_FMB)SHARED_BASE_HEAP;
	FMBlist->nextFMB 	= Ur_NULL;
	FMBlist->nextUMB 	= Ur_NULL;
	FMBlist->preUMB 	= Ur_NULL;
}

void*   UrKMalloc(U32 size)
{
	P_FMB freeMB,newFMB,preFMB;
	P_UMB usedMB,tmpUMB;
	U8*   memAddr;
	U32   freeSize;
	U32   KheapAddr;

	#if CFG_PAR_CHECKOUT_EN >0              /* Check validity of parameter        */
	    if( size == 0 )
	    {
	        return Ur_NULL;
	    }
	#endif

	/* Word alignment,and add used memory head size */
	size      = (((size+3)>>2)<<2) + 8;
	KheapAddr = Kheap.endAddr;        /* Get the end address of kernel heap   */
	freeMB = FMBlist;                 /* Get first item of free memory list   */
	preFMB = Ur_NULL;
	while(freeMB != Ur_NULL )            /* Is out of free memory list?          */
	{                                 /* No                                   */
		if(freeMB->nextUMB == Ur_NULL)   /* Is last item of free memory list?    */
		{                             /* Yes,get size for this free item      */
			freeSize = KheapAddr - (U32)(freeMB);
		}
		else                          /* No,get size for this free item       */
		{
			freeSize = (U32)(freeMB->nextUMB) -1 - (U32)(freeMB);
		}
		if(freeSize >= size)        /* If the size equal or greater than need */
		{                           /* Yes,assign in this free memory         */
			usedMB=(P_UMB)freeMB;/* Get the address for used memory block head*/

			/* Get the address for used memory block                          */
			memAddr = (U8*)((U32)(usedMB) + 8);

			/* Is left size of free memory smaller than 12?                   */
			if((freeSize-size) < 12)
			{
				/* Yes,malloc together(12 is the size of the header information
				   of free memory block ).                                    */
				if(preFMB != Ur_NULL)/* Is first item of free memory block list? */
				{                             /* No,set the link for list     */
					preFMB->nextFMB = freeMB->nextFMB;
				}
				else                          /* Yes,reset the first item     */
				{
					FMBlist = freeMB->nextFMB;
				}

				if(freeMB->nextUMB != Ur_NULL)   /* Is last item?                */
				{                             /* No,set the link for list     */
					tmpUMB = (P_UMB)((U32)(freeMB->nextUMB)-1);
					tmpUMB->preMB = (void*)((U32)usedMB|0x1);
				}

				usedMB->nextMB = freeMB->nextUMB;/* Set used memory block link*/
				usedMB->preMB  = freeMB->preUMB;
			}
			else                            /* No,the left size more than 12  */
			{
				/* Get new free memory block address                          */
				newFMB = (P_FMB)((U32)(freeMB) + size);

				if(preFMB != Ur_NULL)/* Is first item of free memory block list? */
				{
					preFMB->nextFMB = newFMB; /* No,set the link for list     */
				}
				else
				{
					FMBlist = newFMB;         /* Yes,reset the first item     */
				}

				/* Set link for new free memory block                         */
				newFMB->preUMB  = (P_UMB)((U32)usedMB|0x1);
				newFMB->nextUMB = freeMB->nextUMB;
				newFMB->nextFMB = freeMB->nextFMB;

				if(freeMB->nextUMB != Ur_NULL) /* Is last item?                  */
				{                           /* No,set the link for list       */
					tmpUMB = (P_UMB)((U32)(freeMB->nextUMB)-1);
					tmpUMB->preMB = newFMB;
				}

				usedMB->nextMB = newFMB;    /* Set used memory block link     */
				usedMB->preMB  = freeMB->preUMB;
			}

			if(freeMB->preUMB != Ur_NULL)      /* Is first item?                 */
			{                               /* No,set the link for list       */
				tmpUMB = (P_UMB)((U32)(freeMB->preUMB)-1);
				tmpUMB->nextMB = (void*)((U32)usedMB|0x1);
			}

			return memAddr;               /* Return used memory block address */
		}
		preFMB = freeMB;        /* Save current free memory block as previous */
		freeMB = freeMB->nextFMB;         /* Get the next item as current item*/
	}
	return Ur_NULL;                          /* Error return                     */
}

static P_FMB GetPreFMB(P_UMB usedMB)
{
    P_UMB preUMB;
    preUMB = usedMB;
    while(((U32)(preUMB->preMB)&0x1))   /* Is previous MB as FMB?             */
    {                                   /* No,get previous MB                 */
        preUMB = (P_UMB)((U32)(preUMB->preMB)-1);
    }
    return (P_FMB)(preUMB->preMB);      /* Yes,return previous MB             */
}

void   UrKFree(void* memBuf)
{
	P_FMB    curFMB,nextFMB,preFMB;
	    P_UMB    usedMB,nextUMB,preUMB;

	#if CFG_PAR_CHECKOUT_EN >0              /* Check validity of parameter        */
	    if(memBuf == Ur_NULL)
	    {
	        return;
	    }
	#endif

	    usedMB = (P_UMB)((U32)(memBuf)-8);

	#if CFG_PAR_CHECKOUT_EN >0              /* Check validity of parameter        */
	    if((U32)(memBuf) < Kheap.startAddr)
	    {
	        return;
	    }
	    if((U32)(memBuf) > Kheap.endAddr)
	    {
	        return;
	    }
	#endif

	#if CFG_PAR_CHECKOUT_EN >0              /* Check UMB in list                  */
	    if((U32)(usedMB) < (U32)(FMBlist))
	    {
	        preUMB = (P_UMB)((U32)(FMBlist->preUMB)-1);
	        while(preUMB != usedMB)
	        {
	            if(preUMB == Ur_NULL)
	            {
	                OsSchedUnlock();
	                return;
	            }
	            preUMB = (P_UMB)((U32)(preUMB->preMB)-1);
	        }
	    }
	    else
	    {
	        if(FMBlist == Ur_NULL)
	        {
	            nextUMB = (P_UMB)(Kheap.startAddr);
	        }
	        else
	        {
	            if(FMBlist->nextUMB != Ur_NULL)
	            {
	                nextUMB = (P_UMB)((U32)(FMBlist->nextUMB)-1);
	            }
	            else
	            {
	                nextUMB = Ur_NULL;
	            }
	        }

	        while(nextUMB != usedMB)
	        {
	            if(nextUMB == Ur_NULL)
	            {
	                OsSchedUnlock();
	                return;
	            }
	            if(((U32)(nextUMB->nextMB)&0x1) == 0)
	            {
	                nextFMB = (P_FMB)(nextUMB->nextMB);
	                nextUMB = (P_UMB)((U32)(nextFMB->nextUMB)-1);
	            }
	            else
	            {
	                nextUMB = (P_UMB)((U32)(nextUMB->nextMB)-1);
	            }
	        }
	    }
	#endif


	    /* Is between two free memory block? */
	    if( (((U32)(usedMB->nextMB)&0x1) == 0) && (((U32)(usedMB->preMB)&0x1)==0) )
	    {                             /* Yes,is the only one item in kernel heap? */
	        if((usedMB->nextMB == Ur_NULL) && (usedMB->preMB == Ur_NULL))
	        {
	            curFMB = (P_FMB)usedMB;       /* Yes,release this item            */
	            curFMB->nextFMB = Ur_NULL;
	            curFMB->nextUMB = Ur_NULL;
	            curFMB->preUMB  = Ur_NULL;
	            FMBlist = curFMB;
	        }
	        else if(usedMB->preMB == Ur_NULL)    /* Is the first item in kernel heap */
	        {
	            /* Yes,release this item,and set link for list                    */
	            curFMB  = (P_FMB)usedMB;
	            nextFMB = (P_FMB)usedMB->nextMB;

	            curFMB->nextFMB = nextFMB->nextFMB;
	            curFMB->nextUMB = nextFMB->nextUMB;
	            curFMB->preUMB  = Ur_NULL;
	            FMBlist         = curFMB;
	        }
	        else if(usedMB->nextMB == Ur_NULL)   /* Is the last item in kernel heap  */
	        {                      /* Yes,release this item,and set link for list */
	            curFMB = (P_FMB)(usedMB->preMB);
	            curFMB->nextFMB = Ur_NULL;
	            curFMB->nextUMB = Ur_NULL;
	        }
	        else                  /* All no,show this item between two normal FMB */
	        {
	            /* release this item,and set link for list                        */
	            nextFMB = (P_FMB)usedMB->nextMB;
	            curFMB  = (P_FMB)(usedMB->preMB);

	            curFMB->nextFMB = nextFMB->nextFMB;
	            curFMB->nextUMB = nextFMB->nextUMB;
	        }
	    }
	    else if(((U32)(usedMB->preMB)&0x1) == 0)  /* Is between FMB and UMB?      */
	    {
	        if(usedMB->preMB == Ur_NULL)   /* Yes,is the first item in kernel heap?  */
	        {
	            /* Yes,release this item,and set link for list                    */
	            curFMB          = (P_FMB)usedMB;
	            nextUMB         = (P_UMB)usedMB->nextMB;
	            curFMB->nextUMB = nextUMB;
	            curFMB->preUMB  = Ur_NULL;
	            curFMB->nextFMB = FMBlist;
	            FMBlist         = curFMB;
	        }
	        else                    /* No,release this item,and set link for list */
	        {
	            curFMB          = (P_FMB)usedMB->preMB;
	            nextUMB         = (P_UMB)usedMB->nextMB;
	            curFMB->nextUMB = nextUMB;
	        }

	    }
	    else if(((U32)(usedMB->nextMB)&0x1) == 0)   /* Is between UMB and FMB?    */
	    {                                           /* Yes                        */
	        preUMB = (P_UMB)(usedMB->preMB);        /* Get previous UMB           */
	        curFMB = (P_FMB)(usedMB);               /* new FMB                    */
	        preFMB = GetPreFMB(usedMB);             /* Get previous FMB           */
	        if(preFMB == Ur_NULL)                      /* Is previous FMB==Ur_NULL?     */
	        {
	            nextFMB = FMBlist;                  /* Yes,get next FMB           */
	            FMBlist = curFMB;   /* Reset new FMB as the first item of FMB list*/
	        }
	        else
	        {
	            nextFMB = preFMB->nextFMB;          /* No,get next FMB            */
	            preFMB->nextFMB  = curFMB;          /* Set link for FMB list      */
	        }

	        if(nextFMB == Ur_NULL)           /* Is new FMB as last item of FMB list? */
	        {
	            curFMB->preUMB  = preUMB;           /* Yes,set link for list      */
	            curFMB->nextUMB = Ur_NULL;
	            curFMB->nextFMB = Ur_NULL;
	        }
	        else
	        {
	            curFMB->preUMB  = preUMB;           /* No,set link for list       */
	            curFMB->nextUMB = nextFMB->nextUMB;
	            curFMB->nextFMB = nextFMB->nextFMB;
	        }
	    }
	    else                                    /* All no,show UMB between two UMB*/
	    {
	        curFMB  = (P_FMB)(usedMB);          /* new FMB                        */
	        preFMB  = GetPreFMB(usedMB);        /* Get previous FMB               */
	        preUMB  = (P_UMB)(usedMB->preMB);   /* Get previous UMB               */
	        nextUMB = (P_UMB)(usedMB->nextMB);  /* Get next UMB                   */

	        if(preFMB == Ur_NULL )                 /* Is previous FMB==Ur_NULL?         */
	        {
	            nextFMB = FMBlist;              /* Yes,get next FMB               */
	            FMBlist = curFMB;  /* Reset new FMB as the first item of FMB list */
	      	}
	      	else
	      	{
	            nextFMB = preFMB->nextFMB;      /* No,get next FMB                */
	            preFMB->nextFMB = curFMB;       /* Set link for FMB list          */
	      	}

	        curFMB->preUMB  = preUMB;           /* Set current FMB link for list  */
	        curFMB->nextUMB = nextUMB;
	        curFMB->nextFMB = nextFMB;
	    }

	    if(curFMB->preUMB != Ur_NULL)/* Is current FMB as first item in kernel heap? */
	    {                         /* No,set link for list                         */
	      	preUMB = (P_UMB)((U32)(curFMB->preUMB)-1);
	      	preUMB->nextMB = (void*)curFMB;
	    }
	    if(curFMB->nextUMB != Ur_NULL)/* Is current FMB as last item in kernel heap? */
	    {                          /* No,set link for list                        */
	      	nextUMB = (P_UMB)((U32)(curFMB->nextUMB)-1);
	      	nextUMB->preMB = (void*)curFMB;
	    }
}

