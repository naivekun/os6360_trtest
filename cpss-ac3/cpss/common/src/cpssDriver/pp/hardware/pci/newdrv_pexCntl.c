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
* @file newdrv_pexMbusCntl.c
*
* @brief Driver for PEX/PEXMBUS connected PP
*       Supports both legacy and 8-region address completion
* Resources are mapped to user-space
*
* @version   1
********************************************************************************
*/
#include <cpssDriver/pp/hardware/private/prvCpssDriverCreate.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/hwDriver/cpssHwDriverGeneric.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/log/prvCpssLog.h>


char msg1[] = "Address space greater than 16 [%d] \n";
char msg2[] = "Address space [%d] is not initialized \n";

typedef struct CPSS_HW_DRIVER_PEX_ALL_STCT {
    CPSS_HW_DRIVER_STC  common;
    CPSS_HW_DRIVER_STC* as[16];
} CPSS_HW_DRIVER_PEX_ALL_STC;

static GT_STATUS cpssHwDriverPexAllRead(
    IN  CPSS_HW_DRIVER_PEX_ALL_STC *drv,
    IN  GT_U32  addrSpace,
    IN  GT_U32  regAddr,
    OUT GT_U32 *dataPtr,
    IN  GT_U32  count
)
{
    if (addrSpace >= 16)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, msg1, addrSpace);
    if (drv->as[addrSpace] == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, msg2, addrSpace);
    return drv->as[addrSpace]->read(drv->as[addrSpace], 0, regAddr, dataPtr, count);
}

static GT_STATUS cpssHwDriverPexAllWriteMask(
    IN  CPSS_HW_DRIVER_PEX_ALL_STC *drv,
    IN  GT_U32  addrSpace GT_UNUSED,
    IN  GT_U32  regAddr,
    IN  GT_U32 *dataPtr,
    IN  GT_U32  count,
    IN  GT_U32  mask
)
{
    if (addrSpace >= 16)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, msg1, addrSpace);
    if (drv->as[addrSpace] == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, msg2, addrSpace);
    return drv->as[addrSpace]->writeMask(drv->as[addrSpace], 0, regAddr, dataPtr, count, mask);
}

static GT_STATUS cpssHwDriverPexAllDestroy(
    IN  CPSS_HW_DRIVER_PEX_ALL_STC *drv
)
{
    GT_U32 i;
    for (i = 0; i < 16; i++)
    {
        if (drv->as[i] != 0)
            cpssHwDriverDestroy(drv->as[i]);
    }
    cpssOsFree(drv);
    return GT_OK;
}

/**
* @internal cpssHwDriverPexCreateDrv function
* @endinternal
*
* @brief Create driver instance for PEX connected PP
*        (legacy 4-region address completion)
*        Resources are mapped to user-space
*
* @param[in] hwInfo             - HW info
*
* @retval CPSS_HW_DRIVER_STC* or NULL
*/
CPSS_HW_DRIVER_STC *cpssHwDriverPexCreateDrv(
    IN  CPSS_HW_INFO_STC    *hwInfo
)
{
    CPSS_HW_DRIVER_PEX_ALL_STC *drv;

    drv = cpssOsMalloc(sizeof(*drv));
    if (drv == NULL)
        return NULL;
    cpssOsMemSet(drv,0,sizeof(*drv));
    drv->common.read = (CPSS_HW_DRIVER_METHOD_READ)cpssHwDriverPexAllRead;
    drv->common.writeMask = (CPSS_HW_DRIVER_METHOD_WRITE_MASK)cpssHwDriverPexAllWriteMask;
    drv->common.destroy = (CPSS_HW_DRIVER_METHOD_DESTROY)cpssHwDriverPexAllDestroy;

    if (hwInfo->resource.cnm.start)
        drv->as[CPSS_HW_DRIVER_AS_CNM_E] = cpssHwDriverGenMmapCreateDrv(
            hwInfo->resource.cnm.start,
            hwInfo->resource.cnm.size);

    if (hwInfo->resource.switching.start)
    {
        GT_U32 data = 0x00010000;
        CPSS_HW_DRIVER_STC *d = cpssHwDriverGenMmapAc4CreateDrv(
                        hwInfo->resource.switching.start);
        drv->as[CPSS_HW_DRIVER_AS_SWITCHING_E] = d;
        d->writeMask(d, 0, 0x140, &data, 1, 0x00010000);
    }

    if (hwInfo->resource.resetAndInitController.start)
        drv->as[CPSS_HW_DRIVER_AS_RESET_AND_INIT_CONTROLLER_E] =
                cpssHwDriverGenMmapCreateDrv(
                        hwInfo->resource.resetAndInitController.start,
                        hwInfo->resource.resetAndInitController.size);

    return (CPSS_HW_DRIVER_STC*)drv;
}


/**
* @internal cpssHwDriverPexMbusCreateDrv function
* @endinternal
*
* @brief Create driver instance for eArch PP
*        Resources are mapped to user-space
*
* @param[in] hwInfo             - HW info
* @param[in] compRegionMask     - the bitmap of address completion regions to use
*
* @retval CPSS_HW_DRIVER_STC* or NULL
*/
CPSS_HW_DRIVER_STC *cpssHwDriverPexMbusCreateDrv(
    IN  CPSS_HW_INFO_STC    *hwInfo,
    IN  GT_U32      compRegionMask
)
{
    CPSS_HW_DRIVER_PEX_ALL_STC *drv;

    if ((compRegionMask & 0xfe) == 0)
        return NULL;

    drv = cpssOsMalloc(sizeof(*drv));
    if (drv == NULL)
        return NULL;
    cpssOsMemSet(drv,0,sizeof(*drv));
    drv->common.read = (CPSS_HW_DRIVER_METHOD_READ)cpssHwDriverPexAllRead;
    drv->common.writeMask = (CPSS_HW_DRIVER_METHOD_WRITE_MASK)cpssHwDriverPexAllWriteMask;
    drv->common.destroy = (CPSS_HW_DRIVER_METHOD_DESTROY)cpssHwDriverPexAllDestroy;

    if (hwInfo->resource.cnm.start)
        drv->as[CPSS_HW_DRIVER_AS_CNM_E] = cpssHwDriverGenMmapCreateDrv(
            hwInfo->resource.cnm.start,
            hwInfo->resource.cnm.size);

    if (hwInfo->resource.switching.start)
    {
        GT_U32 data = 0;
        CPSS_HW_DRIVER_STC *d;
#if __WORDSIZE == 64
        if (hwInfo->resource.switching.size >= 0x100000000L)
        {
            d = cpssHwDriverGenMmapCreateDrv(
                        hwInfo->resource.switching.start,
                        0x100000000L);
            /* Address Completion Control:
             * set bit 16 to 0, bit 4 to 1 */
            data = 0x00000010;
            d->writeMask(d, 0, 0x140, &data, 1, 0x00010010);
        }
        else
#endif
        {
            d = cpssHwDriverGenMmapAc8CreateDrv(
                        hwInfo->resource.switching.start,
                        hwInfo->resource.switching.size,
                        compRegionMask);
            d->writeMask(d, 0, 0x140, &data, 1, 0x00010000);
        }
        drv->as[CPSS_HW_DRIVER_AS_SWITCHING_E] = d;
    }

    if (hwInfo->resource.resetAndInitController.start)
        drv->as[CPSS_HW_DRIVER_AS_RESET_AND_INIT_CONTROLLER_E] =
                cpssHwDriverGenMmapCreateDrv(
                        hwInfo->resource.resetAndInitController.start,
                        hwInfo->resource.resetAndInitController.size);

    if (hwInfo->resource.mg1.start)
        drv->as[CPSS_HW_DRIVER_AS_MG1_E] = cpssHwDriverGenMmapAc8CreateDrv(
                        hwInfo->resource.mg1.start,
                        hwInfo->resource.mg1.size,
                        0x3);
    if (hwInfo->resource.mg2.start)
        drv->as[CPSS_HW_DRIVER_AS_MG2_E] = cpssHwDriverGenMmapAc8CreateDrv(
                        hwInfo->resource.mg2.start,
                        hwInfo->resource.mg2.size,
                        0x1);
    if (hwInfo->resource.mg3.start)
        drv->as[CPSS_HW_DRIVER_AS_MG3_E] = cpssHwDriverGenMmapAc8CreateDrv(
                        hwInfo->resource.mg3.start,
                        hwInfo->resource.mg3.size,
                        0x1);


    return (CPSS_HW_DRIVER_STC*)drv;
}



