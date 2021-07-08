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
* @file prvTgfPolicerPortModeAddressSelect.h
*
* @brief Policer Address Select mode UT.
*
* @version   1.1
********************************************************************************
*/
#ifndef __prvTgfPolicerPortModeAddressSelecth
#define __prvTgfPolicerPortModeAddressSelecth

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfPolicerStageGen.h>


/**
* @internal prvTgfPolicerPortModeAddressSelectConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         Create VLAN 2 on all ports (0,8,18,23)
*/
GT_VOID prvTgfPolicerPortModeAddressSelectConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfPolicerPortModeAddressSelectFullRedCmdDropTrafficGenerate function
* @endinternal
*
* @brief   Configure Address Select mode to Full.
*         Configure Port mode for Ingress Stage 0.
*         Configure different Storm Type index for each port for unicast.
*         Configure for each port and storm type index matching meter entry,
*         with two red packets and red command - drop.
*         Configure for each port and storm type index not matching meter
*         entry (match for compressed mode), with two red packets and red
*         command - forward.
*         Generate traffic:
*         Send from each port 2 unicast packets:
*         macDa = 00:00:00:44:55:44
*         macSa = 00:00:00:00:11:BB
*         Success Criteria:
*         All packets are dropped.
*         Management counters of each port counted two drop packets.
* @param[in] stage                    - Policer stage
*                                       None
*/
GT_VOID prvTgfPolicerPortModeAddressSelectFullRedCmdDropTrafficGenerate
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT stage
);

/**
* @internal prvTgfPolicerPortModeAddressSelectFullRedCmdFrwdTrafficGenerate function
* @endinternal
*
* @brief   Configure Address Select mode to Full.
*         Configure Port mode for Ingress Stage 0.
*         Configure different Storm Type index for each port for unicast.
*         Configure for each port and storm type index matching meter entry,
*         with two red packets and red command - forward.
*         Configure for each port and storm type index not matching meter
*         entry (match for compressed mode), with two red packets and red
*         command - drop.
*         Generate traffic:
*         Send from each port 2 unicast packets:
*         macDa = 00:00:00:44:55:44
*         macSa = 00:00:00:00:11:BB
*         Success Criteria:
*         All packets received, each time on another port.
*         Management counters of each port counted two red packets.
* @param[in] stage                    - Policer stage
*                                       None
*/
GT_VOID prvTgfPolicerPortModeAddressSelectFullRedCmdFrwdTrafficGenerate
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT stage
);

/**
* @internal prvTgfPolicerPortModeAddressSelectCompresYellowCmdDropTrafficGenerate function
* @endinternal
*
* @brief   Configure Address Select mode to Compressed.
*         Configure Port mode for Ingress Stage 1.
*         Configure different Storm Type index for each port for unkown unicast.
*         Configure for each port and storm type index matching meter entry,
*         with two yellow packets and yellow command - drop.
*         Configure for each port and storm type index not matching meter
*         entry (match for compressed mode), with two yellow packets and yellow
*         command - forward.
*         Generate traffic:
*         Send from each port 2 unknown unicast packets:
*         macDa = 00:00:00:00:11:BB
*         macSa = 00:00:00:00:11:AA
*         Success Criteria:
*         All packets are dropped.
*         Management counters of each port counted two drop packets.
* @param[in] stage                    - Policer stage
*                                       None
*/
GT_VOID prvTgfPolicerPortModeAddressSelectCompresYellowCmdDropTrafficGenerate
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT stage
);

/**
* @internal prvTgfPolicerPortModeAddressSelectCompresYellowCmdFrwdTrafficGenerate function
* @endinternal
*
* @brief   Configure Address Select mode to Compressed.
*         Configure Port mode for Ingress Stage 1.
*         Configure different Storm Type index for each port for unkown unicast.
*         Configure for each port and storm type index matching meter entry,
*         with two yellow packets and yellow command - forward.
*         Configure for each port and storm type index not matching meter
*         entry (match for compressed mode), with two yellow packets and yellow
*         command - drop.
*         Generate traffic:
*         Send from each port 2 unknown unicast packets:
*         macDa = 00:00:00:00:11:BB
*         macSa = 00:00:00:00:11:AA
*         Success Criteria:
*         All packets should be received on all ports.
*         Management counters of each port counted two yellow packets.
* @param[in] stage                    - Policer stage
*                                       None
*/
GT_VOID prvTgfPolicerPortModeAddressSelectCompresYellowCmdFrwdTrafficGenerate
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT stage
);

/**
* @internal prvTgfPolicerPortModeAddressSelectGlobalConfigurationRestore function
* @endinternal
*
* @brief   Restore Global test configuration
*/
GT_VOID prvTgfPolicerPortModeAddressSelectGlobalConfigurationRestore
(
    void
);

/**
* @internal prvTgfPolicerPortModeAddressSelectFullGreenTrafficGenerate function
* @endinternal
*
* @brief   Configure Address Select mode to Full.
*         Configure Port mode.
*         Configure different Storm Type index for each port for known unicast.
*         Configure for each port and storm type index matching meter entry,
*         with two green packets.
*         Configure for each port and storm type index not matching meter
*         entry (match for compressed mode), with two yellow packets and yellow
*         command - forward.
*         Generate traffic:
*         Send from each port 2 unknown unicast packets:
*         macDa = 00:00:00:00:11:BB
*         macSa = 00:00:00:00:11:AA
*         Success Criteria:
*         All packets are received.
*         Management counters of each port counted two green packets.
* @param[in] stage                    - Policer stage
*                                       None
*/
GT_VOID prvTgfPolicerPortModeAddressSelectFullGreenTrafficGenerate
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT stage
);

/**
* @internal prvTgfPolicerPortModeAddressSelectCompresGreenTrafficGenerate function
* @endinternal
*
* @brief   Configure Address Select mode to Compressed.
*         Configure Port mode.
*         Configure different Storm Type index for each port for unkown unicast.
*         Configure for each port and storm type index matching meter entry,
*         with two green packets.
*         Configure for each port and storm type index not matching meter
*         entry (match for compressed mode), with two yellow packets and yellow
*         command - forward.
*         Generate traffic:
*         Send from each port 2 unknown unicast packets:
*         macDa = 00:00:00:00:11:BB
*         macSa = 00:00:00:00:11:AA
*         Success Criteria:
*         All packets are received.
*         Management counters of each port counted two green packets.
* @param[in] stage                    - Policer stage
*                                       None
*/
GT_VOID prvTgfPolicerPortModeAddressSelectCompresGreenTrafficGenerate
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT stage
);

/**
* @internal prvTgfPolicerPortModeAddressSelectConfigurationRestore function
* @endinternal
*
* @brief   Restore Policer per stage test configuration
*
* @param[in] stage                    - Policer stage
*                                       None
*/
GT_VOID prvTgfPolicerPortModeAddressSelectConfigurationRestore
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT stage
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfPolicerPortModeAddressSelecth */



