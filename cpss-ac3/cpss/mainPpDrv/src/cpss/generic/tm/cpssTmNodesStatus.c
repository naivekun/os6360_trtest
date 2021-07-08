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
* @file cpssTmNodesStatus.c
*
* @brief TM nodes status reading APIs
*
* @version   2
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/generic/tm/private/prvCpssGenTmLog.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/extServices/os/gtOs/gtGenTypes.h>
#include <cpss/generic/tm/cpssTmServices.h>
#include <cpss/generic/tm/cpssTmNodesStatus.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <tm_nodes_status.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>


/**
* @internal internal_cpssTmQueueStatusRead function
* @endinternal
*
* @brief   Read Queue status.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - Node index.
*
* @param[out] statusPtr                - (pointer to) Node status structure.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssTmQueueStatusRead
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      index,
    OUT CPSS_TM_NODE_STATUS_STC     *statusPtr
)
{
    GT_STATUS   rc = GT_OK;
    int         ret = 0;
    struct tm_node_status stat;

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                           | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_PUMA_E | CPSS_PUMA3_E  );
    CPSS_NULL_PTR_CHECK_MAC(statusPtr);

    ret = tm_read_queue_status(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle, index, &stat);
    rc = XEL_TO_CPSS_ERR_CODE(ret);

    cpssOsMemCpy(statusPtr, &stat, sizeof(struct tm_node_status));

    return rc;
}

/**
* @internal cpssTmQueueStatusRead function
* @endinternal
*
* @brief   Read Queue status.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - Node index.
*
* @param[out] statusPtr                - (pointer to) Node status structure.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmQueueStatusRead
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      index,
    OUT CPSS_TM_NODE_STATUS_STC     *statusPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmQueueStatusRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, statusPtr));

    rc = internal_cpssTmQueueStatusRead(devNum, index, statusPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, statusPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssTmAnodeStatusRead function
* @endinternal
*
* @brief   Read A-Node status.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - Node index.
*
* @param[out] statusPtr                - (pointer to) Node status structure.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssTmAnodeStatusRead
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      index,
    OUT CPSS_TM_NODE_STATUS_STC     *statusPtr
)
{
    GT_STATUS   rc = GT_OK;
    int         ret = 0;
    struct tm_node_status stat;

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                           | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_PUMA_E | CPSS_PUMA3_E  );

    CPSS_NULL_PTR_CHECK_MAC(statusPtr);

    ret = tm_read_a_node_status(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle, index, &stat);
    rc = XEL_TO_CPSS_ERR_CODE(ret);

    cpssOsMemCpy(statusPtr, &stat, sizeof(struct tm_node_status));

    return rc;
}

/**
* @internal cpssTmAnodeStatusRead function
* @endinternal
*
* @brief   Read A-Node status.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - Node index.
*
* @param[out] statusPtr                - (pointer to) Node status structure.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmAnodeStatusRead
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      index,
    OUT CPSS_TM_NODE_STATUS_STC     *statusPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmAnodeStatusRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, statusPtr));

    rc = internal_cpssTmAnodeStatusRead(devNum, index, statusPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, statusPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssTmBnodeStatusRead function
* @endinternal
*
* @brief   Read B-Node status.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - Node index.
*
* @param[out] statusPtr                - (pointer to) Node status structure.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssTmBnodeStatusRead
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      index,
    OUT CPSS_TM_NODE_STATUS_STC     *statusPtr
)
{
    GT_STATUS   rc = GT_OK;
    int         ret = 0;
    struct tm_node_status stat;

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                           | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_PUMA_E | CPSS_PUMA3_E  );
    CPSS_NULL_PTR_CHECK_MAC(statusPtr);

    ret = tm_read_b_node_status(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle, index, &stat);
    rc = XEL_TO_CPSS_ERR_CODE(ret);

    cpssOsMemCpy(statusPtr, &stat, sizeof(struct tm_node_status));

    return rc;
}

/**
* @internal cpssTmBnodeStatusRead function
* @endinternal
*
* @brief   Read B-Node status.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - Node index.
*
* @param[out] statusPtr                - (pointer to) Node status structure.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmBnodeStatusRead
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      index,
    OUT CPSS_TM_NODE_STATUS_STC     *statusPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmBnodeStatusRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, statusPtr));

    rc = internal_cpssTmBnodeStatusRead(devNum, index, statusPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, statusPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssTmCnodeStatusRead function
* @endinternal
*
* @brief   Read C-Node status.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - Node index.
*
* @param[out] statusPtr                - (pointer to) Node status structure.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssTmCnodeStatusRead
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      index,
    OUT CPSS_TM_NODE_STATUS_STC     *statusPtr
)
{
    GT_STATUS   rc = GT_OK;
    int         ret = 0;
    struct tm_node_status stat;

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                           | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_PUMA_E | CPSS_PUMA3_E  );

    CPSS_NULL_PTR_CHECK_MAC(statusPtr);

    ret = tm_read_c_node_status(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle, index, &stat);
    rc = XEL_TO_CPSS_ERR_CODE(ret);

    cpssOsMemCpy(statusPtr, &stat, sizeof(struct tm_node_status));

    return rc;
}

/**
* @internal cpssTmCnodeStatusRead function
* @endinternal
*
* @brief   Read C-Node status.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - Node index.
*
* @param[out] statusPtr                - (pointer to) Node status structure.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmCnodeStatusRead
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      index,
    OUT CPSS_TM_NODE_STATUS_STC     *statusPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmCnodeStatusRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, statusPtr));

    rc = internal_cpssTmCnodeStatusRead(devNum, index, statusPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, statusPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssTmPortStatusRead function
* @endinternal
*
* @brief   Read Port status.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - Node index.
*
* @param[out] statusPtr                - (pointer to) Node status structure.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssTmPortStatusRead
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      index,
    OUT CPSS_TM_PORT_STATUS_STC     *statusPtr
)
{
    GT_STATUS   rc = GT_OK;
    int         ret = 0;
    struct tm_port_status stat;
    int i=0;

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                           | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_PUMA_E | CPSS_PUMA3_E  );

    CPSS_NULL_PTR_CHECK_MAC(statusPtr);

    ret = tm_read_port_status(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle, (uint8_t)index, &stat);
    rc = XEL_TO_CPSS_ERR_CODE(ret);

    statusPtr->maxBucketLevel = stat.max_bucket_level;
    statusPtr->minBucketLevel = stat.min_bucket_level;
    for(i=0; i<8; i++)
    {
        statusPtr->deficitArr[i] = stat.deficit[i];
    }

    return rc;
}

/**
* @internal cpssTmPortStatusRead function
* @endinternal
*
* @brief   Read Port status.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - Node index.
*
* @param[out] statusPtr                - (pointer to) Node status structure.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmPortStatusRead
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      index,
    OUT CPSS_TM_PORT_STATUS_STC     *statusPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmPortStatusRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, statusPtr));

    rc = internal_cpssTmPortStatusRead(devNum, index, statusPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, statusPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssTmDropQueueLengthGet function
* @endinternal
*
* @brief   Read Queue drop length.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] level                    - Node  (Port/C/B/A/Queue).
* @param[in] index                    - Node index.
*
* @param[out] instantQueueLengthPtr    - (pointer to) Current Queue length structure.
* @param[out] averageQueueLengthPtr    - (pointer to) Average Queue length structure.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssTmDropQueueLengthGet
(
    IN  GT_U8                       devNum,
    IN  CPSS_TM_LEVEL_ENT           level,
    IN  GT_U32                      index,
    OUT GT_U32                      *instantQueueLengthPtr,
    OUT GT_U32                      *averageQueueLengthPtr
)
{
    GT_STATUS   rc = GT_OK;
    int         ret = 0;

    uint32_t inst_ql;
    uint32_t av_ql;

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                           | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_PUMA_E | CPSS_PUMA3_E  );

    CPSS_NULL_PTR_CHECK_MAC(instantQueueLengthPtr);
    CPSS_NULL_PTR_CHECK_MAC(averageQueueLengthPtr);

    ret = tm_drop_get_queue_length(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle,
                                   level,
                                   index,
                                   &inst_ql,
                                   &av_ql);
    rc = XEL_TO_CPSS_ERR_CODE(ret);
    if(rc)
        return rc;

    *instantQueueLengthPtr = inst_ql;
    *averageQueueLengthPtr = av_ql;
    return rc;
}

/**
* @internal cpssTmDropQueueLengthGet function
* @endinternal
*
* @brief   Read Queue drop length.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] level                    - Node  (Port/C/B/A/Queue).
* @param[in] index                    - Node index.
*
* @param[out] instantQueueLengthPtr    - (pointer to) Current Queue length structure.
* @param[out] averageQueueLengthPtr    - (pointer to) Average Queue length structure.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmDropQueueLengthGet
(
    IN  GT_U8                       devNum,
    IN  CPSS_TM_LEVEL_ENT           level,
    IN  GT_U32                      index,
    OUT GT_U32                      *instantQueueLengthPtr,
    OUT GT_U32                      *averageQueueLengthPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmDropQueueLengthGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, level, index, instantQueueLengthPtr, averageQueueLengthPtr));

    rc = internal_cpssTmDropQueueLengthGet(devNum, level, index, instantQueueLengthPtr, averageQueueLengthPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, level, index, instantQueueLengthPtr, averageQueueLengthPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

