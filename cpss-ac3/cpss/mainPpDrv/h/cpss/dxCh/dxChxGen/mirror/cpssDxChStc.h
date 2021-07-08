/*******************************************************************************H
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
* @file cpssDxChStc.h
*
* @brief CPSS DxCh Sampling To CPU (STC) APIs
*
*
* @version   8
********************************************************************************
*/


#ifndef __cpssDxChStch
#define __cpssDxChStch

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/********* include ************************************************************/
#include <cpss/common/cpssTypes.h>
#include <cpssCommon/cpssPresteraDefs.h>

/**
* @enum CPSS_DXCH_STC_COUNT_MODE_ENT
 *
 * @brief Enum for Sampling To CPU (STC) Count mode.
*/
typedef enum{

    /** @brief All packets received without any
     *  MAClevel error are subject to
     *  Sampling To CPU (STC).
     */
    CPSS_DXCH_STC_COUNT_ALL_PACKETS_E,

    /** @brief Only non-dropped packets
     *  are subject to Sampling To CPU (STC)
     */
    CPSS_DXCH_STC_COUNT_NON_DROPPED_PACKETS_E

} CPSS_DXCH_STC_COUNT_MODE_ENT;


/**
* @enum CPSS_DXCH_STC_COUNT_RELOAD_MODE_ENT
 *
 * @brief Enum for Sampling To CPU (STC) count reload mode.
*/
typedef enum{

    /** @brief The port ingress/egress STC
     *  mechanism reloads the sampling frequency value that was last
     *  configured by the CPU.
     */
    CPSS_DXCH_STC_COUNT_RELOAD_CONTINUOUS_E,

    /** @brief The port ingress/egress STC
     *  mechanism only reloads a new sampling frequency value if it
     *  was updated by the CPU since the previous reload operation.
     *  If the CPU did not update the frequency value since the
     *  previous reload, sampling is halted until a new frequency
     *  value is updated by the CPU.
     *  When a new sampling frequency is loaded into the port
     *  internal counter, a CPU interrupt is generated, indicating
     *  that a new port sampling frequency can be configured, to be
     *  used on the next internal counter reload. Alternatively,
     *  the CPU can read a status register field, to learn whether
     *  the last sampling frequency has been loaded into the
     *  internal counter.
     */
    CPSS_DXCH_STC_COUNT_RELOAD_TRIGGERED_E

} CPSS_DXCH_STC_COUNT_RELOAD_MODE_ENT;


/**
* @enum CPSS_DXCH_STC_TYPE_ENT
 *
 * @brief Enum for Sampling To CPU (STC) type.
*/
typedef enum{

    /** Ingress Sampling To CPU (STC) */
    CPSS_DXCH_STC_INGRESS_E,

    /** Egress Sampling To CPU (STC) */
    CPSS_DXCH_STC_EGRESS_E

} CPSS_DXCH_STC_TYPE_ENT;


/**
* @internal cpssDxChStcIngressCountModeSet function
* @endinternal
*
* @brief   Set the type of packets subject to Ingress Sampling to CPU.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon.
*
* @param[in] devNum                   - the device number
* @param[in] mode                     - CPSS_DXCH_STC_COUNT_ALL_PACKETS_E - All packets without
*                                      any MAC-level errors.
*                                      CPSS_DXCH_STC_COUNT_NON_DROPPED_PACKETS_E -
*                                      only non-dropped packets.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChStcIngressCountModeSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_STC_COUNT_MODE_ENT    mode
);

/**
* @internal cpssDxChStcIngressCountModeGet function
* @endinternal
*
* @brief   Get the type of packets that are subject to Ingress Sampling to CPU.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon.
*
* @param[in] devNum                   - the device number
*
* @param[out] modePtr                  - CPSS_DXCH_STC_COUNT_ALL_PACKETS_E = All packets without
*                                      any MAC-level errors.
*                                      CPSS_DXCH_STC_COUNT_NON_DROPPED_PACKETS_E -
*                                      only non-dropped packets.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChStcIngressCountModeGet
(
    IN  GT_U8                           devNum,
    OUT CPSS_DXCH_STC_COUNT_MODE_ENT    *modePtr
);

/**
* @internal cpssDxChStcReloadModeSet function
* @endinternal
*
* @brief   Set the type of Sampling To CPU (STC) count reload mode.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon.
*
* @param[in] devNum                   - the device number
* @param[in] stcType                  - Sampling To CPU (STC) type: ingress or egress
* @param[in] mode                     - Sampling to CPU (STC) Reload mode
*                                      CPSS_DXCH_STC_COUNT_RELOAD_CONTINUOUS_E - contiuous mode
*                                      CPSS_DXCH_STC_COUNT_RELOAD_TRIGGERED_E - triggered mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number,STC type or mode.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChStcReloadModeSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_STC_TYPE_ENT                  stcType,
    IN  CPSS_DXCH_STC_COUNT_RELOAD_MODE_ENT     mode
);

/**
* @internal cpssDxChStcReloadModeGet function
* @endinternal
*
* @brief   Get the type of Sampling To CPU (STC) count reload mode.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon.
*
* @param[in] devNum                   - the device number
* @param[in] stcType                  - Sampling To CPU (STC) type: ingress or egress
*
* @param[out] modePtr                  - (pointer to) Sampling to CPU (STC) Reload mode
*                                      CPSS_DXCH_STC_COUNT_RELOAD_CONTINUOUS_E - contiuous mode
*                                      CPSS_DXCH_STC_COUNT_RELOAD_TRIGGERED_E - triggered mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number or STC type.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChStcReloadModeGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_STC_TYPE_ENT                  stcType,
    OUT CPSS_DXCH_STC_COUNT_RELOAD_MODE_ENT     *modePtr
);

/**
* @internal cpssDxChStcEnableSet function
* @endinternal
*
* @brief   Global Enable/Disable Sampling To CPU (STC).
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon.
*
* @param[in] devNum                   - the device number
* @param[in] stcType                  - Sampling To CPU (STC) type: ingress or egress
* @param[in] enable                   - GT_TRUE =  Sampling To CPU (STC)
*                                      GT_FALSE = disable Sampling To CPU (STC)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number or STC type.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChStcEnableSet
(
    IN  GT_U8                       devNum,
    IN  CPSS_DXCH_STC_TYPE_ENT      stcType,
    IN  GT_BOOL                     enable
);

/**
* @internal cpssDxChStcEnableGet function
* @endinternal
*
* @brief   Get the global status of Sampling To CPU (STC) (Enabled/Disabled).
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon.
*
* @param[in] devNum                   - the device number
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
GT_STATUS cpssDxChStcEnableGet
(
    IN  GT_U8                       devNum,
    IN  CPSS_DXCH_STC_TYPE_ENT      stcType,
    OUT GT_BOOL                     *enablePtr
);

/**
* @internal cpssDxChStcPortLimitSet function
* @endinternal
*
* @brief   Set Sampling to CPU (STC) limit per port.
*         The limit to be loaded into the Count Down Counter.
*         This counter is decremented for each packet received on this port and is
*         subject to sampling according to the setting of STC Count mode.
*         When this counter is decremented from 1 to 0, the packet causing this
*         decrement is sampled to the CPU.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon.
*
* @param[in] devNum                   - the device number
* @param[in] port                     -  number.
* @param[in] stcType                  - Sampling To CPU (STC) type: ingress or egress
* @param[in] limit                    - Count Down Limit (APPLICABLE RANGES: 0..0x3FFFFFFF)
*                                      when limit value is 1 - every packet is sampled to CPU
*                                      when limit value is 0 - there is no sampling
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number,port number or STC type.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - limit is out of range
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChStcPortLimitSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        port,
    IN  CPSS_DXCH_STC_TYPE_ENT      stcType,
    IN  GT_U32                      limit
);

/**
* @internal cpssDxChStcPortLimitGet function
* @endinternal
*
* @brief   Get Sampling to CPU (STC) limit per port.
*         The limit to be loaded into the Count Down Counter.
*         This counter is decremented for each packet received on this port and is
*         subject to sampling according to the setting of STC Count mode.
*         When this counter is decremented from 1 to 0, the packet causing this
*         decrement is sampled to the CPU.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon.
*
* @param[in] devNum                   - the device number
* @param[in] port                     -  number
* @param[in] stcType                  - Sampling To CPU (STC) type: ingress or egress
*
* @param[out] limitPtr                 - pointer to Count Down Limit
*                                      when limit value is 1 - every packet is sampled to CPU
*                                      when limit value is 0 - there is no sampling
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number,port number or STC type.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChStcPortLimitGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        port,
    IN  CPSS_DXCH_STC_TYPE_ENT      stcType,
    OUT GT_U32                      *limitPtr
);

/**
* @internal cpssDxChStcPortReadyForNewLimitGet function
* @endinternal
*
* @brief   Check whether Sampling to CPU is ready to get new STC Limit Value per
*         port.
*         The function cpssDxChStcPortLimitSet sets new limit value.
*         But only after device finishes handling of new limit value the
*         cpssDxChStcPortLimitSet may be called once more.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon.
*
* @param[in] devNum                   - the device number
* @param[in] port                     -  number
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
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - This request is not supportted.
*/
GT_STATUS cpssDxChStcPortReadyForNewLimitGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        port,
    IN  CPSS_DXCH_STC_TYPE_ENT      stcType,
    OUT GT_BOOL                     *isReadyPtr
);

/**
* @internal cpssDxChStcPortCountdownCntrGet function
* @endinternal
*
* @brief   Get STC Countdown Counter per port.
*         This counter is decremented for each packet received on this port and is
*         subject to sampling according to the setting of STC Count mode.
*         When this counter is decremented from 1 to 0, the packet causing this
*         decrement is sampled to the CPU.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon.
*
* @param[in] devNum                   - the device number
* @param[in] port                     -  number
* @param[in] stcType                  - Sampling To CPU (STC) type: ingress or egress
*
* @param[out] cntrPtr                  - (pointer to) STC Count down counter. This is the number of
*                                      packets left to send/receive in order that a packet will be
*                                      sampled to CPU and a new value will be loaded.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number,port number or STC type.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - This request is not supportted.
*/
GT_STATUS cpssDxChStcPortCountdownCntrGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        port,
    IN  CPSS_DXCH_STC_TYPE_ENT      stcType,
    OUT GT_U32                      *cntrPtr
);

/**
* @internal cpssDxChStcPortSampledPacketsCntrSet function
* @endinternal
*
* @brief   Set the number of packets Sampled to CPU per port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon.
*
* @param[in] devNum                   - the device number
* @param[in] port                     -  number
* @param[in] stcType                  - Sampling To CPU (STC) type: ingress or egress
* @param[in] cntr                     - STC Sampled to CPU packet's counter (APPLICABLE RANGES: 0..0xFFFF).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number,port number or STC type.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_OUT_OF_RANGE          - on wrong cntr
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - This request is not supportted.
*/
GT_STATUS cpssDxChStcPortSampledPacketsCntrSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        port,
    IN  CPSS_DXCH_STC_TYPE_ENT      stcType,
    IN  GT_U32                      cntr
);

/**
* @internal cpssDxChStcPortSampledPacketsCntrGet function
* @endinternal
*
* @brief   Get the number of packets Sampled to CPU per port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon.
*
* @param[in] devNum                   - the device number
* @param[in] port                     -  number
* @param[in] stcType                  - Sampling To CPU (STC) type: ingress or egress
*
* @param[out] cntrPtr                  - (pointer to) STC Sampled to CPU packet's counter.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number,port number or STC type.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - This request is not supportted.
*/
GT_STATUS cpssDxChStcPortSampledPacketsCntrGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        port,
    IN  CPSS_DXCH_STC_TYPE_ENT      stcType,
    OUT GT_U32                      *cntrPtr
);

/**
* @internal cpssDxChStcEgressAnalyzerIndexSet function
* @endinternal
*
* @brief   Set the egress STC analyzer index.
*         feature behavior:
*         If a packet is marked for both egress STC and egress mirroring , the
*         packet is only replicated once, depending on a configurable priority
*         between the mechanisms. The configurable <STC analyzer index> determines
*         the priority of STC compared to mirroring to the analyzer.
*         Note:
*         1. that STC does not actually use this index to determine the target
*         of the mirrored packet.
*         2. the indexes used by egress mirroring engines are in range 0..6.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - global enable/disable analyzer index for egress STC.
*                                      - GT_TRUE - enable analyzer index.
*                                      - GT_FALSE - no analyzer index.
* @param[in] index                    - Analyzer index. (APPLICABLE RANGES: 0..14)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_HW_ERROR              - on HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note function uses same parameters as mirroring functions like
*       cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexSet(devNum,enable,index).
*
*/
GT_STATUS cpssDxChStcEgressAnalyzerIndexSet
(
    IN GT_U8     devNum,
    IN GT_BOOL   enable,
    IN GT_U32    index
);

/**
* @internal cpssDxChStcEgressAnalyzerIndexGet function
* @endinternal
*
* @brief   Get the egress STC analyzer index.
*         feature behavior:
*         If a packet is marked for both egress STC and egress mirroring , the
*         packet is only replicated once, depending on a configurable priority
*         between the mechanisms. The configurable <STC analyzer index> determines
*         the priority of STC compared to mirroring to the analyzer.
*         Note:
*         1. that STC does not actually use this index to determine the target
*         of the mirrored packet.
*         2. the indexes used by egress mirroring engines are in range 0..6.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Falcon.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - (pointer to) global enable/disable analyzer index for egress STC.
*                                      - GT_TRUE - enable analyzer index.
*                                      - GT_FALSE - no analyzer index.
* @param[out] indexPtr                 - (pointer to) Analyzer destination interface index.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note function uses same parameters as mirroring functions like
*       cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexGet(devNum,enablePtr,indexPtr).
*
*/
GT_STATUS cpssDxChStcEgressAnalyzerIndexGet
(
    IN GT_U8     devNum,
    OUT GT_BOOL   *enablePtr,
    OUT GT_U32    *indexPtr
);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChStch */



