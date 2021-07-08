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
* @file prvTgfTablesFillingLtt.h
*
* @brief Low Level IP table filling test
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfTablesFillingLtth
#define __prvTgfTablesFillingLtth

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfTablesFillingLttConfigurationSet function
* @endinternal
*
* @brief   Set common configuration
*/
GT_VOID prvTgfTablesFillingLttConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfTablesFillingLttTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] protocol                 - type of IP stack used
* @param[in] numLoops                 - number of loops of the test to discover
*                                      a memory leak and so on
*                                       None
*/
GT_VOID prvTgfTablesFillingLttTrafficGenerate
(
    IN CPSS_IP_PROTOCOL_STACK_ENT protocol,
    IN GT_U32                     numLoops
);

/**
* @internal prvTgfTablesFillingLttConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfTablesFillingLttConfigurationRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfTablesFillingLtth */


