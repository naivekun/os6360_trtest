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
* @file mvHwsPortCtrlSupervisor.c
*
* @brief Port Control Supervisor State Machine
*
* @version   1
********************************************************************************
*/
#include <mvHwsPortCtrlInc.h>
#include <mvHwsPortCtrlDoorbellEvents.h>
#ifdef BV_DEV_SUPPORT
#include <mcdApiTypes.h>
#include <mcdHwSerdesCntl.h>
extern MCD_DEV mcdDevDb;
#endif

#ifdef MICRO_INIT
extern void pollingTaskDisableEnable(GT_U8 disEna);
#endif

static MV_HWS_PORT_CTRL_COMMON *infoPtr;
static GT_U32 spvHighMsgCount = 0;
static GT_U32 spvLowMsgCount  = 0;

#ifdef MV_SERDES_AVAGO_IP
#ifdef BV_DEV_SUPPORT
extern void mcdSerdesAaplAddrGet(MCD_DEV_PTR pDev,unsigned int  *devAddr, unsigned int  *devSize);

extern void mcdSerdesAaplInit( MCD_DEV_PTR  pDev);
#else

extern void mvHwsAvagoAaplAddrGet(unsigned char devNum, unsigned int *devAddr,
                                  unsigned int *devSize);
extern void mvHwsAvagoAaplInit(unsigned char devNum);
#endif
#ifndef MV_HWS_AVAGO_NO_VOS_WA
extern GT_STATUS mvHwsAvagoSerdesManualVosParamsSet(unsigned char devNum,
                                                    unsigned long *vosParamsPtr,
                                                    unsigned char bufferIdx,
                                                    unsigned char bufferLength);
extern GT_STATUS mvHwsAvagoSerdesVosOverrideModeSet(GT_U8 devNum, GT_BOOL vosOverride);
extern GT_STATUS mvHwsAvagoSerdesManualVosMinAndBaseForBc3ParamsSet(
                                                    unsigned char  devNum,
                                                    unsigned long  minVosValueToSet,
                                                    unsigned long  baseVosValueToSet);
void mvHwsAvagoAaplInit(unsigned char devNum);
#endif /* #ifndef MV_HWS_AVAGO_NO_VOS_WA */
#endif /*MV_SERDES_AVAGO_IP*/

#if defined(MV_HWS_FREE_RTOS) && !defined(CM3)
/* This is needed only for service CPU on MSYS */
#include <hw/common.h>
#define IPC_CACHE_INVALIDATE(_ptr,_len) armv7_dcache_wbinv_range((unsigned long)(_ptr), (_len))
#else
#define IPC_CACHE_INVALIDATE(_ptr,_len)
#endif /* MV_HWS_FREE_RTOS */

#if (defined (BOBK_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || defined(ALDRIN_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT))
extern GT_STATUS mvHwsAvagoSerdesTxParametersOffsetSet
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 phyPortNum,
    MV_HWS_SERDES_SPEED serdesSpeed,
    GT_U8  serdesLane,
    GT_U16 offsets,
    GT_U32 *baseAddrPtr,
    GT_U32 *sizePtr
) ;
#endif

#ifdef PIPE_DEV_SUPPORT
//#define LINK_BINDING_PRINT
#define LINK_BINDING_PORTS_NUM 4
unsigned char linkBindingPortNum[LINK_BINDING_PORTS_NUM]                = {0xFF, 0xFF, 0xFF, 0xFF};
unsigned char linkBindingPortPairNum[LINK_BINDING_PORTS_NUM]            = {0xFF, 0xFF, 0xFF, 0xFF};
unsigned char linkBindingPortPairFaultEnabled[LINK_BINDING_PORTS_NUM]   = {0xFF, 0xFF, 0xFF, 0xFF};
unsigned char linkBindingPortLinkStatus[LINK_BINDING_PORTS_NUM]         = {0xFF, 0xFF, 0xFF, 0xFF};
/* temporary path for this API */
/* this API add ports-pair to link binding DB.
 * if link on port1 changed - so the remote fault enabled on port 2
 */
void linkBindingPortsSet(unsigned char port1, unsigned char port2, unsigned char enable);
void linkBindingPortsSet(unsigned char port1, unsigned char port2, unsigned char enable)
{
    unsigned char ii;
    GT_BOOL newPortLinkStatus;
    GT_STATUS rc;

    if (enable == 1)
    {
        /* search for free space in DB */
        for (ii = 0; ii < LINK_BINDING_PORTS_NUM; ii++)
        {
            if (linkBindingPortNum[ii] == 0xFF)
                break;
        }

        if (ii ==  LINK_BINDING_PORTS_NUM)
        {
            #ifdef LINK_BINDING_PRINT
            mvPcPrintf("LinkBinding: no free index\n");
            #endif
            return;
        }

        #ifdef LINK_BINDING_PRINT
        mvPcPrintf("LinkBinding: enable on ports %u %u (index=%u)\n", port1, port2, ii);
        #endif

        /* update DB */
        linkBindingPortNum[ii] = port1;
        linkBindingPortPairNum[ii] = port2;

        /* read the port link status */
        rc = mvHwsPortLinkStatusGet(0, 0, linkBindingPortNum[ii], _10GBase_SR_LR, &newPortLinkStatus);
        if (rc != GT_OK)
        {
            #ifdef LINK_BINDING_PRINT
            mvPcPrintf("LinkBinding: mvHwsPortLinkStatusGet return: rc=%d\n", rc);
            #endif
            return;
        }

        /* update link status DB for this port */
        linkBindingPortLinkStatus[ii] = (unsigned char)newPortLinkStatus;

        /* disable the fault bit on pair port */
        /* TBD instead of this configuration maybe we can get the fault status and update the DB. but no get API... */
        rc = hwsPortFaultSendSet(0, 0, linkBindingPortPairNum[ii], _10GBase_SR_LR, GT_FALSE);
        if (rc != GT_OK)
        {
            #ifdef LINK_BINDING_PRINT
            mvPcPrintf("LinkBinding: hwsPortFaultSendSet return: rc=%d\n", rc);
            #endif
            return;
        }

        /* update the fault status DB for pair port (fault disabled) */
        linkBindingPortPairFaultEnabled[ii] = 0x0;
    }
    else /* disable */
    {
        /* search DB for input ports */
        for (ii = 0; ii < LINK_BINDING_PORTS_NUM; ii++)
        {
            if ((linkBindingPortNum[ii] == port1) && (linkBindingPortPairNum[ii] == port2))
                break; /* ports founded in DB */
        }

        if (ii ==  LINK_BINDING_PORTS_NUM)
        {
            #ifdef LINK_BINDING_PRINT
            mvPcPrintf("LinkBinding: no such ports pair\n");
            #endif
            return;
        }

        #ifdef LINK_BINDING_PRINT
        mvPcPrintf("LinkBinding: disable on ports %u %u (index=%u)\n", port1, port2, ii);
        #endif

        /* reset DB */
        linkBindingPortNum[ii] = 0xFF;
        linkBindingPortPairNum[ii] = 0xFF;
        linkBindingPortLinkStatus[ii] = 0xFF;
        linkBindingPortPairFaultEnabled[ii] = 0xFF;
    }

    return;
}
#endif
/**
* @internal mvPortCtrlSpvRoutine function
* @endinternal
*
* @brief   Supervisor process execution sequence
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void mvPortCtrlSpvRoutine(void* pvParameters)
{
    mvPortCtrlSyncLock();
    mvPortCtrlSyncUnlock();

    infoPtr = &(mvPortCtrlSpv.info);
    for( ;; )
    {
        /*
        ** State Machine Transitions Table
        ** +================+=============+=============+=============+
        ** + Event \ State  + High Msg O1 + Low Msg O2  + Delay O3    +
        ** +================+=============+=============+=============+
        ** + High Msg       +     O1      +     O2      +    ==> O1   +
        ** +================+=============+=============+=============+
        ** + Low Msg        +   ==> O2    +   Invalid   +   Invalid   +
        ** +================+=============+=============+=============+
        ** + Delay          +   Invalid   +   ==> O3    +   Invalid   +
        ** +================+=============+=============+=============+
        */
        if (mvPortCtrlSpv.funcTbl[infoPtr->event][infoPtr->state] != NULL)
        {
            mvPortCtrlSpv.funcTbl[infoPtr->event][infoPtr->state]();
        }
        else
        {
            mvPcPrintf("Error, Port Ctrl, Supervisor Func table: state[%d] event[%d] is NULL\n",
                       infoPtr->state, infoPtr->event);
        }

#ifdef PIPE_DEV_SUPPORT
        /* LinkBindingFw */
        {
            GT_BOOL newPortLinkStatus;
            unsigned char ii, jj;
            GT_STATUS rc;

            /* go over all link binding ports */
            for (ii = 0; ii < LINK_BINDING_PORTS_NUM; ii++)
            {
                /* continue to next port if port not enabled for link binding */
                if (linkBindingPortNum[ii] == 0xFF)
                    continue;

                /* read the port link status */
                rc = mvHwsPortLinkStatusGet(0, 0, linkBindingPortNum[ii], _10GBase_SR_LR, &newPortLinkStatus);
                if (rc != GT_OK)
                {
                    #ifdef LINK_BINDING_PRINT
                    mvPcPrintf("LinkBinding: mvHwsPortLinkStatusGet return: rc=%d\n", rc);
                    #endif
                }

                /* check if link status was changed from previous link status */
                if (newPortLinkStatus != linkBindingPortLinkStatus[ii])
                {
                    if (newPortLinkStatus == 0) /* link changed to down */
                    {
                        /* search if this port not in fault enabled. searh the pairPort DB */
                        for (jj = 0; jj < LINK_BINDING_PORTS_NUM; jj++)
                        {
                            if ((linkBindingPortNum[ii] == linkBindingPortPairNum[jj]) && (linkBindingPortPairFaultEnabled[jj] == 0x1))
                                break; /* the port in fault status */
                        }

                        /* perform algorithm just if port not in fault status because of another pair. */
                        if (jj == LINK_BINDING_PORTS_NUM)
                        {
                            #ifdef LINK_BINDING_PRINT
                            mvPcPrintf("LB: port %u was down. enable fault ort %u (index=%u)\n",
                                   linkBindingPortNum[ii], linkBindingPortPairNum[ii], ii);
                            #endif

                            /* update new link status on the port */
                            linkBindingPortLinkStatus[ii] = (unsigned char)newPortLinkStatus;

                            /* enable the fault bit on pair port */
                            rc = hwsPortFaultSendSet(0, 0, linkBindingPortPairNum[ii], _10GBase_SR_LR, GT_TRUE);
                            if (rc != GT_OK)
                            {
                                #ifdef LINK_BINDING_PRINT
                                mvPcPrintf("LinkBinding: hwsPortFaultSendSet return: rc=%d\n", rc);
                                #endif
                            }

                            /* update the fault status DB for pair port (fault enabled) */
                            linkBindingPortPairFaultEnabled[ii] = 0x1;
                        }
                    }
                    else /* link changed to up */
                    {
                        #ifdef LINK_BINDING_PRINT
                        mvPcPrintf("LB: port %u was up. stop fault on port %u. (index=%u)\n",
                               linkBindingPortNum[ii], linkBindingPortPairNum[ii], ii);
                        #endif

                        /* update new link status on the port */
                        linkBindingPortLinkStatus[ii] = (unsigned char)newPortLinkStatus;

                        /* stop the fault bit on pair port */
                        rc = hwsPortFaultSendSet(0, 0, linkBindingPortPairNum[ii], _10GBase_SR_LR, GT_FALSE);
                        if (rc != GT_OK)
                        {
                            #ifdef LINK_BINDING_PRINT
                            mvPcPrintf("LinkBinding: hwsPortFaultSendSet return: rc=%d\n", rc);
                            #endif
                        }

                        /* update the fault status DB for pair port (fault disabled) */
                        linkBindingPortPairFaultEnabled[ii] = 0x0;
                    }
                }
            } /* for */
        } /* LinkBindingFw */
#endif
    }
}

/**
* @internal mvPortCtrlSpvHighMsgExecute function
* @endinternal
*
* @brief   Supervisor High priority message process state processing
*         - Handle "Pizza Arbbiter" Configuration
*         Read message from high priority queue
*         Message received
*         Update database
*         Add Log entry
*/
void mvPortCtrlSpvHighMsgExecute(MV_HWS_IPC_CTRL_MSG_STRUCT *recvMsg)
{
    MV_HWS_IPC_PORT_INFO_STRUCT *msgParams;
    MV_HWS_PORT_CTRL_AP_PORT_TIMER *portTimer;
    GT_U8 portNumBase = 0;
    GT_U8 portIndex;

#ifdef BC2_DEV_SUPPORT
    if(BobcatA0 == hwsDeviceSpecInfo[0].devType)
    {
        portNumBase = MV_PORT_CTRL_BC2_AP_PORT_NUM_BASE;
    }
    else if(Alleycat3A0 == hwsDeviceSpecInfo[0].devType)
    {
        portNumBase = MV_PORT_CTRL_AC3_AP_PORT_NUM_BASE;
    }
    else
    {
        portNumBase = 0;
    }
#else
    portNumBase = MV_PORT_CTRL_AP_PORT_NUM_BASE;
#endif

    if (recvMsg->ctrlMsgType == MV_HWS_IPC_PORT_AP_SYS_CFG_VALID_MSG)
    {
        /* Extract target port from IPC message and update system config valid status */
        /* System configuration use the format of Port Init message */

        msgParams = &(recvMsg->msgData.apSysConfig);
        portIndex = MV_HWS_PORT_CTRL_AP_LOGICAL_PORT_GET(msgParams->phyPortNum, portNumBase);
        portTimer = &(mvHwsPortCtrlApPortMng.apPortTimer[portIndex]);

        portTimer->sysCfStatus = PORT_SM_SERDES_SYSTEM_VALID;
        portTimer->sysCfMode = msgParams->portMode;
        mvPortCtrlLogAdd(SPV_LOG(SPV_HIGH_MSG_PROCESS_STATE, recvMsg->ctrlMsgType,
                                 msgParams->phyPortNum, AP_PORT));
        /* TODO - Add handling of system state DISABLE / ENABLE */

        spvHighMsgCount++;
        if (spvHighMsgCount >= mvPortCtrlSpv.highMsgThreshold)
        {
            spvHighMsgCount = 0;
            infoPtr->event = SPV_LOW_MSG_EVENT;
        }
    }
}

/**
* @internal mvPortCtrlSpvHighMsg function
* @endinternal
*
* @brief   Supervisor High priority message process state execution
*/
void mvPortCtrlSpvHighMsg(void)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT recvMsg;

    infoPtr->state = SPV_HIGH_MSG_PROCESS_STATE;
    osMemSet(&recvMsg, 0, sizeof(recvMsg));
    /* Process messages from High priority IPC message queue */
    if (mvHwsIpcRequestGet(MV_HWS_IPC_HIGH_PRI_QUEUE,
                           sizeof(MV_HWS_IPC_CTRL_MSG_STRUCT),
                           (char*)&recvMsg) != GT_OK)
    {
        spvHighMsgCount = 0;
        infoPtr->event = SPV_LOW_MSG_EVENT;
    }
    else
    {
         mvPortCtrlSpvHighMsgExecute(&recvMsg);
    }
}

/**
* @internal mvPortCtrlSpvLowMsgExecute function
* @endinternal
*
* @brief   Supervisor Low priority message process state processing
*         - Validate system process can receive messgae
*         In case Valid
*         Handle Port Init / Reset IPC Message
*         Add log entry
*         Send message to Port process
*         Handle AP Port IPC Message
*         Add log entry
*         Send message to AP Detection process
*         Handle General IPC Message
*         Add log entry
*         Send message to General process
*         Message not received
*         Trigger state change
*         - Not Valid
*         Trigger state change
*/
void mvPortCtrlSpvLowMsgExecute(MV_HWS_IPC_CTRL_MSG_STRUCT *recvMsg)
{
    MV_HWS_IPC_PORT_INIT_DATA_STRUCT *msgParams;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyMsg;
    unsigned int avagoAaplAddr;
    unsigned int avagoAaplSize;
    GT_U32 avagoGuiState;

    msgParams = (MV_HWS_IPC_PORT_INIT_DATA_STRUCT*)&(recvMsg->msgData);
    switch (recvMsg->ctrlMsgType)
    {
    case MV_HWS_IPC_HWS_LOG_GET_MSG:
        mvHwsLogInfoGet(&replyMsg.readData.logGet.fwBaseAddr,
                &replyMsg.readData.logGet.logBaseAddr,
                &replyMsg.readData.logGet.logPointerAddr,
                &replyMsg.readData.logGet.logCountAddr,
                &replyMsg.readData.logGet.logResetAddr);

        replyMsg.returnCode = GT_OK;
        replyMsg.replyTo = recvMsg->ctrlMsgType;
        mvHwsIpcReplyMsg(recvMsg->msgQueueId, &replyMsg);
        break;

    case MV_HWS_IPC_VOS_OVERRIDE_PARAMS_DATA_MSG:
#ifndef MV_HWS_AVAGO_NO_VOS_WA
        if ( !(recvMsg->msgData.vosOverrideParams.vosOverrideParamsBufferIdx == 0xFFFFFFFF &&
            recvMsg->msgData.vosOverrideParams.vosOverrideParamsBufferLength == 2) )
        {
            replyMsg.returnCode = mvHwsAvagoSerdesManualVosParamsSet(0,
                                &(recvMsg->msgData.vosOverrideParams.vosOverrideParamsBufferPtr[0]),
                                recvMsg->msgData.vosOverrideParams.vosOverrideParamsBufferIdx,
                                recvMsg->msgData.vosOverrideParams.vosOverrideParamsBufferLength);
        }
        else if (recvMsg->msgData.vosOverrideParams.vosOverrideParamsBufferIdx == 0xFFFFFFFF &&
            recvMsg->msgData.vosOverrideParams.vosOverrideParamsBufferLength == 2)
        {
            replyMsg.returnCode = mvHwsAvagoSerdesManualVosMinAndBaseForBc3ParamsSet(0,
                                recvMsg->msgData.vosOverrideParams.vosOverrideParamsBufferPtr[0],
                                recvMsg->msgData.vosOverrideParams.vosOverrideParamsBufferPtr[1]);
        }
#else
        replyMsg.returnCode = GT_OK;
#endif
        replyMsg.replyTo = recvMsg->ctrlMsgType;
        mvHwsIpcReplyMsg(recvMsg->msgQueueId, &replyMsg);
        break;
    case MV_HWS_IPC_VOS_OVERRIDE_MODE_SET_MSG:
#ifndef MV_HWS_AVAGO_NO_VOS_WA
        replyMsg.returnCode = mvHwsAvagoSerdesVosOverrideModeSet(0,
                                recvMsg->msgData.vosOverrideModeSet.vosOverrideMode);
#else
        replyMsg.returnCode = GT_OK;
#endif
        replyMsg.replyTo = recvMsg->ctrlMsgType;
        mvHwsIpcReplyMsg(recvMsg->msgQueueId, &replyMsg);
        break;
    case MV_HWS_IPC_PORT_AVAGO_SERDES_INIT_MSG:

#ifdef BV_DEV_SUPPORT
        mcdSerdesAaplAddrGet(&mcdDevDb, &avagoAaplAddr, &avagoAaplSize);
#elif MV_SERDES_AVAGO_IP
        mvHwsAvagoAaplAddrGet(0, &avagoAaplAddr, &avagoAaplSize);
#else
        avagoAaplAddr = 0;
        avagoAaplSize = 0;
#endif /* MV_SERDES_AVAGO_IP */
        replyMsg.replyTo    = recvMsg->ctrlMsgType;
        replyMsg.returnCode = GT_OK;
        replyMsg.readData.portAvagoAaplGet.addr = avagoAaplAddr;
        replyMsg.readData.portAvagoAaplGet.size = avagoAaplSize;
        /* Host is about to deliver the full content of AAPL structure.
         * Need to invalidate this area to be able to see content.*/
        IPC_CACHE_INVALIDATE(avagoAaplAddr, avagoAaplSize);
        mvHwsIpcReplyMsg(recvMsg->msgQueueId, &replyMsg);
        break;
    case MV_HWS_IPC_PORT_AVAGO_SERDES_RESET_DATA_MSG:
#ifdef MV_SERDES_AVAGO_IP
#ifndef BV_DEV_SUPPORT
        mvHwsAvagoAaplInit(0);
#else
        mcdSerdesAaplInit(&mcdDevDb);
#endif
#endif
        replyMsg.replyTo    = recvMsg->ctrlMsgType;
        replyMsg.returnCode = GT_OK;
        mvHwsIpcReplyMsg(recvMsg->msgQueueId, &replyMsg);
        break;
    case MV_HWS_IPC_PORT_AVAGO_GUI_SET_MSG:
        avagoGuiState = recvMsg->msgData.avagoGuiSet.state;
        avagoGuiState &= MV_PORT_CTRL_AVAGO_GUI_MASK;
        mvHwsApDetectStateSet(avagoGuiState);
        replyMsg.replyTo    = recvMsg->ctrlMsgType;
        replyMsg.returnCode = GT_OK;
        mvHwsIpcReplyMsg(recvMsg->msgQueueId, &replyMsg);
        break;

    /* Send messages to Port Mng process message queue */
#ifdef REG_PORT_TASK
    case MV_HWS_IPC_PORT_INIT_MSG:
    case MV_HWS_IPC_PORT_RESET_MSG:
    case MV_HWS_IPC_PORT_RESET_EXT_MSG:
        mvPortCtrlPortSm[msgParams->phyPortNum].type = REG_PORT;
        mvPortCtrlLogAdd(SPV_LOG(SPV_LOW_MSG_PROCESS_STATE, recvMsg->ctrlMsgType,
                                 msgParams->phyPortNum, REG_PORT));
        mvPortCtrlProcessMsgSend(M2_PORT_MNG, recvMsg);
        break;
#endif
    /* Send messages to AP Detection process message queue */
    case MV_HWS_IPC_PORT_AP_ENABLE_MSG:
    case MV_HWS_IPC_PORT_AP_DISABLE_MSG:
    case MV_HWS_IPC_PORT_AP_CFG_GET_MSG:
    case MV_HWS_IPC_PORT_AP_STATUS_MSG:
    case MV_HWS_IPC_PORT_AP_STATS_MSG:
    case MV_HWS_IPC_PORT_AP_STATS_RESET_MSG:
    case MV_HWS_IPC_PORT_AP_INTROP_GET_MSG:
    case MV_HWS_IPC_PORT_AP_INTROP_SET_MSG:
    case MV_HWS_IPC_PORT_AP_DEBUG_GET_MSG:
    case MV_HWS_IPC_PORT_ENABLE_CTRL_SET_MSG:
    case MV_HWS_IPC_PORT_ENABLE_CTRL_GET_MSG:

#if (defined (BOBK_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || defined(ALDRIN_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT))
    case MV_HWS_IPC_PORT_SERDES_RX_CONFIG_OVERRIDE_MSG:
#endif /* (defined (BOBK_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || defined(ALDRIN_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)) */
#ifdef REG_PORT_TASK
        mvPortCtrlPortSm[msgParams->phyPortNum].type = AP_PORT;
#endif
        mvPortCtrlLogAdd(SPV_LOG(SPV_LOW_MSG_PROCESS_STATE, recvMsg->ctrlMsgType,
                                 msgParams->phyPortNum, AP_PORT));
        mvPortCtrlProcessMsgSend(M5_AP_PORT_DET, recvMsg);
        break;

    case MV_HWS_IPC_NOOP_MSG:
        {
#ifdef MICRO_INIT
            /**
             * This will stop polling task upon cpssInitSystem fastBoot.
             */
            pollingTaskDisableEnable(0);
#endif
#if 1
/* AP to CPSS Doorbell events test */
            if (recvMsg->msgData.noop.data == 8)
                recvMsg->msgData.noop.data = apDoorbellIntrAddEvent(AP_DOORBELL_EVENT_IPC_E);
            if (recvMsg->msgData.noop.data == 10)
                recvMsg->msgData.noop.data = apDoorbellIntrAddEvent(AP_DOORBELL_EVENT_PORT_STATUS_CHANGE(30));
#endif
            replyMsg.readData.noopReply.data = recvMsg->msgData.noop.data+2;
            replyMsg.returnCode = GT_OK;
            replyMsg.replyTo = recvMsg->ctrlMsgType;
            mvHwsIpcReplyMsg(recvMsg->msgQueueId, &replyMsg);
            break;
        }

#if (defined (BOBK_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || defined(ALDRIN_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT))
    case MV_HWS_IPC_PORT_SERDES_RX_PARAMETERS_OFFSET_CONFIG_MSG:
        mvPortCtrlLogAdd(SPV_LOG(SPV_LOW_MSG_PROCESS_STATE, recvMsg->ctrlMsgType,
                                 msgParams->phyPortNum, AP_PORT));
        replyMsg.returnCode = mvHwsAvagoSerdesTxParametersOffsetSet(recvMsg->devNum,
                  recvMsg->msgData.portSerdesTxParametersOffsetConfig.portGroup,
                  recvMsg->msgData.portSerdesTxParametersOffsetConfig.phyPortNum,
                  recvMsg->msgData.portSerdesTxParametersOffsetConfig.serdesSpeed,
                  recvMsg->msgData.portSerdesTxParametersOffsetConfig.serdesLane,
                  recvMsg->msgData.portSerdesTxParametersOffsetConfig.offsets,
                 &replyMsg.readData.portSerdesTxParametersOffsetInfoCfg.txOffsetBaseAddr,
                 &replyMsg.readData.portSerdesTxParametersOffsetInfoCfg.txOffsetSize);
        replyMsg.replyTo = recvMsg->ctrlMsgType;
        mvHwsIpcReplyMsg(recvMsg->msgQueueId, &replyMsg);
        break;
#endif /* (defined (BOBK_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || defined(ALDRIN_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)) */

    default:
        replyMsg.returnCode = GT_NOT_SUPPORTED;
        replyMsg.replyTo = recvMsg->ctrlMsgType;
        mvHwsIpcReplyMsg(recvMsg->msgQueueId, &replyMsg);
        break;
    }
    spvLowMsgCount++;
}

/**
* @internal mvPortCtrlSpvLowMsg function
* @endinternal
*
* @brief   Supervisor Low priority message process state execution
*/
void mvPortCtrlSpvLowMsg(void)
{
    GT_STATUS                  rcode;
#ifdef REG_PORT_TASK
    GT_U32                     portMsgNum;
#endif
    GT_U32                     apMsgNum;
#ifdef AP_GENERAL_TASK
    GT_U32                     genMsgNum;
#endif
    MV_HWS_IPC_CTRL_MSG_STRUCT recvMsg;

    infoPtr->state = SPV_LOW_MSG_PROCESS_STATE;

    /* Validate system process can receive messgaes
    ** Threshold is set to max of 6 message at each processing interval
    ** under the validation that the target queue has enough space
    */
    rcode = mvPortCtrlProcessPendMsgNum(M5_AP_PORT_DET, &apMsgNum);
#ifdef REG_PORT_TASK
    rcode |= mvPortCtrlProcessPendMsgNum(M2_PORT_MNG, &portMsgNum);
#endif
#ifdef AP_GENERAL_TASK
    rcode |= mvPortCtrlProcessPendMsgNum(M6_GENERAL, &genMsgNum);
#endif
    if (rcode != GT_OK)
    {
        mvPcPrintf("Error, Supervisor, Failed to get number of pending messages\n");
        return;
    }
    if ((apMsgNum       < MV_PORT_CTRL_SPV_MSG_EXEC_THRESHOLD) &&
#ifdef AP_GENERAL_TASK
        (genMsgNum      < MV_PORT_CTRL_SPV_MSG_EXEC_THRESHOLD) &&
#endif
        (spvLowMsgCount < mvPortCtrlSpv.lowMsgThreshold))
    {
        /* Process messages from Low priority IPC message queue */
        if (mvHwsIpcRequestGet(MV_HWS_IPC_LOW_PRI_QUEUE,
                               sizeof(MV_HWS_IPC_CTRL_MSG_STRUCT),
                               (char*)&recvMsg) != GT_OK)
        {
            spvLowMsgCount = 0;
            infoPtr->event = SPV_DELAY_EVENT;
        }
        else
        {
           mvPortCtrlSpvLowMsgExecute(&recvMsg);
        }
    }
    else
    {
        spvLowMsgCount = 0;
        infoPtr->event = SPV_DELAY_EVENT;
    }
}

/**
* @internal mvPortCtrlSpvDelay function
* @endinternal
*
* @brief   Superviosr message delay state execution
*         - Execute delay
*         - Trigger state change
*/
void mvPortCtrlSpvDelay(void)
{
    infoPtr->state = SPV_DELAY_STATE;
    mvPortCtrlProcessDelay(infoPtr->delayDuration);
    infoPtr->event = SPV_HIGH_MSG_EVENT;
}


