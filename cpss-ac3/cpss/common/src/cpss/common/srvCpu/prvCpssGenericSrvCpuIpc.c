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
* @file prvCpssGenericSrvCpuIpc.c
*
* @brief Service CPU IPC APIs
*
* @version   1
********************************************************************************
*/

#include <cpss/common/srvCpu/prvCpssGenericSrvCpuIpc.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpssDriver/pp/hardware/cpssDriverPpHw.h>
#include "prvCpssGenericSrvCpuIpcDevCfg.h"

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
* @note   APPLICABLE DEVICES:      AC3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
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
)
{
    PRV_SRVCPU_IPC_CHANNEL_STC *s = (PRV_SRVCPU_IPC_CHANNEL_STC*)fwChannel;

    if (s == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);

    if (shmIpcSend(&(s->shm), channel, msgData, msgDataSize) != 0)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    return GT_OK;
}

/**
* @internal prvCpssGenericSrvCpuIpcMessageRecv function
* @endinternal
*
* @brief   This API allows the application to fetch message sent by the other
*         side. Application may choose to wait for incoming message or not
*         (if application wish to use polling it can loop with no waiting).
*
* @note   APPLICABLE DEVICES:      AC3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
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
)
{
    int     rc;
    int     size = 1024;
    PRV_SRVCPU_IPC_CHANNEL_STC *s = (PRV_SRVCPU_IPC_CHANNEL_STC*)fwChannel;

    if (s == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);

    rc = shmIpcRecv(&(s->shm), channel, msgData, &size);
    if (rc < 0)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    if (rc == 0)
    {
        CPSS_LOG_INFORMATION_MAC("prvCpssGenericSrvCpuIpcMessageRecv return code is %d", GT_NO_MORE);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_MORE, LOG_ERROR_NO_MSG);
    }

    if (msgDataSize != NULL)
        *msgDataSize = (GT_U32)size;
    return GT_OK;
}



/**
* @internal prvCpssGenericSrvCpuWrite function
* @endinternal
*
* @brief   Write to Service CPU "firmware section"
*
* @note   APPLICABLE DEVICES:      AC3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
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
)
{
    PRV_SRVCPU_IPC_CHANNEL_STC *s = (PRV_SRVCPU_IPC_CHANNEL_STC*)fwChannel;

    if (s == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);

    s->drv->writeMask(s->drv, 0, srvCpuAddr, (GT_U32*)data, (size+3)/4, 0xffffffff);
    return GT_OK;
}

/**
* @internal prvCpssGenericSrvCpuRead function
* @endinternal
*
* @brief   Read from Service CPU "firmware section"
*
* @note   APPLICABLE DEVICES:      AC3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
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
)
{
    PRV_SRVCPU_IPC_CHANNEL_STC *s = (PRV_SRVCPU_IPC_CHANNEL_STC*)fwChannel;

    if (s == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);

    s->drv->read(s->drv, 0, srvCpuAddr, (GT_U32*)data, (size+3)/4);
    return GT_OK;
}

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
)
{
    PRV_SRVCPU_IPC_CHANNEL_STC *s = (PRV_SRVCPU_IPC_CHANNEL_STC*)fwChannel;

    if (s == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    shmIpcRegWrite(&(s->shm), regAddr, value);
    return GT_OK;
}

#ifndef ASIC_SIMULATION
/**
* @internal prvCpssSrvCpuIpcHandleIrq function
* @endinternal
*
* @brief   Handle irq: retrieve IPC messages and put them to messagequeues
*
* @note   APPLICABLE DEVICES:      AC3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] intIndex                 - The interrupt index.
*                                       None.
*/
static GT_VOID prvCpssSrvCpuIpcHandleIrq(
    IN GT_U8    devNum,
    IN GT_U32   intIndex GT_UNUSED
)
{
#ifndef __GNUC__
    (void)intIndex; /* unused */
#endif
    /*TODO: handle IPC incoming messages */
cpssOsPrintf("Doorbell IRQ devNum=%d\n",devNum);
}
#endif /* !defined(ASIC_SIMULATION) */

/**
* @internal prvCpssGenericSrvCpuRegisterRead function
* @endinternal
*
* @brief   Read a service CPU IPC register value
* @param[in] devNum                - The PP to read from.
* @param[in] portGroupId           - The port group id. Not used
* @param[in] regAddr               - The register's address to read from.
*                                    Note - regAddr should be < 0x1000000
*
* @param[out] dataPtr              - Includes the register value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
*/
GT_STATUS prvCpssGenericSrvCpuRegisterRead
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId GT_UNUSED,
    IN GT_U32   regAddr,
    IN GT_U32   *dataPtr
)
{
#ifndef ASIC_SIMULATION
    PRV_SRVCPU_IPC_CHANNEL_STC *s;

#ifndef __GNUC__
    (void)portGroupId; /* unused */
#endif
    *dataPtr = 0;
    if (devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    s = prvIpcDevCfg[devNum];
    if (s == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    *dataPtr = shmIpcRegRead(&(s->shm), regAddr);
#else
    (void)devNum;
    (void)portGroupId;
    (void)regAddr;
    *dataPtr = 0;
#endif
    return GT_OK;
}

/**
* @internal prvCpssGenericSrvCpuRegisterWrite function
* @endinternal
*
* @brief   Read a service CPU IPC register value
*
* @param[in] devNum                - The PP to write to.
* @param[in] portGroupId           - The port group id. Not used
* @param[in] regAddr               - The register's address to write to.
* @param[in] value                 - The value to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
*/
GT_STATUS prvCpssGenericSrvCpuRegisterWrite
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId GT_UNUSED,
    IN GT_U32   regAddr,
    IN GT_U32   value
)
{
#ifndef ASIC_SIMULATION
    PRV_SRVCPU_IPC_CHANNEL_STC *s;

#ifndef __GNUC__
    (void)portGroupId; /* unused */
#endif
    if (devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    s = prvIpcDevCfg[devNum];
    if (s == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    shmIpcRegWrite(&(s->shm), regAddr, value);
#else
    (void)devNum;
    (void)portGroupId;
    (void)regAddr;
    (void)value;
#endif
    return GT_OK;
}

/**
* @internal prvCpssGenericSrvCpuIpcInitIrq function
* @endinternal
*
* @brief   Bind MSYS/CM3 doorbell interrupt
*
* @note   APPLICABLE DEVICES:      AC3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvCpssGenericSrvCpuIpcInitIrq
(
    IN GT_U8    devNum
)
{
#ifndef ASIC_SIMULATION
    PRV_SRVCPU_IPC_CHANNEL_STC *s;
    GT_STATUS rc = GT_OK;
    PRV_CPSS_DRV_EV_REQ_NODE_STC *intReqNodeListPtr;
    GT_U32 numInts, i;

    if (devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    s = prvIpcDevCfg[devNum];
    if (s == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);

    if (s->irqBound != GT_FALSE)
        return GT_OK;

    numInts = prvCpssDrvPpConfig[devNum]->intCtrl.numOfIntBits;

    intReqNodeListPtr = prvCpssDrvPpConfig[devNum]->intCtrl.portGroupInfo[0].intNodesPool;
    for (i = 0; i < numInts; i++)
    {
        if (intReqNodeListPtr[i].uniEvCause == CPSS_SRVCPU_IPC_E)
        {
            intReqNodeListPtr[i].intCbFuncPtr = prvCpssSrvCpuIpcHandleIrq;
            break;
        }
    }
    if (i >= numInts)
    {
        cpssOsPrintf("Failed to set callback for CPSS_SRVCPU_IPC_E\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    rc = cpssEventDeviceMaskSet(devNum,
            CPSS_SRVCPU_IPC_E,
            CPSS_EVENT_UNMASK_E);
    if (rc != GT_OK)
    {
        cpssOsPrintf("cpssEventDeviceMaskSet(CPSS_SRVCPU_IPC_E): rc=%d\n", rc);
        return rc;
    }
    s->irqBound = GT_TRUE;

    return rc;
#else
    (void)devNum;
    return GT_OK;
#endif
}
