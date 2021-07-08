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
* @file prvTgfTaggedUntaggedComb.c
*
* @brief L2 MLL Tagged\Untagged for Ingress\Egress combinations test
*
* @version   5
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfL2MllGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>

#include <l2mll/prvTgfL2MllUtils.h>
#include <l2mll/prvTgfTaggedUntaggedComb.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN Id */
#define PRV_TGF_VLANID_CNS  1

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS     3

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 1;

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x01, 0x00, 0x00, 0x00, 0x00, 0x01},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x99}                /* srcMac */
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

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS  PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo = {
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/* PARTS of UNTAGGED packet */
static TGF_PACKET_PART_STC prvTgfUntaggedPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* LENGTH of UNTAGGED packet */
#define PRV_TGF_UNTAGGED_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* LENGTH of UNTAGGED packet with CRC */
#define PRV_TGF_UNTAGGED_PACKET_CRC_LEN_CNS  PRV_TGF_UNTAGGED_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* PACKET to send */
static TGF_PACKET_STC prvTgfUntaggedPacketInfo = {
    PRV_TGF_UNTAGGED_PACKET_LEN_CNS,                                     /* totalLen */
    sizeof(prvTgfUntaggedPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfUntaggedPacketPartArray                                        /* partsArray */
};

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/


/**
* @internal prvTgfTaggedUntaggedCombConfigurationSet function
* @endinternal
*
* @brief   Set configuration.
*/
GT_VOID prvTgfTaggedUntaggedCombConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc;
    PRV_TGF_L2_MLL_LTT_ENTRY_STC   lttEntry;

    PRV_TGF_PAIR_READ_WRITE_FORM_ENT     mllPairWriteForm;
    PRV_TGF_L2_MLL_PAIR_STC              mllPairEntry;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* Save L2 MLL configurations */
    prvTgfL2MllUtilsGlobalConfigurationGetAndSave();

    /* AUTODOC: add FDB entry with MAC 01:00:00:00:00:01, VLAN 1, eVIDX 4096 */
    rc = prvTgfBrgDefFdbMacEntryOnVidxSet(prvTgfPacketL2Part.daMac, PRV_TGF_VLANID_CNS,
                                          4096, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnVidxSet: eVidx 4096");

    /* create L2 MLL LTT entry */
    cpssOsMemSet(&lttEntry, 0, sizeof(PRV_TGF_L2_MLL_LTT_ENTRY_STC));

    lttEntry.mllPointer = 0;
    lttEntry.entrySelector = 0;

    /* AUTODOC: add L2 MLL LTT entry 0 with: */
    /* AUTODOC:   mllPointer=0, mllMaskProfileEnable=False, mllMaskProfile=0 */
    rc = prvTgfL2MllLttEntrySet(prvTgfDevNum, 0, &lttEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllLttEntrySet: index 0");

    /* create L2 MLL entry */
    cpssOsMemSet(&mllPairEntry, 0, sizeof(PRV_TGF_L2_MLL_PAIR_STC));

    mllPairWriteForm = PRV_TGF_PAIR_READ_WRITE_FIRST_ONLY_E;

    mllPairEntry.nextPointer = 100;
    mllPairEntry.entrySelector = PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;
    mllPairEntry.firstMllNode.last = GT_TRUE;
    mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_PORT_E;
    mllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum = prvTgfDevNum;
    mllPairEntry.firstMllNode.egressInterface.devPort.portNum = prvTgfPortsArray[1];

    /* AUTODOC: add L2 MLL entry 0: */
    /* AUTODOC:   nextMllPointer=100, port=1, last=True */
    rc = prvTgfL2MllPairWrite(prvTgfDevNum, 0, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairWrite: index 0");
}

/**
* @internal prvTgfTaggedUntaggedCombTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfTaggedUntaggedCombTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    portIter;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    GT_U32    expectedTxPacket;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* AUTODOC: disable MLL lookup for all multi-target packets */
    rc = prvTgfL2MllLookupForAllEvidxEnableSet(prvTgfDevNum, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllLookupForAllEvidxEnableSet: enable %d",
                                 GT_FALSE);

    /* Tagged(I) to Untagged(E) */

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* AUTODOC: send Tagged Ethernet packet from port 3 with: */
    /* AUTODOC:   DA=01:00:00:00:00:01, SA=00:00:00:00:00:99, VID=1 */
    prvTgfL2MllUtilsPacketSend(&prvTgfPacketInfo, prvTgfBurstCount, PRV_TGF_SEND_PORT_IDX_CNS);

    /* AUTODOC: verify Ethernet packet on ports [1(UnTagged),3] (based on eVidx 4096 MLL) */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* check Rx counters - Tx from port[1] (based on eVidx 4096 MLL) & from port[3] (the tester) */
        if (1 == portIter || PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            expectedTxPacket = 1;
        }
        else
        {
            expectedTxPacket = 0;
        }

        UTF_VERIFY_EQUAL2_STRING_MAC(expectedTxPacket, portCntrs.goodPktsSent.l[0],
                                     "get another Tx packets than expected on port[%d] = %d",
                                     portIter, prvTgfPortsArray[portIter]);

        /* Tagged(I) to Untagged(E) */
        if(1 == portIter)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(PRV_TGF_UNTAGGED_PACKET_CRC_LEN_CNS,
                                         portCntrs.goodOctetsSent.l[0],
                                         "get another Tx octets than expected on port[%d] = %d",
                                         portIter, prvTgfPortsArray[portIter]);
        }
    }

    /* Untagged(I) to Untagged(E) */

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* AUTODOC: send UnTagged Ethernet packet from port 3 with: */
    /* AUTODOC:   DA=01:00:00:00:00:01, SA=00:00:00:00:00:99 */
    prvTgfL2MllUtilsPacketSend(&prvTgfUntaggedPacketInfo, prvTgfBurstCount, PRV_TGF_SEND_PORT_IDX_CNS);

    /* AUTODOC: verify Ethernet packet on ports [1(UnTagged),3] (based on eVidx 4096 MLL) */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* check Rx counters - Tx from port[1] (based on eVidx 4096 MLL) & from port[3] (the tester) */
        if (1 == portIter || PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            expectedTxPacket = 1;
        }
        else
        {
            expectedTxPacket = 0;
        }

        UTF_VERIFY_EQUAL2_STRING_MAC(expectedTxPacket, portCntrs.goodPktsSent.l[0],
                                     "get another Tx packets than expected on port[%d] = %d",
                                     portIter, prvTgfPortsArray[portIter]);

        /* Untagged(I) to Untagged(E) */
        if(1 == portIter)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(PRV_TGF_UNTAGGED_PACKET_CRC_LEN_CNS,
                                         portCntrs.goodOctetsSent.l[0],
                                         "get another Tx octets than expected on port[%d] = %d",
                                         portIter, prvTgfPortsArray[portIter]);
        }
    }

    /* AUTODOC: add Tagged port 1 to VLAN 1 */
    rc = prvTgfBrgVlanMemberAdd(prvTgfDevNum, 1, prvTgfPortsArray[1], GT_TRUE);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberAdd: %d, %d, %d, %d",
                                 prvTgfDevNum, 1, prvTgfPortsArray[1], GT_TRUE);

    /* Tagged(I) to Tagged(E) */

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* AUTODOC: send Tagged Ethernet packet from port 3 with: */
    /* AUTODOC:   DA=01:00:00:00:00:01, SA=00:00:00:00:00:99, VID=1 */
    prvTgfL2MllUtilsPacketSend(&prvTgfPacketInfo, prvTgfBurstCount, PRV_TGF_SEND_PORT_IDX_CNS);

    /* AUTODOC: verify Ethernet packet on ports [1(Tagged),3] (based on eVidx 4096 MLL) */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* check Rx counters - Tx from port[1] (based on eVidx 4096 MLL) & from port[3] (the tester) */
        if (1 == portIter || PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            expectedTxPacket = 1;
        }
        else
        {
            expectedTxPacket = 0;
        }

        UTF_VERIFY_EQUAL2_STRING_MAC(expectedTxPacket, portCntrs.goodPktsSent.l[0],
                                     "get another Tx packets than expected on port[%d] = %d",
                                     portIter, prvTgfPortsArray[portIter]);

        /* Tagged(I) to Tagged(E) */
        if(1 == portIter)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(PRV_TGF_PACKET_CRC_LEN_CNS,
                                         portCntrs.goodOctetsSent.l[0],
                                         "get another Tx octets than expected on port[%d] = %d",
                                         portIter, prvTgfPortsArray[portIter]);
        }
    }

    /* Untagged(I) to Tagged(E) */

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* AUTODOC: send UnTagged Ethernet packet from port 3 with: */
    /* AUTODOC:   DA=01:00:00:00:00:01, SA=00:00:00:00:00:99 */
    prvTgfL2MllUtilsPacketSend(&prvTgfUntaggedPacketInfo, prvTgfBurstCount, PRV_TGF_SEND_PORT_IDX_CNS);

    /* AUTODOC: verify Ethernet packet on ports [1(Tagged),3] (based on eVidx 4096 MLL) */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* check Rx counters - Tx from port[1] (based on eVidx 4096 MLL) & from port[3] (the tester) */
        if (1 == portIter || PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            expectedTxPacket = 1;
        }
        else
        {
            expectedTxPacket = 0;
        }

        UTF_VERIFY_EQUAL2_STRING_MAC(expectedTxPacket, portCntrs.goodPktsSent.l[0],
                                     "get another Tx packets than expected on port[%d] = %d",
                                     portIter, prvTgfPortsArray[portIter]);

        /* Untagged(I) to Tagged(E) */
        if(1 == portIter)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(PRV_TGF_PACKET_CRC_LEN_CNS,
                                         portCntrs.goodOctetsSent.l[0],
                                         "get another Tx octets than expected on port[%d] = %d",
                                         portIter, prvTgfPortsArray[portIter]);
        }
    }

    /* AUTODOC: restore UnTagged port 1 to VLAN 1 */
    rc = prvTgfBrgVlanMemberAdd(prvTgfDevNum, 1, prvTgfPortsArray[1], GT_FALSE);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberAdd: %d, %d, %d, %d",
                                 prvTgfDevNum, 1, prvTgfPortsArray[1], GT_FALSE);

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();
}

/**
* @internal prvTgfTaggedUntaggedCombConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration.
*/
GT_VOID prvTgfTaggedUntaggedCombConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    lttIndexArray[1];
    GT_U32    mllIndexArray[1];

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    lttIndexArray[0] = 0;
    mllIndexArray[0] = 0;

    /* AUTODOC: clear L2 MLL and LTT entries */
    prvTgfL2MllUtilsMllAndLttEntriesClear(lttIndexArray, 1, mllIndexArray, 1);

    /* AUTODOC: restore default L2 MLL configurations */
    prvTgfL2MllUtilsGlobalConfigurationRestore();
}


