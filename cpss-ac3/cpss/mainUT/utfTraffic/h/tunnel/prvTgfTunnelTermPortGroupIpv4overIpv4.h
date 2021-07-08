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
* @file prvTgfTunnelTermPortGroupIpv4overIpv4.h
*
* @brief Tunnel Term: Ipv4 over Ipv4 Port Group - Basic
*
* @version   7
********************************************************************************
*/
#ifndef __prvTgfTunnelTermPortGroupIpv4overIpv4h
#define __prvTgfTunnelTermPortGroupIpv4overIpv4h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* macro to build 'global port' number from local port + port group Id */
#define PRV_TGF_SEND_PORT_GROUP_X_PORT_NUM_MAC(portGroup,localPort) ((portGroup << 4) | localPort)

/* port number to send traffic from */
extern GT_U32 prvTgfSendPort;
/**
* @internal prvTgfTunnelTermPortGroupIpv4overIpv4BaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfTunnelTermPortGroupIpv4overIpv4BaseConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermPortGroupIpv4overIpv4RouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
*                                       None
*/
GT_VOID prvTgfTunnelTermPortGroupIpv4overIpv4RouteConfigurationSet
(
    IN GT_U32      prvUtfVrfId
);

/**
* @internal prvTgfTunnelTermPortGroupIpv4overIpv4TtiConfigurationSet function
* @endinternal
*
* @brief   Set TTI Configuration
*
* @param[in] prvUtfVrfId              - virtual router index,
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                       None
*/
GT_VOID prvTgfTunnelTermPortGroupIpv4overIpv4TtiConfigurationSet
(
    IN  GT_U32                 prvUtfVrfId,
    IN  GT_PORT_GROUPS_BMP     portGroupsBmp
);

/**
* @internal prvTgfTunnelTermPortGroupRuleValidStatusSet function
* @endinternal
*
* @brief   Set TTI Rule Valid Status
*/
GT_VOID prvTgfTunnelTermPortGroupRuleValidStatusSet
(
    GT_BOOL   validStatus
);

/**
* @internal prvTgfTunnelTermPortGroupIpv4overIpv4TrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfTunnelTermPortGroupIpv4overIpv4TrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermPortGroupIpv4overIpv4TrafficGenerateExpectNoTraffic function
* @endinternal
*
* @brief   Generate traffic - expect no traffic
*/
GT_VOID prvTgfTunnelTermPortGroupIpv4overIpv4TrafficGenerateExpectNoTraffic
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermPortGroupIpv4overIpv4ConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @param[in] prvUtfVrfId              - virtual router index
*                                       None
*
* @note 3. Restore TTI Configuration
*       2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfTunnelTermPortGroupIpv4overIpv4ConfigurationRestore
(
    IN GT_U32      prvUtfVrfId
);

/**
* @internal prvTgfTunnelTermPortGroupIpv4overIpv4PortConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration per port
*
* @param[in] sendPort                 - send port
*                                       None
*/
GT_VOID prvTgfTunnelTermPortGroupIpv4overIpv4PortConfigurationRestore
(
    GT_U32      sendPort
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfTunnelTermPortGroupIpv4overIpv4h */


