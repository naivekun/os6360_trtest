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
* @file prvTgfTunnelpv4Ecn.h
*
* @brief Tunnel Term: Ipv4 over GRE Ipv4 - Basic with enabling/disabling GRE
* extensions
*
* @version   2
********************************************************************************
*/
#ifndef __prvTgfTunnelpv4Ecnh
#define __prvTgfTunnelpv4Ecnh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfTunnelGen.h>

/* block number */
#define PRV_TGF_CNC0_BLOCK_NUM_CNS        1
#define PRV_TGF_CNC1_BLOCK_NUM_CNS        2

/* counter number */
#define PRV_TGF_CNC0_COUNTER_NUM_CNS      10
#define PRV_TGF_CNC1_COUNTER_NUM_CNS      22

/* nextHop port number to receive traffic from */
#define PRV_TGF_NEXTHOPE_PORT_IDX_CNS     3
#define PRV_TGF_NEXTHOPE1_PORT_IDX_CNS    2
#define PRV_TGF_NEXTHOPE2_PORT_IDX_CNS    1

/**
* @internal prvTgfTunnelIpv4EcnBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*
* @param[in] vrfId                    - virtual router index
* @param[in] testNum                  - test number (each test has it's own configuration)
*/
GT_VOID prvTgfTunnelIpv4EcnBaseConfigurationSet
(
    GT_U32     vrfId,
    GT_U32     testNum
);

/**
* @internal prvTgfTunnelIpv4EcnRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*/
GT_VOID prvTgfTunnelIpv4EcnRouteConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelIpv4EcnTtiConfigurationSet function
* @endinternal
*
* @brief   Set TTI Configuration
*
* @param[in] testNum                  - test number (each test has it's own configuration)
*/
GT_VOID prvTgfTunnelIpv4EcnTtiConfigurationSet
(
    IN GT_U32     testNum
);

/**
* @internal prvTgfTunnelIpv4EcnOverrideTtiRuleWithIllegalParams function
* @endinternal
*
* @brief   Override TTI rule with bad parameters and verify failures
*/
GT_VOID prvTgfTunnelIpv4EcnOverrideTtiRuleWithIllegalParams
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermRuleValidStatusSet function
* @endinternal
*
* @brief   Set TTI Rule Valid Status
*/
GT_VOID prvTgfTunnelTermRuleValidStatusSet
(
    GT_BOOL   validStatus
);

/**
* @internal prvTgfTunnelIpv4EcnTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] ecnMode        - ECN mode
* @param[in] outerEcn       - ECN of outer packet
* @param[in] passengerEcn   - ECN of passenger packet
*
*/
GT_VOID prvTgfTunnelIpv4EcnTrafficGenerate
(
    PRV_TGF_TUNNEL_START_ECN_MODE_ENT    ecnMode,
    GT_U8                                outerEcn,
    GT_U8                                passengerEcn
);

/**
* @internal prvTgfTunnelIpv4EcnTest function
* @endinternal
*
* @brief   Test ECN mode
*
* @param[in] ecnMode    - ECN mode
*
*/
GT_VOID prvTgfTunnelIpv4EcnTest
(
    PRV_TGF_TUNNEL_START_ECN_MODE_ENT    ecnMode
);

/**
* @internal prvTgfTunnelIpv4EcnConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 3. Restore TTI Configuration
*       2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfTunnelIpv4EcnConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfTunnelIpv4EcnTtiBasicRule function
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
GT_VOID prvTgfTunnelIpv4EcnTtiBasicRule
(
    IN GT_U32                       pclId,
    IN TGF_MAC_ADDR                 *macAddrPtr,
    IN TGF_IPV4_ADDR                *sipPtr,
    IN TGF_IPV4_ADDR                *dipPtr,
    IN GT_U16                       vid,
    INOUT PRV_TGF_TTI_RULE_UNT      *ttiPatternPtr,
    INOUT PRV_TGF_TTI_RULE_UNT      *ttiMaskPtr
);

/**
* @internal prvTgfTunnelTermEmulateLegacyIpv4ForUdbOnlyIpv4KeyNeededUdbsSet function
* @endinternal
*
* @brief   set the needed UDBs that need to be emulated like legacy IPv4 key.
*
* @param[in] keyType                  - UDB only key type, that need to be emulated like legacy IPv4 key.
*                                       None
*/
GT_VOID prvTgfTunnelTermEmulateLegacyIpv4ForUdbOnlyIpv4KeyNeededUdbsSet
(
    PRV_TGF_TTI_KEY_TYPE_ENT     keyType
);

/**
* @internal prvTgfTunnelTermEmulateLegacyIpv4ForUdbOnlyIpv4PatternMaskBuild function
* @endinternal
*
* @brief   build the pattern/mask UDBs that need to be emulated like legacy IPv4 pattern/mask.
*
* @param[in] legacyIpv4PatternPtr     - legacy ipv4 style pattern
* @param[in] legacyIpv4MaskPtr        - legacy ipv4 style mask
*
* @param[out] udbPatternArray          - UDBs style pattern
* @param[out] udbMaskArray             - UDBs style mask
*                                       None
*/
GT_STATUS prvTgfTunnelTermEmulateLegacyIpv4ForUdbOnlyIpv4PatternMaskBuild
(
    IN   PRV_TGF_TTI_RULE_UNT   *legacyIpv4PatternPtr,
    IN   PRV_TGF_TTI_RULE_UNT   *legacyIpv4MaskPtr,
    OUT  PRV_TGF_TTI_RULE_UNT   *udbPatternArray,
    OUT  PRV_TGF_TTI_RULE_UNT   *udbMaskArray
);

/**
* @internal prvTgfTunnelIpv4EcnMacToMeEnableSet function
* @endinternal
*
* @brief   enable/disable IPv4 TTI lookup only for mac to me packets
*
* @param[in] enable                   - GT_TRUE:  IPv4 TTI lookup only for mac to me packets
*                                      GT_FALSE: disable IPv4 TTI lookup only for mac to me packets
*                                       None
*/
GT_VOID prvTgfTunnelIpv4EcnMacToMeEnableSet
(
    IN GT_BOOL    enable
);

/**
* @internal prvTgfTunnelIpv4EcnMacToMeSet function
* @endinternal
*
* @brief   Set a MAC 2 Me entry
*/
GT_VOID prvTgfTunnelIpv4EcnMacToMeSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelIpv4EcnMacToMeDel function
* @endinternal
*
* @brief   Delete a MAC 2 Me entry
*/
GT_VOID prvTgfTunnelIpv4EcnMacToMeDel
(
    GT_VOID
);
/**
* @internal prvTgfTunnelIpv4EcnIpv4TunnelConfigurationSet function
* @endinternal
*
* @brief   Set Tunnel Configuration
*/
GT_VOID prvTgfTunnelIpv4EcnIpv4TunnelConfigurationSet
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfTunnelpv4Ecnh */
