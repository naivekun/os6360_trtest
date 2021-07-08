/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file mvHwsPortCtrlPort.c
*
* @brief Port Control Port State Machine
*
* @version   1
********************************************************************************
*/
#ifdef REG_PORT_TASK
#include <mvHwsPortCtrlInc.h>

static MV_HWS_PORT_CTRL_COMMON *infoPtr;

/**
* @internal mvPortCtrlPortRoutine function
* @endinternal
*
* @brief   Port mng process execution sequence
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void mvPortCtrlPortRoutine(void* pvParameters)
{
    mvPortCtrlSyncLock();
    mvPortCtrlSyncUnlock();

    infoPtr = &(mvPortCtrlPortMng.info);

    for( ;; )
    {
        /*
        ** State Machine Transitions Table
        ** +================+=============+=============+
        ** + Event \ State  +   Msg O1    +   Delay O2  +
        ** +================+=============+=============+
        ** + Msg            +     O1      +    ==> O1   +
        ** +================+=============+=============+
        ** + Delay          +   ==> O2    +   Invalid   +
        ** +================+=============+=============+
        */
        if (mvPortCtrlPortMng.funcTbl[infoPtr->event][infoPtr->state] != NULL)
        {
            mvPortCtrlPortMng.funcTbl[infoPtr->event][infoPtr->state]();
        }
        else
        {
            mvPcPrintf("Error, Port Ctrl, Port Func table: state[%d] event[%d] is NULL\n",
                       infoPtr->state, infoPtr->event);
        }
    }
}

/**
* @internal mvPortCtrlPortResetExec function
* @endinternal
*
* @brief   Port mng message processing - reset execution
*         - Execute port reset and return to Idle state
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvPortCtrlPortResetExec(GT_U32 port,
                                  MV_HWS_PORT_CTRL_PORT_SM *info,
                                  MV_HWS_IPC_CTRL_MSG_STRUCT *msg)
{
    MV_HWS_IPC_PORT_RESET_DATA_STRUCT *msgParams;

    msgParams = &(msg->msgData.portReset);

    info->status = PORT_SM_DELETE_IN_PROGRESS;
    mvPortCtrlLogAdd(PORT_SM_LOG(info->state, info->status, port));

    /* Execute HWS Port Reset */
    if (mvHwsPortReset(msg->devNum,
                       msgParams->portGroup,
                       msgParams->phyPortNum,
                       msgParams->portMode,
                       msgParams->action) != GT_OK)
    {
        info->status = PORT_SM_DELETE_FAILURE;
        mvPortCtrlLogAdd(PORT_SM_LOG(info->state, info->status, port));

        return GT_FAIL;
    }

    info->state = PORT_SM_IDLE_STATE;
    info->status = PORT_SM_DELETE_SUCCESS;
    mvPortCtrlLogAdd(PORT_SM_LOG(info->state, info->status, port));

    return GT_OK;
}

/**
* @internal mvPortCtrlPortMsg function
* @endinternal
*
* @brief   Port mng message processing state execution
*         - Read message from port message queue queue
*         Message received
*         Add Log entry
*         Execute message to completion
*         Send reply message
*         Message not received
*         Trigger state change
*/
void mvPortCtrlPortMsg(void)
{
    GT_STATUS                        rcode;
    MV_HWS_IPC_CTRL_MSG_STRUCT       recvMsg;
    GT_U16                           msgPhyPortNum;

    infoPtr->state = PORT_MNG_MSG_PROCESS_STATE;
    osMemSet(&recvMsg, 0, sizeof(MV_HWS_IPC_CTRL_MSG_STRUCT));
    do
    {
        rcode = mvPortCtrlProcessMsgRecv(M2_PORT_MNG, MV_PROCESS_MSG_RX_NO_DELAY, &recvMsg);
        if (rcode == GT_OK)
        {
            msgPhyPortNum = recvMsg.msgData.portGeneral.phyPortNum;

            mvPortCtrlLogAdd(PORT_MNG_LOG(PORT_MNG_MSG_PROCESS_STATE, msgPhyPortNum, recvMsg.ctrlMsgType));

            rcode = mvPortCtrlPortMsgExec(msgPhyPortNum, &recvMsg);

            mvPortCtrlPortMsgReply(rcode, recvMsg.msgQueueId, recvMsg.ctrlMsgType);
        }
    } while (rcode == GT_OK);

    infoPtr->event = PORT_MNG_DELAY_EVENT;
}

/**
* @internal mvPortCtrlPortMsgExec function
* @endinternal
*
* @brief   Port mng message processing state execution
*         - Idle State
*         Init message - Execute port init from Idle state to Active state
*         Reset message - Execute port reset and return to Idle state
*         - Active State
*         Init message - Return Error
*         Reset message - Execuet port reset and return to Idle state
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvPortCtrlPortMsgExec(GT_U32 port,
                                MV_HWS_IPC_CTRL_MSG_STRUCT *msg)
{

    MV_HWS_IPC_PORT_INIT_DATA_STRUCT *msgParams;
    MV_HWS_PORT_CTRL_PORT_SM         *portCtrlSmPtr = &(mvPortCtrlPortSm[port]);
    GT_U8                            msgType = msg->ctrlMsgType;
    MV_HWS_PORT_INIT_INPUT_PARAMS   portInitInParam;

    switch (portCtrlSmPtr->state)
    {
        case PORT_SM_IDLE_STATE:
            if (msgType == MV_HWS_IPC_PORT_INIT_MSG)
            {
                portCtrlSmPtr->status = PORT_SM_START_EXECUTE;
                mvPortCtrlLogAdd(PORT_SM_LOG(portCtrlSmPtr->state, portCtrlSmPtr->status, port));
                portCtrlSmPtr->state  = PORT_SM_SERDES_CONFIG_STATE;
                portCtrlSmPtr->status = PORT_SM_SERDES_CONFIG_IN_PROGRESS;
                mvPortCtrlLogAdd(PORT_SM_LOG(portCtrlSmPtr->state, portCtrlSmPtr->status, port));
                /**PLACE Holder: HWS config SERDES*/
                if (0)
                {
                    portCtrlSmPtr->status = PORT_SM_MAC_PCS_CONFIG_FAILURE;
                    mvPortCtrlLogAdd(PORT_SM_LOG(portCtrlSmPtr->state, portCtrlSmPtr->status, port));
                    /* Move to Idle state to enable SM execution for future messages */
                    portCtrlSmPtr->state  = PORT_SM_IDLE_STATE;
                    return GT_FAIL;
                }
                portCtrlSmPtr->status = PORT_SM_SERDES_CONFIG_SUCCESS;
                portCtrlSmPtr->state  = PORT_SM_MAC_PCS_CONFIG_STATE;
                mvPortCtrlLogAdd(PORT_SM_LOG(portCtrlSmPtr->state, portCtrlSmPtr->status, port));
                portCtrlSmPtr->status = PORT_SM_MAC_PCS_CONFIG_IN_PROGRESS;
                mvPortCtrlLogAdd(PORT_SM_LOG(portCtrlSmPtr->state, portCtrlSmPtr->status, port));
                /**PLACE Holder: HWS config MAC / PCS*/
                msgParams = &(msg->msgData.portInit);
                osMemSet(&portInitInParam, 0,sizeof(portInitInParam));
                portInitInParam.lbPort = msgParams->lbPort;
                portInitInParam.refClock = msgParams->refClock;
                portInitInParam.refClockSource = msgParams->refClockSource;

                /* Execute HWS Port Init */
                if (mvHwsPortInit(msg->devNum,
                                  msgParams->portGroup,
                                  msgParams->phyPortNum,
                                  msgParams->portMode,
                                  &portInitInParam) != GT_OK)
                {
                    mvPcPrintf("Error, Port Mng, Port-%d Init Failed\n", port);

                    portCtrlSmPtr->status = PORT_SM_SERDES_CONFIG_FAILURE;
                    mvPortCtrlLogAdd(PORT_SM_LOG(portCtrlSmPtr->state, portCtrlSmPtr->status, port));
                    /* Move to Idle state to enable SM execution for future messages */
                    portCtrlSmPtr->state  = PORT_SM_IDLE_STATE;
                    return GT_FAIL;
                }
                portCtrlSmPtr->status = PORT_SM_MAC_PCS_CONFIG_SUCCESS;
                portCtrlSmPtr->state  = PORT_SM_ACTIVE_STATE;
                mvPortCtrlLogAdd(PORT_SM_LOG(portCtrlSmPtr->state, portCtrlSmPtr->status, port));
            }
            else if ((msgType == MV_HWS_IPC_PORT_RESET_MSG) ||
                     (msgType == MV_HWS_IPC_PORT_RESET_EXT_MSG))
            {
                if (mvPortCtrlPortResetExec(port, portCtrlSmPtr, msg) != GT_OK)
                {
                    return GT_FAIL;
                }
            }
            break;

        case PORT_SM_ACTIVE_STATE:
            if ((msgType == MV_HWS_IPC_PORT_RESET_MSG) ||
                (msgType == MV_HWS_IPC_PORT_RESET_EXT_MSG))
            {
                if (mvPortCtrlPortResetExec(port, portCtrlSmPtr, msg) != GT_OK)
                {
                    return GT_FAIL;
                }
            }
            else if (msgType == MV_HWS_IPC_PORT_INIT_MSG)
            {
                mvPcPrintf("Error, Port Mng, Port-%d Init in Active state\n", port);
            }
            break;
    }
    return GT_OK;
}

/**
* @internal mvPortCtrlPortMsgReply function
* @endinternal
*
* @brief   Port mng message reply
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvPortCtrlPortMsgReply(GT_U32 rcode, GT_U8 queueId, GT_U8 msgType)
{
    MV_HWS_IPC_REPLY_MSG_STRUCT reply;

    reply.replyTo    = msgType;
    reply.returnCode = rcode;

    /* Build and Send IPC reply */
    CHECK_STATUS(mvHwsIpcReplyMsg(queueId, &reply));

    return GT_OK;
}

/**
* @internal mvPortCtrlPortDelay function
* @endinternal
*
* @brief   Port mng message delay state execution
*         - Execute delay
*         - Trigger state change
*/
void mvPortCtrlPortDelay(void)
{
    infoPtr->state = PORT_MNG_DELAY_STATE;
    mvPortCtrlProcessDelay(infoPtr->delayDuration);
    infoPtr->event = PORT_MNG_MSG_PROCESS_EVENT;
}
#endif
