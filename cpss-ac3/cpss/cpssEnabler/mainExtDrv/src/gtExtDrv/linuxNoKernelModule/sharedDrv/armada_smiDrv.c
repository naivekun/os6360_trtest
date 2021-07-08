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
* @file armada_smiDrv.c
*
* @brief SMI driver for Marvell Armada platform
* Initialization:
* armadaSmiCreateDrv(platform_drv, as, smiMgmtReg)
*
* @version   1
********************************************************************************
*/
#define _BSD_SOURCE
#include <endian.h>
#include <cpss/generic/hwDriver/cpssHwDriverAPI.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>



typedef struct ARMADA_SMI_DRIVER_STCT {
    CPSS_HW_DRIVER_STC  common;
    CPSS_OS_MUTEX       mtx;
    GT_U32              as; /* address space in parent */
    GT_U32              reg; /* SMI management register */
}ARMADA_SMI_DRIVER_STC;

#define SMI_MNG_BUSY            (1<<28)
#define SMI_MNG_RDVALID         (1<<27)
#define SMI_MNG_OPCODE          (1<<26)
#define SMI_MNG_OPCODE_READ     (1<<26)
#define SMI_MNG_OPCODE_WRITE    0
#define SMI_MNG_REG_ADDR(_a)    ((_a)<<21)
#define SMI_MNG_PHY_ADDR(_p)    ((_p)<<16)


static GT_STATUS armadaSmiRead(
    IN  ARMADA_SMI_DRIVER_STC *drv,
    IN  GT_U32  phyAddr,
    IN  GT_U32  regAddr,
    OUT GT_U32 *dataPtr,
    IN  GT_U32  count
)
{
    GT_U32  reg;
    if ((phyAddr >= 32) || (regAddr + count > 32))
        return GT_BAD_PARAM;
    cpssOsMutexLock(drv->mtx);

    for (;count;count--,regAddr++, dataPtr++)
    {
        do /* read reg until Busy is 0 */
        {
            drv->common.parent->read(drv->common.parent, drv->as, drv->reg, &reg, 1);
            /* TODO: timeout,delay??? */
        } while (reg & SMI_MNG_BUSY);
        /* write opcode, regAddr, phyAddr */
        reg = SMI_MNG_OPCODE_READ | SMI_MNG_REG_ADDR(regAddr) | SMI_MNG_PHY_ADDR(phyAddr);
        drv->common.parent->writeMask(drv->common.parent, drv->as, drv->reg, &reg, 1, 0xffffffff);
        do /* read reg until ReadValid is 1 */
        {
            drv->common.parent->read(drv->common.parent, drv->as, drv->reg, &reg, 1);
            /* TODO: timeout,delay??? */
        } while ((reg & SMI_MNG_RDVALID) == 0);

        *dataPtr = reg & 0x0000ffff;
    }
    cpssOsMutexUnlock(drv->mtx);
    return GT_OK;
}

static GT_STATUS armadaSmiWriteMask(
    IN  ARMADA_SMI_DRIVER_STC *drv,
    IN  GT_U32  phyAddr,
    IN  GT_U32  regAddr,
    IN  GT_U32 *dataPtr,
    IN  GT_U32  count,
    IN  GT_U32  mask
)
{
    GT_U32  reg, data;
    if ((phyAddr >= 32) || (regAddr + count > 32))
        return GT_BAD_PARAM;
    cpssOsMutexLock(drv->mtx);
    for (;count;count--,regAddr++, dataPtr++)
    {
        data = *dataPtr;
        if ((mask & 0x0000ffff) != 0x0000ffff)
        {
            armadaSmiRead(drv, phyAddr, regAddr, &data, 1);
            data &= ~mask;
            data |= (*dataPtr) & mask;
        }
        data &= 0x0000ffff;
        /* now write */
        do /* read reg until Busy is 0 */
        {
            drv->common.parent->read(drv->common.parent, drv->as, drv->reg, &reg, 1);
            /* TODO: timeout,delay??? */
        } while (reg & SMI_MNG_BUSY);
        /* write opcode, regAddr, phyAddr */
        reg = SMI_MNG_OPCODE_WRITE | SMI_MNG_REG_ADDR(regAddr) | SMI_MNG_PHY_ADDR(phyAddr) | data;
        drv->common.parent->writeMask(drv->common.parent, drv->as, drv->reg, &reg, 1, 0xffffffff);
    }
    cpssOsMutexUnlock(drv->mtx);
    return GT_OK;
}

static GT_STATUS armadaSmiDestroyDrv(
    IN  ARMADA_SMI_DRIVER_STC *drv
)
{
    cpssOsMutexDelete(drv->mtx);
    cpssOsFree(drv);
    return GT_OK;
}

CPSS_HW_DRIVER_STC* armadaSmiCreateDrv(CPSS_HW_DRIVER_STC *parent, GT_U32 as, GT_U32 smiMgmtReg)
{
    ARMADA_SMI_DRIVER_STC *drv;
    char buf[64];
    if (!smiMgmtReg)
        return NULL;
    drv = cpssOsMalloc(sizeof(*drv));
    if (!drv)
        return NULL;
    cpssOsMemSet(drv,0,sizeof(*drv));
    drv->common.read = (CPSS_HW_DRIVER_METHOD_READ)armadaSmiRead;
    drv->common.writeMask = (CPSS_HW_DRIVER_METHOD_WRITE_MASK)armadaSmiWriteMask;
    drv->common.destroy = (CPSS_HW_DRIVER_METHOD_DESTROY)armadaSmiDestroyDrv;
    drv->common.parent = parent;

    cpssOsSprintf(buf, "smi0x%x", smiMgmtReg);
    cpssOsMutexCreate(buf, &(drv->mtx));
    drv->as = as;
    drv->reg = smiMgmtReg;
    return (CPSS_HW_DRIVER_STC*)drv;
}




