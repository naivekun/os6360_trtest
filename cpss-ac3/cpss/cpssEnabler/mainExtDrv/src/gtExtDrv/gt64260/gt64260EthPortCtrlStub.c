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
* @file gt64260EthPortCtrlStub.c
*
* @brief This file includes stub functions for receiving packet buffers
* from upper layer, and sending them trough the PP's NIC.
*
* @version   2
********************************************************************************
*/

#include <gtOs/gtOsGen.h>
#include <gtExtDrv/drivers/gtEthPortCtrl.h>
/*#include <prestera/driver/interrupts/gtDriverInterrupts.h>*/

/**
* @internal extDrvEthPortRxInit function
* @endinternal
*
* @brief   Init the ethernet port Rx interface
*
* @param[in] rxBufPoolSize            - buffer pool size
* @param[in] rxBufPool_PTR            - the address of the pool
* @param[in] rxQNum                   - the number of RX queues
* @param[in] rxQbufPercentage[]       the buffer percentage dispersal for all queues
* @param[in] rxBufSize                - the buffer requested size
* @param[in,out] numOfRxBufs_PTR          - number of requested buffers, and actual buffers created
* @param[in] headerOffset             - packet header offset size
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
GT_STATUS extDrvEthPortRxInit
(
    IN GT_U32           rxBufPoolSize,
    IN GT_U8_PTR        rxBufPool_PTR,
    IN GT_U32           rxQNum,
    IN GT_U32           rxQbufPercentage[],
    IN GT_U32           rxBufSize,
    INOUT GT_U32        *numOfRxBufs_PTR,
    IN GT_U32           headerOffset
)
{
    return GT_FAIL;
}


/**
* @internal extDrvEthPortTxInit function
* @endinternal
*
* @brief   Init the ethernet port Tx interface
*
* @param[in] numOfTxBufs              - number of requested buffers
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
GT_STATUS extDrvEthPortTxInit
(
    IN GT_U32           numOfTxBufs
)
{
    return GT_FAIL;
}


/**
* @internal extDrvEthPortEnable function
* @endinternal
*
* @brief   Enable the ethernet port interface
*/
GT_STATUS extDrvEthPortEnable
(
    GT_VOID
)
{
    return GT_FAIL;
}


/**
* @internal extDrvEthPortDisable function
* @endinternal
*
* @brief   Disable the ethernet port interface
*/
GT_STATUS extDrvEthPortDisable
(
    GT_VOID
)
{
    return GT_FAIL;
}


/**
* @internal extDrvEthPortRxCb function
* @endinternal
*
* @brief   This function receives a packet from the ethernet port.
*
* @param[in] segmentList[]            - A list of pointers to the packets segments.
* @param[in] segmentLen[]             - A list of segement length.
* @param[in] numOfSegments            - The number of segment in segment list.
* @param[in] queueNum                 - the received queue number
*                                       GT_TRUE if successful, or
*                                       GT_FALSE otherwise.
*/
GT_STATUS extDrvEthPortRxCb
(
    IN GT_U8_PTR        segmentList[],
    IN GT_U32           segmentLen[],   
    IN GT_U32           numOfSegments,
    IN GT_U32           queueNum
)
{
    return GT_FAIL;
}


/**
* @internal extDrvEthPortTxEndCb function
* @endinternal
*
* @brief   This function receives a packet from the ethernet port.
*
* @param[in] segmentList[]            - A list of pointers to the packets segments.
* @param[in] numOfSegments            - The number of segment in segment list.
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
GT_STATUS extDrvEthPortTxEndCb
(
    IN GT_U8_PTR        segmentList[],
    IN GT_U32           numOfSegments
)
{
    return GT_FAIL;
}


/**
* @internal extDrvEthPortTx function
* @endinternal
*
* @brief   This function transmits a packet through the ethernet port in salsa.
*
* @param[in] segmentList[]            - A list of pointers to the packets segments.
* @param[in] segmentLen[]             - A list of segement length.
* @param[in] numOfSegments            - The number of segment in segment list.
* @param[in] txQueue                  - The TX queue.
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
GT_STATUS extDrvEthPortTx
(
    IN GT_U8_PTR        segmentList[],
    IN GT_U32           segmentLen[],
    IN GT_U32           numOfSegments,
    IN GT_U32           txQueue
)
{
    /* send the packet */
    return GT_FAIL;
}


/**
* @internal extDrvEthInputHookAdd function
* @endinternal
*
* @brief   This bind the user Rx callback
*
* @param[in] userRxFunc               - the user Rx callbak function
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
GT_STATUS extDrvEthInputHookAdd
(
    IN GT_Rx_FUNCPTR    userRxFunc
)
{
    return GT_FAIL;
}


/**
* @internal extDrvEthTxCompleteHookAdd function
* @endinternal
*
* @brief   This bind the user Tx complete callback
*
* @param[in] userTxFunc               - the user Tx callback function
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
GT_STATUS extDrvEthTxCompleteHookAdd
(
    IN GT_Tx_COMPLETE_FUNCPTR userTxFunc
)
{
    return GT_FAIL;
}


/**
* @internal extDrvEthRxPacketFree function
* @endinternal
*
* @brief   This routine frees the recievd Rx buffer.
*
* @param[in] segmentList[]            - A list of pointers to the packets segments.
* @param[in] numOfSegments            - The number of segment in segment list.
* @param[in] queueNum                 - receive queue number
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
GT_STATUS extDrvEthRxPacketFree
(
    IN GT_U8_PTR        segmentList[],
    IN GT_U32           numOfSegments,
    IN GT_U32           queueNum
)
{
    return GT_FAIL;
}

/**
* @internal extDrvEthPortTxModeSet function
* @endinternal
*
* @brief   Set the ethernet port tx mode
*
* @param[in] txMode                   - extDrvEthTxMode_asynch_E - don't wait for TX done - free packet
*                                      when interrupt received
*                                      extDrvEthTxMode_asynch_E - wait to TX done and free packet
*                                      immediately
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise
*                                       GT_NOT_SUPPORTED
*/
GT_STATUS extDrvEthPortTxModeSet
(
    extDrvEthTxMode_ENT    txMode
)
{
    return GT_NOT_SUPPORTED;
}


