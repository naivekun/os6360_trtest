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
* @file prvTgfCos.h
*
* @brief Class of services (Cos)
*
* @version   3
********************************************************************************
*/
#ifndef __prvTgfCos
#define __prvTgfCos

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfCosTestCommonConfigure function
* @endinternal
*
* @brief   This function configures Cos all tests
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       - trunk library was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - some values are out of range
* @retval GT_BAD_PARAM             - on illegal parameters
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_ALREADY_EXIST         - one of the members already exists in another trunk
*/
GT_STATUS prvTgfCosTestCommonConfigure
(
    GT_VOID
);

/**
* @internal prvTgfCosTestCommonReset function
* @endinternal
*
* @brief   This function resets configuration of Cos
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       - trunk library was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - some values are out of range
* @retval GT_BAD_PARAM             - on illegal parameters
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_ALREADY_EXIST         - one of the members already exists in another trunk
*/
GT_STATUS prvTgfCosTestCommonReset
(
    GT_VOID
);

/**
* @internal prvTgfCosExpTestTrafficGenerateAndCheck function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet:
*         Check amount of egressed packets with both TCs
*/
GT_VOID prvTgfCosExpTestTrafficGenerateAndCheck
(
    GT_VOID
);

/**
* @internal prvTgfCosVlanTagSelectTestTrafficGenerateAndCheck function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet:
*         Check amount of egressed packets with both TCs
*/
GT_VOID prvTgfCosVlanTagSelectTestTrafficGenerateAndCheck
(
    GT_VOID
);


/**
* @internal tgfCosMappingTableIndexTest function
* @endinternal
*
* @brief   test 3 of the 12 mapping tables that the device hold.
*/
GT_VOID tgfCosMappingTableIndexTest
(
    GT_VOID
);

/**
* @internal tgfCosPortProtocolTest function
* @endinternal
*
* @brief   test port protocol qos assignment
*/
GT_VOID tgfCosPortProtocolTest
(
    GT_VOID
);




#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfCos */


