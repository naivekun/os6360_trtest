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
* @file prvTgfBrgVlanIndexing.h
*
* @brief VLAN Indexing
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfBrgVlanIndexing
#define __prvTgfBrgVlanIndexing

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfBrgBrgVlanMembersIndexingConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*/
GT_VOID prvTgfBrgBrgVlanMembersIndexingConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgBrgVlanMembersIndexingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfBrgBrgVlanMembersIndexingTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgBrgVlanMembersIndexingConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgBrgVlanMembersIndexingConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfBrgBrgVlanStgIndexingConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*/
GT_VOID prvTgfBrgBrgVlanStgIndexingConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgBrgVlanStgIndexingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfBrgBrgVlanStgIndexingTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgBrgVlanStgIndexingConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgBrgVlanStgIndexingConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfBrgBrgVlanTagStateIndexingConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*/
GT_VOID prvTgfBrgBrgVlanTagStateIndexingConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgBrgVlanTagStateIndexingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfBrgBrgVlanTagStateIndexingTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgBrgVlanTagStateIndexingConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgBrgVlanTagStateIndexingConfigurationRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfBrgVlanIndexing */


