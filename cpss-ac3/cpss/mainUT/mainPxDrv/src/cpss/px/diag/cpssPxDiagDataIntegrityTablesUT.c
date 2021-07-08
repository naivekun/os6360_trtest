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
* @file cpssPxDiagDataIntegrityTablesUT.c
*
* @brief Unit tests for cpssPxDiagDataIntegrityTables
* CPSS PX Diagnostic Tables API
*
* @version   1
********************************************************************************
*/
#include <cpss/px/diag/cpssPxDiagDataIntegrityTables.h>
#include <cpss/px/diag/private/prvCpssPxDiagDataIntegrityMainMappingDb.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <appDemo/userExit/px/appDemoPxDiagDataIntegrityAux.h>


/* max number of words in entry */
#define MAX_ENTRY_SIZE_CNS   64

/* debug flag to open trace of events */
static GT_U32 traceEvents = 0;

GT_U32 pxTimeOut = 10;
GT_U32 entryValue[4];

extern TEST_PX_DIAG_DATA_INTEGRITY_COUNTER_DB_STC * appDemoPxDiagDataIntegrityCountersDbGet(GT_VOID);
#define PRV_CPSS_DATA_INTEGRITY_ENUM_STRING_GET_MAC(tableType, tableEnum, strBuffer)



static CPSS_PX_TABLE_ENT selectedTableType = 0xFFFFFFFF;
static GT_U32 selectedNumOfEntries = 0xFFFFFFFF;


GT_STATUS prvCpssPxDiagDataIntegrityHwTableCheck
(
    IN GT_U8   devNum,
    IN CPSS_PX_TABLE_ENT tableType,
    OUT CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      *memTypePtr
);

GT_STATUS prvCpssPxPortGroupWriteTableEntry
(
    IN GT_SW_DEV_NUM     devNum,
    IN GT_U32                   portGroupId,
    IN CPSS_PX_TABLE_ENT  tableType,
    IN GT_U32                   entryIndex,
    IN GT_U32                  *entryValuePtr
);

GT_STATUS prvFwImageTableEntryWrite
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  GT_U32                                  hwTableEntryIndex
);

GT_STATUS prvFwImageTableEntryRead
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  GT_U32                                  entryIndex,
    IN  GT_U32                                  ppg,
    OUT GT_U32                                  *readValue
);

GT_VOID prvPxDebugTableTypeSet(CPSS_PX_TABLE_ENT tableType, GT_U32 numEntries, GT_U32 timeOutVal)
{
    PRV_CPSS_DATA_INTEGRITY_ENUM_STRING_GET_MAC(tableType, CPSS_PX_TABLE_ENT, strNameBuffer);
    selectedTableType = tableType;
    selectedNumOfEntries = numEntries;
    pxTimeOut = timeOutVal;
    cpssOsPrintf("tableType %d (%d entries %d mSec time out)\r\n", tableType, selectedNumOfEntries, pxTimeOut);
    return;
}

GT_STATUS prvCpssPxReadTableEntry_fromShadow
(
    IN GT_SW_DEV_NUM           devNum,
    IN CPSS_PX_TABLE_ENT       tableType,
    IN GT_U32                  entryIndex,
    OUT GT_U32                 *numBitsPerEntryPtr,
    OUT GT_U32                 *entryValuePtr
);
GT_STATUS prvCpssPxDiagDataIntegrityShadowAndHwSynch
(
    IN GT_SW_DEV_NUM                               devNum
);


#ifndef ASIC_SIMULATION
/* maximal number of events callback function can store */
#define PRV_MAX_CALLBACK_EVENTS_COUNT_CNS 40


static GT_U32 tableEntry[MAX_ENTRY_SIZE_CNS] = {0xFF};
typedef GT_STATUS PX_DATA_INTEGRITY_EVENT_CB_FUNC
(
    IN  GT_U8                                       devNum,
    IN  CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_STC     *eventPtr
);

extern PX_DATA_INTEGRITY_EVENT_CB_FUNC    *pxDataIntegrityEventIncrementFunc;


typedef enum {
    PRV_DATA_INTEGRITY_EVENT_MATCH_TYPE_ALL_E,
    PRV_DATA_INTEGRITY_EVENT_MATCH_TYPE_ANY_E,
    PRV_DATA_INTEGRITY_EVENT_MATCH_TYPE_THE_ONLY_E
} PRV_DATA_INTEGRITY_EVENT_MATCH_TYPE_ENT;


/* Data structure holds HW and RAM info from event */
typedef struct
{
    CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   eventsType;
    CPSS_PX_RAM_INDEX_INFO_STC                      eventRamEntryInfo;
    CPSS_PX_HW_INDEX_INFO_STC                       eventHwEntryInfo;
    CPSS_PX_LOGICAL_TABLE_INFO_STC                  eventLogicalEntryInfo;
} PRV_PX_DATA_INTEGRITY_EVENT_INFO_STC;



typedef struct
{
    GT_U32 eventsCount;
    PRV_PX_DATA_INTEGRITY_EVENT_INFO_STC eventsArr[PRV_MAX_CALLBACK_EVENTS_COUNT_CNS];
} PX_DATA_INTEGRITY_EVENTS_STC;

PX_DATA_INTEGRITY_EVENTS_STC pxOccuredEvents = {0}; /* .eventsCount==0 */


/* the struct to store data integrity event specific info.
   Filled by event handler callback function */
typedef struct
{
    GT_U32 allEventsCount;      /* counter of all events */
    GT_U32 tsEventsCount;       /* counter of events related to Tunnel Start/ARP/NAT HW table */
    CPSS_PX_LOGICAL_TABLE_INFO_STC                 eventsInfo[PRV_MAX_CALLBACK_EVENTS_COUNT_CNS];
    CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   eventsType[PRV_MAX_CALLBACK_EVENTS_COUNT_CNS];
} PRV_PX_DATA_INTEGRITY_TS_EVENTS_STC;

PRV_PX_DATA_INTEGRITY_TS_EVENTS_STC tunnelStartEvents;
GT_U32 prvDataIntegrityFwImageTestAddressesList[] = {0x730, 0x1030, 0x1600, 0x1ef0};

/**
* @internal hwInfoEventErrorHandlerFunc function
* @endinternal
*
* @brief   Callback called in case of data integrity error.
*         Store HW related info from event to global structure.
* @param[in] dev                      - the device.
* @param[in] eventPtr                 - filled structure which describes data integrity error event
*/
static GT_STATUS hwInfoEventErrorHandlerFunc
(
    IN GT_U8                                     dev,
    IN CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_STC  *eventPtr

)
{

    PRV_PX_DATA_INTEGRITY_EVENT_INFO_STC *curEventPtr;
    CPSS_NULL_PTR_CHECK_MAC(eventPtr);
    if(dev >= PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        /* check device out of range */
        return GT_BAD_PARAM;
    }

    if (pxOccuredEvents.eventsCount >=  PRV_MAX_CALLBACK_EVENTS_COUNT_CNS)
    {
        PRV_UTF_LOG0_MAC("Maximum number of Data Integrity events is exceeded!\n");
        return GT_FAIL;
    }

    if (traceEvents > 1)
    {
        PRV_UTF_LOG0_MAC("======DATA INTEGRITY ERROR EVENT======\n");
        PRV_UTF_LOG1_MAC("Event type        :  %d\n",   eventPtr->eventsType);
        PRV_UTF_LOG1_MAC("HW entry index    :  %d\n",   eventPtr->location.hwEntryInfo.hwTableEntryIndex);
        PRV_UTF_LOG1_MAC("RAM entry index   :  %d\n",   eventPtr->location.ramEntryInfo.ramRow);
        PRV_UTF_LOG1_MAC("HW table type     :  %s\n",   eventPtr->location.hwEntryInfo.hwTableType);
        PRV_UTF_LOG1_MAC("Memory table type :  %s\n",   eventPtr->location.ramEntryInfo.memType);
        PRV_UTF_LOG3_MAC("                     pipe %d, client %d, mem %d\n",
                         eventPtr->location.ramEntryInfo.memLocation.dfxPipeId,
                         eventPtr->location.ramEntryInfo.memLocation.dfxClientId,
                         eventPtr->location.ramEntryInfo.memLocation.dfxMemoryId);
    }


    curEventPtr = &pxOccuredEvents.eventsArr[pxOccuredEvents.eventsCount++];

    curEventPtr->eventsType = eventPtr->eventsType;
    curEventPtr->eventLogicalEntryInfo = eventPtr->location.logicalEntryInfo;
    curEventPtr->eventHwEntryInfo = eventPtr->location.hwEntryInfo;
    curEventPtr->eventRamEntryInfo = eventPtr->location.ramEntryInfo;

    return GT_OK;
}


GT_VOID pxPrintEventHwInfo
(
    IN PRV_PX_DATA_INTEGRITY_EVENT_INFO_STC *eventPtr
)
{
    cpssOsPrintf("eventsType              %d\n", eventPtr->eventsType);
    cpssOsPrintf("hwTableType             %d\n", eventPtr->eventHwEntryInfo.hwTableType);
    cpssOsPrintf("memType                 %d\n", eventPtr->eventRamEntryInfo.memType);
    cpssOsPrintf("                        pipe %d, client %d, mem %d\n",
                 eventPtr->eventRamEntryInfo.memLocation.dfxPipeId,
                 eventPtr->eventRamEntryInfo.memLocation.dfxClientId,
                 eventPtr->eventRamEntryInfo.memLocation.dfxMemoryId);

    cpssOsPrintf("hwTableEntryIndex       %d\n", eventPtr->eventHwEntryInfo.hwTableEntryIndex);
}

static GT_STATUS prvCpssPxDiagDataIntegrityEventHwInfoDbCheck
(
    IN CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT eventTypeExp,
    IN CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memTypeExp,
    IN CPSS_PX_TABLE_ENT                             tableTypeExp,
    IN GT_U32                                        entryIndexExp,
    IN PRV_DATA_INTEGRITY_EVENT_MATCH_TYPE_ENT       matchType
)
{
    PRV_PX_DATA_INTEGRITY_EVENT_INFO_STC *eventPtr;
    CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT memTypeCur;
    CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT eventTypeCur;
    CPSS_PX_TABLE_ENT tableTypeCur;
    GT_U32 entryIndexCur;
    GT_U32 i;
    GT_U32 foundMatch = 0;
    GT_U32 foundNotMatch = 0;
    GT_U32 failed = 0;
    GT_U32 firstEventToPrint = 0;
    GT_U32 eventsNumToPrint = 0;
    if (pxOccuredEvents.eventsCount == 0)
    {
        PRV_UTF_LOG0_MAC("No events occured!");
        return GT_FAIL;
    }
    if (traceEvents)
    {
        PRV_UTF_LOG3_MAC("prvCpssPxDiagDataIntegrityEventHwInfoDbCheck  expEntryIndex: %x expTableType =%d memTypeExp %d \n", entryIndexExp, tableTypeExp, memTypeExp);
        PRV_UTF_LOG1_MAC("eventsCount    : %d\n", pxOccuredEvents.eventsCount);
    }

    for (i = 0; i< pxOccuredEvents.eventsCount && !failed; i++)
    {
        eventPtr = &pxOccuredEvents.eventsArr[i];

        memTypeCur    = eventPtr->eventRamEntryInfo.memType;
        eventTypeCur  = eventPtr->eventsType;
        tableTypeCur  = eventPtr->eventHwEntryInfo.hwTableType;
        entryIndexCur = eventPtr->eventHwEntryInfo.hwTableEntryIndex;
        if (traceEvents)
        {
            PRV_UTF_LOG3_MAC("EVENT EntryIndex: %x TableType =%d memType%d \n",entryIndexCur,tableTypeCur,memTypeCur );
        }

        foundMatch = foundMatch || ((eventTypeCur  == eventTypeExp) &&
                                    (memTypeExp    == memTypeCur)   &&
                                    (tableTypeCur  == tableTypeExp) &&
                                    (entryIndexCur == entryIndexExp));
        foundNotMatch = foundNotMatch || (! foundMatch);

        failed = ((i == (pxOccuredEvents.eventsCount-1)) && (!foundMatch));
        firstEventToPrint = 0;
        eventsNumToPrint  = i + 1;
        if (traceEvents)
        {
            PRV_UTF_LOG1_MAC("eventsNumToPrint %d: \n" ,eventsNumToPrint);
        }
    }

    if (failed)
    {
        if (eventsNumToPrint)
        {
            for (i = 0; i < eventsNumToPrint; i++)
            {
                PRV_UTF_LOG1_MAC("'bad' events[%d]: \n" ,firstEventToPrint + i);
                eventPtr = &pxOccuredEvents.eventsArr[firstEventToPrint + i];
                pxPrintEventHwInfo(eventPtr);
            }
        }
        return GT_FAIL;
    }
    return GT_OK;
}

static GT_VOID prvCpssPxDiagDataIntegrityEventsClear
(
    GT_VOID
)
{
    pxOccuredEvents.eventsCount = 0;
}

#endif
GT_STATUS prvCpssPxDiagDataIntegrityTableLogicalToHwListGet
(
    IN  CPSS_PX_LOGICAL_TABLE_ENT         logicalTable,
    OUT const CPSS_PX_TABLE_ENT           **hwTableslistPtr
);
GT_BOOL  prvCpssPxDiagDataIntegrityTableCheckIsDevSupportHwTable(
    IN GT_SW_DEV_NUM       devNum ,
    IN CPSS_PX_TABLE_ENT    hwTableType,
    IN GT_U32               entryIndex
);
GT_STATUS prvCpssPxDiagDataIntegrityTableHwMaxIndexGet
(
    IN  GT_SW_DEV_NUM                      devNum,
    IN  CPSS_PX_TABLE_ENT                   hwTable,
    OUT GT_U32                              *maxNumEntriesPtr
);
GT_STATUS prvCpssPxDiagDataIntegrityDfxParamsConvert
(
    IN  GT_SW_DEV_NUM                                   devNum,
    IN  CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC    *memLocationPtr,
    IN  GT_U32                                          failedRow,
    OUT CPSS_PX_HW_INDEX_INFO_STC                       *hwErrorInfoPtr
);
CPSS_PX_SHADOW_TYPE_ENT  prvCpssPxDiagDataIntegrityTableCheckIsDevSupportHwTableShadow(
    IN GT_SW_DEV_NUM       devNum ,
    IN CPSS_PX_TABLE_ENT  hwTableType
);

/**
* @internal prvPxDebugTraceEnable function
* @endinternal
*
* @brief   Routine to enable trace for data integrity events
*
* @param[in] enable                   - enable/disable trace output
*                                       None
*/
void prvPxDebugTraceEnable
(
    IN GT_U32 enable
)
{
    traceEvents = enable;
    return;
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxDiagDataIntegrityTableScan
(
    IN  GT_U8                                 devNum,
    IN  CPSS_PX_LOCATION_SPECIFIC_INFO_STC    *locationPtr,
    IN  GT_U32                                numOfEntries,
    OUT GT_U32                                *nextEntryIndexPtr,
    OUT GT_BOOL                               *wasWrapAroundPtr
)
*/

UTF_TEST_CASE_MAC(cpssPxDiagDataIntegrityTableScan)
{
/*
    ITERATE_DEVICES(Pipe)

    1.1.1 Check correct values
        call with numOfEntries = 0;
        call with hwTableType  = CPSS_PX_TABLE_TXQ_QCN_CN_SAMPLE_INTERVALS_E
        call with numOfEntries = 100;
        call with hwTableType = CPSS_PX_TABLE_PHA_HA_TABLE_E
        call with hwTableEntryIndex = 10
        call with location.type = CPSS_PX_LOCATION_LOGICAL_INDEX_INFO_TYPE;
    Expected: GT_OK
    1.1.2 Check pointers values
    1.1.2.1 Check wrong numOfEntries
    Expected: GT_BAD_PARAM
    1.1.2.2 Check wasWrapAround can be NULL
    Expected: GT_OK
    1.1.3 Check location
    1.1.3.1 Call with wrong enum values location.type
    Expected: GT_BAD_PARAM
    1.1.3.2 Call with wrong enum values location.info.hwEntryInfo.hwTableType
    Expected: GT_BAD_PARAM
    1.1.3.3 Call with wrong enum values location.info.logicalEntryInfo.logicaTableInfo[0].logicalTableType
    Expected: GT_BAD_PARAM
    1.2. For unaware port groups check that function returns GT_OK.
    Expected: GT_OK
    2. For not-active devices and devices from non-applicable family
    Expected: GT_BAD_PARAM
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC   location;
    GT_U32                               numOfEntries;
    GT_U32                               nextEntryIndex;
    GT_BOOL                              wasWrapAround;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    GM_NOT_SUPPORT_THIS_TEST_MAC

    cpssOsBzero((GT_CHAR*)&location, sizeof(location));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* call with numOfEntries = 0; */
        /* call with hwTableType  = CPSS_PX_TABLE_TXQ_QCN_CN_SAMPLE_INTERVALS_E */
        numOfEntries  = 0;
        location.type = CPSS_PX_LOCATION_HW_INDEX_INFO_TYPE;
        location.info.hwEntryInfo.hwTableType = CPSS_PX_TABLE_TXQ_QCN_CN_SAMPLE_INTERVALS_E;
        location.info.hwEntryInfo.hwTableEntryIndex = 0;

        st = cpssPxDiagDataIntegrityTableScan(dev, &location, numOfEntries, &nextEntryIndex, &wasWrapAround);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,
                                    location.info.hwEntryInfo.hwTableType);
        /* call with numOfEntries = 100; */
        numOfEntries  = 100;

        st = cpssPxDiagDataIntegrityTableScan(dev, &location, numOfEntries, &nextEntryIndex, &wasWrapAround);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,
                                    location.info.hwEntryInfo.hwTableType);
        /* call with hwTableType = CPSS_PX_TABLE_PHA_HA_TABLE_E */
        location.info.hwEntryInfo.hwTableType = CPSS_PX_TABLE_PHA_HA_TABLE_E;
        st = cpssPxDiagDataIntegrityTableScan(dev, &location, numOfEntries, &nextEntryIndex, &wasWrapAround);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,
                                    location.info.hwEntryInfo.hwTableType);
        /* call with hwTableEntryIndex = 10 */
        location.info.hwEntryInfo.hwTableEntryIndex = 10;
        st = cpssPxDiagDataIntegrityTableScan(dev, &location, numOfEntries, &nextEntryIndex, &wasWrapAround);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,
                                    location.info.hwEntryInfo.hwTableType);

        /* call with location.type = CPSS_PX_LOCATION_LOGICAL_INDEX_INFO_TYPE; */
        location.type = CPSS_PX_LOCATION_LOGICAL_INDEX_INFO_TYPE;
        location.info.logicalEntryInfo.logicalTableType = CPSS_PX_LOGICAL_TABLE_BUFFERS_MANAGER_MULTICAST_COUNTERS_E;
        location.info.logicalEntryInfo.logicalTableEntryIndex = 0;
        st = cpssPxDiagDataIntegrityTableScan(dev, &location, numOfEntries, &nextEntryIndex, &wasWrapAround);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,
                                    location.info.logicalEntryInfo.logicalTableType);

        /*
            1.1.2 Check pointers values
            Expected: GT_BAD_PTR
        */
        st = cpssPxDiagDataIntegrityTableScan(dev, NULL, numOfEntries, &nextEntryIndex, &wasWrapAround);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        st = cpssPxDiagDataIntegrityTableScan(dev, &location, numOfEntries, NULL, &wasWrapAround);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.1.2.1 Check wrong numOfEntries
            Expected: GT_BAD_PARAM
        */

        location.type = CPSS_PX_LOCATION_HW_INDEX_INFO_TYPE;
        location.info.hwEntryInfo.hwTableType = CPSS_PX_TABLE_PHA_HA_TABLE_E;
        location.info.hwEntryInfo.hwTableEntryIndex = 0;

        numOfEntries = PRV_PX_TABLE_INFO_PTR_GET_MAC(dev,location.info.hwEntryInfo.hwTableType)->directAccessInfo.maxNumOfEntries;

        st = cpssPxDiagDataIntegrityTableScan(dev, &location, numOfEntries+1, &nextEntryIndex, &wasWrapAround);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev,
                location.info.hwEntryInfo.hwTableType);

        /*
            1.1.2.2 Check wasWrapAround can be NULL
            Expected: GT_OK
        */
        numOfEntries = 1;
        st = cpssPxDiagDataIntegrityTableScan(dev, &location, numOfEntries, &nextEntryIndex, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, location.info.hwEntryInfo.hwTableType);

        /* 1.1.3 Check location */

        /*
           1.1.3.1 Call with wrong enum values location.type
           Expected: GT_BAD_PARAM
        */
        UTF_ENUMS_CHECK_MAC(cpssPxDiagDataIntegrityTableScan
                            (dev, &location, numOfEntries, &nextEntryIndex, &wasWrapAround),
                            location.type);

        /*
           1.1.3.1 Call with wrong enum values location.info.hwEntryInfo.hwTableType
           Expected: GT_BAD_PARAM
        */
        numOfEntries  = 1;
        location.type = CPSS_PX_LOCATION_HW_INDEX_INFO_TYPE;
        location.info.hwEntryInfo.hwTableEntryIndex = 0;

        UTF_ENUMS_CHECK_MAC(cpssPxDiagDataIntegrityTableScan(dev, &location, numOfEntries, &nextEntryIndex, &wasWrapAround),
                            location.info.hwEntryInfo.hwTableType);

        /*
           1.1.3.1 Call with wrong enum values location.info.logicalEntryInfo.logicaTableInfo[0].logicalTableType
           Expected: GT_BAD_PARAM
        */
        location.type = CPSS_PX_LOCATION_LOGICAL_INDEX_INFO_TYPE;
        location.info.logicalEntryInfo.logicalTableType = CPSS_PX_LOGICAL_TABLE_PHA_SHARED_DMEM_E;
        location.info.logicalEntryInfo.logicalTableEntryIndex = 0;

        UTF_ENUMS_CHECK_MAC(cpssPxDiagDataIntegrityTableScan
                            (dev, &location, numOfEntries, &nextEntryIndex, &wasWrapAround),
                            location.info.logicalEntryInfo.logicalTableType);

        st = cpssPxDiagDataIntegrityTableScan(dev, &location, numOfEntries, &nextEntryIndex, &wasWrapAround);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        numOfEntries = 1;
        st = cpssPxDiagDataIntegrityTableScan(dev, &location, numOfEntries, &nextEntryIndex, &wasWrapAround);
        UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK, st);
    }

}
#ifndef ASIC_SIMULATION
GT_STATUS prvCpssPxDiagDataIntegrityCountersDbCheck
(
    IN CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memTypeExp,
    IN GT_U32                                        entryIndex,
    IN CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT eventTypeExp,
    IN GT_U32                                        eventCountExp,
    OUT CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC *dfxMemLocationPtr
)
{
    GT_U32 currentDbKey;
    GT_U32 eventCountCur = 0;
    CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT memTypeCur = CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TABLE_LAST_E;
    CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT eventTypeCur;
    CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC dfxMemLocation;
    TEST_PX_DIAG_DATA_INTEGRITY_COUNTER_DB_STC * dbEventCounterPtr = appDemoPxDiagDataIntegrityCountersDbGet();

    dfxMemLocationPtr->dfxPipeId   = 0xFFFFFFFF;
    dfxMemLocationPtr->dfxClientId = 0xFFFFFFFF;
    dfxMemLocationPtr->dfxMemoryId = 0xFFFFFFFF;

    for (currentDbKey = 0; currentDbKey < BIT_17; currentDbKey++)
    {
         if (dbEventCounterPtr->eventCounterArr[currentDbKey] == 0)
        {
            continue;
        }

        eventCountCur = dbEventCounterPtr->eventCounterArr[currentDbKey];
        memTypeCur = dbEventCounterPtr->memTypeArr[currentDbKey];

        dfxMemLocation.dfxMemoryId  = U32_GET_FIELD_MAC(currentDbKey,  0 , 7);
        dfxMemLocation.dfxClientId  = U32_GET_FIELD_MAC(currentDbKey,  7 , 5);
        dfxMemLocation.dfxPipeId    = U32_GET_FIELD_MAC(currentDbKey, 12 , 3);

        if (traceEvents > 2)
        {
            cpssOsPrintf("dfxPipeId = 0x%x\r\n", dfxMemLocation.dfxPipeId);
            cpssOsPrintf("dfxClientId = 0x%x\r\n", dfxMemLocation.dfxClientId);
            cpssOsPrintf("dfxMemoryId = %d\r\n", dfxMemLocation.dfxMemoryId);

            cpssOsPrintf("currentDbKey = 0x%x\r\n", currentDbKey);
            cpssOsPrintf("eventCounter = 0x%x\r\n", eventCountCur);
            cpssOsPrintf("eventsType = 0x%x\r\n", (currentDbKey >> 15) & 0x3);
            cpssOsPrintf("memType = 0x%x\r\n", memTypeCur);
        }

        switch ((currentDbKey >> 15) & 0x3)
        {
            case 1:
                eventTypeCur = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
                break;
            case 2:
                eventTypeCur = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
                break;
            default:
                continue;
        }

        if (eventTypeCur == eventTypeExp)
        {

            if (eventCountExp != eventCountCur || memTypeExp != memTypeCur)
            {

                UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(memTypeExp, memTypeCur,
                                                       "Expected memory type  :   %d, (%d)\r\n", memTypeExp, memTypeCur);
                UTF_VERIFY_EQUAL3_STRING_NO_RETURN_MAC(eventCountExp, eventCountCur,
                                                       "Expected event counter:   %d, (%d) %d\r\n", eventCountExp, eventCountCur, memTypeCur);
                return GT_FAIL;
            }

            /* Match found */
            *dfxMemLocationPtr = dfxMemLocation;
            return GT_OK;
        }
    }

    UTF_VERIFY_EQUAL3_STRING_NO_RETURN_MAC(eventCountExp, 0,
                                           "Expected DI event type %d for memory type %d entry index %d not triggered",
                                           eventTypeExp,
                                           memTypeExp, entryIndex);
    return GT_NOT_FOUND;
}


static void prvCpssPxDiagDataIntegrityCountersDbClear
(
    void
)
{
    TEST_PX_DIAG_DATA_INTEGRITY_COUNTER_DB_STC * dbEventCounterPtr = appDemoPxDiagDataIntegrityCountersDbGet();

    cpssOsBzero((GT_CHAR *)dbEventCounterPtr, sizeof(TEST_PX_DIAG_DATA_INTEGRITY_COUNTER_DB_STC));
    return;
}


/**
* @internal prvUtfDataIntegrityErrorInjectionTablesSkipCheck function
* @endinternal
*
* @brief   Function checks table avalability for test based on DFX injection error.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - PP device number
* @param[in] tableType                - table type
*
* @param[out] memTypePtr               - (pointer to) memory type
* @param[out] errTypePtr               - (pointer to) error type
* @param[out] numOfEntriesPtr          - (pointer to) number of table entries
*
* @retval GT_OK                    - on success
* @retval GT_NOT_SUPPORTED         - on not supported tableType for device
* @retval GT_NOT_IMPLEMENTED       - on table that does not support error injection
* @retval GT_BAD_VALUE             - on table without protection, error detection does not work
* @retval GT_FAIL                  - on error in DB or in test
*/
static GT_STATUS prvUtfDataIntegrityErrorInjectionTablesSkipCheck
(
    IN GT_U8   devNum,
    IN CPSS_PX_TABLE_ENT tableType,
    OUT CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT        *memTypePtr,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   *errTypePtr,
    OUT GT_U32                                          *numOfEntriesPtr
)
{
    GT_STATUS rc;
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC                      locationInfo;
    CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT  protectionType;

    /* check HW Table */
    rc = prvCpssPxDiagDataIntegrityHwTableCheck(devNum, tableType, memTypePtr);
    if (rc != GT_OK)
    {
        if (rc == GT_NOT_SUPPORTED)
        {
            /* tableType is not supported for device */
            return GT_NOT_SUPPORTED;
        }
        else if (rc == GT_NOT_FOUND)
        {
            /* table does not exist in RAM info DB */
            switch (tableType)
            {
                case CPSS_PX_TABLE_TXQ_DQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG_E : /* not supported by Data Integrity*/
                /** @brief txq shaper per port token bucket configuration
                 *  CPSS_PX_TABLE_TXQ_DQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E : DQ[1]
                 */
                case CPSS_PX_TABLE_TXQ_DQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E :         /* not supported by Data Integrity*/

                /** @brief Map the ingress port to TxQ port for PFC response
                 *  CPSS_PX_TABLE_TXQ_DQ_PFC_RESPONSE_PORT_MAPPING_E : DQ[1]
                 */
                case CPSS_PX_TABLE_TXQ_DQ_PFC_RESPONSE_PORT_MAPPING_E :                  /* not supported by Data Integrity*/

                /** Tail Drop Maximum Queue Limits */
                case CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_MAX_QUEUE_LIMITS_E :             /* not supported by Data Integrity*/
                /** @brief Tail Drop Counters -
                 *  Buffers Queue Maintenance counters
                 */
                case CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_COUNTERS_Q_MAIN_BUFF_E :          /* not supported by Data Integrity*/
                /** PHA source port data table. */
                case CPSS_PX_TABLE_PHA_SRC_PORT_DATA_E:                                  /* not supported by Data Integrity*/
                /** PHA target port data table. */
                case CPSS_PX_TABLE_PHA_TARGET_PORT_DATA_E:                               /* not supported by Data Integrity*/

                case CPSS_PX_MULTI_INSTANCE_TXQ_DQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG____DQ_1___E :/* not supported by Data Integrity*/

                case CPSS_PX_MULTI_INSTANCE_TXQ_DQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG____DQ_1___E :         /* not supported by Data Integrity*/

                case CPSS_PX_MULTI_INSTANCE_TXQ_DQ_PFC_RESPONSE_PORT_MAPPING_E____DQ_1___E :                 /* not supported by Data Integrity*/
                    /** PFC Counters */
                case CPSS_PX_TABLE_PFC_LLFC_COUNTERS_E :                                                      /* not supported by Data Integrity*/

                    return GT_NOT_IMPLEMENTED;
                default:
                    break;
            }
            return GT_FAIL;
        }
    }

    if ((tableType == CPSS_PX_TABLE_TXQ_QCN_CN_SAMPLE_INTERVALS_E)  ||/* erratum TXQ -1735 - error injection does not work */
        (tableType == CPSS_PX_TABLE_BMA_PORT_MAPPING_E)   ||
        (tableType == CPSS_PX_TABLE_BMA_MULTICAST_COUNTERS_E))

    {
        return GT_NOT_IMPLEMENTED;
    }
    /* check protection type */
    locationInfo.type = CPSS_PX_LOCATION_HW_INDEX_INFO_TYPE;
    locationInfo.info.hwEntryInfo.hwTableType = tableType;
    rc = cpssPxDiagDataIntegrityProtectionTypeGet(devNum, &locationInfo, &protectionType);
    if (rc != GT_OK)
    {
        /* there is bug in DB */
        return GT_FAIL;
    }

    switch (protectionType)
    {
        case CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ECC_E:
            *errTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
            break;

        case CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_PARITY_E:
            *errTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
            break;
        default: /*CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E */
            return GT_BAD_VALUE;
    }
    /* get number of entries in the table */
    rc = prvCpssPxDiagDataIntegrityTableHwMaxIndexGet(devNum,tableType,numOfEntriesPtr);
    if(rc != GT_OK)
    {
        return GT_FALSE;
    }

    return GT_OK;
}

UTF_TEST_CASE_MAC(cpssPxDiagDataIntegrityHwTablesErrorInjectionTest)
{
/*
    ITERATE_DEVICES (Pipe)
*/
    GT_STATUS           st = GT_OK;
    GT_U8               dev;

    GT_U32                                          tableType;
    CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT        memType;
    CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errType;
    GT_U32                                          entryIndex;
    GT_U32                                          i;
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC              dfxMemLocationInfo;
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC              location;
    GT_U32                                          nextEntryIndex;
    GT_BOOL                                         wasWrapAround;
    GT_U32                                          scanEntries;
    CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC    dfxMemLocation;
    GT_U32                                          errorCounter;
    GT_U32                                          failedRow;
    GT_U32                                          failedSegment;
    GT_U32                                          failedSyndrome;
    GT_U32                                          *tableEntryPtr = &tableEntry[0];
    GT_U32                                          tblsCount;
    GT_U32                                          readEntryBuf[MAX_ENTRY_SIZE_CNS];

    GT_U32                                          maxEntries = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    cpssOsMemSet(tableEntryPtr, 0xFF, sizeof(tableEntry));
    cpssOsMemSet(&dfxMemLocationInfo, 0, sizeof(dfxMemLocationInfo));
    cpssOsMemSet(&location, 0, sizeof(location));
    cpssOsMemSet(&dfxMemLocation, 0, sizeof(dfxMemLocation));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* clean DB */
        prvCpssPxDiagDataIntegrityCountersDbClear();

        tblsCount = 0;
        /* loop over all memories and inject errors */
        for(tableType = 0 ; tableType < CPSS_PX_TABLE_LAST_E; tableType++)
        {

            st = prvUtfDataIntegrityErrorInjectionTablesSkipCheck(dev, tableType, &memType, &errType, &maxEntries);

            if (st != GT_OK)
            {
                if (traceEvents)
                {
                    switch (st)
                    {
                        case GT_NOT_FOUND:
                            cpssOsPrintf(" table %d - not found in DB\r\n",tableType);
                            break;
                        case GT_NOT_SUPPORTED:
                            cpssOsPrintf(" table %d - not exists \r\n", tableType);
                            break;
                        case GT_NOT_IMPLEMENTED:
                            cpssOsPrintf(" table %d - not used in test\r\n", tableType);
                            break;
                        case GT_BAD_VALUE:
                            cpssOsPrintf(" table %d - not protected table\r\n", tableType);
                            break;
                    }
                }

                continue;
            }

            if (traceEvents)
            {
                cpssOsPrintf("Test table %d\n", tableType);
            }

            /* Set max table entries for scan */

            scanEntries = maxEntries;
            tblsCount++;
            for (i = 0; i < 3; i++)
            {
                /* Calculate entry index and write table entry field */
                switch (tableType)
                {
                    case CPSS_PX_TABLE_PHA_FW_IMAGE_E:
                        /*address should be from the predefined list - since we don't want to stuck Tensilica:
                          */
                        entryIndex = prvDataIntegrityFwImageTestAddressesList[i];
                        break;
                    default:
                        entryIndex = (i == 2) ? 0 : ((maxEntries - 1) / (i + 1));
                        break;
                }

                /* Check entry index for HW table */
                if(GT_FALSE ==
                    prvCpssPxDiagDataIntegrityTableCheckIsDevSupportHwTable(dev, tableType, entryIndex))
                {
                    PRV_UTF_LOG1_MAC("table: not supported %d\n", tableType);
                    continue;
                }
                dfxMemLocationInfo.type = CPSS_PX_LOCATION_RAM_INDEX_INFO_TYPE;
                dfxMemLocationInfo.info.ramEntryInfo.memType = memType;

                if (selectedTableType != 0xFFFFFFFF)
                {
                    PRV_UTF_LOG0_MAC("cpssPxDiagDataIntegrityEventMaskSet:\n");
                }
                /* Enable error counter */
                st = cpssPxDiagDataIntegrityErrorCountEnableSet(dev, &dfxMemLocationInfo, errType, GT_TRUE);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                /* Table found in DFX data base - unmask interrupt for DFX memory */
                st = cpssPxDiagDataIntegrityEventMaskSet(dev, &dfxMemLocationInfo, errType, CPSS_EVENT_UNMASK_E);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                if (selectedTableType != 0xFFFFFFFF)
                {
                    PRV_UTF_LOG0_MAC("cpssPxDiagDataIntegrityErrorInjectionConfigSet:\n");
                }

                /* Enable error injection */
                st = cpssPxDiagDataIntegrityErrorInjectionConfigSet(dev, &dfxMemLocationInfo, CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E, GT_TRUE);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                if (selectedTableType != 0xFFFFFFFF)
                {
                    PRV_UTF_LOG0_MAC("prvCpssPxPortGroupWriteTableEntry:\n");
                }

                if (tableType == CPSS_PX_TABLE_CNC_0_COUNTERS_E ||
                    tableType == CPSS_PX_TABLE_CNC_1_COUNTERS_E)
                {
                    /* It's prohibited to write to CNC memory. Device stuck.
                       Need to use read operation instead. The read operation
                       writes to CNC memory also because it's clear on read.*/
                    st = prvCpssPxPortGroupReadTableEntry(
                        dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                        tableType, entryIndex, &readEntryBuf[0]);
                }
                else if (tableType == CPSS_PX_TABLE_PHA_FW_IMAGE_E)
                {

                    st = prvFwImageTableEntryWrite(dev,entryIndex);

                }
                else
                {
                    st = prvCpssPxPortGroupWriteTableEntry(
                        dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                        tableType, entryIndex, tableEntryPtr);
                }


                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                location.type = CPSS_PX_LOCATION_HW_INDEX_INFO_TYPE;
                location.info.hwEntryInfo.hwTableType = tableType;
                location.info.hwEntryInfo.hwTableEntryIndex = 0;
                if (selectedNumOfEntries != 0xFFFFFFFF)
                {
                    scanEntries = selectedNumOfEntries;
                    location.info.hwEntryInfo.hwTableEntryIndex = entryIndex;
                }

                if (selectedTableType != 0xFFFFFFFF)
                {
                    PRV_UTF_LOG0_MAC("cpssPxDiagDataIntegrityTableScan:\n");
                }
                else
                {
                    cpssOsTimerWkAfter(10);
                }


                /* Scan whole table */
                st = cpssPxDiagDataIntegrityTableScan(dev, &location, scanEntries, &nextEntryIndex, &wasWrapAround);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /* Sleep 10 mSec */
                cpssOsTimerWkAfter(pxTimeOut);

                /* Check specific event for current memory */
                st = prvCpssPxDiagDataIntegrityCountersDbCheck(memType, entryIndex, errType, 1, &dfxMemLocation);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                if (selectedTableType != 0xFFFFFFFF)
                {
                    PRV_UTF_LOG1_MAC("prvCpssPxDiagDataIntegrityCountersDbCheck:st = %d\n",st);
                }

                dfxMemLocationInfo.info.ramEntryInfo.memLocation.dfxPipeId =   dfxMemLocation.dfxPipeId;
                dfxMemLocationInfo.info.ramEntryInfo.memLocation.dfxClientId = dfxMemLocation.dfxClientId;
                dfxMemLocationInfo.info.ramEntryInfo.memLocation.dfxMemoryId = dfxMemLocation.dfxMemoryId;

                st = cpssPxDiagDataIntegrityErrorInfoGet(dev, &dfxMemLocationInfo, NULL,
                                                           &errorCounter, &failedRow, &failedSegment, &failedSyndrome);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                if (st == GT_OK)
                {
                    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(0, errorCounter, dev);
                    if (errorCounter == 0)
                    {
                        cpssOsPrintf("%d - table (%d scan entries) - Current error counter = 0 \r\n",
                                     tableType, scanEntries);
                    }
                }

                /* Disable error injection */
                st = cpssPxDiagDataIntegrityErrorInjectionConfigSet(
                    dev, &dfxMemLocationInfo,
                    CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E,
                    GT_FALSE);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                /* Write the entry. This operation may generate Data
                   Integrity interrupt when one RAM have several HW entries.
                   Need to clean DB after this call. */

                if (tableType == CPSS_PX_TABLE_PHA_FW_IMAGE_E)
                {

                    st = prvFwImageTableEntryWrite(dev,entryIndex);

                }

                else if ((tableType != CPSS_PX_TABLE_CNC_0_COUNTERS_E) && /* It's prohibited to write to CNC memory in BC3 and above. Device stuck. */
                    (tableType != CPSS_PX_TABLE_CNC_1_COUNTERS_E))
                {
                   st = prvCpssPxPortGroupWriteTableEntry(
                        dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                        tableType, entryIndex, tableEntryPtr);
                }
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
                /* Sleep  */
                cpssOsTimerWkAfter(pxTimeOut);

                /* clean DB */
                prvCpssPxDiagDataIntegrityCountersDbClear();

                /* Mask DFX memory interrupt */
                st = cpssPxDiagDataIntegrityEventMaskSet(dev, &dfxMemLocationInfo, errType, CPSS_EVENT_MASK_E);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            }
        }
        if (selectedTableType != 0xFFFFFFFF)
        {
            PRV_UTF_LOG2_MAC("Device %d Number tested HW Tables %d:\n", dev, tblsCount);
        }
    }
}
#endif

UTF_TEST_CASE_MAC(cpssPxDiagDataIntegrityTableScan_1)
{
    GT_STATUS                                       rc;
    GT_U8                                           devNum;
    GT_U32                                          ii;
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC              specificLocation;
    GT_BOOL                                         foundMatch;
    GT_U32                                          numOfEntries;
    GT_U32                                          nextEntryIndex;
    GT_U32                                          expectedNextEntryIndex;
    GT_BOOL                                         wasWrapAround;
    GT_U32                                          global_maxTableSize;
    CPSS_PX_TABLE_ENT                               hwTableType;
    GT_U32                                          maxTableSize;
    const CPSS_PX_TABLE_ENT                         *hwNameArr;
    CPSS_PX_LOGICAL_TABLE_INFO_STC                  *logicalInfoPtr;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    GM_NOT_SUPPORT_THIS_TEST_MAC;

    cpssOsBzero((GT_CHAR*)&specificLocation, sizeof(specificLocation));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {

        /* test logical tables */
        specificLocation.type = CPSS_PX_LOCATION_LOGICAL_INDEX_INFO_TYPE;
        logicalInfoPtr = &specificLocation.info.logicalEntryInfo;
        for(ii = 0 ; ii < CPSS_PX_LOGICAL_TABLE_LAST_E; ii++)
        {
            GT_U32  jj;

            logicalInfoPtr->logicalTableType = ii;

            foundMatch = GT_FALSE;

            global_maxTableSize = 0;
            rc = prvCpssPxDiagDataIntegrityTableLogicalToHwListGet((CPSS_PX_LOGICAL_TABLE_ENT)ii, &hwNameArr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, devNum);


            for(jj = 0; hwNameArr[jj] != LAST_VALID_PX_TABLE_CNS; jj++)
            { /* hw tables iteration */
                hwTableType = hwNameArr[jj];

                if(GT_FALSE ==
                    prvCpssPxDiagDataIntegrityTableCheckIsDevSupportHwTable(devNum,hwTableType,0))
                {
                    continue;
                }

                rc = prvCpssPxDiagDataIntegrityTableHwMaxIndexGet((GT_U8)devNum,hwTableType,&maxTableSize);
                if(rc != GT_OK)
                {
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, rc, devNum, ii, hwTableType, maxTableSize);
                }

                if(global_maxTableSize < maxTableSize)
                {
                    /* update the 'global' max range */
                    global_maxTableSize = maxTableSize;
                }

                foundMatch = GT_TRUE;/* at least one table supported by the device */
                /*break; need to continue for the 'global_maxTableSize' */
            }
            numOfEntries = (global_maxTableSize / 2)- 1;
            /* from (almost) mid table till (almost) end */
            logicalInfoPtr->logicalTableEntryIndex =
                (global_maxTableSize / 2);

            rc = cpssPxDiagDataIntegrityTableScan(devNum,&specificLocation,
                numOfEntries,
                &nextEntryIndex,
                &wasWrapAround);


            if (foundMatch == GT_TRUE)
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc, devNum, ii);
            }
            else
            {
                /* expected an error ! */
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, rc, devNum, ii);
                /* in any way nothing to do */
                continue;
            }

            expectedNextEntryIndex = logicalInfoPtr->logicalTableEntryIndex + numOfEntries;

            if(nextEntryIndex != expectedNextEntryIndex)
            {
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, GT_BAD_STATE, devNum, ii, nextEntryIndex, expectedNextEntryIndex);
            }

            if(wasWrapAround != GT_FALSE)
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, GT_BAD_STATE, devNum);
            }

            /* we are done with previous scan .. update to 'next' */
            /* keeping 'numOfEntries' */
            logicalInfoPtr->logicalTableEntryIndex =
                nextEntryIndex;
            rc = cpssPxDiagDataIntegrityTableScan(devNum,&specificLocation,
                numOfEntries,
                &nextEntryIndex,
                &wasWrapAround);

            if(rc != GT_OK)
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc, devNum, ii);
            }

            expectedNextEntryIndex = logicalInfoPtr->logicalTableEntryIndex + numOfEntries;
            expectedNextEntryIndex %= global_maxTableSize;

            if (nextEntryIndex != expectedNextEntryIndex)
            {
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, GT_BAD_STATE, devNum, ii, nextEntryIndex, expectedNextEntryIndex);
            }

            if(wasWrapAround != GT_TRUE)
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, GT_BAD_STATE, devNum, ii);
            }
        }

        /* test HW tables */
        specificLocation.type = CPSS_PX_LOCATION_HW_INDEX_INFO_TYPE;
        for(ii = 0 ; ii < CPSS_PX_TABLE_LAST_E; ii++)
        {
            if(GT_FALSE ==
                prvCpssPxDiagDataIntegrityTableCheckIsDevSupportHwTable(devNum,ii,
                0))
            {
                /* the device not supports this HW table */
                continue;
            }

            hwTableType = ii;

            /* get number of entries in the table */
            rc = prvCpssPxDiagDataIntegrityTableHwMaxIndexGet(devNum,hwTableType,&global_maxTableSize);
            if(rc != GT_OK)
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, devNum);
            }

            specificLocation.info.hwEntryInfo.hwTableType = ii;

            specificLocation.info.hwEntryInfo.hwTableEntryIndex =
                (global_maxTableSize / 2) - 1;
            numOfEntries = global_maxTableSize / 2;

            rc = cpssPxDiagDataIntegrityTableScan(devNum,&specificLocation,
                    numOfEntries,
                    &nextEntryIndex,
                    &wasWrapAround);
            if(rc != GT_OK)
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, devNum);
            }

            if(nextEntryIndex !=
                (specificLocation.info.hwEntryInfo.hwTableEntryIndex + numOfEntries))
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, GT_BAD_STATE, devNum);
            }

            if(wasWrapAround != GT_FALSE)
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, GT_BAD_STATE, devNum);
            }

            specificLocation.info.hwEntryInfo.hwTableEntryIndex = nextEntryIndex;
            rc = cpssPxDiagDataIntegrityTableScan(devNum,&specificLocation,
                    numOfEntries,
                    &nextEntryIndex,
                    &wasWrapAround);
            if(rc != GT_OK)
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, devNum);
            }

            if(nextEntryIndex !=
                ((specificLocation.info.hwEntryInfo.hwTableEntryIndex + numOfEntries) % global_maxTableSize))
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, GT_BAD_STATE, devNum);
            }

            if(wasWrapAround != GT_TRUE)
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, GT_BAD_STATE, devNum);
            }
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        numOfEntries = 1;
        rc = cpssPxDiagDataIntegrityTableScan(devNum, &specificLocation, numOfEntries, &nextEntryIndex, &wasWrapAround);
        UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK, rc);
    }

}

UTF_TEST_CASE_MAC(cpssPxDiagDataIntegrityTableEntryFix)
{
    GT_STATUS   rc;
    GT_U8                                           devNum;
    PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    *dbArrayPtr; /* pointer to data integrity DB */
    GT_U32                                          dbArrayEntryNum; /* size of data integrity DB */
    PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    *currentEntryPtr;
    /*PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    *nextEntryPtr;*/
    CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC    currentRamInfo;
    GT_U32                                          failedRow;
    GT_U32                                          ii/*tmpIndex*/;
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC              specificLocation;
    GT_BOOL                                         foundMatch;
    const CPSS_PX_TABLE_ENT                         *hwNameArr;
    GT_BOOL                                         isErrorExpected;
    CPSS_PX_HW_INDEX_INFO_STC                       hwEntryInfo;
    CPSS_PX_TABLE_ENT                               hwTableType;
     /* prepare device iterator */
     PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    cpssOsBzero((GT_CHAR*)&specificLocation, sizeof(specificLocation));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        if(prvTgfResetModeGet() == GT_FALSE)
        {
            /* the 'fix' function currently setting 'dummy' values to the tables */
            /* that cause bad configurations ... after the test we need 'HW reset'*/
            /* but if we can't reset ... we should not run the test */
            SKIP_TEST_MAC;
        }


        prvCpssPxDiagDataIntegrityDbPointerSet(&dbArrayPtr, &dbArrayEntryNum);

        currentEntryPtr = &dbArrayPtr[0];

        specificLocation.type = CPSS_PX_LOCATION_RAM_INDEX_INFO_TYPE;
        specificLocation.info.ramEntryInfo.memType = CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_MG_MGCAM_E;
        /* test DFX memory */
        for(ii = 0 ; ii < dbArrayEntryNum ; ii++ , currentEntryPtr++)
        {
            currentRamInfo.dfxPipeId   = U32_GET_FIELD_MAC(currentEntryPtr->key, 12, 3);
            currentRamInfo.dfxClientId = U32_GET_FIELD_MAC(currentEntryPtr->key, 7,  5);
            currentRamInfo.dfxMemoryId = U32_GET_FIELD_MAC(currentEntryPtr->key, 0,  7);

            /* use last index of this RAM */
            failedRow = 31;

            specificLocation.info.ramEntryInfo.memLocation = currentRamInfo;
            specificLocation.info.ramEntryInfo.ramRow = failedRow;

            hwTableType = CPSS_PX_INTERNAL_TABLE_MANAGEMENT_E;
            /* convert RAM info to 'HW info' */
            rc = prvCpssPxDiagDataIntegrityDfxParamsConvert(devNum,
                    &specificLocation.info.ramEntryInfo.memLocation,
                    specificLocation.info.ramEntryInfo.ramRow,&hwEntryInfo);
            if(rc == GT_EMPTY)
            {
                /* ignore this RAM */
            }
            else
            {
                if (rc != GT_OK)
                {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc, devNum, currentEntryPtr->key);
                }

                hwTableType = hwEntryInfo.hwTableType;

                while(failedRow)
                {
                    if(GT_FALSE ==
                       prvCpssPxDiagDataIntegrityTableCheckIsDevSupportHwTable(devNum, hwTableType, hwEntryInfo.hwTableEntryIndex))
                    {
                        /* we get new index into hwEntryInfo.hwTableEntryIndex */
                        failedRow--;/* started at 31 and ... going down */
                        specificLocation.info.ramEntryInfo.ramRow = failedRow;

                        /* convert RAM info to 'HW info' */
                        rc = prvCpssPxDiagDataIntegrityDfxParamsConvert(devNum,
                                                                        &specificLocation.info.ramEntryInfo.memLocation,
                                                                        failedRow,&hwEntryInfo);
                        continue;
                    }
                    break;/* the .hwTableEntryIndex is supported in the HW table */
                }
            }
            if(CPSS_PX_SHADOW_TYPE_NONE_E !=
                prvCpssPxDiagDataIntegrityTableCheckIsDevSupportHwTableShadow(devNum,hwTableType))
            {
                isErrorExpected = GT_FALSE;
            }
            else
            {
                isErrorExpected = GT_TRUE;
            }

            rc = cpssPxDiagDataIntegrityTableEntryFix(devNum,&specificLocation);
            if(rc != GT_OK)
            {
                if(isErrorExpected == GT_FALSE)
                {
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, rc, devNum, currentEntryPtr->key, failedRow, hwEntryInfo.hwTableEntryIndex);
                }
                else
                {
                    UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, rc, devNum, currentEntryPtr->key, failedRow, hwEntryInfo.hwTableEntryIndex);
                }
            }
        }

        /* test logical tables */
        failedRow = 7;
        specificLocation.type = CPSS_PX_LOCATION_LOGICAL_INDEX_INFO_TYPE;
        specificLocation.info.logicalEntryInfo.logicalTableEntryIndex = failedRow;

        for(ii = 8 ; ii < CPSS_PX_LOGICAL_TABLE_LAST_E; ii++)
        {
            GT_U32  jj;

            specificLocation.info.logicalEntryInfo.logicalTableType = ii;

            foundMatch = GT_FALSE;

            rc = prvCpssPxDiagDataIntegrityTableLogicalToHwListGet(ii, &hwNameArr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, devNum);

            isErrorExpected = GT_FALSE;
            for(jj = 0; hwNameArr[jj] != LAST_VALID_PX_TABLE_CNS; jj++)
            { /* hw tables iteration */
                CPSS_PX_TABLE_ENT                     hwTableType;
                hwTableType = hwNameArr[jj];
                if(GT_FALSE ==
                   prvCpssPxDiagDataIntegrityTableCheckIsDevSupportHwTable(
                       devNum,hwTableType, failedRow))
                {
                    /* the device not supports this HW table */
                    continue;
                }
                if(CPSS_PX_SHADOW_TYPE_NONE_E !=
                    prvCpssPxDiagDataIntegrityTableCheckIsDevSupportHwTableShadow(devNum,hwTableType))
                {
                    isErrorExpected = GT_FALSE;
                }
                else
                {
                    isErrorExpected = GT_TRUE;
                }
                foundMatch = GT_TRUE;/* at least one table supported by the device */
                break;
            }

            rc = cpssPxDiagDataIntegrityTableEntryFix(devNum,&specificLocation);
            if(foundMatch == GT_TRUE && rc != GT_OK && isErrorExpected == GT_FALSE)
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, devNum);
            }
            else
            if(isErrorExpected == GT_TRUE && rc == GT_OK )
            {
                /* expected error ! */
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, GT_BAD_STATE, devNum);
            }
            else
            if(foundMatch == GT_FALSE && rc == GT_OK)
            {
                /* expected error ! */

                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, GT_BAD_STATE, devNum);
            }
        }

        /* test HW tables */
        specificLocation.type = CPSS_PX_LOCATION_HW_INDEX_INFO_TYPE;
        specificLocation.info.hwEntryInfo.hwTableEntryIndex = failedRow;
        for(ii = 0 ; ii < CPSS_PX_TABLE_LAST_E; ii++)
        {
            if(GT_FALSE ==
                prvCpssPxDiagDataIntegrityTableCheckIsDevSupportHwTable(devNum,ii,
                failedRow))
            {
                /* the device doesn't support this HW table entry writing */
                continue;
            }

            if(CPSS_PX_SHADOW_TYPE_NONE_E !=
                prvCpssPxDiagDataIntegrityTableCheckIsDevSupportHwTableShadow(devNum,ii))
            {
                isErrorExpected = GT_FALSE;
            }
            else
            {
                isErrorExpected = GT_TRUE;
            }

            specificLocation.info.hwEntryInfo.hwTableType = ii;

            rc = cpssPxDiagDataIntegrityTableEntryFix(devNum,&specificLocation);
            if(isErrorExpected == GT_FALSE)
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc, devNum, ii);
            }
            else
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, rc, devNum, ii);
            }
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        rc = cpssPxDiagDataIntegrityTableEntryFix(devNum, &specificLocation);
        UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK, rc);
    }
}

#if 0
GT_VOID cpssPxDiagDataIntegrityTables_cleanup_withSystemReset(GT_VOID)
{
    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;
    /* the function cpssPxDiagDataIntegrityTableEntryFix() may leave the
       device in bad state */
    if(prvTgfResetModeGet() == GT_TRUE)
    {
        /* give priority to reset with HW */
        prvTgfResetAndInitSystem();
    }
    else
    {
        SKIP_TEST_MAC;
    }
}

UTF_TEST_CASE_MAC(cpssPxDiagDataIntegrityTables_cleanup_withSystemReset)
{
    cpssPxDiagDataIntegrityTables_cleanup_withSystemReset();
}
#endif
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxDiagDataIntegrityShadowEntryInfoGet
(
    IN     GT_U8                                     devNum,
    INOUT  CPSS_PX_LOGICAL_TABLES_SHADOW_INFO_STC *tablesInfoPtr,
    OUT    GT_U32                                   *totalMemSizePtr
)
*/

UTF_TEST_CASE_MAC(cpssPxDiagDataIntegrityShadowEntryInfoGet)
{
/*
    ITERATE_DEVICES(Pipe)

    1.1.1 Check correct values
    Expected: GT_OK
        - call with CPSS_PX_SHADOW_TYPE_ALL_CPSS_CNS
        - call with NUM_OF_LOGICAL_TABLES_CNS
        - check that return values are not out of range
    1.1.2 Check NULL pointers values
    Expected: GT_BAD_PTR
    1.1.3 Check with wrong enum value shadowType
    Expected: GT_BAD_PARAM
    2. For not-active devices and devices from non-applicable family
    Expected: GT_BAD_PARAM
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;
    GT_U32      i;

    CPSS_PX_LOGICAL_SHADOW_TABLES_INFO_STC tablesInfo;
    GT_U32                                   totalMemSize;
#define NUM_OF_LOGICAL_TABLES_CNS 3
    CPSS_PX_LOGICAL_TABLE_SHADOW_INFO_STC logArr[NUM_OF_LOGICAL_TABLES_CNS];

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    GM_NOT_SUPPORT_THIS_TEST_MAC;

    cpssOsMemSet(&tablesInfo, 0, sizeof(CPSS_PX_LOGICAL_SHADOW_TABLES_INFO_STC));
    cpssOsMemSet(logArr, 0, sizeof(CPSS_PX_LOGICAL_SHADOW_TABLES_INFO_STC)*NUM_OF_LOGICAL_TABLES_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1.1 Check correct values
            Expected: GT_OK
        */

        /* call with CPSS_PX_SHADOW_TYPE_ALL_CPSS_CNS */
        tablesInfo.numOfDataIntegrityElements = CPSS_PX_SHADOW_TYPE_ALL_CPSS_CNS;
        st = cpssPxDiagDataIntegrityShadowTableSizeGet(dev, &tablesInfo, &totalMemSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if(GT_OK == st)
        {
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(totalMemSize, 0, dev);
        }

        /* call with NUM_OF_LOGICAL_TABLES_CNS */
        tablesInfo.numOfDataIntegrityElements = NUM_OF_LOGICAL_TABLES_CNS;
        tablesInfo.logicalTablesArr = logArr;

        tablesInfo.logicalTablesArr[0].logicalTableName = CPSS_PX_LOGICAL_TABLE_INGRESS_DST_PORT_MAP_TABLE_E;
        tablesInfo.logicalTablesArr[0].shadowType       = CPSS_PX_SHADOW_TYPE_CPSS_E;

        tablesInfo.logicalTablesArr[1].logicalTableName = CPSS_PX_LOGICAL_TABLE_INGRESS_PORT_FILTERING_TABLE_E;
        tablesInfo.logicalTablesArr[1].shadowType       = CPSS_PX_SHADOW_TYPE_CPSS_E;

        tablesInfo.logicalTablesArr[2].logicalTableName = CPSS_PX_LOGICAL_TABLE_EGRESS_HEADER_ALTERATION_TABLE_E;
        tablesInfo.logicalTablesArr[2].shadowType       = CPSS_PX_SHADOW_TYPE_CPSS_E;

        st = cpssPxDiagDataIntegrityShadowTableSizeGet(dev, &tablesInfo, &totalMemSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if(GT_OK == st)
        {
            if(totalMemSize) /* mean if shadow enabled in general */
            {
                /* check that return values are not out of range */
                for(i = 0; i < NUM_OF_LOGICAL_TABLES_CNS; i++)
                {
                    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(tablesInfo.logicalTablesArr[i].numOfBytes, 0, dev);
                    UTF_VERIFY_EQUAL1_PARAM_MAC(tablesInfo.logicalTablesArr[i].isSupported, GT_OK, dev);
                }
            }
        }

        /*
            1.1.2 Check NULL pointers values
            Expected: GT_BAD_PTR
        */
        st = cpssPxDiagDataIntegrityShadowTableSizeGet(dev, NULL, &totalMemSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        st = cpssPxDiagDataIntegrityShadowTableSizeGet(dev, &tablesInfo, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);


        tablesInfo.numOfDataIntegrityElements = NUM_OF_LOGICAL_TABLES_CNS;
        tablesInfo.logicalTablesArr = NULL;
        st = cpssPxDiagDataIntegrityShadowTableSizeGet(dev, &tablesInfo, &totalMemSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        tablesInfo.logicalTablesArr = logArr; /* restore */

        /*
            1.1.3 Check with wrong enum value shadowType
            Expected: GT_BAD_PARAM
        */
        for(i = 0; i < NUM_OF_LOGICAL_TABLES_CNS; i++)
        {
            UTF_ENUMS_CHECK_MAC(cpssPxDiagDataIntegrityShadowTableSizeGet
                                (dev, &tablesInfo, &totalMemSize),
                                tablesInfo.logicalTablesArr[i].shadowType);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    GM_NOT_SUPPORT_THIS_TEST_MAC;
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxDiagDataIntegrityShadowTableSizeGet(dev, &tablesInfo, &totalMemSize);
        UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK, st);
    }
}
/* max number of words in entry */
#define WRITE_ONLY  0
#define CHECK_SHADOW 1
#define CHECK_SHADOW_WITH_SELF_COMPARE  2

#define IGNORE_TABLE 0x0
static GT_BOOL didError_shadowTest_onDemand = GT_FALSE;

/* test the shadow of the CPSS :
    set entries in the HW table and check that read from HW match read from shadow.

*/
static void cpssPxDiagDataIntegrityTables_shadowTest_entry(
    IN GT_U8               devNum,
    IN CPSS_PX_TABLE_ENT   hwTableType,
    IN GT_U32              entryIndex,
    IN GT_U32              operation
)
{
    GT_STATUS   rc;
    GT_PORT_GROUPS_BMP portGroupId = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    static GT_U32  buff_writeHw[MAX_ENTRY_SIZE_CNS];
    static GT_U32  buff_readHw[MAX_ENTRY_SIZE_CNS];
    static GT_U32  buff_readShadow[MAX_ENTRY_SIZE_CNS];
    static GT_U32  compareBitsPtr[MAX_ENTRY_SIZE_CNS];
    static GT_U32  buff_readHw_entry_0[MAX_ENTRY_SIZE_CNS];
    GT_U32 ii;
    GT_U32  numBitsPerEntry;/* number of bits per entry */
    GT_U32  maxEntrySize = MAX_ENTRY_SIZE_CNS;

    if(operation == CHECK_SHADOW_WITH_SELF_COMPARE)
    {
        GT_U32 entryIndex_0 = 0;

        if(entryIndex == 0)
        {
            /* save original entry 0 values */
            cpssOsMemSet(buff_readHw_entry_0,0,sizeof(buff_readHw_entry_0));
            rc = prvCpssPxPortGroupReadTableEntry(devNum,portGroupId,hwTableType,entryIndex_0,buff_readHw_entry_0);
            if(rc != GT_OK)
            {
                PRV_UTF_LOG2_MAC("error 'save original' HW from table[%d] entry[%d] \n",
                    hwTableType,entryIndex_0);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
            }

            /*********************************************/
            /* need to learn about the mask of the table */
            /*********************************************/
            operation = WRITE_ONLY;

            /* continue to 'WRITE_ONLY' operation for index 0 */

        }
        else /* indication to restore index 0 to original value */
        {
            rc = prvCpssPxPortGroupWriteTableEntry(devNum,portGroupId,
                hwTableType,entryIndex_0,buff_readHw_entry_0);
            if(rc != GT_OK)
            {
                PRV_UTF_LOG2_MAC("error 'restore' HW to table[%d] entry[%d] \n",
                    hwTableType,entryIndex_0);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
            }

            return;
        }
    }

    if(operation == WRITE_ONLY)
    {
        cpssOsMemSet(buff_writeHw,(0xFF-(GT_U8)entryIndex),sizeof(buff_writeHw));

        rc = prvCpssPxPortGroupWriteTableEntry(devNum,portGroupId,
            hwTableType,entryIndex,buff_writeHw);
        if(rc != GT_OK)
        {
            UTF_VERIFY_EQUAL4_STRING_MAC(
                            GT_OK, rc,
                            "error write HW to table[%d] hwTableType %d entry[%d] portGroupId %d",
                            hwTableType,hwTableType,entryIndex, portGroupId);
        }
    }
    else /* CHECK_SHADOW */
    {
        cpssOsMemSet(buff_readHw,0,sizeof(buff_readHw));
        rc = prvCpssPxPortGroupReadTableEntry(devNum,portGroupId,
            hwTableType,entryIndex,buff_readHw);

        if(rc != GT_OK)
        {
            PRV_UTF_LOG2_MAC("error read HW from table[%d] entry[%d] \n",
                hwTableType,entryIndex);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        }

        if(entryIndex == 0)
        {
            /* the read from HW of this entry will give us the 'mask' of applicable bits */
            cpssOsMemCpy(compareBitsPtr,buff_readHw,sizeof(compareBitsPtr));
            /* check if this table is 'special' */
        }


        cpssOsMemSet(buff_readShadow,0,sizeof(buff_readShadow));

        rc = prvCpssPxReadTableEntry_fromShadow(devNum,hwTableType,entryIndex,&numBitsPerEntry, buff_readShadow);
#ifdef ASIC_SIMULATION

        maxEntrySize = (numBitsPerEntry % 32)? (numBitsPerEntry / 32 + 1) : (numBitsPerEntry / 32);

        for(ii = 0; ii < maxEntrySize ; ii++)
        {
           compareBitsPtr[ii] = (1 << (numBitsPerEntry - 32 *ii)) - 1;
        }
#endif
        if(rc != GT_OK)
        {
            PRV_UTF_LOG2_MAC("error read Shadow from table[%d] entry[%d] \n",
                hwTableType,entryIndex);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        }

        /* compare read from HW and from Shadow */

        for(ii = 0; ii <  maxEntrySize ; ii++)
        {
            GT_U32  hwValue,shadowValue;

            hwValue     = buff_readHw[ii];
            shadowValue = buff_readShadow[ii];

            /* mask with bits that should be compared*/
            hwValue     &= compareBitsPtr[ii];
            shadowValue &= compareBitsPtr[ii];


            if(hwValue != shadowValue)
            {
                PRV_UTF_LOG5_MAC("Shadow and HW different at table[%d] entry[%d] word[%d] : HW[0x%8.8x] shadow[0x%8.8x]\n",
                    hwTableType,entryIndex,
                    ii,
                     buff_readHw[ii],
                     buff_readShadow[ii]);
                PRV_UTF_LOG1_MAC("Shadow and HW different at table[%d] \n",hwTableType);
                if(compareBitsPtr[ii] != 0xFFFFFFFF)
                {
                    PRV_UTF_LOG3_MAC("with compareBits[0x%8.8x] : HW[0x%8.8x] shadow[0x%8.8x]\n",
                        compareBitsPtr[ii],hwValue,shadowValue);
                }

                UTF_VERIFY_EQUAL1_PARAM_MAC(hwValue, shadowValue,devNum);
            }
        }
    }

    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, devNum);
}

static CPSS_PX_TABLE_ENT debugSpecificTable = CPSS_PX_TABLE_LAST_E;
/* test the shadow of the CPSS :
    set entries in the HW table and check that read from HW match read from shadow.

*/
static void cpssPxDiagDataIntegrityTables_shadowTest_operation(
    IN GT_U8               devNum,
    IN GT_U32       operation
)
{
    GT_STATUS   rc;
    CPSS_PX_TABLE_ENT hwTableType;
    GT_U32     maxTableSize;
    GT_U32  index;
    GT_U32  mainOperation = operation;
    GT_U32  origOperation = operation;

    if(origOperation == CHECK_SHADOW_WITH_SELF_COMPARE)
    {
        mainOperation = CHECK_SHADOW;
    }

    for(hwTableType = 0 ; hwTableType < CPSS_PX_TABLE_LAST_E; hwTableType++)
    {
        if (hwTableType == CPSS_PX_TABLE_PHA_FW_IMAGE_E)
        {
            continue;
        }
        if(debugSpecificTable != CPSS_PX_TABLE_LAST_E &&
           debugSpecificTable != hwTableType)
        {
            /* the debug process not need this table */
            continue;
        }

        if(GT_FALSE ==
            prvCpssPxDiagDataIntegrityTableCheckIsDevSupportHwTable(devNum,hwTableType,
            0))
        {
            /* the device not supports this HW table */
            continue;
        }

        if(CPSS_PX_SHADOW_TYPE_NONE_E ==
            prvCpssPxDiagDataIntegrityTableCheckIsDevSupportHwTableShadow(devNum,hwTableType))
        {
            /* the device not supports shadow for the table */
            continue;
        }

        /* get number of entries in the table */
        rc = prvCpssPxDiagDataIntegrityTableHwMaxIndexGet(devNum,hwTableType,&maxTableSize);
        if(rc != GT_OK)
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, devNum);
        }

        if(origOperation == CHECK_SHADOW_WITH_SELF_COMPARE)
        {
            index = 0;
            cpssPxDiagDataIntegrityTables_shadowTest_entry(devNum,hwTableType,index,origOperation);
        }

        for(index = 0 ; index < maxTableSize; index++)
        {
            if(GT_FALSE ==
                prvCpssPxDiagDataIntegrityTableCheckIsDevSupportHwTable(devNum,hwTableType,
                index))
            {
                /* the HW table not support this index */
                continue;
            }

            cpssPxDiagDataIntegrityTables_shadowTest_entry(devNum,hwTableType,index,mainOperation);

            if(utfErrorCountGet())
            {
                didError_shadowTest_onDemand = GT_TRUE;
                break;
            }
        }

        if(operation == CHECK_SHADOW_WITH_SELF_COMPARE)
        {
            index = 1;/* non-zero ... to restore values in index 0 */
            cpssPxDiagDataIntegrityTables_shadowTest_entry(devNum,hwTableType,index,origOperation);
        }
    }
}
UTF_TEST_CASE_MAC(cpssPxDiagDataIntegrityTables_shadowTest)
{
    GT_U8   devNum;
    GT_U32  devCounter = 0;
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        if(prvTgfResetModeGet() == GT_FALSE)
        {
            /* the 'fix' function currently setting 'dummy' values to the tables */
            /* that cause bad configurations ... after the test we need 'HW reset'*/
            /* but if we can't reset ... we should not run the test */
            SKIP_TEST_MAC;
        }

        cpssOsPrintf("device[%d] started \n", devNum);

        devCounter++;
        /* write to all tables to all entries*/
        cpssPxDiagDataIntegrityTables_shadowTest_operation(devNum,WRITE_ONLY);
        /* read from all tables on all entries and compare to shadow */
        cpssPxDiagDataIntegrityTables_shadowTest_operation(devNum,CHECK_SHADOW);

        cpssOsPrintf("device[%d] finished \n", devNum);
    }

    if(0 == devCounter)
    {
        /* state that the test skipped */
        SKIP_TEST_MAC;
    }

}

/* function needed because MACRO is good for function that return 'void'
but not for function that return GT_BOOL like cpssPxDiagDataIntegrityTables_shadowTest_onDemand */
static void cpssPxDiagDataIntegrityTables_shadowTest_onDemand_getFirstDev_restDevIterator(OUT GT_U8 *devNumPtr)
{
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(devNumPtr, UTF_NONE_FAMILY_E);
}

/* return indication of error :
    GT_TRUE  - error
    GT_FALSE - no error

*/
GT_BOOL cpssPxDiagDataIntegrityTables_shadowTest_onDemand(void)
{
    GT_U8   devNum;

    utfErrorCountGet();/*reset the error count*/
    didError_shadowTest_onDemand = GT_FALSE;

    if(GT_TRUE == prvUtfIsGmCompilation())
    {
        return didError_shadowTest_onDemand;
    }

    cpssPxDiagDataIntegrityTables_shadowTest_onDemand_getFirstDev_restDevIterator(&devNum);

    /* run only on first device that iterator returns */
    if(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        cpssOsPrintf("device[%d] started to test shadow \n", devNum);

        /* check that all tables in HW with same values as in shadow */
        cpssPxDiagDataIntegrityTables_shadowTest_operation(devNum,CHECK_SHADOW_WITH_SELF_COMPARE);

        cpssOsPrintf("device[%d] finished to test shadow \n", devNum);

        if(didError_shadowTest_onDemand == GT_TRUE)
        {
            /* fix errors so the next tests will not fail on the same tables that already found */
            prvCpssPxDiagDataIntegrityShadowAndHwSynch(devNum);
        }
    }

    return didError_shadowTest_onDemand;
}

GT_BOOL cpssPxDiagDataIntegrityTables_shadowTest_onDemand_specificTable
(
    IN GT_U8   devNum,
    IN CPSS_PX_TABLE_ENT table
)
{
    didError_shadowTest_onDemand = GT_FALSE;

    if(GT_TRUE == prvUtfIsGmCompilation())
    {
        return didError_shadowTest_onDemand;
    }

    debugSpecificTable = table;

    /* run the check */
    cpssPxDiagDataIntegrityTables_shadowTest_operation(devNum,CHECK_SHADOW_WITH_SELF_COMPARE);

    /* restore 'no debug' */
    debugSpecificTable = CPSS_PX_TABLE_LAST_E;


    return didError_shadowTest_onDemand;
}

#ifndef ASIC_SIMULATION


#define PRV_UTF_ERROR_INJECT_LOOPS_PER_MEM 3
UTF_TEST_CASE_MAC(cpssPxDiagDataIntegrityHwTablesInfoGetTest)
{
/*
    ITERATE_DEVICES (Pipe)
*/
    GT_STATUS           st = GT_OK;
    GT_U8               dev;
    CPSS_PX_TABLE_ENT tableType;
    CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT        memType;
    CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errType;
    GT_U32                                          entryIndex;
    GT_U32                                          expectedEntryIndex;
    GT_U32                                          i;
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC              locationInfo;
    GT_U32                                          nextEntryIndex;
    GT_BOOL                                         wasWrapAround;
    GT_U32                                          maxEntries;
    GT_U32                                          scanEntries;
    GT_U32                                          *tableEntryPtr = &tableEntry[0];
    PX_DATA_INTEGRITY_EVENT_CB_FUNC                 *savedDataIntegrityHandler;
    GT_U32                                          tblsCount;
    CPSS_PX_TABLE_ENT                               expTableType;
    GT_U32                                          readEntryBuf[MAX_ENTRY_SIZE_CNS];

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    GM_NOT_SUPPORT_THIS_TEST_MAC

    cpssOsMemSet(tableEntryPtr, 0xFF, sizeof(tableEntry));

    /* save data integrity error callback. */
    savedDataIntegrityHandler = pxDataIntegrityEventIncrementFunc;

    /* Clear captured events */
    prvCpssPxDiagDataIntegrityEventsClear();

    /* AUTODOC: set own callback */
    pxDataIntegrityEventIncrementFunc = hwInfoEventErrorHandlerFunc;

    locationInfo.type        = CPSS_PX_LOCATION_HW_INDEX_INFO_TYPE;

    /* AUTODOC:  go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        tblsCount = 0;

        /* AUTODOC: loop over all memories and inject errors */
        for(tableType = CPSS_PX_TABLE_BMA_PORT_MAPPING_E; tableType < CPSS_PX_TABLE_LAST_E; tableType++)
        {

            st = prvUtfDataIntegrityErrorInjectionTablesSkipCheck(dev, tableType, &memType, &errType, &maxEntries);

            if (st != GT_OK)
            {
                if (traceEvents)
                {
                    switch (st)
                    {
                        case GT_NOT_FOUND:
                            cpssOsPrintf(" table %d - not found in DB\r\n",tableType);
                            break;
                        case GT_NOT_SUPPORTED:
                            cpssOsPrintf(" table %d - not exists \r\n", tableType);
                            break;
                        case GT_NOT_IMPLEMENTED:
                            cpssOsPrintf(" table %d - not used in test\r\n", tableType);
                            break;
                        case GT_BAD_VALUE:
                            cpssOsPrintf(" table %d - not protected table\r\n", tableType);
                            break;
                    }
                }

                continue;
            }

            if (traceEvents)
            {
                cpssOsPrintf("Test table %d\n", tableType);
            }
            tblsCount++;

            if (selectedTableType != 0xFFFFFFFF)
            {
                PRV_UTF_LOG0_MAC("cpssPxDiagDataIntegrityErrorCountEnableSet:\n");
            }
            else
            {
                cpssOsTimerWkAfter(5);
            }

            locationInfo.info.hwEntryInfo.hwTableType = tableType;

            /* AUTODOC: enable error counter */
            st = cpssPxDiagDataIntegrityErrorCountEnableSet(dev, &locationInfo, errType, GT_TRUE);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            if (selectedTableType != 0xFFFFFFFF)
            {
                PRV_UTF_LOG0_MAC("cpssPxDiagDataIntegrityEventMaskSet:\n");
            }

            /* AUTODOC: unmask interrupt for DFX memory */
            st = cpssPxDiagDataIntegrityEventMaskSet(dev, &locationInfo, errType, CPSS_EVENT_UNMASK_E);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            for (i = 0; i < PRV_UTF_ERROR_INJECT_LOOPS_PER_MEM; i++)
            {
                if (tableType == CPSS_PX_TABLE_PHA_FW_IMAGE_E)
                {
                    /*address should be from the predefined list - since we don't want to stuck Tensilica:
                      */
                    entryIndex = prvDataIntegrityFwImageTestAddressesList[i];
                }
                else
                {
                    /* AUTODOC: calculate entry index and write table entry field */
                    entryIndex = (PRV_UTF_ERROR_INJECT_LOOPS_PER_MEM - i - 1) * (maxEntries - 1) / (PRV_UTF_ERROR_INJECT_LOOPS_PER_MEM - 1);
                }

                /* AUTODOC: check entry index for HW table */
                if(GT_FALSE == prvCpssPxDiagDataIntegrityTableCheckIsDevSupportHwTable(dev, tableType, entryIndex))
                {
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, GT_FAIL, "Bad index %d for table %ds\n", entryIndex, tableType);
                    continue;
                }


                if (selectedTableType != 0xFFFFFFFF)
                {
                    PRV_UTF_LOG0_MAC("cpssPxDiagDataIntegrityErrorInjectionConfigSet:\n");
                }
                else
                {
                    cpssOsTimerWkAfter(5);
                }

                /* AUTODOC: enable error injection */
                st = cpssPxDiagDataIntegrityErrorInjectionConfigSet(dev, &locationInfo, CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E, GT_TRUE);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                if (selectedTableType != 0xFFFFFFFF)
                {
                    PRV_UTF_LOG1_MAC("prvCpssPxPortGroupWriteTableEntry1 entryIndex %x\n",entryIndex);
                }

                /* AUTODOC: write table entry */
                if (tableType == CPSS_PX_TABLE_CNC_0_COUNTERS_E ||
                    tableType == CPSS_PX_TABLE_CNC_1_COUNTERS_E)
                {
                    /* It's prohibited to write to CNC memory. Device stuck.
                       Need to use read operation instead. The read operation
                       writes to CNC memory also because it's clear on read.*/
                    st = prvCpssPxPortGroupReadTableEntry(
                        dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                        tableType, entryIndex, &readEntryBuf[0]);
                }
                else if (tableType == CPSS_PX_TABLE_PHA_FW_IMAGE_E)
                {

                    st = prvFwImageTableEntryWrite(dev,entryIndex);

                }

                else
                {
                    st = prvCpssPxPortGroupWriteTableEntry(
                        dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                        tableType, entryIndex, tableEntryPtr);
                }
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                if (selectedNumOfEntries != 0xFFFFFFFF)
                {
                    /* AUTODOC: scan only written entry */
                    scanEntries = selectedNumOfEntries;
                    locationInfo.info.hwEntryInfo.hwTableEntryIndex = entryIndex;
                }
                else
                {
                    /* AUTODOC: scan full table */
                    locationInfo.info.hwEntryInfo.hwTableEntryIndex = 0;
                    scanEntries = maxEntries;

                }

                if (selectedTableType != 0xFFFFFFFF)
                {
                    PRV_UTF_LOG0_MAC("cpssPxDiagDataIntegrityTableScan:\n");
                }
                else
                {
                    cpssOsTimerWkAfter(5);
                }

                /* AUTODOC: scan whole table */
                st = cpssPxDiagDataIntegrityTableScan(dev, &locationInfo, scanEntries,
                                                        &nextEntryIndex, &wasWrapAround);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


                /* AUTODOC: write table entry again to reset erroneous bit.(it is done as FF bit)
                 * If don't do this  next scenario is likely to happen for memories with disabled auto-correction:
                 * Inject single error into entry #0. Scan full table. Error is captured
                 * for entry #0. Good.
                 ** Inject single error into entry #100. scan full table. Two errors are
                 * captured: for entries #100 (good), #0 - from previous iteration (bad) */
                if (tableType == CPSS_PX_TABLE_PHA_FW_IMAGE_E)
                {

                    st = prvFwImageTableEntryWrite(dev,entryIndex);

                }

                else if ((tableType != CPSS_PX_TABLE_CNC_0_COUNTERS_E) && /* It's prohibited to write to CNC memory in BC3 and above. Device stuck. */
                   (tableType != CPSS_PX_TABLE_CNC_1_COUNTERS_E))
                {
                   st = prvCpssPxPortGroupWriteTableEntry(
                        dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                        tableType, entryIndex, tableEntryPtr);
                }
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                /* determine table entry index which we expect to find in DataIntegrity event  */
                expectedEntryIndex = entryIndex;
                expTableType = tableType;
                /* AUTODOC: sleep 20 mSec */
                cpssOsTimerWkAfter(20);
                 /* AUTODOC: check specific event for current memory */
                st = prvCpssPxDiagDataIntegrityEventHwInfoDbCheck(
                    errType, memType, expTableType, expectedEntryIndex, PRV_DATA_INTEGRITY_EVENT_MATCH_TYPE_ANY_E);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st,
                                            locationInfo.info.hwEntryInfo.hwTableType,
                                            locationInfo.info.hwEntryInfo.hwTableEntryIndex);

                prvCpssPxDiagDataIntegrityEventsClear();

                if (selectedTableType != 0xFFFFFFFF)
                {
                    PRV_UTF_LOG0_MAC("prvCpssPxDiagDataIntegrityEventHwInfoDbCheck:\n");
                }
                else
                {
                    cpssOsTimerWkAfter(5);
                }

            }

            /* AUTODOC: disable error injection */
            st = cpssPxDiagDataIntegrityErrorInjectionConfigSet(dev, &locationInfo, CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E, GT_FALSE);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* AUTODOC: Mask DFX memory interrupt */
            st = cpssPxDiagDataIntegrityEventMaskSet(dev, &locationInfo, errType, CPSS_EVENT_MASK_E);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        }
        if (selectedTableType != 0xFFFFFFFF)
        {
            PRV_UTF_LOG2_MAC("Device %d Number tested HW Tables %d:\n", dev, tblsCount);
        }
    }

    /* AUTODOC: Restore pxpxhDataIntegrityEventIncrementFunc */
    pxDataIntegrityEventIncrementFunc = savedDataIntegrityHandler;
}
/* inject error
   check that error information is as expected
   fix the entry
   verify that data in the fixed entry is correct*/
UTF_TEST_CASE_MAC(cpssPxDiagDataIntegrityFwImageTablesFixTest)
{
/*
    ITERATE_DEVICES (Pipe)
*/
    GT_STATUS           st = GT_OK;
    GT_U8               dev;
    CPSS_PX_TABLE_ENT tableType;
    CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT        memType;
    CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errType;
    GT_U32                                          entryIndex;
    GT_U32                                          expectedEntryIndex;
    GT_U32                                          i,j;
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC              locationInfo;
    GT_U32                                          nextEntryIndex;
    GT_BOOL                                         wasWrapAround;
    GT_U32                                          maxEntries;
    GT_U32                                          scanEntries;
    GT_U32                                          readFwEntry[4],readEntryAfterFix[4];
    GT_U32                                          *tableEntryPtr = &tableEntry[0];
    PX_DATA_INTEGRITY_EVENT_CB_FUNC                 *savedDataIntegrityHandler;
    CPSS_PX_TABLE_ENT                               expTableType;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    GM_NOT_SUPPORT_THIS_TEST_MAC

    cpssOsMemSet(tableEntryPtr, 0xFF, sizeof(tableEntry));

    /* save data integrity error callback. */
    savedDataIntegrityHandler = pxDataIntegrityEventIncrementFunc;

    /* Clear captured events */
    prvCpssPxDiagDataIntegrityEventsClear();

    /* AUTODOC: set own callback */
    pxDataIntegrityEventIncrementFunc = hwInfoEventErrorHandlerFunc;

    locationInfo.type        = CPSS_PX_LOCATION_HW_INDEX_INFO_TYPE;

    /* AUTODOC:  go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        tableType = CPSS_PX_TABLE_PHA_FW_IMAGE_E;
        /* define memType , errorType and maxEntries*/
        st = prvUtfDataIntegrityErrorInjectionTablesSkipCheck(dev, tableType, &memType, &errType, &maxEntries);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        if (selectedTableType != 0xFFFFFFFF)
        {
            PRV_UTF_LOG0_MAC("cpssPxDiagDataIntegrityErrorCountEnableSet:\n");
        }
        else
        {
            cpssOsTimerWkAfter(5);
        }

        locationInfo.info.hwEntryInfo.hwTableType = tableType;

        /* AUTODOC: enable error counter */
        st = cpssPxDiagDataIntegrityErrorCountEnableSet(dev, &locationInfo, errType, GT_TRUE);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        if (selectedTableType != 0xFFFFFFFF)
        {
            PRV_UTF_LOG0_MAC("cpssPxDiagDataIntegrityEventMaskSet:\n");
        }


        /* AUTODOC: unmask interrupt for DFX memory */
        st = cpssPxDiagDataIntegrityEventMaskSet(dev, &locationInfo, errType, CPSS_EVENT_UNMASK_E);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        for (i = 0; i < PRV_UTF_ERROR_INJECT_LOOPS_PER_MEM; i++)
        {
            entryIndex = prvDataIntegrityFwImageTestAddressesList[i];
            /* read the entry*/
            prvFwImageTableEntryRead(dev,entryIndex,0,readFwEntry);
            if (selectedTableType != 0xFFFFFFFF)
            {
                PRV_UTF_LOG4_MAC("prvFwImageTableEntryRead readFwEntry %x %x %x %x\n",readFwEntry[0],readFwEntry[1],readFwEntry[2],readFwEntry[3]);
            }
            if (selectedTableType != 0xFFFFFFFF)
            {
                PRV_UTF_LOG0_MAC("cpssPxDiagDataIntegrityErrorInjectionConfigSet:\n");
            }
            else
            {
                cpssOsTimerWkAfter(5);
            }

            /* AUTODOC: enable error injection */
            st = cpssPxDiagDataIntegrityErrorInjectionConfigSet(dev, &locationInfo, CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E, GT_TRUE);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            if (selectedTableType != 0xFFFFFFFF)
            {
                PRV_UTF_LOG1_MAC("prvCpssPxPortGroupWriteTableEntry1 entryIndex %x\n",entryIndex);
            }

            st = prvFwImageTableEntryWrite(dev,entryIndex);

            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            if (selectedNumOfEntries != 0xFFFFFFFF)
            {
                /* AUTODOC: scan only written entry */
                scanEntries = selectedNumOfEntries;
                locationInfo.info.hwEntryInfo.hwTableEntryIndex = entryIndex;
            }
            else
            {
                /* AUTODOC: scan full table */
                locationInfo.info.hwEntryInfo.hwTableEntryIndex = 0;
                scanEntries = maxEntries;

            }

            if (selectedTableType != 0xFFFFFFFF)
            {
                PRV_UTF_LOG0_MAC("cpssPxDiagDataIntegrityTableScan:\n");
            }
            else
            {
                cpssOsTimerWkAfter(5);
            }

            /* AUTODOC: scan whole table*/
            st = cpssPxDiagDataIntegrityTableScan(dev, &locationInfo, scanEntries,
                                                    &nextEntryIndex, &wasWrapAround);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


            /* AUTODOC: write table entry again to reset erroneous bit.(it is done as FF bit) */
            st = prvFwImageTableEntryWrite(dev,entryIndex);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* determine table entry index which we expect to find in DataIntegrity event  */
            expectedEntryIndex = entryIndex;
            expTableType = tableType;
            /* AUTODOC: sleep 20 mSec */
            cpssOsTimerWkAfter(20);
            /* AUTODOC: check specific event for current memory */
            st = prvCpssPxDiagDataIntegrityEventHwInfoDbCheck(errType, memType,
                                                              expTableType, expectedEntryIndex, PRV_DATA_INTEGRITY_EVENT_MATCH_TYPE_ANY_E);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st,
                                            locationInfo.info.hwEntryInfo.hwTableType,
                                            locationInfo.info.hwEntryInfo.hwTableEntryIndex);

            locationInfo.info.hwEntryInfo.hwTableEntryIndex = entryIndex;
            locationInfo.info.hwEntryInfo.hwTableType = tableType;
            st = cpssPxDiagDataIntegrityTableEntryFix(dev,&locationInfo);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            prvFwImageTableEntryRead(dev,entryIndex,0,readEntryAfterFix);
            if (selectedTableType != 0xFFFFFFFF)
            {
                PRV_UTF_LOG4_MAC("prvFwImageTableEntryRead readFwEntry after fix %x %x %x %x\n",
                             readEntryAfterFix[0],readEntryAfterFix[1],readEntryAfterFix[2],readEntryAfterFix[3]);
            }
            for (j=0; j<4; j++)
            {
                UTF_VERIFY_EQUAL0_PARAM_MAC(readFwEntry[j], readEntryAfterFix[j]);
            }

            prvCpssPxDiagDataIntegrityEventsClear();

            if (selectedTableType != 0xFFFFFFFF)
            {
                    PRV_UTF_LOG0_MAC("prvCpssPxDiagDataIntegrityEventHwInfoDbCheck:\n");
            }
            else
            {
                    cpssOsTimerWkAfter(5);
            }

        }

        /* AUTODOC: disable error injection */
        st = cpssPxDiagDataIntegrityErrorInjectionConfigSet(dev, &locationInfo, CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E, GT_FALSE);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* AUTODOC: Mask DFX memory interrupt */
        st = cpssPxDiagDataIntegrityEventMaskSet(dev, &locationInfo, errType, CPSS_EVENT_MASK_E);

        cpssOsTimerWkAfter(10);
    }

    /* AUTODOC: Restore pxpxhDataIntegrityEventIncrementFunc */
    pxDataIntegrityEventIncrementFunc = savedDataIntegrityHandler;
}


/* The UT needs to ensure that in CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_MULTIPLE_E mode
   the corrupted data is written to the referenced address
 */
UTF_TEST_CASE_MAC(cpssPxDiagDataIntegrityMultiErrorInject)
{
/*
  Iterate device (Pipe) ports:
      1) get an entry from CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_LIMITS_DP0_E table.
      2) enable error injection for the table.
      3) write the stored entry into the table to trigger error injection.
      4) read the entry from HW and compare it with stored value
*/

    GT_U8                                dev;
    GT_PORT_NUM                          index;
    GT_STATUS                            st;
    GT_U32                               hwEntry[MAX_ENTRY_SIZE_CNS];
    CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT        memType;
    CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errType;
    GT_U32                                          maxEntries;
    GT_U32                               errHwEntry[MAX_ENTRY_SIZE_CNS];
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC location;
    CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_ENT injectMode = CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_MULTIPLE_E;
    CPSS_PX_TABLE_ENT tableType;


    cpssOsMemSet(&location, 0, sizeof(location));
    location.type                         = CPSS_PX_LOCATION_HW_INDEX_INFO_TYPE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(tableType = 0; tableType < CPSS_PX_TABLE_LAST_E; tableType++)
        {
            location.info.hwEntryInfo.hwTableType = tableType;

            st = prvUtfDataIntegrityErrorInjectionTablesSkipCheck(dev, tableType, &memType, &errType, &maxEntries);

            if (st != GT_OK)
            {
                if (traceEvents)
                {
                    switch (st)
                    {
                        case GT_NOT_FOUND:
                            cpssOsPrintf(" table %d - not found in DB\r\n",tableType);
                            break;
                        case GT_NOT_SUPPORTED:
                            cpssOsPrintf(" table %d - not exists \r\n", tableType);
                            break;
                        case GT_NOT_IMPLEMENTED:
                            cpssOsPrintf(" table %d - not used in test\r\n", tableType);
                            break;
                        case GT_BAD_VALUE:
                            cpssOsPrintf(" table %d - not protected table\r\n", tableType);
                            break;
                    }
                }

                continue;
            }
            if (tableType == CPSS_PX_TABLE_CNC_0_COUNTERS_E ||
                tableType == CPSS_PX_TABLE_CNC_1_COUNTERS_E ||
                tableType == CPSS_PX_TABLE_PHA_FW_IMAGE_E)
            {
                continue;
            }
            if (traceEvents)
            {
                cpssOsPrintf("Test table %d\n", tableType);
            }

            for (index = 0;index <16; index++)
            {
                cpssOsMemSet(&hwEntry,     0, sizeof(hwEntry));
                cpssOsMemSet(&errHwEntry,  0, sizeof(errHwEntry));
                /*st = cpssPxIngressPortRedirectSet(dev, index,0x5555);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);*/

                /* AUTODOC: 1. get the HW table entry */
                st = prvCpssPxReadTableEntry(dev, tableType, index, hwEntry);
                if (traceEvents)
                {
                    PRV_UTF_LOG2_MAC ("!@cpssPxDiagDataIntegrityMultiErrorInject hwEntry[] = %x, index= %d\n",hwEntry[0],index);
                }
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, tableType, index);
                /* AUTODOC: 2. enable error injection */
                st = cpssPxDiagDataIntegrityErrorInjectionConfigSet(dev, &location, injectMode, GT_TRUE);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
                cpssOsTimerWkAfter(10);
                /* AUTODOC: 3. write the stored entry into the table to trigger error injection */
                st = prvCpssPxWriteTableEntry(dev, tableType, index, hwEntry);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                 /* AUTODOC: disable error injection */
                st = cpssPxDiagDataIntegrityErrorInjectionConfigSet(dev, &location, injectMode, GT_FALSE);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                /* AUTODOC: 4. get the HW table entry (with error) */
                st = prvCpssPxReadTableEntry(dev, tableType, index, errHwEntry);
                if (traceEvents)
                {
                    PRV_UTF_LOG1_MAC ("cpssPxDiagDataIntegrityMultiErrorInject errHwEntry[0] = %x\n",errHwEntry[0]);
                }
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);


                /* DFX engine can choose ECC code bits instead of table entry bits
                   to make them erroneous. Such injection is not detected by entry reading.
                   The problem can't be solved by executing error injection several times
                   (it seems errorneous bit number is not random). Just try to choose
                   another table (manually) */
                UTF_VERIFY_NOT_EQUAL1_STRING_MAC(hwEntry[0], errHwEntry[0],
                    "An erroneous bit is not found after error injection, index %d\n", index);

                cpssOsTimerWkAfter(10);

            }
        }
    }
}

#endif
/*-----------------------------------------------------------------------------*/
/*
 * Configuration of cpssPxDiagDataIntegrity suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssPxDiagDataIntegrityTables)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxDiagDataIntegrityTableScan)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxDiagDataIntegrityTableScan_1)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxDiagDataIntegrityTableEntryFix)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxDiagDataIntegrityShadowEntryInfoGet)

#ifndef ASIC_SIMULATION
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxDiagDataIntegrityHwTablesErrorInjectionTest)
#endif

    UTF_SUIT_DECLARE_TEST_MAC(cpssPxDiagDataIntegrityTables_shadowTest)
#ifndef ASIC_SIMULATION
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxDiagDataIntegrityHwTablesInfoGetTest)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxDiagDataIntegrityMultiErrorInject)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxDiagDataIntegrityFwImageTablesFixTest)
#endif
    /* should be last to clean after previous tests */
    /*UTF_SUIT_DECLARE_TEST_MAC(cpssPxDiagDataIntegrityTables_cleanup_withSystemReset)*/

UTF_SUIT_END_TESTS_MAC(cpssPxDiagDataIntegrityTables)

