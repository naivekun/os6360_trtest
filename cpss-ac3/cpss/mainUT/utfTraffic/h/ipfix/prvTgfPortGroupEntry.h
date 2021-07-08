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
* @file prvTgfPortGroupEntry.h
*
* @brief IPFIX Entry per port group test for IPFIX declarations
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfPortGroupEntryh
#define __prvTgfPortGroupEntryh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfIpfixPortGroupEntryTestInit function
* @endinternal
*
* @brief   IPFIX per port group entry manipulation test configuration set.
*/
GT_VOID prvTgfIpfixPortGroupEntryTestInit
(
    GT_VOID
);

/**
* @internal prvTgfIpfixPortGroupEntryTestTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfIpfixPortGroupEntryTestTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfIpfixPortGroupEntryTestRestore function
* @endinternal
*
* @brief   IPFIX per port group entry manipulation test configuration restore.
*/
GT_VOID prvTgfIpfixPortGroupEntryTestRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfPortGroupEntryh */

