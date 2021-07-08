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
* @file prvTgfPolicerEgressDscpModify.h
*
* @brief Test for DSCP modification modes due to Egress policer.
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfPolicerEgressDscpModify
#define __prvTgfPolicerEgressDscpModify

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfPolicerGen.h>

/**
* @internal prvTgfPolicerEgressDscpModifyTest function
* @endinternal
*
* @brief   Modify DSCP in EGRESS policer test
*
* @param[in] prvUtfVrfId              - virtual router index
*                                       None
*/
GT_VOID prvTgfPolicerEgressDscpModifyTest
(
    GT_U32      prvUtfVrfId
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfPolicerEgressDscpModify */



