/*******************************************************************************
*              (c), Copyright 2016, Marvell International Ltd.                 *
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
* @file prvTgfNetIfTxSdmaEvents.c
*
* @brief Tx SDMA events testing
*
* @version   1
********************************************************************************
*/

/* Includes */

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <utf/private/prvUtfHelpers.h>

#include <common/tgfNetIfGen.h>
#include <netIf/prvTgfNetIfTxSdmaEvents.h>

#include <appDemo/userExit/userEventHandler.h>

#include <gtUtil/gtBmPool.h>

/* Defines */
#define TX5_SDMA_CNS (5)
#define BURST_COUNT_CNS (10)
#define PRV_TGF_SEND_PORT_IDX_CNS (1)

/* Local variables */
static GT_U8 packetData[]  =
{
    0x00, 0x00, 0x02, 0x03, 0x04, 0x05,
    0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x33, 0x33,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F
};

/* Packet length */
#define PRV_TGF_PACKET_LENGTH_CNS    64

static EVENT_NOTIFY_FUNC *eventHandlerPtr;
static CPSS_OS_SIG_SEM eventBufCame = CPSS_OS_SEMB_EMPTY_E;
static CPSS_OS_SIG_SEM eventEndCame = CPSS_OS_SEMB_EMPTY_E;
static GT_U32 evBufCtr = BURST_COUNT_CNS + 1; /* Make sure semaphores will */
static GT_U32 evEndCtr = BURST_COUNT_CNS + 1; /* not signal before the TX*/
static CPSS_EVENT_MASK_SET_ENT bufMasked;
static CPSS_EVENT_MASK_SET_ENT endMasked;
static GT_POOL_ID prvTgfTxBuffersPoolId = 0;

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal netIfTxSdmaEventsHandler function
* @endinternal
*
* @brief   Function that handles events of SDMA Tx engine
*
* @param[in] devNum                   - device identifier
* @param[in] uniEv                    - event type
* @param[in] evExtData                - payload data
*                                       None
*
* @note The routine is only for CPSS_PP_TX_END_E, CPSS_PP_TX_BUFFER_QUEUE_E
*       cannot be handled along with it.
*       The routine is thread safe because it is short enough and events
*       come one after another.
*
*/
static GT_VOID netIfTxSdmaEventsHandler
(
        IN GT_U8    devNum,
        IN GT_U32   uniEv,
        IN GT_U32   evExtData
)
{
    if (devNum == prvTgfDevNum) 
    {
        if(CPSS_PP_TX_END_E == uniEv)
        {
            if(TX5_SDMA_CNS == evExtData)
            {
                evEndCtr++;
                if (evEndCtr == BURST_COUNT_CNS)
                {
                    cpssOsSigSemSignal(eventEndCame);
                }
            }
        }
        else
        {
            /* Do nothing for other events */
        }
    }
}

/**
* @internal netIfTxSdmaBufferEventsHandler function
* @endinternal
*
* @brief   Function that handles events of SDMA Tx engine
*
* @param[in] hndl                     - user handle for bounded event
* @param[in] cookie                   - user cookie
*                                       None
*
* @note The routine is thread safe because it is short and enough events come
*       one after another.
*
*/
static GT_VOID netIfTxSdmaBufferEventsHandler
(
        IN GT_UINTPTR hndl,
        IN GT_VOID    *cookie
)
{
    TGF_PARAM_NOT_USED(cookie);
    TGF_PARAM_NOT_USED(hndl);
    evBufCtr++;
    if (evBufCtr == BURST_COUNT_CNS)
    {
        cpssOsSigSemSignal(eventBufCame);
    }

}

/**
* @internal netIfTxSdmaPacketParamsInit function
* @endinternal
*
* @brief   Init packet to send
*
* @param[out] pcktParamsPtr            - packet parameters
*                                       None
*
* @note Does not set parmeters connect with TxBufferQueueEvent
*
*/
static GT_VOID netIfTxSdmaPacketParamsInit
(
    OUT PRV_TGF_NET_TX_PARAMS_STC *pcktParamsPtr
)
{
    cpssOsMemSet(pcktParamsPtr, 0, sizeof(PRV_TGF_NET_TX_PARAMS_STC));
    pcktParamsPtr->sdmaInfo.txQueue = TX5_SDMA_CNS;
    pcktParamsPtr->sdmaInfo.recalcCrc = 1;
    pcktParamsPtr->sdmaInfo.invokeTxBufferQueueEvent = GT_FALSE;
    pcktParamsPtr->packetIsTagged = GT_FALSE;
    if(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS] < BIT_5)
    {
        pcktParamsPtr->dsaParam.dsaType = TGF_DSA_2_WORD_TYPE_E;
    }
    else
    {
        pcktParamsPtr->dsaParam.dsaType = TGF_DSA_4_WORD_TYPE_E;
    }
    pcktParamsPtr->dsaParam.dsaCommand = TGF_DSA_CMD_FROM_CPU_E;
    pcktParamsPtr->dsaParam.dsaInfo.fromCpu.srcHwDev = prvTgfDevNum;
    pcktParamsPtr->dsaParam.dsaInfo.fromCpu.cascadeControl = GT_FALSE;
    pcktParamsPtr->dsaParam.dsaInfo.fromCpu.tc = 0;
    pcktParamsPtr->dsaParam.dsaInfo.fromCpu.dp = 0;
    pcktParamsPtr->dsaParam.dsaInfo.fromCpu.egrFilterEn = GT_FALSE;
    pcktParamsPtr->dsaParam.dsaInfo.fromCpu.egrFilterRegistered = GT_FALSE;
    pcktParamsPtr->dsaParam.dsaInfo.fromCpu.srcId = 0;
    pcktParamsPtr->dsaParam.dsaInfo.fromCpu.dstInterface.type =
        CPSS_INTERFACE_PORT_E;
    pcktParamsPtr->dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum =
        prvTgfDevNum;
    pcktParamsPtr->dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum =
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    pcktParamsPtr->dsaParam.dsaInfo.fromCpu.dstInterface.vlanId = 1;
    pcktParamsPtr->dsaParam.dsaInfo.fromCpu.extDestInfo.devPort.dstIsTagged =
        GT_FALSE;
    pcktParamsPtr->dsaParam.dsaInfo.fromCpu.extDestInfo.devPort.
        mailBoxToNeighborCPU = GT_FALSE;
    pcktParamsPtr->dsaParam.commonParams.cfiBit = 0;
    pcktParamsPtr->dsaParam.commonParams.vid = 1;
    pcktParamsPtr->dsaParam.commonParams.vpt = 0;
    pcktParamsPtr->dsaParam.commonParams.dropOnSource = GT_FALSE;
    pcktParamsPtr->dsaParam.commonParams.packetIsLooped = GT_FALSE;    
}

/**
* @internal prvTgfNetIfTxSdmaEventsInit function
* @endinternal
*
* @brief   Initialize test:
*         - Save old event handler
*         - Set new event handler
*         - Save enabled state of tx queue
*         - Enable tx queue
*         - Save masked state of events
*         - Unmask events
*         - Create TX buffers pool
*/
GT_VOID prvTgfNetIfTxSdmaEventsInit
(
    GT_VOID
)
{
    GT_STATUS rc;

    eventHandlerPtr = notifyEventArrivedFunc;
    notifyEventArrivedFunc = netIfTxSdmaEventsHandler;

    rc = cpssEventDeviceMaskWithEvExtDataGet(prvTgfDevNum,
        CPSS_PP_TX_BUFFER_QUEUE_E, TX5_SDMA_CNS, &bufMasked);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
        "cpssEventDeviceMaskWithEvExtDataGet: %d, %d, %d", prvTgfDevNum,
        CPSS_PP_TX_BUFFER_QUEUE_E, TX5_SDMA_CNS);

    rc = cpssEventDeviceMaskWithEvExtDataGet(prvTgfDevNum,
        CPSS_PP_TX_END_E, TX5_SDMA_CNS, &endMasked);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
        "cpssEventDeviceMaskWithEvExtDataGet: %d, %d, %d", prvTgfDevNum,
        CPSS_PP_TX_END_E, TX5_SDMA_CNS);

    rc = cpssEventDeviceMaskWithEvExtDataSet(prvTgfDevNum,
        CPSS_PP_TX_BUFFER_QUEUE_E, TX5_SDMA_CNS, CPSS_EVENT_UNMASK_E);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
        "cpssEventDeviceMaskWithEvExtDataSet: %d, %d, %d, %d", prvTgfDevNum,
        CPSS_PP_TX_BUFFER_QUEUE_E, TX5_SDMA_CNS, CPSS_EVENT_UNMASK_E);

    rc = cpssEventDeviceMaskWithEvExtDataSet(prvTgfDevNum,
        CPSS_PP_TX_END_E, TX5_SDMA_CNS, CPSS_EVENT_UNMASK_E);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
        "cpssEventDeviceMaskWithEvExtDataSet: %d, %d, %d, %d", prvTgfDevNum,
        CPSS_PP_TX_END_E, TX5_SDMA_CNS, CPSS_EVENT_UNMASK_E);

    rc = cpssOsSigSemBinCreate("eventBufCame", CPSS_OS_SEMB_EMPTY_E,
        &eventBufCame);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssOsSigSemBinCreate");

    rc = cpssOsSigSemBinCreate("eventEndCame", CPSS_OS_SEMB_EMPTY_E,
        &eventEndCame);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssOsSigSemBinCreate");

    rc = gtPoolCreateDmaPool(PRV_TGF_PACKET_LENGTH_CNS,
        GT_4_BYTE_ALIGNMENT,
        1, /* Only one buffer in pool */
        GT_TRUE,
        &prvTgfTxBuffersPoolId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "gtPoolCreateDmaPool");
}

/**
* @internal prvTgfNetIfTxSdmaEventsGenerate function
* @endinternal
*
* @brief   - Generate traffic
*         - Catch events
*/
GT_VOID prvTgfNetIfTxSdmaEventsGenerate
(
    GT_VOID
)
{
    PRV_TGF_NET_TX_PARAMS_STC pcktParams;
    GT_UINTPTR txBufHndl[BURST_COUNT_CNS];
    GT_STATUS  rc;
    GT_U8      *buffList[1];
    GT_U8      *bufferPtr;
    GT_U32     buffLenList[1];
    GT_U32     timeout = 5000;
    GT_U32     ii;
    CPSS_UNI_EV_CAUSE_ENT uniEventArr[1];

    /* AUTODOC: Catch CPSS_PP_TX_BUFFER_QUEUE_E and CPSS_PP_TX_END_E */
    /* AUTODOC: Initialize packet descriptor */
    uniEventArr[0] = CPSS_PP_TX_BUFFER_QUEUE_E;
    netIfTxSdmaPacketParamsInit(&pcktParams);
    if(prvUtfIsDoublePhysicalPortsModeUsed())
    {
        PRV_TGF_NET_TX_GENERATOR_FORCE_USE_EPORT_MAC(pcktParams);
    }

    /* AUTODOC: Prepare TX buffer */
    bufferPtr = gtPoolGetBuf(prvTgfTxBuffersPoolId);
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC(0, (GT_UINTPTR)bufferPtr, "gtPoolGetBuf");
    if(NULL == bufferPtr)
    {
        return;
    }

    cpssOsMemCpy(bufferPtr, packetData, PRV_TGF_PACKET_LENGTH_CNS);
    buffList[0] = bufferPtr;
    buffLenList[0] = PRV_TGF_PACKET_LENGTH_CNS;

    pcktParams.sdmaInfo.invokeTxBufferQueueEvent = GT_TRUE;

    /* AUTODOC: Send packet ten times */
    evEndCtr = 0;
    evBufCtr = 0;
    for(ii = 0; ii < BURST_COUNT_CNS; ii++)
    {
        /* AUTODOC: Bind ISR to event handle */
        rc = cpssEventIsrBind(uniEventArr, 1, netIfTxSdmaBufferEventsHandler,
            0, &txBufHndl[ii]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssEventBind");

        /* AUTODOC: Set own event handle for each packet */
        pcktParams.sdmaInfo.evReqHndl = txBufHndl[ii];

        /* AUTODOC: Send packet */
        rc = prvTgfNetIfSdmaTxPacketSend(prvTgfDevNum, &pcktParams, buffList,
            buffLenList, 1);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfNetIfSdmaTxPacketSend: "
            "devNum=[%d], bufNum=[%d]", prvTgfDevNum, 1);
        cpssOsTimerWkAfter(50);
    }

    /* AUTODOC: Wait for events to come */
    rc = cpssOsSigSemWait(eventBufCame, timeout);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssOsSigSemWait");

    rc = cpssOsSigSemWait(eventEndCame, timeout);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssOsSigSemWait");


    /* AUTODOC: Check event count */
    UTF_VERIFY_EQUAL2_STRING_MAC(BURST_COUNT_CNS, evBufCtr,
        "Unexpected CPSS_PP_TX_BUFFER_QUEUE_E event count %d, expected %d",
        evBufCtr, BURST_COUNT_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(BURST_COUNT_CNS, evEndCtr,
        "Unexpected CPSS_PP_TX_END_E event count %d, expected %d",
        evEndCtr, BURST_COUNT_CNS);

    PRV_UTF_LOG0_MAC("\n================================================\n");
    PRV_UTF_LOG1_MAC("CPSS_PP_TX_BUFFER_QUEUE_E events: %d\n", evBufCtr);
    PRV_UTF_LOG1_MAC("CPSS_PP_TX_END_E events: %d\n", evEndCtr);
    PRV_UTF_LOG0_MAC("================================================\n");

    /* AUTODOC: Destroy event handles */
    for(ii = 0; ii < BURST_COUNT_CNS; ii++)
    {
        rc = cpssEventDestroy(txBufHndl[ii]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssEventDestroy");
    }
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    rc = gtPoolFreeBuf(prvTgfTxBuffersPoolId, bufferPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "gtPoolFreeBuf");

}

/**
* @internal prvTgfNetIfTxSdmaEventsRestore function
* @endinternal
*
* @brief   Restore configuration:
*         - Delete TX buffers pool
*         - Restore masked state of events
*         - Restore enabled state of tx queue
*         - Restore default handler
*/
GT_VOID prvTgfNetIfTxSdmaEventsRestore
(
    GT_VOID
)
{
    GT_STATUS rc;

    gtPoolDeletePool(prvTgfTxBuffersPoolId);

    rc = cpssEventDeviceMaskWithEvExtDataSet(prvTgfDevNum,
        CPSS_PP_TX_BUFFER_QUEUE_E,
        PRV_CPSS_DRV_EV_REQ_UNI_EV_EXTRA_DATA_ANY_CNS,
        bufMasked);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
        "cpssEventDeviceMaskWithEvExtDataSet: %d, %d, %d, %d",
        prvTgfDevNum, CPSS_PP_TX_BUFFER_QUEUE_E, TX5_SDMA_CNS, bufMasked);

    rc = cpssEventDeviceMaskWithEvExtDataSet(prvTgfDevNum, CPSS_PP_TX_END_E,
        TX5_SDMA_CNS, endMasked);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
        "cpssEventDeviceMaskWithEvExtDataSet: %d, %d, %d, %d", prvTgfDevNum,
        CPSS_PP_TX_END_E, TX5_SDMA_CNS, endMasked);


    notifyEventArrivedFunc = eventHandlerPtr;
}


