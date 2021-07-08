/******************************************************************************
 *              (c), Copyright 2018, Marvell International Ltd.                 *
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
* @file tgfQcnGen.c
*
* @brief Generic API for QCN Trigger configuration.
*
* @version   1
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
#include <common/tgfPortGen.h>

#ifdef CHX_FAMILY
    #include <cpss/dxCh/dxChxGen/port/cpssDxChPortTx.h>
    #include <cpss/dxCh/dxChxGen/port/cpssDxChPortBufMg.h>
    #include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
    #include <cpss/dxCh/dxChxGen/config/cpssDxChCfgInit.h>
    #include <cpss/dxCh/dxChxGen/port/cpssDxChPortPfc.h>
    #include <cpss/dxCh/dxChxGen/port/cpssDxChPortTxDba.h>
    #include <cpss/dxCh/dxChxGen/port/cpssDxChPortCn.h>
#endif /* CHX_FAMILY */

/******************************************************************************\
 *                       CPSS generic API section                             *
\******************************************************************************/

/**
 * @internal internal_cpssDxChPortCnModeEnableSet function
 * @endinternal
 *
 * @brief   Enable/Disable Congestion Notification (CN) mechanism.
 *
 * @param[in] devNum                   - device number.
 * @param[in] enable                   - Enable/disable congestion notification mechanism.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong device number
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 * @note Do not use the global enable/disable CCFC under traffic - instead use
 *       cpssDxChPortCnQueueAwareEnableSet to enable/disable queue for BCN.
 *
 */
GT_STATUS prvTgfPortCnModeEnableSet
(
    IN GT_U8                       devNum,
    IN CPSS_DXCH_PORT_CN_MODE_ENT  enable
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;               /* function return value */

    /* call device specific API */
    rc = cpssDxChPortCnModeEnableSet(devNum, enable);

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortCnModeEnableSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;

#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enable);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortCnModeEnableGet function
* @endinternal
*
* @brief   Gets the current status of Congestion Notification (CN) mode.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - (pointer to) status of Congestion Notification mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortCnModeEnableGet
(
    IN  GT_U8                      devNum,
    OUT CPSS_DXCH_PORT_CN_MODE_ENT *enablePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;               /* function return value */

    /* call device specific API */
    rc = cpssDxChPortCnModeEnableGet(devNum, enablePtr);

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortCnModeEnableGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;

#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enablePtr);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortCnMessageTriggeringStateSet function
* @endinternal
*
* @brief   Sets CNM Triggering State Variables
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] tcQueue                  - traffic class queue (0..7).
* @param[in] qcnSampleInterval        -  parameter that defines the probability to trigger
*                                      the next CN frame based on the current value of the
*                                      calculated feedback (Fb), measured in Bytes.
*                                      (APPLICABLE RANGES: 0..0xFFFFF)
* @param[in] qlenOld                  - a snapshot of current length of the queue in buffers,
*                                      taken when the QcnSampleInterval expires.
*                                      (APPLICABLE RANGES: 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number,portNum,tc
* @retval GT_OUT_OF_RANGE          - on wrong qcnSampleInterval, qlenOld
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note qlenOld reset value and QcnSampleInterval init value must be set by the
*       application prior to configuring the queue to be CN aware
*
*/
GT_STATUS prvTgfPortCnMessageTriggeringStateSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U8                   tcQueue,
    IN  GT_U32                  qcnSampleInterval,
    IN  GT_U32                  qlenOld
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;               /* function return value */

    /* call device specific API */
    rc = cpssDxChPortCnMessageTriggeringStateSet(devNum, portNum, tcQueue,
                                                 qcnSampleInterval, qlenOld);

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortCnMessageTriggeringStateSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(tcQueue);
    TGF_PARAM_NOT_USED(qcnSampleInterval);
    TGF_PARAM_NOT_USED(qLenOld);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortCnMessageTriggeringStateGet function
* @endinternal
*
* @brief   Gets CNM Triggering State Variables
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] tcQueue                  - traffic class queue (0..7).
*
* @param[out] qcnSampleIntervalPtr     -  (pointer to) parameter that defines the probability to trigger
*                                      the next CN frame based on the current value of the
*                                      calculated feedback (Fb), measured in Bytes.
* @param[out] qlenOldPtr               - (pointer to) snapshot of current length of the queue in buffers,
*                                      taken when the QcnSampleInterval expires.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number,portNum,tc
* @retval GT_OUT_OF_RANGE          - on wrong qcnSampleInterval, qlenOld
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note qlenOld reset value and QcnSampleInterval init value must be set by the
*       application prior to configuring the queue to be CN aware
*
*/
GT_STATUS prvTgfPortCnMessageTriggeringStateGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U8                   tcQueue,
    OUT GT_U32                  *qcnSampleIntervalPtr,
    OUT GT_U32                  *qlenOldPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;               /* function return value */

    /* call device specific API */
    rc = cpssDxChPortCnMessageTriggeringStateGet(devNum, portNum, tcQueue,
                                                 qcnSampleIntervalPtr, qlenOldPtr);

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortCnMessageTriggeringStateGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(tcQueue);
    TGF_PARAM_NOT_USED(&qcnSampleIntervalPtr);
    TGF_PARAM_NOT_USED(&qLenOldPtr);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortCnEtherTypeSet function
* @endinternal
*
* @brief   Sets the EtherType to identify CN frames.
*         This EtherType also inserted in the CN header of generated CN frames.
*
* @param[in] devNum                   - device number.
* @param[in] etherType                - EtherType for CN frames (0..0xFFFF).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range etherType values.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The CN EtherType must be unique in the system.
*
*/
GT_STATUS prvTgfPortCnEtherTypeSet
(
    IN GT_U8    devNum,
    IN GT_U32   etherType
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;               /* function return value */

    /* call device specific API */
    rc = cpssDxChPortCnEtherTypeSet(devNum, etherType);

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortCnEtherTypeSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(etherType);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortCnEtherTypeGet function
* @endinternal
*
* @brief   Gets the EtherType for CN frames. The EtherType inserted in the CN
*         header.
*
* @param[in] devNum                   - device number.
*
* @param[out] etherTypePtr             - (pointer to) EtherType for CN frames.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortCnEtherTypeGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *etherTypePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;               /* function return value */

    /* call device specific API */
    rc = cpssDxChPortCnEtherTypeGet(devNum, etherTypePtr);

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortCnEtherTypeGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(etherTypePtr);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortCnProfileQueueConfigSet function
* @endinternal
*
* @brief   Sets CN profile configurations for given tc queue.
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - profile set.
* @param[in] tcQueue                  - traffic class queue (0..7).
* @param[in] cnProfileCfgPtr          - pointer to CN Profile configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number, profile set
*                                       or traffic class queue
* @retval GT_OUT_OF_RANGE          - on out of range threshold value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note CN Profile is part of the Tail-Drop Profile.
*       To bind port to Tail-Drop Profile use cpssDxChPortTxBindPortToDpSet.
*
*/
GT_STATUS prvTgfPortCnProfileQueueConfigSet
(
    IN GT_U8                               devNum,
    IN CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    IN GT_U8                               tcQueue,
    IN CPSS_PORT_CN_PROFILE_CONFIG_STC     *cnProfileCfgPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;               /* function return value */

    /* call device specific API */
    rc = cpssDxChPortCnProfileQueueConfigSet(devNum, profileSet, tcQueue, cnProfileCfgPtr);

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortCnProfileQueueConfigSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(profileSet);
    TGF_PARAM_NOT_USED(tcQueue);
    TGF_PARAM_NOT_USED(cnProfileCfgPtr);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortCnProfileQueueConfigGet function
* @endinternal
*
* @brief   Gets CN profile configurations for given tc queue.
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - profile set.
* @param[in] tcQueue                  - traffic class queue (0..7).
*
* @param[out] cnProfileCfgPtr          - pointer to CN Profile configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number, profile set
*                                       or traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortCnProfileQueueConfigGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    IN  GT_U8                               tcQueue,
    OUT CPSS_PORT_CN_PROFILE_CONFIG_STC     *cnProfileCfgPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;               /* function return value */

    /* call device specific API */
    rc = cpssDxChPortCnProfileQueueConfigGet(devNum, profileSet, tcQueue, cnProfileCfgPtr);

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortCnProfileQueueConfigGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(profileSet);
    TGF_PARAM_NOT_USED(tcQueue);
    TGF_PARAM_NOT_USED(cnProfileCfgPtr);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortCnFbCalcConfigSet function
* @endinternal
*
* @brief   Sets Fb (Feedback) calculation configuration.
*
* @param[in] devNum                   - device number.
* @param[in] fbCalcCfgPtr             - (pointer to) feedback Calculation configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To implement CCFC functionality set fbCalcCfgPtr fields to:
*       deltaEnable = 0
*
*/
GT_STATUS prvTgfPortCnFbCalcConfigSet
(
    IN GT_U8                                  devNum,
    IN CPSS_PORT_CN_FB_CALCULATION_CONFIG_STC *fbCalcCfgPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;               /* function return value */

    /* call device specific API */
    rc = cpssDxChPortCnFbCalcConfigSet(devNum, fbCalcCfgPtr);

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortCnFbCalcConfigSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(fbCalcCfgPtr);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortCnFbCalcConfigGet function
* @endinternal
*
* @brief   Gets Fb (Feedback) calculation configuration.
*
* @param[in] devNum                   - device number.
*
* @param[out] fbCalcCfgPtr             - (pointer to) feedback Calculation configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortCnFbCalcConfigGet
(
    IN  GT_U8                                  devNum,
    OUT CPSS_PORT_CN_FB_CALCULATION_CONFIG_STC *fbCalcCfgPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;               /* function return value */

    /* call device specific API */
    rc = cpssDxChPortCnFbCalcConfigGet(devNum, fbCalcCfgPtr);

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortCnFbCalcConfigGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(fbCalcCfgPtr);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortCnSampleEntrySet function
* @endinternal
*
* @brief   Sets CN sample interval entry.
*
* @param[in] devNum                   - device number.
* @param[in] entryIndex               - index: qFb 3 msb. (0..7)
* @param[in] entryPtr                 - (pointer to) entry settings.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, or entry index
* @retval GT_OUT_OF_RANGE          - on out of range interval
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1.To implement CCFC functionality set entryPtr fields to:
*       interval = (MTU+256b)/16
*       randBitmap = 0
*       2.To achieve uniform distribution of random values, clear the interval
*       to be randomized to 0. For example, if interval = 0xF0 and
*       randBitmap = 0x0F then the sampling interval is a random value
*       in [0xF0,0xFF].
*
*/
GT_STATUS prvTgfPortCnSampleEntrySet
(
    IN GT_U8                                  devNum,
    IN GT_U8                                  entryIndex,
    IN CPSS_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC *entryPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;               /* function return value */

    /* call device specific API */
    rc = cpssDxChPortCnSampleEntrySet(devNum, entryIndex, entryPtr);

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortCnSampleEntrySet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(entryIndex);
    TGF_PARAM_NOT_USED(entryPtr);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortCnSampleEntryGet function
* @endinternal
*
* @brief   Gets CN sample interval entry.
*
* @param[in] devNum                   - device number.
* @param[in] entryIndex               - index: qFb 3 msb. (0..7)
*
* @param[out] entryPtr                 - (pointer to) entry settings.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, or entry index
* @retval GT_OUT_OF_RANGE          - on out of range interval
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortCnSampleEntryGet
(
    IN  GT_U8                                  devNum,
    IN  GT_U8                                  entryIndex,
    OUT CPSS_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC *entryPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;               /* function return value */

    /* call device specific API */
    rc = cpssDxChPortCnSampleEntryGet(devNum, entryIndex, entryPtr);

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortCnSampleEntryGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(entryIndex);
    TGF_PARAM_NOT_USED(entryPtr);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortCnQueueStatusModeEnableSet function
* @endinternal
*
* @brief   Enable/Disable queue status mode. When enabled, CNM is sent to a
*         configurable target that examines the queue status.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - GT_TRUE: CNM is sent to a targetDev/targetPort.
*                                      GT_FALSE: CNM is sent to a sampled packet's source.
* @param[in] targetHwDev              Target HW device number for the CNM.
*                                      Relevant only if enable==GT_TRUE.
* @param[in] targetPort               - Target port for the CNM.
*                                      Relevant only if enable==GT_TRUE.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range targetDev or targetPort
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortCnQueueStatusModeEnableSet
(
    IN GT_U8            devNum,
    IN GT_BOOL          enable,
    IN GT_HW_DEV_NUM    targetHwDev,
    IN GT_PORT_NUM      targetPort
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;               /* function return value */

    /* call device specific API */
    rc = cpssDxChPortCnQueueStatusModeEnableSet(devNum, enable, targetHwDev, targetPort);

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortCnQueueStatusModeEnableSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enable);
    TGF_PARAM_NOT_USED(targetHwDev);
    TGF_PARAM_NOT_USED(targetPort);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortCnQueueStatusModeEnableGet function
* @endinternal
*
* @brief   Gets the current status of queue status mode.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - (pointer to) status of queue status mode.
*                                      GT_TRUE: CNM is sent to a targetDev/targetPort.
*                                      GT_FALSE: CNM is sent to a sampled packet's source.
* @param[out] targetHwDevPtr           (pointer to) Target HW device number for the CNM.
*                                      Relevant only if enable==GT_TRUE.
* @param[out] targetPortPtr            - (pointer to) Target port for the CNM.
*                                      Relevant only if enable==GT_TRUE.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortCnQueueStatusModeEnableGet
(
    IN  GT_U8           devNum,
    OUT GT_BOOL         *enablePtr,
    OUT GT_HW_DEV_NUM   *targetHwDevPtr,
    OUT GT_PORT_NUM     *targetPortPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;               /* function return value */

    /* call device specific API */
    rc = cpssDxChPortCnQueueStatusModeEnableGet(devNum, enablePtr, targetHwDevPtr, targetPortPtr);

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortCnQueueStatusModeEnableGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enablePtr);
    TGF_PARAM_NOT_USED(targetHwDevPtr);
    TGF_PARAM_NOT_USED(targetPortPtr);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortCnMessageGenerationConfigSet function
* @endinternal
*
* @brief   Sets CNM generation configuration.
*
* @param[in] devNum                   - device number.
* @param[in] cnmGenerationCfgPtr      - (pointer to) CNM generation configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortCnMessageGenerationConfigSet
(
    IN GT_U8                               devNum,
    IN CPSS_PORT_CNM_GENERATION_CONFIG_STC *cnmGenerationCfgPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;               /* function return value */

    /* call device specific API */
    rc = cpssDxChPortCnMessageGenerationConfigSet(devNum, cnmGenerationCfgPtr);

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortCnMessageGenerationConfigSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(cnmGenerationCfgPtr);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortCnMessageGenerationConfigGet function
* @endinternal
*
* @brief   Gets CNM generation configuration.
*
* @param[in] devNum                   - device number.
*
* @param[out] cnmGenerationCfgPtr      - (pointer to) CNM generation configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortCnMessageGenerationConfigGet
(
    IN  GT_U8                               devNum,
    OUT CPSS_PORT_CNM_GENERATION_CONFIG_STC *cnmGenerationCfgPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;               /* function return value */

    /* call device specific API */
    rc = cpssDxChPortCnMessageGenerationConfigGet(devNum, cnmGenerationCfgPtr);

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortCnMessageGenerationConfigGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(cnmGenerationCfgPtr);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
 * @internal prvTgfPortCnDbaModeEnableSet function
 * @endinternal
 *
 * @brief   Enable/disable Dynamic Buffers Allocation (DBA) mode of resource
 *         management.
 *
 * @param[in] devNum                   - device number
 * @param[in] enable                   - GT_TRUE -  DBA mode enabled
 *                                      GT_FALSE - DBA mode disabled.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong device number, index
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvTgfPortCnDbaModeEnableSet
(
 IN  GT_U8           devNum,
 IN  GT_BOOL         enable
 )
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;               /* function return value */

    /* call device specific API */
    rc = cpssDxChPortCnDbaModeEnableSet(devNum, enable);

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortCnDbaModeEnableSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enable);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
 * @internal prvTgfPortCnDbaModeEnableGet function
 * @endinternal
 *
 * @brief   Get state of Dynamic Buffers Allocation (DBA) mode of resource
 *         management.
 *
 * @param[in] devNum                   - device number
 *
 * @param[out] enablePtr                - pointer to DBA mode state.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PTR               - on NULL pointer
 * @retval GT_BAD_PARAM             - on wrong device number, index
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvTgfPortCnDbaModeEnableGet
(
 IN  GT_U8           devNum,
 OUT GT_BOOL         *enablePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;               /* function return value */

    /* call device specific API */
    rc = cpssDxChPortCnDbaModeEnableGet(devNum, enablePtr);

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortCnDbaModeEnableGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enablePtr);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
 * @internal prvTgfPortCnDbaAvailableBuffSet function
 * @endinternal
 *
 * @brief   Set amount of buffers available for dynamic allocation for CN.
 *
 * @param[in] devNum                   - device number
 * @param[in] availableBuff            - amount of buffers available for dynamic allocation for
 *                                      CN (APPLICABLE RANGES: 0..0xFFFFF).
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong device number, index
 * @retval GT_OUT_OF_RANGE          - on out of range value
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvTgfPortCnDbaAvailableBuffSet
(
 IN  GT_U8           devNum,
 IN  GT_U32          availableBuff
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;               /* function return value */

    /* call device specific API */
    rc = cpssDxChPortCnDbaAvailableBuffSet(devNum, availableBuff);

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortCnAvailableBuffSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(availableBuff);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
 * @internal prvTgfPortCnDbaAvailableBuffGet function
 * @endinternal
 *
 * @brief   Get amount of buffers available for dynamic allocation for CN.
 *
 * @param[in] devNum                   - device number
 *
 * @param[out] availableBuffPtr         - pointer to amount of buffers available for dynamic
 *                                      allocation for CN.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong device number, index
 * @retval GT_BAD_PTR               - on NULL pointer
 * @retval GT_OUT_OF_RANGE          - on out of range value
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvTgfPortCnDbaAvailableBuffGet
(
 IN  GT_U8           devNum,
 OUT GT_U32          *availableBuffPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;               /* function return value */

    /* call device specific API */
    rc = cpssDxChPortCnDbaAvailableBuffGet(devNum, availableBuffPtr);

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortCnAvailableBuffGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(availableBuffPtr);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
 * @internal prvTgfPortCnTerminationEnableSet function
 * @endinternal
 *
 * @brief   Enable/Disable termination of CNM (Congestion Notification Message) on given port.
 *
 * @param[in] devNum                   - device number
 *
 * @param[in] portNum                  - port number.
 *
 * @param[in] enable                   - GT_TRUE: enable. CNM is terminated, i.e not transmited on this port.
 *                                      GT_FALSE: disable. CNM isn't terminated, i.e transmited on this port.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong device number, index
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvTgfPortCnTerminationEnableSet
(
 IN  GT_U8                 devNum,
 IN GT_PHYSICAL_PORT_NUM   portNum,
 IN GT_BOOL                enable
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;               /* function return value */

    /* call device specific API */
    rc = cpssDxChPortCnTerminationEnableSet(devNum, portNum, enable);

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortCnTerminationEnableSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(enable);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
 * @internal prvTgfPortCnTerminationEnableGet function
 * @endinternal
 *
 * @brief   Gets the status of CNM termination on given port..
 *
 * @param[in] devNum                   - device number
 *
 * @param[in] portNum                  - port number.
 *
 * @param[in] enable                   - (pointer to)
 *                                     GT_TRUE: enable. CNM is terminated, i.e not transmited on this port.
 *                                     GT_FALSE: disable. CNM isn't terminated, i.e transmited on this port.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong device number, index
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvTgfPortCnTerminationEnableGet
(
 IN  GT_U8                 devNum,
 IN  GT_PHYSICAL_PORT_NUM  portNum,
 OUT GT_BOOL               *enablePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;               /* function return value */

    /* call device specific API */
    rc = cpssDxChPortCnTerminationEnableGet(devNum, portNum, enablePtr);

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortCnTerminationEnableGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(enablePtr);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortCnDbaPoolAvailableBuffSet function
* @endinternal
*
* @brief   Set amount of pool buffers available for dynamic allocation for CN.
*
* @param[in] devNum                   - device number.
* @param[in] poolNum                  - pool number for QCN DBA buffers
*                                      (APPLICABLE RANGES: 0..1)
* @param[in] poolAvailableBuff        - amount of pool buffers available for dynamic allocation for CN.
*                                      (APPLICABLE RANGES: 0..0xFFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, or index
* @retval GT_OUT_OF_RANGE          - on out of range interval
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1.To implement CCFC functionality set entryPtr fields to:
*       interval = (MTU+256b)/16
*       randBitmap = 0
*       2.To achieve uniform distribution of random values, clear the interval
*       to be randomized to 0. For example, if interval = 0xF0 and
*       randBitmap = 0x0F then the sampling interval is a random value
*       in [0xF0,0xFF].
*
*/
GT_STATUS prvTgfPortCnDbaPoolAvailableBuffSet
(
    IN GT_U8                                  devNum,
    IN  GT_U32                                poolNum,
    IN  GT_U32                                poolAvailableBuff
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;               /* function return value */

    /* call device specific API */
    rc = cpssDxChPortCnDbaPoolAvailableBuffSet(devNum, poolNum, poolAvailableBuff);

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortCnDbaPoolAvailableBuffSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(poolNum);
    TGF_PARAM_NOT_USED(poolAvailableBuff);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortCnDbaPoolAvailableBuffGet function
* @endinternal
*
* @brief   Get amount of pool buffers available for dynamic allocation for CN.
*
* @param[in] devNum                   - device number.
* @param[in] poolNum                  - pool number for QCN DBA buffers
*                                      (APPLICABLE RANGES: 0..1)
* @param[out] availableBuffPtr        - pointer to amount of pool buffers available for dynamic allocation for CN.
*                                      (APPLICABLE RANGES: 0..0xFFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, or index
* @retval GT_OUT_OF_RANGE          - on out of range interval
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortCnDbaPoolAvailableBuffGet
(
    IN  GT_U8                                  devNum,
    IN  GT_U32                                 poolNum,
    OUT GT_U32                                 *poolAvailableBuffPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;               /* function return value */

    /* call device specific API */
    rc = cpssDxChPortCnDbaPoolAvailableBuffGet(devNum, poolNum, poolAvailableBuffPtr);

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortCnDbaPoolAvailableBuffGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(poolNum);
    TGF_PARAM_NOT_USED(poolAvailableBuffPtr);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}


/**
* @internal prvTgfPortTxTailDropGlobalParamsSet function
* @endinternal
*
* @brief   Set amount of available buffers for QCN DBA.
*
* @param[in] devNum                   -  Device number
* @param[in] resourceMode             -  Defines which free buffers resource is used to calculate the
*                                       dynamic buffer limit for the Port/Queue limits(global or pool)
* @param[in] globalAvailableBuffers   -  Global amount of available buffers for dynamic buffers allocation
* @param[in] pool0AvailableBuffers    -  Pool 0 amount of available buffers for dynamic buffers allocation
* @param[in] pool1AvailableBuffers    -  Pool 1 amount of available buffers for dynamic buffers allocation
*
* @retval GT_OK                       - on success
* @retval GT_FAIL                     - on error
* @retval GT_HW_ERROR                 - on hardware error
* @retval GT_BAD_PARAM                - on invalid input parameters value
* @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
*/
GT_STATUS prvTgfPortTxTailDropGlobalParamsSet
(
    IN  GT_U8                                     devNum,
    IN  CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_ENT  resourceMode,
    IN  GT_U32                                    globalAvailableBuffers,
    IN  GT_U32                                    pool0AvailableBuffers,
    IN  GT_U32                                    pool1AvailableBuffers
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;               /* function return value */

    /* call device specific API */
    rc = cpssDxChPortTxTailDropGlobalParamsSet(devNum, resourceMode, globalAvailableBuffers,
                                               pool0AvailableBuffers, pool1AvailableBuffers);

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortTxTailDropGlobalParamsSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(resourceMode);
    TGF_PARAM_NOT_USED(globalAvailableBuffers);
    TGF_PARAM_NOT_USED(pool0AvailableBuffers);
    TGF_PARAM_NOT_USED(pool1AvailableBuffers);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortTxTailDropGlobalParamsGet function
* @endinternal
*
* @brief   Get amount of available buffers for QCN DBA.
*
* @param[in] devNum                   -  Device number
* @param[in] resourceMode             -  Defines which free buffers resource is used to calculate the
*                                       dynamic buffer limit for the Port/Queue limits(global or pool)
* @param[in] globalAvailableBuffers   - (pointer to) Global amount of available buffers for dynamic buffers allocation
* @param[in] pool0AvailableBuffers    - (pointer to) Pool 0 amount of available buffers for dynamic buffers allocation
* @param[in] pool1AvailableBuffers    - (pointer to) Pool 1 amount of available buffers for dynamic buffers allocation
*
* @retval GT_OK                       - on success
* @retval GT_FAIL                     - on error
* @retval GT_HW_ERROR                 - on hardware error
* @retval GT_BAD_PARAM                - on invalid input parameters value
* @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
*/
GT_STATUS prvTgfPortTxTailDropGlobalParamsGet
(
    IN  GT_U8                                     devNum,
    IN  CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_ENT  *resourceMode,
    IN  GT_U32                                    *globalAvailableBuffers,
    IN  GT_U32                                    *pool0AvailableBuffers,
    IN  GT_U32                                    *pool1AvailableBuffers
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;               /* function return value */

    /* call device specific API */
    rc = cpssDxChPortTxTailDropGlobalParamsGet(devNum, resourceMode, globalAvailableBuffers,
                                               pool0AvailableBuffers, pool1AvailableBuffers);

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortTxTailDropGlobalParamsSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(resourceMode);
    TGF_PARAM_NOT_USED(globalAvailableBuffers);
    TGF_PARAM_NOT_USED(pool0AvailableBuffers);
    TGF_PARAM_NOT_USED(pool1AvailableBuffers);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortCnQcnTriggerSet function
* @endinternal
*
* @brief   Sets QCN trigger threshold.
*
* @param[in] devNum                   - device number.
* @param[in] index                    - index to trigger threshold table.
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] qcnTriggerPtr            - (pointer to) qcn trigger threshold.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, or index
* @retval GT_OUT_OF_RANGE          - on out of range interval
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvTgfPortCnQcnTriggerSet
(
    IN GT_U8                                  devNum,
    IN GT_U32                                 index,
    IN  CPSS_DXCH_PORT_CN_TRIGGER_CONFIG_STC *qcnTriggerPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;               /* function return value */

    /* call device specific API */
    rc = cpssDxChPortCnQcnTriggerSet(devNum, index, qcnTriggerPtr);

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortCnQcnTriggerSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(index);
    TGF_PARAM_NOT_USED(qcnTriggerPtr);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortCnQcnTriggerGet function
* @endinternal
*
* @brief   Gets configured QCN trigger threshold.
*
* @param[in] devNum                   - device number.
* @param[in] index                    - index to trigger threshold table.
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] qcnTriggerPtr            - (pointer to) qcn trigger threshold.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, or entry index
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortCnQcnTriggerGet
(
    IN  GT_U8                                  devNum,
    IN  GT_U32                                 index,
    OUT CPSS_DXCH_PORT_CN_TRIGGER_CONFIG_STC   *qcnTriggerPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;               /* function return value */

    /* call device specific API */
    rc = cpssDxChPortCnQcnTriggerGet(devNum, index, qcnTriggerPtr);

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortCnQcnTriggerGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(index);
    TGF_PARAM_NOT_USED(qcnTriggerPtr);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}


/**
 * @internal prvTgfPortCnCNMEnableSet function
 * @endinternal
 *
 * @brief   Enable/disable CNM generation for CN untagged frames.
 *
 * @param[in] devNum                   - device number
 * @param[in] enable                   - GT_TRUE - CNM generation enabled
 *                                      GT_FALSE - CNM generation disabled.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong device number, index
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvTgfPortCnCNMEnableSet
(
 IN  GT_U8           devNum,
 IN  GT_BOOL         enable
 )
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;               /* function return value */

    /* call device specific API */
    rc = cpssDxChPortCnCNMEnableSet(devNum, enable);

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortCnCNMEnableSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enable);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
 * @internal prvTgfPortCnCNMEnableGet function
 * @endinternal
 *
 * @brief   Get state of CNM enable for CN untagged frames.
 *
 * @param[in] devNum                   - device number
 * @param[out] enablePtr                - pointer to CNM enable state.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PTR               - on NULL pointer
 * @retval GT_BAD_PARAM             - on wrong device number, index
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvTgfPortCnCNMEnableGet
(
 IN  GT_U8           devNum,
 OUT GT_BOOL         *enablePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;               /* function return value */

    /* call device specific API */
    rc = cpssDxChPortCnCNMEnableGet(devNum, enablePtr);

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortCnCNMEnableGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enablePtr);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}


/**
* @internal prvTgfPortCnCongestedQPriorityLocationSet function
* @endinternal
*
* @brief   Sets location of the congested q/priority bits in the CNM header for
*          parsing and generation of CNM.
*
* @param[in] devNum                     - device number.
* @param[in] congestedQPriorityLocation - congested q/priority location.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortCnCongestedQPriorityLocationSet
(
    IN GT_U8                                               devNum,
    IN CPSS_DXCH_PORT_CN_CONGESTED_Q_PRIORITY_LOCATION_ENT congestedQPriorityLocation
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;               /* function return value */

    /* call device specific API */
    rc = cpssDxChPortCnCongestedQPriorityLocationSet(devNum, congestedQPriorityLocation);

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortCnCongestedQPriorityLocationSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(congestedQPriorityLocation);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortCnCongestedQPriorityLocationGet function
* @endinternal
*
* @brief   Gets location of congested q/priority bits in CNM header.
*
* @param[in] devNum                         - device number.
* @param[out] congestedQPriorityLocationPtr - (pointer to) congested q/priority location.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortCnCongestedQPriorityLocationGet
(
    IN  GT_U8                                               devNum,
    OUT CPSS_DXCH_PORT_CN_CONGESTED_Q_PRIORITY_LOCATION_ENT *congestedQPriorityLocationPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;               /* function return value */

    /* call device specific API */
    rc = cpssDxChPortCnCongestedQPriorityLocationGet(devNum, congestedQPriorityLocationPtr);

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortCnCongestedQPriorityLocationGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(congestedQPriorityLocationPtr);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortCnProfilePortConfigSet function
* @endinternal
*
* @brief   Sets CN profile configurations for given port profile.
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - profile set.
* @param[in] cnProfileCfgPtr          - pointer to CN Profile configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number, profile set
* @retval GT_OUT_OF_RANGE          - on out of range threshold value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note CN Profile is part of the Tail-Drop Profile.
*       To bind port to Tail-Drop Profile use cpssDxChPortTxBindPortToDpSet.
*
*/
GT_STATUS prvTgfPortCnProfilePortConfigSet
(
    IN GT_U8                               devNum,
    IN CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    IN CPSS_PORT_CN_PROFILE_CONFIG_STC     *cnProfileCfgPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;               /* function return value */

    /* call device specific API */
    rc = cpssDxChPortCnProfilePortConfigSet(devNum, profileSet, cnProfileCfgPtr);

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortCnProfilePortConfigSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(profileSet);
    TGF_PARAM_NOT_USED(cnProfileCfgPtr);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPortCnProfilePortConfigGet function
* @endinternal
*
* @brief   Gets CN profile configurations for given port profile.
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - profile set.
* @param[out] cnProfileCfgPtr          - pointer to CN Profile configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number, profile set
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortCnProfilePortConfigGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    OUT CPSS_PORT_CN_PROFILE_CONFIG_STC     *cnProfileCfgPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;               /* function return value */

    /* call device specific API */
    rc = cpssDxChPortCnProfilePortConfigGet(devNum, profileSet, cnProfileCfgPtr);

    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPortCnProfilePortConfigGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(profileSet);
    TGF_PARAM_NOT_USED(cnProfileCfgPtr);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

