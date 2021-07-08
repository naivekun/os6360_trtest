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
* @file cpssDxChCos.h
*
* @brief CPSS Cos facility API.
*
*
*
* @version   30
********************************************************************************
*/

#ifndef __cpssDxChCosh
#define __cpssDxChCosh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cos/cpssCosTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>


/**
* @struct CPSS_DXCH_COS_PROFILE_STC
 *
 * @brief The Prestera Class-of-Service profile parameters.
*/
typedef struct{

    /** drop precedence (color). */
    CPSS_DP_LEVEL_ENT dropPrecedence;

    /** IEEEE 802.1p User Priority (APPLICABLE RANGES: 0..7). */
    GT_U32 userPriority;

    /** egress tx traffic class queue (APPLICABLE RANGES: 0..7). */
    GT_U32 trafficClass;

    /** ip DCSP field (APPLICABLE RANGES: 0..63). */
    GT_U32 dscp;

    /** @brief MPLS label exp value (APPLICABLE RANGES: 0..7)
     *  (APPLICABLE DEVICES: DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X).
     */
    GT_U32 exp;

} CPSS_DXCH_COS_PROFILE_STC;




/**
* @internal cpssDxChCosProfileEntrySet function
* @endinternal
*
* @brief   Configures the Profile Table Entry and
*         Initial Drop Precedence (DP) for Policer.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] profileIndex             - index of a profile in the profile table
*                                      (APPLICABLE RANGES: DxCh1, DxCh1_Diamond 0..71)
*                                      (APPLICABLE RANGES: DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2 0..127).
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..1023).
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
GT_STATUS cpssDxChCosProfileEntrySet
(
     IN  GT_U8                      devNum,
     IN  GT_U32                     profileIndex,
     IN  CPSS_DXCH_COS_PROFILE_STC  *cosPtr
);

/**
* @internal cpssDxChCosProfileEntryGet function
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
*                                      (APPLICABLE RANGES: DxCh1, DxCh1_Diamond 0..71)
*                                      (APPLICABLE RANGES: DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2 0..127).
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..1023).
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
GT_STATUS cpssDxChCosProfileEntryGet
(
     IN  GT_U8                      devNum,
     IN  GT_U32                     profileIndex,
     OUT CPSS_DXCH_COS_PROFILE_STC  *cosPtr
);

/**
* @internal cpssDxChCosDscpToProfileMapSet function
* @endinternal
*
* @brief   Maps the packet DSCP (or remapped DSCP) to a QoS Profile.
*         APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum    - device number.
*         mappingTableIndex - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*         (APPLICABLE RANGES : Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..11)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*                                      (APPLICABLE RANGES : Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..11)
*                                      APPLICABLE DEVICES : Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] dscp                     - DSCP of a IP packet (APPLICABLE RANGES: 0..63).
* @param[in] profileIndex             - profile index, which is assigned to a IP packet with
*                                      the DSCP on ports with enabled trust L3 or trust L2-L3.
*                                      (APPLICABLE RANGES: DxCh1, DxCh1_Diamond 0..71)
*                                      (APPLICABLE RANGES: DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2 0..127)
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..1023).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, dscp or profileIndex
*                                       or mappingTableIndex > 11 (APPLICABLE DEVICES : Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosDscpToProfileMapSet
(
    IN GT_U8    devNum,
    IN GT_U32   mappingTableIndex,
    IN GT_U8    dscp,
    IN GT_U32   profileIndex
);

/**
* @internal cpssDxChCosDscpToProfileMapGet function
* @endinternal
*
* @brief   Get the Mapping: packet DSCP (or remapped DSCP) to a QoS Profile.
*         APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum    - device number.
*         mappingTableIndex - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*         (APPLICABLE RANGES : Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..11)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*                                      (APPLICABLE RANGES : Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..11)
*                                      APPLICABLE DEVICES : Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
* @param[in] dscp                     - DSCP of a IP packet (APPLICABLE RANGES: 0..63)
*
* @param[out] profileIndexPtr          - (pointer to) profile index, which is assigned to
*                                      an IP packet with the DSCP on ports with enabled
*                                      trust L3 or trust L2-L3.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, dscp .
*                                       or mappingTableIndex > 11 (APPLICABLE DEVICES : Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosDscpToProfileMapGet
(
    IN  GT_U8    devNum,
    IN GT_U32   mappingTableIndex,
    IN  GT_U8    dscp,
    OUT GT_U32  *profileIndexPtr
);

/**
* @internal cpssDxChCosDscpMutationEntrySet function
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
*         (APPLICABLE RANGES : Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..11)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*                                      (APPLICABLE RANGES : Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..11)
*                                      APPLICABLE DEVICES : Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
* @param[in] dscp                     - DSCP of a IP packet (APPLICABLE RANGES: 0..63).
* @param[in] newDscp                  - new DSCP, which is assigned to a IP packet with the DSCP on
*                                      ports with enabled trust L3 or trust L2-L3 and enabled DSCP mutation.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, dscp or newDscp
*                                       or mappingTableIndex > 11 (APPLICABLE DEVICES : Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosDscpMutationEntrySet
(
    IN GT_U8   devNum,
    IN GT_U32   mappingTableIndex,
    IN GT_U8   dscp,
    IN GT_U8   newDscp
);

/**
* @internal cpssDxChCosDscpMutationEntryGet function
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
*         (APPLICABLE RANGES : Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..11)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*                                      (APPLICABLE RANGES : Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..11)
*                                      APPLICABLE DEVICES : Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
* @param[in] dscp                     - DSCP of a IP packet (APPLICABLE RANGES: 0..63)
*
* @param[out] newDscpPtr               - new DSCP, which is assigned to a IP packet with the DSCP on
*                                      ports with enabled trust L3 or trust L2-L3 and enabled DSCP mutation.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, dscp
*                                       or mappingTableIndex > 11 (APPLICABLE DEVICES : Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosDscpMutationEntryGet
(
    IN  GT_U8   devNum,
    IN GT_U32   mappingTableIndex,
    IN  GT_U8   dscp,
    OUT GT_U8  *newDscpPtr
);

/**
* @internal cpssDxChCosUpCfiDeiToProfileMapSet function
* @endinternal
*
* @brief   Maps the UP Profile Index and packet's User Priority and CFI/DEI bit (Canonical Format
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
*         (APPLICABLE RANGES : Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..11)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*                                      (APPLICABLE RANGES : Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..11)
*                                      APPLICABLE DEVICES : Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
* @param[in] upProfileIndex           - the UP profile index (table selector)
*                                      (APPLICABLE DEVICES Lion; xCat2; Lion2).
*                                      see also API : cpssDxChCosPortUpProfileIndexSet
* @param[in] up                       - User Priority of a VLAN or DSA tagged packet (APPLICABLE RANGES: 0..7).
* @param[in] cfiDeiBit                - value of CFI/DEI bit (APPLICABLE RANGES: 0..1).
*                                      (APPLICABLE DEVICES DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3).
* @param[in] profileIndex             - QoS Profile index, which is assigned to a VLAN or DSA
*                                      tagged packet with the UP on ports with enabled trust
*                                      L2 or trust L2-L3.
*                                      (APPLICABLE RANGES: DxCh1, DxCh1_Diamond 0..71)
*                                      (APPLICABLE RANGES: DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2 0..127)
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..1023).
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, up or cfiDeiBit.
*                                       or mappingTableIndex > 11 (APPLICABLE DEVICES : Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
* @retval GT_OUT_OF_RANGE          - on profileIndex out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosUpCfiDeiToProfileMapSet
(
    IN GT_U8    devNum,
    IN GT_U32   mappingTableIndex,
    IN GT_U32   upProfileIndex,
    IN GT_U8    up,
    IN GT_U8    cfiDeiBit,
    IN GT_U32   profileIndex
);

/**
* @internal cpssDxChCosUpCfiDeiToProfileMapGet function
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
*         (5APPLICABLE RANGES : Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon, Aldrin, AC3X 0..11)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*                                      (5APPLICABLE RANGES : Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon, Aldrin, AC3X 0..11)
*                                      APPLICABLE DEVICES : Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
* @param[in] upProfileIndex           - the UP profile index (table selector)
*                                      (APPLICABLE DEVICES Lion; xCat2; Lion2).
*                                      see also API : cpssDxChCosPortUpProfileIndexSet
* @param[in] up                       - User Priority of a VLAN or DSA tagged packet (APPLICABLE RANGES: 0..7).
* @param[in] cfiDeiBit                - value of CFI/DEI bit (APPLICABLE RANGES: 0..1).
*                                      (APPLICABLE DEVICES DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3).
*
* @param[out] profileIndexPtr          - (pointer to)QoS Profile index, which is assigned to a
*                                      VLAN or DSA tagged packet with the UP on ports with
*                                      enabled trust L2 or trust L2-L3.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, up or cfiDeiBit.
*                                       or mappingTableIndex > 11 (APPLICABLE DEVICES : Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosUpCfiDeiToProfileMapGet
(
    IN  GT_U8   devNum,
    IN GT_U32   mappingTableIndex,
    IN GT_U32   upProfileIndex,
    IN  GT_U8   up,
    IN  GT_U8   cfiDeiBit,
    OUT GT_U32 *profileIndexPtr
);

/**
* @internal cpssDxChCosPortDpToCfiDeiMapEnableSet function
* @endinternal
*
* @brief   Enables or disables mapping of Drop Precedence to Drop Eligibility
*         Indicator bit.
*         When enabled on egress port the DEI(CFI) bit at the outgoing VLAN
*         tag is set according to the packet DP(after mapping).
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
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
GT_STATUS cpssDxChCosPortDpToCfiDeiMapEnableSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
);

/**
* @internal cpssDxChCosPortDpToCfiDeiMapEnableGet function
* @endinternal
*
* @brief   Gets status (Enabled/Disabled) of Drop Precedence mapping to
*         Drop Eligibility Indicator bit.
*         When enabled on egress port the DEI(CFI) bit at the outgoing VLAN
*         tag is set according to the packet DP(after mapping).
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
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
GT_STATUS cpssDxChCosPortDpToCfiDeiMapEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal cpssDxChCosPortDpToCfiDei1MapEnableSet function
* @endinternal
*
* @brief   Enables or disables mapping of Drop Precedence to Drop Eligibility
*         Indicator 1 bit.
*         When enabled on egress port the DEI(CFI) bit at the outgoing Tag1 VLAN
*         tag is set according to the packet DP(after mapping).
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - egress port number including CPU port.
* @param[in] enable                   - enable/disable DP-to-CFI1/DEI1 mapping on the-to-CFI1/DEI1 mapping on the
*                                      egress port:
*                                      GT_TRUE  - DP-to-CFI1/DEI1 mapping enable on port.
*                                      GT_FALSE - DP-to-CFI1/DEI1 mapping disable on port.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or port.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosPortDpToCfiDei1MapEnableSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
);

/**
* @internal cpssDxChCosPortDpToCfiDei1MapEnableGet function
* @endinternal
*
* @brief   Gets status (Enabled/Disabled) of Drop Precedence mapping to
*         Drop Eligibility Indicator 1 bit.
*         When enabled on egress port the DEI(CFI) bit at the outgoing Tag1 VLAN
*         tag is set according to the packet DP(after mapping).
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - egress port number including CPU port.
*
* @param[out] enablePtr                - (pointer to) value of DP-to-CFI1/DEI1 mapping on the
*                                      egress port (enable/disable):
*                                      GT_TRUE  - DP-to-CFI1/DEI1 mapping enable on port.
*                                      GT_FALSE - DP-to-CFI1/DEI1 mapping disable on port.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or port.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosPortDpToCfiDei1MapEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal cpssDxChCosDpToCfiDeiMapSet function
* @endinternal
*
* @brief   Maps Drop Precedence to Drop Eligibility Indicator bit.
*         Ports that support S-Tags can be enabled to map the packet's DP
*         (derived from QoS Profile assignment) to the DEI bit(appeared
*         in the outgoing S-Tag) by cpssDxChCosPortDpToCfiDeiMapEnableSet().
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
* @param[in] dp                       - Drop Precedence [Green, Red].
* @param[in] cfiDeiBit                - value of CFI/DEI bit (APPLICABLE RANGES: 0..1).
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
GT_STATUS cpssDxChCosDpToCfiDeiMapSet
(
    IN GT_U8                devNum,
    IN CPSS_DP_LEVEL_ENT    dp,
    IN GT_U8                cfiDeiBit
);

/**
* @internal cpssDxChCosDpToCfiDeiMapGet function
* @endinternal
*
* @brief   Gets Drop Precedence mapping to Drop Eligibility Indicator bit.
*         Ports that support S-Tags can be enabled to map the packet's DP
*         (derived from QoS Profile assignment) to the DEI bit(appeared
*         in the outgoing S-Tag) by cpssDxChCosPortDpToCfiDeiMapEnableSet().
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
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
GT_STATUS cpssDxChCosDpToCfiDeiMapGet
(
    IN  GT_U8               devNum,
    IN  CPSS_DP_LEVEL_ENT   dp,
    OUT GT_U8               *cfiDeiBitPtr
);

/**
* @internal cpssDxChCosExpToProfileMapSet function
* @endinternal
*
* @brief   Maps the packet MPLS exp to a QoS Profile.
*         APPLICABLE DEVICES:
*         DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond.
*         INPUTS:
*         devNum     - device number.
*         mappingTableIndex - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*         (APPLICABLE RANGES : Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..11)
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*                                      (APPLICABLE RANGES : Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..11)
*                                      APPLICABLE DEVICES : Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] exp                      - mpls  (APPLICABLE RANGES: 0..7).
* @param[in] profileIndex             - profile index, which is assigned to a VLAN or
*                                      Marvell tagged packet with the up on ports with
*                                      enabled trust L2 or trust L2-L3.
*                                      (APPLICABLE RANGES: DxCh1, DxCh1_Diamond 0..71)
*                                      (APPLICABLE RANGES: DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2 0..127).
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..1023).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, exp or profileIndex
*                                       or mappingTableIndex > 11 (APPLICABLE DEVICES : Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Starting with BobCat2, there is no direct MPLS EXP to QoS profile mapping, rather this function sets
*       MPLS EXP to QoS Profile mappings entry in the Trust QoS Mapping table. Mapping table per ePort is
*       set by cpssDxChCosPortTrustQosMappingTableIndexSet
*
*/
GT_STATUS cpssDxChCosExpToProfileMapSet
(
    IN  GT_U8   devNum,
    IN GT_U32   mappingTableIndex,
    IN  GT_U32  exp,
    IN  GT_U32  profileIndex
);

/**
* @internal cpssDxChCosExpToProfileMapGet function
* @endinternal
*
* @brief   Returns the mapping of packet MPLS exp to a QoS Profile.
*         APPLICABLE DEVICES:
*         DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond.
*         INPUTS:
*         devNum     - device number.
*         mappingTableIndex - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*         (APPLICABLE RANGES : Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..11)
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - mapping table index (see API cpssDxChCosPortTrustQosMappingTableIndexSet)
*                                      (APPLICABLE RANGES : Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..11)
*                                      APPLICABLE DEVICES : Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
* @param[in] exp                      - mpls exp.
*
* @param[out] profileIndexPtr          - (pointer to)profile index, which is assigned to
*                                      a VLAN or Marvell tagged packet with the up on
*                                      ports with enabled trust L2 or trust L2-L3.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, exp
*                                       or mappingTableIndex > 11 (APPLICABLE DEVICES : Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosExpToProfileMapGet
(
    IN  GT_U8   devNum,
    IN GT_U32   mappingTableIndex,
    IN  GT_U32  exp,
    OUT GT_U32  *profileIndexPtr
);

/**
* @internal cpssDxChCosPortQosConfigSet function
* @endinternal
*
* @brief   Configures the port's QoS attributes.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port.
*                                      In eArch devices portNum is default ePort.
* @param[in] portQosCfgPtr            - Pointer to QoS related configuration of a port.
* @param[in] portQosCfgPtr            -
*                                      (APPLICABLE RANGES: DxCh1, DxCh1_Diamond 0..71)
*                                      (APPLICABLE RANGES: DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2, Bobcat2, Caelum, Bobcat3; Aldrin2; Falcon, Aldrin, AC3X 0..127);
* @param[in] portQosCfgPtr            and
* @param[in] portQosCfgPtr
*                                      only [CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E,
*                                      CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E]
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, port or portQosCfgPtr
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosPortQosConfigSet
(
    IN GT_U8               devNum,
    IN GT_PORT_NUM         portNum,
    IN CPSS_QOS_ENTRY_STC  *portQosCfgPtr
);

/**
* @internal cpssDxChCosPortQosConfigGet function
* @endinternal
*
* @brief   Get the port's QoS attributes configuration.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port.
*                                      In eArch devices portNum is default ePort.
*
* @param[out] portQosCfgPtr            - Pointer to QoS related configuration of a port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, port or portQosCfgPtr
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosPortQosConfigGet
(
    IN  GT_U8               devNum,
    IN  GT_PORT_NUM         portNum,
    OUT CPSS_QOS_ENTRY_STC  *portQosCfgPtr
);

/**
* @internal cpssDxChCosMacQosEntrySet function
* @endinternal
*
* @brief   Set the QoS Attribute of the MAC QoS Table Entr
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] entryIdx                 - Index of a MAC QoS table's entry (APPLICABLE RANGES: 1..7).
* @param[in] macQosCfgPtr             - Pointer to QoS related configuration of a MAC QoS table's entry.
* @param[in] macQosCfgPtr             -
*                                      (APPLICABLE RANGES: DxCh1, DxCh1_Diamond 0..71)
*                                      (APPLICABLE RANGES: DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2, Bobcat2, Caelum, Bobcat3; Aldrin2; Falcon, Aldrin, AC3X 0..127).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, entryIndex or macQosCfgPtr
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosMacQosEntrySet
(
    IN GT_U8               devNum,
    IN GT_U32              entryIdx,
    IN CPSS_QOS_ENTRY_STC *macQosCfgPtr
);

/**
* @internal cpssDxChCosMacQosEntryGet function
* @endinternal
*
* @brief   Returns the QoS Attribute of the MAC QoS Table Entry.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] entryIdx                 - Index of a MAC QoS table's entry (APPLICABLE RANGES: 1..7).
*
* @param[out] macQosCfgPtr             - Pointer to QoS related configuration of a MAC QoS
*                                      table's entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, entryIndex or macQosCfgPtr.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosMacQosEntryGet
(
    IN  GT_U8               devNum,
    IN  GT_U32              entryIdx,
    OUT CPSS_QOS_ENTRY_STC *macQosCfgPtr
);

/**
* @internal cpssDxChCosMacQosConflictResolutionSet function
* @endinternal
*
* @brief   Configure QoS Marking Conflict Mode.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] macQosResolvType         - Enum holding two attributes for selecting
*                                      the SA command or the DA command
*                                      (0 - DA command, 1 - SA caommand).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or macQosResolvType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosMacQosConflictResolutionSet
(
    IN GT_U8                        devNum,
    IN CPSS_MAC_QOS_RESOLVE_ENT     macQosResolvType
);

/**
* @internal cpssDxChCosMacQosConflictResolutionGet function
* @endinternal
*
* @brief   Get Configured QoS Marking Conflict Mode.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] macQosResolvTypePtr      - Pointer Enum holding two optional attributes for
*                                      selecting the SA command or the DA
*                                      command (0 - DA command, 1 - SA caommand).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or macQosResolvType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosMacQosConflictResolutionGet
(
    IN  GT_U8                        devNum,
    OUT CPSS_MAC_QOS_RESOLVE_ENT    *macQosResolvTypePtr
);

/**
* @internal cpssDxChCosRemarkTblEntrySet function
* @endinternal
*
* @brief   Sets new profile index for out profile packets.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2.
* @note   NOT APPLICABLE DEVICES:  DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] profileIndex             - profile index in the QoS Profile Table
*                                      (APPLICABLE RANGES: DxCh1, DxCh1_Diamond 0..71)
*                                      (APPLICABLE RANGES: DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2, Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..127).
* @param[in] newProfileIndex          - new profile index, which is assigned for out-profile
*                                      packets according to Traffic Conditioner command.
*                                      (APPLICABLE RANGES: DxCh1, DxCh1_Diamond 0..71)
*                                      (APPLICABLE RANGES: DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2, Bobcat2, Caelum, Bobcat3; Aldrin2; Falcon, Aldrin, AC3X 0..127).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, profileIndex or newProfileIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosRemarkTblEntrySet
(
    IN GT_U8    devNum,
    IN GT_U32   profileIndex,
    IN GT_U32   newProfileIndex
);

/**
* @internal cpssDxChCosRemarkTblEntryGet function
* @endinternal
*
* @brief   Get new profile index setting for out profile packets.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2.
* @note   NOT APPLICABLE DEVICES:  DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] profileIndex             - profile index in the QoS Profile Table
*                                      (APPLICABLE RANGES: DxCh1, DxCh1_Diamond 0..71)
*                                      (APPLICABLE RANGES: DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2, Bobcat2, Caelum, Bobcat3; Aldrin2; Falcon, Aldrin, AC3X 0..127).
*
* @param[out] newProfileIndexPtr       - (pointer to) new profile index, which will be
*                                      assigned for out-profile packets according to
*                                      Traffic Conditioner command.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, profileIndex
* @retval GT_BAD_PTR               - NULL == newProfileIndexPtr
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosRemarkTblEntryGet
(
    IN  GT_U8    devNum,
    IN  GT_U32   profileIndex,
    OUT GT_U32  *newProfileIndexPtr
);

/**
* @internal cpssDxChPortDefaultUPSet function
* @endinternal
*
* @brief   Set default user priority (VPT) for untagged packet to a given port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - packet processor device number.
* @param[in] portNum                  - port number, CPU port.
*                                      In eArch devices portNum is default ePort.
* @param[in] defaultUserPrio          - default user priority (VPT) (APPLICABLE RANGES: 0..7).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortDefaultUPSet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_U8        defaultUserPrio
);

/**
* @internal cpssDxChPortDefaultUPGet function
* @endinternal
*
* @brief   Get default user priority (VPT) for untagged packet to a given port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - packet processor device number.
* @param[in] portNum                  - port number, CPU port.
*                                      In eArch devices portNum is default ePort.
*
* @param[out] defaultUserPrioPtr       - (pointer to) default user priority (VPT).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortDefaultUPGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U8           *defaultUserPrioPtr
);

/**
* @internal cpssDxChPortModifyUPSet function
* @endinternal
*
* @brief   Enable/Disable overriding a tagged packet's User Priority by
*         other assignment mechanisms.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - packet processor device number.
* @param[in] portNum                  - port number, CPU port.
*                                      In eArch devices portNum is default ePort.
* @param[in] upOverrideEnable         - GT_TRUE, enable overriding a tagged packet's
*                                      User Priority by other assignment
*                                      mechanisms.
*                                      GT_FALSE for disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum or port.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortModifyUPSet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      upOverrideEnable
);


/**
* @internal cpssDxChPortModifyUPGet function
* @endinternal
*
* @brief   Get Enable/Disable overriding a tagged packet's User Priority by
*         other assignment mechanisms setting.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - packet processor device number.
* @param[in] portNum                  - port number, CPU port.
*                                      In eArch devices portNum is default ePort.
*
* @param[out] upOverrideEnablePtr      - GT_TRUE, enable overriding a tagged packet's
*                                      User Priority by other assignment
*                                      mechanisms.
*                                      GT_FALSE for disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum or port.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortModifyUPGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *upOverrideEnablePtr
);

/**
* @internal cpssDxChCosPortQosTrustModeSet function
* @endinternal
*
* @brief   Configures port's Trust Mode.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port.
*                                      In eArch devices portNum is default ePort.
* @param[in] portQosTrustMode         - QoS trust mode of a port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, port or trustMode.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosPortQosTrustModeSet
(
    IN GT_U8                         devNum,
    IN GT_PORT_NUM                   portNum,
    IN CPSS_QOS_PORT_TRUST_MODE_ENT  portQosTrustMode
);

/**
* @internal cpssDxChCosPortQosTrustModeGet function
* @endinternal
*
* @brief   Get Configured port's Trust Mode.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port.
*                                      In eArch devices portNum is default ePort.
*
* @param[out] portQosTrustModePtr      - QoS trust mode of a port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, port or trustMode.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosPortQosTrustModeGet
(
    IN  GT_U8                         devNum,
    IN  GT_PORT_NUM                   portNum,
    OUT CPSS_QOS_PORT_TRUST_MODE_ENT  *portQosTrustModePtr
);

/**
* @internal cpssDxChCosPortReMapDSCPSet function
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
*                                      In eArch devices portNum is default ePort.
* @param[in] enableDscpMutation       - GT_TRUE: Packet's DSCP is remapped.
*                                      GT_FALSE: Packet's DSCP is not remapped.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or port.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosPortReMapDSCPSet
(
    IN GT_U8                        devNum,
    IN GT_PORT_NUM                  portNum,
    IN GT_BOOL                      enableDscpMutation
);

/**
* @internal cpssDxChCosPortReMapDSCPGet function
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
*                                      In eArch devices portNum is default ePort.
*
* @param[out] enableDscpMutationPtr    - GT_TRUE: Packet's DSCP is remapped.
*                                      GT_FALSE: Packet's DSCP is not remapped.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or port.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosPortReMapDSCPGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enableDscpMutationPtr
);

/**
* @internal cpssDxChCosPortVlanQoSCfgEntryGet function
* @endinternal
*
* @brief   Read an entry from HW from Ports VLAN and QoS Configuration Table.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] port                     - logical number, CPU port
*
* @param[out] entryPtr                 - pointer to memory where will be placed next entry.
*                                      size 32 Bytes.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or port.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosPortVlanQoSCfgEntryGet
(
    IN    GT_U8         devNum,
    IN    GT_PORT_NUM   port,
    OUT   GT_U32        *entryPtr
);

/**
* @internal cpssDxChCosRemarkEntryGet function
* @endinternal
*
* @brief   Read an entry from HW Policers QoS Remarking and Initial DP Table.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2.
* @note   NOT APPLICABLE DEVICES:  DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] index                    -  of entry
*                                      (APPLICABLE RANGES: DxCh1, DxCh1_Diamond 0..71)
*                                      (APPLICABLE RANGES: DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2, Bobcat2, Caelum, Bobcat3; Aldrin2; Falcon, Aldrin, AC3X 0..127).
*
* @param[out] entryPtr                 - pointer to memory where will be placed next entry. size 4 Bytes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum or entry index.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosRemarkEntryGet
(
    IN    GT_U8     devNum,
    IN    GT_U32    index,
    OUT   GT_U32   *entryPtr
);

/**
* @internal cpssDxChCosQoSProfileEntryGet function
* @endinternal
*
* @brief   Read an entry from HW QoS Profile Table on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] index                    -  of entry
*                                      (APPLICABLE RANGES: DxCh1, DxCh1_Diamond 0..71)
*                                      (APPLICABLE RANGES: DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2, Bobcat2, Caelum, Bobcat3; Aldrin2; Falcon, Aldrin, AC3X 0..127).
*
* @param[out] entryPtr                 - pointer to memory where will be placed next entry. size 4 Bytes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum or entry index.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosQoSProfileEntryGet
(
    IN    GT_U8     devNum,
    IN    GT_U32    index,
    OUT   GT_U32   *entryPtr
);

/**
* @internal cpssDxChCosTrustDsaTagQosModeSet function
* @endinternal
*
* @brief   Set trust DSA tag QoS mode. To enable end to end QoS in cascade system,
*         the DSA carries QoS profile assigned to the packet in previous device.
*         When the mode is set to GT_TRUE and the packet's DSA tag contains QoS
*         profile, the QoS profile assigned to the packet is assigned from the
*         DSA tag.
*         Relevant for cascading port only.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port.
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   - trust DSA tag QoS mode.
*                                      GT_FALSE - The QoS profile isn't assigned from DSA tag.
*                                      GT_TRUE  - If the packets DSA tag contains a QoS profile,
*                                      it is assigned to the packet.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or port.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosTrustDsaTagQosModeSet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
);


/**
* @internal cpssDxChCosTrustDsaTagQosModeGet function
* @endinternal
*
* @brief   Get trust DSA tag QoS mode. To enable end to end QoS in cascade system,
*         the DSA carries QoS profile assigned to the packet in previous device.
*         When the mode is GT_TRUE and the packet's DSA tag contains QoS
*         profile, the QoS profile assigned to the packet is assigned from the
*         DSA tag.
*         Relevant for cascading port only.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number, CPU port.
*                                      In eArch devices portNum is default ePort.
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
GT_STATUS cpssDxChCosTrustDsaTagQosModeGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal cpssDxChCosTrustExpModeSet function
* @endinternal
*
* @brief   Enable/Disable trust EXP mode for MPLS packets.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port.
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   - trust the MPLS header's EXP field.
*                                      GT_FALSE - QoS profile from Exp To Qos Profile table
*                                      not assigned to the MPLS packet.
*                                      GT_TRUE  - QoS profile from Exp To Qos Profile table
*                                      assigned to the MPLS packet.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or port.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosTrustExpModeSet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
);

/**
* @internal cpssDxChCosTrustExpModeGet function
* @endinternal
*
* @brief   Get Enable/Disable state of trust EXP mode for MPLS packets.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port.
*                                      In eArch devices portNum is default ePort.
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
GT_STATUS cpssDxChCosTrustExpModeGet
(
    IN  GT_U8        devNum,
    IN  GT_PORT_NUM  portNum,
    OUT GT_BOOL      *enablePtr
);

/**
* @internal cpssDxChCosL2TrustModeVlanTagSelectSet function
* @endinternal
*
* @brief   Select type of VLAN tag (either Tag0 or Tag1) for Trust L2 Qos mode.
*         APPLICABLE DEVICES:
*         xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*         INPUTS:
*         devNum     - device number.
*         portNum     - port number, CPU port.
*         In eArch devices portNum is default ePort for default VLAN Tag type.
*         isDefaultVlanTagType - indication that the vlanTagType is the default vlanTagType or
*         used for re-parse after TTI lookup:
*         1. Parse inner Layer2 after TT
*         2. Re-parse Layer2 after popping EVB/BPE tag
*         3. Parsing of passenger packet of non-TT packets (TRILL, IP-GRE, MPLS)
*         GT_TRUE - used as default vlanTagType
*         GT_FALSE - used for re-parse after TTI lookup.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port.
*                                      In eArch devices portNum is default ePort for default VLAN Tag type.
* @param[in] isDefaultVlanTagType     - indication that the vlanTagType is the default vlanTagType or
*                                      used for re-parse after TTI lookup:
*                                      1. Parse inner Layer2 after TT
*                                      2. Re-parse Layer2 after popping EVB/BPE tag
*                                      3. Parsing of passenger packet of non-TT packets (TRILL, IP-GRE, MPLS)
*                                      GT_TRUE  - used as default vlanTagType
*                                      GT_FALSE - used for re-parse after TTI lookup.
*                                      APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
* @param[in] vlanTagType              - Vlan Tag Type (tag0 or tag1)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or port or vlanTagType.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosL2TrustModeVlanTagSelectSet
(
    IN GT_U8                     devNum,
    IN GT_PORT_NUM               portNum,
    IN GT_BOOL                   isDefaultVlanTagType,
    IN CPSS_VLAN_TAG_TYPE_ENT    vlanTagType
);

/**
* @internal cpssDxChCosL2TrustModeVlanTagSelectGet function
* @endinternal
*
* @brief   Get Selected type of VLAN tag (either Tag0 or Tag1) for Trust L2 Qos mode.
*         APPLICABLE DEVICES:
*         xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*         INPUTS:
*         devNum   - device number.
*         portNum  - port number, CPU port.
*         In eArch devices portNum is default ePort for default VLAN Tag type.
*         isDefaultVlanTagType - indication that the vlanTagType is the default vlanTagType or
*         used for re-parse after TTI lookup:
*         1. Parse inner Layer2 after TT
*         2. Re-parse Layer2 after popping EVB/BPE tag
*         3. Parsing of passenger packet of non-TT packets (TRILL, IP-GRE, MPLS)
*         GT_TRUE - used as default vlanTagType
*         GT_FALSE - used for re-parse after TTI lookup.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port.
*                                      In eArch devices portNum is default ePort for default VLAN Tag type.
* @param[in] isDefaultVlanTagType     - indication that the vlanTagType is the default vlanTagType or
*                                      used for re-parse after TTI lookup:
*                                      1. Parse inner Layer2 after TT
*                                      2. Re-parse Layer2 after popping EVB/BPE tag
*                                      3. Parsing of passenger packet of non-TT packets (TRILL, IP-GRE, MPLS)
*                                      GT_TRUE  - used as default vlanTagType
*                                      GT_FALSE - used for re-parse after TTI lookup.
*                                      APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
*
* @param[out] vlanTagTypePtr           - (pointer to)Vlan Tag Type (tag0 or tag1)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or port or vlanTagType.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosL2TrustModeVlanTagSelectGet
(
    IN  GT_U8                     devNum,
    IN  GT_PORT_NUM               portNum,
    IN GT_BOOL                   isDefaultVlanTagType,
    OUT CPSS_VLAN_TAG_TYPE_ENT    *vlanTagTypePtr
);

/**
* @internal cpssDxChCosPortUpProfileIndexSet function
* @endinternal
*
* @brief   Set per ingress port and per UP (user priority) the 'UP profile index' (table selector).
*         See also API cpssDxChCosUpCfiDeiToProfileMapSet
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - ingress port number including CPU port.
* @param[in] up                       - user priority of a VLAN or DSA tagged packet (APPLICABLE RANGES: 0..7).
* @param[in] upProfileIndex           - the UP profile index (table selector) (APPLICABLE RANGES: 0..1).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, portNum or up
* @retval GT_OUT_OF_RANGE          - upProfileIndex > 1
* @retval GT_HW_ERROR              - on HW error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosPortUpProfileIndexSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U8                    up,
    IN GT_U32                   upProfileIndex
);

/**
* @internal cpssDxChCosPortUpProfileIndexGet function
* @endinternal
*
* @brief   Get per ingress port and per UP (user priority) the 'UP profile index' (table selector).
*         See also API cpssDxChCosUpCfiDeiToProfileMapSet
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - ingress port number including CPU port.
* @param[in] up                       - user priority of a VLAN or DSA tagged packet (APPLICABLE RANGES: 0..7).
*
* @param[out] upProfileIndexPtr        - (pointer to)the UP profile index (table selector)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, portNum or up.
* @retval GT_HW_ERROR              - on HW error.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosPortUpProfileIndexGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U8                    up,
    OUT GT_U32                  *upProfileIndexPtr
);


/**
* @internal cpssDxChCosPortTrustQosMappingTableIndexSet function
* @endinternal
*
* @brief   Set per ingress port the 'Trust Qos Mapping Table Index' (table selector).
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - ingress port number including CPU port.
*                                      In eArch devices portNum is default ePort.
* @param[in] useUpAsIndex             - indication if the L2 cos parameters using 'Trust Qos Mapping Table Index'
*                                      selected according to the UP (user priority) of the packet or
*                                      according the mappingTableIndex parameter.
*                                      GT_TRUE  - according to the UP (user priority) of the packet.
*                                      GT_FALSE - according the mappingTableIndex parameter.
* @param[in] mappingTableIndex        - the 'Trust Qos Mapping Table Index' (table selector).
*                                      Note: even when useUpAsIndex == GT_TRUE this param maybe used
*                                      for L3 cos parameters. (APPLICABLE RANGES : 0..11)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_OUT_OF_RANGE          - mappingTableIndex > 11
* @retval GT_HW_ERROR              - on HW error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosPortTrustQosMappingTableIndexSet
(
    IN GT_U8                    devNum,
    IN GT_PORT_NUM              portNum,
    IN GT_BOOL                  useUpAsIndex,
    IN GT_U32                   mappingTableIndex
);

/**
* @internal cpssDxChCosPortTrustQosMappingTableIndexGet function
* @endinternal
*
* @brief   Get per ingress port the 'Trust Qos Mapping Table Index' (table selector).
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - ingress port number including CPU port.
*                                      In eArch devices portNum is default ePort.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on HW error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosPortTrustQosMappingTableIndexGet
(
    IN GT_U8                    devNum,
    IN GT_PORT_NUM              portNum,
    OUT GT_BOOL                 *useUpAsIndexPtr,
    OUT GT_U32                  *mappingTableIndexPtr
);


/**
* @internal cpssDxChCosPortEgressQosMappingTableIndexSet function
* @endinternal
*
* @brief   Set the table set index for egress port QoS remapping.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
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
GT_STATUS cpssDxChCosPortEgressQosMappingTableIndexSet
(
    IN GT_U8                    devNum,
    IN GT_PORT_NUM              portNum,
    IN GT_U32                   mappingTableIndex
);


/**
* @internal cpssDxChCosPortEgressQosMappingTableIndexGet function
* @endinternal
*
* @brief   Get the table set index for egress port QoS remapping.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - egress port number.
*
* @param[out] mappingTableIndexPtr     - pointer to Egress Qos Mapping Table Index (table set selector).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, portNum or mappingTableIndex
* @retval GT_HW_ERROR              - on HW error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosPortEgressQosMappingTableIndexGet
(
    IN GT_U8                    devNum,
    IN GT_PORT_NUM              portNum,
    OUT GT_U32                  *mappingTableIndexPtr
);


/**
* @internal cpssDxChCosPortEgressQosExpMappingEnableSet function
* @endinternal
*
* @brief   Enable/disable Egress port EXP Mapping.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
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
GT_STATUS cpssDxChCosPortEgressQosExpMappingEnableSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
);


/**
* @internal cpssDxChCosPortEgressQosExpMappingEnableGet function
* @endinternal
*
* @brief   Get enable/disable status of Egress port EXP Mapping.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
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
GT_STATUS cpssDxChCosPortEgressQosExpMappingEnableGet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    OUT GT_BOOL         *enablePtr
);


/**
* @internal cpssDxChCosPortEgressQosTcDpMappingEnableSet function
* @endinternal
*
* @brief   Enable/disable Egress port (TC, DP) mapping to (UP,EXP,DSCP).
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - egress port number.
* @param[in] enable                   - enable/disable Egress port {TC, DP} mapping.
*                                      GT_TRUE  - enable mapping.
*                                      GT_FALSE - disable mapping.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or port.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCosPortEgressQosTcDpMappingEnableSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
);


/**
* @internal cpssDxChCosPortEgressQosTcDpMappingEnableGet function
* @endinternal
*
* @brief   Get enable/disable status of Egress port (TC, DP) mapping to (UP,EXP,DSCP).
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
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
GT_STATUS cpssDxChCosPortEgressQosTcDpMappingEnableGet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal cpssDxChCosPortEgressQosUpMappingEnableSet function
* @endinternal
*
* @brief   Enable/disable Egress port UP Mapping.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
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
GT_STATUS cpssDxChCosPortEgressQosUpMappingEnableSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
);


/**
* @internal cpssDxChCosPortEgressQosUpMappingEnableGet function
* @endinternal
*
* @brief   Get enable/disable status of Egress UP Mapping.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
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
GT_STATUS cpssDxChCosPortEgressQosUpMappingEnableGet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    OUT GT_BOOL         *enablePtr
);


/**
* @internal cpssDxChCosPortEgressQosDscpMappingEnableSet function
* @endinternal
*
* @brief   Enable/disable Egress port DSCP Mapping.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
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
GT_STATUS cpssDxChCosPortEgressQosDscpMappingEnableSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
);


/**
* @internal cpssDxChCosPortEgressQosDscpMappingEnableGet function
* @endinternal
*
* @brief   Get enable/disable status of Egress Port DSCP Mapping.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
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
GT_STATUS cpssDxChCosPortEgressQosDscpMappingEnableGet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    OUT GT_BOOL         *enablePtr
);


/**
* @internal cpssDxChCosEgressDscp2DscpMappingEntrySet function
* @endinternal
*
* @brief   Egress mapping of current DSCP to a new DSCP value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - egress mapping table index (see API cpssDxChCosPortEgressQoSMappingTableIndexSet)
*                                      (APPLICABLE RANGES : 0..11)
* @param[in] dscp                     - current DSCP  (APPLICABLE RANGES: 0..63).
* @param[in] newDscp                  - new DSCP assigned to packet(APPLICABLE RANGES: 0..63).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, or exp, or newExp, or mappingTableIndex .
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChCosEgressDscp2DscpMappingEntrySet
(
    IN GT_U8   devNum,
    IN GT_U32  mappingTableIndex,
    IN GT_U32  dscp,
    IN GT_U32  newDscp
);


/**
* @internal cpssDxChCosEgressDscp2DscpMappingEntryGet function
* @endinternal
*
* @brief   Get egress mapped packet DSCP value for current dscp.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
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
GT_STATUS cpssDxChCosEgressDscp2DscpMappingEntryGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  mappingTableIndex,
    IN  GT_U32  dscp,
    OUT GT_U32  *newDscpPtr
);


/**
* @internal cpssDxChCosEgressExp2ExpMappingEntrySet function
* @endinternal
*
* @brief   Egress mapping of current packet Exp to a new Exp value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - egress mapping table index (see API cpssDxChCosPortEgressQoSMappingTableIndexSet)
*                                      (APPLICABLE RANGES : 0..11)
* @param[in] exp                      - current  (APPLICABLE RANGES: 0..7).
* @param[in] newExp                   - new Exp assigned to packet (APPLICABLE RANGES: 0..7).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, or exp, or newExp, or mappingTableIndex .
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChCosEgressExp2ExpMappingEntrySet
(
    IN GT_U8   devNum,
    IN GT_U32  mappingTableIndex,
    IN GT_U32  exp,
    IN GT_U32  newExp
);


/**
* @internal cpssDxChCosEgressExp2ExpMappingEntryGet function
* @endinternal
*
* @brief   Get egress mapped packet exp value for current exp.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - egress mapping table index (see API cpssDxChCosPortEgressQoSMappingTableIndexSet)
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
GT_STATUS cpssDxChCosEgressExp2ExpMappingEntryGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  mappingTableIndex,
    IN  GT_U32  exp,
    OUT GT_U32  *newExpPtr
);


/**
* @internal cpssDxChCosEgressUp2UpMappingEntrySet function
* @endinternal
*
* @brief   Egress mapping of current packet up to a new up value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - egress mapping table index (see API cpssDxChCosPortEgressQoSMappingTableIndexSet)
*                                      (APPLICABLE RANGES : 0..11)
* @param[in] up                       - current packet    (APPLICABLE RANGES: 0..7).
* @param[in] newUp                    - new Up assigned to packet (APPLICABLE RANGES: 0..7).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, or up, or newUp, or mappingTableIndex .
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChCosEgressUp2UpMappingEntrySet
(
    IN GT_U8   devNum,
    IN GT_U32  mappingTableIndex,
    IN GT_U32  up,
    IN GT_U32  newUp
);


/**
* @internal cpssDxChCosEgressUp2UpMappingEntryGet function
* @endinternal
*
* @brief   Get egress mapped packet Up value for current Up.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - egress mapping table index (see API cpssDxChCosPortEgressQoSMappingTableIndexSet)
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
GT_STATUS cpssDxChCosEgressUp2UpMappingEntryGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  mappingTableIndex,
    IN  GT_U32  up,
    OUT GT_U32  *newUpPtr
);


/**
* @internal cpssDxChCosEgressTcDp2UpExpDscpMappingEntrySet function
* @endinternal
*
* @brief   Set Egress (TC, DP) mapping to (UP,EXP,DSCP).
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - egress mapping table index (see API cpssDxChCosPortEgressQoSMappingTableIndexSet)
*                                      (APPLICABLE RANGES : 0..11).
* @param[in] tc                       - traffic class assigned to packet  (APPLICABLE RANGES: 0..7).
* @param[in] dp                       - drop precedence.
* @param[in] up                       -  value (APPLICABLE RANGES: 0..7).
* @param[in] exp                      -  value (APPLICABLE RANGES: 0..7).
* @param[in] dscp                     -  value (APPLICABLE RANGES: 0..63).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, or tc,dp, or up,exp,dscp or mappingTableIndex .
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChCosEgressTcDp2UpExpDscpMappingEntrySet
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
* @internal cpssDxChCosEgressTcDp2UpExpDscpMappingEntryGet function
* @endinternal
*
* @brief   Get Egress up,exp,dscp values mapped for current (tc, dp).
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] mappingTableIndex        - egress mapping table index (see API cpssDxChCosPortEgressQoSMappingTableIndexSet)
*                                      (APPLICABLE RANGES : 0..11).
* @param[in] tc                       - traffic class assigned to packet  (APPLICABLE RANGES: 0..7).
* @param[in] dp                       - drop precedence.
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
GT_STATUS cpssDxChCosEgressTcDp2UpExpDscpMappingEntryGet
(
    IN  GT_U8             devNum,
    IN  GT_U32            mappingTableIndex,
    IN  GT_U32            tc,
    IN  CPSS_DP_LEVEL_ENT dp,
    OUT GT_U32            *upPtr,
    OUT GT_U32            *expPtr,
    OUT GT_U32            *dscpPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChCosh */

