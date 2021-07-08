/**
********************************************************************************
* @file gtOsMsgQ.h
*
* @brief Operating System wrapper. Message queues
*
* @version   2
********************************************************************************
*/
/*******************************************************************************
* gtOsMsgQ.h
*
* DESCRIPTION:
*       Operating System wrapper. Message queues
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 2 $
*******************************************************************************/

#ifndef __gtOsMsgQh
#define __gtOsMsgQh

#ifdef __cplusplus
extern "C" {
#endif

/************* Includes *******************************************************/

#include <gtOs/gtGenTypes.h>

/************* Defines ********************************************************/
#define OS_MSGQ_WAIT_FOREVER 0
#define OS_MSGQ_NO_WAIT      0xffffffff

/************* Typedefs *******************************************************/
typedef GT_UINTPTR GT_MSGQ_ID;

/************* Functions ******************************************************/

/**
* @internal CPSS_osMsgQCreate function
* @endinternal
*
* @brief   Create and initialize a message queue.
*
* @param[in] name                     - message queue name
* @param[in] maxMsgs                  - max messages in queue
* @param[in] maxMsgSize               - max length of single message
*
* @param[out] msgqId                   - message queue id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
#define osMsgQCreate CPSS_osMsgQCreate
GT_STATUS CPSS_osMsgQCreate
(
    IN  const char    *name,
    IN  GT_U32        maxMsgs,
    IN  GT_U32        maxMsgSize,
    OUT GT_MSGQ_ID    *msgqId
);

/**
* @internal CPSS_osMsgQDelete function
* @endinternal
*
* @brief   Delete message queue
*
* @param[in] msgqId                   - message queue Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
#define osMsgQDelete CPSS_osMsgQDelete
GT_STATUS CPSS_osMsgQDelete
(
    IN GT_MSGQ_ID msgqId
);

/**
* @internal CPSS_osMsgQSend function
* @endinternal
*
* @brief   Send message to queue
*
* @param[in] msgqId                   - Message queue Idsemaphore Id
* @param[in] message                  -  data pointer
* @param[in] messageSize              - message size
* @param[in] timeOut                  - time out in miliseconds or
*                                      OS_MSGQ_WAIT_FOREVER or OS_MSGQ_NO_WAIT
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_TIMEOUT               - on time out
*/
#define osMsgQSend CPSS_osMsgQSend
GT_STATUS CPSS_osMsgQSend
(
    IN GT_MSGQ_ID   msgqId,
    IN GT_PTR       message,
    IN GT_U32       messageSize,
    IN GT_U32       timeOut
);

/**
* @internal osMsgQRecv function
* @endinternal
*
* @brief   Receive message from queuee
*
* @param[in] msgqId                   - Message queue Idsemaphore Id
* @param[in,out] messageSize              - size of buffer pointed by message
* @param[in] timeOut                  - time out in miliseconds or
*                                      OS_MSGQ_WAIT_FOREVER or OS_MSGQ_NO_WAIT
*
* @param[out] message                  -  data pointer
* @param[in,out] messageSize              - actual message size
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_TIMEOUT               - on time out
*/
#define osMsgQRecv CPSS_osMsgQRecv
GT_STATUS CPSS_osMsgQRecv
(
    IN    GT_MSGQ_ID   msgqId,
    OUT   GT_PTR       message,
    INOUT GT_U32       *messageSize,
    IN    GT_U32       timeOut
);

/**
* @internal CPSS_osMsgQNumMsgs function
* @endinternal
*
* @brief   Return number of messages pending in queue
*
* @param[in] msgqId                   - Message queue Idsemaphore Id
*
* @param[out] numMessages              - number of messages pending in queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
#define osMsgQNumMsgs CPSS_osMsgQNumMsgs
GT_STATUS CPSS_osMsgQNumMsgs
(
    IN    GT_MSGQ_ID   msgqId,
    OUT   GT_U32       *numMessages
);

#ifdef __cplusplus
}
#endif

#endif  /* __gtOsMsgQh */

