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
* @file prvTgfStc.h
*
* @brief CPSS STC (sampling to CPU)
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfStc
#define __prvTgfStc

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfStcGen.h>

/**
* @internal prvTgfStcBasicConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         1. create VLAN with it's members
*         2. enable ingress/egress STC
*         3. set reload mode
*         4. set STC limit on ingress/egress port
*         5. reset sampled counter
*         6. CPU RX table:
*         clear table , set capturing mode , start capture.
* @param[in] stcType                  - STC source type : ingress/egress
* @param[in] reloadMode               - reload mode : continuous / triggered
*                                       None
*/
GT_VOID prvTgfStcBasicConfigurationSet
(
    IN PRV_TGF_STC_TYPE_ENT                 stcType,
    IN PRV_TGF_STC_COUNT_RELOAD_MODE_ENT    reloadMode
);

/**
* @internal prvTgfStcBasicTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic , with next steps:
*         1. clear counters
*         2. build packet (TX setup)
*         3. send packets from 'Sender port' (regardless to ingress/egress STC)
*         4. check port counters (traffic do flooding)
*         for trigger mode --> change 7 times the limit and resent packets.
*         5. check CPU got the 'expected' num of STC packets from the ingress/egress port.
*         6. check that sampled counter in the 'per port' STC entry match the 'expected'
* @param[in] stcType                  - STC source type : ingress/egress
* @param[in] reloadMode               - reload mode : continuous / triggered
*                                       None
*/
GT_VOID prvTgfStcBasicTrafficGenerate
(
    IN PRV_TGF_STC_TYPE_ENT                 stcType,
    IN PRV_TGF_STC_COUNT_RELOAD_MODE_ENT    reloadMode
);

/**
* @internal prvTgfStcBasicConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*
* @param[in] stcType                  - STC source type : ingress/egress
* @param[in] reloadMode               - reload mode : continuous / triggered
*                                       None
*/
GT_VOID prvTgfStcBasicConfigurationRestore
(
    IN PRV_TGF_STC_TYPE_ENT                 stcType,
    IN PRV_TGF_STC_COUNT_RELOAD_MODE_ENT    reloadMode
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfStc */


