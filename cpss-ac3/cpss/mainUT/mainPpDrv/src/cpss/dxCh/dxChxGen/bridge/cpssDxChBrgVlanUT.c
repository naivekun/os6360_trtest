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
* @file cpssDxChBrgVlanUT.c
*
* @brief Unit tests for cpssDxChBrgVlan, that provides
* CPSS DxCh VLAN facility implementation.
*
* @version   98
********************************************************************************
*/
/* includes */
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgVlan.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/generic/bridge/private/prvCpssBrgVlanTypes.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/* defines */

/* Default valid value for port id */
#define BRG_VLAN_VALID_PHY_PORT_CNS    0

/* Default invalid value for port id */
#define BRG_VLAN_INVALID_PORT_CNS(_devNum)  UTF_CPSS_PP_MAX_PORT_NUM_CNS(_devNum)

/* Tests use this vlan id for testing VLAN functions            */
#define BRG_VLAN_TESTED_VLAN_ID_CNS  100

/* Invalid vlan id for testing VLAN functions*/
#define BRG_VLAN_INVALID_VLAN_ID_CNS 0xAFFF

/* maximal index of analyzer */
#define BRG_VLAN_MIRROR_TO_ANALYZER_MAX_INDEX_CNS 6

/* check if device supports TR101 */
#define IS_TR101_SUPPORTED_MAC(_devNum) \
        (((_devNum) < PRV_CPSS_MAX_PP_DEVICES_CNS) ?                        \
    (PRV_CPSS_DXCH_PP_MAC(_devNum)->fineTuning.featureInfo.tr101Supported != GT_FALSE) : GT_FALSE)

/* value for STG ID out of range testing */
#define MAX_STG_ID_MAC(_devNum)                                             \
        (((_devNum) < PRV_CPSS_MAX_PP_DEVICES_CNS) ?                        \
            (1 + PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_STG_ID_MAC(_devNum)) :     \
            256)

/* not valid value for VIDX */
#define MAX_VIDX_MAC(_devNum)                                            \
        (GT_U16)(((_devNum) < PRV_CPSS_MAX_PP_DEVICES_CNS) ?             \
            (1 + PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_VIDX_MAC(_devNum)) : \
            4096)

/* macro to check that device supports 'range' of physical ports rather then only 'existing physical ports' */
#define IS_E_ARCH_AND_PHYSICAL_PORT_IN_RANGE_MAC(_dev,_port)   \
    (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(_dev) && ((_port) <= (UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(_dev)-1)))

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanEntryWrite
(
    IN GT_U8                        devNum,
    IN GT_U16                       vlanId,
    IN CPSS_PORTS_BMP_STC           *portsMembersPtr,
    IN CPSS_PORTS_BMP_STC           *portsTaggingPtr,
    IN CPSS_DXCH_BRG_VLAN_INFO_STC  *vlanInfoPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanEntryWrite)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with vlanId [100],
                   portsMemberPtr {[0, 16]},
                   portsTaggingPtr {[0, 16]},
                   vlanInfoPtr {unkSrcAddrSecBreach [GT_FALSE],
                                unregNonIpMcastCmd [CPSS_PACKET_CMD_FORWARD_E],
                                unregIpv4McastCmd [CPSS_PACKET_CMD_MIRROR_TO_CPU_E],
                                unregIpv6McastCmd [CPSS_PACKET_CMD_TRAP_TO_CPU_E],
                                unkUcastCmd [CPSS_PACKET_CMD_DROP_HARD_E],
                                unregIpv4BcastCmd [CPSS_PACKET_CMD_DROP_SOFT_E],
                                unregNonIpv4BcastCmd [CPSS_PACKET_CMD_FORWARD_E],
                                ipv4IgmpToCpuEn [GT_TRUE],
                                mirrToRxAnalyzerEn [GT_TRUE],
                                ipv6IcmpToCpuEn[GT_TRUE],
                                ipCtrlToCpuEn [CPSS_DXCH_BRG_IP_CTRL_NONE_E],
                                ipv4IpmBrgMode [CPSS_BRG_IPM_SGV_E],
                                ipv6IpmBrgMode [CPSS_BRG_IPM_GV_E],
                                ipv4IpmBrgEn[GT_TRUE],
                                ipv6IpmBrgEn [GT_TRUE],
                                ipv6SiteIdMode [CPSS_IP_SITE_ID_INTERNAL_E],
                                ipv4UcastRouteEn[GT_FALSE],
                                ipv4McastRouteEn [GT_FALSE],
                                ipv6UcastRouteEn[GT_FALSE],
                                ipv6McastRouteEn[GT_FALSE],
                                stgId[0],
                                autoLearnDisable [GT_FALSE],
                                naMsgToCpuEn [GT_FALSE],
                                mruIdx[0],
                                bcastUdpTrapMirrEn [GT_FALSE],
                                vrfId [0],

                                (next fields relevant Bobcat2; Caelum; Bobcat3 only)
                                mirrToRxAnalyzerIndex[1],
                                mirrToTxAnalyzerEn[GT_TRUE],
                                mirrToTxAnalyzerIndex[2],
                                fidValue[PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_FID_MAC(dev)],
                                unknownMacSaCmd[CPSS_PACKET_CMD_DROP_HARD_E],
                                ipv4McBcMirrToAnalyzerEn[GT_TRUE],
                                ipv4McBcMirrToAnalyzerIndex[3],
                                ipv6McMirrToAnalyzerEn[GT_TRUE],
                                ipv6McMirrToAnalyzerIndex[4]
                                }.
    Expected: GT_OK.
    1.2. Call cpssDxChBrgVlanEntryRead with the same vlanId.
    Expected: GT_OK and the same portsMembersPtr, portsTaggingPtr and vlanInfoPtr.
            Check vlanInfo by fields:
            1) skip ipv6SiteIdMode, ipv4McastRateEn,
                ipv6McastRateEn, autoLearnDisable, naMsgToCpuEn, mruIdx, bcastUdpTrapMirrEn
                fields for Cheetah, but they should be checked for DxCh2, DxCh3;
            2) ipv4IpmBrgMode and ipv6IpnBrgMode should be checked only if
                corresponding ipv4/6IpmBrgEn = GT_TRUE.
            3) for Bobcat2; Caelum; Bobcat3 check also mirrToRxAnalyzerIndex, mirrToTxAnalyzerEn,
                mirrToTxAnalyzerIndex, fidValue, unknownMacSaCmd,
                ipv4McBcMirrToAnalyzerEn, ipv4McBcMirrToAnalyzerIndex,
                ipv6McMirrToAnalyzerEn, ipv6McMirrToAnalyzerIndex.
    1.3. Call with out of range ipv4IpmBrgMode,
                   ipv4IpmBrgEn [GT_FALSE] (in this case ipv4IpmBrgMode is not relevant)
                   and other parameters from 1.1.
    Expected: GT_OK.
    1.4. Call with out of range ipv4IpmBrgMode,
                   ipv4IpmBrgEn [GT_TRUE] (in this case ipv4IpmBrgMode is relevant)
                   and other parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.5. Call with out of range ipv6IpmBrgMode,
                   ipv6IpmBrgEn [GT_FALSE] (in this case ipv6IpmBrgMode is not relevant)
                   and other parameters from 1.1.
    Expected: GT_OK.
    1.6. Call with out of range ipv6IpmBrgMode,
                   ipv6IpmBrgEn [GT_TRUE] (in this case ipv6IpmBrgMode is relevant)
                   and other parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.7. Call with ipCtrlToCpuEn [CPSS_DXCH_BRG_IP_CTRL_IPV4_E]
                   and other parameters from 1.1.
    Expected: GT_OK for DxCh2, DxCh3 and NOT GT_OK for others.
    1.8. Call with ipCtrlToCpuEn [CPSS_DXCH_BRG_IP_CTRL_IPV6_E]
                   and other parameters from 1.1.
    Expected: GT_OK for DxCh2, DxCh3 and NOT GT_OK for others.
    1.9. Call with out of range vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)]
                   and other parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.10. Call with out of range unregNonIpMcastCmd
                    and other parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.11. Call with out of range unregIpv4McastCmd
                    and other parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.12. Call with out of range unregIpv6McastCmd
                    and other parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.13. Call with out of range unkUcastCmd
                    and other parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.14. Call with out of range unregIpv4BcastCmd
                    and other parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.15. Call with out of range unregNonIpv4BcastCmd
                    and other parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.16. Call with out of range ipCtrlToCpuEn
                    and other parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.17. Call with out of range ipv6SiteIdMode
                    and other parameters from 1.1.
    Expected: GT_BAD_PARAM for DxCh2, DxCh3 and GT_OK for others
                (supported only by Cheetah2,3).
    1.18. Call with vlanId [100]
                    and portsMembersPtr [NULL]
                    and other parameters from 1.1.
    Expected: GT_BAD_PTR.
    1.19. Call with vlanId [100]
                    and portsTaggingPtr [NULL]
                    and other parameters from 1.1.
    Expected: GT_BAD_PTR.
    1.20. Call with vlanId [100]
                    and vlanInfoPtr [NULL]
                    and other parameters from 1.1.
    Expected: GT_BAD_PTR.
    1.21. Call with out of range stgId [256]
                    and other parameters from 1.1.
    Expected: NON GT_OK.
    1.22. Call with mruIdx [PRV_CPSS_DXCH_BRG_MRU_INDEX_MAX_CNS+1=8]
          (no constraints by contract) and other parameters from 1.1.
    Expected: GT_OK.
    1.23. Call with out of range mirrToRxAnalyzerIndex[7] and other parameters from 1.1.
    Expected: NON GT_OK for Bobcat2; Caelum; Bobcat3 and GT_OK for others.

    1.24. Call with mirrToTxAnalyzerEn[GT_FALSE],
        out of range mirrToTxAnalyzerIndex[7](not relevant) and
        other parameters from 1.1.
    Expected: GT_OK.

    1.25. Call with mirrToTxAnalyzerEn[GT_TRUE],
        out of range mirrToTxAnalyzerIndex[7](is relevant) and other parameters from 1.1.
    Expected: NON GT_OK for Bobcat2; Caelum; Bobcat3 and GT_OK for others.

    1.26. Call with out of range fidValue[PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_FID_MAC(dev)+1]
        and other parameters from 1.1.
    Expected: NON GT_OK for Bobcat2; Caelum; Bobcat3 and GT_OK for others.

    1.27. Call with out of range unknownMacSaCmd[0x5AAAAAA5] and other parameters from 1.1.
    Expected: NON GT_OK for Bobcat2; Caelum; Bobcat3 and GT_OK for others.

    1.28. Call with ipv4McBcMirrToAnalyzerEn[GT_FALSE],
        out of range ipv4McBcMirrToAnalyzerIndex[7](not relevant) and
        other parameters from 1.1.
    Expected: GT_OK.

    1.29. Call with ipv4McBcMirrToAnalyzerEn[GT_TRUE](is relevant),
        out of range ipv4McBcMirrToAnalyzerIndex[7] and
        other parameters from 1.1.
    Expected: NON GT_OK for Bobcat2; Caelum; Bobcat3 and GT_OK.

    1.30. Call with ipv6McMirrToAnalyzerEn[GT_FALSE],
        out of range ipv6McMirrToAnalyzerIndex[7](not relevant) and
        other parameters from 1.1.
    Expected: GT_OK.

    1.31. Call with ipv6McMirrToAnalyzerEn[GT_TRUE],
        out of range ipv6McMirrToAnalyzerIndex[7](is relevant) and
        other parameters from 1.1.
    Expected: NON GT_OK for Bobcat2; Caelum; Bobcat3 and GT_OK for others.

    1.32. Call with out of range vrfId [4096] and other parameters from 1.1.
    Expected: NON GT_OK for DxCh3 and GT_OK for others (supported only by Cheetah3).
    1.33. Call function with
             fcoeForwardingEn [GT_FALSE / GT_ TRUE],
             unregIpmEVidxMode
             [ CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_BOTH_E /
               CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_IPV4_E /
               CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_IPV6_E /
               CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_BOTH_IPV6_INCREMENT_E],
             unregIpmEVidx [0 / 0xA5A5 / BIT_16-1]
             and other parameters from 1.1.
    Expected: GT_OK for Bobcat2; Caelum; Bobcat3.
    1.34. Call function with out of range unregIpmEVidx [BIT_16],
             fcoeForwardingEn [GT_FALSE] and
             unregIpmEVidxMode [CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_BOTH_E]
             and other parameters from 1.1.
    Expected: NOT GT_OK for Bobcat2; Caelum; Bobcat3.
    1.35. Call function with out of range unregIpmEVidxMode,
                                          unregIpmEVidx [0],
                                          fcoeForwardingEn [GT_FALSE]
                                          and other parameters from 1.1.
    Expected: GT_BAD_PARAM for Bobcat2; Caelum; Bobcat3.
    1.36. Call function with fdbLookupKeyMode = CPSS_DXCH_BRG_FDB_LOOKUP_KEY_MODE_FID_VID1_E
    Expected: GT_OK for SIP 5.10 devices (bobcat2 b0)
    1.37. Call cpssDxChBrgVlanEntryInvalidate with the same vlanId to invalidate all changes.
    Expected: GT_OK.
*/
    GT_STATUS   st     = GT_OK;
    GT_U32      index  = 0;

    GT_U8                       dev;
    GT_U16                      vlanId = 0;
    CPSS_PORTS_BMP_STC          portsMembers;
    CPSS_PORTS_BMP_STC          portsTagging;
    CPSS_DXCH_BRG_VLAN_INFO_STC vlanInfo;
    CPSS_PORTS_BMP_STC          portsMembersGet;
    CPSS_PORTS_BMP_STC          portsTaggingGet;
    CPSS_DXCH_BRG_VLAN_INFO_STC vlanInfoGet;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC  portsTaggingCmd;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC  *portsTaggingCmdPtr = NULL;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC  portsTaggingCmdGet;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC  *portsTaggingCmdGetPtr;
    GT_BOOL                     isValid   = GT_FALSE;
    GT_BOOL                     isEqual   = GT_FALSE;
    CPSS_PP_FAMILY_TYPE_ENT     devFamily = CPSS_PP_FAMILY_CHEETAH_E;
    cpssOsBzero((GT_VOID*) &vlanInfo, sizeof(vlanInfo));
    cpssOsBzero((GT_VOID*) &portsTaggingCmd, sizeof(portsTaggingCmd));
    cpssOsBzero((GT_VOID*) &portsMembersGet, sizeof(portsMembersGet));
    cpssOsBzero((GT_VOID*) &portsTaggingGet, sizeof(portsTaggingGet));
    cpssOsBzero((GT_VOID*) &vlanInfoGet, sizeof(vlanInfoGet));
    cpssOsBzero((GT_VOID*) &portsTaggingCmdGet, sizeof(portsTaggingCmdGet));

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Getting device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /*
            1.1. Call with vlanId [100],
                          portsMemberPtr {[0, 16]},
                          portsTaggingPtr {[0, 16]},
                          vlanInfoPtr {unkSrcAddrSecBreach [GT_FALSE],
                                unregNonIpMcastCmd [CPSS_PACKET_CMD_FORWARD_E],
                                unregIpv4McastCmd [CPSS_PACKET_CMD_MIRROR_TO_CPU_E],
                                unregIpv6McastCmd [CPSS_PACKET_CMD_TRAP_TO_CPU_E],
                                unkUcastCmd [CPSS_PACKET_CMD_DROP_HARD_E],
                                unregIpv4BcastCmd [CPSS_PACKET_CMD_DROP_SOFT_E],
                                unregNonIpv4BcastCmd [CPSS_PACKET_CMD_FORWARD_E],
                                ipv4IgmpToCpuEn [GT_TRUE],
                                mirrToRxAnalyzerEn [GT_TRUE],
                                ipv6IcmpToCpuEn[GT_TRUE],
                                ipCtrlToCpuEn [CPSS_DXCH_BRG_IP_CTRL_NONE_E],
                                ipv4IpmBrgMode [CPSS_BRG_IPM_SGV_E],
                                ipv6IpmBrgMode [CPSS_BRG_IPM_GV_E],
                                ipv4IpmBrgEn[GT_TRUE],
                                ipv6IpmBrgEn [GT_TRUE],
                                ipv6SiteIdMode [CPSS_IP_SITE_ID_INTERNAL_E],
                                ipv4UcastRouteEn[GT_FALSE],
                                ipv4McastRouteEn [GT_FALSE],
                                ipv6UcastRouteEn[GT_FALSE],
                                ipv6McastRouteEn[GT_FALSE],
                                stgId[0],
                                autoLearnDisable [GT_FALSE],
                                naMsgToCpuEn [GT_FALSE],
                                mruIdx[0],
                                bcastUdpTrapMirrEn [GT_FALSE],

                                (next fields relevant Bobcat2; Caelum; Bobcat3 only)
                                mirrToRxAnalyzerIndex[1],
                                mirrToTxAnalyzerEn[GT_TRUE],
                                mirrToTxAnalyzerIndex[2],
                                fidValue[PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_FID_MAC(dev)],
                                unknownMacSaCmd[CPSS_PACKET_CMD_DROP_HARD_E],
                                ipv4McBcMirrToAnalyzerEn[GT_FALSE],
                                ipv4McBcMirrToAnalyzerIndex[3],
                                ipv6McMirrToAnalyzerEn[GT_TRUE],
                                ipv6McMirrToAnalyzerIndex[4]
                                }.
            Expected: GT_OK.
        */
        vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;

        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);
        portsMembers.ports[0] = 17;

        /* portsTagging relevant for TR101 disabled devices */
        portsTagging.ports[0] = 17;

        if (IS_TR101_SUPPORTED_MAC(dev))
        {
            /* portsTaggingCmd relevant for TR101 enabled devices */
            portsTaggingCmd.portsCmd[0] = CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E;
            portsTaggingCmd.portsCmd[1] = CPSS_DXCH_BRG_VLAN_PORT_TAG1_CMD_E;
            portsTaggingCmd.portsCmd[2] = CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E;
            portsTaggingCmd.portsCmd[3] = CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E;
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            {
                portsTaggingCmd.portsCmd[8] = CPSS_DXCH_BRG_VLAN_PORT_DO_NOT_MODIFY_TAG_CMD_E;
            }
            else
            {
                portsTaggingCmd.portsCmd[8] = CPSS_DXCH_BRG_VLAN_PORT_PUSH_TAG0_CMD_E;
            }
            portsTaggingCmd.portsCmd[9] = CPSS_DXCH_BRG_VLAN_PORT_POP_OUTER_TAG_CMD_E;
            portsTaggingCmd.portsCmd[17] = CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E;
            portsTaggingCmd.portsCmd[22] = CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E;

            /* port tagging CMD is used */
            portsTaggingCmdPtr = &portsTaggingCmd;
            portsTaggingCmdGetPtr = &portsTaggingCmdGet;
        }
        else
        {
            /* port tagging CMD should be ignored */
            CPSS_COVERITY_NON_ISSUE_BOOKMARK
            /* coverity[assign_zero] */
            portsTaggingCmdPtr = portsTaggingCmdGetPtr = NULL;
        }

        vlanInfo.unkSrcAddrSecBreach  = GT_FALSE;
        vlanInfo.unregNonIpMcastCmd   = CPSS_PACKET_CMD_FORWARD_E;
        vlanInfo.unregIpv4McastCmd    = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
        vlanInfo.unregIpv6McastCmd    = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
        vlanInfo.unkUcastCmd          = CPSS_PACKET_CMD_DROP_HARD_E;
        vlanInfo.unregIpv4BcastCmd    = CPSS_PACKET_CMD_DROP_SOFT_E;
        vlanInfo.unregNonIpv4BcastCmd = CPSS_PACKET_CMD_FORWARD_E;
        vlanInfo.ipv4IgmpToCpuEn      = GT_TRUE;
        vlanInfo.mirrToRxAnalyzerEn   = GT_TRUE;
        vlanInfo.ipv6IcmpToCpuEn      = GT_TRUE;
        vlanInfo.ipCtrlToCpuEn        = CPSS_DXCH_BRG_IP_CTRL_NONE_E;
        vlanInfo.ipv4IpmBrgMode       = CPSS_BRG_IPM_SGV_E;
        vlanInfo.ipv6IpmBrgMode       = CPSS_BRG_IPM_GV_E;
        vlanInfo.ipv4IpmBrgEn         = GT_TRUE;
        vlanInfo.ipv6IpmBrgEn         = GT_TRUE;
        vlanInfo.ipv6SiteIdMode       = CPSS_IP_SITE_ID_INTERNAL_E;
        vlanInfo.ipv4UcastRouteEn     = GT_FALSE;
        vlanInfo.ipv4McastRouteEn     = GT_FALSE;
        vlanInfo.ipv6UcastRouteEn     = GT_FALSE;
        vlanInfo.ipv6McastRouteEn     = GT_FALSE;
        vlanInfo.stgId                = 17;
        vlanInfo.autoLearnDisable     = GT_FALSE;
        vlanInfo.naMsgToCpuEn         = GT_FALSE;
        vlanInfo.mruIdx               = 0;
        vlanInfo.bcastUdpTrapMirrEn   = GT_FALSE;
        vlanInfo.vrfId                = 100;
        vlanInfo.mcastLocalSwitchingEn = GT_TRUE;
        vlanInfo.portIsolationMode    = CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_L3_CMD_E;

        if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            vlanInfo.mirrToRxAnalyzerIndex = 1;
            vlanInfo.mirrToTxAnalyzerEn = GT_TRUE;
            vlanInfo.mirrToTxAnalyzerIndex = 2;
            vlanInfo.mirrToTxAnalyzerMode = CPSS_DXCH_MIRROR_EGRESS_TAIL_DROP_E;
            vlanInfo.fidValue = UTF_CPSS_PP_MAX_FID_NUM_CNS(dev) - 1;
            vlanInfo.unknownMacSaCmd = CPSS_PACKET_CMD_DROP_HARD_E;
            vlanInfo.ipv4McBcMirrToAnalyzerEn = GT_TRUE;
            vlanInfo.ipv4McBcMirrToAnalyzerIndex = 3;
            vlanInfo.ipv6McMirrToAnalyzerEn = GT_TRUE;
            vlanInfo.ipv6McMirrToAnalyzerIndex = 4;
        }

        CPSS_COVERITY_NON_ISSUE_BOOKMARK
        /* coverity[var_deref_model] */
        st = cpssDxChBrgVlanEntryWrite(dev, vlanId, &portsMembers,
                                       &portsTagging, &vlanInfo, portsTaggingCmdPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, vlanId);

        /*
            1.2. Call cpssDxChBrgVlanEntryRead with the same vlanId.
            Expected: GT_OK and the same portsMembersPtr, portsTaggingPtr and vlanInfoPtr.
                    Check vlanInfo by fields:
                    1) skip ipv6SiteIdMode, ipv4McastRateEn,
                            ipv6McastRateEn, autoLearnDisable, naMsgToCpuEn,
                            mruIdx, bcastUdpTrapMirrEn fields for Cheetah,
                            but they should be checked for DxCh2, DxCh3;
                    2) ipv4IpmBrgMode and ipv6IpnBrgMode should be
                            checked only if  corresponding
                            ipv4/6IpmBrgEn = GT_TRUE.
                    3) for Bobcat2; Caelum; Bobcat3 check also mirrToRxAnalyzerIndex, mirrToTxAnalyzerEn,
                            mirrToTxAnalyzerIndex, fidValue, unknownMacSaCmd,
                            ipv4McBcMirrToAnalyzerEn, ipv4McBcMirrToAnalyzerIndex,
                            ipv6McMirrToAnalyzerEn, ipv6McMirrToAnalyzerIndex.
        */
        CPSS_COVERITY_NON_ISSUE_BOOKMARK
        /* coverity[var_deref_model] */ st = cpssDxChBrgVlanEntryRead(dev, vlanId, &portsMembersGet, &portsTaggingGet,
                                                                      &vlanInfoGet, &isValid, portsTaggingCmdGetPtr);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgVlanEntryRead: %d, %d", dev, vlanId);

        UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isValid,
                "Vlan entry is NOT valid: %d, %d", dev, vlanId);

        if (GT_TRUE == isValid)
        {
            /* Verifying portsMembersPtr */
            isEqual = (0 == cpssOsMemCmp((GT_VOID*) &portsMembers,
                                         (GT_VOID*) &portsMembersGet,
                                         sizeof(portsMembers))) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
                       "get another portsMembersPtr than was set: %d, %d", dev, vlanId);

            if (IS_TR101_SUPPORTED_MAC(dev))
            {
                /* Verifying portsTaggingCmd */
                isEqual = (0 == cpssOsMemCmp((GT_VOID*) &portsTaggingCmd,
                                             (GT_VOID*) &portsTaggingCmdGet,
                                             sizeof(portsTaggingCmd))) ? GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
                           "get another portsTaggingCmd than was set: %d, %d", dev, vlanId);
            }
            else
            {
                /* Verifying portsTaggingPtr */
                isEqual = (0 == cpssOsMemCmp((GT_VOID*) &portsTagging,
                                             (GT_VOID*) &portsTaggingGet,
                                             sizeof(portsTagging))) ? GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
                           "get another portsTaggingPtr than was set: %d, %d", dev, vlanId);
            }

            /* Verifying vlanInfoPtr fields */
            UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.unkSrcAddrSecBreach,
                                         vlanInfoGet.unkSrcAddrSecBreach,
             "get another vlanInfoPtr->unkSrcAddrSecBreach than was set: %d, %d", dev, vlanId);
            UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.unregNonIpMcastCmd,
                                         vlanInfoGet.unregNonIpMcastCmd,
             "get another vlanInfoPtr->unregNonIpMcastCmd than was set: %d, %d", dev, vlanId);
            UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.unregIpv4McastCmd,
                                         vlanInfoGet.unregIpv4McastCmd,
             "get another vlanInfoPtr->unregIpv4McastCmd than was set: %d, %d", dev, vlanId);
            UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.unregIpv6McastCmd,
                                         vlanInfoGet.unregIpv6McastCmd,
             "get another vlanInfoPtr->unregIpv6McastCmd than was set: %d, %d", dev, vlanId);
            UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.unkUcastCmd,
                                         vlanInfoGet.unkUcastCmd,
             "get another vlanInfoPtr->unkUcastCmd than was set: %d, %d", dev, vlanId);
            UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.unregIpv4BcastCmd,
                                         vlanInfoGet.unregIpv4BcastCmd,
             "get another vlanInfoPtr->unregIpv4BcastCmd than was set: %d, %d", dev, vlanId);
            UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.unregNonIpv4BcastCmd,
                                         vlanInfoGet.unregNonIpv4BcastCmd,
             "get another vlanInfoPtr->unregNonIpv4BcastCmd than was set: %d, %d", dev, vlanId);
            UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.ipv4IgmpToCpuEn,
                                         vlanInfoGet.ipv4IgmpToCpuEn,
             "get another vlanInfoPtr->ipv4IgmpToCpuEn than was set: %d, %d", dev, vlanId);
            UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.mirrToRxAnalyzerEn,
                                         vlanInfoGet.mirrToRxAnalyzerEn,
             "get another vlanInfoPtr->mirrToRxAnalyzerEn than was set: %d, %d", dev, vlanId);
            UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.ipv6IcmpToCpuEn,
                                         vlanInfoGet.ipv6IcmpToCpuEn,
             "get another vlanInfoPtr->ipv6IcmpToCpuEn than was set: %d, %d", dev, vlanId);
            UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.ipCtrlToCpuEn,
                                         vlanInfoGet.ipCtrlToCpuEn,
             "get another vlanInfoPtr->ipCtrlToCpuEn than was set: %d, %d", dev, vlanId);
            UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.ipv4IpmBrgEn,
                                         vlanInfoGet.ipv4IpmBrgEn,
             "get another vlanInfoPtr->ipv4IpmBrgEn than was set: %d, %d", dev, vlanId);
            UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.ipv6IpmBrgEn,
                                         vlanInfoGet.ipv6IpmBrgEn,
             "get another vlanInfoPtr->ipv6IpmBrgEn than was set: %d, %d", dev, vlanId);

            UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.ipv4IpmBrgMode,
                                         vlanInfoGet.ipv4IpmBrgMode,
             "get another vlanInfoPtr->ipv4IpmBrgMode than was set: %d, %d", dev, vlanId);

            UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.ipv6IpmBrgMode,
                                         vlanInfoGet.ipv6IpmBrgMode,
             "get another vlanInfoPtr->ipv6IpmBrgMode than was set: %d, %d", dev, vlanId);

            UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.ipv4UcastRouteEn,
                                         vlanInfoGet.ipv4UcastRouteEn,
             "get another vlanInfoPtr->ipv4UcastRouteEn than was set: %d, %d", dev, vlanId);
            UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.ipv6UcastRouteEn,
                                         vlanInfoGet.ipv6UcastRouteEn,
             "get another vlanInfoPtr->ipv6UcastRouteEn than was set: %d, %d", dev, vlanId);
            UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.stgId,
                                         vlanInfoGet.stgId,
             "get another vlanInfoPtr->stgId than was set: %d, %d", dev, vlanId);

            if (devFamily != CPSS_PP_FAMILY_CHEETAH_E)
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.ipv6SiteIdMode,
                                             vlanInfoGet.ipv6SiteIdMode,
                 "get another vlanInfoPtr->ipv6SiteIdMode than was set: %d, %d", dev, vlanId);
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.ipv4McastRouteEn,
                                             vlanInfoGet.ipv4McastRouteEn,
                 "get another vlanInfoPtr->ipv4McastRouteEn than was set: %d, %d", dev, vlanId);
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.ipv6McastRouteEn,
                                             vlanInfoGet.ipv6McastRouteEn,
                 "get another vlanInfoPtr->ipv6McastRouteEn than was set: %d, %d", dev, vlanId);
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.autoLearnDisable,
                                             vlanInfoGet.autoLearnDisable,
                 "get another vlanInfoPtr->autoLearnDisable than was set: %d, %d", dev, vlanId);
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.naMsgToCpuEn,
                                             vlanInfoGet.naMsgToCpuEn,
                 "get another vlanInfoPtr->naMsgToCpuEn than was set: %d, %d", dev, vlanId);
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.mruIdx,
                                             vlanInfoGet.mruIdx,
                 "get another vlanInfoPtr->mruIdx than was set: %d, %d", dev, vlanId);
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.bcastUdpTrapMirrEn,
                                             vlanInfoGet.bcastUdpTrapMirrEn,
                 "get another vlanInfoPtr->bcastUdpTrapMirrEn than was set: %d, %d", dev, vlanId);
            }

            if(devFamily >= CPSS_PP_FAMILY_DXCH_XCAT_E)
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.mcastLocalSwitchingEn,
                                             vlanInfoGet.mcastLocalSwitchingEn,
                 "get another vlanInfoPtr->mcastLocalSwitchingEn than was set: %d, %d", dev, vlanId);
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.portIsolationMode,
                                             vlanInfoGet.portIsolationMode,
                 "get another vlanInfoPtr->portIsolationMode than was set: %d, %d", dev, vlanId);
            }

            if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.mirrToRxAnalyzerIndex,
                                             vlanInfoGet.mirrToRxAnalyzerIndex,
                 "get another vlanInfoPtr->mirrToRxAnalyzerIndex than was set: %d, %d", dev, vlanId);

                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.mirrToTxAnalyzerEn,
                                             vlanInfoGet.mirrToTxAnalyzerEn,
                 "get another vlanInfoPtr->mirrToTxAnalyzerEn than was set: %d, %d", dev, vlanId);

                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.mirrToTxAnalyzerIndex,
                                             vlanInfoGet.mirrToTxAnalyzerIndex,
                 "get another vlanInfoPtr->mirrToTxAnalyzerIndex than was set: %d, %d", dev, vlanId);

                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.fidValue,
                                             vlanInfoGet.fidValue,
                 "get another vlanInfoPtr->fidValue than was set: %d, %d", dev, vlanId);

                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.unknownMacSaCmd,
                                             vlanInfoGet.unknownMacSaCmd,
                 "get another vlanInfoPtr->unknownMacSaCmd than was set: %d, %d", dev, vlanId);

                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.ipv4McBcMirrToAnalyzerEn,
                                             vlanInfoGet.ipv4McBcMirrToAnalyzerEn,
                 "get another vlanInfoPtr->ipv4McBcMirrToAnalyzerEn than was set: %d, %d", dev, vlanId);

                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.ipv4McBcMirrToAnalyzerIndex,
                                             vlanInfoGet.ipv4McBcMirrToAnalyzerIndex,
                 "get another vlanInfoPtr->ipv4McBcMirrToAnalyzerIndex than was set: %d, %d", dev, vlanId);

                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.ipv6McMirrToAnalyzerEn,
                                             vlanInfoGet.ipv6McMirrToAnalyzerEn,
                 "get another vlanInfoPtr->ipv6McMirrToAnalyzerEn than was set: %d, %d", dev, vlanId);

                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.ipv6McMirrToAnalyzerIndex,
                                             vlanInfoGet.ipv6McMirrToAnalyzerIndex,
                 "get another vlanInfoPtr->ipv6McMirrToAnalyzerIndex than was set: %d, %d", dev, vlanId);
            }

            if (PRV_CPSS_SIP_6_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.mirrToTxAnalyzerMode,
                                             vlanInfoGet.mirrToTxAnalyzerMode,
                                             "get another vlanInfoPtr->mirrToTxAnalyzerMode than was set: %d, %d", dev, vlanId);
            }

        }

        /* VRF ID supported starting from DxCh3 but xCat2 does not support it */
        if ((devFamily >= CPSS_PP_FAMILY_CHEETAH3_E) && (devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E))
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.vrfId, vlanInfoGet.vrfId,
                 "get another vlanInfoPtr->vrfId than was set: %d, %d", dev, vlanId);
        }

        /*
            1.3. Call with wrong enum values ipv4IpmBrgMode,
                          ipv4IpmBrgEn [GT_FALSE] (in this case ipv4IpmBrgMode is not relevant)
                          and other parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */
        vlanInfo.ipv4IpmBrgEn = GT_FALSE;

        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanEntryWrite(dev, vlanId, &portsMembers,
                                       &portsTagging, &vlanInfo, portsTaggingCmdPtr),
                                     vlanInfo.ipv4IpmBrgMode);

        /*
            1.4. Call with wrong enum values ipv4IpmBrgMode,
                           ipv4IpmBrgEn [GT_TRUE] (in this case ipv4IpmBrgMode is relevant)
                           and other parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */
        vlanInfo.ipv4IpmBrgEn = GT_TRUE;

        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanEntryWrite
                                                        (dev, vlanId, &portsMembers, &portsTagging,
                             &vlanInfo, portsTaggingCmdPtr),
                                                        vlanInfo.ipv4IpmBrgMode);

        /*
            1.5. Call with wrong enum values ipv6IpmBrgMode,
                          ipv6IpmBrgEn [GT_FALSE] (in this case ipv6IpmBrgMode is not relevant)
                          and other parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */
        vlanInfo.ipv6IpmBrgEn = GT_FALSE;

        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanEntryWrite(dev, vlanId, &portsMembers,
                                       &portsTagging, &vlanInfo, portsTaggingCmdPtr),
                                     vlanInfo.ipv6IpmBrgMode);

        /*
            1.6. Call with out of range ipv6IpmBrgMode,
                               ipv6IpmBrgEn [GT_TRUE] (in this case ipv6IpmBrgMode is relevant)
                               and other parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */
        vlanInfo.ipv6IpmBrgEn = GT_TRUE;

        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanEntryWrite
                                                        (dev, vlanId, &portsMembers, &portsTagging,
                             &vlanInfo, portsTaggingCmdPtr),
                                                        vlanInfo.ipv6IpmBrgMode);

        vlanInfo.ipv6IpmBrgMode = CPSS_BRG_IPM_GV_E;

        /*
            1.7. Call with ipCtrlToCpuEn [CPSS_DXCH_BRG_IP_CTRL_IPV4_E]
                               and other parameters from 1.1.
            Expected: GT_OK for DxCh2, DxCh3 and NOT GT_OK for others.
        */
        vlanInfo.ipCtrlToCpuEn = CPSS_DXCH_BRG_IP_CTRL_IPV4_E;

        st = cpssDxChBrgVlanEntryWrite(dev, vlanId, &portsMembers,
                                       &portsTagging, &vlanInfo, portsTaggingCmdPtr);

        if (devFamily != CPSS_PP_FAMILY_CHEETAH_E)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->ipCtrlToCpuEn = %d",
                                         dev, vlanInfo.ipCtrlToCpuEn);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->ipCtrlToCpuEn = %d",
                                             dev, vlanInfo.ipCtrlToCpuEn);
        }

        /*
            1.8. Call with ipCtrlToCpuEn [CPSS_DXCH_BRG_IP_CTRL_IPV6_E]
                      and other parameters from 1.1.
            Expected: GT_OK for DxCh2, DxCh3 and NOT GT_OK for others.
        */
        vlanInfo.ipCtrlToCpuEn = CPSS_DXCH_BRG_IP_CTRL_IPV6_E;

        st = cpssDxChBrgVlanEntryWrite(dev, vlanId, &portsMembers,
                                       &portsTagging, &vlanInfo, portsTaggingCmdPtr);

        if (devFamily != CPSS_PP_FAMILY_CHEETAH_E)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->ipCtrlToCpuEn = %d",
                                         dev, vlanInfo.ipCtrlToCpuEn);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->ipCtrlToCpuEn = %d",
                                             dev, vlanInfo.ipCtrlToCpuEn);
        }

        vlanInfo.ipCtrlToCpuEn = CPSS_DXCH_BRG_IP_CTRL_NONE_E;

        /*
            1.9. Call with out of range vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)]
                           and other parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */
        vlanId = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev);

        st = cpssDxChBrgVlanEntryWrite(dev, vlanId, &portsMembers,
                                       &portsTagging, &vlanInfo, portsTaggingCmdPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, vlanId);

        vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;

        /*
            1.10. Call with wrong enum values unregNonIpMcastCmd and other parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanEntryWrite
                                                        (dev, vlanId, &portsMembers, &portsTagging,
                             &vlanInfo, portsTaggingCmdPtr),
                                                        vlanInfo.unregNonIpMcastCmd);

        vlanInfo.unregNonIpMcastCmd = CPSS_PACKET_CMD_FORWARD_E;

        /*
            1.11. Call with wrong enum values unregIpv4McastCmd and other parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanEntryWrite
                                                        (dev, vlanId, &portsMembers,
                                           &portsTagging, &vlanInfo, portsTaggingCmdPtr),
                                                        vlanInfo.unregIpv4McastCmd);

        vlanInfo.unregIpv4McastCmd = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;

        /*
            1.12. Call with wrong enum values unregIpv6McastCmd and other parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanEntryWrite
                                                        (dev, vlanId, &portsMembers, &portsTagging,
                             &vlanInfo, portsTaggingCmdPtr),
                                                        vlanInfo.unregIpv6McastCmd);

        vlanInfo.unregIpv6McastCmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;

        /*
            1.13. Call with wrong enum values unkUcastCmd and other parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanEntryWrite
                                                        (dev, vlanId, &portsMembers, &portsTagging,
                             &vlanInfo, portsTaggingCmdPtr),
                                                        vlanInfo.unkUcastCmd);

        vlanInfo.unkUcastCmd = CPSS_PACKET_CMD_DROP_HARD_E;

        /*
            1.14. Call with wrong enum values unregIpv4BcastCmd and other parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanEntryWrite
                                                        (dev, vlanId, &portsMembers, &portsTagging,
                             &vlanInfo, portsTaggingCmdPtr),
                                                        vlanInfo.unregIpv4BcastCmd);

        vlanInfo.unregIpv4BcastCmd = CPSS_PACKET_CMD_DROP_SOFT_E;

        /*
            1.15. Call with wrong enum values unregNonIpv4BcastCmd and other parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanEntryWrite
                                                        (dev, vlanId, &portsMembers, &portsTagging,
                             &vlanInfo, portsTaggingCmdPtr),
                                                        vlanInfo.unregNonIpv4BcastCmd);

        vlanInfo.unregNonIpv4BcastCmd = CPSS_PACKET_CMD_FORWARD_E;

        /*
            1.16. Call with wrong enum values ipCtrlToCpuEn and other parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanEntryWrite
                                                        (dev, vlanId, &portsMembers, &portsTagging,
                             &vlanInfo, portsTaggingCmdPtr),
                                                        vlanInfo.ipCtrlToCpuEn);

        vlanInfo.ipCtrlToCpuEn = CPSS_DXCH_BRG_IP_CTRL_NONE_E;

        /*
            1.17. Call with wrong enum values ipv6SiteIdMode and other parameters from 1.1.
            Expected: GT_OK for Cheetah and xCat2
                      and GT_BAD_PARAM for others (supported only by Cheetah2).
        */
        if ((devFamily != CPSS_PP_FAMILY_CHEETAH_E) && (devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E))
        {
            UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanEntryWrite
                                (dev, vlanId, &portsMembers, &portsTagging,
                                 &vlanInfo, portsTaggingCmdPtr),
                                vlanInfo.ipv6SiteIdMode);
        }
        else
        {
            for(index = 0; index < utfInvalidEnumArrSize; index++)
            {
                vlanInfo.ipv6SiteIdMode = utfInvalidEnumArr[index];
                st = cpssDxChBrgVlanEntryWrite(dev, vlanId, &portsMembers,
                                               &portsTagging, &vlanInfo, portsTaggingCmdPtr);

                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->ipv6SiteIdMode = %d",
                                             dev, vlanInfo.ipv6SiteIdMode);
            }
        }

        vlanInfo.ipv6SiteIdMode = CPSS_IP_SITE_ID_INTERNAL_E;

        /*
            1.18. Call with vlanId [100] and portsMembersPtr [NULL]
                            and other parameters from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgVlanEntryWrite(dev, vlanId, NULL,
                                       &portsTagging, &vlanInfo, portsTaggingCmdPtr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, portsMembersPtr = NULL", dev);

        /*
            1.19. Call with vlanId [100] and portsTaggingPtr [NULL]
                           and other parameters from 1.1.
            Expected: GT_BAD_PTR for TR101 disabled devices,
                      GT_OK for TR101 enabled devices.
        */
        st = cpssDxChBrgVlanEntryWrite(dev, vlanId, &portsMembers,
                                       NULL, &vlanInfo, portsTaggingCmdPtr);
        if(IS_TR101_SUPPORTED_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "%d, portsTaggingPtr = NULL", dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, portsTaggingPtr = NULL", dev);
        }

        /*
            1.19.1 Call with vlanId [100] and portsTaggingCmdPtr [NULL]
                           and other parameters from 1.1.
            Expected: GT_BAD_PTR for TR101 enabled devices,
                      GT_OK for TR101 disabled devices.
        */
        st = cpssDxChBrgVlanEntryWrite(dev, vlanId, &portsMembers,
                                       &portsTagging, &vlanInfo, NULL);
        if(!IS_TR101_SUPPORTED_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "%d, portsTaggingCmdPtr = NULL", dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, portsTaggingCmdPtr = NULL", dev);
        }

        /*
            1.20. Call with vlanId [100] and vlanInfoPtr [NULL]
                           and other parameters from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgVlanEntryWrite(dev, vlanId, &portsMembers,
                                       &portsTagging, NULL, portsTaggingCmdPtr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, vlanInfoPtr = NULL", dev);

        /*
            1.21. Call with out of range stgId [256]
                                and other parameters from 1.1.
            Expected: NON GT_OK.
        */
        vlanInfo.stgId = MAX_STG_ID_MAC(dev);

        st = cpssDxChBrgVlanEntryWrite(dev, vlanId, &portsMembers,
                                       &portsTagging, &vlanInfo, portsTaggingCmdPtr);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->stgId = %d",
                                         dev, vlanInfo.stgId);
        vlanInfo.stgId = 0;

        /*
            1.22. Call with mruIdx [PRV_CPSS_DXCH_BRG_MRU_INDEX_MAX_CNS+1=8]
                      (no constraints by contract) and other parameters from 1.1.
            Expected: non GT_OK.
        */
        vlanInfo.mruIdx = 8;

        st = cpssDxChBrgVlanEntryWrite(dev, vlanId, &portsMembers,
                                       &portsTagging, &vlanInfo, portsTaggingCmdPtr);
        if(devFamily != CPSS_PP_FAMILY_CHEETAH_E)
        {
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->mruIdx = %d",
                                         dev, vlanInfo.mruIdx);
        }
        else
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->mruIdx = %d",
                                         dev, vlanInfo.mruIdx);

        }

        vlanInfo.mruIdx = 0;

        /*
            1.23. Call with out of range mirrToRxAnalyzerIndex[7] and other parameters from 1.1.
            Expected: NON GT_OK for Bobcat2; Caelum; Bobcat3 and GT_OK for others (supported only by Bobcat2; Caelum; Bobcat3).
        */
        vlanInfo.mirrToRxAnalyzerIndex = 7;

        st = cpssDxChBrgVlanEntryWrite(dev, vlanId, &portsMembers,
                                       &portsTagging, &vlanInfo, portsTaggingCmdPtr);

        if (0 == UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->mirrToRxAnalyzerIndex = %d",
                                         dev, vlanInfo.mirrToRxAnalyzerIndex);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->mirrToRxAnalyzerIndex = %d",
                                             dev, vlanInfo.mirrToRxAnalyzerIndex);
        }

        /* restore previous value */
        vlanInfo.mirrToRxAnalyzerIndex = 1;

        /*
            1.24. Call with mirrToTxAnalyzerEn[GT_FALSE],
                out of range mirrToTxAnalyzerIndex[7](not relevant) and
                other parameters from 1.1.
            Expected: GT_OK.
        */
        vlanInfo.mirrToTxAnalyzerIndex = 7;
        vlanInfo.mirrToTxAnalyzerEn = GT_FALSE;
        vlanInfo.mirrToTxAnalyzerMode = CPSS_DXCH_MIRROR_EGRESS_CONGESTION_E;

        st = cpssDxChBrgVlanEntryWrite(dev, vlanId, &portsMembers,
                                       &portsTagging, &vlanInfo, portsTaggingCmdPtr);

        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->mirrToTxAnalyzerIndex = %d",
                                     dev, vlanInfo.mirrToTxAnalyzerIndex);

        /*
            1.25. Call with mirrToTxAnalyzerEn[GT_TRUE],
                out of range mirrToTxAnalyzerIndex[7](is relevant) and other parameters from 1.1.
            Expected: NON GT_OK for Bobcat2; Caelum; Bobcat3 and GT_OK for others.
        */
        vlanInfo.mirrToTxAnalyzerIndex = 7;
        vlanInfo.mirrToTxAnalyzerEn = GT_TRUE;        

        st = cpssDxChBrgVlanEntryWrite(dev, vlanId, &portsMembers,
                                       &portsTagging, &vlanInfo, portsTaggingCmdPtr);

        if (0 == UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->mirrToTxAnalyzerIndex = %d",
                                         dev, vlanInfo.mirrToTxAnalyzerIndex);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->mirrToTxAnalyzerIndex = %d",
                                             dev, vlanInfo.mirrToTxAnalyzerIndex);
        }

        vlanInfo.mirrToTxAnalyzerMode = CPSS_DXCH_MIRROR_EGRESS_CONGESTION_OR_TAIL_DROP_E;
        st = cpssDxChBrgVlanEntryWrite(dev, vlanId, &portsMembers,
                                       &portsTagging, &vlanInfo, portsTaggingCmdPtr);
        
        if (0 == UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->mirrToTxAnalyzerMode = %d",
                dev, vlanInfo.mirrToTxAnalyzerMode);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->mirrToTxAnalyzerMode = %d",
                dev, vlanInfo.mirrToTxAnalyzerMode);
        }

        /* restore previous value */
        vlanInfo.mirrToTxAnalyzerIndex = 2;

        /*
            1.26. Call with out of range fidValue[PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_FID_MAC(dev)+1]
                and other parameters from 1.1.
            Expected: NON GT_OK for Bobcat2; Caelum; Bobcat3 and GT_OK for others (supported only by Bobcat2; Caelum; Bobcat3).
        */
        vlanInfo.fidValue = UTF_CPSS_PP_MAX_FID_NUM_CNS(dev);

        st = cpssDxChBrgVlanEntryWrite(dev, vlanId, &portsMembers,
                                       &portsTagging, &vlanInfo, portsTaggingCmdPtr);

        if (0 == UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->fidValue = %d",
                                         dev, vlanInfo.fidValue);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->fidValue = %d",
                                             dev, vlanInfo.fidValue);
        }

        /* restore previous value */
        vlanInfo.fidValue = UTF_CPSS_PP_MAX_FID_NUM_CNS(dev) - 1;

        /*
            1.27. Call with out of range unknownMacSaCmd[0x5AAAAAA5] and other parameters from 1.1.
            Expected: NON GT_OK for Bobcat2; Caelum; Bobcat3 and GT_OK for others (supported only by Bobcat2; Caelum; Bobcat3).
        */
        vlanInfo.unknownMacSaCmd = 0x5AAAAAA5;

        st = cpssDxChBrgVlanEntryWrite(dev, vlanId, &portsMembers,
                                       &portsTagging, &vlanInfo, portsTaggingCmdPtr);

        if (0 == UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->unknownMacSaCmd = %d",
                                         dev, vlanInfo.unknownMacSaCmd);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->unknownMacSaCmd = %d",
                                             dev, vlanInfo.unknownMacSaCmd);
        }

        /* restore previous value */
        vlanInfo.unknownMacSaCmd = CPSS_PACKET_CMD_DROP_HARD_E;

        /*
            1.28. Call with ipv4McBcMirrToAnalyzerEn[GT_FALSE],
                out of range ipv4McBcMirrToAnalyzerIndex[7](not relevant) and
                other parameters from 1.1.
            Expected: GT_OK.
        */
        vlanInfo.ipv4McBcMirrToAnalyzerIndex = 7;
        vlanInfo.ipv4McBcMirrToAnalyzerEn = GT_FALSE;

        st = cpssDxChBrgVlanEntryWrite(dev, vlanId, &portsMembers,
                                       &portsTagging, &vlanInfo, portsTaggingCmdPtr);

        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->ipv4McBcMirrToAnalyzerIndex = %d",
                                     dev, vlanInfo.ipv4McBcMirrToAnalyzerIndex);

        /*
            1.29. Call with ipv4McBcMirrToAnalyzerEn[GT_TRUE](is relevant),
                out of range ipv4McBcMirrToAnalyzerIndex[7] and
                other parameters from 1.1.
            Expected: NON GT_OK for Bobcat2; Caelum; Bobcat3 and GT_OK.
        */
        vlanInfo.ipv4McBcMirrToAnalyzerIndex = 7;
        vlanInfo.ipv4McBcMirrToAnalyzerEn = GT_TRUE;

        st = cpssDxChBrgVlanEntryWrite(dev, vlanId, &portsMembers,
                                       &portsTagging, &vlanInfo, portsTaggingCmdPtr);

        if (0 == UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->ipv4McBcMirrToAnalyzerIndex = %d",
                                         dev, vlanInfo.ipv4McBcMirrToAnalyzerIndex);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->ipv4McBcMirrToAnalyzerIndex = %d",
                                             dev, vlanInfo.ipv4McBcMirrToAnalyzerIndex);
        }

        /* restore previous value */
        vlanInfo.ipv4McBcMirrToAnalyzerIndex = 3;

        /*
            1.30. Call with ipv6McMirrToAnalyzerEn[GT_FALSE],
                out of range ipv6McMirrToAnalyzerIndex[7](not relevant) and
                other parameters from 1.1.
            Expected: GT_OK.
        */

        vlanInfo.ipv6McMirrToAnalyzerIndex = 7;
        vlanInfo.ipv6McMirrToAnalyzerEn = GT_FALSE;

        st = cpssDxChBrgVlanEntryWrite(dev, vlanId, &portsMembers,
                                       &portsTagging, &vlanInfo, portsTaggingCmdPtr);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->ipv6McMirrToAnalyzerIndex = %d",
                                     dev, vlanInfo.ipv6McMirrToAnalyzerIndex);

        /*
            1.31. Call with ipv6McMirrToAnalyzerEn[GT_TRUE],
                out of range ipv6McMirrToAnalyzerIndex[7](is relevant) and
                other parameters from 1.1.
            Expected: NON GT_OK for Bobcat2; Caelum; Bobcat3 and GT_OK for others.
        */
        vlanInfo.ipv6McMirrToAnalyzerIndex = 7;
        vlanInfo.ipv6McMirrToAnalyzerEn = GT_TRUE;

        st = cpssDxChBrgVlanEntryWrite(dev, vlanId, &portsMembers,
                                       &portsTagging, &vlanInfo, portsTaggingCmdPtr);

        if (0 == UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->ipv6McMirrToAnalyzerIndex = %d",
                                         dev, vlanInfo.ipv6McMirrToAnalyzerIndex);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->ipv6McMirrToAnalyzerIndex = %d",
                                             dev, vlanInfo.ipv6McMirrToAnalyzerIndex);
        }

        /* restore previous value */
        vlanInfo.ipv6McMirrToAnalyzerIndex = 4;

        /*
            1.32. Call with out of range vrfId [4096]
                            and other parameters from 1.1.
            Expected: NON GT_OK for DxCh3 and above exclude xCat2
                      and GT_OK for others (supported only by Cheetah3 and above)
        */
        vlanInfo.vrfId = 4096;

        st = cpssDxChBrgVlanEntryWrite(dev, vlanId, &portsMembers,
                                       &portsTagging, &vlanInfo, portsTaggingCmdPtr);
        if ((devFamily >= CPSS_PP_FAMILY_CHEETAH3_E) && (devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E))
        {
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->vrfId = %d",
                                             dev, vlanInfo.vrfId);
        }
        else
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->vrfId = %d",
                                         dev, vlanInfo.vrfId);
        }

        vlanInfo.vrfId = 5;

        if(devFamily >= CPSS_PP_FAMILY_DXCH_XCAT_E)
        {
            vlanInfo.mcastLocalSwitchingEn = GT_TRUE;

            for(vlanInfo.mcastLocalSwitchingEn = GT_FALSE ;
                vlanInfo.mcastLocalSwitchingEn <= GT_TRUE ;
                vlanInfo.mcastLocalSwitchingEn ++)
            {
                CPSS_COVERITY_NON_ISSUE_BOOKMARK
                /* coverity[var_deref_model] */
                st = cpssDxChBrgVlanEntryWrite(dev, vlanId, &portsMembers,
                                               &portsTagging, &vlanInfo, portsTaggingCmdPtr);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, vlanId);

                /*
                    1.2. Call cpssDxChBrgVlanEntryRead with the same vlanId.
                    Expected: GT_OK and the same portsMembersPtr, portsTaggingPtr and vlanInfoPtr.
                             Check vlanInfo by fields: 1) skip ipv6SiteIdMode, ipv4McastRateEn,
                                                         ipv6McastRateEn, autoLearnDisable, naMsgToCpuEn,
                                                         mruIdx, bcastUdpTrapMirrEn fields for Cheetah,
                                                         but they should be checked for DxCh2, DxCh3;
                                                      2) ipv4IpmBrgMode and ipv6IpnBrgMode should be
                                                         checked only if  corresponding
                                                         ipv4/6IpmBrgEn = GT_TRUE.
                */
                CPSS_COVERITY_NON_ISSUE_BOOKMARK
                /* coverity[var_deref_model] */ st = cpssDxChBrgVlanEntryRead(dev, vlanId, &portsMembersGet, &portsTaggingGet,
                                                                              &vlanInfoGet, &isValid, portsTaggingCmdGetPtr);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                           "cpssDxChBrgVlanEntryRead: %d, %d", dev, vlanId);

                UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isValid,
                        "Vlan entry is NOT valid: %d, %d", dev, vlanId);

                if (GT_TRUE == isValid)
                {
                    UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.mcastLocalSwitchingEn,
                                                 vlanInfoGet.mcastLocalSwitchingEn,
                     "get another vlanInfoPtr->mcastLocalSwitchingEn than was set: %d, %d", dev, vlanId);
                }
            }

            vlanInfo.mcastLocalSwitchingEn = GT_FALSE;


            for(vlanInfo.portIsolationMode = CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_DISABLE_CMD_E ;
                vlanInfo.portIsolationMode <= CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_L2_L3_CMD_E ;
                vlanInfo.portIsolationMode ++)
            {
                CPSS_COVERITY_NON_ISSUE_BOOKMARK
                /* coverity[var_deref_model] */
                st = cpssDxChBrgVlanEntryWrite(dev, vlanId, &portsMembers,
                                               &portsTagging, &vlanInfo, portsTaggingCmdPtr);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, vlanId);

                /*
                    1.2. Call cpssDxChBrgVlanEntryRead with the same vlanId.
                    Expected: GT_OK and the same portsMembersPtr, portsTaggingPtr and vlanInfoPtr.
                             Check vlanInfo by fields: 1) skip ipv6SiteIdMode, ipv4McastRateEn,
                                                         ipv6McastRateEn, autoLearnDisable, naMsgToCpuEn,
                                                         mruIdx, bcastUdpTrapMirrEn fields for Cheetah,
                                                         but they should be checked for DxCh2, DxCh3;
                                                      2) ipv4IpmBrgMode and ipv6IpnBrgMode should be
                                                         checked only if  corresponding
                                                         ipv4/6IpmBrgEn = GT_TRUE.
                */
                CPSS_COVERITY_NON_ISSUE_BOOKMARK
                /* coverity[var_deref_model] */ st = cpssDxChBrgVlanEntryRead(dev, vlanId, &portsMembersGet, &portsTaggingGet,
                                                                              &vlanInfoGet, &isValid, portsTaggingCmdGetPtr);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                           "cpssDxChBrgVlanEntryRead: %d, %d", dev, vlanId);

                UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isValid,
                        "Vlan entry is NOT valid: %d, %d", dev, vlanId);

                if (GT_TRUE == isValid)
                {
                    UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.portIsolationMode,
                                                 vlanInfoGet.portIsolationMode,
                     "get another vlanInfoPtr->portIsolationMode than was set: %d, %d", dev, vlanId);
                }
            }
            vlanInfo.portIsolationMode = CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_DISABLE_CMD_E;

            UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanEntryWrite
                                (dev, vlanId, &portsMembers, &portsTagging,
                                 &vlanInfo, portsTaggingCmdPtr),
                                vlanInfo.portIsolationMode);
        }

        /* clear all members for the test of cpssDxChBrgVlanIsDevMember*/
        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
        st = cpssDxChBrgVlanEntryWrite(dev, vlanId, &portsMembers,
                                       &portsTagging, &vlanInfo, portsTaggingCmdPtr);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgVlanEntryWrite: %d, %d", dev, vlanId);
        /*
            1.33. Call function with
                fcoeForwardingEn [GT_FALSE / GT_ TRUE],
                unregIpmEVidxMode
                [ CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_BOTH_E /
                 CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_IPV4_E /
                 CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_IPV6_E /
                 CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_BOTH_IPV6_INCREMENT_E],
                unregIpmEVidx [0 / 0xA5A5 / BIT_16-1]
                and other parameters from 1.1.
            Expected: GT_OK for Bobcat2; Caelum; Bobcat3.
        */
        if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev) != 0)
        {
            /*
                Call with unregIpmEVidxMode
                               [CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_BOTH_E],
                          fcoeForwardingEn [GT_FALSE],
                          unregIpmEVidx [0]
                          and other parameters from 1.1.
            */
            vlanInfo.fcoeForwardingEn  = GT_FALSE;
            vlanInfo.unregIpmEVidx     = 0;
            vlanInfo.unregIpmEVidxMode =
                                 CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_BOTH_E;

            st = cpssDxChBrgVlanEntryWrite(dev, vlanId, &portsMembers,
                                  &portsTagging, &vlanInfo, portsTaggingCmdPtr);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK,st, dev,vlanInfo.fcoeForwardingEn,
                            vlanInfo.unregIpmEVidx, vlanInfo.unregIpmEVidxMode);

            /* verify values*/
            st = cpssDxChBrgVlanEntryRead(dev, vlanId, &portsMembersGet,
               &portsTaggingGet, &vlanInfoGet, &isValid, portsTaggingCmdGetPtr);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                               "cpssDxChBrgVlanEntryRead: %d, %d", dev, vlanId);

            UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.fcoeForwardingEn,
                                         vlanInfoGet.fcoeForwardingEn,
               "get another vlanInfoPtr->fcoeForwardingEn than was set: %d, %d",
                                         dev, vlanId);
            UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.unregIpmEVidx,
                                         vlanInfoGet.unregIpmEVidx,
                  "get another vlanInfoPtr->unregIpmEVidx than was set: %d, %d",
                                         dev, vlanId);
            UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.unregIpmEVidxMode,
                                         vlanInfoGet.unregIpmEVidxMode,
              "get another vlanInfoPtr->unregIpmEVidxMode than was set: %d, %d",
                                         dev, vlanId);

            /*
                Call with unregIpmEVidxMode
                               [CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_IPV4_E],
                          fcoeForwardingEn [GT_ TRUE],
                          unregIpmEVidx [0xA5A5]
                          and other parameters from 1.1.
            */
            vlanInfo.fcoeForwardingEn  = GT_TRUE;
            vlanInfo.unregIpmEVidx     = 0xA5A5;
            vlanInfo.unregIpmEVidxMode =
                                 CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_IPV4_E;

            st = cpssDxChBrgVlanEntryWrite(dev, vlanId, &portsMembers,
                                  &portsTagging, &vlanInfo, portsTaggingCmdPtr);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK,st, dev,vlanInfo.fcoeForwardingEn,
                            vlanInfo.unregIpmEVidx, vlanInfo.unregIpmEVidxMode);

            /* verify values*/
            st = cpssDxChBrgVlanEntryRead(dev, vlanId, &portsMembersGet,
               &portsTaggingGet, &vlanInfoGet, &isValid, portsTaggingCmdGetPtr);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                               "cpssDxChBrgVlanEntryRead: %d, %d", dev, vlanId);

            UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.fcoeForwardingEn,
                                         vlanInfoGet.fcoeForwardingEn,
               "get another vlanInfoPtr->fcoeForwardingEn than was set: %d, %d",
                                         dev, vlanId);
            UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.unregIpmEVidx,
                                         vlanInfoGet.unregIpmEVidx,
                  "get another vlanInfoPtr->unregIpmEVidx than was set: %d, %d",
                                         dev, vlanId);
            UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.unregIpmEVidxMode,
                                         vlanInfoGet.unregIpmEVidxMode,
              "get another vlanInfoPtr->unregIpmEVidxMode than was set: %d, %d",
                                         dev, vlanId);

            /*
                Call with unregIpmEVidxMode
                               [CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_IPV6_E],
                          fcoeForwardingEn [GT_FALSE],
                          unregIpmEVidx [BIT_16-1]
                          and other parameters from 1.1.
            */
            vlanInfo.fcoeForwardingEn  = GT_FALSE;
            vlanInfo.unregIpmEVidx     = BIT_16-1;
            vlanInfo.unregIpmEVidxMode =
                                 CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_IPV6_E;

            st = cpssDxChBrgVlanEntryWrite(dev, vlanId, &portsMembers,
                                  &portsTagging, &vlanInfo, portsTaggingCmdPtr);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK,st, dev,vlanInfo.fcoeForwardingEn,
                            vlanInfo.unregIpmEVidx, vlanInfo.unregIpmEVidxMode);

            /* verify values*/
            st = cpssDxChBrgVlanEntryRead(dev, vlanId, &portsMembersGet,
               &portsTaggingGet, &vlanInfoGet, &isValid, portsTaggingCmdGetPtr);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                               "cpssDxChBrgVlanEntryRead: %d, %d", dev, vlanId);

            UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.fcoeForwardingEn,
                                         vlanInfoGet.fcoeForwardingEn,
               "get another vlanInfoPtr->fcoeForwardingEn than was set: %d, %d",
                                         dev, vlanId);
            UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.unregIpmEVidx,
                                         vlanInfoGet.unregIpmEVidx,
                  "get another vlanInfoPtr->unregIpmEVidx than was set: %d, %d",
                                         dev, vlanId);
            UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.unregIpmEVidxMode,
                                         vlanInfoGet.unregIpmEVidxMode,
              "get another vlanInfoPtr->unregIpmEVidxMode than was set: %d, %d",
                                         dev, vlanId);

            /*
                Call with unregIpmEVidxMode
                [CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_BOTH_IPV6_INCREMENT_E],
                          fcoeForwardingEn [GT_ TRUE] and
                          unregIpmEVidx [BIT_16-1]
                          and other parameters from 1.1.
            */
            vlanInfo.fcoeForwardingEn  = GT_TRUE;
            vlanInfo.unregIpmEVidx     = BIT_16-1;
            vlanInfo.unregIpmEVidxMode =
                  CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_BOTH_IPV6_INCREMENT_E;

            st = cpssDxChBrgVlanEntryWrite(dev, vlanId, &portsMembers,
                                  &portsTagging, &vlanInfo, portsTaggingCmdPtr);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK,st, dev,vlanInfo.fcoeForwardingEn,
                            vlanInfo.unregIpmEVidx, vlanInfo.unregIpmEVidxMode);

            /* verify values*/
            st = cpssDxChBrgVlanEntryRead(dev, vlanId, &portsMembersGet,
               &portsTaggingGet, &vlanInfoGet, &isValid, portsTaggingCmdGetPtr);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                               "cpssDxChBrgVlanEntryRead: %d, %d", dev, vlanId);

            UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.fcoeForwardingEn,
                                         vlanInfoGet.fcoeForwardingEn,
               "get another vlanInfoPtr->fcoeForwardingEn than was set: %d, %d",
                                         dev, vlanId);
            UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.unregIpmEVidx,
                                         vlanInfoGet.unregIpmEVidx,
                  "get another vlanInfoPtr->unregIpmEVidx than was set: %d, %d",
                                         dev, vlanId);
            UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.unregIpmEVidxMode,
                                         vlanInfoGet.unregIpmEVidxMode,
              "get another vlanInfoPtr->unregIpmEVidxMode than was set: %d, %d",
                                         dev, vlanId);
        }
        /*
           1.34. Call function with out of range unregIpmEVidx [BIT_16],
             fcoeForwardingEn [GT_FALSE] and
             unregIpmEVidxMode [CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_BOTH_E]
             and other parameters from 1.1.
           Expected: NOT GT_OK for Bobcat2; Caelum; Bobcat3.
        */
        if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev) != 0)
        {
            vlanInfo.fcoeForwardingEn  = GT_FALSE;
            vlanInfo.unregIpmEVidx = BIT_16;
            vlanInfo.unregIpmEVidxMode =
                                 CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_BOTH_E;

            st = cpssDxChBrgVlanEntryWrite(dev, vlanId, &portsMembers,
                                  &portsTagging, &vlanInfo, portsTaggingCmdPtr);
            UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, dev,
                                            vlanInfo.fcoeForwardingEn,
                                            vlanInfo.unregIpmEVidx,
                                            vlanInfo.unregIpmEVidxMode);

            vlanInfo.unregIpmEVidx = 0;
        }
        /*
            1.35. Call function with out of range unregIpmEVidxMode,
                                            unregIpmEVidx [0],
                                            fcoeForwardingEn [GT_FALSE]
                                            and other parameters from 1.1.
            Expected: GT_BAD_PARAM for Bobcat2; Caelum; Bobcat3.
        */
        if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev) != 0)
        {
            vlanInfo.fcoeForwardingEn  = GT_FALSE;
            vlanInfo.unregIpmEVidx = 0;

            UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanEntryWrite
                                (dev, vlanId, &portsMembers,
                                 &portsTagging, &vlanInfo, portsTaggingCmdPtr),
                                vlanInfo.unregIpmEVidxMode);

            vlanInfo.unregIpmEVidxMode =
                                 CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_BOTH_E;
        }
        /*
          1.36. Call function with fdbLookupKeyMode
                                  = CPSS_DXCH_BRG_FDB_LOOKUP_KEY_MODE_FID_VID1_E
          Expected: GT_OK for SIP 5.10 devices (bobcat2 b0)
        */
        if (PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            vlanInfo.fdbLookupKeyMode =
                                   CPSS_DXCH_BRG_FDB_LOOKUP_KEY_MODE_FID_VID1_E;
            st = cpssDxChBrgVlanEntryWrite(dev, vlanId, &portsMembers,
                                  &portsTagging, &vlanInfo, portsTaggingCmdPtr);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK,st, dev,vlanInfo.fcoeForwardingEn,
                            vlanInfo.unregIpmEVidx, vlanInfo.unregIpmEVidxMode);

            /* verify values*/
            st = cpssDxChBrgVlanEntryRead(dev, vlanId, &portsMembersGet,
               &portsTaggingGet, &vlanInfoGet, &isValid, portsTaggingCmdGetPtr);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                               "cpssDxChBrgVlanEntryRead: %d, %d", dev, vlanId);

            UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.fdbLookupKeyMode,
                                         vlanInfoGet.fdbLookupKeyMode,
               "get another vlanInfoPtr->fdbLookupKeyMode than was set: %d, %d",
                                         dev, vlanId);
        }
        /*
           1.37. Call cpssDxChBrgVlanEntryInvalidate with the same
                 vlanId to invalidate all changes.
           Expected: GT_OK.
        */
        st = cpssDxChBrgVlanEntryInvalidate(dev, vlanId);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgVlanEntryInvalidate: %d, %d", dev, vlanId);
    }

    vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);
    portsMembers.ports[0] = 1000;
    portsTagging.ports[0] = 1500;

    vlanInfo.unkSrcAddrSecBreach  = GT_FALSE;
    vlanInfo.unregNonIpMcastCmd   = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregIpv4McastCmd    = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
    vlanInfo.unregIpv6McastCmd    = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    vlanInfo.unkUcastCmd          = CPSS_PACKET_CMD_DROP_HARD_E;
    vlanInfo.unregIpv4BcastCmd    = CPSS_PACKET_CMD_DROP_SOFT_E;
    vlanInfo.unregNonIpv4BcastCmd = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.ipv4IgmpToCpuEn      = GT_TRUE;
    vlanInfo.mirrToRxAnalyzerEn   = GT_TRUE;
    vlanInfo.ipv6IcmpToCpuEn      = GT_TRUE;
    vlanInfo.ipCtrlToCpuEn        = CPSS_DXCH_BRG_IP_CTRL_NONE_E;
    vlanInfo.ipv4IpmBrgMode       = CPSS_BRG_IPM_SGV_E;
    vlanInfo.ipv6IpmBrgMode       = CPSS_BRG_IPM_GV_E;
    vlanInfo.ipv4IpmBrgEn         = GT_TRUE;
    vlanInfo.ipv6IpmBrgEn         = GT_TRUE;
    vlanInfo.ipv6SiteIdMode       = CPSS_IP_SITE_ID_INTERNAL_E;
    vlanInfo.ipv4UcastRouteEn     = GT_FALSE;
    vlanInfo.ipv4McastRouteEn     = GT_FALSE;
    vlanInfo.ipv6UcastRouteEn     = GT_FALSE;
    vlanInfo.ipv6McastRouteEn     = GT_FALSE;
    vlanInfo.stgId                = 0;
    vlanInfo.autoLearnDisable     = GT_FALSE;
    vlanInfo.naMsgToCpuEn         = GT_FALSE;
    vlanInfo.mruIdx               = 0;
    vlanInfo.bcastUdpTrapMirrEn   = GT_FALSE;
    vlanInfo.vrfId                = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanEntryWrite(dev, vlanId, &portsMembers,
                                       &portsTagging, &vlanInfo, portsTaggingCmdPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanEntryWrite(dev, vlanId, &portsMembers,
                                       &portsTagging, &vlanInfo, portsTaggingCmdPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanEntriesRangeWrite(
    IN GT_U8                        devNum,
    IN GT_U16                       vlanId,
    IN GT_U32                       numOfEntries,
    IN CPSS_PORTS_BMP_STC           *portsMembersPtr,
    IN CPSS_PORTS_BMP_STC           *portsTaggingPtr,
    IN CPSS_DXCH_BRG_VLAN_INFO_STC  *vlanInfoPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanEntriesRangeWrite)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with vlanId [100],
                   numOfEntries[10],
                   portsMemberPtr {[0, 16]},
                   portsTaggingPtr {[0, 16]},
                   vlanInfoPtr {unkSrcAddrSecBreach [GT_FALSE],
                                unregNonIpMcastCmd [CPSS_PACKET_CMD_FORWARD_E],
                                unregIpv4McastCmd [CPSS_PACKET_CMD_MIRROR_TO_CPU_E],
                                unregIpv6McastCmd [CPSS_PACKET_CMD_TRAP_TO_CPU_E],
                                unkUcastCmd [CPSS_PACKET_CMD_DROP_HARD_E],
                                unregIpv4BcastCmd [CPSS_PACKET_CMD_DROP_SOFT_E],
                                unregNonIpv4BcastCmd [CPSS_PACKET_CMD_FORWARD_E],
                                ipv4IgmpToCpuEn [GT_TRUE],
                                mirrToRxAnalyzerEn [GT_TRUE],
                                ipv6IcmpToCpuEn[GT_TRUE],
                                ipCtrlToCpuEn [CPSS_DXCH_BRG_IP_CTRL_NONE_E],
                                ipv4IpmBrgMode [CPSS_BRG_IPM_SGV_E],
                                ipv6IpmBrgMode [CPSS_BRG_IPM_GV_E],
                                ipv4IpmBrgEn[GT_TRUE],
                                ipv6IpmBrgEn [GT_TRUE],
                                ipv6SiteIdMode [CPSS_IP_SITE_ID_INTERNAL_E],
                                ipv4UcastRouteEn[GT_FALSE],
                                ipv4McastRouteEn [GT_FALSE],
                                ipv6UcastRouteEn[GT_FALSE],
                                ipv6McastRouteEn[GT_FALSE],
                                stgId[0],
                                utoLearnDisable
                                [GT_FALSE],
                                naMsgToCpuEn
                                [GT_FALSE],
                                mruIdx[0],
                                bcastUdpTrapMirrEn [GT_FALSE],
                                vrfId [0],

                                (next fields relevant Bobcat2; Caelum; Bobcat3 only)
                                mirrToRxAnalyzerIndex[1],
                                mirrToTxAnalyzerEn[GT_TRUE],
                                mirrToTxAnalyzerIndex[2],
                                fidValue[PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_FID_MAC(dev)],
                                unknownMacSaCmd[CPSS_PACKET_CMD_DROP_HARD_E],
                                ipv4McBcMirrToAnalyzerEn[GT_TRUE],
                                ipv4McBcMirrToAnalyzerIndex[3],
                                ipv6McMirrToAnalyzerEn[GT_TRUE],
                                ipv6McMirrToAnalyzerIndex[4]
                                }.
    Expected: GT_OK.
    1.2. Call cpssDxChBrgVlanEntryRead with the same vlanId for each entry.
    Expected: GT_OK and the same portsMembersPtr, portsTaggingPtr and vlanInfoPtr.
    Check vlanInfo by fields:
            1) skip ipv6SiteIdMode,
                    ipv4McastRateEn,
                    ipv6McastRateEn,
                    autoLearnDisable,
                    naMsgToCpuEn,
                    mruIdx,
                    bcastUdpTrapMirrEn fields for Cheetah,
                    but they should be checked for Cheetah2;
            2) ipv4IpmBrgMode and ipv6IpnBrgMode
                 should be checked only if corresponding ipv4/6IpmBrgEn = GT_TRUE.
            3) for Bobcat2; Caelum; Bobcat3 check also mirrToRxAnalyzerIndex, mirrToTxAnalyzerEn,
                mirrToTxAnalyzerIndex, fidValue, unknownMacSaCmd,
                ipv4McBcMirrToAnalyzerEn, ipv4McBcMirrToAnalyzerIndex,
                ipv6McMirrToAnalyzerEn, ipv6McMirrToAnalyzerIndex.

    1.3. Call with out of range ipv4IpmBrgMode,
                   ipv4IpmBrgEn [GT_FALSE] (in this case ipv4IpmBrgMode is not relevant)
                   and other parameters from 1.1.
    Expected: GT_OK.
    1.4. Call with out of range ipv4IpmBrgMode,
                   ipv4IpmBrgEn [GT_TRUE] (in this case ipv4IpmBrgMode is relevant)
                   and other parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.5. Call with out of range ipv6IpmBrgMode,
                   ipv6IpmBrgEn [GT_FALSE] (in this case ipv6IpmBrgMode is not relevant)
                   and other parameters from 1.1.
    Expected: GT_OK.
    1.6. Call with out of range ipv6IpmBrgMode,
                   ipv6IpmBrgEn [GT_TRUE] (in this case ipv6IpmBrgMode is relevant)
                   and other parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.7. Call with ipCtrlToCpuEn [CPSS_DXCH_BRG_IP_CTRL_IPV4_E]
                   and other parameters from 1.1.
    Expected: GT_OK for DxCh2, DxCh3 and NOT GT_OK for others.
    1.8. Call with ipCtrlToCpuEn [CPSS_DXCH_BRG_IP_CTRL_IPV6_E]
                   and other parameters from 1.1.
    Expected: GT_OK for DxCh2, DxCh3 and NOT GT_OK for others.
    1.9. Call with out of range vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)]
                   and other parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.10. Call with vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev) - 1],
                    numOfEntries [1] and other parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.11. Call with out of range unregNonIpMcastCmd
                    and other parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.12. Call with out of range unregIpv4McastCmd
                    and other parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.13. Call with out of range unregIpv6McastCmd
                    and other parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.14. Call with out of range unkUcastCmd
                    and other parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.15. Call with out of range unregIpv4BcastCmd
                    and other parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.16. Call with out of range unregNonIpv4BcastCmd
                    and other parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.17. Call with out of range ipCtrlToCpuEn
                    and other parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.18. Call with out of range ipv6SiteIdMode
                    and other parameters from 1.1.
    Expected: GT_BAD_PARAM for DxCh2, DxCh3 and GT_OK for others (supported only by Cheetah2,3).
    1.19. Call with vlanId [100] and portsMembersPtr [NULL]
                    and other parameters from 1.1.
    Expected: GT_BAD_PTR.
    1.20. Call with vlanId [100] and portsTaggingPtr [NULL]
                    and other parameters from 1.1.
    Expected: GT_BAD_PTR.
    1.21. Call with vlanId [100] and vlanInfoPtr [NULL]
                    and other parameters from 1.1.
    Expected: GT_BAD_PTR.
    1.22. Call with out of range stgId [256]
                    and other parameters from 1.1.
    Expected: NON GT_OK.
    1.23. Call with mruIdx [PRV_CPSS_DXCH_BRG_MRU_INDEX_MAX_CNS+1=8]
                    and other parameters from 1.1.
    Expected: non GT_OK.

    1.24. Call with out of range mirrToRxAnalyzerIndex[7] and other parameters from 1.1.
    Expected: NON GT_OK for Bobcat2; Caelum; Bobcat3 and GT_OK for others.

    1.25. Call with mirrToTxAnalyzerEn[GT_FALSE],
        out of range mirrToTxAnalyzerIndex[7](not relevant) and
        other parameters from 1.1.
    Expected: GT_OK.

    1.26. Call with mirrToTxAnalyzerEn[GT_TRUE],
        out of range mirrToTxAnalyzerIndex[7](is relevant) and other parameters from 1.1.
    Expected: NON GT_OK for Bobcat2; Caelum; Bobcat3 and GT_OK for others.

    1.27. Call with out of range fidValue[PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_FID_MAC(dev)+1]
        and other parameters from 1.1.
    Expected: NON GT_OK for Bobcat2; Caelum; Bobcat3 and GT_OK for others.

    1.28. Call with out of range unknownMacSaCmd[0x5AAAAAA5] and other parameters from 1.1.
    Expected: NON GT_OK for Bobcat2; Caelum; Bobcat3 and GT_OK for others.

    1.29. Call with ipv4McBcMirrToAnalyzerEn[GT_FALSE],
        out of range ipv4McBcMirrToAnalyzerIndex[7](not relevant) and
        other parameters from 1.1.
    Expected: GT_OK.

    1.30. Call with ipv4McBcMirrToAnalyzerEn[GT_TRUE](is relevant),
        out of range ipv4McBcMirrToAnalyzerIndex[7] and
        other parameters from 1.1.
    Expected: NON GT_OK for Bobcat2; Caelum; Bobcat3 and GT_OK.

    1.31. Call with ipv6McMirrToAnalyzerEn[GT_FALSE],
        out of range ipv6McMirrToAnalyzerIndex[7](not relevant) and
        other parameters from 1.1.
    Expected: GT_OK.

    1.32. Call with ipv6McMirrToAnalyzerEn[GT_TRUE],
        out of range ipv6McMirrToAnalyzerIndex[7](is relevant) and
        other parameters from 1.1.
    Expected: NON GT_OK for Bobcat2; Caelum; Bobcat3 and GT_OK for others.

    1.33. Call with out of range vrfId [4096]
                    and other parameters from 1.1.
    Expected: NON GT_OK for DxCh3 and GT_OK for others (supported only by Cheetah3)

    1.34. Call cpssDxChBrgVlanEntryInvalidate with the same vlanId to invalidate all changes.
    Expected: GT_OK.
*/
    GT_STATUS st     = GT_OK;
    GT_U32    index  = 0;

    GT_U8                       dev;
    GT_U16                      vlanId       = 0;
    GT_U32                      numOfEntries = 0;
    CPSS_PORTS_BMP_STC          portsMembers;
    CPSS_PORTS_BMP_STC          portsTagging;
    CPSS_DXCH_BRG_VLAN_INFO_STC vlanInfo;
    CPSS_PORTS_BMP_STC          portsMembersGet;
    CPSS_PORTS_BMP_STC          portsTaggingGet;
    CPSS_DXCH_BRG_VLAN_INFO_STC vlanInfoGet;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC  portsTaggingCmd;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC  *portsTaggingCmdPtr = NULL;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC  portsTaggingCmdGet;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC  *portsTaggingCmdGetPtr;

    GT_BOOL                     isValid   = GT_FALSE;
    GT_BOOL                     isEqual   = GT_FALSE;
    CPSS_PP_FAMILY_TYPE_ENT     devFamily = CPSS_PP_FAMILY_CHEETAH_E;
    GT_U16                      vlanIdIterator = 0;


    cpssOsBzero((GT_VOID*) &vlanInfo, sizeof(vlanInfo));
    cpssOsBzero((GT_VOID*) &portsMembersGet, sizeof(portsMembersGet));
    cpssOsBzero((GT_VOID*) &portsTaggingGet, sizeof(portsTaggingGet));
    cpssOsBzero((GT_VOID*) &vlanInfoGet, sizeof(vlanInfoGet));
    cpssOsBzero((GT_VOID*) &portsTaggingCmd, sizeof(portsTaggingCmd));
    cpssOsBzero((GT_VOID*) &portsTaggingCmdGet, sizeof(portsTaggingCmdGet));

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Getting device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /*
            1.1. Call with vlanId [100],
                   numOfEntries[10],
                   portsMemberPtr {[0, 16]},
                   portsTaggingPtr {[0, 16]},
                   vlanInfoPtr {unkSrcAddrSecBreach [GT_FALSE],
                                unregNonIpMcastCmd [CPSS_PACKET_CMD_FORWARD_E],
                                unregIpv4McastCmd [CPSS_PACKET_CMD_MIRROR_TO_CPU_E],
                                unregIpv6McastCmd [CPSS_PACKET_CMD_TRAP_TO_CPU_E],
                                unkUcastCmd [CPSS_PACKET_CMD_DROP_HARD_E],
                                unregIpv4BcastCmd [CPSS_PACKET_CMD_DROP_SOFT_E],
                                unregNonIpv4BcastCmd [CPSS_PACKET_CMD_FORWARD_E],
                                ipv4IgmpToCpuEn [GT_TRUE],
                                mirrToRxAnalyzerEn [GT_TRUE],
                                ipv6IcmpToCpuEn[GT_TRUE],
                                ipCtrlToCpuEn [CPSS_DXCH_BRG_IP_CTRL_NONE_E],
                                ipv4IpmBrgMode [CPSS_BRG_IPM_SGV_E],
                                ipv6IpmBrgMode [CPSS_BRG_IPM_GV_E],
                                ipv4IpmBrgEn[GT_TRUE],
                                ipv6IpmBrgEn [GT_TRUE],
                                ipv6SiteIdMode [CPSS_IP_SITE_ID_INTERNAL_E],
                                ipv4UcastRouteEn[GT_FALSE],
                                ipv4McastRouteEn [GT_FALSE],
                                ipv6UcastRouteEn[GT_FALSE],
                                ipv6McastRouteEn[GT_FALSE],
                                stgId[0],
                                utoLearnDisable
                                [GT_FALSE],
                                naMsgToCpuEn
                                [GT_FALSE],
                                mruIdx[0],
                                bcastUdpTrapMirrEn [GT_FALSE],
                                vrfId [0],

                                (next fields relevant Bobcat2; Caelum; Bobcat3 only)
                                mirrToRxAnalyzerIndex[1],
                                mirrToTxAnalyzerEn[GT_TRUE],
                                mirrToTxAnalyzerIndex[2],
                                fidValue[PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_FID_MAC(dev)],
                                unknownMacSaCmd[CPSS_PACKET_CMD_DROP_HARD_E],
                                ipv4McBcMirrToAnalyzerEn[GT_TRUE],
                                ipv4McBcMirrToAnalyzerIndex[3],
                                ipv6McMirrToAnalyzerEn[GT_TRUE],
                                ipv6McMirrToAnalyzerIndex[4]}.
            Expected: GT_OK.
        */
        vlanId       = BRG_VLAN_TESTED_VLAN_ID_CNS;
        numOfEntries = 10;

        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);
        portsMembers.ports[0] = 17;
        portsTagging.ports[0] = 17;

        if (IS_TR101_SUPPORTED_MAC(dev))
        {
            /* portsTaggingCmd relevant for TR101 enabled devices */
            portsTaggingCmd.portsCmd[0] = CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E;
            portsTaggingCmd.portsCmd[1] = CPSS_DXCH_BRG_VLAN_PORT_TAG1_CMD_E;
            portsTaggingCmd.portsCmd[2] = CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E;
            portsTaggingCmd.portsCmd[3] = CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E;
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            {
                portsTaggingCmd.portsCmd[8] = CPSS_DXCH_BRG_VLAN_PORT_DO_NOT_MODIFY_TAG_CMD_E;
            }
            else
            {
                portsTaggingCmd.portsCmd[8] = CPSS_DXCH_BRG_VLAN_PORT_PUSH_TAG0_CMD_E;
            }
            portsTaggingCmd.portsCmd[9] = CPSS_DXCH_BRG_VLAN_PORT_POP_OUTER_TAG_CMD_E;
            portsTaggingCmd.portsCmd[17] = CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E;
            portsTaggingCmd.portsCmd[22] = CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E;

            /* port tagging CMD is used */
            portsTaggingCmdPtr = &portsTaggingCmd;
            portsTaggingCmdGetPtr = &portsTaggingCmdGet;
        }
        else
        {
            /* port tagging CMD should be ignored */
            /* coverity[assign_zero] */ portsTaggingCmdPtr = portsTaggingCmdGetPtr = NULL;
        }

        vlanInfo.unkSrcAddrSecBreach  = GT_FALSE;
        vlanInfo.unregNonIpMcastCmd   = CPSS_PACKET_CMD_FORWARD_E;
        vlanInfo.unregIpv4McastCmd    = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
        vlanInfo.unregIpv6McastCmd    = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
        vlanInfo.unkUcastCmd          = CPSS_PACKET_CMD_DROP_HARD_E;
        vlanInfo.unregIpv4BcastCmd    = CPSS_PACKET_CMD_DROP_SOFT_E;
        vlanInfo.unregNonIpv4BcastCmd = CPSS_PACKET_CMD_FORWARD_E;
        vlanInfo.ipv4IgmpToCpuEn      = GT_TRUE;
        vlanInfo.mirrToRxAnalyzerEn   = GT_TRUE;
        vlanInfo.ipv6IcmpToCpuEn      = GT_TRUE;
        vlanInfo.ipCtrlToCpuEn        = CPSS_DXCH_BRG_IP_CTRL_NONE_E;
        vlanInfo.ipv4IpmBrgMode       = CPSS_BRG_IPM_SGV_E;
        vlanInfo.ipv6IpmBrgMode       = CPSS_BRG_IPM_GV_E;
        vlanInfo.ipv4IpmBrgEn         = GT_TRUE;
        vlanInfo.ipv6IpmBrgEn         = GT_TRUE;
        vlanInfo.ipv6SiteIdMode       = CPSS_IP_SITE_ID_INTERNAL_E;
        vlanInfo.ipv4UcastRouteEn     = GT_FALSE;
        vlanInfo.ipv4McastRouteEn     = GT_FALSE;
        vlanInfo.ipv6UcastRouteEn     = GT_FALSE;
        vlanInfo.ipv6McastRouteEn     = GT_FALSE;
        vlanInfo.stgId                = 0;
        vlanInfo.autoLearnDisable     = GT_FALSE;
        vlanInfo.naMsgToCpuEn         = GT_FALSE;
        vlanInfo.mruIdx               = 0;
        vlanInfo.bcastUdpTrapMirrEn   = GT_FALSE;
        vlanInfo.vrfId                = 100;

        if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            vlanInfo.mirrToRxAnalyzerIndex = 1;
            vlanInfo.mirrToTxAnalyzerEn = GT_TRUE;
            vlanInfo.mirrToTxAnalyzerIndex = 2;
            vlanInfo.mirrToTxAnalyzerMode = CPSS_DXCH_MIRROR_EGRESS_TAIL_DROP_E;
            vlanInfo.fidValue = UTF_CPSS_PP_MAX_FID_NUM_CNS(dev) - 1;
            vlanInfo.unknownMacSaCmd = CPSS_PACKET_CMD_DROP_HARD_E;
            vlanInfo.ipv4McBcMirrToAnalyzerEn = GT_TRUE;
            vlanInfo.ipv4McBcMirrToAnalyzerIndex = 3;
            vlanInfo.ipv6McMirrToAnalyzerEn = GT_TRUE;
            vlanInfo.ipv6McMirrToAnalyzerIndex = 4;
        }

        CPSS_COVERITY_NON_ISSUE_BOOKMARK
        /* coverity[var_deref_model] */
        st = cpssDxChBrgVlanEntriesRangeWrite(dev, vlanId, numOfEntries, &portsMembers,
                                              &portsTagging, &vlanInfo, portsTaggingCmdPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, vlanId);

        /*
            1.2. Call cpssDxChBrgVlanEntryRead with the same vlanId for each entry.
            Expected: GT_OK and the same portsMembersPtr, portsTaggingPtr and vlanInfoPtr.
            Check vlanInfo by fields:
                    1) skip ipv6SiteIdMode,
                            ipv4McastRateEn,
                            ipv6McastRateEn,
                            autoLearnDisable,
                            naMsgToCpuEn,
                            mruIdx,
                            bcastUdpTrapMirrEn fields for Cheetah,
                            but they should be checked for Cheetah2;
                    2) ipv4IpmBrgMode and ipv6IpnBrgMode
                       should be checked only if corresponding ipv4/6IpmBrgEn = GT_TRUE.
                    3) for Bobcat2; Caelum; Bobcat3 check also mirrToRxAnalyzerIndex, mirrToTxAnalyzerEn,
                        mirrToTxAnalyzerIndex, fidValue, unknownMacSaCmd,
                        ipv4McBcMirrToAnalyzerEn, ipv4McBcMirrToAnalyzerIndex,
                    ipv6McMirrToAnalyzerEn, ipv6McMirrToAnalyzerIndex.
        */
        for (vlanIdIterator = vlanId; vlanIdIterator < (vlanId + numOfEntries); vlanIdIterator++)
        {
            CPSS_COVERITY_NON_ISSUE_BOOKMARK
            /* coverity[var_deref_model] */
            st = cpssDxChBrgVlanEntryRead(dev, vlanIdIterator, &portsMembersGet, &portsTaggingGet,
                                          &vlanInfoGet, &isValid, portsTaggingCmdGetPtr);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgVlanEntryRead: %d, %d", dev, vlanIdIterator);

            UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isValid,
                    "Vlan entry is NOT valid: %d, %d", dev, vlanIdIterator);

            if (GT_TRUE == isValid)
            {
                /* Verifying portsMembersPtr */
                isEqual = (0 == cpssOsMemCmp((GT_VOID*) &portsMembers,
                                             (GT_VOID*) &portsMembersGet,
                                             sizeof(portsMembers))) ? GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
                   "get another portsMembersPtr than was set: %d, %d", dev, vlanIdIterator);

                if (IS_TR101_SUPPORTED_MAC(dev))
                {
                    /* Verifying portsTaggingCmd */
                    isEqual = (0 == cpssOsMemCmp((GT_VOID*) &portsTaggingCmd,
                                                 (GT_VOID*) &portsTaggingCmdGet,
                                                 sizeof(portsTaggingCmd))) ? GT_TRUE : GT_FALSE;
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
                               "get another portsTaggingCmd than was set: %d, %d", dev, vlanId);
                }
                else
                {
                    /* Verifying portsTaggingPtr */
                    isEqual = (0 == cpssOsMemCmp((GT_VOID*) &portsTagging,
                                                 (GT_VOID*) &portsTaggingGet,
                                                 sizeof(portsTagging))) ? GT_TRUE : GT_FALSE;
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
                               "get another portsTaggingPtr than was set: %d, %d", dev, vlanId);
                }

                /* Verifying vlanInfoPtr fields */
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.unkSrcAddrSecBreach,
                                             vlanInfoGet.unkSrcAddrSecBreach,
                 "get another vlanInfoPtr->unkSrcAddrSecBreach than was set: %d, %d", dev, vlanIdIterator);
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.unregNonIpMcastCmd,
                                             vlanInfoGet.unregNonIpMcastCmd,
                 "get another vlanInfoPtr->unregNonIpMcastCmd than was set: %d, %d", dev, vlanIdIterator);
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.unregIpv4McastCmd,
                                             vlanInfoGet.unregIpv4McastCmd,
                 "get another vlanInfoPtr->unregIpv4McastCmd than was set: %d, %d", dev, vlanIdIterator);
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.unregIpv6McastCmd,
                                             vlanInfoGet.unregIpv6McastCmd,
                 "get another vlanInfoPtr->unregIpv6McastCmd than was set: %d, %d", dev, vlanIdIterator);
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.unkUcastCmd,
                                             vlanInfoGet.unkUcastCmd,
                 "get another vlanInfoPtr->unkUcastCmd than was set: %d, %d", dev, vlanIdIterator);
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.unregIpv4BcastCmd,
                                             vlanInfoGet.unregIpv4BcastCmd,
                 "get another vlanInfoPtr->unregIpv4BcastCmd than was set: %d, %d", dev, vlanIdIterator);
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.unregNonIpv4BcastCmd,
                                             vlanInfoGet.unregNonIpv4BcastCmd,
                 "get another vlanInfoPtr->unregNonIpv4BcastCmd than was set: %d, %d", dev, vlanIdIterator);
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.ipv4IgmpToCpuEn,
                                             vlanInfoGet.ipv4IgmpToCpuEn,
                 "get another vlanInfoPtr->ipv4IgmpToCpuEn than was set: %d, %d", dev, vlanIdIterator);
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.mirrToRxAnalyzerEn,
                                             vlanInfoGet.mirrToRxAnalyzerEn,
                 "get another vlanInfoPtr->mirrToRxAnalyzerEn than was set: %d, %d", dev, vlanIdIterator);
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.ipv6IcmpToCpuEn,
                                             vlanInfoGet.ipv6IcmpToCpuEn,
                 "get another vlanInfoPtr->ipv6IcmpToCpuEn than was set: %d, %d", dev, vlanIdIterator);
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.ipCtrlToCpuEn,
                                             vlanInfoGet.ipCtrlToCpuEn,
                 "get another vlanInfoPtr->ipCtrlToCpuEn than was set: %d, %d", dev, vlanIdIterator);
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.ipv4IpmBrgEn,
                                             vlanInfoGet.ipv4IpmBrgEn,
                 "get another vlanInfoPtr->ipv4IpmBrgEn than was set: %d, %d", dev, vlanIdIterator);
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.ipv6IpmBrgEn,
                                             vlanInfoGet.ipv6IpmBrgEn,
                 "get another vlanInfoPtr->ipv6IpmBrgEn than was set: %d, %d", dev, vlanIdIterator);

                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.ipv4IpmBrgMode,
                                             vlanInfoGet.ipv4IpmBrgMode,
                 "get another vlanInfoPtr->ipv4IpmBrgMode than was set: %d, %d", dev, vlanIdIterator);

                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.ipv6IpmBrgMode,
                                             vlanInfoGet.ipv6IpmBrgMode,
                 "get another vlanInfoPtr->ipv6IpmBrgMode than was set: %d, %d", dev, vlanIdIterator);

                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.ipv4UcastRouteEn,
                                             vlanInfoGet.ipv4UcastRouteEn,
                 "get another vlanInfoPtr->ipv4UcastRouteEn than was set: %d, %d", dev, vlanIdIterator);
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.ipv6UcastRouteEn,
                                             vlanInfoGet.ipv6UcastRouteEn,
                 "get another vlanInfoPtr->ipv6UcastRouteEn than was set: %d, %d", dev, vlanIdIterator);
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.stgId,
                                             vlanInfoGet.stgId,
                 "get another vlanInfoPtr->stgId than was set: %d, %d", dev, vlanIdIterator);

                if (devFamily != CPSS_PP_FAMILY_CHEETAH_E)
                {
                    if (devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E)
                    {
                        UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.ipv6SiteIdMode,
                                                     vlanInfoGet.ipv6SiteIdMode,
                         "get another vlanInfoPtr->ipv6SiteIdMode than was set: %d, %d", dev, vlanIdIterator);
                        UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.ipv4McastRouteEn,
                                                     vlanInfoGet.ipv4McastRouteEn,
                         "get another vlanInfoPtr->ipv4McastRouteEn than was set: %d, %d", dev, vlanIdIterator);
                        UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.ipv6McastRouteEn,
                                                     vlanInfoGet.ipv6McastRouteEn,
                         "get another vlanInfoPtr->ipv6McastRouteEn than was set: %d, %d", dev, vlanIdIterator);
                    }
                    UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.autoLearnDisable,
                                                 vlanInfoGet.autoLearnDisable,
                     "get another vlanInfoPtr->autoLearnDisable than was set: %d, %d", dev, vlanIdIterator);
                    UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.naMsgToCpuEn,
                                                 vlanInfoGet.naMsgToCpuEn,
                     "get another vlanInfoPtr->naMsgToCpuEn than was set: %d, %d", dev, vlanIdIterator);
                    UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.mruIdx,
                                                 vlanInfoGet.mruIdx,
                     "get another vlanInfoPtr->mruIdx than was set: %d, %d", dev, vlanIdIterator);
                    UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.bcastUdpTrapMirrEn,
                                                 vlanInfoGet.bcastUdpTrapMirrEn,
                     "get another vlanInfoPtr->bcastUdpTrapMirrEn than was set: %d, %d", dev, vlanIdIterator);
                }

                if ((devFamily >= CPSS_PP_FAMILY_CHEETAH3_E) && (devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E))
                {
                    UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.vrfId,
                                                 vlanInfoGet.vrfId,
                     "get another vlanInfoPtr->vrfId than was set: %d, %d", dev, vlanIdIterator);
                }

                if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
                {
                    UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.mirrToRxAnalyzerIndex,
                                                 vlanInfoGet.mirrToRxAnalyzerIndex,
                     "get another vlanInfoPtr->mirrToRxAnalyzerIndex than was set: %d, %d", dev, vlanId);

                    UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.mirrToTxAnalyzerEn,
                                                 vlanInfoGet.mirrToTxAnalyzerEn,
                     "get another vlanInfoPtr->mirrToTxAnalyzerEn than was set: %d, %d", dev, vlanId);

                    UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.mirrToTxAnalyzerIndex,
                                                 vlanInfoGet.mirrToTxAnalyzerIndex,
                     "get another vlanInfoPtr->mirrToTxAnalyzerIndex than was set: %d, %d", dev, vlanId);

                    UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.fidValue,
                                                 vlanInfoGet.fidValue,
                     "get another vlanInfoPtr->fidValue than was set: %d, %d", dev, vlanId);

                    UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.unknownMacSaCmd,
                                                 vlanInfoGet.unknownMacSaCmd,
                     "get another vlanInfoPtr->unknownMacSaCmd than was set: %d, %d", dev, vlanId);

                    UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.ipv4McBcMirrToAnalyzerEn,
                                                 vlanInfoGet.ipv4McBcMirrToAnalyzerEn,
                     "get another vlanInfoPtr->ipv4McBcMirrToAnalyzerEn than was set: %d, %d", dev, vlanId);

                    UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.ipv4McBcMirrToAnalyzerIndex,
                                                 vlanInfoGet.ipv4McBcMirrToAnalyzerIndex,
                     "get another vlanInfoPtr->ipv4McBcMirrToAnalyzerIndex than was set: %d, %d", dev, vlanId);

                    UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.ipv6McMirrToAnalyzerEn,
                                                 vlanInfoGet.ipv6McMirrToAnalyzerEn,
                     "get another vlanInfoPtr->ipv6McMirrToAnalyzerEn than was set: %d, %d", dev, vlanId);

                    UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.ipv6McMirrToAnalyzerIndex,
                                                 vlanInfoGet.ipv6McMirrToAnalyzerIndex,
                     "get another vlanInfoPtr->ipv6McMirrToAnalyzerIndex than was set: %d, %d", dev, vlanId);
                }

                if (PRV_CPSS_SIP_6_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.mirrToTxAnalyzerMode,
                                                 vlanInfoGet.mirrToTxAnalyzerMode,
                                                 "get another vlanInfoPtr->mirrToTxAnalyzerMode than was set: %d, %d", dev, vlanId);
                }

            }
        }

        /*
            1.3. Call with wrong enum values ipv4IpmBrgMode,
                          ipv4IpmBrgEn [GT_FALSE] (in this case ipv4IpmBrgMode is not relevant)
                          and other parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */
        vlanInfo.ipv4IpmBrgEn = GT_FALSE;

        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanEntriesRangeWrite(dev, vlanId, numOfEntries, &portsMembers,
                                              &portsTagging, &vlanInfo, portsTaggingCmdPtr),
                                     vlanInfo.ipv4IpmBrgMode);

        /*
            1.4. Call with wrong enum values ipv4IpmBrgMode,
                           ipv4IpmBrgEn [GT_TRUE] (in this case ipv4IpmBrgMode is relevant)
                           and other parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */
        vlanInfo.ipv4IpmBrgEn = GT_TRUE;

        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanEntriesRangeWrite
                                                        (dev, vlanId, numOfEntries, &portsMembers,
                             &portsTagging, &vlanInfo, portsTaggingCmdPtr),
                                                        vlanInfo.ipv4IpmBrgMode);

        /*
            1.5. Call with wrong enum values ipv6IpmBrgMode,
                          ipv6IpmBrgEn [GT_FALSE] (in this case ipv6IpmBrgMode is not relevant)
                          and other parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */
        vlanInfo.ipv6IpmBrgEn = GT_FALSE;

        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanEntriesRangeWrite(dev, vlanId, numOfEntries, &portsMembers,
                                              &portsTagging, &vlanInfo, portsTaggingCmdPtr),
                                     vlanInfo.ipv6IpmBrgMode);

        /*
            1.6. Call with wrong enum values ipv6IpmBrgMode,
                               ipv6IpmBrgEn [GT_TRUE] (in this case ipv6IpmBrgMode is relevant)
                               and other parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */
        vlanInfo.ipv6IpmBrgEn = GT_TRUE;

        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanEntriesRangeWrite
                                                        (dev, vlanId, numOfEntries, &portsMembers,
                             &portsTagging, &vlanInfo, portsTaggingCmdPtr),
                                                        vlanInfo.ipv6IpmBrgMode);

        /*
            1.7. Call with ipCtrlToCpuEn [CPSS_DXCH_BRG_IP_CTRL_IPV4_E]
                               and other parameters from 1.1.
            Expected: GT_OK for DxCh2, DxCh3 and NOT GT_OK for others.
        */
        vlanInfo.ipCtrlToCpuEn = CPSS_DXCH_BRG_IP_CTRL_IPV4_E;

        st = cpssDxChBrgVlanEntriesRangeWrite(dev, vlanId, numOfEntries, &portsMembers,
                                              &portsTagging, &vlanInfo, portsTaggingCmdPtr);

        if (devFamily != CPSS_PP_FAMILY_CHEETAH_E)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->ipCtrlToCpuEn = %d",
                                         dev, vlanInfo.ipCtrlToCpuEn);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->ipCtrlToCpuEn = %d",
                                             dev, vlanInfo.ipCtrlToCpuEn);
        }

        /*
            1.8. Call with ipCtrlToCpuEn [CPSS_DXCH_BRG_IP_CTRL_IPV6_E]
                      and other parameters from 1.1.
            Expected: GT_OK for DxCh2, DxCh3 and NOT GT_OK for others.
        */
        vlanInfo.ipCtrlToCpuEn = CPSS_DXCH_BRG_IP_CTRL_IPV6_E;

        st = cpssDxChBrgVlanEntriesRangeWrite(dev, vlanId, numOfEntries, &portsMembers,
                                              &portsTagging, &vlanInfo, portsTaggingCmdPtr);

        if (devFamily != CPSS_PP_FAMILY_CHEETAH_E)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->ipCtrlToCpuEn = %d",
                                         dev, vlanInfo.ipCtrlToCpuEn);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->ipCtrlToCpuEn = %d",
                                             dev, vlanInfo.ipCtrlToCpuEn);
        }

        vlanInfo.ipCtrlToCpuEn = CPSS_DXCH_BRG_IP_CTRL_NONE_E;

        /*
            1.9. Call with out of range vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)]
                           and other parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */
        vlanId = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev);

        st = cpssDxChBrgVlanEntriesRangeWrite(dev, vlanId, numOfEntries, &portsMembers,
                                              &portsTagging, &vlanInfo, portsTaggingCmdPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, vlanId);

        vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;

        /*
            1.10. Call with vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev) - 1],
                            numOfEntries [1] and other parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */
        vlanId       = (GT_U16) (UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev) - 1);
        numOfEntries = 2;

        st = cpssDxChBrgVlanEntriesRangeWrite(dev, vlanId, numOfEntries, &portsMembers,
                                              &portsTagging, &vlanInfo, portsTaggingCmdPtr);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, vlanId, numOfEntries);

        vlanId       = BRG_VLAN_TESTED_VLAN_ID_CNS;
        numOfEntries = 10;

        /*
            1.11. Call with wrong enum values unregNonIpMcastCmd and other parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanEntriesRangeWrite
                                                        (dev, vlanId, numOfEntries, &portsMembers,
                             &portsTagging, &vlanInfo, portsTaggingCmdPtr),
                                                        vlanInfo.unregNonIpMcastCmd);

        vlanInfo.unregNonIpMcastCmd = CPSS_PACKET_CMD_FORWARD_E;

        /*
            1.12. Call with wrong enum values unregIpv4McastCmd and other parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanEntriesRangeWrite
                                                        (dev, vlanId, numOfEntries, &portsMembers,
                             &portsTagging, &vlanInfo, portsTaggingCmdPtr),
                                                        vlanInfo.unregIpv4McastCmd);

        vlanInfo.unregIpv4McastCmd = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;

        /*
            1.13. Call with wrong enum values unregIpv6McastCmd and other parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanEntriesRangeWrite
                                                        (dev, vlanId, numOfEntries, &portsMembers,
                             &portsTagging, &vlanInfo, portsTaggingCmdPtr),
                                                        vlanInfo.unregIpv6McastCmd);

        vlanInfo.unregIpv6McastCmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;

        /*
            1.14. Call with wrong enum values unkUcastCmd and other parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanEntriesRangeWrite
                                                        (dev, vlanId, numOfEntries, &portsMembers,
                             &portsTagging, &vlanInfo, portsTaggingCmdPtr),
                                                        vlanInfo.unkUcastCmd);

        vlanInfo.unkUcastCmd = CPSS_PACKET_CMD_DROP_HARD_E;

        /*
            1.15. Call with wrong enum values unregIpv4BcastCmd and other parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanEntriesRangeWrite
                                                        (dev, vlanId, numOfEntries, &portsMembers,
                             &portsTagging, &vlanInfo, portsTaggingCmdPtr),
                                                        vlanInfo.unregIpv4BcastCmd);

        vlanInfo.unregIpv4BcastCmd = CPSS_PACKET_CMD_DROP_SOFT_E;

        /*
            1.16. Call with wrong enum values unregNonIpv4BcastCmd and other parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanEntriesRangeWrite
                                                        (dev, vlanId, numOfEntries, &portsMembers,
                             &portsTagging, &vlanInfo, portsTaggingCmdPtr),
                                                        vlanInfo.unregNonIpv4BcastCmd);

        vlanInfo.unregNonIpv4BcastCmd = CPSS_PACKET_CMD_FORWARD_E;

        /*
            1.17. Call with wrong enum values ipCtrlToCpuEn and other parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanEntriesRangeWrite
                                                        (dev, vlanId, numOfEntries, &portsMembers,
                             &portsTagging, &vlanInfo, portsTaggingCmdPtr),
                                                        vlanInfo.ipCtrlToCpuEn);

        vlanInfo.ipCtrlToCpuEn = CPSS_DXCH_BRG_IP_CTRL_NONE_E;

        /*
            1.18. Call with wrong enum values ipv6SiteIdMode and other parameters from 1.1.
            Expected: GT_BAD_PARAM for DxCh2, DxCh3 and GT_OK for others
                      (supported only by Cheetah2,3).
        */
        if ((devFamily != CPSS_PP_FAMILY_CHEETAH_E) && (devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E))
        {
            UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanEntriesRangeWrite
                                (dev, vlanId, numOfEntries, &portsMembers,
                                 &portsTagging, &vlanInfo, portsTaggingCmdPtr),
                                vlanInfo.ipv6SiteIdMode);
        }
        else
        {
            for(index = 0; index < utfInvalidEnumArrSize; index++)
            {
                vlanInfo.ipv6SiteIdMode = utfInvalidEnumArr[index];

                st = cpssDxChBrgVlanEntriesRangeWrite(dev, vlanId, numOfEntries, &portsMembers,
                                                      &portsTagging, &vlanInfo, portsTaggingCmdPtr);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->ipv6SiteIdMode = %d",
                                             dev, vlanInfo.ipv6SiteIdMode);
            }
        }

        vlanInfo.ipv6SiteIdMode = CPSS_IP_SITE_ID_INTERNAL_E;

        /*
            1.19. Call with vlanId [100]
                            and portsMembersPtr [NULL]
                            and other parameters from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgVlanEntriesRangeWrite(dev, vlanId, numOfEntries, NULL,
                                              &portsTagging, &vlanInfo, portsTaggingCmdPtr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, portsMembersPtr = NULL", dev);

        /*
            1.20. Call with vlanId [100]
                           and portsTaggingPtr [NULL]
                           and other parameters from 1.1.
            Expected: GT_BAD_PTR for TR101 disabled devices,
                      GT_OK for TR101 enabled devices.
        */
        st = cpssDxChBrgVlanEntriesRangeWrite(dev, vlanId, numOfEntries, &portsMembers,
                                              NULL, &vlanInfo, portsTaggingCmdPtr);
        if(IS_TR101_SUPPORTED_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "%d, portsTaggingPtr = NULL", dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, portsTaggingPtr = NULL", dev);
        }

        /*
            1.20.1 Call with vlanId [100]
                           and portsTaggingCmdPtr [NULL]
                           and other parameters from 1.1.
            Expected: GT_BAD_PTR for TR101 enabled devices,
                      GT_OK for TR101 disabled devices.
        */
        st = cpssDxChBrgVlanEntriesRangeWrite(dev, vlanId, numOfEntries, &portsMembers,
                                       &portsTagging, &vlanInfo, NULL);
        if(!IS_TR101_SUPPORTED_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "%d, portsTaggingCmdPtr = NULL", dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, portsTaggingCmdPtr = NULL", dev);
        }

        /*
            1.21. Call with vlanId [100] and vlanInfoPtr [NULL]
                           and other parameters from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgVlanEntriesRangeWrite(dev, vlanId, numOfEntries, &portsMembers,
                                              &portsTagging, NULL, portsTaggingCmdPtr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, vlanInfoPtr = NULL", dev);

        /*
            1.21. Call with out of range stgId [256] and other parameters from 1.1.
            Expected: NON GT_OK.
        */
        vlanInfo.stgId = MAX_STG_ID_MAC(dev);

        st = cpssDxChBrgVlanEntriesRangeWrite(dev, vlanId, numOfEntries, &portsMembers,
                                              &portsTagging, &vlanInfo, portsTaggingCmdPtr);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->stgId = %d",
                                         dev, vlanInfo.stgId);
        vlanInfo.stgId = 0;

        /*
            1.22. Call with mruIdx [PRV_CPSS_DXCH_BRG_MRU_INDEX_MAX_CNS+1=8]
                       and other parameters from 1.1.
            Expected: non GT_OK.
        */
        vlanInfo.mruIdx = 8;

        st = cpssDxChBrgVlanEntriesRangeWrite(dev, vlanId, numOfEntries, &portsMembers,
                                              &portsTagging, &vlanInfo, portsTaggingCmdPtr);
        if(devFamily != CPSS_PP_FAMILY_CHEETAH_E)
        {
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->mruIdx = %d",
                                         dev, vlanInfo.mruIdx);
        }
        else
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->mruIdx = %d",
                                         dev, vlanInfo.mruIdx);

        }

        vlanInfo.mruIdx = 0;

        /*
            1.23. Call with out of range mirrToRxAnalyzerIndex[7] and other parameters from 1.1.
            Expected: NON GT_OK for Bobcat2; Caelum; Bobcat3 and GT_OK for others (supported only by Bobcat2; Caelum; Bobcat3).
        */
        vlanInfo.mirrToRxAnalyzerIndex = 7;

        st = cpssDxChBrgVlanEntriesRangeWrite(dev, vlanId, numOfEntries, &portsMembers,
                                       &portsTagging, &vlanInfo, portsTaggingCmdPtr);

        if (0 == UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->mirrToRxAnalyzerIndex = %d",
                                         dev, vlanInfo.mirrToRxAnalyzerIndex);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->mirrToRxAnalyzerIndex = %d",
                                             dev, vlanInfo.mirrToRxAnalyzerIndex);
        }

        /* restore previous value */
        vlanInfo.mirrToRxAnalyzerIndex = 1;

        /*
            1.24. Call with mirrToTxAnalyzerEn[GT_FALSE],
                out of range mirrToTxAnalyzerIndex[7](not relevant) and
                other parameters from 1.1.
            Expected: GT_OK.
        */
        vlanInfo.mirrToTxAnalyzerIndex = 7;
        vlanInfo.mirrToTxAnalyzerEn = GT_FALSE;
        vlanInfo.mirrToTxAnalyzerMode = CPSS_DXCH_MIRROR_EGRESS_CONGESTION_E;

        st = cpssDxChBrgVlanEntriesRangeWrite(dev, vlanId, numOfEntries, &portsMembers,
                                       &portsTagging, &vlanInfo, portsTaggingCmdPtr);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->mirrToTxAnalyzerIndex = %d",
                                     dev, vlanInfo.mirrToTxAnalyzerIndex);

        /*
            1.25. Call with mirrToTxAnalyzerEn[GT_TRUE],
                out of range mirrToTxAnalyzerIndex[7](is relevant) and other parameters from 1.1.
            Expected: NON GT_OK for Bobcat2; Caelum; Bobcat3 and GT_OK for others.
        */
        vlanInfo.mirrToTxAnalyzerIndex = 7;
        vlanInfo.mirrToTxAnalyzerEn = GT_TRUE;

        st = cpssDxChBrgVlanEntriesRangeWrite(dev, vlanId, numOfEntries, &portsMembers,
                                       &portsTagging, &vlanInfo, portsTaggingCmdPtr);

        if (0 == UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->mirrToTxAnalyzerIndex = %d",
                                         dev, vlanInfo.mirrToTxAnalyzerIndex);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->mirrToTxAnalyzerIndex = %d",
                                             dev, vlanInfo.mirrToTxAnalyzerIndex);
        }

        vlanInfo.mirrToTxAnalyzerMode = CPSS_DXCH_MIRROR_EGRESS_CONGESTION_OR_TAIL_DROP_E;
        st = cpssDxChBrgVlanEntriesRangeWrite(dev, vlanId, numOfEntries, &portsMembers,
                                       &portsTagging, &vlanInfo, portsTaggingCmdPtr);

        if (0 == UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->mirrToTxAnalyzerIndex = %d",
                                         dev, vlanInfo.mirrToTxAnalyzerMode);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->mirrToTxAnalyzerIndex = %d",
                dev, vlanInfo.mirrToTxAnalyzerMode);
        }

        /* restore previous value */
        vlanInfo.mirrToTxAnalyzerIndex = 2;

        /*
            1.26. Call with out of range fidValue[PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_FID_MAC(dev)+1]
                and other parameters from 1.1.
            Expected: NON GT_OK for Bobcat2; Caelum; Bobcat3 and GT_OK for others (supported only by Bobcat2; Caelum; Bobcat3).
        */
        vlanInfo.fidValue = UTF_CPSS_PP_MAX_FID_NUM_CNS(dev);

        st = cpssDxChBrgVlanEntriesRangeWrite(dev, vlanId, numOfEntries, &portsMembers,
                                       &portsTagging, &vlanInfo, portsTaggingCmdPtr);

        if (0 == UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->fidValue = %d",
                                         dev, vlanInfo.fidValue);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->fidValue = %d",
                                             dev, vlanInfo.fidValue);
        }

        /* restore previous value */
        vlanInfo.fidValue = UTF_CPSS_PP_MAX_FID_NUM_CNS(dev) - 1;

        /*
            1.27. Call with out of range unknownMacSaCmd[0x5AAAAAA5] and other parameters from 1.1.
            Expected: NON GT_OK for Bobcat2; Caelum; Bobcat3 and GT_OK for others (supported only by Bobcat2; Caelum; Bobcat3).
        */
        vlanInfo.unknownMacSaCmd = 0x5AAAAAA5;

        st = cpssDxChBrgVlanEntriesRangeWrite(dev, vlanId, numOfEntries, &portsMembers,
                                       &portsTagging, &vlanInfo, portsTaggingCmdPtr);

        if (0 == UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->unknownMacSaCmd = %d",
                                         dev, vlanInfo.unknownMacSaCmd);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->unknownMacSaCmd = %d",
                                             dev, vlanInfo.unknownMacSaCmd);
        }

        /* restore previous value */
        vlanInfo.unknownMacSaCmd = CPSS_PACKET_CMD_DROP_HARD_E;

        /*
            1.28. Call with ipv4McBcMirrToAnalyzerEn[GT_FALSE],
                out of range ipv4McBcMirrToAnalyzerIndex[7](not relevant) and
                other parameters from 1.1.
            Expected: GT_OK.
        */
        vlanInfo.ipv4McBcMirrToAnalyzerIndex = 7;
        vlanInfo.ipv4McBcMirrToAnalyzerEn = GT_FALSE;

        st = cpssDxChBrgVlanEntriesRangeWrite(dev, vlanId, numOfEntries, &portsMembers,
                                       &portsTagging, &vlanInfo, portsTaggingCmdPtr);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->ipv4McBcMirrToAnalyzerIndex = %d",
                                     dev, vlanInfo.ipv4McBcMirrToAnalyzerIndex);

        /*
            1.29. Call with ipv4McBcMirrToAnalyzerEn[GT_TRUE](is relevant),
                out of range ipv4McBcMirrToAnalyzerIndex[7] and
                other parameters from 1.1.
            Expected: NON GT_OK for Bobcat2; Caelum; Bobcat3 and GT_OK.
        */
        vlanInfo.ipv4McBcMirrToAnalyzerIndex = 7;
        vlanInfo.ipv4McBcMirrToAnalyzerEn = GT_TRUE;

        st = cpssDxChBrgVlanEntriesRangeWrite(dev, vlanId, numOfEntries, &portsMembers,
                                       &portsTagging, &vlanInfo, portsTaggingCmdPtr);

        if (0 == UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->ipv4McBcMirrToAnalyzerIndex = %d",
                                         dev, vlanInfo.ipv4McBcMirrToAnalyzerIndex);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->ipv4McBcMirrToAnalyzerIndex = %d",
                                             dev, vlanInfo.ipv4McBcMirrToAnalyzerIndex);
        }

        /* restore previous value */
        vlanInfo.ipv4McBcMirrToAnalyzerIndex = 3;

        /*
            1.30. Call with ipv6McMirrToAnalyzerEn[GT_FALSE],
                out of range ipv6McMirrToAnalyzerIndex[7](not relevant) and
                other parameters from 1.1.
            Expected: GT_OK.
        */
        vlanInfo.ipv6McMirrToAnalyzerIndex = 7;
        vlanInfo.ipv6McMirrToAnalyzerEn = GT_FALSE;

        st = cpssDxChBrgVlanEntriesRangeWrite(dev, vlanId, numOfEntries, &portsMembers,
                                       &portsTagging, &vlanInfo, portsTaggingCmdPtr);

        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->ipv6McMirrToAnalyzerIndex = %d",
                                     dev, vlanInfo.ipv6McMirrToAnalyzerIndex);

        /*
            1.31. Call with ipv6McMirrToAnalyzerEn[GT_TRUE],
                out of range ipv6McMirrToAnalyzerIndex[7](is relevant) and
                other parameters from 1.1.
            Expected: NON GT_OK for Bobcat2; Caelum; Bobcat3 and GT_OK for others.
        */
        vlanInfo.ipv6McMirrToAnalyzerIndex = 7;
        vlanInfo.ipv6McMirrToAnalyzerEn = GT_TRUE;

        st = cpssDxChBrgVlanEntriesRangeWrite(dev, vlanId, numOfEntries, &portsMembers,
                                       &portsTagging, &vlanInfo, portsTaggingCmdPtr);

        if (0 == UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->ipv6McMirrToAnalyzerIndex = %d",
                                         dev, vlanInfo.ipv6McMirrToAnalyzerIndex);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->ipv6McMirrToAnalyzerIndex = %d",
                                             dev, vlanInfo.ipv6McMirrToAnalyzerIndex);
        }

        /* restore previous value */
        vlanInfo.ipv6McMirrToAnalyzerIndex = 4;


        /*
            1.32. Call with out of range vrfId [4096]
                            and other parameters from 1.1.
            Expected: NON GT_OK for DxCh3 and GT_OK for others (supported only by Cheetah3)
        */
        vlanInfo.vrfId = 4096;

        st = cpssDxChBrgVlanEntriesRangeWrite(dev, vlanId, numOfEntries, &portsMembers,
                                       &portsTagging, &vlanInfo, portsTaggingCmdPtr);

        if ((devFamily >= CPSS_PP_FAMILY_CHEETAH3_E) && (devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E))
        {
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->vrfId = %d",
                                             dev, vlanInfo.vrfId);
        }
        else
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, vlanInfoPtr->vrfId = %d",
                                         dev, vlanInfo.vrfId);
        }

        vlanInfo.vrfId = 0;
        /* clear all members for the test of cpssDxChBrgVlanIsDevMember*/
        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
        st = cpssDxChBrgVlanEntryWrite(dev, vlanId, &portsMembers,
                                       &portsTagging, &vlanInfo, portsTaggingCmdPtr);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgVlanEntryWrite: %d, %d", dev, vlanId);

        /*
            1.33. Call cpssDxChBrgVlanEntryInvalidate with the same vlanId
                  to invalidate all changes.
            Expected: GT_OK.
        */
        st = cpssDxChBrgVlanEntryInvalidate(dev, vlanId);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgVlanEntryInvalidate: %d, %d", dev, vlanId);
    }

    vlanId       = BRG_VLAN_TESTED_VLAN_ID_CNS;
    numOfEntries = 10;

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);
    portsMembers.ports[0] = 1000;
    portsTagging.ports[0] = 1500;
    portsTagging.ports[1] = 0;

    vlanInfo.unkSrcAddrSecBreach  = GT_FALSE;
    vlanInfo.unregNonIpMcastCmd   = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregIpv4McastCmd    = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
    vlanInfo.unregIpv6McastCmd    = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    vlanInfo.unkUcastCmd          = CPSS_PACKET_CMD_DROP_HARD_E;
    vlanInfo.unregIpv4BcastCmd    = CPSS_PACKET_CMD_DROP_SOFT_E;
    vlanInfo.unregNonIpv4BcastCmd = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.ipv4IgmpToCpuEn      = GT_TRUE;
    vlanInfo.mirrToRxAnalyzerEn   = GT_TRUE;
    vlanInfo.ipv6IcmpToCpuEn      = GT_TRUE;
    vlanInfo.ipCtrlToCpuEn        = CPSS_DXCH_BRG_IP_CTRL_NONE_E;
    vlanInfo.ipv4IpmBrgMode       = CPSS_BRG_IPM_SGV_E;
    vlanInfo.ipv6IpmBrgMode       = CPSS_BRG_IPM_GV_E;
    vlanInfo.ipv4IpmBrgEn         = GT_TRUE;
    vlanInfo.ipv6IpmBrgEn         = GT_TRUE;
    vlanInfo.ipv6SiteIdMode       = CPSS_IP_SITE_ID_INTERNAL_E;
    vlanInfo.ipv4UcastRouteEn     = GT_FALSE;
    vlanInfo.ipv4McastRouteEn     = GT_FALSE;
    vlanInfo.ipv6UcastRouteEn     = GT_FALSE;
    vlanInfo.ipv6McastRouteEn     = GT_FALSE;
    vlanInfo.stgId                = 0;
    vlanInfo.autoLearnDisable     = GT_FALSE;
    vlanInfo.naMsgToCpuEn         = GT_FALSE;
    vlanInfo.mruIdx               = 0;
    vlanInfo.bcastUdpTrapMirrEn   = GT_FALSE;
    vlanInfo.vrfId                = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanEntriesRangeWrite(dev, vlanId, numOfEntries, &portsMembers,
                                              &portsTagging, &vlanInfo, portsTaggingCmdPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanEntriesRangeWrite(dev, vlanId, numOfEntries, &portsMembers,
                                              &portsTagging, &vlanInfo, portsTaggingCmdPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanEntryRead
(
    IN  GT_U8                                devNum,
    IN  GT_U16                               vlanId,
    OUT CPSS_PORTS_BMP_STC                   *portsMembersPtr,
    OUT CPSS_PORTS_BMP_STC                   *portsTaggingPtr,
    OUT CPSS_DXCH_BRG_VLAN_INFO_STC          *vlanInfoPtr,
    OUT GT_BOOL                              *isValidPtr,
    OUT CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC *portsTaggingCmdPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanEntryRead)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with vlanId [100],
                   non-null portsMemberPtr,
                   non-null portsTaggingPtr,
                   non-null vlanInfoPtr,
                   non-null isValidPtr.
    Expected: GT_OK.
    1.2. Call with out of range vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)]
                   and non_null other parameters.
    Expected: GT_BAD_PARAM.
    1.3. Call with vlanId [100],
                   portsMembersPtr [NULL]
                   and non_null other parameters.
    Expected: GT_BAD_PTR.
    1.4. Call with vlanId [100],
                   portsTaggingPtr [NULL]
                   and non_null other parameters.
    Expected: GT_BAD_PTR.
    1.5. Call with vlanId [100],
                   vlanInfoPtr [NULL]
                   and non_null other parameters
    Expected: GT_BAD_PTR.
    1.6. Call with vlanId [100],
                   isValidPtr [NULL]
                   and non_null other parameters.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8                       dev;
    GT_U16                      vlanId = 0;
    CPSS_PORTS_BMP_STC          portsMembers;
    CPSS_PORTS_BMP_STC          portsTagging;
    CPSS_DXCH_BRG_VLAN_INFO_STC vlanInfo;
    GT_BOOL                     isValid = GT_FALSE;

    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;

    cpssOsBzero((GT_VOID*) &portsMembers, sizeof(portsMembers));
    cpssOsBzero((GT_VOID*) &portsTagging, sizeof(portsTagging));
    cpssOsBzero((GT_VOID*) &vlanInfo, sizeof(vlanInfo));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with vlanId [100],
                           non-null portsMemberPtr,
                           non-null portsTaggingPtr,
                           non-null vlanInfoPtr,
                           non-null isValidPtr,
                           non-null portsTaggingCmdPtr.
            Expected: GT_OK.
        */
        vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;

        st = cpssDxChBrgVlanEntryRead(dev, vlanId, &portsMembers, &portsTagging,
                                      &vlanInfo, &isValid, &portsTaggingCmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, vlanId);

        /*
            1.2. Call with out of range vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)]
                           and non_null other parameters.
            Expected: GT_BAD_PARAM.
        */
        vlanId = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev);

        st = cpssDxChBrgVlanEntryRead(dev, vlanId, &portsMembers, &portsTagging,
                                      &vlanInfo, &isValid, &portsTaggingCmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, vlanId);

        vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;

        /*
            1.3. Call with vlanId [100], portsMembersPtr [NULL]
                 and non_null other parameters.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgVlanEntryRead(dev, vlanId, NULL, &portsTagging,
                                      &vlanInfo, &isValid, &portsTaggingCmd);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, portsMembersPtr = NULL", dev);

        /*
            1.4. Call with vlanId [100], portsTaggingPtr [NULL]
                 and non_null other parameters.
            Expected: GT_BAD_PTR for TR101 disabled devices,
                      GT_OK for TR101 enabled devices.
        */
        st = cpssDxChBrgVlanEntryRead(dev, vlanId, &portsMembers, NULL,
                                      &vlanInfo, &isValid, &portsTaggingCmd);
        if(IS_TR101_SUPPORTED_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "%d, portsTaggingPtr = NULL", dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, portsTaggingPtr = NULL", dev);
        }

        /*
            1.4.1 Call with vlanId [100], portsTaggingCmdPtr [NULL]
                 and non_null other parameters.
            Expected: GT_BAD_PTR for TR101 enabled devices,
                      GT_OK for TR101 disabled devices.
        */
        st = cpssDxChBrgVlanEntryRead(dev, vlanId, &portsMembers, &portsTagging,
                                      &vlanInfo, &isValid, NULL);
        if(!IS_TR101_SUPPORTED_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "%d, portsTaggingPtr = NULL", dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, portsTaggingPtr = NULL", dev);
        }

        /*
            1.5. Call with vlanId [100], vlanInfoPtr [NULL]
                 and non_null other parameters
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgVlanEntryRead(dev, vlanId, &portsMembers, &portsTagging,
                                      NULL, &isValid, &portsTaggingCmd);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, vlanInfoPtr = NULL", dev);

        /*
            1.6. Call with vlanId [100], isValidPtr [NULL]
                 and non_null other parameters.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgVlanEntryRead(dev, vlanId, &portsMembers, &portsTagging,
                                      &vlanInfo, NULL, &portsTaggingCmd);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, isValidPtr = NULL", dev);
    }

    vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanEntryRead(dev, vlanId, &portsMembers, &portsTagging,
                                      &vlanInfo, &isValid, &portsTaggingCmd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanEntryRead(dev, vlanId, &portsMembers, &portsTagging,
                                  &vlanInfo, &isValid, &portsTaggingCmd);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanMemberAdd
(
    IN GT_U8   devNum,
    IN GT_U16  vlanId,
    IN GT_U8   portNum,
    IN GT_BOOL isTagged
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanMemberAdd)
{
/*
    ITERATE_DEVICES_PHY_PORT (DxChx)
    1.1.1. Call with vlanId [100],
                     isTagged [GT_FALSE and GT_ TRUE].
    Expected: GT_OK.
    1.1.2. Call with out of range vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)]
                     and isTagged [GT_TRUE].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st       = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM       port     = BRG_VLAN_VALID_PHY_PORT_CNS;
    GT_U16      vlanId   = 0;
    GT_BOOL     isTagged = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with vlanId [100],
                                 isTagged [GT_FALSE and GT_ TRUE].
                Expected: GT_OK.
            */

            /* Call function with isTagged [GT_FALSE] */
            vlanId   = BRG_VLAN_TESTED_VLAN_ID_CNS;
            isTagged = GT_FALSE;

            st = cpssDxChBrgVlanMemberAdd(dev, vlanId, port, isTagged, 0);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, vlanId, port, isTagged);

            /* Call function with isTagged [GT_TRUE] */
            vlanId   = BRG_VLAN_TESTED_VLAN_ID_CNS;
            isTagged = GT_TRUE;

            st = cpssDxChBrgVlanMemberAdd(dev, vlanId, port, isTagged, 0);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, vlanId, port, isTagged);

            /*
                1.1.2. Call with out of range vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)]
                                 and isTagged [GT_TRUE].
                Expected: GT_BAD_PARAM.
            */
            vlanId   = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev);

            st = cpssDxChBrgVlanMemberAdd(dev, vlanId, port, isTagged, 0);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "%d, vlanId = %d, port = %d",
                                         dev, vlanId, port);
        }

        vlanId   = BRG_VLAN_TESTED_VLAN_ID_CNS;
        isTagged = GT_FALSE;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanMemberAdd(dev, vlanId, port, isTagged, 0);
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            {
                if(port >= UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev))
                {
                    UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                }
            }
            else
            if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(dev) ||
               port >= PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
            }
            else
            {
                /* the device supports ports up to PRV_CPSS_DXCH_MAX_PHY_PORT_MAC(dev) */
                /* regardless to physical ports */
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChBrgVlanMemberAdd(dev, vlanId, port, isTagged, 0);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK  */
        /* for CPU port number.                                      */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanMemberAdd(dev, vlanId, port, isTagged, 0);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    vlanId   = BRG_VLAN_TESTED_VLAN_ID_CNS;
    isTagged = GT_FALSE;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = BRG_VLAN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanMemberAdd(dev, vlanId, port, isTagged, 0);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanMemberAdd(dev, vlanId, port, isTagged, 0);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortDelete
(
    IN GT_U8          devNum,
    IN GT_U16         vlanId,
    IN GT_U8          port
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortDelete)
{
/*
    ITERATE_DEVICES_PHY_PORT (DxChx)
    1.1.1. Call with vlanId [100].
    Expected: GT_OK.
    1.1.2. Call with out of range vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM       port   = BRG_VLAN_VALID_PHY_PORT_CNS;
    GT_U16      vlanId = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with vlanId [100].
                Expected: GT_OK.
            */
            vlanId   = BRG_VLAN_TESTED_VLAN_ID_CNS;

            st = cpssDxChBrgVlanPortDelete(dev, vlanId, port);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlanId, port);

            /*
                1.1.2. Call with out of range vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)].
                Expected: GT_BAD_PARAM.
            */
            vlanId   = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev);

            st = cpssDxChBrgVlanPortDelete(dev, vlanId, port);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, vlanId, port);
        }

        vlanId   = BRG_VLAN_TESTED_VLAN_ID_CNS;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanPortDelete(dev, vlanId, port);
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            {
                if(port >= UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev))
                {
                    UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                }
            }
            else
            if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(dev) ||
               port >= PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
            }
            else
            {
                /* the device supports ports up to PRV_CPSS_DXCH_MAX_PHY_PORT_MAC(dev) */
                /* regardless to physical ports */
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChBrgVlanPortDelete(dev, vlanId, port);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                     */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanPortDelete(dev, vlanId, port);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    vlanId   = BRG_VLAN_TESTED_VLAN_ID_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = BRG_VLAN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortDelete(dev, vlanId, port);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortDelete(dev, vlanId, port);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortVidGet
(
    IN  GT_U8   devNum,
    IN  GT_U8   port,
    OUT GT_U16  *vidPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortVidGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxChx)
    1.1.1. Call with non-null vidPtr.
    Expected: GT_OK.
    1.1.2. Call with vidPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;

    GT_U8       dev;
    GT_PORT_NUM       port = BRG_VLAN_VALID_PHY_PORT_CNS;
    GT_U16      vid  = 0;
    CPSS_DIRECTION_ENT          direction;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(direction = CPSS_DIRECTION_INGRESS_E ; direction <= CPSS_DIRECTION_EGRESS_E; direction++)
        {
            prvUtfNextGenericPortItaratorTypeSet(   direction == CPSS_DIRECTION_INGRESS_E ?
                                                    UTF_GENERIC_PORT_ITERATOR_TYPE_DEFAULT_EPORT_E :
                                                    UTF_GENERIC_PORT_ITERATOR_TYPE_VIRTUAL_E);

            st = prvUtfNextGenericPortReset(&port, dev);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* 1.1. For all active devices go over all available physical ports. */
            while(GT_OK == prvUtfNextGenericPortGet(&port, GT_TRUE))
            {
                /*
                    1.1.1. Call with non-null vidPtr.
                    Expected: GT_OK.
                */
                st = cpssDxChBrgVlanPortVidGet(dev, port, direction,&vid);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                /*
                    1.1.2. Call with vidPtr [NULL].
                    Expected: GT_BAD_PTR.
                */
                st = cpssDxChBrgVlanPortVidGet(dev, port, direction,NULL);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, vidPtr = NULL", dev, port);
            }

            vid = BRG_VLAN_TESTED_VLAN_ID_CNS;

            st = prvUtfNextGenericPortReset(&port, dev);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* 1.2. For all active devices go over all non available physical ports. */
            while(GT_OK == prvUtfNextGenericPortGet(&port, GT_FALSE))
            {
                /* 1.2.1. Call function for each non-active port */
                st = cpssDxChBrgVlanPortVidGet(dev, port, direction,&vid);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
            }

            /* 1.3. For active device check that function returns GT_BAD_PARAM */
            /* for out of bound value for port number.                         */

            port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

            st = cpssDxChBrgVlanPortVidGet(dev, port, direction,&vid);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

            /* 1.4. For active device check that function returns GT_BAD_PARAM */
            /* for CPU port number.                                            */
            port = CPSS_CPU_PORT_NUM_CNS;

            st = cpssDxChBrgVlanPortVidGet(dev, port, direction,&vid);
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            else
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }

    direction = CPSS_DIRECTION_INGRESS_E;

    vid = BRG_VLAN_TESTED_VLAN_ID_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = BRG_VLAN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortVidGet(dev, port, direction,&vid);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortVidGet(dev, port, direction,&vid);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortVidSet
(
    IN  GT_U8   devNum,
    IN  GT_U8   port,
    IN  GT_U16  vlanId
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortVidSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxChx)
    1.1.1. Call with vlanId [100].
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgVlanPortVidGet.
    Expected: GT_OK and the same vlanId.
    1.1.3. Call with out of range vlanId [UTF_CPSS_PP_MAX_VLAN_NUM_CNS(dev)].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st        = GT_OK;

    GT_U8       dev;
    GT_PORT_NUM       port      = BRG_VLAN_VALID_PHY_PORT_CNS;
    GT_U16      vlanId    = 0;
    GT_U16      vlanIdGet = 0;
    CPSS_DIRECTION_ENT          direction;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(direction = CPSS_DIRECTION_INGRESS_E ; direction <= CPSS_DIRECTION_EGRESS_E; direction++)
        {
            prvUtfNextGenericPortItaratorTypeSet(   direction == CPSS_DIRECTION_INGRESS_E ?
                                                    UTF_GENERIC_PORT_ITERATOR_TYPE_DEFAULT_EPORT_E :
                                                    UTF_GENERIC_PORT_ITERATOR_TYPE_VIRTUAL_E);

            st = prvUtfNextGenericPortReset(&port, dev);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* 1.1. For all active devices go over all available physical ports. */
            while(GT_OK == prvUtfNextGenericPortGet(&port, GT_TRUE))
            {
                /*
                    1.1.1. Call with vlanId [100].
                    Expected: GT_OK.
                */
                vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;

                st = cpssDxChBrgVlanPortVidSet(dev, port, direction,vlanId);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, vlanId);

                /*
                    1.1.2. Call cpssDxChBrgVlanPortVidGet.
                    Expected: GT_OK and the same vlanId.
                */
                st = cpssDxChBrgVlanPortVidGet(dev, port, direction,&vlanIdGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "cpssDxChBrgVlanPortVidGet: %d, %d", dev, port);

                UTF_VERIFY_EQUAL2_STRING_MAC(vlanId, vlanIdGet,
                                         "get another vlanId than was set: %d, %d", dev, port);

                /*
                    1.1.3. Call with out of range vlanId [UTF_CPSS_PP_MAX_VLAN_NUM_CNS(dev)].
                    Expected: GT_BAD_PARAM.
                */
                vlanId   = UTF_CPSS_PP_MAX_VLAN_NUM_CNS(dev);

                st = cpssDxChBrgVlanPortVidSet(dev, port, direction,vlanId);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, vlanId);
            }

            vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;

            st = prvUtfNextGenericPortReset(&port, dev);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* 1.2. For all active devices go over all non available physical ports. */
            while(GT_OK == prvUtfNextGenericPortGet(&port, GT_FALSE))
            {
                /* 1.2.1. Call function for each non-active port */
                st = cpssDxChBrgVlanPortVidSet(dev, port, direction,vlanId);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
            }

            /* 1.3. For active device check that function returns GT_BAD_PARAM */
            /* for out of bound value for port number.                         */

            port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

            st = cpssDxChBrgVlanPortVidSet(dev, port, direction,vlanId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

            /* 1.4. For active device check that function returns GT_BAD_PARAM */
            /* for CPU port number.                                            */
            port = CPSS_CPU_PORT_NUM_CNS;

            st = cpssDxChBrgVlanPortVidSet(dev, port, direction,vlanId);
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            else
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }

    direction = CPSS_DIRECTION_INGRESS_E;

    vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = BRG_VLAN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortVidSet(dev, port, direction,vlanId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortVidSet(dev, port, direction,vlanId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortIngFltEnable
(
    IN GT_U8   devNum,
    IN GT_U8   port,
    IN GT_BOOL enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortIngFltEnable)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxChx)
    1.1.1. Call with enable [GT_TRUE and GT_FALSE].
    Expected: GT_OK.
*/
    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_PORT_NUM       port  = BRG_VLAN_VALID_PHY_PORT_CNS;
    GT_BOOL     state = GT_TRUE;
    GT_BOOL     stateGet = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with enable [GT_TRUE and GT_FALSE].
                Expected: GT_OK.
            */

            /* Call function with enable [GT_FALSE] */
            state = GT_FALSE;

            st = cpssDxChBrgVlanPortIngFltEnable(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*
                1.1.2. Call get func.
                Expected: GT_OK and the same state.
            */

            st = cpssDxChBrgVlanPortIngFltEnableGet(dev, port, &stateGet);

            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);
            UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                               "get another state than was set: %d", dev);

            /* Call function with enable [GT_TRUE] */
            state = GT_TRUE;

            st = cpssDxChBrgVlanPortIngFltEnable(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);
            /*
                1.1.2. Call get func.
                Expected: GT_OK and the same state.
            */

            st = cpssDxChBrgVlanPortIngFltEnableGet(dev, port, &stateGet);

            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);
            UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                               "get another state than was set: %d", dev);

        }

        state = GT_TRUE;

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanPortIngFltEnable(dev, port, state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanPortIngFltEnable(dev, port, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanPortIngFltEnable(dev, port, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    state = GT_TRUE;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = BRG_VLAN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortIngFltEnable(dev, port, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortIngFltEnable(dev, port, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortIngFltEnableGet
(
    IN GT_U8   devNum,
    IN GT_U8   port,
    OUT GT_BOOL *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortIngFltEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxChx)
    1.1.1. Call with not null enable.
    Expected: GT_OK.
    1.1.2. Call with wrong enable [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_PORT_NUM       port  = BRG_VLAN_VALID_PHY_PORT_CNS;
    GT_BOOL     state = GT_TRUE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with not null enable.
                Expected: GT_OK.
            */

            st = cpssDxChBrgVlanPortIngFltEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*
                1.1.2. Call with wrong enable [NULL].
                Expected: GT_BAD_PTR.
            */

            st = cpssDxChBrgVlanPortIngFltEnableGet(dev, port, NULL);

            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanPortIngFltEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanPortIngFltEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanPortIngFltEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = BRG_VLAN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortIngFltEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortIngFltEnableGet(dev, port, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortProtoClassVlanEnable
(
    IN GT_U8   devNum,
    IN GT_U8   port,
    IN GT_BOOL enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortProtoClassVlanEnable)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxChx)
    1.1.1. Call with enable [GT_TRUE and GT_FALSE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgVlanPortProtoClassVlanEnableGet.
    Expected: GT_OK and same enable.
*/
    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_PORT_NUM       port  = BRG_VLAN_VALID_PHY_PORT_CNS;
    GT_BOOL     state = GT_TRUE;
    GT_BOOL     stateGet = GT_TRUE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextDefaultEPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextDefaultEPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with enable [GT_TRUE and GT_FALSE].
                Expected: GT_OK.
            */

            /* Call function with enable [GT_FALSE] */
            state = GT_FALSE;

            st = cpssDxChBrgVlanPortProtoClassVlanEnable(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*  1.1.2.  */
            st = cpssDxChBrgVlanPortProtoClassVlanEnableGet(dev, port, &stateGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                "get another state than was set: %d", dev);

            /* Call function with enable [GT_TRUE] */
            state = GT_TRUE;

            st = cpssDxChBrgVlanPortProtoClassVlanEnable(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*  1.1.2.  */
            st = cpssDxChBrgVlanPortProtoClassVlanEnableGet(dev, port, &stateGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                "get another state than was set: %d", dev);
        }

        state = GT_TRUE;

        st = prvUtfNextDefaultEPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextDefaultEPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanPortProtoClassVlanEnable(dev, port, state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanPortProtoClassVlanEnable(dev, port, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanPortProtoClassVlanEnable(dev, port, state);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        else
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    state = GT_TRUE;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = BRG_VLAN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortProtoClassVlanEnable(dev, port, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortProtoClassVlanEnable(dev, port, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortProtoClassVlanEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxChx)
    1.1.1. Call with not NULL enablePtr.
    Expected: GT_OK.
    1.1.2. Call with NULL enablePtr.
    Expected: GT_BAD_PTR.

*/
    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_PORT_NUM       port  = BRG_VLAN_VALID_PHY_PORT_CNS;
    GT_BOOL     state = GT_TRUE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextDefaultEPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextDefaultEPortGet(&port, GT_TRUE))
        {
            /*  1.1.1.  */
            st = cpssDxChBrgVlanPortProtoClassVlanEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*  1.1.2.  */
            st = cpssDxChBrgVlanPortProtoClassVlanEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
        }

        st = prvUtfNextDefaultEPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextDefaultEPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanPortProtoClassVlanEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanPortProtoClassVlanEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanPortProtoClassVlanEnableGet(dev, port, &state);
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        else
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = BRG_VLAN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortProtoClassVlanEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortProtoClassVlanEnableGet(dev, port, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortProtoClassQosEnable
(
    IN GT_U8   devNum,
    IN GT_U8   port,
    IN GT_BOOL enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortProtoClassQosEnable)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxChx)
    1.1.1. Call with enable [GT_TRUE and GT_FALSE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgVlanPortProtoClassQosEnableGet.
    Expected: GT_OK and same enablePtr.
*/
    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_PORT_NUM       port  = BRG_VLAN_VALID_PHY_PORT_CNS;
    GT_BOOL     state = GT_TRUE;
    GT_BOOL     stateGet = GT_TRUE;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextDefaultEPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextDefaultEPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with enable [GT_TRUE and GT_FALSE].
                Expected: GT_OK.
            */

            /* Call function with enable [GT_FALSE] */
            state = GT_FALSE;

            st = cpssDxChBrgVlanPortProtoClassQosEnable(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*  1.1.2.  */
            st = cpssDxChBrgVlanPortProtoClassQosEnableGet(dev, port, &stateGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                "get another state than was set: %d", dev);

            /* Call function with enable [GT_TRUE] */
            state = GT_TRUE;

            st = cpssDxChBrgVlanPortProtoClassQosEnable(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*  1.1.2.  */
            st = cpssDxChBrgVlanPortProtoClassQosEnableGet(dev, port, &stateGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                "get another state than was set: %d", dev);
        }

        state = GT_TRUE;

        st = prvUtfNextDefaultEPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextDefaultEPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanPortProtoClassQosEnable(dev, port, state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanPortProtoClassQosEnable(dev, port, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanPortProtoClassQosEnable(dev, port, state);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        else
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    state = GT_TRUE;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = BRG_VLAN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortProtoClassQosEnable(dev, port, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortProtoClassQosEnable(dev, port, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortProtoClassQosEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxChx)
    1.1.1. Call with not NULL enablePtr.
    Expected: GT_OK.
    1.1.2. Call with NULL enablePtr.
    Expected: GT_BAD_PTR.

*/
    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_PORT_NUM       port  = BRG_VLAN_VALID_PHY_PORT_CNS;
    GT_BOOL     state = GT_TRUE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextDefaultEPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextDefaultEPortGet(&port, GT_TRUE))
        {
            /*  1.1.1.  */
            st = cpssDxChBrgVlanPortProtoClassQosEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*  1.1.2.  */
            st = cpssDxChBrgVlanPortProtoClassQosEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
        }

        st = prvUtfNextDefaultEPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextDefaultEPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanPortProtoClassQosEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanPortProtoClassQosEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanPortProtoClassQosEnableGet(dev, port, &state);
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        else
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = BRG_VLAN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortProtoClassQosEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortProtoClassQosEnableGet(dev, port, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanProtoClassSet
(
    IN GT_U8                            devNum,
    IN GT_U32                           entryNum,
    IN GT_U16                           etherType,
    IN CPSS_PROT_CLASS_ENCAP_STC        *encListPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanProtoClassSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with entryNum [0],
                   etherType [1],
                   and encListPtr {GT_TRUE, GT_FALSE, GT_FALSE}.
    Expected: GT_OK.
    1.2. Call cpssDxChBrgVlanProtoClassGet.
    Expected: GT_OK and the same etherType and encList.
    1.3. Call with out of range entryNum [8].
    Expected: NON GT_OK.
    1.4. Call with entryNum [1],
                   etherType [0xFFFF] (no any constraints),
                   and encListPtr {GT_FALSE, GT_TRUE, GT_FALSE}.
    Expected: GT_OK.
    1.5. Call with entryNum [2],
                   etherType [5]
                   and encListPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                 st           = GT_OK;

    GT_U8                     dev;
    GT_U32                    entryNum     = 0;
    GT_U16                    etherType    = 0;
    GT_U16                    etherTypeGet = 0;
    CPSS_PROT_CLASS_ENCAP_STC encList;
    CPSS_PROT_CLASS_ENCAP_STC encListGet;
    GT_BOOL                   isValidEntry = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with entryNum [0],
                           etherType [1],
                           and encListPtr {GT_TRUE, GT_FALSE, GT_FALSE}.
            Expected: GT_OK.
        */
        entryNum  = 0;
        etherType = 1;

        encList.ethV2      = GT_TRUE;
        encList.nonLlcSnap = GT_FALSE;
        encList.llcSnap    = GT_FALSE;

        st = cpssDxChBrgVlanProtoClassSet(dev, entryNum, etherType, &encList);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, entryNum, etherType);

        /*
            1.2. Call cpssDxChBrgVlanProtoClassGet.
            Expected: GT_OK and the same etherType and encList.
        */
        st = cpssDxChBrgVlanProtoClassGet(dev, entryNum, &etherTypeGet,
                                          &encListGet, &isValidEntry);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                     "cpssDxChBrgVlanProtoClassGet: %d, %d", dev, entryNum);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isValidEntry,
                                     "Entry is not valid: %d", dev);

        if (GT_TRUE == isValidEntry)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(etherType, etherTypeGet,
                                         "get another etherType than was set: %d", dev);

            /* Verifying struct fields */
            UTF_VERIFY_EQUAL1_STRING_MAC(encList.ethV2, encListGet.ethV2,
                                         "get another encListPtr->ethV2 than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(encList.nonLlcSnap, encListGet.nonLlcSnap,
                                         "get another encListPtr->nonLlcSnap than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(encList.llcSnap, encListGet.llcSnap,
                                         "get another encListPtr->llcSnap than was set: %d", dev);
        }

        /*
            1.3. Call with out of range entryNum [8 or 12].
            Expected: NON GT_OK.
        */
        if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(dev))
        {
            entryNum = 12;
        }
        else
            entryNum = 8;

        st = cpssDxChBrgVlanProtoClassSet(dev, entryNum, etherType, &encList);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, entryNum);

        /*
            1.4. Call with entryNum [1],
                           etherType [0xFFFF] (no any constraints),
                           and encListPtr {GT_FALSE, GT_TRUE, GT_FALSE}.
            Expected: GT_OK.
        */
        entryNum  = 1;
        etherType = 0xFFFF;

        encList.ethV2      = GT_FALSE;
        encList.nonLlcSnap = GT_TRUE;
        encList.llcSnap    = GT_FALSE;

        st = cpssDxChBrgVlanProtoClassSet(dev, entryNum, etherType, &encList);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, entryNum, etherType);

        /*
            1.5. Call with entryNum [2],
                           etherType [5]
                           and encListPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        entryNum  = 2;
        etherType = 5;

        st = cpssDxChBrgVlanProtoClassSet(dev, entryNum, etherType, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, encListPtr = NULL", dev);
    }

    entryNum  = 0;
    etherType = 1;

    encList.ethV2      = GT_TRUE;
    encList.nonLlcSnap = GT_FALSE;
    encList.llcSnap    = GT_FALSE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanProtoClassSet(dev, entryNum, etherType, &encList);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanProtoClassSet(dev, entryNum, etherType, &encList);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanProtoClassGet
(
    IN GT_U8                            devNum,
    IN GT_U32                           entryNum,
    OUT GT_U16                          *etherTypePtr,
    OUT CPSS_PROT_CLASS_ENCAP_STC       *encListPtr,
    OUT GT_BOOL                         *validEntryPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanProtoClassGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with entryNum [0],
                   non-null etherTypePtr,
                   non-null encListPtr
                   and non-null validEntryPtr.
    Expected: GT_OK.
    1.2. Call with entryNum [8] and other non-null parameters.
    Expected: non GT_OK.
    1.3. Call with entryNum [1],
                   null etherTypePtr [NULL],
                   non-null encListPtr
                   and non-null validEntryPtr.
    Expected: GT_BAD_PTR.
    1.4. Call with entryNum [2],
                   non-null etherTypePtr,
                   null encListPtr [NULL]
                   and non-null validEntryPtr.
    Expected: GT_BAD_PTR.
    1.5. Call with entryNum [3],
                   non-null etherTypePtr,
                   non-null encListPtr
                   and null validEntryPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                 st           = GT_OK;

    GT_U8                     dev;
    GT_U32                    entryNum     = 0;
    GT_U16                    etherTypePtr = 0;
    CPSS_PROT_CLASS_ENCAP_STC encListPtr;
    GT_BOOL                   validEntry   = GT_FALSE;


    cpssOsBzero((GT_VOID*) &encListPtr, sizeof(encListPtr));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with entryNum [0],
                           non-null etherTypePtr,
                           non-null encListPtr
                           and non-null validEntryPtr.
            Expected: GT_OK.
        */
        entryNum = 0;

        st = cpssDxChBrgVlanProtoClassGet(dev, entryNum, &etherTypePtr,
                                          &encListPtr, &validEntry);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, entryNum);

        /*  1.2. Call with entryNum [8 or 12] and other non-null parameters.
            Expected: non GT_OK. */

        if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(dev))
        {
            entryNum = 12;
        }
        else
            entryNum = 8;

        st = cpssDxChBrgVlanProtoClassGet(dev, entryNum, &etherTypePtr,
                                          &encListPtr, &validEntry);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, entryNum);

        /*
            1.3. Call with entryNum [1],
                           null etherTypePtr [NULL],
                           non-null encListPtr
                           and non-null validEntryPtr.
            Expected: GT_BAD_PTR.
        */
        entryNum = 1;

        st = cpssDxChBrgVlanProtoClassGet(dev, entryNum, NULL,
                                          &encListPtr, &validEntry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, etherTypePtr = NULL", dev);

        /*
            1.4. Call with entryNum [2],
                           non-null etherTypePtr,
                           null encListPtr [NULL]
                           and non-null validEntryPtr.
            Expected: GT_BAD_PTR.
        */
        entryNum = 2;

        st = cpssDxChBrgVlanProtoClassGet(dev, entryNum, &etherTypePtr,
                                          NULL, &validEntry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, encListPtr = NULL", dev);

        /*
            1.5. Call with entryNum [3],
                           non-null etherTypePtr,
                           non-null encListPtr
                           and null validEntryPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        entryNum = 3;

        st = cpssDxChBrgVlanProtoClassGet(dev, entryNum, &etherTypePtr,
                                          &encListPtr, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, validEntryPtr = NULL", dev);
    }

    entryNum = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanProtoClassGet(dev, entryNum, &etherTypePtr,
                                          &encListPtr, &validEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanProtoClassGet(dev, entryNum, &etherTypePtr,
                                          &encListPtr, &validEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanProtoClassInvalidate
(
    IN GT_U8                            devNum,
    IN GT_U32                           entryNum
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanProtoClassInvalidate)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with entryNum [0 / 7].
    Expected: GT_OK.
    1.2. Call with [8]. Expected: non GT_OK.
*/
    GT_STATUS st       = GT_OK;

    GT_U8     dev;
    GT_U32    entryNum = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with entryNum [0 / 7].
            Expected: GT_OK.
        */
        entryNum = 0;

        st = cpssDxChBrgVlanProtoClassInvalidate(dev, entryNum);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, entryNum);

        entryNum = 7;

        st = cpssDxChBrgVlanProtoClassInvalidate(dev, entryNum);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, entryNum);

        /*
            1.2. Call with entryNum [8 or 12].
            Expected: non GT_OK.
        */
        if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(dev))
        {
            entryNum = 12;
        }
        else
            entryNum = 8;

        st = cpssDxChBrgVlanProtoClassInvalidate(dev, entryNum);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, entryNum);
    }

    entryNum = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanProtoClassInvalidate(dev, entryNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanProtoClassInvalidate(dev, entryNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortProtoVlanQosSet
(
    IN GT_U8                                    devNum,
    IN GT_U8                                    port,
    IN GT_U32                                   entryNum,
    IN CPSS_DXCH_BRG_VLAN_PROT_CLASS_CFG_STC    *vlanCfgPtr,
    IN CPSS_DXCH_BRG_QOS_PROT_CLASS_CFG_STC     *qosCfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortProtoVlanQosSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxChx)
    1.1.1. Call with entryNum [0],
                     vlanCfgPtr {vlanId [100],
                                 vlanIdAssignCmd  [CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E /
                                                   CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_TAGGED_E],
                                 vlanAssignPrecedence [CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E /
                                                       CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E]},
                     qosCfgPtr [{qosAssignCmd [CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_UNTAGGED_E /
                                               CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E],
                                 qosProfileId [10],
                                 qosAssignPrecedence [CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E/
                                                      CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E],
                                 enableModifyUp [GT_TRUE/ GT_FALSE],
                                 enableModifyDscp [GT_TRUE/ GT_FALSE]}].
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgVlanPortProtoVlanQosGet with the same entryNum.
    Expected: GT_OK and the same vlanCfgPtr, qosCfgPtr and validEntryPtr [GT_TRUE].
    1.1.3. Call with entryNum [1],
                     vlanCfgPtr {out of range vlanId [UTF_CPSS_PP_MAX_VLAN_NUM_CNS(dev)],
                                 vlanIdAssignCmd [CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E],
                                 vlanAssignPrecedence [CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E]},
                     qosCfgPtr [NULL]
    Expected: GT_BAD_PARAM.
    1.1.4. Call with entryNum [2],
                     vlanCfgPtr {vlanId [100],
                                 out of range vlanIdAssignCmd,
                                 vlanAssignPrecedence [CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E}],
                     qosCfgPtr [NULL].
    Expected: GT_BAD_PARAM.
    1.1.5. Call with entryNum [3],
                     vlanCfgPtr {vlanId [100],
                                 vlanIdAssignCmd  [CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E]
                                 and out of range vlanAssignPrecedence}],
                     qosCfgPtr [NULL].
    Expected: GT_BAD_PARAM.
    1.1.6. Call with out of range entryNum [8 ] and other valid parameters from 1.1.1.
    Expected: non GT_OK.
    1.1.7. Call with entryNum [4],
                     vlanCfgPtr[NULL]
                     and other valid parameters from 1.1.1.
    Expected: GT_OK.
    1.1.8. Call with entryNum [5],
                     qosCfgPtr [NULL]
                     and other valid parameters from 1.1.1.
    Expected: GT_OK.
    1.1.9. Call with entryNum [6],
                     vlanCfgPtr[NULL],
                     qosCfgPtr [NULL].
    Expected: NON GT_ OK.
    1.1.10. Call with entryNum [1],
                      wrong enum values qosCfgPtr ->qosAssignCmd ,
                      valid other fields of qosCfg
                      and vlanCfgPtr [NULL]
    Expected: GT_BAD_PARAM.
    1.1.11. Call with entryNum [1],
                      wrong enum values qosCfgPtr ->qosAssignPrecedence ,
                      valid other fields of qosCfg
                      and vlanCfgPtr [NULL]
    Expected: GT_BAD_PARAM.
    1.1.12. Call with entryNum [1],
                      qosCfgPtr ->qosProfileId [PRV_CPSS_DXCH_QOS_PROFILE_NUM_MAX_CNS],
                      valid other fields of qosCfg
                      and vlanCfgPtr [NULL]
    Expected: NON GT_OK for Cheetah, GT_OK for Cheetah2.
    1.1.13. Call with entryNum [1],
                      qosCfgPtr ->qosProfileId [PRV_CPSS_DXCH2_QOS_PROFILE_NUM_MAX_CNS],
                      valid other fields of qosCfg
                      and vlanCfgPtr [NULL]
    Expected: NON GT_OK.
*/
    GT_STATUS  st    = GT_OK;


    GT_U8                                 dev;
    GT_PORT_NUM                           port     = BRG_VLAN_VALID_PHY_PORT_CNS;
    GT_PORT_NUM                           tmpPort;
    GT_U32                                entryNum = 0;
    CPSS_DXCH_BRG_VLAN_PROT_CLASS_CFG_STC vlanCfg;
    CPSS_DXCH_BRG_QOS_PROT_CLASS_CFG_STC  qosCfg;
    CPSS_DXCH_BRG_VLAN_PROT_CLASS_CFG_STC vlanCfgGet;
    CPSS_DXCH_BRG_QOS_PROT_CLASS_CFG_STC  qosCfgGet;

    GT_BOOL                               isValid   = GT_FALSE;
    CPSS_PP_FAMILY_TYPE_ENT               devFamily = CPSS_PP_FAMILY_CHEETAH_E;


    cpssOsBzero((GT_VOID*) &vlanCfg, sizeof(vlanCfg));
    cpssOsBzero((GT_VOID*) &qosCfg, sizeof(qosCfg));
    cpssOsBzero((GT_VOID*) &vlanCfgGet, sizeof(vlanCfgGet));
    cpssOsBzero((GT_VOID*) &qosCfgGet, sizeof(qosCfgGet));

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        tmpPort = port;
        st = prvUtfNextDefaultEPortReset(&tmpPort, dev);

        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextDefaultEPortGet(&tmpPort, GT_TRUE))
        {
            /*
                1.1.1. Call with entryNum [0],
                     vlanCfgPtr {vlanId [100],
                                 vlanIdAssignCmd  [CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E /
                                                   CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_TAGGED_E],
                                 vlanAssignPrecedence [CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E /
                                                       CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E]},
                     qosCfgPtr [{qosAssignCmd [CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_UNTAGGED_E /
                                               CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E],
                                 qosProfileId [10],
                                 qosAssignPrecedence [CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E/
                                                      CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E],
                                 enableModifyUp [GT_TRUE/ GT_FALSE],
                                 enableModifyDscp [GT_TRUE/ GT_FALSE]}].
                Expected: GT_OK.
            */

            /* Call function with 1 entries */
            port = tmpPort;
            entryNum = 0;

            vlanCfg.vlanId                 = BRG_VLAN_TESTED_VLAN_ID_CNS;
            vlanCfg.vlanIdAssignCmd        = CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E;
            vlanCfg.vlanIdAssignPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;

            qosCfg.qosAssignCmd        = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_UNTAGGED_E;
            qosCfg.qosProfileId        = 10;
            qosCfg.qosAssignPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
            qosCfg.enableModifyUp      = GT_TRUE;
            qosCfg.enableModifyDscp    = GT_TRUE;

            st = cpssDxChBrgVlanPortProtoVlanQosSet(dev, port, entryNum, &vlanCfg, &qosCfg);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, entryNum);

            /*
                1.1.2. Call cpssDxChBrgVlanPortProtoVlanQosGet with the same entryNum.
                Expected: GT_OK and the same vlanCfgPtr, qosCfgPtr and validEntryPtr [GT_TRUE].
            */
            st = cpssDxChBrgVlanPortProtoVlanQosGet(dev, port, entryNum,
                                                    &vlanCfgGet, &qosCfgGet, &isValid);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgVlanPortProtoVlanQosGet: %d, %d, %d", dev, port, entryNum);

            UTF_VERIFY_EQUAL3_STRING_MAC(GT_TRUE, isValid, "Entry is NOT valid: %d, %d, %d", dev, port, entryNum);

            if (GT_TRUE == isValid)
            {
                /* Verifying vlanCfgPtr */
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanCfg.vlanId, vlanCfgGet.vlanId,
                           "get another vlanCfgPtr->vlanId than was set: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanCfg.vlanIdAssignCmd, vlanCfgGet.vlanIdAssignCmd,
                           "get another vlanCfgPtr->vlanIdAssignCmd than was set: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanCfg.vlanIdAssignPrecedence, vlanCfgGet.vlanIdAssignPrecedence,
                           "get another vlanCfgPtr->vlanIdAssignPrecedence than was set: %d, %d", dev, port);

                /* Verifying qosCfgPtr */
                UTF_VERIFY_EQUAL2_STRING_MAC(qosCfg.qosAssignCmd, qosCfgGet.qosAssignCmd,
                           "get another qosCfgPtr->qosAssignCmd than was set: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(qosCfg.qosProfileId, qosCfgGet.qosProfileId,
                           "get another qosCfgPtr->qosProfileId than was set: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(qosCfg.qosAssignPrecedence, qosCfgGet.qosAssignPrecedence,
                           "get another qosCfgPtr->qosAssignPrecedence than was set: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(qosCfg.enableModifyUp, qosCfgGet.enableModifyUp,
                           "get another qosCfgPtr->enableModifyUp than was set: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(qosCfg.enableModifyDscp, qosCfgGet.enableModifyDscp,
                           "get another qosCfgPtr->enableModifyDscp than was set: %d, %d", dev, port);
            }

            /* Call function with 2 entries */
            vlanCfg.vlanIdAssignCmd        = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_TAGGED_E;
            vlanCfg.vlanIdAssignPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;

            qosCfg.qosAssignCmd        = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
            qosCfg.qosAssignPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
            qosCfg.enableModifyUp      = GT_FALSE;
            qosCfg.enableModifyDscp    = GT_FALSE;

            st = cpssDxChBrgVlanPortProtoVlanQosSet(dev, port, entryNum,
                                                    &vlanCfg, &qosCfg);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, entryNum);

            /*
                1.1.2. Call cpssDxChBrgVlanPortProtoVlanQosGet with the same entryNum.
                Expected: GT_OK and the same vlanCfgPtr, qosCfgPtr and validEntryPtr [GT_TRUE].
            */
            st = cpssDxChBrgVlanPortProtoVlanQosGet(dev, port, entryNum,
                                                    &vlanCfgGet, &qosCfgGet, &isValid);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgVlanPortProtoVlanQosGet: %d, %d, %d", dev, port, entryNum);

            UTF_VERIFY_EQUAL3_STRING_MAC(GT_TRUE, isValid, "Entry is NOT valid: %d, %d, %d", dev, port, entryNum);

            if (GT_TRUE == isValid)
            {
                /* Verifying vlanCfgPtr */
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanCfg.vlanId, vlanCfgGet.vlanId,
                           "get another vlanCfgPtr->vlanId than was set: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanCfg.vlanIdAssignCmd, vlanCfgGet.vlanIdAssignCmd,
                           "get another vlanCfgPtr->vlanIdAssignCmd than was set: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanCfg.vlanIdAssignPrecedence, vlanCfgGet.vlanIdAssignPrecedence,
                           "get another vlanCfgPtr->vlanIdAssignPrecedence than was set: %d, %d", dev, port);

                /* Verifying qosCfgPtr */
                UTF_VERIFY_EQUAL2_STRING_MAC(qosCfg.qosAssignCmd, qosCfgGet.qosAssignCmd,
                           "get another qosCfgPtr->qosAssignCmd than was set: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(qosCfg.qosProfileId, qosCfgGet.qosProfileId,
                           "get another qosCfgPtr->qosProfileId than was set: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(qosCfg.qosAssignPrecedence, qosCfgGet.qosAssignPrecedence,
                           "get another qosCfgPtr->qosAssignPrecedence than was set: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(qosCfg.enableModifyUp, qosCfgGet.enableModifyUp,
                           "get another qosCfgPtr->enableModifyUp than was set: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(qosCfg.enableModifyDscp, qosCfgGet.enableModifyDscp,
                           "get another qosCfgPtr->enableModifyDscp than was set: %d, %d", dev, port);
            }

            /*
                1.1.3. Call with entryNum [1],
                                 vlanCfgPtr {out of range vlanId [UTF_CPSS_PP_MAX_VLAN_NUM_CNS(dev)],
                                             vlanIdAssignCmd [CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E],
                                             vlanAssignPrecedence [CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E]},
                                 qosCfgPtr [NULL]
                Expected: GT_BAD_PARAM.
            */
            entryNum = 1;

            vlanCfg.vlanId                 = UTF_CPSS_PP_MAX_VLAN_NUM_CNS(dev);
            vlanCfg.vlanIdAssignCmd        = CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E;
            vlanCfg.vlanIdAssignPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;

            st = cpssDxChBrgVlanPortProtoVlanQosSet(dev, port, entryNum,
                                                    &vlanCfg, NULL);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "%d, %d, vlanCfgPtr->vlanId = %d",
                                         dev, port, vlanCfg.vlanId);

            /*
                1.1.4. Call with entryNum [2], vlanCfgPtr {vlanId [100],
                        wrong enum values vlanIdAssignCmd,
                        vlanAssignPrecedence [CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E}],
                        qosCfgPtr [NULL].
                Expected: GT_BAD_PARAM.
            */
            entryNum = 2;
            vlanCfg.vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;

            UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanPortProtoVlanQosSet
                                                        (dev, port, entryNum, &vlanCfg, NULL),
                                                        vlanCfg.vlanIdAssignCmd);

            vlanCfg.vlanIdAssignCmd = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;

            /*
                1.1.5. Call with entryNum [3],
                             vlanCfgPtr {vlanId [100],
                                         vlanIdAssignCmd  [CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E]
                                         and wrong enum values vlanAssignPrecedence}],
                             qosCfgPtr [NULL].
                Expected: GT_BAD_PARAM.
            */
            entryNum = 3;
            vlanCfg.vlanIdAssignCmd = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;

            UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanPortProtoVlanQosSet
                                                        (dev, port, entryNum, &vlanCfg, NULL),
                                                        vlanCfg.vlanIdAssignPrecedence);

            vlanCfg.vlanIdAssignPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;

            /*
                1.1.6. Call with out of range entryNum [8 or 12] and other valid
                       parameters from 1.1.1.
                Expected: non GT_OK.
            */
            if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(dev))
            {
                entryNum = 12;
            }
            else
                entryNum = 8;

            st = cpssDxChBrgVlanPortProtoVlanQosSet(dev, port, entryNum, &vlanCfg, &qosCfg);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, entryNum);

            /*
                1.1.7. Call with entryNum [4], vlanCfgPtr[NULL]
                                 and other valid parameters from 1.1.1.
                Expected: GT_OK.
            */
            entryNum = 4;

            st = cpssDxChBrgVlanPortProtoVlanQosSet(dev, port, entryNum, NULL, &qosCfg);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, entryNum);

            /*
                1.1.8. Call with entryNum [5],
                                 qosCfgPtr [NULL]
                                 and other valid parameters from 1.1.1.
                Expected: GT_OK.
            */
            entryNum = 5;

            st = cpssDxChBrgVlanPortProtoVlanQosSet(dev, port, entryNum, &vlanCfg, NULL);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, entryNum);

            /*
                1.1.9. Call with entryNum [6],
                                 vlanCfgPtr[NULL],
                                 qosCfgPtr [NULL].
                Expected: NON GT_ OK.
            */
            entryNum = 6;

            st = cpssDxChBrgVlanPortProtoVlanQosSet(dev, port, entryNum, NULL, NULL);
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                    "%d, %d, vlanCfgPtr = NULL, qosCfgPtr = NULL", dev, port);

            /*
                1.1.10. Call with entryNum [1],
                                  wrong enum values qosCfgPtr->qosAssignCmd,
                                  valid other fields of qosCfg
                                  and vlanCfgPtr [NULL]
                Expected: GT_BAD_PARAM.
            */
            entryNum = 1;

            UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanPortProtoVlanQosSet
                                                        (dev, port, entryNum, NULL, &qosCfg),
                                                        qosCfg.qosAssignCmd);

            qosCfg.qosAssignCmd  = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;

            /*
                1.1.11. Call with entryNum [1],
                                  wrong enum values qosCfgPtr->qosAssignPrecedence ,
                                  valid other fields of qosCfg
                                  and vlanCfgPtr [NULL]
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanPortProtoVlanQosSet
                                                        (dev, port, entryNum, NULL, &qosCfg),
                                                        qosCfg.qosAssignPrecedence);

            qosCfg.qosAssignPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;

            /* Getting device family */
            st = prvUtfDeviceFamilyGet(dev, &devFamily);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

            /*
                1.1.12. Call with entryNum [1],
                                  qosCfgPtr->qosProfileId [PRV_CPSS_DXCH_QOS_PROFILE_NUM_MAX_CNS],
                                  valid other fields of qosCfg
                                  and vlanCfgPtr [NULL]
                Expected: NON GT_OK for Cheetah, GT_OK for Cheetah2.
            */
            qosCfg.qosProfileId        = PRV_CPSS_DXCH_QOS_PROFILE_NUM_MAX_CNS;

            st = cpssDxChBrgVlanPortProtoVlanQosSet(dev, port, entryNum,
                                                    NULL, &qosCfg);
            if (devFamily >= CPSS_PP_FAMILY_CHEETAH2_E)
            {
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, qosCfgPtr->qosProfileId = %d",
                                             dev, port, qosCfg.qosProfileId);
            }
            else
            {
                UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, qosCfgPtr->qosProfileId = %d",
                                                 dev, port, qosCfg.qosProfileId);
            }

            /*
                1.1.13. Call with entryNum [1],
                                  qosCfgPtr->qosProfileId [PRV_CPSS_DXCH2_QOS_PROFILE_NUM_MAX_CNS],
                                  valid other fields of qosCfg
                                  and vlanCfgPtr [NULL]
                Expected: NON GT_OK.
            */
            qosCfg.qosProfileId = PRV_CPSS_DXCH_QOS_PROFILE_MAX_MAC(dev);

            st = cpssDxChBrgVlanPortProtoVlanQosSet(dev, port, entryNum,
                                                    NULL, &qosCfg);
            UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, qosCfgPtr->qosProfileId = %d",
                                                 dev, port, qosCfg.qosProfileId);
        }

        entryNum = 0;

        vlanCfg.vlanId                 = BRG_VLAN_TESTED_VLAN_ID_CNS;
        vlanCfg.vlanIdAssignCmd        = CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E;
        vlanCfg.vlanIdAssignPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;

        qosCfg.qosAssignCmd        = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_UNTAGGED_E;
        qosCfg.qosProfileId        = 10;
        qosCfg.qosAssignPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
        qosCfg.enableModifyUp      = GT_TRUE;
        qosCfg.enableModifyDscp    = GT_TRUE;

        st = prvUtfNextDefaultEPortReset(&tmpPort, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextDefaultEPortGet(&tmpPort, GT_FALSE))
        {
            port = tmpPort;
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanPortProtoVlanQosSet(dev, port, entryNum,
                                                    &vlanCfg, &qosCfg);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanPortProtoVlanQosSet(dev, port, entryNum,
                                                &vlanCfg, &qosCfg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanPortProtoVlanQosSet(dev, port, entryNum,
                                                &vlanCfg, &qosCfg);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            /* CPU port is in valid range */
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }

    entryNum = 0;

    vlanCfg.vlanId                 = BRG_VLAN_TESTED_VLAN_ID_CNS;
    vlanCfg.vlanIdAssignCmd        = CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E;
    vlanCfg.vlanIdAssignPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;

    qosCfg.qosAssignCmd        = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_UNTAGGED_E;
    qosCfg.qosProfileId        = 10;
    qosCfg.qosAssignPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
    qosCfg.enableModifyUp      = GT_TRUE;
    qosCfg.enableModifyDscp    = GT_TRUE;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = BRG_VLAN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortProtoVlanQosSet(dev, port, entryNum,
                                                &vlanCfg, &qosCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortProtoVlanQosSet(dev, port, entryNum,
                                                &vlanCfg, &qosCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortProtoVlanQosGet
(
    IN GT_U8                                    devNum,
    IN GT_U8                                    port,
    IN GT_U32                                   entryNum,
    OUT CPSS_DXCH_BRG_VLAN_PROT_CLASS_CFG_STC   *vlanCfgPtr,
    OUT CPSS_DXCH_BRG_QOS_PROT_CLASS_CFG_STC    *qosCfgPtr,
    OUT GT_BOOL                                 *validEntryPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortProtoVlanQosGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxChx)
    1.1.1. Call with entryNum [0],
                     non-null vlanCfgPtr,
                     non-null qosCfgPtr
                     and non-null validEntryPtr.
    Expected: GT_OK.
    1.1.2. Call with out of range entryNum [8] and other non-null parameters.
    Expected: non GT_OK.
    1.1.3. Call with entryNum [1],
                     vlanCfgPtr [NULL],
                     non-null qosCfgPtr
                     and non-null validEntryPtr.
    Expected: GT_OK.
    1.1.4. Call with entryNum [2],
                     non-null vlanCfgPtr,
                     qosCfgPtr [NULL]
                     and non-null validEntryPtr.
    Expected: GT_OK.
    1.1.5. Call with entryNum [2],
                     vlanCfgPtr [NULL],
                     qosCfgPtr [NULL]
                     and non-null validEntryPtr.
    Expected: GT_OK.
    1.1.6. Call with entryNum [3],
                     non-null vlanCfgPtr,
                     non-null qosCfgPtr
                     and null validEntryPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                             st       = GT_OK;

    GT_U8                                 dev;
    GT_PORT_NUM                           port     = BRG_VLAN_VALID_PHY_PORT_CNS;
    GT_PORT_NUM                           tmpPort;
    GT_U32                                entryNum = 0;
    CPSS_DXCH_BRG_VLAN_PROT_CLASS_CFG_STC vlanCfg;
    CPSS_DXCH_BRG_QOS_PROT_CLASS_CFG_STC  qosCfg;
    GT_BOOL                               validEntry;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextDefaultEPortReset(&tmpPort, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextDefaultEPortGet(&tmpPort, GT_TRUE))
        {
            /*
                1.1.1. Call with entryNum [0],
                                 non-null vlanCfgPtr,
                                 non-null qosCfgPtr
                                 and non-null validEntryPtr.
                Expected: GT_OK.
            */
            port = tmpPort;
            entryNum = 0;

            st = cpssDxChBrgVlanPortProtoVlanQosGet(dev, port, entryNum,
                                                    &vlanCfg, &qosCfg, &validEntry);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, entryNum);

            /*
                1.1.2. Call with out of range entryNum [8 or 12] and other non-null
                parameters. Expected: non GT_OK.
            */
            if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(dev))
            {
                entryNum = 12;
            }
            else
                entryNum = 8;

            st = cpssDxChBrgVlanPortProtoVlanQosGet(dev, port, entryNum,
                                                    &vlanCfg, &qosCfg, &validEntry);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, entryNum);

            /*
                1.1.3. Call with entryNum [1],
                                 vlanCfgPtr [NULL],
                                 non-null qosCfgPtr
                                 and non-null validEntryPtr.
                Expected: GT_OK.
            */
            entryNum = 1;

            st = cpssDxChBrgVlanPortProtoVlanQosGet(dev, port, entryNum,
                                                    NULL, &qosCfg, &validEntry);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, %d, vlanCfgPtr = NULL", dev, port);

            /*
                1.1.4. Call with entryNum [2],
                                 non-null vlanCfgPtr,
                                 qosCfgPtr [NULL]
                                 and non-null validEntryPtr.
                Expected: GT_OK.
            */
            entryNum = 2;

            st = cpssDxChBrgVlanPortProtoVlanQosGet(dev, port, entryNum,
                                                    &vlanCfg, NULL, &validEntry);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, %d, qosCfg = NULL", dev, port);

            /*
                1.1.5. Call with entryNum [2],
                                 vlanCfgPtr [NULL],
                                 qosCfgPtr [NULL]
                                 and non-null validEntryPtr.
                Expected: GT_OK.
            */
            st = cpssDxChBrgVlanPortProtoVlanQosGet(dev, port, entryNum,
                                                    NULL, NULL, &validEntry);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, %d, vlanCfgPtr = NULL, qosCfg = NULL",
                                             dev, port);

            /*
                1.1.6. Call with entryNum [3],
                                 non-null vlanCfgPtr,
                                 non-null qosCfgPtr
                                 and null validEntryPtr [NULL].
                Expected: GT_BAD_PTR.
            */
            entryNum = 3;

            st = cpssDxChBrgVlanPortProtoVlanQosGet(dev, port, entryNum,
                                                    &vlanCfg, &qosCfg, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, validEntry = NULL", dev, port);
        }

        entryNum = 0;

        st = prvUtfNextDefaultEPortReset(&tmpPort, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextDefaultEPortGet(&tmpPort, GT_FALSE))
        {
            port = tmpPort;
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanPortProtoVlanQosGet(dev, port, entryNum,
                                                    &vlanCfg, &qosCfg, &validEntry);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanPortProtoVlanQosGet(dev, port, entryNum,
                                                &vlanCfg, &qosCfg, &validEntry);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanPortProtoVlanQosGet(dev, port, entryNum,
                                                &vlanCfg, &qosCfg, &validEntry);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            /* CPU port is in valid range */
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }

    entryNum = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = BRG_VLAN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortProtoVlanQosGet(dev, port, entryNum,
                                                &vlanCfg, &qosCfg, &validEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortProtoVlanQosGet(dev, port, entryNum,
                                            &vlanCfg, &qosCfg, &validEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortProtoVlanQosInvalidate
(
    IN GT_U8    devNum,
    IN GT_U8    port,
    IN GT_U32   entryNum
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortProtoVlanQosInvalidate)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxChx)
    1.1.1. Call with entryNum [0].
    Expected: GT_OK.
    1.1.2. Call with out of range entryNum [8].
    Expected: non GT_OK.
*/
    GT_STATUS  st       = GT_OK;

    GT_U8      dev;
    GT_PORT_NUM port     = BRG_VLAN_VALID_PHY_PORT_CNS;
    GT_U32     entryNum = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextDefaultEPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextDefaultEPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with entryNum [0].
                Expected: GT_OK.
            */
            entryNum = 0;

            st = cpssDxChBrgVlanPortProtoVlanQosInvalidate(dev, port, entryNum);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, entryNum);

            /*
                1.1.2. Call with out of range entryNum [8].
                Expected: non GT_OK.
            */
            if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(dev))
            {
                entryNum = 12;
            }
            else
                entryNum = 8;

            st = cpssDxChBrgVlanPortProtoVlanQosInvalidate(dev, port, entryNum);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, entryNum);
        }

        entryNum = 0;

        st = prvUtfNextDefaultEPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextDefaultEPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanPortProtoVlanQosInvalidate(dev, port, entryNum);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_GET_MAX_DEFAULT_EPORTS_NUM_MAC(dev);

        st = cpssDxChBrgVlanPortProtoVlanQosInvalidate(dev, port, entryNum);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanPortProtoVlanQosInvalidate(dev, port, entryNum);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            /* CPU port is in valid range */
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }

    entryNum = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = BRG_VLAN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortProtoVlanQosInvalidate(dev, port, entryNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortProtoVlanQosInvalidate(dev, port, entryNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanLearningStateSet
(
    IN GT_U8    devNum,
    IN GT_U16   vlanId,
    IN GT_BOOL  status
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanLearningStateSet)
{
/*
    ITERATE_DEVICES (DxCh2 and above)
    1.1. Call with vlanId [100] and status [GT_FALSE and GT_ TRUE].
    Expected: GT_OK.
    1.2. Call with out of range vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)]
         and status [GT_FALSE].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st     = GT_OK;

    GT_U8     dev;
    GT_U16    vlanId = 0;
    GT_BOOL   status = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with vlanId [100] and status [GT_FALSE and GT_ TRUE].
            Expected: GT_OK.
        */

        /* Call with status [GT_FALSE] */
        vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;
        status = GT_FALSE;

        st = cpssDxChBrgVlanLearningStateSet(dev, vlanId, status);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlanId, status);

        /* Call with status [GT_TRUE] */
        status = GT_TRUE;

        st = cpssDxChBrgVlanLearningStateSet(dev, vlanId, status);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlanId, status);

        /*
            1.2. Call with out of range vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)]
                           and status [GT_FALSE].
            Expected: GT_BAD_PARAM.
        */
        vlanId = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev);
        status = GT_FALSE;

        st = cpssDxChBrgVlanLearningStateSet(dev, vlanId, status);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, vlanId, status);
    }

    vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;
    status = GT_FALSE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanLearningStateSet(dev, vlanId, status);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanLearningStateSet(dev, vlanId, status);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_BOOL cpssDxChBrgVlanIsDevMember
(
    IN GT_U8    devNum,
    IN GT_U16   vlanId
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanIsDevMember)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with vlanId [100].
    Expected: GT_OK.
    1.2. Call with out of range vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)].
    Expected: GT_BAD_PARAM.
*/
    GT_BOOL isMember     = GT_OK;

    GT_U8     dev;
    GT_U16    vlanId = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with vlanId [100].
            Expected: GT_OK.
        */
        vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;

        isMember = cpssDxChBrgVlanIsDevMember(dev, vlanId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_FALSE, isMember, dev, vlanId);

        /*
            1.2. Call with out of range vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)].
            Expected: GT_BAD_PARAM.
        */
        vlanId = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev);

        isMember = cpssDxChBrgVlanIsDevMember(dev, vlanId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, isMember, dev, vlanId);
    }

    vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        isMember = cpssDxChBrgVlanIsDevMember(dev, vlanId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, isMember, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    isMember = cpssDxChBrgVlanIsDevMember(dev, vlanId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, isMember, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanToStpIdBind
(
    IN GT_U8    devNum,
    IN GT_U16   vlanId,
    IN GT_U16   stpId
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanToStpIdBind)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with vlanId [100],
                   stpId [10].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgVlanStpIdGet with same vlandId.
    Expected: GT_OK and the same stpId.
    1.3. Call with vlanId [100],
                   stpId [PRV_CPSS_DXCH_STG_MAX_NUM_CNS].
    Expected: NON  GT_OK.
    1.4. Call with out of range vlandId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)]
                   and stpId [5].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st       = GT_OK;

    GT_U8     dev;
    GT_U16    vlanId   = 0;
    GT_U16    stpId    = 0;
    GT_U16    stpIdGet = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with vlanId [100],
                           stpId [10].
            Expected: GT_OK.
        */
        vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;
        stpId  = 10;

        st = cpssDxChBrgVlanToStpIdBind(dev, vlanId, stpId);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlanId, stpId);

        /*
            1.2. Call cpssDxChBrgVlanStpIdGet with same vlandId.
            Expected: GT_OK and the same stpId.
        */
        st = cpssDxChBrgVlanStpIdGet(dev, vlanId, &stpIdGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgVlanStpIdGet: %d, %d", dev, vlanId);

        UTF_VERIFY_EQUAL1_STRING_MAC(stpId, stpIdGet,
                   "get another stpId than was set: %d", dev);

        /*
            1.3. Call with vlanId [100],
                           stpId [PRV_CPSS_DXCH_STG_MAX_NUM_CNS].
            Expected: NON  GT_OK.
        */
        vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;
        stpId  = (GT_U16)UTF_CPSS_PP_MAX_STG_ID_NUM_CNS(dev);

        st = cpssDxChBrgVlanToStpIdBind(dev, vlanId, stpId);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, stpId = %d", dev, stpId);

        /*
            1.4. Call with out of range vlandId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)]
                               and stpId [5].
            Expected: GT_BAD_PARAM.
        */
        vlanId = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev);
        stpId  = 5;

        st = cpssDxChBrgVlanToStpIdBind(dev, vlanId, stpId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, vlanId);
    }

    vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;
    stpId  = 10;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanToStpIdBind(dev, vlanId, stpId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanToStpIdBind(dev, vlanId, stpId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanStpIdGet
(
    IN  GT_U8    devNum,
    IN  GT_U16   vlanId,
    OUT GT_U16   *stpIdPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanStpIdGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with vlanId [100],
                   non-null stpIdPtr.
    Expected: GT_OK.
    1.2. Call with vlanId [100]
                   and stpIdPtr [NULL].
    Expected: GT_BAD_PTR.
    1.3. Call with out of range vlandId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)]
                   and non-null stpIdPtr.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st     = GT_OK;

    GT_U8     dev;
    GT_U16    vlanId = 0;
    GT_U16    stpId  = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with vlanId [100],
                           non-null stpIdPtr.
            Expected: GT_OK.
        */
        vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;

        st = cpssDxChBrgVlanStpIdGet(dev, vlanId, &stpId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, vlanId);

        /*
            1.2. Call with vlanId [100]
                           and stpIdPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgVlanStpIdGet(dev, vlanId, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, stpIdPtr = NULL", dev);

        /*
            1.3. Call with out of range vlandId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)]
                           and non-null stpIdPtr.
            Expected: GT_BAD_PARAM.
        */
        vlanId = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev);

        st = cpssDxChBrgVlanStpIdGet(dev, vlanId, &stpId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, vlanId);
    }

    vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanStpIdGet(dev, vlanId, &stpId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanStpIdGet(dev, vlanId, &stpId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanEtherTypeSet
(
    IN  GT_U8                   devNum,
    IN  CPSS_ETHER_MODE_ENT     etherTypeMode,
    IN  GT_U16                  etherType,
    IN  GT_U16                  vidRange
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanEtherTypeSet)
{
/*
    ITERATE_DEVICES (DxCh1, DxCh2 and DxCh3)
    1.1. Call with etherTypeMode [CPSS_VLAN_ETHERTYPE0_E /
                                  CPSS_VLAN_ETHERTYPE1_E],
                   etherType [0],
                   and vidRange [100].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgVlanEtherTypeGet
    Expected: GT_OK and same etherType, vidRange.
    1.3. Call with etherTypeMode [CPSS_VLAN_ETHERTYPE0_E],
                   etherType [FFFF] (no any constraints),
                   and vidRange [100].
    Expected: GT_OK.
    1.4. Call with out of range vidRange [UTF_CPSS_PP_MAX_VLAN_NUM_CNS(dev)]
                   and other valid parameters.
    Expected: GT_BAD_PARAM.
    1.5. Call with out of range etherTypeMode
                   and other valid parameters.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st    = GT_OK;

    GT_U8               dev;
    CPSS_ETHER_MODE_ENT etherTypeMode = CPSS_VLAN_ETHERTYPE0_E;
    GT_U16              etherType     = 0;
    GT_U16              vidRange      = 0;
    GT_U16              etherTypeGet  = 0;
    GT_U16              vidRangeGet   = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with etherTypeMode [CPSS_VLAN_ETHERTYPE0_E /
                                          CPSS_VLAN_ETHERTYPE1_E],
                           etherType [0],
                           and vidRange [100].
            Expected: GT_OK.
        */

        /* Call with etherTypeMode [CPSS_VLAN_ETHERTYPE0_E] */
        etherTypeMode = CPSS_VLAN_ETHERTYPE0_E;
        etherType     = 0;
        vidRange      = BRG_VLAN_TESTED_VLAN_ID_CNS;

        st = cpssDxChBrgVlanEtherTypeSet(dev, etherTypeMode, etherType, vidRange);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, etherTypeMode, etherType, vidRange);

        /* 1.2 */
        st = cpssDxChBrgVlanEtherTypeGet(dev, etherTypeMode, &etherTypeGet, &vidRangeGet);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, etherTypeMode, etherType, vidRange);

        UTF_VERIFY_EQUAL1_STRING_MAC(etherType, etherTypeGet,
            "get another etherType than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(vidRange, vidRangeGet,
            "get another vidRange than was set: %d", dev);

        /* Call with etherTypeMode [CPSS_VLAN_ETHERTYPE1_E] */
        etherTypeMode = CPSS_VLAN_ETHERTYPE1_E;

        st = cpssDxChBrgVlanEtherTypeSet(dev, etherTypeMode, etherType, vidRange);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, etherTypeMode, etherType, vidRange);

        /* 1.2 */
        st = cpssDxChBrgVlanEtherTypeGet(dev, etherTypeMode, &etherTypeGet, &vidRangeGet);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, etherTypeMode, etherType, vidRange);

        UTF_VERIFY_EQUAL1_STRING_MAC(etherType, etherTypeGet,
            "get another etherType than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(vidRange, vidRangeGet,
            "get another vidRange than was set: %d", dev);

        /*
            1.3. Call with etherTypeMode [CPSS_VLAN_ETHERTYPE0_E],
                           etherType [FFFF] (no any constraints),
                           and vidRange [100].
            Expected: GT_OK.
        */
        etherTypeMode = CPSS_VLAN_ETHERTYPE0_E;
        etherType     = 0xFFFF;

        st = cpssDxChBrgVlanEtherTypeSet(dev, etherTypeMode, etherType, vidRange);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, etherTypeMode, etherType, vidRange);

        /*
            1.4. Call with out of range vidRange [UTF_CPSS_PP_MAX_VLAN_NUM_CNS(dev)]
                           and other valid parameters.
            Expected: GT_BAD_PARAM.
        */
        etherType = 0;
        vidRange  = UTF_CPSS_PP_MAX_VLAN_NUM_CNS(dev);

        st = cpssDxChBrgVlanEtherTypeSet(dev, etherTypeMode, etherType, vidRange);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, vidRange = %d", dev, vidRange);

        /*
            1.5. Call with wrong enum values etherTypeMode and other valid parameters.
            Expected: GT_BAD_PARAM.
        */
        vidRange = BRG_VLAN_TESTED_VLAN_ID_CNS;

        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanEtherTypeSet
                                                        (dev, etherTypeMode, etherType, vidRange),
                                                        etherTypeMode);
    }

    etherTypeMode = CPSS_VLAN_ETHERTYPE0_E;
    etherType     = 0;
    vidRange      = BRG_VLAN_TESTED_VLAN_ID_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanEtherTypeSet(dev, etherTypeMode, etherType, vidRange);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanEtherTypeSet(dev, etherTypeMode, etherType, vidRange);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanEtherTypeGet)
{
/*
    ITERATE_DEVICES (DxCh1, DxCh2 and DxCh3)
    1.1. Call with etherTypeMode [CPSS_VLAN_ETHERTYPE0_E /
                                  CPSS_VLAN_ETHERTYPE1_E],
                   and not NULL etherType, vidRange.
    Expected: GT_OK.
    1.2. Call with out of range etherTypeMode
                   and other valid parameters.
    Expected: GT_BAD_PARAM.
    1.3. Call with NULL etherType
    Expected: GT_BAD_PTR.
    1.4. Call with NULL vidRange
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st    = GT_OK;

    GT_U8               dev;
    CPSS_ETHER_MODE_ENT etherTypeMode = CPSS_VLAN_ETHERTYPE0_E;
    GT_U16              etherType     = 0;
    GT_U16              vidRange      = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. */
        etherTypeMode = CPSS_VLAN_ETHERTYPE0_E;

        st = cpssDxChBrgVlanEtherTypeGet(dev, etherTypeMode, &etherType, &vidRange);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, etherTypeMode);

        /*  1.1. */
        etherTypeMode = CPSS_VLAN_ETHERTYPE1_E;

        st = cpssDxChBrgVlanEtherTypeGet(dev, etherTypeMode, &etherType, &vidRange);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, etherTypeMode);

        /* 1.2. */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanEtherTypeGet
                                                        (dev, etherTypeMode, &etherType, &vidRange),
                                                        etherTypeMode);

        /*  1.3. */
        st = cpssDxChBrgVlanEtherTypeGet(dev, etherTypeMode, NULL, &vidRange);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, etherTypeMode);

        /*  1.4. */
        st = cpssDxChBrgVlanEtherTypeGet(dev, etherTypeMode, &etherType, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, etherTypeMode);
    }

    etherTypeMode = CPSS_VLAN_ETHERTYPE0_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanEtherTypeGet(dev, etherTypeMode, &etherType, &vidRange);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanEtherTypeGet(dev, etherTypeMode, &etherType, &vidRange);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanTableInvalidate
(
    IN GT_U8 devNum
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanTableInvalidate)
{
/*
    ITERATE_DEVICES (DxChx)
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    /* Skip this test for GM - performance reason */
    GM_SKIP_TEST_DUE_TO_POOR_PERFORMANCE_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChBrgVlanTableInvalidate(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanTableInvalidate(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanTableInvalidate(dev);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanEntryInvalidate
(
    IN GT_U8    devNum,
    IN GT_U16   vlanId
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanEntryInvalidate)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with vlanId [100].
    Expected: GT_OK.
    1.2. Call with out of range vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st     = GT_OK;

    GT_U8     dev;
    GT_U16    vlanId = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with vlanId [100].
            Expected: GT_OK.
        */
        vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;

        st = cpssDxChBrgVlanEntryInvalidate(dev, vlanId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, vlanId);

        /*
            1.2. Call with out of range vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)].
            Expected: GT_BAD_PARAM.
        */
        vlanId = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev);

        st = cpssDxChBrgVlanEntryInvalidate(dev, vlanId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, vlanId);
    }

    vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanEntryInvalidate(dev, vlanId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanEntryInvalidate(dev, vlanId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanUnkUnregFilterSet
(
    IN GT_U8                                devNum,
    IN GT_U16                               vlanId,
    IN CPSS_DXCH_BRG_VLAN_PACKET_TYPE_ENT   packetType,
    IN CPSS_PACKET_CMD_ENT                  cmd
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanUnkUnregFilterSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with vlanId [100],
                   packetType [CPSS_DXCH_BRG_VLAN_PACKET_UNK_UCAST_E /
                               CPSS_DXCH_BRG_VLAN_PACKET_UNREG_NON_IP_MCAST_E /
                               CPSS_DXCH_BRG_VLAN_PACKET_UNREG_IPV4_MCAST_E /
                               CPSS_DXCH_BRG_VLAN_PACKET_UNREG_IPV6_MCAST_E /
                               CPSS_DXCH_BRG_VLAN_PACKET_UNREG_NON_IPV4_BCAST_E /
                               CPSS_DXCH_BRG_VLAN_PACKET_UNREG_IPV4_BCAST_E]
                   and cmd [CPSS_PACKET_CMD_FORWARD_E /
                            CPSS_PACKET_CMD_MIRROR_TO_CPU_E /
                            CPSS_PACKET_CMD_TRAP_TO_CPU_E /
                            CPSS_PACKET_CMD_DROP_HARD_E /
                            CPSS_PACKET_CMD_DROP_SOFT_E].
    Expected: GT_OK.
    1.2. Call with out of range vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)],
                   packetType [CPSS_DXCH_BRG_VLAN_PACKET_UNK_UCAST_E]
                   and cmd [CPSS_PACKET_CMD_FORWARD_E].
    Expected: GT_BAD_PARAM.
    1.3. Call with vlanId[100],
                   out of range packetType
                   and cmd [CPSS_PACKET_CMD_MIRROR_TO_CPU_E].
    Expected: GT_BAD_PARAM.
    1.4. Call with vlanId[100],
                   packetType [CPSS_DXCH_BRG_VLAN_PACKET_UNREG_NON_IP_MCAST_E]
                   and out of range cmd.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS  st    = GT_OK;


    GT_U8                              dev;
    GT_U16                             vlanId     = 0;
    CPSS_DXCH_BRG_VLAN_PACKET_TYPE_ENT packetType = CPSS_DXCH_BRG_VLAN_PACKET_UNK_UCAST_E;
    CPSS_PACKET_CMD_ENT                cmd        = CPSS_PACKET_CMD_FORWARD_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with vlanId [100],
                           packetType [CPSS_DXCH_BRG_VLAN_PACKET_UNK_UCAST_E /
                                       CPSS_DXCH_BRG_VLAN_PACKET_UNREG_NON_IP_MCAST_E /
                                       CPSS_DXCH_BRG_VLAN_PACKET_UNREG_IPV4_MCAST_E /
                                       CPSS_DXCH_BRG_VLAN_PACKET_UNREG_IPV6_MCAST_E /
                                       CPSS_DXCH_BRG_VLAN_PACKET_UNREG_NON_IPV4_BCAST_E /
                                       CPSS_DXCH_BRG_VLAN_PACKET_UNREG_IPV4_BCAST_E]
                           and cmd [CPSS_PACKET_CMD_FORWARD_E /
                                    CPSS_PACKET_CMD_MIRROR_TO_CPU_E /
                                    CPSS_PACKET_CMD_TRAP_TO_CPU_E /
                                    CPSS_PACKET_CMD_DROP_HARD_E /
                                    CPSS_PACKET_CMD_DROP_SOFT_E].
            Expected: GT_OK.
        */
        vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;

        /* Call with packetType [CPSS_DXCH_BRG_VLAN_PACKET_UNK_UCAST_E] */
        /*           and cmd [CPSS_PACKET_CMD_FORWARD_E]                */
        packetType = CPSS_DXCH_BRG_VLAN_PACKET_UNK_UCAST_E;
        cmd        = CPSS_PACKET_CMD_FORWARD_E;

        st = cpssDxChBrgVlanUnkUnregFilterSet(dev, vlanId, packetType, cmd);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, vlanId, packetType, cmd);

        /* Call with packetType [CPSS_DXCH_BRG_VLAN_PACKET_UNREG_NON_IP_MCAST_E] */
        /*           and cmd [CPSS_PACKET_CMD_MIRROR_TO_CPU_E]                   */
        packetType = CPSS_DXCH_BRG_VLAN_PACKET_UNREG_NON_IP_MCAST_E;
        cmd        = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;

        st = cpssDxChBrgVlanUnkUnregFilterSet(dev, vlanId, packetType, cmd);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, vlanId, packetType, cmd);

        /* Call with packetType [CPSS_DXCH_BRG_VLAN_PACKET_UNREG_IPV4_MCAST_E] */
        /*           and cmd [CPSS_PACKET_CMD_TRAP_TO_CPU_E]                   */
        packetType = CPSS_DXCH_BRG_VLAN_PACKET_UNK_UCAST_E;
        cmd        = CPSS_PACKET_CMD_FORWARD_E;

        st = cpssDxChBrgVlanUnkUnregFilterSet(dev, vlanId, packetType, cmd);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, vlanId, packetType, cmd);

        /* Call with packetType [CPSS_DXCH_BRG_VLAN_PACKET_UNREG_IPV6_MCAST_E] */
        /*           and cmd [CPSS_PACKET_CMD_DROP_HARD_E]                     */
        packetType = CPSS_DXCH_BRG_VLAN_PACKET_UNREG_IPV6_MCAST_E;
        cmd        = CPSS_PACKET_CMD_DROP_HARD_E;

        st = cpssDxChBrgVlanUnkUnregFilterSet(dev, vlanId, packetType, cmd);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, vlanId, packetType, cmd);

        /* Call with packetType [CPSS_DXCH_BRG_VLAN_PACKET_UNREG_NON_IPV4_BCAST_E] */
        /*           and cmd [CPSS_PACKET_CMD_DROP_SOFT_E]                         */
        packetType = CPSS_DXCH_BRG_VLAN_PACKET_UNREG_NON_IPV4_BCAST_E;
        cmd        = CPSS_PACKET_CMD_DROP_SOFT_E;

        st = cpssDxChBrgVlanUnkUnregFilterSet(dev, vlanId, packetType, cmd);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, vlanId, packetType, cmd);

        /*
            1.2. Call with out of range vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)],
                           packetType [CPSS_DXCH_BRG_VLAN_PACKET_UNK_UCAST_E]
                           and cmd [CPSS_PACKET_CMD_FORWARD_E].
            Expected: GT_BAD_PARAM.
        */
        vlanId     = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev);
        packetType = CPSS_DXCH_BRG_VLAN_PACKET_UNK_UCAST_E;
        cmd        = CPSS_PACKET_CMD_FORWARD_E;

        st = cpssDxChBrgVlanUnkUnregFilterSet(dev, vlanId, packetType, cmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, vlanId);

        /*
            1.3. Call with vlanId[100], wrong enum values packetType
                           and cmd [CPSS_PACKET_CMD_MIRROR_TO_CPU_E].
            Expected: GT_BAD_PARAM.
        */
        vlanId     = BRG_VLAN_TESTED_VLAN_ID_CNS;
        cmd        = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;

        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanUnkUnregFilterSet
                                                        (dev, vlanId, packetType, cmd),
                                                        packetType);

        packetType = CPSS_DXCH_BRG_VLAN_PACKET_UNREG_NON_IP_MCAST_E;

        /*
            1.4. Call with vlanId[100],
                           packetType [CPSS_DXCH_BRG_VLAN_PACKET_UNREG_NON_IP_MCAST_E]
                           and wrong enum values cmd.
            Expected: GT_BAD_PARAM.
        */
        packetType = CPSS_DXCH_BRG_VLAN_PACKET_UNREG_NON_IP_MCAST_E;

        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanUnkUnregFilterSet
                                                        (dev, vlanId, packetType, cmd),
                                                        cmd);
    }

    vlanId     = BRG_VLAN_TESTED_VLAN_ID_CNS;
    packetType = CPSS_DXCH_BRG_VLAN_PACKET_UNK_UCAST_E;
    cmd        = CPSS_PACKET_CMD_FORWARD_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanUnkUnregFilterSet(dev, vlanId, packetType, cmd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanUnkUnregFilterSet(dev, vlanId, packetType, cmd);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanForcePvidEnable
(
    IN GT_U8   devNum,
    IN GT_U8   port,
    IN GT_BOOL enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanForcePvidEnable)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxChx)
    1.1.1. Call with enable [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call get func.
    Expected: GT_OK and the same state.
*/
    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_PORT_NUM       port  = BRG_VLAN_VALID_PHY_PORT_CNS;
    GT_BOOL     state = GT_TRUE;
    GT_BOOL     stateGet = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextDefaultEPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextDefaultEPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with enable [GT_TRUE and GT_FALSE].
                Expected: GT_OK.
            */

            /* Call function with enable [GT_FALSE] */
            state = GT_FALSE;

            st = cpssDxChBrgVlanForcePvidEnable(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*
                1.1.2. Call get func.
                Expected: GT_OK and the same state.
            */

            st = cpssDxChBrgVlanForcePvidEnableGet(dev, port, &stateGet);

            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);
            UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                               "get another state than was set: %d", dev);

            /* Call function with enable [GT_TRUE] */
            state = GT_TRUE;

            st = cpssDxChBrgVlanForcePvidEnable(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*
                1.1.2. Call get func.
                Expected: GT_OK and the same state.
            */

            st = cpssDxChBrgVlanForcePvidEnableGet(dev, port, &stateGet);

            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);
            UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                               "get another state than was set: %d", dev);
        }

        state = GT_TRUE;

        st = prvUtfNextDefaultEPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextDefaultEPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanForcePvidEnable(dev, port, state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanForcePvidEnable(dev, port, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanForcePvidEnable(dev, port, state);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        else
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    state = GT_TRUE;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = BRG_VLAN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanForcePvidEnable(dev, port, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanForcePvidEnable(dev, port, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanForcePvidEnableGet
(
    IN GT_U8   devNum,
    IN GT_U8   port,
    OUT GT_BOOL *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanForcePvidEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxChx)
    1.1.1. Call with not null enable.
    Expected: GT_OK.
    1.1.2. Call with wrong enable [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_PORT_NUM       port  = BRG_VLAN_VALID_PHY_PORT_CNS;
    GT_BOOL     state = GT_TRUE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextDefaultEPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextDefaultEPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with not null enable.
                Expected: GT_OK.
            */

            st = cpssDxChBrgVlanForcePvidEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*
                1.1.2. Call with wrong enable [NULL].
                Expected: GT_BAD_PTR.
            */

            st = cpssDxChBrgVlanForcePvidEnableGet(dev, port, &state);

            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);
        }

        st = prvUtfNextDefaultEPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextDefaultEPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanForcePvidEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanForcePvidEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanForcePvidEnableGet(dev, port, &state);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        else
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = BRG_VLAN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanForcePvidEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanForcePvidEnableGet(dev, port, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortVidPrecedenceSet
(
    IN GT_U8                                        devNum,
    IN GT_U8                                        port,
    IN CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT  precedence
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortVidPrecedenceSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxChx)
    1.1.1. Call with precedence [CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E /
                                 CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E].
    Expected: GT_OK.
    1.1.2. Call with out of range precedence.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS  st    = GT_OK;


    GT_U8                                       dev;
    GT_PORT_NUM                                 port       = BRG_VLAN_VALID_PHY_PORT_CNS;
    CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT precedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextDefaultEPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextDefaultEPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with precedence [CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E /
                                             CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E].
                Expected: GT_OK.
            */

            /* Call function with precedence [CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E] */
            precedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;

            st = cpssDxChBrgVlanPortVidPrecedenceSet(dev, port, precedence);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, precedence);

            /* Call function with precedence [CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E] */
            precedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;

            st = cpssDxChBrgVlanPortVidPrecedenceSet(dev, port, precedence);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, precedence);

            /*
                1.1.2. Call with wrong enum values precedence.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanPortVidPrecedenceSet
                                                        (dev, port, precedence),
                                                        precedence);
        }

        precedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;

        st = prvUtfNextDefaultEPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextDefaultEPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanPortVidPrecedenceSet(dev, port, precedence);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanPortVidPrecedenceSet(dev, port, precedence);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanPortVidPrecedenceSet(dev, port, precedence);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        else
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    precedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = BRG_VLAN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortVidPrecedenceSet(dev, port, precedence);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortVidPrecedenceSet(dev, port, precedence);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanIpUcRouteEnable
(
    IN GT_U8                                    devNum,
    IN GT_U16                                   vlanId,
    IN CPSS_IP_PROTOCOL_STACK_ENT               protocol,
    IN GT_BOOL                                  enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanIpUcRouteEnable)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with vlanId [100],
                   protocol [CPSS_IP_PROTOCOL_IPV4_E /
                             CPSS_IP_PROTOCOL_IPV6_E /
                             CPSS_IP_PROTOCOL_IPV4V6_E]
                   and enable [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.2. Call with out of range vlandId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)],
                   protocol [CPSS_IP_PROTOCOL_IPV4V6_E]
                   and enable [GT_TRUE].
    Expected: GT_BAD_PARAM.
    1.3. Call with vlanId [100],
                   out of range protocol
                   and enable [GT_TRUE].
    Expected: GT_BAD_PARAM.

*/
    GT_STATUS  st    = GT_OK;


    GT_U8                      dev;
    GT_U16                     vlanId   = 0;
    CPSS_IP_PROTOCOL_STACK_ENT protocol = CPSS_IP_PROTOCOL_IPV4_E;
    GT_BOOL                    enable   = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with vlanId [100],
                           protocol [CPSS_IP_PROTOCOL_IPV4_E /
                                     CPSS_IP_PROTOCOL_IPV6_E /
                                     CPSS_IP_PROTOCOL_IPV4V6_E]
                           and enable [GT_FALSE and GT_TRUE].
            Expected: GT_OK.
        */

        vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;

        /* Call with protocol [CPSS_IP_PROTOCOL_IPV4_E] and enable [GT_FALSE] */
        protocol = CPSS_IP_PROTOCOL_IPV4_E;
        enable   = GT_FALSE;

        st = cpssDxChBrgVlanIpUcRouteEnable(dev, vlanId, protocol, enable);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, vlanId, protocol, enable);

        /* Call with protocol [CPSS_IP_PROTOCOL_IPV6_E] and enable [GT_FALSE] */
        protocol = CPSS_IP_PROTOCOL_IPV6_E;

        st = cpssDxChBrgVlanIpUcRouteEnable(dev, vlanId, protocol, enable);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, vlanId, protocol, enable);

        /* Call with protocol [CPSS_IP_PROTOCOL_IPV4V6_E] and enable [GT_FALSE] */
        protocol = CPSS_IP_PROTOCOL_IPV4V6_E;

        st = cpssDxChBrgVlanIpUcRouteEnable(dev, vlanId, protocol, enable);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, vlanId, protocol, enable);

        /* Call with protocol [CPSS_IP_PROTOCOL_IPV4_E] and enable [GT_TRUE] */
        protocol = CPSS_IP_PROTOCOL_IPV4_E;
        enable   = GT_TRUE;

        st = cpssDxChBrgVlanIpUcRouteEnable(dev, vlanId, protocol, enable);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, vlanId, protocol, enable);

        /* Call with protocol [CPSS_IP_PROTOCOL_IPV6_E] and enable [GT_TRUE] */
        protocol = CPSS_IP_PROTOCOL_IPV6_E;

        st = cpssDxChBrgVlanIpUcRouteEnable(dev, vlanId, protocol, enable);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, vlanId, protocol, enable);

        /* Call with protocol [CPSS_IP_PROTOCOL_IPV4V6_E] and enable [GT_TRUE] */
        protocol = CPSS_IP_PROTOCOL_IPV4V6_E;

        st = cpssDxChBrgVlanIpUcRouteEnable(dev, vlanId, protocol, enable);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, vlanId, protocol, enable);


        /*
            1.2. Call with out of range vlandId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)],
                           protocol [CPSS_IP_PROTOCOL_IPV4V6_E]
                           and enable [GT_TRUE].
            Expected: GT_BAD_PARAM.
        */
        vlanId   = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev);
        protocol = CPSS_IP_PROTOCOL_IPV4V6_E;

        st = cpssDxChBrgVlanIpUcRouteEnable(dev, vlanId, protocol, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, vlanId);

        /*
            1.3. Call with vlanId [100], wrong enum values protocol and enable [GT_TRUE].
            Expected: GT_BAD_PARAM.
        */
        vlanId   = BRG_VLAN_TESTED_VLAN_ID_CNS;

        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanIpUcRouteEnable
                                                        (dev, vlanId, protocol, enable),
                                                        protocol);
    }

    vlanId   = BRG_VLAN_TESTED_VLAN_ID_CNS;
    protocol = CPSS_IP_PROTOCOL_IPV4_E;
    enable   = GT_FALSE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanIpUcRouteEnable(dev, vlanId, protocol, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanIpUcRouteEnable(dev, vlanId, protocol, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanIpMcRouteEnable
(
    IN GT_U8                            devNum,
    IN GT_U16                           vlanId,
    IN CPSS_IP_PROTOCOL_STACK_ENT       protocol,
    IN GT_BOOL                          enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanIpMcRouteEnable)
{
/*
    ITERATE_DEVICES (DxCh2 and above)
    1.1. Call with vlanId [100],
                   protocol [CPSS_IP_PROTOCOL_IPV4_E /
                             CPSS_IP_PROTOCOL_IPV6_E /
                             CPSS_IP_PROTOCOL_IPV4V6_E]
                   and enable [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.2. Call with out of range vlandId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)],
                   protocol [CPSS_IP_PROTOCOL_IPV4V6_E]
                   and enable [GT_TRUE].
    Expected: GT_BAD_PARAM.
    1.3. Call with vlanId [100],
                   out of range protocol
                   and enable [GT_TRUE].
    Expected: GT_BAD_PARAM.

*/
    GT_STATUS  st    = GT_OK;


    GT_U8                      dev;
    GT_U16                     vlanId   = 0;
    CPSS_IP_PROTOCOL_STACK_ENT protocol = CPSS_IP_PROTOCOL_IPV4_E;
    GT_BOOL                    enable   = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with vlanId [100],
                           protocol [CPSS_IP_PROTOCOL_IPV4_E /
                                     CPSS_IP_PROTOCOL_IPV6_E /
                                     CPSS_IP_PROTOCOL_IPV4V6_E]
                           and enable [GT_FALSE and GT_TRUE].
            Expected: GT_OK.
        */

        vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;

        /* Call with protocol [CPSS_IP_PROTOCOL_IPV4_E] and enable [GT_FALSE] */
        protocol = CPSS_IP_PROTOCOL_IPV4_E;
        enable   = GT_FALSE;

        st = cpssDxChBrgVlanIpMcRouteEnable(dev, vlanId, protocol, enable);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, vlanId, protocol, enable);

        /* Call with protocol [CPSS_IP_PROTOCOL_IPV6_E] and enable [GT_FALSE] */
        protocol = CPSS_IP_PROTOCOL_IPV6_E;

        st = cpssDxChBrgVlanIpMcRouteEnable(dev, vlanId, protocol, enable);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, vlanId, protocol, enable);

        /* Call with protocol [CPSS_IP_PROTOCOL_IPV4V6_E] and enable [GT_FALSE] */
        protocol = CPSS_IP_PROTOCOL_IPV4V6_E;

        st = cpssDxChBrgVlanIpMcRouteEnable(dev, vlanId, protocol, enable);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, vlanId, protocol, enable);

        /* Call with protocol [CPSS_IP_PROTOCOL_IPV4_E] and enable [GT_TRUE] */
        protocol = CPSS_IP_PROTOCOL_IPV4_E;
        enable   = GT_TRUE;

        st = cpssDxChBrgVlanIpMcRouteEnable(dev, vlanId, protocol, enable);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, vlanId, protocol, enable);

        /* Call with protocol [CPSS_IP_PROTOCOL_IPV6_E] and enable [GT_TRUE] */
        protocol = CPSS_IP_PROTOCOL_IPV6_E;

        st = cpssDxChBrgVlanIpMcRouteEnable(dev, vlanId, protocol, enable);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, vlanId, protocol, enable);

        /* Call with protocol [CPSS_IP_PROTOCOL_IPV4V6_E] and enable [GT_TRUE] */
        protocol = CPSS_IP_PROTOCOL_IPV4V6_E;

        st = cpssDxChBrgVlanIpMcRouteEnable(dev, vlanId, protocol, enable);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, vlanId, protocol, enable);


        /*
            1.2. Call with out of range vlandId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)],
                           protocol [CPSS_IP_PROTOCOL_IPV4V6_E]
                           and enable [GT_TRUE].
            Expected: GT_BAD_PARAM.
        */
        vlanId   = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev);
        protocol = CPSS_IP_PROTOCOL_IPV4V6_E;

        st = cpssDxChBrgVlanIpMcRouteEnable(dev, vlanId, protocol, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, vlanId);

        /*
            1.3. Call with vlanId [100], wrong enum values protocol and enable [GT_TRUE].
            Expected: GT_BAD_PARAM.
        */
        vlanId   = BRG_VLAN_TESTED_VLAN_ID_CNS;

        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanIpMcRouteEnable
                                                        (dev, vlanId, protocol, enable),
                                                        protocol);
    }

    vlanId   = BRG_VLAN_TESTED_VLAN_ID_CNS;
    protocol = CPSS_IP_PROTOCOL_IPV4_E;
    enable   = GT_FALSE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanIpMcRouteEnable(dev, vlanId, protocol, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanIpMcRouteEnable(dev, vlanId, protocol, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanNASecurEnable
(
    IN GT_U8           devNum,
    IN GT_U16          vlanId,
    IN GT_BOOL         enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanNASecurEnable)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with vlanId [100]
                   and enable [GT_FALSE and GT_ TRUE].
    Expected: GT_OK.
    1.2. Call with out of range vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)]
                   and enable [GT_FALSE].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st     = GT_OK;

    GT_U8     dev;
    GT_U16    vlanId = 0;
    GT_BOOL   status = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with vlanId [100]
                           and enable [GT_FALSE and GT_ TRUE].
            Expected: GT_OK.
        */

        /* Call with enable [GT_FALSE] */
        vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;
        status = GT_FALSE;

        st = cpssDxChBrgVlanNASecurEnable(dev, vlanId, status);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlanId, status);

        /* Call with enable [GT_TRUE] */
        status = GT_TRUE;

        st = cpssDxChBrgVlanNASecurEnable(dev, vlanId, status);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlanId, status);

        /*
            1.2. Call with out of range vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)]
                           and enable [GT_FALSE].
            Expected: GT_BAD_PARAM.
        */
        vlanId = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev);
        status = GT_FALSE;

        st = cpssDxChBrgVlanNASecurEnable(dev, vlanId, status);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, vlanId, status);
    }

    vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;
    status = GT_FALSE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanNASecurEnable(dev, vlanId, status);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanNASecurEnable(dev, vlanId, status);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanIgmpSnoopingEnable
(
    IN GT_U8   devNum,
    IN GT_U16  vlanId,
    IN GT_BOOL enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanIgmpSnoopingEnable)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with vlanId [100] and enable [GT_FALSE and GT_ TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgVlanIgmpSnoopingEnableGet with the same parameters.
    Expected: GT_OK.
    1.3. Call with out of range vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)]
                   and enable [GT_FALSE].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st     = GT_OK;

    GT_U8     dev;
    GT_U16    vlanId = 0;
    GT_BOOL   status = GT_FALSE;
    GT_BOOL   statusGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with vlanId [100]
                           and enable [GT_FALSE and GT_ TRUE].
            Expected: GT_OK.
        */

        /* Call with enable [GT_FALSE] */
        vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;
        status = GT_FALSE;

        st = cpssDxChBrgVlanIgmpSnoopingEnable(dev, vlanId, status);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlanId, status);

        /*
            1.2. Call cpssDxChBrgVlanIgmpSnoopingEnableGet
                 with the same parameters.
            Expected: GT_OK.
        */
        st = cpssDxChBrgVlanIgmpSnoopingEnableGet(dev, vlanId, &statusGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgVlanIgmpSnoopingEnableGet: %d", dev);
        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(status, statusGet,
                   "get another status than was set: %d", dev);


        /* Call with enable [GT_TRUE] */
        status = GT_TRUE;

        st = cpssDxChBrgVlanIgmpSnoopingEnable(dev, vlanId, status);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlanId, status);

        /*
            1.2. Call cpssDxChBrgVlanIgmpSnoopingEnableGet
                 with the same parameters.
            Expected: GT_OK.
        */
        st = cpssDxChBrgVlanIgmpSnoopingEnableGet(dev, vlanId, &statusGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgVlanIgmpSnoopingEnableGet: %d", dev);
        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(status, statusGet,
                   "get another status than was set: %d", dev);

        /*
            1.3. Call with out of range vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)]
                           and enable [GT_FALSE].
            Expected: GT_BAD_PARAM.
        */
        vlanId = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev);
        status = GT_FALSE;

        st = cpssDxChBrgVlanIgmpSnoopingEnable(dev, vlanId, status);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, vlanId, status);
    }

    vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;
    status = GT_FALSE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanIgmpSnoopingEnable(dev, vlanId, status);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanIgmpSnoopingEnable(dev, vlanId, status);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanIgmpSnoopingEnableGet
(
    IN GT_U8   devNum,
    IN GT_U16  vlanId,
    OUT GT_BOOL *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanIgmpSnoopingEnableGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with vlanId [100] and non-null enablePtr.
    Expected: GT_OK.
    1.2. Call with out of range vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)].
    Expected: GT_BAD_PARAM.
    1.3. Call with wrong enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st     = GT_OK;

    GT_U8     dev;
    GT_U16    vlanId = 0;
    GT_BOOL   statusGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with vlanId [100] and non-null enablePtr.
            Expected: GT_OK.
        */

        vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;

        st = cpssDxChBrgVlanIgmpSnoopingEnableGet(dev, vlanId, &statusGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlanId, statusGet);

        /*
            1.2. Call with out of range vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)].
            Expected: GT_BAD_PARAM.
        */
        vlanId = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev);

        st = cpssDxChBrgVlanIgmpSnoopingEnableGet(dev, vlanId, &statusGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, vlanId, statusGet);

        vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;

        /*
            1.3. Call with wrong enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgVlanIgmpSnoopingEnableGet(dev, vlanId, NULL);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, vlanId, statusGet);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanIgmpSnoopingEnableGet(dev, vlanId, &statusGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanIgmpSnoopingEnableGet(dev, vlanId, &statusGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanIpCntlToCpuSet
(
    IN GT_U8                               devNum,
    IN GT_U16                              vlanId,
    IN CPSS_DXCH_BRG_IP_CTRL_TYPE_ENT      ipCntrlType
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanIpCntlToCpuSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with vlanId [100],
                   ipCntrlType [CPSS_DXCH_BRG_IP_CTRL_NONE_E /
                                CPSS_DXCH_BRG_IP_CTRL_IPV4_IPV6_E].
    Expected: GT_OK.
    1.2. Call with vlanId [100],
                   ipCntrlType [CPSS_DXCH_BRG_IP_CTRL_IPV4_E /
                                CPSS_DXCH_BRG_IP_CTRL_IPV6_E].
    Expected: GT_OK for DxCh2, DxCh3 and NOT GT_OK for others.
    1.3. Call cpssDxChBrgVlanIpCntlToCpuGet with the same
                             parameters as in 1.1 and 1.2.
    Expected: GT_OK and the same value.
    1.4. Call with out of range vlandId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)],
                   and ipCntrlType [CPSS_DXCH_BRG_IP_CTRL_NONE_E].
    Expected: GT_BAD_PARAM.
    1.5. Call with vlanId [100]
                   and out of range ipCntrlType.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS  st    = GT_OK;


    GT_U8                          dev;
    GT_U16                         vlanId      = 0;
    CPSS_DXCH_BRG_IP_CTRL_TYPE_ENT ipCntrlType = CPSS_DXCH_BRG_IP_CTRL_NONE_E;
    CPSS_DXCH_BRG_IP_CTRL_TYPE_ENT ipCntrlTypeGet;
    CPSS_PP_FAMILY_TYPE_ENT        devFamily;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with vlanId [100],
                           ipCntrlType [CPSS_DXCH_BRG_IP_CTRL_NONE_E /
                                        CPSS_DXCH_BRG_IP_CTRL_IPV4_IPV6_E].
            Expected: GT_OK.
        */

        /* Call with ipCntrlType [CPSS_DXCH_BRG_IP_CTRL_NONE_E] */
        vlanId      = BRG_VLAN_TESTED_VLAN_ID_CNS;
        ipCntrlType = CPSS_DXCH_BRG_IP_CTRL_NONE_E;

        st = cpssDxChBrgVlanIpCntlToCpuSet(dev, vlanId, ipCntrlType);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlanId, ipCntrlType);

        /*
            1.3. Call cpssDxChBrgVlanIpCntlToCpuGet with the same
                                     parameters as in 1.1 and 1.2.
            Expected: GT_OK and the same value.
        */

        st = cpssDxChBrgVlanIpCntlToCpuGet(dev, vlanId, &ipCntrlTypeGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgVlanIpCntlToCpuGet: %d", dev);
        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(ipCntrlType, ipCntrlTypeGet,
                   "get another ipCntrlType than was set: %d", dev);


        /* Call with ipCntrlType [CPSS_DXCH_BRG_IP_CTRL_IPV4_IPV6_E] */
        ipCntrlType = CPSS_DXCH_BRG_IP_CTRL_IPV4_IPV6_E;

        st = cpssDxChBrgVlanIpCntlToCpuSet(dev, vlanId, ipCntrlType);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlanId, ipCntrlType);

        /*
            1.3. Call cpssDxChBrgVlanIpCntlToCpuGet with the same
                                     parameters as in 1.1 and 1.2.
            Expected: GT_OK and the same value.
        */

        st = cpssDxChBrgVlanIpCntlToCpuGet(dev, vlanId, &ipCntrlTypeGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgVlanIpCntlToCpuGet: %d", dev);
        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(ipCntrlType, ipCntrlTypeGet,
                   "get another ipCntrlType than was set: %d", dev);


        /* Getting device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /*
            1.2. Call with vlanId [100],
                           ipCntrlType [CPSS_DXCH_BRG_IP_CTRL_IPV4_E /
                                        CPSS_DXCH_BRG_IP_CTRL_IPV6_E].
            Expected: GT_OK for DxCh2, DxCh3 and NOT GT_OK for others.
        */

        /* Call with ipCntrlType [CPSS_DXCH_BRG_IP_CTRL_IPV4_E] */
        ipCntrlType = CPSS_DXCH_BRG_IP_CTRL_IPV4_E;

        st = cpssDxChBrgVlanIpCntlToCpuSet(dev, vlanId, ipCntrlType);
        if (devFamily != CPSS_PP_FAMILY_CHEETAH_E)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, ipCntrlType = %d",
                                         dev, ipCntrlType);
            if(GT_OK == st)
            {
                /*
                    1.3. Call cpssDxChBrgVlanIpCntlToCpuGet with the same
                                             parameters as in 1.1 and 1.2.
                    Expected: GT_OK and the same value.
                */

                st = cpssDxChBrgVlanIpCntlToCpuGet(dev, vlanId, &ipCntrlTypeGet);

                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                           "cpssDxChBrgVlanIpCntlToCpuGet: %d", dev);
                /* Verifying values */
                UTF_VERIFY_EQUAL1_STRING_MAC(ipCntrlType, ipCntrlTypeGet,
                           "get another ipCntrlType than was set: %d", dev);
            }
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, ipCntrlType = %d",
                                             dev, ipCntrlType);
        }

        /* Call with ipCntrlType [CPSS_DXCH_BRG_IP_CTRL_IPV6_E] */
        ipCntrlType = CPSS_DXCH_BRG_IP_CTRL_IPV6_E;

        st = cpssDxChBrgVlanIpCntlToCpuSet(dev, vlanId, ipCntrlType);
        if (devFamily != CPSS_PP_FAMILY_CHEETAH_E)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, ipCntrlType = %d",
                                         dev, ipCntrlType);
            if(GT_OK == st)
            {
                /*
                    1.3. Call cpssDxChBrgVlanIpCntlToCpuGet with the same
                                             parameters as in 1.1 and 1.2.
                    Expected: GT_OK and the same value.
                */

                st = cpssDxChBrgVlanIpCntlToCpuGet(dev, vlanId, &ipCntrlTypeGet);

                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                           "cpssDxChBrgVlanIpCntlToCpuGet: %d", dev);
                /* Verifying values */
                UTF_VERIFY_EQUAL1_STRING_MAC(ipCntrlType, ipCntrlTypeGet,
                           "get another ipCntrlType than was set: %d", dev);
            }
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, ipCntrlType = %d",
                                             dev, ipCntrlType);
        }

        /*
            1.4. Call with out of range vlandId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)],
                           and ipCntrlType [CPSS_DXCH_BRG_IP_CTRL_NONE_E].
            Expected: GT_BAD_PARAM.
        */
        vlanId      = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev);
        ipCntrlType = CPSS_DXCH_BRG_IP_CTRL_NONE_E;

        st = cpssDxChBrgVlanIpCntlToCpuSet(dev, vlanId, ipCntrlType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, vlanId);

        /*
            1.5. Call with vlanId [100] and wrong enum values ipCntrlType.
            Expected: GT_BAD_PARAM.
        */
        vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;

        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanIpCntlToCpuSet
                                                        (dev, vlanId, ipCntrlType),
                                                        ipCntrlType);
    }

    vlanId      = BRG_VLAN_TESTED_VLAN_ID_CNS;
    ipCntrlType = CPSS_DXCH_BRG_IP_CTRL_NONE_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanIpCntlToCpuSet(dev, vlanId, ipCntrlType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanIpCntlToCpuSet(dev, vlanId, ipCntrlType);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanIpCntlToCpuGet
(
    IN GT_U8   devNum,
    IN GT_U16  vlanId,
    OUT CPSS_DXCH_BRG_IP_CTRL_TYPE_ENT      *ipCntrlTypePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanIpCntlToCpuGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with vlanId [100] and non-null ipCntrlTypePtr.
    Expected: GT_OK.
    1.2. Call with out of range vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)].
    Expected: GT_BAD_PARAM.
    1.3. Call with wrong ipCntrlTypePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st     = GT_OK;

    GT_U8     dev;
    GT_U16    vlanId = 0;
    CPSS_DXCH_BRG_IP_CTRL_TYPE_ENT ipCntrlTypeGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with vlanId [100] and non-null ipCntrlTypePtr.
            Expected: GT_OK.
        */

        vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;

        st = cpssDxChBrgVlanIpCntlToCpuGet(dev, vlanId, &ipCntrlTypeGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlanId, ipCntrlTypeGet);

        /*
            1.2. Call with out of range vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)].
            Expected: GT_BAD_PARAM.
        */
        vlanId = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev);

        st = cpssDxChBrgVlanIpCntlToCpuGet(dev, vlanId, &ipCntrlTypeGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, vlanId, ipCntrlTypeGet);

        vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;

        /*
            1.3. Call with wrong ipCntrlTypePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgVlanIpCntlToCpuGet(dev, vlanId, NULL);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, vlanId, ipCntrlTypeGet);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanIpCntlToCpuGet(dev, vlanId, &ipCntrlTypeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanIpCntlToCpuGet(dev, vlanId, &ipCntrlTypeGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanIpV6IcmpToCpuEnable
(
    IN GT_U8      devNum,
    IN GT_U16     vlanId,
    IN GT_BOOL    enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanIpV6IcmpToCpuEnable)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with vlanId [100] and enable [GT_FALSE and GT_ TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgVlanIpV6IcmpToCpuEnableGet with the same parameters.
    Expected: GT_OK.
    1.3. Call with out of range vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)]
                   and enable [GT_FALSE].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st     = GT_OK;

    GT_U8     dev;
    GT_U16    vlanId = 0;
    GT_BOOL   status = GT_FALSE;
    GT_BOOL   statusGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with vlanId [100] and enable [GT_FALSE and GT_ TRUE].
            Expected: GT_OK.
        */

        /* Call with enable [GT_FALSE] */
        vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;
        status = GT_FALSE;

        st = cpssDxChBrgVlanIpV6IcmpToCpuEnable(dev, vlanId, status);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlanId, status);

        /*
            1.2. Call cpssDxChBrgVlanIpV6IcmpToCpuEnableGet
                 with the same parameters.
            Expected: GT_OK.
        */
        st = cpssDxChBrgVlanIpV6IcmpToCpuEnableGet(dev, vlanId, &statusGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgVlanIpV6IcmpToCpuEnableGet: %d", dev);
        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(status, statusGet,
                   "get another status than was set: %d", dev);

        /* Call with enable [GT_TRUE] */
        status = GT_TRUE;

        st = cpssDxChBrgVlanIpV6IcmpToCpuEnable(dev, vlanId, status);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlanId, status);

        /*
            1.2. Call cpssDxChBrgVlanIpV6IcmpToCpuEnableGet
                 with the same parameters.
            Expected: GT_OK.
        */
        st = cpssDxChBrgVlanIpV6IcmpToCpuEnableGet(dev, vlanId, &statusGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgVlanIpV6IcmpToCpuEnableGet: %d", dev);
        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(status, statusGet,
                   "get another status than was set: %d", dev);

        /*
            1.3. Call with out of range vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)]
                           and enable [GT_FALSE].
            Expected: GT_BAD_PARAM.
        */
        vlanId = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev);
        status = GT_FALSE;

        st = cpssDxChBrgVlanIpV6IcmpToCpuEnable(dev, vlanId, status);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, vlanId, status);
    }

    vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;
    status = GT_FALSE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanIpV6IcmpToCpuEnable(dev, vlanId, status);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanIpV6IcmpToCpuEnable(dev, vlanId, status);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanIpV6IcmpToCpuEnableGet
(
    IN GT_U8   devNum,
    IN GT_U16  vlanId,
    OUT GT_BOOL *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanIpV6IcmpToCpuEnableGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with vlanId [100] and non-null enablePtr.
    Expected: GT_OK.
    1.2. Call with out of range vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)].
    Expected: GT_BAD_PARAM.
    1.3. Call with wrong enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st     = GT_OK;

    GT_U8     dev;
    GT_U16    vlanId = 0;
    GT_BOOL   statusGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with vlanId [100] and non-null enablePtr.
            Expected: GT_OK.
        */

        vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;

        st = cpssDxChBrgVlanIpV6IcmpToCpuEnableGet(dev, vlanId, &statusGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlanId, statusGet);

        /*
            1.2. Call with out of range vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)].
            Expected: GT_BAD_PARAM.
        */
        vlanId = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev);

        st = cpssDxChBrgVlanIpV6IcmpToCpuEnableGet(dev, vlanId, &statusGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, vlanId, statusGet);

        vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;

        /*
            1.3. Call with wrong enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgVlanIpV6IcmpToCpuEnableGet(dev, vlanId, NULL);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, vlanId, statusGet);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanIpV6IcmpToCpuEnableGet(dev, vlanId, &statusGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanIpV6IcmpToCpuEnableGet(dev, vlanId, &statusGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanUdpBcPktsToCpuEnable
(
    IN  GT_U8       devNum,
    IN  GT_U16      vlanId,
    IN  GT_BOOL     enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanUdpBcPktsToCpuEnable)
{
/*
    ITERATE_DEVICES (DxCh2 and above)
    1.1. Call with vlanId [100] and enable [GT_FALSE and GT_ TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgVlanUdpBcPktsToCpuEnableGet with the same parameters.
    Expected: GT_OK and the same value.
    1.3. Call with out of range vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)]
                   and enable [GT_FALSE].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st     = GT_OK;

    GT_U8     dev;
    GT_U16    vlanId = 0;
    GT_BOOL   status = GT_FALSE;
    GT_BOOL   statusGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with vlanId [100] and enable [GT_FALSE and GT_ TRUE].
            Expected: GT_OK.
        */

        /* Call with enable [GT_FALSE] */
        vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;
        status = GT_FALSE;

        st = cpssDxChBrgVlanUdpBcPktsToCpuEnable(dev, vlanId, status);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlanId, status);

        /*
            1.2. Call cpssDxChBrgVlanUdpBcPktsToCpuEnableGet
                 with the same parameters.
            Expected: GT_OK.
        */
        st = cpssDxChBrgVlanUdpBcPktsToCpuEnableGet(dev, vlanId, &statusGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgVlanUdpBcPktsToCpuEnableGet: %d", dev);
        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(status, statusGet,
                   "get another status than was set: %d", dev);


        /* Call with enable [GT_TRUE] */
        status = GT_TRUE;

        st = cpssDxChBrgVlanUdpBcPktsToCpuEnable(dev, vlanId, status);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlanId, status);

        /*
            1.2. Call cpssDxChBrgVlanUdpBcPktsToCpuEnableGet
                 with the same parameters.
            Expected: GT_OK.
        */
        st = cpssDxChBrgVlanUdpBcPktsToCpuEnableGet(dev, vlanId, &statusGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgVlanUdpBcPktsToCpuEnableGet: %d", dev);
        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(status, statusGet,
                   "get another status than was set: %d", dev);

        /*
            1.3. Call with out of range vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)]
                           and enable [GT_FALSE].
            Expected: GT_BAD_PARAM.
        */
        vlanId = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev);
        status = GT_FALSE;

        st = cpssDxChBrgVlanUdpBcPktsToCpuEnable(dev, vlanId, status);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, vlanId, status);
    }

    vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;
    status = GT_FALSE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanUdpBcPktsToCpuEnable(dev, vlanId, status);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanUdpBcPktsToCpuEnable(dev, vlanId, status);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanUdpBcPktsToCpuEnableGet
(
    IN GT_U8   devNum,
    IN GT_U16  vlanId,
    OUT GT_BOOL *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanUdpBcPktsToCpuEnableGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with vlanId [100] and non-null enablePtr.
    Expected: GT_OK.
    1.2. Call with out of range vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)].
    Expected: GT_BAD_PARAM.
    1.3. Call with wrong enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st     = GT_OK;

    GT_U8     dev;
    GT_U16    vlanId = 0;
    GT_BOOL   statusGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with vlanId [100] and non-null enablePtr.
            Expected: GT_OK.
        */

        vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;

        st = cpssDxChBrgVlanUdpBcPktsToCpuEnableGet(dev, vlanId, &statusGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlanId, statusGet);

        /*
            1.2. Call with out of range vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)].
            Expected: GT_BAD_PARAM.
        */
        vlanId = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev);

        st = cpssDxChBrgVlanUdpBcPktsToCpuEnableGet(dev, vlanId, &statusGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, vlanId, statusGet);

        vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;

        /*
            1.3. Call with wrong enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgVlanUdpBcPktsToCpuEnableGet(dev, vlanId, NULL);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, vlanId, statusGet);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanUdpBcPktsToCpuEnableGet(dev, vlanId, &statusGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanUdpBcPktsToCpuEnableGet(dev, vlanId, &statusGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanIpv6SourceSiteIdSet
(
    IN GT_U8                                devNum,
    IN GT_U16                               vlanId,
    IN CPSS_IP_SITE_ID_ENT                  siteId
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanIpv6SourceSiteIdSet)
{
/*
    ITERATE_DEVICES (DxCh2 and above)
    1.1. Call with vlanId [100],
                   siteId [CPSS_IP_SITE_ID_INTERNAL_E /
                           CPSS_IP_SITE_ID_EXTERNAL_E].
    Expected: GT_OK.
    1.2. Call with out of range vlandId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)],
                   siteId [CPSS_IP_SITE_ID_INTERNAL_E].
    Expected: GT_BAD_PARAM.
    1.3. Call with vlanId [100]
                   and out of range siteId.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS  st    = GT_OK;


    GT_U8               dev;
    GT_U16              vlanId = 0;
    CPSS_IP_SITE_ID_ENT siteId = CPSS_IP_SITE_ID_INTERNAL_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with vlanId [100],
                           siteId [CPSS_IP_SITE_ID_INTERNAL_E /
                                   CPSS_IP_SITE_ID_EXTERNAL_E].
            Expected: GT_OK.
        */

        /* Call with siteId [CPSS_IP_SITE_ID_INTERNAL_E] */
        vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;
        siteId = CPSS_IP_SITE_ID_INTERNAL_E;

        st = cpssDxChBrgVlanIpv6SourceSiteIdSet(dev, vlanId, siteId);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlanId, siteId);

        /* Call with siteId [CPSS_IP_SITE_ID_EXTERNAL_E] */
        siteId = CPSS_IP_SITE_ID_EXTERNAL_E;

        st = cpssDxChBrgVlanIpv6SourceSiteIdSet(dev, vlanId, siteId);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlanId, siteId);

        /*
            1.2. Call with out of range vlandId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)],
                           siteId [CPSS_IP_SITE_ID_INTERNAL_E].
            Expected: GT_BAD_PARAM.
        */
        vlanId = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev);
        siteId = CPSS_IP_SITE_ID_INTERNAL_E;

        st = cpssDxChBrgVlanIpv6SourceSiteIdSet(dev, vlanId, siteId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, vlanId);

        /*
            1.3. Call with vlanId [100] and wrong enum values siteId.
            Expected: GT_BAD_PARAM.
        */
        vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;

        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanIpv6SourceSiteIdSet
                                                        (dev, vlanId, siteId),
                                                        siteId);
    }

    vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;
    siteId = CPSS_IP_SITE_ID_INTERNAL_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanIpv6SourceSiteIdSet(dev, vlanId, siteId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanIpv6SourceSiteIdSet(dev, vlanId, siteId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanIpmBridgingEnable
(
    IN  GT_U8                       devNum,
    IN  GT_U16                      vlanId,
    IN  CPSS_IP_PROTOCOL_STACK_ENT  ipVer,
    IN  GT_BOOL                     enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanIpmBridgingEnable)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with vlanId [100],
                   ipVer [CPSS_IP_PROTOCOL_IPV4_E /
                          CPSS_IP_PROTOCOL_IPV6_E /
                          CPSS_IP_PROTOCOL_IPV4V6_E]
                   and enable [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.2. Call with out of range vlandId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)],
                   ipVer [CPSS_IP_PROTOCOL_IPV4V6_E]
                   and enable [GT_TRUE].
    Expected: GT_BAD_PARAM.
    1.3. Call with vlanId [100],
                   out of range ipVer
                   and enable [GT_TRUE].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st    = GT_OK;


    GT_U8                      dev;
    GT_U16                     vlanId = 0;
    CPSS_IP_PROTOCOL_STACK_ENT ipVer  = CPSS_IP_PROTOCOL_IPV4_E;
    GT_BOOL                    enable = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with vlanId [100],
                           ipVer [CPSS_IP_PROTOCOL_IPV4_E /
                                  CPSS_IP_PROTOCOL_IPV6_E /
                                  CPSS_IP_PROTOCOL_IPV4V6_E]
                           and enable [GT_FALSE and GT_TRUE].
            Expected: GT_OK.
        */

        /* Call with ipVer [CPSS_IP_PROTOCOL_IPV4_E] and enable [GT_FALSE] */
        vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;
        ipVer  = CPSS_IP_PROTOCOL_IPV4_E;
        enable = GT_FALSE;

        st = cpssDxChBrgVlanIpmBridgingEnable(dev, vlanId, ipVer, enable);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, vlanId, ipVer, enable);

        /* Call with ipVer [CPSS_IP_PROTOCOL_IPV6_E] and enable [GT_FALSE] */
        ipVer  = CPSS_IP_PROTOCOL_IPV6_E;

        st = cpssDxChBrgVlanIpmBridgingEnable(dev, vlanId, ipVer, enable);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, vlanId, ipVer, enable);

        /* Call with ipVer [CPSS_IP_PROTOCOL_IPV4V6_E] and enable [GT_FALSE] */
        ipVer  = CPSS_IP_PROTOCOL_IPV4V6_E;

        st = cpssDxChBrgVlanIpmBridgingEnable(dev, vlanId, ipVer, enable);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, vlanId, ipVer, enable);

        /* Call with ipVer [CPSS_IP_PROTOCOL_IPV4_E] and enable [GT_TRUE] */
        ipVer  = CPSS_IP_PROTOCOL_IPV4_E;
        enable = GT_TRUE;

        st = cpssDxChBrgVlanIpmBridgingEnable(dev, vlanId, ipVer, enable);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, vlanId, ipVer, enable);

        /* Call with ipVer [CPSS_IP_PROTOCOL_IPV6_E] and enable [GT_TRUE] */
        ipVer  = CPSS_IP_PROTOCOL_IPV6_E;

        st = cpssDxChBrgVlanIpmBridgingEnable(dev, vlanId, ipVer, enable);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, vlanId, ipVer, enable);

        /* Call with ipVer [CPSS_IP_PROTOCOL_IPV4V6_E] and enable [GT_TRUE] */
        ipVer  = CPSS_IP_PROTOCOL_IPV4V6_E;

        st = cpssDxChBrgVlanIpmBridgingEnable(dev, vlanId, ipVer, enable);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, vlanId, ipVer, enable);

        /*
            1.2. Call with out of range vlandId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)],
                           ipVer [CPSS_IP_PROTOCOL_IPV4V6_E]
                           and enable [GT_TRUE].
            Expected: GT_BAD_PARAM.
        */
        vlanId = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev);

        st = cpssDxChBrgVlanIpmBridgingEnable(dev, vlanId, ipVer, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, vlanId);

        /*
            1.3. Call with vlanId [100], wrong enum values ipVer and enable [GT_TRUE].
            Expected: GT_BAD_PARAM.
        */
        vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;

        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanIpmBridgingEnable
                                                        (dev, vlanId, ipVer, enable),
                                                        ipVer);
    }

    vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;
    ipVer  = CPSS_IP_PROTOCOL_IPV4_E;
    enable = GT_FALSE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanIpmBridgingEnable(dev, vlanId, ipVer, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanIpmBridgingEnable(dev, vlanId, ipVer, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanIpmBridgingModeSet
(
    IN  GT_U8                       devNum,
    IN  GT_U16                      vlanId,
    IN  CPSS_IP_PROTOCOL_STACK_ENT  ipVer,
    IN  CPSS_BRG_IPM_MODE_ENT       ipmMode
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanIpmBridgingModeSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with vlanId [100],
                   ipVer [CPSS_IP_PROTOCOL_IPV4_E /
                          CPSS_IP_PROTOCOL_IPV6_E /
                          CPSS_IP_PROTOCOL_IPV4V6_E]
                   and ipmMode [CPSS_BRG_IPM_SGV_E /
                                CPSS_BRG_IPM_GV_E].
    Expected: GT_OK.
    1.2. Call with out of range vlandId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)],
                   ipVer [CPSS_IP_PROTOCOL_IPV4V6_E]
                   and ipmMode [CPSS_BRG_IPM_SGV_E].
    Expected: GT_BAD_PARAM.
    1.3. Call with vlanId [100],
                   out of range ipVer
                   and ipmMode [CPSS_BRG_IPM_GV_E].
    Expected: GT_BAD_PARAM.
    1.4. Call with vlanId [100],
                   ipVer [CPSS_IP_PROTOCOL_IPV6_E]
                   and out of range ipmMode.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS  st    = GT_OK;


    GT_U8                      dev;
    GT_U16                     vlanId  = 0;
    CPSS_IP_PROTOCOL_STACK_ENT ipVer   = CPSS_IP_PROTOCOL_IPV4_E;
    CPSS_BRG_IPM_MODE_ENT      ipmMode = CPSS_BRG_IPM_SGV_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with vlanId [100],
                           ipVer [CPSS_IP_PROTOCOL_IPV4_E /
                                  CPSS_IP_PROTOCOL_IPV6_E /
                                  CPSS_IP_PROTOCOL_IPV4V6_E]
                           and ipmMode [CPSS_BRG_IPM_SGV_E /
                                        CPSS_BRG_IPM_GV_E].
            Expected: GT_OK.
        */
        /* Call with ipVer [CPSS_IP_PROTOCOL_IPV4_E] and ipmMode [CPSS_BRG_IPM_SGV_E] */
        vlanId  = BRG_VLAN_TESTED_VLAN_ID_CNS;
        ipVer   = CPSS_IP_PROTOCOL_IPV4_E;
        ipmMode = CPSS_BRG_IPM_SGV_E;

        st = cpssDxChBrgVlanIpmBridgingModeSet(dev, vlanId, ipVer, ipmMode);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, vlanId, ipVer, ipmMode);

        /* Call with ipVer [CPSS_IP_PROTOCOL_IPV6_E] and ipmMode [CPSS_BRG_IPM_SGV_E] */
        ipVer = CPSS_IP_PROTOCOL_IPV6_E;

        st = cpssDxChBrgVlanIpmBridgingModeSet(dev, vlanId, ipVer, ipmMode);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, vlanId, ipVer, ipmMode);

        /* Call with ipVer [CPSS_IP_PROTOCOL_IPV4V6_E] and ipmMode [CPSS_BRG_IPM_SGV_E] */
        ipVer = CPSS_IP_PROTOCOL_IPV4V6_E;

        st = cpssDxChBrgVlanIpmBridgingModeSet(dev, vlanId, ipVer, ipmMode);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, vlanId, ipVer, ipmMode);

        /* Call with ipVer [CPSS_IP_PROTOCOL_IPV4_E] and ipmMode [CPSS_BRG_IPM_GV_E] */
        ipVer   = CPSS_IP_PROTOCOL_IPV4_E;
        ipmMode = CPSS_BRG_IPM_GV_E;

        st = cpssDxChBrgVlanIpmBridgingModeSet(dev, vlanId, ipVer, ipmMode);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, vlanId, ipVer, ipmMode);

        /* Call with ipVer [CPSS_IP_PROTOCOL_IPV6_E] and ipmMode [CPSS_BRG_IPM_GV_E] */
        ipVer = CPSS_IP_PROTOCOL_IPV6_E;

        st = cpssDxChBrgVlanIpmBridgingModeSet(dev, vlanId, ipVer, ipmMode);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, vlanId, ipVer, ipmMode);

        /* Call with ipVer [CPSS_IP_PROTOCOL_IPV4V6_E] and ipmMode [CPSS_BRG_IPM_GV_E] */
        ipVer = CPSS_IP_PROTOCOL_IPV4V6_E;

        st = cpssDxChBrgVlanIpmBridgingModeSet(dev, vlanId, ipVer, ipmMode);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, vlanId, ipVer, ipmMode);

        /*
            1.2. Call with out of range vlandId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)],
                           ipVer [CPSS_IP_PROTOCOL_IPV4V6_E]
                           and ipmMode [CPSS_BRG_IPM_SGV_E].
            Expected: GT_BAD_PARAM.
        */
        vlanId  = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev);
        ipmMode = CPSS_BRG_IPM_SGV_E;

        st = cpssDxChBrgVlanIpmBridgingModeSet(dev, vlanId, ipVer, ipmMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, vlanId);

        /*
            1.3. Call with vlanId [100], wrong enum values ipVer and ipmMode [CPSS_BRG_IPM_GV_E].
            Expected: GT_BAD_PARAM.
        */
        vlanId  = BRG_VLAN_TESTED_VLAN_ID_CNS;
        ipmMode = CPSS_BRG_IPM_GV_E;

        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanIpmBridgingModeSet
                                                        (dev, vlanId, ipVer, ipmMode),
                                                        ipVer);

        ipVer   = CPSS_IP_PROTOCOL_IPV6_E;

        /*
            1.4. Call with vlanId [100], ipVer [CPSS_IP_PROTOCOL_IPV6_E]
                 and wrong enum values ipmMode.
            Expected: GT_BAD_PARAM.
        */
        vlanId  = BRG_VLAN_TESTED_VLAN_ID_CNS;
        ipVer   = CPSS_IP_PROTOCOL_IPV6_E;

        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanIpmBridgingModeSet
                                                        (dev, vlanId, ipVer, ipmMode),
                                                        ipmMode);
    }

    vlanId  = BRG_VLAN_TESTED_VLAN_ID_CNS;
    ipVer   = CPSS_IP_PROTOCOL_IPV4_E;
    ipmMode = CPSS_BRG_IPM_SGV_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanIpmBridgingModeSet(dev, vlanId, ipVer, ipmMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanIpmBridgingModeSet(dev, vlanId, ipVer, ipmMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanIngressMirrorEnable
(
    IN GT_U8   devNum,
    IN GT_U16  vlanId,
    IN GT_BOOL enable,
    IN GT_U32  index
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanIngressMirrorEnable)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with vlanId [100],
                   enable[GT_FALSE, GT_ TRUE],
                   index[0](applicable only for Bobcat2; Caelum; Bobcat3).
    Expected: GT_OK.
    1.2. Call with out of range vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)]
        and other valid parameters same as 1.1.
    Expected: GT_BAD_PARAM.
    1.3. For Bobcat2; Caelum; Bobcat3 call with index[6] and other parameters same as 1.1.
    Expected: GT_OK.
    1.4. For Bobcat2; Caelum; Bobcat3 call with out of range index[7] and other valid
        parameters same as 1.1.
    Expected: GT_BAD_PARAM.
*/

    GT_STATUS   st      = GT_OK;
    GT_U8       dev     = 0;
    GT_U16      vlanId  = 0;
    GT_BOOL     status  = GT_FALSE;
    GT_U32      index   = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /*
            1.1. Call with vlanId [100], enable[GT_FALSE, GT_ TRUE],
                index[0](applicable only for Bobcat2; Caelum; Bobcat3).
            Expected: GT_OK.
        */

        /* Call with enable [GT_FALSE] */
        vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;
        index = 0;
        status = GT_FALSE;

        st = cpssDxChBrgVlanIngressMirrorEnable(dev, vlanId, status, index);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlanId, status);

        /* Call with enable [GT_TRUE] */
        status = GT_TRUE;

        st = cpssDxChBrgVlanIngressMirrorEnable(dev, vlanId, status, index);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlanId, status);

        /*
            1.2. Call with out of range
                vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)] and
                other valid params same as 1.1.
            Expected: GT_BAD_PARAM.
        */
        vlanId = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev);

        st = cpssDxChBrgVlanIngressMirrorEnable(dev, vlanId, status, index);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, vlanId, status);

        /* restore previous value */
        vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;

        /*
            1.3. For Bobcat2; Caelum; Bobcat3 call with index[6] and other parameters same as 1.1.
            Expected: GT_OK.
        */
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            index = 6;

            st = cpssDxChBrgVlanIngressMirrorEnable(dev, vlanId, status, index);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlanId, status);

            index = 0;
        }

        /*
            1.4. For Bobcat2; Caelum; Bobcat3 call with out of range index[7] and other valid
                parameters same as 1.1.
            Expected: GT_BAD_PARAM.
        */
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            index = 7;

            st = cpssDxChBrgVlanIngressMirrorEnable(dev, vlanId, status, index);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, vlanId, status);

            index = 0;
        }
    }

    vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;
    status = GT_FALSE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanIngressMirrorEnable(dev, vlanId, status,0);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanIngressMirrorEnable(dev, vlanId, status,0);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortAccFrameTypeSet
(
    IN  GT_U8                               devNum,
    IN  GT_U8                               portNum,
    IN  CPSS_PORT_ACCEPT_FRAME_TYPE_ENT     frameType
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortAccFrameTypeSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxCh2 and above)
    1.1.1. Call with frameType [CPSS_PORT_ACCEPT_FRAME_TAGGED_E /
                                CPSS_PORT_ACCEPT_FRAME_ALL_E /
                                CPSS_PORT_ACCEPT_FRAME_UNTAGGED_E].
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgVlanPortAccFrameTypeGet with the same params.
    Expected: GT_OK and the same value.
    1.1.3. Call with frameType [CPSS_PORT_ACCEPT_FRAME_NONE_E].
    Expected: non GT_OK.
    1.1.4. Call with out of range frameType.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS  st    = GT_OK;


    GT_U8                           dev;
    GT_PORT_NUM                     port      = BRG_VLAN_VALID_PHY_PORT_CNS;
    CPSS_PORT_ACCEPT_FRAME_TYPE_ENT frameType = CPSS_PORT_ACCEPT_FRAME_TAGGED_E;
    CPSS_PORT_ACCEPT_FRAME_TYPE_ENT frameTypeGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with frameType [CPSS_PORT_ACCEPT_FRAME_TAGGED_E /
                                            CPSS_PORT_ACCEPT_FRAME_ALL_E /
                                            CPSS_PORT_ACCEPT_FRAME_UNTAGGED_E].
                Expected: GT_OK.
            */

            /* Call with frameType [CPSS_PORT_ACCEPT_FRAME_TAGGED_E] */
            frameType = CPSS_PORT_ACCEPT_FRAME_TAGGED_E;

            st = cpssDxChBrgVlanPortAccFrameTypeSet(dev, port, frameType);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, frameType);

            /*
                1.1.2. Call cpssDxChBrgVlanPortAccFrameTypeGet with the same params.
                Expected: GT_OK and the same value.
            */

            st = cpssDxChBrgVlanPortAccFrameTypeGet(dev, port, &frameTypeGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, frameType);

            UTF_VERIFY_EQUAL2_STRING_MAC(frameType, frameTypeGet,
                       "get another frameType than was set: %d, %d", dev, port);

            /* Call with frameType [CPSS_PORT_ACCEPT_FRAME_ALL_E] */
            frameType = CPSS_PORT_ACCEPT_FRAME_ALL_E;

            st = cpssDxChBrgVlanPortAccFrameTypeSet(dev, port, frameType);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, frameType);

            /*
                1.1.2. Call cpssDxChBrgVlanPortAccFrameTypeGet with the same params.
                Expected: GT_OK and the same value.
            */

            st = cpssDxChBrgVlanPortAccFrameTypeGet(dev, port, &frameTypeGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, frameType);

            UTF_VERIFY_EQUAL2_STRING_MAC(frameType, frameTypeGet,
                       "get another frameType than was set: %d, %d", dev, port);

            /* Call with frameType [CPSS_PORT_ACCEPT_FRAME_UNTAGGED_E] */
            frameType = CPSS_PORT_ACCEPT_FRAME_UNTAGGED_E;

            st = cpssDxChBrgVlanPortAccFrameTypeSet(dev, port, frameType);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, frameType);

            /*
                1.1.2. Call cpssDxChBrgVlanPortAccFrameTypeGet with the same params.
                Expected: GT_OK and the same value.
            */

            st = cpssDxChBrgVlanPortAccFrameTypeGet(dev, port, &frameTypeGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, frameType);

            UTF_VERIFY_EQUAL2_STRING_MAC(frameType, frameTypeGet,
                       "get another frameType than was set: %d, %d", dev, port);

            /*
                1.1.3. Call with frameType [CPSS_PORT_ACCEPT_FRAME_NONE_E].
                Expected: non GT_OK.
            */

            frameType = CPSS_PORT_ACCEPT_FRAME_NONE_E;

            st = cpssDxChBrgVlanPortAccFrameTypeSet(dev, port, frameType);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, frameType);

            /*
                1.1.4. Call with wrong enum values frameType.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanPortAccFrameTypeSet
                                                        (dev, port, frameType),
                                                        frameType);
        }
        frameType = CPSS_PORT_ACCEPT_FRAME_TAGGED_E;

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanPortAccFrameTypeSet(dev, port, frameType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanPortAccFrameTypeSet(dev, port, frameType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanPortAccFrameTypeSet(dev, port, frameType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    frameType = CPSS_PORT_ACCEPT_FRAME_TAGGED_E;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = BRG_VLAN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortAccFrameTypeSet(dev, port, frameType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortAccFrameTypeSet(dev, port, frameType);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortAccFrameTypeGet
(
    IN  GT_U8                               devNum,
    IN  GT_U8                               portNum,
    OUT CPSS_PORT_ACCEPT_FRAME_TYPE_ENT     *frameTypePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortAccFrameTypeGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxCh2 and above)
    1.1.1. Call with not NULL frameTypePtr.
    Expected: GT_OK.
    1.1.2. Call with null frameTypePtr [NULL].
    Expected: GT_BAD_PTR
*/
    GT_STATUS                       st        = GT_OK;

    GT_U8                           dev;
    GT_PORT_NUM                     port      = BRG_VLAN_VALID_PHY_PORT_CNS;
    CPSS_PORT_ACCEPT_FRAME_TYPE_ENT frameTypeGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with not NULL frameTypePtr.
                Expected: GT_OK.
            */
            st = cpssDxChBrgVlanPortAccFrameTypeGet(dev, port, &frameTypeGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call with null frameTypePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgVlanPortAccFrameTypeGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanPortAccFrameTypeGet(dev, port, &frameTypeGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanPortAccFrameTypeGet(dev, port, &frameTypeGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanPortAccFrameTypeGet(dev, port, &frameTypeGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = BRG_VLAN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortAccFrameTypeGet(dev, port, &frameTypeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortAccFrameTypeGet(dev, port, &frameTypeGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanMruProfileIdxSet
(
    IN GT_U8     devNum,
    IN GT_U16    vlanId,
    IN GT_U32    mruIndex
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanMruProfileIdxSet)
{
/*
    ITERATE_DEVICES (DxCh2 and above)
    1.1. Call with vlanId [100], mruIndex [0].
    Expected: GT_OK.
    1.2. Call with out of range vlandId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)], mruIndex [1].
    Expected: GT_BAD_PARAM.
    1.3. Call with vlanId [100] and mruIndex [PRV_CPSS_DXCH_BRG_MRU_INDEX_MAX_CNS + 1].
    Expected: non GT_OK.
*/
    GT_STATUS st       = GT_OK;

    GT_U8     dev;
    GT_U16    vlanId   = 0;
    GT_U32    mruIndex = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with vlanId [100], mruIndex [0].
            Expected: GT_OK.
        */
        vlanId   = BRG_VLAN_TESTED_VLAN_ID_CNS;
        mruIndex = 0;

        st = cpssDxChBrgVlanMruProfileIdxSet(dev, vlanId, mruIndex);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlanId, mruIndex);

        /*
            1.2. Call with out of range vlandId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)],
                           mruIndex [1].
            Expected: GT_BAD_PARAM.
        */
        vlanId = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev);
        mruIndex = 1;

        st = cpssDxChBrgVlanMruProfileIdxSet(dev, vlanId, mruIndex);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, vlanId);

        /*
            1.3. Call with vlanId [100]
                           and mruIndex [PRV_CPSS_DXCH_BRG_MRU_INDEX_MAX_CNS + 1].
            Expected: non GT_OK.
        */
        vlanId   = BRG_VLAN_TESTED_VLAN_ID_CNS;
        mruIndex = 8;

        st = cpssDxChBrgVlanMruProfileIdxSet(dev, vlanId, mruIndex);
        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlanId, mruIndex);
    }

    vlanId   = BRG_VLAN_TESTED_VLAN_ID_CNS;
    mruIndex = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanMruProfileIdxSet(dev, vlanId, mruIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanMruProfileIdxSet(dev, vlanId, mruIndex);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanMruProfileValueSet
(
    IN GT_U8     devNum,
    IN GT_U32    mruIndex,
    IN GT_U32    mruValue
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanMruProfileValueSet)
{
/*
ITERATE_DEVICES (DxCh2 and above)
    1.1. Call with mruIndex [0] and mruValue[1 / 0xFFFF].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgVlanMruProfileValueGet
    Expected: GT_OK and same mruValue.
    1.3. Call with mruIndex [PRV_CPSS_DXCH_BRG_MRU_INDEX_MAX_CNS + 1], mruValue [16].
    Expected: non GT_OK.
    1.4. Call with mruIndex [2] and mruValue [0x10000].
    Expected: non GT_OK.
*/
    GT_STATUS st       = GT_OK;

    GT_U8     dev;
    GT_U32    mruIndex = 0;
    GT_U32    mruValue = 0;
    GT_U32    mruValueGet = 0;
    GT_U32    mruMaxValue;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with mruIndex [0] and mruValue[1 / 0xFFFF].
            Expected: GT_OK.
        */
        mruIndex = 0;
        mruValue = 1;
        mruMaxValue = (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev)) ? 0x3FFF : 0xFFFF;

        st = cpssDxChBrgVlanMruProfileValueSet(dev, mruIndex, mruValue);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, mruIndex, mruValue);

        /*  1.2. */
        st = cpssDxChBrgVlanMruProfileValueGet(dev, mruIndex, &mruValueGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mruIndex);

        UTF_VERIFY_EQUAL1_STRING_MAC(mruValue, mruValueGet,
            "get another mruValue than was set: %d", dev);

        mruValue = mruMaxValue;

        st = cpssDxChBrgVlanMruProfileValueSet(dev, mruIndex, mruValue);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, mruIndex, mruValue);

        /*  1.2. */
        st = cpssDxChBrgVlanMruProfileValueGet(dev, mruIndex, &mruValueGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mruIndex);

        UTF_VERIFY_EQUAL1_STRING_MAC(mruValue, mruValueGet,
            "get another mruValue than was set: %d", dev);

        /*
            1.3. Call with mruIndex [PRV_CPSS_DXCH_BRG_MRU_INDEX_MAX_CNS + 1],
                           mruValue [16].
            Expected: non GT_OK.
        */
        mruIndex = 8;
        mruValue = 16;

        st = cpssDxChBrgVlanMruProfileValueSet(dev, mruIndex, mruValue);
        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, mruIndex, mruValue);

        /*
            1.4. Call with mruIndex [2] and mruValue [0x10000].
            Expected: non GT_OK.
        */
        mruIndex = 2;
        mruValue = mruMaxValue + 1;

        st = cpssDxChBrgVlanMruProfileValueSet(dev, mruIndex, mruValue);
        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, mruIndex, mruValue);
    }

    mruIndex = 0;
    mruValue = 1;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanMruProfileValueSet(dev, mruIndex, mruValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanMruProfileValueSet(dev, mruIndex, mruValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanMruProfileValueGet)
{
/*
ITERATE_DEVICES (DxCh2 and above)
    1.1. Call with mruIndex [0] not NULL mruValuePtr.
    Expected: GT_OK.
    1.2. Call with mruIndex [PRV_CPSS_DXCH_BRG_MRU_INDEX_MAX_CNS + 1]
    Expected: non GT_OK.
    1.3. Call with NULL mruValuePtr
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st       = GT_OK;

    GT_U8     dev;
    GT_U32    mruIndex = 0;
    GT_U32    mruValue = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        mruIndex = 0;

        st = cpssDxChBrgVlanMruProfileValueGet(dev, mruIndex, &mruValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mruIndex);

        mruIndex = 3;

        st = cpssDxChBrgVlanMruProfileValueGet(dev, mruIndex, &mruValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mruIndex);

        mruIndex = 7;

        st = cpssDxChBrgVlanMruProfileValueGet(dev, mruIndex, &mruValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mruIndex);

        /*  1.2. */
        mruIndex = 8;

        st = cpssDxChBrgVlanMruProfileValueGet(dev, mruIndex, &mruValue);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, mruIndex);

        mruIndex = 0;

        /*  1.3.  */
        st = cpssDxChBrgVlanMruProfileValueGet(dev, mruIndex, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, mruIndex);
    }

    mruIndex = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanMruProfileValueGet(dev, mruIndex, &mruValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanMruProfileValueGet(dev, mruIndex, &mruValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanNaToCpuEnable
(
    IN GT_U8     devNum,
    IN GT_U16    vlanId,
    IN GT_BOOL   enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanNaToCpuEnable)
{
/*
    ITERATE_DEVICES (DxCh2 and above)
    1.1. Call with vlanId [100] and enable [GT_FALSE and GT_ TRUE].
    Expected: GT_OK.
    1.2. Call with out of range vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)]
                   and enable [GT_FALSE].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st     = GT_OK;

    GT_U8     dev;
    GT_U16    vlanId = 0;
    GT_BOOL   status = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with vlanId [100] and enable [GT_FALSE and GT_ TRUE].
            Expected: GT_OK.
        */
        /* Call with enable [GT_FALSE] */
        vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;
        status = GT_FALSE;

        st = cpssDxChBrgVlanNaToCpuEnable(dev, vlanId, status);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlanId, status);

        /* Call with enable [GT_TRUE] */
        status = GT_TRUE;

        st = cpssDxChBrgVlanNaToCpuEnable(dev, vlanId, status);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlanId, status);

        /*
            1.2. Call with out of range vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)]
                           and enable [GT_FALSE].
            Expected: GT_BAD_PARAM.
        */
        vlanId = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev);
        status = GT_FALSE;

        st = cpssDxChBrgVlanNaToCpuEnable(dev, vlanId, status);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, vlanId, status);
    }

    vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;
    status = GT_FALSE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanNaToCpuEnable(dev, vlanId, status);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanNaToCpuEnable(dev, vlanId, status);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanBridgingModeSet
(
    IN GT_U8                devNum,
    IN CPSS_BRG_MODE_ENT    brgMode
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanBridgingModeSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with brgMode [CPSS_BRG_MODE_802_1Q_E /
                            CPSS_BRG_MODE_802_1D_E].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgVlanBridgingModeGet
    Expected: GT_OK and same brgMode.
    1.3. Call with out of range brgMode.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS         st    = GT_OK;
    GT_U8             dev;
    CPSS_BRG_MODE_ENT brgMode = CPSS_BRG_MODE_802_1Q_E;
    CPSS_BRG_MODE_ENT brgModeGet = CPSS_BRG_MODE_802_1Q_E;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ( UTF_CPSS_PP_E_ARCH_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with brgMode [CPSS_BRG_MODE_802_1Q_E /
                                    CPSS_BRG_MODE_802_1D_E].
            Expected: GT_OK.
        */
        /* Call with brgMode [CPSS_BRG_MODE_802_1Q_E] */
        brgMode = CPSS_BRG_MODE_802_1Q_E;

        st = cpssDxChBrgVlanBridgingModeSet(dev, brgMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, brgMode);

        /* 1.2. */
        st = cpssDxChBrgVlanBridgingModeGet(dev, &brgModeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(brgMode, brgModeGet,
            "get another brgMode than was set: %d", dev);

        /* Call with brgMode [CPSS_BRG_MODE_802_1D_E] */
        brgMode = CPSS_BRG_MODE_802_1D_E;

        st = cpssDxChBrgVlanBridgingModeSet(dev, brgMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, brgMode);

        /* 1.2. */
        st = cpssDxChBrgVlanBridgingModeGet(dev, &brgModeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(brgMode, brgModeGet,
            "get another brgMode than was set: %d", dev);

        /*
            1.3. Call with wrong enum values brgMode.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanBridgingModeSet
                                                        (dev, brgMode),
                                                        brgMode);
    }

    brgMode = CPSS_BRG_MODE_802_1Q_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanBridgingModeSet(dev, brgMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanBridgingModeSet(dev, brgMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanBridgingModeGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with not NULL brgMode.
    Expected: GT_OK.
    1.2. Call with NULL brgMode.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st    = GT_OK;


    GT_U8             dev;
    CPSS_BRG_MODE_ENT brgMode = CPSS_BRG_MODE_802_1Q_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ( UTF_CPSS_PP_E_ARCH_CNS));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        st = cpssDxChBrgVlanBridgingModeGet(dev, &brgMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChBrgVlanBridgingModeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ( UTF_CPSS_PP_E_ARCH_CNS));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanBridgingModeGet(dev, &brgMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanBridgingModeGet(dev, &brgMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanVrfIdSet
(
    IN GT_U8                   devNum,
    IN GT_U16                  vlanId,
    IN GT_U32                  vrfId
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanVrfIdSet)
{
/*
    ITERATE_DEVICES (DxCh3 and above)
    1.1. Call with vlanId [100 / 4095], vrfId [0 / 4095].
    Expected: GT_OK.
    1.2. Call with out of range vlandId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)], vrfId [0].
    Expected: GT_BAD_PARAM.
    1.3. Call with vlanId [100] and vrfId [4096]
    Expected: non GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U16  vlanId = 0;
    GT_U32  vrfId  = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with vlanId [100 / 4095], vrfId [0 / 4095].
            Expected: GT_OK.
        */

        /* Call with vlanId [100] */
        vlanId = 100;
        vrfId  = 0;

        st = cpssDxChBrgVlanVrfIdSet(dev, vlanId, vrfId);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlanId, vrfId);

        /* Call with vlanId [4095] */
        vlanId = 4095;
        vrfId  = 4095;

        st = cpssDxChBrgVlanVrfIdSet(dev, vlanId, vrfId);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlanId, vrfId);

        /*
            1.2. Call with out of range vlandId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)], vrfId [0].
            Expected: GT_BAD_PARAM.
        */
        vlanId = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev);

        st = cpssDxChBrgVlanVrfIdSet(dev, vlanId, vrfId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, vlanId);

        vlanId = 100;

        /*
            1.3. Call with vlanId [100] and vrfId [4096]
            Expected: non GT_OK.
        */
        vrfId = 4096;

        st = cpssDxChBrgVlanVrfIdSet(dev, vlanId, vrfId);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, vrfId = %d", dev, vrfId);
    }

    vlanId = 100;
    vrfId  = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanVrfIdSet(dev, vlanId, vrfId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanVrfIdSet(dev, vlanId, vrfId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortTranslationEnableSet
(
    IN GT_U8                       devNum,
    IN GT_U8                       port,
    IN CPSS_DIRECTION_ENT          direction,
    IN GT_BOOL                     enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortTranslationEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxCh3 and above)
    1.1.1. Call with direction [CPSS_DIRECTION_INGRESS_E /
                                CPSS_DIRECTION_EGRESS_E]
    Expected: GT_BAD_PARAM when enable=CPSS_DXCH_BRG_VLAN_TRANSLATION_SID_VID0_E
              and ingress translation or egress translation and device is not
              above Bobcat2 B0).
              GT_OK - overwise.
    1.1.2. Call cpssDxChBrgVlanPortTranslationEnableGet with same direction.
    Expected: GT_OK and the same enable.
    1.1.3. Call with out of range direction.
    Expected: GT_BAD_PARAM.
*/
    GT_U32              i;
    GT_STATUS           st   = GT_OK;
    GT_U8               dev;
    GT_PORT_NUM         port = BRG_VLAN_VALID_PHY_PORT_CNS;
    CPSS_DIRECTION_ENT  direction = CPSS_DIRECTION_INGRESS_E;
    CPSS_DXCH_BRG_VLAN_TRANSLATION_ENT enable    = GT_FALSE;
    CPSS_DXCH_BRG_VLAN_TRANSLATION_ENT enableGet = GT_FALSE;
    CPSS_DXCH_BRG_VLAN_TRANSLATION_ENT transArr[] = {
                                      CPSS_DXCH_BRG_VLAN_TRANSLATION_VID0_E,
                                      CPSS_DXCH_BRG_VLAN_TRANSLATION_SID_VID0_E,
                                      CPSS_DXCH_BRG_VLAN_TRANSLATION_DISABLE_E};


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {

            for (i = 0; i < sizeof(transArr)/sizeof(transArr[0]); i++)
            {
                /*
                    1.1.1. Call with direction [CPSS_DIRECTION_INGRESS_E /
                                                CPSS_DIRECTION_EGRESS_E]
                */
                enable = transArr[i];

                /* Call with direction [CPSS_DIRECTION_INGRESS_E] */
                direction = CPSS_DIRECTION_INGRESS_E;

                st = cpssDxChBrgVlanPortTranslationEnableSet(dev, port, direction, enable);

                if ((UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) &&
                     port >= UTF_GET_MAX_DEFAULT_EPORTS_NUM_MAC(dev))
                    ||
                    (enable >= CPSS_DXCH_BRG_VLAN_TRANSLATION_SID_VID0_E))
                {
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, port, direction, enable);
                }
                else
                {
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, direction, enable);

                    /*
                        1.1.2. Call cpssDxChBrgVlanPortTranslationEnableGet with same direction.
                        Expected: GT_OK and the same enable.
                    */
                    st = cpssDxChBrgVlanPortTranslationEnableGet(dev, port, direction, &enableGet);
                    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                               "cpssDxChBrgVlanPortTranslationEnableGet: %d, %d, %d", dev, port, direction);

                    /* Verifying values */
                    UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                               "get another enable than was set: %d, %d", dev, port);
                }

                /* Call with direction [CPSS_DIRECTION_EGRESS_E] */
                direction = CPSS_DIRECTION_EGRESS_E;

                st = cpssDxChBrgVlanPortTranslationEnableSet(dev, port, direction, enable);

                if (enable >= CPSS_DXCH_BRG_VLAN_TRANSLATION_SID_VID0_E &&
                    !PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, port, direction, enable);
                }
                else
                {
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, direction, enable);
                    /*
                      1.1.2. Call cpssDxChBrgVlanPortTranslationEnableGet with same direction.
                      Expected: GT_OK and the same enable.
                    */
                    st = cpssDxChBrgVlanPortTranslationEnableGet(dev, port, direction, &enableGet);
                    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                           "cpssDxChBrgVlanPortTranslationEnableGet: %d, %d, %d",
                           dev, port, direction);

                    /* Verifying values */
                    UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                           "get another enable than was set: %d, %d", dev, port);

                }


                if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) &&
                   port >= UTF_GET_MAX_DEFAULT_EPORTS_NUM_MAC(dev))
                {
                    /* the ingress direction supports default ePorts range and not full ePorts range */
                }
                else
                {
                    /*
                        1.1.3. Call with wrong enum values direction.
                        Expected: GT_BAD_PARAM.
                    */
                    UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanPortTranslationEnableSet
                                                                (dev, port, direction, GT_FALSE),
                                                                direction);
                }
            }
        }

        direction = CPSS_DIRECTION_INGRESS_E;
        enable    = CPSS_DXCH_BRG_VLAN_TRANSLATION_DISABLE_E;

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanPortTranslationEnableSet(dev, port, direction, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanPortTranslationEnableSet(dev, port, direction, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanPortTranslationEnableSet(dev, port, direction, enable);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        else
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    direction = CPSS_DIRECTION_INGRESS_E;
    enable    = CPSS_DXCH_BRG_VLAN_TRANSLATION_DISABLE_E;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = BRG_VLAN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortTranslationEnableSet(dev, port, direction, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortTranslationEnableSet(dev, port, direction, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortTranslationEnableGet
(
    IN  GT_U8                       devNum,
    IN  GT_U8                       port,
    IN  CPSS_DIRECTION_ENT          direction,
    OUT GT_BOOL                     *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortTranslationEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxCh3 and above)
    1.1.1. Call with direction [CPSS_DIRECTION_INGRESS_E /
                                CPSS_DIRECTION_EGRESS_E]
                     and not-NULL enablePtr.
    Expected: GT_OK.
    1.1.2. Call with out of range direction.
    Expected: GT_BAD_PARAM.
    1.1.3. Call with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                          st        = GT_OK;
    GT_U8                              dev;
    GT_PORT_NUM                        port      = BRG_VLAN_VALID_PHY_PORT_CNS;
    CPSS_DIRECTION_ENT                 direction = CPSS_DIRECTION_INGRESS_E;
    CPSS_DXCH_BRG_VLAN_TRANSLATION_ENT enable;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with direction [CPSS_DIRECTION_INGRESS_E /
                                            CPSS_DIRECTION_EGRESS_E]
                                 and not-NULL enablePtr.
                Expected: GT_OK.
            */

            /* Call with direction [CPSS_DIRECTION_INGRESS_E] */
            direction = CPSS_DIRECTION_INGRESS_E;

            st = cpssDxChBrgVlanPortTranslationEnableGet(dev, port, direction, &enable);
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) &&
               port >= UTF_GET_MAX_DEFAULT_EPORTS_NUM_MAC(dev))
            {
                /* the ingress direction supports default ePorts range and not full ePorts range */
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, direction);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, direction);
            }

            /* Call with direction [CPSS_DIRECTION_EGRESS_E] */
            direction = CPSS_DIRECTION_EGRESS_E;

            st = cpssDxChBrgVlanPortTranslationEnableGet(dev, port, direction, &enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, direction);

            /*
                1.1.2. Call with wrong enum values direction.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanPortTranslationEnableGet
                                                        (dev, port, direction, &enable),
                                                        direction);

            direction = CPSS_DIRECTION_INGRESS_E;

            /*
                1.1.3. Call with enablePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgVlanPortTranslationEnableGet(dev, port, direction, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, enablePtr = NULL", dev, port);
        }

        direction = CPSS_DIRECTION_INGRESS_E;

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanPortTranslationEnableGet(dev, port, direction, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanPortTranslationEnableGet(dev, port, direction, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanPortTranslationEnableGet(dev, port, direction, &enable);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        else
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    direction = CPSS_DIRECTION_INGRESS_E;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = BRG_VLAN_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortTranslationEnableGet(dev, port, direction, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortTranslationEnableGet(dev, port, direction, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanTranslationEntryWrite
(
    IN GT_U8                        devNum,
    IN GT_U16                       vlanId,
    IN CPSS_DIRECTION_ENT           direction,
    IN GT_U16                       transVlanId
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanTranslationEntryWrite)
{
/*
    ITERATE_DEVICES (DxCh3 and above)
    1.1. Call with vlanId [0 / 4095],
                   direction [CPSS_DIRECTION_INGRESS_E /
                              CPSS_DIRECTION_EGRESS_E]
                   and transVlanId [100 / 4095].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgVlanTranslationEntryRead with same vlanId and direction.
    Expected: GT_OK and the same transVlanId.
    1.3. Call with out of range vlandId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)].
    Expected: GT_BAD_PARAM.
    1.4. Call with out of range direction.
    Expected: GT_BAD_PARAM.
    1.5. Call with out of range transVlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;

    GT_U8       dev;

    GT_U16              vlanId         = 0;
    CPSS_DIRECTION_ENT  direction      = CPSS_DIRECTION_INGRESS_E;
    GT_U16              transVlanId    = 0;
    GT_U16              transVlanIdGet = 0;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with vlanId [0 / 4095],
                           direction [CPSS_DIRECTION_INGRESS_E /
                                      CPSS_DIRECTION_EGRESS_E]
                           and transVlanId [100 / 4095].
            Expected: GT_OK.
        */

        /* Call with vlanId [100] */
        vlanId      = 100;
        direction   = CPSS_DIRECTION_INGRESS_E;
        transVlanId = 100;

        st = cpssDxChBrgVlanTranslationEntryWrite(dev, vlanId, direction, transVlanId);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, vlanId, direction, transVlanId);

        /*
            1.2. Call cpssDxChBrgVlanTranslationEntryRead with same vlanId and direction.
            Expected: GT_OK and the same transVlanId.
        */
        st = cpssDxChBrgVlanTranslationEntryRead(dev, vlanId, direction, &transVlanIdGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgVlanTranslationEntryRead: %d, %d, %d", dev, vlanId, direction);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(transVlanId, transVlanIdGet,
                   "get another transVlanId than was set: %d", dev);

        /* Call with vlanId [4095] */
        vlanId      = 4095;
        direction   = CPSS_DIRECTION_EGRESS_E;
        transVlanId = 4095;

        st = cpssDxChBrgVlanTranslationEntryWrite(dev, vlanId, direction, transVlanId);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, vlanId, direction, transVlanId);

        /*
            1.2. Call cpssDxChBrgVlanTranslationEntryRead with same vlanId and direction.
            Expected: GT_OK and the same transVlanId.
        */
        st = cpssDxChBrgVlanTranslationEntryRead(dev, vlanId, direction, &transVlanIdGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgVlanTranslationEntryRead: %d, %d, %d", dev, vlanId, direction);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(transVlanId, transVlanIdGet,
                   "get another transVlanId than was set: %d", dev);

        /*
            1.3. Call with out of range vlandId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)].
            Expected: GT_BAD_PARAM.
        */
        vlanId = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev);

        st = cpssDxChBrgVlanTranslationEntryWrite(dev, vlanId, direction, transVlanId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, vlanId);

        vlanId = 100;

        /*
            1.4. Call with wrong enum values direction.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanTranslationEntryWrite
                                                        (dev, vlanId, direction, transVlanId),
                                                        direction);

        direction = CPSS_DIRECTION_INGRESS_E;

        /*
            1.5. Call with out of range transVlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)].
            Expected: GT_BAD_PARAM.
        */
        transVlanId = UTF_CPSS_PP_MAX_VLAN_NUM_CNS(dev);

        st = cpssDxChBrgVlanTranslationEntryWrite(dev, vlanId, direction, transVlanId);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, transVlanId = %d", dev, transVlanId);


        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            /* the ingress support translate from 4K range to 8K range */
            /* the egress support  translate from 8K range to 4K range */

            direction = CPSS_DIRECTION_INGRESS_E;
            vlanId = _6K;
            transVlanId = _2K;

            st = cpssDxChBrgVlanTranslationEntryWrite(dev, vlanId, direction, transVlanId);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, transVlanId = %d", dev, transVlanId);


            vlanId = _3K;
            transVlanId = _6K;

            st = cpssDxChBrgVlanTranslationEntryWrite(dev, vlanId, direction, transVlanId);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, transVlanId = %d", dev, transVlanId);

            st = cpssDxChBrgVlanTranslationEntryRead(dev, vlanId, direction, &transVlanIdGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgVlanTranslationEntryRead: %d, %d, %d", dev, vlanId, direction);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(transVlanId, transVlanIdGet,
                       "get another transVlanId than was set: %d", dev);

            direction = CPSS_DIRECTION_EGRESS_E;
            vlanId = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev) < _6K ?
                        (UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev) - 1) :
                        _6K;
            transVlanId = _2K;

            st = cpssDxChBrgVlanTranslationEntryWrite(dev, vlanId, direction, transVlanId);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, transVlanId = %d", dev, transVlanId);

            st = cpssDxChBrgVlanTranslationEntryRead(dev, vlanId, direction, &transVlanIdGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgVlanTranslationEntryRead: %d, %d, %d", dev, vlanId, direction);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(transVlanId, transVlanIdGet,
                       "get another transVlanId than was set: %d", dev);

            vlanId = _3K;
            transVlanId = _4K;

            st = cpssDxChBrgVlanTranslationEntryWrite(dev, vlanId, direction, transVlanId);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, transVlanId = %d", dev, transVlanId);
        }


    }

    vlanId      = 100;
    direction   = CPSS_DIRECTION_INGRESS_E;
    transVlanId = 100;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanTranslationEntryWrite(dev, vlanId, direction, transVlanId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanTranslationEntryWrite(dev, vlanId, direction, transVlanId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanTranslationEntryRead
(
    IN  GT_U8                       devNum,
    IN  GT_U16                      vlanId,
    IN CPSS_DIRECTION_ENT           direction,
    OUT GT_U16                      *transVlanIdPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanTranslationEntryRead)
{
/*
    ITERATE_DEVICES (DxCh3 and above)
    1.1. Call with vlanId [0 / 4095],
                   direction [CPSS_DIRECTION_INGRESS_E /
                              CPSS_DIRECTION_EGRESS_E]
                   and non-NULL transVlanIdPtr.
    Expected: GT_OK.
    1.2. Call with out of range vlandId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)].
    Expected: GT_BAD_PARAM.
    1.3. Call with out of range direction.
    Expected: GT_BAD_PARAM.
    1.4. Call with transVlanIdPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;

    GT_U8       dev;

    GT_U16              vlanId      = 0;
    CPSS_DIRECTION_ENT  direction   = CPSS_DIRECTION_INGRESS_E;
    GT_U16              transVlanId = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with vlanId [0 / 4095],
                           direction [CPSS_DIRECTION_INGRESS_E /
                                      CPSS_DIRECTION_EGRESS_E]
                           and non-NULL transVlanIdPtr.
            Expected: GT_OK.
        */
        /* Call with vlanId [100] */
        vlanId      = 100;
        direction   = CPSS_DIRECTION_INGRESS_E;

        st = cpssDxChBrgVlanTranslationEntryRead(dev, vlanId, direction, &transVlanId);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlanId, direction);

        /* Call with vlanId [4095] */
        vlanId      = 4095;
        direction   = CPSS_DIRECTION_EGRESS_E;

        st = cpssDxChBrgVlanTranslationEntryRead(dev, vlanId, direction, &transVlanId);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlanId, direction);

        /*
            1.2. Call with out of range vlandId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)].
            Expected: GT_BAD_PARAM.
        */
        vlanId = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev);

        st = cpssDxChBrgVlanTranslationEntryRead(dev, vlanId, direction, &transVlanId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, vlanId);

        vlanId = 100;

        /*
            1.3. Call with wrong enum values direction.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanTranslationEntryRead
                                                        (dev, vlanId, direction, &transVlanId),
                                                        direction);

        direction = CPSS_DIRECTION_INGRESS_E;

        /*
            1.4. Call with transVlanIdPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgVlanTranslationEntryRead(dev, vlanId, direction, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, transVlanIdPtr = NULL", dev);
    }

    vlanId      = 100;
    direction   = CPSS_DIRECTION_INGRESS_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanTranslationEntryRead(dev, vlanId, direction, &transVlanId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanTranslationEntryRead(dev, vlanId, direction, &transVlanId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanValidCheckEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanValidCheckEnableSet)
{
/*
    ITERATE_DEVICES (DxCh2 and above)
    1.1. Call function with enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgVlanValidCheckEnableGet
    Expected: GT_OK and the same enablePtr.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_BOOL     state    = GT_FALSE;
    GT_BOOL     stateGet = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with enable [GT_FALSE / GT_TRUE].
            Expected: GT_OK.
        */
        /* Call with enable [GT_FALSE] */
        state = GT_FALSE;

        st = cpssDxChBrgVlanValidCheckEnableSet(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call cpssDxChBrgVlanValidCheckEnableGet
            Expected: GT_OK and the same enablePtr.
        */
        st = cpssDxChBrgVlanValidCheckEnableGet(dev, &stateGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgVlanValidCheckEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                       "get another enable than was set: %d", dev);

        /* Call with enable [GT_TRUE] */
        state = GT_TRUE;

        st = cpssDxChBrgVlanValidCheckEnableSet(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call cpssDxChBrgVlanValidCheckEnableGet
            Expected: GT_OK and the same enablePtr.
        */
        st = cpssDxChBrgVlanValidCheckEnableGet(dev, &stateGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgVlanValidCheckEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                       "get another enable than was set: %d", dev);
    }

    state = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanValidCheckEnableSet(dev, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanValidCheckEnableSet(dev, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanValidCheckEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanValidCheckEnableGet)
{
/*
    ITERATE_DEVICES (DxCh2 and above)
    1.1. Call with non-null enablePtr.
    Expected: GT_OK.
    1.2. Call with null enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_BOOL     state = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgVlanValidCheckEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with null enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgVlanValidCheckEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanValidCheckEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanValidCheckEnableGet(dev, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanFloodVidxModeSet
(
    IN  GT_U8                                   devNum,
    IN  GT_U16                                  vlanId,
    IN  GT_U16                                  floodVidx,
    IN  CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_ENT  floodVidxMode
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanFloodVidxModeSet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call function with correct parameters  and floodVidxMode =
                  [ CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_UNREG_MC_E /
                    CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E ].
    Expected: GT_OK.
    1.2. Call function with incorrect floodVidx > BIT_12.
    Expected: GT_OUT_OF_RANGE.
    1.3. Call function with incorrect vlanId.
    Expected: GT_BAD_PARAM.
    1.4. Call function with wrong enum values floodVidxMode.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;

    GT_U8       dev;
    CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_ENT floodVidxMode =
                                    CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_UNREG_MC_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with correct parameters  and floodVidxMode =
                  [ CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_UNREG_MC_E /
                    CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E ].
            Expected: GT_OK.
        */
        st = cpssDxChBrgVlanFloodVidxModeSet(dev, 0, 0,
                                     CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_UNREG_MC_E);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChBrgVlanFloodVidxModeSet(dev, 0, 0,
                            CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call function with incorrect floodVidx > BIT_12.
            Expected: GT_OUT_OF_RANGE.
        */
        st = cpssDxChBrgVlanFloodVidxModeSet(dev, 0, MAX_VIDX_MAC(dev),
                            CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        /*
            1.3. Call function with incorrect vlanId = BRG_VLAN_INVALID_VLAN_ID_CNS.
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChBrgVlanFloodVidxModeSet(dev, BRG_VLAN_INVALID_VLAN_ID_CNS, 0,
                            CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /*
            1.4. Call function with wrong enum values floodVidxMode.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanFloodVidxModeSet
                                                        (dev, 0, 0, floodVidxMode),
                                                        floodVidxMode);

        floodVidxMode = 0;
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanFloodVidxModeSet(dev, 0, 0,
                                  CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_UNREG_MC_E);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanFloodVidxModeSet(dev, 0, 0,
                              CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_UNREG_MC_E);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanLocalSwitchingEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_U16    vlanId,
    IN  CPSS_DXCH_BRG_VLAN_LOCAL_SWITCHING_TRAFFIC_TYPE_ENT  trafficType,
    IN  GT_BOOL   enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanLocalSwitchingEnableSet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call function with correct parameters  and trafficType =
               [ CPSS_DXCH_BRG_VLAN_LOCAL_SWITCHING_TRAFFIC_TYPE_KNOWN_UC_E /
                 CPSS_DXCH_BRG_VLAN_LOCAL_SWITCHING_TRAFFIC_TYPE_FLOODED_E ].
    Expected: GT_OK.
    1.2. Call function with wrong enum values vlanId = BRG_VLAN_INVALID_VLAN_ID_CNS.
    Expected: GT_BAD_PARAM.
    1.3. Call function with wrong enum values trafficType.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;

    GT_U8       dev;
    CPSS_DXCH_BRG_VLAN_LOCAL_SWITCHING_TRAFFIC_TYPE_ENT trafficType =
                        CPSS_DXCH_BRG_VLAN_LOCAL_SWITCHING_TRAFFIC_TYPE_KNOWN_UC_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with correct parameters and trafficType =
               [ CPSS_DXCH_BRG_VLAN_LOCAL_SWITCHING_TRAFFIC_TYPE_KNOWN_UC_E /
                 CPSS_DXCH_BRG_VLAN_LOCAL_SWITCHING_TRAFFIC_TYPE_FLOODED_E ]
                                 and  enable status [GT_TRUE / GT_FALSE].
            Expected: GT_OK.
        */
        st = cpssDxChBrgVlanLocalSwitchingEnableSet(dev, BRG_VLAN_TESTED_VLAN_ID_CNS,
                CPSS_DXCH_BRG_VLAN_LOCAL_SWITCHING_TRAFFIC_TYPE_KNOWN_UC_E, GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChBrgVlanLocalSwitchingEnableSet(dev, BRG_VLAN_TESTED_VLAN_ID_CNS,
                CPSS_DXCH_BRG_VLAN_LOCAL_SWITCHING_TRAFFIC_TYPE_FLOODED_E, GT_FALSE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call function with incorrect vlanId = BRG_VLAN_INVALID_VLAN_ID_CNS.
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChBrgVlanLocalSwitchingEnableSet(dev, BRG_VLAN_INVALID_VLAN_ID_CNS,
               CPSS_DXCH_BRG_VLAN_LOCAL_SWITCHING_TRAFFIC_TYPE_FLOODED_E, GT_FALSE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /*
            1.3. Call function with wrong enum values trafficType.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanLocalSwitchingEnableSet
                                                        (dev, BRG_VLAN_TESTED_VLAN_ID_CNS, trafficType, GT_FALSE),
                                                        trafficType);
    }

    trafficType = CPSS_DXCH_BRG_VLAN_LOCAL_SWITCHING_TRAFFIC_TYPE_FLOODED_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanLocalSwitchingEnableSet(dev, BRG_VLAN_TESTED_VLAN_ID_CNS,
                CPSS_DXCH_BRG_VLAN_LOCAL_SWITCHING_TRAFFIC_TYPE_FLOODED_E, GT_FALSE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanLocalSwitchingEnableSet(dev, BRG_VLAN_TESTED_VLAN_ID_CNS,
            CPSS_DXCH_BRG_VLAN_LOCAL_SWITCHING_TRAFFIC_TYPE_FLOODED_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanMemberSet
(
    IN  GT_U8                               devNum,
    IN  GT_U16                              vlanId,
    IN  GT_U8                               portNum,
    IN  GT_BOOL                             isMember,
    IN  GT_BOOL                             isTagged,
    IN  CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT taggingCmd
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanMemberSet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call function with correct parameters:
                isMember = [GT_FALSE, GT_TRUE, GT_TRUE],
                isTagged = [GT_FALSE, GT_TRUE, GT_TRUE],
                taggingCmd = [CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E,
                              CPSS_DXCH_BRG_VLAN_PORT_PUSH_TAG0_CMD_E,
                              CPSS_DXCH_BRG_VLAN_PORT_DO_NOT_MODIFY_TAG_CMD_E(SIP5 only)].
    Expected: GT_OK.
    1.2. Call function with incorrect vlanId.
    Expected: GT_BAD_PARAM.
    1.3. Call function with wrong enum values taggingCmd.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;
    GT_U32      index = 0;

    GT_U8                   dev;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT taggingCmd = CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /*
        1.1. Call function with correct parameters:
                    isMember = [GT_FALSE, GT_TRUE, GT_TRUE],
                    isTagged = [GT_FALSE, GT_TRUE, GT_TRUE],
                    taggingCmd = [CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E,
                                  CPSS_DXCH_BRG_VLAN_PORT_PUSH_TAG0_CMD_E,
                                  CPSS_DXCH_BRG_VLAN_PORT_DO_NOT_MODIFY_TAG_CMD_E(SIP5 only)].
             Expected: GT_OK.
        */
        st = cpssDxChBrgVlanMemberSet(dev, BRG_VLAN_TESTED_VLAN_ID_CNS,
                                      BRG_VLAN_VALID_PHY_PORT_CNS, GT_FALSE, GT_TRUE,
                                      CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        st = cpssDxChBrgVlanMemberSet(dev, BRG_VLAN_TESTED_VLAN_ID_CNS,
                                      BRG_VLAN_VALID_PHY_PORT_CNS, GT_TRUE, GT_FALSE,
                                      CPSS_DXCH_BRG_VLAN_PORT_PUSH_TAG0_CMD_E);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            st = cpssDxChBrgVlanMemberSet(dev, BRG_VLAN_TESTED_VLAN_ID_CNS,
                                          BRG_VLAN_VALID_PHY_PORT_CNS, GT_TRUE, GT_TRUE,
                                          CPSS_DXCH_BRG_VLAN_PORT_DO_NOT_MODIFY_TAG_CMD_E);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        }

        /*
            1.2. Call function with incorrect vlanId.
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChBrgVlanMemberSet(dev, BRG_VLAN_INVALID_VLAN_ID_CNS,
                                      BRG_VLAN_VALID_PHY_PORT_CNS, GT_TRUE, GT_TRUE, 0);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /*
            1.3. Call function with wrong enum values taggingCmd.
            Expected: GT_BAD_PARAM.
        */
        if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(dev))
        {
            UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanMemberSet
                                (dev, BRG_VLAN_TESTED_VLAN_ID_CNS,
                                 BRG_VLAN_VALID_PHY_PORT_CNS, GT_TRUE, GT_TRUE, taggingCmd),
                                taggingCmd);
        }
        else
        {
            for(index = 0; index < utfInvalidEnumArrSize; index++)
            {
                taggingCmd = utfInvalidEnumArr[index];

                st = cpssDxChBrgVlanMemberSet(dev, BRG_VLAN_TESTED_VLAN_ID_CNS,
                                    BRG_VLAN_VALID_PHY_PORT_CNS, GT_TRUE, GT_TRUE, taggingCmd);

                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            }
        }
    }

    taggingCmd = CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanMemberSet(dev, BRG_VLAN_TESTED_VLAN_ID_CNS,
                                   BRG_VLAN_VALID_PHY_PORT_CNS, GT_FALSE, GT_TRUE, 0);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanMemberSet(dev, BRG_VLAN_TESTED_VLAN_ID_CNS,
                               BRG_VLAN_VALID_PHY_PORT_CNS, GT_FALSE, GT_TRUE, 0);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortEgressTpidGet
(
    IN  GT_U8                devNum,
    IN  GT_U8                portNum,
    IN  CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ENT  ethMode,
    OUT GT_U32               *tpidEntryIndexPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortEgressTpidGet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call function with correct parameters and ethMode =
              [ CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E /
                CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E /
                CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_EGRESS_TS_ETHERTYPE_E ].
    Expected: GT_OK.
    1.2. Call function with incorrect portNum = BRG_VLAN_INVALID_PORT_CNS(dev).
    Expected: GT_BAD_PARAM.
    1.3. Call function with wrong enum values ethMode.
    Expected: GT_BAD_PARAM.
    1.4. Call function with incorrect tpidEntryIndexPtr (NULL).
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;

    GT_U8       dev;
    CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ENT  ethMode = CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E;
    GT_U32      tpidEntryIndexPtr;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with correct parameters and ethMode =
              [ CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E /
                CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E /
                CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_EGRESS_TS_ETHERTYPE_E ].
            Expected: GT_OK.
        */
        st = cpssDxChBrgVlanPortEgressTpidGet(dev, BRG_VLAN_VALID_PHY_PORT_CNS,
                                         CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E, &tpidEntryIndexPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChBrgVlanPortEgressTpidGet(dev, BRG_VLAN_VALID_PHY_PORT_CNS,
                                         CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E, &tpidEntryIndexPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        if(GT_TRUE == PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_ENABLED_MAC(dev))
        {
            st = cpssDxChBrgVlanPortEgressTpidGet(dev, BRG_VLAN_VALID_PHY_PORT_CNS,
                                             CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_EGRESS_TS_ETHERTYPE_E, &tpidEntryIndexPtr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }
        /*
            1.2. Call function with incorrect portNum = BRG_VLAN_INVALID_PORT_CNS(dev).
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChBrgVlanPortEgressTpidGet(dev, BRG_VLAN_INVALID_PORT_CNS(dev),
                                         CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E, &tpidEntryIndexPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /*
            1.3. Call function with wrong enum values ethMode.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanPortEgressTpidGet
                                                        (dev, BRG_VLAN_VALID_PHY_PORT_CNS, ethMode, &tpidEntryIndexPtr),
                                                        ethMode);

        /*
            1.4. Call function with incorrect tpidEntryIndexPtr (NULL).
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgVlanPortEgressTpidGet(dev, BRG_VLAN_VALID_PHY_PORT_CNS,
                                         CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortEgressTpidGet(dev, BRG_VLAN_VALID_PHY_PORT_CNS,
                                         CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E, &tpidEntryIndexPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortEgressTpidGet(dev, BRG_VLAN_VALID_PHY_PORT_CNS,
                                     CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E, &tpidEntryIndexPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortEgressTpidSet
(
    IN  GT_U8                devNum,
    IN  GT_U8                portNum,
    IN  CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ENT  ethMode,
    IN  GT_U32               tpidEntryIndex
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortEgressTpidSet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call function with correct parameters and ethMode =
                  [ CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E /
                    CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E /
                    CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_EGRESS_TS_ETHERTYPE_E].
    Expected: GT_OK and the same tpidEntryIndex.
    1.2. Call function with incorrect portNum = BRG_VLAN_INVALID_PORT_CNS(dev).
    Expected: GT_BAD_PARAM.
    1.3. Call function with wrong enum values ethMode.
    Expected: GT_BAD_PARAM.
    1.4. Call function with incorrect tpidEntryIndex = 9.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;

    GT_U8       dev;
    CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ENT  ethMode = CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E;
    GT_U32      tpidEntryIndex    = 1;
    GT_U32      tpidEntryIndexGet = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with correct parameters and ethMode =
              [ CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E /
                CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E /
                CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_EGRESS_TS_ETHERTYPE_E ].
            Expected: GT_OK and the same tpidEntryIndex.
        */
        st = cpssDxChBrgVlanPortEgressTpidSet(dev, BRG_VLAN_VALID_PHY_PORT_CNS,
                                         CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E, tpidEntryIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Get value of tpidEntryIndex*/
        st = cpssDxChBrgVlanPortEgressTpidGet(dev, BRG_VLAN_VALID_PHY_PORT_CNS,
                                         CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E, &tpidEntryIndexGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Check if we get the same tpidEntryIndex*/
        UTF_VERIFY_EQUAL2_STRING_MAC(tpidEntryIndex, tpidEntryIndexGet,
         "get another tpidEntryIndex than was set: tpidBmp = %d,  tpidEntryIndexGet = %d",
                                     tpidEntryIndex, tpidEntryIndexGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChBrgVlanPortEgressTpidSet(dev, BRG_VLAN_VALID_PHY_PORT_CNS,
                                         CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E, tpidEntryIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Get value of tpidEntryIndex*/
        st = cpssDxChBrgVlanPortEgressTpidGet(dev, BRG_VLAN_VALID_PHY_PORT_CNS,
                                         CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E, &tpidEntryIndexGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Check if we get the same tpidEntryIndex*/
        UTF_VERIFY_EQUAL2_STRING_MAC(tpidEntryIndex, tpidEntryIndexGet,
         "get another tpidEntryIndex than was set: tpidBmp = %d,  tpidEntryIndexGet = %d",
                                     tpidEntryIndex, tpidEntryIndexGet);

        if(GT_TRUE == PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_ENABLED_MAC(dev))
        {
            st = cpssDxChBrgVlanPortEgressTpidSet(dev, BRG_VLAN_VALID_PHY_PORT_CNS,
                                             CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_EGRESS_TS_ETHERTYPE_E, tpidEntryIndex);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* Get value of tpidEntryIndex*/
            st = cpssDxChBrgVlanPortEgressTpidGet(dev, BRG_VLAN_VALID_PHY_PORT_CNS,
                                             CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_EGRESS_TS_ETHERTYPE_E, &tpidEntryIndexGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* Check if we get the same tpidEntryIndex*/
            UTF_VERIFY_EQUAL2_STRING_MAC(tpidEntryIndex, tpidEntryIndexGet,
             "get another tpidEntryIndex than was set: tpidBmp = %d,  tpidEntryIndexGet = %d",
                                         tpidEntryIndex, tpidEntryIndexGet);
        }
        /*
            1.2. Call function with incorrect portNum = BRG_VLAN_INVALID_PORT_CNS(dev).
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChBrgVlanPortEgressTpidSet(dev, BRG_VLAN_INVALID_PORT_CNS(dev),
                                         CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E, tpidEntryIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /*
            1.3. Call function with wrong enum values ethMode.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanPortEgressTpidSet
                                                        (dev, BRG_VLAN_VALID_PHY_PORT_CNS, ethMode, tpidEntryIndex),
                                                        ethMode);

        /*
            1.4. Call function with incorrect tpidEntryIndexPtr (9).
            Expected: GT_OUT_OF_RANGE.
        */
        st = cpssDxChBrgVlanPortEgressTpidSet(dev, BRG_VLAN_VALID_PHY_PORT_CNS,
                                              CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E, 9);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortEgressTpidSet(dev, BRG_VLAN_VALID_PHY_PORT_CNS,
                                         CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E, tpidEntryIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortEgressTpidSet(dev, BRG_VLAN_VALID_PHY_PORT_CNS,
                                     CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E, tpidEntryIndex);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortIngressTpidGet
(
    IN  GT_U8                devNum,
    IN  GT_U8                portNum,
    IN  CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ENT  ethMode,
    OUT GT_U32               *tpidBmpPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortIngressTpidGet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call function with correct parameters and ethMode =
              [ CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E /
                CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E /
                CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_INGRESS_PASSENGER_ETHERTYPE0_E /
                CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_INGRESS_PASSENGER_ETHERTYPE1_E ].
    Expected: GT_OK.
    1.2. Call function with incorrect portNum = BRG_VLAN_INVALID_PORT_CNS(dev).
    Expected: GT_BAD_PARAM.
    1.3. Call function with wrong enum values ethMode.
    Expected: GT_BAD_PARAM.
    1.4. Call function with incorrect tpidBmpGet (NULL).
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = BRG_VLAN_VALID_PHY_PORT_CNS;
    CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ENT  ethMode = CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E;
    GT_U32      tpidBmpGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* prepare physical port iterator */
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /*
                1.1. Call function with correct parameters and ethMode =
                  [ CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E /
                    CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E /
                    CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_INGRESS_PASSENGER_ETHERTYPE0_E /
                    CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_INGRESS_PASSENGER_ETHERTYPE1_E ].
                Expected: GT_OK.
            */
            st = cpssDxChBrgVlanPortIngressTpidGet(dev, port,
                                 CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E, &tpidBmpGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            st = cpssDxChBrgVlanPortIngressTpidGet(dev, port,
                                 CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E, &tpidBmpGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            if(GT_TRUE == PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_ENABLED_MAC(dev))
            {
                st = cpssDxChBrgVlanPortIngressTpidGet(dev, port,
                                     CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_INGRESS_PASSENGER_ETHERTYPE0_E, &tpidBmpGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                st = cpssDxChBrgVlanPortIngressTpidGet(dev, port,
                                     CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_INGRESS_PASSENGER_ETHERTYPE1_E, &tpidBmpGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            }
            /*
                1.2. Call function with incorrect
                     portNum = BRG_VLAN_INVALID_PORT_CNS(dev).
                Expected: GT_BAD_PARAM.
            */
            st = cpssDxChBrgVlanPortIngressTpidGet(dev,
                 BRG_VLAN_INVALID_PORT_CNS(dev),CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E, &tpidBmpGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            /*
                1.3. Call function with wrong enum values ethMode.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanPortIngressTpidGet
                                                        (dev, port, ethMode, &tpidBmpGet),
                                                        ethMode);

            /*
                1.4. Call function with incorrect tpidBmpGet (NULL).
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgVlanPortIngressTpidGet(dev, port,
                                             CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E, NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortIngressTpidGet(dev, port,
                                    CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E, &tpidBmpGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortIngressTpidGet(dev, port,
                                     CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E, &tpidBmpGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortIngressTpidSet
(
    IN  GT_U8                devNum,
    IN  GT_U8                portNum,
    IN  CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ENT  ethMode,
    IN  GT_U32               tpidBmp
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortIngressTpidSet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call function with correct parameters and ethMode =
                  [ CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E /
                    CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E /
                    CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_INGRESS_PASSENGER_ETHERTYPE0_E /
                    CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_INGRESS_PASSENGER_ETHERTYPE1_E ].
    Expected: GT_OK and the same tpidBmp.
    1.2. Call function with incorrect portNum = BRG_VLAN_INVALID_PORT_CNS(dev).
    Expected: GT_BAD_PARAM.
    1.3. Call function with wrong enum values ethMode.
    Expected: GT_BAD_PARAM.
    1.4. Call function with incorrect tpidBmp = BIT_8.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = BRG_VLAN_VALID_PHY_PORT_CNS;
    CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ENT  ethMode = CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E;
    GT_U32      tpidBmp    = 1;
    GT_U32      tpidBmpGet = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* prepare physical port iterator */
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /*
                1.1. Call function with correct parameters and ethMode =
                  [ CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E /
                    CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E /
                    CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_INGRESS_PASSENGER_ETHERTYPE0_E /
                    CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_INGRESS_PASSENGER_ETHERTYPE1_E ].
                Expected: GT_OK and the same tpidBmp.
            */
            st = cpssDxChBrgVlanPortIngressTpidSet(dev, port,
                                 CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E, tpidBmp);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* Get value of tpidBmp*/
            st = cpssDxChBrgVlanPortIngressTpidGet(dev, port,
                              CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E, &tpidBmpGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* Check if we get the same tpidBmp*/
            UTF_VERIFY_EQUAL2_STRING_MAC(tpidBmp, tpidBmpGet,
              "get another tpidBmp than was set: tpidBmp = %d, tpidBmpGet = %d",
                                         tpidBmp, tpidBmpGet);

            st = cpssDxChBrgVlanPortIngressTpidSet(dev, port,
                                 CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E, tpidBmp);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* Get value of tpidBmp*/
            st = cpssDxChBrgVlanPortIngressTpidGet(dev, port,
                              CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E, &tpidBmpGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* Check if we get the same tpidBmp*/
            UTF_VERIFY_EQUAL2_STRING_MAC(tpidBmp, tpidBmpGet,
              "get another tpidBmp than was set: tpidBmp = %d, tpidBmpGet = %d",
                                         tpidBmp, tpidBmpGet);

             if(GT_TRUE == PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_ENABLED_MAC(dev))
            {
                st = cpssDxChBrgVlanPortIngressTpidSet(dev, port,
                                     CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_INGRESS_PASSENGER_ETHERTYPE0_E, tpidBmp);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /* Get value of tpidBmp*/
                st = cpssDxChBrgVlanPortIngressTpidGet(dev, port,
                                  CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_INGRESS_PASSENGER_ETHERTYPE0_E, &tpidBmpGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /* Check if we get the same tpidBmp*/
                UTF_VERIFY_EQUAL2_STRING_MAC(tpidBmp, tpidBmpGet,
                  "get another tpidBmp than was set: tpidBmp = %d, tpidBmpGet = %d",
                                             tpidBmp, tpidBmpGet);

                 st = cpssDxChBrgVlanPortIngressTpidSet(dev, port,
                                     CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_INGRESS_PASSENGER_ETHERTYPE1_E, tpidBmp);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /* Get value of tpidBmp*/
                st = cpssDxChBrgVlanPortIngressTpidGet(dev, port,
                                  CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_INGRESS_PASSENGER_ETHERTYPE1_E, &tpidBmpGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /* Check if we get the same tpidBmp*/
                UTF_VERIFY_EQUAL2_STRING_MAC(tpidBmp, tpidBmpGet,
                  "get another tpidBmp than was set: tpidBmp = %d, tpidBmpGet = %d",
                                             tpidBmp, tpidBmpGet);
            }

            /*
                1.2. Call function with incorrect
                     portNum = BRG_VLAN_INVALID_PORT_CNS(dev).
                Expected: GT_BAD_PARAM.
            */
            st = cpssDxChBrgVlanPortIngressTpidSet(dev,
                 BRG_VLAN_INVALID_PORT_CNS(dev), CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E, tpidBmp);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            /*
                1.3. Call function with wrong enum values ethMode.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanPortIngressTpidSet
                                (dev, port, ethMode, tpidBmp),
                                ethMode);

            /*
                1.4. Call function with incorrect tpidBmpPtr (BIT_8).
                Expected: GT_OUT_OF_RANGE.
            */
            st = cpssDxChBrgVlanPortIngressTpidSet(dev, port,
                                     CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E, BIT_8);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortIngressTpidSet(dev, port,
                                     CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E, tpidBmp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortIngressTpidSet(dev, port,
                             CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E, tpidBmp);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortIsolationCmdSet
(
    IN  GT_U8                                      devNum,
    IN  GT_U16                                     vlanId,
    IN  CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_CMD_ENT  cmd
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortIsolationCmdSet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call function with correct parameters and cmd =
                  [ CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_DISABLE_CMD_E /
                    CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_L3_CMD_E ]
    Expected: GT_OK.
    1.2. Call function with incorrect vlanId = BRG_VLAN_INVALID_VLAN_ID_CNS.
    Expected: GT_BAD_PARAM.
    1.3. Call function with wrong enum values cmd.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;

    GT_U8       dev;
    CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_CMD_ENT cmd = CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_L2_CMD_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with correct parameters and cmd =
                          [ CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_DISABLE_CMD_E /
                            CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_L3_CMD_E ]
            Expected: GT_OK.
        */
        st = cpssDxChBrgVlanPortIsolationCmdSet(dev, BRG_VLAN_TESTED_VLAN_ID_CNS,
                                 CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_DISABLE_CMD_E);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChBrgVlanPortIsolationCmdSet(dev, BRG_VLAN_TESTED_VLAN_ID_CNS,
                                      CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_L3_CMD_E);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call function with incorrect vlanId = BRG_VLAN_INVALID_VLAN_ID_CNS.
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChBrgVlanPortIsolationCmdSet(dev, BRG_VLAN_INVALID_VLAN_ID_CNS,
                                       CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_L3_CMD_E);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /*
            1.3. Call function with wrong enum values cmd.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanPortIsolationCmdSet
                                                        (dev, BRG_VLAN_TESTED_VLAN_ID_CNS, cmd),
                                                        cmd);
    }

    cmd = CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_L3_CMD_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortIsolationCmdSet(dev, BRG_VLAN_TESTED_VLAN_ID_CNS,
                                 CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_DISABLE_CMD_E);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortIsolationCmdSet(dev, BRG_VLAN_TESTED_VLAN_ID_CNS,
                             CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_DISABLE_CMD_E);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortVidPrecedenceGet
(
    IN  GT_U8                                         devNum,
    IN  GT_U8                                         port,
    OUT CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT  *precedencePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortVidPrecedenceGet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call function with correct parameters.
    Expected: GT_OK.
    1.2. Call function with incorrect portNum = BRG_VLAN_INVALID_PORT_CNS(dev).
    Expected: GT_BAD_PARAM.
    1.3. Call function with incorrect precedencePtr (NULL).
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT      precedencePtr;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with correct parameters.
            Expected: GT_OK.
        */
        st = cpssDxChBrgVlanPortVidPrecedenceGet(dev, BRG_VLAN_VALID_PHY_PORT_CNS,
                                                 &precedencePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call function with incorrect portNum = BRG_VLAN_INVALID_PORT_CNS(dev).
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChBrgVlanPortVidPrecedenceGet(dev, BRG_VLAN_INVALID_PORT_CNS(dev),
                                                 &precedencePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /*
            1.3. Call function with incorrect precedencePtr (NULL).
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgVlanPortVidPrecedenceGet(dev, BRG_VLAN_VALID_PHY_PORT_CNS, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortVidPrecedenceGet(dev, BRG_VLAN_VALID_PHY_PORT_CNS,
                                                 &precedencePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortVidPrecedenceGet(dev, BRG_VLAN_VALID_PHY_PORT_CNS,
                                             &precedencePtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanRangeGet
(
    IN  GT_U8   devNum,
    OUT GT_U16  *vidRangePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanRangeGet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call function with correct parameters.
    Expected: GT_OK.
    1.2. Call function with incorrect vidRangePtr (NULL).
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U16      vidRangePtr;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with correct parameters.
            Expected: GT_OK.
        */
        st = cpssDxChBrgVlanRangeGet(dev, &vidRangePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call function with incorrect vidRangePtr (NULL).
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgVlanRangeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanRangeGet(dev, &vidRangePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanRangeGet(dev, &vidRangePtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanRangeSet
(
    IN  GT_U8   devNum,
    IN  GT_U16  vidRange
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanRangeSet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call function with correct parameters.
    Expected: GT_OK and the same vidRange.
    1.2. Call function with incorrect vidRange = UTF_CPSS_PP_MAX_VLAN_NUM_CNS(dev).
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U16      vidRange    = 1;
    GT_U16      vidRangeGet = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with correct parameters and
                 ethMode = CPSS_VLAN_ETHERTYPE0_E.
            Expected: GT_OK and the same vidRange.
        */
        st = cpssDxChBrgVlanRangeSet(dev, vidRange);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Get value of vidRange*/
        st = cpssDxChBrgVlanRangeGet(dev, &vidRangeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Check if we get the same vidRange*/
        UTF_VERIFY_EQUAL2_STRING_MAC(vidRange, vidRangeGet,
          "get another vidRange than was set: vidRange = %d, vidRangeGet = %d",
                                     vidRange, vidRangeGet);
        /*
            1.2. Call function with incorrect
                 vidRangePtr (UTF_CPSS_PP_MAX_VLAN_NUM_CNS(dev)).
            Expected: GT_OUT_OF_RANGE.
        */
        st = cpssDxChBrgVlanRangeSet(dev, (GT_U16)UTF_CPSS_PP_MAX_VLAN_NUM_CNS(dev));
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanRangeSet(dev, vidRange);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanRangeSet(dev, vidRange);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanTpidEntryGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_ETHERTYPE_TABLE_ENT   tableType,
    IN  GT_U32                          entryIndex,
    OUT GT_U16                          *etherTypePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanTpidEntryGet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call function with correct parameters: tableType =
                   [    CPSS_DXCH_ETHERTYPE_TABLE_INGRESS_E/
                        CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_E/
                        CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_LOGICAL_PORT_E/
                        CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_TS_E ].
    Expected: GT_OK.
    1.2. Call function with wrong enum values tableType.
    Expected: GT_BAD_PARAM.
    1.3. Call function with incorrect entryIndex = 8.
    Expected: GT_BAD_PARAM.
    1.4. Call function with incorrect etherTypePtr (NULL).
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;

    GT_U8       dev;
    CPSS_DXCH_ETHERTYPE_TABLE_ENT  tableType = CPSS_DXCH_ETHERTYPE_TABLE_INGRESS_E;
    GT_U16      etherTypePtr;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with correct parameters: tableType =
                             [  CPSS_DXCH_ETHERTYPE_TABLE_INGRESS_E/
                                CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_E/
                                CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_LOGICAL_PORT_E/
                                CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_TS_E ].
            Expected: GT_OK.
        */
        st = cpssDxChBrgVlanTpidEntryGet(dev, CPSS_DXCH_ETHERTYPE_TABLE_INGRESS_E,
                                         0, &etherTypePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChBrgVlanTpidEntryGet(dev, CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_E,
                                         0, &etherTypePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            st = cpssDxChBrgVlanTpidEntryGet(dev, CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_LOGICAL_PORT_E,
                                             0, &etherTypePtr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            st = cpssDxChBrgVlanTpidEntryGet(dev, CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_TS_E,
                                             0, &etherTypePtr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        /*
            1.2. Call function with wrong enum values tableType.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanTpidEntryGet
                                                        (dev, tableType, 0, &etherTypePtr),
                                                        tableType);

        /*
            1.3. Call function with incorrect entryIndex = 8.
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChBrgVlanTpidEntryGet(dev, CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E,
                                         8, &etherTypePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /*
            1.4. Call function with incorrect etherTypePtr (NULL).
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgVlanTpidEntryGet(dev, CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E,
                                         0, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanTpidEntryGet(dev, CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E,
                                         0, &etherTypePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanTpidEntryGet(dev, CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E,
                                     0, &etherTypePtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanTpidEntrySet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_ETHERTYPE_TABLE_ENT   tableType,
    IN  GT_U32                          entryIndex,
    IN  GT_U16                          etherType
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanTpidEntrySet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call function with correct parameters: tableType =
                   [    CPSS_DXCH_ETHERTYPE_TABLE_INGRESS_E/
                        CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_E/
                        CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_LOGICAL_PORT_E/
                        CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_TS_E ].
    Expected: GT_OK and the same etherType.
    1.2. Call function with wrong enum values tableType.
    Expected: GT_BAD_PARAM.
    1.3. Call function with incorrect entryIndex = 8.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st           = GT_OK;

    GT_U8       dev;
    CPSS_DXCH_ETHERTYPE_TABLE_ENT  tableType = CPSS_DXCH_ETHERTYPE_TABLE_INGRESS_E;
    GT_U16      etherType    = 1;
    GT_U16      etherTypeGet = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with correct parameters: tableType =
                   [    CPSS_DXCH_ETHERTYPE_TABLE_INGRESS_E/
                        CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_E/
                        CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_LOGICAL_PORT_E/
                        CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_TS_E ].
            Expected: GT_OK and the same etherType.
        */
        st = cpssDxChBrgVlanTpidEntrySet(dev, CPSS_DXCH_ETHERTYPE_TABLE_INGRESS_E, 0, etherType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Get value of etherType*/
        st = cpssDxChBrgVlanTpidEntryGet(dev, CPSS_DXCH_ETHERTYPE_TABLE_INGRESS_E, 0, &etherTypeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Check if we get the same etherType*/
        UTF_VERIFY_EQUAL2_STRING_MAC(etherType, etherTypeGet,
               "get another etherType than was set: etherType = %d, etherTypeGet = %d",
                                     etherType, etherTypeGet);

        st = cpssDxChBrgVlanTpidEntrySet(dev, CPSS_DIRECTION_EGRESS_E, 0, etherType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Get value of etherType*/
        st = cpssDxChBrgVlanTpidEntryGet(dev, CPSS_DIRECTION_EGRESS_E, 0, &etherTypeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Check if we get the same etherType*/
        UTF_VERIFY_EQUAL2_STRING_MAC(etherType, etherTypeGet,
               "get another etherType than was set: etherType = %d, etherTypeGet = %d",
                                     etherType, etherTypeGet);

        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            st = cpssDxChBrgVlanTpidEntrySet(dev, CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_LOGICAL_PORT_E, 0, etherType);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* Get value of etherType*/
            st = cpssDxChBrgVlanTpidEntryGet(dev, CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_LOGICAL_PORT_E, 0, &etherTypeGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* Check if we get the same etherType*/
            UTF_VERIFY_EQUAL2_STRING_MAC(etherType, etherTypeGet,
                   "get another etherType than was set: etherType = %d, etherTypeGet = %d",
                                         etherType, etherTypeGet);

            st = cpssDxChBrgVlanTpidEntrySet(dev, CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_TS_E, 0, etherType);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* Get value of etherType*/
            st = cpssDxChBrgVlanTpidEntryGet(dev, CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_TS_E, 0, &etherTypeGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* Check if we get the same etherType*/
            UTF_VERIFY_EQUAL2_STRING_MAC(etherType, etherTypeGet,
                   "get another etherType than was set: etherType = %d, etherTypeGet = %d",
                                         etherType, etherTypeGet);

        }
            /*
            1.2. Call function with wrong enum values tableType.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanTpidEntrySet
                                                        (dev, tableType, 0, etherType),
                                                        tableType);

        /*
            1.3. Call function with incorrect entryIndex = 8.
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChBrgVlanTpidEntrySet(dev, CPSS_DXCH_ETHERTYPE_TABLE_INGRESS_E, 8, etherType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanTpidEntrySet(dev, CPSS_DXCH_ETHERTYPE_TABLE_INGRESS_E, 0, etherType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanTpidEntrySet(dev, CPSS_DXCH_ETHERTYPE_TABLE_INGRESS_E, 0, etherType);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanInit
(
    IN GT_U8 devNum
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanInit)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with correct dev.
    Expected: GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with correct dev.
            Expected: GT_OK.
        */
        st = cpssDxChBrgVlanInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanRemoveVlanTag1IfZeroModeSet
(
    IN  GT_U8                                               devNum,
    IN  CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_MODE_ENT     mode
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanRemoveVlanTag1IfZeroModeSet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call with mode [CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_DISABLE_E /
                         CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_NOT_DOUBLE_TAG_E /
                         CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_E].
    Expected: GT_OK.
    1.2. Call with wrong enum values mode.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;
    GT_STATUS   expectedSt;
    GT_U8       dev;
    CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_MODE_ENT mode =
                            CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_DISABLE_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with mode [CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_DISABLE_E /
                                 CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_NOT_DOUBLE_TAG_E /
                                 CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_E].
            Expected: GT_OK.
        */

        if( (CPSS_PP_FAMILY_DXCH_LION_E == PRV_CPSS_PP_MAC(dev)->devFamily) &&
            ( PRV_CPSS_PP_MAC(dev)->revision <= 2 ))
        {
            expectedSt = GT_NOT_APPLICABLE_DEVICE;
        }
        else
        {
            expectedSt = GT_OK;
        }

        /* Call with mode [CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_DISABLE_E] */
        mode = CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_DISABLE_E;

        st = cpssDxChBrgVlanRemoveVlanTag1IfZeroModeSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expectedSt, st, dev, mode);

        /* Call with mode [CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_NOT_DOUBLE_TAG_E] */
        mode = CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_NOT_DOUBLE_TAG_E;

        st = cpssDxChBrgVlanRemoveVlanTag1IfZeroModeSet(dev, mode);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, mode);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(expectedSt, st, dev, mode);
        }

        /* Call with mode [CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_AND_INGRESS_WITHOUT_TAG1_E] */
        mode = CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_AND_INGRESS_WITHOUT_TAG1_E;

        st = cpssDxChBrgVlanRemoveVlanTag1IfZeroModeSet(dev, mode);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);
        }
        else
        if(expectedSt == GT_OK)
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, mode);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(expectedSt, st, dev, mode);
        }

        /* Call with mode [CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_E] */
        mode = CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_E;

        st = cpssDxChBrgVlanRemoveVlanTag1IfZeroModeSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expectedSt, st, dev, mode);

        /*
            1.2. Call with wrong enum values mode.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanRemoveVlanTag1IfZeroModeSet
                                                        (dev, mode),
                                                        mode);
    }

    mode = CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanRemoveVlanTag1IfZeroModeSet(dev, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanRemoveVlanTag1IfZeroModeSet(dev, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanRemoveVlanTag1IfZeroModeGet
(
    IN  GT_U8                                               devNum,
    OUT CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_MODE_ENT     *modePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanRemoveVlanTag1IfZeroModeGet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call with not null mode.
    Expected: GT_OK.
    1.2. Call with wrong modePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;

    GT_U8       dev;
    CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_MODE_ENT mode;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not null mode.
            Expected: GT_OK.
        */

        st = cpssDxChBrgVlanRemoveVlanTag1IfZeroModeGet(dev, &mode);
        if( (CPSS_PP_FAMILY_DXCH_LION_E == PRV_CPSS_PP_MAC(dev)->devFamily) &&
            ( PRV_CPSS_PP_MAC(dev)->revision <= 2 ))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, &mode);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, &mode);
        }

        /*
            1.2. Call with wrong modePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgVlanRemoveVlanTag1IfZeroModeGet(dev, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, &mode);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanRemoveVlanTag1IfZeroModeGet(dev, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanRemoveVlanTag1IfZeroModeGet(dev, &mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanForceNewDsaToCpuEnableSet
(
    IN  GT_U8   dev,
    IN  GT_BOOL enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanForceNewDsaToCpuEnableSet)
{
/*
    ITERATE_DEVICES (Lion and above)
    1.1 Call function with enable [GT_TRUE / GT_FALSE].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgVlanForceNewDsaToCpuEnableGet.
    Expected: GT_OK and the same enable than was set.
*/
    GT_STATUS   st       = GT_OK;
    GT_U8       dev;
    GT_BOOL     state    = GT_TRUE;
    GT_BOOL     stateGet = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1 Call function with enable [GT_TRUE / GT_FALSE].
            Expected: GT_OK.
        */
        /* call with state = GT_TRUE; */
        state = GT_TRUE;

        st = cpssDxChBrgVlanForceNewDsaToCpuEnableSet(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call cpssDxChBrgVlanForceNewDsaToCpuEnableGet.
            Expected: GT_OK and the same enable than was set.
        */
        st = cpssDxChBrgVlanForceNewDsaToCpuEnableGet(dev, &stateGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgVlanForceNewDsaToCpuEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                               "get another state than was set: %d", dev);

        /* call with state = GT_FALSE; */
        state = GT_FALSE;

        st = cpssDxChBrgVlanForceNewDsaToCpuEnableSet(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call cpssDxChBrgVlanForceNewDsaToCpuEnableGet.
            Expected: GT_OK and the same enable than was set.
        */
        st = cpssDxChBrgVlanForceNewDsaToCpuEnableGet(dev, &stateGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgVlanForceNewDsaToCpuEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                               "get another state than was set: %d", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanForceNewDsaToCpuEnableSet(dev, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanForceNewDsaToCpuEnableSet(dev, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanForceNewDsaToCpuEnableGet
(
    IN  GT_U8   dev,
    OUT GT_BOOL *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanForceNewDsaToCpuEnableGet)
{
/*
    ITERATE_DEVICES (Lion and above)
    1.1. Call with not null enablePtr.
    Expected: GT_OK.
    1.2. Call with incorrect enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;

    GT_U8       dev;
    GT_BOOL     state;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not null enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgVlanForceNewDsaToCpuEnableGet(dev, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call with incorrect enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgVlanForceNewDsaToCpuEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, state);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanForceNewDsaToCpuEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanForceNewDsaToCpuEnableGet(dev, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}



/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanKeepVlan1EnableSet
(
    IN  GT_U8       devNum,
    IN  GT_U8       portNum,
    IN  GT_U8       up,
    IN  GT_BOOL     enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanKeepVlan1EnableSet)
{
/*
    ITERATE_DEVICES (Lion and above)
    1.1 Call function with enable [GT_TRUE / GT_FALSE].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgVlanKeepVlan1EnableSet.
    Expected: GT_OK and the same enable than was set.
    1.3 Call function with wrong portNum [CPSS_CPU_PORT_NUM_CNS, UTF_CPSS_PP_MAX_PORTS_NUM_CNS].
    Expected: GT_BAD_PARAM.
    1.4 Call function with wrong up [8].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st       = GT_OK;
    GT_U8       dev;
    GT_PORT_NUM       portNum;
    GT_U8       up;
    GT_BOOL     state    = GT_TRUE;
    GT_BOOL     stateGet = GT_FALSE;
    CPSS_DXCH_CFG_DEV_INFO_STC   devInfo;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.0 Get device info */
        st = cpssDxChCfgDevInfoGet(dev, &devInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.1 Call function with portNum = 0, up = 4, enable [GT_TRUE / GT_FALSE].
            Expected: GT_OK.
        */
        /* call with state = GT_TRUE; */
        state = GT_TRUE;
        portNum = 0;
        up = 4;

        st = cpssDxChBrgVlanKeepVlan1EnableSet(dev, portNum, up, state);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, portNum, up, state);

        /*
            1.2. Call cpssDxChBrgVlanKeepVlan1EnableGet.
            Expected: GT_OK and the same enable than was set.
        */
        st = cpssDxChBrgVlanKeepVlan1EnableGet(dev, portNum, up, &stateGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgVlanKeepVlan1EnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                               "get another state than was set: %d", dev);

        /* call with state = GT_FALSE; */
        state = GT_FALSE;

        st = cpssDxChBrgVlanKeepVlan1EnableSet(dev, portNum, up, state);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, portNum, up, state);

        /*
            1.2. Call cpssDxChBrgVlanForceNewDsaToCpuEnableGet.
            Expected: GT_OK and the same enable than was set.
        */
        st = cpssDxChBrgVlanKeepVlan1EnableGet(dev, portNum, up, &stateGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgVlanKeepVlan1EnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                               "get another state than was set: %d", dev);

        /*
            1.1 Call function with portNum = 32, up = 7, enable = GT_TRUE.
            Expected: GT_OK.
        */
        state = GT_TRUE;
        if (devInfo.genDevInfo.maxPortNum > 32)
        {
            portNum = 32;
        }
        else
        {
            portNum = (devInfo.genDevInfo.maxPortNum / 2);
        }

        up = 7;

        st = cpssDxChBrgVlanKeepVlan1EnableSet(dev, portNum, up, state);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, portNum, up, state);

        /*
            1.2. Call cpssDxChBrgVlanKeepVlan1EnableGet.
            Expected: GT_OK and the same enable than was set.
        */
        st = cpssDxChBrgVlanKeepVlan1EnableGet(dev, portNum, up, &stateGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgVlanKeepVlan1EnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                               "get another state than was set: %d", dev);
        /*
            1.1 Call function with portNum = 54, up = 0, enable = GT_FALSE.
            Expected: GT_OK.
        */
        state = GT_TRUE;
        if (devInfo.genDevInfo.maxPortNum > 54)
        {
            portNum = 54;
        }
        else
        {
            portNum = ((devInfo.genDevInfo.maxPortNum / 2) + (devInfo.genDevInfo.maxPortNum / 4));
        }

        up = 0;

        st = cpssDxChBrgVlanKeepVlan1EnableSet(dev, portNum, up, state);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, portNum, up, state);

        /*
            1.2. Call cpssDxChBrgVlanKeepVlan1EnableGet.
            Expected: GT_OK and the same enable than was set.
        */
        st = cpssDxChBrgVlanKeepVlan1EnableGet(dev, portNum, up, &stateGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgVlanKeepVlan1EnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                               "get another state than was set: %d", dev);


        /* 1.3 Call function with wrong portNum [for CPU port number, UTF_CPSS_PP_MAX_PORTS_NUM_CNS].
           Expected: GT_BAD_PARAM.
        */
        state = GT_TRUE;
        portNum = CPSS_CPU_PORT_NUM_CNS;
        up = 0;

        st = cpssDxChBrgVlanKeepVlan1EnableSet(dev, portNum, up, state);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, portNum, up, state);
        }
        else
        {
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum, up, state);
        }

        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChBrgVlanKeepVlan1EnableSet(dev, portNum, up, state);
        if(IS_E_ARCH_AND_PHYSICAL_PORT_IN_RANGE_MAC(dev,portNum))
        {
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, portNum, up, state);
        }
        else
        {
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum, up, state);
        }

        /* 1.4 Call function with wrong up [8].
           Expected: GT_BAD_PARAM.
        */
        portNum = 0;
        up = 8;

        st = cpssDxChBrgVlanKeepVlan1EnableSet(dev, portNum, up, state);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);

    portNum = 0;
    up = 0;

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanKeepVlan1EnableSet(dev, portNum, up, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanKeepVlan1EnableSet(dev, portNum, up, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanKeepVlan1EnableGet
(
    IN  GT_U8       devNum,
    IN  GT_U8       portNum,
    IN  GT_U8       up,
    OUT GT_BOOL     *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanKeepVlan1EnableGet)
{
/*
    ITERATE_DEVICES (Lion and above)
    1.1 Call function with valid parameters.
    Expected: GT_OK.
    1.2 Call function with wrong portNum [UTF_CPSS_PP_MAX_PORTS_NUM_CNS].
    Expected: GT_BAD_PARAM.
    1.3 Call function with wrong up [8].
    Expected: GT_BAD_PARAM.
    1.4 Call function with Null pointer.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st       = GT_OK;
    GT_U8       dev;
    GT_PORT_NUM       portNum = 0;
    GT_U8       up = 0;
    GT_BOOL     state;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with valid parameters.
            Expected: GT_OK.
        */
        portNum = 10;
        up = 2;

        st = cpssDxChBrgVlanKeepVlan1EnableGet(dev, portNum, up, &state);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, portNum, up, state);

        /*
            1.2. Call function with wrong portNum [UTF_CPSS_PP_MAX_PORTS_NUM_CNS].
            Expected: GT_BAD_PARAM.
        */
        CPSS_TBD_BOOKMARK_EARCH /* remove casting */
        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChBrgVlanKeepVlan1EnableGet(dev, portNum, up, &state);
        if(IS_E_ARCH_AND_PHYSICAL_PORT_IN_RANGE_MAC(dev,portNum))
        {
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, portNum, up, state);
        }
        else
        {
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum, up, state);
        }

        /*
            1.3. Call function with wrong up [8].
            Expected: GT_BAD_PARAM.
        */
        portNum = 20;
        up = 8;

        st = cpssDxChBrgVlanKeepVlan1EnableGet(dev, portNum, up, &state);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum, up, state);

        /*
            1.4. Call function with NULL pointer.
            Expected: GT_BAD_PTR.
        */
        portNum = 20;
        up = 7;

        st = cpssDxChBrgVlanKeepVlan1EnableGet(dev, portNum, up, NULL);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PTR, st, dev, portNum, up, state);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanKeepVlan1EnableGet(dev, portNum, up, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanKeepVlan1EnableGet(dev, portNum, up, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
    Test function to Fill Vlan table.
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanFillVlanTable)
{
/*
    ITERATE_DEVICE (DxChx)
    1.1. Get table Size.
         Call cpssDxChCfgTableNumEntriesGet with table [CPSS_DXCH_CFG_TABLE_VLAN_E]
                                                 and non-NULL numEntriesPtr.
    Expected: GT_OK.
    1.2. Fill all entries in Vlan table.
            Call cpssDxChBrgVlanEntryWrite with vlanId [0..numEntries-1],
                               portsMemberPtr {[0, 16]},
                               portsTaggingPtr {[0, 16]},
                               vlanInfoPtr {unkSrcAddrSecBreach [GT_FALSE],
                                            unregNonIpMcastCmd [CPSS_PACKET_CMD_FORWARD_E],
                                            unregIpv4McastCmd [CPSS_PACKET_CMD_MIRROR_TO_CPU_E],
                                            unregIpv6McastCmd [CPSS_PACKET_CMD_TRAP_TO_CPU_E],
                                            unkUcastCmd [CPSS_PACKET_CMD_DROP_HARD_E],
                                            unregIpv4BcastCmd [CPSS_PACKET_CMD_DROP_SOFT_E],
                                            unregNonIpv4BcastCmd [CPSS_PACKET_CMD_FORWARD_E],
                                            ipv4IgmpToCpuEn [GT_TRUE],
                                            mirrToRxAnalyzerEn [GT_TRUE],
                                            ipv6IcmpToCpuEn[GT_TRUE],
                                            ipCtrlToCpuEn [CPSS_DXCH_BRG_IP_CTRL_NONE_E],
                                            ipv4IpmBrgMode [CPSS_BRG_IPM_SGV_E],
                                            ipv6IpmBrgMode [CPSS_BRG_IPM_GV_E],
                                            ipv4IpmBrgEn[GT_TRUE],
                                            ipv6IpmBrgEn [GT_TRUE],
                                            ipv6SiteIdMode [CPSS_IP_SITE_ID_INTERNAL_E],
                                            ipv4UcastRouteEn[GT_FALSE],
                                            ipv4McastRouteEn [GT_FALSE],
                                            ipv6UcastRouteEn[GT_FALSE],
                                            ipv6McastRouteEn[GT_FALSE],
                                            stgId[0],
                                            autoLearnDisable [GT_FALSE],
                                            naMsgToCpuEn [GT_FALSE],
                                            mruIdx[0],
                                            bcastUdpTrapMirrEn [GT_FALSE],
                                            vrfId [0]}.
    Expected: GT_OK.
    1.3. Try to write entry with index out of range.
            Call cpssDxChBrgVlanEntryWrite with vlanId [numEntries] and other params from 1.2.
    Expected: NOT GT_OK.
    1.4. Read all entries in Vlan table and compare with original.
         Call cpssDxChBrgVlanEntryRead with the same vlanId.
    Expected: GT_OK and the same portsMembersPtr, portsTaggingPtr and vlanInfoPtr.
    Check vlanInfo by fields:
                1) skip ipv6SiteIdMode, ipv4McastRateEn, ipv6McastRateEn,
                   autoLearnDisable, naMsgToCpuEn, mruIdx, bcastUdpTrapMirrEn
                   fields for Cheetah, but they should be checked for DxCh2, DxCh3;
                2) ipv4IpmBrgMode and ipv6IpnBrgMode should be checked only if
                   corresponding ipv4/6IpmBrgEn = GT_TRUE.
    1.5. Try to read entry with index out of range.
         Call cpssDxChBrgVlanEntryRead with the same vlanId [numEntries].
    Expected: NOT GT_OK.
    1.6. Delete all entries in Vlan table.
         Call cpssDxChBrgVlanEntryInvalidate with the same vlanId to invalidate all changes.
    Expected: GT_OK.
    1.7. Try to delete entry with index out of range.
         Call cpssDxChBrgVlanEntryInvalidate with the same vlanId [numEntries].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32      numEntries = 0;
    GT_U16      iTemp      = 0;
    GT_U16               step = 1;
    GT_U32   gmFillMaxIterations = 128;

    CPSS_PORTS_BMP_STC          portsMembers;
    CPSS_PORTS_BMP_STC          portsTagging;
    CPSS_DXCH_BRG_VLAN_INFO_STC vlanInfo;

    CPSS_PORTS_BMP_STC          portsMembersGet;
    CPSS_PORTS_BMP_STC          portsTaggingGet;
    CPSS_DXCH_BRG_VLAN_INFO_STC vlanInfoGet;

    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC  portsTaggingCmd;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC  *portsTaggingCmdPtr;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC  portsTaggingCmdGet;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC  *portsTaggingCmdGetPtr;

    GT_BOOL                     isValid = GT_FALSE;
    GT_BOOL                     isEqual = GT_FALSE;

    CPSS_PP_FAMILY_TYPE_ENT     devFamily;

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    cpssOsBzero((GT_VOID*) &vlanInfo, sizeof(vlanInfo));
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);
    cpssOsBzero((GT_VOID*) &portsTaggingCmd, sizeof(portsTaggingCmd));
    cpssOsBzero((GT_VOID*) &vlanInfoGet, sizeof(vlanInfoGet));
    cpssOsBzero((GT_VOID*) &portsMembersGet, sizeof(portsMembersGet));
    cpssOsBzero((GT_VOID*) &portsTaggingGet, sizeof(portsTaggingGet));
    cpssOsBzero((GT_VOID*) &portsTaggingCmdGet, sizeof(portsTaggingCmdGet));

    /* Fill the entry for vlan table */
    portsMembers.ports[0] = 17;
    portsTagging.ports[0] = 17;

    vlanInfo.unkSrcAddrSecBreach  = GT_FALSE;
    vlanInfo.unregNonIpMcastCmd   = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregIpv4McastCmd    = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
    vlanInfo.unregIpv6McastCmd    = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    vlanInfo.unkUcastCmd          = CPSS_PACKET_CMD_DROP_HARD_E;
    vlanInfo.unregIpv4BcastCmd    = CPSS_PACKET_CMD_DROP_SOFT_E;
    vlanInfo.unregNonIpv4BcastCmd = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.ipv4IgmpToCpuEn      = GT_TRUE;
    vlanInfo.mirrToRxAnalyzerEn   = GT_TRUE;
    vlanInfo.ipv6IcmpToCpuEn      = GT_TRUE;
    vlanInfo.ipCtrlToCpuEn        = CPSS_DXCH_BRG_IP_CTRL_NONE_E;
    vlanInfo.ipv4IpmBrgMode       = CPSS_BRG_IPM_SGV_E;
    vlanInfo.ipv6IpmBrgMode       = CPSS_BRG_IPM_GV_E;
    vlanInfo.ipv4IpmBrgEn         = GT_TRUE;
    vlanInfo.ipv6IpmBrgEn         = GT_TRUE;
    vlanInfo.ipv6SiteIdMode       = CPSS_IP_SITE_ID_INTERNAL_E;
    vlanInfo.ipv4UcastRouteEn     = GT_FALSE;
    vlanInfo.ipv4McastRouteEn     = GT_FALSE;
    vlanInfo.ipv6UcastRouteEn     = GT_FALSE;
    vlanInfo.ipv6McastRouteEn     = GT_FALSE;
    vlanInfo.stgId                = 0;
    vlanInfo.autoLearnDisable     = GT_FALSE;
    vlanInfo.naMsgToCpuEn         = GT_FALSE;
    vlanInfo.mruIdx               = 0;
    vlanInfo.bcastUdpTrapMirrEn   = GT_FALSE;
    vlanInfo.vrfId                = 100;

    vlanInfo.mirrToRxAnalyzerIndex = 1;
    vlanInfo.mirrToTxAnalyzerEn = GT_TRUE;
    vlanInfo.mirrToTxAnalyzerIndex = 2;
    vlanInfo.mirrToTxAnalyzerMode = CPSS_DXCH_MIRROR_EGRESS_TAIL_DROP_E;
    vlanInfo.fidValue = 0;
    vlanInfo.unknownMacSaCmd = CPSS_PACKET_CMD_DROP_HARD_E;
    vlanInfo.ipv4McBcMirrToAnalyzerEn = GT_TRUE;
    vlanInfo.ipv4McBcMirrToAnalyzerIndex = 3;
    vlanInfo.ipv6McMirrToAnalyzerEn = GT_TRUE;
    vlanInfo.ipv6McMirrToAnalyzerIndex = 4;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Getting device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* 1.1. Get table Size */
        st = cpssDxChCfgTableNumEntriesGet(dev, CPSS_DXCH_CFG_TABLE_VLAN_E, &numEntries);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChCfgTableNumEntriesGet: %d", dev);

        if (IS_TR101_SUPPORTED_MAC(dev))
        {
            /* portsTaggingCmd relevant for TR101 enabled devices */
            portsTaggingCmd.portsCmd[0] = CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E;
            portsTaggingCmd.portsCmd[1] = CPSS_DXCH_BRG_VLAN_PORT_TAG1_CMD_E;
            portsTaggingCmd.portsCmd[2] = CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E;
            portsTaggingCmd.portsCmd[3] = CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E;
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            {
                portsTaggingCmd.portsCmd[8] = CPSS_DXCH_BRG_VLAN_PORT_DO_NOT_MODIFY_TAG_CMD_E;
            }
            else
            {
                portsTaggingCmd.portsCmd[8] = CPSS_DXCH_BRG_VLAN_PORT_PUSH_TAG0_CMD_E;
            }
            portsTaggingCmd.portsCmd[9] = CPSS_DXCH_BRG_VLAN_PORT_POP_OUTER_TAG_CMD_E;
            portsTaggingCmd.portsCmd[17] = CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E;
            portsTaggingCmd.portsCmd[22] = CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E;

            /* port tagging CMD is used */
            portsTaggingCmdPtr = &portsTaggingCmd;
            portsTaggingCmdGetPtr = &portsTaggingCmdGet;
        }
        else
        {
            /* port tagging CMD should be ignored */
            /* coverity[assign_zero] */ portsTaggingCmdPtr = portsTaggingCmdGetPtr = NULL;
        }

        if (IS_NEEDED_TO_REDUCE_ITERATIONS_DUE_TO_POOR_PERFORMANCE_MAC || GT_TRUE == prvUtfReduceLogSizeFlagGet())
        {
            /* get new step that will match 'max iterations' for GM*/
            step = (GT_U16)prvUtfIterationReduce(dev,numEntries,gmFillMaxIterations,step);
        }

        /* 1.2. Fill all entries in vlan table */
        for(iTemp = 0; iTemp < numEntries; iTemp+=step)
        {
            CPSS_COVERITY_NON_ISSUE_BOOKMARK
            /* coverity[var_deref_model] */
            st = cpssDxChBrgVlanEntryWrite(dev, iTemp, &portsMembers, &portsTagging,
                                           &vlanInfo, portsTaggingCmdPtr);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgVlanEntryWrite: %d, %d", dev, iTemp);
        }

        /* 1.3. Try to write entry with index out of range. */
        CPSS_COVERITY_NON_ISSUE_BOOKMARK
        /* coverity[var_deref_model] */
        st = cpssDxChBrgVlanEntryWrite(dev, (GT_U16)numEntries, &portsMembers,
                                       &portsTagging, &vlanInfo, portsTaggingCmdPtr);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgVlanEntryWrite: %d, %d", dev, numEntries);

        /* 1.4. Read all entries in vlan table and compare with original */
        for(iTemp = 0; iTemp < numEntries; iTemp+=step)
        {
            st = cpssDxChBrgVlanEntryRead(dev, iTemp, &portsMembersGet, &portsTaggingGet,
                                            &vlanInfoGet, &isValid, portsTaggingCmdGetPtr);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgVlanEntryRead: %d, %d", dev, iTemp);

            UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isValid,
                    "Vlan entry is NOT valid: %d, %d", dev, iTemp);

            if (GT_TRUE == isValid)
            {
                /* Verifying portsMembersPtr */
                isEqual = (0 == cpssOsMemCmp((GT_VOID*) &portsMembers,
                                             (GT_VOID*) &portsMembersGet,
                                             sizeof(portsMembers))) ? GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
                           "get another portsMembersPtr than was set: %d, %d", dev, iTemp);

                if (IS_TR101_SUPPORTED_MAC(dev))
                {
                    /* Verifying portsTaggingCmd */
                    isEqual = (0 == cpssOsMemCmp((GT_VOID*) &portsTaggingCmd,
                                                 (GT_VOID*) &portsTaggingCmdGet,
                                                 sizeof(portsTaggingCmd))) ? GT_TRUE : GT_FALSE;
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
                               "get another portsTaggingCmd than was set: %d, %d", dev, iTemp);
                }
                else
                {
                    /* Verifying portsTaggingPtr */
                    isEqual = (0 == cpssOsMemCmp((GT_VOID*) &portsTagging,
                                                 (GT_VOID*) &portsTaggingGet,
                                                 sizeof(portsTagging))) ? GT_TRUE : GT_FALSE;
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
                               "get another portsTaggingPtr than was set: %d, %d", dev, iTemp);
                }

                /* Verifying vlanInfoPtr fields */
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.unkSrcAddrSecBreach,
                                             vlanInfoGet.unkSrcAddrSecBreach,
                 "get another vlanInfoPtr->unkSrcAddrSecBreach than was set: %d, %d", dev, iTemp);
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.unregNonIpMcastCmd,
                                             vlanInfoGet.unregNonIpMcastCmd,
                 "get another vlanInfoPtr->unregNonIpMcastCmd than was set: %d, %d", dev, iTemp);
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.unregIpv4McastCmd,
                                             vlanInfoGet.unregIpv4McastCmd,
                 "get another vlanInfoPtr->unregIpv4McastCmd than was set: %d, %d", dev, iTemp);
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.unregIpv6McastCmd,
                                             vlanInfoGet.unregIpv6McastCmd,
                 "get another vlanInfoPtr->unregIpv6McastCmd than was set: %d, %d", dev, iTemp);
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.unkUcastCmd,
                                             vlanInfoGet.unkUcastCmd,
                 "get another vlanInfoPtr->unkUcastCmd than was set: %d, %d", dev, iTemp);
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.unregIpv4BcastCmd,
                                             vlanInfoGet.unregIpv4BcastCmd,
                 "get another vlanInfoPtr->unregIpv4BcastCmd than was set: %d, %d", dev, iTemp);
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.unregNonIpv4BcastCmd,
                                             vlanInfoGet.unregNonIpv4BcastCmd,
                 "get another vlanInfoPtr->unregNonIpv4BcastCmd than was set: %d, %d", dev, iTemp);
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.ipv4IgmpToCpuEn,
                                             vlanInfoGet.ipv4IgmpToCpuEn,
                 "get another vlanInfoPtr->ipv4IgmpToCpuEn than was set: %d, %d", dev, iTemp);
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.mirrToRxAnalyzerEn,
                                             vlanInfoGet.mirrToRxAnalyzerEn,
                 "get another vlanInfoPtr->mirrToRxAnalyzerEn than was set: %d, %d", dev, iTemp);
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.ipv6IcmpToCpuEn,
                                             vlanInfoGet.ipv6IcmpToCpuEn,
                 "get another vlanInfoPtr->ipv6IcmpToCpuEn than was set: %d, %d", dev, iTemp);
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.ipCtrlToCpuEn,
                                             vlanInfoGet.ipCtrlToCpuEn,
                 "get another vlanInfoPtr->ipCtrlToCpuEn than was set: %d, %d", dev, iTemp);
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.ipv4IpmBrgEn,
                                             vlanInfoGet.ipv4IpmBrgEn,
                 "get another vlanInfoPtr->ipv4IpmBrgEn than was set: %d, %d", dev, iTemp);
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.ipv6IpmBrgEn,
                                             vlanInfoGet.ipv6IpmBrgEn,
                 "get another vlanInfoPtr->ipv6IpmBrgEn than was set: %d, %d", dev, iTemp);

                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.ipv4IpmBrgMode,
                                             vlanInfoGet.ipv4IpmBrgMode,
                 "get another vlanInfoPtr->ipv4IpmBrgMode than was set: %d, %d", dev, iTemp);

                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.ipv6IpmBrgMode,
                                             vlanInfoGet.ipv6IpmBrgMode,
                 "get another vlanInfoPtr->ipv6IpmBrgMode than was set: %d, %d", dev, iTemp);

                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.ipv4UcastRouteEn,
                                             vlanInfoGet.ipv4UcastRouteEn,
                 "get another vlanInfoPtr->ipv4UcastRouteEn than was set: %d, %d", dev, iTemp);
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.ipv6UcastRouteEn,
                                             vlanInfoGet.ipv6UcastRouteEn,
                 "get another vlanInfoPtr->ipv6UcastRouteEn than was set: %d, %d", dev, iTemp);
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.stgId,
                                             vlanInfoGet.stgId,
                 "get another vlanInfoPtr->stgId than was set: %d, %d", dev, iTemp);

                if (devFamily != CPSS_PP_FAMILY_CHEETAH_E)
                {
                    if (devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E)
                    {
                        UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.ipv6SiteIdMode,
                                                     vlanInfoGet.ipv6SiteIdMode,
                         "get another vlanInfoPtr->ipv6SiteIdMode than was set: %d, %d", dev, iTemp);
                        UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.ipv4McastRouteEn,
                                                     vlanInfoGet.ipv4McastRouteEn,
                         "get another vlanInfoPtr->ipv4McastRouteEn than was set: %d, %d", dev, iTemp);
                        UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.ipv6McastRouteEn,
                                                     vlanInfoGet.ipv6McastRouteEn,
                         "get another vlanInfoPtr->ipv6McastRouteEn than was set: %d, %d", dev, iTemp);
                    }
                    UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.autoLearnDisable,
                                                 vlanInfoGet.autoLearnDisable,
                     "get another vlanInfoPtr->autoLearnDisable than was set: %d, %d", dev, iTemp);
                    UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.naMsgToCpuEn,
                                                 vlanInfoGet.naMsgToCpuEn,
                     "get another vlanInfoPtr->naMsgToCpuEn than was set: %d, %d", dev, iTemp);
                    UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.mruIdx,
                                                 vlanInfoGet.mruIdx,
                     "get another vlanInfoPtr->mruIdx than was set: %d, %d", dev, iTemp);
                    UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.bcastUdpTrapMirrEn,
                                                 vlanInfoGet.bcastUdpTrapMirrEn,
                     "get another vlanInfoPtr->bcastUdpTrapMirrEn than was set: %d, %d", dev, iTemp);
                }
            }

            if ((devFamily >= CPSS_PP_FAMILY_CHEETAH3_E) && (devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E))
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.vrfId, vlanInfoGet.vrfId,
                 "get another vlanInfoPtr->vrfId than was set: %d, %d", dev, iTemp);
            }

            if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.mirrToRxAnalyzerIndex,
                                             vlanInfoGet.mirrToRxAnalyzerIndex,
                 "got another vlanInfoPtr->mirrToRxAnalyzerIndex than was set: %d, %d", dev, iTemp);

                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.mirrToTxAnalyzerEn,
                                             vlanInfoGet.mirrToTxAnalyzerEn,
                 "got another vlanInfoPtr->mirrToTxAnalyzerEn than was set: %d, %d", dev, iTemp);

                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.mirrToTxAnalyzerIndex,
                                             vlanInfoGet.mirrToTxAnalyzerIndex,
                 "got another vlanInfoPtr->mirrToTxAnalyzerIndex than was set: %d, %d", dev, iTemp);

                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.fidValue,
                                             vlanInfoGet.fidValue,
                 "got another vlanInfoPtr->fidValue than was set: %d, %d", dev, iTemp);

                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.unknownMacSaCmd,
                                             vlanInfoGet.unknownMacSaCmd,
                 "got another vlanInfoPtr->unknownMacSaCmd than was set: %d, %d", dev, iTemp);

                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.ipv4McBcMirrToAnalyzerEn,
                                             vlanInfoGet.ipv4McBcMirrToAnalyzerEn,
                 "got another vlanInfoPtr->ipv4McBcMirrToAnalyzerEn than was set: %d, %d", dev, iTemp);

                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.ipv4McBcMirrToAnalyzerIndex,
                                             vlanInfoGet.ipv4McBcMirrToAnalyzerIndex,
                 "got another vlanInfoPtr->ipv4McBcMirrToAnalyzerIndex than was set: %d, %d", dev, iTemp);

                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.ipv6McMirrToAnalyzerEn,
                                             vlanInfoGet.ipv6McMirrToAnalyzerEn,
                 "got another vlanInfoPtr->ipv6McMirrToAnalyzerEn than was set: %d, %d", dev, iTemp);

                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.ipv6McMirrToAnalyzerIndex,
                                             vlanInfoGet.ipv6McMirrToAnalyzerIndex,
                 "got another vlanInfoPtr->ipv6McMirrToAnalyzerIndex than was set: %d, %d", dev, iTemp);
            }

            if (PRV_CPSS_SIP_6_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(vlanInfo.mirrToTxAnalyzerMode,
                                             vlanInfoGet.mirrToTxAnalyzerMode,
                                             "get another vlanInfoPtr->mirrToTxAnalyzerMode than was set: %d, %d", dev, iTemp);
            }
        }

        /* 1.5. Try to read entry with index out of range. */
        st = cpssDxChBrgVlanEntryRead(dev, (GT_U16)numEntries, &portsMembersGet,
                        &portsTaggingGet,&vlanInfoGet, &isValid, portsTaggingCmdGetPtr);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgVlanEntryRead: %d, %d", dev, numEntries);

        /* 1.6. Delete all entries in vlan table */
        for(iTemp = 0; iTemp < numEntries; iTemp+=step)
        {
            st = cpssDxChBrgVlanEntryInvalidate(dev, iTemp);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgVlanEntryInvalidate: %d, %d", dev, iTemp);
        }

        /* 1.7. Try to delete entry with index out of range. */
        st = cpssDxChBrgVlanEntryInvalidate(dev, (GT_U16)numEntries);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgVlanEntryInvalidate: %d, %d", dev, numEntries);
    }
}

/*----------------------------------------------------------------------------*/
/*
    Test function to Fill Vlan Translation Table Ingress.
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanFillVlanTranslationTableIngress)
{
/*
    ITERATE_DEVICE (DxCh3)
    1.1. Get table Size.
         Call cpssDxChCfgTableNumEntriesGet with table [CPSS_DXCH_CFG_TABLE_VLAN_E]
                                                 and non-NULL numEntriesPtr.
    Expected: GT_OK.
    1.2. Fill all entries in Vlan Translation Table Ingress.
         Call cpssDxChBrgVlanTranslationEntryWrite with vlanId [0..numEntries-1],
                                                        direction [CPSS_DIRECTION_INGRESS_E]
                                                        and transVlanId [100].
    Expected: GT_OK.
    1.3. Try to write entry with index out of range.
         Call cpssDxChBrgVlanTranslationEntryWrite with vlanId [numEntries],
                                                        direction [CPSS_DIRECTION_INGRESS_E]
                                                        and transVlanId [100].
    Expected: NOT GT_OK.
    1.4. Read all entries in Vlan Translation Table Ingress and compare with original.
         Call cpssDxChBrgVlanTranslationEntryRead with same vlanId and direction.
    Expected: GT_OK and the same transVlanId.
    1.5. Try to read entry with index out of range.
         Call cpssDxChBrgVlanTranslationEntryRead with same vlanId [numEntries] and direction.
    Expected: NOT GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32      numEntries  = 0;
    GT_U16      iTemp       = 0;
    GT_U16               step = 1;
    GT_U32   gmFillMaxIterations = 128;

    CPSS_DIRECTION_ENT  direction      = CPSS_DIRECTION_INGRESS_E;
    GT_U16              transVlanId    = 0;
    GT_U16              transVlanIdGet = 0;
    GT_U32              max_transVlanId;/*max value that transVlanId support for the device (for ingress)*/


    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        max_transVlanId = PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_VID_MAC(dev);

        /* 1.1. Get vlan translation table Size */
        numEntries = _4K;

        if (IS_NEEDED_TO_REDUCE_ITERATIONS_DUE_TO_POOR_PERFORMANCE_MAC || GT_TRUE == prvUtfReduceLogSizeFlagGet())
        {
            /* get new step that will match 'max iterations' for GM*/
            step = (GT_U16)prvUtfIterationReduce(dev,numEntries,gmFillMaxIterations,step);
        }

        /* 1.2. Fill all entries in Vlan Translation Table Ingress */
        for(iTemp = 0; iTemp < numEntries; iTemp+=step)
        {
            /* fill each entry with different value , to check no overlapping */
            transVlanId = (GT_U16)(max_transVlanId - iTemp);

            st = cpssDxChBrgVlanTranslationEntryWrite(dev, iTemp, direction, transVlanId);
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgVlanTranslationEntryWrite: %d, %d, %d, %d",
                                         dev, iTemp, direction, transVlanId);
        }

        /* 1.3. Try to write entry with index out of range. */
        st = cpssDxChBrgVlanTranslationEntryWrite(dev, (GT_U16)numEntries, direction, transVlanId);
        UTF_VERIFY_NOT_EQUAL4_STRING_MAC(GT_OK, st,
                "cpssDxChBrgVlanTranslationEntryWrite: %d, %d, %d, %d",
                                         dev, numEntries, direction, transVlanId);

        /* 1.4. Read all entries in Vlan Translation Table Ingress and compare with original */
        for(iTemp = 0; iTemp < numEntries; iTemp+=step)
        {
            st = cpssDxChBrgVlanTranslationEntryRead(dev, iTemp, direction, &transVlanIdGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgVlanTranslationEntryRead: %d, %d, %d", dev, iTemp, direction);

            /* we already fill each entry with different value , to check no overlapping */
            transVlanId = (GT_U16)(max_transVlanId - iTemp);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(transVlanId, transVlanIdGet,
                       "get another transVlanId than was set: %d", dev);
        }

        /* 1.5. Try to read entry with index out of range. */
        st = cpssDxChBrgVlanTranslationEntryRead(dev, (GT_U16)numEntries,
                                                 direction, &transVlanIdGet);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                "cpssDxChBrgVlanTranslationEntryRead: %d, %d, %d", dev, numEntries, direction);
    }
}

/*----------------------------------------------------------------------------*/
/*
    Test function to Fill Vlan Translation Table Egress.
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanFillVlanTranslationTableEgress)
{
/*
    ITERATE_DEVICE (DxCh3)
    1.1. Get table Size.
         Call cpssDxChCfgTableNumEntriesGet with table [CPSS_DXCH_CFG_TABLE_VLAN_E]
                                                 and non-NULL numEntriesPtr.
    Expected: GT_OK.
    1.2. Fill all entries in Vlan Translation Table Ingress.
         Call cpssDxChBrgVlanTranslationEntryWrite with vlanId [0..numEntries-1],
                                                        direction [CPSS_DIRECTION_INGRESS_E]
                                                        and transVlanId [100].
    Expected: GT_OK.
    1.3. Try to write entry with index out of range.
         Call cpssDxChBrgVlanTranslationEntryWrite with vlanId [numEntries],
                                                        direction [CPSS_DIRECTION_INGRESS_E]
                                                        and transVlanId [100].
    Expected: NOT GT_OK.
    1.4. Read all entries in Vlan Translation Table Ingress and compare with original.
         Call cpssDxChBrgVlanTranslationEntryRead with same vlanId and direction.
    Expected: GT_OK and the same transVlanId.
    1.5. Try to read entry with index out of range.
         Call cpssDxChBrgVlanTranslationEntryRead with same vlanId [numEntries] and direction.
    Expected: NOT GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32      numEntries = 0;
    GT_U16      iTemp      = 0;
    GT_U16               step = 1;
    GT_U32   gmFillMaxIterations = 128;

    CPSS_DIRECTION_ENT  direction      = CPSS_DIRECTION_EGRESS_E;
    GT_U16              transVlanId    = 0;
    GT_U16              transVlanIdGet = 0;
    GT_U32              max_transVlanId;/*max value that transVlanId support for the device (for egress)*/

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    /* Fill the entry for Vlan Translation Table Egress */
    transVlanId = 100;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Get vlan translation table Size */
        numEntries = 1 + PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_TABLE_VLAN_MAC(dev);

        max_transVlanId = (_4K - 1);

        if (IS_NEEDED_TO_REDUCE_ITERATIONS_DUE_TO_POOR_PERFORMANCE_MAC || GT_TRUE == prvUtfReduceLogSizeFlagGet())
        {
            /* get new step that will match 'max iterations' for GM*/
            step = (GT_U16)prvUtfIterationReduce(dev,numEntries,gmFillMaxIterations,step);
        }

        /* 1.2. Fill all entries in Vlan Translation Table Egress */
        for(iTemp = 0; iTemp < numEntries; iTemp+=step)
        {
            /* fill each entry with different value , to check no overlapping */
            transVlanId = (GT_U16)(max_transVlanId - (iTemp % (max_transVlanId+1)));

            st = cpssDxChBrgVlanTranslationEntryWrite(dev, iTemp, direction, transVlanId);
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgVlanTranslationEntryWrite: %d, %d, %d, %d",
                                         dev, iTemp, direction, transVlanId);
        }

        /* 1.3. Try to write entry with index out of range. */
        st = cpssDxChBrgVlanTranslationEntryWrite(dev, (GT_U16)numEntries, direction, transVlanId);
        UTF_VERIFY_NOT_EQUAL4_STRING_MAC(GT_OK, st,
                "cpssDxChBrgVlanTranslationEntryWrite: %d, %d, %d, %d",
                                         dev, numEntries, direction, transVlanId);

        /* 1.4. Read all entries in Vlan Translation Table Egress and compare with original */
        for(iTemp = 0; iTemp < numEntries; iTemp+=step)
        {
            st = cpssDxChBrgVlanTranslationEntryRead(dev, iTemp, direction, &transVlanIdGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgVlanTranslationEntryRead: %d, %d, %d", dev, iTemp, direction);

            /* we already fill each entry with different value , to check no overlapping */
            transVlanId = (GT_U16)(max_transVlanId - (iTemp % (max_transVlanId+1)));
            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(transVlanId, transVlanIdGet,
                       "get another transVlanId than was set: %d", dev);
        }

        /* 1.5. Try to read entry with index out of range. */
        st = cpssDxChBrgVlanTranslationEntryRead(dev, (GT_U16)numEntries, direction, &transVlanIdGet);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                "cpssDxChBrgVlanTranslationEntryRead: %d, %d, %d", dev, numEntries, direction);
    }
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanEgressPortTagStateGet
(
    IN  GT_U8                                dev,
    IN  GT_PORT_NUM                          portNum,
    OUT CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT  *tagCmdPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanEgressPortTagStateGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with non-null tagCmdPtr.
    Expected: GT_OK.
    1.1.2. Call api with wrong tagCmdPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8                                dev;
    GT_PORT_NUM                          portNum;
    CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT  tagCmd;
    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with non-null tagCmdPtr.
                Expected: GT_OK.
            */
            st = cpssDxChBrgVlanEgressPortTagStateGet(dev, portNum, &tagCmd);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call api with wrong tagCmdPtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgVlanEgressPortTagStateGet(dev, portNum, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                       "%d, tagCmdPtr = NULL", dev);
        }
        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanEgressPortTagStateGet(dev, portNum, &tagCmd);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanEgressPortTagStateGet(dev, portNum, &tagCmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanEgressPortTagStateGet(dev, portNum, &tagCmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    /* restore correct values */
    portNum = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanEgressPortTagStateGet(dev, portNum, &tagCmd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanEgressPortTagStateGet(dev, portNum, &tagCmd);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanEgressPortTagStateModeGet
(
    IN  GT_U8                                        dev,
    IN  GT_PORT_NUM                                  portNum,
    OUT CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT  *stateModePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanEgressPortTagStateModeGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with non-null stateModePtr.
    Expected: GT_OK.
    1.1.2. Call api with wrong stateModePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8                                        dev;
    GT_PORT_NUM                                  portNum;
    CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT  stateMode;
    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with non-null stateModePtr.
                Expected: GT_OK.
            */
            st = cpssDxChBrgVlanEgressPortTagStateModeGet(dev, portNum, &stateMode);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call api with wrong stateModePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgVlanEgressPortTagStateModeGet(dev, portNum, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                       "%d, stateModePtr = NULL", dev);
        }

        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           virtual ports.
        */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanEgressPortTagStateModeGet(dev, portNum, &stateMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanEgressPortTagStateModeGet(dev, portNum, &stateMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanEgressPortTagStateModeGet(dev, portNum, &stateMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    /* restore correct values */
    portNum = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanEgressPortTagStateModeGet(dev, portNum, &stateMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanEgressPortTagStateModeGet(dev, portNum, &stateMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanEgressPortTagStateModeSet
(
    IN  GT_U8                                       dev,
    IN  GT_PORT_NUM                                 portNum,
    IN  CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT stateMode
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanEgressPortTagStateModeSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with stateMode[CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_EVLAN_MODE_E /
                             CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_EPORT_MODE_E],
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgVlanEgressPortTagStateModeGet
           with the same parameters.
    Expected: GT_OK and the same values than was set.
    1.1.3. Call api with wrong stateMode [wrong enum values].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st = GT_OK;

    GT_U8                                       dev;
    GT_PORT_NUM                                 portNum;
    CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT stateMode;
    CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT stateModeGet;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    stateMode = CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_EVLAN_MODE_E;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with stateMode[CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_EVLAN_MODE_E /
                                         CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_EPORT_MODE_E],
                Expected: GT_OK.
            */

            /* call with stateMode[CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_EVLAN_MODE_E] */
            stateMode = CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_EVLAN_MODE_E;

            st = cpssDxChBrgVlanEgressPortTagStateModeSet(dev, portNum, stateMode);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call cpssDxChBrgVlanEgressPortTagStateModeGet
                       with the same parameters.
                Expected: GT_OK and the same values than was set.
            */
            st = cpssDxChBrgVlanEgressPortTagStateModeGet(dev, portNum, &stateModeGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChBrgVlanEgressPortTagStateModeGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(stateMode, stateModeGet,
                           "got another stateMode then was set: %d", dev);

            /* call with stateMode[CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_EPORT_MODE_E] */
            stateMode = CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_EPORT_MODE_E;

            st = cpssDxChBrgVlanEgressPortTagStateModeSet(dev, portNum, stateMode);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call get func again, 1.2. */
            st = cpssDxChBrgVlanEgressPortTagStateModeGet(dev, portNum, &stateModeGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChBrgVlanEgressPortTagStateModeGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(stateMode, stateModeGet,
                           "got another stateMode then was set: %d", dev);

            /*
                1.1.3. Call api with wrong stateMode [wrong enum values].
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanEgressPortTagStateModeSet(dev, portNum, stateMode),
                                stateMode);
        }

        stateMode = CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_EVLAN_MODE_E;

        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanEgressPortTagStateModeSet(dev, portNum, stateMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanEgressPortTagStateModeSet(dev, portNum, stateMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanEgressPortTagStateModeSet(dev, portNum, stateMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    /* restore correct values */
    portNum = 0;
    stateMode = CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_EVLAN_MODE_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanEgressPortTagStateModeSet(dev, portNum, stateMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanEgressPortTagStateModeSet(dev, portNum, stateMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanEgressPortTagStateSet
(
    IN  GT_U8                               dev,
    IN  GT_PORT_NUM                         portNum,
    IN  CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT tagCmd
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanEgressPortTagStateSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with tagCmd[CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E /
                            CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E /
                            CPSS_DXCH_BRG_VLAN_PORT_TAG1_CMD_E /
                            CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E /
                            CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E /
                            CPSS_DXCH_BRG_VLAN_PORT_DO_NOT_MODIFY_TAG_CMD_E(SIP5 only)],
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgVlanEgressPortTagStateGet
           with the same parameters.
    Expected: GT_OK and the same values than was set.
    1.1.3. Call api with wrong tagCmd [wrong enum values].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st = GT_OK;

    GT_U8                               dev;
    GT_PORT_NUM                         portNum;
    CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT tagCmd = CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E;
    CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT tagCmdGet = CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E;
    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with tagCmd[CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E /
                                        CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E /
                                        CPSS_DXCH_BRG_VLAN_PORT_TAG1_CMD_E /
                                        CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E /
                                        CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E /
                                        CPSS_DXCH_BRG_VLAN_PORT_DO_NOT_MODIFY_TAG_CMD_E(SIP5 only)],
                Expected: GT_OK.
            */

            /* call with tagCmd[CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E] */
            tagCmd = CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E;

            st = cpssDxChBrgVlanEgressPortTagStateSet(dev, portNum, tagCmd);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call cpssDxChBrgVlanEgressPortTagStateGet
                       with the same parameters.
                Expected: GT_OK and the same values than was set.
            */
            st = cpssDxChBrgVlanEgressPortTagStateGet(dev, portNum, &tagCmdGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChBrgVlanEgressPortTagStateGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(tagCmd, tagCmdGet,
                           "got another tagCmd then was set: %d", dev);

            /* call with tagCmd[CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E] */
            tagCmd = CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E;

            st = cpssDxChBrgVlanEgressPortTagStateSet(dev, portNum, tagCmd);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call get func again, 1.1.2. */
            st = cpssDxChBrgVlanEgressPortTagStateGet(dev, portNum, &tagCmdGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChBrgVlanEgressPortTagStateGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(tagCmd, tagCmdGet,
                           "got another tagCmd then was set: %d", dev);

            /* call with tagCmd[CPSS_DXCH_BRG_VLAN_PORT_TAG1_CMD_E] */
            tagCmd = CPSS_DXCH_BRG_VLAN_PORT_TAG1_CMD_E;

            st = cpssDxChBrgVlanEgressPortTagStateSet(dev, portNum, tagCmd);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call get func again, 1.1.2. */
            st = cpssDxChBrgVlanEgressPortTagStateGet(dev, portNum, &tagCmdGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChBrgVlanEgressPortTagStateGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(tagCmd, tagCmdGet,
                           "got another tagCmd then was set: %d", dev);

            /* call with tagCmd[CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E] */
            tagCmd = CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E;

            st = cpssDxChBrgVlanEgressPortTagStateSet(dev, portNum, tagCmd);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call get func again, 1.1.2. */
            st = cpssDxChBrgVlanEgressPortTagStateGet(dev, portNum, &tagCmdGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChBrgVlanEgressPortTagStateGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(tagCmd, tagCmdGet,
                           "got another tagCmd then was set: %d", dev);

            /* call with tagCmd[CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E] */
            tagCmd = CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E;

            st = cpssDxChBrgVlanEgressPortTagStateSet(dev, portNum, tagCmd);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call get func again, 1.1.2. */
            st = cpssDxChBrgVlanEgressPortTagStateGet(dev, portNum, &tagCmdGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChBrgVlanEgressPortTagStateGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(tagCmd, tagCmdGet,
                           "got another tagCmd then was set: %d", dev);

            if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            {

                /* call with tagCmd[CPSS_DXCH_BRG_VLAN_PORT_DO_NOT_MODIFY_TAG_CMD_E] */
                tagCmd = CPSS_DXCH_BRG_VLAN_PORT_DO_NOT_MODIFY_TAG_CMD_E;

                st = cpssDxChBrgVlanEgressPortTagStateSet(dev, portNum, tagCmd);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /* call get func again, 1.1.2. */
                st = cpssDxChBrgVlanEgressPortTagStateGet(dev, portNum, &tagCmdGet);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                             "cpssDxChBrgVlanEgressPortTagStateGet: %d ", dev);

                /* verifying values */
                UTF_VERIFY_EQUAL1_STRING_MAC(tagCmd, tagCmdGet,
                                             "got another tagCmd then was set: %d", dev);
            }

            /*
                1.1.3. Call api with wrong tagCmd [wrong enum values].
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanEgressPortTagStateSet
                                (dev, portNum, tagCmd),
                                tagCmd);
        }

        tagCmd = CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E;

        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanEgressPortTagStateSet(dev, portNum, tagCmd);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanEgressPortTagStateSet(dev, portNum, tagCmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanEgressPortTagStateSet(dev, portNum, tagCmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    /* restore correct values */
    portNum = 0;
    tagCmd = CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanEgressPortTagStateSet(dev, portNum, tagCmd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanEgressPortTagStateSet(dev, portNum, tagCmd);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanEgressTagTpidSelectModeGet
(
    IN  GT_U8                                        dev,
    IN  GT_PORT_NUM                                  portNum,
    IN  CPSS_ETHER_MODE_ENT                          ethMode,
    OUT CPSS_DXCH_BRG_VLAN_TAG_TPID_SELECT_MODE_ENT  *modePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanEgressTagTpidSelectModeGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with ethMode[CPSS_VLAN_ETHERTYPE0_E /
                             CPSS_VLAN_ETHERTYPE1_E],
                     non NULL modePtr.
    Expected: GT_OK.
    1.1.2. Call api with wrong ethMode [wrong enum values].
    Expected: GT_BAD_PARAM.
    1.1.3. Call api with wrong modePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8                                        dev;
    GT_PORT_NUM                                  portNum;
    CPSS_ETHER_MODE_ENT                          ethMode = CPSS_VLAN_ETHERTYPE0_E;
    CPSS_DXCH_BRG_VLAN_TAG_TPID_SELECT_MODE_ENT  mode;
    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with ethMode[CPSS_VLAN_ETHERTYPE0_E /
                                         CPSS_VLAN_ETHERTYPE1_E],
                                 non NULL modePtr.
                Expected: GT_OK.
            */

            /* call with ethMode[CPSS_VLAN_ETHERTYPE0_E] */
            ethMode = CPSS_VLAN_ETHERTYPE0_E;

            st = cpssDxChBrgVlanEgressTagTpidSelectModeGet(dev, portNum,
                                         ethMode, &mode);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call with ethMode[CPSS_VLAN_ETHERTYPE1_E] */
            ethMode = CPSS_VLAN_ETHERTYPE1_E;

            st = cpssDxChBrgVlanEgressTagTpidSelectModeGet(dev, portNum,
                                         ethMode, &mode);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call api with wrong ethMode [wrong enum values].
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanEgressTagTpidSelectModeGet
                                (dev, portNum, ethMode, &mode),
                                ethMode);

            /*
                1.1.3. Call api with wrong modePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgVlanEgressTagTpidSelectModeGet(dev, portNum,
                                         ethMode, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                       "%d, modePtr = NULL", dev);
        }

        ethMode = CPSS_VLAN_ETHERTYPE0_E;

        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanEgressTagTpidSelectModeGet(dev, portNum,
                                         ethMode, &mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanEgressTagTpidSelectModeGet(dev, portNum,
                                     ethMode, &mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanEgressTagTpidSelectModeGet(dev, portNum,
                                     ethMode, &mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    /* restore correct values */
    portNum = 0;
    ethMode = CPSS_VLAN_ETHERTYPE0_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanEgressTagTpidSelectModeGet(dev, portNum,
                                     ethMode, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanEgressTagTpidSelectModeGet(dev, portNum,
                                     ethMode, &mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanEgressTagTpidSelectModeSet
(
    IN  GT_U8                                       dev,
    IN  GT_PORT_NUM                                 portNum,
    IN  CPSS_ETHER_MODE_ENT                         ethMode,
    IN  CPSS_DXCH_BRG_VLAN_TAG_TPID_SELECT_MODE_ENT mode
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanEgressTagTpidSelectModeSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with the following parameters:
                   ethMode[CPSS_VLAN_ETHERTYPE0_E /
                           CPSS_VLAN_ETHERTYPE1_E],
                   mode[CPSS_DXCH_BRG_VLAN_EGRESS_TAG_TPID_SELECT_MODE_E /
                        CPSS_DXCH_BRG_VLAN_VID_TO_TPID_SELECT_TABLE_MODE_E],
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgVlanEgressTagTpidSelectModeGet
           with the same parameters.
    Expected: GT_OK and the same values than was set.
    1.1.3. Call api with wrong ethMode [wrong enum values].
    Expected: GT_BAD_PARAM.
    1.1.4. Call api with wrong mode [wrong enum values].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st = GT_OK;

    GT_U8                                       dev;
    GT_PORT_NUM                                 portNum;
    CPSS_ETHER_MODE_ENT                         ethMode;
    CPSS_DXCH_BRG_VLAN_TAG_TPID_SELECT_MODE_ENT mode;
    CPSS_DXCH_BRG_VLAN_TAG_TPID_SELECT_MODE_ENT modeGet;
    GT_U32      notAppFamilyBmp;
    ethMode = CPSS_VLAN_ETHERTYPE0_E;
    mode = CPSS_DXCH_BRG_VLAN_EGRESS_TAG_TPID_SELECT_MODE_E;
    modeGet = CPSS_DXCH_BRG_VLAN_EGRESS_TAG_TPID_SELECT_MODE_E;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with the following parameters:
                               ethMode[CPSS_VLAN_ETHERTYPE0_E /
                                       CPSS_VLAN_ETHERTYPE1_E],
                               mode[CPSS_DXCH_BRG_VLAN_EGRESS_TAG_TPID_SELECT_MODE_E /
                                    CPSS_DXCH_BRG_VLAN_VID_TO_TPID_SELECT_TABLE_MODE_E],
                Expected: GT_OK.
            */

            /* call with ethMode[CPSS_VLAN_ETHERTYPE0_E],
                         mode[CPSS_DXCH_BRG_VLAN_EGRESS_TAG_TPID_SELECT_MODE_E] */
            ethMode = CPSS_VLAN_ETHERTYPE0_E;
            mode = CPSS_DXCH_BRG_VLAN_EGRESS_TAG_TPID_SELECT_MODE_E;

            st = cpssDxChBrgVlanEgressTagTpidSelectModeSet(dev, portNum,
                                         ethMode, mode);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call cpssDxChBrgVlanEgressTagTpidSelectModeGet
                       with the same parameters.
                Expected: GT_OK and the same values than was set.
            */
            st = cpssDxChBrgVlanEgressTagTpidSelectModeGet(dev, portNum,
                                         ethMode, &modeGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChBrgVlanEgressTagTpidSelectModeGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                           "got another mode then was set: %d", dev);

            /* call with ethMode[CPSS_VLAN_ETHERTYPE1_E],
                         mode[CPSS_DXCH_BRG_VLAN_VID_TO_TPID_SELECT_TABLE_MODE_E] */
            ethMode = CPSS_VLAN_ETHERTYPE1_E;
            mode = CPSS_DXCH_BRG_VLAN_VID_TO_TPID_SELECT_TABLE_MODE_E;

            st = cpssDxChBrgVlanEgressTagTpidSelectModeSet(dev, portNum,
                                         ethMode, mode);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call get func again, 1.1.2. */
            st = cpssDxChBrgVlanEgressTagTpidSelectModeGet(dev, portNum,
                                         ethMode, &modeGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChBrgVlanEgressTagTpidSelectModeGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                           "got another mode then was set: %d", dev);

            /*
                1.1.3. Call api with wrong ethMode [wrong enum values].
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanEgressTagTpidSelectModeSet
                                (dev, portNum, ethMode, mode),
                                ethMode);

            /*
                1.1.4. Call api with wrong mode [wrong enum values].
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanEgressTagTpidSelectModeSet
                                (dev, portNum, ethMode, mode),
                                mode);
        }

        ethMode = CPSS_VLAN_ETHERTYPE0_E;
        mode = CPSS_DXCH_BRG_VLAN_EGRESS_TAG_TPID_SELECT_MODE_E;

        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanEgressTagTpidSelectModeSet(dev, portNum,
                                         ethMode, mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanEgressTagTpidSelectModeSet(dev, portNum,
                                     ethMode, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanEgressTagTpidSelectModeSet(dev, portNum,
                                     ethMode, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    /* restore correct values */
    portNum = 0;
    ethMode = CPSS_VLAN_ETHERTYPE0_E;
    mode = CPSS_DXCH_BRG_VLAN_EGRESS_TAG_TPID_SELECT_MODE_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanEgressTagTpidSelectModeSet(dev, portNum,
                                     ethMode, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanEgressTagTpidSelectModeSet(dev, portNum,
                                     ethMode, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanForwardingIdGet
(
    IN  GT_U8   dev,
    IN  GT_U16  vlanId,
    OUT GT_U32  *fidValuePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanForwardingIdGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with vlanId[BRG_VLAN_TESTED_VLAN_ID_CNS],
    Expected: GT_OK.
    1.2. Call api with wrong vlanId [BRG_VLAN_INVALID_VLAN_ID_CNS].
    Expected: NOT GT_OK.
    1.3. Call api with wrong fidValuePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8   dev;
    GT_U16  vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;
    GT_U32  fidValue;
    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with vlanId[BRG_VLAN_TESTED_VLAN_ID_CNS],
            Expected: GT_OK.
        */

        /* call with vlanId[BRG_VLAN_TESTED_VLAN_ID_CNS] */
        vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;

        st = cpssDxChBrgVlanForwardingIdGet(dev, vlanId, &fidValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call api with wrong vlanId [BRG_VLAN_INVALID_VLAN_ID_CNS].
            Expected: NOT GT_OK.
        */
        vlanId = BRG_VLAN_INVALID_VLAN_ID_CNS;

        st = cpssDxChBrgVlanForwardingIdGet(dev, vlanId, &fidValue);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;

        /*
            1.3. Call api with wrong fidValuePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgVlanForwardingIdGet(dev, vlanId, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                   "%d, fidValuePtr = NULL", dev);
    }

    /* restore correct values */
    vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanForwardingIdGet(dev, vlanId, &fidValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanForwardingIdGet(dev, vlanId, &fidValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanForwardingIdSet
(
    IN  GT_U8  dev,
    IN  GT_U16 vlanId,
    IN  GT_U32 fidValue
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanForwardingIdSet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with vlanId[BRG_VLAN_TESTED_VLAN_ID_CNS],
                   fidValue[PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_FID_MAC(dev)]
    1.2. Call cpssDxChBrgVlanForwardingIdGet
    Expected: GT_OK and the same values than was set.
    1.3. Call with wrong vlanId[BRG_VLAN_INVALID_VLAN_ID_CNS],
                   fidValue[PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_FID_MAC(dev)]
    Expected: NOT GT_OK.
    1.4. Call with vlanId[BRG_VLAN_TESTED_VLAN_ID_CNS],
         and wrong fidValue[PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_FID_MAC(dev) + 1]
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st = GT_OK;

    GT_U8  dev;
    GT_U16 vlanId = 0;
    GT_U32 fidValue = 0;
    GT_U32 fidValueGet = 0;
    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with vlanId[BRG_VLAN_TESTED_VLAN_ID_CNS],
                           fidValue[PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_FID_MAC(dev)]
            Expected: GT_OK.
        */
        vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;
        fidValue = UTF_CPSS_PP_MAX_FID_NUM_CNS(dev) - 1;

        st = cpssDxChBrgVlanForwardingIdSet(dev, vlanId, fidValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChBrgVlanForwardingIdGet
            Expected: GT_OK and the same values than was set.
        */
        st = cpssDxChBrgVlanForwardingIdGet(dev, vlanId, &fidValueGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                  "cpssDxChBrgVlanForwardingIdGet: %d ", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(fidValue, fidValueGet,
                       "got another fidValue then was set: %d", dev);

        /*
            1.3. Call with wrong vlanId[BRG_VLAN_INVALID_VLAN_ID_CNS],
                           fidValue[PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_FID_MAC(dev)]
            Expected: NOT GT_OK.
        */
        /* call with vlanId BRG_VLAN_TESTED_VLAN_ID_CNS */
        vlanId = BRG_VLAN_INVALID_VLAN_ID_CNS;
        fidValue = UTF_CPSS_PP_MAX_FID_NUM_CNS(dev) - 1;

        st = cpssDxChBrgVlanForwardingIdSet(dev, vlanId, fidValue);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.4. Call with vlanId[BRG_VLAN_TESTED_VLAN_ID_CNS],
                 and wrong fidValue[PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_FID_MAC(dev) + 1]
            Expected: GT_BAD_PARAM.
        */
        vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;
        fidValue = UTF_CPSS_PP_MAX_FID_NUM_CNS(dev);

        st = cpssDxChBrgVlanForwardingIdSet(dev, vlanId, fidValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* restore correct values */
    vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;
    fidValue = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanForwardingIdSet(dev, vlanId, fidValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanForwardingIdSet(dev, vlanId, fidValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexGet
(
    IN  GT_U8     dev,
    IN  GT_U16    vlanId,
    OUT GT_BOOL   *enablePtr,
    OUT GT_U32    *indexPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with vlanId
         [BIT_MASK_MAC(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.flexFieldNumBitsSupport.vid)],
                   non NULL enablePtr,
                   non NULL indexPtr.
    Expected: GT_OK.
    1.2. Call api with wrong vlanId
         [BIT_MASK_MAC(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.flexFieldNumBitsSupport.vid) + 1],
    Expected: NOT GT_OK.
    1.3. Call api with wrong enablePtr [NULL].
    Expected: GT_BAD_PTR.
    1.4. Call api with wrong indexPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8     dev;
    GT_U16    vlanId;
    GT_BOOL   enable;
    GT_U32    index;
    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with vlanId
                 [BIT_MASK_MAC(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.flexFieldNumBitsSupport.vid)],
                           non NULL enablePtr,
                           non NULL indexPtr.
            Expected: GT_OK.
        */
        vlanId = (GT_U16) (UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev) - 1);

        st = cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexGet(dev,
                            vlanId, &enable, &index);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call api with wrong vlanId
                 [BIT_MASK_MAC(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.flexFieldNumBitsSupport.vid) + 1],
            Expected: NOT GT_OK.
        */
        vlanId = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev);

        st = cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexGet(dev,
                            vlanId, &enable, &index);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        vlanId = (GT_U16) (UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev) - 1);

        /*
            1.3. Call api with wrong enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexGet(dev,
                            vlanId, NULL, &index);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                   "%d, enablePtr = NULL", dev);

        /*
            1.4. Call api with wrong indexPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexGet(dev,
                            vlanId, &enable, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                   "%d, indexPtr = NULL", dev);
    }

    /* restore correct values */
    vlanId = 100;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexGet(dev,
                            vlanId, &enable, &index);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexGet(dev,
                            vlanId, &enable, &index);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexSet
(
    IN  GT_U8   dev,
    IN  GT_U16  vlanId,
    IN  GT_BOOL enable,
    IN  GT_U32  index
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexSet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with vlanId
         [BIT_MASK_MAC(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.flexFieldNumBitsSupport.vid)],
                   enable[GT_TRUE / GT_FALSE],
                   index[0 - BRG_VLAN_MIRROR_TO_ANALYZER_MAX_INDEX_CNS],
    Expected: GT_OK.
    1.2. Call cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexGet
    Expected: GT_OK and the same values than was set.
    1.3. Call api with wrong vlanId
         [BIT_MASK_MAC(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.flexFieldNumBitsSupport.vid) + 1].
    Expected: NOT GT_OK.
    1.4. Call api with wrong index [BRG_VLAN_MIRROR_TO_ANALYZER_MAX_INDEX_CNS + 1].
    Expected: NOT GT_OK.
*/
    GT_STATUS st = GT_OK;

    GT_U8   dev;
    GT_U16  vlanId = 0;
    GT_BOOL enable = GT_FALSE;
    GT_U32  index = 0;
    GT_BOOL enableGet = GT_FALSE;
    GT_U32  indexGet = 0;
    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with vlanId
                 [BIT_MASK_MAC(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.flexFieldNumBitsSupport.vid)],
                           enable[GT_TRUE / GT_FALSE],
                           index[0 - PRV_CPSS_DXCH_MIRROR_TO_ANALYZER_MAX_INDEX_CNS],
            Expected: GT_OK.
        */

        /* call with the following parameters:
           vlanId [BIT_MASK_MAC(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.flexFieldNumBitsSupport.vid)],
           enable[GT_FALSE],
           index[0],
        */
        vlanId = (GT_U16) (UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev) - 1);
        enable = GT_FALSE;
        index = 0;

        st = cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexSet(dev,
                            vlanId, enable, index);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexGet
            Expected: GT_OK and the same values than was set.
        */
        st = cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexGet(dev,
                            vlanId, &enableGet, &indexGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                  "cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexGet: %d ", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                       "got another enable then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(index, indexGet,
                       "got another index then was set: %d", dev);

        /* call with the following parameters:
           vlanId [BIT_MASK_MAC(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.flexFieldNumBitsSupport.vid)],
           enable[GT_TRUE],
           index[BRG_VLAN_MIRROR_TO_ANALYZER_MAX_INDEX_CNS],
        */
        vlanId = (GT_U16) (UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev) - 1);
        enable = GT_TRUE;
        index = BRG_VLAN_MIRROR_TO_ANALYZER_MAX_INDEX_CNS;

        st = cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexSet(dev,
                            vlanId, enable, index);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexGet
            Expected: GT_OK and the same values than was set.
        */
        st = cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexGet(dev,
                            vlanId, &enableGet, &indexGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                  "cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexGet: %d ", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                       "got another enable then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(index, indexGet,
                       "got another index then was set: %d", dev);

        /*
            1.3. Call api with wrong vlanId
                 [BIT_MASK_MAC(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.flexFieldNumBitsSupport.vid) + 1].
            Expected: NOT GT_OK.
        */
        vlanId = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev);

        st = cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexSet(dev,
                            vlanId, enable, index);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        vlanId = (GT_U16) (UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev) - 1);

        /*
            1.4. Call api with wrong index [BRG_VLAN_MIRROR_TO_ANALYZER_MAX_INDEX_CNS + 1].
            Expected: NOT GT_OK.
        */
        index = BRG_VLAN_MIRROR_TO_ANALYZER_MAX_INDEX_CNS + 1;

        st = cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexSet(dev,
                            vlanId, enable, index);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        index = 0;
    }

    /* restore correct values */
    vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;
    enable = GT_TRUE;
    index = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexSet(dev,
                            vlanId, enable, index);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexSet(dev,
                            vlanId, enable, index);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexGet
(
    IN  GT_U8     dev,
    IN  GT_U16    vlanId,
    OUT GT_BOOL   *enablePtr,
    OUT GT_U32    *indexPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with vlanId
         [BIT_MASK_MAC(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.flexFieldNumBitsSupport.vid)],
                   non NULL enablePtr,
                   non NULL indexPtr.
    Expected: GT_OK.
    1.2. Call api with wrong vlanId
         [BIT_MASK_MAC(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.flexFieldNumBitsSupport.vid) + 1],
    Expected: NOT GT_OK.
    1.3. Call api with wrong enablePtr [NULL].
    Expected: GT_BAD_PTR.
    1.4. Call api with wrong indexPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS        st                                         = GT_OK;
    GT_U8             dev                                        = 0;
    GT_U16            vlanId                                 = 0;
    GT_BOOL           enable                                = GT_FALSE;
    GT_U32            index                                = 0;
    GT_U32      notAppFamilyBmp                = 0;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with vlanId
                 [BIT_MASK_MAC(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.flexFieldNumBitsSupport.vid)],
                           non NULL enablePtr,
                           non NULL indexPtr.
            Expected: GT_OK.
        */
        vlanId = (GT_U16) (UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev) - 1);

        st = cpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexGet(dev, vlanId,
                                                                                                           &enable, &index);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call api with wrong vlanId
                 [BIT_MASK_MAC(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.flexFieldNumBitsSupport.vid) + 1].
            Expected: NOT GT_OK.
        */
        vlanId = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev);

        st = cpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexGet(dev, vlanId,
                                                                                                           &enable, &index);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        vlanId = (GT_U16) (UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev) - 1);

        /*
            1.3. Call api with wrong enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexGet(dev, vlanId,
                                                                                                           NULL, &index);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                     "%d, enablePtr = NULL", dev);

        /*
            1.4. Call api with wrong indexPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexGet(dev,
                            vlanId, &enable, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                     "%d, indexPtr = NULL", dev);
    }

    /* restore correct values */
    vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexGet(dev, vlanId,
                                                                                                           &enable, &index);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexGet(dev, vlanId,
                                                                                                       &enable, &index);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexSet
(
    IN  GT_U8   dev,
    IN  GT_U16  vlanId,
    IN  GT_BOOL enable,
    IN  GT_U32  index
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexSet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with vlanId
         [BIT_MASK_MAC(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.flexFieldNumBitsSupport.vid)],
                   enable[GT_TRUE / GT_FALSE],
                   index[0 - BRG_VLAN_MIRROR_TO_ANALYZER_MAX_INDEX_CNS],
    Expected: GT_OK.
    1.2. Call cpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexGet
    Expected: GT_OK and the same values than was set.
    1.3. Call api with wrong vlanId
         [BIT_MASK_MAC(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.flexFieldNumBitsSupport.vid) + 1].
    Expected: NOT GT_OK.
    1.4. Call api with wrong index [BRG_VLAN_MIRROR_TO_ANALYZER_MAX_INDEX_CNS + 1].
    Expected: NOT GT_OK.
*/
    GT_STATUS         st                                         = GT_OK;
    GT_U8           dev                                        = 0;
    GT_U16          vlanId                                 = 0;
    GT_BOOL         enable                                 = GT_FALSE;
    GT_U32          index                                 = 0;
    GT_BOOL         enableGet                         = GT_FALSE;
    GT_U32          indexGet                         = 0;
    GT_U32      notAppFamilyBmp                = 0;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with vlanId
                 [BIT_MASK_MAC(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.flexFieldNumBitsSupport.vid)],
                           enable[GT_TRUE / GT_FALSE],
                           index[0 - PRV_CPSS_DXCH_MIRROR_TO_ANALYZER_MAX_INDEX_CNS],
            Expected: GT_OK.
        */

        /* call with the following parameters:
           vlanId [BIT_MASK_MAC(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.flexFieldNumBitsSupport.vid)],
           enable[GT_FALSE],
           index[0],
        */
        vlanId = (GT_U16) (UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev) - 1);
        enable = GT_FALSE;
        index = 0;

        st = cpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexSet(dev, vlanId,
                                                                                                           enable, index);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexGet
            Expected: GT_OK and the same values than was set.
        */
        st = cpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexGet(dev, vlanId,
                                                                                                        &enableGet, &indexGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                  "cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexGet: %d ", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                       "got another enable then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(index, indexGet,
                       "got another index then was set: %d", dev);

        /* call with the following parameters:
           vlanId [BIT_MASK_MAC(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.flexFieldNumBitsSupport.vid)],
           enable[GT_TRUE],
           index[BRG_VLAN_MIRROR_TO_ANALYZER_MAX_INDEX_CNS],
        */
        vlanId = (GT_U16) (UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev) - 1);
        enable = GT_TRUE;
        index = BRG_VLAN_MIRROR_TO_ANALYZER_MAX_INDEX_CNS;

        st = cpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexSet(dev,
                            vlanId, enable, index);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexGet
            Expected: GT_OK and the same values than was set.
        */
        st = cpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexGet(dev,
                            vlanId, &enableGet, &indexGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                  "cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexGet: %d ", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                       "got another enable then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(index, indexGet,
                       "got another index then was set: %d", dev);

        /*
            1.3. Call api with wrong vlanId
                 [BIT_MASK_MAC(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.flexFieldNumBitsSupport.vid) + 1].
            Expected: NOT GT_OK.
        */
        vlanId = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev);

        st = cpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexSet(dev,
                            vlanId, enable, index);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        vlanId = (GT_U16) (UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev) - 1);

        /*
            1.4. Call api with wrong index [BRG_VLAN_MIRROR_TO_ANALYZER_MAX_INDEX_CNS + 1].
            Expected: NOT GT_OK.
        */
        index = BRG_VLAN_MIRROR_TO_ANALYZER_MAX_INDEX_CNS + 1;

        st = cpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexSet(dev,
                            vlanId, enable, index);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        index = 0;
    }

    /* restore correct values */
    vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;
    enable = GT_TRUE;
    index = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexSet(dev,
                            vlanId, enable, index);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexSet(dev,
                            vlanId, enable, index);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortCfiEnableGet
(
    IN  GT_U8        dev,
    IN  GT_PORT_NUM  portNum,
    OUT GT_BOOL      *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortCfiEnableGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with not NULL enablePtr
    Expected: GT_OK.
    1.1.2. Call api with wrong enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8        dev;
    GT_PORT_NUM  portNum;
    GT_BOOL      enable;
    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with not NULL enablePtr
                Expected: GT_OK.
            */
            st = cpssDxChBrgVlanPortCfiEnableGet(dev, portNum, &enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call api with wrong enablePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgVlanPortCfiEnableGet(dev, portNum, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                       "%d, enablePtr = NULL", dev);
        }

        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanPortCfiEnableGet(dev, portNum, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanPortCfiEnableGet(dev, portNum, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanPortCfiEnableGet(dev, portNum, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    /* restore correct values */
    portNum = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortCfiEnableGet(dev, portNum, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortCfiEnableGet(dev, portNum, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortCfiEnableSet
(
    IN  GT_U8       dev,
    IN  GT_PORT_NUM portNum,
    IN  GT_BOOL     enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortCfiEnableSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with enable[GT_TRUE / GT_FALSE],
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgVlanPortCfiEnableGet
    Expected: GT_OK and the same values that was set.
*/
    GT_STATUS st = GT_OK;

    GT_U8       dev;
    GT_PORT_NUM portNum;
    GT_BOOL     enable = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;
    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with enable[GT_TRUE / GT_FALSE],
                Expected: GT_OK.
            */
            enable = GT_TRUE;

            st = cpssDxChBrgVlanPortCfiEnableSet(dev, portNum, enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call cpssDxChBrgVlanPortCfiEnableGet
                Expected: GT_OK and the same values that was set.
            */
            st = cpssDxChBrgVlanPortCfiEnableGet(dev, portNum, &enableGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChBrgVlanPortCfiEnableGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                           "got another enable then was set: %d", dev);

            /* call with enable[GT_FALSE] */
            enable = GT_FALSE;

            st = cpssDxChBrgVlanPortCfiEnableSet(dev, portNum, enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call get func again, 1.1.2. */
            st = cpssDxChBrgVlanPortCfiEnableGet(dev, portNum, &enableGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChBrgVlanPortCfiEnableGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                           "got another enable then was set: %d", dev);
        }

        enable = GT_TRUE;

        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanPortCfiEnableSet(dev, portNum, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanPortCfiEnableSet(dev, portNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanPortCfiEnableSet(dev, portNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    /* restore correct values */
    portNum = 0;
    enable = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortCfiEnableSet(dev, portNum, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortCfiEnableSet(dev, portNum, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortNumOfTagWordsToPopSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_DXCH_BRG_VLAN_NUM_BYTES_TO_POP_ENT numBytesToPop
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortNumOfTagWordsToPopSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with enable[GT_TRUE / GT_FALSE],
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgVlanPortNumOfTagWordsToPopSet
    Expected: GT_OK and the same values that was set.
*/
    GT_STATUS st = GT_OK;

    GT_U8       dev;
    GT_PORT_NUM portNum;
    GT_DXCH_BRG_VLAN_NUM_BYTES_TO_POP_ENT      numberOfBytesInTag;
    GT_DXCH_BRG_VLAN_NUM_BYTES_TO_POP_ENT      numberOfBytesInTagGet;
    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with numberOfBytesInTag = 0,1,2
                Expected: GT_OK.
            */
            for(numberOfBytesInTag  = GT_DXCH_BRG_VLAN_NUM_BYTES_TO_POP_NONE_E ;
                numberOfBytesInTag <= GT_DXCH_BRG_VLAN_NUM_BYTES_TO_POP_6_E    ;
                numberOfBytesInTag++)
            {
                st = cpssDxChBrgVlanPortNumOfTagWordsToPopSet(dev, portNum, numberOfBytesInTag);

                if(numberOfBytesInTag <= GT_DXCH_BRG_VLAN_NUM_BYTES_TO_POP_8_E)
                {
                    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                }
                else
                if(PRV_CPSS_SIP_5_15_CHECK_MAC(dev) &&
                   numberOfBytesInTag == GT_DXCH_BRG_VLAN_NUM_BYTES_TO_POP_6_E)
                {
                    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                }
                else
                {
                    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
                    continue;
                }

                /*
                    1.1.2. Call cpssDxChBrgVlanPortNumOfTagWordsToPopGet
                    Expected: GT_OK and the same values that was set.
                */
                st = cpssDxChBrgVlanPortNumOfTagWordsToPopGet(dev, portNum, &numberOfBytesInTagGet);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                          "cpssDxChBrgVlanPortNumOfTagWordsToPopGet: %d ", dev);

                /* Verifying values */
                UTF_VERIFY_EQUAL1_STRING_MAC(numberOfBytesInTag, numberOfBytesInTagGet,
                               "got another value then was set: %d", dev);
            }

            UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanPortNumOfTagWordsToPopSet
                                (dev, portNum, numberOfBytesInTag),
                                numberOfBytesInTag);

        }

        numberOfBytesInTag = GT_DXCH_BRG_VLAN_NUM_BYTES_TO_POP_4_E;

        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanPortNumOfTagWordsToPopSet(dev, portNum, numberOfBytesInTag);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanPortNumOfTagWordsToPopSet(dev, portNum, numberOfBytesInTag);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanPortNumOfTagWordsToPopSet(dev, portNum, numberOfBytesInTag);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    /* restore correct values */
    portNum = 0;
    numberOfBytesInTag = GT_DXCH_BRG_VLAN_NUM_BYTES_TO_POP_4_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortNumOfTagWordsToPopSet(dev, portNum, numberOfBytesInTag);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortNumOfTagWordsToPopSet(dev, portNum, numberOfBytesInTag);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortNumOfTagWordsToPopGet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    OUT  GT_DXCH_BRG_VLAN_NUM_BYTES_TO_POP_ENT *numBytesToPopPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortNumOfTagWordsToPopGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with non-NULL numberOfBytesInTagPtr.
    Expected: GT_OK.
    1.1.2. Call with wrong numberOfBytesInTagPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st      = GT_OK;
    GT_U8       dev     = 0;
    GT_PORT_NUM portNum = 0;
    GT_U32      notAppFamilyBmp = 0;

    GT_DXCH_BRG_VLAN_NUM_BYTES_TO_POP_ENT      numberOfBytesInTag = GT_DXCH_BRG_VLAN_NUM_BYTES_TO_POP_NONE_E;


    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with non-NULL numberOfBytesInTagPtr.
                Expected: GT_OK.
            */
            st = cpssDxChBrgVlanPortNumOfTagWordsToPopGet(dev, portNum, &numberOfBytesInTag);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);

            /*
                1.1.2. Call with wrong numberOfBytesInTagPtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgVlanPortNumOfTagWordsToPopGet(dev, portNum, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, numberOfBytesInTagPtr = NULL", dev, portNum);
        }

        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanPortNumOfTagWordsToPopGet(dev, portNum, &numberOfBytesInTag);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanPortNumOfTagWordsToPopGet(dev, portNum, &numberOfBytesInTag);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanPortNumOfTagWordsToPopGet(dev, portNum, &numberOfBytesInTag);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    /* restore correct values */
    portNum = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortNumOfTagWordsToPopGet(dev, portNum, &numberOfBytesInTag);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortNumOfTagWordsToPopGet(dev, portNum, &numberOfBytesInTag);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidGet
(
    IN  GT_U8        dev,
    IN  GT_PORT_NUM  portNum,
    OUT GT_BOOL      *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with not NULL enablePtr
    Expected: GT_OK.
    1.1.2. Call api with wrong enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8        dev;
    GT_PORT_NUM  portNum;
    GT_BOOL      enable;
    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with not NULL enablePtr
                Expected: GT_OK.
            */
            st = cpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidGet(dev, portNum, &enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call api with wrong enablePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidGet(dev, portNum, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                       "%d, enablePtr = NULL", dev);
        }

        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidGet(dev, portNum, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidGet(dev, portNum, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidGet(dev, portNum, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    /* restore correct values */
    portNum = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidGet(dev,
                                     portNum, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidGet(dev,
                                     portNum, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidSet
(
    IN  GT_U8       dev,
    IN  GT_PORT_NUM portNum,
    IN  GT_BOOL     enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with enable[GT_TRUE / GT_FALSE],
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidGet
    Expected: GT_OK and the same values that was set.
*/
    GT_STATUS st = GT_OK;

    GT_U8       dev;
    GT_PORT_NUM portNum;
    GT_BOOL     enable = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;
    GT_U32      notAppFamilyBmp;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with enable[GT_TRUE / GT_FALSE],
                Expected: GT_OK.
            */
            enable = GT_TRUE;

            st = cpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidSet(dev, portNum, enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call cpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidGet
                Expected: GT_OK and the same values that was set.
            */
            st = cpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidGet(dev, portNum, &enableGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                           "got another enable then was set: %d", dev);

            /* call with enable[GT_FALSE] */
            enable = GT_FALSE;

            st = cpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidSet(dev, portNum, enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call get func again, 1.1.2. */
            st = cpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidGet(dev, portNum, &enableGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                           "got another enable then was set: %d", dev);
        }

        enable = GT_TRUE;

        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidSet(dev, portNum, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidSet(dev, portNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidSet(dev, portNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    /* restore correct values */
    portNum = 0;
    enable = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidSet(dev,
                                     portNum, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidSet(dev,
                                     portNum, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortPushVlanCommandGet
(
    IN  GT_U8                                     dev,
    IN  GT_PORT_NUM                               portNum,
    OUT CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_ENT  *vlanCmdPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortPushVlanCommandGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with not NULL vlanCmdPtr
    Expected: GT_OK.
    1.1.2. Call api with wrong vlanCmdPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8                                     dev;
    GT_PORT_NUM                               portNum;
    CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_ENT  vlanCmd;
    GT_U32      notAppFamilyBmp;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with not NULL vlanCmdPtr
                Expected: GT_OK.
            */
            st = cpssDxChBrgVlanPortPushVlanCommandGet(dev, portNum, &vlanCmd);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call api with wrong vlanCmdPtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgVlanPortPushVlanCommandGet(dev, portNum, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                       "%d, vlanCmdPtr = NULL", dev);
        }

        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanPortPushVlanCommandGet(dev, portNum, &vlanCmd);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanPortPushVlanCommandGet(dev, portNum, &vlanCmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanPortPushVlanCommandGet(dev, portNum, &vlanCmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    /* restore correct values */
    portNum = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortPushVlanCommandGet(dev, portNum, &vlanCmd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortPushVlanCommandGet(dev, portNum, &vlanCmd);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortPushVlanCommandSet
(
    IN  GT_U8                                    dev,
    IN  GT_PORT_NUM                              portNum,
    IN  CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_ENT vlanCmd
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortPushVlanCommandSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with vlanCmd[CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_NONE_E /
                             CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_PUSH_SINGLE_TAG_ON_TUNNEL_E /
                             CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_PUSH_SINGLE_TAG_ON_PASSENGER_E],
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgVlanPortPushVlanCommandGet
    Expected: GT_OK and the same values than was set.
    1.1.3. Call api with wrong vlanCmd [wrong enum values].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st = GT_OK;

    GT_U8                                    dev;
    GT_PORT_NUM                              portNum;
    CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_ENT vlanCmd;
    CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_ENT vlanCmdGet;
    GT_U32      notAppFamilyBmp;

    vlanCmd = CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_NONE_E;
    vlanCmdGet = CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_NONE_E;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with vlanCmd[CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_NONE_E /
                             CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_PUSH_SINGLE_TAG_ON_TUNNEL_E /
                             CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_PUSH_SINGLE_TAG_ON_PASSENGER_E],
                Expected: GT_OK.
            */

            /* call with vlanCmd[CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_NONE_E] */
            vlanCmd = CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_NONE_E;

            st = cpssDxChBrgVlanPortPushVlanCommandSet(dev, portNum, vlanCmd);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call cpssDxChBrgVlanPortPushVlanCommandGet
                Expected: GT_OK and the same values than was set.
            */
            st = cpssDxChBrgVlanPortPushVlanCommandGet(dev, portNum, &vlanCmdGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChBrgVlanPortPushVlanCommandGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(vlanCmd, vlanCmdGet,
                           "got another vlanCmd then was set: %d", dev);

            /* call with vlanCmd[CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_PUSH_SINGLE_TAG_ON_TUNNEL_E] */
            vlanCmd = CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_PUSH_SINGLE_TAG_ON_TUNNEL_E;

            st = cpssDxChBrgVlanPortPushVlanCommandSet(dev, portNum, vlanCmd);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call get func again, 1.1.2. */
            st = cpssDxChBrgVlanPortPushVlanCommandGet(dev, portNum, &vlanCmdGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChBrgVlanPortPushVlanCommandGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(vlanCmd, vlanCmdGet,
                           "got another vlanCmd then was set: %d", dev);

            if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
            {

                /* call with vlanCmd[CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_PUSH_SINGLE_TAG_ON_PASSENGER_E] */
                vlanCmd = CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_PUSH_SINGLE_TAG_ON_PASSENGER_E;

                st = cpssDxChBrgVlanPortPushVlanCommandSet(dev, portNum, vlanCmd);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /* call get func again, 1.1.2. */
                st = cpssDxChBrgVlanPortPushVlanCommandGet(dev, portNum, &vlanCmdGet);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                          "cpssDxChBrgVlanPortPushVlanCommandGet: %d ", dev);

                /* Verifying values */
                UTF_VERIFY_EQUAL1_STRING_MAC(vlanCmd, vlanCmdGet,
                               "got another vlanCmd then was set: %d", dev);
            }

            /*
                1.1.3. Call api with wrong vlanCmd [wrong enum values].
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanPortPushVlanCommandSet
                                (dev, portNum, vlanCmd),
                                vlanCmd);
        }

        vlanCmd = CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_NONE_E;

        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanPortPushVlanCommandSet(dev, portNum, vlanCmd);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanPortPushVlanCommandSet(dev, portNum, vlanCmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanPortPushVlanCommandSet(dev, portNum, vlanCmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    /* restore correct values */
    portNum = 0;
    vlanCmd = CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_NONE_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortPushVlanCommandSet(dev, portNum, vlanCmd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortPushVlanCommandSet(dev, portNum, vlanCmd);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortPushedTagTpidSelectEgressGet
(
    IN  GT_U8        dev,
    IN  GT_PORT_NUM  portNum,
    OUT GT_U32       *tpidEntryIndexPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortPushedTagTpidSelectEgressGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with not NULL tpidEntryIndexPtr
    Expected: GT_OK.
    1.1.2. Call api with wrong tpidEntryIndexPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8        dev;
    GT_PORT_NUM  portNum;
    GT_U32       tpidEntryIndex;
    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with not NULL tpidEntryIndexPtr
                Expected: GT_OK.
            */
            st = cpssDxChBrgVlanPortPushedTagTpidSelectEgressGet(dev, portNum, &tpidEntryIndex);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call api with wrong tpidEntryIndexPtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgVlanPortPushedTagTpidSelectEgressGet(dev, portNum, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                       "%d, tpidEntryIndexPtr = NULL", dev);
        }

        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanPortPushedTagTpidSelectEgressGet(dev, portNum, &tpidEntryIndex);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanPortPushedTagTpidSelectEgressGet(dev, portNum, &tpidEntryIndex);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanPortPushedTagTpidSelectEgressGet(dev, portNum, &tpidEntryIndex);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    /* restore correct values */
    portNum = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortPushedTagTpidSelectEgressGet(dev,
                                     portNum, &tpidEntryIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortPushedTagTpidSelectEgressGet(dev,
                                     portNum, &tpidEntryIndex);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortPushedTagTpidSelectEgressSet
(
    IN  GT_U8       dev,
    IN  GT_PORT_NUM portNum,
    IN  GT_U32      tpidEntryIndex
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortPushedTagTpidSelectEgressSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with tpidEntryIndex[0 / 5 / BIT_3 - 1],
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgVlanPortPushedTagTpidSelectEgressGet
           with the same parameters.
    Expected: GT_OK and the same values than was set.
    1.1.3. Call api with wrong tpidEntryIndex [BIT_3].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st = GT_OK;

    GT_U8       dev;
    GT_PORT_NUM portNum;
    GT_U32      tpidEntryIndex = 0;
    GT_U32      tpidEntryIndexGet = 0;
    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with tpidEntryIndex[0 / 5 / BIT_3 - 1],
                Expected: GT_OK.
            */

            /* call with tpidEntryIndex[0] */
            tpidEntryIndex = 0;

            st = cpssDxChBrgVlanPortPushedTagTpidSelectEgressSet(dev,
                                         portNum, tpidEntryIndex);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call cpssDxChBrgVlanPortPushedTagTpidSelectEgressGet
                       with the same parameters.
                Expected: GT_OK and the same values than was set.
            */
            st = cpssDxChBrgVlanPortPushedTagTpidSelectEgressGet(dev,
                                         portNum, &tpidEntryIndexGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChBrgVlanPortPushedTagTpidSelectEgressGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(tpidEntryIndex, tpidEntryIndexGet,
                           "got another tpidEntryIndex then was set: %d", dev);

            /* call with tpidEntryIndex[5] */
            tpidEntryIndex = 5;

            st = cpssDxChBrgVlanPortPushedTagTpidSelectEgressSet(dev,
                                         portNum, tpidEntryIndex);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call get func again, 1.1.2. */
            st = cpssDxChBrgVlanPortPushedTagTpidSelectEgressGet(dev,
                                         portNum, &tpidEntryIndexGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChBrgVlanPortPushedTagTpidSelectEgressGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(tpidEntryIndex, tpidEntryIndexGet,
                           "got another tpidEntryIndex then was set: %d", dev);

            /* call with tpidEntryIndex[BIT_3 - 1] */
            tpidEntryIndex = BIT_3 - 1;

            st = cpssDxChBrgVlanPortPushedTagTpidSelectEgressSet(dev,
                                         portNum, tpidEntryIndex);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call get func again, 1.1.2. */
            st = cpssDxChBrgVlanPortPushedTagTpidSelectEgressGet(dev,
                                         portNum, &tpidEntryIndexGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChBrgVlanPortPushedTagTpidSelectEgressGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(tpidEntryIndex, tpidEntryIndexGet,
                           "got another tpidEntryIndex then was set: %d", dev);

            /*
                1.1.3. Call api with wrong tpidEntryIndex [BIT_3].
                Expected: GT_BAD_PARAM.
            */
            tpidEntryIndex = BIT_3;

            st = cpssDxChBrgVlanPortPushedTagTpidSelectEgressSet(dev,
                                         portNum, tpidEntryIndex);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            tpidEntryIndex = 0;
        }

        tpidEntryIndex = 0;

        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanPortPushedTagTpidSelectEgressSet(dev,
                                         portNum, tpidEntryIndex);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanPortPushedTagTpidSelectEgressSet(dev,
                                     portNum, tpidEntryIndex);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanPortPushedTagTpidSelectEgressSet(dev,
                                     portNum, tpidEntryIndex);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    /* restore correct values */
    portNum = 0;
    tpidEntryIndex = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortPushedTagTpidSelectEgressSet(dev,
                                     portNum, tpidEntryIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortPushedTagTpidSelectEgressSet(dev,
                                     portNum, tpidEntryIndex);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortPushedTagValueGet
(
    IN  GT_U8        dev,
    IN  GT_PORT_NUM  portNum,
    OUT GT_U16       *tagValuePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortPushedTagValueGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with not NULL tagValuePtr
    Expected: GT_OK.
    1.1.2. Call api with wrong tagValuePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8        dev;
    GT_PORT_NUM  portNum;
    GT_U16       tagValue;
    GT_U32      notAppFamilyBmp;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with not NULL tagValuePtr
                Expected: GT_OK.
            */
            st = cpssDxChBrgVlanPortPushedTagValueGet(dev, portNum, &tagValue);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call api with wrong tagValuePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgVlanPortPushedTagValueGet(dev, portNum, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                       "%d, tagValuePtr = NULL", dev);
        }

        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanPortPushedTagValueGet(dev, portNum, &tagValue);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanPortPushedTagValueGet(dev, portNum, &tagValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanPortPushedTagValueGet(dev, portNum, &tagValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    /* restore correct values */
    portNum = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortPushedTagValueGet(dev, portNum, &tagValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortPushedTagValueGet(dev, portNum, &tagValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortPushedTagValueSet
(
    IN  GT_U8       dev,
    IN  GT_PORT_NUM portNum,
    IN  GT_U16      tagValue
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortPushedTagValueSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with tagValue[0 / 100 / BIT_12 - 1],
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgVlanPortPushedTagValueGet
           with the same parameters.
    Expected: GT_OK and the same values than was set.
    1.1.3. Call api with wrong tpidEntryIndex [BIT_12].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st = GT_OK;

    GT_U8       dev;
    GT_PORT_NUM portNum = 0;
    GT_U16      tagValue = 0;
    GT_U16      tagValueGet = 0;
    GT_U32      notAppFamilyBmp;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with tagValue[0 / 100 / BIT_12 - 1],
                Expected: GT_OK.
            */

            /* call with tagValue[0] */
            tagValue = 0;

            st = cpssDxChBrgVlanPortPushedTagValueSet(dev,
                                         portNum, tagValue);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call cpssDxChBrgVlanPortPushedTagValueGet
                       with the same parameters.
                Expected: GT_OK and the same values than was set.
            */
            st = cpssDxChBrgVlanPortPushedTagValueGet(dev, portNum, &tagValueGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChBrgVlanPortPushedTagValueGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(tagValue, tagValueGet,
                           "got another tagValue then was set: %d", dev);

            /* call with tagValue[100] */
            tagValue = 100;

            st = cpssDxChBrgVlanPortPushedTagValueSet(dev,
                                         portNum, tagValue);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call get func again, 1.1.2. */
            st = cpssDxChBrgVlanPortPushedTagValueGet(dev, portNum, &tagValueGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChBrgVlanPortPushedTagValueGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(tagValue, tagValueGet,
                           "got another tagValue then was set: %d", dev);

            /* call with tagValue[BIT_12 - 1] */
            tagValue = BIT_12 - 1;

            st = cpssDxChBrgVlanPortPushedTagValueSet(dev,
                                         portNum, tagValue);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call get func again, 1.1.2. */
            st = cpssDxChBrgVlanPortPushedTagValueGet(dev, portNum, &tagValueGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChBrgVlanPortPushedTagValueGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(tagValue, tagValueGet,
                           "got another tagValue then was set: %d", dev);

            /*
                1.1.3. Call api with wrong tagValue [BIT_12].
                Expected: GT_BAD_PARAM.
            */
            tagValue = BIT_12;

            st = cpssDxChBrgVlanPortPushedTagValueSet(dev, portNum, tagValue);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            tagValue = 0;
        }

        tagValue = 0;

        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanPortPushedTagValueSet(dev, portNum, tagValue);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanPortPushedTagValueSet(dev, portNum, tagValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanPortPushedTagValueSet(dev, portNum, tagValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    /* restore correct values */
    portNum = 0;
    tagValue = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortPushedTagValueSet(dev, portNum, tagValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortPushedTagValueSet(dev, portNum, tagValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortUp0CommandEnableGet
(
    IN  GT_U8        dev,
    IN  GT_PORT_NUM  portNum,
    OUT GT_BOOL      *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortUp0CommandEnableGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with not NULL enablePtr
    Expected: GT_OK.
    1.1.2. Call api with wrong enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8        dev;
    GT_PORT_NUM  portNum;
    GT_BOOL      enable;
    GT_U32      notAppFamilyBmp;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with not NULL enablePtr
                Expected: GT_OK.
            */
            st = cpssDxChBrgVlanPortUp0CommandEnableGet(dev, portNum, &enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call api with wrong enablePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgVlanPortUp0CommandEnableGet(dev, portNum, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                       "%d, enablePtr = NULL", dev);
        }

        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanPortUp0CommandEnableGet(dev, portNum, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanPortUp0CommandEnableGet(dev, portNum, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanPortUp0CommandEnableGet(dev, portNum, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    /* restore correct values */
    portNum = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortUp0CommandEnableGet(dev, portNum, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortUp0CommandEnableGet(dev, portNum, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortUp0CommandEnableSet
(
    IN  GT_U8       dev,
    IN  GT_PORT_NUM portNum,
    IN  GT_BOOL     enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortUp0CommandEnableSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with enable[GT_TRUE / GT_FALSE],
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgVlanPortUp0CommandEnableGet
    Expected: GT_OK and the same values that was set.
*/
    GT_STATUS st = GT_OK;

    GT_U8       dev;
    GT_PORT_NUM portNum = 0;
    GT_BOOL     enable = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;
    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with enable[GT_TRUE / GT_FALSE],
                Expected: GT_OK.
            */
            enable = GT_TRUE;

            st = cpssDxChBrgVlanPortUp0CommandEnableSet(dev, portNum, enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call cpssDxChBrgVlanPortUp0CommandEnableGet
                Expected: GT_OK and the same values that was set.
            */
            st = cpssDxChBrgVlanPortUp0CommandEnableGet(dev, portNum, &enableGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChBrgVlanPortUp0CommandEnableGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                           "got another enable then was set: %d", dev);

            /* call with enable[GT_FALSE] */
            enable = GT_FALSE;

            st = cpssDxChBrgVlanPortUp0CommandEnableSet(dev, portNum, enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call get func again, 1.1.2. */
            st = cpssDxChBrgVlanPortUp0CommandEnableGet(dev, portNum, &enableGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChBrgVlanPortUp0CommandEnableGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                           "got another enable then was set: %d", dev);
        }

        enable = GT_TRUE;

        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanPortUp0CommandEnableSet(dev, portNum, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanPortUp0CommandEnableSet(dev, portNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanPortUp0CommandEnableSet(dev, portNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    /* restore correct values */
    portNum = 0;
    enable = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortUp0CommandEnableSet(dev, portNum, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortUp0CommandEnableSet(dev, portNum, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortUp0Get
(
    IN  GT_U8        dev,
    IN  GT_PORT_NUM  portNum,
    OUT GT_U32       *up0Ptr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortUp0Get)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with not NULL up0Ptr
    Expected: GT_OK.
    1.1.2. Call api with wrong up0Ptr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8        dev;
    GT_PORT_NUM  portNum;
    GT_U32       up0;
    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with not NULL up0Ptr
                Expected: GT_OK.
            */
            st = cpssDxChBrgVlanPortUp0Get(dev, portNum, &up0);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call api with wrong up0Ptr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgVlanPortUp0Get(dev, portNum, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                       "%d, up0Ptr = NULL", dev);
        }

        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanPortUp0Get(dev, portNum, &up0);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanPortUp0Get(dev, portNum, &up0);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanPortUp0Get(dev, portNum, &up0);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    /* restore correct values */
    portNum = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortUp0Get(dev, portNum, &up0);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortUp0Get(dev, portNum, &up0);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortUp0Set
(
    IN  GT_U8       dev,
    IN  GT_PORT_NUM portNum,
    IN  GT_U32      up0
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortUp0Set)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with up0 [0 / 5 / BIT_3 - 1],
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgVlanPortUp0Get
           with the same parameters.
    Expected: GT_OK and the same values that was set.
    1.1.3. Call api with wrong up0 [BIT_3].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st = GT_OK;

    GT_U8       dev;
    GT_PORT_NUM portNum = 0;
    GT_U32      up0 = 0;
    GT_U32      up0Get = 0;
    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with up0 [0 / 5 / BIT_3 - 1],
                Expected: GT_OK.
            */

            /* call with up0[0] */
            up0 = 0;

            st = cpssDxChBrgVlanPortUp0Set(dev, portNum, up0);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call cpssDxChBrgVlanPortUp0Get
                       with the same parameters.
                Expected: GT_OK and the same values that was set.
            */
            st = cpssDxChBrgVlanPortUp0Get(dev, portNum, &up0Get);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChBrgVlanPortUp0Get: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(up0, up0Get,
                           "got another up0 then was set: %d", dev);

            /* call with up0[5] */
            up0 = 5;

            st = cpssDxChBrgVlanPortUp0Set(dev, portNum, up0);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call get func again, 1.1.2. */
            st = cpssDxChBrgVlanPortUp0Get(dev,
                                         portNum, &up0Get);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChBrgVlanPortUp0Get: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(up0, up0Get,
                           "got another up0 then was set: %d", dev);

            /* call with up0[BIT_3 - 1] */
            up0 = BIT_3 - 1;

            st = cpssDxChBrgVlanPortUp0Set(dev, portNum, up0);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call get func again, 1.1.2. */
            st = cpssDxChBrgVlanPortUp0Get(dev,
                                         portNum, &up0Get);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChBrgVlanPortUp0Get: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(up0, up0Get,
                           "got another up0 then was set: %d", dev);

            /*
                1.1.3. Call api with wrong up0 [BIT_3].
                Expected: GT_BAD_PARAM.
            */
            up0 = BIT_3;

            st = cpssDxChBrgVlanPortUp0Set(dev, portNum, up0);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            up0 = 0;
        }

        up0 = 0;

        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanPortUp0Set(dev, portNum, up0);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanPortUp0Set(dev, portNum, up0);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanPortUp0Set(dev, portNum, up0);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    /* restore correct values */
    portNum = 0;
    up0 = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortUp0Set(dev, portNum, up0);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortUp0Set(dev, portNum, up0);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortUp1CommandEnableGet
(
    IN  GT_U8        dev,
    IN  GT_PORT_NUM  portNum,
    OUT GT_BOOL      *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortUp1CommandEnableGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with not NULL enablePtr
    Expected: GT_OK.
    1.1.2. Call api with wrong enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8        dev;
    GT_PORT_NUM  portNum;
    GT_BOOL      enable;
    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with not NULL enablePtr
                Expected: GT_OK.
            */
            st = cpssDxChBrgVlanPortUp1CommandEnableGet(dev, portNum, &enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call api with wrong enablePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgVlanPortUp1CommandEnableGet(dev, portNum, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                       "%d, enablePtr = NULL", dev);
        }

        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanPortUp1CommandEnableGet(dev, portNum, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanPortUp1CommandEnableGet(dev, portNum, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanPortUp1CommandEnableGet(dev, portNum, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    /* restore correct values */
    portNum = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortUp1CommandEnableGet(dev, portNum, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortUp1CommandEnableGet(dev, portNum, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortUp1CommandEnableSet
(
    IN  GT_U8       dev,
    IN  GT_PORT_NUM portNum,
    IN  GT_BOOL     enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortUp1CommandEnableSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with enable[GT_TRUE / GT_FALSE],
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgVlanPortUp1CommandEnableGet
    Expected: GT_OK and the same values that was set.
*/
    GT_STATUS st = GT_OK;

    GT_U8       dev;
    GT_PORT_NUM portNum;
    GT_BOOL     enable = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;
    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with enable[GT_TRUE / GT_FALSE],
                Expected: GT_OK.
            */
            enable = GT_TRUE;

            st = cpssDxChBrgVlanPortUp1CommandEnableSet(dev, portNum, enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call cpssDxChBrgVlanPortUp1CommandEnableGet
                Expected: GT_OK and the same values that was set.
            */
            st = cpssDxChBrgVlanPortUp1CommandEnableGet(dev, portNum, &enableGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChBrgVlanPortUp1CommandEnableGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                           "got another enable then was set: %d", dev);

            /* call with enable[GT_FALSE] */
            enable = GT_FALSE;

            st = cpssDxChBrgVlanPortUp1CommandEnableSet(dev, portNum, enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call get func again, 1.1.2. */
            st = cpssDxChBrgVlanPortUp1CommandEnableGet(dev, portNum, &enableGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChBrgVlanPortUp1CommandEnableGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                           "got another enable then was set: %d", dev);
        }

        enable = GT_TRUE;

        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanPortUp1CommandEnableSet(dev, portNum, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanPortUp1CommandEnableSet(dev, portNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanPortUp1CommandEnableSet(dev, portNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    /* restore correct values */
    portNum = 0;
    enable = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortUp1CommandEnableSet(dev, portNum, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortUp1CommandEnableSet(dev, portNum, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortUp1Get
(
    IN  GT_U8        dev,
    IN  GT_PORT_NUM  portNum,
    OUT GT_U32       *up1Ptr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortUp1Get)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with not NULL up1Ptr
    Expected: GT_OK.
    1.1.2. Call api with wrong up1Ptr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8        dev;
    GT_PORT_NUM  portNum;
    GT_U32       up1;
    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with not NULL up1Ptr
                Expected: GT_OK.
            */
            st = cpssDxChBrgVlanPortUp1Get(dev, portNum, &up1);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call api with wrong up1Ptr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgVlanPortUp1Get(dev, portNum, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                       "%d, up1Ptr = NULL", dev);
        }

        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanPortUp1Get(dev, portNum, &up1);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanPortUp1Get(dev, portNum, &up1);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanPortUp1Get(dev, portNum, &up1);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    /* restore correct values */
    portNum = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortUp1Get(dev, portNum, &up1);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortUp1Get(dev, portNum, &up1);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortUp1Set
(
    IN  GT_U8       dev,
    IN  GT_PORT_NUM portNum,
    IN  GT_U32      up1
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortUp1Set)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with up1 [0 / 5 / BIT_3 - 1],
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgVlanPortUp1Get
           with the same parameters.
    Expected: GT_OK and the same values that was set.
    1.1.3. Call api with wrong up1 [BIT_3].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st = GT_OK;

    GT_U8       dev;
    GT_PORT_NUM portNum = 0;
    GT_U32      up1 = 0;
    GT_U32      up1Get = 0;
    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with up1 [0 / 5 / BIT_3 - 1],
                Expected: GT_OK.
            */

            /* call with up1[0] */
            up1 = 0;

            st = cpssDxChBrgVlanPortUp1Set(dev, portNum, up1);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call cpssDxChBrgVlanPortUp1Get
                       with the same parameters.
                Expected: GT_OK and the same values that was set.
            */
            st = cpssDxChBrgVlanPortUp1Get(dev, portNum, &up1Get);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChBrgVlanPortUp1Get: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(up1, up1Get,
                           "got another up1 then was set: %d", dev);

            /* call with up1[5] */
            up1 = 5;

            st = cpssDxChBrgVlanPortUp1Set(dev, portNum, up1);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call get func again, 1.1.2. */
            st = cpssDxChBrgVlanPortUp1Get(dev,
                                         portNum, &up1Get);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChBrgVlanPortUp1Get: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(up1, up1Get,
                           "got another up1 then was set: %d", dev);

            /* call with up1[BIT_3 - 1] */
            up1 = BIT_3 - 1;

            st = cpssDxChBrgVlanPortUp1Set(dev, portNum, up1);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call get func again, 1.1.2. */
            st = cpssDxChBrgVlanPortUp1Get(dev,
                                         portNum, &up1Get);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChBrgVlanPortUp1Get: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(up1, up1Get,
                           "got another up1 then was set: %d", dev);

            /*
                1.1.3. Call api with wrong up1 [BIT_3].
                Expected: GT_BAD_PARAM.
            */
            up1 = BIT_3;

            st = cpssDxChBrgVlanPortUp1Set(dev, portNum, up1);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            up1 = 0;
        }

        up1 = 0;

        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanPortUp1Set(dev, portNum, up1);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanPortUp1Set(dev, portNum, up1);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanPortUp1Set(dev, portNum, up1);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    /* restore correct values */
    portNum = 0;
    up1 = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortUp1Set(dev, portNum, up1);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortUp1Set(dev, portNum, up1);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortUpAndCfiAssignmentCommandGet
(
    IN  GT_U8        dev,
    IN  GT_PORT_NUM  portNum,
    OUT GT_BOOL      *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortUpAndCfiAssignmentCommandGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with not NULL enablePtr
    Expected: GT_OK.
    1.1.2. Call api with wrong enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8        dev;
    GT_PORT_NUM  portNum;
    GT_BOOL      enable;
    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with not NULL enablePtr
                Expected: GT_OK.
            */
            st = cpssDxChBrgVlanPortUpAndCfiAssignmentCommandGet(dev, portNum, &enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call api with wrong enablePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgVlanPortUpAndCfiAssignmentCommandGet(dev, portNum, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                       "%d, enablePtr = NULL", dev);
        }

        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanPortUpAndCfiAssignmentCommandGet(dev, portNum, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanPortUpAndCfiAssignmentCommandGet(dev, portNum, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanPortUpAndCfiAssignmentCommandGet(dev, portNum, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    /* restore correct values */
    portNum = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortUpAndCfiAssignmentCommandGet(dev,
                                     portNum, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortUpAndCfiAssignmentCommandGet(dev,
                                     portNum, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortUpAndCfiAssignmentCommandSet
(
    IN  GT_U8       dev,
    IN  GT_PORT_NUM portNum,
    IN  GT_BOOL     enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortUpAndCfiAssignmentCommandSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with enable[GT_TRUE / GT_FALSE],
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgVlanPortUpAndCfiAssignmentCommandGet
    Expected: GT_OK and the same values that was set.
*/
    GT_STATUS st = GT_OK;

    GT_U8       dev;
    GT_PORT_NUM portNum;
    GT_BOOL     enable = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;
    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with enable[GT_TRUE / GT_FALSE],
                Expected: GT_OK.
            */
            enable = GT_TRUE;

            st = cpssDxChBrgVlanPortUpAndCfiAssignmentCommandSet(dev, portNum, enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call cpssDxChBrgVlanPortUpAndCfiAssignmentCommandGet
                Expected: GT_OK and the same values that was set.
            */
            st = cpssDxChBrgVlanPortUpAndCfiAssignmentCommandGet(dev, portNum, &enableGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChBrgVlanPortUpAndCfiAssignmentCommandGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                           "got another enable then was set: %d", dev);

            /* call with enable[GT_FALSE] */
            enable = GT_FALSE;

            st = cpssDxChBrgVlanPortUpAndCfiAssignmentCommandSet(dev, portNum, enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call get func again, 1.1.2. */
            st = cpssDxChBrgVlanPortUpAndCfiAssignmentCommandGet(dev, portNum, &enableGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChBrgVlanPortUpAndCfiAssignmentCommandGet %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                           "got another enable then was set: %d", dev);
        }

        enable = GT_TRUE;

        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanPortUpAndCfiAssignmentCommandSet(dev, portNum, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanPortUpAndCfiAssignmentCommandSet(dev, portNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanPortUpAndCfiAssignmentCommandSet(dev, portNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    /* restore correct values */
    portNum = 0;
    enable = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortUpAndCfiAssignmentCommandSet(dev,
                                     portNum, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortUpAndCfiAssignmentCommandSet(dev,
                                     portNum, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortUpGet
(
    IN  GT_U8        dev,
    IN  GT_PORT_NUM  portNum,
    OUT GT_U32       *upPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortUpGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with not NULL upPtr
    Expected: GT_OK.
    1.1.2. Call api with wrong upPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8        dev;
    GT_PORT_NUM  portNum = 0;
    GT_U32       up;
    GT_U32      notAppFamilyBmp;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with not NULL upPtr
                Expected: GT_OK.
            */
            st = cpssDxChBrgVlanPortUpGet(dev, portNum, &up);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call api with wrong upPtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgVlanPortUpGet(dev, portNum, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                       "%d, upPtr = NULL", dev);
        }

        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanPortUpGet(dev, portNum, &up);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanPortUpGet(dev, portNum, &up);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanPortUpGet(dev, portNum, &up);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    /* restore correct values */
    portNum = BRG_VLAN_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortUpGet(dev, portNum, &up);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortUpGet(dev, portNum, &up);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortUpSet
(
    IN  GT_U8       dev,
    IN  GT_PORT_NUM portNum,
    IN  GT_U32      up
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortUpSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with up [0 / 5 / BIT_3 - 1],
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgVlanPortUpGet
           with the same parameters.
    Expected: GT_OK and the same values that was set.
    1.1.3. Call api with wrong up [BIT_3].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st = GT_OK;

    GT_U8       dev;
    GT_PORT_NUM portNum;
    GT_U32      up = 0;
    GT_U32      upGet = 0;
    GT_U32      notAppFamilyBmp;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with up [0 / 5 / BIT_3 - 1],
                Expected: GT_OK.
            */

            /* call with up[0] */
            up = 0;

            st = cpssDxChBrgVlanPortUpSet(dev, portNum, up);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call cpssDxChBrgVlanPortUpGet
                       with the same parameters.
                Expected: GT_OK and the same values that was set.
            */
            st = cpssDxChBrgVlanPortUpGet(dev, portNum, &upGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChBrgVlanPortUpGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(up, upGet,
                           "got another up then was set: %d", dev);

            /* call with up[5] */
            up = 5;

            st = cpssDxChBrgVlanPortUpSet(dev, portNum, up);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call get func again, 1.1.2. */
            st = cpssDxChBrgVlanPortUpGet(dev,
                                         portNum, &upGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChBrgVlanPortUpGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(up, upGet,
                           "got another up then was set: %d", dev);

            /* call with up[BIT_3 - 1] */
            up = BIT_3 - 1;

            st = cpssDxChBrgVlanPortUpSet(dev, portNum, up);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call get func again, 1.1.2. */
            st = cpssDxChBrgVlanPortUpGet(dev,
                                         portNum, &upGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChBrgVlanPortUpGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(up, upGet,
                           "got another up then was set: %d", dev);

            /*
                1.1.3. Call api with wrong up [BIT_3].
                Expected: GT_BAD_PARAM.
            */
            up = BIT_3;

            st = cpssDxChBrgVlanPortUpSet(dev, portNum, up);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            up = 0;
        }

        up = 0;

        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanPortUpSet(dev, portNum, up);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanPortUpSet(dev, portNum, up);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanPortUpSet(dev, portNum, up);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    /* restore correct values */
    portNum = BRG_VLAN_VALID_PHY_PORT_CNS;
    up = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortUpSet(dev, portNum, up);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortUpSet(dev, portNum, up);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortVid0CommandEnableGet
(
    IN  GT_U8        dev,
    IN  GT_PORT_NUM  portNum,
    OUT GT_BOOL      *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortVid0CommandEnableGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with not NULL enablePtr
    Expected: GT_OK.
    1.1.2. Call api with wrong enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8        dev;
    GT_PORT_NUM  portNum;
    GT_BOOL      enable;
    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with not NULL enablePtr
                Expected: GT_OK.
            */
            st = cpssDxChBrgVlanPortVid0CommandEnableGet(dev, portNum, &enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call api with wrong enablePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgVlanPortVid0CommandEnableGet(dev, portNum, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                       "%d, enablePtr = NULL", dev);
        }

        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanPortVid0CommandEnableGet(dev, portNum, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanPortVid0CommandEnableGet(dev, portNum, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanPortVid0CommandEnableGet(dev, portNum, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    /* restore correct values */
    portNum = BRG_VLAN_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortVid0CommandEnableGet(dev, portNum, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortVid0CommandEnableGet(dev, portNum, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortVid0CommandEnableSet
(
    IN  GT_U8       dev,
    IN  GT_PORT_NUM portNum,
    IN  GT_BOOL     enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortVid0CommandEnableSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with enable[GT_TRUE / GT_FALSE],
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgVlanPortVid0CommandEnableGet
    Expected: GT_OK and the same values that was set.
*/
    GT_STATUS st = GT_OK;

    GT_U8       dev;
    GT_PORT_NUM portNum = BRG_VLAN_VALID_PHY_PORT_CNS;
    GT_BOOL     enable = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;
    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with enable[GT_TRUE / GT_FALSE],
                Expected: GT_OK.
            */
            enable = GT_TRUE;

            st = cpssDxChBrgVlanPortVid0CommandEnableSet(dev, portNum, enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call cpssDxChBrgVlanPortVid0CommandEnableGet
                Expected: GT_OK and the same values that was set.
            */
            st = cpssDxChBrgVlanPortVid0CommandEnableGet(dev, portNum, &enableGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChBrgVlanPortVid0CommandEnableGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                           "got another enable then was set: %d", dev);

            /* call with enable[GT_FALSE] */
            enable = GT_FALSE;

            st = cpssDxChBrgVlanPortVid0CommandEnableSet(dev, portNum, enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call get func again, 1.1.2. */
            st = cpssDxChBrgVlanPortVid0CommandEnableGet(dev, portNum, &enableGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChBrgVlanPortVid0CommandEnableGet %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                           "got another enable then was set: %d", dev);
        }

        enable = GT_TRUE;

        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanPortVid0CommandEnableSet(dev, portNum, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanPortVid0CommandEnableSet(dev, portNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanPortVid0CommandEnableSet(dev, portNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    /* restore correct values */
    portNum = BRG_VLAN_VALID_PHY_PORT_CNS;
    enable = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortVid0CommandEnableSet(dev, portNum, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortVid0CommandEnableSet(dev, portNum, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortVid1CommandEnableGet
(
    IN  GT_U8        dev,
    IN  GT_PORT_NUM  portNum,
    OUT GT_BOOL      *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortVid1CommandEnableGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with not NULL enablePtr
    Expected: GT_OK.
    1.1.2. Call api with wrong enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8        dev;
    GT_PORT_NUM  portNum;
    GT_BOOL      enable;
    GT_U32      notAppFamilyBmp;


    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with not NULL enablePtr
                Expected: GT_OK.
            */
            st = cpssDxChBrgVlanPortVid1CommandEnableGet(dev, portNum, &enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call api with wrong enablePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgVlanPortVid1CommandEnableGet(dev, portNum, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                       "%d, enablePtr = NULL", dev);
        }

        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanPortVid1CommandEnableGet(dev, portNum, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanPortVid1CommandEnableGet(dev, portNum, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanPortVid1CommandEnableGet(dev, portNum, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    /* restore correct values */
    portNum = BRG_VLAN_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortVid1CommandEnableGet(dev, portNum, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortVid1CommandEnableGet(dev, portNum, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortVid1CommandEnableSet
(
    IN  GT_U8       dev,
    IN  GT_PORT_NUM portNum,
    IN  GT_BOOL     enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortVid1CommandEnableSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with enable[GT_TRUE / GT_FALSE],
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgVlanPortVid1CommandEnableGet
    Expected: GT_OK and the same values that was set.
*/
    GT_STATUS st = GT_OK;

    GT_U8       dev;
    GT_PORT_NUM portNum = BRG_VLAN_VALID_PHY_PORT_CNS;
    GT_BOOL     enable = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;
    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with enable[GT_TRUE / GT_FALSE],
                Expected: GT_OK.
            */
            enable = GT_TRUE;

            st = cpssDxChBrgVlanPortVid1CommandEnableSet(dev, portNum, enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call cpssDxChBrgVlanPortVid1CommandEnableGet
                Expected: GT_OK and the same values that was set.
            */
            st = cpssDxChBrgVlanPortVid1CommandEnableGet(dev, portNum, &enableGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChBrgVlanPortVid1CommandEnableGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                           "got another enable then was set: %d", dev);

            /* call with enable[GT_FALSE] */
            enable = GT_FALSE;

            st = cpssDxChBrgVlanPortVid1CommandEnableSet(dev, portNum, enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call get func again, 1.1.2. */
            st = cpssDxChBrgVlanPortVid1CommandEnableGet(dev, portNum, &enableGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChBrgVlanPortVid1CommandEnableGet %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                           "got another enable then was set: %d", dev);
        }

        enable = GT_TRUE;

        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanPortVid1CommandEnableSet(dev, portNum, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanPortVid1CommandEnableSet(dev, portNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanPortVid1CommandEnableSet(dev, portNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    /* restore correct values */
    portNum = BRG_VLAN_VALID_PHY_PORT_CNS;
    enable = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortVid1CommandEnableSet(dev, portNum, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortVid1CommandEnableSet(dev, portNum, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortVid1Get
(
    IN  GT_U8        dev,
    IN  GT_PORT_NUM  portNum,
    OUT GT_U16       *vid1Ptr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortVid1Get)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with not NULL vid1Ptr
    Expected: GT_OK.
    1.1.2. Call api with wrong vid1Ptr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8        dev;
    GT_PORT_NUM  portNum;
    GT_U16       vid1;
    GT_U32      notAppFamilyBmp;
    CPSS_DIRECTION_ENT          direction;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(direction = CPSS_DIRECTION_INGRESS_E ; direction <= CPSS_DIRECTION_EGRESS_E; direction++)
        {
            prvUtfNextGenericPortItaratorTypeSet(   direction == CPSS_DIRECTION_INGRESS_E ?
                                                    UTF_GENERIC_PORT_ITERATOR_TYPE_DEFAULT_EPORT_E :
                                                    UTF_GENERIC_PORT_ITERATOR_TYPE_VIRTUAL_E);

            st = prvUtfNextGenericPortReset(&portNum, dev);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* 1.1. For all active devices go over all available virtual ports. */
            while (GT_OK == prvUtfNextGenericPortGet(&portNum, GT_TRUE))
            {
                /*
                    1.1.1. Call with not NULL vid1Ptr
                    Expected: GT_OK.
                */
                st = cpssDxChBrgVlanPortVid1Get(dev, portNum, direction,&vid1);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /*
                    1.1.2. Call api with wrong vid1Ptr [NULL].
                    Expected: GT_BAD_PTR.
                */
                st = cpssDxChBrgVlanPortVid1Get(dev, portNum, direction,NULL);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                           "%d, vid1Ptr = NULL", dev);
            }

            st = prvUtfNextGenericPortReset(&portNum, dev);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* 1.2. For all active devices go over all non available virtual ports. */
            while (GT_OK == prvUtfNextGenericPortGet(&portNum, GT_FALSE))
            {
                /* 1.2.1. Call function for each non-active port */
                st = cpssDxChBrgVlanPortVid1Get(dev, portNum, direction,&vid1);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
            }

            /* 1.3. For active device check that function returns GT_BAD_PARAM */
            /* for out of bound value for port number.                         */
            portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

            st = cpssDxChBrgVlanPortVid1Get(dev, portNum, direction,&vid1);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

            /* 1.4. For active device check that function returns GT_OK */
            /* for CPU port number.                                            */
            portNum = CPSS_CPU_PORT_NUM_CNS;

            st = cpssDxChBrgVlanPortVid1Get(dev, portNum, direction,&vid1);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
        }
    }

    direction = CPSS_DIRECTION_INGRESS_E;

    /* restore correct values */
    portNum = BRG_VLAN_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortVid1Get(dev, portNum, direction,&vid1);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortVid1Get(dev, portNum, direction,&vid1);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortVid1Set
(
    IN  GT_U8       dev,
    IN  GT_PORT_NUM portNum,
    IN  GT_U16      vid1
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortVid1Set)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with vid1 [ 0 / BRG_VLAN_MAX_VALUE_MAC(dev) ].
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgVlanPortVid1Get
    Expected: GT_OK and the same values that was set.
    1.1.3. Call with out of range vid1 [BRG_VLAN_MAX_VALUE_MAC(dev) + 1]
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st = GT_OK;

    GT_U8       dev;
    GT_PORT_NUM portNum = BRG_VLAN_VALID_PHY_PORT_CNS;
    GT_U16      vid1 = 0;
    GT_U16      vid1Get = 0;
    GT_U32      notAppFamilyBmp;
    CPSS_DIRECTION_ENT          direction;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(direction = CPSS_DIRECTION_INGRESS_E ; direction <= CPSS_DIRECTION_EGRESS_E; direction++)
        {
            prvUtfNextGenericPortItaratorTypeSet(   direction == CPSS_DIRECTION_INGRESS_E ?
                                                    UTF_GENERIC_PORT_ITERATOR_TYPE_DEFAULT_EPORT_E :
                                                    UTF_GENERIC_PORT_ITERATOR_TYPE_VIRTUAL_E);

            st = prvUtfNextGenericPortReset(&portNum, dev);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* 1.1. For all active devices go over all available virtual ports. */
            while (GT_OK == prvUtfNextGenericPortGet(&portNum, GT_TRUE))
            {
                /*
                    1.1.1. Call with vid1 [ 0 / BRG_VLAN_MAX_VALUE_MAC(dev) ].
                    Expected: GT_OK.
                */
                /* call with vid1 [0] */
                vid1 = 0;

                st = cpssDxChBrgVlanPortVid1Set(dev, portNum, direction,vid1);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /*
                    1.1.2. Call cpssDxChBrgVlanPortVid1Get
                    Expected: GT_OK and the same values that was set.
                */
                st = cpssDxChBrgVlanPortVid1Get(dev, portNum, direction,&vid1Get);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                          "cpssDxChBrgVlanPortVid1Get: %d ", dev);

                /* Verifying values */
                UTF_VERIFY_EQUAL1_STRING_MAC(vid1, vid1Get,
                               "got another vid1 then was set: %d", dev);

                /* call with vid1 [BRG_VLAN_MAX_VALUE_MAC(dev)] */
                vid1 = PRV_CPSS_MAX_NUM_VLANS_CNS - 1;

                st = cpssDxChBrgVlanPortVid1Set(dev, portNum, direction,vid1);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /*
                    1.1.2. Call cpssDxChBrgVlanPortVid1Get
                    Expected: GT_OK and the same values that was set.
                */
                st = cpssDxChBrgVlanPortVid1Get(dev, portNum, direction,&vid1Get);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                          "cpssDxChBrgVlanPortVid1Get: %d ", dev);

                /* Verifying values */
                UTF_VERIFY_EQUAL1_STRING_MAC(vid1, vid1Get,
                               "got another vid1 then was set: %d", dev);

                /*
                    1.1.3. Call with out of range vid1 [BRG_VLAN_MAX_VALUE_MAC(dev) + 1]
                    Expected: GT_BAD_PARAM.
                */
                vid1 = PRV_CPSS_MAX_NUM_VLANS_CNS;

                st = cpssDxChBrgVlanPortVid1Set(dev, portNum, direction,vid1);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
            }

            vid1 = 0;

            st = prvUtfNextGenericPortReset(&portNum, dev);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* 1.2. For all active devices go over all non available virtual ports. */
            while (GT_OK == prvUtfNextGenericPortGet(&portNum, GT_FALSE))
            {
                /* 1.2.1. Call function for each non-active port */
                st = cpssDxChBrgVlanPortVid1Set(dev, portNum, direction,vid1);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
            }

            /* 1.3. For active device check that function returns GT_BAD_PARAM */
            /* for out of bound value for port number.                         */
            portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

            st = cpssDxChBrgVlanPortVid1Set(dev, portNum, direction,vid1);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

            /* 1.4. For active device check that function returns GT_OK */
            /* for CPU port number.                                            */
            portNum = CPSS_CPU_PORT_NUM_CNS;

            st = cpssDxChBrgVlanPortVid1Set(dev, portNum, direction,vid1);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
        }
    }

    direction = CPSS_DIRECTION_INGRESS_E;

    /* restore correct values */
    portNum = BRG_VLAN_VALID_PHY_PORT_CNS;
    vid1 = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortVid1Set(dev, portNum, direction,vid1);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortVid1Set(dev, portNum, direction,vid1);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanUnknownMacSaCommandGet
(
    IN  GT_U8                dev,
    IN  GT_U16               vlanId,
    OUT CPSS_PACKET_CMD_ENT  *cmdPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanUnknownMacSaCommandGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with vlanId [ 0 / BRG_VLAN_MAX_VALUE_MAC(dev) ].
    Expected: GT_OK.
    1.2. Call with out of range vlanId [BRG_VLAN_MAX_VALUE_MAC(dev) + 1]
    Expected: GT_BAD_PARAM.
    1.4. Call api with wrong cmdPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8                dev;
    GT_U16               vlanId;
    CPSS_PACKET_CMD_ENT  cmd;
    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with vlanId [ 0 / BRG_VLAN_MAX_VALUE_MAC(dev) ].
            Expected: GT_OK.
        */
        /* call with vlanId [0] */
        vlanId = 0;

        st = cpssDxChBrgVlanUnknownMacSaCommandGet(dev, vlanId, &cmd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* call with vlanId [BRG_VLAN_MAX_VALUE_MAC(dev)] */
        vlanId = (GT_U16) (UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev) - 1);

        st = cpssDxChBrgVlanUnknownMacSaCommandGet(dev, vlanId, &cmd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with out of range vlanId [BRG_VLAN_MAX_VALUE_MAC(dev) + 1]
            Expected: GT_BAD_PARAM.
        */
        vlanId = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev);

        st = cpssDxChBrgVlanUnknownMacSaCommandGet(dev, vlanId, &cmd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /*
            1.3. Call api with wrong cmdPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgVlanUnknownMacSaCommandGet(dev, vlanId, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                   "%d, cmdPtr = NULL", dev);
    }

    /* restore correct values */
    vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanUnknownMacSaCommandGet(dev, vlanId, &cmd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanUnknownMacSaCommandGet(dev, vlanId, &cmd);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanUnknownMacSaCommandSet
(
    IN  GT_U8               dev,
    IN  GT_U16              vlanId,
    IN  CPSS_PACKET_CMD_ENT cmd
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanUnknownMacSaCommandSet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with vlanId [ 0 / BRG_VLAN_MAX_VALUE_MAC(dev) ],
                   cmd [ CPSS_PACKET_CMD_FORWARD_E ]
    Expected: GT_OK.
    1.2. Call with vlanId [ BRG_VLAN_MAX_VALUE_MAC(dev) ],
                   cmd [ CPSS_PACKET_CMD_DROP_HARD_E ]
    Expected: GT_OK.
    1.3. Call with out of range vlanId [BRG_VLAN_MAX_VALUE_MAC(dev) + 1],
                                cmd [ CPSS_PACKET_CMD_TRAP_TO_CPU_E ]
    1.4. Call with vlanId [0],
                   cmd[CPSS_PACKET_CMD_MIRROR_TO_CPU_E /
                       CPSS_PACKET_CMD_TRAP_TO_CPU_E /
                       CPSS_PACKET_CMD_DROP_HARD_E /
                       CPSS_PACKET_CMD_DROP_SOFT_E],
    Expected: GT_OK.
    1.5. Call cpssDxChBrgVlanUnknownMacSaCommandGet
         with the same parameters.
    Expected: GT_OK and the same values than was set.
    1.6. Call with vlanId [ BRG_VLAN_MAX_VALUE_MAC(dev) ],
                   wrong cmd [ CPSS_PACKET_CMD_BRIDGE_E ]
    Expected: NOT GT_OK.
    1.7. Call api with wrong cmd [wrong enum values].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st = GT_OK;

    GT_U8               dev;
    GT_U16              vlanId = 0;
    CPSS_PACKET_CMD_ENT cmd = CPSS_PACKET_CMD_FORWARD_E;
    CPSS_PACKET_CMD_ENT cmdGet = CPSS_PACKET_CMD_FORWARD_E;
    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with vlanId [ 0 / BRG_VLAN_MAX_VALUE_MAC(dev) ],
                           cmd [ CPSS_PACKET_CMD_FORWARD_E ]
            Expected: GT_OK.
        */
        /* call with vlanId [0] */
        vlanId = 0;
        cmd = CPSS_PACKET_CMD_FORWARD_E;

        st = cpssDxChBrgVlanUnknownMacSaCommandSet(dev, vlanId, cmd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.5. Call cpssDxChBrgVlanUnknownMacSaCommandGet
                   with the same parameters.
            Expected: GT_OK and the same values than was set.
        */
        st = cpssDxChBrgVlanUnknownMacSaCommandGet(dev, vlanId, &cmdGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                  "cpssDxChBrgVlanUnknownMacSaCommandGet: %d ", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(cmd, cmdGet,
                       "got another cmd then was set: %d", dev);

        /*
            1.2. Call with vlanId [ BRG_VLAN_MAX_VALUE_MAC(dev) ],
                           cmd [ CPSS_PACKET_CMD_DROP_HARD_E ]
            Expected: GT_OK.
        */
        vlanId = (GT_U16) (UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev) - 1);
        cmd = CPSS_PACKET_CMD_DROP_HARD_E;

        st = cpssDxChBrgVlanUnknownMacSaCommandSet(dev, vlanId, cmd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.5. Call cpssDxChBrgVlanUnknownMacSaCommandGet
                   with the same parameters.
            Expected: GT_OK and the same values than was set.
        */
        st = cpssDxChBrgVlanUnknownMacSaCommandGet(dev, vlanId, &cmdGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                  "cpssDxChBrgVlanUnknownMacSaCommandGet: %d ", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(cmd, cmdGet,
                       "got another cmd then was set: %d", dev);

        /*
            1.3. Call with out of range vlanId [BRG_VLAN_MAX_VALUE_MAC(dev) + 1],
                                        cmd [ CPSS_PACKET_CMD_TRAP_TO_CPU_E ]
            Expected: GT_BAD_PARAM.
        */
        vlanId = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev);
        cmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;

        st = cpssDxChBrgVlanUnknownMacSaCommandSet(dev, vlanId, cmd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /*
            1.4. Call with vlanId [0],
                           cmd[CPSS_PACKET_CMD_MIRROR_TO_CPU_E /
                               CPSS_PACKET_CMD_TRAP_TO_CPU_E /
                               CPSS_PACKET_CMD_DROP_HARD_E /
                               CPSS_PACKET_CMD_DROP_SOFT_E],
            Expected: GT_OK.
        */

        /* call with vlanId[0],
                     cmd[CPSS_PACKET_CMD_MIRROR_TO_CPU_E] */
        vlanId = 0;
        cmd = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;

        st = cpssDxChBrgVlanUnknownMacSaCommandSet(dev, vlanId, cmd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.5. Call cpssDxChBrgVlanUnknownMacSaCommandGet
                   with the same parameters.
            Expected: GT_OK and the same values than was set.
        */
        st = cpssDxChBrgVlanUnknownMacSaCommandGet(dev, vlanId, &cmdGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                  "cpssDxChBrgVlanUnknownMacSaCommandGet: %d ", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(cmd, cmdGet,
                       "got another cmd then was set: %d", dev);

        /* call with vlanId[0],
                     cmd[CPSS_PACKET_CMD_TRAP_TO_CPU_E] */
        vlanId = 0;
        cmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;

        st = cpssDxChBrgVlanUnknownMacSaCommandSet(dev, vlanId, cmd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* call get func again, 1.5. */
        st = cpssDxChBrgVlanUnknownMacSaCommandGet(dev, vlanId, &cmdGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                  "cpssDxChBrgVlanUnknownMacSaCommandGet: %d ", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(cmd, cmdGet,
                       "got another cmd then was set: %d", dev);

        /* call with vlanId[0],
                     cmd[CPSS_PACKET_CMD_DROP_HARD_E] */
        vlanId = 0;
        cmd = CPSS_PACKET_CMD_DROP_HARD_E;

        st = cpssDxChBrgVlanUnknownMacSaCommandSet(dev, vlanId, cmd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* call get func again, 1.5. */
        st = cpssDxChBrgVlanUnknownMacSaCommandGet(dev, vlanId, &cmdGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                  "cpssDxChBrgVlanUnknownMacSaCommandGet: %d ", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(cmd, cmdGet,
                       "got another cmd then was set: %d", dev);

        /* call with vlanId[0],
                     cmd[CPSS_PACKET_CMD_DROP_SOFT_E] */
        vlanId = 0;
        cmd = CPSS_PACKET_CMD_DROP_SOFT_E;

        st = cpssDxChBrgVlanUnknownMacSaCommandSet(dev, vlanId, cmd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* call get func again, 1.5. */
        st = cpssDxChBrgVlanUnknownMacSaCommandGet(dev, vlanId, &cmdGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                  "cpssDxChBrgVlanUnknownMacSaCommandGet: %d ", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(cmd, cmdGet,
                       "got another cmd then was set: %d", dev);

        vlanId = 0;

        /*
            1.6. Call with vlanId [ BRG_VLAN_MAX_VALUE_MAC(dev) ],
                           wrong cmd [ CPSS_PACKET_CMD_BRIDGE_E ]
            Expected: GT_OK.
        */
        vlanId = (GT_U16) (UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev) - 1);
        cmd = CPSS_PACKET_CMD_BRIDGE_E;

        st = cpssDxChBrgVlanUnknownMacSaCommandSet(dev, vlanId, cmd);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.7. Call api with wrong cmd [wrong enum values].
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanUnknownMacSaCommandSet
                            (dev, vlanId, cmd),
                            cmd);
    }

    /* restore correct values */
    vlanId = 0;
    cmd = CPSS_PACKET_CMD_FORWARD_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanUnknownMacSaCommandSet(dev, vlanId, cmd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanUnknownMacSaCommandSet(dev, vlanId, cmd);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanIndependentNonFloodVidxEnableSet
(
    IN  GT_U8            devNum,
    IN  GT_BOOL          enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanIndependentNonFloodVidxEnableSet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgVlanIndependentNonFloodVidxEnableGet with not null enablePtr.
    Expected: GT_OK and the same enable.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;
    GT_U32      notAppFamilyBmp = 0;

    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;


    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with enable [GT_FALSE / GT_TRUE].
            Expected: GT_OK.
        */
        /* Call with enable [GT_FALSE] */
        enable = GT_FALSE;

        st = cpssDxChBrgVlanIndependentNonFloodVidxEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*
            1.2. Call cpssDxChBrgVlanIndependentNonFloodVidxEnableGet with not null enablePtr.
            Expected: GT_OK and the same enable.
        */
        st = cpssDxChBrgVlanIndependentNonFloodVidxEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgVlanIndependentNonFloodVidxEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                       "get another enable than was set: %d", dev);

        /* Call with enable [GT_TRUE] */
        enable = GT_TRUE;

        st = cpssDxChBrgVlanIndependentNonFloodVidxEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*
            1.2. Call cpssDxChBrgVlanIndependentNonFloodVidxEnableGet with not null enablePtr.
            Expected: GT_OK and the same enable.
        */
        st = cpssDxChBrgVlanIndependentNonFloodVidxEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgVlanIndependentNonFloodVidxEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                       "get another enable than was set: %d", dev);
    }

    enable = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanIndependentNonFloodVidxEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanIndependentNonFloodVidxEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanIndependentNonFloodVidxEnableGet
(
    IN  GT_U8           devNum,
    OUT GT_BOOL         *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanIndependentNonFloodVidxEnableGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with non-null enablePtr.
    Expected: GT_OK.
    1.2. Call with null enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;
    GT_U32      notAppFamilyBmp = 0;

    GT_BOOL     enable = GT_FALSE;


    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgVlanIndependentNonFloodVidxEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with null enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgVlanIndependentNonFloodVidxEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanIndependentNonFloodVidxEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanIndependentNonFloodVidxEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanIngressTpidProfileSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               profile,
    IN  CPSS_ETHER_MODE_ENT  ethMode,
    IN  GT_U32               tpidBmp
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanIngressTpidProfileSet)
{
        /*
            ITERATE_DEVICES (SIP5)
                   1.1. Call with profile[0, 3, 7],
                                              ethMode[CPSS_VLAN_ETHERTYPE0_E,
                                                          CPSS_VLAN_ETHERTYPE1_E,
                                                          CPSS_VLAN_ETHERTYPE0_E],
                                              tpidBmp[0x00, 0xA5, 0xFF].
            Expected: GT_OK.
            1.2. Call cpssDxChBrgVlanIngressTpidProfileGet.
            Expected: GT_OK and the same tpidBmp.
            1.3. Call with out of range enum value ethMode and other valid
                    parameters same as 1.1.
            Expected: GT_BAD_PARAM.
                1.4. Call with out of range profile[8] and other valid
                    parameters same as 1.1.
            Expected: NOT GT_OK.
                1.5. Call with out of range tpidBmp[BIT_8] and other valid
                    parameters same as 1.1.
            Expected: NOT GT_OK.
        */

        GT_STATUS                   st                                         = GT_OK;
    GT_U8                       dev                                        = 0;
    CPSS_ETHER_MODE_ENT ethMode                         = CPSS_VLAN_ETHERTYPE0_E;
    GT_U32              profile                                = 0;
    GT_U32              tpidBmp                                = 0;
    GT_U32              tpidBmpGet                        = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                                                               UTF_CH2_E | UTF_CH3_E |
                                                                               UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                                                               UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
                /*
                           1.1. Call with profile[0, 3, 7],
                                                          ethMode[CPSS_VLAN_ETHERTYPE0_E,
                                                              CPSS_VLAN_ETHERTYPE1_E,
                                                                  CPSS_VLAN_ETHERTYPE0_E],
                                                  tpidBmp[0x00, 0xA5, 0xFF].
                    Expected: GT_OK.
                */
                profile = 0;
                ethMode = CPSS_VLAN_ETHERTYPE0_E;
                tpidBmp = 0x00;

                st = cpssDxChBrgVlanIngressTpidProfileSet(dev, profile, ethMode,
                                                                                                  tpidBmp);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /*
                    1.2. Call cpssDxChBrgVlanIngressTpidProfileGet.
                    Expected: GT_OK and the same tpidBmp.
                */
                st = cpssDxChBrgVlanIngressTpidProfileGet(dev, profile, ethMode,
                                                                                                  &tpidBmpGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /* verifying values */
                UTF_VERIFY_EQUAL1_STRING_MAC(tpidBmp, tpidBmpGet,
                                                                         "got another value than was set: %d",
                                                                         tpidBmpGet);

                /* call with profile[3] */
                profile = 3;
                ethMode = CPSS_VLAN_ETHERTYPE1_E;
                tpidBmp = 0xA5;

                st = cpssDxChBrgVlanIngressTpidProfileSet(dev, profile, ethMode,
                                                                                                  tpidBmp);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /*
                    1.2. Call cpssDxChBrgVlanIngressTpidProfileGet.
                    Expected: GT_OK and the same tpidBmp.
                */
                st = cpssDxChBrgVlanIngressTpidProfileGet(dev, profile, ethMode,
                                                                                                  &tpidBmpGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /* verifying values */
                UTF_VERIFY_EQUAL1_STRING_MAC(tpidBmp, tpidBmpGet,
                                                                         "got another value than was set: %d",
                                                                         tpidBmpGet);

                /* call with profile[7] */
                profile = 7;
                ethMode = CPSS_VLAN_ETHERTYPE0_E;
                tpidBmp = 0xFF;

                st = cpssDxChBrgVlanIngressTpidProfileSet(dev, profile, ethMode,
                                                                                                  tpidBmp);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /*
                    1.2. Call cpssDxChBrgVlanIngressTpidProfileGet.
                    Expected: GT_OK and the same tpidBmp.
                */
                st = cpssDxChBrgVlanIngressTpidProfileGet(dev, profile, ethMode,
                                                                                                  &tpidBmpGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /* verifying values */
                UTF_VERIFY_EQUAL1_STRING_MAC(tpidBmp, tpidBmpGet,
                                                                         "got another value than was set: %d",
                                                                         tpidBmpGet);

                /*
                    1.3. Call with out of range enum value ethMode and other valid
                            parameters same as 1.1.
                    Expected: GT_BAD_PARAM.
                */
                UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanIngressTpidProfileSet
                                                        (dev, profile, ethMode,  tpidBmp),
                                                        ethMode);

                /*
                        1.4. Call with out of range profile[8] and other valid
                            parameters same as 1.1.
                    Expected: NOT GT_OK.
                */
                profile = 8;

                st = cpssDxChBrgVlanIngressTpidProfileSet(dev, profile, ethMode,
                                                                                                  tpidBmp);
                UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /* restore previous value */
                profile = 7;

                /*
                        1.5. Call with out of range tpidBmp[BIT_8] and other valid
                            parameters same as 1.1.
                    Expected: NOT GT_OK.
                */
                tpidBmp = BIT_8;

                st = cpssDxChBrgVlanIngressTpidProfileSet(dev, profile, ethMode,
                                                                                                  tpidBmp);
                UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /* restore previous value */
                tpidBmp = 0xFF;
    }

        /* 2. For not-active devices and devices from non-applicable family */
        /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                                                               UTF_CH2_E | UTF_CH3_E |
                                                                               UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                                                               UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
                st = cpssDxChBrgVlanIngressTpidProfileSet(dev, profile, ethMode,
                                                                                                  tpidBmp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
        }

        /* 3. Call function with out of bound value for device id.*/
        dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
        st = cpssDxChBrgVlanIngressTpidProfileSet(dev, profile, ethMode,
                                                                                          tpidBmp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanIngressTpidProfileGet
(
    IN  GT_U8                devNum,
    IN  GT_U32               profile,
    IN  CPSS_ETHER_MODE_ENT  ethMode,
    OUT GT_U32               *tpidBmpPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanIngressTpidProfileGet)
{
        /*
            ITERATE_DEVICES (SIP5)
                   1.1. Call with profile[0, 3, 7],
                                              ethMode[CPSS_VLAN_ETHERTYPE0_E,
                                                          CPSS_VLAN_ETHERTYPE1_E,
                                                          CPSS_VLAN_ETHERTYPE0_E].
            Expected: GT_OK.
            1.2. Call with out of range enum value ethMode and other valid
                    parameters same as 1.1.
            Expected: GT_BAD_PARAM.
                1.3. Call with out of range profile[8] and other valid
                    parameters same as 1.1.
            Expected: NOT GT_OK.
                1.4. Call with out of range tpidBmpPtr[NULL] and other valid
                    parameters same as 1.1.
            Expected: GT_BAD_PTR.
        */

        GT_STATUS                   st                                         = GT_OK;
    GT_U8                       dev                                        = 0;
    CPSS_ETHER_MODE_ENT ethMode                         = CPSS_VLAN_ETHERTYPE0_E;
    GT_U32              profile                                = 0;
    GT_U32              tpidBmp                                = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                                                               UTF_CH2_E | UTF_CH3_E |
                                                                               UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                                                               UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
                /*
                           1.1. Call with profile[0, 3, 7],
                                                          ethMode[CPSS_VLAN_ETHERTYPE0_E,
                                                              CPSS_VLAN_ETHERTYPE1_E,
                                                                  CPSS_VLAN_ETHERTYPE0_E].
                    Expected: GT_OK.
                */
                profile = 0;
                ethMode = CPSS_VLAN_ETHERTYPE0_E;

                st = cpssDxChBrgVlanIngressTpidProfileGet(dev, profile, ethMode,
                                                                                                  &tpidBmp);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /* call with profile[3] */
                profile = 3;
                ethMode = CPSS_VLAN_ETHERTYPE1_E;

                st = cpssDxChBrgVlanIngressTpidProfileGet(dev, profile, ethMode,
                                                                                                  &tpidBmp);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /* call with profile[7] */
                profile = 7;
                ethMode = CPSS_VLAN_ETHERTYPE0_E;

                st = cpssDxChBrgVlanIngressTpidProfileGet(dev, profile, ethMode,
                                                                                                  &tpidBmp);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /*
                    1.2. Call with out of range enum value ethMode and other valid
                            parameters same as 1.1.
                    Expected: GT_BAD_PARAM.
                */
                UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanIngressTpidProfileGet
                                                        (dev, profile, ethMode,  &tpidBmp),
                                                        ethMode);

                /*
                        1.3. Call with out of range profile[8] and other valid
                            parameters same as 1.1.
                    Expected: NOT GT_OK.
                */
                profile = 8;

                st = cpssDxChBrgVlanIngressTpidProfileGet(dev, profile, ethMode,
                                                                                                  &tpidBmp);
                UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /* restore previous value */
                profile = 7;

                /*
                        1.4. Call with out of range tpidBmpPtr[NULL] and other valid
                            parameters same as 1.1.
                    Expected: GT_BAD_PTR.
                */
                st = cpssDxChBrgVlanIngressTpidProfileGet(dev, profile, ethMode, NULL);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

        /* 2. For not-active devices and devices from non-applicable family */
        /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                                                               UTF_CH2_E | UTF_CH3_E |
                                                                               UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                                                               UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
                st = cpssDxChBrgVlanIngressTpidProfileGet(dev, profile, ethMode,
                                                                                                  &tpidBmp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
        }

        /* 3. Call function with out of bound value for device id.*/
        dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
        st = cpssDxChBrgVlanIngressTpidProfileGet(dev, profile, ethMode,
                                                                                          &tpidBmp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortIngressTpidProfileSet
(
    IN  GT_U8                devNum,
    IN  GT_PORT_NUM          portNum,
    IN  CPSS_ETHER_MODE_ENT  ethMode,
    IN  GT_BOOL              isDefaultProfile,
    IN  GT_U32               profile
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortIngressTpidProfileSet)
{
/*
    ITERATE_DEVICES (SIP5)
           1.1.1. Call with ethMode[CPSS_VLAN_ETHERTYPE0_E,
                                                      CPSS_VLAN_ETHERTYPE1_E,
                                                      CPSS_VLAN_ETHERTYPE0_E],
                                      isDefaultProfile[GT_FALSE, GT_TRUE, GT_TRUE],
                                      profile[0, 3, 7].
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgVlanPortIngressTpidProfileGet.
    Expected: GT_OK and the same profile.
    1.1.3. Call with out of range enum value ethMode and other valid
            parameters same as 1.1.
    Expected: GT_BAD_PARAM.
        1.1.4. Call with out of range profile[8] and other valid
            parameters same as 1.1.
    Expected: NOT GT_OK.
*/

    GT_STATUS                   st                                         = GT_OK;
    GT_U8                       dev                                        = 0;
    GT_PORT_NUM                 port                                 = BRG_VLAN_VALID_PHY_PORT_CNS;
    CPSS_ETHER_MODE_ENT ethMode                         = CPSS_VLAN_ETHERTYPE0_E;
    GT_BOOL             isDefaultProfile        = GT_FALSE;
    GT_U32              profile                                = 0;
    GT_U32              profileGet                        = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                                                               UTF_CH2_E | UTF_CH3_E |
                                                                               UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                                                               UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(isDefaultProfile = GT_FALSE; isDefaultProfile <= GT_TRUE ; isDefaultProfile++)
        {
            prvUtfNextGenericPortItaratorTypeSet(   isDefaultProfile == GT_TRUE ?
                                                    UTF_GENERIC_PORT_ITERATOR_TYPE_DEFAULT_EPORT_E :
                                                    UTF_GENERIC_PORT_ITERATOR_TYPE_VIRTUAL_E);

            /* prepare ePort iterator */
            st = prvUtfNextGenericPortReset(&port, dev);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* 1.1. For all active devices go over all available ePorts. */
            while(GT_OK == prvUtfNextGenericPortGet(&port, GT_TRUE))
            {
                /*
                        1.1.1. Call with ethMode[CPSS_VLAN_ETHERTYPE0_E,
                                                                   CPSS_VLAN_ETHERTYPE1_E,
                                                                   CPSS_VLAN_ETHERTYPE0_E],
                                           isDefaultProfile[GT_FALSE, GT_TRUE, GT_TRUE],
                                           profile[0, 3, 7].
                        Expected: GT_OK.
                */
                ethMode = CPSS_VLAN_ETHERTYPE0_E;
                profile = 0;

                st = cpssDxChBrgVlanPortIngressTpidProfileSet(dev, port,
                                                              ethMode,
                                                              isDefaultProfile,
                                                              profile);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                /*
                        1.1.2. Call cpssDxChBrgVlanPortIngressTpidProfileGet.
                        Expected: GT_OK and the same profile.
                */
                st = cpssDxChBrgVlanPortIngressTpidProfileGet(dev, port,
                                                              ethMode,
                                                              isDefaultProfile,
                                                              &profileGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                /* verifying values */
                UTF_VERIFY_EQUAL2_STRING_MAC(profile, profileGet,
                                             "got another value than was set:"\
                                             " %d, %d",        dev, port);

                /* call with ethMode[CPSS_VLAN_ETHERTYPE1_E] */
                ethMode = CPSS_VLAN_ETHERTYPE1_E;
                profile = 3;

                st = cpssDxChBrgVlanPortIngressTpidProfileSet(dev, port,
                                                              ethMode,
                                                              isDefaultProfile,
                                                              profile);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                /*
                        1.1.2. Call cpssDxChBrgVlanPortIngressTpidProfileGet.
                        Expected: GT_OK and the same profile.
                */
                st = cpssDxChBrgVlanPortIngressTpidProfileGet(dev, port,
                                                              ethMode,
                                                              isDefaultProfile,
                                                              &profileGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                /* verifying values */
                UTF_VERIFY_EQUAL2_STRING_MAC(profile, profileGet,
                                         "got another value than was set:"\
                                         " %d, %d",        dev, port);

                /* call with ethMode[CPSS_VLAN_ETHERTYPE0_E] */
                ethMode = CPSS_VLAN_ETHERTYPE1_E;
                profile = 7;

                st = cpssDxChBrgVlanPortIngressTpidProfileSet(dev, port,
                                                              ethMode,
                                                              isDefaultProfile,
                                                              profile);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                /*
                        1.1.2. Call cpssDxChBrgVlanPortIngressTpidProfileGet.
                        Expected: GT_OK and the same profile.
                */
                st = cpssDxChBrgVlanPortIngressTpidProfileGet(dev, port,
                                                              ethMode,
                                                              isDefaultProfile,
                                                              &profileGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                /* verifying values */
                UTF_VERIFY_EQUAL2_STRING_MAC(profile, profileGet,
                                             "got another value than was set:"\
                                             " %d, %d",        dev, port);

                /*
                        1.1.3. Call with out of range enum value ethMode and other valid
                                parameters same as 1.1.
                        Expected: GT_BAD_PARAM.
                */
                UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanPortIngressTpidProfileSet
                                                        (dev, port, ethMode, isDefaultProfile, profile),
                                                        ethMode);

                /*
                        1.1.4. Call with out of range profile[8] and other valid
                                parameters same as 1.1.
                        Expected: NOT GT_OK.
                */
                profile = 8;

                st = cpssDxChBrgVlanPortIngressTpidProfileSet(dev, port,
                                                                  ethMode,
                                                                  isDefaultProfile,
                                                                  profile);
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                /* restore previous value */
                profile = 7;

            }

            /* prepare ePort iterator */
            st = prvUtfNextGenericPortReset(&port, dev);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* 1.2. For all active devices go over all non available ePorts. */
            while(GT_OK == prvUtfNextGenericPortGet(&port, GT_FALSE))
            {
                st = cpssDxChBrgVlanPortIngressTpidProfileSet(dev, port,
                                                              ethMode,
                                                              isDefaultProfile,
                                                              profile);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
            }

            /* 1.3. For active device check that function returns GT_BAD_PARAM */
            /* for out of bound value for port number.                         */
            port = BRG_VLAN_INVALID_PORT_CNS(dev);

            st = cpssDxChBrgVlanPortIngressTpidProfileSet(dev, port,
                                                              ethMode,
                                                              isDefaultProfile,
                                                              profile);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

            /* 1.4. For active device check that function returns GT_OK */
            /* for CPU port number.                                     */
            port = CPSS_CPU_PORT_NUM_CNS;
            st = cpssDxChBrgVlanPortIngressTpidProfileSet(dev, port,
                                                              ethMode,
                                                              isDefaultProfile,
                                                              profile);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
    }

    isDefaultProfile = GT_FALSE;

    /* restore values */
    port = BRG_VLAN_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortIngressTpidProfileSet(dev, port,
                                                          ethMode,
                                                          isDefaultProfile,
                                                          profile);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChBrgVlanPortIngressTpidProfileSet(dev, port,
                                                  ethMode,
                                                  isDefaultProfile,
                                                  profile);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortIngressTpidProfileGet
(
    IN  GT_U8                devNum,
    IN  GT_PORT_NUM          portNum,
    IN  CPSS_ETHER_MODE_ENT  ethMode,
    IN GT_BOOL               isDefaultProfile,
    OUT GT_U32               *profilePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortIngressTpidProfileGet)
{
    /*
        ITERATE_DEVICES (SIP5)
               1.1.1. Call with ethMode[CPSS_VLAN_ETHERTYPE0_E,
                                                          CPSS_VLAN_ETHERTYPE1_E,
                                                          CPSS_VLAN_ETHERTYPE0_E],
                                          isDefaultProfile[GT_FALSE, GT_TRUE, GT_TRUE].
        Expected: GT_OK.
        1.1.2. Call with out of range enum value ethMode and other valid
                parameters same as 1.1.
        Expected: GT_BAD_PARAM.
               1.1.3. Call with out of range profilePtr[NULL] and other valid
                       parameters same as 1.1.
            Expected: GT_BAD_PTR.
    */

    GT_STATUS                   st                                         = GT_OK;
    GT_U8                       dev                                        = 0;
    GT_PORT_NUM                 port                                 = BRG_VLAN_VALID_PHY_PORT_CNS;
    CPSS_ETHER_MODE_ENT ethMode                         = CPSS_VLAN_ETHERTYPE0_E;
    GT_BOOL             isDefaultProfile        = GT_FALSE;
    GT_U32              profile                                = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(isDefaultProfile = GT_FALSE; isDefaultProfile <= GT_TRUE ; isDefaultProfile++)
        {
            prvUtfNextGenericPortItaratorTypeSet(   isDefaultProfile == GT_TRUE ?
                                                    UTF_GENERIC_PORT_ITERATOR_TYPE_DEFAULT_EPORT_E :
                                                    UTF_GENERIC_PORT_ITERATOR_TYPE_VIRTUAL_E);

            /* prepare ePort iterator */
            st = prvUtfNextGenericPortReset(&port, dev);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* 1.1. For all active devices go over all available ePorts. */
            while(GT_OK == prvUtfNextGenericPortGet(&port, GT_TRUE))
            {
                /*
                        1.1.1. Call with ethMode[CPSS_VLAN_ETHERTYPE0_E,
                                                                     CPSS_VLAN_ETHERTYPE1_E,
                                                                     CPSS_VLAN_ETHERTYPE0_E],
                                           isDefaultProfile[GT_FALSE, GT_TRUE, GT_TRUE],
                                           profile[0, 3, 7].
                        Expected: GT_OK.
                */
                ethMode = CPSS_VLAN_ETHERTYPE0_E;
                st = cpssDxChBrgVlanPortIngressTpidProfileGet(dev, port,
                                                              ethMode,
                                                              isDefaultProfile,
                                                              &profile);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                /* call with ethMode[CPSS_VLAN_ETHERTYPE1_E] */
                ethMode = CPSS_VLAN_ETHERTYPE1_E;
                st = cpssDxChBrgVlanPortIngressTpidProfileGet(dev, port,
                                                              ethMode,
                                                              isDefaultProfile,
                                                              &profile);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                /* call with ethMode[CPSS_VLAN_ETHERTYPE0_E] */
                ethMode = CPSS_VLAN_ETHERTYPE1_E;
                st = cpssDxChBrgVlanPortIngressTpidProfileGet(dev, port,
                                                              ethMode,
                                                              isDefaultProfile,
                                                              &profile);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);


                /*
                        1.1.2. Call with out of range enum value ethMode and
                                other valid parameters same as 1.1.
                        Expected: GT_BAD_PARAM.
                */
                UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanPortIngressTpidProfileGet
                                                        (dev, port, ethMode, isDefaultProfile, &profile),
                                                        ethMode);

                /*
                        1.1.3. Call with out of range profilePtr[NULL] and
                                other valid parameters same as 1.1.
                        Expected: GT_BAD_PTR.
                */
                st = cpssDxChBrgVlanPortIngressTpidProfileGet(dev, port,
                                                              ethMode,
                                                              isDefaultProfile,
                                                              NULL);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
            }

            /* prepare ePort iterator */
            st = prvUtfNextGenericPortReset(&port, dev);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* 1.2. For all active devices go over all non available ePorts. */
            while(GT_OK == prvUtfNextGenericPortGet(&port, GT_FALSE))
            {
                st = cpssDxChBrgVlanPortIngressTpidProfileGet(dev, port,
                                                              ethMode,
                                                              isDefaultProfile,
                                                              &profile);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
            }

            /* 1.3. For active device check that function returns GT_BAD_PARAM */
            /* for out of bound value for port number.                         */
            port = BRG_VLAN_INVALID_PORT_CNS(dev);

            st = cpssDxChBrgVlanPortIngressTpidProfileGet(dev, port,
                                                          ethMode,
                                                          isDefaultProfile,
                                                          &profile);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

            /* 1.4. For active device check that function returns GT_OK */
            /* for CPU port number.                                     */
            port = CPSS_CPU_PORT_NUM_CNS;
            st = cpssDxChBrgVlanPortIngressTpidProfileGet(dev, port,
                                                          ethMode,
                                                          isDefaultProfile,
                                                          &profile);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
    }

    isDefaultProfile = GT_FALSE;
    /* restore values */
    port = BRG_VLAN_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortIngressTpidProfileGet(dev, port,
                                                      ethMode,
                                                      isDefaultProfile,
                                                      &profile);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChBrgVlanPortIngressTpidProfileGet(dev, port,
                                                  ethMode,
                                                  isDefaultProfile,
                                                  &profile);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanTpidTagTypeSet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1. Call with valid params
                   direction [CPSS_DIRECTION_INGRESS_E/
                              CPSS_DIRECTION_EGRESS_E],
                   index [0/3/7],
                   type[CPSS_BRG_TPID_SIZE_TYPE_4_BYTES_ENT/
                        CPSS_BRG_TPID_SIZE_TYPE_8_BYTES_ENT]
    Expected: GT_OK.
    1.2. Call cpssDxChBrgVlanTpidTagTypeGet
    Expected: GT_OK and same type.
    1.3. Call with not relevant direction[CPSS_DIRECTION_BOTH_E].
    Expected: NOT GT_OK.
    1.4. Call with wrong enum value direction.
    Expected: GT_BAD_PARAM.
    1.5. Call with out of range index[8].
    Expected: Not GT_OK.
    1.6. Call with wrong enum value type.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st = GT_OK;

    GT_U8                        dev;
    CPSS_DIRECTION_ENT           direction = CPSS_DIRECTION_INGRESS_E;
    GT_U32                       index = 0;
    CPSS_BRG_TPID_SIZE_TYPE_ENT  type = CPSS_BRG_TPID_SIZE_TYPE_4_BYTES_E;
    CPSS_BRG_TPID_SIZE_TYPE_ENT  typeGet = CPSS_BRG_TPID_SIZE_TYPE_4_BYTES_E;


    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. */
        direction = CPSS_DIRECTION_INGRESS_E;
        index = 0;
        type = CPSS_BRG_TPID_SIZE_TYPE_4_BYTES_E;

        st = cpssDxChBrgVlanTpidTagTypeSet(dev, direction, index, type);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2. */
        st = cpssDxChBrgVlanTpidTagTypeGet(dev, direction, index, &typeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(type, typeGet,
                       "got another type then was set: %d", dev);

        /*  1.1. */
        direction = CPSS_DIRECTION_EGRESS_E;
        index = 3;
        type = CPSS_BRG_TPID_SIZE_TYPE_8_BYTES_E;

        st = cpssDxChBrgVlanTpidTagTypeSet(dev, direction, index, type);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2. */
        st = cpssDxChBrgVlanTpidTagTypeGet(dev, direction, index, &typeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(type, typeGet,
                       "got another type then was set: %d", dev);

        /*  1.1. */
        direction = CPSS_DIRECTION_INGRESS_E;
        index = 7;
        type = CPSS_BRG_TPID_SIZE_TYPE_8_BYTES_E;

        st = cpssDxChBrgVlanTpidTagTypeSet(dev, direction, index, type);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2. */
        st = cpssDxChBrgVlanTpidTagTypeGet(dev, direction, index, &typeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(type, typeGet,
                       "got another type then was set: %d", dev);

        /*  1.1. */
        direction = CPSS_DIRECTION_EGRESS_E;
        index = 7;
        type = CPSS_BRG_TPID_SIZE_TYPE_8_BYTES_E;

        st = cpssDxChBrgVlanTpidTagTypeSet(dev, direction, index, type);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2. */
        st = cpssDxChBrgVlanTpidTagTypeGet(dev, direction, index, &typeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(type, typeGet,
                       "got another type then was set: %d", dev);

        /*  1.3. */
        direction = CPSS_DIRECTION_BOTH_E;
        index = 1;
        type = CPSS_BRG_TPID_SIZE_TYPE_4_BYTES_E;

        st = cpssDxChBrgVlanTpidTagTypeSet(dev, direction, index, type);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.4. */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanTpidTagTypeSet(dev,
                                      direction, index, type),
                                      direction);

        /*  1.5. */
        index = 8;
        st = cpssDxChBrgVlanTpidTagTypeSet(dev, direction, index, type);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        index = 0;

        /*  1.6. */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanTpidTagTypeSet(dev,
                                      direction, index, type),
                                      type);

        index = 1;
        type = CPSS_BRG_TPID_SIZE_TYPE_6_BYTES_E;

        for(direction = CPSS_DIRECTION_INGRESS_E;
            direction < CPSS_DIRECTION_EGRESS_E;
            direction++)
        {
            st = cpssDxChBrgVlanTpidTagTypeSet(dev, direction, index, type);
            if(0 == PRV_CPSS_SIP_5_15_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
            }
            else
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                st = cpssDxChBrgVlanTpidTagTypeGet(dev, direction, index, &typeGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /* Verifying values */
                UTF_VERIFY_EQUAL1_STRING_MAC(type, typeGet,
                               "got another type than was set: %d", dev);
            }
        }
    }

    /* restore correct values */
    direction = CPSS_DIRECTION_INGRESS_E;
    index = 0;
    type = CPSS_BRG_TPID_SIZE_TYPE_4_BYTES_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanTpidTagTypeSet(dev, direction, index, type);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanTpidTagTypeSet(dev, direction, index, type);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanTpidTagTypeGet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1. Call with valid params
                   direction [CPSS_DIRECTION_INGRESS_E/
                              CPSS_DIRECTION_EGRESS_E],
                   index [0/3/7],
                   and not NULL typePtr.
    Expected: GT_OK.
    1.2. Call with wrong enum value direction.
    Expected: GT_BAD_PARAM.
    1.3. Call with out of range index[8].
    Expected: Not GT_OK.
    1.4. Call with NULL typePtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8                        dev;
    CPSS_DIRECTION_ENT           direction = CPSS_DIRECTION_INGRESS_E;
    GT_U32                       index = 0;
    CPSS_BRG_TPID_SIZE_TYPE_ENT  type = CPSS_BRG_TPID_SIZE_TYPE_4_BYTES_E;


    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. */
        direction = CPSS_DIRECTION_INGRESS_E;
        index = 0;

        st = cpssDxChBrgVlanTpidTagTypeGet(dev, direction, index, &type);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.1. */
        direction = CPSS_DIRECTION_EGRESS_E;
        index = 3;

        st = cpssDxChBrgVlanTpidTagTypeGet(dev, direction, index, &type);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2. */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanTpidTagTypeGet(dev,
                                      direction, index, &type),
                                      direction);

        /*  1.3. */
        index = 8;
        st = cpssDxChBrgVlanTpidTagTypeGet(dev, direction, index, &type);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        index = 0;

        /*  1.4. */
        st = cpssDxChBrgVlanTpidTagTypeGet(dev, direction, index, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* restore correct values */
    direction = CPSS_DIRECTION_INGRESS_E;
    index = 0;
    type = CPSS_BRG_TPID_SIZE_TYPE_4_BYTES_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanTpidTagTypeGet(dev, direction, index, &type);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanTpidTagTypeGet(dev, direction, index, &type);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableSet
(
    IN GT_U8        devNum,
    IN GT_U8        port,
    IN GT_BOOL      enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call with enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableGet with
           not NULL enablePtr
    Expected: GT_OK
*/
    GT_STATUS      st  = GT_OK;
    GT_U8          dev;
    GT_PORT_NUM    port;
    GT_U32         notAppFamilyBmp = 0;
    GT_BOOL        enable    = GT_FALSE;
    GT_BOOL        enableGet = GT_FALSE;


    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with enable [GT_FALSE / GT_TRUE].
                Expected: GT_OK.
            */
            /* iterate with enable = GT_FALSE */
            enable = GT_FALSE;

            st = cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableSet(dev,
                                                                  port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
               1.1.2. Call cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableGet
                      with not NULL enablePtr
               Expected: GT_OK
            */
            st = cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableGet(dev,
                                                              port, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableGet: %d, %d",
                dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                          "got another enable than was set: %d, %d", dev, port);

            /* iterate with enable = GT_TRUE */
            enable = GT_TRUE;

            st = cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableSet(dev,
                                                                  port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
               1.1.2. Call cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableGet
                      with not NULL enablePtr
               Expected: GT_OK
            */
            st = cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableGet(dev,
                                                              port, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableGet: %d, %d",
                dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                          "got another enable than was set: %d, %d", dev, port);
        }

        /* restore correct values*/
        enable = GT_FALSE;

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.2. For all active devices go over all non available physical ports
        */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableSet(dev,
                                                                  port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /*
            1.3. For active device check that function returns GT_BAD_PARAM
                 for out of bound value for port number.
        */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableSet(dev, port,
                                                                   enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /*
            1.4. For active device check that function returns GT_OK
                 for CPU port number.
        */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableSet(dev, port,
                                                                   enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    enable = GT_FALSE;
    port   = 0;

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableSet(dev, port,
                                                                   enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableSet(dev, port,
                                                               enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableGet
(
    IN  GT_U8        devNum,
    IN  GT_U8        port,
    OUT GT_BOOL      *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call with not NULL enablePtr.
    Expected: GT_OK.
    1.1.2. Call with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS      st  = GT_OK;
    GT_U8          dev;
    GT_PORT_NUM    port;
    GT_U32         notAppFamilyBmp = 0;
    GT_BOOL        enable = GT_FALSE;


    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with not NULL enablePtr.
                Expected: GT_OK.
            */
            st = cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableGet(dev,
                                                                 port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call with enablePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableGet(dev,
                                                                    port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                                         "%d, %d, enablePtr = NULL", dev, port);
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.2. For all active devices go over all non available physical ports
        */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableGet(dev,
                                                                 port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /*
            1.3. For active device check that function returns GT_BAD_PARAM
                 for out of bound value for port number.
        */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableGet(dev,
                                                                 port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /*
            1.4. For active device check that function returns GT_OK
                 for CPU port number.
        */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableGet(dev, port,
                                                                       &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    port = 0;

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableGet(dev, port,
                                                                       &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableGet(dev, port,
                                                                       &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanFcoeForwardingEnableSet
(
    IN GT_U8    devNum,
    IN GT_U16   vlanId,
    IN GT_BOOL  enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanFcoeForwardingEnableSet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1. Call function with
                    vlanId [0 / 100 / UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)-1 ] and
                    enable [GT_FALSE and GT_ TRUE].
    Expected: GT_OK.
    1.2. Call function with out of range
                                  vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)] and
                                  enable [GT_FALSE].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st     = GT_OK;
    GT_U8     dev;
    GT_U32    notAppFamilyBmp = 0;
    GT_U16    vlanId = 0;
    GT_BOOL   enable = GT_FALSE;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with
                    vlanId [0 / 100 / UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)-1 ] and
                    enable [GT_FALSE and GT_ TRUE].
            Expected: GT_OK.
        */

        /* Call with enable [GT_FALSE] and vlanId [0] */
        vlanId = 0;
        enable = GT_FALSE;

        st = cpssDxChBrgVlanFcoeForwardingEnableSet(dev, vlanId, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlanId, enable);

        /* Call with enable [GT_TRUE] and vlanId [100] */
        vlanId = 100;
        enable = GT_TRUE;

        st = cpssDxChBrgVlanFcoeForwardingEnableSet(dev, vlanId, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlanId, enable);

        /*
            Call with enable [GT_TRUE] and
                      vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)-1]
        */
        vlanId = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)-1;
        enable = GT_TRUE;

        st = cpssDxChBrgVlanFcoeForwardingEnableSet(dev, vlanId, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlanId, enable);

        /*
            1.2. Call function with out of range
                                  vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)] and
                                  enable [GT_FALSE].
            Expected: GT_BAD_PARAM.
        */
        vlanId = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev);
        enable = GT_FALSE;

        st = cpssDxChBrgVlanFcoeForwardingEnableSet(dev, vlanId, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, vlanId, enable);
    }
    /* restore correct values*/
    vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;
    enable = GT_FALSE;

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanFcoeForwardingEnableSet(dev, vlanId, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanFcoeForwardingEnableSet(dev, vlanId, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanMirrorToTxAnalyzerSet
(
    IN  GT_U8       devNum,
    IN  GT_U16      vlanId,
    IN  GT_BOOL     enable,
    IN  GT_U32      txAnalyzerIndex
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanMirrorToTxAnalyzerSet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1. Call function with
                    vlanId [0 / 100 / UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)-1 ],
                    enable [GT_FALSE and GT_ TRUE] and
                    txAnalyzerIndex [0 / 3 / 6].
    Expected: GT_OK.
    1.2. Call function with out of range
                                  vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)] and
                                  enable [GT_FALSE]
                                  txAnalyzerIndex [3].
    Expected: NOT GT_OK.
    1.3. Call function with out of range txAnalyzerIndex [7]
                                         vlanId [100] and
                                         enable [GT_TRUE].
    Expected: NOT GT_OK.
*/
    GT_STATUS st     = GT_OK;
    GT_U8     dev;
    GT_U32    notAppFamilyBmp = 0;
    GT_U16    vlanId = 0;
    GT_BOOL   enable = GT_FALSE;
    GT_U32    txAnalyzerIndex = 0;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with
                    vlanId [0 / 100 / UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)-1 ],
                    enable [GT_FALSE and GT_ TRUE] and
                    txAnalyzerIndex [0 / 3 / 6].
            Expected: GT_OK.
        */

        /*
            Call with enable [GT_FALSE], vlanId [0] and txAnalyzerIndex [0].
        */
        vlanId = 0;
        txAnalyzerIndex = 0;
        enable = GT_FALSE;

        st = cpssDxChBrgVlanMirrorToTxAnalyzerSet(dev, vlanId, enable,
                                                  txAnalyzerIndex);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, vlanId, enable,
                                    txAnalyzerIndex);

        /*
            Call with enable [GT_TRUE], vlanId [100] and txAnalyzerIndex [3].
        */
        vlanId = 100;
        txAnalyzerIndex = 3;
        enable = GT_TRUE;

        st = cpssDxChBrgVlanMirrorToTxAnalyzerSet(dev, vlanId, enable,
                                                  txAnalyzerIndex);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, vlanId, enable,
                                    txAnalyzerIndex);

        /*
            Call with enable [GT_TRUE],
                      vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)-1] and
                      txAnalyzerIndex [6]
        */
        vlanId = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)-1;
        txAnalyzerIndex = 6;
        enable = GT_TRUE;

        st = cpssDxChBrgVlanMirrorToTxAnalyzerSet(dev, vlanId, enable,
                                                  txAnalyzerIndex);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, vlanId, enable,
                                    txAnalyzerIndex);

        /*
            1.2. Call function with out of range
                                  vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)] and
                                  enable [GT_FALSE]
                                  txAnalyzerIndex [3].
            Expected: NOT GT_OK.
        */
        vlanId = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev);
        txAnalyzerIndex = 3;
        enable = GT_FALSE;

        st = cpssDxChBrgVlanMirrorToTxAnalyzerSet(dev, vlanId, enable,
                                                  txAnalyzerIndex);
        UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, dev, vlanId, enable,
                                        txAnalyzerIndex);
        /*
            1.3. Call function with out of range txAnalyzerIndex [7]
                                                 vlanId [100] and
                                                 enable [GT_TRUE].
            Expected: NOT GT_OK.
        */
        vlanId = 100;
        txAnalyzerIndex = 7;
        enable = GT_TRUE;

        st = cpssDxChBrgVlanMirrorToTxAnalyzerSet(dev, vlanId, enable,
                                                  txAnalyzerIndex);
        UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, dev, vlanId, enable,
                                        txAnalyzerIndex);
    }
    /* restore correct values*/
    txAnalyzerIndex = 0;
    vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;
    enable = GT_FALSE;

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanMirrorToTxAnalyzerSet(dev, vlanId, enable,
                                                  txAnalyzerIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanMirrorToTxAnalyzerSet(dev, vlanId, enable,
                                              txAnalyzerIndex);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanMirrorToTxAnalyzerModeSet
(
    IN  GT_U8       devNum,
    IN  GT_U16      vlanId,
    IN  CPSS_DXCH_MIRROR_EGRESS_MODE_ENT egressMirroringMode
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanMirrorToTxAnalyzerModeSet)
{
/*
    ITERATE_DEVICES (Falcon)
    1.1. Call function with
                    vlanId [0 / 100 / UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)-1 ],
                    egressMirroringMode [CPSS_DXCH_MIRROR_EGRESS_TAIL_DROP_E and CPSS_DXCH_MIRROR_EGRESS_CONGESTION_E].
    Expected: GT_OK.
    1.2. Call function with out of range
                                  vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)].
    Expected: NOT GT_OK.    
*/
    GT_STATUS st     = GT_OK;
    GT_U8     dev;    
    GT_U16    vlanId = 0;
    CPSS_DXCH_MIRROR_EGRESS_MODE_ENT egressMirroringMode;

    egressMirroringMode = CPSS_DXCH_MIRROR_EGRESS_TAIL_DROP_E;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with
                    vlanId [0 / 100 / UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)-1 ],
                    egressMirroringMode [CPSS_DXCH_MIRROR_EGRESS_TAIL_DROP_E and CPSS_DXCH_MIRROR_EGRESS_CONGESTION_E].
            Expected: GT_OK.
        */

        /*
            Call with vlanId [0] and egressMirroringMode [CPSS_DXCH_MIRROR_EGRESS_TAIL_DROP_E].
        */
        vlanId = 0;
        egressMirroringMode = CPSS_DXCH_MIRROR_EGRESS_TAIL_DROP_E;

        st = cpssDxChBrgVlanMirrorToTxAnalyzerModeSet(dev, vlanId, egressMirroringMode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlanId, egressMirroringMode);

        /*
            Call with vlanId [100] and egressMirroringMode [CPSS_DXCH_MIRROR_EGRESS_CONGESTION_E].
        */
        vlanId = 100;
        egressMirroringMode = CPSS_DXCH_MIRROR_EGRESS_CONGESTION_E;

        st = cpssDxChBrgVlanMirrorToTxAnalyzerModeSet(dev, vlanId, egressMirroringMode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlanId, egressMirroringMode);

        /*
            Call with vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)-1] and
                      egressMirroringMode [CPSS_DXCH_MIRROR_EGRESS_TAIL_DROP_E]
        */
        vlanId = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)-1;
        egressMirroringMode = CPSS_DXCH_MIRROR_EGRESS_TAIL_DROP_E;

        st = cpssDxChBrgVlanMirrorToTxAnalyzerModeSet(dev, vlanId, egressMirroringMode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlanId, egressMirroringMode);

        /*
            1.2. Call function with out of range
                                  vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)] and
                                  egressMirroringMode [CPSS_DXCH_MIRROR_EGRESS_CONGESTION_E]
            Expected: NOT GT_OK.
        */
        vlanId = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev);
        egressMirroringMode = CPSS_DXCH_MIRROR_EGRESS_CONGESTION_E;

        st = cpssDxChBrgVlanMirrorToTxAnalyzerModeSet(dev, vlanId, egressMirroringMode);
        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlanId, egressMirroringMode);        
    }
    /* restore correct values*/    
    vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;    

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE)) 
    {
        st = cpssDxChBrgVlanMirrorToTxAnalyzerModeSet(dev, vlanId, egressMirroringMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanMirrorToTxAnalyzerModeSet(dev, vlanId, egressMirroringMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanUnregisteredIpmEVidxSet
(
    IN  GT_U8       devNum,
    IN  GT_U16      vlanId,
    IN  GT_BOOL     unregIpmEVidxMode,
    IN  GT_U32      unregIpmEVidx
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanUnregisteredIpmEVidxSet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1. Call function with
            vlanId [0 / 100 / UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)-1 ],
            unregIpmEVidxMode [ CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_BOTH_E /
                                CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_IPV4_E /
                                CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_IPV6_E /
              CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_BOTH_IPV6_INCREMENT_E] and
                    unregIpmEVidx [0 / 0xA5A5 / BIT_16-1].
    Expected: GT_OK.
    1.2. Call function with out of range
            unregIpmEVidxMode [CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_BOTH_E],
            vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)] and
            unregIpmEVidx [0].
    Expected: NOT GT_OK.
    1.3. Call function with out of range unregIpmEVidx [BIT_16],
            vlanId [100] and
            unregIpmEVidxMode [CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_BOTH_E].
    Expected: NOT GT_OK.
    1.4. Call function with out of range
            unregIpmEVidxMode [CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_BOTH_E],
            unregIpmEVidx [0] and
            vlanId [100].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st     = GT_OK;
    GT_U8    dev;
    GT_U32   notAppFamilyBmp = 0;
    GT_U16   vlanId = 0;
    GT_BOOL  unregIpmEVidxMode = CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_BOTH_E;
    GT_U32   unregIpmEVidx = 0;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with
            vlanId [0 / 100 / UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)-1 ],
            unregIpmEVidxMode [ CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_BOTH_E /
                                CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_IPV4_E /
                                CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_IPV6_E /
              CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_BOTH_IPV6_INCREMENT_E] and
                    unregIpmEVidx [0 / 0xA5A5 / BIT_16-1].
            Expected: GT_OK.
        */

        /*
            Call with
             unregIpmEVidxMode [CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_BOTH_E],
             vlanId [0] and
             unregIpmEVidx [0]
        */
        vlanId = 0;
        unregIpmEVidx = 0;
        unregIpmEVidxMode = CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_BOTH_E;

        st = cpssDxChBrgVlanUnregisteredIpmEVidxSet(dev, vlanId,
                                              unregIpmEVidxMode, unregIpmEVidx);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, vlanId, unregIpmEVidxMode,
                                    unregIpmEVidx);

        /*
            Call with
             unregIpmEVidxMode [CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_IPV4_E],
             vlanId [100] and
             unregIpmEVidx [0xA5A5]
        */
        vlanId = 100;
        unregIpmEVidx = 0xA5A5;
        unregIpmEVidxMode = CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_IPV4_E;

        st = cpssDxChBrgVlanUnregisteredIpmEVidxSet(dev, vlanId,
                                              unregIpmEVidxMode, unregIpmEVidx);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, vlanId, unregIpmEVidxMode,
                                    unregIpmEVidx);

        /*
            Call with
             unregIpmEVidxMode [CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_IPV6_E],
             vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)-1] and
             unregIpmEVidx [BIT_16-1]
        */
        vlanId = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)-1;
        unregIpmEVidx = BIT_16-1;
        unregIpmEVidxMode = CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_IPV6_E;

        st = cpssDxChBrgVlanUnregisteredIpmEVidxSet(dev, vlanId,
                                              unregIpmEVidxMode, unregIpmEVidx);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, vlanId, unregIpmEVidxMode,
                                    unregIpmEVidx);

        /*
            Call with
             unregIpmEVidxMode
                [CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_BOTH_IPV6_INCREMENT_E],
             vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)-1] and
             unregIpmEVidx [BIT_16-1]
        */
        vlanId = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)-1;
        unregIpmEVidx = BIT_16-1;
        unregIpmEVidxMode =
                  CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_BOTH_IPV6_INCREMENT_E;

        st = cpssDxChBrgVlanUnregisteredIpmEVidxSet(dev, vlanId,
                                              unregIpmEVidxMode, unregIpmEVidx);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, vlanId, unregIpmEVidxMode,
                                    unregIpmEVidx);
        /*
          1.2. Call function with out of range
            vlanId [UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev)] and
            unregIpmEVidxMode [CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_BOTH_E],
            unregIpmEVidx [0].
          Expected: NOT GT_OK.
        */
        vlanId = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(dev);
        unregIpmEVidx = 0;
        unregIpmEVidxMode = CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_BOTH_E;

        st = cpssDxChBrgVlanUnregisteredIpmEVidxSet(dev, vlanId,
                                              unregIpmEVidxMode, unregIpmEVidx);
        UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, dev, vlanId,
                                              unregIpmEVidxMode, unregIpmEVidx);
        /*
          1.3. Call function with out of range unregIpmEVidx [BIT_16],
            vlanId [100] and
            unregIpmEVidxMode [CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_BOTH_E].
          Expected: NOT GT_OK.
        */
        vlanId = 100;
        unregIpmEVidx = BIT_16;
        unregIpmEVidxMode = CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_BOTH_E;

        st = cpssDxChBrgVlanUnregisteredIpmEVidxSet(dev, vlanId,
                                              unregIpmEVidxMode, unregIpmEVidx);
        UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, dev, vlanId,
                                              unregIpmEVidxMode, unregIpmEVidx);

        /*
            1.4. Call function with out of range
             unregIpmEVidxMode [CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_BOTH_E],
             unregIpmEVidx [0] and
             vlanId [100].
            Expected: GT_BAD_PARAM.
        */
        vlanId = 100;
        unregIpmEVidx = 0;

        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanUnregisteredIpmEVidxSet
                            (dev, vlanId, unregIpmEVidxMode, unregIpmEVidx),
                            unregIpmEVidxMode);
    }
    /* restore correct values*/
    unregIpmEVidx = 0;
    vlanId = BRG_VLAN_TESTED_VLAN_ID_CNS;
    unregIpmEVidxMode = GT_FALSE;

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanUnregisteredIpmEVidxSet(dev, vlanId,
                                              unregIpmEVidxMode, unregIpmEVidx);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanUnregisteredIpmEVidxSet(dev, vlanId, unregIpmEVidxMode,
                                                unregIpmEVidx);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanMembersTableIndexingModeSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DIRECTION_ENT                      direction,
    IN  CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT   mode
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanMembersTableIndexingModeSet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1. Call function with direction [ CPSS_DIRECTION_INGRESS_E /
                                        CPSS_DIRECTION_EGRESS_E /
                                        CPSS_DIRECTION_BOTH_E] and
                            mode [ CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VLAN_E,
                                   CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VID1_E].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgVlanMembersTableIndexingModeGet with non-NULL mode.
    Expected: GT_OK and the same mode.
    1.3. Call function with out of range direction
         and other parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.4. Call function with out of range mode
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      notAppFamilyBmp = 0;
    CPSS_DIRECTION_ENT direction = CPSS_DIRECTION_INGRESS_E;
    CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT mode    =
                                       CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VLAN_E;
    CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT modeGet =
                                       CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VLAN_E;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with
                            direction [ CPSS_DIRECTION_INGRESS_E /
                                        CPSS_DIRECTION_EGRESS_E /
                                        CPSS_DIRECTION_BOTH_E] and
                            mode [ CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VLAN_E,
                                   CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VID1_E].
            Expected: GT_OK.
        */
        /*
            Call with direction [CPSS_DIRECTION_INGRESS_E]
                      mode [CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VLAN_E],
        */
        direction = CPSS_DIRECTION_INGRESS_E;
        mode      = CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VLAN_E;

        st = cpssDxChBrgVlanMembersTableIndexingModeSet(dev, direction, mode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, direction, mode);

        /* Verifying values */
        st = cpssDxChBrgVlanMembersTableIndexingModeGet(dev, direction,
                                                        &modeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                         "cpssDxChBrgVlanMembersTableIndexingModeGet: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                                      "get another mode than was set: %d", dev);

        /*
            Call with direction [CPSS_DIRECTION_EGRESS_E]
                      mode [CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VLAN_E],
        */
        direction = CPSS_DIRECTION_EGRESS_E;

        st = cpssDxChBrgVlanMembersTableIndexingModeSet(dev, direction, mode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, direction, mode);

        /* Verifying values */
        st = cpssDxChBrgVlanMembersTableIndexingModeGet(dev, direction,
                                                                      &modeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgVlanMembersTableIndexingModeGet: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                   "get another mode than was set: %d", dev);

        /*
            Call with direction [CPSS_DIRECTION_BOTH_E]
                      mode [CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VLAN_E],
        */
        direction = CPSS_DIRECTION_BOTH_E;

        st = cpssDxChBrgVlanMembersTableIndexingModeSet(dev, direction, mode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, direction, mode);

        /* Verifying values */
        direction = CPSS_DIRECTION_INGRESS_E;

        st = cpssDxChBrgVlanMembersTableIndexingModeGet(dev, direction,
                                                                      &modeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgVlanMembersTableIndexingModeGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                   "get another mode than was set: %d", dev);

        direction = CPSS_DIRECTION_EGRESS_E;

        st = cpssDxChBrgVlanMembersTableIndexingModeGet(dev, direction,
                                                                      &modeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgVlanMembersTableIndexingModeGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                   "get another mode than was set: %d", dev);

        /*
            Call with direction [CPSS_DIRECTION_INGRESS_E]
                      mode [CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VID1_E],
        */
        direction = CPSS_DIRECTION_INGRESS_E;
        mode      = CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VID1_E;

        st = cpssDxChBrgVlanMembersTableIndexingModeSet(dev, direction, mode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, direction, mode);

        /* Verifying values */
        st = cpssDxChBrgVlanMembersTableIndexingModeGet(dev, direction,
                                                        &modeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                         "cpssDxChBrgVlanMembersTableIndexingModeGet: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                                      "get another mode than was set: %d", dev);

        /*
            Call with direction [CPSS_DIRECTION_EGRESS_E]
                      mode [CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VID1_E],
        */
        direction = CPSS_DIRECTION_EGRESS_E;

        st = cpssDxChBrgVlanMembersTableIndexingModeSet(dev, direction, mode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, direction, mode);

        /* Verifying values */
        st = cpssDxChBrgVlanMembersTableIndexingModeGet(dev, direction,
                                                                      &modeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgVlanMembersTableIndexingModeGet: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                   "get another mode than was set: %d", dev);

        /*
            Call with direction [CPSS_DIRECTION_BOTH_E]
                      mode [CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VID1_E],
        */
        direction = CPSS_DIRECTION_BOTH_E;

        st = cpssDxChBrgVlanMembersTableIndexingModeSet(dev, direction, mode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, direction, mode);

        /*
            1.2. Call cpssDxChBrgVlanMembersTableIndexingModeGet with
                 non-NULL mode.
            Expected: GT_OK and the same mode.
        */
        direction = CPSS_DIRECTION_INGRESS_E;

        st = cpssDxChBrgVlanMembersTableIndexingModeGet(dev, direction,
                                                                      &modeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgVlanMembersTableIndexingModeGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                   "get another mode than was set: %d", dev);

        direction = CPSS_DIRECTION_EGRESS_E;

        st = cpssDxChBrgVlanMembersTableIndexingModeGet(dev, direction,
                                                                      &modeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgVlanMembersTableIndexingModeGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                   "get another mode than was set: %d", dev);

        /*
            1.3. Call function with out of range direction
                 and other parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanMembersTableIndexingModeSet
                            (dev, direction, mode),
                            direction);

        direction = CPSS_DIRECTION_INGRESS_E;

        /*
            1.4. Call function with direction and out of range mode
            Expected: GT_BAD_PARAM.
        */

        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanMembersTableIndexingModeSet
                            (dev, direction, mode),
                            mode);

        mode = CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VLAN_E;
    }

    /*
      2. For not-active devices and devices from non-applicable family
         check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanMembersTableIndexingModeSet(dev, direction, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanMembersTableIndexingModeSet(dev, direction, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanMembersTableIndexingModeGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DIRECTION_ENT                      direction,
    OUT CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT   *modePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanMembersTableIndexingModeGet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1. Call function with direction [ CPSS_DIRECTION_INGRESS_E /
                                        CPSS_DIRECTION_EGRESS_E] and
                            non-NULL modePtr.
    Expected: GT_OK.
    1.2. Call function with non valid direction [CPSS_DIRECTION_BOTH_E] and
         non-NULL modePtr.
    Expected: GT_BAD_PARAM.
    1.3. Call function with out of range direction and non-NULL modePtr.
    Expected: GT_BAD_PARAM.
    1.4. Call function with modePtr [NULL].
    Expected: GT_BAD_PTR.

*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      notAppFamilyBmp = 0;
    CPSS_DIRECTION_ENT  direction = CPSS_DIRECTION_INGRESS_E;
    CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT modePtr =
                                       CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VLAN_E;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with direction [ CPSS_DIRECTION_INGRESS_E /
                                                CPSS_DIRECTION_EGRESS_E] and
                                    non-NULL modePtr.
            Expected: GT_OK.
        */

        /* Call with direction [CPSS_DIRECTION_INGRESS_E] */
        direction = CPSS_DIRECTION_INGRESS_E;

        st = cpssDxChBrgVlanMembersTableIndexingModeGet(dev, direction,
                                                                      &modePtr);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, direction, modePtr);

        /* Call with direction [CPSS_DIRECTION_EGRESS_E] */
        direction = CPSS_DIRECTION_EGRESS_E;

        st = cpssDxChBrgVlanMembersTableIndexingModeGet(dev, direction,
                                                                      &modePtr);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, direction, modePtr);
        /*
            1.2. Call function with non valid direction [CPSS_DIRECTION_BOTH_E]
                 and non-NULL modePtr.
            Expected: GT_BAD_PARAM.
        */
        direction = CPSS_DIRECTION_BOTH_E;

        st = cpssDxChBrgVlanMembersTableIndexingModeGet(dev, direction,
                                                                      &modePtr);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, direction, modePtr);

        direction = CPSS_DIRECTION_INGRESS_E;
        /*
            1.3. Call function with out of range direction
                 and non-NULL modePtr.
            Expected: GT_BAD_PARAM.
        */

        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanMembersTableIndexingModeGet
                            (dev, direction, &modePtr),
                            direction);

        direction = CPSS_DIRECTION_INGRESS_E;

        /*
            1.4. Call function with modePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgVlanMembersTableIndexingModeGet(dev, direction, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, modePtr = NULL", dev);
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanMembersTableIndexingModeGet(dev, direction,
                                                                      &modePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanMembersTableIndexingModeGet(dev, direction, &modePtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanStgIndexingModeSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DIRECTION_ENT                      direction,
    IN  CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT   mode
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanStgIndexingModeSet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1. Call function with direction [ CPSS_DIRECTION_INGRESS_E /
                                        CPSS_DIRECTION_EGRESS_E /
                                        CPSS_DIRECTION_BOTH_E] and
                            mode [ CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VLAN_E,
                                   CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VID1_E].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgVlanStgIndexingModeGet with non-NULL mode.
    Expected: GT_OK and the same mode.
    1.3. Call function with out of range direction
         and other parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.4. Call function with out of range mode
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      notAppFamilyBmp = 0;
    CPSS_DIRECTION_ENT direction = CPSS_DIRECTION_INGRESS_E;
    CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT mode    =
                                       CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VLAN_E;
    CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT modeGet =
                                       CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VLAN_E;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with
                            direction [ CPSS_DIRECTION_INGRESS_E /
                                        CPSS_DIRECTION_EGRESS_E /
                                        CPSS_DIRECTION_BOTH_E] and
                            mode [ CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VLAN_E,
                                   CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VID1_E].
            Expected: GT_OK.
        */

        /*
            Call with direction [CPSS_DIRECTION_INGRESS_E]
                      mode [CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VLAN_E],
        */
        direction = CPSS_DIRECTION_INGRESS_E;
        mode      = CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VLAN_E;

        st = cpssDxChBrgVlanStgIndexingModeSet(dev, direction, mode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, direction, mode);

        /* Verifying values */
        st = cpssDxChBrgVlanStgIndexingModeGet(dev, direction, &modeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                  "cpssDxChBrgVlanStgIndexingModeGet: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                                      "get another mode than was set: %d", dev);

        /*
            Call with direction [CPSS_DIRECTION_EGRESS_E]
                      mode [CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VLAN_E],
        */
        direction = CPSS_DIRECTION_EGRESS_E;

        st = cpssDxChBrgVlanStgIndexingModeSet(dev, direction, mode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, direction, mode);

        /* Verifying values */
        st = cpssDxChBrgVlanStgIndexingModeGet(dev, direction, &modeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                  "cpssDxChBrgVlanStgIndexingModeGet: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                                      "get another mode than was set: %d", dev);

        /*
            Call with direction [CPSS_DIRECTION_BOTH_E]
                      mode [CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VLAN_E],
        */
        direction = CPSS_DIRECTION_BOTH_E;

        st = cpssDxChBrgVlanStgIndexingModeSet(dev, direction, mode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, direction, mode);

        /* Verifying values */
        direction = CPSS_DIRECTION_EGRESS_E;

        st = cpssDxChBrgVlanStgIndexingModeGet(dev, direction, &modeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                  "cpssDxChBrgVlanStgIndexingModeGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                                      "get another mode than was set: %d", dev);

        direction = CPSS_DIRECTION_INGRESS_E;

        st = cpssDxChBrgVlanStgIndexingModeGet(dev, direction, &modeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                  "cpssDxChBrgVlanStgIndexingModeGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                                      "get another mode than was set: %d", dev);

        /*
            Call with direction [CPSS_DIRECTION_INGRESS_E]
                      mode [CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VID1_E],
        */
        direction = CPSS_DIRECTION_INGRESS_E;
        mode      = CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VID1_E;

        st = cpssDxChBrgVlanStgIndexingModeSet(dev, direction, mode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, direction, mode);

        /* Verifying values */
        st = cpssDxChBrgVlanStgIndexingModeGet(dev, direction, &modeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                  "cpssDxChBrgVlanStgIndexingModeGet: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                                      "get another mode than was set: %d", dev);

        /*
            Call with direction [CPSS_DIRECTION_EGRESS_E]
                      mode [CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VID1_E],
        */
        direction = CPSS_DIRECTION_EGRESS_E;

        st = cpssDxChBrgVlanStgIndexingModeSet(dev, direction, mode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, direction, mode);

        /* Verifying values */
        st = cpssDxChBrgVlanStgIndexingModeGet(dev, direction, &modeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                  "cpssDxChBrgVlanStgIndexingModeGet: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                                      "get another mode than was set: %d", dev);

        /*
            Call with direction [CPSS_DIRECTION_BOTH_E]
                      mode [CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VID1_E],
        */
        direction = CPSS_DIRECTION_BOTH_E;

        st = cpssDxChBrgVlanStgIndexingModeSet(dev, direction, mode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, direction, mode);

        /*
            1.2. Call cpssDxChBrgVlanStgIndexingModeGet with
                 non-NULL mode.
            Expected: GT_OK and the same mode.
        */

        direction = CPSS_DIRECTION_EGRESS_E;

        st = cpssDxChBrgVlanStgIndexingModeGet(dev, direction, &modeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                  "cpssDxChBrgVlanStgIndexingModeGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                                      "get another mode than was set: %d", dev);

        direction = CPSS_DIRECTION_INGRESS_E;

        st = cpssDxChBrgVlanStgIndexingModeGet(dev, direction, &modeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                  "cpssDxChBrgVlanStgIndexingModeGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                                      "get another mode than was set: %d", dev);

        /*
            1.3. Call function with out of range direction
                 and other parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanStgIndexingModeSet
                            (dev, direction, mode),
                            direction);

        direction = CPSS_DIRECTION_INGRESS_E;

        /*
            1.4. Call function with direction and out of range mode
            Expected: GT_BAD_PARAM.
        */

        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanStgIndexingModeSet
                            (dev, direction, mode),
                            mode);

        mode = CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VLAN_E;
    }

    /*
      2. For not-active devices and devices from non-applicable family
         check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanStgIndexingModeSet(dev, direction, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanStgIndexingModeSet(dev, direction, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanStgIndexingModeGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DIRECTION_ENT                      direction,
    OUT CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT   *modePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanStgIndexingModeGet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1. Call function with direction [ CPSS_DIRECTION_INGRESS_E /
                                        CPSS_DIRECTION_EGRESS_E] and
                            non-NULL modePtr.
    Expected: GT_OK.
    1.2. Call function with non valid direction [CPSS_DIRECTION_BOTH_E]
         and non-NULL modePtr.
    Expected: GT_BAD_PARAM.
    1.3. Call function with out of range direction and non-NULL modePtr.
    Expected: GT_BAD_PARAM.
    1.4. Call function with modePtr [NULL].
    Expected: GT_BAD_PTR.

*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      notAppFamilyBmp = 0;
    CPSS_DIRECTION_ENT  direction = CPSS_DIRECTION_INGRESS_E;
    CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT modePtr =
                                       CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VLAN_E;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with direction [ CPSS_DIRECTION_INGRESS_E /
                                                CPSS_DIRECTION_EGRESS_E] and
                                    non-NULL modePtr.
            Expected: GT_OK.
        */

        /* Call with direction [CPSS_DIRECTION_INGRESS_E] */
        direction = CPSS_DIRECTION_INGRESS_E;

        st = cpssDxChBrgVlanStgIndexingModeGet(dev, direction, &modePtr);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, direction, modePtr);

        /* Call with direction [CPSS_DIRECTION_EGRESS_E] */
        direction = CPSS_DIRECTION_EGRESS_E;

        st = cpssDxChBrgVlanStgIndexingModeGet(dev, direction, &modePtr);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, direction, modePtr);

        /*
            1.2. Call function with non valid direction [CPSS_DIRECTION_BOTH_E]
                 and non-NULL modePtr.
            Expected: GT_BAD_PARAM.
        */
        direction = CPSS_DIRECTION_BOTH_E;

        st = cpssDxChBrgVlanStgIndexingModeGet(dev, direction, &modePtr);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, direction, modePtr);

        direction = CPSS_DIRECTION_INGRESS_E;
        /*
            1.3. Call function with out of range direction
                 and non-NULL modePtr.
            Expected: GT_BAD_PARAM.
        */

        UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanStgIndexingModeGet
                            (dev, direction, &modePtr),
                            direction);

        direction = CPSS_DIRECTION_INGRESS_E;

        /*
            1.4. Call function with modePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgVlanStgIndexingModeGet(dev, direction, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, modePtr = NULL", dev);
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanStgIndexingModeGet(dev, direction, &modePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanStgIndexingModeGet(dev, direction, &modePtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanTagStateIndexingModeSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT   mode
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanTagStateIndexingModeSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call function with mode [ CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VLAN_E,
                                     CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VID1_E]
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgVlanTagStateIndexingModeGet with
           not NULL modePtr
    Expected: GT_OK
    1.1.3. Call function with out of range mode
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS               st  = GT_OK;
    GT_U8                   dev;
    GT_PHYSICAL_PORT_NUM    port = BRG_VLAN_VALID_PHY_PORT_CNS;
    GT_U32                  notAppFamilyBmp = 0;
    CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT mode    =
                                       CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VLAN_E;
    CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT modeGet =
                                       CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VLAN_E;


    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call function with
                                mode [ CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VLAN_E,
                                       CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VID1_E]
               Expected: GT_OK.
            */
            /* iterate with mode = CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VLAN_E */
            mode = CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VLAN_E;

            st = cpssDxChBrgVlanTagStateIndexingModeSet(dev, port, mode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, mode);

            /*
               1.1.2. Call cpssDxChBrgVlanTagStateIndexingModeGet
                      with not NULL modePtr
               Expected: GT_OK
            */
            st = cpssDxChBrgVlanTagStateIndexingModeGet(dev, port, &modeGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgVlanTagStateIndexingModeGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(mode, modeGet,
                            "got another mode than was set: %d, %d", dev, port);

            /* iterate with mode = CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VID1_E */
            mode = CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VID1_E;

            st = cpssDxChBrgVlanTagStateIndexingModeSet(dev, port, mode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, mode);

            /*
               1.1.2. Call cpssDxChBrgVlanTagStateIndexingModeGet
                      with not NULL modePtr
               Expected: GT_OK
            */
            st = cpssDxChBrgVlanTagStateIndexingModeGet(dev, port, &modeGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgVlanTagStateIndexingModeGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(mode, modeGet,
                            "got another mode than was set: %d, %d", dev, port);

            /*
                1.1.3. Call function with direction and out of range mode
                Expected: GT_BAD_PARAM.
            */

            UTF_ENUMS_CHECK_MAC(cpssDxChBrgVlanTagStateIndexingModeSet
                                (dev, port, mode),
                                mode);

            mode = CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VLAN_E;
        }

        /* restore correct values*/
        mode = CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VLAN_E;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.2. For all active devices go over all non available physical ports
        */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanTagStateIndexingModeSet(dev, port, mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /*
            1.3. For active device check that function returns GT_BAD_PARAM
                 for out of bound value for port number.
        */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanTagStateIndexingModeSet(dev, port, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /*
            1.4. For active device check that function returns GT_OK
                 for CPU port number.
        */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanTagStateIndexingModeSet(dev, port, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    mode = CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VLAN_E;
    port = BRG_VLAN_VALID_PHY_PORT_CNS;

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanTagStateIndexingModeSet(dev, port, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanTagStateIndexingModeSet(dev, port, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanTagStateIndexingModeGet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    OUT CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT   *modePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanTagStateIndexingModeGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call with not NULL modePtr.
    Expected: GT_OK.
    1.1.2. Call with modePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st  = GT_OK;
    GT_U8                   dev;
    GT_PHYSICAL_PORT_NUM    port = BRG_VLAN_VALID_PHY_PORT_CNS;
    GT_U32                  notAppFamilyBmp = 0;
    CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT modePtr =
                                       CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VLAN_E;


    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with not NULL modePtr.
                Expected: GT_OK.
            */
            st = cpssDxChBrgVlanTagStateIndexingModeGet(dev, port, &modePtr);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call with modePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgVlanTagStateIndexingModeGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                                         "%d, %d, modePtr = NULL", dev, port);
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.2. For all active devices go over all non available physical ports
        */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanTagStateIndexingModeGet(dev, port, &modePtr);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /*
            1.3. For active device check that function returns GT_BAD_PARAM
                 for out of bound value for port number.
        */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanTagStateIndexingModeGet(dev, port, &modePtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /*
            1.4. For active device check that function returns GT_OK
                 for CPU port number.
        */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanTagStateIndexingModeGet(dev, port, &modePtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    port = BRG_VLAN_VALID_PHY_PORT_CNS;

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanTagStateIndexingModeGet(dev, port, &modePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanTagStateIndexingModeGet(dev, port, &modePtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChBrgVlanBpeTagSourceCidDefaultForceSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_BOOL          forceSrcCidDefault
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanBpeTagSourceCidDefaultForceSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call function with forceSrcCidDefault [GT_TRUE, GT_FALSE]
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgVlanBpeTagSourceCidDefaultForceGet with
           not NULL modePtr
    Expected: GT_OK
    1.1.3. Call function with out of range mode
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS        st  = GT_OK;
    GT_U8            dev;
    GT_PORT_NUM      port;
    GT_U32           notAppFamilyBmp = 0;
    GT_BOOL          forceSrcCidDefaultArr[2]     = {GT_TRUE, GT_FALSE};
    GT_BOOL          forceSrcCidDefaultGet;
    GT_U32           idx;


    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available ePorts. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            for (idx = 0;
                  (idx < (sizeof(forceSrcCidDefaultArr) / sizeof(forceSrcCidDefaultArr[0])));
                  idx++)
            {

                /*
                    1.1.1. Call function with
                                    forceSrcCidDefault [GT_TRUE, GT_FALSE]
                   Expected: GT_OK.
                */
                st = cpssDxChBrgVlanBpeTagSourceCidDefaultForceSet(
                    dev, port, forceSrcCidDefaultArr[idx]);
                UTF_VERIFY_EQUAL3_PARAM_MAC(
                    GT_OK, st, dev, port, forceSrcCidDefaultArr[idx]);
                /*
                   1.1.2. Call cpssDxChBrgVlanBpeTagSourceCidDefaultForceGet
                          with not NULL forceSrcCidDefaultPtr
                   Expected: GT_OK
                */
                st = cpssDxChBrgVlanBpeTagSourceCidDefaultForceGet(
                    dev, port, &forceSrcCidDefaultGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgVlanBpeTagSourceCidDefaultForceGet: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(
                    forceSrcCidDefaultArr[idx], forceSrcCidDefaultGet,
                    "got another forceSrcCidDefault than was set: %d, %d", dev, port);
            }
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.2. For all active devices go over all non available ePorts
        */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanBpeTagSourceCidDefaultForceSet(
                dev, port, forceSrcCidDefaultArr[0]);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanBpeTagSourceCidDefaultForceSet(
            dev, port, forceSrcCidDefaultArr[0]);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    port = 0;
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanBpeTagSourceCidDefaultForceSet(
            dev, port, forceSrcCidDefaultArr[0]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanBpeTagSourceCidDefaultForceSet(
        dev, port, forceSrcCidDefaultArr[0]);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChBrgVlanBpeTagSourceCidDefaultForceGet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    OUT GT_BOOL          *forceSrcCidDefaultPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanBpeTagSourceCidDefaultForceGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with not NULL forceSrcCidDefaultPtr
    Expected: GT_OK.
    1.1.2. Call api with wrong forceSrcCidDefaultPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8        dev;
    GT_PORT_NUM  portNum;
    GT_BOOL      forceSrcCidDefault;
    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with not NULL enablePtr
                Expected: GT_OK.
            */
            st = cpssDxChBrgVlanBpeTagSourceCidDefaultForceGet(
                dev, portNum, &forceSrcCidDefault);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call api with wrong enablePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgVlanBpeTagSourceCidDefaultForceGet(
                dev, portNum, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                       "%d, enablePtr = NULL", dev);
        }

        st = prvUtfNextVirtPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgVlanBpeTagSourceCidDefaultForceGet(
                dev, portNum, &forceSrcCidDefault);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgVlanBpeTagSourceCidDefaultForceGet(
            dev, portNum, &forceSrcCidDefault);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgVlanBpeTagSourceCidDefaultForceGet(
            dev, portNum, &forceSrcCidDefault);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
    }

    /* restore correct values */
    portNum = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanBpeTagSourceCidDefaultForceGet(
            dev, portNum, &forceSrcCidDefault);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanBpeTagSourceCidDefaultForceGet(
        dev, portNum, &forceSrcCidDefault);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChBrgVlanBpeTagMcCfgSet
(
    IN  GT_U8             devNum,
    IN  GT_BOOL           eCidOffsetNegative,
    IN  GT_U32            eCidOffset,
    IN  GT_U32            defaultSrcECid
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanBpeTagMcCfgSet)
{
    GT_STATUS st     = GT_OK;
    GT_U8     dev;
    GT_U32    notAppFamilyBmp = 0;
    GT_BOOL   eCidOffsetNegativeGet;
    GT_U32    eCidOffsetGet;
    GT_U32    defaultSrcECidGet;
    GT_U32    i;

    struct
    {
        GT_STATUS rc;
        GT_BOOL   eCidOffsetNegative;
        GT_U32    eCidOffset;
        GT_U32    defaultSrcECid;
    } dataArr[] =
    {
        {GT_OK, GT_FALSE, 0, 0},
        {GT_OK, GT_TRUE, 0x3FFF, 0xFFF},
        {GT_OUT_OF_RANGE, GT_FALSE, 0x4000, 0},
        {GT_OUT_OF_RANGE, GT_FALSE, 0, 0x1000}
    };

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (i = 0; (i < (sizeof(dataArr) / sizeof(dataArr[0]))); i++)
        {
            st = cpssDxChBrgVlanBpeTagMcCfgSet(
                dev, dataArr[i].eCidOffsetNegative,
                dataArr[i].eCidOffset, dataArr[i].defaultSrcECid);
            UTF_VERIFY_EQUAL2_PARAM_MAC(dataArr[i].rc, st, i, dev);
            if (dataArr[i].rc != GT_OK)
            {
                continue;
            }
            st = cpssDxChBrgVlanBpeTagMcCfgGet(
                dev, &eCidOffsetNegativeGet,
                &eCidOffsetGet, &defaultSrcECidGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(dataArr[i].rc, st, i, dev);
            UTF_VERIFY_EQUAL0_PARAM_MAC(
                dataArr[i].eCidOffsetNegative, eCidOffsetNegativeGet);
            UTF_VERIFY_EQUAL0_PARAM_MAC(
                dataArr[i].eCidOffset, eCidOffsetGet);
            UTF_VERIFY_EQUAL0_PARAM_MAC(
                dataArr[i].defaultSrcECid, defaultSrcECidGet);
        }
    }


    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanBpeTagMcCfgSet(
            dev, dataArr[0].eCidOffsetNegative,
            dataArr[0].eCidOffset, dataArr[0].defaultSrcECid);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanBpeTagMcCfgSet(
        dev, dataArr[0].eCidOffsetNegative,
        dataArr[0].eCidOffset, dataArr[0].defaultSrcECid);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChBrgVlanBpeTagMcCfgGet
(
    IN  GT_U8             devNum,
    OUT GT_BOOL           *eCidOffsetNegativePtr,
    OUT GT_U32            *eCidOffsetPtr,
    OUT GT_U32            *defaultSrcECidPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanBpeTagMcCfgGet)
{
    GT_STATUS         st    = GT_OK;
    GT_U32            notAppFamilyBmp = 0;
    GT_U8             dev;
    GT_BOOL           eCidOffsetNegative;
    GT_U32            eCidOffset;
    GT_U32            defaultSrcECid;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* good call */
        st = cpssDxChBrgVlanBpeTagMcCfgGet(
            dev, &eCidOffsetNegative, &eCidOffset, &defaultSrcECid);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  NULL pointer checking */
        st = cpssDxChBrgVlanBpeTagMcCfgGet(
            dev, NULL, &eCidOffset, &defaultSrcECid);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        st = cpssDxChBrgVlanBpeTagMcCfgGet(
            dev, &eCidOffsetNegative, NULL, &defaultSrcECid);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        st = cpssDxChBrgVlanBpeTagMcCfgGet(
            dev, &eCidOffsetNegative, &eCidOffset, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanBpeTagMcCfgGet(
            dev, &eCidOffsetNegative, &eCidOffset, &defaultSrcECid);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanBpeTagMcCfgGet(
        dev, &eCidOffsetNegative, &eCidOffset, &defaultSrcECid);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChBrgVlanBpeTagReservedFieldsSet
(
    IN  GT_U8             devNum,
    IN  GT_U32            reValue,
    IN  GT_U32            ingressECidExtValue,
    IN  GT_U32            eCidExtValue
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanBpeTagReservedFieldsSet)
{
    GT_STATUS st     = GT_OK;
    GT_U8     dev;
    GT_U32    notAppFamilyBmp = 0;
    GT_U32    reValueGet;
    GT_U32    ingressECidExtValueGet;
    GT_U32    eCidExtValueGet;
    GT_U32    i;

    struct
    {
        GT_STATUS rc;
        GT_U32    reValue;
        GT_U32    ingressECidExtValue;
        GT_U32    eCidExtValue;
    } dataArr[] =
    {
        {GT_OK, 0, 0, 0},
        {GT_OK, 3, 0xFF, 0xFF},
        {GT_OUT_OF_RANGE, 4, 0, 0},
        {GT_OUT_OF_RANGE, 0, 0x100, 0},
        {GT_OUT_OF_RANGE, 0, 0, 0x100}
    };

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (i = 0; (i < (sizeof(dataArr) / sizeof(dataArr[0]))); i++)
        {
            st = cpssDxChBrgVlanBpeTagReservedFieldsSet(
                dev, dataArr[i].reValue,
                dataArr[i].ingressECidExtValue, dataArr[i].eCidExtValue);
            UTF_VERIFY_EQUAL2_PARAM_MAC(dataArr[i].rc, st, i, dev);
            if (dataArr[i].rc != GT_OK)
            {
                continue;
            }
            st = cpssDxChBrgVlanBpeTagReservedFieldsGet(
                dev, &reValueGet,
                &ingressECidExtValueGet, &eCidExtValueGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(dataArr[i].rc, st, i, dev);
            UTF_VERIFY_EQUAL0_PARAM_MAC(
                dataArr[i].reValue, reValueGet);
            UTF_VERIFY_EQUAL0_PARAM_MAC(
                dataArr[i].ingressECidExtValue, ingressECidExtValueGet);
            UTF_VERIFY_EQUAL0_PARAM_MAC(
                dataArr[i].eCidExtValue, eCidExtValueGet);
        }
    }


    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanBpeTagReservedFieldsSet(
            dev, dataArr[0].reValue,
            dataArr[0].ingressECidExtValue, dataArr[0].eCidExtValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanBpeTagReservedFieldsSet(
        dev, dataArr[0].reValue,
        dataArr[0].ingressECidExtValue, dataArr[0].eCidExtValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChBrgVlanBpeTagReservedFieldsGet
(
    IN  GT_U8             devNum,
    OUT GT_U32            *reValuePtr,
    OUT GT_U32            *ingressECidExtValuePtr,
    OUT GT_U32            *eCidExtValuePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanBpeTagReservedFieldsGet)
{
    GT_STATUS         st    = GT_OK;
    GT_U32            notAppFamilyBmp = 0;
    GT_U8             dev;
    GT_U32            reValue;
    GT_U32            ingressECidExtValue;
    GT_U32            eCidExtValue;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* good call */
        st = cpssDxChBrgVlanBpeTagReservedFieldsGet(
            dev, &reValue, &ingressECidExtValue, &eCidExtValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  NULL pointer checking */
        st = cpssDxChBrgVlanBpeTagReservedFieldsGet(
            dev, NULL, &ingressECidExtValue, &eCidExtValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        st = cpssDxChBrgVlanBpeTagReservedFieldsGet(
            dev, &reValue, NULL, &eCidExtValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        st = cpssDxChBrgVlanBpeTagReservedFieldsGet(
            dev, &reValue, &ingressECidExtValue, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgVlanBpeTagReservedFieldsGet(
            dev, &reValue, &ingressECidExtValue, &eCidExtValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgVlanBpeTagReservedFieldsGet(
        dev, &reValue, &ingressECidExtValue, &eCidExtValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlanFdbLookupKeyModeSet
(
    IN GT_U8                                 devNum,
    IN GT_U16                                vlanId,
    IN CPSS_DXCH_BRG_FDB_LOOKUP_KEY_MODE_ENT mode
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlanFdbLookupKeyModeSet)
{
/*
    1. Iterate over all active devices (eArch devices):
      1.1. call with vlanId = 100, mode - all from the type's range.
                          Expected: GT_OK (for all devices  except bobcat 2 a0)
                                    GT_NOT_APPLICABLE_DEVICE (for bobcat2 a0)
      1.2. Call with vlan Id out of range. Expected GT_BAD_PARAM
    2. Call with non active device id.     Expected: GT_NOT_APPLICABLE_DEVICE
    3. Call with device id out of range.   Expected: GT_BAD_PARAM

*/
    GT_STATUS                             rc              = GT_OK;
    GT_U8                                 devNum          = 0;
    GT_U16                                vlanId          = 0;
    GT_U16                                badVlanId       = 0;
    GT_BOOL                               notAppFamilyBmp = 0;
    CPSS_DXCH_BRG_FDB_LOOKUP_KEY_MODE_ENT mode1;
    CPSS_DXCH_BRG_FDB_LOOKUP_KEY_MODE_ENT mode2;

    vlanId    = BRG_VLAN_TESTED_VLAN_ID_CNS;
    mode1     = CPSS_DXCH_BRG_FDB_LOOKUP_KEY_MODE_FID_E;
    mode2     = CPSS_DXCH_BRG_FDB_LOOKUP_KEY_MODE_FID_VID1_E;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        badVlanId = UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(devNum);
        /* 1.1 */
        if (!UTF_CPSS_SIP5_0_ONLY_CHECK_MAC(devNum))
        {
            rc = cpssDxChBrgVlanFdbLookupKeyModeSet(devNum, vlanId, mode1);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, devNum);

            rc = cpssDxChBrgVlanFdbLookupKeyModeSet(devNum, vlanId, mode2);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, devNum);

            /* 1.2 */
            rc = cpssDxChBrgVlanFdbLookupKeyModeSet(devNum, badVlanId, mode1);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, devNum);
        }
        else
        {
            rc = cpssDxChBrgVlanFdbLookupKeyModeSet(devNum, vlanId, mode1);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, devNum);
        }
    }

    /* 2. */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        rc = cpssDxChBrgVlanFdbLookupKeyModeSet(devNum, vlanId, mode1);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, devNum);
    }

    /* 4.2.Call function with an out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    rc = cpssDxChBrgVlanFdbLookupKeyModeSet(devNum, vlanId, mode1);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgVlan6BytesTagConfigSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  isPortExtender,
    IN GT_U32   lBitInSrcId,
    IN GT_U32   dBitInSrcId
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlan6BytesTagConfigSet)
{
    GT_STATUS                             rc              = GT_OK;
    GT_U8                                 devNum          = 0;
    GT_U32      notAppFamilyBmp;
    IN GT_BOOL  isPortExtender,isPortExtenderGet;
    IN GT_U32   lBitInSrcId,lBitInSrcIdGet;
    IN GT_U32   dBitInSrcId,dBitInSrcIdGet;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        for(isPortExtender = GT_FALSE ; isPortExtender <= GT_TRUE ; isPortExtender++)
        {
            /* test combinations of good values of lBitInSrcId , dBitInSrcId */
            for(lBitInSrcId = 0 ; lBitInSrcId < 15 ; lBitInSrcId ++)
            {
                dBitInSrcId = 15 - lBitInSrcId;

                /* 1.1 */
                rc = cpssDxChBrgVlan6BytesTagConfigSet(devNum, isPortExtender, lBitInSrcId , dBitInSrcId);
                if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
                {
                    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, devNum);

                    rc = cpssDxChBrgVlan6BytesTagConfigGet(devNum, &isPortExtenderGet, &lBitInSrcIdGet , &dBitInSrcIdGet);
                    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, devNum);

                    UTF_VERIFY_EQUAL1_PARAM_MAC(isPortExtender, isPortExtenderGet, devNum);
                    UTF_VERIFY_EQUAL1_PARAM_MAC(lBitInSrcId, lBitInSrcIdGet, devNum);
                    UTF_VERIFY_EQUAL1_PARAM_MAC(dBitInSrcId, dBitInSrcIdGet, devNum);
                }
                else
                {
                    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, devNum);
                }
            }

            /* test combinations of good and bad values of lBitInSrcId , dBitInSrcId */
            for(lBitInSrcId = 15 ; lBitInSrcId <= 17 ; lBitInSrcId ++)
            {
                for(dBitInSrcId = 15 ; dBitInSrcId <= 17 ; dBitInSrcId ++)
                {
                    rc = cpssDxChBrgVlan6BytesTagConfigSet(devNum, isPortExtender, lBitInSrcId , dBitInSrcId);
                    if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
                    {
                        if(lBitInSrcId > 15 || dBitInSrcId > 15)
                        {
                            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, rc, devNum);
                        }
                        else
                        {
                            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, devNum);

                            rc = cpssDxChBrgVlan6BytesTagConfigGet(devNum, &isPortExtenderGet, &lBitInSrcIdGet , &dBitInSrcIdGet);
                            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, devNum);

                            UTF_VERIFY_EQUAL1_PARAM_MAC(isPortExtender, isPortExtenderGet, devNum);
                            UTF_VERIFY_EQUAL1_PARAM_MAC(lBitInSrcId, lBitInSrcIdGet, devNum);
                            UTF_VERIFY_EQUAL1_PARAM_MAC(dBitInSrcId, dBitInSrcIdGet, devNum);
                        }
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, devNum);
                    }
                }
            }
        }
    }

    isPortExtender = GT_FALSE;
    lBitInSrcId = 0;
    dBitInSrcId = 0;

    /* 2. */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        rc = cpssDxChBrgVlan6BytesTagConfigSet(devNum, isPortExtender, lBitInSrcId , dBitInSrcId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, devNum);
    }

    /* 4.2.Call function with an out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    rc = cpssDxChBrgVlan6BytesTagConfigSet(devNum, isPortExtender, lBitInSrcId , dBitInSrcId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, devNum);
}

/*
GT_STATUS cpssDxChBrgVlan6BytesTagConfigGet
(
    IN GT_U8    devNum,
    IN GT_BOOL  *isPortExtenderPtr,
    IN GT_U32   *lBitInSrcIdPtr,
    IN GT_U32   *dBitInSrcIdPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgVlan6BytesTagConfigGet)
{
    GT_STATUS                             rc              = GT_OK;
    GT_U8                                 devNum          = 0;
    GT_U32      notAppFamilyBmp;
    IN GT_BOOL  isPortExtenderGet;
    IN GT_U32   lBitInSrcIdGet;
    IN GT_U32   dBitInSrcIdGet;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* good call */
        rc = cpssDxChBrgVlan6BytesTagConfigGet(devNum,
            &isPortExtenderGet, &lBitInSrcIdGet , &dBitInSrcIdGet);
        if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, devNum);
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, devNum);
        }

        /*  NULL pointer checking */
        rc = cpssDxChBrgVlan6BytesTagConfigGet(devNum,
            NULL, &lBitInSrcIdGet , &dBitInSrcIdGet);
        if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, rc, devNum);
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, devNum);
        }

        /*  NULL pointer checking */
        rc = cpssDxChBrgVlan6BytesTagConfigGet(devNum,
            &isPortExtenderGet, NULL , &dBitInSrcIdGet);
        if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, rc, devNum);
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, devNum);
        }

        /*  NULL pointer checking */
        rc = cpssDxChBrgVlan6BytesTagConfigGet(devNum,
            &isPortExtenderGet, &lBitInSrcIdGet , NULL);
        if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, rc, devNum);
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, devNum);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        rc = cpssDxChBrgVlan6BytesTagConfigGet(devNum,
            &isPortExtenderGet, &lBitInSrcIdGet , &dBitInSrcIdGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, devNum);
    }

    /* 3. Call function with out of bound value for device id.*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    rc = cpssDxChBrgVlan6BytesTagConfigGet(devNum,
        &isPortExtenderGet, &lBitInSrcIdGet , &dBitInSrcIdGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, devNum);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChBrgVlan suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChBrgVlan)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanEntryWrite)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanEntriesRangeWrite)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanEntryRead)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanMemberAdd)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortDelete)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortVidGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortVidSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortIngFltEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortIngFltEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortProtoClassVlanEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortProtoClassVlanEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortProtoClassQosEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortProtoClassQosEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanProtoClassSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanProtoClassGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanProtoClassInvalidate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortProtoVlanQosSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortProtoVlanQosGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortProtoVlanQosInvalidate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanLearningStateSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanIsDevMember)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanToStpIdBind)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanStpIdGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanEtherTypeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanEtherTypeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanTableInvalidate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanEntryInvalidate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanUnkUnregFilterSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanForcePvidEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanForcePvidEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortVidPrecedenceSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanIpUcRouteEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanIpMcRouteEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanNASecurEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanIgmpSnoopingEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanIgmpSnoopingEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanIpCntlToCpuSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanIpCntlToCpuGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanIpV6IcmpToCpuEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanIpV6IcmpToCpuEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanUdpBcPktsToCpuEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanUdpBcPktsToCpuEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanIpv6SourceSiteIdSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanIpmBridgingEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanIpmBridgingModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanIngressMirrorEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortAccFrameTypeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortAccFrameTypeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanMruProfileIdxSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanMruProfileValueSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanMruProfileValueGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanNaToCpuEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanBridgingModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanBridgingModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanVrfIdSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortTranslationEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortTranslationEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanTranslationEntryWrite)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanTranslationEntryRead)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanValidCheckEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanValidCheckEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanFloodVidxModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanLocalSwitchingEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanMemberSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortEgressTpidGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortEgressTpidSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortIngressTpidGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortIngressTpidSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortIsolationCmdSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortVidPrecedenceGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanRangeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanRangeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanTpidEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanTpidEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanInit)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanRemoveVlanTag1IfZeroModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanRemoveVlanTag1IfZeroModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanForceNewDsaToCpuEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanForceNewDsaToCpuEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanKeepVlan1EnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanKeepVlan1EnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanEgressPortTagStateGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanEgressPortTagStateModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanEgressPortTagStateModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanEgressPortTagStateSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanEgressTagTpidSelectModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanEgressTagTpidSelectModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanForwardingIdGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanForwardingIdSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortCfiEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortCfiEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortNumOfTagWordsToPopSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortNumOfTagWordsToPopGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortPushVlanCommandGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortPushVlanCommandSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortPushedTagTpidSelectEgressGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortPushedTagTpidSelectEgressSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortPushedTagValueGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortPushedTagValueSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortUp0CommandEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortUp0CommandEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortUp0Get)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortUp0Set)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortUp1CommandEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortUp1CommandEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortUp1Get)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortUp1Set)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortUpAndCfiAssignmentCommandGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortUpAndCfiAssignmentCommandSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortUpGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortUpSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortVid0CommandEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortVid0CommandEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortVid1CommandEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortVid1CommandEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortVid1Get)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortVid1Set)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanUnknownMacSaCommandGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanUnknownMacSaCommandSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanIndependentNonFloodVidxEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanIndependentNonFloodVidxEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanTpidTagTypeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanTpidTagTypeGet)
    /* Test filling Table */
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanFillVlanTable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanFillVlanTranslationTableIngress)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanFillVlanTranslationTableEgress)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanIngressTpidProfileSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanIngressTpidProfileGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortIngressTpidProfileSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortIngressTpidProfileGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanMirrorToTxAnalyzerSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanMirrorToTxAnalyzerModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanFcoeForwardingEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanUnregisteredIpmEVidxSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanMembersTableIndexingModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanMembersTableIndexingModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanStgIndexingModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanStgIndexingModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanTagStateIndexingModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanTagStateIndexingModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanBpeTagSourceCidDefaultForceSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanBpeTagSourceCidDefaultForceGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanBpeTagMcCfgSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanBpeTagMcCfgGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanBpeTagReservedFieldsSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanBpeTagReservedFieldsGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlanFdbLookupKeyModeSet)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlan6BytesTagConfigSet )
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgVlan6BytesTagConfigGet)

UTF_SUIT_END_TESTS_MAC(cpssDxChBrgVlan)

