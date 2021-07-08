/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfTunnelTermDualLookup.c
*
* DESCRIPTION:
*       Tunnel: TCAM Check Lookup Order
*
* FILE REVISION NUMBER:
*       $Revision: 2 $
*
*******************************************************************************/

/**
* @internal tgfTunnelTcamCheckLookupOrderBridgeConfigSet function
* @endinternal
*
* @brief   Set bridge configuration
*/
GT_VOID tgfTunnelTcamCheckLookupOrderBridgeConfigSet
(
     GT_VOID
);

/**
* @internal tgfTunnelTcamCheckLookupOrderBuildPacket function
* @endinternal
*
* @brief   Build a packet
*/
GT_VOID tgfTunnelTcamCheckLookupOrderBuildPacket
(
   GT_VOID
);

/**
* @internal tgfTunnelTcamCheckLookupOrderConfigurationSet function
* @endinternal
*
* @brief   Configure TTI rules
*/
GT_VOID tgfTunnelTcamCheckLookupOrderConfigurationSet
(
    GT_U32, GT_U32
);

/**
* @internal tgfTunnelTcamCheckLookupOrderEnableTti1Lookup function
* @endinternal
*
* @brief   Enable 2nd TTI lookup
*/
GT_VOID tgfTunnelTcamCheckLookupOrderEnableTti1Lookup
(
    GT_BOOL
);

/**
* @internal tgfTunnelTcamCheckLookupOrderTrafficOnPort function
* @endinternal
*
* @brief   Check expected traffic
*
* @param[in] capturedPort             - port to check traffic on
*                                       None
*/
GT_VOID tgfTunnelTcamCheckLookupOrderTrafficOnPort
(
    GT_U32 capturedPort
);

/**
* @internal tgfTunnelTcamCheckLookupOrderTrafficGenerate function
* @endinternal
*
* @brief   Generate Traffic
*/
GT_VOID tgfTunnelTcamCheckLookupOrderTrafficGenerate
(
    GT_VOID
);

/**
* @internal tgfTunnelTcamCheckLookupOrderStartTraffic function
* @endinternal
*
* @brief   Start transmitting packets
*/
GT_VOID tgfTunnelTcamCheckLookupOrderStartTraffic
(
    GT_U32, GT_U32
);

/**
* @internal tgfTunnelTcamCheckLookupOrderBasicRuleConfigurationSet function
* @endinternal
*
* @brief   Set basic rule configuration
*/
GT_VOID tgfTunnelTcamCheckLookupOrderBasicRuleConfigurationSet
(
    GT_VOID
);

/**
* @internal tgfTunnelTcamCheckLookupOrderConfigurationRestore function
* @endinternal
*
* @brief   Restore base configuration
*/
GT_VOID tgfTunnelTcamCheckLookupOrderConfigurationRestore
(
    GT_VOID
);

