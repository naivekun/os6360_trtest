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
* @file gtEthPortCtrl.h
*
* @brief Includes ethernet port routines for sending/receiving packets.
*
* @version   8
********************************************************************************
*/
#ifndef __gtEthPortCtrlh
#define __gtEthPortCtrlh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @enum extDrvEthTxMode_ENT
 *
 * @brief MII transmission mode
*/
typedef enum{

    /** execute asynchroniouse packet send */
    extDrvEthTxMode_asynch_E = 0,

    /** @brief execute synchroniouse packet send
     *  Note:
     */
    extDrvEthTxMode_synch_E

} extDrvEthTxMode_ENT;


/**
* @enum extDrvEthNetPortType_ENT
 *
 * @brief This type defines types of switch ports for BSP ETH driver.
*/
typedef enum{

    /** packets forwarded to CPSS */
    extDrvEthNetPortType_cpss_E    = 0,

    /** packets forwarded to OS (without dsa removal) */
    extDrvEthNetPortType_raw_E    = 1,

    /** @brief packets forwarded to OS (with dsa removal)
     *  Note:
     *  The enum has to be compatible with MV_NET_OWN and ap_packet.c
     */
    extDrvEthNetPortType_linux_E    = 2,

    extDrvEthNetPortType_numOfTypes

} extDrvEthNetPortType_ENT;



/*******************************************************************************
* GT_ExtRx_FUNCP
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
typedef GT_STATUS (*GT_Rx_FUNCPTR)
(
    IN GT_U8_PTR   segmentList[],
    IN GT_U32      segmentLen[],   
    IN GT_U32      numOfSegments,
    IN GT_U32      queueNum
);


/*******************************************************************************
* GT_RawSocketRx_FUNCPTR
*
* DESCRIPTION:
*       The prototype of the routine to be called after a raw packet was received
*
* INPUTS:
*       packet          - points to the entire packet.
*       packetLen       - The length of teh packet.
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

typedef GT_STATUS (*GT_RawSocketRx_FUNCPTR)
(
    IN GT_U8_PTR        packet,
    IN GT_U32           packetLen,
    IN GT_U32           rxQueue
);


/*******************************************************************************
* GT_Tx_COMPLETE_FUNCPTR
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
typedef GT_STATUS (*GT_Tx_COMPLETE_FUNCPTR)
(
    IN GT_U8_PTR   segmentList[],
    IN GT_U32      numOfSegments
);


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
);

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
    IN GT_BOOL flag
);

/**
* @internal extDrvEthRawSocketModeGet function
* @endinternal
*
* @brief   returns the raw packet mode
*/
GT_BOOL extDrvEthRawSocketModeGet
(
    IN GT_VOID
);

/**
* @internal extDrvLinuxModeSet function
* @endinternal
*
* @brief   Set port <portNum> to Linux Mode (Linux Only)
*
* @param[in] portNum                  - The port number to be defined for Linux mode
* @param[in] ip1                      ip2, ip3, ip4 - The ip address to assign to the port, 4 numbers
*
* @retval GT_OK                    - always,
*/

GT_STATUS extDrvLinuxModeSet
(
 IN GT_U32 portNum,
 IN GT_U32 ip1,
 IN GT_U32 ip2,
 IN GT_U32 ip3,
 IN GT_U32 ip4
);

/**
* @internal extDrvLinuxModeGet function
* @endinternal
*
* @brief   Get port <portNum> Linux Mode indication (Linux Only)
*
* @param[in] portNum                  - The port number to be defined for Linux mode
*                                       GT_TRUE if Linux mode, or
*                                       GT_FALSE otherwise.
*/

GT_BOOL extDrvLinuxModeGet
(
 IN GT_U32  portNum
);

/**
* @internal extDrvEthMuxSet function
* @endinternal
*
* @brief   Sets the mux mode to one of cpss, raw, linux
*/
GT_STATUS extDrvEthMuxSet
(
 IN unsigned long portNum,
 IN extDrvEthNetPortType_ENT portType
);

/**
* @internal extDrvEthMuxGet function
* @endinternal
*
* @brief   Get the mux mosde of the port
*/
GT_STATUS extDrvEthMuxGet
(
 IN unsigned long portNum,
 OUT extDrvEthNetPortType_ENT *portTypeP
 );

/**
* @internal extDrvEthPortRxInitWithoutInterruptConnection function
* @endinternal
*
* @brief   this function initialises ethernet port Rx interface without interrupt
*         connection
*         The function used in multiprocess appDemo for eth port init in appDemo
*         process. In RxTxProcess used extDrvEthPortRxInit that run
*         RxTxInterrupt thread
* @param[in] rxBufPoolSize            -   buffer pool size
* @param[in] rxBufPool_PTR            -   the address of the pool
* @param[in] rxQNum                   -   the number of RX queues
* @param[in] rxQbufPercentage[]       -   the buffer percentage dispersal for all queues
* @param[in] rxBufSize                -   the buffer requested size
* @param[in,out] numOfRxBufs_PTR          -   number of requested buffers, and actual buffers created
* @param[in] headerOffset             -   packet header offset size
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
);

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
    IN GT_U32        numOfTxBufs
);


/**
* @internal extDrvEthPortEnable function
* @endinternal
*
* @brief   Enable the ethernet port interface
*/
GT_STATUS extDrvEthPortEnable(GT_VOID);


/**
* @internal extDrvEthPortDisable function
* @endinternal
*
* @brief   Disable the ethernet port interface
*/
GT_STATUS extDrvEthPortDisable(GT_VOID);


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
);


/**
* @internal extDrvEthPortRxCb function
* @endinternal
*
* @brief   This function receives a packet from the ethernet port.
*
* @param[in] segmentList[]            - A list of pointers to the packets segments.
* @param[in] segmentLen[]             - A list of segment length.
* @param[in] numOfSegments            - The number of segment in segment list.
* @param[in] queueNum                 - The received queue number
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
GT_STATUS extDrvEthPortRxCb
(
    IN GT_U8_PTR   segmentList[],
    IN GT_U32      segmentLen[],   
    IN GT_U32      numOfSegments,
    IN GT_U32      queueNum
);


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
    IN GT_U8_PTR   segmentList[],
    IN GT_U32      numOfSegments
);


/**
* @internal extDrvEthInputHookAdd function
* @endinternal
*
* @brief   This bind the user Rx callback
*
* @param[in] userRxFunc               - the user Rx callback function
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
GT_STATUS extDrvEthInputHookAdd
(
    IN GT_Rx_FUNCPTR userRxFunc
);

/**
* @internal extDrvEthRawSocketRxHookAdd function
* @endinternal
*
* @brief   This bind the user Rx callback
*
* @param[in] userRxFunc               - the user Rx callback function
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
GT_STATUS extDrvEthRawSocketRxHookAdd
(
    IN GT_RawSocketRx_FUNCPTR userRxFunc
);

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
);


/**
* @internal extDrvEthRxPacketFree function
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
GT_STATUS extDrvEthRxPacketFree
(
    IN GT_U8_PTR        segmentList[],
    IN GT_U32           numOfSegments,
    IN GT_U32           queueNum
);

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
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
* @retval GT_NOT_SUPPORTED         - the API is not supported
*/
GT_STATUS extDrvEthPortTxModeSet
(
    extDrvEthTxMode_ENT    txMode
);

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
);

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
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __gtEthPortCtrlh */



