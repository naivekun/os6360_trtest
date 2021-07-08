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
* @file prvCpssGenericSrvCpuLoad.c
*
* @brief Service CPU firmware load/start APIs
*
* @version   1
********************************************************************************
*/
#include <cpss/common/srvCpu/prvCpssGenericSrvCpuLoad.h>
#include <cpss/generic/ipc/mvShmIpc.h>
#include "prvCpssGenericSrvCpuIpcDevCfg.h"


#ifndef ASIC_SIMULATION
PRV_SRVCPU_IPC_CHANNEL_STC* prvIpcDevCfg[PRV_CPSS_MAX_PP_DEVICES_CNS];

struct PRV_CPSS_SRVCPU_TYPES_STCT {
    GT_U16      devId;
    GT_U16      devIdMask;
    PRV_SRVCPU_OPS_FUNC ops;
} prvSrvCpuDescr[] = {
    { 0xfc00, 0xff00, &prvSrvCpuOpsMsys }, /* bobcat2 */
    { 0xbe00, 0xff00, &prvSrvCpuOpsBobK }, /* cetus */
    { 0xbc00, 0xff00, &prvSrvCpuOpsBobK }, /* caelum */
    { 0xf400, 0xfe00, &prvSrvCpuOpsMsys }, /* alleycat3\s */
    { 0xc800, 0xff00, &prvSrvCpuOpsCM3  }, /* aldrin */
    { 0xd400, 0xff00, &prvSrvCpuOpsCM3  }, /* bc3 */
    { 0xc400, 0xff00, &prvSrvCpuOpsCM3  }, /* pipe */
    { 0xcc00, 0xff00, &prvSrvCpuOpsCM3  }, /* aldrin2 */
    { 0xffff, 0xffff, NULL }
};

static PRV_SRVCPU_OPS_FUNC prvSrvCpuGetOps(
    IN  CPSS_HW_INFO_STC    *hwInfo,
    IN  CPSS_HW_DRIVER_STC  *drv
)
{
    GT_U32 i;
    GT_U16 deviceId; /* PCI device Id */
    GT_STATUS rc;

    if (hwInfo->resource.switching.start == 0 && drv == NULL)
        return NULL;
    if (drv != NULL)
    {
        rc = drv->read(drv, CPSS_HW_DRIVER_AS_SWITCHING_E, 0x4c, &i, 1);
        if (rc != GT_OK)
            return NULL;
    }
    else
    {
        i = CPSS_32BIT_BE(*((volatile GT_U32*)(hwInfo->resource.switching.start + 0x4c)));
    }
    deviceId = (i >> 4) & 0xffff;
    for (i = 0; prvSrvCpuDescr[i].ops; i++)
    {
        if ((deviceId & prvSrvCpuDescr[i].devIdMask) == prvSrvCpuDescr[i].devId)
            break;
    }
    return prvSrvCpuDescr[i].ops;
}
#endif

#ifndef ASIC_SIMULATION

#define REMOTE_UPGRADE_HEADER_SIZE 0x0C /* 12  */
#define REMOTE_UPGRADE_MSG_MIN_LEN 0x4  /* 4  */
#define REMOTE_UPGRADE_MSG_MAX_LEN 0xF8 /* 248 */

/* defined in boot_channel.h */
typedef enum {  MICRO_INIT_BOOT_CH_FW_REMOTE_UPGRADE_ERR_NOR_FLASH_NOT_EXIST_E   = 0x012,
                MICRO_INIT_BOOT_CH_FW_REMOTE_UPGRADE_ERR_PRIMARY_FW_HEADER_TYPE_E,
                MICRO_INIT_BOOT_CH_FW_REMOTE_UPGRADE_ERR_CURR_PRIMARY_FW_INVALID_CRC_E,
                MICRO_INIT_BOOT_CH_FW_REMOTE_UPGRADE_ERR_CURR_SECONDARY_FW_INVALID_CRC_E,
                MICRO_INIT_BOOT_CH_FW_REMOTE_UPGRADE_ERR_NEW_FW_INVALID_CRC_E,
                MICRO_INIT_BOOT_CH_FW_REMOTE_UPGRADE_ERR_INVALID_CHECKSUM_E
} MICRO_INIT_BOOT_CH_FW_REMOTE_UPGRADE_ERR_ENT;

typedef struct { /* remote upgrade message header */
/*  type        name              byte order */
    GT_U16      opCode;       /*   0  - 1    */
    GT_U16      bmp;          /*   2  - 3    */
    GT_U32      bufNum;       /*   4  - 7    */
    GT_U16      bufSize;      /*   8  - 9    */
    GT_U8       checkSum;     /*   10        */
    GT_U8       reseved;      /*   11        */
} upgradeMsgHdr;

/* 8 bit checksum */
static GT_U8 prvChecksum8(GT_U8* start, GT_U32 len, GT_U8 csum)
{
    GT_U8  sum = csum;
    GT_U8* startp = start;

    do {
        sum += *startp;
        startp++;

    } while(--len);

    return (sum);
}
#endif

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
)
{
#ifdef ASIC_SIMULATION
    (void)hwInfo;
    (void)drv;
    (void)scpuId;
    (void)fwId;
    (void)ipcConfig;
    (void)preStartCb;
    (void)preStartCookie;
    (void)fwChannelPtr;
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
#else
    PRV_SRVCPU_OPS_FUNC opsFunc;
    PRV_CPSS_FIRMWARE_IMAGE_FILE_STC *f;
    PRV_SRVCPU_IPC_CHANNEL_STC *s;
    GT_U32    p;
    GT_STATUS rc;

    opsFunc = prvSrvCpuGetOps(hwInfo, drv);
    if (opsFunc == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    rc = opsFunc(hwInfo, drv, scpuId, PRV_SRVCPU_OPS_EXISTS_E, NULL);
    if (rc != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_SUCH, LOG_ERROR_NO_MSG);

    f = prvCpssSrvCpuFirmareOpen(fwId);
    if (f == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_SUCH, LOG_ERROR_NO_MSG);

    rc = opsFunc(hwInfo, drv, scpuId, PRV_SRVCPU_OPS_RESET_E, NULL);
    if (rc != GT_OK)
    {
        prvCpssSrvCpuFirmareClose(f);
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    rc = opsFunc(hwInfo, drv, scpuId, PRV_SRVCPU_OPS_PRE_LOAD_E, &s);
    if (rc != GT_OK)
    {
        prvCpssSrvCpuFirmareClose(f);
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    for (p = 0; p < f->fwSize;)
    {
        GT_U32 buf[128];
        GT_U32 k;
        k = prvCpssSrvCpuFirmareRead(f, buf, sizeof(buf));
        if (k == 0xffffffff)
            break;
        s->drv->writeMask(s->drv, 0, f->fwLoadAddr+p, buf, k/4, 0xffffffff);
        p += k;
    }

    if (p < f->fwSize)
    {
        /* failure reading firmware */
        prvCpssSrvCpuFirmareClose(f);
        prvIpcAccessDestroy(s);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    s->shmAddr = f->ipcShmAddr;
    s->shmSize = f->ipcShmSize;
    prvCpssSrvCpuFirmareClose(f);

    if (ipcConfig != NULL)
    {
        /* setup ipc... */
        rc = prvIpcAccessInit(s, GT_TRUE);
        for (; ipcConfig->channel != 0xff; ipcConfig++)
        {
            shmIpcConfigChannel(&(s->shm), ipcConfig->channel,
                    ipcConfig->maxrx, ipcConfig->rxsize,
                    ipcConfig->maxtx, ipcConfig->txsize);
        }
        /* create fwChannel object */

        /* Write magick to reserved area */
        shmResvdAreaRegWrite(&(s->shm), s->shmResvdRegs, 0, IPC_RESVD_MAGIC);
        /* Write pointer to SHM registers to reserved area */
        shmResvdAreaRegWrite(&(s->shm), s->shmResvdRegs, 1, s->shmAddr + 0x100);
    }
    if (preStartCb != NULL)
        preStartCb((GT_UINTPTR)s, preStartCookie);
    if (fwChannelPtr != NULL)
    {
        *fwChannelPtr = (GT_UINTPTR)s;
    }
    else
    {
        prvIpcAccessDestroy(s);
    }

    rc = opsFunc(hwInfo, drv, scpuId, PRV_SRVCPU_OPS_UNRESET_E, NULL);
    return rc;
#endif
}

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
)
{
#ifndef ASIC_SIMULATION
    CPSS_HW_INFO_STC *hwInfo;
    CPSS_HW_DRIVER_STC *drv;
    GT_STATUS rc;

    if (devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    if (prvCpssDrvPpConfig[devNum] == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    if (prvIpcDevCfg[devNum] != NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, LOG_ERROR_NO_MSG);

    hwInfo = &(prvCpssDrvPpConfig[devNum]->hwInfo[0]);
    drv = prvCpssDrvHwPpPortGroupGetDrv(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS);
    rc = prvCpssGenericSrvCpuInitCust(hwInfo, drv, scpuId, fwId, ipcConfig, preStartCb, preStartCookie, fwChannelPtr);

    /* store fwChannel */
    if (fwChannelPtr != NULL)
        prvIpcDevCfg[devNum] = (PRV_SRVCPU_IPC_CHANNEL_STC*)(*fwChannelPtr);

    return rc;
#else
    (void)devNum;
    (void)scpuId;
    (void)fwId;
    (void)ipcConfig;
    (void)preStartCb;
    (void)preStartCookie;
    (void)fwChannelPtr;
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
#endif
}

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
)
{
#ifdef ASIC_SIMULATION
    (void)hwInfo;
    (void)drv;
    (void)scpuId;
    (void)hasIPC;
    (void)checkIpcCb;
    (void)checkIpcCookie;
    (void)fwChannelPtr;
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
#else
    PRV_SRVCPU_OPS_FUNC opsFunc;
    GT_BOOL isOn;
    GT_STATUS rc;
    PRV_SRVCPU_IPC_CHANNEL_STC *s;

    if (fwChannelPtr != NULL)
        *fwChannelPtr = 0;
    opsFunc = prvSrvCpuGetOps(hwInfo, drv);
    if (opsFunc == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    rc = opsFunc(hwInfo, drv, scpuId, PRV_SRVCPU_OPS_EXISTS_E, NULL);
    if (rc != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_SUCH, LOG_ERROR_NO_MSG);
    rc = opsFunc(hwInfo, drv, scpuId, PRV_SRVCPU_OPS_IS_ON_E, &isOn);
    if (rc != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_SUCH, LOG_ERROR_NO_MSG);
    if (isOn != GT_TRUE)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    rc = opsFunc(hwInfo, drv, scpuId, PRV_SRVCPU_OPS_PRE_CHECK_IPC_E, &s);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    if (hasIPC == GT_FALSE)
    {
        /* No IPC check, return OK */
        if (fwChannelPtr != NULL)
        {
            *fwChannelPtr = (GT_UINTPTR)s;
        }
        else
        {
            prvIpcAccessDestroy(s);
        }
        return GT_OK;
    }
    s->shmSize = 0x800; /* handled later */
    rc = prvIpcAccessInit(s, GT_FALSE);
    if (rc != GT_OK)
    {
        prvIpcAccessDestroy(s);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }
    /* Check magick in reserved area */
    if (shmResvdAreaRegRead(&(s->shm), s->shmResvdRegs, 0) != IPC_RESVD_MAGIC)
    {
        prvIpcAccessDestroy(s);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }
    s->shmAddr = shmResvdAreaRegRead(&(s->shm), s->shmResvdRegs, 1) - 0x100;
    if (s->shmAddr < s->targetBase || s->shmAddr >= s->targetBase+(s->size-4))
    {
        prvIpcAccessDestroy(s);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }
    s->shm.shm = s->shmAddr;
    s->shm.shmLen = s->shmSize;
    /* check if ipc link was initialized */
    if ( (shmIpcRegRead(&(s->shm), IPC_SHM_NUM_REGS) != IPC_SHM_MAGIC)  &&
         (shmIpcRegRead(&(s->shm), IPC_SHM_NUM_REGS) != IPC_SHM_MAGIC+1) )
    {
        prvIpcAccessDestroy(s);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }
    /* fix mirror buffer */
    s->shm.shmLen = shmIpcRegRead(&(s->shm), IPC_SHM_NUM_REGS+1);
    if (s->shmSize < s->shm.shmLen && s->base == 0)
    {
        s->shmSize = s->shm.shmLen;
    }
    /* check ipc transaction */
    if (checkIpcCb != NULL)
    {
        if (checkIpcCb((GT_UINTPTR)s, checkIpcCookie) != GT_TRUE)
        {
            prvIpcAccessDestroy(s);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
        }
    }
    if (fwChannelPtr != NULL)
    {
        *fwChannelPtr = (GT_UINTPTR)s;
    }
    else
    {
        prvIpcAccessDestroy(s);
    }

    return GT_OK;
#endif
}


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
)
{
#ifdef ASIC_SIMULATION
    (void)devNum;
    (void)scpuId;
    (void)hasIPC;
    (void)checkIpcCb;
    (void)checkIpcCookie;
    (void)fwChannelPtr;
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
#else
    CPSS_HW_INFO_STC *hwInfo;
    CPSS_HW_DRIVER_STC *drv;
    GT_STATUS rc;

    if (devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    if (prvCpssDrvPpConfig[devNum] == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    if (prvIpcDevCfg[devNum] != NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, LOG_ERROR_NO_MSG);

    hwInfo = &(prvCpssDrvPpConfig[devNum]->hwInfo[0]);
    drv = prvCpssDrvHwPpPortGroupGetDrv(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS);
    rc = prvCpssGenericSrvCpuProbeCust(hwInfo, drv, scpuId,
            hasIPC, checkIpcCb, checkIpcCookie, fwChannelPtr);

    /* store fwChannel */
    if (rc == GT_OK)
    {
        if (fwChannelPtr != NULL)
            prvIpcDevCfg[devNum] = (PRV_SRVCPU_IPC_CHANNEL_STC*)(*fwChannelPtr);
    }
    return rc;

#endif
}





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
)
{
#ifndef ASIC_SIMULATION
    return prvCpssGenericSrvCpuInit(devNum, 1, "DBA_Bobcat3", NULL, NULL, 0, NULL);
#else
    (void)devNum;
    return GT_OK;
#endif
}

/**
* @internal prvCpssTamToSrvCpuLoad function
* @endinternal
*
* @brief   Upload TAM FW to second service CPU
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2;
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
GT_STATUS prvCpssTamToSrvCpuLoad
(
    IN GT_U8    devNum
)
{
#ifndef ASIC_SIMULATION
    return prvCpssGenericSrvCpuInit(devNum, 1, "TAM_Bobcat3", NULL, NULL, 0, NULL);
#else
    (void)devNum;
    return GT_OK;
#endif
}

/*******************************************************************************
* prvCpssGenericSrvCpuRemove
*
* DESCRIPTION:
*       Cleanup service CPU resources while cpssDxChCfgDevRemove
*
* APPLICABLE DEVICES:
*       AC3; Bobcat2; Caelum; Cetus; Aldrin; Bobcat3; Pipe.
*
* NOT APPLICABLE DEVICES:
*       Lion2.
*
* INPUTS:
*       devNum                   - device number.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_FAIL                  - on error.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssGenericSrvCpuRemove
(
    IN GT_U8    devNum
)
{
#ifndef ASIC_SIMULATION
    if (devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    if (prvIpcDevCfg[devNum] != NULL)
    {
        prvIpcAccessDestroy(prvIpcDevCfg[devNum]);
        prvIpcDevCfg[devNum] = NULL;
    }
#else /* defined(ASIC_SIMULATION) */
    (void)devNum;
#endif
    return GT_OK;
}




/*******************************************************************************
* prvCpssGenericSrvCpuReNumberDevNum
*
* DESCRIPTION:
*       Replace the ID of a device from old device number
*       to a new device number
*
* INPUTS:
*       oldDevNum  - old device num
*       newDevNum  - new device num
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_FAIL                  - on error.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssGenericSrvCpuReNumberDevNum
(
    IN  GT_U8   oldDevNum,
    IN  GT_U8   newDevNum
)
{
#ifndef ASIC_SIMULATION
    if (oldDevNum >= PRV_CPSS_MAX_PP_DEVICES_CNS
        || newDevNum >= PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (newDevNum == oldDevNum)
        return GT_OK;
    if (prvIpcDevCfg[newDevNum] != NULL)
    {
        prvIpcAccessDestroy(prvIpcDevCfg[newDevNum]);
    }
    prvIpcDevCfg[newDevNum] = prvIpcDevCfg[oldDevNum];
    prvIpcDevCfg[oldDevNum] = NULL;
#else
    (void)oldDevNum;
    (void)newDevNum;
#endif
    return GT_OK;
}

/**
* @internal prvCpssGenericSrvCpuFWUpgrade function
* @endinternal
*
* @brief   Send buffer with FW upgrade to service CPU,
*         to be programmed on flash, using boot channel.
*         buffer size is limited to 240 bytes.
*
* @note   APPLICABLE DEVICES:      AC3; Bobcat2; Caelum; Cetus; Aldrin; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number,
* @param[in] bufBmp                   - bitmap of the FW:
*                                      0x01 - primary SuperImage
*                                      0x02 - secondary SuperImage,
*                                      0x03 - both.
* @param[in] bufNum                   - block number,
* @param[in] bufSize                  - buffer size:
*                                      must be: 32B < fwBufSize < 240B,
*                                      last bufSize=bufSize|0x100)
* @param[in] bufPtr                   - FW data ptr,
*
* @param[out] status                   - microInit boot channel retCode.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvCpssGenericSrvCpuFWUpgrade
(
    IN  GT_U8   devNum,
    IN  GT_U32  bufBmp,
    IN  GT_U32  bufNum,
    IN  GT_U32  bufSize,
    IN  GT_U8  *bufPtr,
    OUT GT_U32 *status
)
{

#ifndef ASIC_SIMULATION
    PRV_SRVCPU_IPC_CHANNEL_STC      *s;
    GT_32                           rc = 0, bc_rc = 0;
    GT_U32                          i, j, msgSize;
    GT_U32                          maxDelay;
    GT_U32                          bufWithHeader[256];
    GT_U32                          dummyBuf;
    upgradeMsgHdr                   *msgHeader = NULL;

    cpssOsPrintf("bufNum=%d bufSize=0x%X\n",bufNum, bufSize );

    msgSize = bufSize & 0xFF;
    if( (msgSize<REMOTE_UPGRADE_MSG_MIN_LEN) || (msgSize>REMOTE_UPGRADE_MSG_MAX_LEN) ) {
        cpssOsPrintf("buffer size must be %d < X < %d \n", REMOTE_UPGRADE_MSG_MIN_LEN, REMOTE_UPGRADE_MSG_MAX_LEN);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    if (devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    s = prvIpcDevCfg[devNum];
    if (s == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);

    /* prepare command parameters */
    msgHeader = (upgradeMsgHdr *)&bufWithHeader[0];
    msgHeader->opCode  = 0x10;          /* remote upgrade opCode */
    msgHeader->bmp     = bufBmp & 0xFF;
    msgHeader->bufNum  = bufNum;
    msgHeader->bufSize = (GT_U16)bufSize;
    msgHeader->checkSum = 0;
    msgHeader->reseved  = 0;
    cpssOsMemCpy( (msgHeader + 1), bufPtr, msgSize );

    msgSize = msgSize + REMOTE_UPGRADE_HEADER_SIZE;
    /* calculate checksum and update packet header */
    msgHeader->checkSum = prvChecksum8((GT_U8*)msgHeader,msgSize,0);

    i = 0;
    /* wait reply status about 2 sec for regular packets
         and 30 sec for finalize procedure of bmp #3 (crc validation and copy image to other offset) */
    if ((bufSize | 0x100) && (bufBmp ==3)) maxDelay = 30000;
    else maxDelay = 2000;

    for ( j=0 ; j < 3; j++ ) { /* 3 retries for packet with wrong checkSum */
        if (GT_OK != (rc = shmIpcBootChnSend(&(s->shm), 0, msgHeader, msgSize) ) ){
            if (j >= 3){
                cpssOsPrintf("shmIpcBootChnSend failed %d\n", rc);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
            continue;
        }

        do
        {
            cpssOsTimerWkAfter(1);

            i++;
            /* for each boot channel send operation, feedback receive is necessary.
               if not expected any valuable feedback data - dummy buffer must be used */
            rc=shmIpcBootChnRecv(&s->shm, &bc_rc, &dummyBuf, 4);
            if (rc > 0)
                break;
            if (i > maxDelay)
            {
                cpssOsPrintf("\n\nTimeOut error\n\n");
                *status = bc_rc;
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
        } while(i <= maxDelay);

        /* only in case of invalid checksum - send message again */
        if( bc_rc != MICRO_INIT_BOOT_CH_FW_REMOTE_UPGRADE_ERR_INVALID_CHECKSUM_E )
            break;
    }
    *status = bc_rc;

    return (rc>0) ? bc_rc : GT_FAIL;


#else /* defined(ASIC_SIMULATION) */

    if (bufPtr) {
        devNum  = devNum;
        bufBmp  = bufBmp;
        bufNum  = bufNum;
        bufSize = bufSize;
    };
    *status = 4;
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
#endif /* defined(ASIC_SIMULATION) */
}



















