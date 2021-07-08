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
* @file cmdExtServices.h
*
* @brief Common definition and APIs for Commander external services.
*
*
* @version   38
********************************************************************************
*/

#ifndef __cmdExtServices_h_
#define __cmdExtServices_h_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/* external services support */
#include <cpss/extServices/os/gtOs/gtGenTypes.h>
#include <cpss/extServices/cpssExtServices.h>
#include <cpss/generic/events/cpssGenEventUnifyTypes.h>
#include <cpss/generic/events/cpssGenEventRequests.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/extServices/trace/cpssTraceHw.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>

#ifdef CHX_FAMILY
    #include <cpss/generic/trunk/cpssGenTrunkTypes.h>
    #include <cpss/generic/port/cpssPortCtrl.h>
    #include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h>
    #include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#endif /*CHX_FAMILY*/

#define GT_SOCKET_FD                        CPSS_SOCKET_FD
#define GT_FD                               CPSS_SOCKET_FD
#define GT_SOCKET_PORT                      CPSS_SOCKET_PORT

#define CMD_OS_SOCKET_TCP_CREATE_FUN        CPSS_SOCKET_TCP_CREATE_FUNC
#define CMD_OS_SOCKET_UDP_CREATE_FUN        CPSS_SOCKET_UDP_CREATE_FUNC
#define CMD_OS_SOCKET_TCP_DESTROY_FUN       CPSS_SOCKET_TCP_DESTROY_FUNC
#define CMD_OS_SOCKET_UDP_DESTROY_FUN       CPSS_SOCKET_UDP_DESTROY_FUNC
#define CMD_OS_SOCKET_CREATE_ADDR_FUN       CPSS_SOCKET_CREATE_ADDR_FUNC
#define CMD_OS_SOCKET_DESTROY_ADDR_FUN      CPSS_SOCKET_DESTROY_ADDR_FUNC
#define CMD_OS_SOCKET_BIND_FUN              CPSS_SOCKET_BIND_FUNC
#define CMD_OS_SOCKET_LISTEN_FUN            CPSS_SOCKET_LISTEN_FUNC
#define CMD_OS_SOCKET_ACCEPT_FUN            CPSS_SOCKET_ACCEPT_FUNC
#define CMD_OS_SOCKET_CONNECT_FUN           CPSS_SOCKET_CONNECT_FUNC
#define CMD_OS_SOCKET_SET_NON_BLOCK_FUN     CPSS_SOCKET_SET_NONBLOCK_FUNC
#define CMD_OS_SOCKET_SET_BLOCK_FUN         CPSS_SOCKET_SET_BLOCK_FUNC
#define CMD_OS_SOCKET_SEND_FUN              CPSS_SOCKET_SEND_FUNC
#define CMD_OS_SOCKET_SEND_TO_FUN           CPSS_SOCKET_SENDTO_FUNC
#define CMD_OS_SOCKET_RECV_FUN              CPSS_SOCKET_RECV_FUNC
#define CMD_OS_SOCKET_RECV_FROM_FUN         CPSS_SOCKET_RECVFROM_FUNC
#define CMD_OS_SOCKET_SET_SOCKET_NO_LINGER_FUN  CPSS_SOCKET_SET_SOCKET_NOLINGER_FUNC
#define CMD_OS_SOCKET_EXTRACT_IP_ADDR_FROM_SOCKET_ADDR_FUN  CPSS_SOCKET_EXTRACT_IPADDR_FROM_SOCKET_ADDR_FUNC
#define CMD_OS_SOCKET_GET_SOCKET_ADDR_SIZE_FUN      CPSS_SOCKET_GET_ADDR_SIZE_FUNC
#define CMD_OS_SOCKET_SHUT_DOWN_FUN         CPSS_SOCKET_SHUTDOWN_FUNC

#define CMD_OS_SELECT_CREATE_SET_FUN        CPSS_SOCKET_SELECT_CREATE_SET_FUNC
#define CMD_OS_SELECT_ERASE_SET_FUN         CPSS_SOCKET_SELECT_ERASE_SET_FUNC
#define CMD_OS_SELECT_ZERO_SET_FUN          CPSS_SOCKET_SELECT_ZERO_SET_FUNC
#define CMD_OS_SELECT_ADD_FD_TO_SET_FUN     CPSS_SOCKET_SELECT_ADD_FD_TO_SET_FUNC
#define CMD_OS_SELECT_CLEAR_FD_FROM_SET_FUN CPSS_SOCKET_SELECT_CLEAR_FD_FROM_SET_FUNC
#define CMD_OS_SELECT_IS_FD_SET_FUN         CPSS_SOCKET_SELECT_IS_FD_SET_FUNC
#define CMD_OS_SELECT_COPY_SET_FUN          CPSS_SOCKET_SELECT_COPY_SET_FUNC
#define CMD_OS_SELECT_FUN                   CPSS_SOCKET_SELECT_FUNC
#define CMD_OS_SOCKET_GET_SOCKET_FD_SET_SIZE_FUN    CPSS_SOCKET_SELECT_GET_SOCKET_FD_SIZE_FUNC

/* CMD_OS_FUNC_BIND_SOCKETS_STC -
*    structure that hold the "os sockets" functions needed be bound to CMD.
*
*/
typedef struct{
    CMD_OS_SOCKET_TCP_CREATE_FUN        osSocketTcpCreate;
    CMD_OS_SOCKET_UDP_CREATE_FUN        osSocketUdpCreate;
    CMD_OS_SOCKET_TCP_DESTROY_FUN       osSocketTcpDestroy;
    CMD_OS_SOCKET_UDP_DESTROY_FUN       osSocketUdpDestroy;
    CMD_OS_SOCKET_CREATE_ADDR_FUN       osSocketCreateAddr;
    CMD_OS_SOCKET_DESTROY_ADDR_FUN      osSocketDestroyAddr;
    CMD_OS_SOCKET_BIND_FUN              osSocketBind;
    CMD_OS_SOCKET_LISTEN_FUN            osSocketListen;
    CMD_OS_SOCKET_ACCEPT_FUN            osSocketAccept;
    CMD_OS_SOCKET_CONNECT_FUN           osSocketConnect;
    CMD_OS_SOCKET_SET_NON_BLOCK_FUN     osSocketSetNonBlock;
    CMD_OS_SOCKET_SET_BLOCK_FUN         osSocketSetBlock;
    CMD_OS_SOCKET_SEND_FUN              osSocketSend;
    CMD_OS_SOCKET_SEND_TO_FUN           osSocketSendTo;
    CMD_OS_SOCKET_RECV_FUN              osSocketRecv;
    CMD_OS_SOCKET_RECV_FROM_FUN         osSocketRecvFrom;
    CMD_OS_SOCKET_SET_SOCKET_NO_LINGER_FUN osSocketSetSocketNoLinger;
    CMD_OS_SOCKET_EXTRACT_IP_ADDR_FROM_SOCKET_ADDR_FUN osSocketExtractIpAddrFromSocketAddr;
    CMD_OS_SOCKET_GET_SOCKET_ADDR_SIZE_FUN osSocketGetSocketAddrSize;
    CMD_OS_SOCKET_SHUT_DOWN_FUN         osSocketShutDown;

    CMD_OS_SELECT_CREATE_SET_FUN        osSelectCreateSet;
    CMD_OS_SELECT_ERASE_SET_FUN         osSelectEraseSet;
    CMD_OS_SELECT_ZERO_SET_FUN          osSelectZeroSet;
    CMD_OS_SELECT_ADD_FD_TO_SET_FUN     osSelectAddFdToSet;
    CMD_OS_SELECT_CLEAR_FD_FROM_SET_FUN osSelectClearFdFromSet;
    CMD_OS_SELECT_IS_FD_SET_FUN         osSelectIsFdSet;
    CMD_OS_SELECT_COPY_SET_FUN          osSelectCopySet;
    CMD_OS_SELECT_FUN                   osSelect;
    CMD_OS_SOCKET_GET_SOCKET_FD_SET_SIZE_FUN osSocketGetSocketFdSetSize;
}CMD_OS_FUNC_BIND_SOCKETS_STC;

#define CMD_OS_TASK_GET_SELF_FUN            CPSS_OS_TASK_GET_SELF_FUNC
#ifndef GT_TASK
#define GT_TASK CPSS_TASK
#endif
/*******************************************************************************
* osSetTaskPrior
*
* DESCRIPTION:
*       Changes priority of task/thread.
*
* INPUTS:
*       tid     - Task ID
*       newprio - new priority of task
*
* OUTPUTS:
*       oldprio - old priority of task
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       If tid = 0, change priority of calling task (itself)
*
*******************************************************************************/
typedef GT_STATUS (*CMD_OS_TASK_SET_TASK_PRIOR_FUNC)
(
    IN  GT_TASK tid,
    IN  GT_U32  newprio,
    OUT GT_U32  *oldprio
);
/* CMD_OS_FUNC_BIND_TASKS_STC -
*    structure that hold the "os tasks" functions needed be bound to CMD.
*
*/
typedef struct{
    CMD_OS_TASK_GET_SELF_FUN            osTaskGetSelf;
    CMD_OS_TASK_SET_TASK_PRIOR_FUNC     osSetTaskPrior;
}CMD_OS_FUNC_BIND_TASKS_STC;



#ifndef __gtBmPoolh
/************* Define *********************************************************/

#define GT_POOL_NULL_ID  ((GT_POOL_ID)NULL)



/************* Typedef ********************************************************/


/*
 * Typedef: GT_POOL_ID
 *
 * Description:
 *  Define type for pool ID.
 *  The pool ID is allocated and returned to the user by the poolCreate function.
 *  In order to delet pool, allocate buffer or free buffer, the pool ID is given as
 *  parametr to those service functions.
 *
 */
typedef GT_VOID* GT_POOL_ID;

/**
* @enum GT_POOL_ALIGNMENT
 *
 * @brief Define the buffer alignment supported by the module.
*/
typedef enum{

    GT_1_BYTE_ALIGNMENT  = 0x1,

    GT_4_BYTE_ALIGNMENT  = 0x3,

    GT_8_BYTE_ALIGNMENT  = 0x7,

    GT_16_BYTE_ALIGNMENT = 0xF

} GT_POOL_ALIGNMENT;


#endif /* __gtBmPoolh */

/************* Functions ******************************************************/


/*******************************************************************************
* gtPoolCreatePool
*
* DESCRIPTION:
*           Create pool of buffers. All buffers have the same size.
*
* INPUTS:
*           bufferSize   - Buffer's size
*           alignment    - Buffers' alignment
*           numOfBuffers - number of buffer at the pool
*           useSem       - GT_TRUE to use a semaphore for mutual exclusion on
*                         access to the pool.
*                  Note: Semaphore protection is one for all pools.
*
* OUTPUTS:
*           pPoolId       - Return the pool ID
* RETURNS:
*       GT_OK - For successful operation.
*       GT_BAD_VALUE - Request for zero number of buffers or
*                      buffer's size is smaller than the size of
*                      pointer (usually 4 bytes)
*       GT_BAD_PTR - Bad pointer for the pPoolId
*       GT_NO_RESOURCE - No memory is available to create the pool.
*       GT_NOT_INITIALIZED - In case of un-initialized package.
*
* COMMENTS:
*       Usage of this function is only during FIRST initialization.
*
*******************************************************************************/
typedef GT_STATUS  (*CMD_OS_POOL_CREATE_POOL_FUNC)
(
    IN   GT_U32            reqBufferSize,
    IN   GT_POOL_ALIGNMENT alignment,
    IN   GT_U32            numOfBuffers,
    IN   GT_BOOL           useSem,
    OUT  GT_POOL_ID       *pPoolId
);

/*******************************************************************************
* gtPoolCreateDmaPool
*
* DESCRIPTION:
*           Create pool of buffers. All buffers have the same size.
*           The buffers are created in a physically contiguous area.
*
* INPUTS:
*           bufferSize   - Buffer's size
*           alignment    - Buffers' alignment
*           numOfBuffers - number of buffer at the pool
*           useSem       - GT_TRUE to use a semaphore for mutual exclusion on
*                         access to the pool.
*                  Note: Semaphore protection is one for all pools.
*
* OUTPUTS:
*           pPoolId       - Return the pool ID
* RETURNS:
*       GT_OK - For successful operation.
*       GT_BAD_VALUE - Request for zero number of buffers or
*                      buffer's size is smaller than the size of
*                      pointer (usually 4 bytes)
*       GT_BAD_PTR - Bad pointer for the pPoolId
*       GT_NO_RESOURCE - No memory is available to create the pool.
*       GT_NOT_INITIALIZED - In case of un-initialized package.
*
* COMMENTS:
*       Usage of this function is only during FIRST initialization.
*
*******************************************************************************/
typedef GT_STATUS  (*CMD_OS_POOL_CREATE_DMA_POOL_FUNC)
(
    IN   GT_U32            reqBufferSize,
    IN   GT_POOL_ALIGNMENT alignment,
    IN   GT_U32            numOfBuffers,
    IN   GT_BOOL           useSem,
    OUT  GT_POOL_ID       *pPoolId
);

/*******************************************************************************
* gtPoolReCreatePool
*
* DESCRIPTION:
*           Create pool of buffers. All buffers have the same size.
*
* INPUTS:
*           poolId    - Id of the pool to be re-created.

* OUTPUTS:
*           None.
* RETURNS:
*       GT_OK        - For successful operation.
*       GT_BAD_VALUE - Bad poolId
*       GT_NOT_INITIALIZED - In case of un-initialized package.
*
* COMMENTS:
*       Usage of this function is only during FIRST initialization.
*
*******************************************************************************/
typedef GT_STATUS  (*CMD_OS_POOL_RE_CREATE_POOL_FUNC)
(
    IN  GT_POOL_ID  poolId
);

/*******************************************************************************
* gtPoolDeletePool
*
* DESCRIPTION:
*           Delete a pool and free all the memory occupied by that pool.
*           The operation is failed if not all the buffers belong to that pool
*           have been freed.
* INPUTS:
*           PoolId - The pool ID as returned by the create function.
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK - For successful operation.
*       GT_BAD_STATE - Not all buffers belong to that pool have
*                      been freed.
*
* COMMENTS:
*       None
*
*******************************************************************************/
typedef GT_STATUS  (*CMD_OS_POOL_DELETE_POOL_FUNC)
(
    IN  GT_POOL_ID  poolId
);

/*******************************************************************************
* gtPoolGetBuf
*
* DESCRIPTION:
*           Return pointer to a buffer from the requested pool
*
* INPUTS:
*           PoolId - The pool ID as returned by the create function.
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_VOID* - Pointer to the new allocated buffer. NULL is returned in case
*               no buffer is not available.
*
* COMMENTS:
*       None
*
*******************************************************************************/
typedef GT_VOID* (*CMD_OS_POOL_GET_BUF_FUNC)
(
    IN        GT_POOL_ID  poolId
);


/*******************************************************************************
* gtPoolFreeBuf
*
* DESCRIPTION:
*           Free a buffer back to its pool.
*
* INPUTS:
*           PoolId - The pool ID as returned by the create function.
*           pBuf   - Pointer to buffer to be freed
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_STATUS    GT_OK - For successful operation.
*                      GT_BAD_PTR - The returned buffer is not belongs to that pool
*
* COMMENTS:
*       None
*
*******************************************************************************/
typedef GT_STATUS  (* CMD_OS_POOL_FREE_BUF_FUNC)
(
    IN        GT_POOL_ID  poolId,
    IN        GT_VOID*    pBuf
);


/*******************************************************************************
* gtPoolGetBufSize
*
* DESCRIPTION:
*           Get the buffer size.
*
* INPUTS:
*           PoolId - The pool ID as returned by the create function.
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_U32  - the buffer size.
*
* COMMENTS:
*       None
*
*******************************************************************************/
typedef GT_U32  (* CMD_OS_POOL_GET_BUF_SIZE_FUNC)
(
    IN        GT_POOL_ID  poolId
);

/*******************************************************************************
* gtPoolExpandPool
*
* DESCRIPTION:
*           Expand a pool of buffers. All buffers have the same size.
*
* INPUTS:
*           PoolId - The pool ID as returned by the create function.
*           forDma       - is physically contiguous.
*           numOfBuffers - number of buffer to add to the pool
*
* OUTPUTS:
*       none.
* RETURNS:
*       GT_OK - For successful operation.
*       GT_BAD_VALUE - Request for zero number of buffers or
*                      buffer's size is smaller than the size of
*                      pointer (usually 4 bytes)
*       GT_BAD_PTR - Bad pointer for the pPoolId
*       GT_NO_RESOURCE - No memory is available to create the pool.
*       GT_NOT_INITIALIZED - In case of un-initialized package.
*
* COMMENTS:
*       not used for Dma buffers!
*
*******************************************************************************/
typedef GT_STATUS  (* CMD_OS_POOL_EXPAND_POOL_FUNC)
(
    IN   GT_POOL_ID        pPoolId,
    IN   GT_U32            numOfBuffers
);


/*******************************************************************************
* gtPoolGetBufFreeCnt
*
* DESCRIPTION:
*           Get the free buffer count.
*
* INPUTS:
*           PoolId - The pool ID as returned by the create function.
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_U32  - the free buffer count.
*
* COMMENTS:
*       None
*
*******************************************************************************/
typedef GT_U32 (*CMD_OS_POOL_GET_BUF_FREE_CNT_FUNC)
(
    IN        GT_POOL_ID  poolId
);


/*******************************************************************************
* gtPoolPrintStats
*
* DESCRIPTION:
*           Print pool's statistics for the requested pool, or for all
*           the pols in case poolID is GT_POOL_NULL_ID.
*
* INPUTS:
*           PoolId      - The pool ID as returned by the create function.
*                         or GT_POOL_NULL_ID to print all pools' information.
* OUTPUTS:
*       None
*
* RETURNS:
*       None.
*
* COMMENTS:
*       None
*
*******************************************************************************/
typedef GT_VOID  (* CMD_OS_POOL_PRINT_STATS_FUNC)
(
    IN  GT_POOL_ID poolId
);



/* CMD_OS_FUNC_BIND_POOLS_STC -
*    structure that hold the "os pools" functions needed be bound to CMD.
*
*/
typedef struct{
    CMD_OS_POOL_CREATE_POOL_FUNC        gtPoolCreatePool;
    CMD_OS_POOL_CREATE_DMA_POOL_FUNC    gtPoolCreateDmaPool;
    CMD_OS_POOL_RE_CREATE_POOL_FUNC     gtPoolReCreatePool;
    CMD_OS_POOL_DELETE_POOL_FUNC        gtPoolDeletePool;
    CMD_OS_POOL_GET_BUF_FUNC            gtPoolGetBuf;
    CMD_OS_POOL_FREE_BUF_FUNC           gtPoolFreeBuf;
    CMD_OS_POOL_GET_BUF_SIZE_FUNC       gtPoolGetBufSize;
    CMD_OS_POOL_EXPAND_POOL_FUNC        gtPoolExpandPool;
    CMD_OS_POOL_GET_BUF_FREE_CNT_FUNC   gtPoolGetBufFreeCnt;
    CMD_OS_POOL_PRINT_STATS_FUNC        gtPoolPrintStats;
}CMD_OS_FUNC_BIND_POOLS_STC;


/*******************************************************************************
* cmdIsCpuEtherPortUsed
*
* DESCRIPTION:
*           is CPU Ethernet port used (and not SDMA)
*
* INPUTS:
*           none
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_TRUE - if CPU Ethernet port used (and not SDMA)
*       GT_FALSE - otherwise
*
* COMMENTS:
*       None
*
*******************************************************************************/
typedef GT_BOOL (*CMD_CPU_ETHERNET_IS_CPU_ETHER_PORT_USED)
(
    GT_VOID
);


/* CMD_FUNC_BIND_CPU_ETHERNET_PORT_STC -
*    structure that hold the "cpu Ethernet port" functions needed be bound to CMD.
*
*/
typedef struct{
    CMD_CPU_ETHERNET_IS_CPU_ETHER_PORT_USED        cmdIsCpuEtherPortUsed;
}CMD_FUNC_BIND_CPU_ETHERNET_PORT_STC;



/*******************************************************************************
* CMD_CPSS_EVENT_BIND_FUNC
*
* DESCRIPTION:
*       This routine binds a user process to unified event. The routine returns
*       a handle that is used when the application wants to wait for the event
*       (cpssEventSelect), receive the event(cpssEventRecv) or transmit a packet
*       using the Network Interface.
*
*       NOTE : the function does not mask/unmask the HW events in any device.
*              This is Application responsibility to unmask the relevant events
*              on the needed devices , using function cpssEventDeviceMaskSet
*
* APPLICABLE DEVICES:  All devices (PP / FA / XBAR)
*
* INPUTS:
*       uniEventArr - The unified event list.
*       arrLength   - The unified event list length.
*
* OUTPUTS:
*       hndlPtr     - (pointer to) The user handle for the bounded events.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on failure
*       GT_BAD_PTR  - one of the parameters is NULL pointer
*       GT_OUT_OF_CPU_MEM - failed to allocate CPU memory,
*       GT_FULL - when trying to set the "tx buffer queue unify event"
*                 (CPSS_PP_TX_BUFFER_QUEUE_E) with other events in the same
*                 handler
*       GT_ALREADY_EXIST - one of the unified events already bound to another
*                 handler (not applicable to CPSS_PP_TX_BUFFER_QUEUE_E)
*
* COMMENTS:
*       None.
*
*******************************************************************************/
typedef GT_STATUS (*CMD_CPSS_EVENT_BIND_FUNC)
(
    IN  CPSS_UNI_EV_CAUSE_ENT uniEventArr[],
    IN  GT_U32               arrLength,
    OUT GT_UINTPTR           *hndlPtr
);

/*******************************************************************************
* CMD_CPSS_EVENT_SELECT_FUNC
*
* DESCRIPTION:
*       This function waiting for one of the events ,relate to the handler,
*       to happen , and gets a list of events (in array of bitmaps format) that
*       occurred .
*
* APPLICABLE DEVICES:  All devices (PP / FA / XBAR)
*
* INPUTS:
*       hndl                - The user handle for the bounded events.
*       timeoutPtr          - (pointer to) Wait timeout in milliseconds
*                             NULL pointer means wait forever.
*       evBitmapArrLength   - The bitmap array length (in words).
*
* OUTPUTS:
*       evBitmapArr         - The bitmap array of the received events.
*
* RETURNS:
*       GT_OK  - success
*       GT_FAIL - general failure
*       GT_TIMEOUT - when the "time out" requested by the caller expired and no
*                    event occurred during this period
*       GT_BAD_PARAM - bad hndl parameter , the hndl parameter is not legal
*                     (was not returned by cpssEventBind(...))
*       GT_BAD_PTR  - evBitmapArr parameter is NULL pointer
*                     (and evBitmapArrLength != 0)
*
* COMMENTS:
*       None.
*
*******************************************************************************/
typedef GT_STATUS (*CMD_CPSS_EVENT_SELECT_FUNC)
(
    IN  GT_UINTPTR           hndl,
    IN  GT_U32              *timeoutPtr,
    OUT GT_U32              evBitmapArr[],
    IN  GT_U32              evBitmapArrLength
);

/*******************************************************************************
* CMD_CPSS_EVENT_RECV_FUNC
*
* DESCRIPTION:
*       This function gets general information about the selected unified event.
*       The function retrieve information about the device number that relate to
*       the event , and extra info about port number / priority queue that
*       relate to the event.
*       The function retrieve the info about the first occurrence of this event
*       in the queue.
*
* APPLICABLE DEVICES:  All devices (PP / FA / XBAR)
*
* INPUTS:
*       hndl        - The user handle for the bounded events.
*       evCause     - The specified unify event that info about it's first
*                     waiting occurrence required.
* OUTPUTS:
*       evExtDataPtr- (pointer to)The additional date (port num / priority
*                     queue number) the event was received upon.
*       evDevPtr    - (pointer to)The device the event was received upon
*
* RETURNS:
*       GT_OK  - success
*       GT_FAIL - general failure
*       GT_NO_MORE   - There is no more info to retrieve about the specified
*                      event .
*       GT_BAD_PARAM - bad hndl parameter ,
*                      or hndl bound to CPSS_PP_TX_BUFFER_QUEUE_E --> not allowed
*                      use dedicated "get tx ended info" function instead
*       GT_BAD_PTR  - one of the parameters is NULL pointer
* COMMENTS:
*       None.
*
*******************************************************************************/
typedef GT_STATUS (*CMD_CPSS_EVENT_RECV_FUNC)
(
    IN  GT_UINTPTR            hndl,
    IN  CPSS_UNI_EV_CAUSE_ENT evCause,
    OUT GT_U32              *evExtDataPtr,
    OUT GT_U8               *evDevPtr
);

/*******************************************************************************
* appDemoCpssEventDeviceMaskSet
*
* DESCRIPTION:
*       This routine mask/unmasks an unified event on specific device.
*
* APPLICABLE DEVICES:  All devices (PP / FA / XBAR)
*
* INPUTS:
*       devNum - device number - PP/FA/Xbar device number -
*                depend on the uniEvent
*                if the uniEvent is in range of PP events , then devNum relate
*                to PP
*                if the uniEvent is in range of FA events , then devNum relate
*                to FA
*                if the uniEvent is in range of XBAR events , then devNum relate
*                to XBAR
*       uniEvent   - The unified event.
*       operation  - the operation : mask / unmask
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL - on failure.
*       GT_BAD_PARAM - bad unify event value or bad device number
*
* COMMENTS:
*
*       The OS interrupts and the ExtDrv interrupts are locked during the
*       operation of the function.
*
*******************************************************************************/
typedef GT_STATUS (*CMD_CPSS_EVENT_DEVICE_MASK_SET_FUNC)
(
    IN GT_U8                    devNum,
    IN CPSS_UNI_EV_CAUSE_ENT    uniEvent,
    IN CPSS_EVENT_MASK_SET_ENT  operation
);



/* CMD_FUNC_BIND_EVENTS_STC -
*    structure that hold the "events" functions needed be bound to CMD.
*
*/
typedef struct{
    CMD_CPSS_EVENT_BIND_FUNC            cmdCpssEventBind;
    CMD_CPSS_EVENT_SELECT_FUNC          cmdCpssEventSelect;
    CMD_CPSS_EVENT_RECV_FUNC            cmdCpssEventRecv;
    CMD_CPSS_EVENT_DEVICE_MASK_SET_FUNC cmdCpssEventDeviceMaskSet;
}CMD_FUNC_BIND_EVENTS_STC;

/*******************************************************************************
* appDemoDbEntryAdd
*
* DESCRIPTION:
*       Set AppDemo DataBase value.This value will be considered during system
*       initialization process.
*
* INPUTS:
*       namePtr         - points to parameter name
*       value           - parameter value
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - if name is too long
*       GT_BAD_PTR      - if NULL pointer
*       GT_NO_RESOURCE  - if Database is full
*
* COMMENTS:
*       None.
*
*******************************************************************************/
typedef GT_STATUS (* CMD_APP_DB_ENTRY_ADD_FUNC)
(
    IN  GT_CHAR *namePtr,
    IN  GT_U32  value
);


/******************************************************************************
* appDemoDbEntryGet
*
* DESCRIPTION:
*       Get parameter value from AppDemo DataBase.
*
* INPUTS:
*       namePtr         - points to parameter name
*
* OUTPUTS:
*       valuePtr        - points to parameter value
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - if name is too long
*       GT_BAD_PTR      - if NULL pointer
*       GT_NO_SUCH      - there is no such parameter in Database
*
* COMMENTS:
*       None.
*
******************************************************************************/
typedef GT_STATUS (* CMD_APP_DB_ENTRY_GET_FUNC)
(
    IN  GT_CHAR *namePtr,
    OUT GT_U32  *valuePtr
);


/******************************************************************************
* appDemoDbDump
*
* DESCRIPTION:
*       Dumps entries set in AppDemo database to console.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK       - on success
*
* COMMENTS:
*       None.
*
******************************************************************************/
typedef GT_STATUS (*CMD_APP_DB_DUMP_FUNC)
(
    GT_VOID
);

/**
* @struct CMD_APP_PP_CONFIG
 *
 * @brief Holds Pp configuration that Application hold
*/
typedef struct
{
    CPSS_PP_DEVICE_TYPE             deviceId;
    GT_U8                           devNum;
    GT_BOOL                         valid;
    struct{
        GT_BOOL     trunkToCpuPortNum;
    }wa;
}CMD_APP_PP_CONFIG;


/******************************************************************************
* cmdAppPpConfigGet
*
* DESCRIPTION:
*       get pointer to the application's info about the PP
*
* INPUTS:
*       devIndex - index of the device in the Application DB (0..127)
*
* OUTPUTS:
*       ppConfigPtr - the info about the PP
*
* RETURNS:
*       GT_OK - on success
*       GT_BAD_PTR - on NULL pointer
*       GT_OUT_OF_RANGE - on devIndex > 127
*
* COMMENTS:
*       None.
*
******************************************************************************/
typedef GT_STATUS (*CMD_APP_PP_CONFIG_GET_FUNC)
(
    IN GT_U8    devIndex,
    OUT CMD_APP_PP_CONFIG* ppConfigPtr
);

/******************************************************************************
* cmdAppPpConfigPrint
*
* DESCRIPTION:
*       print Pp configuration
*
* INPUTS:
*       devNum - device number(0..127)
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK - on success
*       GT_OUT_OF_RANGE - on devIndex > 127
*
* COMMENTS:
*       None.
*
******************************************************************************/
typedef GT_STATUS (*CMD_APP_PP_CONFIG_PRINT_FUNC)
(
    IN GT_U8    devNum
);


/*******************************************************************************
* cpssInitSystem
*
* DESCRIPTION:
*       This is the main board initialization function for CPSS driver.
*
* INPUTS:
*       boardIdx      - The index of the board to be initialized from the board
*                       list.
*       boardRevId    - Board revision Id.
*       reloadEeprom  - Whether the Eeprom should be reloaded when
*                       corePpHwStartInit() is called.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
typedef GT_STATUS (*CMD_APP_INIT_SYSTEM_FUNC)
(
    IN  GT_U32  boardIdx,
    IN  GT_U32  boardRevId,
    IN  GT_U32  reloadEeprom
);


/*******************************************************************************
* cpssInitSystemGet
*
* DESCRIPTION:
*       Function gets parameters of cpss init system proccess.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       boardIdxPtr     - (pointer to) The index of the board to be initialized
*                         from the board list.
*       boardRevIdPtr   - (pointer to) Board revision Id.
*       reloadEepromPtr - (pointer to) Whether the Eeprom should be reloaded when
*                         corePpHwStartInit() is called.
*
* RETURNS:
*       None.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
typedef void (*CMD_APP_INIT_SYSTEM_GET_FUNC)
(
    OUT GT_U32  *boardIdxPtr,
    OUT GT_U32  *boardRevIdPtr,
    OUT GT_U32  *reloadEepromPtr
);

/*******************************************************************************
* cpssResetSystem
*
* DESCRIPTION:
*       CPSS reset function for secondary system initialization
*
* INPUTS:
*       doHwReset - indication for HW reset
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
typedef GT_STATUS (*CMD_APP_RESET_SYSTEM_FUNC)
(
    IN GT_BOOL doHwReset
);

/*******************************************************************************
* confi2InitSystem
*
* DESCRIPTION:
*       This is the main board initialization function.
*
* INPUTS:
*       theConfiId  - the confi ID
*       echoOn      - if GT_TRUE the echo is on, otherwise the echo is off.
*
* OUTPUTS:
*       none
*
* RETURNS:
*       GT_OK   - if succeeded,
*       GT_FAIL - if failed
*
* COMMENTS:
*
*
*******************************************************************************/
typedef GT_STATUS (*CMD_APP_INIT_CONFI_FUNC)
(
    IN GT_U8        theConfiId,
    IN GT_BOOL      echoOn
);

/*******************************************************************************
* appDemoShowBoardsList
** DESCRIPTION:
*       This function displays the boards list which have an automatic
*       initialization support.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
* COMMENTS:
*       The printed table should look like:
*       +-------------------------------+-----------------------+
*       | Board name                    | Revisions             |
*       +-------------------------------+-----------------------+
*       | 01 - RD-EX120-24G             |                       |
*       |                               | 01 - Rev 1.0          |
*       |                               | 02 - Rev 2.0          |
*       +-------------------------------+-----------------------+
*       | 02 - DB-MX610-48F4GS          |                       |
*       |                               | 01 - Rev 1.0          |
*       |                               | 02 - Rev 2.5          |
*       +-------------------------------+-----------------------+
*
*******************************************************************************/
typedef GT_STATUS (*CMD_APP_SHOW_BOARDS_LIST_FUNC)
(
    GT_VOID
);

/*******************************************************************************
* cmdAppIsSystemInitialized
** DESCRIPTION:
*       is system initialized
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_TRUE   - system initialized
*       GT_FALSE  - system NOT initialized
*
* COMMENTS:
*
*******************************************************************************/
typedef GT_BOOL (*CMD_APP_IS_SYSTEM_INITIALIZED_FUNC)
(
    GT_VOID
);

#ifdef CHX_FAMILY
/*******************************************************************************
* gtAppDemoLionPortModeSpeedSet
*
* DESCRIPTION:
*       Example of configuration sequence of port interface mode and speed
*       to show move from 1G to 10G and from 10G to 1G port data speed
*
* INPUTS:
*       devNum   - device number
*       portNum  - port number (or CPU port)
*       ifMode    - Interface mode.
*       speed    - port speed
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*       GT_HW_ERROR - hw error
*
* COMMENTS:
*       Current supported modes 1G SGMII (1.25G serdes speed)
*               and 10G RXAUI (6.25G serdes speed)
*
*******************************************************************************/
typedef GT_STATUS (*CMD_APP_LION_PORT_MODE_SPEED_SET_FUNC)
(
    IN  GT_U8     devNum,
    IN  GT_U8     portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT   ifMode,
    IN  CPSS_PORT_SPEED_ENT  speed
);

/*******************************************************************************
* bc2AppDemoPortsConfig
*
* DESCRIPTION:
*     Init required ports of Bobcat2 to specified ifMode and speed
*
* INPUTS:
*       devNumber - device number in appDemo
*       ifMode - port interface mode
*       speed - port speed
*       powerUp -   GT_TRUE - port power up
*                   GT_FALSE - serdes power down
*       numOfPorts - quantity of ports to configure
*                   use 0 if all device ports wanted
*       ... - numbers of ports to configure
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
* COMMENTS:
*       Imlemented for Bobcat2 B0 only, since
*       function cpssDxChPortSerdesSignalDetectGet is not imlemented
*       for Bobcat2 A0
*
*******************************************************************************/
typedef GT_STATUS (*CMD_APP_BC2_PORTS_CONFIG_FUNC)
(
    IN  GT_U8                           devNumber,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  GT_BOOL                         powerUp,
    IN  GT_U32                          numOfPorts,
    ...
);

/*******************************************************************************
* bobkAppDemoPortsConfig
*
* DESCRIPTION:
*     Init required ports of Bobk to specified ifMode and speed
*
* INPUTS:
*       devNumber - device number in appDemo
*       ifMode - port interface mode
*       speed - port speed
*       powerUp -   GT_TRUE - port power up
*                   GT_FALSE - serdes power down
*       squelch - threshold for signal OK (0-15) to change - If out range - ignored
*               - to not change  squelch - use value out of range, for example 0xFF
*
*       numOfPorts - quantity of ports to configure
*                   use 0 if all device ports wanted
*       ... - numbers of ports to configure
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
*
*******************************************************************************/
typedef GT_STATUS (*CMD_APP_BOBK_PORTS_CONFIG_FUNC)
(
    IN  GT_U8                           devNumber,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  GT_BOOL                         powerUp,
    IN  GT_U32                          squelch,
    IN  GT_U32                          numOfPorts,
    ...
);


/*******************************************************************************
* gtAppDemoXcat2StackPortsModeSpeedSet
*
* DESCRIPTION:
*       Example of configuration sequence for stack ports of xcat2
*
* INPUTS:
*       devNum   - device number
*       portNum  - port number (or CPU port)
*       ifMode   - Interface mode.
*       speed    - port speed
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*       GT_HW_ERROR - hw error
*       GT_BAD_PARAM - device not exists
*       GT_NOT_SUPPORTED - wrong ifMode,speed
*       GT_NOT_APPLICABLE_DEVICE - wrong devFamily
*
* COMMENTS:
*       At this moment supported:
*           CPSS_PORT_INTERFACE_MODE_SGMII_E        10M, 100M, 1G, 2.5G
*           CPSS_PORT_INTERFACE_MODE_1000BASE_X_E   1G
*           CPSS_PORT_INTERFACE_MODE_100BASE_FX_E   100M
*
*******************************************************************************/
typedef GT_STATUS (*CMD_APP_XCAT2_STACK_PORTS_MODE_SPEED_SET_FUNC)
(
    IN  GT_U8     devNum,
    IN  GT_U8     portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT   ifMode,
    IN  CPSS_PORT_SPEED_ENT  speed
);

/*******************************************************************************
* gtAppDemoXcat2SfpPortEnable
*
* DESCRIPTION:
*       This function configures MUX switching PHYs to SFPs to enable testing
*       of 2.5G mode on network ports
*
* INPUTS:
*       devNum  - device number
*       portNum - port number (just network ports)
*       enable  -   GT_TRUE  - use SFP port
*                   GT_FALSE - use PHY port
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*       GT_HW_ERROR - hw error
*       GT_BAD_PARAM - wrong device or port
*       GT_NOT_APPLICABLE_DEVICE - wrong devFamily
*
* COMMENTS:
*       Enable SFP must be done only after port configured to one of appropriate
*       modes i.e. 1000BaseX, SGMII 2.5G or SGMII 1G (while in case of SGMII 1G
*       on SFP port to get link you must enable ForceLinkPass and inband autoneg.
*       bypass, because SGMII 1G supposed to work over PHY).
*       And if you want to disable SFP and go back to default (PHY) you must
*       first configure QSGMII or other mode supported by regular network ports
*       on given device and only then call this function.
*
*******************************************************************************/
typedef GT_STATUS (*CMD_APP_XCAT2_SFP_PORT_ENABLE_FUNC)
(
    IN  GT_U8     devNum,
    IN  GT_U8     portNum,
    IN  GT_BOOL   enable
);

/*******************************************************************************
* appDemoDxChHsuOldImagePreUpdatePreparation
*
* DESCRIPTION:
*       This function perform following steps of HSU process:
*       1. Disable interrupts
*       2. Set hsuAuFuRxTxMode
*       3. export of all hsu datatypes
*       4. warm restart
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       isMultipleIteration - GT_TRUE - multiIteration HSU operation
*                             GT_FALSE - single iteration HSU operation
*       origIterationSize   - hsu iteration size
*        hsuAuFuRxTxMode    - hsu mode defines if application want to process
*                             messages arrived during HSU
*       systemRecoveryMode  - hsu mode defines if application want to process
*                             messages arrived during HSU
*
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong parameters
*       GT_NO_RESOURCE           - no place to save the request
*
* COMMENTS:
*       None.
*
*******************************************************************************/
typedef GT_STATUS (*CMD_APP_DXCH_EXTERNAL_HSU_OLD_IMAGE_PRE_UPDATE_PREPARATION_FUNC)
(
    IN GT_BOOL                               isMultipleIteration,
    IN GT_U32                                origIterationSize,
    IN CPSS_SYSTEM_RECOVERY_MODE_STC         systemRecoveryMode
);

/*******************************************************************************
* appDemoDxChHsuNewImageUpdateAndSynch
*
* DESCRIPTION:
*       This function perform following steps of HSU process:
*       1. import of all hsu datatypes
*       2. setting HSU state in "HSU is complete"
*       4. open pp interrupts
*       3. enable interrupts
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       isMultipleIteration - GT_TRUE - multiIteration HSU operation
*                             GT_FALSE - single iteration HSU operation
*       origIterationSize   - hsu iteration size
*       hsuDataSize         - hsu import data size
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong parameters
*       GT_NO_RESOURCE           - no place to save the request
*
* COMMENTS:
*       When appDemoDxChHsuOldImagePreUpdatePreparation is finished and new image
*       have arisen after warm restart, user should perform
*       cpssHsuStateSet(CPSS_HSU_STATE_IN_PROGRESS_E), cpssInitSystem and only
*       after that call appDemoDxChHsuNewImageUpdateAndSynch.
*
*******************************************************************************/
typedef GT_STATUS (*CMD_APP_DXCH_EXTERNAL_HSU_NEW_IMAGE_UPDATE_AND_SYNCH_FUNC)
(
    IN GT_BOOL                               isMultipleIteration,
    IN GT_U32                                origIterationSize,
    IN GT_U32                                hsuDataSize
);

#endif /*CHX_FAMILY*/

#if defined(CHX_FAMILY) || defined (PX_FAMILY)


/*
 * typedef: struct CMD_PORT_FWS_RATE_STC
 *
 * Description:  structure to hold RX/TX FWS port rate
 *
 * Enumerations:
 *         rxPacketRate - RX packet rate
 *         txPacketRate - TX packet rate
 *
 */
typedef struct{
    GT_U32  rxPacketRate;
    GT_U32  txPacketRate;
}CMD_PORT_FWS_RATE_STC;

/*******************************************************************************
* portRateTableGet
*
* DESCRIPTION:
*       Calculates and stores RX/TX rate for all ports
*
* INPUTS:
*       dev            - device number
*       arrSize        - DB array size
*
* OUTPUTS:
*       rateDbPtr      - (pointer to) DB of rates
*
* RETURNS:
*       GT_OK - on success
*
* COMMENTS:
*       None.
*
*******************************************************************************/
typedef GT_STATUS (*CMD_APP_PORT_RATE_TABLE_GET_FUNC)
(
    IN  GT_U8 devNum,
    IN  GT_U32 arrSize,
    OUT CMD_PORT_FWS_RATE_STC *rateDbPtr
);

#endif/*CHX_FAMILY or PX_FAMILY*/

/*******************************************************************************
* allowProcessingOfAuqMessages
*
* DESCRIPTION:
*     Function to allow set the flag of : allowProcessingOfAuqMessages
*
* INPUTS:
*       enable - GT_TRUE: allow the processing of the AUQ messages
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
typedef GT_STATUS (*CMD_APP_ALLOW_PROCESSING_OF_AUQ_MESSAGES_FUNC)
(
    IN  GT_BOOL enable
);

/******************************************************************************
* appDemoTraceHwAccessEnable
*
* DESCRIPTION:
*       Trace HW read access information.
*
* INPUTS:
*       devNum      - PP device number
*       accessType  - access type: read or write
*       enable      - GT_TRUE: enable tracing for given access type
*                     GT_FALSE: disable tracing for given access type
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK        - on success
*       GT_BAD_PARAM - on bad access type
*
* COMMENTS:
*       None.
*
******************************************************************************/
typedef GT_STATUS (*CMD_APP_TRACE_HW_ACCESS_ENABLE_FUNC)
(
    IN GT_U8            devNum,
    IN GT_U32           accessType,
    IN GT_BOOL          enable
);

/******************************************************************************
* appDemoTraceHwAccessOutputModeSet
*
* DESCRIPTION:
*       Set output tracing mode.
*
* INPUTS:
*       mode        - output tracing mode: print, printSync or store.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK        - on success
*       GT_BAD_STATE - on bad state
*       GT_BAD_PARAM - on bad mode
*
* COMMENTS:
*       It is not allowed to change mode, while one of the HW Access DB
*       is enabled.
*
******************************************************************************/
typedef GT_STATUS (*CMD_APP_TRACE_HW_ACCESS_OUTPUT_MODE_SET_FUNC)
(
    IN GT_U32   mode
);

/*******************************************************************************
* osMemStartHeapAllocationCounter
*
* DESCRIPTION:
*        set the current value of heap allocated bytes to the allocation counter
*
* INPUTS:
*
*
* OUTPUTS:
*       None.
*
* RETURNS:
*
*
* COMMENTS:
*
*******************************************************************************/
typedef GT_VOID (*CMD_APP_START_HEAP_ALLOC_COUNTER_FUNC)(GT_VOID);

/*******************************************************************************
* appDemoEventFatalErrorEnable
*
* DESCRIPTION:
*       Set fatal error handling type.
*
* INPUTS:
*       fatalErrorType  - fatal error handling type
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_BAD_PARAM - on wrong fatalErrorType
*
* COMMENTS:
*       None.
*
*******************************************************************************/
typedef GT_STATUS (*CMD_APP_EVANT_FATAL_ERROR_ENABLE_FUNC)
(
    IN GT_32 fatalErrorType /* real type is enum APP_DEMO_FATAL_ERROR_TYPE */
);

/*******************************************************************************
* cmdAppDemoEventsDataBaseGet
*
* DESCRIPTION:
*       get the events data base
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       eventCounterBlockGet - dataBase structure
*
* RETURNS:
*       GT_OK   - on success
*
*
* COMMENTS:
*       None.
*
*******************************************************************************/

typedef GT_STATUS (*CMD_APP_EVANT_DATA_BASE_GET_FUNC)
(
    OUT GT_U32 ***eventCounterBlockGet
);

/*******************************************************************************
* osMemGetHeapAllocationCounter
*
* DESCRIPTION:
*        returns the delta of current allocated bytes number and the value of allocation counter set by
*        preveous startHeapAllocationCounter() function
*
* INPUTS:
*
*
* OUTPUTS:
*       None.
*
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
typedef GT_U32 (*CMD_APP_GET_HEAP_ALLOC_COUNTER_FUNC)(GT_VOID);

/*******************************************************************************
* appDemoTrunkCascadeTrunkPortsSet
*
* DESCRIPTION:
*       The function checks does input parameters belong to uplink ports and
*       configures relay port trunks and source ID accordingly.
*
* INPUTS:
*
*       devNum      - device number
*       trunkId     - trunk id
*       portsMembersPtr - (pointer to) local ports bitmap to be members of the
*                   cascade trunk.
*                   NULL - meaning that the trunk-id is 'invalidated' and
*                          trunk-type will be changed to : CPSS_TRUNK_TYPE_FREE_E
*                   not-NULL - meaning that the trunk-type will be : CPSS_TRUNK_TYPE_CASCADE_E
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK              - on success.
*       GT_FAIL            - on error.
*       GT_NOT_INITIALIZED - the trunk library was not initialized for the device
*       GT_HW_ERROR        - on hardware error
*       GT_OUT_OF_RANGE    - there are ports in the bitmap that not supported by
*                            the device.
*       GT_BAD_PARAM       - bad device number , or bad trunkId number , or number
*                            of ports (in the bitmap) larger then the number of
*                            entries in the 'Designated trunk table'
*       GT_BAD_STATE       - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_CASCADE_E
*       GT_ALREADY_EXIST   - one of the members already exists in another trunk ,
*                            or this trunk hold members defined using cpssDxChTrunkMembersSet(...)
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
* COMMENTS:
*
*******************************************************************************/
typedef GT_STATUS (*CMD_APP_TRUNK_CASCADE_TRUNK_PORTS_SET_FUNC)
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_PORTS_BMP_STC       *portsMembersPtr
);

/*******************************************************************************
* appDemoTrunkMembersSet
*
* DESCRIPTION:
*       The function checks does input parameters belong to uplink ports and
*       configures relay port trunks and source ID accordingly.
*
* INPUTS:
*
*       devNum      - device number
*       trunkId     - trunk id
*       enabledMembersArray - (array of) members to set in this trunk as enabled
*                     members .
*                    (this parameter ignored if numOfEnabledMembers = 0)
*       numOfEnabledMembers - number of enabled members in the array.
*       disabledMembersArray - (array of) members to set in this trunk as disabled
*                     members .
*                    (this parameter ignored if numOfDisabledMembers = 0)
*       numOfDisabledMembers - number of disabled members in the array.
*
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_FAIL                  - on error.
*       GT_NOT_INITIALIZED -the trunk library was not initialized for the device
*       GT_HW_ERROR              - on hardware error
*       GT_OUT_OF_RANGE - when the sum of number of enabled members + number of
*                         disabled members exceed the number of maximum number
*                         of members in trunk (total of 0 - 8 members allowed)
*       GT_BAD_PARAM             - bad device number , or
*                      bad trunkId number , or
*                      bad members parameters :
*                          (device & 0xE0) != 0  means that the HW can't support
*                                              this value , since HW has 5 bit
*                                              for device number
*                          (port & 0xC0) != 0  means that the HW can't support
*                                              this value , since HW has 6 bit
*                                              for port number
*       GT_BAD_PTR               - one of the parameters in NULL pointer
*       GT_ALREADY_EXIST - one of the members already exists in another trunk
*       GT_BAD_STATE       - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
* COMMENTS:
*
*******************************************************************************/
typedef GT_STATUS (*CMD_APP_TRUNK_MEMBERS_SET_FUNC)
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN GT_U32                   numOfEnabledMembers,
    IN CPSS_TRUNK_MEMBER_STC    enabledMembersArray[],
    IN GT_U32                   numOfDisabledMembers,
    IN CPSS_TRUNK_MEMBER_STC    disabledMembersArray[]
);

/*******************************************************************************
* appDemoTrunkMemberAdd
*
* DESCRIPTION:
*       The function checks does input parameters belong to uplink ports and
*       configures relay port trunks and source ID accordingly.
*
* INPUTS:
*       devNum      - the device number on which to add member to the trunk
*       trunkId     - the trunk id.
*       memberPtr   - (pointer to)the member to add to the trunk.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_NOT_INITIALIZED -the trunk library was not initialized for the device
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - bad device number , or
*                      bad trunkId number , or
*                      bad member parameters :
*                          (device & 0xE0) != 0  means that the HW can't support
*                                              this value , since HW has 5 bit
*                                              for device number
*                          (port & 0xC0) != 0  means that the HW can't support
*                                              this value , since HW has 6 bit
*                                              for port number
*       GT_BAD_PTR               - one of the parameters in NULL pointer
*       GT_ALREADY_EXIST         - this member already exists in another trunk.
*       GT_FULL - trunk already contains maximum supported members
*       GT_BAD_STATE       - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*
*******************************************************************************/
typedef GT_STATUS (*CMD_APP_TRUNK_MEMBER_ADD_FUNC)
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
);

/*******************************************************************************
* appDemoTrunkMemberDisable
*
* DESCRIPTION:
*       The function checks does input parameters belong to uplink ports and
*       configures relay port trunks and source ID accordingly.
*
* INPUTS:
*       devNum      - the device number on which to disable member in the trunk
*       trunkId     - the trunk id.
*       memberPtr   - (pointer to)the member to disable in the trunk.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_NOT_INITIALIZED -the trunk library was not initialized for the device
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - bad device number , or
*                      bad trunkId number , or
*                      bad member parameters :
*                          (device & 0xE0) != 0  means that the HW can't support
*                                              this value , since HW has 5 bit
*                                              for device number
*                          (port & 0xC0) != 0  means that the HW can't support
*                                              this value , since HW has 6 bit
*                                              for port number
*       GT_BAD_PTR               - one of the parameters in NULL pointer
*       GT_NOT_FOUND - this member not found (member not exist) in the trunk
*       GT_BAD_STATE       - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
typedef GT_STATUS (*CMD_APP_TRUNK_MEMBER_DISABLE_FUNC)
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
);

/*******************************************************************************
* appDemoTrunkMemberEnable
*
* DESCRIPTION:
*       The function checks does input parameters belong to uplink ports and
*       configures relay port trunks and source ID accordingly.
*
* INPUTS:
*       devNum      - the device number on which to enable member in the trunk
*       trunkId     - the trunk id.
*       memberPtr   - (pointer to)the member to enable in the trunk.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_NOT_INITIALIZED -the trunk library was not initialized for the device
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - bad device number , or
*                      bad trunkId number , or
*                      bad member parameters :
*                          (device & 0xE0) != 0  means that the HW can't support
*                                              this value , since HW has 5 bit
*                                              for device number
*                          (port & 0xC0) != 0  means that the HW can't support
*                                              this value , since HW has 6 bit
*                                              for port number
*       GT_BAD_PTR               - one of the parameters in NULL pointer
*       GT_NOT_FOUND - this member not found (member not exist) in the trunk
*       GT_BAD_STATE       - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
typedef GT_STATUS (*CMD_APP_TRUNK_MEMBER_ENABLE_FUNC)
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
);

/*******************************************************************************
* appDemoTrunkMemberRemove
*
* DESCRIPTION:
*       The function checks does input parameters belong to uplink ports and
*       configures relay port trunks and source ID accordingly.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on error
*       GT_NOT_INITIALIZED -the trunk library was not initialized for the device
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - bad device number , or
*                      bad trunkId number , or
*                      bad member parameters :
*                          (device & 0xE0) != 0  means that the HW can't support
*                                              this value , since HW has 5 bit
*                                              for device number
*                          (port & 0xC0) != 0  means that the HW can't support
*                                              this value , since HW has 6 bit
*                                              for port number
*       GT_BAD_PTR               - one of the parameters in NULL pointer
*       GT_BAD_STATE       - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
typedef GT_STATUS (*CMD_APP_TRUNK_MEMBER_REMOVE_FUNC)
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
);

/*******************************************************************************
* appDemoLion2TrunkUpdatedPortsGet
*
* DESCRIPTION:
*       Get added and removed ports for specified trunk
*
* INPUTS:
*       devNum - device number.
*       trunkId - trunk ID.
*       portsMembersPtr - (pointer to) trunk member ports
*
* OUTPUTS:
*       addedToTrunkPortsBmpPtr - (pointer to) added ports
*       removedFromTrunkPortsBmpPtr - (pointer to) removed ports
*
* RETURNS:
*
* COMMENTS:
*
*
*******************************************************************************/
typedef GT_VOID (*CMD_APP_TRUNK_UPDATED_PORTS_GET_FUNC)
(
    IN GT_U8        devNum,
    IN GT_TRUNK_ID  trunkId,
    IN CPSS_PORTS_BMP_STC * portsMembersPtr,
    OUT CPSS_PORTS_BMP_STC * addedToTrunkPortsBmpPtr,
    OUT CPSS_PORTS_BMP_STC * removedFromTrunkPortsBmpPtr
);


/* CMD_OS_FUNC_BIND_APP_DB_STC -
*    structure that hold the "application DB" functions needed be bound to CMD.
*
*/
typedef struct{
    CMD_APP_DB_ENTRY_ADD_FUNC       cmdAppDbEntryAdd;
    CMD_APP_DB_ENTRY_GET_FUNC       cmdAppDbEntryGet;
    CMD_APP_DB_DUMP_FUNC            cmdAppDbDump;
    CMD_APP_PP_CONFIG_GET_FUNC      cmdAppPpConfigGet;
    CMD_APP_INIT_SYSTEM_FUNC        cmdInitSystem;
    CMD_APP_INIT_SYSTEM_GET_FUNC    cmdInitSystemGet;
    CMD_APP_RESET_SYSTEM_FUNC       cmdResetSystem;
    CMD_APP_INIT_CONFI_FUNC         cmdInitConfi;
    CMD_APP_SHOW_BOARDS_LIST_FUNC   cmdAppShowBoardsList;
    CMD_APP_IS_SYSTEM_INITIALIZED_FUNC  cmdAppIsSystemInitialized;
    CMD_APP_PP_CONFIG_PRINT_FUNC  cmdAppPpConfigPrint;
    CMD_APP_ALLOW_PROCESSING_OF_AUQ_MESSAGES_FUNC   cmdAppAllowProcessingOfAuqMessages;
    CMD_APP_TRACE_HW_ACCESS_ENABLE_FUNC             cmdAppTraceHwAccessEnable;
    CMD_APP_TRACE_HW_ACCESS_OUTPUT_MODE_SET_FUNC    cmdAppTraceHwAccessOutputModeSet;
    CMD_APP_START_HEAP_ALLOC_COUNTER_FUNC           cmdAppStartHeapAllocCounter;
    CMD_APP_GET_HEAP_ALLOC_COUNTER_FUNC             cmdAppGetHeapAllocCounter;
    CMD_APP_EVANT_FATAL_ERROR_ENABLE_FUNC           cmdAppEventFatalErrorEnable;
    CMD_APP_EVANT_DATA_BASE_GET_FUNC                cmdAppDemoEventsDataBaseGet;

#if defined(CHX_FAMILY) || defined(PX_FAMILY)
    CMD_APP_PORT_RATE_TABLE_GET_FUNC                                  cmdPortRateTableGet;
#endif

#ifdef CHX_FAMILY
    CMD_APP_LION_PORT_MODE_SPEED_SET_FUNC                             cmdAppLionPortModeSpeedSet;
    CMD_APP_BC2_PORTS_CONFIG_FUNC                                     cmdAppBc2PortsConfig;
    CMD_APP_BOBK_PORTS_CONFIG_FUNC                                    cmdAppBobkPortsConfig;
    CMD_APP_XCAT2_STACK_PORTS_MODE_SPEED_SET_FUNC                     cmdAppXcat2StackPortsModeSpeedSet;
    CMD_APP_XCAT2_SFP_PORT_ENABLE_FUNC                                cmdAppXcat2SfpPortEnable;
    CMD_APP_DXCH_EXTERNAL_HSU_OLD_IMAGE_PRE_UPDATE_PREPARATION_FUNC   cmdAppDxChHsuOldImagePreUpdatePreparation;
    CMD_APP_DXCH_EXTERNAL_HSU_NEW_IMAGE_UPDATE_AND_SYNCH_FUNC         cmdAppDxChHsuNewImageUpdateAndSynch;

#endif /*CHX_FAMILY*/
}CMD_FUNC_BIND_APP_DB_STC;



/* CPSS_OS_FUNC_BIND_STC -
*    structure that hold the other "os" functions needed be bound to CMD.
*    and not defined for cpss
*
*       osSocketsBindInfo -  set of call back functions -
*                        sockets manipulation
*       osTasksBindInfo -  set of call back functions -
*                        tasks manipulation
*       osPoolsBindInfo -  set of call back functions -
*                        pools manipulation
*       osCpuEthernetPortBindInfo - set of call back functions
*                        CPU Ethernet port manipulation
*/
typedef struct{
    CMD_OS_FUNC_BIND_SOCKETS_STC    osSocketsBindInfo;
    CMD_OS_FUNC_BIND_TASKS_STC      osTasksBindInfo;
    CMD_OS_FUNC_BIND_POOLS_STC      osPoolsBindInfo;
}CMD_OS_FUNC_BIND_EXTRA_STC;


/* CPSS_FUNC_BIND_STC -
*    structure that hold the other  functions needed be bound to CMD.
*    and not defined for cpss
*
*       osCpuEthernetPortBindInfo - set of call back functions
*                        CPU Ethernet port manipulation
*       eventsBindInfo - set of call back functions
*                        events manipulation
*       appDbBindInfo  - set of call back functions
*                        application DB manipulation
*/
typedef struct{
    CMD_FUNC_BIND_CPU_ETHERNET_PORT_STC cpuEthernetPortBindInfo;
    CMD_FUNC_BIND_EVENTS_STC            eventsBindInfo;
    CMD_FUNC_BIND_APP_DB_STC            appDbBindInfo;
}CMD_FUNC_BIND_EXTRA_STC;


/**
* @internal applicationExtServicesBind function
* @endinternal
*
* @brief   the mainCmd calls this function , so the application (that implement
*         this function) will bind the mainCmd (and GaltisWrapper) with OS ,
*         external driver functions and other application functions
*
* @param[out] extDrvFuncBindInfoPtr    - (pointer to) set of external driver call back functions
* @param[out] osFuncBindPtr            - (pointer to) set of OS call back functions
* @param[out] osExtraFuncBindPtr       - (pointer to) set of extra OS call back functions (that CPSS not use)
* @param[out] extraFuncBindPtr         - (pointer to) set of extra call back functions (that CPSS not use) (non OS functions)
* @param[out] traceFuncBindPtr         - (pointer to) set of Trace call back functions
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note This function must be implemented by the Application !!!
*
*/
GT_STATUS   applicationExtServicesBind(
    OUT CPSS_EXT_DRV_FUNC_BIND_STC   *extDrvFuncBindInfoPtr,
    OUT CPSS_OS_FUNC_BIND_STC        *osFuncBindPtr,
    OUT CMD_OS_FUNC_BIND_EXTRA_STC   *osExtraFuncBindPtr,
    OUT CMD_FUNC_BIND_EXTRA_STC      *extraFuncBindPtr,
    OUT CPSS_TRACE_FUNC_BIND_STC     *traceFuncBindPtr
);


#ifndef __gtOsSemh
typedef GT_UINTPTR GT_SEM;
typedef GT_UINTPTR GT_MUTEX;
#endif /*__gtOsSemh*/

/************* global *********************************************************/

extern CPSS_OS_INET_NTOHL_FUNC  cmdOsNtohl;
extern CPSS_OS_INET_HTONL_FUNC  cmdOsHtonl;
extern CPSS_OS_INET_NTOHS_FUNC  cmdOsNtohs;
extern CPSS_OS_INET_HTONS_FUNC  cmdOsHtons;
extern CPSS_OS_INET_NTOA_FUNC   cmdOsInetNtoa;

extern CPSS_OS_IO_BIND_STDOUT_FUNC cmdOsBindStdOut;
extern CPSS_OS_IO_PRINTF_FUNC      cmdOsPrintf;
extern CPSS_OS_IO_SPRINTF_FUNC     cmdOsSprintf;
extern CPSS_OS_IO_PRINT_SYNC_FUNC  cmdOsPrintSync;
extern CPSS_OS_IO_GETS_FUNC        cmdOsGets;
extern CPSS_OS_IO_FOPEN_FUNC       cmdOsFopen;
extern CPSS_OS_IO_FCLOSE_FUNC      cmdOsFclose;
extern CPSS_OS_IO_REWIND_FUNC      cmdOsRewind;
extern CPSS_OS_IO_FPRINTF_FUNC     cmdOsFprintf;
extern CPSS_OS_IO_FGETS_FUNC       cmdOsFgets;

extern CPSS_OS_BZERO_FUNC             cmdOsBzero;
extern CPSS_OS_MEM_SET_FUNC           cmdOsMemSet;
extern CPSS_OS_MEM_CPY_FUNC           cmdOsMemCpy;
extern CPSS_OS_MEM_CMP_FUNC           cmdOsMemCmp;
extern CPSS_OS_STATIC_MALLOC_FUNC     cmdOsStaticMalloc;
/*extern CPSS_OS_STATIC_MALLOC_FUNC     cpssOsStaticMalloc; -- not to be used in CPSS !!*/
#ifdef OS_MALLOC_MEMORY_LEAKAGE_DBG
    extern CPSS_OS_MALLOC_FUNC            cmdOsMalloc_MemoryLeakageDbg;
    extern CPSS_OS_REALLOC_FUNC           cmdOsRealloc_MemoryLeakageDbg;
    extern CPSS_OS_FREE_FUNC              cmdOsFree_MemoryLeakageDbg;
    #define cmdOsMalloc(_size)            cmdOsMalloc_MemoryLeakageDbg(_size           ,__FILE__,__LINE__)
    #define cmdOsRealloc(_ptr,_size)      cmdOsRealloc_MemoryLeakageDbg(_ptr,_size     ,__FILE__,__LINE__)
    #define cmdOsFree(_memblock)          cmdOsFree_MemoryLeakageDbg(_memblock         ,__FILE__,__LINE__)
#else  /*!OS_MALLOC_MEMORY_LEAKAGE_DBG*/
    extern CPSS_OS_MALLOC_FUNC            cmdOsMalloc;
    extern CPSS_OS_REALLOC_FUNC           cmdOsRealloc;
    extern CPSS_OS_FREE_FUNC              cmdOsFree;
#endif /*!OS_MALLOC_MEMORY_LEAKAGE_DBG*/

extern CPSS_OS_MUTEX_CREATE_FUNC        cmdOsMutexCreate;
extern CPSS_OS_MUTEX_DELETE_FUNC        cmdOsMutexDelete;
extern CPSS_OS_MUTEX_LOCK_FUNC          cmdOsMutexLock;
extern CPSS_OS_MUTEX_UNLOCK_FUNC        cmdOsMutexUnlock;

extern CPSS_OS_SIG_SEM_BIN_CREATE_FUNC  cmdOsSigSemBinCreate;
extern CPSS_OS_SIG_SEM_DELETE_FUNC      cmdOsSigSemDelete;
extern CPSS_OS_SIG_SEM_WAIT_FUNC        cmdOsSigSemWait;
extern CPSS_OS_SIG_SEM_SIGNAL_FUNC      cmdOsSigSemSignal;

extern CPSS_OS_STR_LEN_FUNC     cmdOsStrlen;
extern CPSS_OS_STR_CPY_FUNC     cmdOsStrCpy;
extern CPSS_OS_STR_CHR_FUNC     cmdOsStrChr;
extern CPSS_OS_STR_CHR_FUNC     cmdOsStrRevChr;
extern CPSS_OS_STR_CMP_FUNC     cmdOsStrCmp;
extern CPSS_OS_STR_CAT_FUNC     cmdOsStrCat;
extern CPSS_OS_STR_N_CAT_FUNC   cmdOsStrNCat;
extern CPSS_OS_TO_UPPER_FUNC    cmdOsToUpper;
extern CPSS_OS_STR_TO_32_FUNC   cmdOsStrTo32;
extern CPSS_OS_STR_TO_U32_FUNC  cmdOsStrToU32;
extern CPSS_OS_STR_TOL_FUNC     cmdOsStrTol;

extern CPSS_OS_TIME_WK_AFTER_FUNC  cmdOsTimerWkAfter;
extern CPSS_OS_TIME_TICK_GET_FUNC  cmdOsTickGet;
extern CPSS_OS_TIME_GET_FUNC       cmdOsTime;

extern CPSS_OS_TASK_CREATE_FUNC    cmdOsTaskCreate;
extern CPSS_OS_TASK_DELETE_FUNC    cmdOsTaskDelete;

extern CPSS_TRACE_HW_ACCESS_WRITE_FUNC  cmdTraceHwAccessWrite;
extern CPSS_TRACE_HW_ACCESS_READ_FUNC   cmdTraceHwAccessRead;
extern CPSS_TRACE_HW_ACCESS_DELAY_FUNC   cmdTraceHwAccessDelay;

extern CMD_OS_TASK_GET_SELF_FUN    cmdOsTaskGetSelf;
extern CMD_OS_TASK_SET_TASK_PRIOR_FUNC cmdOsSetTaskPrior;

extern CMD_OS_SOCKET_TCP_CREATE_FUN        cmdOsSocketTcpCreate;
extern CMD_OS_SOCKET_UDP_CREATE_FUN        cmdOsSocketUdpCreate;
extern CMD_OS_SOCKET_TCP_DESTROY_FUN       cmdOsSocketTcpDestroy;
extern CMD_OS_SOCKET_UDP_DESTROY_FUN       cmdOsSocketUdpDestroy;
extern CMD_OS_SOCKET_CREATE_ADDR_FUN       cmdOsSocketCreateAddr;
extern CMD_OS_SOCKET_DESTROY_ADDR_FUN      cmdOsSocketDestroyAddr;
extern CMD_OS_SOCKET_BIND_FUN              cmdOsSocketBind;
extern CMD_OS_SOCKET_LISTEN_FUN            cmdOsSocketListen;
extern CMD_OS_SOCKET_ACCEPT_FUN            cmdOsSocketAccept;
extern CMD_OS_SOCKET_CONNECT_FUN           cmdOsSocketConnect;
extern CMD_OS_SOCKET_SET_NON_BLOCK_FUN     cmdOsSocketSetNonBlock;
extern CMD_OS_SOCKET_SET_BLOCK_FUN         cmdOsSocketSetBlock;
extern CMD_OS_SOCKET_SEND_FUN              cmdOsSocketSend;
extern CMD_OS_SOCKET_SEND_TO_FUN           cmdOsSocketSendTo;
extern CMD_OS_SOCKET_RECV_FUN              cmdOsSocketRecv;
extern CMD_OS_SOCKET_RECV_FROM_FUN         cmdOsSocketRecvFrom;
extern CMD_OS_SOCKET_SET_SOCKET_NO_LINGER_FUN cmdOsSocketSetSocketNoLinger;
extern CMD_OS_SOCKET_EXTRACT_IP_ADDR_FROM_SOCKET_ADDR_FUN cmdOsSocketExtractIpAddrFromSocketAddr;
extern CMD_OS_SOCKET_GET_SOCKET_ADDR_SIZE_FUN cmdOsSocketGetSocketAddrSize;
extern CMD_OS_SOCKET_SHUT_DOWN_FUN         cmdOsSocketShutDown;

extern CMD_OS_SELECT_CREATE_SET_FUN        cmdOsSelectCreateSet;
extern CMD_OS_SELECT_ERASE_SET_FUN         cmdOsSelectEraseSet;
extern CMD_OS_SELECT_ZERO_SET_FUN          cmdOsSelectZeroSet;
extern CMD_OS_SELECT_ADD_FD_TO_SET_FUN     cmdOsSelectAddFdToSet;
extern CMD_OS_SELECT_CLEAR_FD_FROM_SET_FUN cmdOsSelectClearFdFromSet;
extern CMD_OS_SELECT_IS_FD_SET_FUN         cmdOsSelectIsFdSet;
extern CMD_OS_SELECT_COPY_SET_FUN          cmdOsSelectCopySet;
extern CMD_OS_SELECT_FUN                   cmdOsSelect;
extern CMD_OS_SOCKET_GET_SOCKET_FD_SET_SIZE_FUN cmdOsSocketGetSocketFdSetSize;


extern CMD_OS_POOL_CREATE_POOL_FUNC        cmdOsPoolCreatePool;
extern CMD_OS_POOL_CREATE_DMA_POOL_FUNC    cmdOsPoolCreateDmaPool;
extern CMD_OS_POOL_RE_CREATE_POOL_FUNC     cmdOsPoolReCreatePool;
extern CMD_OS_POOL_DELETE_POOL_FUNC        cmdOsPoolDeletePool;
extern CMD_OS_POOL_GET_BUF_FUNC            cmdOsPoolGetBuf;
extern CMD_OS_POOL_FREE_BUF_FUNC           cmdOsPoolFreeBuf;
extern CMD_OS_POOL_GET_BUF_SIZE_FUNC       cmdOsPoolGetBufSize;
extern CMD_OS_POOL_EXPAND_POOL_FUNC        cmdOsPoolExpandPool;
extern CMD_OS_POOL_GET_BUF_FREE_CNT_FUNC   cmdOsPoolGetBufFreeCnt;
extern CMD_OS_POOL_PRINT_STATS_FUNC        cmdOsPoolPrintStats;

extern CMD_CPU_ETHERNET_IS_CPU_ETHER_PORT_USED cmdIsCpuEtherPortUsed;

extern CMD_CPSS_EVENT_BIND_FUNC            cmdCpssEventBind;
extern CMD_CPSS_EVENT_SELECT_FUNC          cmdCpssEventSelect;
extern CMD_CPSS_EVENT_RECV_FUNC            cmdCpssEventRecv;
extern CMD_CPSS_EVENT_DEVICE_MASK_SET_FUNC cmdCpssEventDeviceMaskSet;



extern CMD_APP_DB_ENTRY_ADD_FUNC       cmdAppDbEntryAdd;
extern CMD_APP_DB_ENTRY_GET_FUNC       cmdAppDbEntryGet;
extern CMD_APP_DB_DUMP_FUNC            cmdAppDbDump;
extern CMD_APP_PP_CONFIG_GET_FUNC      cmdAppPpConfigGet;
extern CMD_APP_INIT_SYSTEM_FUNC        cmdInitSystem;
extern CMD_APP_INIT_SYSTEM_GET_FUNC    cmdInitSystemGet;
extern CMD_APP_RESET_SYSTEM_FUNC       cmdResetSystem;
extern CMD_APP_INIT_CONFI_FUNC         cmdInitConfi;
extern CMD_APP_SHOW_BOARDS_LIST_FUNC   cmdAppShowBoardsList;
extern CMD_APP_IS_SYSTEM_INITIALIZED_FUNC  cmdAppIsSystemInitialized;
extern CMD_APP_PP_CONFIG_PRINT_FUNC  cmdAppPpConfigPrint;
extern CMD_APP_ALLOW_PROCESSING_OF_AUQ_MESSAGES_FUNC    cmdAppAllowProcessingOfAuqMessages;
extern CMD_APP_TRACE_HW_ACCESS_ENABLE_FUNC              cmdAppTraceHwAccessEnable;
extern CMD_APP_TRACE_HW_ACCESS_OUTPUT_MODE_SET_FUNC     cmdAppTraceHwAccessOutputModeSet;
extern CMD_APP_START_HEAP_ALLOC_COUNTER_FUNC            cmdAppStartHeapAllocCounter;
extern CMD_APP_GET_HEAP_ALLOC_COUNTER_FUNC              cmdAppGetHeapAllocCounter;
extern CMD_APP_EVANT_FATAL_ERROR_ENABLE_FUNC            cmdAppEventFatalErrorEnable;
extern CMD_APP_EVANT_DATA_BASE_GET_FUNC                 cmdAppDemoEventsDataBaseGet;

#if defined(CHX_FAMILY) || defined(PX_FAMILY)
extern CMD_APP_PORT_RATE_TABLE_GET_FUNC                                    cmdPortRateTableGet;
#endif

#ifdef CHX_FAMILY
extern CMD_APP_LION_PORT_MODE_SPEED_SET_FUNC                               cmdAppLionPortModeSpeedSet;
extern CMD_APP_BC2_PORTS_CONFIG_FUNC                                       cmdAppBc2PortsConfig;
extern CMD_APP_BOBK_PORTS_CONFIG_FUNC                                      cmdAppBobkPortsConfig;
extern CMD_APP_XCAT2_STACK_PORTS_MODE_SPEED_SET_FUNC                       cmdAppXcat2StackPortsModeSpeedSet;
extern CMD_APP_XCAT2_SFP_PORT_ENABLE_FUNC                                  cmdAppXcat2SfpPortEnable;
extern CMD_APP_DXCH_EXTERNAL_HSU_OLD_IMAGE_PRE_UPDATE_PREPARATION_FUNC     cmdAppDxChHsuOldImagePreUpdatePreparation;
extern CMD_APP_DXCH_EXTERNAL_HSU_NEW_IMAGE_UPDATE_AND_SYNCH_FUNC           cmdAppDxChHsuNewImageUpdateAndSynch;
extern CMD_APP_TRUNK_CASCADE_TRUNK_PORTS_SET_FUNC                          cmdAppTrunkCascadeTrunkPortsSet;
extern CMD_APP_TRUNK_MEMBERS_SET_FUNC                                      cmdAppTrunkMembersSet;
extern CMD_APP_TRUNK_MEMBER_ADD_FUNC                                       cmdAppTrunkMemberAdd;
extern CMD_APP_TRUNK_MEMBER_DISABLE_FUNC                                   cmdAppTrunkMemberDisable;
extern CMD_APP_TRUNK_MEMBER_ENABLE_FUNC                                    cmdAppTrunkMemberEnable;
extern CMD_APP_TRUNK_MEMBER_REMOVE_FUNC                                    cmdAppTrunkMemberRemove;
extern CMD_APP_TRUNK_UPDATED_PORTS_GET_FUNC                                cmdAppTrunkUpdatedPortsGet;
#endif

/**
* @internal cmdInitExtServices function
* @endinternal
*
* @brief   commander external services initialization
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS cmdInitExtServices
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /*__cmdExtServices_h_*/


