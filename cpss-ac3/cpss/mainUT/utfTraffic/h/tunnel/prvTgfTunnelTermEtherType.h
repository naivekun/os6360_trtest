/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *2
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file prvTgfTunnelTermEtherType.h
*
* @brief Tunnel Term: Ethertype- Basic
*
* @version   4
********************************************************************************
*/

#ifndef __prvTgfTunnelTermEtherTypeh
#define __prvTgfTunnelTermEtherTypeh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfTunnelTermEtherTypeBaseConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set index of the rule in the TCAM to 0
*         - set macMode PRV_TGF_TTI_MAC_MODE_DA_E
*         - set MAC mask as disable
*/
GT_VOID prvTgfTunnelTermEtherTypeBaseConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermEtherTypeVrfIdAssignConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set index of the rule in the TCAM to 1
*         - set macMode PRV_TGF_TTI_MAC_MODE_DA_E
*         - set MAC mask as disable
*         - set TTI rule action - change VRF ID
* @param[in] sendPortNum              - port number to send packet
* @param[in] vrfId                    - VRF ID to set
* @param[in] l2DataPtr                - L2 data of packet
*                                       None
*/
GT_VOID prvTgfTunnelTermEtherTypeVrfIdAssignConfigurationSet
(
    IN  GT_U32                         sendPortNum,
    IN  GT_U32                        vrfId,
    IN  TGF_PACKET_L2_STC            *l2DataPtr
);

/**
* @internal prvTgfTunnelTermEtherTypeRuleValidStatusSet function
* @endinternal
*
* @brief   Set TTI Rule Valid Status
*/
GT_VOID prvTgfTunnelTermEtherTypeRuleValidStatusSet
(
    GT_BOOL   validStatus
);

/**
* @internal prvTgfTunnelTermEtherTypeTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfTunnelTermEtherTypeTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermEtherTypeDifferentTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfTunnelTermEtherTypeDifferentTrafficGenerate
(
    GT_VOID
);


/**
* @internal prvTgfTunnelTermEtherTypeTrafficGenerateExpectNoTraffic function
* @endinternal
*
* @brief   Generate traffic - Expect No Traffic
*/
GT_VOID prvTgfTunnelTermEtherTypeTrafficGenerateExpectNoTraffic
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermEtherTypeConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfTunnelTermEtherTypeConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermEtherTypeVrfIdAssignConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*
* @param[in] sendPortNum              - port number to send packets
*                                       None
*/
GT_VOID prvTgfTunnelTermEtherTypeVrfIdAssignConfigRestore
(
    IN GT_U32  sendPortNum
);

/**
* @internal prvTgfTunnelTermEtherTypeOtherTestInit function
* @endinternal
*
* @brief   Set TTI test settings:
*         - Enable port 0 for Eth lookup
*         - Set Eth key lookup MAC mode to Mac DA
*         - Set TTI rule action
*         - Set TTI rule
* @param[in] sendPortNum              - port number to send packet
* @param[in] vrfId                    - assign VRF ID for packet
*                                      GT_TRUE  - assign VRF ID for packet
*                                      GT_FALSE - not assign VRF ID for packet
* @param[in] vrfId                    - VRF ID to set
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note Used 4 first bytes of anchor 'L3 offset - 2'.
*
*/
GT_VOID prvTgfTunnelTermEtherTypeOtherTestInit
(
    IN  GT_U32                         sendPortNum,
    IN  GT_BOOL                       vrfIdAssign,
    IN  GT_U32                        vrfId
);

/**
* @internal prvTgfTunnelTermEtherTypeOtherBaseConfigurationSet function
* @endinternal
*
* @brief   Test TTI for LLC Non SNAP packets.
*
* @note Used 4 first bytes of anchor 'L3 offset - 2'.
*
*/
GT_VOID prvTgfTunnelTermEtherTypeOtherBaseConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermEtherTypeOtherConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfTunnelTermEtherTypeOtherConfigRestore
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermEtherTypeOtherTrafficGenerateExpectNoTraffic function
* @endinternal
*
* @brief   Generate traffic - Expect No Traffic
*/
GT_VOID prvTgfTunnelTermEtherTypeOtherTrafficGenerateExpectNoTraffic
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvTgfTunnelTermEtherTypeh */

