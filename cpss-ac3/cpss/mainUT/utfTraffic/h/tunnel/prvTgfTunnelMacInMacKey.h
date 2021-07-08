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
* @file prvTgfTunnelMacInMacKey.h
*
* @brief Verify the functionality of MIM TTI key MAC DA or MAC SA
*
* @version   6
********************************************************************************
*/
#ifndef __prvTgfTunnelMacInMacKeyh
#define __prvTgfTunnelMacInMacKeyh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfTunnelTermUseMacSaInMimTtiLookupConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set index of the rule in the TCAM to 0
*         - set macMode PRV_TGF_TTI_MAC_MODE_SA_E
*         - set MAC mask as disable
*/
GT_VOID prvTgfTunnelTermUseMacSaInMimTtiLookupConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermUseMacSaInMimTtiLookupTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 MIM tunneled packet:
*         macDa = 00:00:00:00:00:22,
*         macSa = 00:00:00:00:34:02,
*         Tagged: true (with VLAN tag 5)
*         Ether Type: 0x8988 (MIM Ether type)
*         iSid: 0x123456
*         iUP: 0x5
*         iDP: 0
*         Success Criteria:
*         Packet striped and forwarded to port 23 VLAN 6
*/
GT_VOID prvTgfTunnelTermUseMacSaInMimTtiLookupTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermUseMacSaInMimTtiLookupConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfTunnelTermUseMacSaInMimTtiLookupConfigRestore
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermMaskMacInMimTtiLookupConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set index of the rule in the TCAM to 0
*         - set macMode PRV_TGF_TTI_MAC_MODE_DA_E
*         - set MAC mask as enable
*/
GT_VOID prvTgfTunnelTermMaskMacInMimTtiLookupConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermMaskMacInMimTtiLookupTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 MIM tunneled packet:
*         macDa = 00:00:00:00:00:22,
*         macSa = 00:00:00:00:34:05,
*         Tagged: true (with VLAN tag 5)
*         Ether Type: 0x8988 (MIM Ether type)
*         iSid: 0x123456
*         iUP: 0x5
*         iDP: 0
*         Success Criteria:
*         Packet striped and forwarded to port 23 VLAN 6
*/
GT_VOID prvTgfTunnelTermMaskMacInMimTtiLookupTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermMaskMacInMimTtiLookupConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfTunnelTermMaskMacInMimTtiLookupConfigRestore
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermMimBasicConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set index of the rule in the TCAM to 0
*         - set macMode PRV_TGF_TTI_MAC_MODE_DA_E
*         - set MAC mask as disable
*/
GT_VOID prvTgfTunnelTermMimBasicConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermMimInlifConfigurationSet function
* @endinternal
*
* @brief   Set inlif Configuration
*/
GT_VOID prvTgfTunnelTermMimInlifConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermMimBasicTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 MIM tunneled packet:
*         macDa = 00:00:00:00:00:22,
*         macSa = 00:00:00:00:34:02,
*         Tagged: true (with VLAN tag 5)
*         Ether Type: 0x8988 (MIM Ether type)
*         iSid: 0x123456
*         iUP: 0x5
*         iDP: 0
*         Success Criteria:
*         Packet striped and forwarded to port 23 VLAN 6
* @param[in] expectTraffic            - whether to expect traffic on the egress port
*                                       None
*/
GT_VOID prvTgfTunnelTermMimBasicTrafficGenerate
(
    GT_BOOL   expectTraffic
);

/**
* @internal prvTgfTunnelTermBasicMimConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfTunnelTermBasicMimConfigRestore
(
    GT_VOID
);

/**
* @internal tgfTunnelTermMimPassengerOuterTag0or1Test function
* @endinternal
*
* @brief   Test MIM tunnel termination - passenger tag recognition - tag0 or tag1
*         according to test design
*         http://docil.marvell.com/webtop/drl/objectId/0900dd88801337dd
*         in Cabinets/SWITCHING/CPSS and PSS/R&D Internal/CPSS/DX/Test Design/Lion3
*/
GT_VOID tgfTunnelTermMimPassengerOuterTag0or1Test
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermMimMacToMeEnableSet function
* @endinternal
*
* @brief   enable/disable Mac in Mac TTI lookup only for mac to me packets
*
* @param[in] enable                   - GT_TRUE:  Mac in Mac TTI lookup only for mac to me packets
*                                      GT_FALSE: disable Mac in Mac TTI lookup only for mac to me packets
*                                       None
*/
GT_VOID prvTgfTunnelTermMimMacToMeEnableSet
(
    IN GT_BOOL    enable
);

/**
* @internal prvTgfTunnelTermMimMacToMeSet function
* @endinternal
*
* @brief   Set a MAC 2 Me entry
*/
GT_VOID prvTgfTunnelTermMimMacToMeSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermMimMacToMeDel function
* @endinternal
*
* @brief   Delete a MAC 2 Me entry
*/
GT_VOID prvTgfTunnelTermMimMacToMeDel
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfTunnelMacInMacKeyh */

/**
* @internal prvTgfTunnelTermEmulateLegacyMimForUdbOnlyMimPatternMaskBuild function
* @endinternal
*
* @brief   build the pattern/mask UDBs that need to be emulated like legacy IPv4 pattern/mask.
*
* @param[out] udbPatternArray          - UDBs style pattern
* @param[out] udbMaskArray             - UDBs style mask
*                                       None
*/
GT_STATUS prvTgfTunnelTermEmulateLegacyMimForUdbOnlyMimPatternMaskBuild
(
    IN   PRV_TGF_TTI_RULE_UNT   *legacyMimPatternPtr,
    IN   PRV_TGF_TTI_RULE_UNT   *legacyMimMaskPtr,
    OUT  PRV_TGF_TTI_RULE_UNT   *udbPatternArray,
    OUT  PRV_TGF_TTI_RULE_UNT   *udbMaskArray
);

/**
* @internal prvTgfTunnelTermEmulateLegacyMimForUdbOnlyMimKeyNeededUdbsSet function
* @endinternal
*
* @brief   set the needed UDBs that need to be emulated like legacy MiM key.
*
* @param[in] keyType                  - UDB only key type, that need to be emulated like legacy MiM key.
*                                       None
*/
GT_VOID prvTgfTunnelTermEmulateLegacyMimForUdbOnlyMimKeyNeededUdbsSet
(
    PRV_TGF_TTI_KEY_TYPE_ENT     keyType
);


