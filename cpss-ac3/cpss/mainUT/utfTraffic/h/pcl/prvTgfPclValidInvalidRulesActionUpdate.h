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
* @file prvTgfPclValidInvalidRulesActionUpdate.h
*
* @brief Ingress PCL key STD L2
*
* @version   1.1
********************************************************************************
*/
#ifndef __prvTgfPclValidInvalidRulesActionUpdateh
#define __prvTgfPclValidInvalidRulesActionUpdateh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfPclValidInvalidRulesActionUpdateConfigurationSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfPclValidInvalidRulesActionUpdateConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfPclValidInvalidRulesActionUpdatePclConfigurationSet function
* @endinternal
*
* @brief   Set test PCL configuration
*/
GT_VOID prvTgfPclValidInvalidRulesActionUpdatePclConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfPclValidInvalidRulesActionUpdateTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfPclValidInvalidRulesActionUpdateTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfPclValidInvalidRulesActionUpdateConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclValidInvalidRulesActionUpdateConfigurationRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfPclValidInvalidRulesActionUpdateh */

