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
* @file tgfLogicalTargetGen.c
*
* @brief Generic API implementation for logical target mapping
*
* @version   8
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
#include <common/tgfLogicalTargetGen.h>

#ifdef CHX_FAMILY
/**
* @internal prvTgfConvertGenericToDxChLogicalTargetEntry function
* @endinternal
*
* @brief   Convert generic to device specific logical target entry
*
* @param[in] genTargetLogicalEntryPtr - (pointer to) generic logical target entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChLogicalTargetEntry
(
    IN  PRV_TGF_LOGICAL_TARGET_MAPPING_STC    *genTargetLogicalEntryPtr,
    OUT CPSS_DXCH_LOGICAL_TARGET_MAPPING_STC  *dxChTargetLogicalEntryPtr
)
{
    GT_STATUS rc;

    cpssOsMemSet(dxChTargetLogicalEntryPtr, 0, sizeof(CPSS_DXCH_LOGICAL_TARGET_MAPPING_STC));

    (genTargetLogicalEntryPtr->outputInterface.isTunnelStart == GT_TRUE) ?
        (dxChTargetLogicalEntryPtr->outputInterface.isTunnelStart = GT_TRUE) :
        (dxChTargetLogicalEntryPtr->outputInterface.isTunnelStart = GT_FALSE);

    if (dxChTargetLogicalEntryPtr->outputInterface.isTunnelStart == GT_TRUE)
    {
        switch(genTargetLogicalEntryPtr->outputInterface.tunnelStartInfo.passengerPacketType)
        {
        case PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E:
            dxChTargetLogicalEntryPtr->outputInterface.tunnelStartInfo.passengerPacketType =
                CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E;
            break;
        case PRV_TGF_TUNNEL_PASSENGER_OTHER_E:
            dxChTargetLogicalEntryPtr->outputInterface.tunnelStartInfo.passengerPacketType =
                CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E;
            break;
        default:
            return GT_BAD_PARAM;
        }
        dxChTargetLogicalEntryPtr->outputInterface.tunnelStartInfo.ptr =
            genTargetLogicalEntryPtr->outputInterface.tunnelStartInfo.ptr;
    }
    switch(genTargetLogicalEntryPtr->outputInterface.physicalInterface.type)
    {
    case CPSS_INTERFACE_PORT_E:
        dxChTargetLogicalEntryPtr->outputInterface.physicalInterface.type = CPSS_INTERFACE_PORT_E;
        rc = prvUtfHwDeviceNumberGet(genTargetLogicalEntryPtr->outputInterface.physicalInterface.devPort.hwDevNum,
                                           &(dxChTargetLogicalEntryPtr->outputInterface.physicalInterface.devPort.hwDevNum));
        if(GT_OK != rc)
            return rc;
        dxChTargetLogicalEntryPtr->outputInterface.physicalInterface.devPort.portNum =
            genTargetLogicalEntryPtr->outputInterface.physicalInterface.devPort.portNum;
        break;
    case CPSS_INTERFACE_TRUNK_E:
        dxChTargetLogicalEntryPtr->outputInterface.physicalInterface.type = CPSS_INTERFACE_TRUNK_E;
        dxChTargetLogicalEntryPtr->outputInterface.physicalInterface.trunkId =
            genTargetLogicalEntryPtr->outputInterface.physicalInterface.trunkId;
        break;
    case CPSS_INTERFACE_VIDX_E:
        dxChTargetLogicalEntryPtr->outputInterface.physicalInterface.type = CPSS_INTERFACE_VIDX_E;
        dxChTargetLogicalEntryPtr->outputInterface.physicalInterface.vidx =
            genTargetLogicalEntryPtr->outputInterface.physicalInterface.vidx;
        break;

    default:
        return GT_BAD_PARAM;
    }

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTargetLogicalEntryPtr, genTargetLogicalEntryPtr, egressVlanFilteringEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTargetLogicalEntryPtr, genTargetLogicalEntryPtr, egressVlanTagStateEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTargetLogicalEntryPtr, genTargetLogicalEntryPtr, egressTagTpidIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTargetLogicalEntryPtr, genTargetLogicalEntryPtr, assignVid0Enable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChTargetLogicalEntryPtr, genTargetLogicalEntryPtr, vid0);
    switch (genTargetLogicalEntryPtr->egressVlanTagState)
    {
    case PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E:
        dxChTargetLogicalEntryPtr->egressVlanTagState = CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E;
        break;
    case PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E:
        dxChTargetLogicalEntryPtr->egressVlanTagState = CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E;
        break;
    case PRV_TGF_BRG_VLAN_PORT_TAG1_CMD_E:
        dxChTargetLogicalEntryPtr->egressVlanTagState = CPSS_DXCH_BRG_VLAN_PORT_TAG1_CMD_E;
        break;
    case PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E:
        dxChTargetLogicalEntryPtr->egressVlanTagState = CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E;
        break;
    case PRV_TGF_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E:
        dxChTargetLogicalEntryPtr->egressVlanTagState = CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E;
        break;
    case PRV_TGF_BRG_VLAN_PORT_PUSH_TAG0_CMD_E:
        dxChTargetLogicalEntryPtr->egressVlanTagState = CPSS_DXCH_BRG_VLAN_PORT_PUSH_TAG0_CMD_E;
        break;
    case PRV_TGF_BRG_VLAN_PORT_POP_OUTER_TAG_CMD_E:
        dxChTargetLogicalEntryPtr->egressVlanTagState = CPSS_DXCH_BRG_VLAN_PORT_POP_OUTER_TAG_CMD_E;
        break;
    default:
        return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal prvTgfConvertDxChToGenericLogicalTargetEntry function
* @endinternal
*
* @brief   Convert device specific logical target entry into generic
*
* @param[in] dxChTargetLogicalEntryPtr - (pointer to) DxCh logical target entry
*
* @param[out] genTargetLogicalEntryPtr - (pointer to) generic logical target entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertDxChToGenericLogicalTargetEntry
(
    IN  CPSS_DXCH_LOGICAL_TARGET_MAPPING_STC  *dxChTargetLogicalEntryPtr,
    OUT PRV_TGF_LOGICAL_TARGET_MAPPING_STC    *genTargetLogicalEntryPtr
)
{
    GT_STATUS rc;

    cpssOsMemSet(genTargetLogicalEntryPtr, 0, sizeof(PRV_TGF_LOGICAL_TARGET_MAPPING_STC));
    if (dxChTargetLogicalEntryPtr->outputInterface.isTunnelStart == GT_TRUE)
    {
        genTargetLogicalEntryPtr->outputInterface.isTunnelStart = GT_TRUE;
        switch(dxChTargetLogicalEntryPtr->outputInterface.tunnelStartInfo.passengerPacketType)
        {
        case CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E:
            genTargetLogicalEntryPtr->outputInterface.tunnelStartInfo.passengerPacketType =
                PRV_TGF_TS_PASSENGER_PACKET_TYPE_ETHERNET_E;
            break;
        case CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E:
            genTargetLogicalEntryPtr->outputInterface.tunnelStartInfo.passengerPacketType =
                PRV_TGF_TS_PASSENGER_PACKET_TYPE_OTHER_E;
            break;
        default:
            return GT_BAD_PARAM;
        }
        genTargetLogicalEntryPtr->outputInterface.tunnelStartInfo.ptr =
            dxChTargetLogicalEntryPtr->outputInterface.tunnelStartInfo.ptr;
    }

    switch(dxChTargetLogicalEntryPtr->outputInterface.physicalInterface.type)
    {
    case CPSS_INTERFACE_PORT_E:
        genTargetLogicalEntryPtr->outputInterface.physicalInterface.type = CPSS_INTERFACE_PORT_E;
        rc = prvUtfSwFromHwDeviceNumberGet(dxChTargetLogicalEntryPtr->outputInterface.physicalInterface.devPort.hwDevNum,
                                           &(genTargetLogicalEntryPtr->outputInterface.physicalInterface.devPort.hwDevNum));
        if(GT_OK != rc)
            return rc;
        genTargetLogicalEntryPtr->outputInterface.physicalInterface.devPort.hwDevNum =
            dxChTargetLogicalEntryPtr->outputInterface.physicalInterface.devPort.hwDevNum;
        genTargetLogicalEntryPtr->outputInterface.physicalInterface.devPort.portNum =
            dxChTargetLogicalEntryPtr->outputInterface.physicalInterface.devPort.portNum;
        break;
    case CPSS_INTERFACE_TRUNK_E:
        genTargetLogicalEntryPtr->outputInterface.physicalInterface.type = CPSS_INTERFACE_TRUNK_E;
        genTargetLogicalEntryPtr->outputInterface.physicalInterface.trunkId =
            dxChTargetLogicalEntryPtr->outputInterface.physicalInterface.trunkId;
        break;
    case CPSS_INTERFACE_VIDX_E:
        genTargetLogicalEntryPtr->outputInterface.physicalInterface.type = CPSS_INTERFACE_VIDX_E;
        genTargetLogicalEntryPtr->outputInterface.physicalInterface.vidx =
            dxChTargetLogicalEntryPtr->outputInterface.physicalInterface.vidx;
        break;

    default:
        return GT_BAD_PARAM;
    }

    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTargetLogicalEntryPtr, genTargetLogicalEntryPtr, egressVlanFilteringEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTargetLogicalEntryPtr, genTargetLogicalEntryPtr, egressVlanTagStateEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTargetLogicalEntryPtr, genTargetLogicalEntryPtr, egressTagTpidIndex);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTargetLogicalEntryPtr, genTargetLogicalEntryPtr, assignVid0Enable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChTargetLogicalEntryPtr, genTargetLogicalEntryPtr, vid0);
    switch (dxChTargetLogicalEntryPtr->egressVlanTagState)
    {
    case CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E:
        genTargetLogicalEntryPtr->egressVlanTagState = PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E;
        break;
    case CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E:
        genTargetLogicalEntryPtr->egressVlanTagState = PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E;
        break;
    case CPSS_DXCH_BRG_VLAN_PORT_TAG1_CMD_E:
        genTargetLogicalEntryPtr->egressVlanTagState = PRV_TGF_BRG_VLAN_PORT_TAG1_CMD_E;
        break;
    case CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E:
        genTargetLogicalEntryPtr->egressVlanTagState = PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E;
        break;
    case CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E:
        genTargetLogicalEntryPtr->egressVlanTagState = PRV_TGF_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E;
        break;
    case CPSS_DXCH_BRG_VLAN_PORT_PUSH_TAG0_CMD_E:
        genTargetLogicalEntryPtr->egressVlanTagState = PRV_TGF_BRG_VLAN_PORT_PUSH_TAG0_CMD_E;
        break;
    case CPSS_DXCH_BRG_VLAN_PORT_POP_OUTER_TAG_CMD_E:
        genTargetLogicalEntryPtr->egressVlanTagState = PRV_TGF_BRG_VLAN_PORT_POP_OUTER_TAG_CMD_E;
        break;
    default:
        return GT_BAD_PARAM;
    }
    return GT_OK;
}
#endif


/**
* @internal prvTgfLogicalTargetMappingEnableSet function
* @endinternal
*
* @brief   Enable/disable logical port mapping feature on the specified device.
*
* @param[in] devNum                   - physical device number
* @param[in] enable                   - GT_TRUE:  enable
*                                      GT_FALSE: disable .
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfLogicalTargetMappingEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_BOOL   enable
)
{
#ifdef CHX_FAMILY

    GT_STATUS rc = GT_OK;
    /* call device specific API */
    rc = cpssDxChLogicalTargetMappingEnableSet(devNum, enable);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChLogicalTargetMappingEnableSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;

#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */

}


/**
* @internal prvTgfLogicalTargetMappingEnableGet function
* @endinternal
*
* @brief   Get the Enable/Disable status logical port mapping feature on the
*         specified device.
* @param[in] devNum                   - physical device number
*
* @param[out] enablePtr                - Pointer to the enable/disable state.
*                                      GT_TRUE : enable,
*                                      GT_FALSE: disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfLogicalTargetMappingEnableGet
(
    IN   GT_U8     devNum,
    OUT  GT_BOOL   *enablePtr
)
{
#ifdef CHX_FAMILY

    GT_STATUS rc = GT_OK;
    /* call device specific API */
    rc = cpssDxChLogicalTargetMappingEnableGet(devNum, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChLogicalTargetMappingEnableGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;

#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */

}


/**
* @internal prvTgfLogicalTargetMappingDeviceEnableSet function
* @endinternal
*
* @brief   Enable/disable a target device to be considered as a logical device
*         on the specified device.
* @param[in] devNum                   - physical device number
* @param[in] logicalTargetDevNum      - logical target device number.
* @param[in] enable                   - GT_TRUE:  enable
*                                      GT_FALSE: disable .
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum, logicalDevNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfLogicalTargetMappingDeviceEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_U8     logicalTargetDevNum,
    IN  GT_BOOL   enable
)
{
#ifdef CHX_FAMILY

    GT_STATUS rc = GT_OK;
    /* call device specific API */
    rc = cpssDxChLogicalTargetMappingDeviceEnableSet(devNum, logicalTargetDevNum, enable );
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChLogicalTargetMappingDeviceEnableSet FAILED, rc = [%d]", rc);

        return rc;
    }
    return rc;

#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */

}


/**
* @internal prvTgfLogicalTargetMappingDeviceEnableGet function
* @endinternal
*
* @brief   Get Enable/disable status of target device to be considered as a logical device
*         on the specified device.
* @param[in] devNum                   - physical device number
* @param[in] logicalTargetDevNum      - logical target device number.
*
* @param[out] enablePtr                - Pointer to the  Enable/disable state.
*                                      GT_TRUE : enable,
*                                      GT_FALSE: disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum, logicalDevNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfLogicalTargetMappingDeviceEnableGet
(
    IN   GT_U8     devNum,
    IN   GT_U8     logicalTargetDevNum,
    OUT  GT_BOOL   *enablePtr
)
{
#ifdef CHX_FAMILY

    GT_STATUS rc = GT_OK;
    /* call device specific API */
    rc = cpssDxChLogicalTargetMappingDeviceEnableGet(devNum, logicalTargetDevNum, enablePtr );
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChLogicalTargetMappingDeviceEnableGet FAILED, rc = [%d]", rc);

        return rc;
    }
    return rc;

#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */

}

/**
* @internal prvTgfLogicalTargetMappingTableEntrySet function
* @endinternal
*
* @brief   Set logical target mapping table entry.
*
* @param[in] devNum                   - physical device number.
* @param[in] logicalTargetDevNum      - logical traget device number.
* @param[in] logicalTargetPortNum     - logical target port number.
* @param[in] logicalTargetMappingEntryPtr - points to logical target Mapping  entry
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, logicalDevNum, logicalPortNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on the memebers of virtual port entry struct out of range.
* @retval GT_HW_ERROR              - on Hardware error.
*/
GT_STATUS prvTgfLogicalTargetMappingTableEntrySet
(
    IN GT_U8                                devNum,
    IN GT_U8                                logicalTargetDevNum,
    IN GT_U8                                logicalTargetPortNum,
    IN PRV_TGF_LOGICAL_TARGET_MAPPING_STC   *logicalTargetMappingEntryPtr
)
{
#ifdef CHX_FAMILY

    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_LOGICAL_TARGET_MAPPING_STC    dxChLogicalTargetMappingEntry;

    rc = prvTgfConvertGenericToDxChLogicalTargetEntry(logicalTargetMappingEntryPtr,
                                                      &dxChLogicalTargetMappingEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChLogicalTargetEntry FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    rc = cpssDxChLogicalTargetMappingTableEntrySet(devNum,
                                                   logicalTargetDevNum,
                                                   logicalTargetPortNum,
                                                   &dxChLogicalTargetMappingEntry );
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChLogicalTargetMappingTableEntrySet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;

#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */

}

/**
* @internal prvTgfLogicalTargetMappingTableEntryGet function
* @endinternal
*
* @brief   Get logical target mapping table entry.
*
* @param[in] devNum                   - physical device number.
* @param[in] logicalTargetDevNum      - logical target device number.
* @param[in] logicalTargetPortNum     - logical target port number.
*
* @param[out] logicalTargetMappingEntryPtr - points to logical target Mapping  entry
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, logicalDevNum, logicalPortNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on the memebers of virtual port entry struct out of range.
* @retval GT_HW_ERROR              - on Hardware error.
*/
GT_STATUS prvTgfLogicalTargetMappingTableEntryGet
(
    IN GT_U8                                devNum,
    IN GT_U8                                logicalTargetDevNum,
    IN GT_U8                                logicalTargetPortNum,
    OUT PRV_TGF_LOGICAL_TARGET_MAPPING_STC  *logicalTargetMappingEntryPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_LOGICAL_TARGET_MAPPING_STC  dxChLogicalTargetEntry;

    /* initialize dxChLogicalTargetEntry */
    cpssOsMemSet(&dxChLogicalTargetEntry , 0, sizeof(dxChLogicalTargetEntry));

    /* call device specific API */
    rc = cpssDxChLogicalTargetMappingTableEntryGet(devNum,
                                                   logicalTargetDevNum,
                                                   logicalTargetPortNum,
                                                   &dxChLogicalTargetEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChLogicalTargetMappingTableEntryGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* Convert DXCH  logical target entry into generic format */
    rc = prvTgfConvertDxChToGenericLogicalTargetEntry(&dxChLogicalTargetEntry,
                                                       logicalTargetMappingEntryPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertDxChToGenericLogicalTargetEntry FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;

#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */

}





