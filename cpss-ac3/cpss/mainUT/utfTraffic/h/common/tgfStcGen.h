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
* @file tgfStcGen.h
*
* @brief Generic APIs STC (Sampling to CPU).
*
* @version   2
********************************************************************************
*/
#ifndef __tgfStcGenh
#define __tgfStcGenh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @enum PRV_TGF_STC_COUNT_MODE_ENT
 *
 * @brief Enum for Sampling To CPU (STC) Count mode.
*/
typedef enum{

    /** @brief All packets received without any
     *  MAClevel error are subject to
     *  Sampling To CPU (STC).
     */
    PRV_TGF_STC_COUNT_ALL_PACKETS_E,

    /** @brief Only non-dropped packets
     *  are subject to Sampling To CPU (STC)
     */
    PRV_TGF_STC_COUNT_NON_DROPPED_PACKETS_E

} PRV_TGF_STC_COUNT_MODE_ENT;


/**
* @enum PRV_TGF_STC_COUNT_RELOAD_MODE_ENT
 *
 * @brief Enum for Sampling To CPU (STC) count reload mode.
*/
typedef enum{

    /** @brief The port ingress/egress STC
     *  mechanism reloads the sampling frequency value that was last
     *  configured by the CPU.
     */
    PRV_TGF_STC_COUNT_RELOAD_CONTINUOUS_E,

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
    PRV_TGF_STC_COUNT_RELOAD_TRIGGERED_E

} PRV_TGF_STC_COUNT_RELOAD_MODE_ENT;


/**
* @enum PRV_TGF_STC_TYPE_ENT
 *
 * @brief Enum for Sampling To CPU (STC) type.
*/
typedef enum{

    /** Ingress Sampling To CPU (STC) */
    PRV_TGF_STC_INGRESS_E,

    /** Egress Sampling To CPU (STC) */
    PRV_TGF_STC_EGRESS_E

} PRV_TGF_STC_TYPE_ENT;


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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __tgfStcGenh */


