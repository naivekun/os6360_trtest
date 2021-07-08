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
* @file prvTgfTailDropTxqUtil.h
*
* @brief "TxQ utilization statistics" enhanced UT
*
* @version   1
********************************************************************************
*/

#ifndef __prvTgfTailDropTxqUtilh
#define __prvTgfTailDropTxqUtilh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>

/**
* @internal prvTgfTailDropTxqUtilConfigure function
* @endinternal
*
* @brief Prepare or restore prvTgfTailDropTxqUtil test configuration
*
* @param[in] configSet    - store/restore configuration
*                           GT_TRUE  -- configure
*                           GT_FALSE -- restore
*/
GT_VOID prvTgfTailDropTxqUtilConfigure
(
    GT_BOOL configSet
);

/**
* @internal prvTgfTailDropTxqUtilTrafficGenerate function
* @endinternal
*
* @brief Generate traffic fot prvTgfTailDropTxqUtil test.
*
*/
GT_VOID prvTgfTailDropTxqUtilTrafficGenerate
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfTailDropTxqUtilh */