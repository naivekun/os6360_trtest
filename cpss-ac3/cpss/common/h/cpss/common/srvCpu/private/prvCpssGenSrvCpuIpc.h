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
* @file prvCpssGenSrvCpuIpc.h
*
* @brief Service CPU IPC API's
*
*
* @version   00
********************************************************************************
*/

#ifndef __prvCpssGenSrvCpuIpch
#define __prvCpssGenSrvCpuIpch

#include <cpss/common/cpssTypes.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvCpssGenSrvCpuIpcMessageSend function
* @endinternal
*
* @brief   Send a message to the service CPU.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2;
*
* @param[in] devNum                   - device number.
* @param[in] msgPtr                   - (pointer to) message.
* @param[in] msgSize                  - message size.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvCpssGenSrvCpuIpcMessageSend
(
    IN GT_U8    devNum,
    IN GT_U8    *msgPtr,
    IN GT_U32   msgSize
);

/**
* @internal prvCpssGenSrvCpuIpcMessageGet function
* @endinternal
*
* @brief   Get a message from the service CPU.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2;
*
* @param[in] devNum                   - device number.
*
* @param[out] msgPtr                   - (pointer to) message.
* @param[out] msgSizePtr               - (pointer to) message size.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvCpssGenSrvCpuIpcMessageGet
(
    IN GT_U8    devNum,
    OUT GT_U8   *msgPtr,
    OUT GT_U32  *msgSizePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssGenSrvCpuIpch */

