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
* @file cpssDxChDiagDataIntegrityTablesUT.c
*
* @brief Unit tests for cpssDxChDiagDataIntegrityTables
* CPSS DXCH Diagnostic Tables API
*
* @version   1
********************************************************************************
*/
#include <cpss/dxCh/dxChxGen/diag/cpssDxChDiagDataIntegrityTables.h>
#include <cpss/dxCh/dxChxGen/diag/private/prvCpssDxChDiagDataIntegrityMainMappingDb.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>
#include <cpss/dxCh/dxChxGen/tunnel/cpssDxChTunnel.h>
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIp.h>
#include <cpss/dxCh/dxCh3/policer/private/prvCpssDxCh3Policer.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpss/dxCh/dxChxGen/lpm/cpssDxChLpm.h>

/* the number of entries allocated for IPLR0 to use */
#define IPLR0_SIZE_MAC(devNum) \
    PRV_CPSS_DXCH_PP_MAC(devNum)->policer.memSize[CPSS_DXCH_POLICER_STAGE_INGRESS_0_E]

/* the number of entries allocated for IPLR1 to use */
#define IPLR1_SIZE_MAC(devNum) \
    PRV_CPSS_DXCH_PP_MAC(devNum)->policer.memSize[CPSS_DXCH_POLICER_STAGE_INGRESS_1_E]

/* the number of entries allocated for EPLR to use */
#define EPLR_SIZE_MAC(devNum) \
    PRV_CPSS_DXCH_PP_MAC(devNum)->policer.memSize[CPSS_DXCH_POLICER_STAGE_EGRESS_E]

/* max number of words in entry */
#define MAX_ENTRY_SIZE_CNS   64

/* debug flag to open trace of events */
static GT_U32 traceEvents = 0;

/* Table name for print purpouse */
GT_U8 strNameBuffer[120] = "";
GT_U8 strNameBuffer1[120] = "";

GT_U32 timeOut = 15;
GT_U32 entryValue[4];

typedef struct
{
    GT_U16 eventCounterArr[BIT_17];
    GT_U16 memTypeArr[BIT_17];
    GT_U8  origPortGroupId[BIT_17];
} CMD_DXCH_DIAG_DATA_INTEGRITY_COUNTER_DB_STC;

#ifdef IMPL_GALTIS
extern CMD_DXCH_DIAG_DATA_INTEGRITY_COUNTER_DB_STC * wrCpssDxChDiagDataIntegrityCountersDbGet(GT_VOID);
#endif

#ifdef CPSS_LOG_ENABLE
extern PRV_CPSS_ENUM_STRING_VALUE_PAIR_STC prvCpssLogEnum_map_CPSS_DXCH_TABLE_ENT[];
extern GT_U32                              prvCpssLogEnum_size_CPSS_DXCH_TABLE_ENT;
extern PRV_CPSS_ENUM_STRING_VALUE_PAIR_STC prvCpssLogEnum_map_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT[];
extern GT_U32                              prvCpssLogEnum_size_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT;
extern PRV_CPSS_ENUM_STRING_VALUE_PAIR_STC prvCpssLogEnum_map_CPSS_DXCH_LOGICAL_TABLE_ENT[];
extern GT_U32                              prvCpssLogEnum_size_CPSS_DXCH_LOGICAL_TABLE_ENT;
#define PRV_CPSS_DATA_INTEGRITY_ENUM_STRING_GET_MAC(tableType, tableEnum, strBuffer) \
    PRV_CPSS_LOG_ENUM_STRING_GET_MAC(tableType, tableEnum, strBuffer)

#else

#define PRV_CPSS_DATA_INTEGRITY_ENUM_STRING_GET_MAC(tableType, tableEnum, strBuffer)
#endif

#define SIP5_TABLE_CNC_COUNTERS_CASES_MAC            \
         CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E:      \
    case CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 1:  \
    case CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 2:  \
    case CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 3:  \
    case CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 4:  \
    case CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 5:  \
    case CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 6:  \
    case CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 7:  \
    case CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 8:  \
    case CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 9:  \
    case CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 10: \
    case CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 11: \
    case CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 12: \
    case CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 13: \
    case CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 14: \
    case CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 15: \
    case CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 16: \
    case CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 17: \
    case CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 18: \
    case CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 19: \
    case CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 20: \
    case CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 21: \
    case CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 22: \
    case CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 23: \
    case CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 24: \
    case CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 25: \
    case CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 26: \
    case CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 27: \
    case CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 28: \
    case CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 29: \
    case CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 30: \
    case CPSS_DXCH_SIP5_TABLE_CNC_31_COUNTERS_E


/* check if HW table supports the port groups BMP */
static GT_STATUS isHwTableSupportPortGroupsBmp
(
    IN GT_U8 devNum ,
    IN GT_U32 portGroupsBmp,
    IN CPSS_DXCH_TABLE_ENT hwTableType
)
{
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_TABLE_MAC(devNum,portGroupsBmp,hwTableType);

    return GT_OK;
}

/* check if Logical table supports the port groups BMP */
static GT_STATUS isLogicalTableSupportPortGroupsBmp
(
    IN GT_U8 devNum ,
    IN GT_U32 portGroupsBmp,
    IN CPSS_DXCH_LOGICAL_TABLE_ENT logicalTableType
)
{
    return prvCpssDxChDiagDataIntegrityLogicalTablePortGroupsBmpCheck(devNum,portGroupsBmp,logicalTableType);
}

static CPSS_DXCH_TABLE_ENT selectedTableType = 0xFFFFFFFF;
static GT_U32 selectedNumOfEntries = 0xFFFFFFFF;

GT_VOID prvDebugTableTypeSet(CPSS_DXCH_TABLE_ENT tableType, GT_U32 numEntries, GT_U32 timeOutVal)
{
    PRV_CPSS_DATA_INTEGRITY_ENUM_STRING_GET_MAC(tableType, CPSS_DXCH_TABLE_ENT, strNameBuffer);
    selectedTableType = tableType;
    selectedNumOfEntries = numEntries;
    timeOut = timeOutVal;
    cpssOsPrintf("%s (%d entries %d mSec time out)\r\n", strNameBuffer, selectedNumOfEntries, timeOut);
    return;
}

#ifndef ASIC_SIMULATION
/* maximal number of events callback function can store */
#define PRV_MAX_CALLBACK_EVENTS_COUNT_CNS 40


static GT_U32 tableEntry[MAX_ENTRY_SIZE_CNS] = {0xFF};
typedef GT_STATUS DXCH_DATA_INTEGRITY_EVENT_CB_FUNC
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC     *eventPtr
);

extern DXCH_DATA_INTEGRITY_EVENT_CB_FUNC    *dxChDataIntegrityEventIncrementFunc;


typedef enum {
    PRV_DATA_INTEGRITY_EVENT_MATCH_TYPE_ALL_E,
    PRV_DATA_INTEGRITY_EVENT_MATCH_TYPE_ANY_E,
    PRV_DATA_INTEGRITY_EVENT_MATCH_TYPE_THE_ONLY_E
} PRV_DATA_INTEGRITY_EVENT_MATCH_TYPE_ENT;


/* Data structure holds HW and RAM info from event */
typedef struct
{
    CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   eventsType;
    CPSS_DXCH_RAM_INDEX_INFO_STC                    eventRamEntryInfo;
    CPSS_DXCH_HW_INDEX_INFO_STC                     eventHwEntryInfo;
    CPSS_DXCH_LOGICAL_INDEX_INFO_STC                eventLogicalEntryInfo;
} PRV_DXCH_DATA_INTEGRITY_EVENT_INFO_STC;



typedef struct
{
    GT_U32 eventsCount;
    PRV_DXCH_DATA_INTEGRITY_EVENT_INFO_STC eventsArr[PRV_MAX_CALLBACK_EVENTS_COUNT_CNS];
} DXCH_DATA_INTEGRITY_EVENTS_STC;

DXCH_DATA_INTEGRITY_EVENTS_STC occuredEvents = {0}; /* .eventsCount==0 */

#define DXCH_DATA_INTEGRITY_EVENTS_DB_LOCK() CPSS_ZERO_LEVEL_API_LOCK_MAC
#define DXCH_DATA_INTEGRITY_EVENTS_DB_UNLOCK() CPSS_ZERO_LEVEL_API_UNLOCK_MAC


/* the struct to store data integrity event specific info.
   Filled by event handler callback function */
typedef struct
{
    GT_U32 allEventsCount;      /* counter of all events */
    GT_U32 tsEventsCount;       /* counter of events related to Tunnel Start/ARP/NAT HW table */
    CPSS_DXCH_LOGICAL_INDEX_INFO_STC  eventsInfo[PRV_MAX_CALLBACK_EVENTS_COUNT_CNS];
    CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   eventsType[PRV_MAX_CALLBACK_EVENTS_COUNT_CNS];
} PRV_DXCH_DATA_INTEGRITY_TS_EVENTS_STC;

PRV_DXCH_DATA_INTEGRITY_TS_EVENTS_STC tunnelStartEvents;

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
    IN CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC  *eventPtr

)
{

    PRV_DXCH_DATA_INTEGRITY_EVENT_INFO_STC *curEventPtr;
    CPSS_NULL_PTR_CHECK_MAC(eventPtr);
    if(dev >= PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        /* check device out of range */
        return GT_BAD_PARAM;
    }

    DXCH_DATA_INTEGRITY_EVENTS_DB_LOCK();

    if (occuredEvents.eventsCount >=  PRV_MAX_CALLBACK_EVENTS_COUNT_CNS)
    {
        PRV_UTF_LOG0_MAC("Maximum number of Data Integrity events is exceeded!\n");
        DXCH_DATA_INTEGRITY_EVENTS_DB_UNLOCK();
        return GT_FAIL;
    }

    if (traceEvents > 1)
    {
        PRV_CPSS_DATA_INTEGRITY_ENUM_STRING_GET_MAC(eventPtr->location.hwEntryInfo.hwTableType,
                                                    CPSS_DXCH_TABLE_ENT, strNameBuffer);
        PRV_CPSS_DATA_INTEGRITY_ENUM_STRING_GET_MAC(eventPtr->location.ramEntryInfo.memType,
                                                    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT, strNameBuffer1);
        PRV_UTF_LOG0_MAC("======DATA INTEGRITY ERROR EVENT======\n");
        PRV_UTF_LOG1_MAC("Event type        :  %d\n",   eventPtr->eventsType);
        PRV_UTF_LOG1_MAC("HW entry index    :  %d\n",   eventPtr->location.hwEntryInfo.hwTableEntryIndex);
        PRV_UTF_LOG1_MAC("RAM entry index   :  %d\n",   eventPtr->location.ramEntryInfo.ramRow);
        PRV_UTF_LOG1_MAC("HW table type     :  %s\n",   strNameBuffer);
        PRV_UTF_LOG1_MAC("Memory table type :  %s\n",   strNameBuffer1);
        PRV_UTF_LOG3_MAC("                     pipe %d, client %d, mem %d\n",
                         eventPtr->location.ramEntryInfo.memLocation.dfxPipeId,
                         eventPtr->location.ramEntryInfo.memLocation.dfxClientId,
                         eventPtr->location.ramEntryInfo.memLocation.dfxMemoryId);
    }


    curEventPtr = &occuredEvents.eventsArr[occuredEvents.eventsCount++];

    curEventPtr->eventsType = eventPtr->eventsType;
    curEventPtr->eventLogicalEntryInfo = eventPtr->location.logicalEntryInfo;
    curEventPtr->eventHwEntryInfo = eventPtr->location.hwEntryInfo;
    curEventPtr->eventRamEntryInfo = eventPtr->location.ramEntryInfo;

    DXCH_DATA_INTEGRITY_EVENTS_DB_UNLOCK();
    return GT_OK;
}


/**
* @internal prvTunnelStartEventsHandler function
* @endinternal
*
* @brief   Callback called in case of data integrity error.
*         Stores event relate to HW table CPSS_DXCH2_TABLE_TUNNEL_START_CONFIG_E
*/
static GT_STATUS prvTunnelStartEventsHandler
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC     *eventPtr
)
{
    GT_U32 index;

    CPSS_NULL_PTR_CHECK_MAC(eventPtr);

    if (eventPtr->location.isMppmInfoValid == GT_TRUE)
    {
        PRV_UTF_LOG3_MAC("DataIntegrity event, memory coordinates (MPPM): portGoupId = %d, mppmId = %d, bandk = %d\n",
                         eventPtr->location.mppmMemLocation.portGroupId,
                         eventPtr->location.mppmMemLocation.mppmId,
                         eventPtr->location.mppmMemLocation.bankId);
    }
    else if (eventPtr->location.isTcamInfoValid == GT_TRUE)
    {
        PRV_UTF_LOG2_MAC("DataIntegrity event, memory coordinates (TCAM): array %s, rule index = %5d\n",
                         (eventPtr->location.tcamMemLocation.arrayType ==
                          CPSS_DIAG_DATA_INTEGRITY_TCAM_ARRAY_TYPE_X_E ? "X": "Y"),
                         eventPtr->location.tcamMemLocation.ruleIndex);
    }
    else
    {
        PRV_UTF_LOG4_MAC("DataIntegrity event, memory coordinates: memType = %3d, pipe=%d, client=%2d, mem=%d\n",
                         eventPtr->location.ramEntryInfo.memType,
                         eventPtr->location.ramEntryInfo.memLocation.dfxPipeId,
                         eventPtr->location.ramEntryInfo.memLocation.dfxClientId,
                         eventPtr->location.ramEntryInfo.memLocation.dfxMemoryId);
    }

    tunnelStartEvents.allEventsCount ++;
    if (eventPtr->location.hwEntryInfo.hwTableType == CPSS_DXCH2_TABLE_TUNNEL_START_CONFIG_E)
    {
        tunnelStartEvents.tsEventsCount ++;
        index = tunnelStartEvents.tsEventsCount - 1;
        if (index < PRV_MAX_CALLBACK_EVENTS_COUNT_CNS)
        {
            tunnelStartEvents.eventsType[index] = eventPtr->eventsType;
            tunnelStartEvents.eventsInfo[index] = eventPtr->location.logicalEntryInfo;
        }

    }
    return GT_OK;
}

GT_VOID printEventHwInfo
(
    IN PRV_DXCH_DATA_INTEGRITY_EVENT_INFO_STC *eventPtr
)
{
    cpssOsPrintf("eventsType              %d\n", eventPtr->eventsType);
    PRV_CPSS_DATA_INTEGRITY_ENUM_STRING_GET_MAC(eventPtr->eventHwEntryInfo.hwTableType, CPSS_DXCH_TABLE_ENT, strNameBuffer);
    cpssOsPrintf("hwTableType             %s\n", strNameBuffer);
    PRV_CPSS_DATA_INTEGRITY_ENUM_STRING_GET_MAC(eventPtr->eventRamEntryInfo.memType, CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT, strNameBuffer);
    cpssOsPrintf("memType                 %s\n", strNameBuffer);
    cpssOsPrintf("                        pipe %d, client %d, mem %d\n",
                 eventPtr->eventRamEntryInfo.memLocation.dfxPipeId,
                 eventPtr->eventRamEntryInfo.memLocation.dfxClientId,
                 eventPtr->eventRamEntryInfo.memLocation.dfxMemoryId);

    cpssOsPrintf("hwTableEntryIndex       %d\n", eventPtr->eventHwEntryInfo.hwTableEntryIndex);
}

static GT_STATUS prvCpssDxChDiagDataIntegrityEventHwInfoDbCheck
(
    IN CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT eventTypeExp,
    IN CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT    memTypeExp,
    IN CPSS_DXCH_TABLE_ENT                           tableTypeExp,
    IN GT_U32                                        entryIndexExp,
    IN PRV_DATA_INTEGRITY_EVENT_MATCH_TYPE_ENT       matchType
)
{
    PRV_DXCH_DATA_INTEGRITY_EVENT_INFO_STC *eventPtr;
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT memTypeCur;
    CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT eventTypeCur;
    CPSS_DXCH_TABLE_ENT tableTypeCur;
    GT_U32 entryIndexCur;
    GT_U32 i;
    GT_U32 foundMatch = 0;
    GT_U32 foundNotMatch = 0;
    GT_U32 failed = 0;
    GT_U32 firstEventToPrint = 0;
    GT_U32 eventsNumToPrint = 0;
    GT_CHAR *msg = "";

    DXCH_DATA_INTEGRITY_EVENTS_DB_LOCK();

    if (occuredEvents.eventsCount == 0)
    {
        PRV_UTF_LOG0_MAC("No events occured!");
        DXCH_DATA_INTEGRITY_EVENTS_DB_UNLOCK();
        return GT_FAIL;
    }

    for (i = 0; i< occuredEvents.eventsCount && !failed; i++)
    {
        eventPtr = &occuredEvents.eventsArr[i];

        memTypeCur    = eventPtr->eventRamEntryInfo.memType;
        eventTypeCur  = eventPtr->eventsType;
        tableTypeCur  = eventPtr->eventHwEntryInfo.hwTableType;
        entryIndexCur = eventPtr->eventHwEntryInfo.hwTableEntryIndex;

        /* get "main" table name if tableTypeExp is "alternative" name for the
         * same physical table */
        prvCpssDxChDiagDataIntegrityIsMultipleNames(tableTypeExp, &tableTypeExp);

        foundMatch = foundMatch || ((eventTypeCur  == eventTypeExp) &&
                                    (memTypeExp    == memTypeCur)   &&
                                    (tableTypeCur  == tableTypeExp) &&
                                    (entryIndexCur == entryIndexExp));
        foundNotMatch = foundNotMatch || (! foundMatch);

        switch (matchType)
        {
            case PRV_DATA_INTEGRITY_EVENT_MATCH_TYPE_ALL_E:
                failed = foundNotMatch;
                msg = "FAILED - Some of occured events are not the same as expected event.\n";
                firstEventToPrint = i;
                eventsNumToPrint  = 1;

                break;

            case PRV_DATA_INTEGRITY_EVENT_MATCH_TYPE_ANY_E:
                failed = ((i == (occuredEvents.eventsCount-1)) && (!foundMatch));
                msg = "FAILED - An expected event is not found in the list of occured events.\n";
                firstEventToPrint = 0;
                eventsNumToPrint  = i + 1;
                break;
            case PRV_DATA_INTEGRITY_EVENT_MATCH_TYPE_THE_ONLY_E:
                failed = ((i > 0) || foundNotMatch);
                msg = "FAILED - Expected 1 event. Number of occured event >1.\n";
                firstEventToPrint = i;
                eventsNumToPrint  = 1;
                break;
        }
    }

    if (failed)
    {
        PRV_UTF_LOG1_MAC("\n\n%s\n", msg);

        /* print expected values */
        PRV_UTF_LOG1_MAC("Expected event type    : %d\n", eventTypeExp);

        PRV_CPSS_DATA_INTEGRITY_ENUM_STRING_GET_MAC(
            tableTypeExp, CPSS_DXCH_TABLE_ENT, strNameBuffer);
        PRV_UTF_LOG1_MAC("Expected HW table type : %s\n", strNameBuffer);

        PRV_CPSS_DATA_INTEGRITY_ENUM_STRING_GET_MAC(
            memTypeExp, CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT, strNameBuffer);
        PRV_UTF_LOG1_MAC("Expected memory type   : %s\n", strNameBuffer);

        PRV_UTF_LOG1_MAC("Expected HW table index: %d\n", entryIndexExp);

        /* print "bad" occured events */
        if (eventsNumToPrint)
        {
            PRV_UTF_LOG0_MAC("============== 'bad' events ============\n");

            for (i = 0; i < eventsNumToPrint; i++)
            {
                PRV_UTF_LOG1_MAC("'bad' events[%d]: \n" ,firstEventToPrint + i);
                eventPtr = &occuredEvents.eventsArr[firstEventToPrint + i];
                printEventHwInfo(eventPtr);
            }
        }

        DXCH_DATA_INTEGRITY_EVENTS_DB_UNLOCK();
        return GT_FAIL;
    }

    DXCH_DATA_INTEGRITY_EVENTS_DB_UNLOCK();
    return GT_OK;
}

static GT_VOID prvCpssDxChDiagDataIntegrityEventsClear
(
    GT_VOID
)
{
    DXCH_DATA_INTEGRITY_EVENTS_DB_LOCK();
    occuredEvents.eventsCount = 0;
    DXCH_DATA_INTEGRITY_EVENTS_DB_UNLOCK();
}

#endif

/**
* @internal prvDebugTraceEnable function
* @endinternal
*
* @brief   Routine to enable trace for data integrity events
*
* @param[in] enable                   - enable/disable trace output
*                                       None
*/
void prvDebugTraceEnable
(
    IN GT_U32 enable
)
{
    traceEvents = enable;
    return;
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagDataIntegrityTableScan
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC *locationPtr,
    IN  GT_U32                                numOfEntries,
    OUT GT_U32                               *nextEntryIndexPtr,
    OUT GT_BOOL                              *wasWrapAroundPtr
)
*/

UTF_TEST_CASE_MAC(cpssDxChDiagDataIntegrityTableScan)
{
/*
    ITERATE_DEVICES(Bobcat2, Caelum, Bobcat3, Aldrin, AC3X)

    1.1.1 Check correct values
        call with numOfEntries = 0;
        call with hwTableType  = CPSS_DXCH_SIP5_TABLE_LPM_MEM_E
        call with numOfEntries = 100;
        call with hwTableType = CPSS_DXCH_SIP5_TABLE_IPVX_INGRESS_EPORT_E
        call with hwTableEntryIndex = 10
        call with location.type = CPSS_DXCH_LOCATION_LOGICAL_INDEX_INFO_TYPE;
        check with two logical tables
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
    GT_U32      portGroupId = 0;

    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC location;
    GT_U32                               numOfEntries;
    GT_U32                               nextEntryIndex;
    GT_BOOL                              wasWrapAround;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION2_E | UTF_LION_E | UTF_XCAT2_E);

    cpssOsBzero((GT_CHAR*)&location, sizeof(location));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            location.portGroupsBmp = (1 << portGroupId);

            /*
                1.1.1 Check correct values
                Expected: GT_BAD_PTR
            */

            /* call with numOfEntries = 0; */
            /* call with hwTableType  = CPSS_DXCH_SIP5_TABLE_LPM_MEM_E */
            numOfEntries  = 0;
            location.type = CPSS_DXCH_LOCATION_HW_INDEX_INFO_TYPE;
            location.info.hwEntryInfo.hwTableType = CPSS_DXCH_SIP5_TABLE_LPM_MEM_E;
            location.info.hwEntryInfo.hwTableEntryIndex = 0;

            st = cpssDxChDiagDataIntegrityTableScan(dev, &location, numOfEntries, &nextEntryIndex, &wasWrapAround);
            if(GT_OK == isHwTableSupportPortGroupsBmp(dev,
                location.portGroupsBmp,
                location.info.hwEntryInfo.hwTableType))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,
                    location.info.hwEntryInfo.hwTableType);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev,
                    location.info.hwEntryInfo.hwTableType);
            }

            /* call with numOfEntries = 100; */
            numOfEntries  = 100;
            st = cpssDxChDiagDataIntegrityTableScan(dev, &location, numOfEntries, &nextEntryIndex, &wasWrapAround);
            if(GT_OK == isHwTableSupportPortGroupsBmp(dev,
                location.portGroupsBmp,
                location.info.hwEntryInfo.hwTableType))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,
                    location.info.hwEntryInfo.hwTableType);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev,
                    location.info.hwEntryInfo.hwTableType);
            }

            /* call with hwTableType = CPSS_DXCH_SIP5_TABLE_IPVX_INGRESS_EPORT_E */
            location.info.hwEntryInfo.hwTableType = CPSS_DXCH_SIP5_TABLE_IPVX_INGRESS_EPORT_E;
            st = cpssDxChDiagDataIntegrityTableScan(dev, &location, numOfEntries, &nextEntryIndex, &wasWrapAround);
            if(GT_OK == isHwTableSupportPortGroupsBmp(dev,
                location.portGroupsBmp,
                location.info.hwEntryInfo.hwTableType))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,
                    location.info.hwEntryInfo.hwTableType);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev,
                    location.info.hwEntryInfo.hwTableType);
            }

            /* call with hwTableEntryIndex = 10 */
            location.info.hwEntryInfo.hwTableEntryIndex = 10;
            st = cpssDxChDiagDataIntegrityTableScan(dev, &location, numOfEntries, &nextEntryIndex, &wasWrapAround);
            if(GT_OK == isHwTableSupportPortGroupsBmp(dev,
                location.portGroupsBmp,
                location.info.hwEntryInfo.hwTableType))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,
                    location.info.hwEntryInfo.hwTableType);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev,
                    location.info.hwEntryInfo.hwTableType);
            }

            /* call with location.type = CPSS_DXCH_LOCATION_LOGICAL_INDEX_INFO_TYPE; */
            location.type = CPSS_DXCH_LOCATION_LOGICAL_INDEX_INFO_TYPE;
            location.info.logicalEntryInfo.numOfLogicalTables = 1;
            location.info.logicalEntryInfo.logicaTableInfo[0].logicalTableType = CPSS_DXCH_LOGICAL_TABLE_VLAN_E;
            location.info.logicalEntryInfo.logicaTableInfo[0].logicalTableEntryIndex = 0;
            location.info.logicalEntryInfo.logicaTableInfo[0].numEntries = 1;
            st = cpssDxChDiagDataIntegrityTableScan(dev, &location, numOfEntries, &nextEntryIndex, &wasWrapAround);
            if(GT_OK == isLogicalTableSupportPortGroupsBmp(dev,
                location.portGroupsBmp,
                location.info.logicalEntryInfo.logicaTableInfo[0].logicalTableType))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(
                    GT_OK, st, dev,
                    location.info.logicalEntryInfo.logicaTableInfo[0].logicalTableType);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(
                    GT_BAD_PARAM, st, dev,
                    location.info.logicalEntryInfo.logicaTableInfo[0].logicalTableType);
            }

            /* check with two logical tables */
            location.info.logicalEntryInfo.numOfLogicalTables = 2;
            location.info.logicalEntryInfo.logicaTableInfo[0].logicalTableType = CPSS_DXCH_LOGICAL_TABLE_EPORT_E;
            location.info.logicalEntryInfo.logicaTableInfo[0].logicalTableEntryIndex = 0;
            location.info.logicalEntryInfo.logicaTableInfo[0].numEntries = 1;
            location.info.logicalEntryInfo.logicaTableInfo[1].logicalTableType = CPSS_DXCH_LOGICAL_TABLE_VRF_E;
            location.info.logicalEntryInfo.logicaTableInfo[1].logicalTableEntryIndex = 4;
            location.info.logicalEntryInfo.logicaTableInfo[1].numEntries = 2;
            st = cpssDxChDiagDataIntegrityTableScan(dev, &location, numOfEntries, &nextEntryIndex, &wasWrapAround);
            if(GT_OK == isLogicalTableSupportPortGroupsBmp(dev,
                location.portGroupsBmp,
                location.info.logicalEntryInfo.logicaTableInfo[0].logicalTableType) &&
               GT_OK == isLogicalTableSupportPortGroupsBmp(dev,
                location.portGroupsBmp,
                location.info.logicalEntryInfo.logicaTableInfo[1].logicalTableType) )
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(
                    GT_OK, st, dev,
                    location.info.logicalEntryInfo.logicaTableInfo[0].logicalTableType);

            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(
                    GT_BAD_PARAM, st, dev,
                    location.info.logicalEntryInfo.logicaTableInfo[0].logicalTableType);

            }

            /*
                1.1.2 Check pointers values
                Expected: GT_BAD_PTR
            */
            st = cpssDxChDiagDataIntegrityTableScan(dev, NULL, numOfEntries, &nextEntryIndex, &wasWrapAround);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

            st = cpssDxChDiagDataIntegrityTableScan(dev, &location, numOfEntries, NULL, &wasWrapAround);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

            st = cpssDxChDiagDataIntegrityTableScan(dev, &location, numOfEntries, NULL, &wasWrapAround);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

            /*
                1.1.2.1 Check wrong numOfEntries
                Expected: GT_BAD_PARAM
            */

            location.type = CPSS_DXCH_LOCATION_HW_INDEX_INFO_TYPE;
            location.info.hwEntryInfo.hwTableType = CPSS_DXCH_SIP5_TABLE_LPM_MEM_E;
            location.info.hwEntryInfo.hwTableEntryIndex = 0;

            numOfEntries = PRV_CPSS_DXCH_PP_MAC(dev)->accessTableInfoPtr[CPSS_DXCH_SIP5_TABLE_LPM_MEM_E].maxNumOfEntries;

            st = cpssDxChDiagDataIntegrityTableScan(dev, &location, numOfEntries+1, &nextEntryIndex, &wasWrapAround);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev,
                    location.info.hwEntryInfo.hwTableType);

            /*
                1.1.2.2 Check wasWrapAround can be NULL
                Expected: GT_OK
            */
            numOfEntries = 1;
            st = cpssDxChDiagDataIntegrityTableScan(dev, &location, numOfEntries, &nextEntryIndex, NULL);
            if(GT_OK == isHwTableSupportPortGroupsBmp(dev,
                location.portGroupsBmp,
                location.info.hwEntryInfo.hwTableType))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,
                    location.info.hwEntryInfo.hwTableType);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev,
                    location.info.hwEntryInfo.hwTableType);
            }


            /* 1.1.3 Check location */

            /*
               1.1.3.1 Call with wrong enum values location.type
               Expected: GT_BAD_PARAM
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChDiagDataIntegrityTableScan
                                (dev, &location, numOfEntries, &nextEntryIndex, &wasWrapAround),
                                location.type);

            /*
               1.1.3.1 Call with wrong enum values location.info.hwEntryInfo.hwTableType
               Expected: GT_BAD_PARAM
            */
            numOfEntries  = 1;
            location.type = CPSS_DXCH_LOCATION_HW_INDEX_INFO_TYPE;
            location.info.hwEntryInfo.hwTableEntryIndex = 0;

            UTF_ENUMS_CHECK_MAC(cpssDxChDiagDataIntegrityTableScan
                                (dev, &location, numOfEntries, &nextEntryIndex, &wasWrapAround),
                                location.info.hwEntryInfo.hwTableType);

            /*
               1.1.3.1 Call with wrong enum values location.info.logicalEntryInfo.logicaTableInfo[0].logicalTableType
               Expected: GT_BAD_PARAM
            */
            location.type = CPSS_DXCH_LOCATION_LOGICAL_INDEX_INFO_TYPE;
            location.info.logicalEntryInfo.numOfLogicalTables = 1;
            location.info.logicalEntryInfo.logicaTableInfo[0].logicalTableType = CPSS_DXCH_LOGICAL_TABLE_VLAN_E;
            location.info.logicalEntryInfo.logicaTableInfo[0].logicalTableEntryIndex = 0;
            location.info.logicalEntryInfo.logicaTableInfo[0].numEntries = 1;

            UTF_ENUMS_CHECK_MAC(cpssDxChDiagDataIntegrityTableScan
                                (dev, &location, numOfEntries, &nextEntryIndex, &wasWrapAround),
                                location.info.logicalEntryInfo.logicaTableInfo[0].logicalTableType);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set next non-active port */
            location.portGroupsBmp = (1 << portGroupId);
            numOfEntries = 1;
            st = cpssDxChDiagDataIntegrityTableScan(dev, &location, numOfEntries, &nextEntryIndex, &wasWrapAround);

            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, location.portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.x. For unaware port groups check that function returns GT_OK. */
        location.portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        numOfEntries = 1;
        st = cpssDxChDiagDataIntegrityTableScan(dev, &location, numOfEntries, &nextEntryIndex, &wasWrapAround);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, location.portGroupsBmp);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION2_E | UTF_LION_E | UTF_XCAT2_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        numOfEntries = 1;
        st = cpssDxChDiagDataIntegrityTableScan(dev, &location, numOfEntries, &nextEntryIndex, &wasWrapAround);
        UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK, st);
    }

}
#ifndef ASIC_SIMULATION
static GT_STATUS prvCpssDxChDiagDataIntegrityCountersDbCheck
(
    IN CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT    memTypeExp,
    IN GT_U32                                        entryIndex,
    IN CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT eventTypeExp,
    IN GT_U32                                        eventCountExp,
    OUT CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC *dfxMemLocationPtr
)
{
#ifdef IMPL_GALTIS
    GT_U32 currentDbKey;
    GT_U32 eventCountCur = 0;
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT memTypeCur = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_LAST_E;
    CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT eventTypeCur;
    CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC dfxMemLocation;
    CMD_DXCH_DIAG_DATA_INTEGRITY_COUNTER_DB_STC * dbEventCounterPtr = wrCpssDxChDiagDataIntegrityCountersDbGet();

    dfxMemLocationPtr->dfxPipeId = 0xFFFFFFFF;
    dfxMemLocationPtr->dfxClientId = 0xFFFFFFFF;
    dfxMemLocationPtr->dfxMemoryId = 0xFFFFFFFF;

    PRV_CPSS_DATA_INTEGRITY_ENUM_STRING_GET_MAC(memTypeExp, CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT, strNameBuffer);

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

        if (traceEvents > 1)
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
            /* Reset DB for the current event type */
            cpssOsMemSet(dbEventCounterPtr, 0, sizeof(CMD_DXCH_DIAG_DATA_INTEGRITY_COUNTER_DB_STC));

            if (eventCountExp != eventCountCur || memTypeExp != memTypeCur)
            {
                PRV_CPSS_DATA_INTEGRITY_ENUM_STRING_GET_MAC(memTypeCur, CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT, strNameBuffer1);

                UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(memTypeExp, memTypeCur,
                                                       "Expected memory type  :   %s, (%s)\r\n", strNameBuffer, strNameBuffer1);
                UTF_VERIFY_EQUAL3_STRING_NO_RETURN_MAC(eventCountExp, eventCountCur,
                                                       "Expected event counter:   %d(%d) %s\r\n", eventCountExp, eventCountCur, strNameBuffer1);
                return GT_FAIL;
            }

            /* Match found */
            *dfxMemLocationPtr = dfxMemLocation;
            return GT_OK;
        }
    }

    UTF_VERIFY_EQUAL3_STRING_NO_RETURN_MAC(eventCountExp, 0,
                                           "Expected event type %d for memory type %s entry index %d not triggered",
                                           eventTypeExp,
                                           strNameBuffer, entryIndex);
#else /* !defined(IMPL_GALTIS) */
    (void)memTypeExp;
    (void)entryIndex;
    (void)eventTypeExp;
    (void)eventCountExp;
    (void)dfxMemLocationPtr;
#endif /* !defined(IMPL_GALTIS) */
    return GT_NOT_FOUND;
}


static void prvCpssDxChDiagDataIntegrityCountersDbClear
(
    void
)
{
#ifdef IMPL_GALTIS
    CMD_DXCH_DIAG_DATA_INTEGRITY_COUNTER_DB_STC * dbEventCounterPtr = wrCpssDxChDiagDataIntegrityCountersDbGet();

    cpssOsBzero((GT_CHAR *)dbEventCounterPtr, sizeof(CMD_DXCH_DIAG_DATA_INTEGRITY_COUNTER_DB_STC));
#endif
    return;
}

/**
* @internal prvDxChDiagDataIntegrityMemTypeUpdate function
* @endinternal
*
* @brief   Get DFX memory type storing specified entry in one of
*         Ingress Policer table. Do nothing if table is out of list:
*         CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_METERING_E
*         CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_COUNTING_E
*         CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_METERING_E
*         CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_COUNTING_E
*         For other tables or irrelevant devices do nothing.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - PP device number
* @param[in] tableType                - HW table name.
* @param[in] entryIndex               - HW table entry index
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvDxChDiagDataIntegrityMemTypeUpdate
(
    IN  GT_U8                                      devNum,
    IN  CPSS_DXCH_TABLE_ENT                        tableType,
    IN  GT_U32                                     entryIndex,
    OUT CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT *memTypePtr
)
{
    GT_STATUS rc;
    GT_U32 startRamIx;
    GT_U32 ramsCount;

    GT_U32 ramArr[3];
    GT_U32 ramArrSize = sizeof (ramArr) /sizeof(ramArr[0]);
    GT_U32 p0StartRamIx;
    GT_U32 p0RamsCount;
    GT_U32 p1StartRamIx;
    GT_U32 p1RamsCount;

    GT_U32 ramIx; /* ram number iterator: [0..2] */
    GT_U32 itemsCount;
    GT_BOOL isMetering = GT_FALSE;         /* metering or billing */

    switch (tableType)
    {
        case CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_METERING_E:
        case CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_METERING_E:
            isMetering = GT_TRUE;
            break;

        case CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_COUNTING_E:
        case CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_COUNTING_E:
            isMetering = GT_FALSE;
            break;

        default:
            /* do nothing */
            return GT_OK;
    }

    if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        *memTypePtr = (isMetering == GT_TRUE) ?
            CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_METERING_TB_TBL_E :
            CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_COUNTING_TBL_E;
        return GT_OK;
    };

    /* handle sip5 devices < sip5.20 */

    rc = prvCpssPolicerIngressRamIndexInfoGet(devNum,
                                              &ramArr[0], &ramArr[1], &ramArr[2],
                                              &p0StartRamIx, &p0RamsCount,
                                              &p1StartRamIx, &p1RamsCount);
    if (rc != GT_OK)
    {
        return (rc == GT_NOT_APPLICABLE_DEVICE) ? GT_OK : rc;
    }

    /* init startRamIx, ramsCount */
    switch (tableType)
    {
        case CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_METERING_E:
            if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
            {
                PLR_METERING_SHARED_USE_PLR0_AND_BASE_ADDR_LABEL
                /* the table uses all RAMS */
                startRamIx = MIN(p0StartRamIx, p1StartRamIx);
                ramsCount = p0RamsCount + p1RamsCount;
            }
            else
            {
                startRamIx = p0StartRamIx;
                ramsCount  = p0RamsCount;
            }
            break;

        case CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_METERING_E:
            if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
            {
                PLR_METERING_SHARED_USE_PLR0_AND_BASE_ADDR_LABEL
                /* there are no RAMs for this table  */
                return GT_FAIL;
            }
            else
            {
                startRamIx = p1StartRamIx;
                ramsCount  = p1RamsCount;
            }
            break;

        case CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_COUNTING_E:
            startRamIx = p0StartRamIx;
            ramsCount  = p0RamsCount;
            break;

        case CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_COUNTING_E:
            startRamIx = p1StartRamIx;
            ramsCount  = p1RamsCount;
            break;
        default:
            /* impossible. Previous "switch" filtered all tables except ones in "case"-es */
            return GT_FAIL;
    }


    /* find RAM storing entryIndex */
    if (startRamIx + ramsCount > ramArrSize)
    {
        /* impossible */
        return GT_FAIL;
    }

    for (itemsCount = 0, ramIx = startRamIx; ramIx < startRamIx+ramsCount; ramIx++)
    {
        itemsCount += ramArr[ramIx];
        if (entryIndex < itemsCount)
        {
            /* found a desired RAM */
            break;
        }
    }
    if (ramIx >= ramArrSize)
    {
        /* RAM is not found. Bad entryIndex. */
        return GT_BAD_PARAM;
    }

    switch (ramIx)
    {
        case 0:
            *memTypePtr = (GT_TRUE == isMetering ?
                           CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_METERING_INGRESS_PLR0_E :
                           CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_BILLING_INGRESS_PLR0_E);
            break;

        case 1:
            *memTypePtr = (GT_TRUE == isMetering ?
                           CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_METERING_INGRESS_PLR1_E :
                           CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_BILLING_INGRESS_PLR1_E);
            break;
        case 2:
            *memTypePtr = (GT_TRUE == isMetering ?
                           CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_METERING_INGRESS_PLR2_E :
                           CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_BILLING_INGRESS_PLR2_E);
            break;

        default:
            return GT_FAIL;
    }

    return GT_OK;
}

/**
* @internal prvUtfDataIntegrityErrorInjectionTablesSkipCheck function
* @endinternal
*
* @brief   Function checks table avalability for test based on DFX injection error.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Caelum; Bobcat3; Aldrin2; Falcon.
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
    IN CPSS_DXCH_TABLE_ENT tableType,
    OUT CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      *memTypePtr,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   *errTypePtr,
    OUT GT_U32                                          *numOfEntriesPtr
)
{
    GT_STATUS rc;
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC                    locationInfo;
    CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT  protectionType;

    /* use debug function to check HW Table and get some info for test */
    rc = prvCpssDxChDiagDataIntegrityHwTableCheck(devNum, tableType, memTypePtr, numOfEntriesPtr);
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
                    /* MAC to me implemented as registers. there is no protection for it */
                case CPSS_DXCH3_TABLE_MAC2ME_E:
                    /* TCAM is special entity. There is no error injection for it */
                case CPSS_DXCH_SIP5_TABLE_TCAM_E:
                    /* Port Mirror Index implemented by registers. there is no protection for it */
                case CPSS_DXCH_SIP5_TABLE_EQ_INGRESS_MIRROR_PHYSICAL_PORT_E:
                    /* TRILL Adjacency Entry implemented as registers. there is no protection for it */
                case CPSS_DXCH_SIP5_TABLE_ADJACENCY_E:
                    /* OAM Aging tables implemented as registers. there is no protection for it */
                case CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_KEEPALIVE_AGING_E:
                case CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_KEEPALIVE_AGING_E:
                    /* ERMRK LM Table implemented as registers. there is no protection for it */
                case CPSS_DXCH_SIP5_TABLE_OAM_LM_OFFSET_E:
                    /* EXP To QoS Profile Map Table implemented as registers. there is no protection for it */
                case CPSS_DXCH_SIP5_TABLE_TTI_QOS_MAP_EXP_TO_QOS_PROFILE_E:
                    /* Multicast Counters Memory implemented as registers. there is no protection for it */
                case CPSS_DXCH_SIP5_TABLE_BMA_MULTICAST_COUNTERS_E:
                    /* TxQ Counters implemented as registers. there is no protection for it */
                case CPSS_DXCH_SIP5_TABLE_TAIL_DROP_COUNTERS_Q_MAIN_BUFF_E:
                case CPSS_DXCH_SIP5_TABLE_TAIL_DROP_COUNTERS_Q_MAIN_MC_BUFF_E:
                case CPSS_DXCH_SIP5_TABLE_TXQ_LINK_LIST_COUNTERS_Q_MAIN_DESC_E:
                    /* PFC tables implemented as registers. there is no protection for it */
                case CPSS_DXCH_SIP5_TABLE_PFC_MODE_PROFILE_TC_XOFF_THRESHOLDS_E:
                case CPSS_DXCH_SIP5_TABLE_PFC_MODE_PROFILE_TC_XON_THRESHOLDS_E:
                case CPSS_DXCH_SIP5_TABLE_PFC_LLFC_COUNTERS_E:
                    /* TM is not supported by Data Integrity yet. */
                case CPSS_DXCH_SIP5_TABLE_TM_DROP_DROP_MASKING_E:
                case CPSS_DXCH_SIP5_TABLE_TM_DROP_QUEUE_PROFILE_ID_E:
                case CPSS_DXCH_SIP5_TABLE_TM_QMAP_CPU_CODE_TO_TC_E:
                case CPSS_DXCH_SIP5_TABLE_TM_QMAP_TARGET_DEV_TO_INDEX_E:
                case CPSS_DXCH_SIP5_TABLE_TM_QMAP_POLICING_ENABLE_E:
                case CPSS_DXCH_SIP5_TABLE_TM_QMAP_QUEUE_ID_SELECTOR_E:
                case CPSS_DXCH_SIP5_TABLE_TM_INGR_GLUE_L1_PKT_LEN_OFFSET_E:
                case CPSS_DXCH_SIP5_TABLE_TM_EGR_GLUE_AGING_QUEUE_PROFILE_E:
                case CPSS_DXCH_SIP5_TABLE_TM_EGR_GLUE_AGING_PROFILE_THESHOLDS_E:
                case CPSS_DXCH_SIP5_TABLE_TM_EGR_GLUE_TARGET_INTERFACE_E:
                case CPSS_DXCH_SIP5_TABLE_TM_FCU_ETH_DMA_TO_TM_PORT_MAPPING_E:
                case CPSS_DXCH_SIP5_TABLE_TM_FCU_ILK_DMA_TO_TM_PORT_MAPPING_E:
                case CPSS_DXCH_SIP5_TABLE_TM_FCU_TC_PORT_TO_CNODE_PORT_MAPPING_E:
                case CPSS_DXCH_SIP5_TABLE_TM_FCU_PORT_INGRESS_TIMERS_CONFIG_E:
                case CPSS_DXCH_SIP5_TABLE_TM_FCU_INGRESS_TIMERS_E:
                case CPSS_DXCH_SIP5_TABLE_TM_FCU_PORT_TO_PHYSICAL_PORT_MAPPING_E:
                    return GT_NOT_IMPLEMENTED;

                    /* tables are implemented as registers. there is no protection for it. */
                case CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_EFT_FAST_STACK_FAILOVER_SECONDARY_TARGET_PORT_E:
                case CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_E:
                    if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
                    {
                        return GT_NOT_IMPLEMENTED;
                    }

                case CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_FWD_FROM_CPU_TO_LOOPBACK_MAPPER_E:
                    /* table doesn't exists in bobcat3  */
                    if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
                    {
                        return GT_NOT_IMPLEMENTED;
                    }


                /* tables are implemented as registers. there is no protection for it. */
                case CPSS_DXCH_SIP5_TABLE_BMA_PORT_MAPPING_E:
                case CPSS_DXCH_SIP5_20_TABLE_TTI_QCN_TO_TIMER_PAUSE_MAP_E:
                    if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
                    {
                        return GT_NOT_IMPLEMENTED;
                    }
                    break;

                default:
                    break;
            }

            /* there is bug in DB */
            return GT_FAIL;
        }
    }

    /* Not relevant and well known not supported tables*/
    switch ((GT_U32)tableType) /* cast to avoid compiler warnings: values
                                  CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E+1..30
                                  are out of enum. */
    {
        case CPSS_DXCH_SIP5_TABLE_TCAM_PCL_TTI_ACTION_E:
            if (PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(devNum)  ||
                PRV_CPSS_DXCH_BOBCAT2_CHECK_MAC(devNum) ||
                PRV_CPSS_DXCH_BOBCAT3_CHECK_MAC(devNum) ||
                PRV_CPSS_DXCH_CAELUM_CHECK_MAC(devNum)  ||
                PRV_CPSS_DXCH_CETUS_CHECK_MAC(devNum)   ||
                PRV_CPSS_DXCH_ALDRIN2_CHECK_MAC(devNum))
            {
                /* erratum TCAM - 481 - error injection does not work */
                return GT_NOT_IMPLEMENTED;
            }
            break;
        case CPSS_DXCH_SIP5_TABLE_CN_SAMPLE_INTERVALS_E:
            if (PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(devNum)  ||
                PRV_CPSS_DXCH_BOBCAT2_CHECK_MAC(devNum) ||
                PRV_CPSS_DXCH_BOBCAT3_CHECK_MAC(devNum) ||
                PRV_CPSS_DXCH_CAELUM_CHECK_MAC(devNum)  ||
                PRV_CPSS_DXCH_CETUS_CHECK_MAC(devNum)   ||
                PRV_CPSS_DXCH_ALDRIN2_CHECK_MAC(devNum))
            {
                /* erratum TXQ -1735 - error injection does not work */
                return GT_NOT_IMPLEMENTED;
            }
            break;
        case CPSS_DXCH_LION_TABLE_TXQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG_E:
        case CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG__DQ_1_E:
        case CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG__DQ_2_E:
        case CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG__DQ_3_E:
        case CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG__DQ_4_E:
        case CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG__DQ_5_E:

            /* HWE-3984090 - there is no interrupt for it */
            if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
                PRV_CPSS_DXCH_BOBCAT2_RM_SHAPER_PARITY_ERROR_INTERRUPT_WA_E))
            {
                return GT_NOT_IMPLEMENTED;
            }
            break;

        default:
            break;
    }

    /* check protection type */
    locationInfo.type = CPSS_DXCH_LOCATION_HW_INDEX_INFO_TYPE;
    locationInfo.info.hwEntryInfo.hwTableType = tableType;
    rc = cpssDxChDiagDataIntegrityProtectionTypeGet(devNum, &locationInfo, &protectionType);
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

    return GT_OK;
}


/**
* @internal prvUtfDataIntegrityEplrZeroTablesAndResetInt function
* @endinternal
*
* @brief   The function is primarily intended for Caelum.
*          See PRV_CPSS_DXCH_CAELUM_EGRESS_MEMORIES_NOT_RESET_WA_E.
*          A garbase values in Egress Policer tables trigger DataIntegrity
*          interrupts on reading.
*          To avoid this init the problem Egress Policer tables with zeroes and
*          reset their DataIntegrity interrupts that happened already.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2
*
* @param[in] devNum                   - PP device number
* @param[in] tableType                - table type
*
* @param[out] memTypePtr               - (pointer to) memory type
* @param[out] errTypePtr               - (pointer to) error type
* @param[out] numOfEntriesPtr          - (pointer to) number of table entries
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, portGroup, errorType
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_SUPPORTED         - if HW table exists but doesn't support
*                                    an event masking
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on fail
* @retval GT_NOT_FOUND             - if HW table is not found in Data Integrity DB
*/
static GT_STATUS prvUtfDataIntegrityEplrZeroTablesAndResetInt
(
    GT_U8 devNum
)
{
    GT_STATUS rc;
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC location;
    CPSS_DXCH_TABLE_ENT tablesArr[] = {
        CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_METERING_E,
        CPSS_DXCH_SIP5_15_TABLE_EGRESS_POLICER_METERING_CONFIG_E,
        CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_COUNTING_E};
    GT_U32 i;
    GT_U32 entryIx;
    GT_U32 *emptyEntryPtr;
    GT_U32 entriesNum;

    cpssOsMemSet(&tableEntry[0], 0, sizeof(tableEntry));
    emptyEntryPtr = &tableEntry[0];

    cpssOsMemSet(&location, 0, sizeof(location));

    location.portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    location.type = CPSS_DXCH_LOCATION_HW_INDEX_INFO_TYPE;
    for (i = 0; i < sizeof(tablesArr)/sizeof(tablesArr[0]); i++)
    {
        /* set tables entries to default values (zeroes). */

        rc = prvCpssDxChDiagDataIntegrityTableHwMaxIndexGet(
            devNum, tablesArr[i], &entriesNum, NULL);
        if (rc != GT_OK)
        {
            return rc;
        }
        for (entryIx=0; entryIx < entriesNum ; entryIx++)
        {
            rc = prvCpssDxChWriteTableEntry(devNum, tablesArr[i], entryIx, emptyEntryPtr);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        /* unmask interrupts to catch and reset them. */
        location.info.hwEntryInfo.hwTableType = tablesArr[i];

        /* the API ignores errorType in case of parity-protected memory,
           so errorType = signle_and_multiple_ecc is ok for both
           ecc-protected, parity-protected memory types */
        rc = cpssDxChDiagDataIntegrityEventMaskSet(
            devNum, &location,
            CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E,
            CPSS_EVENT_UNMASK_E);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* let unmasked interrupts happen */
        cpssOsTimerWkAfter(15);

        /* mask interrupts */
        rc = cpssDxChDiagDataIntegrityEventMaskSet(
            devNum, &location,
            CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E,
            CPSS_EVENT_MASK_E);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    return GT_OK;
}

UTF_TEST_CASE_MAC(cpssDxChDiagDataIntegrityHwTablesErrorInjectionTest)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Aldrin, AC3X)
*/
    GT_STATUS           st = GT_OK;
    GT_U8               dev;
    CPSS_DXCH_TABLE_ENT                             tableType;
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType;
    CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errType;
    GT_U32                                          entryIndex;
    GT_U32                                          i;
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            dfxMemLocationInfo;
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            location;
    GT_U32                                          nextEntryIndex;
    GT_BOOL                                         wasWrapAround;
    GT_U32                                          maxEntries;
    GT_U32                                          scanEntries;
    CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC    dfxMemLocation;
    GT_U32                                          errorCounter;
    GT_U32                                          failedRow;
    GT_U32                                          failedSegment;
    GT_U32                                          failedSyndrome;
    GT_U32                                          *tableEntryPtr = &tableEntry[0];
    CPSS_DXCH_TABLE_ENT                             lastTableType;
    GT_U32                                          tblsCount;
    GT_U32                                          readEntryBuf[MAX_ENTRY_SIZE_CNS];

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E | UTF_XCAT3_E |
                                           UTF_FALCON_E);

    cpssOsMemSet(tableEntryPtr, 0xFF, sizeof(tableEntry));
    cpssOsMemSet(&dfxMemLocationInfo, 0, sizeof(dfxMemLocationInfo));
    cpssOsMemSet(&location, 0, sizeof(location));
    cpssOsMemSet(&dfxMemLocation, 0, sizeof(dfxMemLocation));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        lastTableType =
            PRV_CPSS_SIP_5_25_CHECK_MAC(dev) ? CPSS_DXCH_SIP5_25_TABLE_LAST_E :
            PRV_CPSS_SIP_5_20_CHECK_MAC(dev) ? CPSS_DXCH_SIP5_20_TABLE_LAST_E :
            PRV_CPSS_SIP_5_15_CHECK_MAC(dev) ? CPSS_DXCH_SIP5_15_TABLE_LAST_E :
            CPSS_DXCH_BOBCAT2_TABLE_LAST_E;

        if (PRV_CPSS_DXCH_CETUS_CHECK_MAC(dev) ||
            PRV_CPSS_DXCH_CAELUM_CHECK_MAC(dev))
        {
            st = prvUtfDataIntegrityEplrZeroTablesAndResetInt(dev);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        }

        /* clean DB */
        prvCpssDxChDiagDataIntegrityCountersDbClear();

        tblsCount = 0;
        /* loop over all memories and inject errors */

        for(tableType = CPSS_DXCH_TABLE_VLAN_PORT_PROTOCOL_E; tableType < lastTableType; tableType++)
        {
            if ((selectedTableType != 0xFFFFFFFF) && (selectedTableType != tableType))
            {
                continue;
            }

            PRV_CPSS_DATA_INTEGRITY_ENUM_STRING_GET_MAC(
                tableType, CPSS_DXCH_TABLE_ENT, strNameBuffer);
            PRV_UTF_LOG1_MAC("table: %s\n", strNameBuffer);

            st = prvUtfDataIntegrityErrorInjectionTablesSkipCheck(dev, tableType, &memType, &errType, &maxEntries);
            if (st != GT_OK)
            {
                if (traceEvents)
                {
                    PRV_CPSS_DATA_INTEGRITY_ENUM_STRING_GET_MAC(tableType, CPSS_DXCH_TABLE_ENT, strNameBuffer);
                    switch (st)
                    {
                        case GT_NOT_FOUND:
                            cpssOsPrintf("%s - not found in DB\r\n", strNameBuffer);
                            break;
                        case GT_NOT_SUPPORTED:
                            cpssOsPrintf("%s - not exists in SIP5\r\n", strNameBuffer);
                            break;
                        case GT_NOT_IMPLEMENTED:
                            cpssOsPrintf("%s - not used in test\r\n", strNameBuffer);
                            break;
                        case GT_BAD_VALUE:
                            cpssOsPrintf("%s - not protected table\r\n", strNameBuffer);
                            break;
                    }
                }

                continue;
            }

            if (traceEvents)
            {
                PRV_CPSS_DATA_INTEGRITY_ENUM_STRING_GET_MAC(tableType, CPSS_DXCH_TABLE_ENT, strNameBuffer);
                cpssOsPrintf("Test table %s\n", strNameBuffer);
            }

            /* Set max table entries for scan */
            scanEntries = maxEntries;
            tblsCount++;
            for (i = 0; i < 3; i++)
            {
                /* Calculate entry index and write table entry field */
                entryIndex = (i == 2) ? 0 : ((maxEntries - 1) / (i + 1));

                switch (tableType)
                {
                    case CPSS_DXCH_SIP5_TABLE_TCAM_PCL_TTI_ACTION_E:
                    case CPSS_DXCH_TABLE_VLAN_PORT_PROTOCOL_E:
                        /* AUTODOC: avoid not valid entries - 4 LSB must be < 12 */
                        entryIndex = entryIndex & 0xFFFFFFF0;
                        break;
                    case CPSS_DXCH_SIP5_TABLE_LPM_MEM_E:
                        if (PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(dev) ||
                            PRV_CPSS_DXCH_CETUS_CHECK_MAC(dev))
                        {
                            /* AUTODOC: avoid not valid entries -
                               only first 2K of each 16K window are valid */
                            entryIndex = entryIndex & 0xFFFFC7FF;
                        } else if (PRV_CPSS_DXCH_CAELUM_CHECK_MAC(dev))
                        {
                            /* AUTODOC: avoid not valid entries -
                               only first 6K of each 16K window are valid */
                            entryIndex = (entryIndex / _16K) + (entryIndex % _6K);
                        }
                        break;
                    case CPSS_DXCH_SIP5_TABLE_ERMRK_PTP_LOCAL_ACTION_E:
                        /* indexes appropriating PTP domains 5..7 are not valid
                         * in this table for all SIP5 devices */
                        if (entryIndex % 8 > 4)
                        {
                            entryIndex -= 4;
                        }
                    default:
                        break;
                }

                /* Check entry index for HW table */
                if(GT_FALSE ==
                    prvCpssDxChDiagDataIntegrityTableCheckIsDevSupportHwTable(dev, tableType, entryIndex))
                {
                    continue;
                }

                /* get exect memType for SIP5 devices (except bobcat3) Ingress Policer 0/1 tables
                   CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_[0|1]_METERING_E
                   CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_[0|1]_COUNTING_E
                   For other tables/devices do nothing.
                 */
                st = prvDxChDiagDataIntegrityMemTypeUpdate(dev, tableType, entryIndex, &memType);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                dfxMemLocationInfo.type = CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE;
                dfxMemLocationInfo.info.ramEntryInfo.memType = memType;

                if (selectedTableType != 0xFFFFFFFF)
                {
                    PRV_UTF_LOG0_MAC("cpssDxChDiagDataIntegrityEventMaskSet:\n");
                    cpssOsTimerWkAfter(5);
                }

                /* Enable error counter */
                st = cpssDxChDiagDataIntegrityErrorCountEnableSet(dev, &dfxMemLocationInfo, errType, GT_TRUE);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                /* Table found in DFX data base - unmask interrupt for DFX memory */
                st = cpssDxChDiagDataIntegrityEventMaskSet(dev, &dfxMemLocationInfo, errType, CPSS_EVENT_UNMASK_E);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                if (selectedTableType != 0xFFFFFFFF)
                {
                    PRV_UTF_LOG0_MAC("cpssDxChDiagDataIntegrityErrorInjectionConfigSet:\n");
                    cpssOsTimerWkAfter(5);
                }
                /* Enable error injection */
                st = cpssDxChDiagDataIntegrityErrorInjectionConfigSet(dev, &dfxMemLocationInfo, CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E, GT_TRUE);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                if (selectedTableType != 0xFFFFFFFF)
                {
                    PRV_UTF_LOG0_MAC("prvCpssDxChPortGroupWriteTableEntry:\n");
                    cpssOsTimerWkAfter(5);
                }

                if (PRV_CPSS_SIP_5_20_CHECK_MAC(dev) &&
                    tableType >= CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E &&
                    tableType <= CPSS_DXCH_SIP5_TABLE_CNC_31_COUNTERS_E)
                {
                    /* t’s prohibited to write to CNC memory in BC3. Device stuck.
                       Need to use read operation instead. The “read” operation
                       writes to CNC memory also because it’s “cleared” on read.*/
                    st = prvCpssDxChPortGroupReadTableEntry(
                        dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                        tableType, entryIndex, &readEntryBuf[0]);
                }
                else
                {
                    st = prvCpssDxChPortGroupWriteTableEntry(
                        dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                        tableType, entryIndex, tableEntryPtr);
                }
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                location.portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
                location.type = CPSS_DXCH_LOCATION_HW_INDEX_INFO_TYPE;
                location.info.hwEntryInfo.hwTableType = tableType;
                location.info.hwEntryInfo.hwTableEntryIndex = 0;
                if (selectedNumOfEntries != 0xFFFFFFFF)
                {
                    scanEntries = selectedNumOfEntries;
                    location.info.hwEntryInfo.hwTableEntryIndex = entryIndex;
                }

                if (selectedTableType != 0xFFFFFFFF)
                {
                    PRV_UTF_LOG0_MAC("cpssDxChDiagDataIntegrityTableScan:\n");
                    cpssOsTimerWkAfter(5);
                }

                /* Scan whole table */
                st = cpssDxChDiagDataIntegrityTableScan(dev, &location, scanEntries, &nextEntryIndex, &wasWrapAround);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /* Sleep 10 mSec */
                cpssOsTimerWkAfter(timeOut);

                /* Check specific event for current memory */
                st = prvCpssDxChDiagDataIntegrityCountersDbCheck(memType, entryIndex, errType, 1, &dfxMemLocation);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                if (selectedTableType != 0xFFFFFFFF)
                {
                    PRV_UTF_LOG0_MAC("prvCpssDxChDiagDataIntegrityCountersDbCheck:\n");
                    cpssOsTimerWkAfter(5);
                }

                dfxMemLocationInfo.info.ramEntryInfo.memLocation.dfxPipeId =   dfxMemLocation.dfxPipeId;
                dfxMemLocationInfo.info.ramEntryInfo.memLocation.dfxClientId = dfxMemLocation.dfxClientId;
                dfxMemLocationInfo.info.ramEntryInfo.memLocation.dfxMemoryId = dfxMemLocation.dfxMemoryId;

                st = cpssDxChDiagDataIntegrityErrorInfoGet(dev, &dfxMemLocationInfo, NULL,
                                                           &errorCounter, &failedRow, &failedSegment, &failedSyndrome);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                if (st == GT_OK)
                {
                    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(0, errorCounter, dev);
                    if (errorCounter == 0)
                    {
                        PRV_CPSS_DATA_INTEGRITY_ENUM_STRING_GET_MAC(
                            tableType, CPSS_DXCH_TABLE_ENT, strNameBuffer);
                        cpssOsPrintf("%s - table (%d scan entries) - Current error counter = 0 \r\n",
                                     strNameBuffer, scanEntries);
                    }
                }

                /* Disable error injection */
                st = cpssDxChDiagDataIntegrityErrorInjectionConfigSet(
                    dev, &dfxMemLocationInfo,
                    CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E,
                    GT_FALSE);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                /* clean memory by write entry. This operation may generate Data
                   Integrity interrupt when one RAM have several HW entries.
                   Need to clean DB after this call. */

                if (!(PRV_CPSS_SIP_5_20_CHECK_MAC(dev) &&
                      tableType >= CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E &&
                      tableType <= CPSS_DXCH_SIP5_TABLE_CNC_31_COUNTERS_E))
                {
                    /* t’s prohibited to write to CNC memory in BC3. Device stuck. */
                    st = prvCpssDxChPortGroupWriteTableEntry(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                             tableType, entryIndex, tableEntryPtr);
                    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
                }

                /* Sleep 10 mSec */
                cpssOsTimerWkAfter(timeOut);

                /* clean DB */
                prvCpssDxChDiagDataIntegrityCountersDbClear();

                /* Mask DFX memory interrupt */
                st = cpssDxChDiagDataIntegrityEventMaskSet(dev, &dfxMemLocationInfo, errType, CPSS_EVENT_MASK_E);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            }
        }
        PRV_UTF_LOG2_MAC("Device %d Number tested HW Tables %d:\n", dev, tblsCount);
    }
}
#endif


UTF_TEST_CASE_MAC(cpssDxChDiagDataIntegrityTableScan_1)
{
    GT_STATUS   rc;
    GT_U8                                           devNum;
    GT_U32                                          ii;
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            specificLocation;
    GT_BOOL                                         foundMatch;
    GT_U32                                          numOfEntries;
    GT_U32                                          nextEntryIndex;
    GT_U32                                          expectedNextEntryIndex;
    GT_BOOL                                         wasWrapAround;
    GT_U32                                          global_maxTableSize;
    CPSS_DXCH_TABLE_ENT                             hwTableType;
    GT_U32                                          maxTableSize;
    const CPSS_DXCH_TABLE_ENT                       *hwNameArr;
    CPSS_DXCH_LOGICAL_TABLE_INFO_STC                *logicalInfoPtr;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION2_E | UTF_LION_E | UTF_XCAT2_E);

    GM_NOT_SUPPORT_THIS_TEST_MAC;

    cpssOsBzero((GT_CHAR*)&specificLocation, sizeof(specificLocation));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            /* Skip the test for Falcon until data integrity will be ready */
            SKIP_TEST_MAC;
        }

        specificLocation.portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        /* test logical tables */
        specificLocation.type = CPSS_DXCH_LOCATION_LOGICAL_INDEX_INFO_TYPE;
        specificLocation.info.logicalEntryInfo.numOfLogicalTables = 1;
        logicalInfoPtr = &specificLocation.info.logicalEntryInfo.logicaTableInfo[0];
        logicalInfoPtr->numEntries = 1;

        for(ii = 0 ; ii < CPSS_DXCH_LOGICAL_TABLE_LAST_E; ii++)
        {
            GT_U32  jj;

            logicalInfoPtr->logicalTableType = ii;

            foundMatch = GT_FALSE;

            global_maxTableSize = 0;

            rc = prvCpssDxChDiagDataIntegrityTableLogicalToHwListGet(devNum, ii, &hwNameArr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, devNum);


            for(jj = 0; hwNameArr[jj] != LAST_VALID_TABLE_CNS; jj++)
            { /* hw tables iteration */
                hwTableType = hwNameArr[jj];

                if(GT_FALSE ==
                    prvCpssDxChDiagDataIntegrityTableCheckIsDevSupportHwTable(devNum,hwTableType,
                                                                             0))
                {
                    continue;
                }

                rc = prvCpssDxChDiagDataIntegrityLogicalToHwTableMap(
                    devNum, ii/* logical table  */, 0, 0,
                    hwTableType,NULL, NULL,
                    NULL, &maxTableSize);
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

            /* transform HW table entries number to logical entries number */
            if (ii == CPSS_DXCH_LOGICAL_TABLE_ARP_E)
            {
                /* 8 ARP entries per one HW entry */
                numOfEntries = global_maxTableSize / 2 - 8;
            }
            else
            {
                numOfEntries = global_maxTableSize / 2 - 1;
            }
            /* from (almost) mid table till (almost) end */
            logicalInfoPtr->numEntries = numOfEntries;
            logicalInfoPtr->logicalTableEntryIndex =
                (global_maxTableSize / 2);

            rc = cpssDxChDiagDataIntegrityTableScan(devNum,&specificLocation,
                0/*not used !!! for logical tables */,
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
            if (ii == CPSS_DXCH_LOGICAL_TABLE_ARP_E)
            {
                /* One HW entry contains 8 ARP entries. So number of ARP entries scanned
                   in HW table is always even to 8. */
                expectedNextEntryIndex += (8 - expectedNextEntryIndex % 8) % 8;
            }


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
            rc = cpssDxChDiagDataIntegrityTableScan(devNum,&specificLocation,
                0/*not used !!! for logical tables */,
                &nextEntryIndex,
                &wasWrapAround);

            if(rc != GT_OK)
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc, devNum, ii);
            }

            expectedNextEntryIndex = logicalInfoPtr->logicalTableEntryIndex + numOfEntries;
            if (ii == CPSS_DXCH_LOGICAL_TABLE_ARP_E)
            {
                /* Round to nearest upper number even to 8.
                   One HW entry contains 8 ARP entries. So number of ARP entries scanned
                   in HW table is always even to 8. */
                expectedNextEntryIndex += (8 - expectedNextEntryIndex % 8) % 8;
            }
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
        specificLocation.type = CPSS_DXCH_LOCATION_HW_INDEX_INFO_TYPE;
        for(ii = 0 ; ii < CPSS_DXCH_TABLE_LAST_E; ii++)
        {
            if(GT_FALSE ==
                prvCpssDxChDiagDataIntegrityTableCheckIsDevSupportHwTable(devNum,ii,
                0))
            {
                /* the device not supports this HW table */
                continue;
            }

            hwTableType = ii;

            /* get number of entries in the table */
            rc = prvCpssDxChDiagDataIntegrityTableHwMaxIndexGet(devNum,hwTableType,&global_maxTableSize,NULL);
            if(rc != GT_OK)
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, devNum);
            }

            specificLocation.info.hwEntryInfo.hwTableType = ii;

            specificLocation.info.hwEntryInfo.hwTableEntryIndex =
                (global_maxTableSize / 2) -1;
            numOfEntries = global_maxTableSize / 2;

            rc = cpssDxChDiagDataIntegrityTableScan(devNum,&specificLocation,
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
            rc = cpssDxChDiagDataIntegrityTableScan(devNum,&specificLocation,
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
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION2_E | UTF_LION_E | UTF_XCAT2_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        numOfEntries = 1;
        rc = cpssDxChDiagDataIntegrityTableScan(devNum, &specificLocation, numOfEntries, &nextEntryIndex, &wasWrapAround);
        UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK, rc);
    }
}

UTF_TEST_CASE_MAC(cpssDxChDiagDataIntegrityTableEntryFix)
{
    GT_STATUS   rc;
    GT_U8                                           devNum;
    PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    *dbArrayPtr; /* pointer to data integrity DB */
    GT_U32                                          dbArrayEntryNum; /* size of data integrity DB */
    PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    *currentEntryPtr;
    PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    *nextEntryPtr;
    CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC    currentRamInfo;
    GT_U32                                          failedRow;
    GT_U32                                          ii,tmpIndex;
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            specificLocation;
    GT_BOOL                                         foundMatch;
    const CPSS_DXCH_TABLE_ENT                       *hwNameArr;
    GT_BOOL                                         isRamInfoSupported;
    GT_BOOL                                         isErrorExpected;
    CPSS_DXCH_HW_INDEX_INFO_STC                     hwEntryInfo;
    CPSS_DXCH_TABLE_ENT                             hwTableType;
    GT_U32                                          portGroupId; /* unused. Just to pass as output parameter */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION2_E | UTF_LION_E | UTF_XCAT2_E);

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

        isRamInfoSupported = GT_FALSE;
        if(PRV_CPSS_DXCH_BOBCAT2_B0_CHECK_MAC(devNum) ||
           PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(devNum)     ||  /* aldrin or AC3X */
           PRV_CPSS_DXCH_BOBCAT3_CHECK_MAC(devNum)    ||
           PRV_CPSS_DXCH_CETUS_CHECK_MAC(devNum)      ||
           PRV_CPSS_DXCH_CAELUM_CHECK_MAC(devNum)     ||
           PRV_CPSS_DXCH_ALDRIN2_CHECK_MAC(devNum))
        {
            isRamInfoSupported = GT_TRUE;
        }

        prvCpssDxChDiagDataIntegrityDbPointerSet(devNum, &dbArrayPtr, &dbArrayEntryNum);

        currentEntryPtr = &dbArrayPtr[0];

        specificLocation.portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
        specificLocation.type = CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE;
        specificLocation.info.ramEntryInfo.memType = /* dummy not used */
            CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BM_CONTROL_ACCESS_TABLE_E;
        /* test DFX memory */
        for(ii = 0 ; ii < dbArrayEntryNum ; ii++ , currentEntryPtr++)
        {
            currentRamInfo.dfxPipeId   = U32_GET_FIELD_MAC(currentEntryPtr->key, 12, 3);
            currentRamInfo.dfxClientId = U32_GET_FIELD_MAC(currentEntryPtr->key, 7,  5);
            currentRamInfo.dfxMemoryId = U32_GET_FIELD_MAC(currentEntryPtr->key, 0,  7);

            /* use last index of this RAM ... try to get this info from 'next entry' */
            failedRow = 31;
            nextEntryPtr = currentEntryPtr+1;
            tmpIndex = ii+1;
            while(1)
            {
                if(tmpIndex >= dbArrayEntryNum)
                {
                    break;
                }

                if(nextEntryPtr->memType != currentEntryPtr->memType)
                {
                    break;
                }

                if(nextEntryPtr->firstTableLine != 0)
                {
                    failedRow = nextEntryPtr->firstTableLine - 1;
                    break;
                }

                tmpIndex++;
                nextEntryPtr++;
            }


            specificLocation.info.ramEntryInfo.memLocation = currentRamInfo;
            specificLocation.info.ramEntryInfo.ramRow = failedRow;

            hwTableType = CPSS_DXCH_TABLE_VLAN_PORT_PROTOCOL_E;
            /* convert RAM info to 'HW info' */
            rc = prvCpssDxChDiagDataIntegrityDfxErrorConvert(devNum,
                    &specificLocation.info.ramEntryInfo.memLocation,
                    specificLocation.info.ramEntryInfo.ramRow,
                    &portGroupId, &hwEntryInfo);
            if(rc == GT_EMPTY)
            {
                /* ignore this RAM */
            }
            else
            {
                if(isRamInfoSupported == GT_TRUE)
                {
                    if (rc != GT_OK)
                    {
                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc, devNum, currentEntryPtr->key);
                    }

                    hwTableType = hwEntryInfo.hwTableType;

                    while(failedRow)
                    {
                        if(GT_FALSE ==
                            prvCpssDxChDiagDataIntegrityTableCheckIsDevSupportHwTable(devNum, hwTableType, hwEntryInfo.hwTableEntryIndex))
                        {
                            /* we get new index into hwEntryInfo.hwTableEntryIndex */
                            failedRow--;/* started at 31 and ... going down */
                            specificLocation.info.ramEntryInfo.ramRow = failedRow;

                            /* convert RAM info to 'HW info' */
                            rc = prvCpssDxChDiagDataIntegrityDfxErrorConvert(devNum,
                                    &specificLocation.info.ramEntryInfo.memLocation,
                                    failedRow,
                                    &portGroupId, &hwEntryInfo);

                            continue;
                        }

                        break;/* the .hwTableEntryIndex is supported in the HW table */
                    }
                }
                else
                {
                    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, rc, devNum);
                }
            }

            if(CPSS_DXCH_SHADOW_TYPE_NONE_E !=
                prvCpssDxChDiagDataIntegrityTableCheckIsDevSupportHwTableShadow(devNum,hwTableType))
            {
                isErrorExpected = GT_FALSE;
            }
            else
            {
                isErrorExpected = GT_TRUE;
            }

            rc = cpssDxChDiagDataIntegrityTableEntryFix(devNum,&specificLocation);
            if(rc == GT_EMPTY)
            {
                /* ignore this RAM */
            }
            else
            if(rc != GT_OK)
            {
                if(isRamInfoSupported == GT_TRUE && isErrorExpected == GT_FALSE)
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
        specificLocation.type = CPSS_DXCH_LOCATION_LOGICAL_INDEX_INFO_TYPE;
        specificLocation.info.logicalEntryInfo.numOfLogicalTables = 1;
        specificLocation.info.logicalEntryInfo.logicaTableInfo[0].logicalTableEntryIndex = failedRow;
        specificLocation.info.logicalEntryInfo.logicaTableInfo[0].numEntries = 1;

        for(ii = 0 ; ii < CPSS_DXCH_LOGICAL_TABLE_LAST_E; ii++)
        {
            GT_U32  jj;

            specificLocation.info.logicalEntryInfo.logicaTableInfo[0].logicalTableType = ii;

            foundMatch = GT_FALSE;

            rc = prvCpssDxChDiagDataIntegrityTableLogicalToHwListGet(devNum, ii, &hwNameArr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, devNum);

            isErrorExpected = GT_FALSE;
            for(jj = 0; hwNameArr[jj] != LAST_VALID_TABLE_CNS; jj++)
            { /* hw tables iteration */
                CPSS_DXCH_TABLE_ENT                     hwTableType;

                hwTableType = hwNameArr[jj];

                if(GT_FALSE ==
                   prvCpssDxChDiagDataIntegrityTableCheckIsDevSupportHwTable(
                       devNum,hwTableType, failedRow))
                {
                    /* the device not supports this HW table */
                    continue;
                }

                if(CPSS_DXCH_SHADOW_TYPE_NONE_E !=
                    prvCpssDxChDiagDataIntegrityTableCheckIsDevSupportHwTableShadow(devNum,hwTableType))
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


            rc = cpssDxChDiagDataIntegrityTableEntryFix(devNum,&specificLocation);
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
        specificLocation.type = CPSS_DXCH_LOCATION_HW_INDEX_INFO_TYPE;
        specificLocation.info.hwEntryInfo.hwTableEntryIndex = failedRow;
        for(ii = 0 ; ii < CPSS_DXCH_TABLE_LAST_E; ii++)
        {
            if(GT_FALSE ==
                prvCpssDxChDiagDataIntegrityTableCheckIsDevSupportHwTable(devNum,ii,
                failedRow))
            {
                /* the device doesn't support this HW table entry writing */
                continue;
            }

            if(CPSS_DXCH_SHADOW_TYPE_NONE_E !=
                prvCpssDxChDiagDataIntegrityTableCheckIsDevSupportHwTableShadow(devNum,ii))
            {
                isErrorExpected = GT_FALSE;
            }
            else
            {
                isErrorExpected = GT_TRUE;
            }

            specificLocation.info.hwEntryInfo.hwTableType = ii;

            rc = cpssDxChDiagDataIntegrityTableEntryFix(devNum,&specificLocation);
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
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION2_E | UTF_LION_E | UTF_XCAT2_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        rc = cpssDxChDiagDataIntegrityTableEntryFix(devNum, &specificLocation);
        UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK, rc);
    }
}


GT_VOID cpssDxChDiagDataIntegrityTables_cleanup_withSystemReset(GT_VOID)
{
    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;
    /* the function cpssDxChDiagDataIntegrityTableEntryFix() may leave the
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

UTF_TEST_CASE_MAC(cpssDxChDiagDataIntegrityTables_cleanup_withSystemReset)
{
    cpssDxChDiagDataIntegrityTables_cleanup_withSystemReset();
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagDataIntegrityShadowEntryInfoGet
(
    IN     GT_U8                                     devNum,
    INOUT  CPSS_DXCH_LOGICAL_TABLES_SHADOW_INFO_STC *tablesInfoPtr,
    OUT    GT_U32                                   *totalMemSizePtr
)
*/

UTF_TEST_CASE_MAC(cpssDxChDiagDataIntegrityShadowEntryInfoGet)
{
/*
    ITERATE_DEVICES(Bobcat2, Caelum, Bobcat3, Aldrin, AC3X)

    1.1.1 Check correct values
    Expected: GT_OK
        - call with CPSS_DXCH_SHADOW_TYPE_ALL_CPSS_CNS
        - call with CPSS_DXCH_SHADOW_TYPE_AS_MANY_HW_CNS
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

    CPSS_DXCH_LOGICAL_TABLES_SHADOW_INFO_STC tablesInfo;
    GT_U32                                   totalMemSize;
#define NUM_OF_LOGICAL_TABLES_CNS 3
    CPSS_DXCH_LOGICAL_TABLE_SHADOW_INFO_STC logArr[NUM_OF_LOGICAL_TABLES_CNS];

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION2_E | UTF_LION_E | UTF_XCAT2_E);

    cpssOsMemSet(&tablesInfo, 0, sizeof(CPSS_DXCH_LOGICAL_TABLES_SHADOW_INFO_STC));
    cpssOsMemSet(logArr, 0, sizeof(CPSS_DXCH_LOGICAL_TABLE_SHADOW_INFO_STC)*NUM_OF_LOGICAL_TABLES_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1.1 Check correct values
            Expected: GT_OK
        */

        /* call with CPSS_DXCH_SHADOW_TYPE_ALL_CPSS_CNS */
        tablesInfo.numOfDataIntegrityElements = CPSS_DXCH_SHADOW_TYPE_ALL_CPSS_CNS;
        st = cpssDxChDiagDataIntegrityShadowEntryInfoGet(dev, &tablesInfo, &totalMemSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if(GT_OK == st)
        {
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(totalMemSize, 0, dev);
        }

        /* call with CPSS_DXCH_SHADOW_TYPE_AS_MANY_HW_CNS */
        tablesInfo.numOfDataIntegrityElements = CPSS_DXCH_SHADOW_TYPE_AS_MANY_HW_CNS;
        st = cpssDxChDiagDataIntegrityShadowEntryInfoGet(dev, &tablesInfo, &totalMemSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* call with NUM_OF_LOGICAL_TABLES_CNS */
        tablesInfo.numOfDataIntegrityElements = NUM_OF_LOGICAL_TABLES_CNS;
        tablesInfo.logicalTablesArr = logArr;

        tablesInfo.logicalTablesArr[0].logicalTableName = CPSS_DXCH_LOGICAL_TABLE_VLAN_E;
        tablesInfo.logicalTablesArr[0].shadowType       = CPSS_DXCH_SHADOW_TYPE_CPSS_E;

        tablesInfo.logicalTablesArr[1].logicalTableName = CPSS_DXCH_LOGICAL_TABLE_PHYSICAL_PORT_E;
        tablesInfo.logicalTablesArr[1].shadowType       = CPSS_DXCH_SHADOW_TYPE_HW_E;

        tablesInfo.logicalTablesArr[2].logicalTableName = CPSS_DXCH_LOGICAL_TABLE_PORT_ISOLATION_E;
        tablesInfo.logicalTablesArr[2].shadowType       = CPSS_DXCH_SHADOW_TYPE_CPSS_E;

        st = cpssDxChDiagDataIntegrityShadowEntryInfoGet(dev, &tablesInfo, &totalMemSize);
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
        st = cpssDxChDiagDataIntegrityShadowEntryInfoGet(dev, NULL, &totalMemSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        st = cpssDxChDiagDataIntegrityShadowEntryInfoGet(dev, &tablesInfo, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);


        tablesInfo.numOfDataIntegrityElements = NUM_OF_LOGICAL_TABLES_CNS;
        tablesInfo.logicalTablesArr = NULL;
        st = cpssDxChDiagDataIntegrityShadowEntryInfoGet(dev, &tablesInfo, &totalMemSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        tablesInfo.logicalTablesArr = logArr; /* restore */

        /*
            1.1.3 Check with wrong enum value shadowType
            Expected: GT_BAD_PARAM
        */
        for(i = 0; i < NUM_OF_LOGICAL_TABLES_CNS; i++)
        {
            UTF_ENUMS_CHECK_MAC(cpssDxChDiagDataIntegrityShadowEntryInfoGet
                                (dev, &tablesInfo, &totalMemSize),
                                tablesInfo.logicalTablesArr[i].shadowType);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION2_E | UTF_LION_E | UTF_XCAT2_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagDataIntegrityShadowEntryInfoGet(dev, &tablesInfo, &totalMemSize);
        UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK, st);
    }
}
/* max number of words in entry */
#define MAX_ENTRY_SIZE_CNS   64
#define WRITE_ONLY  0
#define CHECK_SHADOW 1
#define CHECK_SHADOW_WITH_SELF_COMPARE  2

#define IGNORE_TABLE 0x0
typedef struct{
    CPSS_DXCH_TABLE_ENT hwTableType;
    GT_U32              appFamilyBmp;
    GT_U32  special_compareBitsPtr[MAX_ENTRY_SIZE_CNS];
}SPECIAL_COMPARE_BITS_STC;
static SPECIAL_COMPARE_BITS_STC special_compareBitsArr[] = {
    {CPSS_DXCH_SIP5_TABLE_TM_FCU_PORT_TO_PHYSICAL_PORT_MAPPING_E ,
     UTF_BOBCAT2_E,
     {0x3F/*bits 0..5*/}}
    ,{CPSS_DXCH_SIP5_TABLE_TM_FCU_PORT_TO_PHYSICAL_PORT_MAPPING_E ,
      UTF_CAELUM_E,
      {0xBF/*bits 0..5 , 7*/}}

    ,{CPSS_DXCH_LION_TABLE_TXQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG_E,
      UTF_ALL_FAMILY_E, /*remove <CurrentBucketSize>*/
      {0xFFFFFFFF,0x1000000F , 61/*every 61 bits*/}}

    ,{CPSS_DXCH_LION_TABLE_TXQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E,
      UTF_ALL_FAMILY_E, /*remove <CurrentBucketSize>*/
      {0xFFFFFFFF,0x1000000F}}

    ,{CPSS_DXCH_LION_TABLE_TXQ_EGRESS_STC_E,
      UTF_ALL_FAMILY_E,
      {0x3FFFFFFF,0x0,0x0}}/* valid only bits 0..29 */
    ,{CPSS_DXCH_SIP5_TABLE_EQ_INGRESS_STC_PHYSICAL_PORT_E,
      UTF_ALL_FAMILY_E,
      {0x3FFFFFFF,0x0,0x0}}/* valid only bits 0..29 */
    ,{CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_COUNTING_E, /* counting : non-valid 0..139(valid ..224) , policy counting : 0..191  */
      UTF_ALL_FAMILY_E,
      {0x0,0x0,0x0,0x0,
       /*128*/0x0,/*160*/0x0,/*192*/0xFFFFFFFF,/*224*/0x0000001F}}
    ,{CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_COUNTING_E,
      UTF_ALL_FAMILY_E,
      {0x0,0x0,0x0,0x0,
       /*128*/0x0,/*160*/0x0,/*192*/0xFFFFFFFF,/*224*/0x0000001F}}
    ,{CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_COUNTING_E, /* counting : non-valid 0..139(valid ..224) , policy counting : 0..191  */
      UTF_ALL_FAMILY_E,
      {0x0,0x0,0x0,0x0,
       /*128*/0x0,/*160*/0x0,/*192*/0xFFFFFFFF,/*224*/0x0000001F}}

    ,{CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_METERING_E,  /* non valid : 0..121 , valid ..240 */
      UTF_BOBCAT2_E,
      {0x0,0x0,0x0,/*96*/0xFC000000,
       /*128*/0xFFFFFFFF,/*160*/0xFFFFFFFF,/*192*/0xFFFFFFFF,/*224*/0x0001FFFF}}
    ,{CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_METERING_E, /* non valid : 0..121 , valid ..240 */
      UTF_BOBCAT2_E,
      {0x0,0x0,0x0,/*96*/0xFC000000,
       /*128*/0xFFFFFFFF,/*160*/0xFFFFFFFF,/*192*/0xFFFFFFFF,/*224*/0x0001FFFF}}
    ,{CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_METERING_E,    /* non valid : 0..121 , valid ..231 */
      UTF_BOBCAT2_E,
      {0x0,0x0,0x0,/*96*/0xFC000000,
       /*128*/0xFFFFFFFF,/*160*/0xFFFFFFFF,/*192*/0xFFFFFFFF,/*224*/0x000000FF}}

    ,{CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_METERING_E,  /* non valid : 0..121 , valid ..210 */
      UTF_ALL_FAMILY_E,
      {0x0,0x0,0x0,/*96*/0xFC000000,
       /*128*/0xFFFFFFFF,/*160*/0xFFFFFFFF,/*192*/0x7FFFF,/*224*/0}}
    ,{CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_METERING_E, /* non valid : 0..121 , valid ..210 */
      UTF_ALL_FAMILY_E,
      {0x0,0x0,0x0,/*96*/0xFC000000,
       /*128*/0xFFFFFFFF,/*160*/0xFFFFFFFF,/*192*/0x7FFFF,/*224*/0}}
    ,{CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_METERING_E,    /* non valid : 0..121 , valid ..210 */
      UTF_ALL_FAMILY_E,
      {0x0,0x0,0x0,/*96*/0xFC000000,
       /*128*/0xFFFFFFFF,/*160*/0xFFFFFFFF,/*192*/0x7FFFF,/*224*/0}}

    ,{CPSS_DXCH_SIP5_TABLE_IPVX_ROUTER_NEXTHOP_AGE_BITS_E,
      UTF_ALL_FAMILY_E,
      {IGNORE_TABLE}}/* the 32 bits are <age> that the device changes to 1 when
                        traffic hit rout entry */
    ,{CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_VLAN_E,    /* non valid : 48..63 */
      UTF_BOBCAT3_E | UTF_ALDRIN2_E,
      {0xFFFFFFFF,0x0000FFFF}}

    ,{CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG__DQ_1_E,
      UTF_BOBCAT3_E | UTF_ALDRIN2_E, /*remove <CurrentBucketSize>*/
      {0xFFFFFFFF,0x1000000F}}
    ,{CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG__DQ_2_E,
      UTF_BOBCAT3_E | UTF_ALDRIN2_E, /*remove <CurrentBucketSize>*/
      {0xFFFFFFFF,0x1000000F}}
    ,{CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG__DQ_3_E,
      UTF_BOBCAT3_E | UTF_ALDRIN2_E, /*remove <CurrentBucketSize>*/
      {0xFFFFFFFF,0x1000000F}}
    ,{CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG__DQ_4_E,
      UTF_BOBCAT3_E, /*remove <CurrentBucketSize>*/
      {0xFFFFFFFF,0x1000000F}}
    ,{CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG__DQ_5_E,
      UTF_BOBCAT3_E, /*remove <CurrentBucketSize>*/
      {0xFFFFFFFF,0x1000000F}}

    ,{CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG__DQ_1_E,
      UTF_BOBCAT3_E | UTF_ALDRIN2_E, /*remove <CurrentBucketSize>*/
      {0xFFFFFFFF,0x1000000F , 61/*every 61 bits*/}}
    ,{CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG__DQ_2_E,
      UTF_BOBCAT3_E | UTF_ALDRIN2_E, /*remove <CurrentBucketSize>*/
      {0xFFFFFFFF,0x1000000F , 61/*every 61 bits*/}}
    ,{CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG__DQ_3_E,
      UTF_BOBCAT3_E | UTF_ALDRIN2_E, /*remove <CurrentBucketSize>*/
      {0xFFFFFFFF,0x1000000F , 61/*every 61 bits*/}}
    ,{CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG__DQ_4_E,
      UTF_BOBCAT3_E, /*remove <CurrentBucketSize>*/
      {0xFFFFFFFF,0x1000000F , 61/*every 61 bits*/}}
    ,{CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG__DQ_5_E,
      UTF_BOBCAT3_E, /*remove <CurrentBucketSize>*/
      {0xFFFFFFFF,0x1000000F , 61/*every 61 bits*/}}

    /* must be last */
    ,{CPSS_DXCH_TABLE_LAST_E , UTF_NONE_FAMILY_E, {0}}/* must be last */
};

extern GT_U32 utfFamilyTypeGet(IN GT_U8 devNum);

static GT_BOOL didError_shadowTest_onDemand = GT_FALSE;

/* test the shadow of the CPSS :
    set entries in the HW table and check that read from HW match read from shadow.

*/
static void cpssDxChDiagDataIntegrityTables_shadowTest_entry(
    IN GT_U8               devNum,
    IN CPSS_DXCH_TABLE_ENT hwTableType,
    IN GT_U32      entryIndex,
    IN GT_U32       operation
)
{
    GT_STATUS   rc;
    GT_PORT_GROUPS_BMP portGroupId = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    static GT_U32  buff_writeHw[MAX_ENTRY_SIZE_CNS];
    static GT_U32  buff_readHw[MAX_ENTRY_SIZE_CNS];
    static GT_U32  buff_readShadow[MAX_ENTRY_SIZE_CNS];
    static GT_U32  compareBitsPtr[MAX_ENTRY_SIZE_CNS];
    static GT_U32  buff_readHw_entry_0[MAX_ENTRY_SIZE_CNS];
    GT_U32  familyTypeGet = utfFamilyTypeGet(devNum);
    GT_U32 ii;
    GT_U32  tmpIndex,numBits,targetStartBit;

    PRV_CPSS_DATA_INTEGRITY_ENUM_STRING_GET_MAC(hwTableType, CPSS_DXCH_TABLE_ENT, strNameBuffer);

    if(operation == CHECK_SHADOW_WITH_SELF_COMPARE)
    {
        GT_U32 entryIndex_0 = 0;

        if(entryIndex == 0)
        {
            /* save original entry 0 values */
            cpssOsMemSet(buff_readHw_entry_0,0,sizeof(buff_readHw_entry_0));
            rc = prvCpssDxChPortGroupReadTableEntry(devNum,portGroupId,
                hwTableType,entryIndex_0,buff_readHw_entry_0);
            if(rc != GT_OK)
            {
                cpssOsPrintf("error 'save original' HW from table[%s] entry[%d] \n",
                    strNameBuffer,entryIndex_0);
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
            rc = prvCpssDxChPortGroupWriteTableEntry(devNum,portGroupId,
                hwTableType,entryIndex_0,buff_readHw_entry_0);
            if(rc != GT_OK)
            {
                cpssOsPrintf("error 'restore' HW to table[%s] entry[%d] \n",
                    strNameBuffer,entryIndex_0);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
            }

            return;
        }
    }

    if(operation == WRITE_ONLY)
    {
        cpssOsMemSet(buff_writeHw,(0xFF-(GT_U8)entryIndex),sizeof(buff_writeHw));

        rc = prvCpssDxChPortGroupWriteTableEntry(devNum,portGroupId,
            hwTableType,entryIndex,buff_writeHw);
        if(rc != GT_OK)
        {
            cpssOsPrintf("error write HW to table[%s] entry[%d] \n",
                strNameBuffer,entryIndex);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        }
    }
    else /* CHECK_SHADOW */
    {
        cpssOsMemSet(buff_readHw,0,sizeof(buff_readHw));
        rc = prvCpssDxChPortGroupReadTableEntry(devNum,portGroupId,
            hwTableType,entryIndex,buff_readHw);
        if(rc != GT_OK)
        {
            cpssOsPrintf("error read HW from table[%s] entry[%d] \n",
                strNameBuffer,entryIndex);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        }

        if(entryIndex == 0)
        {
            /* the read from HW of this entry will give us the 'mask' of applicable bits */
            cpssOsMemCpy(compareBitsPtr,buff_readHw,sizeof(compareBitsPtr));
            /* check if this table is 'special' */
            for(ii = 0 ; special_compareBitsArr[ii].hwTableType != CPSS_DXCH_TABLE_LAST_E; ii++)
            {
                if((special_compareBitsArr[ii].hwTableType == hwTableType) &&
                   (special_compareBitsArr[ii].appFamilyBmp & familyTypeGet))
                {
                    /* the table for the device needs special treatment */
                    cpssOsMemCpy(compareBitsPtr,special_compareBitsArr[ii].special_compareBitsPtr,sizeof(compareBitsPtr));

                    if(CPSS_DXCH_LION_TABLE_TXQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG_E == hwTableType)
                    {
                        numBits = special_compareBitsArr[ii].special_compareBitsPtr[2];
                        for(tmpIndex = 1; tmpIndex < 8; tmpIndex++)
                        {
                            /* need to repeat the 61 bits , 7 more times */
                            targetStartBit = numBits * tmpIndex;

                            /* copy bits from 0 to targetStartBit */
                            copyBitsInMemory(compareBitsPtr,
                                targetStartBit,/*targetStartBit*/
                                0,/*sourceStartBit*/
                                numBits/*numBits*/);
                        }
                    }


                    break;
                }
            }
        }

        /*
        if(entryIndex == 0)
        {
            cpssOsPrintf("check read of table[%s] entry[%d] with HW read values [0x%8.8x][0x%8.8x][0x%8.8x][0x%8.8x]\n",
                strNameBuffer,entryIndex,
                buff_readHw[0],
                buff_readHw[1],
                buff_readHw[2],
                buff_readHw[3]
                );
        }
        */

        prvCpssDxChIndexAsPortNumConvert(devNum, hwTableType, entryIndex,
                                         &tmpIndex /* unused*/, &entryIndex, &hwTableType);

        cpssOsMemSet(buff_readShadow,0,sizeof(buff_readShadow));

        rc = prvCpssDxChPortGroupReadTableEntry_fromShadow(devNum,portGroupId,
            hwTableType,entryIndex,buff_readShadow);
        if(rc != GT_OK)
        {
            cpssOsPrintf("error read Shadow from table[%s] entry[%d] \n",
                strNameBuffer,entryIndex);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        }

        /* compare read from HW and from Shadow */
        for(ii = 0; ii < MAX_ENTRY_SIZE_CNS ; ii++)
        {
            GT_U32  hwValue,shadowValue;

            hwValue     = buff_readHw[ii];
            shadowValue = buff_readShadow[ii];

            /* mask with bits that should be compared*/
            hwValue     &= compareBitsPtr[ii];
            shadowValue &= compareBitsPtr[ii];


            if(hwValue != shadowValue)
            {
                cpssOsPrintf("Shadow and HW different at table[%s] entry[%d] word[%d] : HW[0x%8.8x] shadow[0x%8.8x]\n",
                    strNameBuffer,entryIndex,
                    ii,
                     buff_readHw[ii],
                     buff_readShadow[ii]);

                if(compareBitsPtr[ii] != 0xFFFFFFFF)
                {
                    cpssOsPrintf("with compareBits[0x%8.8x] : HW[0x%8.8x] shadow[0x%8.8x]\n",
                        compareBitsPtr[ii],hwValue,shadowValue);
                }

                UTF_VERIFY_EQUAL1_PARAM_MAC(hwValue, shadowValue,devNum);
            }
        }
    }

    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, devNum);
}

static CPSS_DXCH_TABLE_ENT debugSpecificTable = CPSS_DXCH_TABLE_LAST_E;
/* test the shadow of the CPSS :
    set entries in the HW table and check that read from HW match read from shadow.

*/
static void cpssDxChDiagDataIntegrityTables_shadowTest_operation(
    IN GT_U8               devNum,
    IN GT_U32       operation
)
{
    GT_STATUS   rc;
    CPSS_DXCH_TABLE_ENT hwTableType;
    GT_U32     maxTableSize;
    GT_U32  index;
    GT_U32  mainOperation = operation;
    GT_U32  origOperation = operation;

    if(origOperation == CHECK_SHADOW_WITH_SELF_COMPARE)
    {
        mainOperation = CHECK_SHADOW;
    }

    for(hwTableType = 0 ; hwTableType < CPSS_DXCH_TABLE_LAST_E; hwTableType++)
    {
        if(debugSpecificTable != CPSS_DXCH_TABLE_LAST_E &&
           debugSpecificTable != hwTableType)
        {
            /* the debug process not need this table */
            continue;
        }

        if(GT_FALSE ==
            prvCpssDxChDiagDataIntegrityTableCheckIsDevSupportHwTable(devNum,hwTableType,
            0))
        {
            /* the device not supports this HW table */
            continue;
        }

        if (hwTableType == CPSS_DXCH_SIP5_TABLE_TM_EGR_GLUE_TARGET_INTERFACE_E &&
            GT_FALSE == PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.featureInfo.TmSupported)
        {
            /* skip this table if TM is disabled */
            continue;
        }

        if(CPSS_DXCH_SHADOW_TYPE_NONE_E ==
            prvCpssDxChDiagDataIntegrityTableCheckIsDevSupportHwTableShadow(devNum,hwTableType))
        {
            /* the device not supports shadow for the table */
            continue;
        }

        /* get number of entries in the table */
        rc = prvCpssDxChDiagDataIntegrityTableHwMaxIndexGet(devNum,hwTableType,&maxTableSize,NULL);
        if(rc != GT_OK)
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, devNum);
        }

        if(origOperation == CHECK_SHADOW_WITH_SELF_COMPARE)
        {
            index = 0;
            cpssDxChDiagDataIntegrityTables_shadowTest_entry(devNum,hwTableType,index,origOperation);
        }

        for(index = 0 ; index < maxTableSize; index++)
        {
            if(GT_FALSE ==
                prvCpssDxChDiagDataIntegrityTableCheckIsDevSupportHwTable(devNum,hwTableType,
                index))
            {
                /* the HW table not support this index */
                continue;
            }

            cpssDxChDiagDataIntegrityTables_shadowTest_entry(devNum,hwTableType,index,mainOperation);

            if(utfErrorCountGet())
            {
                didError_shadowTest_onDemand = GT_TRUE;
                break;
            }
        }

        if(origOperation == CHECK_SHADOW_WITH_SELF_COMPARE)
        {
            index = 1;/* non-zero ... to restore values in index 0 */
            cpssDxChDiagDataIntegrityTables_shadowTest_entry(devNum,hwTableType,index,origOperation);
        }
    }
}

UTF_TEST_CASE_MAC(cpssDxChDiagDataIntegrityTables_shadowTest)
{
    GT_U8   devNum;
    GT_U32  devCounter = 0;
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION2_E |
                                           UTF_LION_E | UTF_XCAT2_E);

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
        cpssDxChDiagDataIntegrityTables_shadowTest_operation(devNum,WRITE_ONLY);
        /* read from all tables on all entries and compare to shadow */
        cpssDxChDiagDataIntegrityTables_shadowTest_operation(devNum,CHECK_SHADOW);

        cpssOsPrintf("device[%d] finished \n", devNum);
    }

    if(0 == devCounter)
    {
        /* state that the test skipped */
        SKIP_TEST_MAC;
    }

}
/* function needed because MACRO is good for function that return 'void'
but not for function that return GT_BOOL like cpssDxChDiagDataIntegrityTables_shadowTest_onDemand */
static void cpssDxChDiagDataIntegrityTables_shadowTest_onDemand_getFirstDev_restDevIterator(OUT GT_U8 *devNumPtr)
{
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(devNumPtr, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION2_E |
                                           UTF_LION_E | UTF_XCAT2_E);
}

/* return indication of error :
    GT_TRUE  - error
    GT_FALSE - no error

*/
GT_BOOL cpssDxChDiagDataIntegrityTables_shadowTest_onDemand(void)
{
    GT_U8   devNum;

    utfErrorCountGet();/*reset the error count*/
    didError_shadowTest_onDemand = GT_FALSE;

    if(GT_TRUE == prvUtfIsGmCompilation())
    {
        return didError_shadowTest_onDemand;
    }

    cpssDxChDiagDataIntegrityTables_shadowTest_onDemand_getFirstDev_restDevIterator(&devNum);

    /* run only on first device that iterator returns */
    if(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        cpssOsPrintf("device[%d] started to test shadow \n", devNum);

        /* check that all tables in HW with same values as in shadow */
        cpssDxChDiagDataIntegrityTables_shadowTest_operation(devNum,CHECK_SHADOW_WITH_SELF_COMPARE);

        cpssOsPrintf("device[%d] finished to test shadow \n", devNum);

        if(didError_shadowTest_onDemand == GT_TRUE)
        {
            /* fix errors so the next tests will not fail on the same tables that already found */
            prvCpssDxChDiagDataIntegrityShadowAndHwSynch(devNum);
        }
    }

    return didError_shadowTest_onDemand;
}

GT_BOOL cpssDxChDiagDataIntegrityTables_shadowTest_onDemand_specificTable
(
    IN GT_U8   devNum,
    IN CPSS_DXCH_TABLE_ENT table
)
{
    didError_shadowTest_onDemand = GT_FALSE;

    if(GT_TRUE == prvUtfIsGmCompilation())
    {
        return didError_shadowTest_onDemand;
    }

    debugSpecificTable = table;

    /* run the check */
    cpssDxChDiagDataIntegrityTables_shadowTest_operation(devNum,CHECK_SHADOW_WITH_SELF_COMPARE);

    /* restore 'no debug' */
    debugSpecificTable = CPSS_DXCH_TABLE_LAST_E;


    return didError_shadowTest_onDemand;
}

/*
    code not ready for the test

    #define TEST_FIX_ERROR
*/

#ifdef TEST_FIX_ERROR

#define SET_HW_AND_SHADOW       5
#define SET_HW_AND_NOT_SHADOW   6
#define GET_HW_MASK_ON_TABLE    7
#define FIX_HW_BY_SHADOW        8

#define TEST_FIX                9/* operation to do GET_HW_MASK_ON_TABLE and FIX_HW_BY_SHADOW */

/* test the shadow of the CPSS :
    set entries in the HW table and check that read from HW match read from shadow.

*/
static void cpssDxChDiagDataIntegrityTables_FixErrorTest_dev_entry(
    IN GT_U8               devNum,
    IN CPSS_DXCH_TABLE_ENT hwTableType,
    IN GT_U32      entryIndex,
    IN GT_U32       operation
)
{
    GT_STATUS   rc = GT_BAD_PARAM;
    GT_PORT_GROUPS_BMP portGroupId = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    static GT_U32  buff_writeHw[MAX_ENTRY_SIZE_CNS];
    static GT_U32  buff_readHw[MAX_ENTRY_SIZE_CNS];
    static GT_U32  buff_readShadow[MAX_ENTRY_SIZE_CNS];
    static GT_U32  compareBitsPtr[MAX_ENTRY_SIZE_CNS];
    GT_U32  familyTypeGet = utfFamilyTypeGet(devNum);
    GT_U32 ii;
    GT_U32  tmpIndex,numBits,targetStartBit;
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            specificLocation;

    PRV_CPSS_DATA_INTEGRITY_ENUM_STRING_GET_MAC(hwTableType, CPSS_DXCH_TABLE_ENT, strNameBuffer);

    if(operation == SET_HW_AND_SHADOW ||
       operation == SET_HW_AND_NOT_SHADOW)/* the caller disconnected the update of the HW from shadow */
    {
        if(operation == SET_HW_AND_NOT_SHADOW)/* the caller disconnected the update of the HW from shadow */
        {
            /* setting different values into HW from what shadow holds */
            cpssOsMemSet(buff_writeHw,((GT_U8)entryIndex),sizeof(buff_writeHw));
        }
        else
        {
            cpssOsMemSet(buff_writeHw,(0xFF-(GT_U8)entryIndex),sizeof(buff_writeHw));
        }

        rc = prvCpssDxChPortGroupWriteTableEntry(devNum,portGroupId,
            hwTableType,entryIndex,buff_writeHw);
        if(rc != GT_OK)
        {
            cpssOsPrintf("error write HW to table[%s] entry[%d] \n",
                strNameBuffer,entryIndex);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        }
    }
    else
    if(operation == GET_HW_MASK_ON_TABLE)
    {
        if(entryIndex == 0)
        {
            cpssOsMemSet(buff_readHw,0,sizeof(buff_readHw));
            rc = prvCpssDxChPortGroupReadTableEntry(devNum,portGroupId,
                hwTableType,entryIndex,buff_readHw);
            if(rc != GT_OK)
            {
                cpssOsPrintf("error read HW from table[%s] entry[%d] \n",
                    strNameBuffer,entryIndex);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
            }

            /* entry 0 was set with '0xFF' on all 'bytes' */

            /* the read from HW of this entry will give us the 'mask' of applicable bits */
            cpssOsMemCpy(compareBitsPtr,buff_readHw,sizeof(compareBitsPtr));
            /* check if this table is 'special' */
            for(ii = 0 ; special_compareBitsArr[ii].hwTableType != CPSS_DXCH_TABLE_LAST_E; ii++)
            {
                if((special_compareBitsArr[ii].hwTableType == hwTableType) &&
                   (special_compareBitsArr[ii].appFamilyBmp & familyTypeGet))
                {
                    /* the table for the device needs special treatment */
                    cpssOsMemCpy(compareBitsPtr,special_compareBitsArr[ii].special_compareBitsPtr,sizeof(compareBitsPtr));

                    if(CPSS_DXCH_LION_TABLE_TXQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG_E == hwTableType)
                    {
                        numBits = special_compareBitsArr[ii].special_compareBitsPtr[2];
                        for(tmpIndex = 1; tmpIndex < 8; tmpIndex++)
                        {
                            /* need to repeat the 61 bits , 7 more times */
                            targetStartBit = numBits * tmpIndex;

                            /* copy bits from 0 to targetStartBit */
                            copyBitsInMemory(compareBitsPtr,
                                targetStartBit,/*targetStartBit*/
                                0,/*sourceStartBit*/
                                numBits/*numBits*/);
                        }
                    }


                    break;
                }
            }
        }
    }
    else /* operation == FIX_HW_BY_SHADOW */
    {
        cpssOsMemSet(buff_readHw,0,sizeof(buff_readHw));
        rc = prvCpssDxChPortGroupReadTableEntry(devNum,portGroupId,
            hwTableType,entryIndex,buff_readHw);
        if(rc != GT_OK)
        {
            cpssOsPrintf("error read HW from table[%s] entry[%d] \n",
                strNameBuffer,entryIndex);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        }

        cpssOsMemSet(buff_readShadow,0,sizeof(buff_readShadow));
        rc = prvCpssDxChPortGroupReadTableEntry_fromShadow(devNum,portGroupId,
            hwTableType,entryIndex,buff_readShadow);
        if(rc != GT_OK)
        {
            cpssOsPrintf("error read Shadow from table[%s] entry[%d] \n",
                strNameBuffer,entryIndex);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        }

        /* compare read from HW and from Shadow -- make sure 'not equal' */
        for(ii = 0; ii < MAX_ENTRY_SIZE_CNS ; ii++)
        {
            GT_U32  hwValue,shadowValue;

            hwValue     = buff_readHw[ii];
            shadowValue = buff_readShadow[ii];

            /* mask with bits that should be compared*/
            hwValue     &= compareBitsPtr[ii];
            shadowValue &= compareBitsPtr[ii];


            if(hwValue != shadowValue)
            {
                break;
            }
        }

        if (ii == MAX_ENTRY_SIZE_CNS)
        {
            cpssOsPrintf("Shadow and HW are the same at table[%s] entry[%d] \n",
                strNameBuffer,entryIndex);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, GT_BAD_PARAM, devNum);
        }

        specificLocation.portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
        specificLocation.type = CPSS_DXCH_LOCATION_HW_INDEX_INFO_TYPE;
        specificLocation.info.hwEntryInfo.hwTableType = hwTableType;
        specificLocation.info.hwEntryInfo.hwTableEntryIndex = entryIndex;

        rc = cpssDxChDiagDataIntegrityTableEntryFix(devNum,&specificLocation);
        if(rc != GT_OK)
        {
            cpssOsPrintf("error fix from Shadow from table[%s] entry[%d] \n",
                strNameBuffer,entryIndex);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        }

    }

    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, devNum);
}

/* test the FIX of the CPSS :
    get entries out of synch from shadow and check that FIX sets shadow value.

*/
static void cpssDxChDiagDataIntegrityTables_FixErrorTest_dev(
    IN GT_U8               devNum,
    IN GT_U32       operation
)
{
    GT_STATUS   rc;
    CPSS_DXCH_TABLE_ENT hwTableType;
    GT_U32     maxTableSize;
    GT_U32  index;

    for(hwTableType = 0 ; hwTableType < CPSS_DXCH_TABLE_LAST_E; hwTableType++)
    {
        if(GT_FALSE ==
            prvCpssDxChDiagDataIntegrityTableCheckIsDevSupportHwTable(devNum,hwTableType,
            0))
        {
            /* the device not supports this HW table */
            continue;
        }

        if(CPSS_DXCH_SHADOW_TYPE_NONE_E ==
            prvCpssDxChDiagDataIntegrityTableCheckIsDevSupportHwTableShadow(devNum,hwTableType))
        {
            /* the device not supports shadow for the table */
            continue;
        }

        if (GT_TRUE == prvCpssDxChDiagDataIntegrityIsMultipleNames(hwTableType, NULL))
        {
            /* skip all tables except one in case if they are physically the same HW table.
               example: CPSS_DXCH_SIP5_TABLE_IP_MLL_E, CPSS_DXCH_SIP5_TABLE_L2_MLL_E - is
               the same table in PP (at least in SIP5 devices) */
            continue;
        }

        /* get number of entries in the table */
        rc = prvCpssDxChDiagDataIntegrityTableHwMaxIndexGet(devNum,hwTableType,&maxTableSize,NULL);
        if(rc != GT_OK)
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, devNum);
        }

        if(operation == TEST_FIX)
        {
            index = 0;
            cpssDxChDiagDataIntegrityTables_FixErrorTest_dev_entry(devNum,hwTableType,index,GET_HW_MASK_ON_TABLE);
        }

        for(index = 0 ; index < maxTableSize; index++)
        {
            if(GT_FALSE ==
                prvCpssDxChDiagDataIntegrityTableCheckIsDevSupportHwTable(devNum,hwTableType,
                index))
            {
                /* the HW table not support this index */
                continue;
            }

            if(operation != SET_HW_AND_SHADOW && index == 0)
            {
                /* do not change/quarry index 0 ... because it is used for 'mask' of
                   the valid bits in the table */
                continue;
            }


            if(operation == TEST_FIX)
            {
                cpssDxChDiagDataIntegrityTables_FixErrorTest_dev_entry(devNum,hwTableType,index,FIX_HW_BY_SHADOW);
            }
            else
            {
                cpssDxChDiagDataIntegrityTables_FixErrorTest_dev_entry(devNum,hwTableType,index,operation);
            }

            if(utfErrorCountGet())
            {
                break;
            }
        }

    }
}

extern void debugMode_cpssDxChDiagDataIntegrity_forceNoUpdateOfShadow
(
    IN GT_U32  forceNoUpdateOfShadow
);

UTF_TEST_CASE_MAC(cpssDxChDiagDataIntegrityTableEntryFix_1)
{
    GT_U8   devNum;
    GT_U32  devCounter = 0;
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION2_E |
                                           UTF_LION_E | UTF_XCAT2_E);

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

        devCounter++;

        /* set HW and shadow with same values */
        cpssDxChDiagDataIntegrityTables_FixErrorTest_dev(devNum,SET_HW_AND_SHADOW);

        /* set 'debug' state to 'DO not update shadow' */
        debugMode_cpssDxChDiagDataIntegrity_forceNoUpdateOfShadow(1);

        /* update HW only (no change in shadow) */
        cpssDxChDiagDataIntegrityTables_FixErrorTest_dev(devNum,SET_HW_AND_NOT_SHADOW);

        /* restore 'debug' state */
        debugMode_cpssDxChDiagDataIntegrity_forceNoUpdateOfShadow(0);

        /* test that 'fix' restore HW to original values */
        cpssDxChDiagDataIntegrityTables_FixErrorTest_dev(devNum,TEST_FIX);
    }

    if(0 == devCounter)
    {
        /* state that the test skipped */
        SKIP_TEST_MAC;
    }

}
#endif /*TEST_FIX_ERROR*/

#ifndef ASIC_SIMULATION


/* value that can't be found in the DataIntegrity DB */
#define PRV_BAD_PORT_GROUP_CNS 0xFFFFFFFC

/* get width of RAM(s) composing the table */
static GT_STATUS prvDataIntegrityHwTableRamWidthGet
(
    IN GT_U8                devNum,
    IN CPSS_DXCH_TABLE_ENT  tableType, /* GT_U32 ? */
    OUT GT_U32             *ramWidthPtr
)
{
    PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC *dbPtr;
    GT_U32                                        dbSize;
    GT_U32                                        i;
    GT_STATUS                                     rc;

    rc = prvCpssDxChDiagDataIntegrityHwTableGet(devNum, &tableType, NULL, NULL);
    if (rc != GT_OK)
    {
        return rc;
    }

    prvCpssDxChDiagDataIntegrityDbPointerSet(devNum, &dbPtr, &dbSize);

    for (i = 0; i< dbSize && dbPtr->hwTableName != tableType; i++, dbPtr++);

    *ramWidthPtr =  (i == dbSize) ?
        0 : (dbPtr->lastTableDataBit - dbPtr->firstTableDataBit + 1);

    return GT_OK;
}


/* it is possible that injection info HW entry will produce events with
 * HW entry index different than injected. */
static GT_VOID prvDataIntegrityEventExpectedHwEntryClarify
(
    IN    GT_U8                dev,
    INOUT CPSS_DXCH_TABLE_ENT  *tableTypePtr,
    IN    GT_U32               ramWidth,
    INOUT GT_U32               *entryIndexPtr,
    OUT   PRV_DATA_INTEGRITY_EVENT_MATCH_TYPE_ENT *matchTypePtr
)
{
    GT_U32                                  entryIndex = *entryIndexPtr;
    GT_U32                                  firstBit = 0;
    GT_U32                                  entryWidth;
    GT_U32                                  entriesPerLine;
    PRV_DATA_INTEGRITY_EVENT_MATCH_TYPE_ENT matchType = PRV_DATA_INTEGRITY_EVENT_MATCH_TYPE_ANY_E;
    GT_U32                                  lineIndex = *entryIndexPtr;
    CPSS_DXCH_LPM_RAM_MEM_MODE_ENT          lpmMode; /* sip 5.20 */
    GT_STATUS                               rc;

    if (PRV_CPSS_SIP_5_20_CHECK_MAC(dev) &&
        *tableTypePtr == CPSS_DXCH_SIP5_20_TABLE_LPM_MEM_PIPE_1_E)
    {
        rc = cpssDxChLpmMemoryModeGet(dev, &lpmMode);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        if (CPSS_DXCH_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E == lpmMode)
        {
            *tableTypePtr = CPSS_DXCH_SIP5_TABLE_LPM_MEM_E;
        }
    }

    if (GT_TRUE == prvCpssDxChTableEngineMultiEntriesInLineIndexAndGlobalBitConvert(
            dev, *tableTypePtr, &lineIndex, &firstBit, &entryWidth, &entriesPerLine)
        && entryWidth != 0)
    {

        /* if single RAM line contains several entries first one will be returned.*/
        entryIndex -= (firstBit % ramWidth) / entryWidth;
        if (PRV_CPSS_SIP_5_20_CHECK_MAC(dev) &&
            *tableTypePtr == CPSS_DXCH_SIP5_TABLE_ERMRK_PTP_LOCAL_ACTION_E)
        {
            /* entriesPerLine = 8, but in fact there are only 6 entries. */
            entriesPerLine = 6;
        }

        /* if single table line contains several table entries, writing the
         * single entry causes updating full table line. If the line is
         * composed of sevaral RAMs there will be several erroneous entries. */
        if (!(entriesPerLine & (FRACTION_INDICATION_CNS |
                                FRACTION_HALF_TABLE_INDICATION_CNS)))
        {
            if (entriesPerLine & VERTICAL_INDICATION_CNS)
            {
                entriesPerLine -= VERTICAL_INDICATION_CNS;
            }

            if (entriesPerLine > 1)
            {
                matchType = PRV_DATA_INTEGRITY_EVENT_MATCH_TYPE_ANY_E;
            }
        }
    }
    *entryIndexPtr = entryIndex;
    *matchTypePtr  = matchType;
}

#define PRV_UTF_ERROR_INJECT_LOOPS_PER_MEM 7
UTF_TEST_CASE_MAC(cpssDxChDiagDataIntegrityHwTablesInfoGetTest)
{
/*
    ITERATE_DEVICES (Bobcat2, Aldrin, AC3X, Bobcat3)
*/
    GT_STATUS           st = GT_OK;
    GT_U8               dev;
    CPSS_DXCH_TABLE_ENT tableType;
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType;
    CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errType;
    GT_U32                                          entryIndex;
    GT_U32                                          expectedEntryIndex;
    GT_U32                                          i;
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            locationInfo;
    GT_U32                                          nextEntryIndex;
    GT_BOOL                                         wasWrapAround;
    GT_U32                                          maxEntries;
    GT_U32                                          scanEntries;
    GT_U32                                          *tableEntryPtr = &tableEntry[0];
    DXCH_DATA_INTEGRITY_EVENT_CB_FUNC               *savedDataIntegrityHandler;
    CPSS_DXCH_TABLE_ENT                             lastTableType;
    GT_U32                                          tblsCount;
    GT_BOOL                                         tbScanDisable;
    GT_U32                                          regAddr;
    GT_U32                                          savedRegData;
    PRV_DATA_INTEGRITY_EVENT_MATCH_TYPE_ENT         matchType;
    GT_U32                                          ramWidth = 0;
    CPSS_DXCH_TABLE_ENT                             expTableType;
    GT_U32                                          readEntryBuf[MAX_ENTRY_SIZE_CNS];

    /* AUTODOC: prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E  | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E  | UTF_CH3_E   |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E | UTF_XCAT3_E |
                                           UTF_FALCON_E);

    cpssOsMemSet(tableEntryPtr, 0xFF, sizeof(tableEntry));

    /* AUTODOC: save data integrity error callback. */
    savedDataIntegrityHandler = dxChDataIntegrityEventIncrementFunc;

    /* AUTODOC: set own callback */
    dxChDataIntegrityEventIncrementFunc = hwInfoEventErrorHandlerFunc;

    locationInfo.portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    locationInfo.type        = CPSS_DXCH_LOCATION_HW_INDEX_INFO_TYPE;

    /* AUTODOC:  go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        if (PRV_CPSS_DXCH_CETUS_CHECK_MAC(dev) ||
            PRV_CPSS_DXCH_CAELUM_CHECK_MAC(dev))
        {
            st = prvUtfDataIntegrityEplrZeroTablesAndResetInt(dev);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        }

        /* AUTODOC: Clear captured events */
        prvCpssDxChDiagDataIntegrityEventsClear();

        lastTableType =
            PRV_CPSS_SIP_5_25_CHECK_MAC(dev) ? CPSS_DXCH_SIP5_25_TABLE_LAST_E :
            PRV_CPSS_SIP_5_20_CHECK_MAC(dev) ? CPSS_DXCH_SIP5_20_TABLE_LAST_E :
            PRV_CPSS_SIP_5_15_CHECK_MAC(dev) ? CPSS_DXCH_SIP5_15_TABLE_LAST_E :
            CPSS_DXCH_BOBCAT2_TABLE_LAST_E;

        tblsCount = 0;

        /* AUTODOC: loop over all memories and inject errors */
        for(tableType = CPSS_DXCH_TABLE_VLAN_PORT_PROTOCOL_E; tableType < lastTableType; tableType++)
        {
            if ((selectedTableType != 0xFFFFFFFF) && (selectedTableType != tableType))
            {
                continue;
            }

            PRV_CPSS_DATA_INTEGRITY_ENUM_STRING_GET_MAC(tableType, CPSS_DXCH_TABLE_ENT, strNameBuffer);
            PRV_UTF_LOG1_MAC("Test table %s\n", strNameBuffer);

            st = prvUtfDataIntegrityErrorInjectionTablesSkipCheck(dev, tableType, &memType, &errType, &maxEntries);
            if (st != GT_OK)
            {
                if (traceEvents)
                {
                    switch (st)
                    {
                        case GT_NOT_FOUND:
                            cpssOsPrintf("%s - not found in DB\r\n", strNameBuffer);
                            break;
                        case GT_NOT_SUPPORTED:
                            cpssOsPrintf("%s - not exists in SIP5\r\n", strNameBuffer);
                            break;
                        case GT_NOT_IMPLEMENTED:
                            cpssOsPrintf("%s - not used in test\r\n", strNameBuffer);
                            break;
                        case GT_BAD_VALUE:
                            cpssOsPrintf("%s - not protected table\r\n", strNameBuffer);
                            break;
                    }
                }

                if (st == GT_FAIL)
                {
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "Error for table %s\n", strNameBuffer );
                }

                continue;
            }

            /* take care cases when one RAM includes several HW entries or
               several RAM lines is one HW entry  */
            st = prvDataIntegrityHwTableRamWidthGet(dev, tableType, &ramWidth);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            tbScanDisable = GT_FALSE;
            tblsCount++;

            if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
            {
                /* the device not supports those SIP5 TXQ features */
            }
            else
            if (memType == CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PORT_E
                ||
                memType == CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PRIO_E)
            {
                /* this memories are updated constantly by device. Disable shaper
                   tocken bucket update to guarantee that Error Injection bit
                   will not be reset by a memory update before we read it */
                tbScanDisable = GT_TRUE;

                regAddr = PRV_DXCH_REG1_UNIT_TXQ_DQ_MAC(dev).shaper.tokenBucketUpdateRate;
                st = prvCpssHwPpReadRegister(dev, regAddr, &savedRegData);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                if (st != GT_OK)
                {
                    /* to avoid restoring register's data later.  */
                    tbScanDisable = GT_FALSE;
                }

                /* set TBScanEn to zero */
                st = prvCpssHwPpSetRegField(dev, regAddr, 29, 1, 0);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            }

            if (selectedTableType != 0xFFFFFFFF)
            {
                PRV_UTF_LOG0_MAC("cpssDxChDiagDataIntegrityErrorCountEnableSet:\n");
                cpssOsTimerWkAfter(5);
            }

            locationInfo.info.hwEntryInfo.hwTableType = tableType;

            /* AUTODOC: enable error counter */
            st = cpssDxChDiagDataIntegrityErrorCountEnableSet(dev, &locationInfo, errType, GT_TRUE);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            if (selectedTableType != 0xFFFFFFFF)
            {
                PRV_UTF_LOG0_MAC("cpssDxChDiagDataIntegrityEventMaskSet:\n");
                cpssOsTimerWkAfter(5);
            }

            /* AUTODOC: unmask interrupt for DFX memory */
            st = cpssDxChDiagDataIntegrityEventMaskSet(dev, &locationInfo, errType, CPSS_EVENT_UNMASK_E);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            for (i = 0; i < PRV_UTF_ERROR_INJECT_LOOPS_PER_MEM; i++)
            {
                /* AUTODOC: calculate entry index and write table entry field */
                entryIndex = (PRV_UTF_ERROR_INJECT_LOOPS_PER_MEM - i - 1) * (maxEntries - 1) / (PRV_UTF_ERROR_INJECT_LOOPS_PER_MEM - 1);

                switch (tableType)
                {
                    case CPSS_DXCH_SIP5_TABLE_TCAM_PCL_TTI_ACTION_E:
                    case CPSS_DXCH_TABLE_VLAN_PORT_PROTOCOL_E:
                        /* AUTODOC: avoid not valid entries - 4 LSB must be < 12 */
                        entryIndex = entryIndex & 0xFFFFFFF0;
                        break;
                    case CPSS_DXCH_SIP5_TABLE_LPM_MEM_E:
                        if (PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(dev) ||
                            PRV_CPSS_DXCH_CETUS_CHECK_MAC(dev))
                        {
                            /* AUTODOC: avoid not valid entries -
                               only first 2K of each 16K window are valid */
                            entryIndex = entryIndex & 0xFFFFC7FF;
                        } else if (PRV_CPSS_DXCH_CAELUM_CHECK_MAC(dev))
                        {
                            /* AUTODOC: avoid not valid entries -
                               only first 6K of each 16K window are valid */
                            entryIndex = (entryIndex / _16K) + (entryIndex % _6K);
                        } else if (PRV_CPSS_DXCH_ALDRIN2_CHECK_MAC(dev))
                        {
                            /* AUTODOC: avoid not valid entries -
                               only first 8K of each 16K window are valid */
                            entryIndex = entryIndex % _8K;
                        }
                        break;
                    case CPSS_DXCH_SIP5_TABLE_ERMRK_PTP_LOCAL_ACTION_E:
                        /* indexes appropriating PTP domains 5..7 are not valid
                         * in this table for all SIP5 devices */
                        if (entryIndex % 8 > 4)
                        {
                            entryIndex -= 4;
                        }
                    case CPSS_DXCH_SIP5_20_TABLE_LPM_MEM_PIPE_1_E:
                        if (PRV_CPSS_DXCH_ALDRIN2_CHECK_MAC(dev))
                        {
                            /* AUTODOC: avoid not valid entries -
                               only first 8K of each 16K window are valid */
                            entryIndex = entryIndex % _8K;
                        }
                        break;
                    default:
                        break;
                }

                /* AUTODOC: check entry index for HW table */
                if(GT_FALSE ==
                    prvCpssDxChDiagDataIntegrityTableCheckIsDevSupportHwTable(dev, tableType, entryIndex))
                {
                    PRV_CPSS_DATA_INTEGRITY_ENUM_STRING_GET_MAC(tableType, CPSS_DXCH_TABLE_ENT, strNameBuffer);
                    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, GT_FAIL, "Bad index %d in iter %d for table %s\n", entryIndex, i, strNameBuffer);
                    continue;
                }

                /* get exect memType for SIP5 devices (except bobcat3) Ingress Policer 0/1 tables
                   CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_[0|1]_METERING_E
                   CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_[0|1]_COUNTING_E
                   For other tables/devices do nothing.
                 */
                st = prvDxChDiagDataIntegrityMemTypeUpdate(dev, tableType, entryIndex, &memType);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                if (selectedTableType != 0xFFFFFFFF)
                {
                    PRV_UTF_LOG0_MAC("cpssDxChDiagDataIntegrityErrorInjectionConfigSet:\n");
                    cpssOsTimerWkAfter(5);
                }

                /* AUTODOC: enable error injection */
                st = cpssDxChDiagDataIntegrityErrorInjectionConfigSet(dev, &locationInfo, CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E, GT_TRUE);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                if (selectedTableType != 0xFFFFFFFF)
                {
                    PRV_UTF_LOG0_MAC("prvCpssDxChPortGroupWriteTableEntry\n");
                    cpssOsTimerWkAfter(5);
                }

                /* AUTODOC: write table entry */
                if (PRV_CPSS_SIP_5_20_CHECK_MAC(dev) &&
                    tableType >= CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E &&
                    tableType <= CPSS_DXCH_SIP5_TABLE_CNC_31_COUNTERS_E)
                {
                    /* It's prohibited to write to CNC memory in BC3 and above. Device stuck.
                       Need to use read operation instead. The read operation
                       writes to CNC memory also because it's clear on read.*/
                    st = prvCpssDxChPortGroupReadTableEntry(
                        dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                        tableType, entryIndex, &readEntryBuf[0]);
                }
                else
                {
                    st = prvCpssDxChPortGroupWriteTableEntry(
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
                    PRV_UTF_LOG0_MAC("cpssDxChDiagDataIntegrityTableScan:\n");
                    cpssOsTimerWkAfter(5);
                }

                /* AUTODOC: scan whole table */
                st = cpssDxChDiagDataIntegrityTableScan(dev, &locationInfo, scanEntries,
                                                        &nextEntryIndex, &wasWrapAround);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /* AUTODOC: write table entry again to reset erroneous bit. If don't do this
                 * next scenario is likely to happen for memories with disabled auto-correction:
                 * Inject single error into entry #0. Scan full table. Error is captured
                 * for entry #0. Good.
                 *
                 * Inject single error into entry #100. scan full table. Two errors are
                 * captured: for entries #100 (good), #0 - from previous iteration (bad) */
                if (!(PRV_CPSS_SIP_5_20_CHECK_MAC(dev) &&
                      tableType >= CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E &&
                      tableType <= CPSS_DXCH_SIP5_TABLE_CNC_31_COUNTERS_E))
                {
                    /* It's prohibited to write to CNC memory in BC3 and above. Device stuck. */
                    st = prvCpssDxChPortGroupWriteTableEntry(
                        dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                        tableType, entryIndex, tableEntryPtr);
                    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
                }

                /* determine table entry index which we expect to find in DataIntegrity event  */
                expectedEntryIndex = entryIndex;
                expTableType = tableType;
                prvDataIntegrityEventExpectedHwEntryClarify(dev, &expTableType, ramWidth,
                                                            &expectedEntryIndex,
                                                            &matchType);
                /* AUTODOC: sleep 10 mSec */
                cpssOsTimerWkAfter(timeOut);

                /* AUTODOC: check specific event for current memory */
                st = prvCpssDxChDiagDataIntegrityEventHwInfoDbCheck(
                    errType, memType, expTableType, expectedEntryIndex, matchType);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, tableType, entryIndex);

                prvCpssDxChDiagDataIntegrityEventsClear();

                if (selectedTableType != 0xFFFFFFFF)
                {
                    PRV_UTF_LOG0_MAC("prvCpssDxChDiagDataIntegrityEventHwInfoDbCheck:\n");
                    cpssOsTimerWkAfter(5);
                }
            }

            /* AUTODOC: disable error injection */
            st = cpssDxChDiagDataIntegrityErrorInjectionConfigSet(dev, &locationInfo, CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E, GT_FALSE);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* AUTODOC: Mask DFX memory interrupt */
            st = cpssDxChDiagDataIntegrityEventMaskSet(dev, &locationInfo, errType, CPSS_EVENT_MASK_E);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* restore modified register value */
            if (tbScanDisable)
            {
                regAddr = PRV_DXCH_REG1_UNIT_TXQ_DQ_MAC(dev).shaper.tokenBucketUpdateRate;
                st = prvCpssHwPpWriteRegister(dev, regAddr, savedRegData);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                tbScanDisable = GT_FALSE;
            }
        }
        PRV_UTF_LOG2_MAC("Device %d Number tested HW Tables %d:\n", dev, tblsCount);
    }

    /* AUTODOC: Restore dxChDataIntegrityEventIncrementFunc */
    dxChDataIntegrityEventIncrementFunc = savedDataIntegrityHandler;
}



/**
* @internal prvCpssDxChDiagDataIntegrityCheckTsEvents function
* @endinternal
*
* @brief   Functions receives a struct describing Tunnel-Start HW table related
*         Data Integrity events and logical coordinates where the events occured.
*         The function checks that event occured in specified entries.
* @param[in] dev                      - device number
* @param[in] eventsPtr                - Data Integrity events info
* @param[in] expectedErrorType        - expected event's error type
* @param[in] expectedArpIndex         - ARP index for search in events
* @param[in] expectedTsNatIndex       - TS/NAT index for search in events
*
* @retval GT_OK                    - specified indexes are found
* @retval GT_FAIL                  - specified indexes aren't found
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityCheckTsEvents
(
    IN GT_U8                                          dev,
    IN PRV_DXCH_DATA_INTEGRITY_TS_EVENTS_STC         *eventsPtr,
    IN CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT  expectedErrorType,
    IN GT_U32                                         expectedArpIndex,
    IN GT_U32                                         expectedTsNatIndex
)
{

    GT_U32                            eventsIter;
    GT_U32                            tabIter;
    GT_U32                            expectedEntryIndex;
    GT_U32                            expectedNumEntries;
    GT_BOOL                           isArpIndexFound     = GT_FALSE;
    GT_BOOL                           isTsIndexFound      = GT_FALSE;
    GT_BOOL                           isNatIndexFound     = GT_FALSE;
    GT_CHAR                          *logicalTableNameStr = NULL;
    GT_BOOL                          *foundFlagPtr        = NULL;
    CPSS_DXCH_LOGICAL_TABLE_INFO_STC *logicalPtr          = NULL;
    GT_U32                            indexRangeMin;
    GT_U32                            indexRangeMax;
    GT_U32                            isDevBobkOrAldrin;
    PRV_UTF_LOG1_MAC("Total number of events occured: %d\n",
                     eventsPtr->allEventsCount);


    if (eventsPtr->tsEventsCount == 0)
    {
        PRV_UTF_LOG1_MAC("Device %d. No Tunnel-Start HW Table events", dev);
        return GT_FAIL;
    }

    /* We expect to find all 3 logical tables - Tunnel-Start, ARP, NAT -
     * inside of every Data Integrity Event related to Tunnel Start HW table.
     * Several events can occur (because 1 HW entry consists of 6 memories).
     * But expected ARP entry number or TS entry number must be found
     * inside some of this events. */

    isDevBobkOrAldrin = (PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(dev) ||
                         IS_BOBK_DEV_MAC(dev));

    for (eventsIter=0; eventsIter < eventsPtr->tsEventsCount; eventsIter++)
    {
        if (expectedErrorType != eventsPtr->eventsType[0])
        {
            PRV_UTF_LOG3_MAC("Device %d. Wrong Event Type. Expected %d, received %d\n",
                             dev, expectedErrorType, eventsPtr->eventsType[eventsIter]);
            return GT_FAIL;
        }

        for (tabIter=0; tabIter< eventsPtr->eventsInfo[eventsIter].numOfLogicalTables; tabIter++)
        {
            logicalPtr = &eventsPtr->eventsInfo[eventsIter].logicaTableInfo[tabIter];
            indexRangeMin = logicalPtr->logicalTableEntryIndex;
            indexRangeMax = logicalPtr->logicalTableEntryIndex + logicalPtr->numEntries - 1;


            switch (logicalPtr->logicalTableType)
            {
                case CPSS_DXCH_LOGICAL_TABLE_ARP_E:
                    /* there are 8 ARP entries per one HW table entry. Every HW
                     * entry consists of 6 (bobcat2, bobcat3) or 3 (bobk, aldrin) memories.
                     * So each memory contains parts of two/three ARP entries. */
                    expectedNumEntries = isDevBobkOrAldrin ? 3 : 2;
                    expectedEntryIndex = expectedArpIndex;
                    foundFlagPtr = &isArpIndexFound;
                    logicalTableNameStr = "ARP";
                    break;

                case CPSS_DXCH_LOGICAL_TABLE_TUNNEL_START_CONFIG_E:
                    expectedNumEntries = 1;
                    expectedEntryIndex = expectedTsNatIndex;
                    foundFlagPtr = &isTsIndexFound;
                    logicalTableNameStr = "Tunnel Start";
                    break;

                case CPSS_DXCH_LOGICAL_TABLE_NAT_E:
                    expectedNumEntries = 1;
                    expectedEntryIndex = expectedTsNatIndex;
                    foundFlagPtr = &isNatIndexFound;
                    logicalTableNameStr = "NAT";
                    break;

                default:
                    PRV_UTF_LOG2_MAC("Device %d. Bad Logical Table %d\n",
                                     dev, logicalPtr->logicalTableType);
                    return GT_FALSE;
                    break;
            }

            if (expectedNumEntries != logicalPtr->numEntries)
            {
                PRV_UTF_LOG4_MAC("Device %d. Wrong logical table \"%s\" numEntries. Expected %d, Received %d\n",
                                 dev, logicalTableNameStr, expectedNumEntries, logicalPtr->numEntries);
                return GT_FAIL;
            }

            if (expectedEntryIndex >= indexRangeMin && expectedEntryIndex <= indexRangeMax)
            {
                *foundFlagPtr = GT_TRUE;
            }
        }
    }

    if (isArpIndexFound == GT_FALSE)
    {
        PRV_UTF_LOG2_MAC("Device %d. ARP index %d is not found\n", dev, expectedArpIndex);
        return GT_FAIL;
    }
    if (isTsIndexFound == GT_FALSE)
    {
        PRV_UTF_LOG2_MAC("Device %d. TS index %d is not found\n", dev, expectedTsNatIndex);
        return GT_FAIL;
    }
    if (isNatIndexFound == GT_FALSE)
    {
        PRV_UTF_LOG2_MAC("Device %d. NAT index %d is not found\n", dev, expectedTsNatIndex);
        return GT_FAIL;
    }

    PRV_UTF_LOG0_MAC("========== Events checking passed ==========\n");
    return GT_OK;
}

UTF_TEST_CASE_MAC(cpssDxChDiagDataIntegrityLogicalTablesTsArpTest)
{
/*
    ITERATE_DEVICES (Bobcat2, Aldrin, AC3X, Bobcat3)

    1. Unmask event by Logical table to TS/ARP
    2. Enable injection by Logical table to TS/ARP
    3. Write TS entry indexes 0,1, max, max-1,  max/2, max/2+1 (to cover both even, odd entries numbers)  by TS API
    4. Read TS entry by TS API
    5. Check that event came and LOGICAL index is OK
    6. Make same for ARP entry

*/
    GT_STATUS           st = GT_OK;
    GT_U8               dev;
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            memLocation;
    CPSS_DXCH_LOGICAL_TABLE_ENT                     logicalTableName;
    DXCH_DATA_INTEGRITY_EVENT_CB_FUNC               *savedCallbackFn;
    CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_ENT  injectMode = CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E;
    CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorType;
    CPSS_DXCH_TUNNEL_START_CONFIG_UNT tunnelConfig;
    CPSS_DXCH_TUNNEL_START_CONFIG_UNT tunnelConfigGet; /* to store read value */
    CPSS_TUNNEL_TYPE_ENT              tunnelType = CPSS_TUNNEL_GENERIC_IPV4_E;
    CPSS_TUNNEL_TYPE_ENT              tunnelTypeGet;   /* to store read value */
    GT_ETHERADDR                      macAddr = {{0xAB, 0xCD, 0xEF, 0x00, 0x00, 0x01}};
    GT_ETHERADDR                      macAddrGet; /* to store read value */
    GT_U32 logTabIter;                            /* iterator through Logical Tables */
    GT_U32 entryIter;                             /* iterator through list of entries */
    GT_U32 entryMin;
    GT_U32 entryMiddle;
    GT_U32 entryMax;
    GT_U32 entryIndex;
    GT_U32 expectedArpEntryIndex;
    GT_U32 expectedTsNatEntryIndex;
    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E | UTF_XCAT3_E);


    cpssOsMemSet(&memLocation, 0, sizeof(memLocation));
    cpssOsMemSet(&tunnelConfig, 0, sizeof(tunnelConfig));
    cpssOsMemSet(&tunnelConfigGet, 0, sizeof(tunnelConfigGet));

    /* init Tunnel Start config struct. Arbitrary values taken from other UT */
    tunnelConfig.ipv4Cfg.tagEnable    = GT_TRUE;
    tunnelConfig.ipv4Cfg.vlanId       = 100;
    tunnelConfig.ipv4Cfg.upMarkMode   = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
    tunnelConfig.ipv4Cfg.up           = 0;
    tunnelConfig.ipv4Cfg.dscpMarkMode = CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
    tunnelConfig.ipv4Cfg.dscp         = 0;
    tunnelConfig.ipv4Cfg.cfi          = 0;
    tunnelConfig.ipv4Cfg.retainCRC    = GT_FALSE;

    tunnelConfig.ipv4Cfg.macDa.arEther[0] = 0x0;
    tunnelConfig.ipv4Cfg.macDa.arEther[1] = 0x1A;
    tunnelConfig.ipv4Cfg.macDa.arEther[2] = 0xFF;
    tunnelConfig.ipv4Cfg.macDa.arEther[3] = 0xFF;
    tunnelConfig.ipv4Cfg.macDa.arEther[4] = 0xFF;
    tunnelConfig.ipv4Cfg.macDa.arEther[5] = 0xFF;

    tunnelConfig.ipv4Cfg.dontFragmentFlag = GT_TRUE;
    tunnelConfig.ipv4Cfg.ttl              = 0;
    tunnelConfig.ipv4Cfg.autoTunnel       = GT_TRUE;
    tunnelConfig.ipv4Cfg.autoTunnelOffset = 1;

    tunnelConfig.ipv4Cfg.destIp.arIP[0] = 10;
    tunnelConfig.ipv4Cfg.destIp.arIP[1] = 15;
    tunnelConfig.ipv4Cfg.destIp.arIP[2] = 1;
    tunnelConfig.ipv4Cfg.destIp.arIP[3] = 250;

    tunnelConfig.ipv4Cfg.srcIp.arIP[0] = 10;
    tunnelConfig.ipv4Cfg.srcIp.arIP[1] = 15;
    tunnelConfig.ipv4Cfg.srcIp.arIP[2] = 1;
    tunnelConfig.ipv4Cfg.srcIp.arIP[3] = 255;

    tunnelConfig.ipv4Cfg.ipHeaderProtocol       = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E;
    tunnelConfig.ipv4Cfg.profileIndex           = 3;
    tunnelConfig.ipv4Cfg.greProtocolForEthernet = 0x0800;
    tunnelConfig.ipv4Cfg.greFlagsAndVersion     = 0xFFFF;
    tunnelConfig.ipv4Cfg.udpDstPort             = 0;
    tunnelConfig.ipv4Cfg.udpSrcPort             = 0;

    /* initialize memLocation fields common for both - tunnel start, arp - tables  */
    memLocation.type = CPSS_DXCH_LOCATION_LOGICAL_INDEX_INFO_TYPE;
    memLocation.info.logicalEntryInfo.numOfLogicalTables = 1;
    memLocation.info.logicalEntryInfo.logicaTableInfo[0].numEntries = 1;
    memLocation.info.logicalEntryInfo.logicaTableInfo[0].logicalTableType = CPSS_DXCH_LOGICAL_TABLE_TUNNEL_START_CONFIG_E;


    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        errorType = PRV_CPSS_SIP_5_20_CHECK_MAC(dev) ?
            CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E :
            CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;

        /* two iterations. 1st: write ARP entry, 2nd: write Tunnel-Start IPv4 entry */
        for (logTabIter=0; logTabIter < 2; logTabIter++)
        {
            PRV_UTF_LOG0_MAC("\n");
            if (logTabIter == 0)
            {
                logicalTableName = CPSS_DXCH_LOGICAL_TABLE_TUNNEL_START_CONFIG_E;
                entryMin    = 0;
                entryMax    = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.tunnelStart - 1;
                entryMiddle = entryMax / 2;
            }
            else
            {
                logicalTableName = CPSS_DXCH_LOGICAL_TABLE_ARP_E;
                entryMin    = 0;
                entryMax    = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.routerArp - 1;
                entryMiddle = entryMax / 2;
            }

            /* initialize memLocation fields specific for table type */
            memLocation.info.logicalEntryInfo.logicaTableInfo[0].logicalTableType = logicalTableName;

            /* 1. unmask interrupts for logical table */
            st = cpssDxChDiagDataIntegrityEventMaskSet(dev, &memLocation, errorType,
                                                       CPSS_EVENT_UNMASK_E);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


            /* iterate through  entries to be written: minimal, minimal+1, middle, middle+1, maximal-1, maximal */
            for (entryIter = 0; entryIter < 6; entryIter++)
            {
                /* entryIter = 0, 2, 4 appropriate to entryMin, entryMiddle, entryMax-1 */
                entryIndex =
                    entryIter/2 == 0 ? entryMin :
                    entryIter/2 == 1 ? entryMiddle : entryMax - 1;
                /* entryIter = 1, 3, 5 appropriate to entryMin+1, entryMiddle+1, entryMax */
                if (entryIter % 2)
                {
                    entryIndex++;
                }

                memLocation.info.logicalEntryInfo.logicaTableInfo[0].logicalTableEntryIndex = entryIndex;

                PRV_UTF_LOG2_MAC("Error Injection into \"%s\" logical table, entryIndex=%d\n",
                                 (logicalTableName == CPSS_DXCH_LOGICAL_TABLE_TUNNEL_START_CONFIG_E? "Tunnel Start" :
                                  logicalTableName == CPSS_DXCH_LOGICAL_TABLE_ARP_E? "ARP" : "UNKNOWN"),
                                 entryIndex);

                /* 2. enable error injection  */
                st = cpssDxChDiagDataIntegrityErrorInjectionConfigSet(
                                        dev, &memLocation, injectMode, GT_TRUE);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, entryIndex);

                /* activate our events handler */
                savedCallbackFn = dxChDataIntegrityEventIncrementFunc;

                tunnelStartEvents.tsEventsCount = 0;
                tunnelStartEvents.allEventsCount = 0;
                dxChDataIntegrityEventIncrementFunc = prvTunnelStartEventsHandler;


                /* 3. write an entry
                 * 4. read the entry */
                if (logicalTableName == CPSS_DXCH_LOGICAL_TABLE_TUNNEL_START_CONFIG_E)
                {
                    /* init expected entry numbers  */
                    expectedArpEntryIndex  = entryIndex * 4;
                    expectedTsNatEntryIndex = entryIndex;

                    /* write TunnelStart entry */
                    PRV_UTF_LOG1_MAC("Write Tunnel Start entry #%d\n", entryIndex);
                    st = cpssDxChTunnelStartEntrySet(dev, entryIndex, tunnelType,
                                                     &tunnelConfig);
                    UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, entryIndex);

                    /* read TunnelStart entry */
                    PRV_UTF_LOG1_MAC("Read  Tunnel Start entry #%d\n", entryIndex);
                    st = cpssDxChTunnelStartEntryGet(dev, entryIndex,
                                                     &tunnelTypeGet, &tunnelConfigGet);
                    /* don't check return code. The injected error can cause it to be != GT_OK.
                     * Internall call to tunnelStartConfigHw2LogicFormat_sip5 can fail
                     * on converting HW binary data(erroneous) to struct.*/
                }
                else
                {
                    /* ARP table  */

                    /* init expected entry numbers  */
                    expectedArpEntryIndex  = entryIndex;
                    expectedTsNatEntryIndex = entryIndex / 4;

                    /* write ARP entry */
                    PRV_UTF_LOG1_MAC("Write ARP entry #%d\n", entryIndex);
                    st = cpssDxChIpRouterArpAddrWrite(dev, entryIndex, &macAddr);

                    UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, entryIndex);

                    /* read ARP entry */
                    PRV_UTF_LOG1_MAC("Read  ARP entry #%d\n", entryIndex);
                    st = cpssDxChIpRouterArpAddrRead(dev, entryIndex, &macAddrGet);
                    UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, entryIndex);
                }

                /* give the time for interrupt to occur */
                cpssOsTimerWkAfter(50);

                /* restore original callback */
                dxChDataIntegrityEventIncrementFunc = savedCallbackFn;

                /* 5. Check the events */
                st = prvCpssDxChDiagDataIntegrityCheckTsEvents(dev, &tunnelStartEvents,
                                                               errorType,
                                                               expectedArpEntryIndex,
                                                               expectedTsNatEntryIndex);

                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            }

            /* unmask interrupts for logical table */
            st = cpssDxChDiagDataIntegrityEventMaskSet(dev, &memLocation, errorType,
                                                       CPSS_EVENT_MASK_E);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, entryIndex);
        }
    }
}

/* The UT needs to ensure that shadow DB is not corrupted by weak bit occured
   in HW table entry. The test need to check next scenario:
   1) weak bit occured in HW table.
   2) application calls prvCpssDxChWriteTableEntryField. This API reads entry
      from HW (with "weak" bit), updates required bits and writes the result
      info the HW that cause writing the same entry into shadow DB ->
      shadow DB is corrupted.
      Even if the HW table is protected by ECC/Parity the interrupt is
      handled in different thread and doesn't prevent corruption of shadow DB.
 */
UTF_TEST_CASE_MAC(cpssDxChDiagDataIntegrityShadowDbWeakHwBit)
{
/*
  Iterate device (Bobcat2, Aldrin, AC3X, Bobcat3) ports:
      1) get an entry from CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_E HW table.
      2) enable error injection for the table.
      3) write the stored entry into the table to trigger error injection.
         After this stage the HW table contains weak bit and shadow DB - not.
      4) read the entry from HW and compare it with stored value to understand
         which bit is erroneous.
      5) Choose API to be sure that we don't rewrite weak bit. One of
           - cpssDxChBrgFdbNaStormPreventSet or
           - cpssDxChBrgFdbPortVid1LearningEnableSet
         This cause reading from HW (with weak bit), setting some bits,
         writing back to HW (updates the shadow DB).
      6) ensure that shadow DB entry still doesn't contain weak bit.
*/

    GT_U8                                dev;
    GT_PORT_NUM                          port;
    GT_STATUS                            st;
    GT_U32                               hwEntry[MAX_ENTRY_SIZE_CNS];
    GT_U32                               errHwEntry[MAX_ENTRY_SIZE_CNS];
    GT_U32                               shadowEntry[MAX_ENTRY_SIZE_CNS];
    GT_U32                               weakIx; /* entry's word index where weak bit is */
    GT_U32                               weakMask;  /* mask to extract weak bits */
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC location;
    CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_ENT injectMode = CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_MULTIPLE_E;
    CPSS_DXCH_TABLE_ENT tableType = CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_E;


    cpssOsMemSet(&location, 0, sizeof(location));
    location.portGroupsBmp                = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    location.type                         = CPSS_DXCH_LOCATION_HW_INDEX_INFO_TYPE;
    location.info.hwEntryInfo.hwTableType = tableType;

    /* AUTODOC: prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E | UTF_XCAT3_E);
    /* AUTODOC:  go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /* AUTODOC: prepare iterator for go over ePorts */
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* AUTODOC:  go over all ePorts. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            cpssOsMemSet(&hwEntry,     0, sizeof(hwEntry));
            cpssOsMemSet(&errHwEntry,  0, sizeof(errHwEntry));
            cpssOsMemSet(&shadowEntry, 0, sizeof(shadowEntry));

            /* AUTODOC: 1. get the HW table entry */
            st = prvCpssDxChReadTableEntry(dev, tableType, port, hwEntry);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* AUTODOC: 2. enable error injection */
            st = cpssDxChDiagDataIntegrityErrorInjectionConfigSet(dev, &location, injectMode, GT_TRUE);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* AUTODOC: 3. write the stored entry into the table to trigger error injection */
            st = prvCpssDxChWriteTableEntry(dev, tableType, port, hwEntry);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* AUTODOC: disable error injection */
            st = cpssDxChDiagDataIntegrityErrorInjectionConfigSet(dev, &location, injectMode, GT_FALSE);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* AUTODOC: 4. get the HW table entry (with weak bit) */
            st = prvCpssDxChReadTableEntry(dev, tableType, port, errHwEntry);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* AUTODOC: determine a weak bit's mask and word number */
            weakMask = 0;
            for (weakIx = 0; weakIx < MAX_ENTRY_SIZE_CNS; weakIx++)
            {
                weakMask = (hwEntry[weakIx] ^ errHwEntry[weakIx]);
                if (weakMask)
                {
                    break;
                }
            }

            /* DFX engine can choose ECC code bits instead of table entry bits
               to make them erroneous. Such injection is not detected by entry reading.
               The problem can't be solved by executing error injection several times
               (it seems errorneous bit number is not random). Just try to choose
               another table (manually) */
            UTF_VERIFY_NOT_EQUAL1_STRING_MAC(0, weakMask,
                "An erroneous bit is not found after error injection, port %d", port);

            /* AUTODOC: 5. Choose (and call) an API which doesn't rewrite weak bit */

            /* convertFieldInfoToGlobalBit field is a bit #2 in Ingress Bridge ePort Table  */
            if ((hwEntry[0] ^ errHwEntry[0] ) != (GT_U32)(1 << 2))
            {
                st = cpssDxChBrgFdbNaStormPreventSet(dev, port, GT_TRUE);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            }
            else
            {
                st = cpssDxChBrgFdbPortVid1LearningEnableSet(dev, port, GT_TRUE);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            }

            /* AUTODOC: 6. ensure that shadow DB entry doesn't contain weak bit. */

            /* read shadow DB entry */
            st = prvCpssDxChPortGroupReadTableEntry_fromShadow(
                                          dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                          tableType,
                                          port, /* entry index */
                                          shadowEntry);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* check that shadow DB doesn't contain weak bit */
            UTF_VERIFY_EQUAL0_STRING_MAC(
                (hwEntry[weakIx]     & weakMask),
                (shadowEntry[weakIx] & weakMask),
                "Shadow DB contains weak bit" );
        }
    }
}

static GT_STATUS prvCpssDxChDiagDataIntegrityEventLogicalPolicerCheck
(
    GT_U32                                           devNum,
    IN CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT expEventType,
    IN CPSS_DXCH_LOGICAL_TABLE_ENT                   expTableType,
    IN GT_U32                                        expEntryIndex,
    IN GT_U32                                        expNumEntries
)
{
    GT_U32                                        tablesNum;
    CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT curEventType;
    CPSS_DXCH_LOGICAL_TABLE_ENT                   curTableType;
    GT_U32                                        curEntryIndex;
    GT_U32                                        curNumEntries;
    CPSS_DXCH_LOGICAL_TABLE_INFO_STC              *tableInfoPtr;
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT    expMemTypesArr[2] = {CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_LAST_E,
                                                                       CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_LAST_E};
    CPSS_DXCH_TABLE_ENT                           expHwTablesArr[2];
    GT_U32                                        expHwEntryIndex = expEntryIndex;
    GT_U32                                        expTablesCount;
    PRV_DXCH_DATA_INTEGRITY_EVENT_INFO_STC        *eventPtr;
    GT_U32                                        i;
    GT_U32                                        evIx;
    GT_STATUS                                     rc = GT_OK;

    DXCH_DATA_INTEGRITY_EVENTS_DB_LOCK();

    for (evIx = 0; evIx < occuredEvents.eventsCount; evIx++)
    {
        eventPtr = &occuredEvents.eventsArr[evIx];
        tablesNum = eventPtr->eventLogicalEntryInfo.numOfLogicalTables;

        if (tablesNum != 1)
        {
            utfFailureMsgLog("FAILED - eventLogicalEntryInfo.numOfLogicalTables is not equal to 1 \n", NULL, 0);
            rc = GT_FAIL;
            break;
        }
        else
        {
            curEventType = eventPtr->eventsType;
            curEntryIndex = eventPtr->eventHwEntryInfo.hwTableEntryIndex;

            tableInfoPtr  = &eventPtr->eventLogicalEntryInfo.logicaTableInfo[0];
            curTableType  = tableInfoPtr->logicalTableType;
            curEntryIndex = tableInfoPtr->logicalTableEntryIndex;
            curNumEntries = tableInfoPtr->numEntries;


            if ((curEventType != expEventType)   ||
                (curTableType != expTableType)   ||
                (curEntryIndex != expEntryIndex) ||
                (curNumEntries != expNumEntries))
            {
                rc = GT_FAIL;
                break;
            }
        }
    }
    if (rc != GT_OK)
    {
        /* AUTODOC: print expected values info*/
        cpssOsPrintf("=========== expected event logical tables info ================\n");
        cpssOsPrintf("    expected eventType     : %d\n", expEventType);
        PRV_CPSS_DATA_INTEGRITY_ENUM_STRING_GET_MAC(
            expTableType, CPSS_DXCH_LOGICAL_TABLE_ENT, strNameBuffer);
        cpssOsPrintf("    expected logical table : %s\n", strNameBuffer);
        cpssOsPrintf("    expected logical index : %d\n", expEntryIndex);
        cpssOsPrintf("    expected numEntries    : %d\n", expNumEntries);

        /* AUTODOC: print an occured event's logical table info */
        cpssOsPrintf("=========== occured event logical tables info =================\n");
        cpssOsPrintf("eventLogicalEntryInfo numOfLogicalTables %d\n", tablesNum);
        cpssOsPrintf("number of occured events       : %d\n", occuredEvents.eventsCount);
        for (i=0; i< tablesNum; i++)
        {
            tableInfoPtr  = &eventPtr->eventLogicalEntryInfo.logicaTableInfo[i];
            cpssOsPrintf("event #%d, table #%d       :\n", evIx, i);
            cpssOsPrintf("    occured eventType      : %d\n", expEventType);
            PRV_CPSS_DATA_INTEGRITY_ENUM_STRING_GET_MAC(tableInfoPtr->logicalTableType,
                                                        CPSS_DXCH_LOGICAL_TABLE_ENT,
                                                        strNameBuffer);
            cpssOsPrintf("    occured logical table  : %s\n", strNameBuffer);
            cpssOsPrintf("    occured logical index  : %d\n", tableInfoPtr->logicalTableEntryIndex);
            cpssOsPrintf("    occured numEntries     : %d\n", tableInfoPtr->numEntries);
        }
        DXCH_DATA_INTEGRITY_EVENTS_DB_UNLOCK();
        return rc;
    }

    /* AUTODOC: get HW table/entry and memType for specified logical table */
    expTablesCount = 0;
    switch (expTableType)
    {
        /* don't care with filling expMemTypesArr for Metering/Counting tables.
           It will be assigned with prvDxChDiagDataIntegrityMemTypeUpdate */
        case CPSS_DXCH_LOGICAL_TABLE_INGRESS_POLICER_0_METERING_E:
            if (PRV_CPSS_DXCH_BOBCAT2_CHECK_MAC(devNum))
            {
                expHwTablesArr[0] = CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_METERING_E;
                expTablesCount = 1;
            }
            else
            {
                if (expEntryIndex < IPLR0_SIZE_MAC(devNum))
                {
                    expHwTablesArr[0] = CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_METERING_E;

                    expHwTablesArr[1] = CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_0_METERING_CONFIG_E;
                    expMemTypesArr[1] = PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)?
                        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_METERING_CONF_TBL_E :
                        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_METERING_CONF_INGRESS_E;
                    expTablesCount = 2;
                }
            }
            break;

        case CPSS_DXCH_LOGICAL_TABLE_INGRESS_POLICER_1_METERING_E:
            if (PRV_CPSS_DXCH_BOBCAT2_CHECK_MAC(devNum))
            {
                expHwTablesArr[0] = CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_METERING_E;
                expTablesCount = 1;
            }
            else
            {
                if (expEntryIndex < IPLR1_SIZE_MAC(devNum))
                {
                    expHwTablesArr[0] = CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_METERING_E;

                    expHwTablesArr[1] = CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_0_METERING_CONFIG_E;
                    expMemTypesArr[1] = PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)?
                        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_METERING_CONF_TBL_E :
                        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_METERING_CONF_INGRESS_E;
                    expTablesCount = 2;
                    expHwEntryIndex += IPLR0_SIZE_MAC(devNum);
                }
            }
            break;

        case CPSS_DXCH_LOGICAL_TABLE_INGRESS_POLICER_0_COUNTING_E:
            expHwTablesArr[0] = CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_COUNTING_E;
            expTablesCount = 1;
            break;

        case CPSS_DXCH_LOGICAL_TABLE_INGRESS_POLICER_1_COUNTING_E:
            if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                expHwTablesArr[0] = CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_COUNTING_E;
                expHwEntryIndex += IPLR0_SIZE_MAC(devNum);
            }
            else
            {
                expHwTablesArr[0] = CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_COUNTING_E;
            }
            expTablesCount = 1;

            break;

        case CPSS_DXCH_LOGICAL_TABLE_EGRESS_POLICER_METERING_E:
            if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                if (expEntryIndex < EPLR_SIZE_MAC(devNum))
                {
                    expHwTablesArr[0] = CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_METERING_E;
                    expMemTypesArr[0] = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_METERING_TB_TBL_E;
                    expHwEntryIndex += IPLR0_SIZE_MAC(devNum) + IPLR1_SIZE_MAC(devNum);
                    expTablesCount = 1;
                }
            }
            else
            {
                expHwTablesArr[0] = CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_METERING_E;
                expMemTypesArr[0] = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EPLR_METERING_EGRESS_E;
                expTablesCount = 1;
            }
            break;

        case CPSS_DXCH_LOGICAL_TABLE_EGRESS_POLICER_COUNTING_E:
            if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                expHwTablesArr[0] = CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_COUNTING_E;
                expMemTypesArr[0] = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_COUNTING_TBL_E;
                expHwEntryIndex += IPLR0_SIZE_MAC(devNum) + IPLR1_SIZE_MAC(devNum);
            }
            else
            {
                expHwTablesArr[0] = CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_COUNTING_E;
                expMemTypesArr[0] = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EPLR_BILLING_EGRESS_E;

            }
            expTablesCount = 1;
            break;
        default:
            UTF_VERIFY_EQUAL1_STRING_NO_RETURN_MAC(
                GT_OK, rc, "unknown logical table", expTableType);
            break;
    }

    for (i=0; i < expTablesCount; i++)
    {
        rc = prvDxChDiagDataIntegrityMemTypeUpdate(devNum,
                                                   expHwTablesArr[i],
                                                   expHwEntryIndex,
                                                   &expMemTypesArr[i]);
        UTF_VERIFY_EQUAL3_NO_RETURN_MAC(
            GT_OK, rc, devNum, expHwTablesArr[i], expHwEntryIndex);

        rc = prvCpssDxChDiagDataIntegrityEventHwInfoDbCheck(
            expEventType, expMemTypesArr[i], expHwTablesArr[i], expHwEntryIndex,
            PRV_DATA_INTEGRITY_EVENT_MATCH_TYPE_ANY_E);
        UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, rc);
    }

    DXCH_DATA_INTEGRITY_EVENTS_DB_UNLOCK();
    return rc;
}


/*
  Test is similar to cpssDxChDiagDataIntegrityHwTablesInfoGetTest but for LOGICAL tables
      CPSS_DXCH_LOGICAL_TABLE_INGRESS_POLICER_0_METERING_E,
      CPSS_DXCH_LOGICAL_TABLE_INGRESS_POLICER_1_METERING_E,
      CPSS_DXCH_LOGICAL_TABLE_EGRESS_POLICER_METERING_E,
      CPSS_DXCH_LOGICAL_TABLE_INGRESS_POLICER_0_COUNTING_E,
      CPSS_DXCH_LOGICAL_TABLE_INGRESS_POLICER_1_COUNTING_E,
      CPSS_DXCH_LOGICAL_TABLE_EGRESS_POLICER_COUNTING_E,
  1. Error injection and interrupt management by LOGICAL
  2. Write entry by using API cpssDxCh3PolicerMeteringEntrySet and cpssDxCh3PolicerBillingEntrySet
  3. Scan by cpssDxChDiagDataIntegrityTableScan with using LOGICAL type
  4. Event check should compare LOGICAL table type and index.
 */
UTF_TEST_CASE_MAC(cpssDxChDiagDataIntegrityLogicalTablesPolicerTest)
{
/*
    ITERATE_DEVICES (Bobcat2, Aldrin, AC3X, bobcat3)
*/
    GT_STATUS           st = GT_OK;
    GT_U8               devNum;
    CPSS_DXCH_LOGICAL_TABLE_ENT                     logicalTable;
    CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errType;
    CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_ENT  injMode = CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E;
    GT_U32                                          entryIndex;
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            locationInfo;
    GT_U32                                          nextEntryIndex;
    GT_BOOL                                         wasWrapAround;
    GT_U32                                          maxEntries = 0;
    GT_U32                                          *tableEntryPtr = &tableEntry[0];
    DXCH_DATA_INTEGRITY_EVENT_CB_FUNC               *savedDataIntegrityHandler;
    CPSS_DXCH3_POLICER_BILLING_ENTRY_STC            billingEntry;
    CPSS_DXCH3_POLICER_METERING_ENTRY_STC           meteringEntry;
    CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT          tbParams; /* value doesn't matter */
    GT_BOOL                                         countEnableGet;
    GT_U32                                          injectIx;
    GT_U32                                          tabIx;
    struct {
        CPSS_DXCH_LOGICAL_TABLE_ENT tableName;
        GT_BOOL isMetering;     /* if GT_FALSE - this is one of counting table */
        CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage;
    } logicalEntriesArr[] = {
        {CPSS_DXCH_LOGICAL_TABLE_INGRESS_POLICER_0_METERING_E , GT_TRUE  , CPSS_DXCH_POLICER_STAGE_INGRESS_0_E},
        {CPSS_DXCH_LOGICAL_TABLE_INGRESS_POLICER_1_METERING_E , GT_TRUE  , CPSS_DXCH_POLICER_STAGE_INGRESS_1_E},
        {CPSS_DXCH_LOGICAL_TABLE_EGRESS_POLICER_METERING_E    , GT_TRUE  , CPSS_DXCH_POLICER_STAGE_EGRESS_E},
        {CPSS_DXCH_LOGICAL_TABLE_INGRESS_POLICER_0_COUNTING_E , GT_FALSE , CPSS_DXCH_POLICER_STAGE_INGRESS_0_E},
        {CPSS_DXCH_LOGICAL_TABLE_INGRESS_POLICER_1_COUNTING_E , GT_FALSE , CPSS_DXCH_POLICER_STAGE_INGRESS_1_E},
        {CPSS_DXCH_LOGICAL_TABLE_EGRESS_POLICER_COUNTING_E    , GT_FALSE , CPSS_DXCH_POLICER_STAGE_EGRESS_E}};
    GT_U32 logicTableArrLen = sizeof(logicalEntriesArr) / sizeof (logicalEntriesArr[0]);



    /* AUTODOC: prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E | UTF_XCAT3_E |
                                           UTF_FALCON_E);

    cpssOsBzero((GT_CHAR*)&billingEntry,  sizeof(billingEntry));
    cpssOsBzero((GT_CHAR*)&meteringEntry, sizeof(meteringEntry));

    cpssOsMemSet(tableEntryPtr, 0xFF, sizeof(tableEntry));

    /* AUTODOC: save data integrity error callback. */
    savedDataIntegrityHandler = dxChDataIntegrityEventIncrementFunc;

    /* AUTODOC: set own callback */
    dxChDataIntegrityEventIncrementFunc = hwInfoEventErrorHandlerFunc;

    /* AUTODOC: init a Policer Billing entry fields with values applicable for writting */
    billingEntry.packetSizeMode  = CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E;
    billingEntry.greenCntr.l[0]  = 0;
    billingEntry.yellowCntr.l[0] = 0;
    billingEntry.redCntr.l[0]    = 0;

    /* AUTODOC: init a Policer Metering entry fields with values applicable for writting */
    meteringEntry.countingEntryIndex                = 0;
    meteringEntry.mngCounterSet                     = CPSS_DXCH3_POLICER_MNG_CNTR_SET0_E;
    meteringEntry.meterColorMode                    = CPSS_POLICER_COLOR_BLIND_E;
    meteringEntry.meterMode                         = CPSS_DXCH3_POLICER_METER_MODE_SR_TCM_E;
    meteringEntry.tokenBucketParams.srTcmParams.cir = 0;
    meteringEntry.tokenBucketParams.srTcmParams.cbs = 0;
    meteringEntry.tokenBucketParams.srTcmParams.ebs = 0;
    meteringEntry.modifyUp                          = CPSS_DXCH_POLICER_MODIFY_DSCP_DISABLE_E;
    meteringEntry.modifyDscp                        = CPSS_DXCH_POLICER_MODIFY_DSCP_DISABLE_E;
    meteringEntry.modifyDp                          = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    meteringEntry.modifyExp                         = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    meteringEntry.modifyTc                          = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    meteringEntry.yellowPcktCmd                     = CPSS_DXCH3_POLICER_NON_CONFORM_CMD_NO_CHANGE_E;
    meteringEntry.redPcktCmd                        = CPSS_DXCH3_POLICER_NON_CONFORM_CMD_NO_CHANGE_E;
    meteringEntry.packetSizeMode                    = CPSS_POLICER_PACKET_SIZE_L3_ONLY_E;

    /* AUTODOC:  go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {

        if (PRV_CPSS_DXCH_CETUS_CHECK_MAC(devNum) ||
            PRV_CPSS_DXCH_CAELUM_CHECK_MAC(devNum))
        {
            st = prvUtfDataIntegrityEplrZeroTablesAndResetInt(devNum);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        }

        /* AUTODOC: Clear captured events */
        prvCpssDxChDiagDataIntegrityEventsClear();

        /* AUTODOC: loop over all logical tables and inject errors */
        for(tabIx = 0; tabIx < logicTableArrLen; tabIx++)
        {
            logicalTable = logicalEntriesArr[tabIx].tableName;
            switch (logicalEntriesArr[tabIx].stage)
            {
                case CPSS_DXCH_POLICER_STAGE_INGRESS_0_E:
                    maxEntries = IPLR0_SIZE_MAC(devNum);
                    break;
                case CPSS_DXCH_POLICER_STAGE_INGRESS_1_E:
                    maxEntries = IPLR1_SIZE_MAC(devNum);
                    break;
                default:
                    /* CPSS_DXCH_POLICER_STAGE_EGRESS_E */
                    maxEntries = EPLR_SIZE_MAC(devNum);
                    break;
            }

            cpssOsBzero((GT_CHAR*)&locationInfo, sizeof(locationInfo));
            locationInfo.type = CPSS_DXCH_LOCATION_LOGICAL_INDEX_INFO_TYPE;
            locationInfo.portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
            locationInfo.info.logicalEntryInfo.numOfLogicalTables = 1;
            locationInfo.info.logicalEntryInfo.logicaTableInfo[0].numEntries = 1;
            locationInfo.info.logicalEntryInfo.logicaTableInfo[0].logicalTableType = logicalTable;

            for (injectIx = 0; injectIx < PRV_UTF_ERROR_INJECT_LOOPS_PER_MEM; injectIx++)
            {
                /* AUTODOC: calculate entry index and write table entry field */
                entryIndex = (PRV_UTF_ERROR_INJECT_LOOPS_PER_MEM - injectIx - 1) * (maxEntries - 1) / (PRV_UTF_ERROR_INJECT_LOOPS_PER_MEM - 1);

                locationInfo.info.logicalEntryInfo.logicaTableInfo[0].logicalTableEntryIndex = entryIndex;

                PRV_UTF_LOG3_MAC("******** tabIx = %d, injectIx == %d, entryIndex = %d ********\n",
                                 tabIx, injectIx, entryIndex);

                /* AUTODOC: get error Type for specified logical table */
                if (traceEvents)
                {
                    PRV_UTF_LOG0_MAC("cpssDxChDiagDataIntegrityErrorCountEnableGet\n");
                }
                st = cpssDxChDiagDataIntegrityErrorCountEnableGet(devNum, &locationInfo,
                                                                  &errType, &countEnableGet);
                if (errType == CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E)
                {
                    errType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
                }
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, logicalTable, injectIx, entryIndex);
                /* AUTODOC: enable error counter */
                if (traceEvents)
                {
                    PRV_UTF_LOG0_MAC("cpssDxChDiagDataIntegrityErrorCountEnableSet\n");
                }
                st = cpssDxChDiagDataIntegrityErrorCountEnableSet(devNum, &locationInfo,
                                                                  errType, GT_TRUE);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, logicalTable, injectIx, entryIndex);

                /* AUTODOC: table found in DFX data base - unmask interrupt for DFX memory */
                if (traceEvents)
                {
                    PRV_UTF_LOG0_MAC("cpssDxChDiagDataIntegrityEventMaskSet\n");
                }
                st = cpssDxChDiagDataIntegrityEventMaskSet(devNum, &locationInfo,
                                                           errType,
                                                           CPSS_EVENT_UNMASK_E);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, logicalTable, injectIx, entryIndex);

                /* AUTODOC: enable error injection */
                if (traceEvents)
                {
                    PRV_UTF_LOG0_MAC("cpssDxChDiagDataIntegrityErrorInjectionConfigSet\n");
                }
                st = cpssDxChDiagDataIntegrityErrorInjectionConfigSet(
                    devNum, &locationInfo, injMode, GT_TRUE);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, logicalTable, injectIx, entryIndex);

                /* AUTODOC: write table entries using CPSS APIs */
                if (GT_TRUE == logicalEntriesArr[tabIx].isMetering)
                {
                    if (traceEvents)
                {
                    PRV_UTF_LOG0_MAC("cpssDxCh3PolicerMeteringEntrySet\n");
                }
                    st = cpssDxCh3PolicerMeteringEntrySet(devNum,
                                                          logicalEntriesArr[tabIx].stage,
                                                          entryIndex,
                                                          &meteringEntry,
                                                          &tbParams);
                }
                else
                {
                    if (traceEvents)
                    {
                        PRV_UTF_LOG0_MAC("cpssDxCh3PolicerBillingEntrySet\n");
                    }
                    st = cpssDxCh3PolicerBillingEntrySet(devNum,
                                                         logicalEntriesArr[tabIx].stage,
                                                         entryIndex,
                                                         &billingEntry);
                }
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, logicalTable, entryIndex);

                /* AUTODOC: sleep 10 mSec */
                cpssOsTimerWkAfter(timeOut);

                /* AUTODOC: scan single logical table entry */
                if (traceEvents)
                {
                    PRV_UTF_LOG0_MAC("cpssDxChDiagDataIntegrityTableScan\n");
                }
                st = cpssDxChDiagDataIntegrityTableScan(devNum, &locationInfo, 1 /* ignored for logical tables*/,
                                                        &nextEntryIndex, &wasWrapAround);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);


                /* AUTODOC: set pause to allow event to happen */
                cpssOsTimerWkAfter(timeOut);

                /* AUTODOC: check the Data Integrity event is occured in the expected logical table.
                            Clear event info after comparison. */
                st = prvCpssDxChDiagDataIntegrityEventLogicalPolicerCheck(
                    devNum, errType, logicalTable, entryIndex, 1);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                /* AUTODOC: Clear captured events */
                prvCpssDxChDiagDataIntegrityEventsClear();


                /* AUTODOC: disable error injection */
                if (traceEvents)
                {
                    PRV_UTF_LOG0_MAC("cpssDxChDiagDataIntegrityErrorInjectionConfigSet\n");
                }
                st = cpssDxChDiagDataIntegrityErrorInjectionConfigSet(
                    devNum, &locationInfo, injMode, GT_FALSE);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                /* AUTODOC: Mask DFX memory interrupt */
                st = cpssDxChDiagDataIntegrityEventMaskSet(devNum, &locationInfo, errType, CPSS_EVENT_MASK_E);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            }
        }
    }

    /* AUTODOC: Restore dxChDataIntegrityEventIncrementFunc */
    dxChDataIntegrityEventIncrementFunc = savedDataIntegrityHandler;
}

#endif /*ASIC_SIMULATION */

/* transform portGroupId to value that can be used as array index.
 * 'Raw' portGroupId is inconvenient because it can be equal
 * CPSS_PORT_GROUP_UNAWARE_MODE_CNS = 0xFFFFFFFF - too huge be an index */
#define PRV_PORT_GROUP_TO_INDEX_MAC(portGroupId)    \
    ((portGroupId) == CPSS_PORT_GROUP_UNAWARE_MODE_CNS ? 0 : (portGroupId) + 1)

/* macro reverse to PRV_PORT_GROUP_TO_INDEX_MAC */
#define PRV_INDEX_TO_PORT_GROUP_MAC(index)                              \
    ((index) == 0) ? CPSS_PORT_GROUP_UNAWARE_MODE_CNS : (index-1)

/* maximum number of tables a single memType can be met in */
#define TABLES_PER_RAM_MAX_CNS  200
#define PORT_GROUPS_NUM_CNS 3
typedef struct {
    GT_U32 instanceAmount;
    GT_U32 hwTableName[TABLES_PER_RAM_MAX_CNS];
} PRV_TGF_RAM_INFO;

typedef struct {
    PRV_TGF_RAM_INFO   ramInfo[CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_LAST_E][PORT_GROUPS_NUM_CNS];
}PRV_TGF_DFX_DB_TEST_STC;

#define PRV_DONT_PRINT_NAME 0xFFFFFFFF

static void prvTgfLogTablesNames(GT_U32 hwTableType, GT_U32 memType)
{
    if (PRV_DONT_PRINT_NAME != hwTableType)
    {
        PRV_CPSS_DATA_INTEGRITY_ENUM_STRING_GET_MAC(hwTableType,
                                                    CPSS_DXCH_TABLE_ENT, strNameBuffer);
        PRV_UTF_LOG1_MAC("HW table type     :  %s\n",   strNameBuffer);
    }

    if (PRV_DONT_PRINT_NAME != memType)
    {
        PRV_CPSS_DATA_INTEGRITY_ENUM_STRING_GET_MAC(memType,
                                                    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT, strNameBuffer1);
        PRV_UTF_LOG1_MAC("Memory table type :  %s\n",   strNameBuffer1);
    }
}

const static GT_U32 prvExpect_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_CNC_COUNTERS_E[] = {
CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E    ,
CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 1,
CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 2,
CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 3,
CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 4,
CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 5,
CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 6,
CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 7,
CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 8,
CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 9,
CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 10,
CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 11,
CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 12,
CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 13,
CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 14,
CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 15,
CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 16,
CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 17,
CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 18,
CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 19,
CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 20,
CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 21,
CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 22,
CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 23,
CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 24,
CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 25,
CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 26,
CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 27,
CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 28,
CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 29,
CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 30,
CPSS_DXCH_SIP5_TABLE_CNC_31_COUNTERS_E,
CPSS_DXCH_TABLE_LAST_E          /* bobcat 3 only */
};

const static GT_U32 prvExpect_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PCL_CONFIG_E[] = {
    CPSS_DXCH_TABLE_PCL_CONFIG_E,
    CPSS_DXCH_XCAT_TABLE_INGRESS_PCL_LOOKUP1_CONFIG_E,
    CPSS_DXCH_XCAT3_TABLE_LAST_E};

const static GT_U32 prvExpect_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_IPFIX_AGING_E[] = {
    CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_IPFIX_ALERT_AGING_E,
    CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_IPFIX_ALERT_AGING_E};

const static GT_U32 prvExpect_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_IPFIX_WRAPAROUND_E[] = {
    CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_IPFIX_ALERT_WRAPAROUND_E,
    CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_IPFIX_ALERT_WRAPAROUND_E};

const static GT_U32 prvExpect_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_QOS_REMARKING_TABLE_E[] = {
    CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_REMARKING_E,
    CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_REMARKING_E};

const static GT_U32 prvExpect_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_OAM_OAM_TABLE_E[] = {
    CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_E,
    CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_E};

const static GT_U32 prvExpect_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_OAM_RAM_SOURCE_INTERFACE_EXCEPTION_E[] = {
    CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_SOURCE_INTERFACE_EXCEPTION_E,
    CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_SOURCE_INTERFACE_EXCEPTION_E};

const static GT_U32 prvExpect_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_OAM_RAM_EXCESS_KEEPALIVE_EXCEPTION_E[] = {
    CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_KEEPALIVE_EXCESS_E,
    CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_KEEPALIVE_EXCESS_E};

const static GT_U32 prvExpect_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_OAM_RAM_SUMMARY_EXCEPTION_E[] = {
    CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_EXCEPTION_SUMMARY_E,
    CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_EXCEPTION_SUMMARY_E};

const static GT_U32 prvExpect_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_OAM_RAM_RDI_STATUS_CHANGE_EXCEPTION_E[] = {
    CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_RDI_STATUS_CHANGE_EXCEPTION_E,
    CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_RDI_STATUS_CHANGE_EXCEPTION_E};

const static GT_U32 prvExpect_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_OAM_RAM_TX_PERIOD_EXCEPTION_E[] = {
    CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_TX_PERIOD_EXCEPTION_E,
    CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_TX_PERIOD_EXCEPTION_E};

const static GT_U32 prvExpect_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_OAM_RAM_INVALID_KEEPALIVE_HASH_EXCEPTION_E[] = {
    CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_INVALID_KEEPALIVE_HASH_E,
    CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_INVALID_KEEPALIVE_HASH_E};

const static GT_U32 prvExpect_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_OAM_RAM_MEL_EXCEPTION_E[] = {
    CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_MEG_EXCEPTION_E,
    CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_MEG_EXCEPTION_E};

const static GT_U32 prvExpect_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_OAM_OPCODE_PROFILE_E[] = {
    CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_OPCODE_PACKET_COMMAND_E,
    CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_OPCODE_PACKET_COMMAND_E};

const static GT_U32 prvExpect_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_SIP_PLR_E_ATTR_TBL_E[] = {
    CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_E_ATTRIBUTES_E,
    CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_E_ATTRIBUTES_E,
    CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_E_ATTRIBUTES_E};

const static GT_U32 prvExpect_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PCL_UDB_SELECTION_TABLE_E[] = {
    CPSS_DXCH_SIP5_TABLE_IPCL0_UDB_SELECT_E,
    CPSS_DXCH_SIP5_TABLE_IPCL1_UDB_SELECT_E,
    CPSS_DXCH_SIP5_TABLE_IPCL2_UDB_SELECT_E};

const static GT_U32 prvExpect_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_LPM_MEM_E[] = {
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 0),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 1),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 2),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 3),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 4),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 5),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 6),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 7),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 8),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 9),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 10),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 11),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 12),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 13),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 14),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 15),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 16),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 17),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 18),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 19)
};
const static GT_U32 prvExpect_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_METERING_SIGN_TBL_INGRESS_EVEN_E[] = {
    CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_0_METERING_CONFORM_SIGN_E,
    CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_1_METERING_CONFORM_SIGN_E};

const static GT_U32 prvExpect_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_METERING_SIGN_TBL_INGRESS_ODD_E[] = {
    CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_0_METERING_CONFORM_SIGN_E,
    CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_1_METERING_CONFORM_SIGN_E};

/* sip5.20 specific */
const static GT_U32 prvExpect__sip5_20_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EQ_STC_TABLE_E[] = {
    CPSS_DXCH_SIP5_TABLE_EQ_INGRESS_STC_PHYSICAL_PORT_E,
    CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_EQ_INGRESS_STC_PHYSICAL_PORT_PIPE_1_E};

const static GT_U32 prvExpect_sip5_20_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_IPFIX_WRAPAROUND_E[] = {
    CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_IPFIX_ALERT_WRAPAROUND_E,
    CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_IPFIX_ALERT_WRAPAROUND_E,
    CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_IPFIX_ALERT_WRAPAROUND_E};

const static GT_U32 prvExpect_sip5_20_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_IPFIX_AGING_E[] = {
    CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_IPFIX_ALERT_AGING_E,
    CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_IPFIX_ALERT_AGING_E,
    CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_IPFIX_ALERT_AGING_E};

const static GT_U32 prvExpect_sip5_20_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_METERING_SIGN_TBL_EVEN_E[] = {
    CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_0_METERING_CONFORM_SIGN_E,
    CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_1_METERING_CONFORM_SIGN_E,
    CPSS_DXCH_SIP5_15_TABLE_EGRESS_POLICER_METERING_CONFORM_SIGN_E};

const static GT_U32 prvExpect_sip5_20_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_METERING_SIGN_TBL_ODD_E[] = {
    CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_0_METERING_CONFORM_SIGN_E,
    CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_1_METERING_CONFORM_SIGN_E,
    CPSS_DXCH_SIP5_15_TABLE_EGRESS_POLICER_METERING_CONFORM_SIGN_E};

const static GT_U32 prvExpect_sip5_20_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_STC_E[] = {
    CPSS_DXCH_LION_TABLE_TXQ_EGRESS_STC_E,
    CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TXQ_EGRESS_STC__DQ_1_E,
    CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TXQ_EGRESS_STC__DQ_2_E,
    CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TXQ_EGRESS_STC__DQ_3_E,
    CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TXQ_EGRESS_STC__DQ_4_E,
    CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TXQ_EGRESS_STC__DQ_5_E
};

const static GT_U32  prvExpect_sip5_20_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PORT_E[] = {
    CPSS_DXCH_LION_TABLE_TXQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E,
    CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG__DQ_1_E,
    CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG__DQ_2_E,
    CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG__DQ_3_E,
    CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG__DQ_4_E,
    CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG__DQ_5_E
};

const static GT_U32  prvExpect_sip5_20_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PRIO_E[] = {
    CPSS_DXCH_LION_TABLE_TXQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG_E,
    CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG__DQ_1_E,
    CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG__DQ_2_E,
    CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG__DQ_3_E,
    CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG__DQ_4_E,
    CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG__DQ_5_E
};

const static GT_U32  prvExpect_sip5_20_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EQ_OAM_PROTECTION_LOC_TABLE_E[] = {
    CPSS_DXCH_SIP5_TABLE_EQ_PROTECTION_LOC_E,
    CPSS_DXCH_SIP5_20_TABLE_EQ_TX_PROTECTION_LOC_E
};

const static GT_U32  prvExpect_sip5_20_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IPLR_QOS_REMARKING_TABLE_INGRESS_E[] = {
    CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_REMARKING_E,
    CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_REMARKING_E
};

const static GT_U32  prvExpect_sip5_20_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_LPM_MEM_BASIC_E[] = {
    CPSS_DXCH_TABLE_LAST_E,
    CPSS_DXCH_TABLE_LAST_E,
    CPSS_DXCH_TABLE_LAST_E,
    CPSS_DXCH_TABLE_LAST_E,
    CPSS_DXCH_TABLE_LAST_E,
    CPSS_DXCH_TABLE_LAST_E,
    CPSS_DXCH_TABLE_LAST_E,
    CPSS_DXCH_TABLE_LAST_E,
    CPSS_DXCH_TABLE_LAST_E,
    CPSS_DXCH_TABLE_LAST_E,
    CPSS_DXCH_TABLE_LAST_E,
    CPSS_DXCH_TABLE_LAST_E,
    CPSS_DXCH_TABLE_LAST_E,
    CPSS_DXCH_TABLE_LAST_E,
    CPSS_DXCH_TABLE_LAST_E,
    CPSS_DXCH_TABLE_LAST_E,
    CPSS_DXCH_TABLE_LAST_E,
    CPSS_DXCH_TABLE_LAST_E,
    CPSS_DXCH_TABLE_LAST_E,
    CPSS_DXCH_TABLE_LAST_E,
    CPSS_DXCH_TABLE_LAST_E,
    CPSS_DXCH_TABLE_LAST_E,
    CPSS_DXCH_TABLE_LAST_E,
    CPSS_DXCH_TABLE_LAST_E,
    CPSS_DXCH_TABLE_LAST_E,
    CPSS_DXCH_TABLE_LAST_E,
    CPSS_DXCH_TABLE_LAST_E,
    CPSS_DXCH_TABLE_LAST_E,
    CPSS_DXCH_TABLE_LAST_E,
    CPSS_DXCH_TABLE_LAST_E,
    CPSS_DXCH_TABLE_LAST_E,
    CPSS_DXCH_TABLE_LAST_E,
    CPSS_DXCH_TABLE_LAST_E,
    CPSS_DXCH_TABLE_LAST_E,
    CPSS_DXCH_TABLE_LAST_E,
    CPSS_DXCH_TABLE_LAST_E,
    CPSS_DXCH_TABLE_LAST_E,
    CPSS_DXCH_TABLE_LAST_E,
    CPSS_DXCH_TABLE_LAST_E,
    CPSS_DXCH_TABLE_LAST_E,
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 0),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 1),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 2),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 3),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 4),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 5),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 6),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 7),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 8),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 9),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 10),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 11),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 12),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 13),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 14),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 15),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 16),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 17),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 18),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 19),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 20),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 21),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 22),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 23),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 24),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 25),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 26),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 27),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 28),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 29),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 30),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 31),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 32),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 33),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 34),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 35),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 36),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 37),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 38),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 39)
};

const static GT_U32  prvExpect_sip5_20_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TTI_PORT_MAP_E[] = {
    CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TTI_PORT_MAPPING__DQ_0_E,
    CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TTI_PORT_MAPPING__DQ_1_E,
    CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TTI_PORT_MAPPING__DQ_2_E,
    CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TTI_PORT_MAPPING__DQ_3_E,
    CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TTI_PORT_MAPPING__DQ_4_E,
    CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TTI_PORT_MAPPING__DQ_5_E
};


const static GT_U32 prvExpect_caelum_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_LPM_MEM_E[] = {
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 0),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 0),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 1),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 1),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 2),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 2),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 3),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 3),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 4),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 4),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 5),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 5),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 6),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 6),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 7),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 7),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 8),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 8),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 9),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 9),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 10),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 10),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 11),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 11),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 12),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 12),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 13),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 13),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 14),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 14),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 15),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 15),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 16),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 16),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 17),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 17),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 18),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 18),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 19),
    (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 19)
};


/* a comparison function to be passed into qsort */
static int compareIntFunc(const void *a, const void *b)
{
    return *(int*)a - *(int*)b;
}
static void prvDiagDataIntegrityDbCheck
(
    GT_U8       devNum,
    PRV_TGF_DFX_DB_TEST_STC  *testDbPtr
)
{
    PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    *dbArrayPtr;
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT       memType;
    GT_U32        hwTableName;
    GT_U32        dbArrayEntryNum;
    GT_U32        instanceNum;
    GT_U32        i;
    GT_U32        kk;
    GT_BOOL       isError = GT_TRUE;
    const GT_U32 *expectArr;
    GT_U32        *testTablesPtr;
    GT_U32        pgIndex;      /* portGroupId's index in an array */
    GT_STATUS     rc;

    /* assign DB pointer and size */
    prvCpssDxChDiagDataIntegrityDbPointerSet(devNum, &dbArrayPtr, &dbArrayEntryNum);
    if ((dbArrayPtr == NULL) || (dbArrayEntryNum == 0))
    {
        /* should not happen */
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "DB is absent" );
    }

    /* loop over all memories in DataIntegrity DB and gather info
     * about memType->hwTable bindings into local DB testDbPtr indexed
     * by memType */
    for(i = 0; i < dbArrayEntryNum; i++)
    {
        memType = dbArrayPtr[i].memType;
        hwTableName = dbArrayPtr[i].hwTableName;
        pgIndex = PRV_PORT_GROUP_TO_INDEX_MAC(dbArrayPtr[i].causePortGroupId);
        if (pgIndex >= PORT_GROUPS_NUM_CNS)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "Local DB size is too small (portGroupId number)" );
        }
        instanceNum = testDbPtr->ramInfo[memType][pgIndex].instanceAmount;
        testTablesPtr = &testDbPtr->ramInfo[memType][pgIndex].hwTableName[0];

        if (instanceNum)
        {
            /* this is not first RAM of the memType with specified portGroupId*/
            if (hwTableName != testTablesPtr[instanceNum - 1])
            {
                if (instanceNum >= TABLES_PER_RAM_MAX_CNS)
                {
                    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "Local DB size too small (hw tables number)" );
                }
                else
                {
                    testTablesPtr[instanceNum] = hwTableName;
                }

                testDbPtr->ramInfo[memType][pgIndex].instanceAmount++;
            }
        }
        else
        {
            /* first RAM */
            testTablesPtr[0] = dbArrayPtr[i].hwTableName;
            testDbPtr->ramInfo[memType][pgIndex].instanceAmount++;
        }
    }

    /* sort local DB memType's HW table list */
    for (memType=0; memType < CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_LAST_E; memType++)
    {
        for (pgIndex = 0; pgIndex < PORT_GROUPS_NUM_CNS; pgIndex++)
        {
            instanceNum = testDbPtr->ramInfo[memType][pgIndex].instanceAmount;
            if (instanceNum > 1)
            {
                rc = cpssOsQsort(testDbPtr->ramInfo[memType][pgIndex].hwTableName,
                                 instanceNum,
                                 sizeof(testDbPtr->ramInfo[memType][pgIndex].hwTableName[0]),
                                 compareIntFunc);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "qsort error" );
            }
        }
    }

    /* check local DB for multiple HW tables with same RAM */
    for(memType = 0; memType < CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_LAST_E; memType++)
    {
        for (pgIndex = 0; pgIndex < PORT_GROUPS_NUM_CNS; pgIndex++)
        {
            instanceNum = testDbPtr->ramInfo[memType][pgIndex].instanceAmount;
            if (instanceNum <= 1)
            {
                continue;
            }

            expectArr = NULL;
            switch (memType)
            {
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_CNC_COUNTERS_E:
                    expectArr = prvExpect_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_CNC_COUNTERS_E;
                    break;
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PCL_CONFIG_E:
                    expectArr = prvExpect_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PCL_CONFIG_E;
                    break;
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_IPFIX_AGING_E:
                    expectArr = PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) ?
                        prvExpect_sip5_20_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_IPFIX_AGING_E :
                        prvExpect_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_IPFIX_AGING_E;
                    break;
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_IPFIX_WRAPAROUND_E:
                    expectArr = PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) ?
                        prvExpect_sip5_20_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_IPFIX_WRAPAROUND_E :
                        prvExpect_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_IPFIX_WRAPAROUND_E;
                    break;
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_QOS_REMARKING_TABLE_E:
                    expectArr = prvExpect_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_QOS_REMARKING_TABLE_E;
                    break;
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_OAM_OAM_TABLE_E:
                    expectArr = prvExpect_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_OAM_OAM_TABLE_E;
                    break;
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_OAM_RAM_SOURCE_INTERFACE_EXCEPTION_E:
                    expectArr = prvExpect_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_OAM_RAM_SOURCE_INTERFACE_EXCEPTION_E;
                    break;
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_OAM_RAM_EXCESS_KEEPALIVE_EXCEPTION_E:

                    expectArr = prvExpect_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_OAM_RAM_EXCESS_KEEPALIVE_EXCEPTION_E;
                    break;
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_OAM_RAM_SUMMARY_EXCEPTION_E:
                    expectArr = prvExpect_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_OAM_RAM_SUMMARY_EXCEPTION_E;
                    break;
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_OAM_RAM_RDI_STATUS_CHANGE_EXCEPTION_E:
                    expectArr = prvExpect_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_OAM_RAM_RDI_STATUS_CHANGE_EXCEPTION_E;
                    break;
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_OAM_RAM_TX_PERIOD_EXCEPTION_E:
                    expectArr = prvExpect_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_OAM_RAM_TX_PERIOD_EXCEPTION_E;
                    break;
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_OAM_RAM_INVALID_KEEPALIVE_HASH_EXCEPTION_E:
                    expectArr = prvExpect_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_OAM_RAM_INVALID_KEEPALIVE_HASH_EXCEPTION_E;
                    break;
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_OAM_RAM_MEL_EXCEPTION_E:
                    expectArr = prvExpect_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_OAM_RAM_MEL_EXCEPTION_E;
                    break;
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_OAM_OPCODE_PROFILE_E:
                    expectArr = prvExpect_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_OAM_OPCODE_PROFILE_E;
                    break;
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_SIP_PLR_E_ATTR_TBL_E:
                    expectArr = prvExpect_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_SIP_PLR_E_ATTR_TBL_E;
                    break;
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PCL_UDB_SELECTION_TABLE_E:
                    expectArr = prvExpect_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PCL_UDB_SELECTION_TABLE_E;
                    break;
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_LPM_MEM_E:
                    /* every of LPM 0..19 is met twice in Caelum db:
                       first half of memories is relevant for both Cetus, Caelum,
                       second half of memories (pipe 3) is relevant for Caelum only */

                    expectArr = PRV_CPSS_DXCH_CAELUM_CHECK_MAC(devNum) ?
                        prvExpect_caelum_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_LPM_MEM_E :
                        PRV_CPSS_DXCH_ALDRIN2_CHECK_MAC(devNum) ?
                        prvExpect_sip5_20_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_LPM_MEM_BASIC_E :
                        prvExpect_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_LPM_MEM_E;

                    break;
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_METERING_SIGN_TBL_INGRESS_EVEN_E:
                    expectArr = prvExpect_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_METERING_SIGN_TBL_INGRESS_EVEN_E;
                    break;
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_METERING_SIGN_TBL_INGRESS_ODD_E:
                    expectArr = prvExpect_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_METERING_SIGN_TBL_INGRESS_ODD_E;
                    break;
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EQ_STC_TABLE_E:
                    if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
                    {
                        expectArr = prvExpect__sip5_20_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EQ_STC_TABLE_E;
                    }
                    break;
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_STC_E:
                    if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
                    {
                        /* bobcat3: first 3 tables are met in pipe 0 only ,
                           second 3 tables - in pipe 1  only */
                        expectArr = prvExpect_sip5_20_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_STC_E;
                        if (PRV_INDEX_TO_PORT_GROUP_MAC(pgIndex))
                        {
                            expectArr += 3;
                        }
                    }
                    break;
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PORT_E:
                    if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
                    {
                        /* bobcat3: first 3 tables are met in pipe 0 only ,
                           second 3 tables - in pipe 1  only */
                        expectArr = prvExpect_sip5_20_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PORT_E;
                        if (PRV_INDEX_TO_PORT_GROUP_MAC(pgIndex))
                        {
                            expectArr += 3;
                        }
                    }
                    break;
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PRIO_E:
                    if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
                    {
                        /* bobcat3: first 3 tables are met in pipe 0 only ,
                           second 3 tables - in pipe 1  only */
                        expectArr = prvExpect_sip5_20_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PRIO_E;
                        if (PRV_INDEX_TO_PORT_GROUP_MAC(pgIndex))
                        {
                            expectArr += 3;
                        }
                    }
                    break;
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EQ_OAM_PROTECTION_LOC_TABLE_E:
                    if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
                    {
                        expectArr = prvExpect_sip5_20_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EQ_OAM_PROTECTION_LOC_TABLE_E;
                    }
                    break;
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IPLR_QOS_REMARKING_TABLE_INGRESS_E:
                    expectArr = prvExpect_sip5_20_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IPLR_QOS_REMARKING_TABLE_INGRESS_E;
                    break;
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_LPM_MEM_BASIC_E:
                    expectArr = prvExpect_sip5_20_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_LPM_MEM_BASIC_E;
                    break;
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TTI_PORT_MAP_E:
                    /* this memory appeared in sip5.20 first */
                    expectArr = prvExpect_sip5_20_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TTI_PORT_MAP_E;
                    /* bobcat3: first 3 tables are met in pipe 0 only ,
                       second 3 tables - in pipe 1  only */
                    if (PRV_INDEX_TO_PORT_GROUP_MAC(pgIndex))
                    {
                        expectArr += 3;
                    }
                    break;
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_METERING_SIGN_TBL_ODD_E:
                    expectArr = prvExpect_sip5_20_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_METERING_SIGN_TBL_ODD_E;
                    break;
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_METERING_SIGN_TBL_EVEN_E:
                    expectArr = prvExpect_sip5_20_HW_tbl_for_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_METERING_SIGN_TBL_EVEN_E;
                    break;
                default:
                    break;
            }

            if (expectArr == NULL)
            {
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "Unknown expected results" );
                prvTgfLogTablesNames(PRV_DONT_PRINT_NAME, memType);
            }
            else
            {
                if (cpssOsMemCmp(testDbPtr->ramInfo[memType][pgIndex].hwTableName,
                                 expectArr,
                                 instanceNum*sizeof(GT_U32)))
                {
                    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "More than one HW table for RAM" );
                    PRV_UTF_LOG1_MAC("PG %d, ", PRV_INDEX_TO_PORT_GROUP_MAC(pgIndex));
                    prvTgfLogTablesNames(PRV_DONT_PRINT_NAME, memType);
                    for (kk = 0; kk < instanceNum; kk++)
                    {
                        PRV_UTF_LOG0_MAC(" - ");
                        prvTgfLogTablesNames(testDbPtr->ramInfo[memType][pgIndex].hwTableName[kk], PRV_DONT_PRINT_NAME);
                    }
                }
            }
        }
    }

    /* check multiple RAMs with same HW Table */
    for (hwTableName = 0; hwTableName < CPSS_DXCH_TABLE_LAST_E; hwTableName++)
    {
        #define MAX_RAM_FOR_HW_TBL 10

        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT instanceArr[MAX_RAM_FOR_HW_TBL];


        for (pgIndex = 0; pgIndex < PORT_GROUPS_NUM_CNS; pgIndex++)
        {
            cpssOsBzero((GT_CHAR*)instanceArr, sizeof(instanceArr));
            instanceNum = 0;

            for(memType = 0; memType < CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_LAST_E; memType++)
            {
                for (kk = 0; kk < testDbPtr->ramInfo[memType][pgIndex].instanceAmount; kk++)
                {
                    if (testDbPtr->ramInfo[memType][pgIndex].hwTableName[kk] == hwTableName)
                    {
                        if (instanceNum >= MAX_RAM_FOR_HW_TBL)
                        {
                            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "Local DB size too small" );
                        }
                        else
                        {
                            instanceArr[instanceNum++] = memType;
                        }
                    }
                }
            }

            if (instanceNum > 1)
            {
                CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT iplrSignExpectedArr[] = {
                    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_METERING_SIGN_TBL_INGRESS_EVEN_E,
                    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_METERING_SIGN_TBL_INGRESS_ODD_E };

                CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT eplrSignExpectedArr[] = {
                    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EPLR_METERING_SIGN_TBL_EGRESS_EVEN_E,
                    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EPLR_METERING_SIGN_TBL_EGRESS_ODD_E };

                CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT plrSignExpectedArr [] = {
                    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_METERING_SIGN_TBL_EVEN_E,
                    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_METERING_SIGN_TBL_ODD_E };

                CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT *cmpArr;
                isError = GT_TRUE;

                switch (hwTableName)
                {
                    case CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_0_METERING_CONFORM_SIGN_E:
                    case CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_1_METERING_CONFORM_SIGN_E:
                        cmpArr = PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) ? plrSignExpectedArr : iplrSignExpectedArr;
                        break;
                    case CPSS_DXCH_SIP5_15_TABLE_EGRESS_POLICER_METERING_CONFORM_SIGN_E:
                        cmpArr = PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) ? plrSignExpectedArr : eplrSignExpectedArr;
                        break;
                    default:
                        cmpArr = NULL;
                }
                if (cmpArr &&
                    0 == cpssOsMemCmp(cmpArr, instanceArr, instanceNum*sizeof(CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT)))
                {
                    isError = GT_FALSE;
                }

                if (isError)
                {
                    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "More than one RAM for HW table" );
                    prvTgfLogTablesNames(hwTableName, PRV_DONT_PRINT_NAME);
                    for (kk = 0; kk < instanceNum; kk++)
                    {
                        PRV_UTF_LOG0_MAC(" - ");
                        prvTgfLogTablesNames(PRV_DONT_PRINT_NAME, instanceArr[kk]);
                    }
                }
            }
        }
    }

    if (traceEvents)
    {
        /* print table */
        for (pgIndex = 0; pgIndex < PORT_GROUPS_NUM_CNS; pgIndex++)
        {
            for(memType = 0; memType < CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_LAST_E; memType++)
            {
                instanceNum = testDbPtr->ramInfo[memType][pgIndex].instanceAmount;
                if (instanceNum)
                {
                    PRV_UTF_LOG1_MAC("PG %d, ", PRV_INDEX_TO_PORT_GROUP_MAC(pgIndex));
                    prvTgfLogTablesNames(PRV_DONT_PRINT_NAME, memType);
                    for (kk = 0; kk < instanceNum; kk++)
                    {
                        PRV_UTF_LOG0_MAC(" - ");
                        prvTgfLogTablesNames(testDbPtr->ramInfo[memType][pgIndex].hwTableName[kk],
                                             PRV_DONT_PRINT_NAME);
                    }
                }
            }
        }
    }
    return;
}

/* Test checks validity of RAM DB and to HW table convertions */
UTF_TEST_CASE_MAC(cpssDxChDiagDataIntegrityDbCheck)
{
/*
    ITERATE_DEVICES(Bobcat2, Aldrin, AC3X, Bobcat3)
*/
    GT_U8       dev = 0;
    PRV_TGF_DFX_DB_TEST_STC  *testDbPtr;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION2_E | UTF_LION_E | UTF_XCAT2_E);

    testDbPtr = (PRV_TGF_DFX_DB_TEST_STC *)cpssOsMalloc(sizeof(PRV_TGF_DFX_DB_TEST_STC));
    if (testDbPtr == NULL)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_OUT_OF_CPU_MEM, "Out of CPU Memory" );
        return;
    }

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        cpssOsBzero((GT_CHAR*)testDbPtr, sizeof(PRV_TGF_DFX_DB_TEST_STC));
        prvDiagDataIntegrityDbCheck(dev, testDbPtr);
    }
    cpssOsFree(testDbPtr);
}


/*-----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChDiagDataIntegrity suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChDiagDataIntegrityTables)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagDataIntegrityDbCheck)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagDataIntegrityTableScan)
#ifndef ASIC_SIMULATION
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagDataIntegrityHwTablesErrorInjectionTest)
#endif
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagDataIntegrityTableScan_1)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagDataIntegrityTableEntryFix)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagDataIntegrityTables_shadowTest)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagDataIntegrityShadowEntryInfoGet)

#ifdef TEST_FIX_ERROR
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagDataIntegrityTableEntryFix_1)
#endif /*TEST_FIX_ERROR*/

#ifndef ASIC_SIMULATION
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagDataIntegrityHwTablesInfoGetTest)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagDataIntegrityLogicalTablesTsArpTest)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagDataIntegrityShadowDbWeakHwBit)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagDataIntegrityLogicalTablesPolicerTest)
#endif

    /* should be last to clean after previous tests */
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagDataIntegrityTables_cleanup_withSystemReset)
UTF_SUIT_END_TESTS_MAC(cpssDxChDiagDataIntegrityTables)
