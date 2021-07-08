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
* @file cstTunnelStartMplsVpws.c
*
* @brief Basic VPWS test.
*
* @version   6
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>
#include <common/customer/cstCommon.h>
#include <common/customer/cstBridge.h>
#include <common/tgfIpGen.h>
#include <common/tgfBridgeGen.h>
#include <tunnel/customer/cstTunnelStartMplsVpws.h>
#include <common/tgfPacketGen.h>
#include <gtOs/gtOsMem.h>

#ifdef CHX_FAMILY
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgGen.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdb.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgE2Phy.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

static GT_HW_DEV_NUM hwDevNum = 0x10;

static GT_U32   errorLine = 0;

/* used ports */
static  GT_U32    portsArray[] = {0, 18, 36, 58};

/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS           200

#define PRV_TGF_DEFAULT_VLANID_CNS        1

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS            4

/* default number of packets to send */
#define PRV_TGF_BURST_COUNT_DEFAULT_CNS     1

/* default number of packets to send */
static GT_U32        prvTgfBurstCount   = PRV_TGF_BURST_COUNT_DEFAULT_CNS;

/* value of MacDa */
static TGF_MAC_ADDR prvTgfTsMacDaArr={0x00, 0x00, 0xAA, 0xBB, 0x77, 0xEE};

/* value of MacSa */
static TGF_MAC_ADDR prvTgfGlobalMacSaArr={0x00, 0x00, 0x00, 0x00, 0x00, 0x09};

/* destination ePort */
#define PRV_TGF_DESTINATION_EPORT_CNS   (UTF_CPSS_PP_MAX_PORT_NUM_CNS(prvTgfDevNum)  - 5)


#define PRV_UTF_LOG_VERIFY_GT_OK(rc, name)                                   \
    if (GT_OK != (rc))                                                       \
    {                                                                        \
        PRV_UTF_LOG2_MAC("\n[TGF]: %s FAILED, rc = [%d]\n", name, rc);       \
    }

/* port number to send traffic to in default VLAN */
#define PRV_TGF_SEND_PORT_IDX_CNS         0

/* port number used for FDB configuration */
#define PRV_TGF_FDB_PORT_IDX_CNS          1

/* port number not used as send or receive ports */
#define PRV_TGF_FREE_PORT_IDX_CNS         2

/* port number to receive traffic from */
#define PRV_TGF_RECEIVE_PORT_IDX_CNS      3

/* mac SA index in global mac sa table */
#define PRV_TGF_MAC_SA_GLOBAL_INDEX_CNS          7

/* line index for the tunnel start entry in the router ARP / tunnel start table (0..1023) */
static GT_U32   routerArpTunnelStartLineIndex = 20;

#define _18K    (18 * _1K)
/* those indexes must be divided by 3 without leftovers */
static GT_U32   tunnelTerminationIndex[4] = {_18K +0x111*3,_18K +0x222*3,_18K + 0x444*3/*for eport C*/,_18K + (0x555*3)/*for ePort D*/};

static GT_U32   tunnelTerminationIndexBobK[4] = {_9K +0x111*3,_9K +0x222*3,_9K + 0x444*3/*for eport C*/,_9K + (0x555*3)/*for ePort D*/};

/* aldrin and all other devices with only 3 TCAM floors use floor#1 first 6 TCAM arrays for TTI_0 lookup
   Need to use absolute indexes of floor#1 _3K + m + 12*n, where m 0..5 - array index, n - row index 0..255  */
#define ALDRIN_TTI_0_INDEX_MAC(_relativeIdx)  (_3K + ((3*_relativeIdx) % 6) + ((3*_relativeIdx)/6)*12)

static GT_U32   tunnelTerminationIndexAldrin[4] = {ALDRIN_TTI_0_INDEX_MAC(11),ALDRIN_TTI_0_INDEX_MAC(22),ALDRIN_TTI_0_INDEX_MAC(44)/*for eport C*/,ALDRIN_TTI_0_INDEX_MAC(55)/*for ePort D*/};

GT_U32   * tunnelTerminationIndexPtr;
/* set in runtime */
static GT_U32   generic_tunnelTerminationIndex[4];


/* TPID table entry index */
static GT_U32   tpidEntryIndex = 5;

/* default values for restore */
static GT_BOOL                              defaultMacSaModifyEnableGet;
static CPSS_MAC_SA_LSB_MODE_ENT             defaultMacSaLsbModeGet;
static GT_ETHERADDR                         defaultMacSaAddrGet;
static GT_U32                               defaultRouterMacSaIndex;
static CPSS_INTERFACE_INFO_STC              defaultEportToPhysicalPortTargetMappingGet;
static CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC   defaultEportToPhysicalPortEgressPortInfoGet;
static CPSS_TUNNEL_TYPE_ENT                 defaultTunnelTypeGet;
static CPSS_DXCH_TUNNEL_START_CONFIG_UNT    defaultTsConfigGet;
static GT_U16                               defaultEtherTypeGet;
static GT_U32                               defaultTpidEntryIndexGet;
static GT_BOOL                              defaultTtiPortLookupEnableGet;
static CPSS_DXCH_TTI_MAC_MODE_ENT           defaultTtiMacModeGet;


/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}                /* saMac */
};

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2PartGet = {
    {0x00, 0x00, 0xAA, 0xBB, 0x77, 0xEE},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x09}                /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_DEFAULT_VLANID_CNS                   /* pri, cfi, VlanId */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPartGet = {
    0x9200,                       /* etherType */
    6, 0, 0x1E6                   /* pri, cfi, VlanId */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,
    0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,
    0x56,0x56,0x56,0x56,0x56,0x56,0x56,0x56,0x56,0x56,0x56,0x56,0x56,0x56,0x56,0x56,
    0x78,0x78,0x78,0x78,0x78,0x78,0x78,0x78,0x78,0x78,0x78,0x78,0x78,0x78,0x78,0x78,
    0x9a,0x9a,0x9a,0x9a,0x9a,0x9a,0x9a,0x9a,0x9a,0x9a,0x9a,0x9a,0x9a,0x9a,0x9a,0x9a,
    0xbc,0xbc,0xbc,0xbc,0xbc,0xbc,0xbc,0xbc,0xbc,0xbc,0xbc,0xbc,0xbc,0xbc,0xbc,0xbc
};

static TGF_PACKET_ETHERTYPE_STC prvTgfPacketMplsEtherTypePart = {TGF_ETHERTYPE_8847_MPLS_TAG_CNS};
static TGF_PACKET_MPLS_STC prvTgfPacketMplsLabelPart =
{
    3,/*filled in runtime*/ /* label */
    2,                  /* exp */
    1,                  /* stack */
    0x2f                /* timeToLive */
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet -  tagged */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* PARTS of packet -  tagged GET*/
static TGF_PACKET_PART_STC prvTgfPacketPartGetArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2PartGet},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPartGet},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketMplsEtherTypePart},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMplsLabelPart},
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* PARTS of packet - non tagged */
static TGF_PACKET_PART_STC prvTgfPacke2PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* PARTS of packet - non tagged GET */
static TGF_PACKET_PART_STC prvTgfPacke2PartGetArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2PartGet},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPartGet},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketMplsEtherTypePart},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMplsLabelPart},
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal cstTsMplsVpwsBrgConfigSet function
* @endinternal
*
* @brief   Set Base Configuration
*
* @param[in] devNum                   - device number
* @param[in] fdbPortNum               - the FDB port num to set in the macEntry destination Interface
*                                       None
*/
static GT_STATUS cstTsMplsVpwsBrgConfigSet
(
    GT_U8   devNum,
    GT_U32   fdbPortNum
)
{

    GT_STATUS                   rc = GT_OK;
    GT_U8                       tagArray[] = {1, 1, 1, 0};
    CPSS_MAC_ENTRY_EXT_STC      macEntry;
    GT_ETHERADDR                macSaAddr;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN 200 with tagged ports 0,1,2 and untagged 3 */
    rc = cstBrgDefVlanEntryWithPortsSet(devNum,PRV_TGF_SEND_VLANID_CNS,portsArray, NULL, tagArray, 4);
    CHECK_RC_MAC(rc);

    /* AUTODOC: save default MAC SA modification on port egrPort */
    rc = cpssDxChIpRouterMacSaModifyEnableGet(devNum,prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],&defaultMacSaModifyEnableGet);
    CHECK_RC_MAC(rc);

    /* AUTODOC: enable MAC SA modification on port egrPort */
    rc = cpssDxChIpRouterMacSaModifyEnable(devNum,prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],GT_TRUE);
    CHECK_RC_MAC(rc);

    /* AUTODOC: save default  MAC SA LSB mode for port  */
    rc = cpssDxChIpPortRouterMacSaLsbModeGet(devNum,prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],&defaultMacSaLsbModeGet);
    CHECK_RC_MAC(rc);

    /* AUTODOC: set MAC SA LSB mode for port  */
    rc = cpssDxChIpPortRouterMacSaLsbModeSet(devNum,prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],CPSS_SA_LSB_FULL_48_BIT_GLOBAL);
    CHECK_RC_MAC(rc);

    /* AUTODOC: save default MAC SA configured to global MAC SA table into index 7 */
    rc = cpssDxChIpRouterGlobalMacSaGet(devNum, PRV_TGF_MAC_SA_GLOBAL_INDEX_CNS, &defaultMacSaAddrGet);
    CHECK_RC_MAC(rc);

    /* AUTODOC: configure MAC SA (00:00:00:00:00:09) to global MAC SA table into index 7 */
    cpssOsMemCpy(macSaAddr.arEther, prvTgfGlobalMacSaArr, sizeof(TGF_MAC_ADDR));
    rc = cpssDxChIpRouterGlobalMacSaSet(devNum, PRV_TGF_MAC_SA_GLOBAL_INDEX_CNS, &macSaAddr);
    CHECK_RC_MAC(rc);

    /* AUTODOC: save default index to global MAC SA table in eport attribute table */
    rc =  prvTgfIpRouterPortGlobalMacSaIndexGet(devNum,PRV_TGF_DESTINATION_EPORT_CNS,&defaultRouterMacSaIndex);
    CHECK_RC_MAC(rc);

    /* AUTODOC: configure index to global MAC SA table in eport attribute table */
    rc =  prvTgfIpRouterPortGlobalMacSaIndexSet(devNum,PRV_TGF_DESTINATION_EPORT_CNS,PRV_TGF_MAC_SA_GLOBAL_INDEX_CNS);
    CHECK_RC_MAC(rc);

    /* -------------------------------------------------------------------------
     * Set the FDB entry With DA_ROUTE
     */

    /* AUTODOC: get hw dev num */
    rc = cpssDxChCfgHwDevNumGet(devNum,&hwDevNum);
    CHECK_RC_MAC(rc);

    /* create a macEntry with .daRoute = GT_FALSE */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    macEntry.key.entryType                  = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId         = PRV_TGF_SEND_VLANID_CNS;
    macEntry.dstInterface.type              = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum  = hwDevNum;
    macEntry.dstInterface.devPort.portNum   = fdbPortNum;
    macEntry.isStatic                       = GT_TRUE;
    macEntry.daCommand                      = CPSS_MAC_TABLE_FRWRD_E;
    macEntry.saCommand                      = CPSS_MAC_TABLE_FRWRD_E;
    macEntry.daRoute                        = GT_FALSE;
    macEntry.sourceID                       = 0;
    macEntry.userDefined                    = 0;
    macEntry.daQosIndex                     = 0;
    macEntry.saQosIndex                     = 0;
    macEntry.daSecurityLevel                = 0;
    macEntry.saSecurityLevel                = 0;
    macEntry.appSpecificCpuCode             = GT_FALSE;
    macEntry.mirrorToRxAnalyzerPortEn       = GT_FALSE;

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 200, port 1 */
    rc = cpssDxChBrgFdbMacEntrySet(devNum,&macEntry);
    CHECK_RC_MAC(rc);
    return GT_OK;
}

/**
* @internal cstTsMplsVpwsEportConfigSet function
* @endinternal
*
* @brief   Set Base Eport Configuration
*
* @param[in] devNum                   - device number
*                                       None
*/
static GT_STATUS cstTsMplsVpwsEportConfigSet
(
    GT_U8   devNum
)
{
    GT_STATUS                               rc = GT_OK;
    CPSS_INTERFACE_INFO_STC                 physicalInfo;
    CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC      egressInfo;

    /* AUTODOC: EPORT CONFIGURATION: */

    /* AUTODOC: Set physical info for the given ePort in the E2PHY mapping table */
    cpssOsMemSet(&physicalInfo,0,sizeof(physicalInfo));
    physicalInfo.type=CPSS_INTERFACE_PORT_E;
    physicalInfo.devPort.hwDevNum = hwDevNum;
    physicalInfo.devPort.portNum = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];

    /* AUTODOC: save default physical info for the given ePort in the E2PHY mapping table */
    rc = cpssDxChBrgEportToPhysicalPortTargetMappingTableGet(devNum,PRV_TGF_DESTINATION_EPORT_CNS,&defaultEportToPhysicalPortTargetMappingGet);
    CHECK_RC_MAC(rc);

    rc = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum,PRV_TGF_DESTINATION_EPORT_CNS,&physicalInfo);
    CHECK_RC_MAC(rc);

    /* AUTODOC: Set physical info for the given ePort in the HA Egress port table. */
    cpssOsMemSet(&egressInfo,0,sizeof(egressInfo));
    egressInfo.tunnelStart=GT_TRUE;
    egressInfo.tunnelStartPtr=routerArpTunnelStartLineIndex;
    egressInfo.tsPassengerPacketType=CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E;
    egressInfo.arpPtr=0;
    egressInfo.modifyMacSa=GT_FALSE;
    egressInfo.modifyMacDa=GT_FALSE;

    /* AUTODOC: save default  physical info for the given ePort in the HA Egress port table. */
    rc = cpssDxChBrgEportToPhysicalPortEgressPortInfoGet(devNum,PRV_TGF_DESTINATION_EPORT_CNS,&defaultEportToPhysicalPortEgressPortInfoGet);
    CHECK_RC_MAC(rc);

    rc = cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(devNum,PRV_TGF_DESTINATION_EPORT_CNS,&egressInfo);
    CHECK_RC_MAC(rc);

    return GT_OK;
}

/**
* @internal cstTsMplsVpwsTsConfigSet function
* @endinternal
*
* @brief   Set Base TS Configuration
*
* @param[in] devNum                   - device number
*                                       None
*/
static GT_STATUS cstTsMplsVpwsTsConfigSet
(
    GT_U8   devNum
)
{
    GT_STATUS rc = GT_OK;

    CPSS_DXCH_TUNNEL_START_CONFIG_UNT       tsEntry;
    CPSS_DXCH_TUNNEL_START_MPLS_CONFIG_STC  *mplsTsEntryPtr;

    cpssOsMemSet(&tsEntry,0,sizeof(tsEntry));
    mplsTsEntryPtr = &tsEntry.mplsCfg;

    /* AUTODOC: define the TS entry */
    mplsTsEntryPtr->tagEnable = GT_TRUE;
    mplsTsEntryPtr->vlanId = 486;/* 0x1E6 */
    mplsTsEntryPtr->upMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
    mplsTsEntryPtr->up = 6;
    cpssOsMemCpy(&(mplsTsEntryPtr->macDa), &prvTgfTsMacDaArr, sizeof(TGF_MAC_ADDR));/* 00 00 AA BB 77 EE */
    mplsTsEntryPtr->numLabels = 1;
    mplsTsEntryPtr->ttl = 47;
    mplsTsEntryPtr->ttlMode = CPSS_DXCH_TUNNEL_START_TTL_TO_INCOMING_TTL_E;
    mplsTsEntryPtr->label1 = 3;
    mplsTsEntryPtr->exp1MarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
    mplsTsEntryPtr->exp1 = 2;
    mplsTsEntryPtr->label2 = 0;
    mplsTsEntryPtr->exp2MarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
    mplsTsEntryPtr->exp2 = 0;
    mplsTsEntryPtr->label3 = 0;
    mplsTsEntryPtr->exp3MarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
    mplsTsEntryPtr->exp3 = 0;
    mplsTsEntryPtr->retainCRC = GT_FALSE;
    mplsTsEntryPtr->setSBit = GT_TRUE;
    mplsTsEntryPtr->cfi = 0;
    mplsTsEntryPtr->controlWordEnable = GT_FALSE;
    mplsTsEntryPtr->controlWordIndex = 0;
    mplsTsEntryPtr->mplsEthertypeSelect = CPSS_DXCH_TUNNEL_START_MPLS_ETHER_TYPE_UC_E;

    /* AUTODOC: save the tunnel start on index routerArpTunnelStartLineIndex */
    rc = cpssDxChTunnelStartEntryGet(devNum,routerArpTunnelStartLineIndex,&defaultTunnelTypeGet, &defaultTsConfigGet);
    CHECK_RC_MAC(rc);

    /* AUTODOC: add the tunnel start */
    rc = cpssDxChTunnelStartEntrySet(devNum,routerArpTunnelStartLineIndex,CPSS_TUNNEL_X_OVER_MPLS_E, &tsEntry);
    CHECK_RC_MAC(rc);

   /* AUTODOC: Configure TPID for Tunnel Start header */

    /* AUTODOC: save TPID (Tag Protocol ID) table entry  */
    rc = cpssDxChBrgVlanTpidEntryGet(devNum,CPSS_DIRECTION_EGRESS_E,tpidEntryIndex,&defaultEtherTypeGet);
    CHECK_RC_MAC(rc);

    /* AUTODOC: Sets TPID (Tag Protocol ID) table entry  */
    rc = cpssDxChBrgVlanTpidEntrySet(devNum,CPSS_DIRECTION_EGRESS_E,tpidEntryIndex,0x9200);
    CHECK_RC_MAC(rc);

    /* AUTODOC: save index of TPID tunnel-start header */
    rc = cpssDxChTunnelStartHeaderTpidSelectGet(devNum, PRV_TGF_DESTINATION_EPORT_CNS, &defaultTpidEntryIndexGet);
    CHECK_RC_MAC(rc);

    /* AUTODOC: Set index of TPID tunnel-start header */
    rc = cpssDxChTunnelStartHeaderTpidSelectSet(devNum, PRV_TGF_DESTINATION_EPORT_CNS, tpidEntryIndex);
    CHECK_RC_MAC(rc);

    return GT_OK;
}

/**
* @internal cstTsMplsVpwsTtiConfigSet function
* @endinternal
*
* @brief   Set Base TTI Configuration
*
* @param[in] devNum                   - device number
*                                       None
*/
static GT_STATUS cstTsMplsVpwsTtiConfigSet
(
    GT_U8   devNum
)
{
    GT_STATUS rc = GT_OK;
    CPSS_DXCH_TTI_RULE_UNT          ttiPattern;
    CPSS_DXCH_TTI_ETH_RULE_STC      *ttiEthPatternPtr;
    CPSS_DXCH_TTI_RULE_UNT          ttiMask;
    CPSS_DXCH_TTI_ETH_RULE_STC      *ttiEthMaskPtr;
    CPSS_DXCH_TTI_ACTION_UNT        ttiAction;
    CPSS_DXCH_TTI_ACTION_2_STC      *ttiAction2Ptr;
    GT_U32      ii;

    /* AUTODOC: set TTI entry for tunnel start */

    /* AUTODOC: save TTI lookup on port 0 */
    rc = cpssDxChTtiPortLookupEnableGet(devNum,portsArray[PRV_TGF_SEND_PORT_IDX_CNS],CPSS_DXCH_TTI_KEY_ETH_E,&defaultTtiPortLookupEnableGet);
    CHECK_RC_MAC(rc);

   /* AUTODOC: enable TTI lookup on port 0 */
    rc = cpssDxChTtiPortLookupEnableSet(devNum,portsArray[PRV_TGF_SEND_PORT_IDX_CNS],CPSS_DXCH_TTI_KEY_ETH_E,GT_TRUE);
    CHECK_RC_MAC(rc);

    /* AUTODOC: save the lookup Mac mode for the specified key type */
    rc = cpssDxChTtiMacModeGet(devNum,CPSS_DXCH_TTI_KEY_ETH_E,&defaultTtiMacModeGet);
    CHECK_RC_MAC(rc);

    /* AUTODOC: sets the lookup Mac mode for the specified key type */
    rc = cpssDxChTtiMacModeSet(devNum,CPSS_DXCH_TTI_KEY_ETH_E,CPSS_DXCH_TTI_MAC_MODE_DA_E);
    CHECK_RC_MAC(rc);

    /* AUTODOC: sets TTI Rule and Action for tagged packet */
    cpssOsMemSet(&ttiPattern,0,sizeof(ttiPattern));
    cpssOsMemSet(&ttiMask   ,0,sizeof(ttiMask));
    cpssOsMemSet(&ttiAction ,0,sizeof(ttiAction));

    /*action*/
    ttiAction2Ptr = &ttiAction.type2;

    ttiAction2Ptr->tunnelTerminate = GT_FALSE;
    ttiAction2Ptr->ttPassengerPacketType = CPSS_DXCH_TTI_PASSENGER_IPV4_E;
    ttiAction2Ptr->tsPassengerPacketType = CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E;
    ttiAction2Ptr->ttHeaderLength = 0;
    ttiAction2Ptr->continueToNextTtiLookup = GT_FALSE;
    ttiAction2Ptr->copyTtlExpFromTunnelHeader = GT_FALSE;
    ttiAction2Ptr->mplsCommand = CPSS_DXCH_TTI_MPLS_NOP_CMD_E;
    ttiAction2Ptr->command = CPSS_PACKET_CMD_FORWARD_E;
    ttiAction2Ptr->redirectCommand = CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E;
    ttiAction2Ptr->egressInterface.type = CPSS_INTERFACE_PORT_E;
    ttiAction2Ptr->egressInterface.devPort.hwDevNum = hwDevNum;
    ttiAction2Ptr->egressInterface.devPort.portNum = PRV_TGF_DESTINATION_EPORT_CNS;
    ttiAction2Ptr->tag0VlanCmd = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;
    ttiAction2Ptr->tag0VlanId = 0;
    ttiAction2Ptr->tag1VlanCmd = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;
    ttiAction2Ptr->tag1VlanId = 0;
    ttiAction2Ptr->tag0VlanPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    ttiAction2Ptr->bridgeBypass = GT_TRUE;
    ttiAction2Ptr->ingressPipeBypass = GT_FALSE;

    /* Rule: pattern+mask*/
    ttiEthPatternPtr = &ttiPattern.eth;
    ttiEthMaskPtr = &ttiMask.eth;

    /* pattern */
    ttiEthPatternPtr->common.srcIsTrunk=GT_FALSE;
    ttiEthPatternPtr->common.srcPortTrunk=portsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    ttiEthPatternPtr->common.isTagged=GT_TRUE;
    ttiEthPatternPtr->common.vid=PRV_TGF_DEFAULT_VLANID_CNS;

    /* mask */
    ttiEthMaskPtr->common.srcIsTrunk=BIT_1-1;/*1bit*/
    ttiEthMaskPtr->common.srcPortTrunk=BIT_8-1;/*8 bits*/
    ttiEthMaskPtr->common.isTagged=BIT_1-1;/*1bit*/
    ttiEthMaskPtr->common.vid=BIT_12-1;/*12bit*/

    if (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->fineTuning.tableSize.tunnelTerm <= _9K)
    {
        tunnelTerminationIndexPtr = &tunnelTerminationIndexAldrin[0];
    }
    else if (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->fineTuning.tableSize.tunnelTerm < _36K)
    {
        tunnelTerminationIndexPtr = &tunnelTerminationIndexBobK[0];
    }
    else
    {
        tunnelTerminationIndexPtr = &tunnelTerminationIndex[0];
    }

    for(ii = 0 ; ii < (sizeof(generic_tunnelTerminationIndex)/sizeof(generic_tunnelTerminationIndex[0])) ; ii++)
    {
        generic_tunnelTerminationIndex[ii] =
            prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(0,tunnelTerminationIndexPtr[ii]);
    }

    tunnelTerminationIndexPtr = &generic_tunnelTerminationIndex[0];

    rc = cpssDxChTtiRuleSet(devNum,
                            tunnelTerminationIndexPtr[0],
                            CPSS_DXCH_TTI_RULE_ETH_E,
                            &ttiPattern,
                            &ttiMask,
                            CPSS_DXCH_TTI_ACTION_TYPE2_ENT,
                            &ttiAction);
    CHECK_RC_MAC(rc);

    /* AUTODOC: sets TTI Rule and Action for NON tagged packet */

    /*action*/
    ttiAction2Ptr->tag0VlanCmd = CPSS_DXCH_TTI_VLAN_DO_NOT_MODIFY_E;

    /* Rule: pattern+mask*/

    /* pattern */
    ttiEthPatternPtr->common.isTagged=GT_FALSE;

    /* mask */
    ttiEthMaskPtr->common.vid=0;/* don't care */

    rc = cpssDxChTtiRuleSet(devNum,
                            tunnelTerminationIndexPtr[1],
                            CPSS_DXCH_TTI_RULE_ETH_E,
                            &ttiPattern,
                            &ttiMask,
                            CPSS_DXCH_TTI_ACTION_TYPE2_ENT,
                            &ttiAction);
    CHECK_RC_MAC(rc);

    return GT_OK;
}

/**
* @internal cstTunnelStartMplsVpwsTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic - One tagged packet and one untagged packet
*
* @param[in] isTaggedPacket           - GT_TRUE:send a tagged packet
*                                      GT_FALSE:send a NON tagged packet
*                                       None
*/
GT_VOID cstTunnelStartMplsVpwsTrafficGenerate
(
    GT_BOOL     isTaggedPacket
)
{
    GT_STATUS                       rc = GT_OK;
    GT_U32                          portsCount = prvTgfPortsNum;
    GT_U32                          portIter   = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;

    CPSS_INTERFACE_INFO_STC         portInterface;
    GT_U32                          partsCount  = 0;
    GT_U32                          packetSize  = 0;
    TGF_PACKET_PART_STC             *packetPartsPtr;
    GT_U32                          partsCountGet  = 0;
    GT_U32                          packetSizeGet  = 0;
    TGF_PACKET_PART_STC             *packetPartsGetPtr;
    TGF_PACKET_STC                  packetInfo;
    GT_U32                          numVfd = 0;/* number of VFDs in vfdArray */
    TGF_VFD_INFO_STC                vfdArray[2];
    GT_BOOL                         isOk;

    TGF_PACKET_STC                  packetInfo_egress;
    GT_U32                          actualCapturedNumOfPackets = 0;


    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* reset counters */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* setup portInterface for capturing */
    portInterface.type              = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];

    /* enable capture port on 3 */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                 prvTgfDevNum, TGF_CAPTURE_MODE_MIRRORING_E);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /*  Generating Traffic */
    if(isTaggedPacket==GT_TRUE)
    {
        partsCount = sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);
        packetPartsPtr = prvTgfPacketPartArray;

        partsCountGet = sizeof(prvTgfPacketPartGetArray) / sizeof(prvTgfPacketPartGetArray[0]);
        packetPartsGetPtr = prvTgfPacketPartGetArray;
    }
    else
    {
        partsCount = sizeof(prvTgfPacke2PartArray) / sizeof(prvTgfPacke2PartArray[0]);
        packetPartsPtr = prvTgfPacke2PartArray;

        partsCountGet = sizeof(prvTgfPacke2PartGetArray) / sizeof(prvTgfPacke2PartGetArray[0]);
        packetPartsGetPtr = prvTgfPacke2PartGetArray;
    }

    /* calculate packet size */
    rc = prvTgfPacketSizeGet(packetPartsPtr, partsCount, &packetSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

    /* calculate packet size */
    rc = prvTgfPacketSizeGet(packetPartsGetPtr, partsCountGet, &packetSizeGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");


    /* build ingress packet */
    packetInfo.totalLen   = packetSize;
    packetInfo.numOfParts = partsCount;
    packetInfo.partsArray = packetPartsPtr;

    /* build egress packet */
    packetInfo_egress.totalLen   = packetSizeGet;
    packetInfo_egress.numOfParts = partsCountGet;
    packetInfo_egress.partsArray = packetPartsGetPtr;


     /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, prvTgfBurstCount, numVfd, &vfdArray[0]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* set transmit timings */
    rc = prvTgfSetTxSetup2Eth(0, 50);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetup2Eth: %d", prvTgfDevNum);

    /* AUTODOC: send packet from port 0 with: */
    /* AUTODOC: DA=00:00:00:00:34:02, SA=00:00:00:00:00:01, VID=1 if the packet is tagged */

    /* send packet -- send from specific port */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    cpssOsTimerWkAfter(1000);


    /* disable capture on port 3 , before check the packets to the CPU */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                 prvTgfDevNum, TGF_CAPTURE_MODE_MIRRORING_E);

    /* AUTODOC: check counters - packet is captured on port 3 */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if(portIter ==PRV_TGF_SEND_PORT_IDX_CNS)
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
            if(portIter == PRV_TGF_RECEIVE_PORT_IDX_CNS)
            {
                    /* packetSize is not changed */
                    expectedCntrs.goodOctetsSent.l[0] = (packetSizeGet + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                    expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                    expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                    expectedCntrs.brdcPktsSent.l[0]   = 0;
                    expectedCntrs.mcPktsSent.l[0]     = 0;
                    expectedCntrs.goodOctetsRcv.l[0]  = (packetSizeGet + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                    expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                    expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
                    expectedCntrs.brdcPktsRcv.l[0]    = 0;
                    expectedCntrs.mcPktsRcv.l[0]      = 0;
            }
            else
            {
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

    /* reset counters */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* print captured packets and check TriggerCounters */
    rc = tgfTrafficGeneratorPortTxEthCaptureCompare(
            &portInterface,
            &packetInfo_egress,
            1,/* numOfExpectedPackets */
            0/*vfdNum*/,
            NULL /*vfdArray*/,
            NULL, /* bytesNum's skip list */
            0,    /* length of skip list */
            &actualCapturedNumOfPackets,
            NULL/*onFirstPacketNumTriggersBmpPtr*/);
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "tgfTrafficGeneratorPortTxEthTriggerCountersGet:"
                                 " port = %d, rc = 0x%02X\n", portInterface.devPort.portNum, rc);
}


/**
* @internal cstTunnelStartMplsVpwsConfigRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_STATUS cstTunnelStartMplsVpwsConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc        = GT_OK;
    GT_U8       devNum    = 0;

    /* AUTODOC: RESTORE CONFIGURATION: */

    PRV_UTF_LOG0_MAC("======= Restoring Configuration =======\n");

    prvTgfBurstCount = PRV_TGF_BURST_COUNT_DEFAULT_CNS;/* restore value to default */

    /* delete the FDB entry */
    rc = cstBrgFdbFlush(GT_TRUE);
    CHECK_RC_MAC(rc);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    CHECK_RC_MAC(rc);

    /* -------------------------------------------------------------------------
     * 1. Restore Base Configuration
     */


    /* delete VLANs */

    /* AUTODOC: invalidate vlan 200 */
    rc = cstBrgDefVlanEntryInvalidate(devNum, PRV_TGF_SEND_VLANID_CNS);
    CHECK_RC_MAC(rc);

    /* AUTODOC: restore MAC SA modification on port egrPort */
    rc = cpssDxChIpRouterMacSaModifyEnable(devNum,prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],defaultMacSaModifyEnableGet);
    CHECK_RC_MAC(rc);

    /* AUTODOC: restore MAC SA LSB mode for port  */
    rc = cpssDxChIpPortRouterMacSaLsbModeSet(devNum,prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],defaultMacSaLsbModeGet);
    CHECK_RC_MAC(rc);

    /* AUTODOC: restore MAC SA configured to global MAC SA table into index 7 */
    rc = cpssDxChIpRouterGlobalMacSaSet(devNum, PRV_TGF_MAC_SA_GLOBAL_INDEX_CNS, &defaultMacSaAddrGet);
    CHECK_RC_MAC(rc);

    /* AUTODOC: restore index to global MAC SA table in eport attribute table */
    rc =  prvTgfIpRouterPortGlobalMacSaIndexSet(devNum,PRV_TGF_DESTINATION_EPORT_CNS,defaultRouterMacSaIndex);
    CHECK_RC_MAC(rc);

    {
        /* when tests trying to restore value that got from the 'get' ,
           in GM may come with 256..511 */
        defaultEportToPhysicalPortTargetMappingGet.devPort.portNum &= 0xFF;CPSS_TBD_BOOKMARK_BOBCAT3
    }

    /* AUTODOC: restore physical info for the given ePort in the E2PHY mapping table */
    rc = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum,PRV_TGF_DESTINATION_EPORT_CNS,&defaultEportToPhysicalPortTargetMappingGet);
    CHECK_RC_MAC(rc);

    /* AUTODOC: restore default  physical info for the given ePort in the HA Egress port table. */
    rc = cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(devNum,PRV_TGF_DESTINATION_EPORT_CNS,&defaultEportToPhysicalPortEgressPortInfoGet);
    CHECK_RC_MAC(rc);

    /* AUTODOC: restore the tunnel start on index routerArpTunnelStartLineIndex */
    rc = cpssDxChTunnelStartEntrySet(devNum,routerArpTunnelStartLineIndex,defaultTunnelTypeGet, &defaultTsConfigGet);
    CHECK_RC_MAC(rc);

    /* AUTODOC: restore TPID (Tag Protocol ID) table entry  */
    rc = cpssDxChBrgVlanTpidEntrySet(devNum,CPSS_DIRECTION_EGRESS_E,tpidEntryIndex,defaultEtherTypeGet);
    CHECK_RC_MAC(rc);

    /* AUTODOC: restore index of TPID tunnel-start header */
    rc = cpssDxChTunnelStartHeaderTpidSelectSet(devNum, PRV_TGF_DESTINATION_EPORT_CNS, defaultTpidEntryIndexGet);
    CHECK_RC_MAC(rc);

    /* AUTODOC: restore TTI lookup on port 0 */
    rc = cpssDxChTtiPortLookupEnableSet(devNum,portsArray[PRV_TGF_SEND_PORT_IDX_CNS],CPSS_DXCH_TTI_KEY_ETH_E,defaultTtiPortLookupEnableGet);
    CHECK_RC_MAC(rc);

    /* AUTODOC: restore the lookup Mac mode for the specified key type */
    rc = cpssDxChTtiMacModeSet(devNum,CPSS_DXCH_TTI_KEY_ETH_E,defaultTtiMacModeGet);
    CHECK_RC_MAC(rc);

    /* AUTODOC: invalidate added rules */
    rc = cpssDxChTtiRuleValidStatusSet(devNum, tunnelTerminationIndexPtr[0], GT_FALSE);
    CHECK_RC_MAC(rc);

    rc = cpssDxChTtiRuleValidStatusSet(devNum, tunnelTerminationIndexPtr[1], GT_FALSE);
    CHECK_RC_MAC(rc);

    return GT_OK;
}

/**
* @internal cstTunnelStartMplsVpwsConfiguration function
* @endinternal
*
* @brief   see file description.
*/
GT_STATUS cstTunnelStartMplsVpwsConfiguration
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U8   devNum = prvTgfDevNum;

    rc = cstTsMplsVpwsBrgConfigSet(devNum,portsArray[PRV_TGF_FDB_PORT_IDX_CNS]);
    CHECK_RC_MAC(rc);

    rc = cstTsMplsVpwsEportConfigSet(devNum);
    CHECK_RC_MAC(rc);

    rc = cstTsMplsVpwsTsConfigSet(devNum);
    CHECK_RC_MAC(rc);

    rc = cstTsMplsVpwsTtiConfigSet(devNum);
    CHECK_RC_MAC(rc);

    return GT_OK;
}

/**
* @internal csTunnelStartMplsVpwsRoutingTest function
* @endinternal
*
* @brief   see file description.
*/
GT_STATUS csTunnelStartMplsVpwsRoutingTest
(
    GT_VOID
)
{
    GT_STATUS   rc;
    /* to avoid compilation warnings.
       errorLine is used in CHECK_RC_MAC */
    TGF_PARAM_NOT_USED(errorLine);

    {/* use tested ports , because {0, 18, 36, 58} are not always valid */
        GT_U32  ii;
        for(ii = 0; ii < 4 ; ii++)
        {
            portsArray[ii] = prvTgfPortsArray[ii];
        }
    }

    /* perform basic L2 configurations */
    rc = cstTunnelStartMplsVpwsConfiguration();
    CHECK_RC_MAC(rc);

    cstTunnelStartMplsVpwsTrafficGenerate(GT_TRUE);

    cstTunnelStartMplsVpwsTrafficGenerate(GT_FALSE);

    rc = cstTunnelStartMplsVpwsConfigRestore();
    CHECK_RC_MAC(rc);

    return GT_OK;
}

#endif /* CHX_FAMILY */

