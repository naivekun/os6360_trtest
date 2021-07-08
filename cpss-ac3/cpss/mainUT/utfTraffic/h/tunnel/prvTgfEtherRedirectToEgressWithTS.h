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
* @file prvTgfEtherRedirectToEgressWithTS.h
*
* @brief Ethernet lookup and redirect to egress with Redirect To TS
*
* @version   2
********************************************************************************
*/


/**
* @internal prvTgfEtherRedirectToEgressWithTsBridgeConfigSet function
* @endinternal
*
* @brief   Set Bridge Configuration
*/
GT_VOID prvTgfEtherRedirectToEgressWithTsBridgeConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfEtherRedirectToEgressWithTsTtiConfigSet function
* @endinternal
*
* @brief   Set TTI test settings:
*         - Enable port 0 for Eth lookup
*         - Set Eth key lookup MAC mode to Mac DA
*         - Set TTI rule action
*         - Set TTI rule
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_VOID prvTgfEtherRedirectToEgressWithTsTtiConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfEtherRedirectToEgressWithTsInilifConfigSet function
* @endinternal
*
* @brief   Set inlif configuration
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_VOID prvTgfEtherRedirectToEgressWithTsInilifConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfEtherRedirectToEgressWithTsTunnelConfigurationSet function
* @endinternal
*
* @brief   Set Tunnel Configuration
*/
GT_VOID prvTgfEtherRedirectToEgressWithTsTunnelConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfEtherRedirectToEgressWithTsTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:11,
*/
GT_VOID prvTgfEtherRedirectToEgressWithTsTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfEtherRedirectToEgressWithTsConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfEtherRedirectToEgressWithTsConfigurationRestore
(
    GT_VOID
);


