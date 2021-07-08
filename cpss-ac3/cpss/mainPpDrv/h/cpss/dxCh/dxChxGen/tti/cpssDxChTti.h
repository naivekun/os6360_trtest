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
* @file cpssDxChTti.h
*
* @brief CPSS tunnel termination declarations.
*
* @version   54
********************************************************************************
*/

#ifndef __cpssDxChTtih
#define __cpssDxChTtih

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/dxCh/dxChxGen/tti/cpssDxChTtiTypes.h>
#include <cpss/generic/ip/cpssIpTypes.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>

/**
* General NOTEs about portGroupsBmp In TCAM related APIs:
*   1. In Falcon :
*     the TCAM is shared between 2 port groups.
*     the caller need to use 'representative' port groups , for example:
*     value 0x00000014 hold bits 2,4 represent TCAMs of port groups 2,3 and 4,5
*     value 0x00000041 hold bits 0,6 represent TCAMs of port groups 0,1 and 6,7
*     Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.(represent all port groups)
*
**/



/**
* @internal cpssDxChTtiMacToMeSet function
* @endinternal
*
* @brief   function sets a TTI MacToMe entry.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] entryIndex               - Index of mac and vlan in MacToMe table.
*                                      (APPLICABLE RANGES:
*                                      xCat3; Lion2: 0..7
*                                      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2: 0..127)
* @param[in] valuePtr                 - points to Mac To Me and Vlan To Me
* @param[in] maskPtr                  - points to mac and vlan's masks
* @param[in] interfaceInfoPtr         - points to source interface info (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id or wrong vlan/mac values
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiMacToMeSet
(
    IN  GT_U8                                           devNum,
    IN  GT_U32                                          entryIndex,
    IN  CPSS_DXCH_TTI_MAC_VLAN_STC                      *valuePtr,
    IN  CPSS_DXCH_TTI_MAC_VLAN_STC                      *maskPtr,
    IN  CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC  *interfaceInfoPtr
);


/**
* @internal cpssDxChTtiMacToMeGet function
* @endinternal
*
* @brief   This function gets a TTI MacToMe entry.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] entryIndex               - Index of mac and vlan in MacToMe table.
*                                      (APPLICABLE RANGES:
*                                      xCat3; Lion2: 0..7
*                                      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2: 0..127)
*
* @param[out] valuePtr                 - points to Mac To Me and Vlan To Me
* @param[out] maskPtr                  - points to mac and vlan's masks
* @param[out] interfaceInfoPtr         - points to source interface info (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter's value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiMacToMeGet
(
    IN  GT_U8                                           devNum,
    IN  GT_U32                                          entryIndex,
    OUT CPSS_DXCH_TTI_MAC_VLAN_STC                      *valuePtr,
    OUT CPSS_DXCH_TTI_MAC_VLAN_STC                      *maskPtr,
    OUT CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC  *interfaceInfoPtr
);

/**
* @internal cpssDxChTtiPortLookupEnableSet function
* @endinternal
*
* @brief   This function enables/disables the TTI lookup for the specified key
*         type at the port.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] keyType                  - TTI key type
* @param[in] enable                   - GT_TRUE:  TTI lookup
*                                      GT_FALSE: disable TTI lookup
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPortLookupEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT          keyType,
    IN  GT_BOOL                             enable
);
/**
* @internal cpssDxChTtiPortLookupEnableGet function
* @endinternal
*
* @brief   This function gets the port's current state (enable/disable) of the
*         TTI lookup for the specified key type.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] keyType                  - TTI key type
*
* @param[out] enablePtr                - points to enable/disable TTI lookup
*                                      GT_TRUE: TTI lookup is enabled
*                                      GT_FALSE: TTI lookup is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id, port or key type
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPortLookupEnableGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT          keyType,
    OUT GT_BOOL                             *enablePtr
);

/**
* @internal cpssDxChTtiPortIpv4OnlyTunneledEnableSet function
* @endinternal
*
* @brief   This function enables/disables the IPv4 TTI lookup for only tunneled
*         packets received on port.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   - GT_TRUE:  IPv4 TTI lookup only for tunneled packets
*                                      GT_FALSE: disable IPv4 TTI lookup only for tunneled packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPortIpv4OnlyTunneledEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  GT_BOOL                             enable
);

/**
* @internal cpssDxChTtiPortIpv4OnlyTunneledEnableGet function
* @endinternal
*
* @brief   This function gets the port's current state (enable/disable) of the
*         IPv4 TTI lookup for only tunneled packets received on port.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
*
* @param[out] enablePtr                - points to enable/disable IPv4 TTI lookup only for
*                                      tunneled packets
*                                      GT_TRUE: IPv4 TTI lookup only for tunneled packets is enabled
*                                      GT_FALSE: IPv4 TTI lookup only for tunneled packets is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPortIpv4OnlyTunneledEnableGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    OUT GT_BOOL                             *enablePtr
);

/**
* @internal cpssDxChTtiPortIpv4OnlyMacToMeEnableSet function
* @endinternal
*
* @brief   This function enables/disables the IPv4 TTI lookup for only mac to me
*         packets received on port.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   - GT_TRUE:  IPv4 TTI lookup only for mac to me packets
*                                      GT_FALSE: disable IPv4 TTI lookup only for mac to me packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPortIpv4OnlyMacToMeEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  GT_BOOL                             enable
);

/**
* @internal cpssDxChTtiPortIpv4OnlyMacToMeEnableGet function
* @endinternal
*
* @brief   This function gets the port's current state (enable/disable) of the
*         IPv4 TTI lookup for only mac to me packets received on port.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
*
* @param[out] enablePtr                - points to enable/disable IPv4 TTI lookup only for
*                                      mac to me packets
*                                      GT_TRUE: IPv4 TTI lookup only for mac to me packets is enabled
*                                      GT_FALSE: IPv4 TTI lookup only for mac to me packets is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPortIpv4OnlyMacToMeEnableGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    OUT GT_BOOL                             *enablePtr
);

/**
* @internal cpssDxChTtiIpv4McEnableSet function
* @endinternal
*
* @brief   This function enables/disables the TTI lookup for IPv4 multicast
*         (relevant only to IPv4 lookup keys).
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE:  TTI lookup for IPv4 MC
*                                      GT_FALSE: disable TTI lookup for IPv4 MC
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiIpv4McEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     enable
);

/**
* @internal cpssDxChTtiIpv4McEnableGet function
* @endinternal
*
* @brief   This function gets the current state (enable/disable) of TTI lookup for
*         IPv4 multicast (relevant only to IPv4 lookup keys).
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - points to enable/disable TTI lookup for IPv4 MC
*                                      GT_TRUE: TTI lookup for IPv4 MC enabled
*                                      GT_FALSE: TTI lookup for IPv4 MC disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiIpv4McEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
);

/**
* @internal cpssDxChTtiPortMplsOnlyMacToMeEnableSet function
* @endinternal
*
* @brief   This function enables/disables the MPLS TTI lookup for only mac to me
*         packets received on port.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   - GT_TRUE:  MPLS TTI lookup only for mac to me packets
*                                      GT_FALSE: disable MPLS TTI lookup only for mac to me packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPortMplsOnlyMacToMeEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  GT_BOOL                             enable
);

/**
* @internal cpssDxChTtiPortMplsOnlyMacToMeEnableGet function
* @endinternal
*
* @brief   This function gets the port's current state (enable/disable) of the
*         MPLS TTI lookup for only mac to me packets received on port.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
*
* @param[out] enablePtr                - points to enable/disable IPv4 TTI lookup only for
*                                      mac to me packets
*                                      GT_TRUE: MPLS TTI lookup only for mac to me packets is enabled
*                                      GT_FALSE: MPLS TTI lookup only for mac to me packets is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPortMplsOnlyMacToMeEnableGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    OUT GT_BOOL                             *enablePtr
);

/**
* @internal cpssDxChTtiPortMimOnlyMacToMeEnableSet function
* @endinternal
*
* @brief   This function enables/disables the MIM TTI lookup for only mac to me
*         packets received on port.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   - GT_TRUE:   MIM TTI lookup only for mac to me packets
*                                      GT_FALSE: disable MIM TTI lookup only for mac to me packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPortMimOnlyMacToMeEnableSet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    IN  GT_BOOL         enable
);

/**
* @internal cpssDxChTtiPortMimOnlyMacToMeEnableGet function
* @endinternal
*
* @brief   This function gets the port's current state (enable/disable) of the
*         MIM TTI lookup for only mac to me packets received on port.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
*
* @param[out] enablePtr                - points to enable/disable MIM TTI lookup only for
*                                      mac to me packets
*                                      GT_TRUE:  MIM TTI lookup only for mac to me packets is enabled
*                                      GT_FALSE: MIM TTI lookup only for mac to me packets is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPortMimOnlyMacToMeEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal cpssDxChTtiRuleSet function
* @endinternal
*
* @brief   This function sets the TTI Rule Pattern, Mask and Action for specific
*         TCAM location according to the rule Key Type.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] index                    - Global  of the rule in the TCAM
* @param[in] ruleType                 - TTI rule type
* @param[in] patternPtr               - points to the rule's pattern
* @param[in] maskPtr                  - points to the rule's mask. The rule mask is "AND STYLED
*                                      ONE". Mask bit's 0 and pattern bit's 0 means don't care bit
*                                      (corresponding bit in the pattern is not used in the TCAM lookup).
*                                      Mask bit's 0 and pattern bit's 1 means ALWAYS MISS (full entry miss).
*                                      Mask bit's 1 means that corresponding bit in the pattern
*                                      is using in the TCAM lookup.
* @param[in] actionType               - type of the action to use
* @param[in] actionPtr                - points to the TTI rule action that applied on packet
*                                      if packet's search key matched with masked pattern.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiRuleSet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              index,
    IN  CPSS_DXCH_TTI_RULE_TYPE_ENT         ruleType,
    IN  CPSS_DXCH_TTI_RULE_UNT              *patternPtr,
    IN  CPSS_DXCH_TTI_RULE_UNT              *maskPtr,
    IN  CPSS_DXCH_TTI_ACTION_TYPE_ENT       actionType,
    IN  CPSS_DXCH_TTI_ACTION_UNT            *actionPtr
);

/**
* @internal cpssDxChTtiRuleGet function
* @endinternal
*
* @brief   This function gets the TTI Rule Pattern, Mask and Action for specific
*         TCAM location according to the rule Key Type.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] index                    - Global  of the rule in the TCAM
* @param[in] ruleType                 - TTI rule type
* @param[in] actionType               - type of the action to use
*
* @param[out] patternPtr               - points to the rule's pattern
* @param[out] maskPtr                  - points to the rule's mask. The rule mask is "AND STYLED
*                                      ONE". Mask bit's 0 and pattern bit's 0 means don't care bit
*                                      (corresponding bit in the pattern is not used in the TCAM lookup).
*                                      Mask bit's 0 and pattern bit's 1 means ALWAYS MISS (full entry miss).
*                                      Mask bit's 1 means that corresponding bit in the pattern
*                                      is using in the TCAM lookup.
* @param[out] actionPtr                - points to the TTI rule action that applied on packet
*                                      if packet's search key matched with masked pattern.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_STATE             - a rule of different type was found in the specified index
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiRuleGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              index,
    IN  CPSS_DXCH_TTI_RULE_TYPE_ENT         ruleType,
    OUT CPSS_DXCH_TTI_RULE_UNT              *patternPtr,
    OUT CPSS_DXCH_TTI_RULE_UNT              *maskPtr,
    IN  CPSS_DXCH_TTI_ACTION_TYPE_ENT       actionType,
    OUT CPSS_DXCH_TTI_ACTION_UNT            *actionPtr
);

/**
* @internal cpssDxChTtiRuleActionUpdate function
* @endinternal
*
* @brief   This function updates rule action.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] index                    - Global  of the rule in the TCAM
* @param[in] actionType               - type of the action to use
* @param[in] actionPtr                - points to the TTI rule action that applied on packet
*                                      if packet's search key matched with masked pattern.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiRuleActionUpdate
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              index,
    IN  CPSS_DXCH_TTI_ACTION_TYPE_ENT       actionType,
    IN  CPSS_DXCH_TTI_ACTION_UNT            *actionPtr
);

/**
* @internal cpssDxChTtiRuleValidStatusSet function
* @endinternal
*
* @brief   This function validates / invalidates the rule in TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] index                    - Global  of the rule in the TCAM
* @param[in] valid                    - GT_TRUE - valid, GT_FALSE - invalid
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note If valid == GT_TRUE it is assumed that the TCAM entry already contains
*       the TTI entry information.
*
*/
GT_STATUS cpssDxChTtiRuleValidStatusSet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              index,
    IN  GT_BOOL                             valid
);

/**
* @internal cpssDxChTtiRuleValidStatusGet function
* @endinternal
*
* @brief   This function returns the valid status of the rule in TCAM
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] index                    - Global  of the rule in the TCAM
*
* @param[out] validPtr                 - GT_TRUE - valid, GT_FALSE - invalid
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiRuleValidStatusGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              index,
    OUT GT_BOOL                             *validPtr
);

/**
* @internal cpssDxChTtiMacModeSet function
* @endinternal
*
* @brief   This function sets the lookup Mac mode for the specified key type.
*         This setting controls the Mac that would be used for key generation
*         (Source/Destination).
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] keyType                  - TTI key type; valid values:
*                                      CPSS_DXCH_TTI_KEY_IPV4_E
*                                      CPSS_DXCH_TTI_KEY_MPLS_E
*                                      CPSS_DXCH_TTI_KEY_ETH_E
*                                      CPSS_DXCH_TTI_KEY_MIM_E  (APPLICABLE DEVICES xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
* @param[in] macMode                  - MAC mode to use; valid values:
*                                      CPSS_DXCH_TTI_MAC_MODE_DA_E
*                                      CPSS_DXCH_TTI_MAC_MODE_SA_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Not needed for TRILL key
*
*/
GT_STATUS cpssDxChTtiMacModeSet
(
    IN  GT_U8                             devNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT        keyType,
    IN  CPSS_DXCH_TTI_MAC_MODE_ENT        macMode
);

/**
* @internal cpssDxChTtiMacModeGet function
* @endinternal
*
* @brief   This function gets the lookup Mac mode for the specified key type.
*         This setting controls the Mac that would be used for key generation
*         (Source/Destination).
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] keyType                  - TTI key type; valid values:
*                                      CPSS_DXCH_TTI_KEY_IPV4_E
*                                      CPSS_DXCH_TTI_KEY_MPLS_E
*                                      CPSS_DXCH_TTI_KEY_ETH_E
*                                      CPSS_DXCH_TTI_KEY_MIM_E  (APPLICABLE DEVICES xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*
* @param[out] macModePtr               - MAC mode to use; valid values:
*                                      CPSS_DXCH_TTI_MAC_MODE_DA_E
*                                      CPSS_DXCH_TTI_MAC_MODE_SA_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id or key type
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Not needed for TRILL key
*
*/
GT_STATUS cpssDxChTtiMacModeGet
(
    IN  GT_U8                             devNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT        keyType,
    OUT CPSS_DXCH_TTI_MAC_MODE_ENT        *macModePtr
);

/**
* @internal cpssDxChTtiPclIdSet function
* @endinternal
*
* @brief   This function sets the PCL ID for the specified key type. The PCL ID
*         is used to distinguish between different TTI keys in the TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] keyType                  - TTI key type
* @param[in] pclId                    - PCL ID value (10 bits)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The TTI PCL ID values are only relevant for the TTI TCAM and are not
*       related to the PCL ID values in the PCL TCAM.
*
*/
GT_STATUS cpssDxChTtiPclIdSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT      keyType,
    IN  GT_U32                          pclId
);

/**
* @internal cpssDxChTtiPclIdGet function
* @endinternal
*
* @brief   This function gets the PCL ID for the specified key type. The PCL ID
*         is used to distinguish between different TTI keys in the TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] keyType                  - TTI key type
*
* @param[out] pclIdPtr                 - (points to) PCL ID value (10 bits)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The TTI PCL ID values are only relevant for the TTI TCAM and are not
*       related to the PCL ID values in the PCL TCAM.
*
*/
GT_STATUS cpssDxChTtiPclIdGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT      keyType,
    OUT GT_U32                          *pclIdPtr
);

/**
* @internal cpssDxChTtiExceptionCmdSet function
* @endinternal
*
* @brief   Set tunnel termination exception command.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] exceptionType            - tunnel termination exception type to set command for
* @param[in] command                  -  to set
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For Bobcat2, Caelum, Bobcat3, Aldrin, AC3X, Aldrin2:
*       For the following exceptions:
*       CPSS_DXCH_TTI_EXCEPTION_IPV4_HEADER_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV4_UNSUP_GRE_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV4_SIP_ADDRESS_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV4_OPTION_FRAG_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV6_UNSUP_GRE_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV6_HEADER_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV6_HBH_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV6_NON_HBH_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV6_SIP_ADDRESS_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_MPLS_ILLEGAL_TTL_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_MPLS_UNSUPPORTED_ERROR_E
*       The commands are
*       CPSS_PACKET_CMD_FORWARD_E
*       CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*       CPSS_PACKET_CMD_TRAP_TO_CPU_E
*       CPSS_PACKET_CMD_DROP_HARD_E
*       CPSS_PACKET_CMD_DROP_SOFT_E
*       For xCat3, Lion2:
*       For the following exceptions:
*       CPSS_DXCH_TTI_EXCEPTION_IPV4_HEADER_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV4_OPTION_FRAG_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV4_UNSUP_GRE_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_MPLS_ILLEGAL_TTL_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_MPLS_UNSUPPORTED_ERROR_E
*       The commands are:
*       CPSS_PACKET_CMD_TRAP_TO_CPU_E
*       CPSS_PACKET_CMD_DROP_HARD_E
*       For the following exceptions:
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_IS_IS_ADJACENCY_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_TREE_ADJACENCY_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_BAD_VERSION_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_HOPCOUNT_IS_ZERO_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_OPTIONS_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_CHBH_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_BAD_OUTER_VID0_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_NOT_TO_ME_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_BAD_OUTER_DA_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_BAD_OUTER_DA_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_OUTER_UC_INNER_MC_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_WITH_BAD_OUTER_DA_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_OPTIONS_TOO_LONG_OPTION_E
*       The commands are:
*       CPSS_PACKET_CMD_FORWARD_E
*       CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*       CPSS_PACKET_CMD_TRAP_TO_CPU_E
*       CPSS_PACKET_CMD_DROP_HARD_E
*       CPSS_PACKET_CMD_DROP_SOFT_E
*
*/
GT_STATUS cpssDxChTtiExceptionCmdSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_EXCEPTION_ENT         exceptionType,
    IN  CPSS_PACKET_CMD_ENT                 command
);

/**
* @internal cpssDxChTtiExceptionCmdGet function
* @endinternal
*
* @brief   Get tunnel termination exception command.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - physical device number
* @param[in] exceptionType            - tunnel termination exception type to set command for
*
* @param[out] commandPtr               - points to the command for the exception type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For Bobcat2, Caelum, Bobcat3, Aldrin, AC3X:
*       For the following exceptions:
*       CPSS_DXCH_TTI_EXCEPTION_IPV4_HEADER_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV4_UNSUP_GRE_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV4_SIP_ADDRESS_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV4_OPTION_FRAG_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV6_UNSUP_GRE_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV6_HEADER_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV6_HBH_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV6_NON_HBH_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV6_SIP_ADDRESS_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_MPLS_ILLEGAL_TTL_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_MPLS_UNSUPPORTED_ERROR_E
*       The commands are
*       CPSS_PACKET_CMD_FORWARD_E
*       CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*       CPSS_PACKET_CMD_TRAP_TO_CPU_E
*       CPSS_PACKET_CMD_DROP_HARD_E
*       CPSS_PACKET_CMD_DROP_SOFT_E
*       For xCat3, Lion2:
*       For the following exceptions:
*       CPSS_DXCH_TTI_EXCEPTION_IPV4_HEADER_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV4_OPTION_FRAG_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_IPV4_UNSUP_GRE_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_MPLS_ILLEGAL_TTL_ERROR_E
*       CPSS_DXCH_TTI_EXCEPTION_MPLS_UNSUPPORTED_ERROR_E
*       The commands are:
*       CPSS_PACKET_CMD_TRAP_TO_CPU_E
*       CPSS_PACKET_CMD_DROP_HARD_E
*       For the following exceptions:
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_IS_IS_ADJACENCY_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_TREE_ADJACENCY_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_BAD_VERSION_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_HOPCOUNT_IS_ZERO_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_OPTIONS_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_CHBH_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_BAD_OUTER_VID0_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_NOT_TO_ME_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_BAD_OUTER_DA_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_UC_BAD_OUTER_DA_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_OUTER_UC_INNER_MC_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_MC_WITH_BAD_OUTER_DA_E
*       CPSS_DXCH_TTI_EXCEPTION_TRILL_OPTIONS_TOO_LONG_OPTION_E
*       The commands are:
*       CPSS_PACKET_CMD_FORWARD_E
*       CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*       CPSS_PACKET_CMD_TRAP_TO_CPU_E
*       CPSS_PACKET_CMD_DROP_HARD_E
*       CPSS_PACKET_CMD_DROP_SOFT_E
*
*/
GT_STATUS cpssDxChTtiExceptionCmdGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_EXCEPTION_ENT         exceptionType,
    OUT CPSS_PACKET_CMD_ENT                 *commandPtr
);

/**
* @internal cpssDxChTtiExceptionCpuCodeSet function
* @endinternal
*
* @brief   Set tunnel termination exception CPU code.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] exceptionType            - tunnel termination exception type to set CPU code for.
*                                      valid values:
*                                      CPSS_DXCH_TTI_EXCEPTION_IPV4_SIP_ADDRESS_ERROR_E
*                                      CPSS_DXCH_TTI_EXCEPTION_IPV6_UNSUP_GRE_ERROR_E
*                                      CPSS_DXCH_TTI_EXCEPTION_IPV6_HEADER_ERROR_E
*                                      CPSS_DXCH_TTI_EXCEPTION_IPV6_HBH_ERROR_E
*                                      CPSS_DXCH_TTI_EXCEPTION_IPV6_NON_HBH_ERROR_E
*                                      CPSS_DXCH_TTI_EXCEPTION_IPV6_SIP_ADDRESS_ERROR_E
* @param[in] code                     - the  for the exception type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiExceptionCpuCodeSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_EXCEPTION_ENT         exceptionType,
    IN  CPSS_NET_RX_CPU_CODE_ENT            code
);

/**
* @internal cpssDxChTtiExceptionCpuCodeGet function
* @endinternal
*
* @brief   Get tunnel termination exception CPU code.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] exceptionType            - tunnel termination exception type to get CPU code for.
*                                      valid values:
*                                      CPSS_DXCH_TTI_EXCEPTION_IPV4_SIP_ADDRESS_ERROR_E
*                                      CPSS_DXCH_TTI_EXCEPTION_IPV6_UNSUP_GRE_ERROR_E
*                                      CPSS_DXCH_TTI_EXCEPTION_IPV6_HEADER_ERROR_E
*                                      CPSS_DXCH_TTI_EXCEPTION_IPV6_HBH_ERROR_E
*                                      CPSS_DXCH_TTI_EXCEPTION_IPV6_NON_HBH_ERROR_E
*                                      CPSS_DXCH_TTI_EXCEPTION_IPV6_SIP_ADDRESS_ERROR_E
*
* @param[out] codePtr                  - (points to) the code for the exception type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiExceptionCpuCodeGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_EXCEPTION_ENT         exceptionType,
    OUT CPSS_NET_RX_CPU_CODE_ENT            *codePtr
);

/**
* @internal cpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableSet function
* @endinternal
*
* @brief   IPv4 Tunnel Termination Header Error exception is detected if ANY of the following criteria are NOT met:
*         - IPv4 header <Checksum> is correct
*         - IPv4 header <Version> = 4
*         - IPv4 header <IHL> (IP Header Length) >= 5 (32-bit words)
*         - IPv4 header <IHL> (IP Header Length) <= IPv4 header <Total Length> / 4
*         - IPv4 header <Total Length> + packet L3 Offset + 4 (CRC length) <= MAC layer packet byte count
*         - IPv4 header <SIP> != IPv4 header <DIP>
*         This function globally enables/disables bypassing IPv4 header length criteria checks as part of
*         IPv4 header exception checking.
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE:   bypass header length check
*                                      GT_FALSE: disable bypass header length check
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_BOOL                             enable
);

/**
* @internal cpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableGet function
* @endinternal
*
* @brief   IPv4 Tunnel Termination Header Error exception is detected if ANY of the following criteria are NOT met:
*         - IPv4 header <Checksum> is correct
*         - IPv4 header <Version> = 4
*         - IPv4 header <IHL> (IP Header Length) >= 5 (32-bit words)
*         - IPv4 header <IHL> (IP Header Length) <= IPv4 header <Total Length> / 4
*         - IPv4 header <Total Length> + packet L3 Offset + 4 (CRC length) <= MAC layer packet byte count
*         - IPv4 header <SIP> != IPv4 header <DIP>
*         This function gets the globally bypassing IPv4 header length criteria check as part of
*         IPv4 header exception checking.
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - points to enable/disable bypass header length check
*                                      GT_TRUE:  enable bypass header length check
*                                      GT_FALSE: disable bypass header length check
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiBypassHeaderLengthCheckInIpv4TtiHeaderExceptionEnableGet
(
    IN  GT_U8                               devNum,
    OUT GT_BOOL                             *enablePtr
);

/**
* @internal cpssDxChTtiPortIpTotalLengthDeductionEnableSet function
* @endinternal
*
* @brief   For MACSEC packets over IPv4/6 tunnel, that are to be tunnel terminated,
*         this configuration enables aligning the IPv4/6 total header length to the
*         correct offset.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   - Enable/disable Ipv4/6 Total Length Deduction.
*                                      GT_TRUE: When enabled, and ePort default attribute
*                                      <IPv4/6 Total Length Deduction Enable> == Enabled:
*                                      - for IPv4:
*                                      <IPv4 Total Length> = Packet IPv4 header <Total Length> -
*                                      Global configuration < IPv4 Total Length Deduction Value>
*                                      - for IPv6:
*                                      <IPv6 Total Length> = Packet IPv6 header <Total Length> -
*                                      Global configuration < IPv6 Total Length Deduction Value>
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant for Ethernet-over-IPv4/6-GRE packets
*
*/
GT_STATUS cpssDxChTtiPortIpTotalLengthDeductionEnableSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
);

/**
* @internal cpssDxChTtiPortIpTotalLengthDeductionEnableGet function
* @endinternal
*
* @brief   Get if IPv4/6 total header length is aligned to the correct offset
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
*
* @param[out] enablePtr                -  Enable/disable Ipv4/6 Total Length Deduction.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant for Ethernet-over-IPv4/6-GRE packets
*
*/
GT_STATUS cpssDxChTtiPortIpTotalLengthDeductionEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal cpssDxChTtiIpTotalLengthDeductionValueSet function
* @endinternal
*
* @brief   Set Global configuration IPv4 or IPv6 Total Length Deduction Value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] ipType                   - IPv4 or Ipv6; valid values:
*                                      CPSS_IP_PROTOCOL_IPV4_E
*                                      CPSS_IP_PROTOCOL_IPV6_E
* @param[in] value                    - IPv4 or IPv6 Total Length Deduction Value .(APPLICABLE RANGES: 0..63)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant for Ethernet-over-IPv4/6-GRE packets
*
*/
GT_STATUS cpssDxChTtiIpTotalLengthDeductionValueSet
(
    IN GT_U8                                devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT           ipType,
    IN GT_U32                               value
);

/**
* @internal cpssDxChTtiIpTotalLengthDeductionValueGet function
* @endinternal
*
* @brief   Get Global configuration IPv4 or IPv6 Total Length Deduction Value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] ipType                   - IPv4 or Ipv6; valid values:
*                                      CPSS_IP_PROTOCOL_IPV4_E
*                                      CPSS_IP_PROTOCOL_IPV6_E
*
* @param[out] valuePtr                 -  (pointer to) IPv4 or IPv6 Total Length Deduction Value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant for Ethernet-over-IPv4/6-GRE packets
*
*/
GT_STATUS cpssDxChTtiIpTotalLengthDeductionValueGet
(
    IN  GT_U8                               devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT           ipType,
    OUT GT_U32                              *valuePtr
);

/**
* @internal cpssDxChTtiEthernetTypeSet function
* @endinternal
*
* @brief   This function sets the TTI Ethernet type value that is used
*         to identify packets for TTI triggering.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] ethertype                - Ethernet type
* @param[in] ethertype                - Ethernet type value (range 16 bits)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note If there are 2 registers used for ethertype configuration,
*       one for ethertype identification of incoming tunneled packets in TTI
*       and one for setting the ethertype for outgoing packets in tunnel start
*       header alteration, these registers are configured to have the same value.
*
*/
GT_STATUS cpssDxChTtiEthernetTypeSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_TUNNEL_ETHERTYPE_TYPE_ENT      ethertypeType,
    IN  GT_U32                              ethertype
);

/**
* @internal cpssDxChTtiEthernetTypeGet function
* @endinternal
*
* @brief   This function gets the TTI Ethernet type value that is used
*         to identify packets for TTI triggering.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] ethertypeType            - Ethernet type
*
* @param[out] ethertypePtr             - Points to Ethernet type value (range 16 bits)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note If there are 2 registers used for ethertype configuration,
*       one for ethertype identification of incoming tunneled packets in TTI
*       and one for setting the ethertype for outgoing packets in tunnel start
*       header alteration, these registers are configured to have the same value.
*
*/
GT_STATUS cpssDxChTtiEthernetTypeGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_TUNNEL_ETHERTYPE_TYPE_ENT      ethertypeType,
    OUT GT_U32                              *ethertypePtr
);

/**
* @internal cpssDxChTtiTrillCpuCodeBaseSet function
* @endinternal
*
* @brief   This function sets the Trill cpu code base.
*         TRILL CPU codes are relative to global configurable CPU code base value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] cpuCodeBase              - The base CPU code value for the TRILL engine .(APPLICABLE RANGES: 192..244)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
GT_STATUS cpssDxChTtiTrillCpuCodeBaseSet
(
    IN  GT_U8           devNum,
    IN  GT_U32          cpuCodeBase
);

/**
* @internal cpssDxChTtiTrillCpuCodeBaseGet function
* @endinternal
*
* @brief   This function sets the Trill cpu code base.
*         TRILL CPU codes are relative to global configurable CPU code base value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] cpuCodeBasePtr           - Points to the base CPU code value for the TRILL engine
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
GT_STATUS cpssDxChTtiTrillCpuCodeBaseGet
(
    IN  GT_U8           devNum,
    OUT GT_U32          *cpuCodeBasePtr
);

/**
* @internal cpssDxChTtiTrillAdjacencyCheckEntrySet function
* @endinternal
*
* @brief   This function sets entry in the TRILL Adjacency Check dedicated TCAM.
*         A TCAM lookup is performed for every TRILL packet processed by the TRILL engine.
*         The TRILL engine uses a single TCAM lookup to implement
*         the following TRILL adjacency checks:
*         1. TRILL IS-IS Adjacency check -
*         Checks that the single-destination TRILL frame arrives from a
*         (neighbor, port) for which an IS-IS adjacency exists.
*         2. TRILL Tree Adjacency Check -
*         Checks that the multi-destination TRILL frame arrives from a
*         (neighbor, port) that is a branch on the given TRILL distribution tree.
*         If there is TCAM MISS, invoke the respective UC or Multi-Target exception command.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] entryIndex               - entry index in TRILL Adjacency TCAM .(APPLICABLE RANGES: 0..255)
* @param[in] ttiTrillAdjacencyPtr     - points to TRILL Adjacency Entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter's value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiTrillAdjacencyCheckEntrySet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              entryIndex,
    IN  CPSS_DXCH_TTI_TRILL_ADJACENCY_STC   *ttiTrillAdjacencyPtr
);

/**
* @internal cpssDxChTtiTrillAdjacencyCheckEntryGet function
* @endinternal
*
* @brief   This function gets entry in the TRILL Adjacency Check dedicated TCAM.
*         A TCAM lookup is performed for every TRILL packet processed by the TRILL engine.
*         The TRILL engine uses a single TCAM lookup to implement
*         the following TRILL adjacency checks:
*         1. TRILL IS-IS Adjacency check -
*         Checks that the single-destination TRILL frame arrives from a
*         (neighbor, port) for which an IS-IS adjacency exists.
*         2. TRILL Tree Adjacency Check -
*         Checks that the multi-destination TRILL frame arrives from a
*         (neighbor, port) that is a branch on the given TRILL distribution tree.
*         If there is TCAM MISS, invoke the respective UC or Multi-Target exception command.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] entryIndex               - entry index in TRILL Adjacency TCAM .(APPLICABLE RANGES: 0..255)
*
* @param[out] ttiTrillAdjacencyPtr     - points to TRILL Adjacency Entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter's value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiTrillAdjacencyCheckEntryGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              entryIndex,
    OUT CPSS_DXCH_TTI_TRILL_ADJACENCY_STC   *ttiTrillAdjacencyPtr
);

/**
* @internal cpssDxChTtiTrillMaxVersionSet function
* @endinternal
*
* @brief   This function sets the max Trill version.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] maxTrillVersion          - max TRILL version value (range 2 bits)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiTrillMaxVersionSet
(
    IN  GT_U8           devNum,
    IN  GT_U32          maxTrillVersion
);

/**
* @internal cpssDxChTtiTrillMaxVersionGet function
* @endinternal
*
* @brief   This function gets the max Trill version.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] maxTrillVersionPtr       - Points to max TRILL version value (range 2 bits)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiTrillMaxVersionGet
(
    IN  GT_U8           devNum,
    OUT GT_U32          *maxTrillVersionPtr
);

/**
* @internal cpssDxChTtiPortTrillEnableSet function
* @endinternal
*
* @brief   This function enables/disables the TRILL engine on the port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   - GT_TRUE:  TRILL engine on port
*                                      GT_FALSE: disable TRILL engine on port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Configure TRILL Interface Enable bit for Egress ePort in table HA Egress ePort Attribute Table 1
*       Configure TRILL Engine Enable bit for Ingress ePort in table Pre-TTI Lookup Ingress ePort Table
*
*/
GT_STATUS cpssDxChTtiPortTrillEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  GT_BOOL                             enable
);

/**
* @internal cpssDxChTtiPortTrillEnableGet function
* @endinternal
*
* @brief   This function gets the current state enables/disables of TRILL engine
*         on the port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
*
* @param[out] enablePtr                - points to enable/disable TRILL
*                                      GT_TRUE:  TRILL engine enabled on port
*                                      GT_FALSE: TRILL engine disabled on port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Same value is configured for Egress ePort in table HA Egress ePort Attribute Table 1
*       and Ingress ePort in table Pre-TTI Lookup Ingress ePort Table
*
*/
GT_STATUS cpssDxChTtiPortTrillEnableGet
(
    IN  GT_U8               devNum,
    IN  GT_PORT_NUM         portNum,
    OUT GT_BOOL             *enablePtr
);

/**
* @internal cpssDxChTtiPortTrillOuterVid0Set function
* @endinternal
*
* @brief   This function sets the Outer Tag0 VID that must be for all TRILL packets
*         from the port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] vlanId                   - TRILL Outer Tag0 VID .(APPLICABLE RANGES: 0..4095)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPortTrillOuterVid0Set
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    IN  GT_U16          vlanId
);

/**
* @internal cpssDxChTtiPortTrillOuterVid0Get function
* @endinternal
*
* @brief   This function gets the Outer Tag0 VID that must be for all TRILL packets
*         from the port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
*
* @param[out] vlanIdPtr                - (pointer to) TRILL Outer Tag0 VID
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPortTrillOuterVid0Get
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U16          *vlanIdPtr
);

/**
* @internal cpssDxChTtiPortGroupMacToMeSet function
* @endinternal
*
* @brief   This function sets a TTI MacToMe entry.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] entryIndex               - Index of mac and vlan in MacToMe table.
*                                      (APPLICABLE RANGES:
*                                      xCat3; Lion2: 0..7
*                                      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2: 0..127)
* @param[in] valuePtr                 - points to Mac To Me and Vlan To Me
* @param[in] maskPtr                  - points to mac and vlan's masks
* @param[in] interfaceInfoPtr         - points to source interface info (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id or wrong vlan/mac values
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPortGroupMacToMeSet
(
    IN  GT_U8                                           devNum,
    IN  GT_PORT_GROUPS_BMP                              portGroupsBmp,
    IN  GT_U32                                          entryIndex,
    IN  CPSS_DXCH_TTI_MAC_VLAN_STC                      *valuePtr,
    IN  CPSS_DXCH_TTI_MAC_VLAN_STC                      *maskPtr,
    IN  CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC  *interfaceInfoPtr
);

/**
* @internal cpssDxChTtiPortGroupMacToMeGet function
* @endinternal
*
* @brief   This function gets a TTI MacToMe entry.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] entryIndex               - Index of mac and vlan in MacToMe table.
*                                      (APPLICABLE RANGES:
*                                      xCat3; Lion2: 0..7
*                                      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2: 0..127)
*
* @param[out] valuePtr                 - points to Mac To Me and Vlan To Me
* @param[out] maskPtr                  - points to mac and vlan's masks
* @param[out] interfaceInfoPtr         - points to source interface info (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id or wrong vlan/mac values
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPortGroupMacToMeGet
(
    IN  GT_U8                                           devNum,
    IN  GT_PORT_GROUPS_BMP                              portGroupsBmp,
    IN  GT_U32                                          entryIndex,
    OUT CPSS_DXCH_TTI_MAC_VLAN_STC                      *valuePtr,
    OUT CPSS_DXCH_TTI_MAC_VLAN_STC                      *maskPtr,
    OUT CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC  *interfaceInfoPtr
);

/**
* @internal cpssDxChTtiPortGroupMacModeSet function
* @endinternal
*
* @brief   This function sets the lookup Mac mode for the specified key type.
*         This setting controls the Mac that would be used for key generation
*         (Source/Destination).
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] keyType                  - TTI key type; valid values:
*                                      CPSS_DXCH_TTI_KEY_IPV4_E
*                                      CPSS_DXCH_TTI_KEY_MPLS_E
*                                      CPSS_DXCH_TTI_KEY_ETH_E
*                                      CPSS_DXCH_TTI_KEY_MIM_E  (APPLICABLE DEVICES xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
* @param[in] macMode                  - MAC mode to use; valid values:
*                                      CPSS_DXCH_TTI_MAC_MODE_DA_E
*                                      CPSS_DXCH_TTI_MAC_MODE_SA_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Not needed for TRILL key
*
*/
GT_STATUS cpssDxChTtiPortGroupMacModeSet
(
    IN  GT_U8                             devNum,
    IN  GT_PORT_GROUPS_BMP                portGroupsBmp,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT        keyType,
    IN  CPSS_DXCH_TTI_MAC_MODE_ENT        macMode
);

/**
* @internal cpssDxChTtiPortGroupMacModeGet function
* @endinternal
*
* @brief   This function gets the lookup Mac mode for the specified key type.
*         This setting controls the Mac that would be used for key generation
*         (Source/Destination).
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] keyType                  - TTI key type; valid values:
*                                      CPSS_DXCH_TTI_KEY_IPV4_E
*                                      CPSS_DXCH_TTI_KEY_MPLS_E
*                                      CPSS_DXCH_TTI_KEY_ETH_E
*                                      CPSS_DXCH_TTI_KEY_MIM_E  (APPLICABLE DEVICES xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*
* @param[out] macModePtr               - MAC mode to use; valid values:
*                                      CPSS_DXCH_TTI_MAC_MODE_DA_E
*                                      CPSS_DXCH_TTI_MAC_MODE_SA_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id or key type
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Not needed for TRILL key
*
*/
GT_STATUS cpssDxChTtiPortGroupMacModeGet
(
    IN  GT_U8                             devNum,
    IN  GT_PORT_GROUPS_BMP                portGroupsBmp,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT        keyType,
    OUT CPSS_DXCH_TTI_MAC_MODE_ENT        *macModePtr
);

/**
* @internal cpssDxChTtiPortGroupRuleSet function
* @endinternal
*
* @brief   This function sets the TTI Rule Pattern, Mask and Action for specific
*         TCAM location according to the rule Key Type.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - Global  of the rule in the TCAM
* @param[in] ruleType                 - TTI rule type
* @param[in] patternPtr               - points to the rule's pattern
* @param[in] maskPtr                  - points to the rule's mask. The rule mask is "AND STYLED
*                                      ONE". Mask bit's 0 and pattern bit's 0 means don't care bit
*                                      (corresponding bit in the pattern is not used in the TCAM lookup).
*                                      Mask bit's 0 and pattern bit's 1 means ALWAYS MISS (full entry miss).
*                                      Mask bit's 1 means that corresponding bit in the pattern
*                                      is using in the TCAM lookup.
* @param[in] actionType               - type of the action to use
* @param[in] actionPtr                - points to the TTI rule action that applied on packet
*                                      if packet's search key matched with masked pattern.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPortGroupRuleSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    IN  CPSS_DXCH_TTI_RULE_TYPE_ENT         ruleType,
    IN  CPSS_DXCH_TTI_RULE_UNT              *patternPtr,
    IN  CPSS_DXCH_TTI_RULE_UNT              *maskPtr,
    IN  CPSS_DXCH_TTI_ACTION_TYPE_ENT       actionType,
    IN  CPSS_DXCH_TTI_ACTION_UNT            *actionPtr
);

/**
* @internal cpssDxChTtiPortGroupRuleGet function
* @endinternal
*
* @brief   This function gets the TTI Rule Pattern, Mask and Action for specific
*         TCAM location according to the rule Key Type.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - Global  of the rule in the TCAM
* @param[in] ruleType                 - TTI rule type
* @param[in] actionType               - type of the action to use
*
* @param[out] patternPtr               - points to the rule's pattern
* @param[out] maskPtr                  - points to the rule's mask. The rule mask is "AND STYLED
*                                      ONE". Mask bit's 0 and pattern bit's 0 means don't care bit
*                                      (corresponding bit in the pattern is not used in the TCAM lookup).
*                                      Mask bit's 0 and pattern bit's 1 means ALWAYS MISS (full entry miss).
*                                      Mask bit's 1 means that corresponding bit in the pattern
*                                      is using in the TCAM lookup.
* @param[out] actionPtr                - points to the TTI rule action that applied on packet
*                                      if packet's search key matched with masked pattern.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_STATE             - a rule of different type was found in the specified index
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPortGroupRuleGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    IN  CPSS_DXCH_TTI_RULE_TYPE_ENT         ruleType,
    OUT CPSS_DXCH_TTI_RULE_UNT              *patternPtr,
    OUT CPSS_DXCH_TTI_RULE_UNT              *maskPtr,
    IN  CPSS_DXCH_TTI_ACTION_TYPE_ENT       actionType,
    OUT CPSS_DXCH_TTI_ACTION_UNT            *actionPtr
);

/**
* @internal cpssDxChTtiPortGroupRuleActionUpdate function
* @endinternal
*
* @brief   This function updates rule action.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - Index of the rule in the TCAM
* @param[in] actionType               - type of the action to use
* @param[in] actionPtr                - points to the TTI rule action that applied on packet
*                                      if packet's search key matched with masked pattern.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPortGroupRuleActionUpdate
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    IN  CPSS_DXCH_TTI_ACTION_TYPE_ENT       actionType,
    IN  CPSS_DXCH_TTI_ACTION_UNT            *actionPtr
);

/**
* @internal cpssDxChTtiPortGroupRuleValidStatusSet function
* @endinternal
*
* @brief   This function validates / invalidates the rule in TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      port group(s). If a bit of non  port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - Global  of the rule in the TCAM
* @param[in] valid                    - GT_TRUE - valid, GT_FALSE - invalid
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note If valid == GT_TRUE it is assumed that the TCAM entry already contains
*       the TTI entry information.
*
*/
GT_STATUS cpssDxChTtiPortGroupRuleValidStatusSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    IN  GT_BOOL                             valid
);

/**
* @internal cpssDxChTtiPortGroupRuleValidStatusGet function
* @endinternal
*
* @brief   This function returns the valid status of the rule in TCAM
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - Global  of the rule in the TCAM
*
* @param[out] validPtr                 - GT_TRUE - valid, GT_FALSE - invalid
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPortGroupRuleValidStatusGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    OUT GT_BOOL                             *validPtr
);

/**
* @internal cpssDxChTtiPortGroupUserDefinedByteSet function
* @endinternal
*
* @brief   The function configures the User Defined Byte (UDB)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] devNum                   - device number
* @param[in] keyType                  - key Type; valid values:
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_MPLS_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_FRAGMENT_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_OTHER_E
*                                      CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV6_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV6_TCP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV6_UDP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE0_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE1_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE2_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE3_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE4_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE5_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE6_E
* @param[in] udbIndex                 - index of User Defined Byte to configure.(APPLICABLE RANGES: 0..29)
* @param[in] offsetType               - the type of offset
* @param[in] offset                   - The  of the user-defined byte, in bytes,from the
*                                      place in the packet indicated by the offset type. .(APPLICABLE RANGES: 0..127)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChTtiPortGroupUserDefinedByteSet
(
    IN GT_U8                                devNum,
    IN GT_PORT_GROUPS_BMP                   portGroupsBmp,
    IN CPSS_DXCH_TTI_KEY_TYPE_ENT           keyType,
    IN GT_U32                               udbIndex,
    IN CPSS_DXCH_TTI_OFFSET_TYPE_ENT        offsetType,
    IN GT_U8                                offset
);

/**
* @internal cpssDxChTtiPortGroupUserDefinedByteGet function
* @endinternal
*
* @brief   The function gets the User Defined Byte (UDB) configuration
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] keyType                  - key Type; valid values:
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_MPLS_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_FRAGMENT_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_OTHER_E
*                                      CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV6_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV6_TCP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV6_UDP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE0_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE1_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE2_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE3_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE4_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE5_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE6_E
* @param[in] udbIndex                 - index of User Defined Byte to configure.(APPLICABLE RANGES: 0..29)
*
* @param[out] offsetTypePtr            - (pointer to) The type of offset
* @param[out] offsetPtr                - (pointer to) The offset of the user-defined byte,
*                                      in bytes,from the place in the packet
*                                      indicated by the offset type.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChTtiPortGroupUserDefinedByteGet
(
    IN  GT_U8                                devNum,
    IN  GT_PORT_GROUPS_BMP                   portGroupsBmp,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT           keyType,
    IN  GT_U32                               udbIndex,
    OUT CPSS_DXCH_TTI_OFFSET_TYPE_ENT       *offsetTypePtr,
    OUT GT_U8                               *offsetPtr
);


/**
* @internal cpssDxChTtiPortPassengerOuterIsTag0Or1Set function
* @endinternal
*
* @brief   Set per port TTI passenger Outer Tag is Tag0 or Tag1.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] mode                     - recognize Outer Tag existence according to TPID1 or TPID0
*                                      CPSS_VLAN_ETHERTYPE0_E: Outer Tag exists if passenger outer VLAN Tag TPID = Tag0 TPID
*                                      CPSS_VLAN_ETHERTYPE1_E: Outer Tag exists if passenger outer VLAN tag TPID = Tag1 TPID
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, port , mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPortPassengerOuterIsTag0Or1Set
(
    IN  GT_U8                        devNum,
    IN  GT_PORT_NUM                  portNum,
    IN  CPSS_ETHER_MODE_ENT          mode
);

/**
* @internal cpssDxChTtiPortPassengerOuterIsTag0Or1Get function
* @endinternal
*
* @brief   Set per port TTI passenger Outer Tag is Tag0 or Tag1.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
*
* @param[out] modePtr                  - (pointer to)recognize Outer Tag existence according to TPID1 or TPID0
*                                      CPSS_VLAN_ETHERTYPE0_E: Outer Tag exists if passenger outer VLAN Tag TPID = Tag0 TPID
*                                      CPSS_VLAN_ETHERTYPE1_E: Outer Tag exists if passenger outer VLAN tag TPID = Tag1 TPID
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, port
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPortPassengerOuterIsTag0Or1Get
(
    IN  GT_U8                        devNum,
    IN  GT_PORT_NUM                  portNum,
    OUT  CPSS_ETHER_MODE_ENT          *modePtr
);

/**
* @internal cpssDxChTtiUserDefinedByteSet function
* @endinternal
*
* @brief   The function configures the User Defined Byte (UDB)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] keyType                  - key Type; valid values:
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_MPLS_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_FRAGMENT_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_OTHER_E
*                                      CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV6_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV6_TCP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV6_UDP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE0_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE1_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE2_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE3_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE4_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE5_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE6_E
* @param[in] udbIndex                 - index of User Defined Byte to configure.(APPLICABLE RANGES: 0..29)
* @param[in] offsetType               - the type of offset
* @param[in] offset                   - The  of the user-defined byte, in bytes,from the
*                                      place in the packet indicated by the offset type.(APPLICABLE RANGES: 0..127)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChTtiUserDefinedByteSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_TTI_KEY_TYPE_ENT           keyType,
    IN GT_U32                               udbIndex,
    IN CPSS_DXCH_TTI_OFFSET_TYPE_ENT        offsetType,
    IN GT_U8                                offset
);

/**
* @internal cpssDxChTtiUserDefinedByteGet function
* @endinternal
*
* @brief   The function gets the User Defined Byte (UDB) configuration
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] keyType                  - key Type; valid values:
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_MPLS_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_FRAGMENT_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_OTHER_E
*                                      CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV6_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV6_TCP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV6_UDP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE0_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE1_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE2_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE3_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE4_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE5_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE6_E
* @param[in] udbIndex                 - index of User Defined Byte to configure.(APPLICABLE RANGES: 0..29)
*
* @param[out] offsetTypePtr            - (pointer to) The type of offset
* @param[out] offsetPtr                - (pointer to) The offset of the user-defined byte,
*                                      in bytes,from the place in the packet
*                                      indicated by the offset type.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChTtiUserDefinedByteGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT          keyType,
    IN  GT_U32                              udbIndex,
    OUT CPSS_DXCH_TTI_OFFSET_TYPE_ENT       *offsetTypePtr,
    OUT GT_U8                               *offsetPtr
);

/**
* @internal cpssDxChTtiPacketTypeKeySizeSet function
* @endinternal
*
* @brief   function sets key type size.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] keyType                  - key type; valid values:
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_MPLS_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_FRAGMENT_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_OTHER_E
*                                      CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV6_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV6_TCP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV6_UDP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE0_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE1_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE2_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE3_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE4_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE5_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE6_E
* @param[in] size                     - key  in TCAM
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPacketTypeKeySizeSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_TTI_KEY_TYPE_ENT           keyType,
    IN CPSS_DXCH_TTI_KEY_SIZE_ENT           size
);

/**
* @internal cpssDxChTtiPacketTypeKeySizeGet function
* @endinternal
*
* @brief   function gets key type size.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] keyType                  - key type; valid values:
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_MPLS_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_FRAGMENT_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV4_OTHER_E
*                                      CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV6_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV6_TCP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_IPV6_UDP_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE0_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE1_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE2_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE3_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE4_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE5_E
*                                      CPSS_DXCH_TTI_KEY_UDB_UDE6_E
*
* @param[out] sizePtr                  - points to key size in TCAM
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPacketTypeKeySizeGet
(
    IN  GT_U8                                devNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT           keyType,
    OUT CPSS_DXCH_TTI_KEY_SIZE_ENT           *sizePtr
);

/**
* @internal cpssDxChTtiTcamSegmentModeSet function
* @endinternal
*
* @brief   Sets a TTI TCAM segment mode for a specific key type
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] keyType                  - key type
* @param[in] segmentMode              - TTI TCAM segment mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiTcamSegmentModeSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_TTI_KEY_TYPE_ENT               keyType,
    IN CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_ENT  segmentMode
);

/**
* @internal cpssDxChTtiTcamSegmentModeGet function
* @endinternal
*
* @brief   Gets a TTI TCAM segment mode for a specific key type
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] keyType                  - key type
*
* @param[out] segmentModePtr           - (pointer to) TTI TCAM segment mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - on invalid HW value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiTcamSegmentModeGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT              keyType,
    OUT CPSS_DXCH_TTI_KEY_TCAM_SEGMENT_MODE_ENT *segmentModePtr
);

/**
* @internal cpssDxChTtiMcTunnelDuplicationModeSet function
* @endinternal
*
* @brief   Set the tunnel duplication mode for a specific protocol on a specific
*         port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] protocol                 - the protocol
* @param[in] mode                     - the tunnel duplication mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiMcTunnelDuplicationModeSet
(
    IN GT_U8                                            devNum,
    IN GT_PORT_NUM                                      portNum,
    IN CPSS_TUNNEL_MULTICAST_TYPE_ENT                   protocol,
    IN CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENT     mode
);

/**
* @internal cpssDxChTtiMcTunnelDuplicationModeGet function
* @endinternal
*
* @brief   Get the tunnel duplication mode for a specific protocol on a specific
*         port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] protocol                 - the protocol
*
* @param[out] modePtr                  - (pointer to) the tunnel duplication mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiMcTunnelDuplicationModeGet
(
    IN GT_U8                                            devNum,
    IN GT_PORT_NUM                                      portNum,
    IN  CPSS_TUNNEL_MULTICAST_TYPE_ENT                  protocol,
    OUT CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENT    *modePtr
);

/**
* @internal cpssDxChTtiMcTunnelDuplicationUdpDestPortSet function
* @endinternal
*
* @brief   Set the UDP destination port used to trigger IPv4/IPv6 Multicast
*         replication
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] udpPort                  - the UDP destination port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This configuration is relevant only when default ePort
*       <IPv4 MC Duplication IP Protocol Mode> = UDP or Default ePort
*       <IPv6 MC Duplication IP Protocol Mode> = UDP
*
*/
GT_STATUS cpssDxChTtiMcTunnelDuplicationUdpDestPortSet
(
    IN GT_U8    devNum,
    IN GT_U16   udpPort
);

/**
* @internal cpssDxChTtiMcTunnelDuplicationUdpDestPortGet function
* @endinternal
*
* @brief   Get the UDP destination port used to trigger IPv4/IPv6 Multicast
*         replication
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] udpPortPtr               - (pointer to) the UDP destination port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This configuration is relevant only when default ePort
*       <IPv4 MC Duplication IP Protocol Mode> = UDP or Default ePort
*       <IPv6 MC Duplication IP Protocol Mode> = UDP
*
*/
GT_STATUS cpssDxChTtiMcTunnelDuplicationUdpDestPortGet
(
    IN GT_U8    devNum,
    OUT GT_U16  *udpPortPtr
);

/**
* @internal cpssDxChTtiMplsMcTunnelTriggeringMacDaSet function
* @endinternal
*
* @brief   Set the MPLS multicast MAC DA and mask used for duplication triggering
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] addressPtr               - (pointer to) the MAC DA
* @param[in] maskPtr                  - (pointer to) the address mask
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiMplsMcTunnelTriggeringMacDaSet
(
    IN GT_U8            devNum,
    IN GT_ETHERADDR     *addressPtr,
    IN GT_ETHERADDR     *maskPtr
);

/**
* @internal cpssDxChTtiMplsMcTunnelTriggeringMacDaGet function
* @endinternal
*
* @brief   Get the MPLS multicast MAC DA and mask used for duplication triggering
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] addressPtr               - (pointer to) the MAC DA
* @param[out] maskPtr                  - (pointer to) the address mask
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiMplsMcTunnelTriggeringMacDaGet
(
    IN GT_U8            devNum,
    OUT GT_ETHERADDR    *addressPtr,
    OUT GT_ETHERADDR    *maskPtr
);

/**
* @internal cpssDxChTtiPwCwExceptionCmdSet function
* @endinternal
*
* @brief   Set a PW CW exception command
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number
* @param[in] exceptionType            - exception type
* @param[in] command                  - the packet command
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported mode
*/
GT_STATUS cpssDxChTtiPwCwExceptionCmdSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_TTI_PW_CW_EXCEPTION_ENT    exceptionType,
    IN CPSS_PACKET_CMD_ENT                  command
);

/**
* @internal cpssDxChTtiPwCwExceptionCmdGet function
* @endinternal
*
* @brief   Get a PW CW exception command
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] exceptionType            - exception type
*
* @param[out] commandPtr               - (pointer to) the packet command
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported mode
*/
GT_STATUS cpssDxChTtiPwCwExceptionCmdGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_PW_CW_EXCEPTION_ENT   exceptionType,
    OUT CPSS_PACKET_CMD_ENT                 *commandPtr
);

/**
* @internal cpssDxChTtiPwCwCpuCodeBaseSet function
* @endinternal
*
* @brief   Set the base CPU code value for PWE3
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] cpuCodeBase              - the base CPU code value for PWE3
*                                      (APPLICABLE RANGES: 192..251)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPwCwCpuCodeBaseSet
(
    IN GT_U8    devNum,
    IN GT_U32   cpuCodeBase
);

/**
* @internal cpssDxChTtiPwCwCpuCodeBaseGet function
* @endinternal
*
* @brief   Get the base CPU code value for PWE3
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] cpuCodeBasePtr           - (pointer to) the base CPU code value for PWE3
*                                      (APPLICABLE RANGES: 192..255)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPwCwCpuCodeBaseGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *cpuCodeBasePtr
);

/**
* @internal cpssDxChTtiPwCwSequencingSupportEnableSet function
* @endinternal
*
* @brief   This function enables/disables the check for Pseudo Wire-Control Word
*         Data Word format <Sequence Number>==0 in terminated Pseudo Wires.
*         < Sequence Number > are the 16 LSB of "CW as Data Word format".
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE: PW-CW sequencing supported
*                                      GT_FALSE: PW-CW sequencing is not supported
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPwCwSequencingSupportEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     enable
);

/**
* @internal cpssDxChTtiPwCwSequencingSupportEnableGet function
* @endinternal
*
* @brief   This function gets the current state (enable/disable) of the check
*         for Pseudo Wire-Control Word Data Word format <Sequence Number>==0
*         in terminated Pseudo Wires.
*         < Sequence Number > are the 16 LSB of "CW as Data Word format".
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - points to enable/disable PW-CW sequencing support
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiPwCwSequencingSupportEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
);

/**
* @internal cpssDxChTtiIPv6ExtensionHeaderSet function
* @endinternal
*
* @brief   Set one of the 2 configurable IPv6 extension headers.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] extensionHeaderId        - the configurable extension header index
*                                      (APPLICABLE RANGES: 0..1)
* @param[in] extensionHeaderValue     - the configurable extension header value
*                                      (APPLICABLE RANGES: 0..255)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The design assumes that the information following the IPv6 header is L4
*       unless there is a known next hop header. The design supports several
*       hard-coded next hop header values and two configurable ones.
*       If any of the following next hop values are identified, the <Is L4 Valid>
*       indication in the IPCL key is set to NOT VALID:
*       - HBH (0)
*       - IPv6 Routing header (43)
*       - IPv6 Fragment header (44)
*       - Encapsulation Security Payload (50)
*       - Authentication Header (51)
*       - IPv6 Destination Options (60)
*       - Mobility Header (135)
*       - <IPv6 Extension Value0>
*       - <IPv6 Extension Value1>
*       To disable a configurable header, set its value to one of the above
*       values.
*
*/
GT_STATUS cpssDxChTtiIPv6ExtensionHeaderSet
(
    IN GT_U8    devNum,
    IN GT_U32   extensionHeaderId,
    IN GT_U32   extensionHeaderValue
);

/**
* @internal cpssDxChTtiIPv6ExtensionHeaderGet function
* @endinternal
*
* @brief   Get one of the 2 configurable IPv6 extension headers.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] extensionHeaderId        - the configurable extension header index
*                                      (APPLICABLE RANGES: 0..1)
*
* @param[out] extensionHeaderValuePtr  - the configurable extension header value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The design assumes that the information following the IPv6 header is L4
*       unless there is a known next hop header. The design supports several
*       hard-coded next hop header values and two configurable ones.
*       If any of the following next hop values are identified, the <Is L4 Valid>
*       indication in the IPCL key is set to NOT VALID:
*       - HBH (0)
*       - IPv6 Routing header (43)
*       - IPv6 Fragment header (44)
*       - Encapsulation Security Payload (50)
*       - Authentication Header (51)
*       - IPv6 Destination Options (60)
*       - Mobility Header (135)
*       - <IPv6 Extension Value0>
*       - <IPv6 Extension Value1>
*
*/
GT_STATUS cpssDxChTtiIPv6ExtensionHeaderGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  extensionHeaderId,
    OUT GT_U32  *extensionHeaderValuePtr
);

/**
* @internal cpssDxChTtiSourceIdBitsOverrideSet function
* @endinternal
*
* @brief   Set the SrcID bits that are overridden by the TTI action of SrcID
*         assignment.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] lookup                   - the  number (APPLICABLE RANGES: 0..3)
* @param[in] overrideBitmap           - 12 bits bitmap. For each bit:
*                                      0 - do not override
*                                      1 - override
*                                      (APPLICABLE RANGES: 0..4095)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiSourceIdBitsOverrideSet
(
    IN GT_U8    devNum,
    IN GT_U32   lookup,
    IN GT_U32   overrideBitmap
);

/**
* @internal cpssDxChTtiSourceIdBitsOverrideGet function
* @endinternal
*
* @brief   Get the SrcID bits that are overridden by the TTI action of SrcID
*         assignment.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] lookup                   - the  number (APPLICABLE RANGES: 0..3)
*
* @param[out] overrideBitmapPtr        - 12 bits bitmap. For each bit:
*                                      0 - do not override
*                                      1 - override
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiSourceIdBitsOverrideGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  lookup,
    OUT GT_U32  *overrideBitmapPtr
);

/**
* @internal cpssDxChTtiGreExtensionsEnableSet function
* @endinternal
*
* @brief   Enable/Disable parsing of extensions (Checksum, Sequence, Key) on
*         IPv4/IPv6 GRE tunnels.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - enable/disable GRE extensions
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiGreExtensionsEnableSet
(
    IN GT_U8        devNum,
    IN GT_BOOL      enable
);

/**
* @internal cpssDxChTtiGreExtensionsEnableGet function
* @endinternal
*
* @brief   Gt the enabling status of the parsing of extensions (Checksum, Sequence,
*         Key) on IPv4/IPv6 GRE tunnels.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) GRE extensions enabling status
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiGreExtensionsEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
);

/**
* @internal cpssDxChTtiFcoeForwardingEnableSet function
* @endinternal
*
* @brief   When enabled, if the packet matches the "FCoE Ethertype", it is parsed as
*         FCoE including S_ID and D_ID, and can be forwarded (FCF) in the Router Engine
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - enable/disable FCoE Forwarding
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiFcoeForwardingEnableSet
(
    IN GT_U8        devNum,
    IN GT_BOOL      enable
);

/**
* @internal cpssDxChTtiFcoeForwardingEnableGet function
* @endinternal
*
* @brief   Return if a packet that matches the "FCoE Ethertype", is parsed as
*         FCoE including S_ID and D_ID, and can be forwarded (FCF) in the Router Engine
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) enabling status
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiFcoeForwardingEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
);

/**
* @internal cpssDxChTtiFcoeEtherTypeSet function
* @endinternal
*
* @brief   Set the EtherType of FCoE packets
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] etherType                - EtherType of FCoE packets
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiFcoeEtherTypeSet
(
    IN GT_U8        devNum,
    IN GT_U16       etherType
);

/**
* @internal cpssDxChTtiFcoeEtherTypeGet function
* @endinternal
*
* @brief   Return the EtherType of FCoE packets
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] etherTypePtr             - (pointer to) EtherType of FCoE packets
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiFcoeEtherTypeGet
(
    IN  GT_U8       devNum,
    OUT GT_U16      *etherTypePtr
);

/**
* @internal cpssDxChTtiFcoeExceptionConfiguratiosEnableSet function
* @endinternal
*
* @brief   Enable FCoE Exception Configuration.
*         A packet triggered for FCoE Forwarding generates an FCoE exception if
*         the exception conditions are TRUE and the exception configuration is enabled.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] exceptionType            - FCoE Exception type to enable/disable
* @param[in] enable                   - enable/disable FCoE Exception
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiFcoeExceptionConfiguratiosEnableSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_TTI_FCOE_EXCEPTION_ENT     exceptionType,
    IN GT_BOOL                              enable
);

/**
* @internal cpssDxChTtiFcoeExceptionConfiguratiosEnableGet function
* @endinternal
*
* @brief   Return FCoE Exception Configuration status
*         A packet triggered for FCoE Forwarding generates an FCoE exception if
*         the exception conditions are TRUE and the exception configuration is enabled.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] exceptionType            - FCoE Exception type to enable/disable
*
* @param[out] enablePtr                - (pointer to) enabling status
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiFcoeExceptionConfiguratiosEnableGet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_TTI_FCOE_EXCEPTION_ENT     exceptionType,
    OUT GT_BOOL                             *enablePtr
);

/**
* @internal cpssDxChTtiFcoeExceptionCountersGet function
* @endinternal
*
* @brief   Return number of FCoE exceptions of a given type.
*         When the counter reaches 0xFF it sticks to this value, i.e., it does not wrap around
*         The counter is clear on read
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] exceptionType            - FCoE Exception type to enable/disable
*
* @param[out] counterValuePtr          - (pointer to) number of FCoE exceptions
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiFcoeExceptionCountersGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_FCOE_EXCEPTION_ENT    exceptionType,
    OUT GT_U32                             *counterValuePtr
);

/**
* @internal cpssDxChTtiFcoeAssignVfIdEnableSet function
* @endinternal
*
* @brief   When enabled, if the packet is FCoE and it contains a VF Tag, then the
*         VRF-ID is assigned with the value VF_ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   - enable/disable assignment of VRF-ID with the value VF_ID-ID with the value VF_ID
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiFcoeAssignVfIdEnableSet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
);

/**
* @internal cpssDxChTtiFcoeAssignVfIdEnableGet function
* @endinternal
*
* @brief   Return if VRF-ID is assigned with the value VF_ID
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
*
* @param[out] enablePtr                - (pointer to) enabling status
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant for FCoE packets which contain a VF Tag
*
*/
GT_STATUS cpssDxChTtiFcoeAssignVfIdEnableGet
(
    IN  GT_U8       devNum,
    IN GT_PORT_NUM  portNum,
    OUT GT_BOOL     *enablePtr
);

/**
* @internal cpssDxChTtiFcoeExceptionPacketCommandSet function
* @endinternal
*
* @brief   Set packet command of invalid FCoE packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] command                  -  to set
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The commands are:
*       CPSS_PACKET_CMD_FORWARD_E
*       CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*       CPSS_PACKET_CMD_TRAP_TO_CPU_E
*       CPSS_PACKET_CMD_DROP_HARD_E
*       CPSS_PACKET_CMD_DROP_SOFT_E
*
*/
GT_STATUS cpssDxChTtiFcoeExceptionPacketCommandSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_PACKET_CMD_ENT                 command
);

/**
* @internal cpssDxChTtiFcoeExceptionPacketCommandGet function
* @endinternal
*
* @brief   Get packet command of invalid FCoE packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - physical device number
*
* @param[out] commandPtr               - points to the command for invalid FCoE packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The commands are:
*       CPSS_PACKET_CMD_FORWARD_E
*       CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*       CPSS_PACKET_CMD_TRAP_TO_CPU_E
*       CPSS_PACKET_CMD_DROP_HARD_E
*       CPSS_PACKET_CMD_DROP_SOFT_E
*
*/
GT_STATUS cpssDxChTtiFcoeExceptionPacketCommandGet
(
    IN  GT_U8                               devNum,
    OUT CPSS_PACKET_CMD_ENT                 *commandPtr
);

/**
* @internal cpssDxChTtiFcoeExceptionCpuCodeSet function
* @endinternal
*
* @brief   Set FCoE Exception CPU Code.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] cpuCode                  - CPU code
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiFcoeExceptionCpuCodeSet
(
    IN GT_U8 devNum,
    IN CPSS_NET_RX_CPU_CODE_ENT cpuCode
);

/**
* @internal cpssDxChTtiFcoeExceptionCpuCodeGet function
* @endinternal
*
* @brief   Get FCoE Exception CPU Code.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - physical device number
*
* @param[out] cpuCodePtr               - points to the CPU code
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiFcoeExceptionCpuCodeGet
(
    IN  GT_U8                               devNum,
    OUT CPSS_NET_RX_CPU_CODE_ENT            *cpuCodePtr
);

/**
* @internal cpssDxChTtiEcnPacketCommandSet function
* @endinternal
*
* @brief   Set packet command assigned to tunnel terminated packets that have
*           an ECN indication (Congestion Experienced) in tunnel header, and
*           Not-ECT in the passenger header.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] command               - packet command.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device or packet command.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiEcnPacketCommandSet
(
    IN GT_U8                devNum,
    IN CPSS_PACKET_CMD_ENT  command
);

/**
* @internal cpssDxChTtiEcnPacketCommandGet function
* @endinternal
*
* @brief   Get packet command assigned to tunnel terminated packets that have
*           an ECN indication (Congestion Experienced) in tunnel header, and
*           Not-ECT in the passenger header.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
*
* @param[out] commandPtr           - (pointer to) packet command.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device.
* @retval GT_HW_ERROR              - on HW error.
* @retval GT_BAD_VALUE             - on bad packet command value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiEcnPacketCommandGet
(
    IN GT_U8                devNum,
    IN CPSS_PACKET_CMD_ENT  *commandPtr
);

/**
* @internal cpssDxChTtiEcnCpuCodeSet function
* @endinternal
*
* @brief   Set the CPU/Drop Code assigned to the packet if the packet
*          command is drop, trap or mirror to CPU.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] cpuCode               - CPU code.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device or CPU code.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiEcnCpuCodeSet
(
    IN GT_U8                     devNum,
    IN CPSS_NET_RX_CPU_CODE_ENT  cpuCode
);

/**
* @internal cpssDxChTtiEcnCpuCodeGet function
* @endinternal
*
* @brief   Get the CPU/Drop Code assigned to the packet if the packet
*          command is drop, trap or mirror to CPU.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
*
* @param[out] cpuCodePtr           - (pointer to) CPU code.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTtiEcnCpuCodeGet
(
    IN GT_U8                      devNum,
    IN CPSS_NET_RX_CPU_CODE_ENT   *cpuCodePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChTtih */

