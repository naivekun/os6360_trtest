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
* @file prvCpssDxChLpmHw.h
*
* @brief HW memory read/write internal functions
*
* @version   4
********************************************************************************
*/
#ifndef __prvCpssDxChLpmHwh
#define __prvCpssDxChLpmHwh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/dxCh/dxChxGen/lpm/cpssDxChLpmTypes.h>
#include <cpss/dxCh/dxChxGen/private/lpm/prvCpssDxChLpmTypes.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamTypes.h>

#define PRV_CPSS_DXCH_SIP6_CONVERT_LEAF_SW_INDEX_TO_LEAF_HW_INDEX_MAC(leafSwIndex) \
    (((leafSwIndex / 5) << 3) + leafSwIndex % 5)

#define PRV_CPSS_DXCH_SIP6_CONVERT_LEAF_HW_INDEX_TO_LEAF_SW_INDEX_MAC(leafHwIndex) \
    ((leafHwIndex >> 3) * 5 + (leafHwIndex & 0x7))

/**
* @internal prvCpssDxChLpmHwVrfEntryWrite function
* @endinternal
*
* @brief   Write a VRF table entry
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] shadowType               - the shadow type
* @param[in] vrId                     - the virtual router id
* @param[in] protocol                 - the protocol
* @param[in] nodeType                 - the type of the node
* @param[in] rootNodeAddr             - the line number of the root node
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_OUT_OF_RANGE          - rootNodeAddr is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*
* @note VRF root nodeType can be only CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E or
*       CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E.
*
*/
GT_STATUS prvCpssDxChLpmHwVrfEntryWrite
(
    IN  GT_U8                                   devNum,
    IN  PRV_CPSS_DXCH_LPM_SHADOW_TYPE_ENT       shadowType,
    IN  GT_U32                                  vrId,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT    protocol,
    IN  CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT     nodeType,
    IN  GT_U32                                  rootNodeAddr
);

/**
* @internal prvCpssDxChLpmHwVrfEntryRead function
* @endinternal
*
* @brief   Read a VRF table entry
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] vrId                     - the virtual router id
* @param[in] protocol                 - the protocol
*
* @param[out] nodeTypePtr              - the type of the node
* @param[out] rootNodeAddrPtr          - the line number of the root node
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*
* @note VRF root nodeTypePtr can be only CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E
*       or CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E.
*
*/
GT_STATUS prvCpssDxChLpmHwVrfEntryRead
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  vrId,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT    protocol,
    OUT CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT     *nodeTypePtr,
    OUT GT_U32                                  *rootNodeAddrPtr
);

/**
* @internal prvCpssDxChLpmHwVrfEntryReadSip6 function
* @endinternal
 *
* @brief   Read a VRF table entry
 *
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2;
*         DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum;
*         Aldrin; AC3X; Bobcat3.
 *
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*
* @note VRF root nodeTypePtr can be only CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E
*       CPSS_DXCH_LPM_LEAFE_NODE_PTR_TYPE_E,
*       or CPSS_DXCH_LPM_ONE_COMPRESSED_NODE_PTR_TYPE_E.
*
* @param[in] devNum                   - the device number
* @param[in] vrId                     - the virtual router id
* @param[in] protocol                 - the
* @param[out] nodeTypePtr              - the type of the node
* @param[out] rootNodeAddrPtr          - the line number of the root node
*/
GT_STATUS prvCpssDxChLpmHwVrfEntryReadSip6
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  vrId,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT    protocol,
    OUT PRV_CPSS_DXCH_LPM_CHILD_TYPE_ENT        *nodeTypePtr,
    OUT GT_U32                                  *rootNodeAddrPtr
);

/**
* @internal prvCpssDxChLpmHwEcmpEntryWrite function
* @endinternal
*
* @brief   Write an ECMP entry
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; xCat2; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] ecmpEntryIndex           - the index of the entry in the ECMP table
* @param[in] ecmpEntryPtr             - (pointer to) the ECMP entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
*/
GT_STATUS prvCpssDxChLpmHwEcmpEntryWrite
(
    IN GT_U8                                devNum,
    IN GT_U32                               ecmpEntryIndex,
    IN PRV_CPSS_DXCH_LPM_ECMP_ENTRY_STC     *ecmpEntryPtr
);

/**
* @internal prvCpssDxChLpmHwEcmpEntryRead function
* @endinternal
*
* @brief   Read an ECMP entry
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; xCat2; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] ecmpEntryIndex           - the index of the entry in the ECMP table
*
* @param[out] ecmpEntryPtr             - (pointer to) the ECMP entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
*/
GT_STATUS prvCpssDxChLpmHwEcmpEntryRead
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              ecmpEntryIndex,
    OUT PRV_CPSS_DXCH_LPM_ECMP_ENTRY_STC    *ecmpEntryPtr
);
/**
* @internal prvCpssDxChLpmLeafEntryCalcLpmHwIndexFromSwIndex function
* @endinternal
*
* @brief   Calculate HW Index for an LPM leaf entry
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] leafIndex                - leaf index within the LPM PBR block
*
* @param[out] lpmIndexPtr              - (pointer to) leaf index within the LPM PBR block in HW representation
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvCpssDxChLpmLeafEntryCalcLpmHwIndexFromSwIndex
(
    IN GT_U8            devNum,
    IN GT_U32           leafIndex,
    OUT GT_U32          *lpmIndexPtr
);

/**
* @internal prvCpssDxChLpmLeafEntryCalcLpmSwIndexFromHwIndex function
* @endinternal
*
* @brief   Calculate SW Index for an LPM leaf HW index
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] leafHwIndex              - leaf HW index within the LPM PBR block in HW representation
*
* @param[out] leafSwIndexPtr           - (pointer to)leaf index within the LPM PBR block SW representation
*
* @retval GT_OK                    - on success
*/
GT_VOID prvCpssDxChLpmLeafEntryCalcLpmSwIndexFromHwIndex
(
    IN GT_U8            devNum,
    IN GT_U32           leafHwIndex,
    OUT GT_U32          *leafSwIndexPtr
);

/**
* @internal prvCpssLpmFalconBuildLeafValue function
* @endinternal
*
* @brief   Build leaf data for single leaf
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
*
* @param[in] leafNodePtr            - leaf data
*
* @param[out] valuePtr              - pointer to leaf data in hw format
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on any parameter is wrong
*/
GT_STATUS prvCpssLpmFalconBuildLeafValue
(
    IN  PRV_CPSS_DXCH_LPM_LEAF_ENTRY_STC *leafNodePtr,
    OUT GT_U32                           *valuePtr
);

/**
* @internal prvCpssLpmFalconFillPrvLeafValue function
* @endinternal
*
* @brief   translate CPSS_DXCH_LPM_LEAF_ENTRY_STC leaf into PRV_CPSS_DXCH_LPM_LEAF_ENTRY_STC leaf entry
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
*
* @param[in] leafNodePtr            - leaf data
*
* @param[out] valuePtr              - pointer to leaf data in hw format.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on any parameter is wrong
*/
GT_VOID prvCpssLpmFalconFillPrvLeafValue
(
    IN  CPSS_DXCH_LPM_LEAF_ENTRY_STC      *leafEntryPtr,
    OUT PRV_CPSS_DXCH_LPM_LEAF_ENTRY_STC  *prvLeafEntryPtr
);


/**
* @internal prvCpssDxChLpmNextPointerDataBuild function
* @endinternal
*
* @brief   Local function for LPM bucket next pointer HW format build.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2.
*
* @param[in] dev                      -  number
* @param[in] nextPointerData          - the next pointer data.
*
* @param[out] hwDataPtr                - node data in HW format
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if any parameter is wrong
*/
GT_STATUS prvCpssDxChLpmNextPointerDataBuild
(
    IN  GT_U8                                       dev,
    IN  CPSS_DXCH_LPM_NODE_NEXT_POINTER_STC         nextPointerData,
    OUT GT_U32                                      *hwDataPtr
);

/**
* @internal prvCpssDxChLpmNextPointerDataDecode function
* @endinternal
*
* @brief   Local function for LPM bucket next pointer HW format build
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2.
*
* @param[in] hwDataPtr                - node data in HW format
*
* @param[out] nextPointerDataPtr       - (pointer to) next pointer data.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on any parameter is wrong
*/
GT_STATUS prvCpssDxChLpmNextPointerDataDecode
(
    IN  GT_U32                                      *hwDataPtr,
    OUT CPSS_DXCH_LPM_NODE_NEXT_POINTER_STC         *nextPointerDataPtr
);

/**
* @internal prvCpssDxChLpmFalconCompressedNodeDataBuild function
* @endinternal
*
* @brief   Build hw format for LPM compressed nodes
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
*
* @param[in] devNum                     - the device number
* @param[in] compressedNodePtr          - node data
* @param[in] groupOfNodesArrayPtrPtr    - pointer to adress of current data in hw format
*
* @param[out] groupOfNodesArrayPtrPtr   - (pointer to pointer) to data in hw format
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on any parameter is wrong
*/
GT_STATUS prvCpssDxChLpmFalconCompressedNodeDataBuild
(
    IN  PRV_CPSS_DXCH_LPM_COMPRESSED_STC            *compressedNodePtr,
    INOUT GT_U32                                    **groupOfNodesArrayPtrPtr
);

/**
* @internal prvCpssDxChLpmFalconLeafNodeDataBuild function
* @endinternal
*
* @brief   Build hw format for LPM leaves nodes
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
*
* @param[in] compressedNodePtr          - node data
* @param[in] leavesCounter              - number of leaves so far
* @param[in] startNewGonLeaves          - leaves from the new GON started to process
* @param[in] groupOfNodesArrayPtrPtr    - pointer to adress of current data in hw format
*
* @param[out] groupOfNodesArrayPtrPtr   - (pointer to pointer) to data in hw format
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on any parameter is wrong
*/
GT_STATUS prvCpssDxChLpmFalconLeafNodeDataBuild
(
    IN  PRV_CPSS_DXCH_LPM_LEAF_ENTRY_STC            *leafNodePtr,
    IN  GT_U32                                      leavesCounter,
    IN  GT_BOOL                                     startNewGonLeaves,
    INOUT GT_U32                                    **groupOfNodesArrayPtrPtr
);

/**
* @internal prvCpssLpmFalconDecodeLeafValue function
* @endinternal
*
* @brief   Decode leaf entry from HW value
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
*
* @param[in] value                  - leaf data in HW format
*
* @param[out] leafNodePtr           - pointer to leaf entry
*
* @retval GT_OK                     - on success
* @retval GT_BAD_PARAM              - on any parameter is wrong
*/
GT_STATUS prvCpssLpmFalconDecodeLeafValue
(
    IN  GT_U32                          value,
    OUT CPSS_DXCH_LPM_LEAF_ENTRY_STC    *leafNodePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChLpmHwh */


