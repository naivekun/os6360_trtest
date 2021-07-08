/*******************************************************************************
*                Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
* cmdNetReceive.c
*
* DESCRIPTION:
*       galtis agent interface for packet capture using networkIf
*
* DEPENDENCIES:
*       cmdNetReceive.h
*
* FILE REVISION NUMBER:
*       $Revision: 19 $
*******************************************************************************/

#include <cmdShell/common/cmdCommon.h>
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <galtisAgent/wrapUtil/cmdCpssNetReceive.h>

#define CMD_RX_DEFULT_NUM_OF_ENTRIES_CNS 10

/******************************** Locals *************************************/

static CMD_RX_ENTRY *rxEntriesArr;
static GALTIS_RX_MODE_ENT rxTableMode = GALTIS_RX_CYCLIC_E;
static GT_U32  maxRxBufferSize = GALTIS_RX_BUFFER_MAX_SIZE_CNS; /*1536*/
static GT_U32  maxNumOfRxEntries = CMD_RX_DEFULT_NUM_OF_ENTRIES_CNS;
static GT_U32  indexToInsertPacket = 0;
static GT_U32  doCapture = GT_FALSE;
static GT_BOOL wasInitDone = GT_FALSE;

static GT_MUTEX  cmdCpssRxMtx = 0;
#define CMD_RX_MTX_CREATE_MAC   if(cmdCpssRxMtx==0) cmdOsMutexCreate("cmdCpssRxMtx", &cmdCpssRxMtx)
#define CMD_RX_MTX_LOCK_MAC     cmdOsMutexLock(cmdCpssRxMtx)
#define CMD_RX_MTX_UNLOCK_MAC     cmdOsMutexUnlock(cmdCpssRxMtx)
#define CMD_RX_MTX_DELETE_MAC   if(cmdCpssRxMtx!=0) cmdOsMutexDelete(cmdCpssRxMtx)

#ifdef SHARED_MEMORY
GT_U32   *enhUtUseCaptureToCpuPtr = NULL;
#else
static GT_U32  enhUtUseCaptureToCpu = 0;
GT_U32   *enhUtUseCaptureToCpuPtr = &enhUtUseCaptureToCpu;
#endif

static GT_U32   forceShowAllPackets = 0;

#if (defined CHX_FAMILY)
static GT_U32  checkIsEnhUtUseCaptureToCpu(
    IN GT_VOID  *specificDeviceFormatPtr
);
#endif /*(defined CHX_FAMILY)*/

/**
* @internal freeRxPacketInfo function
* @endinternal
*
* @brief   free the specific rx packet info, by calling to the registered callback
*
* @param[in] index                    -  to the rxEntriesArr array
*                                       None
*/
static void freeRxPacketInfo
(
    IN GT_U32   index
)
{
    if(rxEntriesArr[index].packetBufferPtr)
    {
        cmdOsFree(rxEntriesArr[index].packetBufferPtr);
        rxEntriesArr[index].packetBufferPtr = NULL;
    }


    if(rxEntriesArr[index].specificDeviceFormatPtr == NULL)
    {
        return;
    }

    if(rxEntriesArr[index].freeRxInfoFunc)
    {
        /* free by call back*/
        rxEntriesArr[index].freeRxInfoFunc(rxEntriesArr[index].specificDeviceFormatPtr);
    }
    else
    {
        /* free with no specific extra needs */
        cmdOsFree(rxEntriesArr[index].specificDeviceFormatPtr);
    }

    /* set not used pointer */
    rxEntriesArr[index].specificDeviceFormatPtr = NULL;
}

/**
* @internal cmdCpssRxInitLibSystemReset function
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
static GT_VOID cmdCpssRxInitLibSystemReset
(
    GT_VOID
)
{
    GT_U32  ii;

    CMD_RX_MTX_LOCK_MAC;

    if (rxEntriesArr != NULL)
    {
        /* free the specific packet's device format */
        for(ii = 0 ; ii < maxNumOfRxEntries ; ii++)
        {
            if(rxEntriesArr[ii].isEmpty == GT_FALSE)
            {
                freeRxPacketInfo(ii);
            }
        }

        cmdOsFree(rxEntriesArr);
    }

    rxEntriesArr = NULL;

    rxTableMode = GALTIS_RX_CYCLIC_E;
    maxRxBufferSize = GALTIS_RX_BUFFER_MAX_SIZE_CNS; /*1536*/
    maxNumOfRxEntries = CMD_RX_DEFULT_NUM_OF_ENTRIES_CNS;
    indexToInsertPacket = 0;
    doCapture = GT_FALSE;
    wasInitDone = GT_FALSE;

    if (enhUtUseCaptureToCpuPtr != NULL)
        *enhUtUseCaptureToCpuPtr = 0;
    forceShowAllPackets = 0;

    CMD_RX_MTX_UNLOCK_MAC;


}

/**
* @internal cmdCpssRxInitLib function
* @endinternal
*
* @brief   initialize the rx library
*
* @note GalTis:
*       None
*
*/
static GT_VOID cmdCpssRxInitLib(void)
{
    int ii;

    if(wasInitDone == GT_TRUE)
        return;

    /* create semaphore */
    CMD_RX_MTX_CREATE_MAC;

    rxEntriesArr = cmdOsMalloc(CMD_RX_DEFULT_NUM_OF_ENTRIES_CNS * sizeof(CMD_RX_ENTRY));
    if (NULL == rxEntriesArr)
    {
        CMD_RX_MTX_DELETE_MAC;
        return;
    }
    for (ii = 0; ii < CMD_RX_DEFULT_NUM_OF_ENTRIES_CNS; ii++) 
    {
        rxEntriesArr[ii].isEmpty = GT_TRUE;
    }

    /* register function to reset DB so after system reset we can send traffic again */
    wrCpssRegisterResetCb(cmdCpssRxInitLibSystemReset);

    wasInitDone = GT_TRUE;
}


/**
* @internal cmdCpssRxStartCapture function
* @endinternal
*
* @brief   Start collecting the recived packets
*
* @retval GT_OK                    - on success
*
* @note GalTis:
*       Command - cmdCpssRxStartCapture
*
*/
GT_STATUS cmdCpssRxStartCapture(void)
{
    if(wasInitDone == GT_FALSE)
        cmdCpssRxInitLib();

    if(rxEntriesArr == NULL)
    {
        return GT_OUT_OF_CPU_MEM;
    }

    CMD_RX_MTX_LOCK_MAC;
    doCapture = GT_TRUE;
    CMD_RX_MTX_UNLOCK_MAC;

    return GT_OK;
}


/**
* @internal cmdCpssRxStopCapture function
* @endinternal
*
* @brief   Stop collecting the recived packets
*
* @retval GT_OK                    - on success
*
* @note GalTis:
*       Command - cmdCpssRxStopCapture
*
*/
GT_STATUS cmdCpssRxStopCapture()
{
    CMD_RX_MTX_LOCK_MAC;
    doCapture = GT_FALSE;
    CMD_RX_MTX_UNLOCK_MAC;

    return GT_OK;
}

/**
* @internal cmdCpssRxSetMode function
* @endinternal
*
* @brief   set recived packets collection mode and parameters
*
* @param[in] rxMode                   - collection mode
* @param[in] buferSize                - buffer size
* @param[in] numOfEnteries            - number of entries
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_CPU_MEM        - memory allocation Failure
*
* @note GalTis:
*       Command - cmdCpssRxSetMode
*
*/
GT_STATUS cmdCpssRxSetMode
(
    IN GALTIS_RX_MODE_ENT   rxMode,
    IN GT_U32       buferSize,
    IN GT_U32       numOfEnteries
)
{
    GT_U32 ii;

    if (buferSize == 0 || numOfEnteries == 0)
    {
        return GT_BAD_PARAM;
    }

    if(wasInitDone == GT_FALSE)
    {
        cmdCpssRxInitLib();
        if(rxEntriesArr == NULL)
        {
            return GT_OUT_OF_CPU_MEM;
        }
    }

    CMD_RX_MTX_LOCK_MAC;

    rxTableMode = rxMode;
    maxRxBufferSize = buferSize;

    if (rxEntriesArr != NULL)
    {
        /* free the specific packet's device format */
        for(ii = 0 ; ii < maxNumOfRxEntries ; ii++)
        {
            if(rxEntriesArr[ii].isEmpty == GT_FALSE)
            {
                freeRxPacketInfo(ii);
            }
        }

        cmdOsFree(rxEntriesArr);
    }

    rxEntriesArr = cmdOsMalloc(numOfEnteries * sizeof(CMD_RX_ENTRY));

    if(rxEntriesArr == NULL)
    {
        CMD_RX_MTX_UNLOCK_MAC;
        return GT_OUT_OF_CPU_MEM;
    }

    for(ii = 0; ii < numOfEnteries; ii++)
    {
        rxEntriesArr[ii].isEmpty = GT_TRUE;
    }

    maxNumOfRxEntries = numOfEnteries;

    CMD_RX_MTX_UNLOCK_MAC;

    return GT_OK;
}

/**
* @internal cmdCpssRxGetMode function
* @endinternal
*
* @brief   Get recived packets collection mode and parameters
*
* @retval GT_OK                    - on success
*
* @note GalTis:
*       Command - cmdCpssRxGetMode
*
*/
GT_STATUS cmdCpssRxGetMode
(
    OUT GALTIS_RX_MODE_ENT  *modePtr,
    OUT GT_U32      *buffSizePtr,
    OUT GT_U32      *numEntriesPtr
)
{
    CMD_RX_MTX_LOCK_MAC;
    *modePtr = rxTableMode;
    *buffSizePtr = maxRxBufferSize;
    *numEntriesPtr = maxNumOfRxEntries;
    CMD_RX_MTX_UNLOCK_MAC;

    return GT_OK;
}

/**
* @internal cmdCpssRxPktReceive function
* @endinternal
*
* @brief   Receive packet callback function . the caller wants to register its
*         packet info with the manager array.
*         This function give services to all the rx packets info formats.
* @param[in] devNum                   - The device number in which the packet was received.
* @param[in] queue                    - The Rx  in which the packet was received.
* @param[in] specificDeviceFormatPtr  - (pointer to) the specific device Rx info format.
*                                      format of DXCH
*                                      NOTE : this pointer is allocated by the specific
*                                      device "C" file.
* @param[in] freeRxInfoFunc           - callback function to free the specific rx info format,
* @param[in] copyRxInfoFunc           - callback function to copy the specific rx info format,
* @param[in] numOfBuff                - Num of used buffs in packetBuffs
* @param[in] packetBuffs[]            - The received packet buffers list
* @param[in] buffLen[]                - List of buffer lengths for packetBuffs
*
* @retval GT_OK                    - on success
* @retval GT_FULL                  - when buffer is full
* @retval GT_BAD_STATE             - the galtis-cmd not ready/set to "save" the packet with info
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_OUT_OF_CPU_MEM        - on CPU memory allocation failure
*
* @note GalTis:
*       None
*
*/
GT_STATUS cmdCpssRxPktReceive
(
    IN GT_U8           devNum,
    IN GT_U8           queue,
    IN void*           specificDeviceFormatPtr,
    IN FREE_RX_INFO_FUNC freeRxInfoFunc,
    IN COPY_RX_INFO_FUNC copyRxInfoFunc,
    IN GT_U32          numOfBuff,
    IN GT_U8           *packetBuffs[],
    IN GT_U32          buffLen[]
)
{
    GT_STATUS   rc;
    GT_U32 tmpBuffSize;
    GT_U32 toCopyBufSize;
    GT_U32 rxPcktBufSize;
    GT_U32 ii;

    if(rxEntriesArr == NULL)
    {
        return GT_OUT_OF_CPU_MEM;
    }

    if(doCapture != GT_TRUE)
        return GT_BAD_STATE;

    if(wasInitDone == GT_FALSE)
        return GT_BAD_STATE;

    if(packetBuffs == NULL ||
       buffLen == NULL)
    {
        return GT_BAD_PARAM;
    }

    CMD_RX_MTX_LOCK_MAC;

    if((rxTableMode == GALTIS_RX_ONCE_E && indexToInsertPacket < maxNumOfRxEntries - 1)
        || (rxTableMode == GALTIS_RX_CYCLIC_E))
    {
        if(rxEntriesArr[indexToInsertPacket].isEmpty != GT_TRUE)
        {
            freeRxPacketInfo(indexToInsertPacket);
        }

        /* get the packet actual length */
        rxPcktBufSize = 0;
        for(ii = 0; ii < numOfBuff; ii++)
        {
            rxPcktBufSize += buffLen[ii];
        }

        /* set the number of buffer bytes to copy */
        if (rxPcktBufSize > maxRxBufferSize)
        {
            /* packet length is bigger than destination buffer size. */
            toCopyBufSize = maxRxBufferSize;
        }
        else
        {
            toCopyBufSize = rxPcktBufSize;
        }

        /* allocate memory for the packet data */
        rxEntriesArr[indexToInsertPacket].packetBufferPtr = cmdOsMalloc(toCopyBufSize);

        if (NULL == rxEntriesArr[indexToInsertPacket].packetBufferPtr)
        {
            CMD_RX_MTX_UNLOCK_MAC;
            return GT_OUT_OF_CPU_MEM;
        }

        rxEntriesArr[indexToInsertPacket].isEmpty = GT_FALSE;
        rxEntriesArr[indexToInsertPacket].devNum = devNum;
        rxEntriesArr[indexToInsertPacket].queue = queue;
#if (defined CHX_FAMILY)
        rxEntriesArr[indexToInsertPacket].specificDeviceFormatPtr = specificDeviceFormatPtr;
        rxEntriesArr[indexToInsertPacket].freeRxInfoFunc = freeRxInfoFunc;
        rxEntriesArr[indexToInsertPacket].copyRxInfoFunc = copyRxInfoFunc;
#endif
        rxEntriesArr[indexToInsertPacket].packetOriginalLen = rxPcktBufSize;
        rxEntriesArr[indexToInsertPacket].bufferLen = toCopyBufSize;

#if (defined CHX_FAMILY)
        if(checkIsEnhUtUseCaptureToCpu(specificDeviceFormatPtr))
        {
            rxEntriesArr[indexToInsertPacket].entryType = GT_RX_ENTRY_TYPE_CAPTURED_BY_ENH_UT_E;
        }
        else
#endif
        {
            rxEntriesArr[indexToInsertPacket].entryType = GT_RX_ENTRY_TYPE_GENERIC_E;
        }

        cmdOsMemSet(rxEntriesArr[indexToInsertPacket].packetBufferPtr,0,toCopyBufSize);

        tmpBuffSize = 0;
        /* copy packet data to allocated buffer */
        for(ii = 0; ii < numOfBuff; ii++)
        {
            if(tmpBuffSize + buffLen[ii] > toCopyBufSize)
            {
                /* packet length is bigger than destination buffer size */
                cmdOsMemCpy(rxEntriesArr[indexToInsertPacket].packetBufferPtr + tmpBuffSize,
                         packetBuffs[ii], toCopyBufSize - tmpBuffSize);
                break;
            }
            /* copy the packet buffer to destination buffer */
            cmdOsMemCpy(rxEntriesArr[indexToInsertPacket].packetBufferPtr + tmpBuffSize,
                     packetBuffs[ii], buffLen[ii]);
            tmpBuffSize += buffLen[ii];
        }

        if((indexToInsertPacket == maxNumOfRxEntries -1))
            indexToInsertPacket = 0;
        else
            indexToInsertPacket++;

        rc = GT_OK;
    }
    else
    {
        rc = GT_FULL;
    }

    CMD_RX_MTX_UNLOCK_MAC;

    return rc;
}

/**
* @internal cmdCpssRxPktClearTbl function
* @endinternal
*
* @brief   clear cmdCpssRxPktClearTbl table
*
* @retval GT_OK                    - on success
*
* @note GalTis:
*       Table - rxNetworkIf
*
*/
GT_STATUS cmdCpssRxPktClearTbl(void)
{
    GT_U32 ii;

    if(wasInitDone == GT_FALSE)
        cmdCpssRxInitLib();

    if(rxEntriesArr == NULL)
    {
        return GT_OUT_OF_CPU_MEM;
    }

    CMD_RX_MTX_LOCK_MAC;

    for(ii = 0; ii < maxNumOfRxEntries; ii++)
    {
        if(rxEntriesArr[ii].isEmpty == GT_FALSE)
        {
            freeRxPacketInfo(ii);
            rxEntriesArr[ii].isEmpty = GT_TRUE;
        }
    }

    indexToInsertPacket = 0;

    CMD_RX_MTX_UNLOCK_MAC;

    return GT_OK;
}

/**
* @internal cmdCpssNetForceShowAllPackets function
* @endinternal
*
* @brief   function to indicate the 'cmd Cpss Net' need to show all packet even those which are internal
*
* @param[in] force                    - GT_TRUE  - indicate the 'cmd Cpss Net' show all packet , even the internal packets.
*                                      GT_FALSE - indicate the 'cmd Cpss Net' NOT show all packet , so internal packets are not shown.
*                                       GT_TRUE
*
* @note NOTE: this is debug function to allow check that mechanism is OK.
*
*/
GT_STATUS  cmdCpssNetForceShowAllPackets(
    IN GT_U32   force
)
{
    forceShowAllPackets = force;
    return GT_OK;
}

/**
* @internal cmdCpssNetEnhUtUseCaptureToCpu function
* @endinternal
*
* @brief   function to indicate the 'cmd Cpss Net' that the enh-UT are using 'capture to CPU'
*         using specific CPU codes:
*         CPSS_NET_LAST_USER_DEFINED_E , CPSS_NET_INGRESS_MIRRORED_TO_ANLYZER_E ,
*         CPSS_NET_EGRESS_MIRRORED_TO_ANLYZER_E
* @param[in] useCapture               - GT_TRUE  - indicate the 'cmd Cpss Net' that the enh-UT are     using 'capture to CPU'
*                                      GT_FALSE - indicate the 'cmd Cpss Net' that the enh-UT are NOT using 'capture to CPU'
*                                       GT_TRUE
*/
GT_STATUS  cmdCpssNetEnhUtUseCaptureToCpu(
    IN GT_U32   useCapture
)
{
    if (enhUtUseCaptureToCpuPtr != NULL)
        *enhUtUseCaptureToCpuPtr = 0;
    return GT_OK;
}

#if (defined CHX_FAMILY)
/**
* @internal checkIsEnhUtUseCaptureToCpu function
* @endinternal
*
* @brief   check if the enhanced UT uses the 'Capture to CPU' , so those packet are
*         considered 'internal' and not need to be shown in the rxNetworkIf table
* @param[in] specificDeviceFormatPtr  - pointer to dxch/puma format.
*
* @retval 0                        - packet is NOT 'Enh Ut Captured packet'
* @retval 1                        - packet is 'Enh Ut Captured packet' so considered internal
*
* @note code based on function tgfTrafficGeneratorRxInCpuGet
*
*/
static GT_U32  checkIsEnhUtUseCaptureToCpu(
    IN GT_VOID  *specificDeviceFormatPtr
)
{
    CPSS_DXCH_NET_RX_PARAMS_STC   *dxChPcktParamsPtr   = NULL;

    if (enhUtUseCaptureToCpuPtr == NULL)
    {
        return 0;
    }
    if (*enhUtUseCaptureToCpuPtr == 0)
    {
        return 0;
    }
    if(specificDeviceFormatPtr == NULL)
    {
        return 0;
    }

    dxChPcktParamsPtr = (CPSS_DXCH_NET_RX_PARAMS_STC*) specificDeviceFormatPtr;

    if(CPSS_DXCH_NET_DSA_CMD_TO_CPU_E == dxChPcktParamsPtr->dsaParam.dsaType)
    {
        if((dxChPcktParamsPtr->dsaParam.dsaInfo.toCpu.cpuCode == CPSS_NET_LAST_USER_DEFINED_E)||
           (dxChPcktParamsPtr->dsaParam.dsaInfo.toCpu.cpuCode == CPSS_NET_INGRESS_MIRRORED_TO_ANLYZER_E))
        {
            /* 'Captured' packet */
            return 1;
        }
    }

    return 0;
}
#endif  /*(defined CHX_FAMILY)*/

/**
* @internal checkIsPacketInternal function
* @endinternal
*
* @brief   check if this packet is 'internal one' and not need to be shown in the
*         rxNetworkIf table
* @param[in] rxEntrtPry               - the pointer to the relevant RX entry in the captured packets.
*
* @retval 0                        - packet is NOT internal , so need to be reflected in the rxNetworkIf table
* @retval 1                        - packet is internal , so MUST NOT be be reflected in the rxNetworkIf table
*/
static GT_U32  checkIsPacketInternal(
    IN CMD_RX_ENTRY *rxEntrtPry
)
{

    if(forceShowAllPackets ||
       rxEntrtPry->entryType == GT_RX_ENTRY_TYPE_GENERIC_E)
    {
        return 0;
    }

    return 1;
}

/**
* @internal cmdCpssRxPkGet function
* @endinternal
*
* @brief   Get Next entry from rxNetworkIf table
*
* @param[in,out] currentIndexPtr          - the PREVIOUS entry's index (relevant only for getNext == GT_FALSE)
* @param[in,out] packetBufLenPtr          - the length of the user space for the packet
* @param[in,out] currentIndexPtr          - get current entry's index
*
* @param[out] packetBufPtr             - packet's buffer (pre allocated by the user)
* @param[in,out] packetBufLenPtr          - the length of the copied packet to gtBuf
* @param[out] packetLenPtr             - the Rx packet original length
* @param[out] devNumPtr                - packet's device number
* @param[out] queuePtr                 - The Rx queue in which the packet was received.
* @param[out] specificDeviceFormatPtr  - (pointer to) the specific device Rx info format.
*                                      format of DXCH
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - when no more entries
*
* @note GalTis:
*       Table - rxNetworkIf
*
*/
static GT_STATUS cmdCpssRxPkGet
(
    INOUT GT_U32        *currentIndexPtr,
    OUT GT_U8           *packetBufPtr,
    INOUT GT_U32        *packetBufLenPtr,
    OUT GT_U32          *packetLenPtr,
    OUT GT_U8           *devNumPtr,
    OUT GT_U8           *queuePtr,
    OUT void*           specificDeviceFormatPtr,
    IN  GT_BOOL         getNext
)
{
    GT_STATUS rc = GT_OK;
    GT_U32 currIndex; /* current index */
    GT_U32  filterThisPacket; /*indication that buffer MUST NOT be reflected in the 'refresh' of current table*/

    if(wasInitDone == GT_FALSE)
        cmdCpssRxInitLib();

    if(rxEntriesArr == NULL)
    {
        return GT_OUT_OF_CPU_MEM;
    }

    CMD_RX_MTX_LOCK_MAC;

    if(getNext == GT_TRUE)
    {
getNextPacket_lbl:
        /* increment the current index */
        (*currentIndexPtr)++;

        if(*currentIndexPtr == maxNumOfRxEntries)
        {
            *currentIndexPtr = 0;
        }
        /* check if reached the end of table (support also the "cyclic table")*/
        if(indexToInsertPacket == (*currentIndexPtr))
        {
            CMD_RX_MTX_UNLOCK_MAC;
            return GT_NO_MORE;
        }
    }
    else
    {
        if(rxTableMode == GALTIS_RX_ONCE_E || /* the table is flat --> start at index 0 */
           (rxTableMode == GALTIS_RX_CYCLIC_E && /* the table was not "looped" yet -- "cyclic table"*/
            rxEntriesArr[indexToInsertPacket].isEmpty == GT_TRUE))
        {
            *currentIndexPtr = 0;
        }
        else
        {
            *currentIndexPtr = indexToInsertPacket; /* support "cyclic table" */
        }

    }

    currIndex = *currentIndexPtr;

    /*Check if this buffer MUST NOT be reflected in the 'refresh' of current table */
    filterThisPacket = checkIsPacketInternal(&rxEntriesArr[currIndex]);
    if(filterThisPacket)
    {
        goto getNextPacket_lbl;
    }


    if(rxEntriesArr[currIndex].isEmpty == GT_FALSE)
    {
        /* if packet is smaller than gtBuf update the gtBufLength */
        if (rxEntriesArr[currIndex].bufferLen < *packetBufLenPtr)
        {
            *packetBufLenPtr = rxEntriesArr[currIndex].bufferLen;
        }

        if (*packetBufLenPtr > maxRxBufferSize)
        {
            /* the packet is bigger than the table packet size */
            *packetBufLenPtr = maxRxBufferSize;
        }

        /* set the real packet size */
        *packetLenPtr = rxEntriesArr[currIndex].packetOriginalLen;

        /* copy the packet to user buffer */
        cmdOsMemCpy(packetBufPtr,
                 rxEntriesArr[currIndex].packetBufferPtr,
                 *packetBufLenPtr);

        if (specificDeviceFormatPtr != NULL)
        {
            /* copy specific packet format info */
            rxEntriesArr[currIndex].copyRxInfoFunc(
                rxEntriesArr[currIndex].specificDeviceFormatPtr,
                specificDeviceFormatPtr);
        }
        *queuePtr  = rxEntriesArr[currIndex].queue;
        *devNumPtr = rxEntriesArr[currIndex].devNum;
    }
    else
    {
        rc = GT_NO_MORE;
    }

    CMD_RX_MTX_UNLOCK_MAC;
    return rc;
}

/**
* @internal cmdCpssRxPkGetFirst function
* @endinternal
*
* @brief   Get Next entry from rxNetworkIf table
*
* @param[in,out] packetBufLenPtr          - the length of the user space for the packet
*
* @param[out] currentIndexPtr          - get current entry's index
* @param[out] packetBufPtr             - packet's buffer (pre allocated by the user)
* @param[in,out] packetBufLenPtr          - the length of the copied packet to gtBuf
* @param[out] packetLenPtr             - the Rx packet original length
* @param[out] devNumPtr                - packet's device number
* @param[out] queuePtr                 - The Rx queue in which the packet was received.
* @param[out] specificDeviceFormatPtr  - (pointer to) the specific device Rx info format.
*                                      format of DXCH
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - when no more entries
*
* @note GalTis:
*       Table - rxNetworkIf
*
*/
GT_STATUS cmdCpssRxPkGetFirst
(
    OUT GT_U32          *currentIndexPtr,
    OUT GT_U8           *packetBufPtr,
    INOUT GT_U32        *packetBufLenPtr,
    OUT GT_U32          *packetLenPtr,
    OUT GT_U8           *devNumPtr,
    OUT GT_U8           *queuePtr,
    OUT void*           specificDeviceFormatPtr

)
{
    return cmdCpssRxPkGet(currentIndexPtr,packetBufPtr,
                      packetBufLenPtr,packetLenPtr,
                      devNumPtr,queuePtr,
                      specificDeviceFormatPtr,
                      GT_FALSE);/* get first */
}

/**
* @internal cmdCpssRxPkGetNext function
* @endinternal
*
* @brief   Get Next entry from rxNetworkIf table
*
* @param[in,out] packetBufLenPtr          - the length of the user space for the packet
* @param[in,out] currentIndexPtr          - the PREVIOUS entry's index
* @param[in,out] currentIndexPtr          - get current entry's index
*
* @param[out] packetBufPtr             - packet's buffer (pre allocated by the user)
* @param[in,out] packetBufLenPtr          - the length of the copied packet to gtBuf
* @param[out] packetLenPtr             - the Rx packet original length
* @param[out] devNumPtr                - packet's device number
* @param[out] queuePtr                 - The Rx queue in which the packet was received.
* @param[out] specificDeviceFormatPtr  - (pointer to) the specific device Rx info format.
*                                      format of DXCH / EXMX /DXSAL ...
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - when no more entries
*
* @note GalTis:
*       Table - rxNetworkIf
*
*/
GT_STATUS cmdCpssRxPkGetNext
(
    INOUT GT_U32        *currentIndexPtr,
    OUT GT_U8           *packetBufPtr,
    INOUT GT_U32        *packetBufLenPtr,
    OUT GT_U32          *packetLenPtr,
    OUT GT_U8           *devNumPtr,
    OUT GT_U8           *queuePtr,
    OUT void*           specificDeviceFormatPtr

)
{
    return cmdCpssRxPkGet(currentIndexPtr,packetBufPtr,
                      packetBufLenPtr,packetLenPtr,
                      devNumPtr,queuePtr,
                      specificDeviceFormatPtr,
                      GT_TRUE);/* get next */
}



/**
* @internal cmdCpssRxPktIsCaptureReady function
* @endinternal
*
* @brief   check if the galtis-cmd is ready to capture rx packets
*
* @retval GT_TRUE                  - the galtis-cmd is ready to capure rx packets
* @retval GT_FALSE                 - the galtis-cmd is NOT ready to capure rx packets
*/
GT_BOOL cmdCpssRxPktIsCaptureReady(void)
{
    return (doCapture == GT_TRUE && wasInitDone == GT_TRUE) ? GT_TRUE : GT_FALSE;
}



