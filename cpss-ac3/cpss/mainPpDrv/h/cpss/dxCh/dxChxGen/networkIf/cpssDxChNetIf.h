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
* @file cpssDxChNetIf.h
*
* @brief Include DxCh network interface API functions
*
*
* @version   36
********************************************************************************
*/
#ifndef __cpssDxChNetIfh
#define __cpssDxChNetIfh

#ifdef __cplusplus
extern "C" {
#endif

#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h>


#if defined CPSS_TRAFFIC_API_LOCK_DISABLE
#define CPSS_TRAFFIC_API_LOCK_MAC(_devNum,_functionality)
#define CPSS_TRAFFIC_API_UNLOCK_MAC(_devNum,_functionality)

#else
#define CPSS_TRAFFIC_API_LOCK_MAC(_devNum,_functionality) CPSS_API_LOCK_MAC(_devNum,_functionality)
#define CPSS_TRAFFIC_API_UNLOCK_MAC(_devNum,_functionality) CPSS_API_UNLOCK_MAC(_devNum,_functionality)

#endif

/**
* @internal cpssDxChNetIfInit function
* @endinternal
*
* @brief   Initialize the network interface SDMA structures, Rx descriptors & buffers
*         and Tx descriptors.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device to initialize.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. The application must call only one of the following APIs per device:
*       cpssDxChNetIfMiiInit - for MII/RGMII Ethernet port networkIf initialization.
*       cpssDxChNetIfInit - for SDMA networkIf initialization.
*       In case more than one of the mentioned above API is called
*       GT_FAIL will be return.
*       2. In case CPSS_RX_BUFF_NO_ALLOC_E is set, the application must enable
*       RX queues after attaching the buffers. See: cpssDxChNetIfSdmaRxQueueEnable.
*
*/
GT_STATUS cpssDxChNetIfInit
(
    IN  GT_U8       devNum
);

/**
* @internal cpssDxChNetIfRemove function
* @endinternal
*
* @brief   This function is called upon Hot removal of a device, inorder to release
*         all Network Interface related structures.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device that was removed.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNetIfRemove
(
    IN  GT_U8   devNum
);

/**
* @internal cpssDxChNetIfPrePendTwoBytesHeaderSet function
* @endinternal
*
* @brief   Enables/Disable pre-pending a two-byte header to all packets forwarded
*         to the CPU (via the CPU port or the PCI interface).
*         This two-byte header is used to align the IPv4 header to 32 bits.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE  - Two-byte header is pre-pended to packets
*                                      forwarded to the CPU.
*                                      GT_FALSE - Two-byte header is not pre-pended to packets
*                                      forward to the CPU.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNetIfPrePendTwoBytesHeaderSet
(
    IN  GT_U8                           devNum,
    IN  GT_BOOL                         enable
);

/**
* @internal cpssDxChNetIfPrePendTwoBytesHeaderGet function
* @endinternal
*
* @brief   Get the Enable/Disable status of pre-pending a two-byte header to all
*         packets forwarded to the CPU (via the CPU port or the PCI interface).
*         This two-byte header is used to align the IPv4 header to 32 bits.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                -  (pointer to) GT_TRUE  - Two-byte header is pre-pended
*                                      to packets forwarded to the CPU.
*                                      GT_FALSE - Two-byte header is not pre-pended to packets
*                                      forward to the CPU.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNetIfPrePendTwoBytesHeaderGet
(
    IN  GT_U8        devNum,
    OUT  GT_BOOL    *enablePtr
);

/**
* @internal cpssDxChNetIfRxBufFree function
* @endinternal
*
* @brief   Frees a list of buffers, that where previously passed to the upper layer
*         in an Rx event.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number through which these buffers where
*                                      received.
* @param[in] rxQueue                  - The Rx queue number through which these buffers where received
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon 0..31).
*                                      (APPLICABLE RANGES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X 0..7).
* @param[in] rxBuffList[]             - List of Rx buffers to be freed.
* @param[in] buffListLen              - Length of rxBufList.
*                                       GT_OK on success, or
*                                       GT_FAIL otherwise.
*
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. This function is not supported in CPSS_RX_BUFF_NO_ALLOC_E buffer
*       allocation method, GT_NOT_SUPPORTED returned.
*
*/
GT_STATUS cpssDxChNetIfRxBufFree
(
    IN GT_U8    devNum,
    IN GT_U8    rxQueue,
    IN GT_U8    *rxBuffList[],
    IN GT_U32   buffListLen
);

/**
* @internal cpssDxChNetIfRxBufFreeWithSize function
* @endinternal
*
* @brief   Frees a list of buffers, that where previously passed to the upper layer
*         in an Rx event.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number through which these buffers where
*                                      received.
* @param[in] rxQueue                  - The Rx queue number through which these buffers where
*                                      received (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon 0..31).
*                                      (APPLICABLE RANGES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X 0..7).
* @param[in] rxBuffList[]             - List of Rx buffers to be freed.
* @param[in] rxBuffSizeList[]         - List of Rx Buffers sizes.
* @param[in] buffListLen              - Length of rxBuffList and rxBuffSize lists.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported buffer allocation method
*
* @note 1. This function supported only in CPSS_RX_BUFF_NO_ALLOC_E buffer
*       allocation method, otherwise GT_NOT_SUPPORTED returned.
*       2. Buffer size must be multiple of 8 bytes and buffer address must be
*       128-byte aligned, otherwise GT_BAD_PARAM returned.
*
*/
GT_STATUS cpssDxChNetIfRxBufFreeWithSize
(
    IN GT_U8    devNum,
    IN GT_U8    rxQueue,
    IN GT_U8    *rxBuffList[],
    IN GT_U32   rxBuffSizeList[],
    IN GT_U32   buffListLen
);

/**
* @internal cpssDxChNetIfSdmaRxPacketGet function
* @endinternal
*
* @brief   This function returns packets from PP destined to the CPU port.
*         -- SDMA relate.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1_Diamond.
*
* @param[in] devNum                   - Device number.
* @param[in] queueIdx                 - The queue from which this packet was received
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon 0..31).
*                                      (APPLICABLE RANGES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X 0..7).
* @param[in,out] numOfBuffPtr             Num of buffs in packetBuffsArrPtr.
* @param[in,out] numOfBuffPtr             - Num of used buffs in packetBuffsArrPtr.
*
* @param[out] packetBuffsArrPtr[]      - (pointer to)The received packet buffers list.
* @param[out] buffLenArr[]             - List of buffer lengths for packetBuffsArrPtr.
* @param[out] rxParamsPtr              - (Pointer to)information parameters of received packets
*
* @retval GT_NO_MORE               - no more packets on the device/queue
* @retval GT_OK                    - packet got with no error.
* @retval GT_FAIL                  - failed to get the packet
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - wrong devNum,portNum,queueIdx.
* @retval GT_NOT_INITIALIZED       - the library was not initialized
* @retval GT_DSA_PARSING_ERROR     - DSA tag parsing error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note It is recommended to call cpssDxChNetIfRxBufFree for this queue
*       i.e. return the buffer to their original queue.
*
*/
GT_STATUS cpssDxChNetIfSdmaRxPacketGet
(
    IN GT_U8                                devNum,
    IN GT_U8                                queueIdx,
    INOUT GT_U32                            *numOfBuffPtr,
    OUT GT_U8                               *packetBuffsArrPtr[],
    OUT GT_U32                              buffLenArr[],
    OUT CPSS_DXCH_NET_RX_PARAMS_STC         *rxParamsPtr
);

/**
* @internal cpssDxChNetIfSdmaRxCountersGet function
* @endinternal
*
* @brief   For each packet processor, get the Rx packet counters from its SDMA
*         packet interface. Return the aggregate counter values for the given
*         traffic class queue. -- SDMA relate
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1_Diamond.
*
* @param[in] devNum                   - device number
* @param[in] queueIdx                 - traffic class queue
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon 0..31).
*                                      (APPLICABLE RANGES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X 0..7).
*
* @param[out] rxCountersPtr            - (pointer to) rx counters on this queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Counters are reset on every read.
*
*/
GT_STATUS cpssDxChNetIfSdmaRxCountersGet
(
    IN    GT_U8      devNum,
    IN    GT_U8      queueIdx,
    OUT   CPSS_DXCH_NET_SDMA_RX_COUNTERS_STC *rxCountersPtr
);

/**
* @internal cpssDxChNetIfSdmaRxErrorCountGet function
* @endinternal
*
* @brief   Returns the total number of Rx resource errors that occurred on a given
*         Rx queue . -- SDMA relate
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1_Diamond.
*
* @param[in] devNum                   - device number
*
* @param[out] rxErrCountPtr            - (pointer to) The total number of Rx resource errors on
*                                      the device for all the queues.
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The counters returned by this function reflects the number of Rx errors
*       that occurred since the last call to this function.
*
*/
GT_STATUS cpssDxChNetIfSdmaRxErrorCountGet
(
    IN GT_U8                                    devNum,
    OUT CPSS_NET_SDMA_RX_ERROR_COUNTERS_STC     *rxErrCountPtr
);

/**
* @internal cpssDxChNetIfSdmaTxPacketSend function
* @endinternal
*
* @brief   This function receives packet buffers & parameters from Application .
*         Prepares them for the transmit operation, and enqueues the prepared
*         descriptors to the PP's tx queues. -- SDMA relate.
*         function activates Tx SDMA , function doesn't wait for event of
*         "Tx buffer queue" from PP
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1_Diamond.
*
* @param[in] devNum                   - Device number.
* @param[in] pcktParamsPtr            The internal packet params to be set into the packet
*                                      descriptors.
* @param[in] buffList[]               - The packet data buffers list.
* @param[in] buffLenList[]            - A list of the buffers len in buffList.
* @param[in] numOfBufs                - Length of buffList.
*
* @retval GT_OK                    - on success, or
* @retval GT_NO_RESOURCE           - if there is not enough free elements in the fifo
*                                       associated with the Event Request Handle.
* @retval GT_EMPTY                 - if there are not enough descriptors to do the sending.
* @retval GT_BAD_PARAM             - on bad DSA params or the data buffer is longer
*                                       than allowed. Buffer data can occupied up to the
*                                       maximum number of descriptors defined.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                       GT_FAIL otherwise.
*
* @note 1. Support both regular DSA tag and extended DSA tag.
*       2. Each buffer must be at least 8 bytes long. First buffer must be at
*       least 24 bytes for tagged packet, 20 for untagged packet.
*       3. If returned status is GT_NO_RESOURCE then the application should free
*       the elements in the fifo (associated with the Handle) by calling
*       cpssDxChNetIfTxBufferQueueGet, and send the packet again.
*       4. If returned status is GT_EMPTY and CPSS handling the events
*       of the device then the application should wait and try to send the
*       packet again.
*
*/
GT_STATUS cpssDxChNetIfSdmaTxPacketSend
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_NET_TX_PARAMS_STC              *pcktParamsPtr,
    IN GT_U8                                    *buffList[],
    IN GT_U32                                   buffLenList[],
    IN GT_U32                                   numOfBufs
);

/*******************************************************************************
* cpssDxChNetIfSdmaSyncTxPacketSend
*
*       This function receives packet buffers & parameters from Applications .
*       Prepares them for the transmit operation, and enqueues the prepared
*       descriptors to the PP's tx queues.
*       After transmition end all transmitted packets descriptors are freed.
*        -- SDMA relate.
*       function activates Tx SDMA , function wait for PP to finish processing
*       the buffers.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*        DxCh1_Diamond.
*
* INPUTS:
*       devNum       - Device number.
*       pcktParamsPtr- The internal packet params to be set into the packet
*                      descriptors.
*       buffList     - The packet data buffers list.
*       buffLenList  - A list of the buffers len in buffList.
*       numOfBufs    - Length of buffList.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK          - on success, or
*       GT_NO_RESOURCE - if there is not enough free elements in the fifo
*                        associated with the Event Request Handle.
*       GT_EMPTY       - if there are not enough descriptors to do the sending.
*       GT_HW_ERROR    - when after transmission last descriptor own bit wasn't
*                        changed for long time.
*       GT_BAD_PARAM   - on bad DSA params or the data buffer is longer
*                        than allowed. Buffer data can occupied up to the
*                        maximum number of descriptors defined.
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_INITIALIZED       - the library was not initialized
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_FAIL otherwise.
*
* COMMENTS:
*    1. Support both regular DSA tag and extended DSA tag.
*    2. Each buffer must be at least 8 bytes long. First buffer must be at
*       least 24 bytes for tagged packet, 20 for untagged packet.
*    3. If returned status is GT_NO_RESOURCE then the application should free
*       the elements in the fifo (associated with the Handle) by calling
*       cpssDxChNetIfTxBufferQueueGet, and send the packet again.
*    4. If returned status is GT_EMPTY and CPSS handling the events
*       of the device then the application should wait and try to send the
*       packet again.
*
*******************************************************************************/
GT_STATUS cpssDxChNetIfSdmaSyncTxPacketSend
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_NET_TX_PARAMS_STC              *pcktParamsPtr,
    IN GT_U8                                    *buffList[],
    IN GT_U32                                   buffLenList[],
    IN GT_U32                                   numOfBufs
);

/**
* @internal cpssDxChNetIfSdmaRxQueueEnable function
* @endinternal
*
* @brief   Enable/Disable the specified traffic class queue for RX
*         on all packet processors in the system. -- SDMA relate
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1_Diamond.
*
* @param[in] devNum                   - device number
* @param[in] queue                    - traffic class queue
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon 0..31).
*                                      (APPLICABLE RANGES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X 0..7).
* @param[in] enable                   - GT_TRUE,  queue
*                                      GT_FALSE, disable queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNetIfSdmaRxQueueEnable
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           queue,
    IN  GT_BOOL                         enable
);

/**
* @internal cpssDxChNetIfSdmaRxQueueEnableGet function
* @endinternal
*
* @brief   Get status of the specified traffic class queue for RX
*         packets in CPU. -- SDMA relate
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1_Diamond.
*
* @param[in] devNum                   - device number
* @param[in] queue                    - traffic class queue
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon 0..31).
*                                      (APPLICABLE RANGES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X 0..7).
*
* @param[out] enablePtr                - GT_TRUE, enable queue
*                                      GT_FALSE, disable queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS cpssDxChNetIfSdmaRxQueueEnableGet
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           queue,
    OUT GT_BOOL                         *enablePtr
);

/**
* @internal cpssDxChNetIfSdmaTxQueueEnable function
* @endinternal
*
* @brief   Enable/Disable the specified traffic class queue for TX
*         on all packet processors in the system. -- SDMA relate
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1_Diamond.
*
* @param[in] devNum                   - device number
* @param[in] queue                    - traffic class queue
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon 0..31).
*                                      (APPLICABLE RANGES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X 0..7).
* @param[in] enable                   - GT_TRUE,  queue
*                                      GT_FALSE, disable queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on CPU port is not SDMA
*/
GT_STATUS cpssDxChNetIfSdmaTxQueueEnable
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           queue,
    IN  GT_BOOL                         enable
);

/**
* @internal cpssDxChNetIfSdmaTxQueueEnableGet function
* @endinternal
*
* @brief   Get status of the specified traffic class queue for TX
*         packets from CPU. -- SDMA relate
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1_Diamond.
*
* @param[in] devNum                   - device number
* @param[in] queue                    - traffic class queue
*
* @param[out] enablePtr                - GT_TRUE, enable queue
*                                      GT_FALSE, disable queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on CPU port is not SDMA
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS cpssDxChNetIfSdmaTxQueueEnableGet
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           queue,
    OUT GT_BOOL                         *enablePtr
);

/**
* @internal cpssDxChNetIfTxBufferQueueGet function
* @endinternal
*
* @brief   This routine returns the caller the TxEnd parameters for a transmitted
*         packet.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] hndl                     - Handle got from gtEventBind.
*
* @param[out] devPtr                   - The device the packet was transmitted from.
* @param[out] cookiePtr                - The user cookie handed on the transmit request.
* @param[out] queuePtr                 - The queue from which this packet was transmitted
* @param[out] statusPtr                - GT_OK if packet transmission was successful, GT_FAIL on
*                                      packet reject.
*                                       GT_OK on success, or
*
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NO_MORE               - no more packets
* @retval GT_BAD_STATE             - the CPSS not handling the ISR for the device
*                                       so function must not be called
* @retval GT_NOT_INITIALIZED       - the library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                       GT_FAIL otherwise.
*/
GT_STATUS cpssDxChNetIfTxBufferQueueGet
(
    IN  GT_UINTPTR          hndl,
    OUT GT_U8               *devPtr,
    OUT GT_PTR              *cookiePtr,
    OUT GT_U8               *queuePtr,
    OUT GT_STATUS           *statusPtr
);

/**
* @internal cpssDxChNetIfDsaTagBuild function
* @endinternal
*
* @brief   Build DSA tag bytes on the packet from the DSA tag parameters .
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number (different DXCH devices may support differently
*                                      the DSA tag)
* @param[in] dsaInfoPtr               - (pointer to) the DSA parameters that need to be built into
*                                      the packet
*
* @param[out] dsaBytesPtr              - pointer to the start of DSA tag in the packet
*                                      This pointer in the packet should hold space for :
*                                      4 bytes when using regular DSA tag.
*                                      8 bytes when using extended DSA tag.
*                                      16 bytes when using eDSA tag.
*                                      This function only set values into the 4, 8 or 16 bytes
*                                      according to the parameters in parameter dsaInfoPtr.
*                                      NOTE : this memory is the caller's responsibility (to
*                                      allocate / free)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note support:
*       ===== regular DSA tag ====
*       ===== extended DSA tag ====
*       ===== eDSA tag     ====
*
*/
GT_STATUS cpssDxChNetIfDsaTagBuild
(
    IN  GT_U8                          devNum,
    IN  CPSS_DXCH_NET_DSA_PARAMS_STC   *dsaInfoPtr,
    OUT GT_U8                          *dsaBytesPtr
);

/**
* @internal cpssDxChNetIfDsaTagParse function
* @endinternal
*
* @brief   parse the DSA tag parameters from the DSA tag on the packet.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number (different DXCH devices may support differently
*                                      the DSA tag)
* @param[in] dsaBytesPtr              - pointer to the start of DSA tag in the packet
*
* @param[out] dsaInfoPtr               - (pointer to) the DSA parameters that were parsed from the
*                                      packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer.
* @retval GT_DSA_PARSING_ERROR     - DSA tag parsing error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note support:
*       ===== regular DSA tag ====
*       ===== extended DSA tag ====
*       ===== eDSA tag     ====
*
*/
GT_STATUS cpssDxChNetIfDsaTagParse
(
    IN  GT_U8                          devNum,
    IN  GT_U8                          *dsaBytesPtr,
    OUT CPSS_DXCH_NET_DSA_PARAMS_STC   *dsaInfoPtr
);

/**
* @internal cpssDxChNetIfDuplicateEnableSet function
* @endinternal
*
* @brief   Enable descriptor duplication (mirror, STC and ingress mirroring
*         to analyzer port when the analyzer port is the CPU), Or Disable any kind
*         of duplication.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number
* @param[in] enable                   -
*                                      GT_FALSE = Disable any kind of duplication
*                                      GT_TRUE = enable descriptor duplication (mirror, STC and ingress
*                                      mirroring to analyzer port when the analyzer port is
*                                      the CPU).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNetIfDuplicateEnableSet
(
    IN  GT_U8                          devNum,
    IN  GT_BOOL                        enable
);

/**
* @internal cpssDxChNetIfDuplicateEnableGet function
* @endinternal
*
* @brief   Get descriptor duplication (mirror, STC and ingress mirroring
*         to analyzer port when the analyzer port is the CPU) status.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                -
*                                      GT_FALSE = Disable any kind of duplication
*                                      GT_TRUE = enable descriptor duplication (mirror, STC and ingress
*                                      mirroring to analyzer port when the analyzer port is
*                                      the CPU).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS cpssDxChNetIfDuplicateEnableGet
(
    IN  GT_U8                          devNum,
    OUT GT_BOOL                        *enablePtr
);

/**
* @internal cpssDxChNetIfPortDuplicateToCpuSet function
* @endinternal
*
* @brief   set per ingress port if Packets received from the port that are
*         duplicated and their duplication target is the CPU, ( mirror, STC and
*         ingress mirroring to analyzer port when the analyzer port is the CPU)
*         are will be duplicated or just are only forwarded to their original
*         target.
*         NOTE : port may be "CPU port" (63)
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   -
*                                      GT_FALSE = Packets received from the port that are duplicated
*                                      and their duplication target is the CPU, (mirror, STC
*                                      and ingress mirroring to analyzer port when the
*                                      analyzer port is the CPU) are not duplicated and are
*                                      only forwarded to their original target.
*                                      GT_TRUE = Packets received from the port that are duplicated and
*                                      their duplication target is the CPU, (mirror, STC and
*                                      ingress mirroring to analyzer port when the analyzer
*                                      port is the CPU) are duplicated and are only forwarded
*                                      to their original target and to the CPU.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNetIfPortDuplicateToCpuSet
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    IN  GT_BOOL                        enable
);

/**
* @internal cpssDxChNetIfPortDuplicateToCpuGet function
* @endinternal
*
* @brief   get per ingress port if Packets received from the port that are
*         duplicated and their duplication target is the CPU, ( mirror, STC and
*         ingress mirroring to analyzer port when the analyzer port is the CPU)
*         are will be duplicated or just are only forwarded to their original
*         target.
*         NOTE : port may be "CPU port" (63)
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                -
*                                      GT_FALSE = Packets received from the port that are duplicated
*                                      and their duplication target is the CPU, (mirror, STC
*                                      and ingress mirroring to analyzer port when the
*                                      analyzer port is the CPU) are not duplicated and are
*                                      only forwarded to their original target.
*                                      GT_TRUE = Packets received from the port that are duplicated and
*                                      their duplication target is the CPU, (mirror, STC and
*                                      ingress mirroring to analyzer port when the analyzer
*                                      port is the CPU) are duplicated and are only forwarded
*                                      to their original target and to the CPU.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL Ptr
*/
GT_STATUS cpssDxChNetIfPortDuplicateToCpuGet
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    OUT GT_BOOL                        *enablePtr
);

/**
* @internal cpssDxChNetIfAppSpecificCpuCodeTcpSynSet function
* @endinternal
*
* @brief   Enable/Disable Application Specific CPU Code for TCP SYN packets
*         forwarded to the CPU - TCP_SYN_TO_CPU.
*         The feature enabled for packets if one of conditions are true:
*         1. MAC DA is found in the FDB and Application Specific CPU Code field
*         of FDB entry is True (see appCpuCodeEn in GT_MAC_ENTRY)
*         2. Packet passed Routing and Application Specific CPU Code field is
*         True in the Route entry (Next Hop)
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - Device number.
* @param[in] enable                   - GT_TRUE  -  application specific CPU Code for TCP SYN
*                                      packets.
*                                      GT_FALSE - disable application specific CPU Code for TCP SYN
*                                      packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNetIfAppSpecificCpuCodeTcpSynSet
(
    IN  GT_U8                          devNum,
    IN  GT_BOOL                        enable
);

/**
* @internal cpssDxChNetIfAppSpecificCpuCodeTcpSynGet function
* @endinternal
*
* @brief   Get whether Application Specific CPU Code for TCP SYN packets forwarded
*         to the CPU - TCP_SYN_TO_CPU.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - Device number.
*
* @param[out] enablePtr                - (pointer to) is the feature enabled
*                                      GT_TRUE  - enabled ,application specific CPU Code for TCP SYN
*                                      packets.
*                                      GT_FALSE - disabled ,application specific CPU Code for TCP SYN
*                                      packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNetIfAppSpecificCpuCodeTcpSynGet
(
    IN  GT_U8                          devNum,
    OUT GT_BOOL                        *enablePtr
);

/**
* @internal cpssDxChNetIfAppSpecificCpuCodeIpProtocolSet function
* @endinternal
*
* @brief   Set IP Protocol in the IP Protocol CPU Code Table with specific
*         CPU Code. There are 8 IP Protocols may be defined.
*         The feature enabled for packets if one of conditions are true:
*         1. MAC DA is found in the FDB and Application Specific CPU Code field
*         of FDB entry is True (see appCpuCodeEn in GT_MAC_ENTRY)
*         2. Packet passed Routing and Application Specific CPU Code field is
*         True in the Route entry (Next Hop)
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - the  of the IP Protocol (APPLICABLE RANGES: 0..7)
* @param[in] protocol                 - IP  (APPLICABLE RANGES: 0..255)
* @param[in] cpuCode                  - CPU Code for given IP protocol
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad index or bad cpuCode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNetIfAppSpecificCpuCodeIpProtocolSet
(
    IN GT_U8            devNum,
    IN GT_U32           index,
    IN GT_U8            protocol,
    IN CPSS_NET_RX_CPU_CODE_ENT   cpuCode
);

/**
* @internal cpssDxChNetIfAppSpecificCpuCodeIpProtocolInvalidate function
* @endinternal
*
* @brief   Invalidate entry in the IP Protocol CPU Code Table .
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - the  of the IP Protocol (APPLICABLE RANGES: 0..7)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad index
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNetIfAppSpecificCpuCodeIpProtocolInvalidate
(
    IN GT_U8            devNum,
    IN GT_U32           index
);

/**
* @internal cpssDxChNetIfAppSpecificCpuCodeIpProtocolGet function
* @endinternal
*
* @brief   Get IP Protocol in the IP Protocol CPU Code Table with specific
*         CPU Code.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - the  of the IP Protocol (APPLICABLE RANGES: 0..7)
*
* @param[out] validPtr                 - (pointer to)valid entry.
*                                      GT_FALSE - the entry is not valid
*                                      GT_TRUE - the entry is valid
* @param[out] protocolPtr              - (pointer to)IP protocol (APPLICABLE RANGES: 0..255)
* @param[out] cpuCodePtr               - (pointer to)CPU Code for given IP protocol
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad index
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNetIfAppSpecificCpuCodeIpProtocolGet
(
    IN GT_U8            devNum,
    IN GT_U32           index,
    OUT GT_BOOL         *validPtr,
    OUT GT_U8           *protocolPtr,
    OUT CPSS_NET_RX_CPU_CODE_ENT   *cpuCodePtr
);


/**
* @internal cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeSet function
* @endinternal
*
* @brief   Set range to TCP/UPD Destination Port Range CPU Code Table with specific
*         CPU Code. There are 16 ranges may be defined.
*         The feature enabled for packets if one of conditions are true:
*         1. MAC DA is found in the FDB and Application Specific CPU Code field
*         of FDB entry is True (see appCpuCodeEn in GT_MAC_ENTRY)
*         2. Packet passed Routing and Application Specific CPU Code field is
*         True in the Route entry (Next Hop)
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - Device number.
* @param[in] rangeIndex               - the index of the range (APPLICABLE RANGES: 0..15)
* @param[in] maxDstPort               - the maximum destination port in this range.
* @param[in] minDstPort               - the minimum destination port in this range
* @param[in] packetType               - packet type (Unicast/Multicast)
* @param[in] protocol                 -  type (UDP/TCP)
* @param[in] cpuCode                  - CPU Code Index for this TCP/UDP range
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad rangeIndex or
*                                       bad packetType or bad protocol or bad cpuCode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeSet
(
    IN GT_U8                            devNum,
    IN GT_U32                           rangeIndex,
    IN GT_U16                           minDstPort,
    IN GT_U16                           maxDstPort,
    IN CPSS_NET_TCP_UDP_PACKET_TYPE_ENT packetType,
    IN CPSS_NET_PROT_ENT                protocol,
    IN CPSS_NET_RX_CPU_CODE_ENT         cpuCode
);

/**
* @internal cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeInvalidate function
* @endinternal
*
* @brief   invalidate range to TCP/UPD Destination Port Range CPU Code Table with
*         specific CPU Code. There are 16 ranges may be defined.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - Device number.
* @param[in] rangeIndex               - the index of the range (APPLICABLE RANGES: 0..15)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad rangeIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeInvalidate
(
    IN GT_U8                            devNum,
    IN GT_U32                           rangeIndex
);


/**
* @internal cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeGet function
* @endinternal
*
* @brief   Get range for TCP/UPD Destination Port Range CPU Code Table with specific
*         CPU Code. There are 16 ranges may be defined.
*         The feature enabled for packets if one of conditions are true:
*         1. MAC DA is found in the FDB and Application Specific CPU Code field
*         of FDB entry is True (see appCpuCodeEn in GT_MAC_ENTRY)
*         2. Packet passed Routing and Application Specific CPU Code field is
*         True in the Route entry (Next Hop)
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - Device number.
* @param[in] rangeIndex               - the index of the range (APPLICABLE RANGES: 0..15)
*
* @param[out] validPtr                 - (pointer to) is the entry valid
*                                      GT_FALSE - range is't valid
*                                      GT_TRUE  - range is valid
* @param[out] maxDstPortPtr            - (pointer to)the maximum destination port in this range.
* @param[out] minDstPortPtr            - (pointer to)the minimum destination port in this range
* @param[out] packetTypePtr            - (pointer to)packet type (Unicast/Multicast)
* @param[out] protocolPtr              (pointer to)protocol type (UDP/TCP)
*                                      NOTE : this field will hold valid value only when
* @param[out] validPtr                 = GT_TRUE
*                                      because in HW the "valid" is one of the protocol
*                                      options
* @param[out] cpuCodePtr               - (pointer to)CPU Code Index for this TCP/UDP range
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad rangeIndex
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeGet
(
    IN GT_U8                             devNum,
    IN GT_U32                            rangeIndex,
    OUT GT_BOOL                          *validPtr,
    OUT GT_U16                           *minDstPortPtr,
    OUT GT_U16                           *maxDstPortPtr,
    OUT CPSS_NET_TCP_UDP_PACKET_TYPE_ENT *packetTypePtr,
    OUT CPSS_NET_PROT_ENT                *protocolPtr,
    OUT CPSS_NET_RX_CPU_CODE_ENT         *cpuCodePtr
);

/**
* @internal cpssDxChNetIfCpuCodeIpLinkLocalProtSet function
* @endinternal
*
* @brief   Configure CPU code for IPv4 and IPv6 Link Local multicast packets
*         with destination IP in range of 224.0.0.0 ... 224.0.0.255
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - Device number.
* @param[in] ipVer                    - IP version: Ipv4 or Ipv6
* @param[in] protocol                 - Specifies the LSB of IP Link Local multicast protocol
* @param[in] cpuCode                  - The CPU Code
*                                      The 4 CPU acceptable CPU Codes are:
*                                      CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_E
*                                      CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_1_E
*                                      CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_2_E
*                                      CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_3_E
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad ipVer or bad cpuCode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNetIfCpuCodeIpLinkLocalProtSet
(
    IN GT_U8                devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT    ipVer,
    IN GT_U8                protocol,
    IN CPSS_NET_RX_CPU_CODE_ENT  cpuCode
);

/**
* @internal cpssDxChNetIfCpuCodeIpLinkLocalProtGet function
* @endinternal
*
* @brief   get the Configuration CPU code for IPv4 and IPv6 Link Local multicast
*         packets with destination IP in range of 224.0.0.0 ... 224.0.0.255
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - Device number.
* @param[in] ipVer                    - IP version: Ipv4 or Ipv6
* @param[in] protocol                 - Specifies the LSB of IP Link Local multicast protocol
*
* @param[out] cpuCodePtr               - (pointer to) The CPU Code
*                                      The 4 CPU acceptable CPU Codes are:
*                                      CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_E
*                                      CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_1_E
*                                      CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_2_E
*                                      CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_3_E
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad ipVer
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNetIfCpuCodeIpLinkLocalProtGet
(
    IN GT_U8                devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT    ipVer,
    IN GT_U8                protocol,
    OUT CPSS_NET_RX_CPU_CODE_ENT  *cpuCodePtr
);

/**
* @internal cpssDxChNetIfCpuCodeIeeeReservedMcastProtSet function
* @endinternal
*
* @brief   Configure CPU code for particular IEEE reserved mcast protocol
*         (01-80-C2-00-00-00 ... 01-80-C2-00-00-FF)
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - Device number.
* @param[in] protocol                 - Specifies the LSB of IEEE Multicast protocol
* @param[in] cpuCode                  - The CPU Code
*                                      The 4 CPU acceptable CPU Codes are:
*                                      CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_E
*                                      CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_1_E
*                                      CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_2_E
*                                      CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_3_E
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad cpuCode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNetIfCpuCodeIeeeReservedMcastProtSet
(
    IN GT_U8            devNum,
    IN GT_U8            protocol,
    IN CPSS_NET_RX_CPU_CODE_ENT   cpuCode
);

/**
* @internal cpssDxChNetIfCpuCodeIeeeReservedMcastProtGet function
* @endinternal
*
* @brief   get the Configuration CPU code for particular IEEE reserved mcast protocol
*         (01-80-C2-00-00-00 ... 01-80-C2-00-00-FF)
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - Device number.
* @param[in] protocol                 - Specifies the LSB of IEEE Multicast protocol
*
* @param[out] cpuCodePtr               - (pointer to)The CPU Code
*                                      The 4 CPU acceptable CPU Codes are:
*                                      CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_E
*                                      CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_1_E
*                                      CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_2_E
*                                      CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_3_E
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNetIfCpuCodeIeeeReservedMcastProtGet
(
    IN GT_U8            devNum,
    IN GT_U8            protocol,
    OUT CPSS_NET_RX_CPU_CODE_ENT   *cpuCodePtr
);

/**
* @internal cpssDxChNetIfCpuCodeTableSet function
* @endinternal
*
* @brief   Function to set the "CPU code table" entry, for a specific CPU code.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cpuCode                  - the CPU code (key parameter to access the "CPU code table").
*                                      use CPSS_NET_ALL_CPU_OPCODES_E as a wildcard to be used when
*                                      need to set all the table entries with the same info.
* @param[in] entryInfoPtr             - (pointer to) The entry information
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad cpuCode or bad one of
*                                       bad one entryInfoPtr parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNetIfCpuCodeTableSet
(
    IN GT_U8                    devNum,
    IN CPSS_NET_RX_CPU_CODE_ENT cpuCode,
    IN CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC *entryInfoPtr
);

/**
* @internal cpssDxChNetIfCpuCodeTableGet function
* @endinternal
*
* @brief   Function to get the "CPU code table" entry, for a specific CPU code.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cpuCode                  - the CPU code (key parameter to access the "CPU code table").
*                                      use CPSS_NET_ALL_CPU_OPCODES_E as a wildcard to be used when
*                                      need to set all the table entries with the same info.
*
* @param[out] entryInfoPtr             - (pointer to) The entry information
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad cpuCode
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNetIfCpuCodeTableGet
(
    IN  GT_U8                    devNum,
    IN  CPSS_NET_RX_CPU_CODE_ENT cpuCode,
    OUT CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC *entryInfoPtr
);

/**
* @internal cpssDxChNetIfCpuCodeStatisticalRateLimitsTableSet function
* @endinternal
*
* @brief   Function to set the "Statistical Rate Limits Table" entry.
*         The cpssDxChNetIfCpuCodeTableSet(...) points to this table (for a
*         specific CPU Code)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] index                    -  into the Statistical Rate Limits Table
*                                      (APPLICABLE RANGES: DxCh1, DxCh1_Diamond, DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2 0..31;
*                                      Bobcat2, Caelum, Bobcat3; Aldrin2, Aldrin, AC3X 0..255)
* @param[in] statisticalRateLimit     - The statistical rate limit compared to the
*                                      32-bit pseudo-random generator (PRNG).
*                                      NOTE:
*                                      For all packets to be forwarded to the CPU, set this field to
*                                      0xFFFFFFFF.
*                                      For all packets to be dropped, set this field to 0x0.
*                                      This mechanism is based on a hardware-based 32-bit pseudo-random
*                                      generator (PRNG). The PRNG generates evenly-distributed 32-bit
*                                      numbers ranging from 0 through 0xFFFFFFFF.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad index
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNetIfCpuCodeStatisticalRateLimitsTableSet
(
    IN GT_U8        devNum,
    IN GT_U32       index,
    IN GT_U32       statisticalRateLimit
);

/**
* @internal cpssDxChNetIfCpuCodeStatisticalRateLimitsTableGet function
* @endinternal
*
* @brief   Function to get the "Statistical Rate Limits Table" entry.
*         The cpssDxChNetIfCpuCodeTableSet(...) points to this table (for a
*         specific CPU Code)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] index                    -  into the Statistical Rate Limits Table
*                                      (APPLICABLE RANGES: DxCh1, DxCh1_Diamond, DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2 0..31;
*                                      Bobcat2, Caelum, Bobcat3; Aldrin2, Aldrin, AC3X 0..255)
*
* @param[out] statisticalRateLimitPtr  - (pointer to)The statistical rate limit
*                                      compared to the 32-bit pseudo-random generator (PRNG).
*                                      NOTE:
*                                      For all packets to be forwarded to the CPU, set this field to
*                                      0xFFFFFFFF.
*                                      For all packets to be dropped, set this field to 0x0.
*                                      This mechanism is based on a hardware-based 32-bit pseudo-random
*                                      generator (PRNG). The PRNG generates evenly-distributed 32-bit
*                                      numbers ranging from 0 through 0xFFFFFFFF.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad index
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNetIfCpuCodeStatisticalRateLimitsTableGet
(
    IN  GT_U8       devNum,
    IN  GT_U32      index,
    OUT GT_U32      *statisticalRateLimitPtr
);

/**
* @internal cpssDxChNetIfCpuCodeDesignatedDeviceTableSet function
* @endinternal
*
* @brief   Function to set the "designated device Table" entry.
*         The cpssDxChNetIfCpuCodeTableSet(...) points to this table (for a
*         specific CPU Code)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] index                    -  into the designated device table (APPLICABLE RANGES: 1..7)
* @param[in] designatedHwDevNum       - The designated HW device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad index or bad
*                                       designatedHwDevNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNetIfCpuCodeDesignatedDeviceTableSet
(
    IN GT_U8            devNum,
    IN GT_U32           index,
    IN GT_HW_DEV_NUM    designatedHwDevNum
);

/**
* @internal cpssDxChNetIfCpuCodeDesignatedDeviceTableGet function
* @endinternal
*
* @brief   Function to get the "designated device Table" entry.
*         The cpssDxChNetIfCpuCodeTableSet(...) points to this table (for a
*         specific CPU Code)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] index                    -  into the designated device table (APPLICABLE RANGES: 1..7)
*
* @param[out] designatedHwDevNumPtr    - (pointer to)The designated HW device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad index
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNetIfCpuCodeDesignatedDeviceTableGet
(
    IN  GT_U8           devNum,
    IN  GT_U32          index,
    OUT GT_HW_DEV_NUM   *designatedHwDevNumPtr
);

/**
* @internal cpssDxChNetIfCpuCodeRateLimiterTableSet function
* @endinternal
*
* @brief   Configure CPU rate limiter entry attributes: window size and max packets
*         allowed within that window.
*         cpssDxChNetIfCpuCodeTableSet() points to this table (for a
*         specific CPU Code)
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - Device number.
* @param[in] rateLimiterIndex         - CPU Code Rate Limiter Index
*                                      (APPLICABLE RANGES: DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2 1..31;
*                                      Bobcat2, Caelum, Bobcat3; Aldrin2, Aldrin, AC3X 1..255).
* @param[in] windowSize               - window size for this Rate Limiter in steps of
*                                      Rate Limiter Window Resolution set by
*                                      cpssDxChNetIfCpuCodeRateLimiterWindowResolutionSet(...)
*                                      (APPLICABLE RANGES: 0..0xFFF) (12 bits)
* @param[in] pktLimit                 - number of packets allowed to be sent to CPU
*                                      during within the configured windowSize
*                                      (APPLICABLE RANGES: 0..0xFFFF) (16 bits)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad rateLimiterIndex
* @retval GT_OUT_OF_RANGE          - on pktLimit >= 0x10000 or windowSize >= 0x1000
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNetIfCpuCodeRateLimiterTableSet
(
    IN GT_U8  devNum,
    IN GT_U32 rateLimiterIndex,
    IN GT_U32 windowSize,
    IN GT_U32 pktLimit
);

/**
* @internal cpssDxChNetIfCpuCodeRateLimiterTableGet function
* @endinternal
*
* @brief   Get the Configured CPU rate limiter enrty attributes: window size
*         and max packets allowed within that window.
*         cpssDxChNetIfCpuCodeTableSet() points to this table (for a
*         specific CPU Code)
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - Device number.
* @param[in] rateLimiterIndex         - CPU Code Rate Limiter Index
*                                      (APPLICABLE RANGES: DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2 1..31;
*                                      Bobcat2, Caelum, Bobcat3; Aldrin2, Aldrin, AC3X 1..255).
*
* @param[out] windowSizePtr            - (pointer to) window size for this Rate Limiter in steps
*                                      of Rate Limiter Window Resolution set by
*                                      cpssDxChNetIfCpuCodeRateLimiterWindowResolutionSet(...)
* @param[out] pktLimitPtr              - (pointer to) number of packets allowed to be sent to CPU
*                                      during within the configured windowSize
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad rateLimiterIndex
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNetIfCpuCodeRateLimiterTableGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  rateLimiterIndex,
    OUT GT_U32  *windowSizePtr,
    OUT GT_U32  *pktLimitPtr
);

/**
* @internal cpssDxChNetIfCpuCodeRateLimiterWindowResolutionSet function
* @endinternal
*
* @brief   Set the TO CPU Rate Limiter Window Resolution,
*         which is used as steps in Rate Limiter Window size by
*         cpssDxChNetIfCpuCodeRateLimiterTableSet(...)
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - Device number.
* @param[in] windowResolution         - The TO CPU window size resolution -- this field is
*                                      the number of NanoSeconds.
*                                      NOTE:
*                                      The PP's actual resolution is in 32 system clock
*                                      cycles (for a system clock of 200 MHz = 5nano
*                                      per tick --> from 160 nanoseconds up to 327
*                                      milliseconds, steps of 160 nanoseconds)
*                                      So this function will round the value to the nearest
*                                      PP's option.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_OUT_OF_RANGE          - on windowResolution too large(depends on system clock)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNetIfCpuCodeRateLimiterWindowResolutionSet
(
    IN GT_U8  devNum,
    IN GT_U32 windowResolution
);

/**
* @internal cpssDxChNetIfCpuCodeRateLimiterWindowResolutionGet function
* @endinternal
*
* @brief   Get the TO CPU Rate Limiter Window Resolution,
*         which is used as steps in Rate Limiter Window size by
*         cpssDxChNetIfCpuCodeRateLimiterTableSet(...)
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - Device number.
*
* @param[out] windowResolutionPtr      - (pointer to)The TO CPU window size resolution --
*                                      this field is the number of NanoSeconds.
*                                      NOTE:
*                                      The PP's actual resolution is in 32 system clock
*                                      cycles (for a system clock of 200 MHz = 5nano
*                                      per tick --> from 160 nanoseconds up to 327
*                                      milliseconds, steps of 160 nanoseconds)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNetIfCpuCodeRateLimiterWindowResolutionGet
(
    IN  GT_U8  devNum,
    OUT GT_U32 *windowResolutionPtr
);

/**
* @internal cpssDxChNetIfCpuCodeRateLimiterPacketCntrGet function
* @endinternal
*
* @brief   Gets the cpu code rate limiter packet counter for specific
*         rate limiter index.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - physical device number.
* @param[in] rateLimiterIndex         - CPU Code Rate Limiter Index
*                                      (APPLICABLE RANGES: DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2 1..31;
*                                      Bobcat2, Caelum, Bobcat3; Aldrin2, Aldrin, AC3X 1..255).
*
* @param[out] packetCntrPtr            - pointer to the number of packets forwarded to the CPU
*                                      during the current window (set by
*                                      cpssDxChNetIfCpuCodeRateLimiterTableSet)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The counter is reset to 0 when the window ends.
*
*/
GT_STATUS cpssDxChNetIfCpuCodeRateLimiterPacketCntrGet
(
    IN   GT_U8               devNum,
    IN   GT_U32              rateLimiterIndex,
    OUT  GT_U32              *packetCntrPtr
);

/**
* @internal cpssDxChNetIfCpuCodeRateLimiterDropCntrGet function
* @endinternal
*
* @brief   get the number of packets forwarded to the CPU and dropped due to any
*         of the CPU rate limiters.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - Device number.
*
* @param[out] dropCntrPtr              - (pointer to) The total number of dropped packets
*                                      due to any of the CPU rate limiters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNetIfCpuCodeRateLimiterDropCntrGet
(
    IN GT_U8                devNum,
    OUT GT_U32              *dropCntrPtr
);

/**
* @internal cpssDxChNetIfCpuCodeRateLimiterDropCntrSet function
* @endinternal
*
* @brief   set the cpu code rate limiter drop counter to a specific value. This counter
*         counts the number of packets forwarded to the CPU and dropped due to any
*         of the CPU rate limiters.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - Device number.
* @param[in] dropCntrVal              - the value to be configured.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNetIfCpuCodeRateLimiterDropCntrSet
(
    IN GT_U8                devNum,
    IN GT_U32               dropCntrVal
);

/**
* @internal cpssDxChNetIfFromCpuDpSet function
* @endinternal
*
* @brief   Set DP for from CPU DSA tag packets.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] dpLevel                  - drop precedence level [Green, Red].
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number, dpLevel
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNetIfFromCpuDpSet
(
    IN GT_U8               devNum,
    IN  CPSS_DP_LEVEL_ENT  dpLevel
);

/**
* @internal cpssDxChNetIfFromCpuDpGet function
* @endinternal
*
* @brief   Get DP for from CPU DSA tag packets.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
*
* @param[out] dpLevelPtr               - pointer to drop precedence level
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - invalid hardware value for drop precedence level
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNetIfFromCpuDpGet
(
    IN  GT_U8              devNum,
    OUT CPSS_DP_LEVEL_ENT  *dpLevelPtr
);

/**
* @internal cpssDxChNetIfSdmaRxResourceErrorModeSet function
* @endinternal
*
* @brief   Set a bit per TC queue which defines the behavior in case of
*         RX resource error
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] queue                    - traffic class queue
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon 0..31).
*                                      (APPLICABLE RANGES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X 0..7).
* @param[in] mode                     - current packet mode: retry to send or abort
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number, queue, mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - the request is not supported
*/
GT_STATUS cpssDxChNetIfSdmaRxResourceErrorModeSet
(
    IN  GT_U8                                   devNum,
    IN  GT_U8                                   queue,
    IN  CPSS_DXCH_NET_RESOURCE_ERROR_MODE_ENT   mode
);

/**
* @internal cpssDxChNetIfSdmaRxResourceErrorModeGet function
* @endinternal
*
* @brief   Get a bit per TC queue which defines the behavior in case of
*         RX resource error
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] queue                    - traffic class queue
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon 0..31).
*                                      (APPLICABLE RANGES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X 0..7).
*
* @param[out] modePtr                  - current packet mode: retry to send or abort
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number, queue, mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - the request is not supported
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS cpssDxChNetIfSdmaRxResourceErrorModeGet
(
    IN  GT_U8                                   devNum,
    IN  GT_U8                                   queue,
    OUT CPSS_DXCH_NET_RESOURCE_ERROR_MODE_ENT   *modePtr
);

/**
* @internal cpssDxChNetIfSdmaTxBufferQueueDescFreeAndCookieGet function
* @endinternal
*
* @brief   This routine frees all transmitted packets descriptors. In addition, all
*         user relevant data in Tx End FIFO.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1_Diamond.
*
* @param[in] devNum                   - The device number the packet was transmitted from
* @param[in] txQueue                  - The queue the packet was transmitted upon
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon 0..31).
*                                      (APPLICABLE RANGES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X 0..7).
*
* @param[out] cookiePtr                - (pointer to) the cookie attached to packet that was send
*                                      from this queue
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device or queue
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NO_MORE               - no more packet cookies to get
* @retval GT_ERROR                 - the Tx descriptor is corrupted
* @retval GT_NOT_INITIALIZED       - the library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. returns the caller the cookie attached to the frame that was sent from the
*       device on the specific queue.
*       2. put the descriptors of this sent packet back to the 'Free descriptors' list
*       --> SDMA relate
*       3. this function should be used only when the ISR of the CPSS is not in use,
*       meaning that the intVecNum (given in function cpssDxChHwPpPhase1Init) was
*       set to CPSS_PP_INTERRUPT_VECTOR_NOT_USED_CNS.
*       NOTE: the caller is responsible to synchronize the calls to 'Tx send' and
*       this function , since both deal with the 'Tx descriptors'.
*
*/
GT_STATUS cpssDxChNetIfSdmaTxBufferQueueDescFreeAndCookieGet
(
    IN GT_U8                devNum,
    IN GT_U8                txQueue,
    OUT GT_PTR             *cookiePtr
);

/**
* @internal cpssDxChNetIfSdmaTxFreeDescripotrsNumberGet function
* @endinternal
*
* @brief   This function return the number of free Tx descriptors for given
*         device and txQueue
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1_Diamond.
*
* @param[in] devNum                   - Device number.
* @param[in] txQueue                  - Tx queue number.
*
* @param[out] numberOfFreeTxDescriptorsPtr - pointer to number of free
*                                      descriptors for the given queue.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device or queue
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_INITIALIZED       - the library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNetIfSdmaTxFreeDescripotrsNumberGet
(
    IN GT_U8                devNum,
    IN GT_U8                txQueue,
    OUT GT_U32             *numberOfFreeTxDescriptorsPtr
);

/**
* @internal cpssDxChNetIfPortGroupCpuCodeRateLimiterTableSet function
* @endinternal
*
* @brief   Configure CPU rate limiter entry attributes: window size and max packets
*         allowed within that window.
*         cpssDxChNetIfCpuCodeTableSet() points to this table (for a
*         specific CPU Code)
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - Device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter
*                                      is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] rateLimiterIndex         - CPU Code Rate Limiter Index
*                                      (APPLICABLE RANGES: DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2 1..31;
*                                      Bobcat2, Caelum, Bobcat3; Aldrin2, Aldrin, AC3X 1..255)
* @param[in] windowSize               - window size for this Rate Limiter in steps of
*                                      Rate Limiter Window Resolution set by
*                                      cpssDxChNetIfCpuCodeRateLimiterWindowResolutionSet(...)
*                                      (APPLICABLE RANGES: 0..0xFFF) (12 bits)
* @param[in] pktLimit                 - number of packets allowed to be sent to CPU
*                                      during the configured windowSize.
*                                      (APPLICABLE RANGES: 0..0xFFFF) (16 bits)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on pktLimit >= 0x10000 or windowSize >= 0x1000
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note for multi portgroup device, a rate limiter limits the rate of the incoming
*       traffic from a given port group. i.e. if a CPU code is configured to allow up
*       to 10 packets per window, it will actually allow up to 10 packets per window
*       from each portgroup.
*
*/
GT_STATUS cpssDxChNetIfPortGroupCpuCodeRateLimiterTableSet
(
    IN GT_U8                devNum,
    IN GT_PORT_GROUPS_BMP   portGroupsBmp,
    IN GT_U32               rateLimiterIndex,
    IN GT_U32               windowSize,
    IN GT_U32               pktLimit
);

/**
* @internal cpssDxChNetIfPortGroupCpuCodeRateLimiterTableGet function
* @endinternal
*
* @brief   Get the Configured CPU rate limiter enrty attributes: window size
*         and max packets allowed within that window.
*         cpssDxChNetIfCpuCodeTableSet() points to this table (for a
*         specific CPU Code)
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - Device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter
*                                      is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] rateLimiterIndex         - CPU Code Rate Limiter Index
*                                      (APPLICABLE RANGES: DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2 1..31;
*                                      Bobcat2, Caelum, Bobcat3; Aldrin2, Aldrin, AC3X 1..255)
*
* @param[out] windowSizePtr            - (pointer to) window size for this Rate Limiter in steps
*                                      of Rate Limiter Window Resolution set by
*                                      cpssDxChNetIfCpuCodeRateLimiterWindowResolutionSet(...)
* @param[out] pktLimitPtr              - (pointer to) number of packets allowed to be sent to CPU
*                                      during the configured windowSize.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad rateLimiterIndex
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note for multi portgroup device, a rate limiter limits the rate of the incoming
*       traffic from a given port group. i.e. if a CPU code is configured to allow up
*       to 10 packets per window, it will actually allow up to 10 packets per window
*       from each portgroup.
*
*/
GT_STATUS cpssDxChNetIfPortGroupCpuCodeRateLimiterTableGet
(
    IN  GT_U8               devNum,
    IN GT_PORT_GROUPS_BMP   portGroupsBmp,
    IN  GT_U32              rateLimiterIndex,
    OUT GT_U32              *windowSizePtr,
    OUT GT_U32              *pktLimitPtr
);

/**
* @internal cpssDxChNetIfPortGroupCpuCodeRateLimiterWindowResolutionSet function
* @endinternal
*
* @brief   Set the TO CPU Rate Limiter Window Resolution,
*         which is used as steps in Rate Limiter Window size by
*         cpssDxChNetIfCpuCodeRateLimiterTableSet(...)
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - Device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter
*                                      is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] windowResolution         - The TO CPU window size resolution -- this field is
*                                      the number of NanoSeconds.
*                                      NOTE:
*                                      The PP's actual resolution is in 32 system clock
*                                      cycles (for a system clock of 200 MHz = 5nano
*                                      per tick --> from 160 nanoseconds up to 327
*                                      milliseconds, steps of 160 nanoseconds)
*                                      So this function will round the value to the nearest
*                                      PP's option.
*                                      field range: minimal value is 1. Maximal is 2047.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_OUT_OF_RANGE          - on windowResolution too large(depends on system clock)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNetIfPortGroupCpuCodeRateLimiterWindowResolutionSet
(
    IN GT_U8                devNum,
    IN GT_PORT_GROUPS_BMP   portGroupsBmp,
    IN GT_U32               windowResolution
);

/**
* @internal cpssDxChNetIfPortGroupCpuCodeRateLimiterWindowResolutionGet function
* @endinternal
*
* @brief   Get the TO CPU Rate Limiter Window Resolution,
*         which is used as steps in Rate Limiter Window size by
*         cpssDxChNetIfCpuCodeRateLimiterTableSet(...)
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - Device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter
*                                      is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
* @param[out] windowResolutionPtr      - (pointer to)The TO CPU window size resolution --
*                                      this field is the number of NanoSeconds.
*                                      NOTE:
*                                      The PP's actual resolution is in 32 system clock
*                                      cycles (for a system clock of 200 MHz = 5nano
*                                      per tick --> from 160 nanoseconds up to 327
*                                      milliseconds, steps of 160 nanoseconds)
*                                      field range: minimal value is 1. Maximal is 2047.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNetIfPortGroupCpuCodeRateLimiterWindowResolutionGet
(
    IN  GT_U8                devNum,
    IN  GT_PORT_GROUPS_BMP   portGroupsBmp,
    OUT GT_U32               *windowResolutionPtr
);

/**
* @internal cpssDxChNetIfPortGroupCpuCodeRateLimiterPacketCntrGet function
* @endinternal
*
* @brief   Gets the cpu code rate limiter packet counter for specific
*         rate limiter index.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - physical device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] rateLimiterIndex         - CPU Code Rate Limiter Index
*                                      (APPLICABLE RANGES: DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2 1..31;
*                                      Bobcat2, Caelum, Bobcat3; Aldrin2, Aldrin, AC3X 1..255)
*
* @param[out] packetCntrPtr            - pointer to the number of packets forwarded to the CPU
*                                      during the current window (set by
*                                      cpssDxChNetIfPortGroupCpuCodeRateLimiterTableSet)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The counter is reset to 0 when the window ends.
*
*/
GT_STATUS cpssDxChNetIfPortGroupCpuCodeRateLimiterPacketCntrGet
(
    IN   GT_U8               devNum,
    IN   GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN   GT_U32              rateLimiterIndex,
    OUT  GT_U32              *packetCntrPtr
);

/**
* @internal cpssDxChNetIfPortGroupCpuCodeRateLimiterDropCntrGet function
* @endinternal
*
* @brief   get the number of packets forwarded to the CPU and dropped due to any
*         of the CPU rate limiters.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - Device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
* @param[out] dropCntrPtr              - (pointer to) The total number of dropped packets
*                                      due to any of the CPU rate limiters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNetIfPortGroupCpuCodeRateLimiterDropCntrGet
(
    IN GT_U8                devNum,
    IN GT_PORT_GROUPS_BMP   portGroupsBmp,
    OUT GT_U32              *dropCntrPtr
);

/**
* @internal cpssDxChNetIfPortGroupCpuCodeRateLimiterDropCntrSet function
* @endinternal
*
* @brief   set the cpu code rate limiter drop counter to a specific value. This counter
*         counts the number of packets forwarded to the CPU and dropped due to any
*         of the CPU rate limiters.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] dropCntrVal              - the value to be configured.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNetIfPortGroupCpuCodeRateLimiterDropCntrSet
(
    IN GT_U8                devNum,
    IN GT_PORT_GROUPS_BMP   portGroupsBmp,
    IN GT_U32               dropCntrVal
);


/**
* @internal cpssDxChNetIfSdmaTxGeneratorPacketAdd function
* @endinternal
*
* @brief   This function adds a new packet to Tx SDMA working as Packet Generator.
*         This packet will be transmitted by the selected Tx SDMA with previous
*         packets already transmitted by this Packet generator.
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - Device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is
*                                      IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES None)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] packetParamsPtr          - (pointer to) The internal packet params to be set
*                                      into the packet descriptors.
* @param[in] packetDataPtr            - (pointer to) packet data.
* @param[in] packetDataLength         - packet data length. Buffer size configured during
*                                      initialization phase must be sufficient for packet
*                                      length and the DSA tag that will be added to it.
*
* @param[out] packetIdPtr              - (pointer to) packet identification number, used by
*                                      other Packet Generator functions which require access
*                                      to this packet.
*
* @retval GT_OK                    - on success.
* @retval GT_NO_RESOURCE           - no available buffer or descriptor.
* @retval GT_BAD_PARAM             - Tx SDMA is not in Generator mode, bad DSA
*                                       params or the data buffer is longer than
*                                       size configured.
* @retval GT_BAD_VALUE             - addition of packet will violate the required
*                                       rate configuration.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_INITIALIZED       - the library was not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1. Packet can be added while Tx SDMA is enabled.
*       2. The packet buffer supplied by the application is copied to internal
*       prealloocated generator buffer, therefore application can free or reuse
*       that buffer as soon as function returns.
*
*/
GT_STATUS cpssDxChNetIfSdmaTxGeneratorPacketAdd
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    IN  CPSS_DXCH_NET_TX_PARAMS_STC *packetParamsPtr,
    IN  GT_U8                       *packetDataPtr,
    IN  GT_U32                      packetDataLength,
    OUT GT_U32                      *packetIdPtr
);


/**
* @internal cpssDxChNetIfSdmaTxGeneratorPacketUpdate function
* @endinternal
*
* @brief   This function updates already transmitted packet content and parameters.
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - Device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is
*                                      IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES None)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] packetId                 - packet identification number assign during packet
*                                      addition to the Generator.
* @param[in] packetParamsPtr          - (pointer to) The internal packet params to be set
*                                      into the packet descriptors.
* @param[in] packetDataPtr            - (pointer to) packet data.
* @param[in] packetDataLength         - packet data length. Buffer size configured during
*                                      initialization phase must be sufficient for packet
*                                      length and the DSA tag that will be added to it.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - Tx SDMA is not in Generator mode, packet is
*                                       not transmitted on this Tx SDMA, bad DSA
*                                       params or the data buffer is longer than
*                                       size configured.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_INITIALIZED       - the library was not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1. Packet can be updated while Tx SDMA is enabled.
*       2. The packet buffer supplied by the application is copied to internal
*       prealloocated generator buffer, therefore application can free or reuse
*       that buffer as soon as function returns.
*
*/
GT_STATUS cpssDxChNetIfSdmaTxGeneratorPacketUpdate
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    IN  GT_U32                      packetId,
    IN  CPSS_DXCH_NET_TX_PARAMS_STC *packetParamsPtr,
    IN  GT_U8                       *packetDataPtr,
    IN  GT_U32                      packetDataLength

);


/**
* @internal cpssDxChNetIfSdmaTxGeneratorPacketRemove function
* @endinternal
*
* @brief   This function removes packet from Tx SDMA working as a Packet Generator.
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - Device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is
*                                      IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES None)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] txQueue                  - The Tx SDMA queue to remove the packet from.
* @param[in] packetId                 - packet identification number assign during packet
*                                      addition to the Generator.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - Tx SDMA is not in Generator mode or packet
*                                       is not transmitted on this Tx SDMA.
* @retval GT_BAD_VALUE             - removal of packet will violate the required
*                                       rate configuration.
* @retval GT_NOT_INITIALIZED       - the library was not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note Packet can be removed while Tx SDMA is enabled.
*
*/
GT_STATUS cpssDxChNetIfSdmaTxGeneratorPacketRemove
(
    IN  GT_U8               devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN  GT_U8               txQueue,
    IN  GT_U32              packetId
);


/**
* @internal cpssDxChNetIfSdmaTxGeneratorRateSet function
* @endinternal
*
* @brief   This function sets Tx SDMA Generator transmission packet rate.
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - Device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is
*                                      IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES None)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] txQueue                  - The Tx SDMA queue.
* @param[in] rateMode                 - transmission rate mode.
* @param[in] rateValue                - packets per second for rateMode ==
*                                      CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_GLOBAL_THROUGHPUT_E,
*                                      nanoseconds for rateMode ==
*                                      CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_PER_FLOW_INTERVAL_E.
*                                      Not relevant to CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_NO_GAP_E.
*
* @param[out] actualRateValuePtr       - (pointer to) the actual configured rate value.
*                                      Same units as rateValue.
*                                      Not relevant to CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_NO_GAP_E.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - Tx SDMA is not in Generator mode, bad rate
*                                       mode or value.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_BAD_VALUE             - rate cannot be configured for Tx SDMA.
* @retval GT_NOT_INITIALIZED       - the library was not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note Rate can be set while Tx SDMA is enabled.
*
*/
GT_STATUS cpssDxChNetIfSdmaTxGeneratorRateSet
(
    IN  GT_U8                                       devNum,
    IN  GT_PORT_GROUPS_BMP                          portGroupsBmp,
    IN  GT_U8                                       txQueue,
    IN  CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_ENT    rateMode,
    IN  GT_U64                                      rateValue,
    OUT GT_U64                                      *actualRateValuePtr
);


/**
* @internal cpssDxChNetIfSdmaTxGeneratorRateGet function
* @endinternal
*
* @brief   This function gets Tx SDMA Generator transmission packet rate.
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - Device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is
*                                      IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES None)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] txQueue                  - The Tx SDMA queue.
*
* @param[out] rateModePtr              - (pointer to) transmission rate mode
* @param[out] rateValuePtr             - (pointer to)
*                                      packets per second for rateMode ==
*                                      CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_GLOBAL_THROUGHPUT_E,
*                                      nanoseconds for rateMode ==
*                                      CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_PER_FLOW_INTERVAL_E.
*                                      Not relevant to CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_NO_GAP_E.
* @param[out] actualRateValuePtr       - (pointer to) the actual configured rate value.
*                                      Same units as rateValuePtr.
*                                      Not relevant to CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_NO_GAP_E.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - Tx SDMA is not in Generator mode.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_INITIALIZED       - the library was not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChNetIfSdmaTxGeneratorRateGet
(
    IN  GT_U8                                       devNum,
    IN  GT_PORT_GROUPS_BMP                          portGroupsBmp,
    IN  GT_U8                                       txQueue,
    OUT CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_ENT    *rateModePtr,
    OUT GT_U64                                      *rateValuePtr,
    OUT GT_U64                                      *actualRateValuePtr
);


/**
* @internal cpssDxChNetIfSdmaTxGeneratorEnable function
* @endinternal
*
* @brief   This function enables selected Tx SDMA Generator.
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - Device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is
*                                      IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES None)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] txQueue                  - The Tx SDMA queue to enable.
* @param[in] burstEnable              - GT_TRUE for packets burst generation,
*                                      - GT_FALSE for continuous packets generation.
* @param[in] burstPacketsNumber       - Number of packets in burst.
*                                      Relevant only if burstEnable == GT_TRUE.
*                                      (APPLICABLE RANGES: 0..512M-1)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - Tx SDMA is not in Generator mode or burst
*                                       size.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_INITIALIZED       - the library was not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note If queue is empty, configuration is done beside the enabling which will
*       be implicitly performed after the first packet will be added to the
*       queue.
*       On each HW queue enabling the Tx SDMA Current Descriptor Pointer
*       register will set to point to the first descriptor in chain list.
*
*/
GT_STATUS cpssDxChNetIfSdmaTxGeneratorEnable
(
    IN  GT_U8                           devNum,
    IN  GT_PORT_GROUPS_BMP              portGroupsBmp,
    IN  GT_U8                           txQueue,
    IN  GT_BOOL                         burstEnable,
    IN  GT_U32                          burstPacketsNumber
);


/**
* @internal cpssDxChNetIfSdmaTxGeneratorDisable function
* @endinternal
*
* @brief   This function disables selected Tx SDMA Generator.
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - Device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is
*                                      IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES None)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] txQueue                  - The Tx SDMA queue to enable.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - Tx SDMA is not in Generator mode.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_INITIALIZED       - the library was not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChNetIfSdmaTxGeneratorDisable
(
    IN  GT_U8                           devNum,
    IN  GT_PORT_GROUPS_BMP              portGroupsBmp,
    IN  GT_U8                           txQueue
);


/**
* @internal cpssDxChNetIfSdmaTxGeneratorBurstStatusGet function
* @endinternal
*
* @brief   This function gets Tx SDMA Generator burst counter status.
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - Device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is
*                                      IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES None)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] txQueue                  - The Tx SDMA queue to enable.
*
* @param[out] burstStatusPtr           - (pointer to) Burst counter status.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - Tx SDMA is not in Generator mode.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_INITIALIZED       - the library was not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChNetIfSdmaTxGeneratorBurstStatusGet
(
    IN  GT_U8                                       devNum,
    IN  GT_PORT_GROUPS_BMP                          portGroupsBmp,
    IN  GT_U8                                       txQueue,
    OUT CPSS_DXCH_NET_TX_GENERATOR_BURST_STATUS_ENT *burstStatusPtr
);

/**
* @internal cpssDxChNetIfToCpuFlowIdOrTtOffsetModeSet function
* @endinternal
*
* @brief   Set which mode the 'TO_CPU' dsa tag field <Flow-ID/TT Offset> will use
*         'flowid' or 'ttOffset'.
*         related to CPSS_DXCH_NET_DSA_TO_CPU_STC::flowIdTtOffset
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] mode                     - one of the 'flowid' or 'ttOffset' modes.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number, mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNetIfToCpuFlowIdOrTtOffsetModeSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_NET_TO_CPU_FLOW_ID_OR_TT_OFFSET_MODE_ENT   mode
);

/**
* @internal cpssDxChNetIfToCpuFlowIdOrTtOffsetModeGet function
* @endinternal
*
* @brief   Get which mode the 'TO_CPU' dsa tag field <Flow-ID/TT Offset> will use
*         'flowid' or 'ttOffset'.
*         related to CPSS_DXCH_NET_DSA_TO_CPU_STC::flowIdTtOffset
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
*
* @param[out] modePtr                  - (pointer to) one of the 'flowid' or 'ttOffset' modes.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - on non synch value between 2 relevant units.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNetIfToCpuFlowIdOrTtOffsetModeGet
(
    IN  GT_U8                                   devNum,
    OUT  CPSS_DXCH_NET_TO_CPU_FLOW_ID_OR_TT_OFFSET_MODE_ENT   *modePtr
);

/**
* @internal cpssDxChNetIfCpuCodeToPhysicalPortGet function
* @endinternal
*
* @brief   Get the CPU code to physical port mapping.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2
*
* @param[in] devNum                   - Device number.
* @param[in] cpuCode                  - The cpu code
*
* @param[out] portNumPtr               – The physical port number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad cpu code or bad physical port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on Illegal pointer value
*/
GT_STATUS cpssDxChNetIfCpuCodeToPhysicalPortGet
(
    IN  GT_U8                        devNum,
    IN  CPSS_NET_RX_CPU_CODE_ENT     cpuCode,
    OUT GT_PHYSICAL_PORT_NUM         *portNumPtr
);

/**
* @internal cpssDxChNetIfCpuCodeToPhysicalPortSet function
* @endinternal
*
* @brief   CPU code to physical port mapping.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2
*
* @param[in] devNum                   - Device number.
* @param[in] cpuCode                  - The cpu code
* @param[in] portNum                  - The physical port number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad cpu code or bad physical port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNetIfCpuCodeToPhysicalPortSet
(
    IN GT_U8                        devNum,
    IN CPSS_NET_RX_CPU_CODE_ENT     cpuCode,
    IN GT_PHYSICAL_PORT_NUM         portNum
);


/**
* @internal cpssDxChNetIfSdmaPhysicalPortToQueueGet function
* @endinternal
*
* @brief  Get the 'first' queue of the CPU SDMA port.
*         NOTE: the CPU SDMA hold 8 queues.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;
*                                  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                - Device number.
* @param[in] portNum               - The physical port number of one of the CPU SDMA ports
* @param[out] firstQueuePtr        - (pointer to) the 'first' queue of the CPU SDMA port.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad physical port
* @retval GT_BAD_STATE             - on physical port that is not 'CPU SDMA' port.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNetIfSdmaPhysicalPortToQueueGet
(
    IN GT_U8                        devNum,
    IN GT_PHYSICAL_PORT_NUM         portNum,
    OUT GT_U8                       *firstQueuePtr
);

/**
* @internal cpssDxChNetIfSdmaQueueToPhysicalPortGet function
* @endinternal
*
* @brief  Get the CPU SDMA physical port , that serves the SDMA queue index
*         NOTE: the CPU SDMA hold 8 queues.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;
*                                  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                - Device number.
* @param[in] queueIdx              - the global queue index (one of 8) of the CPU SDMA port.
* @param[out] portNumPtr           - (pointer to) The physical port number of the CPU SDMA port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad queue index
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNetIfSdmaQueueToPhysicalPortGet
(
    IN GT_U8                        devNum,
    IN GT_U8                        queueIdx,
    OUT GT_PHYSICAL_PORT_NUM        *portNumPtr
);

#ifdef __cplusplus
}
#endif

#endif  /* __cpssDxChNetIfh */

