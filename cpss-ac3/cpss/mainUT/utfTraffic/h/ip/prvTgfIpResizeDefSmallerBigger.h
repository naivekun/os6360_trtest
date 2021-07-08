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
* @file prvTgfIpResizeDefSmallerBigger.h
*
* @brief Complicated IP Resize:
* - from Default --> Smaller --> Bigger size
* - from Default --> Smaller --> Bigger --> Smaller size
* - from Default --> Bigger --> Smaller --> Smaller size
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfIpResizeDefSmallerBiggerh
#define __prvTgfIpResizeDefSmallerBiggerh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfIpResizeDefSmallerBiggerBaseConfigSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfIpResizeDefSmallerBiggerBaseConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfIpResizeDefSmallerBiggerRouteConfigSet function
* @endinternal
*
* @brief   Set PBR Configuration
*
* @param[in] testStageNum             - test stage
*                                       None
*/
GT_VOID prvTgfIpResizeDefSmallerBiggerRouteConfigSet
(
    IN GT_U32       testStageNum
);

/**
* @internal prvTgfIpResizeDefSmallerBiggerTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] testStageNum             - test stage
*                                       None
*/
GT_VOID prvTgfIpResizeDefSmallerBiggerTrafficGenerate
(
    IN GT_U32       testStageNum
);

/**
* @internal prvTgfIpResizeDefSmallerBiggerConfigRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfIpResizeDefSmallerBiggerConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfIpResizeDefSmallerBiggerh */


