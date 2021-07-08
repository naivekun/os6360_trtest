/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* tgfMpls.h
*
* DESCRIPTION:
*       Generic API for MPLS
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/
#ifndef __prvTgfMplsh
#define __prvTgfMplsh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef EXMXPM_FAMILY
    #include <cpssCommon/cpssPresteraDefs.h>
    #include <cpss/extServices/private/prvCpssBindFunc.h>

    #include <utf/utfMain.h>
    #include <utf/private/prvUtfHelpers.h>

    #include <trafficEngine/tgfTrafficGenerator.h>
    #include <trafficEngine/tgfTrafficTable.h>

    #include <common/tgfMplsGen.h>
#endif /* EXMXPM_FAMILY */


/**
* @internal prvTgfMplsBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfMplsBaseConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfMplsRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*/
GT_VOID prvTgfMplsRouteConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfMplsConfigurationSet function
* @endinternal
*
* @brief   Set MPLS Configuration
*/
GT_VOID prvTgfMplsConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfMplsNhlfeBasicConfigurationSet function
* @endinternal
*
* @brief   Set NHLFE Configuration
*/
GT_VOID prvTgfMplsNhlfeBasicConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfMplsNhlfeSwapConfigurationSet function
* @endinternal
*
* @brief   Set NHLFE SWAP Configuration
*/
GT_VOID prvTgfMplsNhlfeSwapConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfMplsNhlfePushConfigurationSet function
* @endinternal
*
* @brief   Set NHLFE PUSH Configuration
*/
GT_VOID prvTgfMplsNhlfePushConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfMplsNhlfePhpConfigurationSet function
* @endinternal
*
* @brief   Set NHLFE PUSH Configuration
*/
GT_VOID prvTgfMplsNhlfePhpConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfMplsBasicTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfMplsBasicTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfMplsSwapTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic - Swap Label
*/
GT_VOID prvTgfMplsSwapTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfMplsPushTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic - Push Label
*/
GT_VOID prvTgfMplsPushTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfMplsPhpTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic - Push Label
*/
GT_VOID prvTgfMplsPhpTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfMplsConfigurationRestore function
* @endinternal
*
* @brief   None
*
* @retval GT_OK                    - on success
*/
GT_VOID prvTgfMplsConfigurationRestore
(
    GT_VOID
);

#ifdef __cplusplus
#endif /* __cplusplus */

#endif /* __prvTgfMplsh */

