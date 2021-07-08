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
* @file prvTgfSampling.h
*
* @brief Sampling tests for IPFIX declarations
*
* @version   2
********************************************************************************
*/
#ifndef __prvTgfSamplingh
#define __prvTgfSamplingh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfIpfixSamplingSimpleTestInit function
* @endinternal
*
* @brief   Set test configuration:
*         Enable the Ingress Policy Engine set Pcl rule.
*/
GT_VOID prvTgfIpfixSamplingSimpleTestInit
(
    GT_VOID
);

/**
* @internal prvTgfIpfixSamplingToCpuTestInit function
* @endinternal
*
* @brief   Set test configuration:
*         Enable the Ingress Policy Engine set Pcl rule.
*/
GT_VOID prvTgfIpfixSamplingToCpuTestInit
(
    GT_VOID
);

/**
* @internal prvTgfIpfixEgressSamplingToCpuTestInit function
* @endinternal
*
* @brief   Set test configuration:
*         Enable the Egress Policy Engine set Pcl rule.
*/
GT_VOID prvTgfIpfixEgressSamplingToCpuTestInit
(
    GT_VOID
);

/**
* @internal prvTgfIpfixAlarmEventsTestInit function
* @endinternal
*
* @brief   Set test configuration:
*         Enable the Ingress Policy Engine set Pcl rule.
*/
GT_VOID prvTgfIpfixAlarmEventsTestInit
(
    GT_VOID
);

/**
* @internal prvTgfIpfixTimestampToCpuTestInit function
* @endinternal
*
* @brief   Set test configuration:
*/
GT_VOID prvTgfIpfixTimestampToCpuTestInit
(
    GT_VOID
);

/**
* @internal prvTgfIpfixSamplingSimpleTestTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfIpfixSamplingSimpleTestTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfIpfixEgressSamplingTestTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfIpfixEgressSamplingTestTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfIpfixSamplingToCpuTestTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfIpfixSamplingToCpuTestTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfIpfixAlarmEventsTestTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfIpfixAlarmEventsTestTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfIpfixTimestampToCpuTestTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfIpfixTimestampToCpuTestTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfIpfixSamplingTestRestore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfIpfixSamplingTestRestore
(
    GT_VOID
);

/**
* @internal prvTgfIpfixEgressSamplingTestRestore function
* @endinternal
*
* @brief   Restore configuration
*/

GT_VOID prvTgfIpfixEgressSamplingTestRestore
(
    GT_VOID
);
/**
* @internal prvTgfIpfixTimestampToCpuTestRestore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfIpfixTimestampToCpuTestRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfSamplingh */

