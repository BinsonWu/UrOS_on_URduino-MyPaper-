#ifndef URSPINLOCK_H_
#define URSPINLOCK_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct  Spinlock
{
	U8 locked;
}SPINLOCK,*P_SPINLOCK;

extern void UrAcquire(P_SPINLOCK lk);
extern void UrRelease(P_SPINLOCK lk);

#ifdef __cplusplus
}
#endif

#endif /* URSPINLOCK_H_ */
