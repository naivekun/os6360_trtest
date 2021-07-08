/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* wrapCpssPxNetif.c
*
* DESCRIPTION:
*       Wrapper functions for Pipe network interface API functions
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>
#include <galtisAgent/wrapUtil/cmdCpssNetReceive.h>


/* Feature specific includes */
#include <cpss/common/cpssTypes.h>
#include <cpss/px/networkIf/cpssPxNetIf.h>
#include <cpss/px/networkIf/cpssPxNetIfTypes.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpssCommon/private/prvCpssMemLib.h>
#include <galtisAgent/wrapCpss/px/networkif/cmdCpssPxNetTransmit.h>

/* table TxNetIf/ExtTxNetIf global variables */
static GT_U32      netTxCurrIndex=0; /*Index of last descriptor got from table*/
extern PX_PKT_DESC_STC * cpssPxTxPacketDescTbl;
/* Traffic generator transmit table descriptors */
extern GT_U32 wrCpssPxNetIfTxPxTblCapacity;
extern GT_U32 wrCpssPxNetIfTxGenPxTblCapacity[8];
extern GT_UINTPTR  cpssPxTxGenPacketEvReqHndl;

GT_POOL_ID txBuffersPoolId;

static const char hexcode[] = "0123456789ABCDEF";
static GT_BOOL  poolCreated = GT_FALSE;

/****************Table cpssPxNetIfSdmaRxCounters***************/
static GT_U8 gQueueIdx;

/**
* @internal wrCpssPxNetIfSdmaRxCountersGet function
* @endinternal
*
* @brief   For each packet processor, get the Rx packet counters from its SDMA
*         packet interface. Return the aggregate counter values for the given
*         traffic class queue. -- SDMA relate
*
* @note   APPLICABLE DEVICES:      ALL PCI/SDMA PX Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on bad parameters
*
* @note Counters are reset on every read.
*
*/
CMD_STATUS wrCpssPxNetIfSdmaRxCountersGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_SW_DEV_NUM                       devNum;
    CPSS_PX_NET_SDMA_RX_COUNTERS_STC    rxCounters;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    if (gQueueIdx >= CPSS_TC_RANGE_CNS)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* call cpss api function */
    result = cpssPxNetIfSdmaRxCountersGet(devNum, gQueueIdx, &rxCounters);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    inFields[0] = rxCounters.rxInPkts;
    inFields[1] = rxCounters.rxInOctets;

    /* pack and output table fields */
    fieldOutput("%d%d%d", gQueueIdx++, inFields[0], inFields[1]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");

    return CMD_OK;
}

CMD_STATUS wrCpssPxNetIfSdmaRxCountersGetFirst

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    gQueueIdx = 0;
    return wrCpssPxNetIfSdmaRxCountersGet(inArgs, inFields, numFields, outArgs);
}

CMD_STATUS wrCpssPxNetIfSdmaRxCountersGetNext

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssPxNetIfSdmaRxCountersGet(inArgs, inFields, numFields, outArgs);
}

/****************Table cpssPxNetIfSdmaRxErrorCount***************/
static GT_U8                                  gQueueNum;
static CPSS_NET_SDMA_RX_ERROR_COUNTERS_STC    gRxErrCount;
/**
* @internal wrCpssPxNetIfSdmaRxErrorCountGetFirst function
* @endinternal
*
* @brief   Returns the total number of Rx resource errors that occurred on a given
*         Rx queue . -- SDMA relate
*
* @note   APPLICABLE DEVICES:      ALL PCI/SDMA PX Devices
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - otherwise.
*
* @note The counters returned by this function reflects the number of Rx errors
*       that occurred since the last call to this function.
*
*/
CMD_STATUS wrCpssPxNetIfSdmaRxErrorCountGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_SW_DEV_NUM   devNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    gQueueNum = 0;

    /* call cpss api function */
    result = cpssPxNetIfSdmaRxErrorCountGet(devNum, &gRxErrCount);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    /* pack and output table fields */
    fieldOutput("%d%d", gQueueNum, gRxErrCount.counterArray[gQueueNum]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");
    return CMD_OK;
}

/**
* @internal wrCpssPxNetIfSdmaRxErrorCountGetNext function
* @endinternal
*
* @brief   Returns the total number of Rx resource errors that occurred on a given
*         Rx queue . -- SDMA relate
*
* @note   APPLICABLE DEVICES:      ALL PCI/SDMA PX Devices
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - otherwise.
*
* @note The counters returned by this function reflects the number of Rx errors
*       that occurred since the last call to this function.
*
*/
CMD_STATUS wrCpssPxNetIfSdmaRxErrorCountGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    gQueueNum++;

    if (gQueueNum >= CPSS_TC_RANGE_CNS) 
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* pack and output table fields */
    fieldOutput("%d%d", gQueueNum, gRxErrCount.counterArray[gQueueNum]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "%f");
    return CMD_OK;
}

/**
* @internal wrCpssPxNetIfSdmaRxQueueEnable function
* @endinternal
*
* @brief   Enable/Disable the specified traffic class queue for RX
*         on all packet processors in the system. -- SDMA relate
*
* @note   APPLICABLE DEVICES:      ALL PCI/SDMA PX Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad parameters
*/
CMD_STATUS wrCpssPxNetIfSdmaRxQueueEnable
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_SW_DEV_NUM   devNum;
    GT_U8   queue;
    GT_BOOL enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    queue = (GT_U8)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* call cpss api function */
    result = cpssPxNetIfSdmaRxQueueEnable(devNum, queue, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssPxNetIfSdmaRxQueueEnableGet function
* @endinternal
*
* @brief   Get status of the specified traffic class queue for RX
*         packets in CPU. -- SDMA relate
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
CMD_STATUS wrCpssPxNetIfSdmaRxQueueEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_SW_DEV_NUM   devNum;
    GT_U8   queue;
    GT_BOOL enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    queue = (GT_U8)inArgs[1];

    /* call cpss api function */
    result = cpssPxNetIfSdmaRxQueueEnableGet(devNum, queue, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**
* @internal wrCpssPxNetIfSdmaTxQueueEnable function
* @endinternal
*
* @brief   Enable/Disable the specified traffic class queue for TX
*         on all packet processors in the system. -- SDMA relate
*
* @note   APPLICABLE DEVICES:      ALL PCI/SDMA PX Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad parameters
*/
CMD_STATUS wrCpssPxNetIfSdmaTxQueueEnable
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_SW_DEV_NUM   devNum;
    GT_U8   queue;
    GT_BOOL enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    queue = (GT_U8)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* call cpss api function */
    result = cpssPxNetIfSdmaTxQueueEnable(devNum, queue, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssPxNetIfSdmaTxQueueEnableGet function
* @endinternal
*
* @brief   Get status of the specified traffic class queue for TX
*         packets from CPU. -- SDMA relate
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on CPU port is not SDMA
* @retval GT_BAD_PTR               - on NULL ptr
*/
CMD_STATUS wrCpssPxNetIfSdmaTxQueueEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_SW_DEV_NUM   devNum;
    GT_U8   queue;
    GT_BOOL enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    queue = (GT_U8)inArgs[1];

    /* call cpss api function */
    result = cpssPxNetIfSdmaTxQueueEnableGet(devNum, queue, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/****************************
    TX
*****************************/

/**
* @internal setBufferArraysOfEqualSize function
* @endinternal
*
* @brief   build GT_BYTE_ARRY from string expanding or trunkating to size
*
* @param[in] sourceDataPtr            - byte array buffer (hexadecimal string)
* @param[in] totalSize                - exact total size of the buffer in bytes
* @param[in] bufferSize               - wanted size of each data buffer
*
* @param[out] pcktData[MAX_NUM_OF_SDMA_BUFFERS_PER_CHAIN] - array of byte array data
* @param[out] pcktDataLen[MAX_NUM_OF_SDMA_BUFFERS_PER_CHAIN] - array of byte array size
* @param[out] numOfBuffs               - number of buffers the data was splited to.
*                                       None
*/
static GT_STATUS setBufferArraysOfEqualSize
(
    IN GT_U8   *sourceDataPtr,
    IN GT_U32   totalSize,
    IN GT_U32   bufferSize,
    OUT GT_U8   *pcktData[MAX_NUM_OF_SDMA_BUFFERS_PER_CHAIN],
    OUT GT_U32  pcktDataLen[MAX_NUM_OF_SDMA_BUFFERS_PER_CHAIN],
    OUT GT_U32  *numOfBuffs

)
{
    GT_U32 element;
    GT_U32 len = totalSize;
    GT_U32 numOfBuffers = len / bufferSize;
    GT_U32 sizeOfLastBuff = len % bufferSize;
    GT_U32 i;

    numOfBuffers = (sizeOfLastBuff > 0) ? numOfBuffers + 1 : numOfBuffers;

    if (numOfBuffers > MAX_NUM_OF_SDMA_BUFFERS_PER_CHAIN)
        return GT_NO_RESOURCE;

    if(cmdOsPoolGetBufFreeCnt(txBuffersPoolId) < numOfBuffers)
        return GT_NO_RESOURCE;

    /* if the length of the data is bigger than the length given by the user,
       the data is truncate otherwise it is expanded */

    for(i = 0; i < numOfBuffers ; i++)
    {
        if(len < bufferSize)
            bufferSize = len;
        else
            len = len - bufferSize;

        pcktData[i] = cmdOsPoolGetBuf(txBuffersPoolId);

        /*for every buffer, we will copy the data*/
        for (element = 0; element < bufferSize; element++)
        {
            pcktData[i][element] =(GT_U8)( (GT_UINTPTR)(cmdOsStrChr(hexcode, cmdOsToUpper(*sourceDataPtr++)) - (GT_UINTPTR)hexcode)<< 4);

            pcktData[i][element]+= (GT_U8)((GT_UINTPTR)(cmdOsStrChr(hexcode, cmdOsToUpper(*sourceDataPtr++)) - (GT_UINTPTR)hexcode) );
        }

        pcktDataLen[i] = bufferSize;

    }

    *numOfBuffs = numOfBuffers;

    return GT_OK;
}

/**
* @internal freeBufferArraysOfEqualSize function
* @endinternal
*
* @brief   Free GT_BYTE_ARRY from string
*
* @param[in] pcktData[MAX_NUM_OF_SDMA_BUFFERS_PER_CHAIN] - array of byte array data
* @param[in] pcktDataLen[MAX_NUM_OF_SDMA_BUFFERS_PER_CHAIN] - array of byte array size
*                                       None
*
* @note Toolkit:
*
*/
static GT_VOID freeBufferArraysOfEqualSize
(
    IN GT_U8   *pcktData[MAX_NUM_OF_SDMA_BUFFERS_PER_CHAIN],
    IN GT_U32  pcktDataLen[MAX_NUM_OF_SDMA_BUFFERS_PER_CHAIN]
)
{

    GT_U32 i;
    for (i=0; i<MAX_NUM_OF_SDMA_BUFFERS_PER_CHAIN; i++)
    {

        /*nothing to free */
        if((pcktData[i] == NULL) || (pcktDataLen[i] == 0))
            break;

        cmdOsPoolFreeBuf(txBuffersPoolId, pcktData[i]);
   }
}

/*============================
          TX
=============================*/

/**
* @internal wrCpssPxNetIfTxStart function
* @endinternal
*
* @brief   Starts transmitting of packets
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
*
* @note none
*
*/
CMD_STATUS wrCpssPxNetIfTxStart
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* call api function */
    result = cmdCpssPxTxStart();

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssPxNetIfTxStop function
* @endinternal
*
* @brief   Stop transmitting of packets.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
*
* @note none
*
*/
CMD_STATUS wrCpssPxNetIfTxStop
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* call api function */
    result = cmdCpssPxTxStop();

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssPxNetIfTxSetMode function
* @endinternal
*
* @brief   This command will set the the transmit parameters.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
*
* @note none
*
*/
CMD_STATUS wrCpssPxNetIfTxSetMode
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U32 lCyclesNum;
    GT_U32 lGap;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    lCyclesNum = (GT_U32)inArgs[0];
    lGap = (GT_U32)inArgs[1];

    /* call api function */
    result = cmdCpssPxTxSetMode(lCyclesNum, lGap);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssPxNetIfTxGetMode function
* @endinternal
*
* @brief   This command will get the the transmit parameters.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
*
* @note none
*
*/
CMD_STATUS wrCpssPxNetIfTxGetMode
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U32 lCyclesNum;
    GT_U32 lGap;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* call api function */
    result = cmdCpssPxTxGetMode(&lCyclesNum, &lGap);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", lCyclesNum,lGap);
    return CMD_OK;
}


/**
* @internal wrTxSystemReset function
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
static GT_VOID wrTxSystemReset
(
    GT_VOID
)
{
    if(txBuffersPoolId)
    {
        cmdOsPoolDeletePool(txBuffersPoolId);
        txBuffersPoolId = 0;
    }

    if(cpssPxTxPacketDescTbl)
    {
        cmdOsFree(cpssPxTxPacketDescTbl);
        cpssPxTxPacketDescTbl = 0;
    }

    netTxCurrIndex = 0;

    wrCpssPxNetIfTxPxTblCapacity = 0;

    poolCreated = GT_FALSE;

    cpssEventDestroy(cpssPxTxGenPacketEvReqHndl);
    cpssPxTxGenPacketEvReqHndl = 0;

}

/*
* Table: TxNetIf
*
* Description:
*     Transmitted packet descriptor table.
*
* Comments:
*/
/**
* @internal wrCpssPxNetIfSetTxNetIfTblEntryFromCpu function
* @endinternal
*
* @brief   Creates new transmit parameters description entry in the table.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
CMD_STATUS wrCpssPxNetIfSetTxNetIfTblEntryFromCpu
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS status;
    PX_PKT_DESC_STC * netTxPacketDesc;
    GT_U32   entryIndex=0;
    GT_U32   numOfBuffs;

    /* check for valid arguments */
    if(inArgs == NULL || outArgs == NULL)
        return CMD_AGENT_ERROR;

    /* creat the pool only once */
    if(poolCreated == GT_FALSE)
    {
        /* register function to reset DB so after system reset we can send traffic again */
        wrCpssRegisterResetCb(wrTxSystemReset);
        /* create pool of buffers from Cache */
        status = cmdOsPoolCreateDmaPool(
                                  TX_BUFFER_SIZE + 64 /* ALIGN_ADDR_CHUNK */,
                                  GT_4_BYTE_ALIGNMENT,
                                  MAX_NUM_OF_BUFFERS_AT_POOL,
                                  GT_TRUE,
                                  &txBuffersPoolId);

        if (status != GT_OK)
        {
                cmdOsPrintf("ERROR-PoolCreateDmaPool\n\n");
                return status;
        }

        poolCreated = GT_TRUE;
    }

    netTxPacketDesc = cmdOsMalloc(sizeof(PX_PKT_DESC_STC));
    if (netTxPacketDesc==NULL)
    {
        galtisOutput(outArgs, GT_NO_RESOURCE, "");
        return CMD_OK;
    }
    cmdOsMemSet(netTxPacketDesc, 0, sizeof(PX_PKT_DESC_STC));


    /* Non zero data len */
    if(((GT_U32)inFields[8] == 0))
    {
        galtisOutput(outArgs, GT_BAD_PARAM, "");
        return CMD_OK;
    }
    status = setBufferArraysOfEqualSize((GT_U8*)inFields[7],
                               (GT_U32)inFields[8],
                               TX_BUFFER_SIZE,
                               netTxPacketDesc->pcktData,
                               netTxPacketDesc->pcktDataLen,
                               &numOfBuffs);


    if (status != GT_OK)
    {
        cmdOsFree(netTxPacketDesc);
        freeBufferArraysOfEqualSize(netTxPacketDesc->pcktData,
                                    netTxPacketDesc->pcktDataLen);
        galtisOutput(outArgs, GT_NO_RESOURCE, "");
        return CMD_OK;
    }


    netTxPacketDesc->numOfBuffers = numOfBuffs;
    netTxPacketDesc->devNum = (GT_SW_DEV_NUM)inFields[0];
    netTxPacketDesc->entryId = (GT_U32)inFields[1];
    netTxPacketDesc->sdmaInfo.txQueue = (GT_U8)inFields[2];
    netTxPacketDesc->sdmaInfo.recalcCrc = (GT_BOOL)inFields[3];
    netTxPacketDesc->pcktsNum = (GT_U32)inFields[4];
    netTxPacketDesc->gap = (GT_U32)inFields[5];
    netTxPacketDesc->waitTime = (GT_U32)inFields[6];

    netTxPacketDesc->numSentPackets = (GT_U32)inFields[10];


    /* Insert into table */
    status = cmdCpssPxTxSetPacketDesc(
                                netTxPacketDesc->devNum,
                                netTxPacketDesc->entryId,
                                netTxPacketDesc,
                                &entryIndex);

    cmdOsFree(netTxPacketDesc);
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxNetIfGetTxNetIfTblEntryFromCpu function
* @endinternal
*
* @brief   Get entry in the table.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
static CMD_STATUS wrCpssPxNetIfGetTxNetIfTblEntryFromCpu
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_BYTE_ARRY    lGtBuff;
    GT_U32          totalLength;
    GT_U32          i;

    /* check for valid arguments */
    if(inArgs == NULL || outArgs == NULL)
        return CMD_AGENT_ERROR;

    {
        GT_STATUS status;
        PX_PKT_DESC_STC SpecificPcktDesc;

        /* init to zeros */
        cmdOsMemSet(&SpecificPcktDesc, 0, sizeof(SpecificPcktDesc));

        status = cmdCpssPxTxGetPacketDesc(&netTxCurrIndex, &SpecificPcktDesc);
        if (status != GT_OK)
        {
            /* indicates that table is empty or (GT_U32)inArgs[0] is invalid */
            if( (status == GT_EMPTY) ||
                (status == GT_NO_SUCH) ||
                (status == GT_NOT_FOUND) )
                galtisOutput(outArgs, GT_OK, "%d", -1);
            else
                galtisOutput(outArgs, status, "");

            cmdCpssPxTxEndGetPacketDesc();
            return CMD_OK;
        }

         /* Mapping structure to fields */
         inFields[0] = SpecificPcktDesc.devNum;
         inFields[1] = SpecificPcktDesc.entryId;
         inFields[2] = SpecificPcktDesc.sdmaInfo.txQueue;
         inFields[3] = SpecificPcktDesc.sdmaInfo.recalcCrc;
         inFields[4] = SpecificPcktDesc.pcktsNum;
         inFields[5] = SpecificPcktDesc.gap;
         inFields[6] = SpecificPcktDesc.waitTime;

         inFields[9] = SpecificPcktDesc.numOfBuffers;
         inFields[10] = SpecificPcktDesc.numSentPackets;

         /* Send to Galtis only the first buffer of the packet */
         lGtBuff.data = cmdOsMalloc(SpecificPcktDesc.pcktDataLen[0]);
         lGtBuff.length = SpecificPcktDesc.pcktDataLen[0];
         if (!lGtBuff.data)
         {
             galtisOutput(outArgs, GT_NO_RESOURCE, "");
             return CMD_OK;
         }

         totalLength = 0;
         for( i = 0; i < MAX_NUM_OF_SDMA_BUFFERS_PER_CHAIN; i++)
         {
             if(SpecificPcktDesc.pcktDataLen[i] == 0)
                 break; /*End of the chain */

             totalLength += SpecificPcktDesc.pcktDataLen[i];
         }

         cmdOsMemCpy(lGtBuff.data, SpecificPcktDesc.pcktData[0],
             SpecificPcktDesc.pcktDataLen[0]);

         fieldOutput("%d%d%d%d%d%d%d%s%d%d%d",
             inFields[0], inFields[1], inFields[2], inFields[3],
             inFields[4], inFields[5], inFields[6], galtisBArrayOut(&lGtBuff), 
             totalLength, inFields[9], inFields[10]);

         galtisOutput(outArgs, GT_OK, "%f");

         cmdOsFree(lGtBuff.data);
         cmdOsFree(SpecificPcktDesc.pcktData[0]);
    }

    return CMD_OK;
}


/**
* @internal wrCpssPxNetIfGetTxNetIfTblFirstEntryFromCpu function
* @endinternal
*
* @brief   Get first entry in the table.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
CMD_STATUS wrCpssPxNetIfGetTxNetIfTblFirstEntryFromCpu
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS status;

    /* check for valid arguments */
    if(inArgs == NULL || outArgs == NULL)
        return CMD_AGENT_ERROR;

    netTxCurrIndex = 0;

    status = cmdCpssPxTxBeginGetPacketDesc();
    if (status != GT_OK)
    {
        galtisOutput(outArgs, status, "");
        return CMD_OK;
    }

    return wrCpssPxNetIfGetTxNetIfTblEntryFromCpu(inArgs, inFields, numFields, outArgs);
}

/**
* @internal wrCpssPxNetIfGetTxNetIfTblNextEntryFromCpu function
* @endinternal
*
* @brief   Get next entry in the table.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
CMD_STATUS wrCpssPxNetIfGetTxNetIfTblNextEntryFromCpu
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    netTxCurrIndex++;
    return wrCpssPxNetIfGetTxNetIfTblEntryFromCpu(inArgs, inFields, numFields, outArgs);
}

/**
* @internal wrCpssPxNetIfDeleteTxNetIfTblEntryFromCpu function
* @endinternal
*
* @brief   Delete packet descriptor entry from the table.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
CMD_STATUS wrCpssPxNetIfDeleteTxNetIfTblEntryFromCpu
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS status;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;


    status = cmdCpssPxTxDelPacketDesc((GT_SW_DEV_NUM)inFields[0], (GT_U32)inFields[1]);
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxNetIfClearTxNetIfTableFromCpu function
* @endinternal
*
* @brief   Clear all packet descriptors entries from the table.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
CMD_STATUS wrCpssPxNetIfClearTxNetIfTableFromCpu
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS status;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    status = cmdCpssPxTxClearPacketDesc();
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

static GT_U32 netRxExtLastIndex;

/**
* @internal wrCpssPxNetIfRxPacketTableGet function
* @endinternal
*
* @brief   Get first/next entry from the PX rxNetIf table.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval getNext                  - GT_TRUE - "get next"
* @retval GT_FALSE                 - "get first"
*
* @note none
*
*/
CMD_STATUS wrCpssPxNetIfRxPacketTableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER],
    IN  GT_BOOL getNext,
    IN  GT_U32  version
)
{
    GT_BYTE_ARRY byteArray;
    GT_STATUS rc;
    GT_U32 packetActualLength;
    GT_U8  packetBuff[GALTIS_RX_BUFFER_MAX_SIZE_CNS];
    GT_U32 buffLen = GALTIS_RX_BUFFER_MAX_SIZE_CNS;
    GT_U8  devNum;
    GT_U8  queue;
    GT_U8*      packetString;
    GT_U32  ii;


    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if (getNext == GT_FALSE)
    {
        rc = cmdCpssRxPkGetFirst(&netRxExtLastIndex,
                                 packetBuff, &buffLen, &packetActualLength,
                                 &devNum,&queue,
                                 NULL);
    }
    else
    {
        rc = cmdCpssRxPkGetNext(&netRxExtLastIndex,
                                 packetBuff, &buffLen, &packetActualLength,
                                 &devNum,&queue,
                                 NULL);
    }

    if (rc != GT_OK)
    {
        if (rc == GT_NO_MORE)
        {
            galtisOutput(outArgs, GT_OK, "%d", -1);
        }
        else
        {
            galtisOutput(outArgs, rc, "");
        }

        return CMD_OK;
    }


    byteArray.data    = packetBuff;
    byteArray.length  = buffLen;

    /* initialize the index of "inFields" parameters */
    ii = 0;

    inFields[ii++] = devNum;                                      /*0*/
    inFields[ii++] = queue;                                       /*1*/

    ii++;/* one for the packetString */
    packetString = galtisBArrayOut(&byteArray);                   /*2*/

    inFields[ii++] = packetActualLength;                          /*3*/



    /* pack and output table fields */
    /*            0 1 2 3 */
    fieldOutput("%d%d%s%d",
                inFields[0],
                inFields[1],
                packetString,
                inFields[3]);

    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}

/**
* @internal wrCmdPxRxStartCapture function
* @endinternal
*
* @brief   Start collecting the received packets
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
*
* @note none
*
*/
CMD_STATUS wrCmdPxRxStartCapture
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* call api function */
    result = cmdCpssRxStartCapture();

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCmdPxRxStopCapture function
* @endinternal
*
* @brief   Stop collecting the received packets
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
*
* @note none
*
*/
CMD_STATUS wrCmdPxRxStopCapture
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* call api function */
    result = cmdCpssRxStopCapture();

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCmdPxRxSetMode function
* @endinternal
*
* @brief   set received packets collection mode and parameters
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
*
* @note none
*
*/
CMD_STATUS wrCmdPxRxSetMode
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GALTIS_RX_MODE_ENT lRxMode;
    GT_U32 lBuferSize;
    GT_U32 lNumOfEnteries;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    lRxMode = (GALTIS_RX_MODE_ENT)inArgs[0];
    lBuferSize = (GT_U32)inArgs[1];
    lNumOfEnteries = (GT_U32)inArgs[2];

    /* call tapi api function */
    result = cmdCpssRxSetMode(lRxMode, lBuferSize, lNumOfEnteries);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCmdPxRxGetMode function
* @endinternal
*
* @brief   Get received packets collection mode and parameters
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
*
* @note none
*
*/
CMD_STATUS wrCmdPxRxGetMode
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GALTIS_RX_MODE_ENT lMode;
    GT_U32 lBuffSize;
    GT_U32 lNumEntries;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* call api function */
    result = cmdCpssRxGetMode(&lMode, &lBuffSize, &lNumEntries);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d", lMode, lBuffSize,
        lNumEntries);
    return CMD_OK;
}

/**
* @internal wrCpssPxNetIfRxPacketTableGetFirst function
* @endinternal
*
* @brief   Get first entry from the rxNetIf table.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
CMD_STATUS wrCpssPxNetIfRxPacketTableGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssPxNetIfRxPacketTableGet(inArgs,inFields,numFields,outArgs,
                    GT_FALSE, 0);/* get first */
}

/**
* @internal wrCpssPxNetIfRxPacketTableGetNext function
* @endinternal
*
* @brief   Get next entry from the rxNetIf table.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
CMD_STATUS wrCpssPxNetIfRxPacketTableGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssPxNetIfRxPacketTableGet(inArgs,inFields,numFields,outArgs,
                    GT_TRUE, 0);/* get next */
}

/**
* @internal wrCpssPxNetIfRxPacketTableClear function
* @endinternal
*
* @brief   clear all entries from the rxNetIf table.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
CMD_STATUS wrCpssPxNetIfRxPacketTableClear
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cmdCpssRxPktClearTbl();
    galtisOutput(outArgs, GT_OK, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxNetIfSdmaRxResourceErrorModeSet function
* @endinternal
*
* @brief   Set a bit per TC queue which defines the behavior in case of
*         RX resource error
*
* @note   APPLICABLE DEVICES:      Pipe
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number, queue, mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - the request is not supported
*/
static CMD_STATUS wrCpssPxNetIfSdmaRxResourceErrorModeSet
(

    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;
    IN  GT_U8                                   devNum;
    IN  GT_U8                                   queue;
    IN  CPSS_PX_NET_RESOURCE_ERROR_MODE_ENT     mode;

    /* Check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    queue  = (GT_U8)inArgs[1];
    mode   = (CPSS_PX_NET_RESOURCE_ERROR_MODE_ENT)inArgs[2];

    /* call cpss API function */
    result = cpssPxNetIfSdmaRxResourceErrorModeSet(devNum, queue, mode);

    /* pack output arguments to GalTis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxNetIfSdmaRxResourceErrorModeGet function
* @endinternal
*
* @brief   Get a bit per TC queue which defines the behavior in case of
*         RX resource error
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number, queue, mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - the request is not supported
* @retval GT_BAD_PTR               - on NULL ptr
*/
CMD_STATUS wrCpssPxNetIfSdmaRxResourceErrorModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8                                   devNum;
    GT_U8                                   queue;
    CPSS_PX_NET_RESOURCE_ERROR_MODE_ENT     mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    queue = (GT_U8)inArgs[1];

    /* call cpss api function */
    result = cpssPxNetIfSdmaRxResourceErrorModeGet(devNum, queue, &mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mode);
    return CMD_OK;
}

/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
    {"cpssPxNetIfSdmaRxCountersGetFirst",
        &wrCpssPxNetIfSdmaRxCountersGetFirst,
        1, 0},

    {"cpssPxNetIfSdmaRxCountersGetNext",
        &wrCpssPxNetIfSdmaRxCountersGetNext,
        1, 0},

    {"cpssPxNetIfSdmaRxErrorCountGetFirst",
        &wrCpssPxNetIfSdmaRxErrorCountGetFirst,
        1, 0},

    {"cpssPxNetIfSdmaRxErrorCountGetNext",
        &wrCpssPxNetIfSdmaRxErrorCountGetNext,
        1, 0},

    {"cpssPxNetIfSdmaRxQueueEnable",
        &wrCpssPxNetIfSdmaRxQueueEnable,
        3, 0},

    {"cpssPxNetIfSdmaRxQueueEnableGet",
        &wrCpssPxNetIfSdmaRxQueueEnableGet,
        2, 0},

    {"cpssPxNetIfSdmaTxQueueEnable",
        &wrCpssPxNetIfSdmaTxQueueEnable,
        3, 0},

    {"cpssPxNetIfSdmaTxQueueEnableGet",
        &wrCpssPxNetIfSdmaTxQueueEnableGet,
        2, 0},

    {"cpssPxNetIfTxFromCpuSet",
        &wrCpssPxNetIfSetTxNetIfTblEntryFromCpu,
        1, 11},

    {"cpssPxNetIfTxFromCpuGetFirst",
        &wrCpssPxNetIfGetTxNetIfTblFirstEntryFromCpu,
        1, 0},

    {"cpssPxNetIfTxFromCpuGetNext",
        &wrCpssPxNetIfGetTxNetIfTblNextEntryFromCpu,
        1, 0},

    {"cpssPxNetIfTxFromCpuDelete",
        &wrCpssPxNetIfDeleteTxNetIfTblEntryFromCpu,
        1, 11},

    {"cpssPxNetIfTxFromCpuClear",
        &wrCpssPxNetIfClearTxNetIfTableFromCpu,
        1, 0},

    {"cpssPxTxSetMode",
        &wrCpssPxNetIfTxSetMode,
        2, 0},

    {"cpssPxTxGetMode",
        &wrCpssPxNetIfTxGetMode,
        0, 0},

    {"cpssPxTxStart",
        &wrCpssPxNetIfTxStart,
        0, 0},

    {"cpssPxTxStop",
        &wrCpssPxNetIfTxStop,
        0, 0},

    /* Rx packets table -- start */
    {"cmdPxRxStartCapture",
        &wrCmdPxRxStartCapture,
        0, 0},

    {"cmdPxRxStopCapture",
        &wrCmdPxRxStopCapture,
        0, 0},

    {"cmdPxRxSetMode",
        &wrCmdPxRxSetMode,
        3, 0},

    {"cmdPxRxGetMode",
        &wrCmdPxRxGetMode,
        0, 0},

    {"cpssPxNetIfRxPacketTableGetFirst",
        &wrCpssPxNetIfRxPacketTableGetFirst,
        0, 0},

    {"cpssPxNetIfRxPacketTableGetNext",
        &wrCpssPxNetIfRxPacketTableGetNext,
        0, 0},

    {"cpssPxNetIfRxPacketTableClear",
        &wrCpssPxNetIfRxPacketTableClear,
        0, 0},
    /* Rx packets table -- End */

    {"cpssPxNetIfSdmaRxResourceErrorModeSet",
        &wrCpssPxNetIfSdmaRxResourceErrorModeSet,
        3, 0},

    {"cpssPxNetIfSdmaRxResourceErrorModeGet",
        &wrCpssPxNetIfSdmaRxResourceErrorModeGet,
        2, 0},
};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssPxNetIf function
* @endinternal
*
* @brief   Library database initialization function.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*
* @note none
*
*/
GT_STATUS cmdLibInitCpssPxNetIf
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}


