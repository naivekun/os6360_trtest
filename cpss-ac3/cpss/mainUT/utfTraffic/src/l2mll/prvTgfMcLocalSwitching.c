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
* @file prvTgfMcLocalSwitching.c
*
* @brief L2 MLL MC local switching test
*
* @version   9
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
#include <l2mll/prvTgfMcLocalSwitching.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN Id */
#define PRV_TGF_VLANID_CNS  1

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS     2

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

/* errata flag (FE-6932750)
   L2 MLL Out Multicast Packets Counter do not count non IP packets */
static GT_BOOL  l2MllOutMcPcktCntErrata   = GT_FALSE;

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/


/**
* @internal prvTgfMcLocalSwitchingConfigurationSet function
* @endinternal
*
* @brief   Set configuration.
*/
GT_VOID prvTgfMcLocalSwitchingConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc;
    PRV_TGF_L2_MLL_LTT_ENTRY_STC   lttEntry;

    PRV_TGF_PAIR_READ_WRITE_FORM_ENT    mllPairWriteForm;
    PRV_TGF_L2_MLL_PAIR_STC             mllPairEntry;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* Save L2 MLL configurations */
    prvTgfL2MllUtilsGlobalConfigurationGetAndSave();

    /* AUTODOC: add FDB entry with MAC 01:00:00:00:00:01, VLAN 1, eVIDX 0x1FFF */
    rc = prvTgfBrgDefFdbMacEntryOnVidxSet(prvTgfPacketL2Part.daMac, PRV_TGF_VLANID_CNS,
                                          0x1FFF, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnVidxSet: eVidx 0x1FFF");

    /* create L2 MLL LTT entry */
    cpssOsMemSet(&lttEntry, 0, sizeof(PRV_TGF_L2_MLL_LTT_ENTRY_STC));

    lttEntry.mllPointer = 10;
    lttEntry.entrySelector = 0;

    /* AUTODOC: add L2 MLL LTT entry 0x1FFF with: */
    /* AUTODOC:   mllPointer=10, mllMaskProfileEnable=False, mllMaskProfile=0 */
    rc = prvTgfL2MllLttEntrySet(prvTgfDevNum, 0x1FFF, &lttEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllLttEntrySet: index 0x1FFF");

    /* create L2 MLL entries */
    cpssOsMemSet(&mllPairEntry, 0, sizeof(PRV_TGF_L2_MLL_PAIR_STC));

    mllPairWriteForm = PRV_TGF_PAIR_READ_WRITE_WHOLE_E;

    mllPairEntry.nextPointer = 0;
    mllPairEntry.entrySelector = PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;

    mllPairEntry.firstMllNode.last = GT_FALSE;
    mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_PORT_E;
    mllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum = prvTgfDevNum;
    mllPairEntry.firstMllNode.egressInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];

    mllPairEntry.secondMllNode.last = GT_TRUE;
    mllPairEntry.secondMllNode.egressInterface.type = CPSS_INTERFACE_PORT_E;
    mllPairEntry.secondMllNode.egressInterface.devPort.hwDevNum = prvTgfDevNum;
    mllPairEntry.secondMllNode.egressInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];

    /* AUTODOC: add L2 MLL entry 10: */
    /* AUTODOC:   nextMllPointer=11, port=1 */
    rc = prvTgfL2MllPairWrite(prvTgfDevNum, 10, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairWrite: index 10");

    if(UTF_CPSS_SIP5_0_ONLY_CHECK_MAC(prvTgfDevNum))
    {
        /*  due to an BC2 A0 errata FE-6932750 the
           "L2 MLL Out Multicast Packets Counter" do not count non IP packets */
        l2MllOutMcPcktCntErrata = GT_TRUE;
    }
}

/**
* @internal prvTgfMcLocalSwitchingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfMcLocalSwitchingTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    portIter;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    GT_U32    expectedTxPacket;

    PRV_TGF_PAIR_READ_WRITE_FORM_ENT    mllPairWriteForm;
    PRV_TGF_L2_MLL_PAIR_STC             mllPairEntry;

    GT_U32                          mllOutMCPkts;
    GT_U32                          oldMllOutMCPktsSet0;
    GT_U32                          oldMllOutMCPktsSet1;
    GT_U32                          mllCntSet_0 = 0, mllCntSet_1 = 1;
    PRV_TGF_L2MLL_COUNTER_SET_INTERFACE_CFG_STC cntIntCfg;


    /* AUTODOC: GENERATE TRAFFIC: */
    /* the first 2 tests are with disable local switching: 
      1. both mll entrys have same out interface that is ingress port 36 . the replication will be dropped by local switching.
      2. first entry port is 12, second entry port is 36.
      the replication to the first entry should not filtered and replication to the second entry should be filtered by local switching */
    /* AUTODOC: enable MLL lookup for all multi-target packets */
    rc = prvTgfL2MllLookupForAllEvidxEnableSet(prvTgfDevNum, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllLookupForAllEvidxEnableSet: enable %d",
                                 GT_TRUE);

     /* AUTODOC: get MLL counters for MC subscriber 1 - clear on read */
    rc = prvTgfL2MllMcCntGet(prvTgfDevNum, mllCntSet_0, &oldMllOutMCPktsSet0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllCntGet");
    PRV_UTF_LOG1_MAC("oldMllOutMCPktsSet0: oldMllOutMCPktsSet0 = %d\n", oldMllOutMCPktsSet0);
    /* AUTODOC: get MLL counters for MC subscriber 2 - clear on read */
    rc = prvTgfL2MllMcCntGet(prvTgfDevNum, mllCntSet_1, &oldMllOutMCPktsSet1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllCntGet");
    PRV_UTF_LOG1_MAC("oldMllOutMCPktsSet1: oldMllOutMCPktsSet1 = %d\n", oldMllOutMCPktsSet1);

    /* AUTODOC: set MLL counters mode for MC subscriber 1 */
    cpssOsMemSet(&cntIntCfg, 0, sizeof(cntIntCfg));
    cntIntCfg.ipMode           = CPSS_IP_PROTOCOL_IPV4V6_E;
    cntIntCfg.devNum           = prvTgfDevNum;
    cntIntCfg.portTrunkCntMode = PRV_TGF_L2MLL_PORT_CNT_MODE_E;
    cntIntCfg.portTrunk.port   = prvTgfPortsArray[1];

    rc = prvTgfL2MllSetCntInterfaceCfg(mllCntSet_0, &cntIntCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllSetCntInterfaceCfg");

    /* AUTODOC: set MLL counters mode for MC subscriber 2 */
    cntIntCfg.portTrunk.port   = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];

    rc = prvTgfL2MllSetCntInterfaceCfg(mllCntSet_1, &cntIntCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllSetCntInterfaceCfg");

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* AUTODOC: send Ethernet packet from port 2 with: */
    /* AUTODOC:   DA=01:00:00:00:00:01, SA=00:00:00:00:00:99, VID=1 */
    prvTgfL2MllUtilsPacketSend(&prvTgfPacketInfo, prvTgfBurstCount, PRV_TGF_SEND_PORT_IDX_CNS);

    /* AUTODOC: get MLL counters for MC subscriber 1 - expect mllOutMCPkts = 0 because all replication should be filtered due to local switching */
    rc = prvTgfL2MllMcCntGet(prvTgfDevNum, mllCntSet_0, &mllOutMCPkts);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllCntGet");
    PRV_UTF_LOG1_MAC("MC subscriber 1: mllOutMCPkts = %d\n", mllOutMCPkts);
    UTF_VERIFY_EQUAL2_STRING_MAC(0, mllOutMCPkts,
                                    "get wrong mllOutMCPkts on port[%d] = %d",
                                     1, prvTgfPortsArray[1]);

    /* AUTODOC: get MLL counters for MC subscriber 2 - expect mllOutMCPkts = 0 because all replication should be filtered due to local switching */
    rc = prvTgfL2MllMcCntGet(prvTgfDevNum, mllCntSet_1, &mllOutMCPkts);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllCntGet");
    PRV_UTF_LOG1_MAC("MC subscriber 2: mllOutMCPkts = %d\n", mllOutMCPkts);
    UTF_VERIFY_EQUAL2_STRING_MAC(0, mllOutMCPkts, 
                                        "get wrong mllOutMCPkts on port[%d] = %d",
                                         PRV_TGF_SEND_PORT_IDX_CNS, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    
    /* change first entry port to get un filter replication in MC local switching */
    mllPairWriteForm = PRV_TGF_PAIR_READ_WRITE_FIRST_ONLY_E;

    rc = prvTgfL2MllPairRead(prvTgfDevNum, 10, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairRead: index 10 - firstMllNode");

    mllPairEntry.firstMllNode.egressInterface.devPort.portNum = prvTgfPortsArray[1];

    /* AUTODOC: update L2 MLL entry 10 - firstMllNode */
    rc = prvTgfL2MllPairWrite(prvTgfDevNum, 10, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairWrite: index 10 - firstMllNode");

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* AUTODOC: send Ethernet packet from port 2 with: */
    /* AUTODOC:   DA=01:00:00:00:00:01, SA=00:00:00:00:00:99, VID=1 */
    prvTgfL2MllUtilsPacketSend(&prvTgfPacketInfo, prvTgfBurstCount, PRV_TGF_SEND_PORT_IDX_CNS);

    /* AUTODOC: verify Ethernet packet on ports [1,2] */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* check Rx counters - Tx from port[1] (based on eVidx 0x1FFF MLL) & from port[2] (the tester) */
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
    }

    /* AUTODOC: get MLL counters for MC subscriber 1 - expect mllOutMCPkts=1 based on eVidx 0x1FFF MLL */
    rc = prvTgfL2MllMcCntGet(prvTgfDevNum, mllCntSet_0, &mllOutMCPkts);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllCntGet");
    PRV_UTF_LOG1_MAC("MC subscriber 1: mllOutMCPkts = %d\n", mllOutMCPkts);
    if(l2MllOutMcPcktCntErrata==GT_TRUE)
    {
        /* "L2 MLL Out Multicast Packets Counter" do not count non IP packets - FE-6932750 */
        UTF_VERIFY_EQUAL2_STRING_MAC(0, mllOutMCPkts,
                                    "get wrong mllOutMCPkts on port[%d] = %d",
                                     1, prvTgfPortsArray[1]);
    }
    else
    {
        UTF_VERIFY_EQUAL2_STRING_MAC(1, mllOutMCPkts,
                                    "get wrong mllOutMCPkts on port[%d] = %d",
                                     1, prvTgfPortsArray[1]);
    }

    /* AUTODOC: get MLL counters for MC subscriber 2 - expect mllOutMCPkts=0 no use of MLL,
       get to port[2] the tester */
    rc = prvTgfL2MllMcCntGet(prvTgfDevNum, mllCntSet_1, &mllOutMCPkts);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllCntGet");
    PRV_UTF_LOG1_MAC("MC subscriber 2: mllOutMCPkts = %d\n", mllOutMCPkts);
    if (PRV_CPSS_SIP_5_20_CHECK_MAC(prvTgfDevNum)) 
    {
        /* AUTODOC: Bobcat3 increment the counter for the last replication no matter if it filtered or not */
        UTF_VERIFY_EQUAL2_STRING_MAC(1, mllOutMCPkts, 
                                        "get wrong mllOutMCPkts on port[%d] = %d",
                                         PRV_TGF_SEND_PORT_IDX_CNS, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    }
    else
    {
        UTF_VERIFY_EQUAL2_STRING_MAC(0, mllOutMCPkts, 
                                        "get wrong mllOutMCPkts on port[%d] = %d",
                                         PRV_TGF_SEND_PORT_IDX_CNS, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    }

    /* enable MC local switching */
    mllPairWriteForm = PRV_TGF_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E;

    rc = prvTgfL2MllPairRead(prvTgfDevNum, 10, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairRead: index 10 - secondMllNode");

    mllPairEntry.secondMllNode.mcLocalSwitchingEnable=GT_TRUE;

    /* AUTODOC: update L2 MLL entry 10 - secondMllNode */
    /* AUTODOC:   enable MC Local switching */
    rc = prvTgfL2MllPairWrite(prvTgfDevNum, 10, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairWrite: index 10 - secondMllNode");

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* AUTODOC: send Ethernet packet from port 2 with: */
    /* AUTODOC:   DA=01:00:00:00:00:01, SA=00:00:00:00:00:99, VID=1 */
    prvTgfL2MllUtilsPacketSend(&prvTgfPacketInfo, prvTgfBurstCount, PRV_TGF_SEND_PORT_IDX_CNS);

    /* give time to drain all packets in TxQ before read counters */
    cpssOsTimerWkAfter(100);

    /* AUTODOC: verify "multi" replicated Ethernet packet on ports [1,2] */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* check Rx counters - Tx from port[1] (based on eVidx 0xFFFF MLL) & from port[2] (the tester) */
        /* with "multi" replications.   */
        if (1 == portIter || PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            if( portCntrs.goodPktsSent.l[0] < 10 )
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(10, portCntrs.goodPktsSent.l[0],
                                     "get much less Tx packets than expected on port[%d] = %d",
                                     portIter, prvTgfPortsArray[portIter]);
            }
        }
        else
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                         "get another Tx packets than expected on port[%d] = %d",
                                         portIter, prvTgfPortsArray[portIter]);
        }
    }

    /* disable PE filtering */
    mllPairWriteForm = PRV_TGF_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E;

    rc = prvTgfL2MllPairRead(prvTgfDevNum, 10, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairRead: index 10 - secondMllNode");

    mllPairEntry.secondMllNode.meshId = 0;

    /* AUTODOC: update L2 MLL entry 10 - secondMllNode */
    rc = prvTgfL2MllPairWrite(prvTgfDevNum, 10, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairWrite: index 10 - secondMllNode");

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* AUTODOC: send Ethernet packet from port 2 with: */
    /* AUTODOC:   DA=01:00:00:00:00:01, SA=00:00:00:00:00:99, VID=1 */
    prvTgfL2MllUtilsPacketSend(&prvTgfPacketInfo, prvTgfBurstCount, PRV_TGF_SEND_PORT_IDX_CNS);

    /* give time to drain all packets in TxQ before read counters */
    cpssOsTimerWkAfter(100);

    /* AUTODOC: verify "multi" replicated Ethernet packet on ports [1,2] */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* check Rx counters - Tx from port[1] (based on eVidx 0xFFFF MLL) & from port[2] (the tester) */
        /* with "multi" replications.   */
        if (1 == portIter || PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            if( portCntrs.goodPktsSent.l[0] < 10 )
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(10, portCntrs.goodPktsSent.l[0],
                                     "get much less Tx packets than expected on port[%d] = %d",
                                     portIter, prvTgfPortsArray[portIter]);
            }
        }
        else
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                         "get another Tx packets than expected on port[%d] = %d",
                                         portIter, prvTgfPortsArray[portIter]);
        }
    }

    /* Disable MC local switching, enable PE filtering */
    mllPairWriteForm = PRV_TGF_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E;

    rc = prvTgfL2MllPairRead(prvTgfDevNum, 10, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairRead: index 10 - secondMllNode");

    mllPairEntry.secondMllNode.mcLocalSwitchingEnable = GT_FALSE;
    mllPairEntry.secondMllNode.meshId = 1;

    /* AUTODOC: update L2 MLL entry 10 - secondMllNode */
    /* AUTODOC: disable MC Local switching */
    /* AUTODOC: enable PE Filtering */
    rc = prvTgfL2MllPairWrite(prvTgfDevNum, 10, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairWrite: index 10 - secondMllNode");

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* AUTODOC: send Ethernet packet from port 2 with: */
    /* AUTODOC:   DA=01:00:00:00:00:01, SA=00:00:00:00:00:99, VID=1 */
    prvTgfL2MllUtilsPacketSend(&prvTgfPacketInfo, prvTgfBurstCount, PRV_TGF_SEND_PORT_IDX_CNS);

    /* AUTODOC: verify Ethernet packet on ports [1,2] */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        tgfSimSleep(1);

        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* check Rx counters - Tx from port[1] (based on eVidx 0x1FFF MLL) & from port[2] (the tester) */
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
    }

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();
}

/**
* @internal prvTgfMcLocalSwitchingConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration.
*/
GT_VOID prvTgfMcLocalSwitchingConfigurationRestore
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

    lttIndexArray[0] = 0x1FFF;

    mllIndexArray[0] = 10;

    /* AUTODOC: clear L2 MLL and LTT entries */
    prvTgfL2MllUtilsMllAndLttEntriesClear(lttIndexArray, 1, mllIndexArray, 1);

    /* AUTODOC: restore default L2 MLL configurations */
    prvTgfL2MllUtilsGlobalConfigurationRestore();
}


