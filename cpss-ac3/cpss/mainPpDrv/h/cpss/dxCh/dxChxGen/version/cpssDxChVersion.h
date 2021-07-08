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
* @file cpssDxChVersion.h
*
* @brief Includes software CPSS DxCh version information.
*
*
* @version   5
********************************************************************************
*/
#ifndef __cpssDxChVersionh
#define __cpssDxChVersionh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/version/cpssGenVersion.h>
#include <cpssCommon/cpssPresteraDefs.h>

/* string to define the CPSS DxCh version used */
#define CPSS_DXCH_VERSION_CNS        "CPSS 4.2.2018.8"

/**
* @internal cpssDxChVersionGet function
* @endinternal
*
* @brief   This function returns CPSS DxCh version.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @param[out] versionPtr               - (pointer to)CPSS DxCh version info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one parameter is NULL pointer
* @retval GT_BAD_SIZE              - the version name is too long
*/
GT_STATUS cpssDxChVersionGet
(
    OUT CPSS_VERSION_INFO_STC   *versionPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChVersionh */


