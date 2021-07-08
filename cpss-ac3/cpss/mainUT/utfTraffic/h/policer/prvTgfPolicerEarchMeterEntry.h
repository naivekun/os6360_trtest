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
* @file prvTgfPolicerEarchMeterEntry.h
*
* @brief Header file for new & updted metering entry fields due to earch format
* tests.
*
* @version   2
********************************************************************************
*/
#ifndef __prvTgfPolicerEarchMeterEntry
#define __prvTgfPolicerEarchMeterEntry

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfPolicerGen.h>

/**
* @internal prvTgfPolicerEarchMeterEntryGlobalConfigurationSet function
* @endinternal
*
* @brief   Create VLAN on all ports
*/
GT_VOID prvTgfPolicerEarchMeterEntryGlobalConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfPolicerEarchMeterEntryGlobalConfigurationRestore function
* @endinternal
*
* @brief   Restore Global test configuration
*/
GT_VOID prvTgfPolicerEarchMeterEntryGlobalConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfPolicerBasicEarchMeterEntryTest function
* @endinternal
*
* @brief   Basic eArch metering entry test
*
* @param[in] stage                    - Policer stage
*                                       None
*/
GT_VOID prvTgfPolicerBasicEarchMeterEntryTest
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT stage
);

/**
* @internal prvTgfPolicerEarchEgrMeterEntryManagementCountCheck function
* @endinternal
*
* @brief   Check Management counters value on egress port
*
* @param[in] mngCntrType              - packet type
*                                       None
*/
GT_VOID prvTgfPolicerEarchEgrMeterEntryManagementCountCheck
(
    IN  PRV_TGF_POLICER_MNG_CNTR_TYPE_ENT   mngCntrType
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfPolicerEarchMeterEntry */



