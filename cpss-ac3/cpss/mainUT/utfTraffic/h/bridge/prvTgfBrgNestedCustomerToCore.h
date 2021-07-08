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
* @file prvTgfBrgNestedCustomerToCore.h
*
* @brief Nested customer to core
*
* @version   2
********************************************************************************
*/
#ifndef __prvTgfBrgNestedCustomerToCoreh
#define __prvTgfBrgNestedCustomerToCoreh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <trafficEngine/tgfTrafficEngine.h>
/**
* @internal prvTgfBrgNestedCustomerToCoreConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfBrgNestedCustomerToCoreConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgNestedCustomerToCoreTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBrgNestedCustomerToCoreTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgNestedCustomerToCoreConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgNestedCustomerToCoreConfigRestore
(
    GT_VOID
);

/**
* @internal prvTgfBrgNestedCustomerToCoreTestAsDsa function
* @endinternal
*
* @brief   set the tests to be with egress DSA tag (extended DSA/eDSA) on the egress ports.
*
* @param[in] egressDsaType            - type of DSA tag on egress port
*                                       None
*/
GT_STATUS prvTgfBrgNestedCustomerToCoreTestAsDsa
(
    TGF_DSA_TYPE_ENT    egressDsaType
);
/**
* @internal prvTgfBrgNestedCustomerToCoreEgressCascadePortTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBrgNestedCustomerToCoreEgressCascadePortTrafficGenerate
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfBrgNestedCustomerToCoreh */



