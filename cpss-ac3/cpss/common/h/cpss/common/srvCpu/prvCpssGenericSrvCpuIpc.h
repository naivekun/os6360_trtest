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
* @file prvCpssGenericSrvCpuIpc.h
*
* @brief Service CPU IPC APIs
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssGenericSrvCpuIpc_h__
#define __prvCpssGenericSrvCpuIpc_h__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>

/**
* @internal prvCpssGenericSrvCpuIpcMessageSend function
* @endinternal
*
* @brief   This API allows the application to send a message to other side,
*         using a buffer it was previously fetched from the IPC (by using the
*         Buffer-Get API)
*         It may choose to send asynchrony or synchronic and in case
*         synchronic how much tome to wait for processing.
*
* @note   APPLICABLE DEVICES:      AC3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Pipe.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] fwChannel             - object ID for IPC operations, returned by
*                                    prvCpssGenericSrvCpuInit
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       - srvCpu IPC not initialized for this device
*/
GT_STATUS prvCpssGenericSrvCpuIpcMessageSend
(
    IN  GT_UINTPTR fwChannel,
    IN  GT_U32   channel,
    IN  GT_U8   *msgData,
    IN  GT_U32   msgDataSize
);

/**
* @internal prvCpssGenericSrvCpuIpcMessageRecv function
* @endinternal
*
* @brief   This API allows the application to fetch message sent by the other
*         side. Application may choose to wait for incoming message or not
*         (if application wish to use polling it can loop with no waiting).
*
* @note   APPLICABLE DEVICES:      AC3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Pipe.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] fwChannel             - object ID for IPC operations, returned by
*                                    prvCpssGenericSrvCpuInit
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_NO_MORE               - no messages ready.
* @retval GT_NOT_INITIALIZED       - srvCpu IPC not initialized for this device
*/
GT_STATUS prvCpssGenericSrvCpuIpcMessageRecv
(
    IN  GT_UINTPTR fwChannel,
    IN  GT_U32   channel,
    OUT GT_U8   *msgData,
    OUT GT_U32  *msgDataSize
);

/**
* @internal prvCpssGenericSrvCpuWrite function
* @endinternal
*
* @brief   Write to Service CPU "firmware section"
*
* @note   APPLICABLE DEVICES:      AC3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Pipe.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] fwChannel             - object ID for IPC operations, returned by
*                                    prvCpssGenericSrvCpuInit
* @param[in] srvCpuAddr            - The srvCpu internal address
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       - srvCpu IPC not initialized for this device
*/
GT_STATUS prvCpssGenericSrvCpuWrite
(
    IN  GT_UINTPTR fwChannel,
    IN  GT_U32   srvCpuAddr,
    IN  void    *data,
    IN  GT_U32   size
);

/**
* @internal prvCpssGenericSrvCpuRead function
* @endinternal
*
* @brief   Read from Service CPU "firmware section"
*
* @note   APPLICABLE DEVICES:      AC3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Pipe.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] fwChannel             - object ID for IPC operations, returned by
*                                    prvCpssGenericSrvCpuInit
* @param[in] srvCpuAddr               - The srvCpu internal address
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       - srvCpu IPC not initialized for this device
*/
GT_STATUS prvCpssGenericSrvCpuRead
(
    IN  GT_UINTPTR fwChannel,
    IN  GT_U32   srvCpuAddr,
    OUT void    *data,
    IN  GT_U32   size
);

/**
* @internal prvCpssGenericSrvCpuRegisterWriteFwChannel function
* @endinternal
*
* @brief   Read a service CPU IPC register value
*
* @note   APPLICABLE DEVICES:      AC3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Pipe.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] fwChannel             - object ID for IPC operations, returned by
*                                    prvCpssGenericSrvCpuInit
* @param[in] regAddr               - The The register's address to write to
* @param[in] data                  - The value to be written
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       - srvCpu IPC not initialized for this device
*/
GT_STATUS prvCpssGenericSrvCpuRegisterWriteFwChannel
(
    IN GT_UINTPTR   fwChannel,
    IN GT_U32       regAddr,
    IN GT_U32       value
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssGenericSrvCpuIpc_h__ */

