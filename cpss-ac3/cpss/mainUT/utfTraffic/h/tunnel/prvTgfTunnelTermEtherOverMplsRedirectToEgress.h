/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfTunnelTermEtherOverMplsRedirectToEgress.c
*
* DESCRIPTION:
*       Tunnel term Ethernet over Mpls redirect to egress
*
* FILE REVISION NUMBER:
*       $Revision: 10 $
*******************************************************************************/

/**
* @internal prvTgfTunnelTermEtherOverMplsRedirectToEgressBridgeConfigSet function
* @endinternal
*
* @brief   Set Bridge Configuration
*/
GT_VOID prvTgfTunnelTermEtherOverMplsRedirectToEgressBridgeConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermEtherOverMplsRedirectToEgressTtiConfigSet function
* @endinternal
*
* @brief   Set TTI test settings:
*         - Enable port 0 for Eth lookup
*         - Set Eth key lookup MAC mode to Mac DA
*         - Set TTI rule action
*         - Set TTI rule
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_VOID prvTgfTunnelTermEtherOverMplsRedirectToEgressTtiConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermEtherOverMplsRedirectToEgressTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:01,
* @param[in] expectTraffic            - GT_TRUE: traffic should pass
*                                      GT_FALSE: traffic should be dropped
*                                       None
*/
GT_VOID prvTgfTunnelTermEtherOverMplsRedirectToEgressTrafficGenerate
(
    IN GT_BOOL expectTraffic
);


/**
* @internal prvTgfTunnelTermEtherOverMplsRedirectToEgressBuildPacketSet function
* @endinternal
*
* @brief   Build packet
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note Packet description:
*       tunnel:
*       TGF_PACKET_PART_L2_E,
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_ETHERTYPE_E
*       TGF_PACKET_PART_IPV4_E
*       TGF_PACKET_PART_PAYLOAD_E (GRE)
*       passenger:
*       TGF_PACKET_PART_L2_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_PAYLOAD_E
*
*/
GT_VOID prvTgfTunnelTermEtherOverMplsRedirectToEgressBuildPacketSet
(
    GT_VOID
);
/**
* @internal prvTgfTunnelTermEtherOverMplsRedirectToEgressOldKeyTtiConfigSet function
* @endinternal
*
* @brief   Set TTI test settings:
*         - Enable port 0 for Eth lookup
*         - Set Eth key lookup MAC mode to Mac DA
*         - Set TTI rule action
*         - Set TTI rule
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_VOID prvTgfTunnelTermEtherOverMplsRedirectToEgressOldKeyTtiConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermEtherOverMplsRedirectToEgressConfigurationRestore function
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
GT_VOID prvTgfTunnelTermEtherOverMplsRedirectToEgressConfigurationRestore
(
    GT_BOOL     useUdbConf
);


/**
* @internal prvTgfTunnelTermEmulateLegacyMplsForUdbOnlyMplsPatternMaskBuild function
* @endinternal
*
* @brief   build the pattern/mask UDBs that need to be emulated like legacy MPLS pattern/mask.
*
* @param[out] udbPatternArray          - UDBs style pattern
* @param[out] udbMaskArray             - UDBs style mask
*                                       None
*/
GT_STATUS prvTgfTunnelTermEmulateLegacyMplsForUdbOnlyMplsPatternMaskBuild
(
    IN   PRV_TGF_TTI_RULE_UNT   *legacyMplsPatternPtr,
    IN   PRV_TGF_TTI_RULE_UNT   *legacyMplsMaskPtr,
    OUT  PRV_TGF_TTI_RULE_UNT   *udbPatternArray,
    OUT  PRV_TGF_TTI_RULE_UNT   *udbMaskArray
);

/**
* @internal prvTgfTunnelTermEthernetOverMplsTtiBasicRule function
* @endinternal
*
* @brief   build TTI Basic rule
*
* @param[in] pclId                    - pclId
* @param[in] macAddrPtr               - pointer to mac address
* @param[in] vid                      - vlan id
* @param[in] mplsLabel1PartPtr        - pointer to label 1
* @param[in,out] ttiActionPtr             - (pointer to) tti action
* @param[in,out] ttiPatternPtr            - (pointer to) tti pattern
* @param[in,out] ttiMaskPtr               - (pointer to) tti mask
* @param[in,out] ttiActionPtr             - (pointer to) tti action
* @param[in,out] ttiPatternPtr            - (pointer to) tti pattern
* @param[in,out] ttiMaskPtr               - (pointer to) tti mask
*                                       None
*/
GT_VOID prvTgfTunnelTermEthernetOverMplsTtiBasicRule
(
    IN GT_U8                       pclId,
    IN TGF_MAC_ADDR                 *macAddrPtr,
    IN GT_U16                       vid,
    IN TGF_PACKET_MPLS_STC          *mplsLabel1PartPtr,
    INOUT PRV_TGF_TTI_ACTION_STC   *ttiActionPtr,
    INOUT PRV_TGF_TTI_RULE_UNT     *ttiPatternPtr,
    INOUT PRV_TGF_TTI_RULE_UNT     *ttiMaskPtr
);

/**
* @internal prvTgfTunnelTermEmulateLegacyMplsForUdbOnlyMplsKeyNeededUdbsSet function
* @endinternal
*
* @brief   set the needed UDBs that need to be emulated like legacy MPLS key.
*
* @param[in] keyType                  - UDB only key type, that need to be emulated like legacy MPLS key.
*                                       None
*/
GT_VOID prvTgfTunnelTermEmulateLegacyMplsForUdbOnlyMplsKeyNeededUdbsSet
(
    PRV_TGF_TTI_KEY_TYPE_ENT     keyType
);
/**
* @internal prvTgfTunnelTermEtherOverMplsRedirectToEgressConfiguredUdbConfigSet function
* @endinternal
*
* @brief   Set TTI test settings:
*         - Enable port 0 for Eth lookup
*         - Set TTI rule action
*         - Set TTI rule
*         - Set Udb
*/
GT_VOID prvTgfTunnelTermEtherOverMplsRedirectToEgressConfiguredUdbConfigSet
(
    GT_VOID
);
/**
* @internal prvTgfTunnelTermEtherOverMplsRedirectToEgressConfiguredUdbConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 1. Restore TTI Configuration
*       2. Restore Base Configuration
*
*/
GT_VOID prvTgfTunnelTermEtherOverMplsRedirectToEgressConfiguredUdbConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermEthernetOverMplsMacToMeEnableSet function
* @endinternal
*
* @brief   enable/disable MPLS TTI lookup only for mac to me packets
*
* @param[in] enable                   - GT_TRUE:  MPLS TTI lookup only for mac to me packets
*                                      GT_FALSE: disable MPLS TTI lookup only for mac to me packets
*                                       None
*/
GT_VOID prvTgfTunnelTermEthernetOverMplsMacToMeEnableSet
(
    IN GT_BOOL    enable
);

/**
* @internal prvTgfTunnelTermEthernetOverMplsMacToMeSet function
* @endinternal
*
* @brief   Set a MAC 2 Me entry
*/
GT_VOID prvTgfTunnelTermEthernetOverMplsMacToMeSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermEthernetOverMplsMacToMeDel function
* @endinternal
*
* @brief   Delete a MAC 2 Me entry
*/
GT_VOID prvTgfTunnelTermEthernetOverMplsMacToMeDel
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermEtherOverMplsRedirectToEgressDsaUse function
* @endinternal
*
* @brief   state that the test should use ingress DSA tag (ingress port is cascade port)
*
* @param[in] ingressIsCascadePort
*
* @note save info to be used for :
*       1. configure ingress port as cascade port
*       2. add DSA when send packet into ingress port
*       3. remove the 'cascade' from port when 'restore'
*
*/
GT_VOID prvTgfTunnelTermEtherOverMplsRedirectToEgressDsaUse
(
    IN GT_BOOL     ingressIsCascadePort
);

/**
* @internal prvTgfTunnelTermEtherOverMplsRedirectToEgressNumOfMplsLabels function
* @endinternal
*
* @brief   state the number of MPLS labels in the tunnel header
*
* @param[in] numOfLabels
*/
GT_VOID prvTgfTunnelTermEtherOverMplsRedirectToEgressNumOfMplsLabels
(
    IN GT_U32     numOfLabels
);

/**
* @internal prvTgfTunnelTermEtherOverMplsRedirectToEgressUdbKeyTtiConfigSet function
* @endinternal
*
* @brief   Set TTI test settings:
*         - Enable port 0 for Eth lookup
*         - Set TTI rule action
*         - Set TTI rule
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_VOID prvTgfTunnelTermEtherOverMplsRedirectToEgressUdbKeyTtiConfigSet
(
    GT_VOID
);

