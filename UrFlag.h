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
    struct FlagNode*  nextNode;         /*!< A pointer to next flag node      */
    struct FlagNode*  prevNode;         /*!< A pointer to prev flag node      */
    U32               waitFlags;        /*!< Flag value                       */
    P_TASK           waitTask;         /*!< A pointer to task waitting flag  */
    U8                waitType;         /*!< Wait type                        */
}FLAG_NODE,*P_FLAG_NODE;


/**
 * @struct  Flag    flag.h
 * @brief   Flag control block
 * @details This struct use to mange event flag.
 */
typedef struct Flag
{
    U32           flagRdy;              /*!< Ready flag                       */
    U32           resetOpt;             /*!< Reset option                     */
    U32           flagActive;           /*!< Active flag                      */
    P_FLAG_NODE   headNode;             /*!< Head node                        */
    P_FLAG_NODE   tailNode;             /*!< Tail node                        */
}FLAG,*P_FLAG;

#ifdef __cplusplus
}
#endif

#endif /* URFLAG_H_ */
