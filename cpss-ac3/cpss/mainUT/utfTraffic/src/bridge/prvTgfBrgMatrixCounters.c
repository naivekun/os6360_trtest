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
* @file prvTgfBrgMatrixCounters.c
*
* @brief The test sets a specific MAC DA and SA
* to be monitored by Matrix counter groups
*
* @version   3
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
#include <bridge/prvTgfBrgMatrixCounters.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN ID */
#define PRV_TGF_DEF_VLANID_CNS                  1

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS               0

/* capturing port number  */
#define PRV_TGF_CAPTURE_PORT_IDX_CNS            1

/* namber of packets to send */
#define PRV_TGF_SEND_PACKETS_NUM_CNS            4

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 1;

/* expected number of Rx packets on ports*/
static GT_U8 prvTgfPacketsCountRxArr[PRV_TGF_SEND_PACKETS_NUM_CNS][PRV_TGF_PORTS_NUM_CNS] =
{
    {1, 1, 0, 0},
    {1, 1, 0, 0},
    {1, 1, 0, 0},
    {1, 0, 0, 0}
};

/* expected number of Tx packets on ports*/
static GT_U8 prvTgfPacketsCountTxArr[PRV_TGF_SEND_PACKETS_NUM_CNS][PRV_TGF_PORTS_NUM_CNS] =
{
    {1, 1, 1, 1},
    {1, 1, 1, 1},
    {1, 1, 1, 1},
    {1, 0, 0, 0}
};

/* expected number of capturing packets */
static GT_U8 numTriggersArr[PRV_TGF_SEND_PACKETS_NUM_CNS] =
{
    1, 1, 1, 0
};

/* expected number  of host group counter */
static GT_U32 prvTgfMatrixCountrArr[PRV_TGF_SEND_PACKETS_NUM_CNS] =
{
    2, 0, 0, 0
};

/* set a specific MAC DA and SA */
static GT_ETHERADDR prvTgfBrgMacDaSaArr[] = 
{
    {{0x00, 0x00, 0x00, 0x00, 0x00, 0x02}},
    {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}}
};

/* array of DA macs for test */
static TGF_MAC_ADDR prvTgfBrgDaMacArr[PRV_TGF_SEND_PACKETS_NUM_CNS] = 
{
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x02},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x03},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x02},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}
};

/* array of SA macs for test */
static TGF_MAC_ADDR prvTgfBrgSaMacArr[PRV_TGF_SEND_PACKETS_NUM_CNS] = 
{
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x02}
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
    0, 0, PRV_TGF_DEF_VLANID_CNS                        /* pri, cfi, VlanId */
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
static struct 
{
    GT_ETHERADDR    prvTgfMacSa;
    GT_ETHERADDR    prvTgfMacDa;
} prvTgfRestoreCfg;

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfBrgMatrixCountersConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfBrgMatrixCountersConfigSet
(
    GT_VOID
)
{
    GT_STATUS                   rc = GT_OK;


    /* AUTODOC: SETUP CONFIGURATION: */

    /* save a default MAC DA and SA */
    rc = prvTgfBrgCntMacDaSaGet(prvTgfDevNum, &prvTgfRestoreCfg.prvTgfMacSa, 
                                &prvTgfRestoreCfg.prvTgfMacDa);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgCntMacDaSaGet: %d", prvTgfDevNum);

    /* AUTODOC: set specific MACs to be monitored by Host\Matrix counter groups: */
    /* AUTODOC:   DA=00:00:00:00:00:02, SA=00:00:00:00:00:01 */
    rc = prvTgfBrgCntMacDaSaSet(&prvTgfBrgMacDaSaArr[1], 
                                &prvTgfBrgMacDaSaArr[0]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgCntMacDaSaSet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfBrgMatrixCountersTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBrgMatrixCountersTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                       rc = GT_OK;
    GT_U8                           portIter = 0;
    GT_U8                           sendIter = 0;
    GT_U32                          packetLen = 0;
    GT_U32                          numTriggers = 0;
    TGF_VFD_INFO_STC                vfdArray[1];
    CPSS_INTERFACE_INFO_STC         portInterface;
    GT_U32                          prvTgfMatrixCntSaDaPktsPtr = 0;


    /* AUTODOC: GENERATE TRAFFIC: */
    for (sendIter = 0; sendIter < PRV_TGF_SEND_PACKETS_NUM_CNS; sendIter++)
    {
        /* clear capturing RxPcktTable */
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

        /* reset counters */
        rc = prvTgfEthCountersReset(prvTgfDevNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

        /* enable capture */
        portInterface.type            = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum  = prvTgfDevNum;
        portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS];

        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

        /* set Dst Mac */
        cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfBrgDaMacArr[sendIter], sizeof(TGF_MAC_ADDR));

        /* set Srs Mac */
        cpssOsMemCpy(prvTgfPacketL2Part.saMac, prvTgfBrgSaMacArr[sendIter], sizeof(TGF_MAC_ADDR));

        /* setup packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfTaggedPacketInfo,
                                 prvTgfBurstCount, 0, NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup");

        /* AUTODOC: send 4 packets on port 0 with: */
        /* AUTODOC:   DA=00:00:00:00:00:02, SA=00:00:00:00:00:01, VID=1 */
        /* AUTODOC:   DA=00:00:00:00:00:03, SA=00:00:00:00:00:01, VID=1 */
        /* AUTODOC:   DA=00:00:00:00:00:02, SA=00:00:00:00:00:11, VID=1 */
        /* AUTODOC:   DA=00:00:00:00:00:01, SA=00:00:00:00:00:02, VID=1 */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* disable capture */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                         prvTgfDevNum, portInterface.devPort.portNum);

        /* AUTODOC: verify traffic: */
        /* AUTODOC:   1 packet - received on ports 1,2,3 */
        /* AUTODOC:   2 packet - received on ports 1,2,3 */
        /* AUTODOC:   3 packet - received on ports 1,2,3 */
        /* AUTODOC:   4 packet - dropped */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            packetLen = (portIter == PRV_TGF_SEND_PORT_IDX_CNS) ?
                prvTgfTaggedPacketInfo.totalLen :
                prvTgfTaggedPacketInfo.totalLen - TGF_VLAN_TAG_SIZE_CNS;

            /* check ETH counters */
            rc = prvTgfEthCountersCheck(prvTgfDevNum,
                                        prvTgfPortsArray[portIter],
                                        prvTgfPacketsCountRxArr[sendIter][portIter],
                                        prvTgfPacketsCountTxArr[sendIter][portIter],
                                        packetLen, prvTgfBurstCount);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
        }

        /* AUTODOC: check  matrix counters - 2,0,0,0 */
        rc = prvTgfBrgCntMatrixGroupCntrsGet(prvTgfDevNum, &prvTgfMatrixCntSaDaPktsPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgCntMatrixGroupCntrsGet");

        UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfMatrixCountrArr[sendIter], prvTgfMatrixCntSaDaPktsPtr,
                   "get another prvTgfMatrixCntSaDaPktsPtr that expected: expected - %d, recieved - %d\n",
                   prvTgfMatrixCountrArr[sendIter], prvTgfMatrixCntSaDaPktsPtr);

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

        UTF_VERIFY_EQUAL2_STRING_MAC(numTriggersArr[sendIter], numTriggers,
                         "get another trigger that expected: expected - %d, recieved - %d\n",
                         numTriggersArr[sendIter], numTriggers);
    }
}

/**
* @internal prvTgfBrgMatrixCountersConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgMatrixCountersConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;


    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: reset hw counters */
    rc = prvTgfCommonAllBridgeCntrsReset();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCommonAllCntrsReset");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d",
                                 prvTgfDevNum, GT_TRUE);

    /* AUTODOC: restore a default MAC DA and SA */
    rc = prvTgfBrgCntMacDaSaSet(&prvTgfRestoreCfg.prvTgfMacSa, 
                                &prvTgfRestoreCfg.prvTgfMacDa);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgCntMacDaSaGet: %d", prvTgfDevNum);
}


