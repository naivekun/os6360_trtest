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
* @file prvTgfTunnelTermIpv4overIpv4.h
*
* @brief Tunnel Term: Ipv4 over Ipv4 - Basic
*
* @version   19
********************************************************************************
*/
#ifndef __prvTgfTunnelTermIpv4overIpv4h
#define __prvTgfTunnelTermIpv4overIpv4h

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
* @internal prvTgfTunnelTermIpv4overIpv4BaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*
* @param[in] vrfId                    - virtual router index
* @param[in] testNum                  - test number (each test has it's own configuration)
* @param[in] isUc                     - GT_TRUE: use IP unicast addrsses
*                                      GT_FALSE: use IP multicast address
*                                       None
*/
GT_VOID prvTgfTunnelTermIpv4overIpv4BaseConfigurationSet
(
    GT_U32     vrfId,
    GT_U32     testNum,
    GT_BOOL    isUc
);

/**
* @internal prvTgfTunnelTermIpv4overIpv4RouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*/
GT_VOID prvTgfTunnelTermIpv4overIpv4RouteConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermIpv4overIpv4TtiConfigurationSet function
* @endinternal
*
* @brief   Set TTI Configuration
*
* @param[in] testNum                  - test number (each test has it's own configuration)
* @param[in] isUc                     - GT_TRUE: use IP unicast addrsses
*                                      GT_FALSE: use IP multicast address
*                                       None
*/
GT_VOID prvTgfTunnelTermIpv4overIpv4TtiConfigurationSet
(
    IN GT_U32     testNum,
    IN GT_BOOL    isUc
);

/**
* @internal prvTgfTunnelTermIpv4overIpv4OverrideTtiRuleWithIllegalParams function
* @endinternal
*
* @brief   Override TTI rule with bad parameters and verify failures
*/
GT_VOID prvTgfTunnelTermIpv4overIpv4OverrideTtiRuleWithIllegalParams
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
* @internal prvTgfTunnelTermIpv4overIpv4TrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] nextHopePortIndex        - nextHop port index
* @param[in] isUc                     - GT_TRUE: use IP unicast address
*                                      GT_FALSE: use IP multicast address
* @param[in] readCncCounters          - whether to read CNC counters
*                                       None
*/
GT_VOID prvTgfTunnelTermIpv4overIpv4TrafficGenerate
(
    GT_U32  nextHopePortIndex,
    GT_BOOL isUc,
    GT_BOOL readCncCounters
);


/**
* @internal prvTgfTunnelTermIpv4overIpv4TrafficGenerateExpectNoTraffic function
* @endinternal
*
* @brief   Generate traffic - expect no traffic
*
* @param[in] isUc                     - GT_TRUE: use IP unicast address
*                                      GT_FALSE: use IP multicast address
* @param[in] readCncCounters          - whether to read CNC counters
*                                       None
*/
GT_VOID prvTgfTunnelTermIpv4overIpv4TrafficGenerateExpectNoTraffic
(
    GT_BOOL isUc,
    GT_BOOL readCncCounters
);

/**
* @internal prvTgfTunnelTermIpv4overIpv4ConfigurationRestore function
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
GT_VOID prvTgfTunnelTermIpv4overIpv4ConfigurationRestore
(
    GT_BOOL     useUdbConf,
    GT_BOOL     noRouting
);

/**
* @internal prvTgfTunnelTermIpv4overIpv4TtiBasicRule function
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
GT_VOID prvTgfTunnelTermIpv4overIpv4TtiBasicRule
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
* @internal prvTgfTunnelTermIpv4overIpv4MacToMeEnableSet function
* @endinternal
*
* @brief   enable/disable IPv4 TTI lookup only for mac to me packets
*
* @param[in] enable                   - GT_TRUE:  IPv4 TTI lookup only for mac to me packets
*                                      GT_FALSE: disable IPv4 TTI lookup only for mac to me packets
*                                       None
*/
GT_VOID prvTgfTunnelTermIpv4overIpv4MacToMeEnableSet
(
    IN GT_BOOL    enable
);

/**
* @internal prvTgfTunnelTermIpv4overIpv4MacToMeSet function
* @endinternal
*
* @brief   Set a MAC 2 Me entry
*/
GT_VOID prvTgfTunnelTermIpv4overIpv4MacToMeSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermIpv4overIpv4MacToMeDel function
* @endinternal
*
* @brief   Delete a MAC 2 Me entry
*/
GT_VOID prvTgfTunnelTermIpv4overIpv4MacToMeDel
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfTunnelTermIpv4overIpv4h */


