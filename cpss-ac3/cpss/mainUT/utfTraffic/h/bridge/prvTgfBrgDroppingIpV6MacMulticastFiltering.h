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
* @file prvTgfBrgDroppingIpV6MacMulticastFiltering.h
*
* @brief Verify that packets with IP MC Address
* range are: 33-33-xx-xx-xx are dropped.
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfBrgDroppingIpV6MacMulticastFilteringh
#define __prvTgfBrgDroppingIpV6MacMulticastFilteringh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfBrgDroppingIpV6MacMulticastFilteringConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfBrgDroppingIpV6MacMulticastFilteringConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgDroppingIpV6MacMulticastFilteringTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBrgDroppingIpV6MacMulticastFilteringTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgDroppingIpV6MacMulticastFilteringConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgDroppingIpV6MacMulticastFilteringConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfBrgDroppingIpV6MacMulticastFilteringh */


