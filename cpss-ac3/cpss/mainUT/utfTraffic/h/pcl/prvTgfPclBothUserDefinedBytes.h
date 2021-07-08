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
* @file prvTgfPclBothUserDefinedBytes.h
*
* @brief Second Lookup match
*
* @version   2
********************************************************************************
*/
#ifndef __prvTgfPclBothUserDefinedBytesh
#define __prvTgfPclBothUserDefinedBytesh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfPclBothUserDefinedBytesConfigurationSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfPclBothUserDefinedBytesConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfPclBothUserDefinedBytesPclConfigurationSet function
* @endinternal
*
* @brief   Set test PCL configuration
*/
GT_VOID prvTgfPclBothUserDefinedBytesPclConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfPclBothUserDefinedBytesTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfPclBothUserDefinedBytesTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfPclBothUserDefinedBytesConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfPclBothUserDefinedBytesConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfPclUserDefinedBytes_L2_0_8_ConfigurationSet function
* @endinternal
*
* @brief   Set test PCL configuration
*/
GT_VOID prvTgfPclUserDefinedBytes_L2_0_8_ConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfPclUdbL3Minus2IngressLlcNonSnapTest function
* @endinternal
*
* @brief   Test on IPCL L3Minus2 anchor for LLC Non SNAP packets
*         UDBs for anchor L3Minus2 offset 0-3.
*/
GT_VOID prvTgfPclUdbL3Minus2IngressLlcNonSnapTest
(
    GT_VOID
);

/**
* @internal prvTgfPclUserDefinedBytes_L2_0_3_ConfigurationSet function
* @endinternal
*
* @brief   Set test PCL configuration
*/
GT_VOID prvTgfPclUserDefinedBytes_L2_0_3_ConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfPclUserDefinedBytes_L4_0_15_ConfigurationSet function
* @endinternal
*
* @brief   Set test PCL configuration
*/
GT_VOID prvTgfPclUserDefinedBytes_L4_0_15_ConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfPclUserDefinedBytes_L2_0_11_ConfigurationSet function
* @endinternal
*
* @brief   Set test PCL configuration
*/
GT_VOID prvTgfPclUserDefinedBytes_L2_0_11_ConfigurationSet
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfPclBothUserDefinedBytesh */

