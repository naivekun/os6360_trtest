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
* @file cpssDxChRxHandling.c
*
* @brief This file implements functions to allow application to receive RX
* packets from PP , and to free the resources when Rx packet's buffers not
* needed.
*
* @version   45
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/networkIf/private/prvCpssDxChNetworkIfLog.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/dxCh/dxChxGen/cos/private/prvCpssDxChCoS.h>

#include <cpss/generic/networkIf/cpssGenNetIfMii.h>
#include <cpss/generic/networkIf/private/prvCpssGenNetworkIfMii.h>

/*******************************************************************************
* External usage environment parameters
*******************************************************************************/

/*******************************************************************************
* Internal usage environment parameters
*******************************************************************************/
static GT_BOOL debug_dumpFullPacket = GT_FALSE;

/**
* @internal dxChNetIfSdmaRxBufFreeWithSize function
* @endinternal
*
* @brief   Frees a list of buffers, that where previously passed to the upper layer
*         in an Rx event. -- SDMA relate
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1_Diamond.
*
* @param[in] devNum                   - The device number through which these buffers where
*                                      received.
* @param[in] rxQueue                  - The Rx queue number through which these buffers where
*                                      received.
* @param[in] rxBuffList[]             - List of Rx buffers to be freed.
* @param[in] rxBuffSizeList[]         - List of Rx Buffers sizes , to set to the free
*                                      descriptor
*                                      if this parameter is NULL --> we ignore it.
* @param[in] buffListLen              - Length of rxBufList.
*                                       GT_OK on success, or
*                                       GT_FAIL otherwise.
*/
static GT_STATUS dxChNetIfSdmaRxBufFreeWithSize
(
    IN GT_U8    devNum,
    IN GT_U8    rxQueue,
    IN GT_U8    *rxBuffList[],
    IN GT_U32   rxBuffSizeList[],
    IN GT_U32   buffListLen
)
{
    GT_STATUS                        rc;
    PRV_CPSS_RX_DESC_LIST_STC       *rxDescList;
    PRV_CPSS_RX_DESC_STC            *rxDesc;
    GT_U32                          tmpData;
    GT_UINTPTR                      phyAddr;
    GT_U32                          i;
    PRV_CPSS_SW_RX_DESC_STC         *first2Return;

    /* we will use the 'First active port group' , to represent the whole device.
       that way we allow application to give SDMA memory to single port group instead
       of split it between all active port groups
    */

    rxDescList = &(PRV_CPSS_PP_MAC(devNum)->intCtrl.rxDescList[rxQueue]);

    first2Return = rxDescList->next2Return;
    for(i = 0; i < buffListLen; i++)
    {
        /* If rxDescList->rxDescList->next2Return equal to rxDescList->rxDescList->next2Receive
          and rxDescList->freeDescNum is bigger then 0 this means that all descriptors
          in the ring pointing to buffer, therefore we cannot add new buufer to the ring.
          This may indicate a problem in the application, that tries to free the buffers
          more then once before receiving it from PP.
         */
        if((rxDescList->freeDescNum > 0) &&
           (rxDescList->next2Return == rxDescList->next2Receive) )
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FULL, LOG_ERROR_NO_MSG);

        rxDescList->freeDescNum++;
        rc = cpssOsVirt2Phy((GT_UINTPTR)(rxBuffList[i]),&phyAddr);
        if (rc != GT_OK)
        {
            return rc;
        }
        #if __WORDSIZE == 64    /* phyAddr must fit in 32 bit */
            if (0 != (phyAddr & 0xffffffff00000000L))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }
        #endif

        tmpData = CPSS_32BIT_LE((GT_U32)phyAddr);
        rxDesc = rxDescList->next2Return->rxDesc;
        rxDesc->buffPointer = tmpData;

        if(i != 0)
        {/* first descriptor will be freed last */
            GT_SYNC;
            RX_DESC_RESET_MAC(devNum,rxDescList->next2Return->rxDesc);
            if(rxBuffSizeList)
            {
                RX_DESC_SET_BUFF_SIZE_FIELD_MAC(devNum,rxDescList->next2Return->rxDesc,rxBuffSizeList[i]);
            }
            GT_SYNC;
        }
        rxDescList->next2Return = rxDescList->next2Return->swNextDesc;
    }

    /* return first descriptor in chain to sdma ownership, it's done last to prevent multiple resource errors,
     * when jumbo packets arrive to cpu on high speed and cpu frees descriptors one by one in simple sequence
     * sdma immediately uses them and reaches cpu owned descriptor and again causes resource error
     */
    GT_SYNC;

    if(rxBuffSizeList)
    {
        RX_DESC_SET_BUFF_SIZE_FIELD_MAC(devNum, first2Return->rxDesc, rxBuffSizeList[0]);
    }
    RX_DESC_RESET_MAC(devNum, first2Return->rxDesc);
    GT_SYNC;

    return GT_OK;
}
/**
* @internal internal_cpssDxChNetIfRxBufFree function
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
static GT_STATUS internal_cpssDxChNetIfRxBufFree
(
    IN GT_U8    devNum,
    IN GT_U8    rxQueue,
    IN GT_U8    *rxBuffList[],
    IN GT_U32   buffListLen
)
{
    GT_U32                  firstBuffOffset = 0;/* offset to the actual start of
                                                   the FIRST buffer */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(rxBuffList);
    PRV_CPSS_CHECK_SDMA_Q_MAC(devNum,rxQueue);
    if(buffListLen == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /*check that the DB of networkIf library was initialized*/
    PRV_CPSS_DXCH_NETIF_INITIALIZED_CHECK_MAC(devNum);

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->netIf.allocMethod[rxQueue] == CPSS_RX_BUFF_NO_ALLOC_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    if((PRV_CPSS_PP_MAC(devNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_SDMA_E)
        && !PRV_CPSS_DXCH_CHECK_SDMA_PACKETS_FROM_CPU_CORRUPT_WA_MAC(devNum))
    {
        /* In DXCH devices CPSS always have at least 4 bytes on the first Rx
           buffer those are not used. Those bytes exist because the DXCH device
           receive every packet with DSA tag (8 or 16 bytes). And if the packet
           came untagged then CPSS remove all bytes of DSA tag (8 or 16 bytes).
           CPSS removes 4 bytes less then DSA tag (4 or 12 bytes) if the packet
           came tagged. The buffer's pointer is moved to number of removed bytes.
           The number of removed bytes are stored in the buffer before first
           byte of the packet. Use this info to get actual beginning of the buffer.
           See using of PRV_CPSS_DXCH_RX_ORIG_PCKT_OFFSET_SET_MAC. */
        if(PRV_CPSS_DXCH_DEV_MODULE_CFG_MAC(devNum)->useMultiNetIfSdma == GT_FALSE)
        {
            firstBuffOffset = PRV_CPSS_DXCH_RX_ORIG_PCKT_OFFSET_GET_MAC(rxBuffList[0],
                                PRV_CPSS_DXCH_DEV_MODULE_CFG_MAC(devNum)->netIfCfg.rxBufInfo.headerOffset);
        }
        else
        {
            firstBuffOffset = PRV_CPSS_DXCH_RX_ORIG_PCKT_OFFSET_GET_MAC(rxBuffList[0],
                                PRV_CPSS_DXCH_DEV_MODULE_CFG_MAC(devNum)->multiNetIfCfg.rxSdmaQueuesConfig[rxQueue/CPSS_MAX_RX_QUEUE_CNS][rxQueue%CPSS_MAX_RX_QUEUE_CNS].buffHeaderOffset);
        }

        /* update the buffer pointer to the actual start of the buffer */
        rxBuffList[0] -= firstBuffOffset;

        /* SDMA buffer free */
        return dxChNetIfSdmaRxBufFreeWithSize(devNum,rxQueue,rxBuffList,NULL,buffListLen);
    }
    else if((PRV_CPSS_PP_MAC(devNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_SDMA_E)
            && PRV_CPSS_DXCH_CHECK_SDMA_PACKETS_FROM_CPU_CORRUPT_WA_MAC(devNum))
    {
        return prvCpssGenNetIfMiiRxBufFree(devNum, rxQueue, rxBuffList, buffListLen);
    }
    else /* CPSS_NET_CPU_PORT_MODE_MII_E, CPSS_NET_CPU_PORT_MODE_NONE_E */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);

}

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
* @param[in] rxQueue                  - The Rx queue number through which these buffers where
*                                      received (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon 0..31).
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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNetIfRxBufFree);

    CPSS_TRAFFIC_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, rxQueue, rxBuffList, buffListLen));

    rc = internal_cpssDxChNetIfRxBufFree(devNum, rxQueue, rxBuffList, buffListLen);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, rxQueue, rxBuffList, buffListLen));
    CPSS_TRAFFIC_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChNetIfRxBufFreeWithSize function
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
static GT_STATUS internal_cpssDxChNetIfRxBufFreeWithSize
(
    IN GT_U8    devNum,
    IN GT_U8    rxQueue,
    IN GT_U8    *rxBuffList[],
    IN GT_U32   rxBuffSizeList[],
    IN GT_U32   buffListLen
)
{
    GT_U32 i;            /* Iterator */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(rxBuffList);
    CPSS_NULL_PTR_CHECK_MAC(rxBuffSizeList);
    PRV_CPSS_CHECK_SDMA_Q_MAC(devNum,rxQueue);
    if(buffListLen == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /*check that the DB of networkIf library was initialized*/
    PRV_CPSS_DXCH_NETIF_INITIALIZED_CHECK_MAC(devNum);

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->netIf.allocMethod[rxQueue] != CPSS_RX_BUFF_NO_ALLOC_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    if((PRV_CPSS_PP_MAC(devNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_SDMA_E)
        && !PRV_CPSS_DXCH_CHECK_SDMA_PACKETS_FROM_CPU_CORRUPT_WA_MAC(devNum))
    {
        for (i=0;i<buffListLen;i++)
        {
            /* check the buffer size*/
            if (rxBuffSizeList[i] % RX_BUFF_SIZE_MULT)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            /* check the buffer address alignment*/
            if ((GT_UINTPTR)rxBuffList[i] % RX_BUFF_ALIGN)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }

        return dxChNetIfSdmaRxBufFreeWithSize(devNum,rxQueue,rxBuffList,rxBuffSizeList,buffListLen);

    }
    else /* CPSS_NET_CPU_PORT_MODE_MII_E, CPSS_NET_CPU_PORT_MODE_NONE_E */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNetIfRxBufFreeWithSize);

    CPSS_TRAFFIC_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, rxQueue, rxBuffList, rxBuffSizeList, buffListLen));

    rc = internal_cpssDxChNetIfRxBufFreeWithSize(devNum, rxQueue, rxBuffList, rxBuffSizeList, buffListLen);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, rxQueue, rxBuffList, rxBuffSizeList, buffListLen));
    CPSS_TRAFFIC_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);

    return rc;
}

/**
* @internal dxChNetIfRxPacketGet function
* @endinternal
*
* @brief   This function returns packets from PP destined to the CPU port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] queueIdx                 - The queue from which this packet was received.
* @param[in,out] numOfBuffPtr             Num of buffs in packetBuffsArrPtr.
* @param[in] lastBufferSize           - last buffer size
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
* @retval GT_NOT_SUPPORTED         - on not supported DSA tag type.
* @retval GT_BAD_PARAM             - wrong devNum,portNum,queueIdx.
* @retval GT_DSA_PARSING_ERROR     - DSA tag parsing error.
*
* @note It is recommended to call cpssDxChNetIfRxBufFree for this queue
*       i.e. return the buffer to their original queue.
*       common function for both for the :
*       1) parse DMA packet
*       2) parse Eth port packet
*
*/
static GT_STATUS dxChNetIfRxPacketGet
(
    IN GT_U8                                devNum,
    IN GT_U8                                queueIdx,
    INOUT GT_U32                            *numOfBuffPtr,
    OUT GT_U8                               *packetBuffsArrPtr[],
    OUT GT_U32                              buffLenArr[],
    OUT CPSS_DXCH_NET_RX_PARAMS_STC         *rxParamsPtr,
    IN  GT_U32                              lastBufferSize
)
{
    GT_STATUS   rc;    /* Return Code                                  */
    GT_U32      i;          /* Iterator                                     */
    GT_U32      byte2Remove;/* Bytes to remove in order to remove DSA tag   */
    GT_U32      pcktOffset; /* Offset from the start of the buffer          */
    GT_U8       *buffPtr;   /* temporary buffer ptr                         */
    GT_U32      hdrOffset;   /* Packet header offset                        */
    GT_U8       *dsaTagPtr;  /* Extended DSA tag                            */
    GT_BOOL     srcOrDstIsTagged;/* send application the packet tagged or untagged */
    GT_U32      dsaOffset;/* offset of the DSA tag from start of packet */
    GT_U32      cpuPortNum;

    if(PRV_CPSS_DXCH_DEV_MODULE_CFG_MAC(devNum)->useMultiNetIfSdma == GT_FALSE)
    {
        hdrOffset = PRV_CPSS_DXCH_DEV_MODULE_CFG_MAC(devNum)->netIfCfg.rxBufInfo.headerOffset;
    }
    else
    {
        hdrOffset = PRV_CPSS_DXCH_DEV_MODULE_CFG_MAC(devNum)->multiNetIfCfg.rxSdmaQueuesConfig[queueIdx/CPSS_MAX_RX_QUEUE_CNS][queueIdx%CPSS_MAX_RX_QUEUE_CNS].buffHeaderOffset;
    }
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->netIf.prePendTwoBytesHeader == GT_TRUE)
    {
        /* the PP will put the start of the Ethernet header of the packet , in 2
           bytes offset , to allow the IP header of the packet to be 4 bytes
           aligned */

        dsaOffset = PRV_CPSS_DSA_TAG_PCKT_OFFSET_CNS + /* 12 */
                    PRV_CPSS_DXCH_IP_ALIGN_OFFSET_CNS; /* 2 */
    }
    else
    {
        dsaOffset = PRV_CPSS_DSA_TAG_PCKT_OFFSET_CNS; /* 12 */
    }

    if(debug_dumpFullPacket)
    {
        GT_U32 iter;
        GT_U32 offset = 0;
        GT_U32 jj;
        GT_U8*  bufferPtr = &packetBuffsArrPtr[0][0];
        GT_U32  length = buffLenArr[0];
        GT_U32  totalLen = 0;

        /* calc totalLen */
        for(jj = 0; jj < *numOfBuffPtr; jj++){
            totalLen += buffLenArr[jj];
        }

        if(queueIdx >= 8)
        {
            /* the test may send packet from the 'CPU' but not from queue 0..7
               that relate to 'port 63' ..
               so expected port may differ from 63  */
            rc = cpssDxChNetIfSdmaQueueToPhysicalPortGet(devNum,
                queueIdx,
                &cpuPortNum);
            if(rc != GT_OK)
            {
                cpuPortNum = 0xFFFFFFFF;/*error*/
            }
            cpssOsPrintf("dxChNetIfRxPacketGet: device[%d] cpuPortNum[%d] queue[%d] [%d] bytes (include DSA bytes !)\n",
                devNum,cpuPortNum,queueIdx,totalLen);
        }
        else
        {
            cpssOsPrintf("dxChNetIfRxPacketGet: device[%d] queue[%d] [%d] bytes (include DSA bytes !)\n",
                devNum,queueIdx,totalLen);
        }


        for(jj = 0; jj < *numOfBuffPtr; jj++){
            bufferPtr = &packetBuffsArrPtr[jj][0];
            length = buffLenArr[jj];

            for(iter = 0; iter < length; iter++)
            {
                if((offset & 0x0F) == 0)
                {
                    cpssOsPrintf("0x%4.4x :", offset);
                }
                cpssOsPrintf(" %2.2x", bufferPtr[iter]);

                if((offset & 0x0F) == 0x0F)
                {
                    cpssOsPrintf("\n");
                }

                offset++;
            }/*iter*/
        }
        cpssOsPrintf("\n");
    }

    /* get the 8 bytes of the extended DSA tag from the first buffer */
    dsaTagPtr = &(packetBuffsArrPtr[0][dsaOffset]);/*offset 12*/

    rc = cpssDxChNetIfDsaTagParse(devNum, dsaTagPtr , &rxParamsPtr->dsaParam);

    if(rc != GT_OK)
    {
        buffLenArr[(*numOfBuffPtr) -1] = lastBufferSize;
        (void)dxChNetIfSdmaRxBufFreeWithSize(devNum,queueIdx,packetBuffsArrPtr,buffLenArr,*numOfBuffPtr);
        return rc;
    }

    switch(rxParamsPtr->dsaParam.dsaType)
    {
        case CPSS_DXCH_NET_DSA_CMD_TO_CPU_E:
            srcOrDstIsTagged = rxParamsPtr->dsaParam.dsaInfo.toCpu.isTagged;
            break;
        case CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E:
            if(rxParamsPtr->dsaParam.dsaInfo.fromCpu.dstInterface.type == CPSS_INTERFACE_PORT_E)
            {
                srcOrDstIsTagged = rxParamsPtr->dsaParam.dsaInfo.fromCpu.extDestInfo.devPort.dstIsTagged;
            }
            else
            {
                srcOrDstIsTagged = GT_TRUE;
            }
            break;
        case CPSS_DXCH_NET_DSA_CMD_TO_ANALYZER_E:
            srcOrDstIsTagged = rxParamsPtr->dsaParam.dsaInfo.toAnalyzer.isTagged;
            break;
        case CPSS_DXCH_NET_DSA_CMD_FORWARD_E:
            srcOrDstIsTagged = rxParamsPtr->dsaParam.dsaInfo.forward.srcIsTagged;
            break;
        default:
            buffLenArr[(*numOfBuffPtr) -1] = lastBufferSize;
            rc = dxChNetIfSdmaRxBufFreeWithSize(devNum,queueIdx,packetBuffsArrPtr,buffLenArr,*numOfBuffPtr);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    /*************************************/
    /* remove the DSA tag from buffer[0] */
    /*************************************/
    /* need to remove the DSA tag length , but leave 4 bytes if packet can 'tagged'
       so we can override it with the vlan TAG */
    byte2Remove = ((rxParamsPtr->dsaParam.commonParams.dsaTagType + 1) * 4);

    if(GT_TRUE == srcOrDstIsTagged)
    {
        /* packet was tagged originally */
        byte2Remove -= 4 ;/* leave 4 bytes from the DSA tag to be able to override with TAG info */

        /* restore the "vlan tag style" to the needed bytes */
        buffPtr = (GT_U8 *)&packetBuffsArrPtr[0][dsaOffset];

        /* set 0x8100 */
        buffPtr[0] = (GT_U8)PRV_CPSS_VLAN_ETHR_TYPE_MSB_CNS;
        buffPtr[1] = (GT_U8)PRV_CPSS_VLAN_ETHR_TYPE_LSB_CNS;

        /* reset the cfiBit value */
        buffPtr[2] &= ~(1<<4);

        /* set the cfiBit -- according to info */
        buffPtr[2] |= ((rxParamsPtr->dsaParam.commonParams.cfiBit == 1) ? (1 << 4) : 0);

        /* jump after the vlan TAG */
        pcktOffset = dsaOffset + 4;/*16 or 18 */
    }
    else
    {
        /* after the mac addresses  */
        pcktOffset = dsaOffset;/*12 or 14 */
    }

    /* move the start of the data on the buffer , to point to the last
       byte of macs + optional vlan tag

       So point to macSa[byte 5] or to vlanTag[byte 3]
       */
    buffPtr = (GT_U8 *)&packetBuffsArrPtr[0][pcktOffset-1];

    /* copy from the last byte of needed data (macs + optional vlan tag)
       replacing removed DSA tag bytes */
    for(i = pcktOffset ; i; i-- , buffPtr--)
    {
        buffPtr[byte2Remove] = buffPtr[0];
    }

    /***************************************************************************
    *  from this point the "free buffers" in case of error must call function  *
    *  cpssDxChNetIfRxBufFree(...) and not to dxChNetIfRxBufFree(...)      *
    ***************************************************************************/


    packetBuffsArrPtr[0] = (buffPtr + 1) + byte2Remove;
    buffLenArr[0]     =  buffLenArr[0]   - byte2Remove;

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->netIf.allocMethod[queueIdx] != CPSS_RX_BUFF_NO_ALLOC_E)
    {
        /* Next code is for internal use :
           to know how to retrieve actual pointer to the buffer when free the buffer

           Note : this action of offset storage must be done after the
           packetBuffsArrPtr[0] is stable (the pointer is not changed any more)
        */
        PRV_CPSS_DXCH_RX_ORIG_PCKT_OFFSET_SET_MAC(packetBuffsArrPtr[0], hdrOffset, byte2Remove);
    }

    return GT_OK;
}


/**
* @internal dxChNetIfSdmaRxResErrPacketHandle function
* @endinternal
*
* @brief   This function free the descriptors of packets from PP destined to the
*         CPU port with Rx Resource Error. -- SDMA relate
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1_Diamond.
*
* @param[in] devNum                   - Device number.
* @param[in] queueIdx                 - The queue from which this packet was received.
*
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
static GT_STATUS dxChNetIfSdmaRxResErrPacketHandle
(
    IN GT_U8                               devNum,
    IN GT_U8                               queueIdx
)
{
    GT_STATUS rc = GT_OK;
    PRV_CPSS_DXCH_PP_REGS_ADDR_STC      *regsAddr;/* alias to register address*/
    PRV_CPSS_SW_RX_DESC_STC   *swRxDesc;    /* A pointer to a Rx descriptor   */
    PRV_CPSS_SW_RX_DESC_STC   *swN2fRxDesc; /* A pointer to the Next 2        */
                                            /* free Rx descriptor.            */
    PRV_CPSS_RX_DESC_LIST_STC        *rxDescList;    /* alias to queue descriptor list   */
    GT_U32         netIfNum;
    GT_U32      mgUnitId;/*the MG to serve the SDMA operation*/

    regsAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum);

    rxDescList = &(PRV_CPSS_PP_MAC(devNum)->intCtrl.rxDescList[queueIdx]);

    netIfNum = queueIdx>>3;
    /************************************/
    /* convert netIfNum to mgUnitId     */
    /************************************/
    PRV_CPSS_DXCH_NETIF_CHECK_NETIF_NUM_AND_CONVERT_TO_MG_UNIT_ID_MAC(devNum,netIfNum,mgUnitId);

    if (GT_FALSE == rxDescList->forbidQEn)
    {
        /* Disable the corresponding Queue. */
        rc = cpssDxChNetIfSdmaRxQueueEnable(devNum, queueIdx, GT_FALSE);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* Check that the SDMA is disabled. */
        /* wait for bit 0 to clear */
        rc = prvCpssHwPpMgRegBusyWaitByMask(
            devNum, mgUnitId,
            regsAddr->sdmaRegs.rxQCmdReg,(queueIdx&0x07),GT_FALSE);
        if(rc != GT_OK)
        {
            return rc;
        }

        rxDescList->forbidQEn = GT_TRUE;
    }

    /* set the SW descriptor shadow pointers */
    swRxDesc        = rxDescList->next2Receive;
    swN2fRxDesc     = rxDescList->next2Return;

    /* copy the Rx descriptor to SW shadow */
    swRxDesc->shadowRxDesc.word1 = CPSS_32BIT_LE(swRxDesc->rxDesc->word1);
    swRxDesc->shadowRxDesc.word2 = CPSS_32BIT_LE(swRxDesc->rxDesc->word2);

    /* Reset all descriptors with set resource error bit. */
    while (RX_DESC_GET_REC_ERR_BIT(&(swRxDesc->shadowRxDesc)) == 1 ||
           (RX_DESC_GET_OWN_BIT(&(swRxDesc->shadowRxDesc)) == RX_DESC_CPU_OWN &&
            RX_DESC_GET_FIRST_BIT(&(swRxDesc->shadowRxDesc)) != 1))
    {
        swN2fRxDesc->rxDesc->buffPointer = swRxDesc->rxDesc->buffPointer;

        /* copy buffer size */
        swN2fRxDesc->rxDesc->word2 = swRxDesc->rxDesc->word2;

        GT_SYNC;

        RX_DESC_RESET_MAC(devNum,swN2fRxDesc->rxDesc);

        GT_SYNC;

        swN2fRxDesc = swN2fRxDesc->swNextDesc;
        swRxDesc = swRxDesc->swNextDesc;
        swRxDesc->shadowRxDesc.word1 = CPSS_32BIT_LE(swRxDesc->rxDesc->word1);
        swRxDesc->shadowRxDesc.word2 = CPSS_32BIT_LE(swRxDesc->rxDesc->word2);
    }

    /* update the Rx desc list if error was detected */
    rxDescList->next2Receive = swRxDesc;
    rxDescList->next2Return  = swN2fRxDesc;

    /* Enable the Rx SDMA only if there are free descriptors in the Rx queue. */
    if (rxDescList->freeDescNum > 0)
    {
        rc = cpssDxChNetIfSdmaRxQueueEnable(devNum, queueIdx, GT_TRUE);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    rxDescList->forbidQEn = GT_FALSE;

    return GT_OK;
}

/**
* @internal internal_cpssDxChNetIfSdmaRxPacketGet function
* @endinternal
*
* @brief   This function returns packets from PP destined to the CPU port.
*         -- SDMA relate.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1_Diamond.
*
* @param[in] devNum                   - Device number.
* @param[in] queueIdx                 - The queue from which this packet was received (APPLICABLE RANGES: 0..7).
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
static GT_STATUS internal_cpssDxChNetIfSdmaRxPacketGet
(
    IN GT_U8                                devNum,
    IN GT_U8                                queueIdx,
    INOUT GT_U32                            *numOfBuffPtr,
    OUT GT_U8                               *packetBuffsArrPtr[],
    OUT GT_U32                              buffLenArr[],
    OUT CPSS_DXCH_NET_RX_PARAMS_STC         *rxParamsPtr
)
{
    GT_U32              i;              /* iterator                         */
    GT_STATUS           rc;             /* return code                      */
    PRV_CPSS_DXCH_MODULE_CONFIG_STC *moduleCfgPtr; /* pointer to module configuration */
    PRV_CPSS_RX_DESC_LIST_STC   *rxDescList;
    PRV_CPSS_SW_RX_DESC_STC   *firstRxDesc;   /* First Rx descriptor        */
    PRV_CPSS_SW_RX_DESC_STC   *swRxDesc;      /* Rx descriptor              */
    GT_U32              descNum;        /* Num of desc this packet occupies */
    GT_U32              packetLen;      /* Length of packet in bytes        */
    GT_U32              temp;           /* temporary word                   */
    GT_UINTPTR          virtAddr;
    GT_U8               *tempBufferPtr;
    GT_U32              tempBufferSize = 0; /* Size of a single buffer.         */
    GT_BOOL             buffersInCachedMem;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    /* Diamond Cut have not SDMA registers */

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_DIAMOND_E);

    PRV_CPSS_CHECK_SDMA_Q_MAC(devNum,queueIdx);
    CPSS_NULL_PTR_CHECK_MAC(numOfBuffPtr);
    CPSS_NULL_PTR_CHECK_MAC(packetBuffsArrPtr);
    CPSS_NULL_PTR_CHECK_MAC(buffLenArr);
    CPSS_NULL_PTR_CHECK_MAC(rxParamsPtr);

    /*check that the DB of networkIf library was initialized*/
    PRV_CPSS_DXCH_NETIF_INITIALIZED_CHECK_MAC(devNum);

    moduleCfgPtr = PRV_CPSS_DXCH_DEV_MODULE_CFG_MAC(devNum);

    if((PRV_CPSS_PP_MAC(devNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_SDMA_E)
        && !PRV_CPSS_DXCH_CHECK_SDMA_PACKETS_FROM_CPU_CORRUPT_WA_MAC(devNum))
    {
        firstRxDesc = NULL;
        rxDescList = &(PRV_CPSS_PP_MAC(devNum)->intCtrl.rxDescList[queueIdx]);

        if(rxDescList->freeDescNum == 0)
        {
            *numOfBuffPtr = 0;
            return /* it's not error for log */ GT_NO_MORE;
        }

        swRxDesc = rxDescList->next2Receive;

        swRxDesc->shadowRxDesc.word1 = CPSS_32BIT_LE(swRxDesc->rxDesc->word1);
        swRxDesc->shadowRxDesc.word2 = CPSS_32BIT_LE(swRxDesc->rxDesc->word2);

        /* Handle resource error if happened. */
        if ( RX_DESC_GET_REC_ERR_BIT(&(swRxDesc->shadowRxDesc)) == 1 ||
             ( RX_DESC_GET_OWN_BIT(&(swRxDesc->shadowRxDesc)) == RX_DESC_CPU_OWN &&
               RX_DESC_GET_FIRST_BIT(&(swRxDesc->shadowRxDesc)) != 1))
        {
            /* handle SDMA error */
            dxChNetIfSdmaRxResErrPacketHandle(devNum,queueIdx);

            /* update the shadow pointer , due to SDMA error */
            swRxDesc = rxDescList->next2Receive;

            swRxDesc->shadowRxDesc.word1 = CPSS_32BIT_LE(swRxDesc->rxDesc->word1);
            swRxDesc->shadowRxDesc.word2 = CPSS_32BIT_LE(swRxDesc->rxDesc->word2);
        }

        /* No more Packets to process, return */
        if(RX_DESC_GET_OWN_BIT(&(swRxDesc->shadowRxDesc)) != RX_DESC_CPU_OWN)
        {
            *numOfBuffPtr = 0;
            return /* it's not error for log */ GT_NO_MORE;
        }

        descNum     = 1;
        firstRxDesc = swRxDesc;

        /* Get the packet's descriptors.        */
        while(RX_DESC_GET_LAST_BIT(&(swRxDesc->shadowRxDesc)) == 0)
        {
            swRxDesc = swRxDesc->swNextDesc;
            swRxDesc->shadowRxDesc.word1 = CPSS_32BIT_LE(swRxDesc->rxDesc->word1);
            swRxDesc->shadowRxDesc.word2 = CPSS_32BIT_LE(swRxDesc->rxDesc->word2);
            descNum++;
        }

        rxDescList->next2Receive = swRxDesc->swNextDesc;
        rxDescList->freeDescNum -= descNum;

        swRxDesc = firstRxDesc;

        /* Validate that the given packet array length is big enough. */
        if (descNum > *numOfBuffPtr)
        {
            /* Drop the packet */
            /* Free all buffers - free buffers one by one */
            for(i = 0; i < descNum ; i++)
            {
                temp = swRxDesc->rxDesc->buffPointer;
                cpssOsPhy2Virt((GT_UINTPTR)(CPSS_32BIT_LE(temp)), &virtAddr);
                tempBufferPtr = (GT_U8*)virtAddr;
                tempBufferSize = RX_DESC_GET_BUFF_SIZE_FIELD_MAC(&(swRxDesc->shadowRxDesc));

                rc = dxChNetIfSdmaRxBufFreeWithSize(devNum,queueIdx,&tempBufferPtr,&tempBufferSize,1);
                swRxDesc = swRxDesc->swNextDesc;
            }

            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_SIZE, LOG_ERROR_NO_MSG);
        }

        packetLen   = RX_DESC_GET_BYTE_COUNT_FIELD(&(firstRxDesc->shadowRxDesc));

        /* build the array of buffers , and array of buffer length*/
        for(i = 0; i < descNum ; i++)
        {
            tempBufferSize = RX_DESC_GET_BUFF_SIZE_FIELD_MAC(&(swRxDesc->shadowRxDesc));
            if(packetLen > tempBufferSize)
            {
                buffLenArr[i] = tempBufferSize;
            }
            else
            {
                buffLenArr[i] = packetLen;
            }
            packetLen -= buffLenArr[i];

            temp = swRxDesc->rxDesc->buffPointer;
            cpssOsPhy2Virt((GT_UINTPTR)(CPSS_32BIT_LE(temp)), &virtAddr);
            packetBuffsArrPtr[i] = (GT_U8*)virtAddr;

            if(PRV_CPSS_DXCH_DEV_MODULE_CFG_MAC(devNum)->useMultiNetIfSdma == GT_FALSE)
            {
                buffersInCachedMem = moduleCfgPtr->netIfCfg.rxBufInfo.buffersInCachedMem;
            }
            else
            {
                buffersInCachedMem = PRV_CPSS_DXCH_DEV_MODULE_CFG_MAC(devNum)->multiNetIfCfg.rxSdmaQueuesConfig[queueIdx/CPSS_MAX_RX_QUEUE_CNS][queueIdx%CPSS_MAX_RX_QUEUE_CNS].buffersInCachedMem;
            }
            /* Invalidate data cache for cached buffer */
            if(GT_TRUE == buffersInCachedMem)
            {
                /* invalidate data cache */
                cpssExtDrvMgmtCacheInvalidate(CPSS_MGMT_DATA_CACHE_E, packetBuffsArrPtr[i], buffLenArr[i]);
            }

            swRxDesc  = swRxDesc->swNextDesc;
        }

        /* update the actual number of buffers in packet */
        *numOfBuffPtr = descNum;

        return dxChNetIfRxPacketGet(devNum, queueIdx, numOfBuffPtr, packetBuffsArrPtr, buffLenArr, rxParamsPtr,tempBufferSize);
    }
    else if((PRV_CPSS_PP_MAC(devNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_SDMA_E)
            && PRV_CPSS_DXCH_CHECK_SDMA_PACKETS_FROM_CPU_CORRUPT_WA_MAC(devNum))
    {
        rc = prvCpssDxChNetIfMiiRxGet(devNum, queueIdx, numOfBuffPtr, packetBuffsArrPtr,
                                             buffLenArr, rxParamsPtr);
        if (rc == GT_OK)
        {
            /* In SDMA CRC is not calculated, */
            /* instead 0x55555555 constant is used. */
            /* This code emulates SDMA behavior. */
            temp = buffLenArr[*numOfBuffPtr - 1];
            packetBuffsArrPtr[*numOfBuffPtr - 1][temp-1] = 0x55;
            packetBuffsArrPtr[*numOfBuffPtr - 1][temp-2] = 0x55;
            packetBuffsArrPtr[*numOfBuffPtr - 1][temp-3] = 0x55;
            packetBuffsArrPtr[*numOfBuffPtr - 1][temp-4] = 0x55;
        }
        return rc;
    }

    /* should not arrive here */
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
}

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
* @param[in] queueIdx                 - The queue from which this packet was received (APPLICABLE RANGES: 0..7).
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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNetIfSdmaRxPacketGet);

    CPSS_TRAFFIC_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, queueIdx, numOfBuffPtr, packetBuffsArrPtr, buffLenArr, rxParamsPtr));

    rc = internal_cpssDxChNetIfSdmaRxPacketGet(devNum, queueIdx, numOfBuffPtr, packetBuffsArrPtr, buffLenArr, rxParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, queueIdx, numOfBuffPtr, packetBuffsArrPtr, buffLenArr, rxParamsPtr));
    CPSS_TRAFFIC_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChNetIfPrePendTwoBytesHeaderSet function
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
static GT_STATUS internal_cpssDxChNetIfPrePendTwoBytesHeaderSet
(
    IN  GT_U8                           devNum,
    IN  GT_BOOL                         enable
)
{
    GT_STATUS   rc;
    GT_U32  regAddr;         /* The register address to write to.    */
    GT_U32  data;
    GT_U32  bitIndex;/* bit index*/
    GT_U32  sdmaByMiiNeeded;/* is the SDMA by MII emulation needed ?
                                0 - not
                                1 - yes */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    /* NOTE: in Bobcat2, Caelum, Bobcat3 the config is per 'physical port' */

    sdmaByMiiNeeded = PRV_CPSS_DXCH_CHECK_SDMA_PACKETS_FROM_CPU_CORRUPT_WA_MAC(devNum);
#ifdef ASIC_SIMULATION
    /* This WA should not be performed in simulation */
    sdmaByMiiNeeded = 0;
#endif /*ASIC_SIMULATION*/

    if (0 == sdmaByMiiNeeded)
    {
        data = (enable == GT_TRUE) ? 1 : 0;

        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            rc = prvCpssDxChWriteTableEntryField(devNum,
                                                  CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
                                                  CPSS_CPU_PORT_NUM_CNS,/*global port*/
                                                  PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                                  SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_PRE_PEND_TWO_BYTES_HEADER_EN_E, /* field name */
                                                  PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                                  data);
        }
        else
        {
            if (0 == PRV_CPSS_DXCH_LION_FAMILY_CHECK_MAC(devNum))
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.cscdHeadrInsrtConf[0];
                bitIndex = 28;
            }
            else
            {
                if(PRV_CPSS_PP_MAC(devNum)->devFamily >= CPSS_PP_FAMILY_DXCH_LION2_E)
                {
                    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.txdmaBufMemMiscTresholdsCfg;
                    bitIndex = 26;
                }
                else
                {
                    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.hdrAltGlobalConfig;
                    bitIndex = 15;
                }
            }

            rc = prvCpssHwPpSetRegField(devNum,regAddr,bitIndex,1,data);
        }
    }
    else
    {
        /* In case of SDMA emulation do not prepend on PP side, */
        /* this will disable DSA tag recognition on CPU side */
        /* Make the IP alignment on CPU side */
        rc = cpssExtDrvEthPrePendTwoBytesHeaderSet(enable);
    }

    if(rc == GT_OK)
    {
        /* save info to the DB */
        PRV_CPSS_DXCH_PP_MAC(devNum)->netIf.prePendTwoBytesHeader = enable;
    }

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNetIfPrePendTwoBytesHeaderSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChNetIfPrePendTwoBytesHeaderSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);

    return rc;
}

/**
* @internal prvCpssDxChNetIfPrePendTwoBytesHeaderFromHwGet function
* @endinternal
*
* @brief   Geg from HW the value (not from DB)
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
GT_STATUS prvCpssDxChNetIfPrePendTwoBytesHeaderFromHwGet
(
    IN  GT_U8        devNum,
    OUT  GT_BOOL    *enablePtr
)
{
    GT_STATUS   rc;
    GT_U32  regAddr;         /* The register address to write to.    */
    GT_U32  data;
    GT_U32  bitIndex;/* bit index*/

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /*  NOTE :
        this function NOT care about 'PRV_CPSS_DXCH_CHECK_SDMA_PACKETS_FROM_CPU_CORRUPT_WA_MAC'
        because in 'emulated' mode. the CPSS did not write to HW.
        so we can read the HW any way !!!
    */


    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        rc = prvCpssDxChReadTableEntryField(devNum,
                                              CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
                                              CPSS_CPU_PORT_NUM_CNS,/*global port*/
                                              PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                              SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_PRE_PEND_TWO_BYTES_HEADER_EN_E, /* field name */
                                              PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                              &data);
    }
    else
    {
        if (0 == PRV_CPSS_DXCH_LION_FAMILY_CHECK_MAC(devNum))
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.cscdHeadrInsrtConf[0];
            bitIndex = 28;
        }
        else
        {
            if(PRV_CPSS_PP_MAC(devNum)->devFamily >= CPSS_PP_FAMILY_DXCH_LION2_E)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.txdmaBufMemMiscTresholdsCfg;
                bitIndex = 26;
            }
            else
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.hdrAltGlobalConfig;
                bitIndex = 15;
            }
        }

        rc = prvCpssHwPpGetRegField(devNum,regAddr,bitIndex,1,&data);
    }
    *enablePtr = BIT2BOOL_MAC(data);

    return rc;
}


/**
* @internal internal_cpssDxChNetIfPrePendTwoBytesHeaderGet function
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
static GT_STATUS internal_cpssDxChNetIfPrePendTwoBytesHeaderGet
(
    IN  GT_U8        devNum,
    OUT  GT_BOOL    *enablePtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    /* get info from the DB */
    *enablePtr = PRV_CPSS_DXCH_PP_MAC(devNum)->netIf.prePendTwoBytesHeader;
    return GT_OK;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNetIfPrePendTwoBytesHeaderGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChNetIfPrePendTwoBytesHeaderGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChNetIfSdmaRxCountersGet function
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
static GT_STATUS internal_cpssDxChNetIfSdmaRxCountersGet
(
    IN    GT_U8      devNum,
    IN    GT_U8      queueIdx,
    OUT   CPSS_DXCH_NET_SDMA_RX_COUNTERS_STC *rxCountersPtr
)
{
    GT_U32  regAddr;        /* The register address to write to.    */
    GT_U32  data;           /* Data read from the register.         */
    GT_STATUS rc = GT_OK;
    GT_U32  netIfNum;
    GT_U32      mgUnitId;/*the MG to serve the SDMA operation*/


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(rxCountersPtr);
    PRV_CPSS_CHECK_SDMA_Q_MAC(devNum,queueIdx);

    if((PRV_CPSS_PP_MAC(devNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_SDMA_E)
        && !PRV_CPSS_DXCH_CHECK_SDMA_PACKETS_FROM_CPU_CORRUPT_WA_MAC(devNum))
    {
        /* Diamond Cut have not SDMA registers */

        PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_DIAMOND_E);

        netIfNum    = queueIdx>>3;
        /************************************/
        /* convert netIfNum to mgUnitId     */
        /************************************/
        PRV_CPSS_DXCH_NETIF_CHECK_NETIF_NUM_AND_CONVERT_TO_MG_UNIT_ID_MAC(devNum,netIfNum,mgUnitId);

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->sdmaRegs.rxDmaPcktCnt[queueIdx&0x07];
        rc = prvCpssHwPpMgReadReg(
            devNum, mgUnitId, regAddr, &data);
        if(rc != GT_OK)
            return rc;
        rxCountersPtr->rxInPkts = data;
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->sdmaRegs.rxDmaByteCnt[queueIdx&0x07];
        rc = prvCpssHwPpMgReadReg(
            devNum,  mgUnitId, regAddr, &data);
        if(rc != GT_OK)
            return rc;
        rxCountersPtr->rxInOctets = data;
    }
    else if((PRV_CPSS_PP_MAC(devNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_SDMA_E)
            && PRV_CPSS_DXCH_CHECK_SDMA_PACKETS_FROM_CPU_CORRUPT_WA_MAC(devNum))
    {
        rc = prvCpssGenNetIfMiiRxQueueCountersGet(queueIdx,
                                                  &(rxCountersPtr->rxInPkts),
                                                  &(rxCountersPtr->rxInOctets));
        if (rc != GT_OK)
        {
            return rc;
        }

        /* emulate ROC */
        return prvCpssGenNetIfMiiRxQueueCountersClear(queueIdx);

    }
    else /* CPSS_NET_CPU_PORT_MODE_MII_E, CPSS_NET_CPU_PORT_MODE_NONE_E */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);

    return GT_OK;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNetIfSdmaRxCountersGet);

    CPSS_TRAFFIC_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, queueIdx, rxCountersPtr));

    rc = internal_cpssDxChNetIfSdmaRxCountersGet(devNum, queueIdx, rxCountersPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, queueIdx, rxCountersPtr));
    CPSS_TRAFFIC_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChNetIfSdmaRxErrorCountGet function
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
static GT_STATUS internal_cpssDxChNetIfSdmaRxErrorCountGet
(
    IN GT_U8    devNum,
    OUT CPSS_NET_SDMA_RX_ERROR_COUNTERS_STC  *rxErrCountPtr
)
{
    GT_U32      regAddr;    /* The register address to read from.   */
    GT_U32      data;       /* Data read from register.             */
    GT_U32      ii, numOfNetIfs;
    GT_STATUS   rc = GT_OK;
    GT_U32      mgUnitId;/*the MG to serve the SDMA operation*/

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(rxErrCountPtr);

    if((PRV_CPSS_PP_MAC(devNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_SDMA_E)
        && !PRV_CPSS_DXCH_CHECK_SDMA_PACKETS_FROM_CPU_CORRUPT_WA_MAC(devNum))
    {
        /* Diamond Cut have not SDMA registers */

        PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_DIAMOND_E);

        if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE ||
            CPSS_PP_FAMILY_DXCH_XCAT3_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
        {
            rc = prvCpssDxChNetIfMultiNetIfNumberGet(devNum, &numOfNetIfs);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
            /* get the counters of Queues 0..31 */
            for(ii = 0; ii < (CPSS_TC_RANGE_CNS*numOfNetIfs); ii++)
            {
                /******************************/
                /* convert ii to mgUnitId     */
                /******************************/
                PRV_CPSS_DXCH_NETIF_CHECK_NETIF_NUM_AND_CONVERT_TO_MG_UNIT_ID_MAC(devNum,(ii>>3),mgUnitId);

                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->sdmaRegs.rxSdmaResourceErrorCountAndMode[ii&0x07];
                rc = prvCpssHwPpMgReadReg(
                    devNum,  mgUnitId, regAddr, &data);
                if(rc != GT_OK)
                    return rc;
                rxErrCountPtr->counterArray[ii] = U32_GET_FIELD_MAC(data, 0, 8);
            }
        }
        else
        {
        /***************************************************************************
           the registers are "Clear on read" (Read only Clear - ROC)
           but the  counters are only 8 bits , and 4 queue counters in single
           register  -> so read for one counter will reset the values for the other
           3 counters in the register --> that is why the API return all 8 queue
           counters in single action !
        ***************************************************************************/

            /******************************/
            /* convert 0 to mgUnitId     */
            /******************************/
            PRV_CPSS_DXCH_NETIF_CHECK_NETIF_NUM_AND_CONVERT_TO_MG_UNIT_ID_MAC(devNum,0,mgUnitId);

            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->sdmaRegs.rxDmaResErrCnt[0];
            rc = prvCpssHwPpMgReadReg(devNum, mgUnitId,regAddr, &data);
            if(rc != GT_OK)
                return rc;
            /* set the counters of Queues 0..3 */
            for(ii = 0 ; ii < (CPSS_TC_RANGE_CNS / 2) ; ii++)
            {
                rxErrCountPtr->counterArray[ii] = U32_GET_FIELD_MAC(data,8*ii,8);
            }

            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->sdmaRegs.rxDmaResErrCnt[1];
            rc = prvCpssHwPpMgReadReg(devNum, mgUnitId,regAddr, &data);
            if(rc != GT_OK)
                return rc;
            /* set the counters of Queues 4..7 */
            for(ii = 0 ; ii < (CPSS_TC_RANGE_CNS / 2) ; ii++)
            {
                rxErrCountPtr->counterArray[(ii + 4)] = U32_GET_FIELD_MAC(data,8*ii,8);
            }
        }

    }
    else if((PRV_CPSS_PP_MAC(devNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_SDMA_E)
            && PRV_CPSS_DXCH_CHECK_SDMA_PACKETS_FROM_CPU_CORRUPT_WA_MAC(devNum))
    {
        for(ii = 0 ; ii < CPSS_TC_RANGE_CNS; ii++)
            rxErrCountPtr->counterArray[ii] = 0;
    }
    else /* CPSS_NET_CPU_PORT_MODE_MII_E, CPSS_NET_CPU_PORT_MODE_NONE_E */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);

    return GT_OK;
}

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
    IN GT_U8    devNum,
    OUT CPSS_NET_SDMA_RX_ERROR_COUNTERS_STC  *rxErrCountPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNetIfSdmaRxErrorCountGet);

    CPSS_TRAFFIC_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, rxErrCountPtr));

    rc = internal_cpssDxChNetIfSdmaRxErrorCountGet(devNum, rxErrCountPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, rxErrCountPtr));
    CPSS_TRAFFIC_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChNetIfSdmaRxQueueEnable function
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
static GT_STATUS internal_cpssDxChNetIfSdmaRxQueueEnable
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           queue,
    IN  GT_BOOL                         enable
)
{
    GT_U32  regAddr;        /* The register address to write to.    */
    GT_U32  data;
    GT_U32  netif, numOfNetIfs = 1;
    GT_STATUS      rc;
    GT_U32      mgUnitId;/*the MG to serve the SDMA operation*/
    GT_BOOL         toCpuQueueDisableErr;
    CPSS_DXCH_NET_RESOURCE_ERROR_MODE_ENT   rxResourceMode;

    PRV_CPSS_DXCH_MODULE_CONFIG_STC *moduleCfgPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_CHECK_SDMA_Q_MAC(devNum,queue);

    /* Diamond Cut have not SDMA registers */

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_DIAMOND_E);

    toCpuQueueDisableErr =
        PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
        PRV_CPSS_DXCH_ALDRIN2_QUEUE_VALUE_DISABLE_WA_E);

    /* WA - When disabling a queue, the application must
     * change Retry mode to Abort mode using the
     * cpssDxChNetIfSdmaRxResourceErrorModeSet API. */
    if (toCpuQueueDisableErr)
    {
        if(enable == GT_FALSE)
        {
            rc = cpssDxChNetIfSdmaRxResourceErrorModeGet(devNum, queue,
                    &rxResourceMode);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }

            if(rxResourceMode != CPSS_DXCH_NET_RESOURCE_ERROR_MODE_ABORT_E)
            {
                /* Set this flag to indicate if restore required */
                PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
                    info_PRV_CPSS_DXCH_ALDRIN2_QUEUE_VALUE_DISABLE_WA_E.restoreRequired[queue] = GT_TRUE;

                rc = cpssDxChNetIfSdmaRxResourceErrorModeSet(devNum, queue,
                        CPSS_DXCH_NET_RESOURCE_ERROR_MODE_ABORT_E);
                if(rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }
            }
        }
        /* Restore Errata changes */
        else if(PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
                    info_PRV_CPSS_DXCH_ALDRIN2_QUEUE_VALUE_DISABLE_WA_E.restoreRequired)
        {
            rc = cpssDxChNetIfSdmaRxResourceErrorModeSet(devNum, queue,
                    CPSS_DXCH_NET_RESOURCE_ERROR_MODE_RETRY_E);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }
    }

    netif = queue>>3;
    moduleCfgPtr = PRV_CPSS_DXCH_DEV_MODULE_CFG_MAC(devNum);
    if(moduleCfgPtr->useMultiNetIfSdma == GT_TRUE)
    {

        rc = prvCpssDxChNetIfMultiNetIfNumberGet(devNum, &numOfNetIfs);
        if(rc != GT_OK)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

    }
    if(netif >= numOfNetIfs)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Queue %u dosnt exist. number of network interfaces is %u",queue,numOfNetIfs);
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->sdmaRegs.rxQCmdReg;

    data = (1 << (queue&0x07));
    if (enable != GT_TRUE)
    {
        data <<= 8;
    }

    /************************************/
    /* convert netIfNum to mgUnitId     */
    /************************************/
    PRV_CPSS_DXCH_NETIF_CHECK_NETIF_NUM_AND_CONVERT_TO_MG_UNIT_ID_MAC(devNum,netif,mgUnitId);

    return prvCpssHwPpMgWriteReg(
            devNum,  mgUnitId, regAddr, data);
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNetIfSdmaRxQueueEnable);

    CPSS_TRAFFIC_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, queue, enable));

    rc = internal_cpssDxChNetIfSdmaRxQueueEnable(devNum, queue, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, queue, enable));
    CPSS_TRAFFIC_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChNetIfSdmaRxQueueEnableGet function
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
static GT_STATUS internal_cpssDxChNetIfSdmaRxQueueEnableGet
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           queue,
    OUT GT_BOOL                         *enablePtr
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;     /* The register address to write to.    */
    GT_U32      hwData, hwValue, netIfNum;
    GT_U32      mgUnitId;/*the MG to serve the SDMA operation*/


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_CHECK_SDMA_Q_MAC(devNum,queue);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* Diamond Cut have not SDMA registers */
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_DIAMOND_E);

    netIfNum = queue>>3;

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->sdmaRegs.rxQCmdReg;

    /************************************/
    /* convert netIfNum to mgUnitId     */
    /************************************/
    PRV_CPSS_DXCH_NETIF_CHECK_NETIF_NUM_AND_CONVERT_TO_MG_UNIT_ID_MAC(devNum,netIfNum,mgUnitId);

    rc = prvCpssHwPpMgReadReg(
        devNum,  mgUnitId, regAddr, &hwValue);
    if(GT_OK != rc)
    {
        return rc;
    }

    hwData = hwValue & 0xFFFF;

    /* Need check only RxENQ[7:0] bit to get Rx status.
      The RX SDMA is not active when RxENQ[7:0] is 0. */
    if((hwData >> (queue&0x07)) & 1)
    {
        *enablePtr = GT_TRUE;
    }
    else
    {
        *enablePtr = GT_FALSE;
    }

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNetIfSdmaRxQueueEnableGet);

    CPSS_TRAFFIC_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, queue, enablePtr));

    rc = internal_cpssDxChNetIfSdmaRxQueueEnableGet(devNum, queue, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, queue, enablePtr));
    CPSS_TRAFFIC_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);

    return rc;
}

/* debug function to allow print (dump) of the Rx Packet include DSA tag. (before DSA tag is removed) */
void dxChNetIfRxPacketParse_DebugDumpEnable
(
    IN GT_BOOL  enableDumpRxPacket
)
{
    debug_dumpFullPacket = enableDumpRxPacket;
}

