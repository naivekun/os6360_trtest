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
* @file prvTgfEgressInterface.c
*
* @brief L2 MLL Egress Interface test
*
* @version   15
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfL2MllGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfTrunkGen.h>
#include <common/tgfIpGen.h>

#include <l2mll/prvTgfL2MllUtils.h>
#include <l2mll/prvTgfEgressInterface.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN Id */
#define PRV_TGF_VLANID_CNS  1

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS     0

/* ePort used in MLL */
#define PRV_TGF_L2_MLL_EPORT_CNS    ALIGN_EPORT_TO_512_TILL_MAX_DEV_MAC(_8K - 5)

/* vidx used in MLL - One less than device maximum */
#define PRV_TGF_L2_MLL_VIDX_CNS     ((GT_U16)(PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_TABLE_VIDX_MAC(prvTgfDevNum) - 1))

/* trunk IDs */
#define PRV_TGF_TRUNK_ID_1_CNS  2
#define PRV_TGF_TRUNK_ID_2_CNS  127

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 1;

#define TGF_PORTS_NUM_CNS       10

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x01, 0x00, 0x00, 0x00, 0x00, 0x22},               /* dstMac */
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

static GT_U8 origPrvTgfPortsNum;
static GT_U8 origPrvTgfDevsArray[PRV_TGF_MAX_PORTS_NUM_CNS];
static GT_U32 origPrvTgfPortsArray[PRV_TGF_MAX_PORTS_NUM_CNS];
static CPSS_INTERFACE_INFO_STC origPhysicalInfo;
/* Aldrin Z0: ports 0..4, 16..19, 32..35, 56..59, 64..71, 63(CPU SDMA) */
static GT_PHYSICAL_PORT_NUM    portsForTest_aldrinZ0[TGF_PORTS_NUM_CNS] =
    {0,         1,          2,          3,
     16,       32,         18,         34,
     17,       19};

/* Aldrin: ports 0..31, 32(CPU Port) */
static GT_PHYSICAL_PORT_NUM    portsForTest_aldrin[TGF_PORTS_NUM_CNS] =
    {0,         1,          2,          3,
     16,       28,         18,         30,
     17,       19};

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/


/**
* @internal prvTgfEgressInterfaceConfigurationSet function
* @endinternal
*
* @brief   Set configuration.
*/
GT_VOID prvTgfEgressInterfaceConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_TRUNK_ID                 trunkId;
    PRV_TGF_PAIR_READ_WRITE_FORM_ENT    mllPairWriteForm;
    PRV_TGF_L2_MLL_PAIR_STC             mllPairEntry;
    GT_U32                       portsArray[3];
    CPSS_TRUNK_MEMBER_STC       enabledMembersArray[3];
    GT_U8                       i;
    CPSS_INTERFACE_INFO_STC     physicalInfo;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* Save L2 MLL configurations */
    prvTgfL2MllUtilsGlobalConfigurationGetAndSave();

    /* Save UTs ports array */
    origPrvTgfPortsNum = prvTgfPortsNum;
    for(i = 0 ; i < TGF_PORTS_NUM_CNS ; i ++)
    {
        origPrvTgfDevsArray[i] = prvTgfDevsArray[i];
        origPrvTgfPortsArray[i] = prvTgfPortsArray[i];
    }

    prvTgfPortsNum = TGF_PORTS_NUM_CNS;
    if(MUST_USE_ORIG_PORTS_MAC(prvTgfDevNum))
    {
        /* do not modify the 12 ports in prvTgfPortsArray[] !!! */
    }
    else if (PRV_CPSS_PP_MAC(prvTgfDevNum)->devType == CPSS_98DX8332_Z0_CNS)
    {
        prvTgfDevsArray[0] = prvTgfDevNum;

        for(i = 0 ; i < TGF_PORTS_NUM_CNS ; i ++)
        {
            prvTgfPortsArray[i] = portsForTest_aldrinZ0[i];
        }
    }
    else if ((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E) ||
            (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E))
    {
        prvTgfDevsArray[0] = prvTgfDevNum;

        for(i = 0 ; i < TGF_PORTS_NUM_CNS ; i ++)
        {
            prvTgfPortsArray[i] = portsForTest_aldrin[i];
        }
    }
    else
    {
        prvTgfDevsArray[0] = prvTgfDevNum;
        prvTgfPortsArray[0] = 0;
        /* AUTODOC: update ports list: */
        /* AUTODOC:   4, 2, 20, 18, 36, 34, 52, 50, 8  */
        for(i = 1 ; i < prvTgfPortsNum - 1 ; i ++)
        {
            prvTgfDevsArray[i] = prvTgfDevNum;
            prvTgfPortsArray[i] = ((i-1)/2)*16 + ((i%2)+1) * 2;

            if ((PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))&&
                (prvTgfPortsArray[i]==52))
            {
                /* port 52 have problems getting traffic on board */
                prvTgfPortsArray[i]=13;
            }
            else
            if ((IS_BOBK_DEV_MAC(prvTgfDevNum))&&
                (prvTgfPortsArray[i]==50))
            {
                /* port 50 not exists in the device . use port 57 */
                prvTgfPortsArray[i]=57;
            }
        }
        prvTgfDevsArray[i] = prvTgfDevNum;
        prvTgfPortsArray[i] = 8;
    }

    /* New ports added: Link up and reset counters */
    prvTgfEthCountersReset(prvTgfDevNum);

    /* AUTODOC: disable MLL lookup for all multi-target packets */
    rc = prvTgfL2MllLookupForAllEvidxEnableSet(prvTgfDevNum, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllLookupForAllEvidxEnableSet: enable %d",
                                 GT_FALSE);

    /* create eVidx 1 with port members 2 & 18 & 8*/
    portsArray[0] = prvTgfPortsArray[1];
    portsArray[1] = prvTgfPortsArray[3];
    portsArray[2] = prvTgfPortsArray[9];

    /* AUTODOC: create eVIDX 1 with ports [1,3,9] */
    rc = prvTgfBrgVidxEntrySet(1, portsArray, NULL, 3);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVidxEntrySet: %d, %d",
                                 prvTgfDevNum, 1);

    /* create eVidx 4094 with port members 4 & 20 */
    portsArray[0] = prvTgfPortsArray[2];
    portsArray[1] = prvTgfPortsArray[4];

    /* AUTODOC: create eVIDX 4094 with ports [2,4] */
    rc = prvTgfBrgVidxEntrySet(PRV_TGF_L2_MLL_VIDX_CNS, portsArray, NULL, 2);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVidxEntrySet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_L2_MLL_VIDX_CNS);

    trunkId = PRV_TGF_TRUNK_ID_1_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    enabledMembersArray[0].hwDevice = prvTgfDevsArray[5];
    enabledMembersArray[0].port   = prvTgfPortsArray[5];
    enabledMembersArray[1].hwDevice = prvTgfDevsArray[7];
    enabledMembersArray[1].port   = prvTgfPortsArray[7];

    /* AUTODOC: create create trunk 2 with ports [5,7] */
    rc = prvTgfTrunkMembersSet(
        trunkId /*trunkId*/,
        2 /*numOfEnabledMembers*/,
        enabledMembersArray,
        0 /*numOfDisabledMembers*/,
        (CPSS_TRUNK_MEMBER_STC*)NULL  /*disabledMembersArray*/);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTrunkMembersSet: %d, %d",
                                 prvTgfDevNum, trunkId);

    trunkId = PRV_TGF_TRUNK_ID_2_CNS;
    if (trunkId > PRV_CPSS_DEV_TRUNK_INFO_MAC(prvTgfDevNum)->numberOfTrunks)
    {
        trunkId = (GT_TRUNK_ID) PRV_CPSS_DEV_TRUNK_INFO_MAC(prvTgfDevNum)->numberOfTrunks;
    }
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    enabledMembersArray[0].hwDevice = prvTgfDevsArray[6];
    enabledMembersArray[0].port   = prvTgfPortsArray[6];
    enabledMembersArray[1].hwDevice = prvTgfDevsArray[8];
    enabledMembersArray[1].port   = prvTgfPortsArray[8];

    /* AUTODOC: create create trunk 4095 with ports [6,8] */
    rc = prvTgfTrunkMembersSet(
        trunkId /*trunkId*/,
        2 /*numOfEnabledMembers*/,
        enabledMembersArray,
        0 /*numOfDisabledMembers*/,
        (CPSS_TRUNK_MEMBER_STC*)NULL  /*disabledMembersArray*/);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTrunkMembersSet: %d, %d",
                                 prvTgfDevNum, trunkId);

    /* AUTODOC: add FDB entry with MAC 01:00:00:00:00:22, VLAN 1, eVIDX 4096 */
    rc = prvTgfBrgDefFdbMacEntryOnVidxSet(prvTgfPacketL2Part.daMac, PRV_TGF_VLANID_CNS,
                                          4096, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnVidxSet: eVidx 4096");

    /* save ePort mapping configuration */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableGet(prvTgfDevNum,
                                                           PRV_TGF_L2_MLL_EPORT_CNS,
                                                           &origPhysicalInfo);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableGet: "
                                            "dev %d, ePort %d",
                                            prvTgfDevNum, PRV_TGF_L2_MLL_EPORT_CNS);

    /* set ePort mapping configuration */
    physicalInfo.type = CPSS_INTERFACE_PORT_E;
    physicalInfo.devPort.hwDevNum = prvTgfDevNum;
    physicalInfo.devPort.portNum = prvTgfPortsArray[9];

    /* AUTODOC: map port 9 to ePort 500 */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                           PRV_TGF_L2_MLL_EPORT_CNS,
                                                           &physicalInfo);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableSet: "
                                            "dev %d, ePort %d",
                                            prvTgfDevNum, PRV_TGF_L2_MLL_EPORT_CNS);

    /* create L2 MLL entries */
    cpssOsMemSet(&mllPairEntry, 0, sizeof(PRV_TGF_L2_MLL_PAIR_STC));

    mllPairWriteForm = PRV_TGF_PAIR_READ_WRITE_WHOLE_E;
    mllPairEntry.nextPointer = 1;
    mllPairEntry.entrySelector = PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;

    /* AUTODOC:   nextMllPointer=1, maskBitmap=0x11, port=9 */
    mllPairEntry.firstMllNode.last = GT_FALSE;
    mllPairEntry.firstMllNode.maskBitmap = 0x11;
    mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_PORT_E;
    mllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum = prvTgfDevNum;
    mllPairEntry.firstMllNode.egressInterface.devPort.portNum = prvTgfPortsArray[9];

    /* AUTODOC:   maskBitmap=0x16, vidx=1 */
    mllPairEntry.secondMllNode.last = GT_FALSE;
    mllPairEntry.secondMllNode.maskBitmap = 0x16;
    mllPairEntry.secondMllNode.egressInterface.type = CPSS_INTERFACE_VIDX_E;
    mllPairEntry.secondMllNode.egressInterface.vidx = 1;

    /* AUTODOC: add L2 MLL entry 0: */
    rc = prvTgfL2MllPairWrite(prvTgfDevNum, 0, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairWrite: index 0");

    mllPairEntry.nextPointer = 2;
    mllPairEntry.entrySelector = PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;

    /* AUTODOC:   maskBitmap=0x14, vidx=4094 */
    mllPairEntry.firstMllNode.last = GT_FALSE;
    mllPairEntry.firstMllNode.maskBitmap = 0x14;
    mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_VIDX_E;
    mllPairEntry.firstMllNode.egressInterface.vidx = PRV_TGF_L2_MLL_VIDX_CNS;

    /* AUTODOC:   maskBitmap=0x1A, trunkId=2 */
    mllPairEntry.secondMllNode.last = GT_FALSE;
    mllPairEntry.secondMllNode.maskBitmap = 0x1A;
    mllPairEntry.secondMllNode.egressInterface.type = CPSS_INTERFACE_TRUNK_E;
    mllPairEntry.secondMllNode.egressInterface.trunkId = PRV_TGF_TRUNK_ID_1_CNS;/*2*/

    /* AUTODOC: add L2 MLL entry 1: */

    rc = prvTgfL2MllPairWrite(prvTgfDevNum, 1, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairWrite: index 1");

    mllPairEntry.nextPointer = 0;
    mllPairEntry.entrySelector = PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;

    /* AUTODOC:   maskBitmap=0x18, trunkId=127 */
    mllPairEntry.firstMllNode.last = GT_FALSE;
    mllPairEntry.firstMllNode.maskBitmap = 0x18;
    mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_TRUNK_E;
    mllPairEntry.firstMllNode.egressInterface.trunkId = trunkId;/*127*/

    /* AUTODOC:   maskBitmap=0x20, ePort=500 */
    mllPairEntry.secondMllNode.last = GT_TRUE;
    mllPairEntry.secondMllNode.maskBitmap = 0x20;
    mllPairEntry.secondMllNode.egressInterface.type = CPSS_INTERFACE_PORT_E;
    mllPairEntry.secondMllNode.egressInterface.devPort.hwDevNum = prvTgfDevNum;
    mllPairEntry.secondMllNode.egressInterface.devPort.portNum = PRV_TGF_L2_MLL_EPORT_CNS;

    /* AUTODOC: add L2 MLL entry 2: */
    rc = prvTgfL2MllPairWrite(prvTgfDevNum, 2, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairWrite: index 2");



}

/**
* @internal prvTgfEgressInterfaceTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfEgressInterfaceTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    portIter;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    GT_U32    expectedTxPacket, expectedTxPacketTrunk1, expectedTxPacketTrunk2;
    PRV_TGF_L2_MLL_LTT_ENTRY_STC   lttEntry;


    /* AUTODOC: GENERATE TRAFFIC: */

    /* create L2 MLL LTT entries */
    cpssOsMemSet(&lttEntry, 0, sizeof(PRV_TGF_L2_MLL_LTT_ENTRY_STC));

    lttEntry.mllPointer = 0;
    lttEntry.entrySelector = 0;
    /* no mask profile - replicate to all:  */
    /* port 8                               */
    /* ePort XX -> port 8                   */
    /* vidx 1 - ports 4, 20, 8              */
    /* vidx 4094 - ports 2, 18              */
    /* trunk 2 - port 36 or 52              */
    /* trunk 127 - port 34 or 50            */
    lttEntry.mllMaskProfileEnable = GT_FALSE;

    /* AUTODOC: add L2 MLL LTT entry 0 with: */
    /* AUTODOC:   mllPointer=0, mllMaskProfileEnable=False, mllMaskProfile=0 */
    rc = prvTgfL2MllLttEntrySet(prvTgfDevNum, 0, &lttEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllLttEntrySet: index 0");

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* AUTODOC: send Ethernet packet from port 0 with: */
    /* AUTODOC:   DA=01:00:00:00:00:22, SA=00:00:00:00:00:99, VID=1 */
    prvTgfL2MllUtilsPacketSend(&prvTgfPacketInfo, prvTgfBurstCount, PRV_TGF_SEND_PORT_IDX_CNS);

    expectedTxPacketTrunk1 = 0;
    expectedTxPacketTrunk2 = 0;

    /* AUTODOC: verify received Ethernet packets: */
    /* AUTODOC:   ports [0..4] - 1 packet */
    /* AUTODOC:   trunks [2,4095] - 1 packet */
    /* AUTODOC:   ports [9] - 3 packets */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        switch(portIter)
        {
            case 0: /* port 0 */ expectedTxPacket = 1;
                    break;
            case 1: /* port 4 */ expectedTxPacket = 1;
                    break;
            case 2: /* port 2 */ expectedTxPacket = 1;
                    break;
            case 3: /* port 20 */ expectedTxPacket = 1;
                    break;
            case 4: /* port 18 */ expectedTxPacket = 1;
                    break;
            case 5: /* port 36 */ expectedTxPacketTrunk1 = portCntrs.goodPktsSent.l[0];
                    continue;
                    break;
            case 6: /* port 34 */ expectedTxPacketTrunk2 = portCntrs.goodPktsSent.l[0];
                    continue;
                    break;
            case 7: /* port 52 */ expectedTxPacket = 1 - expectedTxPacketTrunk1;
                    break;
            case 8: /* port 50 */ expectedTxPacket = 1 - expectedTxPacketTrunk2;
                    break;
            case 9: /* port 8 */ expectedTxPacket = 3;
                    break;
            default: expectedTxPacket = 0;
                     break;
        }

        UTF_VERIFY_EQUAL2_STRING_MAC(expectedTxPacket, portCntrs.goodPktsSent.l[0],
                                     "get another Tx packets than expected on port[%d] = %d",
                                     portIter, prvTgfPortsArray[portIter]);
    }


    /* mask profile 0 - replicate to : port 8 */
    lttEntry.mllMaskProfileEnable = GT_TRUE;
    lttEntry.mllMaskProfile = 0;

    /* AUTODOC: update L2 MLL LTT entry 0 with: */
    /* AUTODOC:   mllPointer=0, mllMaskProfileEnable=True, mllMaskProfile=0 */
    rc = prvTgfL2MllLttEntrySet(prvTgfDevNum, 0, &lttEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllLttEntrySet: index 0");

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* AUTODOC: send Ethernet packet from port 0 with: */
    /* AUTODOC:   DA=01:00:00:00:00:22, SA=00:00:00:00:00:99, VID=1 */
    prvTgfL2MllUtilsPacketSend(&prvTgfPacketInfo, prvTgfBurstCount, PRV_TGF_SEND_PORT_IDX_CNS);

    /* AUTODOC: verify received Ethernet packets: */
        /* AUTODOC:   ports [0,9] - 1 packet */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        switch(portIter)
        {
            case 0: /* port 0 */ expectedTxPacket = 1;
                    break;
            case 9: /* port 8 */ expectedTxPacket = 1;
                    break;
            default: expectedTxPacket = 0;
                     break;
        }

        UTF_VERIFY_EQUAL2_STRING_MAC(expectedTxPacket, portCntrs.goodPktsSent.l[0],
                                     "get another Tx packets than expected on port[%d] = %d",
                                     portIter, prvTgfPortsArray[portIter]);
    }


    /* mask profile 1 - replicate to:  */
    /* vidx 1 - ports 4, 20, 8         */
    /* trunk 2 - port 36 or 52         */
    lttEntry.mllMaskProfileEnable = GT_TRUE;
    lttEntry.mllMaskProfile = 1;

    /* AUTODOC: update L2 MLL LTT entry 0 with: */
    /* AUTODOC:   mllPointer=0, mllMaskProfileEnable=True, mllMaskProfile=1 */
    rc = prvTgfL2MllLttEntrySet(prvTgfDevNum, 0, &lttEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllLttEntrySet: index 0");

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* AUTODOC: send Ethernet packet from port 0 with: */
    /* AUTODOC:   DA=01:00:00:00:00:22, SA=00:00:00:00:00:99, VID=1 */
    prvTgfL2MllUtilsPacketSend(&prvTgfPacketInfo, prvTgfBurstCount, PRV_TGF_SEND_PORT_IDX_CNS);

    /* AUTODOC: verify received Ethernet packets: */
    /* AUTODOC:   ports [0,1,3,9] - 1 packet */
    /* AUTODOC:   trunks [2] - 1 packet */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        switch(portIter)
        {
            case 0: /* port 0 */ expectedTxPacket = 1;
                    break;
            case 1: /* port 4 */ expectedTxPacket = 1;
                    break;
            case 3: /* port 20 */ expectedTxPacket = 1;
                    break;
            case 5: /* port 36 */ expectedTxPacketTrunk1 = portCntrs.goodPktsSent.l[0];
                    continue;
                    break;
            case 7: /* port 52 */ expectedTxPacket = 1 - expectedTxPacketTrunk1;
                    break;
            case 9: /* port 8 */ expectedTxPacket = 1;
                    break;
            default: expectedTxPacket = 0;
                     break;
        }

        UTF_VERIFY_EQUAL2_STRING_MAC(expectedTxPacket, portCntrs.goodPktsSent.l[0],
                                     "get another Tx packets than expected on port[%d] = %d",
                                     portIter, prvTgfPortsArray[portIter]);
    }

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* AUTODOC: send Ethernet packet from port 9 with: */
    /* AUTODOC:   DA=01:00:00:00:00:22, SA=00:00:00:00:00:99, VID=1 */
    prvTgfL2MllUtilsPacketSend(&prvTgfPacketInfo, prvTgfBurstCount, 9);

    /* AUTODOC: verify received Ethernet packets: */
    /* AUTODOC:   ports [1,3,9] - 1 packet */
    /* AUTODOC:   trunks [2] - 1 packet */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        switch(portIter)
        {
            case 0: /* port 0 */ expectedTxPacket = 0;
                    break;
            case 1: /* port 4 */ expectedTxPacket = 1;
                    break;
            case 3: /* port 20 */ expectedTxPacket = 1;
                    break;
            case 5: /* port 36 */ expectedTxPacketTrunk1 = portCntrs.goodPktsSent.l[0];
                    continue;
                    break;
            case 7: /* port 52 */ expectedTxPacket = 1 - expectedTxPacketTrunk1;
                    break;
            case 9: /* port 8 */ expectedTxPacket = 1;
                    break;
            default: expectedTxPacket = 0;
                     break;
        }

        UTF_VERIFY_EQUAL2_STRING_MAC(expectedTxPacket, portCntrs.goodPktsSent.l[0],
                                     "get another Tx packets than expected on port[%d] = %d",
                                     portIter, prvTgfPortsArray[portIter]);
    }

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* AUTODOC: send Ethernet packet from port 5 with: */
    /* AUTODOC:   DA=01:00:00:00:00:22, SA=00:00:00:00:00:99, VID=1 */
    prvTgfL2MllUtilsPacketSend(&prvTgfPacketInfo, prvTgfBurstCount, 5);

    /* AUTODOC: verify received Ethernet packets: */
    /* AUTODOC:   ports [1,3,5,9] - 1 packet */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        switch(portIter)
        {
            case 0: /* port 0 */ expectedTxPacket = 0;
                    break;
            case 1: /* port 4 */ expectedTxPacket = 1;
                    break;
            case 3: /* port 20 */ expectedTxPacket = 1;
                    break;
            case 5: /* port 36 */ expectedTxPacket = 1;
                    break;
            case 7: /* port 52 */ expectedTxPacket = 0;
                    break;
            case 9: /* port 8 */ expectedTxPacket = 1;
                    break;


            default: expectedTxPacket = 0;
                     break;
        }

        UTF_VERIFY_EQUAL2_STRING_MAC(expectedTxPacket, portCntrs.goodPktsSent.l[0],
                                     "get another Tx packets than expected on port[%d] = %d",
                                     portIter, prvTgfPortsArray[portIter]);
    }


    /* mask profile 2 - replicate to:  */
    /* vidx 1 - ports 4, 20, 8         */
    /* vidx 4094 - ports 2, 18         */
    lttEntry.mllMaskProfileEnable = GT_TRUE;
    lttEntry.mllMaskProfile = 2;

    /* AUTODOC: update L2 MLL LTT entry 0 with: */
    /* AUTODOC:   mllPointer=0, mllMaskProfileEnable=True, mllMaskProfile=2 */
    rc = prvTgfL2MllLttEntrySet(prvTgfDevNum, 0, &lttEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllLttEntrySet: index 0");

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* AUTODOC: send Ethernet packet from port 0 with: */
    /* AUTODOC:   DA=01:00:00:00:00:22, SA=00:00:00:00:00:99, VID=1 */
    prvTgfL2MllUtilsPacketSend(&prvTgfPacketInfo, prvTgfBurstCount, PRV_TGF_SEND_PORT_IDX_CNS);

    /* AUTODOC: verify received Ethernet packets: */
    /* AUTODOC:   ports [0..4,9] - 1 packet */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        switch(portIter)
        {
            case 0: /* port 0 */
            case 1: /* port 4 */
            case 2: /* port 2 */
            case 3: /* port 20 */
            case 4: /* port 18 */
            case 9: /* port 8 */ expectedTxPacket = 1;
                    break;
            case 5: /* port 36 */
            case 6: /* port 34 */
            case 7: /* port 52 */
            case 8: /* port 50 */
            default: expectedTxPacket = 0;
                     break;
        }

        UTF_VERIFY_EQUAL2_STRING_MAC(expectedTxPacket, portCntrs.goodPktsSent.l[0],
                                     "get another Tx packets than expected on port[%d] = %d",
                                     portIter, prvTgfPortsArray[portIter]);
    }


    /* mask profile 3 - replicate to:  */
    /* trunk 2 - port 36 or 52         */
    /* trunk 127 - port 34 or 50       */
    lttEntry.mllMaskProfileEnable = GT_TRUE;
    lttEntry.mllMaskProfile = 3;

    /* AUTODOC: update L2 MLL LTT entry 0 with: */
    /* AUTODOC:   mllPointer=0, mllMaskProfileEnable=True, mllMaskProfile=3 */
    rc = prvTgfL2MllLttEntrySet(prvTgfDevNum, 0, &lttEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllLttEntrySet: index 0");

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* AUTODOC: send Ethernet packet from port 0 with: */
    /* AUTODOC:   DA=01:00:00:00:00:22, SA=00:00:00:00:00:99, VID=1 */
    prvTgfL2MllUtilsPacketSend(&prvTgfPacketInfo, prvTgfBurstCount, PRV_TGF_SEND_PORT_IDX_CNS);

    /* AUTODOC: verify received Ethernet packets: */
    /* AUTODOC:   ports [0] - 1 packet */
    /* AUTODOC:   trunks [2,4095] - 1 packet */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        switch(portIter)
        {
            case 0: /* port 0 */ expectedTxPacket = 1;
                    break;
            case 5: /* port 36 */ expectedTxPacketTrunk1 = portCntrs.goodPktsSent.l[0];
                    continue;
                    break;
            case 6: /* port 34 */ expectedTxPacketTrunk2 = portCntrs.goodPktsSent.l[0];
                    continue;
                    break;
            case 7: /* port 52 */ expectedTxPacket = 1 - expectedTxPacketTrunk1;
                    break;
            case 8: /* port 50 */ expectedTxPacket = 1 - expectedTxPacketTrunk2;
                    break;
            case 1: /* port 4 */
            case 2: /* port 2 */
            case 3: /* port 20 */
            case 4: /* port 18 */
            case 9: /* port 8 */
            default: expectedTxPacket = 0;
                     break;
        }

        UTF_VERIFY_EQUAL2_STRING_MAC(expectedTxPacket, portCntrs.goodPktsSent.l[0],
                                     "get another Tx packets than expected on port[%d] = %d",
                                     portIter, prvTgfPortsArray[portIter]);
    }


    /* mask profile 4 - replicate to all:  */
    /* port 8                              */
    /* vidx 1 - ports 4, 20, 8             */
    /* vidx 4094 - ports 2, 18             */
    /* trunk 2 - port 36 or 52             */
    /* trunk 127 - port 34 or 50           */
    lttEntry.mllMaskProfileEnable = GT_TRUE;
    lttEntry.mllMaskProfile = 4;

    /* AUTODOC: update L2 MLL LTT entry 0 with: */
    /* AUTODOC:   mllPointer=0, mllMaskProfileEnable=True, mllMaskProfile=4 */
    rc = prvTgfL2MllLttEntrySet(prvTgfDevNum, 0, &lttEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllLttEntrySet: index 0");

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* AUTODOC: send Ethernet packet from port 0 with: */
    /* AUTODOC:   DA=01:00:00:00:00:22, SA=00:00:00:00:00:99, VID=1 */
    prvTgfL2MllUtilsPacketSend(&prvTgfPacketInfo, prvTgfBurstCount, PRV_TGF_SEND_PORT_IDX_CNS);

    /* AUTODOC: verify received Ethernet packets: */
    /* AUTODOC:   ports [0..4] - 1 packet */
    /* AUTODOC:   trunks [2,4095] - 1 packet */
    /* AUTODOC:   ports [9] - 2 packets */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        switch(portIter)
        {
            case 0: /* port 0 */ expectedTxPacket = 1;
                    break;
            case 1: /* port 4 */ expectedTxPacket = 1;
                    break;
            case 2: /* port 2 */ expectedTxPacket = 1;
                    break;
            case 3: /* port 20 */ expectedTxPacket = 1;
                    break;
            case 4: /* port 18 */ expectedTxPacket = 1;
                    break;
            case 5: /* port 36 */ expectedTxPacketTrunk1 = portCntrs.goodPktsSent.l[0];
                    continue;
                    break;
            case 6: /* port 34 */ expectedTxPacketTrunk2 = portCntrs.goodPktsSent.l[0];
                    continue;
                    break;
            case 7: /* port 52 */ expectedTxPacket = 1 - expectedTxPacketTrunk1;
                    break;
            case 8: /* port 50 */ expectedTxPacket = 1 - expectedTxPacketTrunk2;
                    break;
            case 9: /* port 8 */ expectedTxPacket = 2;
                    break;
            default: expectedTxPacket = 0;
                     break;
        }

        UTF_VERIFY_EQUAL2_STRING_MAC(expectedTxPacket, portCntrs.goodPktsSent.l[0],
                                     "get another Tx packets than expected on port[%d] = %d",
                                     portIter, prvTgfPortsArray[portIter]);
    }

    /* mask profile 5 - replicate only to ePort XX -> port 8 */
    lttEntry.mllMaskProfileEnable = GT_TRUE;
    lttEntry.mllMaskProfile = 5;

    /* AUTODOC: update L2 MLL LTT entry 0 with: */
    /* AUTODOC:   mllPointer=0, mllMaskProfileEnable=True, mllMaskProfile=5 */
    rc = prvTgfL2MllLttEntrySet(prvTgfDevNum, 0, &lttEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllLttEntrySet: index 0");

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* AUTODOC: send Ethernet packet from port 0 with: */
    /* AUTODOC:   DA=01:00:00:00:00:22, SA=00:00:00:00:00:99, VID=1 */
    prvTgfL2MllUtilsPacketSend(&prvTgfPacketInfo, prvTgfBurstCount, PRV_TGF_SEND_PORT_IDX_CNS);

    /* AUTODOC: verify received Ethernet packets: */
    /* AUTODOC:   ports [0,9] - 1 packet */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        switch(portIter)
        {
            case 0: /* port 0 */ expectedTxPacket = 1;
            case 9: /* port 8 */ expectedTxPacket = 1;
                    break;
            default: expectedTxPacket = 0;
                     break;
        }

        UTF_VERIFY_EQUAL2_STRING_MAC(expectedTxPacket, portCntrs.goodPktsSent.l[0],
                                     "get another Tx packets than expected on port[%d] = %d",
                                     portIter, prvTgfPortsArray[portIter]);
    }

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();
}

/**
* @internal prvTgfEgressInterfaceEntrySelectorTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic with different configuration of entrySelector field in
*         the "L2 MLL LTT Entry" and "L2 MLL Entry"
*/
GT_VOID prvTgfEgressInterfaceEntrySelectorTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    portIter;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    GT_U32    expectedTxPacket, expectedTxPacketTrunk1, expectedTxPacketTrunk2;
    PRV_TGF_L2_MLL_LTT_ENTRY_STC   lttEntry;

    PRV_TGF_PAIR_READ_WRITE_FORM_ENT    mllPairWriteForm;
    PRV_TGF_L2_MLL_PAIR_STC             mllPairEntry;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* create L2 MLL LTT entries */
    cpssOsMemSet(&lttEntry, 0, sizeof(PRV_TGF_L2_MLL_LTT_ENTRY_STC));

    lttEntry.mllPointer = 0;
    lttEntry.entrySelector = 0;
    /* no mask profile - replicate to all:  */
    /* port 8                               */
    /* ePort XX -> port 8                   */
    /* vidx 1 - ports 4, 20, 8              */
    /* vidx 4094 - ports 2, 18              */
    /* trunk 2 - port 36 or 52              */
    /* trunk 127 - port 34 or 50            */
    lttEntry.mllMaskProfileEnable = GT_FALSE;

    /* AUTODOC: add L2 MLL LTT entry 0 with: */
    /* AUTODOC:   mllPointer=0, mllMaskProfileEnable=False, mllMaskProfile=0 */
    rc = prvTgfL2MllLttEntrySet(prvTgfDevNum, 0, &lttEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllLttEntrySet: index 0");

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* AUTODOC: send Ethernet packet from port 0 with: */
    /* AUTODOC:   DA=01:00:00:00:00:22, SA=00:00:00:00:00:99, VID=1 */
    prvTgfL2MllUtilsPacketSend(&prvTgfPacketInfo, prvTgfBurstCount, PRV_TGF_SEND_PORT_IDX_CNS);

    expectedTxPacketTrunk1 = 0;
    expectedTxPacketTrunk2 = 0;

    /* AUTODOC: verify received Ethernet packets: */
    /* AUTODOC:   ports [0..4] - 1 packet */
    /* AUTODOC:   trunks [2,4095] - 1 packet */
    /* AUTODOC:   ports [9] - 3 packets */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        switch(portIter)
        {
            case 0: /* port 0 */ expectedTxPacket = 1;
                    break;
            case 1: /* port 4 */ expectedTxPacket = 1;
                    break;
            case 2: /* port 2 */ expectedTxPacket = 1;
                    break;
            case 3: /* port 20 */ expectedTxPacket = 1;
                    break;
            case 4: /* port 18 */ expectedTxPacket = 1;
                    break;
            case 5: /* port 36 */ expectedTxPacketTrunk1 = portCntrs.goodPktsSent.l[0];
                    continue;
                    break;
            case 6: /* port 34 */ expectedTxPacketTrunk2 = portCntrs.goodPktsSent.l[0];
                    continue;
                    break;
            case 7: /* port 52 */ expectedTxPacket = 1 - expectedTxPacketTrunk1;
                    break;
            case 8: /* port 50 */ expectedTxPacket = 1 - expectedTxPacketTrunk2;
                    break;
            case 9: /* port 8 */ expectedTxPacket = 3;
                    break;
            default: expectedTxPacket = 0;
                     break;
        }

        UTF_VERIFY_EQUAL2_STRING_MAC(expectedTxPacket, portCntrs.goodPktsSent.l[0],
                                     "get another Tx packets than expected on port[%d] = %d",
                                     portIter, prvTgfPortsArray[portIter]);
    }


    /* ltt entrySelector = PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_SECOND_MLL_E -
       replicate to all except port 8 in mll index=0 */
    lttEntry.entrySelector = PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_SECOND_MLL_E;

    /* AUTODOC: update L2 MLL LTT entry 0 with: */
    /* AUTODOC:   mllPointer=0, lttEntry.entrySelector = second */
    rc = prvTgfL2MllLttEntrySet(prvTgfDevNum, 0, &lttEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllLttEntrySet: index 0");

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* AUTODOC: send Ethernet packet from port 0 with: */
    /* AUTODOC:   DA=01:00:00:00:00:22, SA=00:00:00:00:00:99, VID=1 */
    prvTgfL2MllUtilsPacketSend(&prvTgfPacketInfo, prvTgfBurstCount, PRV_TGF_SEND_PORT_IDX_CNS);

    /* AUTODOC:   ports [0..4] - 1 packet */
    /* AUTODOC:   trunks [2,4095] - 1 packet */
    /* AUTODOC:   ports [9] - 2 packets */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        switch(portIter)
        {
            case 0: /* port 0 */ expectedTxPacket = 1;
                    break;
            case 1: /* port 4 */ expectedTxPacket = 1;
                    break;
            case 2: /* port 2 */ expectedTxPacket = 1;
                    break;
            case 3: /* port 20 */ expectedTxPacket = 1;
                    break;
            case 4: /* port 18 */ expectedTxPacket = 1;
                    break;
            case 5: /* port 36 */ expectedTxPacketTrunk1 = portCntrs.goodPktsSent.l[0];
                    continue;
                    break;
            case 6: /* port 34 */ expectedTxPacketTrunk2 = portCntrs.goodPktsSent.l[0];
                    continue;
                    break;
            case 7: /* port 52 */ expectedTxPacket = 1 - expectedTxPacketTrunk1;
                    break;
            case 8: /* port 50 */ expectedTxPacket = 1 - expectedTxPacketTrunk2;
                    break;
            case 9: /* port 8 */ expectedTxPacket = 2;
                    break;
            default: expectedTxPacket = 0;
                     break;
        }

        UTF_VERIFY_EQUAL2_STRING_MAC(expectedTxPacket, portCntrs.goodPktsSent.l[0],
                                     "get another Tx packets than expected on port[%d] = %d",
                                     portIter, prvTgfPortsArray[portIter]);
    }


   /* ltt entrySelector = PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_SECOND_MLL_E -
      replicate to all except port 8 in mll index=0 */
    lttEntry.entrySelector = PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_SECOND_MLL_E;

    /* AUTODOC: update L2 MLL LTT entry 0 with: */
    /* AUTODOC:   mllPointer=0, mllMaskProfileEnable=True, mllMaskProfile=1 */
    rc = prvTgfL2MllLttEntrySet(prvTgfDevNum, 0, &lttEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllLttEntrySet: index 0");

    /* MLL Entry entrySelector = second - replicate to all second element in a pair:  */
    /* port 8                    - don't replicate     */
    /* ePort XX -> port 8        - replicate           */
    /* vidx 1 - ports 4, 20, 8   - replicate           */
    /* vidx 4094 - ports 2, 18   - don't replicate     */
    /* trunk 2 - port 36 or 52   - replicate           */
    /* trunk 127 - port 34 or 50 - don't replicate     */

    mllPairWriteForm = PRV_TGF_PAIR_READ_WRITE_WHOLE_E;

    rc = prvTgfL2MllPairRead(prvTgfDevNum, 0, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairRead: index 0");

    mllPairEntry.entrySelector = PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_SECOND_MLL_E;

    rc = prvTgfL2MllPairWrite(prvTgfDevNum, 0, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairWrite: index 0");

    rc = prvTgfL2MllPairRead(prvTgfDevNum, 1, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairRead: index 1");

    mllPairEntry.entrySelector = PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_SECOND_MLL_E;

    rc = prvTgfL2MllPairWrite(prvTgfDevNum, 1, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairWrite: index 1");

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* AUTODOC: send Ethernet packet from port 0 with: */
    /* AUTODOC:   DA=01:00:00:00:00:22, SA=00:00:00:00:00:99, VID=1 */
    prvTgfL2MllUtilsPacketSend(&prvTgfPacketInfo, prvTgfBurstCount, PRV_TGF_SEND_PORT_IDX_CNS);

    /* AUTODOC:   ports [0..3] - 1 packet */
    /* AUTODOC:   trunks [2] - 1 packet */
    /* AUTODOC:   ports [9] - 2 packets */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        switch(portIter)
        {
            case 0: /* port 0 */ expectedTxPacket = 1;
                    break;
            case 1: /* port 4 */ expectedTxPacket = 1;
                    break;
            case 3: /* port 20 */ expectedTxPacket = 1;
                    break;
            case 5: /* port 36 */ expectedTxPacketTrunk1 = portCntrs.goodPktsSent.l[0];
                    continue;
                    break;
            case 7: /* port 52 */ expectedTxPacket = 1 - expectedTxPacketTrunk1;
                    break;
            case 9: /* port 8 */ expectedTxPacket = 2;
                    break;
            default: expectedTxPacket = 0;
                     break;
        }

        UTF_VERIFY_EQUAL2_STRING_MAC(expectedTxPacket, portCntrs.goodPktsSent.l[0],
                                     "get another Tx packets than expected on port[%d] = %d",
                                     portIter, prvTgfPortsArray[portIter]);
    }



   /* ltt entrySelector = PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_SECOND_MLL_E -
      replicate to all except port 8 in mll index=0 */
    lttEntry.entrySelector = PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;

    /* AUTODOC: update L2 MLL LTT entry 0 with: */
    /* AUTODOC:   mllPointer=0, mllMaskProfileEnable=True, mllMaskProfile=1 */
    rc = prvTgfL2MllLttEntrySet(prvTgfDevNum, 0, &lttEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllLttEntrySet: index 0");

    /* MLL Entry entrySelector = second - replicate to all second element in a pair:  */
    /* port 8                    - replicate           */
    /* ePort XX -> port 8        - replicate           */
    /* vidx 1 - ports 4, 20, 8   - replicate           */
    /* vidx 4094 - ports 2, 18   - don't replicate     */
    /* trunk 2 - port 36 or 52   - replicate           */
    /* trunk 127 - port 34 or 50 - don't replicate     */

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* AUTODOC: send Ethernet packet from port 0 with: */
    /* AUTODOC:   DA=01:00:00:00:00:22, SA=00:00:00:00:00:99, VID=1 */
    prvTgfL2MllUtilsPacketSend(&prvTgfPacketInfo, prvTgfBurstCount, PRV_TGF_SEND_PORT_IDX_CNS);

    /* AUTODOC: verify received Ethernet packets: */
    /* AUTODOC:   ports [0..3] - 1 packet */
    /* AUTODOC:   trunks[2] - 1 packet */
    /* AUTODOC:   ports [9] - 3 packets */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        switch(portIter)
        {
            case 0: /* port 0 */ expectedTxPacket = 1;
                    break;
            case 1: /* port 4 */ expectedTxPacket = 1;
                    break;
            case 3: /* port 20 */ expectedTxPacket = 1;
                    break;
            case 5: /* port 36 */ expectedTxPacketTrunk1 = portCntrs.goodPktsSent.l[0];
                    continue;
                    break;
            case 7: /* port 52 */ expectedTxPacket = 1 - expectedTxPacketTrunk1;
                    break;
            case 9: /* port 8 */ expectedTxPacket = 3;
                    break;
            default: expectedTxPacket = 0;
                     break;
        }

        UTF_VERIFY_EQUAL2_STRING_MAC(expectedTxPacket, portCntrs.goodPktsSent.l[0],
                                     "get another Tx packets than expected on port[%d] = %d",
                                     portIter, prvTgfPortsArray[portIter]);
    }

    /* MLL Entry entrySelector = second - replicate to all second element in a pair:  */
    /* port 8                    - replicate           */
    /* ePort XX -> port 8        - replicate           */
    /* vidx 1 - ports 4, 20, 8   - replicate           */
    /* vidx 4094 - ports 2, 18   - replicate           */
    /* trunk 2 - port 36 or 52   - replicate           */
    /* trunk 127 - port 34 or 50 - replicate           */

    mllPairWriteForm = PRV_TGF_PAIR_READ_WRITE_WHOLE_E;

    rc = prvTgfL2MllPairRead(prvTgfDevNum, 0, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairRead: index 0");

    mllPairEntry.entrySelector = PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;

    rc = prvTgfL2MllPairWrite(prvTgfDevNum, 0, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairWrite: index 0");

    rc = prvTgfL2MllPairRead(prvTgfDevNum, 1, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairRead: index 1");

    mllPairEntry.entrySelector = PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;

    rc = prvTgfL2MllPairWrite(prvTgfDevNum, 1, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairWrite: index 1");

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

    /* AUTODOC: send Ethernet packet from port 0 with: */
    /* AUTODOC:   DA=01:00:00:00:00:22, SA=00:00:00:00:00:99, VID=1 */
    prvTgfL2MllUtilsPacketSend(&prvTgfPacketInfo, prvTgfBurstCount, PRV_TGF_SEND_PORT_IDX_CNS);

     /* AUTODOC: verify received Ethernet packets: */
    /* AUTODOC:   ports [0,8] - 1 packet */
    /* AUTODOC:   ports [9]   - 3 packet */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        switch(portIter)
        {
            case 0: /* port 0 */ expectedTxPacket = 1;
                    break;
            case 1: /* port 4 */ expectedTxPacket = 1;
                    break;
            case 2: /* port 2 */ expectedTxPacket = 1;
                    break;
            case 3: /* port 20 */ expectedTxPacket = 1;
                    break;
            case 4: /* port 18 */ expectedTxPacket = 1;
                    break;
            case 5: /* port 36 */ expectedTxPacketTrunk1 = portCntrs.goodPktsSent.l[0];
                    continue;
                    break;
            case 6: /* port 34 */ expectedTxPacketTrunk2 = portCntrs.goodPktsSent.l[0];
                    continue;
                    break;
            case 7: /* port 52 */ expectedTxPacket = 1 - expectedTxPacketTrunk1;
                    break;
            case 8: /* port 50 */ expectedTxPacket = 1 - expectedTxPacketTrunk2;
                    break;
            case 9: /* port 8 */ expectedTxPacket = 3;
                    break;
            default: expectedTxPacket = 0;
                     break;
        }

        UTF_VERIFY_EQUAL2_STRING_MAC(expectedTxPacket, portCntrs.goodPktsSent.l[0],
                                     "get another Tx packets than expected on port[%d] = %d",
                                     portIter, prvTgfPortsArray[portIter]);
    }

    /* reset counters */
    prvTgfL2MllUtilsResetAllEthernetCounters();

}

/**
* @internal prvTgfEgressInterfaceConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration.
*/
GT_VOID prvTgfEgressInterfaceConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_TRUNK_ID trunkId;
    GT_U32    lttIndexArray[1];
    GT_U32    mllIndexArray[6];
    GT_U8     i;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* AUTODOC: clear eVidx 1 */
    rc = prvTgfBrgVidxEntrySet(1, NULL, NULL, 0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVidxEntrySet: %d, %d",
                                 prvTgfDevNum, 1);

    /* AUTODOC: clear eVidx 4094 */
    rc = prvTgfBrgVidxEntrySet(PRV_TGF_L2_MLL_VIDX_CNS, NULL, NULL, 0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVidxEntrySet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_L2_MLL_VIDX_CNS);


    trunkId = PRV_TGF_TRUNK_ID_1_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    /* AUTODOC: clear trunk 2 */
    rc = prvTgfTrunkMembersSet(
        trunkId /*trunkId*/,
        0 /*numOfEnabledMembers*/,
        (CPSS_TRUNK_MEMBER_STC*)NULL /*enabledMembersArray*/,
        0 /*numOfDisabledMembers*/,
        (CPSS_TRUNK_MEMBER_STC*)NULL  /*disabledMembersArray*/);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTrunkMembersSet: %d, %d",
                                 prvTgfDevNum, trunkId);

    trunkId = PRV_TGF_TRUNK_ID_2_CNS;
    if (trunkId > PRV_CPSS_DEV_TRUNK_INFO_MAC(prvTgfDevNum)->numberOfTrunks)
    {
        trunkId = (GT_TRUNK_ID) PRV_CPSS_DEV_TRUNK_INFO_MAC(prvTgfDevNum)->numberOfTrunks;
    }
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    /* AUTODOC: clear trunk 4095 */
    rc = prvTgfTrunkMembersSet(
        trunkId /*trunkId*/,
        0 /*numOfEnabledMembers*/,
        (CPSS_TRUNK_MEMBER_STC*)NULL /*enabledMembersArray*/,
        0 /*numOfDisabledMembers*/,
        (CPSS_TRUNK_MEMBER_STC*)NULL  /*disabledMembersArray*/);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTrunkMembersSet: %d, %d",
                                 prvTgfDevNum, trunkId);

    lttIndexArray[0] = 0;

    mllIndexArray[0] = 0;
    mllIndexArray[1] = 1;
    mllIndexArray[2] = 2;
    mllIndexArray[3] = 3;
    mllIndexArray[4] = 4;
    mllIndexArray[5] = 5;

    /* AUTODOC: clear L2 MLL and LTT entries */
    prvTgfL2MllUtilsMllAndLttEntriesClear(lttIndexArray, 1, mllIndexArray, 6);

    /* AUTODOC: restore default ports array */
    prvTgfPortsNum = origPrvTgfPortsNum;
    for(i = 0 ; i < TGF_PORTS_NUM_CNS ; i ++)
    {
        prvTgfDevsArray[i] = origPrvTgfDevsArray[i];
        prvTgfPortsArray[i] = origPrvTgfPortsArray[i];
    }

    /* AUTODOC: restore default L2 MLL configurations */
    prvTgfL2MllUtilsGlobalConfigurationRestore();

    /* AUTODOC: restore ePort 500 mapping configuration */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                           PRV_TGF_L2_MLL_EPORT_CNS,
                                                           &origPhysicalInfo);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableSet: "
                                            "dev %d, ePort %d",
                                            prvTgfDevNum, PRV_TGF_L2_MLL_EPORT_CNS);
}


