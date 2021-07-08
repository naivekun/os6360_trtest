/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
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
* @file prvCpssDrvPpHwApi.c
*
* @brief Prestera driver Hardware API
*
* @version   11
********************************************************************************
*/
/* LOG in this module may be enabled only after adding CPSS_LOCK protection in it. */
/*#define CPSS_LOG_IN_MODULE_ENABLE*/
#include <cpssDriver/log/private/prvCpssDriverLog.h>
#include <cpssDriver/pp/interrupts/generic/prvCpssDrvInterrupts.h>
#include <cpssDriver/pp/config/prvCpssDrvPpCfgInit.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpssDriver/pp/hardware/prvCpssDrvHwCntl.h>
#include <cpssDriver/pp/hardware/prvCpssDrvObj.h>
#include <cpssDriver/pp/hardware/cpssDriverPpHw.h>
#include <cpssDriver/pp/hardware/prvCpssDrvHwPpPortGroupCntl.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpssDriver/pp/hardware/private/prvCpssDriverCreate.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#ifdef ASIC_SIMULATION
#include <asicSimulation/SCIB/scib.h>
#endif

/*******************************************************************************
* External usage environment parameters
*******************************************************************************/

GT_BOOL memoryAccessTraceOn = GT_FALSE;

/* Object that holds callback functions to HW access */
CPSS_DRV_HW_ACCESS_OBJ_STC prvCpssHwAccessObj = {NULL, NULL,NULL, NULL,
                                                 NULL, NULL,NULL, NULL,
                                                 NULL, NULL,NULL, NULL,
                                                 NULL};

#define PRV_CPSS_DRV_CHECK_PP_DEV_MAC(devNum)   \
    if (prvCpssDrvPpConfig[devNum] == NULL ) \
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, \
           prvCpssLogErrorMsgDeviceNotInitialized, devNum);\
    if (prvCpssDrvPpObjConfig[devNum]->HAState == CPSS_SYS_HA_MODE_STANDBY_E) \
        return GT_OK

#define PRV_CPSS_DRV_GET_PP_DRV(devNum,portGroupId,drv) \
    drv = prvCpssDrvPpConfig[devNum]->drivers[portGroupId]

#define PRV_CPSS_DRV_CHECK_GET_PP_DRV(devNum,portGroupId,drv) \
    PRV_CPSS_DRV_CHECK_PP_DEV_MAC(devNum); \
    PRV_CPSS_DRV_GET_PP_DRV(devNum,portGroupId,drv)


/* macro to start a loop on active port group ids */
#define PRV_CPSS_DRV_START_HANDLE_PORT_GROUPS_MAC(_devNum, _portGroupId)     \
{                                                                            \
    GT_U32 firstPg,lastPg,activePortGroupsBmp;                               \
    if (_portGroupId == CPSS_PORT_GROUP_UNAWARE_MODE_CNS)                    \
    {                                                                        \
        firstPg =                                                            \
            prvCpssDrvPpConfig[_devNum]->portGroupsInfo.firstActivePortGroup;\
        lastPg =                                                             \
            prvCpssDrvPpConfig[_devNum]->portGroupsInfo.lastActivePortGroup; \
        activePortGroupsBmp =                                                \
            prvCpssDrvPpConfig[_devNum]->portGroupsInfo.activePortGroupsBmp; \
    }                                                                        \
    else                                                                     \
    {                                                                        \
        firstPg = (_portGroupId);                                            \
        lastPg = (_portGroupId);                                             \
        activePortGroupsBmp = (1<<(_portGroupId));                           \
    }                                                                        \
    for((_portGroupId) = firstPg; (_portGroupId) <= lastPg; (_portGroupId)++)\
    {                                                                        \
        if(0 == (activePortGroupsBmp & (1<<(_portGroupId))))                 \
        {                                                                    \
            continue;                                                        \
        }

/* macro to end a loop on active port groups */
#define PRV_CPSS_DRV_END_HANDLE_PORT_GROUPS_MAC(_devNum, _portGroupId)       \
    }                                                                        \
}

extern GT_BOOL  prvCpssDrvTraceHwWrite[PRV_CPSS_MAX_PP_DEVICES_CNS];
extern GT_BOOL  prvCpssDrvTraceHwRead[PRV_CPSS_MAX_PP_DEVICES_CNS];

static GT_U8   onEmulator_devNum_iterateRamAction = 0xFF;
void onEmulator_devNum_iterateRamActionSet(IN GT_U8  devNum)
{
    onEmulator_devNum_iterateRamAction = devNum;
}

#define GENERIC_MG_INSTANCE     BIT_16
#define MG_x_INSTANCE_ID(x) ((x) | GENERIC_MG_INSTANCE)

/* check if the MG is not MG 0 :
return :
1 - MG that is not 0
0 - not MG or MG0
*/
#define SIP6_IS_MG_NON_0(_as)                                    \
    ((((as) & GENERIC_MG_INSTANCE) ||                       \
      ((_as) >= CPSS_DRV_HW_RESOURCE_MG1_CORE_E &&          \
       (_as) <= CPSS_DRV_HW_RESOURCE_MG3_CORE_E)) ? 1 : 0)

/**
* @internal prvCpssDrvHwPpDoReadOpDrv function
* @endinternal
*
* @brief   Read a selected register, perform trace operations
*
* @param[in] devNum                   - The PP's device number
* @param[in] portGroupId              - The port group id.
* @param[in] drv                      - Driver object ptr
* @param[in] as                       - addess space
* @param[in] isrContext               -
* @param[in] regAddr                  - The register's address to read from.
*
* @param[out] dataPtr                  - Data to read from the register.
*
* @retval GT_OK                    - on success, otherwise error
*
* @note
*       Galtis:
*       None.
*
*/
static GT_STATUS prvCpssDrvHwPpDoReadOpDrv
(
    IN GT_U8                        devNum,
    IN GT_U32                       portGroupId,
    IN CPSS_HW_DRIVER_STC          *drv,
    IN GT_U32                       as,
    IN GT_BOOL                      isrContext,
    IN GT_U32                       regAddr,
    OUT GT_U32                      *dataPtr
)
{
    GT_STATUS rc;

    if (PRV_CPSS_PP_MAC(devNum) && PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        if(SIP6_IS_MG_NON_0(as))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                "The device not supports MG x as 'address space'");
        }

        /* all the next are part of 'switching core' */
        if(as == CPSS_HW_DRIVER_AS_DFX_E)
        {
            /* sip6 : calling the 'non-Drv' for the DFX functions */
            return prvCpssHwPpReadRegister(devNum,regAddr,dataPtr);
        }
    }


    rc = drv->read(drv, as, regAddr, dataPtr, 1);
    if (rc != GT_OK)
        return rc;

    /* Check if trace hw read is enabled */
    if (prvCpssDrvTraceHwRead[devNum] == GT_TRUE)
    {
        rc = cpssTraceHwAccessRead(devNum,
                                     portGroupId,
                                     isrContext,
                                     (CPSS_TRACE_HW_ACCESS_ADDR_SPACE_ENT)as,
                                     regAddr,
                                     1, /* length to trace the data in words */
                                     dataPtr);
    }
    return rc;
}

/**
* @internal prvCpssDrvHwPpWriteRegBitMaskDrv function
* @endinternal
*
* @brief   write a selected register bitmasked value.
*
* @param[in] devNum                   - The PP's device number
* @param[in] portGroupId              - The port group id.
* @param[in] drv                      - Driver object ptr
* @param[in] as                       - addess space
* @param[in] isrContext               -
* @param[in] regAddr                  - The register's address to read from.
* @param[in] mask                     - Mask for selecting the written bits.
* @param[in] value                    - Data to be written to register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note
*       Galtis:
*       None.
*
*/
static GT_STATUS prvCpssDrvHwPpWriteRegBitMaskDrv
(
    IN GT_U8                        devNum,
    IN GT_U32                       portGroupId,
    IN CPSS_HW_DRIVER_STC           *drv,
    IN GT_U32                       as,
    IN GT_BOOL                      isrContext,
    IN GT_U32                       regAddr,
    IN GT_U32                       mask,
    IN GT_U32                       value
)
{
    GT_STATUS rc;

    SYSTEM_STATE_CAUSED_SKIP_MAC;

    if (PRV_CPSS_PP_MAC(devNum) && PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        if(SIP6_IS_MG_NON_0(as))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                "The device not supports MG x as 'address space'");
        }

        /* all the next are part of 'switching core' */
        if(as == CPSS_HW_DRIVER_AS_DFX_E)
        {
            /* sip6 : calling the 'non-Drv' for the DFX functions */
            return prvCpssHwPpWriteRegBitMask(devNum,regAddr,mask,value);
        }
    }

    rc = drv->writeMask(drv, as, regAddr, &value, 1, mask);
    /* Check if trace hw write is enabled */
    if (prvCpssDrvTraceHwWrite[devNum] == GT_TRUE)
    {
        rc = cpssTraceHwAccessWrite(devNum,
                                      portGroupId,
                                      isrContext,
                                      (CPSS_TRACE_HW_ACCESS_ADDR_SPACE_ENT)as,
                                      regAddr,
                                      1, /* length to trace the data in words */
                                      &value,
                                      mask);
    }
    return rc;
}

/**
* @internal prvCpssDrvHwPpGetRegFieldDrv function
* @endinternal
*
* @brief   Read a selected register field.
*
* @param[in] devNum                   - The PP's device number
* @param[in] portGroupId              - The port group id.
* @param[in] drv                      - Driver object ptr
* @param[in] as                       - addess space
* @param[in] isrContext               -
* @param[in] regAddr                  - The register's address to read from.
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be read.
*
* @param[out] fieldData                - Data to read from the register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note
*       Galtis:
*       None.
*
*/
static GT_STATUS prvCpssDrvHwPpGetRegFieldDrv
(
    IN GT_U8                        devNum,
    IN GT_U32                       portGroupId,
    IN CPSS_HW_DRIVER_STC          *drv,
    IN GT_U32                       as,
    IN GT_BOOL                      isrContext,
    IN GT_U32                       regAddr,
    IN GT_U32                       fieldOffset,
    IN GT_U32                       fieldLength,
    OUT GT_U32                      *fieldData
)
{
    GT_U32 data, mask;
    GT_STATUS rc;

    rc = prvCpssDrvHwPpDoReadOpDrv(devNum, portGroupId, drv, as, isrContext,
            regAddr, &data);

    if (rc != 0)
        return rc;

    CALC_MASK_MAC(fieldLength, fieldOffset, mask);
    *fieldData = (GT_U32)((data & mask) >> fieldOffset);
    return rc;
}

/**
* @internal prvCpssDrvHwPpSetRegFieldDrv function
* @endinternal
*
* @brief   write a selected register field.
*
* @param[in] devNum                   - The PP's device number
* @param[in] portGroupId              - The port group id.
* @param[in] drv                      - Driver object ptr
* @param[in] as                       - addess space
* @param[in] isrContext               -
* @param[in] regAddr                  - The register's address to read from.
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be read.
* @param[in] fieldData                - Data to read from the register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note
*       Galtis:
*       None.
*
*/
static GT_STATUS prvCpssDrvHwPpSetRegFieldDrv
(
    IN GT_U8                        devNum,
    IN GT_U32                       portGroupId,
    IN CPSS_HW_DRIVER_STC          *drv,
    IN GT_U32                       as,
    IN GT_BOOL                      isrContext,
    IN GT_U32                       regAddr,
    IN GT_U32                       fieldOffset,
    IN GT_U32                       fieldLength,
    IN GT_U32                       fieldData
)
{
    GT_U32 mask, value;

    CALC_MASK_MAC(fieldLength, fieldOffset, mask);
    value = ((fieldData << fieldOffset) & mask);

    return prvCpssDrvHwPpWriteRegBitMaskDrv(devNum, portGroupId, drv,
            as, isrContext,
            regAddr, mask, value);
}

/**
* @internal prvCpssDrvHwPpReadRegBitMaskDrv function
* @endinternal
*
* @brief   Read a selected register bit masked value.
*
* @param[in] devNum                   - The PP's device number
* @param[in] portGroupId              - The port group id.
* @param[in] drv                      - Driver object ptr
* @param[in] as                       - addess space
* @param[in] isrContext               -
* @param[in] regAddr                  - The register's address to read from.
* @param[in] mask                     - Mask for selecting the read bits.
*
* @param[out] dataPtr                  - Data read from register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note
*       Galtis:
*       None.
*
*/
static GT_STATUS prvCpssDrvHwPpReadRegBitMaskDrv
(
    IN GT_U8                        devNum,
    IN GT_U32                       portGroupId,
    IN CPSS_HW_DRIVER_STC          *drv,
    IN GT_U32                       as,
    IN GT_BOOL                      isrContext,
    IN GT_U32                       regAddr,
    IN GT_U32                       mask,
    OUT GT_U32                      *dataPtr
)
{
    GT_STATUS rc;

    rc = prvCpssDrvHwPpDoReadOpDrv(devNum, portGroupId, drv, as, isrContext,
            regAddr, dataPtr);
    if (rc != 0)
        return rc;
    *dataPtr &= mask;
    return rc;
}

typedef struct PRV_CPSS_HW_DRIVER_EMULATOR_CHECK_STCT {
    CPSS_HW_DRIVER_STC  common;
    GT_U8               devNum;
    GT_U32              portGroupId;
} PRV_CPSS_HW_DRIVER_EMULATOR_CHECK_STC;

static GT_STATUS prvCpssHwDriverEmulatorCheckRead(
    IN  PRV_CPSS_HW_DRIVER_EMULATOR_CHECK_STC *drv,
    IN  GT_U32  addrSpace,
    IN  GT_U32  regAddr,
    OUT GT_U32 *dataPtr,
    IN  GT_U32  count
)
{
    if (addrSpace == CPSS_HW_DRIVER_AS_SWITCHING_E)
    {
        GT_STATUS addrValid;
        addrValid = prvCpssOnEmulatorSupportedAddrCheck(drv->devNum,drv->portGroupId,regAddr);
        if (addrValid == GT_FALSE)
        {
            /* the memory/register is not in supported memory space */
            /* ignore the read operation , return dummy read value */
            *dataPtr = 0;
            return GT_OK;
        }
    }
    return drv->common.parent->read(drv->common.parent, addrSpace, regAddr, dataPtr, count);
}

static GT_STATUS prvCpssHwDriverEmulatorCheckWriteMask(
    IN  PRV_CPSS_HW_DRIVER_EMULATOR_CHECK_STC *drv,
    IN  GT_U32  addrSpace GT_UNUSED,
    IN  GT_U32  regAddr,
    IN  GT_U32 *dataPtr,
    IN  GT_U32  count,
    IN  GT_U32  mask
)
{
    if (addrSpace == CPSS_HW_DRIVER_AS_SWITCHING_E)
    {
        GT_STATUS addrValid;
        addrValid = prvCpssOnEmulatorSupportedAddrCheck(drv->devNum,drv->portGroupId,regAddr);
        if (addrValid == GT_FALSE)
        {
            /* the memory/register is not in supported memory space */
            /* ignore the write operation */
            return GT_OK;
        }
    }
    return drv->common.parent->writeMask(drv->common.parent, addrSpace, regAddr, dataPtr, count, mask);
}

static GT_STATUS prvCpssHwDriverEmulatorCheckDestroy(
    IN  PRV_CPSS_HW_DRIVER_EMULATOR_CHECK_STC *drv
)
{
    cpssOsFree(drv);
    return GT_OK;
}

/**
* @internal prvCpssHwDriverEmulatorCheckDrv function
* @endinternal
*
* @brief Create driver which will check if register address
*        valid ine mulator
*
* @param[in] hwInfo             - HW info
*
* @retval CPSS_HW_DRIVER_STC* or NULL
*/
CPSS_HW_DRIVER_STC *prvCpssHwDriverEmulatorCheckDrv(
    IN  CPSS_HW_DRIVER_STC    *parent,
    IN  GT_U8                  devNum,
    IN  GT_U32                 portGroupId
)
{
    PRV_CPSS_HW_DRIVER_EMULATOR_CHECK_STC *drv;

    drv = cpssOsMalloc(sizeof(*drv));
    if (drv == NULL)
        return NULL;
    cpssOsMemSet(drv,0,sizeof(*drv));
    drv->common.read = (CPSS_HW_DRIVER_METHOD_READ)prvCpssHwDriverEmulatorCheckRead;
    drv->common.writeMask = (CPSS_HW_DRIVER_METHOD_WRITE_MASK)prvCpssHwDriverEmulatorCheckWriteMask;
    drv->common.destroy = (CPSS_HW_DRIVER_METHOD_DESTROY)prvCpssHwDriverEmulatorCheckDestroy;
    drv->devNum = devNum;
    drv->portGroupId = portGroupId;
    drv->common.parent = parent;
    return (CPSS_HW_DRIVER_STC*)drv;
}

/**
* @internal prvCpssDrvHwCntlInit function
* @endinternal
*
* @brief   This function initializes the Hw control structure of a given PP.
*
* @param[in] devNum                   - The PP's device number to init the structure for.
* @param[in] portGroupId              - The port group id.
*                                      (APPLICABLE DEVICES Lion; Lion2)
*                                      relevant only to 'multi-port-group' devices.
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*                                      baseAddr        - The PP base address on the host interface bus.
*                                      internalPciBase - Base address to which the internal pci registers
*                                      are mapped to.
* @param[in] isDiag                   - Is this initialization is for diagnostics purposes
*                                      (GT_TRUE), or is it a final initialization of the Hw
*                                      Cntl unit (GT_FALSE)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note 1. In case isDiag == GT_TRUE, no semaphores are initialized.
*       Galtis:
*       None.
*
*/
GT_STATUS prvCpssDrvHwCntlInit
(
    IN GT_U8        devNum,
    IN GT_U32       portGroupId,
    IN CPSS_HW_INFO_STC *hwInfo,
    IN GT_BOOL      isDiag
)
{
    GT_STATUS rc;
    CPSS_HW_DRIVER_STC *drv = NULL;
    char drvName[32];

    /* check if driver object is ready */
    PRV_CPSS_DRV_CHECK_GEN_FUNC_OBJ_MAC(devNum);

    if(prvCpssDrvPpConfig[devNum]->portGroupsInfo.isMultiPortGroupDevice == GT_FALSE)
    {
        cpssOsSprintf(drvName, "/pp%d", devNum);
        portGroupId = CPSS_NON_MULTI_PORT_GROUP_DEVICE_INDEX_CNS;
        prvCpssDrvPpConfig[devNum]->portGroupsInfo.numOfPortGroups        = 1;
        prvCpssDrvPpConfig[devNum]->portGroupsInfo.activePortGroupsBmp    = BIT_0;
        prvCpssDrvPpConfig[devNum]->portGroupsInfo.firstActivePortGroup   = CPSS_NON_MULTI_PORT_GROUP_DEVICE_INDEX_CNS;
        prvCpssDrvPpConfig[devNum]->portGroupsInfo.lastActivePortGroup    = CPSS_NON_MULTI_PORT_GROUP_DEVICE_INDEX_CNS;

        prvCpssDrvPpConfig[devNum]->intCtrl.
            portGroupInfo[CPSS_NON_MULTI_PORT_GROUP_DEVICE_INDEX_CNS].
                isrCookieInfo.devNum = devNum;
        prvCpssDrvPpConfig[devNum]->intCtrl.
            portGroupInfo[CPSS_NON_MULTI_PORT_GROUP_DEVICE_INDEX_CNS].
                isrCookieInfo.portGroupId = CPSS_NON_MULTI_PORT_GROUP_DEVICE_INDEX_CNS;
    }
    else if(0 == PRV_CPSS_DRV_IS_IN_RANGE_PORT_GROUP_ID_MAC(devNum,portGroupId))
    {
        /* out of range parameter */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    else /* multi port groups device */
    {
        cpssOsSprintf(drvName, "/pp%d.%d", devNum, portGroupId);
        prvCpssDrvPpConfig[devNum]->portGroupsInfo.numOfPortGroups++;
        if(prvCpssDrvPpConfig[devNum]->portGroupsInfo.numOfPortGroups >= 32)
        {
            /* limit to 32 due to current GT_U32 as bitmap */
            /* for active port groupss                     */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FULL, LOG_ERROR_NO_MSG);
        }

        prvCpssDrvPpConfig[devNum]->portGroupsInfo.activePortGroupsBmp    |= (1 << portGroupId);

        /* check if need to update the firstActivePortGroup */
        if(prvCpssDrvPpConfig[devNum]->portGroupsInfo.firstActivePortGroup > portGroupId)
        {
            prvCpssDrvPpConfig[devNum]->portGroupsInfo.firstActivePortGroup = portGroupId;
        }

        /* check if need to update the lastActivePortGroup */
        if(prvCpssDrvPpConfig[devNum]->portGroupsInfo.lastActivePortGroup < portGroupId)
        {
            prvCpssDrvPpConfig[devNum]->portGroupsInfo.lastActivePortGroup = portGroupId;
        }

        prvCpssDrvPpConfig[devNum]->intCtrl.portGroupInfo[portGroupId].isrCookieInfo.devNum = devNum;
        prvCpssDrvPpConfig[devNum]->intCtrl.portGroupInfo[portGroupId].isrCookieInfo.portGroupId = portGroupId;

        /* initializer the 'force' portGroupId to 'Not forcing' */
        prvCpssDrvPpConfig[devNum]->portGroupsInfo.debugForcedPortGroupId = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    }
    drv = cpssHwDriverLookup(drvName);
    if (drv != NULL)
    {
        /* driver already exists */
        prvCpssDrvPpConfig[devNum]->drivers[portGroupId] = drv;
        return GT_OK;
    }

    prvCpssDrvPpConfig[devNum]->hwInfo[portGroupId] = *hwInfo;
    drv = hwInfo->driver;
#ifdef ASIC_SIMULATION
    if (drv == NULL)
    {
        if(prvCpssDrvPpConfig[devNum]->mngInterfaceType !=
                                                  CPSS_CHANNEL_PEX_EAGLE_E)
        {
            GT_U32 completion =
                    prvCpssDrvPpConfig[devNum]->hwCtrl[portGroupId].appAddrCompletionRegionsBmp |
                    prvCpssDrvPpConfig[devNum]->hwCtrl[portGroupId].isrAddrCompletionRegionsBmp;
            drv = cpssHwDriverSimulationCreateDrv(hwInfo,
                        (prvCpssDrvPpConfig[devNum]->mngInterfaceType == CPSS_CHANNEL_PEX_MBUS_E)
                        ? completion : 0xffffffff);
        }
        else
        {
            drv = cpssHwDriverEaglePexCreateDrv(hwInfo);
        }
        if (drv)
        {
            char s[16];
            cpssOsSprintf(s, "SIM:0x%x", (GT_U32)hwInfo->resource.switching.start);
            cpssHwDriverRegister(drv, s);
        }
    }
#else
    switch (prvCpssDrvPpConfig[devNum]->mngInterfaceType)
    {
        case CPSS_CHANNEL_PEX_KERNEL_E:
            if (drv == NULL)
                prvCpssDrvPpConfig[devNum]->mngInterfaceType = CPSS_CHANNEL_PEX_E;
            break;
        default:
            break;
    }
    if (drv == NULL)
    {
        char s[32];
        switch (prvCpssDrvPpConfig[devNum]->mngInterfaceType)
        {
            case CPSS_CHANNEL_PCI_E:
            case CPSS_CHANNEL_PEX_E:
                drv = cpssHwDriverPexCreateDrv(hwInfo);
                cpssOsSprintf(s, "PCI%02x:%02x.%d",
                        hwInfo->hwAddr.busNo,
                        hwInfo->hwAddr.devSel,
                        hwInfo->hwAddr.funcNo);
                break;
            case CPSS_CHANNEL_PEX_MBUS_E:
                drv = cpssHwDriverPexMbusCreateDrv(hwInfo,
                    prvCpssDrvPpConfig[devNum]->hwCtrl[portGroupId].appAddrCompletionRegionsBmp |
                    prvCpssDrvPpConfig[devNum]->hwCtrl[portGroupId].isrAddrCompletionRegionsBmp);
                cpssOsSprintf(s, "PEX%02x:%02x.%d",
                        hwInfo->hwAddr.busNo,
                        hwInfo->hwAddr.devSel,
                        hwInfo->hwAddr.funcNo);
                break;
            case CPSS_CHANNEL_PEX_EAGLE_E:
                drv = cpssHwDriverEaglePexCreateDrv(hwInfo);
                cpssOsSprintf(s, "EAGLE%02x:%02x.%d",
                        hwInfo->hwAddr.busNo,
                        hwInfo->hwAddr.devSel,
                        hwInfo->hwAddr.funcNo);

            default:
                break;
        }
        if (drv)
            cpssHwDriverRegister(drv, s);
    }
#endif

    if (drv != NULL)
    {
        char s[32];
        if (cpssDeviceRunCheck_onEmulator())
        {
            cpssHwDriverRegister(drv, s);
            drv = prvCpssHwDriverEmulatorCheckDrv(drv, devNum, portGroupId);
            cpssOsSprintf(s, "EmCheck%d.%d", devNum, portGroupId);
        }
        prvCpssDrvPpConfig[devNum]->drivers[portGroupId] = drv;
        /* TODO: multi-port-group devices: /pp%d.%d  deNum.portGroup */
        cpssHwDriverAddAlias(drv, drvName);
        return GT_OK;
    }
    /* driver not created, fallback to old-style driver, to be removed */
    rc = prvCpssDrvPpObjConfig[devNum]->busPtr->drvHwCntlInit(devNum,portGroupId,isDiag);
    return rc;
}

#ifdef ASIC_SIMULATION
/**
* @internal prvCpssDrvDevNumToSimDevIdConvert function
* @endinternal
*
* @brief   convert CPSS devNum,coreId to simulation deviceId as used by the SCIB
*         layer of the simulation
* @param[in] devNum                   - The PP device number to write to.
* @param[in] portGroupId              - The port group id.
*                                      relevant only to 'multi-port-groups' devices.
*                                      (NOT Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad devNum,portGroupId
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS   prvCpssDrvDevNumToSimDevIdConvert(
    IN  GT_U8       devNum,
    IN  GT_U32      portGroupId,
    OUT GT_U32      *simDeviceIdPtr
)
{
    PRV_CPSS_DRV_CHECK_BUS_FUNC_OBJ_MAC(devNum);
    PRV_CPSS_DRV_CHECK_PORT_GROUP_ID_MAC(devNum, portGroupId);
    CPSS_NULL_PTR_CHECK_MAC(simDeviceIdPtr);

    if(portGroupId >= CPSS_MAX_PORT_GROUPS_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    (*simDeviceIdPtr) = scibGetDeviceId(prvCpssDrvPpConfig[devNum]->hwInfo[portGroupId].resource.switching.start);

    return GT_OK;

}
#endif


/**
* @internal prvCpssDrvHwPpReadRegister function
* @endinternal
*
* @brief   Read a register value from the given PP.
*
* @param[in] devNum                   - The PP to read from.
* @param[in] regAddr                  - The register's address to read from.
*
* @param[out] data                     - Includes the register value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvHwPpReadRegister
(
    IN GT_U8    devNum,
    IN GT_U32   regAddr,
    OUT GT_U32  *data
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, prvCpssDrvHwPpReadRegister);

    CPSS_LOG_API_ENTER_MAC((funcId, devNum, regAddr, data));

    rc = prvCpssDrvHwPpPortGroupReadRegister(
        devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,regAddr,data);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);

    return rc;
}


/**
* @internal prvCpssDrvHwPpWriteRegister function
* @endinternal
*
* @brief   Write to a PP's given register.
*
* @param[in] devNum                   - The PP to write to.
* @param[in] regAddr                  - The register's address to write to.
*                                      data    - The value to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvHwPpWriteRegister
(
    IN GT_U8 devNum,
    IN GT_U32 regAddr,
    IN GT_U32 value
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, prvCpssDrvHwPpWriteRegister);

    CPSS_LOG_API_ENTER_MAC((funcId, devNum, regAddr, value));

    rc = prvCpssDrvHwPpPortGroupWriteRegister(
        devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,regAddr,value);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);

    return rc;
}


/**
* @internal prvCpssDrvHwPpGetRegField function
* @endinternal
*
* @brief   Read a selected register field.
*
* @param[in] devNum                   - The PP device number to read from.
* @param[in] regAddr                  - The register's address to read from.
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be read.
*
* @param[out] fieldData                - Data to read from the register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvHwPpGetRegField
(
    IN GT_U8    devNum,
    IN GT_U32   regAddr,
    IN GT_U32   fieldOffset,
    IN GT_U32   fieldLength,
    OUT GT_U32  *fieldData
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, prvCpssDrvHwPpGetRegField);

    CPSS_LOG_API_ENTER_MAC((funcId, devNum, regAddr, fieldOffset, fieldLength, fieldData));

    rc = prvCpssDrvHwPpPortGroupGetRegField(
        devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
        regAddr,fieldOffset,fieldLength,fieldData);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);

    return rc;
}


/**
* @internal prvCpssDrvHwPpSetRegField function
* @endinternal
*
* @brief   Write value to selected register field.
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] regAddr                  - The register's address to write to.
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be written to register.
* @param[in] fieldData                - Data to be written into the register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note this function actually read the register modifies the requested field
*       and writes the new value back to the HW.
*
*/
GT_STATUS prvCpssDrvHwPpSetRegField
(
    IN GT_U8 devNum,
    IN GT_U32 regAddr,
    IN GT_U32 fieldOffset,
    IN GT_U32 fieldLength,
    IN GT_U32 fieldData

)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, prvCpssDrvHwPpSetRegField);

    CPSS_LOG_API_ENTER_MAC((funcId, devNum, regAddr, fieldOffset, fieldLength, fieldData));

    rc = prvCpssDrvHwPpPortGroupSetRegField(
        devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
        regAddr,fieldOffset,fieldLength,fieldData);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);

    return rc;
}


/**
* @internal prvCpssDrvHwPpReadRegBitMask function
* @endinternal
*
* @brief   Reads the unmasked bits of a register.
*
* @param[in] devNum                   - PP device number to read from.
* @param[in] regAddr                  - Register address to read from.
* @param[in] mask                     - Mask for selecting the read bits.
*
* @param[out] dataPtr                  - Data read from register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note The bits in value to be read are the masked bit of 'mask'.
*
*/
GT_STATUS prvCpssDrvHwPpReadRegBitMask
(
    IN GT_U8    devNum,
    IN GT_U32   regAddr,
    IN GT_U32   mask,
    OUT GT_U32  *dataPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, prvCpssDrvHwPpReadRegBitMask);

    CPSS_LOG_API_ENTER_MAC((funcId, devNum, regAddr, mask, dataPtr));

    rc = prvCpssDrvHwPpPortGroupReadRegBitMask(
        devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
        regAddr,mask,dataPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);

    return rc;
}


/**
* @internal prvCpssDrvHwPpWriteRegBitMask function
* @endinternal
*
* @brief   Writes the unmasked bits of a register.
*
* @param[in] devNum                   - PP device number to write to.
* @param[in] regAddr                  - Register address to write to.
* @param[in] mask                     - Mask for selecting the written bits.
* @param[in] value                    - Data to be written to register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note The bits in value to be written are the masked bit of 'mask'.
*
*/
GT_STATUS prvCpssDrvHwPpWriteRegBitMask
(
    IN GT_U8 devNum,
    IN GT_U32 regAddr,
    IN GT_U32 mask,
    IN GT_U32 value
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, prvCpssDrvHwPpWriteRegBitMask);

    CPSS_LOG_API_ENTER_MAC((funcId, devNum, regAddr, mask, value));

    rc = prvCpssDrvHwPpPortGroupWriteRegBitMask(
        devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
        regAddr,mask,value);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);

    return rc;
}


/**
* @internal prvCpssDrvHwPpReadRam function
* @endinternal
*
* @brief   Read from PP's RAM.
*
* @param[in] devNum                   - The PP device number to read from.
* @param[in] addr                     - Address offset to read from.
* @param[in] length                   - Number of Words (4 byte) to read.
*
* @param[out] data                     - An array containing the read data.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvHwPpReadRam
(
    IN GT_U8    devNum,
    IN GT_U32   addr,
    IN GT_U32   length,
    OUT GT_U32  *data
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, prvCpssDrvHwPpReadRam);

    CPSS_LOG_API_ENTER_MAC((funcId, devNum, addr, length, data));

    rc = prvCpssDrvHwPpPortGroupReadRam(
        devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,addr,length,data);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);

    return rc;

}


/**
* @internal prvCpssDrvHwPpWriteRam function
* @endinternal
*
* @brief   Writes to PP's RAM.
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] addr                     - Address offset to write to.
* @param[in] length                   - Number of Words (4 byte) to write.
* @param[in] data                     - An array containing the  to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvHwPpWriteRam
(
    IN GT_U8  devNum,
    IN GT_U32 addr,
    IN GT_U32 length,
    IN GT_U32 *data
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, prvCpssDrvHwPpWriteRam);

    CPSS_LOG_API_ENTER_MAC((funcId, devNum, addr, length, data));

    rc = prvCpssDrvHwPpPortGroupWriteRam(
        devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,addr,length,data);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);

    return rc;
}


/**
* @internal prvCpssDrvHwPpReadVec function
* @endinternal
*
* @brief   Read from PP's RAM a vector of addresses.
*
* @param[in] devNum                   - The PP device number to read from.
* @param[in] addrArr[]                - Address array to read from.
* @param[in] arrLen                   - The size of addrArr/dataArr.
*
* @param[out] dataArr[]                - An array containing the read data.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvHwPpReadVec
(
    IN GT_U8    devNum,
    IN GT_U32   addrArr[],
    OUT GT_U32  dataArr[],
    IN GT_U32   arrLen
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, prvCpssDrvHwPpReadVec);

    CPSS_LOG_API_ENTER_MAC((funcId, devNum, addrArr, dataArr, arrLen));

    rc = prvCpssDrvHwPpPortGroupReadVec(
        devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,addrArr,dataArr,arrLen);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);

    return rc;
}


/**
* @internal prvCpssDrvHwPpWriteVec function
* @endinternal
*
* @brief   Writes to PP's RAM a vector of addresses.
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] addrArr[]                - Address offset to write to.
* @param[in] dataArr[]                - An array containing the data to be written.
* @param[in] arrLen                   - The size of addrArr/dataArr.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvHwPpWriteVec
(
    IN GT_U8    devNum,
    IN GT_U32   addrArr[],
    IN GT_U32   dataArr[],
    IN GT_U32   arrLen
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, prvCpssDrvHwPpWriteVec);

    CPSS_LOG_API_ENTER_MAC((funcId, devNum, addrArr, dataArr, arrLen));

    rc = prvCpssDrvHwPpPortGroupWriteVec(
        devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,addrArr,dataArr,arrLen);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);

    return rc;
}


/**
* @internal prvCpssDrvHwPpWriteRamInReverse function
* @endinternal
*
* @brief   Writes to PP's RAM in reverse.
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] addr                     - Address offset to write to.
* @param[in] length                   - Number of Words (4 byte) to write.
* @param[in] data                     - An array containing the  to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvHwPpWriteRamInReverse
(
    IN GT_U8 devNum,
    IN GT_U32 addr,
    IN GT_U32 length,
    IN GT_U32 *data
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, prvCpssDrvHwPpWriteRamInReverse);

    CPSS_LOG_API_ENTER_MAC((funcId, devNum, addr, length, data));

    rc =  prvCpssDrvHwPpPortGroupWriteRamInReverse(
        devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,addr,length,data);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);

    return rc;
}

/*******************************************************************************
* prvCpssDrvHwPpIsrRead
*
* DESCRIPTION:
*       Read a register value using special interrupt address completion region.
*
* INPUTS:
*       devNum  - The PP to read from.
*       regAddr - The register's address to read from.
*                 Note: regAddr should be < 0x1000000
*
* OUTPUTS:
*       dataPtr - Includes the register value.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on hardware error
*       GT_NOT_INITIALIZED - if the driver was not initialized
*       GT_HW_ERROR - on hardware error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_INLINE GT_STATUS prvCpssDrvHwPpIsrRead
(
    IN GT_U8 devNum,
    IN GT_U32 regAddr,
    IN GT_U32 *dataPtr
)
{
    return prvCpssDrvHwPpPortGroupIsrRead(
        devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,regAddr,dataPtr);
}
/**
* @internal prvCpssDrvHwPpIsrWrite function
* @endinternal
*
* @brief   Write a register value using special interrupt address completion region
*
* @param[in] devNum                   - The PP to write to.
* @param[in] regAddr                  - The register's address to write to.
*                                      data    - The value to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvHwPpIsrWrite
(
    IN GT_U8 devNum,
    IN GT_U32 regAddr,
    IN GT_U32 value
)
{
    return prvCpssDrvHwPpPortGroupIsrWrite(
        devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,regAddr,value);
}
/**
* @internal prvCpssDrvHwPpReadInternalPciReg function
* @endinternal
*
* @brief   This function reads from an internal pci register, it's used by the
*         initialization process and the interrupt service routine.
* @param[in] devNum                   - The Pp's device numbers.
* @param[in] regAddr                  - The register's address to read from.
*
* @param[out] data                     - The read data.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvHwPpReadInternalPciReg
(
    IN  GT_U8   devNum,
    IN  GT_U32  regAddr,
    OUT GT_U32  *data
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, prvCpssDrvHwPpReadInternalPciReg);

    CPSS_LOG_API_ENTER_MAC((funcId, devNum, regAddr, data));

    rc = prvCpssDrvHwPpPortGroupReadInternalPciReg(
        devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,regAddr,data);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);

    return rc;
}


/**
* @internal prvCpssDrvHwPpWriteInternalPciReg function
* @endinternal
*
* @brief   This function reads from an internal pci register, it's used by the
*         initialization process and the interrupt service routine.
* @param[in] devNum                   - The Pp's device numbers.
* @param[in] regAddr                  - The register's address to read from.
* @param[in] data                     - Data to be written.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvHwPpWriteInternalPciReg
(
    IN  GT_U8   devNum,
    IN  GT_U32  regAddr,
    IN  GT_U32  data
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, prvCpssDrvHwPpWriteInternalPciReg);

    CPSS_LOG_API_ENTER_MAC((funcId, devNum, regAddr, data));

    rc = prvCpssDrvHwPpPortGroupWriteInternalPciReg(
        devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,regAddr,data);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);

    return rc;
}

/**
* @internal prvCpssDrvHwPpResetAndInitControllerReadReg function
* @endinternal
*
* @brief   Read a register value from the Reset and Init Controller.
*
* @param[in] devNum                   - The PP to read from.
* @param[in] regAddr                  - The register's address to read from.
*
* @param[out] data                     - Includes the register value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvHwPpResetAndInitControllerReadReg
(
    IN  GT_U8   devNum,
    IN  GT_U32  regAddr,
    OUT GT_U32  *data
)
{
    GT_STATUS rc;
    CPSS_HW_DRIVER_STC *drv;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, prvCpssDrvHwPpResetAndInitControllerReadReg);

    CPSS_LOG_API_ENTER_MAC((funcId, devNum, regAddr, data));

    PRV_CPSS_DRV_CHECK_GET_PP_DRV(devNum,CPSS_NON_MULTI_PORT_GROUP_DEVICE_INDEX_CNS,drv);

    if (drv == NULL)
    {
        rc = prvCpssDrvPpObjConfig[devNum]->busPtr->
                    drvHwPpResetAndInitControllerReadReg(devNum, regAddr, data);

        CPSS_LOG_API_EXIT_MAC(funcId, rc);
        return rc;
    }

    rc = prvCpssDrvHwPpDoReadOpDrv(devNum, 0, drv,
        CPSS_HW_DRIVER_AS_RESET_AND_INIT_CONTROLLER_E,
        GT_FALSE, regAddr, data);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);

    return rc;
}


/**
* @internal prvCpssDrvHwPpResetAndInitControllerGetRegField function
* @endinternal
*
* @brief   Read a selected register field from the Reset and Init Controller.
*
* @param[in] devNum                   - The PP device number to read from.
* @param[in] regAddr                  - The register's address to read from.
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be read.
*
* @param[out] fieldData                - Data to read from the register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvHwPpResetAndInitControllerGetRegField
 (
    IN  GT_U8   devNum,
    IN GT_U32   regAddr,
    IN GT_U32   fieldOffset,
    IN GT_U32   fieldLength,
    OUT GT_U32  *fieldData
)
{
    GT_STATUS rc;
    CPSS_HW_DRIVER_STC *drv;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, prvCpssDrvHwPpResetAndInitControllerGetRegField);

    CPSS_LOG_API_ENTER_MAC((funcId, devNum,  regAddr, fieldOffset, fieldLength, fieldData));

    PRV_CPSS_DRV_CHECK_GET_PP_DRV(devNum,CPSS_NON_MULTI_PORT_GROUP_DEVICE_INDEX_CNS,drv);

    if (drv == NULL)
    {
        rc = prvCpssDrvPpObjConfig[devNum]->busPtr->
            drvHwPpResetAndInitControllerGetRegField(
                                                devNum, regAddr,
                                                fieldOffset,fieldLength,fieldData);
        CPSS_LOG_API_EXIT_MAC(funcId, rc);
        return rc;
    }

    rc = prvCpssDrvHwPpGetRegFieldDrv(devNum, 0, drv,
            CPSS_HW_DRIVER_AS_RESET_AND_INIT_CONTROLLER_E,
            GT_FALSE, regAddr,fieldOffset,fieldLength,fieldData);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);

    return rc;
}


/**
* @internal prvCpssDrvHwPpResetAndInitControllerWriteReg function
* @endinternal
*
* @brief   Write to the Reset and Init Controller given register.
*
* @param[in] devNum                   - The PP to write to.
* @param[in] regAddr                  - The register's address to write to.
* @param[in] data                     - The value to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvHwPpResetAndInitControllerWriteReg
(
    IN  GT_U8   devNum,
    IN  GT_U32  regAddr,
    IN  GT_U32  data
)
{
    GT_STATUS rc;
    CPSS_HW_DRIVER_STC *drv;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, prvCpssDrvHwPpResetAndInitControllerWriteReg);

    CPSS_LOG_API_ENTER_MAC((funcId, devNum, regAddr, data));


    PRV_CPSS_DRV_CHECK_GET_PP_DRV(devNum,CPSS_NON_MULTI_PORT_GROUP_DEVICE_INDEX_CNS,drv);

    if (drv == NULL)
    {
        rc = prvCpssDrvPpObjConfig[devNum]->busPtr->
                       drvHwPpResetAndInitControllerWriteReg(devNum, regAddr, data);
        CPSS_LOG_API_EXIT_MAC(funcId, rc);
        return rc;
    }

    rc = prvCpssDrvHwPpWriteRegBitMaskDrv(devNum, 0, drv,
            CPSS_HW_DRIVER_AS_RESET_AND_INIT_CONTROLLER_E,
            GT_FALSE, regAddr, 0xffffffff, data);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);

    return rc;
}


/**
* @internal prvCpssDrvHwPpResetAndInitControllerSetRegField function
* @endinternal
*
* @brief   Write value to selected register field of the Reset and Init Controller.
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] regAddr                  - The register's address to write to.
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be written to register.
* @param[in] fieldData                - Data to be written into the register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note this function actually read the register modifies the requested field
*       and writes the new value back to the HW.
*
*/
GT_STATUS prvCpssDrvHwPpResetAndInitControllerSetRegField
 (
    IN  GT_U8   devNum,
    IN GT_U32   regAddr,
    IN GT_U32   fieldOffset,
    IN GT_U32   fieldLength,
    IN GT_U32   fieldData
)
{
    GT_STATUS rc;
    CPSS_HW_DRIVER_STC *drv;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, prvCpssDrvHwPpResetAndInitControllerSetRegField);

    CPSS_LOG_API_ENTER_MAC((funcId, devNum, regAddr, fieldOffset, fieldLength, fieldData));

    PRV_CPSS_DRV_CHECK_GET_PP_DRV(devNum,CPSS_NON_MULTI_PORT_GROUP_DEVICE_INDEX_CNS,drv);

    if (drv == NULL)
    {
        rc = prvCpssDrvPpObjConfig[devNum]->busPtr->
            drvHwPpResetAndInitControllerSetRegField(
                                                devNum, regAddr,
                                                fieldOffset,fieldLength,fieldData);
        CPSS_LOG_API_EXIT_MAC(funcId, rc);
        return rc;
    }

    rc = prvCpssDrvHwPpSetRegFieldDrv(devNum, 0, drv,
            CPSS_HW_DRIVER_AS_RESET_AND_INIT_CONTROLLER_E,
            GT_FALSE, regAddr, fieldOffset,fieldLength,fieldData);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);

    return rc;
}

/*******************************************************************************
* prvCpssDrvHwPpPortGroupGetDrv
*
* DESCRIPTION:
*       Read a register value from the given PP.
*       in the specific port group in the device
* INPUTS:
*       devNum      - The PP to read from.
*       portGroupId - The port group id. relevant only to 'multi-port-groups'
*                     devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*                          (APPLICABLE DEVICES: Lion; Lion2)
*
* OUTPUTS:
*       None
*
* RETURNS:
*       pointer to device driver or NULL
*
* COMMENTS:
*       None.
*
*******************************************************************************/
CPSS_HW_DRIVER_STC* prvCpssDrvHwPpPortGroupGetDrv
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId
)
{
    PRV_CPSS_DRV_PORT_GROUPS_INFO_STC *pgInfo;

    if (prvCpssDrvPpConfig[devNum] == NULL )
       return 0;
    pgInfo = &(prvCpssDrvPpConfig[devNum]->portGroupsInfo);
    if(pgInfo->isMultiPortGroupDevice == GT_TRUE)
    {
        if(pgInfo->debugForcedPortGroupId != CPSS_PORT_GROUP_UNAWARE_MODE_CNS)
            portGroupId = pgInfo->debugForcedPortGroupId;

        if((portGroupId) == CPSS_PORT_GROUP_UNAWARE_MODE_CNS)
        {   /*use the first active port group */
            portGroupId = pgInfo->firstActivePortGroup;
        }
        else if(0 == PRV_CPSS_DRV_IS_IN_RANGE_PORT_GROUP_ID_MAC(devNum,portGroupId) ||
               (0 == (pgInfo->activePortGroupsBmp & (1<<(portGroupId)))))
        {   /* non active port group */
            return NULL;
        }
    }
    else  /*use the only 'active port group' */
    {
        portGroupId = pgInfo->firstActivePortGroup;
    }
    return prvCpssDrvPpConfig[devNum]->drivers[portGroupId];
}

/**
* @internal prvCpssDrvHwPpPortGroupReadRegister function
* @endinternal
*
* @brief   Read a register value from the given PP.
*         in the specific port group in the device
* @param[in] devNum                   - The PP to read from.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*                                      (APPLICABLE DEVICES Lion; Lion2)
* @param[in] regAddr                  - The register's address to read from.
*
* @param[out] data                     - Includes the register value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note
*       Galtis:
*       None.
*
*/
GT_STATUS prvCpssDrvHwPpPortGroupReadRegister
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    OUT GT_U32  *data
)
{
    GT_STATUS rc = GT_OK;
    CPSS_HW_DRIVER_STC *drv;
    GT_BOOL skipHwAccess = GT_FALSE;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, prvCpssDrvHwPpPortGroupReadRegister);

    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupId, regAddr, data));

    PRV_CPSS_DRV_CHECK_PP_DEV_MAC(devNum);
    PRV_CPSS_DRV_CHECK_ACTIVE_PORT_GROUP_ID_FOR_READ_OPERATION_MAC(
        devNum, portGroupId);
    PRV_CPSS_DRV_GET_PP_DRV(devNum,portGroupId,drv);

    if (prvCpssDrvPpConfig[devNum]->errata.addrCheckFuncPtr != NULL)
    {
        /* check address */
        rc = prvCpssDrvPpConfig[devNum]->errata.addrCheckFuncPtr(devNum,
                                                                 portGroupId,
                                                                 regAddr,
                                                                 1);
        if (rc != GT_OK)
        {
            CPSS_LOG_API_EXIT_MAC(funcId, rc);
            return rc;
        }
    }

    /* Check if HW access required. */
    if(prvCpssHwAccessObj.hwAccessRegisterReadFunc != NULL)
    {
       rc = prvCpssHwAccessObj.hwAccessRegisterReadFunc(
                                                  devNum, portGroupId,
                                                  regAddr, data,
                                                  CPSS_DRV_HW_ACCESS_STAGE_PRE_E);
       if(rc == GT_ABORTED)
       {
           CPSS_LOG_API_EXIT_MAC(funcId, GT_OK);
           return GT_OK;
       }

       if(rc!= GT_OK)
       {
           CPSS_LOG_API_ERROR_EXIT_AND_RETURN_MAC(funcId, rc);
       }
    }

    if (prvCpssDrvPpConfig[devNum]->errata.callbackFuncPtr != NULL)
    {
        rc = prvCpssDrvPpConfig[devNum]->errata.callbackFuncPtr(
            devNum, portGroupId,
            prvCpssDrvPpConfig[devNum]->errata.callbackDataPtr,
            (PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_PREPARE_E |
             PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_REGISTER_E |
             PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_READ_E),
            regAddr/*addr*/, 1/*length in words*/,
            0/*param0*/, 0/*param1*/, data/*inoutDataPtr*/, &skipHwAccess);
        if (rc!= GT_OK)
        {
            CPSS_LOG_API_ERROR_EXIT_AND_RETURN_MAC(funcId, rc);
        }
    }
    if (skipHwAccess == GT_FALSE)
    {
        if (drv == NULL)
        {
            rc = prvCpssDrvPpObjConfig[devNum]->busPtr->drvHwPpReadReg(
                devNum, portGroupId, regAddr, data);
        }
        else
        {
            rc = prvCpssDrvHwPpDoReadOpDrv(devNum, portGroupId, drv,
                    CPSS_HW_DRIVER_AS_SWITCHING_E,
                    GT_FALSE, regAddr, data);
        }
    }
    if (prvCpssDrvPpConfig[devNum]->errata.callbackFuncPtr != NULL)
    {
        rc = prvCpssDrvPpConfig[devNum]->errata.callbackFuncPtr(
            devNum, portGroupId,
            prvCpssDrvPpConfig[devNum]->errata.callbackDataPtr,
            (PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_COMPLETE_E |
             PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_REGISTER_E |
             PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_READ_E),
            regAddr/*addr*/, 1/*length in words*/,
             0/*param0*/, 0/*param1*/, data/*inoutDataPtr*/, &skipHwAccess);
        if (rc!= GT_OK)
        {
            CPSS_LOG_API_ERROR_EXIT_AND_RETURN_MAC(funcId, rc);
        }
    }
    /* Check if HW access required. */
    if(prvCpssHwAccessObj.hwAccessRegisterReadFunc != NULL)
    {
       prvCpssHwAccessObj.hwAccessRegisterReadFunc(
                                                  devNum, portGroupId,
                                                  regAddr, data,
                                                  CPSS_DRV_HW_ACCESS_STAGE_POST_E);
    }

    CPSS_LOG_API_EXIT_MAC(funcId, rc);

    return rc;
}

/**
* @internal prvCpssDrvHwPpPortGroupWriteRegister function
* @endinternal
*
* @brief   Write to a PP's given register.
*         in the specific port group in the device
* @param[in] devNum                   - The PP to write to.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*                                      (APPLICABLE DEVICES Lion; Lion2)
* @param[in] regAddr                  - The register's address to write to.
*                                      data        - The value to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note
*       Galtis:
*       None.
*
*/
GT_STATUS prvCpssDrvHwPpPortGroupWriteRegister
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   value
)
{
    GT_STATUS rc = GT_OK;
    CPSS_HW_DRIVER_STC *drv;
    GT_BOOL skipHwAccess = GT_FALSE;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, prvCpssDrvHwPpPortGroupWriteRegister);

    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupId, regAddr, value));

    PRV_CPSS_DRV_CHECK_PP_DEV_MAC(devNum);
    PRV_CPSS_DRV_CHECK_PORT_GROUP_ID_MAC(devNum,portGroupId);

    if (prvCpssDrvPpConfig[devNum]->errata.addrCheckFuncPtr != NULL)
    {
        /* check address */
        rc = prvCpssDrvPpConfig[devNum]->errata.addrCheckFuncPtr(devNum,
                                                                 portGroupId,
                                                                 regAddr,
                                                                 1);
        if (rc != GT_OK)
        {
            CPSS_LOG_API_EXIT_MAC(funcId, rc);
            return rc;
        }
    }

    /* Check if HW access required. */
    if(prvCpssHwAccessObj.hwAccessRegisterWriteFunc != NULL)
    {
       rc = prvCpssHwAccessObj.hwAccessRegisterWriteFunc(
                                                  devNum, portGroupId,
                                                  regAddr, value,
                                                  CPSS_DRV_HW_ACCESS_STAGE_PRE_E);
       if(rc == GT_ABORTED)
       {
           CPSS_LOG_API_EXIT_MAC(funcId, GT_OK);
           return GT_OK;
       }

       if(rc!= GT_OK)
       {
           CPSS_LOG_API_ERROR_EXIT_AND_RETURN_MAC(funcId, rc);
       }
    }

    PRV_CPSS_DRV_START_HANDLE_PORT_GROUPS_MAC(devNum,portGroupId)
    {
        if (prvCpssDrvPpConfig[devNum]->errata.callbackFuncPtr != NULL)
        {
            rc = prvCpssDrvPpConfig[devNum]->errata.callbackFuncPtr(
                devNum, portGroupId,
                prvCpssDrvPpConfig[devNum]->errata.callbackDataPtr,
                (PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_PREPARE_E |
                 PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_REGISTER_E |
                 PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_WRITE_E),
                regAddr/*addr*/, 1/*length in words*/,
                 0/*param0*/, 0/*param1*/, &value/*inoutDataPtr*/, &skipHwAccess);
            if (rc!= GT_OK)
            {
                CPSS_LOG_API_ERROR_EXIT_AND_RETURN_MAC(funcId, rc);
            }
        }
        if (skipHwAccess == GT_FALSE)
        {
            drv = prvCpssDrvPpConfig[devNum]->drivers[portGroupId];
            if (drv == NULL)
            {
                rc = prvCpssDrvPpObjConfig[devNum]->busPtr->drvHwPpWriteReg(
                    devNum,portGroupId,regAddr,value);
            }
            else
            {
                rc = prvCpssDrvHwPpWriteRegBitMaskDrv(devNum, portGroupId, drv,
                        CPSS_HW_DRIVER_AS_SWITCHING_E,
                        GT_FALSE, regAddr, 0xffffffff, value);
            }
        }
        if (prvCpssDrvPpConfig[devNum]->errata.callbackFuncPtr != NULL)
        {
            rc = prvCpssDrvPpConfig[devNum]->errata.callbackFuncPtr(
                devNum, portGroupId,
                prvCpssDrvPpConfig[devNum]->errata.callbackDataPtr,
                (PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_COMPLETE_E |
                 PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_REGISTER_E |
                 PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_WRITE_E),
                regAddr/*addr*/, 1/*length in words*/,
                 0/*param0*/, 0/*param1*/, &value/*inoutDataPtr*/, &skipHwAccess);
            if (rc != GT_OK)
            {
                CPSS_LOG_API_ERROR_EXIT_AND_RETURN_MAC(funcId, rc);
            }
        }
        if(rc != GT_OK)
        {
            CPSS_LOG_API_EXIT_MAC(funcId, rc);
            return rc;
        }
    }
    PRV_CPSS_DRV_END_HANDLE_PORT_GROUPS_MAC(devNum,portGroupId)

    /* Check if HW access required. */
    if(prvCpssHwAccessObj.hwAccessRegisterWriteFunc != NULL)
    {
       prvCpssHwAccessObj.hwAccessRegisterWriteFunc(
                                                  devNum, portGroupId,
                                                  regAddr, value,
                                                  CPSS_DRV_HW_ACCESS_STAGE_POST_E);
    }

    CPSS_LOG_API_EXIT_MAC(funcId, rc);

    return rc;
}



/**
* @internal prvCpssDrvHwPpPortGroupGetRegField function
* @endinternal
*
* @brief   Read a selected register field.
*         in the specific port group in the device
* @param[in] devNum                   - The PP device number to read from.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*                                      (APPLICABLE DEVICES Lion; Lion2)
* @param[in] regAddr                  - The register's address to read from.
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be read.
*
* @param[out] fieldData                - Data to read from the register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note
*       Galtis:
*       None.
*
*/
GT_STATUS prvCpssDrvHwPpPortGroupGetRegField
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   fieldOffset,
    IN GT_U32   fieldLength,
    OUT GT_U32  *fieldData
)
{
    GT_STATUS rc = GT_OK;
    CPSS_HW_DRIVER_STC *drv;
    GT_BOOL skipHwAccess = GT_FALSE;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, prvCpssDrvHwPpPortGroupGetRegField);

    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupId, regAddr, fieldOffset, fieldLength, fieldData));

    PRV_CPSS_DRV_CHECK_PP_DEV_MAC(devNum);
    PRV_CPSS_DRV_CHECK_ACTIVE_PORT_GROUP_ID_FOR_READ_OPERATION_MAC(
        devNum, portGroupId);
    PRV_CPSS_DRV_GET_PP_DRV(devNum,portGroupId,drv);

    if (prvCpssDrvPpConfig[devNum]->errata.addrCheckFuncPtr != NULL)
    {
        /* check address */
        rc = prvCpssDrvPpConfig[devNum]->errata.addrCheckFuncPtr(devNum,
                                                                 portGroupId,
                                                                 regAddr,
                                                                 1);
        if (rc != GT_OK)
        {
            CPSS_LOG_API_EXIT_MAC(funcId, rc);
            return rc;
        }
    }

    /* Check if HW access required. */
    if(prvCpssHwAccessObj.hwAccessRegisterFieldReadFunc != NULL)
    {
       rc = prvCpssHwAccessObj.hwAccessRegisterFieldReadFunc(
                                                  devNum, portGroupId, regAddr,
                                                  fieldOffset, fieldLength, fieldData,
                                                  CPSS_DRV_HW_ACCESS_STAGE_PRE_E);
        if(rc!= GT_OK)
        {
            CPSS_LOG_API_EXIT_MAC(funcId, rc);
            return rc;
        }
    }

    if (prvCpssDrvPpConfig[devNum]->errata.callbackFuncPtr != NULL)
    {
        rc = prvCpssDrvPpConfig[devNum]->errata.callbackFuncPtr(
            devNum, portGroupId,
            prvCpssDrvPpConfig[devNum]->errata.callbackDataPtr,
            (PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_PREPARE_E |
             PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_REGISTER_E |
             PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_GET_FIELD_E),
            regAddr/*addr*/, 1/*length in words*/,
            fieldOffset/*param0*/, fieldLength/*param1*/, fieldData/*inoutDataPtr*/, &skipHwAccess);
        if (rc != GT_OK)
        {
            CPSS_LOG_API_ERROR_EXIT_AND_RETURN_MAC(funcId, rc);
        }
    }
    if (skipHwAccess == GT_FALSE)
    {
        if (drv == NULL)
        {
            rc = prvCpssDrvPpObjConfig[devNum]->busPtr->drvHwPpGetRegField(
                devNum, portGroupId,
                regAddr,fieldOffset,fieldLength,fieldData);
        }
        else
        {
            rc = prvCpssDrvHwPpGetRegFieldDrv(devNum, portGroupId, drv,
                CPSS_HW_DRIVER_AS_SWITCHING_E,
                GT_FALSE, regAddr,fieldOffset,fieldLength,fieldData);
        }
    }
    if (prvCpssDrvPpConfig[devNum]->errata.callbackFuncPtr != NULL)
    {
        rc = prvCpssDrvPpConfig[devNum]->errata.callbackFuncPtr(
            devNum, portGroupId,
            prvCpssDrvPpConfig[devNum]->errata.callbackDataPtr,
            (PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_COMPLETE_E |
             PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_REGISTER_E |
             PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_GET_FIELD_E),
            regAddr/*addr*/, 1/*length in words*/,
            fieldOffset/*param0*/, fieldLength/*param1*/, fieldData/*inoutDataPtr*/, &skipHwAccess);
        if (rc != GT_OK)
        {
            CPSS_LOG_API_ERROR_EXIT_AND_RETURN_MAC(funcId, rc);
        }
    }

     /* Check if HW access required. */
     if(prvCpssHwAccessObj.hwAccessRegisterFieldReadFunc != NULL)
     {
         rc = prvCpssHwAccessObj.hwAccessRegisterFieldReadFunc(
                                                    devNum, portGroupId, regAddr,
                                                    fieldOffset, fieldLength, fieldData,
                                                    CPSS_DRV_HW_ACCESS_STAGE_POST_E);
     }

     CPSS_LOG_API_EXIT_MAC(funcId, rc);

     return rc;
}

/**
* @internal prvCpssDrvHwPpPortGroupSetRegField function
* @endinternal
*
* @brief   Write value to selected register field.
*         in the specific port group in the device
* @param[in] devNum                   - The PP device number to write to.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*                                      (APPLICABLE DEVICES Lion; Lion2)
* @param[in] regAddr                  - The register's address to write to.
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be written to register.
* @param[in] fieldData                - Data to be written into the register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note
*       Galtis:
*       None.
*
*/
GT_STATUS prvCpssDrvHwPpPortGroupSetRegField
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   fieldOffset,
    IN GT_U32   fieldLength,
    IN GT_U32   fieldData
)
{
    GT_STATUS rc = GT_OK;
    CPSS_HW_DRIVER_STC *drv;
    GT_BOOL skipHwAccess = GT_FALSE;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, prvCpssDrvHwPpPortGroupSetRegField);

    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupId, regAddr, fieldOffset, fieldLength, fieldData));

    PRV_CPSS_DRV_CHECK_PP_DEV_MAC(devNum);
    PRV_CPSS_DRV_CHECK_PORT_GROUP_ID_MAC(devNum,portGroupId);

    if (prvCpssDrvPpConfig[devNum]->errata.addrCheckFuncPtr != NULL)
    {
        /* check address */
        rc = prvCpssDrvPpConfig[devNum]->errata.addrCheckFuncPtr(devNum,
                                                                 portGroupId,
                                                                 regAddr,
                                                                 1);
        if (rc != GT_OK)
        {
            CPSS_LOG_API_EXIT_MAC(funcId, rc);
            return rc;
        }
    }

    /* Check if HW access required. */
    if(prvCpssHwAccessObj.hwAccessRegisterFieldWriteFunc != NULL)
    {
       rc = prvCpssHwAccessObj.hwAccessRegisterFieldWriteFunc(
                                                  devNum, portGroupId, regAddr,
                                                  fieldOffset, fieldLength, fieldData,
                                                  CPSS_DRV_HW_ACCESS_STAGE_PRE_E);
       if(rc == GT_ABORTED)
       {
           CPSS_LOG_API_EXIT_MAC(funcId, GT_OK);
           return GT_OK;
       }

       if(rc!= GT_OK)
       {
           CPSS_LOG_API_ERROR_EXIT_AND_RETURN_MAC(funcId, rc);
       }
    }

    PRV_CPSS_DRV_START_HANDLE_PORT_GROUPS_MAC(devNum,portGroupId)
    {
        if (prvCpssDrvPpConfig[devNum]->errata.callbackFuncPtr != NULL)
        {
            rc = prvCpssDrvPpConfig[devNum]->errata.callbackFuncPtr(
                devNum, portGroupId,
                prvCpssDrvPpConfig[devNum]->errata.callbackDataPtr,
                (PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_PREPARE_E |
                 PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_REGISTER_E |
                 PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_SET_FIELD_E),
                regAddr/*addr*/, 1/*length in words*/,
                fieldOffset/*param0*/, fieldLength/*param1*/, &fieldData/*inoutDataPtr*/, &skipHwAccess);
            if (rc != GT_OK)
            {
                CPSS_LOG_API_ERROR_EXIT_AND_RETURN_MAC(funcId, rc);
            }
        }
        if (skipHwAccess == GT_FALSE)
        {
            drv = prvCpssDrvPpConfig[devNum]->drivers[portGroupId];
            if (drv == NULL)
            {
                rc = prvCpssDrvPpObjConfig[devNum]->busPtr->drvHwPpSetRegField(
                    devNum, portGroupId,
                    regAddr,fieldOffset,fieldLength,fieldData);
            }
            else
            {
                rc = prvCpssDrvHwPpSetRegFieldDrv(devNum, portGroupId, drv,
                    CPSS_HW_DRIVER_AS_SWITCHING_E,
                    GT_FALSE, regAddr,fieldOffset,fieldLength,fieldData);
            }
        }
        if (prvCpssDrvPpConfig[devNum]->errata.callbackFuncPtr != NULL)
        {
            rc = prvCpssDrvPpConfig[devNum]->errata.callbackFuncPtr(
                devNum, portGroupId,
                prvCpssDrvPpConfig[devNum]->errata.callbackDataPtr,
                (PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_COMPLETE_E |
                 PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_REGISTER_E |
                 PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_SET_FIELD_E),
                regAddr/*addr*/, 1/*length in words*/,
                fieldOffset/*param0*/, fieldLength/*param1*/, &fieldData/*inoutDataPtr*/, &skipHwAccess);
            if (rc != GT_OK)
            {
                CPSS_LOG_API_ERROR_EXIT_AND_RETURN_MAC(funcId, rc);
            }
        }
        if(rc != GT_OK)
        {
            CPSS_LOG_API_EXIT_MAC(funcId, rc);
            return rc;
        }
    }
    PRV_CPSS_DRV_END_HANDLE_PORT_GROUPS_MAC(devNum, portGroupId)

    /* Check if HW access required. */
    if(prvCpssHwAccessObj.hwAccessRegisterFieldWriteFunc != NULL)
    {
        prvCpssHwAccessObj.hwAccessRegisterFieldWriteFunc(
                                                   devNum, portGroupId, regAddr,
                                                   fieldOffset, fieldLength, fieldData,
                                                   CPSS_DRV_HW_ACCESS_STAGE_POST_E);
    }

    CPSS_LOG_API_EXIT_MAC(funcId, rc);

    return rc;
}

/**
* @internal prvCpssDrvHwPpPortGroupReadRegBitMask function
* @endinternal
*
* @brief   Reads the unmasked bits of a register.
*         in the specific port group in the device
* @param[in] devNum                   - PP device number to read from.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*                                      (APPLICABLE DEVICES Lion; Lion2)
* @param[in] regAddr                  - Register address to read from.
* @param[in] mask                     - Mask for selecting the read bits.
*
* @param[out] dataPtr                  - Data read from register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note The bits in value to be read are the masked bit of 'mask'.
*       GalTis:
*       Command - hwPpReadRegBitMask
*
*/
GT_STATUS prvCpssDrvHwPpPortGroupReadRegBitMask
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   mask,
    OUT GT_U32  *dataPtr
)
{
    GT_STATUS rc = GT_OK;
    CPSS_HW_DRIVER_STC *drv;
    GT_BOOL skipHwAccess = GT_FALSE;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, prvCpssDrvHwPpPortGroupReadRegBitMask);

    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupId, regAddr, mask, dataPtr));

    PRV_CPSS_DRV_CHECK_PP_DEV_MAC(devNum);
    PRV_CPSS_DRV_CHECK_ACTIVE_PORT_GROUP_ID_FOR_READ_OPERATION_MAC(
        devNum, portGroupId);
    PRV_CPSS_DRV_GET_PP_DRV(devNum,portGroupId,drv);

    if (prvCpssDrvPpConfig[devNum]->errata.addrCheckFuncPtr != NULL)
    {
        /* check address */
        rc = prvCpssDrvPpConfig[devNum]->errata.addrCheckFuncPtr(devNum,
                                                                 portGroupId,
                                                                 regAddr,
                                                                 1);
        if (rc != GT_OK)
        {
            CPSS_LOG_API_EXIT_MAC(funcId, rc);
            return rc;
        }
    }

    /* Check if HW access required. */
    if(prvCpssHwAccessObj.hwAccessRegisterBitMaskReadFunc != NULL)
    {
       rc = prvCpssHwAccessObj.hwAccessRegisterBitMaskReadFunc(
                                                  devNum, portGroupId, regAddr,
                                                  mask, dataPtr,
                                                  CPSS_DRV_HW_ACCESS_STAGE_PRE_E);
       if(rc == GT_ABORTED)
       {
           CPSS_LOG_API_EXIT_MAC(funcId, GT_OK);
           return GT_OK;
       }

       if(rc!= GT_OK)
       {
           CPSS_LOG_API_ERROR_EXIT_AND_RETURN_MAC(funcId, rc);
       }
    }

    if (prvCpssDrvPpConfig[devNum]->errata.callbackFuncPtr != NULL)
    {
        rc = prvCpssDrvPpConfig[devNum]->errata.callbackFuncPtr(
            devNum, portGroupId,
            prvCpssDrvPpConfig[devNum]->errata.callbackDataPtr,
            (PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_PREPARE_E |
             PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_REGISTER_E |
             PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_READ_MASK_E),
            regAddr/*addr*/, 1/*length in words*/,
            mask/*param0*/, 0/*param1*/, dataPtr/*inoutDataPtr*/, &skipHwAccess);
        if (rc != GT_OK)
        {
            CPSS_LOG_API_ERROR_EXIT_AND_RETURN_MAC(funcId, rc);
        }
    }
    if (skipHwAccess == GT_FALSE)
    {
        if (drv == NULL)
        {
            rc = prvCpssDrvPpObjConfig[devNum]->busPtr->drvHwPpReadRegBitMask(
                devNum, portGroupId, regAddr, mask, dataPtr);
        }
        else
        {
            rc = prvCpssDrvHwPpReadRegBitMaskDrv(devNum, portGroupId, drv,
                CPSS_HW_DRIVER_AS_SWITCHING_E,
                GT_FALSE, regAddr, mask, dataPtr);
        }
    }
    if (prvCpssDrvPpConfig[devNum]->errata.callbackFuncPtr != NULL)
    {
        rc = prvCpssDrvPpConfig[devNum]->errata.callbackFuncPtr(
            devNum, portGroupId,
            prvCpssDrvPpConfig[devNum]->errata.callbackDataPtr,
            (PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_COMPLETE_E |
             PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_REGISTER_E |
             PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_READ_MASK_E),
            regAddr/*addr*/, 1/*length in words*/,
            mask/*param0*/, 0/*param1*/, dataPtr/*inoutDataPtr*/, &skipHwAccess);
        if (rc != GT_OK)
        {
            CPSS_LOG_API_ERROR_EXIT_AND_RETURN_MAC(funcId, rc);
        }
    }
    /* Check if HW access required. */
    if(prvCpssHwAccessObj.hwAccessRegisterBitMaskReadFunc != NULL)
    {
       prvCpssHwAccessObj.hwAccessRegisterBitMaskReadFunc(
                                                  devNum, portGroupId, regAddr,
                                                  mask, dataPtr,
                                                  CPSS_DRV_HW_ACCESS_STAGE_POST_E);
    }


    CPSS_LOG_API_EXIT_MAC(funcId, rc);

    return rc;
}

/**
* @internal prvCpssDrvHwPpPortGroupWriteRegBitMask function
* @endinternal
*
* @brief   Writes the unmasked bits of a register.
*         in the specific port group in the device
* @param[in] devNum                   - PP device number to write to.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*                                      (APPLICABLE DEVICES Lion; Lion2)
* @param[in] regAddr                  - Register address to write to.
* @param[in] mask                     - Mask for selecting the written bits.
* @param[in] value                    - Data to be written to register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note The bits in value to be written are the masked bit of 'mask'.
*       GalTis:
*       Command - hwPpWriteRegBitMask
*
*/
GT_STATUS prvCpssDrvHwPpPortGroupWriteRegBitMask
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   mask,
    IN GT_U32   value
)
{
    GT_STATUS rc = GT_OK;
    CPSS_HW_DRIVER_STC *drv;
    GT_BOOL skipHwAccess = GT_FALSE;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, prvCpssDrvHwPpPortGroupWriteRegBitMask);

    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupId, regAddr, mask, value));

    PRV_CPSS_DRV_CHECK_PP_DEV_MAC(devNum);
    PRV_CPSS_DRV_CHECK_PORT_GROUP_ID_MAC(devNum, portGroupId);

    if (prvCpssDrvPpConfig[devNum]->errata.addrCheckFuncPtr != NULL)
    {
        /* check address */
        rc = prvCpssDrvPpConfig[devNum]->errata.addrCheckFuncPtr(devNum,
                                                                 portGroupId,
                                                                 regAddr,
                                                                 1);
        if (rc != GT_OK)
        {
            CPSS_LOG_API_EXIT_MAC(funcId, rc);
            return rc;
        }
    }

    /* Check if HW access required. */
    if(prvCpssHwAccessObj.hwAccessRegisterBitMaskWriteFunc != NULL)
    {
       rc = prvCpssHwAccessObj.hwAccessRegisterBitMaskWriteFunc(
                                                  devNum, portGroupId, regAddr,
                                                  mask, value,
                                                  CPSS_DRV_HW_ACCESS_STAGE_PRE_E);
       if(rc == GT_ABORTED)
       {
           CPSS_LOG_API_EXIT_MAC(funcId, GT_OK);
           return GT_OK;
       }

       if(rc!= GT_OK)
       {
           CPSS_LOG_API_ERROR_EXIT_AND_RETURN_MAC(funcId, rc);
       }
    }

    PRV_CPSS_DRV_START_HANDLE_PORT_GROUPS_MAC(devNum, portGroupId)
    {
        if (prvCpssDrvPpConfig[devNum]->errata.callbackFuncPtr != NULL)
        {
            rc = prvCpssDrvPpConfig[devNum]->errata.callbackFuncPtr(
                devNum, portGroupId,
                prvCpssDrvPpConfig[devNum]->errata.callbackDataPtr,
                (PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_PREPARE_E |
                 PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_REGISTER_E |
                 PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_WRITE_MASK_E),
                regAddr/*addr*/, 1/*length in words*/,
                mask/*param0*/, 0/*param1*/, &value/*inoutDataPtr*/, &skipHwAccess);
            if (rc != GT_OK)
            {
                CPSS_LOG_API_ERROR_EXIT_AND_RETURN_MAC(funcId, rc);
            }
        }
        if (skipHwAccess == GT_FALSE)
        {
            drv = prvCpssDrvPpConfig[devNum]->drivers[portGroupId];
            if (drv == NULL)
            {
                rc = prvCpssDrvPpObjConfig[devNum]->busPtr->drvHwPpWriteRegBitMask(
                    devNum, portGroupId, regAddr, mask, value);
            }
            else
            {
                rc = prvCpssDrvHwPpWriteRegBitMaskDrv(devNum, portGroupId, drv,
                    CPSS_HW_DRIVER_AS_SWITCHING_E,
                    GT_FALSE, regAddr, mask, value);
            }
        }
        if (prvCpssDrvPpConfig[devNum]->errata.callbackFuncPtr != NULL)
        {
            rc = prvCpssDrvPpConfig[devNum]->errata.callbackFuncPtr(
                devNum, portGroupId,
                prvCpssDrvPpConfig[devNum]->errata.callbackDataPtr,
                (PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_COMPLETE_E |
                 PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_REGISTER_E |
                 PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_WRITE_MASK_E),
                regAddr/*addr*/, 1/*length in words*/,
                mask/*param0*/, 0/*param1*/, &value/*inoutDataPtr*/, &skipHwAccess);
            if (rc != GT_OK)
            {
                CPSS_LOG_API_ERROR_EXIT_AND_RETURN_MAC(funcId, rc);
            }
        }
        if(rc != GT_OK)
        {
            CPSS_LOG_API_EXIT_MAC(funcId, rc);
            return rc;
        }
    }
    PRV_CPSS_DRV_END_HANDLE_PORT_GROUPS_MAC(devNum, portGroupId)

    /* Check if HW access required. */
    if(prvCpssHwAccessObj.hwAccessRegisterBitMaskWriteFunc != NULL)
    {
       prvCpssHwAccessObj.hwAccessRegisterBitMaskWriteFunc(
                                                  devNum, portGroupId, regAddr,
                                                  mask, value,
                                                  CPSS_DRV_HW_ACCESS_STAGE_POST_E);
    }

    CPSS_LOG_API_EXIT_MAC(funcId, rc);

    return rc;
}

/**
* @internal prvCpssDrvHwPpPortGroupReadRam function
* @endinternal
*
* @brief   Read from PP's RAM.
*         in the specific port group in the device
* @param[in] devNum                   - The PP device number to read from.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*                                      (APPLICABLE DEVICES Lion; Lion2)
* @param[in] addr                     - Address offset to read from.
* @param[in] length                   - Number of Words (4 byte) to read.
*
* @param[out] data                     - An array containing the read data.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note
*       GalTis:
*       None.
*
*/
GT_STATUS prvCpssDrvHwPpPortGroupReadRam
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   addr,
    IN GT_U32   length,
    OUT GT_U32  *data
)
{
    GT_STATUS rc = GT_OK;
    CPSS_HW_DRIVER_STC *drv;
    GT_BOOL skipHwAccess = GT_FALSE;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, prvCpssDrvHwPpPortGroupReadRam);

    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupId, addr, length, data));

    PRV_CPSS_DRV_CHECK_PP_DEV_MAC(devNum);
    PRV_CPSS_DRV_CHECK_ACTIVE_PORT_GROUP_ID_FOR_READ_OPERATION_MAC(
        devNum, portGroupId);
    PRV_CPSS_DRV_CHECK_GET_PP_DRV(devNum,portGroupId,drv);

    if(cpssDeviceRunCheck_onEmulator() && devNum == onEmulator_devNum_iterateRamAction)
    {
        GT_U32  ii;

        if(prvCpssDrvTraceHwWrite[devNum] == GT_TRUE)
        {
            cpssOsPrintf("Read RAM:addr[0x%8.8x] [%d]words \n",addr,length);
        }

        rc = GT_OK;
        /* allow the emulator to do read one by one - to support trace with sleep between transactions */
        for(ii = 0 ; ii < length; ii++)
        {
            rc |= prvCpssDrvHwPpPortGroupReadRegister(devNum,portGroupId,addr+ii*4,&data[ii]);
        }

        return rc;
    }

    if (prvCpssDrvPpConfig[devNum]->errata.addrCheckFuncPtr != NULL)
    {
        /* check address range*/
        rc = prvCpssDrvPpConfig[devNum]->errata.addrCheckFuncPtr(devNum,
                                                                 portGroupId,
                                                                 addr,
                                                                 length);
        if (rc != GT_OK)
        {
            CPSS_LOG_API_EXIT_MAC(funcId, rc);
            return rc;
        }
    }

    /* Check if HW access required. */
    if(prvCpssHwAccessObj.hwAccessRamReadFunc!= NULL)
    {
       rc = prvCpssHwAccessObj.hwAccessRamReadFunc(
                                        devNum, portGroupId,
                                        addr, length, data,
                                        CPSS_DRV_HW_ACCESS_STAGE_PRE_E);
       if(rc == GT_ABORTED)
       {
           CPSS_LOG_API_EXIT_MAC(funcId, GT_OK);
           return GT_OK;
       }

       if(rc!= GT_OK)
       {
           CPSS_LOG_API_ERROR_EXIT_AND_RETURN_MAC(funcId, rc);
       }
    }

    if (prvCpssDrvPpConfig[devNum]->errata.callbackFuncPtr != NULL)
    {
        rc = prvCpssDrvPpConfig[devNum]->errata.callbackFuncPtr(
            devNum, portGroupId,
            prvCpssDrvPpConfig[devNum]->errata.callbackDataPtr,
            (PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_PREPARE_E |
             PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_MEMORY_E |
             PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_READ_E),
            addr/*addr*/, length/*length in words*/,
            0/*param0*/, 0/*param1*/, data/*inoutDataPtr*/, &skipHwAccess);
        if (rc != GT_OK)
        {
            CPSS_LOG_API_ERROR_EXIT_AND_RETURN_MAC(funcId, rc);
        }
    }
    if (skipHwAccess == GT_FALSE)
    {
        if (drv == NULL)
        {
            rc = prvCpssDrvPpObjConfig[devNum]->busPtr->drvHwPpReadRam(
                devNum, portGroupId, addr, length, data);
        }
        else
        {
            rc = drv->read(drv, CPSS_HW_DRIVER_AS_SWITCHING_E, addr, data, length);
            if (rc == GT_OK && prvCpssDrvTraceHwRead[devNum] == GT_TRUE)
            {
                rc = cpssTraceHwAccessRead(devNum, portGroupId,
                        GT_FALSE, CPSS_TRACE_HW_ACCESS_ADDR_SPACE_PP_E,
                        addr, length, data);
            }
        }
    }
    if (prvCpssDrvPpConfig[devNum]->errata.callbackFuncPtr != NULL)
    {
        rc = prvCpssDrvPpConfig[devNum]->errata.callbackFuncPtr(
            devNum, portGroupId,
            prvCpssDrvPpConfig[devNum]->errata.callbackDataPtr,
            (PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_COMPLETE_E |
             PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_MEMORY_E |
             PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_READ_E),
            addr/*addr*/, length/*length in words*/,
            0/*param0*/, 0/*param1*/, data/*inoutDataPtr*/, &skipHwAccess);
        if (rc != GT_OK)
        {
            CPSS_LOG_API_ERROR_EXIT_AND_RETURN_MAC(funcId, rc);
        }
    }

    /* Check if HW access required. */
    if(prvCpssHwAccessObj.hwAccessRamReadFunc!= NULL)
    {
       prvCpssHwAccessObj.hwAccessRamReadFunc(
                                        devNum, portGroupId,
                                        addr, length, data,
                                        CPSS_DRV_HW_ACCESS_STAGE_POST_E);
    }

    CPSS_LOG_API_EXIT_MAC(funcId, rc);

    return rc;
}


/**
* @internal prvCpssDrvHwPpPortGroupWriteRam function
* @endinternal
*
* @brief   Writes to PP's RAM.
*         in the specific port group in the device
* @param[in] devNum                   - The PP device number to write to.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*                                      (APPLICABLE DEVICES Lion; Lion2)
* @param[in] addr                     - Address offset to write to.
* @param[in] length                   - Number of Words (4 byte) to write.
* @param[in] data                     - An array containing the  to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note
*       GalTis:
*       Table - PPWrite
*
*/
GT_STATUS prvCpssDrvHwPpPortGroupWriteRam
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   addr,
    IN GT_U32   length,
    IN GT_U32   *data
)
{
    GT_STATUS rc = GT_OK;
    CPSS_HW_DRIVER_STC *drv;
    GT_BOOL skipHwAccess = GT_FALSE;

    SYSTEM_STATE_CAUSED_SKIP_MAC;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, prvCpssDrvHwPpPortGroupWriteRam);

    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupId, addr, length, data));

    PRV_CPSS_DRV_CHECK_PP_DEV_MAC(devNum);
    PRV_CPSS_DRV_CHECK_PORT_GROUP_ID_MAC(devNum, portGroupId);

    if(cpssDeviceRunCheck_onEmulator() && devNum == onEmulator_devNum_iterateRamAction)
    {
        GT_U32  ii;
        if(prvCpssDrvTraceHwRead[devNum] == GT_TRUE)
        {
            cpssOsPrintf("Write RAM:addr[0x%8.8x] [%d]words \n",addr,length);
        }

        rc = GT_OK;

        /* allow the emulator to do write one by one - to support trace with sleep between transactions */
        for(ii = 0 ; ii < length; ii++)
        {
            rc |= prvCpssDrvHwPpPortGroupWriteRegister(devNum,portGroupId,addr+ii*4,data[ii]);
        }

        return rc;
    }

    if (prvCpssDrvPpConfig[devNum]->errata.addrCheckFuncPtr != NULL)
    {
        /* check address range*/
        rc = prvCpssDrvPpConfig[devNum]->errata.addrCheckFuncPtr(devNum,
                                                                 portGroupId,
                                                                 addr,
                                                                 length);
        if (rc != GT_OK)
        {
            CPSS_LOG_API_EXIT_MAC(funcId, rc);
            return rc;
        }
    }

    /* Check if HW access required. */
    if(prvCpssHwAccessObj.hwAccessRamWriteFunc!= NULL)
    {
       rc = prvCpssHwAccessObj.hwAccessRamWriteFunc(
                                        devNum, portGroupId,
                                        addr, length, data,
                                        CPSS_DRV_HW_ACCESS_STAGE_PRE_E);
       if(rc == GT_ABORTED)
       {
           CPSS_LOG_API_EXIT_MAC(funcId, GT_OK);
           return GT_OK;
       }

       if(rc!= GT_OK)
       {
           CPSS_LOG_API_ERROR_EXIT_AND_RETURN_MAC(funcId, rc);
       }
    }

    PRV_CPSS_DRV_START_HANDLE_PORT_GROUPS_MAC(devNum, portGroupId)
    {
        if (prvCpssDrvPpConfig[devNum]->errata.callbackFuncPtr != NULL)
        {
            rc = prvCpssDrvPpConfig[devNum]->errata.callbackFuncPtr(
                devNum, portGroupId,
                prvCpssDrvPpConfig[devNum]->errata.callbackDataPtr,
                (PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_PREPARE_E |
                 PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_MEMORY_E |
                 PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_WRITE_E),
                addr/*addr*/, length/*length in words*/,
                0/*param0*/, 0/*param1*/, data/*inoutDataPtr*/, &skipHwAccess);
            if (rc != GT_OK)
            {
                CPSS_LOG_API_ERROR_EXIT_AND_RETURN_MAC(funcId, rc);
            }
        }
        if (skipHwAccess == GT_FALSE)
        {
            drv = prvCpssDrvPpConfig[devNum]->drivers[portGroupId];
            if (drv == NULL)
            {
                rc = prvCpssDrvPpObjConfig[devNum]->busPtr->drvHwPpWriteRam(
                    devNum, portGroupId, addr, length, data);
            }
            else
            {
                rc = drv->writeMask(drv, CPSS_HW_DRIVER_AS_SWITCHING_E, addr, data, length, 0xffffffff);
                if (rc == GT_OK && prvCpssDrvTraceHwWrite[devNum] == GT_TRUE)
                {
                    rc = cpssTraceHwAccessWrite(devNum, portGroupId, GT_FALSE, CPSS_TRACE_HW_ACCESS_ADDR_SPACE_PP_E,
                            addr, length, data, 0xFFFFFFFF);
                }
            }
        }
        if (prvCpssDrvPpConfig[devNum]->errata.callbackFuncPtr != NULL)
        {
            rc = prvCpssDrvPpConfig[devNum]->errata.callbackFuncPtr(
                devNum, portGroupId,
                prvCpssDrvPpConfig[devNum]->errata.callbackDataPtr,
                (PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_COMPLETE_E |
                 PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_MEMORY_E |
                 PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_WRITE_E),
                addr/*addr*/, length/*length in words*/,
                0/*param0*/, 0/*param1*/, data/*inoutDataPtr*/, &skipHwAccess);
            if (rc != GT_OK)
            {
                CPSS_LOG_API_ERROR_EXIT_AND_RETURN_MAC(funcId, rc);
            }
        }
        if(rc != GT_OK)
        {
            CPSS_LOG_API_EXIT_MAC(funcId, rc);
            return rc;
        }
    }
    PRV_CPSS_DRV_END_HANDLE_PORT_GROUPS_MAC(devNum, portGroupId)

    /* Check if HW access required. */
    if(prvCpssHwAccessObj.hwAccessRamWriteFunc!= NULL)
    {
       prvCpssHwAccessObj.hwAccessRamWriteFunc(
                                        devNum, portGroupId,
                                        addr, length, data,
                                        CPSS_DRV_HW_ACCESS_STAGE_POST_E);
    }

    CPSS_LOG_API_EXIT_MAC(funcId, rc);

    return rc;
}

/**
* @internal prvCpssDrvHwPpPortGroupWriteRamInReverse function
* @endinternal
*
* @brief   Writes to PP's RAM in reverse.
*         in the specific port group in the device
* @param[in] devNum                   - The PP device number to write to.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*                                      (APPLICABLE DEVICES Lion; Lion2)
* @param[in] addr                     - Address offset to write to.
* @param[in] length                   - Number of Words (4 byte) to write.
* @param[in] data                     - An array containing the  to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvHwPpPortGroupWriteRamInReverse
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   addr,
    IN GT_U32   length,
    IN GT_U32   *data
)
{
    GT_STATUS rc = GT_OK;
    CPSS_HW_DRIVER_STC *drv;
    GT_BOOL skipHwAccess = GT_FALSE;

    SYSTEM_STATE_CAUSED_SKIP_MAC;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, prvCpssDrvHwPpPortGroupWriteRamInReverse);

    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupId, addr, length, data));

    PRV_CPSS_DRV_CHECK_BUS_FUNC_OBJ_MAC(devNum);
    PRV_CPSS_DRV_CHECK_PORT_GROUP_ID_MAC(devNum, portGroupId);

    if (prvCpssDrvPpConfig[devNum]->errata.addrCheckFuncPtr != NULL)
    {
        /* check address range*/
        rc = prvCpssDrvPpConfig[devNum]->errata.addrCheckFuncPtr(devNum,
                                                                 portGroupId,
                                                                 addr,
                                                                 length);
        if (rc != GT_OK)
        {
            CPSS_LOG_API_EXIT_MAC(funcId, rc);
            return rc;
        }
    }

    /* Check if HW access required. */
    if(prvCpssHwAccessObj.hwAccessRamWriteFunc!= NULL)
    {
       rc = prvCpssHwAccessObj.hwAccessRamWriteInReverseFunc(
                                        devNum, portGroupId,
                                        addr, length, data,
                                        CPSS_DRV_HW_ACCESS_STAGE_PRE_E);
       if(rc == GT_ABORTED)
       {
           CPSS_LOG_API_EXIT_MAC(funcId, GT_OK);
           return GT_OK;
       }

       if(rc!= GT_OK)
       {
           CPSS_LOG_API_ERROR_EXIT_AND_RETURN_MAC(funcId, rc);
       }
    }

    PRV_CPSS_DRV_START_HANDLE_PORT_GROUPS_MAC(devNum, portGroupId)
    {
        if (prvCpssDrvPpConfig[devNum]->errata.callbackFuncPtr != NULL)
        {
            rc = prvCpssDrvPpConfig[devNum]->errata.callbackFuncPtr(
                devNum, portGroupId,
                prvCpssDrvPpConfig[devNum]->errata.callbackDataPtr,
                (PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_PREPARE_E |
                 PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_MEMORY_E |
                 PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_WRITE_E),
                addr/*addr*/, length/*length in words*/,
                0/*param0*/, 0/*param1*/, data/*inoutDataPtr*/, &skipHwAccess);
            if (rc != GT_OK)
            {
                CPSS_LOG_API_ERROR_EXIT_AND_RETURN_MAC(funcId, rc);
            }
        }
        if (skipHwAccess == GT_FALSE)
        {
            drv = prvCpssDrvPpConfig[devNum]->drivers[portGroupId];
            if (drv != NULL)
            {
                GT_U32 j;
                for (j = 0; j < length; j++)
                {
                    GT_U32 i = length - j - 1;
                    rc = drv->writeMask(drv, CPSS_HW_DRIVER_AS_SWITCHING_E,
                            addr + i*4, &(data[i]), 1, 0xffffffff);
                    if (rc != GT_OK)
                        break;
                }
                if (rc == GT_OK)
                {
                        cpssTraceHwAccessWrite(devNum, portGroupId, GT_FALSE,
                                             CPSS_TRACE_HW_ACCESS_ADDR_SPACE_PP_E,
                                             addr,
                                             length, /* length to trace the data in words */
                                             data,
                                             0xFFFFFFFF);
                }
            }
            else
            {
                rc = prvCpssDrvPpObjConfig[devNum]->busPtr->drvHwPpWriteRamRev(
                    devNum, portGroupId, addr, length, data);
            }
        }
        if (prvCpssDrvPpConfig[devNum]->errata.callbackFuncPtr != NULL)
        {
            rc = prvCpssDrvPpConfig[devNum]->errata.callbackFuncPtr(
                devNum, portGroupId,
                prvCpssDrvPpConfig[devNum]->errata.callbackDataPtr,
                (PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_COMPLETE_E |
                 PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_MEMORY_E |
                 PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_WRITE_E),
                addr/*addr*/, length/*length in words*/,
                0/*param0*/, 0/*param1*/, data/*inoutDataPtr*/, &skipHwAccess);
            if (rc != GT_OK)
            {
                CPSS_LOG_API_ERROR_EXIT_AND_RETURN_MAC(funcId, rc);
            }
        }
        if(rc != GT_OK)
        {
            CPSS_LOG_API_EXIT_MAC(funcId, rc);
            return rc;
        }
    }
    PRV_CPSS_DRV_END_HANDLE_PORT_GROUPS_MAC(devNum, portGroupId)

    /* Check if HW access required. */
    if(prvCpssHwAccessObj.hwAccessRamWriteFunc!= NULL)
    {
       prvCpssHwAccessObj.hwAccessRamWriteInReverseFunc(
                                        devNum, portGroupId,
                                        addr, length, data,
                                        CPSS_DRV_HW_ACCESS_STAGE_POST_E);
    }

    CPSS_LOG_API_EXIT_MAC(funcId, rc);

    return rc;
}

/**
* @internal prvCpssDrvHwPpPortGroupReadVec function
* @endinternal
*
* @brief   Read from PP's RAM a vector of addresses.
*         in the specific port group in the device
* @param[in] devNum                   - The PP device number to read from.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*                                      (APPLICABLE DEVICES Lion; Lion2)
* @param[in] addrArr[]                - Address array to read from.
* @param[in] arrLen                   - The size of addrArr/dataArr.
*
* @param[out] dataArr[]                - An array containing the read data.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvHwPpPortGroupReadVec
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   addrArr[],
    OUT GT_U32  dataArr[],
    IN GT_U32   arrLen
)
{
    GT_U32      i;
    GT_STATUS   rc = GT_OK;
    CPSS_HW_DRIVER_STC *drv;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, prvCpssDrvHwPpPortGroupReadVec);

    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupId, addrArr, dataArr, arrLen));

    PRV_CPSS_DRV_CHECK_PP_DEV_MAC(devNum);
    PRV_CPSS_DRV_CHECK_ACTIVE_PORT_GROUP_ID_FOR_READ_OPERATION_MAC(
        devNum, portGroupId);
    PRV_CPSS_DRV_CHECK_GET_PP_DRV(devNum, portGroupId, drv);

    if (prvCpssDrvPpConfig[devNum]->errata.addrCheckFuncPtr != NULL)
    {
        /* check address ranges */
        for (i = 0; i < arrLen; i++)
        {
            rc = prvCpssDrvPpConfig[devNum]->errata.addrCheckFuncPtr(devNum,
                                                                     portGroupId,
                                                                     addrArr[i],
                                                                     1);
            if (rc != GT_OK)
            {
                CPSS_LOG_API_EXIT_MAC(funcId, rc);
                return rc;
            }
        }
    }

    /* Check if HW access required. */
    if(prvCpssHwAccessObj.hwAccessVectorReadFunc != NULL)
    {
       rc = prvCpssHwAccessObj.hwAccessVectorReadFunc(
                                                  devNum, portGroupId,
                                                  addrArr, dataArr, arrLen,
                                                  CPSS_DRV_HW_ACCESS_STAGE_PRE_E);
       if(rc == GT_ABORTED)
       {
           CPSS_LOG_API_EXIT_MAC(funcId, GT_OK);
           return GT_OK;
       }

       if(rc!= GT_OK)
       {
           CPSS_LOG_API_ERROR_EXIT_AND_RETURN_MAC(funcId, rc);
       }
    }

    if (drv == NULL)
    {
        rc = prvCpssDrvPpObjConfig[devNum]->busPtr->drvHwPpReadVec(
            devNum, portGroupId, addrArr, dataArr, arrLen);
    }
    else
    {
        for (i = 0; i < arrLen; i++)
        {
            rc = prvCpssDrvHwPpDoReadOpDrv(devNum, portGroupId, drv,
                    CPSS_HW_DRIVER_AS_SWITCHING_E,
                    GT_FALSE, addrArr[i], &dataArr[i]);
            if (GT_OK != rc)
            {
                break;
            }
        }
    }

    /* Check if HW access required. */
    if(prvCpssHwAccessObj.hwAccessVectorReadFunc != NULL)
    {
       prvCpssHwAccessObj.hwAccessVectorReadFunc(
                                                  devNum, portGroupId,
                                                  addrArr, dataArr, arrLen,
                                                  CPSS_DRV_HW_ACCESS_STAGE_POST_E);
    }

    CPSS_LOG_API_EXIT_MAC(funcId, rc);

    return rc;
}

/**
* @internal prvCpssDrvHwPpPortGroupWriteVec function
* @endinternal
*
* @brief   Writes to PP's RAM a vector of addresses.
*         in the specific port group in the device
* @param[in] devNum                   - The PP device number to write to.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*                                      (APPLICABLE DEVICES Lion; Lion2)
* @param[in] addrArr[]                - Address offset to write to.
* @param[in] dataArr[]                - An array containing the data to be written.
* @param[in] arrLen                   - The size of addrArr/dataArr.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvHwPpPortGroupWriteVec
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   addrArr[],
    IN GT_U32   dataArr[],
    IN GT_U32   arrLen
)
{
    GT_U32      i;
    GT_STATUS rc = GT_OK;
    CPSS_HW_DRIVER_STC *drv;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, prvCpssDrvHwPpPortGroupWriteVec);

    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupId, addrArr, dataArr, arrLen));

    PRV_CPSS_DRV_CHECK_PP_DEV_MAC(devNum);
    PRV_CPSS_DRV_CHECK_PORT_GROUP_ID_MAC(devNum, portGroupId);

    if (prvCpssDrvPpConfig[devNum]->errata.addrCheckFuncPtr != NULL)
    {
        /* check address ranges */
        for (i = 0; i < arrLen; i++)
        {
            rc = prvCpssDrvPpConfig[devNum]->errata.addrCheckFuncPtr(devNum,
                                                                     portGroupId,
                                                                     addrArr[i],
                                                                     1);
            if (rc != GT_OK)
            {
                CPSS_LOG_API_EXIT_MAC(funcId, rc);
                return rc;
            }
        }
    }

    /* Check if HW access required. */
    if(prvCpssHwAccessObj.hwAccessVectorWriteFunc != NULL)
    {
       rc = prvCpssHwAccessObj.hwAccessVectorWriteFunc(
                                                  devNum, portGroupId,
                                                  addrArr, dataArr, arrLen,
                                                  CPSS_DRV_HW_ACCESS_STAGE_PRE_E);
       if(rc == GT_ABORTED)
       {
           CPSS_LOG_API_EXIT_MAC(funcId, GT_OK);
           return GT_OK;
       }

       if(rc!= GT_OK)
       {
           CPSS_LOG_API_ERROR_EXIT_AND_RETURN_MAC(funcId, rc);
       }
    }

    PRV_CPSS_DRV_START_HANDLE_PORT_GROUPS_MAC(devNum, portGroupId)
    {
        drv = prvCpssDrvPpConfig[devNum]->drivers[portGroupId];
        if (drv == NULL)
        {
            rc = prvCpssDrvPpObjConfig[devNum]->busPtr->drvHwPpWriteVec(
                devNum, portGroupId, addrArr, dataArr, arrLen);
        }
        else
        {
            for (i = 0; i < arrLen; i++)
            {
                rc = prvCpssDrvHwPpWriteRegBitMaskDrv(devNum, portGroupId,
                        drv, CPSS_HW_DRIVER_AS_SWITCHING_E,
                        GT_FALSE, addrArr[i], 0xffffffff, dataArr[i]);
                if (rc != GT_OK)
                    break;
            }
        }
        if(rc != GT_OK)
        {
            CPSS_LOG_API_EXIT_MAC(funcId, rc);
            return rc;
        }
    }
    PRV_CPSS_DRV_END_HANDLE_PORT_GROUPS_MAC(devNum, portGroupId)

    /* Check if HW access required. */
    if(prvCpssHwAccessObj.hwAccessVectorWriteFunc != NULL)
    {
       rc = prvCpssHwAccessObj.hwAccessVectorWriteFunc(
                                                  devNum, portGroupId,
                                                  addrArr, dataArr, arrLen,
                                                  CPSS_DRV_HW_ACCESS_STAGE_POST_E);
    }

    CPSS_LOG_API_EXIT_MAC(funcId, rc);

    return rc;
}

/*******************************************************************************
* prvCpssDrvHwPpPortGroupIsrRead
*
* DESCRIPTION:
*       Read a register value using special interrupt address completion region.
*       in the specific port group in the device
*
* INPUTS:
*       devNum      - The PP to read from.
*       portGroupId - The port group id. relevant only to 'multi-port-groups'
*                     devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*                          (APPLICABLE DEVICES: Lion; Lion2)
*       regAddr     - The register's address to read from.
*                     Note - regAddr should be < 0x1000000
*
* OUTPUTS:
*       dataPtr - Includes the register value.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on hardware error
*       GT_NOT_INITIALIZED - if the driver was not initialized
*       GT_HW_ERROR - on hardware error
*
* COMMENTS:
*       None.
*
* GalTis:
*       None.
*
*******************************************************************************/
GT_INLINE GT_STATUS prvCpssDrvHwPpPortGroupIsrRead
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   *dataPtr
)
{
    GT_STATUS rc;
    CPSS_HW_DRIVER_STC *drv;

    PRV_CPSS_DRV_CHECK_PP_DEV_MAC(devNum);
    PRV_CPSS_DRV_CHECK_ACTIVE_PORT_GROUP_ID_FOR_READ_OPERATION_MAC(
        devNum, portGroupId);
    PRV_CPSS_DRV_CHECK_GET_PP_DRV(devNum,portGroupId,drv);

    /* Check if HW access required. */
    if(prvCpssHwAccessObj.hwAccessRegisterIsrReadFunc != NULL)
    {
       rc = prvCpssHwAccessObj.hwAccessRegisterIsrReadFunc(
                                                  devNum, portGroupId,
                                                  regAddr, dataPtr,
                                                  CPSS_DRV_HW_ACCESS_STAGE_PRE_E);
       if(rc == GT_ABORTED)
       {
           return GT_OK;
       }

       if(rc!= GT_OK)
       {
           return rc;
       }
    }

    if (drv == NULL)
    {
        rc = prvCpssDrvPpObjConfig[devNum]->busPtr->drvHwPpIsrRead(
            devNum, portGroupId, regAddr, dataPtr);
    }
    else
    {
        rc = prvCpssDrvHwPpDoReadOpDrv(devNum, portGroupId, drv,
                CPSS_HW_DRIVER_AS_SWITCHING_E,
                GT_TRUE, regAddr, dataPtr);
    }

    /* Check if HW access required. */
    if(prvCpssHwAccessObj.hwAccessRegisterIsrReadFunc != NULL)
    {
       prvCpssHwAccessObj.hwAccessRegisterIsrReadFunc(
                                                  devNum, portGroupId,
                                                  regAddr, dataPtr,
                                                  CPSS_DRV_HW_ACCESS_STAGE_POST_E);
    }


    return rc;
}

/**
* @internal prvCpssDrvHwPpPortGroupIsrWrite function
* @endinternal
*
* @brief   Write a register value using special interrupt address completion region
*         in the specific port group in the device
* @param[in] devNum                   - The PP to write to.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*                                      (APPLICABLE DEVICES Lion; Lion2)
* @param[in] regAddr                  - The register's address to write to.
*                                      data        - The value to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note
*       GalTis:
*       None.
*
*/
GT_STATUS prvCpssDrvHwPpPortGroupIsrWrite
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   value
)
{
    GT_STATUS rc = GT_OK;
    CPSS_HW_DRIVER_STC *drv;

    PRV_CPSS_DRV_CHECK_PP_DEV_MAC(devNum);
    PRV_CPSS_DRV_CHECK_PORT_GROUP_ID_MAC(devNum, portGroupId);

    /* Check if HW access required. */
    if(prvCpssHwAccessObj.hwAccessRegisterIsrWriteFunc != NULL)
    {
       rc = prvCpssHwAccessObj.hwAccessRegisterIsrWriteFunc(
                                                  devNum, portGroupId,
                                                  regAddr, value,
                                                  CPSS_DRV_HW_ACCESS_STAGE_PRE_E);
       if(rc == GT_ABORTED)
       {
           return GT_OK;
       }

       if(rc!= GT_OK)
       {
           return rc;
       }
    }

    PRV_CPSS_DRV_START_HANDLE_PORT_GROUPS_MAC(devNum, portGroupId)
    {
        drv = prvCpssDrvPpConfig[devNum]->drivers[portGroupId];
        if (drv == NULL)
        {
            rc = prvCpssDrvPpObjConfig[devNum]->busPtr->drvHwPpIsrWrite(
                devNum, portGroupId, regAddr, value);
        }
        else
        {
            rc = prvCpssDrvHwPpWriteRegBitMaskDrv(devNum, portGroupId, drv,
                    CPSS_HW_DRIVER_AS_SWITCHING_E,
                    GT_TRUE, regAddr, 0xffffffff, value);
        }
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_DRV_END_HANDLE_PORT_GROUPS_MAC(devNum, portGroupId)

    /* Check if HW access required. */
    if(prvCpssHwAccessObj.hwAccessRegisterIsrWriteFunc != NULL)
    {
       prvCpssHwAccessObj.hwAccessRegisterIsrWriteFunc(
                                                  devNum, portGroupId,
                                                  regAddr, value,
                                                  CPSS_DRV_HW_ACCESS_STAGE_POST_E);
    }

    return rc;
}

/**
* @internal prvCpssDrvHwPpPortGroupReadInternalPciReg function
* @endinternal
*
* @brief   This function reads from an internal pci register, it's used by the
*         initialization process and the interrupt service routine.
*         in the specific port group in the device
* @param[in] devNum                   - The Pp's device numbers.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*                                      (APPLICABLE DEVICES Lion; Lion2)
* @param[in] regAddr                  - The register's address to read from.
*
* @param[out] data                     - The read data.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvHwPpPortGroupReadInternalPciReg
(
    IN  GT_U8   devNum,
    IN GT_U32   portGroupId,
    IN  GT_U32  regAddr,
    OUT GT_U32  *data
)
{
    GT_STATUS rc;
    CPSS_HW_DRIVER_STC *drv;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, prvCpssDrvHwPpPortGroupReadInternalPciReg);

    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupId, regAddr, data));

    PRV_CPSS_DRV_CHECK_PP_DEV_MAC(devNum);
    PRV_CPSS_DRV_CHECK_ACTIVE_PORT_GROUP_ID_FOR_READ_OPERATION_MAC(
        devNum, portGroupId);
    PRV_CPSS_DRV_GET_PP_DRV(devNum,portGroupId,drv);

    /* support CPSS-ISR calls with embedded info in the 2 LSBits */
    /* this is part of 'interrupt mask register' that it's value indicate that
        need to use internal_pci/dfx/regular PP register access.
        see use of PRV_CPSS_DRV_PEX_INDICATION_IN_ADDR_MAC(...)
    */
    regAddr = PRV_CPSS_REG_MASK_MAC(regAddr);

    if (drv == NULL)
    {
        rc = prvCpssDrvPpObjConfig[devNum]->busPtr->drvHwPpReadIntPciReg(
            devNum, portGroupId, regAddr, data);
    }
    else
    {
        rc = prvCpssDrvHwPpDoReadOpDrv(devNum, portGroupId, drv,
                CPSS_HW_DRIVER_AS_CNM_E,
                GT_FALSE, regAddr, data);
    }

    CPSS_LOG_API_EXIT_MAC(funcId, rc);

    return rc;
}

/**
* @internal prvCpssDrvHwPpPortGroupWriteInternalPciReg function
* @endinternal
*
* @brief   This function reads from an internal pci register, it's used by the
*         initialization process and the interrupt service routine.
*         in the specific port group in the device
* @param[in] devNum                   - The Pp's device numbers.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*                                      (APPLICABLE DEVICES Lion; Lion2)
* @param[in] regAddr                  - The register's address to read from.
* @param[in] data                     - Data to be written.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvHwPpPortGroupWriteInternalPciReg
(
    IN  GT_U8   devNum,
    IN GT_U32   portGroupId,
    IN  GT_U32  regAddr,
    IN  GT_U32  data
)
{
    GT_STATUS rc = GT_OK;
    CPSS_HW_DRIVER_STC *drv;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, prvCpssDrvHwPpPortGroupWriteInternalPciReg);

    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupId, regAddr, data));

    PRV_CPSS_DRV_CHECK_PP_DEV_MAC(devNum);
    PRV_CPSS_DRV_CHECK_PORT_GROUP_ID_MAC(devNum, portGroupId);

    /* support CPSS-ISR calls with embedded info in the 2 LSBits */
    /* this is part of 'interrupt mask register' that it's value indicate that
        need to use internal_pci/dfx/regular PP register access.
        see use of PRV_CPSS_DRV_PEX_INDICATION_IN_ADDR_MAC(...)
    */
    regAddr = PRV_CPSS_REG_MASK_MAC(regAddr);

    PRV_CPSS_DRV_START_HANDLE_PORT_GROUPS_MAC(devNum, portGroupId)
    {
        drv = prvCpssDrvPpConfig[devNum]->drivers[portGroupId];
        if (drv == NULL)
        {
            rc = prvCpssDrvPpObjConfig[devNum]->busPtr->drvHwPpWriteIntPciReg(
                devNum, portGroupId, regAddr, data);
        }
        else
        {
            rc = prvCpssDrvHwPpWriteRegBitMaskDrv(devNum, portGroupId, drv,
                    CPSS_HW_DRIVER_AS_CNM_E,
                    GT_FALSE, regAddr, 0xffffffff, data);
        }
        if(rc != GT_OK)
        {
            CPSS_LOG_API_EXIT_MAC(funcId, rc);
            return rc;
        }
    }
    PRV_CPSS_DRV_END_HANDLE_PORT_GROUPS_MAC(devNum, portGroupId)

    CPSS_LOG_API_EXIT_MAC(funcId, rc);

    return rc;
}

/**
* @internal prvCpssDrvHwPpPortGroupDebugForcePortGroupId function
* @endinternal
*
* @brief   debug tool --> force port group Id for all operations in the cpssDriver level
*
* @param[in] devNum                   - The PP to read from.
* @param[in] forcedPortGroupId        - The port group Id to force.
*                                      (APPLICABLE DEVICES Lion; Lion2)
*                                      CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*                                      --> remove the forcing.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - non-multi port groups device
*
* @note
*       Galtis:
*       None.
*
*/
GT_STATUS prvCpssDrvHwPpPortGroupDebugForcePortGroupId
(
    IN GT_U8    devNum,
    IN GT_U32   forcedPortGroupId
)
{
    /* must save the value before call PRV_CPSS_DRV_CHECK_PORT_GROUP_ID_MAC(...) */
    GT_U32  copyOfPortGroupId = forcedPortGroupId;

    PRV_CPSS_DRV_CHECK_PP_DEV_MAC(devNum);
    /* check the forcedPortGroupId value */
    PRV_CPSS_DRV_CHECK_PORT_GROUP_ID_MAC(devNum, copyOfPortGroupId);

    if(prvCpssDrvPpConfig[devNum]->portGroupsInfo.isMultiPortGroupDevice == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    /* set the new 'debugForcedPortGroupId' */
    prvCpssDrvPpConfig[devNum]->portGroupsInfo.debugForcedPortGroupId =
        forcedPortGroupId;

    return GT_OK;
}

/**
* @internal prvCpssDrvHwPpMemoryAccessTraceEnableSet function
* @endinternal
*
* @brief   debug tool --> make debug prints for all memory access in the cpssDriver level
*
* @param[in] enable                   - GT_TRUE  -  memory access debug prints
*                                      GT_FALSE - disable memory access debug prints is off
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvCpssDrvHwPpMemoryAccessTraceEnableSet
(
    IN GT_BOOL    enable
)
{
    memoryAccessTraceOn = enable;
    return GT_OK;
}
/**
* @internal prvCpssDrvHwPpMgReadReg function
* @endinternal
*
* @brief   Read a register value from the management unit.
*
* @param[in] devNum                   - The PP to read from.
* @param[in] mgNum                    - The management unit to read from.
* @param[in] regAddr                  - The register's address to read from.
*
* @param[out] data                     - Includes the register value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/

GT_STATUS prvCpssDrvHwPpMgReadReg
(
    IN  GT_U8   devNum,
    IN  GT_U32  mgNum,
    IN  GT_U32  regAddr,
    OUT GT_U32  *data
)
{
    GT_STATUS rc;
    CPSS_HW_DRIVER_STC *drv;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, prvCpssDrvHwPpMgReadReg);

    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mgNum, regAddr, data));

    PRV_CPSS_DRV_CHECK_GET_PP_DRV(devNum,CPSS_NON_MULTI_PORT_GROUP_DEVICE_INDEX_CNS,drv);

    if (drv == NULL)
    {
        rc = prvCpssDrvPpObjConfig[devNum]->busPtr->drvHwPpMgReadReg(devNum, mgNum, regAddr, data);

        CPSS_LOG_API_EXIT_MAC(funcId, rc);
        return rc;
    }
    else
    {
        GT_U32 as;
        switch (mgNum)
        {
            case 0:
                as = CPSS_HW_DRIVER_AS_MG0_E;
                break;
            case 1:
                as = CPSS_HW_DRIVER_AS_MG1_E;
                break;
            case 2:
                as = CPSS_HW_DRIVER_AS_MG2_E;
                break;
            case 3:
                as = CPSS_HW_DRIVER_AS_MG3_E;
                break;
            default:
                as = MG_x_INSTANCE_ID(mgNum);
                break;
        }
        rc = prvCpssDrvHwPpDoReadOpDrv(devNum, 0, drv, as, GT_FALSE,
                regAddr, data);
    }

    CPSS_LOG_API_EXIT_MAC(funcId, rc);

    return rc;
}

/**
* @internal prvCpssDrvHwPpMgWriteReg function
* @endinternal
*
* @brief   Write to the management unit given register.
*
* @param[in] devNum                   - The PP to write to.
* @param[in] mgNum                    - The management unit to write to.
* @param[in] regAddr                  - The register's address to write to.
* @param[in] data                     - The value to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvHwPpMgWriteReg
(
    IN  GT_U8   devNum,
    IN  GT_U32  mgNum,
    IN  GT_U32  regAddr,
    IN  GT_U32  data
)
{
    GT_STATUS rc;
    CPSS_HW_DRIVER_STC *drv;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, prvCpssDrvHwPpMgWriteReg);

    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mgNum, regAddr, data));


    PRV_CPSS_DRV_CHECK_GET_PP_DRV(devNum,CPSS_NON_MULTI_PORT_GROUP_DEVICE_INDEX_CNS,drv);

    if (drv == NULL)
    {
        rc = prvCpssDrvPpObjConfig[devNum]->busPtr->drvHwPpMgWriteReg(devNum, mgNum, regAddr, data);
        CPSS_LOG_API_EXIT_MAC(funcId, rc);
        return rc;
    }
    else
    {
        GT_U32 as;
        switch (mgNum)
        {
            case 0:
                as = CPSS_HW_DRIVER_AS_MG0_E;
                break;
            case 1:
                as = CPSS_HW_DRIVER_AS_MG1_E;
                break;
            case 2:
                as = CPSS_HW_DRIVER_AS_MG2_E;
                break;
            case 3:
                as = CPSS_HW_DRIVER_AS_MG3_E;
                break;
            default:
                as = MG_x_INSTANCE_ID(mgNum);
                break;
        }
        rc = prvCpssDrvHwPpWriteRegBitMaskDrv(devNum, 0, drv, as, GT_FALSE,
                regAddr, 0xffffffff, data);
    }
    CPSS_LOG_API_EXIT_MAC(funcId, rc);

    return rc;
}

/**
* @internal prvCpssDrvHwPpMgGetRegField function
* @endinternal
*
* @brief   Read a selected register field from the Management Unit.
*
* @param[in] devNum                   - The PP device number to read from.
* @param[in] mgNum                    - The management unit number.
* @param[in] regAddr                  - The register's address to read from.
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be read.
*
* @param[out] fieldData                - Data to read from the register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvHwPpMgGetRegField
 (
    IN  GT_U8   devNum,
    IN  GT_U32  mgNum,
    IN  GT_U32  regAddr,
    IN  GT_U32  fieldOffset,
    IN  GT_U32  fieldLength,
    OUT GT_U32  *fieldData
)
{
    GT_STATUS rc;
    CPSS_HW_DRIVER_STC *drv;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, prvCpssDrvHwPpMgGetRegField);

    CPSS_LOG_API_ENTER_MAC((funcId, devNum,  regAddr, fieldOffset, fieldLength, fieldData));

    PRV_CPSS_DRV_CHECK_GET_PP_DRV(devNum,CPSS_NON_MULTI_PORT_GROUP_DEVICE_INDEX_CNS,drv);

    if (drv == NULL)
    {
        rc = prvCpssDrvPpObjConfig[devNum]->busPtr->
            drvHwPpMgGetRegField(
                                                devNum, mgNum, regAddr,
                                                fieldOffset,fieldLength,fieldData);
        CPSS_LOG_API_EXIT_MAC(funcId, rc);
        return rc;
    }
    else
    {
        GT_U32 as;
        switch (mgNum)
        {
            case 0:
                as = CPSS_HW_DRIVER_AS_MG0_E;
                break;
            case 1:
                as = CPSS_HW_DRIVER_AS_MG1_E;
                break;
            case 2:
                as = CPSS_HW_DRIVER_AS_MG2_E;
                break;
            case 3:
                as = CPSS_HW_DRIVER_AS_MG3_E;
                break;
            default:
                as = MG_x_INSTANCE_ID(mgNum);
                break;
        }
        rc = prvCpssDrvHwPpGetRegFieldDrv(devNum, 0, drv, as, GT_FALSE,
                regAddr, fieldOffset, fieldLength, fieldData);
    }
    CPSS_LOG_API_EXIT_MAC(funcId, rc);

    return rc;
}
/**
* @internal prvCpssDrvHwPpMgSetRegField function
* @endinternal
*
* @brief   Write value to selected register field of the Management Unit.
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] mgNum                    - The management unit number.
* @param[in] regAddr                  - The register's address to write to.
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be written to register.
* @param[in] fieldData                - Data to be written into the register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note this function actually read the register modifies the requested field
*       and writes the new value back to the HW.
*
*/
GT_STATUS prvCpssDrvHwPpMgSetRegField
 (
    IN  GT_U8   devNum,
    IN  GT_U32  mgNum,
    IN  GT_U32  regAddr,
    IN  GT_U32  fieldOffset,
    IN  GT_U32  fieldLength,
    IN  GT_U32  fieldData
)
{
    GT_STATUS rc;
    CPSS_HW_DRIVER_STC *drv;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, prvCpssDrvHwPpMgSetRegField);

    CPSS_LOG_API_ENTER_MAC((funcId, devNum, regAddr, fieldOffset, fieldLength, fieldData));

    PRV_CPSS_DRV_CHECK_GET_PP_DRV(devNum,CPSS_NON_MULTI_PORT_GROUP_DEVICE_INDEX_CNS,drv);

    if (drv == NULL)
    {
        if (prvCpssDrvPpObjConfig[devNum]->busPtr->drvHwPpMgSetRegField == NULL)
        {
            /* MG functions not initialized*/
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
        }
        rc = prvCpssDrvPpObjConfig[devNum]->busPtr->drvHwPpMgSetRegField(
                                                        devNum, mgNum, regAddr,
                                                        fieldOffset,fieldLength,fieldData);
        CPSS_LOG_API_EXIT_MAC(funcId, rc);
        return rc;
    }
    else
    {
        GT_U32 as;
        switch (mgNum)
        {
            case 0:
                as = CPSS_HW_DRIVER_AS_MG0_E;
                break;
            case 1:
                as = CPSS_HW_DRIVER_AS_MG1_E;
                break;
            case 2:
                as = CPSS_HW_DRIVER_AS_MG2_E;
                break;
            case 3:
                as = CPSS_HW_DRIVER_AS_MG3_E;
                break;
            default:
                as = MG_x_INSTANCE_ID(mgNum);
                break;
        }
        rc = prvCpssDrvHwPpSetRegFieldDrv(devNum, 0, drv, as, GT_FALSE,
                regAddr, fieldOffset, fieldLength, fieldData);
    }
    CPSS_LOG_API_EXIT_MAC(funcId, rc);

    return rc;
}


/**
* @internal prvCpssDrvHwPpResourceReadRegister function
* @endinternal
*
* @brief   Read a register value from the given PP. - generic (non-driver) API
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2 ; Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - The PP to read from.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
* @param[in] resourceType             - The Resource to read from.
* @param[in] regAddr                  - The register's address to read from.
*
* @param[out] dataPtr                  - (pointer to) memory for the register value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - wrong devNum,resourceType.
*/
GT_STATUS prvCpssDrvHwPpResourceReadRegister
(
    IN GT_U8                          devNum,
    IN GT_U32                         portGroupId,
    IN CPSS_DRV_HW_RESOURCE_TYPE_ENT  resourceType,
    IN GT_U32                         regAddr,
    OUT GT_U32                        *dataPtr
)
{
    GT_STATUS  rc;
    CPSS_HW_DRIVER_STC *drv;

    PRV_CPSS_DRV_CHECK_GET_PP_DRV(devNum,CPSS_NON_MULTI_PORT_GROUP_DEVICE_INDEX_CNS,drv);

    if (drv == NULL)
    {
        switch (resourceType)
        {
            case CPSS_DRV_HW_RESOURCE_SWITCHING_AND_MG0_CORE_E:
                rc = prvCpssDrvHwPpPortGroupReadRegister(devNum,portGroupId,regAddr,dataPtr);
                break;
            case CPSS_DRV_HW_RESOURCE_INTERNAL_PCI_E:
                rc = prvCpssDrvHwPpPortGroupReadInternalPciReg(devNum,portGroupId,regAddr,dataPtr);
                break;
            case CPSS_DRV_HW_RESOURCE_DFX_CORE_E:
                rc = prvCpssDrvHwPpResetAndInitControllerReadReg(devNum,regAddr,dataPtr);
                break;
            case CPSS_DRV_HW_RESOURCE_MG1_CORE_E:
                rc = prvCpssDrvHwPpMgReadReg(devNum,1,regAddr,dataPtr);
                break;
            case CPSS_DRV_HW_RESOURCE_MG2_CORE_E:
                rc = prvCpssDrvHwPpMgReadReg(devNum,2,regAddr,dataPtr);
                break;
            case CPSS_DRV_HW_RESOURCE_MG3_CORE_E:
                rc = prvCpssDrvHwPpMgReadReg(devNum,3,regAddr,dataPtr);
                break;
            default:
                rc = GT_BAD_PARAM;
        }
    }
    else
    {
        rc = prvCpssDrvHwPpDoReadOpDrv(devNum, portGroupId, drv,
                (GT_U32)resourceType, GT_FALSE, regAddr, dataPtr);
    }
    return rc;
}

/**
* @internal prvCpssDrvHwPpResourceWriteRegister function
* @endinternal
*
* @brief   Write to a PP's given register. - generic (non-driver) API
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2 ; Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - The PP to write to.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
* @param[in] resourceType             - The Resource to read from.
* @param[in] regAddr                  - The register's address to write to.
* @param[in] value                    - The  to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDrvHwPpResourceWriteRegister
(
    IN GT_U8                            devNum,
    IN GT_U32                           portGroupId,
    IN CPSS_DRV_HW_RESOURCE_TYPE_ENT    resourceType,
    IN GT_U32                           regAddr,
    IN GT_U32                           value
)
{
    GT_STATUS  rc;
    CPSS_HW_DRIVER_STC *drv;

    PRV_CPSS_DRV_CHECK_GET_PP_DRV(devNum,CPSS_NON_MULTI_PORT_GROUP_DEVICE_INDEX_CNS,drv);

    if (drv == NULL)
    {
        switch (resourceType)
        {
            case CPSS_DRV_HW_RESOURCE_SWITCHING_AND_MG0_CORE_E:
                rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum,portGroupId,regAddr,value);
                break;
            case CPSS_DRV_HW_RESOURCE_INTERNAL_PCI_E:
                rc = prvCpssDrvHwPpPortGroupWriteInternalPciReg(devNum,portGroupId,regAddr,value);
                break;
            case CPSS_DRV_HW_RESOURCE_DFX_CORE_E:
                rc = prvCpssDrvHwPpResetAndInitControllerWriteReg(devNum,regAddr,value);
                break;
            case CPSS_DRV_HW_RESOURCE_MG1_CORE_E:
                rc = prvCpssDrvHwPpMgWriteReg(devNum,1,regAddr,value);
                break;
            case CPSS_DRV_HW_RESOURCE_MG2_CORE_E:
                rc = prvCpssDrvHwPpMgWriteReg(devNum,2,regAddr,value);
                break;
            case CPSS_DRV_HW_RESOURCE_MG3_CORE_E:
                rc = prvCpssDrvHwPpMgWriteReg(devNum,3,regAddr,value);
                break;
            default:
                rc = GT_BAD_PARAM;
        }
    }
    else
    {
        rc = prvCpssDrvHwPpWriteRegBitMaskDrv(devNum, portGroupId, drv,
                (GT_U32)resourceType, GT_FALSE, regAddr, 0xffffffff, value);
    }
    return rc;
}

/**
* @internal prvCpssDrvHwPpResourceGetRegField function
* @endinternal
*
* @brief   Read a selected register field. - generic (non-driver) API
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2 ; Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - The PP device number to read from.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
* @param[in] resourceType             - The Resource to read from.
* @param[in] regAddr                  - The register's address to read from.
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be read.
*
* @param[out] fieldDataPtr             - (pointer to) Data to read from the register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_IMPLEMENTED       - this feature is not implemented
*/
GT_STATUS prvCpssDrvHwPpResourceGetRegField
(
    IN GT_U8                            devNum,
    IN GT_U32                           portGroupId,
    IN CPSS_DRV_HW_RESOURCE_TYPE_ENT    resourceType,
    IN GT_U32                           regAddr,
    IN GT_U32                           fieldOffset,
    IN GT_U32                           fieldLength,
    OUT GT_U32                          *fieldDataPtr
)

{
    GT_STATUS  rc;
    CPSS_HW_DRIVER_STC *drv;

    PRV_CPSS_DRV_CHECK_GET_PP_DRV(devNum,CPSS_NON_MULTI_PORT_GROUP_DEVICE_INDEX_CNS,drv);

    if (drv == NULL)
    {
        switch (resourceType)
        {
            case CPSS_DRV_HW_RESOURCE_SWITCHING_AND_MG0_CORE_E:
                rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,portGroupId,regAddr,fieldOffset,fieldLength,fieldDataPtr);
                break;
            case CPSS_DRV_HW_RESOURCE_INTERNAL_PCI_E:
                rc = GT_NOT_IMPLEMENTED;
                break;
            case CPSS_DRV_HW_RESOURCE_DFX_CORE_E:
                rc = prvCpssDrvHwPpResetAndInitControllerGetRegField(devNum,regAddr,fieldOffset,fieldLength,fieldDataPtr);
                break;
            case CPSS_DRV_HW_RESOURCE_MG1_CORE_E:
                rc = prvCpssDrvHwPpMgGetRegField(devNum,1,regAddr,fieldOffset,fieldLength,fieldDataPtr);
                break;
            case CPSS_DRV_HW_RESOURCE_MG2_CORE_E:
                rc = prvCpssDrvHwPpMgGetRegField(devNum,2,regAddr,fieldOffset,fieldLength,fieldDataPtr);
                break;
            case CPSS_DRV_HW_RESOURCE_MG3_CORE_E:
                rc = prvCpssDrvHwPpMgGetRegField(devNum,3,regAddr,fieldOffset,fieldLength,fieldDataPtr);
                break;
            default:
                rc = GT_BAD_PARAM;
        }
    }
    else
    {
        rc = prvCpssDrvHwPpGetRegFieldDrv(devNum, portGroupId, drv,
                (GT_U32)resourceType, GT_FALSE,
                regAddr, fieldOffset,fieldLength,fieldDataPtr);
    }
    return rc;
}

/**
* @internal prvCpssDrvHwPpResourceSetRegField function
* @endinternal
*
* @brief   Write value to selected register field. - generic (non-driver) API
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2 ; Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
* @param[in] resourceType             - The Resource to read from.
* @param[in] regAddr                  - The register's address to write to.
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be written to register.
* @param[in] fieldData                - Data to be written into the register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDrvHwPpResourceSetRegField
(
    IN GT_U8                            devNum,
    IN GT_U32                           portGroupId,
    IN CPSS_DRV_HW_RESOURCE_TYPE_ENT    resourceType,
    IN GT_U32                           regAddr,
    IN GT_U32                           fieldOffset,
    IN GT_U32                           fieldLength,
    IN GT_U32                           fieldData
)
{
    GT_STATUS  rc;
    CPSS_HW_DRIVER_STC *drv;

    PRV_CPSS_DRV_CHECK_GET_PP_DRV(devNum,CPSS_NON_MULTI_PORT_GROUP_DEVICE_INDEX_CNS,drv);

    if (drv == NULL)
    {
        switch (resourceType)
        {
            case CPSS_DRV_HW_RESOURCE_SWITCHING_AND_MG0_CORE_E:
                rc = prvCpssDrvHwPpPortGroupSetRegField(devNum,portGroupId,regAddr,fieldOffset,fieldLength,fieldData);
                break;
            case CPSS_DRV_HW_RESOURCE_INTERNAL_PCI_E:
                rc = GT_NOT_IMPLEMENTED;
                break;
            case CPSS_DRV_HW_RESOURCE_DFX_CORE_E:
                rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum,regAddr,fieldOffset,fieldLength,fieldData);
                break;
            case CPSS_DRV_HW_RESOURCE_MG1_CORE_E:
                rc = prvCpssDrvHwPpMgSetRegField(devNum,1,regAddr,fieldOffset,fieldLength,fieldData);
                break;
            case CPSS_DRV_HW_RESOURCE_MG2_CORE_E:
                rc = prvCpssDrvHwPpMgSetRegField(devNum,2,regAddr,fieldOffset,fieldLength,fieldData);
                break;
            case CPSS_DRV_HW_RESOURCE_MG3_CORE_E:
                rc = prvCpssDrvHwPpMgSetRegField(devNum,3,regAddr,fieldOffset,fieldLength,fieldData);
                break;
            default:
                rc = GT_BAD_PARAM;
        }
    }
    else
    {
        rc = prvCpssDrvHwPpSetRegFieldDrv(devNum, portGroupId, drv,
                (GT_U32)resourceType, GT_FALSE,
                regAddr, fieldOffset,fieldLength,fieldData);
    }
    return rc;
}
/**
* @internal prvCpssDrvHwPpResourceWriteRegBitMask function
* @endinternal
*
* @brief   write a selected register bitmasked value.
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
* @param[in] resourceType             - The Resource to read from.
* @param[in] regAddr                  - The register's address to read from.
* @param[in] mask                     - Mask for selecting the written bits.
* @param[in] value                    - Data to be written to register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvHwPpResourceWriteRegBitMask
(

    IN GT_U8                            devNum,
    IN GT_U32                           portGroupId,
    IN CPSS_DRV_HW_RESOURCE_TYPE_ENT    resourceType,
    IN GT_U32                           regAddr,
    IN GT_U32                           mask,
    IN GT_U32                           value
)
{
    GT_U32 data;
    GT_STATUS rc;
    CPSS_HW_DRIVER_STC *drv;

    PRV_CPSS_DRV_CHECK_GET_PP_DRV(devNum,CPSS_NON_MULTI_PORT_GROUP_DEVICE_INDEX_CNS,drv);

    if (drv == NULL)
    {
        if(mask == 0xffffffff)
        {
            return prvCpssDrvHwPpResourceWriteRegister(devNum, portGroupId, resourceType, regAddr, value);
        }
        rc = prvCpssDrvHwPpResourceReadRegister(devNum, portGroupId, resourceType, regAddr, &data);
        if (rc != GT_OK)
            return rc;
        data &= ~mask;
        data |= (value & mask);
        rc = prvCpssDrvHwPpResourceWriteRegister(devNum, portGroupId, resourceType, regAddr, data);
    }
    else
    {
        rc = prvCpssDrvHwPpWriteRegBitMaskDrv(devNum, portGroupId, drv,
                (GT_U32)resourceType, GT_FALSE, regAddr, mask, value);
    }
    return rc;
}

/**
* @internal prvCpssDrvHwPpMg1ReadReg function
* @endinternal
*
* @brief   Read a register value from the management unit #1.
*
* @param[in] devNum                   - The PP to read from.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
* @param[in] regAddr                  - The register's address to read from.
*
* @param[out] dataPtr                  - Includes the register value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/

GT_STATUS prvCpssDrvHwPpMg1ReadReg
(
    IN  GT_U8   devNum,
    IN  GT_U32  portGroupId,
    IN  GT_U32  regAddr,
    OUT GT_U32  *dataPtr
)
{
    portGroupId = portGroupId; /*avoid warning*/
    return prvCpssDrvHwPpMgReadReg(devNum, 1, PRV_CPSS_MGS_REG_MASK_MAC(regAddr), dataPtr);
}

/**
* @internal prvCpssDrvHwPpMg1WriteReg function
* @endinternal
*
* @brief   Write to the management unit 1 given register.
*
* @param[in] devNum                   - The PP to write to.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
* @param[in] regAddr                  - The register's address to write to.
* @param[in] data                     - The value to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvHwPpMg1WriteReg
(
    IN  GT_U8   devNum,
    IN  GT_U32  portGroupId,
    IN  GT_U32  regAddr,
    IN  GT_U32  data
)
{

    portGroupId = portGroupId; /*avoid warning*/
    return prvCpssDrvHwPpMgWriteReg(devNum, 1, PRV_CPSS_MGS_REG_MASK_MAC(regAddr), data);
}

/**
* @internal prvCpssDrvHwPpMg2ReadReg function
* @endinternal
*
* @brief   Read a register value from the management unit #2.
*
* @param[in] devNum                   - The PP to read from.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
* @param[in] regAddr                  - The register's address to read from.
*
* @param[out] dataPtr                  - Includes the register value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/

GT_STATUS prvCpssDrvHwPpMg2ReadReg
(
    IN  GT_U8   devNum,
    IN  GT_U32  portGroupId,
    IN  GT_U32  regAddr,
    OUT GT_U32  *dataPtr
)
{
    portGroupId = portGroupId; /*avoid warning*/
    return prvCpssDrvHwPpMgReadReg(devNum, 2, PRV_CPSS_MGS_REG_MASK_MAC(regAddr), dataPtr);
}

/**
* @internal prvCpssDrvHwPpMg2WriteReg function
* @endinternal
*
* @brief   Write to the management unit 2 given register.
*
* @param[in] devNum                   - The PP to write to.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
* @param[in] regAddr                  - The register's address to write to.
* @param[in] data                     - The value to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvHwPpMg2WriteReg
(
    IN  GT_U8   devNum,
    IN  GT_U32  portGroupId,
    IN  GT_U32  regAddr,
    IN  GT_U32  data
)
{

    portGroupId = portGroupId; /*avoid warning*/
    return prvCpssDrvHwPpMgWriteReg(devNum, 2, PRV_CPSS_MGS_REG_MASK_MAC(regAddr), data);
}

/**
* @internal prvCpssDrvHwPpMg3ReadReg function
* @endinternal
*
* @brief   Read a register value from the management unit #3.
*
* @param[in] devNum                   - The PP to read from.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
* @param[in] regAddr                  - The register's address to read from.
*
* @param[out] dataPtr                  - Includes the register value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/

GT_STATUS prvCpssDrvHwPpMg3ReadReg
(
    IN  GT_U8   devNum,
    IN  GT_U32  portGroupId,
    IN  GT_U32  regAddr,
    OUT GT_U32  *dataPtr
)
{
    portGroupId = portGroupId; /*avoid warning*/
    return prvCpssDrvHwPpMgReadReg(devNum, 3, PRV_CPSS_MGS_REG_MASK_MAC(regAddr), dataPtr);
}

/**
* @internal prvCpssDrvHwPpMg3WriteReg function
* @endinternal
*
* @brief   Write to the management unit #3 given register.
*
* @param[in] devNum                   - The PP to write to.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
* @param[in] regAddr                  - The register's address to write to.
* @param[in] data                     - The value to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvHwPpMg3WriteReg
(
    IN  GT_U8   devNum,
    IN  GT_U32  portGroupId,
    IN  GT_U32  regAddr,
    IN  GT_U32  data
)
{

    portGroupId = portGroupId; /*avoid warning*/
    return prvCpssDrvHwPpMgWriteReg(devNum, 3, PRV_CPSS_MGS_REG_MASK_MAC(regAddr), data);
}

