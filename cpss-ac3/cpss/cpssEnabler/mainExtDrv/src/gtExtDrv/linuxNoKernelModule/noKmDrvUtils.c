/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file noKmDrvUtils.c
*
* @brief misc utils
*
* @version   1
********************************************************************************
*/

#define _BSD_SOURCE
#define _POSIX_SOURCE
#define _DEFAULT_SOURCE

#include <gtExtDrv/drivers/gtDmaDrv.h>
#include "prvNoKmDrv.h"
#include <cpssDriver/pp/hardware/prvCpssDrvHwPpPortGroupCntl.h>
/* definitions collide */
#undef _4K
#undef _64K
#undef _256K
#undef _512K
#undef _1M
#undef _2M
#undef _4M
#undef _8M
#undef _16M
#undef _64M
#ifdef CHX_FAMILY
#include <cpss/generic/cpssHwInit/private/prvCpssHwInit.h>
#endif /*CHX_FAMILY*/
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>

/***************************************************/
/*   register read/write definitions               */
/***************************************************/
GT_U32 prvNoKm_reg_read(GT_UINTPTR regsBase, GT_U32 regAddr)
{
    GT_U32 val = *((volatile GT_U32*)(regsBase+regAddr));
#ifdef NOKMDRV_DEBUG
    fprintf(stderr, "RD base=%p regAddr=0x%08x data=0x%08x\n",(void*)regsBase,regAddr,val);
#endif
    return le32toh(val);
}
void prvNoKm_reg_write(GT_UINTPTR regsBase, GT_U32 regAddr, GT_U32 value)
{
    *((volatile GT_U32*)(regsBase+regAddr)) = htole32(value);
#ifdef NOKMDRV_DEBUG
    fprintf(stderr, "WR base=%p regAddr=0x%08x data=0x%08x\n",(void*)regsBase,regAddr,value);
#endif
}
void prvNoKm_reg_write_field(GT_UINTPTR regsBase, GT_U32 regAddr, GT_U32 mask, GT_U32 value)
{
    GT_U32 val;
    val = prvNoKm_reg_read(regsBase, regAddr);
    val &= ~mask;
    val |= (value & mask);
    prvNoKm_reg_write(regsBase, regAddr, val);
}

/*******************************************************************************
* prvNoKmDrv_configure_dma
*   Configure DMA for PP
*
*******************************************************************************/
GT_STATUS prvNoKmDrv_configure_dma(
    IN GT_UINTPTR   regsBase,
    IN int          targetIsMbus
)
{
    GT_U32     attr, target, dmaSize, win;
    GT_UINTPTR dmaBase;

    if (extDrvGetDmaBase(&dmaBase) != GT_OK)
    {
        printf("extDrvGetDmaBase() failed, dma not configured\n");
        return GT_FAIL;
    }

    if (dmaBase == 0L)
        return GT_FAIL;
#ifdef NOKMDRV_DEBUG
    fprintf(stderr, "prvExtDrvDmaPtr=%p\n", (void*)dmaBase);
#endif
    extDrvGetDmaSize(&dmaSize);
    if ((dmaBase & 0x000fffff) != 0)
    {
        fprintf(stderr, "The DMA base address %p must be at least 1M aligned\n", (void*)dmaBase);
        return GT_FAIL;
    }
    if ((dmaSize & 0x000fffff) != 0)
    {
        fprintf(stderr, "The DMA size 0x%x must be at least 1M aligned\n", dmaSize);
        return GT_FAIL;
    }

    if (targetIsMbus)
    {
        attr    = 0x00003e00;
        target  = 0x000003e0;
    }
    else
    {
        attr    = 0x00000e04;
        target  = 0x000000e4;
    }

    /* Set Unit Default ID (UDID) Register (0x00000204)
     * DATTR(11:4) = 0x000000e0 == identical to base address reg attr
     * DIDR(3:0)   = 0x00000004 == Target Unit ID PCIe
     */
    prvNoKm_reg_write(regsBase, 0x00000204, target);

    for (win = 0; win < 6 && dmaSize; win++)
    {
        GT_U32 size_mask, size_next;
        /* Calculate the BLOCK_SIZE==2^n where
         * baseAddr is BLOCK_SIZE aligned
         * and
         * dmaSize <= BLOCK_SIZE
         */
        size_mask = 0x00000000;
        while ((dmaBase & size_mask) == 0)
        {
            size_next = (size_mask<<1) | 0x00010000;
            if ((dmaBase & size_next) || size_next > dmaSize)
                break;
            size_mask = size_next;
            if (size_mask == 0xffff0000)
                break;
        }
        /* Configure DMA base in Base Address n Register (0x0000020c+n*8)
         * Attr(15:8)  = 0x00000e00 == target specific attr
         * Target(3:0) = 0x00000004 == target resource == PCIe
         */
        prvNoKm_reg_write(regsBase, 0x0000020c+win*8, (((GT_U32)dmaBase) & 0xffff0000) | attr);
        /* write dmaBase[63:32] to register 0x23c+n*4 (high address remap n) */
        prvNoKm_reg_write(regsBase, 0x0000023c+win*4, ((GT_U32)(prvExtDrvDmaPhys64 >> 32)));

        /* Configure size n register (0x00000210+n*8)
         * Set max size
         */
        prvNoKm_reg_write(regsBase, 0x00000210+win*8, size_mask);

        /* Window Control register n (0x00000254+n*4)
         * WinApn(2:1)    = 0x00000006 == RW
         * BARenable(0:0) = 0x00000000 == Enable
         */
        prvNoKm_reg_write(regsBase, 0x00000254+win*4, 0x00000006);

        if (size_mask == 0xffff0000)
            break;
        dmaBase += (size_mask + 0x10000); /* += BLOCK_SIZE */
        dmaSize -= (size_mask + 0x10000); /* -= BLOCK_SIZE */
    }

    /*  Debug dma reg - according to old code in
     *  arch/arm/mach-armadaxp/pss/hwServices.c
     *
     */
    prvNoKm_reg_write(regsBase, 0x2684, 0xaaba);

    return GT_OK;
}

/* PEX ATU (Address Translation Unit) registers */
#define ATU_REGISTERS_OFFSET_IN_BAR0  0x1200
#define ATU_REGION_CTRL_1_REG         0x0
#define ATU_REGION_CTRL_2_REG         0x4
#define ATU_LOWER_BASE_ADDRESS_REG    0x8
#define ATU_UPPER_BASE_ADDRESS_REG    0xC
#define ATU_LIMIT_ADDRESS_REG         0x10
#define ATU_LOWER_TARGET_ADDRESS_REG  0x14
#define ATU_UPPER_TARGET_ADDRESS_REG  0x18

#define oATU_REGISTER_ADDRESS(_register)  \
    (ATU_REGISTERS_OFFSET_IN_BAR0 + (_register))

GT_STATUS prvNoKmDrv_init_dma_pp_to_host_cpu(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId
)
{
    GT_STATUS rc;
    CPSS_HW_DRIVER_STC *drv;
    GT_UINTPTR dmaBase;
    GT_U32 data;

    drv = prvCpssDrvPpConfig[devNum]->drivers[portGroupId];
    if( NULL == drv )
    {
        fprintf(stderr, "HW Driver not initialized\n");
        return GT_FAIL;
    }

    if (extDrvGetDmaBase(&dmaBase) != GT_OK)
    {
        printf("extDrvGetDmaBase() failed, dma not configured\n");
        return GT_FAIL;
    }

    if (dmaBase == 0L)
        return GT_FAIL;

    data = 0x80000000;
    rc = drv->writeMask(drv, CPSS_HW_DRIVER_AS_ATU_E,
                        oATU_REGISTER_ADDRESS(ATU_LOWER_BASE_ADDRESS_REG),
                        &data, 1, 0xffffffff);
    if( GT_OK != rc )
    {
        return rc;
    }

    data = 0x0;
    rc = drv->writeMask(drv, CPSS_HW_DRIVER_AS_ATU_E,
                        oATU_REGISTER_ADDRESS(ATU_UPPER_BASE_ADDRESS_REG),
                        &data, 1, 0xffffffff);
    if( GT_OK != rc )
    {
        return rc;
    }

    data = 0xffffffff;
    rc = drv->writeMask(drv, CPSS_HW_DRIVER_AS_ATU_E,
                        oATU_REGISTER_ADDRESS(ATU_LIMIT_ADDRESS_REG),
                        &data, 1, 0xffffffff);
    if( GT_OK != rc )
    {
        return rc;
    }

    data = dmaBase & 0x80000000;
    rc = drv->writeMask(drv, CPSS_HW_DRIVER_AS_ATU_E,
                        oATU_REGISTER_ADDRESS(ATU_LOWER_TARGET_ADDRESS_REG),
                        &data, 1, 0xffffffff);
    if( GT_OK != rc )
    {
        return rc;
    }

    data = (GT_U32)(prvExtDrvDmaPhys64 >> 32);
    rc = drv->writeMask(drv, CPSS_HW_DRIVER_AS_ATU_E,
                        oATU_REGISTER_ADDRESS(ATU_UPPER_TARGET_ADDRESS_REG),
                        &data, 1, 0xffffffff);
    if( GT_OK != rc )
    {
        return rc;
    }

    data = 0x0;
    rc = drv->writeMask(drv, CPSS_HW_DRIVER_AS_ATU_E,
                        oATU_REGISTER_ADDRESS(ATU_REGION_CTRL_1_REG),
                        &data, 1, 0xffffffff);
    if( GT_OK != rc )
    {
        return rc;
    }

    data = 0x80000000;
    rc = drv->writeMask(drv, CPSS_HW_DRIVER_AS_ATU_E,
                        oATU_REGISTER_ADDRESS(ATU_REGION_CTRL_2_REG),
                        &data, 1, 0xffffffff);
    if( GT_OK != rc )
    {
        return rc;
    }

    return GT_OK;
}

/*******************************************************************************
* prvNoKmDrv_configure_dma_per_devNum
*   Configure DMA for PP , per device using it's 'cpssDriver' to write the
*   registers needed in Falcon that the cpssDriver is complex
*
*******************************************************************************/
GT_STATUS prvNoKmDrv_configure_dma_per_devNum(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId
)
{
    GT_STATUS   rc;
    GT_U32     dmaSize, win;
    GT_UINTPTR dmaBase;
    GT_U32  mgOffset;
    GT_U32  ambMgCnmNicId = 3;/* (from the CnM Subsystem figure) 'leg' #3 of the CnM NIC connected to <AMB MG> */
    GT_U32  defaultAttribute = 0xE;/* 0xE is 'default' attribute from register 0x00000204 */
    GT_U32  BaseAddressValue;
    GT_U32  UnitDefaultIDValue;
    GT_U32  WindowControlValue;

    if (extDrvGetDmaBase(&dmaBase) != GT_OK)
    {
        printf("extDrvGetDmaBase() failed, dma not configured\n");
        return GT_FAIL;
    }

    printf("prvNoKmDrv_configure_dma_per_devNum : dmaBase[0x%x]\n",(GT_U32)dmaBase);

    if (dmaBase == 0L)
        return GT_FAIL;
#ifdef NOKMDRV_DEBUG
    fprintf(stderr, "prvExtDrvDmaPtr=%p\n", (void*)dmaBase);
#endif
    extDrvGetDmaSize(&dmaSize);

    printf("prvNoKmDrv_configure_dma_per_devNum : dmaSize[0x%x]\n",dmaSize);

    if ((dmaBase & 0x000fffff) != 0)
    {
        fprintf(stderr, "The DMA base address %p must be at least 1M aligned\n", (void*)dmaBase);
        return GT_FAIL;
    }
    if ((dmaSize & 0x000fffff) != 0)
    {
        fprintf(stderr, "The DMA size 0x%x must be at least 1M aligned\n", dmaSize);
        return GT_FAIL;
    }

    if(dmaSize > 0x80000000)
    {
        fprintf(stderr, "The DMA size 0x%x must NOT be more than '2GB' (the device not supports more) \n",
            dmaSize);
        return GT_FAIL;
    }

    rc = prvNoKmDrv_init_dma_pp_to_host_cpu(devNum,portGroupId);
    if( GT_OK != rc )
    {
        return rc;
    }

#ifdef CHX_FAMILY
    mgOffset = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_MG_E,NULL);
#else /*! CHX_FAMILY*/
    mgOffset = 0;
#endif/*! CHX_FAMILY*/

    {
        CPSS_HW_DRIVER_STC *drv;
        GT_U32  regValue;
        GT_U32  PCIExpressCommandAndStatusRegisterAddr = 0x4;

        drv = prvCpssDrvPpConfig[devNum]->drivers[portGroupId];
        if( NULL == drv )
        {
            fprintf(stderr, "HW Driver not initialized\n");
            return GT_FAIL;
        }

        /* Enable PCIe bus mastering.
         * Should be enabled for SDMA
         * PP will generate master transactions as End Point
         *
         * PCI Express Command and Status Register
         * 0x40004
         * Bit 2: Master Enable. This bit controls the ability of the device
         *        to act as a master on the PCI Express port.
         *        When set to 0, no memory or I/O read/write request packets
         *        are generated to PCI Express.
         */
        rc = drv->read(drv,
                       CPSS_HW_DRIVER_AS_PCI_CONFIG_E,
                       PCIExpressCommandAndStatusRegisterAddr,
                       &regValue,
                       1);
        if(rc != GT_OK)
        {
            return rc;
        }
        regValue |= 0x4;/* <Master Enable> - bit 2 */
        rc = drv->writeMask(drv,
                            CPSS_HW_DRIVER_AS_PCI_CONFIG_E,
                            PCIExpressCommandAndStatusRegisterAddr,
                            &regValue,
                            1, 0xffffffff);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    BaseAddressValue    = defaultAttribute << 8/*Attr*/               | ambMgCnmNicId/*<Target >*/;
    UnitDefaultIDValue  = defaultAttribute << 4/*Default Attributes*/ | ambMgCnmNicId/*<Default ID >*/;

    /* Set Unit Default ID (UDID) Register (0x00000204)
     * DATTR(11:4) = 0x000000e0 == identical to base address reg attr
     * DIDR(3:0)   = 0x00000003 == Target Unit ID PCIe
     */
    rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum,portGroupId, mgOffset + 0x00000204, UnitDefaultIDValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Window Control register n (0x00000254+n*4)
     * WinApn(2:1)    = 0x00000006 == RW
     * BARenable(0:0) = 0x00000000 == Enable
        need to set bits 3 <Window Remap Enable> value 0x1
        and in bits 16..31 <Window Remap Value> value 0x8000
     */
    /* we need the remap because the CnM subsystem need the address to be in range of :
        0x80000000 - 0xFFFFFFFF

        the 'map back' is done in BAR0 in the OATU (outband ATU) windows
        (direction from the CnM to the Host CPU).
    */
    WindowControlValue =  (3 << 1) |/*WinApn(2:1) - allow read/write access */
                          (1 << 3) |/*<Window Remap Enable>*/
                          (0x8000 << 16); /*<Window Remap Value>*/


    for (win = 0; win < 6 && dmaSize; win++)
    {
        GT_U32 size_mask, size_next;
        /* Calculate the BLOCK_SIZE==2^n where
         * baseAddr is BLOCK_SIZE aligned
         * and
         * dmaSize <= BLOCK_SIZE
         */
        size_mask = 0x00000000;
        while ((dmaBase & size_mask) == 0)
        {
            size_next = (size_mask<<1) | 0x00010000;
            if ((dmaBase & size_next) || size_next > dmaSize)
                break;
            size_mask = size_next;
            if (size_mask == 0xffff0000)
                break;
        }

        printf("prvNoKmDrv_configure_dma_per_devNum : size_mask[0x%x]\n",size_mask);
        printf("prvNoKmDrv_configure_dma_per_devNum : (GT_U32)(prvExtDrvDmaPhys64 >> 32)[0x%x]\n",(GT_U32)(prvExtDrvDmaPhys64 >> 32));

        /* Configure DMA base in Base Address n Register (0x0000020c+n*8)
         * Attr(15:8)  = 0x00000e00 == target specific attr
         * Target(3:0) = 0x00000003 == target resource == PCIe
         */
        rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum,portGroupId, mgOffset + 0x0000020c+win*8, (((GT_U32)dmaBase) & 0xffff0000) | BaseAddressValue);
        if(rc != GT_OK)
        {
            return rc;
        }

#if 0
        /*
            Bits 63:32 of the MG don't go out anymore.
            We now have a 32bit addressing support within Falcon and relayed on the host remapping capabilities (anyway needed in >32bit hosts).
            These upper bits have to be set by the remapping window
        */

        /* write dmaBase[63:32] to register 0x23c+n*4 (high address remap n) */
        rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum,portGroupId, mgOffset + 0x0000023c+win*4, ((GT_U32)(prvExtDrvDmaPhys64 >> 32)));
        if(rc != GT_OK)
        {
            return rc;
        }
#endif /* 0 */
        /* Configure size n register (0x00000210+n*8)
         * Set max size
         */
        rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum,portGroupId, mgOffset + 0x00000210+win*8, size_mask);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* Window Control register n (0x00000254+n*4)
         * WinApn(2:1)    = 0x00000006 == RW
         * BARenable(0:0) = 0x00000000 == Enable
            need to set bits 3 <Window Remap Enable> value 0x1
            and in bits 16..31 <Window Remap Value> value 0x8000
         */

        /* clear bits 16..30 */
        WindowControlValue &= (~0x7FFF0000);
        /* OR the bits 16..31 (remember that bit 31 is set at WindowControlValue) */
        WindowControlValue |= (((GT_U32)dmaBase) & 0xffff0000);

        printf("prvNoKmDrv_configure_dma_per_devNum : WindowControlValue[0x%x]\n",WindowControlValue);

        rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum,portGroupId, mgOffset + 0x00000254+win*4, WindowControlValue);
        if(rc != GT_OK)
        {
            return rc;
        }

        if (size_mask == 0xffff0000)
            break;
        dmaBase += (size_mask + 0x10000); /* += BLOCK_SIZE */
        dmaSize -= (size_mask + 0x10000); /* -= BLOCK_SIZE */
    }

    /* the register doesn't exist in Falcon
        prvCpssDrvHwPpPortGroupWriteRegister(devNum,portGroupId, mgOffset + 0x2684, 0xaaba);
    */

    return GT_OK;
}


/* debug function to print physical memory (according to value written in the register)*/
GT_STATUS prvNoKmDrv_debug_print_physical_memory(
    IN GT_U32   phyAddr_0_31,
    IN GT_U32   numOfWords
)
{
    GT_U32  ii,offset_from_startDma;
    uint64_t phyAddr_64 = (uint64_t)phyAddr_0_31;
    uint64_t *memPtr;

    printf("physical prvExtDrvDmaPhys  [%p] \n"    ,prvExtDrvDmaPhys  );
    printf("physical prvExtDrvDmaPhys64[0x%llx] \n",prvExtDrvDmaPhys64);
    printf("VIRTUAL  prvExtDrvDmaPtr   [%p] \n"    ,prvExtDrvDmaPtr   );

    if(phyAddr_64 < prvExtDrvDmaPhys)
    {
        printf("phyAddr_0_31 [0x%x] must be more than prvExtDrvDmaPhys\n"    ,phyAddr_0_31   );
        return GT_BAD_PARAM;
    }

    if(phyAddr_64 >= (prvExtDrvDmaPhys + prvExtDrvDmaLen))
    {
        printf("phyAddr_0_31 [0x%x] must be less than prvExtDrvDmaPhys + 'length'[0x%x] bytes\n"    ,phyAddr_0_31 ,prvExtDrvDmaLen  );
        return GT_BAD_PARAM;
    }

    offset_from_startDma = (phyAddr_64 - prvExtDrvDmaPhys);
    printf("offset_from_startDma [0x%x] \n"    ,offset_from_startDma   );

    memPtr = (uint64_t*)((GT_U8*)prvExtDrvDmaPtr + offset_from_startDma);

    for(ii = 0 ; ii < numOfWords/2; ii++,memPtr++)
    {
        printf("word[%2.2d] (low)  value[0x%8.8x] \n",
            ii*2,(GT_U32)(*memPtr));
        printf("word[%2.2d] (high) value[0x%8.8x] \n",
            1 + (ii*2),(GT_U32)((*memPtr) >> 32));
    }

    return GT_OK;
}


