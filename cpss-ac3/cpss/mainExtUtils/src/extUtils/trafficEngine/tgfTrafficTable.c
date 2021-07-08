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
* @file tgfTrafficTable.c
*
* @brief Manage the Rx Tx tables of the traffic sent from CPU and traffic
* received at the CPU
*
* @version   12
********************************************************************************
*/

#ifdef COW_INTERNAL_COMPILE
    /* those 3 are defined , so remove them */
    #undef EXMXPM_FAMILY
    #undef EX_FAMILY
    #undef CHX_FAMILY

    /* define only the tested one */
    /*#define CHX_FAMILY*/

    #undef IMPL_GALTIS
    #define IMPL_GALTIS
#endif /*COW_INTERNAL_COMPILE*/

#include <cpss/common/cpssTypes.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <extUtils/trafficEngine/tgfTrafficTable.h>
#include <extUtils/trafficEngine/tgfTrafficGenerator.h>
#include <gtOs/gtOsSem.h>
#include <extUtils/trafficEngine/prvTgfLog.h>
#include <extUtils/trafficEngine/private/prvTgfInternalDefs.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>

/******************************************************************************\
 *                              Define section                                *
\******************************************************************************/

#ifdef CHX_FAMILY
    #include <cpss/generic/cpssCommonDefs.h>

    #ifndef TGF_RX_PARAM_SIZE_DEFINED
        #include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h>

        static GT_U32 prvTgfDxChRxParamSize = sizeof(CPSS_DXCH_NET_RX_PARAMS_STC);
        #define TGF_RX_PARAM_SIZE_DEFINED
    #endif /* TGF_RX_PARAM_SIZE_DEFINED */
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    #ifndef TGF_RX_PARAM_SIZE_DEFINED
        #include <cpss/exMxPm/exMxPmGen/networkIf/cpssExMxPmNetIfTypes.h>

        static GT_U32 prvTgfExMxPmRxParamSize = sizeof(CPSS_EXMXPM_NET_RX_PARAMS_STC);
        #define TGF_RX_PARAM_SIZE_DEFINED
    #endif /* TGF_RX_PARAM_SIZE_DEFINED */
#endif /* EXMXPM_FAMILY */

#ifdef EX_FAMILY
    #ifndef TGF_RX_PARAM_SIZE_DEFINED
        #include <cpss/exMx/exMxGen/networkIf/cpssExMxGenNetIfTypes.h>

        static GT_U32 prvTgfExMxRxParamSize = sizeof(CPSS_EXMX_NET_RX_PARAMS_STC);
        #define TGF_RX_PARAM_SIZE_DEFINED
    #endif /* TGF_RX_PARAM_SIZE_DEFINED */
#endif /* EX_FAMILY */

#ifdef PX_FAMILY
    #ifndef TGF_RX_PARAM_SIZE_DEFINED
        #define TGF_RX_PARAM_SIZE_DEFINED
    #endif /* TGF_RX_PARAM_SIZE_DEFINED */
#endif /* EX_FAMILY */

#ifndef TGF_RX_PARAM_SIZE_DEFINED
    static GT_U32 prvTgfDxChRxParamSize = 0;
#endif /* TGF_RX_PARAM_SIZE_DEFINED */

#define TGF_RX_DEF_NUM_OF_ENTRIES_CNS 1024
static TGF_RX_ENTRY     prvTgfRxEntriesArr[TGF_RX_DEF_NUM_OF_ENTRIES_CNS];
static TGF_RX_MODE_ENT  prvTgfRxTableMode = TGF_RX_CYCLIC_E;
static GT_U32           prvTgfMaxRxBufferSize = TGF_RX_BUFFER_MAX_SIZE_CNS;
static GT_U32           prvTgfMaxNumOfRxEntries = TGF_RX_DEF_NUM_OF_ENTRIES_CNS;
static GT_U32           prvTgfIndexToInsertPacket = 0;
static GT_U32           prvTgfDoCapture = GT_FALSE;
static GT_BOOL          prvTgfIsInitialized = GT_FALSE;
static GT_U32           prvTgfGlobalRxParamSize = 0;
static GT_SEM           prvTgfTrafficRxSem = 0;
static GT_BOOL          prvTgfRxCbRegistered = GT_FALSE;
static GT_BOOL          prvTgfRxPacketDoCountOnly = GT_FALSE;
static GT_BOOL          prvTgfRxPacketDoRxToTx = GT_FALSE;
static volatile GT_U32           prvTgfRxPacketCount = 0;

/* the state of re-run variable */
TGF_RE_RUN_STATUS_ENT              prvTgfReRunVariable = TGF_RE_RUN_STATUS_DISABLED_E;

/* macro to check if library initialized */
#define TGF_RX_LIB_INIT_MAC                                                   \
            if (!prvTgfIsInitialized)                                         \
            {                                                                 \
                prvTgfTrafficTableRxLibInit();                                \
            }

/* macro to create semaphore */
#define TGF_RX_SEM_CREATE_MAC                                                 \
            if(prvTgfTrafficRxSem == 0)                                       \
            {                                                                 \
                osSemBinCreate("prvTgfTrafficRxSem", 1, &prvTgfTrafficRxSem); \
            }
/* macro to get semaphore */
#define TGF_RX_SEM_TAKE_MAC     osSemWait(prvTgfTrafficRxSem, OS_WAIT_FOREVER)
/* macro to release semaphore */
#define TGF_RX_SEM_GIVE_MAC     osSemSignal(prvTgfTrafficRxSem)


/******************************************************************************\
 *                     Extern function's declaration                          *
\******************************************************************************/

/******************************************************************************\
 *                     Private function's implementation                      *
\******************************************************************************/

/**
* @internal prvTgfTrafficTableRxPacketInfoFree function
* @endinternal
*
* @brief   Free the specific rx packet info, by calling to the registered callback
*
* @param[in] index                    -  to the prvTgfRxEntriesArr array
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfTrafficTableRxPacketInfoFree
(
    IN GT_U32           index
)
{
    if (index >= prvTgfMaxNumOfRxEntries)
    {
        return GT_BAD_PARAM;
    }

    if(prvTgfRxEntriesArr[index].packetBufferPtr)
    {
        cpssOsFree(prvTgfRxEntriesArr[index].packetBufferPtr);
        prvTgfRxEntriesArr[index].packetBufferPtr = NULL;
    }

    if(prvTgfRxEntriesArr[index].specificDeviceFormatPtr == NULL)
    {
        return GT_OK;
    }

    if(prvTgfRxEntriesArr[index].freeRxInfoFunc)
    {
        /* free by call back*/
        prvTgfRxEntriesArr[index].freeRxInfoFunc(prvTgfRxEntriesArr[index].specificDeviceFormatPtr);
    }
    else
    {
        /* free with no specific extra needs */
        cpssOsFree(prvTgfRxEntriesArr[index].specificDeviceFormatPtr);
    }

    /* set not used pointer */
    prvTgfRxEntriesArr[index].specificDeviceFormatPtr = NULL;

    return GT_OK;
}


/**
* @internal prvTgfTrafficTableRxPacketSystemReset function
* @endinternal
*
* @brief   Preparation for system reset
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_VOID prvTgfTrafficTableRxPacketSystemReset
(
    GT_VOID
)
{
    /* stop capture */
    (void)prvTgfTrafficTableRxStartCapture(GT_FALSE);
    /* clear table */
    (void)tgfTrafficTableRxPcktTblClear();
}

/**
* @internal prvTgfTrafficTableRxLibInit function
* @endinternal
*
* @brief   Initialize the rx library
*/
static GT_VOID prvTgfTrafficTableRxLibInit
(
    GT_VOID
)
{
    GT_U32 i = 0;

    if(GT_TRUE == prvTgfIsInitialized)
        return;

#ifdef CHX_FAMILY
    prvTgfGlobalRxParamSize = prvTgfDxChRxParamSize;
#elif defined EXMXPM_FAMILY
    prvTgfGlobalRxParamSize = prvTgfExMxPmRxParamSize;
#elif defined EX_FAMILY
    prvTgfGlobalRxParamSize = prvTgfExMxRxParamSize;
#endif

    /* create semaphore */
    TGF_RX_SEM_CREATE_MAC;
    for(i = 0; i < TGF_RX_DEF_NUM_OF_ENTRIES_CNS; i++)
    {
        prvTgfRxEntriesArr[i].isEmpty = GT_TRUE;
    }

    prvTgfIsInitialized = GT_TRUE;

}


/******************************************************************************\
 *                              API implementation                            *
\******************************************************************************/

/**
* @internal prvTgfTrafficTableRxStartCapture function
* @endinternal
*
* @brief   Start/Stop collecting the received packets without delay.
*
* @param[in] enable                   - GT_TRUE - start capture
*                                      GT_FALSE - stop capture
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvTgfTrafficTableRxStartCapture
(
    IN GT_BOOL          enable
)
{
    /* Initialize Rx library */
    TGF_RX_LIB_INIT_MAC;

    TGF_RX_SEM_TAKE_MAC;
    prvTgfDoCapture = enable;
    TGF_RX_SEM_GIVE_MAC;
    if (prvTgfReRunVariable == TGF_RE_RUN_STATUS_CAPABLE_E)
    {
        /* turn on flag to state the test uses capture */
        prvTgfReRunVariable = TGF_RE_RUN_STATUS_DO_RE_RUN_E;
    }

    return GT_OK;
}
/**
* @internal tgfTrafficTableRxStartCapture function
* @endinternal
*
* @brief   Start/Stop collecting the received packets.
*         Use delay for capture disable.
* @param[in] enable                   - GT_TRUE - start capture
*                                      GT_FALSE - stop capture
*
* @retval GT_OK                    - on success
*/
GT_STATUS tgfTrafficTableRxStartCapture
(
    IN GT_BOOL          enable
)
{
#ifndef  EMBEDDED
    if (enable == GT_FALSE)
    {
        GT_U32    loops;

        /* let the packets be captured */
        loops = prvTgfCaptureTimeLoop;
        while(loops--)/* allow debugging by set this value = 1000 */
        {
            cpssOsTimerWkAfter(TGF_DEFAULT_CAPTURE_SLEEP_TIME_CNS);
        }
    }
#endif

    return prvTgfTrafficTableRxStartCapture(enable);
}

/**
* @internal tgfTrafficTableRxModeSet function
* @endinternal
*
* @brief   Set received packets collection mode and parameters
*
* @param[in] rxMode                   - collection mode
* @param[in] buferSize                - buffer size
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_CPU_MEM        - memory allocation Failure
*/
GT_STATUS tgfTrafficTableRxModeSet
(
    IN TGF_RX_MODE_ENT  rxMode,
    IN GT_U32           buferSize
)
{
    GT_STATUS   rc = GT_OK;
    /* Initialize Rx library */
    TGF_RX_LIB_INIT_MAC;

    /* Should NOT be inside sem wait (takes the same semaphore)*/
    rc = tgfTrafficTableRxPcktTblClear();
    if (rc != GT_OK)
    {
        return rc;
    }

    TGF_RX_SEM_TAKE_MAC;

    prvTgfRxTableMode     = rxMode;
    prvTgfMaxRxBufferSize = buferSize;
    TGF_RX_SEM_GIVE_MAC;

    return rc;
}

/**
* @internal prvTgfTrafficTableRxPcktReceive function
* @endinternal
*
* @brief   Receive packet callback function. The caller wants to register its
*         packet info with the manager array.
* @param[in] devNum                   - device number in which the packet was received
* @param[in] queue                    - Rx  in which the packet was received
*                                      specificDevFormatPtr - (pointer to) the specific device Rx info format
* @param[in] freeRxInfoFunc           - callback to free the specific rx info format
* @param[in] copyRxInfoFunc           - callback to copy the specific rx info format
* @param[in] numOfBuff                - num of used buffs in packetBuffs
* @param[in] packetBuffs[]            - the received packet buffers list
* @param[in] buffLen[]                - list of buffer lengths for packetBuffs
*
* @retval GT_OK                    - on success
* @retval GT_FULL                  - when buffer is full
* @retval GT_BAD_STATE             - the engine not ready
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_OUT_OF_CPU_MEM        - on CPU memory allocation failure
*
* @note This function give services to all the rx packets info formats.
*
*/
static GT_STATUS prvTgfTrafficTableRxPcktReceive
(
    IN GT_U8                 devNum,
    IN GT_U8                 queue,
    IN GT_VOID               *specificDeviceFormatPtr,
    IN TGF_FREE_RX_INFO_FUNC freeRxInfoFunc,
    IN TGF_COPY_RX_INFO_FUNC copyRxInfoFunc,
    IN GT_U32                numOfBuff,
    IN GT_U8                 *packetBuffs[],
    IN GT_U32                buffLen[]
)
{
    GT_STATUS   rc            = GT_OK;
    GT_U32      tmpBuffSize   = 0;
    GT_U32      toCopyBufSize = 0;
    GT_U32      rxPcktBufSize = 0;
    GT_U32      i             = 0;

    CPSS_NULL_PTR_CHECK_MAC(copyRxInfoFunc);
    CPSS_NULL_PTR_CHECK_MAC(packetBuffs);
    CPSS_NULL_PTR_CHECK_MAC(buffLen);
    if(PRV_CPSS_PP_MAC(devNum) && CPSS_IS_PX_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        /* no rxParamsPtr for PX device */
    }
    else
    {
        CPSS_NULL_PTR_CHECK_MAC(specificDeviceFormatPtr);
    }

    if(!prvTgfDoCapture || !prvTgfIsInitialized)
    {
        static GT_U32   nonExpectedNum = 0;
#ifdef CHX_FAMILY
        CPSS_DXCH_NET_RX_PARAMS_STC* dxChPcktParamsPtr = (CPSS_DXCH_NET_RX_PARAMS_STC*) specificDeviceFormatPtr;
        char * dsaTypeArr[] = {"TO_CPU", "FROM_CPU", "TO_ANALYZER", "FORWARD", "WRONG"};
        GT_U32 dsaType;
        GT_U32 port;
        GT_U32 cpuCode;
        GT_U32 bytes;
        char * dsaTypePtr;

        dsaType = dxChPcktParamsPtr->dsaParam.dsaType;
        dsaTypePtr = (dsaType > 3) ? dsaTypeArr[4]:dsaTypeArr[dsaType];
        bytes = 0;
        cpuCode = 0;

        if(CPSS_DXCH_NET_DSA_CMD_TO_CPU_E == dxChPcktParamsPtr->dsaParam.dsaType)
        {
            port = dxChPcktParamsPtr->dsaParam.dsaInfo.toCpu.interface.portNum;
            bytes = dxChPcktParamsPtr->dsaParam.dsaInfo.toCpu.originByteCount;
            cpuCode = dxChPcktParamsPtr->dsaParam.dsaInfo.toCpu.cpuCode;
        }
        else if(dxChPcktParamsPtr->dsaParam.dsaType == CPSS_DXCH_NET_DSA_CMD_FORWARD_E)
        {
            port = dxChPcktParamsPtr->dsaParam.dsaInfo.forward.source.portNum;
        }
        else
        {
            /* dummy one */
            port = 1000000;
        }

        PRV_TGF_LOG5_MAC("[TGF]: prvTgfTrafficTableRxPcktReceive not expected packet number [%d] port [%d] DSA Type [%s] bytes [%d] CPU Code [%d]\n", nonExpectedNum, port, dsaTypePtr, bytes, cpuCode);
#else /* CHX_FAMILY */

        PRV_TGF_LOG1_MAC("[TGF]: prvTgfTrafficTableRxPcktReceive packet received when not expected number [%d] \n", nonExpectedNum);
#endif /* CHX_FAMILY */
        nonExpectedNum++;

        return GT_BAD_STATE;
    }

    TGF_RX_SEM_TAKE_MAC;

    if((prvTgfRxTableMode == TGF_RX_ONCE_E && prvTgfIndexToInsertPacket < prvTgfMaxNumOfRxEntries - 1)
        || (prvTgfRxTableMode == TGF_RX_CYCLIC_E))
    {
        if(prvTgfRxEntriesArr[prvTgfIndexToInsertPacket].isEmpty != GT_TRUE)
        {
            prvTgfTrafficTableRxPacketInfoFree(prvTgfIndexToInsertPacket);
        }

        /* get the packet actual length */
        rxPcktBufSize = 0;
        for(i = 0; i < numOfBuff; i++)
        {
            rxPcktBufSize += buffLen[i];
        }

        /* set the number of buffer bytes to copy */
        if (rxPcktBufSize > prvTgfMaxRxBufferSize)
        {
            /* packet length is bigger than destination buffer size. */
            toCopyBufSize = prvTgfMaxRxBufferSize;
        }
        else
        {
            toCopyBufSize = rxPcktBufSize;
        }

        /* allocate memory for the packet data */
        prvTgfRxEntriesArr[prvTgfIndexToInsertPacket].packetBufferPtr = cpssOsMalloc(toCopyBufSize + 1);

        if (NULL == prvTgfRxEntriesArr[prvTgfIndexToInsertPacket].packetBufferPtr)
        {
            TGF_RX_SEM_GIVE_MAC;
            return GT_OUT_OF_CPU_MEM;
        }

        prvTgfRxEntriesArr[prvTgfIndexToInsertPacket].isEmpty = GT_FALSE;
        prvTgfRxEntriesArr[prvTgfIndexToInsertPacket].devNum = devNum;
        prvTgfRxEntriesArr[prvTgfIndexToInsertPacket].queue = queue;
        if(specificDeviceFormatPtr)
        {
            prvTgfRxEntriesArr[prvTgfIndexToInsertPacket].specificDeviceFormatPtr =
                cpssOsMalloc(prvTgfGlobalRxParamSize);

            copyRxInfoFunc(specificDeviceFormatPtr,
                           prvTgfRxEntriesArr[prvTgfIndexToInsertPacket].specificDeviceFormatPtr);
        }

        prvTgfRxEntriesArr[prvTgfIndexToInsertPacket].freeRxInfoFunc = freeRxInfoFunc;
        prvTgfRxEntriesArr[prvTgfIndexToInsertPacket].copyRxInfoFunc = copyRxInfoFunc;
        prvTgfRxEntriesArr[prvTgfIndexToInsertPacket].packetOriginalLen = rxPcktBufSize;
        prvTgfRxEntriesArr[prvTgfIndexToInsertPacket].bufferLen = toCopyBufSize + 1;

        cpssOsMemSet(prvTgfRxEntriesArr[prvTgfIndexToInsertPacket].packetBufferPtr, 0, toCopyBufSize + 1);

        tmpBuffSize = 0;

        /* copy packet data to allocated buffer */
        for(i = 0; i < numOfBuff; i++)
        {
            if(tmpBuffSize + buffLen[i] > toCopyBufSize)
            {
                /* packet length is bigger than destination buffer size */
                cpssOsMemCpy(prvTgfRxEntriesArr[prvTgfIndexToInsertPacket].packetBufferPtr + tmpBuffSize,
                             packetBuffs[i], toCopyBufSize - tmpBuffSize);
                break;
            }

            /* copy the packet buffer to destination buffer */
            cpssOsMemCpy(prvTgfRxEntriesArr[prvTgfIndexToInsertPacket].packetBufferPtr + tmpBuffSize,
                         packetBuffs[i], buffLen[i]);
            tmpBuffSize += buffLen[i];
        }

        if((prvTgfIndexToInsertPacket == prvTgfMaxNumOfRxEntries - 1))
            prvTgfIndexToInsertPacket = 0;
        else
            prvTgfIndexToInsertPacket++;

        rc = GT_OK;
    }
    else
    {
        rc = GT_FULL;
    }

    TGF_RX_SEM_GIVE_MAC;

    return rc;
}

/**
* @internal tgfTrafficTableRxPcktTblClear function
* @endinternal
*
* @brief   Clear table
*
* @retval GT_OK                    - on success
*/
GT_STATUS tgfTrafficTableRxPcktTblClear
(
    GT_VOID
)
{
    GT_U32 i = 0;

    /* Initialize Rx library */
    TGF_RX_LIB_INIT_MAC;

    TGF_RX_SEM_TAKE_MAC;

    for(i = 0; i < prvTgfMaxNumOfRxEntries; i++)
    {
        if(prvTgfRxEntriesArr[i].isEmpty == GT_FALSE)
        {
            prvTgfTrafficTableRxPacketInfoFree(i);
            prvTgfRxEntriesArr[i].isEmpty = GT_TRUE;
        }
    }

    prvTgfIndexToInsertPacket = 0;

    TGF_RX_SEM_GIVE_MAC;

    return GT_OK;
}


/**
* @internal tgfTrafficTableRxPcktGet function
* @endinternal
*
* @brief   Get entry from rxNetworkIf table
*
* @param[in,out] currentIndex             - the current entry's index
* @param[in,out] packetBufLenPtr          - the length of the user space for the packet
*
* @param[out] packetBufPtr             - packet's buffer (pre allocated by the user)
* @param[in,out] packetBufLenPtr          - the length of the copied packet to gtBuf
* @param[out] packetLenPtr             - the Rx packet original length
* @param[out] devNumPtr                - packet's device number
* @param[out] queuePtr                 - The Rx queue in which the packet was received.
* @param[out] specificDeviceFormatPtr  - (pointer to) the specific device Rx info format.
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - index is out of range
* @retval GT_NOT_FOUND             - entry in index is entry not valid
*/
GT_STATUS tgfTrafficTableRxPcktGet
(
    INOUT GT_U32        currentIndex,
    OUT   GT_U8         *packetBufPtr,
    INOUT GT_U32        *packetBufLenPtr,
    OUT   GT_U32        *packetLenPtr,
    OUT   GT_U8         *devNumPtr,
    OUT   GT_U8         *queuePtr,
    OUT   GT_VOID       *specificDeviceFormatPtr
)
{
    /* Initialize Rx library */
    TGF_RX_LIB_INIT_MAC;

    TGF_RX_SEM_TAKE_MAC;

    if(currentIndex >= prvTgfMaxNumOfRxEntries)
    {
        TGF_RX_SEM_GIVE_MAC;
        return GT_OUT_OF_RANGE;
    }

    if(prvTgfRxEntriesArr[currentIndex].isEmpty == GT_TRUE)
    {
        TGF_RX_SEM_GIVE_MAC;
        return GT_NOT_FOUND;
    }

    /* if packet is smaller than gtBuf update the gtBufLength */
    if (prvTgfRxEntriesArr[currentIndex].bufferLen < *packetBufLenPtr)
    {
        *packetBufLenPtr = prvTgfRxEntriesArr[currentIndex].bufferLen;
    }

    if (*packetBufLenPtr > prvTgfMaxRxBufferSize)
    {
        /* the packet is bigger than the table packet size */
        *packetBufLenPtr = prvTgfMaxRxBufferSize;
    }

    /* set the real packet size */
    *packetLenPtr = prvTgfRxEntriesArr[currentIndex].packetOriginalLen;

    /* copy the packet to user buffer */
    cpssOsMemCpy(packetBufPtr,
                 prvTgfRxEntriesArr[currentIndex].packetBufferPtr,
                 *packetBufLenPtr);

    if(specificDeviceFormatPtr && prvTgfRxEntriesArr[currentIndex].specificDeviceFormatPtr)
    {
        prvTgfRxEntriesArr[currentIndex].copyRxInfoFunc(
            prvTgfRxEntriesArr[currentIndex].specificDeviceFormatPtr,
            specificDeviceFormatPtr);
    }

    *queuePtr  = prvTgfRxEntriesArr[currentIndex].queue;
    *devNumPtr = prvTgfRxEntriesArr[currentIndex].devNum;

    TGF_RX_SEM_GIVE_MAC;

    return GT_OK;
}

/**
* @internal tgfTrafficTableRxPcktIsCaptureReady function
* @endinternal
*
* @brief   Check if the engine is ready to capture rx packets
*
* @retval GT_TRUE                  - the engine is ready to capture rx packets
* @retval GT_FALSE                 - the engine is NOT ready to capture rx packets
*/
GT_BOOL tgfTrafficTableRxPcktIsCaptureReady
(
    GT_VOID
)
{
    return (prvTgfDoCapture == GT_TRUE &&
            prvTgfIsInitialized == GT_TRUE) ? GT_TRUE : GT_FALSE;
}

/**
* @internal prvTgfTrafficTableRxInfoFree function
* @endinternal
*
* @brief   Function to free the specific rx info format.
*
* @param[in] specificDeviceFormatPtr  - (pointer to) the specific device Rx info format.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS prvTgfTrafficTableRxInfoFree
(
    IN GT_VOID          *specificDeviceFormatPtr
)
{
    CPSS_NULL_PTR_CHECK_MAC(specificDeviceFormatPtr);

    cpssOsFree(specificDeviceFormatPtr);

    return GT_OK;
}

/**
* @internal prvTgfTrafficTableRxInfoCopy function
* @endinternal
*
* @brief   Function to COPY the specific rx info format.
*
* @param[in] srcSpecificDeviceFormatPtr - (pointer to) the SOURCE specific device Rx
*                                      info format.
*
* @param[out] dstSpecificDeviceFormatPtr - (pointer to) the DESTINATION specific device Rx
*                                      info format.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS prvTgfTrafficTableRxInfoCopy
(
    IN  const GT_VOID   *srcSpecificDeviceFormatPtr,
    OUT       GT_VOID   *dstSpecificDeviceFormatPtr
)
{
    CPSS_NULL_PTR_CHECK_MAC(srcSpecificDeviceFormatPtr);
    CPSS_NULL_PTR_CHECK_MAC(dstSpecificDeviceFormatPtr);

    cpssOsMemCpy(dstSpecificDeviceFormatPtr,
                 srcSpecificDeviceFormatPtr,
                 prvTgfGlobalRxParamSize);

    return GT_OK;
}

/**
* @internal tgfTrafficTableRxDoCountOnlySet function
* @endinternal
*
* @brief   Function sets Counting Only for incoming Rx packet in the CPU
*
* @param[in] doCountOnly              - GT_TRUE - only count packets
*                                      GT_FALSE - make regular capture treatment.
*                                       previouse state of Do Count Only
*/
GT_BOOL tgfTrafficTableRxDoCountOnlySet
(
    IN GT_BOOL         doCountOnly
)
{
    GT_BOOL prevState = prvTgfRxPacketDoCountOnly;
    prvTgfRxPacketDoCountOnly = doCountOnly;
    return prevState;
}

/**
* @internal tgfTrafficTableRxToTxSet function
* @endinternal
*
* @brief   Function sets Rx To Tx mode of Rx Packet callback
*
* @param[in] doRxToTx                 - GT_TRUE - each Rx packet goes to Tx from CPU
*                                      GT_FALSE - make regular capture treatment.
*                                       previouse state of Rx To Tx mode
*/
GT_BOOL tgfTrafficTableRxToTxSet
(
    IN GT_BOOL         doRxToTx
)
{
    GT_BOOL prevState = prvTgfRxPacketDoRxToTx;
    prvTgfRxPacketDoRxToTx = doRxToTx;
    return prevState;
}

/**
* @internal tgfTrafficTableRxCountGet function
* @endinternal
*
* @brief   Function returns counter of RX packets
*/
GT_U32 tgfTrafficTableRxCountGet
(
    GT_VOID
)
{
    return prvTgfRxPacketCount;
}

/**
* @internal tgfTrafficTableRxPcktReceiveCb function
* @endinternal
*
* @brief   Function called to handle incoming Rx packet in the CPU
*
* @param[in] devNum                   - Device number.
* @param[in] queueIdx                 - The queue from which this packet was received.
* @param[in] numOfBuff                - Num of used buffs in packetBuffs.
* @param[in] packetBuffs[]            - The received packet buffers list.
* @param[in] buffLen[]                - List of buffer lengths for packetBuffs.
* @param[in] rxParamsPtr              - (pointer to)information parameters of received packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS tgfTrafficTableRxPcktReceiveCb
(
    IN GT_UINTPTR                cookie,
    IN RX_EV_HANDLER_EV_TYPE_ENT evType,
    IN GT_U8            devNum,
    IN GT_U8            queueIdx,
    IN GT_U32           numOfBuff,
    IN GT_U8            *packetBuffs[],
    IN GT_U32           buffLen[],
    IN GT_VOID          *rxParamsPtr
)
{
    GT_STATUS rc             = GT_OK;

    CPSS_NULL_PTR_CHECK_MAC(packetBuffs);
    CPSS_NULL_PTR_CHECK_MAC(buffLen);

    cookie = cookie;
    evType = evType;

    prvTgfRxPacketCount++;

    if (prvTgfRxPacketDoCountOnly)
    {
        return rc;
    }

    if (prvTgfRxPacketDoRxToTx)
    {
        return prvTgfTrafficGeneratorRxToTxSend(devNum, numOfBuff,
                                                packetBuffs, buffLen);
    }

    rc = prvTgfTrafficTableRxPcktReceive(devNum, queueIdx, rxParamsPtr,
                                         &prvTgfTrafficTableRxInfoFree,
                                         &prvTgfTrafficTableRxInfoCopy,
                                         numOfBuff, packetBuffs, buffLen);
    return rc;
}

/**
* @internal tgfTrafficTableNetRxPacketCbRegister function
* @endinternal
*
* @brief   Register a CB function to be called on every RX packet to CPU
*
* @param[in] rxPktReceiveCbFun        - CB function to be called on every RX packet to CPU
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_STATE             - om illegal state
*
* @note Function is implemented in the AppDemo application
*
*/
GT_STATUS tgfTrafficTableNetRxPacketCbRegister
(
    IN  RX_EV_PACKET_RECEIVE_CB_FUN  rxPktReceiveCbFun
)
{
    GT_STATUS   rc = GT_FAIL;

    /* check if CB already registered */
    if (GT_FALSE == prvTgfRxCbRegistered)
    {
        rc = rxEventHandlerAddCallback(
                RX_EV_HANDLER_DEVNUM_ALL,
                RX_EV_HANDLER_QUEUE_ALL,
                RX_EV_HANDLER_TYPE_ANY_E,
                rxPktReceiveCbFun,
                0);

        if (GT_ALREADY_EXIST == rc)
            rc = GT_OK;
        if (GT_OK != rc)
        {
            return rc;
        }

        prvTgfRxCbRegistered = GT_TRUE;
    }
    else
    {
        return GT_ALREADY_EXIST;
    }

    return rc;
}


