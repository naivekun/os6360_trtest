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
* @file prvTgfCscdPortStackAggregation.h
*
* @brief CPSS Cascade Stack Aggregation
*
* @version   2
********************************************************************************
*/
#ifndef __prvTgfCscdPortStackAggregation
#define __prvTgfCscdPortStackAggregation

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfCscdPortStackAggregationConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set FDB entry with device number 31, port 27, MAC 00:00:00:00: 00:56.
*         - Set lookup mode to Port for accessing the Device Map table.
*         - Set the cascade map table
*         - Target device 0, target port 0, link type port, link number 23.
*/
GT_VOID prvTgfCscdPortStackAggregationConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfCscdPortStackAggregationTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Set FDB entry with device number 15, port 6, MAC 00:00:00:00: 00:56.
*         Set the cascade map table
*         - Target device 15, target port 6, link type port, link number 18.
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:56,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         1 packet is captured on ports 18 with updated DSA tag (src info).
*         Set FDB entry with device number 15, port 6, MAC 00:00:00:00: 00:56.
*         Set the cascade map table
*         - Target device 15, target port 6, link type port, link number ??.
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:56,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         1 packet is captured on ports ?? with updated DSA tag (src info).
*/
GT_VOID prvTgfCscdPortStackAggregationTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfCscdPortStackAggregationConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfCscdPortStackAggregationConfigurationRestore
(
    GT_VOID
);


/**
* @internal prvTgfCscdPortStackAggregationFullPathConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set TTI rule: action, pattern and mask
*         - Set stack aggregation enabled and configure.
*         - Set lookup mode to Port for accessing the Device Map table.
*         - Set the cascade map table
*         - Target device 0, target port 0, link type port, link number 23.
*/
GT_VOID prvTgfCscdPortStackAggregationFullPathConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfCscdPortStackAggregationFullPathTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 8 packet:
*         macDa = 00:00:00:00:00:56,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         1 packet is captured on ports 18.
*/
GT_VOID prvTgfCscdPortStackAggregationFullPathTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfCscdPortStackAggregationFullPathConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfCscdPortStackAggregationFullPathConfigurationRestore
(
    GT_VOID
);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfCscdPortStackAggregation */


