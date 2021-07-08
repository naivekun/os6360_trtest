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
* @file prvTgfIngressPclKeyStdL2.h
*
* @brief Ingress PCL key STD L2
*
* @version   1.2
********************************************************************************
*/
#ifndef __prvTgfIngressPclKeyStdL2h
#define __prvTgfIngressPclKeyStdL2h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfConfigurationSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfPclConfigurationSet function
* @endinternal
*
* @brief   Set test PCL configuration
*/
GT_VOID prvTgfPclConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfConfigurationRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfIngressPclKeyStdL2h */

