/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* tgfMirrorGen.c
*
* DESCRIPTION:
*       Generic API for Cut-Through APIs.
*
* FILE REVISION NUMBER:
*       $Revision: 5 $
*
*******************************************************************************/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/private/prvUtfHelpers.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficEngine.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfCutThrough.h>

#ifdef CHX_FAMILY

#include <cpss/dxCh/dxChxGen/cutThrough/cpssDxChCutThrough.h>
#include <cpss/dxCh/dxChxGen/cutThrough/private/prvCpssDxChCutThrough.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortPip.h>

#endif
/******************************************************************************\
 *                       CPSS generic API section                             *
\******************************************************************************/

/**
* @internal prvTgfCutThroughBypassRouterAndPolicerEnableSet function
* @endinternal
*
* @brief   Enable / Disable bypassing the Router and Ingress Policer engines.
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCutThroughBypassRouterAndPolicerEnableSet
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
        rc = cpssDxChCutThroughBypassRouterAndPolicerEnableSet(devNum, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCutThroughBypassRouterAndPolicerEnableSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(enable);

    return GT_NOT_IMPLEMENTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfCutThroughGlobalEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable of Cut Through feature on all related devices.
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_TRUE                  - enable, GT_FALSE - disable
*/
GT_BOOL prvTgfCutThroughGlobalEnableGet
(
    GT_VOID
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc = GT_OK;                                               \
    PRV_TGF_MEMBER_FORCE_INFO_STC *currMemPtr;
    GT_U32  ii;

    if (prvTgfDevPortForceNum)
    {
        /* look for this member in the DB */
        currMemPtr = &prvTgfDevPortForceArray[0];
        for(ii = 0 ; ii < prvTgfDevPortForceNum; ii++,currMemPtr++)
        {
            if(currMemPtr->forceToVlan == GT_FALSE)
            {
                /* member not need to be forced to any vlan */
                continue;
            }
            if (PRV_CPSS_DXCH_PP_MAC(currMemPtr->member.devNum)->cutThroughEnable == GT_FALSE)
            {
                return GT_FALSE;
            }
        }
    }

    /* prepare device iterator */
    devNum = 0;
    rc = prvUtfNextNotApplicableDeviceReset(&devNum, UTF_NONE_FAMILY_E);
    if (rc != GT_OK)
    {
        return GT_FALSE;
    }

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        if (PRV_CPSS_DXCH_PP_MAC(devNum)->cutThroughEnable == GT_FALSE)
        {
            return GT_FALSE;
        }
    }
    return GT_TRUE;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    return GT_TRUE;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_TRUE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfCutThroughPortEnableSet function
* @endinternal
*
* @brief   Enable/Disable Cut Through forwarding for packets received on the port.
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Cut Through forwarding for tagged packets is enabled
*       per source port and UP. See cpssDxChCutThroughUpEnableSet.
*
*/
GT_STATUS prvTgfCutThroughPortEnableSet
(
    IN GT_U32    portNum,
    IN GT_BOOL  enable,
    IN GT_BOOL  untaggedEnable
)
{

#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
    PRV_TGF_MEMBER_FORCE_INFO_STC *currMemPtr;
    GT_U32  ii;
    static GT_U32   forcedPorts = 0;/* did we forced ports for Cut Through ? */

    if(enable == GT_TRUE && forcedPorts == 0 && prvTgfDevPortForceNum)
    {
        /* look for this member in the DB */
        currMemPtr = &prvTgfDevPortForceArray[0];
        for(ii = 0 ; ii < prvTgfDevPortForceNum; ii++,currMemPtr++)
        {
            if(currMemPtr->forceToVlan == GT_FALSE)
            {
                /* member not need to be forced to any vlan */
                continue;
            }
            /* call device specific API */
            CPSS_TBD_BOOKMARK_EARCH /* casting of GT_U8 issue */
            rc = cpssDxChCutThroughPortEnableSet(currMemPtr->member.devNum,
                                                 (GT_U8)currMemPtr->member.portNum,
                                                 enable,
                                                 untaggedEnable);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCutThroughPortEnableSet FAILED, rc = [%d]", rc);

                return rc;
            }
        }

        forcedPorts = 1;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChCutThroughPortEnableSet(devNum, portNum, enable,
                                             untaggedEnable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCutThroughPortEnableSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(enable);
    TGF_PARAM_NOT_USED(untaggedEnable);

    return GT_NOT_IMPLEMENTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfCutThroughUpEnableSet function
* @endinternal
*
* @brief   Enable / Disable tagged packets with the specified UP
*         to be Cut Through.
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or up.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The ingress port should be enabled for Cut Through forwarding.
*       Use cpssDxChCutThroughPortEnableSet for it.
*
*/
GT_STATUS prvTgfCutThroughUpEnableSet
(
    IN GT_U8    up,
    IN GT_BOOL  enable
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
        rc = cpssDxChCutThroughUpEnableSet(devNum, up, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCutThroughUpEnableSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(up);
    TGF_PARAM_NOT_USED(enable);

    return GT_NOT_IMPLEMENTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfCutThroughVlanEthertypeSet function
* @endinternal
*
* @brief   Set VLAN Ethertype in order to identify tagged packets.
*         A packed is identified as VLAN tagged for cut-through purposes.
*         Packet considered as tagged if packet's Ethertype equals to one of two
*         configurable VLAN Ethertypes.
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_OUT_OF_RANGE          - on wrong etherType0 or etherType1
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCutThroughVlanEthertypeSet
(
    IN GT_U32   etherType0,
    IN GT_U32   etherType1
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
        if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            rc = cpssDxChPortPipGlobalEtherTypeProtocolSet(
                devNum, CPSS_DXCH_PORT_PIP_PROTOCOL_VLAN_TAG_E,
                0 /*index*/, (GT_U16)(etherType0 & 0xFFFF));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC(
                    "[TGF]: cpssDxChPortPipGlobalEtherTypeProtocolSet FAILED, rc = [%d]", rc);

                rc1 = rc;
            }
            rc = cpssDxChPortPipGlobalEtherTypeProtocolSet(
                devNum, CPSS_DXCH_PORT_PIP_PROTOCOL_VLAN_TAG_E,
                1 /*index*/, (GT_U16)(etherType1 & 0xFFFF));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC(
                    "[TGF]: cpssDxChPortPipGlobalEtherTypeProtocolSet FAILED, rc = [%d]", rc);

                rc1 = rc;
            }
        }
        else
        {
            rc = cpssDxChCutThroughVlanEthertypeSet(devNum, etherType0, etherType1);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCutThroughVlanEthertypeSet FAILED, rc = [%d]", rc);

                rc1 = rc;
            }
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(etherType0);
    TGF_PARAM_NOT_USED(etherType1);

    return GT_NOT_IMPLEMENTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfCutThroughBypassModeSet function
* @endinternal
*
* @brief   Set Bypass mode.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] bypassModePtr            - pointer to Bypass mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong Bypass Ingress PCL mode
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_IMPLEMENTED       - function not implemented for the family
* @retval GT_BAD_STATE             - no such functionality for the device
*/
GT_STATUS prvTgfCutThroughBypassModeSet
(
    IN GT_U8                             devNum,
    IN PRV_TGF_CUT_THROUGH_BYPASS_STC    *bypassModePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc;
    CPSS_DXCH_CUT_THROUGH_BYPASS_STC cpssBypassMode;

    cpssBypassMode.bypassMll = bypassModePtr->bypassMll;
    cpssBypassMode.bypassRouter = bypassModePtr->bypassRouter;
    cpssBypassMode.bypassIngressPolicerStage0 =
        bypassModePtr->bypassIngressPolicerStage0;
    cpssBypassMode.bypassIngressPolicerStage1 =
        bypassModePtr->bypassIngressPolicerStage1;
    cpssBypassMode.bypassEgressPcl = bypassModePtr->bypassEgressPcl;
    cpssBypassMode.bypassEgressPolicer = bypassModePtr->bypassEgressPolicer;
    cpssBypassMode.bypassEgressOam = bypassModePtr->bypassEgressOam;
    cpssBypassMode.bypassIngressOam = bypassModePtr->bypassIngressOam;

    switch(bypassModePtr->bypassIngressPcl)
    {
        case PRV_TGF_CUT_THROUGH_BYPASS_INGRESS_PCL_NO_BYPASS_E:
            cpssBypassMode.bypassIngressPcl =
                CPSS_DXCH_CUT_THROUGH_BYPASS_INGRESS_PCL_NO_BYPASS_E;
            break;
        case PRV_TGF_CUT_THROUGH_BYPASS_INGRESS_PCL_ONE_LOOKUP_E:
            cpssBypassMode.bypassIngressPcl =
                CPSS_DXCH_CUT_THROUGH_BYPASS_INGRESS_PCL_ONE_LOOKUP_E;
            break;
        case PRV_TGF_CUT_THROUGH_BYPASS_INGRESS_PCL_TWO_LOOKUP_E:
            cpssBypassMode.bypassIngressPcl =
                CPSS_DXCH_CUT_THROUGH_BYPASS_INGRESS_PCL_TWO_LOOKUP_E;
            break;
        case PRV_TGF_CUT_THROUGH_BYPASS_INGRESS_PCL_FULL_BYPASS_E:
            cpssBypassMode.bypassIngressPcl =
                CPSS_DXCH_CUT_THROUGH_BYPASS_INGRESS_PCL_FULL_BYPASS_E;
            break;
    }

    /* go over all active devices */
        /* call device specific API */
    rc = cpssDxChCutThroughBypassModeSet(devNum, &cpssBypassMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCutThroughBypassModeSet FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(bypassModePtr);
    return GT_NOT_IMPLEMENTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfCutThroughBypassModeGet function
* @endinternal
*
* @brief   Get Bypass mode.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
*
* @param[out] bypassModePtr            - pointer to Bypass mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_IMPLEMENTED       - function not implemented for the family
* @retval GT_BAD_STATE             - no such functionality for the device
*/
GT_STATUS prvTgfCutThroughBypassModeGet
(
    IN  GT_U8                             devNum,
    OUT PRV_TGF_CUT_THROUGH_BYPASS_STC    *bypassModePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc;
    CPSS_DXCH_CUT_THROUGH_BYPASS_STC cpssBypassMode;

    if(NULL == bypassModePtr)
    {
        PRV_UTF_LOG0_MAC(
            "[TGF]: cpssDxChCutThroughBypassModeGet FAILED: Bad pointer!"
        );
        return GT_BAD_PTR;
    }

    /* call device specific API */
    rc = cpssDxChCutThroughBypassModeGet(devNum, &cpssBypassMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC(
            "[TGF]: cpssDxChCutThroughBypassModeGet FAILED, rc = [%d]", rc
        );
        return rc;
    }

    bypassModePtr->bypassMll = cpssBypassMode.bypassMll;
    bypassModePtr->bypassRouter = cpssBypassMode.bypassRouter;
    bypassModePtr->bypassIngressPolicerStage0 =
        cpssBypassMode.bypassIngressPolicerStage0;
    bypassModePtr->bypassIngressPolicerStage1 =
        cpssBypassMode.bypassIngressPolicerStage1;
    bypassModePtr->bypassEgressPcl = cpssBypassMode.bypassEgressPcl;
    bypassModePtr->bypassEgressPolicer = cpssBypassMode.bypassEgressPolicer;
    bypassModePtr->bypassEgressOam = cpssBypassMode.bypassEgressOam;
    bypassModePtr->bypassIngressOam = cpssBypassMode.bypassIngressOam;

    switch(cpssBypassMode.bypassIngressPcl)
    {
        case CPSS_DXCH_CUT_THROUGH_BYPASS_INGRESS_PCL_NO_BYPASS_E:
            bypassModePtr->bypassIngressPcl =
                PRV_TGF_CUT_THROUGH_BYPASS_INGRESS_PCL_NO_BYPASS_E;
            break;
        case CPSS_DXCH_CUT_THROUGH_BYPASS_INGRESS_PCL_ONE_LOOKUP_E:
            bypassModePtr->bypassIngressPcl =
                PRV_TGF_CUT_THROUGH_BYPASS_INGRESS_PCL_ONE_LOOKUP_E;
            break;
        case CPSS_DXCH_CUT_THROUGH_BYPASS_INGRESS_PCL_TWO_LOOKUP_E:
            bypassModePtr->bypassIngressPcl =
                PRV_TGF_CUT_THROUGH_BYPASS_INGRESS_PCL_TWO_LOOKUP_E;
            break;
        case CPSS_DXCH_CUT_THROUGH_BYPASS_INGRESS_PCL_FULL_BYPASS_E:
            bypassModePtr->bypassIngressPcl =
                PRV_TGF_CUT_THROUGH_BYPASS_INGRESS_PCL_FULL_BYPASS_E;
            break;
    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(bypassModePtr);
    return GT_NOT_IMPLEMENTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfCutThroughEarlyProcessingModeSet function
* @endinternal
*
* @brief   Set Early Processing mode of Cut Through packets.
*          Set Reduced Latency Store and Forward mode or Pure Cut Through mode.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in] mode                  - early processing mode of cut through packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvTgfCutThroughEarlyProcessingModeSet
(
    IN PRV_TGF_EARLY_PROCESSING_MODE_ENT        mode
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
    CPSS_DXCH_EARLY_PROCESSING_MODE_ENT dxChMode;

    switch (mode)
    {
        case PRV_TGF_EARLY_PROCESSING_CUT_THROUGH_E:
            dxChMode = CPSS_DXCH_EARLY_PROCESSING_CUT_THROUGH_E;
            break;
        case PRV_TGF_EARLY_PROCESSING_STORE_AND_FORWARD_REDUCED_LATENCY_E:
            dxChMode = CPSS_DXCH_EARLY_PROCESSING_STORE_AND_FORWARD_REDUCED_LATENCY_E;
            break;
        default:
            PRV_UTF_LOG1_MAC(
                "[TGF]:prvTgfCutThroughEarlyProcessingModeSet  wrong mode [%d]", mode);
            return GT_BAD_PARAM;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChCutThroughEarlyProcessingModeSet(
            devNum, dxChMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC(
                "[TGF]: cpssDxChCutThroughEarlyProcessingModeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#else /* not CHX_FAMILY */
    /* avoid warnings */
    TGF_PARAM_NOT_USED(dxChMode);
    return GT_NOT_IMPLEMENTED;
#endif /* CHX_FAMILY */
}

/**
* @internal prvTgfDxChCutThroughUdeCfgSet function
* @endinternal
*
* @brief   Set Cut Through configuration of User Defined Ethernet packets.
*          Related to 4 Generic ethernet types configured by
*          cpssDxChPortPipGlobalEtherTypeProtocolSet.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in] udeIndex                    - UDE index (APPLICABLE RANGES: 0..3)
* @param[in] udeEthertype                - Ethertype of UDE packets.
* @param[in] udeCutThroughEnable         - GT_TRUE/GT_FALSE enable/disable Cut Through mode for UDE packets.
* @param[in] udeByteCount                - default CT Byte Count for UDE packets.
*                                          (APPLICABLE RANGES: 0..0x3FFF)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_OUT_OF_RANGE          - on out of range udeByteCount
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvTgfDxChCutThroughUdeCfgSet
(
    IN GT_U32      udeIndex,
    IN GT_U16      udeEthertype,
    IN GT_BOOL     udeCutThroughEnable,
    IN GT_U32      udeByteCount
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
        rc = cpssDxChPortPipGlobalEtherTypeProtocolSet(
            devNum, CPSS_DXCH_PORT_PIP_PROTOCOL_UDE_E, udeIndex, udeEthertype);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC(
                "[TGF]: cpssDxChPortPipGlobalEtherTypeProtocolSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
        /* call device specific API */
        rc = cpssDxChCutThroughUdeCfgSet(
            devNum, udeIndex, udeCutThroughEnable, udeByteCount);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCutThroughUpEnableSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#else /* not CHX_FAMILY */
    /* avoid warnings */
    TGF_PARAM_NOT_USED(udeIndex);
    TGF_PARAM_NOT_USED(udeEthertype);
    TGF_PARAM_NOT_USED(udeCutThroughEnable);
    TGF_PARAM_NOT_USED(udeByteCount);
    return GT_NOT_IMPLEMENTED;
#endif /* CHX_FAMILY */
}

/**
* @internal prvTgfDxChCutThroughByteCountExtractFailsCounterGet function
* @endinternal
*
* @brief   Get counter of fails extracting CT Packet Byte Count by packet header.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[out] countPtr  - pointer to count of packet Byte Count extracting fails.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfDxChCutThroughByteCountExtractFailsCounterGet
(
    OUT GT_U32      *countPtr
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
    GT_U32      counter;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    *countPtr = 0; /* initialize sum accomulator */

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChCutThroughByteCountExtractFailsCounterGet(
            devNum, &counter);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC(
                "[TGF]: cpssDxChCutThroughByteCountExtractFailsCounterGet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
        *countPtr += counter;
    }

    return rc1;
#else /* not CHX_FAMILY */
    /* avoid warnings */
    TGF_PARAM_NOT_USED(countPtr);
    return GT_NOT_IMPLEMENTED;
#endif /* CHX_FAMILY */
}

/**
* @internal prvTgfCutThroughPacketHdrIntegrityCheckCfgSet function
* @endinternal
*
* @brief   Set configuration Packet Header Integrity Check.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in] cfgPtr  - pointer to Packet Header Integrity Check.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvTgfCutThroughPacketHdrIntegrityCheckCfgSet
(
    IN PRV_TGF_CUT_THROUGH_PACKET_HDR_INTEGRITY_CHECK_CFG_STC   *cfgPtr
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
    CPSS_DXCH_CUT_THROUGH_PACKET_HDR_INTEGRITY_CHECK_CFG_STC  dxChCfg;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    dxChCfg.minByteCountBoundary           = cfgPtr->minByteCountBoundary;
    dxChCfg.maxByteCountBoundary           = cfgPtr->maxByteCountBoundary;
    dxChCfg.enableByteCountBoundariesCheck = cfgPtr->enableByteCountBoundariesCheck;
    dxChCfg.enableIpv4HdrCheckByChecksum   = cfgPtr->enableIpv4HdrCheckByChecksum;
    dxChCfg.hdrIntergrityExceptionPktCmd   = cfgPtr->hdrIntergrityExceptionPktCmd;
    dxChCfg.hdrIntergrityExceptionCpuCode  = cfgPtr->hdrIntergrityExceptionCpuCode;

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChCutThroughPacketHdrIntegrityCheckCfgSet(
            devNum, &dxChCfg);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC(
                "[TGF]: cpssDxChCutThroughPacketHdrIntegrityCheckCfgSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#else /* not CHX_FAMILY */
    /* avoid warnings */
    TGF_PARAM_NOT_USED(cfgPtr);
    return GT_NOT_IMPLEMENTED;
#endif /* CHX_FAMILY */
}

/**
* @internal prvTgfCutThroughPortByteCountUpdateSet function
* @endinternal
*
* @brief   Set configuration for updating byte count per port.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in] portNum                   - physical port number including CPU port.
* @param[in] toSubtractOrToAdd         - GT_TRUE - to subtract, GT_FALSE to add.
* @param[in] subtractedOrAddedValue    - value to subtract or to add to packet length
*                                        (APPLICABLE RANGES: 0..7)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or portNum
* @retval GT_OUT_OF_RANGE          - on out of range subtractedOrAddedValue
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvTgfCutThroughPortByteCountUpdateSet
(
    IN GT_PHYSICAL_PORT_NUM  portNum,
    IN GT_BOOL               toSubtractOrToAdd,
    IN GT_U32                subtractedOrAddedValue
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
        rc = cpssDxChCutThroughPortByteCountUpdateSet(
            devNum, portNum, toSubtractOrToAdd, subtractedOrAddedValue);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC(
                "[TGF]: cpssDxChCutThroughPortByteCountUpdateSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#else /* not CHX_FAMILY */
    /* avoid warnings */
    TGF_PARAM_NOT_USED(cfgPtr);
    return GT_NOT_IMPLEMENTED;
#endif /* CHX_FAMILY */
}

/**
* @internal prvTgfDxChCutThroughRxErrorConfigSet function
* @endinternal
*
* @brief   Set Error Configuration of RX when obtained actual packet size different from calculated.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in]  paddedPacketEofWithError    - When a CT packet is padded it‘s last
*                                           write transaction to the PB should be marked
*                                           GT_FALSE - as EOP or GT_TRUE - as EOP with Error,
* @param[in]  truncatedPacketEofWithError - When a CT packet is truncated it‘s last
*                                           write transaction to the PB should be marked
*                                           GT_FALSE - as EOP or GT_TRUE - as EOP with Error,
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or portNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvTgfDxChCutThroughRxErrorConfigSet
(
    IN  GT_BOOL               paddedPacketEofWithError,
    IN  GT_BOOL               truncatedPacketEofWithError
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
        rc = prvCpssDxChCutThroughRxErrorConfigSet(
            devNum, paddedPacketEofWithError, truncatedPacketEofWithError);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC(
                "[TGF]: prvCpssDxChCutThroughRxErrorConfigSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#else /* not CHX_FAMILY */
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(paddedPacketEofWithError);
    TGF_PARAM_NOT_USED(truncatedPacketEofWithError);
    return GT_NOT_IMPLEMENTED;
#endif /* CHX_FAMILY */
}


