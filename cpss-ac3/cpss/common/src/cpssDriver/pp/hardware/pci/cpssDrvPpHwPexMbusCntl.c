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
* @file cpssDrvPpHwPexMbusCntl.c
*
* @brief Prestera driver Hardware read and write functions implementation.
* Relevant for PEX or MBUS with 8 address comlection regions configured
* in 8 separate registers.
*
* @version   3
********************************************************************************
*/
/*#define HW_DEBUG_TRACE*/

#include <cpssDriver/pp/interrupts/generic/prvCpssDrvInterrupts.h>
#include <cpssDriver/pp/config/prvCpssDrvPpCfgInit.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpssDriver/pp/hardware/prvCpssDrvHwCntl.h>
#include <cpssDriver/pp/hardware/prvCpssDrvObj.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpssDriver/pp/hardware/cpssDriverPpHw.h>

extern CPSS_PP_FAMILY_TYPE_ENT prvCpssDrvHwPpPrePhase1NextDevFamilyGet(GT_VOID);
/* NOT operator for GT_BOOL values */
#define PRV_BOOL_NOT_MAC(_val) (((_val) == GT_FALSE) ? GT_TRUE : GT_FALSE)

/* debug counters , for number of PEX/MBUS read/write access */
static GT_U32 writeCounter[PRV_CPSS_MAX_PP_DEVICES_CNS];
static GT_U32 readCounter[PRV_CPSS_MAX_PP_DEVICES_CNS];
#ifdef HW_DEBUG_TRACE
/* macro that state that 'write' to the bus is about to happen */
#define WRITE_BOOKMARK(devNum) writeCounter[devNum]++;  \
      if(cpssDrvAddrComp_millisecSleep) cpssOsTimerWkAfter(cpssDrvAddrComp_millisecSleep)
/* macro that state that 'read' from the bus is about to happen */
#define READ_BOOKMARK(devNum) readCounter[devNum]++;  \
      if(cpssDrvAddrComp_millisecSleep) cpssOsTimerWkAfter(cpssDrvAddrComp_millisecSleep)
#else
/* macro that state that 'write' to the bus is about to happen */
#define WRITE_BOOKMARK(devNum) writeCounter[devNum]++;
/* macro that state that 'read' from the bus is about to happen */
#define READ_BOOKMARK(devNum) readCounter[devNum]++;
#endif

static GT_U32 prvCpssDrvTraceHwDataMask[PRV_CPSS_MAX_PP_DEVICES_CNS];

/* function is 'ROC' : return value and clear the value for next time */
GT_U32  pexMbusCounterWriteGet(IN GT_32 devNum)
{
    GT_U32  value;
    if(devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        return 0;
    }

    value = writeCounter[devNum];
    writeCounter[devNum] = 0;

    return value;
}
/* function is 'ROC' : return value and clear the value for next time */
GT_U32  pexMbusCounterReadGet(IN GT_32 devNum)
{
    GT_U32  value;
    if(devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        return 0;
    }

    value = readCounter[devNum];
    readCounter[devNum] = 0;

    return value;
}

/* dummy counter , needed to allow to put break point during debug */
static GT_U32   counter_debugAccessToNotValidAddr = 0;
/* debug function to allow to put break point in order to catch code that doing
   'access address that is not valid' */
static void debugAccessToNotValidAddr(void)
{
    /* allow to put break point here in order to catch code that doing
       'access address that is not valid' */
    counter_debugAccessToNotValidAddr++;
}

#define CHECK_VALID_ADDR_CNS(addr)  \
    if((addr)&0x3)                  \
    {                               \
        /* try to access address that is not valid . probably since the DB of  */   \
        /*   registers/table was not properly initialized or this register in DB */ \
        /*   is not supported for this device */                                    \
        debugAccessToNotValidAddr();                                                \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED,"Try to access not valid Addr [0x%8.8x]", \
            (addr));                \
    }


#ifdef ASIC_SIMULATION
#include <asicSimulation/SCIB/scib.h>
/* Macro to convert device ID to simulation device ID */
#define SIM_DEV_ID(dev_num,_portGroupId)     \
    (prvCpssDrvPpConfig[(dev_num)]->hwCtrl[_portGroupId].simInfo.devId)

#define SIM_ADDR_COMPL_EN_GET(dev_num,_portGroupId)  \
    (prvCpssDrvPpConfig[(dev_num)]->hwCtrl[_portGroupId].simInfo.addrCompletEnable)

/* the WM simulation does not support address completion */
/* the GM simulation supports address completion */
#define ADDRESS_COMLETION_ENABLE_MAC(dev_num,_portGroupId)\
    SIM_ADDR_COMPL_EN_GET(dev_num,_portGroupId)
#define ADDRESS_COMLETION_DISABLE_SUPPORTED_CNS 1
#else /*ASIC_SIMULATION*/
/* the HW supports address completion */
#define ADDRESS_COMLETION_DISABLE_SUPPORTED_CNS 0
#endif /*ASIC_SIMULATION*/

/* bits of address passes as is throw PCI window */
#define NOT_ADDRESS_COMPLETION_BITS_NUM_CNS 19
/* bits of address extracted from address completion registers */
#define ADDRESS_COMPLETION_BITS_MASK_CNS    (0xFFFFFFFF << NOT_ADDRESS_COMPLETION_BITS_NUM_CNS)

extern GT_BOOL  prvCpssDrvTraceHwWrite[PRV_CPSS_MAX_PP_DEVICES_CNS];
extern GT_BOOL  prvCpssDrvTraceHwRead[PRV_CPSS_MAX_PP_DEVICES_CNS];
extern GT_BOOL  prvCpssDrvTraceHwWriteBeforeAccess;

/* Object that holds callback function to HW access */
extern CPSS_DRV_HW_ACCESS_OBJ_STC prvCpssHwAccessObj;

#define debugPrint(_x)\
    if(memoryAccessTraceOn == GT_TRUE)\
       cpssOsPrintf _x

#ifdef HW_DEBUG_TRACE
#define debugTrace(_x)      cpssOsPrintSync _x
#else
#define debugTrace(_x)
#endif  /* HW_DEBUG_TRACE */


#define DEBUG_WRITE_ADDR(memAddr,data)  \
    debugTrace(("PEX W[0x%8.8x][0x%8.8x]\n",memAddr,data))

#define DEBUG_READ_ADDR(memAddr)  \
    debugTrace(("PEX R[0x%8.8x]\n",memAddr))


/*******************************************************************************
* Intenal Macros and definitions
*******************************************************************************/
#define HWCTRL(_devNum,_portGroupId) \
    (prvCpssDrvPpConfig[_devNum]->hwCtrl[_portGroupId])
#define HWINFO(_devNum,_portGroupId) \
    (prvCpssDrvPpConfig[_devNum]->hwInfo[_portGroupId])
#define BASEADDR(_devNum,_portGroupId) (HWINFO(_devNum,_portGroupId).resource.switching.start)


#if (ADDRESS_COMLETION_DISABLE_SUPPORTED_CNS == 0)
#define CPU_MEMORY_BASE_ADDR_MAC(deviceId, portGroupId) \
    BASEADDR(deviceId,portGroupId)
#else
#define CPU_MEMORY_BASE_ADDR_MAC(deviceId, portGroupId)                \
    ((ADDRESS_COMLETION_ENABLE_MAC(deviceId, portGroupId) == GT_FALSE) \
    ? 0 : BASEADDR(deviceId,portGroupId))
#endif


#ifdef USE_PUMA_LPM_DWORD
#ifndef ASIC_SIMULATION
#define HW_SUPPORT_PUMA_LPM_DWORD
/*  Address range of PUMA's LPM SRAM - used in hwWrite */
#define PUMA_LPM_SRAM_START     0x50000000
#define PUMA_LPM_SRAM_END       0x90000000
#endif /*no ASIC_SIMULATION*/
#endif /*USE_PUMA_LPM_DWORD*/

static GT_U32   cpssDrvAddrComp_millisecSleep = 0;
void cpssDrvAddrComp_millisecSleepSet(IN GT_U32 timeout)
{
    cpssDrvAddrComp_millisecSleep = timeout;
}


/**
* @internal cpuReadMemoryWord function
* @endinternal
*
* @brief   Read memory from device.
*
* @param[in] deviceId                 - ID of device, which is equal to PSS Core API device ID.
* @param[in] portGroupId              - port Group Id
* @param[in] memAddr                  - address of first word to read.
*
* @param[out] dataPtr                  - pointer to copy read data.
*/
static void cpuReadMemoryWord
(
    IN  GT_U32        deviceId,
    IN  GT_U32        portGroupId,
    IN  GT_U32        memAddr,
    OUT GT_U32        *dataPtr
)
{
    GT_U32     temp;    /* temp word */
    GT_UINTPTR address; /* address adjusted to window */

    READ_BOOKMARK(deviceId);

    address = CPU_MEMORY_BASE_ADDR_MAC(deviceId, portGroupId) + memAddr;

    DEBUG_READ_ADDR(address);
#ifdef ASIC_SIMULATION
    scibReadMemory(SIM_DEV_ID(deviceId, portGroupId), address, 1/*length*/, &temp);
#else /*ASIC_SIMULATION*/
    temp = *((volatile GT_U32 *)address);
    GT_SYNC; /* to avoid read combining */
#endif /*ASIC_SIMULATION*/
    *dataPtr = CPSS_32BIT_LE(temp);
}

/**
* @internal cpuWriteMemoryWord function
* @endinternal
*
* @brief   Write memory from device.
*
* @param[in] deviceId                 - ID of device, which is equal to PSS Core API device ID.
* @param[in] portGroupId              - port Group Id
* @param[in] memAddr                  - address of first word to read.
* @param[in] data                     - write data.
*
* @param[out] data                     - pointer to copy read data.
*/
static void cpuWriteMemoryWord
(
    IN  GT_U32        deviceId,
    IN  GT_U32        portGroupId,
    IN  GT_U32        memAddr,
    OUT GT_U32        data
)
{
    GT_U32     leData;  /*little endian data*/
    GT_UINTPTR address; /* address adjusted to window */

    WRITE_BOOKMARK(deviceId);

    address = CPU_MEMORY_BASE_ADDR_MAC(deviceId, portGroupId) + memAddr;
    leData = CPSS_32BIT_LE(data);

    DEBUG_WRITE_ADDR(address,leData);

#ifdef ASIC_SIMULATION
    scibWriteMemory(SIM_DEV_ID(deviceId, portGroupId), address, 1/*length*/, &leData);
#else /*ASIC_SIMULATION*/
    *((volatile GT_U32*)address) = leData;
    /* need to put GT_SYNC to avoid from having the CPU doing
    * write combine. Some CPUs like advanced new PPC can do write combining
    * of two writes to continuous addresses. So, to avoid that we use the eioio
    * in PPC architecture */
    GT_SYNC; /* to avoid from write combining */
#endif /*ASIC_SIMULATION*/
}

/**
* @internal cpuReadMemoryArray function
* @endinternal
*
* @brief   Read memory from device.
*
* @param[in] deviceId                 - ID of device, which is equal to PSS Core API device ID.
* @param[in] portGroupId              - port Group Id
* @param[in] memAddr                  - address of first word to read.
* @param[in] length                   - number of words to read.
*
* @param[out] dataPtr                  - pointer to copy read data.
*/
static void cpuReadMemoryArray
(
    IN  GT_U32        deviceId,
    IN  GT_U32        portGroupId,
    IN  GT_U32        memAddr,
    IN  GT_U32        length,
    OUT GT_U32        *dataPtr
)
{
    GT_U32     temp;    /* temp word       */
    GT_U32     i;       /* loop index      */
    GT_UINTPTR address; /* address adjusted to window */
    GT_UINTPTR base;    /* address adjusted to window */

    base = CPU_MEMORY_BASE_ADDR_MAC(deviceId, portGroupId) + memAddr;
    for (i = 0; (i < length); i++)
    {
        READ_BOOKMARK(deviceId);

        address = base + (4 * i);
        DEBUG_READ_ADDR(address);
#ifdef ASIC_SIMULATION
        scibReadMemory(SIM_DEV_ID(deviceId, portGroupId), address, 1/*length*/, &temp);
#else /*ASIC_SIMULATION*/
        temp = *( (volatile GT_U32 *) address);
        GT_SYNC; /* to avoid read combining */
#endif /*ASIC_SIMULATION*/
        dataPtr[i] = CPSS_32BIT_LE(temp);
    }
}

/**
* @internal cpuWriteMemoryArray function
* @endinternal
*
* @brief   Write memory from device.
*
* @param[in] deviceId                 - ID of device, which is equal to PSS Core API device ID.
* @param[in] portGroupId              - port Group Id
* @param[in] memAddr                  - address of first word to read.
* @param[in] length                   - number of words to read.
* @param[in] dataPtr                  - pointer to copy read data.
*/
static void cpuWriteMemoryArray
(
    IN  GT_U32        deviceId,
    IN  GT_U32        portGroupId,
    IN  GT_U32        memAddr,
    IN  GT_U32        length,
    OUT GT_U32        *dataPtr
)
{
    GT_U32     leData;  /* little endian data */
    GT_U32     i;       /* loop index         */
    GT_UINTPTR address; /* address adjusted to window */
    GT_UINTPTR base;    /* address adjusted to window */

    base = CPU_MEMORY_BASE_ADDR_MAC(deviceId, portGroupId) + memAddr;
    for (i = 0; (i < length); i++)
    {
        WRITE_BOOKMARK(deviceId);

        address = base + (4 * i);
        leData = CPSS_32BIT_LE(dataPtr[i]);

        DEBUG_WRITE_ADDR(address,leData);

#ifdef ASIC_SIMULATION
        scibWriteMemory(SIM_DEV_ID(deviceId, portGroupId), address, 1/*length*/, &leData);
#else /*ASIC_SIMULATION*/
        *((volatile GT_U32*)address) = leData;
        /* need to put GT_SYNC to avoid from having the CPU doing
        * write combine. Some CPUs like advanced new PPC can do write combining
        * of two writes to continuous addresses. So, to avoid that we use the eioio
        * in PPC architecture */
        GT_SYNC; /* to avoid from write combining */
#endif /*ASIC_SIMULATION*/
    }
}

/**
* @internal cpuWriteMemoryArrayInReverse function
* @endinternal
*
* @brief   Write memory from device in reverce order.
*
* @param[in] deviceId                 - ID of device, which is equal to PSS Core API device ID.
* @param[in] portGroupId              - port Group Id
* @param[in] memAddr                  - address of first word to read.
* @param[in] length                   - number of words to read.
* @param[in] dataPtr                  - pointer to copy read data.
*/
static void cpuWriteMemoryArrayInReverse
(
    IN  GT_U32        deviceId,
    IN  GT_U32        portGroupId,
    IN  GT_U32        memAddr,
    IN  GT_U32        length,
    OUT GT_U32        *dataPtr
)
{
    GT_U32     leData;  /* little endian data */
    GT_U32     i,j;      /* loop index         */
    GT_UINTPTR address; /* address adjusted to window */
    GT_UINTPTR base;    /* address adjusted to window */

    base = CPU_MEMORY_BASE_ADDR_MAC(deviceId, portGroupId) + memAddr;
    for (j = 0; (j < length); j++)
    {
        WRITE_BOOKMARK(deviceId);

        i = length - 1 - j;
        address = base + (4 * i);
        leData = CPSS_32BIT_LE(dataPtr[i]);

        DEBUG_WRITE_ADDR(address,leData);

#ifdef ASIC_SIMULATION
        scibWriteMemory(SIM_DEV_ID(deviceId, portGroupId), address, 1/*length*/, &leData);
#else /*ASIC_SIMULATION*/
        *((volatile GT_U32*)address) = leData;
        /* need to put GT_SYNC to avoid from having the CPU doing
        * write combine. Some CPUs like advanced new PPC can do write combining
        * of two writes to continuous addresses. So, to avoid that we use the eioio
        * in PPC architecture */
        GT_SYNC; /* to avoid from write combining */
#endif /*ASIC_SIMULATION*/
    }
}

/**
* @internal setAddrCompletion function
* @endinternal
*
* @brief   This function sets the address completion registers value (in shadow and
*         Hw), for a read / write operation.
* @param[in] devNum                   - The device number to set the address completion value for.
* @param[in] portGroupId              - The port group Id.
* @param[in] regAddr                  - The address to read / write from .
* @param[in] appAccess                - GT_TRUE - regular CPSS API, GT_FALSE - ISR engine
*
* @param[out] compIdxPtr               - The address completion index that was choosen.
* @param[out] pciAddr                  - The address to be accessed on the Pci (for read / write).
*                                      (without PCI window base)
* @param[out] signalSem                - Should the semaphore be signaled after the read / write is
*                                      performed.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note Source address bits are [31:19] - completion bits, [18:0] passed bits.
*       Comlpetion bits must be written to one of region address completion registers
*       Access address is Window_base + bits [21:19] - region number, [18:0] passed bits.
*
*/
static GT_STATUS setAddrCompletion
(
    IN  GT_U8       devNum,
    IN  GT_U32      portGroupId,
    IN  GT_U32      regAddr,
    IN  GT_BOOL     appAccess,
    OUT GT_U8       *compIdxPtr,
    OUT GT_U32      *pciAddr,
    OUT GT_BOOL     *signalSem
)
{
    GT_U32  addrRegion;  /* 13 bit MSB value of PP internal address */
    GT_U32  address;     /* address of completion register in region 0 */
    GT_U32  compIdx;     /* address completion register field index 0-3*/
    GT_U32  i;           /* count iterator for the completion index compare loop*/
    GT_U32  regBmp;      /* the bitmap of regions  */
    GT_U32  regMin;      /* the minimal of regions */
    GT_U32  regMax;      /* the maximal of regions */
    GT_U32  *regUsedPtr; /* pointer to last used region   */
    PRV_CPSS_DRV_STRUCT_PP_PORT_GROUP_HW_STC *hwCtrlPtr; /* pointer to dev+portGroup Control */

    hwCtrlPtr = &(HWCTRL(devNum,portGroupId));

    if ((regAddr & ADDRESS_COMPLETION_BITS_MASK_CNS) == 0)
    {
        /* address comletion bits are 0, region0 used */
        *compIdxPtr = 0;
        *pciAddr    = regAddr;
        *signalSem  = GT_FALSE;
        return GT_OK;
    }

    if (appAccess == GT_FALSE)
    {
        /*ISR*/
        *signalSem  = GT_FALSE;
        regBmp      = hwCtrlPtr->isrAddrCompletionRegionsBmp;
        regUsedPtr  = &(hwCtrlPtr->isrAddrRegionUsed);
        regMin      = hwCtrlPtr->isrAddrRegionMin;
        regMax      = hwCtrlPtr->isrAddrRegionMax;
    }
    else
    {
        /*CPSS API*/
        *signalSem  = GT_TRUE;
        regBmp      = hwCtrlPtr->appAddrCompletionRegionsBmp;
        regUsedPtr  = &(hwCtrlPtr->appAddrRegionUsed);
        regMin      = hwCtrlPtr->appAddrRegionMin;
        regMax      = hwCtrlPtr->appAddrRegionMax;
    }

    if (*signalSem == GT_TRUE)
    {
        cpssOsMutexLock(hwCtrlPtr->hwComplSem);
    }

    addrRegion = (regAddr >> NOT_ADDRESS_COMPLETION_BITS_NUM_CNS);

    /* Compare addr region to existing Address regions. */
    compIdx = 0xFFFF; /* wrong value as stamp */
    for (i = regMin; (i <= regMax); i++)
    {
        /* bypass not assigned regions */
        if ((regBmp & (1 << i)) == 0)
            continue;

        if (addrRegion == hwCtrlPtr->addrCompletShadow[i])
        {
            /* ready region found */
            compIdx = i;
            break;
        }
    }

    if (compIdx == 0xFFFF)
    {
        /* needed to configure region */
        /* try from next bit */
        compIdx = *regUsedPtr + 1;
        if (compIdx > regMax)
        {
            compIdx = regMin;
        }
        else
        {
            /* look for next bit in bitmap */
            for (i = compIdx; (i <= regMax); i++)
            {
                if ((regBmp & (1 << i)) != 0)
                {
                    compIdx = i;
                    break;
                }
            }
        }
        /* save new used region */
        *regUsedPtr = compIdx;

        /* configure completion region in HW */
        /* adders base added at the lower level cpuWrite/cpuRead */
        address = PEX_MBUS_ADDR_COMP_REG_MAC(compIdx);
        cpuWriteMemoryWord(devNum, portGroupId, address, addrRegion);
        /* save configuration */
        hwCtrlPtr->addrCompletShadow[compIdx] = addrRegion;

    }

    /* adders base added at the lower level cpuWrite/cpuRead */
    *pciAddr =
        ((compIdx << NOT_ADDRESS_COMPLETION_BITS_NUM_CNS)
        | (regAddr & (~ ADDRESS_COMPLETION_BITS_MASK_CNS)));
    *compIdxPtr = compIdx;

    return GT_OK;
}

/**
* @internal hwRead function
* @endinternal
*
* @brief   This function receives address and buffer pointer and reads the the PP
*         RAM into the buffer.
* @param[in] devNum                   - The PP dev Num to read from.
* @param[in] portGroupId              - The port group Id.
* @param[in] regAddr                  - The register's address .
* @param[in] length                   - Nubmber of writes in 4 byte each.
* @param[in] appAccess                - GT_TRUE - regular CPSS API, GT_FALSE - ISR engine
* @param[in] dataPtr                  - Pointer to data array.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
*/
static GT_STATUS hwRead
(
    IN  GT_U8       devNum,
    IN  GT_U32      portGroupId,
    IN  GT_U32      regAddr,
    IN  GT_U32      length,
    IN  GT_BOOL     appAccess,
    OUT GT_U32      *dataPtr
)
{
    GT_STATUS rc;        /* return code                             */
    GT_U8     compIdx;   /* address completion register field index */
    GT_U32    address;   /* physical access address for PCI access  */
    GT_BOOL   signalSem; /* is needed to unlock semaphore           */
    GT_U32    lenInReg;  /* length of the part in the same region   */

    CHECK_VALID_ADDR_CNS(regAddr);

    if(cpssDeviceRunCheck_onEmulator())
    {
        if(GT_FALSE == prvCpssOnEmulatorSupportedAddrCheck(devNum,portGroupId,regAddr))
        {
            /* the memory/register is not in supported memory space */

            /* ignore the read operation , return dummy read value */

            *dataPtr = 0;

            return GT_OK;
        }
    }

#if (ADDRESS_COMLETION_DISABLE_SUPPORTED_CNS != 0)
    if (ADDRESS_COMLETION_ENABLE_MAC(devNum, portGroupId) == GT_FALSE)
    {
        cpuReadMemoryArray(
            devNum, portGroupId, regAddr, length, dataPtr);

        if (prvCpssDrvTraceHwRead[devNum] == GT_TRUE)
        {
            return cpssTraceHwAccessRead(devNum,
                                         portGroupId,
                                         PRV_BOOL_NOT_MAC(appAccess), /* ISR context */
                                         CPSS_TRACE_HW_ACCESS_ADDR_SPACE_PP_E, /* PCI/PEX space */
                                         regAddr,
                                         length, /* length to trace the data in words */
                                         dataPtr);
        }

        return GT_OK;
    }
#endif /*ADDRESS_COMLETION_DISABLE_SUPPORTED_CNS*/

    /* support the region bound crossing address space by splitting to recursive calls */
    if (((regAddr + (4 * (length - 1))) & ADDRESS_COMPLETION_BITS_MASK_CNS)
        != (regAddr & ADDRESS_COMPLETION_BITS_MASK_CNS))
    {
        while (1)
        {
            /* distance to the end of region in words */
            lenInReg =
                (((regAddr & ADDRESS_COMPLETION_BITS_MASK_CNS)
                 + (1 << NOT_ADDRESS_COMPLETION_BITS_NUM_CNS)) - regAddr) / 4;

            if (length <= lenInReg)
            {
                lenInReg = length;
            }
            /* read part in current region */
            rc = hwRead(
                devNum, portGroupId, regAddr, lenInReg, appAccess, dataPtr);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* increment addresses and decrement length */
            regAddr += (4 * lenInReg);
            dataPtr += lenInReg;
            length  -= lenInReg;

            if (length == 0)
                break;
        }

        return GT_OK;
    }

    rc = setAddrCompletion(
        devNum, portGroupId, regAddr, appAccess,
        &compIdx, &address, &signalSem);
    if (rc != GT_OK)
    {
        return rc;
    }

    cpuReadMemoryArray(
        devNum, portGroupId, address, length, dataPtr);

    if (signalSem != GT_FALSE)
    {
        cpssOsMutexUnlock(HWCTRL(devNum,portGroupId).hwComplSem);
    }

    if (prvCpssDrvTraceHwRead[devNum] == GT_TRUE)
    {
        return cpssTraceHwAccessRead(devNum,
                                     portGroupId,
                                     PRV_BOOL_NOT_MAC(appAccess), /* ISR context */
                                     CPSS_TRACE_HW_ACCESS_ADDR_SPACE_PP_E, /* PCI/PEX space */
                                     regAddr,
                                     length, /* length to trace the data in words */
                                     dataPtr);
    }

    return GT_OK;
}

#ifdef HW_SUPPORT_PUMA_LPM_DWORD
/**
* @internal hwWriteCopletedDouble function
* @endinternal
*
* @brief   This function receives address and data pointer and write the data to the PP
*         Write width is 64 bit if it's possible
* @param[in] devNum                   - The PP dev Num to read from.
* @param[in] portGroupId              - The port group Id.
* @param[in] completedAddress         - The register's address with address completion bits.
* @param[in] length                   - Nubmber of writes in 4 byte each.
* @param[in] dataPtr                  - Pointer to data array.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on NULL pointer
*/
static GT_STATUS hwWriteCopletedDouble
(
    IN  GT_U8       devNum,
    IN  GT_U32      portGroupId,
    IN  GT_U32      completedAddress,
    IN  GT_U32      length,
    IN  GT_U32      *dataPtr
)
{
    GT_U32      lastWord;
    GT_U32      index;
    GT_U32      address;
    GT_UINTPTR  pciAddress;

    address = completedAddress;

    /* save pointer to the last word  in case that 'length' will be changed */
    lastWord = dataPtr[length-1];

    /* write 32-bit prefix, if exists  - address should be 64-bit aligned */
    if (address & 0x7)
    {
        cpuWriteMemoryWord(devNum, portGroupId, address, dataPtr[0]);
        address +=4;
        length -=1;
    }

    /* 64 bit writes - data divided to two 32-bit words to avoid compiler issues. Swap should be done since we are writing to PCI/PEX */
    for(index = 0;index < (length & 0xFFFFFFFE);index+=2,address+=8)
    {
        pciAddress = address + BASEADDR(devNum,portGroupId);
        cpssExtDrvPciDoubleWrite(address, CPSS_32BIT_LE(dataPtr[index]), CPSS_32BIT_LE(dataPtr[index+1]));
    }

    /* write 32-bit postfix, if exists  - check if length is odd */
    if (length & 0x1)
    {
        cpuWriteMemoryWord(devNum, portGroupId, address, lastWord);
    }

    return GT_OK;
}
#endif /* HW_SUPPORT_PUMA_LPM_DWORD */

/**
* @internal hwWrite function
* @endinternal
*
* @brief   This function receives address and data pointer and write the data to
*         the PP.
* @param[in] devNum                   - The PP dev Num to read from.
* @param[in] portGroupId              - The port group Id.
* @param[in] regAddr                  - The register's address .
* @param[in] length                   - Nubmber of writes in 4 byte each.
* @param[in] appAccess                - GT_TRUE - regular CPSS API, GT_FALSE - ISR engine
* @param[in] dataPtr                  - Pointer to data array.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
*/
static GT_STATUS hwWrite
(
    IN  GT_U8       devNum,
    IN  GT_U32      portGroupId,
    IN  GT_U32      regAddr,
    IN  GT_U32      length,
    IN  GT_BOOL     appAccess,
    IN  GT_U32      *dataPtr
)
{
    GT_STATUS rc;        /* return code                             */
    GT_U8     compIdx;   /* address completion register field index */
    GT_U32    address;   /* physical access address for PCI access  */
    GT_BOOL   signalSem; /* is needed to unlock semaphore           */
    GT_U32    lenInReg;  /* length of the part in the same region   */
#ifdef HW_SUPPORT_PUMA_LPM_DWORD
    GT_BOOL pumaLpmSram = GT_FALSE; /* for Puma - if destination is LPM SRAM */
#endif

    SYSTEM_STATE_CAUSED_SKIP_MAC;

    CHECK_VALID_ADDR_CNS(regAddr);

    if(cpssDeviceRunCheck_onEmulator())
    {
        if(GT_FALSE == prvCpssOnEmulatorSupportedAddrCheck(devNum,portGroupId,regAddr))
        {
            /* the memory/register is not in supported memory space */

            /* ignore the write operation */
            return GT_OK;
        }
    }

#if (ADDRESS_COMLETION_DISABLE_SUPPORTED_CNS != 0)
    if (ADDRESS_COMLETION_ENABLE_MAC(devNum, portGroupId) == GT_FALSE)
    {
        if (prvCpssDrvTraceHwWrite[devNum] == GT_TRUE &&
            prvCpssDrvTraceHwWriteBeforeAccess == GT_TRUE)
        {
            (void)cpssTraceHwAccessWrite(devNum,
                                         portGroupId,
                                         PRV_BOOL_NOT_MAC(appAccess), /* ISR context */
                                         CPSS_TRACE_HW_ACCESS_ADDR_SPACE_PP_E, /* PCI/PEX space */
                                         regAddr,
                                         length, /* length to trace the data in words */
                                         dataPtr,
                                         prvCpssDrvTraceHwDataMask[devNum]);
        }

        cpuWriteMemoryArray(
            devNum, portGroupId, regAddr, length, dataPtr);

        if (prvCpssDrvTraceHwWrite[devNum] == GT_TRUE &&
            prvCpssDrvTraceHwWriteBeforeAccess == GT_FALSE)
        {
            return cpssTraceHwAccessWrite(devNum,
                                         portGroupId,
                                         PRV_BOOL_NOT_MAC(appAccess), /* ISR context */
                                         CPSS_TRACE_HW_ACCESS_ADDR_SPACE_PP_E, /* PCI/PEX space */
                                         regAddr,
                                         length, /* length to trace the data in words */
                                         dataPtr,
                                         prvCpssDrvTraceHwDataMask[devNum]);
        }

        return GT_OK;
    }
#endif /*ADDRESS_COMLETION_DISABLE_SUPPORTED_CNS*/

    /* support the region bound crossing address space by splitting to recursive calls */
    if (((regAddr + (4 * (length - 1))) & ADDRESS_COMPLETION_BITS_MASK_CNS)
        != (regAddr & ADDRESS_COMPLETION_BITS_MASK_CNS))
    {
        while (1)
        {
            /* distance to the end of region in words */
            lenInReg =
                (((regAddr & ADDRESS_COMPLETION_BITS_MASK_CNS)
                 + (1 << NOT_ADDRESS_COMPLETION_BITS_NUM_CNS)) - regAddr) / 4;

            if (length <= lenInReg)
            {
                lenInReg = length;
            }
            /* write part in current region */
            rc = hwWrite(
                devNum, portGroupId, regAddr, lenInReg, appAccess, dataPtr);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* increment addresses and decrement length */
            regAddr += (4 * lenInReg);
            dataPtr += lenInReg;
            length  -= lenInReg;

            if (length == 0)
                break;
        }

        return GT_OK;
    }

    rc = setAddrCompletion(
        devNum, portGroupId, regAddr, appAccess,
        &compIdx, &address, &signalSem);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (prvCpssDrvTraceHwWrite[devNum] == GT_TRUE &&
        prvCpssDrvTraceHwWriteBeforeAccess == GT_TRUE)
    {
        (void)cpssTraceHwAccessWrite(devNum,
                                     portGroupId,
                                     PRV_BOOL_NOT_MAC(appAccess), /* ISR context */
                                     CPSS_TRACE_HW_ACCESS_ADDR_SPACE_PP_E, /* PCI/PEX space */
                                     regAddr,
                                     length, /* length to trace the data in words */
                                     dataPtr,
                                     prvCpssDrvTraceHwDataMask[devNum]);
    }



#ifdef HW_SUPPORT_PUMA_LPM_DWORD
    /* Check if destination is within LPM SRAM space  of PUMA */
    if (CPSS_IS_EXMXPM_FAMILY_MAC(prvCpssDrvPpConfig[devNum]->devFamily))
    {
        if((regAddr >= PUMA_LPM_SRAM_START) && (regAddr < PUMA_LPM_SRAM_END))
        {
            pumaLpmSram = GT_TRUE;
        }
    }

    if (pumaLpmSram == GT_FALSE)
    {
#endif /*HW_SUPPORT_PUMA_LPM_DWORD*/
        cpuWriteMemoryArray(
            devNum, portGroupId, address, length, dataPtr);
#ifdef HW_SUPPORT_PUMA_LPM_DWORD
    }
    else
    {
        hwWriteCopletedDouble(
            devNum, portGroupId, address, length, dataPtr);
    }
#endif /*HW_SUPPORT_PUMA_LPM_DWORD*/

    if (signalSem != GT_FALSE)
    {
        cpssOsMutexUnlock(HWCTRL(devNum,portGroupId).hwComplSem);
    }

    if (prvCpssDrvTraceHwWrite[devNum] == GT_TRUE &&
            prvCpssDrvTraceHwWriteBeforeAccess == GT_FALSE)
    {
        return cpssTraceHwAccessWrite(devNum,
                                     portGroupId,
                                     PRV_BOOL_NOT_MAC(appAccess), /* ISR context */
                                     CPSS_TRACE_HW_ACCESS_ADDR_SPACE_PP_E, /* PCI/PEX space */
                                     regAddr,
                                     length, /* length to trace the data in words */
                                     dataPtr,
                                     prvCpssDrvTraceHwDataMask[devNum]);
    }

    return GT_OK;
}

/**
* @internal hwWriteInReverse function
* @endinternal
*
* @brief   This function receives address and data pointer and perform reverse
*         write of the data to the PP.
* @param[in] devNum                   - The PP dev Num to read from.
* @param[in] portGroupId              - The port group Id.
* @param[in] regAddr                  - The register's address .
* @param[in] length                   - Nubmber of writes in 4 byte each.
* @param[in] appAccess                - GT_TRUE - regular CPSS API, GT_FALSE - ISR engine
* @param[in] dataPtr                  - Pointer to data array.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
*/
static GT_STATUS hwWriteInReverse
(
    IN  GT_U8       devNum,
    IN  GT_U32      portGroupId,
    IN  GT_U32      regAddr,
    IN  GT_U32      length,
    IN  GT_BOOL     appAccess,
    IN  GT_U32      *dataPtr
)
{
    GT_STATUS rc;            /* return code                             */
    GT_U8     compIdx;       /* address completion register field index */
    GT_U32    address;       /* physical access address for PCI access  */
    GT_BOOL   signalSem;     /* is needed to unlock semaphore           */
    GT_U32    lenInReg;      /* length of the part in the same region   */
    GT_U32    regAddrLast;   /* last register address in region         */
    GT_U32    regAddrFirst;  /* first register address in region        */
    GT_U32    *dataPtrFirst; /* first data word to write in region      */

    SYSTEM_STATE_CAUSED_SKIP_MAC;

    CHECK_VALID_ADDR_CNS(regAddr);

    if(cpssDeviceRunCheck_onEmulator())
    {
        if(GT_FALSE == prvCpssOnEmulatorSupportedAddrCheck(devNum,portGroupId,regAddr))
        {
            /* the memory/register is not in supported memory space */

            /* ignore the write operation */
            return GT_OK;
        }
    }
#if (ADDRESS_COMLETION_DISABLE_SUPPORTED_CNS != 0)
    if (ADDRESS_COMLETION_ENABLE_MAC(devNum, portGroupId) == GT_FALSE)
    {
        cpuWriteMemoryArrayInReverse(
            devNum, portGroupId, regAddr, length, dataPtr);

        if (prvCpssDrvTraceHwWrite[devNum] == GT_TRUE)
        {
            return cpssTraceHwAccessWrite(devNum,
                                         portGroupId,
                                         PRV_BOOL_NOT_MAC(appAccess), /* ISR context */
                                         CPSS_TRACE_HW_ACCESS_ADDR_SPACE_PP_E, /* PCI/PEX space */
                                         regAddr,
                                         length, /* length to trace the data in words */
                                         dataPtr,
                                         0xFFFFFFFF);
        }

        return GT_OK;
    }
#endif /*ADDRESS_COMLETION_DISABLE_SUPPORTED_CNS*/

    /* support the region bound crossing address space by splitting to recursive calls */
    if (((regAddr + (4 * (length - 1))) & ADDRESS_COMPLETION_BITS_MASK_CNS)
        != (regAddr & ADDRESS_COMPLETION_BITS_MASK_CNS))
    {
        while (1)
        {
            regAddrLast  = regAddr + (4 * (length - 1));
            regAddrFirst = (regAddrLast & ADDRESS_COMPLETION_BITS_MASK_CNS);
            if (regAddr > regAddrFirst)
            {
                regAddrFirst = regAddr;
            }

            lenInReg = ((regAddrLast - regAddrFirst) / 4) + 1;

            dataPtrFirst = dataPtr + (length - lenInReg);

            /* write part in current region */
            rc = hwWriteInReverse(
                devNum, portGroupId, regAddrFirst, lenInReg, appAccess, dataPtrFirst);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* decrement length */
            length  -= lenInReg;

            if (length == 0)
                break;
        }

        return GT_OK;
    }

    rc = setAddrCompletion(
        devNum, portGroupId, regAddr, appAccess,
        &compIdx, &address, &signalSem);
    if (rc != GT_OK)
    {
        return rc;
    }

    cpuWriteMemoryArrayInReverse(
        devNum, portGroupId, address, length, dataPtr);

    if (signalSem != GT_FALSE)
    {
        cpssOsMutexUnlock(HWCTRL(devNum,portGroupId).hwComplSem);
    }

    if (prvCpssDrvTraceHwWrite[devNum] == GT_TRUE)
    {
        return cpssTraceHwAccessWrite(devNum,
                                     portGroupId,
                                     PRV_BOOL_NOT_MAC(appAccess), /* ISR context */
                                     CPSS_TRACE_HW_ACCESS_ADDR_SPACE_PP_E, /* PCI/PEX space */
                                     regAddr,
                                     length, /* length to trace the data in words */
                                     dataPtr,
                                     0xFFFFFFFF);
    }

    return GT_OK;
}

/**
* @internal hwPpReadBurst function
* @endinternal
*
* @brief   This function performs a burst read from a given device's Ram address.
*
* @param[in] devNum                   - The PP device number to read from.
* @param[in] portGroupId              - The port group Id.
* @param[in] srcAddr                  - The source address to read from.
* @param[in] length                   - Number of 4 byte words to read.
* @param[in] ramBurstInfo             - Holds data regarding the burst to be performed on the
*                                      Ram.
*
* @param[out] data                     - Pointer to array holding the read data.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
*/
static GT_STATUS hwPpReadBurst
(
    IN  GT_U8                             devNum,
    IN GT_U32                             portGroupId,
    IN  GT_U32                            srcAddr,
    IN  GT_U32                            length,
    IN  PRV_CPSS_DRV_RAM_BURST_INFO_STC   *ramBurstInfo,
    OUT GT_U32                            *data
)
{
    GT_U8       addrCompIdx;    /* Index of the address completion byte to  */
                                /* be used for this pci access.             */
    GT_U32      complAddr;      /* The address to be used for pci access,   */
                                /* after setting the address completion     */
                                /* register.                                */
    GT_BOOL     signalSem;      /* Should the semaphore be signaled after   */
                                /* the burst operation.                     */
    GT_U32      tempBurstBuff[PRV_CPSS_DRV_MAX_BURST_BLOCK_SIZE_CNS];/* A temporary buffer   */
    GT_U32      *tempData;      /* used to hold the data to be written in   */
                                /* case a read-modify write operation is    */
                                /* needed.                                  */
    GT_U32      words2Allign;   /* Number of additional words need to be    */
                                /* read to reach the needed alignment for  */
                                /* this Ram.                                */
    GT_U32      allignSrcAddr;  /* The aligned source address.             */
    GT_U32      allignSize;     /* The actual size to be read as a result   */
                                /* of the alignment.                       */
    GT_U32      *tempDstAddr;   /* Used when multiple burst are needed to   */
    GT_U32      tempSrcAddr;    /* perform the read operation, i.e addrRes- */
                                /* -olution != 1.                           */
    GT_U32      numOfBursts;    /* Number of bursts to be performed, used   */
                                /* when addrResolution != 1.                */
    GT_U32      sizeOfBurst;    /* Size of each burst.                      */
    GT_U32      i;
    GT_STATUS   retVal;
#ifdef ASIC_SIMULATION
    GT_UINTPTR  base;

    base = CPU_MEMORY_BASE_ADDR_MAC(devNum, portGroupId);
#endif

    /* Set the destination address to an aligned address   */
    words2Allign    = (srcAddr / sizeof(GT_U32)) % ramBurstInfo->burstSize;
    allignSrcAddr   = srcAddr - (words2Allign * sizeof(GT_U32));
    allignSize      = length + words2Allign;

    if((allignSize % ramBurstInfo->burstSize) != 0)
    {
        allignSize += (ramBurstInfo->burstSize -
                       (allignSize % ramBurstInfo->burstSize));
    }

    /* Set the source address according to the          */
    /* required address resolution.                     */
    allignSrcAddr   = (((allignSrcAddr & (~ramBurstInfo->ramBaseMask)) *
                        ramBurstInfo->addrResolution) +
                       (allignSrcAddr & ramBurstInfo->ramBaseMask));

    /* Set the address completion register. */
    retVal = setAddrCompletion(
        devNum,portGroupId,allignSrcAddr, GT_TRUE /*appAccess*/,
        &addrCompIdx, &complAddr, &signalSem);
    if(retVal != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

    if(ramBurstInfo->addrResolution != 1)
    {
        sizeOfBurst = ramBurstInfo->burstSize;
    }
    else
    {
        sizeOfBurst = allignSize;
    }
    numOfBursts = allignSize / sizeOfBurst;

    if(allignSize != length)
    {
        tempData = tempBurstBuff;
    }
    else
    {
        tempData        = data;
    }

#ifdef ASIC_SIMULATION
    for(i = 0; i < numOfBursts; i++)
    {
        tempDstAddr = (GT_U32*)(((GT_UINTPTR)tempData) + (i * sizeOfBurst *
                                                      sizeof(GT_U32)));
        tempSrcAddr = (allignSrcAddr +
                       (i * sizeOfBurst * ramBurstInfo->addrResolution *
                        sizeof(GT_U32)));

        scibReadMemory(
            SIM_DEV_ID(devNum,portGroupId),
            (base + tempSrcAddr), sizeOfBurst, tempDstAddr);
    }
#else /*ASIC_SIMULATION*/
    /* Update allignSrcAddr with the address completion part.   */
    allignSrcAddr = complAddr +
        BASEADDR(devNum,portGroupId);

    for(i = 0; i < numOfBursts; i++)
    {
        tempDstAddr = (GT_U32*)(((GT_UINTPTR)tempData) + (i * sizeOfBurst *
                                                      sizeof(GT_U32)));
        tempSrcAddr = (allignSrcAddr +
                       (i * sizeOfBurst * ramBurstInfo->addrResolution *
                        sizeof(GT_U32)));

        retVal = cpssExtDrvDmaRead(tempSrcAddr,sizeOfBurst,ramBurstInfo->burstSize,
                               tempDstAddr);
        if(retVal != GT_OK)
        {
            if(signalSem == GT_TRUE)
            {
                cpssOsMutexUnlock(HWCTRL(devNum,portGroupId).hwComplSem);
            }
            return retVal;
        }
    }
#endif /*ASIC_SIMULATION*/

    /* Copy the appropriate part of 'tempData' into  'data',    */
    /* according to words2Allign Value.                         */
    for(i = 0; i < length; i++)
    {
        data[i] = CPSS_32BIT_LE(tempData[i + words2Allign]);
    }
    if(memoryAccessTraceOn == GT_TRUE)
    {
        for(i = 0; i < allignSize; i++)
        {
            debugPrint(("data[%d] - 0x%x.\n",i,data[i]));
        }
    }

    /* Free the address completion semaphore (if needed).       */
    if(signalSem == GT_TRUE)
    {
        cpssOsMutexUnlock(HWCTRL(devNum,portGroupId).hwComplSem);
    }

    /* Check if trace hw read is enabled */
    if (prvCpssDrvTraceHwRead[devNum] == GT_TRUE)
    {
        /* trace HW read access */
        retVal = cpssTraceHwAccessRead(devNum,
                                       portGroupId,
                                       GT_FALSE, /* ISR context */
                                       CPSS_TRACE_HW_ACCESS_ADDR_SPACE_PP_E, /* PCI/PEX space */
                                       allignSrcAddr,
                                       allignSize, /* length to trace the data in words */
                                       tempData);
        if (retVal != GT_OK)
        {
            return retVal;
        }
    }

    return GT_OK;
}

/**
* @internal hwPpWriteBurst function
* @endinternal
*
* @brief   This function performs a burst write to a given device's Ram address.
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] portGroupId              - The port group Id.
* @param[in] destAddr                 - The destination address to write to.
* @param[in] length                   - Number of 4 byte words to write.
* @param[in] data                     - Pointer to  array to be written.
* @param[in] ramBurstInfo             - Holds data regarding the burst to be performed on the
*                                      Ram.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
*
* @note If the given destination address is not a multiple of burstSize, then
*       a read - modify write is performed on the Ram.
*
*/
static GT_STATUS hwPpWriteBurst
(
    IN  GT_U8                             devNum,
    IN GT_U32                             portGroupId,
    IN  GT_U32                            destAddr,
    IN  GT_U32                            length,
    IN  GT_U32                            *data,
    IN  PRV_CPSS_DRV_RAM_BURST_INFO_STC   *ramBurstInfo
)
{
    GT_U8       addrCompIdx;    /* Index of the address completion byte to  */
                                /* be used for this pci access.             */
    GT_U32      complAddr;      /* The address to be used for pci access,   */
                                /* after setting the address completion     */
                                /* register.                                */
    GT_BOOL     signalSem;      /* Should the semaphore be signaled after   */
                                /* the burst operation.                     */

    GT_U32      tempBurstBuff[PRV_CPSS_DRV_MAX_BURST_BLOCK_SIZE_CNS];/* A temporary buffer   */
    GT_U32      *tempData;      /* used to hold the data to be written in   */
                                /* case a read-modify write operation is    */
                                /* needed.                                  */
    GT_U32      words2Allign;   /* Number of additional words need to be    */
                                /* read to reach the needed alignment for  */
                                /* this Ram.                                */
    GT_U32      allignDestAddr; /* The aligned destination address.        */
    GT_U32      allignSize;     /* The actual size to be read as a result   */
                                /* of the alignment.                       */
    GT_U32      *tempSrcAddr;   /* Used when multiple burst are needed to   */
    GT_U32      tempDestAddr;   /* perform the write operation, i.e addrRes-*/
                                /* -olution != 1.                           */
    GT_U32      numOfBursts;    /* Number of bursts to be performed, used   */
                                /* when addrResolution != 1.                */
    GT_U32      sizeOfBurst;    /* Size of each burst.                      */
    GT_U32      i, t;
    GT_STATUS   retVal = GT_OK;
    GT_UINTPTR  base;

    CHECK_VALID_ADDR_CNS(destAddr);

    if(cpssDeviceRunCheck_onEmulator())
    {
        if(GT_FALSE == prvCpssOnEmulatorSupportedAddrCheck(devNum,portGroupId,destAddr))
        {
            /* the memory/register is not in supported memory space */

            /* ignore the write operation */
            return GT_OK;
        }
    }


    base = CPU_MEMORY_BASE_ADDR_MAC(devNum, portGroupId);

    /* If the buffer to be written is larger than PRV_CPSS_DRV_MAX_BURST_BLOCK_SIZE_CNS, */
    /* split this write to multiple writes.                             */
    /* Take any possible needed alignments into account.               */
    if(length > (PRV_CPSS_DRV_MAX_BURST_BLOCK_SIZE_CNS - PRV_CPSS_DRV_MAX_BURST_EXTRA_ALLIGN_CNS))
    {
        sizeOfBurst = PRV_CPSS_DRV_MAX_BURST_BLOCK_SIZE_CNS - PRV_CPSS_DRV_MAX_BURST_EXTRA_ALLIGN_CNS;
        t = 0;
        for(i = 0; i < length / sizeOfBurst; i++)
        {
            retVal = hwPpWriteBurst(devNum,portGroupId,destAddr + (i * sizeOfBurst * 4),sizeOfBurst,
                           data + (i * sizeOfBurst),ramBurstInfo);
            if(retVal != GT_OK)
                return retVal;
            t++;
        }

        /* write remainder */
        if(length%sizeOfBurst != 0)
        {
            retVal = hwPpWriteBurst(devNum,portGroupId,
                                    destAddr + (t * sizeOfBurst * 4),
                                    length%sizeOfBurst,
                                    data + (t * sizeOfBurst),ramBurstInfo);
        }
        return retVal;
    }

    /* Set the destination address to an aligned address   */
    words2Allign    = (destAddr / sizeof(GT_U32)) % ramBurstInfo->burstSize;
    allignDestAddr  = destAddr - (words2Allign * sizeof(GT_U32));
    allignSize      = length + words2Allign;

    /* Check if the end of the block is properly aligned.  */
    if((allignSize % ramBurstInfo->burstSize) != 0)
    {
        allignSize += (ramBurstInfo->burstSize -
                       (allignSize % ramBurstInfo->burstSize));
    }

    /* Set the destination address according to the     */
    /* required address resolution.                     */
    allignDestAddr = (((allignDestAddr & (~ramBurstInfo->ramBaseMask)) *
                       ramBurstInfo->addrResolution) +
                      (allignDestAddr & ramBurstInfo->ramBaseMask));

    /* Set the address completion register. */
    retVal = setAddrCompletion(
        devNum,portGroupId,allignDestAddr,GT_TRUE /*appAccess*/,
        &addrCompIdx, &complAddr, &signalSem);
    if(retVal != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    if(ramBurstInfo->addrResolution != 1)
    {
        sizeOfBurst = ramBurstInfo->burstSize;
    }
    else
    {
        sizeOfBurst = allignSize;
    }
    numOfBursts = allignSize / sizeOfBurst;

    /* Update allignDstAddr with the address completion part.   */
    allignDestAddr = complAddr + base;

    if(allignSize != length)
    {
        tempData = tempBurstBuff;

        for(i = 0; i < numOfBursts; i++)
        {
            tempSrcAddr =
                (GT_U32*)(((GT_UINTPTR)tempData) + (i * sizeOfBurst *
                                                sizeof(GT_U32)));
            tempDestAddr= (allignDestAddr +
                           (i * sizeOfBurst * ramBurstInfo->addrResolution *
                            sizeof(GT_U32)));

#ifdef ASIC_SIMULATION
            scibReadMemory(
                SIM_DEV_ID(devNum,portGroupId),
                tempDestAddr, sizeOfBurst, tempSrcAddr);
            retVal = GT_OK;
#else /*ASIC_SIMULATION*/
            retVal = cpssExtDrvDmaRead(tempDestAddr,sizeOfBurst,
                                   ramBurstInfo->burstSize,tempSrcAddr);
#endif /*ASIC_SIMULATION*/
            if(retVal != GT_OK)
            {
                if(signalSem == GT_TRUE)
                {
                    cpssOsMutexUnlock(HWCTRL(devNum,portGroupId).hwComplSem);
                }
                return retVal;
            }
        }
    }
    else
    {
        tempData        = data;
    }

    /* Copy 'data' into 'tempData', according to words2Allign Value */
    for(i = 0; i < length; i++)
    {
        tempData[i + words2Allign] = CPSS_32BIT_LE(data[i]);
    }
    if(memoryAccessTraceOn == GT_TRUE)
    {
        for(i = 0; i < allignSize; i++)
        {
            debugPrint(("tempData[%d] - 0x%x.\n",i,tempData[i]));
        }
    }
    /* Now write the whole data back into the Ram.              */
    for(i = 0; i < numOfBursts; i++)
    {
        tempSrcAddr =
            (GT_U32*)(((GT_UINTPTR)tempData) + (i * ramBurstInfo->burstSize *
                                            sizeof(GT_U32)));
        tempDestAddr= (allignDestAddr +
                       (i * ramBurstInfo->burstSize *
                        ramBurstInfo->addrResolution * sizeof(GT_U32)));

#ifdef ASIC_SIMULATION
            scibWriteMemory(
                SIM_DEV_ID(devNum,portGroupId),
                tempDestAddr, sizeOfBurst, tempSrcAddr);
            retVal = GT_OK;
#else /*ASIC_SIMULATION*/
            retVal = cpssExtDrvDmaWrite(tempDestAddr,tempSrcAddr,sizeOfBurst,
                                    ramBurstInfo->burstSize);
#endif /*ASIC_SIMULATION*/
        if(retVal != GT_OK)
        {
            if(signalSem == GT_TRUE)
            {
                cpssOsMutexUnlock(HWCTRL(devNum,portGroupId).hwComplSem);
            }
            return retVal;
        }
    }

    /* Restore the value of 'data', if it was byte swapped. */
    if(tempData == data)
    {
        for(i = 0; i < length; i++)
        {
            data[i] = CPSS_32BIT_LE(data[i]);
        }
    }

    /* Free the address completion semaphore (if needed).       */
    if(signalSem == GT_TRUE)
    {
        cpssOsMutexUnlock(HWCTRL(devNum,portGroupId).hwComplSem);
    }

    /* Check if trace hw write is enabled */
    if (prvCpssDrvTraceHwWrite[devNum] == GT_TRUE)
    {
        /* trace HW write access */
        retVal = cpssTraceHwAccessWrite(devNum,
                                        portGroupId,
                                        GT_FALSE, /* ISR context */
                                        CPSS_TRACE_HW_ACCESS_ADDR_SPACE_PP_E, /* PCI/PEX space */
                                        allignDestAddr,
                                        allignSize, /* length to trace the data in words */
                                        data,
                                        0xFFFFFFFF);
        if (retVal != GT_OK)
        {
            return retVal;
        }
    }

    return GT_OK;
}

/**
* @internal hwPpPexMbusReadRegister function
* @endinternal
*
* @brief   Read a register value from the given PP.
*
* @param[in] devNum                   - The PP to read from.
* @param[in] portGroupId              - The port group Id.
* @param[in] regAddr                  - The register's address to read from.
*
* @param[out] data                     - Includes the register value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
*/
static GT_STATUS hwPpPexMbusReadRegister
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    OUT GT_U32  *data
)
{
    debugPrint(("Read from dev %d, reg 0x%x,",devNum, regAddr));
    hwRead (devNum, portGroupId, regAddr, 1, GT_TRUE /*appAccess*/, data);
    debugPrint((" data 0x%x.\n", *data));
    return GT_OK;
}

/**
* @internal hwPpPexMbusWriteRegister function
* @endinternal
*
* @brief   Write to a PP's given register.
*
* @param[in] devNum                   - The PP to write to.
* @param[in] portGroupId              - The port group Id.
* @param[in] regAddr                  - The register's address to write to.
*                                      data    - The value to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
*/
static GT_STATUS hwPpPexMbusWriteRegister
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   value
)
{
    debugPrint(
        ("Write to device %d, reg 0x%x, data 0x%x.\n",devNum,regAddr,
        value));

    hwWrite(devNum, portGroupId, regAddr, 1, GT_TRUE /*appAccess*/, &value);

    return GT_OK;
}

/**
* @internal hwPpPexMbusGetRegField function
* @endinternal
*
* @brief   Read a selected register field.
*
* @param[in] devNum                   - The PP device number to read from.
* @param[in] portGroupId              - The port group Id.
* @param[in] regAddr                  - The register's address to read from.
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be read.
*
* @param[out] fieldData                - Data to read from the register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
*/
static GT_STATUS hwPpPexMbusGetRegField
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   fieldOffset,
    IN GT_U32   fieldLength,
    OUT GT_U32  *fieldData
)
{
    GT_U32 data = 0, mask;

    debugPrint(("GetField from dev %d, reg 0x%x, offset %d, length %d,",
                devNum, regAddr, fieldOffset, fieldLength));

    hwRead(devNum, portGroupId, regAddr, 1, GT_TRUE /*appAccess*/, &data);
    CALC_MASK_MAC(fieldLength, fieldOffset, mask);
    *fieldData = (GT_U32)((data & mask) >> fieldOffset);

    debugPrint(("data 0x%x.\n", *fieldData));

    return GT_OK;
}

/**
* @internal hwPpPexMbusSetRegField function
* @endinternal
*
* @brief   Write value to selected register field.
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] portGroupId              - The port group Id.
* @param[in] regAddr                  - The register's address to write to.
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be written to register.
* @param[in] fieldData                - Data to be written into the register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
*
* @note this function actually read the register modifies the requested field
*       and writes the new value back to the HW.
*
*/
static GT_STATUS hwPpPexMbusSetRegField
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   fieldOffset,
    IN GT_U32   fieldLength,
    IN GT_U32   fieldData
)
{
    GT_U32 data = 0, mask;

    debugPrint(("SetField to dev %d, reg 0x%x, offset %d, length %d, \
        data 0x%x.\n",devNum, regAddr, fieldOffset, fieldLength, fieldData));

    hwRead(devNum, portGroupId, regAddr, 1, GT_TRUE /*appAccess*/, &data);
    CALC_MASK_MAC(fieldLength, fieldOffset, mask);
    data &= ~mask;   /* turn the field off */
    /* insert the new value of field in its place */
    data |= ((fieldData << fieldOffset) & mask);
    prvCpssDrvTraceHwDataMask[devNum] = mask;
    hwWrite(devNum, portGroupId, regAddr, 1, GT_TRUE /*appAccess*/, &data);
    prvCpssDrvTraceHwDataMask[devNum] = 0xFFFFFFFF;
    return GT_OK;
}

/**
* @internal hwPpPexMbusReadRegBitMask function
* @endinternal
*
* @brief   Reads the unmasked bits of a register.
*
* @param[in] devNum                   - PP device number to read from.
* @param[in] portGroupId              - The port group Id.
* @param[in] regAddr                  - Register address to read from.
* @param[in] mask                     - Mask for selecting the read bits.
*
* @param[out] dataPtr                  - Data read from register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
*
* @note The bits in value to be read are the masked bit of 'mask'.
*
*/
static GT_STATUS hwPpPexMbusReadRegBitMask
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   mask,
    OUT GT_U32  *dataPtr
)
{
    GT_U32 data = 0;

    debugPrint(("\nhwPpReadRegBitMask on device %d, from register 0x%x, \
                mask 0x%x,", devNum, regAddr, mask));

    hwRead(devNum, portGroupId, regAddr, 1, GT_TRUE /*appAccess*/, &data);
    *dataPtr = (data) & mask;

    debugPrint((" data is 0x%x.\n", *dataPtr));
    return GT_OK;
}

/**
* @internal hwPpPexMbusWriteRegBitMask function
* @endinternal
*
* @brief   Writes the unmasked bits of a register.
*
* @param[in] devNum                   - PP device number to write to.
* @param[in] portGroupId              - The port group Id.
* @param[in] regAddr                  - Register address to write to.
* @param[in] mask                     - Mask for selecting the written bits.
* @param[in] value                    - Data to be written to register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
*
* @note The bits in value to be written are the masked bit of 'mask'.
*
*/
static GT_STATUS hwPpPexMbusWriteRegBitMask
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   mask,
    IN GT_U32   value
)

{
    GT_U32 data = 0;

    debugPrint(("\nhwPpWriteRegBitMask on device %d, from register 0x%x, \
                mask 0x%x, data is 0x%x.\n",
                devNum, regAddr, mask, value));

    prvCpssDrvTraceHwDataMask[devNum]=mask;
    /* in case mask = 0xffffffff call hwWrite only */
    if(mask == 0xffffffff)
    {
        hwWrite(devNum, portGroupId, regAddr, 1, GT_TRUE /*appAccess*/, &value);
        prvCpssDrvTraceHwDataMask[devNum]=0xFFFFFFFF;
        return GT_OK;
    }
    /* else read modify write */
    hwRead(devNum, portGroupId,regAddr, 1, GT_TRUE /*appAccess*/, &data);
    data &= (~mask);            /* Turn the field off.                        */
    data |= (value & mask);     /* Insert the new value of field in its place.*/
    hwWrite(devNum, portGroupId, regAddr, 1, GT_TRUE /*appAccess*/, &data);
    prvCpssDrvTraceHwDataMask[devNum]=0xFFFFFFFF;
    return GT_OK;
}

/**
* @internal hwPpPexMbusReadRam function
* @endinternal
*
* @brief   Read from PP's RAM.
*
* @param[in] devNum                   - The PP device number to read from.
* @param[in] portGroupId              - The port group Id.
* @param[in] addr                     - Address offset to read from.
* @param[in] length                   - Number of Words (4 byte) to read.
*
* @param[out] data                     - An array containing the read data.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
*/
static GT_STATUS hwPpPexMbusReadRam
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   addr,
    IN GT_U32   length,
    OUT GT_U32  *data
)
{
    PRV_CPSS_DRV_RAM_BURST_INFO_STC   *ramBurstInfo;  /* Holds the devices ram burst  */
                                        /* parameters.                  */
    GT_U32               i;

    for( i = 0; i < length; i++)
    {
        debugPrint(("\nhwPpReadRam on device %d, from address 0x%x, \
                    data is 0x%x.\n",
                    devNum, addr + 4 * i, data[i]));
    }

    ramBurstInfo = HWCTRL(devNum,portGroupId).ramBurstInfo;
    for(i = 0; i < HWCTRL(devNum,portGroupId).ramBurstInfoLen; i++)
    {
        if(ramBurstInfo[i].ramBase == (ramBurstInfo[i].ramBaseMask & addr))
            return hwPpReadBurst(devNum,portGroupId,addr,length,&(ramBurstInfo[i]),data);
    }

    hwRead(devNum, portGroupId,addr, length, GT_TRUE /*appAccess*/, data);

    return GT_OK;
}

/**
* @internal hwPpPexMbusWriteRam function
* @endinternal
*
* @brief   Writes to PP's RAM.
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] portGroupId              - The port group Id.
* @param[in] addr                     - Address offset to write to.
* @param[in] length                   - Number of Words (4 byte) to write.
* @param[in] data                     - An array containing the  to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
*/
static GT_STATUS hwPpPexMbusWriteRam
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   addr,
    IN GT_U32   length,
    IN GT_U32   *data
)
{
    GT_STATUS           retVal;
    PRV_CPSS_DRV_RAM_BURST_INFO_STC   *ramBurstInfo;  /* Holds the devices ram burst  */
                                        /* parameters.                  */
    GT_U32              i;
    SYSTEM_STATE_CAUSED_SKIP_MAC;

    if(memoryAccessTraceOn == GT_TRUE)
    {
        for( i = 0; i < length; i++)
        {
          debugPrint(("\nhwPpWriteRam on device %d, from address 0x%x, \
                       data is 0x%x.\n",devNum, addr + i * 4, data[i]));
        }
    }
    ramBurstInfo = HWCTRL(devNum,portGroupId).ramBurstInfo;
    for(i = 0; i < HWCTRL(devNum,portGroupId).ramBurstInfoLen; i++)
    {
        if(ramBurstInfo[i].ramBase == (ramBurstInfo[i].ramBaseMask & addr))
        {
            retVal =  hwPpWriteBurst(devNum,portGroupId,addr,length,data,
                                     &(ramBurstInfo[i]));
            return retVal;
        }
    }

    hwWrite(devNum, portGroupId,addr, length, GT_TRUE /*appAccess*/, data);
    return GT_OK;
}

/**
* @internal hwPpPexMbusReadVec function
* @endinternal
*
* @brief   Read from PP's RAM.
*
* @param[in] devNum                   - The PP device number to read from.
* @param[in] portGroupId              - The port group Id.
*                                      addr    - Address offset to read from.
*                                      length  - Number of Words (4 byte) to read.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
*/
static GT_STATUS hwPpPexMbusReadVec
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   addrArr[],
    OUT GT_U32  dataArr[],
    IN GT_U32   arrLen
)
{
    GT_U32      i;
    GT_STATUS   rc;

    for (i = 0; i < arrLen; i++)
    {
        rc = hwRead(
            devNum, portGroupId, addrArr[i], 1, GT_TRUE /*appAccess*/, &dataArr[i]);
        if (GT_OK != rc)
        {
            return rc;
        }
        debugPrint(("\nhwPpPexMbusReadVec on device %d, from address 0x%x, \
                    data is 0x%x.\n",
                    devNum, addrArr[i], dataArr[i]));
    }

    return GT_OK;
}


/**
* @internal hwPpPexMbusWriteVec function
* @endinternal
*
* @brief   Writes to PP's RAM.
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] portGroupId              - The port group Id.
*                                      addr    - Address offset to write to.
*                                      length  - Number of Words (4 byte) to write.
*                                      data    - An array containing the data to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
*/
static GT_STATUS hwPpPexMbusWriteVec
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   addrArr[],
    IN GT_U32   dataArr[],
    IN GT_U32   arrLen
)
{
    GT_U32      i;
    GT_STATUS   rc;

    for (i = 0; i < arrLen; i++)
    {
        rc = hwWrite(
            devNum, portGroupId,addrArr[i], 1, GT_TRUE /*appAccess*/, &dataArr[i]);
        if (GT_OK != rc)
        {
            return rc;
        }
        debugPrint(("\nhwPpPexMbusWriteVec on device %d, from address 0x%x, \
                    data is 0x%x.\n",
                    devNum, addrArr[i], dataArr[i]));
    }

    return GT_OK;
}

/**
* @internal hwPpPexMbusWriteRamInReverse function
* @endinternal
*
* @brief   Writes to PP's RAM in reverse.
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] portGroupId              - The port group Id.
* @param[in] addr                     - Address offset to write to.
* @param[in] length                   - Number of Words (4 byte) to write.
* @param[in] data                     - An array containing the  to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
*/
static GT_STATUS hwPpPexMbusWriteRamInReverse
(
    IN GT_U8 devNum,
    IN GT_U32   portGroupId,
    IN GT_U32 addr,
    IN GT_U32 length,
    IN GT_U32 *data
)
{
    GT_STATUS           status;
    GT_U32              i = 0;
    if(memoryAccessTraceOn == GT_TRUE)
    {
        for( i = 0; i < length; i++)
        {
           debugPrint(("\nhwPpWriteRam on device %d, from address 0x%x, \
                        data is 0x%x.\n",devNum, addr + i * 4, data[i]));
        }
    }
    status = hwWriteInReverse(
        devNum, portGroupId,addr, length, GT_TRUE /*appAccess*/, data);
    return status;
}

/**
* @internal hwPpPexMbusIsrRead function
* @endinternal
*
* @brief   Read a register value using special interrupt address completion region.
*
* @param[in] devNum                   - The PP to read from.
* @param[in] portGroupId              - The port group Id.
* @param[in] regAddr                  - The register's address to read from.
*                                      Note: regAddr should be < 0x1000000
*
* @param[out] dataPtr                  - Includes the register value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
*/
static GT_STATUS hwPpPexMbusIsrRead
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   *dataPtr
)
{
    debugPrint(("Read from dev %d, reg 0x%x,",devNum, regAddr));
    hwRead(devNum, portGroupId, regAddr, 1, GT_FALSE /*appAccess*/, dataPtr);
    debugPrint((" data 0x%x.\n", *dataPtr));
    return GT_OK;
}

/**
* @internal hwPpPexMbusIsrWrite function
* @endinternal
*
* @brief   Write a register value using special interrupt address completion region
*
* @param[in] devNum                   - The PP to write to.
* @param[in] portGroupId              - The port group Id.
* @param[in] regAddr                  - The register's address to write to.
*                                      data    - The value to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
*/
static GT_STATUS hwPpPexMbusIsrWrite
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   value
)
{
    debugPrint(
        ("Write to device %d, reg 0x%x, data 0x%x.\n",devNum,regAddr,
        value));

    hwWrite(devNum, portGroupId, regAddr, 1, GT_FALSE /*appAccess*/, &value);

    return GT_OK;
}

/**
* @internal hwPpPexMbusReadInternalPciReg function
* @endinternal
*
* @brief   This function reads from an internal pci register, it's used by the
*         initialization process and the interrupt service routine.
* @param[in] devNum                   - The Pp's device numbers.
* @param[in] portGroupId              - The port group Id.
* @param[in] regAddr                  - The register's address to read from.
*
* @param[out] data                     - The read data.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS hwPpPexMbusReadInternalPciReg
(
    IN  GT_U8   devNum,
    IN GT_U32   portGroupId,
    IN  GT_U32  regAddr,
    OUT GT_U32  *data
)
{

    GT_U32  temp;

#ifndef ASIC_SIMULATION
    GT_UINTPTR address;     /* physical access address for PCI transaction. */
    address = (HWINFO(devNum,portGroupId).resource.cnm.start + regAddr);
#endif

    READ_BOOKMARK(devNum);

#ifdef ASIC_SIMULATION
    scibPciRegRead(SIM_DEV_ID(devNum,portGroupId), regAddr, 1, &temp);
#else /*ASIC_SIMULATION*/
    DEBUG_READ_ADDR(address);
    temp = *((volatile GT_U32 *)address);
    GT_SYNC; /*to avoid read combining */
#endif /*ASIC_SIMULATION*/


    *data = CPSS_32BIT_LE(temp);

    /* Check if trace hw read is enabled */
    if (prvCpssDrvTraceHwRead[devNum] == GT_TRUE)
    {
        /* Check if trace hw read is enabled */
        return cpssTraceHwAccessRead(devNum,
                                     portGroupId,
                                     GT_FALSE,  /* ISR context */
                                     CPSS_TRACE_HW_ACCESS_ADDR_SPACE_PCI_PEX_E,   /* PCI/PEX space */
                                     regAddr,
                                     1,    /* length to trace the data in words */
                                     data);
    }

    return GT_OK;
}


/**
* @internal hwPpPexMbusWriteInternalPciReg function
* @endinternal
*
* @brief   This function reads from an internal pci register, it's used by the
*         initialization process and the interrupt service routine.
* @param[in] devNum                   - The Pp's device numbers.
* @param[in] portGroupId              - The port group Id.
* @param[in] regAddr                  - The register's address to read from.
* @param[in] data                     - Data to be written.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS hwPpPexMbusWriteInternalPciReg
(
    IN  GT_U8   devNum,
    IN GT_U32   portGroupId,
    IN  GT_U32  regAddr,
    IN  GT_U32  data
)
{
    GT_U32     leData;      /* LE data */

#ifndef ASIC_SIMULATION
    GT_UINTPTR address;     /* physical address for PCI transaction.    */
#endif
    SYSTEM_STATE_CAUSED_SKIP_MAC;

#ifndef ASIC_SIMULATION
    address = (HWINFO(devNum,portGroupId).resource.cnm.start + regAddr);
#endif

    WRITE_BOOKMARK(devNum);

    leData = CPSS_32BIT_LE(data);

#ifdef ASIC_SIMULATION
    scibPciRegWrite(SIM_DEV_ID(devNum,portGroupId), regAddr, 1, &leData);
#else /*ASIC_SIMULATION*/
    DEBUG_WRITE_ADDR(address,leData);

    *((volatile GT_U32*)address) = leData;
    /* need to put GT_SYNC to avoid from having the CPU doing
    * write combine. Some CPUs like advanced new PPC can do write combining
    * of two writes to continuous addresses. So, to avoid that we use the eioio
    * in PPC architecture */
    GT_SYNC; /* to avoid from write combining */
#endif /*ASIC_SIMULATION*/

    /* Check if trace hw write is enabled */
    if (prvCpssDrvTraceHwWrite[devNum] == GT_TRUE)
    {
        /* Check if trace hw write is enabled */
        return cpssTraceHwAccessWrite(devNum,
                                      portGroupId,
                                      GT_FALSE, /* ISR context */
                                      CPSS_TRACE_HW_ACCESS_ADDR_SPACE_PCI_PEX_E,  /* PCI/PEX space */
                                      regAddr,
                                      1,    /* length to trace the data in words */
                                      &data,
                                      0xFFFFFFFF);
    }

    return GT_OK;
}

/**
* @internal hwPpPexMbusResetAndInitCtrlReadRegister function
* @endinternal
*
* @brief   This function reads a register from the Reset and Init Controller.
*
* @param[in] devNum                   - The Pp's device numbers.
* @param[in] regAddr                  - The register's address to read from.
*
* @param[out] data                     - The read data.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS hwPpPexMbusResetAndInitCtrlReadRegister
(
    IN  GT_U8   devNum,
    IN  GT_U32  regAddr,
    OUT GT_U32  *data
)
{
    GT_U32     temp;        /* data in HW format                        */
    GT_UINTPTR address;     /* physical access address for transaction. */

    READ_BOOKMARK(devNum);

#ifdef ASIC_SIMULATION
    address = regAddr;
    scibMemoryClientRegRead(
        SIM_DEV_ID(devNum,0), SCIB_MEM_ACCESS_DFX_E, address, 1, &temp);
#else /*ASIC_SIMULATION*/
    address = (HWINFO(devNum,0).resource.resetAndInitController.start + regAddr);
    DEBUG_READ_ADDR(address);
    temp = *((volatile GT_U32 *)address);
    GT_SYNC; /*to avoid read combining */
#endif /*ASIC_SIMULATION*/

    *data = CPSS_32BIT_LE(temp);

    debugPrint(("Reset and Init Controller Read: addr 0x%X data 0x%X \n",
                regAddr, *data));

    /* Check if trace hw read is enabled */
    if (prvCpssDrvTraceHwRead[devNum] == GT_TRUE)
    {
        return cpssTraceHwAccessRead(devNum,
                                     0,
                                     GT_FALSE,  /* ISR context */
                                     CPSS_TRACE_HW_ACCESS_ADDR_SPACE_RESET_AND_INIT_CTRL_E,   /* Address space */
                                     regAddr,
                                     1,    /* length to trace the data in words */
                                     data);
    }

    return GT_OK;
}

/**
* @internal hwPpPexMbusResetAndInitCtrlWriteRegister function
* @endinternal
*
* @brief   This function writes a register to the Reset and Init Controller.
*
* @param[in] devNum                   - The Pp's device numbers.
* @param[in] regAddr                  - The register's address to wrire to.
* @param[in] data                     - Data to be written.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS hwPpPexMbusResetAndInitCtrlWriteRegister
(
    IN  GT_U8   devNum,
    IN  GT_U32  regAddr,
    IN  GT_U32  data
)
{
    GT_U32     temp;        /* data in HW format                        */
    GT_UINTPTR address;     /* physical access address for transaction. */

    SYSTEM_STATE_CAUSED_SKIP_MAC;

    debugPrint(("Reset and Init Controller Write: addr 0x%X data 0x%X \n", regAddr, data));

    temp = CPSS_32BIT_LE(data);

    WRITE_BOOKMARK(devNum);

#ifdef ASIC_SIMULATION
    address = regAddr;
    scibMemoryClientRegWrite(
        SIM_DEV_ID(devNum,0), SCIB_MEM_ACCESS_DFX_E, address, 1, &temp);
#else /*ASIC_SIMULATION*/
    address = (HWINFO(devNum,0).resource.resetAndInitController.start + regAddr);

    DEBUG_WRITE_ADDR(address,temp);

    *((volatile GT_U32*)address) = temp;
    GT_SYNC; /*to avoid read combining */
#endif /*ASIC_SIMULATION*/

    /* Check if trace hw write is enabled */
    if (prvCpssDrvTraceHwWrite[devNum] == GT_TRUE)
    {
        return cpssTraceHwAccessWrite(devNum,
                                      0,
                                      GT_FALSE, /* ISR context */
                                      CPSS_TRACE_HW_ACCESS_ADDR_SPACE_RESET_AND_INIT_CTRL_E,  /* Address space */
                                      regAddr,
                                      1,    /* length to trace the data in words */
                                      &data,
                                      prvCpssDrvTraceHwDataMask[devNum]);
    }

    return GT_OK;
}

/**
* @internal hwPpPexMbusResetAndInitCtrlGetRegField function
* @endinternal
*
* @brief   This function reads a selected register field from the Reset and Init
*         Controller.
* @param[in] devNum                   - The Pp's device numbers.
* @param[in] regAddr                  - The register's address to read from.
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be read.
*
* @param[out] fieldData                - the read field data.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS hwPpPexMbusResetAndInitCtrlGetRegField
(
    IN GT_U8    devNum,
    IN GT_U32   regAddr,
    IN GT_U32   fieldOffset,
    IN GT_U32   fieldLength,
    OUT GT_U32  *fieldData
)
{
    GT_STATUS rc;
    GT_U32 data = 0, mask;

    rc = hwPpPexMbusResetAndInitCtrlReadRegister(devNum, regAddr, &data);
    if( GT_OK != rc )
    {
        return rc;
    }

    CALC_MASK_MAC(fieldLength, fieldOffset, mask);
    *fieldData = (GT_U32)((data & mask) >> fieldOffset);

    return GT_OK;
}

/**
* @internal hwPpPexMbusResetAndInitCtrlSetRegField function
* @endinternal
*
* @brief   This function writes a selected register field to the Reset and Init
*         Controller.
* @param[in] devNum                   - The Pp's device numbers.
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be written to register.
* @param[in] fieldData                - Data to be written into the register.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS hwPpPexMbusResetAndInitCtrlSetRegField
(
    IN GT_U8    devNum,
    IN GT_U32   regAddr,
    IN GT_U32   fieldOffset,
    IN GT_U32   fieldLength,
    IN GT_U32   fieldData
)
{
    GT_STATUS rc;
    GT_U32 data = 0, mask;

    rc = hwPpPexMbusResetAndInitCtrlReadRegister(devNum, regAddr, &data);
    if( GT_OK != rc )
    {
        return rc;
    }

    CALC_MASK_MAC(fieldLength, fieldOffset, mask);
    data &= ~mask;   /* turn the field off */
    /* insert the new value of field in its place */
    data |= ((fieldData << fieldOffset) & mask);
    prvCpssDrvTraceHwDataMask[devNum] = mask;
    rc =  hwPpPexMbusResetAndInitCtrlWriteRegister(devNum, regAddr, data);
    prvCpssDrvTraceHwDataMask[devNum] = 0xFFFFFFFF;
    return rc;
}

/**
* @internal driverHwPexMbusCntlInit function
* @endinternal
*
* @brief   This function initializes the Hw control structure of a given PP.
*
* @param[in] devNum                   - The PP's device number to init the structure for.
* @param[in] portGroupId              - The port group Id.
* @param[in] isDiag                   - Is this initialization is for diagnostics purposes
*                                      (GT_TRUE), or is it a final initialization of the Hw
*                                      Cntl unit (GT_FALSE)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
*
* @note 1. In case isDiag == GT_TRUE, no semaphores are initialized.
*       Galtis:
*       None.
*
*/
static GT_STATUS driverHwPexMbusCntlInit
(
    IN GT_U8        devNum,
    IN GT_U32       portGroupId,
    IN GT_BOOL      isDiag
)
{
    GT_U8   i;               /* iterator      */
    char    name[20];        /* name of mutex */
    GT_U32  data;            /* regiser data  */
    GT_U32  regBmpIsr;       /* the bitmap of regions for ISR  */
    GT_U32  regBmpApp;       /* the bitmap of regions for CPSS */
    GT_U32  regBmp;          /* the bitmap of regions (work) */

    HWCTRL(devNum,portGroupId).compIdx            = 0;

    /* Check completion masks validity */
    regBmpIsr = HWCTRL(devNum,portGroupId).isrAddrCompletionRegionsBmp;
    regBmpApp = HWCTRL(devNum,portGroupId).appAddrCompletionRegionsBmp;
    if (((regBmpIsr & 0xFE) == 0) || ((regBmpIsr & (~ 0xFE)) != 0))
    {
        /* regions 1-7 only allowed to use */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (((regBmpApp & 0xFE) == 0) || ((regBmpApp & (~ 0xFE)) != 0))
    {
        /* regions 1-7 only allowed to use */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if ((regBmpIsr & regBmpApp) != 0)
    {
        /* ISR and CPSS must use different regions */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* ISR bitmap bounds */
    regBmp = regBmpIsr;
    /* find first 1-bit index */
    for (i = 0; ((regBmp & 1) == 0); regBmp = (regBmp >> 1), i++){};
    HWCTRL(devNum,portGroupId).isrAddrRegionMin = i;
    /* find last 1-bit index */
    for (/**/; (regBmp != 0); regBmp = (regBmp >> 1), i++){};
    HWCTRL(devNum,portGroupId).isrAddrRegionMax = i - 1;

    /* APP bitmap bounds */
    regBmp = regBmpApp;
    /* find first 1-bit index */
    for (i = 0; ((regBmp & 1) == 0); regBmp = (regBmp >> 1), i++){};
    HWCTRL(devNum,portGroupId).appAddrRegionMin = i;
    /* find last 1-bit index */
    for (/**/; (regBmp != 0); regBmp = (regBmp >> 1), i++){};
    HWCTRL(devNum,portGroupId).appAddrRegionMax = i - 1;

    /* init last used regions as MAX regions */
    HWCTRL(devNum,portGroupId).isrAddrRegionUsed =
        HWCTRL(devNum,portGroupId).isrAddrRegionMax;

    HWCTRL(devNum,portGroupId).appAddrRegionUsed =
        HWCTRL(devNum,portGroupId).appAddrRegionMax;

    /* Reset Hw Address Completion registers not needed         */
    /* The 0-th region must be zero and never updated           */
    /* Other regions were be updated when will be needed after  */
    /* unsucssess searching needed value in shadow              */

    /* Update Address Completion shadow     */
    for (i = 0; i < 8; i++)
    {
        HWCTRL(devNum,portGroupId).addrCompletShadow[i] = 0;
    }

    if ((isDiag == GT_FALSE) && (HWCTRL(devNum,portGroupId).hwComplSem == (CPSS_OS_MUTEX)0))
    {
        cpssOsSprintf (name,"hwComplSem-%d_%ld",devNum,portGroupId);

        cpssOsMutexCreate (name,&(HWCTRL(devNum,portGroupId).hwComplSem));
    }

#ifdef ASIC_SIMULATION
    HWCTRL(devNum,portGroupId).simInfo.devId =
                            (GT_U8)scibGetDeviceId(BASEADDR(devNum,portGroupId));
    HWCTRL(devNum,portGroupId).simInfo.addrCompletEnable =
        scibAddressCompletionStatusGet(SIM_DEV_ID(devNum,portGroupId));
#endif

    if(prvCpssDrvHwPpPrePhase1NextDevFamilyGet() == CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        /* the Falcon not support the address completion in the device */
    }
    else
    {
        /* set 8-region mode: regAddr = 0x140, set bit16 to 0 */
        cpuReadMemoryWord(devNum, portGroupId, 0x140, &data);
        data &= (~ (1 << 16));
        cpuWriteMemoryWord(devNum, portGroupId, 0x140, data);
    }

    return GT_OK;
}


/**
* @internal hwPpPexMbusMgReadRegister function
* @endinternal
*
* @brief   This function reads a register from MG1, MG2 or MG3.
*         MG0 is accesses from the main register functions
* @param[in] devNum                   - The Pp's device numbers.
* @param[in] mgNum                    - The Management Unit number
* @param[in] regAddr                  - The register's address to read from.
*
* @param[out] data                     - The read data.
*
* @retval GT_OK                    - on success,
* @retval GT_NOT_INITIALIZED       - if the MG is not mapped,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS hwPpPexMbusMgReadRegister
(
    IN  GT_U8   devNum,
    IN  GT_U32  mgNum,
    IN  GT_U32  regAddr,
    OUT GT_U32  *data
)
{
    GT_U32     temp;        /* data in HW format                        */
    GT_UINTPTR address;     /* physical access address for transaction. */
    CPSS_TRACE_HW_ACCESS_ADDR_SPACE_ENT mgTrace;

    READ_BOOKMARK(devNum);

#ifdef ASIC_SIMULATION

    address = regAddr;
    temp = 0;
    mgTrace = CPSS_TRACE_HW_ACCESS_ADDR_SPACE_MG1_E; /*avoid warnning*/
    mgNum = mgNum +temp + address;
    /* TBD need to complete sim support
    scibMemoryClientRegRead(
        SIM_DEV_ID(devNum,0), SCIB_MEM_ACCESS_DFX_E, mgNum + address, 1, &temp);*/
#else /*ASIC_SIMULATION*/
    switch (mgNum)
    {
        case 1:
            address = (HWINFO(devNum, 0).resource.mg1.start );
            mgTrace =  CPSS_TRACE_HW_ACCESS_ADDR_SPACE_MG1_E;
            break;
        case 2:
            address = (HWINFO(devNum, 0).resource.mg2.start );
            mgTrace =  CPSS_TRACE_HW_ACCESS_ADDR_SPACE_MG2_E;
            break;
        case 3:
            address = (HWINFO(devNum, 0).resource.mg3.start );
            mgTrace =  CPSS_TRACE_HW_ACCESS_ADDR_SPACE_MG3_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "No Such MG: %u",mgNum);

    }
    if (address == 0)
    {
        return/*do not log as error*/GT_NOT_INITIALIZED;
    }
    else
    {
        address = address + regAddr;
    }

    DEBUG_READ_ADDR(address);

    temp = *((volatile GT_U32 *)address);
    GT_SYNC; /*to avoid read combining */
#endif /*ASIC_SIMULATION*/

    *data = CPSS_32BIT_LE(temp);

    debugPrint(("Reset and Init Controller Read: addr 0x%X data 0x%X \n",
                regAddr, *data));

    /* Check if trace hw read is enabled */
    if (prvCpssDrvTraceHwRead[devNum] == GT_TRUE)
    {
        return cpssTraceHwAccessRead(devNum,
                                     0,
                                     GT_FALSE,  /* ISR context */
                                     mgTrace,   /* Address space */
                                     regAddr,
                                     1,    /* length to trace the data in words */
                                     data);
    }

    return GT_OK;
}
/**
* @internal hwPpPexMbusMgWriteRegister function
* @endinternal
*
* @brief   This function writes a register to the Reset and Init Controller.
*
* @param[in] devNum                   - The Pp's device numbers.
* @param[in] mgNum                    - The Management Unit number
* @param[in] regAddr                  - The register's address to wrire to.
* @param[in] data                     - Data to be written.
*
* @retval GT_OK                    - on success,
* @retval GT_NOT_INITIALIZED       - if the MG is not mapped,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS hwPpPexMbusMgWriteRegister
(
    IN  GT_U8   devNum,
    IN  GT_U32  mgNum,
    IN  GT_U32  regAddr,
    IN  GT_U32  data
)
{
    GT_U32     temp;        /* data in HW format                        */
    GT_UINTPTR address;     /* physical access address for transaction. */
    CPSS_TRACE_HW_ACCESS_ADDR_SPACE_ENT mgTrace;

    SYSTEM_STATE_CAUSED_SKIP_MAC;

    debugPrint(("Reset and Init Controller Write: addr 0x%X data 0x%X \n", regAddr, data));

    temp = CPSS_32BIT_LE(data);

    WRITE_BOOKMARK(devNum);

#ifdef ASIC_SIMULATION
    address = regAddr;
    mgTrace = CPSS_TRACE_HW_ACCESS_ADDR_SPACE_MG1_E; /*avoid warnning*/
    mgNum = mgNum + temp + address;
    /*TBD need to cmplete sim support
    scibMemoryClientRegWrite(
        SIM_DEV_ID(devNum,0), SCIB_MEM_ACCESS_DFX_E, address + mgNum, 1, &temp);*/
#else /*ASIC_SIMULATION*/

    switch (mgNum)
    {
        case 1:
            address = (HWINFO(devNum, 0).resource.mg1.start);
            mgTrace =  CPSS_TRACE_HW_ACCESS_ADDR_SPACE_MG1_E;
            break;
        case 2:
            address = (HWINFO(devNum, 0).resource.mg2.start);
            mgTrace =  CPSS_TRACE_HW_ACCESS_ADDR_SPACE_MG2_E;
            break;
        case 3:
            address = (HWINFO(devNum, 0).resource.mg3.start);
            mgTrace =  CPSS_TRACE_HW_ACCESS_ADDR_SPACE_MG3_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "No Such MG: %u",mgNum);
    }
    if (address == 0)
    {
        return/*do not log as error*/GT_NOT_INITIALIZED;
    }
    else
    {
        address = address + regAddr;
    }

    DEBUG_WRITE_ADDR(address,temp);

    *((volatile GT_U32*)address) = temp;
    GT_SYNC; /*to avoid read combining */
#endif /*ASIC_SIMULATION*/

    /* Check if trace hw write is enabled */
    if (prvCpssDrvTraceHwWrite[devNum] == GT_TRUE)
    {
        return cpssTraceHwAccessWrite(devNum,
                                      0,
                                      GT_FALSE, /* ISR context */
                                      mgTrace,  /* Address space */
                                      regAddr,
                                      1,    /* length to trace the data in words */
                                      &data,
                                      prvCpssDrvTraceHwDataMask[devNum]);
    }

    return GT_OK;
}

/**
* @internal hwPpPexMbusMgGetRegField function
* @endinternal
*
* @brief   This function reads a selected register field from the Management Unit.
*
* @param[in] devNum                   - The Pp's device numbers.
* @param[in] mgNum                    - The management unit number
* @param[in] regAddr                  - The register's address to read from.
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be read.
*
* @param[out] fieldData                - the read field data.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS hwPpPexMbusMgGetRegField
(
    IN GT_U8    devNum,
    IN GT_U32   mgNum,
    IN GT_U32   regAddr,
    IN GT_U32   fieldOffset,
    IN GT_U32   fieldLength,
    OUT GT_U32  *fieldData
)
{
    GT_STATUS rc;
    GT_U32 data = 0, mask;

    rc = hwPpPexMbusMgReadRegister(devNum, mgNum, regAddr, &data);
    if( GT_OK != rc )
    {
        return rc;
    }

    CALC_MASK_MAC(fieldLength, fieldOffset, mask);
    *fieldData = (GT_U32)((data & mask) >> fieldOffset);

    return GT_OK;
}

/**
* @internal hwPpPexMbusMgSetRegField function
* @endinternal
*
* @brief   This function writes a selected register field to the Management Unit.
*
* @param[in] devNum                   - The Pp's device numbers.
* @param[in] mgNum                    - The management unit number
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be written to register.
* @param[in] fieldData                - Data to be written into the register.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS hwPpPexMbusMgSetRegField
(
    IN GT_U8    devNum,
    IN GT_U32   mgNum,
    IN GT_U32   regAddr,
    IN GT_U32   fieldOffset,
    IN GT_U32   fieldLength,
    IN GT_U32   fieldData
)
{
    GT_STATUS rc;
    GT_U32 data = 0, mask;

    rc = hwPpPexMbusMgReadRegister(devNum, mgNum, regAddr, &data);
    if( GT_OK != rc )
    {
        return rc;
    }

    CALC_MASK_MAC(fieldLength, fieldOffset, mask);
    data &= ~mask;   /* turn the field off */
    /* insert the new value of field in its place */
    data |= ((fieldData << fieldOffset) & mask);
     prvCpssDrvTraceHwDataMask[devNum] = mask;
     rc = hwPpPexMbusMgWriteRegister(devNum, mgNum, regAddr, data);
     prvCpssDrvTraceHwDataMask[devNum] = 0xFFFFFFFF;
     return rc;
}
/**
* @internal prvCpssDrvHwPexMbusDriverObjectInit function
* @endinternal
*
* @brief   This function creates and initializes PEX device with 8 address complete regions driver object
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - if the driver object have been created before
*/
GT_STATUS prvCpssDrvHwPexMbusDriverObjectInit
(
     void
)
{
#ifdef INCL_EXDXMX_DRIVER
    /* driver object initialization */
    prvCpssDrvMngInfPexMbusPtr->drvHwPpReadReg         = hwPpPexMbusReadRegister;
    prvCpssDrvMngInfPexMbusPtr->drvHwPpWriteReg        = hwPpPexMbusWriteRegister;
    prvCpssDrvMngInfPexMbusPtr->drvHwPpGetRegField     = hwPpPexMbusGetRegField;
    prvCpssDrvMngInfPexMbusPtr->drvHwPpSetRegField     = hwPpPexMbusSetRegField;
    prvCpssDrvMngInfPexMbusPtr->drvHwPpReadRegBitMask  = hwPpPexMbusReadRegBitMask;
    prvCpssDrvMngInfPexMbusPtr->drvHwPpWriteRegBitMask = hwPpPexMbusWriteRegBitMask;
    prvCpssDrvMngInfPexMbusPtr->drvHwPpReadRam         = hwPpPexMbusReadRam;
    prvCpssDrvMngInfPexMbusPtr->drvHwPpWriteRam        = hwPpPexMbusWriteRam;
    prvCpssDrvMngInfPexMbusPtr->drvHwPpReadVec         = hwPpPexMbusReadVec;
    prvCpssDrvMngInfPexMbusPtr->drvHwPpWriteVec        = hwPpPexMbusWriteVec;
    prvCpssDrvMngInfPexMbusPtr->drvHwPpWriteRamRev     = hwPpPexMbusWriteRamInReverse;
    prvCpssDrvMngInfPexMbusPtr->drvHwPpIsrRead         = hwPpPexMbusIsrRead;
    prvCpssDrvMngInfPexMbusPtr->drvHwPpIsrWrite        = hwPpPexMbusIsrWrite;
    prvCpssDrvMngInfPexMbusPtr->drvHwPpReadIntPciReg   = hwPpPexMbusReadInternalPciReg;
    prvCpssDrvMngInfPexMbusPtr->drvHwPpWriteIntPciReg  = hwPpPexMbusWriteInternalPciReg;
    prvCpssDrvMngInfPexMbusPtr->drvHwPpResetAndInitControllerReadReg     =
        hwPpPexMbusResetAndInitCtrlReadRegister;
    prvCpssDrvMngInfPexMbusPtr->drvHwPpResetAndInitControllerWriteReg    =
        hwPpPexMbusResetAndInitCtrlWriteRegister;
    prvCpssDrvMngInfPexMbusPtr->drvHwPpResetAndInitControllerGetRegField =
        hwPpPexMbusResetAndInitCtrlGetRegField;
    prvCpssDrvMngInfPexMbusPtr->drvHwPpResetAndInitControllerSetRegField =
        hwPpPexMbusResetAndInitCtrlSetRegField;
    prvCpssDrvMngInfPexMbusPtr->drvHwCntlInit          = driverHwPexMbusCntlInit;
    prvCpssDrvMngInfPexMbusPtr->drvHwPpMgReadReg       = hwPpPexMbusMgReadRegister;
    prvCpssDrvMngInfPexMbusPtr->drvHwPpMgWriteReg      = hwPpPexMbusMgWriteRegister;
    prvCpssDrvMngInfPexMbusPtr->drvHwPpMgGetRegField   = hwPpPexMbusMgGetRegField;
    prvCpssDrvMngInfPexMbusPtr->drvHwPpMgSetRegField   = hwPpPexMbusMgSetRegField;
#endif /*INCL_EXDXMX_DRIVER*/
    return GT_OK;
}

