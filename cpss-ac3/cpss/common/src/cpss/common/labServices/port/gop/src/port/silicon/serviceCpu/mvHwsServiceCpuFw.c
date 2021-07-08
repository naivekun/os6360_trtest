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
* @file mvHwsServiceCpuFw.c
*
* @brief This file contains HWS Firmware loading support
*
* @version   1
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortMiscIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortApInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsIpcApis.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCtrlApInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsIpcDefs.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/common/labServices/port/gop/common/os/mvSemaphore.h>
#if defined(CPU_BE)
#include <cpssCommon/cpssPresteraDefs.h>
#include <aapl.h>
#include <aapl_core.h>
#endif


#include <cpss/common/srvCpu/prvCpssGenericSrvCpuLoad.h>

/* TODO:
 * Implement interrupt handling for user space IPC
 */

/**************************** Pre-Declaration ********************************************/

/**************************** Globals *****************************************/

GT_U32 mvHwsServiceCpuEnabled[HWS_MAX_DEVICE_NUM] = {0};
GT_U32 mvHwsServiceCpuDbaEnabled = 0;

/* loading FW from NFS variables */
GT_BOOL loadFromNfs = GT_FALSE;
GT_U32 nfsFwSize;

/**************************** Functions *****************************************/
extern void mvHwsAvagoAaplAddrGet(unsigned char devNum, unsigned int **devAddr,
                                  unsigned int *devSize);

extern void mvHwsApPortCtrlDebugParamsSet(GT_U32 apFwLogBaseAddr, GT_U32 apFwLogCountAddr,
                                          GT_U32 apFwLogPointerAddr, GT_U32 apFwLogResetAddr);

extern void mvHwsLogParamsSet(GT_U32 fwLogBaseAddr,GT_U32 fwLogCountAddr,
                        GT_U32 fwLogPointerAddr, GT_U32 fwLogResetAddr);

#if !defined(MV_HWS_AVAGO_NO_VOS_WA) && !defined(ASIC_SIMULATION)
extern GT_STATUS mvHwsAvagoSerdesVosParamsGet(GT_U8 devNum, GT_U32 *vosOverrideParamsBufferPtr);
extern GT_U32 minVosValue;
extern GT_U32 baseVosValue;
/* eFuse Database flag*/
extern GT_BOOL serdesVos64BitDbInitialized;
#endif /* !defined(MV_HWS_AVAGO_NO_VOS_WA) && !defined(ASIC_SIMULATION) */

/**
* @internal mvHwsServiceCpuEnable function
* @endinternal
*
* @brief   Service CPU Enable for device
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsServiceCpuEnable(GT_U8 devNum, GT_U32 enable)
{
    mvHwsServiceCpuEnabled[devNum] = enable;

    return GT_OK;
}

/**
* @internal mvHwsServiceCpuEnableGet function
* @endinternal
*
* @brief   Return Service CPU Enable state
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_U32 mvHwsServiceCpuEnableGet(GT_U8 devNum)
{
    return (mvHwsServiceCpuEnabled[devNum]);
}

/**
* @internal mvHwsServiceCpuDbaEnable function
* @endinternal
*
* @brief   Service CPU Enable for device for DBA
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsServiceCpuDbaEnable(GT_U8 devNum, GT_U32 enable)
{
    devNum = devNum;
    mvHwsServiceCpuDbaEnabled = enable;

    return GT_OK;
}

/**
* @internal mvHwsServiceCpuDbaEnableGet function
* @endinternal
*
* @brief   Return Service CPU Enable state for DBA
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_U32 mvHwsServiceCpuDbaEnableGet(GT_U8 devNum)
{
    devNum = devNum;
    return (mvHwsServiceCpuDbaEnabled);
}

/**
* @internal mvHwsServiceCpuDbaFwInit function
* @endinternal
*
* @brief   HW Services Firmware initialization
*         DBA Firmware loading and Activate (take out of reset)
* @param[in] devNum                   - device number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsServiceCpuDbaFwInit
(
    GT_U8 devNum
)
{
    GT_STATUS rc;

    rc = prvCpssDbaToSrvCpuLoad(devNum);

    return rc;
}

extern GT_UINTPTR hwsIpcChannelHandlers[HWS_MAX_DEVICE_NUM];
static GT_STATUS prvMvHwsPreStartAP(
    GT_UINTPTR  fwChannel,
    GT_UINTPTR  cookie
)
{
    prvCpssGenericSrvCpuRegisterWriteFwChannel(fwChannel, 2, (GT_U32)cookie);
    return GT_OK;
}
#if defined(CPU_LE) && !defined(CPSS_32BIT_LE)
#define CPSS_32BIT_LE(X)    (X)
#endif
static GT_BOOL prvMvHwsCheckAPIPC(
    GT_UINTPTR  fwChannel,
    GT_UINTPTR  cookie
)
{
    GT_STATUS rc;
    MV_HWS_IPC_CTRL_MSG_STRUCT msgData;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;
    int     chn, tries;

    /* cleanup input channels, assume up to 8 msgs per channel */
    for (chn = MV_HWS_IPC_TX_0_CH_ID; chn <= MV_HWS_IPC_TX_3_CH_ID; chn++)
    {
        for (tries = 0; tries < 8; tries++)
        {
            rc = prvCpssGenericSrvCpuIpcMessageRecv(fwChannel,
                    chn, (GT_U8*)&replyData, NULL);
            if (rc != GT_OK)
                break;
        }
    }

    cpssOsMemSet(&msgData, 0, sizeof(msgData));
    cpssOsMemSet(&replyData, 0, sizeof(replyData));

    msgData.msgData.noop.data = CPSS_32BIT_LE(3);
    msgData.ctrlMsgType = MV_HWS_IPC_NOOP_MSG;
    msgData.devNum = (GT_U8)cookie;
    msgData.msgLength = (GT_U8)(sizeof(msgData.msgData.noop));
    msgData.msgQueueId = (GT_U8)MV_HWS_IPC_TX_0_CH_ID;
#define IPC_OFFSETOF(_type,_member) ((GT_U32)((GT_UINTPTR)&(((_type*)NULL)->_member)))
    rc = prvCpssGenericSrvCpuIpcMessageSend(fwChannel, MV_HWS_IPC_LOW_PRI_QUEUE,
            (GT_U8*)&msgData,
            msgData.msgLength + IPC_OFFSETOF(MV_HWS_IPC_CTRL_MSG_STRUCT,msgData.noop));
    if (rc != GT_OK)
    {
        return GT_FALSE;
    }

    /* receive msg */
    cpssOsTimerWkAfter(100);
    rc = prvCpssGenericSrvCpuIpcMessageRecv(fwChannel, MV_HWS_IPC_TX_0_CH_ID, (GT_U8*)&replyData, NULL);
    if (rc != GT_OK)
    {
#if 1
        cpssOsPrintf (" wrong noop data:\n",replyData.readData.noopReply.data);
#endif
        return GT_FALSE;
    }
    if (CPSS_32BIT_LE(replyData.replyTo) != (GT_U32)MV_HWS_IPC_NOOP_MSG)
    {
#if 1
        cpssOsPrintf (" wrong msg ID %d Expected %d queue ID %d\n",replyData.replyTo, MV_HWS_IPC_NOOP_MSG,MV_HWS_IPC_TX_0_CH_ID);
#endif
        return GT_FALSE;
    }
        /* The expected data == 5 */
    if (CPSS_32BIT_LE(replyData.readData.noopReply.data) != 5)
    {
#if 1
        cpssOsPrintf (" wrong noop data:\n",replyData.readData.noopReply.data);
#endif
        return GT_FALSE;
    }
    return GT_TRUE;
}

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
GT_STATUS mvHwsServiceCpuFwInit
(
    GT_U8 devNum,
    GT_U32 scpuId,
    const char *fwId
)
{
    GT_STATUS rc;
    CPSS_SYSTEM_RECOVERY_INFO_STC system_recovery; /* holds system recovery information */
    GT_UINTPTR fwChannel = 0;
    GT_U32 smemData, devId, revId;
    MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;

    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);
    rc = hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].deviceInfoFunc(devNum, &devId, &revId);
    if (rc != GT_OK)
        return rc;
    /* 32Bits of data represent 28Bits hws device type enum and 4Bits device revision id */
    smemData = ( (hwsDeviceSpecInfo[devNum].devType & 0xFFFFFFF) | ( (revId & 0xF) << 28) );

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
      return rc;
    }
    if (system_recovery.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
    {
        /* TODO: check cb */
        rc = prvCpssGenericSrvCpuProbe(devNum, scpuId, GT_TRUE,
                prvMvHwsCheckAPIPC, (GT_UINTPTR)devNum, &fwChannel);
        if (rc == GT_NO_SUCH)
            return rc;
        if (rc != GT_OK)
            fwChannel = 0;
    }
    if (fwChannel == 0)
    {
        PRV_CPSS_GENERIC_SRV_CPU_IPC_CHANNEL_CONFIG_STC ipcCfg[] = {
            { 0,   0,  0, 4, 64 },
            { 1,   0,  0, 4, 64 },
            { 2,   4, 64, 0,  0 },
            { 3,   4, 64, 0,  0 },
            { 4,   4, 64, 0,  0 },
            { 5,   4, 64, 0,  0 },
            { 0xff,0,  0, 0,  0 }
        };
        rc = prvCpssGenericSrvCpuInit(devNum, scpuId, fwId, ipcCfg, prvMvHwsPreStartAP, (GT_UINTPTR)smemData, &fwChannel);
        if (rc != GT_OK && rc != GT_ALREADY_EXIST)
            return rc;
    }
    hwsIpcChannelHandlers[devNum] = fwChannel;

    if (rc == GT_ALREADY_EXIST)
        return GT_OK;

    mvHwsIpcDbInit(devNum);

    return GT_OK;
}

/**
* @internal mvHwsServiceCpuFwPostInit function
* @endinternal
*
* @brief   Init HWS Firmware After Avago Serdes init
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsServiceCpuFwPostInit(GT_U8 devNum)
{
    GT_U32       serviceCpuAvagoCfgAddr;
    GT_U32       serviceCpuAvagoCfgSize;
    unsigned int *hostAvagoCfgAddr = 0;
#ifndef ASIC_SIMULATION
    unsigned int hostAvagoCfgSize = 0;
#endif
    MV_HWS_FW_LOG apDebug = {0,0,0,0,0};
    MV_HWS_FW_LOG hwsLog = {0,0,0,0,0};
#if !defined(MV_HWS_AVAGO_NO_VOS_WA) && !defined(ASIC_SIMULATION)
    GT_U32 tempVosParams32BitPtr[64];
    GT_U32 tempVosParams64BitPtr[72*2];
    GT_U32 tempVosParamsPtrSize;
    GT_U32 vosParamsBuffersDevision;
    GT_U32 vosParamsBuffersDevisionIdx;
    GT_U32 ipcVosParamsBufferSize;
#endif /* !defined(MV_HWS_AVAGO_NO_VOS_WA) && !defined(ASIC_SIMULATION) */
    GT_STATUS rc;
    CPSS_SYSTEM_RECOVERY_INFO_STC system_recovery; /* holds system recovery information */

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
      return rc;
    }

    if (devNum >= HWS_MAX_DEVICE_NUM)
    {
        return GT_BAD_PARAM;
    }
    if (system_recovery.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
    {
        if (((hwsDeviceSpecInfo[devNum].devType == BobK) || (hwsDeviceSpecInfo[devNum].devType == Aldrin)
             || (hwsDeviceSpecInfo[devNum].devType == Bobcat3) || (hwsDeviceSpecInfo[devNum].devType == Pipe) || (hwsDeviceSpecInfo[devNum].devType == Aldrin2)) &&
            (mvHwsServiceCpuEnableGet(devNum)))
        {
            /* In BOBK-CM3 the Host is responsible to load Avago Firmware and init AAPL structure
            ** Once initialized the Host should load Host AAPL structure instance to the CM3 AAPL
            ** structure instance
            ** First step is to get CM3 AAPL structure base address in CM3 'firmware section"
            ** Second step is to write into the CM3 'firmware section" AAPL structure base address
            ** the Host AAPL structure contect
            */
            if (mvHwsPortAvagoCfgAddrGetIpc(devNum, 0, 0, &serviceCpuAvagoCfgAddr, &serviceCpuAvagoCfgSize) == GT_OK)
            {
    #ifndef ASIC_SIMULATION
                mvHwsAvagoAaplAddrGet(devNum, &hostAvagoCfgAddr, &hostAvagoCfgSize);
    #endif
#if defined(CPU_BE)
                /* Convert data under hostAvagoCfgAddr to LE */
                {
                    GT_U32 i;
                    GT_UINTPTR s = (GT_UINTPTR)hostAvagoCfgAddr;
                    GT_UINTPTR fwChannel = hwsIpcChannelHandlers[devNum];
                    for (i = 0; i < serviceCpuAvagoCfgSize; i+=4 )
                    {
                        int wordSize = 4;
#define OFFSET_AAPL(_member) ((GT_U32)((GT_UINTPTR)(&(((Aapl_t*)NULL)->_member))))
                        if (i >= OFFSET_AAPL(ip_rev[0][0][0])
                                && i < OFFSET_AAPL(spico_running[0][0][0]))
                            wordSize = 2;
                        if (i >= OFFSET_AAPL(spico_running[0][0][0])
                                && i < OFFSET_AAPL(return_code))
                            wordSize = 1;
                        if (wordSize == 4)
                        {
                            GT_U32 u32[1];
                            u32[0] = CPSS_32BIT_LE(*((GT_U32*)(s+i)));
                            prvCpssGenericSrvCpuWrite(fwChannel, serviceCpuAvagoCfgAddr+i,u32,4);
                        }
#undef OFFSET_AAPL
                        if (wordSize == 2)
                        {
                            GT_U16 u16[2];
                            u16[0] = CPSS_16BIT_LE(*((GT_U16*)(s+i)));
                            u16[1] = CPSS_16BIT_LE(*((GT_U16*)(s+i+2)));
                            prvCpssGenericSrvCpuWrite(fwChannel, serviceCpuAvagoCfgAddr+i,u16,4);
                        }
                        if (wordSize == 1)
                        {
                            GT_U8  u8[4];
                            u8[0] = *((GT_U8*)(s+i));
                            u8[1] = *((GT_U8*)(s+i+1));
                            u8[2] = *((GT_U8*)(s+i+2));
                            u8[3] = *((GT_U8*)(s+i+3));
                            prvCpssGenericSrvCpuWrite(fwChannel, serviceCpuAvagoCfgAddr+i,u8,4);
                        }
                    }
                }
#else
                prvCpssGenericSrvCpuWrite(hwsIpcChannelHandlers[devNum], serviceCpuAvagoCfgAddr, hostAvagoCfgAddr, serviceCpuAvagoCfgSize);
#endif
                if (mvHwsPortAvagoAaplInitIpc(devNum, 0, 0) != GT_OK)
                {
                    hwsOsPrintf("mvHwsServiceCpuFwPostInit Failed to init Avago AAPL\n");
                    return GT_FAIL;
                }
            }
            else
            {
                hwsOsPrintf("mvHwsServiceCpuFwPostInit Failed to load Avago AAPL\n");
                return GT_FAIL;
            }
        }
    }

    /* In order provide access to Service CPU real-time log (AP and HWS)
    ** The Host is required to receive the LOGs parameters addresses.
    ** Once received he, the LOGs can be access from Host
    */
    /* Service CPU AP Log */
    if (mvHwsApPortCtrlDebugGet(devNum, 0, 0, &apDebug) == GT_OK)
    {
        mvHwsApPortCtrlDebugParamsSet(apDebug.fwLogBaseAddr,
                                      apDebug.fwLogCountAddr,
                                      apDebug.fwLogPointerAddr,
                                      apDebug.fwLogResetAddr);
    }
    else
    {
         hwsOsPrintf("mvHwsServiceCpuFwPostInit Failed to retrive FW AP LOG parameters\n");
         return GT_FAIL;
    }
    /* Service CPU Hws Log */
    if (mvHwsFwHwsLogGet(devNum, &hwsLog) == GT_OK)
    {
        mvHwsLogParamsSet(hwsLog.fwLogBaseAddr,
                          hwsLog.fwLogCountAddr,
                          hwsLog.fwLogPointerAddr,
                          hwsLog.fwLogResetAddr);
    }
    else
    {
         hwsOsPrintf("mvHwsServiceCpuFwPostInit Failed to retrive FW HWS LOG parameters\n");
         return GT_FAIL;
    }
#if !defined(MV_HWS_AVAGO_NO_VOS_WA) && !defined(ASIC_SIMULATION)
    if (hwsDeviceSpecInfo[devNum].serdesInfo.serdesVosOverride == GT_TRUE)
    {
        /* Send vos override params- reading the values, dividing them to smaller buffers to
           fit IPC message size limit then send each buffer with its index and size. */

        /* number of elements in IPC vos buffer (8) */
        ipcVosParamsBufferSize = sizeof( ((MV_HWS_IPC_VOS_OVERRIDE_PARAMS *)0)->vosOverrideParamsBufferPtr ) / sizeof(GT_U32);

        if ((hwsDeviceSpecInfo[devNum].devType == Bobcat3) || (hwsDeviceSpecInfo[devNum].devType == Pipe) ||
                (hwsDeviceSpecInfo[devNum].devType == Aldrin2))
        {
            /* number of elements in the avago db per-serdes array */
            tempVosParamsPtrSize = sizeof(tempVosParams64BitPtr) / sizeof(GT_U32);
            hwsOsMemSetFuncPtr(&tempVosParams64BitPtr[0], 0, sizeof(tempVosParams64BitPtr));

            if (mvHwsAvagoSerdesVosParamsGet(devNum, &tempVosParams64BitPtr[0]) != GT_OK )
            {
                hwsOsPrintf("mvHwsServiceCpuFwPostInit Failed to get VOS Override DB \n");
                return GT_FAIL;
            }
        }
        else
        {
            /* number of elements in the avago db per-serdes array */
            tempVosParamsPtrSize = sizeof(tempVosParams32BitPtr) / sizeof(GT_U32);
            hwsOsMemSetFuncPtr(&tempVosParams32BitPtr[0], 0, sizeof(tempVosParams32BitPtr));

            if (mvHwsAvagoSerdesVosParamsGet(devNum, &tempVosParams32BitPtr[0]) != GT_OK )
            {
                hwsOsPrintf("mvHwsServiceCpuFwPostInit Failed to get VOS Override DB \n");
                return GT_FAIL;
            }
        }

        /* devide avago db per-serdes array to chunks of size as number of elements in IPC vos buffer (8)*/
        if ((hwsDeviceSpecInfo[devNum].devType == Pipe))
        {
            /* for Pipe we have 4 chunks of vos buffer size (8)*/
            vosParamsBuffersDevision = sizeof(GT_U32);
        }
        else
        {
            vosParamsBuffersDevision = tempVosParamsPtrSize / ipcVosParamsBufferSize;
        }

        for ( vosParamsBuffersDevisionIdx = 0;
              vosParamsBuffersDevisionIdx < vosParamsBuffersDevision;
              vosParamsBuffersDevisionIdx++)
        {
            if ((hwsDeviceSpecInfo[devNum].devType == Bobcat3) || (hwsDeviceSpecInfo[devNum].devType == Pipe) ||
                (hwsDeviceSpecInfo[devNum].devType == Aldrin2))
            {
                if (!serdesVos64BitDbInitialized)
                {
                    /* AVG block was not found, therefore there is no need to do VOS override */
                    break;
                }
                else if(mvHwsPortVosOVerrideParamsSetIpc(devNum, &tempVosParams64BitPtr[(vosParamsBuffersDevisionIdx*ipcVosParamsBufferSize)],
                                                      vosParamsBuffersDevisionIdx, ipcVosParamsBufferSize) != GT_OK )
                {
                    hwsOsPrintf("mvHwsServiceCpuFwPostInit Failed to send VOS OVERRIDE parameters\n");
                    return GT_FAIL;
                }
            }
            else
            {
                if ( mvHwsPortVosOVerrideParamsSetIpc(devNum, &tempVosParams32BitPtr[(vosParamsBuffersDevisionIdx*ipcVosParamsBufferSize)],
                                                      vosParamsBuffersDevisionIdx, ipcVosParamsBufferSize) != GT_OK )
                {
                    hwsOsPrintf("mvHwsServiceCpuFwPostInit Failed to send VOS OVERRIDE parameters\n");
                    return GT_FAIL;
                }
            }
        }

        if ((hwsDeviceSpecInfo[devNum].devType == Bobcat3) || (hwsDeviceSpecInfo[devNum].devType == Pipe) ||
                (hwsDeviceSpecInfo[devNum].devType == Aldrin2))
        {
            tempVosParams64BitPtr[0] = minVosValue;
            tempVosParams64BitPtr[1] = baseVosValue;
            if (serdesVos64BitDbInitialized)
            {
                if (mvHwsPortVosOVerrideParamsSetIpc(devNum, &tempVosParams64BitPtr[0],
                                                      0xFFFFFFFF, 2) != GT_OK )
                {
                    hwsOsPrintf("mvHwsServiceCpuFwPostInit Failed to send VOS OVERRIDE min and base parameters\n");
                    return GT_FAIL;
                }
            }
        }
    }
#endif /*#if !defined(MV_HWS_AVAGO_NO_VOS_WA) && !defined(ASIC_SIMULATION) */
    return GT_OK;
}

/**
* @internal mvHwsServiceCpuEnableNfsLoad function
* @endinternal
*
* @brief   Debug function which Enables FW loading from NFS
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsServiceCpuEnableNfsLoad(GT_BOOL enable, GT_U32 fwSize)
{
    hwsOsPrintf("%s FW load from NFS\n", (enable) ? "Enable" : "Disable");
    loadFromNfs = enable;
    nfsFwSize = fwSize;

    return GT_OK;
}


