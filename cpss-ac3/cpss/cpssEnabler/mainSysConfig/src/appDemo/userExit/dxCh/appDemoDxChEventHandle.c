/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file appDemoDxChEventHandle.c
*
* @brief this library contains the implementation of the event handling functions
* for the DXCH device , and the redirection to the GalTis Agent for
* extra actions .
*
*
* ---> this file is compiled only under CHX_FAMILY <---
*
*
* @version   16
********************************************************************************
*/

#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfMii.h>
#include <cpss/dxCh/dxChxGen/trunk/cpssDxChTrunk.h>
#include <appDemo/userExit/userEventHandler.h>
#include <appDemo/userExit/dxCh/appDemoDxChEventHandle.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfigDefaults.h>
#include <cpss/generic/cpssHwInit/private/prvCpssHwInit.h>
/****************************************************************************
* Debug                                                                     *
****************************************************************************/

#define BUFF_LEN    20

#define MAX_REGISTER    5
static GT_U32                   numRegistrations=0;
static RX_PACKET_RECEIVE_CB_FUN rxPktReceiveCbArray[MAX_REGISTER];
extern GT_UINTPTR rxEventHanderHnd;

/* workaround */
static GT_STATUS prvAppDemoDxChNetRxPacketRegisteredCb
(
  IN  GT_UINTPTR                cookie,
  IN  RX_EV_HANDLER_EV_TYPE_ENT evType,
  IN  GT_U8                     devNum,
  IN  GT_U8                     queueIdx,
  IN  GT_U32                    numOfBuff,
  IN  GT_U8                     *packetBuffs[],
  IN  GT_U32                    buffLen[],
  IN  GT_VOID                   *rxParamsPtr
)
{
    GT_U32  ii;
    evType = evType;
    cookie = cookie;

    for(ii = 0 ; ii < numRegistrations ; ii++)
    {
        (rxPktReceiveCbArray[ii])(devNum,queueIdx,numOfBuff,
                                packetBuffs,buffLen,rxParamsPtr);
    }
    return GT_OK;
}

/**
* @internal appDemoDxChNetRxPacketCbRegister function
* @endinternal
*
* @brief   register a CB function to be called on every RX packet to CPU
*         NOTE: function is implemented in the 'AppDemo' (application)
* @param[in] rxPktReceiveCbFun        - CB function to be called on every RX packet to CPU
*/
GT_STATUS appDemoDxChNetRxPacketCbRegister
(
    IN  RX_PACKET_RECEIVE_CB_FUN  rxPktReceiveCbFun
)
{
    GT_U32  ii;

    for(ii = 0 ; ii < numRegistrations ; ii++)
    {
        if(rxPktReceiveCbArray[ii] == rxPktReceiveCbFun)
        {
            /* Already registered */
            return GT_OK;
        }
    }

    if(numRegistrations >= MAX_REGISTER)
    {
        return GT_FULL;
    }

    rxPktReceiveCbArray[numRegistrations] = rxPktReceiveCbFun;

    numRegistrations++;

    /* will be added once */
    rxEventHandlerAddCallback(
            RX_EV_HANDLER_DEVNUM_ALL,
            RX_EV_HANDLER_QUEUE_ALL,
            RX_EV_HANDLER_TYPE_ANY_E,
            prvAppDemoDxChNetRxPacketRegisteredCb,
            0);

    return GT_OK;
}


/**
* @internal appDemoDxChNetRxPktHandle function
* @endinternal
*
* @brief   application routine to receive frames . -- DXCH function
*
* @param[in] devNum                   - Device number.
* @param[in] queueIdx                 - The queue from which this packet was received.
*                                       None.
*/
GT_STATUS appDemoDxChNetRxPktHandle
(
    IN GT_UINTPTR                           evHandler,
    IN RX_EV_HANDLER_EV_TYPE_ENT            evType,
    IN GT_U8                                devNum,
    IN GT_U8                                queueIdx
)
{
    GT_STATUS                           rc;
    GT_U32                              numOfBuff = BUFF_LEN;
    GT_U8                               *packetBuffs[BUFF_LEN];
    GT_U32                              buffLenArr[BUFF_LEN];
    CPSS_DXCH_NET_RX_PARAMS_STC         rxParams;

    /* get the packet from the device */
    if(appDemoPpConfigList[devNum].cpuPortMode == CPSS_NET_CPU_PORT_MODE_SDMA_E)
    {
        rc = cpssDxChNetIfSdmaRxPacketGet(devNum, queueIdx,&numOfBuff,
                            packetBuffs,buffLenArr,&rxParams);
    }
    else if(appDemoPpConfigList[devNum].cpuPortMode == CPSS_NET_CPU_PORT_MODE_MII_E)
    {
        rc = cpssDxChNetIfMiiRxPacketGet(devNum,queueIdx,&numOfBuff,packetBuffs,buffLenArr,&rxParams);
    }
    else
    {
        rc = GT_FAIL;
    }

    if (rc != GT_OK)  /* GT_NO_MORE is good and not an error !!! */
    {
        /* no need to free buffers because not got any */
        return rc;
    }

    if(evHandler)
    {
        rxEventHandlerDoCallbacks(evHandler, evType,
            devNum,queueIdx,numOfBuff,
            packetBuffs,buffLenArr,&rxParams);
    }

    if(appDemoPpConfigList[devNum].cpuPortMode == CPSS_NET_CPU_PORT_MODE_SDMA_E)
    {
        /* now you need to free the buffers */
        if(appDemoPpConfigList[devNum].allocMethod != CPSS_RX_BUFF_NO_ALLOC_E)
        {
            rc = cpssDxChNetIfRxBufFree(devNum,queueIdx,packetBuffs,numOfBuff);
        }
        else
        {

            /* move the first buffer pointer to the original place*/
            packetBuffs[0] = (GT_U8*)((((GT_UINTPTR)(packetBuffs[0])) >> APP_DEMO_RX_BUFF_ALLIGN_BITS_DEF)
                                      << APP_DEMO_RX_BUFF_ALLIGN_BITS_DEF);

            /* fix the last buffer's size*/
            buffLenArr[numOfBuff-1] = APP_DEMO_RX_BUFF_SIZE_DEF;

            rc = cpssDxChNetIfRxBufFreeWithSize(devNum,queueIdx,packetBuffs,buffLenArr,numOfBuff);
        }

    }
    else /* CPSS_NET_CPU_PORT_MODE_MII_E */
    {
        rc = cpssDxChNetIfMiiRxBufFree(devNum,queueIdx,packetBuffs,numOfBuff);
    }

    return rc;
}

extern void dxChNetIfRxPacketParse_DebugDumpEnable
(
    IN GT_BOOL  enableDumpRxPacket
);
/**
* @internal appDemoDxChNetRxPktHandle_allQueues function
* @endinternal
*
* @brief   allow call from terminal to treat Rx Packets to the CPU. -- DXCH function

*
* @param[in] devNum                   - Device number.
*
*/
GT_STATUS appDemoDxChNetRxPktHandle_allQueues
(
    IN GT_U8                                devNum
)
{
    GT_STATUS   rc;
    GT_U8    queueIdx;
    GT_UINTPTR  evHandler = 0;
    GT_U32      numOfNetIfs = 1;
    /* allow to 'see' the packets that we get (with the DSA) */
    dxChNetIfRxPacketParse_DebugDumpEnable(GT_TRUE);

    rc = prvCpssDxChNetIfMultiNetIfNumberGet(devNum,&numOfNetIfs);
    if (rc != GT_OK)
    {
        numOfNetIfs = 1;
    }


    for(queueIdx = 0 ;queueIdx < (8*numOfNetIfs) ; queueIdx++)
    {
        appDemoDxChNetRxPktHandle(evHandler,0,devNum,queueIdx);
    }

    return GT_OK;
}

/**
* @internal appDemoDxChNetRxPktGet function
* @endinternal
*
* @brief   application routine to get the packets from the queue and
*         put it in msgQ. -- DXCH function
* @param[in] devNum                   - Device number.
* @param[in] queueIdx                 - The queue from which this packet was received.
* @param[in] msgQId                   - Message queue Id.
*                                       None.
*/
GT_STATUS appDemoDxChNetRxPktGet
(
    IN GT_U8            devNum,
    IN GT_U8            queueIdx,
    IN CPSS_OS_MSGQ_ID  msgQId
)
{
    GT_STATUS                 rc;
    APP_DEMO_RX_PACKET_PARAMS rxParams;
    rxParams.numOfBuff = BUFF_LEN;
    cpssOsMutexLock(rxMutex);
    /* get the packet from the device */
    if(appDemoPpConfigList[devNum].cpuPortMode == CPSS_NET_CPU_PORT_MODE_SDMA_E)
    {
        rc = cpssDxChNetIfSdmaRxPacketGet(devNum, queueIdx,&(rxParams.numOfBuff),
                            rxParams.packetBuffs,rxParams.buffLenArr,
                                          &(rxParams.dxChNetRxParams));
    }
    else if(appDemoPpConfigList[devNum].cpuPortMode == CPSS_NET_CPU_PORT_MODE_MII_E)
    {
        rc = cpssDxChNetIfMiiRxPacketGet(devNum, queueIdx,&(rxParams.numOfBuff),
                            rxParams.packetBuffs,rxParams.buffLenArr,
                                          &(rxParams.dxChNetRxParams));
    }
    else
    {
        rc = GT_FAIL;
    }
    cpssOsMutexUnlock(rxMutex);

    if (rc != GT_OK)
    {
        /* no need to free buffers because not got any */
        return rc;
    }

    rxParams.devNum = devNum;
    rxParams.queue = queueIdx;
    /* put in msgQ */
    rc = cpssOsMsgQSend(msgQId,&rxParams,
                        sizeof(APP_DEMO_RX_PACKET_PARAMS),CPSS_OS_MSGQ_WAIT_FOREVER);
    return rc;
}

/**
* @internal appDemoDxChNetRxPktTreat function
* @endinternal
*
* @brief   application routine to treat the packets . -- DXCH function
*
* @param[in] rxParamsPtr              - (pointer to) rx paacket params
*                                       None.
*/
GT_STATUS appDemoDxChNetRxPktTreat
(
    IN APP_DEMO_RX_PACKET_PARAMS  *rxParamsPtr
)
{
    GT_STATUS   rc;
    GT_U8       devNum ;
    if (rxParamsPtr == NULL)
    {
        /* debug */
        return GT_BAD_PTR;
    }

    devNum = rxParamsPtr->devNum;

    rxEventHandlerDoCallbacks(rxEventHanderHnd, RX_EV_HANDLER_TYPE_RX_E,
            rxParamsPtr->devNum,rxParamsPtr->queue,
            rxParamsPtr->numOfBuff,rxParamsPtr->packetBuffs,
            rxParamsPtr->buffLenArr,&(rxParamsPtr->dxChNetRxParams));

    cpssOsMutexLock(rxMutex);
    if(appDemoPpConfigList[devNum].cpuPortMode == CPSS_NET_CPU_PORT_MODE_SDMA_E)
    {
        /* now you need to free the buffers */
        if(appDemoPpConfigList[devNum].allocMethod != CPSS_RX_BUFF_NO_ALLOC_E)
        {
            rc = cpssDxChNetIfRxBufFree(rxParamsPtr->devNum,rxParamsPtr->queue,rxParamsPtr->packetBuffs,
                                        rxParamsPtr->numOfBuff);
        }
        else
        {

            /* move the first buffer pointer to the original place*/
            rxParamsPtr->packetBuffs[0] = (GT_U8*)((((GT_UINTPTR)(rxParamsPtr->packetBuffs[0])) >> APP_DEMO_RX_BUFF_ALLIGN_BITS_DEF)
                                      << APP_DEMO_RX_BUFF_ALLIGN_BITS_DEF);
            /* fix the last buffer's size*/
            rxParamsPtr->buffLenArr[rxParamsPtr->numOfBuff-1] = APP_DEMO_RX_BUFF_SIZE_DEF;

            rc = cpssDxChNetIfRxBufFreeWithSize(rxParamsPtr->devNum,rxParamsPtr->queue,rxParamsPtr->packetBuffs,
                                                rxParamsPtr->buffLenArr,rxParamsPtr->numOfBuff);
        }
    }
    else if (appDemoPpConfigList[devNum].cpuPortMode == CPSS_NET_CPU_PORT_MODE_MII_E)
    {
        rc = cpssDxChNetIfMiiRxBufFree(rxParamsPtr->devNum,rxParamsPtr->queue
                                    ,rxParamsPtr->packetBuffs,rxParamsPtr->numOfBuff);
    }
    else
    {
        rc = GT_FAIL;
    }
    cpssOsMutexUnlock(rxMutex);

    return rc;
}

static GT_BOOL ptpOverUdpWaEnable = GT_FALSE;

/**
* @internal appDemoDxChNetPtpOverUdpWaCb function
* @endinternal
*
* @brief   Ptp over Udp (FEr-3112) WA implementation - RX to CPU CB function
*
* @note   APPLICABLE DEVICES:      XCAT2,Lion
* @param[in] devNum                   - Device number.
* @param[in] queueIdx                 - The queue from which this packet was received.
* @param[in] numOfBuff                - Num of used buffs in packetBuffs.
* @param[in] packetBuffs[]            - The received packet buffers list.
* @param[in] buffLen[]                - List of buffer lengths for packetBuffs.
* @param[in] rxParamsPtr              - (Pointer to)information parameters of received packets
*
* @retval GT_OK                    - no error
*/
GT_STATUS appDemoDxChNetPtpOverUdpWaCb
(
    IN GT_U8    devNum,
    IN GT_U8    queueIdx,
    IN GT_U32   numOfBuff,
    IN GT_U8    *packetBuffs[],
    IN GT_U32   buffLen[],
    IN void     *rxParamsPtr
)
{
    CPSS_DXCH_NET_RX_PARAMS_STC         *rxDxChParamsPtr;
    CPSS_DXCH_NET_TX_PARAMS_STC          txDxChParams;
    CPSS_TRUNK_MEMBER_STC                trunkMem;
    GT_TRUNK_ID                          trunkId;
    GT_STATUS rc;

    queueIdx = queueIdx;

    osMemSet(&txDxChParams, 0, sizeof(txDxChParams));
    if (ptpOverUdpWaEnable == GT_FALSE)
    {
        return GT_OK;
    }
    rxDxChParamsPtr = (CPSS_DXCH_NET_RX_PARAMS_STC *)rxParamsPtr;

    /* this function handles only TO_CPU PTP error packets */
    if (rxDxChParamsPtr->dsaParam.dsaType != CPSS_DXCH_NET_DSA_CMD_TO_CPU_E ||
        rxDxChParamsPtr->dsaParam.dsaInfo.toCpu.cpuCode != CPSS_NET_PTP_HEADER_ERROR_E)
    {
        return GT_OK;
    }

    txDxChParams.sdmaInfo.recalcCrc = GT_TRUE;
    txDxChParams.sdmaInfo.txQueue = 0;
    txDxChParams.packetIsTagged = GT_FALSE;
    txDxChParams.dsaParam.commonParams = rxDxChParamsPtr->dsaParam.commonParams;
    txDxChParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E;
    txDxChParams.dsaParam.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_VID_E;
    txDxChParams.dsaParam.dsaInfo.fromCpu.dstInterface.vlanId =
        rxDxChParamsPtr->dsaParam.commonParams.vid;
    txDxChParams.dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludeInterface = GT_TRUE;
    txDxChParams.dsaParam.dsaInfo.fromCpu.egrFilterEn = GT_TRUE;

    txDxChParams.dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludedInterface.hwDevNum =
            rxDxChParamsPtr->dsaParam.dsaInfo.toCpu.hwDevNum;

    /* check if source port is a trunk member. */
    trunkMem.hwDevice = rxDxChParamsPtr->dsaParam.dsaInfo.toCpu.hwDevNum;
    trunkMem.port = rxDxChParamsPtr->dsaParam.dsaInfo.toCpu.interface.portNum;
    rc = cpssDxChTrunkDbIsMemberOfTrunk(devNum,&trunkMem,&trunkId);

    if (rc != GT_OK && rc != GT_NOT_FOUND)
    {
        osPrintf("appDemoDxChNetPtpOverUdpWaCb: cpssDxChTrunkDbIsMemberOfTrunk failed! devNum[%d], rc=[%d]\n", devNum,rc);
        return rc;
    }

    if (rc == GT_OK) /* trunk */
    {
        txDxChParams.dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludedInterface.type =
            CPSS_INTERFACE_TRUNK_E;
        txDxChParams.dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludedInterface.trunkId = trunkId;
    }
    else /*port */
    {
        txDxChParams.dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludedInterface.type =
            CPSS_INTERFACE_PORT_E;
        txDxChParams.dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludedInterface.devPort.portNum =
            rxDxChParamsPtr->dsaParam.dsaInfo.toCpu.interface.portNum;
        txDxChParams.dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludedInterface.devPort.hwDevNum =
            rxDxChParamsPtr->dsaParam.dsaInfo.toCpu.hwDevNum;
    }

    rc = cpssDxChNetIfSdmaSyncTxPacketSend(devNum,&txDxChParams,packetBuffs,buffLen,numOfBuff);
    if (rc != GT_OK)
    {
        osPrintf("appDemoDxChNetPtpOverUdpWaCb: cpssDxChNetIfSdmaSyncTxPacketSend failed! devNum[%d], rc=[%d]\n", devNum,rc);
    }

    return GT_OK;
}

/**
* @internal appDemoDxChNetPtpOverUdpWaEnable function
* @endinternal
*
* @brief   Ptp over Udp (FEr-3112) WA implementation - enable function
*
* @note   APPLICABLE DEVICES:      XCAT2,Lion
* @param[in] enableWa                 - enable WA.
*
* @retval GT_OK                    - no error
*/
GT_STATUS appDemoDxChNetPtpOverUdpWaEnable
(
    IN GT_BOOL enableWa
)
{
    ptpOverUdpWaEnable = enableWa;
    return GT_OK;
}



