/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file prvNoKmDrv.h
*
* @brief implementation header
*
* @version   1
********************************************************************************
*/
#ifndef __prvNoKmDrv_h__
#define __prvNoKmDrv_h__

#include <gtExtDrv/drivers/gtPciDrv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <endian.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

/* #define NOKMDRV_DEBUG */
/* #define NOKMDRV_INFO */
#ifdef NOKMDRV_INFO
# define NOKMDRV_IPRINTF(_params) printf _params
#else
# define NOKMDRV_IPRINTF(_params)
#endif

#define MV_MBUS_DRV_IRQ_POLLING_CNS 0xfffffffe

#define SHMEM_PP_MAPPINGS_FILENAME "/dev/shm/cpss_pp_mappings"

#ifndef SYSTEM_PAGE_SHIFT
#define SYSTEM_PAGE_SHIFT 12
#endif

/* heplful macros */
#ifndef _4K
#define _4K     0x00001000
#define _64K    0x00010000
#define _256K   0x00040000
#define _512K   0x00080000
#define _1M     0x00100000
#define _2M     0x00200000
#define _4M     0x00400000
#define _8M     0x00800000
#define _16M    0x01000000
#define _64M    0x04000000
#if __WORDSIZE == 64
#define _2G     0x80000000
#define _4G     0x100000000L
#endif
#endif
#define SIZE_TO_BAR_REG(_size) ((((_size)>>16)-1)<<16)
#define BITS(_data,_start,_end) (((_data)>>(_end)) & ((1 << ((_start)-(_end)+1))-1))

#define DEVID_TO_FAMILY(dev) ((dev >> 16) & 0xff00)

/* to simplify code bus,dev,func*/
#define DBDF pciDomain, pciBus, pciDev, pciFunc
#define DBDF_DECL \
    IN  GT_U32  pciDomain, \
    IN  GT_U32  pciBus, \
    IN  GT_U32  pciDev, \
    IN  GT_U32  pciFunc


#define NOKM_DEVICES_MAX 32
extern CPSS_HW_INFO_STC noKmMappingsList[NOKM_DEVICES_MAX];
extern GT_U32 noKmMappingsNum;
#ifdef SHARED_MEMORY
extern GT_UINTPTR prvNoKmDrv_resource_virt_addr;
#endif

typedef enum {
    PRV_NOKM_DEVFAMILY_UNKNOWN_E,
    PRV_NOKM_DEVFAMILY_BOBCAT2_E,
    PRV_NOKM_DEVFAMILY_AC3_E,
    PRV_NOKM_DEVFAMILY_BOBK_E,
    PRV_NOKM_DEVFAMILY_ALDRIN_E,
    PRV_NOKM_DEVFAMILY_BOBCAT3_E,
    PRV_NOKM_DEVFAMILY_PIPE_E,
    PRV_NOKM_DEVFAMILY_ALDRIN2_E

} PRV_NOKM_DEVFAMILY_ENT;

extern GT_U16 prvNoKmDevId;

/* dma related stuff */
extern void*  prvExtDrvDmaPtr;
extern GT_UINTPTR prvExtDrvDmaPhys;
extern uint64_t prvExtDrvDmaPhys64;
extern GT_U32 prvExtDrvDmaLen;
extern off_t mvDmaDrvOffset;
GT_BOOL prvExtDrvMvIntDrvConnected(void);
GT_STATUS prvExtDrvMvIntDrvEnableMsi(
    DBDF_DECL
);

/* TWSI function */
typedef enum {
    PRV_EXT_DRV_I2C_OFFSET_TYPE_NONE_E,
    PRV_EXT_DRV_I2C_OFFSET_TYPE_8_E,
    PRV_EXT_DRV_I2C_OFFSET_TYPE_16_E
} PRV_EXT_DRV_I2C_OFFSET_TYPE_ENT;

/**
* @internal prvExtDrvHwIfTwsiReadByteWithOffset function
* @endinternal
*
* @brief   TODO
*
* @param[in] slave_address            - the target device slave address on I2C bus
* @param[in] buffer                   - buffer length
* @param[in] offset                   - None, 8_bit or 16_bit
* @param[in] offset                   - internal  to read from
*
* @param[out] buffer                   - received buffer
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvExtDrvHwIfTwsiReadByteWithOffset(
  IN  GT_U8    slave_address,
  IN  GT_U32   buffer_size,
  IN  PRV_EXT_DRV_I2C_OFFSET_TYPE_ENT offset_type,
  IN  GT_U32   offset,
  OUT  GT_U8  *buffer
);


/***************************************************/
/*   register read/write definitions               */
/***************************************************/
GT_U32 prvNoKm_reg_read(GT_UINTPTR regsBase, GT_U32 regAddr);
void prvNoKm_reg_write(GT_UINTPTR regsBase, GT_U32 regAddr, GT_U32 value);
void prvNoKm_reg_write_field(GT_UINTPTR regsBase, GT_U32 regAddr, GT_U32 mask, GT_U32 value);

/*******************************************************************************
* prvNoKmDrv_configure_dma
*   Configure DMA for PP
*
*******************************************************************************/
GT_STATUS prvNoKmDrv_configure_dma(
    IN GT_UINTPTR   regsBase,
    IN int          targetIsMbus
);

/*******************************************************************************
* prvNoKmDrv_configure_dma_per_devNum
*   Configure DMA for PP , per device using it's 'cpssDriver' to write the
*   registers needed in Falcon that the cpssDriver is complex
*
*******************************************************************************/
GT_STATUS prvNoKmDrv_configure_dma_per_devNum(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId
);

/******************************/
/******************************/
/***  sysfs definitions     ***/
/******************************/
/******************************/





/**
* @internal prvExtDrvPciSysfsConfigure function
* @endinternal
*
* @brief   This routine maps all PP resources userspace and detects IRQ
*
* @param[out] hwInfoPtr                - pointer to HW info
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/
GT_STATUS prvExtDrvPciSysfsConfigure
(
    DBDF_DECL,
    IN  GT_U32                  flags,
    OUT CPSS_HW_INFO_STC       *hwInfoPtr
);


/**
* @internal prvExtDrvPciSysfsConfigWriteReg function
* @endinternal
*
* @brief   This routine write register to the PCI configuration space.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/
GT_STATUS prvExtDrvPciSysfsConfigWriteReg
(
    DBDF_DECL,
    IN  GT_U32  regAddr,
    IN  GT_U32  data
);

/**
* @internal prvExtDrvPciSysfsConfigReadReg function
* @endinternal
*
* @brief   This routine read register from the PCI configuration space.
*
* @param[out] data                     - the read data.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/
GT_STATUS prvExtDrvPciSysfsConfigReadReg
(
    DBDF_DECL,
    IN  GT_U32  regAddr,
    OUT GT_U32  *data
);

/**
* @internal prvExtDrvSysfsInitDevices function
* @endinternal
*
* @brief   Scan for pci devices and initialize them
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*
* @note Linux only
*
*/
GT_STATUS prvExtDrvSysfsInitDevices(void);

/**
* @internal prvExtDrvSysfsFindDev function
* @endinternal
*
* @brief   This routine returns the next instance of the given device (defined by
*         vendorId & devId).
* @param[in] vendorId                 - The device vendor Id.
* @param[in] devId                    - The device Id.
* @param[in] instance                 - The requested device instance.
*
* @param[out] busNo                    - PCI bus number.
* @param[out] devSel                   - the device devSel.
* @param[out] funcNo                   - function number.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/
GT_STATUS prvExtDrvSysfsFindDev
(
    IN  GT_U16  vendorId,
    IN  GT_U16  devId,
    IN  GT_U32  instance,
    OUT GT_U32  *busNo,
    OUT GT_U32  *devSel,
    OUT GT_U32  *funcNo
);
#ifdef SHARED_MEMORY
void prvNoKmSysfsRemap(
    const char *fname,
    void *vaddr,
    void *vsize,
    void *offset
);
#endif

/**
* @internal prvNoKmSysfsUnmap function
* @endinternal
*
* @brief   This routine deletes the mappings for the specified address range.
* @param[in] addr                 - start address.
* @param[in] length               - address region size.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/
GT_STATUS prvNoKmSysfsUnmap(
   IN void *addr,
   IN size_t length
);

/**
* @internal prvExtDrvSysfsConfigDev function
* @endinternal
*
* @brief   This routine enables the PCI device.
*
* @param[in] pciBus                - PCI bus number.
* @param[in] pciDev                - the device devSel.
* @param[in] pciFunc               - function number.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/
GT_STATUS prvExtDrvSysfsConfigDev
(
    IN GT_U32  pciBus,
    IN GT_U32  pciDev,
    IN GT_U32  pciFunc
);






/******************************/
/******************************/
/***  MBUS definitions      ***/
/******************************/
/******************************/
#define IS_MBUS_ADDR(_b,_d,_f) (((_b)==0xff) && ((_d)==0xff) && ((_f)==0xff))

/**
* @internal extDrvPciMbusConfigReadReg function
* @endinternal
*
* @brief   This routine read register from the PCI configuration space.
*
* @param[in] regAddr                  - Register offset in the configuration space.
*
* @param[out] data                     - the read data.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/
GT_STATUS extDrvPciMbusConfigReadReg
(
    IN  GT_U32  regAddr,
    OUT GT_U32  *data
);

/**
* @internal prvExtDrvMbusConfigure function
* @endinternal
*
* @brief   This routine maps all PP resources userspace and detects IRQ
*
* @param[out] hwInfoPtr                - pointer to HW info
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/
GT_STATUS prvExtDrvMbusConfigure
(
    IN  GT_U32              flags,
    OUT CPSS_HW_INFO_STC   *hwInfoPtr
);

void prvNoKmMbusRemap(
    void *vaddr,
    void *vsize,
    int res
);

/**
* @internal prvExtDrvIsMbusDevExists function
* @endinternal
*
* @brief   This routine returns GT_TRUE if MBus device found
*
* @param[in] devId                    - The device Id.
*                                       GT_TRUE, GT_FALSE
*/
GT_BOOL prvExtDrvIsMbusDevExists
(
    IN  GT_U16  devId
);


/**
* @internal mbus_map_resource function
* @endinternal
*
* @brief   Map an mbus resource to application memory space.
*
* @param[in] mbusResource  - resource id (defined in "drivers/mvResources.h")
* @param[in] maxSize       - max size of mapping.
*
* @param[out] mappingPtr   - Pointer to resource HW info structure
*                            with base virtual address and size.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/
GT_STATUS mbus_map_resource(
    IN  int                         mbusResource,
    IN  GT_UINTPTR                  maxSize,
    OUT CPSS_HW_INFO_RESOURCE_MAPPING_STC *mappingPtr
);
#endif /* __prvNoKmDrv_h__ */


