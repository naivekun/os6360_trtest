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
* @file prvTgfTunnelTermGlobalPort2LocalPort.h
*
* @brief Tunnel Term: Global Port 2 Local Port - Basic
*
* @version   5
********************************************************************************
*/
#ifndef __prvTgfTunnelTermGlobalPort2LocalPorth
#define __prvTgfTunnelTermGlobalPort2LocalPorth

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* local port 2 in port groups */
extern GT_U32 prvTgfTtiMultiPortGroupLocalPort2Array[CPSS_MAX_PORT_GROUPS_CNS];
extern GT_U32 prvTgfTtiMultiPortGroupLocalPort2_XLG_Array[CPSS_MAX_PORT_GROUPS_CNS];

/* nextHop port number to receive traffic from */
#define PRV_TGF_NEXTHOPE_PORT_NUM_CNS     32   /*0x16*/
/**
* @internal prvTgfTunnelTermGlobalPort2LocalPortBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfTunnelTermGlobalPort2LocalPortBaseConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermGlobalPort2LocalPortRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*/
GT_VOID prvTgfTunnelTermGlobalPort2LocalPortRouteConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermGlobalPort2LocalPortTtiConfigurationSet function
* @endinternal
*
* @brief   Set TTI Configuration
*/
GT_VOID prvTgfTunnelTermGlobalPort2LocalPortTtiConfigurationSet
(
    GT_U32      portTestNum,
    GT_U32      portConfigSet
);

/**
* @internal prvTgfTunnelTermGlobalPort2LocalPortTtiConfiguration2Set function
* @endinternal
*
* @brief   Set TTI Configuration
*
* @note sending Basic IPv4 Tunnel Termination - to Specific Port Group 1, local port 3
*
*/
GT_VOID prvTgfTunnelTermGlobalPort2LocalPortTtiConfiguration2Set
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermGlobalPort2LocalPortTtiConfiguration3Set function
* @endinternal
*
* @brief   Set TTI Configuration
*
* @note sending Basic IPv4 Tunnel Termination - Port Groups 0,2 , local port 3
*
*/
GT_VOID prvTgfTunnelTermGlobalPort2LocalPortTtiConfiguration3Set
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermGlobalPort2LocalPortRuleValidStatusSet function
* @endinternal
*
* @brief   Set TTI Rule Valid Status
*/
GT_VOID prvTgfTunnelTermGlobalPort2LocalPortRuleValidStatusSet
(
    GT_BOOL   validStatus
);

/**
* @internal prvTgfTunnelTermGlobalPort2LocalPortTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfTunnelTermGlobalPort2LocalPortTrafficGenerate
(
    GT_U32 testNum,
    GT_U32 iteration,
    GT_U32 transmission
);

/**
* @internal prvTgfTunnelTermGlobalPort2LocalPortConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 3. Restore TTI Configuration
*       2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfTunnelTermGlobalPort2LocalPortConfigurationRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfTunnelTermGlobalPort2LocalPorth */


