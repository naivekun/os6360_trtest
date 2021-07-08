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
* @file tgfDitGen.c
*
* @brief Generic API implementation for DIT
*
* @version   2
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/private/prvUtfHelpers.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficEngine.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfDitGen.h>

#ifdef EXMXPM_FAMILY
    #include <cpss/exMxPm/exMxPmGen/config/cpssExMxPmCfg.h>
    #include <cpss/exMxPm/exMxPmGen/config/private/prvCpssExMxPmInfo.h>
    #include <cpss/exMxPm/exMxPmGen/dit/cpssExMxPmDit.h>
#endif /*EXMXPM_FAMILY*/


/******************************************************************************\
 *                           CPSS generic API section                          *
\******************************************************************************/

/**
* @internal prvTgfDitFcoeEnableSet function
* @endinternal
*
* @brief   Enable/Disable FCoE (Fiber Channel over Ethernet)
*
* @param[in] devNum                   - physical device number
* @param[in] enable                   - If GT_TRUE   FCoE
*                                      If GT_FALSE disable FCoE
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on illegal parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfDitFcoeEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
{
#ifdef EXMXPM_FAMILY
    GT_STATUS   rc;

    /* call device specific API */
    rc = cpssExMxPmDitFcoeEnableSet(devNum, enable);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmDitFcoeEnableSet FAILED, rc = [%d]", rc);
    }
    return rc;
#else /* !EXMXPM_FAMILY */

    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enable);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

}

/**
* @internal prvTgfDitFcoeEnableGet function
* @endinternal
*
* @brief   Gets whether FCoE (Fiber Channel over Ethernet) is enabled/disabled.
*
* @param[in] devNum                   - physical device number
*
* @param[out] enabledPtr               - If GT_TRUE  then FCoE is enabled.
*                                      If GT_FALSE then FCoE is disabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on illegal parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfDitFcoeEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enabledPtr
)
{
#ifdef EXMXPM_FAMILY
    GT_STATUS           rc = GT_OK;
    GT_BOOL             enable;

    /* call device specific API */
    /* call device specific API */
    rc = cpssExMxPmDitFcoeEnableGet(devNum, &enable);
    if (rc != GT_OK)
        return rc;

    *enabledPtr=enable;

    return rc;

#else /* !EXMXPM_FAMILY */

    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enabledPtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */
}


/**
* @internal prvTgfDitFcoeEtherTypeSet function
* @endinternal
*
* @brief   Set FCoE (Fiber Channel over Ethernet) Ether Type value.
*
* @param[in] devNum                   - physical device number
* @param[in] etherType                - Ether Type Value
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on illegal parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfDitFcoeEtherTypeSet
(
    IN  GT_U8       devNum,
    IN  GT_U16      etherType
)
{
#ifdef EXMXPM_FAMILY
    GT_STATUS   rc;

    /* call device specific API */
    rc = cpssExMxPmDitFcoeEtherTypeSet(devNum, etherType);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmDitFcoeEtherTypeSet FAILED, rc = [%d]", rc);
    }
    return rc;
#else /* !EXMXPM_FAMILY */

    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(etherType);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

}

/**
* @internal prvTgfDitFcoeEtherTypeGet function
* @endinternal
*
* @brief   Gets FCoE (Fiber Channel over Ethernet) Ether Type
*
* @note   APPLICABLE DEVICES:      Puma2; Puma3.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*
* @param[out] etherTypePtr             - Ether Type Value
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on illegal parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfDitFcoeEtherTypeGet
(
    IN  GT_U8       devNum,
    OUT GT_U16      *etherTypePtr
)
{
#ifdef EXMXPM_FAMILY
    GT_STATUS           rc = GT_OK;
    GT_U16              etherType;

    /* call device specific API */
    rc = cpssExMxPmDitFcoeEtherTypeGet(devNum, &etherType);
    if (rc != GT_OK)
        return rc;

    *etherTypePtr=etherType;

    return rc;

#else /* !EXMXPM_FAMILY */

    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(etherTypePtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */
}

#ifdef EXMXPM_FAMILY
/**
* @internal prvTgfConvertGenericToExMxPmDitVplsEntry function
* @endinternal
*
* @brief   Convert generic DIT VPLS entry into device specific DIT VPLS entry
*
* @param[in] ditEntryPtr              - (pointer to)DIT VPLS  entry
*
* @param[out] exMxPmDitEntryPtr        - (pointer to) ExMxPm DIT VPLS route entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToExMxPmDitVplsEntry
(
    IN  PRV_TGF_DIT_VPLS_STC                     *ditEntryPtr,
    OUT CPSS_EXMXPM_DIT_VPLS_STC                 *exMxPmDitEntryPtr
)
{
    /* convert DIT VPLS entry into device specific format */

    /* set interface info */
    cpssOsMemCpy((GT_VOID*) &(exMxPmDitEntryPtr->outLif),
                 (GT_VOID*) &(ditEntryPtr->outLif),
                 sizeof(ditEntryPtr->outLif));

    /* convert tunnelStartPassengerPacketType into device specific format */
    switch (ditEntryPtr->tunnelStartPassengerPacketType)
    {
        case PRV_TGF_TS_PASSENGER_PACKET_TYPE_ETHERNET_E:
            exMxPmDitEntryPtr->tunnelStartPassengerPacketType=CPSS_EXMXPM_TS_PASSENGER_PACKET_TYPE_ETHERNET_E;
            break;
        case PRV_TGF_TS_PASSENGER_PACKET_TYPE_OTHER_E:
            exMxPmDitEntryPtr->tunnelStartPassengerPacketType=CPSS_EXMXPM_TS_PASSENGER_PACKET_TYPE_OTHER_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    /* convert vlanTagCommand into device specific format */
    switch (ditEntryPtr->vlanTagCommand)
    {
        case PRV_TGF_DIT_VLAN_TAG_COMMAND_TAGGED_FROM_DIT_E:
            exMxPmDitEntryPtr->vlanTagCommand=CPSS_EXMXPM_DIT_VLAN_TAG_COMMAND_TAGGED_FROM_DIT_E;
            break;
        case PRV_TGF_DIT_VLAN_TAG_COMMAND_TAGGED_DONT_MODIFY_E:
            exMxPmDitEntryPtr->vlanTagCommand=CPSS_EXMXPM_DIT_VLAN_TAG_COMMAND_TAGGED_DONT_MODIFY_E;
            break;
        case PRV_TGF_DIT_VLAN_TAG_COMMAND_UNTAGGED_E:
            exMxPmDitEntryPtr->vlanTagCommand=CPSS_EXMXPM_DIT_VLAN_TAG_COMMAND_UNTAGGED_E;
            break;
        case PRV_TGF_DIT_VLAN_TAG_COMMAND_DONT_MODIFY_E:
            exMxPmDitEntryPtr->vlanTagCommand=CPSS_EXMXPM_DIT_VLAN_TAG_COMMAND_DONT_MODIFY_E;
            break;
        case PRV_TGF_DIT_VLAN_TAG_COMMAND_KEEP_TAG_BY_VLT_E:
            exMxPmDitEntryPtr->vlanTagCommand=CPSS_EXMXPM_DIT_VLAN_TAG_COMMAND_KEEP_TAG_BY_VLT_E;
            break;
        case PRV_TGF_DIT_VLAN_TAG_COMMAND_OVERRIDE_TAG_FROM_VLT_E:
            exMxPmDitEntryPtr->vlanTagCommand=CPSS_EXMXPM_DIT_VLAN_TAG_COMMAND_OVERRIDE_TAG_FROM_VLT_E;
            break;
        case PRV_TGF_DIT_VLAN_TAG_COMMAND_KEEP_ADD_EXTERNAL_TAG_E:
            exMxPmDitEntryPtr->vlanTagCommand=CPSS_EXMXPM_DIT_VLAN_TAG_COMMAND_KEEP_ADD_EXTERNAL_TAG_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    /* convert qosExpCommand into device specific format */
    switch (ditEntryPtr->qosExpCommand)
    {
        case PRV_TGF_DIT_QOS_COMMAND_FROM_DESCRIPTOR_E:
            exMxPmDitEntryPtr->qosExpCommand=CPSS_EXMXPM_DIT_QOS_COMMAND_FROM_DESCRIPTOR_E;
            break;
        case PRV_TGF_DIT_QOS_COMMAND_FROM_ENTRY_E:
            exMxPmDitEntryPtr->qosExpCommand=CPSS_EXMXPM_DIT_QOS_COMMAND_FROM_ENTRY_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    /* convert qosUpCommand into device specific format */
    switch (ditEntryPtr->qosUpCommand)
    {
        case PRV_TGF_DIT_QOS_COMMAND_FROM_DESCRIPTOR_E:
            exMxPmDitEntryPtr->qosUpCommand=CPSS_EXMXPM_DIT_QOS_COMMAND_FROM_DESCRIPTOR_E;
            break;
        case PRV_TGF_DIT_QOS_COMMAND_FROM_ENTRY_E:
            exMxPmDitEntryPtr->qosUpCommand=CPSS_EXMXPM_DIT_QOS_COMMAND_FROM_ENTRY_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmDitEntryPtr, ditEntryPtr, isTunnelStart);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmDitEntryPtr, ditEntryPtr, tunnelStartPtr);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmDitEntryPtr, ditEntryPtr, arpPointer);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmDitEntryPtr, ditEntryPtr, vlanId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmDitEntryPtr, ditEntryPtr, ttl);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmDitEntryPtr, ditEntryPtr, label);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmDitEntryPtr, ditEntryPtr, exp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmDitEntryPtr, ditEntryPtr, up);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmDitEntryPtr, ditEntryPtr, mplsCommand);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmDitEntryPtr, ditEntryPtr, isProviderEdge);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmDitEntryPtr, ditEntryPtr, vplsUnregisteredBcEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmDitEntryPtr, ditEntryPtr, vplsUnknownUcEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmDitEntryPtr, ditEntryPtr, vplsUnregisteredMcEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmDitEntryPtr, ditEntryPtr, pwId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmDitEntryPtr, ditEntryPtr, nextPointer);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmDitEntryPtr, ditEntryPtr, isLast);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmDitEntryPtr, ditEntryPtr, vllEgressFilteringEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmDitEntryPtr, ditEntryPtr, bypassDitTargetEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmDitEntryPtr, ditEntryPtr, fcoeMacDaPseudoRoutingEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmDitEntryPtr, ditEntryPtr, fcoeMacSaPseudoRoutingEnable);

    return GT_OK;
}

/**
* @internal prvTgfConvertExMxPmToGenericDitVplsEntry function
* @endinternal
*
* @brief   Convert device specific DIT VPLS entry into generic DIT VPLS entry
*
* @param[in] exMxPmDitEntryPtr        - (pointer to) ExMxPm DIT VPLS route entry
*
* @param[out] ditEntryPtr              - (pointer to)DIT VPLS  entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertExMxPmToGenericDitVplsEntry
(
    IN  CPSS_EXMXPM_DIT_VPLS_STC                 *exMxPmDitEntryPtr,
    OUT PRV_TGF_DIT_VPLS_STC                     *ditEntryPtr
)
{
    /* convert DIT VPLS entry into generic format */

    /* set interface info */
    cpssOsMemCpy((GT_VOID*) &(ditEntryPtr->outLif),
                 (GT_VOID*) &(exMxPmDitEntryPtr->outLif),
                 sizeof(exMxPmDitEntryPtr->outLif));

    /* convert tunnelStartPassengerPacketType into device specific format */
    switch (exMxPmDitEntryPtr->tunnelStartPassengerPacketType)
    {
        case CPSS_EXMXPM_TS_PASSENGER_PACKET_TYPE_ETHERNET_E:
            ditEntryPtr->tunnelStartPassengerPacketType=PRV_TGF_TS_PASSENGER_PACKET_TYPE_ETHERNET_E;
            break;
        case CPSS_EXMXPM_TS_PASSENGER_PACKET_TYPE_OTHER_E:
            ditEntryPtr->tunnelStartPassengerPacketType=PRV_TGF_TS_PASSENGER_PACKET_TYPE_OTHER_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    /* convert vlanTagCommand into device specific format */
    switch (exMxPmDitEntryPtr->vlanTagCommand)
    {
        case CPSS_EXMXPM_DIT_VLAN_TAG_COMMAND_TAGGED_FROM_DIT_E:
            ditEntryPtr->vlanTagCommand=PRV_TGF_DIT_VLAN_TAG_COMMAND_TAGGED_FROM_DIT_E;
            break;
        case CPSS_EXMXPM_DIT_VLAN_TAG_COMMAND_TAGGED_DONT_MODIFY_E:
            ditEntryPtr->vlanTagCommand=PRV_TGF_DIT_VLAN_TAG_COMMAND_TAGGED_DONT_MODIFY_E;
            break;
        case CPSS_EXMXPM_DIT_VLAN_TAG_COMMAND_UNTAGGED_E:
            ditEntryPtr->vlanTagCommand=PRV_TGF_DIT_VLAN_TAG_COMMAND_UNTAGGED_E;
            break;
        case CPSS_EXMXPM_DIT_VLAN_TAG_COMMAND_DONT_MODIFY_E:
            ditEntryPtr->vlanTagCommand=PRV_TGF_DIT_VLAN_TAG_COMMAND_DONT_MODIFY_E;
            break;
        case CPSS_EXMXPM_DIT_VLAN_TAG_COMMAND_KEEP_TAG_BY_VLT_E:
            ditEntryPtr->vlanTagCommand=PRV_TGF_DIT_VLAN_TAG_COMMAND_KEEP_TAG_BY_VLT_E;
            break;
        case CPSS_EXMXPM_DIT_VLAN_TAG_COMMAND_OVERRIDE_TAG_FROM_VLT_E:
            ditEntryPtr->vlanTagCommand=PRV_TGF_DIT_VLAN_TAG_COMMAND_OVERRIDE_TAG_FROM_VLT_E;
            break;
        case CPSS_EXMXPM_DIT_VLAN_TAG_COMMAND_KEEP_ADD_EXTERNAL_TAG_E:
            ditEntryPtr->vlanTagCommand=PRV_TGF_DIT_VLAN_TAG_COMMAND_KEEP_ADD_EXTERNAL_TAG_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    /* convert qosExpCommand into device specific format */
    switch (exMxPmDitEntryPtr->qosExpCommand)
    {
        case CPSS_EXMXPM_DIT_QOS_COMMAND_FROM_DESCRIPTOR_E:
            ditEntryPtr->qosExpCommand=PRV_TGF_DIT_QOS_COMMAND_FROM_DESCRIPTOR_E;
            break;
        case CPSS_EXMXPM_DIT_QOS_COMMAND_FROM_ENTRY_E:
            ditEntryPtr->qosExpCommand=PRV_TGF_DIT_QOS_COMMAND_FROM_ENTRY_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    /* convert qosUpCommand into device specific format */
    switch (exMxPmDitEntryPtr->qosUpCommand)
    {
        case CPSS_EXMXPM_DIT_QOS_COMMAND_FROM_DESCRIPTOR_E:
            ditEntryPtr->qosUpCommand=PRV_TGF_DIT_QOS_COMMAND_FROM_DESCRIPTOR_E;
            break;
        case CPSS_EXMXPM_DIT_QOS_COMMAND_FROM_ENTRY_E:
            ditEntryPtr->qosUpCommand=PRV_TGF_DIT_QOS_COMMAND_FROM_ENTRY_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmDitEntryPtr, ditEntryPtr, isTunnelStart);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmDitEntryPtr, ditEntryPtr, tunnelStartPtr);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmDitEntryPtr, ditEntryPtr, arpPointer);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmDitEntryPtr, ditEntryPtr, vlanId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmDitEntryPtr, ditEntryPtr, ttl);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmDitEntryPtr, ditEntryPtr, label);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmDitEntryPtr, ditEntryPtr, exp);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmDitEntryPtr, ditEntryPtr, up);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmDitEntryPtr, ditEntryPtr, mplsCommand);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmDitEntryPtr, ditEntryPtr, isProviderEdge);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmDitEntryPtr, ditEntryPtr, vplsUnregisteredBcEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmDitEntryPtr, ditEntryPtr, vplsUnknownUcEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmDitEntryPtr, ditEntryPtr, vplsUnregisteredMcEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmDitEntryPtr, ditEntryPtr, pwId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmDitEntryPtr, ditEntryPtr, nextPointer);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmDitEntryPtr, ditEntryPtr, isLast);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmDitEntryPtr, ditEntryPtr, vllEgressFilteringEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmDitEntryPtr, ditEntryPtr, bypassDitTargetEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmDitEntryPtr, ditEntryPtr, fcoeMacDaPseudoRoutingEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmDitEntryPtr, ditEntryPtr, fcoeMacSaPseudoRoutingEnable);

    return GT_OK;
}
#endif /* EXMXPM_FAMILY */


/**
* @internal prvTgfDitVpwsTargetTaggedModeSet function
* @endinternal
*
* @brief   Controls the default target tagged mode for VPWS packets.
*
* @param[in] devNum                   - device number
* @param[in] vpwsTargetTaggedMode     - VPWS target tagged mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on illegal parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfDitVpwsTargetTaggedModeSet
(
    IN  GT_U8                                       devNum,
    IN  PRV_TGF_DIT_VPWS_TARGET_TAGGED_MODE_ENT     vpwsTargetTaggedMode
)
{
#ifdef EXMXPM_FAMILY
    GT_STATUS   rc;
    CPSS_EXMXPM_DIT_VPWS_TARGET_TAGGED_MODE_ENT cpssVpwsTargetTaggedMode;

    switch(vpwsTargetTaggedMode)
    {
        case PRV_TGF_DIT_VPWS_TARGET_TAGGED_VLAN_TABLE_MODE_E:
            cpssVpwsTargetTaggedMode = CPSS_EXMXPM_DIT_VPWS_TARGET_TAGGED_VLAN_TABLE_MODE_E;
            break;
        case PRV_TGF_DIT_VPWS_TARGET_TAGGED_CONTROL_MODE_E:
            cpssVpwsTargetTaggedMode = CPSS_EXMXPM_DIT_VPWS_TARGET_TAGGED_CONTROL_MODE_E;
            break;
        default:
            return GT_BAD_PARAM;
    }
    /* call device specific API */
    rc = cpssExMxPmDitVpwsTargetTaggedModeSet(devNum, cpssVpwsTargetTaggedMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmDitVpwsTargetTaggedModeSet FAILED, rc = [%d]", rc);
    }
    return rc;
#else /* !EXMXPM_FAMILY */

    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(vpwsTargetTaggedMode);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

}

/**
* @internal prvTgfDitVpwsTargetTaggedModeGet function
* @endinternal
*
* @brief   Get the default target tagged mode for VPWS packets.
*
* @param[in] devNum                   - device number
*
* @param[out] vpwsTargetTaggedModePtr  - points to VPWS target tagged mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on illegal parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfDitVpwsTargetTaggedModeGet
(
    IN  GT_U8                                       devNum,
    OUT PRV_TGF_DIT_VPWS_TARGET_TAGGED_MODE_ENT     *vpwsTargetTaggedModePtr
)
{
#ifdef EXMXPM_FAMILY
    GT_STATUS                                   rc = GT_OK;
    CPSS_EXMXPM_DIT_VPWS_TARGET_TAGGED_MODE_ENT cpssVpwsTargetTaggedMode;

    /* call device specific API */
    rc = cpssExMxPmDitVpwsTargetTaggedModeGet(devNum, &cpssVpwsTargetTaggedMode);
    if (rc != GT_OK)
        return rc;

    switch(cpssVpwsTargetTaggedMode)
    {
        case PRV_TGF_DIT_VPWS_TARGET_TAGGED_VLAN_TABLE_MODE_E:
            *vpwsTargetTaggedModePtr = CPSS_EXMXPM_DIT_VPWS_TARGET_TAGGED_VLAN_TABLE_MODE_E;
            break;
        case PRV_TGF_DIT_VPWS_TARGET_TAGGED_CONTROL_MODE_E:
            *vpwsTargetTaggedModePtr = CPSS_EXMXPM_DIT_VPWS_TARGET_TAGGED_CONTROL_MODE_E;
            break;
        default:
            return GT_BAD_VALUE;
    }
    return rc;

#else /* !EXMXPM_FAMILY */

    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(vpwsTargetTaggedModePtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */
}

/**
* @internal prvTgfDitVplsEntryWrite function
* @endinternal
*
* @brief   Writes a Downstream interface Table (DIT) Vpls entry to hw.
*
* @param[in] devNum                   - device number
* @param[in] ditEntryIndex            - The DIT Vpls entry index.
* @param[in] ditType                  - dit packet type selection (unicast/multicast)
* @param[in] ditEntryPtr              - The downstream interface entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on illegal parameter
* @retval GT_BAD_PTR               - on one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfDitVplsEntryWrite
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        ditEntryIndex,
    IN  CPSS_UNICAST_MULTICAST_ENT    ditType,
    IN  PRV_TGF_DIT_VPLS_STC          *ditEntryPtr
)
{
#ifdef EXMXPM_FAMILY
    GT_STATUS   rc;
    CPSS_EXMXPM_DIT_VPLS_STC  exMxPmDitEntry;;

    cpssOsMemSet(&exMxPmDitEntry, 0, sizeof(exMxPmDitEntry));

    rc = prvTgfConvertGenericToExMxPmDitVplsEntry(ditEntryPtr,&exMxPmDitEntry);

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToExMxPmDitVplsEntry FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    rc = cpssExMxPmDitVplsEntryWrite(devNum, ditEntryIndex, ditType, &exMxPmDitEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmDitVplsEntryWrite FAILED, rc = [%d]", rc);
    }
    return rc;
#else /* !EXMXPM_FAMILY */

    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(ditEntryIndex);
    TGF_PARAM_NOT_USED(ditType);
    TGF_PARAM_NOT_USED(ditEntryPtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

}


/**
* @internal prvTgfDitVplsEntryRead function
* @endinternal
*
* @brief   Reads a Downstream interface Table (DIT) Vpls entry from the hw.
*
* @param[in] devNum                   - device number
* @param[in] ditEntryIndex            - The DIT Vpls entry index.
* @param[in] ditType                  - dit packet type selection (unicast/multicast)
*
* @param[out] ditEntryPtr              - The downstream interface entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on illegal parameter
* @retval GT_BAD_PTR               - on one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_UNFIXABLE_ECC_ERROR   - the CPSS detected ECC error that can't
*                                       be fixed when read from the memory that
*                                       protected by ECC generated.
*                                       if entry can't be fixed due to 2 data errors
*                                       NOTE: relevant only to memory controller that
*                                       work with ECC protection , and the CPSS need
*                                       to emulate ECC protection.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfDitVplsEntryRead
(
    IN  GT_U8                        devNum,
    IN  GT_U32                       ditEntryIndex,
    IN  CPSS_UNICAST_MULTICAST_ENT   ditType,
    OUT PRV_TGF_DIT_VPLS_STC         *ditEntryPtr
)
{
#ifdef EXMXPM_FAMILY
    GT_STATUS                   rc = GT_OK;
    CPSS_EXMXPM_DIT_VPLS_STC    exMxPmDitEntry;

    cpssOsMemSet(&exMxPmDitEntry, 0, sizeof(exMxPmDitEntry));

    /* call device specific API */
    rc = cpssExMxPmDitVplsEntryRead(devNum, ditEntryIndex, ditType, &exMxPmDitEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmDitVplsEntryRead FAILED, rc = [%d]", rc);
        return rc;
    }

    rc = prvTgfConvertExMxPmToGenericDitVplsEntry(&exMxPmDitEntry,ditEntryPtr);

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertExMxPmToGenericDitVplsEntry FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;

#else /* !EXMXPM_FAMILY */

    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(ditEntryIndex);
    TGF_PARAM_NOT_USED(ditType);
    TGF_PARAM_NOT_USED(ditEntryPtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */
}

