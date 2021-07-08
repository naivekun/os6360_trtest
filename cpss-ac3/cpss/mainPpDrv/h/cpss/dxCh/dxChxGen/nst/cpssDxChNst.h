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
* @file cpssDxChNst.h
*
* @brief Network Shield Technology facility Cheetah CPSS declarations.
*
*
* @version   8
********************************************************************************
*/

#ifndef __cpssDxChNsth
#define __cpssDxChNsth

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/nst/cpssNstTypes.h>



/**
* @internal cpssDxChNstBridgeAccessMatrixCmdSet function
* @endinternal
*
* @brief   Set bridge access matrix entry.
*         The packet is assigned two access levels based on its MAC SA/SIP and MAC
*         DA/DIP.
*         The device supports up to 8 SA and up to 8 DA levels.
*         The Access Matrix Configuration tables controls which access level
*         pairs can communicate with each other. Based on the access level pair,
*         the packet is assigned a command of: FORWARD, HARD DROP, and SOFT DROP.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - physical device number
* @param[in] saAccessLevel            - Source address access level
* @param[in] daAccessLevel            - Destination address access level
* @param[in] command                  -  assigned to a packet
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, saAccessLevel, daAccessLevel.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNstBridgeAccessMatrixCmdSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      saAccessLevel,
    IN  GT_U32                      daAccessLevel,
    IN  CPSS_PACKET_CMD_ENT         command
);

/**
* @internal cpssDxChNstBridgeAccessMatrixCmdGet function
* @endinternal
*
* @brief   Get bridge access matrix entry.
*         The packet is assigned two access levels based on its MAC SA/SIP and MAC
*         DA/DIP.
*         The device supports up to 8 SA and up to 8 DA levels.
*         The Access Matrix Configuration tables controls which access level
*         pairs can communicate with each other. Based on the access level pair,
*         the packet is assigned a command of: FORWARD, HARD DROP, and SOFT DROP.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - physical device number
* @param[in] saAccessLevel            - Source address access level
* @param[in] daAccessLevel            - Destination address access level
*
* @param[out] commandPtr               - command assigned to a packet
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, saAccessLevel, daAccessLevel.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNstBridgeAccessMatrixCmdGet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      saAccessLevel,
    IN  GT_U32                      daAccessLevel,
    OUT CPSS_PACKET_CMD_ENT         *commandPtr
);

/**
* @internal cpssDxChNstDefaultAccessLevelsSet function
* @endinternal
*
* @brief   Set default access levels for Bridge Access Matrix Configuration table
*         that controls which access level pairs can communicate with each other.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - physical device number
* @param[in] paramType                -   type of bridge acess matrix global parameter to
*                                      configure default access level .
* @param[in] accessLevel              -  default security access level for parameter
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum,paramType,accessLevel
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNstDefaultAccessLevelsSet
(
    IN GT_U8                    devNum,
    IN CPSS_NST_AM_PARAM_ENT    paramType,
    IN GT_U32                   accessLevel
);

/**
* @internal cpssDxChNstDefaultAccessLevelsGet function
* @endinternal
*
* @brief   Get default access levels for Bridge Access Matrix Configuration table
*         that controls which access level pairs can communicate with each other.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - physical device number
* @param[in] paramType                -   type of bridge acess matrix global parameter to
*                                      configure default access level .
*
* @param[out] accessLevelPtr           - (pointer to) default security access level for parameter
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum,paramType
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNstDefaultAccessLevelsGet
(
    IN  GT_U8                    devNum,
    IN  CPSS_NST_AM_PARAM_ENT    paramType,
    OUT GT_U32                   *accessLevelPtr
);

/**
* @internal cpssDxChNstProtSanityCheckSet function
* @endinternal
*
* @brief   Set packet sanity checks.
*         Sanity Check engine identifies "suspicious" packets and
*         provides an option for assigning them a Hard Drop packet command.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - physical device number
* @param[in] checkType                - the type of sanity check
* @param[in] enable                   - GT_TRUE   packet's sanity check. Packet that not passed
*                                      check will be dropped and treated as security breach event.
*                                      - GT_FALSE disable packet's sanity check.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum,checkType
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNstProtSanityCheckSet
(
    IN GT_U8                 devNum,
    IN CPSS_NST_CHECK_ENT    checkType,
    IN GT_BOOL               enable
);

/**
* @internal cpssDxChNstProtSanityCheckGet function
* @endinternal
*
* @brief   Get packet sanity checks.
*         Sanity Check engine identifies "suspicious" packets and
*         provides an option for assigning them a Hard Drop packet command.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - physical device number
* @param[in] checkType                - the type of sanity check
*
* @param[out] enablePtr                - GT_TRUE  enable packet's sanity check. Packet that not passed
*                                      check will be dropped and treated as security breach event.
*                                      - GT_FALSE disable packet's sanity check.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum,checkType
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNstProtSanityCheckGet
(
    IN  GT_U8                 devNum,
    IN  CPSS_NST_CHECK_ENT    checkType,
    OUT GT_BOOL               *enablePtr
);

/**
* @internal cpssDxChNstPortIngressFrwFilterSet function
* @endinternal
*
* @brief   Set port ingress forwarding filter.
*         For a given ingress port Enable/Disable traffic if it is destinied to:
*         CPU, ingress analyzer, network.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] filterType               - ingress filter type
* @param[in] enable                   - GT_TRUE -  ingress forwarding restiriction according
*                                      to filterType, filterred packets will be dropped.
*                                      - GT_FALSE - disable ingress forwarding restiriction
*                                      according to filterType
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, port, filterType
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNstPortIngressFrwFilterSet
(
    IN GT_U8                            devNum,
    IN GT_PORT_NUM                      portNum,
    IN CPSS_NST_INGRESS_FRW_FILTER_ENT  filterType,
    IN GT_BOOL                          enable
);

/**
* @internal cpssDxChNstPortIngressFrwFilterGet function
* @endinternal
*
* @brief   Get port ingress forwarding filter.
*         For a given ingress port Enable/Disable traffic if it is destinied to:
*         CPU, ingress analyzer, network.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] filterType               - ingress filter type
*
* @param[out] enablePtr                - GT_TRUE - enable ingress forwarding restiriction according
*                                      to filterType, filterred packets will be dropped.
*                                      - GT_FALSE - disable ingress forwarding restiriction
*                                      according to filterType
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, port, filterType
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNstPortIngressFrwFilterGet
(
    IN  GT_U8                            devNum,
    IN  GT_PORT_NUM                      portNum,
    IN  CPSS_NST_INGRESS_FRW_FILTER_ENT  filterType,
    OUT GT_BOOL                          *enablePtr
);


/**
* @internal cpssDxChNstPortEgressFrwFilterSet function
* @endinternal
*
* @brief   Set port egress forwarding filter.
*         For a given egress port Enable/Disable traffic if the packet was:
*         sent from CPU with FROM_CPU DSA tag, bridged or policy switched,
*         routed or policy routed.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] filterType               - egress filter type
* @param[in] enable                   - GT_TRUE -  egress forwarding restiriction according
*                                      to filterType, filterred packets will be dropped.
*                                      - GT_FALSE - disable egress forwarding restiriction
*                                      according to filterType.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, port, filterType
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNstPortEgressFrwFilterSet
(
    IN GT_U8                            devNum,
    IN GT_PORT_NUM                      portNum,
    IN CPSS_NST_EGRESS_FRW_FILTER_ENT   filterType,
    IN GT_BOOL                          enable
);

/**
* @internal cpssDxChNstPortEgressFrwFilterGet function
* @endinternal
*
* @brief   Get port egress forwarding filter.
*         For a given egress port Enable/Disable traffic if the packet was:
*         sent from CPU with FROM_CPU DSA tag, bridged or policy switched,
*         routed or policy routed.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] filterType               - egress filter type
*
* @param[out] enablePtr                - GT_TRUE - enable egress forwarding restiriction according
*                                      to filterType, filterred packets will be dropped.
*                                      - GT_FALSE - disable egress forwarding restiriction
*                                      according to filterType.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, port, filterType
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNstPortEgressFrwFilterGet
(
    IN  GT_U8                            devNum,
    IN  GT_PORT_NUM                      portNum,
    IN  CPSS_NST_EGRESS_FRW_FILTER_ENT   filterType,
    OUT GT_BOOL                          *enablePtr
);

/**
* @internal cpssDxChNstIngressFrwFilterDropCntrSet function
* @endinternal
*
* @brief   Set the global ingress forwarding restriction drop packet counter.
*         This counter counts the number of packets dropped due to Ingress forward
*         restrictions.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] ingressCnt               - the counter value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNstIngressFrwFilterDropCntrSet
(
    IN GT_U8       devNum,
    IN GT_U32      ingressCnt
);

/**
* @internal cpssDxChNstIngressFrwFilterDropCntrGet function
* @endinternal
*
* @brief   Reads the global ingress forwarding restriction drop packet counter.
*         This counter counts the number of packets dropped due to Ingress forward
*         restrictions.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - physical device number
*
* @param[out] ingressCntPtr            - the counter value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNstIngressFrwFilterDropCntrGet
(
    IN GT_U8       devNum,
    OUT GT_U32     *ingressCntPtr
);

/**
* @internal cpssDxChNstRouterAccessMatrixCmdSet function
* @endinternal
*
* @brief   Set Routers access matrix entry.
*         The packet is assigned two access levels based on its SIP and DIP.
*         The device supports up to 8 SIP and up to 8 DIP levels.
*         The Access Matrix Configuration tables controls which access level
*         pairs can communicate with each other. Based on the access level pair,
*         the packet is assigned a command of: FORWARD, HARD DROP, and SOFT DROP.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] sipAccessLevel           - the sip access level
* @param[in] dipAccessLevel           - the dip access level
* @param[in] command                  -  assigned to a packet
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, saAccessLevel, daAccessLevel.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNstRouterAccessMatrixCmdSet
(
    IN  GT_U8                      devNum,
    IN  GT_U32                     sipAccessLevel,
    IN  GT_U32                     dipAccessLevel,
    IN  CPSS_PACKET_CMD_ENT        command
);

/**
* @internal cpssDxChNstRouterAccessMatrixCmdGet function
* @endinternal
*
* @brief   Get Routers access matrix entry.
*         The packet is assigned two access levels based on its SIP and DIP.
*         The device supports up to 8 SIP and up to 8 DIP levels.
*         The Access Matrix Configuration tables controls which access level
*         pairs can communicate with each other. Based on the access level pair,
*         the packet is assigned a command of: FORWARD, HARD DROP, and SOFT DROP.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] sipAccessLevel           - the sip access level
* @param[in] dipAccessLevel           - the dip access level
*
* @param[out] commandPtr               - command assigned to a packet
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, saAccessLevel, daAccessLevel.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNstRouterAccessMatrixCmdGet
(
    IN  GT_U8                      devNum,
    IN  GT_U32                     sipAccessLevel,
    IN  GT_U32                     dipAccessLevel,
    OUT  CPSS_PACKET_CMD_ENT        *commandPtr
);

/**
* @internal cpssDxChNstPortGroupIngressFrwFilterDropCntrSet function
* @endinternal
*
* @brief   Set the global ingress forwarding restriction drop packet counter.
*         This counter counts the number of packets dropped due to Ingress forward
*         restrictions.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] ingressCnt               - the counter value
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNstPortGroupIngressFrwFilterDropCntrSet
(
    IN GT_U8                   devNum,
    IN GT_PORT_GROUPS_BMP      portGroupsBmp,
    IN GT_U32                  ingressCnt
);

/**
* @internal cpssDxChNstPortGroupIngressFrwFilterDropCntrGet function
* @endinternal
*
* @brief   Reads the global ingress forwarding restriction drop packet counter.
*         This counter counts the number of packets dropped due to Ingress forward
*         restrictions.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - physical device number
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
* @param[out] ingressCntPtr            - the counter value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNstPortGroupIngressFrwFilterDropCntrGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    OUT GT_U32                  *ingressCntPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChNsth */


