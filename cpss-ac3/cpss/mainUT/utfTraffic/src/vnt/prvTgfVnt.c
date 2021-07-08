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
* @file prvTgfVnt.c
*
* @brief CPSS Virtual Network Tester (VNT) Technology facility implementation.
* VNT features:
* - Operation, Administration, and Maintenance (OAM).
* - Connectivity Fault Management (CFM).
*
* @version   27
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfVntGen.h>
#include <common/tgfPortGen.h>
#include <common/tgfTunnelGen.h>
#include <vnt/prvTgfVnt.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN0 Id */
#define PRV_TGF_VLANID_CNS                  5

/* ether type used for "vlan tag" */
#define TGF_ETHERTYPE_8100_VLAN_TAG_CNS     0x8100

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS           1

/* another port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS   0

/* TTI rule index */
#define PRV_TGF_TTI_RULE_INDEX_CNS  12

/* default number of packets to send */
static GT_U32  prvTgfBurstCount = 1;


/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x02}                /* srcMac */
};

/* L2 part of CFM packet */
static TGF_PACKET_L2_STC prvTgfCfmPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x00, 0x11, 0x02},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x22, 0x02}                /* srcMac */
};

/* First VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                            /* pri, cfi, VlanId */
};

/* DATA of packet */
static GT_U8 prvTgfOamPayloadDataArr[] =
{
    0x01, 0x80, 0xC2, 0x00, 0x00, 0x02, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x88, 0x09,
    0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x02, 0x81, 0x00, 0x05, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* OAM PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketOamPayloadPart =
{
    sizeof(prvTgfOamPayloadDataArr),                       /* dataLength */
    prvTgfOamPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* PARTS of CFM packet */
static TGF_PACKET_PART_STC prvTgfCfmPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfCfmPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* PARTS of OAM packet */
static TGF_PACKET_PART_STC prvTgfOamPacketPartArray[] =
{
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketOamPayloadPart}
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

/* CFM PACKET to send */
static TGF_PACKET_STC prvTgfCfmPacketInfo =
{
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfCfmPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfCfmPacketPartArray                                        /* partsArray */
};

/* LENGTH of packet */
#define PRV_TGF_OAM_PACKET_LEN_CNS sizeof(prvTgfOamPayloadDataArr)

/* PACKET to send */
static TGF_PACKET_STC prvTgfOamPacketInfo =
{
    PRV_TGF_OAM_PACKET_LEN_CNS,                                     /* totalLen */
    sizeof(prvTgfOamPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfOamPacketPartArray                                        /* partsArray */
};


/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfVntTestPacketSend function
* @endinternal
*
* @brief   Function sends packet, performs trace and check expected results.
*
* @param[in] portNum                  - port number to send traffic from
* @param[in] packetInfoPtr            - PACKET to send
*                                       None
*/
static GT_VOID prvTgfVntTestPacketSend
(
    IN GT_U32           portNum,
    IN TGF_PACKET_STC *packetInfoPtr
)
{
    GT_STATUS       rc = GT_OK;

    /* use different 'etherType for vlan tagging' instead of 0x8100 */
    /* this will make our packet that has '0x8100' to be recognized as 'untagged'
      by the 'traffic generator' and also the CPSS will not override the ethertype
      of tag0 with the DSA tag */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_NON_VALID_TAG_CNS);

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, prvTgfBurstCount, 0, NULL);

    /* send Packet from port portNum */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, portNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, portNum);

    /* restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);
}


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfVntOamPortPduTrapSet function
* @endinternal
*
* @brief   Set test configuration:
*         Enable OAM Port PPU Trap feature.
*/
GT_VOID prvTgfVntOamPortPduTrapSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;


    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN 5 with untagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d",
                                 PRV_TGF_VLANID_CNS);

    /* AUTODOC: enable TRAP_TO_CPU of 802.3ah LLC protocol on port 1 */
    rc = prvTgfVntOamPortPduTrapEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfVntOamPortPduTrapEnableSet: %d %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE);
}

/**
* @internal prvTgfVntLoopbackModeConfSet function
* @endinternal
*
* @brief   Set test configuration:
*         Configure Loopback Mode om port 8.
*/
GT_VOID prvTgfVntLoopbackModeConfSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;


    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN 5 with untagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d",
                                 PRV_TGF_VLANID_CNS);

    /* AUTODOC: enable 802.3ah Loopback mode on  port 1 */
    rc = prvTgfVntOamPortLoopBackModeEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfVntOamPortLoopBackModeEnableSet: %d %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE);

    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum,  (UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_CPSS_PP_E_ARCH_CNS)))
    {
        /* AUTODOC: enable TRAP_TO_CPU of 802.3ah LLC protocol on port 1 */
        rc = prvTgfVntOamPortPduTrapEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfVntOamPortPduTrapEnableSet: %d %d",
                                     prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE);
    }
}

/**
* @internal prvTgfVntOamPortPduTrapTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 8 packet:
*         macDa = 01:80:C2:00:00:02,
*         macSa = 00:00:00:00:00:02
*         Ether Type 8809
*         Subtype 0x03
*         Success Criteria:
*         Packet is captured on CPU.
*/
GT_VOID prvTgfVntOamPortPduTrapTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS    rc = GT_OK;
    GT_U32       portIter = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    CPSS_INTERFACE_INFO_STC       portInterface;
    GT_U8                packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS];
    GT_U32               buffLen = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32               packetActualLength;
    GT_U8                devNum;
    GT_U8                queue;
    TGF_NET_DSA_STC      rxParam;


    /* AUTODOC: GENERATE TRAFFIC: */

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* enable capture */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];

    /* Start capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum, GT_TRUE);

    /* AUTODOC: send OAM control packet from port 1 with: */
    /* AUTODOC:   DA=01:80:C2:00:00:02, SA=00:00:00:00:00:00 */
    /* AUTODOC:   EtherType=0x8809, SubType=0x03 */
    prvTgfVntTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfOamPacketInfo);

    /* Stop capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum, GT_FALSE);

    /* AUTODOC: verify traffic is captured on CPU */
    /* Get entry from captured packet's table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_CAPTURE_E,
                                       GT_TRUE, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue,
                                       &rxParam);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Error in tgfTrafficGeneratorRxInCpuGet %d",
                                 TGF_PACKET_TYPE_CAPTURE_E);

    /* check captured packet size */
    UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(0, packetActualLength);

    /* AUTODOC: verify no Tx traffic on ports [0,2,3] */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }

        /* check Tx counters */
        UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
    }

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
}

/**
* @internal prvTgfVntLoopbackTrafficConfGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 8 OAM control packet:
*         macDa = 01:80:C2:00:00:02,
*         macSa = 00:00:00:00:00:02,
*         Ether Type 8809.
*         Subtype 0x03.
*         Success Criteria:
*         Packet is captured on CPU.
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         Packet is captured on port 8.
*/
GT_VOID prvTgfVntLoopbackTrafficConfGenerate
(
    GT_VOID
)
{
    GT_STATUS    rc = GT_OK;
    GT_U32       portIter = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    GT_BOOL      isPuma = GT_FALSE;


    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_PUMA2_E | UTF_PUMA3_E))
    {
        /* Puma2, Puma3 devices  */
        isPuma = GT_TRUE;
    }

    /* AUTODOC: GENERATE TRAFFIC: */

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: send OAM control packet from port 0 with: */
    /* AUTODOC:   DA=01:80:C2:00:00:02, SA=00:00:00:00:00:00 */
    /* AUTODOC:   EtherType=0x8809, SubType=0x03 */
    prvTgfVntTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS], &prvTgfOamPacketInfo);
    cpssOsTimerWkAfter(1);

    /* AUTODOC: verify Tx traffic on ports [2,3], no Tx traffic on port 1 */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* The loopback port feature works in TTI unit (ingress processing)
           in Puma devices. The egress of loopback port is working as usual
           one. The usual flooding will be in Puma devices. */
        if (isPuma == GT_FALSE)
        {
            /* traffic is discarded on port 8 in which loopback mode is enabled */
            if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
            {
                /* check Rx\Tx counters */
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsRcv.l[0],
                                             "get another goodPktsRcv counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                             "get another goodPktsRcv counter than expected");
                continue;
            }
        }
        /* check Rx counters on send port */
        if (PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
        }

        /* check Tx counters */
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
    }

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: send BC packet from port 1 with: */
    /* AUTODOC:   DA=FF:FF:FF:FF:FF:FF, SA=00:00:00:00:00:02, VID=5 */
    prvTgfVntTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo);
    cpssOsTimerWkAfter(10);

    /* AUTODOC: verify no Tx traffic on ports [0,2,3] */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* the test will result in infinite traffic within
               PRV_TGF_SEND_PORT_NUM_CNS port because:
                - OAM loopback feature send packet back to ingress port
                - ingress port has MAC loopback so packet looped back by
                  OAM feature will be injected back by MAC loopback.
               The OK criteria is number of Rx/Tx packets is NOT 0 */

            /* check Rx counters */
            UTF_VERIFY_NOT_EQUAL0_STRING_MAC(0, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            /* check Tx counters */
            UTF_VERIFY_NOT_EQUAL0_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");

            continue;
        }

        /* check Tx counters */
        UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
    }

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
}

/**
* @internal prvTgfVntLoopbackConfRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfVntLoopbackConfRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */
    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum,  (UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_CPSS_PP_E_ARCH_CNS)))
    {
        /* AUTODOC: disable TRAP_TO_CPU of 802.3ah LLC protocol on port 1 */
        rc = prvTgfVntOamPortPduTrapEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfVntOamPortPduTrapEnableRestore: %d %d",
                                     prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    }

    /* AUTODOC: disable 802.3ah Loopback mode on  port 1 */
    rc = prvTgfVntOamPortLoopBackModeEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfVntOamPortLoopBackModeEnableSet: %d %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);

    /* invalidate VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d",
                                 PRV_TGF_VLANID_CNS);

    /* let to simulation handle packets ran in the loop */
    cpssOsTimerWkAfter(300);
}

/**
* @internal prvTgfVntOamPortPduTrapRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfVntOamPortPduTrapRestore
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: disable TRAP_TO_CPU of 802.3ah LLC protocol on port 1 */
    rc = prvTgfVntOamPortPduTrapEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfVntOamPortPduTrapEnableRestore: %d %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* invalidate VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d",
                                 PRV_TGF_VLANID_CNS);
}

/**
* @internal prvTgfVntCfmEtherTypeIdentificationSet function
* @endinternal
*
* @brief   Set test configuration:
*         Enable the Ingress Policy Engine.
*         Set Pcl rule to trap packets with ethertype 3434 and UDB 0xFF.
*         Set CFM ethertype to 3434.
*/
GT_VOID prvTgfVntCfmEtherTypeIdentificationSet
(
    GT_VOID
)
{
    PRV_TGF_PCL_LOOKUP_CFG_STC lookupCfg;
    GT_STATUS               rc = GT_OK;
    GT_U32                           ruleIndex = 0;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    PRV_TGF_PCL_ACTION_STC           action;
    CPSS_INTERFACE_INFO_STC          interfaceInfo;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;


    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN 5 with untagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d",
                                 PRV_TGF_VLANID_CNS);

     /* Init PCL. */
    rc = prvTgfPclInit();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"prvTgfPclInit");

    /* AUTODOC: enable PCL ingress policy */
    rc = prvTgfPclIngressPolicyEnable(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", GT_TRUE);

    /* AUTODOC: enables PCL ingress policy for port 1 */
    rc = prvTgfPclPortIngressPolicyEnable(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d, %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE);

    /* AUTODOC: configure lookup0 ingress entry by Port area 0 */
    rc = prvTgfPclPortLookupCfgTabAccessModeSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                           CPSS_PCL_DIRECTION_INGRESS_E,
                                           CPSS_PCL_LOOKUP_0_E,
                                           0,
                                  PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet: %d, %d, %d, %d, %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],CPSS_PCL_DIRECTION_INGRESS_E,
                                 CPSS_PCL_LOOKUP_0_E,0,PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);

    /* AUTODOC: configure User Defined Byte (UDB) idx 15: */
    /* AUTODOC:   format INGRESS_STD_NOT_IP */
    /* AUTODOC:   packet type UDE */
    /* AUTODOC:   offsetType OFFSET_L2, offset 18 */
    rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                                     PRV_TGF_PCL_PACKET_TYPE_UDE_E,
                                     CPSS_PCL_DIRECTION_INGRESS_E,
                                     15,
                                     PRV_TGF_PCL_OFFSET_L2_E,
                                     18);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d, %d, %d, %d, %d",
                                 PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,PRV_TGF_PCL_PACKET_TYPE_IPV4_TCP_E,0,PRV_TGF_PCL_OFFSET_L2_E,14);

    /* Set PCL configuration table */
    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));

    interfaceInfo.type            = CPSS_INTERFACE_PORT_E;
    interfaceInfo.devPort.hwDevNum  = prvTgfDevNum;
    interfaceInfo.devPort.portNum = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    lookupCfg.enableLookup        = GT_TRUE;

    lookupCfg.enableLookup = GT_TRUE;
    lookupCfg.pclId = 0;

    lookupCfg.groupKeyTypes.nonIpKey = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key  = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    lookupCfg.groupKeyTypes.ipv6Key  = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;

    /* AUTODOC: set PCL config table for port 1 lookup0 with: */
    /* AUTODOC:   nonIpKey=INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key=INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key=INGRESS_STD_IP_L2_QOS */
    rc = prvTgfPclCfgTblSet(&interfaceInfo, CPSS_PCL_DIRECTION_INGRESS_E,
                            CPSS_PCL_LOOKUP_0_E, &lookupCfg);

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclCfgTblSet");

    /* set PCL rule */
    ruleFormat = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    ruleIndex  = 0;

    cpssOsMemSet(&mask, 0, sizeof(mask));
    mask.ruleStdNotIp.etherType = 0xFFFF;
    mask.ruleStdNotIp.udb[0] = 0xFF;

    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    pattern.ruleStdNotIp.etherType = 3434;
    pattern.ruleStdNotIp.udb[0] = 0xFF;

    cpssOsMemSet(&action, 0, sizeof(action));

    action.pktCmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    action.mirror.cpuCode = CPSS_NET_UDP_BC_MIRROR_TRAP3_E;

    /* AUTODOC: add PCL rule 0 with: */
    /* AUTODOC:   format INGRESS_STD_NOT_IP */
    /* AUTODOC:   cmd TRAP_TO_CPU */
    /* AUTODOC:   pattern etherType=3434, UDB=0xFF */
    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
                                  ruleFormat, ruleIndex);

    /* AUTODOC: set EtherType 3434 to identify CFM PDUs */
    rc = prvTgfVntCfmEtherTypeSet(pattern.ruleStdNotIp.etherType);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfVntCfmEtherTypeSet: %d",
                                 pattern.ruleStdNotIp.etherType);
}

/**
* @internal prvTgfVntCfmOpcodeIdentificationSet function
* @endinternal
*
* @brief   Set test configuration:
*         Enable the Ingress Policy Engine.
*         Set Pcl rule to trap packets with ethertype 3434 and UDB 0xFF and
*         to redirect them to the send port.
*         Set CFM ethertype to 3434.
*         Set opcode to 0x7F.
*/
GT_VOID prvTgfVntCfmOpcodeIdentificationSet
(
    GT_VOID
)
{
    PRV_TGF_PCL_LOOKUP_CFG_STC lookupCfg;
    GT_STATUS               rc = GT_OK;
    GT_U32                           ruleIndex = 0;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    PRV_TGF_PCL_ACTION_STC           action;
    CPSS_INTERFACE_INFO_STC          interfaceInfo;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* The Egress Port should be tagged for Puma2, Lion2 and above devices. Because
       the Puma, Lion2 and above devices egress tagging depends only from VLAN table.
       The ingress tagging in previous devices preserved by vntL2Echo feature.*/
    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_LION2_E | UTF_CPSS_PP_E_ARCH_CNS | UTF_PUMA2_E | UTF_PUMA3_E))
    {
        /* Lion2 and Bobcat2; Caelum; Bobcat3 devices  */
        rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(PRV_TGF_VLANID_CNS, PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWriteWithTaggingCmd: %d",
                                     PRV_TGF_VLANID_CNS);
    }
    else
    {
        /* AUTODOC: create VLAN 5 with untagged ports [0,1,2,3] */
        rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d",
                                     PRV_TGF_VLANID_CNS);
    }

     /* Init PCL. */
    rc = prvTgfPclInit();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"prvTgfPclInit");

    /* AUTODOC: enable PCL ingress policy */
    rc = prvTgfPclIngressPolicyEnable(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", GT_TRUE);

    /* AUTODOC: enable PCL ingress policy for port 1 */
    rc = prvTgfPclPortIngressPolicyEnable(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d, %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE);

    /* AUTODOC: configure lookup0 ingress entry by Port area 0 */
    rc = prvTgfPclPortLookupCfgTabAccessModeSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                           CPSS_PCL_DIRECTION_INGRESS_E,
                                           CPSS_PCL_LOOKUP_0_E,
                                           0,
                                  PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet: %d, %d, %d, %d, %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],CPSS_PCL_DIRECTION_INGRESS_E,
                                 CPSS_PCL_LOOKUP_0_E,0,PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);

    /* AUTODOC: configure User Defined Byte (UDB) idx 15: */
    /* AUTODOC:   format INGRESS_STD_NOT_IP */
    /* AUTODOC:   packet type UDE */
    /* AUTODOC:   offsetType OFFSET_L2, offset 18 */
    rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                                     PRV_TGF_PCL_PACKET_TYPE_UDE_E,
                                     CPSS_PCL_DIRECTION_INGRESS_E,
                                     15,
                                     PRV_TGF_PCL_OFFSET_L2_E,
                                     18);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d, %d, %d, %d, %d",
                                 PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,PRV_TGF_PCL_PACKET_TYPE_IPV4_TCP_E,0,PRV_TGF_PCL_OFFSET_L2_E,14);

    /* Set PCL configuration table */
    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));

    interfaceInfo.type            = CPSS_INTERFACE_PORT_E;
    interfaceInfo.devPort.hwDevNum  = prvTgfDevNum;
    interfaceInfo.devPort.portNum = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    lookupCfg.enableLookup        = GT_TRUE;

    lookupCfg.enableLookup = GT_TRUE;
    lookupCfg.pclId = 0;

    lookupCfg.groupKeyTypes.nonIpKey = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key  = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    lookupCfg.groupKeyTypes.ipv6Key  = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;

    /* AUTODOC: set PCL config table for port 1 lookup0 with: */
    /* AUTODOC:   nonIpKey=INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key=INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key=INGRESS_STD_IP_L2_QOS */
    rc = prvTgfPclCfgTblSet(&interfaceInfo, CPSS_PCL_DIRECTION_INGRESS_E,
                             CPSS_PCL_LOOKUP_0_E, &lookupCfg);

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclCfgTblSet");

    /* set PCL rule */
    ruleFormat = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    ruleIndex  = 0;

    cpssOsMemSet(&mask, 0, sizeof(mask));
    mask.ruleStdNotIp.etherType = 0xFFFF;
    mask.ruleStdNotIp.udb[0] = 0xFF;

    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    pattern.ruleStdNotIp.etherType = 3434;
    pattern.ruleStdNotIp.udb[0] = 0xFF;

    cpssOsMemSet(&action, 0, sizeof(action));

    action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
    action.redirect.redirectCmd = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
    action.bypassBridge = GT_TRUE;/* (sip5) must be set explicitly */
    action.redirect.data.outIf.outInterface.type = CPSS_INTERFACE_PORT_E;
    action.redirect.data.outIf.outInterface.devPort.hwDevNum = prvTgfDevNum;
    action.redirect.data.outIf.outInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS];
    action.redirect.data.outIf.vntL2Echo = GT_TRUE;

    /* AUTODOC: add PCL rule 0 with: */
    /* AUTODOC:   format INGRESS_STD_NOT_IP */
    /* AUTODOC:   cmd FORWARD on port 0 */
    /* AUTODOC:   pattern etherType=3434, UDB=0xFF */
    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
                                  ruleFormat, ruleIndex);

    /* AUTODOC: set EtherType 3434 to identify CFM PDUs */
    rc = prvTgfVntCfmEtherTypeSet(pattern.ruleStdNotIp.etherType);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfVntCfmEtherTypeSet: %d",
                                  pattern.ruleStdNotIp.etherType);

    /* AUTODOC: set CFM LBR opcode 0x7F */
    rc = prvTgfVntCfmLbrOpcodeSet(0x7F);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfVntCfmLbrOpcodeSet: %d", 0x7F);
}

/**
* @internal prvTgfVntCfmOpcodeChangeSet function
* @endinternal
*
* @brief   Set test configuration:
*         Enable the TTI.
*         Set TTI rule to trap packets with ethertype 3434 and
*         to redirect them to the send port.
*         Set CFM ethertype to 3434.
*         Set opcode to 0x7F.
*/
GT_VOID prvTgfVntCfmOpcodeChangeSet
(
    GT_VOID
)
{
    GT_STATUS               rc = GT_OK;

    PRV_TGF_TTI_RULE_UNT        ttiMask;
    PRV_TGF_TTI_RULE_UNT        ttiPattern;
    PRV_TGF_TTI_ACTION_STC      ttiAction;

    /* AUTODOC: SETUP CONFIGURATION: */

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    /* The Egress Port should be tagged for Puma2, Lion2 and above devices. Because
       the Puma, Lion2 and above devices egress tagging depends only from VLAN table.
       The ingress tagging in previous devices preserved by vntL2Echo feature.*/
    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_LION2_E  | UTF_CPSS_PP_E_ARCH_CNS | UTF_PUMA2_E | UTF_PUMA3_E))
    {
        /* Lion2 and Bobcat2; Caelum; Bobcat3 devices  */
        rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(PRV_TGF_VLANID_CNS, PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWriteWithTaggingCmd: %d",
                                     PRV_TGF_VLANID_CNS);
    }
    else
    {
        /* AUTODOC: create VLAN 5 with untagged ports [0,1,2,3] */
        rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d",
                                     PRV_TGF_VLANID_CNS);
    }

    /* AUTODOC: enable TTI lookup for Ethernet key on port 1 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_ETH_E,
                                      GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));
    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));

    ttiMask.eth.common.srcIsTrunk      = GT_TRUE;
    ttiMask.eth.common.srcPortTrunk    = 0xFF;
    ttiMask.eth.etherType              = 0xFFFFFFFF;

    ttiPattern.eth.common.srcIsTrunk   = GT_FALSE;
    ttiPattern.eth.common.srcPortTrunk = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    ttiPattern.eth.etherType           = 3434;

    ttiAction.redirectCommand          = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;
    ttiAction.interfaceInfo.type            = CPSS_INTERFACE_PORT_E;
    ttiAction.interfaceInfo.devPort.hwDevNum  = prvTgfDevNum;
    ttiAction.interfaceInfo.devPort.portNum =
                        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS];

    ttiAction.bridgeBypass = GT_TRUE;

    ttiAction.vntl2Echo                = GT_TRUE;

    /* AUTODOC: add TTI Rule with: */
    /* AUTODOC: key=TTI_KEY_ETH, srcPortTrunk=1, ethertype=CFM type */
    /* AUTODOC: vntl2Echo set in action  */
    rc = prvTgfTtiRuleSet(PRV_TGF_TTI_RULE_INDEX_CNS, PRV_TGF_TTI_KEY_ETH_E,
                          &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet");

    /* AUTODOC: set EtherType 3434 to identify CFM PDUs */
    rc = prvTgfVntCfmEtherTypeSet(ttiPattern.eth.etherType);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfVntCfmEtherTypeSet: %d",
                                 ttiPattern.eth.etherType);

    /* AUTODOC: set CFM LBR opcode 0x7F */
    rc = prvTgfVntCfmLbrOpcodeSet(0x7F);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfVntCfmLbrOpcodeSet: %d", 0x7F);
}


/**
* @internal prvTgfVntCfmEtherTypeIdentificationTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         ethertype = 2222
*         Success Criteria:
*         Packet is captured on ports 0,18,23.
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         ethertype = 3434
*         Success Criteria:
*         Packet is captured on CPU only.
*/
GT_VOID prvTgfVntCfmEtherTypeIdentificationTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS    rc = GT_OK;
    GT_U32       portIter = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;


    /* AUTODOC: GENERATE TRAFFIC: */

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: send BC packet from port 1 with: */
    /* AUTODOC:   DA=FF:FF:FF:FF:FF:FF, SA=00:00:00:00:00:02, VID=5 */
    prvTgfVntTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo);

    cpssOsTimerWkAfter(1);

    /* AUTODOC: verify Tx traffic on ports [0,2,3] */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }

        /* check Tx counters */
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
    }

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* Set ethertype to value that was set in pcl rule (CFM ethertype) */
    prvTgfPayloadDataArr[0] = 0x0D;
    prvTgfPayloadDataArr[1] = 0x6A;

    /* User Defined Fields in TCAM is 0xFF */
    prvTgfPayloadDataArr[2] = 0xFF;

    /* AUTODOC: send BC packet from port 1 with: */
    /* AUTODOC:   DA=FF:FF:FF:FF:FF:FF, SA=00:00:00:00:00:02, VID=5 */
    /* AUTODOC:   EtherType=0x0D6A, Byte[18]=0xFF */
    prvTgfVntTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo);
    cpssOsTimerWkAfter(1);

    /* AUTODOC: verify no Tx traffic on ports [0,2,3] */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");

            continue;
        }

        /* check Tx counters */
        UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
    }

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
}

/**
* @internal prvTgfVntCfmEtherTypeIdentificationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfVntCfmEtherTypeIdentificationRestore
(
    GT_VOID
)
{
    GT_STATUS    rc = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */

    prvTgfPayloadDataArr[0] = 0;
    prvTgfPayloadDataArr[1] = 0;
    prvTgfPayloadDataArr[2] = 0;
    prvTgfPayloadDataArr[3] = 0;

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: disable PCL ingress policy */
    rc = prvTgfPclIngressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", GT_FALSE);

    /* AUTODOC: disable PCL ingress policy for port 1 */
    rc = prvTgfPclPortIngressPolicyEnable(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d, %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);

    /* AUTODOC: invalidate PCL rule 0 */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E, 0, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d",
                                 prvTgfDevNum, GT_TRUE);

    /* restore PCL configuration table */
    prvTgfPclRestore();

    /* invalidate VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d",
                                 PRV_TGF_VLANID_CNS);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

}

/**
* @internal prvTgfVntCfmOpcodeIdentificationTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         ethertype = 3434
*         Success Criteria:
*         Packet is captured on CPU.
*         Packet is captured on port 8 with opcode 0x7F
*/
GT_VOID prvTgfVntCfmOpcodeIdentificationTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS    rc = GT_OK;
    GT_U32       portIter = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    CPSS_INTERFACE_INFO_STC       portInterface;
    GT_U8                trigPacketBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32               buffLen = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32               packetActualLength = 0;
    GT_U8                devNum;
    GT_U8                queue;
    TGF_NET_DSA_STC      rxParam;
    TGF_VFD_INFO_STC     vfd;
    GT_BOOL              triggered;
    GT_U32               vfdNum;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) &vfd, 0, sizeof(vfd));

    /* set VFD array of expected fields value in recieved packet*/
    vfd.cycleCount = 2;
    vfd.mode = TGF_VFD_MODE_STATIC_E;
    vfd.modeExtraInfo = 0;
    vfd.offset = 19;
    if ( UTF_CPSS_SIP5_0_ONLY_CHECK_MAC(prvTgfDevNum) )
    {
        vfd.patternPtr[0] = 0x03;
    }
    else
    {
        vfd.patternPtr[0] = 0x7F;
    }
    vfd.patternPtr[1] = 0x00;
    vfdNum = 1;

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* Set ethertype to value that was set in pcl rule (CFM ethertype) */
    prvTgfPayloadDataArr[0] = 0x0D;
    prvTgfPayloadDataArr[1] = 0x6A;

    /* User Defined Fields in TCAM is 0xFF */
    prvTgfPayloadDataArr[2] = 0xFF;

    /* The fourth byte is Opcode one. The test for Puma devices should
       use LBM Opcode - 3 */
    prvTgfPayloadDataArr[3] = 0x03;

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* enable capture */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS];

    /* Start capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, portInterface.devPort.portNum);

    /* AUTODOC: send CFM packet from port 1 with: */
    /* AUTODOC:   DA=00:00:00:00:11:02, SA=00:00:00:00:22:02, VID=5 */
    /* AUTODOC:   EtherType=0x0D6A, Byte[18]=0xFF */
    prvTgfVntTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfCfmPacketInfo);
    cpssOsTimerWkAfter(1);

    /* Stop capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, portInterface.devPort.portNum);

    /* Get entry from captured packet's table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_CAPTURE_E,
                                       GT_TRUE, GT_TRUE, trigPacketBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue,
                                       &rxParam);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Error in tgfTrafficGeneratorRxInCpuGet %d", TGF_PACKET_TYPE_CAPTURE_E);

    /* check captured packet size */
    UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(0, packetActualLength);

    /* AUTODOC: verify forwarded Tx traffic on port 0: */
    /* AUTODOC:   DA=00:00:00:00:22:02, SA=00:00:00:00:11:02, VID=5 */
    /* AUTODOC:   EtherType=0x0D6A, Byte[18]=0xFF, Opcode=0x7F */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            /* check Tx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsRcv counter than expected");

            /* set max allowed buff len */
            buffLen = TGF_RX_BUFFER_MAX_SIZE_CNS;

            /* check triggers */
            rc = tgfTrafficGeneratorTxEthTriggerCheck(trigPacketBuff,
                                                      buffLen,
                                                      1,
                                                      &vfd,
                                                      &triggered);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "Error in tgfTrafficGeneratorTxEthTriggerCheck %d, %d",
                                         buffLen, vfdNum);

            UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, triggered,
                                         "Packet Opcode is wrong");

            continue;
        }

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsRcv counter than expected");

            continue;
        }

        /* check Tx counters */
        UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
    }

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
}

/**
* @internal prvTgfVntCfmOpcodeIdentificationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfVntCfmOpcodeIdentificationRestore
(
    GT_VOID
)
{
    GT_STATUS    rc = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */

    prvTgfPayloadDataArr[0] = 0;
    prvTgfPayloadDataArr[1] = 0;
    prvTgfPayloadDataArr[2] = 0;
    prvTgfPayloadDataArr[3] = 0;

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: disable PCL ingress policy */
    rc = prvTgfPclIngressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", GT_FALSE);

    /* AUTODOC: disable PCL ingress policy for port 1 */
    rc = prvTgfPclPortIngressPolicyEnable(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d, %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);

    /* AUTODOC: invalidate PCL rule 0 */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E, 0, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d", prvTgfDevNum, GT_TRUE);

    /* restore PCL configuration table */
    prvTgfPclRestore();

    /* invalidate VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d",
                                 PRV_TGF_VLANID_CNS);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

}

/**
* @internal prvTgfVntCfmOpcodeChangeTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         ethertype = 3434
*         Success Criteria:
*         Packet is captured on CPU.
*         Packet is captured on port 8 with opcode 0x7F
*/
GT_VOID prvTgfVntCfmOpcodeChangeTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS    rc = GT_OK;
    GT_U32       portIter = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    CPSS_INTERFACE_INFO_STC       portInterface;
    GT_U8                trigPacketBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32               buffLen = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32               packetActualLength = 0;
    GT_U8                devNum;
    GT_U8                queue;
    TGF_NET_DSA_STC      rxParam;
    TGF_VFD_INFO_STC     vfd;
    GT_BOOL              triggered;
    GT_U32               vfdNum;

    PRV_TGF_VNT_CFM_REPLY_MODE_ENT   cfmReplyMode;


    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) &vfd, 0, sizeof(vfd));

    /* set VFD array of expected fields value in recieved packet*/
    vfd.cycleCount = 2;
    vfd.mode = TGF_VFD_MODE_STATIC_E;
    vfd.modeExtraInfo = 0;
    vfd.offset = 19;
    vfd.patternPtr[0] = 0x7F;
    vfd.patternPtr[1] = 0x00;
    vfdNum = 1;

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* Set ethertype to value that was set in pcl rule (CFM ethertype) */
    prvTgfPayloadDataArr[0] = 0x0D;
    prvTgfPayloadDataArr[1] = 0x6A;

    /* User Defined Fields in TCAM is 0xFF */
    prvTgfPayloadDataArr[2] = 0xFF;

    /* The fourth byte is Opcode one. The test for Puma devices should
       use LBM Opcode - 3 */
    prvTgfPayloadDataArr[3] = 0x03;

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* enable capture */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS];

    /* Start capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, portInterface.devPort.portNum);

    /* AUTODOC: send CFM packet from port 1 with: */
    /* AUTODOC:   DA=00:00:00:00:11:02, SA=00:00:00:00:22:02, VID=5 */
    /* AUTODOC:   EtherType=0x0D6A, Byte[18]=0xFF */
    prvTgfVntTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfCfmPacketInfo);
    cpssOsTimerWkAfter(1);

    /* Stop capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, portInterface.devPort.portNum);

    /* Get entry from captured packet's table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_CAPTURE_E,
                                       GT_TRUE, GT_TRUE, trigPacketBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue,
                                       &rxParam);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Error in tgfTrafficGeneratorRxInCpuGet %d", TGF_PACKET_TYPE_CAPTURE_E);

    /* check captured packet size */
    UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(0, packetActualLength);

    /* AUTODOC: verify forwarded Tx traffic on port 0: */
    /* AUTODOC:   DA=00:00:00:00:22:02, SA=00:00:00:00:11:02, VID=5 */
    /* AUTODOC:   EtherType=0x0D6A, Byte[18]=0xFF, Opcode=0x7F */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            /* check Tx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsRcv counter than expected");

            /* set max allowed buff len */
            buffLen = TGF_RX_BUFFER_MAX_SIZE_CNS;

            /* check triggers */
            rc = tgfTrafficGeneratorTxEthTriggerCheck(trigPacketBuff,
                                                      buffLen,
                                                      1,
                                                      &vfd,
                                                      &triggered);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "Error in tgfTrafficGeneratorTxEthTriggerCheck %d, %d",
                                         buffLen, vfdNum);

            UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, triggered,
                                         "Packet Opcode is wrong");

            continue;
        }

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsRcv counter than expected");

            continue;
        }

        /* check Tx counters */
        UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
    }

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    if(!UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
    {
        return;
    }

    rc = prvTgfVntCfmReplyModeGet(prvTgfDevNum, &cfmReplyMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error in prvTgfVntCfmReplyModeGet");

    /* AUTODOC: Change CFM relpy mode to clear lsb */
    rc = prvTgfVntCfmReplyModeSet(PRV_TGF_VNT_CFM_REPLY_MODE_CHANGE_LSBIT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error in prvTgfVntCfmReplyModeSet");

    vfd.patternPtr[0] = 0x02;

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* Start capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, portInterface.devPort.portNum);

    /* AUTODOC: send CFM packet from port 1 with: */
    /* AUTODOC:   DA=00:00:00:00:11:02, SA=00:00:00:00:22:02, VID=5 */
    /* AUTODOC:   EtherType=0x0D6A, Byte[18]=0xFF */
    prvTgfVntTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfCfmPacketInfo);
    cpssOsTimerWkAfter(1);

    /* Stop capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, portInterface.devPort.portNum);

    /* Get entry from captured packet's table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_CAPTURE_E,
                                       GT_TRUE, GT_TRUE, trigPacketBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue,
                                       &rxParam);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Error in tgfTrafficGeneratorRxInCpuGet %d", TGF_PACKET_TYPE_CAPTURE_E);

    /* check captured packet size */
    UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(0, packetActualLength);

    /* AUTODOC: verify forwarded Tx traffic on port 0: */
    /* AUTODOC:   DA=00:00:00:00:22:02, SA=00:00:00:00:11:02, VID=5 */
    /* AUTODOC:   EtherType=0x0D6A, Byte[18]=0xFF, Opcode=0x02 */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            /* check Tx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsRcv counter than expected");

            /* set max allowed buff len */
            buffLen = TGF_RX_BUFFER_MAX_SIZE_CNS;

            /* check triggers */
            rc = tgfTrafficGeneratorTxEthTriggerCheck(trigPacketBuff,
                                                      buffLen,
                                                      1,
                                                      &vfd,
                                                      &triggered);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "Error in tgfTrafficGeneratorTxEthTriggerCheck %d, %d",
                                         buffLen, vfdNum);

            UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, triggered,
                                         "Packet Opcode is wrong");

            continue;
        }

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsRcv counter than expected");

            continue;
        }

        /* check Tx counters */
        UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
    }

    rc = prvTgfVntCfmReplyModeSet(cfmReplyMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error in prvTgfVntCfmReplyModeSet");

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
}

/**
* @internal prvTgfVntCfmOpcodeChangeRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfVntCfmOpcodeChangeRestore
(
    GT_VOID
)
{
    GT_STATUS    rc = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */

    prvTgfPayloadDataArr[0] = 0;
    prvTgfPayloadDataArr[1] = 0;
    prvTgfPayloadDataArr[2] = 0;
    prvTgfPayloadDataArr[3] = 0;

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");



    /* AUTODOC: invalidate TTI Rule */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_RULE_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet");

    /* AUTODOC: disable TTI lookup on port 1 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_ETH_E,
                                      GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* invalidate VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d",
                                 PRV_TGF_VLANID_CNS);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);
}

/**
* @internal prvTgfVntOamPortUnidirectionalTransmitModeSet function
* @endinternal
*
* @brief   Set test configuration:
*         Enable/Disable the port for unidirectional transmit.
*/
GT_VOID prvTgfVntOamPortUnidirectionalTransmitModeSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;


    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN 5 with untagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d",
                                 PRV_TGF_VLANID_CNS);

    /* AUTODOC: enable unidirectional transmit on port 0 */
    rc = prvTgfVntOamPortUnidirectionalEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfVntOamPortUnidirectionalEnableSet: %d %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS], GT_TRUE);



    /* AUTODOC: enable Force Link Down on port 0 */
    rc = prvTgfPortForceLinkDownEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChPortForceLinkDownEnableSet: %d %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS], GT_TRUE);


}

/**
* @internal prvTgfVntOamPortUnidirectionalTransmitModeTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         Packet is captured on ports 0,18,23.
*/
GT_VOID prvTgfVntOamPortUnidirectionalTransmitModeTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS    rc = GT_OK;
    GT_U32       portIter = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: send BC packet from port 1 with: */
    /* AUTODOC:   DA=FF:FF:FF:FF:FF:FF, SA=00:00:00:00:00:02, VID=5 */
    prvTgfVntTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo);

    /* AUTODOC: verify Tx traffic on ports [0,2,3] */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }

        /* check Tx counters */
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
    }

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
}

/**
* @internal prvTgfVntOamPortUnidirectionalTransmitModeRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfVntOamPortUnidirectionalTransmitModeRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;


    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: disable Force Link Down on port 0 */
    prvTgfPortForceLinkDownEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChPortForceLinkDownEnableSet: %d %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS], GT_FALSE);

    /* AUTODOC: enable unidirectional transmit on port 0 */
    rc = prvTgfVntOamPortUnidirectionalEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfVntOamPortUnidirectionalEnableSet: %d %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS], GT_FALSE);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* invalidate VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d",
                                 PRV_TGF_VLANID_CNS);
    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);
}


