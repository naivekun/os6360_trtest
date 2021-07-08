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
* @file prvTgfDetailedMimTunnelStart.h
*
* @brief Verify the functionality of UP marking in MIM tunnel start
*
* @version   3
********************************************************************************
*/
#ifndef __prvTgfDetailedMimTunnelStarth
#define __prvTgfDetailedMimTunnelStarth

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfTunnelStartUpMarkingModeConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set index of the rule in the TCAM to 0
*         - set macMode PRV_TGF_TTI_MAC_MODE_SA_E
*         - set MAC mask as disable
*/
GT_VOID prvTgfTunnelStartUpMarkingModeConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelStartUpMarkingModeTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 MIM tunneled packet:
*         macDa = 00:00:00:00:00:22,
*         macSa = 00:00:00:00:34:02,
*         Tagged: true (with VLAN tag 5)
*         Ether Type: 0x0800
*         Success Criteria:
*         Packet striped and forwarded to port 23 VLAN 6
*/
GT_VOID prvTgfTunnelStartUpMarkingModeTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfTunnelStartUpMarkingModeConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfTunnelStartUpMarkingModeConfigRestore
(
    GT_VOID
);

/**
* @internal prvTgfTunnelStartMimEtherTypeConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set index of the rule in the TCAM to 0
*         - set macMode PRV_TGF_TTI_MAC_MODE_DA_E
*/
GT_VOID prvTgfTunnelStartMimEtherTypeConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelStartMimEtherTypeTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macSa = 00:00:00:00:00:22,
*         macDa = 00:00:00:00:34:02,
*         Tagged: true (with VLAN tag 5)
*         Ether Type: 0x0800
*         Success Criteria:
*         Packet striped and forwarded to port 23 VLAN 6
*/
GT_VOID prvTgfTunnelStartMimEtherTypeTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfTunnelStartMimEtherTypeConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfTunnelStartMimEtherTypeConfigRestore
(
    GT_VOID
);

/**
* @internal prvTgfTunnelStartIdeiMarkingModeConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set index of the rule in the TCAM to 0
*         - set macMode PRV_TGF_TTI_MAC_MODE_DA_E
*/
GT_VOID prvTgfTunnelStartIdeiMarkingModeConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelStartIdeiMarkingModeTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macSa = 00:00:00:00:00:22,
*         macDa = 00:00:00:00:34:02,
*         Tagged: true (with VLAN tag 5)
*         Ether Type: 0x0800
*         Success Criteria:
*         Packet striped and forwarded to port 23 VLAN 6
*/
GT_VOID prvTgfTunnelStartIdeiMarkingModeTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfTunnelStartIdeiMarkingModeConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfTunnelStartIdeiMarkingModeConfigRestore
(
    GT_VOID
);

/**
* @internal prvTgfTunnelStartMimISidAndBdaSet function
* @endinternal
*
* @brief   Set test configuration:
*         - set index of the rule in the TCAM to 0
*         - set macMode PRV_TGF_TTI_MAC_MODE_DA_E
* @param[in] mimISidAssignMode        - tunnel start MIM I-SID assign mode
* @param[in] mimBDaAssignMode         - tunnel start MIM B-DA (Backbone destination address)
*                                      assign mode
*                                       None
*/
GT_VOID prvTgfTunnelStartMimISidAndBdaSet
(
    IN PRV_TGF_TUNNEL_START_MIM_I_SID_ASSIGN_MODE_ENT  mimISidAssignMode,
    IN PRV_TGF_TUNNEL_START_MIM_B_DA_ASSIGN_MODE_ENT   mimBDaAssignMode
);

/**
* @internal prvTgfTunnelStartMimISidAndBdaTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macSa = 00:00:00:00:00:22,
*         macDa = 00:00:00:00:34:02,
*         Tagged: true (with VLAN tag 5)
*         Ether Type: 0x0800
*         Success Criteria:
*         Packet striped and forwarded to port 23 VLAN 6
* @param[in] mimISidAssignMode        - tunnel start MIM I-SID assign mode
* @param[in] mimBDaAssignMode         - tunnel start MIM B-DA (Backbone destination address)
*                                      assign mode
*                                       None
*/
GT_VOID prvTgfTunnelStartMimISidAndBdaTrafficGenerate
(
    IN PRV_TGF_TUNNEL_START_MIM_I_SID_ASSIGN_MODE_ENT  mimISidAssignMode,
    IN PRV_TGF_TUNNEL_START_MIM_B_DA_ASSIGN_MODE_ENT   mimBDaAssignMode
);

/**
* @internal prvTgfTunnelStartMimISidAndBdaConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfTunnelStartMimISidAndBdaConfigRestore
(
    GT_VOID
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfDetailedMimTunnelStarth */

