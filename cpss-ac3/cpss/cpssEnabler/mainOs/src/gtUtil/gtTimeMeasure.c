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
* @file gtTimeMeasure.c
*
* @brief Tool for measuring exact times of part of code or
* function calls execution.
*
* @version   1
********************************************************************************
*/


/********* include ************************************************************/

#include <gtUtil/gtTimeMeasure.h>
#include <gtOs/gtOsIo.h>
#include <gtOs/gtOsStr.h>
#include <gtOs/gtOsTimer.h>

/* first node in print chain */
static GT_TIME_EXACT_NODE *firstPrintNodePtr = NULL;

/* previous node in print chain */
static GT_TIME_EXACT_NODE *prevPrintNodePtr = NULL;

/* previous processed node */
static GT_TIME_EXACT_NODE *prevProcessedNodePtr = NULL;

/**
* @internal gtTimeExactAdd function
* @endinternal
*
* @brief   Used to add exact times
*
* @param[in] time1Ptr                 - pointer to the time1 for summing
* @param[in] time2Ptr                 - pointer to the time2 for summing
*
* @param[out] sumPtr                   - pointer to the time sum
*                                       None.
*
* @note The sum can be the same than one (or both) of added times
*
*/
GT_VOID  gtTimeExactAdd
(
    IN      GT_TIME_EXACT *time1Ptr,
    IN      GT_TIME_EXACT *time2Ptr,
    OUT     GT_TIME_EXACT *sumPtr
)
{
    GT_TIME_EXACT tempTime;

    tempTime.seconds =
        time1Ptr->seconds + time2Ptr->seconds;
    tempTime.nanoSeconds =
        time1Ptr->nanoSeconds + time2Ptr->nanoSeconds;
    if (tempTime.nanoSeconds > 1000000000)
    {
        tempTime.seconds ++;
        tempTime.nanoSeconds -= 1000000000;
    }

    *sumPtr = tempTime;
}

/**
* @internal gtTimeExactSub function
* @endinternal
*
* @brief   Used to subtract exact times
*
* @param[in] time1Ptr                 - pointer to the time1 to subtract from
* @param[in] time2Ptr                 - pointer to the time2 to subtract it
*
* @param[out] subPtr                   - pointer to the time sum
*                                       None.
*
* @note The sum can be the same than one (or both) of subtracted times
*
*/
GT_VOID  gtTimeExactSub
(
    IN      GT_TIME_EXACT *time1Ptr,
    IN      GT_TIME_EXACT *time2Ptr,
    OUT     GT_TIME_EXACT *subPtr
)
{
    GT_TIME_EXACT tempTime;

    if (time1Ptr->seconds < time2Ptr->seconds)
    {
        /* wrong value for incorrect case */
        subPtr->seconds    = 0xFFFFFFFF;
        subPtr->nanoSeconds = 0xFFFFFFFF;
        return;
    }

    if ((time1Ptr->seconds == time2Ptr->seconds)
        && (time1Ptr->nanoSeconds < time2Ptr->nanoSeconds))
    {
        /* wrong value for incorrect case */
        subPtr->seconds    = 0xFFFFFFFF;
        subPtr->nanoSeconds = 0xFFFFFFFF;
        return;
    }

    tempTime.seconds =
        time1Ptr->seconds - time2Ptr->seconds;
    if (time1Ptr->nanoSeconds < time2Ptr->nanoSeconds)
    {
        tempTime.seconds --;
        tempTime.nanoSeconds =
            1000000000 + time1Ptr->nanoSeconds - time2Ptr->nanoSeconds;
    }
    else
    {
        tempTime.nanoSeconds =
            time1Ptr->nanoSeconds - time2Ptr->nanoSeconds;
    }

    *subPtr = tempTime;
}

/**
* @internal gtTimeExactProcessNode function
* @endinternal
*
* @brief   Used to process the node
*
* @param[in,out] nodePtr                  - pointer to processed node
* @param[in,out] nodePtr                  - pointer to processed node
*                                       None.
*/
GT_VOID  gtTimeExactProcessNode
(
    INOUT   GT_TIME_EXACT_NODE *nodePtr
)
{
    GT_TIME_EXACT tempTime;

    if (firstPrintNodePtr == NULL)
    {
        /* save the first node in the print chain */
        firstPrintNodePtr = nodePtr;
    }

    if (nodePtr->isInitialized == GT_FALSE)
    {
        /* add the new node to the print chain */
        if (prevPrintNodePtr != NULL)
        {
            prevPrintNodePtr->nextNodePtr = nodePtr;
        }
        prevPrintNodePtr = nodePtr;
        nodePtr->isInitialized = GT_TRUE;
    }

    /* save current time in the current node */
    osTimeRT(
        &(nodePtr->lastMeasure.seconds),
        &(nodePtr->lastMeasure.nanoSeconds));

    if (prevProcessedNodePtr != NULL)
    {
        /* add the time passed from the previous node */
        /* processing to the previous node sum        */
        gtTimeExactSub(
            &(nodePtr->lastMeasure),
            &(prevProcessedNodePtr->lastMeasure),
            &tempTime);
        gtTimeExactAdd(
            &(prevProcessedNodePtr->summaryTime),
            &tempTime,
            &(prevProcessedNodePtr->summaryTime));
        prevProcessedNodePtr->eventCount ++;
    }

    /* save the last processed node */
    prevProcessedNodePtr = nodePtr;
}

/**
* @internal gtTimeExactDumpNodes function
* @endinternal
*
* @brief   Used to dump all the nodes
*/
GT_VOID  gtTimeExactDumpNodes
(
    GT_VOID
)
{
    GT_TIME_EXACT_NODE *nodePtr;

    osPrintf("Time Exact Nodes Dump Start\n");
    for (nodePtr = firstPrintNodePtr;
          (nodePtr != NULL);
          nodePtr = nodePtr->nextNodePtr)
    {
        osPrintf(
            "%08d.%09d [%08d]--- %s\n",
            nodePtr->summaryTime.seconds,
            nodePtr->summaryTime.nanoSeconds,
            nodePtr->eventCount,
            nodePtr->nodeName);
    }
    osPrintf("Time Exact Nodes Dump End\n");
}

/**
* @internal gtTimeExactResetNodes function
* @endinternal
*
* @brief   Used to Reset all the nodes
*/
GT_VOID  gtTimeExactResetNodes
(
    GT_VOID
)
{
    GT_TIME_EXACT_NODE *nodePtr;
    GT_TIME_EXACT_NODE *nextNodePtr;

    for (nodePtr = firstPrintNodePtr;
          (nodePtr != NULL);
          nodePtr = nextNodePtr)
    {
        nextNodePtr = nodePtr->nextNodePtr;
        nodePtr->isInitialized = GT_FALSE;
        nodePtr->nextNodePtr = NULL;
        nodePtr->lastMeasure.seconds     = 0;
        nodePtr->lastMeasure.nanoSeconds = 0;
        nodePtr->summaryTime.seconds     = 0;
        nodePtr->summaryTime.nanoSeconds = 0;
        nodePtr->eventCount              = 0;
    }

    firstPrintNodePtr = NULL;
    prevPrintNodePtr = NULL;
    prevProcessedNodePtr = NULL;
}




