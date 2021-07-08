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
* @file prvTgfExactMatchDefaultActionUdbPcl.h
*
* @brief Test Exact Match Default Action functionality with PCL Action Type and
*        UDB key
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfExactMatchDefaultActionUdbPclh
#define __prvTgfExactMatchDefaultActionUdbPclh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfExactMatchGen.h>

/**
* @internal prvTgfExactMatchDefaultActionUdbPclInvalidateRule function
* @endinternal
*
* @brief   Invalidate PCL Rule
*/
GT_VOID prvTgfExactMatchDefaultActionUdbPclInvalidateRule
(
    GT_VOID
);
/**
* @internal GT_VOID prvTgfExactMatchDefaultActionUdbPclConfigSet function
* @endinternal
*
* @brief   Set PCL test configuration related to Exact Match Default Action
*/
GT_VOID prvTgfExactMatchDefaultActionUdbPclConfigSet
(
    GT_VOID
);
/**
* @internal GT_VOID prvTgfExactMatchDefaultActionUdbPclConfigRestore function
* @endinternal
*
* @brief   Restore PCL test configuration related to Exact Match Default Action
*/
GT_VOID prvTgfExactMatchDefaultActionUdbPclConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
