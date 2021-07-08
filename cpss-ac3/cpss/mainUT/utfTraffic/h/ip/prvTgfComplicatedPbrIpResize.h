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
* @file prvTgfComplicatedPbrIpResize.h
*
* @brief Complicated PBR IP Resize
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfComplicatedPbrIpResizeh
#define __prvTgfComplicatedPbrIpResizeh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfComplicatedPbrIpResizeBaseConfigSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfComplicatedPbrIpResizeBaseConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfComplicatedPbrIpResizeRouteConfigSet function
* @endinternal
*
* @brief   Set PBR Configuration
*/
GT_VOID prvTgfComplicatedPbrIpResizeRouteConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfComplicatedPbrIpResizeTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfComplicatedPbrIpResizeTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfComplicatedPbrIpResizeConfigRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfComplicatedPbrIpResizeConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfComplicatedPbrIpResizeh */


