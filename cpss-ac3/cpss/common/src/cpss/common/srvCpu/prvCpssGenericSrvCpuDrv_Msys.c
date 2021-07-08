/*******************************************************************************
*              (c), Copyright 2017, Marvell International Ltd.                 *
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
* @file prvCpssGenericSrvCpuDrv_Msys.c
*
* @brief MSYS Service CPU support
*
* @version   1
********************************************************************************
*/
#include "prvCpssGenericSrvCpuIpcDevCfg.h"

/* forward declarations */
static void mv_configure_pex_window_sram(
    CPSS_HW_DRIVER_STC *drv,
    GT_U32     sramPhys,
    GT_U32     sramSize
);
static GT_STATUS mv_configure_sram(
    CPSS_HW_DRIVER_STC *drv,
    GT_U32     sramPhys,
    GT_U32     sramSize
);

/************************************************************/
/***    Msys support                                      ***/
/************************************************************/
GT_STATUS prvSrvCpuOpsMsys(
    IN  CPSS_HW_INFO_STC    *hwInfo,
    IN  CPSS_HW_DRIVER_STC  *drv,
    IN  GT_U32              scpuId,
    IN  PRV_SRVCPU_OPS_ENT  op,
    OUT void                *ret
)
{
    GT_U32  data, i;
    GT_STATUS rc;
    PRV_SRVCPU_IPC_CHANNEL_STC *s;

    if (scpuId != 0xffffffff)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_SUCH, LOG_ERROR_NO_MSG);
    if (hwInfo->busType == CPSS_HW_INFO_BUS_TYPE_MBUS_E)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_SUCH, LOG_ERROR_NO_MSG);

    switch (op)
    {
        case PRV_SRVCPU_OPS_EXISTS_E:
            return GT_OK;
        case PRV_SRVCPU_OPS_IS_ON_E:
            rc = drv->read(drv, CPSS_HW_DRIVER_AS_CNM_E, 0x20800, &data, 1);
            if (rc != GT_OK)
                return rc;
            *((GT_BOOL*)ret) = (data & 1) ? GT_FALSE : GT_TRUE;
            return GT_OK;
        case PRV_SRVCPU_OPS_PRE_LOAD_E:
            cpssOsPrintf("Configure SRAM...\n");
            /* update hwInfo->sram.phys */
            hwInfo->resource.sram.phys = 0xfff80000;
            rc = mv_configure_sram(drv, 0xfff80000, _512K);
            if (rc != GT_OK)
                return rc;
            s = (PRV_SRVCPU_IPC_CHANNEL_STC*) cpssOsMalloc(sizeof(*s));
            if (!s)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            cpssOsMemSet(s, 0, sizeof(*s));
            s->base = hwInfo->resource.sram.start;
            s->size = _512K;
            s->targetBase = 0xfff80000;
            s->drv = prvSrvCpuRamDirectCreate(s->base, s->targetBase, s->size);
            *((PRV_SRVCPU_IPC_CHANNEL_STC**)ret) = s;
            return GT_OK;
        case PRV_SRVCPU_OPS_RESET_E:
        case PRV_SRVCPU_OPS_UNRESET_E:
            rc = drv->read(drv, CPSS_HW_DRIVER_AS_CNM_E, 0x20800, &data, 1);
            if (rc != GT_OK)
                return rc;
            if (op == PRV_SRVCPU_OPS_RESET_E)
                data |= 1;
            else
                data &= 0xfffffffe;
            return drv->writeMask(drv, CPSS_HW_DRIVER_AS_CNM_E, 0x20800, &data, 1, 0xffffffff);
        case PRV_SRVCPU_OPS_PRE_CHECK_IPC_E:
            s = (PRV_SRVCPU_IPC_CHANNEL_STC*) cpssOsMalloc(sizeof(*s));
            if (!s)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            cpssOsMemSet(s, 0, sizeof(*s));
            s->targetBase = 0;
            s->size = 0;
            for (i = 0; i < 4; i++)
            {
                GT_U32 base, size, data;
                rc = drv->read(drv, CPSS_HW_DRIVER_AS_CNM_E, 0x20240+i*4, &data, 1);
                if (rc != GT_OK)
                    return GT_FALSE;
                if (rc != GT_OK)
                    return GT_FALSE;
                if ((data & 1) == 0)
                    continue;

                base = data & 0xffff0000;
                size = ((data & 0x00000700) << 8) + 0x00010000;
                if (s->size == 0)
                {
                    s->targetBase = base;
                    s->size = size;
                }
                else
                {
                    if (s->targetBase + s->size == base)
                        s->size += size;
                }
            }
            if (s->size == 0)
            {
                cpssOsFree(s);
                return GT_FALSE; /* No SRAM mapping configured */
            }
            /* configure PEX window */
            mv_configure_pex_window_sram(drv, s->targetBase, s->size);
            /* update hwInfo */
            hwInfo->resource.sram.phys = (GT_PHYSICAL_ADDR)s->targetBase;
            s->base = hwInfo->resource.sram.start;
            s->drv = prvSrvCpuRamDirectCreate(s->base, s->targetBase, s->size);
            *((PRV_SRVCPU_IPC_CHANNEL_STC**)ret) = s;
            return GT_OK;
    }
    return GT_OK;
}


/* #define CONFIG_SRAM_DEBUG */
#define BITS(_data,_start,_end) (((_data)>>(_end)) & ((1 << ((_start)-(_end)+1))-1))
static GT_U32 mv_reg_read(CPSS_HW_DRIVER_STC *drv, GT_U32 regAddr)
{
    GT_U32 val;
    if (drv->read(drv, CPSS_HW_DRIVER_AS_CNM_E, regAddr, &val, 1) != GT_OK)
        val = 0;
#ifdef CONFIG_SRAM_DEBUG
    cpssOsPrintf("RD regAddr=0x%08x data=0x%08x\n",regAddr,val);
#endif
    return val;
}
static void mv_reg_write(CPSS_HW_DRIVER_STC *drv, GT_U32 regAddr, GT_U32 value)
{
    drv->writeMask(drv, CPSS_HW_DRIVER_AS_CNM_E, regAddr, &value, 1, 0xffffffff);
#ifdef CONFIG_SRAM_DEBUG
    cpssOsPrintf("WR regAddr=0x%08x data=0x%08x\n",regAddr,value);
#endif
}
static void mv_reg_write_field(CPSS_HW_DRIVER_STC *drv, GT_U32 regAddr, GT_U32 mask, GT_U32 value)
{
    GT_U32 val;
    val = mv_reg_read(drv, regAddr);
    val &= ~mask;
    val |= value;
    mv_reg_write(drv, regAddr, val);
}

/* Configure SRAM window if it not configired yet
 * or confiruration is wrong (mvPpDrv:AC3 only)
 * Set WIN_BASE to PEX BAR4 + 2M
 */
static void mv_configure_pex_window_sram(
    CPSS_HW_DRIVER_STC *drv,
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
    base_val = mv_reg_read(drv, 0x40020) & 0xffff0000;
    base_val += _2M;
    ctl_val = SIZE_TO_BAR_REG(sramSize) |
            (0/*target_id*/ << 4) |
            (0x3e/*attr*/  << 8) |
            (1/*bar2*/ << 1);
    remap_val = sramPhys | 1;
    data_ctl = mv_reg_read(drv, WIN_CTRL);
    data_base = mv_reg_read(drv, WIN_BASE);
    data_remap = mv_reg_read(drv, WIN_REMAP);
    if (   (data_ctl == (ctl_val | 0x1))
        && (data_remap == remap_val)
        && ((data_base & 0xffff0000) == base_val) )
    {
        /* already configured */
        return;
    }
    mv_reg_write(drv, WIN_CTRL, 0);
    mv_reg_write(drv, WIN_BASE, base_val);
    mv_reg_write(drv, WIN_REMAP, remap_val);
    mv_reg_write(drv, WIN_CTRL, ctl_val);
    mv_reg_write(drv, WIN_CTRL, ctl_val|1);
}
static GT_STATUS mv_configure_sram(
    CPSS_HW_DRIVER_STC *drv,
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
    mv_reg_write_field(drv, 0x20280, 0x00001000, 0x00000000);

    /* enable L2 cache and make L2 Auxiliary Control Register enable */
    mv_reg_write_field(drv, 0x8100, 0x00000001, 1);
#ifdef CONFIG_SRAM_DEBUG
    cpssOsPrintf("  Configuring SRAM. base=%p...\n",(void*)drv);

    /* L2 Control Register */
    data = mv_reg_read(drv, 0x8100);
    cpssOsPrintf("L2 Control Register=0x%08x\n", data);
#endif
    /* L2 Auxiliary Control Register */
    data = mv_reg_read(drv, 0x8104);
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
    mv_reg_write(drv, 0x20200, 0x00000102);
    /* CPU0 Configuration Register 0x21800
     * bit16: Shared L2 present
     */
    mv_reg_write_field(drv, 0x21800, 0x00010000, 0x00000000);

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
    mv_reg_write_field(drv, 0x8900, waysMask, waysMask);
    mv_reg_write_field(drv, 0x8908, waysMask, waysMask);
    mv_reg_write_field(drv, 0x8904, waysMask, waysMask);
    mv_reg_write_field(drv, 0x890c, waysMask, waysMask);
    mv_reg_write_field(drv, 0x8984, waysMask, waysMask);

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
        mv_reg_write(drv, 0x878c, data);
    }

    /* L2 Control register: disable L2 */
    mv_reg_write_field(drv, 0x8100, 0x00000001, 0);

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
            mv_reg_write(drv, 0x20240+i*4, data);
        } else {
            /* disable */
            mv_reg_write(drv, 0x20240+i*4, 0);
        }
    }

    /* Disable window 13 used by BootROM */
    mv_reg_write(drv, 0x200b8, 0);

    mv_configure_pex_window_sram(drv, sramPhys, sramSize);

    return 0;
}
