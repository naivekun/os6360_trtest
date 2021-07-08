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
* @file prvTgfBrgVlanPortAccFrameType.c
*
* @brief Acceptable frame type
*
* @version   8
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <bridge/prvTgfBrgVlanPortAccFrameType.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN0 Id */
#define PRV_TGF_VLANID_CNS            2

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS     0

/* port index to send traffic to */
#define PRV_TGF_SEND1_PORT_IDX_CNS    1

/* number of send stages */
#define PRV_TGF_STAGES_NUM_CNS        3

/* number of config stages */
#define PRV_TGF_CONFIG_STAGES_NUM_CNS 2

/* capturing port number  */
#define PRV_TGF_CAPTURE_PORT_IDX_CNS  3

/* default number of packets to send */
static GT_U32  prvTgfBurstCount = 1;

/* port indexes to send traffic */
static GT_U8 prvTgfSendPortsIdxArr[PRV_TGF_STAGES_NUM_CNS][2] =
{
    {PRV_TGF_SEND1_PORT_IDX_CNS, PRV_TGF_SEND1_PORT_IDX_CNS},
    {PRV_TGF_SEND_PORT_IDX_CNS,  PRV_TGF_SEND_PORT_IDX_CNS},
    {PRV_TGF_SEND1_PORT_IDX_CNS, PRV_TGF_SEND_PORT_IDX_CNS}
};

/* acceptable frame type on ports */
static CPSS_PORT_ACCEPT_FRAME_TYPE_ENT prvTgfAccFrameTypeArr[PRV_TGF_CONFIG_STAGES_NUM_CNS][PRV_TGF_PORTS_NUM_CNS] =
{
    {
        CPSS_PORT_ACCEPT_FRAME_TAGGED_E,
        CPSS_PORT_ACCEPT_FRAME_UNTAGGED_E,
        CPSS_PORT_ACCEPT_FRAME_ALL_E,
        CPSS_PORT_ACCEPT_FRAME_ALL_E
    },
    {
        CPSS_PORT_ACCEPT_FRAME_ALL_E,
        CPSS_PORT_ACCEPT_FRAME_ALL_E,
        CPSS_PORT_ACCEPT_FRAME_ALL_E,
        CPSS_PORT_ACCEPT_FRAME_ALL_E
    }
};

/* expected dropped packets */
static GT_U8   prvTgfExpectedDropPkts[PRV_TGF_CONFIG_STAGES_NUM_CNS][PRV_TGF_STAGES_NUM_CNS][2] =
{
   {
     {0,  1},
     {1,  0},
     {0,  0}
   },
   {
     {0,  0},
     {0,  0},
     {0,  0}
   }
};

/* expected number of Rx and Tx packets on ports*/
static GT_U8 prvTgfPacketsCountRxTxArr[PRV_TGF_CONFIG_STAGES_NUM_CNS][18][PRV_TGF_PORTS_NUM_CNS] =
{
    {
        {0, 1, 0, 1},  /* Rx count for priority tagged trafic*/
        {0, 1, 0, 1},  /* Rx count for untagged trafic*/
        {0, 1, 0, 0},  /* Rx count for tagged trafic  */
        {1, 1, 1, 1},  /* Tx count for priority tagged trafic*/
        {1, 1, 1, 1},  /* Tx count for untagged trafic*/
        {0, 1, 0, 0},  /* Tx count for tagged trafic  */
        {1, 0, 0, 0},  /* Rx count for prority tagged trafic*/
        {1, 0, 0, 0},  /* Rx count for untagged trafic*/
        {1, 0, 0, 1},  /* Rx count for tagged trafic  */
        {1, 0, 0, 0},  /* Tx count for priority tagged trafic*/
        {1, 0, 0, 0},  /* Tx count for untagged trafic*/
        {1, 1, 1, 1},  /* Tx count for tagged trafic  */
        {0, 1, 0, 1},  /* Rx count for priority tagged trafic*/
        {0, 1, 0, 1},  /* Rx count for untagged trafic*/
        {1, 0, 0, 1},  /* Rx count for tagged trafic  */
        {1, 1, 1, 1},  /* Tx count for priority tagged trafic*/
        {1, 1, 1, 1},  /* Tx count for untagged trafic*/
        {1, 1, 1, 1}   /* Tx count for tagged trafic  */
    },
    {
        {0, 1, 0, 1},  /* Rx count for priority tagged trafic*/
        {0, 1, 0, 1},  /* Rx count for untagged trafic*/
        {0, 1, 0, 1},  /* Rx count for tagged trafic  */
        {1, 1, 1, 1},  /* Tx count for priority tagged trafic*/
        {1, 1, 1, 1},  /* Tx count for untagged trafic*/
        {1, 1, 1, 1},  /* Tx count for tagged trafic  */
        {1, 0, 0, 1},  /* Rx count for priorirty tagged trafic*/
        {1, 0, 0, 1},  /* Rx count for untagged trafic*/
        {1, 0, 0, 1},  /* Rx count for tagged trafic  */
        {1, 1, 1, 1},  /* Tx count for prioriryt tagged trafic*/
        {1, 1, 1, 1},  /* Tx count for untagged trafic*/
        {1, 1, 1, 1},  /* Tx count for tagged trafic  */
        {0, 1, 0, 1},  /* Rx count for priortiy tagged trafic*/
        {0, 1, 0, 1},  /* Rx count for untagged trafic*/
        {1, 0, 0, 1},  /* Rx count for tagged trafic  */
        {1, 1, 1, 1},  /* Tx count for priorrity tagged trafic*/
        {1, 1, 1, 1},  /* Tx count for untagged trafic*/
        {1, 1, 1, 1}   /* Tx count for tagged trafic  */
    }
};

/* expected number of Triggers on portIdx 3 */
static GT_U8 prvTgfPacketsCountTriggerArr[PRV_TGF_CONFIG_STAGES_NUM_CNS][PRV_TGF_STAGES_NUM_CNS][PRV_TGF_CONFIG_STAGES_NUM_CNS] =
{
    {
        {1, 0},
        {0, 1},
        {1, 1}
    },
    {
        {1, 1},
        {1, 1},
        {1, 1}
    }
};

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

/* Priority TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketPriorityTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, 0                            /* pri, cfi, VlanId */
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

/* PARTS of untagged packet */
static TGF_PACKET_PART_STC prvTgfUntaggedPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* PARTS of tagged packet */
static TGF_PACKET_PART_STC prvTgfTaggedPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* PARTS of priority tagged packet */
static TGF_PACKET_PART_STC prvTgfPriorityTaggedPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketPriorityTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* LENGTH of UNTAGGED packet */
#define PRV_TGF_UNTAGGED_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* LENGTH of packet */
#define PRV_TGF_TAGGED_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* LENGTH of priority tagged packet */
#define PRV_TGF_PRIORITY_TAGGED_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* PACKET to send */
static TGF_PACKET_STC prvTgfUntaggedPacketInfo =
{
    PRV_TGF_UNTAGGED_PACKET_LEN_CNS,                                     /* totalLen */
    sizeof(prvTgfUntaggedPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfUntaggedPacketPartArray                                        /* partsArray */
};

/* PACKET to send */
static TGF_PACKET_STC prvTgfTaggedPacketInfo =
{
    PRV_TGF_TAGGED_PACKET_LEN_CNS,                                     /* totalLen */
    sizeof(prvTgfTaggedPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfTaggedPacketPartArray                                        /* partsArray */
};

/* PACKET to send */
static TGF_PACKET_STC prvTgfPriorityTaggedPacketInfo =
{
    PRV_TGF_PRIORITY_TAGGED_PACKET_LEN_CNS,                            /* totalLen */
    sizeof(prvTgfPriorityTaggedPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPriorityTaggedPacketPartArray                                        /* partsArray */
};
/******************************************************************************/


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfBrgVlanPortAccFrameTypeConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfBrgVlanPortAccFrameTypeConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      portIter = 0;


    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN 2 with all untagged ports */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d", prvTgfDevNum);

    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* AUTODOC: set PVID to 2 for each port */
        rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[portIter],
                                     PRV_TGF_VLANID_CNS);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter],
                                     PRV_TGF_VLANID_CNS);
    }

    /* AUTODOC: config drop counter to count only packets dropped due acceptable frame type */
    rc = prvTgfBrgCntDropCntrModeSet(PRV_TGF_BRG_DROP_CNTR_ACCEPT_FRAME_TYPE_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgCntDropCntrModeSet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_BRG_DROP_CNTR_ACCEPT_FRAME_TYPE_E);
}

/**
* @internal prvTgfBrgVlanPortAccFrameTypeTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBrgVlanPortAccFrameTypeTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                   rc          = GT_OK;
    GT_U32                      portIter    = 0;
    GT_U32                      stageIter   = 0;
    GT_U32                      configIter  = 0;
    GT_U32                      dropPkts    = 0;                  /* dropped packets */
    GT_U32                      packetLen   = 0;
    GT_U32                      numTriggers = 0;
    TGF_VFD_INFO_STC            vfdArray[1];
    CPSS_INTERFACE_INFO_STC     portInterface;
    GT_U32                      expectPacketSize;


    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    PRV_UTF_LOG0_MAC("======= Sending packets =======\n\n");

    /* AUTODOC: make 2 iterations: */
    for (configIter = 0; configIter < PRV_TGF_CONFIG_STAGES_NUM_CNS; configIter++)
    {
        PRV_UTF_LOG0_MAC("========== Sending packets with acceptable frame type config:\n");

        /* AUTODOC: set frame type to [TAGGED, UNTAGGED, ALL, ALL] for ports [0, 1, 2, 3] for 1 iter */
        /* AUTODOC: set frame type to [ALL, ALL, ALL, ALL] for ports [0, 1, 2, 3] for 2 iter */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            PRV_UTF_LOG2_MAC("port[%d] = %d\n", prvTgfPortsArray[portIter], prvTgfAccFrameTypeArr[configIter][portIter]);

            /* set acceptable frame type on each port */
            rc = prvTgfBrgVlanPortAccFrameTypeSet(prvTgfDevNum, prvTgfPortsArray[portIter],
                                                  prvTgfAccFrameTypeArr[configIter][portIter]);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortAccFrameTypeSet: %d, %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter],
                                         prvTgfAccFrameTypeArr[configIter][portIter]);
        }

        for(stageIter = 0; stageIter < PRV_TGF_STAGES_NUM_CNS; stageIter++)
        {

            /* clear capturing RxPcktTable */
            rc = tgfTrafficTableRxPcktTblClear();
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

            /* enable capture */
            portInterface.type            = CPSS_INTERFACE_PORT_E;
            portInterface.devPort.hwDevNum  = prvTgfDevNum;
            portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS];

            rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                         prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);


            /* reset counters */
            for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
            {
                rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                             prvTgfDevNum, prvTgfPortsArray[portIter]);
            }

            /* clear drop counters */
            rc = prvTgfBrgCntDropCntrSet(0);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgCntDropCntrSet: %d", prvTgfDevNum);

            /* setup priority tagged packet */
            rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPriorityTaggedPacketInfo, prvTgfBurstCount, 0, NULL);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup");

            PRV_UTF_LOG2_MAC("========== Sending priority tagged packet stageIter=%d  on port=%d==========\n\n",
                             stageIter + 1, prvTgfPortsArray[prvTgfSendPortsIdxArr[stageIter][0]]);

            /* AUTODOC: send 3 priorirty tagged packets on ports [1, 0, 1] for each iter with: */
            /* AUTODOC:   DA = 00:00:00:00:00:02, SA = 00:00:00:00:00:01 */
            rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[prvTgfSendPortsIdxArr[stageIter][0]]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);

            /* disable capture */
            rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                         prvTgfDevNum, portInterface.devPort.portNum);

            /* AUTODOC: verify traffic received according to accept frame type */
            for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
            {
                if (prvTgfPortsArray[prvTgfSendPortsIdxArr[stageIter][0]] == prvTgfPortsArray[portIter])
                {
                    /* send port */
                    expectPacketSize = PRV_TGF_PRIORITY_TAGGED_PACKET_LEN_CNS;
                }
                else
                {
                    /* recieve port */
                    expectPacketSize = PRV_TGF_UNTAGGED_PACKET_LEN_CNS;
                }

                /* ckeck ETH counters */
                rc = prvTgfEthCountersCheck(prvTgfDevNum,
                                            prvTgfPortsArray[portIter],
                                            prvTgfPacketsCountRxTxArr[configIter][6*stageIter][portIter],
                                            prvTgfPacketsCountRxTxArr[configIter][(6*stageIter)+3][portIter],
                                            expectPacketSize, prvTgfBurstCount);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
            }

            /* get drop counter */
            rc = prvTgfBrgCntDropCntrGet(&dropPkts);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgCntDropCntrGet: %d", prvTgfDevNum);

            /* AUTODOC: check that drop counters are: */
            /* AUTODOC:   for 1 iteration - [0, 1, 0] */
            /* AUTODOC:   for 2 iteration - [0, 0, 0] */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfExpectedDropPkts[configIter][stageIter][0]*(prvTgfBurstCount), dropPkts,
                                        "get another drop counter than expected");
            PRV_UTF_LOG1_MAC("The number of packets that were dropped = %d  \n\n\n", dropPkts );

            /* get Trigger Counters */
            PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

            /* clear VFD array */
            cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

            /* set vfd for destination MAC */
            vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
            vfdArray[0].modeExtraInfo = 0;
            vfdArray[0].offset = 0;
            vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
            cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

            /* get trigger counters */
            rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
            PRV_UTF_LOG2_MAC("numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

            /* check pattern of TriggerCounters */
            UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfPacketsCountTriggerArr[configIter][stageIter][0], numTriggers,
                                         "get another trigger that expected: expected - %d, recieved - %d\n",
                                         prvTgfPacketsCountTriggerArr[configIter][stageIter][0], numTriggers);


            /* clear capturing RxPcktTable */
            rc = tgfTrafficTableRxPcktTblClear();
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

            /* enable capture */
            portInterface.type            = CPSS_INTERFACE_PORT_E;
            portInterface.devPort.hwDevNum  = prvTgfDevNum;
            portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS];

            rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                         prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);


            /* reset counters */
            for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
            {
                rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                             prvTgfDevNum, prvTgfPortsArray[portIter]);
            }

            /* clear drop counters */
            rc = prvTgfBrgCntDropCntrSet(0);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgCntDropCntrSet: %d", prvTgfDevNum);

            /* setup UnTagged packet */
            rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfUntaggedPacketInfo, prvTgfBurstCount, 0, NULL);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup");

            PRV_UTF_LOG2_MAC("========== Sending UNTAGGED packet stageIter=%d  on port=%d==========\n\n",
                             stageIter + 1, prvTgfPortsArray[prvTgfSendPortsIdxArr[stageIter][0]]);

            /* AUTODOC: send 3 UnTagged packets on ports [1, 0, 1] for each iter with: */
            /* AUTODOC:   DA = 00:00:00:00:00:02, SA = 00:00:00:00:00:01 */
            rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[prvTgfSendPortsIdxArr[stageIter][0]]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);

            /* disable capture */
            rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                         prvTgfDevNum, portInterface.devPort.portNum);

            /* AUTODOC: verify traffic received according to accept frame type */
            for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
            {
                /* ckeck ETH counters */
                rc = prvTgfEthCountersCheck(prvTgfDevNum,
                                            prvTgfPortsArray[portIter],
                                            prvTgfPacketsCountRxTxArr[configIter][6*stageIter + 1][portIter],
                                            prvTgfPacketsCountRxTxArr[configIter][(6*stageIter) + 1 + 3][portIter],
                                            PRV_TGF_UNTAGGED_PACKET_LEN_CNS, prvTgfBurstCount);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
            }

            /* get drop counter */
            rc = prvTgfBrgCntDropCntrGet(&dropPkts);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgCntDropCntrGet: %d", prvTgfDevNum);

            /* AUTODOC: check that drop counters are: */
            /* AUTODOC:   for 1 iteration - [0, 1, 0] */
            /* AUTODOC:   for 2 iteration - [0, 0, 0] */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfExpectedDropPkts[configIter][stageIter][0]*(prvTgfBurstCount), dropPkts,
                                        "get another drop counter than expected");
            PRV_UTF_LOG1_MAC("The number of packets that were dropped = %d  \n\n\n", dropPkts );

            /* get Trigger Counters */
            PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

            /* clear VFD array */
            cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

            /* set vfd for destination MAC */
            vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
            vfdArray[0].modeExtraInfo = 0;
            vfdArray[0].offset = 0;
            vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
            cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

            /* get trigger counters */
            rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
            PRV_UTF_LOG2_MAC("numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

            /* check pattern of TriggerCounters */
            UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfPacketsCountTriggerArr[configIter][stageIter][0], numTriggers,
                                         "get another trigger that expected: expected - %d, recieved - %d\n",
                                         prvTgfPacketsCountTriggerArr[configIter][stageIter][0], numTriggers);

            /* clear capturing RxPcktTable */
            rc = tgfTrafficTableRxPcktTblClear();
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

            /* enable capture */
            portInterface.type            = CPSS_INTERFACE_PORT_E;
            portInterface.devPort.hwDevNum  = prvTgfDevNum;
            portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS];

            rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                         prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

            /* reset counters */
            for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
            {
                rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                             prvTgfDevNum, prvTgfPortsArray[portIter]);
            }

            /* clear drop counters */
            rc = prvTgfBrgCntDropCntrSet(0);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgCntDropCntrSet: %d", prvTgfDevNum);

            /* setup Tagged packet */
            rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfTaggedPacketInfo, prvTgfBurstCount, 0, NULL);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup");

            PRV_UTF_LOG2_MAC("========== Sending TAGGED packet stageIter=%d  on port=%d==========\n\n",
                             stageIter + 1, prvTgfPortsArray[prvTgfSendPortsIdxArr[stageIter][1]]);

            /* AUTODOC: send 3 Tagged packets on ports [1, 0, 1] for each iter with: */
            /* AUTODOC:   DA = 00:00:00:00:00:02, SA = 00:00:00:00:00:01 */
            /* AUTODOC:   VID = 2 */
            rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[prvTgfSendPortsIdxArr[stageIter][1]]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);

            /* disable capture */
            rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                         prvTgfDevNum, portInterface.devPort.portNum);

            /* AUTODOC: verify traffic received according to accept frame type */
            for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
            {
                /* calculate packet length */
                packetLen = prvTgfTaggedPacketInfo.totalLen - TGF_VLAN_TAG_SIZE_CNS * (portIter != prvTgfSendPortsIdxArr[stageIter][1]);

                /* ckeck ETH counters */
                rc = prvTgfEthCountersCheck(prvTgfDevNum,
                                            prvTgfPortsArray[portIter],
                                            prvTgfPacketsCountRxTxArr[configIter][(6*stageIter) + 2][portIter],
                                            prvTgfPacketsCountRxTxArr[configIter][((6*stageIter) + 2) + 3][portIter],
                                            packetLen, prvTgfBurstCount);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
            }

            /* get drop counter */
            rc = prvTgfBrgCntDropCntrGet(&dropPkts);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgCntDropCntrGet: %d", prvTgfDevNum);

            /* AUTODOC: verify all drop counters are 0 */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfExpectedDropPkts[configIter][stageIter][1]*(prvTgfBurstCount), dropPkts,
                                        "get another drop counter than expected");
            PRV_UTF_LOG1_MAC("The number of packets that were dropped = %d  \n\n\n", dropPkts);

            /* get Trigger Counters */
            PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

            /* clear VFD array */
            cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

            /* set vfd for destination MAC */
            vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
            vfdArray[0].modeExtraInfo = 0;
            vfdArray[0].offset = 0;
            vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
            cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

            /* get trigger counters */
            rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
            PRV_UTF_LOG2_MAC("numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

            /* check pattern of TriggerCounters */
            UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfPacketsCountTriggerArr[configIter][stageIter][1], numTriggers,
                                         "get another trigger that expected: expected - %d, recieved - %d\n",
                                         prvTgfPacketsCountTriggerArr[configIter][stageIter][1], numTriggers);
        }
    }
}

/**
* @internal prvTgfBrgVlanPortAccFrameTypeConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanPortAccFrameTypeConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      portIter = 0;


    /* AUTODOC: RESTORE CONFIGURATION: */

        /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: restore configure drop counter mode */
    rc = prvTgfBrgCntDropCntrModeSet(PRV_TGF_BRG_DROP_CNTR_COUNT_ALL_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgCntDropCntrModeSet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_BRG_DROP_CNTR_COUNT_ALL_E);

    /* clear drop counters */
    rc = prvTgfBrgCntDropCntrSet(0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgCntDropCntrSet: %d", prvTgfDevNum);

    /* AUTODOC: set default acceptable frame type on each port */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfBrgVlanPortAccFrameTypeSet(prvTgfDevNum, prvTgfPortsArray[portIter],
                                              CPSS_PORT_ACCEPT_FRAME_ALL_E);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortAccFrameTypeSet: %d, %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter],
                                     CPSS_PORT_ACCEPT_FRAME_ALL_E);

        /* AUTODOC: restore PVID for each port */
        rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[portIter],
                                     1);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter],
                                     1);

    }

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d",
                                 prvTgfDevNum, GT_TRUE);
}


