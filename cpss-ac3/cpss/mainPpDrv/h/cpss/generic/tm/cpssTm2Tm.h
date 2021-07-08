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
* @file cpssTm2Tm.h
*
* @brief TM to TM configuration.
*
* @version   1
********************************************************************************
*/

#ifndef __cpssTm2Tmh
#define __cpssTm2Tmh

#include <cpss/generic/tm/cpssTmPublicDefs.h>


/**
* @internal cpssTm2TmGlobalConfig function
* @endinternal
*
* @brief   TM2TM Global Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] cosSelector              - C level CoS DP selector (APPLICABLE RANGES: 0..7).
* @param[in] extHeadersPtr            - External Headers structure pointer.
* @param[in] ctrlPacketStrPtr         - Control Packet structure pointer.
* @param[in] rangePtr                 - Delta Range Mapping to Priority structure pointer.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTm2TmGlobalConfig
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          cosSelector,
    IN  CPSS_TM2TM_EXTERNAL_HDR_STC     *extHeadersPtr,
    IN  CPSS_TM2TM_CNTRL_PKT_STRUCT_STC *ctrlPacketStrPtr,
    IN  CPSS_TM2TM_DELTA_RANGE_STC      *rangePtr
);


/**
* @internal cpssTm2TmChannelPortConfig function
* @endinternal
*
* @brief   TM2TM Port Channel Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] egressElements           - Number of Elements in Egress TMs.
* @param[in] srcLevel                 - TM2TM Source Level (Port/C/B/A).
* @param[in] bpLevel                  - BP Destination Level (C/Q).
* @param[in] bpOffset                 - BP Base Offset.
* @param[in] bpXon                    - BP Xon Thresholds.
* @param[in] bpXoff                   - BP Xoff Thresholds.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTm2TmChannelPortConfig
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          egressElements,
    IN  CPSS_TM_LEVEL_ENT               srcLevel,
    IN  CPSS_TM_LEVEL_ENT               bpLevel,
    IN  GT_U32                          bpOffset,
    IN  GT_U32                          bpXon,
    IN  GT_U32                          bpXoff
);


/**
* @internal cpssTm2TmChannelNodeConfig function
* @endinternal
*
* @brief   TM2TM Node Channel Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] egressElements           - Number of Elements in Egress TMs.
* @param[in] srcLevel                 - TM2TM Source Level (Port/C/B/A).
* @param[in] bpLevel                  - BP Destination Level (C/Q).
* @param[in] bpOffset                 - BP Base Offset.
* @param[in] bpXon                    - BP Xon Thresholds.
* @param[in] bpXoff                   - BP Xoff Thresholds.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTm2TmChannelNodeConfig
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          egressElements,
    IN  CPSS_TM_LEVEL_ENT               srcLevel,
    IN  CPSS_TM_LEVEL_ENT               bpLevel,
    IN  GT_U32                          bpOffset,
    IN  GT_U32                          bpXon,
    IN  GT_U32                          bpXoff
);


/**
* @internal cpssTm2TmDpQueryResponceLevelSet function
* @endinternal
*
* @brief   Drop Query Response Select.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] dpType                   - 0 - local only, 1 - remote only, 2 - both local & remote.
* @param[in] portDp                   - 0 - Global, 1 - CoS.
* @param[in] localLevel               - Local response level (Q/A/B/C/Port).
* @param[in] remoteLevel              - Remote response level (Node/Port).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTm2TmDpQueryResponceLevelSet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          dpType,
    IN  GT_U32                          portDp,
    IN  CPSS_TM_LEVEL_ENT               localLevel,
    IN  CPSS_TM2TM_CHANNEL_ENT          remoteLevel
);


/**
* @internal cpssTm2TmLcConfig function
* @endinternal
*
* @brief   TM2TM LC Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] quantum                  - Generation Data Quantum.
* @param[in] qmapHeader               - Qmap Header.
* @param[in] ctrlHeader               - Control Header (Line Card ID).
* @param[in] tmPort                   - TM Port for Control Packets.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTm2TmLcConfig
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          quantum,
    IN  GT_U64                          qmapHeader,
    IN  GT_U32                          ctrlHeader,
    IN  GT_U32                          tmPort
);


/**
* @internal cpssTm2TmEgressDropAqmModeSet function
* @endinternal
*
* @brief   TM2TM Egress Drop AQM Mode Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] level                    - Level (Q/A/B/C/Port).
* @param[in] colorNum                 - Number of colors for this level.
* @param[in] dpSourcePtr              - Array of DP sources for each color.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTm2TmEgressDropAqmModeSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_TM_LEVEL_ENT               level,
    IN  CPSS_TM_COLOR_NUM_ENT           colorNum,
    IN  CPSS_TM_DP_SOURCE_ENT           *dpSourcePtr
);


/**
* @internal cpssTm2TmIngressDropAqmModeSet function
* @endinternal
*
* @brief   TM2TM Ingress Drop AQM Mode Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] channel                  - Channel (Node/Port).
* @param[in] colorNum                 - Number of colors for this channel.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or color number.
* @retval GT_NOT_INITIALIZED       - on not configured channel at system setup.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTm2TmIngressDropAqmModeSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_TM2TM_CHANNEL_ENT          channel,
    IN  CPSS_TM_COLOR_NUM_ENT           colorNum
);


/**
* @internal cpssTm2TmEnable function
* @endinternal
*
* @brief   TM2TM Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTm2TmEnable
(
    IN  GT_U8                           devNum
);

#endif 	    /* __cpssTm2Tmh */


