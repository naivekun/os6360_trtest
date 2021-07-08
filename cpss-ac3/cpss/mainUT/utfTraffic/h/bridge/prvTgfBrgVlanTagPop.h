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
* @file prvTgfBrgVlanTagPop.h
*
* @brief VLAN Tag words Pop.
*
* @version   2
********************************************************************************
*/
#ifndef __prvTgfBrgVlanTagPop_h
#define __prvTgfBrgVlanTagPop_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfBridgeGen.h>

/**
* @internal prvTgfBrgVlanTagPopTtiTest function
* @endinternal
*
* @brief   Test on VLAN Tag Ingress Pop 0,4,8 Tag bytes.
*         The correct TTI removing.
* @param[in] numBytesToPop            - enumeration to set the number of bytes to pop.
*                                      (for popping 0/4/8/6 bytes )
* @param[in] withTtiActionReassignSrcEport - indication that need TTI action that
*                                      will do re-assign src EPort
*                                      (when GT_FALSE - meaning that the device will also
*                                      not do 'L2 reparse' so the popped tags are those
*                                      AFTER tag0,tag1)
*                                       None
*/
GT_VOID prvTgfBrgVlanTagPopTtiTest
(
    IN PRV_TGF_BRG_VLAN_NUM_BYTES_TO_POP_ENT   numBytesToPop,
    IN GT_BOOL  withTtiActionReassignSrcEport
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfBrgVlanTagPop_h */


