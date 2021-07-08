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
* @file prvTgfPolicerECNMarkingCheck.h
*
* @brief Header file for test of UP modification modes due to Egress policer.
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfPolicerECNMarkingCheck
#define __prvTgfPolicerECNMarkingCheck

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfPolicerGen.h>


/**
* @internal prvTgfPolicerECNMarkingConfigurationSet function
* @endinternal
*
* @brief   Test configurations
*/
GT_VOID prvTgfPolicerECNMarkingConfigurationSet();

/**
* @internal prvTgfPolicerECNMarkingRestore function
* @endinternal
*
* @brief   Test restore configurations
*/
GT_VOID prvTgfPolicerECNMarkingRestore();

/**
* @internal prvTgfPolicerECNMarkingGenerateTraffic function
* @endinternal
*
* @brief   Test generate traffic and check results
*/
GT_VOID prvTgfPolicerECNMarkingGenerateTraffic();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfPolicerECNMarkingCheck */



