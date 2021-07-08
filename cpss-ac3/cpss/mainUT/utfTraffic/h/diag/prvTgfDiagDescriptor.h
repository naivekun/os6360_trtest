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
* @file prvTgfDiagDescriptor.h
*
* @brief Diag Descriptor
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfDiagDescriptor
#define __prvTgfDiagDescriptor

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfDiagDescriptorBridgeConfigurationSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfDiagDescriptorBridgeConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfDiagDescriptorBridgeTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfDiagDescriptorBridgeTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfDiagDescriptorBridgeRestore function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfDiagDescriptorBridgeRestore
(
    GT_VOID
);

/**
* @internal prvTgfDiagDescriptorPolicerConfigurationSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfDiagDescriptorPolicerConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfDiagDescriptorPolicerTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfDiagDescriptorPolicerTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfDiagDescriptorPolicerRestore function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfDiagDescriptorPolicerRestore
(
    GT_VOID
);

/**
* @internal prvTgfDiagDescriptorTunnelTermConfigurationSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfDiagDescriptorTunnelTermConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfDiagDescriptorTunnelTermTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfDiagDescriptorTunnelTermTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfDiagDescriptorTunnelTermRestore function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfDiagDescriptorTunnelTermRestore
(
    GT_VOID
);

/**
* @internal prvTgfDiagDescriptorPclConfigurationSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfDiagDescriptorPclConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfDiagDescriptorPclTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfDiagDescriptorPclTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfDiagDescriptorPclRestore function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfDiagDescriptorPclRestore
(
    GT_VOID
);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfDiagDescriptor */


