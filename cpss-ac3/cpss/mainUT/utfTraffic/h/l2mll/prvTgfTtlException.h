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
* @file prvTgfTtlException.h
*
* @brief L2 MLL TTL exception test definitions
*
* @version   2
********************************************************************************
*/
#ifndef __prvTgfTtlExceptionh
#define __prvTgfTtlExceptionh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfTtlExceptionConfigurationSet function
* @endinternal
*
* @brief   Set configuration.
*/
GT_VOID prvTgfTtlExceptionConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfTtlExceptionTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfTtlExceptionTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfTtlExceptionConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration.
*/
GT_VOID prvTgfTtlExceptionConfigurationRestore
(
    GT_VOID
);


/**
* @internal prvTgfTtlExceptionVlanPortTranslationConfigurationSet function
* @endinternal
*
* @brief   Set configuration.
*/
GT_STATUS prvTgfTtlExceptionVlanPortTranslationConfigurationSet
(
    GT_U32 ttlThreshold
);

/**
* @internal prvTgfTtlExceptionVlanPortTranslationTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfTtlExceptionVlanPortTranslationTrafficGenerate
(
    GT_VOID
);


/**
* @internal prvTgfTtlExceptionVlanPortTranslationConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration.
*/
GT_VOID prvTgfTtlExceptionVlanPortTranslationConfigurationRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfTtlExceptionh */


