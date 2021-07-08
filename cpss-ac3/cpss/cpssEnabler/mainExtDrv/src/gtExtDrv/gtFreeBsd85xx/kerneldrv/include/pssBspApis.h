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
* @file pssBspApis.h
*
* @brief Enable managment of cache memory
*
* @version   2
********************************************************************************
*/

#ifndef __pssBspApisH
#define __pssBspApisH

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stddef.h> /* for size_t */

/* Defines */

#undef  IN
#define IN
#undef  OUT
#define OUT
#undef  INOUT
#define INOUT

/* In c++ compiler typedef void is not allowed therefore define is used */
#define GT_VOID void

typedef char GT_8;
typedef short GT_16;
typedef long GT_32;
typedef unsigned char GT_U8, *GT_U8_PTR;
typedef unsigned short GT_U16;
typedef unsigned long GT_U32;
typedef enum {GT_FALSE = 0, GT_TRUE = 1} GT_BOOL;
typedef unsigned int GT_STATUS;

typedef int 	(*GT_FUNCPTR) (void);	  /* ptr to function returning int   */
typedef void 	(*GT_VOIDFUNCPTR) (void *); /* ptr to function returning void  */
typedef double 	(*GT_DBLFUNCPTR) (void);  /* ptr to function returning double*/
typedef float 	(*GT_FLTFUNCPTR) (void);  /* ptr to function returning float */

#define GT_OK               (0x00)  /* Operation succeeded                   */

/**
* @enum bspCacheType_ENT
 *
 * @brief This type defines used cache types
*/
typedef enum{

    /** cache of commands */
    bspCacheType_InstructionCache_E,

    /** @brief cache of data
     *  Note:
     *  The enum has to be compatible with GT_MGMT_CACHE_TYPE_ENT.
     */
    bspCacheType_DataCache_E

} bspCacheType_ENT;

/*
 * Description: Enumeration For PCI interrupt lines.
 *
 * Enumerations:
 *      bspPciInt_PCI_INT_A_E - PCI INT# A
 *      bspPciInt_PCI_INT_B_ - PCI INT# B
 *      bspPciInt_PCI_INT_C - PCI INT# C
 *      bspPciInt_PCI_INT_D - PCI INT# D
 *
 * Assumption:
 *      This enum should be identical to bspPciInt_PCI_INT.
 */
typedef enum
{
    bspPciInt_PCI_INT_A = 1,
    bspPciInt_PCI_INT_B,
    bspPciInt_PCI_INT_C,
    bspPciInt_PCI_INT_D
} bspPciInt_PCI_INT;

/**
* @enum bspSmiAccessMode_ENT
 *
 * @brief PP SMI access mode.
*/
typedef enum{

    /** direct access mode (single/parallel) */
    bspSmiAccessMode_Direct_E,

    /** @brief indirect access mode
     *  Note:
     *  The enum has to be compatible with GT_MGMT_CACHE_TYPE_ENT.
     */
    bspSmiAccessMode_inDirect_E

} bspSmiAccessMode_ENT;


/*******************************************************************************
* BSP_RX_CALLBACK_FUNCPTR
*
* DESCRIPTION:
*       The prototype of the routine to be called after a packet was received
*
* INPUTS:
*       segmentList     - A list of pointers to the packets segments.
*       segmentLen      - A list of segment length.
*       numOfSegments   - The number of segment in segment list.
*       queueNum        - the received queue number
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_TRUE if it has handled the input packet and no further action should 
*               be taken with it, or
*       GT_FALSE if it has not handled the input packet and normal processing.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
typedef GT_STATUS (*BSP_RX_CALLBACK_FUNCPTR)
(
    IN GT_U8_PTR   segmentList[],
    IN GT_U32      segmentLen[],   
    IN GT_U32      numOfSegments,
    IN GT_U32      queueNum
);

/*******************************************************************************
* BSP_TX_COMPLETE_CALLBACK_FUNCPTR
*
* DESCRIPTION:
*       The prototype of the routine to be called after a packet was received
*
* INPUTS:
*       segmentList     - A list of pointers to the packets segments.
*       numOfSegments   - The number of segment in segment list.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_TRUE if it has handled the input packet and no further action should 
*               be taken with it, or
*       GT_FALSE if it has not handled the input packet and normal processing.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
typedef GT_STATUS (*BSP_TX_COMPLETE_CALLBACK_FUNCPTR)
(
    IN GT_U8_PTR   segmentList[],
    IN GT_U32      numOfSegments
);

/*** reset ***/
/**
* @internal bspResetInit function
* @endinternal
*
* @brief   This routine calls in init to do system init config for reset.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS bspResetInit
(
    GT_VOID
);


/**
* @internal bspReset function
* @endinternal
*
* @brief   This routine calls to reset of CPU.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS bspReset
(
    GT_VOID
);

/*** cache ***/
/**
* @internal bspCacheFlush function
* @endinternal
*
* @brief   Flush to RAM content of cache
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/
GT_STATUS bspCacheFlush
(
    IN bspCacheType_ENT         cacheType, 
    IN void                     *address_PTR, 
    IN size_t                   size
);

/**
* @internal bspCacheInvalidate function
* @endinternal
*
* @brief   Invalidate current content of cache
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/
GT_STATUS bspCacheInvalidate 
(
    IN bspCacheType_ENT         cacheType, 
    IN void                     *address_PTR, 
    IN size_t                   size
);

/*** DMA ***/
/**
* @internal bspDmaWrite function
* @endinternal
*
* @brief   Write a given buffer to the given address using the Dma.
*
* @param[in] address                  - The destination  to write to.
* @param[in] buffer                   - The  to be written.
* @param[in] length                   - Length of buffer in words.
* @param[in] burstLimit               - Number of words to be written on each burst.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*
* @note 1. The given buffer is allways 4 bytes aligned, any further allignment
*       requirements should be handled internally by this function.
*       2. The given buffer may be allocated from an uncached memory space, and
*       it's to the function to handle the cache flushing.
*       3. The Prestera Driver assumes that the implementation of the DMA is
*       blocking, otherwise the Driver functionality might be damaged.
*
*/
GT_STATUS bspDmaWrite
(
    IN  GT_U32  address,
    IN  GT_U32  *buffer,
    IN  GT_U32  length,
    IN  GT_U32  burstLimit
);

/**
* @internal bspDmaRead function
* @endinternal
*
* @brief   Read a memory block from a given address.
*
* @param[in] address                  - The  to read from.
* @param[in] length                   - Length of the memory block to read (in words).
* @param[in] burstLimit               - Number of words to be read on each burst.
*
* @param[out] buffer                   - The read data.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*
* @note 1. The given buffer is allways 4 bytes aligned, any further allignment
*       requirements should be handled internally by this function.
*       2. The given buffer may be allocated from an uncached memory space, and
*       it's to the function to handle the cache flushing.
*       3. The Prestera Driver assumes that the implementation of the DMA is
*       blocking, otherwise the Driver functionality might be damaged.
*
*/
GT_STATUS bspDmaRead
(
    IN  GT_U32  address,
    IN  GT_U32  length,
    IN  GT_U32  burstLimit,
    OUT GT_U32  *buffer
);

/*******************************************************************************
* bspCacheDmaMalloc
*
* DESCRIPTION:
*       Allocate a cache free area for DMA devices.
*
* INPUTS:
*       size_t bytes - number of bytes to allocate
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       pointer to allocated data per success
*       NULL - per failure to allocate space
*
* COMMENTS:
*       None
*
*******************************************************************************/
void *bspCacheDmaMalloc
(
    IN size_t bytes
);

/**
* @internal bspCacheDmaFree function
* @endinternal
*
* @brief   free a cache free area back to pool.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS bspCacheDmaFree
(
    IN void *pBuf
);

/*** PCI ***/
/**
* @internal bspPciConfigWriteReg function
* @endinternal
*
* @brief   This routine write register to the PCI configuration space.
*
* @param[in] busNo                    - PCI bus number.
* @param[in] devSel                   - the device devSel.
* @param[in] funcNo                   - function number.
* @param[in] regAddr                  - Register offset in the configuration space.
* @param[in] data                     -  to write.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/
GT_STATUS bspPciConfigWriteReg
(
    IN  GT_U32  busNo,
    IN  GT_U32  devSel,
    IN  GT_U32  funcNo,
    IN  GT_U32  regAddr,
    IN  GT_U32  data
);

/**
* @internal bspPciConfigReadReg function
* @endinternal
*
* @brief   This routine read register from the PCI configuration space.
*
* @param[in] busNo                    - PCI bus number.
* @param[in] devSel                   - the device devSel.
* @param[in] funcNo                   - function number.
* @param[in] regAddr                  - Register offset in the configuration space.
*
* @param[out] data                     - the read data.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/
GT_STATUS bspPciConfigReadReg
(
    IN  GT_U32  busNo,
    IN  GT_U32  devSel,
    IN  GT_U32  funcNo,
    IN  GT_U32  regAddr,
    OUT GT_U32  *data
);

/**
* @internal bspPciFindDev function
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
GT_STATUS bspPciFindDev
(
    IN  GT_U16  vendorId,
    IN  GT_U16  devId,
    IN  GT_U32  instance,
    OUT GT_U32  *busNo,
    OUT GT_U32  *devSel,
    OUT GT_U32  *funcNo
);

/**
* @internal bspPciGetIntVec function
* @endinternal
*
* @brief   This routine return the PCI interrupt vector.
*
* @param[in] pciInt                   - PCI interrupt number.
*
* @param[out] intVec                   - PCI interrupt vector.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS bspPciGetIntVec
(
    IN  bspPciInt_PCI_INT  pciInt,
    OUT void               **intVec
);

/**
* @internal bspPciGetIntMask function
* @endinternal
*
* @brief   This routine return the PCI interrupt vector.
*
* @param[in] pciInt                   - PCI interrupt number.
*
* @param[out] intMask                  - PCI interrupt mask.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - otherwise.
*
* @note PCI interrupt mask should be used for interrupt disable/enable.
*
*/
GT_STATUS bspPciGetIntMask
(
    IN  bspPciInt_PCI_INT  pciInt,
    OUT GT_U32             *intMask
);

/**
* @internal bspPciEnableCombinedAccess function
* @endinternal
*
* @brief   This function enables / disables the Pci writes / reads combining
*         feature.
*         Some system controllers support combining memory writes / reads. When a
*         long burst write / read is required and combining is enabled, the master
*         combines consecutive write / read transactions, if possible, and
*         performs one burst on the Pci instead of two. (see comments)
* @param[in] enWrCombine              - GT_TRUE enables write requests combining.
* @param[in] enRdCombine              - GT_TRUE enables read requests combining.
*
* @retval GT_OK                    - on sucess,
* @retval GT_NOT_SUPPORTED         - if the controller does not support this feature,
* @retval GT_FAIL                  - otherwise.
*
* @note 1. Example for combined write scenario:
*       The controller is required to write a 32-bit data to address 0x8000,
*       while this transaction is still in progress, a request for a write
*       operation to address 0x8004 arrives, in this case the two writes are
*       combined into a single burst of 8-bytes.
*
*/
GT_STATUS bspPciEnableCombinedAccess
(
    IN  GT_BOOL     enWrCombine,
    IN  GT_BOOL     enRdCombine
);


/*** SMI ***/
/**
* @internal bspSmiInitDriver function
* @endinternal
*
* @brief   Init the TWSI interface
*
* @param[out] smiAccessMode            - direct/indirect mode
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS bspSmiInitDriver
(
    bspSmiAccessMode_ENT  *smiAccessMode
);

/**
* @internal bspSmiReadReg function
* @endinternal
*
* @brief   Reads a register from SMI slave.
*
* @param[in] devSlvId                 - Slave Device ID
* @param[in] actSmiAddr               - actual smi addr to use (relevant for SX PPs)
* @param[in] regAddr                  - Register address to read from.
*
* @param[out] valuePtr                 - Data read from register.
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS bspSmiReadReg
(               
    IN  GT_U32  devSlvId,
    IN  GT_U32  actSmiAddr,
    IN  GT_U32  regAddr,
    OUT GT_U32 *valuePtr
);

/**
* @internal bspSmiWriteReg function
* @endinternal
*
* @brief   Writes a register to an SMI slave.
*
* @param[in] devSlvId                 - Slave Device ID
* @param[in] actSmiAddr               - actual smi addr to use (relevant for SX PPs)
* @param[in] regAddr                  - Register address to read from.
* @param[in] value                    - data to be written.
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS bspSmiWriteReg
(
    IN GT_U32 devSlvId,
    IN GT_U32 actSmiAddr,
    IN GT_U32 regAddr,
    IN GT_U32 value
);


/*** TWSI ***/
/**
* @internal bspTwsiInitDriver function
* @endinternal
*
* @brief   Init the TWSI interface
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS bspTwsiInitDriver
(
    GT_VOID
);

/**
* @internal bspTwsiWaitNotBusy function
* @endinternal
*
* @brief   Wait for TWSI interface not BUSY
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS bspTwsiWaitNotBusy
(
    GT_VOID
);

/**
* @internal bspTwsiMasterReadTrans function
* @endinternal
*
* @brief   do TWSI interface Transaction
*
* @param[in] devId                    - I2c slave ID
* @param[in] pData                    - Pointer to array of chars (address / data)
*                                      len   - pData array size (in chars).
*                                      stop  - Indicates if stop bit is needed.
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS bspTwsiMasterReadTrans
(
    IN GT_U8           devId,       /* I2c slave ID                              */ 
    IN GT_U8           *pData,      /* Pointer to array of chars (address / data)*/
    IN GT_U8           len,         /* pData array size (in chars).              */
    IN GT_BOOL         stop         /* Indicates if stop bit is needed in the end  */
);

/**
* @internal bspTwsiMasterWriteTrans function
* @endinternal
*
* @brief   do TWSI interface Transaction
*
* @param[in] devId                    - I2c slave ID
* @param[in] pData                    - Pointer to array of chars (address / data)
*                                      len   - pData array size (in chars).
*                                      stop  - Indicates if stop bit is needed.
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS bspTwsiMasterWriteTrans
(
    IN GT_U8           devId,       /* I2c slave ID                              */ 
    IN GT_U8           *pData,      /* Pointer to array of chars (address / data)*/
    IN GT_U8           len,         /* pData array size (in chars).              */
    IN GT_BOOL         stop         /* Indicates if stop bit is needed in the end  */
);

/*** Ethernet Driver ***/
/**
* @internal bspEthPortRxInit function
* @endinternal
*
* @brief   Init the ethernet port Rx interface
*
* @param[in] rxBufPoolSize            - buffer pool size
* @param[in] rxBufPool_PTR            - the address of the pool
* @param[in] rxBufSize                - the buffer requested size
* @param[in,out] numOfRxBufs_PTR          - number of requested buffers, and actual buffers created
* @param[in] headerOffset             - packet header offset size
* @param[in] rxQNum                   - the number of RX queues
* @param[in] rxQbufPercentage[]       the buffer percentage dispersal for all queues
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
GT_STATUS bspEthPortRxInit
(
    IN GT_U32           rxBufPoolSize,
    IN GT_U8_PTR        rxBufPool_PTR,
    IN GT_U32           rxBufSize,
    INOUT GT_U32        *numOfRxBufs_PTR,
    IN GT_U32           headerOffset,
    IN GT_U32           rxQNum,
    IN GT_U32           rxQbufPercentage[]
);

/**
* @internal bspEthPortTxInit function
* @endinternal
*
* @brief   Init the ethernet port Tx interface
*
* @param[in] numOfTxBufs              - number of requested buffers
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
GT_STATUS bspEthPortTxInit
(
    IN GT_U32           numOfTxBufs
);

/**
* @internal bspEthPortEnable function
* @endinternal
*
* @brief   Enable the ethernet port interface
*/
GT_STATUS bspEthPortEnable
(
    GT_VOID
);

/**
* @internal bspEthPortDisable function
* @endinternal
*
* @brief   Disable the ethernet port interface
*/
GT_STATUS bspEthPortDisable
(
    GT_VOID
);


/**
* @internal bspEthPortTx function
* @endinternal
*
* @brief   This function is called after a TxEnd event has been received, it passes
*         the needed information to the Tapi part.
* @param[in] segmentList[]            - A list of pointers to the packets segments.
* @param[in] segmentLen[]             - A list of segment length.
* @param[in] numOfSegments            - The number of segment in segment list.
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
GT_STATUS bspEthPortTx
(
    IN GT_U8_PTR        segmentList[],
    IN GT_U32           segmentLen[],   
    IN GT_U32           numOfSegments
);

/**
* @internal bspEthInputHookAdd function
* @endinternal
*
* @brief   This bind the user Rx callback
*
* @param[in] userRxFunc               - the user Rx callback function
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
GT_STATUS bspEthInputHookAdd
(
    IN BSP_RX_CALLBACK_FUNCPTR    userRxFunc
);

/**
* @internal bspEthTxCompleteHookAdd function
* @endinternal
*
* @brief   This bind the user Tx complete callback
*
* @param[in] userTxFunc               - the user Tx callback function
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
GT_STATUS bspEthTxCompleteHookAdd
(
    IN BSP_TX_COMPLETE_CALLBACK_FUNCPTR    userTxFunc
);

/**
* @internal bspEthRxPacketFree function
* @endinternal
*
* @brief   This routine frees the received Rx buffer.
*
* @param[in] segmentList[]            - A list of pointers to the packets segments.
* @param[in] numOfSegments            - The number of segment in segment list.
* @param[in] queueNum                 - Receive queue number
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
GT_STATUS bspEthRxPacketFree
(
    IN GT_U8_PTR        segmentList[],
    IN GT_U32           numOfSegments,
    IN GT_U32           queueNum
);

/**
* @internal bspIntConnect function
* @endinternal
*
* @brief   Connect a specified C routine to a specified interrupt vector.
*
* @param[in] vector                   - interrupt  number to attach to
* @param[in] routine                  -  to be called
* @param[in] parameter                -  to be passed to routine
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS bspIntConnect
(
    IN  GT_U32           vector,
    IN  GT_VOIDFUNCPTR   routine,
    IN  GT_U32           parameter
);

/**
* @internal bspIntEnable function
* @endinternal
*
* @brief   Enable corresponding interrupt bits
*
* @param[in] intMask                  - new interrupt bits
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS bspIntEnable
(
    IN GT_U32   intMask
);

/**
* @internal bspIntDisable function
* @endinternal
*
* @brief   Disable corresponding interrupt bits.
*
* @param[in] intMask                  - new interrupt bits
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS bspIntDisable
(
    IN GT_U32   intMask
);



/**
* @internal bspIntLock function
* @endinternal
*
* @brief   Lock interrupts
*/
GT_U32 bspIntLock(GT_VOID);

/**
* @internal bspIntUnlock function
* @endinternal
*
* @brief   Unlock interrupts
*/
GT_U32 bspIntUnlock
(
    IN   GT_U32 key
);

/**
* @internal bspVirt2Phys function
* @endinternal
*
* @brief   Converts virtual address to physical.
*
* @param[in] vAddr                    - virtual address
*                                       physical address on success
*
* @retval 0                        - on error
*/
GT_U32 bspVirt2Phys
(
	IN GT_U32 vAddr
);

/**
* @internal bspPhys2Virt function
* @endinternal
*
* @brief   Converts physical address to virtual.
*
* @param[in] pAddr                    - physical address
*                                       virtual address on success
*
* @retval 0                        - on error
*/
GT_U32 bspPhys2Virt
(
	IN GT_U32 pAddr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __pssBspApisH */


