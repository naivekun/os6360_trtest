/*******************************************************************************
*              (c), Copyright 2015, Marvell International Ltd.                 *
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
* @file prvCpssGenericSrvCpuLoad.h
*
* @brief Service CPU firmware load/start APIs
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssGenericSrvCpuLoad_h__
#define __prvCpssGenericSrvCpuLoad_h__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssHwInfo.h>

typedef struct PRV_CPSS_GENERIC_SRV_CPU_IPC_CHANNEL_CONFIG_STCT {
    GT_U8   channel; /* 0xff for end */
    GT_U32  maxrx;
    GT_U32  rxsize;
    GT_U32  maxtx;
    GT_U32  txsize;
} PRV_CPSS_GENERIC_SRV_CPU_IPC_CHANNEL_CONFIG_STC;


/**
* @internal prvCpssGenericSrvCpuInitCust function
* @endinternal
*
* @brief   Upload firmware to service CPU
*          Start it, initialize IPC
*
* @note   APPLICABLE DEVICES:      AC3; Bobcat2; Caelum; Cetus; Aldrin; Bobcat3; PIPE;
* @note   NOT APPLICABLE DEVICES:  Lion2;
*
* @param[in] hwInfo                - CPSS_HW_INFO_STC*
* @param[in] drv                   - CPSS_HW_DRIVER_STC* or NULL
* @param[in] scpuId                - service CPU id:
*                                    0 - CM3_0
*                                    1 - CM3_1
*                                    ...
*                                    0xffffffff - MSYS
* @param[in] fwId                  - firmware identifier string
*                                    "AP_BobK", "AP_Bobcat3", "DBA_xxxx"
* @param[in] ipcConfig             - IPC configuration, optional
*                                    should end with channel==0xff
* @param[in] preStartCb            - A custom routine executed before unreset
*                                    service CPU. (optional can be NULL)
* @param[in] preStartCookie        - parameter for preStartCb
* @param[out] fwChannelPtr         - object ID for IPC operations with this
*                                    service CPU. If this pointer is NULL, then
*                                    access object destroyed before return
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_SUPPORTED         - no service CPU on this PP,
* @retval GT_NO_SUCH               - scpuId not applicable for this PP
*                                    or firmare not found
* @retval GT_FAIL                  - on error.
*/
GT_STATUS prvCpssGenericSrvCpuInitCust
(
    IN  CPSS_HW_INFO_STC    *hwInfo,
    IN  CPSS_HW_DRIVER_STC  *drv,
    IN  GT_U32      scpuId,
    IN  const char *fwId,
    IN  PRV_CPSS_GENERIC_SRV_CPU_IPC_CHANNEL_CONFIG_STC *ipcConfig,
    IN  GT_STATUS (*preStartCb)(GT_UINTPTR  fwChannel, GT_UINTPTR  cookie),
    IN  GT_UINTPTR  preStartCookie,
    OUT GT_UINTPTR *fwChannelPtr
);

/**
* @internal prvCpssGenericSrvCpuInit function
* @endinternal
*
* @brief   Upload firmware to service CPU
*          Start it, initialize IPC
*
* @note   APPLICABLE DEVICES:      AC3; Bobcat2; Caelum; Cetus; Aldrin; Bobcat3; PIPE;
* @note   NOT APPLICABLE DEVICES:  Lion2;
*
* @param[in] devNum                - device number.
* @param[in] scpuId                - service CPU id:
*                                    0 - CM3_0
*                                    1 - CM3_1
*                                    ...
*                                    0xffffffff - MSYS
* @param[in] fwId                  - firmware identifier string
*                                    "AP_BobK", "AP_Bobcat3", "DBA_xxxx"
* @param[in] ipcConfig             - IPC configuration, optional
*                                    should end with channel==0xff
* @param[in] preStartCb            - A custom routine executed before unreset
*                                    service CPU. (optional can be NULL)
* @param[in] preStartCookie        - parameter for preStartCb
* @param[out] fwChannelPtr         - object ID for IPC operations with this
*                                    service CPU. If this pointer is NULL, then
*                                    access object destroyed before return
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_SUPPORTED         - no service CPU on this PP,
* @retval GT_NO_SUCH               - scpuId not applicable for this PP
*                                    or firmare not found
* @retval GT_FAIL                  - on error.
* @retval GT_ALREADY_EXIST         - Already initialized for given devNum
*/
GT_STATUS prvCpssGenericSrvCpuInit
(
    IN  GT_U8       devNum,
    IN  GT_U32      scpuId,
    IN  const char *fwId,
    IN  PRV_CPSS_GENERIC_SRV_CPU_IPC_CHANNEL_CONFIG_STC *ipcConfig,
    IN  GT_STATUS (*preStartCb)(GT_UINTPTR  fwChannel, GT_UINTPTR  cookie),
    IN  GT_UINTPTR  preStartCookie,
    OUT GT_UINTPTR *fwChannelPtr
);

/**
* @internal prvCpssGenericSrvCpuProbeCust function
* @endinternal
*
* @brief   Probe Service CPU
*          Start it, initialize IPC
*
* @note   APPLICABLE DEVICES:      AC3; Bobcat2; Caelum; Cetus; Aldrin; Bobcat3; PIPE;
* @note   NOT APPLICABLE DEVICES:  Lion2;
*
* @param[in] hwInfo                - CPSS_HW_INFO_STC*
* @param[in] drv                   - CPSS_HW_DRIVER_STC* or NULL
* @param[in] scpuId                - service CPU id:
*                                    0 - CM3_0
*                                    1 - CM3_1
*                                    ...
*                                    0xffffffff - MSYS
* @param[in] hasIPC                - GT_TRUE if IPC should be probed on this
*                                    serviceCPU
* @param[in] checkIpcCb            - A custom routine executed when IPC structure
*                                    detected. Could be used for check transaction
*                                    Returns GT_TRUE if success
* @param[in] checkIpcCookie        - parameter for checkIpcCb
* @param[out] fwChannelPtr         - object ID for IPC operations with this
*                                    service CPU. If this pointer is NULL, then
*                                    access object destroyed before return
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_SUPPORTED         - no service CPU on this PP,
* @retval GT_NO_SUCH               - scpuId not applicable for this PP
*                                    or firmare not found
* @retval GT_FAIL                  - on error.
* @retvel GT_NOT_INITIALIZED       - not initialized. FW chould be reloaded
*/
GT_STATUS prvCpssGenericSrvCpuProbeCust
(
    IN  CPSS_HW_INFO_STC    *hwInfo,
    IN  CPSS_HW_DRIVER_STC  *drv,
    IN  GT_U32      scpuId,
    IN  GT_BOOL     hasIPC,
    IN  GT_BOOL (*checkIpcCb)(GT_UINTPTR  fwChannel, GT_UINTPTR  cookie),
    IN  GT_UINTPTR checkIpcCookie,
    OUT GT_UINTPTR *fwChannelPtr
);

/**
* @internal prvCpssGenericSrvCpuProbe function
* @endinternal
*
* @brief   Probe Service CPU
*          Start it, initialize IPC
*
* @note   APPLICABLE DEVICES:      AC3; Bobcat2; Caelum; Cetus; Aldrin; Bobcat3; PIPE;
* @note   NOT APPLICABLE DEVICES:  Lion2;
*
* @param[in] devNum                - device number.
* @param[in] scpuId                - service CPU id:
*                                    0 - CM3_0
*                                    1 - CM3_1
*                                    ...
*                                    0xffffffff - MSYS
* @param[in] hasIPC                - GT_TRUE if IPC should be probed on this
*                                    serviceCPU
* @param[in] checkIpcCb            - A custom routine executed when IPC structure
*                                    detected. Could be used for check transaction
*                                    Returns GT_TRUE if success
* @param[in] checkIpcCookie        - parameter for checkIpcCb
* @param[out] fwChannelPtr         - object ID for IPC operations with this
*                                    service CPU. If this pointer is NULL, then
*                                    access object destroyed before return
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_SUPPORTED         - no service CPU on this PP,
* @retval GT_NO_SUCH               - scpuId not applicable for this PP
*                                    or firmare not found
* @retval GT_FAIL                  - on error.
* @retvel GT_NOT_INITIALIZED       - not initialized. FW chould be reloaded
* @retval GT_ALREADY_EXIST         - Already initialized for given devNum
*/
GT_STATUS prvCpssGenericSrvCpuProbe
(
    IN  GT_U8       devNum,
    IN  GT_U32      scpuId,
    IN  GT_BOOL     hasIPC,
    IN  GT_BOOL (*checkIpcCb)(GT_UINTPTR  fwChannel, GT_UINTPTR  cookie),
    IN  GT_UINTPTR checkIpcCookie,
    OUT GT_UINTPTR *fwChannelPtr
);

/**
* @internal prvCpssDbaToSrvCpuLoad function
* @endinternal
*
* @brief   Upload DBA FW to second service CPU
*
* @note   APPLICABLE DEVICES:      Bobcat3;
* @note   NOT APPLICABLE DEVICES:  AC3; Bobcat2; Caelum; Cetus; Aldrin;
*
* @param[in] devNum                - device number.
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_SUPPORTED         - no service CPU on this PP,
* @retval GT_NO_SUCH               - scpuId not applicable for this PP
*                                    or firmare not found
* @retval GT_FAIL                  - on error.
*/
GT_STATUS prvCpssDbaToSrvCpuLoad
(
    IN GT_U8    devNum
);

/**
* @internal prvCpssGenericSrvCpuRemove function
* @endinternal
*
* @brief   Cleanup service CPU resources while cpssDxChCfgDevRemove
*
* @note   APPLICABLE DEVICES:      AC3; Bobcat2; Caelum; Cetus; Aldrin; Bobcat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2;
*
* @param[in] devNum                   - device number.
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
*/
GT_STATUS prvCpssGenericSrvCpuRemove
(
    IN GT_U8    devNum
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssGenericSrvCpuLoad_h__ */
