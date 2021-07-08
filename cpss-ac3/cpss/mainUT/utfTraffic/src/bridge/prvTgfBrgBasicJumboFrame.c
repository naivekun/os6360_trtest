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
* @file prvTgfBrgBasicJumboFrame.c
*
* @brief Basic Bridge Jumbo Frame
*
* @version   4
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>
#include <trafficEngine/tgfTrafficEngine.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfPortGen.h>
#include <bridge/prvTgfBrgBasicJumboFrame.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN0 Id */
#define PRV_TGF_VLANID_CNS        5

/* send port index */
#define PRV_TGF_SEND_PORT_IDX_CNS 0

/* count data of payload */
#define PRV_TGF_MRU_VAL_CNS       1970

/* max MRU size */
#define PRV_TGF_MAX_MRU_SIZE_CNS  10240

/* count of VFD elements */
#define PRV_TGF_NUM_OF_VFD_CNS    2

/* mask for VFD */
#define PRV_TGF_VFD_MASKS_CNS     3 /* 1|2 */

/* mask for VFD */
#define PRV_TGF_PORT_CAPTURE_IDX_CNS 3

/* MRU default value */
#define PRV_TGF_DEF_MRU_VALUE_CNS 1518

/* default number of packets to send */
static GT_U32  prvTgfBurstCnt = 1;

/* L2 part of unknown UC packet */
static TGF_PACKET_L2_STC prvTgfUcPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x88}  /* srcMac */
};

/* L2 part of BC packet */
static TGF_PACKET_L2_STC prvTgfBcPacketL2Part =
{
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, /* dstMac */
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}  /* srcMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS, /* etherType */
    0,  0,  PRV_TGF_VLANID_CNS       /* pri, cfi, VlanId */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[PRV_TGF_MRU_VAL_CNS] =
{
    0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPayloadDataArr), 	 /* dataLength */
    prvTgfPayloadDataArr     	 	 /* dataPtr */
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* PARTS of unknown UC packet */
static TGF_PACKET_PART_STC prvTgfUcPcktArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfUcPacketL2Part}, /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* PARTS of BC packet */
static TGF_PACKET_PART_STC prvTgfBcPcktArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfBcPacketL2Part}, /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* number packets in array of Global Packet Info */
#define PRV_TGF_PACKET_NUM_CNS   2

/* global Packet Info structure */
static TGF_PACKET_STC  prvTgfGlobalPacketInfo [PRV_TGF_PACKET_NUM_CNS] =
{
	/* unknown UC */
	{
		PRV_TGF_PACKET_LEN_CNS,                                 /* totalLen */
		sizeof(prvTgfUcPcktArray) / sizeof(TGF_PACKET_PART_STC),/* numOfParts */
		prvTgfUcPcktArray                                       /* partsArray */
	},
	/* BC */
	{
		PRV_TGF_PACKET_LEN_CNS,                                 /* totalLen */
		sizeof(prvTgfBcPcktArray) / sizeof(TGF_PACKET_PART_STC),/* numOfParts */
		prvTgfBcPcktArray                                       /* partsArray */
	}
};

/* how many packets should be recived */
static GT_U8 prvTgfNumPcktsRxArr[2][4] = {{1,0,0,1}, {1,0,0,1}};

/* how many packets should be transmited */
static GT_U8 prvTgfNumPcktsTxArr[2][4] = {{1,1,1,1}, {1,1,1,1}};

/* parameters that is needed to be restored */
static struct
{
    GT_U32 pcktSizeMruArr[PRV_TGF_DEFAULT_NUM_PORTS_IN_TEST_CNS];
} prvTgfRestoreCfg;


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfBrgBasicJumboFrameConfigSet function
* @endinternal
*
* @brief   Setup base configuration
*/
GT_VOID prvTgfBrgBasicJumboFrameConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_FAIL;
    GT_U8       profileIter;
    GT_U8       portIter;
    GT_U8       tagArray[] = {1,1,1,1};


    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("------- Jumbo Frame Config Set\n");

    /* AUTODOC: create VLAN 2 with all tagged ports */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_CNS, prvTgfPortsArray,
                                           NULL, tagArray, prvTgfPortsNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgDefVlanEntryWithPortsSet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: enable Jumbo frames for all used ports */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* get default MRU for ports */
        rc = prvTgfPortMruGet(prvTgfPortsArray[portIter],
                              &prvTgfRestoreCfg.pcktSizeMruArr[portIter]);
        UTF_VERIFY_EQUAL1_STRING_MAC(rc, GT_OK, "ERROR of prvTgfPortMruGet %d", GT_TRUE);

        /* AUTODOC: set MRU value 10240 for all ports */
        rc = prvTgfPortMruSet(prvTgfPortsArray[portIter], PRV_TGF_MAX_MRU_SIZE_CNS);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfPortMruSet: %d",
                                     prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: set MRU for each profiles to 10240) */
    for (profileIter = 0 ; profileIter < 8 ; profileIter++ )
    {
        rc = prvTgfBrgVlanMruProfileValueSet(profileIter, PRV_TGF_MAX_MRU_SIZE_CNS);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgVlanMruProfileValueSet: %d",
                                     prvTgfPortsArray[portIter]);
    }
}

/**
* @internal prvTgfBrgBasicJumboFrameTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBrgBasicJumboFrameTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS        rc = GT_FAIL;
    GT_U32           packetIter  = 0;
    GT_U32           portIter    = 0;
    GT_U32           numTriggers = 0;
    TGF_VFD_INFO_STC vfdArray[PRV_TGF_NUM_OF_VFD_CNS];
    CPSS_INTERFACE_INFO_STC  portInt;

    /* AUTODOC: GENERATE TRAFIC: */
    PRV_UTF_LOG0_MAC("------- Jumbo Frame UC/BC Traffic Generate \n");

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersReset");

    /* setup nexthope portInterface for capturing */
    portInt.type            = CPSS_INTERFACE_PORT_E;
    portInt.devPort.hwDevNum  = prvTgfDevNum;
    portInt.devPort.portNum = prvTgfPortsArray[PRV_TGF_PORT_CAPTURE_IDX_CNS];

    /* AUTODOC: send packet with mac DA 00:00:00:00:00:01 from port 0 (unknown UC) */
    /* AUTODOC:   verify packet arrives to all ports in Vlan */
    /* AUTODOC: send packet with mac DA FF:FF:FF:FF:FF:FF from port 0 (BC) */
    /* AUTODOC:   verify packet arrives to all ports */
    for (packetIter = 0; packetIter < PRV_TGF_PACKET_NUM_CNS; packetIter++)
    {
        /* print log for each stage */
        switch (packetIter)
        {
            case 0:
                PRV_UTF_LOG0_MAC("======= Sending unicast packet =======\n");
                break;

            case 1:
                PRV_UTF_LOG0_MAC("======= Sending broadcast packet =======\n");
                break;

            default:
                PRV_UTF_LOG0_MAC("======= Unknown stage =======\n");
        }

        /* clear capturing RxPcktTable */
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficTableRxPcktTblClear");

        /* enable capture on next hop port/trunk */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInt, TGF_CAPTURE_MODE_PCL_E, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                     prvTgfDevNum, portInt.devPort.portNum );

        /* setup Packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfGlobalPacketInfo[packetIter],
                                 prvTgfBurstCnt, 0, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* send Packet */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* disable capture on a MC subscriber port */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInt, TGF_CAPTURE_MODE_PCL_E, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                     prvTgfDevNum, portInt.devPort.portNum);

        /* read and check counters */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            /* read unknown unicast and broadcast counters */
            rc = prvTgfEthCountersCheck(prvTgfDevNum,  prvTgfPortsArray[portIter],
                                        prvTgfNumPcktsRxArr[packetIter][portIter],
                                        prvTgfNumPcktsTxArr[packetIter][portIter],
                                        prvTgfGlobalPacketInfo[packetIter].totalLen,
                                        prvTgfBurstCnt);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck: %d, %d\n",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);
        }

        /* get trigger counters where packet has MAC DA */
        vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
        vfdArray[0].modeExtraInfo = 0;
        vfdArray[0].offset = 0;
        vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
        cpssOsMemCpy(vfdArray[0].patternPtr,
                     packetIter==0?prvTgfUcPacketL2Part.daMac:prvTgfBcPacketL2Part.daMac,
                     sizeof(TGF_MAC_ADDR));

        /* get trigger counters where packet has MAC SA */
        vfdArray[1].mode = TGF_VFD_MODE_STATIC_E;
        vfdArray[1].modeExtraInfo = 0;
        vfdArray[1].offset = sizeof(TGF_MAC_ADDR);
        vfdArray[1].cycleCount = sizeof(TGF_MAC_ADDR);
        cpssOsMemCpy(vfdArray[1].patternPtr,
                     packetIter==0?prvTgfUcPacketL2Part.saMac:prvTgfBcPacketL2Part.saMac,
                     sizeof(TGF_MAC_ADDR));

        rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInt, PRV_TGF_NUM_OF_VFD_CNS,
                                                            vfdArray, &numTriggers);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, (GT_NO_MORE == rc) ? GT_OK : rc,
                "ERROR of tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d\n", numTriggers);
        UTF_VERIFY_EQUAL1_STRING_MAC(PRV_TGF_VFD_MASKS_CNS, numTriggers,
                "got wrong numTriggers: numTriggers = %d\n", numTriggers);

        /* check if captured packet has the same MAC DA as prvTgfArpMac */
        UTF_VERIFY_EQUAL0_STRING_MAC(1, (PRV_TGF_VFD_MASKS_CNS & numTriggers) & 1,
                                     "\n  wrong MAC DA of captured packet");

        /* check if captured packet has the same MAC SA as prvTgfArpMac */
        UTF_VERIFY_EQUAL0_STRING_MAC(2, (PRV_TGF_VFD_MASKS_CNS & numTriggers) & 2,
                                     "\n  wrong MAC SA of captured packet");
    }
}

/**
* @internal prvTgfBrgBasicJumboFrameConfigRestore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfBrgBasicJumboFrameConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_FAIL;
    GT_U8       portIter;
    GT_U8       profileIter;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficTableRxPcktTblClear");

	/* AUTODOC: restore default MRU for all ports */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
		rc = prvTgfPortMruSet(prvTgfPortsArray[portIter],
		                      prvTgfRestoreCfg.pcktSizeMruArr[portIter]);
		UTF_VERIFY_EQUAL1_STRING_MAC(rc, GT_OK, "ERROR of prvTgfPortMruSet %d", GT_TRUE);
    }

    /* AUTODOC: restore MRU for each profiles to 10240) */
    for (profileIter = 0 ; profileIter < 8 ; profileIter++ )
    {
        rc = prvTgfBrgVlanMruProfileValueSet(profileIter, PRV_TGF_DEF_MRU_VALUE_CNS);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgVlanMruProfileValueSet");
    }

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

}

