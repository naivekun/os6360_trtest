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
* @file tgfCosGen.h
*
* @brief Generic API for Cos Technology facility API.
*
* @version   19
********************************************************************************
*/
#ifndef __tgfCosGenh
#define __tgfCosGenh

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cos/cpssCosTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>

#include <common/tgfCommon.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* default UP profile index to use */
#define PRV_TGF_COS_UP_PROFILE_INDEX_DEFAULT_CNS    0

/******************************************************************************\
 *                          Private type definitions                          *
\******************************************************************************/

/**
* @struct PRV_TGF_COS_PROFILE_STC
 *
 * @brief The Prestera Class-of-Service profile parameters
*/
typedef struct{

    /** drop precedence (color) (0..2) */
    CPSS_DP_LEVEL_ENT dropPrecedence;

    /** IEEEE 802.1p User Priority (0..7) */
    GT_U32 userPriority;

    /** egress tx traffic class queue (0..7) */
    GT_U32 trafficClass;

    /** ip DCSP field (0..63) */
    GT_U32 dscp;

    /** MPLS label exp value (0..7) */
    GT_U32 exp;

} PRV_TGF_COS_PROFILE_STC;

/**
* @enum PRV_TGF_COS_PARAM_ENT
 *
 * @brief QoS parameter enumeration
*/
typedef enum{

    /** DSA QoS parameter */
    PRV_TGF_COS_PARAM_DSA_QOS_PROFILE_E,

    /** User Priority QoS parameter */
    PRV_TGF_COS_PARAM_UP_E,

    /** Drop Precedence */
    PRV_TGF_COS_PARAM_DP_E,

    /** Traffic Class */
    PRV_TGF_COS_PARAM_TC_E,

    /** DSCP QoS parameter */
    PRV_TGF_COS_PARAM_DSCP_E,

    /** EXP QoS parameter */
    PRV_TGF_COS_PARAM_EXP_E

} PRV_TGF_COS_PARAM_ENT;


/**********************************************************************
 * typedef: struct PRV_TGF_QOS_ENTRY_STC
 *
 * Description:
 *      QoS configuration entry
 *
 * Enumerations:
 *      qosParamsModify  -  Enable/Disable modifing per QoS parameter
 *      qosParams        -  QoS Parameters
 *      qosPrecedence    -  QoS Precedence.
 * COMMENTS:
 ***********************************************************************/
typedef struct
{
    PRV_TGF_QOS_PARAM_MODIFY_STC                 qosParamsModify;
    PRV_TGF_QOS_PARAM_STC                        qosParams;
    CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT  qosPrecedence;
}PRV_TGF_QOS_ENTRY_STC;

/**
* @enum PRV_TGF_QOS_MAC_RESOLVE_ENT
 *
 * @brief MAC QoS Marking Conflict Resolution mode.
*/
typedef enum{

    /** @brief get the QoS attribute index
     *  from FDB entry according to DA lookup.
     */
    PRV_TGF_QOS_MAC_RESOLVE_USE_DA_E = 0,

    /** @brief get the QoS attribute index
     *  from FDB entry according to SA lookup.
     */
    PRV_TGF_QOS_MAC_RESOLVE_USE_SA_E,

    /** @brief get QoS attribute
     *  with the maximum TC. If TC_SA==TC_DA select QoS attribute
     *  index from FDB entry according to SA lookup.
     *  PRV_TGF_QOS_MAC_RESOLVE_USE_MAX_TC_DA_E- get QoS attribute
     *  with the maximum TC. If TC_DA==TC_SA select QoS attribute
     *  index from FDB entry according to DA lookup.
     */
    PRV_TGF_QOS_MAC_RESOLVE_USE_MAX_TC_SA_E,

    PRV_TGF_QOS_MAC_RESOLVE_USE_MAX_TC_DA_E,

    /** @brief get QoS attribute
     *  with the minimum TC. If TC_SA==TC_DA select QoS attribute
     *  index from FDB entry according to SA lookup.
     */
    PRV_TGF_QOS_MAC_RESOLVE_USE_MIN_TC_SA_E,

    /** @brief get QoS attribute
     *  with the minimum TC. If TC_DA==TC_SA select QoS attribute
     *  index from FDB entry according to DA lookup.
     */
    PRV_TGF_QOS_MAC_RESOLVE_USE_MIN_TC_DA_E

} PRV_TGF_QOS_MAC_RESOLVE_ENT;


/******************************************************************************\
 *                       CPSS generic API section                             *
\******************************************************************************/

/**
* @internal prvTgfCosExpToProfileMapSet function
* @endinternal
*
* @brief   Maps the packet MPLS exp to a QoS Profile
*         INPUTS:
*         mappingTableIndex - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*         [APPLICABLE RANGE : 0..11]
* @param[in] mappingTableIndex        - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*                                      [APPLICABLE RANGE : 0..11]
*                                      APPLICABLE DEVICES : Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] exp                      - mpls exp
* @param[in] profileIndex             - profile index
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong dev, exp or profileIndex
*/
GT_STATUS prvTgfCosExpToProfileMapSet
(
    IN GT_U32   mappingTableIndex,
    IN  GT_U32                        exp,
    IN  GT_U32                        profileIndex
);

/**
* @internal prvTgfCosExpToProfileMapGet function
* @endinternal
*
* @brief   Returns the mapping of packet MPLS exp to a QoS Profile
*         INPUTS:
*         mappingTableIndex - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*         [APPLICABLE RANGE : 0..11]
* @param[in] mappingTableIndex        - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*                                      [APPLICABLE RANGE : 0..11]
*                                      APPLICABLE DEVICES : Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] exp                      - mpls exp
*
* @param[out] profileIndexPtr          - (pointer to) profile index
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong dev, exp or profileIndex
*/
GT_STATUS prvTgfCosExpToProfileMapGet
(
    IN GT_U32   mappingTableIndex,
    IN  GT_U32                        exp,
    OUT GT_U32                       *profileIndexPtr
);

/**
* @internal prvTgfCosDscpToProfileMapSet function
* @endinternal
*
* @brief   Maps the packet DSCP (or remapped DSCP) to a QoS Profile
*         INPUTS:
*         mappingTableIndex - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*         [APPLICABLE RANGE : 0..11]
* @param[in] mappingTableIndex        - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*                                      [APPLICABLE RANGE : 0..11]
*                                      APPLICABLE DEVICES : Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] dscp                     - DSCP of a IP packet
* @param[in] profileIndex             - profile index
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong dev, dscp or profileIndex
*/
GT_STATUS prvTgfCosDscpToProfileMapSet
(
    IN GT_U32   mappingTableIndex,
    IN  GT_U8                         dscp,
    IN  GT_U32                        profileIndex
);

/**
* @internal prvTgfCosDscpToProfileMapGet function
* @endinternal
*
* @brief   Get the Mapping: packet DSCP (or remapped DSCP) to a QoS Profile
*         INPUTS:
*         mappingTableIndex - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*         [APPLICABLE RANGE : 0..11]
* @param[in] mappingTableIndex        - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*                                      [APPLICABLE RANGE : 0..11]
*                                      APPLICABLE DEVICES : Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] dscp                     - DSCP of a IP packet
*
* @param[out] profileIndexPtr          - (pointer to) profile index
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong dev, dscp or profileIndex
*/
GT_STATUS prvTgfCosDscpToProfileMapGet
(
    IN GT_U32   mappingTableIndex,
    IN  GT_U8                         dscp,
    OUT GT_U32                       *profileIndexPtr
);

/**
* @internal prvTgfCosPortQosConfigSet function
* @endinternal
*
* @brief   Configures the port's QoS attributes.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices.
* @param[in] port                     - logical number, CPU port.
* @param[in] portQosCfgPtr            - Pointer to QoS related configuration of a port.
* @param[in] portQosCfgPtr            -
*                                      [0..71] for Ch,
*                                      [0..127] for Ch2 and above;
* @param[in] portQosCfgPtr            and
* @param[in] portQosCfgPtr
*                                      only [CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E,
*                                      CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E]
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong dev, port or portQosCfgPtr
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCosPortQosConfigSet
(
    IN GT_U32                port,
    IN CPSS_QOS_ENTRY_STC  *portQosCfgPtr
);

/**
* @internal prvTgfCosPortQosConfigGet function
* @endinternal
*
* @brief   Get the port's QoS attributes configuration.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port.
*
* @param[out] portQosCfgPtr            - Pointer to QoS related configuration of a port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, port or portQosCfgPtr
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCosPortQosConfigGet
(
    IN  GT_U8               devNum,
    IN  GT_PORT_NUM         portNum,
    OUT CPSS_QOS_ENTRY_STC  *portQosCfgPtr
);



/**
* @internal prvTgfCosPortQosTrustModeSet function
* @endinternal
*
* @brief   Configures port's Trust Mode.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices.
* @param[in] port                     - logical number, CPU port.
* @param[in] portQosTrustMode         - QoS trust mode of a port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong dev, port or trustMode.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCosPortQosTrustModeSet
(
    IN GT_U32                         port,
    IN CPSS_QOS_PORT_TRUST_MODE_ENT  portQosTrustMode
);

/**
* @internal prvTgfCosPortQosTrustModeGet function
* @endinternal
*
* @brief   Get Configured port's Trust Mode.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port.
*
* @param[out] portQosTrustModePtr      - QoS trust mode of a port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, port or trustMode.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCosPortQosTrustModeGet
(
    IN  GT_U8                         devNum,
    IN  GT_PORT_NUM                   portNum,
    OUT CPSS_QOS_PORT_TRUST_MODE_ENT  *portQosTrustModePtr
);


/**
* @internal prvTgfCosProfileEntrySet function
* @endinternal
*
* @brief   Configures the Profile Table Entry and
*         Initial Drop Precedence (DP) for Policer.
* @param[in] profileIndex             - index of a profile in the profile table
*                                      Ch values (0..71); Ch2 and above values (0..127).
* @param[in] cosPtr                   - Pointer to new CoS values for packet
*                                      (dp = CPSS_DP_YELLOW_E is not applicable).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or cos.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For DxCh3 devices Initial DP supports 3 levels (Yellow, Red, Green),
*       but QoS profile entry supports only 2 levels by the following way:
*       - The function set value 0 for Green DP
*       - The function set value 1 for both Yellow and Red DPs.
*
*/
GT_STATUS prvTgfCosProfileEntrySet
(
     IN  GT_U32                     profileIndex,
     IN  PRV_TGF_COS_PROFILE_STC    *cosPtr
);

/**
* @internal prvTgfCosProfileEntryGet function
* @endinternal
*
* @brief   Get Profile Table Entry configuration and
*         initial Drop Precedence (DP) for Policer.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] profileIndex             - index of a profile in the profile table
*                                      Ch values (0..71); Ch2 and above values (0..127).
*
* @param[out] cosPtr                   - Pointer to new CoS values for packet.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or cos.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For DxCh3 devices Initial DP supports 3 levels (Yellow, Red, Green),
*       but QoS profile entry supports only 2 levels.
*       Therefore DP value is taken from the Initial DP table.
*
*/
GT_STATUS prvTgfCosProfileEntryGet
(
     IN  GT_U8                      devNum,
     IN  GT_U32                     profileIndex,
     OUT PRV_TGF_COS_PROFILE_STC  *cosPtr
);


/**
* @internal prvTgfCosTrustExpModeSet function
* @endinternal
*
* @brief   Set Enable/Disable state of trust EXP mode for MPLS packets.
*
* @param[in] port                     - number, CPU port.
* @param[in] enable                   - trust the MPLS header's EXP field.
*                                      GT_FALSE - QoS profile from Exp To Qos Profile table
*                                      not assigned to the MPLS packet.
*                                      GT_TRUE  - QoS profile from Exp To Qos Profile table
*                                      assigned to the MPLS packet.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or port.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCosTrustExpModeSet
(
    IN  GT_U32      port,
    OUT GT_BOOL    enable
);

/**
* @internal prvTgfCosTrustExpModeGet function
* @endinternal
*
* @brief   Get Enable/Disable state of trust EXP mode for MPLS packets.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port.
*
* @param[out] enablePtr                - (pointer to) trust the MPLS header's EXP field.
*                                      GT_FALSE - QoS profile from Exp To Qos Profile table
*                                      not assigned to the MPLS packet.
*                                      GT_TRUE  - QoS profile from Exp To Qos Profile table
*                                      assigned to the MPLS packet.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or port.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCosTrustExpModeGet
(
    IN  GT_U8        devNum,
    IN  GT_PORT_NUM  portNum,
    OUT GT_BOOL      *enablePtr
);


/**
* @internal prvTgfCosUpCfiDeiToProfileMapSet function
* @endinternal
*
* @brief   Maps the UP Profile Index and packet's User Priority and CFI/DEI bit (Canonical Format
*         Indicator/Drop Eligibility Indicator) to QoS Profile.
*         The mapping relevant for L2 and L2-L3 QoS Trust modes for 802.1Q
*         or DSA tagged packets.
*         INPUTS:
*         mappingTableIndex - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*         [APPLICABLE RANGE : 0..11]
* @param[in] mappingTableIndex        - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*                                      [APPLICABLE RANGE : 0..11]
*                                      APPLICABLE DEVICES : Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] up                       - the UP profile index (table selector)
*                                      this parameter not relevant for ch1,2,3,xcat.
*                                      see also API : cpssDxChCosPortUpProfileIndexSet
* @param[in] up                       - User Priority of a VLAN or DSA tagged packet [0..7].
* @param[in] cfiDeiBit                - value of CFI/DEI bit[0..1].
*                                      this parameter not relevant for ch1,2.
* @param[in] profileIndex             - QoS Profile index, which is assigned to a VLAN or DSA
*                                      tagged packet with the UP on ports with enabled trust
*                                      L2 or trust L2-L3. Range is [0..127].(for ch1 range is [0..71])
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, up or cfiDeiBit.
* @retval GT_OUT_OF_RANGE          - on profileIndex out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCosUpCfiDeiToProfileMapSet
(
    IN GT_U32   mappingTableIndex,
    IN GT_U32   upProfileIndex,
    IN GT_U8    up,
    IN GT_U8    cfiDeiBit,
    IN GT_U32   profileIndex
);

/**
* @internal prvTgfCosUpCfiDeiToProfileMapGet function
* @endinternal
*
* @brief   Get the Map from UP Profile Index and packet's User Priority and CFI/DEI bit (Canonical Format
*         Indicator/Drop Eligibility Indicator) to QoS Profile.
*         The mapping relevant for L2 and L2-L3 QoS Trust modes for 802.1Q
*         or DSA tagged packets.
*         APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum     - device number.
*         mappingTableIndex - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*         [APPLICABLE RANGE : 0..11]
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*                                      [APPLICABLE RANGE : 0..11]
*                                      APPLICABLE DEVICES : Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
* @param[in] up                       - the UP profile index (table selector)
*                                      this parameter not relevant for ch1,2,3,xcat.
*                                      see also API : cpssDxChCosPortUpProfileIndexSet
* @param[in] up                       - User Priority of a VLAN or DSA tagged packet [0..7].
* @param[in] cfiDeiBit                - value of CFI/DEI bit[0..1].
*                                      this parameter not relevant for ch1,2.
*
* @param[out] profileIndexPtr          - (pointer to)QoS Profile index, which is assigned to a
*                                      VLAN or DSA tagged packet with the UP on ports with
*                                      enabled trust L2 or trust L2-L3. Range is [0..127]. (for ch1 range is [0..71])
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, up or cfiDeiBit.
*                                       or mappingTableIndex > 11 (APPLICABLE DEVICES : Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCosUpCfiDeiToProfileMapGet
(
    IN  GT_U8   devNum,
    IN GT_U32   mappingTableIndex,
    IN GT_U32   upProfileIndex,
    IN  GT_U8   up,
    IN  GT_U8   cfiDeiBit,
    OUT GT_U32 *profileIndexPtr
);

/**
* @internal prvTgfCosDpToCfiDeiMapSet function
* @endinternal
*
* @brief   Maps Drop Precedence to Drop Eligibility Indicator bit.
*
* @param[in] dp                       - Drop Precedence [Green, Red].
* @param[in] cfiDeiBit                - value of CFI/DEI bit[0..1].
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or dp.
* @retval GT_OUT_OF_RANGE          - on cfiDeiBit out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Three DP levels are mapped to only two values of CFI/DEI bit.
*
*/
GT_STATUS prvTgfCosDpToCfiDeiMapSet
(
    IN CPSS_DP_LEVEL_ENT    dp,
    IN GT_U8                cfiDeiBit
);

/**
* @internal prvTgfCosL2TrustModeVlanTagSelectSet function
* @endinternal
*
* @brief   Select type of VLAN tag (either Tag0 or Tag1) for Trust L2 Qos mode.
*
* @param[in] port                     - number, CPU port.
* @param[in] isDefaultVlanTagType     - indication that the vlanTagType is the default vlanTagType or
*                                      used for re-parse after TTI lookup:
*                                      1. Parse inner Layer2 after TT
*                                      2. Re-parse Layer2 after popping EVB/BPE tag
*                                      3. Parsing of passenger packet of non-TT packets (TRILL, IP-GRE, MPLS)
*                                      GT_TRUE  - used as default vlanTagType
*                                      GT_FALSE - used for re-parse after TTI lookup.
* @param[in] vlanTagType              - Vlan Tag Type (tag0 or tag1)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or port or vlanTagType.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCosL2TrustModeVlanTagSelectSet
(
    IN GT_U32                     port,
    IN GT_BOOL                   isDefaultVlanTagType,
    IN CPSS_VLAN_TAG_TYPE_ENT    vlanTagType
);

/**
* @internal prvTgfCosL2TrustModeVlanTagSelectGet function
* @endinternal
*
* @brief   Get select type of VLAN tag (either Tag0 or Tag1) for Trust L2 Qos mode.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port.
* @param[in] isDefaultVlanTagType     - indication that the vlanTagType is the default vlanTagType or
*                                      used for re-parse after TTI lookup:
*                                      1. Parse inner Layer2 after TT
*                                      2. Re-parse Layer2 after popping EVB/BPE tag
*                                      3. Parsing of passenger packet of non-TT packets (TRILL, IP-GRE, MPLS)
*                                      GT_TRUE  - used as default vlanTagType
*                                      GT_FALSE - used for re-parse after TTI lookup.
*
* @param[out] vlanTagTypePtr           - (pointer to)Vlan Tag Type (tag0 or tag1)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or port or vlanTagType.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCosL2TrustModeVlanTagSelectGet
(
    IN  GT_U8                     devNum,
    IN  GT_PORT_NUM               portNum,
    IN GT_BOOL                   isDefaultVlanTagType,
    OUT CPSS_VLAN_TAG_TYPE_ENT    *vlanTagTypePtr
);

/**
* @internal prvTgfCosPortUpProfileIndexSet function
* @endinternal
*
* @brief   set per ingress port and per UP (user priority) the 'UP profile index' (table selector).
*         See also API prvTgfCosUpCfiDeiToProfileMapSet
*
* @note   APPLICABLE DEVICES:      Lion and above.
* @param[in] devNum                   - device number.
* @param[in] portNum                  - ingress port number including CPU port.
* @param[in] up                       - user priority of a VLAN or DSA tagged packet [0..7].
* @param[in] upProfileIndex           - the UP profile index (table selector) , value 0..1
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, portNum or up
* @retval GT_OUT_OF_RANGE          - upProfileIndex > 1
* @retval GT_HW_ERROR              - on HW error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCosPortUpProfileIndexSet
(
    IN GT_U8    devNum,
    IN GT_U32    portNum,
    IN GT_U8    up,
    IN GT_U32   upProfileIndex
);

/**
* @internal prvTgfCosPortModifyQoSParamEnableSet function
* @endinternal
*
* @brief   Enable/Disable per port to modify packet's the given QoS Parameter
*         (UP, DSCP and EXP)
*
* @note   APPLICABLE DEVICES:      Puma2; Puma3.
* @param[in] portNum                  - port number.(Not CPU port)
* @param[in] cosParamType             - Cos Param Type (Up, Tc, Dscp, Dp, Exp, DsaProfile)
* @param[in] modifyEnable             - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or port number
* @retval GT_OUT_OF_RANGE          - when up is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCosPortModifyQoSParamEnableSet
(
    IN GT_U32                 portNum,
    IN PRV_TGF_COS_PARAM_ENT cosParamType,
    IN GT_BOOL               modifyEnable
);

/**
* @internal prvTgfCosPortDefaultUpSet function
* @endinternal
*
* @brief   Set default User Priority for untagged packet per port.
*
* @note   APPLICABLE DEVICES:      Puma2; Puma3.
* @param[in] portNum                  - port number.(Not CPU port)
* @param[in] up                       - default user priority (Vlan Priority Tag).
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or port number
* @retval GT_OUT_OF_RANGE          - when up is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCosPortDefaultUpSet
(
    IN GT_U32    portNum,
    IN GT_U8    up
);

/**
* @internal prvTgfCosPortDefaultUpGet function
* @endinternal
*
* @brief   Get default user priority (VPT) for untagged packet to a given port.
*
* @param[in] devNum                   - packet processor device number.
* @param[in] portNum                  - port number, CPU port.
*                                      In eArch devices portNum is default ePort.
*
* @param[out] upPtr                    - (pointer to) default user priority (VPT).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_OUT_OF_RANGE          - on bad input or output argument
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCosPortDefaultUpGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U8           *upPtr
);

/**
* @internal prvTgfCosPortDefaultTcSet function
* @endinternal
*
* @brief   Set default Traffic Class for untagged packet per port.
*
* @note   APPLICABLE DEVICES:      Puma2; Puma3.
* @param[in] portNum                  - port number.(Not CPU port)
* @param[in] tc                       - default traffic class.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or port number
* @retval GT_OUT_OF_RANGE          - when up is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCosPortDefaultTcSet
(
    IN GT_U32    portNum,
    IN GT_U32   tc
);

/**
* @internal prvTgfCosPortDefaultDpSet function
* @endinternal
*
* @brief   Set default Drop Precedence for untagged packet per port.
*
* @note   APPLICABLE DEVICES:      Puma2; Puma3.
* @param[in] portNum                  - port number.(Not CPU port)
* @param[in] dp                       - default Drop Precedence
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or port number
* @retval GT_OUT_OF_RANGE          - when up is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCosPortDefaultDpSet
(
    IN GT_U32                portNum,
    IN CPSS_DP_LEVEL_ENT    dp
);

/**
* @internal prvTgfCosUpToQosParamMapSet function
* @endinternal
*
* @brief   Maps the VLAN Tagged packet QoS parameters to a QoS Parameters
*         (dp, tc, exp and dscp) according to packet's user priority.
*         Relevant when packet is VLAN/Priority tagged and port trusts UP.
* @param[in] up                       - IEEEE 802.1p User Priority (0..7),
* @param[in] qosParamPtr              - pointer to qos parameters,
*                                      The User Priority in qosParam is ignored.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, up or dp
* @retval GT_OUT_OF_RANGE          - QoS parameter is out of range
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCosUpToQosParamMapSet
(
    IN GT_U32                       up,
    IN PRV_TGF_COS_PROFILE_STC     *qosParamPtr
);

/**
* @internal prvTgfCosUnkUcEnableSet function
* @endinternal
*
* @brief   Enable or disable assignment of a default QoS profile (MAC QoS entry#8 )
*         for Unknown Unicast packets.
*         When the assignment is enabled, its corresponding packets are
*         unconditionally assigned QoS based on the MAC QoS Entry #8 (regardless
*         of SA QoS)
*         If the the assignment is disabled, it's corresponding packets are
*         assigned QoS based on the regular algorithm (MAC-SA QoS if found, or
*         preserve previous settings if MAC-SA not found)
* @param[in] enable                   - GT_TRUE = the corresponding packets are unconditionally
*                                      assigned QoS based on the Entry #8
*                                      (regardless of SA QoS)
*                                      GT_FALSE = the corresponding packets are assigned QoS
*                                      based on the regular algorithm
*                                      (MAC-SA QoS if found, or preserve previous
*                                      settings if MAC-SA not found)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCosUnkUcEnableSet
(
    IN GT_BOOL  enable
);

/**
* @internal prvTgfCosMacQosEntrySet function
* @endinternal
*
* @brief   Set the QoS Attribute of the MAC QoS Table Entry
*
* @param[in] entryIdx                 - Index of a MAC QoS table's entry. Valid values 0 - 8.
* @param[in] macQosCfgPtr             - Pointer to QoS related configuration of a MAC QoS
*                                      table's entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, entryIndex or macQosCfgPtr
* @retval GT_OUT_OF_RANGE          - QoS parameter out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Indexes 0-7 are approached from FDB and index 8 is
*       reserved for unknown unicast or unregistered
*       multicast and broadcast packets.
*
*/
GT_STATUS prvTgfCosMacQosEntrySet
(
    IN GT_U32                           entryIdx,
    IN PRV_TGF_QOS_ENTRY_STC            *macQosCfgPtr
);

/**
* @internal prvTgfCosMacQosConflictResolutionSet function
* @endinternal
*
* @brief   Configure QoS Marking Conflict Mode.
*
* @param[in] macQosResolvType         - Enum holding six attributes for selecting the
*                                      SA command or the DA command.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, entryIndex or macQosResolvType
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCosMacQosConflictResolutionSet
(
    IN PRV_TGF_QOS_MAC_RESOLVE_ENT  macQosResolvType
);


/**
* @internal qosMappingTableIterator function
* @endinternal
*
* @brief   run a test under qos mapping table integrator.
*
* @param[in] testName                 - the test that relevant to Qos.
*                                       None
*/
void qosMappingTableIterator(
    IN UT_TEST_FUNC_TYPE testName
);

/**
* @internal prvTgfCosPortTrustQosMappingTableIndexSet function
* @endinternal
*
* @brief   Set per ingress port the 'Trust Qos Mapping Table Index' (table selector).
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - ingress port number including CPU port.
* @param[in] useUpAsIndex             - indication if the 'Trust Qos Mapping Table Index' selected
*                                      according to the UP (user priority - 0..7) of the packet or
*                                      according the mappingTableIndex parameter (0..11).
*                                      GT_TRUE - according to the UP (user priority) of the packet.
*                                      GT_FALSE - according the mappingTableIndex parameter.
* @param[in] mappingTableIndex        - the  'Trust Qos Mapping Table Index' (table selector).
*                                      the value is ignored when useUpAsIndex == GT_TRUE
*                                      [APPLICABLE RANGE: 0..11]
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, portNum or up
* @retval GT_OUT_OF_RANGE          - mappingTableIndex > 11 and useUpAsIndex == GT_FALSE
* @retval GT_HW_ERROR              - on HW error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCosPortTrustQosMappingTableIndexSet
(
    IN GT_U8                    devNum,
    IN GT_PORT_NUM              portNum,
    IN GT_BOOL                  useUpAsIndex,
    IN GT_U32                   mappingTableIndex
);

/**
* @internal prvTgfCosPortTrustQosMappingTableIndexGet function
* @endinternal
*
* @brief   Get per ingress port the 'Trust Qos Mapping Table Index' (table selector).
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - ingress port number including CPU port.
*
* @param[out] useUpAsIndexPtr          - indication if the 'Trust Qos Mapping Table Index' selected
*                                      according to the UP (user priority - 0..7) of the packet or
*                                      according the mappingTableIndex parameter (0..11).
*                                      GT_TRUE - according to the UP (user priority) of the packet.
*                                      GT_FALSE - according the (mappingTableIndexPtr) parameter.
* @param[out] mappingTableIndexPtr     - the  'Trust Qos Mapping Table Index' (table selector).
*                                      the value is relevant only when (useUpAsIndexPtr) == GT_FALSE
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, portNum or up
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on HW error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCosPortTrustQosMappingTableIndexGet
(
    IN GT_U8                    devNum,
    IN GT_PORT_NUM              portNum,
    OUT GT_BOOL                 *useUpAsIndexPtr,
    OUT GT_U32                  *mappingTableIndexPtr
);

/**
* @internal prvTgfCosDscpMutationEntrySet function
* @endinternal
*
* @brief   Maps the packet DSCP to a new, mutated DSCP.
*         APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum  - device number.
*         mappingTableIndex - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*         [APPLICABLE RANGE : 0..11]
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*                                      [APPLICABLE RANGE : 0..11]
*                                      APPLICABLE DEVICES : Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
* @param[in] dscp                     - DSCP of a IP packet [0..63]
* @param[in] newDscp                  - new DSCP, which is assigned to a IP packet with the DSCP on
*                                      ports with enabled trust L3 or trust L2-L3 and enabled DSCP mutation.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, dscp or newDscp
*                                       or mappingTableIndex > 11 (APPLICABLE DEVICES : Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCosDscpMutationEntrySet
(
    IN GT_U8   devNum,
    IN GT_U32   mappingTableIndex,
    IN GT_U8   dscp,
    IN GT_U8   newDscp
);

/**
* @internal prvTgfCosDscpMutationEntryGet function
* @endinternal
*
* @brief   Get the Mapping: packet DSCP to a new, mutated DSCP.
*         APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum  - device number.
*         mappingTableIndex - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*         [APPLICABLE RANGE : 0..11]
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*                                      [APPLICABLE RANGE : 0..11]
*                                      APPLICABLE DEVICES : Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
* @param[in] dscp                     - DSCP of a IP packet [0..63]
*
* @param[out] newDscpPtr               - new DSCP, which is assigned to a IP packet with the DSCP on
*                                      ports with enabled trust L3 or trust L2-L3 and enabled DSCP mutation.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, dscp
*                                       or mappingTableIndex > 11 (APPLICABLE DEVICES : Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCosDscpMutationEntryGet
(
    IN  GT_U8   devNum,
    IN GT_U32   mappingTableIndex,
    IN  GT_U8   dscp,
    OUT GT_U8  *newDscpPtr
);

/**
* @internal prvTgfCosPortReMapDSCPSet function
* @endinternal
*
* @brief   Enable/Disable DSCP-to-DSCP re-mapping.
*         Relevant for L2-L3 or L3 trust modes.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port.
* @param[in] enableDscpMutation       - GT_TRUE: Packet's DSCP is remapped.
*                                      GT_FALSE: Packet's DSCP is not remapped.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or port.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCosPortReMapDSCPSet
(
    IN GT_U8                        devNum,
    IN GT_PORT_NUM                  portNum,
    IN GT_BOOL                      enableDscpMutation
);

/**
* @internal prvTgfCosPortReMapDSCPGet function
* @endinternal
*
* @brief   Get status (Enabled/Disabled) for DSCP-to-DSCP re-mapping.
*         Relevant for L2-L3 or L3 trust modes.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port.
*
* @param[out] enableDscpMutationPtr    - GT_TRUE: Packet's DSCP is remapped.
*                                      GT_FALSE: Packet's DSCP is not remapped.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or port.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCosPortReMapDSCPGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enableDscpMutationPtr
);


/**
* @internal prvTgfCosPortEgressQoStcDpMappingEnableGet function
* @endinternal
*
* @brief   Get enable/disable status of Egress port (TC, DP) mapping to (UP,EXP,DSCP).
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - egress port number.
*
* @param[out] enablePtr                - pointer to enable/disable Egress port {TC, DP} mapping status to {UP,EXP,DSCP} .
*                                      GT_TRUE  -   mapping  is enabled.
*                                      GT_FALSE -  mapping is disabled.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or port.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCosPortEgressQoStcDpMappingEnableGet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal prvTgfChCosPortEgressQoStcDpMappingEnableSet function
* @endinternal
*
* @brief   Enable/disable Egress port (TC, DP) mapping to (UP,EXP,DSCP).
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - egress port number.
* @param[in] enable                   - enable/disable Egress port {TC, DP} mapping .
*                                      GT_TRUE  - enable mapping.
*                                      GT_FALSE - disable mapping.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or port.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfChCosPortEgressQoStcDpMappingEnableSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
);

/**
* @internal prvTgfCosPortEgressQoSUpMappingEnableGet function
* @endinternal
*
* @brief   Get enable/disable status of Egress UP Mapping.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - egress port number.
*
* @param[out] enablePtr                - pointer to enable/disable Egress UP Mapping status.
*                                      GT_TRUE  -  mapping is enabled.
*                                      GT_FALSE -  mapping is disabled.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or port.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCosPortEgressQoSUpMappingEnableGet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    OUT GT_BOOL         *enablePtr
);



/**
* @internal prvTgfCosTrustDsaTagQosModeSet function
* @endinternal
*
* @brief   Set trust DSA tag QoS mode. To enable end to end QoS in cascade system,
*         the DSA carries QoS profile assigned to the packet in previous device.
*         When the mode is set to GT_TRUE and the packet's DSA tag contains QoS
*         profile, the QoS profile assigned to the packet is assigned from the
*         DSA tag.
*         Relevant for cascading port only.
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port.
* @param[in] enable                   - trust DSA tag QoS mode.
*                                      GT_FALSE - The QoS profile isn't assigned from DSA tag.
*                                      GT_TRUE  - If the packets DSA tag contains a QoS profile,
*                                      it is assigned to the packet.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or port.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCosTrustDsaTagQosModeSet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
);


/**
* @internal prvTgfCosTrustDsaTagQosModeGet function
* @endinternal
*
* @brief   Get trust DSA tag QoS mode. To enable end to end QoS in cascade system,
*         the DSA carries QoS profile assigned to the packet in previous device.
*         When the mode is GT_TRUE and the packet's DSA tag contains QoS
*         profile, the QoS profile assigned to the packet is assigned from the
*         DSA tag.
*         Relevant for cascading port only.
* @param[in] devNum                   - device number.
* @param[in] portNum                  -  physical port number, CPU port.
*
* @param[out] enablePtr                - trust DSA tag QoS mode.
*                                      GT_FALSE - The QoS profile isn't assigned from DSA tag.
*                                      GT_TRUE  - If the packets DSA tag contains a QoS profile,
*                                      it is assigned to the packet.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or port.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCosTrustDsaTagQosModeGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
);



/**
* @internal prvTgfCosPortEgressQoSUpMappingEnableSet function
* @endinternal
*
* @brief   Enable/disable Egress port UP Mapping.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - egress port number.
* @param[in] enable                   - enable/disable Egress port UP Mapping.
*                                      GT_TRUE  - enable mapping.
*                                      GT_FALSE - disable mapping.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or port.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCosPortEgressQoSUpMappingEnableSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
);

/**
* @internal prvTgfCosPortEgressQoSDscpMappingEnableGet function
* @endinternal
*
* @brief   Get enable/disable status of Egress Port DSCP Mapping.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - egress port number.
*
* @param[out] enablePtr                - pointer to enable/disable Egress port DSCP Mapping status.
*                                      GT_TRUE  -  mapping is enabled.
*                                      GT_FALSE -  mappingis disabled.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or port.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCosPortEgressQoSDscpMappingEnableGet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    OUT GT_BOOL         *enablePtr
);


/**
* @internal prvTgfCosPortEgressQoSDscpMappingEnableSet function
* @endinternal
*
* @brief   Enable/disable Egress port DSCP Mapping.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - egress port number.
* @param[in] enable                   - enable/disable Egress port DSCP Mapping.
*                                      GT_TRUE  - enable mapping.
*                                      GT_FALSE - disable mapping.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or port.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCosPortEgressQoSDscpMappingEnableSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
);

/**
* @internal prvTgfCosPortEgressQoSMappingTableIndexGet function
* @endinternal
*
*
* @param[out] mappingTableIndexPtr     - pointer to Egress Qos Mapping Table Index (table set selector).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, portNum or mappingTableIndex
* @retval GT_HW_ERROR              - on HW error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCosPortEgressQoSMappingTableIndexGet
(
    IN GT_U8                    devNum,
    IN GT_PORT_NUM              portNum,
    OUT GT_U32                  *mappingTableIndexPtr
);

/**
* @internal prvTgfCosPortEgressQoSMappingTableIndexSet function
* @endinternal
*
* @brief   Set the table set index for egress port QoS remapping.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - egress port number.
* @param[in] mappingTableIndex        - Egress Qos Mapping Table Index (table set selector).
*                                      (APPLICABLE RANGES : 0..11).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, portNum or mappingTableIndex
* @retval GT_HW_ERROR              - on HW error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCosPortEgressQoSMappingTableIndexSet
(
    IN GT_U8                    devNum,
    IN GT_PORT_NUM              portNum,
    IN GT_U32                   mappingTableIndex
);

/**
* @internal prvTgfCosEgressTcDp2UpExpDscpMappingEntryGet function
* @endinternal
*
* @brief   Get Egress up,exp,dscp values mapped for current (tc, dp).
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - egress mapping table index
*                                      (APPLICABLE RANGES : 0..11).
* @param[in] tc                       - traffic class assigned to packet  (APPLICABLE RANGES: 0..7).
* @param[in] dp                       - drop precedence (APPLICABLE RANGES: 0..2).
*
* @param[out] upPtr                    - pointer to up value.
* @param[out] expPtr                   - pointer to exp value.
* @param[out] dscpPtr                  - pointer to dscp value.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, or tc, or dp, or mappingTableIndex .
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfCosEgressTcDp2UpExpDscpMappingEntryGet
(
    IN GT_U8              devNum,
    IN GT_U32             mappingTableIndex,
    IN GT_U32             tc,
    IN CPSS_DP_LEVEL_ENT  dp,
    OUT GT_U32            *upPtr,
    OUT GT_U32            *expPtr,
    OUT GT_U32            *dscpPtr
);

/**
* @internal prvTgfCosEgressTcDp2UpExpDscpMappingEntrySet function
* @endinternal
*
* @brief   Set Egress (TC, DP) mapping to (UP,EXP,DSCP).
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - egress mapping table index
*                                      (APPLICABLE RANGES : 0..11).
* @param[in] tc                       - traffic class assigned to packet  (APPLICABLE RANGES: 0..7).
* @param[in] dp                       - drop precedence (APPLICABLE RANGES: 0..2).
* @param[in] up                       -  value (APPLICABLE RANGES: 0..7).
* @param[in] exp                      -  value (APPLICABLE RANGES: 0..7).
* @param[in] dscp                     -  value (APPLICABLE RANGES: 0..63).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, or tc,dp, or up,exp,dscp or mappingTableIndex .
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfCosEgressTcDp2UpExpDscpMappingEntrySet
(
    IN GT_U8              devNum,
    IN GT_U32             mappingTableIndex,
    IN GT_U32             tc,
    IN CPSS_DP_LEVEL_ENT  dp,
    IN GT_U32             up,
    IN GT_U32             exp,
    IN GT_U32             dscp
);


/**
* @internal prvTgfCosEgressUp2UpMappingEntryGet function
* @endinternal
*
* @brief   Get egress mapped packet Up value for current Up.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - egress mapping table index
*                                      (APPLICABLE RANGES : 0..11)
* @param[in] up                       - current packet    (APPLICABLE RANGES: 0..7).
*
* @param[out] newUpPtr                 - pointer to mapped up value.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, or up, or newUp, or mappingTableIndex .
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfCosEgressUp2UpMappingEntryGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  mappingTableIndex,
    IN  GT_U32  up,
    OUT GT_U32  *newUpPtr
);


/**
* @internal prvTgfCosEgressUp2UpMappingEntrySet function
* @endinternal
*
* @brief   Egress mapping of current packet up to a new up value.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - egress mapping table index
*                                      (APPLICABLE RANGES : 0..11)
* @param[in] up                       - current packet    (APPLICABLE RANGES: 0..7).
* @param[in] newUp                    - new Up assigned to packet (APPLICABLE RANGES: 0..7).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, or up, or newUp, or mappingTableIndex .
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfCosEgressUp2UpMappingEntrySet
(
    IN GT_U8   devNum,
    IN GT_U32  mappingTableIndex,
    IN GT_U32  up,
    IN GT_U32  newUp
);

/**
* @internal prvTgfCosEgressDscp2DscpMappingEntryGet function
* @endinternal
*
* @brief   Get egress mapped packet DSCP value for current dscp.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - egress mapping table index (see API cpssDxChCosPortEgressQoSMappingTableIndexSet)
*                                      (APPLICABLE RANGES : 0..11)
* @param[in] dscp                     - current  DSCP  (APPLICABLE RANGES: 0..63).
*
* @param[out] newDscpPtr               - pointer to mapped DSCP value.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, or exp, or newExp, or mappingTableIndex .
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfCosEgressDscp2DscpMappingEntryGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  mappingTableIndex,
    IN  GT_U32  dscp,
    OUT GT_U32  *newDscpPtr
);

/**
* @internal prvTgfCosEgressDscp2DscpMappingEntrySet function
* @endinternal
*
* @brief   Egress mapping of current DSCP to a new DSCP value.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - egress mapping table index
*                                      (APPLICABLE RANGES : 0..11)
* @param[in] dscp                     - current DSCP  (APPLICABLE RANGES: 0..63).
* @param[in] newDscp                  - new DSCP assigned to packet(APPLICABLE RANGES: 0..63).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, or exp, or newExp, or mappingTableIndex .
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfCosEgressDscp2DscpMappingEntrySet
(
    IN GT_U8   devNum,
    IN GT_U32  mappingTableIndex,
    IN GT_U32  dscp,
    IN GT_U32  newDscp
);

/**
* @internal prvTgfCosPortEgressQoSExpMappingEnableGet function
* @endinternal
*
* @brief   Get enable/disable status of Egress port EXP Mapping.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - egress port number.
*
* @param[out] enablePtr                - pointer to enable/disable Egress EXP Mapping status on the egress port.
*                                      GT_TRUE  -  mapping is enabled.
*                                      GT_FALSE -  mapping is disabled.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or port.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCosPortEgressQoSExpMappingEnableGet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal prvTgfCosPortEgressQoSExpMappingEnableSet function
* @endinternal
*
* @brief   Enable/disable Egress port EXP Mapping.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - egress port number.
* @param[in] enable                   - enable/disable Egress port EXP Mapping.
*                                      GT_TRUE  - enable mapping.
*                                      GT_FALSE - disable mapping.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or port.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCosPortEgressQoSExpMappingEnableSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
);

/**
* @internal prvTgfCosEgressExp2ExpMappingEntryGet function
* @endinternal
*
* @brief   Get egress mapped packet exp value for current exp.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - egress mapping table index
*                                      (APPLICABLE RANGES : 0..11)
* @param[in] exp                      - current packet    (APPLICABLE RANGES: 0..7).
*
* @param[out] newExpPtr                - pointer to mapped Exp value.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, or exp, or newExp, or mappingTableIndex .
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfCosEgressExp2ExpMappingEntryGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  mappingTableIndex,
    IN GT_U32   exp,
    OUT GT_U32  *newExpPtr
);

/**
* @internal prvTgfCosEgressExp2ExpMappingEntrySet function
* @endinternal
*
* @brief   Egress mapping of current packet Exp to a new Exp value.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - egress mapping table index
*                                      (APPLICABLE RANGES : 0..11)
* @param[in] exp                      - current  (APPLICABLE RANGES: 0..7).
* @param[in] newExp                   - new Exp assigned to packet (APPLICABLE RANGES: 0..7).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, or exp, or newExp, or mappingTableIndex .
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfCosEgressExp2ExpMappingEntrySet
(
    IN GT_U8   devNum,
    IN GT_U32  mappingTableIndex,
    IN GT_U32  exp,
    IN GT_U32  newExp
);

/**
* @internal prvTgfCosPortDpToCfiDeiMapEnableGet function
* @endinternal
*
* @brief   Gets status (Enabled/Disabled) of Drop Precedence mapping to
*         Drop Eligibility Indicator bit.
*         When enabled on egress port the DEI(CFI) bit at the outgoing VLAN
*         tag is set according to the packet DP(after mapping).
* @param[in] devNum                   - device number.
* @param[in] portNum                  - egress port number including CPU port.
*
* @param[out] enablePtr                - (pointer to) value of DP-to-CFI/DEI mapping on the
*                                      egress port (enable/disable):
*                                      GT_TRUE  - DP-to-CFI/DEI mapping enable on port.
*                                      GT_FALSE - DP-to-CFI/DEI mapping disable on port.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or port.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCosPortDpToCfiDeiMapEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal prvTgfCosPortDpToCfiDeiMapEnableSet function
* @endinternal
*
* @brief   Enables or disables mapping of Drop Precedence to Drop Eligibility
*         Indicator bit.
*         When enabled on egress port the DEI(CFI) bit at the outgoing VLAN
*         tag is set according to the packet DP(after mapping).
* @param[in] devNum                   - device number.
* @param[in] portNum                  - egress port number including CPU port.
* @param[in] enable                   - enable/disable DP-to-CFI/DEI mapping on the-to-CFI/DEI mapping on the
*                                      egress port:
*                                      GT_TRUE  - DP-to-CFI/DEI mapping enable on port.
*                                      GT_FALSE - DP-to-CFI/DEI mapping disable on port.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or port.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCosPortDpToCfiDeiMapEnableSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
);

/**
* @internal prvTgfCosDpToCfiDeiMapGet function
* @endinternal
*
* @brief   Gets Drop Precedence mapping to Drop Eligibility Indicator bit.
*         Ports that support S-Tags can be enabled to map the packet's DP
*         (derived from QoS Profile assignment) to the DEI bit(appeared
*         in the outgoing S-Tag) by cpssDxChCosPortDpToCfiDeiMapEnableSet().
* @param[in] devNum                   - device number.
* @param[in] dp                       - Drop Precedence [Green, Red].
*
* @param[out] cfiDeiBitPtr             - (pointer to) value of CFI/DEI bit.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or dp.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Three DP levels are mapped to only two values of CFI/DEI bit.
*
*/
GT_STATUS prvTgfCosDpToCfiDeiMapGet
(
    IN  GT_U8               devNum,
    IN  CPSS_DP_LEVEL_ENT   dp,
    OUT GT_U8               *cfiDeiBitPtr
);






#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __tgfCosGenh */


