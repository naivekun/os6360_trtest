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
* @file noKmDrvEthPortCtrlStub.c
*
* @brief This file includes all needed functions for receiving packet buffers
* from upper layer, and sending them trough the PP's NIC.
*
* @version   1
********************************************************************************
*/
#include <gtOs/gtOsGen.h>
#include <gtExtDrv/drivers/gtEthPortCtrl.h>
#include <gtExtDrv/drivers/gtIntDrv.h>

/**
* @internal extDrvNetIfIntConnect function
* @endinternal
*
* @brief   Connect network interface rx/tx events.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_U32 extDrvNetIfIntConnect(GT_VOID)
{
    return GT_FAIL;
}

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
    (void)rxBufPoolSize;
    (void)rxBufPool_PTR;
    (void)rxQNum;
    (void)rxQbufPercentage;
    (void)rxBufSize;
    (void)numOfRxBufs_PTR;
    (void)headerOffset;

    return GT_FAIL;
}

/**
* @internal extDrvEthRawSocketModeSet function
* @endinternal
*
* @brief   Sets the raw packet mode
*
* @param[in] flag                     - GT_TRUE  - rawSocketMode is set to GT_TRUE
*                                      - GT_FALSE - rawSocketMode is set to GT_FALSE
*                                       GT_OK always
*/
GT_STATUS extDrvEthRawSocketModeSet
(
 IN GT_BOOL          flag
 )
{
    (void)flag;
    return GT_FAIL;
}

/**
* @internal extDrvEthRawSocketModeGet function
* @endinternal
*
* @brief   returns the raw packet mode
*/
GT_BOOL extDrvEthRawSocketModeGet
(
 IN GT_VOID
 )
{
  return GT_FALSE;
}

/**
* @internal extDrvLinuxModeGet function
* @endinternal
*
* @brief   Get port <portNum> Linux Mode indication (Linux Only)
*         INPUTS:
*/

GT_BOOL extDrvLinuxModeGet
(
 IN GT_U32  portNum
)
{
    (void)portNum;
    return GT_FALSE;
}

/**
* @internal extDrvEthPortRxInitWithoutInterruptConnection function
* @endinternal
*
* @brief   this function initialises ethernet port Rx interface without interrupt
*         connection
*         The function used in multiprocess appDemo for eth port init in appDemo
*         process. In RxTxProcess used extDrvEthPortRxInit that run
*         RxTxInterrupt thread
* @param[in] rxBufPoolSize            - buffer pool size
* @param[in] rxBufPool_PTR            - the address of the pool
* @param[in] rxQNum                   - the number of RX queues
* @param[in] rxQbufPercentage[]       - the buffer percentage dispersal for all queues
* @param[in] rxBufSize                - the buffer requested size
* @param[in,out] numOfRxBufs_PTR          - number of requested buffers, and actual buffers created
* @param[in] headerOffset             - packet header offset size
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
GT_STATUS extDrvEthPortRxInitWithoutInterruptConnection
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
    (void)rxBufPoolSize;
    (void)rxBufPool_PTR;
    (void)rxQNum;
    (void)rxQbufPercentage;
    (void)rxBufSize;
    (void)numOfRxBufs_PTR;
    (void)headerOffset;

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
    (void)numOfTxBufs;
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
    (void)segmentList;
    (void)segmentLen;
    (void)numOfSegments;
    (void)queueNum;
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
    (void)segmentList;
    (void)numOfSegments;
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
    (void)segmentList;
    (void)segmentLen;
    (void)numOfSegments;
    (void)txQueue;
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
    (void)txMode;

    return GT_NOT_SUPPORTED;
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
    (void)userRxFunc;
    return GT_FAIL;
}

/**
* @internal extDrvEthRawSocketRxHookAdd function
* @endinternal
*
* @brief   bind the raw packet Rx callback
*/
GT_STATUS extDrvEthRawSocketRxHookAdd
(
 IN GT_RawSocketRx_FUNCPTR    userRxFunc
 )
{
    (void)userRxFunc;
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
    (void)userTxFunc;
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
    (void)segmentList;
    (void)numOfSegments;
    (void)queueNum;

    return GT_FAIL;
}

/**
* @internal extDrvEthCpuCodeToQueue function
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
GT_STATUS extDrvEthCpuCodeToQueue
(
    IN GT_U32  dsaCpuCode,
    IN GT_U8   rxQueue
)
{
    (void)dsaCpuCode;
    (void)rxQueue;

    return GT_FAIL;
}

/**
* @internal extDrvEthPrePendTwoBytesHeaderSet function
* @endinternal
*
* @brief   Enables/Disable pre-pending a two-byte header to all packets arriving
*         to the CPU.
* @param[in] enable                   - GT_TRUE  - Two-byte header is pre-pended to packets
*                                      arriving to the CPU.
*                                      GT_FALSE - Two-byte header is not pre-pended to packets
*                                      arriving to the CPU.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
* @retval GT_NOT_SUPPORTED         - the API is not supported
*/
GT_STATUS extDrvEthPrePendTwoBytesHeaderSet
(
    IN GT_BOOL enable
)
{
    /*CPSS_TBD_BOOKMARK*/
    /* add support */
    return GT_OK;
}


