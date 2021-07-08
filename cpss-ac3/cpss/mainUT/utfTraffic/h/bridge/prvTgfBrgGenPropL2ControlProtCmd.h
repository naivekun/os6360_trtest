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
* @file prvTgfBrgGenPropL2ControlProtCmd.h
*
* @brief Cisco L2 protocol commands / Proprietary Layer 2 control MC
*
* @version   2
********************************************************************************
*/
#ifndef __prvTgfBrgGenPropL2ControlProtCmdh
#define __prvTgfBrgGenPropL2ControlProtCmdh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfBrgGenPropL2ControlSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfBrgGenPropL2ControlSet(
    GT_VOID
);

/**
* @internal prvTgfBrgGenPropL2ControlTest function
* @endinternal
*
* @brief   Manage the test settings and traffic
*/
GT_VOID prvTgfBrgGenPropL2ControlTest(
    GT_VOID
);

/**
* @internal prvTgfBrgGenPropL2ControlRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgGenPropL2ControlRestore(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* prvTgfBrgGenPropL2ControlProtCmd */


