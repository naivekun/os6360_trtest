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
* @file prvCpssTmDrop.h
*
* @brief TM Drop Unit configuration privat file.
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssTmDroph
#define __prvCpssTmDroph

#include <cpss/generic/tm/cpssTmPublicDefs.h>

/**
* @internal prvCpssTmDropProfileGetNext function
* @endinternal
*
* @brief   Get Next Drop profile.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] level                    - A Nodes  for the next Drop profile (Port/C/B/A/Q).
* @param[in] cos                      - CoS of Drop profile (APPLICABLE RANGES: 0..7).
* @param[in,out] profileIndPtr            - (pointer to) Drop profile Index (APPLICABLE RANGES: 0..MAX_Drop_Profile_Per_Level).
*
* @retval GT_OK                    - next drop profile does exist.
* @retval GT_NOT_FOUND             - next drop profile index does not exist.
*/
GT_STATUS prvCpssTmDropProfileGetNext
(
    IN GT_U8                            devNum,
    IN CPSS_TM_LEVEL_ENT                level,
    IN GT_U32                           cos,
    INOUT GT_U32                        *profileIndPtr
);

#endif 	    /* __prvCpssTmDroph */



