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
* @file prvTgfFdbLocalMcSwitching.c
*
* @brief Verify that packets with same MC location (port) as original received.
*
* @version   4
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfTrunkGen.h>

#include <bridge/prvTgfFdbLocalMcSwitching.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* capture port index */
#define PRV_TGF_CPORT_IDX_CNS                   1

/* default vlanId */
#define PRV_TGF_VLAN_ID_CNS                     2

/* default VIDX */
#define PRV_TGF_VIDX_CNS                        1

/* default trunk index */
#define PRV_TGF_TRUNK_ID_CNS                    2

/* trunk enabled members num */
#define PRV_TGF_EMEMBERS_NUM_CNS                2

/* trunk enabled members index array */
static GT_U32 prvTgfTrunkMembersIdx[PRV_TGF_EMEMBERS_NUM_CNS] = {0, 3};

/* default FDB portIdx */
#define PRV_TGF_FDB_PORT_IDX_CNS                1

/* portIdx to send traffic to */
#define PRV_TGF_TX_PORT_IDX_CNS                 0

/* default portIdx of Mc packet switching to */
#define PRV_TGF_SWITCHING_PORT_IDX_CNS          0

/* default traffic Tx delay in mS */
#define PRV_TGF_TX_DELAY_CNS                    100

/* default capturing mode */
#define PRV_TGF_DEF_CAPTURE_MODE_CNS            TGF_CAPTURE_MODE_PCL_E

/* total tests stages */
#define PRV_TGF_STAGE_NUM_CNS                   3

/* default number of packets to send */
static GT_U32  prvTgfBurstCount               = 10;

/* expected number of Rx and Tx packets on ports*/
static GT_U8 prvTgfPacketsCountRxTxArr[][PRV_TGF_PORTS_NUM_CNS] =
{
    {1, 0, 0, 0},  /* Rx count for mode with switching              */
    {1, 1, 1, 0},  /* Tx count for mode with switching              */
    {1, 0, 0, 0},  /* Rx count after adding FDB entry               */
    {1, 0, 0, 0},  /* Tx count after adding FDB entry               */
    {1, 0, 0, 0},  /* Rx count with disabled switching              */
    {1, 0, 0, 0},  /* Tx count with disabled switching              */
};

/* default stage number to enable MC packet switching */
#define PRV_TGF_ENABLE_SWITCHING_STAGE_NUM_CNS      0

/* default stages number to add MAC entry at FDB table */
#define PRV_TGF_ADD_ENTRY_STAGE_NUM_CNS             1

/* default stage number to disable MC packet switching */
#define PRV_TGF_DISABLE_SWITCHING_STAGE_NUM_CNS     2

/* expected capture triggers */
static GT_U32  prvTgfExpTriggersArr[] =
{
    0x0FFFFF,   /* vfd[0]=1, vfd[1]=1 for mode with switching         */
    0x00,       /* vfd[0]=0, vfd[1]=0 after adding MAC entry entry    */
    0x00        /* vfd[0]=1, vfd[1]=1 with disabled switching         */
};

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x01, 0x00, 0x5E, 0x00, 0x00, 0x02},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}                /* srcMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLAN_ID_CNS                           /* pri, cfi, VlanId */
};

/* default packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart =
{
    TGF_ETHERTYPE_0800_IPV4_TAG_CNS
};

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part =
{
    4,                                  /* version                */
    5,                                  /* headerLen              */
    0,                                  /* typeOfService          */
    0x2E,                               /* totalLen = 46 (20+26)  */
    0,                                  /* id                     */
    0,                                  /* flags                  */
    0,                                  /* offset                 */
    0x40,                               /* timeToLive = 64        */
    0xFF,                               /* protocol - reserved    */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,  /* chksum                 */
    {192,   1,   1,   1},               /* srcAddr = ipSrc        */
    {224,   0,   0,   1}                /* dstAddr = ipGrp        */
};

/* data of packet, len = 26 (0x1a) */
static GT_U8 prvTgfPayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19
};

/* payload part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPayloadDataArr),       /* dataLength */
    prvTgfPayloadDataArr                /* dataPtr    */
};

/* parts of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},       /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},  /* VLAN_TAG part */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},/* ethertype     */
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},     /* packet's IPv4 */
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}   /* PAYLOAD part  */
};

/* length of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + \
    TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* length of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS  PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* packet to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen   */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static struct
{
    GT_BOOL  mcSwitching[PRV_TGF_MAX_PORTS_NUM_CNS];
} prvTgfRestoreCfg;

/******************************************************************************/


/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/
/**
* @internal prvTgfFdbLocalMcSwitchingIsPortIdxInTrunk function
* @endinternal
*
* @brief   Check PortIdx is a trunk member
*
* @param[in] portIdx                  - port index number
*
* @retval GT_TRUE                  - on portIdx is a trunk member
* @retval GT_FALSE                 - on portIdx is not a trunk member
*/
static GT_BOOL prvTgfFdbLocalMcSwitchingIsPortIdxInTrunk
(
    IN GT_U32      portIdx
)
{
    GT_U32          trunkMemb  = 0;

    /* iterate thru all trunk members*/
    for (trunkMemb = 0; trunkMemb < PRV_TGF_EMEMBERS_NUM_CNS; trunkMemb++ )
    {
        /* check port index is equal to trunk member index */
        if (portIdx == prvTgfTrunkMembersIdx[trunkMemb])
        {
            return GT_TRUE;
        }
    }

    return GT_FALSE;
}

/**
* @internal prvTgfFdbLocalMcSwitchingTrunkEgressPacketNumGet function
* @endinternal
*
* @brief   Get expected egress packet number of all trunk members
*
* @param[in] stageNum                 - Traffic generate test stage number
*                                       Summary egress packets number for all members of trunk
*/
static GT_U32 prvTgfFdbLocalMcSwitchingTrunkEgressPacketNumGet
(
    IN  GT_U32      stageNum
)
{
    GT_U32          portIter    = 0;
    GT_U32          egressCount = 0;
    GT_U32          rowTab      = 0;

    /* calculate row number of expected results array */
    rowTab = 1 + (stageNum * 2);

    /* iterate thru all ports */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* add counter  if port is a trunk member */
        if (GT_TRUE == prvTgfFdbLocalMcSwitchingIsPortIdxInTrunk(portIter))
        {
            /* expected number of packets */
            egressCount += prvTgfPacketsCountRxTxArr[rowTab][portIter];
        }
    }

    return egressCount;
}


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/
/**
* @internal prvTgfFdbLocalMcSwitchingConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfFdbLocalMcSwitchingConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc          = GT_OK;

    GT_U32                  portIter = 0;
    GT_TRUNK_ID             trunkId;
    CPSS_TRUNK_MEMBER_STC   enabledMembersArray[PRV_TGF_EMEMBERS_NUM_CNS];

    /* AUTODOC: SETUP CONFIGURATION: */

    PRV_UTF_LOG0_MAC("\n========== Setup config =====================\n\n");

    /* calculate trunk id */
    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    /* fill enabled members parameters array */
    for (portIter = 0; portIter < PRV_TGF_EMEMBERS_NUM_CNS; portIter++)
    {
        enabledMembersArray[portIter].hwDevice = prvTgfDevNum;
        enabledMembersArray[portIter].port = prvTgfPortsArray[prvTgfTrunkMembersIdx[portIter]];
    }

    /* AUTODOC: create trunk 2 with ports [0,3] */
    rc = prvTgfTrunkMembersSet(trunkId, 
                               PRV_TGF_EMEMBERS_NUM_CNS, 
                               enabledMembersArray, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, 
                                 "prvTgfTrunkMembersSet: %d", prvTgfDevNum);

    /* AUTODOC: create VLAN 2 with all untagged ports */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLAN_ID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, 
                                 "Error of prvTgfBrgDefVlanEntryWrite");

    rc = prvTgfBrgPortEgressMcLocalEnableGet(prvTgfDevNum, 
                                             prvTgfPortsArray[PRV_TGF_SWITCHING_PORT_IDX_CNS],
                                             &prvTgfRestoreCfg.mcSwitching[PRV_TGF_SWITCHING_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, 
                                 "Error of prvTgfBrgPortEgressMcLocalEnableGet");
}

/**
* @internal prvTgfFdbLocalMcSwitchingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfFdbLocalMcSwitchingTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS   rc                      = GT_OK;
    GT_U32      portIter                = 0;
    GT_U32      expPacketLen            = 0;
    GT_U32      numTriggers             = 0;
    GT_U32      rxPacketsCount          = 0;
    GT_U32      txPacketsCount          = 0;
    GT_U32      stageNum                = 0;
    GT_U32      expectPacketNumOnTrunk  = 0;

    CPSS_INTERFACE_INFO_STC         portInterface   = {0};
    TGF_VFD_INFO_STC                vfdArray[2]     = {{0}};
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs       = {{{0}}};

    /* AUTODOC: GENERATE TRAFFIC: */

    /* AUTODOC: Iterate thru 3 stages: */
    for (stageNum = 0; stageNum < PRV_TGF_STAGE_NUM_CNS; stageNum++ )
    {
        PRV_UTF_LOG1_MAC("\n====== Generate traffic stage #%d ======== \n", stageNum+1);

        /* select mode and print log messages */
        switch (stageNum)
        {
            case PRV_TGF_ENABLE_SWITCHING_STAGE_NUM_CNS:
                PRV_UTF_LOG0_MAC("\nEnable MC packet switching\n");

                /* AUTODOC: for Stage#1 enable MC local switching for port 0 */
                rc = prvTgfBrgPortEgressMcLocalEnable(prvTgfDevNum, 
                                                      prvTgfPortsArray[PRV_TGF_SWITCHING_PORT_IDX_CNS],
                                                      GT_TRUE);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgPortEgressMcLocalEnable");

                break;

            case PRV_TGF_DISABLE_SWITCHING_STAGE_NUM_CNS:
                PRV_UTF_LOG0_MAC("\nDisable MC packet switching\n");

                /* AUTODOC: for Stage#3 disable MC local switching for port 0 */
                rc = prvTgfBrgPortEgressMcLocalEnable(prvTgfDevNum, 
                                                      prvTgfPortsArray[PRV_TGF_SWITCHING_PORT_IDX_CNS],
                                                      GT_FALSE);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgPortEgressMcLocalEnable");

                break;

            case PRV_TGF_ADD_ENTRY_STAGE_NUM_CNS:
                PRV_UTF_LOG0_MAC("\nAdd MAC entry on switching port\n");

                /* AUTODOC: for Stage#2 add FDB entry with MAC 01:00:5e:00:00:02, VLAN 2, VIDX 1 */
                rc = prvTgfBrgDefFdbMacEntryOnVidxSet( prvTgfPacketL2Part.daMac, 
                                                     PRV_TGF_VLAN_ID_CNS, 
                                                     PRV_TGF_VIDX_CNS, 
                                                     GT_FALSE);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgDefFdbMacEntryOnVidxSet");

                break;

            default:
                break;
        }

        /* clear capturing RxPcktTable */
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, 
                                     "ERROR of tgfTrafficTableRxPcktTblClear");

        /* reset all counters */
        rc = prvTgfEthCountersReset(prvTgfDevNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, 
                                     "ERROR of prvTgfEthCountersReset");

        /* enable capture */
        portInterface.type            = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum  = prvTgfDevNum;
        portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_CPORT_IDX_CNS];
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, 
                                                    PRV_TGF_DEF_CAPTURE_MODE_CNS, 
                                                    GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, 
                                     "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet\n");

        /* setup packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, 
                                 prvTgfBurstCount, 0, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, 
                                     "ERROR of SetTxSetupEth: dev=%d, burst=%d",
                                     prvTgfDevNum, prvTgfBurstCount);

        /* set Tx delay */
        rc = prvTgfSetTxSetup2Eth(1, PRV_TGF_TX_DELAY_CNS);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, 
                                     "ERROR of prvTgfSetTxSetup2Eth");

        /* AUTODOC: for each stage send 10 IPv4 UC packets on port 0 with: */
        /* AUTODOC:   DA=01:00:5e:00:00:02, SA=00:00:00:00:00:00, VID=2 */
        /* AUTODOC:   sIP=192.1.1.1, dIP=224.0.0.1 */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, 
                                       prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]); 
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, 
                                     "ERROR of StartTransmitingEth: dev=%d, port=%d",
                                     prvTgfDevNum, 
                                     prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]);

        /* remove Tx delay */
        rc = prvTgfSetTxSetup2Eth(0, 0);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, 
                                     "ERROR of prvTgfSetTxSetup2Eth");

        /* disable capture */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, 
                                                    PRV_TGF_DEF_CAPTURE_MODE_CNS, 
                                                    GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, 
                                     "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                     prvTgfDevNum, portInterface.devPort.portNum);

        /* get expected egress packets num on trunk for this stage */
        expectPacketNumOnTrunk = prvTgfFdbLocalMcSwitchingTrunkEgressPacketNumGet(stageNum);

        /* correct by burstCount */
        expectPacketNumOnTrunk *= prvTgfBurstCount;

        PRV_UTF_LOG1_MAC("\nexpectPacketNumOnTrunk = %d\n", expectPacketNumOnTrunk);

        /* AUTODOC: verify traffic: */
        /* AUTODOC:   1 stage - 10 packets received on ports 1,2 */
        /* AUTODOC:   2 stage - traffic filtered */
        /* AUTODOC:   3 stage - traffic filtered */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {           
            /* expected number of Tx packets */
            txPacketsCount = prvTgfPacketsCountRxTxArr[1 + (stageNum*2)][portIter];

            /* expected number of Rx packets */
            rxPacketsCount = prvTgfPacketsCountRxTxArr[0 + (stageNum*2)][portIter]+
                             ((PRV_TGF_CPORT_IDX_CNS == portIter) * txPacketsCount);

            /* expected packet size, packet exit without VLAN tag */
            if (portIter == PRV_TGF_TX_PORT_IDX_CNS)
            {
                expPacketLen =  PRV_TGF_PACKET_LEN_CNS;
            }
            else
            {
                expPacketLen =  PRV_TGF_PACKET_LEN_CNS - 4;
            }

            /* check ETH counters */
            if (GT_TRUE == prvTgfFdbLocalMcSwitchingIsPortIdxInTrunk(portIter))
            {
                /* special case for trunk members */
                PRV_UTF_LOG0_MAC("\nPort is trunkMember\n");

                /* read counters with print enabled */
                rc = prvTgfReadPortCountersEth(prvTgfDevNum, 
                                               prvTgfPortsArray[portIter], 
                                               GT_TRUE, &portCntrs);

                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, 
                                             "ERROR of prvTgfReadPortCountersEth Port=%02d\n",
                                             prvTgfPortsArray[portIter]);

                UTF_VERIFY_EQUAL0_STRING_MAC(rxPacketsCount * prvTgfBurstCount, 
                                             portCntrs.goodPktsRcv.l[0],
                                             "got another goodPktsRcv counter than expected");

                UTF_VERIFY_EQUAL0_STRING_MAC((rxPacketsCount>0) * 
                                             (expPacketLen + TGF_CRC_LEN_CNS) * prvTgfBurstCount, 
                                             portCntrs.goodOctetsRcv.l[0],
                                             "got another goodOctetsRcv counter than expected");

                /* decrease expected counter on trunk members by egress packets num */
                expectPacketNumOnTrunk -= portCntrs.goodPktsSent.l[0];

                /* check received packet length (if need) */
                if (0 < portCntrs.goodPktsSent.l[0])
                {
                    UTF_VERIFY_EQUAL0_STRING_MAC((expPacketLen + TGF_CRC_LEN_CNS) * prvTgfBurstCount, 
                                                 portCntrs.goodOctetsSent.l[0],
                                                 "got another goodOctetsSent counter than expected");
                }
            }
            else
            {
                /* check ETH counters in common case */
                PRV_UTF_LOG0_MAC("\nPort not in trunk\n");

                rc = prvTgfEthCountersCheck(prvTgfDevNum, prvTgfPortsArray[portIter], 
                                            rxPacketsCount, txPacketsCount, expPacketLen,
                                            prvTgfBurstCount);

                UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, 
                                             "ERROR of prvTgfEthCountersCheck"\
                                             "  Port=%02d  expectedRx=%02d"\
                                             "  expectedTx=%02d"\
                                             "  expectedLen=%02d\n",
                                             prvTgfPortsArray[portIter],
                                             rxPacketsCount, txPacketsCount, 
                                             expPacketLen);
            }
        }
        UTF_VERIFY_EQUAL0_STRING_MAC(expectPacketNumOnTrunk, 0,
                                     "illegal expected egress packets counter on trunk members");

        /* get trigger counters */
        PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

        /* get trigger counters where packet has MAC DA as macAddr */
        vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
        vfdArray[0].modeExtraInfo = 0;
        vfdArray[0].offset = 0;
        vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
        cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

        /* get trigger counters where packet has MAC SA as macAddr */
        vfdArray[1].mode = TGF_VFD_MODE_STATIC_E;
        vfdArray[1].modeExtraInfo = 0;
        vfdArray[1].offset = sizeof(TGF_MAC_ADDR);
        vfdArray[1].cycleCount = sizeof(TGF_MAC_ADDR);
        cpssOsMemCpy(vfdArray[1].patternPtr, prvTgfPacketL2Part.saMac, sizeof(TGF_MAC_ADDR));

        rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface,
                                                            sizeof(vfdArray) / sizeof(vfdArray[0]),
                                                            vfdArray, &numTriggers);
        PRV_UTF_LOG1_MAC("numTriggers = %d\n", numTriggers);

        /* check TriggerCounters */
        rc = rc == GT_NO_MORE ? GT_OK : rc;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, 
                                     "ERROR of tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                     portInterface.devPort.hwDevNum, 
                                     portInterface.devPort.portNum);

        /* check if captured packet has the same MAC DA as prvTgfArpMac */
        PRV_UTF_LOG1_MAC("expTriggers = %d\n\n", prvTgfExpTriggersArr[stageNum]);

        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfExpTriggersArr[stageNum], 
                                     numTriggers,
                                     "\n ERROR: Got another MAC DA/SA\n");
    }
}

/**
* @internal prvTgfFdbLocalMcSwitchingConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbLocalMcSwitchingConfigRestore
(
    GT_VOID
)
{
    GT_STATUS       rc          = GT_OK;
    GT_TRUNK_ID     trunkId     = 0;

    /* AUTODOC: RESTORE CONFIGURATION: */

    PRV_UTF_LOG0_MAC("\n========== Restore config: ==================\n\n");

    /* AUTODOC: restore MC local switching state for port 0 */
    rc = prvTgfBrgPortEgressMcLocalEnable(prvTgfDevNum, 
                                          prvTgfPortsArray[PRV_TGF_SWITCHING_PORT_IDX_CNS],
                                          prvTgfRestoreCfg.mcSwitching[PRV_TGF_SWITCHING_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, 
                                 "Error of prvTgfBrgPortEgressMcLocalEnable");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, 
                                 "ERROR of tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgFdbFlush");

    /* calculate trunk id */
    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    /* AUTODOC: remove trunk 2 */
    rc = prvTgfTrunkMembersSet(trunkId, 0, NULL, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, 
                                 "ERROR of prvTgfTrunkMemberRemove: trunk %d", trunkId);

    /* invalidate and reset vlan entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLAN_ID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, 
                                 "ERROR of prvTgfBrgDefVlanEntryInvalidate");
}


