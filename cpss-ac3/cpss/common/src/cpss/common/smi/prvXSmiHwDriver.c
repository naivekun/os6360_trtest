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
* prvXSmiHwDriver.c
*
* DESCRIPTION:
*       SMI driver for Marvell PP
*       Initialization:
*           prvXSmiHwDriverCreateDrv(platform_drv, as, smiMgmtReg)
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



typedef struct PRV_XSMI_HW_DRIVER_STCT {
    CPSS_HW_DRIVER_STC  common;
    CPSS_OS_MUTEX       mtx;
    GT_U32              as;     /* address space in parent */
    GT_U32              ctrlReg;/* XSMI control register */
    GT_U32              addrReg;/* XSMI address register */
    GT_U8               devNum; /* devNum to use with
                                 *  PRV_CPSS_SMI_MAX_NUM_ITERATIONS_CHECK_CNS
                                 */
}PRV_XSMI_HW_DRIVER_STC;


#define XSMI_TIMEOUT    150 /* = 150us */
#define NTRIES          5
#define BIT(n)          (1 << n)
#define XSMI_BUSY           BIT(30)
#define XSMI_READ_VALID     BIT(29)

#define XSMI_OPCODE_OFFS            26
#define XSMI_OPCODE_READ            (7 << XSMI_OPCODE_OFFS)
#define XSMI_OPCODE_READ_SEQ        (6 << XSMI_OPCODE_OFFS)
#define XSMI_OPCODE_WRITE           (5 << XSMI_OPCODE_OFFS)
#define XSMI_PHYADDR_OFFS           16 /* Phy port addr offset */
#define XSMI_PORT_ADDR(x)           ((x & 0x1f) << XSMI_PHYADDR_OFFS)
#define XSMI_DEVADDR_OFFS           21  /* Phy device addr offset */
#define XSMI_DEV_ADDR(x)            ((x & 0x1f) << XSMI_DEVADDR_OFFS)

/**
* @internal prvXSmiHwDriverCheckStatus function
* @endinternal
*
* @brief   The function reads XSMI control register until selected bits will be
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
static GT_STATUS prvXSmiHwDriverCheckStatus(
    IN  PRV_XSMI_HW_DRIVER_STC *drv,
    IN  GT_U32                 mask,
    IN  GT_U32                 value,
    OUT GT_U32                *regPtr
)
{
    GT_U32  reg;
    volatile GT_U32 retryCnt = 0;
    do
    {
        if (drv->common.parent->read(drv->common.parent, drv->as, drv->ctrlReg, &reg, 1) != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        }
#if 0
#ifdef GM_USED
        reg = value;
#endif
#endif
        PRV_CPSS_SMI_MAX_NUM_ITERATIONS_CHECK_CNS(drv->devNum,(retryCnt++),GT_NOT_READY);
    } while ((reg & mask) != value);

    *regPtr = reg;
    return GT_OK;
}

static GT_STATUS prvXSmiHwDriverRead(
    IN  PRV_XSMI_HW_DRIVER_STC *drv,
    IN  GT_U32  phyAddr,
    IN  GT_U32  regAddr,
    OUT GT_U32 *dataPtr,
    IN  GT_U32  count
)
{
    GT_U32  reg;
    GT_U32  phy;
    GT_U32  dev;
    GT_BOOL first = GT_TRUE;
    GT_STATUS rc;

    phy = (phyAddr & 0xff);
    dev = (phyAddr & 0xff00) >> 8;
    if ((phy >= 32) || (dev >= 32) || (regAddr + count > 0x10000))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    phy = XSMI_PORT_ADDR(phy) | XSMI_DEV_ADDR(dev);
    cpssOsMutexLock(drv->mtx);

    for (;count;count--,regAddr++, dataPtr++)
    {
        if (first == GT_TRUE)
        {
            /* read reg until Busy becomes 0 */
            rc = prvXSmiHwDriverCheckStatus(drv, XSMI_BUSY, 0, &reg);
            if (rc != GT_OK)
            {
                cpssOsMutexUnlock(drv->mtx);
                return rc;
            }
            /* Write phy reg addr */
            drv->common.parent->writeMask(drv->common.parent, drv->as, drv->addrReg, &regAddr, 1, 0xffffffff);
        }
        first = GT_FALSE;
        /* write opcode, devAddr, phyAddr */
        if (count == 1) /* last */
            reg = phy | XSMI_OPCODE_READ;
        else
            reg = phy | XSMI_OPCODE_READ_SEQ;

        drv->common.parent->writeMask(drv->common.parent, drv->as, drv->ctrlReg, &reg, 1, 0xffffffff);
        /* read reg until ReadValid becomes 1 */
        rc = prvXSmiHwDriverCheckStatus(drv, XSMI_READ_VALID, XSMI_READ_VALID, &reg);
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

static GT_STATUS prvXSmiHwDriverWriteMask(
    IN  PRV_XSMI_HW_DRIVER_STC *drv,
    IN  GT_U32  phyAddr,
    IN  GT_U32  regAddr,
    IN  GT_U32 *dataPtr,
    IN  GT_U32  count,
    IN  GT_U32  mask
)
{
    GT_U32  reg;
    GT_U32  phy;
    GT_U32  dev;
    GT_U32  data;
    GT_STATUS rc;

    phy = (phyAddr & 0xff);
    dev = (phyAddr & 0xff00) >> 8;
    if ((phy >= 32) || (dev >= 32) || (regAddr + count > 0x10000))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    phy = XSMI_PORT_ADDR(phy) | XSMI_DEV_ADDR(dev);
    cpssOsMutexLock(drv->mtx);

    for (;count;count--,regAddr++, dataPtr++)
    {
        /* read reg until Busy becomes 0 */
        rc = prvXSmiHwDriverCheckStatus(drv, XSMI_BUSY, 0, &data);
        if (rc != GT_OK)
        {
            cpssOsMutexUnlock(drv->mtx);
            return rc;
        }
        /* Write phy reg addr */
        drv->common.parent->writeMask(drv->common.parent, drv->as, drv->addrReg, &regAddr, 1, 0xffffffff);
        data = *dataPtr;
        if ((mask & 0x0000ffff) != 0x0000ffff)
        {
            reg = phy | XSMI_OPCODE_READ;
            drv->common.parent->writeMask(drv->common.parent, drv->as, drv->ctrlReg, &reg, 1, 0xffffffff);
            /* read reg until ReadValid becomes 1 */
            rc = prvXSmiHwDriverCheckStatus(drv, XSMI_READ_VALID, XSMI_READ_VALID, &data);
            if (rc != GT_OK)
            {
                cpssOsMutexUnlock(drv->mtx);
                return rc;
            }
            data &= ~mask;
            data |= (*dataPtr) & mask;
        }
        data &= 0x0000ffff;
        /* now write */
        reg = phy | XSMI_OPCODE_WRITE | data;
        drv->common.parent->writeMask(drv->common.parent, drv->as, drv->ctrlReg, &reg, 1, 0xffffffff);
    }
    cpssOsMutexUnlock(drv->mtx);
    return GT_OK;
}

static GT_STATUS prvXSmiHwDriverDestroyDrv(
    IN  PRV_XSMI_HW_DRIVER_STC *drv
)
{
    cpssOsMutexDelete(drv->mtx);
    cpssOsFree(drv);
    return GT_OK;
}

GT_STATUS prvXSmiHwDriverCreateDrv(
    IN  GT_U8   devNum,
    IN  GT_U32  portGroupId,
    IN  CPSS_PHY_XSMI_INTERFACE_ENT  i
)
{
    GT_U32  ctrlRegAddr;    /* address of XSMI ctrl reg. */
    GT_U32  addrRegAddr;    /* address of XSMI addr reg. */
    GT_STATUS rc;
    CPSS_DRV_HW_RESOURCE_TYPE_ENT resType; /* specifies address space to be accessed */
    PRV_CPSS_GEN_PP_CONFIG_STC *ppConfig;
    PRV_XSMI_HW_DRIVER_STC *drv;
    CPSS_HW_DRIVER_STC *parent, *tmp;
    char buf[64];

    ppConfig = PRV_CPSS_PP_MAC(devNum);
    if (ppConfig->xsmiMasters[portGroupId][i] != NULL)
        return GT_OK; /* already initialized */
    parent = prvCpssDrvHwPpPortGroupGetDrv(devNum, portGroupId);
    if (!parent)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    if(ppConfig->phyInfo.genXSmiBindFunc.cpssPhyXSmiRegsAddrGet == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);

    rc = ppConfig->phyInfo.genXSmiBindFunc.cpssPhyXSmiRegsAddrGet(
                devNum, (CPSS_PHY_XSMI_INTERFACE_ENT)i,
                &ctrlRegAddr, &addrRegAddr);
    if (rc != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    if (!ctrlRegAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    rc = prvCpssGenXsmiInterfaceResourceTypeGet (devNum, i, &resType);
    if (rc != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);

    cpssOsSprintf(buf, "XSMI@%d:0x%x", resType, ctrlRegAddr);
    tmp = cpssHwDriverLookupRelative(parent, buf);
    if (tmp != NULL)
    {
        /* driver already exists */
        ppConfig->xsmiMasters[portGroupId][i] = tmp;
        tmp->numRefs++;
        return GT_OK;
    }

    drv = cpssOsMalloc(sizeof(*drv));
    if (!drv)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    cpssOsMemSet(drv,0,sizeof(*drv));
    drv->common.read = (CPSS_HW_DRIVER_METHOD_READ)prvXSmiHwDriverRead;
    drv->common.writeMask = (CPSS_HW_DRIVER_METHOD_WRITE_MASK)prvXSmiHwDriverWriteMask;
    drv->common.destroy = (CPSS_HW_DRIVER_METHOD_DESTROY)prvXSmiHwDriverDestroyDrv;
    drv->common.parent = parent;

    cpssOsSprintf(buf, "xsmi%d:%d:%d:0x%x", devNum, i ,portGroupId, ctrlRegAddr);
    cpssOsMutexCreate(buf, &(drv->mtx));
    drv->as = resType;
    drv->ctrlReg = ctrlRegAddr;
    drv->addrReg = addrRegAddr;
    drv->devNum = devNum;

    cpssOsSprintf(buf, "XSMI@%d:0x%x", drv->as, ctrlRegAddr);
    cpssHwDriverRegister((CPSS_HW_DRIVER_STC*)drv, buf);
    cpssOsSprintf(buf, "xsmi%d", i);
    cpssHwDriverAddAlias((CPSS_HW_DRIVER_STC*)drv, buf);
    ppConfig->xsmiMasters[portGroupId][i] = (CPSS_HW_DRIVER_STC*)drv;
    return GT_OK;
}


