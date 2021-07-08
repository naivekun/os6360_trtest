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
* @file tgfBridgeGen.c
*
* @brief Generic API implementation for Bridge
*
* @version   146
********************************************************************************
*/
#ifdef COW_INTERNAL_COMPILE
    /* those 3 are defined , so remove them */
    #undef EXMXPM_FAMILY
    #undef EX_FAMILY
    #undef CHX_FAMILY

    /* define only the tested one */
    #define CHX_FAMILY

#endif /*COW_INTERNAL_COMPILE*/

/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/private/prvUtfHelpers.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>


#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/prvTgfCommonList.h>
#include <extUtils/auEventHandler/auFdbUpdateLock.h>

#ifdef CHX_FAMILY
    #include <bridge/prvTgfBasicDynamicLearning.h>
#endif /*CHX_FAMILY*/

#ifdef CHX_FAMILY
    #include <cpss/dxCh/dxChxGen/config/cpssDxChCfgInit.h>
    #include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwTables.h>
    #include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
    #include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgNestVlan.h>
    #include <cpss/dxCh/dxChxGen/tunnel/cpssDxChTunnel.h>
    #include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgStp.h>
    #include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgCount.h>
    #include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdb.h>
    #include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdbHash.h>
    #include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgEgrFlt.h>
    #include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgMc.h>
    #include <cpss/dxCh/dxChxGen/mirror/cpssDxChMirror.h>
    #include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgVlan.h>
    #include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgGen.h>
    #include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgSrcId.h>
    #include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgE2Phy.h>
    #include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgSecurityBreach.h>
    #include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdbRouting.h>
    #include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChErrataMng.h>

#endif /*CHX_FAMILY*/

#ifdef EXMXPM_FAMILY
    #include <cpss/exMxPm/exMxPmGen/config/cpssExMxPmCfg.h>
    #include <cpss/exMxPm/exMxPmGen/config/private/prvCpssExMxPmInfo.h>
    #include <cpss/exMxPm/exMxPmGen/bridge/cpssExMxPmBrgGen.h>
    #include <cpss/exMxPm/exMxPmGen/bridge/cpssExMxPmBrgFdb.h>
    #include <cpss/exMxPm/exMxPmGen/bridge/cpssExMxPmBrgNestVlan.h>
    #include <cpss/exMxPm/exMxPmGen/sct/cpssExMxPmSct.h>
    #include <cpss/exMxPm/exMxPmGen/inlif/cpssExMxPmInlif.h>
    #include <cpss/exMxPm/exMxPmGen/bridge/cpssExMxPmBrgStp.h>
    #include <cpss/exMxPm/exMxPmGen/bridge/cpssExMxPmBrgCount.h>
    #include <cpss/exMxPm/exMxPmGen/bridge/cpssExMxPmBrgFdbHash.h>
    #include <cpss/exMxPm/exMxPmGen/bridge/cpssExMxPmBrgEgrFlt.h>
    #include <cpss/exMxPm/exMxPmGen/bridge/cpssExMxPmBrgMc.h>
    #include <cpss/exMxPm/exMxPmGen/monitor/cpssExMxPmMirror.h>
    #include <cpss/exMxPm/exMxPmGen/bridge/cpssExMxPmBrgVlan.h>
    #include <cpss/exMxPm/exMxPmGen/bridge/cpssExMxPmBrgSrcId.h>
    #include <cpss/exMxPm/exMxPmGen/bridge/cpssExMxPmBrgSecurityBreach.h>
#endif /*EXMXPM_FAMILY*/

#include <trafficEngine/tgfTrafficEngine.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#ifdef CHX_FAMILY

extern GT_STATUS appDemoBrgFdbIpv6UcFreeAddrDataLinkedList
(
    IN GT_U8    devNum
);

#endif /*CHX_FAMILY*/

#define FDB_SHADOW_NOT_SUPPORT_FDB_HW_ITERATIONS                \
    if(prvTgfFdbShadowUsed == GT_TRUE)                          \
    {                                                           \
        cpssOsPrintf("Do not call under GM for FDB iterations");\
        return GT_NOT_SUPPORTED;                                \
    }

/******************************************************************************\
 *                           Private declarations                             *
\******************************************************************************/

/* did we initialized the local variables */
static GT_BOOL isInitialized = GT_FALSE;

/* default mac entry */
static PRV_TGF_BRG_MAC_ENTRY_STC   prvTgfDefMacEntry;

/* default vlan entry */
static PRV_TGF_BRG_VLAN_INFO_STC   prvTgfDefVlanInfo;

static PRV_TGF_BRG_FDB_ACCESS_MODE_ENT prvTgfBrgFdbAccessMode = PRV_TGF_BRG_FDB_ACCESS_MODE_BY_MESSAGE_E;

static GT_U32 prvTgfBrgFdbHashLengthLookup = 4;/* 4 index in bucket -- assume that 4 is the length of lookup */

/* FDB entry 'by message' must succeed */
static GT_BOOL  fdbEntryByMessageMustSucceed = GT_TRUE;
static GT_BOOL  fdbEntryByMessageMustCheck = GT_TRUE;

/* do we 'force' to ignore the SP settings by the test ? */
static GT_BOOL  forceIgnoreNaStormPrevention = GT_FALSE;

/* structure for saving default values from prvTgfBrgPortUnkSrcMacDropSet
and restore these values in prvTgfBrgPortUnkSrcMacDropInvalidate */
static struct
{
    GT_BOOL                 rstStatus;
#ifdef CHX_FAMILY
    GT_BOOL                 status;
    CPSS_PORT_LOCK_CMD_ENT  cmd;
#endif /*CHX_FAMILY*/

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_INLIF_ENTRY_STC     inlifEntry;
    CPSS_EXMXPM_INLIF_PORT_MODE_ENT inlifMode;
    CPSS_PACKET_CMD_ENT             command;
#endif /*EXMXPM_FAMILY*/                                         /*portIndex*/
} prvTgfPortUnkSrcMacDropRestoreCfg[PRV_CPSS_MAX_PP_DEVICES_CNS][PRV_TGF_PORTS_NUM_CNS] = {{{0}}};

/******************************************************************************\
 *                     Private function's implementation                      *
\******************************************************************************/

/* no initialized parameter value */
#define NOT_INIT_CNS    0xFFFFFFFF

/* Loopback configuration status */
extern GT_BOOL tgfLoopBackCt10And40Enable;

#ifdef IMPL_GALTIS
static GT_VOID prvTgfBrgSystemReset
(
    GT_VOID
);
#endif /*IMPL_GALTIS*/

/**
* @internal prvTgfBrgInit function
* @endinternal
*
* @brief   Initialize local default settings
*/
GT_VOID prvTgfBrgInit
(
    GT_VOID
)
{
    if(GT_TRUE == isInitialized)
    {
        return;
    }

#ifdef IMPL_GALTIS
        /* register function to reset DB so after system reset we can send traffic again */
        wrCpssRegisterResetCb(prvTgfBrgSystemReset);
#endif /*IMPL_GALTIS*/

    isInitialized = GT_TRUE;

    /* clear entry */
    cpssOsMemSet(&prvTgfDefMacEntry, 0, sizeof(prvTgfDefMacEntry));

    /* set default mac entry */
    prvTgfDefMacEntry.key.entryType            = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    prvTgfDefMacEntry.key.key.macVlan.vlanId   = 0;
    prvTgfDefMacEntry.dstInterface.type        = CPSS_INTERFACE_VIDX_E;
    prvTgfDefMacEntry.dstInterface.vidx        = 0;
    prvTgfDefMacEntry.isStatic                 = GT_FALSE;
    prvTgfDefMacEntry.daCommand                = PRV_TGF_PACKET_CMD_FORWARD_E;
    prvTgfDefMacEntry.saCommand                = PRV_TGF_PACKET_CMD_FORWARD_E;
    prvTgfDefMacEntry.daRoute                  = GT_FALSE;
    prvTgfDefMacEntry.mirrorToRxAnalyzerPortEn = GT_FALSE;
    prvTgfDefMacEntry.userDefined              = 0;
    prvTgfDefMacEntry.daQosIndex               = 0;
    prvTgfDefMacEntry.saQosIndex               = 0;
    prvTgfDefMacEntry.daSecurityLevel          = 0;
    prvTgfDefMacEntry.saSecurityLevel          = 0;
    prvTgfDefMacEntry.appSpecificCpuCode       = GT_FALSE;
    prvTgfDefMacEntry.pwId                     = 0;
    prvTgfDefMacEntry.spUnknown                = GT_FALSE;
    prvTgfDefMacEntry.sourceId                 = 0;

    /* clear entry */
    cpssOsMemSet(&prvTgfDefVlanInfo, 0, sizeof(prvTgfDefVlanInfo));

    /* set default Vlan info entry */
    prvTgfDefVlanInfo.cpuMember            = GT_FALSE;
    prvTgfDefVlanInfo.unkSrcAddrSecBreach  = GT_FALSE;
    prvTgfDefVlanInfo.unregNonIpMcastCmd   = CPSS_PACKET_CMD_FORWARD_E;
    prvTgfDefVlanInfo.unregIpv4McastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    prvTgfDefVlanInfo.unregIpv6McastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    prvTgfDefVlanInfo.unkUcastCmd          = CPSS_PACKET_CMD_FORWARD_E;
    prvTgfDefVlanInfo.unregIpv4BcastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    prvTgfDefVlanInfo.unregNonIpv4BcastCmd = CPSS_PACKET_CMD_FORWARD_E;
    prvTgfDefVlanInfo.ipv4IgmpToCpuEn      = GT_FALSE;
    prvTgfDefVlanInfo.mirrToRxAnalyzerEn   = GT_FALSE;
    prvTgfDefVlanInfo.mirrorToTxAnalyzerEn = GT_FALSE;
    prvTgfDefVlanInfo.ipv6IcmpToCpuEn      = GT_FALSE;
    prvTgfDefVlanInfo.ipCtrlToCpuEn        = PRV_TGF_BRG_IP_CTRL_NONE_E;
    prvTgfDefVlanInfo.ipv4IpmBrgMode       = CPSS_BRG_IPM_SGV_E;
    prvTgfDefVlanInfo.ipv6IpmBrgMode       = CPSS_BRG_IPM_SGV_E;
    prvTgfDefVlanInfo.ipv4IpmBrgEn         = GT_FALSE;
    prvTgfDefVlanInfo.ipv6IpmBrgEn         = GT_FALSE;
    prvTgfDefVlanInfo.ipv6SiteIdMode       = CPSS_IP_SITE_ID_INTERNAL_E;
    prvTgfDefVlanInfo.ipv4UcastRouteEn     = GT_FALSE;
    prvTgfDefVlanInfo.ipv4McastRouteEn     = GT_FALSE;
    prvTgfDefVlanInfo.ipv6UcastRouteEn     = GT_FALSE;
    prvTgfDefVlanInfo.ipv6McastRouteEn     = GT_FALSE;
    prvTgfDefVlanInfo.stgId                = 0;
    prvTgfDefVlanInfo.autoLearnDisable     = GT_TRUE;
    prvTgfDefVlanInfo.naMsgToCpuEn         = GT_TRUE;
    prvTgfDefVlanInfo.mruIdx               = 0;
    prvTgfDefVlanInfo.bcastUdpTrapMirrEn   = GT_FALSE;
    prvTgfDefVlanInfo.vrfId                = 0;
    prvTgfDefVlanInfo.egressUnregMcFilterCmd = CPSS_UNREG_MC_VLAN_FRWD_E;
    prvTgfDefVlanInfo.floodVidx            = 0xFFF;/* flood into vlan */

    if(forceIgnoreNaStormPrevention == GT_FALSE)
    {
        prvTgfBrgFdbNaStormPreventAllSet(GT_TRUE);
    }
}

/*******************************************************************************
* prvTgfBrgSystemReset
*
* DESCRIPTION:
*       Preparation for system reset
*
* INPUTS:
*       None
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK        - on success
*       GT_FAIL      - on error
*       GT_HW_ERROR  - on hardware error
*       GT_BAD_PARAM - wrong interface
*       GT_BAD_PTR   - on NULL pointer
*
* COMMENTS:
*
*******************************************************************************/
#ifdef IMPL_GALTIS
static GT_VOID prvTgfBrgSystemReset
(
    GT_VOID
)
{
    isInitialized = GT_FALSE;

    prvTgfBrgFdbAccessMode = PRV_TGF_BRG_FDB_ACCESS_MODE_BY_MESSAGE_E;

    prvTgfBrgFdbHashLengthLookup = 4;/* 4 index in bucket -- assume that 4 is the length of lookup */

    /* FDB entry 'by message' must succeed */
    fdbEntryByMessageMustSucceed = GT_TRUE;
    fdbEntryByMessageMustCheck = GT_TRUE;

    /* do we 'force' to ignore the SP settings by the test ? */
    forceIgnoreNaStormPrevention = GT_FALSE;
}
#endif /*IMPL_GALTIS*/

#ifdef CHX_FAMILY


/**
* @internal prvTgfConvertGenericToDxChDropCntMode function
* @endinternal
*
* @brief   Convert generic drop count mode into device specific drop count mode
*
* @param[in] dropMode                 - drop counter mode
*
* @param[out] dxChDropModePtr          - (pointer to) DxCh drop counter mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChDropCntMode
(
    IN  PRV_TGF_BRG_DROP_CNTR_MODE_ENT       dropMode,
    OUT CPSS_DXCH_BRIDGE_DROP_CNTR_MODE_ENT *dxChDropModePtr
)
{
    /* convert drop counter mode into device specific format */
    switch (dropMode)
    {
        PRV_TGF_SWITCH_CASE_MAC(*dxChDropModePtr, PRV_TGF_BRG_DROP_CNTR_COUNT_ALL_E,
                                CPSS_DXCH_BRG_DROP_CNTR_COUNT_ALL_E);
        PRV_TGF_SWITCH_CASE_MAC(*dxChDropModePtr, PRV_TGF_BRG_DROP_CNTR_FDB_ENTRY_CMD_E,
                                CPSS_DXCH_BRG_DROP_CNTR_FDB_ENTRY_CMD_E);
        PRV_TGF_SWITCH_CASE_MAC(*dxChDropModePtr, PRV_TGF_BRG_DROP_CNTR_UNKNOWN_MAC_SA_E,
                                CPSS_DXCH_BRG_DROP_CNTR_UNKNOWN_MAC_SA_E);
        PRV_TGF_SWITCH_CASE_MAC(*dxChDropModePtr, PRV_TGF_BRG_DROP_CNTR_INVALID_SA_E,
                                CPSS_DXCH_BRG_DROP_CNTR_INVALID_SA_E);
        PRV_TGF_SWITCH_CASE_MAC(*dxChDropModePtr, PRV_TGF_BRG_DROP_CNTR_INVALID_VLAN_E,
                                CPSS_DXCH_BRG_DROP_CNTR_INVALID_VLAN_E);
        PRV_TGF_SWITCH_CASE_MAC(*dxChDropModePtr, PRV_TGF_BRG_DROP_CNTR_PORT_NOT_IN_VLAN_E,
                                CPSS_DXCH_BRG_DROP_CNTR_PORT_NOT_IN_VLAN_E);
        PRV_TGF_SWITCH_CASE_MAC(*dxChDropModePtr, PRV_TGF_BRG_DROP_CNTR_VLAN_RANGE_E,
                                CPSS_DXCH_BRG_DROP_CNTR_VLAN_RANGE_E);
        PRV_TGF_SWITCH_CASE_MAC(*dxChDropModePtr, PRV_TGF_BRG_DROP_CNTR_MOVED_STATIC_ADDR_E,
                                CPSS_DXCH_BRG_DROP_CNTR_MOVED_STATIC_ADDR_E);
        PRV_TGF_SWITCH_CASE_MAC(*dxChDropModePtr, PRV_TGF_BRG_DROP_CNTR_ARP_SA_MISMATCH_E,
                                CPSS_DXCH_BRG_DROP_CNTR_ARP_SA_MISMATCH_E);
        PRV_TGF_SWITCH_CASE_MAC(*dxChDropModePtr, PRV_TGF_BRG_DROP_CNTR_SYN_WITH_DATA_E,
                                CPSS_DXCH_BRG_DROP_CNTR_SYN_WITH_DATA_E);
        PRV_TGF_SWITCH_CASE_MAC(*dxChDropModePtr, PRV_TGF_BRG_DROP_CNTR_TCP_OVER_MC_OR_BC_E,
                                CPSS_DXCH_BRG_DROP_CNTR_TCP_OVER_MC_OR_BC_E);
        PRV_TGF_SWITCH_CASE_MAC(*dxChDropModePtr, PRV_TGF_BRG_DROP_CNTR_ACCESS_MATRIX_E,
                                CPSS_DXCH_BRG_DROP_CNTR_ACCESS_MATRIX_E);
        PRV_TGF_SWITCH_CASE_MAC(*dxChDropModePtr, PRV_TGF_BRG_DROP_CNTR_SEC_LEARNING_E,
                                CPSS_DXCH_BRG_DROP_CNTR_SEC_LEARNING_E);
        PRV_TGF_SWITCH_CASE_MAC(*dxChDropModePtr, PRV_TGF_BRG_DROP_CNTR_ACCEPT_FRAME_TYPE_E,
                                CPSS_DXCH_BRG_DROP_CNTR_ACCEPT_FRAME_TYPE_E);
        PRV_TGF_SWITCH_CASE_MAC(*dxChDropModePtr, PRV_TGF_BRG_DROP_CNTR_FRAG_ICMP_E,
                                CPSS_DXCH_BRG_DROP_CNTR_FRAG_ICMP_E);
        PRV_TGF_SWITCH_CASE_MAC(*dxChDropModePtr, PRV_TGF_BRG_DROP_CNTR_TCP_FLAGS_ZERO_E,
                                CPSS_DXCH_BRG_DROP_CNTR_TCP_FLAGS_ZERO_E);
        PRV_TGF_SWITCH_CASE_MAC(*dxChDropModePtr, PRV_TGF_BRG_DROP_CNTR_TCP_FLAGS_FUP_SET_E,
                                CPSS_DXCH_BRG_DROP_CNTR_TCP_FLAGS_FUP_SET_E);
        PRV_TGF_SWITCH_CASE_MAC(*dxChDropModePtr, PRV_TGF_BRG_DROP_CNTR_TCP_FLAGS_SF_SET_E,
                                CPSS_DXCH_BRG_DROP_CNTR_TCP_FLAGS_SF_SET_E);
        PRV_TGF_SWITCH_CASE_MAC(*dxChDropModePtr, PRV_TGF_BRG_DROP_CNTR_TCP_FLAGS_SR_SET_E,
                                CPSS_DXCH_BRG_DROP_CNTR_TCP_FLAGS_SR_SET_E);
        PRV_TGF_SWITCH_CASE_MAC(*dxChDropModePtr, PRV_TGF_BRG_DROP_CNTR_TCP_UDP_PORT_ZERO_E,
                                CPSS_DXCH_BRG_DROP_CNTR_TCP_UDP_PORT_ZERO_E);
        PRV_TGF_SWITCH_CASE_MAC(*dxChDropModePtr, PRV_TGF_BRG_DROP_CNTR_VLAN_MRU_E,
                                CPSS_DXCH_BRG_DROP_CNTR_VLAN_MRU_E);
        PRV_TGF_SWITCH_CASE_MAC(*dxChDropModePtr, PRV_TGF_BRG_DROP_CNTR_RATE_LIMIT_E,
                                CPSS_DXCH_BRG_DROP_CNTR_RATE_LIMIT_E);
        PRV_TGF_SWITCH_CASE_MAC(*dxChDropModePtr, PRV_TGF_BRG_DROP_CNTR_LOCAL_PORT_E,
                                CPSS_DXCH_BRG_DROP_CNTR_LOCAL_PORT_E);
        PRV_TGF_SWITCH_CASE_MAC(*dxChDropModePtr, PRV_TGF_BRG_DROP_CNTR_SPAN_TREE_PORT_ST_E,
                                CPSS_DXCH_BRG_DROP_CNTR_SPAN_TREE_PORT_ST_E);
        PRV_TGF_SWITCH_CASE_MAC(*dxChDropModePtr, PRV_TGF_BRG_DROP_CNTR_IP_MC_E,
                                CPSS_DXCH_BRG_DROP_CNTR_IP_MC_E);
        PRV_TGF_SWITCH_CASE_MAC(*dxChDropModePtr, PRV_TGF_BRG_DROP_CNTR_NON_IP_MC_E,
                                CPSS_DXCH_BRG_DROP_CNTR_NON_IP_MC_E);
        PRV_TGF_SWITCH_CASE_MAC(*dxChDropModePtr, PRV_TGF_BRG_DROP_CNTR_DSATAG_LOCAL_DEV_E,
                                CPSS_DXCH_BRG_DROP_CNTR_DSATAG_LOCAL_DEV_E);
        PRV_TGF_SWITCH_CASE_MAC(*dxChDropModePtr, PRV_TGF_BRG_DROP_CNTR_IEEE_RESERVED_E,
                                CPSS_DXCH_BRG_DROP_CNTR_IEEE_RESERVED_E);
        PRV_TGF_SWITCH_CASE_MAC(*dxChDropModePtr, PRV_TGF_BRG_DROP_CNTR_UREG_L2_NON_IPM_MC_E,
                                CPSS_DXCH_BRG_DROP_CNTR_UREG_L2_NON_IPM_MC_E);
        PRV_TGF_SWITCH_CASE_MAC(*dxChDropModePtr, PRV_TGF_BRG_DROP_CNTR_UREG_L2_IPV6_MC_E,
                                CPSS_DXCH_BRG_DROP_CNTR_UREG_L2_IPV6_MC_E);
        PRV_TGF_SWITCH_CASE_MAC(*dxChDropModePtr, PRV_TGF_BRG_DROP_CNTR_UREG_L2_IPV4_MC_E,
                                CPSS_DXCH_BRG_DROP_CNTR_UREG_L2_IPV4_MC_E);
        PRV_TGF_SWITCH_CASE_MAC(*dxChDropModePtr, PRV_TGF_BRG_DROP_CNTR_UNKNOWN_L2_UC_E,
                                CPSS_DXCH_BRG_DROP_CNTR_UNKNOWN_L2_UC_E);
        PRV_TGF_SWITCH_CASE_MAC(*dxChDropModePtr, PRV_TGF_BRG_DROP_CNTR_UREG_L2_IPV4_BC_E,
                                CPSS_DXCH_BRG_DROP_CNTR_UREG_L2_IPV4_BC_E);
        PRV_TGF_SWITCH_CASE_MAC(*dxChDropModePtr, PRV_TGF_BRG_DROP_CNTR_UREG_L2_NON_IPV4_BC_E,
                                CPSS_DXCH_BRG_DROP_CNTR_UREG_L2_NON_IPV4_BC_E);

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal prvTgfConvertDxChToGenericDropCntMode function
* @endinternal
*
* @brief   Convert generic drop count mode from device specific drop count mode
*
* @param[in] dxChDropMode             - DxCh drop counter mode
*
* @param[out] dropModePtr              - (pointer to) drop counter mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertDxChToGenericDropCntMode
(
    IN  CPSS_DXCH_BRIDGE_DROP_CNTR_MODE_ENT  dxChDropMode,
    OUT PRV_TGF_BRG_DROP_CNTR_MODE_ENT      *dropModePtr
)
{
    /* convert drop counter mode from device specific format */
    switch (dxChDropMode)
    {
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_DXCH_BRG_DROP_CNTR_COUNT_ALL_E,
                                PRV_TGF_BRG_DROP_CNTR_COUNT_ALL_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_DXCH_BRG_DROP_CNTR_FDB_ENTRY_CMD_E,
                                PRV_TGF_BRG_DROP_CNTR_FDB_ENTRY_CMD_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_DXCH_BRG_DROP_CNTR_UNKNOWN_MAC_SA_E,
                                PRV_TGF_BRG_DROP_CNTR_UNKNOWN_MAC_SA_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_DXCH_BRG_DROP_CNTR_INVALID_SA_E,
                                PRV_TGF_BRG_DROP_CNTR_INVALID_SA_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_DXCH_BRG_DROP_CNTR_INVALID_VLAN_E,
                                PRV_TGF_BRG_DROP_CNTR_INVALID_VLAN_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_DXCH_BRG_DROP_CNTR_PORT_NOT_IN_VLAN_E,
                                PRV_TGF_BRG_DROP_CNTR_PORT_NOT_IN_VLAN_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_DXCH_BRG_DROP_CNTR_VLAN_RANGE_E,
                                PRV_TGF_BRG_DROP_CNTR_VLAN_RANGE_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_DXCH_BRG_DROP_CNTR_MOVED_STATIC_ADDR_E,
                                PRV_TGF_BRG_DROP_CNTR_MOVED_STATIC_ADDR_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_DXCH_BRG_DROP_CNTR_ARP_SA_MISMATCH_E,
                                PRV_TGF_BRG_DROP_CNTR_ARP_SA_MISMATCH_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_DXCH_BRG_DROP_CNTR_SYN_WITH_DATA_E,
                                PRV_TGF_BRG_DROP_CNTR_SYN_WITH_DATA_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_DXCH_BRG_DROP_CNTR_TCP_OVER_MC_OR_BC_E,
                                PRV_TGF_BRG_DROP_CNTR_TCP_OVER_MC_OR_BC_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_DXCH_BRG_DROP_CNTR_ACCESS_MATRIX_E,
                                PRV_TGF_BRG_DROP_CNTR_ACCESS_MATRIX_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_DXCH_BRG_DROP_CNTR_SEC_LEARNING_E,
                                PRV_TGF_BRG_DROP_CNTR_SEC_LEARNING_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_DXCH_BRG_DROP_CNTR_ACCEPT_FRAME_TYPE_E,
                                PRV_TGF_BRG_DROP_CNTR_ACCEPT_FRAME_TYPE_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_DXCH_BRG_DROP_CNTR_FRAG_ICMP_E,
                                PRV_TGF_BRG_DROP_CNTR_FRAG_ICMP_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_DXCH_BRG_DROP_CNTR_TCP_FLAGS_ZERO_E,
                                PRV_TGF_BRG_DROP_CNTR_TCP_FLAGS_ZERO_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_DXCH_BRG_DROP_CNTR_TCP_FLAGS_FUP_SET_E,
                                PRV_TGF_BRG_DROP_CNTR_TCP_FLAGS_FUP_SET_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_DXCH_BRG_DROP_CNTR_TCP_FLAGS_SF_SET_E,
                                PRV_TGF_BRG_DROP_CNTR_TCP_FLAGS_SF_SET_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_DXCH_BRG_DROP_CNTR_TCP_FLAGS_SR_SET_E,
                                PRV_TGF_BRG_DROP_CNTR_TCP_FLAGS_SR_SET_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_DXCH_BRG_DROP_CNTR_TCP_UDP_PORT_ZERO_E,
                                PRV_TGF_BRG_DROP_CNTR_TCP_UDP_PORT_ZERO_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_DXCH_BRG_DROP_CNTR_VLAN_MRU_E,
                                PRV_TGF_BRG_DROP_CNTR_VLAN_MRU_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_DXCH_BRG_DROP_CNTR_RATE_LIMIT_E,
                                PRV_TGF_BRG_DROP_CNTR_RATE_LIMIT_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_DXCH_BRG_DROP_CNTR_LOCAL_PORT_E,
                                PRV_TGF_BRG_DROP_CNTR_LOCAL_PORT_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_DXCH_BRG_DROP_CNTR_SPAN_TREE_PORT_ST_E,
                                PRV_TGF_BRG_DROP_CNTR_SPAN_TREE_PORT_ST_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_DXCH_BRG_DROP_CNTR_IP_MC_E,
                                PRV_TGF_BRG_DROP_CNTR_IP_MC_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_DXCH_BRG_DROP_CNTR_NON_IP_MC_E,
                                PRV_TGF_BRG_DROP_CNTR_NON_IP_MC_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_DXCH_BRG_DROP_CNTR_DSATAG_LOCAL_DEV_E,
                                PRV_TGF_BRG_DROP_CNTR_DSATAG_LOCAL_DEV_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_DXCH_BRG_DROP_CNTR_IEEE_RESERVED_E,
                                PRV_TGF_BRG_DROP_CNTR_IEEE_RESERVED_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_DXCH_BRG_DROP_CNTR_UREG_L2_NON_IPM_MC_E,
                                PRV_TGF_BRG_DROP_CNTR_UREG_L2_NON_IPM_MC_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_DXCH_BRG_DROP_CNTR_UREG_L2_IPV6_MC_E,
                                PRV_TGF_BRG_DROP_CNTR_UREG_L2_IPV6_MC_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_DXCH_BRG_DROP_CNTR_UREG_L2_IPV4_MC_E,
                                PRV_TGF_BRG_DROP_CNTR_UREG_L2_IPV4_MC_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_DXCH_BRG_DROP_CNTR_UNKNOWN_L2_UC_E,
                                PRV_TGF_BRG_DROP_CNTR_UNKNOWN_L2_UC_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_DXCH_BRG_DROP_CNTR_UREG_L2_IPV4_BC_E,
                                PRV_TGF_BRG_DROP_CNTR_UREG_L2_IPV4_BC_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_DXCH_BRG_DROP_CNTR_UREG_L2_NON_IPV4_BC_E,
                                PRV_TGF_BRG_DROP_CNTR_UREG_L2_NON_IPV4_BC_E);

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToDxChVlanIndexingMode function
* @endinternal
*
* @brief   Convert generic Vlan tables access indexing mode into device specific mode.
*
* @param[in] mode                     - generic Vlan tables access indexing mode
*
* @param[out] dxChModePtr              - (pointer to) DxCh Vlan tables access indexing mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChVlanIndexingMode
(
    IN  PRV_TGF_BRG_VLAN_TBL_INDEX_MODE_ENT     mode,
    OUT CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT   *dxChModePtr
)
{
    switch (mode)
    {
        PRV_TGF_SWITCH_CASE_MAC(*dxChModePtr, PRV_TGF_BRG_VLAN_TBL_INDEX_MODE_VLAN_E,
                                CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VLAN_E);
        PRV_TGF_SWITCH_CASE_MAC(*dxChModePtr, PRV_TGF_BRG_VLAN_TBL_INDEX_MODE_VID1_E,
                                CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VID1_E);

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal prvTgfConvertDxChToGenericVlanIndexingMode function
* @endinternal
*
* @brief   Convert device specific Vlan tables access indexing mode into generic mode.
*
* @param[in] dxChMode                 - DxCh Vlan tables access indexing mode
*
* @param[out] modePtr                  - (pointer to) generic Vlan tables access indexing mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertDxChToGenericVlanIndexingMode
(
    IN  CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT   dxChMode,
    OUT PRV_TGF_BRG_VLAN_TBL_INDEX_MODE_ENT     *modePtr
)
{
    switch (dxChMode)
    {
        PRV_TGF_SWITCH_CASE_MAC(*modePtr, CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VLAN_E,
                                PRV_TGF_BRG_VLAN_TBL_INDEX_MODE_VLAN_E);
        PRV_TGF_SWITCH_CASE_MAC(*modePtr, CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VID1_E,
                                PRV_TGF_BRG_VLAN_TBL_INDEX_MODE_VID1_E);

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToDxChBrgBypassMode function
* @endinternal
*
* @brief   Convert generic Bridge Bypass mode into device specific mode.
*
* @param[in] mode                     - generic Bridge Bypass mode
*
* @param[out] dxChModePtr              - (pointer to) DxCh Bridge Bypass mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChBrgBypassMode
(
    IN  PRV_TGF_BRG_GEN_BYPASS_MODE_ENT     mode,
    OUT CPSS_DXCH_BRG_GEN_BYPASS_MODE_ENT   *dxChModePtr
)
{
    switch (mode)
    {
        PRV_TGF_SWITCH_CASE_MAC(*dxChModePtr,
                                PRV_TGF_BRG_GEN_BYPASS_MODE_ALL_EXCEPT_SA_LEARNING_E,
                                CPSS_DXCH_BRG_GEN_BYPASS_MODE_ALL_EXCEPT_SA_LEARNING_E);
        PRV_TGF_SWITCH_CASE_MAC(*dxChModePtr,
                                PRV_TGF_BRG_GEN_BYPASS_MODE_ONLY_FORWARDING_DECISION_E,
                                CPSS_DXCH_BRG_GEN_BYPASS_MODE_ONLY_FORWARDING_DECISION_E);

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal prvTgfConvertDxChToGenericBrgBypassMode function
* @endinternal
*
* @brief   Convert device specific Bridge Bypass mode into generic mode.
*
* @param[in] dxChMode                 - Bridge Bypass mode
*
* @param[out] modePtr                  - (pointer to) generic Bridge Bypass mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertDxChToGenericBrgBypassMode
(
    IN  CPSS_DXCH_BRG_GEN_BYPASS_MODE_ENT   dxChMode,
    OUT PRV_TGF_BRG_GEN_BYPASS_MODE_ENT     *modePtr
)
{
    switch (dxChMode)
    {
        PRV_TGF_SWITCH_CASE_MAC(*modePtr,
                                CPSS_DXCH_BRG_GEN_BYPASS_MODE_ALL_EXCEPT_SA_LEARNING_E,
                                PRV_TGF_BRG_GEN_BYPASS_MODE_ALL_EXCEPT_SA_LEARNING_E);
        PRV_TGF_SWITCH_CASE_MAC(*modePtr,
                                CPSS_DXCH_BRG_GEN_BYPASS_MODE_ONLY_FORWARDING_DECISION_E,
                                PRV_TGF_BRG_GEN_BYPASS_MODE_ONLY_FORWARDING_DECISION_E);

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToDxChBrgMtuByteCountMode function
* @endinternal
*
* @brief   Convert generic Bridge eport target MTU byte count mode into device specific mode.
*
* @param[in] mode                     - generic target eport MTU byte count mode
*
* @param[out] dxChModePtr              - (pointer to) DxCh target eport MTU byte count mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChBrgMtuByteCountMode
(
    IN  PRV_TGF_BRG_GEN_BYTE_COUNT_MODE_MTU_ENT     mode,
    OUT CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_ENT   *dxChModePtr
)
{
    switch (mode)
    {
        PRV_TGF_SWITCH_CASE_MAC(*dxChModePtr,
                                PRV_TGF_BRG_GEN_BYTE_COUNT_MODE_MTU_L3_E,
                                CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_L3_E);
        PRV_TGF_SWITCH_CASE_MAC(*dxChModePtr,
                                PRV_TGF_BRG_GEN_BYTE_COUNT_MODE_MTU_L2_E,
                                CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_L2_E);

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal prvTgfConvertDxChToGenericBrgMtuByteCountMode function
* @endinternal
*
* @brief   Convert device specific MTU byte count mode into generic mode.
*
* @param[in] dxChMode                 - target ePort MTU byte count mode
*
* @param[out] modePtr                  - (pointer to) generic MTU byte count mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertDxChToGenericBrgMtuByteCountMode
(
    IN  CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_ENT   dxChMode,
    OUT PRV_TGF_BRG_GEN_BYTE_COUNT_MODE_MTU_ENT     *modePtr
)
{
    switch (dxChMode)
    {
        PRV_TGF_SWITCH_CASE_MAC(*modePtr,
                                CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_L3_E,
                                PRV_TGF_BRG_GEN_BYTE_COUNT_MODE_MTU_L3_E);
        PRV_TGF_SWITCH_CASE_MAC(*modePtr,
                                CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_L2_E,
                                PRV_TGF_BRG_GEN_BYTE_COUNT_MODE_MTU_L2_E);

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

/**
* @internal prvTgfConvertGenericToExMxPmMacEntryKey function
* @endinternal
*
* @brief   Convert generic MAC entry key into device specific MAC entry key
*
* @param[in] macEntryKeyPtr           - (pointer to) MAC entry key parameters
*
* @param[out] exMxPmMacEntryKeyPtr     - (pointer to) ExMxPm MAC entry key parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToExMxPmMacEntryKey
(
    IN  PRV_TGF_MAC_ENTRY_KEY_STC       *macEntryKeyPtr,
    OUT CPSS_EXMXPM_FDB_ENTRY_KEY_STC   *exMxPmMacEntryKeyPtr
)
{
    cpssOsMemSet(exMxPmMacEntryKeyPtr, 0, sizeof(*exMxPmMacEntryKeyPtr));

    /* convert entry type into device specific format */
    switch (macEntryKeyPtr->entryType)
    {
        case PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E:
            exMxPmMacEntryKeyPtr->entryType = CPSS_EXMXPM_FDB_ENTRY_TYPE_MAC_ADDR_E;
            break;

        case PRV_TGF_FDB_ENTRY_TYPE_IPV4_MCAST_E:
            exMxPmMacEntryKeyPtr->entryType = CPSS_EXMXPM_FDB_ENTRY_TYPE_IPV4_MCAST_E;
            break;

        case PRV_TGF_FDB_ENTRY_TYPE_IPV6_MCAST_E:
            exMxPmMacEntryKeyPtr->entryType = CPSS_EXMXPM_FDB_ENTRY_TYPE_IPV6_MCAST_E;
            break;

        case PRV_TGF_FDB_ENTRY_TYPE_VPLS_E:
            exMxPmMacEntryKeyPtr->entryType = CPSS_EXMXPM_FDB_ENTRY_TYPE_VPLS_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert key data into device specific format */
    if ((PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E == macEntryKeyPtr->entryType) ||
        (PRV_TGF_FDB_ENTRY_TYPE_VPLS_E == macEntryKeyPtr->entryType))
    {
        cpssOsMemCpy(exMxPmMacEntryKeyPtr->key.macFid.macAddr.arEther,
                     macEntryKeyPtr->key.macVlan.macAddr.arEther,
                     sizeof(exMxPmMacEntryKeyPtr->key.macFid.macAddr));
        exMxPmMacEntryKeyPtr->key.macFid.fId = macEntryKeyPtr->key.macVlan.vlanId;
    }
    else
    {
        cpssOsMemCpy(exMxPmMacEntryKeyPtr->key.ipMcast.sip,
                     macEntryKeyPtr->key.ipMcast.sip,
                     sizeof(exMxPmMacEntryKeyPtr->key.ipMcast.sip));
        cpssOsMemCpy(exMxPmMacEntryKeyPtr->key.ipMcast.dip,
                     macEntryKeyPtr->key.ipMcast.dip,
                     sizeof(exMxPmMacEntryKeyPtr->key.ipMcast.dip));
        exMxPmMacEntryKeyPtr->key.ipMcast.fId = macEntryKeyPtr->key.ipMcast.vlanId;
    }

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToExMxPmMacEntry function
* @endinternal
*
* @brief   Convert generic MAC entry into device specific MAC entry
*
* @param[in] macEntryPtr              - (pointer to) MAC entry parameters
*
* @param[out] exMxPmMacEntryPtr        - (pointer to) ExMxPm MAC entry parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToExMxPmMacEntry
(
    IN  PRV_TGF_BRG_MAC_ENTRY_STC    *macEntryPtr,
    OUT CPSS_EXMXPM_FDB_ENTRY_STC    *exMxPmMacEntryPtr
)
{
    cpssOsMemSet(exMxPmMacEntryPtr,0,sizeof(CPSS_EXMXPM_FDB_ENTRY_STC));

    /* convert entry type into device specific format */
    switch (macEntryPtr->key.entryType)
    {
        case PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E:
            exMxPmMacEntryPtr->key.entryType = CPSS_EXMXPM_FDB_ENTRY_TYPE_MAC_ADDR_E;
            break;

        case PRV_TGF_FDB_ENTRY_TYPE_IPV4_MCAST_E:
            exMxPmMacEntryPtr->key.entryType = CPSS_EXMXPM_FDB_ENTRY_TYPE_IPV4_MCAST_E;
            break;

        case PRV_TGF_FDB_ENTRY_TYPE_IPV6_MCAST_E:
            exMxPmMacEntryPtr->key.entryType = CPSS_EXMXPM_FDB_ENTRY_TYPE_IPV6_MCAST_E;
            break;

        case PRV_TGF_FDB_ENTRY_TYPE_VPLS_E:
            exMxPmMacEntryPtr->key.entryType = CPSS_EXMXPM_FDB_ENTRY_TYPE_VPLS_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert key data into device specific format */
    if ((PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E == macEntryPtr->key.entryType) ||
        (PRV_TGF_FDB_ENTRY_TYPE_VPLS_E == macEntryPtr->key.entryType))
    {
        cpssOsMemCpy(exMxPmMacEntryPtr->key.key.macFid.macAddr.arEther,
                     macEntryPtr->key.key.macVlan.macAddr.arEther,
                     sizeof(exMxPmMacEntryPtr->key.key.macFid.macAddr));
        exMxPmMacEntryPtr->key.key.macFid.fId = macEntryPtr->key.key.macVlan.vlanId;
    }
    else
    {
        cpssOsMemCpy(exMxPmMacEntryPtr->key.key.ipMcast.sip,
                     macEntryPtr->key.key.ipMcast.sip,
                     sizeof(exMxPmMacEntryPtr->key.key.ipMcast.sip));
        cpssOsMemCpy(exMxPmMacEntryPtr->key.key.ipMcast.dip,
                     macEntryPtr->key.key.ipMcast.dip,
                     sizeof(exMxPmMacEntryPtr->key.key.ipMcast.dip));
        exMxPmMacEntryPtr->key.key.ipMcast.fId = macEntryPtr->key.key.ipMcast.vlanId;
    }

    /* convert interface info into device specific format */
    exMxPmMacEntryPtr->dstOutlif.interfaceInfo.type = macEntryPtr->dstInterface.type;
    switch (exMxPmMacEntryPtr->dstOutlif.interfaceInfo.type)
    {
        case CPSS_INTERFACE_PORT_E:
            exMxPmMacEntryPtr->dstOutlif.interfaceInfo.devPort.hwDevNum  = macEntryPtr->dstInterface.devPort.hwDevNum;
            exMxPmMacEntryPtr->dstOutlif.interfaceInfo.devPort.portNum = macEntryPtr->dstInterface.devPort.portNum;
            break;

        case CPSS_INTERFACE_TRUNK_E:
            exMxPmMacEntryPtr->dstOutlif.interfaceInfo.trunkId = macEntryPtr->dstInterface.trunkId;
            break;

        case CPSS_INTERFACE_VIDX_E:
            exMxPmMacEntryPtr->dstOutlif.interfaceInfo.vidx = macEntryPtr->dstInterface.vidx;
            break;

        case CPSS_INTERFACE_VID_E:
            exMxPmMacEntryPtr->dstOutlif.interfaceInfo.vlanId = macEntryPtr->dstInterface.vlanId;
            break;

        case CPSS_INTERFACE_DEVICE_E:
            exMxPmMacEntryPtr->dstOutlif.interfaceInfo.hwDevNum = macEntryPtr->dstInterface.hwDevNum;
            break;

        case CPSS_INTERFACE_FABRIC_VIDX_E:
            exMxPmMacEntryPtr->dstOutlif.interfaceInfo.fabricVidx = macEntryPtr->dstInterface.fabricVidx;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert interface info into device specific format */
    exMxPmMacEntryPtr->dstOutlif.outlifType = macEntryPtr->dstOutlif.outlifType;
    switch (exMxPmMacEntryPtr->dstOutlif.outlifType)
    {
        case CPSS_EXMXPM_OUTLIF_TYPE_LL_E:
            exMxPmMacEntryPtr->dstOutlif.outlifPointer.arpPtr = macEntryPtr->dstOutlif.outlifPointer.arpPtr;
            break;

        case CPSS_EXMXPM_OUTLIF_TYPE_DIT_E:
            exMxPmMacEntryPtr->dstOutlif.outlifPointer.ditPtr = macEntryPtr->dstOutlif.outlifPointer.ditPtr;
            break;

        case CPSS_EXMXPM_OUTLIF_TYPE_TUNNEL_E:
            /* convert passenger type into device specific format */
            switch (macEntryPtr->dstOutlif.outlifPointer.tunnelStartPtr.passengerPacketType)
            {
                case PRV_TGF_TS_PASSENGER_PACKET_TYPE_ETHERNET_E:
                    exMxPmMacEntryPtr->dstOutlif.outlifPointer.tunnelStartPtr.passengerPacketType =
                        CPSS_EXMXPM_TS_PASSENGER_PACKET_TYPE_ETHERNET_E;
                    break;

                case PRV_TGF_TS_PASSENGER_PACKET_TYPE_OTHER_E:
                    exMxPmMacEntryPtr->dstOutlif.outlifPointer.tunnelStartPtr.passengerPacketType =
                        CPSS_EXMXPM_TS_PASSENGER_PACKET_TYPE_OTHER_E;
                    break;

                default:
                    return GT_BAD_PARAM;
            }

            exMxPmMacEntryPtr->dstOutlif.outlifPointer.tunnelStartPtr.ptr =
                macEntryPtr->dstOutlif.outlifPointer.tunnelStartPtr.ptr;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert daCommand info into device specific format */
    switch (macEntryPtr->daCommand)
    {
        case PRV_TGF_PACKET_CMD_FORWARD_E:
            exMxPmMacEntryPtr->daCommand = CPSS_PACKET_CMD_FORWARD_E;
            break;

        case PRV_TGF_PACKET_CMD_MIRROR_TO_CPU_E:
            exMxPmMacEntryPtr->daCommand = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
            break;

        case PRV_TGF_PACKET_CMD_TRAP_TO_CPU_E:
        case PRV_TGF_PACKET_CMD_CNTL_E:
            exMxPmMacEntryPtr->daCommand = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
            break;

        case PRV_TGF_PACKET_CMD_DROP_HARD_E:
            exMxPmMacEntryPtr->daCommand = CPSS_PACKET_CMD_DROP_HARD_E;
            break;

        case PRV_TGF_PACKET_CMD_DROP_SOFT_E:
            exMxPmMacEntryPtr->daCommand = CPSS_PACKET_CMD_DROP_SOFT_E;
            break;

        case PRV_TGF_PACKET_CMD_ROUTE_E:
            exMxPmMacEntryPtr->daCommand = CPSS_PACKET_CMD_ROUTE_E;
            break;

        case PRV_TGF_PACKET_CMD_ROUTE_AND_MIRROR_E:
            exMxPmMacEntryPtr->daCommand = CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E;
            break;

        case PRV_TGF_PACKET_CMD_BRIDGE_AND_MIRROR_E:
            exMxPmMacEntryPtr->daCommand = CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E;
            break;

        case PRV_TGF_PACKET_CMD_BRIDGE_E:
            exMxPmMacEntryPtr->daCommand = CPSS_PACKET_CMD_BRIDGE_E;
            break;

        case PRV_TGF_PACKET_CMD_NONE_E:
            exMxPmMacEntryPtr->daCommand = CPSS_PACKET_CMD_NONE_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert saCommand info into device specific format */
    switch (macEntryPtr->saCommand)
    {
        case PRV_TGF_PACKET_CMD_FORWARD_E:
            exMxPmMacEntryPtr->saCommand = CPSS_PACKET_CMD_FORWARD_E;
            break;

        case PRV_TGF_PACKET_CMD_MIRROR_TO_CPU_E:
            exMxPmMacEntryPtr->saCommand = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
            break;

        case PRV_TGF_PACKET_CMD_TRAP_TO_CPU_E:
        case PRV_TGF_PACKET_CMD_CNTL_E:
            exMxPmMacEntryPtr->saCommand = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
            break;

        case PRV_TGF_PACKET_CMD_DROP_HARD_E:
            exMxPmMacEntryPtr->saCommand = CPSS_PACKET_CMD_DROP_HARD_E;
            break;

        case PRV_TGF_PACKET_CMD_DROP_SOFT_E:
            exMxPmMacEntryPtr->saCommand = CPSS_PACKET_CMD_DROP_SOFT_E;
            break;

        case PRV_TGF_PACKET_CMD_ROUTE_E:
            exMxPmMacEntryPtr->saCommand = CPSS_PACKET_CMD_ROUTE_E;
            break;

        case PRV_TGF_PACKET_CMD_ROUTE_AND_MIRROR_E:
            exMxPmMacEntryPtr->saCommand = CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E;
            break;

        case PRV_TGF_PACKET_CMD_BRIDGE_AND_MIRROR_E:
            exMxPmMacEntryPtr->saCommand = CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E;
            break;

        case PRV_TGF_PACKET_CMD_BRIDGE_E:
            exMxPmMacEntryPtr->saCommand = CPSS_PACKET_CMD_BRIDGE_E;
            break;

        case PRV_TGF_PACKET_CMD_NONE_E:
            exMxPmMacEntryPtr->saCommand = CPSS_PACKET_CMD_NONE_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert generic vlanInfo into device specific vlanInfo */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmMacEntryPtr, macEntryPtr, isStatic);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmMacEntryPtr, macEntryPtr, daRoute);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmMacEntryPtr, macEntryPtr, mirrorToRxAnalyzerPortEn);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmMacEntryPtr, macEntryPtr, sourceId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmMacEntryPtr, macEntryPtr, userDefined);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmMacEntryPtr, macEntryPtr, daQosIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmMacEntryPtr, macEntryPtr, saQosIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmMacEntryPtr, macEntryPtr, daSecurityLevel);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmMacEntryPtr, macEntryPtr, saSecurityLevel);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmMacEntryPtr, macEntryPtr, appSpecificCpuCode);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmMacEntryPtr, macEntryPtr, pwId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmMacEntryPtr, macEntryPtr, spUnknown);

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToExMxPmDropCntMode function
* @endinternal
*
* @brief   Convert generic drop count mode into device specific drop count mode
*
* @param[in] dropMode                 - drop counter mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToExMxPmDropCntMode
(
    IN  PRV_TGF_BRG_DROP_CNTR_MODE_ENT       dropMode,
    OUT CPSS_EXMXPM_BRG_DROP_CNTR_MODE_ENT  *exMxPmDropModePtr
)
{
    /* convert drop counter mode into device specific format */
    switch (dropMode)
    {
        PRV_TGF_SWITCH_CASE_MAC(*exMxPmDropModePtr, PRV_TGF_BRG_DROP_CNTR_COUNT_ALL_E,
                                CPSS_EXMXPM_BRG_DROP_CNTR_COUNT_ALL_E);
        PRV_TGF_SWITCH_CASE_MAC(*exMxPmDropModePtr, PRV_TGF_BRG_DROP_CNTR_FDB_ENTRY_CMD_E,
                                CPSS_EXMXPM_BRG_DROP_CNTR_FDB_E);
        PRV_TGF_SWITCH_CASE_MAC(*exMxPmDropModePtr, PRV_TGF_BRG_DROP_CNTR_UNKNOWN_MAC_SA_E,
                                CPSS_EXMXPM_BRG_DROP_CNTR_UNKNOWN_MAC_SA_E);
        PRV_TGF_SWITCH_CASE_MAC(*exMxPmDropModePtr, PRV_TGF_BRG_DROP_CNTR_INVALID_SA_E,
                                CPSS_EXMXPM_BRG_DROP_CNTR_INVALID_MAC_SA_E);
        PRV_TGF_SWITCH_CASE_MAC(*exMxPmDropModePtr, PRV_TGF_BRG_DROP_CNTR_INVALID_VLAN_E,
                                CPSS_EXMXPM_BRG_DROP_CNTR_INVALID_VLAN_E);
        PRV_TGF_SWITCH_CASE_MAC(*exMxPmDropModePtr, PRV_TGF_BRG_DROP_CNTR_PORT_NOT_IN_VLAN_E,
                                CPSS_EXMXPM_BRG_DROP_CNTR_PORT_NOT_IN_VLAN_E);
        PRV_TGF_SWITCH_CASE_MAC(*exMxPmDropModePtr, PRV_TGF_BRG_DROP_CNTR_VLAN_RANGE_E,
                                CPSS_EXMXPM_BRG_DROP_CNTR_VLAN_RANGE_E);
        PRV_TGF_SWITCH_CASE_MAC(*exMxPmDropModePtr, PRV_TGF_BRG_DROP_CNTR_MOVED_STATIC_ADDR_E,
                                CPSS_EXMXPM_BRG_DROP_CNTR_MOVED_STATIC_SA_E);
        PRV_TGF_SWITCH_CASE_MAC(*exMxPmDropModePtr, PRV_TGF_BRG_DROP_CNTR_ARP_SA_MISMATCH_E,
                                CPSS_EXMXPM_BRG_DROP_CNTR_ARP_SA_MISMATCH_E);
        PRV_TGF_SWITCH_CASE_MAC(*exMxPmDropModePtr, PRV_TGF_BRG_DROP_CNTR_SYN_WITH_DATA_E,
                                CPSS_EXMXPM_BRG_DROP_CNTR_SYN_WITH_DATA_E);
        PRV_TGF_SWITCH_CASE_MAC(*exMxPmDropModePtr, PRV_TGF_BRG_DROP_CNTR_TCP_OVER_MC_OR_BC_E,
                                CPSS_EXMXPM_BRG_DROP_CNTR_TCP_OVER_MC_BC_E);
        PRV_TGF_SWITCH_CASE_MAC(*exMxPmDropModePtr, PRV_TGF_BRG_DROP_CNTR_ACCESS_MATRIX_E,
                                CPSS_EXMXPM_BRG_DROP_CNTR_ACCESS_MATRIX_E);
        PRV_TGF_SWITCH_CASE_MAC(*exMxPmDropModePtr, PRV_TGF_BRG_DROP_CNTR_SEC_LEARNING_E,
                                CPSS_EXMXPM_BRG_DROP_CNTR_SECURE_LEARNING_E);
        PRV_TGF_SWITCH_CASE_MAC(*exMxPmDropModePtr, PRV_TGF_BRG_DROP_CNTR_ACCEPT_FRAME_TYPE_E,
                                CPSS_EXMXPM_BRG_DROP_CNTR_ACCEPT_FRAME_E);
        PRV_TGF_SWITCH_CASE_MAC(*exMxPmDropModePtr, PRV_TGF_BRG_DROP_CNTR_FRAG_ICMP_E,
                                CPSS_EXMXPM_BRG_DROP_CNTR_FRAG_ICMP_E);
        PRV_TGF_SWITCH_CASE_MAC(*exMxPmDropModePtr, PRV_TGF_BRG_DROP_CNTR_TCP_FLAGS_ZERO_E,
                                CPSS_EXMXPM_BRG_DROP_CNTR_TCP_FLAGS_ZERO_E);
        PRV_TGF_SWITCH_CASE_MAC(*exMxPmDropModePtr, PRV_TGF_BRG_DROP_CNTR_TCP_FLAGS_FUP_SET_E,
                                CPSS_EXMXPM_BRG_DROP_CNTR_TCP_FLAGS_FUP_SET_E);
        PRV_TGF_SWITCH_CASE_MAC(*exMxPmDropModePtr, PRV_TGF_BRG_DROP_CNTR_TCP_FLAGS_SF_SET_E,
                                CPSS_EXMXPM_BRG_DROP_CNTR_TCP_FLAGS_SF_SET_E);
        PRV_TGF_SWITCH_CASE_MAC(*exMxPmDropModePtr, PRV_TGF_BRG_DROP_CNTR_TCP_FLAGS_SR_SET_E,
                                CPSS_EXMXPM_BRG_DROP_CNTR_TCP_FLAGS_SR_SET_E);
        PRV_TGF_SWITCH_CASE_MAC(*exMxPmDropModePtr, PRV_TGF_BRG_DROP_CNTR_TCP_UDP_PORT_ZERO_E,
                                CPSS_EXMXPM_BRG_DROP_CNTR_TCP_UDP_PORT_ZERO_E);
        PRV_TGF_SWITCH_CASE_MAC(*exMxPmDropModePtr, PRV_TGF_BRG_DROP_CNTR_VLAN_MRU_E,
                                CPSS_EXMXPM_BRG_DROP_CNTR_VLAN_MRU_E);
        PRV_TGF_SWITCH_CASE_MAC(*exMxPmDropModePtr, PRV_TGF_BRG_DROP_CNTR_RATE_LIMIT_E,
                                CPSS_EXMXPM_BRG_DROP_CNTR_RATE_LIMIT_E);
        PRV_TGF_SWITCH_CASE_MAC(*exMxPmDropModePtr, PRV_TGF_BRG_DROP_CNTR_LOCAL_PORT_E,
                                CPSS_EXMXPM_BRG_DROP_CNTR_LOCAL_PACKET_E);
        PRV_TGF_SWITCH_CASE_MAC(*exMxPmDropModePtr, PRV_TGF_BRG_DROP_CNTR_IEEE_RESERVED_E,
                                CPSS_EXMXPM_BRG_DROP_CNTR_IEEE_RESERVED_E);
        PRV_TGF_SWITCH_CASE_MAC(*exMxPmDropModePtr, PRV_TGF_BRG_DROP_CNTR_UREG_L2_NON_IPM_MC_E,
                                CPSS_EXMXPM_BRG_DROP_CNTR_UNREG_NON_IPM_MC_E);
        PRV_TGF_SWITCH_CASE_MAC(*exMxPmDropModePtr, PRV_TGF_BRG_DROP_CNTR_UREG_L2_IPV6_MC_E,
                                CPSS_EXMXPM_BRG_DROP_CNTR_UNREG_IPV6_IPM_E);
        PRV_TGF_SWITCH_CASE_MAC(*exMxPmDropModePtr, PRV_TGF_BRG_DROP_CNTR_UREG_L2_IPV4_MC_E,
                                CPSS_EXMXPM_BRG_DROP_CNTR_UNREG_IPV4_IPM_E);
        PRV_TGF_SWITCH_CASE_MAC(*exMxPmDropModePtr, PRV_TGF_BRG_DROP_CNTR_UNKNOWN_L2_UC_E,
                                CPSS_EXMXPM_BRG_DROP_CNTR_UNREG_UC_E);
        PRV_TGF_SWITCH_CASE_MAC(*exMxPmDropModePtr, PRV_TGF_BRG_DROP_CNTR_UREG_L2_IPV4_BC_E,
                                CPSS_EXMXPM_BRG_DROP_CNTR_UNREG_IPV4_BC_E);
        PRV_TGF_SWITCH_CASE_MAC(*exMxPmDropModePtr, PRV_TGF_BRG_DROP_CNTR_UREG_L2_NON_IPV4_BC_E,
                                CPSS_EXMXPM_BRG_DROP_CNTR_UNREG_NON_IPV4_BC_E);
        PRV_TGF_SWITCH_CASE_MAC(*exMxPmDropModePtr, PRV_TGF_BRG_DROP_CNTR_DATA_ERROR_E,
                                CPSS_EXMXPM_BRG_DROP_CNTR_DATA_ERROR_E);
        PRV_TGF_SWITCH_CASE_MAC(*exMxPmDropModePtr, PRV_TGF_BRG_DROP_CNTR_IPMC_IANA_RANGE_E,
                                CPSS_EXMXPM_BRG_DROP_CNTR_IPMC_IANA_RANGE_E);
        PRV_TGF_SWITCH_CASE_MAC(*exMxPmDropModePtr, PRV_TGF_BRG_DROP_CNTR_IPMC_NOT_IANA_RANGE_E,
                                CPSS_EXMXPM_BRG_DROP_CNTR_IPMC_NOT_IANA_RANGE_E);
        PRV_TGF_SWITCH_CASE_MAC(*exMxPmDropModePtr, PRV_TGF_BRG_DROP_CNTR_VALUE_CHASIS_E,
                                CPSS_EXMXPM_BRG_DROP_CNTR_VALUE_CHASIS_E);
        PRV_TGF_SWITCH_CASE_MAC(*exMxPmDropModePtr, PRV_TGF_BRG_DROP_CNTR_BLOCKING_LEARNING_E,
                                CPSS_EXMXPM_BRG_DROP_CNTR_BLOCKING_LEARNING_E);

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal prvTgfConvertExMxPmToGenericDropCntMode function
* @endinternal
*
* @brief   Convert generic drop count mode from device specific drop count mode
*
* @param[in] exMxPmDropMode           - ExMxPm drop counter mode
*
* @param[out] dropModePtr              - (pointer to) drop counter mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertExMxPmToGenericDropCntMode
(
    IN  CPSS_EXMXPM_BRG_DROP_CNTR_MODE_ENT   exMxPmDropMode,
    OUT PRV_TGF_BRG_DROP_CNTR_MODE_ENT      *dropModePtr
)
{
    /* convert drop counter mode from device specific format */
    switch (exMxPmDropMode)
    {
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_EXMXPM_BRG_DROP_CNTR_COUNT_ALL_E,
                                PRV_TGF_BRG_DROP_CNTR_COUNT_ALL_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_EXMXPM_BRG_DROP_CNTR_FDB_E,
                                PRV_TGF_BRG_DROP_CNTR_FDB_ENTRY_CMD_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_EXMXPM_BRG_DROP_CNTR_UNKNOWN_MAC_SA_E,
                                PRV_TGF_BRG_DROP_CNTR_UNKNOWN_MAC_SA_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_EXMXPM_BRG_DROP_CNTR_INVALID_MAC_SA_E,
                                PRV_TGF_BRG_DROP_CNTR_INVALID_SA_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_EXMXPM_BRG_DROP_CNTR_INVALID_VLAN_E,
                                PRV_TGF_BRG_DROP_CNTR_INVALID_VLAN_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_EXMXPM_BRG_DROP_CNTR_PORT_NOT_IN_VLAN_E,
                                PRV_TGF_BRG_DROP_CNTR_PORT_NOT_IN_VLAN_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_EXMXPM_BRG_DROP_CNTR_VLAN_RANGE_E,
                                PRV_TGF_BRG_DROP_CNTR_VLAN_RANGE_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_EXMXPM_BRG_DROP_CNTR_MOVED_STATIC_SA_E,
                                PRV_TGF_BRG_DROP_CNTR_MOVED_STATIC_ADDR_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_EXMXPM_BRG_DROP_CNTR_ARP_SA_MISMATCH_E,
                                PRV_TGF_BRG_DROP_CNTR_ARP_SA_MISMATCH_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_EXMXPM_BRG_DROP_CNTR_SYN_WITH_DATA_E,
                                PRV_TGF_BRG_DROP_CNTR_SYN_WITH_DATA_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_EXMXPM_BRG_DROP_CNTR_TCP_OVER_MC_BC_E,
                                PRV_TGF_BRG_DROP_CNTR_TCP_OVER_MC_OR_BC_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_EXMXPM_BRG_DROP_CNTR_ACCESS_MATRIX_E,
                                PRV_TGF_BRG_DROP_CNTR_ACCESS_MATRIX_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_EXMXPM_BRG_DROP_CNTR_SECURE_LEARNING_E,
                                PRV_TGF_BRG_DROP_CNTR_SEC_LEARNING_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_EXMXPM_BRG_DROP_CNTR_ACCEPT_FRAME_E,
                                PRV_TGF_BRG_DROP_CNTR_ACCEPT_FRAME_TYPE_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_EXMXPM_BRG_DROP_CNTR_FRAG_ICMP_E,
                                PRV_TGF_BRG_DROP_CNTR_FRAG_ICMP_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_EXMXPM_BRG_DROP_CNTR_TCP_FLAGS_ZERO_E,
                                PRV_TGF_BRG_DROP_CNTR_TCP_FLAGS_ZERO_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_EXMXPM_BRG_DROP_CNTR_TCP_FLAGS_FUP_SET_E,
                                PRV_TGF_BRG_DROP_CNTR_TCP_FLAGS_FUP_SET_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_EXMXPM_BRG_DROP_CNTR_TCP_FLAGS_SF_SET_E,
                                PRV_TGF_BRG_DROP_CNTR_TCP_FLAGS_SF_SET_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_EXMXPM_BRG_DROP_CNTR_TCP_FLAGS_SR_SET_E,
                                PRV_TGF_BRG_DROP_CNTR_TCP_FLAGS_SR_SET_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_EXMXPM_BRG_DROP_CNTR_TCP_UDP_PORT_ZERO_E,
                                PRV_TGF_BRG_DROP_CNTR_TCP_UDP_PORT_ZERO_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_EXMXPM_BRG_DROP_CNTR_VLAN_MRU_E,
                                PRV_TGF_BRG_DROP_CNTR_VLAN_MRU_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_EXMXPM_BRG_DROP_CNTR_RATE_LIMIT_E,
                                PRV_TGF_BRG_DROP_CNTR_RATE_LIMIT_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_EXMXPM_BRG_DROP_CNTR_LOCAL_PACKET_E,
                                PRV_TGF_BRG_DROP_CNTR_LOCAL_PORT_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_EXMXPM_BRG_DROP_CNTR_IEEE_RESERVED_E,
                                PRV_TGF_BRG_DROP_CNTR_IEEE_RESERVED_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_EXMXPM_BRG_DROP_CNTR_UNREG_NON_IPM_MC_E,
                                PRV_TGF_BRG_DROP_CNTR_UREG_L2_NON_IPM_MC_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_EXMXPM_BRG_DROP_CNTR_UNREG_IPV6_IPM_E,
                                PRV_TGF_BRG_DROP_CNTR_UREG_L2_IPV6_MC_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_EXMXPM_BRG_DROP_CNTR_UNREG_IPV4_IPM_E,
                                PRV_TGF_BRG_DROP_CNTR_UREG_L2_IPV4_MC_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_EXMXPM_BRG_DROP_CNTR_UNREG_UC_E,
                                PRV_TGF_BRG_DROP_CNTR_UNKNOWN_L2_UC_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_EXMXPM_BRG_DROP_CNTR_UNREG_IPV4_BC_E,
                                PRV_TGF_BRG_DROP_CNTR_UREG_L2_IPV4_BC_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_EXMXPM_BRG_DROP_CNTR_UNREG_NON_IPV4_BC_E,
                                PRV_TGF_BRG_DROP_CNTR_UREG_L2_NON_IPV4_BC_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_EXMXPM_BRG_DROP_CNTR_DATA_ERROR_E,
                                PRV_TGF_BRG_DROP_CNTR_DATA_ERROR_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_EXMXPM_BRG_DROP_CNTR_IPMC_IANA_RANGE_E,
                                PRV_TGF_BRG_DROP_CNTR_IPMC_IANA_RANGE_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_EXMXPM_BRG_DROP_CNTR_IPMC_NOT_IANA_RANGE_E,
                                PRV_TGF_BRG_DROP_CNTR_IPMC_NOT_IANA_RANGE_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_EXMXPM_BRG_DROP_CNTR_VALUE_CHASIS_E,
                                PRV_TGF_BRG_DROP_CNTR_VALUE_CHASIS_E);
        PRV_TGF_SWITCH_CASE_MAC(*dropModePtr, CPSS_EXMXPM_BRG_DROP_CNTR_BLOCKING_LEARNING_E,
                                PRV_TGF_BRG_DROP_CNTR_BLOCKING_LEARNING_E);

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}
#endif /* EXMXPM_FAMILY */


/******************************************************************************\
 *                       CPSS generic API section                             *
\******************************************************************************/

/**
* @internal prvTgfBrgVlanVrfIdSet function
* @endinternal
*
* @brief   Updates VRF Id in HW vlan entry
*
* @param[in] vlanId                   - VLAN Id
* @param[in] vrfId                    - VRF Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vid
* @retval GT_OUT_OF_RANGE          - length of portsMembersPtr or portsTaggingPtr is
*                                       out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvTgfBrgVlanVrfIdSet
(
    IN GT_U16                               vlanId,
    IN GT_U32                               vrfId
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
    CPSS_DXCH_BRG_VLAN_INFO_STC             dxChVlanInfo;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC    dxChPortsTaggingCmd;
    CPSS_PORTS_BMP_STC                      portsMembers;
    CPSS_PORTS_BMP_STC                      portsTagging;
    GT_BOOL                                 isValid;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific read API */
        rc = cpssDxChBrgVlanEntryRead(
            devNum,
            vlanId,
            &portsMembers,
            &portsTagging,
            &dxChVlanInfo,
            &isValid,
            &dxChPortsTaggingCmd);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgVlanEntryRead FAILED, rc = [%d]", rc);

            rc1 = rc;
        }

        if (isValid == GT_FALSE)
        {
            /* bypass invalid VLAN entry */
            rc1 = GT_BAD_STATE;
            continue;
        }

        /* update VRF Id*/
        dxChVlanInfo.vrfId = vrfId;

        /* call device specific write API */
        rc = cpssDxChBrgVlanEntryWrite(
            devNum,
            vlanId,
            &portsMembers,
            &portsTagging,
            &dxChVlanInfo,
            &dxChPortsTaggingCmd);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgVlanEntryWrite FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#else
    /* avoid warnings */
    TGF_PARAM_NOT_USED(vlanId);
    TGF_PARAM_NOT_USED(vrfId);

    /* not applicable for not DXCH devices */
    return GT_NOT_APPLICABLE_DEVICE;
#endif /* CHX_FAMILY */
}

/**
* @internal prvTgfBrgVlanEntryWrite function
* @endinternal
*
* @brief   Builds and writes vlan entry to HW
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN Id
* @param[in] portsMembersPtr          - (pointer to) bmp of ports members in vlan
* @param[in] portsTaggingPtr          - (pointer to) bmp of ports tagged in the vlan
* @param[in] vlanInfoPtr              - (pointer to) VLAN specific information
* @param[in] portsTaggingCmdPtr       - (pointer to) ports tagging commands in the vlan
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vid
* @retval GT_OUT_OF_RANGE          - length of portsMembersPtr or portsTaggingPtr is
*                                       out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvTgfBrgVlanEntryWrite
(
    IN GT_U8                                devNum,
    IN GT_U16                               vlanId,
    IN CPSS_PORTS_BMP_STC                  *portsMembersPtr,
    IN CPSS_PORTS_BMP_STC                  *portsTaggingPtr,
    IN PRV_TGF_BRG_VLAN_INFO_STC           *vlanInfoPtr,
    IN PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC  *portsTaggingCmdPtr
)
{
    GT_U32  ii;
    GT_U32  portIter = 0;
    PRV_TGF_MEMBER_FORCE_INFO_STC *currMemPtr;
    CPSS_PORTS_BMP_STC            tmpPortsMembers;
    CPSS_PORTS_BMP_STC            tmpPortsTagging;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC  tmpPortsTaggingCmd;

#ifdef CHX_FAMILY
    CPSS_DXCH_BRG_VLAN_INFO_STC             dxChVlanInfo;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC    dxChPortsTaggingCmd;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_BRG_VLAN_INFO_STC   exMxPmVlanInfo;
#endif /* EXMXPM_FAMILY */


    if(portsMembersPtr)
    {
        tmpPortsMembers = *portsMembersPtr;
    }
    else
    {
        cpssOsMemSet(&tmpPortsMembers,0,sizeof(tmpPortsMembers));
    }

    if(portsTaggingPtr)
    {
        tmpPortsTagging = *portsTaggingPtr;
    }
    else
    {
        cpssOsMemSet(&tmpPortsTagging,0,sizeof(tmpPortsTagging));
    }

    if(portsTaggingCmdPtr)
    {
        tmpPortsTaggingCmd = *portsTaggingCmdPtr;
    }
    else
    {
        cpssOsMemSet(&tmpPortsTaggingCmd,0,sizeof(tmpPortsTaggingCmd));
    }

    /* look for this member in the DB */
    currMemPtr = &prvTgfDevPortForceArray[0];
    for(ii = 0 ; ii < prvTgfDevPortForceNum; ii++,currMemPtr++)
    {
        if(devNum != currMemPtr->member.devNum)
        {
            continue;
        }

        if(currMemPtr->forceToVlan == GT_FALSE)
        {
            /* member not need to be forced to any vlan */
            continue;
        }

        if(currMemPtr->member.portNum >= CPSS_MAX_PORTS_NUM_CNS)
        {
            return GT_BAD_PARAM;
        }

        /* set the member */
        CPSS_PORTS_BMP_PORT_SET_MAC(&tmpPortsMembers,currMemPtr->member.portNum);

        if(currMemPtr->vlanInfo.tagged == GT_TRUE)
        {
            CPSS_PORTS_BMP_PORT_SET_MAC(&tmpPortsTagging,currMemPtr->member.portNum);
        }

        tmpPortsTaggingCmd.portsCmd[currMemPtr->member.portNum] = currMemPtr->vlanInfo.tagCmd;
    }


#ifdef CHX_FAMILY
    /* clear entry */
    cpssOsMemSet((GT_VOID*) &dxChVlanInfo, 0, sizeof(dxChVlanInfo));
    cpssOsMemSet((GT_VOID*) &dxChPortsTaggingCmd, 0, sizeof(dxChPortsTaggingCmd));

    /* default value */
    dxChVlanInfo.unregIpmEVidx = 0xFFF;

    /* convert generic vlanInfo into device specific vlanInfo */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, unkSrcAddrSecBreach );
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, unregNonIpMcastCmd  );
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, unregIpv4McastCmd   );
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, unregIpv6McastCmd   );
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, unkUcastCmd         );
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, unregIpv4BcastCmd   );
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, unregNonIpv4BcastCmd);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, ipv4IgmpToCpuEn     );
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, mirrToRxAnalyzerEn  );
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, ipv6IcmpToCpuEn     );
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, ipv4IpmBrgMode      );
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, ipv6IpmBrgMode      );
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, ipv4IpmBrgEn        );
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, ipv6IpmBrgEn        );
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, ipv6SiteIdMode      );
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, ipv4UcastRouteEn    );
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, ipv4McastRouteEn    );
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, ipv6UcastRouteEn    );
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, ipv6McastRouteEn    );
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, stgId               );
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, autoLearnDisable    );
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, naMsgToCpuEn        );
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, mruIdx              );
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, bcastUdpTrapMirrEn  );
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, vrfId               );
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, floodVidx           );
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, ucastLocalSwitchingEn);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, mcastLocalSwitchingEn);

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, mirrToRxAnalyzerIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, mirrToTxAnalyzerEn);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, mirrToTxAnalyzerIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, fidValue);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, unknownMacSaCmd);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, ipv4McBcMirrToAnalyzerEn);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, ipv4McBcMirrToAnalyzerIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, ipv6McMirrToAnalyzerEn);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, ipv6McMirrToAnalyzerIndex);

    /* convert generic ipCtrlToCpuEn into device specific ipCtrlToCpuEn */
    switch (vlanInfoPtr->ipCtrlToCpuEn)
    {
        case PRV_TGF_BRG_IP_CTRL_NONE_E:
            dxChVlanInfo.ipCtrlToCpuEn = CPSS_DXCH_BRG_IP_CTRL_NONE_E;
            break;

        case PRV_TGF_BRG_IP_CTRL_IPV4_E:
            dxChVlanInfo.ipCtrlToCpuEn = CPSS_DXCH_BRG_IP_CTRL_IPV4_E;
            break;

        case PRV_TGF_BRG_IP_CTRL_IPV6_E:
            dxChVlanInfo.ipCtrlToCpuEn = CPSS_DXCH_BRG_IP_CTRL_IPV6_E;
            break;

        case PRV_TGF_BRG_IP_CTRL_IPV4_IPV6_E:
            dxChVlanInfo.ipCtrlToCpuEn = CPSS_DXCH_BRG_IP_CTRL_IPV4_IPV6_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert generic floodVidxMode into device specific floodVidxMode */
    switch (vlanInfoPtr->floodVidxMode)
    {
        case PRV_TGF_BRG_VLAN_FLOOD_VIDX_MODE_UNREG_MC_E:
            dxChVlanInfo.floodVidxMode = CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_UNREG_MC_E;
            break;

        case PRV_TGF_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E:
            dxChVlanInfo.floodVidxMode = CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert generic portIsolationMode into device specific portIsolationMode */
    switch (vlanInfoPtr->portIsolationMode)
    {
        case PRV_TGF_BRG_VLAN_PORT_ISOLATION_DISABLE_CMD_E:
            dxChVlanInfo.portIsolationMode = CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_DISABLE_CMD_E;
            break;

        case PRV_TGF_BRG_VLAN_PORT_ISOLATION_L2_CMD_E:
            dxChVlanInfo.portIsolationMode = CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_L2_CMD_E;
            break;

        case PRV_TGF_BRG_VLAN_PORT_ISOLATION_L3_CMD_E:
            dxChVlanInfo.portIsolationMode = CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_L3_CMD_E;
            break;

        case PRV_TGF_BRG_VLAN_PORT_ISOLATION_L2_L3_CMD_E:
            dxChVlanInfo.portIsolationMode = CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_L2_L3_CMD_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert generic portsTaggingCmd into device specific portsTaggingCmd */
    for (portIter = 0; portIter < CPSS_MAX_PORTS_NUM_CNS; portIter++)
    {
        switch (tmpPortsTaggingCmd.portsCmd[portIter])
        {
            case PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E:
                dxChPortsTaggingCmd.portsCmd[portIter] = CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E;
                break;

            case PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E:
                dxChPortsTaggingCmd.portsCmd[portIter] = CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E;
                break;

            case PRV_TGF_BRG_VLAN_PORT_TAG1_CMD_E:
                dxChPortsTaggingCmd.portsCmd[portIter] = CPSS_DXCH_BRG_VLAN_PORT_TAG1_CMD_E;
                break;

            case PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E:
                dxChPortsTaggingCmd.portsCmd[portIter] = CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E;
                break;

            case PRV_TGF_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E:
                dxChPortsTaggingCmd.portsCmd[portIter] = CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E;
                break;

            case PRV_TGF_BRG_VLAN_PORT_PUSH_TAG0_CMD_E:
                dxChPortsTaggingCmd.portsCmd[portIter] = CPSS_DXCH_BRG_VLAN_PORT_PUSH_TAG0_CMD_E;
                break;

            case PRV_TGF_BRG_VLAN_PORT_POP_OUTER_TAG_CMD_E:
                dxChPortsTaggingCmd.portsCmd[portIter] = CPSS_DXCH_BRG_VLAN_PORT_POP_OUTER_TAG_CMD_E;
                break;

            case PRV_TGF_BRG_VLAN_PORT_DO_NOT_MODIFY_TAG_CMD_E:
                dxChPortsTaggingCmd.portsCmd[portIter] = CPSS_DXCH_BRG_VLAN_PORT_DO_NOT_MODIFY_TAG_CMD_E;
                break;

            default:
                return GT_BAD_PARAM;
        }
    }

    /* if fidValue was not set with a legal value */
    if(vlanInfoPtr->fidValue==0)
        dxChVlanInfo.fidValue = vlanId;

    /* call device specific API */
    return cpssDxChBrgVlanEntryWrite(devNum,
                                     vlanId,
                                     &tmpPortsMembers,
                                     &tmpPortsTagging,
                                     &dxChVlanInfo,
                                     &dxChPortsTaggingCmd);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* update portsTagging according to portsTaggingCmd */
        for (portIter = 0; portIter < CPSS_MAX_PORTS_NUM_CNS; portIter++)
        {
                switch (tmpPortsTaggingCmd.portsCmd[portIter])
                {
                        case PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E:
                                /* remove the member */
                                CPSS_PORTS_BMP_PORT_CLEAR_MAC(&tmpPortsTagging, portIter);
                                break;
                        default:
                                /* add the member */
                                CPSS_PORTS_BMP_PORT_SET_MAC(&tmpPortsTagging, portIter);
                }
        }

    /* clear entry */
    cpssOsMemSet((GT_VOID*) &exMxPmVlanInfo, 0, sizeof(exMxPmVlanInfo));

    /* convert generic vlanInfo into device specific vlanInfo */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&exMxPmVlanInfo, vlanInfoPtr, cpuMember             );
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&exMxPmVlanInfo, vlanInfoPtr, mirrorToTxAnalyzerEn  );
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&exMxPmVlanInfo, vlanInfoPtr, ipv4IpmBrgMode        );
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&exMxPmVlanInfo, vlanInfoPtr, ipv6IpmBrgMode        );
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&exMxPmVlanInfo, vlanInfoPtr, ipv4IpmBrgEn          );
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&exMxPmVlanInfo, vlanInfoPtr, ipv6IpmBrgEn          );
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&exMxPmVlanInfo, vlanInfoPtr, stgId                 );
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&exMxPmVlanInfo, vlanInfoPtr, egressUnregMcFilterCmd);

    /* call device specific API */
    return cpssExMxPmBrgVlanEntryWrite(devNum,
                                         vlanId,
                                         &tmpPortsMembers,
                                         &tmpPortsTagging,
                                         &exMxPmVlanInfo);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgVlanEntryRead function
* @endinternal
*
* @brief   Read vlan entry.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN Id
*
* @param[out] portsMembersPtr          - (pointer to) bmp of ports members in vlan
*                                      CPU port supported
* @param[out] portsTaggingPtr          - (pointer to) bmp of ports tagged in the vlan -
*                                      The parameter is relevant for DxCh1, DxCh2
*                                      and DxCh3 devices.
*                                      The parameter is relevant for xCat and
*                                      above devices without TR101 feature support
*                                      The parameter is not relevant and ignored for
*                                      xCat and above devices with TR101 feature
*                                      support.
* @param[out] vlanInfoPtr              - (pointer to) VLAN specific information
* @param[out] isValidPtr               - (pointer to) VLAN entry status
*                                      GT_TRUE = Vlan is valid
*                                      GT_FALSE = Vlan is not Valid
* @param[out] portsTaggingCmdPtr       - (pointer to) ports tagging commands in the vlan -
*                                      The parameter is relevant only for xCat and
*                                      above with TR101 feature support.
*                                      The parameter is not relevant and ignored for
*                                      other devices.
*/
GT_STATUS prvTgfBrgVlanEntryRead
(
   IN GT_U8                                 devNum,
   IN GT_U16                                vlanId,
   OUT CPSS_PORTS_BMP_STC                  *portsMembersPtr,
   OUT CPSS_PORTS_BMP_STC                  *portsTaggingPtr,
   OUT GT_BOOL                             *isValidPtr,
   OUT PRV_TGF_BRG_VLAN_INFO_STC           *vlanInfoPtr,
   OUT PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC  *portsTaggingCmdPtr
)
{
    GT_STATUS   rc, rc1 = GT_OK;

#ifdef CHX_FAMILY
    CPSS_DXCH_BRG_VLAN_INFO_STC             dxChVlanInfo;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC    dxChPortsTaggingCmd;
    GT_U32  ii;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_BRG_VLAN_INFO_STC   exMxPmVlanInfo;
#endif /* EXMXPM_FAMILY */

#ifdef CHX_FAMILY
    /* clear entry */
    cpssOsMemSet((GT_VOID*) &dxChVlanInfo, 0, sizeof(dxChVlanInfo));
    cpssOsMemSet((GT_VOID*) &dxChPortsTaggingCmd, 0, sizeof(dxChPortsTaggingCmd));

    /* call device specific API */
    rc = cpssDxChBrgVlanEntryRead(devNum,
                                  vlanId,
                                  portsMembersPtr,
                                  portsTaggingPtr,
                                  &dxChVlanInfo,
                                  isValidPtr,
                                  &dxChPortsTaggingCmd);

    /* clear entry */
    cpssOsMemSet((GT_VOID*) vlanInfoPtr, 0, sizeof(*vlanInfoPtr));

    if(portsTaggingCmdPtr)
    {
        for(ii = 0 ; ii < CPSS_MAX_PORTS_NUM_CNS; ii++)
        {
            switch (dxChPortsTaggingCmd.portsCmd[ii])
            {
                case CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E :
                    portsTaggingCmdPtr->portsCmd[ii] = PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E;
                    break;

                case CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E :
                    portsTaggingCmdPtr->portsCmd[ii] = PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E;
                    break;

                case CPSS_DXCH_BRG_VLAN_PORT_TAG1_CMD_E:
                    portsTaggingCmdPtr->portsCmd[ii] = PRV_TGF_BRG_VLAN_PORT_TAG1_CMD_E;
                    break;

                case CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E :
                    portsTaggingCmdPtr->portsCmd[ii] = PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E;
                    break;

                case CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E:
                    portsTaggingCmdPtr->portsCmd[ii] = PRV_TGF_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E;
                    break;

                case CPSS_DXCH_BRG_VLAN_PORT_PUSH_TAG0_CMD_E:
                    portsTaggingCmdPtr->portsCmd[ii] = PRV_TGF_BRG_VLAN_PORT_PUSH_TAG0_CMD_E;
                    break;

                case CPSS_DXCH_BRG_VLAN_PORT_POP_OUTER_TAG_CMD_E:
                    portsTaggingCmdPtr->portsCmd[ii] = PRV_TGF_BRG_VLAN_PORT_POP_OUTER_TAG_CMD_E;
                    break;

                case CPSS_DXCH_BRG_VLAN_PORT_DO_NOT_MODIFY_TAG_CMD_E:
                    portsTaggingCmdPtr->portsCmd[ii] = PRV_TGF_BRG_VLAN_PORT_DO_NOT_MODIFY_TAG_CMD_E;
                    break;

                default:
                    return GT_BAD_PARAM;
            }
        }
    }

    /* convert device specific vlanInfo into generic vlanInfo */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, unkSrcAddrSecBreach );
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, unregNonIpMcastCmd  );
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, unregIpv4McastCmd   );
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, unregIpv6McastCmd   );
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, unkUcastCmd         );
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, unregIpv4BcastCmd   );
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, unregNonIpv4BcastCmd);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, ipv4IgmpToCpuEn     );
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, mirrToRxAnalyzerEn  );
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, ipv6IcmpToCpuEn     );
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, ipv4IpmBrgMode      );
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, ipv6IpmBrgMode      );
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, ipv4IpmBrgEn        );
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, ipv6IpmBrgEn        );
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, ipv6SiteIdMode      );
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, ipv4UcastRouteEn    );
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, ipv4McastRouteEn    );
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, ipv6UcastRouteEn    );
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, ipv6McastRouteEn    );
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, stgId               );
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, autoLearnDisable    );
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, naMsgToCpuEn        );
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, mruIdx              );
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, bcastUdpTrapMirrEn  );
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, vrfId               );
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, floodVidx           );
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, floodVidxMode       );
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, ucastLocalSwitchingEn);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, mcastLocalSwitchingEn);

    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, mirrToRxAnalyzerIndex);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, mirrToTxAnalyzerEn);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, mirrToTxAnalyzerIndex);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, fidValue);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, unknownMacSaCmd);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, ipv4McBcMirrToAnalyzerEn);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, ipv4McBcMirrToAnalyzerIndex);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, ipv6McMirrToAnalyzerEn);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChVlanInfo, vlanInfoPtr, ipv6McMirrToAnalyzerIndex);

    if (rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfBrgVlanEntryRead FAILED, rc = [%d]", rc);
        rc1 = rc;
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(portsTaggingCmdPtr);

    /* clear entry */
    cpssOsMemSet((GT_VOID*) &exMxPmVlanInfo, 0, sizeof(exMxPmVlanInfo));

    /* call device specific API */
    rc = cpssExMxPmBrgVlanEntryRead(devNum,
                                         vlanId,
                                         portsMembersPtr,
                                         portsTaggingPtr,
                                         &exMxPmVlanInfo,
                                         isValidPtr);

    /* clear entry */
    cpssOsMemSet((GT_VOID*) vlanInfoPtr, 0, sizeof(*vlanInfoPtr));

    /* convert device specific vlanInfo into generic vlanInfo */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&exMxPmVlanInfo, vlanInfoPtr, cpuMember             );
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&exMxPmVlanInfo, vlanInfoPtr, mirrorToTxAnalyzerEn  );
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&exMxPmVlanInfo, vlanInfoPtr, ipv4IpmBrgMode        );
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&exMxPmVlanInfo, vlanInfoPtr, ipv6IpmBrgMode        );
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&exMxPmVlanInfo, vlanInfoPtr, ipv4IpmBrgEn          );
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&exMxPmVlanInfo, vlanInfoPtr, ipv6IpmBrgEn          );
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&exMxPmVlanInfo, vlanInfoPtr, stgId                 );
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&exMxPmVlanInfo, vlanInfoPtr, egressUnregMcFilterCmd);

    if (rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfBrgVlanEntryRead FAILED, rc = [%d]", rc);
        rc1 = rc;
    }
    return rc1;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgVlanEntryInvalidate function
* @endinternal
*
* @brief   This function invalidates VLAN entry
*
* @param[in] vlanId                   - VLAN id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PARAM             - wrong vid
*/
GT_STATUS prvTgfBrgVlanEntryInvalidate
(
    IN GT_U16               vlanId
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;


#ifdef CHX_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /*PRV_UTF_LOG1_MAC("prvTgfBrgVlanEntryInvalidate(%d) \n",
        vlanId); */

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgVlanEntryInvalidate(devNum, vlanId);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgVlanEntryInvalidate FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmBrgVlanEntryInvalidate(devNum, vlanId);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgVlanEntryInvalidate FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgVlanMemberAdd function
* @endinternal
*
* @brief   Set specific member at VLAN entry
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN Id
* @param[in] portNum                  - port number
* @param[in] isTagged                 - GT_TRUE/GT_FALSE, to set the port as tagged/untagged
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfBrgVlanMemberAdd
(
    IN GT_U8                devNum,
    IN GT_U16               vlanId,
    IN GT_U32                portNum,
    IN GT_BOOL              isTagged
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT dxChPortTaggingCmd = CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E;


    /* set port tagging command */
    dxChPortTaggingCmd = (GT_FALSE == isTagged) ? CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E :
                                                  CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E;
    /* call device specific API */
    return cpssDxChBrgVlanMemberAdd(devNum, vlanId, portNum, isTagged, dxChPortTaggingCmd);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmBrgVlanMemberSet(devNum, vlanId, portNum, GT_TRUE, isTagged);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgVlanTagMemberAdd function
* @endinternal
*
* @brief   Set specific tagged member at VLAN entry
*
* @param[in] vlanId                   - VLAN Id
* @param[in] portNum                  - port number
* @param[in] portTaggingCmd           - port tagging command
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfBrgVlanTagMemberAdd
(
    IN GT_U16                            vlanId,
    IN GT_U32                             portNum,
    IN PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT portTaggingCmd
)
{
    GT_U32  portIter = 0;
#ifdef CHX_FAMILY
    CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT dxChPortTaggingCmd = CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_BOOL isTagged = GT_FALSE;
#endif /* EXMXPM_FAMILY */

    PRV_UTF_LOG3_MAC("prvTgfBrgVlanTagMemberAdd(%d,%d,%d) \n",
        vlanId,portNum,portTaggingCmd);

    /* find port index */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        if (prvTgfPortsArray[portIter] == portNum)
        {
            break;
        }
    }

    /* check if port is valid */
    if (prvTgfPortsArray[portIter] != portNum)
    {
        PRV_UTF_LOG1_MAC("[TGF]: Port [%d] is not valid", portNum);

        return GT_BAD_PARAM;
    }

#ifdef CHX_FAMILY
    /* set port tagging command */
    switch (portTaggingCmd)
    {
        case PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E:
            dxChPortTaggingCmd = CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E;
            break;

        case PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E:
            dxChPortTaggingCmd = CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E;
            break;

        case PRV_TGF_BRG_VLAN_PORT_TAG1_CMD_E:
            dxChPortTaggingCmd = CPSS_DXCH_BRG_VLAN_PORT_TAG1_CMD_E;
            break;

        case PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E:
            dxChPortTaggingCmd = CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E;
            break;

        case PRV_TGF_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E:
            dxChPortTaggingCmd = CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E;
            break;

        case PRV_TGF_BRG_VLAN_PORT_PUSH_TAG0_CMD_E:
            dxChPortTaggingCmd = CPSS_DXCH_BRG_VLAN_PORT_PUSH_TAG0_CMD_E;
            break;

        case PRV_TGF_BRG_VLAN_PORT_POP_OUTER_TAG_CMD_E:
            dxChPortTaggingCmd = CPSS_DXCH_BRG_VLAN_PORT_POP_OUTER_TAG_CMD_E;
            break;

        case PRV_TGF_BRG_VLAN_PORT_DO_NOT_MODIFY_TAG_CMD_E:
            dxChPortTaggingCmd = CPSS_DXCH_BRG_VLAN_PORT_DO_NOT_MODIFY_TAG_CMD_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* call device specific API */
    return cpssDxChBrgVlanMemberAdd(prvTgfDevsArray[portIter], vlanId, portNum, GT_TRUE, dxChPortTaggingCmd);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* check if port is tagged */
    isTagged = (PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E == portTaggingCmd) ? GT_FALSE : GT_TRUE;

    /* call device specific API */
    return cpssExMxPmBrgVlanMemberSet(prvTgfDevsArray[portIter], vlanId, portNum, GT_TRUE, isTagged);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgVlanMemberRemove function
* @endinternal
*
* @brief   Delete port member from vlan entry.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN Id
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vlanId
*/
GT_STATUS prvTgfBrgVlanMemberRemove
(
    IN GT_U8                devNum,
    IN GT_U16               vlanId,
    IN GT_U32                portNum
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgVlanPortDelete(devNum, vlanId, portNum);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmBrgVlanMemberSet(devNum, vlanId, portNum, GT_FALSE, GT_FALSE);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgVlanPortVidSet function
* @endinternal
*
* @brief   Set port's default VLAN Id.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] vlanId                   - VLAN Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong devNum port, or vlanId
*/
GT_STATUS prvTgfBrgVlanPortVidSet
(
    IN  GT_U8               devNum,
    IN  GT_PORT_NUM         portNum,
    IN  GT_U16              vlanId
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgVlanPortVidSet(devNum, portNum, CPSS_DIRECTION_INGRESS_E,vlanId);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmBrgVlanPortPvidSet(devNum, portNum, vlanId);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgVlanPortVidGet function
* @endinternal
*
* @brief   Get port's default VLAN Id
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] vlanIdPtr                - VLAN Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong devNum port, or vlanId
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvTgfBrgVlanPortVidGet
(
    IN  GT_U8               devNum,
    IN  GT_U32               portNum,
    OUT GT_U16             *vlanIdPtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgVlanPortVidGet(devNum, portNum, CPSS_DIRECTION_INGRESS_E,vlanIdPtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmBrgVlanPortPvidGet(devNum, portNum, vlanIdPtr);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgVlanPortVidPrecedenceSet function
* @endinternal
*
* @brief   Set Port VID Precedence
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] precedence               -  type
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfBrgVlanPortVidPrecedenceSet
(
    IN GT_U8                                        devNum,
    IN GT_U32                                        portNum,
    IN CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT  precedence
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgVlanPortVidPrecedenceSet(devNum, portNum, precedence);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmBrgVlanPortPvidPrecedenceSet(devNum, portNum, precedence);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgVlanPortForcePvidEnable function
* @endinternal
*
* @brief   Set Enable PVID Forcing
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   -  force PVid - GT_TRUE, disable - GT_FALSE
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfBrgVlanPortForcePvidEnable
(
    IN GT_U8    devNum,
    IN GT_U32    portNum,
    IN GT_BOOL  enable
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgVlanForcePvidEnable(
        devNum, portNum, enable);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmBrgVlanPortForcePvidEnableSet(
        devNum, portNum, enable);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgFdbMacEntryWrite function
* @endinternal
*
* @brief   Write the new entry in Hardware MAC address table in specified index.
*
* @param[in] index                    - hw mac entry index
* @param[in] skip                     - entry  control
* @param[in] macEntryPtr              - (pointer to) MAC entry parameters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum,saCommand,daCommand
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvTgfBrgFdbMacEntryWrite
(
    IN GT_U32                        index,
    IN GT_BOOL                       skip,
    IN PRV_TGF_BRG_MAC_ENTRY_STC    *macEntryPtr
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
#ifdef CHX_FAMILY
    CPSS_MAC_ENTRY_EXT_STC      dxChMacEntry;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_FDB_ENTRY_STC   exMxPmMacEntry;
#endif /* EXMXPM_FAMILY */

#ifdef CHX_FAMILY
    /* convert key data into device specific format */
    rc = prvTgfConvertGenericToDxChMacEntry(prvTgfDevNum, macEntryPtr, &dxChMacEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChMacEntry FAILED, rc = [%d]", rc);

        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        prvTgfBrgFdbMacEntryWrite_register(devNum, index, skip, macEntryPtr);
        /* call device specific API */
        AU_FDB_UPDATE_LOCK();
        rc = utfCpssDxChBrgFdbMacEntryWrite(devNum, index, skip, &dxChMacEntry);
        AU_FDB_UPDATE_UNLOCK();
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbMacEntryWrite FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* convert key data into device specific format */
    rc = prvTgfConvertGenericToExMxPmMacEntry(macEntryPtr, &exMxPmMacEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToExMxPmMacEntry FAILED, rc = [%d]", rc);

        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmBrgFdbEntryWrite(devNum, index, skip, &exMxPmMacEntry);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgFdbEntryWrite FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgFdbMacEntryRead function
* @endinternal
*
* @brief   Reads the new entry in Hardware MAC address table from specified index.
*
* @param[in] index                    - hw mac entry index
*
* @param[out] validPtr                 - (pointer to) is entry valid
* @param[out] skipPtr                  - (pointer to) is entry skip control
* @param[out] agedPtr                  - (pointer to) is entry aged
* @param[out] hwDevNumPtr              - (pointer to) is HW device number associated with the entry
* @param[out] macEntryPtr              - (pointer to) extended Mac table entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - index out of range
*/
GT_STATUS prvTgfBrgFdbMacEntryRead
(
    IN  GT_U32                        index,
    OUT GT_BOOL                      *validPtr,
    OUT GT_BOOL                      *skipPtr,
    OUT GT_BOOL                      *agedPtr,
    OUT GT_HW_DEV_NUM                *hwDevNumPtr,
    OUT PRV_TGF_BRG_MAC_ENTRY_STC    *macEntryPtr
)
{
    GT_STATUS   rc = GT_OK;
#ifdef CHX_FAMILY
    CPSS_MAC_ENTRY_EXT_STC      dxChMacEntry;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_FDB_ENTRY_STC   exMxPmMacEntry;
    GT_U8                       associatedDevNum;
#endif /* EXMXPM_FAMILY */

#ifdef CHX_FAMILY
    /* call device specific API */
    AU_FDB_UPDATE_LOCK();
    rc = utfCpssDxChBrgFdbMacEntryRead(prvTgfDevNum, index, validPtr, skipPtr,
                                    agedPtr, hwDevNumPtr, &dxChMacEntry);
    AU_FDB_UPDATE_UNLOCK();
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbMacEntryRead FAILED, rc = [%d]", rc);

        return rc;
    }

    if (dxChMacEntry.key.entryType == CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E)
    {
        /* hwDevNumPtr relevant only in case if entry type is CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E */

        /* convert hwDevNumPtr to SW devNum.*/
        rc = prvUtfSwFromHwDeviceNumberGet(*hwDevNumPtr, hwDevNumPtr);
        if(GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvUtfSwFromHwDeviceNumberGet FAILED, rc = [%d]", rc);
            return rc;
        }
    }


    /* convert key data from device specific format */
    return prvTgfConvertDxChToGenericMacEntry(&dxChMacEntry, macEntryPtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    rc = cpssExMxPmBrgFdbEntryRead(prvTgfDevNum, index,validPtr, skipPtr,
                                   agedPtr, &associatedDevNum, &exMxPmMacEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgFdbEntryRead FAILED, rc = [%d]", rc);

        return rc;
    }

    *hwDevNumPtr = associatedDevNum;

    /* convert key data from device specific format */
    return prvTgfConvertExMxPmToGenericMacEntry(&exMxPmMacEntry, macEntryPtr);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgFdbMacEntryInvalidate function
* @endinternal
*
* @brief   Invalidate an entry in Hardware MAC address table in specified index.
*         the invalidation done by resetting to first word of the entry
* @param[in] index                    - hw mac entry index.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_OUT_OF_RANGE          - index out of range
*/
GT_STATUS prvTgfBrgFdbMacEntryInvalidate
(
    IN GT_U32               index
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
    GT_PORT_GROUPS_BMP portGroupBmp;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    portGroupBmp = (usePortGroupsBmp == GT_FALSE) ? CPSS_PORT_GROUP_UNAWARE_MODE_CNS : currPortGroupsBmp;

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        rc = prvTgfBrgFdbMacEntryInvalidatePerDev(devNum,
                                                  portGroupBmp,
                                                  index);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfBrgFdbMacEntryInvalidatePerDev FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
}

/**
* @internal prvTgfBrgFdbEntryByMessageMustSucceedSet function
* @endinternal
*
* @brief   when add/delete FDB entry 'by message' operation can not succeed.
*         the operation MUST finish but is may not succeed.
*         An AU message sent by CPU can fail in the following cases:
*         1. The message type = CPSS_NA_E and the hash chain has reached it's
*         maximum length.
*         2. The message type = CPSS_QA_E or CPSS_AA_E and the FDB entry doesn't
*         exist.
* @param[in] mustSucceed              - GT_TRUE - operation must succeed
*                                      GT_FALSE - operation may fail (must finish but not succeed)
*                                       the previous state of the flag
*/
GT_BOOL prvTgfBrgFdbEntryByMessageMustSucceedSet
(
    IN GT_BOOL     mustSucceed
)
{
    GT_BOOL previousState = fdbEntryByMessageMustSucceed;

    fdbEntryByMessageMustSucceed = mustSucceed ;

    return previousState;
}
/**
* @internal prvTgfBrgFdbEntryByMessageStatusCheckSet function
* @endinternal
*
* @brief   The function enables/disables status of 'by message' API call.
*
* @param[in] mustCheck                - GT_TRUE - operation status must be checked
*                                      GT_FALSE - operation status must not be checked
*                                       the previous state of the flag
*/
GT_BOOL prvTgfBrgFdbEntryByMessageStatusCheckSet
(
    IN GT_BOOL     mustCheck
)
{
    GT_BOOL previousState = fdbEntryByMessageMustCheck;

    fdbEntryByMessageMustCheck = mustCheck ;

    return previousState;
}

/**
* @internal prvTgfBrgFdbMacEntrySet function
* @endinternal
*
* @brief   Create new or update existing entry in Hardware MAC address table
*
* @param[in] macEntryPtr              - (pointer to) mac table entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - aging timeout exceeds hardware limitation.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvTgfBrgFdbMacEntrySet
(
    IN PRV_TGF_BRG_MAC_ENTRY_STC     *macEntryPtr
)
{
    GT_U8       devNum    = 0;
    GT_STATUS   rc, rc1   = GT_OK;
    GT_U32  index;/* fdb index */
#ifdef CHX_FAMILY
    CPSS_MAC_ENTRY_EXT_STC      dxChMacEntry;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_BOOL     completed = GT_FALSE;
    GT_BOOL     succeeded = GT_FALSE;
    CPSS_EXMXPM_FDB_ENTRY_STC   exMxPmMacEntry;
#endif /* EXMXPM_FAMILY */

    if(macEntryPtr->dstInterface.type == CPSS_INTERFACE_VID_E
        && prvTgfDevPortForceNum)
    {
        PRV_UTF_LOG0_MAC("========== prvTgfBrgFdbMacEntrySet : WorkAround !!! : \n");
        PRV_UTF_LOG0_MAC("========== Limitation for multi port group FDB lookup system : set FDB entry on interface VID is not allowed \n");
        PRV_UTF_LOG0_MAC("========== assumption that traffic from the test not really uses this FDB entry (bypass ingress / bypass bridge / routed..)  \n");
        PRV_UTF_LOG0_MAC("========== so replace it with 'FDB on VIDX 0x555'  \n");
        macEntryPtr->dstInterface.type = CPSS_INTERFACE_VIDX_E;
        macEntryPtr->dstInterface.vidx = 0x555;/* test that fail due to this 'WA' must be skipped using :
        PRV_TGF_MULTI_PORT_GROUP_FDB_LOOKUP_SYSTEM_LIMITATION_REDIRECT_TO_VID_INTERFACE_FROM_FDB_CHECK_MAC(testName) */
    }

    if(prvTgfBrgFdbAccessMode == PRV_TGF_BRG_FDB_ACCESS_MODE_BY_INDEX_E)
    {
        /* allow to set the entry by index */

        /* calculate and find the index */
        rc = prvTgfBrgFdbMacEntryIndexFind(&macEntryPtr->key,&index);
        if(rc != GT_FULL)
        {
            if (GT_OK != rc && GT_NOT_FOUND != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfBrgFdbMacEntryIndexFind FAILED, rc = [%d]", rc);

                return rc;
            }

            return prvTgfBrgFdbMacEntryWrite(index,GT_FALSE,macEntryPtr);
        }
        else
        {
            /*
                NOTE: GT_FULL means that BUCKET is FULL --> the operation would
                    have failed on 'by messages' !!!


                fall through to do the 'by message' action
            */

            rc = GT_OK;
        }
    }


#ifdef CHX_FAMILY
    /* convert key data into device specific format */
    rc = prvTgfConvertGenericToDxChMacEntry(prvTgfDevNum, macEntryPtr, &dxChMacEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChMacEntry FAILED, rc = [%d]", rc);

        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* convert key data into device specific format */
    rc = prvTgfConvertGenericToExMxPmMacEntry(macEntryPtr, &exMxPmMacEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToExMxPmMacEntry FAILED, rc = [%d]", rc);

        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);
#endif /* EXMXPM_FAMILY */

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
#ifdef CHX_FAMILY
        /* call device specific API */
        AU_FDB_UPDATE_LOCK();
        prvTgfBrgFdbMacEntrySet_register(devNum, 0xFFFFFFFF,macEntryPtr);
        rc = utfCpssDxChBrgFdbMacEntrySet(devNum, &dxChMacEntry);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbMacEntrySet FAILED, rc = [%d]", rc);

            rc1 = rc;
            AU_FDB_UPDATE_UNLOCK();
            continue;
        }

        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(devNum,fdbEntryByMessageMustSucceed);
        AU_FDB_UPDATE_UNLOCK();
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
        /* call device specific API */
        rc = cpssExMxPmBrgFdbEntrySet(devNum, &exMxPmMacEntry);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgFdbEntrySet FAILED, rc = [%d]", rc);

            rc1 = rc;

            continue;
        }
        /* synchronize call */
        completed = GT_FALSE;
        succeeded = GT_FALSE;
        do {
            cpssExMxPmBrgFdbFromCpuAuMsgStatusGet(devNum, CPSS_QA_E, &completed, &succeeded);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgFdbFromCpuAuMsgStatusGet FAILED, rc = [%d]", rc);

                rc1 = rc;
            }
        } while ((GT_FALSE == completed) && (GT_FALSE == succeeded));
#endif /* EXMXPM_FAMILY */
    }

    return rc1;
}

/**
* @internal prvTgfBrgFdbMacEntryGet function
* @endinternal
*
* @brief   Get existing entry in Hardware MAC address table
*
* @param[in] macEntryKeyPtr           - (pointer to) mac entry key
*
* @param[out] macEntryPtr              - (pointer to) mac table entry
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - entry not found
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - aging timeout exceeds hardware limitation.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvTgfBrgFdbMacEntryGet
(
    IN  PRV_TGF_MAC_ENTRY_KEY_STC    *macEntryKeyPtr,
    OUT PRV_TGF_BRG_MAC_ENTRY_STC    *macEntryPtr
)
{
    GT_STATUS           rc    = GT_OK;
    GT_U32              index = 0;
    GT_BOOL             valid = GT_FALSE;
    GT_BOOL             skip  = GT_FALSE;
    GT_BOOL             aged  = GT_FALSE;
    GT_HW_DEV_NUM       hwDev   = 0;

    if(prvTgfNumOfPortGroups && usePortGroupsBmp == GT_FALSE)
    {
        GT_U32      jj;
        /* save original value*/
        GT_U32 origCurrPortGroupsBmp = currPortGroupsBmp;

        rc = GT_FAIL;

        /***************************************************/
        /* lookup in all the port groups for the FDB entry */
        /***************************************************/

        usePortGroupsBmp = GT_TRUE;
        for(jj = 0 ; jj < prvTgfNumOfPortGroups ; jj++)
        {
            currPortGroupsBmp = 1 << jj;
            rc = prvTgfBrgFdbMacEntryIndexFind(macEntryKeyPtr,&index);
            if(rc == GT_NOT_FOUND || rc == GT_FULL)
            {
                /* try next port group */
                continue;
            }
            else /* (rc != GT_OK) || (rc == GT_OK) */
            {
                break;
            }
        }

        if(rc == GT_NOT_FOUND || rc == GT_FULL || (jj == prvTgfNumOfPortGroups))
        {
            rc = GT_NOT_FOUND;
        }
        else if (rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfBrgFdbMacEntryIndexFind 1 FAILED, rc = [%d]", rc);
        }

        if(rc == GT_OK)
        {
            /* read MAC entry */
            rc = prvTgfBrgFdbMacEntryRead(index, &valid, &skip, &aged, &hwDev, macEntryPtr);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfBrgFdbMacEntryRead 1 FAILED, rc = [%d]", rc);
            }
        }

        /* restore original values */
        currPortGroupsBmp = origCurrPortGroupsBmp;
        usePortGroupsBmp = GT_FALSE;

        return rc;
    }

    /* find the proper index */
    rc = prvTgfBrgFdbMacEntryIndexFind(macEntryKeyPtr,&index);
    if(rc == GT_NOT_FOUND || rc == GT_FULL)
    {
        return GT_NOT_FOUND;
    }
    else if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfBrgFdbMacEntryIndexFind FAILED, rc = [%d]", rc);
        return rc;
    }

    /* read MAC entry */
    rc = prvTgfBrgFdbMacEntryRead(index, &valid, &skip, &aged, &hwDev, macEntryPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfBrgFdbMacEntryRead FAILED, rc = [%d]", rc);

        return rc;
    }

    return GT_OK;
}

/**
* @internal prvTgfBrgFdbMacEntryIndexFind_MultiHash function
* @endinternal
*
* @brief   for CRC multi hash mode.
*         function calculate hash index for this mac address , and then start to read
*         from HW the entries to find an existing entry that match the key.
* @param[in] macEntryKeyPtr           - (pointer to) mac entry key
*
* @param[out] indexPtr                 - (pointer to) :
*                                      when GT_OK (found) --> index of the entry
*                                      when GT_NOT_FOUND (not found) --> first index that can be used
*                                      when GT_FULL (not found) --> index first index
*                                      on other return value --> not relevant
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - entry not found, but bucket is not FULL
* @retval GT_FULL                  - entry not found, but bucket is FULL
* @retval GT_BAD_PARAM             - wrong devNum.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvTgfBrgFdbMacEntryIndexFind_MultiHash
(
    IN  PRV_TGF_MAC_ENTRY_KEY_STC    *macEntryKeyPtr,
    OUT GT_U32                       *indexPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS           rc    ;
    GT_U32              index = 0;
    GT_BOOL             valid ;
    GT_BOOL             skip  ;
    GT_BOOL             aged  ;
    GT_HW_DEV_NUM       hwDev   ;
    GT_U32      ii;
    PRV_TGF_BRG_MAC_ENTRY_STC    macEntry;
    GT_U32      firstEmptyIndex = NOT_INIT_CNS;
    GT_U32      numOfBanks;
    GT_U32      skipBanks = 1;
    CPSS_MAC_ENTRY_EXT_KEY_STC  dxChMacEntryKey;
    GT_U32       crcMultiHashArr[16];

    numOfBanks = PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->bridge.fdbHashParams.numOfBanks;

    /* convert key data into device specific format */
    rc = prvTgfConvertGenericToDxChMacEntryKey(macEntryKeyPtr, &dxChMacEntryKey);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChMacEntryKey FAILED, rc = [%d]", rc);

        return rc;
    }

    rc = cpssDxChBrgFdbHashCrcMultiResultsCalc(prvTgfDevNum,&dxChMacEntryKey,
        0,
        numOfBanks,
        crcMultiHashArr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbHashCrcMultiResultsCalc FAILED, rc = [%d]", rc);

        return rc;
    }

    if ((macEntryKeyPtr->entryType == PRV_TGF_FDB_ENTRY_TYPE_IPV6_UC_ADDR_ENTRY_E) &&
        (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum)))
    {
        /* IPv6 UC 'Address' entry must be in even bankId */
        skipBanks = 2;
    }
    for(ii = 0 ; ii < numOfBanks ; ii += skipBanks)
    {
        index = crcMultiHashArr[ii];

        /* read MAC entry */
        rc = prvTgfBrgFdbMacEntryRead(index, &valid, &skip, &aged, &hwDev, &macEntry);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfBrgFdbMacEntryRead FAILED, rc = [%d]", rc);

            return rc;
        }

        if(valid /*&& skip == GT_FALSE*/)/* in multi hash - not care about 'skip' bit !*/
        {
            /* used entry */
            if((0 == prvTgfBrgFdbMacKeyCompare(macEntryKeyPtr,&macEntry.key)))
            {
                *indexPtr = index;
                return GT_OK;
            }
        }
        else
        {
            /* UN-used entry */
            if(firstEmptyIndex == NOT_INIT_CNS)
            {
                if ((macEntryKeyPtr->entryType == PRV_TGF_FDB_ENTRY_TYPE_IPV6_UC_ADDR_ENTRY_E) &&
                    (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum)))
                {
                    /* Entry following IPv6 UC 'Address' entry must also be empty */
                    /* read MAC entry */
                    rc = prvTgfBrgFdbMacEntryRead(crcMultiHashArr[ii+1], &valid, &skip, &aged, &hwDev, &macEntry);
                    if (GT_OK != rc)
                    {
                        PRV_UTF_LOG1_MAC("[TGF]: prvTgfBrgFdbMacEntryRead_II FAILED, rc = [%d]", rc);

                        return rc;
                    }

                    if(valid == GT_FALSE)
                    {
                        firstEmptyIndex = index;
                    }
                }
                else
                {
                    firstEmptyIndex = index;
                }
            }
        }
    }

    if(firstEmptyIndex == NOT_INIT_CNS)
    {
        /* all entries are used */
        /* the returned index is of 'first' */
        *indexPtr = index;
        return GT_FULL;
    }
    else
    {
        /* the returned index is of first empty entry */
        *indexPtr = firstEmptyIndex;
        return GT_NOT_FOUND;
    }
#else /*CHX_FAMILY*/
    return GT_NOT_SUPPORTED;
#endif /*!CHX_FAMILY*/
}

/**
* @internal prvTgfBrgFdbMacEntryIndexFind function
* @endinternal
*
* @brief   function calculate hash index for this mac address , and then start to read
*         from HW the entries to find an existing entry that match the key.
* @param[in] macEntryKeyPtr           - (pointer to) mac entry key
*
* @param[out] indexPtr                 - (pointer to) :
*                                      when GT_OK (found) --> index of the entry
*                                      when GT_NOT_FOUND (not found) --> first index that can be used
*                                      when GT_FULL (not found) --> index od start of bucket
*                                      on other return value --> not relevant
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - entry not found, but bucket is not FULL
* @retval GT_FULL                  - entry not found, but bucket is FULL
* @retval GT_BAD_PARAM             - wrong devNum.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvTgfBrgFdbMacEntryIndexFind
(
    IN  PRV_TGF_MAC_ENTRY_KEY_STC    *macEntryKeyPtr,
    OUT GT_U32                       *indexPtr
)
{
    GT_STATUS           rc    ;
    GT_U32              index ;
    GT_BOOL             valid ;
    GT_BOOL             skip  ;
    GT_BOOL             aged  ;
    GT_HW_DEV_NUM       hwDev   ;
    GT_U32      ii;
    PRV_TGF_BRG_MAC_ENTRY_STC    macEntry;
    GT_U32      firstEmptyIndex = NOT_INIT_CNS;

#ifdef CHX_FAMILY
    if(PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->bridge.fdbHashParams.hashMode ==
        CPSS_MAC_HASH_FUNC_CRC_MULTI_HASH_E)
    {
        return prvTgfBrgFdbMacEntryIndexFind_MultiHash(macEntryKeyPtr,indexPtr);
    }
#endif /*CHX_FAMILY*/

    /* calculate the hash index */
    rc = prvTgfBrgFdbMacEntryHashCalc(macEntryKeyPtr,&index);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfBrgFdbMacEntryHashCalc FAILED, rc = [%d]", rc);

        return rc;
    }

    for(ii = 0 ; ii < prvTgfBrgFdbHashLengthLookup ; ii++)
    {
        /* read MAC entry */
        rc = prvTgfBrgFdbMacEntryRead((index + ii), &valid, &skip, &aged, &hwDev, &macEntry);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfBrgFdbMacEntryIndexFind FAILED, rc = [%d]", rc);

            return rc;
        }

        if(valid && skip == GT_FALSE)
        {
            /* used entry */
            if((0 == prvTgfBrgFdbMacKeyCompare(macEntryKeyPtr,&macEntry.key)))
            {
                *indexPtr = index + ii;
                return GT_OK;
            }
        }
        else
        {
            /* UN-used entry */
            if(firstEmptyIndex == NOT_INIT_CNS)
            {
                firstEmptyIndex = index + ii;
            }
        }
    }

    if(firstEmptyIndex == NOT_INIT_CNS)
    {
        /* all entries are used */
        /* the returned index is of start of bucket */
        *indexPtr = index;
        return GT_FULL;
    }
    else
    {
        /* the returned index is of first empty entry */
        *indexPtr = firstEmptyIndex;
        return GT_NOT_FOUND;
    }
}


/**
* @internal prvTgfBrgFdbMacEntryDelete function
* @endinternal
*
* @brief   Delete an old entry in Hardware MAC address table
*
* @param[in] macEntryKeyPtr           - (pointer to) key parameters of the mac entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vlanId
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvTgfBrgFdbMacEntryDelete
(
    IN PRV_TGF_MAC_ENTRY_KEY_STC     *macEntryKeyPtr
)
{
    GT_U8       devNum    = 0;
    GT_STATUS   rc, rc1   = GT_OK;
    GT_PORT_GROUPS_BMP portGroupsBmp;

    portGroupsBmp = (GT_FALSE==usePortGroupsBmp) ?
        CPSS_PORT_GROUP_UNAWARE_MODE_CNS : currPortGroupsBmp;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        rc = prvTgfBrgFdbMacEntryDeletePerDev(devNum, portGroupsBmp, macEntryKeyPtr);
        if (GT_OK != rc )
        {
            rc1 = rc;
        }
    }
    return rc1;
}

/**
* @internal prvTgfBrgFdbQaSend function
* @endinternal
*
* @brief   The function Send Query Address (QA) message to the hardware MAC address
*         table.
* @param[in] macEntryKeyPtr           - (pointer to) key parameters of the mac entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong vlanId
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvTgfBrgFdbQaSend
(
    IN PRV_TGF_MAC_ENTRY_KEY_STC     *macEntryKeyPtr
)
{
    GT_U8       devNum    = 0;
    GT_STATUS   rc, rc1   = GT_OK;
#ifdef CHX_FAMILY
    CPSS_MAC_ENTRY_EXT_KEY_STC  dxChMacEntryKey;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_BOOL     completed = GT_FALSE;
    GT_BOOL     succeeded = GT_FALSE;
    CPSS_EXMXPM_FDB_ENTRY_KEY_STC exMxPmMacEntryKey;
#endif /* EXMXPM_FAMILY */

#ifdef CHX_FAMILY
    /* convert key data into device specific format */
    rc = prvTgfConvertGenericToDxChMacEntryKey(macEntryKeyPtr, &dxChMacEntryKey);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChMacEntryKey FAILED, rc = [%d]", rc);

        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* convert key data into device specific format */
    rc = prvTgfConvertGenericToExMxPmMacEntryKey(macEntryKeyPtr, &exMxPmMacEntryKey);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToExMxPmMacEntryKey FAILED, rc = [%d]", rc);

        return rc;
    }

    /* prepare device iterator */
    rc = prvUtfNextDeviceReset(&devNum, UTF_EXMXPM_FAMILY_SET_CNS);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvUtfNextDeviceReset FAILED, rc = [%d]", rc);

        return rc;
    }
#endif /* EXMXPM_FAMILY */

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
#ifdef CHX_FAMILY
        /* call device specific API */
        AU_FDB_UPDATE_LOCK();
        rc = utfCpssDxChBrgFdbQaSend(devNum, &dxChMacEntryKey);
        if (GT_OK != rc)
        {
            if (fdbEntryByMessageMustCheck == GT_TRUE)
            {
                PRV_UTF_LOG1_MAC("[TGF]: utfCpssDxChBrgFdbQaSend FAILED, rc = [%d]", rc);
            }
            rc1 = rc;

            AU_FDB_UPDATE_UNLOCK();

            continue;
        }

        if (fdbEntryByMessageMustCheck == GT_TRUE)
        {
            utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(devNum,fdbEntryByMessageMustSucceed);
        }
        AU_FDB_UPDATE_UNLOCK();
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
        /* call device specific API */
        rc = cpssExMxPmBrgFdbQaSend(devNum, &exMxPmMacEntryKey);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgFdbQaSend FAILED, rc = [%d]", rc);

            rc1 = rc;

            continue;
        }
        /* synchronize call */
        completed = GT_FALSE;
        succeeded = GT_FALSE;
        do {
            cpssExMxPmBrgFdbFromCpuAuMsgStatusGet(devNum, CPSS_QA_E, &completed, &succeeded);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgFdbFromCpuAuMsgStatusGet FAILED, rc = [%d]", rc);

                rc1 = rc;
            }
        } while ((GT_FALSE == completed) && (GT_FALSE == succeeded));
#endif /* EXMXPM_FAMILY */
    }

    return rc1;
}

#ifdef CHX_FAMILY

GT_STATUS testTimeOfReadWriteRegister(
    IN GT_U8    devNum,
    IN GT_U32   regAddr,
    IN GT_U32   value,
    IN GT_BOOL  doWrite,
    IN GT_U32   numOfLoops
)
{
    GT_STATUS   rc;
    GT_U32      ii;
    GT_U32      timeOfOperation;
    GT_U32      tempReadVal;

    if(numOfLoops == 0)
    {
        numOfLoops = 1000;
    }

    ii = numOfLoops;

    timeOfOperation = cpssOsTickGet();

    if(doWrite)
    {
        do
        {
            rc = cpssDrvPpHwRegisterWrite(devNum,0,regAddr,value);
            if(rc != GT_OK)
            {
                return rc;
            }
        }while(ii--);
    }
    else
    {
        do
        {
            rc = cpssDrvPpHwRegisterRead(devNum,0,regAddr,&tempReadVal);
            if(rc != GT_OK)
            {
                return rc;
            }
        }while(ii--);
    }

    timeOfOperation = cpssOsTickGet() - timeOfOperation;

    cpssOsPrintf("total running [%d] ticks (for [%d] loops) \n",
        timeOfOperation,numOfLoops);
    cpssOsPrintf("average per single %s [%d] ticks  , for register[0x%8.8x] \n",
        doWrite ? "write":"read",
        timeOfOperation / numOfLoops,
        regAddr);

    return GT_OK;
}

static GT_U32 maxTableLoops = 1;

/* do flush FDB by delete valid entries 'one by one' */
GT_STATUS tgfFlushByDeleteAllEntries(
    IN GT_U8    devNum,
    IN GT_BOOL includeStatic
)
{
    GT_STATUS   rc;
    GT_U32  fdbSize , entryIndex , valid , skip,isStatic ;
    static GT_U32   hwData[8],hwValue;
    GT_U32  numInvalidated = 0;
    GT_U32      timeOfOperation;

    FDB_SHADOW_NOT_SUPPORT_FDB_HW_ITERATIONS;

    rc = cpssDxChCfgTableNumEntriesGet(devNum,CPSS_DXCH_CFG_TABLE_FDB_E,&fdbSize);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(fdbSize > maxTableLoops)
    {
        fdbSize = maxTableLoops;
    }

    timeOfOperation = cpssOsTickGet();

    for( entryIndex = 0 ; entryIndex < fdbSize; entryIndex++)
    {

        rc = prvCpssDxChReadTableEntry(devNum,
                                       CPSS_DXCH_TABLE_FDB_E,
                                       entryIndex,
                                       &hwData[0]);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            /* valid bit */
            SIP5_FDB_ENTRY_FIELD_GET_MAC(devNum,hwData,
                SIP5_FDB_FDB_TABLE_FIELDS_VALID_E,
                hwValue)
            valid = hwValue;

            /* skip bit */
            SIP5_FDB_ENTRY_FIELD_GET_MAC(devNum,hwData,
                SIP5_FDB_FDB_TABLE_FIELDS_SKIP_E,
                hwValue)
            skip = hwValue;

            /*entryType*/
            SIP5_FDB_ENTRY_FIELD_GET_MAC(devNum,hwData,
                SIP5_FDB_FDB_TABLE_FIELDS_FDB_ENTRY_TYPE_E,
                hwValue);
            if(hwValue < 2)
            {
                /* static bit */
                SIP5_FDB_ENTRY_FIELD_GET_MAC(devNum,hwData,
                    SIP5_FDB_FDB_TABLE_FIELDS_IS_STATIC_E,
                    hwValue)
                isStatic = hwValue;
            }
            else
            {
                isStatic = 1;
            }

        }
        else
        {
            U32_GET_FIELD_IN_ENTRY_MAC(hwData,0,1,valid);
            U32_GET_FIELD_IN_ENTRY_MAC(hwData,1,1,skip);
            U32_GET_FIELD_IN_ENTRY_MAC(hwData,89,1,isStatic);
        }

        if(valid == 0 || skip == 1)
        {
            continue;
        }


        if(isStatic && includeStatic == GT_FALSE)
        {
            continue;
        }

        numInvalidated++;

        hwData[0] = 0; /* invalidate the entry */
        rc = prvCpssDxChWriteTableEntry(devNum,
                                       CPSS_DXCH_TABLE_FDB_E,
                                       entryIndex,
                                       &hwData[0]);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    timeOfOperation = cpssOsTickGet() - timeOfOperation;

    cpssOsPrintf("total running [%d] ticks (for [%d] loops) \n",
        timeOfOperation,fdbSize);
    cpssOsPrintf("average per single entry [%d] ticks  \n",
        timeOfOperation / fdbSize);

    PRV_UTF_LOG1_MAC("tgfFlushByDeleteAllEntries: invalidated [%d] entries \n",numInvalidated);

    return GT_OK;
}

#endif/*CHX_FAMILY*/

/* indication to force the FDB flush with delete of UC route entries */
static GT_BOOL forceFdbFlushWithUcEntries = GT_FALSE;
/**
* @internal prvTgfBrgFdbFlushWithUcEntries function
* @endinternal
*
* @brief   state to force the FDB flush with delete of UC route entries.
*
* @param[in] enable                   - GT_TRUE - force the FDB flush with delete of UC route entries.
*                                      GT_FALSE - do not force the FDB flush with delete of UC route entries.
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvTgfBrgFdbFlushWithUcEntries
(
    IN GT_BOOL                        enable
)
{
    forceFdbFlushWithUcEntries = enable;
    return GT_OK;
}


/**
* @internal prvTgfBrgFdbFlush function
* @endinternal
*
* @brief   Flush FDB table (with/without static entries).
*         function sets actDev and it's mask to 'dont care' so all entries can be
*         flushed (function restore default values at end of operation).
* @param[in] includeStatic            - include static entries
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
*/
GT_STATUS prvTgfBrgFdbFlush
(
    IN GT_BOOL                        includeStatic
)
{
    GT_U8       devNum        = 0;
    GT_STATUS   rc, rc1       = GT_OK;
    GT_BOOL     isCompleted   = GT_FALSE;
    GT_BOOL     isAuqFull;/*is the AUQ full */
    GT_BOOL     messagesToCpuEnabled = GT_FALSE;
#ifdef ASIC_SIMULATION
    #ifdef EXMXPM_FAMILY
        GT_U32      maxRetry      = 100;/* 100*50ms = 5 seconds --> should be enough when no messages to CPU are sent */
    #endif /* EXMXPM_FAMILY */
    #ifdef CHX_FAMILY
        GT_U32      maxRetry      = 200;/* 200*50ms = 10 second --> should be enough when no messages to CPU are sent */
    #endif /* CHX_FAMILY */
#endif /*ASIC_SIMULATION*/

#ifdef CHX_FAMILY
    GT_U32   origActDev;/*Action active device */
    GT_U32   origActDevMask;/*Action active mask*/
    GT_U32   origActUerDefined = 0;       /*action active UerDefined*/
    GT_U32   origAactUerDefinedMask = 0;  /*action active UerDefined Mask*/
    GT_U32   origVlanId,origVlanMask;
    CPSS_DXCH_BRG_FDB_AGE_TRUNK_MODE_ENT origTrunkAgingMode;
    GT_BOOL  routeUcMessagesToCpuEnabled = GT_FALSE;
    GT_BOOL  routeUcDeleteEnabled = GT_FALSE;
    GT_BOOL  ipmcAddrDelEnabled = GT_FALSE;
    CPSS_DXCH_BRG_FDB_DEL_MODE_ENT oldDeletionMode=CPSS_DXCH_BRG_FDB_DEL_MODE_DYNAMIC_ONLY_E;
    CPSS_DXCH_BRG_FDB_DEL_MODE_ENT deletionMode=CPSS_DXCH_BRG_FDB_DEL_MODE_DYNAMIC_ONLY_E;

#endif /* CHX_FAMILY */
#ifdef EXMXPM_FAMILY
    GT_BOOL  staticEnabled = GT_FALSE;
#endif /* EXMXPM_FAMILY */

    if(includeStatic == GT_TRUE)
    {
        PRV_UTF_LOG0_MAC("prvTgfBrgFdbFlush: with static \n");
    }
    else
    {
        PRV_UTF_LOG0_MAC("prvTgfBrgFdbFlush: dynamic only \n");
    }

    /* AUTODOC: flush FDB entries */
#ifdef CHX_FAMILY
    deletionMode = (includeStatic) ? CPSS_DXCH_BRG_FDB_DEL_MODE_ALL_E : CPSS_DXCH_BRG_FDB_DEL_MODE_DYNAMIC_ONLY_E;

    if(prvTgfFdbShadowUsed == GT_TRUE)
    {
        /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);
        /* go over all active devices */
        while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
        {
            prvTgfBrgFdbFlushByShadowPart1(devNum, deletionMode);
            prvTgfBrgFdbFlushByShadowPart2(devNum, deletionMode);
        }

        PRV_UTF_LOG0_MAC("prvTgfBrgFdbFlush: (FROM SHADOW) ended \n");
        /* finished all devices */
        return GT_OK;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);
    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        isCompleted   = GT_FALSE;
        oldDeletionMode = CPSS_DXCH_BRG_FDB_DEL_MODE_DYNAMIC_ONLY_E;
        messagesToCpuEnabled = GT_FALSE;
        routeUcMessagesToCpuEnabled = GT_FALSE;
        routeUcDeleteEnabled = GT_FALSE;
        ipmcAddrDelEnabled = GT_FALSE;

        /* get original act device info */
        rc = cpssDxChBrgFdbActionActiveDevGet(devNum,&origActDev,&origActDevMask);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbActionActiveDevGet FAILED, rc = [%d] \n", rc);

            rc1 = rc;
            continue;
        }

        /* get FDB flush status */
        rc = cpssDxChBrgFdbStaticDelEnableGet(devNum, &oldDeletionMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbStaticDelEnableGet FAILED, rc = [%d] \n", rc);

            rc1 = rc;
            continue;
        }

        if(deletionMode != oldDeletionMode)
        {
            /* set FDB flush status */
            rc = cpssDxChBrgFdbStaticDelEnable(devNum, deletionMode);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbStaticDelEnable FAILED, rc = [%d] \n", rc);

                rc1 = rc;
                continue;
            }
        }

        /* disable AA and TA messages to CPU */
        rc = cpssDxChBrgFdbAAandTAToCpuGet(devNum, &messagesToCpuEnabled);
        if ((rc == GT_OK) && messagesToCpuEnabled)
        {
            rc = cpssDxChBrgFdbAAandTAToCpuSet(devNum, GT_FALSE);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbAAandTAToCpuSet FAILED, rc = [%d] \n", rc);
            }
        }
        else
        {
            if(rc != GT_OK)
            {
                PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbAAandTAToCpuGet FAILED, rc = [%d] \n", rc);
            }
            messagesToCpuEnabled = GT_FALSE;
        }

        if(GT_TRUE == PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum))
        {
            rc = cpssDxChBrgFdbRoutingUcDeleteEnableGet(devNum,&routeUcDeleteEnabled);
            if(rc == GT_OK && includeStatic && forceFdbFlushWithUcEntries && routeUcDeleteEnabled == GT_FALSE)
            {
                routeUcDeleteEnabled = GT_TRUE;

                rc = cpssDxChBrgFdbRoutingUcDeleteEnableSet(devNum,routeUcDeleteEnabled);
                if (GT_OK != rc)
                {
                    PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbRoutingUcDeleteEnableSet FAILED, rc = [%d] \n", rc);
                }
            }
            else
            {
                if (GT_OK != rc)
                {
                    PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbRoutingUcDeleteEnableGet FAILED, rc = [%d] \n", rc);
                }
                routeUcDeleteEnabled = GT_FALSE;
            }

            rc = cpssDxChBrgFdbIpmcAddrDelEnableGet(devNum , &ipmcAddrDelEnabled);
            if ((rc == GT_OK) && (ipmcAddrDelEnabled == GT_FALSE))
            {
                /* enable the IPMC delete entries */
                rc = cpssDxChBrgFdbIpmcAddrDelEnableSet(devNum , GT_TRUE);
                if (GT_OK != rc)
                {
                    PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbIpmcAddrDelEnableSet FAILED, rc = [%d] \n", rc);
                }
            }
            else
            {
                ipmcAddrDelEnabled = GT_FALSE;
                if (GT_OK != rc)
                {
                    PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbIpmcAddrDelEnableGet FAILED, rc = [%d] \n", rc);
                }
            }


            /* disable AA and TA messages to CPU for UC Route Entries */
            rc = cpssDxChBrgFdbRoutingUcAAandTAToCpuGet(devNum, &routeUcMessagesToCpuEnabled);
            if ((rc == GT_OK) && routeUcMessagesToCpuEnabled)
            {
                rc = cpssDxChBrgFdbRoutingUcAAandTAToCpuSet(devNum, GT_FALSE);
                if (GT_OK != rc)
                {
                    PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbRoutingUcAAandTAToCpuSet FAILED, rc = [%d] \n", rc);
                }
            }
            else
            {
                if(rc != GT_OK)
                {
                    PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbRoutingUcAAandTAToCpuGet FAILED, rc = [%d] \n", rc);
                }
                routeUcMessagesToCpuEnabled = GT_FALSE;
            }
        }

        /* set new act device info */
        rc = cpssDxChBrgFdbActionActiveDevSet(devNum,0,0);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbActionActiveDevSet FAILED, rc = [%d]", rc);

            rc1 = rc;
            continue;
        }

        /*get orig value vlans in flush */
        rc = prvTgfBrgFdbActionActiveVlanGet(devNum,&origVlanId,&origVlanMask);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfBrgFdbActionActiveVlanGet FAILED, rc = [%d]", rc);

            rc1 = rc;
            continue;
        }

        /*allow all vlans in flush */
        rc = prvTgfBrgFdbActionActiveVlanSet(0,0);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfBrgFdbActionActiveVlanSet FAILED, rc = [%d]", rc);

            rc1 = rc;
            continue;
        }

        if(GT_TRUE == PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum))
        {
            rc = cpssDxChBrgFdbActionActiveUserDefinedGet(devNum,&origActUerDefined,&origAactUerDefinedMask);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbActionActiveUserDefinedGet FAILED, rc = [%d]", rc);

                rc1 = rc;
                continue;
            }

            rc = cpssDxChBrgFdbActionActiveUserDefinedSet(devNum,0,0);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbActionActiveUserDefinedSet FAILED, rc = [%d]", rc);

                rc1 = rc;
                continue;
            }
        }

        /* save Trunk Aging Mode*/
        rc = cpssDxChBrgFdbTrunkAgingModeGet(devNum, &origTrunkAgingMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbTrunkAgingModeGet FAILED, rc = [%d]", rc);

            rc1 = rc;
            continue;
        }

        /* Set Trunk Aging Mode*/
        rc = cpssDxChBrgFdbTrunkAgingModeSet(devNum, CPSS_DXCH_BRG_FDB_AGE_TRUNK_MODE_REGULAR_E);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbTrunkAgingModeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
            continue;
        }

        /* trigger address deleting */
        AU_FDB_UPDATE_LOCK();
        rc = prvTgfBrgFdbQueueFullGet(devNum,PRV_TGF_BRG_FDB_QUEUE_TYPE_AU_E,&isAuqFull);
        if(rc == GT_NOT_APPLICABLE_DEVICE)
        {
            isAuqFull = GT_FALSE;
            rc = GT_OK;
        }
        while(rc == GT_OK && isAuqFull == GT_TRUE)
        {
            AU_FDB_UPDATE_UNLOCK();

            PRV_UTF_LOG0_MAC("$");
            /* allow the AppDemo to process AUQ messages --> so AUQ will get empty */
            cpssOsTimerWkAfter(50);

            AU_FDB_UPDATE_LOCK();
            rc = prvTgfBrgFdbQueueFullGet(devNum,PRV_TGF_BRG_FDB_QUEUE_TYPE_AU_E,&isAuqFull);
        }

        if(rc == GT_OK)
        {
            rc = cpssDxChBrgFdbTrigActionStart(devNum, CPSS_FDB_ACTION_DELETING_E);
        }
        AU_FDB_UPDATE_UNLOCK();
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbTrigActionStart FAILED, rc = [%d] \n", rc);

            rc1 = rc;

            /* restore AA and TA messages to CPU */
            if (messagesToCpuEnabled)
            {
                rc = cpssDxChBrgFdbAAandTAToCpuSet(devNum, GT_TRUE);
                if (GT_OK != rc)
                {
                    PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbAAandTAToCpuSet FAILED, rc = [%d] \n", rc);
                }
            }
            /* restore AA and TA messages to CPU for UC Route Entries */
            if (routeUcMessagesToCpuEnabled)
            {
                rc = cpssDxChBrgFdbRoutingUcAAandTAToCpuSet(devNum, GT_TRUE);
                if (GT_OK != rc)
                {
                    PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbRoutingUcAAandTAToCpuSet FAILED, rc = [%d] \n", rc);
                }
            }

            if(routeUcDeleteEnabled)
            {
                rc = cpssDxChBrgFdbRoutingUcAAandTAToCpuSet(devNum, GT_FALSE);
                if (GT_OK != rc)
                {
                    PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbRoutingUcAAandTAToCpuSet FAILED, rc = [%d] \n", rc);
                }
            }

            if(ipmcAddrDelEnabled)
            {
                /* disable the IPMC delete entries */
                rc = cpssDxChBrgFdbIpmcAddrDelEnableSet(devNum , GT_FALSE);
                if (GT_OK != rc)
                {
                    PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbIpmcAddrDelEnableSet FAILED, rc = [%d] \n", rc);
                }
            }


            continue;
        }

        /* wait that triggered action is completed */
        while (GT_TRUE != isCompleted)
        {
#ifdef ASIC_SIMULATION
            if((--maxRetry) == 0)
            {
                PRV_UTF_LOG0_MAC("[TGF]: prvTgfBrgFdbFlush : Simulation failed to process the flush operation \n");
                rc1 = GT_TIMEOUT;
                break;
            }
            /* allow simulation to complete the operation*/
            cpssOsTimerWkAfter(50);
#endif /* ASIC_SIMULATION */

            /* get triggered action status */
            rc = cpssDxChBrgFdbTrigActionStatusGet(devNum, &isCompleted);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbTrigActionStatusGet FAILED, rc = [%d] \n", rc);

                rc1 = rc;

                break;
            }
        }

        /* restore AA and TA messages to CPU */
        if (messagesToCpuEnabled)
        {
            rc = cpssDxChBrgFdbAAandTAToCpuSet(devNum, GT_TRUE);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbAAandTAToCpuSet FAILED, rc = [%d] \n", rc);
            }
        }

        /* restore AA and TA messages to CPU for Route UC Entries */
        if (routeUcMessagesToCpuEnabled)
        {
            rc = cpssDxChBrgFdbRoutingUcAAandTAToCpuSet(devNum, GT_TRUE);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbRoutingUcAAandTAToCpuSet FAILED, rc = [%d] \n", rc);
            }
        }

        /* Free FDB Ipv6 UC Address and Data index mapping - Linked List Data Base */
        rc = appDemoBrgFdbIpv6UcFreeAddrDataLinkedList(devNum);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: appDemoBrgFdbIpv6UcFreeAddrDataLinkedList FAILED, rc = [%d] \n", rc);
        }

        if(deletionMode != oldDeletionMode)
        {
            /* restore FDB flush status */
            rc = cpssDxChBrgFdbStaticDelEnable(devNum, oldDeletionMode);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbStaticDelEnable FAILED, rc = [%d] \n", rc);

                rc1 = rc;

                continue;
            }
        }

        /* restore original Trunk Aging Mode*/
        rc = cpssDxChBrgFdbTrunkAgingModeSet(devNum, origTrunkAgingMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbTrunkAgingModeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
            continue;
        }

        /* restore original act device info */
        rc = cpssDxChBrgFdbActionActiveDevSet(devNum,origActDev,origActDevMask);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbActionActiveDevSet FAILED, rc = [%d] \n", rc);

            rc1 = rc;
            continue;
        }

        rc = prvTgfBrgFdbActionActiveVlanSet(origVlanId,origVlanMask);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfBrgFdbActionActiveVlanSet FAILED, rc = [%d]", rc);

            rc1 = rc;
            continue;
        }

        /* restore original act UserDefined info */
        if(GT_TRUE == PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum))
        {
            rc = cpssDxChBrgFdbActionActiveUserDefinedSet(devNum,origActUerDefined,origAactUerDefinedMask);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbActionActiveUserDefinedSet FAILED, rc = [%d]", rc);

                rc1 = rc;
                continue;
            }
        }

    }

    PRV_UTF_LOG1_MAC("prvTgfBrgFdbFlush: ended with rc = %d \n",rc1);
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* get FDB flush status */
        rc = cpssExMxPmBrgFdbStaticDelEnableGet(devNum, &staticEnabled);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgFdbStaticDelEnableGet FAILED, rc = [%d] \n", rc);

            rc1 = rc;

            continue;
        }

        if(includeStatic != staticEnabled)
        {
            /* set FDB flush status */
            rc = cpssExMxPmBrgFdbStaticDelEnableSet(devNum,includeStatic);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgFdbStaticDelEnableSet FAILED, rc = [%d] \n", rc);

                rc1 = rc;

                continue;
            }
        }

        /* disable AA and TA messages to CPU */
        rc = cpssExMxPmBrgFdbAaAndTaToCpuGet(devNum, &messagesToCpuEnabled);
        if ((rc == GT_OK) && messagesToCpuEnabled)
        {
            rc = cpssExMxPmBrgFdbAaAndTaToCpuSet(devNum, GT_FALSE);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgFdbAaAndTaToCpuSet FAILED, rc = [%d] \n", rc);
            }
        }
        else
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgFdbAaAndTaToCpuGet FAILED, rc = [%d] \n", rc);
            messagesToCpuEnabled = GT_FALSE;
        }

        rc = prvTgfBrgFdbQueueFullGet(devNum,PRV_TGF_BRG_FDB_QUEUE_TYPE_AU_E,&isAuqFull);
        while(rc == GT_OK && isAuqFull == GT_TRUE)
        {
            /* allow the AppDemo to process AUQ messages --> so AUQ will get empty */
            PRV_UTF_LOG0_MAC("$");
            cpssOsTimerWkAfter(50);
            rc = prvTgfBrgFdbQueueFullGet(devNum,PRV_TGF_BRG_FDB_QUEUE_TYPE_AU_E,&isAuqFull);
        }

        if(rc == GT_OK)
        {
            /* trigger address deleting */
            rc = cpssExMxPmBrgFdbTriggeredActionStart(devNum,CPSS_EXMXPM_FDB_ACTION_DELETING_E);
        }

        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgFdbTriggeredActionStart FAILED, rc = [%d] \n", rc);

            rc1 = rc;

            continue;
        }

        /* wait that triggered action is completed */
        while (GT_TRUE != isCompleted)
        {
#ifdef ASIC_SIMULATION
            if((--maxRetry) == 0)
            {
                PRV_UTF_LOG0_MAC("[TGF]: prvTgfBrgFdbFlush : Simulation failed to process the flush operation \n");
                rc1 = GT_TIMEOUT;
                break;
            }
            /* allow simulation to complete the operation*/
            cpssOsTimerWkAfter(50);
#endif /* ASIC_SIMULATION */

            /* get triggered action status */
            rc = cpssExMxPmBrgFdbAgingTriggerGet(devNum, &isCompleted);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgFdbAgingTriggerGet FAILED, rc = [%d] \n", rc);

                rc1 = rc;

                break;
            }
        }

        if (messagesToCpuEnabled != GT_FALSE)
        {
            /* restore AA and TA to CPU */
            rc = cpssExMxPmBrgFdbAaAndTaToCpuSet(devNum, GT_TRUE);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgFdbAaAndTaToCpuSet FAILED, rc = [%d] \n", rc);
            }
        }

        if(includeStatic != staticEnabled)
        {
            /* restore FDB flush status */
            rc = cpssExMxPmBrgFdbStaticDelEnableSet(devNum,staticEnabled);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgFdbStaticDelEnableSet FAILED, rc = [%d] \n", rc);

                rc1 = rc;

                continue;
            }
        }
    }

    PRV_UTF_LOG1_MAC("prvTgfBrgFdbFlush: ended with rc = %d \n",rc1);
    return rc1;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgFdbActionStart function
* @endinternal
*
* @brief   Enable actions, sets action type, action mode to CPSS_ACT_TRIG_E and
*         starts triggered action by setting Aging Trigger.
*         This API may be used to start one of triggered actions: Aging, Deleting,
*         Transplanting and FDB Upload.
*         NOTE: caller should call prvTgfBrgFdbActionDoneWait(...) to check that
*         operation done before checking for the action outcomes...
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
* @param[in] mode                     - action mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device or mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - when previous FDB triggered action is not completed yet
*                                       or CNC block upload not finished (or not all of it's
*                                       results retrieved from the common used FU and CNC
*                                       upload queue)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Deleting and Transplanting actions can be done only by CPU triggered
*       FDB action that activated by the function.
*       See below sequence of CPSS API calls for Deleting and
*       Transplanting actions.
*       Before making Delete or Transplant sequence the application need to
*       disable Actions to avoid wrong automatic aging.
*       It is Application responsibility to get and store all parameters
*       that are changed during those actions.
*       1. AA message to CPU status by cpssDxChBrgFdbAAandTAToCpuGet.
*       2. Action Active configuration by
*       cpssDxChBrgFdbActionActiveInterfaceGet,
*       cpssDxChBrgFdbActionActiveDevGet,
*       cpssDxChBrgFdbActionActiveVlanGet.
*       3. Action mode and trigger mode by cpssDxChBrgFdbActionModeGet
*       cpssDxChBrgFdbActionTriggerModeGet.
*       The AA and TA messages may be disabled before the FDB action
*       4. Disable AA and TA messages to CPU by cpssDxChBrgFdbAAandTAToCpuSet.
*       5. Set Active configuration by: cpssDxChBrgFdbActionActiveInterfaceSet,
*       cpssDxChBrgFdbActionActiveVlanSet and cpssDxChBrgFdbActionActiveDevSet.
*       6. Start triggered action by cpssDxChBrgFdbTrigActionStart
*       7. Wait that triggered action is completed by:
*       - Busy-wait poling of status - cpssDxChBrgFdbTrigActionStatusGet
*       - Wait of event CPSS_PP_MAC_AGE_VIA_TRIGGER_ENDED_E.
*       This wait may be done in context of dedicated task to restore
*       Active configuration and AA messages configuration.
*       for multi-port groups device :
*       the trigger is done on ALL port groups regardless to operation mode
*       (Trigger aging/transplant/delete/upload).
*       see also description of function cpssDxChBrgFdbActionModeSet about
*       'multi-port groups device':
*
*/
GT_STATUS prvTgfBrgFdbActionStart
(
    IN PRV_TGF_FDB_ACTION_MODE_ENT  mode
)
{
    GT_STATUS rc;
#ifdef CHX_FAMILY
    CPSS_FDB_ACTION_MODE_ENT dxChMode;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_FDB_ACTION_MODE_ENT exMxPmMode;
#endif /* EXMXPM_FAMILY */

#ifdef CHX_FAMILY
    switch(mode)
    {
        case PRV_TGF_FDB_ACTION_AGE_WITH_REMOVAL_E:
            dxChMode = CPSS_FDB_ACTION_AGE_WITH_REMOVAL_E;
            if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum) &&
               !PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->bridge.supportSingleFdbInstance)
            {
                /* force the use of 'without' removal ... or remove the test ! */
                dxChMode = CPSS_FDB_ACTION_AGE_WITHOUT_REMOVAL_E;
            }
            break;
        case PRV_TGF_FDB_ACTION_AGE_WITHOUT_REMOVAL_E:
            dxChMode = CPSS_FDB_ACTION_AGE_WITHOUT_REMOVAL_E;
            break;
        case PRV_TGF_FDB_ACTION_DELETING_E:
            dxChMode = CPSS_FDB_ACTION_DELETING_E;
            break;
        case PRV_TGF_FDB_ACTION_TRANSPLANTING_E:
            dxChMode = CPSS_FDB_ACTION_TRANSPLANTING_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    AU_FDB_UPDATE_LOCK();
    /* trigger address deleting */
    rc = cpssDxChBrgFdbTrigActionStart(prvTgfDevNum,dxChMode);
    AU_FDB_UPDATE_UNLOCK();
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbTrigActionStart FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    switch(mode)
    {
        case PRV_TGF_FDB_ACTION_AGE_WITH_REMOVAL_E:
            exMxPmMode = CPSS_EXMXPM_FDB_ACTION_AGE_WITH_REMOVAL_E;
            break;
        case PRV_TGF_FDB_ACTION_AGE_WITHOUT_REMOVAL_E:
            exMxPmMode = CPSS_EXMXPM_FDB_ACTION_AGE_WITHOUT_REMOVAL_E;
            break;
        case PRV_TGF_FDB_ACTION_DELETING_E:
            exMxPmMode = CPSS_EXMXPM_FDB_ACTION_DELETING_E;
            break;
        case PRV_TGF_FDB_ACTION_TRANSPLANTING_E:
            exMxPmMode = CPSS_EXMXPM_FDB_ACTION_TRANSPLANTING_E;
            break;
        case PRV_TGF_FDB_ACTION_FDB_UPLOAD_E:
            exMxPmMode = CPSS_EXMXPM_FDB_ACTION_FDB_UPLOAD_E;
            break;
        default:
            return GT_BAD_PARAM;
    }
    /* trigger address deleting */
    rc = cpssExMxPmBrgFdbTriggeredActionStart(prvTgfDevNum,exMxPmMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgFdbTriggeredActionStart FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgFdbActionDoneWaitForDev function
* @endinternal
*
* @brief   wait for the FDB triggered action to end - for specific device
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
* @param[in] devNum                   - device number
* @param[in] aaTaClosed               - indication that the caller closed the AA,TA before the
*                                      action -->
*                                      GT_TRUE - the caller closed AA,TA before calling
*                                      prvTgfBrgFdbActionStart(...)
*                                      so action should be ended quickly.
*                                      timeout is set to 50 milliseconds
*                                      GT_FALSE - the caller NOT closed AA,TA before calling
*                                      prvTgfBrgFdbActionStart(...)
*                                      so action may take unpredictable time.
*                                      timeout is set to 50 seconds !!!
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_TIMEOUT               - after timed out. see timeout definition
*                                       according to aaTaClosed parameter.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbActionDoneWaitForDev
(
    IN  GT_U8    devNum,
    IN  GT_BOOL  aaTaClosed
)
{
    GT_STATUS   rc;
    GT_BOOL     isCompleted   = GT_FALSE;
    GT_U32      timeOut = (aaTaClosed == GT_TRUE) ? 2 : 2000;

#ifdef ASIC_SIMULATION
    if(timeOut < 10)
    {
        /* allow simulation minimal extra time */
        timeOut = 10;
    }
#endif /* ASIC_SIMULATION */

    /* wait that triggered action is completed */
    while (GT_TRUE != isCompleted)
    {
        /* get triggered action status */
#ifdef CHX_FAMILY
        rc = cpssDxChBrgFdbTrigActionStatusGet(devNum, &isCompleted);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbTrigActionStatusGet FAILED, rc = [%d]", rc);
            return rc;
        }
#endif /* CHX_FAMILY */
#ifdef EXMXPM_FAMILY
        rc = cpssExMxPmBrgFdbAgingTriggerGet(devNum, &isCompleted);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgFdbAgingTriggerGet FAILED, rc = [%d]", rc);
            return rc;
        }
#endif /* EXMXPM_FAMILY */

        /* allow device to complete the operation*/
        cpssOsTimerWkAfter(25);
        if(0 == (--timeOut))
        {
            return GT_TIMEOUT;
        }
    }

    return GT_OK;
}
/**
* @internal prvTgfBrgFdbActionDoneWait function
* @endinternal
*
* @brief   wait for the FDB triggered action to end
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
* @param[in] aaTaClosed               - indication that the caller closed the AA,TA before the
*                                      action -->
*                                      GT_TRUE - the caller closed AA,TA before calling
*                                      prvTgfBrgFdbActionStart(...)
*                                      so action should be ended quickly.
*                                      timeout is set to 50 milliseconds
*                                      GT_FALSE - the caller NOT closed AA,TA before calling
*                                      prvTgfBrgFdbActionStart(...)
*                                      so action may take unpredictable time.
*                                      timeout is set to 50 seconds !!!
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_TIMEOUT               - after timed out. see timeout definition
*                                       according to aaTaClosed parameter.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbActionDoneWait
(
    IN  GT_BOOL  aaTaClosed
)
{
    return prvTgfBrgFdbActionDoneWaitForDev(prvTgfDevNum,aaTaClosed);
}

/**
* @internal prvTgfBrgFdbActionDoneWaitForAllDev function
* @endinternal
*
* @brief   wait for the FDB triggered action to end - for all device
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
* @param[in] aaTaClosed               - indication that the caller closed the AA,TA before the
*                                      action -->
*                                      GT_TRUE - the caller closed AA,TA before calling
*                                      prvTgfBrgFdbActionStart(...)
*                                      so action should be ended quickly.
*                                      timeout is set to 50 milliseconds
*                                      GT_FALSE - the caller NOT closed AA,TA before calling
*                                      prvTgfBrgFdbActionStart(...)
*                                      so action may take unpredictable time.
*                                      timeout is set to 50 seconds !!!
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_TIMEOUT               - after timed out. see timeout definition
*                                       according to aaTaClosed parameter.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbActionDoneWaitForAllDev
(
    IN  GT_BOOL  aaTaClosed
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        rc = prvTgfBrgFdbActionDoneWaitForDev(devNum,aaTaClosed);
        if (GT_OK != rc)
        {
            rc1 = rc;
        }
    }

    return rc1;
}

/**
* @internal prvTgfBrgFdbActionModeSet function
* @endinternal
*
* @brief   Sets FDB action mode without setting Action Trigger
*
* @param[in] mode                     - FDB action mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbActionModeSet
(
    IN PRV_TGF_FDB_ACTION_MODE_ENT  mode
)
{
    GT_U8       devNum;
    GT_STATUS   rc, rc1 = GT_OK;
#ifdef CHX_FAMILY
    CPSS_FDB_ACTION_MODE_ENT dxChMode;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_FDB_ACTION_MODE_ENT exMxPmMode;
#endif /* EXMXPM_FAMILY */

#ifdef CHX_FAMILY
    /* convert into device specific format */
    switch(mode)
    {
        case PRV_TGF_FDB_ACTION_AGE_WITH_REMOVAL_E:
            dxChMode = CPSS_FDB_ACTION_AGE_WITH_REMOVAL_E;
            if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum) &&
               !PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->bridge.supportSingleFdbInstance)
            {
                /* force the use of 'without' removal ... or remove the test ! */
                dxChMode = CPSS_FDB_ACTION_AGE_WITHOUT_REMOVAL_E;
            }
            break;
        case PRV_TGF_FDB_ACTION_AGE_WITHOUT_REMOVAL_E:
            dxChMode = CPSS_FDB_ACTION_AGE_WITHOUT_REMOVAL_E;
            break;
        case PRV_TGF_FDB_ACTION_DELETING_E:
            dxChMode = CPSS_FDB_ACTION_DELETING_E;
            break;
        case PRV_TGF_FDB_ACTION_TRANSPLANTING_E:
            dxChMode = CPSS_FDB_ACTION_TRANSPLANTING_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgFdbActionModeSet(devNum, dxChMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbActionModeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* convert into device specific format */
    switch(mode)
    {
        case PRV_TGF_FDB_ACTION_AGE_WITH_REMOVAL_E:
            exMxPmMode = CPSS_EXMXPM_FDB_ACTION_AGE_WITH_REMOVAL_E;
            break;
        case PRV_TGF_FDB_ACTION_AGE_WITHOUT_REMOVAL_E:
            exMxPmMode = CPSS_EXMXPM_FDB_ACTION_AGE_WITHOUT_REMOVAL_E;
            break;
        case PRV_TGF_FDB_ACTION_DELETING_E:
            exMxPmMode = CPSS_EXMXPM_FDB_ACTION_DELETING_E;
            break;
        case PRV_TGF_FDB_ACTION_TRANSPLANTING_E:
            exMxPmMode = CPSS_EXMXPM_FDB_ACTION_TRANSPLANTING_E;
            break;
        case PRV_TGF_FDB_ACTION_FDB_UPLOAD_E:
            exMxPmMode = CPSS_EXMXPM_FDB_ACTION_FDB_UPLOAD_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmBrgFdbActionModeSet(devNum, exMxPmMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgFdbActionModeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

    return rc1;
}

/**
* @internal prvTgfBrgFdbActionModeGet function
* @endinternal
*
* @brief   Gets FDB action mode.
*
* @param[out] modePtr                  - FDB action mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbActionModeGet
(
    OUT PRV_TGF_FDB_ACTION_MODE_ENT     *modePtr
)
{
    GT_U8       devNum  = prvTgfDevNum;
    GT_STATUS   rc = GT_OK;
#ifdef CHX_FAMILY
    CPSS_FDB_ACTION_MODE_ENT dxChMode;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_FDB_ACTION_MODE_ENT exMxPmMode;
#endif /* EXMXPM_FAMILY */

#ifdef CHX_FAMILY

        /* call device specific API */
        rc = cpssDxChBrgFdbActionModeGet(devNum, &dxChMode);
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbActionModeSet FAILED, rc = [%d]", rc);

        /* convert into device specific format */
        switch(dxChMode)
        {
            case CPSS_FDB_ACTION_AGE_WITH_REMOVAL_E:
                *modePtr =  PRV_TGF_FDB_ACTION_AGE_WITH_REMOVAL_E;
                break;
            case  CPSS_FDB_ACTION_AGE_WITHOUT_REMOVAL_E:
                *modePtr = PRV_TGF_FDB_ACTION_AGE_WITHOUT_REMOVAL_E;
                break;
            case CPSS_FDB_ACTION_DELETING_E:
                *modePtr =  PRV_TGF_FDB_ACTION_DELETING_E;
                break;
            case CPSS_FDB_ACTION_TRANSPLANTING_E:
                *modePtr = PRV_TGF_FDB_ACTION_TRANSPLANTING_E;
                break;
            default:
                return GT_BAD_PARAM;
        }


#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

        /* call device specific API */
        rc = cpssExMxPmBrgFdbActionModeGet(devNum, &exMxPmMode);
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgFdbActionModeGet FAILED, rc = [%d]", rc);

        /* convert into device specific format */
    switch(exMxPmMode)
    {
        case CPSS_EXMXPM_FDB_ACTION_AGE_WITH_REMOVAL_E:
            *modePtr = PRV_TGF_FDB_ACTION_AGE_WITH_REMOVAL_E;
            break;
        case CPSS_EXMXPM_FDB_ACTION_AGE_WITHOUT_REMOVAL_E:
            *modePtr = PRV_TGF_FDB_ACTION_AGE_WITHOUT_REMOVAL_E;
            break;
        case CPSS_EXMXPM_FDB_ACTION_DELETING_E:
            *modePtr = PRV_TGF_FDB_ACTION_DELETING_E;
            break;
        case CPSS_EXMXPM_FDB_ACTION_TRANSPLANTING_E:
            *modePtr = PRV_TGF_FDB_ACTION_TRANSPLANTING_E;
            break;
        case CPSS_EXMXPM_FDB_ACTION_FDB_UPLOAD_E:
            *modePtr = PRV_TGF_FDB_ACTION_FDB_UPLOAD_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

    return rc;

}

/**
* @internal prvTgfBrgFdbAgingTimeoutSet function
* @endinternal
*
* @brief   Sets the timeout period in seconds for aging out dynamically learned
*         forwarding information. The standard recommends 300 sec.
* @param[in] timeout                  - aging time in seconds.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - aging timeout exceeds hardware limitation.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbAgingTimeoutSet
(
    IN GT_U32 timeout
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

#ifdef CHX_FAMILY

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgFdbAgingTimeoutSet(devNum, timeout);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbAgingTimeoutSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmBrgFdbAgingTimeoutSet(devNum, timeout);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgFdbAgingTimeoutSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

    return rc1;
}

/**
* @internal prvTgfBrgFdbAgingTimeoutGet function
* @endinternal
*
* @brief   Gets the timeout period in seconds for aging out dynamically learned
*         forwarding information. The standard recommends 300 sec.
*
* @param[out] timeoutPtr               - (pointer to)aging time in seconds.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - aging timeout exceeds hardware limitation.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbAgingTimeoutGet
(
    OUT GT_U32 *timeoutPtr
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

#ifdef CHX_FAMILY

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgFdbAgingTimeoutGet(devNum, timeoutPtr);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbAgingTimeoutSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }

        break;
    }
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmBrgFdbAgingTimeoutGet(devNum, timeoutPtr);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgFdbAgingTimeoutSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
        break;
    }
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

    return rc1;
}

/**
* @internal prvTgfBrgFdbAgeBitDaRefreshEnableSet function
* @endinternal
*
* @brief   Enables/disables destination address-based aging. When this bit is
*         set, the aging process is done both on the source and the destination
*         address (i.e. the age bit will be refresh when MAC DA hit occurs, as
*         well as MAC SA hit).
* @param[in] enable                   - GT_TRUE -  refreshing
*                                      GT_FALSE - disable refreshing
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                       COMMENTS
*/
GT_STATUS prvTgfBrgFdbAgeBitDaRefreshEnableSet
(
    IN GT_BOOL enable
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgFdbAgeBitDaRefreshEnableSet(devNum, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbAgeBitDaRefreshEnableSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#else /* !CHX_FAMILY */
    TGF_PARAM_NOT_USED(enable);

    return GT_NOT_SUPPORTED;
#endif /* CHX_FAMILY */
}

/**
* @internal prvTgfBrgFdbMacTriggerModeSet function
* @endinternal
*
* @brief   Sets Mac address table Triggered\Automatic action mode.
*
* @param[in] mode                     - action mode:
*                                      PRV_TGF_ACT_AUTO_E - Action is done Automatically.
*                                      PRV_TGF_ACT_TRIG_E - Action is done via a trigger from CPU.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbMacTriggerModeSet
(
    IN PRV_TGF_MAC_ACTION_MODE_ENT  mode
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

#ifdef CHX_FAMILY
    CPSS_MAC_ACTION_MODE_ENT dxChMode;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_FDB_TRIGGER_MODE_ENT exMxPmMode;
#endif /* EXMXPM_FAMILY */

#ifdef CHX_FAMILY
    /* convert into device specific format */
    switch(mode)
    {
        case PRV_TGF_ACT_AUTO_E:
            dxChMode = CPSS_ACT_AUTO_E;
            break;
        case PRV_TGF_ACT_TRIG_E:
            dxChMode = CPSS_ACT_TRIG_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        AU_FDB_UPDATE_LOCK();
        rc = cpssDxChBrgFdbMacTriggerModeSet(devNum, dxChMode);
        AU_FDB_UPDATE_UNLOCK();
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbMacTriggerModeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* convert into device specific format */
    switch(mode)
    {
        case PRV_TGF_ACT_AUTO_E:
            exMxPmMode = CPSS_EXMXPM_FDB_TRIGGER_AUTO_E;
            break;
        case PRV_TGF_ACT_TRIG_E:
            exMxPmMode = CPSS_EXMXPM_FDB_TRIGGER_TRIG_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmBrgFdbTriggerModeSet(devNum, exMxPmMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgFdbTriggerModeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

    return rc1;
}

/**
* @internal prvTgfBrgFdbMacTriggerModeGet function
* @endinternal
*
* @brief   Get Mac address table Triggered\Automatic action mode.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - pointer to action mode:
*                                      PRV_TGF_ACT_AUTO_E - Action is done Automatically.
*                                      PRV_TGF_ACT_TRIG_E - Action is done via a trigger from CPU.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbMacTriggerModeGet
(
    IN GT_U8                        devNum,
    IN PRV_TGF_MAC_ACTION_MODE_ENT  *modePtr
)
{
    GT_STATUS   rc = GT_OK;

#ifdef CHX_FAMILY
    CPSS_MAC_ACTION_MODE_ENT dxChMode;

    /* call device specific API */
    rc = cpssDxChBrgFdbMacTriggerModeGet(devNum, &dxChMode);

    /* convert from device specific format */
    switch(dxChMode)
    {
        case CPSS_ACT_AUTO_E:
            *modePtr = PRV_TGF_ACT_AUTO_E;
            break;
        case CPSS_ACT_TRIG_E:
            *modePtr = PRV_TGF_ACT_TRIG_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_FDB_TRIGGER_MODE_ENT exMxPmMode;

    /* call device specific API */
    rc =  cpssExMxPmBrgFdbTriggerModeGet(devNum, &exMxPmMode);

    /* convert from device specific format */
    switch(exMxPmMode)
    {
        case CPSS_EXMXPM_FDB_TRIGGER_AUTO_E:
            *modePtr = PRV_TGF_ACT_AUTO_E;
            break;
        case CPSS_EXMXPM_FDB_TRIGGER_TRIG_E:
            *modePtr = PRV_TGF_ACT_TRIG_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgFdbActionsEnableSet function
* @endinternal
*
* @brief   Enables/Disables FDB actions.
*
* @param[in] enable                   - GT_TRUE -  Actions are enabled
*                                      GT_FALSE -  Actions are disabled
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbActionsEnableSet
(
    IN GT_BOOL  enable
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

#ifdef CHX_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        AU_FDB_UPDATE_LOCK();
        rc = cpssDxChBrgFdbActionsEnableSet(devNum, enable);
        AU_FDB_UPDATE_UNLOCK();
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbActionsEnableSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmBrgFdbActionsEnableSet(devNum, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgFdbActionsEnableSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

    return rc1;
}
/**
* @internal prvTgfBrgFdbActionsEnableGet function
* @endinternal
*
* @brief   Get the status of FDB actions: flushing, deleting, uploading and
*         transplanting.
*
* @note   APPLICABLE DEVICES:      Puma2; Puma3.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @param[out] enablePtr                - GT_TRUE - actions are enabled
*                                      GT_FALSE - actions are disabled
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvTgfBrgFdbActionsEnableGet
(
    OUT GT_BOOL  *enablePtr
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

#ifdef CHX_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        AU_FDB_UPDATE_LOCK();
        rc = cpssDxChBrgFdbActionsEnableGet(devNum, enablePtr);
        AU_FDB_UPDATE_UNLOCK();
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbActionsEnableSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmBrgFdbActionsEnableGet(devNum, enablePtr);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgFdbActionsEnableSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

    return rc1;

}

/**
* @internal prvTgfBrgFdbMacEntryStatusGet function
* @endinternal
*
* @brief   Get the Valid and Skip Values of a FDB entry.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
* @param[in] index                    - hw mac entry index
*
* @param[out] validPtr                 - (pointer to) is entry valid
* @param[out] skipPtr                  - (pointer to) is entry skip control
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - index is out of range
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbMacEntryStatusGet
(
    IN  GT_U32                  index,
    OUT GT_BOOL                 *validPtr,
    OUT GT_BOOL                 *skipPtr
)
{
    GT_STATUS   rc;
#ifdef CHX_FAMILY
    /* call device specific API */
    AU_FDB_UPDATE_LOCK();
    rc = utfCpssDxChBrgFdbMacEntryStatusGet(prvTgfDevNum,index,validPtr,skipPtr);
    AU_FDB_UPDATE_UNLOCK();
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbMacEntryStatusGet FAILED, rc = [%d]", rc);
        return rc;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    rc = cpssExMxPmBrgFdbMacEntryStatusGet(prvTgfDevNum,index,validPtr,skipPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgFdbMacEntryStatusGet FAILED, rc = [%d]", rc);
        return rc;
    }

    return GT_OK;
#endif /*EXMXPM_FAMILY*/

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgFdbMacEntryAgeBitSet function
* @endinternal
*
* @brief   Set age bit in specific FDB entry.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
* @param[in] index                    - hw mac entry index
* @param[in] age                      - Age flag that is used for the two-step Aging process.
*                                      GT_FALSE - The entry will be aged out in the next pass.
*                                      GT_TRUE - The entry will be aged-out in two age-passes.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_OUT_OF_RANGE          - index out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbMacEntryAgeBitSet
(
    IN GT_U32       index,
    IN GT_BOOL      age
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        AU_FDB_UPDATE_LOCK();
        rc = utfCpssDxChBrgFdbMacEntryAgeBitSet(devNum, index, age);
        AU_FDB_UPDATE_UNLOCK();
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbMacEntryAgeBitSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#else /* !CHX_FAMILY */

    TGF_PARAM_NOT_USED(index);
    TGF_PARAM_NOT_USED(age);

    return GT_NOT_SUPPORTED;
#endif /* !CHX_FAMILY */
}

/**
* @internal prvTgfBrgFdbPortGroupDump function
* @endinternal
*
* @brief   This function dump valid entries in specific port group
*
* @param[in] devNum                   - physical device number.
* @param[in] portGroupId              - port group Id
*                                      when CPSS_PORT_GROUP_UNAWARE_MODE_CNS meaning read from first port group
*                                       None.
*/
GT_STATUS prvTgfBrgFdbPortGroupDump
(
    IN  GT_U8   devNum,
    IN  GT_U32  portGroupId
)
{
    GT_STATUS  rc = GT_FAIL;
    GT_U32     hwData[8];
    GT_U32     entryIndex;
    GT_U32     valid;
    GT_U32     skip;
    GT_U32     fdbSize;
#ifdef CHX_FAMILY
    GT_U32     hwValue;
#endif /*CHX_FAMILY*/

#ifdef CHX_FAMILY
    rc = cpssDxChCfgTableNumEntriesGet(devNum,CPSS_DXCH_CFG_TABLE_FDB_E,&fdbSize);
#endif /*CHX_FAMILY*/
#ifdef EXMXPM_FAMILY
    TGF_PARAM_NOT_USED(portGroupId);
    rc = cpssExMxPmCfgTableNumEntriesGet(devNum,CPSS_EXMXPM_CFG_TABLE_FDB_E,&fdbSize);
#endif/*EXMXPM_FAMILY*/

    if(rc != GT_OK)
    {
        return rc;
    }

    FDB_SHADOW_NOT_SUPPORT_FDB_HW_ITERATIONS;

    PRV_UTF_LOG2_MAC("\n FDB Dump PG[%d] size[%d]\n", portGroupId, fdbSize);

    for( entryIndex = 0 ; entryIndex < fdbSize; entryIndex++)
    {
#ifdef CHX_FAMILY
        rc = prvCpssDxChPortGroupReadTableEntry(devNum,portGroupId,
                                       CPSS_DXCH_TABLE_FDB_E,
                                       entryIndex,
                                       &hwData[0]);
#endif /*CHX_FAMILY*/
#ifdef EXMXPM_FAMILY
        rc = prvCpssExMxPmReadTableEntry(devNum,
            PRV_CPSS_EXMXPM_PP_MAC(devNum)->tblsAddr.genericPointers.fdbTablePtr,
            entryIndex,
            &hwData[0]);
#endif/*EXMXPM_FAMILY*/
        if(rc != GT_OK)
        {
            continue;
        }

#ifdef CHX_FAMILY
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            /* valid bit */
            SIP5_FDB_ENTRY_FIELD_GET_MAC(devNum,hwData,
                SIP5_FDB_FDB_TABLE_FIELDS_VALID_E,
                hwValue)
            valid = hwValue;

            /* skip bit */
            SIP5_FDB_ENTRY_FIELD_GET_MAC(devNum,hwData,
                SIP5_FDB_FDB_TABLE_FIELDS_SKIP_E,
                hwValue)
            skip = hwValue;
        }
        else
#endif /*CHX_FAMILY*/
        {
            valid  = U32_GET_FIELD_MAC(hwData[0],0,1);
            skip   = U32_GET_FIELD_MAC(hwData[0],1,1);
        }

        if (valid == 0)
        {
            continue;
        }

        if(skip)
        {
            continue;
        }

        /* dump entry */
        PRV_UTF_LOG5_MAC("  %5d 0x%08X 0x%08X 0x%08X 0x%08X\n", entryIndex, hwData[0], hwData[1], hwData[2], hwData[3]);

    }

    return GT_OK;
}

/**
* @internal prvTgfBrgFdbDump function
* @endinternal
*
* @brief   This function dumps valid FDB entries.
*         in multi ports port groups device the function summarize the entries
*         from all the port groups (don't care about duplications)
*/
GT_STATUS prvTgfBrgFdbDump
(
    void
)
{
    GT_STATUS rc;
    GT_U32     portGroupId;

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(prvTgfDevNum,portGroupId)
    {
        rc  = prvTgfBrgFdbPortGroupDump(prvTgfDevNum,portGroupId);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(prvTgfDevNum,portGroupId)

    return GT_OK;
}




static GT_U32  numOfValid;
static GT_U32  numOfSkip;
static GT_U32  numOfAged;
static GT_U32  numOfStormPrevention;
static GT_BOOL numOfErrors;

/**
* @internal prvTgfBrgFdbPortGroupCount function
* @endinternal
*
* @brief   This function count number of valid ,skip entries - for specific port group
*
* @param[in] devNum                   - physical device number.
* @param[in] portGroupId              - port group Id
*                                      when CPSS_PORT_GROUP_UNAWARE_MODE_CNS meaning read from first port group
*
* @param[out] numOfValidPtr            - (pointer to) number of entries with valid bit set
*                                      (number of valid entries)
* @param[out] numOfSkipPtr             - (pointer to) number of entries with skip bit set
*                                      (number of skipped entries)
*                                      entry not counted when valid = 0
* @param[out] numOfAgedPtr             - (pointer to) number of entries with age bit = 0 !!!
*                                      (number of aged out entries)
*                                      entry not counted when valid = 0 or skip = 1
* @param[out] numOfStormPreventionPtr  - (pointer to) number of entries with SP bit set
*                                      (number of SP entries)
*                                      entry not counted when valid = 0 or skip = 1
* @param[out] numOfErrorsPtr           - (pointer to) number of entries with read error
*                                       None.
*/
GT_STATUS prvTgfBrgFdbPortGroupCount
(
    IN  GT_U8   devNum,
    IN  GT_U32  portGroupId,

    OUT GT_U32  *numOfValidPtr,
    OUT GT_U32  *numOfSkipPtr,
    OUT GT_U32  *numOfAgedPtr,
    OUT GT_U32  *numOfStormPreventionPtr,
    OUT GT_U32  *numOfErrorsPtr
)
{
    GT_STATUS  rc = GT_FAIL;
    GT_U32     hwData[8];
    GT_U32     entryIndex;
    GT_U32     valid;
    GT_U32     skip;
    GT_U32     age;
    GT_U32     spUnknown;
    GT_U32     fdbSize;
#ifdef CHX_FAMILY
    GT_U32     hwValue;
#endif /*CHX_FAMILY*/

    numOfSkip = 0;
    numOfValid = 0;
    numOfAged = 0;
    numOfStormPrevention = 0;
    numOfErrors = 0;

#ifdef CHX_FAMILY
    rc = cpssDxChCfgTableNumEntriesGet(devNum,CPSS_DXCH_CFG_TABLE_FDB_E,&fdbSize);
#endif /*CHX_FAMILY*/
#ifdef EXMXPM_FAMILY
    TGF_PARAM_NOT_USED(portGroupId);
    rc = cpssExMxPmCfgTableNumEntriesGet(devNum,CPSS_EXMXPM_CFG_TABLE_FDB_E,&fdbSize);
#endif/*EXMXPM_FAMILY*/

    if(rc != GT_OK)
    {
        return rc;
    }

    FDB_SHADOW_NOT_SUPPORT_FDB_HW_ITERATIONS;
    for( entryIndex = 0 ; entryIndex < fdbSize; entryIndex++)
    {
#ifdef CHX_FAMILY
        rc = prvCpssDxChPortGroupReadTableEntry(devNum,portGroupId,
                                       CPSS_DXCH_TABLE_FDB_E,
                                       entryIndex,
                                       &hwData[0]);
#endif /*CHX_FAMILY*/
#ifdef EXMXPM_FAMILY
        rc = prvCpssExMxPmReadTableEntry(devNum,
            PRV_CPSS_EXMXPM_PP_MAC(devNum)->tblsAddr.genericPointers.fdbTablePtr,
            entryIndex,
            &hwData[0]);
#endif/*EXMXPM_FAMILY*/
        if(rc != GT_OK)
        {
            numOfErrors++;
            continue;
        }

#ifdef CHX_FAMILY
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            /* valid bit */
            SIP5_FDB_ENTRY_FIELD_GET_MAC(devNum,hwData,
                SIP5_FDB_FDB_TABLE_FIELDS_VALID_E,
                hwValue)
            valid = hwValue;

            /* skip bit */
            SIP5_FDB_ENTRY_FIELD_GET_MAC(devNum,hwData,
                SIP5_FDB_FDB_TABLE_FIELDS_SKIP_E,
                hwValue)
            skip = hwValue;

            /* age bit */
            SIP5_FDB_ENTRY_FIELD_GET_MAC(devNum,hwData,
                SIP5_FDB_FDB_TABLE_FIELDS_AGE_E,
                hwValue)
            age = hwValue;

            /* spUnknown bit */
            SIP5_FDB_ENTRY_FIELD_GET_MAC(devNum,hwData,
                SIP5_FDB_FDB_TABLE_FIELDS_SP_UNKNOWN_E,
                hwValue)
            spUnknown = hwValue;
        }
        else
#endif /*CHX_FAMILY*/
        {
            valid  = U32_GET_FIELD_MAC(hwData[0],0,1);
            skip   = U32_GET_FIELD_MAC(hwData[0],1,1);
            age    = U32_GET_FIELD_MAC(hwData[0],2,1);
            spUnknown = U32_GET_FIELD_MAC(hwData[3],2,1);
        }

        if (valid == 0)
        {
            continue;
        }

        numOfValid++;

        if(skip)
        {
            numOfSkip++;
            continue;
        }

        if (age == 0x0)
        {
            numOfAged++;
        }

        if(spUnknown)
        {
            numOfStormPrevention++;
        }
    }

    if(numOfValidPtr)
        *numOfValidPtr = numOfValid;

    if(numOfSkipPtr)
        *numOfSkipPtr = numOfSkip;

    if(numOfAgedPtr)
        *numOfAgedPtr = numOfAged;

    if(numOfStormPreventionPtr)
        *numOfStormPreventionPtr = numOfStormPrevention;

    if(numOfErrorsPtr)
        *numOfErrorsPtr = numOfErrors;

    return GT_OK;
}

/**
* @internal prvTgfBrgFdbCount function
* @endinternal
*
* @brief   This function count number of valid ,skip entries.
*         in multi ports port groups device the function summarize the entries
*         from all the port groups (don't care about duplications)
*
* @param[out] numOfValidPtr            - (pointer to) number of entries with valid bit set
*                                      (number of valid entries)
* @param[out] numOfSkipPtr             - (pointer to) number of entries with skip bit set
*                                      (number of skipped entries)
*                                      entry not counted when valid = 0
* @param[out] numOfAgedPtr             - (pointer to) number of entries with age bit = 0 !!!
*                                      (number of aged out entries)
*                                      entry not counted when valid = 0 or skip = 1
* @param[out] numOfStormPreventionPtr  - (pointer to) number of entries with SP bit set
*                                      (number of SP entries)
*                                      entry not counted when valid = 0 or skip = 1
* @param[out] numOfErrorsPtr           - (pointer to) number of entries with read error
*                                       None.
*/
GT_STATUS prvTgfBrgFdbCount
(
    OUT GT_U32  *numOfValidPtr,
    OUT GT_U32  *numOfSkipPtr,
    OUT GT_U32  *numOfAgedPtr,
    OUT GT_U32  *numOfStormPreventionPtr,
    OUT GT_U32  *numOfErrorsPtr
)
{
    GT_STATUS rc;
    GT_U32     portGroupId;
    GT_U32  tmpNumOfSkip = 0;
    GT_U32  tmpNumOfValid = 0;
    GT_U32  tmpNumOfAged = 0;
    GT_U32  tmpNumOfStormPrevention = 0;
    GT_BOOL tmpNumOfErrors = 0;
    GT_PORT_GROUPS_BMP portGroupsBmp;

    portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(prvTgfDevNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_FDB_E);

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(prvTgfDevNum,portGroupsBmp,portGroupId)
    {
        rc  = prvTgfBrgFdbPortGroupCount(prvTgfDevNum,portGroupId,
        NULL,NULL,NULL,NULL,NULL);
        if(rc != GT_OK)
        {
            return rc;
        }

        tmpNumOfSkip             += numOfSkip;
        tmpNumOfValid            += numOfValid;
        tmpNumOfAged             += numOfAged;
        tmpNumOfStormPrevention  += numOfStormPrevention;
        tmpNumOfErrors           += numOfErrors;
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(prvTgfDevNum,portGroupsBmp,portGroupId)

    if(tmpNumOfErrors)
        PRV_UTF_LOG1_MAC("prvTgfBrgFdbCount: errors [%d]\n", tmpNumOfErrors);

    if(numOfValidPtr)
        *numOfValidPtr = tmpNumOfValid;

    if(numOfSkipPtr)
        *numOfSkipPtr = tmpNumOfSkip;

    if(numOfAgedPtr)
        *numOfAgedPtr = tmpNumOfAged;

    if(numOfStormPreventionPtr)
        *numOfStormPreventionPtr = tmpNumOfStormPrevention;

    if(numOfErrorsPtr)
        *numOfErrorsPtr = tmpNumOfErrors;

    return GT_OK;
}


/**
* @internal brgFdbAuqMessageQrWait_internal function
* @endinternal
*
* @brief   the function wait for QR (query response)
*
* @param[in] devNum                   - device number
* @param[in] searchKeyPtr             - (pointer to) the search key (mac+vlan)
*                                      OUTPUTS:
* @param[in] indexPtr                 - (pointer to) index
*                                      when NULL --> ignored
* @param[in] agedPtr                  - (pointer to) is entry aged
*                                      when NULL --> ignored
* @param[in] hwDevNumPtr              - (pointer to) is HW device number associated with the entry
*                                      when NULL --> ignored
* @param[in] macEntryPtr              - (pointer to) extended Mac table entry
*                                      when NULL --> ignored
*
* @param[out] indexPtr                 - (pointer to) index
*                                      when NULL --> ignored
* @param[out] agedPtr                  - (pointer to) is entry aged
*                                      when NULL --> ignored
* @param[out] hwDevNumPtr              - (pointer to) is HW device number associated with the entry
*                                      when NULL --> ignored
* @param[out] macEntryPtr              - (pointer to) extended Mac table entry
*                                      when NULL --> ignored
*                                      OUTPUTS:
*                                      None
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - entry not found
* @retval GT_TIMEOUT               - on timeout waiting for the QR
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
*/
static GT_STATUS brgFdbAuqMessageQrWait_internal
(
    IN  GT_U8                         devNum,
    IN  PRV_TGF_MAC_ENTRY_KEY_STC    *searchKeyPtr,
    OUT GT_U32                       *indexPtr,
    OUT GT_BOOL                      *agedPtr,
    OUT GT_HW_DEV_NUM                *hwDevNumPtr,
    OUT PRV_TGF_BRG_MAC_ENTRY_STC    *macEntryPtr
)
{
    GT_STATUS                       rc;
    PRV_TGF_BRG_FDB_AUQ_MESSAGE_PART1_STC *currInfoPtr; /* pointer to current info */
    GT_U32                          maxRetry = 20; /* 1 seconds */
    GT_U32                          index; /* hash index */
    PRV_TGF_BRG_MAC_ENTRY_STC       macEntry;
    PRV_TGF_LIST_ITEM_PTR           itemPtr; /* Is used for moving thru the
                                                brgFdbAuqMessagesPart1 list. */

    while(maxRetry--)
    {
        itemPtr = brgFdbAuqMessagesPart1.firstPtr;
        while (itemPtr != NULL)
        {
            currInfoPtr = (PRV_TGF_BRG_FDB_AUQ_MESSAGE_PART1_STC*) itemPtr;
            itemPtr = itemPtr->nextPtr;

            /* check that this is QR message */
            if(currInfoPtr->message.updType != CPSS_QR_E ||
               devNum !=  currInfoPtr->devNum)
            {
                continue;
            }

            /* convert cpss to TGF format */
            rc = prvTgfConvertCpssToGenericMacEntry(&currInfoPtr->message.macEntry,&macEntry);
            if(rc != GT_OK)
            {
                return rc;
            }

            if(0 != prvTgfBrgFdbMacKeyCompare(&macEntry.key,searchKeyPtr))
            {
                /* response to other query */
                continue;
            }

            if(currInfoPtr->message.entryWasFound == GT_FALSE)
            {
                prvTgfListDelItem(&brgFdbAuqMessagesPart1, &currInfoPtr->item);
                return GT_NOT_FOUND;
            }


            /* we have found the QR */
            if(indexPtr)
            {
                /* get the hash index */
                rc = prvTgfBrgFdbMacEntryIndexFind(&macEntry.key,&index);
                if(rc != GT_OK && rc != GT_NOT_FOUND)
                {
                    prvTgfListDelItem(&brgFdbAuqMessagesPart1,
                                      &currInfoPtr->item);
                    return rc;
                }

                *indexPtr = index + currInfoPtr->message.macEntryIndex;
            }

            if(macEntryPtr)
            {
                *macEntryPtr = macEntry;
            }

            if(agedPtr)
            {
                *agedPtr = currInfoPtr->message.skip;
            }

            if(hwDevNumPtr)
            {
                *hwDevNumPtr = currInfoPtr->message.associatedHwDevNum;
            }
            prvTgfListDelItem(&brgFdbAuqMessagesPart1, &currInfoPtr->item);
            return GT_OK;
        }

        /* unlock the DB */
        AU_FDB_UPDATE_UNLOCK();

        /* wait for message */
        cpssOsTimerWkAfter(50);

        /* lock the DB */
        AU_FDB_UPDATE_LOCK();
    }

    return GT_TIMEOUT;
}

/**
* @internal brgFdbAuqMessageQrWait function
* @endinternal
*
* @brief   the function wait for QR (query response)
*
* @param[in] devNum                   - device number
* @param[in] searchKeyPtr             - (pointer to) the search key (mac+vlan)
*                                      OUTPUTS:
* @param[in] indexPtr                 - (pointer to) index
*                                      when NULL --> ignored
* @param[in] agedPtr                  - (pointer to) is entry aged
*                                      when NULL --> ignored
* @param[in] hwDevNumPtr              - (pointer to) is HW device number associated with the entry
*                                      when NULL --> ignored
* @param[in] macEntryPtr              - (pointer to) extended Mac table entry
*                                      when NULL --> ignored
*
* @param[out] indexPtr                 - (pointer to) index
*                                      when NULL --> ignored
* @param[out] agedPtr                  - (pointer to) is entry aged
*                                      when NULL --> ignored
* @param[out] hwDevNumPtr              - (pointer to) is HW device number associated with the entry
*                                      when NULL --> ignored
* @param[out] macEntryPtr              - (pointer to) extended Mac table entry
*                                      when NULL --> ignored
*                                      OUTPUTS:
*                                      None
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - entry not found
* @retval GT_TIMEOUT               - on timeout waiting for the QR
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
*/
static GT_STATUS brgFdbAuqMessageQrWait
(
    IN  GT_U8                         devNum,
    IN  PRV_TGF_MAC_ENTRY_KEY_STC    *searchKeyPtr,
    OUT GT_U32                       *indexPtr,
    OUT GT_BOOL                      *agedPtr,
    OUT GT_HW_DEV_NUM                *hwDevNumPtr,
    OUT PRV_TGF_BRG_MAC_ENTRY_STC    *macEntryPtr
)
{
    GT_STATUS   rc;
    /* lock the DB */
    AU_FDB_UPDATE_LOCK();

    rc = brgFdbAuqMessageQrWait_internal(devNum,searchKeyPtr,indexPtr,agedPtr,hwDevNumPtr,macEntryPtr);

    /* UNlock the DB */
    AU_FDB_UPDATE_UNLOCK();

    return rc;
}

/**
* @internal prvTgfBrgFdbAuqMessageAaCheck function
* @endinternal
*
* @brief   the function check AA (Aged out) messages.
*         the function allow to get AA only from the specific port groups bmp.
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bmp of allowed port groups
*                                      OUTPUTS:
* @param[in] totalMessagesPtr         - (pointer to) total number of AA messages that received
*                                      from the port groups listed in portGroupsBmp
*                                      NOTE: can by NULL pointer --> ignored
* @param[in] totalErrorMessagesPtr    - (pointer to) total number of not expected messages that received.
*                                      NOTE: can by NULL pointer --> ignored
*
* @param[out] totalMessagesPtr         - (pointer to) total number of AA messages that received
*                                      from the port groups listed in portGroupsBmp
*                                      NOTE: can by NULL pointer --> ignored
* @param[out] totalErrorMessagesPtr    - (pointer to) total number of not expected messages that received.
*                                      NOTE: can by NULL pointer --> ignored
*                                      OUTPUTS:
*                                      None
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvTgfBrgFdbAuqMessageAaCheck
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        portGroupsBmp,
    OUT GT_U32                        *totalMessagesPtr,
    OUT GT_U32                        *totalErrorMessagesPtr
)
{
    GT_STATUS   rc;
    PRV_TGF_BRG_FDB_AUQ_MESSAGE_PART1_STC *currInfoPtr;/* pointer to current info */
    GT_U32 errorCount = 0;
    PRV_TGF_BRG_MAC_ENTRY_STC macEntry;
    PRV_TGF_LIST_ITEM_PTR itemPtr; /* Is used for moving through the
                                      brgFdbAuqMessagesPart1 list. */

    if(totalMessagesPtr)
    {
        (*totalMessagesPtr) = 0;
    }

    /* lock the DB */
    AU_FDB_UPDATE_LOCK();

    itemPtr = brgFdbAuqMessagesPart1.firstPtr;
    while (NULL != itemPtr)
    {
        currInfoPtr = (PRV_TGF_BRG_FDB_AUQ_MESSAGE_PART1_STC*)itemPtr;
        itemPtr = itemPtr->nextPtr;

        /* check that this is AA message */
        if(currInfoPtr->message.updType != CPSS_AA_E)
        {
            continue;
        }

        do{ /*A fake loop. It is easy to leave it in any point with "break".
              It was added to ensure what an additional code (prvTgfListDelItem)
              will be executed after leaving this cycle. */

            /* check that the device is correct */
            if(devNum !=  currInfoPtr->devNum)
            {
                PRV_UTF_LOG1_MAC("======= prvTgfBrgFdbAuqMessageAaCheck : Error : not correct device [%d] =======\n" , currInfoPtr->devNum);

                errorCount++;
                break;
            }

            /* convert cpss to TGF format */
            rc = prvTgfConvertCpssToGenericMacEntry(&currInfoPtr->message.macEntry,&macEntry);
            if(rc != GT_OK)
            {
                PRV_UTF_LOG0_MAC("======= prvTgfBrgFdbAuqMessageAaCheck : Error : convert format =======\n");

                errorCount++;
                break;
            }

            /* check that the port group is correct */
            if(0 == ((BIT_0 << currInfoPtr->message.portGroupId) & portGroupsBmp))
            {
                PRV_UTF_LOG1_MAC("======= prvTgfBrgFdbAuqMessageAaCheck : Error : not correct port group [%d] =======\n" , currInfoPtr->message.portGroupId);

                errorCount++;
                break;
            }

            if(macEntry.dstInterface.type == CPSS_INTERFACE_PORT_E)
            {
                PRV_UTF_LOG4_MAC("======= prvTgfBrgFdbAuqMessageAaCheck : AA : on port group [%d] , [%s] [%d] ,device[%d]=======\n" ,
                                 currInfoPtr->message.portGroupId ,
                                 "INTERFACE_PORT",
                                 macEntry.dstInterface.devPort.portNum,
                                 macEntry.dstInterface.devPort.hwDevNum);
            }
            else
                if(macEntry.dstInterface.type == CPSS_INTERFACE_TRUNK_E)
                {
                    PRV_UTF_LOG3_MAC("======= prvTgfBrgFdbAuqMessageAaCheck : AA : on port group [%d] , [%s] [%d] =======\n" ,
                                     currInfoPtr->message.portGroupId ,
                                     "INTERFACE_TRUNK",
                                     macEntry.dstInterface.trunkId);
                }

            if(totalMessagesPtr)
            {
                (*totalMessagesPtr) ++;
            }
        }
        while (0);
        prvTgfListDelItem(&brgFdbAuqMessagesPart1, &currInfoPtr->item);
    }

    /* unlock the DB */
    AU_FDB_UPDATE_UNLOCK();

    if(totalErrorMessagesPtr)
    {
        (*totalErrorMessagesPtr) = errorCount;
    }

    return errorCount ? GT_FAIL : GT_OK;

}


/**
* @internal prvTgfBrgFdbMacCompare function
* @endinternal
*
* @brief   compare 2 mac entries.
*
* @param[in] mac1Ptr                  - (pointer to) mac 1
* @param[in] mac2Ptr                  - (pointer to) mac 2
*                                      OUTPUTS:
*                                      > 0  - if mac1Ptr is  bigger than mac2Ptr
*                                      == 0 - if mac1Ptr is equal to mac2Ptr
*                                      < 0  - if mac1Ptr is smaller than mac2Ptr
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - entry not found
* @retval GT_TIMEOUT               - on timeout waiting for the QR
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
*/
int prvTgfBrgFdbMacCompare
(
    IN  PRV_TGF_BRG_MAC_ENTRY_STC    *mac1Ptr,
    IN  PRV_TGF_BRG_MAC_ENTRY_STC    *mac2Ptr
)
{
    GT_STATUS   rc;

#ifdef CHX_FAMILY
    CPSS_MAC_ENTRY_EXT_STC  dxChMacEntry1,dxChMacEntry2;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_FDB_ENTRY_STC exMxPmMacEntry1,exMxPmMacEntry2;
#endif /* EXMXPM_FAMILY */

#ifdef CHX_FAMILY
    /* convert key data into device specific format */
    rc = prvTgfConvertGenericToDxChMacEntry(prvTgfDevNum, mac1Ptr, &dxChMacEntry1);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChMacEntry FAILED, rc = [%d]", rc);
        return rc;  /* value is not 0 (GT_OK) */
    }
    rc = prvTgfConvertGenericToDxChMacEntry(prvTgfDevNum, mac2Ptr, &dxChMacEntry2);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChMacEntry FAILED, rc = [%d]", rc);
        return rc;  /* value is not 0 (GT_OK) */
    }

    return cpssOsMemCmp(&dxChMacEntry1,&dxChMacEntry2,sizeof(dxChMacEntry2));
#endif /*CHX_FAMILY*/

#ifdef EXMXPM_FAMILY
    /* convert key data into device specific format */
    rc = prvTgfConvertGenericToExMxPmMacEntry(mac1Ptr, &exMxPmMacEntry1);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToExMxPmMacEntry FAILED, rc = [%d]", rc);

        return rc;  /* value is not 0 (GT_OK) */
    }

    /* convert key data into device specific format */
    rc = prvTgfConvertGenericToExMxPmMacEntry(mac2Ptr, &exMxPmMacEntry2);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToExMxPmMacEntryKey FAILED, rc = [%d]", rc);

        return rc;  /* value is not 0 (GT_OK) */
    }

    return cpssOsMemCmp(&exMxPmMacEntry1,&exMxPmMacEntry2,sizeof(exMxPmMacEntry2));

#endif /* EXMXPM_FAMILY */

}

/**
* @internal prvTgfBrgFdbMacKeyCompare function
* @endinternal
*
* @brief   compare 2 mac keys.
*
* @param[in] key1Ptr                  - (pointer to) key 1
* @param[in] key2Ptr                  - (pointer to) key 2
*                                      OUTPUTS:
*                                      > 0  - if key1Ptr is  bigger than key2Ptr
*                                      == 0 - if key1Ptr is equal to str2
*                                      < 0  - if key1Ptr is smaller than key2Ptr
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - entry not found
* @retval GT_TIMEOUT               - on timeout waiting for the QR
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
*/
int prvTgfBrgFdbMacKeyCompare
(
    IN  PRV_TGF_MAC_ENTRY_KEY_STC    *key1Ptr,
    IN  PRV_TGF_MAC_ENTRY_KEY_STC    *key2Ptr
)
{
    GT_STATUS   rc;

#ifdef CHX_FAMILY
    CPSS_MAC_ENTRY_EXT_KEY_STC  dxChMacEntryKey1,dxChMacEntryKey2;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_FDB_ENTRY_KEY_STC exMxPmMacEntryKey1,exMxPmMacEntryKey2;
#endif /* EXMXPM_FAMILY */

#ifdef CHX_FAMILY
    /* convert key data into device specific format */
    rc = prvTgfConvertGenericToDxChMacEntryKey(key1Ptr, &dxChMacEntryKey1);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChMacEntryKey FAILED, rc = [%d]", rc);
        return rc;  /* value is not 0 (GT_OK) */
    }
    rc = prvTgfConvertGenericToDxChMacEntryKey(key2Ptr, &dxChMacEntryKey2);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChMacEntryKey FAILED, rc = [%d]", rc);
        return rc;  /* value is not 0 (GT_OK) */
    }

    switch(key1Ptr->entryType)
    {
        case PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_FID_VID1_E:
        case PRV_TGF_FDB_ENTRY_TYPE_IPV4_MCAST_FID_VID1_E:
        case PRV_TGF_FDB_ENTRY_TYPE_IPV6_MCAST_FID_VID1_E:
            break;
        default:
            /* compare without the vid1 field */
            dxChMacEntryKey1.vid1 = dxChMacEntryKey2.vid1;
            break;
    }

    return cpssOsMemCmp(&dxChMacEntryKey1,&dxChMacEntryKey2,sizeof(dxChMacEntryKey2));
#endif /*CHX_FAMILY*/

#ifdef EXMXPM_FAMILY
    /* convert key data into device specific format */
    rc = prvTgfConvertGenericToExMxPmMacEntryKey(key1Ptr, &exMxPmMacEntryKey1);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToExMxPmMacEntryKey FAILED, rc = [%d]", rc);

        return rc;  /* value is not 0 (GT_OK) */
    }

    /* convert key data into device specific format */
    rc = prvTgfConvertGenericToExMxPmMacEntryKey(key2Ptr, &exMxPmMacEntryKey2);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToExMxPmMacEntryKey FAILED, rc = [%d]", rc);

        return rc;  /* value is not 0 (GT_OK) */
    }

    return cpssOsMemCmp(&exMxPmMacEntryKey1,&exMxPmMacEntryKey2,sizeof(exMxPmMacEntryKey2));

#endif /* EXMXPM_FAMILY */

}

/**
* @internal prvTgfBrgFdbMacEntryHashCalc function
* @endinternal
*
* @brief   calculate the hash index for the key
*         NOTE:
*         see also function prvTgfBrgFdbMacEntryIndexFind(...) that find actual
*         place according to actual HW capacity
* @param[in] macEntryKeyPtr           - (pointer to) mac entry key
*
* @param[out] indexPtr                 - (pointer to) index
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvTgfBrgFdbMacEntryHashCalc
(
    IN  PRV_TGF_MAC_ENTRY_KEY_STC    *macEntryKeyPtr,
    OUT GT_U32                       *indexPtr
)
{
    GT_STATUS   rc = GT_OK;
#ifdef CHX_FAMILY
    CPSS_MAC_ENTRY_EXT_KEY_STC  dxChMacEntryKey;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_FDB_ENTRY_KEY_STC exMxPmMacEntryKey;
#endif /* EXMXPM_FAMILY */

#ifdef CHX_FAMILY
    /* convert key data into device specific format */
    rc = prvTgfConvertGenericToDxChMacEntryKey(macEntryKeyPtr, &dxChMacEntryKey);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChMacEntryKey FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    rc = cpssDxChBrgFdbHashCalc(prvTgfDevNum, &dxChMacEntryKey, indexPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbHashCalc FAILED, rc = [%d]", rc);

        return rc;
    }
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* convert key data into device specific format */
    rc = prvTgfConvertGenericToExMxPmMacEntryKey(macEntryKeyPtr, &exMxPmMacEntryKey);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToExMxPmMacEntryKey FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    rc = cpssExMxPmBrgFdbInternalHashCalc(prvTgfDevNum, &exMxPmMacEntryKey, indexPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgFdbInternalHashCalc FAILED, rc = [%d]", rc);

        return rc;
    }
#endif /* EXMXPM_FAMILY */

    return rc;
}

/**
* @internal prvTgfBrgFdbMacEntryFind function
* @endinternal
*
* @brief   scan the FDB table and find entry with same mac+vlan
*         entry MUST be valid and non-skipped
* @param[in] searchKeyPtr             - (pointer to) the search key (mac+vlan)
*
* @param[out] indexPtr                 - (pointer to) index
*                                      when NULL --> ignored
*                                      agedPtr     - (pointer to) is entry aged
*                                      when NULL --> ignored
* @param[out] hwDevNumPtr              - (pointer to) is HW device number associated with the entry
*                                      when NULL --> ignored
* @param[out] macEntryPtr              - (pointer to) extended Mac table entry
*                                      when NULL --> ignored
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - entry not found
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
* @retval GT_OUT_OF_RANGE          - index out of range
*/
GT_STATUS prvTgfBrgFdbMacEntryFind
(
    IN  PRV_TGF_MAC_ENTRY_KEY_STC    *searchKeyPtr,
    OUT GT_U32                       *indexPtr,
    OUT GT_BOOL                      *agePtr,
    OUT GT_HW_DEV_NUM                *hwDevNumPtr,
    OUT PRV_TGF_BRG_MAC_ENTRY_STC    *macEntryPtr
)
{
    GT_STATUS  rc;
    GT_U32     entryIndex;
    GT_BOOL    valid;
    GT_BOOL    skip;
    GT_BOOL    age;
    PRV_TGF_BRG_MAC_ENTRY_STC   macEntry;
    GT_HW_DEV_NUM               associatedHwDevNum;
#ifdef CHX_FAMILY
    CPSS_MAC_ENTRY_EXT_KEY_STC dxCpssKey;
#endif /*CHX_FAMILY*/
#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_FDB_ENTRY_KEY_STC pmCpssKey;
#endif/*EXMXPM_FAMILY*/

    if(prvTgfBrgFdbAccessMode == PRV_TGF_BRG_FDB_ACCESS_MODE_BY_MESSAGE_E)
    {
        /* register with the AppDemo to receive QR messages */
        rc = prvTgfBrgFdbAuqCbRegister(CPSS_QR_E,GT_TRUE);
        if(rc != GT_OK)
        {
            return rc;
        }

#ifdef CHX_FAMILY
        rc = prvTgfConvertGenericToDxChMacEntryKey(searchKeyPtr,&dxCpssKey);
        if(rc != GT_OK)
        {
            return rc;
        }
        rc = utfCpssDxChBrgFdbQaSend(prvTgfDevNum,&dxCpssKey);
        if(rc != GT_OK)
        {
            return rc;
        }
#endif /*CHX_FAMILY*/
#ifdef EXMXPM_FAMILY
        rc = prvTgfConvertGenericToExMxPmMacEntryKey(searchKeyPtr,&pmCpssKey);
        if(rc != GT_OK)
        {
            return rc;
        }

        rc = cpssExMxPmBrgFdbQaSend(prvTgfDevNum,&pmCpssKey);
        if(rc != GT_OK)
        {
            return rc;
        }
#endif/*EXMXPM_FAMILY*/

        /* wait for the info from the AppDemo */
        rc = brgFdbAuqMessageQrWait(prvTgfDevNum,searchKeyPtr,indexPtr,agePtr,hwDevNumPtr,macEntryPtr);

        /* unbind the CB function */
        (void)prvTgfBrgFdbAuqCbRegister(CPSS_QR_E,GT_FALSE);

        return rc;
    }

    rc = prvTgfBrgFdbMacEntryIndexFind(searchKeyPtr,&entryIndex);
    if(rc != GT_OK)
    {
        return rc;
    }
    /***************/
    /* entry found */
    /***************/

    /* read MAC entry */
    rc = prvTgfBrgFdbMacEntryRead(entryIndex, &valid, &skip, &age, &associatedHwDevNum, &macEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfBrgFdbMacEntryRead FAILED, rc = [%d]", rc);

        return rc;
    }

    if(indexPtr)
    {
        *indexPtr = entryIndex;
    }

    if(agePtr)
    {
        *agePtr = age;
    }

    if(indexPtr)
    {
        *indexPtr = entryIndex;
    }

    if(hwDevNumPtr)
    {
        *hwDevNumPtr = associatedHwDevNum;
    }

    if(macEntryPtr)
    {
        *macEntryPtr = macEntry;
    }

    return GT_OK;

}


/**
* @internal prvTgfBrgFdbAccessModeSet function
* @endinternal
*
* @brief   Set access mode to the FDB : by message or by index
*
* @param[in] mode                     - access  : by message or by index
*                                       previous state
*/
PRV_TGF_BRG_FDB_ACCESS_MODE_ENT prvTgfBrgFdbAccessModeSet
(
    IN PRV_TGF_BRG_FDB_ACCESS_MODE_ENT  mode
)
{
    PRV_TGF_BRG_FDB_ACCESS_MODE_ENT oldMode = prvTgfBrgFdbAccessMode;
    switch(mode)
    {
        case  PRV_TGF_BRG_FDB_ACCESS_MODE_BY_MESSAGE_E:
        case  PRV_TGF_BRG_FDB_ACCESS_MODE_BY_INDEX_E:
            break;
        default:
            /*error*/
            return (PRV_TGF_BRG_FDB_ACCESS_MODE_ENT)(0x7FFFFFFF);
    }

    prvTgfBrgFdbAccessMode = mode;

    return oldMode;
}

/**
* @internal prvTgfBrgVlanLearningStateSet function
* @endinternal
*
* @brief   Sets state of VLAN based learning to specified VLAN on specified device
*
* @param[in] vlanId                   - vlan Id
* @param[in] status                   - GT_TRUE for enable  or GT_FALSE otherwise.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vid
*/
GT_STATUS prvTgfBrgVlanLearningStateSet
(
    IN GT_U16                         vlanId,
    IN GT_BOOL                        status
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgVlanLearningStateSet(devNum, vlanId, status);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgVlanLearningStateSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    TGF_PARAM_NOT_USED(vlanId);
    TGF_PARAM_NOT_USED(status);

    return GT_OK;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgVlanNaToCpuEnable function
* @endinternal
*
* @brief   Enable/Disable New Address (NA) Message Sending to CPU per VLAN .
*         To send NA to CPU both VLAN and port must be set to send NA to CPU.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanNaToCpuEnable
(
    IN GT_U16                         vlanId,
    IN GT_BOOL                        enable
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgVlanNaToCpuEnable(devNum, vlanId, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgVlanNaToCpuEnable FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgMcMemberAdd function
* @endinternal
*
* @brief   Add new port member to the Multicast Group entry
*
* @param[in] devNum                   - PP's device number
* @param[in] vidx                     - multicast group index
* @param[in] portNum                  - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_OUT_OF_RANGE          - if vidx is larger than the allowed value
*/
GT_STATUS prvTgfBrgMcMemberAdd
(
    IN GT_U8                          devNum,
    IN GT_U16                         vidx,
    IN GT_U32                          portNum
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgMcMemberAdd(devNum, vidx, portNum);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmBrgMcMemberAdd(devNum, vidx, portNum);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgMcEntryWrite function
* @endinternal
*
* @brief   Writes Multicast Group entry to the HW
*
* @param[in] devNum                   - PP's device number
* @param[in] vidx                     - multicast group index
* @param[in] portBitmapPtr            - pointer to the bitmap of ports
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or ports bitmap value.
* @retval GT_OUT_OF_RANGE          - if vidx is larger than the allowed value.
*/
GT_STATUS prvTgfBrgMcEntryWrite
(
    IN GT_U8                          devNum,
    IN GT_U16                         vidx,
    IN CPSS_PORTS_BMP_STC            *portBitmapPtr
)
{
    GT_U32  ii;
    PRV_TGF_MEMBER_FORCE_INFO_STC *currMemPtr;
    CPSS_PORTS_BMP_STC            tmpPortsMembers;

    if(portBitmapPtr)
    {
        tmpPortsMembers = *portBitmapPtr;
    }
    else
    {
        cpssOsMemSet(&tmpPortsMembers,0,sizeof(tmpPortsMembers));
    }

    /* look for this member in the DB */
    currMemPtr = &prvTgfDevPortForceArray[0];
    for(ii = 0 ; ii < prvTgfDevPortForceNum; ii++,currMemPtr++)
    {
        if(devNum != currMemPtr->member.devNum)
        {
            continue;
        }

        if(currMemPtr->forceToVidx == GT_FALSE)
        {
            /* member not need to be forced to any vlan */
            continue;
        }

        if(currMemPtr->member.portNum >= CPSS_MAX_PORTS_NUM_CNS)
        {
            return GT_BAD_PARAM;
        }

        /* set the member */
        CPSS_PORTS_BMP_PORT_SET_MAC(&tmpPortsMembers,currMemPtr->member.portNum);
    }


#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgMcEntryWrite(devNum, vidx, &tmpPortsMembers);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmBrgMcEntryWrite(devNum, vidx, GT_FALSE, &tmpPortsMembers);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgMcEntryRead function
* @endinternal
*
* @brief   Reads the Multicast Group entry from the HW
*
* @param[in] devNum                   - PP's device number.
* @param[in] vidx                     - multicast group index
*
* @param[out] portBitmapPtr            - pointer to the bitmap of ports
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_OUT_OF_RANGE          - if vidx is larger than the allowed value.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
GT_STATUS prvTgfBrgMcEntryRead
(
    IN  GT_U8                         devNum,
    IN  GT_U16                        vidx,
    OUT CPSS_PORTS_BMP_STC           *portBitmapPtr
)
{
#ifdef EXMXPM_FAMILY
    GT_BOOL     isCpuMember = GT_FALSE;
#endif /* EXMXPM_FAMILY */

#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgMcEntryRead(devNum, vidx, portBitmapPtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmBrgMcEntryRead(devNum, vidx, &isCpuMember, portBitmapPtr);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgVlanEgressFilteringEnable function
* @endinternal
*
* @brief   Enable/Disable VLAN Egress Filtering on specified device for Bridged
*         Known Unicast packets
* @param[in] enable                   - enable\disable VLAN egress filtering
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong dev
*/
GT_STATUS prvTgfBrgVlanEgressFilteringEnable
(
    IN GT_BOOL                        enable
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;


#ifdef CHX_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgVlanEgressFilteringEnable(devNum, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgVlanEgressFilteringEnable FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmBrgEgressFilteringVlanEnableSet(devNum, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgEgressFilteringVlanEnableSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgVlanEgressFilteringEnableGet function
* @endinternal
*
* @brief   This function gets the VLAN Egress Filtering current state (enable/disable)
*         on specified device for Bridged Known Unicast packets.
*         If enabled the VLAN egress filter verifies that the egress port is a
*         member of the VID assigned to the packet.
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - points to (enable/disable) bridged known unicast packets filtering
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong dev or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvTgfBrgVlanEgressFilteringEnableGet
(
    IN  GT_U8                         devNum,
    OUT GT_BOOL                      *enablePtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgVlanEgressFilteringEnableGet(devNum, enablePtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmBrgEgressFilteringVlanEnableGet(devNum, enablePtr);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgCntDropCntrModeSet function
* @endinternal
*
* @brief   Sets Drop Counter Mode
*
* @param[in] dropMode                 - Drop Counter mode
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum or dropMode
* @retval GT_NOT_SUPPORTED         - on non-supported dropMode
*/
GT_STATUS prvTgfBrgCntDropCntrModeSet
(
    IN  PRV_TGF_BRG_DROP_CNTR_MODE_ENT dropMode
)
{
    GT_U8       devNum = 0;
    GT_STATUS   rc, rc1 = GT_OK;
#ifdef CHX_FAMILY
    CPSS_DXCH_BRIDGE_DROP_CNTR_MODE_ENT dxChDropMode;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_BRG_DROP_CNTR_MODE_ENT exMxPmDropMode;
#endif /* EXMXPM_FAMILY */


#ifdef CHX_FAMILY
    /* convert drop count mode into device specific format */
    rc = prvTgfConvertGenericToDxChDropCntMode(dropMode, &dxChDropMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChDropCntMode FAILED, rc = [%d]", rc);

        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgCntDropCntrModeSet(devNum, dxChDropMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgCntDropCntrModeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* convert drop count mode into device specific format */
    rc = prvTgfConvertGenericToExMxPmDropCntMode(dropMode, &exMxPmDropMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToExMxPmDropCntMode FAILED, rc = [%d]", rc);

        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmBrgCntDropCntrModeSet(devNum, exMxPmDropMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgCntDropCntrModeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgCntDropCntrModeGet function
* @endinternal
*
* @brief   Gets the Drop Counter Mode
*
* @param[out] dropModePtr              - (pointer to) the Drop Counter mode
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_SUPPORTED         - on non-supported dropMode
*/
GT_STATUS prvTgfBrgCntDropCntrModeGet
(
    OUT  PRV_TGF_BRG_DROP_CNTR_MODE_ENT *dropModePtr
)
{
    GT_STATUS   rc = GT_OK;
#ifdef CHX_FAMILY
    CPSS_DXCH_BRIDGE_DROP_CNTR_MODE_ENT dxChDropMode;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_BRG_DROP_CNTR_MODE_ENT exMxPmDropMode;
#endif /* EXMXPM_FAMILY */


#ifdef CHX_FAMILY
    /* call device specific API */
    rc = cpssDxChBrgCntDropCntrModeGet(prvTgfDevNum, &dxChDropMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgCntDropCntrModeGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert drop count mode from device specific format */
    return prvTgfConvertDxChToGenericDropCntMode(dxChDropMode, dropModePtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    rc = cpssExMxPmBrgCntDropCntrModeGet(prvTgfDevNum, &exMxPmDropMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgCntDropCntrModeGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert drop count mode from device specific format */
    return prvTgfConvertExMxPmToGenericDropCntMode(exMxPmDropMode, dropModePtr);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgVlanPortIngFltEnable function
* @endinternal
*
* @brief   Enable/disable Ingress Filtering for specific port
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - enable\disable ingress filtering
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
*/
GT_STATUS prvTgfBrgVlanPortIngFltEnable
(
    IN GT_U8                          devNum,
    IN GT_PORT_NUM                    portNum,
    IN GT_BOOL                        enable
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgVlanPortIngFltEnable(devNum, portNum, enable);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmBrgVlanPortIngressFilterEnableSet(devNum, portNum, enable);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgVlanPortIngFltEnableGet function
* @endinternal
*
* @brief   Get status of Ingress Filtering for specific port
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - Pointer to Ingress Filtering status
*                                      - GT_TRUE, ingress filtering is enabled
*                                      - GT_FALSE, ingress filtering is disabled
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanPortIngFltEnableGet
(
    IN  GT_U8        devNum,
    IN  GT_PORT_NUM  portNum,
    OUT GT_BOOL      *enablePtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgVlanPortIngFltEnableGet(devNum, portNum, enablePtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmBrgVlanPortIngressFilterEnableGet(devNum, (GT_U8)portNum, enablePtr);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgVlanL2VpnIngressFilterEnableSet function
* @endinternal
*
* @brief   Enable/disable STP and VLAN Ingress Filtering for L2 VPN traffic.
*
* @note   APPLICABLE DEVICES:      Puma3.
* @note   NOT APPLICABLE DEVICES:  Puma2.
*
* @param[in] enable                   - GT_TRUE  -  STP and VLAN Ingress Filtering
*                                      for L2 VPN traffic.
*                                      GT_FALSE - disable STP and VLAN Ingress Filtering
*                                      for L2 VPN traffic.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanL2VpnIngressFilterEnableSet
(
    IN GT_BOOL enable
)
{

#ifdef EXMXPM_FAMILY
    GT_U8       devNum = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmBrgVlanL2VpnIngressFilterEnableSet(devNum, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgVlanL2VpnIngressFilterEnableSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
    return rc1;

#else /* !EXMXPM_FAMILY */

    TGF_PARAM_NOT_USED(enable);

    return GT_NOT_SUPPORTED;
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfBrgCntDropCntrSet function
* @endinternal
*
* @brief   Sets the Bridge Ingress Drop Counter of specified device
*
* @param[in] dropCnt                  - the number of packets that were dropped
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvTgfBrgCntDropCntrSet
(
    IN  GT_U32                        dropCnt
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

#ifdef CHX_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgCntDropCntrSet(devNum, dropCnt);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgCntDropCntrSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmBrgCntDropCntrSet(devNum, dropCnt);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgCntDropCntrSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgCntDropCntrGet function
* @endinternal
*
* @brief   Gets the Bridge Ingress Drop Counter of specified device
*
* @param[out] dropCntPtr               - (pointer to) the number of packets that were dropped
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfBrgCntDropCntrGet
(
    OUT GT_U32                       *dropCntPtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgCntDropCntrGet(prvTgfDevNum, dropCntPtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmBrgCntDropCntrGet(prvTgfDevNum, dropCntPtr);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgVlanLocalSwitchingEnableSet function
* @endinternal
*
* @brief   Function sets local switching of Multicast, known and unknown Unicast,
*         and Broadcast traffic per VLAN.
* @param[in] vlanId                   - VLAN id
* @param[in] trafficType              - local switching traffic type
* @param[in] enable                   - enable/disable of local switching
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong vlanId or trafficType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanLocalSwitchingEnableSet
(
    IN  GT_U16                                              vlanId,
    IN  PRV_TGF_BRG_VLAN_LOCAL_SWITCHING_TRAFFIC_TYPE_ENT   trafficType,
    IN  GT_BOOL                                             enable
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
    CPSS_DXCH_BRG_VLAN_LOCAL_SWITCHING_TRAFFIC_TYPE_ENT dxChTrafficType;


    /* convert trafficType into device specific format */
    switch (trafficType)
    {
        case PRV_TGF_BRG_VLAN_LOCAL_SWITCHING_TRAFFIC_TYPE_KNOWN_UC_E:
            dxChTrafficType = CPSS_DXCH_BRG_VLAN_LOCAL_SWITCHING_TRAFFIC_TYPE_KNOWN_UC_E;
            break;

        case PRV_TGF_BRG_VLAN_LOCAL_SWITCHING_TRAFFIC_TYPE_FLOODED_E:
            dxChTrafficType = CPSS_DXCH_BRG_VLAN_LOCAL_SWITCHING_TRAFFIC_TYPE_FLOODED_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgVlanLocalSwitchingEnableSet(devNum, vlanId, dxChTrafficType, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgVlanLocalSwitchingEnableSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(vlanId);
    TGF_PARAM_NOT_USED(trafficType);
    TGF_PARAM_NOT_USED(enable);

    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgGenUcLocalSwitchingEnable function
* @endinternal
*
* @brief   Enable/disable local switching back through the ingress interface
*         for for known Unicast packets
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_FALSE = Unicast packets whose bridging decision is to be
*                                      forwarded back to its Ingress port or trunk
*                                      are assigned with a soft drop command.
*                                      GT_TRUE  = Unicast packets whose bridging decision is to be
*                                      forwarded back to its Ingress port or trunk may be
*                                      forwarded back to their source.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on on bad device or port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgGenUcLocalSwitchingEnable
(
    IN GT_U8                          devNum,
    IN GT_U32                          portNum,
    IN GT_BOOL                        enable
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgGenUcLocalSwitchingEnable(devNum, portNum, enable);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmBrgGenPortUcLocalSwitchingEnableSet(devNum, portNum, enable);
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgPortEgressMcLocalEnable function
* @endinternal
*
* @brief   Enable/Disable sending Multicast packets back to its source
*         port on the local device.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - Boolean value:
*                                      GT_TRUE  - Multicast packets may be sent back to
*                                      their source port on the local device.
*                                      GT_FALSE - Multicast packets are not sent back to
*                                      their source port on the local device.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong dev
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgPortEgressMcLocalEnable
(
    IN GT_U8                          devNum,
    IN GT_U32                          portNum,
    IN GT_BOOL                        enable
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgPortEgressMcastLocalEnable(devNum, portNum, enable);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    return cpssExMxPmBrgGenPortMcLocalSwitchingEnableSet(devNum, portNum, enable);
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgPortEgressMcLocalEnableGet function
* @endinternal
*
* @brief   Get the current status of Multicast packets local switching.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - points to (enable/disable) sending Multicast packets
*                                      back to its source
*                                      GT_TRUE  - Multicast packets may be sent back to
*                                      their source port on the local device.
*                                      GT_FALSE - Multicast packets are not sent back to
*                                      their source port on the local device.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong dev
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgPortEgressMcLocalEnableGet
(
    IN  GT_U8               devNum,
    IN  GT_U32               portNum,
    OUT GT_BOOL             *enablePtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgPortEgressMcastLocalEnableGet(devNum, portNum, enablePtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmBrgGenPortMcLocalSwitchingEnableGet(devNum, portNum, enablePtr);
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgEgrFltVlanPortFilteringEnableSet function
* @endinternal
*
* @brief   Enable/Disable ePort VLAN Egress Filtering.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - egress port number
* @param[in] enable                   - GT_FALSE - disable ePort VLAN Egress Filtering.
*                                      GT_TRUE  - enable ePort VLAN Egress Filtering.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgEgrFltVlanPortFilteringEnableSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_BOOL          enable
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgEgrFltVlanPortFilteringEnableSet(devNum, portNum, enable);
#else
    return GT_NOT_APPLICABLE_DEVICE;
#endif /* EXMXPM_FAMILY */
}

/**
* @internal prvTgfBrgVlanRangeSet function
* @endinternal
*
* @brief   Function configures the valid VLAN Range
*
* @param[in] vidRange                 - VID range for VLAN filtering (0 - 4095)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_OUT_OF_RANGE          - illegal vidRange
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanRangeSet
(
    IN  GT_U16                        vidRange
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgVlanRangeSet(devNum, vidRange);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgVlanRangeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(vidRange);

    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgVlanRangeGet function
* @endinternal
*
* @brief   Function gets the valid VLAN Range.
*
* @param[out] vidRangePtr              - (pointer to) VID range for VLAN filtering
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - vidRangePtr is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanRangeGet
(
    OUT GT_U16                       *vidRangePtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgVlanRangeGet(prvTgfDevNum, vidRangePtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(vidRangePtr);

    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/******************************************************************************\
 *                       API for default settings                             *
\******************************************************************************/

/**
* @internal prvTgfBrgDefVlanEntryWriteWithTaggingCmd function
* @endinternal
*
* @brief   Set vlan entry, with the all ports of the test as
*         in the vlan with specified tagging command
* @param[in] vlanId                   - the vlan Id
* @param[in] taggingCmd               - GT_TRUE - tagging command for all members
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfBrgDefVlanEntryWriteWithTaggingCmd
(
    IN GT_U16                             vlanId,
    IN PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT  taggingCmd
)
{
    GT_U32                              portIter;
    CPSS_PORTS_BMP_STC                  portsMembers;
    CPSS_PORTS_BMP_STC                  portsTagging;
    PRV_TGF_BRG_VLAN_INFO_STC           vlanInfo;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC  portsTaggingCmd;

    /* initialize default settings */
    prvTgfBrgInit();

    /* set default vlan entry */
    cpssOsMemCpy(&vlanInfo, &prvTgfDefVlanInfo, sizeof(vlanInfo));

    /* start with no ports */
    /* all ports are 'untagged' */
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);

    /* clear portsTaggingCmd */
    cpssOsMemSet(
        &portsTaggingCmd, 0, sizeof(PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC));

    /* add the needed ports to the vlan */
    for(portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        if (prvTgfDevsArray[portIter] != prvTgfDevNum)
        {
            /* this port not belong to current device */
            continue;
        }

        CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers,prvTgfPortsArray[portIter]);

        if(taggingCmd != PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E)
        {
            CPSS_PORTS_BMP_PORT_SET_MAC(&portsTagging,prvTgfPortsArray[portIter]);
        }

        portsTaggingCmd.portsCmd[prvTgfPortsArray[portIter]] =
            taggingCmd;
    }

    return prvTgfBrgVlanEntryWrite(
        prvTgfDevNum, vlanId, &portsMembers, &portsTagging,
        &vlanInfo, &portsTaggingCmd);
}

/**
* @internal prvTgfBrgDefVlanEntryWrite function
* @endinternal
*
* @brief   Set vlan entry, with the all ports of the test as untagged in the vlan
*
* @param[in] vlanId                   - the vlan Id
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfBrgDefVlanEntryWrite
(
    IN GT_U16                         vlanId
)
{
    PRV_UTF_LOG1_MAC("prvTgfBrgDefVlanEntryWrite(%d) \n",
        vlanId);

    return prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
        vlanId, PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E);
}

/**
* @internal prvTgfBrgDefVlanEntryWithPortsSet function
* @endinternal
*
* @brief   Set vlan entry, with the requested ports, tagging
*
* @param[in] vlanId                   - the vlan id
* @param[in] portsArray[]             - array of ports
* @param[in] devArray[]               - array of devices (when NULL -> assume all ports belong to devNum)
* @param[in] tagArray[]               - array of tagging for the ports (when NULL -> assume all ports untagged)
* @param[in] numPorts                 - number of ports in the arrays
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfBrgDefVlanEntryWithPortsSet
(
    IN GT_U16                         vlanId,
    IN GT_U32                          portsArray[],
    IN GT_U8                          devArray[],
    IN GT_U8                          tagArray[],
    IN GT_U32                         numPorts
)
{
    GT_U8                               devNum  = 0;
    GT_STATUS                           rc, rc1 = GT_OK;
    GT_U32                              portIter;
    CPSS_PORTS_BMP_STC                  portsMembers;
    CPSS_PORTS_BMP_STC                  portsTagging;
    PRV_TGF_BRG_VLAN_INFO_STC           vlanInfo;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC  portsTaggingCmd;


    /* initialize default settings */
    prvTgfBrgInit();

    /* start with no ports */
    /* all ports are 'untagged' */
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);

    /* clear portsTaggingCmd */
    cpssOsMemSet(&portsTaggingCmd, 0, sizeof(PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* add the needed ports to the vlan */
        for(portIter = 0; portIter < numPorts; portIter++)
        {
            if(devArray)
            {
                if(devArray[portIter] != devNum)
                {
                    /* this port not belong to current device */
                    continue;
                }
            }

            CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers,portsArray[portIter]);

            if(tagArray)
            {
                CPSS_PORTS_BMP_PORT_SET_MAC(&portsTagging,portsArray[portIter]);

                portsTaggingCmd.portsCmd[portsArray[portIter]] = (tagArray[portIter] & 1) ?
                    PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E :
                    PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E;
            }
        }

        /* set default vlan entry */
        cpssOsMemCpy(&vlanInfo, &prvTgfDefVlanInfo, sizeof(vlanInfo));

        /* call device specific API */
        rc = prvTgfBrgVlanEntryWrite(devNum, vlanId,
                                     &portsMembers, &portsTagging,
                                     &vlanInfo, &portsTaggingCmd);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfBrgVlanEntryWrite FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
}

/**
* @internal prvTgfBrgDefVlanEntryInvalidate function
* @endinternal
*
* @brief   This function invalidates VLAN entry and reset all it's content
*
* @param[in] vlanId                   - VLAN id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PARAM             - wrong devNum or vid
*/
GT_STATUS prvTgfBrgDefVlanEntryInvalidate
(
    IN GT_U16               vlanId
)
{
    GT_STATUS                           rc = GT_OK;
    CPSS_PORTS_BMP_STC                  portsMembers;
    CPSS_PORTS_BMP_STC                  portsTagging;
    PRV_TGF_BRG_VLAN_INFO_STC           vlanInfo;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC  portsTaggingCmd;


    /* initialize default settings */
    prvTgfBrgInit();

    /* start with no ports */
    /* all ports are 'untagged' */
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);

    /* reset vlan entry */
    cpssOsMemSet(&vlanInfo, 0, sizeof(vlanInfo));

    /* reset portsTaggingCmd */
    cpssOsMemSet(&portsTaggingCmd, 0, sizeof(PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC));

    /* write Vlan entry */
    rc = prvTgfBrgVlanEntryWrite(prvTgfDevNum, vlanId,
                                 &portsMembers, &portsTagging,
                                 &vlanInfo, &portsTaggingCmd);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfBrgVlanEntryWrite FAILED, rc = [%d]", rc);

        return rc;
    }

    /* AUTODOC: invalidate vlan entry */
    rc = prvTgfBrgVlanEntryInvalidate(vlanId);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfBrgVlanEntryInvalidate FAILED, rc = [%d]", rc);
    }

    return rc;
}

#ifdef EXMXPM_FAMILY
static CPSS_EXMXPM_INLIF_PORT_MODE_ENT  prvTgfInlifModeArray[PRV_TGF_MAX_PORTS_NUM_CNS];
#endif
/**
* @internal prvTgfBrgVlanMruProfileIdxSet function
* @endinternal
*
* @brief   Set Maximum Receive Unit MRU profile index for a VLAN.
*         MRU VLAN profile sets maximum packet size that can be received
*         for the given VLAN.
*         Value of MRU for profile is set by cpssDxChBrgVlanMruProfileValueSet.
*
* @note   APPLICABLE DEVICES:      DxCh2 and above
* @param[in] vlanId                   - vlan id
* @param[in] mruIndex                 - MRU profile index [0..7]
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanMruProfileIdxSet
(
    IN GT_U16    vlanId,
    IN GT_U32    mruIndex
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;


#ifdef CHX_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgVlanMruProfileIdxSet(devNum, vlanId, mruIndex);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCutThroughVlanEthertypeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_U32      portIter = 0;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            /* store Port Inlif Mode */
            rc = cpssExMxPmInlifPortModeGet(devNum, prvTgfPortsArray[portIter],
                                           &prvTgfInlifModeArray[portIter]);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmInlifPortModeGet FAILED, rc = [%d]", rc);
                rc1 = rc;
            }

            /* set VLAN InLIF Mode per port */
            rc = cpssExMxPmInlifPortModeSet(devNum, prvTgfPortsArray[portIter],
                                           CPSS_EXMXPM_INLIF_PORT_MODE_VLAN_E);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmInlifPortModeSet FAILED, rc = [%d]", rc);
                rc1 = rc;
            }
        }

        /* call device specific API */
        rc = cpssExMxPmInlifUntaggedMruIndexSet(devNum, CPSS_EXMXPM_INLIF_TYPE_VLAN_E,
                                                (GT_U32) vlanId, mruIndex);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmInlifUntaggedMruIndexSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

    return rc1;
}

/**
* @internal prvTgfBrgVlanMruProfileIdxRestore function
* @endinternal
*
* @brief   Restore Maximum Receive Unit MRU profile index for a VLAN/Inlif.
*
* @param[in] vlanId                   - vlan id
* @param[in] mruIndex                 - MRU profile index [0..7]
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanMruProfileIdxRestore
(
    IN GT_U16    vlanId,
    IN GT_U32    mruIndex
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;


#ifdef CHX_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgVlanMruProfileIdxSet(devNum, vlanId, mruIndex);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCutThroughVlanEthertypeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_U32      portIter = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            /* restore port InLIF Mode */
            rc = cpssExMxPmInlifPortModeSet(devNum, prvTgfPortsArray[portIter],
                                           prvTgfInlifModeArray[portIter]);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmInlifPortModeSet FAILED, rc = [%d]", rc);
                rc1 = rc;
            }
        }

        /* call device specific API */
        rc = cpssExMxPmInlifUntaggedMruIndexSet(devNum, CPSS_EXMXPM_INLIF_TYPE_VLAN_E,
                                                (GT_U32) vlanId, mruIndex);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmInlifUntaggedMruIndexSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

    return rc1;
}

/**
* @internal prvTgfBrgVlanMruProfileValueSet function
* @endinternal
*
* @brief   Set MRU value for a VLAN MRU profile.
*         MRU VLAN profile sets maximum packet size that can be received
*         for the given VLAN.
*         cpssDxChBrgVlanMruProfileIdxSet set index of profile for a specific VLAN.
*
* @note   APPLICABLE DEVICES:      DxCh2 and above
* @param[in] mruIndex                 - MRU profile index [0..7]
* @param[in] mruValue                 - MRU value in bytes [0..0xFFFF]
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanMruProfileValueSet
(
    IN GT_U32    mruIndex,
    IN GT_U32    mruValue
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;


#ifdef CHX_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgVlanMruProfileValueSet(devNum, mruIndex, mruValue);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgVlanMruProfileValueSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmBrgVlanUntaggedMruProfileValueSet(devNum, mruIndex, mruValue);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgVlanUntaggedMruProfileValueSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

    return rc1;
}

/**
* @internal prvTgfBrgDefFdbMacEntryOnPortSet function
* @endinternal
*
* @brief   Set FDB entry, as 'mac address + vlan' on port
*
* @param[in] macAddr                  - mac address
* @param[in] vlanId                   - vlan Id
* @param[in] dstDevNum                - destination device
* @param[in] dstPortNum               - destination port
* @param[in] isStatic                 - is the entry static/dynamic
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PARAM             - wrong devNum or vid
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfBrgDefFdbMacEntryOnPortSet
(
    IN TGF_MAC_ADDR                   macAddr,
    IN GT_U16                         vlanId,
    IN GT_HW_DEV_NUM                  dstDevNum,
    IN GT_PORT_NUM                    dstPortNum,
    IN GT_BOOL                        isStatic
)
{
    PRV_TGF_BRG_MAC_ENTRY_STC   macEntry;


    /* initialize default settings */
    prvTgfBrgInit();

    /* create a mac entry with PORT interface */
    cpssOsMemCpy(&macEntry, &prvTgfDefMacEntry, sizeof(macEntry));
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther, macAddr, sizeof(TGF_MAC_ADDR));

    macEntry.key.key.macVlan.vlanId       = vlanId;
    macEntry.dstInterface.type            = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum  = dstDevNum;
    macEntry.dstInterface.devPort.portNum = dstPortNum;
    macEntry.isStatic                     = isStatic;

    return prvTgfBrgFdbMacEntrySet(&macEntry);
}

/**
* @internal prvTgfBrgDefFdbMacEntryOnVidxSet function
* @endinternal
*
* @brief   Set FDB entry, as 'mac address + vlan' on Vidx
*
* @param[in] macAddr                  - mac address
* @param[in] vlanId                   - vlan Id
* @param[in] vidx                     -  number
* @param[in] isStatic                 - is the entry static/dynamic
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PARAM             - wrong devNum or vid
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfBrgDefFdbMacEntryOnVidxSet
(
    IN TGF_MAC_ADDR                   macAddr,
    IN GT_U16                         vlanId,
    IN GT_U16                         vidx,
    IN GT_BOOL                        isStatic
)
{
    PRV_TGF_BRG_MAC_ENTRY_STC   macEntry;


    /* initialize default settings */
    prvTgfBrgInit();

    /* create a mac entry with VIDX interface */
    cpssOsMemCpy(&macEntry, &prvTgfDefMacEntry, sizeof(macEntry));
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther, macAddr, sizeof(TGF_MAC_ADDR));

    macEntry.key.key.macVlan.vlanId = vlanId;
    macEntry.dstInterface.type      = CPSS_INTERFACE_VIDX_E;
    macEntry.dstInterface.vidx      = vidx;
    macEntry.isStatic               = isStatic;

    return prvTgfBrgFdbMacEntrySet(&macEntry);
}

/**
* @internal prvTgfBrgDefFdbMacEntryOnVidSet function
* @endinternal
*
* @brief   Set FDB entry, as 'mac address + vlan' on Vlan ID
*
* @param[in] macAddr                  - mac address
* @param[in] vlanId                   - vlan Id
* @param[in] vid                      - vlan Id
* @param[in] isStatic                 - is the entry static/dynamic
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PARAM             - wrong devNum or vid
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfBrgDefFdbMacEntryOnVidSet
(
    IN TGF_MAC_ADDR                   macAddr,
    IN GT_U16                         vlanId,
    IN GT_U16                         vid,
    IN GT_BOOL                        isStatic
)
{
    PRV_TGF_BRG_MAC_ENTRY_STC   macEntry;


    /* initialize default settings */
    prvTgfBrgInit();

    /* create a mac entry with VLAN interface */
    cpssOsMemCpy(&macEntry, &prvTgfDefMacEntry, sizeof(macEntry));
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther, macAddr, sizeof(TGF_MAC_ADDR));

    macEntry.key.key.macVlan.vlanId = vlanId;
    macEntry.dstInterface.type      = CPSS_INTERFACE_VID_E;
    macEntry.dstInterface.vlanId    = vid;
    macEntry.daRoute                = GT_TRUE;
    macEntry.isStatic               = isStatic;

    return prvTgfBrgFdbMacEntrySet(&macEntry);
}

/**
* @internal prvTgfBrgDefFdbMacEntryOnTrunkSet function
* @endinternal
*
* @brief   Set FDB entry, as 'mac address + vlan' on Trunk
*
* @param[in] macAddr                  - mac address
* @param[in] vlanId                   - vlan Id
* @param[in] trunkId                  - trunk Id
* @param[in] isStatic                 - is the entry static/dynamic
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfBrgDefFdbMacEntryOnTrunkSet
(
    IN TGF_MAC_ADDR                   macAddr,
    IN GT_U16                         vlanId,
    IN GT_U16                         trunkId,
    IN GT_BOOL                        isStatic
)
{
    PRV_TGF_BRG_MAC_ENTRY_STC   macEntry;


    /* initialize default settings */
    prvTgfBrgInit();

    /* create a mac entry with Trunk interface */
    cpssOsMemCpy(&macEntry, &prvTgfDefMacEntry, sizeof(macEntry));
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther, macAddr, sizeof(TGF_MAC_ADDR));

    macEntry.key.key.macVlan.vlanId = vlanId;
    macEntry.dstInterface.type      = CPSS_INTERFACE_TRUNK_E;
    macEntry.dstInterface.trunkId   = trunkId;
    macEntry.isStatic               = isStatic;

    return prvTgfBrgFdbMacEntrySet(&macEntry);
}

/**
* @internal prvTgfBrgVidxEntrySet function
* @endinternal
*
* @brief   Set vidx entry with the requested ports
*
* @param[in] vidx                     -  number
* @param[in] portsArray[]             - array of ports
* @param[in] devicesArray[]           - array of devices
* @param[in] numPorts                 - number of ports in the arrays
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PARAM             - wrong devNum or vid
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note When devicesArray is NULL assume all ports belong to devNum
*
*/
GT_STATUS prvTgfBrgVidxEntrySet
(
    IN GT_U16                         vidx,
    IN GT_U32                          portsArray[],
    IN GT_U8                          devicesArray[],
    IN GT_U8                          numPorts
)
{
    GT_U32              portIter;
    CPSS_PORTS_BMP_STC  portsMembers;


    /* initialize default settings */
    prvTgfBrgInit();

    /* start with no ports */
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);

    /* add the needed ports to the vidx */
    for(portIter = 0; portIter < numPorts; portIter++)
    {
        if(devicesArray)
        {
            if(devicesArray[portIter] != prvTgfDevNum)
            {
                /* this port not belong to current device */
                continue;
            }
        }

        CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers,portsArray[portIter]);
    }

    /* create a new record in MC Group Table with new VDIX entry */
    return prvTgfBrgMcEntryWrite(prvTgfDevNum, vidx, &portsMembers);
}

/**
* @internal prvTgfBrgVlanPvidSet function
* @endinternal
*
* @brief   Set port's Vlan Id to all tested ports
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PARAM             - wrong devNum or vid
*/
GT_STATUS prvTgfBrgVlanPvidSet
(
    IN GT_U16                         pvid
)
{
    GT_STATUS   rc       = GT_OK;
    GT_U32      portIter = 0;


    /* add the needed ports to the vlan */
    for(portIter = 0 ; portIter < prvTgfPortsNum; portIter++)
    {
        if(prvTgfDevsArray[portIter] != prvTgfDevNum)
        {
            /* this port not belong to current device */
            continue;
        }

        rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[portIter], pvid);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfBrgVlanPortVidSet FAILED, rc = [%d]", rc);

            return rc;
        }
    }

    return rc;
}

/**
* @internal prvTgfBrgVlanTpidEntrySet function
* @endinternal
*
* @brief   Function sets TPID (Tag Protocol ID) table entry.
*
* @param[in] direction                - ingress/egress direction
* @param[in] entryIndex               - entry index for TPID table
* @param[in] etherType                - Tag Protocol ID value
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong entryIndex, direction
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanTpidEntrySet
(
    IN  CPSS_DIRECTION_ENT  direction,
    IN  GT_U32              entryIndex,
    IN  GT_U16              etherType
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
    GT_U32  tmpIndex;
    GT_U16  tmpEtherType;
    GT_U32  numOf0x8100Entries = 0;
    GT_U32  largestIndex;
    GT_BOOL allowConfig = GT_TRUE;

    /* PRV_UTF_LOG3_MAC("prvTgfBrgVlanTpidEntrySet(%d,%d,0x%x) \n",
        direction,entryIndex,etherType); */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        if( GT_FALSE == prvTgfCommonIsDeviceForce(devNum))
        {
            /* this device not need it */
            continue;
        }


        /* traffic that egress from cascade/ring port , must be recognized as with
           2 tags (first tag in dsa tag) on the ingress of the cascade port/ring port */
        if(direction == CPSS_DIRECTION_EGRESS_E)
        {
            /* check that there is free index left in the ingress table (look for 0x8100) */
            largestIndex = 0;
            tmpEtherType = 0;
            for(tmpIndex = 8 ; tmpIndex != 0 ; tmpIndex--)
            {
                /* set the member with full bmp for the ethertype */
                rc = cpssDxChBrgVlanTpidEntryGet(devNum,
                                CPSS_DIRECTION_INGRESS_E,
                                tmpIndex - 1,&tmpEtherType);
                if(rc != GT_OK)
                {
                    PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgVlanTpidEntryGet FAILED, rc = [%d]", rc);
                    return rc;
                }

                if(tmpEtherType == etherType)
                {
                    /* the ethretype already exists in the ingress table too */
                    break;
                }

                if(tmpEtherType == 0x8100)
                {
                    if(numOf0x8100Entries == 0)
                    {
                        largestIndex = tmpIndex - 1;
                    }

                    numOf0x8100Entries++;

                    if(numOf0x8100Entries == 2)
                    {
                        /* we can use this index */
                        break;
                    }
                }
            }

            if(tmpEtherType == etherType)
            {
                /* do nothing - the ethretype already exists in the ingress table too */
                continue;
            }
            else if(tmpIndex == 0)
            {
                rc = GT_FULL;
                /* not found free place --> maybe use better algorithm for
                    duplications and not only 0x8100 */
                PRV_UTF_LOG0_MAC("[TGF]: prvTgfBrgVlanTpidEntrySet : table full \n");
                /*return rc;*/
            }
            else
            {
                /* found 2 places , use the largest index */
                rc = cpssDxChBrgVlanTpidEntrySet(devNum, CPSS_DIRECTION_INGRESS_E, largestIndex, etherType);
                if (GT_OK != rc)
                {
                    PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgVlanTpidEntrySet FAILED, rc = [%d]", rc);

                    return rc;
                }
            }
        }
        else /*CPSS_DIRECTION_INGRESS_E*/
        {
            if(entryIndex > 5)
            {
                if(GT_FALSE == prvTgfCommonIsUseLastTpid())
                {
                    /* don't allow test to override last 2 indexes in the ingress --
                        we need it for the cascade/ring ports */
                    allowConfig = GT_FALSE;
                }
            }
        }
    }

    if(allowConfig == GT_FALSE)
    {
        return GT_OK;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgVlanTpidEntrySet(devNum, direction, entryIndex, etherType);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgVlanTpidEntrySet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(direction);
    TGF_PARAM_NOT_USED(entryIndex);
    TGF_PARAM_NOT_USED(etherType);

    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgVlanTpidEntryGet function
* @endinternal
*
* @brief   Function gets TPID (Tag Protocol ID) table entry.
*
* @param[in] direction                - ingress/egress direction
* @param[in] entryIndex               - entry index for TPID table
*
* @param[out] etherTypePtr             - (pointer to) Tag Protocol ID value
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, entryIndex, direction
* @retval GT_BAD_PTR               - etherTypePtr is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanTpidEntryGet
(
    IN  CPSS_DIRECTION_ENT  direction,
    IN  GT_U32              entryIndex,
    OUT GT_U16              *etherTypePtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgVlanTpidEntryGet(prvTgfDevNum, direction, entryIndex, etherTypePtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(direction);
    TGF_PARAM_NOT_USED(entryIndex);
    TGF_PARAM_NOT_USED(etherTypePtr);

    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgVlanPortEgressTpidSet function
* @endinternal
*
* @brief   Function sets index of TPID (Tag protocol ID) table per egress port.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (CPU port is supported)
* @param[in] ethMode                  - TAG0/TAG1 selector
* @param[in] tpidEntryIndex           - TPID table entry index (0-7)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, ethMode
* @retval GT_OUT_OF_RANGE          - illegal tpidEntryIndex (not 0-7)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanPortEgressTpidSet
(
    IN  GT_U8                devNum,
    IN  GT_U32                portNum,
    IN  CPSS_ETHER_MODE_ENT  ethMode,
    IN  GT_U32               tpidEntryIndex
)
{
#ifdef CHX_FAMILY
    if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum))
    {
        GT_STATUS   rc;
        #if 0
            CPSS_TBD_BOOKMARK_EARCH

            /* PATCH to allow TS TPID to be set like the 'ETHERTYPE0' to give
               backwards compatible to TS tests */

        #endif /*0*/
        if(ethMode == CPSS_VLAN_ETHERTYPE0_E)
        {
            rc = cpssDxChTunnelStartHeaderTpidSelectSet(devNum, portNum,tpidEntryIndex);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }


    /* call device specific API */
    return cpssDxChBrgVlanPortEgressTpidSet(devNum, portNum, ethMode, tpidEntryIndex);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(ethMode);
    TGF_PARAM_NOT_USED(tpidEntryIndex);

    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}


/**
* @internal prvTgfBrgVlanEgressTagTpidSelectModeSet function
* @endinternal
*
* @brief   Set Tag 0/1 TPID (Tag Protocol Identifies) Select Mode
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] ethMode                  - TAG0/TAG1 selector
* @param[in] mode                     - 0: TPID of Tag0/1 is selected according to
*                                      <Egress Tag 0/1 TPID select>
*                                      1: TPID of Tag0/1 is selected according to
*                                      VID0/1 to TPID select table
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanEgressTagTpidSelectModeSet
(
    IN GT_U8                                        devNum,
    IN GT_PORT_NUM                                  portNum,
    IN CPSS_ETHER_MODE_ENT                          ethMode,
    IN PRV_TGF_BRG_VLAN_TAG_TPID_SELECT_MODE_ENT    mode
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_BRG_VLAN_TAG_TPID_SELECT_MODE_ENT dxMode;

    switch(mode)
    {
        case PRV_TGF_BRG_VLAN_EGRESS_TAG_TPID_SELECT_MODE_E:
            dxMode = CPSS_DXCH_BRG_VLAN_EGRESS_TAG_TPID_SELECT_MODE_E;
            break;
        case PRV_TGF_BRG_VLAN_VID_TO_TPID_SELECT_TABLE_MODE_E:
            dxMode = CPSS_DXCH_BRG_VLAN_VID_TO_TPID_SELECT_TABLE_MODE_E;
            break;
        default:
            return GT_BAD_PARAM;

    }

    return cpssDxChBrgVlanEgressTagTpidSelectModeSet(devNum,portNum,ethMode,dxMode);
#endif/*CHX_FAMILY*/

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(ethMode);
    TGF_PARAM_NOT_USED(mode);

    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}


/**
* @internal prvTgfBrgVlanPortEgressTpidGet function
* @endinternal
*
* @brief   Function gets index of TPID (Tag protocol ID) table per egress port.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (CPU port is supported)
* @param[in] ethMode                  - TAG0/TAG1 selector
*
* @param[out] tpidEntryIndexPtr        - (pointer to) TPID table entry index
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, ethMode
* @retval GT_OUT_OF_RANGE          - illegal tpidEntryIndex (not 0-7)
* @retval GT_BAD_PTR               - tpidEntryIndexPtr is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanPortEgressTpidGet
(
    IN  GT_U8                devNum,
    IN  GT_U32                portNum,
    IN  CPSS_ETHER_MODE_ENT  ethMode,
    OUT GT_U32               *tpidEntryIndexPtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgVlanPortEgressTpidGet(devNum, portNum, ethMode, tpidEntryIndexPtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(ethMode);
    TGF_PARAM_NOT_USED(tpidEntryIndexPtr);

    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgVlanBridgingModeSet function
* @endinternal
*
* @brief   Set bridging mode either VLAN-aware (802.1Q) or VLAN-unaware (802.1D) mode
*         The device supports a VLAN-unaware mode for 802.1D bridging.
*         When this mode is enabled:
*         - In VLAN-unaware mode, the device does not perform any packet
*         modifications. Packets are always transmitted as-received, without any
*         modification (i.e., packets received tagged are transmitted tagged;
*         packets received untagged are transmitted untagged).
*         - Packets are implicitly assigned with VLAN-ID 1, regardless of
*         VLAN-assignment mechanisms.
*         - Packets are implicitly assigned a VIDX Multicast group index 0xFFF,
*         indicating that the packet flood domain is according to the VLAN-in
*         this case VLAN 1. Registered Multicast is not supported in this mode.
*         All other features are operational in this mode, including internal
*         packet QoS, trapping, filtering, policy, etc.
* @param[in] brgMode                  - bridge mode: IEEE 802.1Q bridge or IEEE 802.1D bridge
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanBridgingModeSet
(
    IN CPSS_BRG_MODE_ENT    brgMode
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgVlanBridgingModeSet(devNum, brgMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgVlanBridgingModeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    if(brgMode == CPSS_BRG_MODE_802_1Q_E)
    {
        /* the device work in this mode , and no API to change it.*/
        return GT_OK;
    }

    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgVlanPortTranslationEnableSet function
* @endinternal
*
* @brief   Enable/Disable Vlan Translation per ingress or egress port.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - port number or CPU port number for egress direction.
* @param[in] direction                - ingress or egress
* @param[in] enable                   - enable/disable ingress/egress Vlan Translation.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanPortTranslationEnableSet
(
    IN GT_U8                              devNum,
    IN GT_PORT_NUM                        portNum,
    IN CPSS_DIRECTION_ENT                 direction,
    IN PRV_TGF_BRG_VLAN_TRANSLATION_ENT   enable
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_BRG_VLAN_TRANSLATION_ENT cpssEnable;

    switch (enable)
    {
        case PRV_TGF_BRG_VLAN_TRANSLATION_DISABLE_E:
            cpssEnable = CPSS_DXCH_BRG_VLAN_TRANSLATION_DISABLE_E;
            break;
        case PRV_TGF_BRG_VLAN_TRANSLATION_VID0_E:
            cpssEnable = CPSS_DXCH_BRG_VLAN_TRANSLATION_VID0_E;
            break;
        case PRV_TGF_BRG_VLAN_TRANSLATION_SID_VID0_E:
            cpssEnable = CPSS_DXCH_BRG_VLAN_TRANSLATION_SID_VID0_E;
            break;
        case PRV_TGF_BRG_VLAN_TRANSLATION_SID_VID0_VID1_E:
            cpssEnable = CPSS_DXCH_BRG_VLAN_TRANSLATION_SID_VID0_VID1_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    return cpssDxChBrgVlanPortTranslationEnableSet(devNum, portNum, direction,
                                                   cpssEnable);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgVlanPortTranslationEnableGet function
* @endinternal
*
* @brief   Get the status of Vlan Translation (Enable/Disable) per ingress or
*         egress port.
* @param[in] devNum                   - the device number
* @param[in] direction                - ingress or egress
* @param[in] portNum                  - port number or CPU port number for egress direction.
*
* @param[out] enablePtr                - (pointer to) ingress/egress Vlan Translation status
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanPortTranslationEnableGet
(
    IN  GT_U8                              devNum,
    IN  GT_PORT_NUM                        portNum,
    IN  CPSS_DIRECTION_ENT                 direction,
    OUT PRV_TGF_BRG_VLAN_TRANSLATION_ENT   *enablePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS                          rc;
    CPSS_DXCH_BRG_VLAN_TRANSLATION_ENT cpssEnable;

    rc = cpssDxChBrgVlanPortTranslationEnableGet(devNum, portNum, direction,
                                                 &cpssEnable);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgVlanPortTranslationEnableGet FAILED, rc = [%d]", rc);
        return rc;
    }

    switch (cpssEnable)
    {
        case CPSS_DXCH_BRG_VLAN_TRANSLATION_DISABLE_E:
            *enablePtr = PRV_TGF_BRG_VLAN_TRANSLATION_DISABLE_E;
            break;
        case CPSS_DXCH_BRG_VLAN_TRANSLATION_VID0_E:
            *enablePtr = PRV_TGF_BRG_VLAN_TRANSLATION_VID0_E;
            break;
        case CPSS_DXCH_BRG_VLAN_TRANSLATION_SID_VID0_E:
            *enablePtr = PRV_TGF_BRG_VLAN_TRANSLATION_SID_VID0_E;
            break;
        case CPSS_DXCH_BRG_VLAN_TRANSLATION_SID_VID0_VID1_E:
            *enablePtr = PRV_TGF_BRG_VLAN_TRANSLATION_SID_VID0_VID1_E;
            break;
        default:
            return GT_FAIL;
    }

    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgVlanTranslationEntryWrite function
* @endinternal
*
* @brief   Write an entry into Vlan Translation Table Ingress or Egress
*         This mapping avoids collisions between a VLAN-ID
*         used for a logical interface and a VLAN-ID used by the local network.
* @param[in] devNum                   - the device number
* @param[in] vlanId                   - VLAN id, used as index in the Translation Table
*                                      (APPLICABLE RANGES: 0..4095).
*                                      for ingress : (APPLICABLE RANGES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin 0..4095)
*                                      for egress  : (APPLICABLE RANGES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin 0..8191)
* @param[in] direction                - ingress or egress
* @param[in] transVlanId              - Translated Vlan ID, use as value in the
*                                      Translation Table (APPLICABLE RANGES: 0..4095).
*                                      for ingress : (APPLICABLE RANGES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin 0..8191)
*                                      for egress  : (APPLICABLE RANGES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin 0..4095)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or vlanId
* @retval GT_OUT_OF_RANGE          - when transVlanId is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanTranslationEntryWrite
(
    IN GT_U8                        devNum,
    IN GT_U16                       vlanId,
    IN CPSS_DIRECTION_ENT           direction,
    IN GT_U16                       transVlanId
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgVlanTranslationEntryWrite(devNum, vlanId, direction, transVlanId);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgVlanTranslationEntryRead function
* @endinternal
*
* @brief   Read an entry from Vlan Translation Table, Ingress or Egress.
*
* @param[in] devNum                   - the device number
* @param[in] vlanId                   - VLAN id, used as index in the Translation Table, 0..4095.
*                                      for ingress : (APPLICABLE RANGES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin 0..4095)
*                                      for egress  : (APPLICABLE RANGES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin 0..8191)
* @param[in] direction                - ingress or egress
*
* @param[out] transVlanIdPtr           - (pointer to) Translated Vlan ID, used as value in the
*                                      Translation Table.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or vlanId
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanTranslationEntryRead
(
    IN  GT_U8                       devNum,
    IN  GT_U16                      vlanId,
    IN CPSS_DIRECTION_ENT           direction,
    OUT GT_U16                      *transVlanIdPtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgVlanTranslationEntryRead(devNum, vlanId, direction, transVlanIdPtr);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgGenIeeeReservedMcastTrapEnable function
* @endinternal
*
* @brief   Enables trapping or mirroring to CPU Multicast packets, with MAC_DA in
*         the IEEE reserved Multicast range (01-80-C2-00-00-xx).
* @param[in] dev                      - physical device number
* @param[in] enable                   - GT_FALSE -
*                                      Trapping or mirroring to CPU of packet with
*                                      MAC_DA = 01-80-C2-00-00-xx disabled.
*                                      GT_TRUE -
*                                      Trapping or mirroring to CPU of packet, with
*                                      MAC_DA = 01-80-C2-00-00-xx enabled according to the setting
*                                      of IEEE Reserved Multicast Configuration Register<n> (0<=n<16)
*                                      by cpssDxChBrgGenIeeeReservedMcastProtCmdSet function
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgGenIeeeReservedMcastTrapEnable
(
    IN GT_U8    dev,
    IN GT_BOOL  enable
)
{
#ifdef EXMXPM_FAMILY
    GT_STATUS   rc;
    GT_U8  port;
#endif /*EXMXPM_FAMILY*/

#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgGenIeeeReservedMcastTrapEnable(dev, enable);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

    for(port = 0 ; port < PRV_CPSS_PP_MAC(dev)->numOfVirtPorts ; port++)
    {
        rc = cpssExMxPmSctPortIeeeReservedMcTrapEnableSet(dev,port,enable);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    /*cpssExMxPmBrgGenBpduTrapEnableSet(dev, enable)*/
    return GT_OK;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgGenPortIeeeReservedMcastProfileIndexSet function
* @endinternal
*
* @brief   Select the IEEE Reserved Multicast profile (table) associated with port
*
* @param[in] dev                      - device number
* @param[in] port                     - number (including CPU port)
* @param[in] profileIndex             - profile index (0..3). The parameter defines profile (table
*                                      number) for the 256 protocols.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong dev, port
* @retval GT_OUT_OF_RANGE          - for profileIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgGenPortIeeeReservedMcastProfileIndexSet
(
    IN GT_U8                dev,
    IN GT_U32                port,
    IN GT_U32               profileIndex
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgGenPortIeeeReservedMcastProfileIndexSet(dev, port, profileIndex);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmSctPortIeeeReservedMcProfileSet(dev, port, profileIndex);
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgGenIeeeReservedMcastProtCmdSet function
* @endinternal
*
* @brief   Enables forwarding, trapping, or mirroring to the CPU any of the
*         packets with MAC DA in the IEEE reserved, Multicast addresses
*         for bridge control traffic, in the range of 01-80-C2-00-00-xx,
*         where 0<=xx<256
* @param[in] dev                      - device number
* @param[in] profileIndex             - profile index (0..3). Parameter is relevant only for
*                                      DxChXcat and above. The parameter defines profile (table
*                                      number) for the 256 protocols. Profile bound per port by
*                                      <prvTgfBrgGenPortIeeeReservedMcastProfileIndexSet>.
* @param[in] protocol                 - specifies the Multicast protocol
* @param[in] cmd                      - supported commands:
*                                      CPSS_PACKET_CMD_FORWARD_E , CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E , CPSS_PACKET_CMD_DROP_SOFT_E
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number, protocol or control packet command
*                                       of profileIndex.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note BPDU packets with MAC DA = 01-80-C2-00-00-00 are not affect
*       by this mechanism. BPDUs are implicitly trapped to the CPU
*       if the ingress port span tree is not disabled.
*
*/
GT_STATUS prvTgfBrgGenIeeeReservedMcastProtCmdSet
(
    IN GT_U8                dev,
    IN GT_U32               profileIndex,
    IN GT_U8                protocol,
    IN CPSS_PACKET_CMD_ENT  cmd
)
{
#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_SCT_IEEE_RSV_MC_PROTOCOL_ENT    protocolType = CPSS_EXMXPM_SCT_IEEE_RSV_MC_PROTOCOL_STD_E;
#endif /* EXMXPM_FAMILY */

#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgGenIeeeReservedMcastProtCmdSet(dev,
                                                     profileIndex,
                                                     protocol,
                                                     cmd);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

    /* convert protocol to : protocol-type and sub-Protocol */

    switch(protocol & 0xF0)
    {
        case 0x00:
            protocolType = CPSS_EXMXPM_SCT_IEEE_RSV_MC_PROTOCOL_STD_E;
            break;
        case 0x20:
            protocolType = CPSS_EXMXPM_SCT_IEEE_RSV_MC_PROTOCOL_GARP_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    protocol &= 0x0F;

    return cpssExMxPmSctIeeeReservedMcProtocolCmdSet(dev,
                                                     profileIndex,
                                                     protocolType,
                                                     protocol,
                                                     cmd);
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgSrcIdPortSrcIdForceEnableSet function
* @endinternal
*
* @brief   Set Source ID Assignment force mode per Ingress Port.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - Physical port number, CPU port
* @param[in] enable                   - enable/disable SourceID force mode
*                                      GT_TRUE - enable Source ID force mode
*                                      GT_FALSE - disable Source ID force mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgSrcIdPortSrcIdForceEnableSet
(
    IN GT_U8    devNum,
    IN GT_U32    portNum,
    IN GT_BOOL  enable
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc;

    /* call device specific API */
    rc = cpssDxChBrgSrcIdPortSrcIdForceEnableSet(devNum, portNum, enable);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgSrcIdPortSrcIdForceEnableSet FAILED, rc = [%d]", rc);
    }
    return rc;
#else /* !CHX_FAMILY */

    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(enable);

    return GT_NOT_SUPPORTED;
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfBrgSrcIdPortSrcIdForceEnableGet function
* @endinternal
*
* @brief   Set Source ID Assignment force mode per Ingress Port.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number, CPU port
*
* @param[out] enablePtr                - (pointer to) Source ID Assignment force mode state
*                                      GT_TRUE - Source ID force mode is enabled
*                                      GT_FALSE - Source ID force mode is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgSrcIdPortSrcIdForceEnableGet
(
    IN GT_U8    devNum,
    IN GT_U32    portNum,
    IN GT_BOOL  *enablePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc;

    /* call device specific API */
    rc = cpssDxChBrgSrcIdPortSrcIdForceEnableGet(devNum, portNum, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgSrcIdPortSrcIdForceEnableGet FAILED, rc = [%d]", rc);
    }
    return rc;
#else /* !CHX_FAMILY */

    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(enablePtr);

    return GT_NOT_SUPPORTED;
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfBrgSrcIdGlobalSrcIdAssignModeSet function
* @endinternal
*
* @brief   Set Source ID Assignment mode.
*
* @param[in] mode                     - the assignment  of the packet Source ID.
*                                      CPSS_BRG_SRC_ID_ASSIGN_MODE_FDB_DA_PORT_DEFAULT_E mode is
*                                      supported only for DxChXcat and above.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgSrcIdGlobalSrcIdAssignModeSet
(
    IN CPSS_BRG_SRC_ID_ASSIGN_MODE_ENT  mode
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgSrcIdGlobalSrcIdAssignModeSet(devNum, mode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgSrcIdGlobalSrcIdAssignModeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#else /* !CHX_FAMILY */
    TGF_PARAM_NOT_USED(mode);

    return GT_NOT_SUPPORTED;
#endif /* CHX_FAMILY */
}

/**
* @internal prvTgfBrgSrcIdPortDefaultSrcIdSet function
* @endinternal
*
* @brief   Configure Port's Default Source ID.
*         The Source ID is used for source based egress filtering.
* @param[in] devNum                   - device number
* @param[in] portNum                  - Physical port number, CPU port
* @param[in] defaultSrcId             - Port's Default Source ID (0..31)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum, defaultSrcId
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgSrcIdPortDefaultSrcIdSet
(
    IN GT_U8   devNum,
    IN GT_U32   portNum,
    IN GT_U32  defaultSrcId
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgSrcIdPortDefaultSrcIdSet(devNum, portNum, defaultSrcId);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmBrgSrcIdPortDefaultSrcIdSet(devNum, portNum, defaultSrcId);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgSrcIdPortDefaultSrcIdGet function
* @endinternal
*
* @brief   Get configuration of Port's Default Source ID.
*         The Source ID is used for Source based egress filtering.
* @param[in] devNum                   - device number
* @param[in] portNum                  - Physical port number, CPU port
*
* @param[out] defaultSrcIdPtr          - pointer to Port's Default Source ID (0..31)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum, defaultSrcId
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgSrcIdPortDefaultSrcIdGet
(
    IN  GT_U8   devNum,
    IN  GT_U32   portNum,
    OUT GT_U32  *defaultSrcIdPtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgSrcIdPortDefaultSrcIdGet(devNum, portNum, defaultSrcIdPtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmBrgSrcIdPortDefaultSrcIdGet(devNum, portNum, defaultSrcIdPtr);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgSrcIdGroupPortDelete function
* @endinternal
*
* @brief   Delete a port from Source ID group. Packets assigned with this
*         Source ID will not be forwarded to this port.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
* @param[in] devNum                   - device number
* @param[in] sourceId                 - Source ID  number that the port is added to (0..31).
* @param[in] portNum                  - Physical port number, CPU port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum, sourceId
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgSrcIdGroupPortDelete
(
    IN GT_U8   devNum,
    IN GT_U32  sourceId,
    IN GT_U32   portNum
)
{
    GT_STATUS   rc    = GT_OK;

#ifdef CHX_FAMILY

    /* call device specific API */
    rc = cpssDxChBrgSrcIdGroupPortDelete(devNum, sourceId, portNum);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgSrcIdGroupPortDelete FAILED, rc = [%d]", rc);

        return rc;
    }
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

        /* call device specific API */
        rc = cpssExMxPmBrgSrcIdGroupPortDelete(devNum, sourceId, portNum);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgSrcIdGroupPortDelete FAILED, rc = [%d]", rc);

            return rc;
        }

#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

    return rc;
}

/**
* @internal prvTgfBrgSrcIdPortUcastEgressFilterSet function
* @endinternal
*
* @brief   Per Egress Port enable or disable Source ID egress filter for unicast
*         packets. The Source ID filter is configured by
*         cpssDxChBrgSrcIdGroupPortAdd and cpssDxChBrgSrcIdGroupPortDelete.
* @param[in] devNum                   - device number
* @param[in] portNum                  - Physical port number/all ports wild card, CPU port
* @param[in] enable                   - GT_TRUE -  Source ID filtering on unicast packets
*                                      forwarded to this port. Unicast packet is dropped
*                                      if egress port is not member in the Source ID group.
*                                      - GT_FALSE - disable Source ID filtering on unicast packets
*                                      forwarded to this port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgSrcIdPortUcastEgressFilterSet
(
    IN GT_U8    devNum,
    IN GT_U32    portNum,
    IN GT_BOOL  enable
)
{
    GT_STATUS   rc    = GT_OK;

#ifdef CHX_FAMILY

    /* call device specific API */
    rc = cpssDxChBrgSrcIdPortUcastEgressFilterSet(devNum, portNum, enable);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgSrcIdPortUcastEgressFilterSet FAILED, rc = [%d]", rc);

        return rc;
    }
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

        /* call device specific API */
        rc = cpssExMxPmBrgSrcIdPortUcEgressFilterEnableSet(devNum, portNum, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgSrcIdPortUcEgressFilterEnableSet FAILED, rc = [%d]", rc);

            return rc;
        }

#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

    return rc;
}

/**
* @internal prvTgfBrgSrcIdGroupPortAdd function
* @endinternal
*
* @brief   Add a port to Source ID group. Packets assigned with this Source ID
*         may be forwarded to this port.
* @param[in] devNum                   - device number
* @param[in] sourceId                 - Source ID  number that the port is added to (0..31).
* @param[in] portNum                  - Physical port number, CPU port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum, sourceId
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgSrcIdGroupPortAdd
(
    IN GT_U8   devNum,
    IN GT_U32  sourceId,
    IN GT_U32   portNum
)
{
    GT_STATUS   rc    = GT_OK;

#ifdef CHX_FAMILY

    /* call device specific API */
    rc = cpssDxChBrgSrcIdGroupPortAdd(devNum, sourceId, portNum);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgSrcIdGroupPortAdd FAILED, rc = [%d]", rc);

        return rc;
    }
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

        /* call device specific API */
        rc = cpssExMxPmBrgSrcIdGroupPortAdd(devNum, sourceId, portNum);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgSrcIdGroupPortAdd FAILED, rc = [%d]", rc);

            return rc;
        }

#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

    return rc;
}

/**
* @internal prvTgfBrgSrcIdPortAssignModeSet function
* @endinternal
*
* @brief   Set Source ID Assignment mode for non-DSA tagged packets
*         per Ingress Port.
* @param[in] devNum                   - device number
* @param[in] portNum                  - Physical port number, CPU port
* @param[in] mode                     - the assignment  of the packet Source ID
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum, defaultSrcId
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgSrcIdPortAssignModeSet
(
    IN GT_U8   devNum,
    IN GT_U32   portNum,
    IN CPSS_BRG_SRC_ID_ASSIGN_MODE_ENT  mode
)
{
    GT_STATUS   rc    = GT_OK;

#ifdef CHX_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(portNum);

    /* call device specific API */
    rc = cpssDxChBrgSrcIdGlobalSrcIdAssignModeSet(devNum, mode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgSrcIdPortSrcIdAssignModeSet FAILED, rc = [%d]", rc);
        return rc;
    }
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    rc = cpssExMxPmBrgSrcIdPortAssignModeSet(devNum, portNum, mode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgSrcIdPortAssignModeSet FAILED, rc = [%d]", rc);
        return rc;
    }
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

    return rc;
}

/**
* @internal prvTgfBrgSrcIdPortAssignModeGet function
* @endinternal
*
* @brief   Get Source ID Assignment mode for non-DSA tagged packets
*         per Ingress Port.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] modePtr                  - the assignment mode of the packet Source ID
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum, defaultSrcId
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgSrcIdPortAssignModeGet
(
    IN  GT_U8                            devNum,
    IN  GT_U8                            portNum,
    OUT CPSS_BRG_SRC_ID_ASSIGN_MODE_ENT  *modePtr
)
{
    GT_STATUS   rc    = GT_OK;

#ifdef CHX_FAMILY
    /* call device specific API */
    rc = cpssDxChBrgSrcIdGlobalSrcIdAssignModeGet(devNum, modePtr);

    /* avoid warnings */
    TGF_PARAM_NOT_USED(portNum);

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgSrcIdPortSrcIdAssignModeGet FAILED, rc = [%d]", rc);
        return rc;
    }
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    rc = cpssExMxPmBrgSrcIdPortAssignModeGet(devNum, portNum, modePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgSrcIdPortSrcIdAssignModeGet FAILED, rc = [%d]", rc);
        return rc;
    }
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

    return rc;
}

/**
* @internal prvTgfBrgVlanRemoveVlanTag1IfZeroModeSet function
* @endinternal
*
* @brief   Set Tag1 removal mode from the egress port Tag State if Tag1 VID
*         is assigned a value of 0.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above
* @param[in] mode                     - Vlan Tag1 Removal  when Tag1 VID=0
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanRemoveVlanTag1IfZeroModeSet
(
    IN  PRV_TGF_BRG_VLAN_REMOVE_TAG1_IF_ZERO_MODE_ENT   mode
)
{
#ifdef CHX_FAMILY
    GT_U8                                             devNum  = 0;
    GT_STATUS                                         rc, rc1 = GT_OK;
    CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_MODE_ENT   cpssMode;

    if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(prvTgfDevNum) &&
        mode == PRV_TGF_BRG_VLAN_REMOVE_TAG1_IF_ZERO_NOT_DOUBLE_TAG_E)
    {
        /* support legacy tests */
        mode = CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_AND_INGRESS_WITHOUT_TAG1_E;
    }

    /* Translate to CPSS mode */
    switch (mode)
    {
        case PRV_TGF_BRG_VLAN_REMOVE_TAG1_IF_ZERO_DISABLE_E:
            cpssMode = CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_DISABLE_E;
            break;
        case PRV_TGF_BRG_VLAN_REMOVE_TAG1_IF_ZERO_NOT_DOUBLE_TAG_E:
            cpssMode = CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_NOT_DOUBLE_TAG_E;
            break;
        case PRV_TGF_BRG_VLAN_REMOVE_TAG1_IF_ZERO_E:
            cpssMode = CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_E;
            break;
        case PRV_TGF_BRG_VLAN_REMOVE_TAG1_IF_ZERO_AND_INGRESS_WITHOUT_TAG1_E:
            cpssMode = CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_AND_INGRESS_WITHOUT_TAG1_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgVlanRemoveVlanTag1IfZeroModeSet(devNum, cpssMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgVlanRemoveVlanTag1IfZeroModeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#else
    /* avoid warnings */
    TGF_PARAM_NOT_USED(mode);

    return GT_BAD_STATE;
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfBrgVlanRemoveVlanTag1IfZeroModeGet function
* @endinternal
*
* @brief   Get Tag1 removal mode from the egress port Tag State when Tag1 VID
*         is assigned a value of 0.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above
*
* @param[out] modePtr                  - (pointer to) Vlan Tag1 Removal mode when Tag1 VID=0
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - modePtr is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanRemoveVlanTag1IfZeroModeGet
(
    OUT PRV_TGF_BRG_VLAN_REMOVE_TAG1_IF_ZERO_MODE_ENT   *modePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc    = GT_OK;

    CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_MODE_ENT   cpssMode;

    /* call device specific API */
    rc = cpssDxChBrgVlanRemoveVlanTag1IfZeroModeGet(prvTgfDevNum, &cpssMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgVlanRemoveVlanTag1IfZeroModeGet FAILED, rc = [%d]", rc);
        return rc;
    }

    /* Translate to CPSS mode */
    switch (cpssMode)
    {
        case CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_DISABLE_E:
            *modePtr = PRV_TGF_BRG_VLAN_REMOVE_TAG1_IF_ZERO_DISABLE_E;
            break;
        case CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_NOT_DOUBLE_TAG_E:
            *modePtr = PRV_TGF_BRG_VLAN_REMOVE_TAG1_IF_ZERO_NOT_DOUBLE_TAG_E;
            break;
        case CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_E:
            *modePtr = PRV_TGF_BRG_VLAN_REMOVE_TAG1_IF_ZERO_E;
            break;
        case CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_AND_INGRESS_WITHOUT_TAG1_E:
            *modePtr = PRV_TGF_BRG_VLAN_REMOVE_TAG1_IF_ZERO_AND_INGRESS_WITHOUT_TAG1_E;
            break;
        default:
            return GT_FAIL;
    }
    return GT_OK;
#else
    /* avoid warnings */
    TGF_PARAM_NOT_USED(modePtr);

    return GT_BAD_STATE;
#endif /* CHX_FAMILY */
}

/**
* @internal prvTgfBrgFdbDeviceTableSet function
* @endinternal
*
* @brief   This function sets the device table of the PP.
*         the PP use this configuration in the FDB aging daemon .
*         once the aging daemon encounter an entry with non-exists devNum
*         associated with it , the daemon will DELETE the entry without sending
*         the CPU any notification.
*         a non-exists device determined by it's bit in the "device table"
*         So for proper work of PP the application must set the relevant bits of
*         all devices in the system prior to inserting FDB entries associated with
*         them
*
* @note   APPLICABLE DEVICES:      All devices except Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
* @param[in] devTableArr[PRV_TGF_FDB_DEV_TABLE_SIZE_CNS] - (array of)bmp of devices to set.
*                                      DxCh devices use only devTableBmp[0]
*                                      ExMxPm devices use devTableBmp[0..3]
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbDeviceTableSet
(
    IN  GT_U32  devTableArr[PRV_TGF_FDB_DEV_TABLE_SIZE_CNS]
)
{
    GT_U8       devNum = 0;
    GT_STATUS   rc, rc1 = GT_OK;

#ifdef CHX_FAMILY

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_CPSS_PP_E_ARCH_CNS);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgFdbDeviceTableSet(devNum, devTableArr[0]);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbDeviceTableSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmBrgFdbDeviceTableSet(devNum, devTableArr);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgFdbDeviceTableSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgFdbDeviceTableGet function
* @endinternal
*
* @brief   This function gets the device table of the PP.
*         the PP use this configuration in the FDB aging daemon.
*         once the aging daemon encounter an entry with non-exists devNum
*         associated with it, the daemon will DELETE the entry without sending
*         the CPU any notification.
*         a non-exists device determined by it's bit in the "device table"
*
* @note   APPLICABLE DEVICES:      All devices except Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[out] devTableArr[PRV_TGF_FDB_DEV_TABLE_SIZE_CNS] - pointer to (Array of) bmp (bitmap) of devices.
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbDeviceTableGet
(
    OUT GT_U32  devTableArr[PRV_TGF_FDB_DEV_TABLE_SIZE_CNS]
)
{
    GT_U8       devNum = 0;
    GT_STATUS   rc, rc1 = GT_OK;

#ifdef CHX_FAMILY

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_CPSS_PP_E_ARCH_CNS);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgFdbDeviceTableGet(devNum, &devTableArr[0]);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbDeviceTableSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }

        break;/* take first device */
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmBrgFdbDeviceTableGet(devNum, devTableArr);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgFdbDeviceTableSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }

        break;/* take first device */
    }

    return rc1;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}


/**
* @internal prvTgfBrgVlanKeepVlan1EnableSet function
* @endinternal
*
* @brief   Enable/Disable keeping VLAN1 in the packet, for packets received with VLAN1,
*         even-though the tag-state of this egress-port is configured in the
*         VLAN-table to "untagged" or "VLAN0".
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] up                       - VLAN tag 0 User Priority
*                                      [0..7].
* @param[in] enable                   - GT_TRUE: If the packet is received with VLAN1 and
*                                      VLAN Tag state is "VLAN0" or "untagged"
*                                      then VLAN1 is not removed from the packet.
*                                      GT_FALSE: Tag state assigned by VLAN is preserved.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_OUT_OF_RANGE          - illegal vidRange
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanKeepVlan1EnableSet
(
    IN  GT_U8       devNum,
    IN  GT_U32       portNum,
    IN  GT_U8       up,
    IN  GT_BOOL     enable
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgVlanKeepVlan1EnableSet(devNum, portNum, up, enable);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(up);
    TGF_PARAM_NOT_USED(enable);

    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgVlanKeepVlan1EnableGet function
* @endinternal
*
* @brief   Get status of keeping VLAN1 in the packet, for packets received with VLAN1.
*         (Enabled/Disabled).
*
* @note   APPLICABLE DEVICES:      Lion and above
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] up                       - VLAN tag 0 User Priority
*                                      [0..7].
*
* @param[out] enablePtr                - GT_TRUE: If the packet is received with VLAN1 and
*                                      VLAN Tag state is "VLAN0" or "untagged"
*                                      then VLAN1 is not removed from the packet.
*                                      GT_FALSE: Tag state assigned by VLAN is preserved.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, port or up
* @retval GT_BAD_PTR               - modePtr is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanKeepVlan1EnableGet
(
    IN  GT_U8       devNum,
    IN  GT_U8       portNum,
    IN  GT_U8       up,
    OUT GT_BOOL     *enablePtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgVlanKeepVlan1EnableGet(devNum, portNum, up, enablePtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(up);
    TGF_PARAM_NOT_USED(enablePtr);

    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgVlanTr101Check function
* @endinternal
*
* @brief   check if the device supports the tr101
*
* @retval GT_TRUE                  - the device supports the tr101
* @retval GT_FALSE                 - the device not supports the tr101
*/
GT_BOOL prvTgfBrgVlanTr101Check(
    void
)
{
#ifdef CHX_FAMILY
    GT_U8     devNum = prvTgfDevNum;

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.featureInfo.tr101Supported == GT_FALSE)
    {
        /* the device not support the tr101 */
        return GT_FALSE;
    }

    return GT_TRUE;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)

    return GT_FALSE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfBrgGenPortIeeeReservedMcastLearningEnableSet function
* @endinternal
*
* @brief   Enable/Disable MAC SA learning on reserved IEEE MC packets, that are
*         trapped to CPU.
*
* @note   APPLICABLE DEVICES:      xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE - learning is performed also for IEEE MC
*                                      trapped packets.
*                                      - GT_FALSE - No learning is performed for IEEE MC
*                                      trapped packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or portNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgGenPortIeeeReservedMcastLearningEnableSet
(
    IN GT_U8    devNum,
    IN GT_U32    portNum,
    IN GT_BOOL  enable
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgGenPortIeeeReservedMcastLearningEnableSet(devNum,
                                                                portNum,
                                                                enable);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    TGF_PARAM_NOT_USED(portNum);
    /* call device specific API */   /* NOTE: the function supported 'per device' and not 'per port' */
    return cpssExMxPmSctIeeeReservedMcTrappedLearnEnableSet(devNum, enable);
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgGenRateLimitGlobalCfgSet function
* @endinternal
*
* @brief   Configures global ingress rate limit parameters - rate limiting mode,
*         Four configurable global time window periods are assigned to ports as
*         a function of their operating speed:
*         10 Gbps, 1 Gbps, 100 Mbps, and 10 Mbps.
* @param[in] devNum                   - device number
* @param[in] brgRateLimitPtr          - pointer to global rate limit parameters structure
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range values
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Supported windows for DxCh devices:
*       1000 Mbps: range - 256-16384 uSec  granularity - 256 uSec
*       100 Mbps: range - 256-131072 uSec  granularity - 256 uSec
*       10  Mbps: range - 256-1048570 uSec granularity - 256 uSec
*       10  Gbps: range - 25.6-104857 uSec granularity - 25.6 uSec
*       For DxCh3 and above: The granularity doesn't depend from PP core clock.
*       For DxCh1, DxCh2:
*       Time ranges and granularity specified for clock 200MHz(DxCh2 - 220 MHz)
*       for they changed linearly with the clock value.
*
*/
GT_STATUS prvTgfBrgGenRateLimitGlobalCfgSet
(
    IN GT_U8                            devNum,
    IN PRV_TGF_BRG_GEN_RATE_LIMIT_STC   *brgRateLimitPtr
)
{
    GT_STATUS   rc;
#ifdef CHX_FAMILY
    CPSS_DXCH_BRG_GEN_RATE_LIMIT_STC    dxChBrgRateLimitCfg;

    dxChBrgRateLimitCfg.win1000Mbps = brgRateLimitPtr->win1000Mbps;
    dxChBrgRateLimitCfg.win100Mbps = brgRateLimitPtr->win100Mbps;
    dxChBrgRateLimitCfg.win10Gbps = brgRateLimitPtr->win10Gbps;
    dxChBrgRateLimitCfg.win10Mbps = brgRateLimitPtr->win10Mbps;
    dxChBrgRateLimitCfg.dropMode = brgRateLimitPtr->dropMode;
    dxChBrgRateLimitCfg.rMode = brgRateLimitPtr->rMode;


    /* call device specific API */
    rc = cpssDxChBrgGenRateLimitGlobalCfgSet(devNum, &dxChBrgRateLimitCfg);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgGenRateLimitGlobalCfgSet FAILED, rc = [%d]", rc);
        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_BRG_GEN_RATE_LIMIT_STC exMxPmBrgRateLimitCfg;

    exMxPmBrgRateLimitCfg.win1000Mbps = brgRateLimitPtr->win1000Mbps;
    exMxPmBrgRateLimitCfg.win100Mbps = brgRateLimitPtr->win100Mbps;
    exMxPmBrgRateLimitCfg.win10Gbps = brgRateLimitPtr->win10Gbps;
    exMxPmBrgRateLimitCfg.win10Mbps = brgRateLimitPtr->win10Mbps;
    exMxPmBrgRateLimitCfg.dropMode = brgRateLimitPtr->dropMode;
    exMxPmBrgRateLimitCfg.rMode = brgRateLimitPtr->rMode;

    switch (brgRateLimitPtr->rateLimitModeL1)
    {
        case PRV_TGF_BRG_GEN_L1_RATE_LIMIT_EXCL_E:
            exMxPmBrgRateLimitCfg.rateLimitModeL1 =
                CPSS_EXMXPM_BRG_GEN_L1_RATE_LIMIT_EXCL_E;
            break;
        case PRV_TGF_BRG_GEN_L1_RATE_LIMIT_INCL_E:
            exMxPmBrgRateLimitCfg.rateLimitModeL1 =
                CPSS_EXMXPM_BRG_GEN_L1_RATE_LIMIT_INCL_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    /* call device specific API */
    rc = cpssExMxPmBrgGenRateLimitGlobalCfgSet(devNum,&exMxPmBrgRateLimitCfg);

    if (rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgGenRateLimitGlobalCfgSet FAILED, rc = [%d]", rc);
        return rc;
    }

    return rc;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}


/**
* @internal prvTgfBrgGenRateLimitGlobalCfgGet function
* @endinternal
*
* @brief   Get global ingress rate limit parameters
*
* @param[in] devNum                   - device number
*
* @param[out] brgRateLimitPtr          - pointer to global rate limit parameters structure
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For DxCh3 and above: The granularity doesn't depend from PP core clock.
*       For DxCh1, DxCh2:
*       Time ranges and granularity specified for clock 200MHz(DxCh2 - 220 MHz)
*       for they changed linearly with the clock value.
*
*/
GT_STATUS prvTgfBrgGenRateLimitGlobalCfgGet
(
    IN  GT_U8                            devNum,
    OUT PRV_TGF_BRG_GEN_RATE_LIMIT_STC   *brgRateLimitPtr
)
{
    GT_STATUS   rc;

#ifdef CHX_FAMILY
    CPSS_DXCH_BRG_GEN_RATE_LIMIT_STC    dxChBrgRateLimitCfg;


    /* call device specific API */
    rc = cpssDxChBrgGenRateLimitGlobalCfgGet(devNum, &dxChBrgRateLimitCfg);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgGenRateLimitGlobalCfgGet FAILED, rc = [%d]", rc);
        return rc;
    }

    brgRateLimitPtr->win1000Mbps = dxChBrgRateLimitCfg.win1000Mbps;
    brgRateLimitPtr->win100Mbps = dxChBrgRateLimitCfg.win100Mbps;
    brgRateLimitPtr->win10Gbps = dxChBrgRateLimitCfg.win10Gbps;
    brgRateLimitPtr->win10Mbps = dxChBrgRateLimitCfg.win10Mbps;
    brgRateLimitPtr->dropMode = dxChBrgRateLimitCfg.dropMode;
    brgRateLimitPtr->rMode = dxChBrgRateLimitCfg.rMode;
    brgRateLimitPtr->rateLimitModeL1 = 0;/* not supported by DXCH */

    return rc;

#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_BRG_GEN_RATE_LIMIT_STC exMxPmBrgRateLimitCfg;

    /* call device specific API */
    rc = cpssExMxPmBrgGenRateLimitGlobalCfgGet(devNum, &exMxPmBrgRateLimitCfg);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgGenRateLimitGlobalCfgGet FAILED, rc = [%d]", rc);
        return rc;
    }

    brgRateLimitPtr->win1000Mbps = exMxPmBrgRateLimitCfg.win1000Mbps;
    brgRateLimitPtr->win100Mbps = exMxPmBrgRateLimitCfg.win100Mbps;
    brgRateLimitPtr->win10Gbps = exMxPmBrgRateLimitCfg.win10Gbps;
    brgRateLimitPtr->win10Mbps = exMxPmBrgRateLimitCfg.win10Mbps;
    brgRateLimitPtr->dropMode = exMxPmBrgRateLimitCfg.dropMode;
    brgRateLimitPtr->rMode = exMxPmBrgRateLimitCfg.rMode;

    brgRateLimitPtr->rateLimitModeL1 = (exMxPmBrgRateLimitCfg.rateLimitModeL1 ==
                                         CPSS_EXMXPM_BRG_GEN_L1_RATE_LIMIT_EXCL_E) ?
        PRV_TGF_BRG_GEN_L1_RATE_LIMIT_EXCL_E : PRV_TGF_BRG_GEN_L1_RATE_LIMIT_INCL_E;

    return rc;

#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgGenPortRateLimitSet function
* @endinternal
*
* @brief   Configures port ingress rate limit parameters
*         Each port maintains rate limits for unknown unicast packets,
*         known unicast packets, multicast packets and broadcast packets,
*         single configurable limit threshold value, and a single internal counter.
* @param[in] devNum                   - device number
* @param[in] port                     -  number
* @param[in] portGfgPtr               - pointer to rate limit configuration for a port
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on error.
* @retval GT_BAD_PARAM             - on wrong device number or port
* @retval GT_OUT_OF_RANGE          - on out of range rate limit values
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgGenPortRateLimitSet
(
    IN  GT_U8                                   devNum,
    IN GT_U32                                    port,
    IN PRV_TGF_BRG_GEN_RATE_LIMIT_PORT_STC      *portGfgPtr
)
{
    GT_STATUS   rc;
#ifdef CHX_FAMILY
    CPSS_DXCH_BRG_GEN_RATE_LIMIT_PORT_STC  dxChPortCfg;

    dxChPortCfg.enableBc = portGfgPtr->enableBc;
    dxChPortCfg.enableMc = portGfgPtr->enableMc;
    dxChPortCfg.enableMcReg = portGfgPtr->enableMcReg;
    dxChPortCfg.enableUcKnown = portGfgPtr->enableUcKnown;
    dxChPortCfg.enableUcUnk = portGfgPtr->enableUcUnk;
    dxChPortCfg.rateLimit = portGfgPtr->rateLimit;

    /* call device specific API */
    rc = cpssDxChBrgGenPortRateLimitSet(devNum, port, &dxChPortCfg);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgGenPortRateLimitSet FAILED, rc = [%d]", rc);
        return rc;
    }

    /* call device specific API for TcpSyn */
    rc = cpssDxChBrgGenPortRateLimitTcpSynSet(devNum, port,
                                              portGfgPtr->enableTcpSyn);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgGenPortRateLimitTcpSynSet FAILED, rc = [%d]", rc);
        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_BRG_GEN_PORT_RATE_LIMIT_STC  exMxPmPortCfg;

    exMxPmPortCfg.enableBc = portGfgPtr->enableBc;
    exMxPmPortCfg.enableMc = portGfgPtr->enableMcReg;
    exMxPmPortCfg.enableMcUnreg = portGfgPtr->enableMc;
    exMxPmPortCfg.enableTcpSyn = portGfgPtr->enableTcpSyn;
    exMxPmPortCfg.enableUcKnown = portGfgPtr->enableUcKnown;
    exMxPmPortCfg.enableUcUnk = portGfgPtr->enableUcUnk;
    exMxPmPortCfg.rateLimit = portGfgPtr->rateLimit;

    /* call device specific API */
    rc = cpssExMxPmBrgGenPortRateLimitSet(devNum, port, &exMxPmPortCfg);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgGenPortRateLimitSet FAILED, rc = [%d]", rc);
        return rc;
    }

    return rc;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}


#ifdef EXMXPM_FAMILY
/**
* @internal prvTgfBrgFdbAuMsgConvertFromExMxPmToGeneric function
* @endinternal
*
* @brief   Convert ExMxPm AU message to generic.
*
* @param[in] exMxPmAuMessagesPtr      - array that holds received ExMxPm AU message
*
* @param[out] auMessagesPtr            - array that holds received AU messages
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - the action succeeded and there are no more waiting
*                                       AU messages
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_PARAM             - wrong devNum
*/
static GT_STATUS prvTgfBrgFdbAuMsgConvertFromExMxPmToGeneric
(
    IN  CPSS_EXMXPM_FDB_UPDATE_MSG_ENTRY_STC    *exMxPmAuMessagesPtr,
    OUT CPSS_MAC_UPDATE_MSG_EXT_STC             *auMessagesPtr
)
{

    auMessagesPtr->aging = exMxPmAuMessagesPtr->aging;
    auMessagesPtr->associatedHwDevNum = exMxPmAuMessagesPtr->associatedDevNum;
    auMessagesPtr->entryOffset = exMxPmAuMessagesPtr->entryOffset;
    auMessagesPtr->entryWasFound = exMxPmAuMessagesPtr->entryWasFound;
    auMessagesPtr->macEntryIndex = exMxPmAuMessagesPtr->fdbEntryIndex;
    auMessagesPtr->skip = exMxPmAuMessagesPtr->skip;
    auMessagesPtr->updType = exMxPmAuMessagesPtr->updType;

    auMessagesPtr->macEntry.appSpecificCpuCode = exMxPmAuMessagesPtr->fdbEntry.appSpecificCpuCode;
    auMessagesPtr->macEntry.daCommand = exMxPmAuMessagesPtr->fdbEntry.daCommand;
    auMessagesPtr->macEntry.daQosIndex = exMxPmAuMessagesPtr->fdbEntry.daQosIndex;
    auMessagesPtr->macEntry.daRoute = exMxPmAuMessagesPtr->fdbEntry.daRoute;
    auMessagesPtr->macEntry.daSecurityLevel = exMxPmAuMessagesPtr->fdbEntry.daSecurityLevel;
    auMessagesPtr->macEntry.isStatic = exMxPmAuMessagesPtr->fdbEntry.isStatic;
    auMessagesPtr->macEntry.mirrorToRxAnalyzerPortEn = exMxPmAuMessagesPtr->fdbEntry.mirrorToRxAnalyzerPortEn;
    auMessagesPtr->macEntry.saCommand = exMxPmAuMessagesPtr->fdbEntry.saCommand;
    auMessagesPtr->macEntry.saQosIndex = exMxPmAuMessagesPtr->fdbEntry.saQosIndex;
    auMessagesPtr->macEntry.saSecurityLevel = exMxPmAuMessagesPtr->fdbEntry.saSecurityLevel;
    auMessagesPtr->macEntry.sourceID = exMxPmAuMessagesPtr->fdbEntry.sourceId;
    auMessagesPtr->macEntry.spUnknown = exMxPmAuMessagesPtr->fdbEntry.spUnknown;
    auMessagesPtr->macEntry.userDefined = exMxPmAuMessagesPtr->fdbEntry.userDefined;

    /* Parse destination interface */
    exMxPmAuMessagesPtr->fdbEntry.dstOutlif.outlifType = CPSS_EXMXPM_OUTLIF_TYPE_LL_E;
    auMessagesPtr->macEntry.dstInterface.type = exMxPmAuMessagesPtr->fdbEntry.dstOutlif.interfaceInfo.type;

    switch (auMessagesPtr->macEntry.dstInterface.type)
    {
        case CPSS_INTERFACE_PORT_E:
            auMessagesPtr->macEntry.dstInterface.devPort.hwDevNum =
                exMxPmAuMessagesPtr->fdbEntry.dstOutlif.interfaceInfo.devPort.hwDevNum;
            auMessagesPtr->macEntry.dstInterface.devPort.portNum =
                exMxPmAuMessagesPtr->fdbEntry.dstOutlif.interfaceInfo.devPort.portNum;
            break;
        case CPSS_INTERFACE_TRUNK_E:
            auMessagesPtr->macEntry.dstInterface.trunkId =
                exMxPmAuMessagesPtr->fdbEntry.dstOutlif.interfaceInfo.trunkId;
            break;
        case CPSS_INTERFACE_VIDX_E:
            auMessagesPtr->macEntry.dstInterface.vidx =
                exMxPmAuMessagesPtr->fdbEntry.dstOutlif.interfaceInfo.vidx;
            break;
        case CPSS_INTERFACE_VID_E:
            auMessagesPtr->macEntry.dstInterface.vlanId =
                exMxPmAuMessagesPtr->fdbEntry.dstOutlif.interfaceInfo.vlanId;
            break;
        case CPSS_INTERFACE_DEVICE_E:
            auMessagesPtr->macEntry.dstInterface.hwDevNum =
                exMxPmAuMessagesPtr->fdbEntry.dstOutlif.interfaceInfo.hwDevNum;
            break;
        case CPSS_INTERFACE_FABRIC_VIDX_E:
            auMessagesPtr->macEntry.dstInterface.fabricVidx =
                exMxPmAuMessagesPtr->fdbEntry.dstOutlif.interfaceInfo.fabricVidx;
            break;
        case CPSS_INTERFACE_INDEX_E:
            auMessagesPtr->macEntry.dstInterface.index =
                exMxPmAuMessagesPtr->fdbEntry.dstOutlif.interfaceInfo.index;
            break;
        default:
            break;
    }

    auMessagesPtr->macEntry.key.entryType = exMxPmAuMessagesPtr->fdbEntry.key.entryType;

    /* Parse key */
    if (auMessagesPtr->macEntry.key.entryType == CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E)
    {
        auMessagesPtr->macEntry.key.key.macVlan.vlanId =
            (GT_U16)exMxPmAuMessagesPtr->fdbEntry.key.key.macFid.fId;
        auMessagesPtr->macEntry.key.key.macVlan.macAddr.arEther[5] =
            exMxPmAuMessagesPtr->fdbEntry.key.key.macFid.macAddr.arEther[5];
        auMessagesPtr->macEntry.key.key.macVlan.macAddr.arEther[4] =
            exMxPmAuMessagesPtr->fdbEntry.key.key.macFid.macAddr.arEther[4];
        auMessagesPtr->macEntry.key.key.macVlan.macAddr.arEther[3] =
            exMxPmAuMessagesPtr->fdbEntry.key.key.macFid.macAddr.arEther[3];
        auMessagesPtr->macEntry.key.key.macVlan.macAddr.arEther[2] =
            exMxPmAuMessagesPtr->fdbEntry.key.key.macFid.macAddr.arEther[2];
        auMessagesPtr->macEntry.key.key.macVlan.macAddr.arEther[1] =
            exMxPmAuMessagesPtr->fdbEntry.key.key.macFid.macAddr.arEther[1];
        auMessagesPtr->macEntry.key.key.macVlan.macAddr.arEther[0] =
            exMxPmAuMessagesPtr->fdbEntry.key.key.macFid.macAddr.arEther[0];
    }
    else
    {
        auMessagesPtr->macEntry.key.key.ipMcast.vlanId =
            (GT_U16)exMxPmAuMessagesPtr->fdbEntry.key.key.ipMcast.fId;

        cpssOsMemCpy(auMessagesPtr->macEntry.key.key.ipMcast.dip,
                     exMxPmAuMessagesPtr->fdbEntry.key.key.ipMcast.dip, 4);
        cpssOsMemCpy(auMessagesPtr->macEntry.key.key.ipMcast.sip,
                     exMxPmAuMessagesPtr->fdbEntry.key.key.ipMcast.sip, 4);
    }

    return GT_OK;
}
#endif /* EXMXPM_FAMILY */

/**
* @internal prvTgfBrgFdbAuMsgBlockGet function
* @endinternal
*
* @brief   The function return a block (array) of FDB Address Update (AU) messages,
*         the max number of elements defined by the caller
* @param[in] devNum                   - the device number from which AU are taken
* @param[in,out] numOfAuPtr               - (pointer to)max number of AU messages to get
* @param[in,out] numOfAuPtr               - (pointer to)actual number of AU messages that were received
*
* @param[out] auMessagesPtr            - array that holds received AU messages
*                                      pointer is allocated by the caller , with enough memory
*                                      for numOfAuPtr entries that was given as INPUT
*                                      parameter.
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - the action succeeded and there are no more waiting
*                                       AU messages
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note for multi-port groups device :
*       1. Unified FDB mode:
*       portGroup indication is required , for:
*       a. Aging calculations of Trunks
*       b. Aging calculations of regular entries when DA refresh is enabled.
*       In these 2 cases entries are considered as aged-out only if AA is
*       receives from all 4 portGroups.
*       2. In Unified-Linked FDB mode and Linked FDB mode:
*       portGroup indication is required for these reasons:
*       a. Aging calculations of Trunk and regular entries which have
*       been written to multiple portGroups.
*       b. New addresses (NA) source portGroup indication is required
*       so application can add new entry to the exact portGroup
*       which saw this station (especially on trunk entries).
*       c. Indication on which portGroup has removed this address when
*       AA of delete is fetched (relevant when same entry was written
*       to multiple portGroups).
*       d. Indication on which portGroup currently has this address
*       when QR (query reply) is being fetched (relevant when same
*       entry was written to multiple portGroups).
*
*/
GT_STATUS prvTgfBrgFdbAuMsgBlockGet
(
    IN     GT_U8                       devNum,
    INOUT  GT_U32                      *numOfAuPtr,
    OUT    CPSS_MAC_UPDATE_MSG_EXT_STC *auMessagesPtr
)
{
    GT_STATUS   rc;

#ifdef CHX_FAMILY
    GT_U32      ii;

    /* call device specific API */
    rc = cpssDxChBrgFdbAuMsgBlockGet(devNum, numOfAuPtr, auMessagesPtr);
    if ((rc != GT_OK) && (rc != GT_NO_MORE))
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbAuMsgBlockGet FAILED, rc = [%d]", rc);
        return rc;
    }

    /* convert device numbers */
    for (ii = 0; ii < *numOfAuPtr; ii++)
    {
        /* convert device number from HW to SW devNum.*/
        rc = prvUtfSwFromHwDeviceNumberGet(auMessagesPtr[ii].associatedHwDevNum, &(auMessagesPtr[ii].associatedHwDevNum));
        if(GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvUtfSwFromHwDeviceNumberGet FAILED, rc = [%d]", rc);
            return rc;
        }

        if (auMessagesPtr[ii].macEntry.dstInterface.type == CPSS_INTERFACE_PORT_E)
        {
            rc = prvUtfSwFromHwDeviceNumberGet(auMessagesPtr[ii].macEntry.dstInterface.devPort.hwDevNum, &(auMessagesPtr[ii].macEntry.dstInterface.devPort.hwDevNum));
            if(GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvUtfSwFromHwDeviceNumberGet FAILED, rc = [%d]", rc);
                return rc;
            }
        }

        if (auMessagesPtr[ii].isMoved == GT_TRUE)
        {
            rc = prvUtfSwFromHwDeviceNumberGet(auMessagesPtr[ii].oldAssociatedHwDevNum, &(auMessagesPtr[ii].oldAssociatedHwDevNum));
            if(GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvUtfSwFromHwDeviceNumberGet FAILED, rc = [%d]", rc);
                return rc;
            }

            if (auMessagesPtr[ii].oldDstInterface.type == CPSS_INTERFACE_PORT_E)
            {
                rc = prvUtfSwFromHwDeviceNumberGet(auMessagesPtr[ii].oldDstInterface.devPort.hwDevNum, &(auMessagesPtr[ii].oldDstInterface.devPort.hwDevNum));
                if(GT_OK != rc)
                {
                    PRV_UTF_LOG1_MAC("[TGF]: prvUtfSwFromHwDeviceNumberGet FAILED, rc = [%d]", rc);
                    return rc;
                }
            }
        }
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_FDB_UPDATE_MSG_ENTRY_STC     *exMxPmAuMessagesPtr;
    GT_U32                                   i;

   /* allocate memory */
    exMxPmAuMessagesPtr = cpssOsMalloc((*numOfAuPtr) * sizeof(CPSS_EXMXPM_FDB_UPDATE_MSG_ENTRY_STC));

    /* reset variables */
    cpssOsMemSet(exMxPmAuMessagesPtr, 0,
                 (*numOfAuPtr) * sizeof(CPSS_EXMXPM_FDB_UPDATE_MSG_ENTRY_STC));

    /* call device specific API */
    rc = cpssExMxPmBrgFdbAuMsgBlockGet(devNum, numOfAuPtr, exMxPmAuMessagesPtr);
    if (rc != GT_OK)
    {
        cpssOsFree(exMxPmAuMessagesPtr);
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgGenPortRateLimitSet FAILED, rc = [%d]", rc);
        return rc;
    }

    for (i = 0; i < (*numOfAuPtr); i++)
    {
        /* Parse AU message */
        rc = prvTgfBrgFdbAuMsgConvertFromExMxPmToGeneric(&(exMxPmAuMessagesPtr[i]),
                                                         &(auMessagesPtr[i]));
        if (rc != GT_OK)
        {
            cpssOsFree(exMxPmAuMessagesPtr);
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfBrgFdbParseAuMsgFromExMxPmToGeneric FAILED, rc = [%d]", rc);
            return rc;
        }
    }
    cpssOsFree(exMxPmAuMessagesPtr);

    return rc;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgFdbNaMsgVid1EnableSet function
* @endinternal
*
* @brief   Enable/Disable Tag1 VLAN Id assignment in vid1 field of the NA AU
*         message CPSS_MAC_UPDATE_MSG_EXT_STC.
*
* @note   APPLICABLE DEVICES:      xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE: vid1 field of the NA AU message is taken from
*                                      Tag1 VLAN.
*                                      GT_FALSE: vid1 field of the NA AU message is taken from
*                                      Tag0 VLAN.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on other error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbNaMsgVid1EnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgFdbNaMsgVid1EnableSet(devNum, enable);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enable);

    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgFdbTriggerAuQueueWa function
* @endinternal
*
* @brief   Trigger the Address Update (AU) queue workaround for case when learning
*         from CPU is stopped due to full Address Update (AU) fifo queue
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_STATE             - the secondary AUQ not reached the end of
*                                       queue , meaning no need to set the secondary
*                                       base address again.
*                                       this protect the SW from losing unread messages
*                                       in the secondary queue (otherwise the PP may
*                                       override them with new messages).
*                                       caller may use function
*                                       cpssDxChBrgFdbSecondaryAuMsgBlockGet(...)
*                                       to get messages from the secondary queue.
* @retval GT_NOT_SUPPORTED         - the device not need / not support the WA
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbTriggerAuQueueWa
(
    IN GT_U8    devNum
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgFdbTriggerAuQueueWa(devNum);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);

    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgFdbQueueRewindStatusGet function
* @endinternal
*
* @brief   function check if the specific AUQ was 'rewind' since the last time
*         this function was called for that AUQ
*         this information allow the application to know when to finish processing
*         of the WA relate to cpssDxChBrgFdbTriggerAuQueueWa(...)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] rewindPtr                - (pointer to) was the AUQ rewind since last call
*                                      GT_TRUE - AUQ was rewind since last call
*                                      GT_FALSE - AUQ wasn't rewind since last call
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbQueueRewindStatusGet
(
    IN  GT_U8                          devNum,
    OUT GT_BOOL                        *rewindPtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgFdbQueueRewindStatusGet(devNum, rewindPtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);

    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgEportToPhysicalPortTargetMappingTableSet function
* @endinternal
*
* @brief   Set physical info for the given ePort in the E2PHY mapping table.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] physicalInfoPtr          - (pointer to) physical Information related to the ePort
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note E2PHY table is accessed by the packet's target ePort
*
*/
GT_STATUS prvTgfBrgEportToPhysicalPortTargetMappingTableSet
(
    IN GT_U8                            devNum,
    IN GT_PORT_NUM                      portNum,
    IN CPSS_INTERFACE_INFO_STC          *physicalInfoPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    if(physicalInfoPtr->type == CPSS_INTERFACE_PORT_E)
    {
        rc = prvUtfHwDeviceNumberGet((GT_U8)physicalInfoPtr->devPort.hwDevNum,
                                     &(physicalInfoPtr->devPort.hwDevNum));
        if(GT_OK != rc)
        {
            return rc;
        }

        if(GT_FALSE == prvUtfIsDoublePhysicalPortsModeUsed())
        {
            /* when tests trying to restore value that got from the 'get' ,
               in GM may come with 256..511 */
            physicalInfoPtr->devPort.portNum &= 0xFF;CPSS_TBD_BOOKMARK_BOBCAT3
        }
        else
        {
            /* the CPSS initialized all 512 entries of the device */
        }
    }
    /* call device specific API */
    rc = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum, portNum, physicalInfoPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgEportToPhysicalPortTargetMappingTableSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(physicalInfoPtr);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfBrgEportToPhysicalPortTargetMappingTableGet function
* @endinternal
*
* @brief   Get physical info for the given ePort in the E2PHY mapping table.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] physicalInfoPtr          - (pointer to) physical Information related to the ePort
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note E2PHY table is accessed by the packet's target ePort
*
*/
GT_STATUS prvTgfBrgEportToPhysicalPortTargetMappingTableGet
(
    IN  GT_U8                            devNum,
    IN  GT_PORT_NUM                      portNum,
    OUT CPSS_INTERFACE_INFO_STC          *physicalInfoPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChBrgEportToPhysicalPortTargetMappingTableGet(devNum, portNum, physicalInfoPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgEportToPhysicalPortTargetMappingTableGet FAILED, rc = [%d]", rc);

        return rc;
    }

    if(physicalInfoPtr->type == CPSS_INTERFACE_PORT_E)
    {
        rc = prvUtfSwFromHwDeviceNumberGet(physicalInfoPtr->devPort.hwDevNum,
                                           &(physicalInfoPtr->devPort.hwDevNum));
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(physicalInfoPtr);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}


/**
* @internal prvTgfBrgEportToPhysicalPortEgressPortInfoSet function
* @endinternal
*
* @brief   Set physical info for the given ePort in the HA Egress port table.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] egressInfoPtr            - (pointer to) physical information related to the egress port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgEportToPhysicalPortEgressPortInfoSet
(
    IN GT_U8                                devNum,
    IN GT_PORT_NUM                          portNum,
    IN PRV_TGF_BRG_EGRESS_PORT_INFO_STC     *egressInfoPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS                           rc = GT_OK;
    CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC  dxChEgressInfo;

    dxChEgressInfo.tunnelStart            = egressInfoPtr->tunnelStart;
    if (egressInfoPtr->tunnelStart == GT_TRUE)
    {
        dxChEgressInfo.tunnelStartPtr = egressInfoPtr->tunnelStartPtr;
        switch (egressInfoPtr->tsPassengerPacketType)
        {
            case PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E:
                dxChEgressInfo.tsPassengerPacketType = CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E;
                break;
            case PRV_TGF_TUNNEL_PASSENGER_OTHER_E:
                dxChEgressInfo.tsPassengerPacketType = CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E;
                break;
            default:
                return GT_BAD_PARAM;
        }
    }
    else /* (egressInfoPtr->tunnelStart == GT_FALSE) */
    {
        dxChEgressInfo.arpPtr      = egressInfoPtr->arpPtr;
        dxChEgressInfo.modifyMacSa = egressInfoPtr->modifyMacSa;
        dxChEgressInfo.modifyMacDa = egressInfoPtr->modifyMacDa;
    }

    /* call device specific API */
    rc = cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(devNum,portNum,&dxChEgressInfo);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgEportToPhysicalPortEgressPortInfoSet FAILED, rc = [%d]", rc);
        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(egressInfoPtr);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}


/**
* @internal prvTgfBrgEportToPhysicalPortEgressPortInfoGet function
* @endinternal
*
* @brief   Get physical info for the given ePort in the HA Egress port table.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] egressInfoPtr            - (pointer to) physical Information related to the egress port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgEportToPhysicalPortEgressPortInfoGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    OUT PRV_TGF_BRG_EGRESS_PORT_INFO_STC    *egressInfoPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS                           rc = GT_OK;
    CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC  dxChEgressInfo;

    /* call device specific API */
    rc = cpssDxChBrgEportToPhysicalPortEgressPortInfoGet(devNum,portNum,&dxChEgressInfo);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgEportToPhysicalPortEgressPortInfoGet FAILED, rc = [%d]", rc);
        return rc;
    }

    egressInfoPtr->tunnelStart = dxChEgressInfo.tunnelStart;
    if (egressInfoPtr->tunnelStart == GT_TRUE)
    {
        egressInfoPtr->tunnelStartPtr = dxChEgressInfo.tunnelStartPtr;
        switch (dxChEgressInfo.tsPassengerPacketType)
        {
            case CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E:
                egressInfoPtr->tsPassengerPacketType = PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E;
                break;
            case CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E:
                egressInfoPtr->tsPassengerPacketType = PRV_TGF_TUNNEL_PASSENGER_OTHER_E;
                break;
            default:
                return GT_BAD_STATE;
        }
    }
    else /* (egressInfoPtr->tunnelStart == GT_FALSE) */
    {
        egressInfoPtr->arpPtr      = dxChEgressInfo.arpPtr;
        egressInfoPtr->modifyMacSa = dxChEgressInfo.modifyMacSa;
        egressInfoPtr->modifyMacDa = dxChEgressInfo.modifyMacDa;
    }
    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(egressInfoPtr);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfBrgVlanEgressPortTagStateModeSet function
* @endinternal
*
* @brief   Set the egress port tag state mode.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] stateMode                - state mode: eVLAN or ePort
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanEgressPortTagStateModeSet
(
    IN GT_U8                                            devNum,
    IN GT_PORT_NUM                                      portNum,
    IN PRV_TGF_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT        stateMode
)
{
#ifdef CHX_FAMILY
    GT_STATUS                                       rc = GT_OK;
    CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT     dxChStateMode;

    switch (stateMode)
    {
        case PRV_TGF_BRG_VLAN_EPORT_TAG_STATE_EVLAN_MODE_E:
            dxChStateMode = CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_EVLAN_MODE_E;
            break;
        case PRV_TGF_BRG_VLAN_EPORT_TAG_STATE_EPORT_MODE_E:
            dxChStateMode = CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_EPORT_MODE_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    /* call device specific API */
    rc = cpssDxChBrgVlanEgressPortTagStateModeSet(devNum,portNum,dxChStateMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgVlanEgressPortTagStateModeSet FAILED, rc = [%d]", rc);
        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(stateMode);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfBrgVlanEgressPortTagStateModeGet function
* @endinternal
*
* @brief   Get the egress port tag state mode.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] stateModePtr             - (pointer to) state mode: eVLAN or ePort
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanEgressPortTagStateModeGet
(
    IN  GT_U8                                               devNum,
    IN  GT_PORT_NUM                                         portNum,
    OUT PRV_TGF_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT           *stateModePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS                                       rc = GT_OK;
    CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT     dxChStateMode;

    /* call device specific API */
    rc = cpssDxChBrgVlanEgressPortTagStateModeGet(devNum,portNum,&dxChStateMode);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgVlanEgressPortTagStateModeGet FAILED, rc = [%d]", rc);
        return rc;
    }

    switch (dxChStateMode)
    {
        case CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_EVLAN_MODE_E:
            *stateModePtr = PRV_TGF_BRG_VLAN_EPORT_TAG_STATE_EVLAN_MODE_E;
            break;
        case CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_EPORT_MODE_E:
            *stateModePtr = PRV_TGF_BRG_VLAN_EPORT_TAG_STATE_EPORT_MODE_E;
            break;
        default:
            return GT_BAD_STATE;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(stateModePtr);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfBrgVlanEgressPortTagStateSet function
* @endinternal
*
* @brief   Determines the egress port state.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] tagCmd                   - Port tag state
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanEgressPortTagStateSet
(
    IN GT_U8                                            devNum,
    IN GT_PORT_NUM                                      portNum,
    IN PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT                tagCmd
)
{
#ifdef CHX_FAMILY
    GT_STATUS                           rc = GT_OK;
    CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT dxChTagCmd;

    switch (tagCmd)
    {
        case PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E:
            dxChTagCmd = CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E;
            break;
        case PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E:
            dxChTagCmd = CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E;
            break;
        case PRV_TGF_BRG_VLAN_PORT_TAG1_CMD_E:
            dxChTagCmd = CPSS_DXCH_BRG_VLAN_PORT_TAG1_CMD_E;
            break;
        case PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E:
            dxChTagCmd = CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E;
            break;
        case PRV_TGF_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E:
            dxChTagCmd = CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E;
            break;
        case PRV_TGF_BRG_VLAN_PORT_PUSH_TAG0_CMD_E:
            dxChTagCmd = CPSS_DXCH_BRG_VLAN_PORT_PUSH_TAG0_CMD_E;
            break;
        case PRV_TGF_BRG_VLAN_PORT_POP_OUTER_TAG_CMD_E:
            dxChTagCmd = CPSS_DXCH_BRG_VLAN_PORT_POP_OUTER_TAG_CMD_E;
            break;
        case PRV_TGF_BRG_VLAN_PORT_DO_NOT_MODIFY_TAG_CMD_E:
            dxChTagCmd = CPSS_DXCH_BRG_VLAN_PORT_DO_NOT_MODIFY_TAG_CMD_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    /* call device specific API */
    rc = cpssDxChBrgVlanEgressPortTagStateSet(devNum,portNum,dxChTagCmd);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgVlanEgressPortTagStateSet FAILED, rc = [%d]", rc);
        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(tagCmd);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfBrgVlanEgressPortTagStateGet function
* @endinternal
*
* @brief   Returns the egress port state.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] tagCmdPtr                - (pointer to) Port tag state
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanEgressPortTagStateGet
(
    IN  GT_U8                                               devNum,
    IN  GT_PORT_NUM                                         portNum,
    OUT PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT                   *tagCmdPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS                           rc = GT_OK;
    CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT dxChTagCmd;

    /* call device specific API */
    rc = cpssDxChBrgVlanEgressPortTagStateGet(devNum,portNum,&dxChTagCmd);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgVlanEgressPortTagStateGet FAILED, rc = [%d]", rc);
        return rc;
    }

    switch (dxChTagCmd)
    {
        case CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E:
            *tagCmdPtr = PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E;
            break;
        case CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E:
            *tagCmdPtr = PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E;
            break;
        case CPSS_DXCH_BRG_VLAN_PORT_TAG1_CMD_E:
            *tagCmdPtr = PRV_TGF_BRG_VLAN_PORT_TAG1_CMD_E;
            break;
        case CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E:
            *tagCmdPtr = PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E;
            break;
        case CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E:
            *tagCmdPtr = PRV_TGF_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E;
            break;
        case CPSS_DXCH_BRG_VLAN_PORT_PUSH_TAG0_CMD_E:
            *tagCmdPtr = PRV_TGF_BRG_VLAN_PORT_PUSH_TAG0_CMD_E;
            break;
        case CPSS_DXCH_BRG_VLAN_PORT_POP_OUTER_TAG_CMD_E:
            *tagCmdPtr = PRV_TGF_BRG_VLAN_PORT_POP_OUTER_TAG_CMD_E;
            break;
        case CPSS_DXCH_BRG_VLAN_PORT_DO_NOT_MODIFY_TAG_CMD_E:
            *tagCmdPtr = PRV_TGF_BRG_VLAN_PORT_DO_NOT_MODIFY_TAG_CMD_E;
            break;
        default:
            return GT_BAD_STATE;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(tagCmdPtr);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfBrgGenDropIpMcEnable function
* @endinternal
*
* @brief   Discard all non-Control-classified Ethernet packets
*         with a MAC Multicast DA corresponding to the IP Multicast range,
*         i.e. the MAC range 01-00-5e-00-00-00 to 01-00-5e-7f-ff-ff.
* @param[in] state                    - GT_TRUE, enable filtering mode
*                                      GT_FALSE, disable filtering mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgGenDropIpMcEnable
(
    IN GT_BOOL  state
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

#ifdef CHX_FAMILY

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgGenDropIpMcEnable(devNum, state);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgGenDropIpMcEnable FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmBrgGenDropIpMcEnableSet(devNum, state);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgGenDropIpMcEnable FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

    return rc1;
}

/**
* @internal prvTgfBrgVlanPortAccFrameTypeSet function
* @endinternal
*
* @brief   Set port access frame type.
*         There are three types of port configuration:
*         - Admit only Vlan tagged frames
*         - Admit only tagged and priority tagged,
*         - Admit all frames.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] frameType                -
*                                      CPSS_PORT_ACCEPT_FRAME_TAGGED_E    -
*                                      All Untagged/Priority Tagged packets received on
*                                      this port are discarded. Only Tagged accepted.
*                                      CPSS_PORT_ACCEPT_FRAME_ALL_E       -
*                                      Both Tagged and Untagged packets are accepted
*                                      on the port.
*                                      CPSS_PORT_ACCEPT_FRAME_UNTAGGED_E  -
*                                      All Tagged packets received on this port are
*                                      discarded. Only Untagged/Priority Tagged accepted.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanPortAccFrameTypeSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  CPSS_PORT_ACCEPT_FRAME_TYPE_ENT     frameType
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgVlanPortAccFrameTypeSet(devNum, portNum, frameType);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmBrgVlanPortAcceptFrameTypeSet(devNum, (GT_U8) portNum, frameType);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgVlanPortProtocolVidEnableSet function
* @endinternal
*
* @brief   Enable/Disable Port Protocol VLAN assignment to packets received
*         on this port,according to it's Layer2 protocol
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE - Enable Port protocol VID assignment.
*                                      GT_FALSE - Disable Port protocol VID assignment
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanPortProtocolVidEnableSet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgVlanPortProtoClassVlanEnable(devNum, portNum, enable);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmBrgVlanPortProtocolVidEnableSet(devNum, (GT_U8) portNum, enable);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgVlanPortProtoClassQosEnable function
* @endinternal
*
* @brief   Enable/Disable Port Protocol QoS Assignment to packets received on this
*         port, according to it's Layer2 protocol
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE for enable, otherwise GT_FALSE.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanPortProtoClassQosEnable
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
)
{
#ifdef CHX_FAMILY
    PRV_UTF_LOG3_MAC("prvTgfBrgVlanPortProtoClassQosEnable : (%d,%d,%d) =======\n" ,
                        devNum ,
                        portNum,
                        enable);
    /* call device specific API */
    return cpssDxChBrgVlanPortProtoClassQosEnable(devNum, portNum, enable);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmBrgVlanPortProtocolQosEnableSet(devNum, (GT_U8) portNum, enable);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}


/**
* @internal prvTgfBrgVlanPortProtoVlanQosSet function
* @endinternal
*
* @brief   Write Port Protocol VID and QOS Assignment values for specific device
*         and port.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] entryNum                 - entry number
* @param[in] vlanCfgPtr               - pointer to VID parameters
* @param[in] qosCfgPtr                - pointer to QoS parameters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, port or vlanId
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanPortProtoVlanQosSet
(
    IN GT_U8                                   devNum,
    IN GT_PORT_NUM                             portNum,
    IN GT_U32                                  entryNum,
    IN PRV_TGF_BRG_VLAN_PROT_CLASS_CFG_STC    *vlanCfgPtr,
    IN PRV_TGF_BRG_QOS_PROT_CLASS_CFG_STC     *qosCfgPtr
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_BRG_VLAN_PROT_CLASS_CFG_STC    dxChVlanCfg = {0};
    CPSS_DXCH_BRG_QOS_PROT_CLASS_CFG_STC     dxChQosCfg  = {0};

    if(vlanCfgPtr)
    {
        /* convert vlanCfg into device specific format */
        dxChVlanCfg.vlanId = vlanCfgPtr->vlanId;
        dxChVlanCfg.vlanIdAssignCmd = vlanCfgPtr->vlanIdAssignCmd;
        dxChVlanCfg.vlanIdAssignPrecedence = vlanCfgPtr->vlanIdAssignPrecedence;
    }

    if(qosCfgPtr)
    {
        /* convert qosCfg into device specific format */
        dxChQosCfg.qosAssignCmd = qosCfgPtr->qosAssignCmd;
        dxChQosCfg.qosProfileId = qosCfgPtr->qosProfileId;
        dxChQosCfg.qosAssignPrecedence = qosCfgPtr->qosPrecedence;
        dxChQosCfg.enableModifyUp = qosCfgPtr->qosParamsModify.modifyUp;
        dxChQosCfg.enableModifyDscp = qosCfgPtr->qosParamsModify.modifyDscp;
    }

    /* call device specific API */
    return cpssDxChBrgVlanPortProtoVlanQosSet(devNum,  portNum, entryNum,
        vlanCfgPtr ? &dxChVlanCfg : NULL , qosCfgPtr ? &dxChQosCfg : NULL);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
        CPSS_EXMXPM_BRG_VLAN_PROT_CLASS_CFG_STC                exMxPmVlanCfg = {0};
    CPSS_EXMXPM_QOS_ENTRY_STC                                        exMxPmQosCfg  = {{0}};

    if(vlanCfgPtr)
    {
        /* convert vlanCfg into device specific format */
        exMxPmVlanCfg.vlanId = vlanCfgPtr->vlanId;
        exMxPmVlanCfg.vlanIdAssignCmd = vlanCfgPtr->vlanIdAssignCmd;
        exMxPmVlanCfg.vlanIdAssignPrecedence = vlanCfgPtr->vlanIdAssignPrecedence;
    }

    if(qosCfgPtr)
    {
        /* convert qosCfg into device specific format */
        exMxPmQosCfg.qosParams.dp = qosCfgPtr->qosParams.dp;
        exMxPmQosCfg.qosParams.dscp = qosCfgPtr->qosParams.dscp;
        exMxPmQosCfg.qosParams.exp = qosCfgPtr->qosParams.exp;
        exMxPmQosCfg.qosParams.tc = qosCfgPtr->qosParams.tc;
        exMxPmQosCfg.qosParams.up = qosCfgPtr->qosParams.up;

        exMxPmQosCfg.qosParamsModify.modifyDp = qosCfgPtr->qosParamsModify.modifyDp;
        exMxPmQosCfg.qosParamsModify.modifyDscp = qosCfgPtr->qosParamsModify.modifyDscp;
        exMxPmQosCfg.qosParamsModify.modifyExp = qosCfgPtr->qosParamsModify.modifyExp;
        exMxPmQosCfg.qosParamsModify.modifyTc = qosCfgPtr->qosParamsModify.modifyTc;
        exMxPmQosCfg.qosParamsModify.modifyUp = qosCfgPtr->qosParamsModify.modifyUp;

        exMxPmQosCfg.qosPrecedence = qosCfgPtr->qosPrecedence;
    }

    /* call device specific API */
    return cpssExMxPmBrgVlanPortProtocolVidQosEntrySet(devNum, (GT_U8) portNum, entryNum,
        vlanCfgPtr ? &exMxPmVlanCfg : NULL , qosCfgPtr ? &exMxPmQosCfg : NULL);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgVlanProtocolEntrySet function
* @endinternal
*
* @brief   Write etherType and encapsulation of Protocol based classification
*         for specific device.
* @param[in] index                    - entry number
* @param[in] etherType                - Ether Type or DSAP/SSAP
* @param[in] encListPtr               - encapsulation.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanProtocolEntrySet
(
    IN GT_U32                           index,
    IN GT_U16                           etherType,
    IN CPSS_PROT_CLASS_ENCAP_STC        *encListPtr
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

#ifdef CHX_FAMILY

    PRV_UTF_LOG5_MAC("prvTgfBrgVlanProtocolEntrySet : (%d,0x%x,%d,%d,%d) =======\n" ,
                        index ,
                        etherType,
                        encListPtr->ethV2,
                        encListPtr->nonLlcSnap,
                        encListPtr->llcSnap);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgVlanProtoClassSet(devNum, index, etherType, encListPtr);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgVlanProtoClassSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmBrgVlanProtocolEntrySet(devNum, index, etherType, encListPtr);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgVlanProtocolEntrySet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

    return rc1;
}

/**
* @internal prvTgfBrgVlanPortProtoVlanQosInvalidate function
* @endinternal
*
* @brief   Invalidate protocol based QoS and VLAN assignment entry for
*         specific port
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] entryNum                 - entry number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanPortProtoVlanQosInvalidate
(
    IN GT_U8                    devNum,
    IN GT_PORT_NUM              portNum,
    IN GT_U32                   entryNum
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgVlanPortProtoVlanQosInvalidate(devNum, portNum, entryNum);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmBrgVlanPortProtocolVidQosEntryInvalidate(devNum, (GT_U8) portNum, entryNum);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgVlanProtocolEntryInvalidate function
* @endinternal
*
* @brief   Invalidates entry of Protocol based classification for specific device.
*
* @param[in] entryNum                 - entry number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanProtocolEntryInvalidate
(
    IN GT_U32                           entryNum
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

#ifdef CHX_FAMILY

    PRV_UTF_LOG1_MAC("prvTgfBrgVlanProtocolEntryInvalidate : (%d) =======\n" ,
                        entryNum);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgVlanProtoClassInvalidate(devNum, entryNum);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgVlanProtoClassInvalidate FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmBrgVlanProtocolEntryInvalidate(devNum, entryNum);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgVlanProtocolEntryInvalidate FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

    return rc1;
}

/**
* @internal prvTgfBrgFdbLsrEnableSet function
* @endinternal
*
* @brief   Enable/Disable the FDB Learning, Scanning and Refresh operations.
*         Halting of these operation allows the CPU un-interrupted access to
*         the FDB.
* @param[in] enable                   - GT_TRUE = Learning, Scanning and Refresh operations do
*                                      their regular work
*                                      GT_FALSE = Learning, Scanning and Refresh operations halt
*                                      to allow the CPU un-interrupted access to the FDB
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbLsrEnableSet
(
    IN GT_BOOL       enable
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
#ifdef CHX_FAMILY

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgFdbActionsEnableSet(devNum, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbActionsEnableSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmBrgFdbLsrEnableSet(devNum, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgFdbLsrEnableSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

    return rc1;
}

/**
* @internal prvTgfBrgFdbMacVlanLookupModeSet function
* @endinternal
*
* @brief   Sets the VLAN Lookup mode.
*
* @param[in] mode                     - lookup mode:
*                                      CPSS_IVL_E - Both MAC and Vlan are used for MAC table lookup
*                                      and learning.
*                                      CPSS_SVL_E - Only MAC is used for MAC table lookup and learning.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad param
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbMacVlanLookupModeSet
(
        IN CPSS_MAC_VL_ENT      mode
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
#ifdef CHX_FAMILY

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgFdbMacVlanLookupModeSet(devNum, mode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbMacVlanLookupModeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmBrgFdbMacVlanLookupModeSet(devNum, mode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgFdbMacVlanLookupModeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

    return rc1;
}

/**
* @internal prvTgfBrgFdbMacVlanLookupModeGet function
* @endinternal
*
* @brief   Get VLAN Lookup mode.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - pointer to lookup mode:
*                                      CPSS_IVL_E - Both MAC and Vlan are used for MAC table lookup
*                                      and learning.
*                                      CPSS_SVL_E - Only MAC is used for MAC table lookup and learning.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad param
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbMacVlanLookupModeGet
(
    IN  GT_U8            devNum,
    OUT CPSS_MAC_VL_ENT  *modePtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgFdbMacVlanLookupModeGet(devNum, modePtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmBrgFdbMacVlanLookupModeGet(devNum, modePtr);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgVlanEgressEtherTypeSet function
* @endinternal
*
* @brief   Sets egress VLAN Ether Type
*
* @param[in] etherType                - Mode of tagged vlan etherType.
*                                      May be either:CPSS_VLAN_ETHERTYPE0_E or
*                                      CPSS_VLAN_ETHERTYPE1_E By default only value 0 would
*                                      be used, unless nested VLANs are used.
*                                      See comment.
* @param[in] etherType                - the EtherType to be recognized tagged packets. See
*                                      comment.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanEgressEtherTypeSet
(
    IN  CPSS_ETHER_MODE_ENT     etherTypeMode,
    IN  GT_U16                  etherType
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

#ifdef CHX_FAMILY
    GT_U32      entryIndex = 0;

    switch (etherTypeMode)
    {
        case CPSS_VLAN_ETHERTYPE0_E:
            entryIndex = 0;
            break;
        case CPSS_VLAN_ETHERTYPE1_E:
            entryIndex = 1;
            break;
        default:
            return GT_BAD_PARAM;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgVlanTpidEntrySet(devNum, CPSS_DIRECTION_EGRESS_E, entryIndex, etherType);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgVlanTpidEntrySet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmBrgVlanEgressEtherTypeSet(devNum, etherTypeMode, etherType);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgVlanEgressEtherTypeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

    return rc1;
}

/**
* @internal prvTgfBrgVlanEgressEtherTypeGet function
* @endinternal
*
* @brief   Gets egress VLAN Ether Type
*
* @param[in] devNum                   - device number
* @param[in] etherTypeMode            - Mode of tagged vlan etherType.
*                                      May be either:CPSS_VLAN_ETHERTYPE0_E or
*                                      CPSS_VLAN_ETHERTYPE1_E By default only value 0 would
*                                      be used, unless nested VLANs are used.
*                                      See comment.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanEgressEtherTypeGet
(
    IN  GT_U8                   devNum,
    IN  CPSS_ETHER_MODE_ENT     etherTypeMode,
    IN  GT_U16                  *etherTypePtr
)
{
#ifdef CHX_FAMILY
    GT_U32      entryIndex = 0;

    switch (etherTypeMode)
    {
        case CPSS_VLAN_ETHERTYPE0_E:
            entryIndex = 0;
            break;
        case CPSS_VLAN_ETHERTYPE1_E:
            entryIndex = 1;
            break;
        default:
            return GT_BAD_PARAM;
    }

    /* call device specific API */
    return cpssDxChBrgVlanTpidEntryGet(devNum, CPSS_DIRECTION_EGRESS_E, entryIndex, etherTypePtr);

#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
   /* call device specific API */
    return cpssExMxPmBrgVlanEgressEtherTypeGet(devNum, etherTypeMode, etherTypePtr);

#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgNestVlanPortEgressEtherTypeSelectSet function
* @endinternal
*
* @brief   Sets selected VLAN EtherType for tagged packets transmitted
*         via this egress port.
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, or CPU port
* @param[in] egressVlanSel            - selected port VLAN EtherType.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or portNum or ingressVlanSel values.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgNestVlanPortEgressEtherTypeSelectSet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                   portNum,
    IN  CPSS_ETHER_MODE_ENT     egressVlanSel
)
{
#ifdef CHX_FAMILY
    GT_U32    entryIndex = 0;
#endif /* CHX_FAMILY */

#ifdef CHX_FAMILY
    switch (egressVlanSel)
    {
        case CPSS_VLAN_ETHERTYPE0_E:
            entryIndex = 0;
            break;
        case CPSS_VLAN_ETHERTYPE1_E:
            entryIndex = 1;
            break;
        default:
            return GT_BAD_PARAM;
    }
    /* call device specific API */
    return cpssDxChBrgVlanPortEgressTpidSet(devNum, portNum, egressVlanSel, entryIndex);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmBrgNestVlanPortEgressEtherTypeSelectSet(devNum, portNum, egressVlanSel);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgNestVlanAccessPortEnableSet function
* @endinternal
*
* @brief   Configure given port as nested VLAN access port.
*         The VID of all the packets received on a nested VLAN access port is
*         discarded and they are assigned with the Port VID.
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, or CPU port
* @param[in] enable                   -  GT_TRUE  - port is nested VLAN Access Port.
*                                      GT_FALSE - port is Core or Customer port.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgNestVlanAccessPortEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_U32       portNum,
    IN  GT_BOOL     enable
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgNestVlanAccessPortSet(devNum, portNum, enable);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmBrgNestVlanAccessPortEnableSet(devNum, portNum, enable);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgFdbActionTransplantDataSet function
* @endinternal
*
* @brief   Prepares the entry for transplanting (old and new interface parameters).
*         VLAN and VLAN mask for transplanting is set by
*         cpssDxChBrgFdbActionActiveVlanSet. Execution of transplanting is done
*         by cpssDxChBrgFdbTrigActionStart.
* @param[in] oldInterfacePtr          - pointer to old Interface parameters
*                                      (it may be only port or trunk).
* @param[in] newInterfacePtr          - pointer to new Interface parameters
*                                      (it may be only port or trunk).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note There is no check done of current status of action's register
*
*/
GT_STATUS prvTgfBrgFdbActionTransplantDataSet
(
    IN CPSS_INTERFACE_INFO_STC  *oldInterfacePtr,
    IN CPSS_INTERFACE_INFO_STC  *newInterfacePtr
)
{
#ifdef CHX_FAMILY

    GT_STATUS   rc1 = GT_OK;
    GT_STATUS   rc  = GT_OK;
    GT_U8       devNum  = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgFdbActionTransplantDataSet(devNum, oldInterfacePtr, newInterfacePtr);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbActionTransplantDataSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
    return rc1;
#else
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgFdbActionTransplantDataGet function
* @endinternal
*
* @brief   Get transplant data: old interface parameters and the new ones.
*
* @param[in] devNum                   - device number
*
* @param[out] oldInterfacePtr          - pointer to old Interface parameters.
*                                      (it may be only port or trunk).
* @param[out] newInterfacePtr          - pointer to new Interface parameters.
*                                      (it may be only port or trunk).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbActionTransplantDataGet
(
    IN  GT_U8                    devNum,
    OUT CPSS_INTERFACE_INFO_STC  *oldInterfacePtr,
    OUT CPSS_INTERFACE_INFO_STC  *newInterfacePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    /* call device specific API */
    rc = cpssDxChBrgFdbActionTransplantDataGet(devNum, oldInterfacePtr, newInterfacePtr);
    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */

}
/**
* @internal prvTgfBrgFdbActionActiveVlanSet function
* @endinternal
*
* @brief   Set action active vlan and vlan mask.
*         All actions will be taken on entries belonging to
*         a certain VLAN or a subset of VLANs.
* @param[in] vlanId                   - vlan Id
* @param[in] vlanMask                 - vlan mask filter
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbActionActiveVlanSet
(
    IN GT_U32   vlanId,
    IN GT_U32   vlanMask
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

#ifdef CHX_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgFdbActionActiveVlanSet(devNum, (GT_U16)vlanId, (GT_U16)vlanMask);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbActionActiveVlanSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmBrgFdbActionActiveFidSet(devNum, vlanId, vlanMask);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgFdbActionActiveFidSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

    return rc1;
}

/**
* @internal prvTgfBrgFdbActionActiveVlanGet function
* @endinternal
*
* @brief   Get action active vlan and vlan mask.
*         All actions are taken on entries belonging to a certain VLAN
*         or a subset of VLANs.
* @param[in] devNum                   - device number
*
* @param[out] vlanIdPtr                - pointer to vlan Id
* @param[out] vlanMaskPtr              - pointer to vlan mask filter
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbActionActiveVlanGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *vlanIdPtr,
    OUT GT_U32   *vlanMaskPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    GT_U16    vlan;
    GT_U16    vlanMask;

    /* call device specific API */
    rc = cpssDxChBrgFdbActionActiveVlanGet(devNum, &vlan, &vlanMask);
    *vlanIdPtr = vlan;
    *vlanMaskPtr = vlanMask;
    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmBrgFdbActionActiveFidGet(devNum, vlanIdPtr, vlanMaskPtr);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgFdbStaticTransEnableSet function
* @endinternal
*
* @brief   This routine determines whether the transplanting operate on static
*         entries.
*         When the PP will do the transplanting , it will/won't transplant
*         FDB static entries (entries that are not subject to aging).
*         When the PP will do the "port flush" (delete FDB entries associated
*         with to specific port), it will/won't Flush the FDB static entries
*         (entries that are not subject to aging).
* @param[in] enable                   - GT_TRUE transplanting is enabled on static entries,
*                                      GT_FALSE otherwise.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbStaticTransEnableSet
(
    IN GT_BOOL  enable
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

#ifdef CHX_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgFdbStaticTransEnable(devNum, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbStaticDelEnable FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmBrgFdbStaticTransEnableSet(devNum, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgFdbStaticDelEnableSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

    return rc1;
}

/**
* @internal prvTgfBrgFdbStaticTransEnableGet function
* @endinternal
*
* @brief   Get whether the transplanting enabled to operate on static entries.
*         when the PP do the transplanting , it do/don't transplant
*         FDB static entries (entries that are not subject to aging).
*         When the PP will do the "port flush" (delete FDB entries associated
*         with to specific port), it do/don't Flush the FDB static entries
*         (entries that are not subject to aging).
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - GT_TRUE transplanting is enabled on static entries,
*                                      GT_FALSE otherwise.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbStaticTransEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgFdbStaticTransEnableGet(devNum, enablePtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmBrgFdbStaticTransEnableGet(devNum, enablePtr);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */


}

/**
* @internal prvTgfBrgFdbStaticDelEnableSet function
* @endinternal
*
* @brief   This routine determines whether flush delete operates on static entries.
*         When the PP will do the Flush , it will/won't Flush
*         FDB static entries (entries that are not subject to aging).
* @param[in] enable                   - GT_TRUE delete is enabled on static entries, GT_FALSE otherwise.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbStaticDelEnableSet
(
    IN GT_BOOL  enable
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

#ifdef CHX_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgFdbStaticDelEnable(devNum, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbStaticDelEnable FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmBrgFdbStaticDelEnableSet(devNum, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgFdbStaticDelEnableSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

    return rc1;
}

/**
* @internal prvTgfBrgFdbStaticDelEnableGet function
* @endinternal
*
* @brief   Get whether flush delete operates on static entries.
*         When the PP do the Flush , it do/don't Flush
*         FDB static entries (entries that are not subject to aging).
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - GT_TRUE delete is enabled on static entries,
*                                      GT_FALSE otherwise.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbStaticDelEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    CPSS_DXCH_BRG_FDB_DEL_MODE_ENT mode = CPSS_DXCH_BRG_FDB_DEL_MODE_DYNAMIC_ONLY_E;
    GT_STATUS rc = cpssDxChBrgFdbStaticDelEnableGet(devNum, &mode);
    if (rc == GT_OK)
    {
        switch (mode)
        {
            case CPSS_DXCH_BRG_FDB_DEL_MODE_DYNAMIC_ONLY_E:
                *enablePtr = GT_FALSE;
                break;
            case CPSS_DXCH_BRG_FDB_DEL_MODE_ALL_E:
                *enablePtr = GT_TRUE;
                break;
            case CPSS_DXCH_BRG_FDB_DEL_MODE_STATIC_ONLY_E:
                return GT_FAIL;
                break;
        }
    }
    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmBrgFdbStaticDelEnableGet(devNum, enablePtr);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgFdbQueueFullGet function
* @endinternal
*
* @brief   The function returns the state that the AUQ/FUQ - is full/not full
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - the device number
* @param[in] queueType                - queue type AU/FU queue
*                                      NOTE: when using shared queue for AU and FU messages, this
*                                      parameter is ignored (and the AUQ is queried)
*                                      meaning that this parameter always ignored for DxCh1
*                                      devices.
*
* @param[out] isFullPtr                - (pointer to) is the relevant queue full
*                                      GT_TRUE  - the queue is full
*                                      GT_FALSE - the queue is not full
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum , queueType
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note for multi-port groups device :
*       'queue is full' when at least one of the port groups has queue full.
*
*/
GT_STATUS prvTgfBrgFdbQueueFullGet
(
    IN GT_U8    devNum,
    IN PRV_TGF_BRG_FDB_QUEUE_TYPE_ENT  queueType,
    OUT GT_BOOL *isFullPtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgFdbQueueFullGet(devNum, (CPSS_DXCH_FDB_QUEUE_TYPE_ENT)queueType, isFullPtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmBrgFdbQueueFullGet(devNum, (CPSS_EXMXPM_FDB_QUEUE_TYPE_ENT)queueType, isFullPtr);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgFdbNaStormPreventAllSet function
* @endinternal
*
* @brief   Enable/Disable New Address messages Storm Prevention.
*         Controlled learning relies on receiving NA messages for new source MAC
*         address. To prevent forwarding multiple NA messages to the CPU for the
*         same source MAC address, when NA Storm Prevention (SP) is enabled, the
*         device auto-learns this address with a special Storm Prevention flag set in
*         the entry. Subsequent packets from this source address do not generate
*         further NA messages to the CPU. Packets destined to this MAC address however
*         are treated as unknown packets. Upon receiving the single NA
*         message, the CPU can then overwrite the SP entry with a normal FDB
*         forwarding entry.
*         Only relevant in controlled address learning mode.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] enable                   - GT_TRUE -  NA Storm Prevention (SP),
*                                      GT_FALSE - disable NA Storm Prevention
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum/portNum/auMsgType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note for multi-port groups device :
*       Enabling this mechanism in any of the ports, creates a situation where the
*       different portGroups populate different MACs at the same indexes,
*       causing FDBs to become unsynchronized, this leads to the situation that once
*       sending MAC update to different portGroups from CPU, it may succeed on some
*       of the portGroups, while failing on the others (due to SP entries already
*       occupying indexes)... So the only way to bypass these problems, assuming SP
*       is a must, is to add new MACs by exact index - this mode of operation
*       overwrites any SP entry currently there.
*       Application that enables the SP in any of the ports should not use the NA
*       sending mechanism (cpssDxChBrgFdbMacEntrySet / cpssDxChBrgFdbPortGroupMacEntrySet APIs)
*       to add new MAC addresses, especially to multiple portGroups -
*       as this may fail on some of the portGroups.
*       Removing or updating existing (non-SP) entries may still use the NA messages.
*
*/
GT_STATUS prvTgfBrgFdbNaStormPreventAllSet
(
    IN GT_BOOL          enable
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
    GT_PORT_NUM     portNum;
    GT_U32              ii;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);


    /* support port numbers that are not returned by prvUtfNextVirtPortGet()
       due to 'optimizations' inside prvUtfNextVirtPortGet()*/
    /* make sure that physical port of the tested ports are set properly */
    for(ii = 0 ; ii < prvTgfPortsNum; ii++)
    {
        rc = cpssDxChBrgFdbNaStormPreventSet(prvTgfDevNum,
                prvTgfPortsArray[ii],
                enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbNaStormPreventSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* prepare physical port iterator */
        rc = prvUtfNextVirtPortReset(&portNum, devNum);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvUtfNextVirtPortReset FAILED, rc = [%d]", rc);
        }

        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            rc = cpssDxChBrgFdbNaStormPreventSet(devNum, portNum, enable);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbNaStormPreventSet FAILED, rc = [%d]", rc);
                rc1 = rc;
            }
        }
    }

    return rc1;
#else
    TGF_PARAM_NOT_USED(enable);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgFdbNaStormPreventAllGet function
* @endinternal
*
* @brief   Get status of New Address messages Storm Prevention.
*         Controlled learning relies on receiving NA messages for new source MAC
*         address. To prevent forwarding multiple NA messages to the CPU for the
*         same source MAC address, when NA Storm Prevention (SP) is enabled, the
*         device auto-learns this address with a special Storm Prevention flag set in
*         the entry. Subsequent packets from this source address do not generate
*         further NA messages to the CPU. Packets destined to this MAC address however
*         are treated as unknown packets. Upon receiving the single NA
*         message, the CPU can then overwrite the SP entry with a normal FDB
*         forwarding entry.
*         Only relevant in controlled address learning mode.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
*
* @param[out] enablePtr                - pointer to the status of repeated NA CPU messages
*                                      GT_TRUE - NA Storm Prevention (SP) is enabled,
*                                      GT_FALSE - NA Storm Prevention is disabled
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum/portNum/auMsgType
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbNaStormPreventAllGet
(
    OUT GT_BOOL         *enablePtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
    GT_PORT_NUM     portNum;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* prepare physical port iterator */
        rc = prvUtfNextVirtPortReset(&portNum, devNum);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvUtfNextVirtPortReset FAILED, rc = [%d]", rc);
        }

        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /* call device specific API */
#ifdef EXMXPM_FAMILY
            CPSS_EXMXPM_INLIF_ENTRY_STC inlifInfo;
            rc = cpssExMxPmInlifEntryGet(devNum, CPSS_EXMXPM_INLIF_TYPE_PORT_E,portNum, &inlifInfo);
            *enablePtr = inlifInfo.naStormPreventionEnable;
#else
            rc = cpssDxChBrgFdbNaStormPreventGet(devNum, portNum, enablePtr);
#endif
            rc1 = (GT_OK != rc) ?  rc : rc1;
            break;
        }
    }

    return rc1;
#else
    TGF_PARAM_NOT_USED(enablePtr);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}


/**
* @internal prvTgfBrgFdbForceIgnoreNaStormPrevention function
* @endinternal
*
* @brief   DEBUG function to be called before running first enh-UT.
*         do we 'force' to ignore the SP settings by the tests ?
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] forceToIgnore            - do we 'force' to ignore the SP settings by the tests ?
*
* @retval GT_OK                    - on success,
*/
GT_STATUS prvTgfBrgFdbForceIgnoreNaStormPrevention
(
    IN GT_BOOL forceToIgnore
)
{
    forceIgnoreNaStormPrevention = forceToIgnore;

    return GT_OK;
}


/**
* @internal prvTgfBrgFdbActionActiveDevSet function
* @endinternal
*
* @brief   Set the Active device number and active device number mask , that
*         control the FDB actions of : Trigger aging , transplant , flush ,
*         flush port.
*         A FDB entry will be treated by the FDB action only if the entry's
*         "associated devNum" masked by the "active device number mask" equal to
*         "Active device number"
* @param[in] devNum                   - device number
* @param[in] actDev                   - Action active device (APPLICABLE RANGES: 0..31)
* @param[in] actDevMask               - Action active mask (APPLICABLE RANGES: 0..31)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
*                                       in 'Dual deviceId system' , if the Pattern/mask
*                                       of the LSBit of the actDev,actDevMask are 'exact match = 1'
*                                       because all devices in this system with 'even numbers'
* @retval GT_OUT_OF_RANGE          - actDev > 31 or actDevMask > 31
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note There is no check done of current status of action's register
*       For 'Dual deviceId system':
*       the LSBit on the actDev,acdDevMask are ignored and actually taken from
*       MSBit of actTrunkPort and actTrunkPortMask from prvTgfBrgFdbActionActiveInterfaceSet
*
*/
GT_STATUS prvTgfBrgFdbActionActiveDevSet
(
    IN  GT_U8   devNum,
    IN GT_U32   actDev,
    IN GT_U32   actDevMask
)
{
#ifdef CHX_FAMILY
   return cpssDxChBrgFdbActionActiveDevSet(devNum, actDev, actDevMask);
#else
    TGF_PARAM_NOT_USED(devNum)
    TGF_PARAM_NOT_USED(actDev)
    TGF_PARAM_NOT_USED(actDevMask)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY)*/
}

/**
* @internal prvTgfBrgFdbActionActiveDevGet function
* @endinternal
*
* @brief   Get the Active device number and active device number mask , that
*         control the FDB actions of : Trigger aging , transplant , flush ,
*         flush port.
*         A FDB entry will be treated by the FDB action only if the entry's
*         "associated devNum" masked by the "active device number mask" equal to
*         "Active device number"
* @param[in] devNum                   - device number
*
* @param[out] actDevPtr                - pointer to action active device (APPLICABLE RANGES: 0..31)
* @param[out] actDevMaskPtr            - pointer to action active mask (APPLICABLE RANGES: 0..31)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_OUT_OF_RANGE          - actDev > 31 or actDevMask > 31
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbActionActiveDevGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *actDevPtr,
    OUT GT_U32   *actDevMaskPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    GT_U32    actDev;
    GT_U32    actDevMask;

    /* call device specific API */
    rc = cpssDxChBrgFdbActionActiveDevGet(devNum, &actDev, &actDevMask);
    *actDevPtr = actDev;
    *actDevMaskPtr = actDevMask;
    return rc;
#else
    TGF_PARAM_NOT_USED(devNum)
    TGF_PARAM_NOT_USED(actDevPtr)
    TGF_PARAM_NOT_USED(actDevMaskPtr)

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfBrgFdbActionActiveInterfaceSet function
* @endinternal
*
* @brief   Set the Active trunkId/portNumber active trunk/port mask and
*         trunkID mask that control the FDB actions of :
*         Triggered aging with removal, Triggered aging without removal and
*         Triggered address deleting. A FDB entry will be treated by the
*         FDB action only if the following cases are true:
*         1. The interface is trunk and "active trunk mask" is set and
*         "associated trunkId" masked by the "active trunk/port mask"
*         equal to "Active trunkId".
*         2. The interface is port and "associated portNumber" masked by the
*         "active trunk/port mask" equal to "Active portNumber".
* @param[in] devNum                   - device number
* @param[in] actIsTrunk               - determines if the interface is port or trunk
* @param[in] actIsTrunkMask           - action active trunk mask.
* @param[in] actTrunkPort             - action active interface (port/trunk)
* @param[in] actTrunkPortMask         - action active port/trunk mask.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbActionActiveInterfaceSet
(
    IN GT_U8   devNum,
    IN GT_U32  actIsTrunk,
    IN GT_U32  actIsTrunkMask,
    IN GT_U32  actTrunkPort,
    IN GT_U32  actTrunkPortMask
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgFdbActionActiveInterfaceSet(devNum,
                                                actIsTrunk, actIsTrunkMask,
                                                actTrunkPort, actTrunkPortMask);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_FDB_SCAN_OUTLIF_STC   actOutlif = { 0 };
    CPSS_EXMXPM_FDB_SCAN_OUTLIF_STC   actOutlifMask = { 0 };

    /* map input arguments to locals */
    actOutlif.isTrunk        = (GT_U8) actIsTrunk;
    actOutlif.portTrunk      = (GT_U16) actTrunkPort;

    actOutlifMask.isTrunk    = (GT_U8) actIsTrunkMask;
    actOutlifMask.portTrunk  = (GT_U16) actTrunkPortMask;

    /* call cpss api function */
    return cpssExMxPmBrgFdbActionActiveOutlifSet(devNum, &actOutlif, &actOutlifMask);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgFdbActionActiveInterfaceGet function
* @endinternal
*
* @brief   Get the Active trunkId/portNumber active trunk/port mask and
*         trunkID mask that control the FDB actions of: Triggered aging with
*         removal, Triggered aging without removal and Triggered address deleting.
* @param[in] devNum                   - device number
*
* @param[out] actIsTrunkPtr            - determines if the interface is port or trunk
* @param[out] actIsTrunkMaskPtr        - action active trunk mask.
* @param[out] actTrunkPortPtr          - action active interface (port/trunk)
* @param[out] actTrunkPortMaskPtr      - action active port/trunk mask.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbActionActiveInterfaceGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *actIsTrunkPtr,
    OUT GT_U32  *actIsTrunkMaskPtr,
    OUT GT_U32  *actTrunkPortPtr,
    OUT GT_U32  *actTrunkPortMaskPtr
)
{
    GT_STATUS rc = GT_OK;

#ifdef CHX_FAMILY
    /* call device specific API */
    rc = cpssDxChBrgFdbActionActiveInterfaceGet(devNum,
                                                  actIsTrunkPtr,
                                                  actIsTrunkMaskPtr,
                                                  actTrunkPortPtr,
                                                  actTrunkPortMaskPtr );
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfBrgFdbActionActiveInterfaceGet FAILED, rc = [%d]", rc);
    }
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_FDB_SCAN_OUTLIF_STC actOutlif = { 0 };
    CPSS_EXMXPM_FDB_SCAN_OUTLIF_STC actOutlifMask = { 0 };

    /* call device specific API */
    rc = cpssExMxPmBrgFdbActionActiveOutlifGet(devNum, &actOutlif, &actOutlifMask);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfBrgFdbActionActiveInterfaceGet FAILED, rc = [%d]", rc);
        return rc;
    }

    /* GT_BAD_STATE is returned in case "isMultiple" indication is turned on,
       which was not supported when this API originally existed. In this case
       dummy values will be inserted into the different params. Please refer
       to "cpssExMxPmBrgFdbActionActiveOutlifGet" for more details. */
    if( actOutlif.isMultiple && actOutlifMask.isMultiple )
    {
        actOutlif.isTrunk = 1;
        actOutlifMask.isTrunk = 1;
        actOutlif.portTrunk = 0xFE;
        actOutlifMask.portTrunk = 0xFF;
    }

    /* unpack output arguments to compatible var size */
    *actIsTrunkPtr       = (GT_U32) actOutlif.isTrunk;
    *actIsTrunkMaskPtr   = (GT_U32) actOutlifMask.isTrunk;
    *actTrunkPortPtr     = (GT_U32) actOutlif.portTrunk;
    *actTrunkPortMaskPtr = (GT_U32) actOutlifMask.portTrunk;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    rc = GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

    return rc;
}

/**
* @internal prvTgfBrgSctIeeeReservedMcProtocolCpuCodeSet function
* @endinternal
*
* @brief   This function set the CPU code for specific IEEE reserved multicast
*         protocol at specific profile.
*         Each profile can be configured with 16 standard protocols
*         and 16 GARP protocols.
* @param[in] profileIndex             - index of profile. Can be either 0 or 1.
* @param[in] protocol                 - This param could be :
*                                      1. Standart protocols, that has MAC range
*                                      01-80-C2-00-00-00 to 01-80-C2-00-00-0F.
*                                      (note that the specific protocol expressed
*                                      by  last 4 bits are set by the next parameter)
*                                      2. GARP protocols, that has a MAC range of:
*                                      01-80-C2-00-00-20 to 01-80-C2-00-00-2F.
*                                      (note that the specific protocol expressed
*                                      by  last 4 bits are set by the next parameter)
* @param[in] protocol                 -  specific  expressed by last 4 bits of
*                                      01-80-C2-00-00-0x or 01-80-C2-00-00-2x.
* @param[in] cpuCode                  -  CPU code for packets trapped or mirrored to CPU.
*                                      Can be either:
*                                      CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_E
*                                      CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_1_E
*                                      CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_2_E
*                                      CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_3_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on invalid input paramteres value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgSctIeeeReservedMcProtocolCpuCodeSet
(
    IN GT_U32                                   profileIndex,
    IN PRV_TGF_SCT_IEEE_RSV_MC_PROTOCOL_ENT     protocolType,
    IN GT_U32                                   protocol,
    IN CPSS_NET_RX_CPU_CODE_ENT                 cpuCode
)
{
#ifdef CHX_FAMILY
    TGF_PARAM_NOT_USED(profileIndex);
    TGF_PARAM_NOT_USED(protocolType);
    TGF_PARAM_NOT_USED(protocol);
    TGF_PARAM_NOT_USED(cpuCode);
    return GT_NOT_SUPPORTED;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
    CPSS_EXMXPM_SCT_IEEE_RSV_MC_PROTOCOL_ENT exMxPmProtocolType;


    /* convert protocolType into device specific format */
    switch (protocolType)
    {
        case PRV_TGF_SCT_IEEE_RSV_MC_PROTOCOL_STD_E:
            exMxPmProtocolType = CPSS_EXMXPM_SCT_IEEE_RSV_MC_PROTOCOL_STD_E;
            break;

        case PRV_TGF_SCT_IEEE_RSV_MC_PROTOCOL_GARP_E:
            exMxPmProtocolType = CPSS_EXMXPM_SCT_IEEE_RSV_MC_PROTOCOL_GARP_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmSctIeeeReservedMcProtocolCpuCodeSet(devNum, profileIndex,
                                                         exMxPmProtocolType, protocol, cpuCode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmSctIeeeReservedMcProtocolCpuCodeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgSctIeeeReservedMcProtocolCmdSet function
* @endinternal
*
* @brief   This function set the command for specific IEEE reserved multicast
*         protocol at specific profile. Each profile can be configured with 16
*         standard protocols and 16 GARP protocols.
* @param[in] profileIndex             - index of profile. Can be either 0 or 1.
* @param[in] protocol                 - This param can be :
*                                      1. Standart protocols, that has MAC range
*                                      01-80-C2-00-00-00 to 01-80-C2-00-00-0F.
*                                      2. GARP protocols, that has a MAC range of:
*                                      01-80-C2-00-00-20 to 01-80-C2-00-00-2F.
* @param[in] protocol                 -  specific  expressed by last 4 bits of
*                                      01-80-C2-00-00-0x or 01-80-C2-00-00-2x
* @param[in] cmd                      -  specify command to process packet with a MAC destination
*                                      Multicast address. In our case the cmd could take the
*                                      the following relevant values:
*                                      CPSS_PACKET_CMD_FORWARD_E,
*                                      CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E,
*                                      CPSS_PACKET_CMD_DROP_SOFT_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on invalid input paramteres value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgSctIeeeReservedMcProtocolCmdSet
(
    IN GT_U32                                   profileIndex,
    IN PRV_TGF_SCT_IEEE_RSV_MC_PROTOCOL_ENT     protocolType,
    IN GT_U32                                   protocol,
    IN CPSS_PACKET_CMD_ENT                      cmd
)
{
#ifdef CHX_FAMILY
    TGF_PARAM_NOT_USED(profileIndex);
    TGF_PARAM_NOT_USED(protocolType);
    TGF_PARAM_NOT_USED(protocol);
    TGF_PARAM_NOT_USED(cmd);
    return GT_NOT_SUPPORTED;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
    CPSS_EXMXPM_SCT_IEEE_RSV_MC_PROTOCOL_ENT exMxPmProtocolType;


    /* convert protocolType into device specific format */
    switch (protocolType)
    {
        case PRV_TGF_SCT_IEEE_RSV_MC_PROTOCOL_STD_E:
            exMxPmProtocolType = CPSS_EXMXPM_SCT_IEEE_RSV_MC_PROTOCOL_STD_E;
            break;

        case PRV_TGF_SCT_IEEE_RSV_MC_PROTOCOL_GARP_E:
            exMxPmProtocolType = CPSS_EXMXPM_SCT_IEEE_RSV_MC_PROTOCOL_GARP_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmSctIeeeReservedMcProtocolCmdSet(devNum, profileIndex,
                                                         exMxPmProtocolType, protocol, cmd);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmSctIeeeReservedMcProtocolCmdSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgVlanIpmBridgingEnableSet function
* @endinternal
*
* @brief   IPv4/Ipv6 Multicast Bridging Enable
*
* @param[in] vlanId                   - Vlan Id
* @param[in] ipVer                    - IPv4 or Ipv6
* @param[in] enable                   - GT_TRUE - IPv4 multicast packets are bridged
*                                      according to ipv4IpmBrgMode
*                                      GT_FALSE - IPv4 multicast packets are bridged
*                                      according to MAC DA
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In xCat3, Caelum, Aldrin, AC3X and Bobcat3 devices, this API also takes part
*       in enabling the IP multicast routing by VLAN feature according to the
*       following logic:
*       BrgEnable  |     BrgMode     |  Bridging  |  Routing
*       ======================================================================
*       GT_FALSE (0) | CPSS_BRG_IPM_SGV_E (0) |  MAC DA   |  VRF-ID
*       ======================================================================
*       GT_FALSE (0) | CPSS_BRG_IPM_GV_E (1) |  MAC DA   |   VID
*       ======================================================================
*       GT_TRUE (1) | CPSS_BRG_IPM_SGV_E (0) | (S,G,V)   |  VRF-ID
*       ======================================================================
*       GT_TRUE (1) | CPSS_BRG_IPM_GV_E (1) | (,G,V)   |  VRF-ID
*       explanation for the table:
*       'BrgEnable' - set by this API (per VLAN, per IP protocol).
*       'BrgMode' - set by prvTgfBrgVlanIpmBridgingModeSet (per VLAN, per IP protocol).
*       'Bridging' - multicast bridging mode, relevant to all devices.
*       'Routing' - multicast routing mode, relevant only to xCat3, Caelum, Aldrin, AC3X and Bobcat3
*       devices. There are 2 possible vrf-id assignments for the ingress packet descriptor:
*       1. VRF-ID: IP multicast routing by VLAN feature is off and vrf-id is assigned according
*       to the VRF-ID field in the VLAN entry.
*       2. VID: IP multicast routing by VLAN feature is on and vrf-id is assigned to be the
*       ingress packet's VID itself.
*       In devices that do not support IP multicast routing by VLAN feature, routing mode is always VRF-ID.
*
*/
GT_STATUS prvTgfBrgVlanIpmBridgingEnableSet
(
    IN  GT_U16                      vlanId,
    IN  CPSS_IP_PROTOCOL_STACK_ENT  ipVer,
    IN  GT_BOOL                     enable
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

#ifdef CHX_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgVlanIpmBridgingEnable(devNum, vlanId, ipVer, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgVlanIpmBridgingEnable FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmBrgVlanIpmBridgingEnableSet(devNum, vlanId, ipVer, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgVlanIpmBridgingEnableSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

    return rc1;
}

/**
* @internal prvTgfBrgVlanIpmBridgingModeSet function
* @endinternal
*
* @brief   Sets the IPM bridging mode of Vlan. Relevant when IPM Bridging enabled.
*
* @param[in] vlanId                   - Vlan Id
* @param[in] ipVer                    - IPv4 or Ipv6
* @param[in] ipmMode                  - IPM bridging mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or vlanId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In xCat3, Caelum, Aldrin, AC3X and Bobcat3 devices, this API also takes part
*       in enabling the IP multicast routing by VLAN feature according to the
*       following logic:
*       BrgEnable  |     BrgMode     |  Bridging  |  Routing
*       ======================================================================
*       GT_FALSE (0) | CPSS_BRG_IPM_SGV_E (0) |  MAC DA   |  VRF-ID
*       ======================================================================
*       GT_FALSE (0) | CPSS_BRG_IPM_GV_E (1) |  MAC DA   |   VID
*       ======================================================================
*       GT_TRUE (1) | CPSS_BRG_IPM_SGV_E (0) | (S,G,V)   |  VRF-ID
*       ======================================================================
*       GT_TRUE (1) | CPSS_BRG_IPM_GV_E (1) | (,G,V)   |  VRF-ID
*       explanation for the table:
*       'BrgEnable' - set by prvTgfBrgVlanIpmBridgingEnableSet (per VLAN, per IP protocol).
*       'BrgMode' - set by this API (per VLAN, per IP protocol).
*       'Bridging' - multicast bridging mode, relevant to all devices.
*       'Routing' - multicast routing mode, relevant only to xCat3, Caelum, Aldrin, AC3X and Bobcat3
*       devices. There are 2 possible vrf-id assignments for the ingress packet descriptor:
*       1. VRF-ID: IP multicast routing by VLAN feature is off and vrf-id is assigned according
*       to the VRF-ID field in the VLAN entry.
*       2. VID: IP multicast routing by VLAN feature is on and vrf-id is assigned to be the
*       ingress packet's VID itself.
*       In devices that do not support IP multicast routing by VLAN feature, routing mode is always VRF-ID.
*
*/
GT_STATUS prvTgfBrgVlanIpmBridgingModeSet
(
    IN  GT_U16                      vlanId,
    IN  CPSS_IP_PROTOCOL_STACK_ENT  ipVer,
    IN  CPSS_BRG_IPM_MODE_ENT       ipmMode
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

#ifdef CHX_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgVlanIpmBridgingModeSet(devNum, vlanId, ipVer, ipmMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgVlanIpmBridgingModeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmBrgVlanIpmBridgingModeSet(devNum, vlanId, ipVer, ipmMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgVlanIpmBridgingModeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

    return rc1;
}

/**
* @internal prvTgfBrgGenDropNonIpMcEnable function
* @endinternal
*
* @brief   Sets an ability(Enable/Disable) to drop non-IPv4/6 Mc packets.
*         If it is enabled, the Drop Command assigned to all Ethernet
*         packets with MAC Multicast DA (MACDA[40] = 1), that are NOT in
*         the IPv4 Multicast IANA range (01-00-5E-00-00-00 to 01-00-5E-7F-FF-FF)
*         or in IPv6 Multicast IANA range (33-33-xx-xx-xx).
*         This does not include ARP Broadcast or Broadcast packets.
* @param[in] state                    - GT_TRUE, enable filtering mode
*                                      GT_FALSE, disable filtering mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgGenDropNonIpMcEnable
(
    IN GT_BOOL  state
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

#ifdef CHX_FAMILY

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgGenDropNonIpMcEnable(devNum, state);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgGenDropNonIpMcEnable FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmBrgGenDropNonIpMcEnableSet(devNum, state);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgGenDropNonIpMcEnableSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

    return rc1;
}

/**
* @internal prvTgfBrgSecurityBreachEventDropModeSet function
* @endinternal
*
* @brief   Sets the Drop Mode for given Security Breach event.
*
* @param[in] eventType                - Security Breach event type.
* @param[in] dropMode                 - soft or hard drop mode.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on HW error.
* @retval GT_BAD_PARAM             - on wrong devNum, eventType or dropMode.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgSecurityBreachEventDropModeSet
(
    IN CPSS_BRG_SECUR_BREACH_EVENTS_ENT     eventType,
    IN CPSS_DROP_MODE_TYPE_ENT              dropMode
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

#ifdef CHX_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgSecurBreachEventDropModeSet(devNum, eventType, dropMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgSecurBreachEventDropModeSet, rc = [%d]", rc);
            rc1 = rc;
        }
    }
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmBrgSecurityBreachEventDropModeSet(devNum, eventType, dropMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgSecurityBreachEventDropModeSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

    return rc1;
}

/**
* @internal prvTgfBrgSecurityBreachEventDropModeGet function
* @endinternal
*
* @brief   Gets Drop Mode for given Security Breach event.
*
* @param[in] devNum                   - device number.
* @param[in] eventType                - security breach event type.
*
* @param[out] dropModePtr              - pointer to the drop mode: soft or hard.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on HW error.
* @retval GT_BAD_PARAM             - on wrong devNum or eventType.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgSecurityBreachEventDropModeGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_BRG_SECUR_BREACH_EVENTS_ENT    eventType,
    OUT CPSS_DROP_MODE_TYPE_ENT             *dropModePtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgSecurBreachEventDropModeGet(devNum, eventType, dropModePtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmBrgSecurityBreachEventDropModeGet(devNum, eventType, dropModePtr);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgSecurityBreachPortVlanDropCntrModeSet function
* @endinternal
*
* @brief   Set Port/VLAN Security Breach Drop Counter to count security breach
*         dropped packets based on there ingress port or their assigned VID.
* @param[in] cntrCfgPtr               - (pointer to) security breach port/vlan counter
*                                      mode and it's configuration parameters.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - bad input parameters.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgSecurityBreachPortVlanDropCntrModeSet
(
    IN CPSS_BRG_SECUR_BREACH_DROP_COUNT_CFG_STC     *cntrCfgPtr
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

#ifdef CHX_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgSecurBreachPortVlanDropCntrModeSet(devNum, cntrCfgPtr);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgSecurBreachPortVlanDropCntrModeSet, rc = [%d]", rc);
            rc1 = rc;
        }
    }
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_BRG_SECUR_BREACH_DROP_COUNT_CFG_STC   cntrCfg = {0};

    /* convert parameters to device specific format */
    switch (cntrCfgPtr->dropCntMode)
    {
        case CPSS_BRG_SECUR_BREACH_DROP_COUNT_PORT_E:
            cntrCfg.dropCntMode = CPSS_EXMXPM_BRG_SECUR_BREACH_DROP_COUNT_PORT_E;
            break;
        case CPSS_BRG_SECUR_BREACH_DROP_COUNT_VLAN_E:
            cntrCfg.dropCntMode = CPSS_EXMXPM_BRG_SECUR_BREACH_DROP_COUNT_FID_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    cntrCfg.port = (GT_U8) cntrCfgPtr->port;
    cntrCfg.fId = cntrCfgPtr->vlan;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmBrgSecurityBreachPortVlanDropCntrModeSet(devNum, &cntrCfg);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgSecurityBreachPortVlanDropCntrModeSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

    return rc1;
}

/**
* @internal prvTgfBrgSecurityBreachPortVlanDropCntrModeGet function
* @endinternal
*
* @brief   Get mode and it's configuration parameters for security breach
*         Port/VLAN drop counter.
*
* @param[out] cntrCfgPtr               - (pointer to) security breach port/vlan counter
*                                      mode and it's configuration parameters.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - bad input parameters.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgSecurityBreachPortVlanDropCntrModeGet
(
    IN  GT_U8                                        devNum,
    OUT CPSS_BRG_SECUR_BREACH_DROP_COUNT_CFG_STC     *cntrCfgPtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgSecurBreachPortVlanDropCntrModeGet(devNum, cntrCfgPtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_STATUS rc = GT_OK;
    CPSS_EXMXPM_BRG_SECUR_BREACH_DROP_COUNT_CFG_STC   cntrCfg = {0};

    /* call device specific API */
    rc = cpssExMxPmBrgSecurityBreachPortVlanDropCntrModeGet(devNum, &cntrCfg);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* convert parameters from device specific format */
    switch (cntrCfg.dropCntMode)
    {
        case CPSS_EXMXPM_BRG_SECUR_BREACH_DROP_COUNT_PORT_E:
            cntrCfgPtr->dropCntMode = CPSS_BRG_SECUR_BREACH_DROP_COUNT_PORT_E;
            break;
        case CPSS_EXMXPM_BRG_SECUR_BREACH_DROP_COUNT_FID_E:
            cntrCfgPtr->dropCntMode = CPSS_BRG_SECUR_BREACH_DROP_COUNT_VLAN_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    cntrCfgPtr->port = cntrCfg.port;
    cntrCfgPtr->vlan = cntrCfg.fId;

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgEgressPortUnknownUcFilteringSet function
* @endinternal
*
* @brief   Enable/Disable egress Filtering for bridged Unknown Unicast packets
*         on the specified egress port.
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE: Unknown Unicast packets are filtered and are
*                                      not forwarded to this port.
*                                      GT_FALSE: Unknown Unicast packets are not filtered and may
*                                      be forwarded to this port.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong dev or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfBrgEgressPortUnknownUcFilteringSet
(
    IN GT_U32            portNum,
    IN GT_BOOL          enable
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

#ifdef CHX_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgPortEgrFltUnkEnable(devNum, (GT_PORT_NUM) portNum, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgPortEgrFltUnkEnable, rc = [%d]", rc);
            rc1 = rc;
        }
    }
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmBrgEgressFilteringPortUnkUnicastEnableSet(devNum, portNum, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgEgressFilteringPortUnkUnicastEnableSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

    return rc1;
}

/**
* @internal prvTgfBrgEgressPortUnknownUcFilteringGet function
* @endinternal
*
* @brief   Gets the current status of bridged Unknown Unicast packets filtering
*         on the specified egress port.
* @param[in] devNum                   - device number
* @param[in] portNum                  - CPU port, virtual port number
*
* @param[out] enablePtr                - pointer to
*                                      GT_TRUE: Unknown Unicast packets are filtered and are
*                                      not forwarded to this port.
*                                      GT_FALSE: Unknown Unicast packets are not filtered and may
*                                      be forwarded to this port.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong dev or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfBrgEgressPortUnknownUcFilteringGet
(
    IN  GT_U8    devNum,
    IN  GT_U32    portNum,
    OUT GT_BOOL  *enablePtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgPortEgrFltUnkEnableGet(devNum, (GT_PORT_NUM) portNum, enablePtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmBrgEgressFilteringPortUnkUnicastEnableGet(devNum, portNum, enablePtr);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgEgressPortUnregMcFilteringSet function
* @endinternal
*
* @brief   Enable/Disable egress filtering for bridged unregistered multicast packets
*         on the specified egress port.
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE: Unregistered Multicast packets are filtered and
*                                      are not forwarded to this port.
*                                      GT_FALSE:Unregistered Multicast packets are not filtered
*                                      and may be forwarded to this port.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong dev or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfBrgEgressPortUnregMcFilteringSet
(
    IN GT_U32            portNum,
    IN GT_BOOL          enable
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

#ifdef CHX_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgPortEgrFltUregMcastEnable(devNum, (GT_PORT_NUM) portNum, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgPortEgrFltUregMcastEnable, rc = [%d]", rc);
            rc1 = rc;
        }
    }
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmBrgEgressFilteringPortUnregMcastEnableSet(devNum, portNum, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgEgressFilteringPortUnregMcastEnableSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

    return rc1;
}

/**
* @internal prvTgfBrgEgressPortUnregMcFilteringGet function
* @endinternal
*
* @brief   This function gets the egress Filtering current state (enable/disable)
*         for bridged Unregistered Multicast packets on the specified egress port.
* @param[in] devNum                   - device number
* @param[in] portNum                  - CPU port, virtual port number
*
* @param[out] enablePtr                - points to:
*                                      GT_TRUE:  Unregistered Multicast packets are filtered and
*                                      are not forwarded to this port.
*                                      GT_FALSE: Unregistered Multicast packets are not filtered
*                                      and may be forwarded to this port.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong dev or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfBrgEgressPortUnregMcFilteringGet
(
    IN  GT_U8    devNum,
    IN  GT_U32    portNum,
    OUT GT_BOOL  *enablePtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgPortEgrFltUregMcastEnableGet(devNum,
                                                   (GT_PORT_NUM) portNum,
                                                   enablePtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmBrgEgressFilteringPortUnregMcastEnableGet(devNum,
                                                               portNum,
                                                               enablePtr);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgEgressPortUnregBcFilteringSet function
* @endinternal
*
* @brief   Enables or disables egress filtering of unregistered broadcast packets.
*         Unregistered broadcast packets are:
*         - packets with destination MAC ff-ff-ff-ff-ff-ff
*         - destination MAC address lookup not finds matching entry
*         - packets were not routed
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE  -  filtering of unregistered broadcast packets.
*                                      unregistered broadcast packets are dropped
*                                      GT_FALSE - disable filtering of unregistered broadcast packets.
*                                      unregistered broadcast packets are not dropped
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong dev or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfBrgEgressPortUnregBcFilteringSet
(
    IN GT_U32            portNum,
    IN GT_BOOL          enable
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

#ifdef CHX_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgPortEgrFltUregBcEnable(devNum, (GT_PORT_NUM) portNum, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgPortEgrFltUnkEnable, rc = [%d]", rc);
            rc1 = rc;
        }
    }
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmBrgEgressFilteringPortUnregBcEnableSet(devNum, portNum, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgEgressFilteringPortUnkUnicastEnableSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

    return rc1;
}

/**
* @internal prvTgfBrgEgressPortUnregBcFilteringGet function
* @endinternal
*
* @brief   This function gets the egress Filtering current state (enable/disable)
*         of unregistered broadcast packets.
*         Unregistered broadcast packets are:
*         - packets with destination MAC ff-ff-ff-ff-ff-ff
*         - destination MAC address lookup not finds matching entry
*         - packets were not routed
* @param[in] devNum                   - device number
* @param[in] portNum                  - CPU port, virtual port number
*
* @param[out] enablePtr                - points to unregistered BC packets filtering
*                                      GT_TRUE  - enable filtering of unregistered broadcast packets.
*                                      unregistered broadcast packets are dropped
*                                      GT_FALSE - disable filtering of unregistered broadcast packets.
*                                      unregistered broadcast packets are not dropped
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong dev or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfBrgEgressPortUnregBcFilteringGet
(
    IN  GT_U8    devNum,
    IN  GT_U32    portNum,
    OUT GT_BOOL  *enablePtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgPortEgrFltUregBcEnableGet(devNum, (GT_PORT_NUM) portNum, enablePtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmBrgEgressFilteringPortUnregBcEnableGet(devNum, portNum, enablePtr);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}


/**
* @internal prvTgfBrgStpEntryRead function
* @endinternal
*
* @brief   Reads a raw STP group entry from HW.
*
* @param[in] dev                      - device number
*                                      stgId        - STG (Spanning Tree Group) index
*
* @param[out] stpEntryPtr              - (Pointer to) The raw stp entry to get.
*                                      memory allocated by the caller.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_OUT_OF_RANGE          - on stgId is larger than allowed
* @retval GT_BAD_PTR               - on NULL pointer parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - the ingress table and egress table values
*                                       do not match
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgStpEntryRead
(
    IN  GT_U8       dev,
    IN  GT_U16      stpId,
    OUT GT_U32      *stpEntryPtr
)
{

#ifdef CHX_FAMILY
    GT_U32      dxChStpEntryWordArr[CPSS_DXCH_STG_ENTRY_SIZE_CNS] = {0};
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_U32      exMxPmStpEntryWordArr[CPSS_EXMXPM_STG_ENTRY_SIZE_CNS] = {0};
#endif /* EXMXPM_FAMILY */

#ifdef CHX_FAMILY
    /* call device specific API */
    if(GT_OK != cpssDxChBrgStpEntryGet(dev, stpId, dxChStpEntryWordArr))
    {
        return GT_FAIL;
    }

    cpssOsMemCpy(stpEntryPtr, dxChStpEntryWordArr, sizeof(dxChStpEntryWordArr));

    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */

    CPSS_COVERITY_NON_ISSUE_BOOKMARK
    /* coverity[overrun-buffer-val] */
    if(GT_OK != cpssExMxPmBrgStpEntryRead(dev, stpId, exMxPmStpEntryWordArr))
    {
        return GT_FAIL;
    }

    cpssOsMemCpy(stpEntryPtr, exMxPmStpEntryWordArr, sizeof(exMxPmStpEntryWordArr));

    return GT_OK;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgStpEntryWrite function
* @endinternal
*
* @brief   Writes a raw STP entry to HW.
*
* @param[in] dev                      - device number
* @param[in] stpId                    - STG (Spanning Tree Group) index
* @param[in] stpEntryPtr              - (Pointer to) The raw stp entry to write
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device or stpId
* @retval GT_BAD_PTR               - on NULL pointer parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgStpEntryWrite
(
    IN  GT_U8       dev,
    IN  GT_U16      stpId,
    IN  GT_U32      *stpEntryPtr
)
{

#ifdef CHX_FAMILY
    GT_U32      dxChStpEntryWordArr[CPSS_DXCH_STG_ENTRY_SIZE_CNS] = {0};
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_U32      exMxPmStpEntryWordArr[CPSS_EXMXPM_STG_ENTRY_SIZE_CNS] = {0};
#endif /* EXMXPM_FAMILY */

#ifdef CHX_FAMILY

    cpssOsMemCpy(dxChStpEntryWordArr, stpEntryPtr, sizeof(dxChStpEntryWordArr));
    /* call device specific API */
    return cpssDxChBrgStpEntryWrite(dev, stpId, dxChStpEntryWordArr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    cpssOsMemCpy(exMxPmStpEntryWordArr, stpEntryPtr, sizeof(exMxPmStpEntryWordArr));

    /* call device specific API */

    CPSS_COVERITY_NON_ISSUE_BOOKMARK
    /* coverity[overrun-buffer-val] */
    return cpssExMxPmBrgStpEntryWrite(dev, stpId, exMxPmStpEntryWordArr);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgVlanIngressEtherTypeSet function
* @endinternal
*
* @brief   Sets Ingress VLAN Ether Type and Range
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanIngressEtherTypeSet
(
    IN  CPSS_ETHER_MODE_ENT     etherTypeMode,
    IN  GT_U16                  etherType,
    IN  GT_U16                  maxVidRange
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
#ifdef CHX_FAMILY
    GT_U32      entryIndex = 0;

    /* configure VLAN ethertype */
    switch (etherTypeMode)
    {
        case CPSS_VLAN_ETHERTYPE0_E:
            entryIndex = 0;
            break;
        case CPSS_VLAN_ETHERTYPE1_E:
            entryIndex = 1;
            break;
        default:
            return GT_BAD_PARAM;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* sets Tag Protocol ID table entry */
        rc = cpssDxChBrgVlanTpidEntrySet(devNum, CPSS_DIRECTION_INGRESS_E, entryIndex, etherType);
        if (rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgVlanTpidEntrySet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }

        /* configures the valid VLAN Range */
        rc = cpssDxChBrgVlanRangeSet(devNum, maxVidRange);
        if (rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgVlanRangeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmBrgVlanIngressEtherTypeSet(devNum, etherTypeMode, etherType, maxVidRange);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgVlanIngressEtherTypeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

    return rc1;
}

/**
* @internal prvTgfBrgVlanIngressEtherTypeGet function
* @endinternal
*
* @brief   Gets Ingress VLAN Ether Type and Range
*
* @param[in] etherTypeMode            - mode of tagged vlan etherType. May be either
*                                      CPSS_VLAN_ETHERTYPE0_E or CPSS_VLAN_ETHERTYPE1_E.
*                                      See comment.
*
* @param[out] etherTypePtr             - the EtherType to be recognized tagged packets. See
*                                      comment.
* @param[out] maxVidRangePtr           - (pointer to)the maximum VID that may be assigned to
*                                      a packet received on a port with <Ingress VLANSel> set
*                                      to 1.
*                                      If the VID assigned to the packet is greater than this
*                                      value, a security breach occurs and the packet is
*                                      assigned with a hard drop or a soft drop command,
*                                      according to the setting of <VLANRange DropMode>
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note If the vid of the incoming packet is greater or equal than the vlan
*       range value, the packet is filtered.
*       Configuration of vid and range for CPSS_VLAN_ETHERTYPE1_E should be done
*       only when nested vlan is used. User can alter the etherType field of
*       the encapsulating vlan by setting a number other then 0x8100 in the
*       parameter <ethertype>.
*
*/
GT_STATUS prvTgfBrgVlanIngressEtherTypeGet
(
    IN   CPSS_ETHER_MODE_ENT     etherTypeMode,
    OUT  GT_U16                  *etherTypePtr,
    OUT  GT_U16                  *maxVidRangePtr
)
{

#ifdef CHX_FAMILY
    GT_U32      entryIndex = 0;
    GT_STATUS   rc;

    /* configure VLAN ethertype */
    switch (etherTypeMode)
    {
        case CPSS_VLAN_ETHERTYPE0_E:
            entryIndex = 0;
            break;
        case CPSS_VLAN_ETHERTYPE1_E:
            entryIndex = 1;
            break;
        default:
            return GT_BAD_PARAM;
    }

    rc = prvTgfBrgVlanRangeGet(maxVidRangePtr);
    return (GT_OK != rc) ? rc :
        cpssDxChBrgVlanTpidEntryGet(prvTgfDevNum, CPSS_DIRECTION_INGRESS_E, entryIndex, etherTypePtr);

#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
   return cpssExMxPmBrgVlanIngressEtherTypeGet(prvTgfDevNum, etherTypeMode, etherTypePtr, maxVidRangePtr);

#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgCntMacDaSaSet function
* @endinternal
*
* @brief   Sets a specific MAC DA and SA to be monitored by Host
*         and Matrix counter groups on specified device.
* @param[in] macSaPtr                 - pointer to the source MAC address.
* @param[in] macDaPtr                 - pointer to the destination MAC address.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on HW error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgCntMacDaSaSet
(
    IN  GT_ETHERADDR    *macSaPtr,
    IN  GT_ETHERADDR    *macDaPtr
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

#ifdef CHX_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgCntMacDaSaSet(devNum, macSaPtr, macDaPtr);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgCntMacDaSaSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmBrgCntMacDaSaSet(devNum, macSaPtr, macDaPtr);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgCntMacDaSaSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

    return rc1;
}

/**
* @internal prvTgfBrgCntMacDaSaGet function
* @endinternal
*
* @brief   Gets a MAC DA and SA are monitored by Host
*         and Matrix counter groups on specified device.
* @param[in] devNum                   - physical device number.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgCntMacDaSaGet
(
    IN   GT_U8           devNum,
    OUT  GT_ETHERADDR    *macSaPtr,
    OUT  GT_ETHERADDR    *macDaPtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgCntMacDaSaGet(devNum, macSaPtr, macDaPtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmBrgCntMacDaSaGet(devNum, macSaPtr, macDaPtr);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgCntHostGroupCntrsGet function
* @endinternal
*
* @brief   Gets Bridge Host group counters value of specified device.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] hostGroupCntrPtr         - pointer to structure with current counters value.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on HW error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Bridge Host counters are clear-on-read.
*
*/
GT_STATUS prvTgfBrgCntHostGroupCntrsGet
(
    IN   GT_U8                        devNum,
    OUT  PRV_TGF_BRG_HOST_CNTR_STC   *hostGroupCntrPtr
)
{
    GT_STATUS   rc = GT_OK;

#ifdef CHX_FAMILY
    CPSS_DXCH_BRIDGE_HOST_CNTR_STC  dxChHostGroupCntr = {0};

    /* call device specific API */
    rc = cpssDxChBrgCntHostGroupCntrsGet(devNum, &dxChHostGroupCntr);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgCntHostGroupCntrsGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* clear entry */
    cpssOsMemSet((GT_VOID*) hostGroupCntrPtr, 0, sizeof(*hostGroupCntrPtr));

    /* convert hostGroupCntrPtr from device specific format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChHostGroupCntr, hostGroupCntrPtr, gtHostInPkts);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChHostGroupCntr, hostGroupCntrPtr, gtHostOutBroadcastPkts);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChHostGroupCntr, hostGroupCntrPtr, gtHostOutMulticastPkts);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChHostGroupCntr, hostGroupCntrPtr, gtHostOutPkts);

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_BRG_HOST_CNTR_STC  exMxPmHostGroupCntr = {0};

    /* call device specific API */
    rc = cpssExMxPmBrgCntHostGroupCntrsGet(devNum, &exMxPmHostGroupCntr);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgCntHostGroupCntrsGet FAILED, rc = [%d]", rc);

        return  rc;
    }

    /* clear entry */
    cpssOsMemSet((GT_VOID*) hostGroupCntrPtr, 0, sizeof(*hostGroupCntrPtr));

    /* convert hostGroupCntrPtr from device specific format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&exMxPmHostGroupCntr, hostGroupCntrPtr, gtHostInPkts);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&exMxPmHostGroupCntr, hostGroupCntrPtr, gtHostOutBroadcastPkts);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&exMxPmHostGroupCntr, hostGroupCntrPtr, gtHostOutMulticastPkts);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&exMxPmHostGroupCntr, hostGroupCntrPtr, gtHostOutPkts);

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgCntMatrixGroupCntrsGet function
* @endinternal
*
* @brief   Gets Bridge Matrix counter value of specified device.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] matrixCntSaDaPktsPtr     - number of packets (good only) with a MAC SA/DA
*                                      matching of the CPU-configured MAC SA/DA.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Bridge Matrix counter is clear-on-read.
*
*/
GT_STATUS prvTgfBrgCntMatrixGroupCntrsGet
(
    IN   GT_U8      devNum,
    OUT  GT_U32     *matrixCntSaDaPktsPtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgCntMatrixGroupCntrsGet(devNum, matrixCntSaDaPktsPtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmBrgCntMatrixGroupCntrsGet(devNum, matrixCntSaDaPktsPtr);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgCntBridgeIngressCntrsGet function
* @endinternal
*
* @brief   Gets a Bridge ingress Port/VLAN/Device counters from
*         specified counter set.
* @param[in] devNum                   - physical device number.
* @param[in] cntrSetId                - counter set number [0:1] (there are 2 Bridge Ingress
*                                      Counter Sets).
*
* @param[out] ingressCntrPtr           - pointer to the structure of Bridge Ingress Counters
*                                      current values.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on HW error.
* @retval GT_BAD_PARAM             - on wrong devNum or cntrSetId.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Bridge Ingress counters are clear-on-read.
*
*/
GT_STATUS prvTgfBrgCntBridgeIngressCntrsGet
(
    IN  GT_U8                               devNum,
    IN  PRV_TGF_BRG_CNTR_SET_ID_ENT         cntrSetId,
    OUT CPSS_BRIDGE_INGRESS_CNTR_STC        *ingressCntrPtr
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_BRIDGE_CNTR_SET_ID_ENT dxChCntrSetId;

    /* convert cntrSetId into device specific format */
    switch (cntrSetId)
    {
        case PRV_TGF_BRG_CNTR_SET_ID_0_E:
            dxChCntrSetId = CPSS_DXCH_BRG_CNT_SET_ID_0_E;
            break;

        case PRV_TGF_BRG_CNTR_SET_ID_1_E:
            dxChCntrSetId = CPSS_DXCH_BRG_CNT_SET_ID_1_E;
            break;

        default:
            return GT_BAD_PARAM;
    }
    /* call device specific API */
    return cpssDxChBrgCntBridgeIngressCntrsGet(devNum, dxChCntrSetId, ingressCntrPtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_BRG_CNTR_SET_ID_ENT exMxPmCntrSetId;
    GT_STATUS                       rc;
    static CPSS_BRIDGE_INGRESS_CNTR_STC    pmLastTimeRead[2];/*WA for Puma3 that not clear on read*/
    static GT_BOOL  initDone = GT_FALSE;
    CPSS_BRIDGE_INGRESS_CNTR_STC pmIngCntr_2;
    GT_U32  puma3_gm_clearOnRead_bug = 0;

    /* convert cntrSetId into device specific format */
    switch (cntrSetId)
    {
        case PRV_TGF_BRG_CNTR_SET_ID_0_E:
            exMxPmCntrSetId = CPSS_EXMXPM_BRG_CNTR_SET_ID_0_E;
            break;

        case PRV_TGF_BRG_CNTR_SET_ID_1_E:
            exMxPmCntrSetId = CPSS_EXMXPM_BRG_CNTR_SET_ID_1_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* call device specific API */
    rc = cpssExMxPmBrgCntBridgeIngressCntrsGet(devNum, exMxPmCntrSetId, ingressCntrPtr);
    if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_PUMA3_E &&
       puma3_gm_clearOnRead_bug == 1 )
    {
        if(initDone == GT_FALSE)
        {
            initDone = GT_TRUE;
            cpssOsMemSet(&pmLastTimeRead,0,sizeof(pmLastTimeRead));
        }

        pmIngCntr_2 = *ingressCntrPtr;

        /* clear on read is not working . */
        ingressCntrPtr->gtBrgInFrames           -=  pmLastTimeRead[cntrSetId].gtBrgInFrames            ;
        ingressCntrPtr->gtBrgVlanIngFilterDisc  -=  pmLastTimeRead[cntrSetId].gtBrgVlanIngFilterDisc   ;
        ingressCntrPtr->gtBrgSecFilterDisc      -=  pmLastTimeRead[cntrSetId].gtBrgSecFilterDisc       ;
        ingressCntrPtr->gtBrgLocalPropDisc      -=  pmLastTimeRead[cntrSetId].gtBrgLocalPropDisc       ;
        /*save for next time*/
        pmLastTimeRead[cntrSetId] = pmIngCntr_2;
    }
    return rc;

#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgCntBridgeIngressCntrModeSet function
* @endinternal
*
* @brief   Configures a specified Set of Bridge Ingress
*         counters to work in requested mode.
* @param[in] cntrSetId                - Counter Set ID
* @param[in] setMode                  - count mode of specified Set of Bridge Ingress Counters.
* @param[in] port                     -  number monitored by Set of Counters.
*                                      This parameter is applied upon CPSS_BRG_CNT_MODE_1_E and
*                                      CPSS_BRG_CNT_MODE_3_E counter modes.
* @param[in] vlan                     - VLAN ID monitored by Counters Set.
*                                      This parameter is applied upon CPSS_BRG_CNT_MODE_2_E and
*                                      CPSS_BRG_CNT_MODE_3_E counter modes.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum, setMode or cntrSetId.
* @retval GT_OUT_OF_RANGE          - on port number or vlan out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgCntBridgeIngressCntrModeSet
(
    IN  PRV_TGF_BRG_CNTR_SET_ID_ENT         cntrSetId,
    IN  CPSS_BRIDGE_INGR_CNTR_MODES_ENT     setMode,
    IN  GT_PORT_NUM                         port,
    IN  GT_U16                              vlan
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
#ifdef CHX_FAMILY
    CPSS_DXCH_BRIDGE_CNTR_SET_ID_ENT dxChCntrSetId;

    /* convert cntrSetId into device specific format */
    switch (cntrSetId)
    {
        case PRV_TGF_BRG_CNTR_SET_ID_0_E:
            dxChCntrSetId = CPSS_DXCH_BRG_CNT_SET_ID_0_E;
            break;

        case PRV_TGF_BRG_CNTR_SET_ID_1_E:
            dxChCntrSetId = CPSS_DXCH_BRG_CNT_SET_ID_1_E;
            break;

        default:
            return GT_BAD_PARAM;
    }


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgCntBridgeIngressCntrModeSet(devNum, dxChCntrSetId, setMode, port, vlan);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgCntBridgeIngressCntrModeSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_BRG_CNTR_SET_ID_ENT exMxPmCntrSetId;

    /* convert cntrSetId into device specific format */
    switch (cntrSetId)
    {
        case PRV_TGF_BRG_CNTR_SET_ID_0_E:
            exMxPmCntrSetId = CPSS_EXMXPM_BRG_CNTR_SET_ID_0_E;
            break;

        case PRV_TGF_BRG_CNTR_SET_ID_1_E:
            exMxPmCntrSetId = CPSS_EXMXPM_BRG_CNTR_SET_ID_1_E;
            break;

        default:
            return GT_BAD_PARAM;
    }


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmBrgCntBridgeIngressCntrModeSet(devNum, exMxPmCntrSetId,
                                                      setMode, (GT_U8) port, vlan);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgCntBridgeIngressCntrModeSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

    return rc1;
}

/**
* @internal prvTgfBrgCntBridgeIngressCntrModeGet function
* @endinternal
*
* @brief   Gets the mode (port number and VLAN Id as well) of specified
*         Bridge Ingress counters Set.
* @param[in] devNum                   - physical device number
* @param[in] cntrSetId                - Counter Set ID
*
* @param[out] setModePtr               - pointer to the count mode of specified Set of Bridge
*                                      Ingress Counters.
* @param[out] portPtr                  - pointer to the port number monitored by Set of Counters.
*                                      This parameter is applied upon CPSS_BRG_CNT_MODE_1_E and
*                                      CPSS_BRG_CNT_MODE_3_E counter modes.
*                                      This parameter can be NULL;
* @param[out] vlanPtr                  - pointer to the VLAN ID monitored by Counters Set.
*                                      This parameter is applied upon CPSS_BRG_CNT_MODE_2_E and
*                                      CPSS_BRG_CNT_MODE_3_E counter modes.
*                                      This parameter can be NULL;
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum or counter set number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgCntBridgeIngressCntrModeGet
(
    IN   GT_U8                               devNum,
    IN   PRV_TGF_BRG_CNTR_SET_ID_ENT         cntrSetId,
    OUT  CPSS_BRIDGE_INGR_CNTR_MODES_ENT     *setModePtr,
    OUT  GT_PORT_NUM                         *portPtr,
    OUT  GT_U16                              *vlanPtr
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_BRIDGE_CNTR_SET_ID_ENT dxChCntrSetId;

    /* convert cntrSetId into device specific format */
    switch (cntrSetId)
    {
        case PRV_TGF_BRG_CNTR_SET_ID_0_E:
            dxChCntrSetId = CPSS_DXCH_BRG_CNT_SET_ID_0_E;
            break;

        case PRV_TGF_BRG_CNTR_SET_ID_1_E:
            dxChCntrSetId = CPSS_DXCH_BRG_CNT_SET_ID_1_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* call device specific API */
    return cpssDxChBrgCntBridgeIngressCntrModeGet(devNum, dxChCntrSetId, setModePtr, portPtr, vlanPtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_BRG_CNTR_SET_ID_ENT exMxPmCntrSetId;
    GT_U8                           port;
    GT_STATUS                       rc;

    /* convert cntrSetId into device specific format */
    switch (cntrSetId)
    {
        case PRV_TGF_BRG_CNTR_SET_ID_0_E:
            exMxPmCntrSetId = CPSS_EXMXPM_BRG_CNTR_SET_ID_0_E;
            break;

        case PRV_TGF_BRG_CNTR_SET_ID_1_E:
            exMxPmCntrSetId = CPSS_EXMXPM_BRG_CNTR_SET_ID_1_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* call device specific API */
    rc = cpssExMxPmBrgCntBridgeIngressCntrModeGet(devNum, exMxPmCntrSetId, (CPSS_EXMXPM_BRG_INGRESS_CNTR_MODE_ENT *)setModePtr,
                                                     (GT_U8 *)&port, vlanPtr);
    *portPtr = port;
    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgVlanIngressMirrorEnable function
* @endinternal
*
* @brief   Enable/Disable ingress mirroring of packets to the Ingress Analyzer port
*         for packets assigned the given VLAN-ID
* @param[in] devNum                   - device id
* @param[in] vlanId                   - VLAN ID which ingress VLAN mirroring is enabled/disabled
* @param[in] enable                   - don't ingress mirrored traffic assigned to this VLAN to the
*                                      analyzer port.
* @param[in] index                    - mirror to analyzer  (applicable to: Lion3, Bobcat2)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanIngressMirrorEnable
(
    IN GT_U8   devNum,
    IN GT_U16  vlanId,
    IN GT_BOOL enable,
    IN GT_U32  index
)
{
#ifdef CHX_FAMILY
    return cpssDxChBrgVlanIngressMirrorEnable(devNum, vlanId, enable, index);
#else /* CHX_FAMILY */
    return GT_NOT_APPLICABLE_DEVICE;
#endif
}

/**
* @internal prvTgfBrgVlanMirrorToTxAnalyzerEnableSet function
* @endinternal
*
* @brief   Enable/Disable egress mirroring on this Vlan.
*         When enabled, egress traffic on this Vlan is mirrored
*         to the Tx Analyzer port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanMirrorToTxAnalyzerEnableSet
(
    GT_VOID
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

#ifdef CHX_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
            rc1 = GT_NOT_SUPPORTED;
            break;
    }
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmBrgVlanMirrorToTxAnalyzerEnableSet(devNum, vlanId, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgVlanMirrorToTxAnalyzerEnableSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

    return rc1;
}

/**
* @internal prvTgfBrgVlanStgIdBind function
* @endinternal
*
* @brief   Bind VLAN to STP Id.
*
* @param[in] vlanId                   - vlan Id
* @param[in] stgId                    - STG Id
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanStgIdBind
(
    IN GT_U16   vlanId,
    IN GT_U16   stgId
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

#ifdef CHX_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgVlanToStpIdBind(devNum, vlanId, stgId);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgVlanToStpIdBind FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_PUMA3_E)
        {
            if(vlanId == stgId)
            {
                rc = GT_OK;
            }
            else
            {
                /* puma 3 supports only stg == vid */
                rc = GT_NOT_SUPPORTED;
            }
        }
        else
        {
            /* call device specific API */
            rc = cpssExMxPmBrgVlanStgIdBind(devNum, vlanId, stgId);
        }

        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgVlanStgIdBind FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

    return rc1;
}

/**
* @internal prvTgfBrgStpPortStateSet function
* @endinternal
*
* @brief   Sets STP state of port belonging within an STP group.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] stgId                    - STG (Spanning Tree Group) index
* @param[in] state                    - STP port state.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device or port or stgId or state
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgStpPortStateSet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               stgId,
    IN CPSS_STP_STATE_ENT   state
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgStpStateSet(devNum, portNum, (GT_U16) stgId, state);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmBrgStpPortStateSet(devNum, (GT_U8) portNum, stgId, state);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgStpPortStateGet function
* @endinternal
*
* @brief   Get STP state of port belonging within an STP group.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] stgId                    - STG (Spanning Tree Group) index  (0..4095)
*
* @param[out] statePtr                 - (pointer ot) STP port state.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device or port or stgId
* @retval GT_BAD_PTR               - on NULL pointer parameter
* @retval GT_BAD_VALUE             - on bad stp state value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - the ingress table and egress table values
*                                       do not match
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgStpPortStateGet
(
    IN  GT_U8               devNum,
    IN  GT_U32               portNum,
    IN  GT_U32              stgId,
    OUT CPSS_STP_STATE_ENT  *statePtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgStpStateGet(devNum, portNum, (GT_U16) stgId, statePtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmBrgStpPortStateGet(devNum, (GT_U8) portNum, stgId, statePtr);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgGenDropInvalidSaEnableSet function
* @endinternal
*
* @brief   Sets an ability (Enable/Disable) to drop Ethernet packets
*         with Multicast MAC SA (MACSA[40] = 1).
* @param[in] enable                   - Boolean value:
*                                      GT_TRUE  - packets with the Mc MAC SA are dropped.
*                                      These packets are assigned with a soft
*                                      drop or a hard drop Command, according to
*                                      the Drop Mode of packets with invalid SA.
*                                      GT_FALSE - packets with the Mc MAC SA aren't dropped.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on HW error.
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgGenDropInvalidSaEnableSet
(
    IN GT_BOOL  enable
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

#ifdef CHX_FAMILY

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgGenDropInvalidSaEnable(devNum, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgGenDropInvalidSaEnable FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmBrgGenDropInvalidSaEnableSet(devNum, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgGenDropInvalidSaEnableSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

    return rc1;
}

/**
* @internal prvTgfBrgSecurityBreachPortMovedStaticAddrEnableSet function
* @endinternal
*
* @brief   Enables\Disables Security Breach for Moved Static addresses.
*         When the FDB entry <Static> bit is set to 1, it is not subject to Aging.
*         In addition, there may be cases in which the Port#/Trunk# in this entry
*         doesn't match the Port#/Trunk# from which this packet was received.
*         In this case the moved static address could (or couldn't) to be
*         considered as Security Breach at the specified port.
* @param[in] port                     -  number.
* @param[in] enable                   - Boolean value:
*                                      GT_TRUE  - moved static address is considered as
*                                      Security Breach at the specified port
*                                      and are dropped according to the
*                                      Moved Static Drop Mode (hard or soft).
*                                      GT_FALSE - moved static address isn't considered
*                                      as Security Breach at the specified port
*                                      and are forwarded regularly.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on HW error.
* @retval GT_BAD_PARAM             - on wrong devNum or port.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgSecurityBreachPortMovedStaticAddrEnableSet
(
    IN GT_U32    port,
    IN GT_BOOL  enable
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

#ifdef CHX_FAMILY
    TGF_PARAM_NOT_USED(port);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgSecurBreachMovedStaticAddrSet(devNum, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgSecurBreachMovedStaticAddrSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmBrgSecurityBreachPortMovedStaticAddrEnableSet(devNum, port, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgSecurityBreachPortMovedStaticAddrEnableSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

    return rc1;
}

/**
* @internal prvTgfBrgSecurityBreachPortMovedStaticAddrEnableGet function
* @endinternal
*
* @brief   Get if static addresses that are moved, are regarded as Security Breach
*         or not (this is only in case that the fdb entry is static or the
*         interface does't match the interface from which this packet was received).
*
* @param[out] enablePtr                - GT_TRUE - moved static addresses are regarded as
*                                      security breach event and are dropped.
*                                      GT_FALSE - moved static addresses are not regarded
*                                      as security breach event and are forwarded.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - bad input parameters.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgSecurityBreachPortMovedStaticAddrEnableGet
(
    IN  GT_U8    devNum,
    IN  GT_U32    port,
    OUT GT_BOOL  *enablePtr
)
{
#ifdef CHX_FAMILY
    TGF_PARAM_NOT_USED(port);

    /* call device specific API */
    return cpssDxChBrgSecurBreachMovedStaticAddrGet(devNum, enablePtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmBrgSecurityBreachPortMovedStaticAddrEnableGet(devNum, port, enablePtr);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgPortUnkSrcMacDropSet function
* @endinternal
*
* @brief   Enable dropping unknown source mac address.
*
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on unknown command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgPortUnkSrcMacDropSet
(
    IN GT_PORT_NUM              portNum
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
    GT_U8       portIdx = 0;

#ifdef EXMXPM_FAMILY

    CPSS_EXMXPM_INLIF_ENTRY_STC     inlifEntry = {0};
#endif /* EXMXPM_FAMILY */

    /* finding portIdx by portNum */
    for (portIdx = 0; portIdx < prvTgfPortsNum; portIdx++)
    {
        if (prvTgfPortsArray[portIdx] == portNum)
        {
            break;
        }
    }

    /* check that portNum isn't out of range */
    if (portIdx == prvTgfPortsNum)
    {
        PRV_UTF_LOG0_MAC("[TGF]: prvTgfBrgPortUnkSrcMacDropSet FAILED");

        return GT_BAD_PARAM;
    }

#ifdef CHX_FAMILY

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* saving default values if such didn't save */
        if (GT_FALSE == prvTgfPortUnkSrcMacDropRestoreCfg[devNum][portIdx].rstStatus)
        {
            /* clear entry */
            cpssOsMemSet(&prvTgfPortUnkSrcMacDropRestoreCfg[devNum][portIdx], 0,
                         sizeof(prvTgfPortUnkSrcMacDropRestoreCfg[0][0]));

            /* call get function for config restore */
            rc = cpssDxChBrgFdbPortLearnStatusGet(devNum, portNum,
                                                  &prvTgfPortUnkSrcMacDropRestoreCfg[devNum][portIdx].status,
                                                  &prvTgfPortUnkSrcMacDropRestoreCfg[devNum][portIdx].cmd);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbPortLearnStatusGet FAILED, rc = [%d]", rc);

                rc1 = rc;
            }

            /* set rstStatus to true */
            prvTgfPortUnkSrcMacDropRestoreCfg[devNum][portIdx].rstStatus = GT_TRUE;
        }

        /* call device specific API */
        rc = cpssDxChBrgFdbPortLearnStatusSet(devNum, portNum, GT_FALSE,
                                              CPSS_LOCK_DROP_E);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbPortLearnStatusSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);


    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* saving default values if such didn't save */
        if (GT_FALSE == prvTgfPortUnkSrcMacDropRestoreCfg[devNum][portIdx].rstStatus)
        {
            /* clear entry */
            cpssOsMemSet(&prvTgfPortUnkSrcMacDropRestoreCfg[devNum][portIdx], 0,
                         sizeof(prvTgfPortUnkSrcMacDropRestoreCfg[0][0]));

            /* save inlif entry on port */
            rc = cpssExMxPmInlifEntryGet(devNum, CPSS_EXMXPM_INLIF_TYPE_PORT_E,
                                         portNum,
                                         &prvTgfPortUnkSrcMacDropRestoreCfg[devNum][portIdx].inlifEntry);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmInlifEntryGet FAILED, rc = [%d]", rc);

                rc1 = rc;
            }

            /* save inlif lookup mode for the ingress port */
            rc = cpssExMxPmInlifPortModeGet(devNum, (GT_U8) portNum,
                                            &prvTgfPortUnkSrcMacDropRestoreCfg[devNum][portIdx].inlifMode);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmInlifPortModeGet FAILED, rc = [%d]", rc);

                rc1 = rc;
            }

            /* save <Unknown MAC SA Command> for this port to hard drop */
            rc = cpssExMxPmBrgFdbPortUnkUcMacSaCmdGet(devNum, (GT_U8) portNum,
                                                      &prvTgfPortUnkSrcMacDropRestoreCfg[devNum][portIdx].command);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgFdbPortUnkUcMacSaCmdGet FAILED, rc = [%d]", rc);

                rc1 = rc;
            }

            /* set rstStatus to true */
            prvTgfPortUnkSrcMacDropRestoreCfg[devNum][portIdx].rstStatus = GT_TRUE;
        }

        /* clear entry */
        cpssOsMemSet(&inlifEntry, 0, sizeof(inlifEntry));

        inlifEntry.autoLearnEnable = GT_TRUE;
        inlifEntry.bridgeEnable = GT_TRUE;
        inlifEntry.unkSaNotSecurBreachEnable = GT_TRUE;
        inlifEntry.unkSaUcCommand  = CPSS_PACKET_CMD_BRIDGE_E;

        /* set inlif entry on port */
        rc = cpssExMxPmInlifEntrySet(devNum, CPSS_EXMXPM_INLIF_TYPE_PORT_E,
                                     portNum, &inlifEntry);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmInlifEntrySet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }

        /* confgure inlif lookup mode for the ingress port */
        rc = cpssExMxPmInlifPortModeSet(devNum, (GT_U8) portNum,
                                        CPSS_EXMXPM_INLIF_PORT_MODE_PORT_E);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmInlifPortModeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }

        /* set the Inlif's unknown addresses to drop packeets */
        rc = cpssExMxPmInlifUnkUnregAddrCmdSet(devNum, CPSS_EXMXPM_INLIF_TYPE_PORT_E,
                                               (GT_U32) portNum,
                                               CPSS_EXMXPM_INLIF_UNK_UC_SA_E,
                                               CPSS_PACKET_CMD_DROP_HARD_E);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmInlifUnkUnregAddrCmdSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }

        /* set <Unknown MAC SA Command> for this port to hard drop */
        rc = cpssExMxPmBrgFdbPortUnkUcMacSaCmdSet(devNum, (GT_U8) portNum, CPSS_PACKET_CMD_DROP_HARD_E);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgFdbPortUnkUcMacSaCmdSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

    return rc1;
}

/**
* @internal prvTgfBrgPortUnkSrcMacDropInvalidate function
* @endinternal
*
* @brief   Invalidate config after prvTgfBrgPortUnkSrcMacDropSet.
*
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on unknown command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgPortUnkSrcMacDropInvalidate
(
    IN GT_PORT_NUM              portNum
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
    GT_U8       portIdx = 0;
#ifdef CHX_FAMILY
    GT_BOOL                  status;
    CPSS_PORT_LOCK_CMD_ENT   cmd;
#endif

    /* finding portIdx by portNum */
    for (portIdx = 0; portIdx < prvTgfPortsNum; portIdx++)
    {
        if (prvTgfPortsArray[portIdx] == portNum)
        {
            break;
        }
    }

    /* check that portNum isn't out of range */
    if (portIdx == prvTgfPortsNum)
    {
        PRV_UTF_LOG0_MAC("[TGF]: cpssDxChBrgFdbPortLearnStatusGet FAILED");

        return GT_BAD_PARAM;
    }

#ifdef CHX_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* restore default values if such was created */
        if (GT_TRUE == prvTgfPortUnkSrcMacDropRestoreCfg[devNum][portIdx].rstStatus)
        {
            status = prvTgfPortUnkSrcMacDropRestoreCfg[devNum][portIdx].status;
            cmd = prvTgfPortUnkSrcMacDropRestoreCfg[devNum][portIdx].cmd;
            /* restore learning of new source MAC addresses for packets received on specified port */
            rc = cpssDxChBrgFdbPortLearnStatusSet(devNum, portNum, status, cmd);

            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbPortLearnStatusGet FAILED, rc = [%d]", rc);

                rc1 = rc;
            }

            /* restore rstStatus */
            prvTgfPortUnkSrcMacDropRestoreCfg[devNum][portIdx].rstStatus = GT_FALSE;
        }
    }
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* restore default values if such was created */
        if (GT_TRUE == prvTgfPortUnkSrcMacDropRestoreCfg[devNum][portIdx].rstStatus)
        {
            /* restore inlif entry on port */
            rc = cpssExMxPmInlifEntrySet(devNum, CPSS_EXMXPM_INLIF_TYPE_PORT_E,
                                         portNum,
                                         &prvTgfPortUnkSrcMacDropRestoreCfg[devNum][portIdx].inlifEntry);

            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmInlifEntrySet FAILED, rc = [%d]", rc);

                rc1 = rc;
            }

            /* restore inlif lookup mode for the ingress port */
            rc = cpssExMxPmInlifPortModeSet(devNum, (GT_U8) portNum,
                                            prvTgfPortUnkSrcMacDropRestoreCfg[devNum][portIdx].inlifMode);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmInlifPortModeSet FAILED, rc = [%d]", rc);

                rc1 = rc;
            }

            /* restore <Unknown MAC SA Command> for port to hard drop */
            rc = cpssExMxPmBrgFdbPortUnkUcMacSaCmdSet(devNum, (GT_U8) portNum,
                                                      prvTgfPortUnkSrcMacDropRestoreCfg[devNum][portIdx].command);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgFdbPortUnkUcMacSaCmdSet FAILED, rc = [%d]", rc);

                rc1 = rc;
            }

            /* restore rstStatus */
            prvTgfPortUnkSrcMacDropRestoreCfg[devNum][portIdx].rstStatus = GT_FALSE;
        }
    }
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

    return rc1;
}
/**
* @internal prvTgfBrgFdbSecondaryAuMsgBlockGet function
* @endinternal
*
* @brief   The function returns a block (array) of FDB Address Update (AU) messages,
*         the max number of elements defined by the caller --> from the secondary AUQ.
*         The function cpssDxChHwPpPhase2Init configures queue for secondary AU
*         messages. If common FU queue is used then function returns all AU
*         messages in the queue including FU ones.
*         If separate AU queue is used then function returns only AU messages.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number from which AU are taken
* @param[in,out] numOfAuPtr               - (pointer to)max number of AU messages to get
* @param[in,out] numOfAuPtr               - (pointer to)actual number of AU messages that were received
*
* @param[out] auMessagesPtr            - array that holds received AU messages
*                                      pointer is allocated by the caller , with enough memory
*                                      for numOfAuPtr entries that was given as INPUT
*                                      parameter.
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - the action succeeded and there are no more waiting
*                                       AU messages
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbSecondaryAuMsgBlockGet
(
    IN      GT_U8                                    devNum,
    INOUT   GT_U32                                   *numOfAuPtr,
    OUT     CPSS_MAC_UPDATE_MSG_EXT_STC              *auMessagesPtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgFdbSecondaryAuMsgBlockGet(devNum, numOfAuPtr, auMessagesPtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);

    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgFdbAAandTAToCpuSet function
* @endinternal
*
* @brief   Enable/Disable the PP to/from sending an AA and TA address
*         update messages to the CPU.
* @param[in] enable                   -  or disable the message
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or auMsgType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note for multi-port groups device :
*       Whenever applicable it's highly recommended disabling these messages,
*       as enabling this would stress the AUQ with excessive messages
*
*/
GT_STATUS prvTgfBrgFdbAAandTAToCpuSet
(
    IN GT_BOOL                      enable
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgFdbAAandTAToCpuSet(devNum, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbAAandTAToCpuSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(enable);

    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgFdbMessagesQueueManagerInfoGet function
* @endinternal
*
* @brief   debug tool - print info that the CPSS hold about the state of the :
*         primary AUQ,FUQ,secondary AU queue
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note relate to CQ#109715
*
*/
GT_STATUS prvTgfBrgFdbMessagesQueueManagerInfoGet
(
    IN  GT_U8   devNum
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgFdbMessagesQueueManagerInfoGet(devNum);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);

    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfFdbAuWaCheck function
* @endinternal
*
* @brief   check if the device supports the AU queue WA
*
* @param[in] devNum                   - device number
* @param[in] useDoubleAuq             - double AUQ is in use
*
* @retval GT_TRUE                  - the device supports the AU queue WA
* @retval GT_FALSE                 - the device not supports the AU queue WA
*/
GT_BOOL prvTgfFdbAuWaCheck
(
    IN  GT_U8   devNum,
    IN  GT_BOOL useDoubleAuq
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        if(PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
             PRV_CPSS_DXCH_FER_FDB_AUQ_LEARNING_AND_SCANING_DEADLOCK_WA_E) == GT_FALSE)
        {
            /* WA for AUQ is disabled */
            return GT_FALSE;
        }

        if(useDoubleAuq == GT_TRUE &&
           PRV_CPSS_DXCH_PP_MAC(devNum)->moduleCfg.useDoubleAuq == GT_FALSE)
        {
            /* the device not support double AUQ */
            return GT_FALSE;
        }
    }

    return GT_TRUE;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)

    return GT_FALSE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfBrgLoopbackCt10And40Check function
* @endinternal
*
* @brief   Check if the device supports the Source ID, cascading, Vlan Table Write
*         Workaround tests.
*
* @retval GT_TRUE                  - the device supports the Source ID, cascading,
*                                       Vlan Table Write Workaround tests.
* @retval GT_FALSE                 - the device not supports the Source ID, cascading,
*                                       Vlan Table Write Workaround tests.
*/
GT_BOOL prvTgfBrgLoopbackCt10And40Check(
    void
)
{
#ifdef CHX_FAMILY

    if(tgfLoopBackCt10And40Enable == GT_FALSE)
    {
        return GT_FALSE;
    }
    else
    {
        return GT_TRUE;
    }
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)

    return GT_FALSE;
#endif /* !(defined CHX_FAMILY) */
}





/**
* @internal prvTgfBrgFdbSecureAutoLearnSet function
* @endinternal
*
* @brief   Configure secure auto learning mode.
*         If Secure Automatic Learning is enabled, and a packet enabled for
*         automatic learning, then:
*         - If the source MAC address is found in the FDB, and the associated
*         location of the address has changed, then the FDB entry is updated
*         with the new location of the address.
*         - If the FDB entry is modified with the new location of the address,
*         a New Address Update message is sent to the CPU,
*         if enabled to do so on the port and the packet VLAN.
*         - If the source MAC address is NOT found in the FDB, then
*         Secure Automatic Learning Unknown Source command (Trap to CPU,
*         Soft Drop or Hard Drop) is assigned to the packet according to mode
*         input parameter and packet treated as security breach event.
* @param[in] mode                     - Enable/Disable secure automatic learning.
*                                      The command assigned to packet with new SA not found in FDB
*                                      when secure automatic learning enable.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbSecureAutoLearnSet
(
    IN  CPSS_MAC_SECURE_AUTO_LEARN_MODE_ENT   mode
)
{
    GT_U8       devNum  = prvTgfDevNum;
    GT_STATUS   rc, rc1 = GT_OK;

#ifdef CHX_FAMILY

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgFdbSecureAutoLearnSet(devNum, mode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbSecureAutoLearnSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmBrgFdbSecureAutoLearnSet(devNum, mode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgFdbSecureAutoLearnSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

    return rc1;
}


/**
* @internal prvTgfBrgFdbSecureAutoLearnGet function
* @endinternal
*
* @brief   Get secure auto learning mode.
*         If Secure Automatic Learning is enabled, and a packet enabled for
*         automatic learning, then:
*         - If the source MAC address is found in the FDB, and the associated
*         location of the address has changed, then the FDB entry is updated
*         with the new location of the address.
*         - If the FDB entry is modified with the new location of the address,
*         a New Address Update message is sent to the CPU,
*         if enabled to do so on the port and the packet VLAN.
*         - If the source MAC address is NOT found in the FDB, then
*         Secure Automatic Learning Unknown Source command (Trap to CPU,
*         Soft Drop or Hard Drop) is assigned to the packet according to mode
*         input parameter and packet treated as security breach event.
*
* @param[out] modePtr                  - Enabled/Disabled secure automatic learning.
*                                      The command assigned to packet with new SA not found in FDB
*                                      when secure automatic learning enabled.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbSecureAutoLearnGet
(
    OUT CPSS_MAC_SECURE_AUTO_LEARN_MODE_ENT  *modePtr
)
{
    GT_U8       devNum  = prvTgfDevNum;

#ifdef CHX_FAMILY
    return cpssDxChBrgFdbSecureAutoLearnGet(devNum, modePtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    return cpssExMxPmBrgFdbSecureAutoLearnGet(devNum, modePtr);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

}


/**
* @internal prvTgfBrgFdbSecureAutoLearnMovedSaEnableSet function
* @endinternal
*
* @brief   Function prevents learning of moved SA enries. When enabled moved SA
*         address are considered as security breach, and are not learned. Relevant
*         when SecureAutoLearning is enabled (cpssExMxPmBrgFdbSecureAutoLearnSet).
* @param[in] enable                   - GT_TRUE - moved SA addresses are considered as security breach
*                                      GT_FALSE - moved SA addresses learned as usual
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Note: Puma3 device applicable starting from revision B0
*
*/
GT_STATUS prvTgfBrgFdbSecureAutoLearnMovedSaEnableSet
(
    IN  GT_BOOL enable
)
{
#ifdef CHX_FAMILY
    TGF_PARAM_NOT_USED(enable);
    return GT_NOT_SUPPORTED;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

    GT_U8       devNum  = prvTgfDevNum;
    GT_STATUS   rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmBrgFdbSecureAutoLearnMovedSaEnableSet(devNum, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgFdbSecureAutoLearnMovedSaEnableSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;

#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

}

/**
* @internal prvTgfBrgFdbSecureAutoLearnMovedSaEnableGet function
* @endinternal
*
* @brief   The function return learning status of moved SA enries.
*
* @param[out] enablePtr                - (pointer to) status of moved SA addresses
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Note: Puma3 device applicable starting from revision B0
*
*/
GT_STATUS prvTgfBrgFdbSecureAutoLearnMovedSaEnableGet
(
    OUT GT_BOOL *enablePtr
)
{
    GT_U8       devNum  = prvTgfDevNum;

#ifdef CHX_FAMILY
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enablePtr);
    return GT_NOT_SUPPORTED;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    return cpssExMxPmBrgFdbSecureAutoLearnMovedSaEnableGet(devNum, enablePtr);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

}


/**
* @internal prvTgfBrgFdbPortAutoLearnEnableSet function
* @endinternal
*
* @brief   Enable/Disable auto-learning per port.
*
* @param[in] devNum                   - device number
*                                      port    - port number
* @param[in] enable                   - GT_TRUE  -  auto-learning
*                                      GT_FALSE - disable auto-learning
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbPortAutoLearnEnableSet
(
    IN  GT_U8   devNum,
    IN  GT_PORT_NUM   portNum,
    IN  GT_BOOL enable
)
{
    GT_STATUS   rc, rc1 = GT_OK;

#ifdef CHX_FAMILY

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgFdbPortLearnStatusSet(devNum, portNum, enable, CPSS_LOCK_FRWRD_E);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbPortLearnStatusSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmBrgFdbPortAutoLearnEnableSet(devNum, (GT_U8)portNum, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmBrgFdbPortAutoLearnEnableSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

    return rc1;
}

/**
* @internal prvTgfBrgFdbPortAutoLearnEnableGet function
* @endinternal
*
* @brief   Get auto-learning status per port.
*         NOT APPLICABLE DEVICES:
*         None.
*
* @note   APPLICABLE DEVICES:      None.
* @param[in] devNum                   - device number
*                                      port    - port number
*
* @param[out] enablePtr                - pointer to auto-learning status
*                                      GT_TRUE  - auto-learning enabled
*                                      GT_FALSE - auto-learning disabled
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbPortAutoLearnEnableGet
(
    IN  GT_U8   devNum,
    IN  GT_PORT_NUM   portNum,
    IN  GT_BOOL *enablePtr
)
{

#ifdef CHX_FAMILY
    CPSS_PORT_LOCK_CMD_ENT cmd;

    return cpssDxChBrgFdbPortLearnStatusGet(devNum, portNum, enablePtr, &cmd);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    return cpssExMxPmBrgFdbPortAutoLearnEnableGet(devNum, (GT_U8)portNum, enablePtr);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

}


/**
* @internal prvTgfBrgFdbNaToCpuPerPortSet function
* @endinternal
*
* @brief   Enable/disable forwarding a new mac address message to CPU --per port
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number, CPU port
* @param[in] enable                   - If GT_TRUE, forward NA message to CPU.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note function also called from cascade management
*
*/
GT_STATUS prvTgfBrgFdbNaToCpuPerPortSet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
)
{
#ifdef CHX_FAMILY
    return cpssDxChBrgFdbNaToCpuPerPortSet(devNum,portNum,enable);
#else
    return GT_NOT_IMPLEMENTED;
#endif
}

/**
* @internal prvTgfBrgFdbNaToCpuPerPortGet function
* @endinternal
*
* @brief   Get Enable/disable forwarding a new mac address message to CPU --
*         per port.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number, CPU port
*
* @param[out] enablePtr                - If GT_TRUE, NA message is forwarded to CPU.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note function also called from cascade management
*
*/
GT_STATUS prvTgfBrgFdbNaToCpuPerPortGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
{
#ifdef CHX_FAMILY
    return cpssDxChBrgFdbNaToCpuPerPortGet(devNum,portNum,enablePtr);
#else
    return GT_NOT_IMPLEMENTED;
#endif
}


/**
* @internal prvTgfBrgVlanIngressTpidProfileSet function
* @endinternal
*
* @brief   Function sets bitmap of TPID (Tag Protocol ID) per profile .
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] profile                  - TPID profile. (APPLICABLE RANGES: 0..7)
* @param[in] ethMode                  - TAG0/TAG1 selector
* @param[in] tpidBmp                  - bitmap represent 8 entries at the TPID Table
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, profile, ethMode
* @retval GT_OUT_OF_RANGE          - tpidBmp initialized with more then 8 bits
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. For the ingress direction multiple TPID may be used to
*       identify Tag0 and Tag1. Each bit at the bitmap represents one of
*       the 8 entries at the TPID Table.
*       2. bind 'port' to 'profile' using function cpssDxChBrgVlanPortIngressTpidProfileSet
*
*/
GT_STATUS prvTgfBrgVlanIngressTpidProfileSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               profile,
    IN  CPSS_ETHER_MODE_ENT  ethMode,
    IN  GT_U32               tpidBmp
)
{
#ifdef CHX_FAMILY
    PRV_UTF_LOG4_MAC("prvTgfBrgVlanIngressTpidProfileSet(%d,%d,%d,%d) \n",devNum,profile,ethMode,tpidBmp);

    return cpssDxChBrgVlanIngressTpidProfileSet(devNum,profile,ethMode,tpidBmp);
#else
    return GT_NOT_IMPLEMENTED;
#endif
}

/**
* @internal prvTgfBrgVlanIngressTpidProfileGet function
* @endinternal
*
* @brief   Function gts bitmap of TPID (Tag Protocol ID) per profile .
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] profile                  - TPID profile. (APPLICABLE RANGES: 0..7)
* @param[in] ethMode                  - TAG0/TAG1 selector
*
* @param[out] tpidBmpPtr               - (pointer to) bitmap represent 8 entries at the TPID Table
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, profile, ethMode
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanIngressTpidProfileGet
(
    IN  GT_U8                devNum,
    IN  GT_U32               profile,
    IN  CPSS_ETHER_MODE_ENT  ethMode,
    OUT GT_U32               *tpidBmpPtr
)
{
#ifdef CHX_FAMILY
    return cpssDxChBrgVlanIngressTpidProfileGet(devNum,profile,ethMode,tpidBmpPtr);
#else
    return GT_NOT_IMPLEMENTED;
#endif
}


/**
* @internal prvTgfBrgVlanPortIngressTpidProfileSet function
* @endinternal
*
* @brief   Function sets TPID (Tag Protocol ID) profile per ingress port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; .
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number or CPU port.
* @param[in] ethMode                  - TAG0/TAG1 selector
* @param[in] isDefaultProfile         - indication that the profile is the default profile or
*                                      used for re-parse after TTI lookup:
*                                      1. Parse inner Layer2 after TT
*                                      2. Re-parse Layer2 after popping EVB/BPE tag
*                                      3. Parsing of passenger packet
*                                      GT_TRUE  - used as default profile
*                                      GT_FALSE - used for re-parse after TTI lookup.
* @param[in] profile                  - TPID profile. (APPLICABLE RANGE: 0..7)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, ethMode
* @retval GT_OUT_OF_RANGE          - profile > 7
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For setting TPID bmp per profile use function cpssDxChBrgVlanIngressTpidProfileSet
*
*/
GT_STATUS prvTgfBrgVlanPortIngressTpidProfileSet
(
    IN  GT_U8                devNum,
    IN  GT_PORT_NUM          portNum,
    IN  CPSS_ETHER_MODE_ENT  ethMode,
    IN GT_BOOL               isDefaultProfile,
    IN  GT_U32               profile
)
{
#ifdef CHX_FAMILY
    PRV_UTF_LOG5_MAC("prvTgfBrgVlanPortIngressTpidProfileSet(%d,%d,%d,%d,%d) \n",devNum,portNum,ethMode,isDefaultProfile,profile);

    return cpssDxChBrgVlanPortIngressTpidProfileSet(devNum,portNum,ethMode,isDefaultProfile,profile);
#else
    return GT_NOT_IMPLEMENTED;
#endif
}

/**
* @internal prvTgfBrgVlanPortIngressTpidProfileGet function
* @endinternal
*
* @brief   Function gets TPID (Tag Protocol ID) profile per ingress port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; .
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number or CPU port.
* @param[in] ethMode                  - TAG0/TAG1 selector
* @param[in] isDefaultProfile         - indication that the profile is the default profile or
*                                      used for re-parse after TTI lookup:
*                                      1. Parse inner Layer2 after TT
*                                      2. Re-parse Layer2 after popping EVB/BPE tag
*                                      3. Parsing of passenger packet
*                                      GT_TRUE  - used as default profile
*                                      GT_FALSE - used for re-parse after TTI lookup.
*
* @param[out] profilePtr               - (pointer to)TPID profile.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, ethMode
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanPortIngressTpidProfileGet
(
    IN  GT_U8                devNum,
    IN  GT_PORT_NUM          portNum,
    IN  CPSS_ETHER_MODE_ENT  ethMode,
    IN GT_BOOL               isDefaultProfile,
    OUT GT_U32               *profilePtr
)
{
#ifdef CHX_FAMILY
    return cpssDxChBrgVlanPortIngressTpidProfileGet(devNum,portNum,ethMode,isDefaultProfile,profilePtr);
#else
    return GT_NOT_IMPLEMENTED;
#endif
}

/**
* @internal prvTgfBrgVlanPortVid1Set function
* @endinternal
*
* @brief   For ingress direction : Set port's default Vid1.
*         For egress direction : Set port's Vid1 when egress port <VID1 Command>=Enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] direction                - ingress/egress direction
* @param[in] vid1                     - VID1 value, range 0..4095
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanPortVid1Set
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN CPSS_DIRECTION_ENT          direction,
    IN  GT_U16           vid1
)
{
#ifdef CHX_FAMILY
    /*PRV_UTF_LOG4_MAC("prvTgfBrgVlanPortVid1Set : (%d,%d,%d,%d) =======\n" ,
                        devNum ,
                        portNum,
                        direction,
                        vid1);*/
    return cpssDxChBrgVlanPortVid1Set(devNum,portNum,direction,vid1);
#else
    return GT_NOT_IMPLEMENTED;
#endif
}

/**
* @internal prvTgfBrgVlanPortVid1Get function
* @endinternal
*
* @brief   For ingress direction : Get port's default Vid1.
*         For egress direction : Get port's Vid1 when egress port <VID1 Command>=Enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] direction                - ingress/egress direction
*
* @param[out] vid1Ptr                  - (pointer to)VID1 value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanPortVid1Get
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN CPSS_DIRECTION_ENT          direction,
    OUT  GT_U16           *vid1Ptr
)
{
#ifdef CHX_FAMILY
    return cpssDxChBrgVlanPortVid1Get(devNum,portNum,direction,vid1Ptr);
#else
    return GT_NOT_IMPLEMENTED;
#endif
}

/**
* @internal prvTgfBrgVlanMembersTableIndexingModeSet function
* @endinternal
*
* @brief   Set the indexing mode for accessing the VLAN Members table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] direction                - ingress/egress direction
* @param[in] mode                     - table access indexing mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanMembersTableIndexingModeSet
(
    IN  CPSS_DIRECTION_ENT                  direction,
    IN  PRV_TGF_BRG_VLAN_TBL_INDEX_MODE_ENT mode
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
    CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT   dxChMode;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    rc = prvTgfConvertGenericToDxChVlanIndexingMode(mode, &dxChMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChVlanIndexingMode FAILED, rc = [%d]", rc);
        return rc;
    }

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgVlanMembersTableIndexingModeSet(devNum,direction,mode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgVlanMembersTableIndexingModeSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(direction);
    TGF_PARAM_NOT_USED(mode);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgVlanMembersTableIndexingModeGet function
* @endinternal
*
* @brief   Get the indexing mode for accessing the VLAN Members table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] direction                - ingress/egress direction
*
* @param[out] modePtr                  - (pointer to) table access indexing mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanMembersTableIndexingModeGet
(
    IN  CPSS_DIRECTION_ENT                  direction,
    OUT PRV_TGF_BRG_VLAN_TBL_INDEX_MODE_ENT *modePtr
)
{
    GT_U8       devNum  = prvTgfDevNum;

#ifdef CHX_FAMILY
    GT_STATUS                               rc;
    CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT   dxChMode;

    rc = cpssDxChBrgVlanMembersTableIndexingModeGet(devNum,direction,&dxChMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: pssDxChBrgVlanMembersTableIndexingModeGet FAILED, rc = [%d]", rc);
        return rc;
    }

    return prvTgfConvertDxChToGenericVlanIndexingMode(dxChMode,modePtr);
#endif

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(direction);
    TGF_PARAM_NOT_USED(modePtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgVlanStgIndexingModeSet function
* @endinternal
*
* @brief   Set the indexing mode for accessing the VLAN table to fetch span state
*         group.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] direction                - ingress/egress direction
* @param[in] mode                     - table access indexing mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanStgIndexingModeSet
(
    IN  CPSS_DIRECTION_ENT                  direction,
    IN  PRV_TGF_BRG_VLAN_TBL_INDEX_MODE_ENT mode
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
    CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT   dxChMode;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    rc = prvTgfConvertGenericToDxChVlanIndexingMode(mode, &dxChMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChVlanIndexingMode FAILED, rc = [%d]", rc);
        return rc;
    }

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgVlanStgIndexingModeSet(devNum,direction,mode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgVlanStgIndexingModeSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(direction);
    TGF_PARAM_NOT_USED(mode);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgVlanStgIndexingModeGet function
* @endinternal
*
* @brief   Get the indexing mode for accessing the VLAN table to fetch span state
*         group.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] direction                - ingress/egress direction
*
* @param[out] modePtr                  - (pointer to) table access indexing mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanStgIndexingModeGet
(
    IN  CPSS_DIRECTION_ENT                  direction,
    OUT PRV_TGF_BRG_VLAN_TBL_INDEX_MODE_ENT *modePtr
)
{
    GT_U8       devNum  = prvTgfDevNum;

#ifdef CHX_FAMILY
    GT_STATUS                               rc;
    CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT   dxChMode;

    rc = cpssDxChBrgVlanStgIndexingModeGet(devNum,direction,&dxChMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgVlanStgIndexingModeGet FAILED, rc = [%d]", rc);
        return rc;
    }

    return prvTgfConvertDxChToGenericVlanIndexingMode(dxChMode,modePtr);
#endif

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(direction);
    TGF_PARAM_NOT_USED(modePtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgVlanTagStateIndexingModeSet function
* @endinternal
*
* @brief   Set the indexing mode for accessing the VLAN table to fetch tag state.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] mode                     - table access indexing mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanTagStateIndexingModeSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  PRV_TGF_BRG_VLAN_TBL_INDEX_MODE_ENT mode
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc, rc1 = GT_OK;
    CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT   dxChMode;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    rc = prvTgfConvertGenericToDxChVlanIndexingMode(mode, &dxChMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChVlanIndexingMode FAILED, rc = [%d]", rc);
        return rc;
    }

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgVlanTagStateIndexingModeSet(devNum,portNum,mode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgVlanTagStateIndexingModeSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(mode);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgVlanTagStateIndexingModeGet function
* @endinternal
*
* @brief   Get the indexing mode for accessing the VLAN table to fetch tag state.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] modePtr                  - (pointer to) table access indexing mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanTagStateIndexingModeGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT PRV_TGF_BRG_VLAN_TBL_INDEX_MODE_ENT *modePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS                               rc;
    CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT   dxChMode;

    rc = cpssDxChBrgVlanTagStateIndexingModeGet(devNum,portNum,&dxChMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgVlanTagStateIndexingModeGet FAILED, rc = [%d]", rc);
        return rc;
    }

    return prvTgfConvertDxChToGenericVlanIndexingMode(dxChMode,modePtr);
#endif

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(modePtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgGenBypassModeSet function
* @endinternal
*
* @brief   Set Bridge Bypass mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] mode                     - bridge bypass mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant for packets which previously assigned <BridgeBypass>, e.g. by
*       TTI or PCL action.
*
*/
GT_STATUS prvTgfBrgGenBypassModeSet
(
    IN  PRV_TGF_BRG_GEN_BYPASS_MODE_ENT mode
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
    CPSS_DXCH_BRG_GEN_BYPASS_MODE_ENT   dxChMode;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    rc = prvTgfConvertGenericToDxChBrgBypassMode(mode, &dxChMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChBrgBypassMode FAILED, rc = [%d]", rc);
        return rc;
    }

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgGenBypassModeSet(devNum,mode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgGenBypassModeSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(mode);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgGenBypassModeGet function
* @endinternal
*
* @brief   Get Bridge Bypass mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - (pointer to) bridge bypass mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number, mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant for packets which previously assigned <BridgeBypass>, e.g. by
*       TTI or PCL action.
*
*/
GT_STATUS prvTgfBrgGenBypassModeGet
(
    IN  GT_U8                           devNum,
    OUT PRV_TGF_BRG_GEN_BYPASS_MODE_ENT *modePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS                           rc;
    CPSS_DXCH_BRG_GEN_BYPASS_MODE_ENT   dxChMode;

    rc = cpssDxChBrgGenBypassModeGet(devNum,&dxChMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgGenBypassModeGet FAILED, rc = [%d]", rc);
        return rc;
    }

    return prvTgfConvertDxChToGenericBrgBypassMode(dxChMode,modePtr);
#endif

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(modePtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgSecurBreachEventPacketCommandSet function
* @endinternal
*
* @brief   Set for given security breach event it's packet command.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - bad input parameters.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note this function similar to cpssDxChBrgSecurBreachEventDropModeSet , but
*       allow additional packet commands.
*
*/
GT_STATUS prvTgfBrgSecurBreachEventPacketCommandSet
(
    IN CPSS_BRG_SECUR_BREACH_EVENTS_ENT     eventType,
    IN CPSS_PACKET_CMD_ENT                  command
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgSecurBreachEventPacketCommandSet(devNum, eventType, command);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgSecurBreachEventPacketCommandSet, rc = [%d]", rc);
            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(eventType);
    TGF_PARAM_NOT_USED(command);
    return GT_NOT_APPLICABLE_DEVICE;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}


/**
* @internal prvTgfBrgGenCfiRelayEnableSet function
* @endinternal
*
* @brief   Enable / Disable CFI bit relay.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE -  Tagged packets transmitted from the device have
*                                      have their CFI bit set according to received
*                                      packet CFI bit if the received packet was tagged
*                                      or set to 0 if the received packet is untagged.
*                                      - GT_FALSE - Tagged packets transmitted from the device
*                                      have their CFI bit set to 0.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgGenCfiRelayEnableSet
(
    IN GT_U8                          devNum,
    IN GT_BOOL                        enable
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChBrgGenCfiRelayEnableSet(devNum, enable);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enable);
    return GT_NOT_APPLICABLE_DEVICE;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}


/**
* @internal prvTgfBrgGenMtuConfigSet function
* @endinternal
*
* @brief   Enable/disable Target ePort MTU check feature.
*         and configure the following parameters:
*         - the MTU byte-count user mode for target ePort MTU check.
*         - the exception command for frames which fails the MTU check
*         - the CPU/drop code assigned to a frame which fails the MTU check
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] mtuCheckEnable           - target ePort MTU check feature enabled/disabled
*                                      GT_TRUE - MTU check enabled, GT_FALSE - MTU check disabled
* @param[in] byteCountMode            - byte-count mode for MTU
* @param[in] exceptionCommand         command assigned to frames that exceed the
*                                      default ePort MTU size
* @param[in] exceptionCpuCode         CPU/drop code assigned to frame which fails MTU check
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, byteCountMode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgGenMtuConfigSet
(
    IN GT_BOOL                                      mtuCheckEnable,
    IN PRV_TGF_BRG_GEN_BYTE_COUNT_MODE_MTU_ENT      byteCountMode,
    IN CPSS_PACKET_CMD_ENT                          exceptionCommand,
    IN CPSS_NET_RX_CPU_CODE_ENT                     exceptionCpuCode
)
{
#ifdef CHX_FAMILY
    GT_U8                                       devNum  = prvTgfDevNum;
    GT_STATUS                                   rc, rc1 = GT_OK;
    CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_ENT   dxChMode;

    rc = prvTgfConvertGenericToDxChBrgMtuByteCountMode(byteCountMode, &dxChMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChBrgBypassMode FAILED, rc = [%d]", rc);
        return rc;
    }

    /* call device specific API */
    rc = cpssDxChBrgGenMtuConfigSet(devNum, mtuCheckEnable,
                                    dxChMode,
                                    exceptionCommand, exceptionCpuCode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgGenMtuConfigSet FAILED, rc = [%d]", rc);
        rc1 = rc;
    }

    return rc1;
#else
    TGF_PARAM_NOT_USED(mtuCheckEnable);
    TGF_PARAM_NOT_USED(byteCountMode);
    TGF_PARAM_NOT_USED(exceptionCommand);
    TGF_PARAM_NOT_USED(exceptionCpuCode);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfBrgGenMtuConfigGet function
* @endinternal
*
* @brief   Get the target ePort MTU feature parameters:
*         - the enable/disable target ePort MTU check feature.
*         - the MTU byte-count user mode for target ePort MTU check.
*         - the exception command for frames which fails the MTU check
*         - the CPU/drop code assigned to a frame which fails the MTU check
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @param[out] mtuCheckEnablePtr        - (pointer to) enable/disable target ePort MTU feature.
* @param[out] byteCountModePtr         - (pointer to) MTU byte-count mode
* @param[out] exceptionCommandPtr      - (pointer to) exception cmd for frames which
*                                      fails MTU check
* @param[out] exceptionCpuCodePtr      - (pointer to) CPU/drop code assigned to frame
*                                      which fails the MTU check
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number, mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on wrong HW value
*/
GT_STATUS prvTgfBrgGenMtuConfigGet
(
    OUT GT_BOOL                                      *mtuCheckEnablePtr,
    OUT PRV_TGF_BRG_GEN_BYTE_COUNT_MODE_MTU_ENT      *byteCountModePtr,
    OUT CPSS_PACKET_CMD_ENT                          *exceptionCommandPtr,
    OUT CPSS_NET_RX_CPU_CODE_ENT                     *exceptionCpuCodePtr
)
{
#ifdef CHX_FAMILY
    GT_U8                                      devNum  = prvTgfDevNum;
    GT_STATUS                                  rc = GT_OK;
    CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_ENT  dxChMode;

    /* call device specific API */
    rc = cpssDxChBrgGenMtuConfigGet(devNum, mtuCheckEnablePtr,
                                    &dxChMode,
                                    exceptionCommandPtr, exceptionCpuCodePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgGenMtuConfigGet FAILED, rc = [%d]", rc);
        return rc;
    }

    return prvTgfConvertDxChToGenericBrgMtuByteCountMode(dxChMode, byteCountModePtr);
#else
    TGF_PARAM_NOT_USED(mtuCheckEnablePtr);
    TGF_PARAM_NOT_USED(byteCountModePtr);
    TGF_PARAM_NOT_USED(exceptionCommandPtr);
    TGF_PARAM_NOT_USED(exceptionCpuCodePtr);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfBrgGenMtuPortProfileIdxSet function
* @endinternal
*
* @brief   Set MTU profile index for the given target ePort.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] mtuProfileIdx            - index to global MTU entry
*                                      (APPLICABLE RANGES: 0..3).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - mtuProfileIdx out of range
*/
GT_STATUS prvTgfBrgGenMtuPortProfileIdxSet
(
    IN GT_U8         devNum,
    IN GT_PORT_NUM   portNum,
    IN GT_U32        mtuProfileIdx
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChBrgGenMtuPortProfileIdxSet(devNum, portNum, mtuProfileIdx);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgGenMtuPortProfileIdxSet FAILED, rc = [%d]", rc);
    }
    return rc;

#else
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(mtuProfileIdx);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfBrgGenMtuPortProfileIdxGet function
* @endinternal
*
* @brief   Get MTU profile index for the given ePort.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] mtuProfileIdxPtr         - (pointer to)index to the global MTU entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgGenMtuPortProfileIdxGet
(
    IN  GT_U8          devNum,
    IN  GT_PORT_NUM    portNum,
    OUT GT_U32         *mtuProfileIdxPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChBrgGenMtuPortProfileIdxGet(devNum, portNum, mtuProfileIdxPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgGenMtuPortProfileIdxGet FAILED, rc = [%d]", rc);
    }
    return rc;

#else
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(mtuProfileIdxPtr);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfBrgGenMtuExceedCntrGet function
* @endinternal
*
* @brief   Get target ePort MTU exceeded counter.
*         The number of packets that triggered the Target ePort MTU
*         exceeded exception check.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @param[out] valuePtr                 - (pointer to) the value of target ePort MTU exceeded counter.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on error.
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgGenMtuExceedCntrGet
(
    OUT GT_U32       *valuePtr
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
    GT_U32      cntrValue, sumCntrValue = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgGenMtuExceedCntrGet(devNum, &cntrValue);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgGenMtuExceedCntrGet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
        else
            sumCntrValue += cntrValue;

    }

    *valuePtr = sumCntrValue;
    return rc1;
#else
    TGF_PARAM_NOT_USED(valuePtr);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfBrgGenMtuProfileSet function
* @endinternal
*
* @brief   Set an MTU size for profile
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] profile                  - the  (APPLICABLE RANGES: 0..3)
* @param[in] mtuSize                  - max transmit packet size in bytes
*                                      (APPLICABLE RANGES: 0..16383)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range mtuSize
*/
GT_STATUS prvTgfBrgGenMtuProfileSet
(
    IN GT_U32   profile,
    IN GT_U32   mtuSize
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = prvTgfDevNum;
    GT_STATUS   rc, rc1 = GT_OK;

    /* call device specific API */
    rc = cpssDxChBrgGenMtuProfileSet(devNum, profile, mtuSize);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgGenMtuProfileSet FAILED, rc = [%d]", rc);
    }

    return rc1;
#else
    TGF_PARAM_NOT_USED(profile);
    TGF_PARAM_NOT_USED(mtuSize);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfBrgGenMtuProfileGet function
* @endinternal
*
* @brief   Get an MTU size for ePort profile
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] profile                  - the  (APPLICABLE RANGES: 0..3)
*
* @param[out] mtuSizePtr               - (pointer to) max transmit packet size in bytes
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or profile
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfBrgGenMtuProfileGet
(
    IN  GT_U32  profile,
    OUT GT_U32  *mtuSizePtr
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = prvTgfDevNum;
    GT_STATUS   rc, rc1 = GT_OK;

    /* call device specific API */
    rc = cpssDxChBrgGenMtuProfileGet(devNum, profile, mtuSizePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgGenMtuProfileGet FAILED, rc = [%d]", rc);
        rc1 = rc;
    }

    return rc1;
#else
    TGF_PARAM_NOT_USED(profile);
    TGF_PARAM_NOT_USED(mtuSizePtr);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfBrgVlanIpCntlToCpuSet function
* @endinternal
*
* @brief   Enable/disable IP control traffic trapping/mirroring to CPU.
*         When set, this enables the following control traffic to be eligible
*         for mirroring/trapping to the CPU:
*         - ARP
*         - IPv6 Neighbor Solicitation
*         - IPv4/v6 Control Protocols Running Over Link-Local Multicast
*         - RIPv1 MAC Broadcast
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or IP control type
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanIpCntlToCpuSet
(
    IN GT_U16                              vlanId,
    IN PRV_TGF_BRG_IP_CTRL_TYPE_ENT        ipCntrlType
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
    CPSS_DXCH_BRG_IP_CTRL_TYPE_ENT      dxIpCntrlType;

    switch (ipCntrlType)
    {
        case PRV_TGF_BRG_IP_CTRL_NONE_E: dxIpCntrlType = CPSS_DXCH_BRG_IP_CTRL_NONE_E; break;
        case PRV_TGF_BRG_IP_CTRL_IPV4_E: dxIpCntrlType = CPSS_DXCH_BRG_IP_CTRL_IPV4_E; break;
        case PRV_TGF_BRG_IP_CTRL_IPV6_E: dxIpCntrlType = CPSS_DXCH_BRG_IP_CTRL_IPV6_E; break;
        case PRV_TGF_BRG_IP_CTRL_IPV4_IPV6_E: dxIpCntrlType = CPSS_DXCH_BRG_IP_CTRL_IPV4_IPV6_E; break;
        default: return GT_FAIL;
    }
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgVlanIpCntlToCpuSet(devNum, vlanId, dxIpCntrlType);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC(
                  "[TGF]: cpssDxChBrgVlanIpCntlToCpuSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(vlanId);
    TGF_PARAM_NOT_USED(ipCntrlType);
    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgGenArpBcastToCpuCmdSet function
* @endinternal
*
* @brief   Enables trapping or mirroring to CPU ARP Broadcast packets for all VLANs,
*         with with IPv4/IPv6 Control Traffic To CPU Enable set by function
*         prvTgfBrgVlanIpCntlToCpuSet
*         INPUTS:
*         devNum - device number
*         cmdMode - command interface mode:
*         PRV_TGF_BRG_ARP_BCAST_CMD_MODE_PORT_E - will set ARP broadcast command for all ports,
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number, command mode or control packet command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgGenArpBcastToCpuCmdSet
(
    IN PRV_TGF_BRG_ARP_BCAST_CMD_MODE_ENT cmdMode,
    IN CPSS_PACKET_CMD_ENT  cmd
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
    CPSS_DXCH_ARP_BCAST_CMD_MODE_ENT dxCmdMode;

    switch(cmdMode)
    {
        case PRV_TGF_BRG_ARP_BCAST_CMD_MODE_PORT_E:
            dxCmdMode = CPSS_DXCH_ARP_BCAST_CMD_MODE_PORT_E;
            break;
        case PRV_TGF_BRG_ARP_BCAST_CMD_MODE_VLAN_E:
            dxCmdMode = CPSS_DXCH_ARP_BCAST_CMD_MODE_VLAN_E;
            break;
        default:
            return GT_BAD_PARAM;
    }
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgGenArpBcastToCpuCmdSet(devNum, dxCmdMode, cmd);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC(
              "[TGF]: cpssDxChBrgGenArpBcastToCpuCmdSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(cmd);
    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgGenArpBcastToCpuCmdGet function
* @endinternal
*
* @brief   Get status of CPU ARP Broadcast packets for all VLANs
*         INPUTS:
*         devNum - device number
*         cmdMode - command interface mode:
*         PRV_TGF_BRG_ARP_BCAST_CMD_MODE_PORT_E - will set ARP broadcast command for all ports,
*
* @param[out] cmdPtr                   - (pointer to) command
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or control packet command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - wrong hardware value.
*/
GT_STATUS prvTgfBrgGenArpBcastToCpuCmdGet
(
    IN PRV_TGF_BRG_ARP_BCAST_CMD_MODE_ENT cmdMode,
    OUT CPSS_PACKET_CMD_ENT  *cmdPtr
)
{

#ifdef CHX_FAMILY
    GT_U8       devNum  = prvTgfDevNum;
    CPSS_DXCH_ARP_BCAST_CMD_MODE_ENT dxCmdMode;

    switch(cmdMode)
    {
        case PRV_TGF_BRG_ARP_BCAST_CMD_MODE_PORT_E:
            dxCmdMode = CPSS_DXCH_ARP_BCAST_CMD_MODE_PORT_E;
            break;
        case PRV_TGF_BRG_ARP_BCAST_CMD_MODE_VLAN_E:
            dxCmdMode = CPSS_DXCH_ARP_BCAST_CMD_MODE_VLAN_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    return cpssDxChBrgGenArpBcastToCpuCmdGet(devNum, dxCmdMode, cmdPtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(cmdPtr);
    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */
#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgVlanPortNumOfTagWordsToPopSet function
* @endinternal
*
* @brief   Set the number of bytes to pop for traffic that ingress from the port.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] numBytesToPop            - enumeration to set the number of bytes to pop.
*                                      (for popping 0/4/8/6 bytes )
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - when parameter numberOfTagWords is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanPortNumOfTagWordsToPopSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  PRV_TGF_BRG_VLAN_NUM_BYTES_TO_POP_ENT           numBytesToPop
)
{

#ifdef CHX_FAMILY
    GT_STATUS   rc;
    GT_DXCH_BRG_VLAN_NUM_BYTES_TO_POP_ENT   dx_numBytesToPop;

    dx_numBytesToPop = (GT_DXCH_BRG_VLAN_NUM_BYTES_TO_POP_ENT)numBytesToPop;


    /* call device specific API */
    rc = cpssDxChBrgVlanPortNumOfTagWordsToPopSet(
        devNum, portNum, dx_numBytesToPop);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC(
            "[TGF]: cpssDxChBrgVlanPortNumOfTagWordsToPopSet, rc = [%d]", rc);
    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(numBytesToPop);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(numBytesToPop);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}


/**
* @internal prvTgfBrgGenCiscoL2ProtCmdSet function
* @endinternal
*
* @brief   Enables trapping or mirroring to CPU Proprietary Layer 2 Control MC packets.
*
* @param[in] cmd                      - supported commands:
*                                      CPSS_PACKET_CMD_FORWARD_E , CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or control packet command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgGenCiscoL2ProtCmdSet
(
    IN CPSS_PACKET_CMD_ENT  cmd
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgGenCiscoL2ProtCmdSet(devNum, cmd);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC(
              "[TGF]: cpssDxChBrgGenArpBcastToCpuCmdSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(cmd);
    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgGenCiscoL2ProtCmdGet function
* @endinternal
*
* @brief   Get current command referring Proprietary Layer 2 Control MC packets.
*
* @param[out] cmdPtr                   - (pointer to) command
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or control packet command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - wrong hardware value.
*/
GT_STATUS prvTgfBrgGenCiscoL2ProtCmdGet
(
    OUT CPSS_PACKET_CMD_ENT  *cmdPtr
)
{

#ifdef CHX_FAMILY
    GT_U8       devNum  = prvTgfDevNum;
    return cpssDxChBrgGenCiscoL2ProtCmdGet(devNum, cmdPtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(cmdPtr);
    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */
#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}




/**
* @internal prvTgfBrgGenIcmpv6MsgTypeSet function
* @endinternal
*
* @brief   set trapping or mirroring to CPU command for IPv6 ICMP packets,
*         accordong to their ICMP message type.
* @param[in] index                    - the  it the table (0 to 7)
* @param[in] msgType                  - ICMP message type
* @param[in] cmd                      - supported commands:
*                                      CPSS_PACKET_CMD_FORWARD_E , CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or control packet command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgGenIcmpv6MsgTypeSet
(
    IN GT_U32               index,
    IN GT_U8                msgType,
    IN CPSS_PACKET_CMD_ENT  cmd
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgGenIcmpv6MsgTypeSet(devNum,index,msgType,cmd);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC(
              "[TGF]: cpssDxChBrgGenIcmpv6MsgTypeSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(index);
    TGF_PARAM_NOT_USED(msgType);
    TGF_PARAM_NOT_USED(cmd);
    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif

}

/**
* @internal prvTgfBrgGenIcmpv6MsgTypeGet function
* @endinternal
*
* @brief   Get current command referring IPv6 ICMP packets.
*
* @param[in] index                    - the entry  in the table (0 to 7)
*
* @param[out] msgTypePtr               - (pointer to) ICMP message type
* @param[out] cmdPtr                   - (pointer to) command
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or control packet command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - wrong hardware value.
*/
GT_STATUS prvTgfBrgGenIcmpv6MsgTypeGet
(
    IN  GT_U32               index,
    OUT GT_U8                *msgTypePtr,
    OUT CPSS_PACKET_CMD_ENT  *cmdPtr
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = prvTgfDevNum;
    return cpssDxChBrgGenIcmpv6MsgTypeGet(devNum,index,msgTypePtr,cmdPtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(index);
    TGF_PARAM_NOT_USED(msgTypePtr);
    TGF_PARAM_NOT_USED(cmdPtr);
    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */
#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}




/**
* @internal prvTgfBrgVlanIpV6IcmpToCpuEnable function
* @endinternal
*
* @brief   Enables trapping or mirroring to CPU IPv6 ICMP packets.
*
* @param[in] vlanId                   - the vlan ID
* @param[in] enable                   - GT_TRUE / GT_FALSE
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or control packet command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanIpV6IcmpToCpuEnable
(
    IN  GT_U16    vlanId,
    IN  GT_BOOL   enable
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgVlanIpV6IcmpToCpuEnable(devNum,vlanId,enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC(
              "[TGF]: cpssDxChBrgGenIcmpv6MsgTypeSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(vlanId);
    TGF_PARAM_NOT_USED(enable);
    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgVlanMirrorToTxAnalyzerSet function
* @endinternal
*
* @brief   Function sets Tx Analyzer port index to given VLAN.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] vlanId                   - VLAN id
* @param[in] enable                   - Enable/disable Tx Mirroring
*                                      GT_TRUE - enable
*                                      GT_FALSE - disable
* @param[in] txAnalyzerIndex          - Tx Analyzer index
*                                      (APPLICABLE RANGES: 0..6)
*                                      relevant only if mirrToTxAnalyzerEn == GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, vlanId or trafficType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanMirrorToTxAnalyzerSet
(
    IN  GT_U16      vlanId,
    IN  GT_BOOL     enable,
    IN  GT_U32      txAnalyzerIndex
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgVlanMirrorToTxAnalyzerSet(devNum,vlanId,enable,txAnalyzerIndex);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC(
              "[TGF]: cpssDxChBrgVlanMirrorToTxAnalyzerSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(vlanId);
    TGF_PARAM_NOT_USED(enable);
    TGF_PARAM_NOT_USED(txAnalyzerIndex);
    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}


/**
* @internal prvTgfBrgFdbDaLookupAnalyzerIndexSet function
* @endinternal
*
* @brief   Set analyzer index used when the FDB DA lookup has
*         <DA Lookup Ingress Mirror to Analyzer Enable> set.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NOTE: If a previous mechanism assigns a different analyzer index,
*       the higher index wins.
*
*/
GT_STATUS prvTgfBrgFdbDaLookupAnalyzerIndexSet
(
    IN GT_BOOL enable,
    IN GT_U32  index
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgFdbDaLookupAnalyzerIndexSet(devNum, enable, index);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC(
              "[TGF]: cpssDxChBrgFdbDaLookupAnalyzerIndexSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(enable);
    TGF_PARAM_NOT_USED(index);
    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif

}

/**
* @internal prvTgfBrgFdbDaLookupAnalyzerIndexGet function
* @endinternal
*
* @brief   Get analyzer index used when the FDB DA lookup has
*         <DA Lookup Ingress Mirror to Analyzer Enable> set.
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - analyzer index field is NOT ZERO
*                                      GT_FALSE - analyzer index field is set to ZERO
* @param[out] indexPtr                 - (pointer to) Mirror to Analyzer Index (APPLICABLE RANGES: 0..6)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbDaLookupAnalyzerIndexGet
(
    OUT GT_BOOL *enablePtr,
    OUT GT_U32  *indexPtr
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = prvTgfDevNum;
    GT_STATUS   rc;
    rc = cpssDxChBrgFdbDaLookupAnalyzerIndexGet(devNum,enablePtr,indexPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC(
          "[TGF]: cpssDxChBrgFdbDaLookupAnalyzerIndexGet FAILED, rc = [%d]", rc);
    }
    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(enablePtr);
    TGF_PARAM_NOT_USED(indexPtr);
    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */
#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgFdbSaLookupAnalyzerIndexSet function
* @endinternal
*
* @brief   Set analyzer index used when the FDB SA lookup has
*         <SA Lookup Ingress Mirror to Analyzer Enable> set.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NOTE: If a previous mechanism assigns a different analyzer index,
*       the higher index wins.
*
*/
GT_STATUS prvTgfBrgFdbSaLookupAnalyzerIndexSet
(
    IN GT_BOOL enable,
    IN GT_U32  index
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgFdbSaLookupAnalyzerIndexSet(devNum, enable, index);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC(
              "[TGF]: cpssDxChBrgFdbSaLookupAnalyzerIndexSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(enable);
    TGF_PARAM_NOT_USED(index);
    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif

}

/**
* @internal prvTgfBrgFdbSaLookupAnalyzerIndexGet function
* @endinternal
*
* @brief   Get analyzer index used when the FDB SA lookup has
*         <SA Lookup Ingress Mirror to Analyzer Enable> set.
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - analyzer index field is NOT ZERO
*                                      GT_FALSE - analyzer index field is set to ZERO
* @param[out] indexPtr                 - (pointer to) Mirror to Analyzer Index (APPLICABLE RANGES: 0..6)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbSaLookupAnalyzerIndexGet
(
    OUT GT_BOOL *enablePtr,
    OUT GT_U32  *indexPtr
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = prvTgfDevNum;
    GT_STATUS   rc;
    rc = cpssDxChBrgFdbSaLookupAnalyzerIndexGet(devNum,enablePtr,indexPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC(
          "[TGF]: cpssDxChBrgFdbSaLookupAnalyzerIndexGet FAILED, rc = [%d]", rc);
    }
    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(enablePtr);
    TGF_PARAM_NOT_USED(indexPtr);
    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */
#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfBrgFdbVid1AssignmentEnableSet function
* @endinternal
*
* @brief   Enable/Disable using of VID1 in FDB entry.
*         The Source ID bits [8:6], SA Security Level and DA Security Level are read as 0
*         from the FDB entry when VID1 enabled.
* @param[in] enable                   - GT_FALSE: <Tag1 VID> is not written in the FDB and is not read from the FDB.
*                                      <SrcID>[8:6] can be used for src-id filtering and <SA Security Level>
*                                      and <DA Security Level> are written/read from the FDB.
*                                      GT_TRUE:  Tag1 VID> is written in the FDB and read from the FDB
*                                      as described in Mac Based VLAN FS section.
*                                      <SrcID>[8:6], <SA Security Level> and <DA Security Level>
*                                      are read as 0 from the FDB entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbVid1AssignmentEnableSet
(
    IN GT_BOOL          enable
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);
    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
           rc = cpssDxChBrgFdbVid1AssignmentEnableSet(devNum, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbVid1AssignmentEnableSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(enable);
    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgFdbPortVid1LearningEnableSet function
* @endinternal
*
* @brief   Enable/Disable copying Tag1 VID from packet to FDB entry
*         at automatic learning.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE -  copying Tag1 VID from packet to FDB entry,
*                                      GT_FALSE - disable, the Tag1Vid in FDB will be 0
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbPortVid1LearningEnableSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_BOOL          enable
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc, rc1 = GT_OK;

    rc = cpssDxChBrgFdbPortVid1LearningEnableSet(devNum, portNum, enable);
        if (GT_OK != rc)
        {
                PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbPortVid1LearningEnableSet FAILED, rc = [%d]", rc);
                rc1 = rc;
        }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(enable);
    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgFdbMacEntryMuxingModeSet function
* @endinternal
*
* @brief   Set FDB mac entry muxing modes.
*         how the 12 bits that are muxed in the FDB mac entry.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;
*
* @param[in] muxingMode               - one of the muxing modes according to
*                                      CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_ENT
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or muxingMode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbMacEntryMuxingModeSet
(
    IN CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_ENT          muxingMode
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    PRV_UTF_LOG1_MAC("prvTgfBrgFdbMacEntryMuxingModeSet:(muxingMode:%d)\n" ,
                        muxingMode);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);
    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgFdbMacEntryMuxingModeSet(devNum, muxingMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbMacEntryMuxingModeSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(enable);
    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgFdbVid1AssignmentEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable status of using of VID1 in FDB entry.
*         The Source ID bits [8:6], SA Security Level and DA Security Level are read as 0
*         from the FDB entry when VID1 enabled.
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_FALSE: <Tag1 VID> is not written in the FDB and is not read from the FDB.
*                                      <SrcID>[8:6] can be used for src-id filtering and <SA Security Level>
*                                      and <DA Security Level> are written/read from the FDB.
*                                      GT_TRUE:  Tag1 VID> is written in the FDB and read from the FDB
*                                      as described in Mac Based VLAN FS section.
*                                      <SrcID>[8:6], <SA Security Level> and <DA Security Level>
*                                      are read as 0 from the FDB entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbVid1AssignmentEnableGet
(
    OUT GT_BOOL         *enablePtr
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = prvTgfDevNum;
    GT_STATUS   rc;

    rc = cpssDxChBrgFdbVid1AssignmentEnableGet(devNum,enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbVid1AssignmentEnableGet FAILED, rc = [%d]", rc);
    }
    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(enablePtr);
    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgFdbPortVid1LearningEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable status of copying Tag1 VID from packet to FDB entry
*         at automatic learning.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE - enable copying Tag1 VID from packet to FDB entry,
*                                      GT_FALSE - disable, the Tag1Vid in FDB will be 0
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbPortVid1LearningEnableGet
(
    IN    GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    OUT GT_BOOL          *enablePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc, rc1 = GT_OK;

    rc = cpssDxChBrgFdbPortVid1LearningEnableGet(devNum, portNum, enablePtr);
        if (GT_OK != rc)
        {
                PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbPortVid1LearningEnableGet FAILED, rc = [%d]", rc);
                rc1 = rc;
        }
    return rc1;
 #endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(enablePtr);
    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgFdbMacEntryMuxingModeGet function
* @endinternal
*
* @brief   Get FDB mac entry muxing modes.
*         how the 12 bits that are muxed in the FDB mac entry.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;
*
*
* @param[out] muxingModePtr            - (pointer to) one of the muxing modes according to
*                                      CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_ENT
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or muxingMode
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbMacEntryMuxingModeGet
(
    OUT CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_ENT         *muxingModePtr
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = prvTgfDevNum;
    GT_STATUS   rc;

    /* call device specific API */
    rc = cpssDxChBrgFdbMacEntryMuxingModeGet(devNum, muxingModePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbMacEntryMuxingModeGet FAILED, rc = [%d]", rc);
    }
    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(enable);
    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}



/**
* @internal prvTgfBrgFdbHashModeSet function
* @endinternal
*
* @brief   Sets the FDB hash function mode.
*         The CRC based hash function provides the best hash index distribution
*         for random addresses and vlans.
*         The XOR based hash function provides optimal hash index distribution
*         for controlled testing scenarios, where sequential addresses and vlans
*         are often used.
* @param[in] devNum                   - device number
* @param[in] mode                     - hash function based mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbHashModeSet
(
    IN GT_U8                           devNum,
    IN PRV_TGF_MAC_HASH_FUNC_MODE_ENT  mode
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    CPSS_MAC_HASH_FUNC_MODE_ENT dxChHashMode;

    switch(mode)
    {
        case PRV_TGF_MAC_HASH_FUNC_XOR_E:
            dxChHashMode = CPSS_MAC_HASH_FUNC_XOR_E;
            break;
        case PRV_TGF_MAC_HASH_FUNC_CRC_E:
            dxChHashMode = CPSS_MAC_HASH_FUNC_CRC_E;
            break;
        case PRV_TGF_MAC_HASH_FUNC_CRC_MULTI_HASH_E:
            dxChHashMode = CPSS_MAC_HASH_FUNC_CRC_MULTI_HASH_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    rc = cpssDxChBrgFdbHashModeSet(devNum, dxChHashMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbHashModeSet FAILED, rc = [%d]", rc);
    }
    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
        /* avoid warnings */
        TGF_PARAM_NOT_USED(devNum);
        TGF_PARAM_NOT_USED(mode);
        /* call device specific API */
        return GT_NOT_SUPPORTED;
 #endif /* EXMXPM_FAMILY */

 #if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
        return GT_BAD_STATE;
 #endif
}


/**
* @internal prvTgfBrgFdbHashModeGet function
* @endinternal
*
* @brief   Gets the FDB hash function mode.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - hash function based mode.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or mode
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbHashModeGet
(
    IN    GT_U8                            devNum,
    OUT   PRV_TGF_MAC_HASH_FUNC_MODE_ENT   *modePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    CPSS_MAC_HASH_FUNC_MODE_ENT dxChHashMode;
    rc = cpssDxChBrgFdbHashModeGet(devNum, &dxChHashMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbPortVid1LearningEnableGet FAILED, rc = [%d]", rc);
        return rc;
    }
    switch(dxChHashMode)
    {
        case CPSS_MAC_HASH_FUNC_XOR_E:
            *modePtr = PRV_TGF_MAC_HASH_FUNC_XOR_E;
            break;
        case CPSS_MAC_HASH_FUNC_CRC_E:
            *modePtr = PRV_TGF_MAC_HASH_FUNC_CRC_E;
            break;
        case CPSS_MAC_HASH_FUNC_CRC_MULTI_HASH_E:
            *modePtr = PRV_TGF_MAC_HASH_FUNC_CRC_MULTI_HASH_E;
            break;
        default:
            return GT_BAD_STATE;
    }
    return GT_OK;
 #endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(modePtr);
    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}


/**
* @internal prvTgfBrgFdbRoutingNextHopPacketCmdSet function
* @endinternal
*
* @brief   Set Packet Commands for FDB routed packets.
*
* @param[in] devNum                   - device number
* @param[in] nhPacketCmd              - Route entry command.
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbRoutingNextHopPacketCmdSet
(
    IN  GT_U8                       devNum,
    IN  CPSS_PACKET_CMD_ENT         nhPacketCmd
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    rc = cpssDxChBrgFdbRoutingNextHopPacketCmdSet(devNum, nhPacketCmd);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbRoutingNextHopPacketCmdSet FAILED, rc = [%d]", rc);
    }
    return rc;
 #endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(nhPacketCmd);
    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif

}


/**
* @internal prvTgfBrgFdbRoutingNextHopPacketCmdGet function
* @endinternal
*
* @brief   Get Packet Commands for FDB routed packets.
*
* @param[in] devNum                   - device number
*
* @param[out] nhPacketCmdPtr           - (pointer to) Route entry command.
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_VALUE             - wrong hw value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbRoutingNextHopPacketCmdGet
(
    IN  GT_U8                       devNum,
    OUT CPSS_PACKET_CMD_ENT         *nhPacketCmdPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    rc = cpssDxChBrgFdbRoutingNextHopPacketCmdGet(devNum, nhPacketCmdPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbPortVid1LearningEnableGet FAILED, rc = [%d]", rc);
    }
    return rc;
 #endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(nhPacketCmdPtr);
    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}



/**
* @internal prvTgfBrgFdbRoutingPortIpUcEnableSet function
* @endinternal
*
* @brief   Enable/Disable using of FDB for Destination IP lookup for IP UC routing.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] protocol                 - ipv4 or ipv6 or both
* @param[in] enable                   - GT_TRUE  -  using of FDB for DIP lookup for IP UC routing,
*                                      GT_FALSE - disable using of FDB for DIP lookup for IP UC routing
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_STATE             - the setting not allowed in current state of
*                                       configuration
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbRoutingPortIpUcEnableSet
(
    IN  GT_U8                        devNum,
    IN  GT_PORT_NUM                  portNum,
    IN  CPSS_IP_PROTOCOL_STACK_ENT   protocol,
    IN  GT_BOOL                      enable
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    rc = cpssDxChBrgFdbRoutingPortIpUcEnableSet(devNum, portNum, protocol, enable);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbRoutingNextHopPacketCmdSet FAILED, rc = [%d]", rc);
    }
    return rc;
 #endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(protocol);
    TGF_PARAM_NOT_USED(enable);

    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif

}


/**
* @internal prvTgfBrgFdbRoutingPortIpUcEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable state of using of FDB for Destination IP lookup for IP UC routing.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] protocol                 - ipv4 or ipv6
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - enable using of FDB for DIP lookup for IP UC routing,
*                                      GT_FALSE - disable using of FDB for DIP lookup for IP UC routing
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbRoutingPortIpUcEnableGet
(
    IN  GT_U8                        devNum,
    IN  GT_PORT_NUM                  portNum,
    IN  CPSS_IP_PROTOCOL_STACK_ENT   protocol,
    OUT GT_BOOL                      *enablePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    rc = cpssDxChBrgFdbRoutingPortIpUcEnableGet(devNum, portNum, protocol, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbRoutingPortIpUcEnableGet FAILED, rc = [%d]", rc);
    }
    return rc;
 #endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(protocol);
    TGF_PARAM_NOT_USED(enablePtr);
    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgFdbRoutingUcRefreshEnableSet function
* @endinternal
*
* @brief   Enable/Disable refresh for UC Routing entries
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE - Perform refresh for UC Routing entries
*                                      GT_FALSE - Do not perform refresh for UC Routing entries
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbRoutingUcRefreshEnableSet
(
    IN  GT_U8                       devNum,
    IN  GT_BOOL                     enable
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    rc = cpssDxChBrgFdbRoutingUcRefreshEnableSet(devNum, enable);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbRoutingUcRefreshEnableSet FAILED, rc = [%d]", rc);
    }
    return rc;
 #endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enable);
    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgFdbRoutingUcRefreshEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable refresh status for UC Routing entries
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - Perform refresh for UC Routing entries
*                                      GT_FALSE - Do not perform refresh for UC Routing entries
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbRoutingUcRefreshEnableGet
(
    IN  GT_U8                       devNum,
    OUT GT_BOOL                     *enablePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    rc = cpssDxChBrgFdbRoutingUcRefreshEnableGet(devNum, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbRoutingUcRefreshEnableGet FAILED, rc = [%d]", rc);
    }
    return rc;
 #endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enablePtr);
    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgFdbRoutingUcAgingEnableSet function
* @endinternal
*
* @brief   Enable/Disable aging for UC Routing entries
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE  - UC routing entries are examined for aging and
*                                      an AA message is sent out for an entry that is
*                                      aged out. Auto delete is not performed
*                                      GT_FALSE - UC routing entries are not examined for aging
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbRoutingUcAgingEnableSet
(
    IN  GT_U8                       devNum,
    IN  GT_BOOL                     enable
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    rc = cpssDxChBrgFdbRoutingUcAgingEnableSet(devNum, enable);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbRoutingUcAgingEnableSet FAILED, rc = [%d]", rc);
    }
    return rc;
 #endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enable);
    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgFdbRoutingUcAgingEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable aging status for UC Routing entries
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - UC routing entries are examined for aging and
*                                      an AA message is sent out for an entry that is
*                                      aged out. Auto delete is not performed
*                                      GT_FALSE - UC routing entries are not examined for aging
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbRoutingUcAgingEnableGet
(
    IN  GT_U8                       devNum,
    OUT GT_BOOL                     *enablePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    rc = cpssDxChBrgFdbRoutingUcAgingEnableGet(devNum, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbRoutingUcAgingEnableGet FAILED, rc = [%d]", rc);
    }
    return rc;
 #endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enablePtr);
    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgFdbRoutingUcTransplantEnableSet function
* @endinternal
*
* @brief   Enable/Disable transplanting for UC Routing entries
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE  - Perform address transplanting for UC Routing entries
*                                      GT_FALSE - Do not perform address transplanting for UC Routing entries
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbRoutingUcTransplantEnableSet
(
    IN  GT_U8                       devNum,
    IN  GT_BOOL                     enable
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    rc = cpssDxChBrgFdbRoutingUcTransplantEnableSet(devNum, enable);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbRoutingUcTransplantEnableSet FAILED, rc = [%d]", rc);
    }
    return rc;
 #endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enable);
    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgFdbRoutingUcTransplantEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable transplanting status for UC Routing entries
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - Perform address transplanting for UC Routing entries
*                                      GT_FALSE - Do not perform address transplanting for UC Routing entries
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbRoutingUcTransplantEnableGet
(
    IN  GT_U8                       devNum,
    OUT GT_BOOL                     *enablePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    rc = cpssDxChBrgFdbRoutingUcTransplantEnableGet(devNum, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbRoutingUcTransplantEnableGet FAILED, rc = [%d]", rc);
    }
    return rc;
 #endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enablePtr);
    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgFdbRoutingUcDeleteEnableSet function
* @endinternal
*
* @brief   Enable/Disable delete for UC Routing entries
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE  - Perform address deleting for UC Routing entries
*                                      GT_FALSE - Do not perform address deleting for UC Routing entries
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbRoutingUcDeleteEnableSet
(
    IN  GT_U8                       devNum,
    IN  GT_BOOL                     enable
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    rc = cpssDxChBrgFdbRoutingUcDeleteEnableSet(devNum, enable);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbRoutingUcDeleteEnableSet FAILED, rc = [%d]", rc);
    }
    return rc;
 #endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enable);
    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif

}


/**
* @internal prvTgfBrgFdbRoutingUcDeleteEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable delete status for UC Routing entries
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - Perform address deleting for UC Routing entries
*                                      GT_FALSE - Do not perform address deleting for UC Routing entries
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbRoutingUcDeleteEnableGet
(
    IN  GT_U8                       devNum,
    OUT GT_BOOL                     *enablePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    rc = cpssDxChBrgFdbRoutingUcDeleteEnableGet(devNum, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbRoutingUcDeleteEnableGet FAILED, rc = [%d]", rc);
    }
    return rc;
 #endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enablePtr);
    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfBrgFdbRoutingUcAAandTAToCpuSet function
* @endinternal
*
* @brief   The device implements a hardware-based aging or transplanting
*         mechanism. When an entry is aged out or is transplanted, a message can
*         be forwarded to the CPU.
*         This field enables/disables forwarding address aged out messages and
*         transplanted address messages to the Host CPU for UC route FDB entries.
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE  - AA and TA messages are not forwarded to the CPU for
*                                      UC route entries.
*                                      GT_FALSE - AA and TA messages are not forwarded to the CPU for
*                                      UC route entries.
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbRoutingUcAAandTAToCpuSet
(
    IN  GT_U8                       devNum,
    IN  GT_BOOL                     enable
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    rc = cpssDxChBrgFdbRoutingUcAAandTAToCpuSet(devNum, enable);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbRoutingUcAAandTAToCpuSet FAILED, rc = [%d]", rc);
    }
    return rc;
 #endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enable);
    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif

}

/**
* @internal prvTgfBrgFdbRoutingUcAAandTAToCpuGet function
* @endinternal
*
* @brief   The device implements a hardware-based aging or transplanting mechanism
*         When an entry is aged out or is transplanted, a message can be forwarded
*         to the CPU.
*         This field get the enables/disable status of forwarding address aged out
*         messages and transplanted address messages to the Host CPU for UC route FDB entries.
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - AA and TA messages are not forwarded to the CPU for
*                                      UC route entries.
*                                      GT_FALSE - AA and TA messages are not forwarded to the CPU for
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbRoutingUcAAandTAToCpuGet
(
    IN  GT_U8                       devNum,
    OUT GT_BOOL                     *enablePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    rc = cpssDxChBrgFdbRoutingUcAAandTAToCpuGet(devNum, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbRoutingUcAAandTAToCpuGet FAILED, rc = [%d]", rc);
    }
    return rc;
 #endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enablePtr);
    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}



/**
* @internal prvTgfBrgFdbBankCounterUpdate function
* @endinternal
*
* @brief   Update (increment/decrement) the counter of the specific bank
*         The function will fail (GT_BAD_STATE) if the PP is busy with previous
*         update.
*         the application can determine if PP is ready by calling
*         cpssDxChBrgFdbBankCounterUpdateStatusGet.
*         relevant to CPSS_MAC_HASH_FUNC_CRC_MULTI_HASH_E
*         NOTE: this function is needed to be called after calling one of the next:
*         'write by index' - cpssDxChBrgFdbMacEntryWrite ,
*         cpssDxChBrgFdbPortGroupMacEntryWrite
*         'invalidate by index' - cpssDxChBrgFdbMacEntryInvalidate ,
*         cpssDxChBrgFdbPortGroupMacEntryInvalidate
*         The application logic should be:
*         if last action was 'write by index' then :
*         if the previous entry (in the index) was valid --
*         do no call this function.
*         if the previous entry (in the index) was not valid --
*         do 'increment'.
*         if last action was 'invalidate by index' then :
*         if the previous entry (in the index) was valid --
*         do 'decrement'.
*         if the previous entry (in the index) was not valid --
*         do no call this function.
* @param[in] devNum                   - device number
* @param[in] bankIndex                - the index of the bank
*                                      (APPLICABLE RANGES: 0..15)
* @param[in] incOrDec                 - increment or decrement the counter by one.
*                                      GT_TRUE - increment the counter
*                                      GT_FALSE - decrement the counter
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_OUT_OF_RANGE          - bankIndex > 15
* @retval GT_BAD_STATE             - the PP is not ready to get a new update from CPU.
*                                       (PP still busy with previous update)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbBankCounterUpdate
(
    IN GT_U8                        devNum,
    IN GT_U32                       bankIndex,
    IN GT_BOOL                      incOrDec
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    rc = cpssDxChBrgFdbBankCounterUpdate(devNum, bankIndex,incOrDec);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbBankCounterUpdate FAILED, rc = [%d]", rc);
    }
    return rc;
 #endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(bankIndex);
    TGF_PARAM_NOT_USED(incOrDec);
    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif

}

/**
* @internal prvTgfBrgFdbBankCounterValueGet function
* @endinternal
*
* @brief   Get the value of counter of the specific FDB table bank.
*         relevant to CPSS_MAC_HASH_FUNC_CRC_MULTI_HASH_E
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] bankIndex                - the index of the bank
*                                      (APPLICABLE RANGES: 0..15)
*
* @param[out] valuePtr                 - (pointer to)the value of the counter.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbBankCounterValueGet
(
    IN  GT_U8           devNum,
    IN  GT_U32          bankIndex,
    OUT GT_U32          *valuePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
     GT_U32     value;
    rc = cpssDxChBrgFdbBankCounterValueGet(devNum, bankIndex,&value);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbBankCounterValueGet FAILED, rc = [%d]", rc);
    }
    *valuePtr=value;
    return rc;
 #endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(bankIndex);
    TGF_PARAM_NOT_USED(valuePtr);
    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif

}

/**
* @internal prvTgfBrgMcPhysicalSourceFilteringEnableSet function
* @endinternal
*
* @brief   Enable/disable source physical port/trunk filtering for packets that are
*         forwarded to a VIDX target.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - enable/disable filtering
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgMcPhysicalSourceFilteringEnableSet
(
    IN  GT_U8                        devNum,
    IN  GT_PORT_NUM                  portNum,
    IN  GT_BOOL                      enable
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    rc = cpssDxChBrgMcPhysicalSourceFilteringEnableSet(devNum, portNum, enable);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgMcPhysicalSourceFilteringEnableSet FAILED, rc = [%d]", rc);
    }
    return rc;
 #endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(enable);

    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif

}


/**
* @internal prvTgfBrgMcPhysicalSourceFilteringEnableGet function
* @endinternal
*
* @brief   Get the enabling status of source physical port/trunk filtering for
*         packets that are forwarded to a VIDX target.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - filtering enabling status
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgMcPhysicalSourceFilteringEnableGet
(
    IN  GT_U8                        devNum,
    IN  GT_PORT_NUM                  portNum,
    OUT GT_BOOL                      *enablePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    rc = cpssDxChBrgMcPhysicalSourceFilteringEnableGet(devNum, portNum, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgMcPhysicalSourceFilteringEnableGet FAILED, rc = [%d]", rc);
    }
    return rc;
 #endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(enablePtr);
    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}


/**
* @internal prvTgfBrgVlanForceNewDsaToCpuEnableSet function
* @endinternal
*
* @brief   Enable / Disable preserving the original VLAN tag
*         and VLAN EtherType (or tags),
*         and including the new VLAN Id assignment in the TO_CPU DSA tag
*         for tagged packets that are sent to the CPU.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] enable                   - GT_TRUE - Double Tag to CPU: Packets sent to the
*                                      CPU whose Vlan Id was modified by the device,
*                                      with a DSA tag that contains the newly
*                                      assigned Vlan Id and the original Vlan tag
*                                      (or tags) that is preserved after the DSA tag.
*                                      - GT_FALSE - If the original packet contained a Vlan tag,
*                                      it is removed and the DSA tag contains
*                                      the newly assigned Vlan Id.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanForceNewDsaToCpuEnableSet
(
    IN GT_BOOL enable
)
{
#if defined(CHX_FAMILY)
    GT_U8      devNum  = 0;
    GT_STATUS  rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChBrgVlanForceNewDsaToCpuEnableSet(devNum, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPclEgressRxAnalyzerUseOrigVidEnableSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }
    return rc1;
#elif defined(EXMXPM_FAMILY)
    TGF_PARAM_NOT_USED(enable);
    return GT_NOT_SUPPORTED;
#else
    return GT_BAD_STATE;
#endif
}


/**
* @internal prvTgfBrgVlanForceNewDsaToCpuEnableGet function
* @endinternal
*
* @brief   Get Force New DSA To Cpu mode for enabling / disabling
*         preserving the original VLAN tag and VLAN EtherType (or tags),
*         and including the new VLAN Id assignment in the TO_CPU DSA tag
*         for tagged packets that are sent to the CPU.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - pointer to Force New DSA To Cpu mode.
*                                      - GT_TRUE - Double Tag to CPU: Packets sent to the
*                                      CPU whose Vlan Id was modified by the device,
*                                      with a DSA tag that contains the newly
*                                      assigned Vlan Id and the original Vlan tag
*                                      (or tags) that is preserved after the DSA tag.
*                                      - GT_FALSE - If the original packet contained a Vlan tag,
*                                      it is removed and the DSA tag contains
*                                      the newly assigned Vlan Id.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - modePtr is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanForceNewDsaToCpuEnableGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
)
{
#if defined(CHX_FAMILY)
    GT_STATUS rc;

    rc = cpssDxChBrgVlanForceNewDsaToCpuEnableGet(devNum, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgVlanForceNewDsaToCpuEnableGet FAILED, rc = [%d]",
                         rc);
    }
    return rc;
#elif defined(EXMXPM_FAMILY)
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enablePtr);
    return GT_NOT_SUPPORTED;
#else
    return GT_BAD_STATE;
#endif
}


/**
* @internal prvTgfBrgFdbMacBankArrayGet function
* @endinternal
*
* @brief   Get FDB MAC addresses for different banks according to FDB hash mode
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - device number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - modePtr is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbMacBankArrayGet
(
    IN      GT_U8               devNum,
    OUT     TGF_MAC_ADDR        prvTgfSaMacArr[],
    OUT     GT_U32              *numOfBanksPtr
)
{
    PRV_TGF_MAC_ENTRY_KEY_STC   macEntryKey;
    TGF_MAC_ADDR                prvTgfSaMac;
    GT_U32                      hashIindex, hashIindexFirst = 0;
    GT_STATUS                   rc;
    GT_U32                      macCount = 0;
    GT_U32                      numOfBanks;
    GT_U32                      macSaLsb;

#define COPY_MAC_TO_ARRAY(prvTgfSaMacArr, prvTgfSaMac) \
    if (prvTgfSaMacArr == NULL) { \
        cpssOsPrintf("MAC address for index[%d] - [%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x] \n", \
            macCount, \
            prvTgfSaMac[0], \
            prvTgfSaMac[1], \
            prvTgfSaMac[2], \
            prvTgfSaMac[3], \
            prvTgfSaMac[4], \
            prvTgfSaMac[5]);\
    } else { \
        cpssOsMemCpy(prvTgfSaMacArr[macCount], &prvTgfSaMac, sizeof(TGF_MAC_ADDR)); \
    } \
    macCount++

    *numOfBanksPtr = 0;/* incase of return from ERROR */

    macEntryKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntryKey.key.macVlan.vlanId = 1;

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbHashParams.hashMode == CPSS_MAC_HASH_FUNC_CRC_MULTI_HASH_E)
    {
        numOfBanks = PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->bridge.fdbHashParams.numOfBanks;
    }
    else
    {
        numOfBanks = 4;
    }

    macSaLsb = 0;

    while (macCount < numOfBanks)
    {
        macSaLsb++;
        prvTgfSaMac[0] = 0;
        prvTgfSaMac[1] = 0;
        prvTgfSaMac[2] = (macSaLsb >> 24) & 0xFF;
        prvTgfSaMac[3] = (macSaLsb >> 16) & 0xFF;
        prvTgfSaMac[4] = (macSaLsb >> 8) & 0xFF;
        prvTgfSaMac[5] = macSaLsb & 0xFF;

        cpssOsMemCpy(macEntryKey.key.macVlan.macAddr.arEther, prvTgfSaMac, sizeof(TGF_MAC_ADDR));

        if(PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbHashParams.hashMode == CPSS_MAC_HASH_FUNC_CRC_MULTI_HASH_E)
        {
            hashIindex = 0;
        }
        else
        {

            /* calculate the hash index */
            rc = prvTgfBrgFdbMacEntryHashCalc(&macEntryKey, &hashIindex);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfBrgFdbMacEntryIndexFind FAILED, rc = [%d]", rc);

                return rc;
            }
        }

        /* get MAC by first calculated index */
        if (hashIindexFirst == 0)
        {
            hashIindexFirst = hashIindex;
            COPY_MAC_TO_ARRAY(prvTgfSaMacArr, prvTgfSaMac);
        }
        else
        {
            if (hashIindexFirst == hashIindex)
            {
                /* get new MAC by the same calculated index */
                COPY_MAC_TO_ARRAY(prvTgfSaMacArr, prvTgfSaMac);
            }
        }
    }

    *numOfBanksPtr = numOfBanks;

    return GT_OK;
}

/* parameters for restore Vlan entry */
static PRV_TGF_BRG_VLAN_ENTRY_DATA_STC prvTgfVlanEntryStoreCfg;

/**
* @internal prvTgfBrgVlanEntryStore function
* @endinternal
*
* @brief   Store content of VLAN entry in static buffer
*
* @param[in] vid                      - VLAN id
*                                       None
*/
GT_VOID prvTgfBrgVlanEntryStore
(
    GT_U16 vid
)
{
    GT_STATUS                   rc;

    /* save VLAN entry parameters */
    rc = prvTgfBrgVlanEntryRead(prvTgfDevNum, vid,
                                &prvTgfVlanEntryStoreCfg.portsMembers,
                                &prvTgfVlanEntryStoreCfg.portsTagging,
                                &prvTgfVlanEntryStoreCfg.isValid,
                                &prvTgfVlanEntryStoreCfg.vlanInfo,
                                &prvTgfVlanEntryStoreCfg.taggingCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryRead %d", prvTgfDevNum);

    return;
}

/**
* @internal prvTgfBrgVlanEntryRestore function
* @endinternal
*
* @brief   Restore content of VLAN entry in device from static buffer
*
* @param[in] vid                      - VLAN id
*                                       None
*/
GT_VOID prvTgfBrgVlanEntryRestore
(
    GT_U16 vid
)
{
    GT_STATUS   rc;

    /* restore VLAN in HW */
    rc = prvTgfBrgVlanEntryWrite(prvTgfDevNum, vid,
                                &prvTgfVlanEntryStoreCfg.portsMembers,
                                &prvTgfVlanEntryStoreCfg.portsTagging,
                                &prvTgfVlanEntryStoreCfg.vlanInfo,
                                &prvTgfVlanEntryStoreCfg.taggingCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite %d", prvTgfDevNum);

    return;
}

/**
* @internal prvTgfBrgVlanFcoeForwardingEnableSet function
* @endinternal
*
* @brief   Set FCoE Forwarding Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] vlanId                   - VLAN ID which FCoE Forwarding enabled/disabled..
* @param[in] enable                   - FCoE Forwarding Enable
*                                      GT_TRUE - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanFcoeForwardingEnableSet
(
    IN  GT_U16           vlanId,
    IN  GT_BOOL          enable
)
{
    GT_STATUS  rc;

    rc = cpssDxChBrgVlanFcoeForwardingEnableSet(prvTgfDevNum, vlanId, enable);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgVlanFcoeForwardingEnableSet FAILED, rc = [%d]", rc);
    }
    return rc;
}

/**
* @internal prvTgfBrgVlanFcoeForwardingEnableGet function
* @endinternal
*
* @brief   Return FCoE Forwarding Enable status
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @param[out] enablePtr                - (pointer to) FCoE Forwarding Enable
*                                      GT_TRUE - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanFcoeForwardingEnableGet
(
    IN  GT_U16           vlanId,
    OUT GT_BOOL          *enablePtr
)
{
    GT_STATUS   rc;

    rc = cpssDxChBrgVlanFcoeForwardingEnableGet(prvTgfDevNum, vlanId, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgVlanFcoeForwardingEnableGet FAILED, rc = [%d]", rc);
    }
    return rc;
}

