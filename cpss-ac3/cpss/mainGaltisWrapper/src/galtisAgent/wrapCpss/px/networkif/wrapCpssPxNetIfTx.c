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
* @file wrapCpssPxNetIfTx.c
*
* @brief Wrapper functions for NetworkIf Tx CPSS functions for Pipe devices
*
* @version   1
********************************************************************************
*/

#include <cmdShell/common/cmdCommon.h>

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/events/cpssGenEventUnifyTypes.h>
#include <cpss/generic/events/cpssGenEventRequests.h>
#include <cpss/px/networkIf/cpssPxNetIfTypes.h>
#include <cpss/px/networkIf/cpssPxNetIf.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpssCommon/private/prvCpssMemLib.h>
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <galtisAgent/wrapCpss/px/networkif/cmdCpssPxNetTransmit.h>


/*******************************************************************************
 * Internal definitions
 ******************************************************************************/
#define GT_GALTIS_TRANSMITE_TASK_PRIOR_CNS            100

/******************************** Globals *************************************/

/* Transmit Table descriptors */
GT_U32 wrCpssPxNetIfTxPxTblCapacity = 0;
PX_PKT_DESC_STC * cpssPxTxPacketDescTbl = NULL;

/* Traffic generator transmit table descriptors */
GT_U32 wrCpssPxNetIfTxGenPxTblCapacity[8] = {0};

/* Transmit task ID */
static GT_TASK taskId = 0;
static GT_SEM  txCmdSemId;                      /* Module semaphore id  */
static GT_BOOL taskBusyFlag = GT_FALSE;

/* Transmit Task operation mode  */
static CPSS_TX_MODE_SETITNGS_STC modeSettings = {1, 1};
static GT_BOOL flagStopTransmit = GT_FALSE;

/******************************** Externals ***********************************/
extern GT_POOL_ID txBuffersPoolId;

GT_UINTPTR  cpssPxTxGenPacketEvReqHndl = 0;


/*============================
          TX Transmit
=============================*/

/**
* @internal startPacketTransmission function
* @endinternal
*
* @brief   The thread function that performs transmitting of packets defined
*         in the table.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note GalTis:
*       None
*
*/
static GT_STATUS startPacketTransmission(void)
{
    GT_U32      i;
    GT_U32      cycles;
    GT_STATUS   status = GT_OK;

    CPSS_PX_NET_TX_PARAMS_STC   cpssPcktParams;
    GT_SW_DEV_NUM               devNum=0;
    GT_SW_DEV_NUM               dev; /* device iterator */

    if (cpssPxTxGenPacketEvReqHndl == 0)
    {
        CPSS_UNI_EV_CAUSE_ENT     evCause[1] = { CPSS_PP_TX_BUFFER_QUEUE_E };

        if (cmdCpssEventBind(evCause, 1, &cpssPxTxGenPacketEvReqHndl) != GT_OK)
        {
            return GT_FAIL;
        }

        for(i = 0; i < 8; i++)
        {
            /* unmask this event with all HW devices */
            status = cmdCpssEventDeviceMaskSet((GT_U8)i, CPSS_PP_TX_BUFFER_QUEUE_E, CPSS_EVENT_UNMASK_E);
            if(status != GT_OK)
            {
                /* there are events that not relevant to PP device,
                vice versa */
                continue;
            }
        }

    }

    cycles = 0;
    while (((cycles < modeSettings.cyclesNum) ||
            (modeSettings.cyclesNum == -1)) && (flagStopTransmit == GT_FALSE))
    {
        for(i = 0; i < wrCpssPxNetIfTxPxTblCapacity; i++) /*For each descriptor*/
        {
            GT_U32 j;

            /* If the transmitting was disabled */
            if (flagStopTransmit == GT_TRUE)
            {
                break;
            }


            if (GT_TRUE != cpssPxTxPacketDescTbl[i].valid)
            {
                /* entry is not valid */
                continue;
            }

            /* Device number per Tx descriptor table entry */
            devNum = cpssPxTxPacketDescTbl[i].devNum;

            for (j = 0; j < cpssPxTxPacketDescTbl[i].pcktsNum; j++)
            {/*For packet sent by this descriptor*/
                /* If the transmitting was disabled */
                if (flagStopTransmit == GT_TRUE)
                {
                    break;
                }


                cpssPcktParams.recalcCrc = cpssPxTxPacketDescTbl[i].sdmaInfo.recalcCrc;
                cpssPcktParams.txQueue = cpssPxTxPacketDescTbl[i].sdmaInfo.txQueue;

                /* find the first active device */
                if(0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
                {
                    for(dev = 0; dev < PRV_CPSS_MAX_PP_DEVICES_CNS; dev++)
                    {
                        if(prvCpssPpConfig[dev] != NULL)
                        {
                            devNum = dev;
                            break;
                        }
                    }
                }


                status = GT_EMPTY;
                while (status == GT_EMPTY)
                {

                    status = cpssPxNetIfSdmaSyncTxPacketSend(
                                            devNum, &cpssPcktParams,
                                            cpssPxTxPacketDescTbl[i].pcktData,
                                            cpssPxTxPacketDescTbl[i].pcktDataLen,
                                            cpssPxTxPacketDescTbl[i].numOfBuffers);

                    /* Not enough descriptors to do the sending */
                    if (status == GT_EMPTY)
                    {
                        /* wait and try to send the packet again */
                        cmdOsTimerWkAfter(1);
                    }
                }

                if((status != GT_OK) && (status != GT_NO_RESOURCE))
                {
                    cmdOsPrintf("Failed to send the packet, status = %d \n", status);
                    return status;
                }

                /* in case transmit succeed */
                cpssPxTxPacketDescTbl[i].numSentPackets++;


                /* wait n milliseconds before sending next packet */
                if(cpssPxTxPacketDescTbl[i].gap != 0)
                {
                    cmdOsTimerWkAfter(cpssPxTxPacketDescTbl[i].gap);
                }
            }

            /* wait n milliseconds before moving to the next entry */
            if(cpssPxTxPacketDescTbl[i].waitTime != 0)
            {
                cmdOsTimerWkAfter(cpssPxTxPacketDescTbl[i].waitTime);
            }
        }


        /* wait n milliseconds before starting the next cycle */
        if(modeSettings.gap != 0)
        {
            cmdOsTimerWkAfter(modeSettings.gap);
        }
        /* move to the next cycle */
        cycles++;
    }

    return GT_OK;
}

/*******************************************************************************
* cpssPxPacketTransminitionTask
*
* DESCRIPTION:
*       Galtis task for packet transmission.
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
* GalTis:
*       None
*
*******************************************************************************/
GT_STATUS __TASKCONV cpssPxPacketTransminitionTask()
{
    GT_STATUS rc;

    /* while forever */
    while(1)
    {
        /* wait on the TX command semaphore */
        taskBusyFlag = GT_FALSE;
        cmdOsSigSemWait(txCmdSemId, CPSS_OS_SEM_WAIT_FOREVER_CNS);
        taskBusyFlag = GT_TRUE;

        rc = startPacketTransmission();
        if (rc == GT_ABORTED)
        {
            break;
        }
    }
    return GT_OK;
}

/**
* @internal cmdCpssPxTxSetPacketDesc function
* @endinternal
*
* @brief   Creates new transmit parameters description entry in the table.
*
* @param[in] devNum                   - device number
*                                      entryID      - Entry ID of packet descriptor to return.
* @param[in] packetDesc               - Pointer to Packet descriptor new entry in the table
*
* @param[out] newEntryIndex            - The index of the new entry in the table.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note GalTis:
*       Table - cmdTxPcktDescs
*       Toolkit:
*       Using - <netSendPktByLport, netSendPktByIf, netSendPktByVid,
*       netFreeTxGtBuf, gtBufCreate>
*
*/
GT_STATUS cmdCpssPxTxSetPacketDesc
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_U32                      entryId,
    IN  PX_PKT_DESC_STC           * packetDesc,
    OUT GT_U32                    * newEntryIndex
)
{
    GT_U32  entryIndex;


    /* Prepare local variables and check whether valid packet */
    if(packetDesc->pcktDataLen[0] == 0)
    {
        return GT_FAIL;
    }

    for(entryIndex = 0; entryIndex < wrCpssPxNetIfTxPxTblCapacity; entryIndex++)
    {
        if (GT_FALSE == cpssPxTxPacketDescTbl[entryIndex].valid)
        {
            /* we found a free space */
            break;
        }

        if(cpssPxTxPacketDescTbl[entryIndex].devNum == devNum &&
           cpssPxTxPacketDescTbl[entryIndex].entryId == entryId)
        {
            return GT_ALREADY_EXIST;
        }
    }

    if(entryIndex == wrCpssPxNetIfTxPxTblCapacity)
    {
        /* Entry not found - creates new one */
        wrCpssPxNetIfTxPxTblCapacity++;

        /* allocate new TX packet descriptor*/
        if(cpssPxTxPacketDescTbl == NULL)
        {
            cpssPxTxPacketDescTbl = cmdOsMalloc(wrCpssPxNetIfTxPxTblCapacity * sizeof(PX_PKT_DESC_STC));
        }
        else
        {
            cpssPxTxPacketDescTbl = cmdOsRealloc(cpssPxTxPacketDescTbl,
                                        wrCpssPxNetIfTxPxTblCapacity * sizeof(PX_PKT_DESC_STC));
        }
        if(cpssPxTxPacketDescTbl == NULL)
        {
            wrCpssPxNetIfTxPxTblCapacity = 0;
            return GT_OUT_OF_CPU_MEM;
        }
        cmdOsMemSet(&cpssPxTxPacketDescTbl[entryIndex], 0, sizeof(PX_PKT_DESC_STC));
    }

    cmdOsMemCpy(&cpssPxTxPacketDescTbl[entryIndex], packetDesc, sizeof(PX_PKT_DESC_STC));

    cpssPxTxPacketDescTbl[entryIndex].valid = GT_TRUE;

    *newEntryIndex = entryIndex;

    return GT_OK;
}



/**
* @internal cmdCpssPxTxGetPacketDesc function
* @endinternal
*
* @brief   Returns specific packet descriptor entry from the table.
*
* @param[in,out] entryIndex               - Entry index in packet descriptor table.
*
* @param[out] packetDesc               - Packet descriptor from the table.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_EMPTY                 - on invalid index
*
* @note GalTis:
*       Table - cmdTxPacketDescs
*
*/
GT_STATUS cmdCpssPxTxGetPacketDesc
(
    INOUT GT_U32          *entryIndex,
    OUT PX_PKT_DESC_STC   *packetDesc
)
{
    GT_U8   i;

    /* check if the index is valid for the table */
    if(*entryIndex >= wrCpssPxNetIfTxPxTblCapacity)
        return GT_NOT_FOUND;


    for (; *entryIndex < wrCpssPxNetIfTxPxTblCapacity; (*entryIndex)++)
    {
        if (GT_TRUE == cpssPxTxPacketDescTbl[*entryIndex].valid)
        {
            /* Entry found */
            break;
        }
    }

    if (*entryIndex == wrCpssPxNetIfTxPxTblCapacity)
    {
        /* entry not found */
        return GT_NOT_FOUND;
    }

    /* copy packet descriptor */
    cmdOsMemSet(packetDesc, 0, sizeof(PX_PKT_DESC_STC));
    cmdOsMemCpy(packetDesc, &cpssPxTxPacketDescTbl[*entryIndex], sizeof(PX_PKT_DESC_STC));

    /* allocate memory for the pkt data */
    for(i = 0; i < MAX_NUM_OF_SDMA_BUFFERS_PER_CHAIN; i++)
    {
        if(cpssPxTxPacketDescTbl[*entryIndex].pcktDataLen[i] == 0)
            break; /*End of the chain */

        packetDesc->pcktData[i] =
            cmdOsMalloc(cpssPxTxPacketDescTbl[*entryIndex].pcktDataLen[i]);

        if(packetDesc->pcktData[i] == NULL)
        {
            return GT_OUT_OF_CPU_MEM;
        }

        /* copy pkt data */
        cmdOsMemCpy(packetDesc->pcktData[i],
                     cpssPxTxPacketDescTbl[*entryIndex].pcktData[i],
                     cpssPxTxPacketDescTbl[*entryIndex].pcktDataLen[i]);
    }

    return GT_OK;
}

/**
* @internal cmdCpssPxTxBeginGetPacketDesc function
* @endinternal
*
* @brief   Indicates for cmdCpssPxTxGetPacketDesc function that user go to get many
*         records in one session. This API make exact copy of table so that
*         user will get table without changes that can be made during get
*         operation. This mechanism is best suitable for Refresh table
*         operation.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note GalTis:
*       Table - cmdTxPacketDescs
*
*/
GT_STATUS cmdCpssPxTxBeginGetPacketDesc
(
)
{
    return GT_OK;
}

/**
* @internal cmdCpssPxTxEndGetPacketDesc function
* @endinternal
*
* @brief   Finish operation began by cmdCpssPxTxBeginGetPacketDesc API function
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note GalTis:
*       Table - cmdTxPacketDescs
*
*/
GT_STATUS cmdCpssPxTxEndGetPacketDesc
(
)
{
    return GT_OK;
}

/**
* @internal cmdCpssPxConvertPacketDesc function
* @endinternal
*
* @brief   Convert packet descriptor structure to CPSS_PX_NET_TX_PARAMS_STC
*
* @param[in] netTxPacketDescPtr       - pointer to TX packet descriptor structure
*
* @param[out] cpssNetTxPacketDescPtr   - pointer to CPSS TX packet descriptor structure
*                                       None
*/
GT_VOID cmdCpssPxConvertPacketDesc
(
    IN PX_PKT_DESC_STC                * netTxPacketDescPtr,
    OUT CPSS_PX_NET_TX_PARAMS_STC     * cpssNetTxPacketDescPtr
)
{
    cpssNetTxPacketDescPtr->recalcCrc = netTxPacketDescPtr->sdmaInfo.recalcCrc;
    cpssNetTxPacketDescPtr->txQueue = netTxPacketDescPtr->sdmaInfo.txQueue;

    return;
}

/**
* @internal cmdCpssPxTxDelPacketDesc function
* @endinternal
*
* @brief   Delete packet descriptor entry from the table.
*
* @param[in] devNum                   - device number
*                                      entryID     - Entry ID of packet descriptor to delete.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note After delete operation all record will be shifted
*       so the table will be not fragmented.
*       GalTis:
*       Table - cmdTxPcktDescs
*       Toolkit:
*
*/
GT_STATUS cmdCpssPxTxDelPacketDesc
(
    IN  GT_SW_DEV_NUM       devNum,
    IN  GT_U32              entryId
)
{
    GT_U32              j;
    PX_PKT_DESC_STC     *txPcktDescPtr;
    GT_U32              entryIndex;

    /* make sure there is no send */
    cmdCpssPxTxStop();

    if(cpssPxTxPacketDescTbl != NULL)
    {
        /* find the entry */
        for (entryIndex = 0; entryIndex < wrCpssPxNetIfTxPxTblCapacity; entryIndex++)
        {
            if (GT_FALSE == cpssPxTxPacketDescTbl[entryIndex].valid)
            {
                /* invalid entry */
                continue;
            }

            if(cpssPxTxPacketDescTbl[entryIndex].devNum == devNum &&
               cpssPxTxPacketDescTbl[entryIndex].entryId == entryId)
            {
                /* Entry found */
                break;
            }
        }

        if (entryIndex == wrCpssPxNetIfTxPxTblCapacity)
        {
            /* entry not found */
            return GT_NOT_FOUND;
        }

        txPcktDescPtr = &cpssPxTxPacketDescTbl[entryIndex];

        /* release Tx buffers */
        for( j=0; j < MAX_NUM_OF_SDMA_BUFFERS_PER_CHAIN ; j++ )
        {
            if (0 == txPcktDescPtr->pcktDataLen[j])
            {
                break;
            }

            if(txPcktDescPtr->pcktData[j] != NULL)
            {

                cmdOsPoolFreeBuf(txBuffersPoolId, txPcktDescPtr->pcktData[j]);
            }
        }/* For all buffers for this descriptor */

        /* clear the packet descriptor data */
        cmdOsMemSet(txPcktDescPtr, 0, sizeof(PX_PKT_DESC_STC));
    }

    return GT_OK;
}

/**
* @internal cmdCpssPxTxClearPacketDesc function
* @endinternal
*
* @brief   Clear all packet descriptors entries from the table.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note GalTis:
*       Table - cmdTxPacketDescs
*
*/
GT_STATUS cmdCpssPxTxClearPacketDesc
(
    void
)
{
    GT_U32 i,j;
    GT_U32 numberOfPacketsDeleted = 0;
    GT_U32 numberOfPacketsNotValid = 0;
    PX_PKT_DESC_STC   *txPcktDescPtr;

    /* make sure there is no send */
    cmdCpssPxTxStop();

    if(cpssPxTxPacketDescTbl != NULL)
    {
         /* release Tx buffers */
         for(i = 0; i < wrCpssPxNetIfTxPxTblCapacity; i++)
         {
             if (GT_TRUE != cpssPxTxPacketDescTbl[i].valid)
             {
                 numberOfPacketsNotValid++;
                 /* entry is not valid */
                 continue;
             }

             txPcktDescPtr = &cpssPxTxPacketDescTbl[i];

             for(j = 0; j < MAX_NUM_OF_SDMA_BUFFERS_PER_CHAIN; j++ )
             {
                 if (0 == txPcktDescPtr->pcktDataLen[j])
                 {
                     break;
                 }

                 if (txPcktDescPtr->pcktData[j] != NULL)
                 {
                     cmdOsPoolFreeBuf(txBuffersPoolId, txPcktDescPtr->pcktData[j]);
                 }

             }/*For all buffers for this descriptor*/

             /* clear the packet descriptor data */
             cmdOsMemSet(txPcktDescPtr, 0, sizeof(PX_PKT_DESC_STC));

             numberOfPacketsDeleted++;

         }/*For all descriptors*/

         if((numberOfPacketsDeleted + numberOfPacketsNotValid) == wrCpssPxNetIfTxPxTblCapacity)
         {
             /* free allocated table */
             cmdOsFree(cpssPxTxPacketDescTbl);
             cpssPxTxPacketDescTbl = NULL;
             wrCpssPxNetIfTxPxTblCapacity = 0;
         }
    }

    return GT_OK;
}

/**
* @internal cmdCpssPxTxStart function
* @endinternal
*
* @brief   Starts transmitting of packets
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note GalTis:
*       Command   - cmdCpssPxTxStart
*
*/
GT_STATUS cmdCpssPxTxStart
(
)
{
    GT_STATUS rc = GT_OK;
    static GT_BOOL   enterOnce = GT_FALSE;

    /* check if there are element in the transmit table */
    if(wrCpssPxNetIfTxPxTblCapacity == 0)
        return GT_OK;

    /* creat the task only once */
    if(enterOnce == GT_FALSE)
    {

        if(cmdOsSigSemBinCreate("txCmd",CPSS_OS_SEMB_EMPTY_E,&txCmdSemId) != GT_OK)
        {
            return GT_FAIL;
        }
        if(cmdOsTaskCreate("GalPktTx",                           /* Task Name                      */
                        GT_GALTIS_TRANSMITE_TASK_PRIOR_CNS,   /* Task Priority                  */
                        0x1000,                               /* Stack Size _4KB                */
                        (unsigned (__TASKCONV *)(void*))cpssPxPacketTransminitionTask, /* Starting Point */
                        (GT_VOID*)NULL,                       /* there is no arguments */
                        &taskId) != GT_OK)                    /* returned task ID */
            return GT_FAIL;
        enterOnce = GT_TRUE;
    }

    /* check if the last transmitting is done */
    if(taskBusyFlag == GT_TRUE)
        return GT_FAIL;

    flagStopTransmit = GT_FALSE;

    /* send a signal for the task to start the transmission */
    cmdOsSigSemSignal(txCmdSemId);

    return  rc;
}

/**
* @internal cmdCpssPxTxStop function
* @endinternal
*
* @brief   Stop transmitting of packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note GalTis:
*       Command   - cmdCpssPxTxStop
*
*/
GT_STATUS cmdCpssPxTxStop
(
    void
)
{
    GT_U32 timeOutCnt = 10;
    /* there is no send task running */
    if(taskId == 0)
    {
        return GT_NO_CHANGE;
    }

    flagStopTransmit = GT_TRUE;
    while (taskBusyFlag == GT_TRUE && timeOutCnt > 0)
    {
        cmdOsTimerWkAfter(50);
        timeOutCnt--;
    }
    if (taskBusyFlag == GT_TRUE)
    {
        return GT_FAIL;
    }
    return GT_OK;
}


/**
* @internal cmdCpssPxTxSetMode function
* @endinternal
*
* @brief   This command will set the the transmit parameters.
*
* @param[in] cyclesNum                - The maximum number of loop cycles (-1 = forever)
* @param[in] gap                      - The time to wit between two cycles
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note GalTis:
*       Command   - cmdTxSetMode
*       Interface  - <prestera/tapi/networkif/commands.api>
*
*/
GT_STATUS cmdCpssPxTxSetMode
(
    IN GT_U32   cyclesNum,
    IN GT_U32   gap
)
{
    modeSettings.cyclesNum = cyclesNum;
    modeSettings.gap = gap;

    return GT_OK;
}


/**
* @internal cmdCpssPxTxGetMode function
* @endinternal
*
* @brief   This command will get the the transmit parameters.
*
* @param[out] cyclesNum                - The maximum number of loop cycles (-1 = forever)
* @param[out] gap                      - The time to wit between two cycles.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note GalTis:
*       Command - cmdCpssPxTxGetMode
*
*/
GT_STATUS cmdCpssPxTxGetMode
(
    OUT GT_U32 *cyclesNum,
    OUT GT_U32 *gap
)
{
    *cyclesNum = modeSettings.cyclesNum;
    *gap = modeSettings.gap;

    return GT_OK;
}

/**
* @internal cmdCpssPxTxGenPacketDescSet function
* @endinternal
*
* @brief   Bind TX generator transmit parameters description entry to DB.
*
* @param[in] packetGenDescPtr         - pointer to TX Packet generator descriptor
*                                      packetGenDescTblEntryPtrPtr - pointer to pointer to TX packet generator descriptor table entry
* @param[in,out] packetGenDescTblSizePtr  - pointer to TX packet generator descriptor tale size
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_CPU_MEM        - on memory allocation fail
* @retval GT_ALREADY_EXIST         - on already exist entry
*/
GT_STATUS cmdCpssPxTxGenPacketDescSet
(
    IN  PX_PKT_GEN_DESC_STC         * packetGenDescPtr,
    INOUT PX_PKT_GEN_DESC_STC       ** packetGenDescTblEntryPtrPtr,
    INOUT GT_U32                    * packetGenDescTblSizePtr,
    OUT GT_U32                      * newEntryIndexPtr
)
{
    GT_U32  entryIndex;
    GT_U32  packetDescTblSize;
    PX_PKT_GEN_DESC_STC  *packetGenDescTblEntryPtr;

    packetDescTblSize = *packetGenDescTblSizePtr;
    packetGenDescTblEntryPtr = *packetGenDescTblEntryPtrPtr;

    for(entryIndex = 0; entryIndex < packetDescTblSize; entryIndex++)
    {
        /* Entry already exists in database */
        if(packetGenDescTblEntryPtr[entryIndex].packetId == packetGenDescPtr->packetId)
        {
            /* Update entry */
            break;
        }
    }

    if(entryIndex == packetDescTblSize)
    {
        /* Entry not found - creates new one */
        packetDescTblSize++;

        /* Allocate new TX generator packet descriptor */
        if(packetGenDescTblEntryPtr == NULL)
        {
            packetGenDescTblEntryPtr =
                cmdOsMalloc(packetDescTblSize * sizeof(PX_PKT_GEN_DESC_STC));
        }
        else
        {
            packetGenDescTblEntryPtr =
                cmdOsRealloc(packetGenDescTblEntryPtr,
                             packetDescTblSize * sizeof(PX_PKT_GEN_DESC_STC));
        }
        if(packetGenDescTblEntryPtr == NULL)
        {
            packetDescTblSize = 0;
            return GT_OUT_OF_CPU_MEM;
        }
        cmdOsMemSet(&packetGenDescTblEntryPtr[entryIndex], 0, sizeof(PX_PKT_GEN_DESC_STC));
    }

    /* Set table entry in database */
    cmdOsMemCpy(&packetGenDescTblEntryPtr[entryIndex], packetGenDescPtr,
                sizeof(PX_PKT_GEN_DESC_STC));

    *newEntryIndexPtr = entryIndex;
    *packetGenDescTblSizePtr = packetDescTblSize;
    *packetGenDescTblEntryPtrPtr = packetGenDescTblEntryPtr;

    return GT_OK;
}

/**
* @internal cmdCpssPxTxGenPacketDescDelete function
* @endinternal
*
* @brief   Delete TX generator transmit parameters description entry.
*
* @param[in] devNum                   - Device number.
* @param[in] txQueue                  - Tx queue.
* @param[in] packetId                 - Packet ID to delete.
*                                      packetGenDescTblEntryPtrPtr - pointer to pointer to TX packet generator table
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - on entry not found
* @retval GT_BAD_PTR               - on bad entry pointer
* @retval GT_EMPTY                 - on empty table
*/
GT_STATUS cmdCpssPxTxGenPacketDescDelete
(
    IN GT_SW_DEV_NUM                    devNum,
    IN GT_U8                            txQueue,
    IN GT_U32                           packetId,
    INOUT PX_PKT_GEN_DESC_STC           ** packetGenDescTblEntryPtrPtr,
    INOUT GT_U32                        * packetGenDescTblSizePtr
)
{
    PX_PKT_GEN_DESC_STC     *packetGenDescTblEntryPtr;
    PX_PKT_GEN_DESC_STC     *txPcktGenDescPtr;       /* Pointer to TX queue generator entry */
    GT_U32                  entryIndex;             /* Table entry index */
    GT_U32                  packetDescTblSize;

    packetDescTblSize = *packetGenDescTblSizePtr;
    packetGenDescTblEntryPtr = *packetGenDescTblEntryPtrPtr;

    if(packetGenDescTblEntryPtr == 0)
    {
        return GT_EMPTY;
    }

    if(packetGenDescTblSizePtr == 0)
    {
        return GT_BAD_PTR;
    }

    /* find the entry */
    for (entryIndex = 0; entryIndex < packetDescTblSize; entryIndex++)
    {
        if (packetGenDescTblEntryPtr[entryIndex].packetId == packetId)
        {
            /* Entry found */
            txPcktGenDescPtr = &packetGenDescTblEntryPtr[entryIndex];
            if(entryIndex < packetDescTblSize - 1)
            {
                /* Copy data to avoid memory fragmentation */
                cmdOsMemCpy(txPcktGenDescPtr, txPcktGenDescPtr + 1,
                           (packetDescTblSize - entryIndex - 1) * sizeof(PX_PKT_GEN_DESC_STC));
            }

            break;
        }
    }

    if (entryIndex == packetDescTblSize)
    {
        /* Entry not found */
        return GT_NOT_FOUND;
    }

    packetDescTblSize--;
    if(packetDescTblSize)
    {
        cmdOsRealloc(packetGenDescTblEntryPtr,
                     packetDescTblSize * sizeof(PX_PKT_GEN_DESC_STC));
    }

    *packetGenDescTblEntryPtrPtr = packetGenDescTblEntryPtr;
    *packetGenDescTblSizePtr = packetDescTblSize;

    return GT_OK;
}

/**
* @internal cmdCpssPxTxGenPacketDescClearAll function
* @endinternal
*
* @brief   Clear TX generator transmit parameters DB.
*
* @param[in] devNum                   - Device number.
* @param[in] txQueue                  - Tx queue.
*                                      packetGenDescTblEntryPtrPtr - pointer to pointer to TX packet generator table
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - on entry not found
* @retval GT_EMPTY                 - on empty table
*/
GT_STATUS cmdCpssPxTxGenPacketDescClearAll
(
    IN GT_SW_DEV_NUM                    devNum,
    IN GT_U8                            txQueue,
    INOUT PX_PKT_GEN_DESC_STC           ** packetGenDescTblEntryPtrPtr
)
{
    GT_STATUS               result;                     /* Return status */
    PX_PKT_GEN_DESC_STC     txPcktGenDesc;              /* Pointer to TX queue generator entry */
    GT_U32                  packetId;                   /* Packet ID */
    PX_PKT_GEN_DESC_STC     *packetGenDescTblEntryPtr;  /* Pointer to TX packet generator table */
    GT_U32                  packetGenDescTblSize;       /* Packet generator table size */

    packetGenDescTblEntryPtr = *packetGenDescTblEntryPtrPtr;

    if(packetGenDescTblEntryPtr == NULL)
    {
        return GT_EMPTY;
    }

    packetGenDescTblSize = wrCpssPxNetIfTxGenPxTblCapacity[txQueue];
    while(packetGenDescTblSize)
    {
        /* Get current entry */
        result = cmdCpssPxTxGenPacketDescGet(0,
                                               packetGenDescTblEntryPtr,
                                               packetGenDescTblSize,
                                               &txPcktGenDesc);
        if(result != GT_OK)
        {
            return result;
        }

        packetId = txPcktGenDesc.packetId;
        /* Delete TX packet generator packet */
        result = cmdCpssPxTxGenPacketDescDelete(devNum, txQueue, packetId,
                                                &packetGenDescTblEntryPtr,
                                                &wrCpssPxNetIfTxGenPxTblCapacity[txQueue]);
        if(result != GT_OK)
        {
            return result;
        }
    }

    /* Free allocated table */
    cmdOsFree(packetGenDescTblEntryPtr);
    *packetGenDescTblEntryPtrPtr = 0;

    return GT_OK;
}

/**
* @internal cmdCpssPxTxGenPacketDescGet function
* @endinternal
*
* @brief   Returns TX generator packet descriptor entry from the table.
*
* @param[in] entryIndex               - table entry index
* @param[in] packetGenDescTblEntryPtr
*                                      - pointer to TX packet generator table
* @param[in] packetGenDescTblSize
*                                      - TX packet generator table size
*
* @param[out] packetGenDescPtr         - pointer to entry in TX packet generator descriptor table.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on bad entry pointer
* @retval GT_OUT_OF_RANGE          - on invalid index
* @retval GT_EMPTY                 - on empty table
*/
GT_STATUS cmdCpssPxTxGenPacketDescGet
(
    IN  GT_U32                      entryIndex,
    IN  PX_PKT_GEN_DESC_STC       * packetGenDescTblEntryPtr,
    IN  GT_U32                      packetGenDescTblSize,
    OUT PX_PKT_GEN_DESC_STC       * packetGenDescPtr
)
{
    if(packetGenDescTblEntryPtr == 0)
    {
        return GT_EMPTY;
    }

    if(packetGenDescPtr == 0)
    {
        return GT_BAD_PTR;
    }

    /* Check if index is valid for the table */
    if(entryIndex >= packetGenDescTblSize)
    {
        return GT_OUT_OF_RANGE;
    }

    /* Copy packet descriptor */
    cmdOsMemCpy(packetGenDescPtr, &packetGenDescTblEntryPtr[entryIndex],
                sizeof(PX_PKT_GEN_DESC_STC));

    return GT_OK;
}


