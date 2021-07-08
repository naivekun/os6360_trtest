/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfBrgCoreToCustomer.h
*
* DESCRIPTION:
*        Core to customer 
*
* FILE REVISION NUMBER:
*       $Revision: 2 $
*
*******************************************************************************/
#ifndef __prvTgfBrgNestedCoreToCustomerh
#define __prvTgfBrgNestedCoreToCustomerh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <trafficEngine/tgfTrafficEngine.h>

/**
* @internal prvTgfBrgNestedCoreToCustomerEgressCascadePortTestAsDsa function
* @endinternal
*
* @brief   set the tests to be with egress DSA tag (extended DSA/eDSA) on the egress ports.
*/
GT_STATUS prvTgfBrgNestedCoreToCustomerEgressCascadePortTestAsDsa
(
    TGF_DSA_TYPE_ENT    egressDsaType
);

/**
* @internal prvTgfBrgNestedCoreToCustomerConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfBrgNestedCoreToCustomerConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgNestedCoreToCustomerTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBrgNestedCoreToCustomerTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgNestedCoreToCustomerEgressCascadePortTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBrgNestedCoreToCustomerEgressCascadePortTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgNestedCoreToCustomerConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgNestedCoreToCustomerConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfBrgNestedCoreToCustomerh */



