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
* @file prvTgfFdbChangeInterfaceType.c
*
* @brief Change Interface Type (port, trunk, vid, vidx)
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
#include <bridge/prvTgfFdbChangeInterfaceType.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN0 Id */
#define PRV_TGF_VLANID_CNS            5

/* port index to send traffic to */
#define PRV_TGF_SEND1_PORT_IDX_CNS    0

/* port index to send traffic to */
#define PRV_TGF_SEND2_PORT_IDX_CNS    1

/* trunk index */
#define PRV_TGF_TRUNK_ID_CNS          2

/* generate traffic vidx */
#define PRV_TGF_VIDX_CNS              1

/* capture port index */
#define PRV_TGF_CPORT_IDX_CNS         3

/* trunk enabled members num */
#define PRV_TGF_EMEMBERS_NUM_CNS      2

/* trunk enabled members index array */
static GT_U32 prvTgfTrunkMembersIdx[PRV_TGF_EMEMBERS_NUM_CNS] = {1, 2};

/* default number of packets to send */
static GT_U32  prvTgfBurstCount = 1;

/* expected number of Rx and Tx packets on ports*/
static GT_U8 prvTgfPacketsCountRxTxArr[][PRV_TGF_PORTS_NUM_CNS] =
{
    {0, 1, 0, 0},  /* Rx count for Port trafic  */
    {1, 1, 0, 0},  /* Tx count for Port trafic  */
    {1, 0, 0, 0},  /* Rx count for VID  trafic  */
    {1, 1, 0, 1},  /* Tx count for VID  trafic  */
    {1, 0, 0, 0},  /* Rx count for Trunk trafic */
    {1, 1, 0, 0},  /* Tx count for Trunk trafic */
    {1, 0, 0, 0},  /* Rx count for VidX trafic  */
    {1, 0, 1, 1}   /* Tx count for VidX trafic  */
};

/* change interfaces tests sequence */
static CPSS_INTERFACE_TYPE_ENT  prvTgfInterfaceTestSequence[] =
{
    CPSS_INTERFACE_PORT_E,
    CPSS_INTERFACE_VID_E,
    CPSS_INTERFACE_TRUNK_E,
    CPSS_INTERFACE_VIDX_E
};

/* length of tests sequence */
#define PRV_TGF_SEQUENCE_LEN_CNS   \
    sizeof(prvTgfInterfaceTestSequence)/sizeof(prvTgfInterfaceTestSequence[0])

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x11, 0x11}                /* srcMac */
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
    sizeof(prvTgfPayloadDataArr),                           /* dataLength */
    prvTgfPayloadDataArr                                    /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},      /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS  PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};
/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static struct
{
    CPSS_PORTS_BMP_STC  defPortMembers;
} prvTgfRestoreCfg;

/******************************************************************************/


/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/
/**
* @internal prvTgfFdbChangeInterfaceTypeIsPortIdxInTrunk function
* @endinternal
*
* @brief   Check PortIdx is a trunk member
*
* @param[in] portIdx                  - port index number
*
* @retval GT_TRUE                  - on portIdx is a trunk member
* @retval GT_FALSE                 - on portIdx is not a trunk member
*/
static GT_BOOL prvTgfFdbChangeInterfaceTypeIsPortIdxInTrunk
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
* @internal prvTgfFdbChangeInterfaceTrunkEgressedPacketNumGet function
* @endinternal
*
* @brief   Get summary egress packet number for all members of trunk
*
* @param[in] stageNum                 - Traffic generate test stage number
*                                       Summary egress packet number for all members of trunk
*/
static GT_U32 prvTgfFdbChangeInterfaceTrunkEgressedPacketNumGet
(
    IN  GT_U32      stageNum
)
{
    GT_U32          portIter    = 0;
    GT_U32          egressCount = 0;

    /* iterate thru all ports */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* add counter  if port is a trunk member */
        if (GT_TRUE == prvTgfFdbChangeInterfaceTypeIsPortIdxInTrunk(portIter))
        {
            /* expected number of packets */
            egressCount += prvTgfPacketsCountRxTxArr[1 + (stageNum * 2)][portIter];
        }
    }

    return egressCount;
}


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfFdbChangeInterfaceTypeConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfFdbChangeInterfaceTypeConfigSet
(
    GT_VOID
)
{
    GT_STATUS               rc = GT_OK;
    GT_U32                  portIter = 0;
    CPSS_PORTS_BMP_STC      portBitmap;
    GT_TRUNK_ID             trunkId;
    CPSS_TRUNK_MEMBER_STC   enabledMembersArray[PRV_TGF_EMEMBERS_NUM_CNS];

    /* AUTODOC: SETUP CONFIGURATION: */

    PRV_UTF_LOG0_MAC("\n========== Setup config =====================\n\n");

    /* AUTODOC: create VLAN 5 with all untagged ports */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite(%d)\n",
                                 PRV_TGF_VLANID_CNS);

    /* save default port members for MC group */
    rc = prvTgfBrgMcEntryRead(prvTgfDevNum, PRV_TGF_VIDX_CNS,
                              &(prvTgfRestoreCfg.defPortMembers));
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgMcEntryRead: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VIDX_CNS);

    /* set MC group bitmap {0x840100, 0x00} */
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portBitmap);
    for (portIter = 1; portIter < prvTgfPortsNum; portIter++)
    {
        CPSS_PORTS_BMP_PORT_SET_MAC(&portBitmap, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: create VIDX 1 with ports 1, 2, 3 */
    rc = prvTgfBrgMcEntryWrite(prvTgfDevNum, PRV_TGF_VIDX_CNS, &portBitmap);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgMcEntryWrite: %d",
                                 prvTgfDevNum);

    /* calculate trunk id */
    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    /* fill enabled members parameters array */
    for (portIter = 0; portIter < PRV_TGF_EMEMBERS_NUM_CNS; portIter++)
    {
        enabledMembersArray[portIter].hwDevice = prvTgfDevNum;
        enabledMembersArray[portIter].port = prvTgfPortsArray[prvTgfTrunkMembersIdx[portIter]];
    }

    /* AUTODOC: create TRUNK 2 with ports 1, 2 */
    rc = prvTgfTrunkMembersSet(trunkId, PRV_TGF_EMEMBERS_NUM_CNS, enabledMembersArray, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfTrunkMembersSet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfFdbChangeInterfaceTypeTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfFdbChangeInterfaceTypeTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS   rc             = GT_OK;
    GT_U32      portIter       = 0;
    GT_TRUNK_ID trunkId        = 0;
    GT_U32      stageNum       = 0;
    GT_U32      expPacketLen   = 0;
    GT_U32      etherPortNum   = 0;
    GT_U32      numTriggers    = 0;
    GT_U32      expTriggers    = 3;
    GT_U32      rxPacketsCount = 0;
    GT_U32      txPacketsCount = 0;
    GT_U32      expectPacketNumOnTrunk = 0;

    CPSS_INTERFACE_INFO_STC         portInterface = {0};
    TGF_VFD_INFO_STC                vfdArray[2];

    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* iterate thru stages */
    for (stageNum = 0; stageNum < PRV_TGF_SEQUENCE_LEN_CNS; stageNum++)
    {
        /* select different interface */
        switch (prvTgfInterfaceTestSequence[stageNum])
        {
            case CPSS_INTERFACE_PORT_E:
                PRV_UTF_LOG0_MAC("\n========== Traffic generate: PORT ===========\n\n");

                /* AUTODOC: add FDB entry with MAC 00:00:00:00:00:01, VLAN 5, port 0 */
                rc = prvTgfBrgDefFdbMacEntryOnPortSet( prvTgfPacketL2Part.daMac,
                                                     PRV_TGF_VLANID_CNS,
                                                     prvTgfDevNum,
                                                     prvTgfPortsArray[PRV_TGF_SEND1_PORT_IDX_CNS],
                                                     GT_FALSE);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgDefFdbMacEntryOnPortSet");

                break;

            case CPSS_INTERFACE_VID_E:
                if(prvTgfDevPortForceNum)
                {
                    PRV_UTF_LOG0_MAC("======================================= \n");
                    PRV_UTF_LOG0_MAC("========== Limitation for multi port group FDB lookup system : set FDB entry on interface VID is not allowed \n");
                    PRV_UTF_LOG0_MAC("========== skip VID interface ========== \n\n");
                    continue;
                }

                PRV_UTF_LOG0_MAC("\n========== Traffic generate: VID ============\n\n");


                /* AUTODOC: add FDB entry with MAC 00:00:00:00:00:01, VLAN 5, VID 5 */
                rc = prvTgfBrgDefFdbMacEntryOnVidSet( prvTgfPacketL2Part.daMac,
                                                    PRV_TGF_VLANID_CNS,
                                                    PRV_TGF_VLANID_CNS,
                                                    GT_FALSE);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"ERROR of prvTgfBrgDefFdbMacEntryOnVidSet");

                break;

            case CPSS_INTERFACE_TRUNK_E:
                PRV_UTF_LOG0_MAC("\n========== Traffic generate: TRUNK ==========\n\n");

                trunkId = PRV_TGF_TRUNK_ID_CNS;
                CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

                /* AUTODOC: add FDB entry with MAC 00:00:00:00:00:01, VLAN 5, TRUNK 2 */
                rc = prvTgfBrgDefFdbMacEntryOnTrunkSet( prvTgfPacketL2Part.daMac,
                                                      PRV_TGF_VLANID_CNS,
                                                      trunkId,
                                                      GT_FALSE);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgDefFdbMacEntryOnTrunkSet");

                break;

            case CPSS_INTERFACE_VIDX_E:
                PRV_UTF_LOG0_MAC("\n========== Traffic generate: VIDX ============\n\n");

                /* AUTODOC: add FDB entry with MAC 00:00:00:00:00:01, VLAN 5, VIDX 1 */
                rc = prvTgfBrgDefFdbMacEntryOnVidxSet( prvTgfPacketL2Part.daMac,
                                                     PRV_TGF_VLANID_CNS,
                                                     PRV_TGF_VIDX_CNS,
                                                     GT_FALSE);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgDefFdbMacEntryOnVidxSet");

                break;

            default:
                rc = GT_BAD_PARAM;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Illegal interface type: %d",
                                             prvTgfInterfaceTestSequence[stageNum]);
        }

        /* clear capturing RxPcktTable */
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficTableRxPcktTblClear");

        /* reset all counters */
        rc = prvTgfEthCountersReset(prvTgfDevNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersReset: dev=%d", prvTgfDevNum);

        if (CPSS_INTERFACE_VID_E == prvTgfInterfaceTestSequence[stageNum])
        {
            /* enable capture */
            portInterface.type            = CPSS_INTERFACE_PORT_E;
            portInterface.devPort.hwDevNum  = prvTgfDevNum;
            portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_CPORT_IDX_CNS];

            rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet\n");
        }

        /* setup packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, prvTgfBurstCount, 0, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetupEth: dev=%d, burst=%d",
                                     prvTgfDevNum, prvTgfBurstCount);

        /* select ethernet port number to transmit */
        etherPortNum = (CPSS_INTERFACE_PORT_E == prvTgfInterfaceTestSequence[stageNum]) ?
                        prvTgfPortsArray[PRV_TGF_SEND2_PORT_IDX_CNS] : prvTgfPortsArray[PRV_TGF_SEND1_PORT_IDX_CNS];

        /* send packet */
        /* AUTODOC: send 4 packets on ports [1, 0, 0, 0] with: */
        /* AUTODOC:   DA=00:00:00:00:00:01, SA=00:00:00:00:11:11 */
        /* AUTODOC:   VID=5 */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, etherPortNum);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitingEth: dev=%d, port=%d",
                                     prvTgfDevNum, etherPortNum);

        if (CPSS_INTERFACE_VID_E == prvTgfInterfaceTestSequence[stageNum])
        {
            /* disable capture */
            rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                         prvTgfDevNum, portInterface.devPort.portNum);
        }

        /* get expected egress packets num on trunk for this stage */
        expectPacketNumOnTrunk = prvTgfFdbChangeInterfaceTrunkEgressedPacketNumGet(stageNum);

        /* correct expected egress packets num in case if capturing port is trunk member */
        if ((CPSS_INTERFACE_VID_E == prvTgfInterfaceTestSequence[stageNum]) &&
            (GT_TRUE == prvTgfFdbChangeInterfaceTypeIsPortIdxInTrunk(PRV_TGF_CPORT_IDX_CNS) ))
        {
            expectPacketNumOnTrunk++;
        }

        /* AUTODOC: verify traffic to get on correct interface - port\vid\trunk\vidx */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {

            /* expected number of Rx packets */
            rxPacketsCount = prvTgfPacketsCountRxTxArr[0 + (stageNum * 2)][portIter] +
                             ((CPSS_INTERFACE_VID_E == prvTgfInterfaceTestSequence[stageNum]) &&
                              (PRV_TGF_CPORT_IDX_CNS == portIter));

            /* expected number of Tx packets */
            txPacketsCount = prvTgfPacketsCountRxTxArr[1 + (stageNum * 2)][portIter];

            /* expected packet size */
            expPacketLen = (prvTgfInterfaceTestSequence[stageNum] == CPSS_INTERFACE_PORT_E)?
                            PRV_TGF_PACKET_LEN_CNS - (TGF_VLAN_TAG_SIZE_CNS * (0 == portIter)):
                            PRV_TGF_PACKET_LEN_CNS - (TGF_VLAN_TAG_SIZE_CNS * (0 != portIter));

            /* special case for trunk members */
            if (GT_TRUE == prvTgfFdbChangeInterfaceTypeIsPortIdxInTrunk(portIter))
            {
                /* read counters with print enabled */
                rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfReadPortCountersEth Port=%02d\n",
                                             prvTgfPortsArray[portIter]);

                UTF_VERIFY_EQUAL0_STRING_MAC( rxPacketsCount, portCntrs.goodPktsRcv.l[0],
                                             "get another goodPktsRcv counter than expected");

                UTF_VERIFY_EQUAL0_STRING_MAC((rxPacketsCount>0) * (expPacketLen + TGF_CRC_LEN_CNS),
                                              portCntrs.goodOctetsRcv.l[0],
                                             "get another goodOctetsRcv counter than expected");

                /* decrease expected counter on trunk members by egress packets num */
                expectPacketNumOnTrunk -= portCntrs.goodPktsSent.l[0];

                /* check received packet length (if need) */
                if (0 < portCntrs.goodPktsSent.l[0])
                {
                    UTF_VERIFY_EQUAL0_STRING_MAC(expPacketLen + TGF_CRC_LEN_CNS, portCntrs.goodOctetsSent.l[0],
                                                  "get another goodOctetsSent counter than expected");
                }

                /* exit from special case for trunk members */
                continue;
            }

            /* check ETH counters */
            rc = prvTgfEthCountersCheck(prvTgfDevNum, prvTgfPortsArray[portIter],
                                        rxPacketsCount, txPacketsCount, expPacketLen,
                                        prvTgfBurstCount);
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck  Port=%02d  ExpectedRx=%02d  ExpectedTx=%02d BurstCnt=%d\n",
                                         prvTgfPortsArray[portIter],
                                         rxPacketsCount, txPacketsCount,
                                         prvTgfBurstCount);
        }

        UTF_VERIFY_EQUAL0_STRING_MAC(expectPacketNumOnTrunk, 0,
                                     "get another trunk members egress packets counter than expected");


        if (CPSS_INTERFACE_VID_E == prvTgfInterfaceTestSequence[stageNum])
        {
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
            PRV_UTF_LOG2_MAC("numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

            /* check TriggerCounters */
            rc = rc == GT_NO_MORE ? GT_OK : rc;
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                         portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

            /* check if captured packet has the same MAC DA as prvTgfArpMac */
            PRV_UTF_LOG1_MAC("expTriggers = %d\n\n", expTriggers);

            UTF_VERIFY_EQUAL0_STRING_MAC(expTriggers, numTriggers,"\n ERROR: Got another MAC DA/SA\n");
        }
    }
}

/**
* @internal prvTgfFdbChangeInterfaceTypeConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbChangeInterfaceTypeConfigRestore
(
    GT_VOID
)
{
    GT_STATUS           rc = GT_OK;
    GT_TRUNK_ID         trunkId;

    /* AUTODOC: RESTORE CONFIGURATION: */

    PRV_UTF_LOG0_MAC("\n========== Restore config: ==================\n\n");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgFdbFlush");

    /* AUTODOC: clear VIDX 1 */
    rc = prvTgfBrgMcEntryWrite(prvTgfDevNum, PRV_TGF_VIDX_CNS,
                               &(prvTgfRestoreCfg.defPortMembers));
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgMcEntryWrite: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VIDX_CNS);

    /* calculate trunk id */
    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    /* AUTODOC: clear TRUNK 2 */
    rc = prvTgfTrunkMembersSet(trunkId, 0, NULL, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfTrunkMemberRemove: trunk %d", trunkId);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgDefVlanEntryInvalidate: vlan %d", PRV_TGF_VLANID_CNS);
}


