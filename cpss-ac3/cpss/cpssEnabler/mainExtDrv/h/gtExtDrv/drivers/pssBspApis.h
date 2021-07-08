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
* @brief API's supported by BSP/LSP/FSP
*
* @version   17
********************************************************************************
*/

#ifndef __pssBspApisH
#define __pssBspApisH

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stddef.h> /* for size_t */

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

/**
* @enum bspEthTxMode_ENT
 *
 * @brief MII transmission mode
*/
typedef enum{

    /** execute asynchroniouse packet send */
    bspEthTxMode_asynch_E = 0,

    /** @brief execute synchroniouse packet send
     *  Note:
     */
    bspEthTxMode_synch_E

} bspEthTxMode_ENT;

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

/**
* @struct GT_BUF_INFO
 *
 * @brief This structure contains information describing one of buffers
 * (fragments) they are built Ethernet packet.
*/
typedef struct{

    /** @brief pointer to buffer
     *  bufPhysAddr - physical address of buffer
     */
    GT_U8* bufVirtPtr;

    unsigned long bufPhysAddr;

    /** buffer memory size */
    GT_U32 bufSize;

    /** size of actual data placed in buffer */
    GT_U32 dataSize;

    /** @brief for backword compatibility
     *  Note:
     */
    GT_U32 memHandle;

} GT_BUF_INFO;

/**
* @struct GT_PKT_INFO
 *
 * @brief This structure contains information describing Ethernet packet.
 * The packet can be divided for few buffers (fragments)
*/
typedef struct{

    unsigned long   osInfo;

    GT_BUF_INFO *pFrags;

    /** packet status */
    GT_U32 status;

    /** packet length */
    GT_U16 pktSize;

    /** number of buffers used for this packet */
    GT_U16 numFrags;

    /** for backword compatibility */
    char srcIdx;

    /** @brief is packet IP fragmented
     *  Note:
     */
    GT_U32 fragIP;

} GT_PKT_INFO;

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

/*******************************************************************************
* bspHsuMalloc
*
* DESCRIPTION:
*       Allocate a free area for HSU usage.
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

void *bspHsuMalloc
(
    IN size_t bytes
);

/**
* @internal bspHsuFree function
* @endinternal
*
* @brief   free a hsu area back to pool.
*
* @retval MV_OK                    - on success
* @retval MV_FAIL                  - on error
*/
GT_STATUS bspHsuFree
(
 IN void *pBuf
);

/**
* @internal bspWarmRestart function
* @endinternal
*
* @brief   This routine performs warm restart.
*
* @retval MV_OK                    - on success.
* @retval MV_FAIL                  - otherwise.
*/

GT_STATUS bspWarmRestart
(
    GT_VOID
);

/**
* @internal bspInboundSdmaEnable function
* @endinternal
*
* @brief   This routine enables inbound sdma access.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - otherwise.
*/

GT_STATUS bspInboundSdmaEnable
(
    GT_VOID
);

/**
* @internal bspInboundSdmaDisable function
* @endinternal
*
* @brief   This routine disables inbound sdma access.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - otherwise.
*/

GT_STATUS bspInboundSdmaDisable
(
    GT_VOID
);

/**
* @internal bspInboundSdmaStateGet function
* @endinternal
*
* @brief   This routine gets the state of inbound sdma access.
*
* @retval GT_TRUE                  - if sdma is enabled.
* @retval GT_FALSE                 - otherwise.
*/

GT_BOOL bspInboundSdmaStateGet
(
    GT_VOID
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

/**
* @internal bspPciDoubleRead function
* @endinternal
*
* @brief   This routine will read a 64-bit data from given address
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/
GT_STATUS bspPciDoubleRead(GT_U32 address, GT_U64 *dataPtr);

/**
* @internal bspPciDoubleWrite function
* @endinternal
*
* @brief   This routine will write a 64-bit data to given address
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/
GT_STATUS bspPciDoubleWrite(GT_U32 address, GT_U32 word1, GT_U32 word2);

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

#ifdef MV_LINUX_BSP
/* bspPciGetResourceStart() and bspPciGetResourceLen() are used in
 * Linux kernel module only. The output value is MV_U64 (!= GT_U64)
 */
#if defined(MV_PPC64) || defined(MIPS64)
typedef unsigned long	MV_U64;
#else
typedef unsigned long long	MV_U64;
#endif
/**
* @internal bspPciGetResourceStart function
* @endinternal
*
* @brief   This routine performs pci_resource_start.
*         In MIPS64 and INTEL64 this function must be used instead of reading the bar
*         directly.
* @param[in] busNo                    - PCI bus number.
* @param[in] devSel                   - the device devSel.
* @param[in] funcNo                   - function number.
* @param[in] barNo                    - Bar Number.
*
* @retval MV_OK                    - on success,
* @retval MV_FAIL                  - othersise.
*/

GT_STATUS bspPciGetResourceStart
(
    IN  GT_U32  busNo,
    IN  GT_U32  devSel,
    IN  GT_U32  funcNo,
    IN  GT_U32  barNo,
    OUT MV_U64  *resourceStart
);

/**
* @internal bspPciGetResourceLen function
* @endinternal
*
* @brief   This routine performs pci_resource_len.
*         In MIPS64 and INTEL64 this function must be used instead of reading the bar
*         directly.
* @param[in] busNo                    - PCI bus number.
* @param[in] devSel                   - the device devSel.
* @param[in] funcNo                   - function number.
* @param[in] barNo                    - Bar Number.
*
* @retval MV_OK                    - on success,
* @retval MV_FAIL                  - othersise.
*/
GT_STATUS bspPciGetResourceLen
(
    IN  GT_U32  busNo,
    IN  GT_U32  devSel,
    IN  GT_U32  funcNo,
    IN  GT_U32  barNo,
    OUT MV_U64  *resourceLen
);
#endif /* MV_LINUX_BSP */

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

/**
* @internal bspEthMuxSet function
* @endinternal
*
* @brief   Set bitmap of PP ports. Set direction of packets:
*         cpss, Linux, or Raw.
*/
GT_STATUS bspEthMuxSet(
 IN GT_U32 portNum,
 IN GT_U32 portType
);

/**
* @internal bspEthMuxGet function
* @endinternal
*
* @brief   Get the mux mosde of the port
*
* @param[in] portNum                  - The port number for the action
*                                      portType  - Address to store portType information
*                                       GT_OK always
*/
GT_STATUS bspEthMuxGet
(
 IN GT_U32 portNum,
 OUT GT_U32 *portTypeP
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
* @internal bspEthInit function
* @endinternal
*
* @brief   Init the ethernet HW and HAL
*
* @param[in] port                     - eth  number
*                                       MV_OK if successful, or
*                                       MV_FAIL otherwise.
*/
GT_VOID bspEthInit
(
    GT_U8 port
);

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
* @internal bspEthPortTxModeSet function
* @endinternal
*
* @brief   Set the ethernet port tx mode
*/
GT_STATUS bspEthPortTxModeSet
(
    bspEthTxMode_ENT    txMode
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
* @internal bspEthPortTxQueue function
* @endinternal
*
* @brief   This function is called after a TxEnd event has been received, it passes
*         the needed information to the Tapi part.
* @param[in] segmentList[]            - A list of pointers to the packets segments.
* @param[in] segmentLen[]             - A list of segment length.
* @param[in] numOfSegments            - The number of segment in segment list.
* @param[in] txQueue                  - The TX queue.
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
GT_STATUS bspEthPortTxQueue
(
    IN GT_U8_PTR        segmentList[],
    IN GT_U32           segmentLen[],
    IN GT_U32           numOfSegments,
    IN GT_U32           txQueue
);

/*******************************************************************************
* bspEthPortRx
*
* DESCRIPTION:
*       This function is called when a packet has received.
*
* INPUTS:
*       rxQueue         - RX Queue.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       MV_OK if successful, or
*       MV_FAIL otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_PKT_INFO* bspEthPortRx
(
    IN GT_U32           rxQueue
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
    IN GT_U8_PTR    segmentList[],
    IN GT_U32       numOfSegments,
    IN GT_U32       queueNum
);

/**
* @internal bspEthCpuCodeToQueue function
* @endinternal
*
* @brief   Binds DSA CPU code to RX queue.
*
* @param[in] dsaCpuCode               - DSA CPU code
* @param[in] rxQueue                  -  rx queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
* @retval GT_NOT_SUPPORTED         - the API is not supported
*/
GT_STATUS bspEthCpuCodeToQueue
(
    IN GT_U32  dsaCpuCode,
    IN GT_U8   rxQueue
);

/**
* @internal bspEthPrePendTwoBytesHeaderSet function
* @endinternal
*
* @brief   Enables/Disable pre-pending a two-byte header to all packets arriving
*         to the CPU.
* @param[in] enable                   - GT_TRUE  - Two-byte header is pre-pended to packets
*                                      arriving to the CPU.
*                                      GT_FALSE - Two-byte header is not pre-pended to packets
*                                      arriving to the CPU.
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
GT_STATUS bspEthPrePendTwoBytesHeaderSet
(
    IN GT_BOOL enable
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

/*******************************************************************************
* extDrvIntEnable
*
* DESCRIPTION:
*       Enable corresponding interrupt bits
*
* INPUTS:
*       intMask - new interrupt bits
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None
*
*******************************************************************************/
#ifndef bspIntEnable

/* Some functions are defined as macros for performance boost.
   See pssBspApisInline.h in Linux.  */

GT_STATUS bspIntEnable
(
    IN GT_U32   intMask
);
#endif

/*******************************************************************************
* extDrvIntDisable
*
* DESCRIPTION:
*       Disable corresponding interrupt bits.
*
* INPUTS:
*       intMask - new interrupt bits
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None
*
*******************************************************************************/
#ifndef bspIntDisable
GT_STATUS bspIntDisable
(
    IN GT_U32   intMask
);
#endif

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

/*******************************************************************************
* bspVirt2Phys
*
* DESCRIPTION:
*       Converts virtual address to physical.
*
* INPUTS:
*       vAddr - virtual address
*
* OUTPUTS:
*       None
*
* RETURNS:
*       physical address on success
*       0 - on error
*
* COMMENTS:
*
*******************************************************************************/
#ifndef bspVirt2Phys
GT_U32 bspVirt2Phys
(
    IN GT_U32 vAddr
);
#endif

/*******************************************************************************
* bspPhys2Virt
*
* DESCRIPTION:
*       Converts physical address to virtual.
*
* INPUTS:
*       pAddr  - physical address
*
* OUTPUTS:
*       None
*
* RETURNS:
*       virtual address on success
*       0 - on error
*
* COMMENTS:
*
*******************************************************************************/
#ifndef bspPhys2Virt
GT_U32 bspPhys2Virt
(
    IN GT_U32 pAddr
);
#endif

/*** Dragonite ***/
/**
* @internal bspDragoniteSWDownload function
* @endinternal
*
* @brief   Download new version of Dragonite firmware to Dragonite MCU
*
* @param[in] sourcePtr                - Pointer to memory where new version of Dragonite firmware resides.
* @param[in] size                     -  of firmware to download to ITCM.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS bspDragoniteSWDownload
(
    IN  const GT_VOID *sourcePtr,
    IN  GT_U32         size
);

/**
* @internal bspDragoniteEnableSet function
* @endinternal
*
* @brief   Enable/Disable Dragonite module
*
* @param[in] enable                   – GT_TRUE  – Dragonite starts work with parameters set by application
*                                      GT_FALSE – Dragonite stops function
*                                       GT_OK
*
* @note call after SW download
*
*/
GT_STATUS bspDragoniteEnableSet
(
    IN  GT_BOOL enable
);


/**
* @internal bspDragoniteInit function
* @endinternal
*
* @brief   Initialize Dragonite module
*
* @note Application will call this before firmware download
*
*/
GT_STATUS bspDragoniteInit
(
    GT_VOID
);

/**
* @internal bspDragoniteSharedMemWrite function
* @endinternal
*
* @brief   Write a given buffer to the given offset in shared memory of Dragonite
*         microcontroller.
* @param[in] offset                   - Offset from beginning of shared memory
* @param[in] buffer                   - The  to be written.
* @param[in] length                   - Length of buffer in bytes.
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - otherwise.
*
* @note Only DTCM is reachable
*
*/
GT_STATUS bspDragoniteSharedMemWrite
(
    IN  GT_U32         offset,
    IN  const GT_VOID *buffer,
    IN  GT_U32         length
);

/**
* @internal bspDragoniteSharedMemRead function
* @endinternal
*
* @brief   Read a memory block from a given offset in shared memory of Dragonite
*         microcontroller.
* @param[in] offset                   - Offset from beginning of shared memory
* @param[in] length                   - Length of the memory block to read (in bytes).
*
* @param[out] buffer                   - The read data.
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - otherwise.
*
* @note Only DTCM is reachanble
*
*/
GT_STATUS bspDragoniteSharedMemRead
(
    IN  GT_U32   offset,
    OUT GT_VOID *buffer,
    IN  GT_U32   length
);

/**
* @internal bspDragoniteSharedMemoryBaseAddrGet function
* @endinternal
*
* @brief   Get start address of DTCM
*
* @param[out] dtcmPtr                  - Pointer to beginning of DTCM where communication structures
*                                      must be placed
*                                       GT_OK
*/
GT_STATUS bspDragoniteSharedMemoryBaseAddrGet
(
    OUT GT_U32 *dtcmPtr
);

/**
* @internal bspDragoniteGetIntVec function
* @endinternal
*
* @brief   This routine return the Dragonite interrupt vector.
*
* @param[out] intVec                   - Dragonite interrupt vector.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS bspDragoniteGetIntVec
(
    OUT GT_U32 *intVec
);

/**
* @internal bspDragoniteFwCrcCheck function
* @endinternal
*
* @brief   This routine executes Dragonite firmware checksum test
*
* @retval MV_OK                    - on success.
* @retval MV_FAIL                  - otherwise.
*/
GT_STATUS bspDragoniteFwCrcCheck
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __pssBspApisH */


