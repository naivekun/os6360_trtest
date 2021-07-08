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
*       Generic API for Mirror APIs.
*
* FILE REVISION NUMBER:
*       $Revision: 23 $
*
*******************************************************************************/

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

#include <trafficEngine/tgfTrafficEngine.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfMirror.h>


/******************************************************************************\
 *                       CPSS generic API section                             *
\******************************************************************************/

/**
* @internal prvTgfMirrorTxGlobalAnalyzerInterfaceIndexSet function
* @endinternal
*
* @brief   This function sets analyzer interface index, used for egress
*         mirroring for Port-Based hop-by-hop mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfMirrorTxGlobalAnalyzerInterfaceIndexSet
(
    IN GT_BOOL   enable,
    IN GT_U32    index
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
        rc = cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexSet(devNum, enable, index);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(enable);
    TGF_PARAM_NOT_USED(index);

    return GT_NOT_IMPLEMENTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfMirrorTxGlobalAnalyzerInterfaceIndexGet function
* @endinternal
*
* @brief   This function gets analyzer interface index, used for egress
*         mirroring for Port-Based hop-by-hop mode.
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - Pointer to global enable/disable mirroring for
*                                      Port-Based hop-by-hop mode.
*                                      - GT_TRUE - enable mirroring.
*                                      - GT_FALSE - No mirroring.
* @param[out] indexPtr                 - pointer to analyzer destination interface index.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfMirrorTxGlobalAnalyzerInterfaceIndexGet
(
    IN  GT_U8     devNum,
    OUT GT_BOOL   *enablePtr,
    OUT GT_U32    *indexPtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexGet(devNum, enablePtr, indexPtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
     /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enablePtr);
    TGF_PARAM_NOT_USED(indexPtr);

    return GT_NOT_IMPLEMENTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet function
* @endinternal
*
* @brief   This function sets analyzer interface index, used for ingress
*         mirroring for Port-Based hop-by-hop mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet
(
    IN GT_BOOL   enable,
    IN GT_U32    index
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
        rc = cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexSet(devNum, enable, index);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(enable);
    TGF_PARAM_NOT_USED(index);

    return GT_NOT_IMPLEMENTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfMirrorRxGlobalAnalyzerInterfaceIndexGet function
* @endinternal
*
* @brief   This function gets analyzer interface index, used for ingress
*         mirroring for Port-Based hop-by-hop mode.
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - Pointer to global enable/disable mirroring for
*                                      Port-Based hop-by-hop mode.
*                                      - GT_TRUE - enable mirroring.
*                                      - GT_FALSE - No mirroring.
* @param[out] indexPtr                 - pointer to analyzer destination interface index.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfMirrorRxGlobalAnalyzerInterfaceIndexGet
(
    IN  GT_U8     devNum,
    OUT GT_BOOL   *enablePtr,
    OUT GT_U32    *indexPtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexGet(devNum, enablePtr, indexPtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
     /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enablePtr);
    TGF_PARAM_NOT_USED(indexPtr);

    return GT_NOT_IMPLEMENTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

static GT_BOOL  prvTgfMirrorAnalyzerInterfaceTypeIsTxMirror = GT_FALSE;

/**
* @internal prvTgfMirrorAnalyzerInterfaceTypeSet function
* @endinternal
*
* @brief   This function sets analyzer interface type. relevant only to ExMxPm.
*
* @param[in] isTxMirror               - is Tx mirror ?
*                                      GT_TRUE : TX mirror
*                                      GT_FALSE : RX mirror
*
* @retval GT_OK                    - on success.
*/
GT_STATUS prvTgfMirrorAnalyzerInterfaceTypeSet
(
    IN GT_BOOL  isTxMirror
)
{
    prvTgfMirrorAnalyzerInterfaceTypeIsTxMirror = isTxMirror;
    return GT_OK;
}

/**
* @internal prvTgfMirrorAnalyzerInterfaceSet function
* @endinternal
*
* @brief   This function sets analyzer interface.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, index, interface type.
* @retval GT_OUT_OF_RANGE          - on wrong port or device number in interfacePtr.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfMirrorAnalyzerInterfaceSet
(
    IN GT_U32    index,
    IN PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC   *interfacePtr
)
{

#ifdef CHX_FAMILY
    CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC cpssInterface;
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    cpssInterface.interface.type = interfacePtr->interface.type;
    if( CPSS_INTERFACE_PORT_E == interfacePtr->interface.type )
    {
        /* need to remove casting and to fix code after lion2 development is done */
        CPSS_TBD_BOOKMARK_EARCH

        rc = prvUtfHwDeviceNumberGet((GT_U8)interfacePtr->interface.devPort.hwDevNum,
                                     &cpssInterface.interface.devPort.hwDevNum);
        if(GT_OK != rc)
            return rc;
    }
    cpssInterface.interface.devPort.portNum = interfacePtr->interface.devPort.portNum;

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChMirrorAnalyzerInterfaceSet(devNum, index, &cpssInterface);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChMirrorAnalyzerInterfaceSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_OUTLIF_INFO_STC cpssInterface;
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
    CPSS_EXMXPM_MIRROR_TYPE_ENT   mirrorType =
        prvTgfMirrorAnalyzerInterfaceTypeIsTxMirror == GT_FALSE ?
            CPSS_EXMXPM_MIRROR_TYPE_INGRESS_E:
            CPSS_EXMXPM_MIRROR_TYPE_EGRESS_E;

    if(index != 0)
    {
        /* the device supports single analyzer for RX/TX mirroring */
        return GT_NOT_SUPPORTED;
    }

    cpssInterface.outlifType = CPSS_EXMXPM_OUTLIF_TYPE_LL_E;
    cpssInterface.interfaceInfo.type = interfacePtr->interface.type;
    if( CPSS_INTERFACE_PORT_E == interfacePtr->interface.type )
    {
        /* need to remove casting and to fix code after lion2 development is done */
        CPSS_TBD_BOOKMARK_EARCH

        rc = prvUtfHwDeviceNumberGet((GT_U8)interfacePtr->interface.devPort.hwDevNum,
                                     &cpssInterface.interfaceInfo.devPort.hwDevNum);
        if(GT_OK != rc)
            return rc;
    }
    cpssInterface.interfaceInfo.devPort.portNum = interfacePtr->interface.devPort.portNum;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        if(mirrorType != CPSS_EXMXPM_MIRROR_TYPE_INGRESS_POLICY_E && index != 0)
        {
            /* explicit note : this device no supports multiple analyzers on ingress/egress mirror
               (supported only on PCL mirror) */
            return GT_BAD_PARAM;
        }
        /* call device specific API */
        rc = cpssExMxPmMirrorDestInterfaceSet(devNum, mirrorType , index, &cpssInterface);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChMirrorAnalyzerInterfaceSet FAILED, rc = [%d]", rc);

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
* @internal prvTgfMirrorAnalyzerInterfaceGet function
* @endinternal
*
* @brief   This function gets analyzer interface.
*
* @param[in] devNum                   - device number.
* @param[in] index                    -  of analyzer interface. (0 - 6)
*
* @param[out] interfacePtr             - Pointer to analyzer interface.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, index.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfMirrorAnalyzerInterfaceGet
(
    IN  GT_U8     devNum,
    IN GT_U32     index,
    OUT PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC   *interfacePtr
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC cpssInterface;
    GT_STATUS   rc;

    /* call device specific API */
    rc = cpssDxChMirrorAnalyzerInterfaceGet(devNum, index, &cpssInterface);
    if(rc != GT_OK)
    {
        return rc;
    }

    interfacePtr->interface.type = cpssInterface.interface.type;
    if( CPSS_INTERFACE_PORT_E == cpssInterface.interface.type )
    {
        /* need to remove casting and to fix code after lion2 development is done */
        CPSS_TBD_BOOKMARK_EARCH
        rc = prvUtfSwFromHwDeviceNumberGet(cpssInterface.interface.devPort.hwDevNum,
                                           &interfacePtr->interface.devPort.hwDevNum);
        if(GT_OK != rc)
            return rc;
    }
    interfacePtr->interface.devPort.portNum = cpssInterface.interface.devPort.portNum;

    return GT_OK;

#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_OUTLIF_INFO_STC cpssInterface;
    GT_STATUS   rc;
    CPSS_EXMXPM_MIRROR_TYPE_ENT   mirrorType =
        prvTgfMirrorAnalyzerInterfaceTypeIsTxMirror == GT_FALSE ?
            CPSS_EXMXPM_MIRROR_TYPE_INGRESS_E:
            CPSS_EXMXPM_MIRROR_TYPE_EGRESS_E;

    if(index != 0)
    {
        /* the device supports single analyzer for RX/TX mirroring */
        return GT_NOT_SUPPORTED;
    }

    /* call device specific API */
    rc = cpssExMxPmMirrorDestInterfaceGet(devNum, mirrorType, index , &cpssInterface);
    if(rc != GT_OK)
    {
        return rc;
    }

    interfacePtr->interface.type = cpssInterface.interfaceInfo.type;
    if( CPSS_INTERFACE_PORT_E == cpssInterface.interfaceInfo.type )
    {
        /* need to remove casting and to fix code after lion2 development is done */
        CPSS_TBD_BOOKMARK_EARCH
        rc = prvUtfSwFromHwDeviceNumberGet(cpssInterface.interfaceInfo.devPort.hwDevNum,
                                           &interfacePtr->interface.devPort.hwDevNum);
        if(GT_OK != rc)
            return rc;
    }
    interfacePtr->interface.devPort.portNum = cpssInterface.interfaceInfo.devPort.portNum;

    return GT_OK;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfMirrorToAnalyzerForwardingModeSet function
* @endinternal
*
* @brief   Set Forwarding mode to Analyzer for egress/ingress mirroring.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, mode.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To change Forwarding mode, applicaton should
*       disable Rx/Tx mirrorred ports.
*
*/
GT_STATUS prvTgfMirrorToAnalyzerForwardingModeSet
(
    IN PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT   mode
)
{

#ifdef CHX_FAMILY
    CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT   cpssMode;
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    switch(mode)
    {
        case PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E:
            cpssMode = CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E;
            break;
        case PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E:
            cpssMode = CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_OVERRIDE_E;
            break;
        case PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E:
            cpssMode = CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E;
            break;

        default:
            return GT_BAD_PARAM;
    }
    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            /* allow the test to keep calling the API although can not change
               the actual value from 'End-to-End' */
            continue;
        }
        /* call device specific API */
        rc = cpssDxChMirrorToAnalyzerForwardingModeSet(devNum, cpssMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChMirrorToAnalyzerForwardingModeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(mode);

    return GT_NOT_IMPLEMENTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfMirrorToAnalyzerForwardingModeGet function
* @endinternal
*
* @brief   Get Forwarding mode to Analyzer for egress/ingress mirroring.
*
* @param[in] devNum                   - device number.
*
* @param[out] modePtr                  - pointer to mode of forwarding To Analyzer packets.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfMirrorToAnalyzerForwardingModeGet
(
    IN  GT_U8     devNum,
    OUT PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT   *modePtr
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT   mode;
    GT_STATUS rc;
    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* allow the test to keep calling the API although can not change
           the actual value from 'End-to-End' */
        *modePtr = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E;
        return GT_OK;
    }

    /* call device specific API */
    rc =  cpssDxChMirrorToAnalyzerForwardingModeGet(devNum, &mode);
    if(rc != GT_OK)
    {
        return rc;
    }

    *modePtr = (mode == CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E) ?
        PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E :
               (mode == CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E) ?
        PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E :
        PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E ;

    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
     /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(modePtr);

    return GT_NOT_IMPLEMENTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfMirrorRxPortSet function
* @endinternal
*
* @brief   Sets a specific port to be Rx mirrored port.
*         NOTE: this is 'eport' mirroring for eArch devices.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, mirrPort.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @param[in] devNum       - the device number
* @param[in] mirrPort  - port number, CPU port supported.
* @param[in] enable    - enable/disable Rx mirror on this port
*                        GT_TRUE - Rx mirroring enabled, packets
*                                  received on a mirrPort are
*                                  mirrored to Rx analyzer.
*                        GT_FALSE - Rx mirroring disabled.
* @param[in] index     - Analyzer destination interface index. (0 - 6)
*                        Supported for DxChXcat and above device.
*                        In xCat3 the parameter is used only if
*                        forwarding mode to analyzer is Source-based.
*/
GT_STATUS prvTgfMirrorRxPortSet
(
    IN  GT_PORT_NUM    mirrPort,
    IN  GT_BOOL  enable,
    IN  GT_U32   index
)
{
    return
        prvTgfMirrorRxPortTypeSet(mirrPort,GT_FALSE/*ePort*/,enable,index);
}

/**
* @internal prvTgfMirrorRxPortGet function
* @endinternal
*
* @brief   Gets status of Rx mirroring (enabled or disabled) of specific port
*         NOTE: this is 'eport' mirroring for eArch devices.
*
* @param[in] mirrPort   - port number, CPU port supported.
*
* @param[out] enablePtr - (pointer to) Rx mirror mode
*                         GT_TRUE - Rx mirroring enabled, packets
*                                   received on a mirrPort are
*                                   mirrored to Rx analyzer.
*                         GT_FALSE - Rx mirroring disabled.
* @param[out] indexPtr  - (pointer to) Analyzer destination interface index. (0 - 6)
*                         Supported for DxChXcat and above device.
*                         In xCat3 the parameter is used only if
*                         forwarding mode to analyzer is Source-based.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, mirrPort.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfMirrorRxPortGet
(
    IN  GT_PORT_NUM    mirrPort,
    OUT GT_BOOL  *enablePtr,
    OUT GT_U32   *indexPtr
)
{
    return
        prvTgfMirrorRxPortTypeGet(mirrPort, GT_FALSE/*ePort*/,
                                   enablePtr, indexPtr);
}

/**
* @internal prvTgfMirrorTxPortSet function
* @endinternal
*
* @brief   Sets a specific port to be Tx mirrored port.
*         NOTE: this is 'physical port' mirroring for eArch devices.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, mirrPort.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @param[in] mirrPort - port number, CPU port supported.
* @param[in] enable   - enable/disable Tx mirror on this port
*                        GT_TRUE - Tx mirroring enabled, packets
*                                  transmitted from a mirrPort are
*                                  mirrored to Tx analyzer.
*                        GT_FALSE - Tx mirroring disabled.
* @param[in] index    - Analyzer destination interface index. (0 - 6)
*                       Supported for DxChXcat and above device.
*                       In xCat, xCat2, xCat3 the parameter is used only if
*                       forwarding mode to analyzer is Source-based.
*/
GT_STATUS prvTgfMirrorTxPortSet
(
    IN  GT_PORT_NUM    mirrPort,
    IN  GT_BOOL  enable,
    IN  GT_U32   index
)
{
    return
        prvTgfMirrorTxPortTypeSet(mirrPort, GT_TRUE, enable, index);
}

/**
* @internal prvTgfMirrorTxPortGet function
* @endinternal
*
* @brief   Get status (enabled/disabled) of Tx mirroring per port .
*         NOTE: this is 'physical port' mirroring for eArch devices.
*
* @param[in] mirrPort   - port number.
*
* @param[out] enablePtr - (pointer to) Tx mirror mode on this port
*                         GT_TRUE - Tx mirroring enabled, packets
*                                   transmitted from a mirrPort are
*                                   mirrored to Tx analyzer.
*                         GT_FALSE - Tx mirroring disabled.

* @param[out] indexPtr  - (pointer to) Analyzer destination interface index. (0 - 6)
*                         Supported for DxChXcat and above device.
*                         In xCat3 the parameter is used only if
*                         forwarding mode to analyzer is Source-based.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or mirrPort.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfMirrorTxPortGet
(
    IN  GT_PORT_NUM    mirrPort,
    OUT GT_BOOL  *enablePtr,
    OUT GT_U32   *indexPtr
)
{
    return prvTgfMirrorTxPortTypeGet(mirrPort, GT_TRUE,
                                   enablePtr, indexPtr);
}
/**
* @internal prvTgfMirrorRxAnalyzerPortSet function
* @endinternal
*
* @brief   Sets a specific port to be an analyzer port of Rx mirrored
*         ports
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device number
* @retval GT_HW_ERROR              - on writing to HW error
*/
GT_STATUS prvTgfMirrorRxAnalyzerPortSet
(
    IN  GT_PORT_NUM    analyzerPort,
    IN  GT_U8    analyzerdev
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
#ifdef CHX_FAMILY
    CPSS_PP_FAMILY_TYPE_ENT                     devFamily;
    CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC     analyzerInf;
    GT_HW_DEV_NUM                               analyzerHwDev;
#endif /*CHX_FAMILY*/
#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_MIRROR_TYPE_ENT   mirrorType     = CPSS_EXMXPM_MIRROR_TYPE_INGRESS_E;
    GT_U32                        pclAnalyzerNum = 0;
    CPSS_EXMXPM_OUTLIF_INFO_STC   interfaceInfo  = {0};
#endif /* EXMXPM_FAMILY */

#ifdef CHX_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        rc = prvUtfDeviceFamilyGet(devNum, &devFamily);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvUtfDeviceFamilyGet FAILED, rc = [%d]", rc);
            return rc;
        }

        /* call device specific API */
        if (devFamily < CPSS_PP_FAMILY_DXCH_XCAT_E)
        {
            rc = prvUtfHwDeviceNumberGet(analyzerdev, &analyzerHwDev);
            if(GT_OK != rc)
                return rc;
            rc = cpssDxChMirrorRxAnalyzerPortSet(devNum, analyzerPort, analyzerHwDev);
        }
        else
        {
            analyzerInf.interface.type = CPSS_INTERFACE_PORT_E;
            /* need to remove casting and to fix code after lion2 development is done */
            CPSS_TBD_BOOKMARK_EARCH
            rc = prvUtfHwDeviceNumberGet(analyzerdev,
                                         &analyzerInf.interface.devPort.hwDevNum);
            if(GT_OK != rc)
                return rc;
            analyzerInf.interface.devPort.portNum = analyzerPort;

            /* use interface 0 for Rx analyzer by default */
            rc = cpssDxChMirrorAnalyzerInterfaceSet(devNum, 0, &analyzerInf);
        }

        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChMirrorRxAnalyzerPortSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* set mirror type */
    mirrorType     = CPSS_EXMXPM_MIRROR_TYPE_INGRESS_E;
    /* PCL analyzer is not used */
    pclAnalyzerNum = 0;

    /* set interface info */
    interfaceInfo.outlifType = CPSS_EXMXPM_OUTLIF_TYPE_LL_E;

    interfaceInfo.interfaceInfo.type            = CPSS_INTERFACE_PORT_E;
    interfaceInfo.interfaceInfo.devPort.hwDevNum  = analyzerdev;
    interfaceInfo.interfaceInfo.devPort.portNum = analyzerPort;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmMirrorDestInterfaceSet(devNum, mirrorType, pclAnalyzerNum, &interfaceInfo);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmMirrorDestInterfaceSet FAILED, rc = [%d]", rc);

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
* @internal prvTgfMirrorRxAnalyzerPortGet function
* @endinternal
*
* @brief   Gets the analyzer port of Rx mirrored ports
*
* @param[in] devNum                   - the device number
*
* @param[out] analyzerPortPtr          - (pointer to) port number of analyzer port
* @param[out] analyzerDevPtr           - (pointer to) pp Device number of analyzer port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device number
* @retval GT_HW_ERROR              - on writing to HW error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvTgfMirrorRxAnalyzerPortGet
(
    IN  GT_U8                         devNum,
    OUT GT_U8                        *analyzerPortPtr,
    OUT GT_U8                        *analyzerDevPtr
)
{
#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_MIRROR_TYPE_ENT   mirrorType     = CPSS_EXMXPM_MIRROR_TYPE_INGRESS_E;
    GT_U32                        pclAnalyzerNum = 0;
    CPSS_EXMXPM_OUTLIF_INFO_STC   interfaceInfo  = {0};
    GT_STATUS                     rc             = GT_OK;
#endif /* EXMXPM_FAMILY */

#ifdef CHX_FAMILY
    CPSS_PP_FAMILY_TYPE_ENT                     devFamily;
    CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC     analyzerInf;
    GT_STATUS                                   rc;
    GT_PORT_NUM                                 tmpPort;
    GT_HW_DEV_NUM                               tmpHwDev;
#endif /*CHX_FAMILY*/

#ifdef CHX_FAMILY
    rc = prvUtfDeviceFamilyGet(devNum, &devFamily);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvUtfDeviceFamilyGet FAILED, rc = [%d]", rc);
        return rc;
    }

    /* call device specific API */
    if (devFamily < CPSS_PP_FAMILY_DXCH_XCAT_E)
    {
        rc = cpssDxChMirrorRxAnalyzerPortGet(devNum, &tmpPort, &tmpHwDev);
        *analyzerPortPtr = (GT_U8)tmpPort;
        *analyzerDevPtr  = (GT_U8)tmpHwDev;
    }
    else
    {
        /* use interface 0 for Rx analyzer by default */
        rc = cpssDxChMirrorAnalyzerInterfaceGet(devNum, 0, &analyzerInf);
        if(GT_OK != rc)
            return rc;
        if( CPSS_INTERFACE_PORT_E == analyzerInf.interface.type )
        {
            /* need to remove casting and to fix code after lion2 development is done */
            CPSS_TBD_BOOKMARK_EARCH
            rc = prvUtfSwFromHwDeviceNumberGet(analyzerInf.interface.devPort.hwDevNum,
                                               (GT_U32 *)analyzerDevPtr);
            if(GT_OK != rc)
                return rc;
        }

        /* need to remove casting and to fix code after lion2 development is done
         analyzerPortPtr should also be of type GT_PORT_NUM */
        CPSS_TBD_BOOKMARK_EARCH

        *analyzerPortPtr = (GT_U8)analyzerInf.interface.devPort.portNum;

    }
    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* set mirror type */
    mirrorType     = CPSS_EXMXPM_MIRROR_TYPE_INGRESS_E;
    /* PCL analyzer is not used */
    pclAnalyzerNum = 0;

    /* call device specific API */
    rc = cpssExMxPmMirrorDestInterfaceGet(devNum, mirrorType, pclAnalyzerNum, &interfaceInfo);
    if (GT_OK == rc)
    {
        if ((CPSS_EXMXPM_OUTLIF_TYPE_LL_E == interfaceInfo.outlifType) &&
            (CPSS_INTERFACE_PORT_E == interfaceInfo.interfaceInfo.type))
        {
            *analyzerDevPtr  = (GT_U8)interfaceInfo.interfaceInfo.devPort.hwDevNum;
            *analyzerPortPtr = (GT_U8)interfaceInfo.interfaceInfo.devPort.portNum;
        }
        else
        {
            rc = GT_FAIL;
        }
    }

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfMirrorPclAnalyzerPortSet function
* @endinternal
*
* @brief   Sets a specific port to be an analyzer port of Rx mirrored
*         ports
* @param[in] analyzerIndex            - Analyzer index 1..7
* @param[in] analyzerPort             - port number of analyzer port
* @param[in] analyzerdev              - pp Device number of analyzer port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device number
* @retval GT_HW_ERROR              - on writing to HW error
*/
GT_STATUS prvTgfMirrorPclAnalyzerPortSet
(
    IN  GT_U32         analyzerIndex,
    IN  GT_PORT_NUM    analyzerPort,
    IN  GT_U8          analyzerdev
)
{
#ifdef EXMXPM_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
    CPSS_EXMXPM_MIRROR_TYPE_ENT   mirrorType     = CPSS_EXMXPM_MIRROR_TYPE_INGRESS_E;
    CPSS_EXMXPM_OUTLIF_INFO_STC   interfaceInfo  = {0};
#endif /* EXMXPM_FAMILY */

#ifdef CHX_FAMILY
    TGF_PARAM_NOT_USED(analyzerIndex);
    return prvTgfMirrorRxAnalyzerPortSet(
        analyzerPort, analyzerdev);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* set mirror type */
    mirrorType     = CPSS_EXMXPM_MIRROR_TYPE_INGRESS_POLICY_E;
    /* set interface info */
    interfaceInfo.outlifType = CPSS_EXMXPM_OUTLIF_TYPE_LL_E;

    interfaceInfo.interfaceInfo.type            = CPSS_INTERFACE_PORT_E;
    interfaceInfo.interfaceInfo.devPort.hwDevNum  = analyzerdev;
    interfaceInfo.interfaceInfo.devPort.portNum = analyzerPort;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmMirrorDestInterfaceSet(
            devNum, mirrorType, analyzerIndex, &interfaceInfo);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmMirrorDestInterfaceSet FAILED, rc = [%d]", rc);

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
* @internal prvTgfMirrorAnalyzerVlanTagRemoveEnableSet function
* @endinternal
*
* @brief   Enable/Disable VLAN tag removal of mirrored traffic.
*         When VLAN tag removal is enabled for a specific analyzer port, all
*         packets that are mirrored to this port are sent without any VLAN tags.
*
* @note   APPLICABLE DEVICES:      Lion and above
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfMirrorAnalyzerVlanTagRemoveEnableSet
(
    IN  GT_PORT_NUM    portNum,
    IN GT_BOOL   enable
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
        rc = cpssDxChMirrorAnalyzerVlanTagRemoveEnableSet(devNum, portNum, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChMirrorAnalyzerVlanTagRemoveEnableSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(enable);
    TGF_PARAM_NOT_USED(portNum);

    return GT_NOT_IMPLEMENTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

}

/**
* @internal prvTgfMirrorPortEgressAnalyzerIndexSet function
* @endinternal
*
* @brief   Set the analyzer index for the egress mirrored traffic per egress port.
*         The packet will be either mirrored due to Vlan egress mirroring or
*         mirroring to CPU or due to STC mirroring or egress mirrored traffic
*         per egress port according to highest value of the analyzer index.
* @param[in] portNum                  - port number
* @param[in] index                    - analyzer  for the egress port.
*                                      (APPLICABLE RANGES: 0..7)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device number, port number
* @retval GT_OUT_OF_RANGE          - wrong index
* @retval GT_HW_ERROR              - on writing to HW error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfMirrorPortEgressAnalyzerIndexSet
(
    IN GT_U32       portNum,
    IN GT_U32      index
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
        rc = cpssExMxPmMirrorPortEgressAnalyzerIndexSet(devNum, portNum, index);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmMirrorPortEgressAnalyzerIndexSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
    return rc1;

#else /* !EXMXPM_FAMILY */

    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(index);

    return GT_NOT_SUPPORTED;
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfMirrorEgressAnalyzerIndexSet function
* @endinternal
*
* @brief   Set the analyzer index for the egress mirrored traffic.
*         The packet will be either mirrored due to Vlan egress mirroring or
*         mirroring to CPU or due to STC mirroring or egress mirrored traffic
*         per egress port according to highest value of the analyzer index.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device number
* @retval GT_OUT_OF_RANGE          - wrong vlanAnalyzerIndex,
*                                       mirrorToCpuAnalyzerIndex, stcAnalyzerIndex
* @retval GT_HW_ERROR              - on writing to HW error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfMirrorEgressAnalyzerIndexSet
(
    IN GT_U32      vlanAnalyzerIndex,
    IN GT_U32      mirrorToCpuAnalyzerIndex,
    IN GT_U32      stcAnalyzerIndex
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
        rc = cpssExMxPmMirrorEgressAnalyzerIndexSet(devNum, vlanAnalyzerIndex,
                                                    mirrorToCpuAnalyzerIndex,
                                                    stcAnalyzerIndex);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmMirrorEgressAnalyzerIndexSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }
    return rc1;

#else /* !EXMXPM_FAMILY */

    TGF_PARAM_NOT_USED(vlanAnalyzerIndex);
    TGF_PARAM_NOT_USED(mirrorToCpuAnalyzerIndex);
    TGF_PARAM_NOT_USED(stcAnalyzerIndex);

    return GT_NOT_SUPPORTED;
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfMirrorEgressAnalyzerIndexGet function
* @endinternal
*
* @brief   Get the analyzer index for the egress mirrored traffic.
*         The packet will be either mirrored due to Vlan egress mirroring or
*         mirroring to CPU or due to STC mirroring or egress mirrored traffic
*         per egress port according to highest value of the analyzer index.
* @param[in] devNum                   - device number.
*
* @param[out] vlanAnalyzerIndexPtr     - pointer to the analyzer index attached
*                                      to packets which need to be duplicated
*                                      due to Vlan egress mirroring.
* @param[out] mirrorToCpuAnalyzerIndexPtr - The analyzer index attached to packets
*                                      which need to be duplicated due
*                                      to mirroring to CPU.
* @param[out] stcAnalyzerIndexPtr      - pointer to analyzer index
*                                      for the STC mirrored traffic.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on writing to HW error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfMirrorEgressAnalyzerIndexGet
(
    IN  GT_U8       devNum,
    OUT GT_U32      *vlanAnalyzerIndexPtr,
    OUT GT_U32      *mirrorToCpuAnalyzerIndexPtr,
    OUT GT_U32      *stcAnalyzerIndexPtr
)
{
#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmMirrorEgressAnalyzerIndexGet(devNum, vlanAnalyzerIndexPtr,
                                                  mirrorToCpuAnalyzerIndexPtr,
                                                  stcAnalyzerIndexPtr);

#else /* !EXMXPM_FAMILY */

    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(vlanAnalyzerIndexPtr);
    TGF_PARAM_NOT_USED(mirrorToCpuAnalyzerIndexPtr);
    TGF_PARAM_NOT_USED(stcAnalyzerIndexPtr);

    return GT_NOT_SUPPORTED;
#endif /* CHX_FAMILY */

}

/**
* @internal prvTgfMirrorRxPortTypeSet function
* @endinternal
*
* @brief   Sets a specific port/eport to be Rx mirrored port.
*         INPUTS:
*         mirrPort - port number, CPU port supported.
*         isPhysicalPort - defines type of mirrPort parameter to be either
*         physical port or ePort.
*         GT_TRUE - mirrPort is physical port
*         GT_FALSE - mirrPort is ePort
* @param[in] mirrPort                 - port number, CPU port supported.
* @param[in] isPhysicalPort           - defines type of mirrPort parameter to be either
*                                      physical port or ePort.
*                                      GT_TRUE  - mirrPort is physical port
*                                      GT_FALSE - mirrPort is ePort
*                                      APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*                                      Ignored by other devices, mirrPort is always physical one.
* @param[in] enable                   - enable/disable Rx mirror on this port
*                                      GT_TRUE - Rx mirroring enabled, packets
*                                      received on a mirrPort are
*                                      mirrored to Rx analyzer.
*                                      GT_FALSE - Rx mirroring disabled.
* @param[in] index                    - Analyzer destination interface index. (0 - 6)- 6)
*                                      Supported for DxChXcat and above device.
*                                      In xCat3 the parameter is used only if
*                                      forwarding mode to analyzer is Source-based.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, mirrPort.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfMirrorRxPortTypeSet
(
    IN  GT_PORT_NUM    mirrPort,
    IN  GT_BOOL  isPhysicalPort,
    IN  GT_BOOL  enable,
    IN  GT_U32   index
)
{
    GT_U32  portIter = 0;
    GT_U8   devNum;


    /* find port index */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        if (prvTgfPortsArray[portIter] == mirrPort)
        {
            break;
        }
    }

    /* check if port is valid */
    if (prvTgfPortsNum >= mirrPort)
    {
        devNum = prvTgfDevsArray[portIter];
    }
    else
    {
        /* support ePort src re-assignment , so the eport is not in the list of ports */
        devNum = prvTgfDevNum;
    }

#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChMirrorRxPortSet(devNum, mirrPort,
                                   isPhysicalPort ,
                                   enable, index);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    TGF_PARAM_NOT_USED(isPhysicalPort);
    TGF_PARAM_NOT_USED(index);
    /* call device specific API */
    return cpssExMxPmMirrorPortEnableSet(devNum, (GT_U8)mirrPort,
                                         CPSS_EXMXPM_MIRROR_TYPE_INGRESS_E, enable);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfMirrorRxPortTypeGet function
* @endinternal
*
* @brief   Gets status of Rx mirroring (enabled or disabled) of specific port/eport
*         INPUTS:
*         mirrPort - port number, CPU port supported.
*         isPhysicalPort - defines type of mirrPort parameter to be either
*         physical port or ePort.
*         GT_TRUE - mirrPort is physical port
*         GT_FALSE - mirrPort is ePort
* @param[in] mirrPort                 - port number, CPU port supported.
* @param[in] isPhysicalPort           - defines type of mirrPort parameter to be either
*                                      physical port or ePort.
*                                      GT_TRUE  - mirrPort is physical port
*                                      GT_FALSE - mirrPort is ePort
*                                      APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*                                      Ignored by other devices, mirrPort is always physical one.
*
* @param[out] enablePtr                - (pointer to) Rx mirror mode
*                                         GT_TRUE - Rx mirroring enabled, packets
*                                                   received on a mirrPort are
*                                                   mirrored to Rx analyzer.
*                                         GT_FALSE - Rx mirroring disabled.
* @param[out] indexPtr                 - (pointer to) Analyzer destination interface index. (0 - 6)
*                                        Supported for DxChXcat and above device.
*                                        In xCat3 the parameter is used only if
*                                        forwarding mode to analyzer is Source-based.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, mirrPort.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfMirrorRxPortTypeGet
(
    IN  GT_PORT_NUM    mirrPort,
    IN  GT_BOOL  isPhysicalPort,
    OUT GT_BOOL  *enablePtr,
    OUT GT_U32   *indexPtr
)
{
    GT_U32  portIter = 0;


    /* find port index */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        if (prvTgfPortsArray[portIter] == mirrPort)
        {
            break;
        }
    }

    /* check if port is valid */
    if (prvTgfPortsArray[portIter] != mirrPort)
    {
        PRV_UTF_LOG1_MAC("[TGF]: Port [%d] is not valid", mirrPort);

        return GT_BAD_PARAM;
    }

#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChMirrorRxPortGet(prvTgfDevsArray[portIter], mirrPort, isPhysicalPort,
                                   enablePtr, indexPtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    TGF_PARAM_NOT_USED(isPhysicalPort);
    TGF_PARAM_NOT_USED(indexPtr);
    /* call device specific API */
    return cpssExMxPmMirrorPortEnableGet(prvTgfDevsArray[portIter], (GT_U8)mirrPort,
                                         CPSS_EXMXPM_MIRROR_TYPE_INGRESS_E, enablePtr);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}


/**
* @internal prvTgfMirrorTxPortTypeSet function
* @endinternal
*
* @brief   Sets a specific port/eport to be Tx mirrored port.
*         INPUTS:
*         mirrPort - port number, CPU port supported.
*         isPhysicalPort - defines type of mirrPort parameter to be either
*         physical port or ePort.
*         GT_TRUE - mirrPort is physical port
*         GT_FALSE - mirrPort is ePort
* @param[in] mirrPort                 - port number, CPU port supported.
* @param[in] isPhysicalPort           - defines type of mirrPort parameter to be either
*                                      physical port or ePort.
*                                      GT_TRUE  - mirrPort is physical port
*                                      GT_FALSE - mirrPort is ePort
*                                      APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*                                      Ignored by other devices, mirrPort is always physical one.
* @param[in] enable                   - enable/disable Tx mirror on this port
*                                      GT_TRUE - Tx mirroring enabled, packets
*                                      transmitted from a mirrPort are
*                                      mirrored to Tx analyzer.
*                                      GT_FALSE - Tx mirroring disabled.
* @param[in] index                    - Analyzer destination interface index. (0 - 6)- 6)
*                                      Supported for DxChXcat and above device.
*                                      Used only if forwarding mode to analyzer is Source-based.
*                                      In xCat3 the parameter is used only if
*                                      forwarding mode to analyzer is Source-based.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, mirrPort.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfMirrorTxPortTypeSet
(
    IN  GT_PORT_NUM    mirrPort,
    IN  GT_BOOL  isPhysicalPort,
    IN  GT_BOOL  enable,
    IN  GT_U32   index
)
{
    GT_U32  portIter = 0;


    if(isPhysicalPort == GT_TRUE)
    {
        /* find port index */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            if (prvTgfPortsArray[portIter] == mirrPort)
            {
                break;
            }
        }

        /* check if port is valid */
        if (prvTgfPortsArray[portIter] != mirrPort)
        {
            PRV_UTF_LOG1_MAC("[TGF]: Port [%d] is not valid", mirrPort);

            return GT_BAD_PARAM;
        }
    }
    else
    {
        portIter = 0;
    }

#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChMirrorTxPortSet(prvTgfDevsArray[portIter], mirrPort, isPhysicalPort, enable, index);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    TGF_PARAM_NOT_USED(isPhysicalPort);
    TGF_PARAM_NOT_USED(index);
    /* call device specific API */
    return cpssExMxPmMirrorPortEnableSet(prvTgfDevsArray[portIter], (GT_U8)mirrPort,
                                         CPSS_EXMXPM_MIRROR_TYPE_EGRESS_E, enable);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfMirrorTxPortTypeGet function
* @endinternal
*
* @brief   Get status (enabled/disabled) of Tx mirroring per port/eport .
*         INPUTS:
*         mirrPort - port number.
*         isPhysicalPort - defines type of mirrPort parameter to be either
*         physical port or ePort.
*         GT_TRUE - mirrPort is physical port
*         GT_FALSE - mirrPort is ePort
*
* @param[in] mirrPort                 - port number.
* @param[in] isPhysicalPort           - defines type of mirrPort parameter to be either
*                                      physical port or ePort.
*                                      GT_TRUE  - mirrPort is physical port
*                                      GT_FALSE - mirrPort is ePort
*                                      APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*                                      Ignored by other devices, mirrPort is always physical one.
*
* @param[out] enablePtr                - (pointer to) Tx mirror mode on this port
*                                        GT_TRUE - Tx mirroring enabled, packets
*                                                  transmitted from a mirrPort are
*                                                  mirrored to Tx analyzer.
*                                        GT_FALSE - Tx mirroring disabled.

* @param[out] indexPtr                 - (pointer to) Analyzer destination interface index. (0 - 6)
*                                        Supported for DxChXcat and above device.
*                                        In xCat3 the parameter is used only if
*                                        forwarding mode to analyzer is Source-based.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or mirrPort.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfMirrorTxPortTypeGet
(
    IN  GT_PORT_NUM    mirrPort,
    IN  GT_BOOL  isPhysicalPort,
    OUT GT_BOOL  *enablePtr,
    OUT GT_U32   *indexPtr
)
{
    GT_U32  portIter = 0;


    /* find port index */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        if (prvTgfPortsArray[portIter] == mirrPort)
        {
            break;
        }
    }

    /* check if port is valid */
    if (prvTgfPortsArray[portIter] != mirrPort)
    {
        PRV_UTF_LOG1_MAC("[TGF]: Port [%d] is not valid", mirrPort);

        return GT_BAD_PARAM;
    }

#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChMirrorTxPortGet(prvTgfDevsArray[portIter], mirrPort, isPhysicalPort,
                                   enablePtr, indexPtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    TGF_PARAM_NOT_USED(isPhysicalPort);
    TGF_PARAM_NOT_USED(indexPtr);
    /* call device specific API */
    return cpssExMxPmMirrorPortEnableGet(prvTgfDevsArray[portIter], (GT_U8)mirrPort,
                                         CPSS_EXMXPM_MIRROR_TYPE_EGRESS_E, enablePtr);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfMirrorTxPortVlanEnableSet function
* @endinternal
*
* @brief   Enable or disable Tx vlan mirroring from specific physical port.
*         (define if the physical port allow/deny egress vlan mirroring)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - physical port number.
* @param[in] enable                   - enable/disable Tx vlan mirroring from this physical port
*                                      GT_TRUE  - Tx vlan mirroring enabled  for this physical port.
*                                      GT_FALSE - Tx vlan mirroring disabled for this physical port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device , bad portNum.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfMirrorTxPortVlanEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChMirrorTxPortVlanEnableSet(devNum, portNum, enable);
#else
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(enable);

    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}
