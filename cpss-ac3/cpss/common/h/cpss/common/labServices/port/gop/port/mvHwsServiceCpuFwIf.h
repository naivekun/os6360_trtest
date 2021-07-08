/*******************************************************************************
*                Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
*/
/**
********************************************************************************
* @file mvHwsServiceCpuFwIf.h
*
* @brief This file contains HWS Service Firmware Interface definition
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwsServiceCpuFwIF_H
#define __mvHwsServiceCpuFwIF_H

#ifdef __cplusplus
extern "C" {
#endif

/**
* @internal mvHwsServiceCpuEnable function
* @endinternal
*
* @brief   Service CPU Enable for device
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsServiceCpuEnable(GT_U8 devNum, GT_U32 enable);

/**
* @internal mvHwsServiceCpuEnableGet function
* @endinternal
*
* @brief   Return Service CPU Enable state
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_U32 mvHwsServiceCpuEnableGet(GT_U8 devNum);

/**
* @internal mvHwsServiceCpuDbaEnable function
* @endinternal
*
* @brief   Service CPU Enable for device for DBA
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsServiceCpuDbaEnable(GT_U8 devNum, GT_U32 enable);

/**
* @internal mvHwsServiceCpuDbaEnableGet function
* @endinternal
*
* @brief   Return Service CPU Enable state for DBA
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_U32 mvHwsServiceCpuDbaEnableGet(GT_U8 devNum);

/**
* @internal mvHwsServiceCpuDbaFwInit function
* @endinternal
*
* @brief   HW Services Firmware initialization
*         DBA Firmware loading and Activate (take out of reset)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsServiceCpuDbaFwInit(GT_U8 devNum);

/**
* @internal mvHwsServiceCpuFwInit function
* @endinternal
*
* @brief   HWS FW initialization.
*
* @param[in] devNum   - device number
* @param[in] scpuId   - service CPU id:
*                       0 - CM3_0
*                       1 - CM3_1
*                       ...
*                       0xffffffff - MSYS
* @param[in] fwId     - firmware identifier string
*                       "AP_BobK", "AP_Bobcat3", ...
* @returns              GT_STATUS
*                       GT_NO_SUCH scpuId not applicable for this PP
*/
GT_STATUS mvHwsServiceCpuFwInit(
    GT_U8 devNum,
    GT_U32 scpuId,
    const char *fwId
);

/**
* @internal mvHwsServiceCpuFwPostInit function
* @endinternal
*
* @brief   Init HWS Firmware After Avago Serdes init
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsServiceCpuFwPostInit(GT_U8 devNum);


GT_STATUS mvHwsServiceCpuUartCtrlSet(GT_BOOL enable);


GT_STATUS mvHwsServiceCpuUartCtrlGet(GT_BOOL *enable);


#ifdef __cplusplus
}
#endif

#endif /* __mvHwsServiceCpuFwIF_H */



