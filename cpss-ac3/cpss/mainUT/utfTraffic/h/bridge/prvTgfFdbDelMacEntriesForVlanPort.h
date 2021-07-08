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
* @file prvTgfFdbDelMacEntriesForVlanPort.h
*
* @brief Verify that when restrict deleting to a specific port + vlan,
* only entries associated with this port + vlan are deleted.
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfFdbDelMacEntriesForVlanPorth
#define __prvTgfFdbDelMacEntriesForVlanPorth

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfFdbDelMacEntriesForVlanPortConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfFdbDelMacEntriesForVlanPortConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfFdbDelMacEntriesForVlanPortTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfFdbDelMacEntriesForVlanPortTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfFdbDelMacEntriesForVlanPortConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbDelMacEntriesForVlanPortConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfFdbDelMacEntriesForVlanPorth */


