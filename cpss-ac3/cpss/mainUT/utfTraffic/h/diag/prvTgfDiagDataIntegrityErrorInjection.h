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
* @file prvTgfDiagDataIntegrityErrorInjection.h
*
* @brief Tests of error injection in various memories
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfDiagDataIntegrityErrorInjection
#define __prvTgfDiagDataIntegrityErrorInjection

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfDiagDataIntegrityErrorInjectionConfigSet function
* @endinternal
*
* @brief   Sets configuration
*/
GT_VOID prvTgfDiagDataIntegrityErrorInjectionConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfDiagDataIntegrityErrorInjectionTrafficGenerate function
* @endinternal
*
* @brief   Restores configuration
*/
GT_VOID prvTgfDiagDataIntegrityErrorInjectionTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfDiagDataIntegrityErrorInjectionRestore function
* @endinternal
*
* @brief   Restores configuration
*/
GT_VOID prvTgfDiagDataIntegrityErrorInjectionRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfDiagDataIntegrityErrorInjection */


