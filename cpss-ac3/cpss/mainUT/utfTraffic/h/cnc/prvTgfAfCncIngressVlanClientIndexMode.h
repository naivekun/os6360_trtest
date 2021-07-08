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
* @file prvTgfAfCncIngressVlanClientIndexMode.h
*
* @brief Centralized Counters (Cnc)
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfAfCncIngressVlanClientIndexModeh
#define __prvTgfAfCncIngressVlanClientIndexModeh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



/**
* @internal prvTgfAfCncIngressVlanClientIndexModeConfigure function
* @endinternal
*
* @brief   This function configures Cnc VLAN client index mode test
*/
GT_VOID prvTgfAfCncIngressVlanClientIndexModeConfigure
(
    GT_VOID
);


/**
* @internal prvTgfAfCncIngressVlanClientIndexModeTest function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet:
*         Check counters.
*/
GT_VOID prvTgfAfCncIngressVlanClientIndexModeTest
(
    GT_VOID
);


/**
* @internal prvTgfAfCncIngressVlanClientIndexModeReset function
* @endinternal
*
* @brief   reset all configurations
*/
GT_VOID prvTgfAfCncIngressVlanClientIndexModeReset
(
    GT_VOID
);





#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfAfCncIngressVlanClientIndexModeh */


