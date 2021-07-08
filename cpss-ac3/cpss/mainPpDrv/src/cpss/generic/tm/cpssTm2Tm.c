/********************* **********************************************************
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
* @file cpssTm2Tm.c
*
* @brief TM2TM Configuration Library APIs
*
* @version   2
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/generic/tm/private/prvCpssGenTmLog.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/extServices/os/gtOs/gtGenTypes.h>
#include <cpss/generic/tm/cpssTmServices.h>
#include <cpss/generic/tm/cpssTm2Tm.h>
#include <tm2tm.h>
#include <tm_defs.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>


/**
* @internal internal_cpssTm2TmGlobalConfig function
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
static GT_STATUS internal_cpssTm2TmGlobalConfig
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          cosSelector,
    IN  CPSS_TM2TM_EXTERNAL_HDR_STC     *extHeadersPtr,
    IN  CPSS_TM2TM_CNTRL_PKT_STRUCT_STC *ctrlPacketStrPtr,
    IN  CPSS_TM2TM_DELTA_RANGE_STC      *rangePtr
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;

    struct tm_ext_hdr str1;
    struct tm_ctrl_pkt_str str2;
    struct tm_delta_range str3;
 	/* copy header data - 32 bytes */

	cpssOsMemCpy(str1.content, extHeadersPtr->contentArr, sizeof(char)*32);

    str1.size = (uint8_t)extHeadersPtr->size;
    str2.nodes = (uint8_t)ctrlPacketStrPtr->nodesNum;
    str2.ports = (uint8_t)ctrlPacketStrPtr->portsNum;
    str3.upper_range0 = (uint8_t)rangePtr->upperRange0;
    str3.upper_range1 = (uint8_t)rangePtr->upperRange1;
    str3.upper_range2 = (uint8_t)rangePtr->upperRange2;
    ret = tm2tm_global_config(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle,
                              (uint8_t)cosSelector,
                              &str1,
                              &str2,
                              &str3);
    rc = XEL_TO_CPSS_ERR_CODE(ret);

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTm2TmGlobalConfig);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cosSelector, extHeadersPtr, ctrlPacketStrPtr, rangePtr));

    rc = internal_cpssTm2TmGlobalConfig(devNum, cosSelector, extHeadersPtr, ctrlPacketStrPtr, rangePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cosSelector, extHeadersPtr, ctrlPacketStrPtr, rangePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssTm2TmChannelPortConfig function
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
static GT_STATUS internal_cpssTm2TmChannelPortConfig
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          egressElements,
    IN  CPSS_TM_LEVEL_ENT               srcLevel,
    IN  CPSS_TM_LEVEL_ENT               bpLevel,
    IN  GT_U32                          bpOffset,
    IN  GT_U32                          bpXon,
    IN  GT_U32                          bpXoff
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;

    ret = tm2tm_port_channel_config(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle,
                                    (uint8_t)egressElements,
                                    srcLevel,
                                    1, /* TM2TM_BP */
                                    bpLevel,
                                    (uint16_t)bpOffset,
                                    (uint8_t)bpXon,
                                    (uint8_t)bpXoff);
    rc = XEL_TO_CPSS_ERR_CODE(ret);

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTm2TmChannelPortConfig);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, egressElements, srcLevel, bpLevel, bpOffset, bpXon, bpXoff));

    rc = internal_cpssTm2TmChannelPortConfig(devNum, egressElements, srcLevel, bpLevel, bpOffset, bpXon, bpXoff);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, egressElements, srcLevel, bpLevel, bpOffset, bpXon, bpXoff));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssTm2TmChannelNodeConfig function
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
static GT_STATUS internal_cpssTm2TmChannelNodeConfig
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          egressElements,
    IN  CPSS_TM_LEVEL_ENT               srcLevel,
    IN  CPSS_TM_LEVEL_ENT               bpLevel,
    IN  GT_U32                          bpOffset,
    IN  GT_U32                          bpXon,
    IN  GT_U32                          bpXoff
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;

    ret = tm2tm_node_channel_config(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle,
                                    (uint16_t)egressElements,
                                    srcLevel,
                                    1, /* TM2TM_BP */
                                    bpLevel,
                                    (uint16_t)bpOffset,
                                    (uint8_t)bpXon,
                                    (uint8_t)bpXoff);
    rc = XEL_TO_CPSS_ERR_CODE(ret);

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTm2TmChannelNodeConfig);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, egressElements, srcLevel, bpLevel, bpOffset, bpXon, bpXoff));

    rc = internal_cpssTm2TmChannelNodeConfig(devNum, egressElements, srcLevel, bpLevel, bpOffset, bpXon, bpXoff);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, egressElements, srcLevel, bpLevel, bpOffset, bpXon, bpXoff));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssTm2TmDpQueryResponceLevelSet function
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
static GT_STATUS internal_cpssTm2TmDpQueryResponceLevelSet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          dpType,
    IN  GT_U32                          portDp,
    IN  CPSS_TM_LEVEL_ENT               localLevel,
    IN  CPSS_TM2TM_CHANNEL_ENT          remoteLevel
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;

    ret = tm_set_dp_query_resp_lvl(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle,
                                   (uint8_t)dpType,
                                   (uint8_t)portDp,
                                   localLevel,
                                   remoteLevel);
    rc = XEL_TO_CPSS_ERR_CODE(ret);

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTm2TmDpQueryResponceLevelSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, dpType, portDp, localLevel, remoteLevel));

    rc = internal_cpssTm2TmDpQueryResponceLevelSet(devNum, dpType, portDp, localLevel, remoteLevel);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, dpType, portDp, localLevel, remoteLevel));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssTm2TmLcConfig function
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
static GT_STATUS internal_cpssTm2TmLcConfig
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          quantum,
    IN  GT_U64                          qmapHeader,
    IN  GT_U32                          ctrlHeader,
    IN  GT_U32                          tmPort
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;

    ret = tm2tm_lc_config(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle,
                          (uint16_t)quantum,
                          &qmapHeader,
                          (uint8_t)ctrlHeader,
                          (uint8_t)tmPort);
    rc = XEL_TO_CPSS_ERR_CODE(ret);

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTm2TmLcConfig);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, quantum, qmapHeader, ctrlHeader, tmPort));

    rc = internal_cpssTm2TmLcConfig(devNum, quantum, qmapHeader, ctrlHeader, tmPort);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, quantum, qmapHeader, ctrlHeader, tmPort));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssTm2TmEgressDropAqmModeSet function
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
static GT_STATUS internal_cpssTm2TmEgressDropAqmModeSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_TM_LEVEL_ENT               level,
    IN  CPSS_TM_COLOR_NUM_ENT           colorNum,
    IN  CPSS_TM_DP_SOURCE_ENT           *dpSourcePtr
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;
    enum tm_dp_source src = *dpSourcePtr;

    ret = tm2tm_set_egress_drop_aqm_mode(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle,
                                         level,
                                         colorNum,
                                         &src);
    rc = XEL_TO_CPSS_ERR_CODE(ret);

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTm2TmEgressDropAqmModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, level, colorNum, dpSourcePtr));

    rc = internal_cpssTm2TmEgressDropAqmModeSet(devNum, level, colorNum, dpSourcePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, level, colorNum, dpSourcePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssTm2TmIngressDropAqmModeSet function
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
static GT_STATUS internal_cpssTm2TmIngressDropAqmModeSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_TM2TM_CHANNEL_ENT          channel,
    IN  CPSS_TM_COLOR_NUM_ENT           colorNum
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;

    ret = tm2tm_set_ingress_drop_aqm_mode(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle, channel, colorNum);
    rc = XEL_TO_CPSS_ERR_CODE(ret);

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTm2TmIngressDropAqmModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, channel, colorNum));

    rc = internal_cpssTm2TmIngressDropAqmModeSet(devNum, channel, colorNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, channel, colorNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssTm2TmEnable function
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
static GT_STATUS internal_cpssTm2TmEnable
(
    IN  GT_U8                           devNum
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;

    ret = tm2tm_enable(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle);
    rc = XEL_TO_CPSS_ERR_CODE(ret);

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTm2TmEnable);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));

    rc = internal_cpssTm2TmEnable(devNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

