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
* @file prvTgfBasicIpv4UcRouting.c
*
* @brief Basic IPV4 UC Routing
*
* @version   60
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
#include <common/tgfTrunkGen.h>
#include <ip/prvTgfBasicIpv4UcRouting.h>
#include <trunk/prvTgfTrunk.h>
#include <common/tgfPacketGen.h>
#include <common/tgfCscdGen.h>
#include <common/tgfConfigGen.h>
#include <gtOs/gtOsMem.h>

#if (defined CHX_FAMILY)
/* indication for Galtis wrappers call the WA 'B' APIs */
extern GT_U32    appDemo_trunk_phy1690_WA_B;
extern GT_STATUS trunkEPortGet_phy1690_WA_B(
    IN GT_TRUNK_ID              trunkId,
    OUT GT_PORT_NUM              *globalEPortPtr
);
extern void prvTgfCaptureForceTtiDisableModeSet(
    IN GT_BOOL                  forceTtiLookupUnchanged
);
extern GT_STATUS prvTgfDxChPclCaptureSet
(
    IN GT_U8    devNum,
    IN GT_PORT_NUM   portNum,
    IN GT_BOOL  start
);

#else
static GT_U32   appDemo_trunk_phy1690_WA_B = 0;
static GT_STATUS trunkEPortGet_phy1690_WA_B(
    IN GT_TRUNK_ID              trunkId,
    OUT GT_PORT_NUM              *globalEPortPtr
)
{
    *globalEPortPtr = 0;
    return GT_OK
}
static void prvTgfCaptureForceTtiDisableModeSet(
    IN GT_BOOL                  forceTtiLookupUnchanged
)
{
    return;
}
GT_STATUS prvTgfDxChPclCaptureSet
(
    IN GT_U8    devNum,
    IN GT_PORT_NUM   portNum,
    IN GT_BOOL  start
)
{
    return GT_OK
}
#endif

extern GT_STATUS appDemoHwAccessFailerBind(
    IN GT_BOOL                  bind,
    GT_U32                      failCounter
);

extern GT_STATUS appDemoHwAccessCounterGet(
    OUT GT_U32   *hwAccessCounterPtr
);

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* indication that this is stacking system */
static PRV_TGF_IPV4_STACKING_SYSTEM_ENT tgfStackingSystemType = PRV_TGF_IPV4_STACKING_SYSTEM_NONE_E;

#define DSA_SRC_DEV_CNS 0x1a
#define DSA_TRG_DEV_CNS 0x16
#define DSA_TRG_PORT_CNS 0x13
#define DSA_TAG_ORIG_SRC_PHY_PORT_CNS 0x13

/* the target device to send the routed packet to over the stack */
static GT_HW_DEV_NUM    prvTgfDsaTargetHwDevNum    = DSA_TRG_DEV_CNS;
static GT_PORT_NUM      prvTgfDsaTargetPortNum   = DSA_TRG_PORT_CNS;
static GT_PHYSICAL_PORT_NUM prvTgfCascadeEgressPortNum = 0;
#define ROUTER_RESERVED_PORT_NUM 61

typedef enum{
    /* route back to the origin device */
    CASCADE_ROUTE_TRG_DEV_MODE_BACK_TO_SRC_E,
    /* route to a device that is not 'me' and is not 'Origin device'*/
    CASCADE_ROUTE_TRG_DEV_MODE_OTHER_E
}CASCADE_ROUTE_TRG_DEV_MODE_ENT;
static CASCADE_ROUTE_TRG_DEV_MODE_ENT cascadeRouteTrgDevMode;

typedef enum{
    /* after route the DSA state that packet came from 'me' and not from 'Origin device' */
    /* with 'Special src port' number (61) */
    CASCADE_ROUTE_SRC_DEV_MODE_MODIFY_E,

    /* after route the DSA state still state that came from 'Origin device' */
    /* with 'Origin src port' number */
    CASCADE_ROUTE_SRC_DEV_MODE_NOT_MODIFY_E
}CASCADE_ROUTE_SRC_DEV_MODE_ENT;
static CASCADE_ROUTE_SRC_DEV_MODE_ENT cascadeRouteSrcDevMode;

static CPSS_CSCD_LINK_TYPE_STC              origCascadeLink[2];
static PRV_TGF_CSCD_TRUNK_LINK_HASH_ENT     origSrcPortTrunkHashEn[2];
static GT_BOOL                              origRemotePhysicalPortMappingEnablingStatus;
static GT_HW_DEV_NUM                        origRemotePhysicalHwDevNum;
static GT_PHYSICAL_PORT_NUM                 origRemotePhysicalPortNum;
static GT_BOOL                              remotePhysicalPortMapWasConfigured = GT_FALSE;

static GT_U32 byteNumMaskList[] = { 23, 24 }; /* list of bytes for which the comparison is prohibited
                                                 byte 24 bit 1 <Src Tag0 Is Outer Tag> - this field is not relevant
                                                 in the eDSA forward tag because <Tag1 Src Tagged> is 0 */

/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS           5

/* nextHop VLAN Id */
#define PRV_TGF_NEXTHOPE_VLANID_CNS       6

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS            4

/* the counter set for a route entry is linked to */
#define PRV_TGF_COUNT_SET_CNS     CPSS_IP_CNT_SET0_E

/* default number of packets to send */
#define PRV_TGF_BURST_COUNT_DEFAULT_CNS     1

/* nextHop VLAN Id  */
static GT_U16 prvTgfNextHopeVlanid = PRV_TGF_NEXTHOPE_VLANID_CNS;

/* if firstCallToLpmConfig= GT_TRUE then save default status
   of bypassing lookup stages 8-31 */
static GT_BOOL       firstCallToLpmConfig=GT_TRUE;

/* default number of packets to send */
static GT_U32        prvTgfBurstCount   = PRV_TGF_BURST_COUNT_DEFAULT_CNS;

/* VLANs array */
static GT_U16         prvTgfVlanArray[] = {PRV_TGF_SEND_VLANID_CNS, PRV_TGF_NEXTHOPE_VLANID_CNS};

/* the Arp Address index of the Router ARP Table
   to write to the UC Route entry Arp nextHopARPPointer field */
static GT_U32        prvTgfRouterArpIndex      = 1;

/* the Route entry index for UC Route entry Table */
static GT_U32        prvTgfRouteEntryBaseIndex = 3;

/* the LPM DB id for LPM Table */
static GT_U32        prvTgfLpmDBId             = 0;

/* the ARP MAC address to write to the Router ARP Table */
static TGF_MAC_ADDR  prvTgfArpMac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x22};
static TGF_MAC_ADDR  prvTgfArpMac1 = {0x00, 0x00, 0x00, 0x00, 0x00, 0x01};

/* use ePort in the test */
static GT_BOOL       useEPort = GT_FALSE;
/* use ePort mapped to VIDX in the test */
static GT_BOOL       useEPortMappedToVidx = GT_FALSE;


/* value of MacSa */
static TGF_MAC_ADDR prvTgfVlanMacSaArr={0x00, 0xA1, 0xB2, 0xC3, 0xD4, 0xE5};

/* value of MacSa */
static TGF_MAC_ADDR prvTgfVidMacSaArr={0x00, 0xA1, 0xB2, 0xC3, 0xD0, 0x06};

/* value of MacSa */
static TGF_MAC_ADDR prvTgfGlobalMacSaArr={0x00, 0x00, 0x00, 0x00, 0x00, 0x01};

static PRV_TGF_IP_UC_ROUTE_ENTRY_STC           routeEntriesArrayGet0[1];
static PRV_TGF_IP_UC_ROUTE_ENTRY_STC           routeEntriesArrayGet1[1];

/* destination ePort */
#define PRV_TGF_DESTINATION_EPORT_CNS   (UTF_CPSS_PP_MAX_PORT_NUM_CNS(prvTgfDevNum)  - 5)

/* VIDX that represents the EPort*/
#define PRV_TGF_VIDX_FROM_EPORT_CNS   (3333 % (PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_TABLE_VIDX_MAC(prvTgfDevNum)+1))


#define PRV_UTF_LOG_VERIFY_GT_OK(rc, name)                                   \
    if (GT_OK != (rc))                                                       \
    {                                                                        \
        PRV_UTF_LOG2_MAC("\n[TGF]: %s FAILED, rc = [%d]\n", name, rc);       \
    }

/* mac SA index in global mac sa table */
#define PRV_TGF_MAC_SA_GLOBAL_INDEX_CNS          7

/* egress port number to receive traffic from */
#define PRV_TGF_EGR_PORT_IDX_CNS        3

/* sip6 supports 12 bits , sip4,5 supports only 8 */
static GT_U32   macSaLsb = 0xAE5;
/* value of MacSa LSB*/
#define PRV_TGF_MAC_SA_LSB_CNS macSaLsb

/* value of Base MacSa */
#define PRV_TGF_MAC_SA_BASE_CNS {0x00, 0xA1, 0xB2, 0xC3, 0xD4, 0x00}

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}                /* saMac */
};
/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_SEND_VLANID_CNS                       /* pri, cfi, VlanId */
};
/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};
/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2A,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0x04,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,             /* csum */
    { 1,  1,  1,  1},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};
/* packet's IPv4 - used for bulk tests */
static TGF_PACKET_IPV4_STC prvTgfPacket2Ipv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2A,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0x04,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,  /* csum */
    { 1,  1,  1,  1},   /* srcAddr */
    { 3,  2,  2,  3}    /* dstAddr */
};
/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

#define VLAN_TAG_INFO_INDEX  1  /* index in prvTgfPacketPartArray[],prvTgfPacket2PartArray[]*/
#define DSA_INFO_INDEX  2       /* index in prvTgfPacketPartArray[],prvTgfPacket2PartArray[]*/

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_SKIP_E ,      NULL},/*reserved for DSA tag (instead of the vlan tag)*/
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* PARTS of packet - used for bulk tests */
static TGF_PACKET_PART_STC prvTgfPacket2PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_SKIP_E ,      NULL},/*reserved for DSA tag (instead of the vlan tag)*/
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacket2Ipv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/*capture type*/
static TGF_CAPTURE_MODE_ENT captureType = TGF_CAPTURE_MODE_MIRRORING_E;

/* parameters that is needed to be restored */
static struct
{
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC            egressInfo;
    CPSS_INTERFACE_INFO_STC                     physicalInfo;
    PRV_TGF_INLIF_PORT_MODE_ENT                 sendPortInlifMode;
    GT_BOOL                                     defaultBypassEnable;
    GT_U32                                      globalMacSaIndex;
    GT_BOOL                                     defaultSourceEportSipLookupEnable;
    GT_BOOL                                     defaultSourceEportSipSaEnable;

    GT_ETHERADDR                                macSaBaseOrig;
    GT_BOOL                                     macSaBaseOrigInitialized;

    CPSS_MAC_SA_LSB_MODE_ENT                    macSaModeOrig;
    GT_U32                                      macSaModeOrigInitialized;

    GT_BOOL                                     macSaModifyEnableOrig;
    GT_U32                                      macSaModifyEnableOrigInitialized;
} prvTgfRestoreCfg;
/* parameters that is needed to be restored */

static TGF_DSA_DSA_FORWARD_STC  prvTgfPacketDsaTagPart_forward = {
    PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E /*GT_TRUE*/ , /*srcIsTagged*/ /* set to GT_TRUE for the copy to CPU , to be with vlan tag 0 */
    DSA_SRC_DEV_CNS,/*srcHwDev */
    GT_FALSE,/* srcIsTrunk */
    /*union*/ /* union fields are set in runtime due to 'Big endian' / 'little endian' issues */
    {
        /*trunkId*/
        5/*portNum*/
    },/*source;*/

    13,/*srcId*/

    GT_FALSE,/*egrFilterRegistered*/
    GT_FALSE,/*wasRouted*/
    51,/*qosProfileIndex*/

    /*CPSS_INTERFACE_INFO_STC         dstInterface*/
    {
        CPSS_INTERFACE_PORT_E,/*type*/

        /*struct*/{
            0,/*devNum*/
            15/*portNum*/
        },/*devPort*/

        0,/*trunkId*/
        0, /*vidx*/
        0,/*vlanId*/
        0,/*devNum*/
        0,/*fabricVidx*/
        0 /*index*/
    },/*dstInterface*/
    GT_FALSE,/*isTrgPhyPortValid*/
    0,/*dstEport --> filled in runtime */
    0,/*tag0TpidIndex*/
    GT_FALSE,/*origSrcPhyIsTrunk*/
    /* union */ /* union fields are set in runtime due to 'Big endian' / 'little endian' issues */
    {
        /*trunkId*/
        DSA_TAG_ORIG_SRC_PHY_PORT_CNS/*portNum*/
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
        PRV_TGF_SEND_VLANID_CNS,/*vid*/
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

/* filled in runtime to expect the DSA tag after routing */
static TGF_DSA_DSA_FORWARD_STC  prvTgfPacketDsaTagPart_forward_afterRoute = {
    PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E /*GT_TRUE*/ , /*srcIsTagged*/ /* set to GT_TRUE for the copy to CPU , to be with vlan tag 0 */
    0,/*srcHwDev --> filled in runtime --> 'ownDev' */
    GT_FALSE,/* srcIsTrunk */
    /*union*/
    {
        /*trunkId*/
        ROUTER_RESERVED_PORT_NUM/*portNum */
    },/*source;*/

    0,/*srcId (from the default of the port in L2i unit) */

    GT_TRUE,/*egrFilterRegistered (was found in the FDB)*/
    GT_TRUE,/*wasRouted*/
    0,/*qosProfileIndex due to remap  (from the default of the port in TTI unit) */

    /*CPSS_INTERFACE_INFO_STC         dstInterface*/
    {
        CPSS_INTERFACE_PORT_E,/*type*/

        /*struct*/{
            DSA_TRG_DEV_CNS,/*devNum*/
            DSA_TRG_PORT_CNS/*portNum*/
        },/*devPort*/

        0,/*trunkId*/
        0, /*vidx*/
        0,/*vlanId*/
        0,/*devNum*/
        0,/*fabricVidx*/
        0 /*index*/
    },/*dstInterface*/
    GT_FALSE,/*isTrgPhyPortValid*/
    0,/*dstEport --> filled in runtime */
    0,/*tag0TpidIndex*/
    GT_FALSE,/*origSrcPhyIsTrunk*/
    /* union */
    {
        /*trunkId*/
        DSA_TAG_ORIG_SRC_PHY_PORT_CNS/*portNum*/
    },/*origSrcPhy*/
    GT_FALSE,/*phySrcMcFilterEnable*/
    0, /* hash */
    GT_TRUE /*skipFdbSaLookup*/
};

static TGF_PACKET_DSA_TAG_STC  prvTgfPacketDsaTagPart_afterRoute = {
    TGF_DSA_CMD_FORWARD_E ,/*dsaCommand*/
    TGF_DSA_2_WORD_TYPE_E ,/*dsaType*/

    /*TGF_DSA_DSA_COMMON_STC*/
    {
        0,/*vpt*/
        0,/*cfiBit*/
        PRV_TGF_NEXTHOPE_VLANID_CNS,/*vid after routing*/
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

/*filled in runtime for the expected routed L2 bytes */
static TGF_PACKET_L2_STC  afterRouteL2  = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}                /* saMac */
};


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfBasicIpv4UcRoutingBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*
* @param[in] fdbPortNum               - the FDB port num to set in the macEntry destination Interface
*                                       None
*/
GT_VOID prvTgfBasicIpv4UcRoutingBaseConfigurationSet
(
    GT_U32   fdbPortNum
)
{
    GT_STATUS rc = GT_OK;
    GT_U8     tagArray[] = {0, 0, 1, 1};
    PRV_TGF_BRG_MAC_ENTRY_STC   macEntry;

    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Base Configuration =======\n");

    /* save default VLAN 1 */
    prvTgfBrgVlanEntryStore(1);

    if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum) && tgfStackingSystemType == PRV_TGF_IPV4_STACKING_SYSTEM_NONE_E)
    {
        /* NOTE : the vid used also in other files (but without similar logic) as
            'HARD CODED' : 'PRV_TGF_NEXTHOPE_VLANID_CNS' ,
            so prvTgfNextHopeVlanid here must be +4K (or +0) value from 'PRV_TGF_NEXTHOPE_VLANID_CNS'

            meaning that we can not decide to work with any prvTgfNextHopeVlanid value
            for eArch devices !
        */
        prvTgfNextHopeVlanid =
            ALIGN_EVID_TO_4K_TILL_MAX_DEV_MAC(PRV_TGF_NEXTHOPE_VLANID_CNS + _4K);
    }
    else
    {
        prvTgfNextHopeVlanid = PRV_TGF_NEXTHOPE_VLANID_CNS;
    }

    /* update next hop VLAN ID according to eArch */
    prvTgfVlanArray[1] = prvTgfNextHopeVlanid;

    if(tgfStackingSystemType != PRV_TGF_IPV4_STACKING_SYSTEM_AFTER_ROUTING_DEVICE_E)
    {
        /* AUTODOC: create VLAN 5 with untagged ports 0,1 */
        rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_VLANID_CNS,
                prvTgfPortsArray, NULL, tagArray, 2);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);
    }

    /* AUTODOC: create VLAN 6 with tagged ports 2,3 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(prvTgfNextHopeVlanid,
            prvTgfPortsArray + 2, NULL, tagArray + 2, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    if(tgfStackingSystemType != PRV_TGF_IPV4_STACKING_SYSTEM_AFTER_ROUTING_DEVICE_E)
    {
        /* AUTODOC: enable VLAN based MAC learning */
        rc = prvTgfBrgVlanLearningStateSet(PRV_TGF_SEND_VLANID_CNS, GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanLearningStateSet: %d", prvTgfDevNum);

        /* -------------------------------------------------------------------------
         * Set the FDB entry With DA_ROUTE
         */

        /* create a macEntry with .daRoute = GT_TRUE */
        cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
        cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

        macEntry.key.entryType                  = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
        macEntry.key.key.macVlan.vlanId         = PRV_TGF_SEND_VLANID_CNS;
        macEntry.dstInterface.type              = CPSS_INTERFACE_PORT_E;
        macEntry.dstInterface.devPort.hwDevNum    = prvTgfDevNum;
        macEntry.dstInterface.devPort.portNum   = fdbPortNum;
        macEntry.isStatic                       = GT_TRUE;
        macEntry.daCommand                      = PRV_TGF_PACKET_CMD_FORWARD_E;
        macEntry.saCommand                      = PRV_TGF_PACKET_CMD_FORWARD_E;
        macEntry.daRoute                        = GT_TRUE;
        macEntry.sourceId                       = 0;
        macEntry.userDefined                    = 0;
        macEntry.daQosIndex                     = 0;
        macEntry.saQosIndex                     = 0;
        macEntry.daSecurityLevel                = 0;
        macEntry.saSecurityLevel                = 0;
        macEntry.appSpecificCpuCode             = GT_FALSE;
        macEntry.mirrorToRxAnalyzerPortEn       = GT_FALSE;

        /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 5, port 1 */
        rc = prvTgfBrgFdbMacEntrySet(&macEntry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryWrite: %d", prvTgfDevNum);
    }
}

/**
* @internal prvTgfBasicIpv4UcRoutingLttRouteConfigurationSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
*                                      sendPortNum         - port sending traffic
* @param[in] nextHopPortNum           - port receiving traffic
* @param[in] useBulk                  - whether to use bulk operation to add prefixes
* @param[in] enableIpv4McRouting      - whether to enable IPv4 MC routing in the virtual router
* @param[in] enableIpv6McRouting      - whether to enable IPv6 MC routing in the virtual router
*                                       None
*/
static GT_VOID prvTgfBasicIpv4UcRoutingLttRouteConfigurationSet
(
    GT_U32   prvUtfVrfId,
    GT_U8    sendPortIndex,
    GT_U32    nextHopPortNum,
    GT_BOOL  useBulk,
    GT_BOOL  enableIpv4McRouting,
    GT_BOOL  enableIpv6McRouting
)
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_INLIF_ENTRY_STC                 inlifEntry;
    GT_ETHERADDR                            arpMacAddr;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           routeEntriesArray[1];
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           *regularEntryPtr;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_IPADDR                               ipAddr;
    GT_U32                                  bulkVrId[2];
    GT_IPADDR                               bulkIpAddr[2];
    GT_U32                                  bulkPrefixLen[2];
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    bulkNextHopInfo[2];
    GT_BOOL                                 bulkOverride[2];
    GT_TRUNK_ID senderTrunkId;/* trunk Id for the sender port that is member of */
    GT_TRUNK_ID nextHopTrunkId;/* trunk Id for the next hop port that is member of */
    CPSS_TRUNK_MEMBER_STC   trunkMember;/* temporary trunk member */
    GT_U32                                  numOfEnabledMembers;
    CPSS_TRUNK_MEMBER_STC   enabledMembersArray[PRV_CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS];
    GT_U32                                  ii;
    GT_U8                                   prvSendPortIndex;
    CPSS_INTERFACE_INFO_STC                 physicalInfo;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC        egressInfo;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    defIpv4UcRouteEntryInfo;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    defIpv6UcRouteEntryInfo;
    PRV_TGF_IP_LTT_ENTRY_STC                *ipLttEntryPtr = NULL;
    PRV_TGF_IP_LTT_ENTRY_STC                *defIpv4McLttPtr = NULL;
    PRV_TGF_IP_LTT_ENTRY_STC                *defIpv6McLttPtr = NULL;
    GT_U32                                  numOfPaths;
    GT_HW_DEV_NUM                           targetHwDevice;
    GT_PORT_NUM                             targetPort;

    PRV_TGF_PORT_INDEX_CHECK_FOR_VOID_FUNC_MAC(sendPortIndex);

    PRV_UTF_LOG0_MAC("==== Setting LTT Route Configuration ====\n");

    if(tgfStackingSystemType != PRV_TGF_IPV4_STACKING_SYSTEM_NONE_E)
    {
        targetHwDevice = prvTgfDsaTargetHwDevNum;
        targetPort   = prvTgfDsaTargetPortNum;
        PRV_UTF_LOG2_MAC("do route to remote device[0x%x] , remote port [0x%x]\n",
            targetHwDevice,targetPort);
    }
    else
    {
        targetHwDevice = prvTgfDevNum;
        targetPort = nextHopPortNum;
    }

    if (useEPort == GT_TRUE)
    {
        /* save ePort attributes configuration */
        rc = prvTgfBrgEportToPhysicalPortEgressPortInfoGet(prvTgfDevNum,
                                                           PRV_TGF_DESTINATION_EPORT_CNS,
                                                           &(prvTgfRestoreCfg.egressInfo));
        PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoGet);

        /* set ePort attributes configuration */
        cpssOsMemSet(&egressInfo,0,sizeof(egressInfo));
        rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                                           PRV_TGF_DESTINATION_EPORT_CNS,
                                                           &egressInfo);
        PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoSet);

        /* save ePort mapping configuration */
        rc = prvTgfBrgEportToPhysicalPortTargetMappingTableGet(prvTgfDevNum,
                                                               PRV_TGF_DESTINATION_EPORT_CNS,
                                                               &(prvTgfRestoreCfg.physicalInfo));
        PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortTargetMappingTableGet);

        /* set ePort mapping configuration */
        physicalInfo.type = CPSS_INTERFACE_PORT_E;

        physicalInfo.devPort.hwDevNum = targetHwDevice;
        physicalInfo.devPort.portNum = targetPort;
        rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                               PRV_TGF_DESTINATION_EPORT_CNS,
                                                               &physicalInfo);
        PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortTargetMappingTableSet);
    }
    /* -------------------------------------------------------------------------
     * 0. Set Inlif Entry - that needs for Puma devices
     */

    /* save current port inlif mode */
    rc = prvTgfInlifPortModeGet(sendPortIndex,&(prvTgfRestoreCfg.sendPortInlifMode));
    PRV_UTF_LOG_VERIFY_GT_OK(rc,prvTgfInlifPortModeGet);
    /* set port inlif mode */
    rc = prvTgfInlifPortModeSet(sendPortIndex,PRV_TGF_INLIF_PORT_MODE_PORT_E);
    PRV_UTF_LOG_VERIFY_GT_OK(rc,prvTgfInlifPortModeSet);

    cpssOsMemSet(&inlifEntry,0,sizeof(inlifEntry));
    inlifEntry.bridgeEnable                   = GT_TRUE;
    inlifEntry.unkSaUcCommand                 = CPSS_PACKET_CMD_BRIDGE_E;
    inlifEntry.bridgeRouterInterfaceEnable    = GT_FALSE;
    inlifEntry.ipv4UcRouteEnable              = GT_TRUE;
    inlifEntry.vrfId                          = prvUtfVrfId;

    rc = prvTgfInlifEntrySet(PRV_TGF_INLIF_TYPE_PORT_E,sendPortIndex,&inlifEntry);
    PRV_UTF_LOG_VERIFY_GT_OK(rc,prvTgfInlifEntrySet);

    /* -------------------------------------------------------------------------
     * 1. Enable Routing
     */


    /* Routing from trunk support:
       check if the port is trunk member.
       when trunk member , set all the members with same configuration.
    */
    trunkMember.port = prvTgfPortsArray[sendPortIndex];
    trunkMember.hwDevice = prvTgfDevNum;

    rc = prvTgfTrunkDbIsMemberOfTrunk(prvTgfDevNum,&trunkMember,&senderTrunkId);
    if(rc == GT_OK && appDemo_trunk_phy1690_WA_B == 0)
    {
        /* the port is member of the trunk */

        /* get all trunk members */
        numOfEnabledMembers = PRV_CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS;
        rc = prvCpssGenericTrunkDbEnabledMembersGet(prvTgfDevNum,senderTrunkId,&numOfEnabledMembers,enabledMembersArray);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvCpssGenericTrunkDbEnabledMembersGet: %d %d",
                                     prvTgfDevNum, senderTrunkId);
        if(numOfEnabledMembers == 0)
        {
            /* empty trunk ??? */
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, GT_EMPTY);
            return;
        }

        for(ii = 0 ; ii < numOfEnabledMembers; ii++)
        {
            /* enable Unicast IPv4 Routing on the trunk members */

            rc = prvTgfDefPortsArrayPortToPortIndex(enabledMembersArray[ii].port,&prvSendPortIndex);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfDefPortsArrayPortToPortIndex: %d", prvSendPortIndex);

            rc = prvTgfIpPortRoutingEnable(prvSendPortIndex, CPSS_IP_UNICAST_E,
                                           CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                         enabledMembersArray[ii].hwDevice, enabledMembersArray[ii].port);
        }
    }
    else
    if(rc == GT_OK && appDemo_trunk_phy1690_WA_B)
    {
        GT_PORT_NUM eportOfTrunk;

        rc = trunkEPortGet_phy1690_WA_B(senderTrunkId,&eportOfTrunk);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "trunkEPortGet_phy1690_WA_B: prvTgfDevNum %d senderTrunkId[%d]",
                                     prvTgfDevNum, senderTrunkId);

        /* in this WA we get srcEPort that represents the trunkId ..
           so need to enable it's routing */
#ifdef CHX_FAMILY
    /* call device specific API */
        rc = cpssDxChIpPortRoutingEnable(prvTgfDevNum, eportOfTrunk, CPSS_IP_UNICAST_E,
                                           CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
#endif /* CHX_FAMILY */
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChIpPortRoutingEnable: prvTgfDevNum %d eportOfTrunk %d",
                                     prvTgfDevNum, eportOfTrunk);
    }
    else
    {
        /* AUTODOC: enable Unicast IPv4 Routing on port 0 */
        rc = prvTgfIpPortRoutingEnable(sendPortIndex, CPSS_IP_UNICAST_E,
                                       CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                     prvTgfDevNum, sendPortIndex);
    }

    /* AUTODOC: update VRF Id in vlan[PRV_TGF_SEND_VLANID_CNS] */
    rc = prvTgfBrgVlanVrfIdSet(PRV_TGF_SEND_VLANID_CNS, prvUtfVrfId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanVrfIdSet: %d %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* AUTODOC: enable IPv4 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* -------------------------------------------------------------------------
     * 2. Create the Route entry (Next hop) in Route table and Router ARP Table
     */

    /* AUTODOC: write a ARP MAC 00:00:00:00:00:22 to the Router ARP Table */
    cpssOsMemCpy(arpMacAddr.arEther, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

    rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterArpIndex, &arpMacAddr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    /* write a UC Route entry to the Route Table */
    cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));

    regularEntryPtr = &routeEntriesArray[0];
    regularEntryPtr->cmd                        = CPSS_PACKET_CMD_ROUTE_E;
    regularEntryPtr->cpuCodeIndex               = 0;
    regularEntryPtr->appSpecificCpuCodeEnable   = GT_FALSE;
    regularEntryPtr->unicastPacketSipFilterEnable = GT_FALSE;
    regularEntryPtr->ttlHopLimitDecEnable       = GT_FALSE;
    regularEntryPtr->ttlHopLimDecOptionsExtChkByPass = GT_FALSE;
    regularEntryPtr->ingressMirror              = GT_FALSE;
    regularEntryPtr->qosProfileMarkingEnable    = GT_FALSE;
    regularEntryPtr->qosProfileIndex            = 0;
    regularEntryPtr->qosPrecedence              = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    regularEntryPtr->modifyUp                   = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    regularEntryPtr->modifyDscp                 = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    regularEntryPtr->countSet                   = PRV_TGF_COUNT_SET_CNS;
    regularEntryPtr->trapMirrorArpBcEnable      = GT_FALSE;
    regularEntryPtr->sipAccessLevel             = 0;
    regularEntryPtr->dipAccessLevel             = 0;
    regularEntryPtr->ICMPRedirectEnable         = GT_FALSE;
    regularEntryPtr->scopeCheckingEnable        = GT_FALSE;
    regularEntryPtr->siteId                     = CPSS_IP_SITE_ID_INTERNAL_E;
    regularEntryPtr->mtuProfileIndex            = 0;
    regularEntryPtr->isTunnelStart              = GT_FALSE;
    regularEntryPtr->nextHopVlanId              = prvTgfNextHopeVlanid;

    if(useEPortMappedToVidx == GT_FALSE)
    {
        /* Routing to trunk support:
           check if the NH port is trunk member.
           when trunk member --> use the trunk ID.
        */
        trunkMember.port = targetPort;
        trunkMember.hwDevice = targetHwDevice;

        rc = prvTgfTrunkDbIsMemberOfTrunk(prvTgfDevNum,&trunkMember,&nextHopTrunkId);
        if(rc == GT_OK && appDemo_trunk_phy1690_WA_B == 0)
        {
            /* the port is member of the trunk */
            regularEntryPtr->nextHopInterface.type      = CPSS_INTERFACE_TRUNK_E;
            regularEntryPtr->nextHopInterface.trunkId = nextHopTrunkId;
        }
        else
        if(rc == GT_OK && appDemo_trunk_phy1690_WA_B)
        {
            GT_PORT_NUM eportOfTrunk;

            rc = trunkEPortGet_phy1690_WA_B(nextHopTrunkId,&eportOfTrunk);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "trunkEPortGet_phy1690_WA_B: prvTgfDevNum %d nextHopTrunkId[%d]",
                                         prvTgfDevNum, nextHopTrunkId);

            regularEntryPtr->nextHopInterface.type      = CPSS_INTERFACE_PORT_E;
            regularEntryPtr->nextHopInterface.devPort.hwDevNum = targetHwDevice;
            regularEntryPtr->nextHopInterface.devPort.portNum = eportOfTrunk;
        }
        else
        {
            regularEntryPtr->nextHopInterface.type      = CPSS_INTERFACE_PORT_E;
            regularEntryPtr->nextHopInterface.devPort.hwDevNum = targetHwDevice;

            if (useEPort == GT_TRUE)
                regularEntryPtr->nextHopInterface.devPort.portNum = PRV_TGF_DESTINATION_EPORT_CNS;
            else
                regularEntryPtr->nextHopInterface.devPort.portNum = targetPort;
        }
    }
    else /* useEPortMappedToVidx == GT_TRUE */
    {
        /* Routing to EPort that represents VIDX from the E2PHY */
        regularEntryPtr->nextHopInterface.type      = CPSS_INTERFACE_PORT_E;
        regularEntryPtr->nextHopInterface.devPort.hwDevNum = targetHwDevice;
        regularEntryPtr->nextHopInterface.devPort.portNum = PRV_TGF_DESTINATION_EPORT_CNS;
    }
    regularEntryPtr->nextHopARPPointer          = prvTgfRouterArpIndex;
    regularEntryPtr->nextHopTunnelPointer       = 0;

    /* AUTODOC: add UC route entry with nexthop VLAN 6 and nexthop port 3 */
    rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndex, routeEntriesArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);


    /* AUTODOC: read and check the UC Route entry from the Route Table */
    cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));

    rc = prvTgfIpUcRouteEntriesRead(prvTgfDevNum, prvTgfRouteEntryBaseIndex, routeEntriesArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesRead: %d", prvTgfDevNum);

    PRV_UTF_LOG2_MAC("nextHopVlanId = %d, portNum = %d\n",
                     routeEntriesArray[0].nextHopVlanId,
                     routeEntriesArray[0].nextHopInterface.devPort.portNum);

    /* -------------------------------------------------------------------------
     * 3. Create Virtual Router [prvUtfVrfId]
     */

    /* AUTODOC: create Virtual Router [prvUtfVrfId] */

    if (0 != prvUtfVrfId)
    {
        /* clear entry */
        cpssOsMemSet(&defIpv4UcRouteEntryInfo, 0,
                     sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
        cpssOsMemSet(&defIpv6UcRouteEntryInfo, 0,
                         sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));

        /* define max number of paths */
        if(prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_PUMA_E))
        {
            numOfPaths = 0;
        }
        else
        {
            numOfPaths = 1;
        }

        for (ii = 0; ii < 2; ii++)
        {
            if (ii == 0)
            {
                ipLttEntryPtr = &defIpv4UcRouteEntryInfo.ipLttEntry;
            }
            if (ii == 1)
            {
                ipLttEntryPtr = &defIpv6UcRouteEntryInfo.ipLttEntry;
            }
            /* set defUcRouteEntryInfo */
            ipLttEntryPtr->routeType                = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;
            ipLttEntryPtr->numOfPaths               = numOfPaths;
            ipLttEntryPtr->routeEntryBaseIndex      = prvTgfRouteEntryBaseIndex;
            ipLttEntryPtr->ucRPFCheckEnable         = GT_FALSE;
            ipLttEntryPtr->sipSaCheckMismatchEnable = GT_FALSE;
            ipLttEntryPtr->ipv6MCGroupScopeLevel    = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

            if ((ii == 0) && (enableIpv4McRouting == GT_TRUE))
            {
                defIpv4McLttPtr = ipLttEntryPtr;
            }
            if ((ii == 1) && (enableIpv6McRouting == GT_TRUE))
            {
                defIpv6McLttPtr = ipLttEntryPtr;
            }
        }

        /* disable Patricia trie validity */
        prvTgfIpValidityCheckEnable(GT_FALSE);

        /* create Virtual Router [prvUtfVrfId] with created default LTT entries */
        rc = prvTgfIpLpmVirtualRouterAdd(prvTgfLpmDBId, prvUtfVrfId,
                                         &defIpv4UcRouteEntryInfo,
                                         &defIpv6UcRouteEntryInfo,
                                         defIpv4McLttPtr, defIpv6McLttPtr, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                     "prvTgfIpLpmVirtualRouterAdd: %d",
                                     prvTgfDevNum);
    }

    /* -------------------------------------------------------------------------
     * 4. Create a new Ipv4 prefix in a Virtual Router for the specified LPM DB
     */

    /* fill a nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex;
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* AUTODOC: for regular test add IPv4 UC prefix 1.1.1.3/32 */
    /* AUTODOC: for bulk test add bulk of 2 IPv4 UC prefixes: 1.1.1.3/32, 3.2.2.3/32 */
    if (useBulk == GT_FALSE)
    {
        /* fill a destination IP address for the prefix */
        cpssOsMemCpy(ipAddr.arIP, prvTgfPacketIpv4Part.dstAddr, sizeof(ipAddr.arIP));

        rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, prvUtfVrfId,
                                        ipAddr, 32, &nextHopInfo, GT_FALSE, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);
    }
    else
    {
        bulkVrId[0] = prvUtfVrfId;
        bulkVrId[1] = prvUtfVrfId;
        cpssOsMemCpy(&bulkIpAddr[0], prvTgfPacketIpv4Part.dstAddr, sizeof(ipAddr.arIP));
        cpssOsMemCpy(&bulkIpAddr[1], prvTgfPacket2Ipv4Part.dstAddr, sizeof(ipAddr.arIP));
        bulkPrefixLen[0] = bulkPrefixLen[1] = 32;
        cpssOsMemCpy(&bulkNextHopInfo[0], &nextHopInfo, sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
        cpssOsMemCpy(&bulkNextHopInfo[1], &nextHopInfo, sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
        bulkOverride[0] = bulkOverride[1] = GT_TRUE;
        rc = prvTgfIpLpmIpv4UcPrefixBulkAdd(prvTgfLpmDBId, bulkVrId, bulkIpAddr,
                                            bulkPrefixLen, bulkNextHopInfo,
                                            bulkOverride, 2);
    }
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);

}

/**
* @internal prvTgfBasicIpv4UcRoutingPbrConfigurationSet function
* @endinternal
*
* @brief   Set PBR Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
* @param[in] sendPortIndex            - index of the port sending traffic
* @param[in] nextHopPortNum           - port receiving traffic
* @param[in] useBulk                  - whether to use bulk operation to add prefixes
*                                       None
*/
static GT_VOID prvTgfBasicIpv4UcRoutingPbrConfigurationSet
(
    GT_U32   prvUtfVrfId,
    GT_U8    sendPortIndex,
    GT_U32    nextHopPortNum,
    GT_BOOL  useBulk
)
{
    GT_STATUS                               rc = GT_OK;
    GT_ETHERADDR                            arpMacAddr;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_IPADDR                               ipAddr;
    GT_U32                                  bulkVrId[2];
    GT_IPADDR                               bulkIpAddr[2];
    GT_U32                                  bulkPrefixLen[2];
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    bulkNextHopInfo[2];
    GT_BOOL                                 bulkOverride[2];
    GT_TRUNK_ID senderTrunkId;/* trunk Id for the sender port that is member of */
    GT_TRUNK_ID nextHopTrunkId;/* trunk Id for the next hop port that is member of */
    CPSS_TRUNK_MEMBER_STC   trunkMember;/* temporary trunk member */
    GT_U32                  numOfEnabledMembers;
    CPSS_TRUNK_MEMBER_STC   enabledMembersArray[PRV_CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS];
    GT_U32                                  ii;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    *defUcNextHopInfoPtr = NULL;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    defIpv4UcNextHopInfo;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    defIpv6UcNextHopInfo;

    PRV_TGF_PORT_INDEX_CHECK_FOR_VOID_FUNC_MAC(sendPortIndex);

    PRV_UTF_LOG0_MAC("==== Setting PBR Configuration ====\n");

    /* Routing from trunk support:
       check if the port is trunk member.
       when trunk member , set all the members with same configuration.
    */
    trunkMember.port = prvTgfPortsArray[sendPortIndex];
    trunkMember.hwDevice = prvTgfDevNum;

    rc = prvTgfTrunkDbIsMemberOfTrunk(prvTgfDevNum,&trunkMember,&senderTrunkId);
    if(rc == GT_OK)
    {
        /* the port is member of the trunk */

        /* get all trunk members */
        numOfEnabledMembers = PRV_CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS;
        rc = prvCpssGenericTrunkDbEnabledMembersGet(prvTgfDevNum,senderTrunkId,&numOfEnabledMembers,enabledMembersArray);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvCpssGenericTrunkDbEnabledMembersGet: %d %d",
                                     prvTgfDevNum, senderTrunkId);
        if(numOfEnabledMembers == 0)
        {
            /* empty trunk ??? */
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, GT_EMPTY);
            return;
        }

        for(ii = 0 ; ii < numOfEnabledMembers; ii++)
        {
            /* enable Unicast IPv4 Routing on the trunk members */
            rc = prvTgfPclDefPortInitExt1(
                enabledMembersArray[ii].port,
                CPSS_PCL_DIRECTION_INGRESS_E,
                CPSS_PCL_LOOKUP_1_E, /* PBR must be in lookup 1 ! */
                PRV_TGF_PCL_PBR_ID_MAC(prvUtfVrfId), /* pclId */
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/,
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E /*ipv4Key*/,
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E /*ipv6Key*/);
            UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, rc, "prvTgfPclDefPortInitExt1: %d", prvTgfDevNum);
        }
    }
    else
    {
        /* -------------------------------------------------------------------------
         * 1. PCL Config
         */

        /* init PCL Engine for send port */
        rc = prvTgfPclDefPortInitExt1(
            prvTgfPortsArray[sendPortIndex],
            CPSS_PCL_DIRECTION_INGRESS_E,
            CPSS_PCL_LOOKUP_1_E, /* PBR must be in lookup 1 ! */
            PRV_TGF_PCL_PBR_ID_MAC(prvUtfVrfId), /* pclId */
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E /*ipv4Key*/,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E /*ipv6Key*/);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "prvTgfPclDefPortInitExt1: %d", prvTgfDevNum);
    }

    /* -------------------------------------------------------------------------
     * 2. IP Config
     */

    /* update VRF Id in vlan[PRV_TGF_SEND_VLANID_CNS] */
    rc = prvTgfBrgVlanVrfIdSet(PRV_TGF_SEND_VLANID_CNS, prvUtfVrfId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanVrfIdSet: %d %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* enable IPv4 Unicast Routing on Vlan */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* write ARP MAC address to the Router ARP Table */
    cpssOsMemCpy(arpMacAddr.arEther, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

    rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterArpIndex, &arpMacAddr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 3. Create Virtual Router [prvUtfVrfId]
     */

    /* AUTODOC: create Virtual Router [prvUtfVrfId] */
    if (0 != prvUtfVrfId)
    {
        /* set next hop to all 0 and change only values other than 0 */
        cpssOsMemSet(&defIpv4UcNextHopInfo, 0,
                     sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
        cpssOsMemSet(&defIpv6UcNextHopInfo, 0,
                     sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));

        /* ipv4 uc and ipv6 uc default route */
        for (ii = 0; ii < 2; ii++)
        {
            if (ii == 0)
            {
                defUcNextHopInfoPtr = &defIpv4UcNextHopInfo;
                defUcNextHopInfoPtr->pclIpUcAction.ipUcRoute.arpDaIndex =
                                                               prvTgfRouterArpIndex;
            }
            else
            {
                defUcNextHopInfoPtr = &defIpv6UcNextHopInfo;
            }
            defUcNextHopInfoPtr->pclIpUcAction.pktCmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
            defUcNextHopInfoPtr->pclIpUcAction.mirror.cpuCode = CPSS_NET_FIRST_USER_DEFINED_E;
            defUcNextHopInfoPtr->pclIpUcAction.vlan.modifyVlan = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
            defUcNextHopInfoPtr->pclIpUcAction.redirect.redirectCmd = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
            defUcNextHopInfoPtr->pclIpUcAction.ipUcRoute.doIpUcRoute = GT_TRUE;
        }

        /* disable Patricia trie validity */
        prvTgfIpValidityCheckEnable(GT_FALSE);

        rc = prvTgfIpLpmVirtualRouterAdd(prvTgfLpmDBId,
                                         prvUtfVrfId,
                                         &defIpv4UcNextHopInfo,
                                         &defIpv6UcNextHopInfo,
                                         NULL, NULL, NULL);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                     "prvTgfIpLpmVirtualRouterAdd: %d",
                                     prvTgfDevNum);
    }

    /* -------------------------------------------------------------------------
     * 4. Create a new Ipv4 prefix in a Virtual Router for the specified LPM DB
     */

    /* fill a nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));

    nextHopInfo.pclIpUcAction.pktCmd                        = CPSS_PACKET_CMD_FORWARD_E;
    nextHopInfo.pclIpUcAction.mirror.cpuCode                = CPSS_NET_CONTROL_E;
    nextHopInfo.pclIpUcAction.matchCounter.enableMatchCount = GT_TRUE;
    nextHopInfo.pclIpUcAction.vlan.vlanId                   = prvTgfNextHopeVlanid;
    nextHopInfo.pclIpUcAction.ipUcRoute.doIpUcRoute         = GT_TRUE;
    nextHopInfo.pclIpUcAction.ipUcRoute.arpDaIndex          = prvTgfRouterArpIndex;

    nextHopInfo.pclIpUcAction.redirect.redirectCmd          = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
    nextHopInfo.pclIpUcAction.redirect.data.vrfId           = prvUtfVrfId;


    /* Routing to trunk support:
       check if the NH port is trunk member.
       when trunk member --> use the trunk ID.
    */
    trunkMember.port = nextHopPortNum;
    trunkMember.hwDevice = prvTgfDevNum;

    rc = prvTgfTrunkDbIsMemberOfTrunk(prvTgfDevNum,&trunkMember,&nextHopTrunkId);
    if(rc == GT_OK)
    {
        /* the port is member of the trunk */
        nextHopInfo.pclIpUcAction.redirect.data.outIf.outInterface.type     = CPSS_INTERFACE_TRUNK_E;
        nextHopInfo.pclIpUcAction.redirect.data.outIf.outInterface.trunkId  = nextHopTrunkId;
    }
    else
    {
        nextHopInfo.pclIpUcAction.redirect.data.outIf.outInterface.type            = CPSS_INTERFACE_PORT_E;
        nextHopInfo.pclIpUcAction.redirect.data.outIf.outInterface.devPort.hwDevNum  = prvTgfDevNum;
        nextHopInfo.pclIpUcAction.redirect.data.outIf.outInterface.devPort.portNum = nextHopPortNum;
    }

    /* AUTODOC: for regular test add IPv4 UC prefix 1.1.1.3/32 */
    /* AUTODOC: for bulk test add bulk of 2 IPv4 UC prefixes: 1.1.1.3/32, 3.2.2.3/32 */
    /* call CPSS function */
    if (useBulk == GT_FALSE)
    {
        /* fill a destination IP address for the prefix */
        cpssOsMemCpy(ipAddr.arIP, prvTgfPacketIpv4Part.dstAddr, sizeof(ipAddr.arIP));
        rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, prvUtfVrfId,
                                        ipAddr, 32, &nextHopInfo, GT_TRUE, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                     "prvTgfIpLpmIpv4UcPrefixAdd: %d",
                                     prvTgfDevNum);
    }
    else
    {
        bulkVrId[0] = prvUtfVrfId;
        bulkVrId[1] = prvUtfVrfId;
        cpssOsMemCpy(&bulkIpAddr[0], prvTgfPacketIpv4Part.dstAddr, sizeof(ipAddr.arIP));
        cpssOsMemCpy(&bulkIpAddr[1], prvTgfPacket2Ipv4Part.dstAddr, sizeof(ipAddr.arIP));
        bulkPrefixLen[0] = bulkPrefixLen[1] = 32;
        cpssOsMemCpy(&bulkNextHopInfo[0], &nextHopInfo, sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
        cpssOsMemCpy(&bulkNextHopInfo[1], &nextHopInfo, sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
        bulkOverride[0] = bulkOverride[1] = GT_TRUE;
        rc = prvTgfIpLpmIpv4UcPrefixBulkAdd(prvTgfLpmDBId, bulkVrId, bulkIpAddr,
                                            bulkPrefixLen, bulkNextHopInfo,
                                            bulkOverride, 2);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                     "prvTgfIpLpmIpv4UcPrefixBulkAdd: %d",
                                     prvTgfDevNum);
    }
}

/**
* @internal prvTgfBasicIpv4UcRoutingInStackingSystemInit function
* @endinternal
*
* @brief   Set Stacking System Init .
*         set ingress port as 'cascade port' to get traffic with DSA
*         set egress port as 'cascade port' to egress traffic with DSA
* @param[in] sendPortNum              - port sending traffic
* @param[in] nextHopPortNum           - port receiving traffic
*                                       None
*/
static GT_VOID prvTgfBasicIpv4UcRoutingInStackingSystemInit
(
    IN GT_U32    sendPortNum,
    IN GT_U32    nextHopPortNum
)
{
    GT_STATUS   rc;
    CPSS_CSCD_LINK_TYPE_STC      cascadeLink;
    CPSS_CSCD_PORT_TYPE_ENT      cascadePortType;

    prvTgfCascadeEgressPortNum = nextHopPortNum;

    /* AUTODOC: set ingress port as 'cascade port' to get traffic with DSA */
    rc = tgfTrafficGeneratorIngressCscdPortEnableSet(prvTgfDevNum,sendPortNum,GT_TRUE);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    if(tgfStackingSystemType != PRV_TGF_IPV4_STACKING_SYSTEM_AFTER_ROUTING_DEVICE_E)
    {
        if (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
        {
            cascadePortType = CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E;
        }
        else
        {
            cascadePortType = CPSS_CSCD_PORT_DSA_MODE_2_WORDS_E;
        }
        /* AUTODOC: set egress port as 'cascade port' to egress traffic with DSA */
        rc = tgfTrafficGeneratorEgressCscdPortEnableSet(prvTgfDevNum,prvTgfCascadeEgressPortNum,GT_TRUE,cascadePortType);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        /* AUTODOC: set direction egress port on local device to reach the remote target device -- orig device */
        rc = prvTgfCscdDevMapTableGet(prvTgfDevNum,
                                 prvTgfDsaTargetHwDevNum,
                                 prvTgfDsaTargetPortNum,
                                 &origCascadeLink[0],
                                 &origSrcPortTrunkHashEn[0]);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        rc = prvTgfCscdDevMapTableGet(prvTgfDevNum,
                                 DSA_SRC_DEV_CNS,
                                 prvTgfDsaTargetPortNum,
                                 &origCascadeLink[1],
                                 &origSrcPortTrunkHashEn[1]);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        /* AUTODOC: set direction egress port on local device to reach the remote target device */
        cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;
        cascadeLink.linkNum = prvTgfCascadeEgressPortNum;
        rc = prvTgfCscdDevMapTableSet(prvTgfDevNum,
                                 prvTgfDsaTargetHwDevNum,
                                 prvTgfDsaTargetPortNum,
                                 &cascadeLink,
                                 GT_FALSE, GT_FALSE);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        /* AUTODOC: set direction egress port on local device to reach the remote target device -- orig device */
        rc = prvTgfCscdDevMapTableSet(prvTgfDevNum,
                                 DSA_SRC_DEV_CNS,
                                 prvTgfDsaTargetPortNum,
                                 &cascadeLink,
                                 GT_FALSE, GT_FALSE);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        if (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
        {
            rc = prvTgfCscdCentralizedChassisRemotePhysicalPortMapEnableGet(prvTgfDevNum,
                                                                            nextHopPortNum,
                                                                            &origRemotePhysicalPortMappingEnablingStatus);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdCentralizedChassisRemotePhysicalPortMapEnableGet: %d %d", prvTgfDevNum, nextHopPortNum);

            rc = prvTgfCscdCentralizedChassisRemotePhysicalPortMapGet(prvTgfDevNum,
                                                                      nextHopPortNum,
                                                                      &origRemotePhysicalHwDevNum,
                                                                      &origRemotePhysicalPortNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdCentralizedChassisRemotePhysicalPortMapGet: %d %d", prvTgfDevNum, nextHopPortNum);

            rc = prvTgfCscdCentralizedChassisRemotePhysicalPortMapEnableSet(prvTgfDevNum,
                                                                            nextHopPortNum,
                                                                            GT_TRUE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdCentralizedChassisRemotePhysicalPortMapEnableSet: %d %d", prvTgfDevNum, nextHopPortNum);

             rc = prvTgfCscdCentralizedChassisRemotePhysicalPortMapSet(prvTgfDevNum,
                                                                      nextHopPortNum,
                                                                      DSA_TRG_DEV_CNS,
                                                                      DSA_TRG_PORT_CNS);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdCentralizedChassisRemotePhysicalPortMapSet: %d %d", prvTgfDevNum, nextHopPortNum);

             remotePhysicalPortMapWasConfigured = GT_TRUE;
        }

        /* AUTODOC: set srdDevice,srcPort modifications in the DSA tag for routed packets */
        rc = prvTgfCfgDsaTagSrcDevPortRoutedPcktModifySet(prvTgfDevNum,prvTgfCascadeEgressPortNum,GT_TRUE);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);


        /* AUTODOC: disable bridge bypass because test need route from the FDB entry */
        rc = prvTgfCscdPortBridgeBypassEnableSet(prvTgfDevNum,sendPortNum,GT_FALSE);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    }
    else
    {
        prvTgfDsaTargetHwDevNum = prvTgfDevNum;
    }


    return;
}


/**
* @internal prvTgfBasicIpv4UcRoutingInStackingSystemRestore function
* @endinternal
*
* @brief   Restore cascade system configurations
*
* @param[in] sendPortNum              - port sending traffic
*                                       None
*/
static GT_VOID prvTgfBasicIpv4UcRoutingInStackingSystemRestore
(
    IN GT_U32    sendPortNum,
    IN GT_U32    nextHopPortNum
)
{
    GT_STATUS   rc;
    CPSS_CSCD_LINK_TYPE_STC      cascadeLink;

    prvTgfPacketPartArray [DSA_INFO_INDEX].type    = TGF_PACKET_PART_SKIP_E;
    prvTgfPacketPartArray [DSA_INFO_INDEX].partPtr = NULL;
    prvTgfPacketPartArray [VLAN_TAG_INFO_INDEX].type    = TGF_PACKET_PART_VLAN_TAG_E;

    prvTgfPacket2PartArray[DSA_INFO_INDEX].type    = TGF_PACKET_PART_SKIP_E;
    prvTgfPacket2PartArray[DSA_INFO_INDEX].partPtr = NULL;
    prvTgfPacket2PartArray[VLAN_TAG_INFO_INDEX].type    = TGF_PACKET_PART_VLAN_TAG_E;

    /* AUTODOC: unset ingress port as 'cascade port' */
    rc = tgfTrafficGeneratorIngressCscdPortEnableSet(prvTgfDevNum,sendPortNum,GT_FALSE);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    if(tgfStackingSystemType != PRV_TGF_IPV4_STACKING_SYSTEM_AFTER_ROUTING_DEVICE_E)
    {
        /* AUTODOC: unset egress port as 'cascade port' */
        rc = tgfTrafficGeneratorEgressCscdPortEnableSet(prvTgfDevNum,prvTgfCascadeEgressPortNum,GT_FALSE,CPSS_CSCD_PORT_NETWORK_E);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        /* AUTODOC: restore local device to reach the remote target device */
        cascadeLink = origCascadeLink[0];
        rc = prvTgfCscdDevMapTableSet(prvTgfDevNum,
                                 prvTgfDsaTargetHwDevNum,
                                 prvTgfDsaTargetPortNum,
                                 &cascadeLink,
                                 origSrcPortTrunkHashEn[0],
                                 GT_FALSE);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        /* AUTODOC: restore local device to reach the remote target device */
        cascadeLink = origCascadeLink[1];
        rc = prvTgfCscdDevMapTableSet(prvTgfDevNum,
                                 DSA_SRC_DEV_CNS,
                                 prvTgfDsaTargetPortNum,
                                 &cascadeLink,
                                 origSrcPortTrunkHashEn[1],
                                 GT_FALSE);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        if (remotePhysicalPortMapWasConfigured == GT_TRUE)
        {
            rc = prvTgfCscdCentralizedChassisRemotePhysicalPortMapEnableSet(prvTgfDevNum,
                                                                            nextHopPortNum,
                                                                            origRemotePhysicalPortMappingEnablingStatus);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdCentralizedChassisRemotePhysicalPortMapEnableSet: %d %d", prvTgfDevNum, nextHopPortNum);

            rc = prvTgfCscdCentralizedChassisRemotePhysicalPortMapSet(prvTgfDevNum,
                                                                      nextHopPortNum,
                                                                      origRemotePhysicalHwDevNum,
                                                                      origRemotePhysicalPortNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdCentralizedChassisRemotePhysicalPortMapSet: %d %d", prvTgfDevNum, nextHopPortNum);

            remotePhysicalPortMapWasConfigured = GT_FALSE;
        }

        /* AUTODOC: unset srdDevice,srcPort modifications in the DSA tag for routed packets */
        rc = prvTgfCfgDsaTagSrcDevPortRoutedPcktModifySet(prvTgfDevNum,prvTgfCascadeEgressPortNum,GT_FALSE);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        /* AUTODOC: enable bridge bypass  (restore default) */
        rc = prvTgfCscdPortBridgeBypassEnableSet(prvTgfDevNum,sendPortNum,GT_TRUE);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    }
    else
    {
        prvTgfDsaTargetHwDevNum = DSA_TRG_DEV_CNS;
    }

}


/**
* @internal prvTgfBasicIpv4UcRoutingRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
* @param[in] sendPortIndex            - index of port sending traffic
* @param[in] nextHopPortNum           - port receiving traffic
* @param[in] useBulk                  - whether to use bulk operation to add prefixes
* @param[in] enableIpv4McRouting      - whether to enable IPv4 MC routing in the virtual router,
*                                      not relevant for policy based routing.
* @param[in] enableIpv6McRouting      - whether to enable IPv6 MC routing in the virtual router,
*                                      not relevant for policy based routing.
*                                       None
*/
GT_VOID prvTgfBasicIpv4UcRoutingRouteConfigurationSet
(
    GT_U32   prvUtfVrfId,
    GT_U8    sendPortIndex,
    GT_U32    nextHopPortNum,
    GT_BOOL  useBulk,
    GT_BOOL  enableIpv4McRouting,
    GT_BOOL  enableIpv6McRouting
)
{
    PRV_TGF_IP_ROUTING_MODE_ENT routingMode;
    GT_STATUS                   rc = GT_OK;

    /* AUTODOC: SETUP ROUTE CONFIGURATION: */

    PRV_TGF_PORT_INDEX_CHECK_FOR_VOID_FUNC_MAC(sendPortIndex);

    if(tgfStackingSystemType != PRV_TGF_IPV4_STACKING_SYSTEM_AFTER_ROUTING_DEVICE_E)
    {
        /* get routing mode */
        rc = prvTgfIpRoutingModeGet(&routingMode);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRoutingModeGet: %d", prvTgfDevNum);

        switch(routingMode)
        {
            case PRV_TGF_IP_ROUTING_MODE_PCL_ACTION_E:

                prvTgfBasicIpv4UcRoutingPbrConfigurationSet(prvUtfVrfId, sendPortIndex,
                                                            nextHopPortNum, useBulk);
                break;

            case PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E:

                prvTgfBasicIpv4UcRoutingLttRouteConfigurationSet(prvUtfVrfId, sendPortIndex,
                                                                 nextHopPortNum, useBulk,
                                                                 enableIpv4McRouting, enableIpv6McRouting);
                if(tgfStackingSystemType == PRV_TGF_IPV4_STACKING_SYSTEM_ROUTING_DEVICE_E) 
                {                
                    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
                    {    
                            rc = cpssDxChIpRouterVlanMacSaLsbSet(prvTgfDevNum,prvTgfNextHopeVlanid,prvTgfNextHopeVlanid);  
                            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpRouterVlanMacSaLsbSet: %d", prvTgfDevNum);  
                    }
                }
                break;

            case PRV_TGF_IP_ROUTING_MODE_UNDEFINED_E:

                break;
        }
    }
    else
    {        
        /* this device not do routing , but only act according to DSA with 'routed = 1' */
    }

   

    if(tgfStackingSystemType != PRV_TGF_IPV4_STACKING_SYSTEM_NONE_E)
    {
        prvTgfBasicIpv4UcRoutingInStackingSystemInit(prvTgfPortsArray[sendPortIndex],nextHopPortNum);
    }
}

/**
* @internal prvTgfBasicIpv4UcRoutingRouteSipLookupConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration - SIP Lookup
*
* @param[in] prvUtfVrfId              - virtual router index
*                                       None
*/
GT_VOID prvTgfBasicIpv4UcRoutingRouteSipLookupConfigurationSet
(
    GT_U32   prvUtfVrfId
)
{
    GT_STATUS   rc = GT_OK;
    GT_IPADDR   ipAddr;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;

    /* AUTODOC: define next hop for prefix same as in prvTgfBasicIpv4UcRoutingLttRouteConfigurationSet  */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex;
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* AUTODOC: add IPV4 UC prefix 1.1.1.1/32  */
   cpssOsMemCpy(ipAddr.arIP, prvTgfPacketIpv4Part.srcAddr, sizeof(ipAddr.arIP));

    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, prvUtfVrfId,
                                    ipAddr, 32, &nextHopInfo, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
           "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);

    /* AUTODOC:  enable SIP/SA Check on eport */
    rc = prvTgfIpPortSipSaEnableSet(prvTgfDevNum,prvTgfPortsArray[0],GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
           "prvTgfIpPortSipSaEnableSet: %d", prvTgfDevNum);

}


/**
* @internal prvTgfBasicIpv4LpmConfigurationSet function
* @endinternal
*
* @brief   Set LPM basic configuration
*
* @param[in] portNum                  - port number to disable the SIP lookup on
* @param[in] bypassEnabled            - the bypass enabling status:
*                                      GT_TRUE  - enable bypassing of lookup stages 8-31
*                                      GT_FALSE - disable bypassing of lookup stages 8-31
* @param[in] sipLookupEnable          - GT_TRUE:  enable SIP Lookup on the port
*                                      GT_FALSE: disable SIP Lookup on the port
*                                       None.
*/
GT_VOID prvTgfBasicIpv4LpmConfigurationSet
(
    GT_PORT_NUM portNum,
    GT_BOOL     bypassEnabled,
    GT_BOOL     sipLookupEnable
)
{
    GT_STATUS   rc = GT_OK;

    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CPSS_PP_E_ARCH_CNS))
    {
        /* keep value for restore - For SIP5 devices LPM Bypass is supported */
        if(firstCallToLpmConfig==GT_TRUE)
        {
            rc = prvTgfLpmLastLookupStagesBypassEnableGet(prvTgfDevNum,&(prvTgfRestoreCfg.defaultBypassEnable));
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfLpmLastLookupStagesBypassEnableGet: %d", prvTgfDevNum);

            rc = prvTgfLpmPortSipLookupEnableGet(prvTgfDevNum, portNum, &(prvTgfRestoreCfg.defaultSourceEportSipLookupEnable));
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfLpmPortSipLookupEnableGet: %d", prvTgfDevNum);

            rc = prvTgfIpPortSipSaEnableGet(prvTgfDevNum,portNum, &(prvTgfRestoreCfg.defaultSourceEportSipSaEnable));
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                         "prvTgfIpPortSipSaEnableGet: %d", prvTgfDevNum);

            firstCallToLpmConfig=GT_FALSE;
        }

        rc = prvTgfLpmLastLookupStagesBypassEnableSet(prvTgfDevNum,bypassEnabled);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfLpmLastLookupStagesBypassEnableSet: %d", prvTgfDevNum);

        rc = prvTgfLpmPortSipLookupEnableSet(prvTgfDevNum,portNum,sipLookupEnable);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChLpmPortSipLookupEnableSet: %d", prvTgfDevNum);
    }

    return;
}


/**
* @internal sendPacket function
* @endinternal
*
* @brief   Setting transmit parameters
*
* @param[in] packetInfoPtr            - (pointer to) the packet info
*                                       None.
*/
static GT_VOID sendPacket
(
    IN GT_U32    sendPortNum,
    IN TGF_PACKET_STC      *packetInfoPtr,
    IN CPSS_INTERFACE_INFO_STC *senderPortInterfacePtr
)
{
    GT_STATUS   rc;

    if(tgfStackingSystemType != PRV_TGF_IPV4_STACKING_SYSTEM_AFTER_ROUTING_DEVICE_E)
    {
        /* enable capture on next hop port/trunk */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(senderPortInterfacePtr, captureType, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                     prvTgfDevNum, captureType);

        /* clear capturing RxPcktTable */
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
    }

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* set transmit timings */
    rc = prvTgfSetTxSetup2Eth(0, 50);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetup2Eth: %d", prvTgfDevNum);

    /* send packet -- send from specific port -- even if member of a trunk */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, sendPortNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n",
                                 prvTgfDevNum, sendPortNum);

    if(tgfStackingSystemType != PRV_TGF_IPV4_STACKING_SYSTEM_AFTER_ROUTING_DEVICE_E)
    {
        cpssOsTimerWkAfter(1000);

        /* disable capture on nexthope port , before check the packets to the CPU */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(senderPortInterfacePtr, captureType, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                     prvTgfDevNum, captureType);
    }

    return;
}

/**
* @internal cascadeRouteStartCheck function
* @endinternal
*
* @brief   Setting transmit parameters
*
* @param[in] packetInfoPtr            - (pointer to) the packet info
*                                       None.
*/
static GT_VOID cascadeRouteStartCheck
(
    IN GT_U32    sendPortNum,
    IN TGF_PACKET_STC      *packetInfoPtr,
    IN TGF_PACKET_PART_STC *packetPartsPtr,
    IN CPSS_INTERFACE_INFO_STC *senderPortInterfacePtr
)
{
    GT_STATUS   rc;
    CPSS_INTERFACE_INFO_STC         portInterface;
    TGF_PACKET_STC                  *expectedCapturedPacketInfoPtr;
    GT_U32  actualCapturedNumOfPackets;/*actual number of packet captured from the interface*/
    GT_VOID *origL2Part;
    GT_U32                          portIter    = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    GT_BOOL                         isEqualCntrs = GT_FALSE;
    GT_BOOL                         packetExpected;
    GT_U32                          *byteNumMaskListPtr;
    GT_U32                          byteNumMaskSize;

    if (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
    {
        prvTgfPacketDsaTagPart.dsaType = TGF_DSA_4_WORD_TYPE_E;
        prvTgfPacketDsaTagPart_afterRoute.dsaType = TGF_DSA_4_WORD_TYPE_E;
    }

    packetPartsPtr[DSA_INFO_INDEX].type    = TGF_PACKET_PART_DSA_TAG_E;
    if(tgfStackingSystemType == PRV_TGF_IPV4_STACKING_SYSTEM_AFTER_ROUTING_DEVICE_E)
    {
        packetPartsPtr[DSA_INFO_INDEX].partPtr = &prvTgfPacketDsaTagPart_afterRoute;
    }
    else
    {
        packetPartsPtr[DSA_INFO_INDEX].partPtr = &prvTgfPacketDsaTagPart;
    }
    packetPartsPtr[VLAN_TAG_INFO_INDEX].type    = TGF_PACKET_PART_SKIP_E;

    packetInfoPtr->totalLen = TGF_PACKET_AUTO_CALC_LENGTH_CNS;

    /* NOTE: the 'Big Endian' is building the 'unions' in 'occurred' way ,
        so we must EXPLICITLY set 'union fields' to needed value */
    prvTgfPacketDsaTagPart_forward.source.portNum = 5;
    prvTgfPacketDsaTagPart_forward.origSrcPhy.portNum = DSA_TAG_ORIG_SRC_PHY_PORT_CNS;

    prvTgfPacketDsaTagPart_forward_afterRoute.source.portNum =
        prvTgfPacketDsaTagPart_forward.source.portNum;
    prvTgfPacketDsaTagPart_forward_afterRoute.origSrcPhy.portNum =
        prvTgfPacketDsaTagPart_forward.origSrcPhy.portNum;

    if (prvTgfPacketDsaTagPart.dsaType == TGF_DSA_4_WORD_TYPE_E)
    {
        prvTgfPacketDsaTagPart_forward.isTrgPhyPortValid = GT_TRUE;
    }

    prvTgfPacketDsaTagPart.dsaInfo.forward = prvTgfPacketDsaTagPart_forward;


    if(tgfStackingSystemType == PRV_TGF_IPV4_STACKING_SYSTEM_AFTER_ROUTING_DEVICE_E)
    {
        if(cascadeRouteSrcDevMode == CASCADE_ROUTE_SRC_DEV_MODE_MODIFY_E)
        {
            /* after route the DSA state that packet came from 'remote device' and not from 'me' */
            /* with 'Special src port' number (61) */
            prvTgfPacketDsaTagPart_forward_afterRoute.srcHwDev = DSA_SRC_DEV_CNS;
            prvTgfPacketDsaTagPart_forward_afterRoute.source.portNum = ROUTER_RESERVED_PORT_NUM;
            /* the egress port is not cascade .. the name prvTgfCascadeEgressPortNum
               is not really about egress cascade port */
            prvTgfPacketDsaTagPart_forward_afterRoute.dstInterface.devPort.portNum = prvTgfCascadeEgressPortNum;
            if (prvTgfPacketDsaTagPart_afterRoute.dsaType == TGF_DSA_4_WORD_TYPE_E)
            {
                prvTgfPacketDsaTagPart_forward_afterRoute.isTrgPhyPortValid = GT_TRUE;
            }

            if(prvUtfIsDoublePhysicalPortsModeUsed())
            {
                if( prvTgfDevNum ==
                    prvTgfPacketDsaTagPart_forward_afterRoute.dstInterface.devPort.hwDevNum)
                {
                    /* not enough bits to set the target physical port */
                    prvTgfPacketDsaTagPart_forward_afterRoute.isTrgPhyPortValid = GT_FALSE;
                    prvTgfPacketDsaTagPart_forward_afterRoute.dstEport =
                        prvTgfPacketDsaTagPart_forward_afterRoute.dstInterface.devPort.portNum;
                }
                else
                {
                    prvTgfPacketDsaTagPart_forward_afterRoute.dstEport = 0;
                }
            }

        }
        else  /*CASCADE_ROUTE_SRC_DEV_MODE_NOT_MODIFY_E*/
        {
            /* after route the DSA state still state that came from 'Origin device' */
            /* with 'Origin src port' number */
            prvTgfPacketDsaTagPart_forward_afterRoute.srcHwDev = prvTgfDevNum;
            prvTgfPacketDsaTagPart_forward_afterRoute.source.portNum = prvTgfPacketDsaTagPart_forward.source.portNum;

            if(prvUtfIsDoublePhysicalPortsModeUsed())
            {
                if( prvTgfDevNum ==
                    prvTgfPacketDsaTagPart_forward_afterRoute.dstInterface.devPort.hwDevNum)
                {
                    /* not enough bits to set the target physical port */
                    prvTgfPacketDsaTagPart_forward_afterRoute.isTrgPhyPortValid = GT_FALSE;
                    prvTgfPacketDsaTagPart_forward_afterRoute.dstEport =
                        prvTgfPacketDsaTagPart_forward_afterRoute.dstInterface.devPort.portNum;
                }
                else
                {
                    prvTgfPacketDsaTagPart_forward_afterRoute.dstEport = 0;
                }
            }
        }

        /* route back to the origin device */
        prvTgfPacketDsaTagPart_forward_afterRoute.dstInterface.devPort.hwDevNum =  prvTgfDevNum;

        prvTgfPacketDsaTagPart_afterRoute.dsaInfo.forward = prvTgfPacketDsaTagPart_forward_afterRoute;
    }
    else
    {
        prvTgfPacketDsaTagPart.dsaInfo.forward = prvTgfPacketDsaTagPart_forward;
    }

    sendPacket(sendPortNum,packetInfoPtr,senderPortInterfacePtr);

    if(tgfStackingSystemType == PRV_TGF_IPV4_STACKING_SYSTEM_AFTER_ROUTING_DEVICE_E)
    {
        /*restore*/
        prvTgfPacketDsaTagPart_forward_afterRoute.dstInterface.devPort.portNum = DSA_TRG_PORT_CNS;
        if(prvUtfIsDoublePhysicalPortsModeUsed())
        {
            /*restore*/
            prvTgfPacketDsaTagPart_forward_afterRoute.dstEport =
                prvTgfPacketDsaTagPart_forward_afterRoute.dstInterface.devPort.portNum;
        }

        /* check egress port counters .. not DSA tag issues on egress */
        /* read and check ethernet counters */
        for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
        {
            /* read counters */
            rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);

            expectedCntrs = portCntrs;
            /* the egress port is not cascade .. the name prvTgfCascadeEgressPortNum
               is not really about egress cascade port */
            if(prvTgfPortsArray[portIter] == sendPortNum)
            {
                /* AUTODOC: check that number of packets match the expectations */
                packetExpected = 1;
                expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
            }
            else
            if(prvTgfPortsArray[portIter] == prvTgfCascadeEgressPortNum)
            {
                if(cascadeRouteSrcDevMode == CASCADE_ROUTE_SRC_DEV_MODE_MODIFY_E)
                {
                    /* the srcHwDev is not 'own' so no reason to filter */
                    packetExpected = 1;
                }
                else
                {
                    /* the srcHwDev is 'own' so filter due to 'Loops prevention'*/
                    packetExpected = 0;
                }
            }
            else
            {
                /* check that no packets egress the port */
                packetExpected = 0;
            }


            expectedCntrs.goodPktsSent.l[0]   = packetExpected * prvTgfBurstCount;
            /* verify counters */
            PRV_TGF_VERIFY_COUNTERS_MAC(isEqualCntrs, expectedCntrs, portCntrs);
            UTF_VERIFY_EQUAL0_STRING_MAC(isEqualCntrs, GT_TRUE, "get another counters values.");

            /* print expected values if not equal */
            PRV_TGF_PRINT_DIFF_COUNTERS_MAC(isEqualCntrs, expectedCntrs, portCntrs);
        }
    }
    else
    {
        portInterface.type            = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum  = prvTgfDevNum;
        portInterface.devPort.portNum = prvTgfCascadeEgressPortNum;

        PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", prvTgfCascadeEgressPortNum);

        if(cascadeRouteSrcDevMode == CASCADE_ROUTE_SRC_DEV_MODE_MODIFY_E)
        {
            /* after route the DSA state that packet came from 'me' and not from 'Origin device' */
            /* with 'Special src port' number (61) */
            prvTgfPacketDsaTagPart_forward_afterRoute.srcHwDev = prvTgfDevNum;
            prvTgfPacketDsaTagPart_forward_afterRoute.source.portNum = ROUTER_RESERVED_PORT_NUM;

            if ((UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum) == GT_FALSE) && (prvTgfCascadeEgressPortNum >= 64))
            {
                /* support Lion2 in 128 ports mode */
                /* the CPSS will convert the <srcDev,Port> to <srcDev+1 , port-64>*/
                prvTgfPacketDsaTagPart_forward_afterRoute.source.portNum += 64;
            }

            if (prvTgfPacketDsaTagPart.dsaType == TGF_DSA_4_WORD_TYPE_E)
            {
                prvTgfPacketDsaTagPart_forward_afterRoute.isTrgPhyPortValid = GT_TRUE;
                prvTgfPacketDsaTagPart_forward_afterRoute.dstInterface.devPort.portNum = DSA_TRG_PORT_CNS;
                prvTgfPacketDsaTagPart_forward_afterRoute.dstInterface.devPort.hwDevNum = DSA_TRG_DEV_CNS;
            }
        }
        else
        if(cascadeRouteSrcDevMode == CASCADE_ROUTE_SRC_DEV_MODE_NOT_MODIFY_E)
        {
            /* after route the DSA state still state that came from 'Origin device' */
            /* with 'Origin src port' number */
            if(PRV_CPSS_SIP_5_25_CHECK_MAC(prvTgfDevNum))
            {
                prvTgfPacketDsaTagPart_forward_afterRoute.isTrgPhyPortValid = GT_TRUE;
            }
            prvTgfPacketDsaTagPart_forward_afterRoute.srcHwDev = prvTgfPacketDsaTagPart_forward.srcHwDev;
            prvTgfPacketDsaTagPart_forward_afterRoute.source.portNum = prvTgfPacketDsaTagPart_forward.source.portNum;
        }
        else
        {
            rc = GT_NOT_SUPPORTED;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,rc,"not supported cascadeRouteSrcDevMode [%d] \n",
                cascadeRouteSrcDevMode);
        }

        if(cascadeRouteTrgDevMode == CASCADE_ROUTE_TRG_DEV_MODE_BACK_TO_SRC_E)
        {
            /* route back to the origin device */
            prvTgfPacketDsaTagPart_forward_afterRoute.dstInterface.devPort.hwDevNum =  prvTgfPacketDsaTagPart_forward.srcHwDev;
        }
        else
        if(cascadeRouteTrgDevMode == CASCADE_ROUTE_TRG_DEV_MODE_OTHER_E)
        {
            /* route to a device that is not 'me' and is not 'Origin device'*/
            prvTgfPacketDsaTagPart_forward_afterRoute.dstInterface.devPort.hwDevNum =  prvTgfDsaTargetHwDevNum;
            if (prvTgfPacketDsaTagPart_afterRoute.dsaType == TGF_DSA_4_WORD_TYPE_E)
            {
                prvTgfPacketDsaTagPart_forward_afterRoute.dstEport = prvTgfDsaTargetPortNum;
            }
        }
        else
        {
            rc = GT_NOT_SUPPORTED;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,rc,"not supported cascadeRouteTrgDevMode [%d] \n",
                cascadeRouteTrgDevMode);
        }

        prvTgfPacketDsaTagPart_afterRoute.dsaInfo.forward = prvTgfPacketDsaTagPart_forward_afterRoute;

        packetPartsPtr[DSA_INFO_INDEX].type    = TGF_PACKET_PART_DSA_TAG_E;
        packetPartsPtr[DSA_INFO_INDEX].partPtr = &prvTgfPacketDsaTagPart_afterRoute;
        packetPartsPtr[VLAN_TAG_INFO_INDEX].type    = TGF_PACKET_PART_SKIP_E;

        origL2Part = packetPartsPtr[0].partPtr;
        packetPartsPtr[0].partPtr = &afterRouteL2;
        /* AUTODOC: expect routed packet - modified mac DA */
        cpssOsMemCpy(afterRouteL2.daMac, prvTgfArpMac, sizeof(TGF_MAC_ADDR));
        /* AUTODOC: expect routed packet - modified mac SA */
        afterRouteL2.saMac[4] = (GT_U8)((prvTgfNextHopeVlanid & 0xFFF) >> 8);
        afterRouteL2.saMac[5] = (GT_U8)(prvTgfNextHopeVlanid & 0xFFF);

        expectedCapturedPacketInfoPtr = packetInfoPtr;

        if (prvTgfPacketDsaTagPart.dsaType == TGF_DSA_4_WORD_TYPE_E)
        {
            byteNumMaskListPtr = byteNumMaskList;
            byteNumMaskSize = sizeof(byteNumMaskList)/sizeof(byteNumMaskList[0]);
        }
        else
        {
            byteNumMaskListPtr = NULL;
            byteNumMaskSize = 0;
        }

        /* print captured packets and check TriggerCounters */
        rc = tgfTrafficGeneratorPortTxEthCaptureCompare(
                &portInterface,
                expectedCapturedPacketInfoPtr,
                prvTgfBurstCount,/*numOfPackets*/
                0/*vfdNum*/,
                NULL /*vfdArray*/,
                byteNumMaskListPtr, /* skip check of bytes in these positions */
                byteNumMaskSize, /* length of skip list */
                &actualCapturedNumOfPackets,
                NULL/*onFirstPacketNumTriggersBmpPtr*/);

        rc = rc == GT_NO_MORE ? GT_OK : rc;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "tgfTrafficGeneratorPortTxEthTriggerCountersGet:"
                                     " port = %d, rc = 0x%02X\n", portInterface.devPort.portNum, rc);

        packetPartsPtr[0].partPtr = origL2Part;
    }

}


/**
* @internal prvTgfBasicIpv4UcRoutingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] sendPortNum              - port sending traffic
* @param[in] nextHopPortNum           - port receiving traffic
* @param[in] useSecondPkt             - whether to use the second packet (prvTgfPacket2PartArray)
* @param[in] useSecondArp             - whether to use the second ARP MAC address (prvTgfArpMac1)
* @param[in] isUrpfTest               - whether this is unicast RPF test
* @param[in] expectNoTraffic          - whether to expect traffic or not
*                                       None
*/
GT_VOID prvTgfBasicIpv4UcRoutingTrafficGenerate
(
    GT_U32    sendPortNum,
    GT_U32    nextHopPortNum,
    GT_BOOL  useSecondPkt,
    GT_BOOL  useSecondArp,
    GT_BOOL  isUrpfTest,
    GT_BOOL  expectNoTraffic
)
{
    GT_STATUS                       rc          = GT_OK;
    GT_U32                          partsCount  = 0;
    GT_U32                          packetSize  = 0;
    TGF_PACKET_PART_STC             *packetPartsPtr;
    TGF_PACKET_STC                  packetInfo;
    GT_U32                          portIter    = 0;
    GT_U32                          numTriggers = 0;
    TGF_VFD_INFO_STC                vfdArray[1];
    CPSS_INTERFACE_INFO_STC         portInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    PRV_TGF_IP_COUNTER_SET_STC      ipCounters;
    PRV_TGF_IP_ROUTING_MODE_ENT     routingMode;
    TGF_MAC_ADDR                    arpMacAddr;
    GT_TRUNK_ID currTrunkId;/* trunk Id for the current port */
    GT_TRUNK_ID senderTrunkId;/* trunk Id for the sender port that is member of */
    GT_TRUNK_ID nextHopTrunkId;/* trunk Id for the next hop port that is member of */
    CPSS_TRUNK_MEMBER_STC   trunkMember;/* temporary trunk member */
    GT_BOOL     stormingDetected;/*indicates that the traffic to check
                   LBH that egress the trunk was originally INGRESSED from the trunk.
                   but since in the 'enhanced UT' the CPU send traffic to a port
                   due to loopback it returns to it, we need to ensure that the
                   member mentioned here should get the traffic since it is the
                   'original sender'*/
    GT_U32      vlanIter;/*vlan iterator*/
    GT_U32      vlanCount = sizeof(prvTgfVlanArray)/sizeof(prvTgfVlanArray[0]);/*number of vlans*/
    GT_U32      numVfd = 0;/* number of VFDs in vfdArray */
    GT_HW_DEV_NUM                           targetHwDevice;
    GT_PORT_NUM                             targetPort;
    GT_U32       egressPortIndex = 0xFF;
    GT_BOOL      isRoutedPacket = GT_FALSE;
    GT_BOOL      is_prvTgfCaptureForceTtiDisableModeSet_GT_TRUE = GT_FALSE;

    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        if(prvTgfPortsArray[portIter] == nextHopPortNum)
        {
            egressPortIndex = portIter;
            break;
        }
    }

    if(tgfStackingSystemType != PRV_TGF_IPV4_STACKING_SYSTEM_NONE_E)
    {
        if(tgfStackingSystemType == PRV_TGF_IPV4_STACKING_SYSTEM_AFTER_ROUTING_DEVICE_E)
        {
            targetHwDevice = prvTgfDevNum;
            targetPort   = nextHopPortNum;
            PRV_UTF_LOG2_MAC("got route from remote device to my device [0x%x] ,on local port [0x%x]\n",
                targetHwDevice,targetPort);
        }
        else
        {
            targetHwDevice = prvTgfDsaTargetHwDevNum;
            targetPort   = prvTgfDsaTargetPortNum;
            PRV_UTF_LOG2_MAC("do route to remote device[0x%x] , remote port [0x%x]\n",
                targetHwDevice,targetPort);
        }
    }
    else
    {
        targetHwDevice = prvTgfDevNum;
        targetPort = nextHopPortNum;
    }

    if(tgfStackingSystemType != PRV_TGF_IPV4_STACKING_SYSTEM_AFTER_ROUTING_DEVICE_E)
    {
        /* -------------------------------------------------------------------------
         * 1. Setup counters and enable capturing
         *
         */

        /* get routing mode */
        rc = prvTgfIpRoutingModeGet(&routingMode);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRoutingModeGet: %d", prvTgfDevNum);

        for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
        {
            /* reset ethernet counters */
            rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);

            if (routingMode == PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E)
            {
                /* reset IP couters and set ROUTE_ENTRY mode */
                rc = prvTgfCountersIpSet(prvTgfPortsArray[portIter], portIter);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCountersIpSet: %d, %d",
                                             prvTgfDevNum, prvTgfPortsArray[portIter]);
            }
        }
    }
    else
    {
        /*this sets all ports to link state 'UP'*/
        prvTgfEthCountersReset(prvTgfDevNum);
    }

    /* Routing from trunk support:
       check if the port is trunk member.
       when trunk member , set all the members with same configuration.
    */
    trunkMember.port = sendPortNum;
    trunkMember.hwDevice = prvTgfDevNum;

    rc = prvTgfTrunkDbIsMemberOfTrunk(prvTgfDevNum,&trunkMember,&senderTrunkId);
    if(rc == GT_OK)
    {
    }
    else
    {
        senderTrunkId = 0;
    }

    /* Routing to trunk support:
       check if the NH port is trunk member.
       when trunk member --> use the trunk ID.
    */
    trunkMember.port = targetPort;
    trunkMember.hwDevice = targetHwDevice;

    rc = prvTgfTrunkDbIsMemberOfTrunk(prvTgfDevNum,&trunkMember,&nextHopTrunkId);
    if(rc == GT_OK)
    {
        /* the port is member of the trunk */

        /* setup nexthope portInterface for capturing */
        portInterface.type            = CPSS_INTERFACE_TRUNK_E;
        portInterface.trunkId         = nextHopTrunkId;

        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
        {
            /* send 64 packets in SIP5 due to new LBH of the trunk */
            prvTgfBurstCount = 64;
        }
        else
        {
            /*since we have 2 members in the NH trunk send 8 packets*/
            prvTgfBurstCount = 8;
        }

        vfdArray[0].modeExtraInfo = 0;
        vfdArray[0].offset = 6 + 5;/* last byte of the mac SA */
        cpssOsMemSet(vfdArray[0].patternPtr,0,sizeof(TGF_MAC_ADDR));
        vfdArray[0].patternPtr[0] = 0;
        vfdArray[0].cycleCount = 1;/*single byte*/
        vfdArray[0].mode = TGF_VFD_MODE_INCREMENT_E;

        numVfd = 1;
    }
    else
    {
        nextHopTrunkId = 0;

        /* setup nexthope portInterface for capturing */
        portInterface.type            = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum  = prvTgfDevNum;
        portInterface.devPort.portNum = nextHopPortNum;
    }

    if((senderTrunkId != 0) && (nextHopTrunkId == senderTrunkId))
    {

        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
        {
            /* send 64 packets in SIP5 due to new LBH of the trunk */
            prvTgfBurstCount = 64;
        }
        else
        {
            /*since we have 4 members in the NH trunk send 16 packets*/
            prvTgfBurstCount = 16;
        }

        /* make all trunk ports members of the vlans */
        for (vlanIter = 0; vlanIter < vlanCount; vlanIter++)
        {
            for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
            {
                rc = prvTgfBrgVlanMemberAdd(prvTgfDevNum, prvTgfVlanArray[vlanIter],
                                            prvTgfPortsArray[portIter], GT_FALSE);
                UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberAdd: %d, %d, %d, %d",
                                             prvTgfDevNum, prvTgfVlanArray[vlanIter],
                                             prvTgfPortsArray[portIter], GT_FALSE);
            }
        }

        /* we route from a trunk to the same trunk */
        /* the 'Mirroring capture' is not good because it sets :
           force PVID = 0 for all traffic that ingress the 'Captured port' .
           but since the 'sender' is also member of the 'NH trunk' this port must
           also be 'captured' , but this will not allow the packets from the CPU
           to initiate routing at all !

           so we use 'Special PCL capture' that 'skip' the ingress vlan (before the routing)
           by this the traffic before route can ingress the device but the
           traffic after the route will be captured to the CPU.
           */
        captureType = TGF_CAPTURE_MODE_PCL_E;

        /* state that no need to capture packets that are in the Vlan before the routing */
        rc = tgfTrafficGeneratorPortTxEthCapturePclExcludeVidSet(GT_TRUE,PRV_TGF_SEND_VLANID_CNS);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCapturePclExcludeVidSet: %d, %d\n",
                                     prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

        if(appDemo_trunk_phy1690_WA_B)
        {
            GT_PORT_NUM eportOfTrunk;

            rc = trunkEPortGet_phy1690_WA_B(senderTrunkId,&eportOfTrunk);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "trunkEPortGet_phy1690_WA_B: prvTgfDevNum %d senderTrunkId[%d]",
                                         prvTgfDevNum, senderTrunkId);
            /* we must not kill the TTI lookups on the ingress port !!! */
            /* the WA is based on it */
            prvTgfCaptureForceTtiDisableModeSet(GT_TRUE);
            is_prvTgfCaptureForceTtiDisableModeSet_GT_TRUE = GT_TRUE;
            /* set PCL for the TRAP on the srcEPort */
            rc = prvTgfDxChPclCaptureSet(prvTgfDevNum,eportOfTrunk,GT_TRUE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfDxChPclCaptureSet: prvTgfDevNum %d eportOfTrunk[%d]",
                                         prvTgfDevNum, eportOfTrunk);
        }
    }
    else
    {
        captureType = TGF_CAPTURE_MODE_MIRRORING_E;
    }


    /* enable capture on next hop port/trunk */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, captureType, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                 prvTgfDevNum, captureType);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");


    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* number of parts in packet */
    if (useSecondPkt == GT_TRUE)
    {
        partsCount = sizeof(prvTgfPacket2PartArray) / sizeof(prvTgfPacket2PartArray[0]);
        packetPartsPtr = prvTgfPacket2PartArray;
    }
    else
    {
        partsCount = sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);
        packetPartsPtr = prvTgfPacketPartArray;
    }

    /* calculate packet size */
    rc = prvTgfPacketSizeGet(packetPartsPtr, partsCount, &packetSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

    /* build packet */
    packetInfo.totalLen   = packetSize;
    packetInfo.numOfParts = partsCount;
    packetInfo.partsArray = packetPartsPtr;

    if(tgfStackingSystemType != PRV_TGF_IPV4_STACKING_SYSTEM_NONE_E)
    {
        /* disable capture on nexthope port , before check the packets to the CPU */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, captureType, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                     prvTgfDevNum, captureType);
        goto tgfStackingSystem_lbl;
    }

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, prvTgfBurstCount, numVfd, &vfdArray[0]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* set transmit timings */
    rc = prvTgfSetTxSetup2Eth(1, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetup2Eth: %d", prvTgfDevNum);

    /* AUTODOC: send packet from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:01, VID=5 */
    /* AUTODOC:   srcIP=1.1.1.1 */
    /* AUTODOC:   dstIP=1.1.1.3 for regular test, 1.1.1.3 or 3.2.2.3 for bulk test */

    /* send packet -- send from specific port -- even if member of a trunk */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, sendPortNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n",
                                 prvTgfDevNum, sendPortNum);
    cpssOsTimerWkAfter(1000);

    /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    /* read and check ethernet counters */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        GT_BOOL isOk;

        /* Routing trunk support:
           check if the port is trunk member.
        */
        trunkMember.port = prvTgfPortsArray[portIter];
        trunkMember.hwDevice = prvTgfDevNum;

        rc = prvTgfTrunkDbIsMemberOfTrunk(prvTgfDevNum,&trunkMember,&currTrunkId);
        if(rc == GT_OK)
        {
            /* this port is member of trunk */

            if(currTrunkId == nextHopTrunkId)
            {
                /* this port is member of the next hop trunk */

                /* we need to SUM all those ports together before we check */
                /* we do the check in trunk dedicated function for 'target trunk'*/
                /* see after this loop */
                continue;
            }
        }

        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);


        /* AUTODOC: verify routed packet on port 3 */
        if (isUrpfTest == GT_TRUE)
        {
            if ( (expectNoTraffic == GT_TRUE && prvTgfPortsArray[portIter]==sendPortNum) ||
                 (expectNoTraffic == GT_FALSE && ((prvTgfPortsArray[portIter]==sendPortNum) || (portIter ==egressPortIndex))) )
            {
                isRoutedPacket = GT_TRUE;
            }
        }
        else
        {   if ( (prvTgfPortsArray[portIter]==sendPortNum) ||
                 ((expectNoTraffic==GT_FALSE)&&(portIter ==egressPortIndex)))
            {
                isRoutedPacket = GT_TRUE;
            }
        }
        if (isRoutedPacket == GT_TRUE)
        {
            /* packetSize is not changed */
                expectedCntrs.goodOctetsSent.l[0] = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;

                isRoutedPacket = GT_FALSE;
        }
        else
        {
                 /* for other ports */
                expectedCntrs.goodOctetsSent.l[0] = 0;
                expectedCntrs.goodPktsSent.l[0]   = 0;
                expectedCntrs.ucPktsSent.l[0]     = 0;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = 0;
                expectedCntrs.goodPktsRcv.l[0]    = 0;
                expectedCntrs.ucPktsRcv.l[0]      = 0;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;
        }

        isOk =
            portCntrs.goodOctetsSent.l[0] == expectedCntrs.goodOctetsSent.l[0] &&
            portCntrs.goodPktsSent.l[0]   == expectedCntrs.goodPktsSent.l[0] &&
            portCntrs.ucPktsSent.l[0]     == expectedCntrs.ucPktsSent.l[0] &&
            portCntrs.brdcPktsSent.l[0]   == expectedCntrs.brdcPktsSent.l[0] &&
            portCntrs.mcPktsSent.l[0]     == expectedCntrs.mcPktsSent.l[0] &&
            portCntrs.goodOctetsRcv.l[0]  == expectedCntrs.goodOctetsRcv.l[0] &&
            portCntrs.goodPktsRcv.l[0]    == expectedCntrs.goodPktsRcv.l[0] &&
            portCntrs.ucPktsRcv.l[0]      == expectedCntrs.ucPktsRcv.l[0] &&
            portCntrs.brdcPktsRcv.l[0]    == expectedCntrs.brdcPktsRcv.l[0] &&
            portCntrs.mcPktsRcv.l[0]      == expectedCntrs.mcPktsRcv.l[0];

        UTF_VERIFY_EQUAL0_STRING_MAC(isOk, GT_TRUE, "get another counters values.");

        /* print expected values if bug */
        if (isOk != GT_TRUE) {
            PRV_UTF_LOG0_MAC("Expected values:\n");
            PRV_UTF_LOG1_MAC("  goodOctetsSent = %d\n", expectedCntrs.goodOctetsSent.l[0]);
            PRV_UTF_LOG1_MAC("    goodPktsSent = %d\n", expectedCntrs.goodPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("      ucPktsSent = %d\n", expectedCntrs.ucPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("    brdcPktsSent = %d\n", expectedCntrs.brdcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("      mcPktsSent = %d\n", expectedCntrs.mcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("   goodOctetsRcv = %d\n", expectedCntrs.goodOctetsRcv.l[0]);
            PRV_UTF_LOG1_MAC("     goodPktsRcv = %d\n", expectedCntrs.goodPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC("       ucPktsRcv = %d\n", expectedCntrs.ucPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC("     brdcPktsRcv = %d\n", expectedCntrs.brdcPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC("       mcPktsRcv = %d\n", expectedCntrs.mcPktsRcv.l[0]);
            PRV_UTF_LOG0_MAC("\n");
        }
    }

    if (expectNoTraffic == GT_TRUE && isUrpfTest == GT_TRUE)
    {
        /* disable capture on nexthope port , before check the packets to the CPU */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, captureType, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                     prvTgfDevNum, captureType);
        return;
    }

    if(nextHopTrunkId)
    {
        /* the traffic should egress the next hop trunk */
        /* we need to SUM all those ports together before we check */
        /* we do the check in trunk dedicated function for 'target trunk'*/
        if(nextHopTrunkId == senderTrunkId)
        {
            trunkMember.port = sendPortNum;
            trunkMember.hwDevice = prvTgfDevNum;

            prvTgfTrunkLoadBalanceCheck(nextHopTrunkId,
                PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_EVEN_E,
                prvTgfBurstCount,
                0,/*tolerance*/
                &trunkMember,/* the port in the trunk that sent the traffic */
                &stormingDetected);/*was storming detected*/

            /* it will detect 'storming' but those are only the LBH of this port */
        }
        else
        {
            prvTgfTrunkLoadBalanceCheck(nextHopTrunkId,
                PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_EVEN_E,
                prvTgfBurstCount,
                0,/*tolerance*/
                NULL,/*NA*/
                NULL);/*NA*/
        }
    }


    /* -------------------------------------------------------------------------
     * 4. Get Trigger Counters
     */

    if(expectNoTraffic==GT_FALSE)
    {
        PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", nextHopPortNum);

        /* get trigger counters where packet has MAC DA as prvTgfArpMac */
        vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
        vfdArray[0].modeExtraInfo = 0;
        vfdArray[0].offset = 0;
        vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
        if (useSecondArp)
        {
            cpssOsMemCpy(arpMacAddr, &prvTgfArpMac1, sizeof(TGF_MAC_ADDR));
        }
        else
        {
            cpssOsMemCpy(arpMacAddr, &prvTgfArpMac, sizeof(TGF_MAC_ADDR));
        }
        cpssOsMemCpy(vfdArray[0].patternPtr, arpMacAddr, sizeof(TGF_MAC_ADDR));

        /* disable capture on nexthope port , before check the packets to the CPU */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, captureType, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                     prvTgfDevNum, captureType);

        if(is_prvTgfCaptureForceTtiDisableModeSet_GT_TRUE)
        {
            CPSS_INTERFACE_INFO_STC eport_portInterface = portInterface;
            /* restore settings */
            prvTgfCaptureForceTtiDisableModeSet(GT_FALSE);
            if(appDemo_trunk_phy1690_WA_B)
            {
                GT_PORT_NUM eportOfTrunk;

                rc = trunkEPortGet_phy1690_WA_B(senderTrunkId,&eportOfTrunk);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "trunkEPortGet_phy1690_WA_B: prvTgfDevNum %d senderTrunkId[%d]",
                                             prvTgfDevNum, senderTrunkId);
                /* unset PCL for the TRAP on the srcEPort */
                rc = prvTgfDxChPclCaptureSet(prvTgfDevNum,eportOfTrunk,GT_FALSE);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfDxChPclCaptureSet: prvTgfDevNum %d eportOfTrunk[%d]",
                                             prvTgfDevNum, eportOfTrunk);

                eport_portInterface.type = CPSS_INTERFACE_PORT_E;
                eport_portInterface.devPort.hwDevNum = prvTgfDevNum;
                eport_portInterface.devPort.portNum  = eportOfTrunk;
                tgfTrafficGeneratorExpectedInterfaceEport(PRV_TGF_EXPECTED_INTERFACE_TYPE_EPORT_NUM_E);
            }

            /* check the packets to the CPU */
            rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&eport_portInterface, 1, vfdArray, &numTriggers);
            PRV_UTF_LOG2_MAC("    numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);
        }
        else
        {
            /* check the packets to the CPU */
            rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
            PRV_UTF_LOG2_MAC("    numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);
        }

        /* check TriggerCounters */
        rc = rc == GT_NO_MORE ? GT_OK : rc;
        if(portInterface.type  == CPSS_INTERFACE_PORT_E)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                         portInterface.devPort.hwDevNum, portInterface.devPort.portNum);
        }
        else
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, trunkId [%d] \n",
                                         prvTgfDevNum, nextHopTrunkId);
        }

        if(prvTgfBurstCount >= 32)
        {
            /* AUTODOC: for more than 32 transmits check that got only first 32 matches of the MAC of packet */
            UTF_VERIFY_EQUAL0_STRING_MAC(0xFFFFFFFF, numTriggers,
                "\n for more than 32 transmits check that got only first 32 matches of the MAC of packet \n");
        }
        else
        {
            /* AUTODOC: check if packet has the same MAC DA as ARP MAC */
            /* number of triggers should be according to number of transmit*/
            UTF_VERIFY_EQUAL6_STRING_MAC(((1<<prvTgfBurstCount)-1), numTriggers,
                    "\n   MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X",
                    arpMacAddr[0], arpMacAddr[1], arpMacAddr[2],
                    arpMacAddr[3], arpMacAddr[4], arpMacAddr[5]);
        }
    }
    else
    {
        /* disable capture on nexthope port , before check the packets to the CPU */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, captureType, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                     prvTgfDevNum, captureType);
    }

    /* -------------------------------------------------------------------------
     * 5. Get IP Counters
     */

    /* AUTODOC: get and print ip counters values */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++) {
        PRV_UTF_LOG1_MAC("IP counters for Port [%d]:\n", prvTgfPortsArray[portIter]);
        prvTgfCountersIpGet(prvTgfDevNum, portIter, GT_TRUE, &ipCounters);
    }
    PRV_UTF_LOG0_MAC("\n");

tgfStackingSystem_lbl:
    if((tgfStackingSystemType != PRV_TGF_IPV4_STACKING_SYSTEM_NONE_E)&&(expectNoTraffic==GT_FALSE))
    {
        PRV_TGF_IP_UC_ROUTE_ENTRY_STC           routeEntriesArray[1];
        cpssOsMemSet(routeEntriesArray, 0, sizeof(PRV_TGF_IP_UC_ROUTE_ENTRY_STC));

        if(tgfStackingSystemType != PRV_TGF_IPV4_STACKING_SYSTEM_AFTER_ROUTING_DEVICE_E)
        {
            cascadeRouteTrgDevMode = CASCADE_ROUTE_TRG_DEV_MODE_OTHER_E;
            if(PRV_CPSS_SIP_5_25_CHECK_MAC(prvTgfDevNum))
            {
                cascadeRouteSrcDevMode = CASCADE_ROUTE_SRC_DEV_MODE_NOT_MODIFY_E;
            }
            else
            {
                cascadeRouteSrcDevMode = CASCADE_ROUTE_SRC_DEV_MODE_MODIFY_E;
            }

            cascadeRouteStartCheck(sendPortNum,&packetInfo,packetPartsPtr,&portInterface);

            rc = prvTgfIpUcRouteEntriesRead(prvTgfDevNum,prvTgfRouteEntryBaseIndex, routeEntriesArray, 1);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesRead: %d", prvTgfDevNum);

            /* modify the NEXT hop to set new target device --  the 'orig device '*/
            routeEntriesArray[0].nextHopInterface.devPort.hwDevNum = DSA_SRC_DEV_CNS;
            rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndex, routeEntriesArray, 1);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);

            if (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
            {
                rc = prvTgfCscdCentralizedChassisRemotePhysicalPortMapSet(prvTgfDevNum,
                                                                          nextHopPortNum,
                                                                          DSA_SRC_DEV_CNS,
                                                                          DSA_TAG_ORIG_SRC_PHY_PORT_CNS);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdCentralizedChassisRemotePhysicalPortMapSet: %d %d", prvTgfDevNum, nextHopPortNum);
            }

            cascadeRouteTrgDevMode = CASCADE_ROUTE_TRG_DEV_MODE_BACK_TO_SRC_E;
            if(PRV_CPSS_SIP_5_25_CHECK_MAC(prvTgfDevNum))
            {
                cascadeRouteSrcDevMode = CASCADE_ROUTE_SRC_DEV_MODE_NOT_MODIFY_E;
            }
            else
            {
                cascadeRouteSrcDevMode = CASCADE_ROUTE_SRC_DEV_MODE_MODIFY_E;
            }
            cascadeRouteStartCheck(sendPortNum,&packetInfo,packetPartsPtr,&portInterface);

            cascadeRouteTrgDevMode = CASCADE_ROUTE_TRG_DEV_MODE_BACK_TO_SRC_E;

            cascadeRouteSrcDevMode = CASCADE_ROUTE_SRC_DEV_MODE_NOT_MODIFY_E;

            /* unset srdDevice,srcPort modifications in the DSA tag for routed packets */
            rc = prvTgfCfgDsaTagSrcDevPortRoutedPcktModifySet(prvTgfDevNum,prvTgfCascadeEgressPortNum,GT_FALSE);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
            cascadeRouteStartCheck(sendPortNum,&packetInfo,packetPartsPtr,&portInterface);
        }
        else
        {
            cascadeRouteTrgDevMode = CASCADE_ROUTE_TRG_DEV_MODE_BACK_TO_SRC_E;
            cascadeRouteSrcDevMode = CASCADE_ROUTE_SRC_DEV_MODE_NOT_MODIFY_E;
            cascadeRouteStartCheck(sendPortNum,&packetInfo,packetPartsPtr,&portInterface);

            cascadeRouteTrgDevMode = CASCADE_ROUTE_TRG_DEV_MODE_BACK_TO_SRC_E;
            cascadeRouteSrcDevMode = CASCADE_ROUTE_SRC_DEV_MODE_MODIFY_E;
            cascadeRouteStartCheck(sendPortNum,&packetInfo,packetPartsPtr,&portInterface);
        }

    }

    /* restore transmit timings */
    rc = prvTgfSetTxSetup2Eth(0, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetup2Eth: %d", prvTgfDevNum);

    /* disable the PCL exclude vid capturing */
    rc = tgfTrafficGeneratorPortTxEthCapturePclExcludeVidSet(GT_FALSE,0/*don't care*/);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCapturePclExcludeVidSet: %d, %d\n",
                                 prvTgfDevNum, GT_FALSE);

    /* just for 'cleanup' */
    captureType = TGF_CAPTURE_MODE_MIRRORING_E;

}

/**
* @internal prvTgfBasicIpv4UcRoutingConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @param[in] prvUtfVrfId              - virtual router index
* @param[in] sendPortIndex            - index of port sending traffic
* @param[in] useBulk                  - whether to use bulk operation to delete the prefixes
*                                       None
*
* @note 2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfBasicIpv4UcRoutingConfigurationRestore
(
    GT_U32   prvUtfVrfId,
    GT_U8    sendPortIndex,
    GT_U8    nextHopPortIndex,
    GT_BOOL  useBulk
)
{
    GT_U32      vlanIter  = 0;
    GT_U32      portIter  = 0;
    GT_U32      vlanCount = sizeof(prvTgfVlanArray)/sizeof(prvTgfVlanArray[0]);
    GT_STATUS   rc        = GT_OK;
    GT_IPADDR   ipAddr;
    PRV_TGF_IP_ROUTING_MODE_ENT routingMode;
    GT_TRUNK_ID senderTrunkId;/* trunk Id for the sender port that is member of */
    CPSS_TRUNK_MEMBER_STC   trunkMember;/* temporary trunk member */
    GT_U32                  numOfEnabledMembers;
    CPSS_TRUNK_MEMBER_STC   enabledMembersArray[PRV_CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS];
    GT_U32                  ii;
    GT_U8                   prvSendPortIndex;
    GT_U32                                  bulkVrId[2];
    GT_IPADDR                               bulkIpAddr[2];
    GT_U32                                  bulkPrefixLen[2];
    CPSS_PORTS_BMP_STC      portBitmap;


    PRV_TGF_PORT_INDEX_CHECK_FOR_VOID_FUNC_MAC(sendPortIndex);

    /* AUTODOC: RESTORE CONFIGURATION: */

    PRV_UTF_LOG0_MAC("======= Restoring Configuration =======\n");

    prvTgfBurstCount = PRV_TGF_BURST_COUNT_DEFAULT_CNS;/* restore value to default */

    /* delete the FDB entry */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d", prvTgfDevNum);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    if (useEPort == GT_TRUE)
    {
        /* restore ePort attributes configuration */
        rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                                           PRV_TGF_DESTINATION_EPORT_CNS,
                                                           &(prvTgfRestoreCfg.egressInfo));
        PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoSet);

        /* restore ePort mapping configuration */
        rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                               PRV_TGF_DESTINATION_EPORT_CNS,
                                                               &(prvTgfRestoreCfg.physicalInfo));
        PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortTargetMappingTableSet);
    }

    if(useEPortMappedToVidx == GT_TRUE)
    {
        useEPortMappedToVidx = GT_FALSE;

        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portBitmap);
        /* AUTODOC: delete the VIDX  */
        rc = prvTgfBrgMcEntryWrite(prvTgfDevNum, PRV_TGF_VIDX_FROM_EPORT_CNS, &portBitmap);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgMcEntryWrite: %d",
                                     prvTgfDevNum);

        /* restore ePort mapping configuration */
        rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                               PRV_TGF_DESTINATION_EPORT_CNS,
                                                               &(prvTgfRestoreCfg.physicalInfo));
        PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortTargetMappingTableSet);
    }

    if(tgfStackingSystemType != PRV_TGF_IPV4_STACKING_SYSTEM_AFTER_ROUTING_DEVICE_E)
    {
        /* -------------------------------------------------------------------------
         * 2. Restore Route Configuration
         */

        if (useBulk == GT_FALSE)
        {
            /* fill the destination IP address for Ipv4 prefix in Virtual Router */
            cpssOsMemCpy(ipAddr.arIP, prvTgfPacketIpv4Part.dstAddr, sizeof(ipAddr.arIP));

            /* AUTODOC: delete the Ipv4 prefix */
            rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, prvUtfVrfId, ipAddr, 32);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);
        }
        else
        {
            bulkVrId[0] = prvUtfVrfId;
            bulkVrId[1] = prvUtfVrfId;
            cpssOsMemCpy(&bulkIpAddr[0], prvTgfPacketIpv4Part.dstAddr, sizeof(ipAddr.arIP));
            cpssOsMemCpy(&bulkIpAddr[1], prvTgfPacket2Ipv4Part.dstAddr, sizeof(ipAddr.arIP));
            bulkPrefixLen[0] = bulkPrefixLen[1] = 32;
            /* AUTODOC: delete bulk of 2 Ipv4 prefixes */
            rc = prvTgfIpLpmIpv4UcPrefixBulkDel(prvTgfLpmDBId, bulkVrId, bulkIpAddr, bulkPrefixLen, 2);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixBulkDel: %d", prvTgfDevNum);
        }

        /* get routing mode */
        rc = prvTgfIpRoutingModeGet(&routingMode);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRoutingModeGet: %d", prvTgfDevNum);


        if (routingMode == PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E)
        {

            /* Routing from trunk support:
               check if the port is trunk member.
               when trunk member , set all the members with same configuration.
            */
            trunkMember.port = prvTgfPortsArray[sendPortIndex];
            trunkMember.hwDevice = prvTgfDevNum;

            rc = prvTgfTrunkDbIsMemberOfTrunk(prvTgfDevNum,&trunkMember,&senderTrunkId);
            if(rc == GT_OK)
            {
                /* the port is member of the trunk */

                /* get all trunk members */
                numOfEnabledMembers = PRV_CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS;
                rc = prvCpssGenericTrunkDbEnabledMembersGet(prvTgfDevNum,senderTrunkId,&numOfEnabledMembers,enabledMembersArray);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvCpssGenericTrunkDbEnabledMembersGet: %d %d",
                                             prvTgfDevNum, senderTrunkId);
                if(numOfEnabledMembers == 0)
                {
                    /* empty trunk ??? */
                    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, GT_EMPTY);
                    return;
                }

                for(ii = 0 ; ii < numOfEnabledMembers; ii++)
                {
                    /* enable Unicast IPv4 Routing on the trunk members */
                    rc = prvTgfDefPortsArrayPortToPortIndex(enabledMembersArray[ii].port,&prvSendPortIndex);
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfDefPortsArrayPortToPortIndex: %d", prvSendPortIndex);

                    rc = prvTgfIpPortRoutingEnable(prvSendPortIndex, CPSS_IP_UNICAST_E,
                                                   CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                                 enabledMembersArray[ii].hwDevice, enabledMembersArray[ii].port);
                }
            }
            else
            {
                /* AUTODOC: disable Unicast IPv4 Routing on port 0 */
                rc = prvTgfIpPortRoutingEnable(sendPortIndex, CPSS_IP_UNICAST_E,
                                               CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                             prvTgfDevNum, sendPortIndex);
            }
        }

        /* AUTODOC: disable IPv4 Unicast Routing on Vlan 5 */
        rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                     prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

        /* restore port inlif mode */
        rc = prvTgfInlifPortModeSet(sendPortIndex,prvTgfRestoreCfg.sendPortInlifMode);
        PRV_UTF_LOG_VERIFY_GT_OK(rc,prvTgfInlifPortModeSet);

        /* remove Virtual Router [prvUtfVrfId] */
        if (0 != prvUtfVrfId)
        {
            rc = prvTgfIpLpmVirtualRouterDel(prvTgfLpmDBId, prvUtfVrfId);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                         "prvTgfIpLpmVirtualRouterDel: %d",
                                         prvTgfDevNum);
        }

        /* enable Patricia trie validity */
        prvTgfIpValidityCheckEnable(GT_TRUE);
    }

    /* -------------------------------------------------------------------------
     * 1. Restore Base Configuration
     */

    /* delete port from both VLANs (support route from trunk A to trunk A) */
    for (vlanIter = 0; vlanIter < vlanCount; vlanIter++)
    {
        /* delete Ports from VLANs */
        for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
        {
            rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum, prvTgfVlanArray[vlanIter],
                                        prvTgfPortsArray[portIter]);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: %d, %d, %d",
                                         prvTgfDevNum, prvTgfVlanArray[vlanIter],
                                         prvTgfPortsArray[portIter]);
        }

        /* delete VLANs */

        /* AUTODOC: invalidate vlan entries 5,6 */
        rc = prvTgfBrgVlanEntryInvalidate(prvTgfVlanArray[vlanIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d, %d",
                                     prvTgfDevNum, prvTgfVlanArray[vlanIter]);
    }

    /* AUTODOC: disable VLAN based MAC learning */
    rc = prvTgfBrgVlanLearningStateSet(PRV_TGF_SEND_VLANID_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanLearningStateSet: %d", prvTgfDevNum);


    if(tgfStackingSystemType != PRV_TGF_IPV4_STACKING_SYSTEM_NONE_E)
    {
        prvTgfBasicIpv4UcRoutingInStackingSystemRestore(prvTgfPortsArray[sendPortIndex], prvTgfPortsArray[nextHopPortIndex]);
    }

    /* For SIP5 devices LPM Bypass is supported */
    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CPSS_PP_E_ARCH_CNS))
    {
       if(firstCallToLpmConfig==GT_FALSE)
       {
           rc = prvTgfLpmLastLookupStagesBypassEnableSet(prvTgfDevNum,prvTgfRestoreCfg.defaultBypassEnable);
           UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfLpmLastLookupStagesBypassEnableSet: %d", prvTgfDevNum);

           rc = prvTgfLpmPortSipLookupEnableSet(prvTgfDevNum, prvTgfPortsArray[sendPortIndex], prvTgfRestoreCfg.defaultSourceEportSipLookupEnable);
           UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChLpmPortSipLookupEnableSet: %d", prvTgfDevNum);

           rc = prvTgfIpPortSipSaEnableSet(prvTgfDevNum,prvTgfPortsArray[sendPortIndex], prvTgfRestoreCfg.defaultSourceEportSipSaEnable);
           UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                         "prvTgfIpPortSipSaEnableSet: %d", prvTgfDevNum);

           firstCallToLpmConfig=GT_TRUE;
       }
    }

    if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
    {
        /* AUTODOC: restore index to global MAC SA table in eport attribute table */
        rc =  prvTgfIpRouterPortGlobalMacSaIndexSet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS], prvTgfRestoreCfg.globalMacSaIndex);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpRouterPortGlobalMacSaIndexSet");
    }

    if(prvTgfRestoreCfg.macSaModeOrigInitialized)
    {
        /* AUTODOC: restore MAC SA LSB mode for port  */
        rc = prvTgfIpPortRouterMacSaLsbModeSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS], prvTgfRestoreCfg.macSaModeOrig);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChIpPortRouterMacSaLsbModeSet");
        prvTgfRestoreCfg.macSaModeOrigInitialized = 0;
    }

    if(prvTgfRestoreCfg.macSaModifyEnableOrigInitialized)
    {
        /* AUTODOC: restore MAC SA modify enable  */
        rc = prvTgfIpRouterMacSaModifyEnable(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS], prvTgfRestoreCfg.macSaModifyEnableOrig);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpRouterMacSaModifyEnable");
        prvTgfRestoreCfg.macSaModifyEnableOrigInitialized = 0;
    }

    if(prvTgfRestoreCfg.macSaBaseOrigInitialized)
    {
        /* AUTODOC: restore MAC SA Base for router  */
        rc = prvTgfIpRouterMacSaBaseSet(prvTgfDevNum, &prvTgfRestoreCfg.macSaBaseOrig);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpRouterMacSaBaseSet");
        prvTgfRestoreCfg.macSaBaseOrigInitialized = 0;
    }

    /* restore default VLAN 1 */
    prvTgfBrgVlanEntryRestore(1);
}

/**
* @internal prvTgfBasicIpv4UcRoutingAgeBitGet function
* @endinternal
*
* @brief   Read router next hop table age bits entry.
*/
GT_VOID prvTgfBasicIpv4UcRoutingAgeBitGet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      activityBit = GT_FALSE;

    /* AUTODOC: read router next hop table age bits entry: */
    rc = prvTgfIpRouterNextHopTableAgeBitsEntryRead(&activityBit);

    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBasicIpv4UcRoutingAgeBitGet: %d", prvTgfDevNum);

    /* AUTODOC:   verify Age Bit is set to 1 */
    UTF_VERIFY_EQUAL1_STRING_MAC(1, activityBit, "ERROR of prvTgfBasicIpv4UcRoutingAgeBitGet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfBasicIpv4UcRoutingUseEPortEnableSet function
* @endinternal
*
* @brief   Mark that ePort should be used in the test.
*
* @param[in] enable                   - GT_TRUE:  use ePort
*                                      GT_FALSE: don't use ePort
*                                       None
*
* @note Call this function before configuration set with GT_TRUE and after
*       configuration restore with GT_FALSE.
*
*/
GT_VOID prvTgfBasicIpv4UcRoutingUseEPortEnableSet
(
    GT_BOOL     enable
)
{
    /* AUTODOC: ePort 1000 is used in the test as nextHopPortNum 3 */
    useEPort = enable;
}

/**
* @internal prvTgfBasicIpv4UcMacSaRoutingConfigurationSet function
* @endinternal
*
* @brief   configure MAC SA assignment mode.
*
* @param[in] macSaMode                - CPSS_SA_LSB_PER_PORT_E,
*                                      CPSS_SA_LSB_PER_PKT_VID_E,
*                                      CPSS_SA_LSB_PER_VLAN_E,
*                                      CPSS_SA_LSB_FULL_48_BIT_GLOBAL
*                                       None
*/
GT_VOID prvTgfBasicIpv4UcMacSaRoutingConfigurationSet
(
    IN CPSS_MAC_SA_LSB_MODE_ENT    macSaMode
)
{
    GT_U32                   egrPort = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];
    GT_STATUS               rc = GT_OK;
    GT_ETHERADDR            macSa;
    GT_ETHERADDR            macSaGet;
    GT_U32                  globalMacSaIndexGet;
    GT_ETHERADDR            macAddr = {PRV_TGF_MAC_SA_BASE_CNS};

    if(prvTgfRestoreCfg.macSaModifyEnableOrigInitialized)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "tests error: missed config restore for prvTgfIpRouterMacSaModifyEnable");
    }
    else
    {
        /* AUTODOC: save MAC SA modification on port egrPort */
        rc = prvTgfIpRouterMacSaModifyEnableGet(prvTgfDevNum, egrPort, &prvTgfRestoreCfg.macSaModifyEnableOrig);
        prvTgfRestoreCfg.macSaModifyEnableOrigInitialized = 1;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpRouterMacSaModifyEnableGet");
    }

    /* AUTODOC: enable MAC SA modification on port egrPort */
    rc = prvTgfIpRouterMacSaModifyEnable(prvTgfDevNum, egrPort, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpRouterMacSaModifyEnable");


    if(prvTgfRestoreCfg.macSaModeOrigInitialized)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "tests error: missed config restore for prvTgfIpPortRouterMacSaLsbModeSet");
    }
    else
    {
        /* AUTODOC: save MAC SA LSB mode for port  */
        rc = prvTgfIpPortRouterMacSaLsbModeGet(prvTgfDevNum, egrPort, &prvTgfRestoreCfg.macSaModeOrig);
        prvTgfRestoreCfg.macSaModeOrigInitialized = 1;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpPortRouterMacSaLsbModeGet");
    }

    /* AUTODOC: set MAC SA LSB mode for port  */
    rc = prvTgfIpPortRouterMacSaLsbModeSet(prvTgfDevNum, egrPort, macSaMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpPortRouterMacSaLsbModeSet");

    if(prvTgfRestoreCfg.macSaBaseOrigInitialized)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "tests error: missed config restore for prvTgfIpRouterMacSaBaseSet");
    }
    else
    {
        /* AUTODOC: save MAC SA LSB mode for port  */
        rc = prvTgfIpRouterMacSaBaseGet(prvTgfDevNum, &prvTgfRestoreCfg.macSaBaseOrig);
        prvTgfRestoreCfg.macSaBaseOrigInitialized = GT_TRUE;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpRouterMacSaBaseGet");
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /* sip6 supports 12 bits ... not 8 like sip5 */
        prvTgfVlanMacSaArr[4] &= 0xF0;
        prvTgfVlanMacSaArr[4] |= (PRV_TGF_MAC_SA_LSB_CNS >> 8);
    }
    else
    {
        macSaLsb &= 0xFF;/* supports only 8 bits */
    }

    switch(macSaMode)
    {
        case CPSS_SA_LSB_PER_VLAN_E:
        case  CPSS_SA_LSB_PER_PKT_VID_E:

             /* AUTODOC: sets 40 MSBs of MAC SA base addr to 00:A1:B2:C3:D4 */
            rc = prvTgfIpRouterMacSaBaseSet(
                prvTgfDevNum, &macAddr);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpRouterMacSaBaseSet");

            /* AUTODOC: sets 8 LSB to 0xE5 Router MAC SA for VLAN */
            rc = prvTgfIpRouterVlanMacSaLsbSet(prvTgfDevNum,
                PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum) ? prvTgfNextHopeVlanid :/*sip6 supports all the eVlans*/
                                                   (prvTgfNextHopeVlanid & 0xFFF) ,
                PRV_TGF_MAC_SA_LSB_CNS);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpRouterVlanMacSaLsbSet");
            break;

        case CPSS_SA_LSB_PER_PORT_E:
            /* AUTODOC: sets 40 MSBs of MAC SA base addr to 00:A1:B2:C3:D4 */
            rc = prvTgfIpRouterMacSaBaseSet(
                prvTgfDevNum, &macAddr);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpRouterMacSaBaseSet");

            /* AUTODOC: sets 8 LSB to 0xE5 Router MAC SA for port */
            rc = prvTgfIpRouterPortMacSaLsbSet(
                prvTgfDevNum, egrPort, (GT_U8)PRV_TGF_MAC_SA_LSB_CNS);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpRouterPortMacSaLsbSet");
            break;

        case CPSS_SA_LSB_FULL_48_BIT_GLOBAL:
             /*  AUTODOC: configure tunnel start mac SA*/
            cpssOsMemCpy((GT_VOID*)macSa.arEther,
                         (GT_VOID*)prvTgfPacketL2Part.saMac,
                         sizeof(GT_ETHERADDR));

             /* AUTODOC: configure MAC SA to global MAC SA table into index 7*/
            rc =  prvTgfIpRouterGlobalMacSaSet(prvTgfDevNum,PRV_TGF_MAC_SA_GLOBAL_INDEX_CNS,&macSa);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpRouterGlobalMacSaSet");

            rc = prvTgfIpRouterGlobalMacSaGet(prvTgfDevNum, PRV_TGF_MAC_SA_GLOBAL_INDEX_CNS, &macSaGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterGlobalMacSaGet: %d", prvTgfDevNum);

            rc = cpssOsMemCmp(&macSa, &macSaGet, sizeof(GT_ETHERADDR)) == 0 ?
                GT_OK : GT_FALSE;
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "wrong mac sa data get");

            /* AUTODOC: save index to global MAC SA table in eport attribute table for restore */
            rc =  prvTgfIpRouterPortGlobalMacSaIndexGet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS], &prvTgfRestoreCfg.globalMacSaIndex);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpRouterPortGlobalMacSaIndexGet");

            /* AUTODOC: configure index to global MAC SA table in eport attribute table */
            rc =  prvTgfIpRouterPortGlobalMacSaIndexSet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],PRV_TGF_MAC_SA_GLOBAL_INDEX_CNS);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpRouterPortGlobalMacSaIndexSet");

            rc = prvTgfIpRouterPortGlobalMacSaIndexGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS], &globalMacSaIndexGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterPortGlobalMacSaIndexGet: %d", prvTgfDevNum);

            rc = (globalMacSaIndexGet == PRV_TGF_MAC_SA_GLOBAL_INDEX_CNS) ?
                GT_OK : GT_FALSE;
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "wrong mac sa index get");
            break;

    }
}


/**
* @internal prvTgfBasicIpv4UcMacSaRoutingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] sendPortNum              - port sending traffic
* @param[in] nextHopPortNum           - port receiving traffic
* @param[in] useSecondPkt             - whether to use the second packet (prvTgfPacket2PartArray)
* @param[in] macSaMode                - CPSS_SA_LSB_PER_PORT_E,
*                                      CPSS_SA_LSB_PER_PKT_VID_E,
*                                      CPSS_SA_LSB_PER_VLAN_E,
*                                      CPSS_SA_LSB_FULL_48_BIT_GLOBAL
*                                       None
*/
GT_VOID prvTgfBasicIpv4UcMacSaRoutingTrafficGenerate
(
    GT_U32    sendPortNum,
    GT_U32    nextHopPortNum,
    GT_BOOL  useSecondPkt,
    CPSS_MAC_SA_LSB_MODE_ENT macSaMode
)
{
    GT_STATUS                       rc          = GT_OK;
    GT_U32                          partsCount  = 0;
    GT_U32                          packetSize  = 0;
    TGF_PACKET_STC                  packetInfo;
    GT_U32                          portIter    = 0;
    GT_U32                          numTriggers = 0;
    TGF_VFD_INFO_STC                vfdArray[1];
    CPSS_INTERFACE_INFO_STC         portInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    PRV_TGF_IP_COUNTER_SET_STC      ipCounters;
    PRV_TGF_IP_ROUTING_MODE_ENT     routingMode;
    GT_TRUNK_ID currTrunkId;/* trunk Id for the current port */
    GT_TRUNK_ID senderTrunkId;/* trunk Id for the sender port that is member of */
    GT_TRUNK_ID nextHopTrunkId;/* trunk Id for the next hop port that is member of */
    CPSS_TRUNK_MEMBER_STC   trunkMember;/* temporary trunk member */
    GT_BOOL     stormingDetected;/*indicates that the traffic to check
                   LBH that egress the trunk was originally INGRESSED from the trunk.
                   but since in the 'enhanced UT' the CPU send traffic to a port
                   due to loopback it returns to it, we need to ensure that the
                   member mentioned here should get the traffic since it is the
                   'original sender'*/
    GT_U32      vlanIter;/*vlan iterator*/
    GT_U32      vlanCount = sizeof(prvTgfVlanArray)/sizeof(prvTgfVlanArray[0]);/*number of vlans*/
    GT_U32               numVfd = 0;/* number of VFDs in vfdArray */

    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* -------------------------------------------------------------------------
     * 1. Setup counters and enable capturing
     *
     */

    /* get routing mode */
    rc = prvTgfIpRoutingModeGet(&routingMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRoutingModeGet: %d", prvTgfDevNum);

    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        /* reset ethernet counters */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (routingMode == PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E)
        {
            /* reset IP couters and set ROUTE_ENTRY mode */
            rc = prvTgfCountersIpSet(prvTgfPortsArray[portIter], portIter);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCountersIpSet: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);
        }
    }

    /* Routing from trunk support:
       check if the port is trunk member.
       when trunk member , set all the members with same configuration.
    */
    trunkMember.port = sendPortNum;
    trunkMember.hwDevice = prvTgfDevNum;

    rc = prvTgfTrunkDbIsMemberOfTrunk(prvTgfDevNum,&trunkMember,&senderTrunkId);
    if(rc == GT_OK)
    {
    }
    else
    {
        senderTrunkId = 0;
    }

    /* Routing to trunk support:
       check if the NH port is trunk member.
       when trunk member --> use the trunk ID.
    */
    trunkMember.port = nextHopPortNum;
    trunkMember.hwDevice = prvTgfDevNum;

    rc = prvTgfTrunkDbIsMemberOfTrunk(prvTgfDevNum,&trunkMember,&nextHopTrunkId);
    if(rc == GT_OK)
    {
        /* the port is member of the trunk */

        /* setup nexthope portInterface for capturing */
        portInterface.type            = CPSS_INTERFACE_TRUNK_E;
        portInterface.trunkId         = nextHopTrunkId;

        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
        {
            /* send 64 packets in SIP5 due to new LBH of the trunk */
            prvTgfBurstCount = 64;
        }
        else
        {
            /*since we have 2 members in the NH trunk send 8 packets*/
            prvTgfBurstCount = 8;
        }

        vfdArray[0].modeExtraInfo = 0;
        vfdArray[0].offset = 6 + 5;/* last byte of the mac SA */
        cpssOsMemSet(vfdArray[0].patternPtr,0,sizeof(TGF_MAC_ADDR));
        vfdArray[0].patternPtr[0] = 0;
        vfdArray[0].cycleCount = 1;/*single byte*/
        vfdArray[0].mode = TGF_VFD_MODE_INCREMENT_E;

        numVfd = 1;
    }
    else
    {
        nextHopTrunkId = 0;

        /* setup nexthope portInterface for capturing */
        portInterface.type            = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum  = prvTgfDevNum;
        portInterface.devPort.portNum = nextHopPortNum;
    }

    if((senderTrunkId != 0) && (nextHopTrunkId == senderTrunkId))
    {

        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
        {
            /* send 64 packets in SIP5 due to new LBH of the trunk */
            prvTgfBurstCount = 64;
        }
        else
        {
            /*since we have 4 members in the NH trunk send 16 packets*/
            prvTgfBurstCount = 16;
        }

        /* make all trunk ports members of the vlans */
        for (vlanIter = 0; vlanIter < vlanCount; vlanIter++)
        {
            for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
            {
                rc = prvTgfBrgVlanMemberAdd(prvTgfDevNum, prvTgfVlanArray[vlanIter],
                                            prvTgfPortsArray[portIter], GT_FALSE);
                UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberAdd: %d, %d, %d, %d",
                                             prvTgfDevNum, prvTgfVlanArray[vlanIter],
                                             prvTgfPortsArray[portIter], GT_FALSE);
            }
        }

        /* we route from a trunk to the same trunk */
        /* the 'Mirroring capture' is not good because it sets :
           force PVID = 0 for all traffic that ingress the 'Captured port' .
           but since the 'sender' is also member of the 'NH trunk' this port must
           also be 'captured' , but this will not allow the packets from the CPU
           to initiate routing at all !

           so we use 'Special PCL capture' that 'skip' the ingress vlan (before the routing)
           by this the traffic before route can ingress the device but the
           traffic after the route will be captured to the CPU.
           */
        captureType = TGF_CAPTURE_MODE_PCL_E;

        /* state that no need to capture packets that are in the Vlan before the routing */
        rc = tgfTrafficGeneratorPortTxEthCapturePclExcludeVidSet(GT_TRUE,PRV_TGF_SEND_VLANID_CNS);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCapturePclExcludeVidSet: %d, %d\n",
                                     prvTgfDevNum, nextHopPortNum);
    }
    else
    {
        captureType = TGF_CAPTURE_MODE_MIRRORING_E;
    }


    /* enable capture on next hop port/trunk */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, captureType, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                 prvTgfDevNum, nextHopPortNum);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");


    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* number of parts in packet */
    if (useSecondPkt == GT_TRUE)
    {
        partsCount = sizeof(prvTgfPacket2PartArray) / sizeof(prvTgfPacket2PartArray[0]);
        /* calculate packet size */
        rc = prvTgfPacketSizeGet(prvTgfPacket2PartArray, partsCount, &packetSize);
    }
    else
    {
        partsCount = sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);
        /* calculate packet size */
        rc = prvTgfPacketSizeGet(prvTgfPacketPartArray, partsCount, &packetSize);
    }

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

    /* build packet */
    packetInfo.totalLen   = packetSize;
    packetInfo.numOfParts = partsCount;
    if (useSecondPkt == GT_TRUE)
    {
        packetInfo.partsArray = prvTgfPacket2PartArray;
    }
    else
    {
        packetInfo.partsArray = prvTgfPacketPartArray;
    }

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, prvTgfBurstCount, numVfd, &vfdArray[0]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* set transmit timings */
    rc = prvTgfSetTxSetup2Eth(0, 50);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetup2Eth: %d", prvTgfDevNum);

    /* AUTODOC: send packet from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:01, VID=5 */
    /* AUTODOC:   srcIP=1.1.1.1 */
    /* AUTODOC:   dstIP=1.1.1.3 for regular test, 1.1.1.3 or 3.2.2.3 for bulk test */

    /* send packet -- send from specific port -- even if member of a trunk */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, sendPortNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n",
                                 prvTgfDevNum, sendPortNum);
    cpssOsTimerWkAfter(1000);

    /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    /* read and check ethernet counters */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        GT_BOOL isOk;

        /* Routing trunk support:
           check if the port is trunk member.
        */
        trunkMember.port = prvTgfPortsArray[portIter];
        trunkMember.hwDevice = prvTgfDevNum;

        rc = prvTgfTrunkDbIsMemberOfTrunk(prvTgfDevNum,&trunkMember,&currTrunkId);
        if(rc == GT_OK)
        {
            /* this port is member of trunk */

            if(currTrunkId == nextHopTrunkId)
            {
                /* this port is member of the next hop trunk */

                /* we need to SUM all those ports together before we check */
                /* we do the check in trunk dedicated function for 'target trunk'*/
                /* see after this loop */
                continue;
            }
        }

        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);



        /* AUTODOC: verify routed packet on port 3 */
        if( (prvTgfPortsArray[portIter]==sendPortNum) || (prvTgfPortsArray[portIter]==nextHopPortNum))
        {
            /* packetSize is not changed */
                expectedCntrs.goodOctetsSent.l[0] = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;
        }
        else
        {
                 /* for other ports */
                expectedCntrs.goodOctetsSent.l[0] = 0;
                expectedCntrs.goodPktsSent.l[0]   = 0;
                expectedCntrs.ucPktsSent.l[0]     = 0;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = 0;
                expectedCntrs.goodPktsRcv.l[0]    = 0;
                expectedCntrs.ucPktsRcv.l[0]      = 0;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;
        }

        isOk =
            portCntrs.goodOctetsSent.l[0] == expectedCntrs.goodOctetsSent.l[0] &&
            portCntrs.goodPktsSent.l[0]   == expectedCntrs.goodPktsSent.l[0] &&
            portCntrs.ucPktsSent.l[0]     == expectedCntrs.ucPktsSent.l[0] &&
            portCntrs.brdcPktsSent.l[0]   == expectedCntrs.brdcPktsSent.l[0] &&
            portCntrs.mcPktsSent.l[0]     == expectedCntrs.mcPktsSent.l[0] &&
            portCntrs.goodOctetsRcv.l[0]  == expectedCntrs.goodOctetsRcv.l[0] &&
            portCntrs.goodPktsRcv.l[0]    == expectedCntrs.goodPktsRcv.l[0] &&
            portCntrs.ucPktsRcv.l[0]      == expectedCntrs.ucPktsRcv.l[0] &&
            portCntrs.brdcPktsRcv.l[0]    == expectedCntrs.brdcPktsRcv.l[0] &&
            portCntrs.mcPktsRcv.l[0]      == expectedCntrs.mcPktsRcv.l[0];

        UTF_VERIFY_EQUAL0_STRING_MAC(isOk, GT_TRUE, "get another counters values.");

        /* print expected values if bug */
        if (isOk != GT_TRUE) {
            PRV_UTF_LOG0_MAC("Expected values:\n");
            PRV_UTF_LOG1_MAC("  goodOctetsSent = %d\n", expectedCntrs.goodOctetsSent.l[0]);
            PRV_UTF_LOG1_MAC("    goodPktsSent = %d\n", expectedCntrs.goodPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("      ucPktsSent = %d\n", expectedCntrs.ucPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("    brdcPktsSent = %d\n", expectedCntrs.brdcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("      mcPktsSent = %d\n", expectedCntrs.mcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("   goodOctetsRcv = %d\n", expectedCntrs.goodOctetsRcv.l[0]);
            PRV_UTF_LOG1_MAC("     goodPktsRcv = %d\n", expectedCntrs.goodPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC("       ucPktsRcv = %d\n", expectedCntrs.ucPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC("     brdcPktsRcv = %d\n", expectedCntrs.brdcPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC("       mcPktsRcv = %d\n", expectedCntrs.mcPktsRcv.l[0]);
            PRV_UTF_LOG0_MAC("\n");
        }
    }

    if(nextHopTrunkId)
    {
        /* the traffic should egress the next hop trunk */
        /* we need to SUM all those ports together before we check */
        /* we do the check in trunk dedicated function for 'target trunk'*/
        if(nextHopTrunkId == senderTrunkId)
        {
            trunkMember.port = sendPortNum;
            trunkMember.hwDevice = prvTgfDevNum;

            prvTgfTrunkLoadBalanceCheck(nextHopTrunkId,
                PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_EVEN_E,
                prvTgfBurstCount,
                0,/*tolerance*/
                &trunkMember,/* the port in the trunk that sent the traffic */
                &stormingDetected);/*was storming detected*/

            /* it will detect 'storming' but those are only the LBH of this port */
        }
        else
        {
            prvTgfTrunkLoadBalanceCheck(nextHopTrunkId,
                PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_EVEN_E,
                prvTgfBurstCount,
                0,/*tolerance*/
                NULL,/*NA*/
                NULL);/*NA*/
        }
    }


    /* -------------------------------------------------------------------------
     * 4. Get Trigger Counters
     */

    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", nextHopPortNum);

    /* get trigger counters where packet has MAC DA as prvTgfArpMac */
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 0;
    vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
    cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

    /* disable capture on nexthope port , before check the packets to the CPU */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, captureType, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                 prvTgfDevNum, nextHopPortNum);

    /* check the packets to the CPU */
    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
    PRV_UTF_LOG2_MAC("    numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

    /* check TriggerCounters */
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    if(portInterface.type  == CPSS_INTERFACE_PORT_E)
    {
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                     portInterface.devPort.hwDevNum, portInterface.devPort.portNum);
    }
    else
    {
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, trunkId [%d] \n",
                                     prvTgfDevNum, nextHopTrunkId);
    }

    if(prvTgfBurstCount >= 32)
    {
        /* AUTODOC: for more than 32 transmits check that got only first 32 matches of the MAC of packet */
        UTF_VERIFY_EQUAL0_STRING_MAC(0xFFFFFFFF, numTriggers,
            "\n for more than 32 transmits check that got only first 32 matches of the MAC of packet \n");
    }
    else
    {
        /* AUTODOC: check if packet has the same MAC DA as ARP MAC */
        /* number of triggers should be according to number of transmit*/
        UTF_VERIFY_EQUAL6_STRING_MAC(((1<<prvTgfBurstCount)-1), numTriggers,
                "\n   MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X",
                prvTgfArpMac[0], prvTgfArpMac[1], prvTgfArpMac[2],
                prvTgfArpMac[3], prvTgfArpMac[4], prvTgfArpMac[5]);
    }

    /* get trigger counters where packet has MAC Sa modify */
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 6;
    vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);


    switch(macSaMode)
    {
        case CPSS_SA_LSB_PER_VLAN_E:
            cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfVlanMacSaArr, sizeof(TGF_MAC_ADDR));
            rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
            PRV_UTF_LOG2_MAC("    numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

            if(prvTgfBurstCount >= 32)
            {
                /* AUTODOC: for more than 32 transmits check that got only first 32 matches of the MAC of packet */
                UTF_VERIFY_EQUAL0_STRING_MAC(0xFFFFFFFF, numTriggers,
                "\n for more than 32 transmits check that got only first 32 matches of the MAC of packet \n");
            }
            else
            {
                /* AUTODOC: check if packet has the same MAC SA as needed */
                /* number of triggers should be according to number of transmit*/
                UTF_VERIFY_EQUAL6_STRING_MAC(((1<<prvTgfBurstCount)-1), numTriggers,
                "\n   MAC SA of captured packet must be: %00X:%A1X:%B2X:%C3X:%D4X:%E5X",
                prvTgfVlanMacSaArr[0], prvTgfVlanMacSaArr[1], prvTgfVlanMacSaArr[2],
                prvTgfVlanMacSaArr[3], prvTgfVlanMacSaArr[4], prvTgfVlanMacSaArr[5]);
            }


            break;

    case  CPSS_SA_LSB_PER_PKT_VID_E:

            cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfVidMacSaArr, sizeof(TGF_MAC_ADDR));
            rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
            PRV_UTF_LOG2_MAC("    numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

            if(prvTgfBurstCount >= 32)
            {
                /* AUTODOC: for more than 32 transmits check that got only first 32 matches of the MAC of packet */
                UTF_VERIFY_EQUAL0_STRING_MAC(0xFFFFFFFF, numTriggers,
                "\n for more than 32 transmits check that got only first 32 matches of the MAC of packet \n");
            }
            else
            {
                /* AUTODOC: check if packet has the same MAC SA as needed */
                /* number of triggers should be according to number of transmit*/
                UTF_VERIFY_EQUAL6_STRING_MAC(((1<<prvTgfBurstCount)-1), numTriggers,
                "\n   MAC SA of captured packet must be: %00X:%A1X:%B2X:%C3X:%D0X:%06X",
                prvTgfVidMacSaArr[0], prvTgfVidMacSaArr[1], prvTgfVidMacSaArr[2],
                prvTgfVidMacSaArr[3], prvTgfVidMacSaArr[4], prvTgfVidMacSaArr[5]);
            }


            break;


    case CPSS_SA_LSB_PER_PORT_E:

            cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfVlanMacSaArr, sizeof(TGF_MAC_ADDR));
            rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
            PRV_UTF_LOG2_MAC("    numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

            if(prvTgfBurstCount >= 32)
            {
                /* AUTODOC: for more than 32 transmits check that got only first 32 matches of the MAC of packet */
                UTF_VERIFY_EQUAL0_STRING_MAC(0xFFFFFFFF, numTriggers,
                "\n for more than 32 transmits check that got only first 32 matches of the MAC of packet \n");
            }
            else
            {
                /* AUTODOC: check if packet has the same MAC SA as needed */
                /* number of triggers should be according to number of transmit*/
                UTF_VERIFY_EQUAL6_STRING_MAC(((1<<prvTgfBurstCount)-1), numTriggers,
                "\n   MAC SA of captured packet must be: %00X:%A1X:%B2X:%C3X:%D4X:%E5X",
                prvTgfVlanMacSaArr[0], prvTgfVlanMacSaArr[1], prvTgfVlanMacSaArr[2],
                prvTgfVlanMacSaArr[3], prvTgfVlanMacSaArr[4], prvTgfVlanMacSaArr[5]);
            }

            break;

    case CPSS_SA_LSB_FULL_48_BIT_GLOBAL:

            cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfGlobalMacSaArr, sizeof(TGF_MAC_ADDR));
            rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
            PRV_UTF_LOG2_MAC("    numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

            if(prvTgfBurstCount >= 32)
            {
                /* AUTODOC: for more than 32 transmits check that got only first 32 matches of the MAC of packet */
                UTF_VERIFY_EQUAL0_STRING_MAC(0xFFFFFFFF, numTriggers,
                "\n for more than 32 transmits check that got only first 32 matches of the MAC of packet \n");
            }
            else
            {
                /* AUTODOC: check if packet has the same MAC SA as needed */
                /* number of triggers should be according to number of transmit*/
                UTF_VERIFY_EQUAL6_STRING_MAC(((1<<prvTgfBurstCount)-1), numTriggers,
                "\n   MAC SA of captured packet must be: %00X:%00X:%00X:%00X:%00X:%01X",
                prvTgfGlobalMacSaArr[0], prvTgfGlobalMacSaArr[1], prvTgfGlobalMacSaArr[2],
                prvTgfGlobalMacSaArr[3], prvTgfGlobalMacSaArr[4], prvTgfGlobalMacSaArr[5]);
            }

            break;

    }

    /* -------------------------------------------------------------------------
     * 5. Get IP Counters
     */

    /* AUTODOC: get and print ip counters values */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++) {
        PRV_UTF_LOG1_MAC("IP counters for Port [%d]:\n", prvTgfPortsArray[portIter]);
        prvTgfCountersIpGet(prvTgfDevNum, portIter, GT_TRUE, &ipCounters);
    }
    PRV_UTF_LOG0_MAC("\n");

    /* restore transmit timings */
    rc = prvTgfSetTxSetup2Eth(0, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetup2Eth: %d", prvTgfDevNum);

    /* disable the PCL exclude vid capturing */
    rc = tgfTrafficGeneratorPortTxEthCapturePclExcludeVidSet(GT_FALSE,0/*don't care*/);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCapturePclExcludeVidSet: %d, %d\n",
                                 prvTgfDevNum, nextHopPortNum);

    /* just for 'cleanup' */
    captureType = TGF_CAPTURE_MODE_MIRRORING_E;
}


/**
* @internal prvTgfBasicIpv4UcRoutingInStackingSystemEnable function
* @endinternal
*
* @brief   state the type of stacking system.
*         to allow current device Y get DSA packet from srcHwDev X , and device Y need to do routing.
*         the target port of the route is on device X.
*         the test checks the DSA tag that device Y send towards device X.
*         need to see that the 'srcHwDev' field in the DSA changed from 'X' to 'Y' so
*         device X will not do filter of packet 'from me from cascade port'
* @param[in] stackingSystemType       - indication that type of stacking system.
*                                       None
*/
GT_VOID prvTgfBasicIpv4UcRoutingInStackingSystemEnable
(
    IN PRV_TGF_IPV4_STACKING_SYSTEM_ENT  stackingSystemType
)
{
    tgfStackingSystemType = stackingSystemType;
}


/**
* @internal prvTgfIpv4UcRoutingDefaultOverrideRouteConfigurationSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
*                                      sendPortNum     - port sending traffic
* @param[in] nextHopPortNum           - port receiving traffic
* @param[in] beforeDefaultOverride    - GT_TRUE: before default override
*                                      GT_FALSE: after default override
*                                       None
*/
GT_VOID prvTgfIpv4UcRoutingDefaultOverrideRouteConfigurationSet
(
    GT_U32   prvUtfVrfId,
    GT_U8    sendPortIndex,
    GT_U32    nextHopPortNum,
    GT_BOOL  beforeDefaultOverride
)
{
    GT_STATUS                               rc = GT_OK;
    GT_ETHERADDR                            arpMacAddr;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           routeEntriesArray[1];
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           *regularEntryPtr;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_IPADDR                               ipAddr;
    CPSS_INTERFACE_INFO_STC                 physicalInfo;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC        egressInfo;
    GT_HW_DEV_NUM                           targetHwDevice;
    GT_PORT_NUM                             targetPort;

    PRV_TGF_PORT_INDEX_CHECK_FOR_VOID_FUNC_MAC(sendPortIndex);

    PRV_UTF_LOG0_MAC("==== Setting LTT Route Configuration ====\n");

    if(tgfStackingSystemType != PRV_TGF_IPV4_STACKING_SYSTEM_NONE_E)
    {
        targetHwDevice = prvTgfDsaTargetHwDevNum;
        targetPort   = prvTgfDsaTargetPortNum;
        PRV_UTF_LOG2_MAC("do route to remote device[0x%x] , remote port [0x%x]\n",
            targetHwDevice,targetPort);
    }
    else
    {
        targetHwDevice = prvTgfDevNum;
        targetPort = nextHopPortNum;
    }
    if (beforeDefaultOverride == GT_TRUE)
    {
        if (useEPort == GT_TRUE)
        {
            /* save ePort attributes configuration */
            rc = prvTgfBrgEportToPhysicalPortEgressPortInfoGet(prvTgfDevNum,
                                                               PRV_TGF_DESTINATION_EPORT_CNS,
                                                               &(prvTgfRestoreCfg.egressInfo));
            PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoGet);

            /* set ePort attributes configuration */
            cpssOsMemSet(&egressInfo,0,sizeof(egressInfo));
            rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                                               PRV_TGF_DESTINATION_EPORT_CNS,
                                                               &egressInfo);
            PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoSet);

            /* save ePort mapping configuration */
            rc = prvTgfBrgEportToPhysicalPortTargetMappingTableGet(prvTgfDevNum,
                                                                   PRV_TGF_DESTINATION_EPORT_CNS,
                                                                   &(prvTgfRestoreCfg.physicalInfo));
            PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortTargetMappingTableGet);

            /* set ePort mapping configuration */
            physicalInfo.type = CPSS_INTERFACE_PORT_E;

            physicalInfo.devPort.hwDevNum = targetHwDevice;
            physicalInfo.devPort.portNum = targetPort;
            rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                                   PRV_TGF_DESTINATION_EPORT_CNS,
                                                                   &physicalInfo);
            PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortTargetMappingTableSet);
        }
        /* -------------------------------------------------------------------------
         * 1. Enable Routing
         */
        /* AUTODOC: enable Unicast IPv4 Routing on port 0 */
        rc = prvTgfIpPortRoutingEnable(sendPortIndex, CPSS_IP_UNICAST_E,
                                       CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                     prvTgfDevNum, sendPortIndex);

        /* AUTODOC: update VRF Id in vlan[PRV_TGF_SEND_VLANID_CNS] */
        rc = prvTgfBrgVlanVrfIdSet(PRV_TGF_SEND_VLANID_CNS, prvUtfVrfId);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanVrfIdSet: %d %d",
                                     prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

        /* AUTODOC: enable IPv4 Unicast Routing on Vlan 5 */
        rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                     prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

        /* -------------------------------------------------------------------------
         * 2. Create the Route entry (Next hop) in Route table and Router ARP Table
         */

        /* AUTODOC: write a ARP MAC 00:00:00:00:00:22 to the Router ARP Table */
        cpssOsMemCpy(arpMacAddr.arEther, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

        rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterArpIndex, &arpMacAddr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

        /* write a UC Route entry to the Route Table */
        cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));

        regularEntryPtr = &routeEntriesArray[0];
        regularEntryPtr->cmd                        = CPSS_PACKET_CMD_ROUTE_E;
        regularEntryPtr->cpuCodeIndex               = 0;
        regularEntryPtr->appSpecificCpuCodeEnable   = GT_FALSE;
        regularEntryPtr->unicastPacketSipFilterEnable = GT_FALSE;
        regularEntryPtr->ttlHopLimitDecEnable       = GT_FALSE;
        regularEntryPtr->ttlHopLimDecOptionsExtChkByPass = GT_FALSE;
        regularEntryPtr->ingressMirror              = GT_FALSE;
        regularEntryPtr->qosProfileMarkingEnable    = GT_FALSE;
        regularEntryPtr->qosProfileIndex            = 0;
        regularEntryPtr->qosPrecedence              = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
        regularEntryPtr->modifyUp                   = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
        regularEntryPtr->modifyDscp                 = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
        regularEntryPtr->countSet                   = PRV_TGF_COUNT_SET_CNS;
        regularEntryPtr->trapMirrorArpBcEnable      = GT_FALSE;
        regularEntryPtr->sipAccessLevel             = 0;
        regularEntryPtr->dipAccessLevel             = 0;
        regularEntryPtr->ICMPRedirectEnable         = GT_FALSE;
        regularEntryPtr->scopeCheckingEnable        = GT_FALSE;
        regularEntryPtr->siteId                     = CPSS_IP_SITE_ID_INTERNAL_E;
        regularEntryPtr->mtuProfileIndex            = 0;
        regularEntryPtr->isTunnelStart              = GT_FALSE;
        regularEntryPtr->nextHopVlanId              = prvTgfNextHopeVlanid;
        /* Routing to trunk support:
           check if the NH port is trunk member.
           when trunk member --> use the trunk ID.
        */
        regularEntryPtr->nextHopInterface.type      = CPSS_INTERFACE_PORT_E;
        regularEntryPtr->nextHopInterface.devPort.hwDevNum = targetHwDevice;

        if (useEPort == GT_TRUE)
            regularEntryPtr->nextHopInterface.devPort.portNum = PRV_TGF_DESTINATION_EPORT_CNS;
        else
            regularEntryPtr->nextHopInterface.devPort.portNum = targetPort;
        regularEntryPtr->nextHopARPPointer          = prvTgfRouterArpIndex;
        regularEntryPtr->nextHopTunnelPointer       = 0;

        /* AUTODOC: read and save the ipv4 uc default route entry */
        cpssOsMemSet(routeEntriesArrayGet0, 0, sizeof(routeEntriesArrayGet0));
        rc = prvTgfIpUcRouteEntriesRead(prvTgfDevNum, 0, routeEntriesArrayGet0, 1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesRead: %d", prvTgfDevNum);

        /* AUTODOC: add UC route entry with nexthop VLAN 6 and nexthop port 3 */
        rc = prvTgfIpUcRouteEntriesWrite(0, routeEntriesArray, 1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);

        /* AUTODOC: read and save the ipv4 uc  route entry */
        cpssOsMemSet(routeEntriesArrayGet1, 0, sizeof(routeEntriesArrayGet1));
        rc = prvTgfIpUcRouteEntriesRead(prvTgfDevNum, prvTgfRouteEntryBaseIndex, routeEntriesArrayGet1, 1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesRead: %d", prvTgfDevNum);
        regularEntryPtr->cmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
        /* AUTODOC: add UC route entry with nexthop VLAN 6 and nexthop port 3 */
        rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndex, routeEntriesArray, 1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);
    }
    else
    {
        /* fill a nexthope info for the prefix */
        cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
        nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex;
        nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;
        ipAddr.arIP[0] = 240;
        ipAddr.arIP[1] = 0;
        ipAddr.arIP[2] = 0;
        ipAddr.arIP[3] = 0;

        rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, prvUtfVrfId,
                                        ipAddr, 4, &nextHopInfo, GT_TRUE, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);
    }

}






/**
* @internal prvTgfIpv4UcRoutingDefaultOverrideConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @param[in] prvUtfVrfId              - virtual router index
* @param[in] sendPortIndex            - index of port sending traffic
*                                       None
*
* @note 2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfIpv4UcRoutingDefaultOverrideConfigurationRestore
(
    GT_U32   prvUtfVrfId,
    GT_U8    sendPortIndex
)
{
    GT_U32      vlanIter  = 0;
    GT_U32      portIter  = 0;
    GT_U32      vlanCount = sizeof(prvTgfVlanArray)/sizeof(prvTgfVlanArray[0]);
    GT_STATUS   rc        = GT_OK;
    GT_IPADDR   ipAddr;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;


    PRV_TGF_PORT_INDEX_CHECK_FOR_VOID_FUNC_MAC(sendPortIndex);

    /* AUTODOC: RESTORE CONFIGURATION: */

    PRV_UTF_LOG0_MAC("======= Restoring Configuration =======\n");

    prvTgfBurstCount = PRV_TGF_BURST_COUNT_DEFAULT_CNS;/* restore value to default */

    /* delete the FDB entry */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d", prvTgfDevNum);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    if (useEPort == GT_TRUE)
    {
        /* restore ePort attributes configuration */
        rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                                           PRV_TGF_DESTINATION_EPORT_CNS,
                                                           &(prvTgfRestoreCfg.egressInfo));
        PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoSet);

        /* restore ePort mapping configuration */
        rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                               PRV_TGF_DESTINATION_EPORT_CNS,
                                                               &(prvTgfRestoreCfg.physicalInfo));
        PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortTargetMappingTableSet);
    }

    /* -------------------------------------------------------------------------
     * 2. Restore Route Configuration
     */

    /* AUTODOC: restore default ipv4 uc route entry */
    rc = prvTgfIpUcRouteEntriesWrite(0, routeEntriesArrayGet0, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);

    /* AUTODOC: restore route entry in  prvTgfRouteEntryBaseIndex */
    rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndex, routeEntriesArrayGet1, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);


    /* fill a nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = 0;
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;
    ipAddr.arIP[0] = 240;
    ipAddr.arIP[1] = 0;
    ipAddr.arIP[2] = 0;
    ipAddr.arIP[3] = 0;
    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, prvUtfVrfId,
                                    ipAddr, 4, &nextHopInfo, GT_TRUE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);

    /* AUTODOC: disable Unicast IPv4 Routing on port 0 */
    rc = prvTgfIpPortRoutingEnable(sendPortIndex, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                 prvTgfDevNum, sendPortIndex);

    /* AUTODOC: disable IPv4 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* enable Patricia trie validity */
    prvTgfIpValidityCheckEnable(GT_TRUE);

    /* -------------------------------------------------------------------------
     * 1. Restore Base Configuration
     */

    /* delete port from both VLANs (support route from trunk A to trunk A) */
    for (vlanIter = 0; vlanIter < vlanCount; vlanIter++)
    {
        /* delete Ports from VLANs */
        for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
        {
            rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum, prvTgfVlanArray[vlanIter],
                                        prvTgfPortsArray[portIter]);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: %d, %d, %d",
                                         prvTgfDevNum, prvTgfVlanArray[vlanIter],
                                         prvTgfPortsArray[portIter]);
        }

        /* delete VLANs */

        /* AUTODOC: invalidate vlan entries 5,6 */
        rc = prvTgfBrgVlanEntryInvalidate(prvTgfVlanArray[vlanIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d, %d",
                                     prvTgfDevNum, prvTgfVlanArray[vlanIter]);
    }

    /* AUTODOC: disable VLAN based MAC learning */
    rc = prvTgfBrgVlanLearningStateSet(PRV_TGF_SEND_VLANID_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanLearningStateSet: %d", prvTgfDevNum);
}



/**
* @internal prvTgfIpv4UcRollBackCheck function
* @endinternal
*
* @brief   This functions cause to failers during prefix insertion and
*         check rollback
*/
GT_VOID   prvTgfIpv4UcRollBackCheck(GT_VOID)
{
    GT_STATUS   rc        = GT_OK;
    GT_IPADDR   ipAddr;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_U32   prvUtfVrfId = 0;
    GT_U32   prvTgfLpmDBId = 0;
    GT_U32   prefixLength = 0;
    GT_U32   seed = 0;
    GT_U32   maxHwWriteNumber = 0;
    GT_U32   maxMallocNumber = 0;
    GT_U32   failedSeqNumber = 0;
    /* get random seed */
    seed = prvUtfSeedFromStreamNameGet();
    /* set specific seed for random generator */
    cpssOsSrand(seed);
    /* fill a nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = 19;
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;
    ipAddr.arIP[0] = 1;
    ipAddr.arIP[1] = cpssOsRand()% 255;
    ipAddr.arIP[2] = cpssOsRand()% 255;
    ipAddr.arIP[3] = cpssOsRand()% 255;
    prvUtfVrfId = 0;
    prvTgfLpmDBId = 0;

    while(0 == prefixLength) /* the prefix len should not be zero */
    {
        prefixLength = cpssOsRand()% 32;
    }

    /* enable calculation of hw writes number */
    rc = appDemoHwAccessFailerBind(GT_TRUE,0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "appDemoHwAccessFailerBind: %d", prvTgfDevNum);
    /* enable calculation of malloc allocations */
    rc = osDbgMallocFailSet(GT_TRUE,0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "osDbgMallocFailSet: %d", prvTgfDevNum);

    /* don't make validation function after prefix insertion */
    prvTgfIpValidityCheckEnable(GT_FALSE);
    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, prvUtfVrfId,
                                    ipAddr, prefixLength, &nextHopInfo, GT_TRUE, GT_FALSE);
    /* don't make validation function after prefix insertion */
    prvTgfIpValidityCheckEnable(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);
    /* get hw writes number during add prefix operation */
    rc = appDemoHwAccessCounterGet(&maxHwWriteNumber);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "appDemoHwAccessCounterGet: %d", prvTgfDevNum);
    /* get number of malloca allocations during add prefix operation*/
    rc = osMemGetMallocAllocationCounter(&maxMallocNumber);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "osMemGetMallocAllocationCounter: %d", prvTgfDevNum);

    rc =  prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId,prvUtfVrfId,ipAddr,prefixLength);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);

    /* calculate which hw write would fail */
    if( 0 == maxHwWriteNumber)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "zero maxHwWriteNumber, should never happen");
    }
    else
    {
        failedSeqNumber = cpssOsRand()% maxHwWriteNumber;
    }

    if (failedSeqNumber == 0)
    {
        failedSeqNumber++;
    }
    rc = appDemoHwAccessFailerBind(GT_TRUE,failedSeqNumber);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "appDemoHwAccessFailerBind: %d", prvTgfDevNum);
    /* This operation is going to fail */
    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, prvUtfVrfId,
                                    ipAddr, prefixLength, &nextHopInfo, GT_TRUE, GT_FALSE);
    /* deactivate hw fail mechanism */
    rc = appDemoHwAccessFailerBind(GT_FALSE,0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "appDemoHwAccessFailerBind: %d", prvTgfDevNum);


    rc = prvTgfIpLpmValidityCheck(prvTgfLpmDBId,prvUtfVrfId,CPSS_IP_PROTOCOL_IPV4_E,CPSS_UNICAST_E,GT_TRUE);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmValidityCheck FAILED, rc = [%d]", rc);
    }
    /* generate traffic for previous prefix:*/
    prvTgfBasicIpv4UcRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS);
    /* do test with CPU memory allocation problem */
    /* calculate which malloc allocation will fail */
    if( 0 == maxMallocNumber)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "zero maxMallocNumber, should never happen");
    }
    else
    {
        failedSeqNumber = cpssOsRand()% maxMallocNumber;
    }

    if (failedSeqNumber == 0)
    {
        failedSeqNumber++;
    }
    rc = osDbgMallocFailSet(GT_TRUE,failedSeqNumber);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "osDbgMallocFailSet: %d", prvTgfDevNum);
    /* This operation is going to fail */
    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, prvUtfVrfId,
                                    ipAddr, prefixLength, &nextHopInfo, GT_TRUE, GT_FALSE);
    /* deactivate allocation fail mechanism */
    rc = osDbgMallocFailSet(GT_FALSE,0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "osDbgMallocFailSet: %d", prvTgfDevNum);
    rc = prvTgfIpLpmValidityCheck(prvTgfLpmDBId,prvUtfVrfId,CPSS_IP_PROTOCOL_IPV4_E,CPSS_UNICAST_E,GT_TRUE);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmValidityCheck FAILED, rc = [%d]", rc);
    }
    /* generate traffic for previous prefix:*/
    prvTgfBasicIpv4UcRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS);

}

/**
* @internal prvTgfBasicIpv4UcRoutingUnicastRpfEnableSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
* @param[in] prvUntUrpfMode           - unicast RPF mode
*                                       None
*/
GT_VOID prvTgfBasicIpv4UcRoutingUnicastRpfEnableSet
(
    GT_U32                            prvUtfVrfId,
    PRV_TGF_URPF_MODE_ENT             prvUntUrpfMode
)
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_IPADDR                               ipAddr;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           routeEntry[1];
    PRV_TGF_IP_EXC_CMD_ENT                  exceptionCmd;
    GT_U32                                  origNextHopPort;

    PRV_UTF_LOG0_MAC("==== Setting uRPF Configuration ====\n");

    /* -------------------------------------------------------------------------
     * 0. Update existing Ipv4 prefix in a Virtual Router for the specified LPM DB
     */

    /* fill a nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex;
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;
    nextHopInfo.ipLttEntry.ucRPFCheckEnable = GT_FALSE;

    /* fill a source IP address for the prefix */
    cpssOsMemCpy(ipAddr.arIP, prvTgfPacketIpv4Part.srcAddr, sizeof(ipAddr.arIP));

    /* AUTODOC: Add IPv4 UC prefix 1.1.1.1/32 */
    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, prvUtfVrfId,
                                    ipAddr, 32, &nextHopInfo, GT_TRUE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);

    /* fill a target IP address for the prefix */
    cpssOsMemCpy(ipAddr.arIP, prvTgfPacketIpv4Part.dstAddr, sizeof(ipAddr.arIP));

    /* AUTODOC: Update IPv4 UC prefix 1.1.1.3/32 */
    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, prvUtfVrfId,
                                    ipAddr, 32, &nextHopInfo, GT_TRUE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);

    rc = prvTgfIpExceptionCommandGet(prvTgfDevNum, PRV_TGF_IP_EXCP_RPF_FAIL_E, CPSS_IP_PROTOCOL_IPV4_E, CPSS_IP_UNICAST_E, &exceptionCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpExceptionCommandGet: %d", prvTgfDevNum);

    switch (prvUntUrpfMode)
    {
    case PRV_TGF_EVLAN_BASED_URPF_MODE_E:

        PRV_UTF_LOG0_MAC("==== Setting eVLAN Based uRPF Configuration ====\n");
        rc = prvTgfIpUcRpfModeSet(prvTgfDevNum,
                            PRV_TGF_SEND_VLANID_CNS,
                            PRV_TGF_IP_URPF_VLAN_MODE_E);

        /* AUTODOC: expect no traffic */
        prvTgfBasicIpv4UcRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, GT_TRUE, GT_TRUE);

        /* AUTODOC: read the UC Route entry from the Route Table and change vlan */
        cpssOsMemSet(routeEntry, 0, sizeof(routeEntry));
        rc = prvTgfIpUcRouteEntriesRead(prvTgfDevNum, prvTgfRouteEntryBaseIndex, routeEntry, 1);
        routeEntry->nextHopVlanId = PRV_TGF_SEND_VLANID_CNS;

        /* AUTODOC: add UC route entry with nexthop VLAN 5 and nexthop port 3 */
        rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndex, routeEntry, 1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);

        /* AUTODOC: compare packet eVlan & nextHop entry eVlan, if not equal assign IPv4 uRPF fail packet command */
        rc = prvTgfIpExceptionCommandSet(prvTgfDevNum, PRV_TGF_IP_EXCP_RPF_FAIL_E, CPSS_IP_PROTOCOL_IPV4_E, CPSS_IP_UNICAST_E, PRV_TGF_IP_EXC_CMD_DROP_HARD_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpExceptionCommandSet: %d", prvTgfDevNum);

        /* AUTODOC: expect traffic */
        prvTgfBasicIpv4UcRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, GT_TRUE, GT_TRUE);

        /* AUTODOC: restore configuration */
        routeEntry->nextHopVlanId = prvTgfNextHopeVlanid;

        /* AUTODOC: add UC route entry with nexthop VLAN 6 and nexthop port 3 */
        rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndex, routeEntry, 1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);

        /* AUTODOC: restore RPF check mode */
        rc = prvTgfIpUcRpfModeSet(prvTgfDevNum,
                                  PRV_TGF_SEND_VLANID_CNS,
                                  PRV_TGF_IP_URPF_DISABLE_MODE_E);
        break;

    case PRV_TGF_EPORT_BASED_URPF_MODE_E:

        PRV_UTF_LOG0_MAC("==== Setting ePort Based uRPF Configuration ====\n");

        rc = prvTgfIpUcRpfModeSet(prvTgfDevNum,
                                  PRV_TGF_SEND_VLANID_CNS,
                                  PRV_TGF_IP_URPF_PORT_TRUNK_MODE_E);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRpfModeSet: %d", prvTgfDevNum);

        /* AUTODOC: expect traffic only on port 0 */
        prvTgfBasicIpv4UcRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, GT_TRUE, GT_TRUE);

        /* AUTODOC: compare Packet ePort & NextHop Entry ePort If not equal, assign IPv4 uRPF fail packet command */
        rc = prvTgfIpExceptionCommandSet(prvTgfDevNum, PRV_TGF_IP_EXCP_RPF_FAIL_E, CPSS_IP_PROTOCOL_IPV4_E, CPSS_IP_UNICAST_E, PRV_TGF_IP_EXC_CMD_DROP_HARD_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpExceptionCommandSet: %d", prvTgfDevNum);

        /* AUTODOC: read the UC Route entry from the Route Table and change vlan */
        cpssOsMemSet(routeEntry, 0, sizeof(routeEntry));
        rc = prvTgfIpUcRouteEntriesRead(prvTgfDevNum, prvTgfRouteEntryBaseIndex, routeEntry, 1);

        /* set next hop port 1 */
        origNextHopPort = routeEntry->nextHopInterface.devPort.portNum;
        routeEntry->nextHopInterface.devPort.portNum = prvTgfPortsArray[0];

        /* AUTODOC: add UC route entry with nex hop port 1 */
        rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndex, routeEntry, 1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);

        /* AUTODOC: expect no traffic */
        prvTgfBasicIpv4UcRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, GT_TRUE, GT_TRUE);

        /* AUTODOC: restore configuration */
        rc = prvTgfIpUcRpfModeSet(prvTgfDevNum,
                                  PRV_TGF_SEND_VLANID_CNS,
                                  PRV_TGF_IP_URPF_DISABLE_MODE_E);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRpfModeSet: %d", prvTgfDevNum);

        /* restore nexthop port */
        routeEntry->nextHopInterface.devPort.portNum = origNextHopPort;
        rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndex, routeEntry, 1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);

        break;

    case PRV_TGF_LOOSE_URPF_MODE_E:

        PRV_UTF_LOG0_MAC("==== Setting Loose uRPF Configuration ====\n");

        rc = prvTgfIpUcRpfModeSet(prvTgfDevNum,
                                  PRV_TGF_SEND_VLANID_CNS,
                                  PRV_TGF_IP_URPF_LOOSE_MODE_E);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRpfModeSet: %d", prvTgfDevNum);

        /* AUTODOC: expect traffic */
        prvTgfBasicIpv4UcRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, GT_TRUE, GT_FALSE);

        /* AUTODOC: read the UC Route entry from the Route Table and change packet command */
        cpssOsMemSet(routeEntry, 0, sizeof(routeEntry));
        rc = prvTgfIpUcRouteEntriesRead(prvTgfDevNum, prvTgfRouteEntryBaseIndex, routeEntry, 1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesRead: %d", prvTgfDevNum);

        routeEntry->cmd = CPSS_PACKET_CMD_DROP_HARD_E;

        /* AUTODOC: add UC route entry with command Drop command */
        rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndex, routeEntry, 1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);

        /* AUTODOC: checks if Next-Hop Route table entry for this address has the field <UC Packet Command> is set
                    to either ROUTE or ROUTE_AND_MIRROR. If it is not, the packet is assigned the
                    global IPv4 Unicast RPF Fail Command */
        rc = prvTgfIpExceptionCommandSet(prvTgfDevNum, PRV_TGF_IP_EXCP_RPF_FAIL_E, CPSS_IP_PROTOCOL_IPV4_E, CPSS_IP_UNICAST_E, PRV_TGF_IP_EXC_CMD_DROP_HARD_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpExceptionCommandSet: %d", prvTgfDevNum);

        /* AUTODOC: expect no traffic */
        prvTgfBasicIpv4UcRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, GT_TRUE, GT_TRUE);

        /* AUTODOC: restore configuration */
        rc = prvTgfIpUcRpfModeSet(prvTgfDevNum,
                                  PRV_TGF_SEND_VLANID_CNS,
                                  PRV_TGF_IP_URPF_DISABLE_MODE_E);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRpfModeSet: %d", prvTgfDevNum);

        routeEntry->cmd = CPSS_PACKET_CMD_ROUTE_E;

        /* AUTODOC: add UC route entry with nexthop VLAN 6 and nexthop port 3 */
        rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndex, routeEntry, 1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);

        break;

    }

    /* AUTODOC: restore the exception command */
    rc = prvTgfIpExceptionCommandSet(prvTgfDevNum, PRV_TGF_IP_EXCP_RPF_FAIL_E, CPSS_IP_PROTOCOL_IPV4_E, CPSS_IP_UNICAST_E, exceptionCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpExceptionCommandSet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfBasicIpv4UcRoutingUnicastRpfLooseModeTypeSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
* @param[in] prvUntUrpfLooseModeType  - unicast RPF Loose mode type
*                                       None
*/
GT_VOID prvTgfBasicIpv4UcRoutingUnicastRpfLooseModeTypeSet
(
    GT_U32                            prvUtfVrfId,
    PRV_TGF_URPF_LOOSE_MODE_TYPE_ENT  prvUntUrpfLooseModeType
)
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_IPADDR                               ipAddr;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           routeEntry[1];
    PRV_TGF_IP_EXC_CMD_ENT                  exceptionCmd;
    PRV_TGF_IP_COUNTER_SET_STC              ipCounters;
    GT_U32                                  expectedInUcTrappedMirrorPkts = 0;

    PRV_UTF_LOG0_MAC("==== Setting uRPF Configuration ====\n");

    /* -------------------------------------------------------------------------
     * 0. Update existing Ipv4 prefix in a Virtual Router for the specified LPM DB
     */

    /* fill a nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex;
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;
    nextHopInfo.ipLttEntry.ucRPFCheckEnable = GT_FALSE;

    /* fill a source IP address for the prefix */
    cpssOsMemCpy(ipAddr.arIP, prvTgfPacketIpv4Part.srcAddr, sizeof(ipAddr.arIP));

    /* AUTODOC: Add IPv4 UC prefix 1.1.1.1/32 */
    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, prvUtfVrfId,
                                    ipAddr, 32, &nextHopInfo, GT_TRUE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);

    /* fill a target IP address for the prefix */
    cpssOsMemCpy(ipAddr.arIP, prvTgfPacketIpv4Part.dstAddr, sizeof(ipAddr.arIP));

    /* AUTODOC: Update IPv4 UC prefix 1.1.1.3/32 */
    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, prvUtfVrfId,
                                    ipAddr, 32, &nextHopInfo, GT_TRUE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);

    rc = prvTgfIpExceptionCommandGet(prvTgfDevNum, PRV_TGF_IP_EXCP_RPF_FAIL_E, CPSS_IP_PROTOCOL_IPV4_E, CPSS_IP_UNICAST_E, &exceptionCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpExceptionCommandGet: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("==== Setting Loose uRPF Configuration ====\n");

    rc = prvTgfIpUcRpfModeSet(prvTgfDevNum,
                                PRV_TGF_SEND_VLANID_CNS,
                                PRV_TGF_IP_URPF_LOOSE_MODE_E);

    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRpfModeSet: %d", prvTgfDevNum);

    rc = prvTgfIpUrpfLooseModeTypeSet(prvTgfDevNum,
                                        prvUntUrpfLooseModeType);

    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUrpfLooseModeTypeSet: %d", prvTgfDevNum);

    /* AUTODOC: expect traffic */
    prvTgfBasicIpv4UcRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, GT_TRUE, GT_FALSE);

    /* AUTODOC: read the UC Route entry from the Route Table */
    cpssOsMemSet(routeEntry, 0, sizeof(routeEntry));
    rc = prvTgfIpUcRouteEntriesRead(prvTgfDevNum, prvTgfRouteEntryBaseIndex, routeEntry, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesRead: %d", prvTgfDevNum);

    routeEntry->cmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;

    /* AUTODOC: add UC route entry with command Trap to CPU */
    rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndex, routeEntry, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);

    /* AUTODOC: checks if Next-Hop Route table entry for this address has the field <UC Packet Command> is set
                to either ROUTE or ROUTE_AND_MIRROR. If it is not, the packet is assigned the
                global IPv4 Unicast RPF Fail Command */
    rc = prvTgfIpExceptionCommandSet(prvTgfDevNum, PRV_TGF_IP_EXCP_RPF_FAIL_E, CPSS_IP_PROTOCOL_IPV4_E, CPSS_IP_UNICAST_E, PRV_TGF_IP_EXC_CMD_DROP_HARD_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpExceptionCommandSet: %d", prvTgfDevNum);

    /* AUTODOC: expect no traffic */
    prvTgfBasicIpv4UcRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, GT_TRUE, GT_TRUE);

    /* AUTODOC: print and check ip counters values */
    PRV_UTF_LOG1_MAC("IP counters for Port [%d]:\n", prvTgfPortsArray[0]);
    prvTgfCountersIpGet(prvTgfDevNum, prvTgfPortsArray[0], GT_TRUE, &ipCounters);

    PRV_UTF_LOG0_MAC("\n");

    if (prvUntUrpfLooseModeType == PRV_TGF_URPF_LOOSE_MODE_TYPE_1_E)
    {
        expectedInUcTrappedMirrorPkts = 1;
    }
    /* check ip counters */
    UTF_VERIFY_EQUAL0_STRING_MAC(expectedInUcTrappedMirrorPkts, ipCounters.inUcTrappedMirrorPkts, "got wrong number of inUcTrappedMirrorPkts");

    /* AUTODOC: restore configuration */
    rc = prvTgfIpUcRpfModeSet(prvTgfDevNum,
                                PRV_TGF_SEND_VLANID_CNS,
                                PRV_TGF_IP_URPF_DISABLE_MODE_E);

    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRpfModeSet: %d", prvTgfDevNum);

    routeEntry->cmd = CPSS_PACKET_CMD_ROUTE_E;

    /* AUTODOC: restore UC route entry */
    rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndex, routeEntry, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);

    /* AUTODOC: restore the exception command */
    rc = prvTgfIpExceptionCommandSet(prvTgfDevNum, PRV_TGF_IP_EXCP_RPF_FAIL_E, CPSS_IP_PROTOCOL_IPV4_E, CPSS_IP_UNICAST_E, exceptionCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpExceptionCommandSet: %d", prvTgfDevNum);

}


/**
* @internal prvTgfBasicIpv4UcRoutingUnicastSipSaEnableSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
*                                       None
*/
GT_VOID prvTgfBasicIpv4UcRoutingUnicastSipSaEnableSet
(
    GT_U32                            prvUtfVrfId
)
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_IPADDR                               ipAddr;
    GT_ETHERADDR                            arpMacAddr;
    PRV_TGF_IP_EXC_CMD_ENT                  exceptionCmd;

    PRV_UTF_LOG0_MAC("==== Setting SIP SA Mismatch Configuration ====\n");

    /* -------------------------------------------------------------------------
     * 0. Update existing Ipv4 prefix in a Virtual Router for the specified LPM DB
     */

    /* fill a nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex;
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;
    nextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_TRUE;

    /* AUTODOC: update IPv4 UC prefix 1.1.1.1/32 */
    /* fill a source IP address for the prefix */
    cpssOsMemCpy(ipAddr.arIP, prvTgfPacketIpv4Part.srcAddr, sizeof(ipAddr.arIP));

    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, prvUtfVrfId,
                                    ipAddr, 32, &nextHopInfo, GT_TRUE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);

    /* AUTODOC: write a ARP MAC 00:00:00:00:00:01 to the Router ARP Table */
    cpssOsMemCpy(arpMacAddr.arEther, prvTgfPacketL2Part.saMac, sizeof(TGF_MAC_ADDR));

    rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterArpIndex, &arpMacAddr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    /* AUTODOC: expect traffic */
    prvTgfBasicIpv4UcRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], PRV_TGF_DEFAULT_BOOLEAN_CNS, GT_TRUE, PRV_TGF_DEFAULT_BOOLEAN_CNS, GT_FALSE);

    /* AUTODOC: write a ARP MAC 00:00:00:00:00:22 to the Router ARP Table */
    cpssOsMemCpy(arpMacAddr.arEther, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

    rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterArpIndex, &arpMacAddr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    rc = prvTgfIpExceptionCommandGet(prvTgfDevNum, PRV_TGF_IP_EXCP_SIP_SA_FAIL_E, CPSS_IP_PROTOCOL_IPV4_E, CPSS_IP_UNICAST_E, &exceptionCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpExceptionCommandGet: %d", prvTgfDevNum);

    /* AUTODOC: Compare Packet MAC SA and ARP Table MAC. If not equal, Assign IPv4 SIP/SA Check fail packet command */
    rc = prvTgfIpExceptionCommandSet(prvTgfDevNum, PRV_TGF_IP_EXCP_SIP_SA_FAIL_E, CPSS_IP_PROTOCOL_IPV4_E, CPSS_IP_UNICAST_E, PRV_TGF_IP_EXC_CMD_DROP_HARD_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpExceptionCommandSet: %d", prvTgfDevNum);

    /* AUTODOC: expect no traffic */
    prvTgfBasicIpv4UcRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], PRV_TGF_DEFAULT_BOOLEAN_CNS, GT_FALSE, PRV_TGF_DEFAULT_BOOLEAN_CNS, GT_TRUE);

    /* restore */

    /* AUTODOC: delete the Ipv4 prefix */
    rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, prvUtfVrfId, ipAddr, 32);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);

    /* AUTODOC: restore the exception command */
    rc = prvTgfIpExceptionCommandSet(prvTgfDevNum, PRV_TGF_IP_EXCP_SIP_SA_FAIL_E, CPSS_IP_PROTOCOL_IPV4_E, CPSS_IP_UNICAST_E, exceptionCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpExceptionCommandSet: %d", prvTgfDevNum);

}

/**
* @internal prvTgfBasicIpv4EPortToVidxRouteConfigurationSet function
* @endinternal
*
* @brief   Set basic ipv4 routing configurations for EPort that is :
*         eport : 1000 -->E2PHY : VIDX 100 : ports :according to portIdxBmp
* @param[in] portIdxBmp               - bmp of port indexes
*                                       None
*/
GT_VOID   prvTgfBasicIpv4EPortToVidxRouteConfigurationSet(
    IN GT_U32    portIdxBmp
)
{
    GT_STATUS   rc;
    CPSS_PORTS_BMP_STC      portBitmap;
    GT_U32  trgEPort = PRV_TGF_DESTINATION_EPORT_CNS;
    CPSS_INTERFACE_INFO_STC physicalInfo;/* physical port info for the analyzer in eArch */
    GT_U32  ii;

    useEPortMappedToVidx = GT_TRUE;

    physicalInfo.type = CPSS_INTERFACE_VIDX_E;
    physicalInfo.vidx = PRV_TGF_VIDX_FROM_EPORT_CNS;

    /* save ePort mapping configuration */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableGet(prvTgfDevNum,
                                                           trgEPort,
                                                           &(prvTgfRestoreCfg.physicalInfo));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortTargetMappingTableGet);

    /* AUTODOC: map Routed EPort to VIDX  */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                trgEPort,
                &physicalInfo);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableSet: %d, %d",
                                 prvTgfDevNum, trgEPort);

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portBitmap);

    /* add the needed ports to the VIDX */
    for(ii = 0 ; ii < 32 ; ii++)
    {
        if(portIdxBmp & (1<<ii))
        {
            CPSS_PORTS_BMP_PORT_SET_MAC(&portBitmap,prvTgfPortsArray[ii]);
        }
    }

    /* AUTODOC: create the VIDX  */
    rc = prvTgfBrgMcEntryWrite(prvTgfDevNum, PRV_TGF_VIDX_FROM_EPORT_CNS, &portBitmap);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgMcEntryWrite: %d",
                                 prvTgfDevNum);


}
/**
* @internal prvTgfBasicIpAddToDefaultSendVlanAndEnableRouting function
* @endinternal
*
* @brief   Add port to default send vlan and enable routing on this port
*
* @param[in] portIdx                  - port index
*                                       None
*/

GT_VOID prvTgfBasicIpAddToDefaultSendVlanAndEnableRouting
(
    IN GT_U8    portIdx,
    IN CPSS_IP_PROTOCOL_STACK_ENT     protocol
)

{
    GT_STATUS rc;

    rc =  prvTgfBrgVlanMemberAdd(prvTgfDevNum,PRV_TGF_SEND_VLANID_CNS,prvTgfPortsArray[portIdx],1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberAdd");

    if(portIdx>=prvTgfPortsNum)
    {
       prvTgfPortsNum = portIdx+1;
    }

    rc = prvTgfIpPortRoutingEnable(portIdx, CPSS_IP_UNICAST_E,
                                       protocol, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIdx]);

}
/**
* @internal prvTgfBasicIpPickPortFromPipe function
* @endinternal
*
* @brief   Picks a port according to required pipe number
*
* @param[in] requiredPipeNum          - pipe number
* @param[in] skipIndex                - we do not want to pick destination port ,so provide index that should not be chosed
*
* @param[out] portIdxPtr               - port index
*
* @retval GT_OK                    - Port picked ok
* @retval GT_FAIL                  - Failure to pick a port
*/

GT_STATUS prvTgfBasicIpPickPortFromPipe
(
   IN GT_U8    requiredPipeNum,
   IN GT_U8    skipIndex,
   OUT GT_U8  * portIdxPtr
)
{
    GT_U8 i;
    GT_U32 globalMacPortNum;
    GT_U32 portPipeId;
    GT_STATUS rc;

    if(requiredPipeNum>1)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfBasicIpPickPortFromPipe Illegal pipe %d \n",requiredPipeNum);
        return GT_FAIL;
    }


    for(i=0;i<PRV_TGF_MAX_PORTS_NUM_CNS;i++)
    {

        PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(prvTgfDevNum,prvTgfPortsArray[i], globalMacPortNum);
        rc = prvCpssDxChHwPpGopGlobalMacPortNumToLocalMacPortInPipeConvert(prvTgfDevNum, globalMacPortNum,&portPipeId , NULL);

        if(rc!=GT_OK)
        {
            PRV_UTF_LOG2_MAC("[TGF]: prvTgfBasicIpPickPortFromPipe Error for i= %d ,port %d \n",i, prvTgfPortsArray[i]);

            return rc;
        }

        if((portPipeId ==requiredPipeNum)&&(i!=skipIndex))
        {

             PRV_UTF_LOG3_MAC("[TGF]: prvTgfBasicIpPickPortFromPipe Found port %d [index %d] that belong to pipe %d \n", prvTgfPortsArray[i],i,requiredPipeNum);

             break;
        }
    }

    *portIdxPtr = i;

    if(i == PRV_TGF_MAX_PORTS_NUM_CNS)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfBasicIpPickPortFromPipe No port found for pipe %d \n",requiredPipeNum);
        rc = GT_FAIL;
    }

    return rc;


}


