/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <gtExtDrv/drivers/gtDragoniteDrv.h>

/* interface to driver */
#define DRAGONITE_IOC_MAGIC             'd'
#define DRAGONITE_IOC_SETMEM_TYPE       _IOW(DRAGONITE_IOC_MAGIC, 0, bool)
#define DRAGONITE_IOC_UNRESET           _IOW(DRAGONITE_IOC_MAGIC, 1, bool)
#define DRAGONITE_IOC_SENDIRQ           _IOW(DRAGONITE_IOC_MAGIC, 2, bool)

/**
* @internal extDrvDragoniteDriverInit function
* @endinternal
*
* @brief   Initialize dragonite driver - addresses of ITCM/DTCM, method
*         to access POE configuration registers
*         (in Packet Processor address space)
* @param[in] hwInfoPtr                - hardware info srtuctute of selected PP
* @param[in] hwData                   - custom data, will be first parameter of hwWriteMaskedFunc
* @param[in] hwWriteMaskedFunc        - function to write PP registers
*                                       GT_OK if successful
*
* @note Available only in NOKM
*
*/
GT_STATUS extDrvDragoniteDriverInit(
    IN CPSS_HW_INFO_STC   *hwInfoPtr GT_UNUSED,
    IN void*              hwData GT_UNUSED,
    IN DRAGONITE_HW_WRITE_MASKED_FUNC hwWriteMaskedFunc GT_UNUSED
)
{
    return GT_OK;
}

#ifdef XCAT_DRV

#include <cpss/extServices/os/gtOs/gtGenTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <gtExtDrv/drivers/pssBspApis.h>
#include <gtExtDrv/drivers/prvExtDrvLinuxMapping.h>

#include "kerneldrv/include/presteraGlob.h"
extern GT_32 gtPpFd;                /* pp file descriptor           */

/**
* @internal extDrvDragoniteShMemBaseAddrGet function
* @endinternal
*
* @brief   Get start address of communication structure in DTCM
*
* @param[out] dtcmPtr                  - Pointer to beginning of DTCM, where
*                                      communication structures must be placed
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
GT_STATUS extDrvDragoniteShMemBaseAddrGet
(
    OUT  GT_U32  *dtcmPtr
)
{
    struct GT_PCI_VMA_ADDRESSES_STC vmConfig;

    if (ioctl(gtPpFd, PRESTERA_IOC_GETVMA, &vmConfig) == 0)
    {
        *dtcmPtr = (GT_U32)vmConfig.xCatDraginiteBase;
    }
    else
    {
        *dtcmPtr = LINUX_VMA_DRAGONITE;
    }

    return GT_OK;
}

/**
* @internal extDrvDragoniteSWDownload function
* @endinternal
*
* @brief   Download FW to instruction shared memory
*
* @note For debug purposes only
*
*/
GT_STATUS extDrvDragoniteSWDownload
(
    GT_VOID
)
{
    FILE      *fp;
    char config_file_name[128] = "/etc/drigonite_fw.bin";
    GT_U8 *binArray = NULL;
    struct GT_DragoniteSwDownload_STC downloadParams;
    GT_U32    size;       /* size of firmware to download */

    if ( (fp = fopen( config_file_name, "rb")) == NULL)
    {
        cpssOsPrintSync("1)appDemoBspDragoniteSWDownload:can't open %s - %s\n", 
                        config_file_name, strerror(errno));
        return GT_FALSE;
    }

    binArray = cpssOsMalloc(_64K);
    if(binArray == NULL)
    {
        cpssOsPrintSync("2)appDemoBspDragoniteSWDownload:failed to allocate binArray\n");
        return GT_OUT_OF_CPU_MEM;
    }

    for(size = 0; feof(fp) == 0; size++)
    {
        /* read binary file by bytes */
        fread(binArray+size, 1, 1, fp);
        if(ferror(fp) != 0)
            cpssOsPrintSync("3)appDemoBspDragoniteSWDownload:ferror=%d\n", ferror(fp));
    }

    fclose(fp);

    downloadParams.buffer = (mv_kmod_uintptr_t)((uintptr_t)binArray);
    downloadParams.length = size-1;

/*  cpssOsPrintf("4)appDemoBspDragoniteSWDownload:sourcePtr=0x%x,size=%d\n", 
                    downloadParams.buffer, downloadParams.length);
*/
    if (ioctl(gtPpFd, PRESTERA_IOC_DRAGONITESWDOWNLOAD, &downloadParams) < 0)
    {
        cpssOsPrintSync("5)bspDragoniteSWDownload fail\n");
        cpssOsFree(binArray);
        return GT_FAIL;
    }

    cpssOsFree(binArray);

    return GT_OK;
}


/**
* @internal extDrvDragoniteGetIntVec function
* @endinternal
*
* @brief   Get interrupt line number of Dragonite
*
* @param[out] intVec                   - interrupt line number
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*
* @note For debug purposes only
*
*/
GT_STATUS extDrvDragoniteGetIntVec
(
    OUT GT_U32 *intVec
)
{
    if (ioctl(gtPpFd, PRESTERA_IOC_DRAGONITEGETINTVEC, intVec) < 0)
    {
        return GT_FAIL;
    }

    return GT_OK;
}

/**
* @internal extDrvDragoniteInit function
* @endinternal
*
* @brief   Get interrupt line number of Dragonite
*
* @note For debug purposes only
*
*/
GT_STATUS extDrvDragoniteInit
(
    GT_VOID
)
{   
    if (ioctl(gtPpFd, PRESTERA_IOC_DRAGONITEINIT) < 0)
        return GT_FAIL;

    return GT_OK;
}

/**
* @internal extDrvDragoniteEnableSet function
* @endinternal
*
* @brief   Get interrupt line number of Dragonite
*
* @param[in] enable                   - GT_TRUE - release Dragonite from reset
*                                      GT_FALSE - reset Dragonite
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*
* @note For debug purposes only
*
*/
GT_STATUS extDrvDragoniteEnableSet
(
    IN  GT_BOOL enable
)
{
    if (ioctl(gtPpFd, PRESTERA_IOC_DRAGONITEENABLE, enable) < 0)
        return GT_FAIL;

    return GT_OK;
}

/**
* @internal extDrvDragoniteFwCrcCheck function
* @endinternal
*
* @brief   This routine executes Dragonite firmware checksum test
*
* @note Mostly for debug purposes, when FW download executed by CPSS CRC check
*       engaged automatically
*
*/
GT_STATUS extDrvDragoniteFwCrcCheck
(
    GT_VOID
)
{   
    if (ioctl(gtPpFd, PRESTERA_IOC_DRAGONITEITCMCRCCHECK) < 0)
        return GT_FAIL;

    return GT_OK;
}
#endif /* XCAT_DRV */




/*******************************************************************************/
/*******************************************************************************/
/*            MSYS_3.4.69 implementation of Dragonite driver                   */
/*******************************************************************************/
/*******************************************************************************/
#if(defined _linux)

static GT_32 extDrvDrgFd = -1;

/**
* @internal extDrvDragoniteDevInit function
* @endinternal
*
* @brief   Config and enable dragonite sub-system. CPU still in reset
*/
GT_STATUS extDrvDragoniteDevInit
(
    GT_VOID
)
{   
    extDrvDrgFd = open("/dev/dragonite", O_RDWR);
    if (extDrvDrgFd < 0)
    {
        fprintf(stderr, "failed to open /dev/dragonite: %s\n", strerror(errno));
        return GT_FAIL;
    }

    return GT_OK;
}


/**
* @internal extDrvDragoniteUnresetSet function
* @endinternal
*
* @brief   Reset/Unreset Dragonite.
*
* @param[in] unresetEnable            - GT_TRUE - set to un-reset state
*                                      GT_FALSE - set to reset state
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
GT_STATUS extDrvDragoniteUnresetSet
(
    GT_BOOL unresetEnable
)
{   
    GT_BOOL msg;

    if(extDrvDrgFd < 0)
    {
        fprintf(stderr, "Illegal FD - please init Dragonite driver\n");
        return GT_NOT_INITIALIZED;
    }

    /* set Dragonite to reset state */
    msg = unresetEnable;
    if(ioctl(extDrvDrgFd, DRAGONITE_IOC_UNRESET, &msg) < 0)
    {
        fprintf(stderr, "Error: Failed to perform UNRESET command: %s\n", strerror(errno));
        return GT_FAIL;
    }

    return GT_OK;
}


/**
* @internal extDrvDragoniteMemoryTypeSet function
* @endinternal
*
* @brief   Reset/Unreset Dragonite.
*
* @param[in] memType                  - 0 - ITCM
*                                      1 - DTCM
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
GT_STATUS extDrvDragoniteMemoryTypeSet
(
    GT_U32 memType
)
{
    GT_BOOL msg;

    if(extDrvDrgFd < 0)
    {
        fprintf(stderr, "Illegal FD - please init Dragonite driver\n");
        return GT_NOT_INITIALIZED;
    }

    if((memType != ITCM_DIR) && (memType != DTCM_DIR))
    {
        return GT_BAD_PARAM;
    }

    /* Set memory type */
    msg = memType;
    if(ioctl(extDrvDrgFd, DRAGONITE_IOC_SETMEM_TYPE, &msg) < 0)
    {
        fprintf(stderr, "Error: Failed to perform SETMEM_TYPE command: %s\n", strerror(errno));
        return GT_FAIL;
    }

    return GT_OK;
}


/**
* @internal extDrvDragoniteFwDownload function
* @endinternal
*
* @brief   Download FW to instruction shared memory
*/
GT_STATUS extDrvDragoniteFwDownload
(
    IN GT_VOID  *buf,
    GT_U32      size
)
{
    GT_BOOL msg;
    ssize_t num;

    if(extDrvDrgFd < 0)
    {
        fprintf(stderr, "Illegal FD - please init Dragonite driver\n");
        return GT_NOT_INITIALIZED;
    }
    if(buf == NULL)
    {
        return GT_BAD_PTR;
    }

    /* set Dragonite to reset state */
    msg = GT_FALSE;
    if(ioctl(extDrvDrgFd, DRAGONITE_IOC_UNRESET, &msg) < 0)
    {
        fprintf(stderr, "Error: Failed to perform UNRESET command: %s\n", strerror(errno));
        return GT_FAIL;
    }

    /* Set ITCM mem */
    msg = ITCM_DIR;
    if(ioctl(extDrvDrgFd, DRAGONITE_IOC_SETMEM_TYPE, &msg) < 0)
    {
        fprintf(stderr, "Error: Failed to perform SETMEM_TYPE command: %s\n", strerror(errno));
        return GT_FAIL;
    }
        
    if(lseek(extDrvDrgFd, DRAGONITE_FW_LOAD_ADDR_CNS, SEEK_SET) < 0)
    {
        fprintf(stderr, "Error: Failed to perform lseek file command: %s\n", strerror(errno));
        return GT_FAIL;
    }

    num = write(extDrvDrgFd, buf, (size_t)size);
    if (num < 0)
    {
        fprintf(stderr, "Error: Failed to perform file write command: %s\n", strerror(errno));
        return GT_FAIL;
    }

    return GT_OK;
}

/**
* @internal extDrvDragoniteRegisterRead function
* @endinternal
*
* @brief   Dragonite register read.
*
* @param[in] addr                     - register address to read from.
*
* @param[out] valuePtr                 - (pointer to) returned value
*                                       GT_OK if successful, or
*
* @retval GT_BAD_PTR               - on NULL pointer
*                                       GT_FAIL otherwise.
*/
GT_STATUS extDrvDragoniteRegisterRead
(
    IN  GT_U32 addr,
    OUT GT_U32 *valuePtr
)
{
    GT_U32 regValue;

    if(valuePtr == NULL)
    {
        return GT_BAD_PTR;
    }

    if(extDrvDrgFd < 0)
    {
        fprintf(stderr, "Illegal FD - please init Dragonite driver\n");
        return GT_NOT_INITIALIZED;
    }

    if(lseek(extDrvDrgFd, addr, SEEK_SET) < 0)
    {
        fprintf(stderr, "Error: Failed to perform lseek file command: %s\n", strerror(errno));
        return GT_FAIL;
    }

    if(read(extDrvDrgFd, &regValue, sizeof(valuePtr) ) < 0)
    {
        fprintf(stderr, "Error: Failed to perform read file command: %s\n", strerror(errno));
        return GT_FAIL;
    }

    *valuePtr = regValue;

    return GT_OK;
}

/**
* @internal extDrvDragoniteRegisterWrite function
* @endinternal
*
* @brief   Dragonite register write.
*
* @param[in] addr                     - register address to write.
*                                      value - register value to write.
*                                       GT_OK if successful, or
*
* @retval GT_BAD_PTR               - on NULL pointer
*                                       GT_FAIL otherwise.
*/
GT_STATUS extDrvDragoniteRegisterWrite
(
    IN  GT_U32 addr,
    IN  GT_U32 regValue
)
{   
    if(extDrvDrgFd < 0)
    {
        fprintf(stderr, "Illegal FD - please init Dragonite driver\n");
        return GT_NOT_INITIALIZED;
    }

    if(lseek(extDrvDrgFd, addr, SEEK_SET) < 0)
    {
        fprintf(stderr, "Error: Failed to perform lseek file command: %s\n", strerror(errno));
        return GT_FAIL;
    }


    if(write(extDrvDrgFd, &regValue, sizeof(regValue)) < 0)
    {
        fprintf(stderr, "Error: Failed to perform read file command: %s\n", strerror(errno));
        return GT_FAIL;
    }

    return GT_OK;
}

/**
* @internal extDrvDragoniteMsgWrite function
* @endinternal
*
* @brief   Dragonite message write.
*
* @param[in] msg[DRAGONITE_DATA_MSG_LEN] - buffer to write.
*                                       GT_OK if successful, or
*
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_READY             - ownership problem
* @retval GT_NOT_INITIALIZED       - driver not initialized
*                                       GT_FAIL otherwise.
*/
GT_STATUS extDrvDragoniteMsgWrite
(
    IN GT_U8 msg[DRAGONITE_DATA_MSG_LEN]
)
{
    GT_STATUS   rc;                         /* return code */
    GT_U32      regVal;                     /* register value */
    GT_32       i;                          /* loop iterator */
    GT_U32      buf[DRAGONITE_DATA_MSG_LEN];/* Dragonite buffer */
    GT_U32      addr;                       /* register address */

    if(extDrvDrgFd < 0)
    {
        fprintf(stderr, "Illegal FD - please init Dragonite driver\n");
        return GT_NOT_INITIALIZED;
    }

    rc = extDrvDragoniteMemoryTypeSet(DTCM_DIR);
    if (rc != GT_OK)
    {
        fprintf(stderr, "extDrvDragoniteMsgWrite: can't set DTCM memory type\r\n");
        return rc;
    }

    addr = TX_MO_ADDR;
    rc = extDrvDragoniteRegisterRead(addr, &regVal); 
    if (rc != GT_OK)
    {
        fprintf(stderr, "extDrvDragoniteMsgWrite: can't read 0x%x address\n", addr);
        return rc;
    }

    if (regVal != TX_MO_HOST_OWNERSHIP_CODE)
    {
        fprintf(stderr, "extDrvDragoniteMsgWrite: error - current ownership belongs to POE, regValue = 0x%x\n", regVal);
        return GT_NOT_READY;
    }
  
    if (lseek(extDrvDrgFd, TX_BUF_ADDR, SEEK_SET) < 0)
    {
        fprintf(stderr, "extDrvDragoniteMsgWrite: Failed to perform lseek file command: %s\n", strerror(errno));
        return GT_FAIL;
    }

    /* Converting from 'GT_U8 buf' to dragonite memory format */
    for(i = 0; i < DRAGONITE_DATA_MSG_LEN; i++)
    {
        buf[i] = msg[i];
    }

    /* write buffer */
    if (write(extDrvDrgFd, &buf, sizeof(buf)) < 0)
    {
        fprintf(stderr, "extDrvDragoniteMsgWrite: Failed to perform write file command: %s\n", strerror(errno));
        return GT_FAIL;
    }

    /* switch to POE ownership */
    return extDrvDragoniteRegisterWrite(addr, TX_MO_POE_OWNERSHIP_CODE);
}


/**
* @internal extDrvDragoniteMsgRead function
* @endinternal
*
* @brief   Dragonite message write.
*
* @param[in] msg[DRAGONITE_DATA_MSG_LEN] - buffer to write.
*                                       GT_OK if successful, or
*
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_READY             - ownership problem
* @retval GT_NOT_INITIALIZED       - driver not initialized
*                                       GT_FAIL otherwise.
*/
GT_STATUS extDrvDragoniteMsgRead
(
    OUT GT_U8 msg[DRAGONITE_DATA_MSG_LEN]
)
{
    GT_STATUS   rc;                         /* return code */
    GT_U32      regVal;                     /* register value */
    GT_32       i;                          /* loop iterator */
    GT_U32      buf[DRAGONITE_DATA_MSG_LEN];/* Dragonite buffer */

    if(extDrvDrgFd < 0)
    {
        fprintf(stderr, "Illegal FD - please init Dragonite driver\n");
        return GT_NOT_INITIALIZED;
    }

    rc = extDrvDragoniteMemoryTypeSet(DTCM_DIR);
    if (rc != GT_OK)
    {
        fprintf(stderr, "extDrvDragoniteMsgRead: can't set DTCM memory type\r\n");
        return rc;
    }

    rc = extDrvDragoniteRegisterRead(RX_MO_ADDR, &regVal); 
    if (rc != GT_OK)
    {
        fprintf(stderr, "extDrvDragoniteMsgRead: can't read 0x%x address\n", TX_MO_ADDR);
        return rc;
    }

    if (regVal != RX_MO_HOST_OWNERSHIP_CODE)
    {
        fprintf(stderr, "extDrvDragoniteMsgRead: error - current ownership belongs to POE, regVal = 0x%x\n", regVal);
        return GT_NOT_READY;
    }
  
    if (lseek(extDrvDrgFd, RX_BUF_ADDR, SEEK_SET) < 0)
    {
        fprintf(stderr, "extDrvDragoniteMsgRead: Failed to perform lseek file command: %s\n", strerror(errno));
        return GT_FAIL;
    }

    if (read(extDrvDrgFd, buf, sizeof(buf)) < 0)
    {
        fprintf(stderr, "extDrvDragoniteMsgRead: Failed to perform read file command: %s\n", strerror(errno));
        return GT_FAIL;
    }
    
    /* Converting from dragonite memory format - to GT_U8 buf */
    for(i = 0; i < DRAGONITE_DATA_MSG_LEN; i++)
    {
        msg[i] = buf[i];
    } 

    return extDrvDragoniteRegisterWrite(RX_MO_ADDR, RX_MO_POE_OWNERSHIP_CODE);
}


#endif /* _linux */



