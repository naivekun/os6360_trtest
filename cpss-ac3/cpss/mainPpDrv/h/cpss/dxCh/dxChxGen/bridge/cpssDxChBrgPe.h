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
* @file cpssDxChBrgPe.h
*
* @brief Header for Bridge Port Extender (BPE) DxCh cpss implementation.
*
* @version   4
********************************************************************************
*/
#ifndef __cpssDxChBrgPeh
#define __cpssDxChBrgPeh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>

/**
* @internal cpssDxChBrgPeEnableSet function
* @endinternal
*
* @brief   Enables globally Bridge Port Extender (BPE) feature.
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE:  BPE feature enabled.
*                                      GT_FALSE: BPE feature disabled.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChBrgPeEnableSet
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
);

/**
* @internal cpssDxChBrgPeEnableGet function
* @endinternal
*
* @brief   Gets Bridge Port Extender (BPE) global enabling status.
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) GT_TRUE:  BPE feature enabled.
*                                      GT_FALSE: BPE feature disabled.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChBrgPeEnableGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
);

/**
* @internal cpssDxChBrgPePortEnableSet function
* @endinternal
*
* @brief   Enables per port Bridge Port Extender (BPE) feature.
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
* @param[in] enable                   - GT_TRUE:  port enabled for BPE feature.
*                                      GT_FALSE: port disabled for BPE feature.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad device or port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChBrgPePortEnableSet
(
    IN GT_U8                  devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_BOOL                enable
);

/**
* @internal cpssDxChBrgPePortEnableGet function
* @endinternal
*
* @brief   Gets Bridge Port Extender (BPE) enabling status per port.
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
*
* @param[out] enablePtr                - (pointer to) GT_TRUE:  port enabled for BPE feature.
*                                      GT_FALSE: port disabled for BPE feature.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad device or port number
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChBrgPePortEnableGet
(
    IN  GT_U8                  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_BOOL                *enablePtr
);


/**
* @internal cpssDxChBrgPePortPcidMcFilterEnableSet function
* @endinternal
*
* @brief   Configures <port,PCID> pair for downsteam multicast source filtering.
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] pcid                     - Port E-Channel Identifier (APPLICABLE RANGES: 0..4095)
* @param[in] mcFilterEnable           - GT_TRUE:  Multicast source filtering is enabled for the
*                                      <port,PCID> pair.
*                                      - GT_FALSE: Multicast source filtering is disabled for the
*                                      <port,PCID> pair.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad device, port number or PCID value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChBrgPePortPcidMcFilterEnableSet
(
    IN GT_U8                  devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_U32                 pcid,
    IN GT_BOOL                mcFilterEnable
);

/**
* @internal cpssDxChBrgPePortPcidMcFilterEnableGet function
* @endinternal
*
* @brief   Gets downsteam multicast source filtering enabling status for <port,PCID> pair.
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] pcid                     - Port E-Channel Identifier (APPLICABLE RANGES: 0..4095)
*
* @param[out] mcFilterEnablePtr        - (pointer to)
*                                      GT_TRUE:  Multicast source filtering is enabled for the
*                                      <port,PCID> pair.
*                                      GT_FALSE: Multicast source filtering is disabled for the
*                                      <port,PCID> pair.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad device, port number or PCID value
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChBrgPePortPcidMcFilterEnableGet
(
    IN  GT_U8                  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  GT_U32                 pcid,
    OUT GT_BOOL                *mcFilterEnablePtr
);

/**
* @internal cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableSet function
* @endinternal
*
* @brief   Associate the cascade port (not DSA cascade port) with a trunkId.
*         this trunk id should represent the 'uplink trunk'
*         The purpose as described in the Functional specification is for "Trunk-ID for E-Tag<Ingress_E-CID_base>
*         assignment for multi-destination traffic"
*         NOTE: this function MUST not be called on 'Control Bridge'
*         APPLICABLE DEVICES:
*         Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; xCat3.
*         INPUTS:
*         devNum     - device number
*         cascadePortNum - physical port number of the BPE 802.1br cascade port (not DSA cascade port)
*         enable     - GT_TRUE: associate the trunkId to cascadePortNum.
*         - GT_FALSE: disassociate the trunkId from cascadePortNum.
*         trunkId    - the trunkId (of the uplink trunk)
*         note: relevant only when enable == GT_TRUE
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; xCat3.
*
* @param[in] devNum                   - device number
* @param[in] cascadePortNum           - physical port number of the BPE 802.1br cascade port (not DSA cascade port)
* @param[in] enable                   - GT_TRUE:  associate the trunkId to cascadePortNum.
*                                      - GT_FALSE: disassociate the trunkId from cascadePortNum.
* @param[in] trunkId                  - the  (of the uplink trunk)
*                                      note: relevant only when enable == GT_TRUE
*                                      APPLICABLE RANGE: 1..(4K-1).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad devNum, cascadePortNum
* @retval GT_OUT_OF_RANGE          - trunkId out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     cascadePortNum,
    IN GT_BOOL                  enable,
    IN GT_TRUNK_ID              trunkId
);

/**
* @internal cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableGet function
* @endinternal
*
* @brief   Get the associate trunkId with the cascade port (not DSA cascade port) .
*         this trunk id should represent the 'uplink trunk'
*         The purpose as described in the Functional specification is for "Trunk-ID for E-Tag<Ingress_E-CID_base>
*         assignment for multi-destination traffic"
*         NOTE: this function MUST not be called on 'Control Bridge'
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; xCat3.
*
* @param[in] devNum                   - device number
* @param[in] cascadePortNum           - physical port number of the BPE 802.1br cascade port (not DSA cascade port)
*
* @param[out] enablePtr                - (pointer to) indication that trunkId associate to the cascadePortNum.
*                                      - GT_TRUE:  trunkId associate to the cascadePortNum.
*                                      - GT_FALSE: trunkId is not associate to the cascadePortNum.
* @param[out] trunkIdPtr               - the trunkId (of the uplink trunk)
*                                      note: relevant only when (enablePtr) == GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad devNum, cascadePortNum
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - inconsistency in HW regarding value of the 'associated trunkId'
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     cascadePortNum,
    OUT GT_BOOL                  *enablePtr,
    OUT GT_TRUNK_ID              *trunkIdPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ____cpssDxChBrgPeh */


