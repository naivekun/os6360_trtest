/*******************************************************************************
*              (c), Copyright 2016, Marvell International Ltd.                 *
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
* @file prvTgfDiagMppmEccProtection.h
*
* @brief Diag Descriptor
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfDiagMppmEccProtectionh
#define __prvTgfDiagMppmEccProtectionh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfDiagMppmEccProtectionConfigurationSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfDiagMppmEccProtectionConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfDiagMppmEccProtectionTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfDiagMppmEccProtectionTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfDiagMppmEccProtectionRestore function
* @endinternal
*
* @brief   Restore saved configuration
*/
GT_VOID prvTgfDiagMppmEccProtectionRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfDiagMppmEccProtectionh */


