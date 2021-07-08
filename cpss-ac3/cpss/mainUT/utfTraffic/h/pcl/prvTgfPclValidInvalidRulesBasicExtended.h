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
* @file prvTgfPclValidInvalidRulesBasicExtended.h
*
* @brief Ingress PCL key STD L2
*
* @version   1.1
********************************************************************************
*/
#ifndef __prvTgfPclValidInvalidRulesBasicExtendedh
#define __prvTgfPclValidInvalidRulesBasicExtendedh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfPclValidInvalidRulesBasicExtendedConfigurationSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfPclValidInvalidRulesBasicExtendedConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfPclValidInvalidRulesBasicExtendedPclConfigurationSet function
* @endinternal
*
* @brief   Set test PCL configuration
*/
GT_VOID prvTgfPclValidInvalidRulesBasicExtendedPclConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfPclValidInvalidRulesBasicExtendedTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfPclValidInvalidRulesBasicExtendedTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfPclValidInvalidRulesBasicExtendedConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclValidInvalidRulesBasicExtendedConfigurationRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfPclValidInvalidRulesBasicExtendedh */

