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
* @file cpssDxChNetIfMii.h
*
* @brief Include DxCh network interface API functions for
* PPs connected to CPU by MII/RGMII Ethernet port but not SDMA
*
* @version   12
********************************************************************************
*/
#ifndef __cpssDxChNetIfMiih
#define __cpssDxChNetIfMiih

#ifdef __cplusplus
extern "C" {
#endif

#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h>
#include <cpss/generic/cpssHwInit/cpssHwInit.h>

/**
* @struct CPSS_DXCH_NETIF_MII_INIT_STC
 *
 * @brief structure of network interface by MII/RGMII Ethernet port
 * initialization parameters.
*/
typedef struct{

    /** @brief number of requested TX descriptors. Decriptors dispersed
     *  evenly among all TX queues.
     *  txInternalBufBlockPtr - (Pointer to) a block of host memory to be used
     *  for internal TX buffers.
     */
    GT_U32 numOfTxDesc;

    GT_U8 *txInternalBufBlockPtr;

    /** @brief The raw size in bytes of
     *  txInternalBufBlock memory.
     *  Recommended size of txInternalBufBlock is
     *  (numOfTxDesc / 2) CPSS_GEN_NETIF_MII_TX_INTERNAL_BUFF_SIZE_CNS).
     */
    GT_U32 txInternalBufBlockSize;

    GT_U32 bufferPercentage[CPSS_MAX_RX_QUEUE_CNS];

    /** The size of the Rx data buffer. */
    GT_U32 rxBufSize;

    /** @brief packet header offset size
     *  rxBufBlockPtr  - (Pointer to) a block of memory to be used for
     *  allocating Rx packet data buffers.
     */
    GT_U32 headerOffset;

    GT_U8 *rxBufBlockPtr;

    /** The raw size in byte of rxDataBufBlock. */
    GT_U32 rxBufBlockSize;

} CPSS_DXCH_NETIF_MII_INIT_STC;

/**
* @internal cpssDxChNetIfMiiTxPacketSend function
* @endinternal
*
* @brief   This function receives packet buffers & parameters from Application,
*         prepares them for the transmit operation, and
*         transmits a packet through the ethernet port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] pcktParamsPtr            - The internal packet params to be set into the packet
*                                      descriptors.
* @param[in] packetBuffsArrPtr[]      - (pointer to)The packet data buffers list.
* @param[in] buffLenArr[]             - A list of the buffers len in packetBuffsArrPtr.
* @param[in] numOfBufs                - Length of packetBuffsArrPtr.
*                                       GT_OK on success, or
*                                       GT_NO_RESOURCE if there is not enough desc. for enqueuing the packet.
*                                       GT_FAIL otherwise.
*
* @note Support both regular and extended DSA Tag.
*
*/
GT_STATUS cpssDxChNetIfMiiTxPacketSend
(
    IN GT_U8                        devNum,
    IN CPSS_DXCH_NET_TX_PARAMS_STC *pcktParamsPtr,
    IN GT_U8                       *packetBuffsArrPtr[],
    IN GT_U32                       buffLenArr[],
    IN GT_U32                       numOfBufs
);

/**
* @internal cpssDxChNetIfMiiSyncTxPacketSend function
* @endinternal
*
* @brief   This function receives packet buffers & parameters from Application,
*         prepares them for the transmit operation, and
*         transmits a packet through the ethernet port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] pcktParamsPtr            - The internal packet params to be set into the packet
*                                      descriptors.
* @param[in] packetBuffsArrPtr[]      - (pointer to)The packet data buffers list.
* @param[in] buffLenArr[]             - A list of the buffers len in packetBuffsArrPtr.
* @param[in] numOfBufs                - Length of packetBuffsArrPtr.
*                                       GT_OK on success, or
*                                       GT_NO_RESOURCE if there is not enough desc. for enqueuing the packet.
*                                       GT_FAIL otherwise.
*
* @note Support both regular and extended DSA Tag.
*
*/
GT_STATUS cpssDxChNetIfMiiSyncTxPacketSend
(
    IN GT_U8                        devNum,
    IN CPSS_DXCH_NET_TX_PARAMS_STC *pcktParamsPtr,
    IN GT_U8                       *packetBuffsArrPtr[],
    IN GT_U32                       buffLenArr[],
    IN GT_U32                       numOfBufs
);

/**
* @internal cpssDxChNetIfMiiTxBufferQueueGet function
* @endinternal
*
* @brief   This routine returns the caller the TxEnd parameters for a transmitted
*         packet.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] hndl                     - Handle got from cpssEventBind.
*
* @param[out] devPtr                   - The device the packet was transmitted from.
* @param[out] cookiePtr                - The user cookie handed on the transmit request.
* @param[out] queuePtr                 - The queue from which this packet was transmitted
* @param[out] statusPtr                - GT_OK if packet transmission was successful, GT_FAIL on
*                                      packet reject.
*                                       GT_OK on success, or
*                                       GT_FAIL otherwise.
*/
GT_STATUS cpssDxChNetIfMiiTxBufferQueueGet
(
    IN GT_UINTPTR            hndl,
    OUT GT_U8               *devPtr,
    OUT GT_PTR              *cookiePtr,
    OUT GT_U8               *queuePtr,
    OUT GT_STATUS           *statusPtr
);

/**
* @internal cpssDxChNetIfMiiRxPacketGet function
* @endinternal
*
* @brief   This function returns packets from PP destined to the CPU port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number throw which packet was received.
* @param[in] queueIdx                 - The queue from which this packet was received (APPLICABLE RANGES: 0..7).
* @param[in,out] numOfBuffPtr             Num of buffs in packetBuffsArrPtr.
* @param[in,out] numOfBuffPtr             - Num of used buffs in packetBuffsArrPtr.
*
* @param[out] packetBuffsArrPtr[]      - (pointer to) The received packet buffers list.
* @param[out] buffLenArr[]             - List of buffer lengths for packetBuffsArrPtr.
* @param[out] rxParamsPtr              - (Pointer to)information parameters of received packets
*
* @retval GT_NO_MORE               - no more packets on the device/queue
* @retval GT_OK                    - packet got with no error.
* @retval GT_FAIL                  - failed to get the packet
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - wrong devNum,queueIdx.
* @retval GT_DSA_PARSING_ERROR     - DSA tag parsing error.
*/
GT_STATUS cpssDxChNetIfMiiRxPacketGet
(
    IN GT_U8                            devNum,
    IN GT_U8                            queueIdx,
    INOUT GT_U32                        *numOfBuffPtr,
    OUT GT_U8                           *packetBuffsArrPtr[],
    OUT GT_U32                          buffLenArr[],
    OUT CPSS_DXCH_NET_RX_PARAMS_STC     *rxParamsPtr
);

/**
* @internal cpssDxChNetIfMiiRxBufFree function
* @endinternal
*
* @brief   Frees a list of buffers, that where previously passed to the upper layer
*         in an Rx event.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number throw which these buffers where
*                                      received.
* @param[in] rxQueue                  - The Rx queue number throw which these buffers where
*                                      received (APPLICABLE RANGES: 0..7).
* @param[in] rxBuffList[]             - List of Rx buffers to be freed.
* @param[in] buffListLen              - Length of rxBufList.
*                                       GT_OK on success, or
*                                       GT_FAIL otherwise.
*/
GT_STATUS cpssDxChNetIfMiiRxBufFree
(
    IN GT_U8    devNum,
    IN GT_U8    rxQueue,
    IN GT_U8    *rxBuffList[],
    IN GT_U32   buffListLen
);

/**
* @internal cpssDxChNetIfMiiInit function
* @endinternal
*
* @brief   Initialize the network interface MII/RGMII Ethernet port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] miiInitPtr               - (Pointer to) initialization parameters.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_CPU_MEM        - on memory allocation failure
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. Only single device can be connected to CPU by ethernet port, therefor
*       this API must be called only once.
*       2. The application must call only one of the following APIs per device:
*       cpssDxChNetIfMiiInit - for MII/RGMII Ethernet port networkIf initialization.
*       cpssDxChNetIfInit - for SDMA networkIf initialization.
*       In case more than one of the mentioned above API is called
*       GT_FAIL will be return.
*
*/
GT_STATUS cpssDxChNetIfMiiInit
(
    IN GT_U8                        devNum,
    IN CPSS_DXCH_NETIF_MII_INIT_STC *miiInitPtr
);


#ifdef __cplusplus
}
#endif

#endif  /* __cpssDxChNetIfh */

