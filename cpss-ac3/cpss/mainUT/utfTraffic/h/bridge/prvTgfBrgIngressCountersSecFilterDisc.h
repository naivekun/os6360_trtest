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
* @file prvTgfBrgIngressCountersSecFilterDisc.h
*
* @brief The test checks that packets are received in
* counter set 1, field <SecFilterDisc> in all modes.
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfBrgIngressCountersSecFilterDisch
#define __prvTgfBrgIngressCountersSecFilterDisch

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfBrgIngressCountersSecFilterDiscConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfBrgIngressCountersSecFilterDiscConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgIngressCountersSecFilterDiscTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBrgIngressCountersSecFilterDiscTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgIngressCountersSecFilterDiscConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgIngressCountersSecFilterDiscConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfBrgIngressCountersSecFilterDisch */


