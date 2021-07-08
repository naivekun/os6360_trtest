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
* @file prvTgfCscdDsaSrcIsTrunk.c
*
* @brief Source is trunk, Source port/trunk, MC filter enable fields in
* Forward eDsa tag test advanced UT.
*
* @version   2
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
#include <common/tgfCscdGen.h>
#include <common/tgfTrunkGen.h>
#include <cscd/prvTgfCscdDsaSrcIsTrunk.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN Id */
#define PRV_TGF_VLANID_1_CNS        6

/* traffic generation sending port */
#define ING_NETWORK_PORT_IDX_CNS    0
#define ING_CASCADE_PORT_IDX_CNS    1

/* target port */
#define EGR_NETWORK_PORT_IDX_CNS    3
#define EGR_CASCADE_PORT_IDX_CNS    2

/* max vlan id value. For random generation */
#define PRV_TGF_MAX_VLAN_ID         _4K

/* max trunk ID value. For random generation*/
#define PRV_TGF_MAX_TRUNK_ID        256

/* trunk ID field. Will be initialized later */
static GT_TRUNK_ID prvTgfTrunkId;

/* vlan used by test. Will be initialized later */
static TGF_VLAN_ID prvTgfVlanId;

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 1;

static GT_BOOL orig_enableOwnDevFltr = GT_FALSE;

static GT_BOOL orig_enableOrigSrcPortFltr = GT_TRUE;

/******************************* Test packets *********************************/

static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x17},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x08}                 /* saMac */
};

/* VLAN_TAG0 part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, 0                            /* pri, cfi, VlanId */
};

/* VLAN_TAG1 part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag1Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_1_CNS                          /* pri, cfi, VlanId */
};

/********************** DSA tag  ***************************/
static TGF_DSA_DSA_FORWARD_STC  prvTgfPacketDsaTagPart_forward = {
    PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E , /*srcIsTagged*/
    0,/*srcHwDev*/
    GT_FALSE,/* srcIsTrunk */
    /*union*/
    {
        /*trunkId*/
        0/*portNum*/
    },/*source;*/

    0,/*srcId*/

    GT_FALSE,/*egrFilterRegistered*/
    GT_FALSE,/*wasRouted*/
    0,/*qosProfileIndex*/

    /*CPSS_INTERFACE_INFO_STC         dstInterface*/
    {
        CPSS_INTERFACE_VID_E,/*type*/

        /*struct*/{
            0,/*devNum*/
            0/*portNum*/
        },/*devPort*/

        0,/*trunkId*/
        0, /*vidx*/
        0,/*vlanId*/
        0,/*devNum*/
        0,/*fabricVidx*/
        0 /*index*/
    },/*dstInterface*/
    GT_FALSE,/*isTrgPortValid*/
    0,/*dstEport*/
    0,/*TPID Index*/
    GT_FALSE,/*origSrcPhyIsTrunk*/
    /*union*/
    {
        /*trunkId*/
        0/*portNum*/
    },/*origSrcPhy;*/
    GT_TRUE,/*phySrcMcFilterEnable*/
    0, /* hash */
    GT_TRUE /*skipFdbSaLookup*/
};

static TGF_PACKET_DSA_TAG_STC  prvTgfPacketDsaTagPart = {
    TGF_DSA_CMD_FORWARD_E ,/*dsaCommand*/
    TGF_DSA_4_WORD_TYPE_E ,/*dsaType*/

    /*TGF_DSA_DSA_COMMON_STC*/
    {
        0,/*vpt*/
        0,/*cfiBit*/
        0,/*vid*/
        GT_FALSE,/*dropOnSource*/
        GT_FALSE/*packetIsLooped*/
    }/*commonParams*/

/*    union{
        TGF_DSA_DSA_TO_CPU_STC         toCpu;
        TGF_DSA_DSA_FROM_CPU_STC       fromCpu;
        TGF_DSA_DSA_TO_ANALYZER_STC    toAnalyzer;
        TGF_DSA_DSA_FORWARD_STC        forward;
    }dsaInfo;*/
};

/* DATA of bypass packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f
};

/* Bypass PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTag1Part},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + 2 * TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS  PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfCascadePacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_DSA_TAG_E,   &prvTgfPacketDsaTagPart},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTag1Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_CASCADE_PACKET_LEN_CNS \
 (TGF_L2_HEADER_SIZE_CNS + TGF_eDSA_TAG_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS \
                                                + sizeof(prvTgfPayloadDataArr))

/* LENGTH of packet with CRC */
#define PRV_TGF_CASCADE_PACKET_CRC_LEN_CNS  (PRV_TGF_CASCADE_PACKET_LEN_CNS \
                                                          + TGF_CRC_LEN_CNS)

/* PACKET to send info */
static TGF_PACKET_STC prvTgfCascadePacketInfo = {
        PRV_TGF_CASCADE_PACKET_LEN_CNS,                                 /* totalLen */
    sizeof(prvTgfCascadePacketPartArray) / sizeof(TGF_PACKET_PART_STC),/* numOfParts */
    prvTgfCascadePacketPartArray                                       /* partsArray */
};

/******************************************************************************\
 *                   Private variables & definitions                          *
\******************************************************************************/

/* Parameters needed to be restored */
static GT_HW_DEV_NUM prvTgfSavedHwDevNum;
static GT_U32        prvTgfSavedPortsArray[PRV_TGF_MAX_PORTS_NUM_CNS];

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/
extern GT_BOOL prvTgfTrafficGeneratorPacketDsaTagGetOrigIsTrunk
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM           ingressPortNum,
    IN  GT_BOOL                        origIsTrunk
);
extern GT_BOOL prvTgfTrafficGeneratorPacketDsaTagGetOrigPhyIsTrunk
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM           ingressPortNum,
    IN  GT_BOOL                        origPhysicalIsTrunk
);

/**
* @internal setEgressPortCascade function
* @endinternal
*
* @brief   set/unset the egress port as cascade with DSA tag .
*
* @param[in] egressPort               - egress port to be DSA tagged
* @param[in] enable                   - enable/disable the cascade port.
*                                       None
*/
static GT_VOID setEgressPortCascade
(
    IN GT_PHYSICAL_PORT_NUM     egressPort,
    IN GT_BOOL                  enable
)
{
    GT_STATUS rc;

    /* AUTODOC: set the egress port to be DSA tagged */
    rc = tgfTrafficGeneratorEgressCscdPortEnableSet(
        prvTgfDevNum, egressPort, enable,
        enable ? CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E : CPSS_CSCD_PORT_NETWORK_E);

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                         "tgfTrafficGeneratorEgressCscdPortEnableSet: [%d] \n");
}
/**
* @internal setIngressPortCascade function
* @endinternal
*
* @brief   set/unset the ingress port as cascade with DSA tag .
*/
static GT_VOID setIngressPortCascade
(
    IN GT_PHYSICAL_PORT_NUM     ingressPort,
    IN GT_BOOL                  enable
)
{
    GT_STATUS rc;

    /* AUTODOC: set the ingress port to be DSA tagged */
    rc = tgfTrafficGeneratorIngressCscdPortEnableSet(
                                             prvTgfDevNum, ingressPort, enable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                        "tgfTrafficGeneratorIngressCscdPortEnableSet: [%d] \n");
}

/**
* @internal setPortAsTrunkMember function
* @endinternal
*
* @brief   set/unset port as member of trunk.
*
* @param[in] portIdx                  - port index
* @param[in] trunkId                  - trunk ID
* @param[in] enable                   - enable/disable port as member of trunk
*                                       None
*/
static GT_VOID setPortAsTrunkMember
(
    IN GT_U32                   portIdx,
    IN GT_TRUNK_ID              trunkId,
    IN GT_BOOL                  enable
)
{
    GT_STATUS               rc;
    CPSS_TRUNK_MEMBER_STC   trunkMembersArray[1] = {{0, 0}};

    /* set trunk members */
    trunkMembersArray[0].hwDevice = prvTgfDevNum;
    trunkMembersArray[0].port     = prvTgfPortsArray[portIdx];

    /* AUTODOC: create trunk 1 with ports [0,1] */

    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);
    if (GT_TRUE == enable)
    {
        rc = prvTgfTrunkMembersSet(trunkId, 1, trunkMembersArray, 0, NULL);
    }
    else
    {
        rc = prvTgfTrunkMembersSet(trunkId, 0, NULL, 0, NULL);
    }

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTrunkMembersSet: %d %d", prvTgfDevNum, trunkId);

}

/**
* @internal prvTgfCscdDsaFrwdSrcIsTrunkTestPacketSend function
* @endinternal
*
* @brief   Function sends packet.
*
* @param[in] portInterfacePtr         - (pointer to) port interface
* @param[in] packetInfoPtr            - (pointer to) the packet info
* @param[in] ingressPortIndex         - ingress port index
*                                       None
*/
static GT_VOID prvTgfCscdDsaFrwdSrcIsTrunkTestPacketSend
(
    IN CPSS_INTERFACE_INFO_STC  *portInterfacePtr,
    IN TGF_PACKET_STC           *packetInfoPtr,
    IN GT_U32                   ingressPortIndex
)
{
    GT_STATUS rc          = GT_OK;
    GT_U32    portsCount  = prvTgfPortsNum;
    GT_U32    portIter    = 0;

    /* use different 'etherType for vlan tagging' instead of 0x8100 */
    /*this will make our packet that has '0x8100' to be recognized as 'untagged'
      by the 'traffic generator' and also the CPSS will not override the
      ethertype of tag0 with the DSA tag */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_NON_VALID_TAG_CNS);

    /* AUTODOC: reset counters */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr, prvTgfBurstCount,
                             0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, prvTgfBurstCount, 0, 0);

    /* AUTODOC: clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: Start capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(portInterfacePtr,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                           "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 portInterfacePtr->devPort.hwDevNum,
                                 portInterfacePtr->devPort.portNum,
                                 GT_TRUE);

    /* AUTODOC: send packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum,
                                   prvTgfPortsArray[ingressPortIndex]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[ingressPortIndex]);

    /* AUTODOC: wait for packets come to CPU */
    (void) tgfTrafficGeneratorRxInCpuNumWait(prvTgfBurstCount, 500, NULL);

    /* AUTODOC: Stop capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(portInterfacePtr,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                           "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 portInterfacePtr->devPort.hwDevNum,
                                 portInterfacePtr->devPort.portNum,
                                 GT_FALSE);

    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterfacePtr->devPort.portNum);

    /* AUTODOC: restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);
}

/**
* @internal prvTgfCscdDsaFrwdSrcIsTrunkTestSendAndCheck function
* @endinternal
*
* @brief   Function sends packet and check results
*
* @param[in] ingressPortIndex         - ingress port index
* @param[in] egressPortIndex          - egress port index
* @param[in] packetInfoPtr            - (pointer to) send packet
* @param[in] isExpected               - is packet expected on source port
*                                       None
*/
static GT_VOID prvTgfCscdDsaFrwdSrcIsTrunkTestSendAndCheck
(
    IN GT_U32                   ingressPortIndex,
    IN GT_U32                   egressPortIndex,
    IN TGF_PACKET_STC           *packetInfoPtr,
    IN GT_BOOL                  isExpected
)
{
    GT_STATUS                       rc;
    CPSS_INTERFACE_INFO_STC         portInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    GT_U32                          portIter;
    GT_U32                          actualNumOfPackets;

    /* AUTODOC: set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[egressPortIndex];

    /* bind the DSA tag FORWARD part */
    prvTgfPacketDsaTagPart.dsaInfo.forward = prvTgfPacketDsaTagPart_forward;

    /* AUTODOC: send packet. */
    prvTgfCscdDsaFrwdSrcIsTrunkTestPacketSend(&portInterface, packetInfoPtr, ingressPortIndex);

    /* AUTODOC: check counters */

    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum,
                                       prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (egressPortIndex == portIter)
        {

            /* AUTODOC: check the captured packet */
            packetInfoPtr = (EGR_CASCADE_PORT_IDX_CNS == egressPortIndex) ?
                                 &prvTgfCascadePacketInfo : &prvTgfPacketInfo;

            rc = tgfTrafficGeneratorPortTxEthCaptureCompare(
                                          &portInterface,
                                          packetInfoPtr,
                                          prvTgfBurstCount,/*numOfPackets*/
                                          0/* vfdNum */,
                                          NULL /* vfdArray */,
                                          NULL, /* bytesNum's skip list */
                                          0,    /* length of skip list */
                                          &actualNumOfPackets,
                                          NULL/* onFirstPacketNumTriggersBmpPtr */);
            rc = rc == GT_NO_MORE ? GT_OK : rc;
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                         "tgfTrafficGeneratorPortTxEthTriggerCountersGet:"
                                         " port = %d, rc = 0x%02X\n", portInterface.devPort.portNum, rc);

            UTF_VERIFY_EQUAL3_STRING_MAC(prvTgfBurstCount, actualNumOfPackets,
                       "Triggers number: port #%d, expected - %d, received - %d\n",
                       portInterface.devPort.portNum, prvTgfBurstCount, actualNumOfPackets);
        }
        else if (ING_NETWORK_PORT_IDX_CNS == portIter)
        {
            if (GT_TRUE == isExpected)
            {
                UTF_VERIFY_EQUAL3_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                            "Tx another goodPktsSent counter %d, than expected %d, on port %d",
                                             portCntrs.goodPktsSent.l[0], prvTgfBurstCount,
                                             prvTgfPortsArray[portIter]);
            }
            else
            {
                UTF_VERIFY_EQUAL3_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                            "Tx another goodPktsSent counter %d, than expected %d, on port %d",
                                             portCntrs.goodPktsSent.l[0], 0,
                                             prvTgfPortsArray[portIter]);
            }
        }
    }

}

/**
* @internal prvTgfCscdDsaFrwdSrcIsTrunkTestConfigurationSet function
* @endinternal
*
* @brief   Test initial configurations
*/
GT_VOID prvTgfCscdDsaFrwdSrcIsTrunkTestConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS        rc;
    GT_U32           portIter = 0;
    GT_U32            portsArray[4];
    GT_U8            tagArray[] = {1, 1, 1, 1};

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: save prvTgfPortsArray */
    cpssOsMemCpy(prvTgfSavedPortsArray, prvTgfPortsArray,
                 sizeof(prvTgfSavedPortsArray));

    /* AUTODOC: re-arrange prvTgfPortsArray to ensure some ports (will be
     * configured cascade) are not remote ports */
    if (UTF_CPSS_DXCH_XCAT3X_CHECK_MAC(prvTgfDevNum))
    {
        rc = prvTgfDefPortsArrange(GT_FALSE,
                                   ING_CASCADE_PORT_IDX_CNS,
                                   EGR_CASCADE_PORT_IDX_CNS,
                                   -1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortsArray rearrangement can't be done");
    }

    /* save Hardware device number */
    rc = prvUtfHwDeviceNumberGet(prvTgfDevNum, &prvTgfSavedHwDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvUtfHwDeviceNumberGet");

    /* initialize randomizer */
    cpssOsSrand(prvUtfSeedFromStreamNameGet());

    /* AUTODOC: initialize vlan and target device by random values */
    prvTgfVlanId  = cpssOsRand() % (PRV_TGF_MAX_VLAN_ID - 2) + 2;
    if (PRV_TGF_MAX_TRUNK_ID <= (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->bridge.l2EcmpNumMembers / 16))
    {
        prvTgfTrunkId = cpssOsRand() % (PRV_TGF_MAX_TRUNK_ID - 1) + 1;
    }
    else
    {
        prvTgfTrunkId = cpssOsRand() % ((GT_TRUNK_ID)(PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->bridge.l2EcmpNumMembers / 16) - 1) + 1;
    }


    PRV_UTF_LOG2_MAC("We will use vlanId = %d, trunkId = %d ", prvTgfVlanId, prvTgfTrunkId);

    portsArray[0] = prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS];
    portsArray[1] = prvTgfPortsArray[ING_CASCADE_PORT_IDX_CNS];
    portsArray[2] = prvTgfPortsArray[EGR_CASCADE_PORT_IDX_CNS];
    portsArray[3] = prvTgfPortsArray[EGR_NETWORK_PORT_IDX_CNS];

    /* AUTODOC: create VLAN with tagged port [0] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_1_CNS, portsArray,
                                           NULL, tagArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", PRV_TGF_VLANID_1_CNS);

    /* AUTODOC: add tag PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E for port 0 */
    rc = prvTgfBrgVlanTagMemberAdd(PRV_TGF_VLANID_1_CNS,
                                   prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS],
                                   PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTagMemberAdd: %d", prvTgfDevNum);

    /* AUTODOC: create VLAN 6 with tagged ports [1,2,3] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(prvTgfVlanId, portsArray + 1,
                                           NULL, tagArray + 1, 3);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfVlanId);

    /* AUTODOC: add tag PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E for ports 1,2,3 */
    for (portIter = 1; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfBrgVlanTagMemberAdd(prvTgfVlanId,
                                       prvTgfPortsArray[portIter],
                                       PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTagMemberAdd: %d", prvTgfDevNum);
    }

    prvTgfPacketDsaTagPart_forward.srcHwDev            = prvTgfSavedHwDevNum;
    prvTgfPacketVlanTag0Part.vid                       = prvTgfVlanId;
    prvTgfPacketDsaTagPart.commonParams.vid            = prvTgfVlanId;
    prvTgfPacketDsaTagPart_forward.dstInterface.vidx   = 0xFFF;

    setIngressPortCascade( prvTgfPortsArray[ING_CASCADE_PORT_IDX_CNS], GT_TRUE );
    setEgressPortCascade(  prvTgfPortsArray[EGR_CASCADE_PORT_IDX_CNS], GT_TRUE );


}

/**
* @internal prvTgfCscdDsaFrwdSrcIsTrunkTestConfigurationRestore function
* @endinternal
*
* @brief   Test restore configurations
*/
GT_VOID prvTgfCscdDsaFrwdSrcIsTrunkTestConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS                   rc;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: restore default Hw device number */
    rc = prvUtfHwDeviceNumberSet(prvTgfDevNum, prvTgfSavedHwDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvUtfHwDeviceNumberSet");

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d", GT_TRUE);

    setIngressPortCascade( prvTgfPortsArray[ING_CASCADE_PORT_IDX_CNS], GT_FALSE );
    setEgressPortCascade(  prvTgfPortsArray[EGR_CASCADE_PORT_IDX_CNS], GT_FALSE );

    /* AUTODOC: invalidate vlans entry (and reset vlans entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(prvTgfVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, prvTgfVlanId);

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_1_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_1_CNS);

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: restore prvTgfPortsArray */
    cpssOsMemCpy(prvTgfPortsArray, prvTgfSavedPortsArray, sizeof(prvTgfSavedPortsArray));
}

/**
* @internal prvTgfCscdDsaFrwdSrcIsTrunkTest function
* @endinternal
*
* @brief   Test for Egress Filter Registered field in Forward eDsa tag
*/
GT_VOID prvTgfCscdDsaFrwdSrcIsTrunkTest
(
    GT_VOID
)
{
    GT_STATUS               rc;

    /* AUTODOC: Set common test configuration */
    prvTgfCscdDsaFrwdSrcIsTrunkTestConfigurationSet();

    /* AUTODOC: CASE 1: from Network port to Cascade port */
    PRV_UTF_LOG2_MAC(
            "\n*** CASE 1: from Network port %02d to Cascade port %02d ***\n",
            prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS],
            prvTgfPortsArray[EGR_CASCADE_PORT_IDX_CNS]);

    /* AUTODOC: set Network port 0 as trunk member */
    setPortAsTrunkMember(ING_NETWORK_PORT_IDX_CNS, prvTgfTrunkId, GT_TRUE);

    prvTgfPacketDsaTagPart_forward.srcIsTrunk           =
        prvTgfTrafficGeneratorPacketDsaTagGetOrigIsTrunk(prvTgfDevNum,
            prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS],
            GT_TRUE);
    if(prvTgfPacketDsaTagPart_forward.srcIsTrunk == GT_TRUE)
    {
        prvTgfPacketDsaTagPart_forward.source.trunkId       = prvTgfTrunkId;
    }
    else /* due to Erratum !!! (see prvTgfTrafficGeneratorPacketDsaTagGetOrigIsTrunk(...))*/
    {
        prvTgfPacketDsaTagPart_forward.source.portNum       = prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS];
    }

    prvTgfPacketDsaTagPart_forward.origSrcPhyIsTrunk    =
        prvTgfTrafficGeneratorPacketDsaTagGetOrigPhyIsTrunk(prvTgfDevNum,
            prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS],
            GT_TRUE);
    if(prvTgfPacketDsaTagPart_forward.origSrcPhyIsTrunk == GT_TRUE)
    {
        prvTgfPacketDsaTagPart_forward.origSrcPhy.trunkId   = prvTgfTrunkId;
    }
    else /* due to Erratum !!! (see prvTgfTrafficGeneratorPacketDsaTagGetOrigIsTrunk(...))*/
    {
        prvTgfPacketDsaTagPart_forward.origSrcPhy.portNum   = prvTgfTrafficGeneratorPacketDsaTagGetOrigSrcPhy(
                                                            prvTgfDevNum,
                                                            prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS]);
    }

    prvTgfPacketDsaTagPart_forward.phySrcMcFilterEnable = GT_TRUE;

    /* AUTODOC: Send  unregistered UC packet*/
    prvTgfCscdDsaFrwdSrcIsTrunkTestSendAndCheck(ING_NETWORK_PORT_IDX_CNS,
                                                EGR_CASCADE_PORT_IDX_CNS,
                                                &prvTgfPacketInfo,
                                                GT_TRUE);

    prvTgfPacketDsaTagPart_forward.srcIsTrunk           = GT_FALSE;
    prvTgfPacketDsaTagPart_forward.source.portNum       = prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS];
    prvTgfPacketDsaTagPart_forward.origSrcPhyIsTrunk    = GT_FALSE;
    prvTgfPacketDsaTagPart_forward.origSrcPhy.portNum   = prvTgfTrafficGeneratorPacketDsaTagGetOrigSrcPhy(
                                                            prvTgfDevNum,
                                                            prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS]);

    /* AUTODOC: invalidate trunk entry */
    setPortAsTrunkMember(ING_NETWORK_PORT_IDX_CNS, prvTgfTrunkId, GT_FALSE);

    /* AUTODOC: Send unregistered UC packet*/
    prvTgfCscdDsaFrwdSrcIsTrunkTestSendAndCheck(ING_NETWORK_PORT_IDX_CNS,
                                                EGR_CASCADE_PORT_IDX_CNS,
                                                &prvTgfPacketInfo,
                                                GT_TRUE);

    prvTgfPacketDsaTagPart_forward.srcIsTrunk           =
        prvTgfTrafficGeneratorPacketDsaTagGetOrigIsTrunk(prvTgfDevNum,
            prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS],
            GT_TRUE);
    if(prvTgfPacketDsaTagPart_forward.srcIsTrunk == GT_TRUE)
    {
        prvTgfPacketDsaTagPart_forward.source.trunkId       = prvTgfTrunkId;
    }
    else /* due to Erratum !!! (see prvTgfTrafficGeneratorPacketDsaTagGetOrigIsTrunk(...))*/
    {
        prvTgfPacketDsaTagPart_forward.source.portNum       = prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS];
    }

    prvTgfPacketDsaTagPart_forward.origSrcPhyIsTrunk    =
        prvTgfTrafficGeneratorPacketDsaTagGetOrigPhyIsTrunk(prvTgfDevNum,
            prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS],
            GT_TRUE);
    if(prvTgfPacketDsaTagPart_forward.origSrcPhyIsTrunk == GT_TRUE)
    {
        prvTgfPacketDsaTagPart_forward.origSrcPhy.trunkId   = prvTgfTrunkId;
    }
    else /* due to Erratum !!! (see prvTgfTrafficGeneratorPacketDsaTagGetOrigIsTrunk(...))*/
    {
        prvTgfPacketDsaTagPart_forward.origSrcPhy.portNum   = prvTgfTrafficGeneratorPacketDsaTagGetOrigSrcPhy(
                                                            prvTgfDevNum,
                                                            prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS]);
    }
    prvTgfPacketDsaTagPart_forward.phySrcMcFilterEnable = GT_FALSE;

    /* AUTODOC: disable source physical port/trunk filtering */
    prvTgfBrgMcPhysicalSourceFilteringEnableSet(prvTgfDevNum,
                                               prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS],
                                               GT_FALSE);

    /* AUTODOC: set Network port 0 as trunk member */
    setPortAsTrunkMember(ING_NETWORK_PORT_IDX_CNS, prvTgfTrunkId, GT_TRUE);

    /* AUTODOC: Send  unregistered UC packet*/
    prvTgfCscdDsaFrwdSrcIsTrunkTestSendAndCheck(ING_NETWORK_PORT_IDX_CNS,
                                                EGR_CASCADE_PORT_IDX_CNS,
                                                &prvTgfPacketInfo,
                                                GT_TRUE);


    prvTgfPacketDsaTagPart_forward.srcIsTrunk           = GT_FALSE;
    prvTgfPacketDsaTagPart_forward.source.portNum       = prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS];
    prvTgfPacketDsaTagPart_forward.origSrcPhyIsTrunk    = GT_FALSE;
    prvTgfPacketDsaTagPart_forward.origSrcPhy.portNum   = prvTgfTrafficGeneratorPacketDsaTagGetOrigSrcPhy(
                                                            prvTgfDevNum,
                                                            prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS]);

    /* AUTODOC: invalidate trunk entry */
    setPortAsTrunkMember(ING_NETWORK_PORT_IDX_CNS, prvTgfTrunkId, GT_FALSE);

    /* AUTODOC: Send unregistered UC packet*/
    prvTgfCscdDsaFrwdSrcIsTrunkTestSendAndCheck(ING_NETWORK_PORT_IDX_CNS,
                                                EGR_CASCADE_PORT_IDX_CNS,
                                                &prvTgfPacketInfo,
                                                GT_TRUE);

    prvTgfPacketDsaTagPart_forward.phySrcMcFilterEnable = GT_TRUE;

    /* AUTODOC: enable source physical port/trunk filtering */
    prvTgfBrgMcPhysicalSourceFilteringEnableSet(prvTgfDevNum,
                                               prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS],
                                               GT_TRUE);

    /* AUTODOC: CASE 2: from Cascade port to Cascade port */
        PRV_UTF_LOG2_MAC(
            "\n*** CASE 2: from Cascade port %02d to Cascade port %02d ***\n",
            prvTgfPortsArray[ING_CASCADE_PORT_IDX_CNS],
            prvTgfPortsArray[EGR_CASCADE_PORT_IDX_CNS]);

    /* set Hw device number to be different from srcHwDev in the eDSA tag */
    rc = prvUtfHwDeviceNumberSet(prvTgfDevNum, prvTgfSavedHwDevNum + 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvUtfHwDeviceNumberSet");

    if(prvUtfIsDoublePhysicalPortsModeUsed())
    {
        /* since the 'FROM_CPU' will not use 'isPhysicalPortValid' but use the eport
           we need to update the 'E2PHY' to redirect to 'local device' (hwDevNumForRestore)
           instead of 'previous' prvTgfTrgDev */
        prvUtfSetE2PhyEqualValue(ING_CASCADE_PORT_IDX_CNS);
    }

    prvTgfPacketDsaTagPart_forward.origSrcPhyIsTrunk    = GT_TRUE;
    prvTgfPacketDsaTagPart_forward.origSrcPhy.trunkId   = prvTgfTrunkId;
    prvTgfPacketDsaTagPart_forward.phySrcMcFilterEnable = GT_TRUE;

    /* AUTODOC: Send  unregistered UC packet*/
    prvTgfCscdDsaFrwdSrcIsTrunkTestSendAndCheck(ING_CASCADE_PORT_IDX_CNS,
                                                EGR_CASCADE_PORT_IDX_CNS,
                                                &prvTgfCascadePacketInfo,
                                                GT_FALSE);

    prvTgfPacketDsaTagPart_forward.origSrcPhyIsTrunk    = GT_FALSE;
    prvTgfPacketDsaTagPart_forward.origSrcPhy.portNum   = prvTgfTrafficGeneratorPacketDsaTagGetOrigSrcPhy(
                                                            prvTgfDevNum,
                                                            prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS]);

    /* AUTODOC: Send unregistered UC packet*/
    prvTgfCscdDsaFrwdSrcIsTrunkTestSendAndCheck(ING_CASCADE_PORT_IDX_CNS,
                                                EGR_CASCADE_PORT_IDX_CNS,
                                                &prvTgfCascadePacketInfo,
                                                GT_FALSE);

    prvTgfPacketDsaTagPart_forward.origSrcPhyIsTrunk    = GT_TRUE;
    prvTgfPacketDsaTagPart_forward.origSrcPhy.trunkId   = prvTgfTrunkId;
    prvTgfPacketDsaTagPart_forward.phySrcMcFilterEnable = GT_FALSE;

    /* AUTODOC: Send  unregistered UC packet*/
    prvTgfCscdDsaFrwdSrcIsTrunkTestSendAndCheck(ING_CASCADE_PORT_IDX_CNS,
                                                EGR_CASCADE_PORT_IDX_CNS,
                                                &prvTgfCascadePacketInfo,
                                                GT_FALSE);

    prvTgfPacketDsaTagPart_forward.origSrcPhyIsTrunk    = GT_FALSE;
    prvTgfPacketDsaTagPart_forward.origSrcPhy.portNum   = prvTgfTrafficGeneratorPacketDsaTagGetOrigSrcPhy(
                                                            prvTgfDevNum,
                                                            prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS]);

    /* AUTODOC: Send unregistered UC packet*/
    prvTgfCscdDsaFrwdSrcIsTrunkTestSendAndCheck(ING_CASCADE_PORT_IDX_CNS,
                                                EGR_CASCADE_PORT_IDX_CNS,
                                                &prvTgfCascadePacketInfo,
                                                GT_FALSE);

    prvTgfPacketDsaTagPart_forward.phySrcMcFilterEnable = GT_TRUE;

    /* AUTODOC: CASE 3: from Cascade port to Network port */
        PRV_UTF_LOG2_MAC(
            "\n*** CASE 3: from Cascade port %02d to Network port %02d ***\n",
            prvTgfPortsArray[ING_CASCADE_PORT_IDX_CNS],
            prvTgfPortsArray[EGR_NETWORK_PORT_IDX_CNS]);

    /* AUTODOC: invalidate vlans entry (and reset vlans entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(prvTgfVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, prvTgfVlanId);

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_1_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_1_CNS);

    /* AUTODOC: create VLAN with all tagged port */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd( prvTgfVlanId,
                                                   PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);
    UTF_VERIFY_EQUAL1_STRING_MAC( GT_OK, rc, "prvTgfBrgDefVlanEntryWriteWithTaggingCmd: %d",
                                  prvTgfDevNum);

    /* set Hw device number to be different from srcHwDev in the eDSA tag */
    rc = prvUtfHwDeviceNumberSet(prvTgfDevNum, prvTgfSavedHwDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvUtfHwDeviceNumberSet");

    if(prvUtfIsDoublePhysicalPortsModeUsed())
    {
        /* since the 'FROM_CPU' will not use 'isPhysicalPortValid' but use the eport
           we need to update the 'E2PHY' to redirect to 'local device' (hwDevNumForRestore)
           instead of 'previous' prvTgfTrgDev */
        prvUtfSetE2PhyEqualValue(ING_CASCADE_PORT_IDX_CNS);
    }

    prvTgfPacketDsaTagPart_forward.srcHwDev = prvTgfSavedHwDevNum;

    prvTgfCscdDsaSrcDevFilterGet(&orig_enableOwnDevFltr);
    /* AUTODOC: Disable filtering local device number.*/
    prvTgfCscdDsaSrcDevFilterSet(GT_FALSE);

    prvTgfCscdOrigSrcPortFilterEnableGet(&orig_enableOrigSrcPortFltr);
    /* AUTODOC: Enable filtering original source port.*/
    prvTgfCscdOrigSrcPortFilterEnableSet(GT_TRUE);

    /* AUTODOC: set Network port 0 as trunk member */
    setPortAsTrunkMember(ING_NETWORK_PORT_IDX_CNS, prvTgfTrunkId, GT_TRUE);

    prvTgfPacketDsaTagPart_forward.origSrcPhyIsTrunk    = GT_TRUE;
    prvTgfPacketDsaTagPart_forward.origSrcPhy.trunkId   = prvTgfTrunkId;
    prvTgfPacketDsaTagPart_forward.phySrcMcFilterEnable = GT_TRUE;

    /* AUTODOC: Send  unregistered UC packet*/
    prvTgfCscdDsaFrwdSrcIsTrunkTestSendAndCheck(ING_CASCADE_PORT_IDX_CNS,
                                                EGR_NETWORK_PORT_IDX_CNS,
                                                &prvTgfCascadePacketInfo,
                                                GT_FALSE);

    prvTgfPacketDsaTagPart_forward.origSrcPhyIsTrunk    = GT_FALSE;
    prvTgfPacketDsaTagPart_forward.origSrcPhy.portNum   = prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS];

    /* AUTODOC: invalidate trunk entry */
    setPortAsTrunkMember(ING_NETWORK_PORT_IDX_CNS, prvTgfTrunkId, GT_FALSE);

    /* AUTODOC: Send unregistered UC packet*/
    prvTgfCscdDsaFrwdSrcIsTrunkTestSendAndCheck(ING_CASCADE_PORT_IDX_CNS,
                                                EGR_NETWORK_PORT_IDX_CNS,
                                                &prvTgfCascadePacketInfo,
                                                GT_FALSE);

    /* AUTODOC: set Network port 0 as trunk member */
    setPortAsTrunkMember(ING_NETWORK_PORT_IDX_CNS, prvTgfTrunkId, GT_TRUE);

    prvTgfPacketDsaTagPart_forward.origSrcPhyIsTrunk    = GT_TRUE;
    prvTgfPacketDsaTagPart_forward.origSrcPhy.trunkId   = prvTgfTrunkId;
    prvTgfPacketDsaTagPart_forward.phySrcMcFilterEnable = GT_FALSE;

    /* AUTODOC: Send  unregistered UC packet*/
    prvTgfCscdDsaFrwdSrcIsTrunkTestSendAndCheck(ING_CASCADE_PORT_IDX_CNS,
                                                EGR_NETWORK_PORT_IDX_CNS,
                                                &prvTgfCascadePacketInfo,
                                                GT_TRUE);

    prvTgfPacketDsaTagPart_forward.origSrcPhyIsTrunk    = GT_FALSE;
    prvTgfPacketDsaTagPart_forward.origSrcPhy.portNum   = prvTgfTrafficGeneratorPacketDsaTagGetOrigSrcPhy(
                                                            prvTgfDevNum,
                                                            prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS]);

    /* AUTODOC: invalidate trunk entry */
    setPortAsTrunkMember(ING_NETWORK_PORT_IDX_CNS, prvTgfTrunkId, GT_FALSE);

    /* AUTODOC: Send unregistered UC packet*/
    prvTgfCscdDsaFrwdSrcIsTrunkTestSendAndCheck(ING_CASCADE_PORT_IDX_CNS,
                                                EGR_NETWORK_PORT_IDX_CNS,
                                                &prvTgfCascadePacketInfo,
                                                GT_TRUE);

    prvTgfPacketDsaTagPart_forward.phySrcMcFilterEnable = GT_TRUE;

    prvTgfCscdDsaSrcDevFilterSet(orig_enableOwnDevFltr);

    prvTgfCscdOrigSrcPortFilterEnableSet(orig_enableOrigSrcPortFltr);

    /* AUTODOC: Restore common test configuration */
    prvTgfCscdDsaFrwdSrcIsTrunkTestConfigurationRestore();
}

