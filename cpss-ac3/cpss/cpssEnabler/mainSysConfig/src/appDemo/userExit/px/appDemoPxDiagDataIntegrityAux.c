/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
********************************************************************************
* @file appDemoPxDiagDataIntegrityAux.c
*
* @brief Auxilary functions for PX Diag Data Integrity
* @version   1
********************************************************************************
*/



/* Feature specific includes */
#include <cpss/common/cpssTypes.h>
#include <cpss/px/diag/cpssPxDiag.h>
#include <cpss/px/diag/cpssPxDiagDataIntegrity.h>
#include <appDemo/userExit/px/appDemoPxDiagDataIntegrityAux.h>



/**
* @enum TEST_PX_DIAG_DATA_INTEGRITY_EVENT_TYPE_FILTER_ENT
 *
 * @brief This enum defines error cause type display filter
*/
typedef enum{

    /** get all events. */
    TEST_PX_DIAG_DATA_INTEGRITY_EVENT_TYPE_FILTER_ALL_E,

    /** get single ECC events. */
    TEST_PX_DIAG_DATA_INTEGRITY_EVENT_TYPE_FILTER_SINGLE_ECC_E,

    /** get multiple ECC events. */
    TEST_PX_DIAG_DATA_INTEGRITY_EVENT_TYPE_FILTER_MULTIPLE_ECC_E,

    /** get parity events. */
    TEST_PX_DIAG_DATA_INTEGRITY_EVENT_TYPE_FILTER_PARITY_E

} TEST_PX_DIAG_DATA_INTEGRITY_EVENT_TYPE_FILTER_ENT;

static TEST_PX_DIAG_DATA_INTEGRITY_COUNTER_DB_STC *testDataIntegrityEventsDbPtr = NULL;
static TEST_PX_DIAG_DATA_INTEGRITY_MPPM_COUNTER_DB_STC *testDataIntegrityMppmEventsDbPtr = NULL;
static GT_BOOL isDataIntegrityInitDone = GT_FALSE;

/* debug flag to open trace of events */
GT_U32 cpssPxTraceEvents = 0;


/**
* @internal appDemoPxDataIntegrityTraceEnable function
* @endinternal
*
* @brief   Routine to enable trace
*
* @param[in] enable                   - enable/disable trace output
*                                       None
*/
void appDemoPxDataIntegrityTraceEnable
(
    IN GT_U32 enable
)
{
    cpssPxTraceEvents = enable;
    return;
}

/**
* @internal appDemoPxDiagDataIntegrityEventCounterIncrement
*           function
* @endinternal
*
* @brief   Function for increment counter per data integrity event
*
* @param[in] devNum                   - device number
* @param[in] eventPtr                 - (pointer to) data integrity event structure
*
* @retval GT_OK                   - on success.
* @retval GT_BAD_PARAM             - on wrong values of input parameters.
* @retval GT_OUT_OF_CPU_MEM        - on out of CPU memory
*
* @note Called from cpssEnabler.
*       First call is init DB, eventPtr = NULL
*
*/
static GT_STATUS appDemoPxDiagDataIntegrityEventCounterIncrement
(
    IN  GT_U8                                     devNum,
    IN  CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_STC     *eventPtr
)
{
    GT_U32 key = 0; /* key index to DB */
    CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType;

    if(devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        /* check device out of range */
        return GT_BAD_PARAM;
    }

    if(isDataIntegrityInitDone == GT_FALSE)
    {
        isDataIntegrityInitDone = GT_TRUE;
        /* first call - DB initialization only */
        testDataIntegrityEventsDbPtr = (TEST_PX_DIAG_DATA_INTEGRITY_COUNTER_DB_STC*)cpssOsMalloc(sizeof(TEST_PX_DIAG_DATA_INTEGRITY_COUNTER_DB_STC));
        testDataIntegrityMppmEventsDbPtr = (TEST_PX_DIAG_DATA_INTEGRITY_MPPM_COUNTER_DB_STC*)cpssOsMalloc(sizeof(TEST_PX_DIAG_DATA_INTEGRITY_MPPM_COUNTER_DB_STC));

        if((testDataIntegrityEventsDbPtr == NULL) || (testDataIntegrityMppmEventsDbPtr == NULL))
        {
            return GT_OUT_OF_CPU_MEM;
        }
        else
        {
            cpssOsMemSet(testDataIntegrityEventsDbPtr, 0, sizeof(TEST_PX_DIAG_DATA_INTEGRITY_COUNTER_DB_STC));
            cpssOsMemSet(testDataIntegrityMppmEventsDbPtr, 0, sizeof(TEST_PX_DIAG_DATA_INTEGRITY_MPPM_COUNTER_DB_STC));
            return GT_OK;
        }
    }
    else
    {
        CPSS_NULL_PTR_CHECK_MAC(eventPtr);

        memType = eventPtr->location.ramEntryInfo.memType;
        if((memType == CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_BANK_RAM_E) ||
           (memType == CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_RX_IDDB_MEM_E) ||
           (memType == CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_TXDMA_RD_BURST_FIFO_E))
        {
            if((eventPtr->location.mppmMemLocation.mppmId > 1) ||
               (eventPtr->location.mppmMemLocation.bankId > 1) ||
               (GT_FALSE == eventPtr->location.isMppmInfoValid))
            {
                return GT_BAD_PARAM;
            }

            key = eventPtr->location.mppmMemLocation.mppmId;
            key |= (eventPtr->location.mppmMemLocation.bankId << 1);
            if(eventPtr->eventsType == CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E)
            {
                key |= (0 << 2);
            }
            else if(eventPtr->eventsType == CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E)
            {
                key |= (1 << 2);
            }
            else
            {
                return GT_BAD_PARAM;
            }

            if(key >= TEST_PX_DIAG_DATA_INTEGRITY_MPPM_DB_SIZE_CNS)
            {
                /* need to update size of DB */
                return GT_FAIL;
            }

            testDataIntegrityMppmEventsDbPtr->eventCounterArr[key] += 1;

            return GT_OK;
        }
        else
        {
            /* calculate index for DB access */
            key = eventPtr->location.ramEntryInfo.memLocation.dfxMemoryId;
            key |= (eventPtr->location.ramEntryInfo.memLocation.dfxClientId << 7);
            key |= (eventPtr->location.ramEntryInfo.memLocation.dfxPipeId << 12);
        }

        switch(eventPtr->eventsType)
        {
            case CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E:
                U32_SET_FIELD_MAC(key, 15, 2, 1);
                break;
            case CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E:
                U32_SET_FIELD_MAC(key, 15, 2, 2);
                break;
            case CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E:
                U32_SET_FIELD_MAC(key, 15, 2, 3);
                break;
            default:
                return GT_BAD_PARAM;
        }

        if (cpssPxTraceEvents)
        {
            cpssOsPrintf("eventPtr->memLocation.dfxPipeId = 0x%x\r\n", U32_GET_FIELD_MAC(key, 12 , 3));
            cpssOsPrintf("eventPtr->memLocation.dfxClientId = 0x%x\r\n", U32_GET_FIELD_MAC(key, 7 , 5));
            cpssOsPrintf("eventPtr->memLocation.dfxMemoryId = %d\r\n", U32_GET_FIELD_MAC(key, 0 , 7));
            cpssOsPrintf("eventPtr->eventsType = 0x%x\r\n", eventPtr->eventsType);
            cpssOsPrintf("eventPtr->memType = 0x%x\r\n", memType);
            cpssOsPrintf("key = 0x%x\r\n", key);
        }

        if(key >= BIT_17)
        {
            /* need to update size of DB */
            return GT_FAIL;
        }

        testDataIntegrityEventsDbPtr->eventCounterArr[key] += 1;
        testDataIntegrityEventsDbPtr->memTypeArr[key] = memType;
     }

    return GT_OK;
}


/**
* @internal appDemoPxDiagDataIntegrityCountersCzllBackGet
*           function
* @endinternal
*
* @brief   Routine to bind a CB function that receives block of
*         data integrity event counters for given device.
* @param[in] dataIntegrityEventCounterBlockGetCB - callback function
*
* @retval GT_OK                   - on success.
*/
GT_STATUS appDemoPxDiagDataIntegrityCountersCallBackGet
(
    PX_DATA_INTEGRITY_EVENT_COUNTER_INCREMENT_FUNC **dataIntegrityEventCounterBlockGetCB
)
{
    *dataIntegrityEventCounterBlockGetCB = appDemoPxDiagDataIntegrityEventCounterIncrement;
    return GT_OK;
}


TEST_PX_DIAG_DATA_INTEGRITY_COUNTER_DB_STC * appDemoPxDiagDataIntegrityCountersDbGet
(
    GT_VOID
)
{
    return testDataIntegrityEventsDbPtr;
}


/**
* @internal appDemoPxDiagDataIntegrityEventTableClear function
* @endinternal
*
* @brief   clear all entries in event counter entry block.
*
* @retval GT_OK
* @note none
*
*/
GT_VOID appDemoPxDiagDataIntegrityEventTableClear
(
    GT_VOID
)
{
    cpssOsMemSet(testDataIntegrityEventsDbPtr, 0, sizeof(TEST_PX_DIAG_DATA_INTEGRITY_COUNTER_DB_STC));
}


/**
* @internal appDemoPxDiagDataIntegrityMppmEventTableClear
*           function
* @endinternal
*
* @brief   clear all entries in event counter entry block.
*
*
*/
GT_VOID cpssPxDiagDataIntegrityMppmEventTableClear
(
)
{
    cpssOsMemSet(testDataIntegrityMppmEventsDbPtr, 0, sizeof(TEST_PX_DIAG_DATA_INTEGRITY_MPPM_COUNTER_DB_STC));


}




