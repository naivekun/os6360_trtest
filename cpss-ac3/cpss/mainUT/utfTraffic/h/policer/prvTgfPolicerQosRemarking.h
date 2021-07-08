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
* @file prvTgfPolicerQosRemarking.h
*
* @brief Policer VLAN Counting UT.
*
* @version   2.
********************************************************************************
*/
#ifndef __prvTgfPolicerQosRemarkingh
#define __prvTgfPolicerQosRemarkingh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfPolicerGen.h>

/**
* @internal prvTgfPolicerQosRemarkingVlanConfigurationSet function
* @endinternal
*
* @brief   Set configuration
*/
GT_VOID prvTgfPolicerQosRemarkingVlanConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfPolicerQosRemarkingConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @param[in] policerStage             - Policer stage
*                                       None
*/
GT_VOID prvTgfPolicerQosRemarkingConfigurationRestore
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT policerStage
);

/**
* @internal prvTgfPolicerIngressQosRemarkingByEntryTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfPolicerIngressQosRemarkingByEntryTrafficGenerate
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT policerStage
);

/**
* @internal prvTgfPolicerIngressQosRemarkingByTableTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfPolicerIngressQosRemarkingByTableTrafficGenerate
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT policerStage
);

/**
* @internal prvTgfPolicerEgressQosRemarkingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfPolicerEgressQosRemarkingTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfPolicerIngressQosRemarkingByTableGreenTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfPolicerIngressQosRemarkingByTableGreenTrafficGenerate
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT policerStage
);


/**
* @internal prvTgfPolicerWSRateRatioRateTypeTest function
* @endinternal
*
* @brief   test checks different cir and cbs combinations with 
*          different rateTypes
* */ 
GT_VOID prvTgfPolicerWSRateRatioRateTypeTest
(
    GT_VOID
);

/**
* @internal prvTgfPolicerWSRateRatioRateTypeSet function
* @endinternal
*
* @brief   set test configuration
*/
GT_VOID prvTgfPolicerWSRateRatioRateTypeSet
(
    GT_VOID
);


/**
* @internal prvTgfPolicerWSRateRatioRateTypeGenerateTraffic
*           function
* @endinternal
*
* @brief   generate ws traffic and check port rate
*/
GT_VOID prvTgfPolicerWSRateRatioRateTypeGenerateTraffic
(
    GT_U32 cir,
    GT_U32 cbs
);


/**
* @internal prvTgfPolicerWSRateRatioRateTypeRestore function
* @endinternal
*
* @brief   Restore Global test configuration
*/
GT_VOID prvTgfPolicerWSRateRatioRateTypeRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfPolicerQosRemarkingh */



