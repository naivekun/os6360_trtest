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
* @file cpssDxChPortCombo.h
*
* @brief CPSS implementation for Combo Port configuration and control facility.
*
* @version   1
********************************************************************************
*/

#ifndef __cpssDxChPortComboh
#define __cpssDxChPortComboh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/port/cpssPortCtrl.h>

/**
* @enum CPSS_DXCH_PORT_SERDES_MODE_ENT
 *
 * @brief Enumeration of Serdes modes
*/
typedef enum{

    /** Regular Serdes are active */
    CPSS_DXCH_PORT_SERDES_MODE_REGULAR_E,

    /** Combo Serdes are active */
    CPSS_DXCH_PORT_SERDES_MODE_COMBO_E

} CPSS_DXCH_PORT_SERDES_MODE_ENT;

/**
* @struct CPSS_DXCH_PORT_MAC_PARAMS_STC
 *
 * @brief Structure describes specific physical MAC
 * APPLICABLE DEVICES: Lion2
*/
typedef struct{

    /** physical MAC number */
    GT_U32 macNum;

    /** @brief number of core where MAC resides, for multi
     *  for single core leave it 0.
     *  connection - connection type (fiber/copper) of this MAC (dependent on
     *  specific board architecture)
     */
    GT_U32 macPortGroupNum;

} CPSS_DXCH_PORT_MAC_PARAMS_STC;

/* max number of MAC's partisipating in combo port */
#define CPSS_DXCH_PORT_MAX_MACS_IN_COMBO_CNS   2

/* 1) don't use preferred MAC while configure combo port;
   2) place in MAC's array free */
#define CPSS_DXCH_PORT_COMBO_NA_MAC_CNS   0x7FFFFFFF

/**
* @struct CPSS_DXCH_PORT_COMBO_PARAMS_STC
 *
 * @brief Parameters describing combo port
 * APPLICABLE DEVICES: Lion2
*/
typedef struct{

    CPSS_DXCH_PORT_MAC_PARAMS_STC macArray[CPSS_DXCH_PORT_MAX_MACS_IN_COMBO_CNS];

    /** @brief (optional) index of MAC in macArray, which should be
     *  activated by default, during port interface
     *  configuration and which should be chosen if port
     *  connected by both exits.
     *  If application not interested to use this option
     *  must set to CPSS_DXCH_PORT_COMBO_NA_MAC_CNS
     */
    GT_U32 preferredMacIdx;

} CPSS_DXCH_PORT_COMBO_PARAMS_STC;

/**
* @internal cpssDxChPortSerdesComboModeSet function
* @endinternal
*
* @brief   Set Serdes mode. If Serdes mode is CPSS_DXCH_PORT_SERDES_MODE_COMBO_E,
*         Combo Serdes are active for the port, else regular Serdes are active for
*         the port.
*
* @note   APPLICABLE DEVICES:      DxCh3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; Lion; xCat2; Lion2; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number, values 20..23.
* @param[in] mode                     - serdes mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum, mode
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssDxChPortSerdesComboModeSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_DXCH_PORT_SERDES_MODE_ENT  mode
);

/**
* @internal cpssDxChPortSerdesComboModeGet function
* @endinternal
*
* @brief   Get Serdes mode. If Serdes mode is CPSS_DXCH_PORT_SERDES_MODE_COMBO_E,
*         Combo Serdes are active for the port, else regular Serdes are active for
*         the port.
*
* @note   APPLICABLE DEVICES:      DxCh3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; Lion; xCat2; Lion2; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number, values 20..23.
*
* @param[out] modePtr                  -  pointer to Serdes mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssDxChPortSerdesComboModeGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_DXCH_PORT_SERDES_MODE_ENT  *modePtr
);

/**
* @internal cpssDxChPortComboModeEnableSet function
* @endinternal
*
* @brief   Register/unregister port as combo in CPSS SW DB and set some specific
*         parameters. This configuration will be used by cpssDxChPortModeSpeedSet,
*         when port's physical interface will be actually configured and for
*         other system needs.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
* @param[in] enable                   -   GT_TRUE - port is combo;
*                                      GT_FALSE - port is regular.
* @param[in] paramsPtr                - (ptr to) parameters describing specific combo port options,
*                                      doesn't matter if enable == GT_FALSE
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - paramsPtr == NULL
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssDxChPortComboModeEnableSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_BOOL                                 enable,
    IN  const   CPSS_DXCH_PORT_COMBO_PARAMS_STC *paramsPtr
);

/**
* @internal cpssDxChPortComboModeEnableGet function
* @endinternal
*
* @brief   Get if port registered as combo and its parameters.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
*
* @param[out] enablePtr                -   GT_TRUE - port is combo;
*                                      GT_FALSE - port is regular.
* @param[out] paramsPtr                - parameters describing specific combo port options
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - enablePtr or paramsPtr == NULL
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssDxChPortComboModeEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr,
    OUT CPSS_DXCH_PORT_COMBO_PARAMS_STC *paramsPtr
);

/**
* @internal cpssDxChPortComboPortActiveMacSet function
* @endinternal
*
* @brief   Activate one of MAC's serving combo port.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
* @param[in] macPtr                   - (ptr to) parameters describing one of MAC of given combo port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - macPtr == NULL
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssDxChPortComboPortActiveMacSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  const   CPSS_DXCH_PORT_MAC_PARAMS_STC  *macPtr
);

/**
* @internal cpssDxChPortComboPortActiveMacGet function
* @endinternal
*
* @brief   Get current activate MAC of combo port.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
*
* @param[out] macPtr                   - (ptr to) parameters describing active MAC
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - macPtr == NULL
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssDxChPortComboPortActiveMacGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_DXCH_PORT_MAC_PARAMS_STC   *macPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChPortComboh */


