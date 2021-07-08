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
* @file prvTgfBrgEgrTagModifNonDsaTaggedEgrPort.h
*
* @brief Egress Tag Modification on Non-DSA-Tagged Egress Port
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfBrgEgrTagModifNonDsaTaggedEgrPorth
#define __prvTgfBrgEgrTagModifNonDsaTaggedEgrPorth

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfBrgEgrTagModifNonDsaTaggedEgrPortConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfBrgEgrTagModifNonDsaTaggedEgrPortConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgEgrTagModifNonDsaTaggedEgrPortTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBrgEgrTagModifNonDsaTaggedEgrPortTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgEgrTagModifNonDsaTaggedEgrPortConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgEgrTagModifNonDsaTaggedEgrPortConfigRestore
(
    GT_VOID
);




#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfBrgEgrTagModifNonDsaTaggedEgrPorth */



