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
* @file prvCpssGenericSrvCpuDrv_BobK.c
*
* @brief BobK Service CPU support
*
* @version   1
********************************************************************************
*/
#include "prvCpssGenericSrvCpuIpcDevCfg.h"

static PRV_SRVCPU_IPC_CHANNEL_STC *prvSrvCpuOpsBobKDrv(
    IN  CPSS_HW_INFO_STC    *hwInfo,
    IN  CPSS_HW_DRIVER_STC  *drv
)
{
    PRV_SRVCPU_IPC_CHANNEL_STC *s;

    *hwInfo = *hwInfo; /* avoid error */
    s = (PRV_SRVCPU_IPC_CHANNEL_STC*) cpssOsMalloc(sizeof(*s));
    if (!s)
        return s;
    cpssOsMemSet(s, 0, sizeof(*s));
    s->size = _128K;
    s->targetBase = 0;

    /* indirect mode only */
    if (drv == NULL)
        return NULL; /* not supported */
    /* return indirect access driver */
    s->drv = prvSrvCpuRamIndirectCreate(
            drv, CPSS_DRV_HW_RESOURCE_SWITCHING_AND_MG0_CORE_E,
            0x40000, _128K);
    return s;
}
/************************************************************/
/***    BobK support                                      ***/
/************************************************************/
GT_STATUS prvSrvCpuOpsBobK(
    IN  CPSS_HW_INFO_STC    *hwInfo,
    IN  CPSS_HW_DRIVER_STC  *drv,
    IN  GT_U32              scpuId,
    IN  PRV_SRVCPU_OPS_ENT  op,
    OUT void                *ret
)
{
    GT_U32  data;
    GT_STATUS rc;

    if (scpuId == 0xffffffff)
        return prvSrvCpuOpsMsys(hwInfo, drv, scpuId, op, ret);
    if (scpuId > 0)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_SUCH, LOG_ERROR_NO_MSG);
    switch (op)
    {
        case PRV_SRVCPU_OPS_EXISTS_E:
            return GT_OK;
        case PRV_SRVCPU_OPS_IS_ON_E:
            /* Confi Processor Global Configuration Register
             * 0x500
             * Bit 28: Controls CM3 activation: 0: Disable 1:enable
             */
            rc = drv->read(drv, CPSS_HW_DRIVER_AS_SWITCHING_E, 0x500, &data, 1);
            if (rc != GT_OK)
                return rc;
            *((GT_BOOL*)ret) = (data & 0x10000000) ? GT_TRUE : GT_FALSE;
            return GT_OK;
        case PRV_SRVCPU_OPS_PRE_LOAD_E:
            /* Confi Processor Global Configuration Register
             * 0x500
             */
            rc = drv->read(drv, CPSS_HW_DRIVER_AS_SWITCHING_E, 0x500, &data, 1);
            if (rc != GT_OK)
                return rc;
            data |= 0x00100000; /* bit 20: init ram */
            data &= 0xfff7ffff; /* bit 19: CM3_PROC_CLK_DIS */
            drv->writeMask(drv, CPSS_HW_DRIVER_AS_SWITCHING_E, 0x500, &data, 1, 0xffffffff);
            drv->read(drv, CPSS_HW_DRIVER_AS_SWITCHING_E, 0x54, &data, 1);
            data |= 1; /* bit 0: CM3_METAL_FIX_JTAG_EN */
            drv->writeMask(drv, CPSS_HW_DRIVER_AS_SWITCHING_E, 0x54, &data, 1, 0xffffffff);
            *((PRV_SRVCPU_IPC_CHANNEL_STC**)ret) = prvSrvCpuOpsBobKDrv(hwInfo, drv);
            return GT_OK;
        case PRV_SRVCPU_OPS_RESET_E:
        case PRV_SRVCPU_OPS_UNRESET_E:
            /* Confi Processor Global Configuration Register
             * 0x500
             * Bit 28: Controls CM3 activation: 0: Disable 1:enable
             */
            rc = drv->read(drv, CPSS_HW_DRIVER_AS_SWITCHING_E, 0x500, &data, 1);
            if (rc != GT_OK)
                return rc;
            if (op == PRV_SRVCPU_OPS_RESET_E)
                data &= ~(0x10000000U);
            else
                data |= 0x10000000;
            return drv->writeMask(drv, CPSS_HW_DRIVER_AS_SWITCHING_E, 0x500, &data, 1, 0xffffffff);
        case PRV_SRVCPU_OPS_PRE_CHECK_IPC_E:
            *((PRV_SRVCPU_IPC_CHANNEL_STC**)ret) = prvSrvCpuOpsBobKDrv(hwInfo, drv);
            return GT_OK;
    }
    return GT_OK;
}
