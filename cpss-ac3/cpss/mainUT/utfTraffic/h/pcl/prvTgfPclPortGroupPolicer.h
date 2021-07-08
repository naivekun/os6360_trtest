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
* @file prvTgfPclPortGroupPolicer.h
*
* @brief Second Lookup match
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfPclPortGroupPolicer
#define __prvTgfPclPortGroupPolicer

#include <trafficEngine/tgfTrafficGenerator.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfPclPortGroupPolicerCounting function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfPclPortGroupPolicerCounting
(
    GT_VOID
);

/**
* @internal prvTgfPclPortGroupPolicerMetering function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfPclPortGroupPolicerMetering
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfPclPortGroupPolicer */
