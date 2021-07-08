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
* @file prvTgfTunnelTermIpv4overGreIpv4.h
*
* @brief Tunnel Term: Ipv4 over GRE Ipv4 - Basic with enabling/disabling GRE
* extensions
*
* @version   2
********************************************************************************
*/
#ifndef __prvTgfTunnelTermIpv4overGreIpv4h
#define __prvTgfTunnelTermIpv4overGreIpv4h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfTunnelGen.h>

/**
* @internal prvTgfTunnelTermIpv4overGreIpv4BaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*
* @param[in] vrfId                    - virtual router index
* @param[in] testNum                  - test number (each test has it's own configuration)
*                                       None
*/
GT_VOID prvTgfTunnelTermIpv4overGreIpv4BaseConfigurationSet
(
    GT_U32     vrfId,
    GT_U32     testNum
);

/**
* @internal prvTgfTunnelTermIpv4overGreIpv4RouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*/
GT_VOID prvTgfTunnelTermIpv4overGreIpv4RouteConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermIpv4overGreIpv4TtiConfigurationSet function
* @endinternal
*
* @brief   Set TTI Configuration
*
* @param[in] testNum                  - test number (each test has it's own configuration)
*                                       None
*/
GT_VOID prvTgfTunnelTermIpv4overGreIpv4TtiConfigurationSet
(
    GT_U32     testNum
);

/**
* @internal prvTgfTunnelTermIpv4overGreIpv4TrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfTunnelTermIpv4overGreIpv4TrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermIpv4overGreIpv4TrafficGenerateExpectNoTraffic function
* @endinternal
*
* @brief   Generate traffic - expect no traffic
*/
GT_VOID prvTgfTunnelTermIpv4overGreIpv4TrafficGenerateExpectNoTraffic
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermIpv4overGreIpv4ConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @param[in] useUdbConf               - tti rule is udb
*                                       None
*
* @note 3. Restore TTI Configuration
*       2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfTunnelTermIpv4overGreIpv4ConfigurationRestore
(
    GT_BOOL     useUdbConf
);

/**
* @internal prvTgfTunnelTermIpv4overGreIpv4TtiBasicRule function
* @endinternal
*
* @brief   build TTI Basic rule
*
* @param[in] pclId                    - pclId
* @param[in] macAddrPtr               - pointer to mac address
* @param[in] sipPtr                   - pointer to SIP
* @param[in] dipPtr                   - pointer to DIP
* @param[in] vid                      - vlan id
* @param[in,out] ttiPatternPtr            - (pointer to) tti pattern
* @param[in,out] ttiMaskPtr               - (pointer to) tti mask
* @param[in,out] ttiPatternPtr            - (pointer to) tti pattern
* @param[in,out] ttiMaskPtr               - (pointer to) tti mask
*                                       None
*/
GT_VOID prvTgfTunnelTermIpv4overGreIpv4TtiBasicRule
(
    IN GT_U32                       pclId,
    IN TGF_MAC_ADDR                 *macAddrPtr,
    IN TGF_IPV4_ADDR                *sipPtr,
    IN TGF_IPV4_ADDR                *dipPtr,
    IN GT_U16                       vid,
    INOUT PRV_TGF_TTI_RULE_UNT     *ttiPatternPtr,
    INOUT PRV_TGF_TTI_RULE_UNT     *ttiMaskPtr
);

/**
* @internal prvTgfTunnelTermIpv4overGreIpv4GreExtensionsSet function
* @endinternal
*
* @brief   Enable/Disable GRE extensions and set the exception command.
*
* @param[in] enable                   - enable/disable the GRE extensions
* @param[in] exceptionCmd             - the exception command
*                                       None
*/
GT_VOID prvTgfTunnelTermIpv4overGreIpv4GreExtensionsSet
(
    IN GT_BOOL                 enable,
    IN CPSS_PACKET_CMD_ENT     exceptionCmd
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfTunnelTermIpv4overGreIpv4h */


