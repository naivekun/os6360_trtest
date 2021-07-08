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
* @file prvTgfPolicerDsaTagCountingMode.h
*
* @brief DSA Tag Counting Mode test.
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfPolicerDsaTagCountingMode
#define __prvTgfPolicerDsaTagCountingMode

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfPolicerGen.h>

/**
* @internal prvTgfPolicerDsaTagCountingModeTest function
* @endinternal
*
* @brief   Basic DSA Tag Counting Mode test
*
* @param[in] stage                    - Policer stage
*                                       None
*/
GT_VOID prvTgfPolicerDsaTagCountingModeTest
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT stage
);

/**
* @internal prvTgfPolicerDsaTagCountingModeGlobalConfigurationSet function
* @endinternal
*
* @brief   Generic global configurations for eArch metering entry tests
*/
GT_VOID prvTgfPolicerDsaTagCountingModeGlobalConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfPolicerDsaTagCountingModeGlobalConfigurationRestore function
* @endinternal
*
* @brief   Restore Global test configuration
*/
GT_VOID prvTgfPolicerDsaTagCountingModeGlobalConfigurationRestore
(
    GT_VOID
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfPolicerEarchMeterEntry */



