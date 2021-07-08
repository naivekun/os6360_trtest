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
* @file tgfPolicerStageGen.h
*
* @brief Generic API for Policer.
*
* @version   2
********************************************************************************
*/
#ifndef __tgfPolicerStageGenh
#define __tgfPolicerStageGenh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfPolicerGen.h>

#ifdef CHX_FAMILY
    #include <cpss/dxCh/dxChxGen/policer/cpssDxChPolicer.h>
    #include <cpss/dxCh/dxCh3/policer/cpssDxCh3Policer.h>
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    #include <cpss/exMxPm/exMxPmGen/policer/cpssExMxPmPolicer.h>
#endif /* EXMXPM_FAMILY */


/******************************************************************************\
 *                          Private type definitions                          *
\******************************************************************************/


/******************************************************************************\
 *                       CPSS generic API section                             *
\******************************************************************************/

/* DB to store Default Policer Counting modes */
extern PRV_TGF_POLICER_COUNTING_MODE_ENT prvTgfPolicerCntrModeDefaultArr[PRV_TGF_POLICER_STAGE_NUM];

/**
* @internal prvTgfPolicerVlanCountingStageDefValuesSet function
* @endinternal
*
* @brief   Set Policer VLAN Default values.
*         1. Counting mode set to VLAN.
*         2. Vlan counting mode set to Packet.
*         3. Enable VLAN counting triggering only for
*         CPSS_PACKET_CMD_FORWARD_E packet cmd.
*         4. Flush counters.
*         5. Reset VLAN counters.
* @param[in] devNum                   - device number
*                                      counterSetIndex - policing counters set index
*                                      countersPtr     - (pointer to) counters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or counterSetIndex
* @retval GT_BAD_PTR               - on one of the parameters is NULL pointer
*/
GT_STATUS prvTgfPolicerVlanCountingStageDefValuesSet
(
    IN  GT_U8                               devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT      stage,
    IN  GT_U16                              defVid
);

/**
* @internal prvTgfPolicerVlanCountingStageDefValuesRestore function
* @endinternal
*
* @brief   Set Policer VLAN Default values.
*         1. Counting mode set to Billing.
*         2. Vlan counting mode set to Packet.
*         3. Enable VLAN counting for all commands.
*         4. Flush counters.
*         5. Reset VLAN counters.
* @param[in] devNum                   - device number
*                                      counterSetIndex - policing counters set index
*                                      countersPtr     - (pointer to) counters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or counterSetIndex
* @retval GT_BAD_PTR               - on one of the parameters is NULL pointer
*/
GT_STATUS prvTgfPolicerVlanCountingStageDefValuesRestore
(
    IN  GT_U8                               devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT      stage,
    IN  GT_U16                              defVid
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __tgfPolicerStageGenh */


