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
* @file cpssTmStatistics.h
*
* @brief TM statistics gathering APIs.
*
* @version   1
********************************************************************************
*/

#ifndef __cpssTmStatisticsh
#define __cpssTmStatisticsh

#include <cpss/generic/tm/cpssTmPublicDefs.h>


/**
* @internal cpssTmQmrPktStatisticsGet function
* @endinternal
*
* @brief   Read QMR Packet Statistics.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
*
* @param[out] pktCntPtr                - (pointer to) QMR Pkt Statistics structure.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmQmrPktStatisticsGet
(
    IN   GT_U8                           devNum,
    OUT  CPSS_TM_QMR_PKT_STATISTICS_STC  *pktCntPtr
);


/**
* @internal cpssTmRcbPktStatisticsGet function
* @endinternal
*
* @brief   Read RCB Packet Statistics.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
*
* @param[out] pktCntPtr                - (pointer to) RCB Pkt Statistics structure.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmRcbPktStatisticsGet
(
    IN   GT_U8                           devNum,
    OUT  CPSS_TM_RCB_PKT_STATISTICS_STC  *pktCntPtr
);


#endif 	    /* __cpssTmStatisticsh */


