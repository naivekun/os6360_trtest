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
* @file prvTgfPolicerVlanCounting.h
*
* @brief Policer VLAN Counting UT.
*
* @version   1.1
********************************************************************************
*/
#ifndef __prvTgfPolicerVlanCountingh
#define __prvTgfPolicerVlanCountingh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfPolicerStageGen.h>

/**
* @internal prvTgfDefPolicerVlanConfigurationSet function
* @endinternal
*
* @brief   Set Default Vlan Policer configuration.
*/
GT_VOID prvTgfDefPolicerVlanConfigurationSet
(
    GT_VOID    
);


/**
* @internal prvTgfPolicerVlanConfigurationRestore function
* @endinternal
*
* @brief   None
*/
GT_VOID prvTgfPolicerVlanConfigurationRestore
(
    GT_VOID
);


/**
* @internal prvTgfDefPolicerVlanStageConfigurationSet function
* @endinternal
*
* @brief   None
*/
GT_VOID prvTgfDefPolicerVlanStageConfigurationSet
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT  stage    
);

/**
* @internal prvTgfPolicerVlanStageConfigurationRestore function
* @endinternal
*
* @brief   None
*/
GT_VOID prvTgfPolicerVlanStageConfigurationRestore
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT  stage    
);

/**
* @internal prvTgPolicerVlanStagefTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send from device's port 8, 5 unknown unicast packets:
*         macDa = 00:00:00:00:11:11,
*         macSa = 00:00:00:00:22:22,
*         Success Criteria:
*         5 Packet is captured on ports 0,18,23.
*         5 Packets are counted in Vlan counter [2].
*         Send from device's port 8, 5 unknown unicast packets with vlan 3:
*         macDa = 00:00:00:00:11:11,
*         macSa = 00:00:00:00:22:22,
*         Success Criteria:
*         5 Packet is captured on ports 0,18,23.
*         0 Packets are counted in Vlan counter [2].
*         Send from device's port 8, 5 known unicast incremented :
*         Repeat it 5 times, while each time, another trigger command is
*         added to Vlan Counting Trigger.
*         start macDa = 00:00:00:00:00:10,
*         macSa = 00:00:00:00:22:22,
*         Success Criteria:
*         1 Packet is counted in Vlan counter [2], for each packet command:
*         CPSS_MAC_TABLE_FRWRD_E, CPSS_MAC_TABLE_DROP_E,
*         CPSS_MAC_TABLE_CNTL_E, CPSS_MAC_TABLE_MIRROR_TO_CPU_E,
*         CPSS_MAC_TABLE_SOFT_DROP_E.
*/
GT_VOID prvTgPolicerVlanStagefTrafficGenerate
(
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT    stage
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfPolicerVlanCountingh */



