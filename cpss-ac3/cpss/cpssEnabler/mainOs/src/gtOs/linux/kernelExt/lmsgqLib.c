/*******************************************************************************
* osLinuxMsgQ.c
*
* DESCRIPTION:
*       Linux User Mode Operating System wrapper. Message queues
*
* DEPENDENCIES:
*       Linux, CPU independed , and posix threads implementation.
*
* FILE REVISION NUMBER:
*       $Revision: 5 $
*******************************************************************************/

#define _GNU_SOURCE
#include "kernelExt.h"
#include <gtOs/gtOsMsgQ.h>
#include <gtOs/gtOsIo.h>


#include <string.h>
#include <stdlib.h>
#include <errno.h>

/************ Public Functions ************************************************/

/**
* @internal osMsgQCreate function
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
GT_STATUS osMsgQCreate
(
    IN  const char    *name,
    IN  GT_U32        maxMsgs,
    IN  GT_U32        maxMsgSize,
    OUT GT_MSGQ_ID    *msgqId
)
{
    int ret;
    mv_msgq_create_stc rparam;

    if (name)
        strncpy(rparam.name, name, sizeof(rparam.name));
    else
        rparam.name[0] = 0;
    rparam.maxMsgs = maxMsgs;
    rparam.maxMsgSize = maxMsgSize;

    ret = mv_ctrl(MVKERNELEXT_IOC_MSGQCREATE, &rparam);
    if (ret < 0)
    {
        *msgqId = 0;
        return GT_FAIL;
    }

    *msgqId = (GT_MSGQ_ID)ret;

    return GT_OK;
}

/**
* @internal osMsgQDelete function
* @endinternal
*
* @brief   Delete message queue
*
* @param[in] msgqId                   - message queue Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osMsgQDelete
(
    IN GT_MSGQ_ID msgqId
)
{
    int rc;

    rc = mv_ctrl(MVKERNELEXT_IOC_MSGQDELETE, msgqId);

    if (rc != 0)
        return GT_FAIL;

    return GT_OK;
}

/**
* @internal osMsgQSend function
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
GT_STATUS osMsgQSend
(
    IN GT_MSGQ_ID   msgqId,
    IN GT_PTR       message,
    IN GT_U32       messageSize,
    IN GT_U32       timeOut
)
{
    int ret;
    mv_msgq_sr_stc params;
    
    params.msgqId = msgqId;
    params.message = message;
    params.messageSize = messageSize;
    if (timeOut == OS_MSGQ_NO_WAIT)
    {
        params.timeOut = 0;
    }
    else if (timeOut == OS_MSGQ_WAIT_FOREVER)
    {
        params.timeOut = -1;
    }
    else
    {
        params.timeOut = timeOut;
    }

    do {
        ret = mv_ctrl(MVKERNELEXT_IOC_MSGQSEND, &params);
    } while (ret < 0 && errno == MVKERNELEXT_EINTR);

    if (ret < 0 && errno == MVKERNELEXT_ETIMEOUT)
        return GT_TIMEOUT;

    if (ret < 0)
        return GT_FAIL;
    return GT_OK;
}

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
GT_STATUS osMsgQRecv
(
    IN    GT_MSGQ_ID   msgqId,
    OUT   GT_PTR       message,
    INOUT GT_U32       *messageSize,
    IN    GT_U32       timeOut
)
{
    int ret;
    mv_msgq_sr_stc params;
    
    params.msgqId = msgqId;
    params.message = message;
    params.messageSize = *messageSize;
    if (timeOut == OS_MSGQ_NO_WAIT)
    {
        params.timeOut = 0;
    }
    else if (timeOut == OS_MSGQ_WAIT_FOREVER)
    {
        params.timeOut = -1;
    }
    else
    {
        params.timeOut = timeOut;
    }

    do {
        ret = mv_ctrl(MVKERNELEXT_IOC_MSGQRECV, &params);
    } while (ret < 0 && errno == MVKERNELEXT_EINTR);

    if (ret < 0 && errno == MVKERNELEXT_ETIMEOUT)
        return GT_TIMEOUT;

    if (ret < 0)
        return GT_FAIL;

    *messageSize = (GT_U32)ret;
    return GT_OK;
}

/**
* @internal osMsgQNumMsgs function
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
GT_STATUS osMsgQNumMsgs
(
    IN    GT_MSGQ_ID   msgqId,
    OUT   GT_U32       *numMessages
)
{
    int ret;

    ret = mv_ctrl(MVKERNELEXT_IOC_MSGQNUMMSGS, msgqId);

    if (ret < 0)
        return GT_FAIL;

    *numMessages = (GT_U32)ret;
    return GT_OK;
}

