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
* @file mvHwsDiagnostic.h
*
* @brief
*
*/

#ifndef __mvHwsDiagnostic_H
#define __mvHwsDiagnostic_H

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>

/**
* @internal mvHwsDiagDeviceDbCheck function
* @endinternal
*
* @brief   This API checks the DB initialization of PCS, MAC, Serdes
*         and DDR units
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS mvHwsDiagDeviceDbCheck(GT_U8 devNum, MV_HWS_DEV_TYPE devType);

/**
* @internal mvHwsAvsVoltageGet function
* @endinternal
*
* @brief   Get the Avs voltage in mv.
*
* @param[in] devNum                   - system device number
*
* @param[out] avsVol                - Avs voltage in mv
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvsVoltageGet(GT_U8 devNum, GT_U32 *avsVol);

/**
* @internal mvHwsAvsVoltageSet function
* @endinternal
*
* @brief   Set the Avs voltage in mv.
*
* @param[in] devNum                  - system device number
* @param[in] vcore                   - Avs voltage requsted in
*       mv.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvsVoltageSet(GT_U8 devNum, GT_U32 vcore);

#endif /* __mvHwsDiagnostic_H */








