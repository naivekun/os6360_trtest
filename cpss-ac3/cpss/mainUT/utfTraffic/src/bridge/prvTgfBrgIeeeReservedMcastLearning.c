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
* @file prvTgfBrgIeeeReservedMcastLearning.c
*
* @brief Bridge Generic Port IEEE Reserved Mcast Learning UT.
*
* @version   5
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <bridge/prvTgfBrgGen.h>
#include <bridge/prvTgfBrgIeeeReservedMcastLearning.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN0 Id */
#define PRV_TGF_VLANID_CNS  2

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 1;

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS   3

/* port number to receive traffic from */
#define PRV_TGF_RECEIVE_PORT_IDX_CNS    1

/* profile table index to test */
static GT_U32  prvTgfBrgGenIeeeReservedMcastProfileTableIndex = 2;
/* protocol to test */
static GT_U8   prvTgfBrgGenIeeeReservedMcastProtocol = 0x55;

#define SET_PROFILE_AND_PROTOCOL    \
    if(0 == CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily))\
    {                                                                         \
        prvTgfBrgGenIeeeReservedMcastProfileTableIndex = 1;                   \
        prvTgfBrgGenIeeeReservedMcastProtocol = 0x2b;                         \
    }


/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x01, 0x80, 0xc2, 0x00, 0x00, 0x55},               /* dstMac */
    {0x00, 0x00, 0x00, 0x11, 0x44, 0x77}                /* srcMac */
};

/* First VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    3, 5, PRV_TGF_VLANID_CNS                            /* pri, cfi, VlanId */
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
/* LENGTH of single tagged packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS  (PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS)

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo = {
    PRV_TGF_PACKET_LEN_CNS,                                    /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};
/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/
/**
* @internal prvTgfBrgGenPortIeeeReservedMcastLearningCfg function
* @endinternal
*
* @brief   Set VLAN entry.
*
* @param[in] enableLearning           - GT_TRUE: enable Ieee Reserved Multicast learning
* @param[in] profileIndex             - profile index (0..3). The parameter defines profile (table
*                                      number) for the 256 protocols.
* @param[in] protocol                 - specifies the Multicast protocol
* @param[in] cmd                      - supported commands:
*                                      CPSS_PACKET_CMD_FORWARD_E , CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E , CPSS_PACKET_CMD_DROP_SOFT_E
* @param[in] trapEnable               - GT_FALSE: Trapping or mirroring to CPU of packet with
*                                      MAC_DA = 01-80-C2-00-00-xx disabled.
*                                      GT_TRUE: Trapping or mirroring to CPU of packet, with
*                                      MAC_DA = 01-80-C2-00-00-xx enabled according to the setting
*                                      of IEEE Reserved Multicast Configuration Register<n> (0<=n<16)
*                                      by cpssDxChBrgGenIeeeReservedMcastProtCmdSet function
*                                       None
*/
static GT_VOID prvTgfBrgGenPortIeeeReservedMcastLearningCfg
(
    IN GT_BOOL              enableLearning,
    IN GT_U32               profileIndex,
    IN GT_U8                protocol,
    IN CPSS_PACKET_CMD_ENT  cmd,
    IN GT_BOOL              trapEnable

)
{
    GT_STATUS   rc;


    /* set IEEE Reserved Multicast specific profile table index for specific port */
    rc = prvTgfBrgGenPortIeeeReservedMcastProfileIndexSet(prvTgfDevNum,
                                                          prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                                          profileIndex);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgGenPortIeeeReservedMcastProfileIndexSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], profileIndex);


    /* Set command for IEEE Reserved Multicast specific profile index and protocol */
    rc = prvTgfBrgGenIeeeReservedMcastProtCmdSet(prvTgfDevNum,
                                            profileIndex,
                                            protocol,
                                            cmd);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgGenIeeeReservedMcastProtCmdSet: %d, %d, %d",
                                 prvTgfDevNum, profileIndex, protocol);

    /* Enable trap to CPU for IEEE Reserved Multicast packets */
    rc = prvTgfBrgGenIeeeReservedMcastTrapEnable(prvTgfDevNum, trapEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgGenIeeeReservedMcastTrapEnable: %d",
                                 prvTgfDevNum);

    /* Enable/Disable Learning of IEEE Reserved Multicast packets
      for specific port */
    rc = prvTgfBrgGenPortIeeeReservedMcastLearningEnableSet(prvTgfDevNum,
                                                            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                                            enableLearning);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgGenIeeeReservedMcastProtCmdSet: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
}

/**
* @internal prvTgfBrgGenIeeeReservedMcastLearningPacketSend function
* @endinternal
*
* @brief   Function sends packet, performs trace and check expected results.
*
* @param[in] expectPacketNum          - expected packets number captured to CPU
*                                       None
*/
static GT_VOID prvTgfBrgGenIeeeReservedMcastLearningPacketSend
(
    IN GT_U32 expectPacketNum
)
{
    GT_STATUS       rc = GT_OK;
    GT_U8           devNum   = 0;
    GT_U8           queue    = 0;
    GT_BOOL         getFirst = GT_TRUE;
    GT_U32          buffLen  = 0x600;
    GT_U32          packetActualLength = 0;
    static  GT_U8   packetBuff[0x600] = {0};
    GT_U8           portIter;
    TGF_NET_DSA_STC rxParam;

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, prvTgfBurstCount, 0, NULL);

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* set capturing mode */
    rc = tgfTrafficTableRxModeSet(TGF_RX_CYCLIC_E, 600);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxModeSet: %d", prvTgfDevNum);

    /* start Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* send Packet from port given port */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* stop Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");


    /* Get entry from captured packet's table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       getFirst, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue,
                                       &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(PRV_TGF_PACKET_CRC_LEN_CNS * expectPacketNum, packetActualLength, "Number of expected packets is wrong\n");
}


/**
* @internal prvTgfBrgGenIeeeReservedMcastLearningCheckFdb function
* @endinternal
*
* @brief   Get MAC entry.
*
* @param[in] expectedResult           - expected result for mac entry get
*                                       None
*/
static GT_VOID prvTgfBrgGenIeeeReservedMcastLearningCheckFdb
(
    IN GT_STATUS expectedResult
)
{
    GT_STATUS                  rc = GT_OK;
    PRV_TGF_BRG_MAC_ENTRY_STC  macEntry;        /* MAC entry */
    PRV_TGF_MAC_ENTRY_KEY_STC  macEntryKey;     /* MAC entry key */


    macEntryKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntryKey.key.macVlan.vlanId = PRV_TGF_VLANID_CNS;

    cpssOsMemCpy(macEntryKey.key.macVlan.macAddr.arEther, prvTgfPacketL2Part.saMac,
                 sizeof(prvTgfPacketL2Part.saMac));
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));


    /* get FDB entry */
    rc = prvTgfBrgFdbMacEntryGet(&macEntryKey, &macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(expectedResult, rc, "prvTgfBrgFdbMacEntryGet: %d", prvTgfDevNum);

    if (expectedResult == GT_OK)
    {
        /* compare entryType */
        rc = (macEntry.key.entryType == macEntryKey.entryType) ? GT_OK : GT_FAIL;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "vlanId: %d", macEntry.key.entryType);

        /* compare MAC address */
        rc = cpssOsMemCmp(macEntry.key.key.macVlan.macAddr.arEther,
                          macEntryKey.key.macVlan.macAddr.arEther,
                          sizeof(TGF_MAC_ADDR)) == 0 ? GT_OK : GT_FAIL;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "MAC address[0]: %d",
                                     macEntry.key.key.macVlan.macAddr.arEther[0]);
    }
}


/**
* @internal prvTgfBrgGenIeeeReservedMcastLearningTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic and check results
*         INPUTS:
*/
static GT_VOID prvTgfBrgGenIeeeReservedMcastLearningTrafficGenerate
(
    IN GT_BOOL              enableLearning,
    IN GT_U32               profileIndex,
    IN GT_U8                protocol,
    IN CPSS_PACKET_CMD_ENT  cmd,
    IN GT_BOOL              trapEnable
)
{
    GT_STATUS rc;                               /* returned status */
    GT_U8  portIter;                            /* loop port iterator */
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;    /* port counters */
    GT_U8  expectedPktsNum;                     /* expected packets number */
    GT_STATUS learningStatus;                   /* expected result of learning */

    /* set mac DA according to match the protocol */
    prvTgfPacketL2Part.daMac[5] = protocol;

    /* Configure IEEE Reserved Multicast Learning */
    prvTgfBrgGenPortIeeeReservedMcastLearningCfg(enableLearning, /* enable IEEE Reserved learning */
                                                 profileIndex,
                                                 protocol,
                                                 cmd,
                                                 trapEnable  /* enable trap to CPU of IEEE packets */
                                                 );

    if ((cmd == CPSS_PACKET_CMD_TRAP_TO_CPU_E) && (trapEnable == GT_TRUE))
    {
        expectedPktsNum = 1;
    }
    else
    {
        expectedPktsNum = 0;
    }

    /* AUTODOC: GENERATE TRAFFIC: */

    /* AUTODOC: send packet with DA = 01:80:C2:00:00:55, SA = 00:00:00:11:44:77 to port 3 */
    prvTgfBrgGenIeeeReservedMcastLearningPacketSend(expectedPktsNum);

    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfReadPortCountersEth FAILED, rc = [%d]", rc);
        }

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }

        else
        {
            if (cmd == CPSS_PACKET_CMD_TRAP_TO_CPU_E)
            {
                expectedPktsNum = 0;
            }
            else
            {
                expectedPktsNum = 1;
            }

            /* check that packets received on Tx ports are as expected */
            UTF_VERIFY_EQUAL0_STRING_MAC(expectedPktsNum, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
    }

    /* Check Learning*/
    if ((enableLearning == GT_FALSE) && (cmd == CPSS_PACKET_CMD_TRAP_TO_CPU_E) &&
        (trapEnable == GT_TRUE))
    {
        /* the packet shouldn't be learned */
        learningStatus = GT_NOT_FOUND;
    }
    else
    {
        /* the packet should be learned */
        learningStatus = GT_OK;
    }

    prvTgfBrgGenIeeeReservedMcastLearningCheckFdb(learningStatus);

    /* AUTODOC: restore default IEEE Reserved Multicast config */
    prvTgfBrgGenPortIeeeReservedMcastLearningCfg(GT_TRUE, /* enable IEEE Reserved learning */
                                                 profileIndex,
                                                 protocol,
                                                 CPSS_PACKET_CMD_FORWARD_E, /* command */
                                                 GT_FALSE  /* disable trap to CPU of IEEE packets */
                                                 );
}

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/
/**
* @internal prvTgfBrgGenIeeeReservedMcastLearningConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         Create VLAN 2 on all ports (0,8,18,23)
*/
GT_VOID prvTgfBrgGenIeeeReservedMcastLearningConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN 2 with all untagged ports */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);

    /* AUTODOC: set PVID to 2 for port 3 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 PRV_TGF_VLANID_CNS);

}

/**
* @internal prvTgfBrgGenIeeeReservedMcastLearningEnableTrafficGenerate function
* @endinternal
*
* @brief   Enable IEEE Reserved Multicast Learning for trapped packets.
*         Set Profile index 2 for port1.
*         Set command TRAP_TO_CPU for profile index 2 and protocol 0x55.
*         Enable trap to CPU for IEEE Reserved Multicast packets.
*         Generate traffic:
*         Send from port0 single tagged packet:
*         macDa = 01:80:C2:00:00:55
*         macSa = 00:00:00:11:44:77
*         Success Criteria:
*         Packet is trapped to CPU and Learned on FDB.
*/
GT_VOID prvTgfBrgGenIeeeReservedMcastLearningEnableTrafficGenerate
(
    void
)
{

    SET_PROFILE_AND_PROTOCOL;

    /* AUTODOC: enable IEEE Reserved Multicast Learning for trapped packets */
    /* AUTODOC: set Profile index 2 for port 1 */
    /* AUTODOC: set command TRAP_TO_CPU for profile index 2 and protocol 0x55 */
    /* AUTODOC: enable trap to CPU for IEEE Reserved Multicast packets */
    prvTgfBrgGenIeeeReservedMcastLearningTrafficGenerate(GT_TRUE, /* enable IEEE learning*/
                                                         prvTgfBrgGenIeeeReservedMcastProfileTableIndex, /* profile table index */
                                                         prvTgfBrgGenIeeeReservedMcastProtocol, /* prtocol */
                                                         CPSS_PACKET_CMD_TRAP_TO_CPU_E, /* packt command */
                                                         GT_TRUE /* enable trap of IEEE Reserved */
                                                         );
    /* AUTODOC: verify packet is trapped to CPU and learned on FDB */
}


/**
* @internal prvTgfBrgGenIeeeReservedMcastLearningDisableTrafficGenerate function
* @endinternal
*
* @brief   Disable IEEE Reserved Multicast Learning for trapped packets.
*         Set Profile index 2 for port1.
*         Set command TRAP_TO_CPU for profile index 2 and protocol 0x55.
*         Enable trap to CPU for IEEE Reserved Multicast packets.
*         Generate traffic:
*         Send from port0 single tagged packet:
*         macDa = 01:80:C2:00:00:55
*         macSa = 00:00:00:11:44:77
*         Success Criteria:
*         Packet is trapped to CPU and is not Learned on FDB.
*/
GT_VOID prvTgfBrgGenIeeeReservedMcastLearningDisableTrafficGenerate
(
    void
)
{
    SET_PROFILE_AND_PROTOCOL;

    /* AUTODOC: disable IEEE Reserved Multicast Learning for trapped packets */
    /* AUTODOC: set Profile index 2 for port 1 */
    /* AUTODOC: set command TRAP_TO_CPU for profile index 2 and protocol 0x55 */
    /* AUTODOC: enable trap to CPU for IEEE Reserved Multicast packets */
    prvTgfBrgGenIeeeReservedMcastLearningTrafficGenerate(GT_FALSE, /* disable IEEE learning*/
                                                         prvTgfBrgGenIeeeReservedMcastProfileTableIndex, /* profile table index */
                                                         prvTgfBrgGenIeeeReservedMcastProtocol, /* prtocol */
                                                         CPSS_PACKET_CMD_TRAP_TO_CPU_E, /* packt command */
                                                         GT_TRUE /* enable trap of IEEE Reserved */
                                                         );
    /* AUTODOC: verify packet is trapped to CPU and not learned on FDB */
}


/**
* @internal prvTgfBrgGenIeeeReservedMcastLearningDisableTrapDisableTrafficGenerate function
* @endinternal
*
* @brief   Disable IEEE Reserved Multicast Learning for trapped packets.
*         Set Profile index 2 for port1.
*         Set command CPSS_PACKET_CMD_FORWARD_E for profile index 2 and
*         protocol 0x55.
*         Disable trap to CPU for IEEE Reserved Multicast packets.
*         Generate traffic:
*         Send from port0 single tagged packet:
*         macDa = 01:80:C2:00:00:55
*         macSa = 00:00:00:11:44:77
*         Success Criteria:
*         Packet is not trapped to CPU and Learned on FDB.
*/
GT_VOID prvTgfBrgGenIeeeReservedMcastLearningDisableTrapDisableTrafficGenerate
(
    void
)
{
    SET_PROFILE_AND_PROTOCOL;

    /* AUTODOC: disable IEEE Reserved Multicast Learning for trapped packets */
    /* AUTODOC: set Profile index 2 for port 1 */
    /* AUTODOC: set command FORWARD for profile index 2 and protocol 0x55 */
    /* AUTODOC: disable trap to CPU for IEEE Reserved Multicast packets */
    prvTgfBrgGenIeeeReservedMcastLearningTrafficGenerate(GT_FALSE, /* disable IEEE learning*/
                                                         prvTgfBrgGenIeeeReservedMcastProfileTableIndex, /* profile table index */
                                                         prvTgfBrgGenIeeeReservedMcastProtocol, /* prtocol */
                                                         CPSS_PACKET_CMD_FORWARD_E, /* packt command */
                                                         GT_FALSE /* enable trap of IEEE Reserved */
                                                         );
    /* AUTODOC: verify packet not trapped to CPU and learned on FDB */
}


/**
* @internal prvTgfBrgGenIeeeReservedMcastLearningConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgGenIeeeReservedMcastLearningConfigurationRestore
(
    void
)
{
    GT_STATUS   rc = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /****************/
    /* Tables Reset */
    /****************/

    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);


    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);
}


