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
* @file prvCpssTimeRtUtils.c
*
* @brief CPSS implementation of ns timer
*
* @version   1
********************************************************************************
*/
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/generic/private/utils/prvCpssTimeRtUtils.h>

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
)
{
    GT_STATUS rc;
    GT_U32 secondsStart;
    GT_U32 nanoSecondsStart;

    if (nsPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    rc = cpssOsTimeRT(&secondsStart,&nanoSecondsStart);
    if (rc != GT_OK)
    {
        return rc;
    }
    nsPtr->nanoSec = nanoSecondsStart;
    nsPtr->sec     = secondsStart;
    return GT_OK;
}


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
)
{
    GT_STATUS rc;
    GT_TIME end;

    GT_U32 secStart;
    GT_U32 nSecStart;
    GT_U32 secEnd;
    GT_U32 nSecEnd;

    GT_U32 seconds;
    GT_U32 nSec;

    if (prv_paTime_msPtr == NULL || prv_paTime_usPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssOsTimeRTns(&end);
    if (rc != GT_OK)
    {
        return rc;
    }


    secStart  = start.sec;
    nSecStart = start.nanoSec;

    secEnd  =  end.sec;
    nSecEnd =  end.nanoSec;

    seconds = secEnd - secStart;
    if(nSecEnd >= nSecStart)
    {
        nSec = nSecEnd - nSecStart;
    }
    else
    {
        nSec = (1000000000 - nSecEnd) + nSecStart;
        seconds--;
    }

    *prv_paTime_msPtr = nSec/1000000;
    *prv_paTime_usPtr = (nSec%1000000)/1000;
    return GT_OK;
}



