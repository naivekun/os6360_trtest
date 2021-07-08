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
* @file prvTgfBrgStpEgressFilteringChangeStates.c
*
* @brief Verify that changing STP state works properly.
*
* @version   6
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <bridge/prvTgfBrgStpEgressFilteringChangeStates.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/*  STG Id */
#define PRV_TGF_STG_ID_CNS                      1
static GT_U16   tempStg = PRV_TGF_STG_ID_CNS;

/*  default VLAN Id */
#define PRV_TGF_DEF_VLANID_CNS                  1

/*  STP config port Id */
#define PRV_TGF_STP_PORT_CONF_CNS               1

/*  VLAN Id 3 */
#define PRV_TGF_VLANID_2_CNS                    2

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS               0

/* capturing port number  */
#define PRV_TGF_CAPTURE_PORT_IDX_CNS            2

/* namber of packets to send */
#define PRV_TGF_SEND_PACKETS_NUM_CNS            4

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 1;

/* array of destination MACs for the tests */
static TGF_MAC_ADDR prvTgfDaMacArr[] =
{
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},
    {0x01, 0x80, 0xc2, 0x00, 0x00, 0x00}
};

/* array of source MACs for the tests */
static TGF_MAC_ADDR prvTgfSaMacArr[2*PRV_TGF_SEND_PACKETS_NUM_CNS] =
{
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x02},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x03},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x04},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x05},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x06},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x07},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x08}
};

/* expected number of Rx packets on ports*/
static GT_U8 prvTgfPacketsCountRxArr[PRV_TGF_SEND_PACKETS_NUM_CNS][PRV_TGF_PORTS_NUM_CNS] =
{
    {2, 2, 0, 0},
    {2, 0, 2, 0},
    {2, 2, 0, 0},
    {2, 0, 2, 0}
};

/* expected number of Tx packets on ports*/
static GT_U8 prvTgfPacketsCountTxArr[PRV_TGF_SEND_PACKETS_NUM_CNS][PRV_TGF_PORTS_NUM_CNS] =
{
    {2, 2, 2, 2},
    {2, 0, 2, 2},
    {2, 2, 2, 2},
    {2, 0, 2, 2}
};

/* expected number of Triggers */
static GT_U8 prvTgfPacketsCountTriggerArr[PRV_TGF_SEND_PACKETS_NUM_CNS] =
{
    2, 2, 2, 2
};

/* capture portIdx array for the tests */
static GT_U8 prvTgfCapturePortIdxArr[PRV_TGF_SEND_PACKETS_NUM_CNS] =
{
    1, 2, 1, 2
};

/* ports state config array for the tests */
static CPSS_STP_STATE_ENT prvTgfStpPortStateArr[PRV_TGF_SEND_PACKETS_NUM_CNS] =
{
    CPSS_STP_FRWRD_E,
    CPSS_STP_LRN_E,
    CPSS_STP_DISABLED_E,
    CPSS_STP_BLCK_LSTN_E
};


/******************************* Test packet **********************************/
/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}                /* srcMac */
};

/* First VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_2_CNS                          /* pri, cfi, VlanId */
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


/******************************************************************************/
/*************************** Restore config ***********************************/

/* Parameters needed to be restored */

/* parametrs for restore default Vlan entry */
static CPSS_PORTS_BMP_STC                  prvTgfDefPortsMembers;
static CPSS_PORTS_BMP_STC                  prvTgfDefPortsTagging;
static GT_BOOL                             prvTgfDefIsValid;
static PRV_TGF_BRG_VLAN_INFO_STC           prvTgfDefVlanInfo;
static PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC  prvTgfDefPortsTaggingCmd;

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfBrgStpEgressFilteringChangeStatesConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfBrgStpEgressFilteringChangeStatesConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U8       tagArray[]  = {1, 1, 1, 1};

    /* AUTODOC: SETUP CONFIGURATION: */

    /* save default Vlan entry parametrs */
    rc = prvTgfBrgVlanEntryRead(prvTgfDevNum, PRV_TGF_DEF_VLANID_CNS,
                                &prvTgfDefPortsMembers,
                                &prvTgfDefPortsTagging,
                                &prvTgfDefIsValid,
                                &prvTgfDefVlanInfo,
                                &prvTgfDefPortsTaggingCmd);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate 0");

    /* AUTODOC: invalidate VLAN 1 entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_DEF_VLANID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate 0");

    /* AUTODOC: create VLAN 2 with all tagged ports */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_2_CNS,
                                           prvTgfPortsArray,
                                           NULL, tagArray, 4);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_PUMA3_E)
    {
        /* puma3 supports STG == vid */
        tempStg = PRV_TGF_VLANID_2_CNS;
    }
    else
    {
        tempStg = PRV_TGF_STG_ID_CNS;
    }

    /* AUTODOC: bind VLAN 2 to STG Id 1 */
    rc = prvTgfBrgVlanStgIdBind(PRV_TGF_VLANID_2_CNS, tempStg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanStgIdBind: %d", prvTgfDevNum);
}

/**
* @internal prvTgfBrgStpEgressFilteringChangeStatesTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBrgStpEgressFilteringChangeStatesTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                   rc = GT_OK;
    GT_U32                      portIter = 0;
    GT_U32                      sendIter = 0;
    GT_U32                      numTriggers = 0;
    TGF_VFD_INFO_STC            vfdArray[1];
    CPSS_INTERFACE_INFO_STC     portInterface;


    /* AUTODOC: GENERATE TRAFFIC: */

    /* AUTODOC: Iterate thru 4 stages: */
    for (sendIter = 0; sendIter < PRV_TGF_SEND_PACKETS_NUM_CNS; sendIter++)
    {
        /* AUTODOC: set following STP states for port 1: */
        /* AUTODOC:   stage#1 - STP_FRWRD */
        /* AUTODOC:   stage#2 - STP_LRN */
        /* AUTODOC:   stage#3 - STP_DISABLED */
        /* AUTODOC:   stage#4 - STP_BLCK_LSTN */
        rc = prvTgfBrgStpPortStateSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_STP_PORT_CONF_CNS],
                                      tempStg, prvTgfStpPortStateArr[sendIter]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgStpPortStateSet: %d", prvTgfDevNum);

        /* reset counters */
        rc = prvTgfEthCountersReset(prvTgfDevNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

        /* set destination MAC address in Packet */
        cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfDaMacArr[0],
                     sizeof(prvTgfPacketL2Part.daMac));

        /* set source MAC address in Packet */
        cpssOsMemCpy(prvTgfPacketL2Part.saMac, prvTgfSaMacArr[2*sendIter],
                     sizeof(prvTgfPacketL2Part.saMac));

        /* setup packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfTaggedPacketInfo,
                                 prvTgfBurstCount, 0, NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup");

        PRV_UTF_LOG2_MAC("\n========== Sending packet iter=%d  on port=%d==========\n\n",
                         sendIter + 1, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* clear capturing RxPcktTable */
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

        /* enable capture */
        portInterface.type            = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum  = prvTgfDevNum;
        portInterface.devPort.portNum = prvTgfPortsArray[prvTgfCapturePortIdxArr[sendIter]];

        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                     prvTgfDevNum, prvTgfPortsArray[1]);

        /* AUTODOC: send packet on port 0 with: */
        /* AUTODOC:   1 stage - DA=00:00:00:00:34:02, SA=00:00:00:00:00:01, VID=2 */
        /* AUTODOC:   2 stage - DA=00:00:00:00:34:02, SA=00:00:00:00:00:03, VID=2 */
        /* AUTODOC:   3 stage - DA=00:00:00:00:34:02, SA=00:00:00:00:00:05, VID=2 */
        /* AUTODOC:   4 stage - DA=00:00:00:00:34:02, SA=00:00:00:00:00:07, VID=2 */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* disable capture */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                         prvTgfDevNum, portInterface.devPort.portNum);

        /* set destination MAC address in Packet */
        cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfDaMacArr[1],
                     sizeof(prvTgfPacketL2Part.daMac));

        /* set source MAC address in Packet */
        cpssOsMemCpy(prvTgfPacketL2Part.saMac, prvTgfSaMacArr[2*sendIter+1],
                     sizeof(prvTgfPacketL2Part.saMac));

        /* setup packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfTaggedPacketInfo,
                                 prvTgfBurstCount, 0, NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup");

        PRV_UTF_LOG2_MAC("\n========== Sending packet iter=%d  on port=%d==========\n\n",
                         sendIter + 1, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* enable capture */
        portInterface.type            = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum  = prvTgfDevNum;
        portInterface.devPort.portNum = prvTgfPortsArray[prvTgfCapturePortIdxArr[sendIter]];

        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                     prvTgfDevNum, prvTgfPortsArray[1]);

        /* AUTODOC: send packet on port 0 with: */
        /* AUTODOC:   1 stage - DA=01:80:c2:00:00:00, SA=00:00:00:00:00:02, VID=2 */
        /* AUTODOC:   2 stage - DA=01:80:c2:00:00:00, SA=00:00:00:00:00:04, VID=2 */
        /* AUTODOC:   3 stage - DA=01:80:c2:00:00:00, SA=00:00:00:00:00:06, VID=2 */
        /* AUTODOC:   4 stage - DA=01:80:c2:00:00:00, SA=00:00:00:00:00:08, VID=2 */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* disable capture */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                         prvTgfDevNum, portInterface .devPort.portNum);

        /* AUTODOC: verify traffic: */
        /* AUTODOC:   1 stage - 2 packets received on ports 1,2,3 */
        /* AUTODOC:   2 stage - 2 packets received on ports 2,3 */
        /* AUTODOC:   3 stage - 2 packets received on ports 1,2,3 */
        /* AUTODOC:   4 stage - 2 packets received on ports 2,3 */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            /* check ETH counters */
            rc = prvTgfEthCountersCheck(prvTgfDevNum,
                                        prvTgfPortsArray[portIter],
                                        prvTgfPacketsCountRxArr[sendIter][portIter],
                                        prvTgfPacketsCountTxArr[sendIter][portIter],
                                        PRV_TGF_TAGGED_PACKET_LEN_CNS, prvTgfBurstCount);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
        }

        /* get Trigger Counters */
        PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

        /* clear VFD array */
        cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

        /* set vfd for destination MAC */
        vfdArray[0].mode = TGF_VFD_MODE_STATIC_NO_MATCH_IGNORED_E;
        vfdArray[0].modeExtraInfo = 0;
        vfdArray[0].offset = 0;
        vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
        cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

        /* get trigger counters */
        rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
        PRV_UTF_LOG2_MAC("numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

        /* check pattern of TriggerCounters */
        UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfPacketsCountTriggerArr[sendIter], numTriggers,
                                     "get another trigger that expected: expected - %d, recieved - %d\n",
                                      prvTgfPacketsCountTriggerArr[sendIter], numTriggers);
    }
}

/**
* @internal prvTgfBrgStpEgressFilteringChangeStatesConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgStpEgressFilteringChangeStatesConfigRestore
(
    GT_VOID
)
{
    GT_STATUS                            rc = GT_OK;


    /* AUTODOC: RESTORE CONFIGURATION: */

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d",
                                 prvTgfDevNum, GT_TRUE);

    if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_PUMA3_E)
    {
        /* puma3 supports STG == vid */
        tempStg = PRV_TGF_VLANID_2_CNS;
    }
    else
    {
        tempStg = 0;
    }

    /* AUTODOC: bind VLAN 2 to STG Id 2 or 0 */
    rc = prvTgfBrgVlanStgIdBind(PRV_TGF_VLANID_2_CNS, tempStg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanStgIdBind: %d", prvTgfDevNum);

    if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_PUMA3_E)
    {
        /* puma3 supports STG == vid */
        tempStg = PRV_TGF_VLANID_2_CNS;
    }
    else
    {
        tempStg = PRV_TGF_STG_ID_CNS;
    }

    /* AUTODOC: restore default STP state of port 1 */
    rc = prvTgfBrgStpPortStateSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_STP_PORT_CONF_CNS],
                                  tempStg, CPSS_STP_DISABLED_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgStpPortStateSet: %d", prvTgfDevNum);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_2_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_2_CNS);

    /* AUTODOC: restore default VLAN 1 on all ports */
    rc = prvTgfBrgVlanEntryWrite(prvTgfDevNum, PRV_TGF_DEF_VLANID_CNS,
                                &prvTgfDefPortsMembers,
                                &prvTgfDefPortsTagging,
                                &prvTgfDefVlanInfo,
                                &prvTgfDefPortsTaggingCmd);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite 0");
}


