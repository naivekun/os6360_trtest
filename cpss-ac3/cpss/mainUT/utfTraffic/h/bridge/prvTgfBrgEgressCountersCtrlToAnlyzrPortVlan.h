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
* @file prvTgfBrgEgressCountersCtrlToAnlyzrPortVlan.h
*
* @brief The test checks that packets are received
* in counter set 1, field <SecFilterDisc> in all modes
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfBrgEgressCountersCtrlToAnlyzrPortVlanh
#define __prvTgfBrgEgressCountersCtrlToAnlyzrPortVlanh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfBrgEgressCountersCtrlToAnlyzrPortVlanConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfBrgEgressCountersCtrlToAnlyzrPortVlanConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgEgressCountersCtrlToAnlyzrPortVlanTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBrgEgressCountersCtrlToAnlyzrPortVlanTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgEgressCountersCtrlToAnlyzrPortVlanConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgEgressCountersCtrlToAnlyzrPortVlanConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfBrgEgressCountersCtrlToAnlyzrPortVlanh */


