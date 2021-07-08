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
* @file common/h/cpss/common/cpssHwInit/cpssHwInit.h
*
* @brief Includes CPSS basic Hw initialization functions, and data structures.
*
* @version   1
********************************************************************************
*/
#ifndef __cpssHwInit_h
#define __cpssHwInit_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/common/cpssTypes.h>

/* Define the maximum number of RX queues */
#define CPSS_MAX_RX_QUEUE_CNS     8

/* Define the maximum number of TX queues */
#define CPSS_MAX_TX_QUEUE_CNS     8

/* constant used for field of :
    CPSS_DXCH_PP_PHASE1_INIT_INFO_STC:: hwInfo[ggg].irq[iii] --> Nofity the cpss NOT capture
    the interrupt line (for this device) with it's ISR
*/
#define CPSS_PP_INTERRUPT_VECTOR_NOT_USED_CNS                 0xFFFFFFFF
/* constant used for field of :
    CPSS_DXCH_PP_PHASE1_INIT_INFO_STC:: hwInfo[ggg].irq[iii] --> Nofity the cpss
    will emulate interrupt by polling.
*/
#define CPSS_PP_INTERRUPT_VECTOR_USE_POLLING_CNS              0xFFFFFFFE


/*
 * Typedef enum CPSS_SYS_HA_MODE_ENT
 *
 * Description:
 *      define the High Availability mode of the CPU
 *
 * Enumerations:
 *    CPSS_SYS_HA_MODE_ACTIVE_E    - unit HA mode is active
 *    CPSS_SYS_HA_MODE_STANDBY_E   - unit HA mode is standby
 *
 */
typedef enum
{
    CPSS_SYS_HA_MODE_ACTIVE_E,
    CPSS_SYS_HA_MODE_STANDBY_E
}CPSS_SYS_HA_MODE_ENT;

/**
* @enum CPSS_PP_INTERFACE_CHANNEL_ENT
 *
 * @brief Packet Processor interface channel to the CPU
 * Used to select driver which performs low-level access to
 * Switching registers
*/
typedef enum
{
    CPSS_CHANNEL_PCI_E    = 0x0,
    CPSS_CHANNEL_SMI_E,
    CPSS_CHANNEL_TWSI_E,
    CPSS_CHANNEL_PEX_E,
    CPSS_CHANNEL_PEX_MBUS_E,
    CPSS_CHANNEL_PEX_EAGLE_E,

    CPSS_CHANNEL_PEX_KERNEL_E,
#ifdef DEVELOPER_NEW_DRIVERS
    CPSS_CHANNEL_PEX_NEWDRV_E,
    CPSS_CHANNEL_PEX_MBUS_NEWDRV_E,
    CPSS_CHANNEL_PEX_MBUS_KERNEL_E,
#endif

    CPSS_CHANNEL_LAST_E
}CPSS_PP_INTERFACE_CHANNEL_ENT;

#ifndef DEVELOPER_NEW_DRIVERS
/* EYALO question - to which defines add CPSS_CHANNEL_PEX_EAGLE_E */
#define CPSS_CHANNEL_IS_PCI_COMPATIBLE_MAC(_channel) \
    ((_channel) == CPSS_CHANNEL_PCI_E || (_channel) == CPSS_CHANNEL_PEX_E \
        || (_channel) == CPSS_CHANNEL_PEX_MBUS_E \
        || (_channel) == CPSS_CHANNEL_PEX_KERNEL_E \
        || (_channel) == CPSS_CHANNEL_PEX_EAGLE_E \
    )

#define CPSS_CHANNEL_IS_PCI_PEX_MAC(_channel) \
    ((_channel) == CPSS_CHANNEL_PCI_E || (_channel) == CPSS_CHANNEL_PEX_E \
        || (_channel) == CPSS_CHANNEL_PEX_KERNEL_E \
        || (_channel) == CPSS_CHANNEL_PEX_EAGLE_E \
    )

#define CPSS_CHANNEL_IS_PEX_MAC(_channel) \
    ((_channel) == CPSS_CHANNEL_PEX_E \
        || (_channel) == CPSS_CHANNEL_PEX_KERNEL_E \
        || (_channel) == CPSS_CHANNEL_PEX_EAGLE_E \
    )

#define CPSS_CHANNEL_IS_PEX_MBUS_MAC(_channel) \
    ((_channel) == CPSS_CHANNEL_PEX_MBUS_E )
#else /* DEVELOPER_NEW_DRIVERS */
/* to be enabled later:
 *    CPSS_CHANNEL_PEX_NEWDRV_E         - new PEX driver
 *    CPSS_CHANNEL_PEX_MBUS_NEWDRV_E    - new PEX_MBUS driver
 *    CPSS_CHANNEL_PEX_MBUS_KERNEL_E    - kernel PEX MBUS driver
 */
#define CPSS_CHANNEL_IS_PCI_COMPATIBLE_MAC(_channel) \
    ((_channel) == CPSS_CHANNEL_PCI_E || (_channel) == CPSS_CHANNEL_PEX_E \
        || (_channel) == CPSS_CHANNEL_PEX_MBUS_E \
        || (_channel) == CPSS_CHANNEL_PEX_NEWDRV_E \
        || (_channel) == CPSS_CHANNEL_PEX_MBUS_NEWDRV_E \
        || (_channel) == CPSS_CHANNEL_PEX_KERNEL_E \
        || (_channel) == CPSS_CHANNEL_PEX_MBUS_KERNEL_E
    )

#define CPSS_CHANNEL_IS_PCI_PEX_MAC(_channel) \
    ((_channel) == CPSS_CHANNEL_PCI_E || (_channel) == CPSS_CHANNEL_PEX_E \
        || (_channel) == CPSS_CHANNEL_PEX_NEWDRV_E \
        || (_channel) == CPSS_CHANNEL_PEX_KERNEL_E \
    )

#define CPSS_CHANNEL_IS_PEX_MAC(_channel) \
    ((_channel) == CPSS_CHANNEL_PEX_E \
        || (_channel) == CPSS_CHANNEL_PEX_NEWDRV_E \
        || (_channel) == CPSS_CHANNEL_PEX_KERNEL_E \
    )

#define CPSS_CHANNEL_IS_PEX_MBUS_MAC(_channel) \
    ((_channel) == CPSS_CHANNEL_PEX_MBUS_E \
        || (_channel) == CPSS_CHANNEL_PEX_MBUS_NEWDRV_E \
        || (_channel) == CPSS_CHANNEL_PEX_MBUS_KERNEL_E \
    )
#endif /* DEVELOPER_NEW_DRIVERS */



/**
* @enum CPSS_RX_BUFF_ALLOC_METHOD_ENT
 *
 * @brief Defines the different allocation methods for the Rx / Tx
 * descriptors, and Rx buffers.
*/
typedef enum{

    CPSS_RX_BUFF_DYNAMIC_ALLOC_E = 0,

    CPSS_RX_BUFF_STATIC_ALLOC_E     ,

    /** @brief CPSS doesn't manage the RX buffers allocation;
     *  application must provide buffers for RX queues.
     *  Supported in SDMA CPU port mode and for DXCH devices only.
     */
    CPSS_RX_BUFF_NO_ALLOC_E

} CPSS_RX_BUFF_ALLOC_METHOD_ENT;

/**
* @enum CPSS_TX_BUFF_ALLOC_METHOD_ENT
 *
 * @brief Defines the different allocation methods for the Tx
 * descriptors and Tx buffers.
*/
typedef enum{

    CPSS_TX_BUFF_DYNAMIC_ALLOC_E = 0,

    CPSS_TX_BUFF_STATIC_ALLOC_E

} CPSS_TX_BUFF_ALLOC_METHOD_ENT;


/**
* @enum CPSS_TX_SDMA_QUEUE_MODE_ENT
 *
 * @brief Defines the different working mode of Tx SDMA queue.
*/
typedef enum{

    /** @brief SDMA updates the descriptor ownership
     *  bit to mark CPU ownership.
     */
    CPSS_TX_SDMA_QUEUE_MODE_NORMAL_E,

    /** @brief SDMA will not return the
     *  descriptor ownership to the CPU.
     */
    CPSS_TX_SDMA_QUEUE_MODE_PACKET_GENERATOR_E

} CPSS_TX_SDMA_QUEUE_MODE_ENT;

/*
 * Typedef: CPSS_TX_BUFF_MALLOC_FUNC
 *
 * Description: Prototype of the function pointer to be provided by the
 *              user to perform Rx descriptors & buffers allocation.
 *              The CPSS invokes this user function when allocating
 *              Rx buffers. This is relevant only if  the DYNAMIC_ALLOC
 *              allocation method is used.
 *
 * Fields:
 *      size   - the size  to allocate.
 *      align - The required byte-alignment for the allocation.
 *
 */
typedef GT_U8* (*CPSS_TX_BUFF_MALLOC_FUNC)
(
    IN GT_U32 size,
    IN GT_U32 align
);

/*
 * Typedef: CPSS_RX_BUFF_MALLOC_FUNC
 *
 * Description: Prototype of the function pointer to be provided by the
 *              user to perform Rx buffer allocation.
 *              The CPSS invokes this user function when allocating
 *              Rx buffers. This is relevant only if  the DYNAMIC_ALLOC
 *              allocation method is used.
 *
 * Fields:
 *      size   - the size of the Rx buffer to allocate.  This value reflects
 *               the Rx buffer size that the user defines in the
 *               TAPI gtSysConfig.h structure GT_SYS_CONFIG
 *      align - The required byte-alignment for the Rx buffer
 *
 */
typedef GT_U8* (*CPSS_RX_BUFF_MALLOC_FUNC)
(
    IN GT_U32 size,
    IN GT_U32 align
);


/**
* @struct CPSS_RX_BUF_INFO_STC
 *
 * @brief Rx buffer Information
*/
typedef struct
{
    CPSS_RX_BUFF_ALLOC_METHOD_ENT   allocMethod;
    GT_U32                  bufferPercentage[CPSS_MAX_RX_QUEUE_CNS];
    GT_U32                  rxBufSize;
    GT_U32                  headerOffset;
    GT_BOOL                 buffersInCachedMem;

    union
    {
        struct
        {
            GT_U32  *rxBufBlockPtr;
            GT_U32  rxBufBlockSize;
        }staticAlloc;

        struct
        {
            CPSS_RX_BUFF_MALLOC_FUNC    mallocFunc;
            GT_U32                      numOfRxBuffers;
        }dynamicAlloc;
    }buffData;
}CPSS_RX_BUF_INFO_STC;


/**
* @struct CPSS_NET_IF_CFG_STC
 *
 * @brief Network interface configuration parameters
*/
typedef struct{

    GT_U32 *txDescBlock;

    /** @brief The raw size in bytes of txDescBlock memory.
     *  rxDescBlock   - Pointer to a block memory to be used for
     *  allocating Rx description structures.(Rx descriptor
     *  structures to cpu)
     */
    GT_U32 txDescBlockSize;

    GT_U32 *rxDescBlock;

    /** The raw size in byte of rxDescBlock. */
    GT_U32 rxDescBlockSize;

    /** Rx buffers allocation information. */
    CPSS_RX_BUF_INFO_STC rxBufInfo;

} CPSS_NET_IF_CFG_STC;

/**
* @struct CPSS_DMA_QUEUE_CFG_STC
 *
 * @brief DMA Queue configuration parameters
*/
typedef struct{

    GT_U8 *dmaDescBlock;

    /** Size of dmaDescBlock in bytes. */
    GT_U32 dmaDescBlockSize;

} CPSS_DMA_QUEUE_CFG_STC;


/**
* @struct CPSS_MULTI_NET_IF_RX_SDMA_QUEUE_STC
 *
 * @brief Network Multi-Group interface configuration parameters for
 * Rx SDMA.
*/
typedef struct
{
    CPSS_RX_BUFF_ALLOC_METHOD_ENT   buffAllocMethod;
    GT_U32                          numOfRxDesc;
    GT_U32                          numOfRxBuff;
    GT_U32                          buffSize;
    GT_U32                          buffHeaderOffset;
    GT_BOOL                         buffersInCachedMem;
    GT_U32                          *descMemPtr;
    GT_U32                          descMemSize;

    union
    {
        struct
            {
            GT_U32  *buffMemPtr;
            GT_U32   buffMemSize;
        } staticAlloc;

        struct
        {
            CPSS_RX_BUFF_MALLOC_FUNC buffMallocFunc;
        } dynamicAlloc;
   } memData;
} CPSS_MULTI_NET_IF_RX_SDMA_QUEUE_STC;


/**
* @struct CPSS_MULTI_NET_IF_TX_SDMA_QUEUE_STC
 *
 * @brief Network Multi-Group interface configuration parameters for
 * Tx SDMA.
*/
typedef struct
{
    CPSS_TX_SDMA_QUEUE_MODE_ENT     queueMode;
    CPSS_TX_BUFF_ALLOC_METHOD_ENT   buffAndDescAllocMethod;
    GT_U32                          numOfTxDesc;
    GT_U32                          numOfTxBuff;
    GT_U32                          buffSize;

    union
    {
        struct
            {
            GT_U32  *buffAndDescMemPtr;
            GT_U32   buffAndDescMemSize;
        } staticAlloc;

        struct
        {
            CPSS_TX_BUFF_MALLOC_FUNC buffAndDescMallocFunc;
        } dynamicAlloc;
     }memData;
} CPSS_MULTI_NET_IF_TX_SDMA_QUEUE_STC;


/**
* @struct CPSS_MULTI_NET_IF_CFG_STC
 *
 * @brief Multi-Group Network interface configuration parameters.
 *
 *  the indexes into rxSdmaQueuesConfig[x][y] and txSdmaQueuesConfig[x][y] are
 *      according to 'global queue index' :
 *      x = 'global queue index' / 8
 *      y = 'global queue index' % 8
 *
 *  NOTE: in Lion/Lion2 : x = PRV_CPSS_NETIF_SDMA_PORT_GROUP_ID_MAC(devNum)
*/
typedef struct{

    CPSS_MULTI_NET_IF_RX_SDMA_QUEUE_STC rxSdmaQueuesConfig[CPSS_MAX_SDMA_CPU_PORTS_CNS][CPSS_MAX_RX_QUEUE_CNS];

    CPSS_MULTI_NET_IF_TX_SDMA_QUEUE_STC txSdmaQueuesConfig[CPSS_MAX_SDMA_CPU_PORTS_CNS][CPSS_MAX_TX_QUEUE_CNS];

} CPSS_MULTI_NET_IF_CFG_STC;

/**
* @struct CPSS_AUQ_CFG_STC
 *
 * @brief Address Update Queue configuration parameters
*/
typedef struct{

    GT_U8 *auDescBlock;

    /** Size of auDescBlock (in Bytes). */
    GT_U32 auDescBlockSize;

} CPSS_AUQ_CFG_STC;

/*
 * Typedef enum CPSS_HW_PP_RESET_SKIP_TYPE_ENT
 *
 * Description:
 *      define the skip type parameter to set. setting this parameters are
 *      relevant when setting <SoftResetTrigger> to 1.
 *          CPSS_HW_PP_RESET_SKIP_TYPE_REGISTER_E:Skip the registers
 *                        initialization at soft reset.
 *              Disable = Set: At software reset, registers are set to their
 *                        default value.
 *              Enable  = Not Set: At software reset, registers are not set to
 *                        their default value. Their value remains as it was
 *                        before the software reset.
 *
 *          CPSS_HW_PP_RESET_SKIP_TYPE_TABLE_E:Skip the tables
 *                        initialization at soft reset.
 *              Disable = Set: At software reset, tables are set to their
 *                        default value.
 *              Enable = Not Set: At software reset, tables are not set to
 *                       their default value. Their value remains as it was
 *                       before the software reset.
 *
 *          CPSS_HW_PP_RESET_SKIP_TYPE_EEPROM_E: Skip the EEPROM
 *                        initialization at soft reset.
 *              Disable  = Performed: At software reset, EEPROM initialization
 *                         is performed.
 *              Enable   = Not Performed: At software reset, EEPROM initialization
 *                         is not performed
 *
 *          CPSS_HW_PP_RESET_SKIP_TYPE_PEX_E: Skip the PEX initialization at soft reset.
 *              Disable = Do Not Skip: Reset the PEX on SW Reset.
 *              Enable  = Skip: Do not reset the PEX on SW Reset.
 *
 *          CPSS_HW_PP_RESET_SKIP_TYPE_LINK_LOSS_E: Soft reset done without link loss.
 *              Disable = Do Not Skip: Link Loss on SW Reset.
 *              Enable  = Skip: No Link Loss on SW Reset.
 *
 *          CPSS_HW_PP_RESET_SKIP_TYPE_CHIPLETS_E: Skip the Chiplets initialization at soft reset.
 *              Disable = Do Not Skip: Reset the Chiplets on SW Reset.
 *              Enable  = Skip: Do not reset the Chiplets on SW Reset.
 *              (APPLICABLE DEVICES: Falcon)
 *              This feature cofigured on Main Dies only.
 *
 *          CPSS_HW_PP_RESET_SKIP_TYPE_ALL_E: All options above
 *              Disable = Do Not Skip on SW Reset.
 *              Enable  = Skip on SW Reset.
 *
 *          CPSS_HW_PP_RESET_SKIP_TYPE_ALL_EXCLUDE_PEX_E :
 *              like CPSS_HW_PP_RESET_SKIP_TYPE_ALL_E but exclude CPSS_HW_PP_RESET_SKIP_TYPE_PEX_E
 *              this case is useful when wanting the device to do soft reset
 *              without the need to reconfigure the PEX again.
 *
 */
typedef enum {
    CPSS_HW_PP_RESET_SKIP_TYPE_REGISTER_E = 0,
    CPSS_HW_PP_RESET_SKIP_TYPE_TABLE_E,
    CPSS_HW_PP_RESET_SKIP_TYPE_EEPROM_E,
    CPSS_HW_PP_RESET_SKIP_TYPE_PEX_E,
    CPSS_HW_PP_RESET_SKIP_TYPE_LINK_LOSS_E,
    CPSS_HW_PP_RESET_SKIP_TYPE_CHIPLETS_E,
    CPSS_HW_PP_RESET_SKIP_TYPE_ALL_E,
    CPSS_HW_PP_RESET_SKIP_TYPE_ALL_EXCLUDE_PEX_E
} CPSS_HW_PP_RESET_SKIP_TYPE_ENT;


/*
 * Typedef enum CPSS_HW_PP_INIT_STAGE_ENT
 *
 * Description:
 *      Indicates the initialization stage of the device.
 *
 *      CPSS_HW_PP_INIT_STAGE_INIT_DURING_RESET_E = Initialization is during reset
 *
 *      CPSS_HW_PP_INIT_STAGE_EEPROM_DONE_INT_MEM_DONE_E = Initialize after reset.
 *                   EEPROM initialization is done and Internal memory
 *                   initialization is not done.
 *
 *      CPSS_HW_PP_INIT_STAGE_EEPROM_NOT_DONE_INT_MEM_DONE_E = Initialize after reset.
 *                   Internal memory initialization is done and EEPROM
 *                   initialization is not done.
 *
 *      CPSS_HW_PP_INIT_STAGE_FULLY_FUNC_E = Initialization is fully functional.
 *
 */
typedef enum {
    CPSS_HW_PP_INIT_STAGE_INIT_DURING_RESET_E = 0,
    CPSS_HW_PP_INIT_STAGE_EEPROM_DONE_INT_MEM_DONE_E,
    CPSS_HW_PP_INIT_STAGE_EEPROM_NOT_DONE_INT_MEM_DONE_E,
    CPSS_HW_PP_INIT_STAGE_FULLY_FUNC_E
} CPSS_HW_PP_INIT_STAGE_ENT;


/**
* @internal cpssPpInterruptsDisable function
* @endinternal
*
* @brief   This API disable PP interrupts
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssPpInterruptsDisable
(
    IN GT_U8     devNum
);


/**
* @internal cpssPpConfigDevDataImport function
* @endinternal
*
* @brief   Imports data for device specific information. Used in catch up process
*         during initialization as the first step in the catch up process for
*         configuring the standby CPU.
*         NOTE: for a device that will be initialized in "pp phase 1" with
*         High availability mode of "standby" , this function MUST be called prior
*         to the "pp phase 1"
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number.
* @param[in] configDevDataBufferPtr   - pointer to a pre allocated buffer for holding
*                                      information on devices in the system
* @param[in] configDevDataBufferSize  - size of the pre allocated buffer
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number or configDevDataBufferSize
*/
GT_STATUS cpssPpConfigDevDataImport
(
    IN GT_U8     devNum,
    IN void      *configDevDataBufferPtr,
    IN GT_U32    configDevDataBufferSize
);

/**
* @internal cpssPpConfigDevDataExport function
* @endinternal
*
* @brief   Imports data for device specific information. Used in catch up process
*         during initialization as the first step in the catch up process for
*         configuring the standby CPU.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number.
* @param[in,out] configDevDataBufferPtr   - pointer to a pre allocated buffer for
*                                      holding information on devices in the system
* @param[in,out] configDevDataBufferSizePtr - size of the pre allocated buffer
* @param[in,out] configDevDataBufferPtr   - pointer to a pre allocated buffer for
*                                      holding information on devices in the system
*                                      filled by the function.
* @param[in,out] configDevDataBufferSizePtr - size of the filled data in the pre
*                                      allocated buffer, in case of failure
*                                      (GT_BAD_SIZE), needed size of the block
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_SIZE              - block is not big enough to hold all the data
* @retval GT_BAD_PARAM             - wrong device Number
*/
GT_STATUS cpssPpConfigDevDataExport
(
    IN    GT_U8     devNum,
    INOUT void      *configDevDataBufferPtr,
    INOUT GT_U32    *configDevDataBufferSizePtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssHwInit_h */


