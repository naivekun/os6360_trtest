/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfBridgeDsaEgrFilterRegister.c
*
* DESCRIPTION:
*       Egress Filter Registered field in Forward eDsa tag test advanced UT.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>
#include <extUtils/trafficEngine/private/prvTgfTrafficParser.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfCscdGen.h>
#include <cscd/prvTgfCscdDsaEgrFilterRegister.h>

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

/* final target device & port */
#define PRV_TGF_DSA_SRC_DEV_CNS     8
#define PRV_TGF_DSA_PORT_CNS        58

/* max vlan id value. For random generation */
#define PRV_TGF_MAX_VLAN_ID         _4K

/* max target device value. For random generation*/
#define PRV_TGF_MAX_TRG_DEV         _1K

/* target device field. Will be initialized later */
static GT_HW_DEV_NUM prvTgfTrgDev;

/* vlan used by test. Will be initialized later */
static TGF_VLAN_ID prvTgfVlanId;

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 1;

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
    16,/*srcHwDev*/
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
            62/*portNum*/
        },/*devPort*/

        0,/*trunkId*/
        0, /*vidx*/
        0,/*vlanId*/
        0,/*devNum*/
        0,/*fabricVidx*/
        0 /*index*/
    },/*dstInterface*/
    GT_FALSE,/*isTrgPortValid*/
    PRV_TGF_DSA_PORT_CNS,/*dstEport*/
    0,/*TPID Index*/
    GT_FALSE,/*srcIsPe*/
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
static TGF_PACKET_PART_STC prvTgfCascasePacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_DSA_TAG_E,   &prvTgfPacketDsaTagPart},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTag1Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_CASCADE_PACKET_LEN_CNS \
 (TGF_L2_HEADER_SIZE_CNS + TGF_DSA_TAG_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS \
                                                + sizeof(prvTgfPayloadDataArr))

/* LENGTH of packet with CRC */
#define PRV_TGF_CASCADE_PACKET_CRC_LEN_CNS  (PRV_TGF_CASCADE_PACKET_LEN_CNS \
                                                          + TGF_CRC_LEN_CNS)

/* PACKET to send info */
static TGF_PACKET_STC prvTgfCascadePacketInfo = {
    PRV_TGF_CASCADE_PACKET_CRC_LEN_CNS,                                 /* totalLen */
    sizeof(prvTgfCascasePacketPartArray) / sizeof(TGF_PACKET_PART_STC),/* numOfParts */
    prvTgfCascasePacketPartArray                                       /* partsArray */
};

/* the expected Tag part from the tag0 egress port */
static TGF_VFD_INFO_STC     tgfUnRegisteredDsaTagVfd[1] =
{
      {TGF_VFD_MODE_STATIC_E,0,0, 12 /*offset*/, {0xF0, 0x00, 0x10, 0x00,
                                                  0x80, 0x00, 0x1F, 0xFF,
                                                  0x80, 0x00, 0x00, 0x00,
                                                  0x06, 0x00, 0x00, 0x40},

                                                 {0x00, 0x00, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x00},
       16/*cycleCount*/ , NULL,0,0}
};

/* the expected Tag part from the tag0 egress port */
static TGF_VFD_INFO_STC     tgfRegisteredDsaTagVfd[1] =
{
      {TGF_VFD_MODE_STATIC_E,0,0, 12 /*offset*/, {0xF0, 0x00, 0x10, 0x00,
                                                  0x90, 0x00, 0x07, 0xC0,
                                                  0x80, 0x00, 0x00, 0x00,
                                                  0x06, 0x00, 0x1D, 0x00},

                                                 {0x00, 0x00, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x00},
       16/*cycleCount*/ , NULL,0,0}
};

/* the expected Tag part from the tag0 egress port */
static TGF_VFD_INFO_STC     tgfVlanTagVfd[1] =
{
      {TGF_VFD_MODE_STATIC_E,0,0, 12 /*offset*/, {0x81, 0x00, 0x00, 0x00,
                                                  0x81, 0x00, 0x00, 0x06},

                                                 {0x00, 0x00, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x00},
       8/*cycleCount*/ , NULL,0,0}
};

/******************************************************************************\
 *                   Private variables & definitions                          *
\******************************************************************************/

/* Parameters needed to be restored */
static GT_U32                            portsArrayForRestore[PRV_TGF_MAX_PORTS_NUM_CNS];
static CPSS_CSCD_LINK_TYPE_STC           cascadeLinkForRestore;
static PRV_TGF_CSCD_TRUNK_LINK_HASH_ENT  srcPortTrunkHashEnForRestore;
static GT_BOOL                           ucFilteringForRestore;
static GT_HW_DEV_NUM                     hwDevNumForRestore;
static CPSS_INTERFACE_INFO_STC           savePhyInfo;

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

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
* @internal prvTgfVlanEDsaTagRedirectMapEport2PhySet function
* @endinternal
*
* @brief   Maps ePort to physical interface
*
* @param[in] devNum                   - devNum
* @param[in] portNum                  -  ePort
* @param[in] phyInfoPtr               -  (pointer to) inteface to map
* @param[in] savePhyInfoPtr           -  (pointer to) save inteface
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfVlanEDsaTagRedirectMapEport2PhySet
(
    IN  GT_U8                            devNum,
    IN  GT_PORT_NUM                      portNum,
    IN  CPSS_INTERFACE_INFO_STC          *phyInfoPtr,
    OUT CPSS_INTERFACE_INFO_STC          *savePhyInfoPtr
)
{
    GT_STATUS rc;

    if (savePhyInfoPtr != NULL)
    {
        rc = prvTgfBrgEportToPhysicalPortTargetMappingTableGet(
            devNum, portNum, savePhyInfoPtr);
        PRV_UTF_VERIFY_GT_OK(
            rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableGet");
    }

    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(
        devNum, portNum, phyInfoPtr);
    PRV_UTF_VERIFY_GT_OK(
        rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableSet");

    return GT_OK;
}

/**
* @internal prvTgfCscdDsaEgrFilterRegisterTestPacketSend function
* @endinternal
*
* @brief   Function sends packet.
*
* @param[in] portInterfacePtr         - (pointer to) port interface
* @param[in] packetInfoPtr            - (pointer to) the packet info
* @param[in] ingressPortIndex         - ingress port index
*                                       None
*/
static GT_VOID prvTgfCscdDsaEgrFilterRegisterTestPacketSend
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
* @internal prvTgfCscdDsaEgrFilterRegisterTestSendAndCheck function
* @endinternal
*
* @brief   Function sends packet and check results
*
* @param[in] ingressPortIndex         - ingress port index
* @param[in] egressPortIndex          - egress port index
* @param[in] packetInfoPtr            - (pointer to) send packet
*                                      vfdArray          - VFD array with expected results
*                                       None
*/
static GT_VOID prvTgfCscdDsaEgrFilterRegisterTestSendAndCheck
(
    IN GT_U32                   ingressPortIndex,
    IN GT_U32                   egressPortIndex,
    IN TGF_PACKET_STC           *packetInfoPtr,
    IN TGF_VFD_INFO_STC         *vfdArrayPtr
)
{
    GT_STATUS                       rc;
    CPSS_INTERFACE_INFO_STC         portInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    GT_U32                          numTriggers = 0;
    GT_U32                          vfdNum = 1;

    /* AUTODOC: set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[egressPortIndex];

    /* bind the DSA tag FORWARD part */
    prvTgfPacketDsaTagPart.dsaInfo.forward = prvTgfPacketDsaTagPart_forward;

    /* AUTODOC: send packet. */
    prvTgfCscdDsaEgrFilterRegisterTestPacketSend(&portInterface, packetInfoPtr, ingressPortIndex);

    /* AUTODOC: check counters */

    /* read Tx counter */
    rc = prvTgfReadPortCountersEth(prvTgfDevNum,
                                   prvTgfPortsArray[egressPortIndex],
                                   GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfReadPortCountersEth: %d, %d, %d",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[egressPortIndex]);
    if (vfdArrayPtr == NULL)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                 "get another goodPktsSent counter than expected on port %d, %d",
                                     prvTgfPortsArray[egressPortIndex]);
    }
    else
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                 "get another goodPktsSent counter than expected on port %d, %d",
                                     prvTgfPortsArray[egressPortIndex]);

        /* get trigger counters */
        rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, vfdNum,
                                                         vfdArrayPtr, &numTriggers);
        PRV_UTF_LOG2_MAC("numTriggers = %d, rc = 0x%02X\n\n",(1 << vfdNum) - 1, rc);

        UTF_VERIFY_EQUAL2_STRING_MAC((1 << vfdNum) - 1, numTriggers,
            "get another trigger that expected: expected - %d, recieved - %d\n",
                                     (1 << vfdNum) - 1, numTriggers);
    }

}

/**
* @internal prvTgfCscdDsaEgrFilterRegisterTestConfigurationSet function
* @endinternal
*
* @brief   Test initial configurations
*/
GT_VOID prvTgfCscdDsaEgrFilterRegisterTestConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                        rc;
    CPSS_CSCD_LINK_TYPE_STC          cascadeLink;
    PRV_TGF_CSCD_TRUNK_LINK_HASH_ENT srcPortTrunkHashEn;
    GT_HW_DEV_NUM                    hwDevNum;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* initialize randomizer */
    cpssOsSrand(prvUtfSeedFromStreamNameGet());

    /* AUTODOC: save prvTgfPortsArray */
    cpssOsMemCpy(portsArrayForRestore, prvTgfPortsArray,
                 sizeof(portsArrayForRestore));

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

    /* get HW device number */
    rc = prvUtfHwDeviceNumberGet(prvTgfDevNum, &hwDevNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvUtfHwDeviceNumberGet: %d, %d",
                                 prvTgfDevNum, hwDevNum);
    /* set srcHwDevNum in expected DSA tag */
    PRV_TGF_SET_HW_DEV_NUM_DSA_TAG_MAC(tgfUnRegisteredDsaTagVfd[0].patternPtr[0], hwDevNum);
    PRV_TGF_SET_HW_DEV_NUM_DSA_TAG_MAC(tgfRegisteredDsaTagVfd[0].patternPtr[0], hwDevNum);

    /* AUTODOC: initialize vlan and target device by random values */
    prvTgfVlanId  = cpssOsRand() % (PRV_TGF_MAX_VLAN_ID - 2) + 2;
    do
    {
        prvTgfTrgDev = cpssOsRand() % (PRV_TGF_MAX_TRG_DEV - 1) + 1;
    }
    while ((prvTgfTrgDev == PRV_TGF_DSA_SRC_DEV_CNS)|| ((prvTgfTrgDev == hwDevNum)));


    PRV_UTF_LOG2_MAC("We will use vlanId = %d, trgDev = %d ", prvTgfVlanId, prvTgfTrgDev);

    prvTgfPacketDsaTagPart_forward.srcHwDev   = hwDevNum;
    prvTgfPacketVlanTag0Part.vid              = prvTgfVlanId;
    prvTgfPacketDsaTagPart.commonParams.vid   = prvTgfVlanId;
    prvTgfPacketDsaTagPart_forward.dstInterface.vlanId = prvTgfVlanId;

    prvTgfPacketDsaTagPart_forward.dstInterface.devPort.hwDevNum = prvTgfTrgDev;
    prvTgfPacketDsaTagPart_forward.dstInterface.hwDevNum = prvTgfTrgDev;

    tgfUnRegisteredDsaTagVfd[0].patternPtr[2] |= (GT_U8)(prvTgfVlanId >> 8);
    tgfUnRegisteredDsaTagVfd[0].patternPtr[3] = (GT_U8)prvTgfVlanId;
    tgfRegisteredDsaTagVfd[0].patternPtr[2] |= (GT_U8)(prvTgfVlanId >> 8);
    tgfRegisteredDsaTagVfd[0].patternPtr[3] = (GT_U8)prvTgfVlanId;
    tgfVlanTagVfd[0].patternPtr[2] |= (GT_U8)(prvTgfVlanId >> 8);
    tgfVlanTagVfd[0].patternPtr[3] = (GT_U8)prvTgfVlanId;

    tgfRegisteredDsaTagVfd[0].patternPtr[7] |= (GT_U8)(prvTgfTrgDev & 0x1F);
    tgfRegisteredDsaTagVfd[0].patternPtr[15] = (GT_U8)(prvTgfTrgDev>>5);

    /* update tgfUnRegisteredDsaTagVfd */
    {
        GT_U8*  dsaBytesPtr = tgfUnRegisteredDsaTagVfd[0].patternPtr;
        TGF_PACKET_DSA_TAG_STC tmpDsa;

        rc = prvTgfTrafficGeneratorPacketDsaTagParse(dsaBytesPtr,
            &tmpDsa);
        UTF_VERIFY_EQUAL1_STRING_MAC( GT_OK, rc, "prvTgfTrafficGeneratorPacketDsaTagParse: %d",
                                  prvTgfDevNum);

        /* update the DSA tag format */
        tmpDsa.dsaInfo.forward.source.portNum = prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS];
        tmpDsa.dsaInfo.forward.origSrcPhy.portNum = prvTgfTrafficGeneratorPacketDsaTagGetOrigSrcPhy(
                                                        prvTgfDevNum,
                                                        tmpDsa.dsaInfo.forward.source.portNum);
        /* SIP6 : the <skipFdbSaLookup> in the eDSA word[2] bit [30] got value 1 , used to be 'reserved' in legacy devices*/ 
        if (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
        {
            tmpDsa.dsaInfo.forward.skipFdbSaLookup = GT_TRUE;
        }
        else
        {
            tmpDsa.dsaInfo.forward.skipFdbSaLookup = GT_FALSE;
        }
        /* convert the DSA into into bytes back */
        rc = prvTgfTrafficGeneratorPacketDsaTagBuild(&tmpDsa,dsaBytesPtr);
        UTF_VERIFY_EQUAL1_STRING_MAC( GT_OK, rc, "prvTgfTrafficGeneratorPacketDsaTagBuild: %d",
                                  prvTgfDevNum);
    }
    /* update tgfRegisteredDsaTagVfd */
    {
        GT_U8*  dsaBytesPtr = tgfRegisteredDsaTagVfd[0].patternPtr;
        TGF_PACKET_DSA_TAG_STC tmpDsa;

        rc = prvTgfTrafficGeneratorPacketDsaTagParse(dsaBytesPtr,
            &tmpDsa);
        UTF_VERIFY_EQUAL1_STRING_MAC( GT_OK, rc, "prvTgfTrafficGeneratorPacketDsaTagParse: %d",
                                  prvTgfDevNum);

        /* update the DSA tag format */
        tmpDsa.dsaInfo.forward.source.portNum = prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS];
        tmpDsa.dsaInfo.forward.origSrcPhy.portNum = prvTgfTrafficGeneratorPacketDsaTagGetOrigSrcPhy(
                                                        prvTgfDevNum,
                                                        tmpDsa.dsaInfo.forward.source.portNum);

        /* SIP6 : the <skipFdbSaLookup> in the eDSA word[2] bit [30] got value 1 , used to be 'reserved' in legacy devices*/ 
        if (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
        {
            tmpDsa.dsaInfo.forward.skipFdbSaLookup = GT_TRUE;
        }
        else
        {
            tmpDsa.dsaInfo.forward.skipFdbSaLookup = GT_FALSE;
        }
        /* convert the DSA into into bytes back */
        rc = prvTgfTrafficGeneratorPacketDsaTagBuild(&tmpDsa,dsaBytesPtr);
        UTF_VERIFY_EQUAL1_STRING_MAC( GT_OK, rc, "prvTgfTrafficGeneratorPacketDsaTagBuild: %d",
                                  prvTgfDevNum);
    }

    /* update prvTgfPacketDsaTagPart_forward */
    {
        prvTgfPacketDsaTagPart_forward.source.portNum = prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS];
        prvTgfPacketDsaTagPart_forward.origSrcPhy.portNum = prvTgfTrafficGeneratorPacketDsaTagGetOrigSrcPhy(
                                                                prvTgfDevNum,
                                                                prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS]);
    }


    /* AUTODOC: create VLAN with all tagged port */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd( prvTgfVlanId,
                                                   PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);
    UTF_VERIFY_EQUAL1_STRING_MAC( GT_OK, rc, "prvTgfBrgDefVlanEntryWriteWithTaggingCmd: %d",
                                  prvTgfDevNum);

    setIngressPortCascade( prvTgfPortsArray[ING_CASCADE_PORT_IDX_CNS], GT_TRUE );
    setEgressPortCascade(  prvTgfPortsArray[EGR_CASCADE_PORT_IDX_CNS], GT_TRUE );

    cascadeLink.linkNum = prvTgfPortsArray[EGR_CASCADE_PORT_IDX_CNS];
    cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;
    srcPortTrunkHashEn = PRV_TGF_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E;

    /* save the current cascade map table */
    rc = prvTgfCscdDevMapTableGet(prvTgfDevNum, prvTgfTrgDev,
                                  PRV_TGF_DSA_PORT_CNS, &cascadeLinkForRestore,
                                  &srcPortTrunkHashEnForRestore);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableGet: %d,%d,%d",
                                 prvTgfDevNum,  prvTgfTrgDev, PRV_TGF_DSA_PORT_CNS);

    /* AUTODOC: set cascade mapping for remote device\port */
    rc = prvTgfCscdDevMapTableSet(prvTgfDevNum, prvTgfTrgDev,
                                  PRV_TGF_DSA_PORT_CNS, &cascadeLink,
                                  srcPortTrunkHashEn, GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,"prvTgfCscdDevMapTableSet: %d,%d,%d",
                                 prvTgfDevNum, prvTgfTrgDev, PRV_TGF_DSA_PORT_CNS);

    /* save current value */
    prvTgfBrgEgressPortUnknownUcFilteringGet(prvTgfDevNum,
                                             prvTgfPortsArray[EGR_NETWORK_PORT_IDX_CNS],
                                             &ucFilteringForRestore);

    /* AUTODOC: set the status of bridged Unknown UC filtering */
    prvTgfBrgEgressPortUnknownUcFilteringSet(prvTgfPortsArray[EGR_NETWORK_PORT_IDX_CNS],
                                             GT_TRUE);

    /* AUTODOC: save default devnum for restore */
    prvUtfHwDeviceNumberGet(prvTgfDevNum, &hwDevNumForRestore);


}

/**
* @internal prvTgfCscdDsaEgrFilterRegisterTestConfigurationRestore function
* @endinternal
*
* @brief   Test restore configurations
*/
GT_VOID prvTgfCscdDsaEgrFilterRegisterTestConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS                   rc;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: restore default devnum */
    prvUtfHwDeviceNumberSet(prvTgfDevNum, hwDevNumForRestore);

    if(prvUtfIsDoublePhysicalPortsModeUsed())
    {
        /* since the 'FROM_CPU' will not use 'isPhysicalPortValid' but use the eport
           we need to update the 'E2PHY' to redirect to 'local device' (hwDevNumForRestore)
           instead of 'previous' prvTgfTrgDev */
        prvUtfSetE2PhyEqualValue(ING_CASCADE_PORT_IDX_CNS);
    }

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d", GT_TRUE);

    /* AUTODOC: set the status of bridged Unknown UC filtering */
    prvTgfBrgEgressPortUnknownUcFilteringSet(prvTgfPortsArray[EGR_NETWORK_PORT_IDX_CNS],
                                             ucFilteringForRestore);

    /* AUTODOC: restore cascade mapping */
    rc = prvTgfCscdDevMapTableSet(prvTgfDevNum, prvTgfTrgDev,
                                  PRV_TGF_DSA_PORT_CNS, &cascadeLinkForRestore,
                                  srcPortTrunkHashEnForRestore, GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,"prvTgfCscdDevMapTableSet: %d,%d,%d",
                                 prvTgfDevNum, prvTgfTrgDev, PRV_TGF_DSA_PORT_CNS);

    setIngressPortCascade( prvTgfPortsArray[ING_CASCADE_PORT_IDX_CNS], GT_FALSE );
    setEgressPortCascade(  prvTgfPortsArray[EGR_CASCADE_PORT_IDX_CNS], GT_FALSE );

    /* AUTODOC: invalidate vlans entry (and reset vlans entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(prvTgfVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, prvTgfVlanId);

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: restore prvTgfPortsArray */
    cpssOsMemCpy(prvTgfPortsArray, portsArrayForRestore, sizeof(portsArrayForRestore));
}

/**
* @internal prvTgfCscdDsaEgrFilterRegisterTest function
* @endinternal
*
* @brief   Test for Egress Filter Registered field in Forward eDsa tag
*/
GT_VOID prvTgfCscdDsaEgrFilterRegisterTest
(
    GT_VOID
)
{
    GT_STATUS                   rc;
    CPSS_INTERFACE_INFO_STC     physicalInfo;

    cpssOsMemSet(&physicalInfo, 0, sizeof(physicalInfo));

    /* AUTODOC: Set common test configuration */
    prvTgfCscdDsaEgrFilterRegisterTestConfigurationSet();

    /* AUTODOC: CASE 1: from Network port to Cascade port */
        PRV_UTF_LOG2_MAC(
            "\n*** CASE 1: from Network port %02d to Cascade port %02d ***\n",
            prvTgfPortsArray[ING_NETWORK_PORT_IDX_CNS],
            prvTgfPortsArray[EGR_CASCADE_PORT_IDX_CNS]);

    /* AUTODOC: Send  unregistered UC packet*/
    prvTgfCscdDsaEgrFilterRegisterTestSendAndCheck(ING_NETWORK_PORT_IDX_CNS,
                                                  EGR_CASCADE_PORT_IDX_CNS,
                                                  &prvTgfPacketInfo,
                                                  &tgfUnRegisteredDsaTagVfd[0]);

    /* AUTODOC: add FDB entry with daMac, vlanId, ingress port */
    prvTgfBrgDefFdbMacEntryOnPortSet( prvTgfPacketL2Part.daMac,
        prvTgfVlanId, prvTgfTrgDev, PRV_TGF_DSA_PORT_CNS, GT_TRUE);

    /* AUTODOC: Send  registered UC packet*/
    prvTgfCscdDsaEgrFilterRegisterTestSendAndCheck(ING_NETWORK_PORT_IDX_CNS,
                                                  EGR_CASCADE_PORT_IDX_CNS,
                                                  &prvTgfPacketInfo,
                                                  &tgfRegisteredDsaTagVfd[0]);

    /* AUTODOC: CASE 2: from Cascade port to Cascade port */
        PRV_UTF_LOG2_MAC(
            "\n*** CASE 2: from Cascade port %02d to Cascade port %02d ***\n",
            prvTgfPortsArray[ING_CASCADE_PORT_IDX_CNS],
            prvTgfPortsArray[EGR_CASCADE_PORT_IDX_CNS]);

    /* AUTODOC: set hwDevNum = source devnum */
    rc = prvUtfHwDeviceNumberSet(prvTgfDevNum, PRV_TGF_DSA_SRC_DEV_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvUtfHwDeviceNumberSet \n");

    /* In HIR mode, SW DevNum and Hw DevNum are same. For this test
       SW devNum and HW DevNum should be different.Below 3 lines is to
       set srcHwDev in eDSA tag to be different HwDevNum */
    prvTgfPacketDsaTagPart_forward.srcHwDev = 0x10;
    /* set srcHwDevNum in expected DSA tag */
    PRV_TGF_SET_HW_DEV_NUM_DSA_TAG_MAC(tgfUnRegisteredDsaTagVfd[0].patternPtr[0], 0x10);
    PRV_TGF_SET_HW_DEV_NUM_DSA_TAG_MAC(tgfRegisteredDsaTagVfd[0].patternPtr[0], 0x10);

    if(prvUtfIsDoublePhysicalPortsModeUsed())
    {
        /* since the 'FROM_CPU' will not use 'isPhysicalPortValid' but use the eport
           we need to update the 'E2PHY' to redirect to 'local device' (0x8)
           instead of 'default' 0x10 */
        prvUtfSetE2PhyEqualValue(ING_CASCADE_PORT_IDX_CNS);
    }


    /* AUTODOC: Send  unregistered UC packet*/
    prvTgfCscdDsaEgrFilterRegisterTestSendAndCheck(ING_CASCADE_PORT_IDX_CNS,
                                                  EGR_CASCADE_PORT_IDX_CNS,
                                                  &prvTgfCascadePacketInfo,
                                                  &tgfUnRegisteredDsaTagVfd[0]);

    prvTgfPacketDsaTagPart_forward.egrFilterRegistered = GT_TRUE;
    prvTgfPacketDsaTagPart_forward.dstInterface.type = CPSS_INTERFACE_PORT_E;

    /* AUTODOC: Send  registered UC packet*/
    prvTgfCscdDsaEgrFilterRegisterTestSendAndCheck(ING_CASCADE_PORT_IDX_CNS,
                                                  EGR_CASCADE_PORT_IDX_CNS,
                                                  &prvTgfCascadePacketInfo,
                                                  &tgfRegisteredDsaTagVfd[0]);

    prvTgfPacketDsaTagPart_forward.egrFilterRegistered = GT_FALSE;
    prvTgfPacketDsaTagPart_forward.dstInterface.type = CPSS_INTERFACE_VID_E;

    /* AUTODOC: CASE 3: from Cascade port to Network port */
        PRV_UTF_LOG2_MAC(
            "\n*** CASE 3: from Cascade port %02d to Network port %02d ***\n",
            prvTgfPortsArray[ING_CASCADE_PORT_IDX_CNS],
            prvTgfPortsArray[EGR_NETWORK_PORT_IDX_CNS]);

    /* AUTODOC: set hwDevNum = destination devnum */
    rc = prvUtfHwDeviceNumberSet(prvTgfDevNum, prvTgfTrgDev);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvUtfHwDeviceNumberSet \n");

    if(prvUtfIsDoublePhysicalPortsModeUsed())
    {
        /* since the 'FROM_CPU' will not use 'isPhysicalPortValid' but use the eport
           we need to update the 'E2PHY' to redirect to 'local device' (prvTgfTrgDev)
           instead of 'previous' 0x8 */
        prvUtfSetE2PhyEqualValue(ING_CASCADE_PORT_IDX_CNS);
    }

    physicalInfo.type = CPSS_INTERFACE_PORT_E;
    physicalInfo.devPort.hwDevNum = prvTgfDevNum;
    physicalInfo.devPort.portNum = prvTgfPortsArray[EGR_NETWORK_PORT_IDX_CNS];

    /* AUTODOC: map ePort to physical port 3 */
    prvTgfVlanEDsaTagRedirectMapEport2PhySet( prvTgfDevNum,PRV_TGF_DSA_PORT_CNS,
                                           &physicalInfo, &savePhyInfo);

    /* AUTODOC: Send  unregistered UC packet*/
    prvTgfCscdDsaEgrFilterRegisterTestSendAndCheck(ING_CASCADE_PORT_IDX_CNS,
                                                  EGR_NETWORK_PORT_IDX_CNS,
                                                  &prvTgfCascadePacketInfo,
                                                  NULL);

    prvTgfPacketDsaTagPart_forward.egrFilterRegistered = GT_TRUE;
    prvTgfPacketDsaTagPart_forward.dstInterface.type = CPSS_INTERFACE_PORT_E;

    /* AUTODOC: Send  registered UC packet*/
    prvTgfCscdDsaEgrFilterRegisterTestSendAndCheck(ING_CASCADE_PORT_IDX_CNS,
                                                  EGR_NETWORK_PORT_IDX_CNS,
                                                  &prvTgfCascadePacketInfo,
                                                  &tgfVlanTagVfd[0]);

    prvTgfPacketDsaTagPart_forward.egrFilterRegistered = GT_FALSE;
    prvTgfPacketDsaTagPart_forward.dstInterface.type = CPSS_INTERFACE_VID_E;

    /* AUTODOC: restore map ePort to physical port */
    prvTgfVlanEDsaTagRedirectMapEport2PhySet( prvTgfDevNum,PRV_TGF_DSA_PORT_CNS,
                                           &savePhyInfo, NULL);

    /* AUTODOC: Restore common test configuration */
    prvTgfCscdDsaEgrFilterRegisterTestConfigurationRestore();
}

