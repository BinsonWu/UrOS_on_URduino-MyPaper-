#include "UrOSInclude.h"

void UrAcquire(P_SPINLOCK lk)
{
	while(lk->locked == 1){}
	lk->locked = 1;
}

void UrRelease(P_SPINLOCK lk)
{
	lk->locked = 0;
}