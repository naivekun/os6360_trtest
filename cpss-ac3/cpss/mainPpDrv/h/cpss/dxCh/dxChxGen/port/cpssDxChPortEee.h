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
* @file cpssDxChPortEee.h
*
* @brief CPSS DxCh Port's IEEE 802.3az Energy Efficient Ethernet (EEE) Support.
*
* @note 1. The device support relevant when the PHY works in EEE slave mode.
* When the PHY works in EEE master mode, the device is not involved in
* EEE processing.
* 2. The feature not relevant when port is not 'tri-speed'
* 3. LPI is short for 'Low Power Idle'
*
*
*
* @version   1
********************************************************************************
*/
#ifndef __cpssDxChPortEee
#define __cpssDxChPortEee

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>

/**
* @enum CPSS_DXCH_PORT_EEE_LPI_MANUAL_MODE_ENT
 *
 * @brief Enumeration of EEE manual modes.
*/
typedef enum{

    /** manual mode is disabled */
    CPSS_DXCH_PORT_EEE_LPI_MANUAL_MODE_DISABLED_E   ,

    /** manual mode is enabled (but not 'forced') */
    CPSS_DXCH_PORT_EEE_LPI_MANUAL_MODE_ENABLED_E    ,

    /** @brief manual mode is enabled and
     *  force transmitting of LPI signalling while working in manual mode
     */
    CPSS_DXCH_PORT_EEE_LPI_MANUAL_MODE_FORCED_E

} CPSS_DXCH_PORT_EEE_LPI_MANUAL_MODE_ENT;


/**
* @struct CPSS_DXCH_PORT_EEE_LPI_STATUS_STC
 *
 * @brief structure for EEE LPI status info
*/
typedef struct{

    /** @brief Indicates that the MAC is sending LPI to the Tx PHY.
     *  GT_FALSE - not sending.
     *  GT_TRUE - sending.
     */
    GT_BOOL macTxPathLpi;

    /** @brief Indicates that the MAC is sending IDLE to the Tx PHY and waiting for Tx timer to end.
     *  GT_FALSE - not sending.
     *  GT_TRUE - sending and wait.
     */
    GT_BOOL macTxPathLpWait;

    /** @brief Indicates that the PHY has detected the assertion / de
     *  GT_FALSE - not detected.
     *  GT_TRUE - detected.
     */
    GT_BOOL macRxPathLpi;

    /** @brief PCS Tx path LPI status .
     *  GT_FALSE - Does not receive: Tx PCS does not receive LP_IDLE.
     *  GT_TRUE - Receives: Tx PCS receives LP_IDLE.
     */
    GT_BOOL pcsTxPathLpi;

    /** @brief PCS Rx path LPI status.
     *  GT_FALSE - Does not receive: Rx PCS does not receive LP_IDLE.
     *  GT_TRUE - Receives: Rx PCS receives LP_IDLE.
     */
    GT_BOOL pcsRxPathLpi;

} CPSS_DXCH_PORT_EEE_LPI_STATUS_STC;

/**
* @internal cpssDxChPortEeeLpiRequestEnableSet function
* @endinternal
*
* @brief   Set port enable/disable transmitting of LPI signaling to the PHY and
*         identification of LPI on reception.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   - GT_TRUE  - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortEeeLpiRequestEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
);

/**
* @internal cpssDxChPortEeeLpiRequestEnableGet function
* @endinternal
*
* @brief   Get port's enable/disable transmitting of LPI signaling to the PHY and
*         identification of LPI on reception.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] enablePtr                -  (pointer to)
*                                      GT_TRUE  - enabled
*                                      GT_FALSE - disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortEeeLpiRequestEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr
);

/**
* @internal cpssDxChPortEeeLpiManualModeSet function
* @endinternal
*
* @brief   Set port manual mode (enabled/disabled/forced) for LPI signaling.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] mode                     - the manual mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum or mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortEeeLpiManualModeSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_DXCH_PORT_EEE_LPI_MANUAL_MODE_ENT  mode
);

/**
* @internal cpssDxChPortEeeLpiManualModeGet function
* @endinternal
*
* @brief   Get port manual mode (enabled/disabled/forced) for LPI signaling.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] modePtr                  - (pointer to) the manual mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortEeeLpiManualModeGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT CPSS_DXCH_PORT_EEE_LPI_MANUAL_MODE_ENT  *modePtr
);

/**
* @internal cpssDxChPortEeeLpiTimeLimitsSet function
* @endinternal
*
* @brief   Set port LPI timers.
*         NOTE: The 'time' granularity is 1 uSec for 1000 Mbps and 10 uSec for 100 Mbps.
*         so the function set value 'as is'.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] twLimit                  - The minimum time from LPI de-assertion until valid data can be sent .
*                                      (APPLICABLE RANGES: 0..4095)
* @param[in] liLimit                  - The minimum time since going into LPI and allowing LPI de-assert.
*                                      (APPLICABLE RANGES: 0..254)
* @param[in] tsLimit                  - The minimum time from emptying of Tx FIFO and LPI assert.
*                                      (APPLICABLE RANGES: 0..255)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_OUT_OF_RANGE          - on out of range : twLimit , liLimit , tsLimit
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortEeeLpiTimeLimitsSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  twLimit,
    IN  GT_U32                  liLimit,
    IN  GT_U32                  tsLimit
);

/**
* @internal cpssDxChPortEeeLpiTimeLimitsGet function
* @endinternal
*
* @brief   Get port LPI timers.
*         NOTE: The 'time' granularity is 1 uSec for 1000 Mbps and 10 uSec for 100 Mbps.
*         so the function get value 'as is'.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] twLimitPtr               - (pointer to) The minimum time from LPI de-assertion until valid data can be sent .
* @param[out] liLimitPtr               - (pointer to) The minimum time since going into LPI and allowing LPI de-assert.
* @param[out] tsLimitPtr               - (pointer to) The minimum time from emptying of Tx FIFO and LPI assert.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortEeeLpiTimeLimitsGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *twLimitPtr,
    OUT GT_U32                  *liLimitPtr,
    OUT GT_U32                  *tsLimitPtr
);

/**
* @internal cpssDxChPortEeeLpiStatusGet function
* @endinternal
*
* @brief   Get port LPI status.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] statusPtr                - (pointer to) the status info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortEeeLpiStatusGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_DXCH_PORT_EEE_LPI_STATUS_STC *statusPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChPortEee */


