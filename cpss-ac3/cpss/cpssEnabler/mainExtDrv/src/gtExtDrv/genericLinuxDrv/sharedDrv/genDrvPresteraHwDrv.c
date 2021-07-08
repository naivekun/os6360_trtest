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
* @file genDrvPresteraHwDrv.c
*
* @brief Prestera driver Hardware read and write functions implementation.
*
* @version   1
********************************************************************************
*/
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/hwDriver/cpssHwDriverAPI.h>
#include <cpss/generic/cpssHwInfo.h>
#include <sys/ioctl.h>
#include "../kerneldrv/include/presteraPpDriverGlob.h"

/* TODO: file descriptor type is int (POSIX) */
extern GT_32 gtPpFd;

typedef struct CPSS_HW_DRIVER_PCIKERN_STCT {
    CPSS_HW_DRIVER_STC  common;
    GT_U32              driverId;
} CPSS_HW_DRIVER_PCIKERN_STC;

/*******************************************************************************
* Intenal Macros and definitions
*******************************************************************************/
#define HWCTRL(_devNum,_portGroupId) \
    (prvCpssDrvPpConfig[_devNum]->hwCtrl[_portGroupId])

#define PP_DRIVER_IO_OP(_op,_reg,_length,_dataPtr) \
    struct mvPpDrvDriverIo_STC io; \
    io.id = drv->driverId; \
    io.op = mvPpDrvDriverIoOps_ ## _op ## _E; \
    io.regAddr = _reg; \
    io.length = _length; \
    io.dataPtr = (mv_kmod_uintptr_t)((uintptr_t)_dataPtr); \
    if (prestera_ctl(PRESTERA_PP_DRIVER_IO, &io)) \
    { \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); \
    } \
    return GT_OK;

static GT_STATUS prvDoRead32(
    IN  CPSS_HW_DRIVER_PCIKERN_STC *drv,
    IN  GT_U32  addrSpace,
    IN  GT_U32  regAddr,
    OUT GT_U32 *dataPtr
)
{
    struct mvPpDrvDriverIo_STC io;
    switch (addrSpace)
    {
        case CPSS_HW_DRIVER_AS_SWITCHING_E:
            io.op = mvPpDrvDriverIoOps_PpRegRead_E;
            break;
        case CPSS_HW_DRIVER_AS_CNM_E:
            io.op = mvPpDrvDriverIoOps_PciRegRead_E;
            break;
        case CPSS_HW_DRIVER_AS_DFX_E:
            io.op = mvPpDrvDriverIoOps_DfxRegRead_E;
            break;
        default:
            return GT_BAD_PARAM;
    }
    io.id = drv->driverId;
    io.regAddr = regAddr;
    io.length = 1;
    io.dataPtr = (mv_kmod_uintptr_t)((uintptr_t)dataPtr);
    if (prestera_ctl(PRESTERA_PP_DRIVER_IO, &io))
    {
        return GT_FAIL;
    }
    return GT_OK;
}

static GT_STATUS prvDoWrite32(
    IN  CPSS_HW_DRIVER_PCIKERN_STC *drv,
    IN  GT_U32  addrSpace,
    IN  GT_U32  regAddr,
    IN  GT_U32  data
)
{
    struct mvPpDrvDriverIo_STC io;
    switch (addrSpace)
    {
        case CPSS_HW_DRIVER_AS_SWITCHING_E:
            io.op = mvPpDrvDriverIoOps_PpRegWrite_E;
            break;
        case CPSS_HW_DRIVER_AS_CNM_E:
            io.op = mvPpDrvDriverIoOps_PciRegWrite_E;
            break;
        case CPSS_HW_DRIVER_AS_DFX_E:
            io.op = mvPpDrvDriverIoOps_DfxRegWrite_E;
            break;
        default:
            return GT_BAD_PARAM;
    }
    io.id = drv->driverId;
    io.regAddr = regAddr;
    io.length = 1;
    io.dataPtr = (mv_kmod_uintptr_t)((uintptr_t)&data);
    if (prestera_ctl(PRESTERA_PP_DRIVER_IO, &io))
    {
        return GT_FAIL;
    }
    return GT_OK;
}

static GT_STATUS cpssHwDriverPciKernRead(
    IN  CPSS_HW_DRIVER_PCIKERN_STC *drv,
    IN  GT_U32  addrSpace,
    IN  GT_U32  regAddr,
    OUT GT_U32 *dataPtr,
    IN  GT_U32  count
)
{
    GT_U32 i;
    GT_STATUS rc;
    if (addrSpace == CPSS_HW_DRIVER_AS_SWITCHING_E)
    {
        struct mvPpDrvDriverIo_STC io;
        io.id = drv->driverId;
        io.op = mvPpDrvDriverIoOps_RamRead_E;
        io.regAddr = regAddr;
        io.length = count;
        io.dataPtr = (mv_kmod_uintptr_t)((uintptr_t)dataPtr);
        if (prestera_ctl(PRESTERA_PP_DRIVER_IO, &io))
        {
            return GT_FAIL;
        }
        return GT_OK;
    }
    for (i = 0; i < count; i++, regAddr += 4, dataPtr++)
    {
        rc = prvDoRead32(drv, addrSpace, regAddr, dataPtr);
        if (rc != GT_OK)
            return rc;
    }
    return GT_OK;
}

static GT_STATUS cpssHwDriverPciKernWriteMask(
    IN  CPSS_HW_DRIVER_PCIKERN_STC *drv,
    IN  GT_U32  addrSpace GT_UNUSED,
    IN  GT_U32  regAddr,
    IN  GT_U32 *dataPtr,
    IN  GT_U32  count,
    IN  GT_U32  mask
)
{
    GT_STATUS rc;
    GT_U32  i, data = 0;
    if (mask == 0xffffffff)
    {
        if (addrSpace == CPSS_HW_DRIVER_AS_SWITCHING_E)
        {
            struct mvPpDrvDriverIo_STC io;
            io.id = drv->driverId;
            io.op = mvPpDrvDriverIoOps_RamWrite_E;
            io.regAddr = regAddr;
            io.length = count;
            io.dataPtr = (mv_kmod_uintptr_t)((uintptr_t)dataPtr);
            if (prestera_ctl(PRESTERA_PP_DRIVER_IO, &io))
            {
                return GT_FAIL;
            }
            return GT_OK;
        }
        for (i = 0; i < count; i++, regAddr += 4, dataPtr++)
        {
            rc = prvDoWrite32(drv, addrSpace, regAddr, *dataPtr);
            if (rc != GT_OK)
                return rc;
        }
        return GT_OK;
    }
    /* mask != 0xffffffff */
    for (i = 0; i < count; i++, regAddr += 4, dataPtr++)
    {
        rc = prvDoRead32(drv, addrSpace, regAddr, &data);
        if (rc != GT_OK)
            return rc;
        data &= ~mask;
        data |= (*dataPtr) & mask;
        rc = prvDoWrite32(drv, addrSpace, regAddr, data);
        if (rc != GT_OK)
            return rc;
    }

    return GT_OK;
}

static GT_STATUS cpssHwDriverPciKernDestroy(
    IN  CPSS_HW_DRIVER_PCIKERN_STC *drv
)
{
    cpssOsFree(drv);
    return GT_OK;
}

/*******************************************************************************
* cpssExtDrvPresteraKernDrvCreate
*
* DESCRIPTION:
*       Implemented in kernel module, no memory maping
*       appAddrCompletionRegionsBmp:
*           0x0f  - use 4 regions address completion (64M virtual addresses)
*           0x03  - use 2 regions address completion (32M virtual addresses)
*           0x3c  - use new 8 regions address completion (4M virtual addresses)
*
*
* INPUTS:
*       hwInfo      - Hardware info (PCI address)
*
* OUTPUTS:
*       drvPtr   - pointer
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error.
*
* COMMENTS:
*
*******************************************************************************/
CPSS_HW_DRIVER_STC* cpssExtDrvPresteraKernDrvCreate
(
    IN  CPSS_HW_INFO_STC            *hwInfo,
    IN  GT_U32                      appAddrCompletionRegionsBmp
)
{
    CPSS_HW_DRIVER_PCIKERN_STC *drv;
    struct mvPpDrvDriverOpen_STC inf;

    inf.busNo = hwInfo->hwAddr.busNo;
    inf.devSel = hwInfo->hwAddr.devSel;
    inf.funcNo = hwInfo->hwAddr.funcNo;
    switch (appAddrCompletionRegionsBmp)
    {
        case 0x0f:
            inf.type = mvPpDrvDriverType_Pci_E;
            break;
        case 0x03:
            inf.type = mvPpDrvDriverType_PciHalf_E;
            break;
        case 0x3c:
            inf.type = mvPpDrvDriverType_PexMbus_E;
            break;
        default:
            return NULL;
    }
    if (prestera_ctl(PRESTERA_PP_DRIVER_OPEN,&inf))
    {
        /* failed */
        return NULL;
    }

    drv = cpssOsMalloc(sizeof(*drv));
    if (drv == NULL)
        return NULL;
    cpssOsMemSet(drv,0,sizeof(*drv));
    drv->common.read = (CPSS_HW_DRIVER_METHOD_READ)cpssHwDriverPciKernRead;
    drv->common.writeMask = (CPSS_HW_DRIVER_METHOD_WRITE_MASK)cpssHwDriverPciKernWriteMask;
    drv->common.destroy = (CPSS_HW_DRIVER_METHOD_DESTROY)cpssHwDriverPciKernDestroy;

    drv->driverId = inf.id;

    return (CPSS_HW_DRIVER_STC*)drv;
}




