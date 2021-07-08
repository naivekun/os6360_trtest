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
* @file prvTgfLogicalTargetIpv4Uc.h
*
* @brief Logical Target Mapping Ipv4 Uc
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfLogicalTargetIpv4Uch
#define __prvTgfLogicalTargetIpv4Uch

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfLogicalTargetIpv4UcBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfLogicalTargetIpv4UcBaseConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfLogicalTargetIpv4UcMappingConfigurationSet function
* @endinternal
*
* @brief   Set Logical Target Mapping configuration
*/
GT_VOID prvTgfLogicalTargetIpv4UcMappingConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfLogicalTargetIpv4UcRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*/
GT_VOID prvTgfLogicalTargetIpv4UcRouteConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfLogicalTargetIpv4UcTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfLogicalTargetIpv4UcTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfLogicalTargetIpv4UcConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfLogicalTargetIpv4UcConfigurationRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfLogicalTargetIpv4Uch */


