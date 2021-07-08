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
* @file tgfDitGen.h
*
* @brief Generic API implementation for DIT
*
* @version   2
********************************************************************************
*/
#ifndef __tgfDitGenh
#define __tgfDitGenh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @enum PRV_TGF_DIT_VLAN_TAG_COMMAND_ENT
 *
 * @brief The possible pre-trasmit manipulations on vlan tagging.
*/
typedef enum{

    /** @brief Packet is transmitted
     *  tagged with the DIT entry's vid and up.
     */
    PRV_TGF_DIT_VLAN_TAG_COMMAND_TAGGED_FROM_DIT_E        = 0,

    /** @brief Packet is transmitted
     *  tagged. If packet is already tagged its not modified.
     */
    PRV_TGF_DIT_VLAN_TAG_COMMAND_TAGGED_DONT_MODIFY_E     = 1,

    /** Packet is transmitted untagged. */
    PRV_TGF_DIT_VLAN_TAG_COMMAND_UNTAGGED_E               = 2,

    /** @brief Packet is transmitted
     *  asis tagged or untagged.
     */
    PRV_TGF_DIT_VLAN_TAG_COMMAND_DONT_MODIFY_E            = 3,

    /** @brief Packet is keeping incoming
     *  vlan tag and transmitted tagged according vlan table.
     */
    PRV_TGF_DIT_VLAN_TAG_COMMAND_KEEP_TAG_BY_VLT_E        = 4,

    /** @brief Packet's vlan
     *  is overrided by entry vid and transmitted tagged according
     *  vlan table.
     */
    PRV_TGF_DIT_VLAN_TAG_COMMAND_OVERRIDE_TAG_FROM_VLT_E  = 5,

    /** @brief Packet is keeping
     *  incoming vlan tag, entry's vid is added as external to
     *  packet and transmitted according vlan table from external tag.
     */
    PRV_TGF_DIT_VLAN_TAG_COMMAND_KEEP_ADD_EXTERNAL_TAG_E  = 6

} PRV_TGF_DIT_VLAN_TAG_COMMAND_ENT;

/**
* @enum PRV_TGF_DIT_QOS_COMMAND_TYPE_ENT
 *
 * @brief The possible Qos parameters assigment types.
*/
typedef enum{

    /** @brief Qos parameter is
     *  taken from incoming descriptor.
     */
    PRV_TGF_DIT_QOS_COMMAND_FROM_DESCRIPTOR_E    = 0,

    /** @brief Qos parameter is
     *  taken from DIT entry.
     */
    PRV_TGF_DIT_QOS_COMMAND_FROM_ENTRY_E         = 1

} PRV_TGF_DIT_QOS_COMMAND_TYPE_ENT;

/*
 * Typedef: struct PRV_TGF_DIT_VPLS_STC
 *
 * Description:
 *      Representation of Vpls entry Downstream Interface Table entry in HW.
 *
 * Fields:
 *
 *  outLif              - Self-explanatory.
 *  isTunnelStart       - if GT_TRUE - this DIT is tunnel start.
 *  tunnelStartPtr      - Pointer to a tunnel start entry
 *                        Relevant if isTunnelStart = GT_TRUE.
 *  tunnelStartPassengerPacketType
 *                      - Tunnel Start passenger protocol types.
 *                        Relevant if isTunnelStart = GT_TRUE.
 *  arpPointer          - ARP pointer. Relevant if isTunnelStart = GT_FALSE.
 *  vlanId              - VLAN-ID associated with current downstream interface.
 *  ttl                 - TTL value. Range 8 bits.
 *  vlanTagCommand      - Specifies pre-trasmit manipulations on vlan tagging.
 *                        Valid commands:
 *                              PRV_TGF_DIT_VLAN_TAG_COMMAND_TAGGED_FROM_DIT_E
 *                              PRV_TGF_DIT_VLAN_TAG_COMMAND_TAGGED_DONT_MODIFY_E
 *                              PRV_TGF_DIT_VLAN_TAG_COMMAND_UNTAGGED_E
 *                              PRV_TGF_DIT_VLAN_TAG_COMMAND_DONT_MODIFY_E
 *                              PRV_TGF_DIT_VLAN_TAG_COMMAND_KEEP_TAG_BY_VLT_E
 *                              PRV_TGF_DIT_VLAN_TAG_COMMAND_OVERRIDE_TAG_FROM_VLT_E
 *                              PRV_TGF_DIT_VLAN_TAG_COMMAND_KEEP_ADD_EXTERNAL_TAG_E
 *  label               -  Virtual connection label. Range 20 bits.
 *  exp                 -  Qos exp parameter. Valid values 0-7.
 *  qosExpCommand       -  Whether to use exp parameter from entry
 *                         or take from incoming descriptor.
 *  up                  -  Qos user priority parameter. Valid values 0-7.
 *  qosUpCommand        -  Whether to use up parameter from entry
 *                         or take from incoming descriptor.
 *  mplsCommand         -  Number of labels to push. Valid values 0,1.
 *  isProviderEdge      -  If GT_TRUE, DIT entry represent Provider's Edge
 *                         downstream interface.
 *                         If GT_FALSE, DIT entry represent Provider's Core
 *                         downstream interface.
 *                         This is used for Split Horizon.
 *  vplsUnregisteredBcEnable -  When TRUE, Unregistered BC packets,
 *                              are NOT egress filtered.
 *  vplsUnknownUcEnable      -  When TRUE, Unknown UC packets,
 *                              are NOT egress filtered.
 *  vplsUnregisteredMcEnable -  When TRUE, Unregistered MC packets,
 *                              are NOT egress filtered.
 *  pwId                -  Pseudo-wire ID. Range 7 bits.
 *  nextPointer         -  Next DIT pointer. Relevant if isLast = GT_FALSE
 *  isLast              -  If GT_TRUE then this is the last DIT entry in the
 *                         list.
 *  vllEgressFilteringEnable - disable/enable egress filtering on VLL
 *                             (Virtual Leased Line) packets.
 *  bypassDitTargetEnable - When enabled, the DIT entry doesn't change
 *                          the descriptor target information
 *                          (APPLICABLE DEVICES: Puma3)
 *  fcoeMacDaPseudoRoutingEnable - This bit should be set to enable pseudo routing
 *                    of FCoE packets. This bit controls the MAC-DA modification.
 *  fcoeMacSaPseudoRoutingEnable - This bit should be set to enable pseudo routing
 *                    of FCoE packets. This bit controls the MAC-SA modification.
 */
typedef struct PRV_TGF_DIT_VPLS_STC
{
    CPSS_INTERFACE_INFO_STC                     outLif;
    GT_BOOL                                     isTunnelStart;
    GT_U32                                      tunnelStartPtr;
    PRV_TGF_TS_PASSENGER_PACKET_TYPE_ENT        tunnelStartPassengerPacketType;
    GT_U32                                      arpPointer;
    GT_U16                                      vlanId;
    GT_U32                                      ttl;
    PRV_TGF_DIT_VLAN_TAG_COMMAND_ENT            vlanTagCommand;
    GT_U32                                      label;
    GT_U32                                      exp;
    PRV_TGF_DIT_QOS_COMMAND_TYPE_ENT            qosExpCommand;
    GT_U32                                      up;
    PRV_TGF_DIT_QOS_COMMAND_TYPE_ENT            qosUpCommand;
    GT_U32                                      mplsCommand;
    GT_BOOL                                     isProviderEdge;
    GT_BOOL                                     vplsUnregisteredBcEnable;
    GT_BOOL                                     vplsUnknownUcEnable;
    GT_BOOL                                     vplsUnregisteredMcEnable;
    GT_U32                                      pwId;
    GT_U32                                      nextPointer;
    GT_BOOL                                     isLast;
    GT_BOOL                                     vllEgressFilteringEnable;
    GT_BOOL                                     bypassDitTargetEnable;
    GT_BOOL                                     fcoeMacDaPseudoRoutingEnable;
    GT_BOOL                                     fcoeMacSaPseudoRoutingEnable;

} PRV_TGF_DIT_VPLS_STC;

/**
* @enum PRV_TGF_DIT_VPWS_TARGET_TAGGED_MODE_ENT
 *
 * @brief default target tagged mode for VPWS packets
*/
typedef enum{

    /** @brief VPWS packets are
     *  transmitted tagged/untagged based on the port configuration
     *  in the VLAN table
     */
    PRV_TGF_DIT_VPWS_TARGET_TAGGED_VLAN_TABLE_MODE_E  = 0,

    /** @brief VPWS packets are
     *  transmitted tagged/untagged based on the <VPWS Target Tagged
     *  Control> configuration
     */
    PRV_TGF_DIT_VPWS_TARGET_TAGGED_CONTROL_MODE_E     = 1

} PRV_TGF_DIT_VPWS_TARGET_TAGGED_MODE_ENT;


/**
* @internal prvTgfDitFcoeEnableSet function
* @endinternal
*
* @brief   Enable/Disable FCoE (Fiber Channel over Ethernet)
*
* @param[in] devNum                   - physical device number
* @param[in] enable                   - If GT_TRUE   FCoE
*                                      If GT_FALSE disable FCoE
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on illegal parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfDitFcoeEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
);

/**
* @internal prvTgfDitFcoeEnableGet function
* @endinternal
*
* @brief   Gets whether FCoE (Fiber Channel over Ethernet) is enabled/disabled.
*
* @param[in] devNum                   - physical device number
*
* @param[out] enabledPtr               - If GT_TRUE  then FCoE is enabled.
*                                      If GT_FALSE then FCoE is disabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on illegal parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfDitFcoeEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enabledPtr
);

/**
* @internal prvTgfDitFcoeEtherTypeSet function
* @endinternal
*
* @brief   Set FCoE (Fiber Channel over Ethernet) Ether Type value.
*
* @param[in] devNum                   - physical device number
* @param[in] etherType                - Ether Type Value
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on illegal parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfDitFcoeEtherTypeSet
(
    IN  GT_U8       devNum,
    IN  GT_U16      etherType
);

/**
* @internal prvTgfDitFcoeEtherTypeGet function
* @endinternal
*
* @brief   Gets FCoE (Fiber Channel over Ethernet) Ether Type
*
* @note   APPLICABLE DEVICES:      Puma2; Puma3.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*
* @param[out] etherTypePtr             - Ether Type Value
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on illegal parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfDitFcoeEtherTypeGet
(
    IN  GT_U8       devNum,
    OUT GT_U16      *etherTypePtr
);

/*******************************************************************************
* prvTgfDitVplsEntryWrite
*
* DESCRIPTION:
*       Writes a Downstream interface Table (DIT) Vpls entry to hw.
*
* INPUTS:
*       devNum          - device number
*       ditEntryIndex   - The DIT Vpls entry index.
*       ditType         - dit packet type selection (unicast/multicast)
*       ditEntryPtr     - The downstream interface entry
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_BAD_PARAM             - on illegal parameter
*       GT_BAD_PTR               - on one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvTgfDitVplsEntryWrite
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        ditEntryIndex,
    IN  CPSS_UNICAST_MULTICAST_ENT    ditType,
    IN  PRV_TGF_DIT_VPLS_STC          *ditEntryPtr
);

/**
* @internal prvTgfDitVplsEntryRead function
* @endinternal
*
* @brief   Reads a Downstream interface Table (DIT) Vpls entry from the hw.
*
* @param[in] devNum                   - device number
* @param[in] ditEntryIndex            - The DIT Vpls entry index.
* @param[in] ditType                  - dit packet type selection (unicast/multicast)
*
* @param[out] ditEntryPtr              - The downstream interface entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on illegal parameter
* @retval GT_BAD_PTR               - on one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_UNFIXABLE_ECC_ERROR   - the CPSS detected ECC error that can't
*                                       be fixed when read from the memory that
*                                       protected by ECC generated.
*                                       if entry can't be fixed due to 2 data errors
*                                       NOTE: relevant only to memory controller that
*                                       work with ECC protection , and the CPSS need
*                                       to emulate ECC protection.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfDitVplsEntryRead
(
    IN  GT_U8                        devNum,
    IN  GT_U32                       ditEntryIndex,
    IN  CPSS_UNICAST_MULTICAST_ENT   ditType,
    OUT PRV_TGF_DIT_VPLS_STC         *ditEntryPtr
);

/**
* @internal prvTgfDitVpwsTargetTaggedModeSet function
* @endinternal
*
* @brief   Controls the default target tagged mode for VPWS packets.
*
* @param[in] devNum                   - device number
* @param[in] vpwsTargetTaggedMode     - VPWS target tagged mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on illegal parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfDitVpwsTargetTaggedModeSet
(
    IN  GT_U8                                       devNum,
    IN  PRV_TGF_DIT_VPWS_TARGET_TAGGED_MODE_ENT     vpwsTargetTaggedMode
);

/**
* @internal prvTgfDitVpwsTargetTaggedModeGet function
* @endinternal
*
* @brief   Get the default target tagged mode for VPWS packets.
*
* @param[in] devNum                   - device number
*
* @param[out] vpwsTargetTaggedModePtr  - points to VPWS target tagged mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on illegal parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfDitVpwsTargetTaggedModeGet
(
    IN  GT_U8                                       devNum,
    OUT PRV_TGF_DIT_VPWS_TARGET_TAGGED_MODE_ENT     *vpwsTargetTaggedModePtr
);

/**
* @internal prvTgfDitVplsEntryWrite function
* @endinternal
*
* @brief   Writes a Downstream interface Table (DIT) Vpls entry to hw.
*
* @param[in] devNum                   - device number
* @param[in] ditEntryIndex            - The DIT Vpls entry index.
* @param[in] ditType                  - dit packet type selection (unicast/multicast)
* @param[in] ditEntryPtr              - The downstream interface entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on illegal parameter
* @retval GT_BAD_PTR               - on one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfDitVplsEntryWrite
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        ditEntryIndex,
    IN  CPSS_UNICAST_MULTICAST_ENT    ditType,
    IN  PRV_TGF_DIT_VPLS_STC          *ditEntryPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __tgfDitGenh */



