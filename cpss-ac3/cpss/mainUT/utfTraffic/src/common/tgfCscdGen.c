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
* @file tgfCscdGen.c
*
* @brief Generic API implementation for Cscd
*
* @version   20
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
#include <common/tgfCscdGen.h>

#ifdef CHX_FAMILY
    #include <cpss/dxCh/dxChxGen/cscd/cpssDxChCscd.h>
#endif /* CHX_FAMILY */

/******************************************************************************\
 *                           Private declarations                             *
\******************************************************************************/

/******************************************************************************\
 *                     Private function's implementation                      *
\******************************************************************************/

/******************************************************************************\
 *                       CPSS generic API section                             *
\******************************************************************************/
/**
* @internal prvTgfCscdDevMapTableSet function
* @endinternal
*
* @brief   Set the cascade map table . the map table define the local port or
*         trunk that packets destined to a destination device
*         should be transmitted to.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
* @param[in] devNum                   - physical device number
* @param[in] targetHwDevNum           - the HW device to be reached via cascade (0..31)
* @param[in] targetPortNum            - target port number
*                                      Relevant only for Lion and above and for
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_TRG_PORT_E mode,
*                                      otherwise ignored.
* @param[in] cascadeLinkPtr           - (pointer to)A structure holding the cascade link type
*                                      (port (0..63) or trunk(0..127)) and the link number
*                                      leading to the target device.
* @param[in] srcPortTrunkHashEn       - Relevant when (cascadeLinkPtr->linkType ==
*                                      CPSS_CSCD_LINK_TYPE_TRUNK_E)
*                                      Enabled to set the load balancing trunk hash for packets
*                                      forwarded via an trunk uplink to be based on the
*                                      packet’s source port, and not on the packets data.
*                                      Indicates the type of uplink.
*                                      PRV_TGF_CSCD_TRUNK_LINK_HASH_IS_INGR_PIPE_HASH_E -
*                                      Load balancing trunk hash is based on the ingress pipe
*                                      hash mode as configured by function
*                                      cpssDxChTrunkHashGeneralModeSet(...)
*                                      PRV_TGF_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E - Load
*                                      balancing trunk hash for this cascade trunk interface is
*                                      based on the packet’s source port, regardless of the
*                                      ingress pipe hash mode
*                                      NOTE : this parameter is relevant only to DXCH2
*                                      and above devices
* @param[in] egressAttributesLocallyEn - Determines whether the egress attributes
*                                      are determined by the target port even if the target
*                                      device is not the local device.
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or target device or target port
*                                       or bad trunk hash mode
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCscdDevMapTableSet
(
    IN GT_U8                        devNum,
    IN GT_HW_DEV_NUM                targetHwDevNum,
    IN GT_PORT_NUM                  targetPortNum,
    IN CPSS_CSCD_LINK_TYPE_STC      *cascadeLinkPtr,
    IN PRV_TGF_CSCD_TRUNK_LINK_HASH_ENT srcPortTrunkHashEn,
    IN GT_BOOL                      egressAttributesLocallyEn
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT dxSrcPortTrunkHashEn;
    GT_STATUS rc;
    GT_HW_DEV_NUM                       hwDevNum;             /* hardware device number */

#endif /* CHX_FAMILY */

#ifdef CHX_FAMILY
    switch(srcPortTrunkHashEn)
    {
        case PRV_TGF_CSCD_TRUNK_LINK_HASH_IS_INGR_PIPE_HASH_E:
            dxSrcPortTrunkHashEn = CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_INGR_PIPE_HASH_E;
            break;
        case PRV_TGF_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E:
            dxSrcPortTrunkHashEn = CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E;
            break;
        case PRV_TGF_CSCD_TRUNK_LINK_HASH_IS_DST_PORT_E:
            dxSrcPortTrunkHashEn = CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_DST_PORT_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    rc = prvUtfHwDeviceNumberGet(targetHwDevNum, &hwDevNum);
    if(rc != GT_OK)
    {
        return rc;
    }
    if (!PRV_CPSS_DXCH_IS_DUAL_HW_DEVICE_MAC(hwDevNum))
    {
        /* targetPortNum limited to 6 bits even for EArch */
        targetPortNum &= 0x3f;
    }

    /* call device specific API */
    return cpssDxChCscdDevMapTableSet(devNum, hwDevNum, 0, targetPortNum,cascadeLinkPtr,
                                      dxSrcPortTrunkHashEn, egressAttributesLocallyEn);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(targetHwDevNum);
    TGF_PARAM_NOT_USED(targetPortNum);
    TGF_PARAM_NOT_USED(cascadeLinkPtr);
    TGF_PARAM_NOT_USED(srcPortTrunkHashEn);
    TGF_PARAM_NOT_USED(egressAttributesLocallyEn);
    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfCscdDevMapTableGet function
* @endinternal
*
* @brief   Get the cascade map table
*
* @param[in] devNum                   - device number
* @param[in] targetHwDevNum           - the HW device to be reached via cascade
* @param[in] targetPortNum            - target port number
*
* @param[out] cascadeLinkPtr           - (pointer to) structure holding the cascade link type
* @param[out] srcPortTrunkHashEnPtr    - (pointer to) set the load balancing trunk
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or target device or target port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvTgfCscdDevMapTableGet
(
    IN  GT_U8                             devNum,
    IN GT_HW_DEV_NUM                targetHwDevNum,
    IN GT_PORT_NUM                  targetPortNum,
    OUT CPSS_CSCD_LINK_TYPE_STC          *cascadeLinkPtr,
    OUT PRV_TGF_CSCD_TRUNK_LINK_HASH_ENT *srcPortTrunkHashEnPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS                          rc       = GT_OK;
    GT_HW_DEV_NUM                      hwDevNum = 0;
    CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT dxChSrcPortTrunkHashEn;
    GT_BOOL                            egressAttributesLocallyEn;

    /* get HW device number */
    rc = prvUtfHwDeviceNumberGet(targetHwDevNum, &hwDevNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    if (!PRV_CPSS_DXCH_IS_DUAL_HW_DEVICE_MAC(hwDevNum))
    {
        /* targetPortNum limited to 6 bits even for EArch */
        targetPortNum &= 0x3f;
    }

    /* call device specific API */
    rc = cpssDxChCscdDevMapTableGet(devNum, hwDevNum, 0, targetPortNum,
                                    cascadeLinkPtr, &dxChSrcPortTrunkHashEn,
                                    &egressAttributesLocallyEn);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCscdDevMapTableGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert from device specific format */
    switch(dxChSrcPortTrunkHashEn)
    {
        case CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_INGR_PIPE_HASH_E:
            *srcPortTrunkHashEnPtr = PRV_TGF_CSCD_TRUNK_LINK_HASH_IS_INGR_PIPE_HASH_E;
            break;
        case CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E:
            *srcPortTrunkHashEnPtr = PRV_TGF_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E;
            break;
        case CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_DST_PORT_E:
            *srcPortTrunkHashEnPtr = PRV_TGF_CSCD_TRUNK_LINK_HASH_IS_DST_PORT_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(targetHwDevNum);
    TGF_PARAM_NOT_USED(targetPortNum);
    TGF_PARAM_NOT_USED(cascadeLinkPtr);
    TGF_PARAM_NOT_USED(srcPortTrunkHashEnPtr);

    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfCscdDevMapLookupModeSet function
* @endinternal
*
* @brief   Set lookup mode for accessing the Device Map table.
*
* @note   APPLICABLE DEVICES:      Lion.
* @param[in] mode                     - device Map lookup mode
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCscdDevMapLookupModeSet
(
    IN PRV_TGF_DEV_MAP_LOOKUP_MODE_ENT   mode
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
    CPSS_DXCH_DEV_MAP_LOOKUP_MODE_ENT dxMode;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        switch(mode)
        {
            case PRV_TGF_DEV_MAP_LOOKUP_MODE_TRG_DEV_E:
                dxMode = CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_E;
                break;
            case PRV_TGF_DEV_MAP_LOOKUP_MODE_TRG_DEV_TRG_PORT_E:
                dxMode = CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_TRG_PORT_E;
                break;
            default:
                return GT_BAD_PARAM;
        }

        rc = cpssDxChCscdDevMapLookupModeSet(devNum,dxMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCscdDevMapLookupModeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#else
    /* avoid warnings */
    TGF_PARAM_NOT_USED(mode);

    /* not applicable for not DXCH devices */
    return GT_NOT_APPLICABLE_DEVICE;
#endif /* CHX_FAMILY */
}

/**
* @internal prvTgfCscdDevMapLookupModeGet function
* @endinternal
*
* @brief   Get lookup mode for accessing the Device Map table
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - (pointer to) device Map lookup mode
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - wrong hardware value.
*/
GT_STATUS prvTgfCscdDevMapLookupModeGet
(
    IN  GT_U8                             devNum,
    OUT PRV_TGF_DEV_MAP_LOOKUP_MODE_ENT   *modePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS                         rc = GT_OK;
    CPSS_DXCH_DEV_MAP_LOOKUP_MODE_ENT dxChMode;


    /* call device specific API */
    rc = cpssDxChCscdDevMapLookupModeGet(devNum, &dxChMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCscdDevMapLookupModeGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert from device specific format */
    switch(dxChMode)
    {
        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_E:
            *modePtr = PRV_TGF_DEV_MAP_LOOKUP_MODE_TRG_DEV_E;
            break;
        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_TRG_PORT_E:
            *modePtr = PRV_TGF_DEV_MAP_LOOKUP_MODE_TRG_DEV_TRG_PORT_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    return rc;
#endif /* CHX_FAMILY */

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
* @internal prvTgfCscdPortLocalDevMapLookupEnableSet function
* @endinternal
*
* @brief   Enable / Disable the local target port for device map lookup
*         for local device.
*
* @note   APPLICABLE DEVICES:      Lion.
* @param[in] portNum                  - port number or CPU port
* @param[in] portDirection            - port's direction:
*                                      CPSS_DIRECTION_INGRESS_E - source port
*                                      CPSS_DIRECTION_EGRESS_E  -  target port
*                                      CPSS_DIRECTION_BOTH_E    - both source and target ports
* @param[in] enable                   - GT_TRUE  - the port is enabled for device map lookup
*                                      for local device.
*                                      - GT_FALSE - No access to Device map table for
*                                      local devices.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or port number or portDirection
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To enable access to the Device Map Table for the local target devices
*       - Enable the local source port for device map lookup
*       - Enable the local target port for device map lookup
*
*/
GT_STATUS prvTgfCscdPortLocalDevMapLookupEnableSet
(
    IN GT_U32                portNum,
    IN CPSS_DIRECTION_ENT   portDirection,
    IN GT_BOOL              enable
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        rc = cpssDxChCscdPortLocalDevMapLookupEnableSet(devNum, portNum,
                                                        portDirection,
                                                        enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCscdDevMapLookupModeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#else
    /* avoid warnings */
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(portDirection);
    TGF_PARAM_NOT_USED(enable);

    /* not applicable for not DXCH devices */
    return GT_NOT_APPLICABLE_DEVICE;
#endif /* CHX_FAMILY */
}

/**
* @internal prvTgfCscdMyPhysicalPortAssignmentModeSet function
* @endinternal
*
* @brief   Set the ingress physical port assignment mode
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port
*                                      assignmentMode      - the physical port assignment mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCscdMyPhysicalPortAssignmentModeSet
(
    IN GT_U8                                                    devNum,
    IN GT_PHYSICAL_PORT_NUM                                     portNum,
    IN PRV_TGF_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_ENT   mode
)
{
#ifdef CHX_FAMILY

    CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_ENT assignmentMode;

    switch(mode)
    {
        case PRV_TGF_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_DISABLED_E:
            assignmentMode = CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_DISABLED_E;
            break;
        case PRV_TGF_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_INTERLAKEN_E:
            assignmentMode = CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_INTERLAKEN_E;
            break;
        case PRV_TGF_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_DSA_E:
            assignmentMode = CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_DSA_E;
            break;
        case PRV_TGF_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_E_TAG_E:
            assignmentMode = CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_E_TAG_E;
            break;
        case PRV_TGF_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_VLAN_TAG_E:
            assignmentMode = CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_VLAN_TAG_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    return cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeSet(devNum,
                                                        portNum, assignmentMode);
#else
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(mode);

    return GT_NOT_SUPPORTED;
#endif
}

/**
* @internal prvTgfCscdMyPhysicalPortAssignmentModeGet function
* @endinternal
*
* @brief   Get the ingress physical port assignment mode
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCscdMyPhysicalPortAssignmentModeGet
(
    IN  GT_U8                                                     devNum,
    IN  GT_PHYSICAL_PORT_NUM                                      portNum,
    OUT PRV_TGF_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_ENT   *modePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS                                                rc = GT_OK;
    CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_ENT assignmentMode;


    /* call device specific API */
    rc = cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeGet(devNum,
                                                    portNum, &assignmentMode);

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert from device specific format */
    switch(assignmentMode)
    {
        case CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_DISABLED_E:
            *modePtr = PRV_TGF_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_DISABLED_E;
            break;
        case CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_INTERLAKEN_E:
            *modePtr = PRV_TGF_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_INTERLAKEN_E;
            break;
        case CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_DSA_E:
            *modePtr = PRV_TGF_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_DSA_E;
            break;
        case CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_E_TAG_E:
            *modePtr = PRV_TGF_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_E_TAG_E;
            break;
        case  CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_VLAN_TAG_E:
             *modePtr = PRV_TGF_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_VLAN_TAG_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    return rc;

#else
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(modePtr);

    return GT_NOT_SUPPORTED;
#endif
}


/**
* @internal prvTgfRemotePhysicalPortVlanTagTpidSet function
* @endinternal
*
* @brief   Set TPID value of the VLAN Tag taken from the
*           outermost VLAN Tag TPID port. Relevant when enabling
*           VLAN Tag based remote physical ports mapping, this
*           field specifies the TPID value of the VLAN Tag
*
* @note   APPLICABLE DEVICES:    Falcon.
* @note   NOT APPLICABLE DEVICES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum            - device number
* @param[in] vlanTagTpid       -  TPID value of the VLAN Tag.
*                                 (APPLICABLE RANGES:0..0xffff)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfRemotePhysicalPortVlanTagTpidSet
(
    IN GT_U8                                        devNum,
    IN GT_U32                                       vlanTagTpid
)
{
#ifdef CHX_FAMILY
    GT_STATUS                                   rc = GT_OK;

    rc = cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidSet(devNum,vlanTagTpid);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidSet FAILED, rc = [%d]", rc);

        return rc;
    }
    return rc;
#else
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(vlanTagTpid);

    return GT_NOT_SUPPORTED;
#endif
}


/**
* @internal prvTgfRemotePhysicalPortVlanTagTpidGet function
* @endinternal
*
* @brief   Get TPID value of the VLAN Tag taken from the
*           outermost VLAN Tag TPID port. Relevant when enabling
*           VLAN Tag based remote physical ports mapping, this
*           field specifies the TPID value of the VLAN Tag
*
* @note   APPLICABLE DEVICES:    Falcon.
* @note   NOT APPLICABLE DEVICES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum            - device number
* @param[out] vlanTagTpidPtr   -  TPID value of the VLAN Tag. 
*                                 (APPLICABLE RANGES:0..0xffff)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfRemotePhysicalPortVlanTagTpidGet
(
    IN GT_U8                                        devNum,
    OUT GT_U32                                       *vlanTagTpidPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS                                   rc = GT_OK;

    rc = cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidGet(devNum,vlanTagTpidPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidSet FAILED, rc = [%d]", rc);

        return rc;
    }
    return rc;
#else
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(vlanTagTpidPtr);

    return GT_NOT_SUPPORTED;
#endif
}


/**
* @internal prvTgfCscdMyPhysicalPortMappingDsaSrcLsbAmountSet function
* @endinternal
*
* @brief   Set the number of LSB bits taken from the DSA tag <source device> and
*         <source port>, that are used to map the My Physical port to a physical
*         port.
*         Relevant when <Assign Physical Port From DSA Enable> is enabled in the
*         My Physical Port Table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
*/
GT_STATUS prvTgfCscdMyPhysicalPortMappingDsaSrcLsbAmountSet
(
    IN GT_U32  srcDevLsbAmount,
    IN GT_U32  srcPortLsbAmount
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                            UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E );

    /* go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        rc = cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountSet(devNum,
                                                          srcDevLsbAmount, srcPortLsbAmount);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;

#else
    /* avoid warnings */
    TGF_PARAM_NOT_USED(srcDevLsbAmount);
    TGF_PARAM_NOT_USED(srcPortLsbAmount);

    /* not applicable for not DXCH devices */
    return GT_NOT_APPLICABLE_DEVICE;
#endif
}

/**
* @internal prvTgfCscdMyPhysicalPortMappingDsaSrcLsbAmountGet function
* @endinternal
*
* @brief   Get the number of LSB bits taken from the DSA tag <source device> and
*         <source port>, that are used to map the My Physical port to a physical
*         port.
*         Relevant when <Assign Physical Port From DSA Enable> is enabled in the
*         My Physical Port Table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @param[out] srcDevLsbAmountPtr       - (pointer to) the number of bits from the DSA
*                                      <source device> used for the mapping
* @param[out] srcPortLsbAmountPtr      - (pointer to) the number of bits from the DSA
*                                      <source port> used for the mapping
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
*/
GT_STATUS prvTgfCscdMyPhysicalPortMappingDsaSrcLsbAmountGet
(
    OUT GT_U32      *srcDevLsbAmountPtr,
    OUT GT_U32      *srcPortLsbAmountPtr
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                            UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E );

    /* go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        rc = cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountGet(devNum,
                                               srcDevLsbAmountPtr, srcPortLsbAmountPtr);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountGet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }

        break;
    }

    return rc1;

#else
    /* avoid warnings */
    TGF_PARAM_NOT_USED(srcDevLsbAmountPtr);
    TGF_PARAM_NOT_USED(srcPortLsbAmountPtr);

    /* not applicable for not DXCH devices */
    return GT_NOT_APPLICABLE_DEVICE;
#endif
}

/**
* @internal prvTgfCscdMyPhysicalPortBaseSet function
* @endinternal
*
* @brief   Set the ingress physical port base
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port
* @param[in] base                     - the physical port  (APPLICABLE RANGES: 0..255)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCscdMyPhysicalPortBaseSet
(
    IN GT_U8                     devNum,
    IN GT_PHYSICAL_PORT_NUM      portNum,
    IN GT_U32                    base
)
{
#ifdef CHX_FAMILY
    return cpssDxChCscdCentralizedChassisMyPhysicalPortBaseSet(devNum, portNum, base);
#else
    return GT_NOT_IMPLEMENTED;
#endif
}

/**
* @internal prvTgfCscdMyPhysicalPortBaseGet function
* @endinternal
*
* @brief   Get the ingress physical port base
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port
*
* @param[out] basePtr                  - the physical port base
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCscdMyPhysicalPortBaseGet
(
    IN  GT_U8                     devNum,
    IN  GT_PHYSICAL_PORT_NUM      portNum,
    OUT GT_U32                   *basePtr
)
{
#ifdef CHX_FAMILY
    return cpssDxChCscdCentralizedChassisMyPhysicalPortBaseGet(devNum, portNum, basePtr);
#else
    return GT_NOT_IMPLEMENTED;
#endif
}

/**
* @internal prvTgfCscdMyPhysicalPortAssignSrcDevEnableSet function
* @endinternal
*
* @brief   Enable the assignment of the packet source device to be the local device
*         number
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port
* @param[in] enable                   -  the assignment
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCscdMyPhysicalPortAssignSrcDevEnableSet
(
    IN GT_U8                     devNum,
    IN GT_PHYSICAL_PORT_NUM      portNum,
    IN GT_BOOL                   enable
)
{
#ifdef CHX_FAMILY
    return cpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableSet(devNum,
                                                    portNum, enable);
#else
    return GT_NOT_IMPLEMENTED;
#endif
}

/**
* @internal prvTgfCscdMyPhysicalPortAssignSrcDevEnableGet function
* @endinternal
*
* @brief   Get the enabling status of the assignment of the packet source device to
*         be the local device number
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port
*
* @param[out] enablePtr                - (pointer to) the enabling status of the assignment
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCscdMyPhysicalPortAssignSrcDevEnableGet
(
    IN GT_U8                     devNum,
    IN GT_PHYSICAL_PORT_NUM      portNum,
    OUT GT_BOOL                 *enablePtr
)
{
#ifdef CHX_FAMILY
    return cpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableGet(devNum,
                                                    portNum, enablePtr);
#else
    return GT_NOT_IMPLEMENTED;
#endif
}


/**
* @internal prvTgfCscdPortTypeSet function
* @endinternal
*
* @brief   Configure a PP port to be a cascade port. Application is responsible
*         for setting the default values of the port.
*         APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum  - physical device number
*         portDirection  - port's direction:
*         CPSS_DIRECTION_INGRESS_E - source port
*         CPSS_DIRECTION_EGRESS_E - target port
*         CPSS_DIRECTION_BOTH_E  - both source and target ports
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portDirection            - port's direction:
*                                      CPSS_DIRECTION_INGRESS_E - source port
*                                      CPSS_DIRECTION_EGRESS_E  - target port
*                                      CPSS_DIRECTION_BOTH_E    - both source and target ports
*                                      APPLICABLE DEVICES: Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
* @param[in] portNum                  - The port to be configured as cascade
* @param[in] portType                 - cascade  type regular/extended DSA tag port or network port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCscdPortTypeSet
(
    IN GT_U8                        devNum,
    IN CPSS_PORT_DIRECTION_ENT      portDirection,
    IN GT_PHYSICAL_PORT_NUM         portNum,
    IN CPSS_CSCD_PORT_TYPE_ENT      portType
)
{

    if(portDirection != CPSS_PORT_DIRECTION_BOTH_E &&
       GT_FALSE == prvTgfCscdIngressPortEnableCheck(devNum))
    {
        /* the device not supports different configuration for the ingress from egress */
        return GT_NOT_SUPPORTED;
    }

#ifdef CHX_FAMILY
    return cpssDxChCscdPortTypeSet(devNum,portNum,portDirection,portType);
#else
    return GT_NOT_IMPLEMENTED;
#endif
}

/**
* @internal prvTgfCscdPortTypeGet function
* @endinternal
*
* @brief   Get configuration of a PP port to be a cascade port. Application is responsible
*         for setting the default values of the port.
*         APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum  - physical device number
*         portNum - The port to be configured as cascade
*         portDirection  - port's direction:
*         CPSS_DIRECTION_INGRESS_E - source port
*         CPSS_DIRECTION_EGRESS_E - target port
*         CPSS_DIRECTION_BOTH_E  - both source and target ports
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - The port to be configured as cascade
* @param[in] portDirection            - port's direction:
*                                      CPSS_DIRECTION_INGRESS_E - source port
*                                      CPSS_DIRECTION_EGRESS_E  - target port
*                                      CPSS_DIRECTION_BOTH_E    - both source and target ports
*                                      APPLICABLE DEVICES: Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
*
* @param[out] portTypePtr              - cascade  type regular/extended DSA tag port or network port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCscdPortTypeGet
(
    IN  GT_U8                        devNum,
    IN  GT_PHYSICAL_PORT_NUM         portNum,
    IN  CPSS_PORT_DIRECTION_ENT      portDirection,
    OUT CPSS_CSCD_PORT_TYPE_ENT     *portTypePtr
)
{

    if(portDirection != CPSS_PORT_DIRECTION_BOTH_E &&
       GT_FALSE == prvTgfCscdIngressPortEnableCheck(devNum))
    {
        /* the device not supports different configuration for the ingress from egress */
        return GT_NOT_SUPPORTED;
    }

#ifdef CHX_FAMILY
    return cpssDxChCscdPortTypeGet(devNum,portNum,portDirection,portTypePtr);
#else
    return GT_NOT_IMPLEMENTED;
#endif
}


/**
* @internal prvTgfCscdIngressPortEnableCheck function
* @endinternal
*
* @brief   check if the device supports the ability to set the cascade on ingress port
*         different then on the egress of this port
*
* @retval GT_TRUE                  - the device supports the ability to set the cascade on ingress port
*                                       different then on the egress of this port
* @retval GT_FALSE                 - the device NOT supports the ability to set the cascade on ingress port
*                                       different then on the egress of this port
*/
GT_BOOL prvTgfCscdIngressPortEnableCheck(
    IN GT_U8    devNum
)
{
    if(0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        return GT_FALSE;
    }

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        return GT_TRUE;
    }

    switch(PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_LION_E   :
        case CPSS_PP_FAMILY_DXCH_LION2_E  :
            return GT_TRUE;
        default:
            return GT_FALSE;
    }
}

/**
* @internal prvTgfCscdPortBridgeBypassEnableSet function
* @endinternal
*
* @brief   The function enables/disables bypass of the bridge engine per port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -  PP's device number.
* @param[in] portNum                  -  port number or CPU port
* @param[in] enable                   -  GT_TRUE  - Enable bypass of the bridge engine.
*                                      GT_FALSE - Disable bypass of the bridge engine.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note If the DSA tag is not extended Forward, the bridging decision
*       is performed regardless of the setting.
*       When bypass enabled the Bridge engine still learn MAC source addresses,
*       but will not modify the packet command, attributes (or forwarding
*       decision).
*
*/
GT_STATUS prvTgfCscdPortBridgeBypassEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_PORT_NUM portNum,
    IN  GT_BOOL     enable
)
{
#ifdef CHX_FAMILY
    return cpssDxChCscdPortBridgeBypassEnableSet(devNum,portNum,enable);
#else
    return GT_NOT_IMPLEMENTED;
#endif
}
/**
* @internal prvTgfCscdDbRemoteHwDevNumModeSet function
* @endinternal
*
* @brief   Set single/dual HW device number mode to remote HW device number.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] hwDevNum                 - HW device number (0..(4K-1)).
* @param[in] hwDevMode                - single/dual HW device number mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - hwDevNum is odd number and hwDevMode is
*                                       CPSS_GEN_CFG_HW_DEV_NUM_MODE_DUAL_E
* @retval GT_OUT_OF_RANGE          - when hwDevNum is out of range
*
* @note 1. Only even device numbers allowed to be marked as "dual HW device"
*       2. "Dual HW device" mode must be configured before any other
*       configuration that uses hwDevNum.
*       3. There are no restrictions on SW devNum for dual mode devices.
*
*/
GT_STATUS prvTgfCscdDbRemoteHwDevNumModeSet
(
    IN GT_HW_DEV_NUM                    hwDevNum,
    IN CPSS_GEN_CFG_HW_DEV_NUM_MODE_ENT hwDevMode
)
{
#ifdef CHX_FAMILY
    return cpssDxChCscdDbRemoteHwDevNumModeSet(hwDevNum,hwDevMode);
#else
    return GT_NOT_IMPLEMENTED;
#endif
}

/**
* @internal prvTgfCscdDsaSrcDevFilterSet function
* @endinternal
*
* @brief   Enable/Disable filtering the ingress DSA tagged packets in which
*         source id equals to local device number.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] enableOwnDevFltr         - enable/disable ingress DSA loop filter
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCscdDsaSrcDevFilterSet
(
    IN GT_BOOL      enableOwnDevFltr
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum,rc , UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        rc = cpssDxChCscdDsaSrcDevFilterSet(devNum,enableOwnDevFltr);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfCscdDsaSrcDevFilterSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#else
    /* avoid warnings */
    TGF_PARAM_NOT_USED(enableOwnDevFltr);

    /* not applicable for not DXCH devices */
    return GT_NOT_APPLICABLE_DEVICE;
#endif /* CHX_FAMILY */
}

/**
* @internal prvTgfCscdDsaSrcDevFilterGet function
* @endinternal
*
* @brief   get value of Enable/Disable filtering the ingress DSA tagged packets in which
*         source id equals to local device number.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @param[out] enableOwnDevFltrPtr      - (pointer to) enable/disable ingress DSA loop filter
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCscdDsaSrcDevFilterGet
(
    OUT GT_BOOL      *enableOwnDevFltrPtr
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum,rc , UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        rc = cpssDxChCscdDsaSrcDevFilterGet(devNum,enableOwnDevFltrPtr);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfCscdDsaSrcDevFilterGet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }

        break;
    }

    return rc1;
#else
    /* avoid warnings */
    TGF_PARAM_NOT_USED(enableOwnDevFltrPtr);

    /* not applicable for not DXCH devices */
    return GT_NOT_APPLICABLE_DEVICE;
#endif /* CHX_FAMILY */
}


/**
* @internal prvTgfCscdOrigSrcPortFilterEnableSet function
* @endinternal
*
* @brief   Enable/Disable filtering the multi-destination packet that was received
*         by the local device, sent to another device, and sent back to this
*         device, from being sent back to the network port at which it was
*         initially received.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] enable                   - GT_TRUE - filter and drop the packet
*                                      - GT_FALSE - don't filter the packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCscdOrigSrcPortFilterEnableSet
(
    IN GT_BOOL      enable
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum,rc , UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        rc = cpssDxChCscdOrigSrcPortFilterEnableSet(devNum,enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfCscdOrigSrcPortFilterEnableSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#else
    /* avoid warnings */
    TGF_PARAM_NOT_USED(enable);

    /* not applicable for not DXCH devices */
    return GT_NOT_APPLICABLE_DEVICE;
#endif /* CHX_FAMILY */
}

/**
* @internal prvTgfCscdOrigSrcPortFilterEnableGet function
* @endinternal
*
* @brief   Get the status of filtering the multi-destination packet that was
*         received by the local device, sent to another device, and sent back to
*         this device, from being sent back to the network port at which it was
*         initially received.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @param[out] enablePtr                - GT_TRUE - filter and drop the packet
*                                      - GT_FALSE - don't filter the packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCscdOrigSrcPortFilterEnableGet
(
    OUT GT_BOOL      *enablePtr
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum,rc , UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        rc = cpssDxChCscdOrigSrcPortFilterEnableGet(devNum,enablePtr);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfCscdOrigSrcPortFilterEnableGet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }

        break;
    }

    return rc1;
#else
    /* avoid warnings */
    TGF_PARAM_NOT_USED(enablePtr);

    /* not applicable for not DXCH devices */
    return GT_NOT_APPLICABLE_DEVICE;
#endif /* CHX_FAMILY */
}


/**
* @internal prvTgfCscdPortStackAggregationEnableSet function
* @endinternal
*
* @brief   Enable/disable stack aggregation per port.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE -  stack aggregation
*                                      GT_FALSE - disable stack aggregation
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or portNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCscdPortStackAggregationEnableSet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_BOOL              enable
)
{
#ifdef CHX_FAMILY
    return cpssDxChCscdPortStackAggregationEnableSet(devNum, portNum, enable);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(enable);

    /* not applicable for not DXCH devices */
    return GT_NOT_APPLICABLE_DEVICE;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif

}


/**
* @internal prvTgfCscdPortStackAggregationEnableGet function
* @endinternal
*
* @brief   Get enable/disable status of stack aggregation per port.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - aggregator port number
*
* @param[out] enablePtr                - (pointer to)GT_TRUE - enable stack aggregation
*                                      GT_FALSE - disable stack aggregation
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or portNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCscdPortStackAggregationEnableGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
)
{
#ifdef CHX_FAMILY
    return cpssDxChCscdPortStackAggregationEnableGet(devNum, portNum, enablePtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(enablePtr);

    /* not applicable for not DXCH devices */
    return GT_NOT_APPLICABLE_DEVICE;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif

}


/**
* @internal prvTgfCscdPortStackAggregationConfigSet function
* @endinternal
*
* @brief   Set stack aggregation configuration per port.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] aggHwDevNum              - aggregator HW device number
* @param[in] aggPortNum               - aggregator port number
* @param[in] aggSrcId                 - aggregator source ID
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, portNum
* @retval GT_OUT_OF_RANGE          - on wrong aggHwDevNum, aggPortNum, aggSrcId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCscdPortStackAggregationConfigSet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_HW_DEV_NUM        aggHwDevNum,
    IN  GT_PORT_NUM          aggPortNum,
    IN  GT_U32               aggSrcId
)
{
#ifdef CHX_FAMILY
    return cpssDxChCscdPortStackAggregationConfigSet(devNum, portNum, aggHwDevNum, aggPortNum, aggSrcId);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(aggHwDevNum);
    TGF_PARAM_NOT_USED(aggPortNum);
    TGF_PARAM_NOT_USED(aggSrcId);

    /* not applicable for not DXCH devices */
    return GT_NOT_APPLICABLE_DEVICE;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif

}


/**
* @internal prvTgfCscdPortStackAggregationConfigGet function
* @endinternal
*
* @brief   Get stack aggregation configuration per port.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] aggHwDevNumPtr           - (pointer to) aggregator HW device number
* @param[out] aggPortNumPtr            - (pointer to) aggregator port number
* @param[out] aggSrcIdPtr              - (pointer to) aggregator source ID
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, portNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCscdPortStackAggregationConfigGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_HW_DEV_NUM        *aggHwDevNumPtr,
    OUT GT_PORT_NUM          *aggPortNumPtr,
    OUT GT_U32               *aggSrcIdPtr
)
{
#ifdef CHX_FAMILY
    return cpssDxChCscdPortStackAggregationConfigGet(devNum, portNum, aggHwDevNumPtr, aggPortNumPtr, aggSrcIdPtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(aggHwDevNumPtr);
    TGF_PARAM_NOT_USED(aggPortNumPtr);
    TGF_PARAM_NOT_USED(aggSrcIdPtr);

    /* not applicable for not DXCH devices */
    return GT_NOT_APPLICABLE_DEVICE;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif

}


/**
* @internal prvTgfCscdStackAggregationCheck function
* @endinternal
*
* @brief   check if the device supports the stack aggregation
*
* @retval GT_TRUE                  - the device supports the stack aggregation
* @retval GT_FALSE                 - the device not supports the stack aggregation
*/
GT_BOOL prvTgfCscdStackAggregationCheck(
    void
)
{
#ifdef CHX_FAMILY
    GT_U8     devNum = prvTgfDevNum;
    GT_U8     revision;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;

    prvUtfDeviceFamilyGet(devNum, &devFamily);

    prvUtfDeviceRevisionGet(devNum, &revision);

    if((devFamily != CPSS_PP_FAMILY_DXCH_LION2_E) || (revision < 1))
    {
        /* the device not support the stack aggregation */
        return GT_FALSE;
    }

    return GT_TRUE;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)

    return GT_FALSE;
#endif /* !(defined CHX_FAMILY) */
}


/**
* @internal prvTgfCscdCentralizedChassisLineCardDefaultEportBaseSet function
* @endinternal
*
* @brief   Set initial default ePort mapping assignment on ingress centralized chassis
*         enabled ports, when packets are received from line-card port/trunk accordingly.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] portBaseEport            - base value used for default ePort mapping on
*                                      ingress CC enabled ports, when packets are
*                                      received from line-card port.
*                                      (APPLICABLE RANGES: 0..0x1FFFF)
* @param[in] trunkBaseEport           base value used for default ePort mapping on
*                                      ingress CC enabled ports, when packets are
*                                      received from line-card trunk.
*                                      (APPLICABLE RANGES: 0..0x1FFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
*/
GT_STATUS prvTgfCscdCentralizedChassisLineCardDefaultEportBaseSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_PORT_NUM                         portBaseEport,
    IN  GT_PORT_NUM                         trunkBaseEport
)
{
#ifdef CHX_FAMILY
    return cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseSet(devNum,portNum,portBaseEport,trunkBaseEport);
#else
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(portBaseEport);
    TGF_PARAM_NOT_USED(trunkBaseEport);
    return GT_NOT_APPLICABLE_DEVICE;
#endif
}

/**
* @internal prvTgfCscdCentralizedChassisLineCardDefaultEportBaseGet function
* @endinternal
*
* @brief   Get initial default ePort mapping assignment on ingress centralized chassis
*         enabled ports, when packets are received from line-card port/trunk accordingly.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] portBaseEportPtr         - (pointer to) base value used for default
*                                      ePort mapping on ingress CC enabled ports,
*                                      when packets are received from line-card port.
*                                      (APPLICABLE RANGES: 0..0x1FFFF)
*                                      trunkBaseEport    - (pointer to) base value used for default
*                                      ePort mapping on ingress CC enabled ports,
*                                      when packets are received from line-card trunk.
*                                      (APPLICABLE RANGES: 0..0x1FFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCscdCentralizedChassisLineCardDefaultEportBaseGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT GT_PORT_NUM                         *portBaseEportPtr,
    OUT GT_PORT_NUM                         *trunkBaseEportPtr
)
{
#ifdef CHX_FAMILY
    return cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseGet(devNum,portNum,portBaseEportPtr,trunkBaseEportPtr);
#else
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(portBaseEportPtr);
    TGF_PARAM_NOT_USED(trunkBaseEportPtr);
    return GT_NOT_APPLICABLE_DEVICE;
#endif
}

/**
* @internal prvTgfCscdCentralizedChassisMappingDsaSrcLsbAmountSet function
* @endinternal
*
* @brief   Set the amount of least significant bits taken from DSA tag
*         for assigning a default source ePort on CC ports, for packets received
*         from line-card device trunks/physical ports accordingly.
*         Relevant only when <CC Mode Enable> = Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] srcTrunkLsbAmount        - amount of least significant bits taken from DSA<SrcTrunk>
*                                      for assigning a default source ePort on CC ports, for
*                                      packets received from line-card device trunks.
*                                      Relevant only for packet DSA<SrcIsTrunk> = 1.
*                                      (APPLICABLE RANGES: 0..7)
* @param[in] srcPortLsbAmount         - amount of least significant bits taken from DSA<SrcPort>
*                                      for assigning a default source ePort on CC ports, for
*                                      packets received from line-card device physical ports.
*                                      Relevant only for packet DSA<SrcIsTrunk> = 0.
*                                      (APPLICABLE RANGES: 0..6)
* @param[in] srcDevLsbAmount          - amount of least significant bits taken from DSA<SrcDev>
*                                      for assigning a default source ePort on CC ports, for
*                                      packets received from line-card device physical ports.
*                                      Relevant only for packet DSA<SrcIsTrunk> = 0.
*                                      (APPLICABLE RANGES: 0..5)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
*/
GT_STATUS prvTgfCscdCentralizedChassisMappingDsaSrcLsbAmountSet
(
    IN  GT_U32                              srcTrunkLsbAmount,
    IN  GT_U32                              srcPortLsbAmount,
    IN  GT_U32                              srcDevLsbAmount
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum,rc , UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        rc = cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountSet(devNum,srcTrunkLsbAmount,srcPortLsbAmount,srcDevLsbAmount);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfCscdDsaSrcDevFilterSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#else
    /* avoid warnings */
    TGF_PARAM_NOT_USED(srcTrunkLsbAmount);
    TGF_PARAM_NOT_USED(srcPortLsbAmount);
    TGF_PARAM_NOT_USED(srcDevLsbAmount);

    /* not applicable for not DXCH devices */
    return GT_NOT_APPLICABLE_DEVICE;
#endif /* CHX_FAMILY */
}

/**
* @internal prvTgfCscdCentralizedChassisMappingDsaSrcLsbAmountGet function
* @endinternal
*
* @brief   Get the amount of least significant bits taken from DSA tag
*         for assigning a default source ePort on CC ports, for packets received
*         from line-card device trunks/physical ports accordingly.
*         Relevant only when <CC Mode Enable> = Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @param[out] srcTrunkLsbAmountPtr     - (pointer to) amount of least significant bits taken
*                                      from DSA<SrcTrunk> for assigning a default source
*                                      ePort on CC ports, for packets received from
*                                      line-card device trunks.
*                                      Relevant only for packet DSA<SrcIsTrunk> = 1.
*                                      (APPLICABLE RANGES: 0..7)
* @param[out] srcPortLsbAmountPtr      - (pointer to) amount of least significant bits taken
*                                      from DSA<SrcPort> for assigning a default source
*                                      ePort on CC ports, for packets received from
*                                      line-card device physical ports.
*                                      Relevant only for packet DSA<SrcIsTrunk> = 0.
*                                      (APPLICABLE RANGES: 0..6)
* @param[out] srcDevLsbAmountPtr       - (pointer to) amount of least significant bits taken
*                                      from DSA<SrcDev> for assigning a default source
*                                      ePort on CC ports, for packets received from
*                                      line-card device physical ports.
*                                      Relevant only for packet DSA<SrcIsTrunk> = 0.
*                                      (APPLICABLE RANGES: 0..5)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCscdCentralizedChassisMappingDsaSrcLsbAmountGet
(
    OUT GT_U32                              *srcTrunkLsbAmountPtr,
    OUT GT_U32                              *srcPortLsbAmountPtr,
    OUT GT_U32                              *srcDevLsbAmountPtr
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                            UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E );

    /* go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        rc = cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountGet(devNum,srcTrunkLsbAmountPtr,srcPortLsbAmountPtr,srcDevLsbAmountPtr);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfCscdDsaSrcDevFilterGet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }

        break;
    }

    return rc1;
#else
    /* avoid warnings */
    TGF_PARAM_NOT_USED(srcTrunkLsbAmountPtr);
    TGF_PARAM_NOT_USED(srcPortLsbAmountPtr);
    TGF_PARAM_NOT_USED(srcDevLsbAmountPtr);

    /* not applicable for not DXCH devices */
    return GT_NOT_APPLICABLE_DEVICE;
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfCscdCentralizedChassisModeEnableSet function
* @endinternal
*
* @brief   Enable/Disable initial local source port assignment from DSA tag, used
*         for centralized chassis.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   - Centralized Chassis Mode
*                                      GT_FALSE: Ingress port is not connected to a line-card
*                                      device in a centralized chassis system
*                                      GT_TRUE:  Ingress port is connected to a line-card
*                                      device in a centralized chassis system
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCscdCentralizedChassisModeEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_BOOL                             enable
)
{
#ifdef CHX_FAMILY
    return cpssDxChCscdCentralizedChassisModeEnableSet(devNum,portNum,enable);
#else
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(enable);
    return GT_NOT_APPLICABLE_DEVICE;
#endif
}

/**
* @internal prvTgfCscdCentralizedChassisModeEnableGet function
* @endinternal
*
* @brief   Get initial local source port assignment from DSA tag, used
*         for centralized chassis.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to) Centralized Chassis Mode
*                                      GT_FALSE: Ingress port is not connected to a line-card
*                                      device in a centralized chassis system
*                                      GT_TRUE:  Ingress port is connected to a line-card
*                                      device in a centralized chassis system
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCscdCentralizedChassisModeEnableGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT GT_BOOL                             *enablePtr
)
{
#ifdef CHX_FAMILY
    return cpssDxChCscdCentralizedChassisModeEnableGet(devNum,portNum,enablePtr);
#else
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(enablePtr);
    return GT_NOT_APPLICABLE_DEVICE;
#endif
}

/**
* @internal prvTgfCscdCentralizedChassisRemotePhysicalPortMapEnableSet function
* @endinternal
*
* @brief   Enable mapping of target physical port to a remote physical port that
*         resides over a DSA-tagged interface
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - target device number
* @param[in] portNum                  - target physical port
* @param[in] enable                   -  mapping
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCscdCentralizedChassisRemotePhysicalPortMapEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_BOOL                             enable
)
{
#ifdef CHX_FAMILY
    return cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableSet(devNum, portNum, enable);
#else
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(enable);
    return GT_NOT_APPLICABLE_DEVICE;
#endif
}

/**
* @internal prvTgfCscdCentralizedChassisRemotePhysicalPortMapEnableGet function
* @endinternal
*
* @brief   Get the enabling status of the mapping of target physical port to a
*         remote physical port that resides over a DSA-tagged interface
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - target device number
* @param[in] portNum                  - target physical port
*
* @param[out] enablePtr                - (pointer to) the enabling status
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCscdCentralizedChassisRemotePhysicalPortMapEnableGet
(
    IN  GT_U8                     devNum,
    IN  GT_PHYSICAL_PORT_NUM      portNum,
    OUT GT_BOOL                   *enablePtr
)
{
#ifdef CHX_FAMILY
    return cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableGet(devNum, portNum, enablePtr);
#else
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(enablePtr);
    return GT_NOT_APPLICABLE_DEVICE;
#endif
}

/**
* @internal prvTgfCscdCentralizedChassisRemotePhysicalPortMapSet function
* @endinternal
*
* @brief   Map a target physical port to a remote physical port that resides over
*         a DSA-tagged interface
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - target device number
* @param[in] portNum                  - target physical port
* @param[in] remotePhysicalHwDevNum   - the remote physical HW device
* @param[in] remotePhysicalPortNum    - the remote physical port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCscdCentralizedChassisRemotePhysicalPortMapSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_HW_DEV_NUM           remotePhysicalHwDevNum,
    IN  GT_PHYSICAL_PORT_NUM    remotePhysicalPortNum
)
{
#ifdef CHX_FAMILY
    return cpssDxChCscdCentralizedChassisRemotePhysicalPortMapSet(devNum, portNum,
                                                                  remotePhysicalHwDevNum,
                                                                  remotePhysicalPortNum);
#else
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(remotePhysicalHwDevNum);
    TGF_PARAM_NOT_USED(remotePhysicalPortNum);
    return GT_NOT_APPLICABLE_DEVICE;
#endif
}

/**
* @internal prvTgfCscdCentralizedChassisRemotePhysicalPortMapGet function
* @endinternal
*
* @brief   Get the mapping of a target physical port to a remote physical port that
*         resides over a DSA-tagged interface
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - target device number
* @param[in] portNum                  - target physical port
*
* @param[out] remotePhysicalHwDevNumPtr - (pointer to) the remote physical HW device
* @param[out] remotePhysicalPortNumPtr - (pointer to) the remote physical port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCscdCentralizedChassisRemotePhysicalPortMapGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_HW_DEV_NUM           *remotePhysicalHwDevNumPtr,
    OUT GT_PHYSICAL_PORT_NUM    *remotePhysicalPortNumPtr
)
{
#ifdef CHX_FAMILY
    return cpssDxChCscdCentralizedChassisRemotePhysicalPortMapGet(devNum, portNum,
                                                                  remotePhysicalHwDevNumPtr,
                                                                  remotePhysicalPortNumPtr);
#else
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(remotePhysicalHwDevNumPtr);
    TGF_PARAM_NOT_USED(remotePhysicalPortNumPtr);
    return GT_NOT_APPLICABLE_DEVICE;
#endif
}


/**
* @internal prvTgfCscdQosPortTcRemapEnableSet function
* @endinternal
*
* @brief   Enable/Disable Traffic Class Remapping on cascading port.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - cascading port number or CPU port
* @param[in] enable                   - GT_TRUE:  Traffic Class remapping
*                                      GT_FALSE: disable Traffic Class remapping
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCscdQosPortTcRemapEnableSet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_BOOL              enable
)
{
#ifdef CHX_FAMILY
    return cpssDxChCscdQosPortTcRemapEnableSet(devNum, portNum, enable);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(enable);

    /* not applicable for not DXCH devices */
    return GT_NOT_APPLICABLE_DEVICE;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif

}

/**
* @internal prvTgfCscdQosPortTcRemapEnableGet function
* @endinternal
*
* @brief   Get the status of Traffic Class Remapping on cascading port
*         (Enabled/Disabled).
* @param[in] devNum                   - device number
* @param[in] portNum                  - cascading port number or CPU port
*
* @param[out] enablePtr                - GT_TRUE: Traffic Class remapping enabled
*                                      GT_FALSE: Traffic Class remapping disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCscdQosPortTcRemapEnableGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
)
{
#ifdef CHX_FAMILY
    return cpssDxChCscdQosPortTcRemapEnableGet(devNum, portNum, enablePtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(enablePtr);

    /* not applicable for not DXCH devices */
    return GT_NOT_APPLICABLE_DEVICE;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif

}

/**
* @internal prvTgfCscdQosTcDpRemapTableGet function
* @endinternal
*
* @brief   Get the remapped value of priority queue, drop precedence and priority queue for PFC
*         assigned to the packet for given (packet TC, packet ingress Port Type,
*         packet DSA cmd, packet drop precedence and packet multi or single-destination) on cascading port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] tcDpRemappingPtr         - (pointer to )tc,dp remapping index structure.
*
* @param[out] remappedTcPtr            - (pointer to) priority queue assigned to the packet. (APPLICABLE RANGES: 0..7).
* @param[out] remappedDpPtr            - (pointer to) drop precedence assigned to the packet.
* @param[out] remappedPfcTcPtr         - (pointer to) priority queue for PFC assigned to the packet.
*                                      (APPLICABLE DEVICES Bobcat3; Aldrin2; Falcon).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCscdQosTcDpRemapTableGet
(
    IN  GT_U8                                    devNum,
    IN  PRV_TGF_CSCD_QOS_TC_DP_REMAP_INDEX_STC   *tcDpRemappingPtr,
    OUT  GT_U32                                  *remappedTcPtr,
    OUT  CPSS_DP_LEVEL_ENT                       *remappedDpPtr,
    OUT  GT_U32                                  *remappedPfcTcPtr
)
{

#ifdef CHX_FAMILY
    CPSS_DXCH_CSCD_QOS_TC_DP_REMAP_INDEX_STC tcDpRemapping;
    CPSS_PORT_TX_DROP_MCAST_PRIORITY_ENT    mcastPriority;
    tcDpRemapping.dp = tcDpRemappingPtr->dp;
    tcDpRemapping.dsaTagCmd = tcDpRemappingPtr->dsaTagCmd;
    tcDpRemapping.isStack = tcDpRemappingPtr->isStack;
    tcDpRemapping.tc = tcDpRemappingPtr->tc;
    tcDpRemapping.targetPortTcProfile = tcDpRemappingPtr->targetPortTcProfile;
    tcDpRemapping.packetIsMultiDestination = tcDpRemappingPtr->packetIsMultiDestination;
    return cpssDxChCscdQosTcDpRemapTableGet(devNum, &tcDpRemapping, remappedTcPtr, remappedDpPtr, remappedPfcTcPtr,&mcastPriority);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(tcDpRemappingPtr);
    TGF_PARAM_NOT_USED(remappedTcPtr);
    TGF_PARAM_NOT_USED(remappedDpPtr);
    TGF_PARAM_NOT_USED(remappedPfcTcPtr);
    /* not applicable for not DXCH devices */
    return GT_NOT_APPLICABLE_DEVICE;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif

}


/**
* @internal prvTgfCscdQosTcDpRemapTableSet function
* @endinternal
*
* @brief   Set remaping of (packet TC, packet ingress Port Type, packet DSA cmd, packet drop precedence and packet cast)
*         on cascading port to new priority queue for enqueuing the packet, new drop precedence and new priority queue for PFC
*         assigned to this packet.
*         APPLICABLE DEVICES:
*         Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*         INPUTS:
*         devNum     - device number
*         tcDpRemappingPtr  - (pointer to )tc,dp remapping index structure.
*         newTc   - new priority queue assigned to the packet. (APPLICABLE RANGES: 0..7).
*         newDp   - new drop precedence assigned to the packet.
*         newPfcTc - new priority queue fot PFC assigned to the packet.
*         (APPLICABLE RANGES: 0..7
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] tcDpRemappingPtr         - (pointer to )tc,dp remapping index structure.
* @param[in] newTc                    - new priority queue assigned to the packet. (APPLICABLE RANGES: 0..7).
* @param[in] newDp                    - new drop precedence assigned to the packet.
* @param[in] newPfcTc                 - new priority queue fot PFC assigned to the packet.
*                                      (APPLICABLE RANGES: 0..7
*                                      APPLICABLE DEVICES: Bobcat3; Aldrin2; Falcon).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCscdQosTcDpRemapTableSet
(
    IN  GT_U8                                    devNum,
    IN  PRV_TGF_CSCD_QOS_TC_DP_REMAP_INDEX_STC   *tcDpRemappingPtr,
    IN  GT_U32                                   newTc,
    IN  CPSS_DP_LEVEL_ENT                        newDp,
    IN  GT_U32                                   newPfcTc
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_CSCD_QOS_TC_DP_REMAP_INDEX_STC tcDpRemapping;
    tcDpRemapping.dp = tcDpRemappingPtr->dp;
    tcDpRemapping.dsaTagCmd = tcDpRemappingPtr->dsaTagCmd;
    tcDpRemapping.isStack = tcDpRemappingPtr->isStack;
    tcDpRemapping.tc = tcDpRemappingPtr->tc;
    tcDpRemapping.targetPortTcProfile = tcDpRemappingPtr->targetPortTcProfile;
    tcDpRemapping.packetIsMultiDestination = tcDpRemappingPtr->packetIsMultiDestination;
    return cpssDxChCscdQosTcDpRemapTableSet(devNum, &tcDpRemapping, newTc, newDp, newPfcTc,CPSS_PORT_TX_DROP_MCAST_PRIORITY_LOW_E);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(tcDpRemappingPtr);
    TGF_PARAM_NOT_USED(newTc);
    TGF_PARAM_NOT_USED(newDp);
    TGF_PARAM_NOT_USED(newPfcTc);
    /* not applicable for not DXCH devices */
    return GT_NOT_APPLICABLE_DEVICE;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}


/**
* @internal prvTgfCscdPortQosDsaModeSet function
* @endinternal
*
* @brief   Configures DSA tag QoS trust mode for cascade port.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - cascading port number.
* @param[in] portQosDsaMode           - DSA tag QoS mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, port,portQosDsaTrustMode.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfCscdPortQosDsaModeSet
(
    IN GT_U8                         devNum,
    IN  GT_PHYSICAL_PORT_NUM         portNum,
    IN PRV_TGF_CSCD_PORT_QOS_DSA_TRUST_MODE_ENT  portQosDsaMode
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_CSCD_PORT_QOS_DSA_TRUST_MODE_ENT  portQosDsaModeSet;
    switch(portQosDsaMode)
    {
    case PRV_TGF_CSCD_PORT_QOS_DSA_TRUST_MODE_GLOBAL_E:
        portQosDsaModeSet = CPSS_DXCH_CSCD_PORT_QOS_DSA_TRUST_MODE_GLOBAL_E;
        break;
    case PRV_TGF_CSCD_PORT_QOS_DSA_TRUST_MODE_EXTENDED_E:
        portQosDsaModeSet = CPSS_DXCH_CSCD_PORT_QOS_DSA_TRUST_MODE_EXTENDED_E;
        break;
    default:
        return GT_BAD_PARAM;
    }
    return cpssDxChCscdPortQosDsaModeSet(devNum, portNum, portQosDsaModeSet);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(portQosDsaMode);
    /* not applicable for not DXCH devices */
    return GT_NOT_APPLICABLE_DEVICE;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}


/**
* @internal prvTgfCscdPortQosDsaModeGet function
* @endinternal
*
* @brief   Get DSA tag QoS trust mode for cascade port.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - cascading port number.
*
* @param[out] portQosDsaTrustModePtr   - pointer to DSA tag QoS trust mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, port.
* @retval GT_BAD_PTR               - portQosDsaTrustModePtr is NULL pointer.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfCscdPortQosDsaModeGet
(
    IN  GT_U8                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                        portNum,
    OUT PRV_TGF_CSCD_PORT_QOS_DSA_TRUST_MODE_ENT   *portQosDsaTrustModePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_CSCD_PORT_QOS_DSA_TRUST_MODE_ENT  portQosDsaModeGet;
    rc =  cpssDxChCscdPortQosDsaModeGet(devNum, portNum, &portQosDsaModeGet);
    if (rc != GT_OK)
    {
        return rc;
    }
    switch(portQosDsaModeGet)
    {
    case CPSS_DXCH_CSCD_PORT_QOS_DSA_TRUST_MODE_GLOBAL_E:
        *portQosDsaTrustModePtr = PRV_TGF_CSCD_PORT_QOS_DSA_TRUST_MODE_GLOBAL_E;
        break;
    case CPSS_DXCH_CSCD_PORT_QOS_DSA_TRUST_MODE_EXTENDED_E:
        *portQosDsaTrustModePtr = PRV_TGF_CSCD_PORT_QOS_DSA_TRUST_MODE_EXTENDED_E;
        break;
    default:
        return GT_BAD_STATE;
    }
    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(portQosDsaTrustModePtr);
    /* not applicable for not DXCH devices */
    return GT_NOT_APPLICABLE_DEVICE;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif

}

/**
* @internal prvTgfCscdPortTcProfiletSet function
* @endinternal
*
* @brief   Sets port TC profile on source/target port.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] portDirection            - port's direction.
* @param[in] portProfile              - port's profile values.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCscdPortTcProfiletSet
(
    IN    GT_U8                      devNum,
    IN    GT_PHYSICAL_PORT_NUM       portNum,
    IN    CPSS_PORT_DIRECTION_ENT    portDirection,
    IN    PRV_TGF_PORT_PROFILE_ENT   portProfile
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_PORT_PROFILE_ENT dxPortProfile = (CPSS_DXCH_PORT_PROFILE_ENT)portProfile;

    return cpssDxChCscdPortTcProfiletSet(devNum, portNum, portDirection, dxPortProfile);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* not applicable for not DXCH devices */
    return GT_NOT_APPLICABLE_DEVICE;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif

}

/**
* @internal prvTgfCscdPortTcProfiletGet function
* @endinternal
*
* @brief   Gets port TC profile on source/target port.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] portDirection            - port's direction.
*
* @param[out] portProfilePtr           - (pointer to) port's profile values.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCscdPortTcProfiletGet
(
    IN    GT_U8                      devNum,
    IN    GT_PHYSICAL_PORT_NUM       portNum,
    IN    CPSS_PORT_DIRECTION_ENT    portDirection,
    OUT   PRV_TGF_PORT_PROFILE_ENT * portProfilePtr
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_PORT_PROFILE_ENT dxPortProfile;
    GT_STATUS rc;
    rc = cpssDxChCscdPortTcProfiletGet(devNum, portNum, portDirection, &dxPortProfile);
    *portProfilePtr = (PRV_TGF_PORT_PROFILE_ENT)dxPortProfile;
    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* not applicable for not DXCH devices */
    return GT_NOT_APPLICABLE_DEVICE;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif

}


