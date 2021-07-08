/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/

#include <gtOs/gtOsMem.h>
#include <gtOs/gtOsIo.h>

#ifndef APPLICATION_SIDE_ONLY
    #include <asicSimulation/SKernel/skernel.h>
    #include <common/SBUF/sbuf.h>
#endif /*!APPLICATION_SIDE_ONLY*/

#include <gtExtDrv/drivers/gtEthPortCtrl.h>

static GT_Tx_COMPLETE_FUNCPTR userTxCallback = NULL;
static GT_Rx_FUNCPTR          userRxCallback = NULL;


/* number of buffers in the pool */
#define SBSP_BUFFERS_NUMBER_CNS                    64
/* pss need up to 300 bytes for management */
#define SBSP_BUFFER_PSS_MANAGEMENT_OFFSET_CNS     300

#ifndef APPLICATION_SIDE_ONLY
    static SBUF_POOL_ID             sbspBuffPool    = NULL;
#endif /*!APPLICATION_SIDE_ONLY*/



#ifndef APPLICATION_SIDE_ONLY
/**
* @internal userRxCbWrapper function
* @endinternal
*
* @brief   This function wraps the Rx callback to disjoin the smain interface and
*         the external driver callback routine.
* @param[in] segmentList[]            - A list of pointers to the packets segments.
* @param[in] segmentLen[]             - A list of segement length.
* @param[in] numOfSegments            - The number of segment in segment list.
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
static GT_STATUS userRxCbWrapper
(
    IN GT_U8_PTR   segmentList[],
    IN GT_U32      segmentLen[],
    IN GT_U32      numOfSegments
)
{
    SBUF_BUF_ID  bufferId;
    GT_U8*      data_PTR;
    GT_U32      data_size;

    if (userRxCallback != NULL)
    {
        /* get buffer */
        bufferId = sbufAlloc(sbspBuffPool,SBUF_DATA_SIZE_CNS);

        if ( bufferId == NULL )
        {
            /* printf */ osPrintf(" userRxCbWrapper: no buffers for receive\n");
            return GT_FAIL;
        }

        /* get actual data pointer */
        sbufDataGet(bufferId, &data_PTR, &data_size);

        data_PTR += SBSP_BUFFER_PSS_MANAGEMENT_OFFSET_CNS;
        /* assume that simulation get only one segment */
        /* memcpy */ osMemCpy(data_PTR,segmentList[0],segmentLen[0]);


        return userRxCallback(&data_PTR, segmentLen , 1, 0);
    }
    else
        return GT_FAIL;
}
#endif /*!APPLICATION_SIDE_ONLY*/


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
#ifndef APPLICATION_SIDE_ONLY
    if(sbspBuffPool == NULL)
    {
        /* init the buffers pool */
        sbspBuffPool = sbufPoolCreate(SBSP_BUFFERS_NUMBER_CNS);
    }
#endif /*!APPLICATION_SIDE_ONLY*/

    return GT_OK;
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
  /* stub */
  return GT_OK;
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
  /* stub */
  return GT_FALSE;
}

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
)
{
  /* stub */
  return GT_OK;
}              

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
)
{
  /* stub */
  return GT_FALSE;
}              

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
 )
{
  /* stub */
  return GT_OK;
}

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
 )
{
  /* stub */
  *portTypeP = extDrvEthNetPortType_numOfTypes; /* make it invalid */
  return GT_OK;
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
  /* stub */
  return GT_OK;
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
    /* STUB for link */
    return GT_OK;
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
    /* STUB for link */
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
    /* STUB for link */
    return GT_OK;
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
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
GT_STATUS extDrvEthPortRxCb
(
    IN GT_U8_PTR   segmentList[],
    IN GT_U32      segmentLen[],
    IN GT_U32      numOfSegments,
    IN GT_U32      queueNum
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
    GT_STATUS status;

#ifndef APPLICATION_SIDE_ONLY
    status = skernelNicOutput(segmentList, segmentLen, numOfSegments);
#else /*!APPLICATION_SIDE_ONLY*/
    status = GT_FAIL;
#endif /*!APPLICATION_SIDE_ONLY*/

    if (userTxCallback)
    {
        userTxCallback(segmentList, numOfSegments);
    }

    return GT_OK == status ? GT_OK : GT_FAIL;
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
    IN GT_Rx_FUNCPTR userRxFunc
)
{
    userRxCallback = userRxFunc;

#ifndef APPLICATION_SIDE_ONLY
    skernelNicRxBind(userRxCbWrapper);
#endif /*!APPLICATION_SIDE_ONLY*/

    return GT_OK;
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
    userTxCallback = userTxFunc;

    return GT_OK;
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
#ifndef APPLICATION_SIDE_ONLY
    SBUF_BUF_ID     bufId;

    /* find the Buffer */
    bufId = sbufGetBufIdByData(sbspBuffPool,segmentList[0]);

    if(bufId == NULL)
    {
        /*printf*/ osPrintf(" extDrvEthRxPacketFree : FAILED to free packet\n");
        return GT_FAIL;
    }


    /* free the buffer */
    sbufFree(sbspBuffPool, bufId);
#endif /*!APPLICATION_SIDE_ONLY*/

    return GT_OK;
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


