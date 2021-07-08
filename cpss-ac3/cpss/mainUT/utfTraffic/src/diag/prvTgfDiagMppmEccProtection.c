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
* @file prvTgfDiagMppmEccProtection.c
*
* @brief Test for buffer memory (MPPM) ECC protection check
*
* @version   1
********************************************************************************
*/

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <extUtils/trafficEngine/tgfTrafficEngine.h>
#include <extUtils/trafficEngine/tgfTrafficGenerator.h>
#include <extUtils/trafficEngine/tgfTrafficTable.h>

#include <common/tgfBridgeGen.h>
#include <common/tgfPortGen.h>
#include <common/tgfNetIfGen.h>

#include <gtUtil/gtBmPool.h>

#include <cpss/dxCh/dxChxGen/diag/cpssDxChDiagDataIntegrity.h>

#define VLAN_ID_CNS                 (3)
#define JUMBO_FRAME_SIZE_CNS        (10240) /* 10K */
#define SEND_PACKET_SIZE_CNS        (10208)
#define PRV_TGF_DEF_MRU_VALUE_CNS   (1518)
#define PACKET_BUFFER_SIZE_CNS      (256)
#define BUFFERS_PER_JUMBO_FRAME_CNS (JUMBO_FRAME_SIZE_CNS / PACKET_BUFFER_SIZE_CNS)
#define WAIT_TIMEOUT_CNS            (10000)
#define SENDER_PORT                 prvTgfPortsArray[1]


/* total nuber of buffers in the device */
#define DEV_BUFFERS_NUM_MAC(devNum)                                     \
    (PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.bufferMemory /  \
        (8 * PACKET_BUFFER_SIZE_CNS))

/* how many packets need to be sent to utilize all buffers?
   We use Jumbo packets (10K).Each Jumbo allocates 10K/256 buffers.
   The UT configures UC bridging. In case of bobcat2 each “send” packet
   uses  2 x  (10K/256) buffers :
   - first one is from CPU port to ingress port.
   - second is UC bridged packet. So test need send
   <total buffers number> / (2 x (10K/256)) packets.
   In case of other SIP5 devices packet will be rejected in the ingress.
   So need to twice more packets */
#define NUM_PACKETS_TO_SEND_MAC(devNum)                                 \
 (PRV_CPSS_DXCH_BOBCAT2_CHECK_MAC(devNum) ?                             \
      DEV_BUFFERS_NUM_MAC(devNum) / BUFFERS_PER_JUMBO_FRAME_CNS / 2 :      \
      DEV_BUFFERS_NUM_MAC(devNum) / BUFFERS_PER_JUMBO_FRAME_CNS)


/* Restore Values */
static GT_U32          savedMruPort0;
static GT_U32          savedMruPort1;
/*
 * Packet Information
 */
/* L2 part of unknown UC packet */
static TGF_PACKET_L2_STC prvTgfUcPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x88}  /* srcMac */
};

static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS, /* etherType */
    0,  0,  VLAN_ID_CNS                  /* pri, cfi, VlanId */
};

static GT_U8 prvTgfPayloadDataArr[SEND_PACKET_SIZE_CNS] =
{
    0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55
};

static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPayloadDataArr),   /* dataLength */
    prvTgfPayloadDataArr            /* dataPtr */
};

static TGF_PACKET_PART_STC prvTgfUcPcktArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfUcPacketL2Part}, /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

#define PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + \
    sizeof(prvTgfPayloadDataArr)

#define PACKET_CRC_LEN_CNS (PACKET_LEN_CNS + TGF_CRC_LEN_CNS)

static TGF_PACKET_STC prvTgfGlobalPacketInfo = {
    PACKET_LEN_CNS,
    sizeof(prvTgfUcPcktArray) / sizeof(TGF_PACKET_PART_STC),
    prvTgfUcPcktArray
};

/* Event counter and semaphore signaling we have all needed events */
static GT_U32          evMppmCtr = 0;
static CPSS_OS_SIG_SEM eventMppmCame = CPSS_OS_SEMB_EMPTY_E;

/* Callback function prototype for event counting */
typedef GT_STATUS DXCH_DATA_INTEGRITY_EVENT_CB_FUNC(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC  *eventPtr
);

extern DXCH_DATA_INTEGRITY_EVENT_CB_FUNC *dxChDataIntegrityEventIncrementFunc;
static DXCH_DATA_INTEGRITY_EVENT_CB_FUNC *savedDataIntegrityErrorCb;


/**
* @internal mppmErrorHandler function
* @endinternal
*
* @brief   Callback called in case of data integrity error in MPPM.
*
* @param[in] dev                      - the device.
* @param[in] eventPtr                 - filled structure which describes data integrity error event
*/
static GT_STATUS mppmErrorHandler
(
    IN GT_U8                                     dev,
    IN CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC  *eventPtr

)
{

    GT_U32 expectedNumOfEvents = PRV_CPSS_DXCH_BOBCAT2_CHECK_MAC(dev) ?
        NUM_PACKETS_TO_SEND_MAC(dev) * 2 :
        NUM_PACKETS_TO_SEND_MAC(dev);

    /* Print debug info */
    PRV_UTF_LOG0_MAC("======DATA INTEGRITY ERROR EVENT======\n");
    PRV_UTF_LOG1_MAC("Device:      %d\n", dev);
    PRV_UTF_LOG1_MAC("Event type:  %d\n", eventPtr->eventsType);
    PRV_UTF_LOG1_MAC("Failed row: %d\n",
        eventPtr->location.ramEntryInfo.ramRow);
    PRV_UTF_LOG1_MAC("RAM memType: %d\n",
        eventPtr->location.ramEntryInfo.memType);

    PRV_UTF_LOG1_MAC("isMppmInfoValid:  %d\n",
        eventPtr->location.isMppmInfoValid);
    if (eventPtr->location.isMppmInfoValid)
    {
        PRV_UTF_LOG1_MAC("Portgroup ID:  %d\n",
            eventPtr->location.mppmMemLocation.portGroupId);
        PRV_UTF_LOG1_MAC("MPPM ID:  %d\n",
            eventPtr->location.mppmMemLocation.mppmId);
        PRV_UTF_LOG1_MAC("Bank ID:  %d\n",
            eventPtr->location.mppmMemLocation.bankId);
    }

    /* each packet generates both single and multiple ECC errors */
    if((dev == prvTgfDevNum) &&
       (eventPtr->location.isMppmInfoValid == GT_TRUE) &&
       (eventPtr->location.ramEntryInfo.memType ==
            CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_BANK_RAM_E)
      )
    {
        evMppmCtr++;
        if(evMppmCtr == expectedNumOfEvents)
        {
            cpssOsSigSemSignal(eventMppmCame);
        }
    }

    return GT_OK;
}

/**
* @internal prvTgfDiagMppmEccProtectionConfigurationSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfDiagMppmEccProtectionConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U8       tagArray[] = {1,1};
    GT_U32      portArray[2];
    GT_U8       profileIter;

    portArray[0] = prvTgfPortsArray[0];
    portArray[1] = SENDER_PORT;

    /* AUTODOC: create VLAN */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(VLAN_ID_CNS, portArray, NULL,
        tagArray, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet:"
    " dev=[%d]", prvTgfDevNum);

    /* AUTODOC: Get default MRU for receiving port */
    rc = prvTgfPortMruGet(prvTgfPortsArray[0], &savedMruPort0);
    UTF_VERIFY_EQUAL1_STRING_MAC(rc, GT_OK, "prvTgfPortMruGet: "
    "port=[%d]", prvTgfPortsArray[0]);

    /* AUTODOC: set MRU value 10240 for receiving port */
    rc = prvTgfPortMruSet(prvTgfPortsArray[0], JUMBO_FRAME_SIZE_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPortMruSet: "
        "port=[%d], MRU=[%d]", prvTgfPortsArray[0], JUMBO_FRAME_SIZE_CNS);

    /* AUTODOC: Get default MRU for sending port */
    rc = prvTgfPortMruGet(SENDER_PORT, &savedMruPort1);
    UTF_VERIFY_EQUAL1_STRING_MAC(rc, GT_OK, "prvTgfPortMruGet: "
    "port=[%d]", SENDER_PORT);

    /* AUTODOC: set MRU value 10240 for sending port */
    rc = prvTgfPortMruSet(prvTgfPortsArray[1], JUMBO_FRAME_SIZE_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPortMruSet: "
        "port=[%d], MRU=[%d]", prvTgfPortsArray[1], JUMBO_FRAME_SIZE_CNS);

    /* AUTODOC: Create semaphore to wait all MPPM errors found */
    rc = cpssOsSigSemBinCreate("eventMppmCame", CPSS_OS_SEMB_EMPTY_E,
        &eventMppmCame);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssOsSigSemBinCreate");

    /* AUTODOC: Save DI error callback and set own */
    savedDataIntegrityErrorCb = dxChDataIntegrityEventIncrementFunc;
    dxChDataIntegrityEventIncrementFunc = mppmErrorHandler;

    /* AUTODOC: set MRU for each profiles to 10240) */
    for (profileIter = 0 ; profileIter < 8 ; profileIter++ )
    {
        rc = prvTgfBrgVlanMruProfileValueSet(profileIter,
            JUMBO_FRAME_SIZE_CNS);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgVlanMruProfileValueSet: profileIter=[%d]", profileIter);
    }

}

/**
* @internal prvTgfDiagMppmEccProtectionTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfDiagMppmEccProtectionTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                 rc;
    GT_U32                    ii;
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC memoryLocation;

    /* AUTODOC: Reset counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* AUTODOC: Setup memory type to test (MPPM) */
    memoryLocation.portGroupsBmp = 0xFFFFFFFF;
    memoryLocation.type = CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE;
    memoryLocation.info.ramEntryInfo.memType =
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_BANK_RAM_E;

    /* AUTODOC: reset number of MPPM events */
    evMppmCtr = 0;

    rc = cpssDxChDiagDataIntegrityEventMaskSet(
        prvTgfDevNum,
        &memoryLocation,
        CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E,
        CPSS_EVENT_UNMASK_E
    );
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
        "cpssDxChDiagDataIntegrityEventMaskSet: "
        "dev=[%d],  eccMode=[%d], unmaskEn=[%d]",
        prvTgfDevNum,
        CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E,
        CPSS_EVENT_UNMASK_E
    );

    /* AUTODOC: Send packets */
    for(ii = NUM_PACKETS_TO_SEND_MAC(prvTgfDevNum); ii > 0; ii--)
    {
        /* AUTODOC: Enable error injection for MPPM */
        rc = cpssDxChDiagDataIntegrityErrorInjectionConfigSet(
            prvTgfDevNum,
            &memoryLocation,
            CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E,
            GT_TRUE
        );
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
            "cpssDxChDiagDataIntegrityErrorInjectionConfigGet: "
            "dev=[%d],  injectMode=[%d], injectEn=[%d]",
            prvTgfDevNum,
            CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E,
            GT_TRUE
        );

        /* Send */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfGlobalPacketInfo,
            1, 0, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth: %d, %d",
            prvTgfDevNum);

        /* Do not print packets */
        tgfTrafficTracePacketByteSet(GT_FALSE);

        rc = prvTgfStartTransmitingEth(prvTgfDevNum, SENDER_PORT);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth: "
            "%d, %d\n", prvTgfDevNum, SENDER_PORT);

        /* AUTODOC: Check counters */
#if 0 /* Debug output */
        {
            CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
            rc = prvTgfReadPortCountersEth(prvTgfDevNum, SENDER_PORT, GT_TRUE,
                &portCntrs);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                "prvTgfReadPortCountersEth: dev=[%d], port=[%d]",
                prvTgfDevNum, SENDER_PORT);

            rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[0],
                GT_TRUE, &portCntrs);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                "prvTgfReadPortCountersEth: dev=[%d], port=[%d]",
                prvTgfDevNum, prvTgfPortsArray[0]);
        }
#endif
        /* provide time for events handling */
        cpssOsTimerWkAfter(10);
    }

    /* Enable back printing packets */
    tgfTrafficTracePacketByteSet(GT_TRUE);

    /* AUTODOC: Disable error injection for MPPM */
    rc = cpssDxChDiagDataIntegrityErrorInjectionConfigSet(
        prvTgfDevNum,
        &memoryLocation,
        CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E,
        GT_FALSE
    );
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
        "cpssDxChDiagDataIntegrityErrorInjectionConfigGet: "
        "dev=[%d],  injectMode=[%d], injectEn=[%d]",
        prvTgfDevNum,
        CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E,
        GT_FALSE
    );

    /* AUTODOC: Wait for MPPM error events to come */
    rc = cpssOsSigSemWait(eventMppmCame, WAIT_TIMEOUT_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "cpssOsSigSemWait. Came %d MPPM error events of %d",
        evMppmCtr,
        (PRV_CPSS_DXCH_BOBCAT2_CHECK_MAC(prvTgfDevNum) ?
         NUM_PACKETS_TO_SEND_MAC(prvTgfDevNum) * 2 :
         NUM_PACKETS_TO_SEND_MAC(prvTgfDevNum)));
}

/**
* @internal prvTgfDiagMppmEccProtectionRestore function
* @endinternal
*
* @brief   Restore saved configuration
*/
GT_VOID prvTgfDiagMppmEccProtectionRestore
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U8     profileIter;

    /* AUTODOC: Restore DI error callback */
    dxChDataIntegrityEventIncrementFunc = savedDataIntegrityErrorCb;

    /* AUTODOC: Delete semaphore */
    rc = cpssOsSigSemDelete(eventMppmCame);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssOsSigSemDelete");

    /* AUTODOC: restore MRU for each profiles to 10240) */
    for (profileIter = 0 ; profileIter < 8 ; profileIter++ )
    {
        rc = prvTgfBrgVlanMruProfileValueSet(profileIter,
            PRV_TGF_DEF_MRU_VALUE_CNS);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "prvTgfBrgVlanMruProfileValueSet");
    }

    /* AUTODOC: restore MRU values for ports*/
    rc = prvTgfPortMruSet(prvTgfPortsArray[0], savedMruPort0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPortMruSet: "
        "port=[%d], MRU=[%d]", prvTgfPortsArray[0], savedMruPort0);
    rc = prvTgfPortMruSet(SENDER_PORT, savedMruPort1);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPortMruSet: "
        "port=[%d], MRU=[%d]", SENDER_PORT, savedMruPort1);

    /* AUTODOC: invalidate VLAN */
    rc = prvTgfBrgDefVlanEntryInvalidate(VLAN_ID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: "
        "dev=[%d], VLAN id=[%d]", prvTgfDevNum, VLAN_ID_CNS);

}

