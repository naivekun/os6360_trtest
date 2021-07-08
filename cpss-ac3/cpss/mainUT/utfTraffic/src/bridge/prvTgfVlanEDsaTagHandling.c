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
* @file prvTgfVlanEDsaTagHandling.c
*
* @brief Test for eDSA tag handling with double tag.
*
* @version   5
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/pcl/cpssPcl.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfTcamGen.h>
#include <common/tgfCosGen.h>
#include <common/tgfCscdGen.h>

#include <bridge/prvTgfVlanEDsaTagHandling.h>
#include <extUtils/trafficEngine/private/prvTgfTrafficParser.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN Id */
#define PRV_TGF_VLANID0_CNS          1
#define PRV_TGF_VLANID1_CNS          100

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 1;

/******************************* Test packets *********************************/

static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x17},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x08}                 /* saMac */
};

/* VLAN_TAG1 part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag1Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID1_CNS                           /* pri, cfi, VlanId */
};

/********************** DSA tag  ***************************/
static TGF_DSA_DSA_FORWARD_STC  prvTgfPacketDsaTagPart_forward = {
    PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E , /*srcIsTagged*/
    3,/*srcHwDev*/
    GT_FALSE,/* srcIsTrunk */
    /*union*/
    {
        /*trunkId*/
        10/*portNum*/  /* Set in runtime to avoid BE init problem for the union */
    },/*source;*/

    0,/*srcId*/

    GT_FALSE,/*egrFilterRegistered*/
    GT_FALSE,/*wasRouted*/
    0,/*qosProfileIndex*/

    /*CPSS_INTERFACE_INFO_STC         dstInterface*/
    {
        CPSS_INTERFACE_PORT_E,/*type*/

        /*struct*/{
            0,/*devNum*/
            1/*portNum*/
        },/*devPort*/

        0,/*trunkId*/
        0, /*vidx*/
        1,/*vlanId*/
        16,/*devNum*/
        0,/*fabricVidx*/
        0 /*index*/
    },/*dstInterface*/
    GT_FALSE,/*isTrgPortValid*/
    1,/*dstEport*/
    0,/*TPID Index*/
    GT_FALSE,/*origSrcPhyIsTrunk*/
    /*union*/
    {
        /*trunkId*/
        0/*portNum*/ /* Set in runtime to avoid BE init problem for the union */
    },/*origSrcPhy;*/
    GT_FALSE,/*phySrcMcFilterEnable*/
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
        PRV_TGF_VLANID0_CNS,/*vid*/
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
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_DSA_TAG_E,   &prvTgfPacketDsaTagPart},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTag1Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
 (TGF_L2_HEADER_SIZE_CNS + TGF_DSA_TAG_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS \
                                                + sizeof(prvTgfPayloadDataArr))

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS  (PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS)

/* PACKET to send info */
static TGF_PACKET_STC prvTgfPacketInfo = {
    PRV_TGF_PACKET_CRC_LEN_CNS,                                 /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC),/* numOfParts */
    prvTgfPacketPartArray                                       /* partsArray */
};

/* the expected Tag part from the untagged egress port */
static TGF_VFD_INFO_STC     tgfVlanUntaggedVfd[1] =
{
     {TGF_VFD_MODE_STATIC_E,0,0, 12 /*offset*/, {0xE3, 0x50, 0x10, 0x01,
                                                 0x80, 0x00, 0x02, 0x50,
                                                 0xA0, 0x00, 0x00, 0x00,
                                                 0x06, 0x00, 0x00, 0x80},

                                                {0x00, 0x00, 0x00, 0x00,
                                                 0x00, 0x00, 0x00, 0x00,
                                                 0x00, 0x00, 0x00, 0x00,
                                                 0x00, 0x00, 0x00, 0x00},
       16/*cycleCount*/ , NULL,0,0}
};

/* the expected Tag part from the tag0 egress port */
static TGF_VFD_INFO_STC     tgfVlanTag0Vfd[1] =
{
      {TGF_VFD_MODE_STATIC_E,0,0, 12 /*offset*/, {0xE3, 0x50, 0x10, 0x01,
                                                  0x80, 0x00, 0x02, 0x50,
                                                  0xA0, 0x00, 0x00, 0x00,
                                                  0x06, 0x00, 0x00, 0x80},

                                                 {0x00, 0x00, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x00},
       16/*cycleCount*/ , NULL,0,0}
};

/* the expected Tag part from the tag1 egress port */
static TGF_VFD_INFO_STC     tgfVlanTag1Vfd[1] =
{
     {TGF_VFD_MODE_STATIC_E,0,0, 12 /*offset*/, {0xE3, 0x50, 0x10, 0x64,
                                                 0x80, 0x00, 0x02, 0x50,
                                                 0xA0, 0x00, 0x00, 0x00,
                                                 0x06, 0x00, 0x00, 0x80},

                                                {0x00, 0x00, 0x00, 0x00,
                                                 0x00, 0x00, 0x00, 0x00,
                                                 0x00, 0x00, 0x00, 0x00,
                                                 0x00, 0x00, 0x00, 0x00},
      16/*cycleCount*/ , NULL,0,0}
};
/* the expected Tag part from the outer tag0 inner tag1 egress port */
static TGF_VFD_INFO_STC     tgfVlanOuterTag0InnerTag1Vfd[2] =
{
     {TGF_VFD_MODE_STATIC_E,0,0, 12 /*offset*/, {0xE3, 0x50, 0x10, 0x01,
                                                 0x80, 0x00, 0x02, 0x50,
                                                 0xA0, 0x00, 0x00, 0x00,
                                                 0x06, 0x00, 0x00, 0x80},

                                                {0x00, 0x00, 0x00, 0x00,
                                                 0x00, 0x00, 0x00, 0x00,
                                                 0x00, 0x00, 0x00, 0x00,
                                                 0x00, 0x00, 0x00, 0x00},
      16/*cycleCount*/ , NULL,0,0},
    {TGF_VFD_MODE_STATIC_E,0,0, 12 + 16 /*offset*/, {0x81,0x00,0x00,0x64},
                                                    {0, 0, 0, 0},
       4/*cycleCount*/ , NULL,0,0}
};

/* the expected Tag part from the outer tag1 inner tag0 egress port */
static TGF_VFD_INFO_STC     tgfVlanOuterTag1InnerTag0Vfd[2] =
{
     {TGF_VFD_MODE_STATIC_E,0,0, 12 /*offset*/, {0xE3, 0x50, 0x10, 0x64,
                                                 0x80, 0x00, 0x02, 0x50,
                                                 0xA0, 0x00, 0x00, 0x00,
                                                 0x06, 0x00, 0x00, 0x80},

                                                {0x00, 0x00, 0x00, 0x00,
                                                 0x00, 0x00, 0x00, 0x00,
                                                 0x00, 0x00, 0x00, 0x00,
                                                 0x00, 0x00, 0x00, 0x00},
     16/*cycleCount*/ , NULL,0,0},
     {TGF_VFD_MODE_STATIC_E,0,0, 12 + 16 /*offset*/, {0x81,0x00,0x00,0x01},
                                                     {0, 0, 0, 0},
      4/*cycleCount*/ , NULL,0,0}
};

/******************************************************************************\
 *                   Private variables & definitions                          *
\******************************************************************************/

/* traffic generation sending port */
#define ING_PORT_IDX_CNS    0

/* target port */
#define EGR_PORT_IDX_CNS    1

/* final target device & port */
#define PRV_TGF_DSA_DEV_CNS     16
#define PRV_TGF_DSA_PORT_CNS    0x01

/* Parameters needed to be restored */

/* parametrs for restore default Vlan entry */
static struct
{
    CPSS_PORTS_BMP_STC                  portsMembers;
    CPSS_PORTS_BMP_STC                  portsTagging;
    GT_BOOL                             isValid;
    PRV_TGF_BRG_VLAN_INFO_STC           vlanInfo;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC  taggingCmd;
} prvTgfVlanRestoreCfg;

static CPSS_INTERFACE_INFO_STC                            savePhyInfo;
static CPSS_CSCD_LINK_TYPE_STC                            cascadeLinkForRestore;
static PRV_TGF_CSCD_TRUNK_LINK_HASH_ENT                   srcPortTrunkHashEnForRestore;
static PRV_TGF_PCL_EGRESS_TARGET_PORT_SELECTION_MODE_ENT  portSelectionModeForRestore;

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfVlanEDsaTagTestVlanInit function
* @endinternal
*
* @brief   Set VLAN entry.
*
* @param[in] vlanId                   -  to be configured
* @param[in] tagCmd                   - tag modification command
*                                       None
*/
static GT_VOID prvTgfVlanEDsaTagTestVlanInit
(
    IN GT_U16                            vlanId,
    IN PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT tagCmd
)
{
    GT_STATUS                   rc        = GT_OK;
    CPSS_PORTS_BMP_STC          portsMembers;
    CPSS_PORTS_BMP_STC          portsTagging;
    PRV_TGF_BRG_VLAN_INFO_STC   vlanInfo;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;

    /* clear entry */
    cpssOsMemSet(&vlanInfo, 0, sizeof(vlanInfo));
    cpssOsMemSet(&portsTaggingCmd, 0,
                 sizeof(PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC));

    /* set vlan entry */
    vlanInfo.unkSrcAddrSecBreach  = GT_FALSE;
    vlanInfo.unregNonIpMcastCmd   = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregIpv4McastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregIpv6McastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unkUcastCmd          = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregIpv4BcastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregNonIpv4BcastCmd = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.ipv4IgmpToCpuEn      = GT_FALSE;
    vlanInfo.mirrToRxAnalyzerEn   = GT_FALSE;
    vlanInfo.ipv6IcmpToCpuEn      = GT_FALSE;
    vlanInfo.ipCtrlToCpuEn        = PRV_TGF_BRG_IP_CTRL_NONE_E;
    vlanInfo.ipv4IpmBrgMode       = CPSS_BRG_IPM_SGV_E;
    vlanInfo.ipv6IpmBrgMode       = CPSS_BRG_IPM_SGV_E;
    vlanInfo.ipv4IpmBrgEn         = GT_FALSE;
    vlanInfo.ipv6IpmBrgEn         = GT_FALSE;
    vlanInfo.ipv6SiteIdMode       = CPSS_IP_SITE_ID_INTERNAL_E;
    vlanInfo.ipv4UcastRouteEn     = GT_FALSE;
    vlanInfo.ipv4McastRouteEn     = GT_FALSE;
    vlanInfo.ipv6UcastRouteEn     = GT_FALSE;
    vlanInfo.ipv6McastRouteEn     = GT_FALSE;
    vlanInfo.stgId                = 0;
    vlanInfo.autoLearnDisable     = GT_TRUE;/* working in controlled learning */
    vlanInfo.naMsgToCpuEn         = GT_TRUE;/* working in controlled learning */
    vlanInfo.mruIdx               = 0;
    vlanInfo.bcastUdpTrapMirrEn   = GT_FALSE;
    vlanInfo.floodVidx            = 0xFFF;
    vlanInfo.floodVidxMode        =
                         PRV_TGF_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E;
    vlanInfo.portIsolationMode    =
                                  PRV_TGF_BRG_VLAN_PORT_ISOLATION_DISABLE_CMD_E;
    vlanInfo.ucastLocalSwitchingEn = GT_FALSE;
    vlanInfo.mcastLocalSwitchingEn = GT_FALSE;
    vlanInfo.fidValue              = vlanId;

    /* set vlan entry */
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);

    CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers,
                                prvTgfPortsArray[ING_PORT_IDX_CNS]);

    portsTaggingCmd.portsCmd[prvTgfPortsArray[ING_PORT_IDX_CNS]] = tagCmd;

    /* reset counters and set force link up */
    rc = prvTgfResetCountersEth(prvTgfDevNum,
                                prvTgfPortsArray[ING_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[ING_PORT_IDX_CNS]);

    CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers,
                                prvTgfPortsArray[EGR_PORT_IDX_CNS]);

    portsTaggingCmd.portsCmd[prvTgfPortsArray[EGR_PORT_IDX_CNS]] = tagCmd;

    /* reset counters and set force link up */
    rc = prvTgfResetCountersEth(prvTgfDevNum,
                                prvTgfPortsArray[EGR_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[EGR_PORT_IDX_CNS]);

    rc = prvTgfBrgVlanEntryWrite(prvTgfDevNum,
                                 vlanId,
                                 &portsMembers,
                                 &portsTagging,
                                 &vlanInfo,
                                 &portsTaggingCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d",
                                 vlanId);
}

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
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                        "tgfTrafficGeneratorIngressCscdPortEnableSet: [%d] \n", ingressPort);
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
* @internal prvTgfVlanEDsaTagTestPacketSend function
* @endinternal
*
* @brief   Function sends packet and check results.
*
* @param[in] portInterfacePtr         - (pointer to) port interface
* @param[in] packetInfoPtr            - (pointer to) the packet info
*                                       None
*/
static GT_VOID prvTgfVlanEDsaTagTestPacketSend
(
    IN CPSS_INTERFACE_INFO_STC  *portInterfacePtr,
    IN TGF_PACKET_STC           *packetInfoPtr
)
{
    GT_STATUS rc          = GT_OK;
    GT_U32    portsCount  = prvTgfPortsNum;
    GT_U32    portIter    = 0;

    setIngressPortCascade( prvTgfPortsArray[ING_PORT_IDX_CNS], GT_TRUE );
    setEgressPortCascade(  prvTgfPortsArray[EGR_PORT_IDX_CNS], GT_TRUE );

    /* Set union fields in the DSA structure */
    prvTgfPacketDsaTagPart_forward.source.portNum = 10;
    prvTgfPacketDsaTagPart_forward.origSrcPhy.portNum = 0;

    /* bind the DSA tag FORWARD part */
    prvTgfPacketDsaTagPart.dsaInfo.forward = prvTgfPacketDsaTagPart_forward;

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
                                   prvTgfPortsArray[ING_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[ING_PORT_IDX_CNS]);

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

    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n",
                     portInterfacePtr->devPort.portNum);

    setEgressPortCascade(  prvTgfPortsArray[EGR_PORT_IDX_CNS], GT_FALSE );
    setIngressPortCascade( prvTgfPortsArray[ING_PORT_IDX_CNS], GT_FALSE );

    /* AUTODOC: restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);
}

/**
* @internal prvTgfVlanEDsaTagTestSendAndCheck function
* @endinternal
*
* @brief   Function sends packetand performs trace.
*
* @param[in] vfdNum                   - VFD number
*                                      vfdArray      - VFD array with expected results
*                                       None
*/
static GT_VOID prvTgfVlanEDsaTagTestSendAndCheck
(
    IN GT_U8                    vfdNum,
    IN TGF_VFD_INFO_STC         *vfdArrayPtr
)
{
    GT_STATUS                       rc;
    CPSS_INTERFACE_INFO_STC         portInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    GT_U32                          numTriggers = 0;

    /* update vfdArrayPtr->patternPtr */
    {
        GT_U8*  dsaBytesPtr = vfdArrayPtr->patternPtr;
        TGF_PACKET_DSA_TAG_STC tmpDsa;
        rc = prvTgfTrafficGeneratorPacketDsaTagParse(dsaBytesPtr,
            &tmpDsa);
        UTF_VERIFY_EQUAL1_STRING_MAC( GT_OK, rc, "prvTgfTrafficGeneratorPacketDsaTagParse: %d",
                                  prvTgfDevNum);

        /* update the DSA tag format */
        tmpDsa.dsaInfo.forward.dstInterface.devPort.portNum = prvTgfPortsArray[EGR_PORT_IDX_CNS];

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



    /* AUTODOC: set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[EGR_PORT_IDX_CNS];

    /* AUTODOC: send packet. */
    prvTgfVlanEDsaTagTestPacketSend(&portInterface, &prvTgfPacketInfo);

    /* AUTODOC: check counters */

    /* read Rx counter */
    rc = prvTgfReadPortCountersEth(prvTgfDevNum,
                                   prvTgfPortsArray[ING_PORT_IDX_CNS],
                                   GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfReadPortCountersEth: %d, %d, %d",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[ING_PORT_IDX_CNS]);

    UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                 "get another goodPktsRcv counter than expected on port %d, %d",
                            prvTgfPortsArray[ING_PORT_IDX_CNS]);

    /* get trigger counters */
    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, vfdNum,
                                                     vfdArrayPtr, &numTriggers);
    PRV_UTF_LOG2_MAC("numTriggers = %d, rc = 0x%02X\n\n",(1 << vfdNum) - 1, rc);

    UTF_VERIFY_EQUAL2_STRING_MAC((1 << vfdNum) - 1, numTriggers,
            "get another trigger that expected: expected - %d, recieved - %d\n",
                                 (1 << vfdNum) - 1, numTriggers);

}

/**
* @internal prvTgfVlanEDsaTagConfigurationSet function
* @endinternal
*
* @brief   Egress PCL Quad lookup test initial configurations
*/
GT_VOID prvTgfVlanEDsaTagConfigurationSet
(
    IN PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT tagCmd
)
{
    GT_STATUS                   rc;
    CPSS_INTERFACE_INFO_STC     physicalInfo;
    CPSS_CSCD_LINK_TYPE_STC cascadeLink;
    PRV_TGF_CSCD_TRUNK_LINK_HASH_ENT srcPortTrunkHashEn;

    /* AUTODOC: SETUP CONFIGURATION: */
    cpssOsMemSet(&physicalInfo, 0, sizeof(physicalInfo));

    physicalInfo.type = CPSS_INTERFACE_PORT_E;
    physicalInfo.devPort.hwDevNum = prvTgfDevNum;
    physicalInfo.devPort.portNum = prvTgfPortsArray[EGR_PORT_IDX_CNS];

    /* save default Vlan entry parametrs */
    rc = prvTgfBrgVlanEntryRead(prvTgfDevNum, PRV_TGF_VLANID0_CNS,
                                &prvTgfVlanRestoreCfg.portsMembers,
                                &prvTgfVlanRestoreCfg.portsTagging,
                                &prvTgfVlanRestoreCfg.isValid,
                                &prvTgfVlanRestoreCfg.vlanInfo,
                                &prvTgfVlanRestoreCfg.taggingCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryRead %d",
                                 prvTgfDevNum);

    /* AUTODOC: create VLAN 1 with port 0,1. */
    prvTgfVlanEDsaTagTestVlanInit(PRV_TGF_VLANID0_CNS, tagCmd);

    /* AUTODOC: set enable bridge bypass for ingress port*/
    rc = prvTgfCscdPortBridgeBypassEnableSet(prvTgfDevNum,
                                             prvTgfPortsArray[ING_PORT_IDX_CNS],
                                             GT_TRUE);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: set enable bridge bypass for egress port*/
    rc = prvTgfCscdPortBridgeBypassEnableSet(prvTgfDevNum,
                                             prvTgfPortsArray[ING_PORT_IDX_CNS],
                                             GT_TRUE);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdPortBridgeBypassEnableSet: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_DSA_PORT_CNS]);

    /* AUTODOC: save target port selection mode for restore */
    rc = prvTgfPclEgressTargetPortSelectionModeGet(prvTgfDevNum,
                                                  &portSelectionModeForRestore);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                             "prvTgfPclEgressTargetPortSelectionModeGet: %d %d",
                             prvTgfDevNum, portSelectionModeForRestore);

    /* AUTODOC: set target port selection mode to LOCAL */
    rc = prvTgfPclEgressTargetPortSelectionModeSet(
                        PRV_TGF_PCL_EGRESS_TARGET_PORT_SELECTION_MODE_LOCAL_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfPclEgressTargetPortSelectionModSet: %d",
                         PRV_TGF_PCL_EGRESS_TARGET_PORT_SELECTION_MODE_LOCAL_E);

    /* AUTODOC: map ePort to physical port 2 */
    prvTgfVlanEDsaTagRedirectMapEport2PhySet( prvTgfDevNum,PRV_TGF_DSA_PORT_CNS,
                                           &physicalInfo, &savePhyInfo);

    cascadeLink.linkNum = prvTgfPortsArray[EGR_PORT_IDX_CNS];
    cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;
    srcPortTrunkHashEn = PRV_TGF_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E;
    /* AUTODOC: set cascade mapping for remote device\port */
    rc = prvTgfCscdDevMapTableSet(prvTgfDevNum, PRV_TGF_DSA_DEV_CNS,
                                  PRV_TGF_DSA_PORT_CNS, &cascadeLink,
                                  srcPortTrunkHashEn, GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,"prvTgfCscdDevMapTableSet: %d,%d,%d",
                                 prvTgfDevNum, PRV_TGF_DSA_DEV_CNS,
                                 PRV_TGF_DSA_PORT_CNS);

}

/**
* @internal prvTgfVlanEDsaTagConfigurationRestore function
* @endinternal
*
* @brief   Egress PCL Quad lookup restore configurations
*/
GT_VOID prvTgfVlanEDsaTagConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS                   rc;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d", GT_TRUE);

    /* AUTODOC: restore source port selection mode */
    rc = prvTgfPclEgressTargetPortSelectionModeSet(portSelectionModeForRestore);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfPclEgressTargetPortSelectionModSet: %d",
                                 portSelectionModeForRestore);

    /* AUTODOC: map ePort to physical port 2 */
    prvTgfVlanEDsaTagRedirectMapEport2PhySet( prvTgfDevNum,PRV_TGF_DSA_PORT_CNS,
                                           &savePhyInfo, NULL);

    /* AUTODOC: restore cascade mapping */
    rc = prvTgfCscdDevMapTableSet(prvTgfDevNum, PRV_TGF_DSA_DEV_CNS,
                                  PRV_TGF_DSA_PORT_CNS, &cascadeLinkForRestore,
                                  srcPortTrunkHashEnForRestore, GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,"prvTgfCscdDevMapTableSet: %d,%d,%d",
                                 prvTgfDevNum,
                                 PRV_TGF_DSA_DEV_CNS, PRV_TGF_DSA_PORT_CNS);

    /* AUTODOC: invalidate vlans entry (and reset vlans entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID0_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID0_CNS);

    /* AUTODOC: restore default VLAN 1 on all ports */
    rc = prvTgfBrgVlanEntryWrite(prvTgfDevNum, PRV_TGF_VLANID0_CNS,
                                &prvTgfVlanRestoreCfg.portsMembers,
                                &prvTgfVlanRestoreCfg.portsTagging,
                                &prvTgfVlanRestoreCfg.vlanInfo,
                                &prvTgfVlanRestoreCfg.taggingCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite %d",
                                 prvTgfDevNum);

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

}

/**
* @internal prvTgfVlanEDsaTagHandlingTest function
* @endinternal
*
* @brief   EPCL Quad lookup test
*/
GT_VOID prvTgfVlanEDsaTagHandlingTest
(
    GT_VOID
)
{
    GT_STATUS     rc = GT_OK;
    GT_HW_DEV_NUM hwDevNum;

    /* AUTODOC: re-arrange prvTgfPortsArray to ensure some ports (will be
     * configured cascade) are not remote ports */
    if (UTF_CPSS_DXCH_XCAT3X_CHECK_MAC(prvTgfDevNum))
    {
        rc = prvTgfDefPortsArrange(GT_FALSE,
                                   ING_PORT_IDX_CNS,
                                   EGR_PORT_IDX_CNS,
                                   -1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortsArray rearrangement can't be done");
    }

    /* get HW device number */
    rc = prvUtfHwDeviceNumberGet(prvTgfDevNum, &hwDevNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvUtfHwDeviceNumberGet: %d, %d",
                                 prvTgfDevNum, hwDevNum);

    /* set HwDevNum in expected DSA tag */
    PRV_TGF_SET_HW_DEV_NUM_DSA_TAG_MAC(tgfVlanUntaggedVfd[0].patternPtr[7], hwDevNum);
    PRV_TGF_SET_HW_DEV_NUM_DSA_TAG_MAC(tgfVlanTag0Vfd[0].patternPtr[7], hwDevNum);
    PRV_TGF_SET_HW_DEV_NUM_DSA_TAG_MAC(tgfVlanTag1Vfd[0].patternPtr[7], hwDevNum);
    PRV_TGF_SET_HW_DEV_NUM_DSA_TAG_MAC(tgfVlanOuterTag1InnerTag0Vfd[0].patternPtr[7], hwDevNum);
    PRV_TGF_SET_HW_DEV_NUM_DSA_TAG_MAC(tgfVlanOuterTag0InnerTag1Vfd[0].patternPtr[7], hwDevNum);

    /* AUTODOC: Configure untagged port */
    prvTgfVlanEDsaTagConfigurationSet(  PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E );

    prvTgfVlanEDsaTagTestSendAndCheck(1, &tgfVlanUntaggedVfd[0]);

    /* AUTODOC: Restore configuration */
    prvTgfVlanEDsaTagConfigurationRestore();

    /* AUTODOC: Configure tag0 port */
    prvTgfVlanEDsaTagConfigurationSet( PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E );

    prvTgfVlanEDsaTagTestSendAndCheck(1, &tgfVlanTag0Vfd[0]);

    /* AUTODOC: Restore configuration */
    prvTgfVlanEDsaTagConfigurationRestore();

    /* AUTODOC: Configure tag1 port */
    prvTgfVlanEDsaTagConfigurationSet( PRV_TGF_BRG_VLAN_PORT_TAG1_CMD_E );

    prvTgfVlanEDsaTagTestSendAndCheck(1, &tgfVlanTag1Vfd[0]);

    /* AUTODOC: Restore configuration */
    prvTgfVlanEDsaTagConfigurationRestore();

    /* AUTODOC: Configure outer tag0 and inner tag1 port */
    prvTgfVlanEDsaTagConfigurationSet(
                             PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);

    prvTgfVlanEDsaTagTestSendAndCheck(2, &tgfVlanOuterTag0InnerTag1Vfd[0]);

    /* AUTODOC: Restore configuration */
    prvTgfVlanEDsaTagConfigurationRestore();

    /* AUTODOC: Configure outer tag1 and inner tag0 port */
    prvTgfVlanEDsaTagConfigurationSet(
                            PRV_TGF_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E );

    /* AUTODOC: PCL action0, action1, action2 & action3 */
    prvTgfVlanEDsaTagTestSendAndCheck(2, &tgfVlanOuterTag1InnerTag0Vfd[0]);

    /* AUTODOC: Restore configuration */
    prvTgfVlanEDsaTagConfigurationRestore();

}

