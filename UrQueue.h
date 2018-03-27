/*
 * UrQueue.h
 *
 *  Created on: 2018年1月28日
 *      Author: dclab
 */

#ifndef URQUEUE_H_
#define URQUEUE_H_

#ifdef __cplusplus
extern "C" {
#endif

// Queue
typedef struct Queue
{
    void    **qStart;                   /*!<                                  */
    U8      id;                         /*!<                                  */
    U16     head;                       /*!< The header of queue              */
    U16     tail;                       /*!< The end of queue                 */
    U16     qMaxSize;                   /*!< The max size of queue            */
    U16     qSize;                      /*!< Current size of queue            */
}QUEUE,*P_QUEUE;

#ifdef __cplusplus
}
#endif

#endif /* URQUEUE_H_ */
