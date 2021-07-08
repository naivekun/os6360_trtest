/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* tgfCutThrough.h
*
* DESCRIPTION:
*       Generic APIs Cut-Through.
*
* FILE REVISION NUMBER:
*       $Revision: 2 $
*
*******************************************************************************/
#ifndef __tgfCutThroughGenh
#define __tgfCutThroughGenh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum
{
    PRV_TGF_CUT_THROUGH_BYPASS_INGRESS_PCL_NO_BYPASS_E,
    PRV_TGF_CUT_THROUGH_BYPASS_INGRESS_PCL_ONE_LOOKUP_E,
    PRV_TGF_CUT_THROUGH_BYPASS_INGRESS_PCL_TWO_LOOKUP_E,
    PRV_TGF_CUT_THROUGH_BYPASS_INGRESS_PCL_FULL_BYPASS_E
} PRV_TGF_CUT_THROUGH_BYPASS_INGRESS_PCL_ENT;

typedef enum{
    PRV_TGF_EARLY_PROCESSING_CUT_THROUGH_E,
    PRV_TGF_EARLY_PROCESSING_STORE_AND_FORWARD_REDUCED_LATENCY_E
} PRV_TGF_EARLY_PROCESSING_MODE_ENT;

typedef struct
{
    GT_BOOL bypassRouter;
    GT_BOOL bypassIngressPolicerStage0;
    GT_BOOL bypassIngressPolicerStage1;
    GT_BOOL bypassEgressPolicer;
    GT_BOOL bypassEgressPcl;
    PRV_TGF_CUT_THROUGH_BYPASS_INGRESS_PCL_ENT bypassIngressPcl;
    GT_BOOL bypassIngressOam;
    GT_BOOL bypassEgressOam;
    GT_BOOL bypassMll;
} PRV_TGF_CUT_THROUGH_BYPASS_STC;

typedef struct{
    GT_U32 minByteCountBoundary;
    GT_U32 maxByteCountBoundary;
    GT_BOOL enableByteCountBoundariesCheck;
    GT_BOOL enableIpv4HdrCheckByChecksum;
    CPSS_PACKET_CMD_ENT hdrIntergrityExceptionPktCmd;
    CPSS_NET_RX_CPU_CODE_ENT hdrIntergrityExceptionCpuCode;
} PRV_TGF_CUT_THROUGH_PACKET_HDR_INTEGRITY_CHECK_CFG_STC;

/**
* @internal prvTgfCutThroughBypassRouterAndPolicerEnableSet function
* @endinternal
*
* @brief   Enable / Disable bypassing the Router and Ingress Policer engines.
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCutThroughBypassRouterAndPolicerEnableSet
(
    IN GT_BOOL enable
);

/**
* @internal prvTgfCutThroughGlobalEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable of Cut Through feature on all related devices.
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_TRUE                  - enable, GT_FALSE - disable
*/
GT_BOOL prvTgfCutThroughGlobalEnableGet
(
    GT_VOID
);

/**
* @internal prvTgfCutThroughPortEnableSet function
* @endinternal
*
* @brief   Enable/Disable Cut Through forwarding for packets received on the port.
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Cut Through forwarding for tagged packets is enabled
*       per source port and UP. See cpssDxChCutThroughUpEnableSet.
*
*/
GT_STATUS prvTgfCutThroughPortEnableSet
(
    IN GT_U32    portNum,
    IN GT_BOOL  enable,
    IN GT_BOOL  untaggedEnable
);

/**
* @internal prvTgfCutThroughUpEnableSet function
* @endinternal
*
* @brief   Enable / Disable tagged packets with the specified UP
*         to be Cut Through.
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or up.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The ingress port should be enabled for Cut Through forwarding.
*       Use cpssDxChCutThroughPortEnableSet for it.
*
*/
GT_STATUS prvTgfCutThroughUpEnableSet
(
    IN GT_U8    up,
    IN GT_BOOL  enable
);

/**
* @internal prvTgfCutThroughVlanEthertypeSet function
* @endinternal
*
* @brief   Set VLAN Ethertype in order to identify tagged packets.
*         A packed is identified as VLAN tagged for cut-through purposes.
*         Packet considered as tagged if packet's Ethertype equals to one of two
*         configurable VLAN Ethertypes.
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_OUT_OF_RANGE          - on wrong etherType0 or etherType1
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCutThroughVlanEthertypeSet
(
    IN GT_U32   etherType0,
    IN GT_U32   etherType1
);

/**
* @internal prvTgfCutThroughBypassModeSet function
* @endinternal
*
* @brief   Set Bypass mode.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] bypassModePtr            - pointer to Bypass mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong Bypass Ingress PCL mode
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_IMPLEMENTED       - function not implemented for the family
* @retval GT_BAD_STATE             - no such functionality for the device
*/
GT_STATUS prvTgfCutThroughBypassModeSet
(
    IN GT_U8                             devNum,
    IN PRV_TGF_CUT_THROUGH_BYPASS_STC    *bypassModePtr
);

/**
* @internal prvTgfCutThroughBypassModeGet function
* @endinternal
*
* @brief   Get Bypass mode.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
*
* @param[out] bypassModePtr            - pointer to Bypass mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_IMPLEMENTED       - function not implemented for the family
* @retval GT_BAD_STATE             - no such functionality for the device
*/
GT_STATUS prvTgfCutThroughBypassModeGet
(
    IN  GT_U8                             devNum,
    OUT PRV_TGF_CUT_THROUGH_BYPASS_STC    *bypassModePtr
);

/**
* @internal prvTgfCutThroughEarlyProcessingModeSet function
* @endinternal
*
* @brief   Set Early Processing mode of Cut Through packets.
*          Set Reduced Latency Store and Forward mode or Pure Cut Through mode.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in] mode                  - early processing mode of cut through packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvTgfCutThroughEarlyProcessingModeSet
(
    IN PRV_TGF_EARLY_PROCESSING_MODE_ENT        mode
);

/**
* @internal prvTgfDxChCutThroughUdeCfgSet function
* @endinternal
*
* @brief   Set Cut Through configuration of User Defined Ethernet packets.
*          Related to 4 Generic ethernet types configured by
*          cpssDxChPortPipGlobalEtherTypeProtocolSet.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in] udeIndex                    - UDE index (APPLICABLE RANGES: 0..3)
* @param[in] udeEthertype                - Ethertype of UDE packets.
* @param[in] udeCutThroughEnable         - GT_TRUE/GT_FALSE enable/disable Cut Through mode for UDE packets.
* @param[in] udeByteCount                - default CT Byte Count for UDE packets.
*                                          (APPLICABLE RANGES: 0..0x3FFF)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_OUT_OF_RANGE          - on out of range udeByteCount
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvTgfDxChCutThroughUdeCfgSet
(
    IN GT_U32      udeIndex,
    IN GT_U16      udeEthertype,
    IN GT_BOOL     udeCutThroughEnable,
    IN GT_U32      udeByteCount
);

/**
* @internal prvTgfDxChCutThroughByteCountExtractFailsCounterGet function
* @endinternal
*
* @brief   Get counter of fails extracting CT Packet Byte Count by packet header.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[out] countPtr  - pointer to count of packet Byte Count extracting fails.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfDxChCutThroughByteCountExtractFailsCounterGet
(
    OUT GT_U32      *countPtr
);

/**
* @internal prvTgfCutThroughPacketHdrIntegrityCheckCfgSet function
* @endinternal
*
* @brief   Set configuration Packet Header Integrity Check.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in] cfgPtr  - pointer to Packet Header Integrity Check.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvTgfCutThroughPacketHdrIntegrityCheckCfgSet
(
    IN PRV_TGF_CUT_THROUGH_PACKET_HDR_INTEGRITY_CHECK_CFG_STC   *cfgPtr
);

/**
* @internal prvTgfCutThroughPortByteCountUpdateSet function
* @endinternal
*
* @brief   Set configuration for updating byte count per port.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in] portNum                   - physical port number including CPU port.
* @param[in] toSubtractOrToAdd         - GT_TRUE - to subtract, GT_FALSE to add.
* @param[in] subtractedOrAddedValue    - value to subtract or to add to packet length
*                                        (APPLICABLE RANGES: 0..7)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or portNum
* @retval GT_OUT_OF_RANGE          - on out of range subtractedOrAddedValue
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvTgfCutThroughPortByteCountUpdateSet
(
    IN GT_PHYSICAL_PORT_NUM  portNum,
    IN GT_BOOL               toSubtractOrToAdd,
    IN GT_U32                subtractedOrAddedValue
);

/**
* @internal prvTgfDxChCutThroughRxErrorConfigSet function
* @endinternal
*
* @brief   Set Error Configuration of RX when obtained actual packet size different from calculated.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Lion; Lion2; Bobcat3; Aldrin2.
*
* @param[in]  paddedPacketEofWithError    - When a CT packet is padded it‘s last
*                                           write transaction to the PB should be marked
*                                           GT_FALSE - as EOP or GT_TRUE - as EOP with Error,
* @param[in]  truncatedPacketEofWithError - When a CT packet is truncated it‘s last
*                                           write transaction to the PB should be marked
*                                           GT_FALSE - as EOP or GT_TRUE - as EOP with Error,
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or portNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvTgfDxChCutThroughRxErrorConfigSet
(
    IN  GT_BOOL               paddedPacketEofWithError,
    IN  GT_BOOL               truncatedPacketEofWithError
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __tgfCutThroughGenh */


