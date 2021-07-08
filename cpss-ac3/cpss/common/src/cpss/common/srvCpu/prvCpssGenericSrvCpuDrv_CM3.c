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
* @file prvCpssGenericSrvCpuDrv_CM3.c
*
* @brief CM3 Service CPU support (Aldrin, BC3, PIPE)
*
* @version   1
********************************************************************************
*/
#include "prvCpssGenericSrvCpuIpcDevCfg.h"

/************************************************************/
/***    Aldrin/BC3/PIPE support                           ***/
/************************************************************/
static PRV_SRVCPU_IPC_CHANNEL_STC *prvSrvCpuOpsCM3Drv(
    IN  CPSS_HW_INFO_STC    *hwInfo,
    IN  CPSS_HW_DRIVER_STC  *drv,
    IN  CPSS_DRV_HW_RESOURCE_TYPE_ENT   res
)
{
    PRV_SRVCPU_IPC_CHANNEL_STC *s;
    GT_U32 data;
    GT_STATUS rc;

    *hwInfo = *hwInfo; /* avoid error */
    s = (PRV_SRVCPU_IPC_CHANNEL_STC*) cpssOsMalloc(sizeof(*s));
    if (!s)
        return s;
    cpssOsMemSet(s, 0, sizeof(*s));

    if (drv == NULL)
        return NULL; /* not supported */
    rc = drv->read(drv, res, 0x4c, &data, 1);
    if (rc != GT_OK) {
        return NULL;
    }

    switch((data >> 4) & 0xff00)
    {
        case 0xcc00: /* aldrin2 */
                     s->size = 384*1024;
                     break;

        default: s->size = 192*1024;
                 break;
    }
    
    s->targetBase = 0;

    /* indirect mode only - return indirect access driver */
    s->drv = prvSrvCpuRamIndirectCreate(
            drv, res, 0x80000, s->size);
    return s;
}
GT_STATUS prvSrvCpuOpsCM3(
    IN  CPSS_HW_INFO_STC    *hwInfo,
    IN  CPSS_HW_DRIVER_STC  *drv,
    IN  GT_U32              scpuId,
    IN  PRV_SRVCPU_OPS_ENT  op,
    OUT void                *ret
)
{
    GT_U32  data;
    GT_STATUS rc;
    CPSS_DRV_HW_RESOURCE_TYPE_ENT res;

    if (scpuId == 0xffffffff)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_SUCH, LOG_ERROR_NO_MSG);
    switch (scpuId)
    {
        case 0:
            res = CPSS_DRV_HW_RESOURCE_SWITCHING_AND_MG0_CORE_E;
            break;
        case 1:
            res = CPSS_DRV_HW_RESOURCE_MG1_CORE_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_SUCH, LOG_ERROR_NO_MSG);
    }
    switch (op)
    {
        case PRV_SRVCPU_OPS_EXISTS_E:
            rc = drv->read(drv, res, 0x500, &data, 1);
            if (rc != GT_OK)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_SUCH, LOG_ERROR_NO_MSG);
            return GT_OK;
        case PRV_SRVCPU_OPS_IS_ON_E:
            /* Confi Processor Global Configuration Register 0x500
             * Bit 29: Controls CM3 activation: 0: Disable 1:enable
             */
            rc = drv->read(drv, res, 0x500, &data, 1);
            if (rc != GT_OK)
                return rc;
            *((GT_BOOL*)ret) = (data & 0x20000000) ? GT_TRUE : GT_FALSE;
            return GT_OK;
        case PRV_SRVCPU_OPS_PRE_LOAD_E:
            /* Confi Processor Global Configuration Register 0x500
             * Bit 28: CM3_Enable
             */
            data = 0x10000000;
            drv->writeMask(drv, res, 0x500, &data, 1, 0x10000000);
            *((PRV_SRVCPU_IPC_CHANNEL_STC**)ret) = prvSrvCpuOpsCM3Drv(hwInfo, drv, res);
            return GT_OK;
        case PRV_SRVCPU_OPS_RESET_E:
        case PRV_SRVCPU_OPS_UNRESET_E:
            /* Confi Processor Global Configuration Register 0x500
             * Bit 29: Controls CM3 activation: 0: Disable 1:enable
             */
            rc = drv->read(drv, res, 0x500, &data, 1);
            if (rc != GT_OK)
                return rc;
            if (rc != GT_OK)
                return rc;
            if (op == PRV_SRVCPU_OPS_RESET_E)
                data &= ~(0x20000000U);
            else
                data |= 0x20000000;
            return drv->writeMask(drv, res, 0x500, &data, 1, 0xffffffff);
        case PRV_SRVCPU_OPS_PRE_CHECK_IPC_E:
            *((PRV_SRVCPU_IPC_CHANNEL_STC**)ret) = prvSrvCpuOpsCM3Drv(hwInfo, drv, res);
            return GT_OK;
    }
    return GT_OK;
}
