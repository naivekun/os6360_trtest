/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
#define _BSD_SOURCE
#define _POSIX_SOURCE
#define _DEFAULT_SOURCE

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <endian.h>


#ifndef PAGE_SHIFT
#define PAGE_SHIFT 12
#endif

#if defined(CHX_FAMILY)
/* CPSS Build. Use CPSS types and services */
#include <gtOs/gtOsTimer.h>
#include <gtExtDrv/drivers/gtSmiDrvCtrl.h>
#include "drivers/mvResources.h"

#else /* CHX_FAMILY */

/* Build without CPSS. Use standard types and define CPSS specific. */
#include <inttypes.h>

typedef unsigned int GT_STATUS;
typedef unsigned int GT_U32;
#define IN
#define OUT
#define GT_ERROR                 (-1)
#define GT_OK                    (0x00) /* Operation succeeded */
#define GT_FAIL                  (0x01) /* Operation failed    */
#define GT_BAD_PARAM             (0x04) /* Illegal parameter in function called  */

#define MV_RESOURCE_SIZE                0x00000000
#define MV_RESOURCE_MBUS_PSS_PORTS      7

GT_STATUS osDelay(IN GT_U32 delay);

#endif /* CHX_FAMILY */

#undef		XSMI_DEBUG
#ifdef		XSMI_DEBUG
#define     xsmi_debug_printf(x)			printf x
#else
#define     xsmi_debug_printf(x)
#endif

#define     CHECK_DEV_MAC(devAddr) \
    if(devAddr>32) {\
        xsmiErr("Illegal device");\
        return GT_BAD_PARAM;\
    }
#define     CHECK_REG_ADDR_MAC(regAddr) \
    if(regAddr>0xFFFF) {\
        xsmiErr("Illegal reg address");\
        return GT_BAD_PARAM;\
    }
#define     CHECK_REG_VAL_MAC(regAddr) \
    if(regAddr>0xFFFF) {\
        xsmiErr("Illegal reg value");\
        return GT_BAD_PARAM;\
    }


/* resource */
#define XSMI_RESOURCE   MV_RESOURCE_MBUS_PSS_PORTS

#define XSMI_TIMEOUT    150 /* = 150us */
#define NTRIES          5
#define BIT(n)          (1 << n)
#define XBUSY           BIT(30)
#define XREAD_VALID     BIT(29)

#define XOPCODE_OFFS        26
#define XOPCODE_ADDR_READ   (7 << XOPCODE_OFFS)
#define XOPCODE_ADDR_WRITE  (5 << XOPCODE_OFFS)
#define XPHYADDR_OFFS       16 /* Phy port addr offset */
#define XPORT_ADDR(x)       ((x & 0x1f) << XPHYADDR_OFFS)
#define XDEVADDR_OFFS       21  /* Phy device addr offset */
#define XDEV_ADDR(x)        ((x & 0x1f) << XDEVADDR_OFFS)

/* Hard coded for now. 
   Should be configurable to allow different SoCs
*/
#ifndef CPU_AARCH64v8
#define XMDIO_MNGMT_BASE    0x30000
#else /* defined CPU_AARCH64v8 */
/* phys: 0xf212a600
 * map:  0xf2120000
 */
#define XMDIO_MMAP_OFFSET   0xf2120000
#define XMDIO_MMAP_SIZE     0x00010000
#define XMDIO_MNGMT_BASE    0x0000a600
#endif
#define XMDIO_REG_MNGMNT    0x0
#define XMDIO_REG_ADDR      0x8
#define XMDIO_REG_CONFIG    0xc

static int mbusDrvFd = -1;
static void *xsmiVirtAddr = MAP_FAILED;

static void xsmiErr(const char *msg)
{
    fprintf(stderr, "%s\n", msg);
}

/**
* @internal extDrvXSmiInit function
* @endinternal
*
* @brief   Inits XSMI subsystem
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS extDrvXSmiInit(void)
{
#ifndef CPU_AARCH64v8
    unsigned long long res;

    if (mbusDrvFd == -1) {
        mbusDrvFd = open("/dev/mvMbusDrv", O_RDWR);
        if (mbusDrvFd < 0)
        {
            perror("can't open /dev/mvMbusDrv");
            return GT_ERROR;
        }
    }

    if(xsmiVirtAddr == MAP_FAILED) {
        lseek(mbusDrvFd, XSMI_RESOURCE | MV_RESOURCE_SIZE, 0);
        if(read(mbusDrvFd, &res, sizeof(res)) <0) {
            xsmiErr("Can't read XSMI resource size");
            return GT_ERROR;
        }

        xsmiVirtAddr = mmap(NULL, (size_t)res,
                            PROT_READ | PROT_WRITE, MAP_SHARED, mbusDrvFd,
                            (off_t)(XSMI_RESOURCE << PAGE_SHIFT));
        if(xsmiVirtAddr == MAP_FAILED) {
            perror("resource map failed");
            return GT_ERROR;
        }
    }

    return GT_OK;
#else /* defined CPU_AARCH64v8 */
    if (mbusDrvFd == -1) {
        mbusDrvFd = open("/dev/mem", O_RDWR);
        if (mbusDrvFd < 0)
        {
            perror("can't open /dev/mem");
            return GT_ERROR;
        }
    }
    if(xsmiVirtAddr == MAP_FAILED) {
        xsmiVirtAddr = mmap(NULL, XMDIO_MMAP_SIZE,
                            PROT_READ | PROT_WRITE, MAP_SHARED, mbusDrvFd,
                            XMDIO_MMAP_OFFSET);
        if(xsmiVirtAddr == MAP_FAILED) {
            perror("resource map from /dev/mem failed");
            return GT_ERROR;
        }
    }
    return GT_OK;
#endif
}


static GT_U32 xsmiRegRead(GT_U32 reg_addr)
{
    return le32toh(*((volatile GT_U32 *)(((uintptr_t)xsmiVirtAddr) + XMDIO_MNGMT_BASE + reg_addr)));
}
static void xsmiRegWrite(int reg_addr, uint32_t data)
{
    *((volatile GT_U32 *)(((uintptr_t)xsmiVirtAddr) + XMDIO_MNGMT_BASE + reg_addr)) = htole32(data);
}

/* Check if XSMI bus is idle */
static int xsmiChckIdle(void)
{
    return !(xsmiRegRead(XMDIO_REG_MNGMNT) & XBUSY);
}

static void xsmiSleep(void)
{
    osDelay(XSMI_TIMEOUT);
}

/* Wait until XSMI bus is free */
static int xsmiWaitFree(void)
{
    int ntries = NTRIES;

    while (ntries > 0) {
        if (xsmiChckIdle())
            return 0;

        xsmiSleep();
        ntries--;
    }

    xsmiErr("timeout while waiting for bus to be free\n");
    return -1;
}

/* Wait until XSMI bus read operation is done */
static int xsmiWaitReadDone(void)
{
    int ntries = NTRIES;
    GT_U32 data;

    while (ntries > 0) {
        data = xsmiRegRead(XMDIO_REG_MNGMNT);
        /* Check if XSMI bus read operaton is done */
        if (data & XREAD_VALID)
            return data & 0xffff;

        xsmiSleep();
        ntries--;
    }

    xsmiErr("timeout while waiting for bus read operation is done");
    return -1;
}

/**
* @internal extDrvXSmiReadReg function
* @endinternal
*
* @brief   Reads a register using XSMI Address
*
* @param[in] phyId                    -  ID
* @param[in] devAddr                  - (Clause 45) Device Address (page number in case of PHY)
* @param[in] regAddr                  - Register address to read from.
*
* @param[out] dataPtr                  - pointer to data read from register.
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
* @retval GT_BAD_PARAM             - bad param
*/
GT_STATUS extDrvXSmiReadReg
(
    IN  GT_U32  phyId,
    IN  GT_U32  devAddr,
    IN  GT_U32  regAddr,
    OUT GT_U32 *dataPtr
)
{
    GT_U32 reg_val;
    int ret;

    xsmi_debug_printf(("reading phy %d dev %d reg 0x%x\n", phyId, devAddr, regAddr));

    CHECK_DEV_MAC(devAddr);
    CHECK_REG_ADDR_MAC(regAddr);

    ret = xsmiWaitFree();
    if (ret)
        return ret;

    /* Write phy reg addr */
    xsmiRegWrite(XMDIO_REG_ADDR, regAddr & 0xffff);

    /* Set phy port and device addrs, and read opcode */
    reg_val = XPORT_ADDR(phyId) | XDEV_ADDR(devAddr) | XOPCODE_ADDR_READ;

    /* Initiate the read operation */
    xsmiRegWrite(XMDIO_REG_MNGMNT, reg_val);

    ret = xsmiWaitReadDone();
    if (ret < 0)
        return GT_ERROR;
    *dataPtr = ret & 0xffff;

    return GT_OK;
}

/**
* @internal extDrvXSmiWriteReg function
* @endinternal
*
* @brief   Writes a register using XSMI Address
*
* @param[in] phyId                    - phy ID
* @param[in] devAddr                  - (Clause 45) Device Address (page number in case of PHY)
* @param[in] regAddr                  - Register address to write.
* @param[in] value                    - Data write to register.
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
* @retval GT_BAD_PARAM             - bad param
*/
GT_STATUS extDrvXSmiWriteReg
(
    IN GT_U32 phyId,
    IN  GT_U32 devAddr,
    IN GT_U32 regAddr,
    IN GT_U32 value
)
{
    GT_U32 reg_val;
    int ret;

    xsmi_debug_printf(("Write phy %d dev %d reg 0x%x\n", phyId, devAddr, regAddr));

    CHECK_DEV_MAC(devAddr);
    CHECK_REG_ADDR_MAC(regAddr);
	CHECK_REG_VAL_MAC(value);

    ret = xsmiWaitFree();
    if (ret)
        return ret;

    /* Write phy reg addr */
    xsmiRegWrite(XMDIO_REG_ADDR, regAddr & 0xffff);

    /* Set phy port and device addrs, write opcode, and value */
    reg_val = XPORT_ADDR(phyId) | XDEV_ADDR(devAddr) | XOPCODE_ADDR_WRITE | value;

    /* Initiate the write operation */
    xsmiRegWrite(XMDIO_REG_MNGMNT, reg_val);

    ret = xsmiWaitFree();
    if (ret)
        return GT_ERROR;

    return GT_OK;
}




