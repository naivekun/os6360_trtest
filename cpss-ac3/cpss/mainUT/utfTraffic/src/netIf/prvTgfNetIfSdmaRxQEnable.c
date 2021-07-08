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
* @file prvTgfNetIfSdmaRxQEnable.c
*
* @brief Rx SDMA queue enable/disable testing
*
* @version   1
********************************************************************************
*/

/* Includes */

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <utf/private/prvUtfHelpers.h>

#include <common/tgfNetIfGen.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>

#include <gtUtil/gtBmPool.h>
#include <common/tgfCommon.h>
#include <common/tgfPortGen.h>
#include <common/tgfBridgeGen.h>

#define PRV_TGF_TRAFFIC_CLASS_CNS   0
#define PRV_TGF_VLANID_CNS          2

/* traffic generation sending port */
#define ING_NETWORK_PORT_IDX_CNS    0
#define EGR_CPU_PORT_IDX_CNS        3
#define PRV_TGF_CPU_CODE_CNS        CPSS_NET_CONTROL_SRC_DST_MAC_TRAP_E

/******************************* Test packet **********************************/
/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x22},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11}                /* srcMac */
};

/* First VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                            /* pri, cfi, VlanId */
};


/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x55, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo = {
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

static PRV_TGF_NET_CPU_CODE_TABLE_ENTRY_STC           prvTgfSavedCpuCodeTableEntry;

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/
/**
* @internal prvTgfNetIfSdmaRxQEnableSendPacketAndVerify function
* @endinternal
*
* @brief   Configuration:
*         - 
*/
GT_VOID prvTgfNetIfSdmaRxQEnableSendPacketAndVerify
(
    GT_U8       tcQueue
)
{
    CPSS_DXCH_NET_SDMA_RX_COUNTERS_STC  rxCounters;
    GT_U8                               burstCount = 1;
    GT_STATUS                           rc;

    /* reset ethernet counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfEthCountersReset");

    /* clear the queue counters */
    rc = cpssDxChNetIfSdmaRxCountersGet(prvTgfDevNum, tcQueue, &rxCounters);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChNetIfSdmaRxCountersGet: %d %d\n",
            prvTgfDevNum, tcQueue);

    /* Send traffic to enabled queue*/
    rc = prvTgfTransmitPackets(prvTgfDevNum,
            prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS],
            &prvTgfPacketInfo,
            burstCount, 0/*numVfd*/, NULL/*vfdArray*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTransmitPackets");

    /* sleep for 1 sec */
    tgfSimSleep(1000);

    /* Verify packet should reach in case of enabled queue*/
    rc = cpssDxChNetIfSdmaRxCountersGet(prvTgfDevNum, tcQueue, &rxCounters);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChNetIfSdmaRxCountersGet: %d %d\n",
            prvTgfDevNum, tcQueue);

    UTF_VERIFY_EQUAL3_STRING_MAC(burstCount, rxCounters.rxInPkts,
            "Error: Expected counter [%d], Received counter [%d], Queue [%d]",
            burstCount, rxCounters.rxInPkts, tcQueue);

    /* Disable the Queue */
    rc = cpssDxChNetIfSdmaRxQueueEnable(prvTgfDevNum, tcQueue, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChNetIfSdmaRxQueueEnable");

    /* Send traffic to the disabled queue */
    rc = prvTgfTransmitPackets(prvTgfDevNum,
            prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS],
            &prvTgfPacketInfo,
            burstCount, 0/*numVfd*/, NULL/*vfdArray*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTransmitPackets");

    /* sleep for 1 sec */
    tgfSimSleep(1000);

    burstCount = 0;
    /* Verify packet should not reach in case of disabled queue*/
    rc = cpssDxChNetIfSdmaRxCountersGet(prvTgfDevNum, tcQueue, &rxCounters);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChNetIfSdmaRxCountersGet: %d %d\n",
            prvTgfDevNum, tcQueue);

    UTF_VERIFY_EQUAL3_STRING_MAC(burstCount, rxCounters.rxInPkts,
            "Error: Expected counter [%d], Received counter [%d], Queue [%d]",
            burstCount, rxCounters.rxInPkts, tcQueue);
}

/**
* @internal prvTgfNetIfSdmaRxQEnableConfigAndCheck function
* @endinternal
*
* @brief   Configuration:
*         - 
*/
GT_VOID prvTgfNetIfSdmaRxQEnableConfigAndCheck
(
    GT_VOID
)
{
    GT_U8                                   tcQueue;
    GT_STATUS                               rc;
    PRV_TGF_NET_CPU_CODE_TABLE_ENTRY_STC    cpuCodeEntry;

    /* AUTODOC: Get CPU code table entry for CPSS_NET_CONTROL_SRC_DST_MAC_TRAP_E */
    cpssOsMemSet(&cpuCodeEntry, 0, sizeof(cpuCodeEntry));
    rc = prvTgfNetIfCpuCodeTableGet(prvTgfDevNum,
            PRV_TGF_CPU_CODE_CNS, &cpuCodeEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfNetIfCpuCodeTableGet %d", prvTgfDevNum);

    /* AUTODOC: save prvTgfSavedCpuCodeTableEntry */
    prvTgfSavedCpuCodeTableEntry = cpuCodeEntry;

    for(tcQueue = 0; tcQueue <= 7; tcQueue++)
    {
        cpuCodeEntry.tc = tcQueue;
        rc = prvTgfNetIfCpuCodeTableSet(prvTgfDevNum,
                PRV_TGF_CPU_CODE_CNS, &cpuCodeEntry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfNetIfCpuCodeTableSet %d", prvTgfDevNum);

        prvTgfNetIfSdmaRxQEnableSendPacketAndVerify(tcQueue);
    }
}

/**
* @internal prvTgfNetIfSdmaRxQEnableInit function
* @endinternal
*
* @brief   Configuration:
*         - 
*/

GT_VOID prvTgfNetIfSdmaRxQEnableInit
(
    GT_VOID
)
{
    GT_STATUS                   rc;
    PRV_TGF_BRG_MAC_ENTRY_STC   macEntry;

    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    macEntry.key.key.macVlan.vlanId         = PRV_TGF_VLANID_CNS;
    macEntry.key.entryType                  = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntry.isStatic                       = GT_TRUE;
    macEntry.dstInterface.type              = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum  = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum   = prvTgfPortsArray[EGR_CPU_PORT_IDX_CNS];
    macEntry.daCommand                      = PRV_TGF_PACKET_CMD_TRAP_TO_CPU_E;

    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther, prvTgfPacketL2Part.daMac, 6);

    /* AUTODOC: create VLAN 2 with all ports */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d", prvTgfDevNum);

    /* AUTODOC: write FDB entry */
    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(rc, GT_OK, "Error in prvTgfBrgFdbMacEntrySet %d", prvTgfDevNum);
}

/**
* @internal prvTgfNetIfSdmaRxQEnableRestore function
* @endinternal
*
* @brief   Restore configuration:
*         - Delete TX buffers pool
*         - Restore masked state of events
*         - Restore enabled state of tx queue
*         - Restore default handler
*/
GT_VOID prvTgfNetIfSdmaRxQEnableRestore
(
    GT_VOID
)
{
    GT_U8       tcQueue;
    GT_STATUS                   rc;

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d", prvTgfDevNum);

    /* AUTODOC: Restore prvTgfSavedCpuCodeTableEntry */
    rc = prvTgfNetIfCpuCodeTableSet(prvTgfDevNum,
            PRV_TGF_CPU_CODE_CNS, &prvTgfSavedCpuCodeTableEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfNetIfCpuCodeTableSet %d", prvTgfDevNum);

    /*AUTODOC: Restore Queue status */
    for (tcQueue = 0; tcQueue <= 7; tcQueue++)
    {
        rc = cpssDxChNetIfSdmaRxQueueEnable(prvTgfDevNum, tcQueue, GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChNetIfSdmaRxQueueEnable");
    }

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d", GT_TRUE);
}

/**
* @internal prvTgfNetIfTxSdmaRxQEnableTest function
* @endinternal
*
* @brief   Rx SDMA queue enable and disable test.
*/
GT_VOID prvTgfNetIfSdmaRxQEnableTest
(
    GT_VOID
)
{
    prvTgfNetIfSdmaRxQEnableInit();

    prvTgfNetIfSdmaRxQEnableConfigAndCheck();

    prvTgfNetIfSdmaRxQEnableRestore();
}

