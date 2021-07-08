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
* @file prvTgfFdbSourceIdModeDefault.h
*
* @brief Verify that when source ID assignment mode is default source ID,
* packets should filtered only if egress port is deleted from the
* default source ID that configured on ingress port.
* When egress port is deleted from the source ID group of the FDB entry,
* packets should not be filtered.
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfFdbSourceIdModeDefaulth
#define __prvTgfFdbSourceIdModeDefaulth

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfFdbSourceIdModeDefaultConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfFdbSourceIdModeDefaultConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfFdbSourceIdModeDefaultTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfFdbSourceIdModeDefaultTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfFdbSourceIdModeDefaultConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbSourceIdModeDefaultConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfFdbSourceIdModeDefaulth */


