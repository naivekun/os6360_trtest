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
* @file prvTgfWraparound.h
*
* @brief Wraparound tests for IPFIX declarations
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfWraparoundh
#define __prvTgfWraparoundh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfIpfixWraparoundTestInit function
* @endinternal
*
* @brief   Set test configuration:
*         Enable the Ingress Policy Engine set Pcl rule.
*/
GT_VOID prvTgfIpfixWraparoundTestInit
(
    GT_VOID
);

/**
* @internal prvTgfIpfixWraparoundFreezeTestTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfIpfixWraparoundFreezeTestTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfIpfixWraparoundClearTestTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfIpfixWraparoundClearTestTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfIpfixWraparoundMaxBytesTestTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfIpfixWraparoundMaxBytesTestTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfIpfixWraparoundTestRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfIpfixWraparoundTestRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfWraparoundh */

