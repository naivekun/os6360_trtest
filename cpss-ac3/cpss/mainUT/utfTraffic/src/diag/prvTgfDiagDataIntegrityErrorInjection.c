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
* @file prvTgfDiagDataIntegrityErrorInjection.c
*
* @brief Tests of error injection in various memories
*
* @version   1
********************************************************************************
*/

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <appDemo/userExit/userEventHandler.h>
#include <cpss/dxCh/dxChxGen/diag/cpssDxChDiagDataIntegrity.h>

#define VLANID_CNS           (5)
#define SENDER_PORT          (0)
#define WAIT_EVENTS_TIME_CNS (10000)

typedef enum
{
    PACKET_COUNTERS_EMPTY_E = 1,
    PACKET_COUNTERS_GOOD_RECEIVER_PORT_E,
    PACKET_COUNTERS_GOOD_SENDER_PORT_E
} PACKET_COUNTERS_STATE_ENT;

/* Memories to test */
typedef struct
{
    const char                      *name;
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT memType;
    GT_U32                          burstCount;
    GT_U32                          expectedErrors;
    PACKET_COUNTERS_STATE_ENT       expectedCounters[PRV_TGF_MAX_PORTS_NUM_CNS];
    GT_U32                          expectedValidPackets;
    GT_U32                          gotErrors;
    GT_U32                          gotValidPackets;
} MEMORY_DESC_STC;

#define MEMORY_ENTRY_CUSTOM_COUNTERS_MAC(__memType, __burstCount, __expEventNum, __sender, __receiver) \
    {#__memType, __memType, __burstCount, __expEventNum, {__sender, __receiver, __receiver, \
     __receiver, 0, }, (__burstCount) * 4, 0, 0}

#define MEMORY_ENTRY_PASSED_MAC(__memType, __burstCount, __expEventNum)  \
    MEMORY_ENTRY_CUSTOM_COUNTERS_MAC(__memType, __burstCount, __expEventNum, \
    PACKET_COUNTERS_GOOD_SENDER_PORT_E, PACKET_COUNTERS_GOOD_RECEIVER_PORT_E)

static MEMORY_DESC_STC memoriesArr[] =
{
    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_LINK_LIST_E,
        1, 4),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_LINK_LIST_PTR_E,
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_WRR_STATE_VARIABLES_E,
        2, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_BC_UPD_E,
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
    	CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QTAIL_E,
        2, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QHEAD_E,
        2, 1),

#if 0  /* need to understand how to get the RAM accessed */
    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_FREE_BUFS_E, 2, 1),
#endif

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PORT_E,
        1, 1),
};


/* memories for bobcat3 */
static MEMORY_DESC_STC memoriesArrBc3[] =
{

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PORT_E,
        1, 6),

    /* WRR occurs (>= 4 times) on every packet transmitting
       except first one. So send two packets. */
    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_WRR_STATE_VARIABLES_E,
        2, 4 /* Can be more than 4 */),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_BC_UPD_E,
        1, 4),

    /* one of QHEAD0...QHEAD3 occurs 4 times on every iteration */
    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QHEAD0_E,
        4, 4),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QHEAD1_E,
        4, 4),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QHEAD2_E,
        4, 4),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QHEAD3_E,
        4, 4),

   /* one of QLAST0, ..., QLAST3 occurs on every packet transmitting
       expect first 4 ones. So send 8 packets to be sure event occurs
       for every of QLAST0, ..., QLAST3 */
    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QLAST0_E,
        8, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QLAST1_E,
        8, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QLAST2_E,
        8, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QLAST3_E,
        8, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_BMX_DATA_BASE_E,
        1, 16 /* can be more that 16 */)
};


/* memories for aldrin2 */
static MEMORY_DESC_STC memoriesArrAldrin2[] =
{

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PORT_E,
        1, 4),

    /* WRR occurs (>= 4 times) on every packet transmitting
       except first one. So send two packets. */
    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_WRR_STATE_VARIABLES_E,
        2, 4 /* Can be more than 4 */),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_BC_UPD_E,
        1, 4),

    /* one of QHEAD0...QHEAD6 occurs 4 times on every iteration */
    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QHEAD0_E,
        8, 4),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QHEAD1_E,
        8, 4),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QHEAD2_E,
        8, 4),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QHEAD3_E,
        8, 4),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QHEAD4_E,
        8, 4),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QHEAD5_E,
        8, 4),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QHEAD6_E,
        8, 4),

   /* one of QLAST0, ..., QLAST6 occurs on every packet transmitting
       except first 7 ones. So send 24 packets to be sure event occurs
       for every of QLAST0, ..., QLAST6 */
    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QLAST0_E,
        24, 4),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QLAST1_E,
        24, 4),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QLAST2_E,
        24, 4),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QLAST3_E,
        24, 4),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QLAST4_E,
        24, 4),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QLAST5_E,
        24, 4),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QLAST6_E,
        24, 4),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_BMX_DATA_BASE_E,
        1, 8 /* can be more that 8 */)
};

static MEMORY_DESC_STC *currentMemEntryPtr;

/* Event counter and semaphore signaling we have all needed events */
static GT_U32          evErrorCtr = 0;
static CPSS_OS_SIG_SEM eventErrorsCame = CPSS_OS_SEMB_EMPTY_E;

/* Callback function prototype for event counting */
typedef GT_STATUS DXCH_DATA_INTEGRITY_EVENT_CB_FUNC(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC  *eventPtr
);

extern DXCH_DATA_INTEGRITY_EVENT_CB_FUNC *dxChDataIntegrityEventIncrementFunc;
static DXCH_DATA_INTEGRITY_EVENT_CB_FUNC *savedDataIntegrityErrorCb;

/*
 * Packets info
 */

/* L2 part of unknown UC packet */
#define SEND_PACKET_SIZE_CNS (64)

static TGF_PACKET_L2_STC ucPacketL2Part =
{
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, /* dstMac */
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x02}  /* srcMac */
};
static TGF_PACKET_VLAN_TAG_STC packetVlanTagPart =
{
        TGF_ETHERTYPE_8100_VLAN_TAG_CNS, /* etherType */
        0,  0,  VLANID_CNS               /* pri, cfi, VlanId */
};
static GT_U8 payloadDataArr[SEND_PACKET_SIZE_CNS] =
{
        0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55
};
static TGF_PACKET_PAYLOAD_STC packetPayloadPart =
{
        sizeof(payloadDataArr),   /* dataLength */
        payloadDataArr            /* dataPtr */
};
static TGF_PACKET_PART_STC ucPcktArray[] =
{
        {TGF_PACKET_PART_L2_E,       &ucPacketL2Part}, /* type, partPtr */
        {TGF_PACKET_PART_VLAN_TAG_E, &packetVlanTagPart},
        {TGF_PACKET_PART_PAYLOAD_E,  &packetPayloadPart}
};
#define PACKET_LEN_CNS \
        TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + \
        sizeof(payloadDataArr)

#define PACKET_CRC_LEN_CNS (PACKET_LEN_CNS + TGF_CRC_LEN_CNS)

#define PACKET_CRC_LEN_WO_TAG_CNS (TGF_L2_HEADER_SIZE_CNS + \
    sizeof(payloadDataArr) + TGF_CRC_LEN_CNS)

static TGF_PACKET_STC packetInfo = {
        PACKET_LEN_CNS,
        sizeof(ucPcktArray) / sizeof(TGF_PACKET_PART_STC),
        ucPcktArray
};

static GT_U32 memToDebug = 0xFFFFFFFF;
void memToDebugSet(GT_U32 newMemToDebug)
{
    memToDebug = newMemToDebug;
}


/**
* @internal dataIntegrityPacketCheck function
* @endinternal
*
* @brief   Callback called in case of data integrity error.
*
* @param[in] packetCountersPtr        - packet counters.
* @param[in] expectedState            - expected state of the packet counters
*/
static GT_BOOL dataIntegrityPacketCheck
(
    IN CPSS_PORT_MAC_COUNTER_SET_STC *packetCountersPtr,
    IN PACKET_COUNTERS_STATE_ENT     expectedState
)
{
    GT_BOOL retVal;
    GT_U32  ii;
    GT_U8   *ptr;

    switch(expectedState)
    {
        case PACKET_COUNTERS_EMPTY_E:
            ptr = (GT_U8 *)packetCountersPtr;
            retVal = GT_TRUE;
            for(ii = 0; ii < sizeof(CPSS_PORT_MAC_COUNTER_SET_STC); ii++)
            {
                if(ptr[ii] != 0)
                {
                    retVal = GT_FALSE;
                }
            }
            break;
        case PACKET_COUNTERS_GOOD_RECEIVER_PORT_E:
            if((packetCountersPtr->goodOctetsSent.l[0] == PACKET_CRC_LEN_WO_TAG_CNS) &&
               (packetCountersPtr->goodPktsSent.l[0]   == 1) &&
               (packetCountersPtr->ucPktsSent.l[0]     == 1)
              )
            {
                retVal = GT_TRUE;
            }
            else
            {
                retVal = GT_FALSE;
            }
            break;
        case PACKET_COUNTERS_GOOD_SENDER_PORT_E:
            if((packetCountersPtr->goodOctetsRcv.l[0]  == PACKET_CRC_LEN_CNS) &&
               (packetCountersPtr->goodPktsRcv.l[0]    == 1) &&
               (packetCountersPtr->goodOctetsSent.l[0] == PACKET_CRC_LEN_CNS) &&
               (packetCountersPtr->goodPktsSent.l[0]   == 1) &&
               (packetCountersPtr->ucPktsRcv.l[0]      == 1) &&
               (packetCountersPtr->ucPktsSent.l[0]     == 1)
              )
            {
                retVal = GT_TRUE;
            }
            else
            {
                retVal = GT_FALSE;
            }
            break;
        default:
            retVal = GT_FALSE;
    }
    return retVal;
}

/**
* @internal dataIntegrityErrorHandler function
* @endinternal
*
* @brief   Callback called in case of data integrity error.
*
* @param[in] dev                      - the device.
* @param[in] eventPtr                 - filled structure which describes data integrity error event
*/
static GT_STATUS dataIntegrityErrorHandler
(
 IN GT_U8                                     dev,
 IN CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC  *eventPtr
 )
{
    /* Print debug info */
    PRV_UTF_LOG0_MAC("======DATA INTEGRITY ERROR EVENT======\n");
    PRV_UTF_LOG2_MAC("Device     : %d - %d\n", dev, prvTgfDevNum);
    PRV_UTF_LOG1_MAC("Event type : %d\n", eventPtr->eventsType);
    PRV_UTF_LOG5_MAC("Memory     : %d {%d, %d, %d}, line # %d\n ",
                     eventPtr->location.ramEntryInfo.memType,
                     eventPtr->location.ramEntryInfo.memLocation.dfxPipeId,
                     eventPtr->location.ramEntryInfo.memLocation.dfxClientId,
                     eventPtr->location.ramEntryInfo.memLocation.dfxMemoryId,
                     eventPtr->location.ramEntryInfo.ramRow);

    if((dev == prvTgfDevNum) &&
        (eventPtr->location.ramEntryInfo.memType == currentMemEntryPtr->memType)
      )
    {
        evErrorCtr++;
        if(evErrorCtr == currentMemEntryPtr->expectedErrors)
        {
            cpssOsSigSemSignal(eventErrorsCame);
        }
    }
    return GT_OK;
}

/**
* @internal dataIntegrityInjectError function
* @endinternal
*
* @brief   Inject error in specified memory or disable injection.
*
* @param[in] memType                  - memory type
* @param[in] enable                   - if GT_TRUE --  error injection. In case of GT_FALSE --
*                                      disable error injection.
*                                       None
*/
static GT_VOID dataIntegrityInjectError
(
    IN CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT memType,
    IN GT_BOOL enable
)
{
    GT_STATUS                            rc;
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC memoryLocation;

    memoryLocation.portGroupsBmp = 0xFFFFFFFF;
    memoryLocation.type = CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE;
    memoryLocation.info.ramEntryInfo.memType = memType;

    rc = cpssDxChDiagDataIntegrityErrorInjectionConfigSet(
        prvTgfDevNum,
        &memoryLocation,
        CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E,
        enable
    );
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
        "cpssDxChDiagDataIntegrityErrorInjectionConfigGet: dev=%d, injectEn=%d",
        prvTgfDevNum, enable );
}

/**
* @internal dataIntegrityInterruptConfig function
* @endinternal
*
* @brief   Mask or unmask interrupt
*
* @param[in] memType                  - memory type
*                                      enable  - if GT_TRUE -- enable error injection. In case of GT_FALSE --
*                                      disable error injection.
*                                       None
*/
static GT_VOID dataIntegrityInterruptConfig
(
    IN CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT memType,
    IN CPSS_EVENT_MASK_SET_ENT                    operation
)
{
    GT_STATUS                            rc;
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC memoryLocation;

    memoryLocation.portGroupsBmp = 0xFFFFFFFF;
    memoryLocation.type = CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE;
    memoryLocation.info.ramEntryInfo.memType = memType;

    rc = cpssDxChDiagDataIntegrityEventMaskSet(
        prvTgfDevNum,
        &memoryLocation,
        CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E,
        operation
    );
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
        "cpssDxChDiagDataIntegrityEventMaskSet: "
        "dev=[%d], "
        "mode=[CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E], "
        "operation=[%d]",
        prvTgfDevNum,
        operation
    );
}


/**
* @internal prvTgfDiagDataIntegrityErrorInjectionConfigSet function
* @endinternal
*
* @brief   Sets configuration
*/
GT_VOID prvTgfDiagDataIntegrityErrorInjectionConfigSet
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: Create semaphore to wait all DI errors found */
    rc = cpssOsSigSemBinCreate("eventErrorsCame", CPSS_OS_SEMB_EMPTY_E,
        &eventErrorsCame);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssOsSigSemBinCreate");

    /* AUTODOC: Save DI error callback and set own */
    savedDataIntegrityErrorCb = dxChDataIntegrityEventIncrementFunc;
    dxChDataIntegrityEventIncrementFunc = dataIntegrityErrorHandler;

    /* AUTODOC: Enable HW errors ignoring */
    appDemoEventFatalErrorEnable(APP_DEMO_FATAL_ERROR_NOTIFY_ONLY_TYPE_E);

    /* AUTODOC: Create VLAN from available ports */
    rc = prvTgfBrgDefVlanEntryWrite(VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgfBrgDefVlanEntryWrite: vlanId=[%d]", VLANID_CNS);
}

/**
* @internal prvTgfDiagDataIntegrityErrorInjectionTrafficGenerate function
* @endinternal
*
* @brief   Restores configuration
*/
GT_VOID prvTgfDiagDataIntegrityErrorInjectionTrafficGenerate
(
    GT_VOID
)
{
    GT_U32                        ii;
    GT_U32                        jj;
    GT_STATUS                     rc;
    GT_U32                        portIter;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    GT_BOOL                       validCntrs;
    GT_U32                        expectedPacketCounter;
    MEMORY_DESC_STC               *memEntryPtr = NULL;
    CPSS_INTERFACE_INFO_STC       iface;
    MEMORY_DESC_STC               *memArr;
    GT_U32                        memArrLen;
    GT_U32                        doSkip;

    cpssOsMemSet(&iface, 0, sizeof(iface));
    iface.type              = CPSS_INTERFACE_PORT_E;
    iface.devPort.hwDevNum  = prvTgfDevNum;


    if (PRV_CPSS_DXCH_BOBCAT3_CHECK_MAC(prvTgfDevNum))
    {
        memArr = memoriesArrBc3;
        memArrLen = sizeof(memoriesArrBc3)/sizeof(memoriesArrBc3[0]);
    }
    else if (PRV_CPSS_DXCH_ALDRIN2_CHECK_MAC(prvTgfDevNum))
    {
        memArr = memoriesArrAldrin2;
        memArrLen = sizeof(memoriesArrAldrin2)/sizeof(memoriesArrAldrin2[0]);
    }
    else
    {
        memArr = memoriesArr;
        memArrLen = sizeof(memoriesArr)/sizeof(memoriesArr[0]);
    }

    /* Setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, 1, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth: dev=[%d]",
        prvTgfDevNum);

    for(ii = 0; ii < memArrLen; ii++)
    {
        if ((memToDebug != 0xFFFFFFFF) && (memToDebug != ii))
        {
            continue;
        }

        memEntryPtr = &memArr[ii];

        switch (memEntryPtr->memType)
        {
            case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_LINK_LIST_E:
                /* The memory is accessed during the test but there are
                   no interrupts. Maybe several writings happen before reading
                   and reset injected bit. Skip. */
                doSkip = IS_BOBK_DEV_MAC(prvTgfDevNum);
                break;

            case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QHEAD_E:
            case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QTAIL_E:
                /* events doesn't occur on bobk with enabled TM. */
                doSkip = (IS_BOBK_DEV_MAC(prvTgfDevNum) &&
                          prvUtfIsTrafficManagerUsed());
                break;

            default:
                doSkip = 0;
                break;
        }
        if (doSkip)
        {
            continue;
        }

        PRV_UTF_LOG1_MAC("=== Start test for %s ===\n",memEntryPtr->name);

        currentMemEntryPtr = memEntryPtr;

        expectedPacketCounter = 0;

        /* AUTODOC: Unmask event */
        dataIntegrityInterruptConfig(memEntryPtr->memType, CPSS_EVENT_UNMASK_E);

        for(jj = 0; jj < memEntryPtr->burstCount; jj++)
        {
            /* AUTODOC: Reset counters */
            rc = prvTgfEthCountersReset(prvTgfDevNum);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

            /* AUTODOC: Inject error */
            dataIntegrityInjectError(memEntryPtr->memType, GT_TRUE);

            /* Do not print packets */
            tgfTrafficTracePacketByteSet(GT_FALSE);

            /* WA for 100G ports. To make 100G link up CPSS API sets
               fields RX_ENA, TX_ENA to 1 in "COMMAND_CONFIG Register".
               RX_ENA=1 causes loopback when packet goes on 100G tx port.
               tgfTrafficGeneratorPortLoopbackModeEnableSet(GT_FALSE)
               sets RX_ENA to 0. Should be placed after prvTgfEthCountersReset,
               because this functions makes link up. */
            for (portIter=0; portIter< prvTgfPortsNum; portIter++)
            {
                iface.devPort.portNum = prvTgfPortsArray[portIter];
                rc = tgfTrafficGeneratorPortLoopbackModeEnableSet(&iface, GT_FALSE);
                UTF_VERIFY_EQUAL0_STRING_MAC(
                    GT_OK, rc, "tgfTrafficGeneratorPortLoopbackModeEnableSet");
            }


            /* AUTODOC: Send packet */
            rc = prvTgfStartTransmitingEth(prvTgfDevNum,
                prvTgfPortsArray[SENDER_PORT]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                "prvTgfStartTransmitingEth: dev=[%d], port=[%d]\n",
                prvTgfDevNum, prvTgfPortsArray[SENDER_PORT]);

            for(portIter = 0; portIter < prvTgfPortsNum; portIter++)
            {
                if(memEntryPtr->expectedCounters[portIter] == 0)
                {
                    break;
                }

                rc = prvTgfReadPortCountersEth(prvTgfDevNum,
                    prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                    "prvTgfReadPortCountersEth: dev=[%d], enablePrint[%d]\n",
                    prvTgfDevNum, prvTgfPortsArray[portIter]);

                validCntrs = dataIntegrityPacketCheck(&portCntrs,
                    memEntryPtr->expectedCounters[portIter]);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, validCntrs,
                    "Got unexpected counters");
                if(validCntrs)
                {
                    expectedPacketCounter++;
                }
            }
            memEntryPtr->gotValidPackets = expectedPacketCounter;
        }

        /* AUTODOC: Wait for events */
        rc = cpssOsSigSemWait(eventErrorsCame, WAIT_EVENTS_TIME_CNS);

        if(rc != GT_OK)
        {
            PRV_UTF_LOG2_MAC("Not all events came. "
                "We have %d, but expected %d\n",
                evErrorCtr, memEntryPtr->expectedErrors);

            /* Send signal to free semaphore */
            cpssOsSigSemSignal(eventErrorsCame);
        }
        else
        {
            /* AUTODOC: Maybe some unexpected signals will come */
            cpssOsTimerWkAfter(1000);
        }

        /* allow number or errors to be more then expected. */
        if(memEntryPtr->expectedErrors >= evErrorCtr)
        {
            UTF_VERIFY_EQUAL3_STRING_MAC(memEntryPtr->expectedErrors, evErrorCtr,
                "Unexpected event count for memType=[%d], expected %d, got %d",
                ii, memEntryPtr->expectedErrors, evErrorCtr);
        }

        memEntryPtr->gotErrors = evErrorCtr;
        evErrorCtr = 0;
        dataIntegrityInjectError(memEntryPtr->memType, GT_FALSE);

        /* AUTODOC: mask event */
        dataIntegrityInterruptConfig(memEntryPtr->memType, CPSS_EVENT_MASK_E);
#if 0
        /* AUTODOC: Do soft reset for clean test */
        rc = prvTgfResetAndInitSystem();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfResetAndInitSystem");
#endif
    }
    /* AUTODOC: Reset counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* Print overall results */
    PRV_UTF_LOG0_MAC("\n\n\n======TEST RESULTS======\n");
    for(ii = 0; ii < memArrLen; ii++)
    {
        memEntryPtr = &memArr[ii];

        PRV_UTF_LOG3_MAC("%s expected errors: %d, got errors %d\n",
            memEntryPtr->name,
            memEntryPtr->expectedErrors,
            memEntryPtr->gotErrors
        );
        PRV_UTF_LOG3_MAC("%s expected packets: %d, got %d\n",
            memEntryPtr->name,
            memEntryPtr->expectedValidPackets,
            memEntryPtr->gotValidPackets
        );

    }
    PRV_UTF_LOG0_MAC("\n\n");
}

/**
* @internal prvTgfDiagDataIntegrityErrorInjectionRestore function
* @endinternal
*
* @brief   Restores configuration
*/
GT_VOID prvTgfDiagDataIntegrityErrorInjectionRestore
(
    GT_VOID
)
{
    GT_STATUS                     rc;

    /* AUTODOC: Disable HW errors ignoring */
    appDemoEventFatalErrorEnable(APP_DEMO_FATAL_ERROR_EXECUTE_TYPE_E);

    /* AUTODOC: Restore data integrity error handler */
    dxChDataIntegrityEventIncrementFunc = savedDataIntegrityErrorCb;

    /* AUTODOC: Delete semaphore */
    rc = cpssOsSigSemDelete(eventErrorsCame);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssOsSigSemDelete");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, VLANID_CNS);
}
