/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfCutThrough.h
*
* DESCRIPTION:
*       CPSS Cut-Through
*
* FILE REVISION NUMBER:
*       $Revision: 3 $
*
*******************************************************************************/
#ifndef __prvTgfCutThrough
#define __prvTgfCutThrough

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfCutThroughBypassRouterAndPolicerBaseConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Create 2 Vlan entries -- (port[0], port[1]) and (port[2], port[3]);
*         - Enable Cut-Through;
*         - Set MRU value for a VLAN MRU profile;
*         - Create FDB entry.
*/
GT_VOID prvTgfCutThroughBypassRouterAndPolicerBaseConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfCutThroughBypassRouterAndPolicerRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*/
GT_VOID prvTgfCutThroughBypassRouterAndPolicerRouteConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfCutThroughBypassRouterAndPolicerTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         - Send traffic;
*         - Check routing mechanism is working;
*         - Enable bypassing the Router and Ingress Policer engines;
*         - Send traffic;
*         - Check that routing mechanism is ignored.
*/
GT_VOID prvTgfCutThroughBypassRouterAndPolicerTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfCutThroughBypassRouterAndPolicerConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfCutThroughBypassRouterAndPolicerConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfCutThroughBypassRouterConfig function
* @endinternal
*
* @brief   Configure test
*/
GT_VOID prvTgfCutThroughBypassRouterConfig
(
    GT_VOID
);

/**
* @internal prvTgfCutThroughBypassRouterTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfCutThroughBypassRouterTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfCutThroughBypassRouterRestore function
* @endinternal
*
* @brief   Restore old configuration
*/
GT_VOID prvTgfCutThroughBypassRouterRestore
(
    GT_VOID
);

/**
* @internal prvTgfCutThroughBypassPclConfig function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfCutThroughBypassPclConfig
(
    GT_VOID
);

/**
* @internal prvTgfCutThroughBypassPclTrafficGenerate function
* @endinternal
*
* @brief   Generate and check testing traffic
*/
GT_VOID prvTgfCutThroughBypassPclTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfCutThroughBypassPclRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfCutThroughBypassPclRestore
(
    GT_VOID
);

/**
* @internal prvTgfCutThroughBypassMllConfig function
* @endinternal
*
* @brief   Configure test
*/
GT_VOID prvTgfCutThroughBypassMllConfig
(
    GT_VOID
);

/**
* @internal prvTgfCutThroughBypassMllTrafficGenerate function
* @endinternal
*
* @brief   Generate and check testing traffic
*/
GT_VOID prvTgfCutThroughBypassMllTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfCutThroughBypassMllRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfCutThroughBypassMllRestore
(
    GT_VOID
);

/**
* @internal prvTgfCutThroughBypassPolicerConfig function
* @endinternal
*
* @brief   Configure test
*/
GT_VOID prvTgfCutThroughBypassPolicerConfig
(
    GT_VOID
);

/**
* @internal prvTgfCutThroughBypassPolicerTrafficGenerate function
* @endinternal
*
* @brief   Generate and check testing traffic
*/
GT_VOID prvTgfCutThroughBypassPolicerTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfCutThroughBypassPolicerRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfCutThroughBypassPolicerRestore
(
    GT_VOID
);

/**
* @internal prvTgfCutThroughBypassOamConfig function
* @endinternal
*
* @brief   Configure test
*/
GT_VOID prvTgfCutThroughBypassOamConfig
(
    GT_VOID
);

/**
* @internal prvTgfCutThroughBypassOamTrafficGenerate function
* @endinternal
*
* @brief   Generate and check testing traffic
*/
GT_VOID prvTgfCutThroughBypassOamTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfCutThroughBypassOamRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfCutThroughBypassOamRestore
(
    GT_VOID
);

/**
* @internal prvTgfCutThroughByteCountTest function
* @endinternal
*
* @brief   Test calculated byte count
*/
GT_VOID prvTgfCutThroughByteCountTest
(
    GT_VOID
);

/**
* @internal prvTgfCutThroughByteCountIpv4OverMplsTest function
* @endinternal
*
* @brief   Test calculated byte count for IPV4 over MPLS packet
*/
GT_VOID prvTgfCutThroughByteCountIpv4OverMplsTest
(
    IN GT_BOOL transitOrTunnelTerm,
    IN GT_BOOL passengerWithL2
);

/**
* @internal prvTgfCutThroughByteCountUdeCtTerminationTest_defByteCount function
* @endinternal
*
* @brief   Test calculated byte count UDE packet with assigned byte count 0x3FFF
*/
GT_VOID prvTgfCutThroughByteCountUdeCtTerminationTest_defByteCount
(
    GT_VOID
);

/**
* @internal prvTgfCutThroughIpv6CtTerminationTest_EPCL function
* @endinternal
*
* @brief   Test EPCL caused Cut Through termination
*/
GT_VOID prvTgfCutThroughIpv6CtTerminationTest_EPCL
(
    GT_VOID
);

/**
* @internal prvTgfCutThroughIpv4CtTerminationTest_SlowToFast function
* @endinternal
*
* @brief   Test Slow to Fast port caused Cut Through termination
*/
GT_VOID prvTgfCutThroughIpv4CtTerminationTest_SlowToFast
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfCutThrough */


