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
* @file cpssDxChBrgSecurityBreach.h
*
* @brief Implementation for CPSS DxCh Bridge Security Breach Functionality.
*
* @version   8
********************************************************************************
*/
#ifndef __cpssDxChBrgSecurityBreachh
#define __cpssDxChBrgSecurityBreachh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/bridge/cpssGenBrgSecurityBreachTypes.h>

/**
* @internal cpssDxChBrgSecurBreachPortVlanDropCntrModeSet function
* @endinternal
*
* @brief   Set Port/VLAN Security Breach Drop Counter to count security breach
*         dropped packets based on there ingress port or their assigned VID.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dev                      - physical device number
* @param[in] cntrCfgPtr               - (pointer to) security breach port/vlan counter
*                                      mode and it's configuration parameters.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - bad input parameters.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSecurBreachPortVlanDropCntrModeSet
(
    IN GT_U8                                        dev,
    IN CPSS_BRG_SECUR_BREACH_DROP_COUNT_CFG_STC     *cntrCfgPtr
);

/**
* @internal cpssDxChBrgSecurBreachPortVlanDropCntrModeGet function
* @endinternal
*
* @brief   Get mode and it's configuration parameters for security breach
*         Port/VLAN drop counter.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dev                      - physical device number
*
* @param[out] cntrCfgPtr               - (pointer to) security breach port/vlan counter
*                                      mode and it's configuration parameters.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - bad input parameters.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSecurBreachPortVlanDropCntrModeGet
(
    IN  GT_U8                                        dev,
    OUT CPSS_BRG_SECUR_BREACH_DROP_COUNT_CFG_STC     *cntrCfgPtr
);

/**
* @internal cpssDxChBrgSecurBreachGlobalDropCntrGet function
* @endinternal
*
* @brief   Get values of security breach global drop counter. Global security breach
*         counter counts all dropped packets due to security breach event.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dev                      - physical device number
*
* @param[out] counValuePtr             - (pointer to) secure breach global counter.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - bad input parameters.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSecurBreachGlobalDropCntrGet
(
    IN  GT_U8    dev,
    OUT GT_U32   *counValuePtr
);

/**
* @internal cpssDxChBrgSecurBreachPortVlanCntrGet function
* @endinternal
*
* @brief   Get value of security breach Port/Vlan drop counter. This counter counts
*         all of the packets received according to Port/Vlan security breach drop
*         count mode configuration and dropped due to any security breach event.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dev                      - physical device number
*
* @param[out] counValuePtr             - (pointer to) secure breach Port/Vlan drop counter.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - bad input parameters.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSecurBreachPortVlanCntrGet
(
    IN  GT_U8    dev,
    OUT GT_U32   *counValuePtr
);

/**
* @internal cpssDxChBrgSecurBreachNaPerPortSet function
* @endinternal
*
* @brief   Enable/Disable per port the unknown Source Addresses to be considered as
*         security breach event. When enabled, the NA message is not sent to CPU
*         and this address is not learned.
*         Only relevant in controlled learning mode.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE - New Address is security breach event
*                                      GT_FALSE - New Address is not security breach event
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - bad input parameters.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSecurBreachNaPerPortSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_BOOL              enable
);

/**
* @internal cpssDxChBrgSecurBreachNaPerPortGet function
* @endinternal
*
* @brief   Get per port the status of NA security breach event (enabled/disabled).
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - GT_TRUE - NA is security breach event
*                                      GT_FALSE - NA is not security breach event
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - bad input parameters.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSecurBreachNaPerPortGet
(
    IN  GT_U8               devNum,
    IN  GT_PORT_NUM         portNum,
    OUT GT_BOOL             *enablePtr
);

/**
* @internal cpssDxChBrgSecurBreachMovedStaticAddrSet function
* @endinternal
*
* @brief   When the FDB entry is signed as a static one, then this entry is not
*         subject to aging. In addition, there may be cases where the interface
*         does't match the interface from which this packet was received. In this
*         case, this feature enables/disables to regard moved static adresses as
*         a security breach event.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dev                      - physical device number
* @param[in] enable                   - GT_TRUE - moved static addresses are regarded as
*                                      security breach event and are dropped.
*                                      GT_FALSE - moved static addresses are not regarded
*                                      as security breach event and are forwarded.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - bad input parameters.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSecurBreachMovedStaticAddrSet
(
    IN GT_U8                dev,
    IN GT_BOOL              enable
);

/**
* @internal cpssDxChBrgSecurBreachMovedStaticAddrGet function
* @endinternal
*
* @brief   Get if static addresses that are moved, are regarded as Security Breach
*         or not (this is only in case that the fdb entry is static or the
*         interface does't match the interface from which this packet was received).
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dev                      - physical device number
*
* @param[out] enablePtr                - GT_TRUE - moved static addresses are regarded as
*                                      security breach event and are dropped.
*                                      GT_FALSE - moved static addresses are not regarded
*                                      as security breach event and are forwarded.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - bad input parameters.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSecurBreachMovedStaticAddrGet
(
    IN  GT_U8                dev,
    OUT GT_BOOL              *enablePtr
);


/**
* @internal cpssDxChBrgSecurBreachEventDropModeSet function
* @endinternal
*
* @brief   Set for given security breach event it's drop mode.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dev                      - physical device number
* @param[in] eventType                - security breach event type.
* @param[in] dropMode                 - soft or hard drop mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - bad input parameters.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The acceptable events are:
*       CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E,
*       CPSS_BRG_SECUR_BREACH_EVENTS_PORT_NOT_IN_VLAN_E,
*       CPSS_BRG_SECUR_BREACH_EVENTS_VLAN_RANGE_DROP_E,
*       CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_VLAN_E,
*       CPSS_BRG_SECUR_BREACH_EVENTS_MOVED_STATIC_E,
*       CPSS_BRG_SECUR_BREACH_EVENTS_FRAME_TYPE,
*       (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*       CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SPOOF_PROTECTION_E,
*       (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*       CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_IS_DA_E,
*       (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*       CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_IS_ZERO_E.
*       (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*
*/
GT_STATUS cpssDxChBrgSecurBreachEventDropModeSet
(
    IN GT_U8                                dev,
    IN CPSS_BRG_SECUR_BREACH_EVENTS_ENT     eventType,
    IN CPSS_DROP_MODE_TYPE_ENT              dropMode
);

/**
* @internal cpssDxChBrgSecurBreachEventDropModeGet function
* @endinternal
*
* @brief   Get for given security breach event it's drop mode.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dev                      - physical device number
* @param[in] eventType                - security breach event type.
*
* @param[out] dropModePtr              - (pointer to) drop mode - soft or hard.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - bad input parameters.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The acceptable events are:
*       CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E,
*       CPSS_BRG_SECUR_BREACH_EVENTS_PORT_NOT_IN_VLAN_E,
*       CPSS_BRG_SECUR_BREACH_EVENTS_VLAN_RANGE_DROP_E,
*       CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_VLAN_E,
*       CPSS_BRG_SECUR_BREACH_EVENTS_MOVED_STATIC_E,
*       CPSS_BRG_SECUR_BREACH_EVENTS_FRAME_TYPE,
*       (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*       CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SPOOF_PROTECTION_E,
*       (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*       CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_IS_DA_E,
*       (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*       CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_IS_ZERO_E.
*       (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*
*/
GT_STATUS cpssDxChBrgSecurBreachEventDropModeGet
(
    IN  GT_U8                               dev,
    IN  CPSS_BRG_SECUR_BREACH_EVENTS_ENT    eventType,
    OUT CPSS_DROP_MODE_TYPE_ENT             *dropModePtr
);

/**
* @internal cpssDxChSecurBreachMsgGet function
* @endinternal
*
* @brief   Reads the Security Breach message update. When a security breach occures
*         and the CPU had read the previous message (the message is locked untill
*         CPU reads it, after it the new message can be written) the security
*         massage parameters are updated. Those parameters include: MAC address of
*         the breaching packet, security breach port, VID and security breach code.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dev                      - device number
*
* @param[out] sbMsgPtr                 - (pointer to) security breach message
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - bad input parameters.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChSecurBreachMsgGet
(
    IN  GT_U8                           dev,
    OUT CPSS_BRG_SECUR_BREACH_MSG_STC   *sbMsgPtr
);

/**
* @internal cpssDxChBrgSecurBreachPortGroupPortVlanCntrGet function
* @endinternal
*
* @brief   Get value of security breach Port/Vlan drop counter. This counter counts
*         all of the packets received according to Port/Vlan security breach drop
*         count mode configuration and dropped due to any security breach event.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dev                      - physical device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
* @param[out] counValuePtr             - (pointer to) secure breach Port/Vlan drop counter.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - bad input parameters.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSecurBreachPortGroupPortVlanCntrGet
(
    IN  GT_U8                   dev,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    OUT GT_U32                  *counValuePtr
);

/**
* @internal cpssDxChBrgSecurBreachPortGroupGlobalDropCntrGet function
* @endinternal
*
* @brief   Get values of security breach global drop counter. Global security breach
*         counter counts all dropped packets due to security breach event.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dev                      - physical device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
* @param[out] counValuePtr             - (pointer to) secure breach global counter.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - bad input parameters.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSecurBreachPortGroupGlobalDropCntrGet
(
    IN  GT_U8                   dev,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    OUT GT_U32                  *counValuePtr
);

/**
* @internal cpssDxChBrgSecurBreachEventPacketCommandSet function
* @endinternal
*
* @brief   Set for given security breach event it's packet command.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dev                      - device number
* @param[in] eventType                - security breach event type.
*                                      The acceptable events are:
*                                      CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E,
*                                      CPSS_BRG_SECUR_BREACH_EVENTS_PORT_NOT_IN_VLAN_E,
*                                      CPSS_BRG_SECUR_BREACH_EVENTS_VLAN_RANGE_DROP_E,
*                                      CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_VLAN_E,
*                                      CPSS_BRG_SECUR_BREACH_EVENTS_MOVED_STATIC_E,
*                                      CPSS_BRG_SECUR_BREACH_EVENTS_FRAME_TYPE,
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*                                      CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SPOOF_PROTECTION_E,
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*                                      CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_IS_DA_E,
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*                                      CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_IS_ZERO_E.
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
* @param[in] command                  - packet command. (APPLICABLE VALUES:
*                                      CPSS_PACKET_CMD_DROP_HARD_E;
*                                      CPSS_PACKET_CMD_DROP_SOFT_E)
*                                      for Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X : also (APPLICABLE VALUES:
*                                      CPSS_PACKET_CMD_FORWARD_E;
*                                      CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - bad input parameters.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note this function similar to cpssDxChBrgSecurBreachEventDropModeSet , but
*       allow additional packet commands.
*
*/
GT_STATUS cpssDxChBrgSecurBreachEventPacketCommandSet
(
    IN GT_U8                                dev,
    IN CPSS_BRG_SECUR_BREACH_EVENTS_ENT     eventType,
    IN CPSS_PACKET_CMD_ENT                  command
);

/**
* @internal cpssDxChBrgSecurBreachEventPacketCommandGet function
* @endinternal
*
* @brief   Get for given security breach event it's packet command.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dev                      - device number
* @param[in] eventType                - security breach event type.
*                                      The acceptable events are:
*                                      CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E,
*                                      CPSS_BRG_SECUR_BREACH_EVENTS_PORT_NOT_IN_VLAN_E,
*                                      CPSS_BRG_SECUR_BREACH_EVENTS_VLAN_RANGE_DROP_E,
*                                      CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_VLAN_E,
*                                      CPSS_BRG_SECUR_BREACH_EVENTS_MOVED_STATIC_E,
*                                      CPSS_BRG_SECUR_BREACH_EVENTS_FRAME_TYPE,
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*                                      CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SPOOF_PROTECTION_E,
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*                                      CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_IS_DA_E,
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*                                      CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_IS_ZERO_E.
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*
* @param[out] commandPtr               - (pointer to) packet command.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - bad input parameters.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note this function similar to cpssDxChBrgSecurBreachEventDropModeGet , but
*       allow additional packet commands.
*
*/
GT_STATUS cpssDxChBrgSecurBreachEventPacketCommandGet
(
    IN GT_U8                                dev,
    IN CPSS_BRG_SECUR_BREACH_EVENTS_ENT     eventType,
    OUT CPSS_PACKET_CMD_ENT                 *commandPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChBrgSecurityBreachh */

