/*******************************************************************************
*              (c), Copyright 2017, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfBrgVlanInvalidate.c
*
* DESCRIPTION:
*       Check Sip5.20 VLAN entry invalidation. Sip5.20 has no valid bit on
*       Egress VLAN - so VLAN entry invalidation may works uncorrectly without
*       port members reset.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/


/**
* @internal prvTgfBrgVlanInvalidateConfigurationSet function
* @endinternal
*
* @brief   Set configuration.
*/
GT_VOID prvTgfBrgVlanInvalidateConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanInvalidateConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration.
*/
GT_VOID prvTgfBrgVlanInvalidateConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfBrgVlanInvalidateTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfBrgVlanInvalidateTrafficGenerate
(
    GT_VOID
);

