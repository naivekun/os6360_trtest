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
* @file gtEthPortCtrl.c
*
* @brief This file includes all needed functions for receiving packet buffers
* from upper layer, and sending them trough the PP's NIC.
*
* @version   8
********************************************************************************
*/

#include <gtOs/gtOsGen.h>
#include <gtExtDrv/drivers/gtEthPortCtrl.h>

/**
* @enum UNIMAC_PORT_SMI_MODE_ENT
 *
 * @brief SMI configuration mode for the UniMAC port
 * ENHANCED_SMI_DISNABLE_E - disable SMI enhanced mode
 * ENHANCED_SMI_ENABLE_E  - enable SMI enhanced mode
*/
typedef enum{

    ENHANCED_SMI_DISNABLE_E     = 0,

    ENHANCED_SMI_ENABLE_E       = 1

} UNIMAC_PORT_SMI_MODE_ENT;


extern GT_STATUS gtEthernetInterfaceRxInit
(
    IN GT_U32           rxBufPoolSize,
    IN GT_U8_PTR        rxBufPool_PTR,
    IN GT_U32           rxBufSize,
    INOUT GT_U32        *numOfRxBufs,
    IN GT_U32           headerOffset,
    IN GT_U32           rxQNum,
    IN GT_U32           rxQbufPercentage[]
);
extern GT_STATUS gtEthernetInterfaceTxInit
(
    IN GT_U32           numOfTxBufs
);
extern GT_STATUS gtEthernetInterfaceEnable(GT_VOID);
extern GT_STATUS gtEthernetInterfaceDisable(GT_VOID);
extern GT_STATUS gtEthernetPacketSend
(
    IN GT_U8_PTR        segmentList[],
    IN GT_U32           segmentLen[],
    IN GT_U32           numOfSegments
);
extern GT_STATUS gtEthernetRxCallbackBind
(
    IN GT_Rx_FUNCPTR    userRxFunc
);
extern GT_STATUS gtEthernetTxCompleteCallbackBind
(
    IN GT_Tx_COMPLETE_FUNCPTR userTxFunc
);
extern GT_STATUS gtEthernetRxPacketFree
(
    IN GT_U8_PTR        segmentList[],
    IN GT_U32           numOfSegments,
    IN GT_U32           queueNum
);
extern GT_VOID miiSmiIfSetMode(GT_U32 mode);


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
    return gtEthernetInterfaceRxInit(rxBufPoolSize, rxBufPool_PTR, rxBufSize,
                                     numOfRxBufs_PTR, headerOffset,
                                     rxQNum, rxQbufPercentage);
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
    return gtEthernetInterfaceTxInit(numOfTxBufs);
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
    GT_STATUS rc;

    rc = gtEthernetInterfaceEnable();

    if (GT_OK != rc)
    {
        return rc;
    }

    /* set the UniMAC SMI mode to enhanced */
    miiSmiIfSetMode(ENHANCED_SMI_ENABLE_E);

    return GT_OK;
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
    return gtEthernetInterfaceDisable();
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
    return GT_NOT_IMPLEMENTED;
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
    return GT_NOT_IMPLEMENTED;
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
    return gtEthernetPacketSend(segmentList, segmentLen, numOfSegments);
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
    return gtEthernetRxCallbackBind((GT_Rx_FUNCPTR) userRxFunc);
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
    return gtEthernetTxCompleteCallbackBind((GT_Tx_COMPLETE_FUNCPTR) userTxFunc);
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
    return gtEthernetRxPacketFree(segmentList, numOfSegments, queueNum);
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


