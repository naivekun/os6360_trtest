/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* tgfCommonEventUT.c
*
* DESCRIPTION:
*       Enhanced UTs for CPSS Px events
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/

#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>
#include <trafficEngine/tgfTrafficTable.h>
#include <cpss/generic/events/private/prvCpssGenEvReq.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvExMxEventsPxPipe.h>
#include <appDemo/userExit/userEventHandler.h>

#include <event/prvTgfEventDeviceGenerate.h>

/* Macros that fills evExtDataArray with
 * linear sequence starting with 0*/
#define PRV_TGF_FILL_EXT_DATA_ARRAY(size) \
        for (i = 0; i < size; i++) \
            evExtDataArray[i] = i; \
        *evExtDataSize = size;

/* Macros skips events which are contained in array */
#define PRV_TGF_SKIP_EVENTS(array, arraySize)                               \
        for (eventArrayItr = 0; eventArrayItr < arraySize; eventArrayItr++) \
        {                                                                   \
          if (eventItr == (GT_U32)array[eventArrayItr])                     \
          {                                                                 \
              skipEvent = GT_TRUE;                                          \
              break;                                                        \
          }                                                                 \
        }                                                                   \
        if (skipEvent == GT_TRUE)                                           \
            continue;

/**
* @internal prvTgfPrintFailedEvents function
* @endinternal
*
* @brief   Print array of failed events.
*
* @param[in] eventArray               - array of PRV_PRINT_EVENT_CTX
*                                      structures for failed events
* @param[in] size                     -  of eventArray
*                                      eventNames - array of string names of unified events
*                                       None
*/
static GT_VOID prvTgfPrintFailedEvents
(
    PRV_PRINT_EVENT_CTX* eventArray, /* Array of failed events */
    GT_U32 size,                     /* Size of array of failed events*/
    char** eventNames,               /* Array of string names of
                                        unified events */
    GT_BOOL overflow                 /* Flag that indicates overflowing
                                        of array of failed events */
)
{
    GT_U32 i;
    char *eventStr; /*string value of unified event*/
    PRV_UTF_LOG0_MAC("\n\n");
    if (size > 0)
    {
        PRV_UTF_LOG0_MAC("******************** Failed Events ********************\n");
        PRV_UTF_LOG0_MAC("\n");
        for (i = 0; i < size; i++)
        {
            eventStr = eventNames[(GT_U32)eventArray[i].event];
            PRV_UTF_LOG4_MAC("Event Name: %s; Returned Code: %d; Counter Value: %d; evData: %d\n",
                    eventStr, eventArray[i].rc, eventArray[i].counter, eventArray[i].evExtData);
        }
        if (overflow == GT_TRUE)
        {
            /* Print dots if array of failed events has been overflowed*/
            PRV_UTF_LOG0_MAC("...\n");
            PRV_UTF_LOG0_MAC("...\n");
            PRV_UTF_LOG0_MAC("...\n");
            PRV_UTF_LOG0_MAC("\n");
        }
    }
    else
        PRV_UTF_LOG0_MAC("All events have been generated successfully\n");
}

/**
* @internal prvTgfCheckEventCounters function
* @endinternal
*
* @brief   This function checks event counter with
*         the expected value
* @param[in] eventCounter             - event counter
* @param[in] evExpectedCouter         - expected event counter
*
* @retval GT_TRUE                  - the event counter matches the expected value
* @retval GT_FALSE                 - the event counter doesn't match the expected value
*/
static GT_BOOL prvTgfCheckEventCounters
(
    GT_U32 eventCounter,            /* current value of event counter */
    GT_U32 evExpectedCouter         /* expected value of event counter */
)
{
    if (eventCounter == evExpectedCouter)
        return GT_TRUE;
    else
        return GT_FALSE;
}

/**
* @internal prvTgfPipeInitEvExtDataArray function
* @endinternal
*
* @brief   This function is used to fill evExtData
*         array for unified event.
* @param[in] event                    -  unified event
* @param[in] event                    -  unified event
*                                      structures for failed events
*                                      size       - size of eventArray
* @param[in] event                    - array of string names of unified events
*
* @param[out] evExtDataArray           - array of evExtData values
* @param[out] evExtDataSize            - size of evExtDataArray
* @param[out] expectedCounter          - (pointer to) expected counter for unified event
*                                       None
*/
static GT_VOID prvTgfPipeInitEvExtDataArray
(
    IN  CPSS_UNI_EV_CAUSE_ENT event, /* unified event */
    OUT GT_U32* evExtDataArray,      /* array of evExtData values */
    OUT GT_U32* evExtDataSize,       /* size of evExtDataArray */
    OUT GT_U32* expectedCounter      /* expected counter for unified event */
)
{
    int i = 0;
    *expectedCounter = 1;

    switch(event)
    {
        case CPSS_PP_GPP_E:
            PRV_TGF_FILL_EXT_DATA_ARRAY(8);
            break;
        case CPSS_PP_TQ_MISC_E:
            PRV_TGF_FILL_EXT_DATA_ARRAY(5);
            break;
        case CPSS_PP_DATA_INTEGRITY_ERROR_E:
            evExtDataArray[ 0] = (GT_U32)PRV_CPSS_PIPE_TXDMA_ECC_SINGLE_ERROR_INT_E;
            evExtDataArray[ 1] = (GT_U32)PRV_CPSS_PIPE_TXDMA_ECC_DOUBLE_ERROR_INT_E;
            evExtDataArray[ 2] = (GT_U32)PRV_CPSS_PIPE_TXQ_TD_CLR_ECC_ONE_ERROR_CORRECTED_INT_E;
            evExtDataArray[ 3] = (GT_U32)PRV_CPSS_PIPE_TXQ_TD_CLR_ECC_TWO_ERROR_DETECTED_INT_E;
            evExtDataArray[ 4] = (GT_U32)PRV_CPSS_PIPE_TXQ_QCN_DESC_ECC_SINGLE_ERROR_DETECTED_INT_E;
            evExtDataArray[ 5] = (GT_U32)PRV_CPSS_PIPE_TXQ_QCN_DESC_ECC_DOUBLE_ERROR_DETECTED_INT_E;
            evExtDataArray[ 6] = (GT_U32)PRV_CPSS_PIPE_MPPM_INTERRUPT_ECC_ERROR_CAUSE_SUM_E;
            evExtDataArray[ 7] = (GT_U32)PRV_CPSS_PIPE_MPPM_ECC_INT_ECC_DOUBLE_ERROR_E;
            evExtDataArray[ 8] = (GT_U32)PRV_CPSS_PIPE_MPPM_ECC_INT_ECC_SINGLE_ERROR_E;
            evExtDataArray[ 9] = (GT_U32)PRV_CPSS_PIPE_TXQ_PFC_PORT_GROUP0_COUNTERS_PARITY_ERR_E;
            evExtDataArray[10] = (GT_U32)PRV_CPSS_PIPE_TXQ_QCN_BUFFER_FIFO_PARITY_ERR_INT_E;
            evExtDataArray[11] = (GT_U32)PRV_CPSS_PIPE_BM_CORE_0_VALID_TABLE_PARITY_ERROR_INTERRUPT_E;
            /* Not supported yet */
            *evExtDataSize = 0;
            break;
        case CPSS_PP_PIPE_PCP_E:
            evExtDataArray[0] = (GT_U32)PRV_CPSS_PIPE_PCP_CPU_ADDRESS_OUT_OF_RANGE_E;
            evExtDataArray[1] = (GT_U32)PRV_CPSS_PIPE_PCP_PACKET_TYPE_KEY_LOOK_UP_MISS_E;
            *evExtDataSize = 2;
            break;
        case CPSS_PP_PHA_E:
            evExtDataArray[0] = (GT_U32)PRV_CPSS_PIPE_PHA_UNMAPPED_HOST_ACCESS_E;
            evExtDataArray[1] = (GT_U32)PRV_CPSS_PIPE_HEADER_SINGLE_ERROR_E;
            evExtDataArray[2] = (GT_U32)PRV_CPSS_PIPE_HEADER_DOUBLE_ERROR_E;
            evExtDataArray[3] = (GT_U32)PRV_CPSS_PIPE_PPA_UNMAPPED_HOST_ACCESS_E;
            evExtDataArray[4] = (GT_U32)PRV_CPSS_PIPE_PPG_0_UNMAPPED_HOST_ACCESS_ERROR_E;
            evExtDataArray[5] = (GT_U32)PRV_CPSS_PIPE_PPG_1_UNMAPPED_HOST_ACCESS_ERROR_E;
            evExtDataArray[6] = (GT_U32)PRV_CPSS_PIPE_PPG_2_UNMAPPED_HOST_ACCESS_ERROR_E;
            evExtDataArray[7] = (GT_U32)PRV_CPSS_PIPE_PPG_3_UNMAPPED_HOST_ACCESS_ERROR_E;
            evExtDataArray[8] = (GT_U32)PRV_CPSS_PIPE_PPG_0_PPN_0_HOST_UNMAPPED_ACCESS_E;
            evExtDataArray[9] = (GT_U32)PRV_CPSS_PIPE_PPG_1_PPN_3_CORE_UNMAPPED_ACCESS_E;
            evExtDataArray[10] = (GT_U32)PRV_CPSS_PIPE_PPG_2_PPN_4_NEAR_EDGE_IMEM_ACCESS_E;
            evExtDataArray[11] = (GT_U32)PRV_CPSS_PIPE_PPG_3_PPN_7_DOORBELL_INTERRUPT_E;
            *evExtDataSize = 12;
            break;
        default:
            evExtDataArray[0] = CPSS_PARAM_NOT_USED_CNS;
            *evExtDataSize = 1;
    }
}

/**
* @internal prvTgfPxGenEvent function
* @endinternal
*
* @brief   The body of prvTgfEventDeviceGenerate test
*/
GT_VOID prvTgfPxGenEvent(GT_VOID)
{
    GT_STATUS rc, rc1; /*CPSS returned codes */
    GT_U32 counter; /* counter for generated events */
    CPSS_UNI_EV_CAUSE_ENT event;
    GT_U32 eventItr; /* event iterator */
    PRV_PRINT_EVENT_CTX failedEvents[PRV_TGF_FAILED_EVENTS_ARRAY_SIZE];

    /*Array of failed events. Used for debugging*/

    GT_BOOL overflow = GT_FALSE; /* Flag that indicates overflowing
                                  of array of failed events */
    GT_U32 failedEventsCnt = 0; /* counter of failed events */

    /* Array of disabled by default events */
    CPSS_UNI_EV_CAUSE_ENT unsupportedEvents[] = PRV_TGF_UNSUPPORTED_EVENTS;
    /* Array of unsupported events */
    GT_U32  disabledEventsSize;      /* Size of disabledEvents array */
    GT_U32  unsupportedEventsSize;   /* Size of unsupportedEvents array */
    GT_U32  eventArrayItr;           /* Iterator for disabledEvents and unsupportedEvents arrays */
    GT_BOOL skipEvent;              /* Flag indicates that the event was found in disabledEvents
                                    or unsupportedEvents array */
    GT_U32 evExtDataItr;            /* Iterator for evExtDataArray */
    GT_U32 evExtDataArray[256];     /* array of evExtData values */
    GT_U32 evExtDataSize;
    GT_U32 evExpectedCouter;

    CPSS_UNI_EV_CAUSE_ENT disabledEvents[] = PRV_TGF_DISABLED_BY_DEFAULT_EVENTS;
                                 /* Array of disabled events */

    char * uniEvName[CPSS_UNI_EVENT_COUNT_E] = {UNI_EV_NAME};
    /* Array that contains string names of unified events */

    disabledEventsSize=sizeof(disabledEvents)/sizeof(disabledEvents[0]);
    unsupportedEventsSize=sizeof(unsupportedEvents)/sizeof(unsupportedEvents[0]);

    /* call the CPSS to enable those interrupts in the HW of the device */
    rc = appDemoEventsToTestsHandlerBind(disabledEvents, disabledEventsSize,
                                         CPSS_EVENT_UNMASK_E);
    if (GT_OK != rc)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoEventsToTestsHandlerBind");
    }

    /* Do the generation of all supported events */
    for (eventItr = (GT_U32)CPSS_PP_UNI_EV_MIN_E;
            eventItr < (GT_U32)CPSS_PP_UNI_EV_MAX_E;
            eventItr++)
    {
        /* Initialize skipEvent flag */
        skipEvent = GT_FALSE;
        /* Skip unsupported events */
        PRV_TGF_SKIP_EVENTS(unsupportedEvents, unsupportedEventsSize);

        event = (CPSS_UNI_EV_CAUSE_ENT)eventItr;

        /* get array of extData for current event */
        prvTgfPipeInitEvExtDataArray(event, evExtDataArray, &evExtDataSize, &evExpectedCouter);

        for (evExtDataItr = 0; evExtDataItr < evExtDataSize; evExtDataItr++)
        {
            /* clean events counter */
            rc = utfGenEventCounterGet(prvTgfDevNum,event,GT_TRUE,NULL);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoGenEventCounterGet");

            /* generate event */
            rc = cpssEventDeviceGenerate(prvTgfDevNum,event,evExtDataArray[evExtDataItr]);
            rc1 = rc;
            /* Exclude failing test on GT_NOT_SUPPORTED and GT_NOT_FOUND codes */
            if (rc !=  GT_NOT_SUPPORTED && rc != GT_NOT_FOUND)
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssEventDeviceGenerate");

            cpssOsTimerWkAfter(10);

            /* get events counter */
            rc=utfGenEventCounterGet(prvTgfDevNum,event,GT_FALSE,&counter);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoGenEventCounterGet");

            /* cpssOsPrintf("Event: %s, counter: %d\n", uniEvName[eventItr], counter);*/

            if (prvTgfCheckEventCounters(counter, evExpectedCouter) == GT_FALSE &&
                    rc1 != GT_NOT_SUPPORTED &&
                    rc1 != GT_NOT_FOUND)
            {
                /*Set Fail Status if event counter is 0*/
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE,GT_FALSE,
                        "\nFailed to generate event %s[%d]\n",
                        uniEvName[eventItr], evExtDataItr);
                /* Filling in PRV_PRINT_EVENT_CTX structure*/
                if (failedEventsCnt < PRV_TGF_FAILED_EVENTS_ARRAY_SIZE)
                {
                    failedEvents[failedEventsCnt].event = eventItr;
                    failedEvents[failedEventsCnt].counter = counter;
                    failedEvents[failedEventsCnt].rc = rc1;
                    failedEvents[failedEventsCnt].evExtData = evExtDataArray[evExtDataItr];
                    failedEventsCnt++;
                }
                else
                    overflow = GT_TRUE;
            }
        }
    }

    /* Mask events disabled by default */
    rc = appDemoEventsToTestsHandlerBind(disabledEvents, disabledEventsSize, CPSS_EVENT_MASK_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoEventsToTestsHandlerBind");

    /* Print all failed events */
    prvTgfPrintFailedEvents(failedEvents, failedEventsCnt, uniEvName, overflow);
    return;
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: prvTgfPxGenEventDeviceGenerate:
    Description:
    The purpose of this test is to check the functionality
    of event generation from CPSS API.

*/
UTF_TEST_CASE_MAC(prvTgfPxGenEventDeviceGenerate)
{
    /* Test */
    prvTgfPxGenEvent();
}

/*
 * Configuration of tgfEvent suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssPxTgfEvent)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxGenEventDeviceGenerate)

UTF_SUIT_END_TESTS_MAC(cpssPxTgfEvent)


