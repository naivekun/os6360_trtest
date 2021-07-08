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
* @file cpssDxChCatchUp.h
*
* @brief CPSS DxCh CatchUp functions.
*
* @version   1
********************************************************************************
*/

#ifndef __cpssDxChCatchUph
#define __cpssDxChCatchUph

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/common/port/cpssPortCtrl.h>

/**
* @internal cpssDxChCatchUpPortModeDbSet function
* @endinternal
*
* @brief   This API fixes port interface mode value in software DB for specified
*         port during system recovery process
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] port                     -  number
* @param[in] ifMode                   - Interface mode
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_STATE             - on not within system recovery process
*
* @note API should be called only within recovery process, otherwise
*       GT_BAD_STATE is returned
*       There are cases in which the Catchup can't decide explicity the Port
*       Interface Mode, so port interface mode is updated in the CPSS port
*       database with wrong value.
*       For example: KR and SR/LR. Catchup can not distinguish between the 2 modes,
*       since the parameters in HW may be the same for both Port Interface Modes.
*       In such cases application involvment is needed in order to update CPSS
*       Port database.
*       This API updates CPSS Port Datadase with port interface mode values.
*
*/
GT_STATUS cpssDxChCatchUpPortModeDbSet
(
    IN GT_U8                        devNum,
    IN GT_PHYSICAL_PORT_NUM         port,
    IN CPSS_PORT_INTERFACE_MODE_ENT ifMode
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChCatchUph */


