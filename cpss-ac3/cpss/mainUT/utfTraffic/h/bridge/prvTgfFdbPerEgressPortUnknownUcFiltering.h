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
* @file prvTgfFdbPerEgressPortUnknownUcFiltering.h
*
* @brief Verify per-Egress port filtering for unknown UC packets.
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfFdbPerEgressPortUnknownUcFilteringh
#define __prvTgfFdbPerEgressPortUnknownUcFilteringh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfFdbPerEgressPortUnknownUcFilteringConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfFdbPerEgressPortUnknownUcFilteringConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfFdbPerEgressPortUnknownUcFilteringTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfFdbPerEgressPortUnknownUcFilteringTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfFdbPerEgressPortUnknownUcFilteringConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbPerEgressPortUnknownUcFilteringConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfFdbPerEgressPortUnknownUcFilteringh */


