#include "UrOSInclude.h"
#include "comm.h"
#include "portmacro.h"

#if CFG_DEBUG_EN > 0

/*
#define E_CREATE_FAIL         	(StatusType)-1
#define E_OK                  	(StatusType)0
#define E_INVALID_ID          	(StatusType)1
#define E_INVALID_PARAMETER   	(StatusType)2
#define E_CALL                	(StatusType)3
#define E_TASK_WAITING        	(StatusType)4
#define E_TIMEOUT             	(StatusType)5
#define E_SEM_FULL            	(StatusType)6
#define E_MBOX_FULL           	(StatusType)7
#define E_QUEUE_FULL          	(StatusType)8
#define E_SEM_EMPTY           	(StatusType)9
#define E_MBOX_EMPTY          	(StatusType)10
#define E_QUEUE_EMPTY         	(StatusType)11
#define E_FLAG_NOT_READY      	(StatusType)12
#define E_ALREADY_IN_WAITING  	(StatusType)13
#define E_TASK_NOT_WAITING    	(StatusType)14
#define E_TASK_WAIT_OTHER     	(StatusType)15
#define E_EXCEED_MAX_NUM      	(StatusType)16
#define E_NOT_IN_DELAY_LIST   	(StatusType)17
#define E_SEV_REQ_FULL        	(StatusType)18
#define E_NOT_FREE_ALL        	(StatusType)19
#define E_PROTECTED_TASK      	(StatusType)20
#define E_OS_IN_LOCK          	(StatusType)21

#define E_MUTEX_DIF_SECTION	 	(StatusType)22
*/

void UrDebug(StatusType ErrorMessage)
{
	switch( ErrorMessage )   
	{  
		case E_CREATE_FAIL:  
			println("E_CREATE_FAIL");
			break;
		case E_OK:  
			println("E_OK");
			break;
		case E_INVALID_ID:  
			println("E_INVALID_ID");
			break;
		case E_INVALID_PARAMETER:  
			println("E_INVALID_PARAMETER");
			break;
		case E_CALL:  
			println("E_CALL");
			break;
		case E_TASK_WAITING:  
			println("E_TASK_WAITING");
			break;
		case E_TIMEOUT:  
			println("E_TIMEOUT");
			break;
		case E_SEM_FULL:  
			println("E_SEM_FULL");
			break;
		case E_MBOX_FULL:  
			println("E_MBOX_FULL");
			break;
		case E_QUEUE_FULL:  
			println("E_QUEUE_FULL");
			break;
		case E_SEM_EMPTY:  
			println("E_SEM_EMPTY");
			break;
		case E_MBOX_EMPTY:  
			println("E_MBOX_EMPTY");
			break;
		case E_QUEUE_EMPTY:  
			println("E_QUEUE_EMPTY");
			break;
		case E_FLAG_NOT_READY:  
			println("E_FLAG_NOT_READY");
			break;
		case E_ALREADY_IN_WAITING:  
			println("E_ALREADY_IN_WAITING");
			break;
		case E_TASK_NOT_WAITING:  
			println("E_TASK_NOT_WAITING");
			break;
		case E_TASK_WAIT_OTHER:  
			println("E_TASK_WAIT_OTHER");
			break;
		case E_EXCEED_MAX_NUM:  
			println("E_EXCEED_MAX_NUM");
			break;
		case E_NOT_IN_DELAY_LIST:  
			println("E_NOT_IN_DELAY_LIST");
			break;
		case E_SEV_REQ_FULL:  
			println("E_SEV_REQ_FULL");
			break;
		case E_NOT_FREE_ALL:  
			println("E_NOT_FREE_ALL");
			break;
		case E_PROTECTED_TASK:  
			println("E_PROTECTED_TASK");
			break;
		case E_OS_IN_LOCK:  
			println("E_OS_IN_LOCK");
			break;
		case E_MUTEX_DIF_SECTION:  
			println("E_MUTEX_DIF_SECTION");
			break;
		default :  
			println("No Message!");
			break; 
	}  
}

#endif