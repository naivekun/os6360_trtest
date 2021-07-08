/**
********************************************************************************
* @file osVxMsgQ.c
*
* @brief vxWorks Operating System wrapper. Message queues
*
* @version   1
********************************************************************************
*/
/*******************************************************************************
* osVxMsgQ.c
*
* DESCRIPTION:
*       vxWorks Operating System wrapper. Message queues
*
* DEPENDENCIES:
*       VxWorks, CPU independed.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/

#include <vxWorks.h>
#include <objLib.h>
#include <msgQLib.h>
#include <sysLib.h>
#include <gtOs/gtOsMsgQ.h>

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
    MSG_Q_ID    id;

    id = msgQCreate(maxMsgs, maxMsgSize, MSG_Q_FIFO);

    if (!id)
        return GT_FAIL;

    if (msgqId)
        *msgqId = (GT_MSGQ_ID)id;

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
    STATUS rc;

    rc = msgQDelete((MSG_Q_ID)msgqId);

    return (rc == OK) ? GT_OK : GT_FAIL;
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
    int     tmo;
    STATUS  rc;

    if (timeOut == OS_MSGQ_NO_WAIT)
        tmo = NO_WAIT;
    else if (timeOut == OS_MSGQ_WAIT_FOREVER)
        tmo = WAIT_FOREVER;
    else
    {
        int num;

        num = sysClkRateGet();
        tmo = (num * timeOut) / 1000;
        if (tmo < 1)
            tmo = 1;
    }
    rc = msgQSend((MSG_Q_ID)msgqId,
            (char*)message, (UINT)messageSize,
            tmo, MSG_PRI_NORMAL);

    if (rc == OK)
        return GT_OK;

    if (errno == S_objLib_OBJ_TIMEOUT)
        return GT_TIMEOUT;
    return GT_FAIL;
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
    int     rc;
    int     tmo;

    if (timeOut == OS_MSGQ_NO_WAIT)
        tmo = NO_WAIT;
    else if (timeOut == OS_MSGQ_WAIT_FOREVER)
        tmo = WAIT_FOREVER;
    else
    {
        int num;

        num = sysClkRateGet();
        tmo = (num * timeOut) / 1000;
        if (tmo < 1)
            tmo = 1;
    }

    rc = msgQReceive((MSG_Q_ID)msgqId,
            (char*)message, (UINT)*messageSize,
            tmo);

    if (rc != ERROR)
    {
        *messageSize = (GT_U32)rc;
        return GT_OK;
    }

    if (errno == S_objLib_OBJ_TIMEOUT)
        return GT_TIMEOUT;
    return GT_FAIL;
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
    int rc;

    rc = msgQNumMsgs((MSG_Q_ID)msgqId);

    if (rc != ERROR)
    {
        if (numMessages)
            *numMessages = (GT_U32)rc;
        return GT_OK;
    }

    return GT_FAIL;
}

