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
* @file cpssTmEligPrioFunc.c
*
* @brief TM Configuration Library Drop APIs
*
* @version   2
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/generic/tm/private/prvCpssGenTmLog.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/extServices/os/gtOs/gtGenTypes.h>
#include <cpss/generic/tm/cpssTmServices.h>
#include <cpss/generic/tm/cpssTmEligPrioFunc.h>
#include <tm_elig_prio_func.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>


/**
* @internal prvCpssTmEligPrioFuncNodeCpy function
* @endinternal
*
* @brief   Copy CPSS TmEligPrioFunc node structure to TM TmEligPrioFunc structure.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] paramsPtr                - CPSS TmEligPrioFunc parameters structure pointer.
*                                       None.
*/
static GT_VOID prvCpssTmEligPrioFuncNodeCpy
(
    IN CPSS_TM_ELIG_PRIO_FUNC_STC      *paramsPtr,
    OUT union tm_elig_prio_func        *prmsPtr
)
{
    int ii;
    int jj;
    for (ii=0; ii<8; ii++) {
        for (jj=0; jj<4; jj++) {
            prmsPtr->node_elig_prio_func[ii][jj].max_tb = (uint8_t)paramsPtr->nodeEligPrioFunc[ii][jj].maxTb;
            prmsPtr->node_elig_prio_func[ii][jj].min_tb = (uint8_t)paramsPtr->nodeEligPrioFunc[ii][jj].minTb;
            prmsPtr->node_elig_prio_func[ii][jj].prop_prio = (uint8_t)paramsPtr->nodeEligPrioFunc[ii][jj].propPrio;
            prmsPtr->node_elig_prio_func[ii][jj].sched_prio = (uint8_t)paramsPtr->nodeEligPrioFunc[ii][jj].schedPrio;
            prmsPtr->node_elig_prio_func[ii][jj].elig = (uint8_t)paramsPtr->nodeEligPrioFunc[ii][jj].elig;
        }

    }
}


/**
* @internal prvCpssTmEligPrioFuncQueueCpy function
* @endinternal
*
* @brief   Copy CPSS TmEligPrioFunc Queue structure to TM TmEligPrioFunc structure.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] paramsPtr                - CPSS TmEligPrioFunc parameters structure pointer.
*                                       None.
*/
static GT_VOID prvCpssTmEligPrioFuncQueueCpy
(
    IN CPSS_TM_ELIG_PRIO_FUNC_STC      *paramsPtr,
    OUT union tm_elig_prio_func        *prmsPtr
)
{
    int ii;
    for (ii=0; ii<4; ii++) {
        prmsPtr->queue_elig_prio_func[ii].max_tb = (uint8_t)paramsPtr->queueEligPrioFunc[ii].maxTb;
        prmsPtr->queue_elig_prio_func[ii].min_tb = (uint8_t)paramsPtr->queueEligPrioFunc[ii].minTb;
        prmsPtr->queue_elig_prio_func[ii].prop_prio = (uint8_t)paramsPtr->queueEligPrioFunc[ii].propPrio;
        prmsPtr->queue_elig_prio_func[ii].sched_prio = (uint8_t)paramsPtr->queueEligPrioFunc[ii].schedPrio;
        prmsPtr->queue_elig_prio_func[ii].elig = (uint8_t)paramsPtr->queueEligPrioFunc[ii].elig;
    }
}


/**
* @internal internal_cpssTmEligPrioFuncQueueConfig function
* @endinternal
*
* @brief   Configure the Queue Eligible Priority Function according
*         to the User Application parameters
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] eligPrioFunc             - The new configured eligible function pointer (APPLICABLE RANGES: 0..63).
* @param[in] funcOutArr               - The Eligible Priority Function structure array pointer.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note The use of Eligible Function Pointer 63 on both Queue and Node level is forbidden.
*       Eligible Function 63 is reserved to indicate that Queue/Node is not eligible.
*
*/
static GT_STATUS internal_cpssTmEligPrioFuncQueueConfig
(
    IN GT_U8                        devNum,
    IN GT_U32                       eligPrioFunc,
    IN CPSS_TM_ELIG_PRIO_FUNC_STC   *funcOutArr
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;
    union tm_elig_prio_func prms;

    CPSS_NULL_PTR_CHECK_MAC(funcOutArr);

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                           | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_PUMA_E | CPSS_PUMA3_E  );

    prvCpssTmEligPrioFuncQueueCpy(funcOutArr, &prms);

    ret = tm_elig_prio_func_config(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle, (uint16_t)eligPrioFunc,Q_LEVEL, &prms);
    rc = XEL_TO_CPSS_ERR_CODE(ret);

    return rc;
}

/**
* @internal cpssTmEligPrioFuncQueueConfig function
* @endinternal
*
* @brief   Configure the Queue Eligible Priority Function according
*         to the User Application parameters
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] eligPrioFunc             - The new configured eligible function pointer (APPLICABLE RANGES: 0..63).
* @param[in] funcOutArr               - The Eligible Priority Function structure array pointer.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note The use of Eligible Function Pointer 63 on both Queue and Node level is forbidden.
*       Eligible Function 63 is reserved to indicate that Queue/Node is not eligible.
*
*/
GT_STATUS cpssTmEligPrioFuncQueueConfig
(
    IN GT_U8                        devNum,
    IN GT_U32                       eligPrioFunc,
    IN CPSS_TM_ELIG_PRIO_FUNC_STC   *funcOutArr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmEligPrioFuncQueueConfig);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, eligPrioFunc, funcOutArr));

    rc = internal_cpssTmEligPrioFuncQueueConfig(devNum, eligPrioFunc, funcOutArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, eligPrioFunc, funcOutArr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssTmEligPrioFuncNodeConfig function
* @endinternal
*
* @brief   Configure the Node Eligible Priority Function according
*         to the User Application parameters
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] eligPrioFuncPtr          - The new configured eligible function pointer (APPLICABLE RANGES: 0..63).
* @param[in] level                    - A  to configure the Eligible function with.
* @param[in] funcOutArr               - The Eligible Priority Function structure array pointer.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note The use of Eligible Function Pointer 63 on both Queue and Node level is forbidden.
*       Eligible Function 63 is reserved to indicate that Queue/Node is not eligible.
*
*/
static GT_STATUS internal_cpssTmEligPrioFuncNodeConfig
(
    IN GT_U8                        devNum,
    IN GT_U32                       eligPrioFuncPtr,
    IN CPSS_TM_LEVEL_ENT            level,
    IN CPSS_TM_ELIG_PRIO_FUNC_STC   *funcOutArr
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;
    union tm_elig_prio_func prms;

    CPSS_NULL_PTR_CHECK_MAC(funcOutArr);

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                           | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_PUMA_E | CPSS_PUMA3_E  );

    prvCpssTmEligPrioFuncNodeCpy(funcOutArr, &prms);

    ret = tm_elig_prio_func_config(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle, (uint16_t)eligPrioFuncPtr, level, &prms);
    rc = XEL_TO_CPSS_ERR_CODE(ret);

    return rc;
}

/**
* @internal cpssTmEligPrioFuncNodeConfig function
* @endinternal
*
* @brief   Configure the Node Eligible Priority Function according
*         to the User Application parameters
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] eligPrioFuncPtr          - The new configured eligible function pointer (APPLICABLE RANGES: 0..63).
* @param[in] level                    - A  to configure the Eligible function with.
* @param[in] funcOutArr               - The Eligible Priority Function structure array pointer.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note The use of Eligible Function Pointer 63 on both Queue and Node level is forbidden.
*       Eligible Function 63 is reserved to indicate that Queue/Node is not eligible.
*
*/
GT_STATUS cpssTmEligPrioFuncNodeConfig
(
    IN GT_U8                        devNum,
    IN GT_U32                       eligPrioFuncPtr,
    IN CPSS_TM_LEVEL_ENT            level,
    IN CPSS_TM_ELIG_PRIO_FUNC_STC   *funcOutArr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmEligPrioFuncNodeConfig);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, eligPrioFuncPtr, level, funcOutArr));

    rc = internal_cpssTmEligPrioFuncNodeConfig(devNum, eligPrioFuncPtr, level, funcOutArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, eligPrioFuncPtr, level, funcOutArr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssTmEligPrioFuncConfigAllLevels function
* @endinternal
*
* @brief   Configure the Eligible Priority Function according
*         to the User Application parameters
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] eligPrioFuncPtr          - The new configured eligible function pointer (APPLICABLE RANGES: 0..63).
* @param[in] funcOutArr               - The Eligible Priority Function structure array pointer.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note The following API configures the same Eligible Priority Functions
*       at all nodes (A, B, C, and Port) levels Elig. Prio. Tables
*       according to the user’s parameters.
*       It has the same functionality as tm_elig_prio_func_config()
*       and can be used at the user convenience to configure the
*       same eligible function to all the Nodes levels (except for Q level)
*       Note:  The use of Eligible Function Pointer 63 on both Queue and Node level is forbidden.
*       Eligible Function 63 is reserved to indicate that Queue/Node is not eligible.
*
*/
static GT_STATUS internal_cpssTmEligPrioFuncConfigAllLevels
(
    IN GT_U8                        devNum,
    IN GT_U32                       eligPrioFuncPtr,
    IN CPSS_TM_ELIG_PRIO_FUNC_STC   *funcOutArr
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;
    union tm_elig_prio_func prms;

    CPSS_NULL_PTR_CHECK_MAC(funcOutArr);

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                           | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_PUMA_E | CPSS_PUMA3_E  );

    prvCpssTmEligPrioFuncNodeCpy(funcOutArr, &prms);

    ret = tm_elig_prio_func_config_all_levels(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle, (uint16_t)eligPrioFuncPtr, &prms);
    rc = XEL_TO_CPSS_ERR_CODE(ret);

    return rc;
}

/**
* @internal cpssTmEligPrioFuncConfigAllLevels function
* @endinternal
*
* @brief   Configure the Eligible Priority Function according
*         to the User Application parameters
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] eligPrioFuncPtr          - The new configured eligible function pointer (APPLICABLE RANGES: 0..63).
* @param[in] funcOutArr               - The Eligible Priority Function structure array pointer.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note The following API configures the same Eligible Priority Functions
*       at all nodes (A, B, C, and Port) levels Elig. Prio. Tables
*       according to the user’s parameters.
*       It has the same functionality as tm_elig_prio_func_config()
*       and can be used at the user convenience to configure the
*       same eligible function to all the Nodes levels (except for Q level)
*       Note:  The use of Eligible Function Pointer 63 on both Queue and Node level is forbidden.
*       Eligible Function 63 is reserved to indicate that Queue/Node is not eligible.
*
*/
GT_STATUS cpssTmEligPrioFuncConfigAllLevels
(
    IN GT_U8                        devNum,
    IN GT_U32                       eligPrioFuncPtr,
    IN CPSS_TM_ELIG_PRIO_FUNC_STC   *funcOutArr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmEligPrioFuncConfigAllLevels);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, eligPrioFuncPtr, funcOutArr));

    rc = internal_cpssTmEligPrioFuncConfigAllLevels(devNum, eligPrioFuncPtr, funcOutArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, eligPrioFuncPtr, funcOutArr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

GT_STATUS tmShowEligFunc(GT_U8 devNum, int level, GT_U16 func_index, int format)
{
    GT_STATUS   rc = GT_OK;

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
           | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_PUMA_E | CPSS_PUMA3_E  );

    rc = tm_dump_elig_func(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle, level, func_index, format);

    return  rc;
}

