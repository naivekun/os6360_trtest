/*******************************************************************************
*                Copyright 2017, MARVELL SEMICONDUCTOR, LTD.                   *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL.                      *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
*                                                                              *
* MARVELL COMPRISES MARVELL TECHNOLOGY GROUP LTD. (MTGL) AND ITS SUBSIDIARIES, *
* MARVELL INTERNATIONAL LTD. (MIL), MARVELL TECHNOLOGY, INC. (MTI), MARVELL    *
* SEMICONDUCTOR, INC. (MSI), MARVELL ASIA PTE LTD. (MAPL), MARVELL JAPAN K.K.  *
* (MJKK), MARVELL ISRAEL LTD. (MSIL).                                          *
********************************************************************************
* prvSmiHwDriver.c
*
* DESCRIPTION:
*       SMI driver for Marvell PP
*       Initialization:
*           prvSmiHwDriverCreateDrv(platform_drv, as, smiMgmtReg)
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/
#include <cpss/generic/hwDriver/cpssHwDriverAPI.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/phy/private/prvCpssPhy.h>
#include <cpss/generic/log/prvCpssLog.h>


typedef struct PRV_SMI_HW_DRIVER_STCT {
    CPSS_HW_DRIVER_STC  common;
    CPSS_OS_MUTEX       mtx;
    GT_U32              as;     /* address space in parent */
    GT_U32              reg;    /* SMI management register */
    GT_U8               devNum; /* devNum to use with
                                 *  PRV_CPSS_SMI_MAX_NUM_ITERATIONS_CHECK_CNS
                                 */
}PRV_SMI_HW_DRIVER_STC;

#define SMI_MNG_BUSY            (1<<28)
#define SMI_MNG_RDVALID         (1<<27)
#define SMI_MNG_OPCODE          (1<<26)
#define SMI_MNG_OPCODE_READ     (1<<26)
#define SMI_MNG_OPCODE_WRITE    0
#define SMI_MNG_REG_ADDR(_a)    ((_a)<<21)
#define SMI_MNG_PHY_ADDR(_p)    ((_p)<<16)

/**
* @internal prvSmiHwDriverCheckStatus function
* @endinternal
*
* @brief   The function reads SMI control register until selected bits will be
*          equal to value
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] drv        - driver instance
* @param[in] mask       - mask to select bits to monitor
* @param[in] value      - wait while (reg&bits) != value
* @param[out] regPtr    - pointer to place register value at exit
*
* @retval GT_OK         - on success,
* @retval GT_HW_ERROR   - hw error
* @retval GT_NOT_READY  - timeout
*/
static GT_STATUS prvSmiHwDriverCheckStatus(
    IN  PRV_SMI_HW_DRIVER_STC *drv,
    IN  GT_U32                 mask,
    IN  GT_U32                 value,
    OUT GT_U32                *regPtr
)
{
    GT_U32  reg;
    volatile GT_U32 retryCnt = 0;
    do
    {
        if (drv->common.parent->read(drv->common.parent, drv->as, drv->reg, &reg, 1) != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        }
#ifdef GM_USED
        reg = value;
#endif
        PRV_CPSS_SMI_MAX_NUM_ITERATIONS_CHECK_CNS(drv->devNum,(retryCnt++),GT_NOT_READY);
    } while ((reg & mask) != value);

    *regPtr = reg;
    return GT_OK;
}

static GT_STATUS prvSmiHwDriverRead(
    IN  PRV_SMI_HW_DRIVER_STC *drv,
    IN  GT_U32  phyAddr,
    IN  GT_U32  regAddr,
    OUT GT_U32 *dataPtr,
    IN  GT_U32  count
)
{
    GT_U32      reg;
    GT_STATUS   rc;
    if ((phyAddr >= 32) || (regAddr + count > 32))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    cpssOsMutexLock(drv->mtx);

    for (;count;count--,regAddr++, dataPtr++)
    {
        /* read reg until Busy becomes 0 */
        rc = prvSmiHwDriverCheckStatus(drv, SMI_MNG_BUSY, 0, &reg);
        if (rc != GT_OK)
        {
            cpssOsMutexUnlock(drv->mtx);
            return rc;
        }
        /* write opcode, regAddr, phyAddr */
        reg = SMI_MNG_OPCODE_READ | SMI_MNG_REG_ADDR(regAddr) | SMI_MNG_PHY_ADDR(phyAddr);
        rc = drv->common.parent->writeMask(drv->common.parent, drv->as, drv->reg, &reg, 1, 0xffffffff);
        if (rc != GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        /* read reg until ReadValid becomes 1 */
        rc = prvSmiHwDriverCheckStatus(drv, SMI_MNG_RDVALID, SMI_MNG_RDVALID, &reg);
        if (rc != GT_OK)
        {
            cpssOsMutexUnlock(drv->mtx);
            return rc;
        }

        *dataPtr = reg & 0x0000ffff;
    }
    cpssOsMutexUnlock(drv->mtx);
    return GT_OK;
}

static GT_STATUS prvSmiHwDriverWriteMask(
    IN  PRV_SMI_HW_DRIVER_STC *drv,
    IN  GT_U32  phyAddr,
    IN  GT_U32  regAddr,
    IN  GT_U32 *dataPtr,
    IN  GT_U32  count,
    IN  GT_U32  mask
)
{
    GT_U32  reg, data;
    GT_STATUS   rc;
    if ((phyAddr >= 32) || (regAddr + count > 32))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    cpssOsMutexLock(drv->mtx);
    for (;count;count--,regAddr++, dataPtr++)
    {
        data = *dataPtr;
        if ((mask & 0x0000ffff) != 0x0000ffff)
        {
            rc = prvSmiHwDriverRead(drv, phyAddr, regAddr, &data, 1);
            if (rc != GT_OK)
            {
                cpssOsMutexUnlock(drv->mtx);
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
            data &= ~mask;
            data |= (*dataPtr) & mask;
        }
        data &= 0x0000ffff;
        /* now write */
        /* read reg until Busy is 0 */
        rc = prvSmiHwDriverCheckStatus(drv, SMI_MNG_BUSY, 0, &reg);
        if (rc != GT_OK)
        {
            cpssOsMutexUnlock(drv->mtx);
            return rc;
        }
        /* write opcode, regAddr, phyAddr */
        reg = SMI_MNG_OPCODE_WRITE | SMI_MNG_REG_ADDR(regAddr) | SMI_MNG_PHY_ADDR(phyAddr) | data;
        rc = drv->common.parent->writeMask(drv->common.parent, drv->as, drv->reg, &reg, 1, 0xffffffff);
        if (rc != GT_OK)
        {
            cpssOsMutexUnlock(drv->mtx);
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }
    cpssOsMutexUnlock(drv->mtx);
    return GT_OK;
}

static GT_STATUS prvSmiHwDriverDestroyDrv(
    IN  PRV_SMI_HW_DRIVER_STC *drv
)
{
    cpssOsMutexDelete(drv->mtx);
    cpssOsFree(drv);
    return GT_OK;
}

GT_STATUS prvSmiHwDriverCreateDrv(
    IN  GT_U8   devNum,
    IN  GT_U32  portGroupId,
    IN  CPSS_PHY_SMI_INTERFACE_ENT  i
)
{
    GT_U32  ctrlRegAddr;    /* address of SMI ctrl reg. */
    GT_STATUS rc;
    PRV_CPSS_GEN_PP_CONFIG_STC *ppConfig;
    PRV_SMI_HW_DRIVER_STC *drv;
    CPSS_HW_DRIVER_STC *parent, *tmp;
    char buf[64];

    ppConfig = PRV_CPSS_PP_MAC(devNum);
    if (ppConfig->smiMasters[portGroupId][i] != NULL)
        return GT_OK; /* already initialized */
    parent = prvCpssDrvHwPpPortGroupGetDrv(devNum, portGroupId);
    if (!parent)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    if(ppConfig->phyInfo.genSmiBindFunc.cpssPhySmiRegsAddrGet == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    rc = ppConfig->phyInfo.genSmiBindFunc.cpssPhySmiRegsAddrGet(
                devNum, i, &ctrlRegAddr);
    if (rc != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    if (ctrlRegAddr == 0xFFFFFFFB)
        return /* not error to the LOG */ GT_NO_RESOURCE;

    cpssOsSprintf(buf, "SMI@%d:0x%x", CPSS_DRV_HW_RESOURCE_SWITCHING_AND_MG0_CORE_E, ctrlRegAddr);
    tmp = cpssHwDriverLookupRelative(parent, buf);
    if (tmp != NULL)
    {
        /* driver already exists */
        ppConfig->smiMasters[portGroupId][i] = tmp;
#ifdef ASIC_SIMULATION
        {
            GT_U32  data = 0;
            drv = (PRV_SMI_HW_DRIVER_STC *)tmp;
            parent->writeMask(parent, drv->as, drv->reg, &data, 1, 0xffffffff);
        }
#endif
        tmp->numRefs++;
        return GT_OK;
    }

    drv = cpssOsMalloc(sizeof(*drv));
    if (!drv)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);

    cpssOsMemSet(drv,0,sizeof(*drv));
    drv->common.read = (CPSS_HW_DRIVER_METHOD_READ)prvSmiHwDriverRead;
    drv->common.writeMask = (CPSS_HW_DRIVER_METHOD_WRITE_MASK)prvSmiHwDriverWriteMask;
    drv->common.destroy = (CPSS_HW_DRIVER_METHOD_DESTROY)prvSmiHwDriverDestroyDrv;
    drv->common.parent = parent;

    cpssOsSprintf(buf, "smi%d:%d:0x%x", devNum, portGroupId, ctrlRegAddr);
    cpssOsMutexCreate(buf, &(drv->mtx));
    drv->as = CPSS_DRV_HW_RESOURCE_SWITCHING_AND_MG0_CORE_E;
    drv->reg = ctrlRegAddr;
    drv->devNum = devNum;
#ifdef ASIC_SIMULATION
    {
        GT_U32  data = 0;
        parent->writeMask(parent, drv->as, drv->reg, &data, 1, 0xffffffff);
    }
#endif

    cpssOsSprintf(buf, "SMI@%d:0x%x", drv->as, ctrlRegAddr);
    cpssHwDriverRegister((CPSS_HW_DRIVER_STC*)drv, buf);
    cpssOsSprintf(buf, "smi%d", i);
    cpssHwDriverAddAlias((CPSS_HW_DRIVER_STC*)drv, buf);
    ppConfig->smiMasters[portGroupId][i] = (CPSS_HW_DRIVER_STC*)drv;
    return GT_OK;
}


