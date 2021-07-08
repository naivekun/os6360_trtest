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
* @file cpssDxChPortPip.h
*
* @brief CPSS DxCh Port's Pre-Ingress Prioritization (PIP).
*
*
* @version   1
********************************************************************************
*/
#ifndef __cpssDxChPortPip
#define __cpssDxChPortPip

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>

/**
* @enum CPSS_DXCH_PORT_PIP_PROTOCOL_ENT
 *
 * @brief Enumeration of PIP type of supported protocols
 * (etherTypes to identify protocol).
*/
typedef enum{

    /** VLAN tag (TPID) */
    CPSS_DXCH_PORT_PIP_PROTOCOL_VLAN_TAG_E,

    /** MPLS protocol */
    CPSS_DXCH_PORT_PIP_PROTOCOL_MPLS_E   ,

    /** IPV4 protocol */
    CPSS_DXCH_PORT_PIP_PROTOCOL_IPV4_E   ,

    /** IPV6 protocol */
    CPSS_DXCH_PORT_PIP_PROTOCOL_IPV6_E   ,

    /** 'user defined ethertype' protocol */
    CPSS_DXCH_PORT_PIP_PROTOCOL_UDE_E

} CPSS_DXCH_PORT_PIP_PROTOCOL_ENT;

/**
* @enum CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ENT
 *
 * @brief Enumeration for PIP classification of packet for priority assignment (per port).
*/
typedef enum{

    /** packet with matched etherType and vid assign priority per port */
    CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ETHERTYPE_AND_VID_E,

    /** packet that was not classified assigned 'default' priority per port */
    CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_DEFAULT_E

} CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ENT;

/**
* @enum CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_ENT
 *
 * @brief Enumeration for PIP classification of packet for priority assignment (per profile).
*/
typedef enum{

    /** @brief packet with DSA TAG and <QOS PROFILE> assign priority per PIP profile
     *  per <QOS PROFILE> (APPLICABLE RANGES: 0..127)
     */
    CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_DSA_QOS_PROFILE_E,

    /** @brief packet with DSA TAG assign priority per PIP profile
     *  per <UP> (APPLICABLE RANGES: 0..7)
     */
    CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_DSA_UP_E,

    /** @brief packet with matched VLAN TAG assign priority per PIP profile
     *  per <UP> (APPLICABLE RANGES: 0..7)
     */
    CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_VLAN_TAG_UP_E,

    /** @brief packet with MPLS assign priority per PIP profile
     *  per <EXP> (APPLICABLE RANGES: 0..7)
     */
    CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_MPLS_EXP_E,

    /** @brief packet with IPV4 assign priority per PIP profile
     *  per <TOS> (APPLICABLE RANGES: 0..255)
     */
    CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_IPV4_TOS_E,

    /** @brief packet with IPV6 assign priority per PIP profile
     *  per <TC> (APPLICABLE RANGES: 0..255)
     */
    CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_IPV6_TC_E,

    /** @brief packet with matched UDE (user defined ethertype) assign priority per PIP profile
     *  per <ude index> (APPLICABLE RANGES: 0..3)
     */
    CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_UDE_INDEX_E,

    /** @brief packet with matched MAC DA assign priority per PIP profile
     *  per <macDa index> (APPLICABLE RANGES: 0..3)
     */
    CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_MAC_DA_INDEX_E

} CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_ENT;

/**
* @struct CPSS_DXCH_PORT_PIP_MAC_DA_CLASSIFICATION_STC
 *
 * @brief structure for PIP MAC DA classification info
*/
typedef struct{

    /** the mac DA value */
    GT_ETHERADDR macAddrValue;

    /** the mac DA mask */
    GT_ETHERADDR macAddrMask;

} CPSS_DXCH_PORT_PIP_MAC_DA_CLASSIFICATION_STC;

/**
* @internal cpssDxChPortPipGlobalEnableSet function
* @endinternal
*
* @brief   Enable/disable the PIP engine. (Global to the device)
*         if enabled packets are dropped according to their parsed priority if FIFOs
*         before the control pipe are above a configurable threshold
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] enable                   - GT_TRUE  -   PIP
*                                      GT_FALSE - disable PIP
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipGlobalEnableSet
(
    IN GT_U8        devNum,
    IN GT_BOOL      enable
);

/**
* @internal cpssDxChPortPipGlobalEnableGet function
* @endinternal
*
* @brief   Get the state Enable/disable of the PIP engine. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - enable  PIP
*                                      GT_FALSE - disable PIP
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipGlobalEnableGet
(
    IN  GT_U8        devNum,
    OUT GT_BOOL      *enablePtr
);

/**
* @internal cpssDxChPortPipGlobalEtherTypeProtocolSet function
* @endinternal
*
* @brief   Set the PIP etherTypes for the protocol. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] protocol                 - the  for which we define the etherType
* @param[in] index                    - relevant to 'vlan tag' that supports 4 etherTypes (TPIDs)
*                                      relevant to 'mpls' that supports 2 etherTypes
*                                      relevant to 'user defined' that supports 4 etherTypes
*                                      ignored for other protocols.
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] etherType                -  to recognize the protocols packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or protocol or index
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipGlobalEtherTypeProtocolSet
(
    IN GT_U8        devNum,
    IN CPSS_DXCH_PORT_PIP_PROTOCOL_ENT  protocol,
    IN GT_U32       index,
    IN GT_U16       etherType
);

/**
* @internal cpssDxChPortPipGlobalEtherTypeProtocolGet function
* @endinternal
*
* @brief   Get the PIP etherTypes for the protocol. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] protocol                 - the  for which we define the etherType
* @param[in] index                    - relevant to 'mpls' that supports 2 etherTypes
*                                      relevant to 'user defined' that supports 4 etherTypes
*                                      ignored for other protocols.
*                                      (APPLICABLE RANGES: 0..3)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or protocol or index
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipGlobalEtherTypeProtocolGet
(
    IN GT_U8        devNum,
    IN CPSS_DXCH_PORT_PIP_PROTOCOL_ENT  protocol,
    IN GT_U32       index,
    OUT GT_U16      *etherTypePtr
);

/**
* @internal cpssDxChPortPipTrustEnableSet function
* @endinternal
*
* @brief   Set port as trusted/not trusted PIP port.
*         A 'trusted' port will set priority level for ingress packets according to
*         packet's fields.
*         Otherwise, for the 'not trusted' port will use the port’s default priority
*         level.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   - GT_TRUE  - set port as 'trusted'
*                                      GT_FALSE - set port as 'not trusted'
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipTrustEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
);

/**
* @internal cpssDxChPortPipTrustEnableGet function
* @endinternal
*
* @brief   Get port's trusted/not trusted .
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] enablePtr                -  (pointer to)
*                                      GT_TRUE  - set port as 'trusted'
*                                      GT_FALSE - set port as 'not trusted'
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipTrustEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr
);

/**
* @internal cpssDxChPortPipProfileSet function
* @endinternal
*
* @brief   Set port's PIP profile.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] pipProfile               - the profile of the port.
*                                      (APPLICABLE RANGES: 0..3)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipProfileSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  pipProfile
);

/**
* @internal cpssDxChPortPipProfileGet function
* @endinternal
*
* @brief   Get port's PIP profile.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] pipProfilePtr            - (pointer to) the profile of the port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipProfileGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *pipProfilePtr
);

/**
* @internal cpssDxChPortPipVidClassificationEnableSet function
* @endinternal
*
* @brief   Set port PIP enable/disable VID classification .
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   - GT_TRUE   -  VID classification
*                                      GT_FALSE  - disable VID classification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipVidClassificationEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
);

/**
* @internal cpssDxChPortPipVidClassificationEnableGet function
* @endinternal
*
* @brief   Get port PIP enable/disable VID classification .
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE   - enable VID classification
*                                      GT_FALSE  - disable VID classification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipVidClassificationEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr
);

/**
* @internal cpssDxChPortPipPrioritySet function
* @endinternal
*
* @brief   Set priority per Port for 'classification' type.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] type                     - 'classification' type
* @param[in] priority                 - the  value to set.
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X 0..2).
*                                      - (0-high,1-mid,2-low)
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon 0..3).
*                                      - (0-very high,1-high,2-mid,3-low)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or portNum or type
* @retval GT_OUT_OF_RANGE          - on out of range priority
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipPrioritySet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ENT type,
    IN  GT_U32                  priority
);

/**
* @internal cpssDxChPortPipPriorityGet function
* @endinternal
*
* @brief   Get priority per Port for 'classification' type.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] type                     - 'classification' type
*
* @param[out] priorityPtr              - (pointer to)the priority value to set.
*                                      For Bobcat3:     For Bobcat2, Caelum, Aldrin, AC3X:
*                                      0 - Very High    0 - High
*                                      1 - High         1 - Medium
*                                      2 - Medium       2 - Low
*                                      3 - Low
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or portNum or type
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipPriorityGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ENT type,
    OUT GT_U32                  *priorityPtr
);

/**
* @internal cpssDxChPortPipGlobalProfilePrioritySet function
* @endinternal
*
* @brief   Set priority per PIP profile Per 'classification' type.
*         (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] pipProfile               - index to the 'profile table'.
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] type                     - 'classification' type
* @param[in] fieldIndex               - index of the priority field in the 'profile entry'
*                                      see details in description of relevant field in
*                                      CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_ENT
* @param[in] priority                 - the  value to set.
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X 0..2).
*                                      - (0-high,1-mid,2-low)
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon 0..3).
*                                      - (0-very high,1-high,2-mid,3-low)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or type or pipProfile or
*                                       fieldIndex
* @retval GT_OUT_OF_RANGE          - on out of range priority
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipGlobalProfilePrioritySet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  pipProfile,
    IN  CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_ENT type,
    IN  GT_U32                  fieldIndex,
    IN  GT_U32                  priority
);


/**
* @internal cpssDxChPortPipGlobalProfilePriorityGet function
* @endinternal
*
* @brief   Get priority per PIP profile Per 'classification' type. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] pipProfile               - index to the 'profile table'.
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] type                     - 'classification' type
* @param[in] fieldIndex               - index of the priority field in the 'profile entry'
*                                      see details in description of relevant field in
*                                      CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_ENT
*
* @param[out] priorityPtr              - (pointer to)the priority value.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or type or pipProfile or fieldIndex
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipGlobalProfilePriorityGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  pipProfile,
    IN  CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_ENT type,
    IN  GT_U32                  fieldIndex,
    OUT GT_U32                  *priorityPtr
);

/**
* @internal cpssDxChPortPipGlobalVidClassificationSet function
* @endinternal
*
* @brief   Set PIP vid classification value per index. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] index                    - one of 4 indexes.
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] vid                      - the  to match
*                                      (APPLICABLE RANGES: 0..(4k-1))
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or index
* @retval GT_OUT_OF_RANGE          - on out of range of vid
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipGlobalVidClassificationSet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  index,
    IN  GT_U16                  vid
);

/**
* @internal cpssDxChPortPipGlobalVidClassificationGet function
* @endinternal
*
* @brief   Get PIP vid classification value per index. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] index                    - one of 4 indexes.
*                                      (APPLICABLE RANGES: 0..3)
*
* @param[out] vidPtr                   - (pointer to)the vid to match
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or index
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipGlobalVidClassificationGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  index,
    OUT GT_U16                  *vidPtr
);


/**
* @internal cpssDxChPortPipGlobalMacDaClassificationEntrySet function
* @endinternal
*
* @brief   Set PIP Mac Da classification entry. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] index                    -  to the 'mac DA classification table'.
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] entryPtr                 - (pointer to) the entry.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or index
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of value in entry
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipGlobalMacDaClassificationEntrySet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  index,
    IN  CPSS_DXCH_PORT_PIP_MAC_DA_CLASSIFICATION_STC *entryPtr
);


/**
* @internal cpssDxChPortPipGlobalMacDaClassificationEntryGet function
* @endinternal
*
* @brief   Get PIP Mac Da classification entry. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] index                    -  to the 'mac DA classification table'.
*                                      (APPLICABLE RANGES: 0..3)
*
* @param[out] entryPtr                 - (pointer to) the entry.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or index
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipGlobalMacDaClassificationEntryGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  index,
    OUT CPSS_DXCH_PORT_PIP_MAC_DA_CLASSIFICATION_STC *entryPtr
);

/**
* @internal cpssDxChPortPipGlobalBurstFifoThresholdsSet function
* @endinternal
*
* @brief   Set Burst FIFO Threshold for specific PIP priority.
*         (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] dataPathBmp              - bitmap of Data Paths
*                                      (APPLICABLE DEVICES Caelum; Aldrin; AC3X; Bobcat3)
*                                      NOTEs:
*                                      1. for non multi data paths device this parameter is
*                                      IGNORED.
*                                      2. for multi data paths device:
*                                      bitmap must be set with at least one bit representing
*                                      valid data path(s). If a bit of non valid data path
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_DATA_PATH_UNAWARE_MODE_CNS is supported.
*                                      3. For multi-pipe device:
*                                      bitmap must be set with at least one bit representing
*                                      pipe(s).  If a bit of non valid pipe is set then
*                                      function returns GT_BAD_PARAM. Value
*                                      CPSS_DATA_PATH_UNAWARE_MODE_CNS is supported.
* @param[in] priority                 - the  to set the threshold.
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X 0..2).
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon 0..3).
* @param[in] threshold                - the  value. (number of packets)
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X 0..0x3FF)
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or priority
* @retval GT_OUT_OF_RANGE          - on out of range threshold
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipGlobalBurstFifoThresholdsSet
(
    IN GT_U8            devNum,
    IN GT_DATA_PATH_BMP dataPathBmp,
    IN GT_U32           priority,
    IN GT_U32           threshold
);

/**
* @internal cpssDxChPortPipGlobalBurstFifoThresholdsGet function
* @endinternal
*
* @brief   Get Burst FIFO Threshold for specific PIP priority.
*         (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] dataPathBmp              - bitmap of Data Paths
*                                      (APPLICABLE DEVICES Caelum; Aldrin; AC3X; Bobcat3)
*                                      NOTEs:
*                                      1. for non multi data paths device this parameter is
*                                      IGNORED.
*                                      2. for multi data paths device:
*                                      bitmap must be set with at least one bit representing
*                                      valid data path(s). If a bit of non valid data path
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_DATA_PATH_UNAWARE_MODE_CNS is supported.
*                                      3. For multi-pipe device:
*                                      bitmap must be set with at least one bit representing
*                                      pipe(s). If a bit of non valid data path is set then
*                                      function returns GT_BAD_PARAM. Value
*                                      PSS_DATA_PATH_UNAWARE_MODE_CNS is supported.
*                                      4. read only from first data path of the bitmap.
* @param[in] priority                 - the  to set the threshold.
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X 0..2).
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon 0..3).
*
* @param[out] thresholdPtr             - (pointer to)the threshold value. (number of packets)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or priority
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipGlobalBurstFifoThresholdsGet
(
    IN GT_U8            devNum,
    IN GT_DATA_PATH_BMP dataPathBmp,
    IN GT_U32           priority,
    OUT GT_U32         *thresholdPtr
);

/**
* @internal cpssDxChPortPipGlobalDropCounterGet function
* @endinternal
*
* @brief   Get all PIP drop counters (Global to the device)
*         NOTE: the counters are 'clear on read'
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Falcon.
*
* @param[in] devNum                   - physical device number
*
* @param[out] countersArr[4]           - (pointer to) the counters of
*                                      - 3 priorities (0-high, 1-mid, 2-low) for Bobcat2,
*                                      Caelum, Aldrin, AC3X
*                                      - 4 priorities (0-very high, 1-high, 2-mid, 3-low) for
*                                      Bobcat3
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or priority
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipGlobalDropCounterGet
(
    IN GT_U8        devNum,
    OUT GT_U64      countersArr[4]
);

/**
* @internal cpssDxChPortPipDropCounterGet function
* @endinternal
*
* @brief   Get PIP drop counter of specific port.
*         NOTE: the counters are 'clear on read'
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] counterPtr               - (pointer to) the counter value.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or portNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPipDropCounterGet
(
    IN GT_U8        devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U64      *counterPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChPortPip */


