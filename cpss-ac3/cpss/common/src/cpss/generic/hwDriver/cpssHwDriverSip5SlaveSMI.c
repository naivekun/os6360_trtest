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
* @file cpssHwDriverSip5SlaveSMI.c
*
* @brief Slave SMI driver for SIP5 devices
*
* @version   1
*
* IGNORE_CPSS_LOG_RETURN_SCRIPT
********************************************************************************
*/
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/hwDriver/cpssHwDriverGeneric.h>
#include <cpss/common/cpssHwInit/cpssHwInit.h>

typedef struct CPSS_HW_DRIVER_SIP5_SSMI_STCT {
    CPSS_HW_DRIVER_STC  common;
    CPSS_OS_MUTEX       mtx;
    GT_U32              phy;
    GT_U32              compl[16];
    int                *asMap;
} CPSS_HW_DRIVER_SIP5_SSMI_STC;


#define SMI_WRITE_ADDRESS_MSB_REGISTER    (0x00)
#define SMI_WRITE_ADDRESS_LSB_REGISTER    (0x01)
#define SMI_WRITE_DATA_MSB_REGISTER       (0x08)
#define SMI_WRITE_DATA_LSB_REGISTER       (0x09)

#define SMI_READ_ADDRESS_MSB_REGISTER     (0x04)
#define SMI_READ_ADDRESS_LSB_REGISTER     (0x05)
#define SMI_READ_DATA_MSB_REGISTER        (0x06)
#define SMI_READ_DATA_LSB_REGISTER        (0x07)

#define SMI_STATUS_REGISTER               (0x1f)
#define SMI_STATUS_WRITE_DONE             (0x02)
#define SMI_STATUS_READ_READY             (0x01)


#define SMI_WAIT_FOR_STATUS_DONE
#define SMI_TIMEOUT_COUNTER  10000

#define SSMI_XBAR_PORT_REMAP(_port) (0x13100+(_port)*4)


#define SMI drv->common.parent

static int prvPipeUnitMapArr[16] = {
    0,  /* 0=CNM */
    -1, /* 1 */
    -1, /* 2 */
    3,  /* 3=MG0 */
    -1, /* 4 */
    5,  /* 5=MG1 */
    -1, /* 6=MG2 */
    -1, /* 7=MG3 */
    8,  /* 8=DFX */
    -1, -1, -1, -1, -1, -1, -1
};

#if 0
static int prvBc3UnitMapArr[16] = {
    0,  /* 0=CNM */
    -1, /* 1 */
    -1, /* 2 */
    3,  /* 3=MG0 */
    -1, /* 4 */
    5,  /* 5=MG1 */
    6,  /* 6=MG2 */
    7,  /* 7=MG3 */
    8,  /* 8=DFX */
    -1, -1, -1, -1, -1, -1, -1
};
#endif

#define SMI_ACCESS_TRACE
#ifdef SMI_ACCESS_TRACE
GT_BOOL smi_access_trace_enable = GT_FALSE;
#define SMI_ACCESS_TRACE_PRINTF(...) \
    if( GT_TRUE == smi_access_trace_enable) \
    {                                       \
        cpssOsPrintf(__VA_ARGS__);          \
    }
void smiAccessTraceEnable(GT_BOOL enable)
{
    smi_access_trace_enable = enable;
}
#else /*!SMI_ACCESS_TRACE*/
#define SMI_ACCESS_TRACE_PRINTF(...)    /*empty*/
#endif/*!SMI_ACCESS_TRACE*/
/**
* @internal prvSlaveSmiWaitForStatus function
* @endinternal
*
* @brief   Slave SMI wait
*/
static GT_STATUS prvSlaveSmiWaitForStatus(CPSS_HW_DRIVER_SIP5_SSMI_STC *drv, GT_U32 waitFor)
{
#ifdef SMI_WAIT_FOR_STATUS_DONE
    GT_U32 stat;
    unsigned int timeOut;
    int rc;

    /* wait for write done */
    timeOut = SMI_TIMEOUT_COUNTER;
    do
    {
        rc = SMI->read(SMI, drv->phy, SMI_STATUS_REGISTER, &stat, 1);
        if (rc != GT_OK)
            return rc;
        if (--timeOut < 1)
        {
            /* printk("bspSmiWaitForStatus timeout !\n"); */
            return GT_TIMEOUT;
        }
    } while ((stat & waitFor) == 0);
#endif
    return GT_OK;
}

#define CHK_RC(cmd) \
        rc = cmd; \
        if (rc != GT_OK) \
            return rc

/**
* @internal prvSlaveSmiRead32 function
* @endinternal
*
* @brief   Slave SMI read 32bit word (low level)
*
* @param[in] regAddr                  - 32-bit register address
*
* @param[out] dataPtr                  - pointer to store data read
*                                       GT_STATUS
*/
static GT_STATUS prvSlaveSmiRead32(
    IN  CPSS_HW_DRIVER_SIP5_SSMI_STC *drv,
    IN  GT_U32  regAddr,
    OUT GT_U32 *dataPtr
)
{
    GT_STATUS rc;
    GT_U32  msb, lsb;

    msb = regAddr >> 16;
    lsb = regAddr & 0xffff;
    CHK_RC(SMI->writeMask(SMI, drv->phy, SMI_READ_ADDRESS_MSB_REGISTER, &msb, 1, 0xffffffff));
    CHK_RC(SMI->writeMask(SMI, drv->phy, SMI_READ_ADDRESS_LSB_REGISTER, &lsb, 1, 0xffffffff));

    CHK_RC(prvSlaveSmiWaitForStatus(drv, SMI_STATUS_READ_READY));

    CHK_RC(SMI->read(SMI, drv->phy, SMI_READ_DATA_MSB_REGISTER, &msb, 1));
    CHK_RC(SMI->read(SMI, drv->phy, SMI_READ_DATA_LSB_REGISTER, &lsb, 1));

    *dataPtr = (msb << 16) | lsb;

    return GT_OK;
}


/**
* @internal prvSlaveSmiWrite32 function
* @endinternal
*
* @brief   Slave SMI write 32bit word (low level)
*
* @param[in] regAddr                  - 32-bit register address
* @param[in] data                     - 32-bit data
*                                       GT_STATUS
*/
static GT_STATUS prvSlaveSmiWrite32(
    IN  CPSS_HW_DRIVER_SIP5_SSMI_STC *drv,
    IN  GT_U32  regAddr,
    IN  GT_U32  data
)
{
    GT_STATUS rc;
    GT_U32  msb, lsb;

    msb = regAddr >> 16;
    lsb = regAddr & 0xffff;

    CHK_RC(SMI->writeMask(SMI, drv->phy, SMI_WRITE_ADDRESS_MSB_REGISTER, &msb, 1, 0xffffffff));
    CHK_RC(SMI->writeMask(SMI, drv->phy, SMI_WRITE_ADDRESS_LSB_REGISTER, &lsb, 1, 0xffffffff));

    msb = data >> 16;
    lsb = data & 0xffff;

    CHK_RC(SMI->writeMask(SMI, drv->phy, SMI_WRITE_DATA_MSB_REGISTER, &msb, 1, 0xffffffff));
    CHK_RC(SMI->writeMask(SMI, drv->phy, SMI_WRITE_DATA_LSB_REGISTER, &lsb, 1, 0xffffffff));

    rc = prvSlaveSmiWaitForStatus(drv, SMI_STATUS_WRITE_DONE);

    return rc;
}

static GT_STATUS prvSlaveSmiCompl(
    IN  CPSS_HW_DRIVER_SIP5_SSMI_STC *drv,
    IN  GT_U32  as,
    IN  GT_U32  regAddr,
    OUT GT_U32  *reg
)
{
    int mbusUnitId = -1;
    GT_U32 r;
    GT_STATUS rc;

    if (as == 0)
    {
        /* cnm */
        *reg = regAddr /* & 0xfffffffc */;
        return GT_OK;
    }
    else if (as < 16 && drv->asMap)
    {
        mbusUnitId = drv->asMap[as];
    }
    else if (as == SSMI_FALCON_ADDRESS_SPACE) {
        mbusUnitId = (regAddr>>20) & 0x0f;
        regAddr &=(~0xF00000);
    }
    if (mbusUnitId < 0)
            return GT_BAD_PARAM;

    r = (regAddr>>28) & 0x0f;
    if (r != drv->compl[mbusUnitId])
    {
        rc = prvSlaveSmiWrite32(drv, SSMI_XBAR_PORT_REMAP(mbusUnitId), r|0x00010000);
        if (rc != GT_OK)
            return rc;
        drv->compl[mbusUnitId] = r;
    }
    *reg = (regAddr&0x0ffffffc) | ((mbusUnitId<<28)|2);
    return GT_OK;
}

static GT_STATUS prvSlaveSmiRead(
    IN  CPSS_HW_DRIVER_SIP5_SSMI_STC *drv,
    IN  GT_U32  as,
    IN  GT_U32  regAddr,
    OUT GT_U32 *dataPtr,
    IN  GT_U32  count
)
{
    GT_STATUS rc = GT_BAD_PARAM;
    GT_U32    reg;
    cpssOsMutexLock(drv->mtx);

    for (; count; count--,regAddr+=4, dataPtr++)
    {
        SMI_ACCESS_TRACE_PRINTF("SMI (before)Read: regAddr 0x%8.8x \n", regAddr);

        rc = prvSlaveSmiCompl(drv, as, regAddr, &reg);
        if (rc != GT_OK)
            break;
        rc = prvSlaveSmiRead32(drv, reg, dataPtr);
        if (rc != GT_OK)
            break;

        SMI_ACCESS_TRACE_PRINTF("SMI (after) Read: rc %x, regAddr 0x%8.8x, data 0x%8.8x\n", rc, regAddr, *dataPtr);
    }

    cpssOsMutexUnlock(drv->mtx);
    return rc;
}

static GT_STATUS prvSlaveSmiWriteMask(
    IN  CPSS_HW_DRIVER_SIP5_SSMI_STC *drv,
    IN  GT_U32  as,
    IN  GT_U32  regAddr,
    IN  GT_U32 *dataPtr,
    IN  GT_U32  count,
    IN  GT_U32  mask
)
{
    GT_STATUS rc = GT_BAD_PARAM;
    GT_U32 dataWr, reg;

    cpssOsMutexLock(drv->mtx);

    for (; count; count--,regAddr+=4, dataPtr++)
    {
        SMI_ACCESS_TRACE_PRINTF("SMI Write: regAddr 0x%8.8x, data 0x%8.8x, mask 0x%8.8x \n", regAddr, *dataPtr, mask);

        rc = prvSlaveSmiCompl(drv, as, regAddr, &reg);
        if (rc != GT_OK)
            break;
        if (mask != 0xffffffff)
        {
            rc = prvSlaveSmiRead32(drv, reg, &dataWr);
            if (rc != GT_OK)
                break;
            dataWr &= ~mask;
            dataWr |= (*dataPtr & mask);
        }
        else
        {
            dataWr = *dataPtr;
        }
        rc = prvSlaveSmiWrite32(drv, reg, dataWr);
        if (rc != GT_OK)
            break;
    }

    if(rc != GT_OK)
    {
        SMI_ACCESS_TRACE_PRINTF("SMI Write: FAILED rc=[%d]\n",rc);
    }

    cpssOsMutexUnlock(drv->mtx);
    return rc;
}

static GT_STATUS prvSlaveSmiDesroy(
    IN  CPSS_HW_DRIVER_SIP5_SSMI_STC *drv
)
{
    cpssOsMutexDelete(drv->mtx);
    cpssOsFree(drv);
    return GT_OK;
}

/**
* @internal cpssHwDriverSip5SlaveSMICreateDrv function
* @endinternal
*
* @brief Create driver instance for Sip5 Slave SMI
*
* @param[in] smi        - pointer to SMI driver
* @param[in] phy        - PP phy address
*
* @retval CPSS_HW_DRIVER_STC* or NULL
*/
CPSS_HW_DRIVER_STC *cpssHwDriverSip5SlaveSMICreateDrv(
    IN  CPSS_HW_DRIVER_STC *smi,
    IN  GT_U32              phy
)
{
    CPSS_HW_DRIVER_SIP5_SSMI_STC *drv;
    char buf[64];
    GT_STATUS rc = GT_OK;
    GT_U32 i;

    /* check params validity */
    if ((smi == NULL) || (phy >= 32))
        return NULL;

    drv = cpssOsMalloc(sizeof(*drv));
    if (drv == NULL)
        return NULL;
    cpssOsMemSet(drv,0,sizeof(*drv));
    drv->common.read = (CPSS_HW_DRIVER_METHOD_READ)prvSlaveSmiRead;
    drv->common.writeMask = (CPSS_HW_DRIVER_METHOD_WRITE_MASK)prvSlaveSmiWriteMask;
    drv->common.destroy = (CPSS_HW_DRIVER_METHOD_DESTROY)prvSlaveSmiDesroy;
    drv->common.parent = smi;
    drv->phy = phy;

    /* init SSMI XBAR */
    for (i = 1; i < 16;i++)
    {
        rc = prvSlaveSmiWrite32(drv, SSMI_XBAR_PORT_REMAP(i), 0x00010000);
        if (rc != GT_OK)
            break;
    }
    if (rc != GT_OK)
    {
        cpssOsFree(drv);
        return NULL;
    }
    /* TODO: detect device ID, apply correct unit map */
    /* PIPE only */
    drv->asMap = prvPipeUnitMapArr;

    cpssOsSprintf(buf, "ssmi@0x%x", phy);
    cpssOsMutexCreate(buf, &(drv->mtx));
    return (CPSS_HW_DRIVER_STC*)drv;
}


/**
* @internal cpssHwDriverSlaveSmiConfigure function
* @endinternal
*
* @brief   Configure Slave SMI device driver
*
* @param[in] smiMasterPath            - driver pointer
* @param[in] slaveSmiPhyId            - SMI Id of slave device
*
* @param[out] hwInfoPtr               - The pointer to HW info, will be used
*                                       for cpssDxChHwPpPhase1Init()
*                                       NULL to not fill HW info
*
* @retval GT_OK                       - on success
* @retval GT_OUT_OF_CPU_MEM
*/
GT_STATUS cpssHwDriverSlaveSmiConfigure(
    IN  const char         *smiMasterPath,
    IN  GT_U32              slaveSmiPhyId,
    OUT CPSS_HW_INFO_STC   *hwInfoPtr
)
{
    CPSS_HW_DRIVER_STC *smi, *ssmi;
    char drvName[16];
    if (smiMasterPath == NULL)
        return GT_BAD_PARAM;
    smi = cpssHwDriverLookup(smiMasterPath);
    if (smi == NULL)
        return GT_NOT_FOUND;
    ssmi = cpssHwDriverSip5SlaveSMICreateDrv(smi, slaveSmiPhyId);
    if (ssmi == NULL)
        return GT_OUT_OF_CPU_MEM;
    cpssOsSprintf(drvName, "SSMI@%d", slaveSmiPhyId);
    cpssHwDriverRegister(ssmi, drvName);
    if (hwInfoPtr != NULL)
    {
        CPSS_HW_INFO_STC hwInfo = CPSS_HW_INFO_STC_DEF;
        hwInfo.busType = CPSS_HW_INFO_BUS_TYPE_SMI_E;
        hwInfo.hwAddr.busNo = 0;
        hwInfo.hwAddr.devSel = slaveSmiPhyId;
        hwInfo.driver = ssmi;
        hwInfo.irq.switching = CPSS_PP_INTERRUPT_VECTOR_USE_POLLING_CNS;
        *hwInfoPtr = hwInfo;
    }
    return GT_OK;
}


