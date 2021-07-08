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
* @file tgfStcGen.c
*
* @brief Generic API for STC APIs. (Sampling to CPU)
*
* @version   3
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
#include <common/tgfStcGen.h>

#ifdef CHX_FAMILY
    #include <cpss/dxCh/dxChxGen/mirror/cpssDxChStc.h>
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    #include <cpss/exMxPm/exMxPmGen/monitor/cpssExMxPmStc.h>
#endif /* EXMXPM_FAMILY */



/******************************************************************************\
 *                       CPSS generic API section                             *
\******************************************************************************/


/**
* @internal prvTgfStcIngressCountModeSet function
* @endinternal
*
* @brief   Set the type of packets subject to Ingress Sampling to CPU.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] mode                     - PRV_TGF_STC_COUNT_ALL_PACKETS_E - All packets without
*                                      any MAC-level errors.
*                                      PRV_TGF_STC_COUNT_NON_DROPPED_PACKETS_E -
*                                      only non-dropped packets.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfStcIngressCountModeSet
(
    IN  PRV_TGF_STC_COUNT_MODE_ENT  mode
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
#ifdef EXMXPM_FAMILY
        /* call device specific API */
        rc = cpssExMxPmStcIngressCountModeSet(devNum, (CPSS_EXMXPM_STC_COUNT_MODE_ENT)mode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmStcIngressCountModeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
#endif

#ifdef CHX_FAMILY
        /* call device specific API */
        rc = cpssDxChStcIngressCountModeSet(devNum, (CPSS_DXCH_STC_COUNT_MODE_ENT)mode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChStcIngressCountModeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
#endif /* CHX_FAMILY */
    }

    return rc1;
}

/**
* @internal prvTgfStcIngressCountModeGet function
* @endinternal
*
* @brief   Get the type of packets that are subject to Ingress Sampling to CPU.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
*
* @param[out] modePtr                  - PRV_TGF_STC_COUNT_ALL_PACKETS_E = All packets without
*                                      any MAC-level errors.
*                                      PRV_TGF_STC_COUNT_NON_DROPPED_PACKETS_E -
*                                      only non-dropped packets.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfStcIngressCountModeGet
(
    OUT PRV_TGF_STC_COUNT_MODE_ENT  *modePtr
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_STC_COUNT_MODE_ENT  pumaMode;
#endif
#ifdef CHX_FAMILY
    CPSS_DXCH_STC_COUNT_MODE_ENT    dxMode;
#endif

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
#ifdef EXMXPM_FAMILY
        /* call device specific API */
        rc = cpssExMxPmStcIngressCountModeGet(devNum, &pumaMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmStcIngressCountModeGet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
        *modePtr = (PRV_TGF_STC_COUNT_MODE_ENT)pumaMode;
#endif

#ifdef CHX_FAMILY
        /* call device specific API */
        rc = cpssDxChStcIngressCountModeGet(devNum, &dxMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChStcIngressCountModeGet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
        *modePtr = (PRV_TGF_STC_COUNT_MODE_ENT)dxMode;
#endif /* CHX_FAMILY */

        break;
    }

    return rc1;
}

/**
* @internal prvTgfStcReloadModeSet function
* @endinternal
*
* @brief   Set the type of Sampling To CPU (STC) count reload mode.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] stcType                  - Sampling To CPU (STC) type: ingress or egress
* @param[in] mode                     - Sampling to CPU (STC) Reload mode
*                                      PRV_TGF_STC_COUNT_RELOAD_CONTINUOUS_E - contiuous mode
*                                      PRV_TGF_STC_COUNT_RELOAD_TRIGGERED_E - triggered mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number,STC type or mode.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfStcReloadModeSet
(
    IN  PRV_TGF_STC_TYPE_ENT                stcType,
    IN  PRV_TGF_STC_COUNT_RELOAD_MODE_ENT   mode
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
#ifdef EXMXPM_FAMILY
        /* call device specific API */
        rc = cpssExMxPmStcReloadModeSet(devNum,
                (CPSS_EXMXPM_STC_TYPE_ENT)stcType , (CPSS_EXMXPM_STC_COUNT_RELOAD_MODE_ENT)mode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmStcReloadModeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
#endif

#ifdef CHX_FAMILY
        /* call device specific API */
        rc = cpssDxChStcReloadModeSet(devNum,
                (CPSS_DXCH_STC_TYPE_ENT)stcType , (CPSS_DXCH_STC_COUNT_RELOAD_MODE_ENT)mode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChStcReloadModeSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
#endif /* CHX_FAMILY */
    }

    return rc1;
}

/**
* @internal prvTgfStcReloadModeGet function
* @endinternal
*
* @brief   Get the type of Sampling To CPU (STC) count reload mode.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] stcType                  - Sampling To CPU (STC) type: ingress or egress
*
* @param[out] modePtr                  - (pointer to) Sampling to CPU (STC) Reload mode
*                                      PRV_TGF_STC_COUNT_RELOAD_CONTINUOUS_E - contiuous mode
*                                      PRV_TGF_STC_COUNT_RELOAD_TRIGGERED_E - triggered mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number or STC type.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfStcReloadModeGet
(
    IN  PRV_TGF_STC_TYPE_ENT                stcType,
    OUT PRV_TGF_STC_COUNT_RELOAD_MODE_ENT   *modePtr
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_STC_COUNT_RELOAD_MODE_ENT   pumaMode;
#endif
#ifdef CHX_FAMILY
    CPSS_DXCH_STC_COUNT_RELOAD_MODE_ENT     dxMode;
#endif

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
#ifdef EXMXPM_FAMILY
        /* call device specific API */
        rc = cpssExMxPmStcReloadModeGet(devNum,
                (CPSS_EXMXPM_STC_TYPE_ENT)stcType,&pumaMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmStcReloadModeGet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
        *modePtr = (PRV_TGF_STC_COUNT_RELOAD_MODE_ENT)pumaMode;
#endif

#ifdef CHX_FAMILY
        /* call device specific API */
        rc = cpssDxChStcReloadModeGet(devNum,
                (CPSS_DXCH_STC_TYPE_ENT)stcType,&dxMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChStcReloadModeGet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
        *modePtr = (PRV_TGF_STC_COUNT_RELOAD_MODE_ENT)dxMode;
#endif /* CHX_FAMILY */

        break;
    }

    return rc1;
}

/**
* @internal prvTgfStcEnableSet function
* @endinternal
*
* @brief   Enable/Disable Sampling To CPU (STC).
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] stcType                  - Sampling To CPU (STC) type: ingress or egress
* @param[in] enable                   - GT_TRUE =  Sampling To CPU (STC)
*                                      GT_FALSE = disable Sampling To CPU (STC)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number or STC type.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfStcEnableSet
(
    IN  PRV_TGF_STC_TYPE_ENT    stcType,
    IN  GT_BOOL                     enable
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
#ifdef EXMXPM_FAMILY
        /* call device specific API */
        rc = cpssExMxPmStcEnableSet(devNum,
                (CPSS_EXMXPM_STC_TYPE_ENT)stcType , enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmStcEnableSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
#endif

#ifdef CHX_FAMILY
        /* call device specific API */
        rc = cpssDxChStcEnableSet(devNum,
                (CPSS_DXCH_STC_TYPE_ENT)stcType , enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChStcEnableSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
#endif /* CHX_FAMILY */
    }

    return rc1;
}

/**
* @internal prvTgfStcEnableGet function
* @endinternal
*
* @brief   Get the status of Sampling To CPU (STC) (Enabled/Disabled).
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] stcType                  - Sampling To CPU (STC) type: ingress or egress
*
* @param[out] enablePtr                - GT_TRUE = enable Sampling To CPU (STC)
*                                      GT_FALSE = disable Sampling To CPU (STC)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number or STC type.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfStcEnableGet
(
    IN  PRV_TGF_STC_TYPE_ENT    stcType,
    OUT GT_BOOL                     *enablePtr
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
#ifdef EXMXPM_FAMILY
        /* call device specific API */
        rc = cpssExMxPmStcEnableGet(devNum,
                (CPSS_EXMXPM_STC_TYPE_ENT)stcType,enablePtr);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmStcReloadModeGet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
#endif

#ifdef CHX_FAMILY
        /* call device specific API */
        rc = cpssDxChStcEnableGet(devNum,
                (CPSS_DXCH_STC_TYPE_ENT)stcType,enablePtr);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChStcReloadModeGet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
#endif /* CHX_FAMILY */

        break;
    }

    return rc1;
}

/**
* @internal prvTgfStcPortLimitSet function
* @endinternal
*
* @brief   Set Sampling to CPU (STC) limit per port.
*         The limit to be loaded into the Count Down Counter.
*         This counter is decremented for each packet received on this port and is
*         subject to sampling according to the setting of STC Count mode.
*         When this counter is decremented from 1 to 0, the packet causing this
*         decrement is sampled to the CPU.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - the device number
* @param[in] port                     -  number  (DX-physical , Puma - virtual)
*                                      For STC of type PRV_TGF_STC_EGRESS_E, CPU port included.
* @param[in] stcType                  - Sampling To CPU (STC) type: ingress or egress
* @param[in] limit                    - Count Down Limit
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number,port number or STC type.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - limit is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfStcPortLimitSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        port,
    IN  PRV_TGF_STC_TYPE_ENT    stcType,
    IN  GT_U32                      limit
)
{
    GT_STATUS rc = GT_NOT_IMPLEMENTED;
#ifdef EXMXPM_FAMILY
    /* call device specific API */
    rc = cpssExMxPmStcPortLimitSet(devNum,(GT_U8)port,
            (CPSS_EXMXPM_STC_TYPE_ENT)stcType , limit);
#endif

#ifdef CHX_FAMILY
    /* call device specific API */
    rc = cpssDxChStcPortLimitSet(devNum,port,
            (CPSS_DXCH_STC_TYPE_ENT)stcType , limit);
#endif /* CHX_FAMILY */

    return rc;
}

/**
* @internal prvTgfStcPortLimitGet function
* @endinternal
*
* @brief   Get Sampling to CPU (STC) limit per port.
*         The limit to be loaded into the Count Down Counter.
*         This counter is decremented for each packet received on this port and is
*         subject to sampling according to the setting of STC Count mode.
*         When this counter is decremented from 1 to 0, the packet causing this
*         decrement is sampled to the CPU.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - the device number
* @param[in] port                     -  number  (DX-physical , Puma - virtual)
*                                      For STC of type PRV_TGF_STC_EGRESS_E, CPU port included.
* @param[in] stcType                  - Sampling To CPU (STC) type: ingress or egress
*
* @param[out] limitPtr                 - Count Down Limit
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number,port number or STC type.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfStcPortLimitGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        port,
    IN  PRV_TGF_STC_TYPE_ENT    stcType,
    OUT GT_U32                      *limitPtr
)
{
    GT_STATUS rc = GT_NOT_IMPLEMENTED;
#ifdef EXMXPM_FAMILY
    /* call device specific API */
    rc = cpssExMxPmStcPortLimitGet(devNum,(GT_U8)port,
            (CPSS_EXMXPM_STC_TYPE_ENT)stcType , limitPtr);
#endif

#ifdef CHX_FAMILY
    /* call device specific API */
    rc = cpssDxChStcPortLimitGet(devNum,port,
            (CPSS_DXCH_STC_TYPE_ENT)stcType , limitPtr);
#endif /* CHX_FAMILY */

    return rc;
}

/**
* @internal prvTgfStcPortReadyForNewLimitGet function
* @endinternal
*
* @brief   Check whether Sampling to CPU is ready to get new STC Limit Value per
*         port.
*         The function prvTgfStcPortLimitSet sets new limit value.
*         But only after device finishes handling of new limit value the
*         prvTgfStcPortLimitSet may be called once more.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - the device number
* @param[in] port                     -  number  (DX-physical , Puma - virtual)
*                                      For STC of type PRV_TGF_STC_EGRESS_E, CPU port included.
* @param[in] stcType                  - Sampling To CPU (STC) type: ingress or egress
*
* @param[out] isReadyPtr               - (pointer to) Is Ready value
*                                      GT_TRUE = Sampling to CPU ready to get new STC Limit Value
*                                      GT_FALSE = Sampling to CPU handles STC Limit Value yet and
*                                      is not ready to get new value.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number,port number or STC type.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfStcPortReadyForNewLimitGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        port,
    IN  PRV_TGF_STC_TYPE_ENT    stcType,
    OUT GT_BOOL                     *isReadyPtr
)
{
    GT_STATUS rc = GT_NOT_IMPLEMENTED;
#ifdef EXMXPM_FAMILY
    /* call device specific API */
    rc = cpssExMxPmStcPortReadyForNewLimitGet(devNum,(GT_U8)port,
            (CPSS_EXMXPM_STC_TYPE_ENT)stcType , isReadyPtr);
#endif

#ifdef CHX_FAMILY
    /* call device specific API */
    rc = cpssDxChStcPortReadyForNewLimitGet(devNum,port,
            (CPSS_DXCH_STC_TYPE_ENT)stcType , isReadyPtr);
#endif /* CHX_FAMILY */

    return rc;
}

/**
* @internal prvTgfStcPortCntrGet function
* @endinternal
*
* @brief   Get Sampling to CPU (STC) Counter per port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - the device number
* @param[in] port                     -  number  (DX-physical , Puma - virtual)
*                                      For STC of type PRV_TGF_STC_EGRESS_E, CPU port included.
* @param[in] stcType                  - Sampling To CPU (STC) type: ingress or egress
*
* @param[out] cntrPtr                  - (pointer to) STC Count down counter. This is the number of
*                                      packets left to send/receive in order that a packet will be
*                                      sampled to CPU and a new value will be loaded.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number,port number or STC type.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfStcPortCntrGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        port,
    IN  PRV_TGF_STC_TYPE_ENT    stcType,
    OUT GT_U32                      *cntrPtr
)
{
    GT_STATUS rc = GT_NOT_IMPLEMENTED;
#ifdef EXMXPM_FAMILY
    /* call device specific API */
    rc = cpssExMxPmStcPortCntrGet(devNum,(GT_U8)port,
            (CPSS_EXMXPM_STC_TYPE_ENT)stcType , cntrPtr);
#endif

#ifdef CHX_FAMILY
    /* call device specific API */
    rc = cpssDxChStcPortCountdownCntrGet(devNum,port,
            (CPSS_DXCH_STC_TYPE_ENT)stcType , cntrPtr);
#endif /* CHX_FAMILY */

    return rc;
}

/**
* @internal prvTgfStcPortSampledPacketsCntrSet function
* @endinternal
*
* @brief   Set the number of packets Sampled to CPU per port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - the device number
* @param[in] port                     -  number  (DX-physical , Puma - virtual)
*                                      For STC of type PRV_TGF_STC_EGRESS_E, CPU port included.
* @param[in] stcType                  - Sampling To CPU (STC) type: ingress or egress
* @param[in] cntr                     - STC Sampled to CPU packet's counter.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number,port number or STC type.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_OUT_OF_RANGE          - on wrong cntr
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfStcPortSampledPacketsCntrSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        port,
    IN  PRV_TGF_STC_TYPE_ENT    stcType,
    IN  GT_U32                      cntr
)
{
    GT_STATUS rc = GT_NOT_IMPLEMENTED;
#ifdef EXMXPM_FAMILY
    /* call device specific API */
    rc = cpssExMxPmStcPortSampledPacketsCntrSet(devNum,(GT_U8)port,
            (CPSS_EXMXPM_STC_TYPE_ENT)stcType , cntr);
#endif

#ifdef CHX_FAMILY
    /* call device specific API */
    rc = cpssDxChStcPortSampledPacketsCntrSet(devNum,port,
            (CPSS_DXCH_STC_TYPE_ENT)stcType , cntr);
#endif /* CHX_FAMILY */

    return rc;
}

/**
* @internal prvTgfStcPortSampledPacketsCntrGet function
* @endinternal
*
* @brief   Get the number of packets Sampled to CPU per port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - the device number
* @param[in] port                     -  number  (DX-physical , Puma - virtual)
*                                      For STC of type PRV_TGF_STC_EGRESS_E, CPU port included.
* @param[in] stcType                  - Sampling To CPU (STC) type: ingress or egress
*
* @param[out] cntrPtr                  - (pointer to) STC Sampled to CPU packet's counter.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number,port number or STC type.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfStcPortSampledPacketsCntrGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        port,
    IN  PRV_TGF_STC_TYPE_ENT    stcType,
    OUT GT_U32                      *cntrPtr
)
{
    GT_STATUS rc = GT_NOT_IMPLEMENTED;
#ifdef EXMXPM_FAMILY
    /* call device specific API */
    rc = cpssExMxPmStcPortSampledPacketsCntrGet(devNum,(GT_U8)port,
            (CPSS_EXMXPM_STC_TYPE_ENT)stcType , cntrPtr);
#endif

#ifdef CHX_FAMILY
    /* call device specific API */
    rc = cpssDxChStcPortSampledPacketsCntrGet(devNum,port,
            (CPSS_DXCH_STC_TYPE_ENT)stcType , cntrPtr);
#endif /* CHX_FAMILY */

    return rc;
}


