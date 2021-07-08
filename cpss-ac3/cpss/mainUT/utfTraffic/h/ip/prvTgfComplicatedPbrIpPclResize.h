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
* @file prvTgfComplicatedPbrIpPclResize.h
*
* @brief Complicated PBR IP-PCL Resize
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfComplicatedPbrIpPclResizeh
#define __prvTgfComplicatedPbrIpPclResizeh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfComplicatedPbrIpPclResizeBaseConfigSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfComplicatedPbrIpPclResizeBaseConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfComplicatedPbrIpPclResizeRouteConfigSet function
* @endinternal
*
* @brief   Set PBR Configuration
*/
GT_VOID prvTgfComplicatedPbrIpPclResizeRouteConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfComplicatedPbrIpPclResizeTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] testStageNum             - test stage
*                                       None
*/
GT_VOID prvTgfComplicatedPbrIpPclResizeTrafficGenerate
(
    IN GT_U32       testStageNum
);

/**
* @internal prvTgfComplicatedPbrIpPclResizeAdditionalRouteConfigSet function
* @endinternal
*
* @brief   Set Additional PBR Configuration
*/
GT_VOID prvTgfComplicatedPbrIpPclResizeAdditionalRouteConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfComplicatedPbrIpPclResizeConfigRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfComplicatedPbrIpPclResizeConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfComplicatedPbrIpPclResizeh */


