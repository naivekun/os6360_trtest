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
* @file prvTgfTunnelTermTunnelHeaderStart.h
*
* @brief Tunnel termination header start offset test
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfTunnelTermTunnelHeaderStarth
#define __prvTgfTunnelTermTunnelHeaderStarth

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfTunnelGen.h>

/* nextHop port number to receive traffic from */
#define PRV_TGF_NEXTHOPE_PORT_IDX_CNS     3
#define PRV_TGF_NEXTHOPE1_PORT_IDX_CNS    2
#define PRV_TGF_NEXTHOPE2_PORT_IDX_CNS    1

/**
* @internal prvTgfTunnelTermTunnelHeaderStartBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*
* @param[in] vrfId                    - virtual router index
*
*/
GT_VOID prvTgfTunnelTermTunnelHeaderStartBaseConfigurationSet
(
    GT_U32     vrfId
);

/**
* @internal prvTgfTunnelTermTunnelHeaderStartRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*/
GT_VOID prvTgfTunnelTermTunnelHeaderStartRouteConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermTunnelHeaderStartTtiConfigurationSet function
* @endinternal
*
* @brief   Set TTI Configuration
*
* @param[in] isL4Offset - tunnelHeaderStartL4Enable value in TTI action
*
*/
GT_VOID prvTgfTunnelTermTunnelHeaderStartTtiConfigurationSet
(
    IN GT_BOOL  isL4Offset
);

/**
* @internal prvTgfTunnelTermTunnelHeaderStartTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] nextHopePortIndex        - nextHop port index
*
*/
GT_VOID prvTgfTunnelTermTunnelHeaderStartTrafficGenerate
(
    GT_U32  nextHopePortIndex
);

/**
* @internal prvTgfTunnelTermTunnelHeaderStartConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @param[in] useUdbConf               - tti rule is udb
* @param[in] noRouting                - no routing in the test
*                                       None
*
* @note 3. Restore TTI Configuration
*       2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfTunnelTermTunnelHeaderStartConfigurationRestore
(
    GT_BOOL     useUdbConf,
    GT_BOOL     noRouting
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfTunnelTermTunnelHeaderStarth */
