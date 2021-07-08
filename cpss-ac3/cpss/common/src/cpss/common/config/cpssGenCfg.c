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
* @file cpssGenCfg.c
*
* @brief CPSS generic configuration functions.
*
* @version   3
********************************************************************************
*/
#ifdef CHX_FAMILY
#define CPSS_LOG_IN_MODULE_ENABLE
#endif /*CHX_FAMILY*/

#include <cpss/common/config/private/prvCpssCommonConfigLog.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>

#define DEVICE_NOT_EXISTS_CNS 0xFF

/**
* @internal internal_cpssPpCfgNextDevGet function
* @endinternal
*
* @brief   Return the number of the next existing device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number to start from. For the first one  should be 0xFF.
*
* @param[out] nextDevNumPtr            - number of next device after devNum.
*
* @retval GT_OK                    - on success.
* @retval GT_NO_MORE               - devNum is the last device. nextDevNumPtr will be set to 0xFF.
* @retval GT_BAD_PARAM             - devNum > max device number
* @retval GT_BAD_PTR               - nextDevNumPtr pointer is NULL.
*/
static GT_STATUS internal_cpssPpCfgNextDevGet
(
    IN  GT_U8 devNum,
    OUT GT_U8 *nextDevNumPtr
)
{
    GT_U8 num;

    /* device should be in the allowed range but not necessary exist */
    if ((devNum > PRV_CPSS_MAX_PP_DEVICES_CNS) && (devNum != DEVICE_NOT_EXISTS_CNS))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (nextDevNumPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    if (devNum == 0xFF)
        num = 0;
    else
        num = (GT_U8)(devNum+1);

    for (; num<PRV_CPSS_MAX_PP_DEVICES_CNS; num++)
    {
        if (prvCpssPpConfig[num] != NULL)
        {
            *nextDevNumPtr = num;
            return GT_OK;
        }
    }

    /* no more devices found */
    *nextDevNumPtr = DEVICE_NOT_EXISTS_CNS;
    return /* it's not error for log */ GT_NO_MORE;
}

/**
* @internal cpssPpCfgNextDevGet function
* @endinternal
*
* @brief   Return the number of the next existing device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number to start from. For the first one  should be 0xFF.
*
* @param[out] nextDevNumPtr            - number of next device after devNum.
*
* @retval GT_OK                    - on success.
* @retval GT_NO_MORE               - devNum is the last device. nextDevNumPtr will be set to 0xFF.
* @retval GT_BAD_PARAM             - devNum > max device number
* @retval GT_BAD_PTR               - nextDevNumPtr pointer is NULL.
*/
GT_STATUS cpssPpCfgNextDevGet
(
    IN  GT_U8 devNum,
    OUT GT_U8 *nextDevNumPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPpCfgNextDevGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, nextDevNumPtr));

    rc = internal_cpssPpCfgNextDevGet(devNum, nextDevNumPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, nextDevNumPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

