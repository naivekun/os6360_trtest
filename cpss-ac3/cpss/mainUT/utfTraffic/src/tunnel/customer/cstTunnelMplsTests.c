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
* @file cstTunnelMplsTests.c
*
* @brief test1: tunnel start unknown uc flooding
* MPLS configuration of device with 1 port in ETHERNET domain, and 3 ports in MPLS domain.
* test check 'flooding' from ethernet port to the other 3 mpls ports.(TS to the MPLS domain)
*
* ePort A is port to Ethernet domain
* eport B is port to MPLS domain with 1 label
* eport C is port to MPLS domain with 2 labels
* eport D is port to MPLS domain with 3 labels and PW label + FLOW label + CW label
*
* test2: transit tunnel termination of known uc (2 labels manipulations)
* MPLS configuration of device with 4 port in MPLS domain.
* test check 2 Labels manipulations from mpls port to other 2 mpls ports.(TT + TS in the MPLS domain)
* ePort A is port to MPLS domain with 2 labels
* eport B is port to MPLS domain with 2 labels. Label 1 is swapped.
* eport C is port to MPLS domain with 2 labels. Label 1 is popped & new label is pushed.
*
* test3: transit tunnel termination of known uc (2 labels manipulations)
* MPLS configuration of device with 4 port in MPLS domain.
* test check 2 Labels manipulations from mpls port to other 2 mpls ports.(TT + TS in the MPLS domain)
* ePort B is port to MPLS domain with 2 labels
* eport A is port to MPLS domain with 1 label. Label 1 is pop_swap.
* eport D is port to MPLS domain with 1 label. Label 1 is popped & swap.
*
* test4: transit tunnel termination of known uc (3 labels manipulations)
* MPLS configuration of device with 4 port in MPLS domain.
* test check 3 Labels manipulations from mpls port to other 3 mpls ports.(TT + TS in the MPLS domain)
* ePort A is port to MPLS domain with 3 labels
* eport B is port to MPLS domain with 1 label. Labels 1,2 are pop2.
* eport C is port to MPLS domain with 1 label. Label 1 is pop1 & label 2 is pop1.
* eport D is port to MPLS domain with 1 label. Labels 1,2 are pop2 & label 2 is pushed.
*
* test5: transit tunnel termination of known uc (3 labels manipulations)
* MPLS configuration of device with 4 port in MPLS domain.
* test check 3 Labels manipulations from mpls port to other 3 mpls ports.(TT + TS in the MPLS domain)
* ePort B is port to MPLS domain with 3 labels
* eport C is port to MPLS domain with 2 label. Label 1 is pop1.
* eport D is port to MPLS domain with 2 label. Label 1,2 are pop2 & new label 2 is push.
* eport A is port to MPLS domain with 2 label. Label 1 is pop1 & label 2 is pop_swap.
*
* test6: transit tunnel termination of known uc (4 labels manipulations)
* MPLS configuration of device with 4 port in MPLS domain.
* test check 4 Labels manipulations from mpls port to other 3 mpls ports.(TT + TS in the MPLS domain)
* ePort A is port to MPLS domain with 4 labels.
* eport B is port to MPLS domain with 1 label. Labels 1,2,3 are pop3
* eport C is port to MPLS domain with 1 label. Label 1,2 are pop2 & label 3 is pop1
* eport D is port to MPLS domain with 1 label. Label 1,2,3 are pop3 & pushed
*
* @version   11
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
#include <common/tgfIpGen.h>
#include <common/tgfTunnelGen.h>
#include <common/tgfL2MllGen.h>
#include <common/tgfPacketGen.h>
#include <l2mll/prvTgfL2MllUtils.h>
#include <tunnel/customer/cstTunnelMplsTests.h>

#ifdef CHX_FAMILY
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/dxCh/dxChxGen/l2mll/cpssDxChL2Mll.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgE2Phy.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgVlan.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdb.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgEgrFlt.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIpCtrl.h>
#include <cpss/dxCh/dxChxGen/tti/cpssDxChTti.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <common/customer/cstCommon.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfTunnelGen.h>
#endif  /* CHX_FAMILY */

#ifdef CHX_FAMILY

static GT_U32   errorLine = 0;


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* MPLS DOMAIN : VLAN Id */
#define PRV_CST_MPLS_VLANID_CNS         6

/* ETHERNET DOMAIN : VLAN Id */
#define PRV_CST_ETHERNET_VLANID_CNS     5

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS       0

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT1_IDX_CNS      1

/* port number to receive traffic */
#define PRV_TGF_RECEIVE_PORT_IDX_CNS    0

/* port number to receive traffic */
#define PRV_TGF_RECEIVE_PORT1_IDX_CNS   1

/* port number to receive traffic */
#define PRV_TGF_RECEIVE_PORT2_IDX_CNS   2

/* port number to receive traffic */
#define PRV_TGF_RECEIVE_PORT3_IDX_CNS   3

/* default number of packets to send */
static GT_U32 prvTgfBurstCount = 1;

/* number of ports */
static GT_U32 numOfPorts = 4;

/* number of labels */
static GT_U32 numOfLabels = 0;

/* MLL entry index */
static GT_U32 mllIndex = 2;

static GT_U32 arpIndex = 123;

/* eVidx to a pointer of linked-list of L2MLL > _4K */
static GT_U16   globalFloodEvidx = 0;

static GT_U8         swDevNum = 0;
static GT_HW_DEV_NUM hwDevNum = 0;

static GT_PHYSICAL_PORT_NUM phyPortArr[4] = {0,18,36,58};/* physical port associated with ePort_A..D */
static GT_PORT_NUM          ePortArr[4];
static GT_U16               eVlan;

/* tunnel start attributes */
static GT_ETHERADDR tsMacDaArr[4] =
{
    {{0}},
    {{0,0,0x44,0x44,0x44,0x44}},
    {{0,0,0x55,0x55,0x55,0x55}},
    {{0,0,0x66,0x66,0x66,0x66}}
};
static GT_ETHERADDR tsMacSaArr[4] =
{
    {{0,0,0,   0xf1,0xcc,0x01}},
    {{0,0,0xAA,0xf2,0xdd,0x02}},
    {{0,0,0xBB,0xf3,0xee,0x03}},
    {{0,0,0xCC,0xf4,0xff,0x04}}
};

/* tunnel termination attributes */
static GT_ETHERADDR ttMacDaArr[6] =
{
    {{0,0,0x11,0x11,0x11,0x11}},
    {{0,0,0x22,0x22,0x22,0x22}},
    {{0,0,0x33,0x33,0x33,0x33}},
    {{0,0,0x44,0x44,0x44,0x44}},
    {{0,0,0x55,0x55,0x55,0x55}},
    {{0,0,0x66,0x66,0x66,0x66}}
};

static GT_ETHERADDR MacAllZero = {{0,0,0,0,0,0}};

static GT_U8    timeToLive = 0x40;
static GT_U8    expTs = 7;
static GT_U32   mplsLabelArr[4]={0,0x11 /*eport B*/,0x22 /*eport C*/,0x33 /*eport D*/};
static GT_U32   mplsTtLabelArr[4]={0xAA ,0xBB,0xCC,0xDD};
static GT_U32   pwLabels[4] = {0,0,0,0x70 /*ePort D*/ };
static GT_U32   pwExp[4] = {0,0,0,6 /*ePort D*/};
static GT_U32   pwTtl[4] = {0,0,0,0x33 /*ePort D*/};

static GT_U32   flowTtl[4] = {0,0,0,0x44 /*ePort D*/};
static GT_U32   cwLabels[4] = {0,0,0,0x112233 /*ePort D*/};
static GT_U32   cwLabelIndex = 9;
static GT_U32   tunnelStartPointerArr[4] = {0/*eport A*/,1/*eport B*/,2/*eport C*/,3/*ePort D*/};

#define _18K    (18 * _1K)
#define _30K    (30 * _1K)
#define numOfTT  11
static GT_U32   tunnelTerminationIndex[numOfTT] = {_18K +0x11*3,_18K +0x22*3,_30K + 0x33*3,_18K + 0x44*3,_18K + 0x55*3, _30K + 0x66*3,
                                                   _18K + 0x77*3, _18K + 0x88*3,_30K + 0x99*3, _18K + 0xAA*3, _30K + 0xBB*3};

#define _15K    (15 * _1K)
static GT_U32   tunnelTerminationIndexBobK[numOfTT] = {_9K +0x11*3,_9K +0x22*3,_15K + 0x33*3,_9K + 0x44*3,_9K + 0x55*3, _15K + 0x66*3,
                                                   _9K + 0x77*3, _9K + 0x88*3,_15K + 0x99*3, _9K + 0xAA*3, _15K + 0xBB*3};

/* aldrin and all other devices with only 3 TCAM floors use floor#1 first 6 TCAM arrays for TTI_0 lookup
   Need to use absolute indexes of floor#1 _3K + m + 12*n, where m 0..5 - array index, n - row index 0..255  */
#define ALDRIN_TTI_0_INDEX_MAC(_relativeIdx)  (_3K + ((3*_relativeIdx) % 6) + ((3*_relativeIdx)/6)*12)
#define ALDRIN_TTI_1_INDEX_MAC(_relativeIdx)  (_3K + 6 + ((3*_relativeIdx) % 6) + ((3*_relativeIdx)/6)*12)

static GT_U32   tunnelTerminationIndexAldrin[numOfTT]  = {ALDRIN_TTI_0_INDEX_MAC(0x11), ALDRIN_TTI_0_INDEX_MAC(0x22), ALDRIN_TTI_1_INDEX_MAC(0x33),
                                                          ALDRIN_TTI_0_INDEX_MAC(0x44), ALDRIN_TTI_0_INDEX_MAC(0x55), ALDRIN_TTI_1_INDEX_MAC(0x66),
                                                          ALDRIN_TTI_0_INDEX_MAC(0x77), ALDRIN_TTI_0_INDEX_MAC(0x88), ALDRIN_TTI_1_INDEX_MAC(0x99),
                                                          ALDRIN_TTI_0_INDEX_MAC(0xAA), ALDRIN_TTI_1_INDEX_MAC(0xBB)};

GT_U32   * tunnelTerminationIndexPtr;

/* set in runtime */
static GT_U32   generic_tunnelTerminationIndex[numOfTT];
#define HIT_0 0
#define HIT_1 1
static GT_U32   generic_TcamHitNum[numOfTT] = {HIT_0,HIT_0,HIT_1,
                                               HIT_0,HIT_0,HIT_1,
                                               HIT_0,HIT_0,HIT_1,
                                               HIT_0,HIT_1 };

/* ethernet packet to send */
static TGF_PACKET_STC prvTgfEtherPacketInfo;
/* ethernet over mpls packet to send */
static TGF_PACKET_STC prvTgfEtherOverMplsPacketInfo;

/* ethernet packet attributes to save */
static TGF_PACKET_L2_STC ethernetPacketL2;
static TGF_PACKET_VLAN_TAG_STC ethernetPacketVlan;
static TGF_PACKET_ETHERTYPE_STC ethernetPacketEtherType;

/* expected counters : all counters should be ZERO */
static CPSS_PORT_MAC_COUNTER_SET_STC   allZeroCntrs;

/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static struct
{
    CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT  stateMode[4];
    CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT          tagCmd[4];
    CPSS_INTERFACE_INFO_STC                      physicalInfo[4];
    GT_BOOL                                      vlanEgressFilteringState;
    GT_BOOL                                      portLookupEnable[2];
    CPSS_DXCH_TTI_MAC_MODE_ENT                   macMode;

} prvCstRestoreCfg;

/******************************************************************************/
/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/
/**
* @internal prvCstSetBridge function
* @endinternal
*
* @brief   Set Bridge Configuration
*
* @param[in] phyPortArr[4]            - physical ports
* @param[in] vlanId                   - vlan id
*                                       None
*/
static GT_STATUS prvCstSetBridge
(
    IN GT_PHYSICAL_PORT_NUM  phyPortArr[4], /* physical port associated with ePort_A..D */
    IN GT_U16                vlanId
)
{
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC    portsTaggingCmd;
    CPSS_PORTS_BMP_STC                      portsMembers;
    CPSS_PORTS_BMP_STC                      portsTagging;
    CPSS_DXCH_BRG_VLAN_INFO_STC             vlanInfo;
    GT_BOOL                                 addPort = GT_TRUE;

    /* AUTODOC: BRIDGE SETUP CONFIGURATION: */

    /* clear entry */
    cpssOsMemSet(&portsMembers,0,sizeof(portsMembers));
    cpssOsMemSet(&portsTagging,0,sizeof(portsTagging));
    cpssOsMemSet(&portsTaggingCmd,0,sizeof(portsTaggingCmd));
    cpssOsMemSet(&vlanInfo,0,sizeof(vlanInfo));

    /* AUTODOC: define global eVlan : physical ports members , controlled learning ,
                specific evidx for flooding */

    /* AUTODOC: add the physical port to global eVlan */
    CPSS_PORTS_BMP_PORT_ENABLE_MAC(&portsMembers,phyPortArr[0],addPort);
    CPSS_PORTS_BMP_PORT_ENABLE_MAC(&portsMembers,phyPortArr[1],addPort);
    CPSS_PORTS_BMP_PORT_ENABLE_MAC(&portsMembers,phyPortArr[2],addPort);
    CPSS_PORTS_BMP_PORT_ENABLE_MAC(&portsMembers,phyPortArr[3],addPort);

    /* AUTODOC: set global eVlan controlled learning*/
    vlanInfo.autoLearnDisable     = GT_TRUE;/* working in controlled learning */
    vlanInfo.naMsgToCpuEn         = GT_TRUE;/* working in controlled learning */

    /* AUTODOC: specific evidx for flooding */
    vlanInfo.floodVidx            = globalFloodEvidx;
    vlanInfo.floodVidxMode        = CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E;

    /* AUTODOC: write the vlan entry */
    CHECK_RC_MAC(
        cpssDxChBrgVlanEntryWrite(swDevNum,
                                  vlanId,
                                  &portsMembers,
                                  &portsTagging,
                                  &vlanInfo,
                                  &portsTaggingCmd));

     /* AUTODOC: disable VLAN egress filtering to allow destination port to be member
                 in MPLS VLAN and not be member of the native ETHERNET VLAN */

    /* AUTODOC: save VLAN egress filtering configuration */
    CHECK_RC_MAC(
        cpssDxChBrgVlanEgressFilteringEnableGet(swDevNum,&(prvCstRestoreCfg.vlanEgressFilteringState)));

    /* AUTODDOC: disable VLAN egress filtering */
    CHECK_RC_MAC(
        cpssDxChBrgVlanEgressFilteringEnable(swDevNum,GT_FALSE));

    return GT_OK;

}

/**
* @internal prvCstSetL2Mll function
* @endinternal
*
* @brief   Set L2MLL configuration
*
* @param[in] ePortArr[4]              - eports
*                                       None
*/
static GT_STATUS prvCstSetL2Mll
(
    IN  GT_PORT_NUM           ePortArr[4]   /* ePort_A..D */
)
{
    CPSS_DXCH_L2_MLL_LTT_ENTRY_STC              lttEntry;
    CPSS_DXCH_L2_MLL_PAIR_STC                   mllPairEntry;
    CPSS_DXCH_L2_MLL_ENTRY_STC                 *mllEntryPtr;
    GT_U32                                      mllPairIndex;

    /* AUTODOC: L2MLL SETUP CONFIGURATION: */

    /* create L2 MLL LTT entries */
    cpssOsMemSet(&lttEntry, 0, sizeof(lttEntry));

    /* AUTODOC: map the eVIDX to a pointer of linked-list of L2MLL */
    mllPairIndex = mllIndex; /*index on first entry */
    lttEntry.mllPointer = mllPairIndex;
    lttEntry.entrySelector = 0; /* start of the entry */

    /* AUTODOC: set the L2MLL LTT entry (index = eVidx of global eVlan) */
    /* AUTODOC: the LTT point to index in the MLL table */
    CHECK_RC_MAC(
        cpssDxChL2MllLttEntrySet(swDevNum,
        (globalFloodEvidx - _4K),/* working in mode of relative to _4K */
        &lttEntry));

    /* create L2 MLL entries */
    cpssOsMemSet(&mllPairEntry, 0, sizeof(mllPairEntry));

    /* AUTODOC: define 'link list' of 'MLL nodes' one per eport */
    /* AUTODOC: use 2 MLL entries */

    /* AUTODOC: set the 3 eports that are members in the evlan into the MLL*/
    mllEntryPtr = &mllPairEntry.firstMllNode;
    mllEntryPtr->last = GT_FALSE;
    mllEntryPtr->egressInterface.type = CPSS_INTERFACE_PORT_E;
    mllEntryPtr->egressInterface.devPort.hwDevNum = hwDevNum;
    mllEntryPtr->egressInterface.devPort.portNum = ePortArr[1];

    mllEntryPtr = &mllPairEntry.secondMllNode;
    mllEntryPtr->last = GT_FALSE;
    mllEntryPtr->egressInterface.type = CPSS_INTERFACE_PORT_E;
    mllEntryPtr->egressInterface.devPort.hwDevNum = hwDevNum;
    mllEntryPtr->egressInterface.devPort.portNum = ePortArr[2];

    mllPairEntry.nextPointer = mllPairIndex + 1;
    mllPairEntry.entrySelector = 0;/* start of the entry */

    /* AUTODOC: write first MLL entry (2 nodes)*/
    CHECK_RC_MAC(
        cpssDxChL2MllPairWrite(swDevNum, mllPairIndex ,
                               CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E,
                               &mllPairEntry));

    mllEntryPtr = &mllPairEntry.firstMllNode;
    mllEntryPtr->last = GT_TRUE;/* AUTODOC: last eport --> the end of the list */
    mllEntryPtr->egressInterface.type = CPSS_INTERFACE_PORT_E;
    mllEntryPtr->egressInterface.devPort.hwDevNum = hwDevNum;
    mllEntryPtr->egressInterface.devPort.portNum = ePortArr[3];

    /* AUTODOC: write second MLL entry (1 node) */
    CHECK_RC_MAC(
        cpssDxChL2MllPairWrite(swDevNum, mllPairIndex + 1,
                               CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E,
                               &mllPairEntry));


    return GT_OK;

}


/**
* @internal prvCstSetMplsTunnelStart function
* @endinternal
*
* @brief   Set MPLS Tunnel Start configuration
*
* @param[in] phyPortArr[4]            - physical ports
* @param[in] ePortArr[4]              - eports
*                                       None
*/
static GT_STATUS prvCstSetMplsTunnelStart
(
    IN  GT_PHYSICAL_PORT_NUM  phyPortArr[4],/* physical port associated with ePort_A..D */
    IN  GT_PORT_NUM           ePortArr[4]   /* ePort_A..D */
)
{
    CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC          egressEportInfo;
    CPSS_DXCH_TUNNEL_START_CONFIG_UNT           tsEntry;
    CPSS_DXCH_TUNNEL_START_MPLS_CONFIG_STC     *mplsTsEntryPtr;
    GT_U32                                      ii;

    /* AUTODOC: EGRESS MPLS TUNNEL SETUP CONFIGURATION: */

    cpssOsMemSet(&egressEportInfo,0,sizeof(egressEportInfo));
    egressEportInfo.tunnelStart = GT_TRUE;
    egressEportInfo.tsPassengerPacketType = CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E;

    /* AUTODOC: define the TS entries for eports B,C,D */

    /* AUTODOC: eport B - need tunnel start with 1 label */
    /* AUTODOC: eport C - need tunnel start with 2 labels */
    /* AUTODOC: eport D - need tunnel start with 3 labels and PW label + FLOW label + CW label */
    for(ii = 1/*eport B*/ ; ii < numOfPorts ; ii++)
    {
        /* set tunnel start entry */
        cpssOsMemSet(&tsEntry,0,sizeof(tsEntry));
        mplsTsEntryPtr = &tsEntry.mplsCfg;

        mplsTsEntryPtr->tagEnable = GT_TRUE;
        mplsTsEntryPtr->vlanId = PRV_CST_MPLS_VLANID_CNS;
        mplsTsEntryPtr->upMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
        mplsTsEntryPtr->up = 0;
        mplsTsEntryPtr->ttl = timeToLive;
        mplsTsEntryPtr->ttlMode = CPSS_DXCH_TUNNEL_START_TTL_TO_INCOMING_TTL_E;
        mplsTsEntryPtr->retainCRC = GT_FALSE;/*AUTODOC: the passenger is without CRC (CRC only for full packet) */
        mplsTsEntryPtr->cfi = 0;
        mplsTsEntryPtr->mplsEthertypeSelect = CPSS_DXCH_TUNNEL_START_MPLS_ETHER_TYPE_UC_E;

        mplsTsEntryPtr->numLabels = ii;
        mplsTsEntryPtr->macDa = tsMacDaArr[ii];

        switch (ii)
        {
        case 1:/* eport B */
            mplsTsEntryPtr->label1 = mplsLabelArr[1];
            mplsTsEntryPtr->exp1MarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            mplsTsEntryPtr->exp1 = expTs;
            break;
        case 2: /* eport C */
            mplsTsEntryPtr->label1 = mplsLabelArr[1];
            mplsTsEntryPtr->exp1MarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            mplsTsEntryPtr->exp1 = expTs;
            mplsTsEntryPtr->label2 = mplsLabelArr[2];
            mplsTsEntryPtr->exp2MarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            mplsTsEntryPtr->exp2 = expTs;
            break;
        case 3: /* eport D */
            mplsTsEntryPtr->label1 = mplsLabelArr[1];
            mplsTsEntryPtr->exp1MarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            mplsTsEntryPtr->exp1 = expTs;
            mplsTsEntryPtr->label2 = mplsLabelArr[2];
            mplsTsEntryPtr->exp2MarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            mplsTsEntryPtr->exp2 = expTs;
            mplsTsEntryPtr->label3 = mplsLabelArr[3];
            mplsTsEntryPtr->exp3MarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
            mplsTsEntryPtr->exp3 = expTs;

            /* AUTODOC: enable the PW label */
            CHECK_RC_MAC(
                cpssDxChTunnelStartMplsPwLabelPushEnableSet(swDevNum,ePortArr[ii],GT_TRUE));

            /* AUTODOC: set the PW label */
            CHECK_RC_MAC(
                cpssDxChTunnelStartMplsPwLabelSet(swDevNum,ePortArr[ii],pwLabels[ii]));

            /* AUTODOC: set the PW label EXP */
            CHECK_RC_MAC(
                cpssDxChTunnelStartMplsPwLabelExpSet(swDevNum,ePortArr[ii],pwExp[ii]));

            /* AUTODOC: set the PW label TTL */
            CHECK_RC_MAC(
                cpssDxChTunnelStartMplsPwLabelTtlSet(swDevNum,ePortArr[ii],pwTtl[ii]));

            /* AUTODOC: enable the FLOW label */
            CHECK_RC_MAC(
                cpssDxChTunnelStartMplsFlowLabelEnableSet(swDevNum,ePortArr[ii],GT_TRUE));

            /* AUTODOC: Set the FLOW label TTL */
            CHECK_RC_MAC(
                cpssDxChTunnelStartMplsFlowLabelTtlSet(swDevNum,(GT_U8)flowTtl[ii]));

            /* AUTODOC: Sets a Pseudo Wire control word */
            CHECK_RC_MAC(
                cpssDxChTunnelStartMplsPwControlWordSet(swDevNum,cwLabelIndex,cwLabels[ii]));

            mplsTsEntryPtr->controlWordEnable = GT_TRUE;
            mplsTsEntryPtr->controlWordIndex = cwLabelIndex;

            break;
        }

        /* AUTODOC: bind the eport to tunnel start */
        egressEportInfo.tunnelStartPtr  = tunnelStartPointerArr[ii];

        /* AUTODOC: config ePort attributes */
        CHECK_RC_MAC(
            cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(swDevNum,ePortArr[ii],&egressEportInfo));

        /* AUTODOC: add the tunnels */
        CHECK_RC_MAC(
            cpssDxChTunnelStartEntrySet(swDevNum,
                                        egressEportInfo.tunnelStartPtr/*routerArpTunnelStartLineIndex*/,
                                        CPSS_TUNNEL_X_OVER_MPLS_E,
                                        &tsEntry));

    }

    for(ii = 0/*eport A*/ ; ii < numOfPorts ; ii++)
    {
        /* AUTODOC: define the 'mac from me' (Global Mac Sa table) --> used as mac SA for TS */
        /* AUTODOC: bind the eport to index (index  = physical port) in the
                    global 'mac from me' table */
        CHECK_RC_MAC(
            cpssDxChIpRouterPortGlobalMacSaIndexSet(
                swDevNum, ePortArr[ii], phyPortArr[ii] & 0xFF));

       /* AUTODOC: set unique mac SA at index (index  = physical port) in the
                   global 'mac from me' table */
       CHECK_RC_MAC(
           cpssDxChIpRouterGlobalMacSaSet(
               swDevNum, phyPortArr[ii] & 0xFF,&tsMacSaArr[ii]));
    }

    if (globalFloodEvidx == 0)
    {
        /* AUTODOC: unset configuration from ePort D */

         /* AUTODOC: disable the PW label */
            CHECK_RC_MAC(
                cpssDxChTunnelStartMplsPwLabelPushEnableSet(swDevNum,ePortArr[3],GT_FALSE));

            /* AUTODOC: disable the FLOW label */
            CHECK_RC_MAC(
                cpssDxChTunnelStartMplsFlowLabelEnableSet(swDevNum,ePortArr[3],GT_FALSE));
    }

    return GT_OK;
}

/**
* @internal prvCstSetUcMplsTunnelStart function
* @endinternal
*
* @brief   Set MPLS Tunnel Start configuration
*
* @param[in] phyPortArr[4]            - physical ports
* @param[in] ePortArr[4]              - eports
*                                       None
*/
static GT_STATUS prvCstSetUcMplsTunnelStart
(
    IN  GT_PHYSICAL_PORT_NUM  phyPortArr[4],/* physical port associated with ePort_A..D */
    IN  GT_PORT_NUM           ePortArr[4]   /* ePort_A..D */
)
{

    CPSS_DXCH_TUNNEL_START_CONFIG_UNT           tsEntry;
    CPSS_DXCH_TUNNEL_START_MPLS_CONFIG_STC     *mplsTsEntryPtr;
    GT_U32                                      ii;

    /* AUTODOC: EGRESS MPLS TUNNEL SETUP CONFIGURATION: */

    /* AUTODOC: define the TS entries with 1 label*/

    /* set tunnel start entry */
    cpssOsMemSet(&tsEntry,0,sizeof(tsEntry));
    mplsTsEntryPtr = &tsEntry.mplsCfg;

    mplsTsEntryPtr->tagEnable = GT_TRUE;
    mplsTsEntryPtr->vlanId = PRV_CST_MPLS_VLANID_CNS;
    mplsTsEntryPtr->upMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
    mplsTsEntryPtr->up = 0;
    mplsTsEntryPtr->ttl = timeToLive;
    mplsTsEntryPtr->ttlMode = CPSS_DXCH_TUNNEL_START_TTL_TO_INCOMING_TTL_E;
    mplsTsEntryPtr->retainCRC = GT_FALSE;/*AUTODOC: the passenger is without CRC (CRC only for full packet) */
    mplsTsEntryPtr->cfi = 0;
    mplsTsEntryPtr->mplsEthertypeSelect = CPSS_DXCH_TUNNEL_START_MPLS_ETHER_TYPE_UC_E;

    mplsTsEntryPtr->numLabels = 1;
    mplsTsEntryPtr->macDa = tsMacDaArr[3];
    mplsTsEntryPtr->exp1MarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
    mplsTsEntryPtr->exp1 = expTs;

    mplsTsEntryPtr->label1 = mplsLabelArr[0]; /* for ePort A*/

    /* AUTODOC: add the tunnels */
    CHECK_RC_MAC(
        cpssDxChTunnelStartEntrySet(swDevNum,
                                    tunnelStartPointerArr[0]/*routerArpTunnelStartLineIndex*/,
                                    CPSS_TUNNEL_X_OVER_MPLS_E,
                                    &tsEntry));


    mplsTsEntryPtr->label1 = mplsLabelArr[3]; /* for ePort D*/

    /* AUTODOC: add the tunnels */
    CHECK_RC_MAC(
        cpssDxChTunnelStartEntrySet(swDevNum,
                                    tunnelStartPointerArr[3]/*routerArpTunnelStartLineIndex*/,
                                    CPSS_TUNNEL_X_OVER_MPLS_E,
                                    &tsEntry));

    for(ii = 0/*eport A*/ ; ii < numOfPorts ; ii++)
    {
        /* AUTODOC: define the 'mac from me' (Global Mac Sa table) --> used as mac SA for TS */
        /* AUTODOC: bind the eport to index (index  = physical port) in the
                    global 'mac from me' table */
        CHECK_RC_MAC(
            cpssDxChIpRouterPortGlobalMacSaIndexSet(
                swDevNum, ePortArr[ii], phyPortArr[ii] & 0xFF));

       /* AUTODOC: set unique mac SA at index (index  = physical port) in the
                   global 'mac from me' table */
       CHECK_RC_MAC(
           cpssDxChIpRouterGlobalMacSaSet(
               swDevNum, phyPortArr[ii] & 0xFF,&tsMacSaArr[ii]));
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(swDevNum))
    {    
        CHECK_RC_MAC( 
            cpssDxChIpRouterVlanMacSaLsbSet(swDevNum,PRV_CST_MPLS_VLANID_CNS,PRV_CST_MPLS_VLANID_CNS));
    }
    

    return GT_OK;
}

/**
* @internal prvCstSetE2Phy function
* @endinternal
*
* @brief   Set E2Phy configuration
*
* @param[in] phyPortArr[4]            - physical ports
* @param[in] ePortArr[4]              - eports
*                                       None
*/
static GT_STATUS prvCstSetE2Phy
(
    IN  GT_PHYSICAL_PORT_NUM  phyPortArr[4],/* physical port associated with ePort_A..D */
    IN  GT_PORT_NUM           ePortArr[4]   /* ePort_A..D */
)
{
    GT_U32  ii;
    CPSS_INTERFACE_INFO_STC phyPortInfo;

    phyPortInfo.type = CPSS_INTERFACE_PORT_E;
    phyPortInfo.devPort.hwDevNum = hwDevNum;

    for(ii = 0 ; ii < numOfPorts ; ii++)
    {
         /* AUTODOC: save physical info for the given ePort in the E2PHY mapping table */
        CHECK_RC_MAC(
            cpssDxChBrgEportToPhysicalPortTargetMappingTableGet(swDevNum,
                                                                ePortArr[ii],&prvCstRestoreCfg.physicalInfo[ii]));

        /* AUTODOC: bind the eport to the needed physical port */
        phyPortInfo.devPort.portNum = phyPortArr[ii];

        CHECK_RC_MAC(
            cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(swDevNum,
                                                                ePortArr[ii],&phyPortInfo));
    }

    return GT_OK;
}

/**
* @internal prvCstSetEgressTagging function
* @endinternal
*
* @brief   Set Egress tagging configuration
*/
static GT_STATUS prvCstSetEgressTagging
(
    IN  GT_PORT_NUM           ePortArr[4]   /* ePort_A..D */
)
{
    GT_U32  ii;

    /* AUTODOC: eport B,C,D - tag0 only*/
    for(ii = 1; ii < numOfPorts ; ii++)
    {
        /* AUTODOC: save the egress port tag state mode */
        CHECK_RC_MAC(
            cpssDxChBrgVlanEgressPortTagStateModeGet(swDevNum,ePortArr[ii],
                                                     &prvCstRestoreCfg.stateMode[ii]));

        /* AUTODOC: use tagging from eport not from vlan */
        CHECK_RC_MAC(
            cpssDxChBrgVlanEgressPortTagStateModeSet(swDevNum,ePortArr[ii],
                                                     CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_EPORT_MODE_E));

        /* AUTODOC: save the egress port state */
        CHECK_RC_MAC(
            cpssDxChBrgVlanEgressPortTagStateGet(swDevNum,ePortArr[ii],
                                                 &prvCstRestoreCfg.tagCmd[ii]));

        /* AUTODOC: egress tag1*/
        CHECK_RC_MAC(
            cpssDxChBrgVlanEgressPortTagStateSet(swDevNum,ePortArr[ii],
                                                 CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E));


    }

    return GT_OK;
}

/**
* @internal prvCstSetMpls2LTunnelTermination function
* @endinternal
*
* @brief   Set ingress 2 Labels tunnel termination configuration
*
* @param[in] phyPortArr[4]            - physical ports
* @param[in] ePortArr[4]              - eports
*                                       None
*/
static GT_STATUS prvCstSetMpls2LTunnelTermination
(
    IN  GT_PHYSICAL_PORT_NUM  phyPortArr[4],/* physical port associated with ePort_A..D */
    IN  GT_PORT_NUM           ePortArr[4]   /* ePort_A..D */
)
{
    CPSS_DXCH_TTI_RULE_UNT          ttiPattern;
    CPSS_DXCH_TTI_MPLS_RULE_STC    *ttiMplsPatternPtr;
    CPSS_DXCH_TTI_RULE_UNT          ttiMask;
    CPSS_DXCH_TTI_MPLS_RULE_STC    *ttiMplsMaskPtr;
    CPSS_DXCH_TTI_ACTION_UNT        ttiAction;
    CPSS_DXCH_TTI_ACTION_2_STC     *ttiAction2Ptr;
    GT_U32      ii;

    cpssOsMemSet(&ttiPattern,0,sizeof(ttiPattern));
    cpssOsMemSet(&ttiMask   ,0,sizeof(ttiMask));
    cpssOsMemSet(&ttiAction ,0,sizeof(ttiAction));

    /* AUTODOC: set 2 Labels tunnel termination */
    /* AUTODOC: use MPLS rule on 'ingress port' + 2 MPLS labels */

    ttiMplsPatternPtr = &ttiPattern.mpls;
    ttiMplsMaskPtr = &ttiMask.mpls;
    ttiAction2Ptr = &ttiAction.type2;

    /* pattern : port (not trunk) , 2 labels */
    ttiMplsPatternPtr->common.srcIsTrunk   = 0;
    ttiMplsPatternPtr->numOfLabels     = 1;/* 1 means 2 labels !!! */

    /* mask : port (not trunk) , 2 labels */
    ttiMplsMaskPtr->common.srcIsTrunk   = BIT_1-1;/*1bit*/
    ttiMplsMaskPtr->numOfLabels  = BIT_2-1;/*2bits*/

    /* pattern : specific port , label1,2, mac da*/
    ttiMplsPatternPtr->common.srcPortTrunk = phyPortArr[0];
    ttiMplsPatternPtr->label0       = mplsTtLabelArr[0];
    ttiMplsPatternPtr->label1       = mplsTtLabelArr[1];

    /* mask : specific port , label1,2, mac da*/
    ttiMplsMaskPtr->common.srcPortTrunk = BIT_8-1;/*8 bits*/
    ttiMplsMaskPtr->label0       = BIT_20-1;/*20 bits*/
    ttiMplsMaskPtr->label1       = BIT_20-1;/*20 bits*/
    cpssOsMemSet(&ttiMplsMaskPtr->common.mac, 0xFF, sizeof(ttiMplsMaskPtr->common.mac));


    /* AUTODOC: define the action for ePort B*/

    /*action*/
    ttiAction2Ptr->tunnelTerminate = GT_FALSE;
    ttiAction2Ptr->tsPassengerPacketType = CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E;
    ttiAction2Ptr->mplsCommand = CPSS_DXCH_TTI_MPLS_SWAP_CMD_E;
    ttiAction2Ptr->mplsTtl = timeToLive;
    ttiAction2Ptr->enableDecrementTtl = GT_TRUE;
    ttiAction2Ptr->command = CPSS_PACKET_CMD_FORWARD_E;
    ttiAction2Ptr->redirectCommand = CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E;
    ttiAction2Ptr->egressInterface.type = CPSS_INTERFACE_PORT_E;
    ttiAction2Ptr->egressInterface.devPort.portNum = ePortArr[1];
    ttiAction2Ptr->egressInterface.devPort.hwDevNum = hwDevNum;
    ttiAction2Ptr->tag1VlanCmd = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;
    ttiAction2Ptr->bridgeBypass = GT_TRUE;
    ttiAction2Ptr->continueToNextTtiLookup = GT_FALSE;
    ttiAction2Ptr->tunnelStart = GT_TRUE;
    ttiAction2Ptr->tunnelStartPtr = tunnelStartPointerArr[1];

    ttiMplsPatternPtr->common.mac   = ttMacDaArr[0];

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
            prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(generic_TcamHitNum[ii],tunnelTerminationIndexPtr[ii]);
    }

    tunnelTerminationIndexPtr = &generic_tunnelTerminationIndex[0];

    /* AUTODOC: set the TTI MPLS rule+action */
    CHECK_RC_MAC(
        cpssDxChTtiRuleSet(swDevNum,
                           tunnelTerminationIndexPtr[0],/* for ePort B*/
                           CPSS_DXCH_TTI_RULE_MPLS_E/* CPSS_TUNNEL_X_OVER_MPLS_E*/,
                           &ttiPattern,&ttiMask,
                           CPSS_DXCH_TTI_ACTION_TYPE2_ENT,&ttiAction));


    /* AUTODOC: define the action for ePort C*/

    ttiMplsPatternPtr->common.mac   = ttMacDaArr[1];
    ttiAction2Ptr->egressInterface.devPort.portNum = ePortArr[2];
    ttiAction2Ptr->mplsCommand = CPSS_DXCH_TTI_MPLS_POP1_CMD_E;
    ttiAction2Ptr->bridgeBypass = GT_TRUE;
    ttiAction2Ptr->tunnelStart = GT_FALSE;
    ttiAction2Ptr->continueToNextTtiLookup = GT_TRUE;

    /* AUTODOC: set the TTI MPLS rule+action */
    CHECK_RC_MAC(
        cpssDxChTtiRuleSet(swDevNum,
                           tunnelTerminationIndexPtr[1],/* for ePort C lookup 0*/
                           CPSS_DXCH_TTI_RULE_MPLS_E/* CPSS_TUNNEL_X_OVER_MPLS_E*/,
                           &ttiPattern,&ttiMask,
                           CPSS_DXCH_TTI_ACTION_TYPE2_ENT,&ttiAction));

    ttiMplsPatternPtr->common.mac   = ttMacDaArr[1];
    ttiAction2Ptr->egressInterface.devPort.portNum = ePortArr[2];
    ttiAction2Ptr->mplsCommand = CPSS_DXCH_TTI_MPLS_PUSH1_CMD_E;
    ttiAction2Ptr->bridgeBypass = GT_TRUE;
    ttiAction2Ptr->tunnelStart = GT_TRUE;
    ttiAction2Ptr->tunnelStartPtr = tunnelStartPointerArr[1];

    /* AUTODOC: set the TTI MPLS rule+action */
    CHECK_RC_MAC(
        cpssDxChTtiRuleSet(swDevNum,
                           tunnelTerminationIndexPtr[2],/* for ePort C lookup 1*/
                           CPSS_DXCH_TTI_RULE_MPLS_E/* CPSS_TUNNEL_X_OVER_MPLS_E*/,
                           &ttiPattern,&ttiMask,
                           CPSS_DXCH_TTI_ACTION_TYPE2_ENT,&ttiAction));


    /* AUTODOC: define the action for ePort A */

    ttiMplsPatternPtr->common.srcPortTrunk = phyPortArr[1];
    ttiMplsPatternPtr->common.mac   = ttMacDaArr[2];
    ttiAction2Ptr->egressInterface.devPort.portNum = ePortArr[0];
    ttiAction2Ptr->mplsCommand = CPSS_DXCH_TTI_MPLS_POP_AND_SWAP_CMD_E;
    ttiAction2Ptr->bridgeBypass = GT_TRUE;
    ttiAction2Ptr->continueToNextTtiLookup = GT_FALSE;
    ttiAction2Ptr->tunnelStart = GT_TRUE;
    ttiAction2Ptr->tunnelStartPtr = tunnelStartPointerArr[1];

    /* AUTODOC: set the TTI MPLS rule+action */
    CHECK_RC_MAC(
        cpssDxChTtiRuleSet(swDevNum,
                           tunnelTerminationIndexPtr[3],/* for ePort A*/
                           CPSS_DXCH_TTI_RULE_MPLS_E/* CPSS_TUNNEL_X_OVER_MPLS_E*/,
                           &ttiPattern,&ttiMask,
                           CPSS_DXCH_TTI_ACTION_TYPE2_ENT,&ttiAction));

    /* AUTODOC: define the action for ePort D */

    ttiMplsPatternPtr->common.srcPortTrunk = phyPortArr[1];
    ttiMplsPatternPtr->common.mac   = ttMacDaArr[3];
    ttiAction2Ptr->egressInterface.devPort.portNum = ePortArr[3];
    ttiAction2Ptr->mplsCommand = CPSS_DXCH_TTI_MPLS_POP1_CMD_E;
    ttiAction2Ptr->bridgeBypass = GT_TRUE;
    ttiAction2Ptr->continueToNextTtiLookup = GT_TRUE;
    ttiAction2Ptr->tunnelStart = GT_FALSE;

    /* AUTODOC: set the TTI MPLS rule+action */
    CHECK_RC_MAC(
        cpssDxChTtiRuleSet(swDevNum,
                           tunnelTerminationIndexPtr[4],/* for ePort D*/
                           CPSS_DXCH_TTI_RULE_MPLS_E/* CPSS_TUNNEL_X_OVER_MPLS_E*/,
                           &ttiPattern,&ttiMask,
                           CPSS_DXCH_TTI_ACTION_TYPE2_ENT,&ttiAction));

    ttiMplsPatternPtr->common.srcPortTrunk = phyPortArr[1];
    ttiMplsPatternPtr->common.mac   = ttMacDaArr[3];
    ttiAction2Ptr->egressInterface.devPort.portNum = ePortArr[3];
    ttiAction2Ptr->mplsCommand = CPSS_DXCH_TTI_MPLS_SWAP_CMD_E;
    ttiAction2Ptr->bridgeBypass = GT_TRUE;
    ttiAction2Ptr->continueToNextTtiLookup = GT_FALSE;
    ttiAction2Ptr->tunnelStart = GT_TRUE;
    ttiAction2Ptr->tunnelStartPtr = tunnelStartPointerArr[1];

    /* AUTODOC: set the TTI MPLS rule+action */
    CHECK_RC_MAC(
        cpssDxChTtiRuleSet(swDevNum,
                           tunnelTerminationIndexPtr[5],/* for ePort D*/
                           CPSS_DXCH_TTI_RULE_MPLS_E/* CPSS_TUNNEL_X_OVER_MPLS_E*/,
                           &ttiPattern,&ttiMask,
                           CPSS_DXCH_TTI_ACTION_TYPE2_ENT,&ttiAction));

    /* AUTODOC: save physical port to do TTI MPLS lookup */
    CHECK_RC_MAC(
        cpssDxChTtiPortLookupEnableGet(swDevNum,phyPortArr[0],
                                       CPSS_DXCH_TTI_KEY_MPLS_E,
                                       &prvCstRestoreCfg.portLookupEnable[0]));

    /* AUTODOC: enable physical port to do TTI MPLS lookup */
    CHECK_RC_MAC(
        cpssDxChTtiPortLookupEnableSet(swDevNum,phyPortArr[0],
                                       CPSS_DXCH_TTI_KEY_MPLS_E,
                                       GT_TRUE));

    /* AUTODOC: save the lookup Mac mode */
    CHECK_RC_MAC(
        cpssDxChTtiMacModeGet(swDevNum,CPSS_DXCH_TTI_RULE_MPLS_E,
                              &prvCstRestoreCfg.macMode));

    /* AUTODOC: sets the lookup Mac mode */
    CHECK_RC_MAC(
        cpssDxChTtiMacModeSet(swDevNum,CPSS_DXCH_TTI_RULE_MPLS_E,
                              CPSS_DXCH_TTI_MAC_MODE_DA_E));

    /* AUTODOC: save physical port to do TTI MPLS lookup */
    CHECK_RC_MAC(
        cpssDxChTtiPortLookupEnableGet(swDevNum,phyPortArr[1],
                                       CPSS_DXCH_TTI_KEY_MPLS_E,
                                       &prvCstRestoreCfg.portLookupEnable[1]));

    /* AUTODOC: enable physical port to do TTI MPLS lookup */
    CHECK_RC_MAC(
        cpssDxChTtiPortLookupEnableSet(swDevNum,phyPortArr[1],
                                       CPSS_DXCH_TTI_KEY_MPLS_E,
                                       GT_TRUE));

    return GT_OK;
}
/**
* @internal prvCstSetMpls3LTunnelTermination function
* @endinternal
*
* @brief   Set ingress 3 Labels tunnel termination configuration
*
* @param[in] phyPortArr[4]            - physical ports
* @param[in] ePortArr[4]              - eports
*                                       None
*/
static GT_STATUS prvCstSetMpls3LTunnelTermination
(
    IN  GT_PHYSICAL_PORT_NUM  phyPortArr[4],/* physical port associated with ePort_A..D */
    IN  GT_PORT_NUM           ePortArr[4]   /* ePort_A..D */
)
{
    CPSS_DXCH_TTI_RULE_UNT          ttiPattern;
    CPSS_DXCH_TTI_MPLS_RULE_STC    *ttiMplsPatternPtr;
    CPSS_DXCH_TTI_RULE_UNT          ttiMask;
    CPSS_DXCH_TTI_MPLS_RULE_STC    *ttiMplsMaskPtr;
    CPSS_DXCH_TTI_ACTION_UNT        ttiAction;
    CPSS_DXCH_TTI_ACTION_2_STC     *ttiAction2Ptr;

    cpssOsMemSet(&ttiPattern,0,sizeof(ttiPattern));
    cpssOsMemSet(&ttiMask   ,0,sizeof(ttiMask));
    cpssOsMemSet(&ttiAction ,0,sizeof(ttiAction));

    /* AUTODOC: set 3 Labels tunnel termination */
    /* AUTODOC: use MPLS rule on 'ingress port' + 3 MPLS labels */

    ttiMplsPatternPtr = &ttiPattern.mpls;
    ttiMplsMaskPtr = &ttiMask.mpls;
    ttiAction2Ptr = &ttiAction.type2;

    /* pattern : port (not trunk) */
    ttiMplsPatternPtr->common.srcIsTrunk   = 0;
    ttiMplsPatternPtr->numOfLabels         = 2;/* 2 means 3 labels !!! */

    /* mask : port (not trunk) */
    ttiMplsMaskPtr->common.srcIsTrunk   = BIT_1-1;/*1bit*/
    ttiMplsMaskPtr->numOfLabels         = BIT_2-1;/*2bits*/

    /* pattern : specific port , label1,2,3 mac da*/
    ttiMplsPatternPtr->common.srcPortTrunk = phyPortArr[0];
    ttiMplsPatternPtr->label0       = mplsTtLabelArr[0];
    ttiMplsPatternPtr->label1       = mplsTtLabelArr[1];
    ttiMplsPatternPtr->label2       = mplsTtLabelArr[2];

    /* mask : specific port , label1,2,3 mac da*/
    ttiMplsMaskPtr->common.srcPortTrunk = BIT_8-1;/*8 bits*/
    ttiMplsMaskPtr->label0              = BIT_20-1;/*20 bits*/
    ttiMplsMaskPtr->label1              = BIT_20-1;/*20 bits*/
    ttiMplsMaskPtr->label2              = BIT_20-1;/*20 bits*/

    cpssOsMemSet(&ttiMplsMaskPtr->common.mac, 0xFF, sizeof(ttiMplsMaskPtr->common.mac));

    /* AUTODOC: define the action for ePort B*/

    /*action*/
    ttiAction2Ptr->tunnelTerminate = GT_FALSE;
    ttiAction2Ptr->tsPassengerPacketType = CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E;
    ttiAction2Ptr->mplsCommand = CPSS_DXCH_TTI_MPLS_POP2_CMD_E;
    ttiAction2Ptr->mplsTtl = timeToLive;
    ttiAction2Ptr->enableDecrementTtl = GT_FALSE;
    ttiAction2Ptr->command = CPSS_PACKET_CMD_FORWARD_E;
    ttiAction2Ptr->redirectCommand = CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E;
    ttiAction2Ptr->egressInterface.type = CPSS_INTERFACE_PORT_E;
    ttiAction2Ptr->egressInterface.devPort.portNum = ePortArr[1];
    ttiAction2Ptr->egressInterface.devPort.hwDevNum = hwDevNum;
    ttiAction2Ptr->tag1VlanCmd = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;
    ttiAction2Ptr->bridgeBypass = GT_TRUE;
    ttiAction2Ptr->continueToNextTtiLookup = GT_FALSE;
    ttiAction2Ptr->tunnelStart = GT_FALSE;
    ttiAction2Ptr->modifyMacSa = GT_TRUE;
    ttiAction2Ptr->modifyMacDa = GT_TRUE;
    ttiAction2Ptr->arpPtr = arpIndex;

    ttiMplsPatternPtr->common.mac   = ttMacDaArr[0];

    /* AUTODOC: set the TTI MPLS rule+action */
    CHECK_RC_MAC(
        cpssDxChTtiRuleSet(swDevNum,
                           tunnelTerminationIndexPtr[0],/* for ePort B*/
                           CPSS_DXCH_TTI_RULE_MPLS_E/* CPSS_TUNNEL_X_OVER_MPLS_E*/,
                           &ttiPattern,&ttiMask,
                           CPSS_DXCH_TTI_ACTION_TYPE2_ENT,&ttiAction));


    /* AUTODOC: define the action for ePort C*/

    ttiMplsPatternPtr->common.mac   = ttMacDaArr[1];
    ttiAction2Ptr->egressInterface.devPort.portNum = ePortArr[2];
    ttiAction2Ptr->mplsCommand = CPSS_DXCH_TTI_MPLS_POP1_CMD_E;
    ttiAction2Ptr->bridgeBypass = GT_TRUE;
    ttiAction2Ptr->tunnelStart = GT_FALSE;
    ttiAction2Ptr->continueToNextTtiLookup = GT_TRUE;

    /* AUTODOC: set the TTI MPLS rule+action */
    CHECK_RC_MAC(
        cpssDxChTtiRuleSet(swDevNum,
                           tunnelTerminationIndexPtr[1],/* for ePort C lookup 0*/
                           CPSS_DXCH_TTI_RULE_MPLS_E/* CPSS_TUNNEL_X_OVER_MPLS_E*/,
                           &ttiPattern,&ttiMask,
                           CPSS_DXCH_TTI_ACTION_TYPE2_ENT,&ttiAction));

    ttiMplsPatternPtr->common.mac   = ttMacDaArr[1];
    ttiAction2Ptr->egressInterface.devPort.portNum = ePortArr[2];
    ttiAction2Ptr->mplsCommand = CPSS_DXCH_TTI_MPLS_POP1_CMD_E;
    ttiAction2Ptr->bridgeBypass = GT_TRUE;
    ttiAction2Ptr->tunnelStart = GT_FALSE;

    /* AUTODOC: set the TTI MPLS rule+action */
    CHECK_RC_MAC(
        cpssDxChTtiRuleSet(swDevNum,
                           tunnelTerminationIndexPtr[2],/* for ePort C lookup 1*/
                           CPSS_DXCH_TTI_RULE_MPLS_E/* CPSS_TUNNEL_X_OVER_MPLS_E*/,
                           &ttiPattern,&ttiMask,
                           CPSS_DXCH_TTI_ACTION_TYPE2_ENT,&ttiAction));

    /* AUTODOC: define the action for ePort D*/

    ttiMplsPatternPtr->common.mac   = ttMacDaArr[2];
    ttiAction2Ptr->egressInterface.devPort.portNum = ePortArr[3];
    ttiAction2Ptr->mplsCommand = CPSS_DXCH_TTI_MPLS_POP2_CMD_E;
    ttiAction2Ptr->bridgeBypass = GT_TRUE;
    ttiAction2Ptr->tunnelStart = GT_FALSE;
    ttiAction2Ptr->continueToNextTtiLookup = GT_TRUE;

    /* AUTODOC: set the TTI MPLS rule+action */
    CHECK_RC_MAC(
        cpssDxChTtiRuleSet(swDevNum,
                           tunnelTerminationIndexPtr[4],/* for ePort D lookup 0*/
                           CPSS_DXCH_TTI_RULE_MPLS_E/* CPSS_TUNNEL_X_OVER_MPLS_E*/,
                           &ttiPattern,&ttiMask,
                           CPSS_DXCH_TTI_ACTION_TYPE2_ENT,&ttiAction));

    ttiMplsPatternPtr->common.mac   = ttMacDaArr[2];
    ttiAction2Ptr->egressInterface.devPort.portNum = ePortArr[3];
    ttiAction2Ptr->mplsCommand = CPSS_DXCH_TTI_MPLS_PUSH1_CMD_E;
    ttiAction2Ptr->bridgeBypass = GT_TRUE;
    ttiAction2Ptr->tunnelStart = GT_TRUE;
    ttiAction2Ptr->tunnelStartPtr = tunnelStartPointerArr[3];
    ttiAction2Ptr->continueToNextTtiLookup = GT_FALSE;
    ttiAction2Ptr->modifyMacDa = GT_FALSE;

    /* AUTODOC: set the TTI MPLS rule+action */
    CHECK_RC_MAC(
        cpssDxChTtiRuleSet(swDevNum,
                           tunnelTerminationIndexPtr[5],/* for ePort D lookup 1*/
                           CPSS_DXCH_TTI_RULE_MPLS_E/* CPSS_TUNNEL_X_OVER_MPLS_E*/,
                           &ttiPattern,&ttiMask,
                           CPSS_DXCH_TTI_ACTION_TYPE2_ENT,&ttiAction));

    /* definitions for second test */

    /* AUTODOC: define the action for ePort C */

    ttiMplsPatternPtr->common.srcPortTrunk = phyPortArr[1];
    ttiMplsPatternPtr->common.mac   = ttMacDaArr[3];
    ttiAction2Ptr->mplsCommand = CPSS_DXCH_TTI_MPLS_POP1_CMD_E;
    ttiAction2Ptr->egressInterface.devPort.portNum = ePortArr[2];
    ttiAction2Ptr->continueToNextTtiLookup = GT_FALSE;
    ttiAction2Ptr->tunnelStart = GT_FALSE;
    ttiAction2Ptr->modifyMacSa = GT_TRUE;
    ttiAction2Ptr->modifyMacDa = GT_TRUE;
    ttiAction2Ptr->arpPtr = arpIndex;

    /* AUTODOC: set the TTI MPLS rule+action */
    CHECK_RC_MAC(
        cpssDxChTtiRuleSet(swDevNum,
                           tunnelTerminationIndexPtr[6],/* for ePort B*/
                           CPSS_DXCH_TTI_RULE_MPLS_E/* CPSS_TUNNEL_X_OVER_MPLS_E*/,
                           &ttiPattern,&ttiMask,
                           CPSS_DXCH_TTI_ACTION_TYPE2_ENT,&ttiAction));


    /* AUTODOC: define the action for ePort D */

    ttiMplsPatternPtr->common.srcPortTrunk = phyPortArr[1];
    ttiMplsPatternPtr->common.mac   = ttMacDaArr[4];
    ttiAction2Ptr->egressInterface.devPort.portNum = ePortArr[3];
    ttiAction2Ptr->mplsCommand = CPSS_DXCH_TTI_MPLS_POP2_CMD_E;
    ttiAction2Ptr->bridgeBypass = GT_TRUE;
    ttiAction2Ptr->tunnelStart = GT_FALSE;
    ttiAction2Ptr->continueToNextTtiLookup = GT_TRUE;

    /* AUTODOC: set the TTI MPLS rule+action */
    CHECK_RC_MAC(
        cpssDxChTtiRuleSet(swDevNum,
                           tunnelTerminationIndexPtr[7],/* for ePort D lookup 0*/
                           CPSS_DXCH_TTI_RULE_MPLS_E/* CPSS_TUNNEL_X_OVER_MPLS_E*/,
                           &ttiPattern,&ttiMask,
                           CPSS_DXCH_TTI_ACTION_TYPE2_ENT,&ttiAction));

    ttiMplsPatternPtr->common.srcPortTrunk = phyPortArr[1];
    ttiMplsPatternPtr->common.mac   = ttMacDaArr[4];
    ttiAction2Ptr->egressInterface.devPort.portNum = ePortArr[3];
    ttiAction2Ptr->mplsCommand = CPSS_DXCH_TTI_MPLS_PUSH1_CMD_E;
    ttiAction2Ptr->bridgeBypass = GT_TRUE;
    ttiAction2Ptr->tunnelStart = GT_TRUE;
    ttiAction2Ptr->tunnelStartPtr = tunnelStartPointerArr[3];
    ttiAction2Ptr->continueToNextTtiLookup = GT_FALSE;
    ttiAction2Ptr->modifyMacDa = GT_FALSE;

    /* AUTODOC: set the TTI MPLS rule+action */
    CHECK_RC_MAC(
        cpssDxChTtiRuleSet(swDevNum,
                           tunnelTerminationIndexPtr[8],/* for ePort D lookup 1*/
                           CPSS_DXCH_TTI_RULE_MPLS_E/* CPSS_TUNNEL_X_OVER_MPLS_E*/,
                           &ttiPattern,&ttiMask,
                           CPSS_DXCH_TTI_ACTION_TYPE2_ENT,&ttiAction));

    /* AUTODOC: define the action for ePort A*/

    ttiMplsPatternPtr->common.srcPortTrunk = phyPortArr[1];
    ttiMplsPatternPtr->common.mac   = ttMacDaArr[5];
    ttiAction2Ptr->egressInterface.devPort.portNum = ePortArr[0];
    ttiAction2Ptr->mplsCommand = CPSS_DXCH_TTI_MPLS_POP_AND_SWAP_CMD_E;
    ttiAction2Ptr->bridgeBypass = GT_TRUE;
    ttiAction2Ptr->tunnelStart = GT_TRUE;
    ttiAction2Ptr->tunnelStartPtr = tunnelStartPointerArr[0];
    ttiAction2Ptr->continueToNextTtiLookup = GT_TRUE;
    ttiAction2Ptr->modifyMacDa = GT_FALSE;

    /* AUTODOC: set the TTI MPLS rule+action */
    CHECK_RC_MAC(
        cpssDxChTtiRuleSet(swDevNum,
                           tunnelTerminationIndexPtr[9],/* for ePort A lookup 0*/
                           CPSS_DXCH_TTI_RULE_MPLS_E/* CPSS_TUNNEL_X_OVER_MPLS_E*/,
                           &ttiPattern,&ttiMask,
                           CPSS_DXCH_TTI_ACTION_TYPE2_ENT,&ttiAction));

    ttiMplsPatternPtr->common.srcPortTrunk = phyPortArr[1];
    ttiMplsPatternPtr->common.mac   = ttMacDaArr[5];
    ttiAction2Ptr->egressInterface.devPort.portNum = ePortArr[0];
    ttiAction2Ptr->mplsCommand = CPSS_DXCH_TTI_MPLS_POP1_CMD_E;
    ttiAction2Ptr->bridgeBypass = GT_TRUE;
    ttiAction2Ptr->tunnelStart = GT_TRUE;
    ttiAction2Ptr->tunnelStartPtr = tunnelStartPointerArr[0];
    ttiAction2Ptr->continueToNextTtiLookup = GT_FALSE;
    ttiAction2Ptr->modifyMacDa = GT_FALSE;


    /* AUTODOC: set the TTI MPLS rule+action */
    CHECK_RC_MAC(
        cpssDxChTtiRuleSet(swDevNum,
                           tunnelTerminationIndexPtr[10],/* for ePort C lookup 1*/
                           CPSS_DXCH_TTI_RULE_MPLS_E/* CPSS_TUNNEL_X_OVER_MPLS_E*/,
                           &ttiPattern,&ttiMask,
                           CPSS_DXCH_TTI_ACTION_TYPE2_ENT,&ttiAction));


    /* AUTODOC: save physical port to do TTI MPLS lookup */
    CHECK_RC_MAC(
        cpssDxChTtiPortLookupEnableGet(swDevNum,phyPortArr[0],
                                       CPSS_DXCH_TTI_KEY_MPLS_E,
                                       &prvCstRestoreCfg.portLookupEnable[0]));

    /* AUTODOC: enable physical port to do TTI MPLS lookup */
    CHECK_RC_MAC(
        cpssDxChTtiPortLookupEnableSet(swDevNum,phyPortArr[0],
                                       CPSS_DXCH_TTI_KEY_MPLS_E,
                                       GT_TRUE));

    /* AUTODOC: save the lookup Mac mode */
    CHECK_RC_MAC(
        cpssDxChTtiMacModeGet(swDevNum,CPSS_DXCH_TTI_RULE_MPLS_E,
                              &prvCstRestoreCfg.macMode));

    /* AUTODOC: sets the lookup Mac mode */
    CHECK_RC_MAC(
        cpssDxChTtiMacModeSet(swDevNum,CPSS_DXCH_TTI_RULE_MPLS_E,
                              CPSS_DXCH_TTI_MAC_MODE_DA_E));

    /* AUTODOC: save physical port to do TTI MPLS lookup */
    CHECK_RC_MAC(
        cpssDxChTtiPortLookupEnableGet(swDevNum,phyPortArr[1],
                                       CPSS_DXCH_TTI_KEY_MPLS_E,
                                       &prvCstRestoreCfg.portLookupEnable[1]));

    /* AUTODOC: enable physical port to do TTI MPLS lookup */
    CHECK_RC_MAC(
        cpssDxChTtiPortLookupEnableSet(swDevNum,phyPortArr[1],
                                       CPSS_DXCH_TTI_KEY_MPLS_E,
                                       GT_TRUE));

    /* AUTODOC: write a ARP MAC address to the Tunnel start Table */
    CHECK_RC_MAC(
        cpssDxChIpRouterArpAddrWrite(swDevNum, arpIndex, &tsMacDaArr[3]));

    return GT_OK;
}

/**
* @internal prvCstSetMpls4LTunnelTermination function
* @endinternal
*
* @brief   Set ingress 4 Labels tunnel termination configuration
*
* @param[in] phyPortArr[4]            - physical ports
* @param[in] ePortArr[4]              - eports
*                                       None
*/
static GT_STATUS prvCstSetMpls4LTunnelTermination
(
    IN  GT_PHYSICAL_PORT_NUM  phyPortArr[4],/* physical port associated with ePort_A..D */
    IN  GT_PORT_NUM           ePortArr[4]   /* ePort_A..D */
)
{
    CPSS_DXCH_TTI_RULE_UNT          ttiPattern;
    CPSS_DXCH_TTI_MPLS_RULE_STC    *ttiMplsPatternPtr;
    CPSS_DXCH_TTI_RULE_UNT          ttiMask;
    CPSS_DXCH_TTI_MPLS_RULE_STC    *ttiMplsMaskPtr;
    CPSS_DXCH_TTI_ACTION_UNT        ttiAction;
    CPSS_DXCH_TTI_ACTION_2_STC     *ttiAction2Ptr;

    cpssOsMemSet(&ttiPattern,0,sizeof(ttiPattern));
    cpssOsMemSet(&ttiMask   ,0,sizeof(ttiMask));
    cpssOsMemSet(&ttiAction ,0,sizeof(ttiAction));

    /* AUTODOC: set 4 Labels tunnel termination */
    /* AUTODOC: use MPLS rule on 'ingress port' + 4 MPLS labels */

    ttiMplsPatternPtr = &ttiPattern.mpls;
    ttiMplsMaskPtr = &ttiMask.mpls;
    ttiAction2Ptr = &ttiAction.type2;

    /* pattern : port (not trunk) */
    ttiMplsPatternPtr->common.srcIsTrunk   = 0;

    /* mask : port (not trunk) */
    ttiMplsMaskPtr->common.srcIsTrunk   = BIT_1-1;/*1bit*/

    /* pattern : specific port , label1,2,3 mac da*/
    ttiMplsPatternPtr->common.srcPortTrunk = phyPortArr[0];
    ttiMplsPatternPtr->label0       = mplsTtLabelArr[0];
    ttiMplsPatternPtr->label1       = mplsTtLabelArr[1];
    ttiMplsPatternPtr->label2       = mplsTtLabelArr[2];

    /* mask : specific port , label1,2,3 mac da*/
    ttiMplsMaskPtr->common.srcPortTrunk = BIT_8-1;/*8 bits*/
    ttiMplsMaskPtr->label0              = BIT_20-1;/*20 bits*/
    ttiMplsMaskPtr->label1              = BIT_20-1;/*20 bits*/
    ttiMplsMaskPtr->label2              = BIT_20-1;/*20 bits*/

    cpssOsMemSet(&ttiMplsMaskPtr->common.mac, 0xFF, sizeof(ttiMplsMaskPtr->common.mac));

    /* AUTODOC: define the action for ePort B*/

    /*action*/
    ttiAction2Ptr->tunnelTerminate = GT_FALSE;
    ttiAction2Ptr->tsPassengerPacketType = CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E;
    ttiAction2Ptr->mplsCommand = CPSS_DXCH_TTI_MPLS_POP3_CMD_E;
    ttiAction2Ptr->mplsTtl = timeToLive;
    ttiAction2Ptr->enableDecrementTtl = GT_FALSE;
    ttiAction2Ptr->command = CPSS_PACKET_CMD_FORWARD_E;
    ttiAction2Ptr->redirectCommand = CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E;
    ttiAction2Ptr->egressInterface.type = CPSS_INTERFACE_PORT_E;
    ttiAction2Ptr->egressInterface.devPort.portNum = ePortArr[1];
    ttiAction2Ptr->egressInterface.devPort.hwDevNum = hwDevNum;
    ttiAction2Ptr->tag1VlanCmd = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;
    ttiAction2Ptr->bridgeBypass = GT_TRUE;
    ttiAction2Ptr->continueToNextTtiLookup = GT_FALSE;
    ttiAction2Ptr->tunnelStart = GT_FALSE;
    ttiAction2Ptr->modifyMacSa = GT_TRUE;
    ttiAction2Ptr->modifyMacDa = GT_TRUE;
    ttiAction2Ptr->arpPtr = arpIndex;

    ttiMplsPatternPtr->common.mac   = ttMacDaArr[0];

    /* AUTODOC: set the TTI MPLS rule+action */
    CHECK_RC_MAC(
        cpssDxChTtiRuleSet(swDevNum,
                           tunnelTerminationIndexPtr[0],/* for ePort B*/
                           CPSS_DXCH_TTI_RULE_MPLS_E/* CPSS_TUNNEL_X_OVER_MPLS_E*/,
                           &ttiPattern,&ttiMask,
                           CPSS_DXCH_TTI_ACTION_TYPE2_ENT,&ttiAction));


    /* AUTODOC: define the action for ePort C*/

    ttiMplsPatternPtr->common.mac   = ttMacDaArr[1];
    ttiAction2Ptr->egressInterface.devPort.portNum = ePortArr[2];
    ttiAction2Ptr->mplsCommand = CPSS_DXCH_TTI_MPLS_POP2_CMD_E;
    ttiAction2Ptr->bridgeBypass = GT_TRUE;
    ttiAction2Ptr->tunnelStart = GT_FALSE;
    ttiAction2Ptr->continueToNextTtiLookup = GT_TRUE;

    /* AUTODOC: set the TTI MPLS rule+action */
    CHECK_RC_MAC(
        cpssDxChTtiRuleSet(swDevNum,
                           tunnelTerminationIndexPtr[1],/* for ePort C lookup 0*/
                           CPSS_DXCH_TTI_RULE_MPLS_E/* CPSS_TUNNEL_X_OVER_MPLS_E*/,
                           &ttiPattern,&ttiMask,
                           CPSS_DXCH_TTI_ACTION_TYPE2_ENT,&ttiAction));

    ttiMplsPatternPtr->common.mac   = ttMacDaArr[1];
    ttiAction2Ptr->egressInterface.devPort.portNum = ePortArr[2];
    ttiAction2Ptr->mplsCommand = CPSS_DXCH_TTI_MPLS_POP1_CMD_E;
    ttiAction2Ptr->bridgeBypass = GT_TRUE;
    ttiAction2Ptr->tunnelStart = GT_FALSE;

    /* AUTODOC: set the TTI MPLS rule+action */
    CHECK_RC_MAC(
        cpssDxChTtiRuleSet(swDevNum,
                           tunnelTerminationIndexPtr[2],/* for ePort C lookup 1*/
                           CPSS_DXCH_TTI_RULE_MPLS_E/* CPSS_TUNNEL_X_OVER_MPLS_E*/,
                           &ttiPattern,&ttiMask,
                           CPSS_DXCH_TTI_ACTION_TYPE2_ENT,&ttiAction));

    /* AUTODOC: define the action for ePort D*/

    ttiMplsPatternPtr->common.mac   = ttMacDaArr[2];
    ttiAction2Ptr->egressInterface.devPort.portNum = ePortArr[3];
    ttiAction2Ptr->mplsCommand = CPSS_DXCH_TTI_MPLS_POP3_CMD_E;
    ttiAction2Ptr->bridgeBypass = GT_TRUE;
    ttiAction2Ptr->tunnelStart = GT_FALSE;
    ttiAction2Ptr->continueToNextTtiLookup = GT_TRUE;
    ttiAction2Ptr->modifyMacDa = GT_FALSE;

    /* AUTODOC: set the TTI MPLS rule+action */
    CHECK_RC_MAC(
        cpssDxChTtiRuleSet(swDevNum,
                           tunnelTerminationIndexPtr[4],/* for ePort D lookup 0*/
                           CPSS_DXCH_TTI_RULE_MPLS_E/* CPSS_TUNNEL_X_OVER_MPLS_E*/,
                           &ttiPattern,&ttiMask,
                           CPSS_DXCH_TTI_ACTION_TYPE2_ENT,&ttiAction));

    ttiMplsPatternPtr->common.mac   = ttMacDaArr[2];
    ttiAction2Ptr->egressInterface.devPort.portNum = ePortArr[3];
    ttiAction2Ptr->mplsCommand = CPSS_DXCH_TTI_MPLS_PUSH1_CMD_E;
    ttiAction2Ptr->bridgeBypass = GT_TRUE;
    ttiAction2Ptr->tunnelStart = GT_TRUE;
    ttiAction2Ptr->tunnelStartPtr = tunnelStartPointerArr[3];
    ttiAction2Ptr->continueToNextTtiLookup = GT_FALSE;
    ttiAction2Ptr->modifyMacDa = GT_FALSE;

    /* AUTODOC: set the TTI MPLS rule+action */
    CHECK_RC_MAC(
        cpssDxChTtiRuleSet(swDevNum,
                           tunnelTerminationIndexPtr[5],/* for ePort C lookup 1*/
                           CPSS_DXCH_TTI_RULE_MPLS_E/* CPSS_TUNNEL_X_OVER_MPLS_E*/,
                           &ttiPattern,&ttiMask,
                           CPSS_DXCH_TTI_ACTION_TYPE2_ENT,&ttiAction));



    /* AUTODOC: save physical port to do TTI MPLS lookup */
    CHECK_RC_MAC(
        cpssDxChTtiPortLookupEnableGet(swDevNum,phyPortArr[0],
                                       CPSS_DXCH_TTI_KEY_MPLS_E,
                                       &prvCstRestoreCfg.portLookupEnable[0]));

    /* AUTODOC: enable physical port to do TTI MPLS lookup */
    CHECK_RC_MAC(
        cpssDxChTtiPortLookupEnableSet(swDevNum,phyPortArr[0],
                                       CPSS_DXCH_TTI_KEY_MPLS_E,
                                       GT_TRUE));

    /* AUTODOC: save the lookup Mac mode */
    CHECK_RC_MAC(
        cpssDxChTtiMacModeGet(swDevNum,CPSS_DXCH_TTI_RULE_MPLS_E,
                              &prvCstRestoreCfg.macMode));

    /* AUTODOC: sets the lookup Mac mode */
    CHECK_RC_MAC(
        cpssDxChTtiMacModeSet(swDevNum,CPSS_DXCH_TTI_RULE_MPLS_E,
                              CPSS_DXCH_TTI_MAC_MODE_DA_E));

    /* AUTODOC: write a ARP MAC address to the Tunnel start Table */
    CHECK_RC_MAC(
        cpssDxChIpRouterArpAddrWrite(swDevNum, arpIndex, &tsMacDaArr[3]));

    return GT_OK;
}

/**
* @internal setCapture function
* @endinternal
*
*/
static GT_VOID setCapture
(
    IN GT_U32    portIndex,
    IN GT_BOOL   enable
)
{
    GT_STATUS   rc;
    CPSS_INTERFACE_INFO_STC portInterface;

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[portIndex];

    if(enable == GT_TRUE)
    {
        /* clear table */
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
    }

    /* Start capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, enable);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum, enable);
}

/**
* @internal sendPacket function
* @endinternal
*
*/
static GT_VOID sendPacket
(
    GT_VOID
)
{
    GT_STATUS   rc;

    TGF_PACKET_STC            packetInfo;
    TGF_PACKET_L2_STC        *l2PartPtr;
    TGF_PACKET_VLAN_TAG_STC  *vlanTagPartPtr;
    TGF_PACKET_ETHERTYPE_STC *etherTypePartPtr;

    /* set number vlan tags in packet */
    rc = prvTgfPacketNumberOfVlanTagsSet(GT_FALSE, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketNumberOfVlanTagsSet");

    /* AUTODOC: get default Ethernet packet */
    rc = prvTgfPacketEthernetPacketDefaultPacketGet(&packetInfo.numOfParts, &packetInfo.partsArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpv4PacketDefaultPacketGet");

    prvTgfEtherPacketInfo.numOfParts = packetInfo.numOfParts;
    prvTgfEtherPacketInfo.totalLen = TGF_PACKET_AUTO_CALC_LENGTH_CNS;
    prvTgfEtherPacketInfo.partsArray =  packetInfo.partsArray;

    rc = prvTgfPacketHeaderPartGet(TGF_PACKET_PART_L2_E, &prvTgfEtherPacketInfo, (GT_VOID *)&l2PartPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketHeaderPartGet");

    /* AUTODOC: save L2 part of packet */
    cpssOsMemCpy(&ethernetPacketL2,
                 l2PartPtr,
                 sizeof(ethernetPacketL2));

    rc = prvTgfPacketHeaderPartGet(TGF_PACKET_PART_VLAN_TAG_E, &prvTgfEtherPacketInfo, (GT_VOID *)&vlanTagPartPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketHeaderPartGet");

    /* AUTODOC: save VLAN part of packet */
    cpssOsMemCpy(&ethernetPacketVlan,
                 vlanTagPartPtr,
                 sizeof(ethernetPacketVlan));

    rc = prvTgfPacketHeaderPartGet(TGF_PACKET_PART_ETHERTYPE_E, &prvTgfEtherPacketInfo, (GT_VOID *)&etherTypePartPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketHeaderPartGet");

    /* AUTODOC: save EtherType part of packet */
    cpssOsMemCpy(&ethernetPacketEtherType,
                 etherTypePartPtr,
                 sizeof(ethernetPacketEtherType));

    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfEtherPacketInfo,
        prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* start transmitting */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

}

/**
* @internal sendTunneledPacket function
* @endinternal
*
*/
static GT_VOID sendTunneledPacket
(
    GT_U32 senderPortIndex,
    GT_U32 packetNumber
)
{
    GT_STATUS   rc;

    TGF_PACKET_STC            packetInfo;
    TGF_PACKET_L2_STC         *l2PartPtr = {0};
    TGF_PACKET_VLAN_TAG_STC   vlanTagPart = {0x8100, 0, 0, 0};
    TGF_PACKET_MPLS_STC       label0Part,label1Part,label2Part,label3Part = {0};

    /* AUTODOC: Sets number of vlan tags in passenger header */
    rc = prvTgfPacketNumberOfVlanTagsSet(GT_TRUE/*passenger*/,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketNumberOfVlanTagsSet");

    /* AUTODOC: Sets number of vlan tags in tunnel header */
    rc = prvTgfPacketNumberOfVlanTagsSet(GT_FALSE/*tunnel*/,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketNumberOfVlanTagsSet");

    /* AUTODOC: Sets number of labels in tunnel header */
    rc = prvTgfPacketNumberOfMplsLabelsSet(numOfLabels);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketNumberOfMplsLabelsSet");

    /* AUTODOC: get default Ethernet over Mpls packet */
    rc = prvTgfPacketEthernetOverMplsPacketDefaultPacketGet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS, &packetInfo.numOfParts, &packetInfo.partsArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthernetOverMplsPacketDefaultPacketGet");

    packetInfo.totalLen = TGF_PACKET_AUTO_CALC_LENGTH_CNS;

    prvTgfEtherOverMplsPacketInfo.numOfParts = packetInfo.numOfParts;
    prvTgfEtherOverMplsPacketInfo.totalLen = TGF_PACKET_AUTO_CALC_LENGTH_CNS;
    prvTgfEtherOverMplsPacketInfo.partsArray =  packetInfo.partsArray;

    vlanTagPart.vid = PRV_CST_MPLS_VLANID_CNS;

    /* AUTODOC: change VLAN part of tunnel */
    rc = prvTgfPacketEthOverMplsPacketHeaderDataSet(TGF_PACKET_PART_VLAN_TAG_E,GT_FALSE,0,&vlanTagPart);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketEthOverMplsPacketHeaderDataSet");

    switch (numOfLabels)
    {
    case 2:
        /* AUTODOC: change Label part of tunnel */
        label0Part.stack = 0;
        label0Part.label = mplsTtLabelArr[0];
        label0Part.exp = expTs;
        label0Part.timeToLive = timeToLive;

        rc = prvTgfPacketEthOverMplsPacketHeaderDataSet(TGF_PACKET_PART_MPLS_E,GT_FALSE,0,&label0Part);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketEthOverMplsPacketHeaderDataSet");

        label1Part.stack = 1;
        label1Part.label = mplsTtLabelArr[1];
        label1Part.exp = expTs;
        label1Part.timeToLive = timeToLive;

        rc = prvTgfPacketEthOverMplsPacketHeaderDataSet(TGF_PACKET_PART_MPLS_E,GT_FALSE,1,&label1Part);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketEthOverMplsPacketHeaderDataSet");
        break;
    case 3:
        /* AUTODOC: change Label part of tunnel */
        label0Part.stack = 0;
        label0Part.label = mplsTtLabelArr[0];
        label0Part.exp = expTs;
        label0Part.timeToLive = timeToLive;

        rc = prvTgfPacketEthOverMplsPacketHeaderDataSet(TGF_PACKET_PART_MPLS_E,GT_FALSE,0,&label0Part);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketEthOverMplsPacketHeaderDataSet");

        label1Part.stack = 0;
        label1Part.label = mplsTtLabelArr[1];
        label1Part.exp = expTs;
        label1Part.timeToLive = timeToLive;

         rc = prvTgfPacketEthOverMplsPacketHeaderDataSet(TGF_PACKET_PART_MPLS_E,GT_FALSE,1,&label1Part);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketEthOverMplsPacketHeaderDataSet");

        label2Part.stack = 1;
        label2Part.label = mplsTtLabelArr[2];
        label2Part.exp = expTs;
        label2Part.timeToLive = timeToLive;

         rc = prvTgfPacketEthOverMplsPacketHeaderDataSet(TGF_PACKET_PART_MPLS_E,GT_FALSE,2,&label2Part);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketEthOverMplsPacketHeaderDataSet");
        break;
    case 4:
        /* AUTODOC: change Label part of tunnel */
        label0Part.stack = 0;
        label0Part.label = mplsTtLabelArr[0];
        label0Part.exp = expTs;
        label0Part.timeToLive = timeToLive;

        rc = prvTgfPacketEthOverMplsPacketHeaderDataSet(TGF_PACKET_PART_MPLS_E,GT_FALSE,0,&label0Part);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketEthOverMplsPacketHeaderDataSet");

        label1Part.stack = 0;
        label1Part.label = mplsTtLabelArr[1];
        label1Part.exp = expTs;
        label1Part.timeToLive = timeToLive;

         rc = prvTgfPacketEthOverMplsPacketHeaderDataSet(TGF_PACKET_PART_MPLS_E,GT_FALSE,1,&label1Part);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketEthOverMplsPacketHeaderDataSet");

        label2Part.stack = 0;
        label2Part.label = mplsTtLabelArr[2];
        label2Part.exp = expTs;
        label2Part.timeToLive = timeToLive;

         rc = prvTgfPacketEthOverMplsPacketHeaderDataSet(TGF_PACKET_PART_MPLS_E,GT_FALSE,2,&label2Part);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketEthOverMplsPacketHeaderDataSet");

        label3Part.stack = 1;
        label3Part.label = mplsTtLabelArr[3];
        label3Part.exp = expTs;
        label3Part.timeToLive = timeToLive;

         rc = prvTgfPacketEthOverMplsPacketHeaderDataSet(TGF_PACKET_PART_MPLS_E,GT_FALSE,3,&label3Part);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketEthOverMplsPacketHeaderDataSet");
        break;
    }


    rc = prvTgfPacketHeaderPartGet(TGF_PACKET_PART_L2_E, &prvTgfEtherOverMplsPacketInfo, (GT_VOID *)&l2PartPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketHeaderPartGet");

    /* AUTODOC: change mac DA part of tunnel */
    cpssOsMemCpy(l2PartPtr->daMac,
                 ttMacDaArr[packetNumber-1].arEther,
                 6);

    /* AUTODOC: change L2 part of tunnel */
    rc = prvTgfPacketEthOverMplsPacketHeaderDataSet(TGF_PACKET_PART_L2_E,GT_FALSE,0,l2PartPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketEthOverMplsPacketHeaderDataSet");


    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfEtherOverMplsPacketInfo,
        prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[senderPortIndex]);

    /* start transmitting */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[senderPortIndex]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[senderPortIndex]);

}

/**
* @internal checkCapturedPacket function
* @endinternal
*
*/
static GT_VOID checkCapturedPacket
(
    IN GT_U32    egressPortIndex
)
{
    GT_STATUS   rc;
    CPSS_INTERFACE_INFO_STC portInterface;
    GT_U32  actualCapturedNumOfPackets = 0;
    GT_U32  numOfExpectedPackets = prvTgfBurstCount;
    TGF_PACKET_STC          packetInfo;
    TGF_PACKET_L2_STC       l2Part;
    TGF_PACKET_VLAN_TAG_STC vlanTagPart = {0x8100, 0, 0, 0};
    TGF_PACKET_MPLS_STC     labelPart,labelPart1,labelPart2 = {0};
    TGF_PACKET_MPLS_STC     pwLabelPart,flowLabelPart = {0};
    TGF_PACKET_WILDCARD_STC cwLabelPart = {0};
    GT_U8   cwLabel[] = {0x00,0x11,0x22,0x33};

    /* AUTODOC: Sets number of vlan tags in passenger header */
    rc = prvTgfPacketNumberOfVlanTagsSet(GT_TRUE/*passenger*/,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketNumberOfVlanTagsSet");

    /* AUTODOC: Sets number of vlan tags in tunnel header */
    rc = prvTgfPacketNumberOfVlanTagsSet(GT_FALSE/*tunnel*/,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketNumberOfVlanTagsSet");

    if (egressPortIndex == 3)
    {
        /* AUTODOC: Sets number of labels in tunnel header + PW + FLOW*/
        rc = prvTgfPacketNumberOfMplsLabelsSet(egressPortIndex + 2);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketNumberOfMplsLabelsSet");

        /* AUTODOC: Sets  CW label in tunnel header */
        prvTgfPacketIsWildcardSet(GT_TRUE);

    }
    else
    {
        /* AUTODOC: Sets number of labels in tunnel header */
        rc = prvTgfPacketNumberOfMplsLabelsSet(egressPortIndex);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketNumberOfMplsLabelsSet");
    }

    /* AUTODOC: get default Ethernet over Mpls packet */
    rc = prvTgfPacketEthernetOverMplsPacketDefaultPacketGet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS, &packetInfo.numOfParts, &packetInfo.partsArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthernetOverMplsPacketDefaultPacketGet");

    packetInfo.totalLen = TGF_PACKET_AUTO_CALC_LENGTH_CNS;

    /* AUTODOC: change L2 part of passenger */
    rc = prvTgfPacketEthOverMplsPacketHeaderDataSet(TGF_PACKET_PART_L2_E,GT_TRUE,0,&ethernetPacketL2);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketEthOverMplsPacketHeaderDataSet");

    /* expect mac DA of TS */
    cpssOsMemCpy(l2Part.daMac,
                 tsMacDaArr[egressPortIndex].arEther,
                 6);
    /* expect unique mac SA according to egress port */
    cpssOsMemCpy(l2Part.saMac,
                 tsMacSaArr[egressPortIndex].arEther,
                 6);

    /* AUTODOC: change L2 part of tunnel */
    rc = prvTgfPacketEthOverMplsPacketHeaderDataSet(TGF_PACKET_PART_L2_E,GT_FALSE,0,&l2Part);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketEthOverMplsPacketHeaderDataSet");

    /* AUTODOC: change VLAN part of passenger */
    rc = prvTgfPacketEthOverMplsPacketHeaderDataSet(TGF_PACKET_PART_VLAN_TAG_E,GT_TRUE,0,&ethernetPacketVlan);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketEthOverMplsPacketHeaderDataSet");

    vlanTagPart.vid = PRV_CST_MPLS_VLANID_CNS;

    /* AUTODOC: change VLAN part of tunnel */
    rc = prvTgfPacketEthOverMplsPacketHeaderDataSet(TGF_PACKET_PART_VLAN_TAG_E,GT_FALSE,0,&vlanTagPart);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketEthOverMplsPacketHeaderDataSet");

    /* AUTODOC: change EtherType part of passenger */
    rc = prvTgfPacketEthOverMplsPacketHeaderDataSet(TGF_PACKET_PART_ETHERTYPE_E,GT_TRUE,0,&ethernetPacketEtherType);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketEthOverMplsPacketHeaderDataSet");

    switch (egressPortIndex)
    {
    case 1:
        /* AUTODOC: change Label 1 of tunnel */
        labelPart.exp = expTs;
        labelPart.label = mplsLabelArr[1];
        labelPart.stack = 1;
        labelPart.timeToLive = timeToLive;

        rc = prvTgfPacketEthOverMplsPacketHeaderDataSet(TGF_PACKET_PART_MPLS_E,GT_FALSE,0,&labelPart);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketEthOverMplsPacketHeaderDataSet");

        break;
    case 2:

        /* AUTODOC: change Label 1 of tunnel */
        labelPart.exp = expTs;
        labelPart.label = mplsLabelArr[2];
        labelPart.stack = 0;
        labelPart.timeToLive = timeToLive;

        rc = prvTgfPacketEthOverMplsPacketHeaderDataSet(TGF_PACKET_PART_MPLS_E,GT_FALSE,0,&labelPart);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketEthOverMplsPacketHeaderDataSet");

        /* AUTODOC: change Label 2 of tunnel */
        labelPart1.exp = expTs;
        labelPart1.label = mplsLabelArr[1];
        labelPart1.stack = 1;
        labelPart1.timeToLive = timeToLive;

        rc = prvTgfPacketEthOverMplsPacketHeaderDataSet(TGF_PACKET_PART_MPLS_E,GT_FALSE,1,&labelPart1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketEthOverMplsPacketHeaderDataSet");

        break;


    case 3:

        /* AUTODOC: change Label 1 of tunnel */
        labelPart.exp = expTs;
        labelPart.label = mplsLabelArr[3];
        labelPart.stack = 0;
        labelPart.timeToLive = timeToLive;

        rc = prvTgfPacketEthOverMplsPacketHeaderDataSet(TGF_PACKET_PART_MPLS_E,GT_FALSE,0,&labelPart);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketEthOverMplsPacketHeaderDataSet");

        /* AUTODOC: change Label 2 of tunnel */
        labelPart1.exp = expTs;
        labelPart1.label = mplsLabelArr[2];
        labelPart1.stack = 0;
        labelPart1.timeToLive = timeToLive;

        rc = prvTgfPacketEthOverMplsPacketHeaderDataSet(TGF_PACKET_PART_MPLS_E,GT_FALSE,1,&labelPart1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketEthOverMplsPacketHeaderDataSet");


        /* AUTODOC: change Label 3 of tunnel */
        labelPart2.exp = expTs;
        labelPart2.label = mplsLabelArr[1];
        labelPart2.stack = 0;
        labelPart2.timeToLive = timeToLive;

        rc = prvTgfPacketEthOverMplsPacketHeaderDataSet(TGF_PACKET_PART_MPLS_E,GT_FALSE,2,&labelPart2);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketEthOverMplsPacketHeaderDataSet");

         /* AUTODOC: change PW Label of tunnel */
        pwLabelPart.exp = (GT_U8)pwExp[3];
        pwLabelPart.label = pwLabels[3];
        pwLabelPart.stack = 0;
        pwLabelPart.timeToLive = (GT_U8)pwTtl[3];

        rc = prvTgfPacketEthOverMplsPacketHeaderDataSet(TGF_PACKET_PART_MPLS_E,GT_FALSE,3,&pwLabelPart);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketEthOverMplsPacketHeaderDataSet");

        /* AUTODOC: change FLOW Label of tunnel */
        flowLabelPart.exp = 0;
        flowLabelPart.label = 0x13; /* hash calc */
        flowLabelPart.stack = 1;
        flowLabelPart.timeToLive = (GT_U8)flowTtl[3];

        rc = prvTgfPacketEthOverMplsPacketHeaderDataSet(TGF_PACKET_PART_MPLS_E,GT_FALSE,4,&flowLabelPart);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketEthOverMplsPacketHeaderDataSet");

        /* AUTODOC: change CW Label of tunnel */
        cwLabelPart.bytesPtr = cwLabel;
        cwLabelPart.numOfBytes = sizeof(cwLabel);
        rc = prvTgfPacketEthOverMplsPacketHeaderDataSet(TGF_PACKET_PART_WILDCARD_E,GT_FALSE,0,&cwLabelPart);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketEthOverMplsPacketHeaderDataSet");

        break;

    default:
        return;
    }



    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[egressPortIndex];

    /* reset the previous value */
    actualCapturedNumOfPackets = 0;

    PRV_UTF_LOG1_MAC("Port [%d] capturing on physical port:\n", prvTgfPortsArray[egressPortIndex]);

    /* print captured packets and check TriggerCounters */
    rc = tgfTrafficGeneratorPortTxEthCaptureCompare(
            &portInterface,
            &packetInfo,
            numOfExpectedPackets,/*numOfPackets*/
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

    /* AUTODOC: restore number of labels in tunnel header */
    rc = prvTgfPacketNumberOfMplsLabelsSet(1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketNumberOfMplsLabelsSet");

    /* AUTODOC: unset CW label in tunnel header */
    prvTgfPacketIsWildcardSet(GT_FALSE);

}

/**
* @internal check2LTunnelCapturedPacket function
* @endinternal
*
*/
static GT_VOID check2LTunnelCapturedPacket
(
    IN GT_U32    egressPortIndex,
    IN GT_U32    packetNumber
)
{
    GT_STATUS   rc;
    CPSS_INTERFACE_INFO_STC portInterface;
    GT_U32  actualCapturedNumOfPackets = 0;
    GT_U32  numOfExpectedPackets = prvTgfBurstCount;
    TGF_PACKET_STC          packetInfo;
    TGF_PACKET_VLAN_TAG_STC vlanTagPart = {0x8100, 0, 0, 0};
    TGF_PACKET_L2_STC       l2Part;
    TGF_PACKET_MPLS_STC     labelPart,labelPart1 = {0};
    GT_U32 ii=0;

    /* AUTODOC: Sets number of vlan tags in passenger header */
    rc = prvTgfPacketNumberOfVlanTagsSet(GT_TRUE/*passenger*/,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketNumberOfVlanTagsSet");

    /* AUTODOC: Sets number of vlan tags in tunnel header */
    rc = prvTgfPacketNumberOfVlanTagsSet(GT_FALSE/*tunnel*/,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketNumberOfVlanTagsSet");

    switch (packetNumber)
    {
    case 1:
        ii = 1;
        /* AUTODOC: Sets number of labels in tunnel header */
        rc = prvTgfPacketNumberOfMplsLabelsSet(2);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketNumberOfMplsLabelsSet");
        break;
    case 2:
        ii = 2;
        /* AUTODOC: Sets number of labels in tunnel header */
        rc = prvTgfPacketNumberOfMplsLabelsSet(2);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketNumberOfMplsLabelsSet");
        break;
    case 3:
        ii = 0;
         /* AUTODOC: Sets number of labels in tunnel header */
        rc = prvTgfPacketNumberOfMplsLabelsSet(1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketNumberOfMplsLabelsSet");
        break;
    case 4:
        /* AUTODOC: Sets number of labels in tunnel header */
        rc = prvTgfPacketNumberOfMplsLabelsSet(1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketNumberOfMplsLabelsSet");
        ii = 3;
        break;
    }


    /* AUTODOC: get default Ethernet over Mpls packet */
    rc = prvTgfPacketEthernetOverMplsPacketDefaultPacketGet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS, &packetInfo.numOfParts, &packetInfo.partsArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthernetOverMplsPacketDefaultPacketGet");

    packetInfo.totalLen = TGF_PACKET_AUTO_CALC_LENGTH_CNS;

    /* expect mac DA of TS */
    cpssOsMemCpy(l2Part.daMac,
                 tsMacDaArr[1].arEther,
                 6);
    /* expect unique mac SA according to egress port */
    cpssOsMemCpy(l2Part.saMac,
                 tsMacSaArr[ii].arEther,
                 6);

    /* AUTODOC: change L2 part of tunnel */
    rc = prvTgfPacketEthOverMplsPacketHeaderDataSet(TGF_PACKET_PART_L2_E,GT_FALSE,0,&l2Part);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketEthOverMplsPacketHeaderDataSet");

    vlanTagPart.vid = PRV_CST_MPLS_VLANID_CNS;

    /* AUTODOC: change VLAN part of tunnel */
    rc = prvTgfPacketEthOverMplsPacketHeaderDataSet(TGF_PACKET_PART_VLAN_TAG_E,GT_FALSE,0,&vlanTagPart);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketEthOverMplsPacketHeaderDataSet");

    /* AUTODOC: change Label 1 of tunnel */
    labelPart.exp = expTs;
    labelPart.label = mplsLabelArr[1];
    labelPart.stack = 0;
    labelPart.timeToLive = timeToLive;

    if (packetNumber >= 3)
        labelPart.stack = 1;

    rc = prvTgfPacketEthOverMplsPacketHeaderDataSet(TGF_PACKET_PART_MPLS_E,GT_FALSE,0,&labelPart);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketEthOverMplsPacketHeaderDataSet");

    if (packetNumber < 3)
    {
        /* AUTODOC: change Label 2 of tunnel */
        labelPart1.exp = expTs;
        labelPart1.label = mplsTtLabelArr[1];
        labelPart1.stack = 1;
        labelPart1.timeToLive = timeToLive;

        rc = prvTgfPacketEthOverMplsPacketHeaderDataSet(TGF_PACKET_PART_MPLS_E,GT_FALSE,1,&labelPart1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketEthOverMplsPacketHeaderDataSet");
    }

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[egressPortIndex];

    /* reset the previous value */
    actualCapturedNumOfPackets = 0;

    PRV_UTF_LOG1_MAC("Port [%d] capturing on physical port:\n", prvTgfPortsArray[egressPortIndex]);

    /* print captured packets and check TriggerCounters */
    rc = tgfTrafficGeneratorPortTxEthCaptureCompare(
            &portInterface,
            &packetInfo,
            numOfExpectedPackets,/*numOfPackets*/
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

    /* AUTODOC: restore number of labels in tunnel header */
    rc = prvTgfPacketNumberOfMplsLabelsSet(1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketNumberOfMplsLabelsSet");


}

/**
* @internal check3LTunnelCapturedPacket function
* @endinternal
*
*/
static GT_VOID check3LTunnelCapturedPacket
(
    IN GT_U32    egressPortIndex,
    IN GT_U32    packetNumber
)
{
    GT_STATUS   rc;
    CPSS_INTERFACE_INFO_STC portInterface;
    GT_U32  actualCapturedNumOfPackets = 0;
    GT_U32  numOfExpectedPackets = prvTgfBurstCount;
    TGF_PACKET_VLAN_TAG_STC vlanTagPart = {0x8100, 0, 0, 0};
    TGF_PACKET_STC          packetInfo;
    TGF_PACKET_L2_STC       l2Part;
    TGF_PACKET_MPLS_STC     labelPart,label1Part = {0};
    GT_ETHERADDR macSa = {{0,0,0,0,0,PRV_CST_MPLS_VLANID_CNS}};

    /* AUTODOC: Sets number of vlan tags in passenger header */
    rc = prvTgfPacketNumberOfVlanTagsSet(GT_TRUE/*passenger*/,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketNumberOfVlanTagsSet");

    /* AUTODOC: Sets number of vlan tags in tunnel header */
    rc = prvTgfPacketNumberOfVlanTagsSet(GT_FALSE/*tunnel*/,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketNumberOfVlanTagsSet");

    switch (packetNumber)
    {
    case 1:
    case 2:
         /* AUTODOC: Sets number of labels in tunnel header */
        rc = prvTgfPacketNumberOfMplsLabelsSet(1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketNumberOfMplsLabelsSet");

        /* expect mac SA of TS */
        cpssOsMemCpy(l2Part.saMac,
                     macSa.arEther,
                     6);
        labelPart.label = mplsTtLabelArr[2];
        labelPart.stack = 1;
        break;

    case 3:
         /* AUTODOC: Sets number of labels in tunnel header */
        rc = prvTgfPacketNumberOfMplsLabelsSet(2);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketNumberOfMplsLabelsSet");

        /* expect mac SA of TS */
        cpssOsMemCpy(l2Part.saMac,
                     tsMacSaArr[3].arEther,
                     6);
        labelPart.label = mplsLabelArr[3];
        labelPart.stack = 0;
        break;

    case 4:
         /* AUTODOC: Sets number of labels in tunnel header */
        rc = prvTgfPacketNumberOfMplsLabelsSet(2);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketNumberOfMplsLabelsSet");

        /* expect mac SA of TS */
        cpssOsMemCpy(l2Part.saMac,
                     macSa.arEther,
                     6);
        labelPart.label = mplsTtLabelArr[1];
        labelPart.stack = 0;
        break;

    case 5:
         /* AUTODOC: Sets number of labels in tunnel header */
        rc = prvTgfPacketNumberOfMplsLabelsSet(2);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketNumberOfMplsLabelsSet");

        /* expect mac SA of TS */
        cpssOsMemCpy(l2Part.saMac,
                     tsMacSaArr[3].arEther,
                     6);
        labelPart.label = mplsLabelArr[3];
        labelPart.stack = 0;
        break;

    case 6:
         /* AUTODOC: Sets number of labels in tunnel header */
        rc = prvTgfPacketNumberOfMplsLabelsSet(2);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketNumberOfMplsLabelsSet");

        /* expect mac SA of TS */
        cpssOsMemCpy(l2Part.saMac,
                     tsMacSaArr[0].arEther,
                     6);
        labelPart.label = mplsLabelArr[0];
        labelPart.stack = 0;
        break;
    }

    /* AUTODOC: get default Ethernet over Mpls packet */
    rc = prvTgfPacketEthernetOverMplsPacketDefaultPacketGet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS, &packetInfo.numOfParts, &packetInfo.partsArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthernetOverMplsPacketDefaultPacketGet");

    packetInfo.totalLen = TGF_PACKET_AUTO_CALC_LENGTH_CNS;

    /* expect mac DA of TS */
    cpssOsMemCpy(l2Part.daMac,
                 tsMacDaArr[3].arEther,
                 6);

    /* AUTODOC: change L2 part of tunnel */
    rc = prvTgfPacketEthOverMplsPacketHeaderDataSet(TGF_PACKET_PART_L2_E,GT_FALSE,0,&l2Part);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketEthOverMplsPacketHeaderDataSet");

    vlanTagPart.vid = PRV_CST_MPLS_VLANID_CNS;

    /* AUTODOC: change VLAN part of tunnel */
    rc = prvTgfPacketEthOverMplsPacketHeaderDataSet(TGF_PACKET_PART_VLAN_TAG_E,GT_FALSE,0,&vlanTagPart);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketEthOverMplsPacketHeaderDataSet");


    /* AUTODOC: change Label 1 of tunnel */
    labelPart.exp = expTs;
    labelPart.timeToLive = timeToLive;

    rc = prvTgfPacketEthOverMplsPacketHeaderDataSet(TGF_PACKET_PART_MPLS_E,GT_FALSE,0,&labelPart);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketEthOverMplsPacketHeaderDataSet");

    /* this is second test with 2 labels */
    if (packetNumber > 2)
    {
        /* AUTODOC: change Label 2 of tunnel */
        label1Part.label = mplsTtLabelArr[2];
        label1Part.stack = 1;
        label1Part.exp = expTs;
        label1Part.timeToLive = timeToLive;

        rc = prvTgfPacketEthOverMplsPacketHeaderDataSet(TGF_PACKET_PART_MPLS_E,GT_FALSE,1,&label1Part);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketEthOverMplsPacketHeaderDataSet");
    }

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[egressPortIndex];

    /* reset the previous value */
    actualCapturedNumOfPackets = 0;

    PRV_UTF_LOG1_MAC("Port [%d] capturing on physical port:\n", prvTgfPortsArray[egressPortIndex]);

    /* print captured packets and check TriggerCounters */
    rc = tgfTrafficGeneratorPortTxEthCaptureCompare(
            &portInterface,
            &packetInfo,
            numOfExpectedPackets,/*numOfPackets*/
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

    /* AUTODOC: restore number of labels in tunnel header */
    rc = prvTgfPacketNumberOfMplsLabelsSet(1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketNumberOfMplsLabelsSet");


}

/**
* @internal check4LTunnelCapturedPacket function
* @endinternal
*
*/
static GT_VOID check4LTunnelCapturedPacket
(
    IN GT_U32    egressPortIndex,
    IN GT_U32    packetNumber
)
{
    GT_STATUS   rc;
    CPSS_INTERFACE_INFO_STC portInterface;
    GT_U32  actualCapturedNumOfPackets = 0;
    GT_U32  numOfExpectedPackets = prvTgfBurstCount;
    TGF_PACKET_VLAN_TAG_STC vlanTagPart = {0x8100, 0, 0, 0};
    TGF_PACKET_STC          packetInfo;
    TGF_PACKET_L2_STC       l2Part;
    TGF_PACKET_MPLS_STC     labelPart, label1Part = {0};
    GT_ETHERADDR macSa = {{0,0,0,0,0,PRV_CST_MPLS_VLANID_CNS}};
    TGF_TTL ttl = timeToLive;

    /* AUTODOC: Sets number of vlan tags in passenger header */
    rc = prvTgfPacketNumberOfVlanTagsSet(GT_TRUE/*passenger*/,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketNumberOfVlanTagsSet");

    /* AUTODOC: Sets number of vlan tags in tunnel header */
    rc = prvTgfPacketNumberOfVlanTagsSet(GT_FALSE/*tunnel*/,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketNumberOfVlanTagsSet");

    switch (packetNumber)
    {
    case 1:
    case 2:
        /* expect mac SA of TS */
        cpssOsMemCpy(l2Part.saMac,
                     macSa.arEther,
                     6);

        labelPart.label = mplsTtLabelArr[3];
        labelPart.stack = 1;

        /* AUTODOC: Sets number of labels in tunnel header */
        rc = prvTgfPacketNumberOfMplsLabelsSet(1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketNumberOfMplsLabelsSet");
        break;

    case 3:
        /* expect mac SA of TS */
        cpssOsMemCpy(l2Part.saMac,
                     tsMacSaArr[3].arEther,
                     6);
        labelPart.label = mplsLabelArr[3];
        labelPart.stack = 0;

        /* AUTODOC: Sets number of labels in tunnel header */
        rc = prvTgfPacketNumberOfMplsLabelsSet(2);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketNumberOfMplsLabelsSet");
        break;
    }

    /* AUTODOC: get default Ethernet over Mpls packet */
    rc = prvTgfPacketEthernetOverMplsPacketDefaultPacketGet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS, &packetInfo.numOfParts, &packetInfo.partsArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthernetOverMplsPacketDefaultPacketGet");

    packetInfo.totalLen = TGF_PACKET_AUTO_CALC_LENGTH_CNS;

    /* expect mac DA of TS */
    cpssOsMemCpy(l2Part.daMac,
                 tsMacDaArr[3].arEther,
                 6);

    /* AUTODOC: change L2 part of tunnel */
    rc = prvTgfPacketEthOverMplsPacketHeaderDataSet(TGF_PACKET_PART_L2_E,GT_FALSE,0,&l2Part);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketEthOverMplsPacketHeaderDataSet");

    vlanTagPart.vid = PRV_CST_MPLS_VLANID_CNS;

    /* AUTODOC: change VLAN part of tunnel */
    rc = prvTgfPacketEthOverMplsPacketHeaderDataSet(TGF_PACKET_PART_VLAN_TAG_E,GT_FALSE,0,&vlanTagPart);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketEthOverMplsPacketHeaderDataSet");


    /* AUTODOC: change Label 1 of tunnel */
    labelPart.exp = expTs;

    labelPart.timeToLive = ttl;

    rc = prvTgfPacketEthOverMplsPacketHeaderDataSet(TGF_PACKET_PART_MPLS_E,GT_FALSE,0,&labelPart);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketEthOverMplsPacketHeaderDataSet");

    if (packetNumber == 3)
    {
        /* AUTODOC: change Label 1 of tunnel */
    label1Part.exp = expTs;
    label1Part.timeToLive = ttl;
    label1Part.label = mplsTtLabelArr[3];
    label1Part.stack = 1;

    rc = prvTgfPacketEthOverMplsPacketHeaderDataSet(TGF_PACKET_PART_MPLS_E,GT_FALSE,1,&label1Part);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketEthOverMplsPacketHeaderDataSet");

    }
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[egressPortIndex];

    /* reset the previous value */
    actualCapturedNumOfPackets = 0;

    PRV_UTF_LOG1_MAC("Port [%d] capturing on physical port:\n", prvTgfPortsArray[egressPortIndex]);

    /* print captured packets and check TriggerCounters */
    rc = tgfTrafficGeneratorPortTxEthCaptureCompare(
            &portInterface,
            &packetInfo,
            numOfExpectedPackets,/*numOfPackets*/
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

    /* AUTODOC: restore number of labels in tunnel header */
    rc = prvTgfPacketNumberOfMplsLabelsSet(1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketNumberOfMplsLabelsSet");

}

/**
* @internal checkTunnelCapturedPacket function
* @endinternal
*
*/
static GT_VOID checkTunnelCapturedPacket
(
    IN GT_U32    egressPortIndex,
    IN GT_U32    packetNumber
)
{
    switch (numOfLabels)
    {
    case 2:
        check2LTunnelCapturedPacket(egressPortIndex,packetNumber);
        break;
    case 3:
        check3LTunnelCapturedPacket(egressPortIndex,packetNumber);
        break;
    case 4:
        check4LTunnelCapturedPacket(egressPortIndex,packetNumber);
        break;
    }
    return;
}

/**
* @internal checkGotNoPacket function
* @endinternal
*
*/
static GT_VOID checkGotNoPacket
(
    IN GT_U32 egressPortIndex
)
{
    GT_STATUS   rc;
    GT_BOOL                         isEqualCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;/* current counters of the port */

    rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[egressPortIndex], GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[egressPortIndex]);

    /* compare the counters */
    PRV_TGF_VERIFY_COUNTERS_MAC(isEqualCntrs, allZeroCntrs, portCntrs);
    UTF_VERIFY_EQUAL0_STRING_MAC(isEqualCntrs, GT_TRUE, "get another counters values.");

    /* print expected values if not equal */
    PRV_TGF_PRINT_DIFF_COUNTERS_MAC(isEqualCntrs, allZeroCntrs, portCntrs);

}

/**
* @internal sendFlood function
* @endinternal
*
*/
static GT_VOID sendFlood
(
    IN GT_U32 bmpFilteredPortIndexBmp
)
{
    GT_U32  ii;
    GT_U32  actualSendNum = 0;


    /*AUTODOC: reset ETH counters (also set link to up on tested ports) */
    PRV_UTF_LOG0_MAC("reset ETH counters (also set link to up on tested ports)\n");
    prvTgfEthCountersReset(prvTgfDevNum);

    /* AUTODOC: we check flood , and need to capture the egress packets on all egress ports */
    for(ii = 0 ; ii < prvTgfPortsNum ; ii++)
    {
        if(ii == PRV_TGF_SEND_PORT_IDX_CNS)
        {
            /* AUTODOC:skip checking the sender port */
            continue;
        }

        /* AUTODOC: start capture + clear captured table */
        PRV_UTF_LOG0_MAC("start capture + clear captured table \n");
        setCapture(ii,GT_TRUE);

        /* AUTODOC: send packet */
        PRV_UTF_LOG0_MAC("send packet \n");
        sendPacket();

        /* AUTODOC: stop capture */
        PRV_UTF_LOG0_MAC("stop capture \n");
        setCapture(ii,GT_FALSE);

        actualSendNum++;

        if(0 == (bmpFilteredPortIndexBmp & (BIT_0 << ii)))
        {
            /* AUTODOC: check that egress as expected */
            PRV_UTF_LOG0_MAC("check that egress as expected \n");
            checkCapturedPacket(ii);
        }
        else
        {
            PRV_UTF_LOG0_MAC("check the filtered port got no packet \n");
            /* AUTODOC: check the filtered port got no packet */
            checkGotNoPacket(ii);
        }

        /* AUTODOC: reset ETH counters */
        PRV_UTF_LOG0_MAC("reset ETH counters \n");
        prvTgfEthCountersReset(prvTgfDevNum);

    }
}

/**
* @internal sendKnowUc function
* @endinternal
*
*/
static GT_VOID sendKnowUc
(
    IN GT_U32 senderPortIndex,
    IN GT_U32 targetPortIndex,
    IN GT_U32 packetNumber
)
{
    GT_U32  ii;

    /*AUTODOC: reset ETH counters (also set link to up on tested ports) */
    PRV_UTF_LOG0_MAC("reset ETH counters (also set link to up on tested ports)\n");
    prvTgfEthCountersReset(prvTgfDevNum);

    /* AUTODOC: start capture + clear captured table */
    PRV_UTF_LOG0_MAC("start capture + clear captured table \n");
    setCapture(targetPortIndex,GT_TRUE);

    /* AUTODOC: send packet */
    PRV_UTF_LOG0_MAC("send packet \n");
    sendTunneledPacket(senderPortIndex,packetNumber);

    /* AUTODOC: stop capture */
    PRV_UTF_LOG0_MAC("stop capture \n");
    setCapture(targetPortIndex,GT_FALSE);

    /* AUTODOC: check that egress as expected (on target port) */
    PRV_UTF_LOG0_MAC("check that egress as expected (on target port) \n");
    checkTunnelCapturedPacket(targetPortIndex, packetNumber);

    /* AUTODOC: we check no flood on other ports */
    for(ii = 0 ; ii < prvTgfPortsNum ; ii++)
    {
        if(ii == senderPortIndex ||
            ii == targetPortIndex)
        {
            /* AUTODOC: skip checking the sender port */
            /* AUTODOC: skip checking the target port */
            continue;
        }

        /* AUTODOC: check this port got no packet */
        PRV_UTF_LOG0_MAC("check this port got no packet \n");
        checkGotNoPacket(ii);
    }

    /* AUTODOC: reset ETH counters */
    PRV_UTF_LOG0_MAC("reset ETH counters \n");
    prvTgfEthCountersReset(prvTgfDevNum);
}

/**
* @internal prvCstSetEgressTagging_restore function
* @endinternal
*
* @brief   restore Egress tagging configuration
*
* @param[in] ePortArr[4]              - eports
*                                       None
*/
static GT_STATUS prvCstSetEgressTagging_restore
(
    IN  GT_PORT_NUM           ePortArr[4]   /* ePort_A..D */
)
{
    GT_U32  ii;

    /* AUTODOC: eport B,C,D - tag0 only*/
    for(ii = 1; ii < numOfPorts ; ii++)
    {
        /* AUTODOC: use tagging from eport not from vlan */
        CHECK_RC_MAC(
            cpssDxChBrgVlanEgressPortTagStateModeSet(swDevNum,ePortArr[ii],
                                                     prvCstRestoreCfg.stateMode[ii]));

        /* AUTODOC: egress tag1*/
        CHECK_RC_MAC(
            cpssDxChBrgVlanEgressPortTagStateSet(swDevNum,ePortArr[ii],
                                                 prvCstRestoreCfg.tagCmd[ii]));
    }

    return GT_OK;
}

/**
* @internal prvCstSetE2Phy_restore function
* @endinternal
*
* @brief   restore E2Phy configuration
*
* @param[in] ePortArr[4]              - eports
*                                       None
*/
static GT_STATUS prvCstSetE2Phy_restore
(
    IN  GT_PORT_NUM           ePortArr[4]   /* ePort_A..D */
)
{
    GT_U32  ii;

    for(ii = 0 ; ii < numOfPorts ; ii++)
    {
        /* when tests trying to restore value that got from the 'get' ,
           in GM may come with 256..511 */
        prvCstRestoreCfg.physicalInfo[ii].devPort.portNum &= 0xFF;CPSS_TBD_BOOKMARK_BOBCAT3
    }

    for(ii = 0 ; ii < numOfPorts ; ii++)
    {
        CHECK_RC_MAC(
            cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(swDevNum,
                                                                ePortArr[ii],&prvCstRestoreCfg.physicalInfo[ii]));
    }

    return GT_OK;
}

/**
* @internal prvCstSetMplsTunnelStart_restore function
* @endinternal
*
* @brief   restore MPLS Tunnel Start configuration
*
* @param[in] phyPortArr[4]            - physical ports
* @param[in] ePortArr[4]              - eports
*                                       None
*/
static GT_STATUS prvCstSetMplsTunnelStart_restore
(
    IN  GT_PHYSICAL_PORT_NUM  phyPortArr[4],/* physical port associated with ePort_A..D */
    IN  GT_PORT_NUM           ePortArr[4]   /* ePort_A..D */
)
{
    CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC          egressEportInfo;
    GT_U32                                      ii;

    cpssOsMemSet(&egressEportInfo,0,sizeof(egressEportInfo));
    egressEportInfo.tunnelStart = GT_FALSE;


    /* AUTODOC: remove the TS entries from eports A,B,C,D */
    for(ii = 0/*eport A*/ ; ii < numOfPorts ; ii++)
    {

        switch (ii)
        {
        case 3: /* eport D */
            /* AUTODOC: disable the PW label */
            CHECK_RC_MAC(
                cpssDxChTunnelStartMplsPwLabelPushEnableSet(swDevNum,ePortArr[ii],GT_FALSE));

            /* AUTODOC: reset the PW label */
            CHECK_RC_MAC(
                cpssDxChTunnelStartMplsPwLabelSet(swDevNum,ePortArr[ii],0));

            /* AUTODOC: reset the PW label EXP */
            CHECK_RC_MAC(
                cpssDxChTunnelStartMplsPwLabelExpSet(swDevNum,ePortArr[ii],0));

            /* AUTODOC: reset the PW label TTL */
            CHECK_RC_MAC(
                cpssDxChTunnelStartMplsPwLabelTtlSet(swDevNum,ePortArr[ii],0));

            /* AUTODOC: disable the FLOW label */
            CHECK_RC_MAC(
                cpssDxChTunnelStartMplsFlowLabelEnableSet(swDevNum,ePortArr[ii],GT_FALSE));

            /* AUTODOC: reset the FLOW label TTL */
            CHECK_RC_MAC(
                cpssDxChTunnelStartMplsFlowLabelTtlSet(swDevNum,0));

            /* AUTODOC: reset a Pseudo Wire control word */
            CHECK_RC_MAC(
                cpssDxChTunnelStartMplsPwControlWordSet(swDevNum,cwLabelIndex,0));

            break;
        }

        /* AUTODOC: unbind eport from TS */
        CHECK_RC_MAC(
            cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(swDevNum,ePortArr[ii],&egressEportInfo));

        /* AUTODOC: unset router mac sa index refered to global MAC SA table */
        CHECK_RC_MAC(
            cpssDxChIpRouterPortGlobalMacSaIndexSet(
                swDevNum, ePortArr[ii], 0));

        /* AUTODOC: unsets full 48-bit Router MAC SA in Global MAC SA table */
        CHECK_RC_MAC(
            cpssDxChIpRouterGlobalMacSaSet(
                swDevNum, phyPortArr[ii] & 0xFF,&MacAllZero /*all zero*/));

    }

    return GT_OK;
}

/**
* @internal prvCstSetUcMplsTunnelStart_restore function
* @endinternal
*
* @brief   restore MPLS Tunnel Start configuration
*
* @param[in] phyPortArr[4]            - physical ports
* @param[in] ePortArr[4]              - eports
*                                       None
*/
static GT_STATUS prvCstSetUcMplsTunnelStart_restore
(
    IN  GT_PHYSICAL_PORT_NUM  phyPortArr[4],/* physical port associated with ePort_A..D */
    IN  GT_PORT_NUM           ePortArr[4]   /* ePort_A..D */
)
{
    GT_U32 ii;

    for(ii = 0/*eport A*/ ; ii < numOfPorts ; ii++)
    {
        /* AUTODOC: unset router mac sa index refered to global MAC SA table */
        CHECK_RC_MAC(
            cpssDxChIpRouterPortGlobalMacSaIndexSet(
                swDevNum, ePortArr[ii], 0));

       /* AUTODOC: unsets full 48-bit Router MAC SA in Global MAC SA table */
        CHECK_RC_MAC(
            cpssDxChIpRouterGlobalMacSaSet(
                swDevNum, phyPortArr[ii] & 0xFF,&MacAllZero/*all zero*/));
    }

    return GT_OK;
}

/**
* @internal prvCstSetBridge_restore function
* @endinternal
*
* @brief   restore Bridge Configuration
*
* @param[in] vlanId                   - vlan id
*                                       None
*/
static GT_STATUS prvCstSetBridge_restore
(
    IN GT_U16                vlanId
)
{

    /* AUTODOC: invalidate VLAN entry */
    CHECK_RC_MAC(
        cpssDxChBrgVlanEntryInvalidate(swDevNum,vlanId));

    /* AUTODOC: restore VLAN egress filtering configuration */
    CHECK_RC_MAC(
        cpssDxChBrgVlanEgressFilteringEnable(swDevNum,prvCstRestoreCfg.vlanEgressFilteringState));

    return GT_OK;

}

/**
* @internal prvCstSetL2Mll_restore function
* @endinternal
*
* @brief   restore L2MLL configuration
*/
static GT_STATUS prvCstSetL2Mll_restore
(
    GT_VOID
)
{
    CPSS_DXCH_L2_MLL_LTT_ENTRY_STC              lttEntry;
    CPSS_DXCH_L2_MLL_PAIR_STC                   mllPairEntry;
    GT_U32                                      mllPairIndex;

    cpssOsMemSet(&lttEntry, 0, sizeof(lttEntry));
    cpssOsMemSet(&mllPairEntry, 0, sizeof(mllPairEntry));

    /* AUTODOC: map the eVIDX to a pointer of linked-list of L2MLL */
    mllPairIndex = mllIndex; /*index on first entry */
    lttEntry.mllPointer = 0;
    lttEntry.entrySelector = 0; /* start of the entry */

    /* AUTODOC: unset the L2MLL LTT entry (index = eVidx of global eVlan) */
    /* AUTODOC: the LTT point to index in the MLL table */
    CHECK_RC_MAC(
        cpssDxChL2MllLttEntrySet(swDevNum,
        (globalFloodEvidx - _4K),/* working in mode of relative to _4K */
        &lttEntry));

    /* AUTODOC: delete first MLL entry (2 nodes)*/
    CHECK_RC_MAC(
        cpssDxChL2MllPairWrite(swDevNum, mllPairIndex ,
                               CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E,
                               &mllPairEntry));

    /* AUTODOC: delete second MLL entry (1 node) */
    CHECK_RC_MAC(
        cpssDxChL2MllPairWrite(swDevNum, mllPairIndex + 1,
                               CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E,
                               &mllPairEntry));


    return GT_OK;

}

/**
* @internal prvCstSetMplsTunnelTermination_restore function
* @endinternal
*
* @brief   restore ingress tunnel termination configuration
*
* @param[in] phyPortArr[4]            - physical ports
*                                       None
*/
static GT_STATUS prvCstSetMplsTunnelTermination_restore
(
    IN  GT_PHYSICAL_PORT_NUM  phyPortArr[4]/* physical port associated with ePort_A..D */
)
{
    GT_U32 ii;

    switch (numOfLabels)
    {
    case 2:
        for (ii = 0; ii<=5; ii++)
        {
            /* invalidate the rules */
            CHECK_RC_MAC(
                cpssDxChTtiRuleValidStatusSet(swDevNum,
                                              tunnelTerminationIndexPtr[ii],/*index*/
                                              GT_FALSE));
        }
        break;
    case 3:
        for (ii = 0; ii<numOfTT; ii++)
        {
            /* skip rule 3 */
            if (ii == 3)
            {
                ii++;
            }
            /* invalidate the rules */
            CHECK_RC_MAC(
                cpssDxChTtiRuleValidStatusSet(swDevNum,
                                              tunnelTerminationIndexPtr[ii],/*index*/
                                              GT_FALSE));
        }
        break;
    case 4:
        for (ii = 0; ii<=5; ii++)
        {
            /* skip rule 3 */
            if (ii == 3)
            {
                ii++;
            }
            /* invalidate the rules */
            CHECK_RC_MAC(
                cpssDxChTtiRuleValidStatusSet(swDevNum,
                                              tunnelTerminationIndexPtr[ii],/*index*/
                                              GT_FALSE));
        }
        break;

    }

    /* AUTODOC: enable physical port to do TTI MPLS lookup */
    CHECK_RC_MAC(
        cpssDxChTtiPortLookupEnableSet(swDevNum,phyPortArr[0],
                                       CPSS_DXCH_TTI_KEY_MPLS_E,
                                       prvCstRestoreCfg.portLookupEnable[0]));

    /* AUTODOC: enable physical port to do TTI MPLS lookup */
    CHECK_RC_MAC(
        cpssDxChTtiPortLookupEnableSet(swDevNum,phyPortArr[1],
                                       CPSS_DXCH_TTI_KEY_MPLS_E,
                                       prvCstRestoreCfg.portLookupEnable[1]));

    /* AUTODOC: sets the lookup Mac mode */
    CHECK_RC_MAC(
        cpssDxChTtiMacModeSet(swDevNum,CPSS_DXCH_TTI_RULE_MPLS_E,
                              prvCstRestoreCfg.macMode));

    return GT_OK;
}

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal cstTunnelStartMplsUnknownUcFloodingConfig function
* @endinternal
*
* @brief   test configuration
*/
GT_STATUS cstTunnelStartMplsUnknownUcFloodingConfig
(
    GT_VOID
)
{
    eVlan = PRV_CST_ETHERNET_VLANID_CNS;

    ePortArr[0] = phyPortArr[0]  + 0x1000;
    ePortArr[1] = phyPortArr[1]  + 0x1000;
    ePortArr[2] = phyPortArr[2]  + 0x1000;
    ePortArr[3] = phyPortArr[3]  + 0x1000;

    globalFloodEvidx = (GT_U16)((eVlan & (_4K-1)) + _4K); /* eVidx to a pointer of linked-list of L2MLL > _4K */


    /* AUTODOC: get hw dev num */
    CHECK_RC_MAC(
    cpssDxChCfgHwDevNumGet(swDevNum,&hwDevNum));

    /* AUTODOC: set e2phy */
    CHECK_RC_MAC(
        prvCstSetE2Phy(phyPortArr,ePortArr));

    /* AUTODOC: set egress tagging */
    CHECK_RC_MAC(
        prvCstSetEgressTagging(ePortArr));

    /* AUTODOC: set MPLS tunnel start */
    CHECK_RC_MAC(
        prvCstSetMplsTunnelStart(phyPortArr,ePortArr));

    /* AUTODOC: set bridge */
    CHECK_RC_MAC(
        prvCstSetBridge(phyPortArr,eVlan));

    /* AUTODOC: set L2Mll */
    CHECK_RC_MAC(
        prvCstSetL2Mll(ePortArr));

    return GT_OK;
}

/**
* @internal cstTunnelStartMplsUnknownUcFloodingTrafficGenerate function
* @endinternal
*
* @brief   generate traffic
*/
GT_STATUS cstTunnelStartMplsUnknownUcFloodingTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    /* AUTODOC: test1: tunnel start unknown uc flooding */
    /* AUTODOC: MPLS configuration of device with 1 port in ETHERNET domain, and 3 ports in MPLS domain. */
    /* AUTODOC: test check 'flooding' from ethernet port to the other 3 mpls ports.(TS to the MPLS domain) */
    /* AUTODOC: ePort A is port to Ethernet domain */
    /* AUTODOC: eport B is port to MPLS domain with 1 label */
    /* AUTODOC: eport C is port to MPLS domain with 2 labels */
    /* AUTODOC: eport D is port to MPLS domain with 3 labels and PW label + FLOW label + CW label */

    PRV_UTF_LOG0_MAC("\ntest1: tunnel start unknown uc flooding \n");
    PRV_UTF_LOG0_MAC(" MPLS configuration of device with 1 port in ETHERNET domain, and 3 ports in MPLS domain. \n");
    PRV_UTF_LOG0_MAC(" test check 'flooding' from ethernet port to the other 3 mpls ports.(TS to the MPLS domain) \n");
    PRV_UTF_LOG0_MAC(" ePort A is port to Ethernet domain \n");
    PRV_UTF_LOG0_MAC(" eport B is port to MPLS domain with 1 label \n");
    PRV_UTF_LOG0_MAC(" eport C is port to MPLS domain with 2 labels \n");
    PRV_UTF_LOG0_MAC(" eport D is port to MPLS domain with 3 labels and PW label + FLOW label + CW label \n\n");

    sendFlood(0/* no filters*/);

    return rc;

}

/**
* @internal cstTunnelStartMplsUnknownUcFloodingRestore function
* @endinternal
*
* @brief   restore test configuration
*/
GT_STATUS cstTunnelStartMplsUnknownUcFloodingRestore
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    /* AUTODOC: unset e2phy */
    CHECK_RC_MAC(
        prvCstSetE2Phy_restore(ePortArr));

    /* AUTODOC: unset egress tagging */
    CHECK_RC_MAC(
        prvCstSetEgressTagging_restore(ePortArr));

    /* AUTODOC: unset MPLS tunnel start */
    CHECK_RC_MAC(
        prvCstSetMplsTunnelStart_restore(phyPortArr,ePortArr));

    /* AUTODOC: unset bridge */
    CHECK_RC_MAC(
        prvCstSetBridge_restore(eVlan));

    /* AUTODOC: unset L2Mll */
    CHECK_RC_MAC(
        prvCstSetL2Mll_restore());

    /* AUTODOC: reset ETH counters */
    prvTgfEthCountersReset(prvTgfDevNum);

    /* AUTODOC: flush FDB include static entries */
    CHECK_RC_MAC(
        prvTgfBrgFdbFlush(GT_TRUE));

    return rc;
}

#endif

/**
* @internal cstTunnelStartMplsUnknownUcFloodingTest function
* @endinternal
*
* @brief   test1: tunnel start unknown uc flooding
*         MPLS configuration of device with 1 port in ETHERNET domain, and 3 ports in MPLS domain.
*         test check 'flooding' from ethernet port to the other 3 mpls ports.(TS to the MPLS domain)
*         ePort A is port to Ethernet domain
*         eport B is port to MPLS domain with 1 label
*         eport C is port to MPLS domain with 2 labels
*         eport D is port to MPLS domain with 3 labels and PW label + FLOW label + CW label
*/
GT_STATUS cstTunnelStartMplsUnknownUcFloodingTest
(
    GT_VOID
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;

    {/* use tested ports , because {0, 18, 36, 58} are not always valid */
        GT_U32  ii;
        for(ii = 0; ii < 4 ; ii++)
        {
            phyPortArr[ii] = prvTgfPortsArray[ii];
        }
    }

    CHECK_RC_MAC(
        cstTunnelStartMplsUnknownUcFloodingConfig());

    CHECK_RC_MAC(
        cstTunnelStartMplsUnknownUcFloodingTrafficGenerate());

    CHECK_RC_MAC(
        cstTunnelStartMplsUnknownUcFloodingRestore());

    return rc;
#else
    return GT_NOT_IMPLEMENTED;
#endif  /* CHX_FAMILY */
}

/**
* @internal cstTunnelTerminationTransitMplsKnownUc2LabelsConfig function
* @endinternal
*
* @brief   test configuration
*/
GT_STATUS cstTunnelTerminationTransitMplsKnownUc2LabelsConfig
(
    GT_VOID
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;

    eVlan = PRV_CST_MPLS_VLANID_CNS;

    ePortArr[0] = phyPortArr[0]  + 0x1000;
    ePortArr[1] = phyPortArr[1]  + 0x1000;
    ePortArr[2] = phyPortArr[2]  + 0x1000;
    ePortArr[3] = phyPortArr[3]  + 0x1000;

    globalFloodEvidx = 0;
    numOfLabels = 2;

    /* AUTODOC: get hw dev num */
    CHECK_RC_MAC(
    cpssDxChCfgHwDevNumGet(swDevNum,&hwDevNum));

    /* AUTODOC: set e2phy */
    CHECK_RC_MAC(
        prvCstSetE2Phy(phyPortArr,ePortArr));

    /* AUTODOC: set egress tagging */
    CHECK_RC_MAC(
        prvCstSetEgressTagging(ePortArr));

    /* AUTODOC: set MPLS tunnel start */
    CHECK_RC_MAC(
        prvCstSetMplsTunnelStart(phyPortArr,ePortArr));

    /* AUTODOC: set MPLS tunnel termination */
    CHECK_RC_MAC(
        prvCstSetMpls2LTunnelTermination(phyPortArr,ePortArr));

    /* AUTODOC: set bridge */
    CHECK_RC_MAC(
        prvCstSetBridge(phyPortArr,eVlan));

    return rc;
#else
    return GT_NOT_IMPLEMENTED;
#endif  /* CHX_FAMILY */
}

/**
* @internal cstTunnelTerminationTransitMplsKnownUc2LabelsTrafficGenerate function
* @endinternal
*
* @brief   generate traffic
*/
GT_STATUS cstTunnelTerminationTransitMplsKnownUc2LabelsTrafficGenerate
(
    GT_VOID
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;

    /* AUTODOC: test2: transit tunnel termination of known uc (2 labels manipulations)*/
    /* AUTODOC: MPLS configuration of device with 4 port in MPLS domain.*/
    /* AUTODOC: test check 2 Labels manipulations from mpls port to other 2 mpls ports.(TT + TS in the MPLS domain)*/
    /* AUTODOC: ePort A is port to MPLS domain with 2 labels */
    /* AUTODOC: eport B is port to MPLS domain with 2 labels. Label 1 is swapped */
    /* AUTODOC: eport C is port to MPLS domain with 2 labels. Label 1 is popped & new label is pushed. */

    PRV_UTF_LOG0_MAC("\ntest2: transit tunnel termination of known uc (2 labels manipulations) \n");
    PRV_UTF_LOG0_MAC("\nMPLS configuration of device with 4 port in MPLS domain. \n");
    PRV_UTF_LOG0_MAC("\test check 2 Labels manipulations from mpls port to other 2 mpls ports.(TT + TS in the MPLS domain) \n");
    PRV_UTF_LOG0_MAC(" ePort A is port to MPLS domain with 2 labels \n");
    PRV_UTF_LOG0_MAC(" eport B is port to MPLS domain with 2 labels. Label 1 is swapped \n");
    PRV_UTF_LOG0_MAC(" eport C is port to MPLS domain with 2 labels. Label 1 is popped & pushed \n");

    sendKnowUc(PRV_TGF_SEND_PORT_IDX_CNS/*sender*/,PRV_TGF_RECEIVE_PORT1_IDX_CNS/*receiver*/, 1/* packet number */);

    sendKnowUc(PRV_TGF_SEND_PORT_IDX_CNS/*sender*/,PRV_TGF_RECEIVE_PORT2_IDX_CNS/*receiver*/, 2/* packet number */);

    /* AUTODOC: test3: transit tunnel termination of known uc (2 labels manipulations)*/
    /* AUTODOC: MPLS configuration of device with 4 port in MPLS domain.*/
    /* AUTODOC: test check 2 Labels manipulations from mpls port to other 2 mpls ports.(TT + TS in the MPLS domain)*/
    /* AUTODOC: ePort B is port to MPLS domain with 2 labels */
    /* AUTODOC: eport A is port to MPLS domain with 1 label. Label 1 is pop_swap */
    /* AUTODOC: eport D is port to MPLS domain with 1 labels. Label 1 is popped & swap */

    PRV_UTF_LOG0_MAC("\ntest3: transit tunnel termination of known uc (2 labels manipulations) \n");
    PRV_UTF_LOG0_MAC("\nMPLS configuration of device with 4 port in MPLS domain. \n");
    PRV_UTF_LOG0_MAC("\ntest check 2 Labels manipulations from mpls port to other 2 mpls ports.(TT + TS in the MPLS domain) \n");
    PRV_UTF_LOG0_MAC(" ePort B is port to MPLS domain with 2 labels \n");
    PRV_UTF_LOG0_MAC(" eport A is port to MPLS domain with 1 label. Label 1 is pop_swap \n");
    PRV_UTF_LOG0_MAC(" eport D is port to MPLS domain with 1 labels. Label 1 is popped & swap \n");

    sendKnowUc(PRV_TGF_SEND_PORT1_IDX_CNS/*sender*/,PRV_TGF_RECEIVE_PORT_IDX_CNS/*receiver*/, 3/* packet number */);

    sendKnowUc(PRV_TGF_SEND_PORT1_IDX_CNS/*sender*/,PRV_TGF_RECEIVE_PORT3_IDX_CNS/*receiver*/, 4/* packet number */);

    return rc;
#else
    return GT_NOT_IMPLEMENTED;
#endif  /* CHX_FAMILY */

}

/**
* @internal cstTunnelTerminationTransitMplsKnownUc2LabelsRestore function
* @endinternal
*
* @brief   restore test configuration
*/
GT_STATUS cstTunnelTerminationTransitMplsKnownUc2LabelsRestore
(
    GT_VOID
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;

    /* AUTODOC: unset e2phy */
    CHECK_RC_MAC(
        prvCstSetE2Phy_restore(ePortArr));

    /* AUTODOC: unset egress tagging */
    CHECK_RC_MAC(
        prvCstSetEgressTagging_restore(ePortArr));

    /* AUTODOC: unset MPLS tunnel start */
    CHECK_RC_MAC(
        prvCstSetMplsTunnelStart_restore(phyPortArr,ePortArr));

    /* AUTODOC: unset bridge */
    CHECK_RC_MAC(
        prvCstSetBridge_restore(eVlan));

    /* AUTODOC: unset MPLS tunnel termination */
    CHECK_RC_MAC(
        prvCstSetMplsTunnelTermination_restore(phyPortArr));

    /* AUTODOC: reset ETH counters */
    prvTgfEthCountersReset(prvTgfDevNum);

    /* AUTODOC: flush FDB include static entries */
    CHECK_RC_MAC(
        prvTgfBrgFdbFlush(GT_TRUE));

    return rc;
#else
    return GT_NOT_IMPLEMENTED;
#endif  /* CHX_FAMILY */
}

/**
* @internal cstTunnelTerminationTransitMplsKnownUc2LabelsTest function
* @endinternal
*
* @brief   test2: transit tunnel termination of known uc (2 labels manipulations)
*         MPLS configuration of device with 4 port in MPLS domain.
*         test check 2 Labels manipulations from mpls port to other 2 mpls ports.(TT + TS in the MPLS domain)
*         ePort A is port to MPLS domain with 2 labels
*         eport B is port to MPLS domain with 2 labels. Label 1 is swapped.
*         eport C is port to MPLS domain with 2 labels. Label 1 is popped & new label is pushed.
*         test3: transit tunnel termination of known uc (2 labels manipulations)
*         MPLS configuration of device with 4 port in MPLS domain.
*         test check 2 Labels manipulations from mpls port to other 2 mpls ports.(TT + TS in the MPLS domain)
*         ePort B is port to MPLS domain with 2 labels
*         eport A is port to MPLS domain with 1 label. Label 1 is pop_swap.
*         eport D is port to MPLS domain with 1 label. Label 1 is popped & swap.
*/
GT_STATUS cstTunnelTerminationTransitMplsKnownUc2LabelsTest
(
    GT_VOID
)
{
#ifdef CHX_FAMILY

    GT_STATUS rc = GT_OK;

    {/* use tested ports , because {0, 18, 36, 58} are not always valid */
        GT_U32  ii;
        for(ii = 0; ii < 4 ; ii++)
        {
            phyPortArr[ii] = prvTgfPortsArray[ii];
        }
    }

    CHECK_RC_MAC(
        cstTunnelTerminationTransitMplsKnownUc2LabelsConfig());

    CHECK_RC_MAC(
        cstTunnelTerminationTransitMplsKnownUc2LabelsTrafficGenerate());

    CHECK_RC_MAC(
        cstTunnelTerminationTransitMplsKnownUc2LabelsRestore());

    return rc;
#else
    return GT_NOT_IMPLEMENTED;
#endif  /* CHX_FAMILY */
}

/**
* @internal cstTunnelTerminationTransitMplsKnownUc3LabelsConfig function
* @endinternal
*
* @brief   test configuration
*/
GT_STATUS cstTunnelTerminationTransitMplsKnownUc3LabelsConfig
(
    GT_VOID
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;

    eVlan = PRV_CST_MPLS_VLANID_CNS;

    ePortArr[0] = phyPortArr[0]  + 0x1000;
    ePortArr[1] = phyPortArr[1]  + 0x1000;
    ePortArr[2] = phyPortArr[2]  + 0x1000;
    ePortArr[3] = phyPortArr[3]  + 0x1000;

    globalFloodEvidx = 0;
    numOfLabels = 3;

    /* AUTODOC: get hw dev num */
    CHECK_RC_MAC(
    cpssDxChCfgHwDevNumGet(swDevNum,&hwDevNum));

    /* AUTODOC: set e2phy */
    CHECK_RC_MAC(
        prvCstSetE2Phy(phyPortArr,ePortArr));

    /* AUTODOC: set egress tagging */
    CHECK_RC_MAC(
        prvCstSetEgressTagging(ePortArr));

    /* AUTODOC: set MPLS tunnel start */
    CHECK_RC_MAC(
        prvCstSetUcMplsTunnelStart(phyPortArr,ePortArr));

    /* AUTODOC: set MPLS tunnel termination */
    CHECK_RC_MAC(
        prvCstSetMpls3LTunnelTermination(phyPortArr,ePortArr));

    /* AUTODOC: set bridge */
    CHECK_RC_MAC(
        prvCstSetBridge(phyPortArr,eVlan));

    return rc;
#else
    return GT_NOT_IMPLEMENTED;
#endif  /* CHX_FAMILY */
}

/**
* @internal cstTunnelTerminationTransitMplsKnownUc3LabelsTrafficGenerate function
* @endinternal
*
* @brief   generate traffic
*/
GT_STATUS cstTunnelTerminationTransitMplsKnownUc3LabelsTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

#ifdef CHX_FAMILY

    /* AUTODOC: test4: transit tunnel termination of known uc (3 labels manipulations) */
    /* AUTODOC: MPLS configuration of device with 4 port in MPLS domain. */
    /* AUTODOC: test check 3 Labels manipulations from mpls port to other 3 mpls ports.(TT + TS in the MPLS domain) */
    /* AUTODOC: ePort A is port to MPLS domain with 3 labels */
    /* AUTODOC: eport B is port to MPLS domain with 1 label. Labels 1,2 are pop2. */
    /* AUTODOC: eport C is port to MPLS domain with 1 label. Label 1 is pop1 & label 2 is pop1. */
    /* AUTODOC: eport D is port to MPLS domain with 1 label. Labels 1,2 are pop2 & label 2 is pushed. */

    PRV_UTF_LOG0_MAC("\ntest4: transit tunnel termination of known uc (3 labels manipulations)) \n");
    PRV_UTF_LOG0_MAC(" MPLS configuration of device with 4 port in MPLS domain. \n");
    PRV_UTF_LOG0_MAC(" test check 3 Labels manipulations from mpls port to other 3 mpls ports.(TT + TS in the MPLS domain) \n");
    PRV_UTF_LOG0_MAC(" ePort A is port to MPLS domain with 3 labels \n");
    PRV_UTF_LOG0_MAC(" eport B is port to MPLS domain with 1 label. Labels 1,2 are pop2. \n");
    PRV_UTF_LOG0_MAC(" eport C is port to MPLS domain with 1 label. Label 1 is pop1 & label 2 is pop1. \n");
    PRV_UTF_LOG0_MAC(" eport D is port to MPLS domain with 1 label. Labels 1,2 are pop2 & label 2 is pushed. \n");


    sendKnowUc(PRV_TGF_SEND_PORT_IDX_CNS/*sender*/,PRV_TGF_RECEIVE_PORT1_IDX_CNS/*receiver*/, 1/* packet number */);

    sendKnowUc(PRV_TGF_SEND_PORT_IDX_CNS/*sender*/,PRV_TGF_RECEIVE_PORT2_IDX_CNS/*receiver*/, 2/* packet number */);

    sendKnowUc(PRV_TGF_SEND_PORT_IDX_CNS/*sender*/,PRV_TGF_RECEIVE_PORT3_IDX_CNS/*receiver*/, 3/* packet number */);

    /* AUTODOC: test5: transit tunnel termination of known uc (3 labels manipulations) */
    /* AUTODOC: MPLS configuration of device with 4 port in MPLS domain. */
    /* AUTODOC: test check 3 Labels manipulations from mpls port to other 3 mpls ports.(TT + TS in the MPLS domain) */
    /* AUTODOC: ePort B is port to MPLS domain with 3 labels */
    /* AUTODOC: eport C is port to MPLS domain with 2 label. Label 1 is pop1. */
    /* AUTODOC: eport D is port to MPLS domain with 2 label. Label 1,2 are pop2 & new label 2 is push. */
    /* AUTODOC: eport A is port to MPLS domain with 2 label. Label 1 is pop1 & label 2 is pop_swap. */

    PRV_UTF_LOG0_MAC("\ntest5: transit tunnel termination of known uc (3 labels manipulations)) \n");
    PRV_UTF_LOG0_MAC(" MPLS configuration of device with 4 port in MPLS domain. \n");
    PRV_UTF_LOG0_MAC(" test check 3 Labels manipulations from mpls port to other 3 mpls ports.(TT + TS in the MPLS domain) \n");
    PRV_UTF_LOG0_MAC(" ePort B is port to MPLS domain with 3 labels \n");
    PRV_UTF_LOG0_MAC(" eport C is port to MPLS domain with 2 label. Label 1 is pop1. \n");
    PRV_UTF_LOG0_MAC(" eport D is port to MPLS domain with 2 label. Label 1,2 are pop2 & new label 2 is push. \n");
    PRV_UTF_LOG0_MAC(" eport A is port to MPLS domain with 2 label. Label 1 is pop1 & label 2 is pop_swap. \n");

    sendKnowUc(PRV_TGF_SEND_PORT1_IDX_CNS /*sender*/,PRV_TGF_RECEIVE_PORT2_IDX_CNS/*receiver*/, 4/* packet number */);

    sendKnowUc(PRV_TGF_SEND_PORT1_IDX_CNS/*sender*/,PRV_TGF_RECEIVE_PORT3_IDX_CNS/*receiver*/, 5/* packet number */);

    sendKnowUc(PRV_TGF_SEND_PORT1_IDX_CNS/*sender*/,PRV_TGF_RECEIVE_PORT_IDX_CNS/*receiver*/, 6/* packet number */);

#else
    rc = GT_NOT_IMPLEMENTED;
#endif  /* CHX_FAMILY */

    return rc;

}

/**
* @internal cstTunnelTerminationTransitMplsKnownUc3LabelsRestore function
* @endinternal
*
* @brief   restore test configuration
*/
GT_STATUS cstTunnelTerminationTransitMplsKnownUc3LabelsRestore
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

#ifdef CHX_FAMILY

    /* AUTODOC: unset e2phy */
    CHECK_RC_MAC(
        prvCstSetE2Phy_restore(ePortArr));

    /* AUTODOC: unset egress tagging */
    CHECK_RC_MAC(
        prvCstSetEgressTagging_restore(ePortArr));

    /* AUTODOC: unset MPLS tunnel start */
    CHECK_RC_MAC(
        prvCstSetUcMplsTunnelStart_restore(phyPortArr,ePortArr));

    /* AUTODOC: unset bridge */
    CHECK_RC_MAC(
        prvCstSetBridge_restore(eVlan));

    /* AUTODOC: unset MPLS tunnel termination */
    CHECK_RC_MAC(
        prvCstSetMplsTunnelTermination_restore(phyPortArr));

    /* AUTODOC: reset ETH counters */
    prvTgfEthCountersReset(prvTgfDevNum);

    /* AUTODOC: flush FDB include static entries */
    CHECK_RC_MAC(
        prvTgfBrgFdbFlush(GT_TRUE));

#else
    rc = GT_NOT_IMPLEMENTED;
#endif  /* CHX_FAMILY */

    return rc;
}

/**
* @internal cstTunnelTerminationTransitMplsKnownUc3LabelsTest function
* @endinternal
*
* @brief   test4: transit tunnel termination of known uc (3 labels manipulations)
*         MPLS configuration of device with 4 port in MPLS domain.
*         test check 3 Labels manipulations from mpls port to other 3 mpls ports.(TT + TS in the MPLS domain)
*         ePort A is port to MPLS domain with 3 labels
*         eport B is port to MPLS domain with 1 label. Labels 1,2 are pop2.
*         eport C is port to MPLS domain with 1 label. Label 1 is pop1 & label 2 is pop1.
*         eport D is port to MPLS domain with 1 label. Labels 1,2 are pop2 & label 2 is pushed.
*         test5: transit tunnel termination of known uc (3 labels manipulations)
*         MPLS configuration of device with 4 port in MPLS domain.
*         test check 3 Labels manipulations from mpls port to other 3 mpls ports.(TT + TS in the MPLS domain)
*         ePort B is port to MPLS domain with 3 labels
*         eport C is port to MPLS domain with 2 label. Label 1 is pop1.
*         eport D is port to MPLS domain with 2 label. Label 1,2 are pop2 & new label 2 is push.
*         eport A is port to MPLS domain with 2 label. Label 1 is pop1 & label 2 is pop_swap.
*/
GT_STATUS cstTunnelTerminationTransitMplsKnownUc3LabelsTest
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

#ifdef CHX_FAMILY

    {/* use tested ports , because {0, 18, 36, 58} are not always valid */
        GT_U32  ii;
        for(ii = 0; ii < 4 ; ii++)
        {
            phyPortArr[ii] = prvTgfPortsArray[ii];
        }
    }

    CHECK_RC_MAC(
        cstTunnelTerminationTransitMplsKnownUc3LabelsConfig());

    CHECK_RC_MAC(
        cstTunnelTerminationTransitMplsKnownUc3LabelsTrafficGenerate());

    CHECK_RC_MAC(
        cstTunnelTerminationTransitMplsKnownUc3LabelsRestore());

#else
    rc = GT_NOT_IMPLEMENTED;
#endif  /* CHX_FAMILY */

    return rc;
}


/**
* @internal cstTunnelTerminationTransitMplsKnownUc4LabelsConfig function
* @endinternal
*
* @brief   test configuration
*/
GT_STATUS cstTunnelTerminationTransitMplsKnownUc4LabelsConfig
(
    GT_VOID
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;

    eVlan = PRV_CST_MPLS_VLANID_CNS;

    ePortArr[0] = phyPortArr[0]  + 0x1000;
    ePortArr[1] = phyPortArr[1]  + 0x1000;
    ePortArr[2] = phyPortArr[2]  + 0x1000;
    ePortArr[3] = phyPortArr[3]  + 0x1000;

    globalFloodEvidx = 0;
    numOfLabels = 4;

    /* AUTODOC: get hw dev num */
    CHECK_RC_MAC(
    cpssDxChCfgHwDevNumGet(swDevNum,&hwDevNum));

    /* AUTODOC: set e2phy */
    CHECK_RC_MAC(
        prvCstSetE2Phy(phyPortArr,ePortArr));

    /* AUTODOC: set egress tagging */
    CHECK_RC_MAC(
        prvCstSetEgressTagging(ePortArr));

    /* AUTODOC: set MPLS tunnel start */
    CHECK_RC_MAC(
        prvCstSetUcMplsTunnelStart(phyPortArr,ePortArr));

    /* AUTODOC: set MPLS tunnel termination */
    CHECK_RC_MAC(
        prvCstSetMpls4LTunnelTermination(phyPortArr,ePortArr));

    /* AUTODOC: set bridge */
    CHECK_RC_MAC(
        prvCstSetBridge(phyPortArr,eVlan));

    return rc;

#else
    return GT_NOT_IMPLEMENTED;
#endif  /* CHX_FAMILY */
}

/**
* @internal cstTunnelTerminationTransitMplsKnownUc4LabelsTrafficGenerate function
* @endinternal
*
* @brief   generate traffic
*/
GT_STATUS cstTunnelTerminationTransitMplsKnownUc4LabelsTrafficGenerate
(
    GT_VOID
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;

    /* AUTODOC: test6: transit tunnel termination of known uc (4 labels manipulations)*/
    /* AUTODOC: MPLS configuration of device with 4 port in MPLS domain.*/
    /* AUTODOC: test check 4 Labels manipulations from mpls port to other 3 mpls ports.(TT + TS in the MPLS domain)*/
    /* AUTODOC: ePort A is port to MPLS domain with 4 labels.  */
    /* AUTODOC: eport B is port to MPLS domain with 1 label. Labels 1,2,3 are pop3 */
    /* AUTODOC: eport C is port to MPLS domain with 1 label. Label 1,2 are pop2 & label 3 is pop1 */
    /* AUTODOC: eport D is port to MPLS domain with 1 label. Label 1,2,3 are pop3 & pushed */

    PRV_UTF_LOG0_MAC("\ntest6: transit tunnel termination of known uc (4 labels manipulations) \n");
    PRV_UTF_LOG0_MAC("\nMPLS configuration of device with 4 port in MPLS domain. \n");
    PRV_UTF_LOG0_MAC("\ntest check 4 Labels manipulations from mpls port to other 3 mpls ports.(TT + TS in the MPLS domain) \n");
    PRV_UTF_LOG0_MAC(" ePort A is port to MPLS domain with 4 labels \n");
    PRV_UTF_LOG0_MAC(" eport B is port to MPLS domain with 1 label. Labels 1,2,3 are pop3 \n");
    PRV_UTF_LOG0_MAC(" eport C is port to MPLS domain with 1 label. Label 1,2 are pop2 & label 3 is pop1 \n");
    PRV_UTF_LOG0_MAC(" eport D is port to MPLS domain with 1 label. Label 1,2,3 are pop3 & pushed \n");

    sendKnowUc(PRV_TGF_SEND_PORT_IDX_CNS/*sender*/,PRV_TGF_RECEIVE_PORT1_IDX_CNS/*receiver*/, 1/* packet number */);

    sendKnowUc(PRV_TGF_SEND_PORT_IDX_CNS/*sender*/,PRV_TGF_RECEIVE_PORT2_IDX_CNS/*receiver*/, 2/* packet number */);

    sendKnowUc(PRV_TGF_SEND_PORT_IDX_CNS/*sender*/,PRV_TGF_RECEIVE_PORT3_IDX_CNS/*receiver*/, 3/* packet number */);

    return rc;
#else
    return GT_NOT_IMPLEMENTED;
#endif  /* CHX_FAMILY */
}

/**
* @internal cstTunnelTerminationTransitMplsKnownUc4LabelsRestore function
* @endinternal
*
* @brief   restore test configuration
*/
GT_STATUS cstTunnelTerminationTransitMplsKnownUc4LabelsRestore
(
    GT_VOID
)
{
#ifdef CHX_FAMILY

    GT_STATUS rc = GT_OK;

    /* AUTODOC: unset e2phy */
    CHECK_RC_MAC(
        prvCstSetE2Phy_restore(ePortArr));

    /* AUTODOC: unset egress tagging */
    CHECK_RC_MAC(
        prvCstSetEgressTagging_restore(ePortArr));

    /* AUTODOC: unset MPLS tunnel start */
    CHECK_RC_MAC(
        prvCstSetUcMplsTunnelStart_restore(phyPortArr,ePortArr));

    /* AUTODOC: unset bridge */
    CHECK_RC_MAC(
        prvCstSetBridge_restore(eVlan));

    /* AUTODOC: unset MPLS tunnel termination */
    CHECK_RC_MAC(
        prvCstSetMplsTunnelTermination_restore(phyPortArr));

    /* AUTODOC: reset ETH counters */
    prvTgfEthCountersReset(prvTgfDevNum);

    /* AUTODOC: flush FDB include static entries */
    CHECK_RC_MAC(
        prvTgfBrgFdbFlush(GT_TRUE));

    return rc;
#else
    return GT_NOT_IMPLEMENTED;
#endif  /* CHX_FAMILY */
}

/**
* @internal cstTunnelTerminationTransitMplsKnownUc4LabelsTest function
* @endinternal
*
* @brief   test6: transit tunnel termination of known uc (4 labels manipulations)
*         MPLS configuration of device with 4 port in MPLS domain.
*         test check 4 Labels manipulations from mpls port to other 3 mpls ports.(TT + TS in the MPLS domain)
*         ePort A is port to MPLS domain with 4 labels.
*         eport B is port to MPLS domain with 1 label. Labels 1,2,3 are pop3
*         eport C is port to MPLS domain with 1 label. Label 1,2 are pop2 & label 3 is pop1
*         eport D is port to MPLS domain with 1 label. Label 1,2,3 are pop3 & pushed
*/
GT_STATUS cstTunnelTerminationTransitMplsKnownUc4LabelsTest
(
    GT_VOID
)
{
#ifdef CHX_FAMILY

    GT_STATUS rc = GT_OK;

    /* to avoid compilation warnings.
       errorLine is used in CHECK_RC_MAC */
    TGF_PARAM_NOT_USED(errorLine);

    {/* use tested ports , because {0, 18, 36, 58} are not always valid */
        GT_U32  ii;
        for(ii = 0; ii < 4 ; ii++)
        {
            phyPortArr[ii] = prvTgfPortsArray[ii];
        }
    }

    CHECK_RC_MAC(
        cstTunnelTerminationTransitMplsKnownUc4LabelsConfig());

    CHECK_RC_MAC(
        cstTunnelTerminationTransitMplsKnownUc4LabelsTrafficGenerate());

    CHECK_RC_MAC(
        cstTunnelTerminationTransitMplsKnownUc4LabelsRestore());

    return rc;
#else
    return GT_NOT_IMPLEMENTED;
#endif  /* CHX_FAMILY */
}



