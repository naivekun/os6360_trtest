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
* @file prvTgfBrgGenIcmpV6.h
*
* @brief IPv6 ICMP command test
* @version   1
********************************************************************************
*/
#ifndef __prvTgfBrgGenIcmpV6h
#define __prvTgfBrgGenIcmpV6h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfBrgGenIcmpV6Set function
* @endinternal
*
* @brief   Set test bridge configuration
*/
GT_VOID prvTgfBrgGenIcmpV6Set(
    GT_VOID
);


/**
* @internal prvTgfBrgGenIcmpV6Test function
* @endinternal
*
* @brief   IPv6 ICMP configuration test
*/
GT_VOID prvTgfBrgGenIcmpV6Test(
    GT_VOID
);

/**
* @internal prvTgfBrgGenIcmpV6Restore function
* @endinternal
*
* @brief   Restore configuration
*/

GT_VOID prvTgfBrgGenIcmpV6Restore(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfBrgGenCiscoL2ProtCmdh */



