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
* @file prvTgfBrgAsicSimulationPerformance.c
*
* @brief Performance test of traffic in ASIC simulation.
*
* @version   1
********************************************************************************
*/

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/*  default VLAN Id */
#define PRV_TGF_VLANID_CNS                      2

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS               0

/* destination port fpr traffic */
#define PRV_TGF_RECV_PORT_IDX_CNS               1

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 100; /* 100 */

/* Packets to send count (multiple of 100) */
#define PACKETS_COUNT_CNS (1000)

/******************************* Test packet **********************************/
/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x02},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}                /* srcMac */
};

/* First VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                            /* pri, cfi, VlanId */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] =
{
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0F, 0x10, 0x11,
    0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
    0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21,
    0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
    0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of tagged packet */
static TGF_PACKET_PART_STC prvTgfTaggedPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* LENGTH of TAGGED packet */
#define PRV_TGF_TAGGED_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* TAGGED PACKET to send */
static TGF_PACKET_STC prvTgfTaggedPacketInfo =
{
    PRV_TGF_TAGGED_PACKET_LEN_CNS,                                     /* totalLen */
    sizeof(prvTgfTaggedPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfTaggedPacketPartArray                                        /* partsArray */
};

/**
* @internal prvTgfBrgAsicSimulationPerformanceConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfBrgAsicSimulationPerformanceConfigSet
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: create VLAN 2 with all untagged ports */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d", \
        prvTgfDevNum);

}

/**
* @internal sendPackets function
* @endinternal
*
* @brief   Sends packets an measures time
*
* @param[in] packetInfoPtr            - packet info pointer
*                                      packetount - how many packets to send
*
* @param[out] secondsPtr               - how msny seconds send process took
* @param[out] nanoSecondsPtr           - hwo many nanoseconds send process took
*                                       None
*/
static GT_VOID sendPackets(
    TGF_PACKET_STC *packetInfoPtr,
    GT_U32          packetCount,
    GT_U32         *secondsPtr,
    GT_U32         *nanoSecondsPtr
)
{
    GT_U32      startSeconds;
    GT_U32      startNanoSeconds;
    GT_U32      stopSeconds;
    GT_U32      stopNanoSeconds;
    GT_U32      ii;
    GT_STATUS   rc;

    /* AUTODOC: Setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr,
        prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth: %d, %d",
                                 prvTgfDevNum, prvTgfBurstCount, 0, NULL);

    /* AUTODOC: Start measuring time */
    cpssOsTimeRT(&startSeconds, &startNanoSeconds);

    /* AUTODOC: Send all packets */
    for(ii = packetCount / prvTgfBurstCount; ii < packetCount;
        ii += prvTgfBurstCount)
    {
        rc = prvTgfStartTransmitingEth(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
            "ERROR of StartTransmitting: %d, %d",
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    }

    /* AUTODOC: Stop measuring time */
    cpssOsTimeRT(&stopSeconds, &stopNanoSeconds);

    /* AUTODOC: Calculate elapsed time */
    if (stopNanoSeconds >= startNanoSeconds)
    {
        *nanoSecondsPtr = (stopNanoSeconds - startNanoSeconds);
        *secondsPtr = (stopSeconds - startSeconds);
    }
    else
    {
        *nanoSecondsPtr = ((1000*1000*1000) + stopNanoSeconds) -
            startNanoSeconds;
        *secondsPtr = (stopSeconds - startSeconds) - 1;
    }

}

/**
* @internal prvTgfBrgAsicSimulationPerformanceTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBrgAsicSimulationPerformanceTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                   rc;
    GT_U32                      elapsedSeconds;
    GT_U32                      elapsedNanoSeconds;
    PRV_TGF_BRG_MAC_ENTRY_STC   macEntry            = {{0}};
    TGF_MAC_ADDR                fdbEntryMacAddr     = {0};


    
    /* AUTODOC: Send configured packet from port 0 */
    sendPackets(&prvTgfTaggedPacketInfo, PACKETS_COUNT_CNS,
        &elapsedSeconds, &elapsedNanoSeconds);
    cpssOsPrintf("======================================================"
                 "==========================\n");
    cpssOsPrintf("Wire speed with UC flood: %d packets per %d.%06d seconds\n",
        PACKETS_COUNT_CNS, elapsedSeconds, elapsedNanoSeconds / (1000));
    cpssOsPrintf("======================================================"
                 "==========================\n");
 
    /* AUTODOC: Add FDB entry to avoid UC flooding */
    macEntry.key.entryType                 = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntry.dstInterface.type             = CPSS_INTERFACE_PORT_E;
    macEntry.daCommand                     = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.saCommand                     = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.isStatic                      = GT_TRUE;
    macEntry.dstInterface.devPort.hwDevNum = prvTgfDevNum;
    macEntry.key.key.macVlan.vlanId        = PRV_TGF_VLANID_CNS;
    macEntry.dstInterface.devPort.portNum  =
        prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS];
    
    /* Create FDB MAC addr */
    cpssOsMemSet(&fdbEntryMacAddr, 0, sizeof(TGF_MAC_ADDR));
    fdbEntryMacAddr[0] = prvTgfPacketL2Part.daMac[0];
    fdbEntryMacAddr[1] = prvTgfPacketL2Part.daMac[1];
    fdbEntryMacAddr[2] = prvTgfPacketL2Part.daMac[2];
    fdbEntryMacAddr[3] = prvTgfPacketL2Part.daMac[3];
    fdbEntryMacAddr[4] = prvTgfPacketL2Part.daMac[4];
    fdbEntryMacAddr[5] = prvTgfPacketL2Part.daMac[5];
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther,
        fdbEntryMacAddr, sizeof(TGF_MAC_ADDR));

    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntrySet at #%d",
        0);

    /* AUTODOC: Send configured packet from port 0 */
    sendPackets(&prvTgfTaggedPacketInfo, PACKETS_COUNT_CNS,
        &elapsedSeconds, &elapsedNanoSeconds);
    cpssOsPrintf("======================================================"
                 "==========================\n");
    cpssOsPrintf("Wire speed w/o UC flood: %d packets per %d.%06d seconds\n",
        PACKETS_COUNT_CNS, elapsedSeconds, elapsedNanoSeconds / (1000));
    cpssOsPrintf("======================================================"
                 "==========================\n");

    /* AUTODOC: Delete static FDB entry */
    rc = prvTgfBrgFdbMacEntryDelete(&macEntry.key);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryDelete");

}

/**
* @internal prvTgfBrgAsicSimulationPerformanceConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgAsicSimulationPerformanceConfigRestore
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: Invalidate entry for VLAN 2 */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate:"
        " %d", prvTgfDevNum);

}


