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
* @file tgfMplsGen.c
*
* @brief Generic API implementation for MPLS
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
#include <common/tgfIpGen.h>
#include <common/tgfMplsGen.h>

/******************************************************************************\
 *                              Marco definitions                             *
\******************************************************************************/

/* max counter field's name length */
#define PRV_TGF_MAX_FIELD_NAME_LEN_CNS 25

/******************************************************************************\
 *                     Private function's implementation                      *
\******************************************************************************/

/**
* @internal prvTgfMplsLsrEnableSet function
* @endinternal
*
* @brief   Enable MPLS LSR(Label Switch Router) switching
*
* @param[in] devNum                   - device number
* @param[in] enable                   - enable/disable feature
*                                      GT_FALSE - MPLS LSR switching is bypassed
*                                      GT_TRUE  - MPLS LSR switching is enabled
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfMplsLsrEnableSet
(
    IN GT_U8        devNum,
    IN GT_BOOL      enable
)
{
#ifdef EXMXPM_FAMILY

    return cpssExMxPmMplsEnableSet(devNum, enable);

#else /* !EXMXPM_FAMILY */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enable);
    /* this request is not support */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */
}

/**
* @internal prvTgfMplsLsrEnableGet function
* @endinternal
*
* @brief   Get MPLS LSR(Label Switch Router) switching state
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) MPLS Engine state
*                                      GT_FALSE - MPLS LSR switching is bypassed
*                                      GT_TRUE  - MPLS LSR switching is enabled
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on HW error.
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfMplsLsrEnableGet
(
    IN GT_U8     devNum,
    OUT GT_BOOL  *enablePtr
)
{
#ifdef EXMXPM_FAMILY
    GT_STATUS rc = GT_OK;
    GT_BOOL   enable;

    rc = cpssExMxPmMplsEnableGet(devNum, &enable);
    if(rc != GT_OK)
        return rc;

    *enablePtr = enable;
    return rc;

#else /* !EXMXPM_FAMILY */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enablePtr);
    /* this request is not support */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

}

/**
* @internal prvTgfInlifMplsRouteEnableSet function
* @endinternal
*
* @brief   Set the MPLS switching on the interface (Enable\disable)
*
* @param[in] devNum                   - device number
* @param[in] inlifType                - inlif type port/vlan/external
* @param[in] inlifIndex               - inlif index in the table
*                                      (APPLICABLE RANGES: 0..63) for inLif type port or
*                                      (APPLICABLE RANGES: 0..4095) for inLif type vlan or
*                                      (APPLICABLE RANGES: Puma2 4096..65535; Puma3 4096..16383)
*                                      for inLif type external
* @param[in] enable                   - GT_TRUE = Enable.
*                                      GT_FALSE = Disable.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_FAIL                  - otherwise
* @retval GT_UNFIXABLE_ECC_ERROR   - the CPSS detected ECC error that can't
*                                       be fixed when read from the memory that
*                                       protected by ECC generated.
*                                       if entry can't be fixed due to 2 data errors
*                                       NOTE: relevant only to memory controller that
*                                       work with ECC protection , and the CPSS need
*                                       to emulate ECC protection.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfInlifMplsRouteEnableSet
(
    IN  GT_U8                       devNum,
    IN  PRV_TGF_INLIF_TYPE_ENT      inlifType,
    IN  GT_U32                      inlifIndex,
    IN  GT_BOOL                     enable
)
{
#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_INLIF_TYPE_ENT              exMxPmInlifType = CPSS_EXMXPM_INLIF_TYPE_PORT_E;
    /* convert to device specific format */
    switch (inlifType)
    {
        case PRV_TGF_INLIF_TYPE_PORT_E:
            exMxPmInlifType = CPSS_EXMXPM_INLIF_TYPE_PORT_E;
            break;

        case PRV_TGF_INLIF_TYPE_VLAN_E:
            exMxPmInlifType = CPSS_EXMXPM_INLIF_TYPE_VLAN_E;
            break;

        case PRV_TGF_INLIF_TYPE_EXTERNAL_E:
            exMxPmInlifType = CPSS_EXMXPM_INLIF_TYPE_EXTERNAL_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* call device specific API */
    return cpssExMxPmInlifMplsRouteEnableSet(devNum, exMxPmInlifType,
                                             inlifIndex, enable);
#else /* !EXMXPM_FAMILY */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(inlifType);
    TGF_PARAM_NOT_USED(inlifIndex);
    TGF_PARAM_NOT_USED(enable);
    /* this request is not support */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */
}

/**
* @internal prvTgfMplsLabelSpaceModeSet function
* @endinternal
*
* @brief   Defines whether the label space is common or per Interface
*
* @param[in] devNum                   - device number
* @param[in] mode                     - MPLS Label Space mode
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on HW error.
* @retval GT_BAD_PARAM             - on illegal parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfMplsLabelSpaceModeSet
(
    IN GT_U8                              devNum,
    IN PRV_TGF_MPLS_LABEL_SPACE_MODE_ENT  mode
)
{
#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_MPLS_LABEL_SPACE_MODE_ENT        cpssMode;

    /* convert to device specific format */
    switch (mode)
    {
        case PRV_TGF_MPLS_PER_PLATFORM_E:
            cpssMode = CPSS_EXMXPM_MPLS_PER_PLATFORM_E;
            break;

        case PRV_TGF_MPLS_PER_INTERFACE_E:
            cpssMode = CPSS_EXMXPM_MPLS_PER_INTERFACE_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* call device specific API */
    return cpssExMxPmMplsLabelSpaceModeSet(devNum, cpssMode);
#else /* !EXMXPM_FAMILY */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(mode);
    /* this request is not support */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */
}

/**
* @internal prvTgfMplsLabelSpaceModeGet function
* @endinternal
*
* @brief   Get label space mode (common or per Interface)
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - (pointer to) MPLS Label Space mode
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on HW error.
* @retval GT_BAD_PARAM             - on illegal parameter
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - the registers relate to the feature are not synchronized
*                                       (NOTE : values returned in OUT parameters are valid)
*/
GT_STATUS prvTgfMplsLabelSpaceModeGet
(
    IN  GT_U8                               devNum,
    OUT PRV_TGF_MPLS_LABEL_SPACE_MODE_ENT   *modePtr
)
{
#ifdef EXMXPM_FAMILY
    GT_STATUS                               rc = GT_OK;
    CPSS_EXMXPM_MPLS_LABEL_SPACE_MODE_ENT   cpssMode;

     /* call device specific API */
    rc = cpssExMxPmMplsLabelSpaceModeGet(devNum, &cpssMode);
    if(rc != GT_OK)
        return rc;

    /* convert to device specific format */
    switch (cpssMode)
    {
        case CPSS_EXMXPM_MPLS_PER_PLATFORM_E:
            *modePtr = PRV_TGF_MPLS_PER_PLATFORM_E;
            break;

        case CPSS_EXMXPM_MPLS_PER_INTERFACE_E:
            *modePtr = PRV_TGF_MPLS_PER_INTERFACE_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    return rc;

#else /* !EXMXPM_FAMILY */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(modePtr);
    /* this request is not support */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */
}


/**
* @internal prvTgfMplsNhlfeSpecialLabelsBaseIndexSet function
* @endinternal
*
* @brief   Set MPLS Special Labels Table base index
*         Used for MPLS labels in the range 0-15.
* @param[in] devNum                   - device number
* @param[in] baseIndex                - base index of the MPLS Special Labels in the NHLFE table.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on HW error.
* @retval GT_BAD_PARAM             - bad paramter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note It is recommended to set baseIndex to 0, meaning that these labels are
*       located at the beggining of the NHLFE Table.
*
*/
GT_STATUS prvTgfMplsNhlfeSpecialLabelsBaseIndexSet
(
    IN GT_U8   devNum,
    IN GT_U32  baseIndex
)
{
#ifdef EXMXPM_FAMILY

    /* call device specific API */
    return cpssExMxPmMplsNhlfeSpecialLabelsBaseIndexSet(devNum,baseIndex);

#else /* !EXMXPM_FAMILY */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(baseIndex);
    /* this request is not support */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */
}

/**
* @internal prvTgfMplsNhlfeSpecialLabelsBaseIndexGet function
* @endinternal
*
* @brief   Get MPLS Special Labels Table base index
*         Used for MPLS labels in the range 0-15.
* @param[in] devNum                   - device number
*
* @param[out] baseIndexPtr             - (pointer to) base index of the MPLS Special Labels
*                                      in the NHLFE table.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on HW error.
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note If baseIndex is 0 it means that these labels are located
*       at the beggining of the NHLFE Table
*
*/
GT_STATUS prvTgfMplsNhlfeSpecialLabelsBaseIndexGet
(
    IN GT_U8    devNum,
    OUT GT_U32  *baseIndexPtr
)
{
#ifdef EXMXPM_FAMILY
    GT_STATUS rc = GT_OK;
    GT_U32    baseIndex;

    /* call device specific API */
    rc = cpssExMxPmMplsNhlfeSpecialLabelsBaseIndexGet(devNum,&baseIndex);
    if(rc != GT_OK)
        return rc;

    *baseIndexPtr = baseIndex;
    return rc;

#else /* !EXMXPM_FAMILY */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(baseIndexPtr);
    /* this request is not support */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */
}

/**
* @internal prvTgfMplsInterfaceEntrySet function
* @endinternal
*
* @brief   Set the MPLS Interface Table Entry
*
* @param[in] devNum                   - device number
* @param[in] index                    - MPLS Interface Table  calculated based on the VRF-ID
* @param[in] mplsInterfaceEntryStcPtr - (pointer to)The MPLS Interface entry to set
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on HW error.
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note When the device is configured to work in Per-Platform Label Space Mode,
*       index 0 is used.
*
*/
GT_STATUS prvTgfMplsInterfaceEntrySet
(
    IN GT_U8                             devNum,
    IN GT_U32                            index,
    IN PRV_TGF_MPLS_INTERFACE_ENTRY_STC  *mplsInterfaceEntryStcPtr
)
{
#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_MPLS_INTERFACE_ENTRY_STC  cpssMplsInterfaceEntryStc;

    cpssMplsInterfaceEntryStc.valid       = mplsInterfaceEntryStcPtr->valid;
    cpssMplsInterfaceEntryStc.minLabel    = mplsInterfaceEntryStcPtr->minLabel;
    cpssMplsInterfaceEntryStc.maxLabel    = mplsInterfaceEntryStcPtr->maxLabel;
    cpssMplsInterfaceEntryStc.baseIndex   =  mplsInterfaceEntryStcPtr->baseIndex;
    cpssMplsInterfaceEntryStc.ecmpQosSize = mplsInterfaceEntryStcPtr->ecmpQosSize;

    switch(mplsInterfaceEntryStcPtr->nextHopRouteMethod)
    {
        case PRV_TGF_MPLS_NEXT_HOP_ROUTE_METHOD_ECMP_E:
            cpssMplsInterfaceEntryStc.nextHopRouteMethod= CPSS_EXMXPM_MPLS_NEXT_HOP_ROUTE_METHOD_ECMP_E;
            break;
        case PRV_TGF_MPLS_NEXT_HOP_ROUTE_METHOD_QOS_E:
            cpssMplsInterfaceEntryStc.nextHopRouteMethod = CPSS_EXMXPM_MPLS_NEXT_HOP_ROUTE_METHOD_QOS_E;
            break;
        case PRV_TGF_MPLS_NEXT_HOP_ROUTE_METHOD_REGULAR_E:
            cpssMplsInterfaceEntryStc.nextHopRouteMethod = CPSS_EXMXPM_MPLS_NEXT_HOP_ROUTE_METHOD_REGULAR_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

        /* call device specific API */
    return cpssExMxPmMplsInterfaceEntrySet(devNum,index,&cpssMplsInterfaceEntryStc);

#else /* !EXMXPM_FAMILY */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(index);
    TGF_PARAM_NOT_USED(mplsInterfaceEntryStcPtr);
    /* this request is not support */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

}

/**
* @internal prvTgfMplsInterfaceEntryGet function
* @endinternal
*
* @brief   Get the MPLS Interface Table Entry
*
* @param[in] devNum                   - device number
* @param[in] index                    - MPLS Interface Table  calculated based on the VRF-ID
*
* @param[out] mplsInterfaceEntryStcPtr - (pointer to) The MPLS Interface entry
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on HW error.
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on invalid hardware value read
*
* @note When the device is configured to work in Per-Platform Label Space Mode,
*       index 0 is used.
*
*/
GT_STATUS prvTgfMplsInterfaceEntryGet
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            index,
    OUT PRV_TGF_MPLS_INTERFACE_ENTRY_STC  *mplsInterfaceEntryStcPtr
)
{
#ifdef EXMXPM_FAMILY
    GT_STATUS rc = GT_OK;
    CPSS_EXMXPM_MPLS_INTERFACE_ENTRY_STC  cpssMplsInterfaceEntryStc;

    /* call device specific API */
    rc = cpssExMxPmMplsInterfaceEntryGet(devNum,index,&cpssMplsInterfaceEntryStc);
    if(rc != GT_OK)
        return rc;

    mplsInterfaceEntryStcPtr->valid = cpssMplsInterfaceEntryStc.valid;
    mplsInterfaceEntryStcPtr->minLabel = cpssMplsInterfaceEntryStc.minLabel;
    mplsInterfaceEntryStcPtr->maxLabel = cpssMplsInterfaceEntryStc.maxLabel;
    mplsInterfaceEntryStcPtr->baseIndex = cpssMplsInterfaceEntryStc.baseIndex;
    mplsInterfaceEntryStcPtr->ecmpQosSize = cpssMplsInterfaceEntryStc.ecmpQosSize;

    switch(cpssMplsInterfaceEntryStc.nextHopRouteMethod)
    {
        case CPSS_EXMXPM_MPLS_NEXT_HOP_ROUTE_METHOD_ECMP_E:
            mplsInterfaceEntryStcPtr->nextHopRouteMethod= PRV_TGF_MPLS_NEXT_HOP_ROUTE_METHOD_ECMP_E;
            break;
        case CPSS_EXMXPM_MPLS_NEXT_HOP_ROUTE_METHOD_QOS_E:
            mplsInterfaceEntryStcPtr->nextHopRouteMethod = PRV_TGF_MPLS_NEXT_HOP_ROUTE_METHOD_QOS_E;
            break;
        case CPSS_EXMXPM_MPLS_NEXT_HOP_ROUTE_METHOD_REGULAR_E:
            mplsInterfaceEntryStcPtr->nextHopRouteMethod = PRV_TGF_MPLS_NEXT_HOP_ROUTE_METHOD_REGULAR_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    return rc;

#else /* !EXMXPM_FAMILY */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(index);
    TGF_PARAM_NOT_USED(mplsInterfaceEntryStcPtr);
    /* this request is not support */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

}

#ifdef EXMXPM_FAMILY
/**
* @internal prvTgfConvertGenericToExMxPmNhlfEntry function
* @endinternal
*
* @brief   Convert generic Next Hop Label Forwarding Entry (NHLFE)
*         into device specific NHLF entry
* @param[in] nhlfEntryPtr             - (pointer to) NHLFE parameters
*
* @param[out] exMxPmNhlfEntryPtr       - (pointer to) ExMxPm entry parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToExMxPmNhlfEntry
(
    IN  PRV_TGF_MPLS_NHLFE_STC       *nhlfEntryPtr,
    OUT CPSS_EXMXPM_MPLS_NHLFE_STC   *exMxPmNhlfEntryPtr
)
{
    cpssOsMemSet(exMxPmNhlfEntryPtr,0,sizeof(CPSS_EXMXPM_MPLS_NHLFE_STC));

    /* convert interface info into device specific format */
    exMxPmNhlfEntryPtr->nhlfeCommonParams.outlifConfig.interfaceInfo.type
        = nhlfEntryPtr->nhlfeCommonParams.outlifConfig.interfaceInfo.type;
    switch (nhlfEntryPtr->nhlfeCommonParams.outlifConfig.interfaceInfo.type)
    {
        case CPSS_INTERFACE_PORT_E:
            exMxPmNhlfEntryPtr->nhlfeCommonParams.outlifConfig.interfaceInfo.devPort.hwDevNum  =
                nhlfEntryPtr->nhlfeCommonParams.outlifConfig.interfaceInfo.devPort.hwDevNum;
            exMxPmNhlfEntryPtr->nhlfeCommonParams.outlifConfig.interfaceInfo.devPort.portNum =
                nhlfEntryPtr->nhlfeCommonParams.outlifConfig.interfaceInfo.devPort.portNum;
            break;

        case CPSS_INTERFACE_TRUNK_E:
            exMxPmNhlfEntryPtr->nhlfeCommonParams.outlifConfig.interfaceInfo.trunkId =
                nhlfEntryPtr->nhlfeCommonParams.outlifConfig.interfaceInfo.trunkId;
            break;

        case CPSS_INTERFACE_VIDX_E:
            exMxPmNhlfEntryPtr->nhlfeCommonParams.outlifConfig.interfaceInfo.vidx =
                nhlfEntryPtr->nhlfeCommonParams.outlifConfig.interfaceInfo.vidx;
            break;

        case CPSS_INTERFACE_VID_E:
            exMxPmNhlfEntryPtr->nhlfeCommonParams.outlifConfig.interfaceInfo.vlanId =
                nhlfEntryPtr->nhlfeCommonParams.outlifConfig.interfaceInfo.vlanId;
            break;

        case CPSS_INTERFACE_DEVICE_E:
            exMxPmNhlfEntryPtr->nhlfeCommonParams.outlifConfig.interfaceInfo.hwDevNum =
                nhlfEntryPtr->nhlfeCommonParams.outlifConfig.interfaceInfo.hwDevNum;
            break;

        case CPSS_INTERFACE_FABRIC_VIDX_E:
            exMxPmNhlfEntryPtr->nhlfeCommonParams.outlifConfig.interfaceInfo.fabricVidx =
                nhlfEntryPtr->nhlfeCommonParams.outlifConfig.interfaceInfo.fabricVidx;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert interface info into device specific format */
    switch (nhlfEntryPtr->nhlfeCommonParams.outlifConfig.outlifType)
    {
        case PRV_TGF_OUTLIF_TYPE_LL_E:
            exMxPmNhlfEntryPtr->nhlfeCommonParams.outlifConfig.outlifType=
                                                    CPSS_EXMXPM_OUTLIF_TYPE_LL_E;
            exMxPmNhlfEntryPtr->nhlfeCommonParams.outlifConfig.outlifPointer.arpPtr =
                nhlfEntryPtr->nhlfeCommonParams.outlifConfig.outlifPointer.arpPtr;
            break;

        case PRV_TGF_OUTLIF_TYPE_DIT_E:
            exMxPmNhlfEntryPtr->nhlfeCommonParams.outlifConfig.outlifType=
                                                    CPSS_EXMXPM_OUTLIF_TYPE_DIT_E;
            exMxPmNhlfEntryPtr->nhlfeCommonParams.outlifConfig.outlifPointer.ditPtr =
                nhlfEntryPtr->nhlfeCommonParams.outlifConfig.outlifPointer.ditPtr;
            break;

        case PRV_TGF_OUTLIF_TYPE_TUNNEL_E:
            exMxPmNhlfEntryPtr->nhlfeCommonParams.outlifConfig.outlifType=
                                                CPSS_EXMXPM_OUTLIF_TYPE_TUNNEL_E;
            /* convert passenger type into device specific format */
            switch (nhlfEntryPtr->nhlfeCommonParams.outlifConfig.outlifPointer.tunnelStartPtr.passengerPacketType)
            {
                case PRV_TGF_TS_PASSENGER_PACKET_TYPE_ETHERNET_E:
                    exMxPmNhlfEntryPtr->nhlfeCommonParams.outlifConfig.outlifPointer.tunnelStartPtr.passengerPacketType =
                        CPSS_EXMXPM_TS_PASSENGER_PACKET_TYPE_ETHERNET_E;
                    break;

                case PRV_TGF_TS_PASSENGER_PACKET_TYPE_OTHER_E:
                    exMxPmNhlfEntryPtr->nhlfeCommonParams.outlifConfig.outlifPointer.tunnelStartPtr.passengerPacketType =
                        CPSS_EXMXPM_TS_PASSENGER_PACKET_TYPE_OTHER_E;
                    break;

                default:
                    return GT_BAD_PARAM;
            }

            exMxPmNhlfEntryPtr->nhlfeCommonParams.outlifConfig.outlifPointer.tunnelStartPtr.ptr =
                nhlfEntryPtr->nhlfeCommonParams.outlifConfig.outlifPointer.tunnelStartPtr.ptr;
            break;

        default:
            return GT_BAD_PARAM;
    }

    switch(nhlfEntryPtr->nhlfeCommonParams.counterSetIndex)
    {
        case PRV_TGF_MPLS_COUNTER_SET_INDEX_0_E:
            exMxPmNhlfEntryPtr->nhlfeCommonParams.counterSetIndex=CPSS_EXMXPM_MPLS_COUNTER_SET_INDEX_0_E;
            break;
        case PRV_TGF_MPLS_COUNTER_SET_INDEX_1_E:
            exMxPmNhlfEntryPtr->nhlfeCommonParams.counterSetIndex=CPSS_EXMXPM_MPLS_COUNTER_SET_INDEX_1_E;
            break;
        case PRV_TGF_MPLS_COUNTER_SET_INDEX_2_E:
            exMxPmNhlfEntryPtr->nhlfeCommonParams.counterSetIndex=CPSS_EXMXPM_MPLS_COUNTER_SET_INDEX_2_E;
            break;
        case PRV_TGF_MPLS_COUNTER_SET_INDEX_3_E:
            exMxPmNhlfEntryPtr->nhlfeCommonParams.counterSetIndex=CPSS_EXMXPM_MPLS_COUNTER_SET_INDEX_3_E;
            break;
        case PRV_TGF_MPLS_COUNTER_SET_DISABLE_E:
            exMxPmNhlfEntryPtr->nhlfeCommonParams.counterSetIndex=CPSS_EXMXPM_MPLS_COUNTER_SET_DISABLE_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    switch(nhlfEntryPtr->nhlfeCommonParams.mplsCmd)
    {
        case PRV_TGF_MPLS_NOP_CMD_E:
            exMxPmNhlfEntryPtr->nhlfeCommonParams.mplsCmd=CPSS_EXMXPM_MPLS_NOP_CMD_E;
            break;
        case PRV_TGF_MPLS_SWAP_CMD_E:
            exMxPmNhlfEntryPtr->nhlfeCommonParams.mplsCmd=CPSS_EXMXPM_MPLS_SWAP_CMD_E;
            break;
        case PRV_TGF_MPLS_PUSH_CMD_E:
            exMxPmNhlfEntryPtr->nhlfeCommonParams.mplsCmd=CPSS_EXMXPM_MPLS_PUSH_CMD_E;
            break;
        case PRV_TGF_MPLS_PHP_CMD_E:
            exMxPmNhlfEntryPtr->nhlfeCommonParams.mplsCmd=CPSS_EXMXPM_MPLS_PHP_CMD_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

     switch(nhlfEntryPtr->nhlfeCommonParams.ttlMode)
    {
        case PRV_TGF_MPLS_TTL_NOT_DECREMENTED_E:
            exMxPmNhlfEntryPtr->nhlfeCommonParams.ttlMode=CPSS_EXMXPM_MPLS_TTL_NOT_DECREMENTED_E;
            break;
        case PRV_TGF_MPLS_TTL_DECREMENTED_BY_ONE_E:
            exMxPmNhlfEntryPtr->nhlfeCommonParams.ttlMode=CPSS_EXMXPM_MPLS_TTL_DECREMENTED_BY_ONE_E;
            break;
        case PRV_TGF_MPLS_TTL_DECREMENTED_BY_ENTRY_E:
            exMxPmNhlfEntryPtr->nhlfeCommonParams.ttlMode=CPSS_EXMXPM_MPLS_TTL_DECREMENTED_BY_ENTRY_E;
            break;
        case PRV_TGF_MPLS_TTL_COPY_ENTRY_E:
            exMxPmNhlfEntryPtr->nhlfeCommonParams.ttlMode=CPSS_EXMXPM_MPLS_TTL_COPY_ENTRY_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    /* convert generic into device specific */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmNhlfEntryPtr->nhlfeCommonParams),
                                   &nhlfEntryPtr->nhlfeCommonParams, ageRefresh);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmNhlfEntryPtr->nhlfeCommonParams),
                                   &nhlfEntryPtr->nhlfeCommonParams, bypassTtlExceptionCheckEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmNhlfEntryPtr->nhlfeCommonParams),
                                   &nhlfEntryPtr->nhlfeCommonParams, cpuCodeIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmNhlfEntryPtr->nhlfeCommonParams),
                                   &nhlfEntryPtr->nhlfeCommonParams, mirrorToIngressAnalyzer);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmNhlfEntryPtr->nhlfeCommonParams),
                                   &nhlfEntryPtr->nhlfeCommonParams, mplsLabel);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmNhlfEntryPtr->nhlfeCommonParams),
                                   &nhlfEntryPtr->nhlfeCommonParams, mtuProfileIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmNhlfEntryPtr->nhlfeCommonParams),
                                   &nhlfEntryPtr->nhlfeCommonParams, nextHopVlanId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmNhlfEntryPtr->nhlfeCommonParams),
                                   &nhlfEntryPtr->nhlfeCommonParams, packetCmd);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmNhlfEntryPtr->nhlfeCommonParams),
                                   &nhlfEntryPtr->nhlfeCommonParams, ttl);

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmNhlfEntryPtr->qosParamsModify),
                                   &nhlfEntryPtr->qosParamsModify, modifyTc);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmNhlfEntryPtr->qosParamsModify),
                                   &nhlfEntryPtr->qosParamsModify, modifyUp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmNhlfEntryPtr->qosParamsModify),
                                   &nhlfEntryPtr->qosParamsModify, modifyDp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmNhlfEntryPtr->qosParamsModify),
                                   &nhlfEntryPtr->qosParamsModify, modifyDscp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmNhlfEntryPtr->qosParamsModify),
                                   &nhlfEntryPtr->qosParamsModify, modifyExp);

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmNhlfEntryPtr->qosParams),
                                   &nhlfEntryPtr->qosParams, tc);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmNhlfEntryPtr->qosParams),
                                   &nhlfEntryPtr->qosParams, dp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmNhlfEntryPtr->qosParams),
                                   &nhlfEntryPtr->qosParams, up);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmNhlfEntryPtr->qosParams),
                                   &nhlfEntryPtr->qosParams, dscp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(exMxPmNhlfEntryPtr->qosParams),
                                   &nhlfEntryPtr->qosParams, exp);

    return GT_OK;
}


/**
* @internal prvTgfConvertExMxPmToGenericNhlfEntry function
* @endinternal
*
* @brief   Convert device specific NHLF entry into generic
*         Next Hop Label Forwarding Entry (NHLFE)
* @param[in] exMxPmNhlfEntryPtr       - (pointer to) ExMxPm entry parameters
*
* @param[out] nhlfEntryPtr             - (pointer to) NHLFE parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertExMxPmToGenericNhlfEntry
(
    IN  CPSS_EXMXPM_MPLS_NHLFE_STC   *exMxPmNhlfEntryPtr,
    OUT PRV_TGF_MPLS_NHLFE_STC       *nhlfEntryPtr
)
{
    cpssOsMemSet(nhlfEntryPtr,0,sizeof(PRV_TGF_MPLS_NHLFE_STC));

    /* convert interface info into device specific format */
    nhlfEntryPtr->nhlfeCommonParams.outlifConfig.interfaceInfo.type =
        exMxPmNhlfEntryPtr->nhlfeCommonParams.outlifConfig.interfaceInfo.type;

    switch (exMxPmNhlfEntryPtr->nhlfeCommonParams.outlifConfig.interfaceInfo.type)
    {
        case CPSS_INTERFACE_PORT_E:
            nhlfEntryPtr->nhlfeCommonParams.outlifConfig.interfaceInfo.devPort.hwDevNum  =
                exMxPmNhlfEntryPtr->nhlfeCommonParams.outlifConfig.interfaceInfo.devPort.hwDevNum;
            nhlfEntryPtr->nhlfeCommonParams.outlifConfig.interfaceInfo.devPort.portNum =
                exMxPmNhlfEntryPtr->nhlfeCommonParams.outlifConfig.interfaceInfo.devPort.portNum;
            break;

        case CPSS_INTERFACE_TRUNK_E:
            nhlfEntryPtr->nhlfeCommonParams.outlifConfig.interfaceInfo.trunkId =
                exMxPmNhlfEntryPtr->nhlfeCommonParams.outlifConfig.interfaceInfo.trunkId;
            break;

        case CPSS_INTERFACE_VIDX_E:
            nhlfEntryPtr->nhlfeCommonParams.outlifConfig.interfaceInfo.vidx =
                exMxPmNhlfEntryPtr->nhlfeCommonParams.outlifConfig.interfaceInfo.vidx;
            break;

        case CPSS_INTERFACE_VID_E:
            nhlfEntryPtr->nhlfeCommonParams.outlifConfig.interfaceInfo.vlanId =
                exMxPmNhlfEntryPtr->nhlfeCommonParams.outlifConfig.interfaceInfo.vlanId;
            break;

        case CPSS_INTERFACE_DEVICE_E:
            nhlfEntryPtr->nhlfeCommonParams.outlifConfig.interfaceInfo.hwDevNum =
                exMxPmNhlfEntryPtr->nhlfeCommonParams.outlifConfig.interfaceInfo.hwDevNum;
            break;

        case CPSS_INTERFACE_FABRIC_VIDX_E:
            nhlfEntryPtr->nhlfeCommonParams.outlifConfig.interfaceInfo.fabricVidx =
                exMxPmNhlfEntryPtr->nhlfeCommonParams.outlifConfig.interfaceInfo.fabricVidx;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert interface info into device specific format */
    switch (exMxPmNhlfEntryPtr->nhlfeCommonParams.outlifConfig.outlifType)
    {
        case CPSS_EXMXPM_OUTLIF_TYPE_LL_E:
            nhlfEntryPtr->nhlfeCommonParams.outlifConfig.outlifType=
                                                    PRV_TGF_OUTLIF_TYPE_LL_E;
            nhlfEntryPtr->nhlfeCommonParams.outlifConfig.outlifPointer.arpPtr =
                exMxPmNhlfEntryPtr->nhlfeCommonParams.outlifConfig.outlifPointer.arpPtr;
            break;

        case CPSS_EXMXPM_OUTLIF_TYPE_DIT_E:
            nhlfEntryPtr->nhlfeCommonParams.outlifConfig.outlifType=
                                                    PRV_TGF_OUTLIF_TYPE_DIT_E;
            nhlfEntryPtr->nhlfeCommonParams.outlifConfig.outlifPointer.ditPtr =
                exMxPmNhlfEntryPtr->nhlfeCommonParams.outlifConfig.outlifPointer.ditPtr;
            break;

        case CPSS_EXMXPM_OUTLIF_TYPE_TUNNEL_E:
            nhlfEntryPtr->nhlfeCommonParams.outlifConfig.outlifType=
                                                PRV_TGF_OUTLIF_TYPE_TUNNEL_E;
            /* convert passenger type into device specific format */
            switch (exMxPmNhlfEntryPtr->nhlfeCommonParams.outlifConfig.outlifPointer.tunnelStartPtr.passengerPacketType)
            {
                case CPSS_EXMXPM_TS_PASSENGER_PACKET_TYPE_ETHERNET_E:
                    nhlfEntryPtr->nhlfeCommonParams.outlifConfig.outlifPointer.tunnelStartPtr.passengerPacketType =
                        PRV_TGF_TS_PASSENGER_PACKET_TYPE_ETHERNET_E;
                    break;

                case CPSS_EXMXPM_TS_PASSENGER_PACKET_TYPE_OTHER_E:
                    nhlfEntryPtr->nhlfeCommonParams.outlifConfig.outlifPointer.tunnelStartPtr.passengerPacketType =
                        PRV_TGF_TS_PASSENGER_PACKET_TYPE_OTHER_E;
                    break;

                default:
                    return GT_BAD_PARAM;
            }

            nhlfEntryPtr->nhlfeCommonParams.outlifConfig.outlifPointer.tunnelStartPtr.ptr =
                exMxPmNhlfEntryPtr->nhlfeCommonParams.outlifConfig.outlifPointer.tunnelStartPtr.ptr;
            break;

        default:
            return GT_BAD_PARAM;
    }

    switch(exMxPmNhlfEntryPtr->nhlfeCommonParams.counterSetIndex)
    {
        case CPSS_EXMXPM_MPLS_COUNTER_SET_INDEX_0_E:
            nhlfEntryPtr->nhlfeCommonParams.counterSetIndex=PRV_TGF_MPLS_COUNTER_SET_INDEX_0_E;
            break;
        case CPSS_EXMXPM_MPLS_COUNTER_SET_INDEX_1_E:
            nhlfEntryPtr->nhlfeCommonParams.counterSetIndex=PRV_TGF_MPLS_COUNTER_SET_INDEX_1_E;
            break;
        case CPSS_EXMXPM_MPLS_COUNTER_SET_INDEX_2_E:
            nhlfEntryPtr->nhlfeCommonParams.counterSetIndex=PRV_TGF_MPLS_COUNTER_SET_INDEX_2_E;
            break;
        case CPSS_EXMXPM_MPLS_COUNTER_SET_INDEX_3_E:
            nhlfEntryPtr->nhlfeCommonParams.counterSetIndex=PRV_TGF_MPLS_COUNTER_SET_INDEX_3_E;
            break;
        case CPSS_EXMXPM_MPLS_COUNTER_SET_DISABLE_E:
            nhlfEntryPtr->nhlfeCommonParams.counterSetIndex=PRV_TGF_MPLS_COUNTER_SET_DISABLE_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    switch(exMxPmNhlfEntryPtr->nhlfeCommonParams.mplsCmd)
    {
        case CPSS_EXMXPM_MPLS_NOP_CMD_E:
            nhlfEntryPtr->nhlfeCommonParams.mplsCmd=PRV_TGF_MPLS_NOP_CMD_E;
            break;
        case CPSS_EXMXPM_MPLS_SWAP_CMD_E:
            nhlfEntryPtr->nhlfeCommonParams.mplsCmd=PRV_TGF_MPLS_SWAP_CMD_E;
            break;
        case CPSS_EXMXPM_MPLS_PUSH_CMD_E:
            nhlfEntryPtr->nhlfeCommonParams.mplsCmd=PRV_TGF_MPLS_PUSH_CMD_E;
            break;
        case CPSS_EXMXPM_MPLS_PHP_CMD_E:
            nhlfEntryPtr->nhlfeCommonParams.mplsCmd=PRV_TGF_MPLS_PHP_CMD_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

     switch(exMxPmNhlfEntryPtr->nhlfeCommonParams.ttlMode)
    {
        case CPSS_EXMXPM_MPLS_TTL_NOT_DECREMENTED_E:
            nhlfEntryPtr->nhlfeCommonParams.ttlMode=PRV_TGF_MPLS_TTL_NOT_DECREMENTED_E;
            break;
        case CPSS_EXMXPM_MPLS_TTL_DECREMENTED_BY_ONE_E:
            nhlfEntryPtr->nhlfeCommonParams.ttlMode=PRV_TGF_MPLS_TTL_DECREMENTED_BY_ONE_E;
            break;
        case CPSS_EXMXPM_MPLS_TTL_DECREMENTED_BY_ENTRY_E:
            nhlfEntryPtr->nhlfeCommonParams.ttlMode=PRV_TGF_MPLS_TTL_DECREMENTED_BY_ENTRY_E;
            break;
        case CPSS_EXMXPM_MPLS_TTL_COPY_ENTRY_E:
            nhlfEntryPtr->nhlfeCommonParams.ttlMode=PRV_TGF_MPLS_TTL_COPY_ENTRY_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    /* convert device specific into generic*/
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmNhlfEntryPtr->nhlfeCommonParams),
                                   &nhlfEntryPtr->nhlfeCommonParams, ageRefresh);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmNhlfEntryPtr->nhlfeCommonParams),
                                   &nhlfEntryPtr->nhlfeCommonParams, bypassTtlExceptionCheckEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmNhlfEntryPtr->nhlfeCommonParams),
                                   &nhlfEntryPtr->nhlfeCommonParams, cpuCodeIndex);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmNhlfEntryPtr->nhlfeCommonParams),
                                   &nhlfEntryPtr->nhlfeCommonParams, mirrorToIngressAnalyzer);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmNhlfEntryPtr->nhlfeCommonParams),
                                   &nhlfEntryPtr->nhlfeCommonParams, mplsLabel);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmNhlfEntryPtr->nhlfeCommonParams),
                                   &nhlfEntryPtr->nhlfeCommonParams, mtuProfileIndex);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmNhlfEntryPtr->nhlfeCommonParams),
                                   &nhlfEntryPtr->nhlfeCommonParams, nextHopVlanId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmNhlfEntryPtr->nhlfeCommonParams),
                                   &nhlfEntryPtr->nhlfeCommonParams, packetCmd);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmNhlfEntryPtr->nhlfeCommonParams),
                                   &nhlfEntryPtr->nhlfeCommonParams, ttl);

    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmNhlfEntryPtr->qosParamsModify),
                                   &nhlfEntryPtr->qosParamsModify, modifyTc);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmNhlfEntryPtr->qosParamsModify),
                                   &nhlfEntryPtr->qosParamsModify, modifyUp);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmNhlfEntryPtr->qosParamsModify),
                                   &nhlfEntryPtr->qosParamsModify, modifyDp);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmNhlfEntryPtr->qosParamsModify),
                                   &nhlfEntryPtr->qosParamsModify, modifyDscp);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmNhlfEntryPtr->qosParamsModify),
                                   &nhlfEntryPtr->qosParamsModify, modifyExp);

    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmNhlfEntryPtr->qosParams),
                                   &nhlfEntryPtr->qosParams, tc);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmNhlfEntryPtr->qosParams),
                                   &nhlfEntryPtr->qosParams, dp);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmNhlfEntryPtr->qosParams),
                                   &nhlfEntryPtr->qosParams, up);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmNhlfEntryPtr->qosParams),
                                   &nhlfEntryPtr->qosParams, dscp);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(exMxPmNhlfEntryPtr->qosParams),
                                   &nhlfEntryPtr->qosParams, exp);

    return GT_OK;
}
#endif /* EXMXPM_FAMILY */

/**
* @internal prvTgfMplsNhlfEntriesWrite function
* @endinternal
*
* @brief   Write an array of NHLFE entries to hw.
*
* @param[in] devNum                   - device number
* @param[in] nhlfEntryBaseIndex       - The base Index in the NHLFE table
*                                      The entries will be written from this base on.
* @param[in] numOfNhlfEntries         - The number NHLF entries to write.
* @param[in] nhlfEntriesArrayPtr      - (pointer to) The NHLF entries array
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on illegal parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note No partial write.
*       Errata FE-8050: MPLS Router always uses a Uniform model on the TTL
*       Description:
*       The device supports MPLS switching.
*       When the incoming packet is MPLS and the following conditions are met:
*       1. Incoming MPLS packet contains one label or more.
*       2. MPLS routing is enabled in the InLIF.
*       3. No Match in the TTI.
*       4. NHLFE<MPLS Cmd> = POP/SWAP/PUSH.
*       5. NHLFE<TTL Mode> = Decrement/Not Decrement/Copy NHLFE<TTL>.
*       6. TTL > 0.
*       The expected TTL of the outer MPLS header is as follows:
*       - POP: TTL is copied from the original outer MPLS header or the Inner MPLS header TTL is used.
*       - SWAP: TTL is copied from the original outer MPLS header or the NHLFE<TTL> is used.
*       - PUSH: TTL is copied from the original inner MPLS header or the NHLFE<TTL> is used.
*       However, due to this erratum, the TTL of the new outer MPLS header contains the TTL of the
*       original outer MPLS header (Uniform model) instead of having the ability to control which
*       TTL is used (Pipe model/Uniform model).
*       When the packet does not pass through the TTI or there is no match to the TTI key, the router
*       always copies/uses the original outer MPLS header’s TTL.
*       Workaround: None
*       Errata FE-8250: Wrong access to internal nhe table
*       Description:
*       When accessing the nhe internal table the address decode is wrong:
*       1. Address decode is:
*       mg[4:0] - resolution in line
*       mg[18:5] - line number
*       2. The Table offset is: 0x00540000 which means that in the address
*       offset bit18 is turned on. This bit is also used for the line number
*       decode. This creates a situation where address 0x00540000 actually accesses line:
*       0x2000 instead of line number: 0x0
*       Workaround:
*       1. lines 0x0-0x1FFF    addresses should be: 0x00580000-0x005BFFFF.
*       2. lines 0x2000-0x3FFF addresses should be: 0x00540000-0x0057FFFF.
*       - This erratum allow to look at this table as 2 separate tables with 2 different offsets.
*       Errata FE-8490: Wrong packet command resolution in IP/MPLS router engine
*       Description:
*       Each ingress pipeline engine can assign a new packet command to the
*       packet according to the description in the command resolution matrix.
*       The command resolution matrix defines that the final command should be
*       trapped if the Previous Engine assigned the Packet Command to Soft Drop,
*       and the New Engine assigned the Packet Command to Mirror.
*       However, in the IP/MPLS Router Engine, the final command in this case
*       will be soft dropped and not trapped.
*       Functional Impact:
*       IP/MPLS Router Engine will not change the packet command according to
*       the Ingress Command Resolution Matrix for all the packets that enter the
*       IP/MPLS Router Engine with packet commands that equal Soft Drop, and if
*       the IP/MPLS Router Engine decided to change the packet command to Mirror.
*       Workaround:
*       None
*
*/
GT_STATUS prvTgfMplsNhlfEntriesWrite
(

    IN  GT_U8                               devNum,
    IN  GT_U32                              nhlfEntryBaseIndex,
    IN  GT_U32                              numOfNhlfEntries,
    IN  PRV_TGF_MPLS_NHLFE_STC              *nhlfEntriesArrayPtr
)
{
#ifdef EXMXPM_FAMILY
    GT_STATUS                   rc = GT_OK;
    CPSS_EXMXPM_MPLS_NHLFE_STC  *exMxPmNhlfEntriesArray;
    GT_U32                      entryIter = 0;


    exMxPmNhlfEntriesArray = cpssOsMalloc(numOfNhlfEntries * sizeof(CPSS_EXMXPM_MPLS_NHLFE_STC));

    /* reset variables */
    cpssOsMemSet((GT_VOID*) exMxPmNhlfEntriesArray, 0,
                 numOfNhlfEntries * sizeof(CPSS_EXMXPM_MPLS_NHLFE_STC));

    /* convert NHLFE into device specific format */
    for (entryIter = 0; entryIter < numOfNhlfEntries; entryIter++)
    {
        rc = prvTgfConvertGenericToExMxPmNhlfEntry(&(nhlfEntriesArrayPtr[entryIter]),
                                                    &(exMxPmNhlfEntriesArray[entryIter]));
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToExMxPmNhlfEntry FAILED, rc = [%d]", rc);

            /* free allocated memory */
            cpssOsFree(exMxPmNhlfEntriesArray);

            return rc;
        }
    }

    /* call device specific API */
    rc = cpssExMxPmMplsNhlfEntriesWrite(devNum, nhlfEntryBaseIndex, numOfNhlfEntries, exMxPmNhlfEntriesArray);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmMplsNhlfEntriesWrite FAILED, rc = [%d]", rc);
    }

    /* free allocated memory */
    cpssOsFree(exMxPmNhlfEntriesArray);
    return rc;


#else /* !EXMXPM_FAMILY */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(nhlfEntryBaseIndex);
    TGF_PARAM_NOT_USED(numOfNhlfEntries);
    TGF_PARAM_NOT_USED(nhlfEntriesArrayPtr);
    /* this request is not support */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

}

/**
* @internal prvTgfMplsNhlfEntriesRead function
* @endinternal
*
* @brief   Read an array of NHLF entries from hw.
*
* @param[in] devNum                   - device number
* @param[in] nhlfEntryBaseIndex       - The base Index in the NHLFE table
*                                      The entries will be read from this base on.
* @param[in] numOfNhlfEntries         - The number NHLF entries to read.
*
* @param[out] nhlfEntriesArrayPtr      - (pointer to) The NHLF entries array
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on illegal parameter
* @retval GT_BAD_PTR               - on NULL pointer
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
GT_STATUS prvTgfMplsNhlfEntriesRead
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          nhlfEntryBaseIndex,
    IN  GT_U32                          numOfNhlfEntries,
    OUT PRV_TGF_MPLS_NHLFE_STC          *nhlfEntriesArrayPtr
)
{
#ifdef EXMXPM_FAMILY
    GT_STATUS                   rc = GT_OK;
    CPSS_EXMXPM_MPLS_NHLFE_STC  *exMxPmNhlfEntriesArray;
    GT_U32                      entryIter = 0;


    exMxPmNhlfEntriesArray = cpssOsMalloc(numOfNhlfEntries * sizeof(CPSS_EXMXPM_MPLS_NHLFE_STC));

    /* reset variables */
    cpssOsMemSet((GT_VOID*) exMxPmNhlfEntriesArray, 0,
                 numOfNhlfEntries * sizeof(CPSS_EXMXPM_MPLS_NHLFE_STC));

    /* call device specific API */
    rc = cpssExMxPmMplsNhlfEntriesRead(devNum, nhlfEntryBaseIndex, numOfNhlfEntries, exMxPmNhlfEntriesArray);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmMplsNhlfEntriesRead FAILED, rc = [%d]", rc);
    }


    /* convert device specific format into NHLFE entry */
    for (entryIter = 0; entryIter < numOfNhlfEntries; entryIter++)
    {
        rc = prvTgfConvertExMxPmToGenericNhlfEntry(&exMxPmNhlfEntriesArray[entryIter],
                                                   &nhlfEntriesArrayPtr[entryIter]);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertExMxPmToGenericNhlfEntry FAILED, rc = [%d]", rc);

            /* free allocated memory */
            cpssOsFree(exMxPmNhlfEntriesArray);

            return rc;
        }
    }

    /* free allocated memory */
    cpssOsFree(exMxPmNhlfEntriesArray);
    return rc;


#else /* !EXMXPM_FAMILY */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(nhlfEntryBaseIndex);
    TGF_PARAM_NOT_USED(numOfNhlfEntries);
    TGF_PARAM_NOT_USED(nhlfEntriesArrayPtr);
    /* this request is not support */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

}

/**
* @internal prvTgfMplsCountersIpSet function
* @endinternal
*
* @brief   Set IP couters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong parameter
*/
GT_STATUS prvTgfMplsCountersIpSet
(
    IN GT_U32                          portNum,
    IN GT_U32                         counterIndex
)
{
#ifdef EXMXPM_FAMILY

    GT_STATUS   rc       = GT_OK;
    GT_U32      portIter = 0;

    CPSS_EXMXPM_IP_COUNTER_BIND_MODE_ENT        bindMode;
    CPSS_EXMXPM_IP_COUNTER_SET_CFG_STC          exMxPmInterfaceModeCfg = {0};
    CPSS_EXMXPM_IP_COUNTER_SET_STC              ipCounters = {0};


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

    /* set counter bind mode */
    bindMode = CPSS_EXMXPM_IP_COUNTER_BIND_TO_ROUTE_ENTRY_E;

    /* set ROUTE_ENTRY mode for IP counters */
    exMxPmInterfaceModeCfg.portTrunkMode      = CPSS_EXMXPM_IP_PORT_COUNTER_MODE_E;
    exMxPmInterfaceModeCfg.portTrunk.devPort.devNum = prvTgfDevsArray[portIter];
    exMxPmInterfaceModeCfg.portTrunk.devPort.port   = portNum;
    exMxPmInterfaceModeCfg.portTrunk.trunk    = 0;
    exMxPmInterfaceModeCfg.packetType         = CPSS_EXMXPM_IP_PACKET_TYPE_MPLS_COUNTER_MODE_E;
    exMxPmInterfaceModeCfg.vlanMode           = CPSS_EXMXPM_IP_DISREGARD_VLAN_COUNTER_MODE_E;
    exMxPmInterfaceModeCfg.vlanId             = 0;

    rc = cpssExMxPmIpCounterConfigSet(prvTgfDevsArray[portIter], counterIndex, bindMode, &exMxPmInterfaceModeCfg);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmIpCounterConfigSet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* reset IP couters */
    rc = cpssExMxPmIpCounterSet(prvTgfDevsArray[portIter], counterIndex, &ipCounters);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmIpCounterSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;

#else /* !EXMXPM_FAMILY */
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(counterIndex);
    /* this request is not support */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */
}

/**
* @internal prvTgfMplsInvalidEntryCmdSet function
* @endinternal
*
* @brief   Set the command assigned to MPLS packets if any of the following is true:
*         - The MPLS Interface Table entry is not valid
*         - The incoming label is out of label space boundaries
* @param[in] devNum                   - device number
* @param[in] cmd                      - Valid commands:
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E - Packets are trapped to the CPU
*                                      with a CPU code of INVALID_ENTRY_ERROR
*                                      CPSS_PACKET_CMD_DROP_HARD_E
*                                      CPSS_PACKET_CMD_DROP_SOFT_E
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on HW error.
* @retval GT_BAD_PARAM             - bad paramter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfMplsInvalidEntryCmdSet
(
    IN GT_U8                               devNum,
    IN CPSS_PACKET_CMD_ENT                 cmd
)
{
#ifdef EXMXPM_FAMILY

    /* call device specific API */
    return cpssExMxPmMplsInvalidEntryCmdSet(devNum,cmd);

#else /* !EXMXPM_FAMILY */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(cmd);
    /* this request is not support */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */
}

/**
* @internal prvTgfMplsInvalidEntryCmdGet function
* @endinternal
*
* @brief   Get the command assigned to MPLS packets if any of the following is true:
*         - The MPLS Interface Table entry is not valid
*         - The incoming label is out of label space boundaries
* @param[in] devNum                   - device number
*
* @param[out] cmdPtr                   - (pointer to) Valid commands:
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E - Packets are trapped to the CPU
*                                      with a CPU code of INVALID_ENTRY_ERROR
*                                      CPSS_PACKET_CMD_DROP_HARD_E
*                                      CPSS_PACKET_CMD_DROP_SOFT_E
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on HW error.
* @retval GT_BAD_PARAM             - bad paramter
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfMplsInvalidEntryCmdGet
(
    IN  GT_U8                               devNum,
    OUT CPSS_PACKET_CMD_ENT                 *cmdPtr
)
{
#ifdef EXMXPM_FAMILY
    GT_STATUS rc = GT_OK;
    CPSS_PACKET_CMD_ENT    cmd;

    /* call device specific API */
    rc = cpssExMxPmMplsInvalidEntryCmdGet(devNum,&cmd);
    if(rc != GT_OK)
        return rc;

    *cmdPtr = cmd;
    return rc;

#else /* !EXMXPM_FAMILY */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(cmdPtr);
    /* this request is not support */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */
}

