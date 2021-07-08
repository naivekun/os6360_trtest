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
* @brief Service CPU FW upload implementation
*
* @version   1
********************************************************************************
*/
#include <cpss/common/srvCpu/prvCpssGenericSrvCpuLoad.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpssDriver/pp/hardware/cpssDriverPpHw.h>
#include "prvCpssGenericSrvCpuIpcDevCfg.h"
#include "cpss/generic/ipc/mvShmIpc.h"
#include "string.h"

PRV_CPSS_IPC_DEV_CFG_STC *prvIpcDevCfg[PRV_CPSS_MAX_PP_DEVICES_CNS];

#ifndef ASIC_SIMULATION


extern GT_STATUS osTimerWkAfter(IN GT_U32 mils);

/* rename to avoid */
#define mvHwsServiceCpuBc2Fw _prv_mvHwsServiceCpuBc2Fw
#define mvHwsServiceCpuBobkFw _prv_mvHwsServiceCpuBobkFw
#define mvHwsServiceCpuCm3Fw _prv_mvHwsServiceCpuCm3Fw
#define mvHwsServiceCpuCm3AldrinFw _prv_mvHwsServiceCpuCm3AldrinFw
#define mvHwsServiceCpuCm3Bc3Fw _prv_mvHwsServiceCpuCm3Bc3Fw
#define mvHwsServiceCpuCm3PipeFw _prv_mvHwsServiceCpuCm3PipeFw
#define mvHwsServiceCpuCm3Aldrin2Fw _prv_mvHwsServiceCpuCm3Aldrin2Fw
#include "mvHwsServiceCpuBc2Fw.h"
#include "mvHwsServiceCpuBobkFw.h"
#include "mvHwsServiceCpuCm3BobKFw.h"
#include "mvHwsServiceCpuCm3AldrinFw.h"
#include "mvHwsServiceCpuCm3Bc3Fw.h"
#include "mvHwsServiceCpuCm3PipeFw.h"
#include "mvHwsServiceCpuCm3Aldrin2Fw.h"

#include "bc3DbaFw.h"
#include "bc3TamFw.h"

#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsIpcDefs.h>

static void mv_configure_pex_window_sram(
    GT_UINTPTR regsBase,
    GT_U32     sramPhys,
    GT_U32     sramSize
);
static GT_STATUS mv_configure_sram(
    GT_UINTPTR regsBase,
    GT_U32     sramPhys,
    GT_U32     sramSize
);

typedef struct PRV_CPSS_FIRMWARE_CONFIG_STCT {
    const char *name;
    GT_U32      sramPhys;   /* MSYS/CM3 address */
    GT_U32      sramSize;
    GT_U32      ipcShmRegs; /* MSYS/CM3 address */
    GT_U32      fwLoadAddr; /* MSYS/CM3 address */
    GT_U8      *fwData;
    GT_U32      fwSize;     /* actual (uncompressed) size */
    GT_U32      fwArrSize;  /* size of buffer */
    GT_U32      compress;   /* 0 - no compression, 2 - simple compression */
    GT_U32      ipcShmAddr; /* MSYS address */
    GT_U32      ipcShmSize; /* Size of shm */
    PRV_CPSS_SRV_CPU_TYPE_ENT scpuType;
    GT_STATUS (*pre_load_func)(
        GT_U8 devNum,
        struct PRV_CPSS_FIRMWARE_CONFIG_STCT *config
    );
} PRV_CPSS_FIRMWARE_CONFIG_STC;

static GT_STATUS prv_msys_pre_load(
    GT_U8 devNum,
    struct PRV_CPSS_FIRMWARE_CONFIG_STCT *config
)
{
#if 0
    GT_STATUS rc;
    /* CPU0 Boot Address Redirect Register */
    rc = cpssDrvPpHwInternalPciRegWrite(
                devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                0x22124, 0xffff0000);
    if (rc != GT_OK)
        return rc;
#endif
    cpssOsPrintf("Configure SRAM...\n");
    /* update hwInfo->sram.phys */
    prvCpssDrvPpConfig[devNum]->hwInfo[0].resource.sram.phys =
            (GT_PHYSICAL_ADDR)config->sramPhys;
    return mv_configure_sram(prvCpssDrvPpConfig[devNum]->hwInfo[0].resource.cnm.start,
                config->sramPhys, config->sramSize);
}

static PRV_CPSS_FIRMWARE_CONFIG_STC msys_fw_config = {
    "bobcat2",                    /* name */
    0xfff80000,                     /* sramPhys */
    _512K,                          /* sramSize */
    0xfffffffc,                     /* ipcShmRegs */
    mvHwsServiceCpuBc2Fw_INFO,
    PRV_CPSS_SRV_CPU_TYPE_MSYS_E,   /* scpuType */
    prv_msys_pre_load               /* pre_load_func */
};

static PRV_CPSS_FIRMWARE_CONFIG_STC bobk_msys_fw_config = {
    "bobk",                    /* name */
    0xfff80000,                     /* sramPhys */
    _512K,                          /* sramSize */
    0xfffffffc,                     /* ipcShmRegs */
    mvHwsServiceCpuBobkFw_INFO,
    PRV_CPSS_SRV_CPU_TYPE_MSYS_E,   /* scpuType */
    prv_msys_pre_load               /* pre_load_func */
};


static GT_STATUS prv_cm3_bobk_pre_load(
    GT_U8 devNum,
    struct PRV_CPSS_FIRMWARE_CONFIG_STCT *config GT_UNUSED
)
{

    GT_U32 reg;
    /* Confi Processor Global Configuration Register
     * 0x500
     */
    cpssDrvPpHwRegisterRead(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                0x500, &reg);
    reg |= 0x00100000; /* bit 20: init ram */
    reg &= 0xfff7ffff; /* bit 19: CM3_PROC_CLK_DIS */
    cpssDrvPpHwRegisterWrite(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                0x500, reg);
    cpssDrvPpHwRegisterRead(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                0x54, &reg);
    reg |= 1; /* bit 0: CM3_METAL_FIX_JTAG_EN */
    cpssDrvPpHwRegisterWrite(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                0x54, reg);


    return GT_OK;
}

static GT_STATUS prv_cm3_pre_load(
    GT_U8 devNum,
    struct PRV_CPSS_FIRMWARE_CONFIG_STCT *config GT_UNUSED
)
{

    GT_U32 reg;
    /* Confi Processor Global Configuration Register
     * 0x500
     */
    cpssDrvPpHwRegisterRead(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                0x500, &reg);
    reg |= 0x00100000; /* bit 20: init ram */
    reg |= 0x10000000; /* bit 28: CM3_Enable */

    cpssDrvPpHwRegisterWrite(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                0x500, reg);

    return GT_OK;
}


static PRV_CPSS_FIRMWARE_CONFIG_STC cm3_bobk_fw_config = {
    "cm3(bobk)",                    /* name */
    0x0,                            /* sramPhys */
    _128K,                          /* sramSize */
    0x1fffc,                        /* ipcShmRegs */
    mvHwsServiceCpuCm3BobKFw_INFO,  /* CM3_BOBK*/
    PRV_CPSS_SRV_CPU_TYPE_CM3_BOBK_E, /* scpuType */
    prv_cm3_bobk_pre_load           /* pre_load_func */
};


static PRV_CPSS_FIRMWARE_CONFIG_STC cm3_aldrin_fw_config = {
    "cm3(aldrin)",                  /* name */
    0x0,                            /* sramPhys */
    _192K,                          /* sramSize */
    0x2fffc,                        /* ipcShmRegs */
    mvHwsServiceCpuCm3AldrinFw_INFO,
    PRV_CPSS_SRV_CPU_TYPE_CM3_ALDRIN_E, /* scpuType */
    prv_cm3_pre_load                /* pre_load_func */
};

static PRV_CPSS_FIRMWARE_CONFIG_STC bc3_cm3_fw_config = {
    "cm3(bc3)",                     /* name */
    0x0,                            /* sramPhys */
    _192K,                          /* sramSize */
    0x2fffc,                        /* ipcShmRegs */
    mvHwsServiceCpuCm3Bc3Fw_INFO,   /* fwSize, fwArrSize, compress, ipcShmAddr, ipcShmSize */
    PRV_CPSS_SRV_CPU_TYPE_CM3_ALDRIN_E, /* scpuType */
    prv_cm3_pre_load                /* pre_load_func */
};

static PRV_CPSS_FIRMWARE_CONFIG_STC pipe_cm3_fw_config = {
    "cm3(pipe)",                  /* name */
    0x0,                            /* sramPhys */
    _192K,                          /* sramSize */
    0x2fffc,                        /* ipcShmRegs */
    mvHwsServiceCpuCm3PipeFw_INFO,
    PRV_CPSS_SRV_CPU_TYPE_CM3_ALDRIN_E, /* scpuType */
    prv_cm3_pre_load                /* pre_load_func */
};

static PRV_CPSS_FIRMWARE_CONFIG_STC aldrin2_cm3_fw_config = {
    "cm3(aldrin2)",                  /* name */
    0x0,                            /* sramPhys */
    _384K,                          /* sramSize */
    0x2fffc,                        /* ipcShmRegs */
    mvHwsServiceCpuCm3Aldrin2Fw_INFO,
    PRV_CPSS_SRV_CPU_TYPE_CM3_ALDRIN2_E, /* scpuType */
    prv_cm3_pre_load                /* pre_load_func */
};

struct PRV_CPSS_FIRMWARE_DETECT_DEV_STCT {
    GT_U16                          devId;
    GT_U16                          devIdMask;
    PRV_CPSS_FIRMWARE_CONFIG_STC   *fw_config;
} fwConfig[] = {
    { 0xfc00, 0xff00, &msys_fw_config }, /* bobcat2 */
    { 0xbe00, 0xff00, &bobk_msys_fw_config }, /* cetus */
    { 0xbc00, 0xff00, &bobk_msys_fw_config }, /* caelum */
    { 0xbe00, 0xff00, &cm3_bobk_fw_config }, /* cetus/CM3 */
    { 0xbc00, 0xff00, &cm3_bobk_fw_config }, /* caelum/CM3 */
    { 0xf400, 0xfe00, &msys_fw_config }, /* alleycat3\s */
    { 0xc800, 0xff00, &cm3_aldrin_fw_config }, /* aldrin */
    { 0xd400, 0xff00, &bc3_cm3_fw_config }, /* bc3 */
    { 0xc400, 0xff00, &pipe_cm3_fw_config }, /* pipe */
    { 0xcc00, 0xff00, &aldrin2_cm3_fw_config }, /* aldrin2 */

    { 0,0,NULL }
};

/**
* @internal prvCpssGenericSrvCpuCheck function
* @endinternal
*
* @brief   Check if device applicable
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
static GT_STATUS prvCpssGenericSrvCpuCheck
(
    IN GT_U8    devNum,
    OUT PRV_CPSS_FIRMWARE_CONFIG_STC **config
)
{
    GT_U32 i;
    GT_U16 deviceId; /* PCI device Id */
    CPSS_HW_INFO_STC *hwInfo = &(prvCpssDrvPpConfig[devNum]->hwInfo[0]);
    *config = NULL;

    if (devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    if (prvCpssDrvPpConfig[devNum] == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    deviceId = ((prvCpssDrvPpConfig[devNum]->devType)>>16) & 0xffff;

    for (i = 0; fwConfig[i].fw_config; i++)
    {
        if ((deviceId & fwConfig[i].devIdMask) == fwConfig[i].devId)
        {
            if (( fwConfig[i].fw_config->scpuType == PRV_CPSS_SRV_CPU_TYPE_MSYS_E) &&
                    (hwInfo->busType == CPSS_HW_INFO_BUS_TYPE_MBUS_E))
            {
                /* CPSS is running on MSYS && FW is running the same MSYS */
                continue;
            }
            *config = fwConfig[i].fw_config;
            break;
        }
    }
    if (!(*config))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/* memcopy 32-bit aligned data */
static GT_STATUS prvMemCpyAndCheck(GT_UINTPTR dest, GT_UINTPTR src, GT_U32 len)
{
    GT_U32 i;

    for (i = 0; i < len; i+=4)
    {
        *((volatile GT_U32*)(dest + i)) = *((GT_U32*)(src+i));
    }
    GT_SYNC;
    for (i = 0; i < len; i+=4)
    {
        if (*((volatile GT_U32*)(dest + i)) != *((GT_U32*)(src+i)))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    }
    return GT_OK;
}

struct obuff {
    GT_U8       buf[10240];
    GT_U32      p;
    GT_U32      o;
    GT_UINTPTR  dest;
};

static GT_STATUS obuff_flush(struct obuff *b)
{
    if (!b->p)
        return GT_OK;

    if (prvMemCpyAndCheck(b->dest+b->o, (GT_UINTPTR)(b->buf), b->p) != GT_OK)
    {
        cpssOsPrintf(" memcopy check failed at offset 0x%x!\n",b->o);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    b->o += b->p;
    b->p = 0;
    return GT_OK;
}

static GT_STATUS obuff_putc(struct obuff *b, GT_U8 c)
{
    GT_STATUS rc;
    b->buf[b->p++] = c;
    if (b->p == sizeof(b->buf))
    {
        rc = obuff_flush(b);
        if (rc != GT_OK)
            return rc;
    }
    return GT_OK;
}
static GT_STATUS prvDoSimpleDecompress(
    IN GT_UINTPTR                    destAddr,
    IN PRV_CPSS_FIRMWARE_CONFIG_STC *config
)
{
    struct obuff b;
    GT_U32 p, i, c;
    GT_STATUS rc;

    b.p = 0;
    b.o = 0;
    b.dest = destAddr;

    for (p = 0; p < config->fwArrSize; )
    {
        c = (config->fwData[p] & 0x7f) + 1;
        if ((config->fwData[p] & 0x80) == 0)
        {
            p++;
            for (i = 0; i < c; i++)
            {
                rc = obuff_putc(&b, config->fwData[p++]);
                if (rc != GT_OK)
                    return rc;
            }
        }
        else
        {
            p++;
            for (i = 0; i < c; i++)
            {
                rc = obuff_putc(&b, 0);
                if (rc != GT_OK)
                    return rc;
            }
        }
    }
    rc = obuff_flush(&b);
    if (rc != GT_OK)
        return rc;
    if (p != config->fwArrSize || b.o != config->fwSize)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    return GT_OK;
}

/**
* @internal prvCpssIpcLinkCheck function
* @endinternal
*
* @brief   Check ipc link
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
static GT_STATUS prvCpssIpcLinkCheck
(
    IN GT_U8    devNum,
    IPC_SHM_STC *shm
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
            rc = shmIpcRecv(shm, chn, &replyData, NULL);
            if (rc != 1)
                break;
        }
    }

    cpssOsMemSet(&msgData, 0, sizeof(msgData));
    cpssOsMemSet(&replyData, 0, sizeof(replyData));

    msgData.msgData.noop.data = 3;
    msgData.ctrlMsgType = MV_HWS_IPC_NOOP_MSG;
    msgData.devNum = devNum;
    msgData.msgLength = (GT_U8)(sizeof(msgData.msgData.noop));
    msgData.msgQueueId = (GT_U8)MV_HWS_IPC_TX_0_CH_ID;
#define IPC_OFFSETOF(_type,_member) ((GT_U32)((GT_UINTPTR)&(((_type*)NULL)->_member)))
    rc = shmIpcSend(shm, MV_HWS_IPC_LOW_PRI_QUEUE,
            &msgData,
            msgData.msgLength + IPC_OFFSETOF(MV_HWS_IPC_CTRL_MSG_STRUCT,msgData.noop));
    if (rc != 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* receive msg */
    cpssOsTimerWkAfter(100);
    rc = shmIpcRecv(shm, MV_HWS_IPC_TX_0_CH_ID, &replyData, NULL);
    if (rc != 1)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    if (replyData.replyTo == (GT_U32)MV_HWS_IPC_NOOP_MSG)
    {
        /* The expected data == 5 */
        if (replyData.readData.noopReply.data != 5)
        {
            osPrintf (" wrong noop data:\n",replyData.readData.noopReply.data);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        osPrintf (" wrong msg ID %d Expected %d queue ID %d\n",replyData.replyTo, MV_HWS_IPC_NOOP_MSG,MV_HWS_IPC_TX_0_CH_ID);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

static void sync_ipc_memory(
    void* cookie,
    IPC_SHM_SYNC_FUNC_MODE_ENT mode,
    void*   ptr,
    IPC_UINTPTR_T targetPtr,
    IPC_U32 size
)
{
    IPC_U32 i;
    /* direct access only */
    (void)cookie;
    if (mode == IPC_SHM_SYNC_FUNC_MODE_READ_E)
    {
        for (i = 0; i < size; i += 4)
        {
            *((IPC_U32*)(((IPC_UINTPTR_T)ptr)+i)) = *((volatile IPC_U32*)(targetPtr+i));
        }
        return;
    }
    /* mode == IPC_SHM_SYNC_FUNC_MODE_WRITE_E */
    for (i = 0; i < size; i += 4)
    {
        *((volatile IPC_U32*)(targetPtr+i)) = *((IPC_U32*)(((IPC_UINTPTR_T)ptr)+i));
    }
}

/**
* @internal prvCpssGenericSrvCpuProbe function
* @endinternal
*
* @brief   Probe for FW
*         Fill prvIpcDevCfg[devNum] is OK
*
* @note   APPLICABLE DEVICES:      Bobcat2, Caelum, AC3, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon, Pipe
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number.
*
* @retval GT_TRUE                  - AP exists and runngin
* @retval GT_FALSE                 - not running
*/
static GT_BOOL prvCpssGenericSrvCpuProbe
(
    IN GT_U8    devNum
)
{
    PRV_CPSS_FIRMWARE_CONFIG_STC *pconfig, config;
    PRV_CPSS_IPC_DEV_CFG_STC *ipc;
    IPC_SHM_STC shm;
    GT_U32  data, i;
    GT_STATUS rc;
    GT_UINTPTR base;
    CPSS_HW_INFO_STC *hwInfo = &(prvCpssDrvPpConfig[devNum]->hwInfo[0]);

    rc = prvCpssGenericSrvCpuCheck(devNum, &pconfig);
    if (rc != GT_OK)
        return rc;

    config = *pconfig;

    /* read service CPU state register */
    if (config.scpuType == PRV_CPSS_SRV_CPU_TYPE_MSYS_E)
    {
        /* CPUn SW Reset Control Register
         * 20800+8*n
         * Bit 0: CPU_SW_RESET
         */
        rc = cpssDrvPpHwInternalPciRegRead(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
               0x20800, &data);
    }
    else
    {
        /* Confi Processor Global Configuration Register
         * 0x500
         * Bit 28 for BobK and Bit 29 for Aldrin: Controls CM3 activation: 0: Disable 1:enable
         */
        rc = cpssDrvPpHwRegisterRead(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                0x500, &data);
    }
    if (rc != GT_OK)
        return GT_FALSE;
#define SRAM_VIRT_ADDR(_sram_addr) (base+((_sram_addr)-config.sramPhys))

    switch (config.scpuType)
    {
        case PRV_CPSS_SRV_CPU_TYPE_MSYS_E:
            if (data & 0x1)
                return GT_FALSE; /* MSYS in reset state now */
            base = hwInfo->resource.sram.start;
            config.sramPhys = 0;
            config.sramSize = 0;
            for (i = 0; i < 4; i++)
            {
                GT_U32 base, size;
                rc = cpssDrvPpHwInternalPciRegRead(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                    0x20240+i*4, &data);
                if (rc != GT_OK)
                    return GT_FALSE;
                if ((data & 1) == 0)
                    continue;

                base = data & 0xffff0000;
                size = ((data & 0x00000700) << 8) + 0x00010000;
                if (config.sramSize == 0)
                {
                    config.sramPhys = base;
                    config.sramSize = size;
                }
                else
                {
                    if (config.sramPhys + config.sramSize == base)
                        config.sramSize += size;
                }
            }
            if (config.sramSize == 0)
                return GT_FALSE; /* No SRAM mapping configured */

            /* configure PEX window */
            mv_configure_pex_window_sram(
                    hwInfo->resource.cnm.start,
                    config.sramPhys,
                    config.sramSize);
            /* update hwInfo */
            hwInfo->resource.sram.phys = (GT_PHYSICAL_ADDR)config.sramPhys;

            break;
        case PRV_CPSS_SRV_CPU_TYPE_CM3_BOBK_E:
            if ((data & 0x10000000) == 0)
                return GT_FALSE; /* BobK CM3 is not running */

            config.sramSize = _128K;
            config.sramPhys = 0;
            base = hwInfo->resource.switching.start + 0x40000;
            break;
        case PRV_CPSS_SRV_CPU_TYPE_CM3_ALDRIN_E:
            if ((data & 0x20000000) == 0)
                return GT_FALSE; /* Aldrin CM3 is not running */

            config.sramSize = _192K;
            config.sramPhys = 0;
            base = hwInfo->resource.switching.start;
            /* set address completion region 7 to 0x80000 */
            *((volatile GT_U32*)(base + 0x120 + (4 * 7))) = CPSS_32BIT_LE(0x80000 >> 19);
            GT_SYNC;
            base += 0x80000 * 7;
            break;
        case PRV_CPSS_SRV_CPU_TYPE_CM3_ALDRIN2_E:
            if ((data & 0x20000000) == 0)
                return GT_FALSE; /* Aldrin CM3 is not running */

            config.sramSize = _384K;
            config.sramPhys = 0;
            base = hwInfo->resource.switching.start;
            /* set address completion region 7 to 0x80000 */
            *((volatile GT_U32*)(base + 0x120 + (4 * 7))) = CPSS_32BIT_LE(0x80000 >> 19);
            GT_SYNC;
            base += 0x80000 * 7;
            break;
        default:
            return GT_FALSE;
    }
    config.ipcShmRegs = config.sramPhys + (config.sramSize-4);
    data = CPSS_32BIT_LE(*((volatile GT_U32*)(SRAM_VIRT_ADDR(config.ipcShmRegs))));
    if (data != IPC_RESVD_MAGIC)
        return GT_FALSE;
    data = CPSS_32BIT_LE(*((volatile GT_U32*)(SRAM_VIRT_ADDR(config.ipcShmRegs-4))));
    config.ipcShmAddr = data - 0x100;
    if (config.ipcShmAddr < config.sramPhys || config.ipcShmAddr >= config.ipcShmRegs)
        return GT_FALSE;

    shm.shm = (IPC_UINTPTR_T)SRAM_VIRT_ADDR(config.ipcShmAddr);
    shm.shmLen = 0x800;
    shm.master = 1;
    shm.syncFunc = sync_ipc_memory;

    /* check if ipc link was initialized */
    if ( (shmIpcRegRead(&shm, IPC_SHM_NUM_REGS) != IPC_SHM_MAGIC)  &&
         (shmIpcRegRead(&shm, IPC_SHM_NUM_REGS) != IPC_SHM_MAGIC+1) )
    {
        return GT_FALSE;
    }

    rc = prvCpssIpcLinkCheck(devNum,&shm);
    if (rc != GT_OK)
        return GT_FALSE;

    /* Fill prvIpcDevCfg[devNum] */
    ipc = (PRV_CPSS_IPC_DEV_CFG_STC*)cpssOsMalloc(sizeof(*ipc));
    if (!ipc)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    prvIpcDevCfg[devNum] = ipc;
    cpssOsMemSet(ipc, 0, sizeof(*ipc));
    ipc->shm = shm;
    ipc->sramBase = (void*)base;
    ipc->srvCpuBaseAddr = config.sramPhys;
    ipc->shmAddress = (void*)SRAM_VIRT_ADDR(config.ipcShmAddr);
    ipc->scpuType = config.scpuType;
    ipc->linkConfigured = 2;
#undef SRAM_VIRT_ADDR
    return GT_TRUE;
}

/**
* @internal prvCpssFwToSecondSrvCpuLoad function
* @endinternal
*
* @brief   Upload FW to second service CPU
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  AC3; Bobcat2; Caelum; Cetus; Aldrin;
*
* @param[in] devNum                   - device number.
* @param[in] fwConfigPtr              - pointer to config FW data.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS prvCpssFwToSecondSrvCpuLoad
(
    IN GT_U8    devNum,
    IN PRV_CPSS_FIRMWARE_CONFIG_STC *fwConfigPtr
)
{
    GT_UINTPTR  base;
    /*GT_U32      sramSize;*/
    GT_U32      addrCompRegIndex;
    GT_STATUS rc;
    CPSS_HW_INFO_STC *hwInfo = &(prvCpssDrvPpConfig[devNum]->hwInfo[0]);

    cpssOsPrintf("START flow for %s FW load!!!\n", fwConfigPtr->name);

    rc = prvCpssDrvHwPpResourceSetRegField(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                           CPSS_DRV_HW_RESOURCE_MG1_CORE_E, 0x500, 29, 1, 0);
    if (rc != GT_OK)
    {
        cpssOsPrintf("Failed to stop CM3_1 for FW\n");
        return rc;
    }

    /* bit 20: init ram */
    rc = prvCpssDrvHwPpResourceSetRegField(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                           CPSS_DRV_HW_RESOURCE_MG1_CORE_E, 0x500, 20, 1, 1);
    if (rc != GT_OK)
    {
        cpssOsPrintf("Failed init ram CM3_1 for FW\n");
        return rc;
    }

    /* bit 28: CM3_Enable */
    rc = prvCpssDrvHwPpResourceSetRegField(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                           CPSS_DRV_HW_RESOURCE_MG1_CORE_E, 0x500, 28, 1, 1);
    if (rc != GT_OK)
    {
        cpssOsPrintf("Failed CM3_Enable CM3_1 for FW\n");
        return rc;
    }

    base = hwInfo->resource.mg1.start;
    /* set address completion region 1 to 0x80000 */
    addrCompRegIndex =1;
    *((volatile GT_U32*)(base + 0x120 + (4 * addrCompRegIndex))) = CPSS_32BIT_LE(0x80000 >> 19);
    GT_SYNC;
    base += (0x80000 * addrCompRegIndex);

    if (0 == fwConfigPtr->compress)
    {
        rc = prvMemCpyAndCheck(base+fwConfigPtr->fwLoadAddr,
                               (GT_UINTPTR)(fwConfigPtr->fwData),
                               fwConfigPtr->fwSize);
        if (rc != GT_OK)
        {
            cpssOsPrintf("FW to CM3_1 memcopy check failed!\n");
            return rc;
        }
    }
    else if (2 == fwConfigPtr->compress)
    {
        rc = prvDoSimpleDecompress((base+fwConfigPtr->fwLoadAddr), fwConfigPtr);
        if (rc != GT_OK)
        {
            cpssOsPrintf("Failed to decompress FW on CM3_1\n");
            return rc;
        }
    }
    else
    {
        cpssOsPrintf("FW - Usupported compression method");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDrvHwPpResourceSetRegField(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                           CPSS_DRV_HW_RESOURCE_MG1_CORE_E, 0x500, 29, 1, 1);
    if (rc != GT_OK)
    {
        cpssOsPrintf("Failed to start FW on CM3_1\n");
        return rc;
    }

    cpssOsPrintf("END flow for %s FW load!!!\n", fwConfigPtr->name);

    return GT_OK;
}

#endif /* !defined(ASIC_SIMULATION) */

/**
* @internal prvCpssDbaToSrvCpuLoad function
* @endinternal
*
* @brief   Upload DBA FW to second service CPU
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  AC3; Bobcat2; Caelum; Cetus; Aldrin;
*
* @param[in] devNum                   - device number.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvCpssDbaToSrvCpuLoad
(
    IN GT_U8    devNum
)
{
#ifndef ASIC_SIMULATION
    GT_STATUS rc;

    PRV_CPSS_FIRMWARE_CONFIG_STC dbaConfig = {"cm3(dba)", 0x0, _192K, 0, bc3DbaFw_INFO, 0, NULL};

    rc = prvCpssFwToSecondSrvCpuLoad(devNum, &dbaConfig);
    if (rc != GT_OK)
    {
        cpssOsPrintf("Failed to start DBA on CM3_1\n");
        return rc;
    }
#else /* defined(ASIC_SIMULATION) */
    (void)devNum;
#endif
    return GT_OK;
}

GT_STATUS prvCpssTamToSrvCpuLoad
(
    IN GT_U8    devNum
)
{
#ifndef ASIC_SIMULATION
    GT_STATUS rc;

    PRV_CPSS_FIRMWARE_CONFIG_STC tamConfig = {"cm3(tam)", 0x0, _192K, 0, bc3TamFw_INFO, 0, NULL};

    rc = prvCpssFwToSecondSrvCpuLoad(devNum, &tamConfig);
    if (rc != GT_OK)
    {
        cpssOsPrintf("Failed to start TAM on CM3_1\n");
        return rc;
    }
#else /* defined(ASIC_SIMULATION) */
    (void)devNum;
#endif
    return GT_OK;
}

/**
* @internal prvCpssGenericSrvCpuLoad function
* @endinternal
*
* @brief   Upload firmware to service CPU according to firmware table
*
* @note   APPLICABLE DEVICES:      AC3; Bobcat2; Caelum; Cetus; Aldrin; Bobcat3; Aldrin2; Falcon; PIPE.
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
GT_STATUS prvCpssGenericSrvCpuLoad
(
    IN GT_U8    devNum
)
{
#ifndef ASIC_SIMULATION
    PRV_CPSS_FIRMWARE_CONFIG_STC *config;
    PRV_CPSS_IPC_DEV_CFG_STC *ipc;
    GT_UINTPTR  base;
    GT_U32      sramSize;
    GT_STATUS rc;
    GT_U32 revId;
    GT_U32 devId;
    MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;
    CPSS_HW_INFO_STC *hwInfo = &(prvCpssDrvPpConfig[devNum]->hwInfo[0]);
    CPSS_SYSTEM_RECOVERY_INFO_STC system_recovery; /* holds system recovery information */

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
      return rc;
    }

    if (prvIpcDevCfg[devNum] != NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, LOG_ERROR_NO_MSG);

    if (system_recovery.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
    {
        if (prvCpssGenericSrvCpuProbe(devNum) == GT_TRUE)
            return GT_OK;
        cpssOsPrintf("prvCpssGenericSrvCpuProbe is failed ...!!!!\n");
    }
    rc = prvCpssGenericSrvCpuCheck(devNum, &config);
    if (rc != GT_OK)
        return rc;

    prvCpssGenericSrvCpuStop(devNum);

    if (config->pre_load_func != NULL)
    {
        rc = config->pre_load_func(devNum, config);
        if (rc != GT_OK)
            return rc;
    }
    sramSize = config->sramSize;
    switch (config->scpuType)
    {
        case PRV_CPSS_SRV_CPU_TYPE_MSYS_E:
            base = hwInfo->resource.sram.start;
            if ((GT_U32)hwInfo->resource.sram.size != 0xffffffff)
                sramSize = (GT_U32)hwInfo->resource.sram.size;
            break;
        case PRV_CPSS_SRV_CPU_TYPE_CM3_BOBK_E:
            base = hwInfo->resource.switching.start + 0x40000;
            break;
        case PRV_CPSS_SRV_CPU_TYPE_CM3_ALDRIN_E:
        case PRV_CPSS_SRV_CPU_TYPE_CM3_ALDRIN2_E:
            base = hwInfo->resource.switching.start;
            /* set address completion region 7 to 0x80000 */
            *((volatile GT_U32*)(base + 0x120 + (4 * 7))) = CPSS_32BIT_LE(0x80000 >> 19);
            GT_SYNC;
            base += 0x80000 * 7;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    /* TODO: load from NFS */

#define SRAM_VIRT_ADDR(_sram_addr) (base+((_sram_addr)-config->sramPhys))
    if (config->fwData == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

    if ((config->fwLoadAddr - config->sramPhys) + config->fwSize > sramSize)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    cpssOsPrintf("Uploading firmware to %s:0x%x\n", config->name, config->fwLoadAddr);
    cpssOsPrintf(" memcpy(%p, fw, 0x%x);\n",
        (void*)SRAM_VIRT_ADDR(config->fwLoadAddr), (unsigned)(config->fwSize));
    if (config->compress == 0)
    {
        rc = prvMemCpyAndCheck(
                    SRAM_VIRT_ADDR(config->fwLoadAddr),
                    (GT_UINTPTR)(config->fwData),
                    config->fwSize);
        if (rc != GT_OK)
        {
            cpssOsPrintf(" memcopy check failed!\n");
            return rc;
        }
    }
    else if (config->compress == 2)
    {
        rc = prvDoSimpleDecompress(SRAM_VIRT_ADDR(config->fwLoadAddr), config);
        if (rc != GT_OK)
            return rc;
    }
    else
    {
        cpssOsPrintf("Usupported compression method");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }
    /* TODO: counter ??? */

    /* Fill prvIpcDevCfg[devNum] */
    ipc = (PRV_CPSS_IPC_DEV_CFG_STC*)cpssOsMalloc(sizeof(*ipc));
    if (!ipc)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    prvIpcDevCfg[devNum] = ipc;
    cpssOsMemSet(ipc, 0, sizeof(*ipc));
    ipc->sramBase = (void*)base;
    ipc->srvCpuBaseAddr = config->sramPhys;
    ipc->shmAddress = (void*)SRAM_VIRT_ADDR(config->ipcShmAddr);
    ipc->linkConfigured = 0;
    ipc->scpuType = config->scpuType;
cpssOsPrintf("SRAM base address=%p==%p shmAddress=%p\n",(void*)base,ipc->sramBase,ipc->shmAddress);

    /* Configure link before FW started */
    cpssOsPrintf("Configuring ipc link... shmAddress=%p\n", ipc->shmAddress);
    rc = shmIpcInit(&(ipc->shm),
            ipc->shmAddress,
            config->ipcShmSize, 1, sync_ipc_memory, NULL);
    if (rc != 0)
    {
        cpssOsPrintf("Failed to setup IPC for device %d\n", devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* Write magick to reserved area */
    shmResvdAreaRegWrite(&(ipc->shm), SRAM_VIRT_ADDR(config->ipcShmRegs), 0, IPC_RESVD_MAGIC);
    /* Write pointer to SHM registers to reserved area */
    shmResvdAreaRegWrite(&(ipc->shm), SRAM_VIRT_ADDR(config->ipcShmRegs), 1, config->ipcShmAddr + 0x100);

    /* configure channels */
    shmIpcConfigChannel(&(ipc->shm), 0, 0,  0, 4, 64);
    shmIpcConfigChannel(&(ipc->shm), 1, 0,  0, 4, 64);
    shmIpcConfigChannel(&(ipc->shm), 2, 1, 64, 0, 0);
    shmIpcConfigChannel(&(ipc->shm), 3, 1, 64, 0, 0);
    shmIpcConfigChannel(&(ipc->shm), 4, 1, 64, 0, 0);
    shmIpcConfigChannel(&(ipc->shm), 5, 1, 64, 0, 0);
    /* Getting the generic device info function */
    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);
    /* Getting the device info */
    if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].deviceInfoFunc == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
    }
    rc = hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].deviceInfoFunc(devNum, &devId, &revId);
    if (rc != GT_OK)
    {
        return rc;
    }

    {
        GT_U32 smemData;
        /* 32Bits of data represent 28Bits hws device type enum and 4Bits device revision id */
        smemData = ( (hwsDeviceSpecInfo[devNum].devType & 0xFFFFFFF) | ( (revId & 0xF) << 28) );
        shmIpcRegWrite(&(ipc->shm), 2, smemData);
    }

#undef SRAM_VIRT_ADDR
    if (system_recovery.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
    {
        ipc->linkConfigured = 1;
        prvCpssGenericSrvCpuStart(devNum);
    }

    return GT_OK;
#else /* defined(ASIC_SIMULATION) */
    prvIpcDevCfg[devNum] = NULL;
    return GT_OK;
#endif /* defined(ASIC_SIMULATION) */
}

/**
* @internal prvCpssGenericSrvCpuRemove function
* @endinternal
*
* @brief   Cleanup service CPU resources while cpssDxChCfgDevRemove
*
* @note   APPLICABLE DEVICES:      AC3; Bobcat2; Caelum; Cetus; Aldrin; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
*/
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
        cpssOsFree(prvIpcDevCfg[devNum]);
        prvIpcDevCfg[devNum] = NULL;
    }
#else /* defined(ASIC_SIMULATION) */
    (void)devNum;
#endif
    return GT_OK;
}


/**
* @internal prvCpssGenericSrvCpuStartStop function
* @endinternal
*
* @brief   Start/Stop (unreset/reset) service CPU(MSYS)
*
* @note   APPLICABLE DEVICES:      AC3; Bobcat2; Caelum; Cetus; Aldrin; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] start                    - start/stop service CPU
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS prvCpssGenericSrvCpuStartStop
(
    IN GT_U8    devNum,
    IN GT_BOOL  start
)
{
#ifndef ASIC_SIMULATION
    GT_U32  reg;
    GT_STATUS rc;
    PRV_CPSS_SRV_CPU_TYPE_ENT  scpuType;
    GT_U32 bit;

    if (prvIpcDevCfg[devNum] != NULL)
    {
        if (prvIpcDevCfg[devNum]->linkConfigured == 2)
        {
            /* pre-configured, no start/stop */
            return GT_OK;
        }
        scpuType = prvIpcDevCfg[devNum]->scpuType;
    }
    else
    {
        PRV_CPSS_FIRMWARE_CONFIG_STC *config;
        rc = prvCpssGenericSrvCpuCheck(devNum, &config);
        if (rc != GT_OK)
            return rc;
        scpuType = config->scpuType;
    }

    switch (scpuType)
    {
        case PRV_CPSS_SRV_CPU_TYPE_MSYS_E:
            /* CPUn SW Reset Control Register
             * 20800+8*n
             * Bit 0: CPU_SW_RESET
             */
            rc = cpssDrvPpHwInternalPciRegRead(
                        devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                        0x20800, &reg);
            if (rc != GT_OK)
                return rc;
            if (start == GT_TRUE)
                reg &= 0xfffffffe;
            else
                reg |= 1;
            rc = cpssDrvPpHwInternalPciRegWrite(
                        devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                        0x20800, reg);
            break;
        case PRV_CPSS_SRV_CPU_TYPE_CM3_BOBK_E:
        case PRV_CPSS_SRV_CPU_TYPE_CM3_ALDRIN_E:
        case PRV_CPSS_SRV_CPU_TYPE_CM3_ALDRIN2_E:
            /* Confi Processor Global Configuration Register
             * 0x500
             * Bit 28 for BobK and Bit 29 for Aldrin: Controls CM3 activation: 0: Disable 1:enable
             */
            bit = (scpuType == PRV_CPSS_SRV_CPU_TYPE_CM3_BOBK_E)? 0x10000000 : 0x20000000;
            rc = cpssDrvPpHwRegisterRead(
                        devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                        0x500, &reg);
            if (rc != GT_OK)
                return rc;
            if (start == GT_TRUE)
            {
                reg |= bit;
            }
            else
            {
                reg &= ~bit;
            }
            rc = cpssDrvPpHwRegisterWrite(
                        devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                        0x500, reg);
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    return rc;
#else /* defined(ASIC_SIMULATION) */
    (void)devNum;
    (void)start;
    return GT_OK;
#endif
}

/**
* @internal prvCpssGenericSrvCpuStop function
* @endinternal
*
* @brief   Stop (reset) service CPU(MSYS)
*
* @note   APPLICABLE DEVICES:      AC3; Bobcat2; Caelum; Cetus; Aldrin; Bobcat3; Aldrin2; Falcon; Pipe.
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
GT_STATUS prvCpssGenericSrvCpuStop
(
    IN GT_U8    devNum
)
{
    return prvCpssGenericSrvCpuStartStop(devNum, GT_FALSE);
}

/**
* @internal prvCpssGenericSrvCpuReNumberDevNum function
* @endinternal
*
* @brief   Replace the ID of a device from old device number
*         to a new device number
* @param[in] oldDevNum                - old device num
* @param[in] newDevNum                - new device num
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
*/
GT_STATUS prvCpssGenericSrvCpuReNumberDevNum
(
    IN  GT_U8   oldDevNum,
    IN  GT_U8   newDevNum
)
{
    if (oldDevNum >= PRV_CPSS_MAX_PP_DEVICES_CNS
        || newDevNum >= PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (newDevNum == oldDevNum)
        return GT_OK;
    if (prvIpcDevCfg[newDevNum] != NULL)
    {
        cpssOsFree(prvIpcDevCfg[newDevNum]);
    }
    prvIpcDevCfg[newDevNum] = prvIpcDevCfg[oldDevNum];
    prvIpcDevCfg[oldDevNum] = NULL;
    return GT_OK;
}

/**
* @internal prvCpssGenericSrvCpuStart function
* @endinternal
*
* @brief   Start (unreset) service CPU(MSYS)
*
* @note   APPLICABLE DEVICES:      AC3; Bobcat2; Caelum; Cetus; Aldrin; Bobcat3; Aldrin2; Falcon; Pipe.
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
GT_STATUS prvCpssGenericSrvCpuStart
(
    IN GT_U8    devNum
)
{
    GT_STATUS rc;
    rc = prvCpssGenericSrvCpuStartStop(devNum, GT_TRUE);
    if (rc == GT_OK)
    {
        /* sleep 2 sec till FW up */
        cpssOsTimerWkAfter(2000);
    }
    return rc;
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
    IPC_SHM_STC                     shm;
    GT_32                           rc = 0, bc_rc = 0;
    GT_U32                          data, i;
    GT_U32                          bootChAddr;
    GT_U32                          maxDelay;
    GT_U32                          bufWithHeader[256];
    GT_U32                          dummyBuf;

    cpssOsPrintf("bufNum=%d bufSize=0x%X\n",bufNum, bufSize );

    if (prvIpcDevCfg[devNum] == NULL){
        if (prvCpssGenericSrvCpuProbe(devNum) != GT_TRUE){
            cpssOsPrintf("\nprvCpssGenericSrvCpuProbe is failed\n");
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    }

    shm.shm = (IPC_UINTPTR_T)(prvIpcDevCfg[devNum]->shmAddress);
    shm.shmLen = 0x800;
    shm.master = 1;  /* We are <master> -> initiate request to MI to write chunk of data */
    shm.syncFunc = sync_ipc_memory;

    bootChAddr = shm.shm;

    /* check status of boot channel */
    data = shmResvdAreaRegRead(&shm, bootChAddr,0);
    if (data != 0)
    {
        *status = data;
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* prepare command parameters */
    bufWithHeader[1] =  ((bufBmp & 0xFF)<<16) + 0x10;  /* opCode=0x10  */
    bufWithHeader[2] =  bufNum;
    bufWithHeader[3] =  bufSize;

    memcpy( &bufWithHeader[4], bufPtr, bufSize );
    shmIpcBootChnSend(&shm, 0, &bufWithHeader[1], (bufSize & 0xFF) + 12);

    i = 0;
    /* wait reply status about 2 sec for regular packets
         and 30 sec for finalize proc of mode#3 (flash erase and copy image)*/
    if ((bufSize | 0x100) && (bufBmp ==3)) maxDelay = 30000;
    else maxDelay = 2000;

    for( i = 0 ; i < maxDelay ; i++ ) /* wait for feedback from service CPU */
    {
        osTimerWkAfter(1);

        /* for each boot channel send operation, feedback receive is necessary.
           if not expected any valuable feedback data - dummy buffer must be used */
        rc=shmIpcBootChnRecv(&shm, &bc_rc, &dummyBuf, 4);
        if ( rc > 0 )
        {
            data = shmResvdAreaRegRead(&shm, bootChAddr,0);
            if ( data == 0 )
                break;
        }
    }
    if ( (data != 0) && (i > maxDelay) ) /* time out error */
        {
            cpssOsPrintf("\n\nTimeOut error\n\n");
            *status = data;
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    *status = rc;
    return GT_OK;


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

#ifndef ASIC_SIMULATION

/* #define CONFIG_SRAM_DEBUG */
#define BITS(_data,_start,_end) (((_data)>>(_end)) & ((1 << ((_start)-(_end)+1))-1))
static GT_U32 mv_reg_read(GT_UINTPTR regsBase, GT_U32 regAddr)
{
    GT_U32 val = *((volatile GT_U32*)(regsBase+regAddr));
#ifdef CONFIG_SRAM_DEBUG
    cpssOsPrintf("RD base=%p regAddr=0x%08x data=0x%08x\n",(void*)regsBase,regAddr,CPSS_32BIT_LE(val));
#endif
    return CPSS_32BIT_LE(val);
}
static void mv_reg_write(GT_UINTPTR regsBase, GT_U32 regAddr, GT_U32 value)
{
    *((volatile GT_U32*)(regsBase+regAddr)) = CPSS_32BIT_LE(value);
#ifdef CONFIG_SRAM_DEBUG
    cpssOsPrintf("WR base=%p regAddr=0x%08x data=0x%08x\n",(void*)regsBase,regAddr,value);
#endif
}
static void mv_reg_write_field(GT_UINTPTR regsBase, GT_U32 regAddr, GT_U32 mask, GT_U32 value)
{
    GT_U32 val;
    val = mv_reg_read(regsBase, regAddr);
    val &= ~mask;
    val |= value;
    mv_reg_write(regsBase, regAddr, val);
}

/* Configure SRAM window if it not configired yet
 * or confiruration is wrong (mvPpDrv:AC3 only)
 * Set WIN_BASE to PEX BAR4 + 2M
 */
static void mv_configure_pex_window_sram(
    GT_UINTPTR regsBase,
    GT_U32     sramPhys,
    GT_U32     sramSize
)
{
    GT_U32 data_ctl, data_remap, data_base, ctl_val, remap_val, base_val;
#define SCPU_WINDOW 2
#define WIN_CTRL    (0x41820+SCPU_WINDOW*0x10)
#define WIN_BASE    (WIN_CTRL+0x4)
#define WIN_REMAP   (WIN_CTRL+0xc)
#define SIZE_TO_BAR_REG(_size) ((((_size)>>16)-1)<<16)
    /* read base address from PCI Express BAR2 Register: 31:16 */
    base_val = mv_reg_read(regsBase, 0x40020) & 0xffff0000;
    base_val += _2M;
    ctl_val = SIZE_TO_BAR_REG(sramSize) |
            (0/*target_id*/ << 4) |
            (0x3e/*attr*/  << 8) |
            (1/*bar2*/ << 1);
    remap_val = sramPhys | 1;
    data_ctl = mv_reg_read(regsBase, WIN_CTRL);
    data_base = mv_reg_read(regsBase, WIN_BASE);
    data_remap = mv_reg_read(regsBase, WIN_REMAP);
    if (   (data_ctl == (ctl_val | 0x1))
        && (data_remap == remap_val)
        && ((data_base & 0xffff0000) == base_val) )
    {
        /* already configured */
        return;
    }
    mv_reg_write(regsBase, WIN_CTRL, 0);
    mv_reg_write(regsBase, WIN_BASE, base_val);
    mv_reg_write(regsBase, WIN_REMAP, remap_val);
    mv_reg_write(regsBase, WIN_CTRL, ctl_val);
    mv_reg_write(regsBase, WIN_CTRL, ctl_val|1);
}
static GT_STATUS mv_configure_sram(
    GT_UINTPTR regsBase,
    GT_U32     sramPhys,
    GT_U32     sramSize
)
{
    GT_U32     i, data;
    GT_U32     waySize = _64K;
    GT_U32     waysNo = 8;
    GT_U32     waysMask = 0xff;
    GT_U32     L2Size = _512K;


    if ((sramSize % _64K) != 0)
        return 1;

    /* Enable access to L2 configuration registers */
    /*reset bit 12 in MSYS_CIB_CONTROL_REG 0x20280 */
    mv_reg_write_field(regsBase, 0x20280, 0x00001000, 0x00000000);

    /* enable L2 cache and make L2 Auxiliary Control Register enable */
    mv_reg_write_field(regsBase, 0x8100, 0x00000001, 1);
#ifdef CONFIG_SRAM_DEBUG
    cpssOsPrintf("  Configuring SRAM. base=%p...\n",(void*)regsBase);

    /* L2 Control Register */
    data = mv_reg_read(regsBase, 0x8100);
    cpssOsPrintf("L2 Control Register=0x%08x\n", data);
#endif
    /* L2 Auxiliary Control Register */
    data = mv_reg_read(regsBase, 0x8104);
#ifdef CONFIG_SRAM_DEBUG
    cpssOsPrintf("L2 Auxiliary Control Register=0x%08x\n", data);
#endif
    if (data != 0)
    {
        /* bits 11:10 are L2 cache size 0-256K,1-512K,2-1M,3-2M */
        L2Size = _256K << BITS(data,11,10);
        /* bits 16:13 are cache way associative */
        if (BITS(data,16,13) == 3) {
            waysNo = 4;
            waysMask = 0x0f;
        }
        if (BITS(data,16,13) == 7) {
            waysNo = 8;
            waysMask = 0xff;
        }
        if (BITS(data,16,13) == 11) {
            waysNo = 16;
            waysMask = 0xffff;
        }
        if (BITS(data,16,13) == 15) {
            waysNo = 32;
            waysMask = 0xffffffff;
        }

        /* bits 19:17 are way size 2-16K,3-32K,4-64K,7-512K */
        waySize = _4K << BITS(data,19,17);
#ifdef CONFIG_SRAM_DEBUG
        cpssOsPrintf("\tL2Size=0x%x\n\twaysNo=%d, waySize=0x%x\n",L2Size,waysNo,waySize);
#endif
    }

    /* check params */
    if (sramSize > L2Size)
        return 2;
    if ((sramSize % waySize) != 0)
        return 2;
    if ((sramSize / waySize) > waysNo)
        return 2;


    /*write 0x01000102 to Coherency Fabric Control Register 0x20200:
     * bit25: snoop cpu1 enable == 0
     * bit24: snoop cpu0 enable == 0
     * bit8: MBUS Error Propagation == 1 (propagated on read)
     */
    mv_reg_write(regsBase, 0x20200, 0x00000102);
    /* CPU0 Configuration Register 0x21800
     * bit16: Shared L2 present
     */
    mv_reg_write_field(regsBase, 0x21800, 0x00010000, 0x00000000);

    /*******************/
    /* sram allocation */
    /*******************/

    /* 1. lockdown way i for all masters
     *   L2 CPU Data Lockdown
     *     0x8900, 0x8908   set bit i to 1
     *   L2 CPU Instruction Lockdown
     *     0x8904, 0x890c   set bit i to 1
     *   IO Bridge Lockdown
     *     0x8984 set bit i to 1
     */
    mv_reg_write_field(regsBase, 0x8900, waysMask, waysMask);
    mv_reg_write_field(regsBase, 0x8908, waysMask, waysMask);
    mv_reg_write_field(regsBase, 0x8904, waysMask, waysMask);
    mv_reg_write_field(regsBase, 0x890c, waysMask, waysMask);
    mv_reg_write_field(regsBase, 0x8984, waysMask, waysMask);

    /* 2. Trigger an Allocation Block command
     *   Set <Allocation Way ID> to be i
     *     0x878c bits 4:0 is way (0..32)
     *   Set <Allocation Data> to Disable (0x0)
     *     0x878c bits 7:6
     *   Set <Allocation Atomicity> to Foreground (0x0)
     *     0x878c bit 8
     *   Set <Allocation Base Address> to be saddr[31:10]
     *     0x878c bits 31:10
     */
    for (i = 0; i < waysNo; i++)
    {
        GT_U32 offset = i * waySize;
        if (offset >= sramSize)
            break;
        data = 0;
        data |= i & 0x1f; /* Allocation Way ID bits 4:0 */
        data |= sramPhys + offset; /* bits 31:10 */
        /* Allocation Data == Disable (0) */
        /* Allocation Atomicity == Foreground (0) */
        mv_reg_write(regsBase, 0x878c, data);
    }

    /* L2 Control register: disable L2 */
    mv_reg_write_field(regsBase, 0x8100, 0x00000001, 0);

    /* 3. Configure one the SRAM windows SRAM Window n Control Register (n=0–3)
     * (Table 346 p. 580) to direct the required range to be an SRAM:
     *   0x20240 + n*4
     * Set Base to be saddr[31:16]
     * Set Size to 64KB (0x0)
     *   bits 10:8  0==64K, 1==128K, 3=256K, 7=512K
     * Set WinEn to True
     *   bit 0 to 1
     */
    for (i = 0; i < 4; i++)
    {
        GT_U32 offset = i * _512K;
        if (offset < sramSize)
        {
            data = sramPhys + offset; /* base, bits 31:16 */
            data |= 0x0700; /* bits 10:8 == 7 == 512K */
            data |= 0x1; /* enable */
            mv_reg_write(regsBase, 0x20240+i*4, data);
        } else {
            /* disable */
            mv_reg_write(regsBase, 0x20240+i*4, 0);
        }
    }

    /* Disable window 13 used by BootROM */
    mv_reg_write(regsBase, 0x200b8, 0);

    mv_configure_pex_window_sram(regsBase, sramPhys, sramSize);

    return 0;
}
#endif /* !defined(ASIC_SIMULATION) */

