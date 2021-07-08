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
* @file prvCpssTimeRtUtils.h
*
* @brief CPSS implementation of ns timer
*
* @version   1
********************************************************************************
*/
#ifndef __PRV_CPSS_TIMER_RT_NS_H
#define __PRV_CPSS_TIMER_RT_NS_H

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * typedef: struct PRV_CPSS_DRV_FLD_DEF_STC
 *
 * time in sec:nanoSec
 *
 * Fields:
 *     nanoSec - nano seconds
 *     sec - seconds;
 */
typedef struct 
{
    GT_U32 nanoSec;
    GT_U32 sec;
}GT_TIME;



/**
* @internal prvCpssOsTimeRTns function
* @endinternal
*
* @brief   get time in sec:nano-sec
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
*
* @param[out] nsPtr                    - place to store time stamp
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on hardware error
*/
GT_STATUS prvCpssOsTimeRTns
(
    OUT GT_TIME *nsPtr
);

/**
* @internal prvCpssOsTimeRTDiff function
* @endinternal
*
* @brief   callc diff ftom taken time in msec-nsec
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] start                    : time point
*
* @param[out] prv_paTime_msPtr         - place to store time diff msec
* @param[out] prv_paTime_usPtr         - place to store time diff nsec
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on hardware error
*/
GT_STATUS prvCpssOsTimeRTDiff
(
    IN   GT_TIME start,
    OUT  GT_U32 *prv_paTime_msPtr,
    OUT  GT_U32 *prv_paTime_usPtr  
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif




