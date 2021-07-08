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
* @file prvTgfTunnelTrillTti.c
*
* @brief Tunnel: TRILL tunnel termination
*
* @version   12
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfPortGen.h>
#include <common/tgfTunnelGen.h>
#include <tunnel/prvTgfTunnelTrillTti.h>
#include <common/tgfCscdGen.h>
#include <common/tgfConfigGen.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* Ethernet VLAN Id */
#define PRV_TGF_ETHERNET_VLANID_CNS         6

/* TRILL VLAN Id */
#define PRV_TGF_TRILL_VLANID_CNS            5

/* FDB entry source ePort */
#define PRV_TGF_SOURCE_EPORT_CNS          700

/* Tti entry index */
#define PRV_TGF_TTI_INDEX_CNS               8

/* MAC2ME entry index */
#define PRV_TGF_MAC_TO_ME_INDEX_CNS         1

/* Adjacency entry index*/
#define PRV_TGF_ADJACENCY_INDEX_CNS         1

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS           0

/* port number to receive traffic from */
#define PRV_TGF_RECEIVE_PORT_IDX_CNS        3

/* port number to receive traffic from */
#define PRV_TGF_RECEIVE_PORT_1_IDX_CNS       2

/* max number of VFD to use */
#define PRV_TGF_VFD_NUM_CNS                 10

/* CC Line-Card Ports Default ePort Base */
#define PRV_TGF_CC_PORTS_DEFAULT_EPORT_BASE_CNS     0x100

/* CC Default ePort Mapping DSA<SrcDev> LSB Amount */
#define PRV_TGF_CC_DEFAULT_EPORT_DSA_SRC_DEV_LSB_AMOUNT_CNS     4

/* CC Default ePort Mapping DSA<SrcPrv> LSB Amount */
#define PRV_TGF_CC_DEFAULT_EPORT_DSA_SRC_PORT_LSB_AMOUNT_CNS    3

/* define DSA tag SRC port/dev */
#define PRV_TGF_DSA_SRC_PORT_CNS            22
#define PRV_TGF_DSA_SRC_DEV_CNS              4

#define VLAN_TAG_INFO_INDEX  1  /* index in prvTgfPacketPartArray[]*/
#define DSA_INFO_INDEX  2       /* index in prvTgfPacketPartArray[]*/

/* indication if current run is for DSA tag testing */
static GT_BOOL      testingDsaTag = GT_FALSE;
/* indication if packet from cascade port should bypass the bridge or not (for destination) */
static GT_BOOL      testingDsaBypassBridge = GT_FALSE;

static GT_U32   origDevTableBmp[PRV_TGF_FDB_DEV_TABLE_SIZE_CNS] = {0};

/* default number of packets to send */
static GT_U32 prvTgfBurstCount = 1;

#define PRV_UTF_LOG_VERIFY_GT_OK(rc, name)                                   \
    if (GT_OK != (rc))                                                       \
    {                                                                        \
        PRV_UTF_LOG2_MAC("\n[TGF]: %s FAILED, rc = [%d]\n", name, rc);       \
    }

/******************************* Test packet **********************************/
/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x55, 0x44, 0x33, 0x22, 0x11},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x05}                 /* saMac */
};

/* VLAN_TAG part of packet */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_TRILL_VLANID_CNS                      /* pri, cfi, VlanId */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {TGF_ETHERTYPE_22F3_TRILL_TAG_CNS};

/* packet's TRILL */
static TGF_PACKET_TRILL_STC prvTgfPacketTrillPart =
{
    0,                  /* version*/
    GT_FALSE,           /* mBit */
    0,                  /* opLength */
    0x20,               /* hopCount */
    0x6543,             /* eRbid */
    0x4321              /* iRbid*/
};

/***************** Incapsulation ethernet frame **********************/
/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2PartInt = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x22}                 /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPartInt =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,        /* etherType */
    0, 0, PRV_TGF_ETHERNET_VLANID_CNS       /* pri, cfi, VlanId */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPayloadDataArr),  /* dataLength */
    prvTgfPayloadDataArr           /* dataPtr */
};

/********************** DSA tag  ***************************/
static TGF_DSA_DSA_FORWARD_STC  prvTgfPacketDsaTagPart_forward = {
    GT_FALSE , /*srcIsTagged*/
    8,/*srcHwDev*/
    GT_FALSE,/* srcIsTrunk */
    /*union*/
    {
        /*trunkId*/
        5/*portNum*/
    },/*source;*/

    13,/*srcId*/

    GT_FALSE,/*egrFilterRegistered*/
    GT_FALSE,/*wasRouted*/
    0,/*qosProfileIndex*/

    /*CPSS_INTERFACE_INFO_STC         dstInterface*/
    {
        CPSS_INTERFACE_PORT_E,/*type*/

        /*struct*/{
            0,/*devNum*/
            18/*portNum*/
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
    0,/*tag0TpidIndex*/
    GT_FALSE,/*origSrcPhyIsTrunk*/
    /* union */ /* union fields are set in runtime due to 'Big endian' / 'little endian' issues */
    {
        /*trunkId*/
        0/*portNum*/
    },/*origSrcPhy*/
    GT_FALSE,/*phySrcMcFilterEnable*/
    0, /* hash */
    GT_TRUE /*skipFdbSaLookup*/
};

static TGF_PACKET_DSA_TAG_STC  prvTgfPacketDsaTagPart = {
    TGF_DSA_CMD_FORWARD_E ,/*dsaCommand*/
    TGF_DSA_2_WORD_TYPE_E ,/*dsaType*/

    /*TGF_DSA_DSA_COMMON_STC*/
    {
        0,/*vpt*/
        0,/*cfiBit*/
        PRV_TGF_TRILL_VLANID_CNS,/*vid*/
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

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},     /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_SKIP_E,       NULL},                   /* will be used for DSA tag testing */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_TRILL_E,     &prvTgfPacketTrillPart},
    /* incapsulation ethernet frame part */
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2PartInt},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPartInt},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Length of transmitted packet */
#define PRV_TGF_TRANSMITTED_PACKET_LEN_CNS    \
    TGF_L2_HEADER_SIZE_CNS * 2   + \
    TGF_VLAN_TAG_SIZE_CNS  * 2   + \
    TGF_ETHERTYPE_SIZE_CNS       + \
    TGF_TRILL_HEADER_SIZE_CNS    + \
    sizeof(prvTgfPayloadDataArr)

/* Length of transmitted packet with DSA */
#define PRV_TGF_TRANSMITTED_PACKET_LEN_WITH_DSA_CNS    \
    TGF_L2_HEADER_SIZE_CNS * 2   + \
    TGF_DSA_TAG_SIZE_CNS         + \
    TGF_VLAN_TAG_SIZE_CNS        + \
    TGF_ETHERTYPE_SIZE_CNS       + \
    TGF_TRILL_HEADER_SIZE_CNS    + \
    sizeof(prvTgfPayloadDataArr)

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    PRV_TGF_TRANSMITTED_PACKET_LEN_CNS,      /* totalLen */
    (sizeof(prvTgfPacketPartArray)
        / sizeof(prvTgfPacketPartArray[0])), /* numOfParts */
    prvTgfPacketPartArray                    /* partsArray */
};

/* PACKET to send with DSA */
static TGF_PACKET_STC prvTgfPacketWithDsaInfo =
{
    PRV_TGF_TRANSMITTED_PACKET_LEN_WITH_DSA_CNS, /* totalLen */
    (sizeof(prvTgfPacketPartArray)
        / sizeof(prvTgfPacketPartArray[0])), /* numOfParts */
    prvTgfPacketPartArray                    /* partsArray */
};

/* LENGTH of expected packet (original packet tunneled over TRILL packet) */
#define PRV_TGF_EXPECTED_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS  + sizeof(prvTgfPayloadDataArr)


/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static struct
{
    GT_BOOL                                     vlanEgressFilteringState;
    PRV_TGF_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT   vlanEgressPortTagStateMode;
    PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT           vlanEgressPortTagState;
    GT_BOOL                                     SrcPortPe;
    GT_U32                                      trillEtherType;
    GT_BOOL                                     trillEnable;
    GT_U16                                      trillOuterVid0;

} prvTgfRestoreCfg;

/******************************************************************************/

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/
/**
* @internal prvTgfTunnelTrillKnownUcDataToTtiConfigurationDsaTagEnableSet function
* @endinternal
*
* @brief   enable/disable running the test in DSA tag mode.
*
* @param[in] enable                   - enable/disable running the test in DSA tag mode.
* @param[in] dsaBypassBridge          - relevant when enable = TRUE
*                                      indicate that the traffic with 'forward' DSA tag do or not
*                                      'bypass' to the bridge.
*                                       None
*/
GT_VOID prvTgfTunnelTrillKnownUcDataToTtiConfigurationDsaTagEnableSet
(
    IN GT_BOOL      enable,
    IN GT_BOOL      dsaBypassBridge
)
{
    GT_HW_DEV_NUM  srcDsaHwDevNum     = PRV_TGF_DSA_SRC_DEV_CNS;
    GT_U32         portIter;
    static GT_U32  devTableBmp[PRV_TGF_FDB_DEV_TABLE_SIZE_CNS];
    CPSS_CSCD_LINK_TYPE_STC      cascadeLink;
    GT_BOOL     dualDeviceId = GT_FALSE;
    GT_STATUS   rc;

    testingDsaBypassBridge = dsaBypassBridge;

    utfGeneralStateMessageSave(0,"dsaTagEnable = %d",enable);
    utfGeneralStateMessageSave(1,"dsaBypassBridge = %d",dsaBypassBridge);

    if(testingDsaTag == enable)
    {
        /* no 'restore' / 'new config' needed */
        return;
    }

    testingDsaTag = enable;

    cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;

    if(testingDsaTag == GT_TRUE)
    {
        /* set DSA tag info related values */

        /* bind the DSA tag info */
        prvTgfPacketPartArray[DSA_INFO_INDEX].type    = TGF_PACKET_PART_DSA_TAG_E;
        prvTgfPacketPartArray[DSA_INFO_INDEX].partPtr = &prvTgfPacketDsaTagPart;
        prvTgfPacketPartArray[VLAN_TAG_INFO_INDEX].type    = TGF_PACKET_PART_SKIP_E;

        prvTgfPacketDsaTagPart_forward.srcHwDev = srcDsaHwDevNum;

        /* allow traffic to be in the FDB that is associated with the 'remote device' */
        rc = prvTgfBrgFdbDeviceTableGet(&origDevTableBmp[0]);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        devTableBmp[0] = origDevTableBmp[0] | (1<<srcDsaHwDevNum);
        rc = prvTgfBrgFdbDeviceTableSet(&devTableBmp[0]);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        /* Set lookup mode for accessing the Device Map table */
        rc = prvTgfCscdDevMapLookupModeSet(PRV_TGF_DEV_MAP_LOOKUP_MODE_TRG_DEV_TRG_PORT_E);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            /* The function enables/disables bypass of the bridge engine per port */
            rc = prvTgfCscdPortBridgeBypassEnableSet(prvTgfDevsArray[portIter],prvTgfPortsArray[portIter],dsaBypassBridge);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

            if(prvTgfPortsArray[portIter] >= 64)
            {
                dualDeviceId = GT_TRUE;

            }
        }

        if(dualDeviceId == GT_TRUE &&
           sysGenGlobalInfo.supportDualDeviceId == GT_TRUE)
        {
            /* allow the src port to be >= 64 */
            rc = prvTgfCscdDbRemoteHwDevNumModeSet(srcDsaHwDevNum,CPSS_GEN_CFG_HW_DEV_NUM_MODE_DUAL_E);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        }

        /* set the device map table to know where to send traffic destined to PRV_TGF_DSA_SRC_DEV_CNS

           DO it only after call to : prvTgfCscdDbRemoteHwDevNumModeSet(srcDsaHwDevNum)
        */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            cascadeLink.linkNum = prvTgfPortsArray[portIter];
            rc = prvTgfCscdDevMapTableSet(prvTgfDevNum,
                                     srcDsaHwDevNum,
                                     prvTgfPortsArray[portIter],
                                     &cascadeLink,
                                     GT_FALSE, GT_FALSE);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        }


    }
    else    /*when restore values make sure to do all restore without rc checking !*/
    {
        /* restore values */
        prvTgfPacketPartArray[VLAN_TAG_INFO_INDEX].type    = TGF_PACKET_PART_VLAN_TAG_E;
        prvTgfPacketPartArray[DSA_INFO_INDEX].type    = TGF_PACKET_PART_SKIP_E;
        prvTgfPacketPartArray[DSA_INFO_INDEX].partPtr = NULL;

        prvTgfBrgFdbDeviceTableSet(&origDevTableBmp[0]);

        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            prvTgfCscdPortBridgeBypassEnableSet(prvTgfDevsArray[portIter],prvTgfPortsArray[portIter],GT_TRUE);
            tgfTrafficGeneratorIngressCscdPortEnableSet(prvTgfDevsArray[portIter],prvTgfPortsArray[portIter],GT_FALSE);
            /* set the device map table to know where to send traffic destined to PRV_TGF_DSA_SRC_DEV_CNS */
            cascadeLink.linkNum = CPSS_NULL_PORT_NUM_CNS;
            prvTgfCscdDevMapTableSet(prvTgfDevNum,
                                     srcDsaHwDevNum,
                                     prvTgfPortsArray[portIter],
                                     &cascadeLink,
                                     GT_FALSE, GT_FALSE);
        }

        prvTgfCscdDbRemoteHwDevNumModeSet(srcDsaHwDevNum,CPSS_GEN_CFG_HW_DEV_NUM_MODE_SINGLE_E);
        prvTgfCscdDevMapLookupModeSet(PRV_TGF_DEV_MAP_LOOKUP_MODE_TRG_DEV_E);
    }
}

/**
* @internal prvTgfTunnelTrillKnownUcDataToTtiConfigSet function
* @endinternal
*
* @brief   Set configuration
*/
GT_VOID prvTgfTunnelTrillKnownUcDataToTtiConfigSet
(
    GT_VOID
)
{
    GT_STATUS                                   rc = GT_OK;
    PRV_TGF_TTI_RULE_UNT                        pattern;
    PRV_TGF_TTI_RULE_UNT                        mask;
    PRV_TGF_TTI_ACTION_STC                      ruleAction = {0};
    PRV_TGF_TTI_TRILL_ADJACENCY_STC             adjacencyPattern;
    PRV_TGF_TTI_TRILL_RBID_TABLE_ENTRY_STC      iRbidEntry;
    PRV_TGF_TTI_TRILL_RBID_TABLE_ENTRY_STC      eRbidEntry;
    /*PRV_TGF_TTI_TRILL_RBID_LTT_TABLE_ENTRY_STC  iRbidLttEntry;
    PRV_TGF_TTI_TRILL_RBID_LTT_TABLE_ENTRY_STC  eRbidLttEntry;*/
    PRV_TGF_TTI_MAC_VLAN_STC                    macToMePattern;
    PRV_TGF_TTI_MAC_VLAN_STC                    macToMeMask;

    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* clear entry */
    cpssOsMemSet((GT_VOID*) &pattern, 0, sizeof(pattern));
    cpssOsMemSet((GT_VOID*) &mask,    0, sizeof(mask));
    cpssOsMemSet((GT_VOID*) &adjacencyPattern, 0, sizeof(adjacencyPattern));
    cpssOsMemSet((GT_VOID*) &iRbidEntry,       0, sizeof(iRbidEntry));
    cpssOsMemSet((GT_VOID*) &eRbidEntry,       0, sizeof(eRbidEntry));
    /*cpssOsMemSet((GT_VOID*) &iRbidLttEntry,    0, sizeof(iRbidLttEntry));
    cpssOsMemSet((GT_VOID*) &eRbidLttEntry,    0, sizeof(eRbidLttEntry));*/
    cpssOsMemSet((GT_VOID*) &macToMePattern,   0, sizeof(macToMePattern));
    cpssOsMemSet((GT_VOID*) &macToMeMask,      0, sizeof(macToMeMask));

    /* AUTODOC: create TRILL VLAN 5 with untagged ports [0,1] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_TRILL_VLANID_CNS, prvTgfPortsArray,
                                           NULL, NULL, 2);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgDefVlanEntryWithPortsSet);

    /* AUTODOC: create TRILL VLAN 6 with untagged ports [2,3] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_ETHERNET_VLANID_CNS, prvTgfPortsArray + 2,
                                           NULL, NULL, 2);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgDefVlanEntryWithPortsSet);

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 6, port 3 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
            prvTgfPacketL2PartInt.daMac, PRV_TGF_ETHERNET_VLANID_CNS, prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], GT_TRUE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgDefFdbMacEntryOnPortSet);

    /* AUTODOC: add FDB entry with MAC 00:55:44:33:22:11, VLAN 5, port 3 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
            prvTgfPacketL2Part.daMac, PRV_TGF_TRILL_VLANID_CNS, prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], GT_TRUE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgDefFdbMacEntryOnPortSet);

    /* set trill ethertype */
    rc = prvTgfTtiEthernetTypeGet(prvTgfDevNum, PRV_TGF_TUNNEL_ETHERTYPE_TYPE_TRILL_E, &(prvTgfRestoreCfg.trillEtherType));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiEthernetTypeGet);

    /* AUTODOC: set TRILL ethertype 0x22F3 */
    rc = prvTgfTtiEthernetTypeSet(prvTgfDevNum, PRV_TGF_TUNNEL_ETHERTYPE_TYPE_TRILL_E, TGF_ETHERTYPE_22F3_TRILL_TAG_CNS);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiEthernetTypeSet);

    /* set trill engine enable on transmit port */
    rc = prvTgfTtiPortTrillEnableGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &(prvTgfRestoreCfg.trillEnable));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiPortTrillEnableGet);

    /* AUTODOC: enable TRILL engine on port 0 */
    rc = prvTgfTtiPortTrillEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiPortTrillEnableSet);

    /* AUTODOC: enable TTI lookup for port 0, key TTI_KEY_TRILL */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_TRILL_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* configure TTI rule for TRILL */
    pattern.trill.trillMBit = GT_FALSE;
    pattern.trill.trillEgressRbid = 0x6543;

    /* configure TTI rule mask for TRILL */
    mask.trill.trillMBit = GT_TRUE;
    mask.trill.trillEgressRbid = 0xFFFF;

    /* configure TTI rule action */
    ruleAction.command                 = CPSS_PACKET_CMD_FORWARD_E;
    ruleAction.tunnelTerminate         = GT_TRUE;
    ruleAction.passengerPacketType     = PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E;
    /*changed from PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E; because when 'no modify' the re-parse not take vid0 from the passenger ! */
    ruleAction.tag0VlanCmd             = PRV_TGF_TTI_VLAN_MODIFY_UNTAGGED_E;
    ruleAction.copyTtlFromTunnelHeader = GT_FALSE;

    /* set TTI rule - A single TRILL TTI rule is used to match all TRILL Unicast-to-me packets */

    /* AUTODOC: add TTI rule 8 with: */
    /* AUTODOC:   key TRILL, cmd FORWARD */
    /* AUTODOC:   pattern trillEgressRbid=0x6543 */
    /* AUTODOC:   action tag0VlanCmd=DO_NOT_MODIFY,tag1VlanCmd=MODIFY_UNTAGGED */
    /* AUTODOC:   action passenger=ETHERNET_NO_CRC */
    rc = prvTgfTtiRuleSet(PRV_TGF_TTI_INDEX_CNS, PRV_TGF_TTI_KEY_TRILL_E,
                          &pattern, &mask, &ruleAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet");

    /* This flag indicates that the packet arrived TRILL-encapsulated,
       and therefore should not be flooded on outgoing TRILL interfaces */

    /* disable VLAN egress filtering, TRILL packets are not subject to bridge VLAN ingress filtering */

    /* save VLAN egress filtering configuration */
    rc = prvTgfBrgVlanEgressFilteringEnableGet(prvTgfDevNum, &(prvTgfRestoreCfg.vlanEgressFilteringState));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressFilteringEnableGet);

    /* AUTODOC: disable VLAN egress filtering */
    rc = prvTgfBrgVlanEgressFilteringEnable(GT_FALSE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressFilteringEnable);

    /* set the Outer Tag0 VID that must be for all TRILL packets from the port */
    rc = prvTgfTtiPortTrillOuterVid0Get(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &(prvTgfRestoreCfg.trillOuterVid0));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiPortTrillOuterVid0Get);

    /* AUTODOC: set Outer Tag0 VID to 5 for all TRILL packets from port 0 */
    rc = prvTgfTtiPortTrillOuterVid0Set(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_TRILL_VLANID_CNS);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiPortTrillOuterVid0Set);

    /* set MAC to ME table */
    cpssOsMemCpy((GT_VOID*)macToMePattern.mac.arEther,
                 (GT_VOID*)prvTgfPacketL2Part.daMac,
                 sizeof(macToMePattern.mac.arEther));
    macToMePattern.vlanId = PRV_TGF_TRILL_VLANID_CNS;

    cpssOsMemSet((GT_VOID*)macToMeMask.mac.arEther, 0xFF,
                 sizeof(macToMeMask.mac.arEther));
    macToMeMask.vlanId = 0xFFF;

    /* AUTODOC: config MAC to ME table 1: */
    /* AUTODOC:   DA=00:55:44:33:22:11, VID=5 */
    rc = prvTgfTtiMacToMeSet(PRV_TGF_MAC_TO_ME_INDEX_CNS, &macToMePattern, &macToMeMask);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiMacToMeSet);

    /* TRILL Adjacency Check Configuration */
    cpssOsMemCpy((GT_VOID*)adjacencyPattern.outerMacSa.arEther,
                 (GT_VOID*)prvTgfPacketL2Part.saMac,
                 sizeof(adjacencyPattern.outerMacSa.arEther));
    adjacencyPattern.trillMBit      = 0;
    adjacencyPattern.srcHwDevice      = prvTgfDevNum;
    adjacencyPattern.srcIsTrunk     = 0;
    adjacencyPattern.srcPortTrunk   = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]; /* default ePort */

    /* AUTODOC: config entry in TRILL Adjacency Check dedicated TCAM: */
    /* AUTODOC:   SA=00:00:00:00:00:05, port=0 */
    rc = prvTgfTtiTrillAdjacencyCheckEntrySet(prvTgfDevNum, PRV_TGF_ADJACENCY_INDEX_CNS, &adjacencyPattern);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiTrillAdjacencyCheckEntrySet);

    /* RBID LTT table configuration */
    /*iRbidLttEntry.rbidEntryIndex = 11;
    eRbidLttEntry.rbidEntryIndex = 22;*/

    /* AUTODOC: config RBID LTT table 0x4321: */
    /* AUTODOC:   iRbidLtt=11, eRbidLtt=22 */
    /*rc = prvTgfTtiTrillRbidLttEntrySet(prvTgfDevNum, 0x4321, &iRbidLttEntry);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiTrillRbidLttEntrySet);*/

    /* AUTODOC: config RBID LTT table 0x6543: */
    /* AUTODOC:   iRbidLtt=11, eRbidLtt=22 */
    /*rc = prvTgfTtiTrillRbidLttEntrySet(prvTgfDevNum, 0x6543, &eRbidLttEntry);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiTrillRbidLttEntrySet);*/

    /* RBID lookup table configuration */
    eRbidEntry.srcTrgPort = 0;
    eRbidEntry.trgHwDevice = prvTgfDevNum; /* own device */

    iRbidEntry.srcTrgPort = PRV_TGF_SOURCE_EPORT_CNS; /* Trg ePort only relevant for TRILL UC transit */
    iRbidEntry.trgHwDevice = prvTgfDevNum; /* own device */

    /* AUTODOC: set srcTrgPort=700 for RBID lookup table 11 */
    /*rc = prvTgfTtiTrillRbidEntrySet(prvTgfDevNum, iRbidLttEntry.rbidEntryIndex, &iRbidEntry);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiTrillRbidEntrySet);*/

    /* AUTODOC: set srcTrgPort=1000 for RBID lookup table 22 */
    /*rc = prvTgfTtiTrillRbidEntrySet(prvTgfDevNum, eRbidLttEntry.rbidEntryIndex, &eRbidEntry);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiTrillRbidEntrySet);*/

    if(testingDsaTag == GT_TRUE)
    {
        /* AUTODOC: enable local source port=0 assignment from DSA tag, used for centralized chassis */
        rc = prvTgfCscdCentralizedChassisModeEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE);
        PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfCscdCentralizedChassisModeEnableSet);

        /* AUTODOC: configure default ePort mapping assignment on ingress centralized chassisenabled ports, */
        /* AUTODOC: when packets are received from line-card port/trunk accordingly. */
        /* AUTODOC: port=0, portBaseEport=PRV_TGF_CC_PORTS_DEFAULT_EPORT_BASE_CNS, trunkBaseEport=0 */
        rc = prvTgfCscdCentralizedChassisLineCardDefaultEportBaseSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_CC_PORTS_DEFAULT_EPORT_BASE_CNS, 0);
        PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfCscdCentralizedChassisLineCardDefaultEportBaseSet);

        /* AUTODOC: configure the amount of least significant bits taken from DSA tag for assigning a default source ePort on CC ports, */
        /* AUTODOC: for packets received from line-card device trunks/physical ports accordingly. */
        /* AUTODOC: srcTrunkLsbAmount=0, srcPortLsbAmount=PRV_TGF_CC_DEFAULT_EPORT_DSA_SRC_PORT_LSB_AMOUNT_CNS, srcDevLsbAmount=PRV_TGF_CC_DEFAULT_EPORT_DSA_SRC_DEV_LSB_AMOUNT_CNS */
        rc = prvTgfCscdCentralizedChassisMappingDsaSrcLsbAmountSet(0, PRV_TGF_CC_DEFAULT_EPORT_DSA_SRC_PORT_LSB_AMOUNT_CNS, PRV_TGF_CC_DEFAULT_EPORT_DSA_SRC_DEV_LSB_AMOUNT_CNS);
        PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfCscdCentralizedChassisMappingDsaSrcLsbAmountSet);

        /* AUTODOC: disable TRILL engine on port 0 */
        rc = prvTgfTtiPortTrillEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
        PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiPortTrillEnableSet);

        /* AUTODOC: Set Port=0 default Source ePort number=12 */
        rc = prvTgfCfgPortDefaultSourceEportNumberSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], 13);
        PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfCfgPortDefaultSourceEportNumberSet);
    }
}

/**
* @internal prvTgfTunnelTrillKnownUcDataToTtiWithDsaTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
static GT_VOID prvTgfTunnelTrillKnownUcDataToTtiWithDsaTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS               rc = GT_OK;
    GT_U32                  portIter    = 0;
    GT_U8                   packetsCountRx;
    GT_U8                   packetsCountTx;
    GT_U32                  packetSize;
    PRV_TGF_BRG_MAC_ENTRY_STC macEntry;
    PRV_TGF_MAC_ENTRY_KEY_STC macEntryKey;
    GT_U32                  ccSrcEPort = 0; /* new value according to cc line card settings */
    GT_HW_DEV_NUM           hwDev;

     if(testingDsaTag == GT_FALSE)
         return;

    /* AUTODOC: GENERATE TRAFFIC: */

     /* enable packet trace */
     rc = tgfTrafficTracePacketByteSet(GT_TRUE);
     UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* AUTODOC: send Tunneled packet with an 8B extended DSA tag in FORWARD format from port 0 */

    /* set the target port of the packet in the DSA tag info */
    prvTgfPacketDsaTagPart_forward.dstInterface.type = CPSS_INTERFACE_PORT_E;
    prvTgfPacketDsaTagPart_forward.dstInterface.devPort.hwDevNum = prvTgfDevNum;
    prvTgfPacketDsaTagPart_forward.dstInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_1_IDX_CNS];

    /* indication that the source device found the FDB entry in it's FDB */
    prvTgfPacketDsaTagPart_forward.egrFilterRegistered = GT_TRUE;

    /* set the info for the 'source learning' */
    prvTgfPacketDsaTagPart_forward.source.portNum = PRV_TGF_DSA_SRC_PORT_CNS;

    /* do this right before the send of packet */
    /* bind the DSA tag FORWARD part (since union is used .. can't do it in compilation time) */
    prvTgfPacketDsaTagPart.dsaInfo.forward = prvTgfPacketDsaTagPart_forward;

    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* AUTODOC: enable the port to assume that ingress traffic hold DSA tag info */
        rc = tgfTrafficGeneratorIngressCscdPortEnableSet(prvTgfDevsArray[portIter], prvTgfPortsArray[portIter], GT_TRUE);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    }
    if (testingDsaBypassBridge == GT_FALSE)
    {
        /* Assign the same new value to ccSrcEPort:
           If DSA<SrcIsTrunk> == 0
                Assign <CC Line-Card Ports Default ePort Base> +
                {DSA<SrcDev>[<CC Default ePort Mapping DSA<SrcDev> LSB Amount>:0],
                DSA<SrcPort>[<CC Default ePort Mapping DSA<SrcPort> LSB Amount>:0]}; */
        /* Verify MAC SA learning from DSA tag is performed */

        U32_SET_FIELD_MASKED_MAC(ccSrcEPort,
                           PRV_TGF_CC_DEFAULT_EPORT_DSA_SRC_PORT_LSB_AMOUNT_CNS ,
                           PRV_TGF_CC_DEFAULT_EPORT_DSA_SRC_DEV_LSB_AMOUNT_CNS,
                           prvTgfPacketDsaTagPart_forward.srcHwDev);

        U32_SET_FIELD_MASKED_MAC(ccSrcEPort,
                           0 ,
                           PRV_TGF_CC_DEFAULT_EPORT_DSA_SRC_PORT_LSB_AMOUNT_CNS,
                           prvTgfPacketDsaTagPart_forward.source.portNum);

        ccSrcEPort += PRV_TGF_CC_PORTS_DEFAULT_EPORT_BASE_CNS;

        rc = prvTgfCscdPortBridgeBypassEnableSet(prvTgfDevNum,ccSrcEPort,GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortBridgeBypassEnableSet");

        rc = prvTgfBrgFdbNaToCpuPerPortSet(prvTgfDevNum, ccSrcEPort, GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbNaToCpuPerPortSet");

        /* send tagged packet with 8B extended DSA tag in FORWARD format */
        rc = prvTgfTransmitPacketsWithCapture(prvTgfDevNum,
                                              prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                              &prvTgfPacketWithDsaInfo, prvTgfBurstCount, 0, NULL,
                                              prvTgfDevNum,
                                              prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
                                              TGF_CAPTURE_MODE_MIRRORING_E, 10);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* -------------------------------------------------------------------------
         * 3. Get Ethernet Counters
         */

        /* AUTODOC: verify to get Ethernet packet on port 2 with: */
        /* AUTODOC: DA=00:00:00:00:34:02, SA=00:00:00:00:00:22 */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            if (portIter == PRV_TGF_SEND_PORT_IDX_CNS)
            {
                packetsCountTx = (GT_U8)prvTgfBurstCount;
                packetsCountRx = (GT_U8)prvTgfBurstCount;
                packetSize     = PRV_TGF_TRANSMITTED_PACKET_LEN_WITH_DSA_CNS;
            }
            else if (portIter == PRV_TGF_RECEIVE_PORT_IDX_CNS)
            {
                packetsCountTx = (GT_U8)prvTgfBurstCount;
                packetsCountRx = (GT_U8)prvTgfBurstCount;
                packetSize     = PRV_TGF_TRANSMITTED_PACKET_LEN_WITH_DSA_CNS - TGF_DSA_TAG_SIZE_CNS;
            }
            else
            {
                packetsCountRx = 0;
                packetsCountTx = 0;
                packetSize = 0;
            }

            /* check ETH counters */
            rc = prvTgfEthCountersCheck(prvTgfDevNum,
                                        prvTgfPortsArray[portIter],
                                        packetsCountRx,
                                        packetsCountTx,
                                        packetSize,
                                        prvTgfBurstCount);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
        }

        rc = prvUtfHwDeviceNumberGet(prvTgfDevNum,&hwDev);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvUtfHwDeviceNumberGet");

        cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
        cpssOsMemSet(&macEntryKey, 0, sizeof(macEntryKey));
        macEntryKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
        macEntryKey.key.macVlan.vlanId = PRV_TGF_TRILL_VLANID_CNS;

        cpssOsMemCpy(macEntryKey.key.macVlan.macAddr.arEther, prvTgfPacketL2Part.saMac, sizeof(TGF_MAC_ADDR));

        rc = prvTgfBrgFdbMacEntryGet(&macEntryKey, &macEntry);

        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_NOT_FOUND, rc, "Device has learned DSA Src Device %d, DSA Src Port %d\n",
                                     macEntry.dstInterface.devPort.hwDevNum, macEntry.dstInterface.devPort.portNum);
        if (macEntry.dstInterface.devPort.hwDevNum != hwDev || macEntry.dstInterface.devPort.portNum != ccSrcEPort)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FALSE, "wrong DSA MAC SA learning");
        }
    }
    else
    {
        /* -------------------------------------------------------------------------
         * 5. Generating Traffic
         */

        /* set the info for the 'source learning' */
        prvTgfPacketDsaTagPart_forward.source.portNum = PRV_TGF_DSA_SRC_PORT_CNS;
        prvTgfPacketDsaTagPart_forward.srcHwDev = PRV_TGF_DSA_SRC_DEV_CNS;

         /* AUTODOC: disable local source port=0 assignment from DSA tag, used for centralized chassis */
        rc = prvTgfCscdCentralizedChassisModeEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
        PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfCscdCentralizedChassisModeEnableSet);

        /* send tagged packet with 8B extended DSA tag in FORWARD format */
        rc = prvTgfTransmitPacketsWithCapture(prvTgfDevNum,
                                              prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                              &prvTgfPacketWithDsaInfo, prvTgfBurstCount, 0, NULL,
                                              prvTgfDevNum,
                                              prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_1_IDX_CNS],
                                              TGF_CAPTURE_MODE_MIRRORING_E, 10);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);


        /* -------------------------------------------------------------------------
         * 3. Get Ethernet Counters
         */

        /* AUTODOC: verify to get Ethernet packet on port 2 with: */
        /* AUTODOC: DA=00:00:00:00:34:02, SA=00:00:00:00:00:22 */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            if (portIter == PRV_TGF_SEND_PORT_IDX_CNS)
            {
                packetsCountTx = (GT_U8)prvTgfBurstCount;
                packetsCountRx = (GT_U8)prvTgfBurstCount;
                packetSize     = PRV_TGF_TRANSMITTED_PACKET_LEN_WITH_DSA_CNS;
            }
            else if (portIter == PRV_TGF_RECEIVE_PORT_1_IDX_CNS)
            {
                packetsCountTx = (GT_U8)prvTgfBurstCount;
                packetsCountRx = (GT_U8)prvTgfBurstCount;
                packetSize     = PRV_TGF_TRANSMITTED_PACKET_LEN_WITH_DSA_CNS - TGF_DSA_TAG_SIZE_CNS;
            }
            else
            {
                packetsCountRx = 0;
                packetsCountTx = 0;
                packetSize = 0;
            }

            /* check ETH counters */
            rc = prvTgfEthCountersCheck(prvTgfDevNum,
                                        prvTgfPortsArray[portIter],
                                        packetsCountRx,
                                        packetsCountTx,
                                        packetSize,
                                        prvTgfBurstCount);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
        }

         /* Verify MAC SA learning from DSA tag is performed */
        cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
        cpssOsMemSet(&macEntryKey, 0, sizeof(macEntryKey));
        macEntryKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
        macEntryKey.key.macVlan.vlanId = PRV_TGF_TRILL_VLANID_CNS;

        cpssOsMemCpy(macEntryKey.key.macVlan.macAddr.arEther, prvTgfPacketL2Part.saMac, sizeof(TGF_MAC_ADDR));

        rc = prvTgfBrgFdbMacEntryGet(&macEntryKey, &macEntry);

        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_NOT_FOUND, rc, "Device has learned DSA Src Device %d, DSA Src Port %d\n",
                                     macEntry.dstInterface.devPort.hwDevNum, macEntry.dstInterface.devPort.portNum);
        if (macEntry.dstInterface.devPort.hwDevNum != PRV_TGF_DSA_SRC_DEV_CNS || macEntry.dstInterface.devPort.portNum != PRV_TGF_DSA_SRC_PORT_CNS)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FALSE, "wrong DSA MAC SA learning");
        }
    }

}

/**
* @internal prvTgfTunnelTrillKnownUcDataToTtiTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
static GT_VOID prvTgfTunnelTrillKnownUcDataToTtiTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS               rc = GT_OK;
    GT_U32                  portIter    = 0;
    CPSS_INTERFACE_INFO_STC portInterface;
    GT_U8                   packetsCountRx;
    GT_U8                   packetsCountTx;
    GT_U32                  packetSize;
    GT_U32                  packetIter  = 0;
    GT_U32                  triggerBitmap = 0;
    TGF_VFD_INFO_STC        vfdArray[PRV_TGF_VFD_NUM_CNS];
    GT_U32                  vfdIndex = 0;/* index in vfdArray */

    if(testingDsaTag == GT_TRUE)
         return;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

     /* enable packet trace */
     rc = tgfTrafficTracePacketByteSet(GT_TRUE);
     UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* AUTODOC: send Tunneled packet from port 0 with: */
    /* AUTODOC:   DA=00:55:44:33:22:11, SA=00:00:00:00:00:05, VID=5 */
    /* AUTODOC:   EtherType=0x22F3, hopCount=0x20, eRbid=0x6543, iRbid=0x4321 */
    /* AUTODOC:   passenger DA=00:00:00:00:34:02, SA=00:00:00:00:00:22, VID=6 */
    rc = prvTgfTransmitPacketsWithCapture(prvTgfDevNum,
                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                    &prvTgfPacketInfo, prvTgfBurstCount, 0, NULL,
                    prvTgfDevNum,
                    prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
                    TGF_CAPTURE_MODE_MIRRORING_E, 10);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    /* AUTODOC: verify to get Ethernet packet on port 3 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:22 */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        if (portIter == PRV_TGF_SEND_PORT_IDX_CNS)
        {
            packetsCountTx = (GT_U8)prvTgfBurstCount;
            packetsCountRx = (GT_U8)prvTgfBurstCount;
            packetSize     = PRV_TGF_TRANSMITTED_PACKET_LEN_CNS;
        }
        else if (portIter == PRV_TGF_RECEIVE_PORT_IDX_CNS)
        {
            packetsCountTx = (GT_U8)prvTgfBurstCount;
            packetsCountRx = (GT_U8)prvTgfBurstCount;
            packetSize     = PRV_TGF_EXPECTED_PACKET_LEN_CNS;
        }
        else
        {
            packetsCountRx = 0;
            packetsCountTx = 0;
            packetSize = 0;
        }

        /* check ETH counters */
        rc = prvTgfEthCountersCheck(prvTgfDevNum,
                                    prvTgfPortsArray[portIter],
                                    packetsCountRx,
                                    packetsCountTx,
                                    packetSize,
                                    prvTgfBurstCount);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
    }

    /* -------------------------------------------------------------------------
     * 4. Get Trigger Counters
     */

    /* setup receive portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];

    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

    /* get trigger counters */

    /* check TRILL da mac */
    vfdArray[vfdIndex].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[vfdIndex].modeExtraInfo = 0;
    vfdArray[vfdIndex].offset     = 0;
    vfdArray[vfdIndex].cycleCount = sizeof(TGF_MAC_ADDR);
    cpssOsMemCpy(vfdArray[vfdIndex].patternPtr, prvTgfPacketL2PartInt.daMac, sizeof(TGF_MAC_ADDR));
    vfdIndex++;

    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface,
                                vfdIndex, vfdArray, &triggerBitmap);
    PRV_UTF_LOG2_MAC("triggerBitmap = %d, rc = 0x%02X\n\n", triggerBitmap, rc);

    /* check TriggerCounters */
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                    "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                    portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

    for(packetIter = 0; packetIter < prvTgfBurstCount; packetIter++)
    {
        /* check if captured packet has all correct vfd's */
        UTF_VERIFY_EQUAL1_STRING_MAC((1 << vfdIndex) - 1,
                                     triggerBitmap & ((1 << vfdIndex) - 1),
                                     "   Errors while triggers summary result checking: packetIter =  %d\n", packetIter);

        /* check if captured packet has expected TRILL passenger da mac */
        UTF_VERIFY_EQUAL6_STRING_MAC(GT_TRUE, triggerBitmap & BIT_0,
                                     "\n   TRILL MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X",
                                     prvTgfPacketL2PartInt.daMac[0],
                                     prvTgfPacketL2PartInt.daMac[1],
                                     prvTgfPacketL2PartInt.daMac[2],
                                     prvTgfPacketL2PartInt.daMac[3],
                                     prvTgfPacketL2PartInt.daMac[4],
                                     prvTgfPacketL2PartInt.daMac[5]);


        /* prepare next iteration */
        triggerBitmap = triggerBitmap >> vfdIndex;
    }
}

/**
* @internal prvTgfTunnelTrillKnownUcDataToTtiAllTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfTunnelTrillKnownUcDataToTtiAllTrafficGenerate
(
    GT_VOID
)
{

    prvTgfTunnelTrillKnownUcDataToTtiWithDsaTrafficGenerate();

    prvTgfTunnelTrillKnownUcDataToTtiTrafficGenerate();

}
/**
* @internal prvTgfTunnelTrillKnownUcDataToTtiConfigRestore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfTunnelTrillKnownUcDataToTtiConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    PRV_TGF_TTI_MAC_VLAN_STC                    macToMePattern;
    PRV_TGF_TTI_MAC_VLAN_STC                    macToMeMask;

    /* AUTODOC: RESTORE CONFIGURATION: */
    /* full mask for VLAN and MAC */
    macToMeMask.mac.arEther[0] =
    macToMeMask.mac.arEther[1] =
    macToMeMask.mac.arEther[2] =
    macToMeMask.mac.arEther[3] =
    macToMeMask.mac.arEther[4] =
    macToMeMask.mac.arEther[5] = 0xFF;
    macToMeMask.vlanId = 0xFFF;

    /* use 0 MAC DA and VID = 4095 */
    macToMePattern.mac.arEther[0] =
    macToMePattern.mac.arEther[1] =
    macToMePattern.mac.arEther[2] =
    macToMePattern.mac.arEther[3] =
    macToMePattern.mac.arEther[4] =
    macToMePattern.mac.arEther[5] = 0;
    macToMePattern.vlanId = 0xFFF;

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_LOG_VERIFY_GT_OK(rc, tgfTrafficTableRxPcktTblClear);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgFdbFlush);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_ETHERNET_VLANID_CNS);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgDefVlanEntryInvalidate);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_TRILL_VLANID_CNS);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgDefVlanEntryInvalidate);

    /* AUTODOC: restore VLAN egress filtering configuration */
    rc = prvTgfBrgVlanEgressFilteringEnable(prvTgfRestoreCfg.vlanEgressFilteringState);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressFilteringEnable);

    /* AUTODOC: restore tag state mode for egress ePort 700 */
    rc = prvTgfBrgVlanEgressPortTagStateModeSet(prvTgfDevNum,
                                                PRV_TGF_SOURCE_EPORT_CNS,
                                                prvTgfRestoreCfg.vlanEgressPortTagStateMode);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressPortTagStateModeSet);

    /* AUTODOC: restore port tag state for egress ePort 700 */
    rc = prvTgfBrgVlanEgressPortTagStateSet(prvTgfDevNum,
                                            PRV_TGF_SOURCE_EPORT_CNS,
                                            prvTgfRestoreCfg.vlanEgressPortTagState);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressPortTagStateSet);

    /* AUTODOC: restore TRILL ethertype 0x22F3 */
    rc = prvTgfTtiEthernetTypeSet(prvTgfDevNum, PRV_TGF_TUNNEL_ETHERTYPE_TYPE_TRILL_E, prvTgfRestoreCfg.trillEtherType);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiEthernetTypeSet);

    /* AUTODOC: restore TRILL engine state on port 0 */
    rc = prvTgfTtiPortTrillEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], prvTgfRestoreCfg.trillEnable);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiPortTrillEnableSet);

    /* AUTODOC: disable TTI lookup for port 0, key TTI_KEY_TRILL */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_TRILL_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* AUTODOC: invalidate TTI rule 8 */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_INDEX_CNS, GT_FALSE);

    /* AUTODOC: restore Outer Tag0 VID for all TRILL packets from port 0 */
    rc = prvTgfTtiPortTrillOuterVid0Set(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], prvTgfRestoreCfg.trillOuterVid0);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiPortTrillOuterVid0Set);

    /* AUTODOC: restore default of MAC to ME entry 1 */
    rc = prvTgfTtiMacToMeSet(PRV_TGF_MAC_TO_ME_INDEX_CNS, &macToMePattern, &macToMeMask);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTtiMacToMeSet);

    /* AUTODOC: disable local source port=0 assignment from DSA tag, used for centralized chassis */
    rc = prvTgfCscdCentralizedChassisModeEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfCscdCentralizedChassisModeEnableSet);

    /* AUTODOC: configure default ePort mapping assignment on ingress centralized chassisenabled ports, */
    /* AUTODOC: when packets are received from line-card port/trunk accordingly. */
    /* AUTODOC: port=0, portBaseEport=0, trunkBaseEport=0 */
    rc = prvTgfCscdCentralizedChassisLineCardDefaultEportBaseSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], 0, 0);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfCscdCentralizedChassisLineCardDefaultEportBaseSet);

    /* AUTODOC: configure the amount of least significant bits taken from DSA tag for assigning a default source ePort on CC ports, */
    /* AUTODOC: for packets received from line-card device trunks/physical ports accordingly. */
    /* AUTODOC: srcTrunkLsbAmount=7, srcPortLsbAmount=6, srcDevLsbAmount=5 */
    rc = prvTgfCscdCentralizedChassisMappingDsaSrcLsbAmountSet(7, 6, 5);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfCscdCentralizedChassisMappingDsaSrcLsbAmountSet);

    /* AUTODOC: Set Port=0 default Source ePort number=0 */
    rc = prvTgfCfgPortDefaultSourceEportNumberSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], 0);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfCfgPortDefaultSourceEportNumberSet);

     /* AUTODOC: Disable packet trace */
     rc = tgfTrafficTracePacketByteSet(GT_FALSE);
     UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

}


