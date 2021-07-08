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
* @file prvTgfBrgVlanFws.h
*
* @brief VLAN Full Wire Speed tests.
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfBrgVlanFws_h
#define __prvTgfBrgVlanFws_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfBrgVlanFwsTest function
* @endinternal
*
* @brief   Function tests Vlan member add/remove under traffic.
*/
GT_VOID prvTgfBrgVlanFwsTest
(
    IN GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/**
* @internal prvTgfBrgFDBFwsTraffic function
* @endinternal
*
* @brief   Full wire speed FDB access test :
*         1. Generate FWS traffic
*         2. Check FWS traffic
*         3. print FDB Entrys
*         4. Check agigng daemon
*         5. Stop traffic and do reset
*/


GT_VOID prvTgfBrgFDBFwsTraffic 
(
  IN GT_VOID 
); 
  


#endif /* __prvTgfBrgVlanFws_h */


