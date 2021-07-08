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
* @file prvTgfBrgGenPropL2ControlProtCmdAf.h
*
* @brief Cisco L2 protocol commands / Proprietary Layer 2 control MC
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfBrgGenPropL2ControlProtCmdAfh
#define __prvTgfBrgGenPropL2ControlProtCmdAfh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfBrgGenPropL2ControlSetAf function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfBrgGenPropL2ControlSetAf(
    GT_VOID
);

/**
* @internal prvTgfBrgGenPropL2ControlTestAf function
* @endinternal
*
* @brief   Manage the test settings and traffic
*/
GT_VOID prvTgfBrgGenPropL2ControlTestAf(
    GT_VOID
);

/**
* @internal prvTgfBrgGenPropL2ControlRestoreAf function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgGenPropL2ControlRestoreAf(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* prvTgfBrgGenPropL2ControlProtCmdAf */


