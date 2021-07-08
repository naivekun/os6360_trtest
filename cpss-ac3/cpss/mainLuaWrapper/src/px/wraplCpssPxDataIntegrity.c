 /*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* wraplCpssPxDataIntegrity.c
*
* DESCRIPTION:
*       A lua wrapper for PX Data Integrity.
*
*
* DEPENDENCIES:
*
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#include <cpss/common/cpssTypes.h>
#include <generic/private/prvWraplGeneral.h>
#include <cpss/px/diag/cpssPxDiagDataIntegrity.h>
#include <cpss/px/diag/cpssPxDiagDataIntegrityTables.h>
#include <extUtils/luaCLI/luaCLIEngine_genWrapper.h>
#include <cpssCommon/wrapCpssDebugInfo.h>
#include <appDemo/userExit/px/appDemoPxDiagDataIntegrityAux.h>
#include <cpss/px/diag/private/prvCpssPxDiagDataIntegrityMainMappingDb.h>

/* value that should be treated as unknown */
#define PRV_CPSS_PX_DIAG_DATA_INTEGRITY_UNASSIGNED_CNS ((GT_U32)-1)

#define QUIT_IF_NOT_OK(_rc) \
    if(GT_OK != _rc)\
    {\
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);\
        return 1;\
    }


typedef struct{

    CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_STC           eventInfo;
    GT_U32                                          errorCounter;
    GT_U32                                          failedRow;
    GT_U32                                          failedSegment;
    GT_U32                                          failedSyndrome;

} CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_FULL_INFO_STC;

typedef struct{

    CPSS_PX_LOGICAL_TABLE_ENT                              table;
    GT_BOOL                                                errorCountEnable;
    CPSS_EVENT_MASK_SET_ENT                                eventMask;
    CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT          errorType;
    GT_BOOL                                                injectEnable;
    CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_ENT         injectMode;
    CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT protectionType;
}CPSS_PX_DIAG_DATA_INTEGRITY_CONFIG_STATUS_STC;

typedef struct{

    CPSS_PX_LOGICAL_TABLE_ENT                              table;
    GT_U32                                                 tableSize;
}CPSS_PX_DIAG_DATA_INTEGRITY_SHADOW_SIZE_STC;


use_prv_enum(CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT);
use_prv_enum(CPSS_PX_LOGICAL_TABLE_ENT);
use_prv_enum(CPSS_EVENT_MASK_SET_ENT);
use_prv_enum(CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT);
use_prv_enum(CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_ENT);
use_prv_enum(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT);

use_prv_struct(CPSS_PX_LOCATION_SPECIFIC_INFO_STC);

extern TEST_PX_DIAG_DATA_INTEGRITY_COUNTER_DB_STC * appDemoPxDiagDataIntegrityCountersDbGet(GT_VOID);

void prv_c_to_lua_CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_FULL_INFO_STC(
    lua_State *L,
    CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_FULL_INFO_STC *val
);

void prv_c_to_lua_CPSS_PX_DIAG_DATA_INTEGRITY_CONFIG_STATUS_STC(
    lua_State *L,
    CPSS_PX_DIAG_DATA_INTEGRITY_CONFIG_STATUS_STC *val
);

void prv_c_to_lua_CPSS_PX_DIAG_DATA_INTEGRITY_SHADOW_SIZE_STC
(
    lua_State *L,
    CPSS_PX_DIAG_DATA_INTEGRITY_SHADOW_SIZE_STC *val
);

GT_BOOL traceWrl = GT_FALSE;

CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_FULL_INFO_STC       myTestEventsArr[20];
GT_U32 myErrorCount[20],myFailedRow[20],myFailedSegment[20],myFAiledSyndrome[20];

void testFillErrorArray(void)
{

    GT_U32 i;

    for (i = 0; i<20; i++)
    {
        myTestEventsArr[i].eventInfo.eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
        myTestEventsArr[i].eventInfo.location.logicalEntryInfo.logicalTableType = CPSS_PX_LOGICAL_TABLE_TAIL_DROP_LIMITS_E;
        myTestEventsArr[i].eventInfo.location.hwEntryInfo.hwTableType = CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_LIMITS_DP0_E +i;
        myTestEventsArr[i].eventInfo.location.hwEntryInfo.hwTableEntryIndex = i;
        myTestEventsArr[i].eventInfo.location.ramEntryInfo.memType = CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_EQ_Q_LIMIT_DP0_E;
        myTestEventsArr[i].eventInfo.location.ramEntryInfo.ramRow = 0;
        myTestEventsArr[i].eventInfo.location.ramEntryInfo.memLocation.dfxPipeId = 0;
        myTestEventsArr[i].eventInfo.location.ramEntryInfo.memLocation.dfxClientId = 5;
        myTestEventsArr[i].eventInfo.location.ramEntryInfo.memLocation.dfxMemoryId = 12;
       /*myTestEventsArr[i].failedSegment = 6;
        myTestEventsArr[i].failedSyndrome = 7;*/
        myTestEventsArr[i].errorCounter= i+4;
        myTestEventsArr[i].failedRow = i+5;
        myTestEventsArr[i].failedSegment = i+2;
        myTestEventsArr[i].failedSyndrome = i+1;
    }
}
/*******************************************************************************
* wrlCpssPxDataIntegrityEventMaskSet
*
* DESCRIPTION:
*       Mask/unmask ECC/Parity event.
*
* APPLICABLE DEVICES:
*       Pipe.
*
* NOT APPLICABLE DEVICES:
*       None.
*
*
* INPUTS:
*       L                   - lua state

     GT_SW_DEV_NUM                               devNum,
     CPSS_PX_LOGICAL_TABLE_ENT                   table
     CPSS_EVENT_MASK_SET_ENT                     operation
*
* OUTPUTS:
*       None
*
* RETURNS:
*       1 and result pushed to lua stack (see OUTPUTS)
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssPxDataIntegrityEventMaskSet
(
    IN lua_State                            *L
)
{
    GT_STATUS                                   status;
    GT_SW_DEV_NUM                               devNum;
    CPSS_PX_LOGICAL_TABLE_ENT                   table;
    CPSS_EVENT_MASK_SET_ENT                     operation;

    CPSS_PX_LOCATION_SPECIFIC_INFO_STC          memEntry;
    CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT  protectionType;


    CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorType = CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_PARITY_E;
    status = GT_OK;
    PARAM_NUMBER(status, devNum, 1, GT_SW_DEV_NUM);
    QUIT_IF_NOT_OK(status);
    PARAM_ENUM(status, table, 2, CPSS_PX_LOGICAL_TABLE_ENT);
    QUIT_IF_NOT_OK(status);
    PARAM_ENUM(status, operation, 3, CPSS_EVENT_MASK_SET_ENT);
    QUIT_IF_NOT_OK(status);

    if (traceWrl)
    {
        cpssOsPrintf("wrlCpssPxDataIntegrityEventMaskSet table %d operation %d\n", table,operation);
    }

    memEntry.type = CPSS_PX_LOCATION_LOGICAL_INDEX_INFO_TYPE;
    memEntry.info.logicalEntryInfo.logicalTableType = table;

    status = cpssPxDiagDataIntegrityProtectionTypeGet(devNum, &memEntry, &protectionType);
    QUIT_IF_NOT_OK(status);

    if (protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_PARITY_E)
    {
         errorType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
    }
    else if (protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ECC_E)
    {
         errorType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
    }

    status = cpssPxDiagDataIntegrityEventMaskSet(devNum, &memEntry, errorType, operation);

    lua_pushinteger(L, (lua_Integer)status);

    return 1;
}
/*******************************************************************************
* wrlCpssPxDiagDataIntegrityErrorCountEnableSet
*
* DESCRIPTION:
*       Enable/disable Data Integrity Error counter
*
* APPLICABLE DEVICES:
*       Pipe.
*
* NOT APPLICABLE DEVICES:
*       None.
*
*
* INPUTS:
*       L                   - lua state
*
*    GT_SW_DEV_NUM                               devNum,
*    CPSS_PX_LOGICAL_TABLE_ENT                   table
*    GT_BOOL                                     countEnable
*
* OUTPUTS:
*       None
*
* RETURNS:
*       1 and result pushed to lua stack (see OUTPUTS)
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssPxDataIntegrityErrorCountEnableSet
(
    IN lua_State                            *L
)
{
    GT_STATUS                                   status;
    GT_SW_DEV_NUM                               devNum;
    CPSS_PX_LOGICAL_TABLE_ENT                   table;
    GT_BOOL                                     countEnable;

    CPSS_PX_LOCATION_SPECIFIC_INFO_STC          memEntry;
    CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT  protectionType;


    CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorType = CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_PARITY_E;
    status = GT_OK;
    PARAM_NUMBER(status, devNum, 1, GT_SW_DEV_NUM);
    QUIT_IF_NOT_OK(status);
    PARAM_ENUM(status, table, 2, CPSS_PX_LOGICAL_TABLE_ENT);
    QUIT_IF_NOT_OK(status);
    PARAM_NUMBER(status, countEnable, 3, GT_BOOL);
    QUIT_IF_NOT_OK(status);

    memEntry.type = CPSS_PX_LOCATION_LOGICAL_INDEX_INFO_TYPE;
    memEntry.info.logicalEntryInfo.logicalTableType = table;

    status = cpssPxDiagDataIntegrityProtectionTypeGet(devNum, &memEntry, &protectionType);
    QUIT_IF_NOT_OK(status);

    if (protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_PARITY_E)
    {
         errorType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
    }
    else if (protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ECC_E)
    {
         errorType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
    }

    status = cpssPxDiagDataIntegrityErrorCountEnableSet(devNum, &memEntry, errorType, countEnable);

    lua_pushinteger(L, (lua_Integer)status);

    return 1;
}
/*******************************************************************************
* wrlCpssPxDiagDataIntegrityErrorInjectionConfigSet
*
* DESCRIPTION:
*       Configure Data Integrity Error Injection
*
* APPLICABLE DEVICES:
*       Pipe.
*
* NOT APPLICABLE DEVICES:
*       None.
*
*
* INPUTS:
*       L                   - lua state
*
*    GT_SW_DEV_NUM                                   devNum,
*    CPSS_PX_LOGICAL_TABLE_ENT                       table
*    CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_ENT  injectMode,
*    GT_BOOL                                         injectionEnable
*
* OUTPUTS:
*       None
*
* RETURNS:
*       1 and result pushed to lua stack (see OUTPUTS)
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssPxDiagDataIntegrityErrorInjectionConfigSet
(
    IN lua_State                            *L
)
{
    GT_STATUS                                       status;
    GT_SW_DEV_NUM                                   devNum;
    CPSS_PX_LOGICAL_TABLE_ENT                       table;
    CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_ENT  injectMode;
    GT_BOOL                                         injectEnable;

    CPSS_PX_LOCATION_SPECIFIC_INFO_STC          memEntry;

    status = GT_OK;
    PARAM_NUMBER(status, devNum, 1, GT_SW_DEV_NUM);
    QUIT_IF_NOT_OK(status);
    PARAM_ENUM(status, table, 2, CPSS_PX_LOGICAL_TABLE_ENT);
    QUIT_IF_NOT_OK(status);
    PARAM_ENUM(status, injectMode, 3, CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_ENT);
    QUIT_IF_NOT_OK(status);
    PARAM_NUMBER(status, injectEnable, 4, GT_BOOL);
    QUIT_IF_NOT_OK(status);
    if (traceWrl)
    {
        cpssOsPrintf("ErrorInjectionConfigSet table %d injectEnable %d\n", table,injectEnable);
    }
    memEntry.type = CPSS_PX_LOCATION_LOGICAL_INDEX_INFO_TYPE;
    memEntry.info.logicalEntryInfo.logicalTableType = table;

    status = cpssPxDiagDataIntegrityErrorInjectionConfigSet(devNum, &memEntry, injectMode, injectEnable);

    lua_pushinteger(L, (lua_Integer)status);

    return 1;
}

/*******************************************************************************
* wrlCpssPxDiagDataIntegrityErrorInjectionConfigSet
*
* DESCRIPTION:
*       Configure Data Integrity Error Injection
*
* APPLICABLE DEVICES:
*       Pipe.
*
* NOT APPLICABLE DEVICES:
*       None.
*
*
* INPUTS:
*       L                   - lua state
*
*    GT_SW_DEV_NUM                                   devNum,
*    CPSS_PX_LOGICAL_TABLE_ENT                       table
*    CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_ENT  injectMode,
*    GT_BOOL                                         injectionEnable
*
* OUTPUTS:
*       None
*
* RETURNS:
*       1 and result pushed to lua stack (see OUTPUTS)
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssPxDiagDataIntegrityTableEntryFix
(
    IN lua_State                            *L
)
{
    GT_STATUS                                       status;
    GT_SW_DEV_NUM                                   devNum;
    CPSS_PX_LOGICAL_TABLE_ENT                       table;
    GT_U32                                          index;

    CPSS_PX_LOCATION_SPECIFIC_INFO_STC          memEntry;

    status = GT_OK;
    PARAM_NUMBER(status, devNum, 1, GT_SW_DEV_NUM);
    QUIT_IF_NOT_OK(status);
    PARAM_ENUM(status, table, 2, CPSS_PX_LOGICAL_TABLE_ENT);
    QUIT_IF_NOT_OK(status);
    PARAM_NUMBER(status, index, 3, GT_U32);
    QUIT_IF_NOT_OK(status);

    memEntry.type = CPSS_PX_LOCATION_LOGICAL_INDEX_INFO_TYPE;
    memEntry.info.logicalEntryInfo.logicalTableType = table;
    memEntry.info.logicalEntryInfo.logicalTableEntryIndex = index;
    status = cpssPxDiagDataIntegrityTableEntryFix(devNum, &memEntry);

    lua_pushinteger(L, (lua_Integer)status);

    return 1;
}

/*******************************************************************************
* wrlCpssPxDiagDataIntegrityTableScan
*
* DESCRIPTION:
*       Scans the entries in the referenced table
*
* APPLICABLE DEVICES:
*       Pipe.
*
* NOT APPLICABLE DEVICES:
*       None.
*
*
* INPUTS:
*       L                   - lua state
*
*    GT_SW_DEV_NUM                                   devNum,
*    CPSS_PX_LOGICAL_TABLE_ENT                       table
*    GT_U32                                          startEntry
*    GT_U32                                          endEntry
*
* OUTPUTS:
*       None
*
* RETURNS:
*       1 and result pushed to lua stack (see OUTPUTS)
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssPxDiagDataIntegrityTableScan
(
    IN lua_State                            *L
)
{
    GT_STATUS                                       status;
    GT_SW_DEV_NUM                                   devNum;
    CPSS_PX_LOGICAL_TABLE_ENT                       table;
    GT_U32                                          startEntry,numOfEntries;
    GT_U32                                          nextEntryIndex;
    GT_BOOL                                         wasWrapAround;
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC              memEntry;

    status = GT_OK;

    PARAM_NUMBER(status, devNum, 1, GT_SW_DEV_NUM);
    QUIT_IF_NOT_OK(status);
    PARAM_ENUM(status, table, 2, CPSS_PX_LOGICAL_TABLE_ENT);
    QUIT_IF_NOT_OK(status);
    PARAM_NUMBER(status, startEntry, 3, GT_U32);
    QUIT_IF_NOT_OK(status);
    PARAM_NUMBER(status, numOfEntries, 4, GT_U32);
    QUIT_IF_NOT_OK(status);

    memEntry.type = CPSS_PX_LOCATION_LOGICAL_INDEX_INFO_TYPE;
    memEntry.info.logicalEntryInfo.logicalTableType = table;
    memEntry.info.logicalEntryInfo.logicalTableEntryIndex = startEntry;

    status = cpssPxDiagDataIntegrityTableScan(devNum,&memEntry,numOfEntries,&nextEntryIndex,&wasWrapAround);

    lua_pushinteger(L, (lua_Integer)status);

    return 1;
}

/*******************************************************************************
* pvrCpssPxDiagDataIntegrityMemoryLocationFromLuaGet
*
* DESCRIPTION:
*       CPSS LUA wrapper to get data integrity memory location from lua stack.
*
* APPLICABLE DEVICES:
*       Pipe.
*
* NOT APPLICABLE DEVICES:
*       None.
*
*
* INPUTS:
*       L                         - lua state
*       L_index                   - entry lua stack index
*       memType                   - data integrity memory type
*
* OUTPUTS:
*       locationPtr               - data integrity memory location
*       errorMessagePtr           - error message
*
* RETURNS:
*       operation succeeded or error code
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS pvrCpssPxDiagDataIntegrityMemoryLocationFromLuaGet
(
    IN  lua_State                                       *L,
    IN  GT_32                                           L_index,
    IN  CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT        memType,
    OUT CPSS_DIAG_DATA_INTEGRITY_PX_MEMORY_LOCATION_UNT *locationPtr,
    OUT GT_CHAR_PTR                                     *errorMessagePtr
)
{
    GT_STATUS                   status          = GT_OK;

    *errorMessagePtr = NULL;

    lua_pushvalue(L, L_index);

    if((memType >= CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TABLE_LAST_E) || (memType < CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_MG_MGCAM_E))
    {
        status              = GT_BAD_PARAM;
        *errorMessagePtr    = "Wrong type of integrity memory";
        lua_pop(L, 1);
        return status;
    }
    switch (memType)
    {
    /*  mppmMemLocation  - special memory location indexes for MPPM memory    */
    case CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_BANK_RAM_E:
    case CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_RX_IDDB_MEM_E:
    case CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_TXDMA_RD_BURST_FIFO_E:
        PARAM_NUMBER(status, locationPtr->mppmMemLocation.bankId, L_index, GT_U32);
        PARAM_NUMBER(status, locationPtr->mppmMemLocation.mppmId, L_index+1, GT_U32);
        break;

    /*  memLocation      - regular memory location indexes                    */
    default:
        PARAM_NUMBER(status, locationPtr->memLocation.dfxPipeId, L_index, GT_U32);
        PARAM_NUMBER(status, locationPtr->memLocation.dfxClientId, L_index+1, GT_U32);
        PARAM_NUMBER(status, locationPtr->memLocation.dfxMemoryId, L_index+2,   GT_U32);
        break;
    }

    lua_pop(L, 1);

    return status;
}
/*******************************************************************************
* wrlCpssPxDiagDataIntegrityErrorInfoGet
*
* DESCRIPTION:
*       Function gets ECC/Parity error info.
*
* APPLICABLE DEVICES:
*       Pipe.
*
* NOT APPLICABLE DEVICES:
*       None.
*
*
* INPUTS:
*       L                            - lua state:
*       L[1]       devNum            - PP device number
*       L[2]       memType           - type of memory(table)
*       L[3,4,5]   locationPtr       - (pointer to) memory location indexes
*
* OUTPUTS:
*       pushed to lua stack:
*           errorCounterPtr    - (pointer to) error counter
*           failedRowPtr       - (pointer to) failed raw
*           failedSyndromePtr  - (pointer to) failed syndrome
*
* RETURNS:
*       status
*
* COMMENTS:
*       Error status or counter that isn't supported returns 0xFFFFFFFF
*
*******************************************************************************/

int wrlCpssPxDiagDataIntegrityErrorInfoGet
(
    IN lua_State *L
)
{
    GT_U8                                           devNum;
    CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT        memType;
    CPSS_DIAG_DATA_INTEGRITY_PX_MEMORY_LOCATION_UNT location;
    GT_U32                                          errorCounter;
    GT_U32                                          failedRow;
    GT_U32                                          failedSegment;
    GT_U32                                          failedSyndrome;
    GT_STATUS                                       status             = GT_OK;
    GT_CHAR_PTR                                     error_message      = NULL;

    CPSS_PX_LOCATION_SPECIFIC_INFO_STC memEntry;
    CPSS_DIAG_DATA_INTEGRITY_MPPM_MEMORY_LOCATION_STC *mppmPtr = NULL;

    PARAM_NUMBER(status, devNum, 1, GT_SW_DEV_NUM);
    QUIT_IF_NOT_OK(status);
    PARAM_ENUM  (status,  memType,  2, CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT);
    QUIT_IF_NOT_OK(status);

    status = pvrCpssPxDiagDataIntegrityMemoryLocationFromLuaGet(L, 3, memType, &location, &error_message);
    QUIT_IF_NOT_OK(status);

    switch (memType)
    {
        /*  mppmMemLocation  - special memory location indexes for MPPM memory    */
        case CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_BANK_RAM_E:
        case CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_RX_IDDB_MEM_E:
        case CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_TXDMA_RD_BURST_FIFO_E:
            mppmPtr = &location.mppmMemLocation;
            break;
        default:
            break;
    }
    memEntry.type = CPSS_PX_LOCATION_RAM_INDEX_INFO_TYPE;
    memEntry.info.ramEntryInfo.memType = memType;
    memEntry.info.ramEntryInfo.memLocation.dfxPipeId = location.memLocation.dfxPipeId;
    memEntry.info.ramEntryInfo.memLocation.dfxClientId = location.memLocation.dfxClientId;
    memEntry.info.ramEntryInfo.memLocation.dfxMemoryId = location.memLocation.dfxMemoryId;


    status = cpssPxDiagDataIntegrityErrorInfoGet(devNum, &memEntry, mppmPtr,
                                                   &errorCounter, &failedRow,
                                                   &failedSegment, &failedSyndrome);

    lua_pushinteger(L, (lua_Integer)status);
    lua_pushinteger(L, (lua_Integer)errorCounter);
    lua_pushinteger(L, (lua_Integer)failedRow);
    lua_pushinteger(L, (lua_Integer)failedSyndrome);
    return 4;
}


/*******************************************************************************
* wrlCpssPxDiagDataIntegrityEventsGet
*
* DESCRIPTION:
*       returns array of data integrity events.
*
* APPLICABLE DEVICES: PIPE
*
*
* NOT APPLICABLE DEVICES: None
*
* INPUTS:
*       L                     - lua state
*
* OUTPUTS:
*
* RETURNS:
*       status, CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_STC[]
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssPxDiagDataIntegrityEventsGet
(
    IN lua_State *L
)
{
    GT_SW_DEV_NUM     devNum;
    GT_STATUS         status = GT_OK;
    CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_FULL_INFO_STC  eventsFullInfoArr[10];
    GT_U32      i, idx;
    GT_U32 currEventCount,currMemType,currentDbKey,currEventType;
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC memEntry;
    CPSS_DIAG_DATA_INTEGRITY_MPPM_MEMORY_LOCATION_STC mppmMemLocation, *mppmMemLocationPtr = NULL;
    TEST_PX_DIAG_DATA_INTEGRITY_COUNTER_DB_STC * dbEventCounterPtr = appDemoPxDiagDataIntegrityCountersDbGet();

    PARAM_NUMBER(status, devNum, 1, GT_SW_DEV_NUM);
    QUIT_IF_NOT_OK(status);
    cpssOsMemSet(eventsFullInfoArr, 0, sizeof(eventsFullInfoArr));


    lua_pushinteger(L, (lua_Integer)status);
    if (status != GT_OK)
    {
        return 1;
    }

    lua_newtable(L);
    idx = 0;

    /* read data */
    for (currentDbKey = 0; currentDbKey < BIT_17; currentDbKey++)
    {
        if (dbEventCounterPtr->eventCounterArr[currentDbKey] == 0)
        {
            continue;
        }

        currEventCount = dbEventCounterPtr->eventCounterArr[currentDbKey];
        currMemType = dbEventCounterPtr->memTypeArr[currentDbKey];
        if (traceWrl)
        {
            cpssOsPrintf("prvCpssPxDiagDataIntegrityCountersDbCheck counter %d key %x \n",currEventCount, currentDbKey);
        }

        if((currMemType == CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_BANK_RAM_E) ||
           (currMemType == CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_RX_IDDB_MEM_E) ||
           (currMemType == CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_TXDMA_RD_BURST_FIFO_E))
        {
            mppmMemLocation.mppmId = U32_GET_FIELD_MAC(currentDbKey,  0 , 1);
            mppmMemLocation.bankId = U32_GET_FIELD_MAC(currentDbKey,  1 , 1);
            mppmMemLocationPtr = &mppmMemLocation;
        }
        else
        {
            memEntry.info.ramEntryInfo.memLocation.dfxMemoryId    = U32_GET_FIELD_MAC(currentDbKey,  0 , 7);
            memEntry.info.ramEntryInfo.memLocation.dfxClientId  = U32_GET_FIELD_MAC(currentDbKey,  7 , 5);
            memEntry.info.ramEntryInfo.memLocation.dfxPipeId  = U32_GET_FIELD_MAC(currentDbKey, 12 , 3);

            if (traceWrl)
            {
                cpssOsPrintf("dfxPipeId = 0x%x\r\n", memEntry.info.ramEntryInfo.memLocation.dfxPipeId);
                cpssOsPrintf("dfxClientId = 0x%x\r\n", memEntry.info.ramEntryInfo.memLocation.dfxClientId);
                cpssOsPrintf("dfxMemoryId = %d\r\n", memEntry.info.ramEntryInfo.memLocation.dfxMemoryId);

                cpssOsPrintf("currentDbKey = 0x%x\r\n", currentDbKey);
                cpssOsPrintf("eventCounter = 0x%x\r\n", currEventCount);
                cpssOsPrintf("eventsType = 0x%x\r\n", (currentDbKey >> 15) & 0x3);
                cpssOsPrintf("memType = 0x%x\r\n", currMemType);
            }
        }
        switch ((currentDbKey >> 15) & 0x3)
        {
            case 1:
                currEventType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
                break;
            case 2:
                currEventType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
                break;
            case 3:
                currEventType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E;
                break;
            default:
                continue;
        }
        if (traceWrl)
        {
            cpssOsPrintf("wrlCpssPxDiagDataIntegrityEventsGet key %x  eventType %d \n", currentDbKey, currEventType);
        }

        for (i =0;  i < currEventCount; i++)
        {
            eventsFullInfoArr[i].eventInfo.eventsType = currEventType;
            memEntry.type = CPSS_PX_LOCATION_RAM_INDEX_INFO_TYPE;
            memEntry.info.ramEntryInfo.memType = currMemType;

            status = cpssPxDiagDataIntegrityErrorInfoGet(devNum, &memEntry, mppmMemLocationPtr,
                                            &eventsFullInfoArr[i].errorCounter, &eventsFullInfoArr[i].failedRow,
                                            &eventsFullInfoArr[i].failedSegment, &eventsFullInfoArr[i].failedSyndrome);
            if(GT_OK != status)
            {
                break;
            }
            if (eventsFullInfoArr[i].failedSegment == 0xFFFFFFFF)
            {

            }
            eventsFullInfoArr[i].eventInfo.location.ramEntryInfo.memLocation.dfxPipeId   = memEntry.info.ramEntryInfo.memLocation.dfxPipeId;
            eventsFullInfoArr[i].eventInfo.location.ramEntryInfo.memLocation.dfxClientId = memEntry.info.ramEntryInfo.memLocation.dfxClientId;
            eventsFullInfoArr[i].eventInfo.location.ramEntryInfo.memLocation.dfxMemoryId = memEntry.info.ramEntryInfo.memLocation.dfxMemoryId;
            eventsFullInfoArr[i].eventInfo.location.ramEntryInfo.memType = currMemType;
            eventsFullInfoArr[i].eventInfo.location.ramEntryInfo.ramRow = eventsFullInfoArr[i].failedRow;
            /* Fill HW, Logical coordinates */
            status = prvCpssPxDiagDataIntegrityFillHwLogicalFromRam(
                devNum, &eventsFullInfoArr[i].eventInfo.location);
            if(GT_OK != status)
            {
                break;
            }

            prv_c_to_lua_CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_FULL_INFO_STC(L, &(eventsFullInfoArr[i]));
            lua_rawseti(L, -2, ++idx);
        }
    }

    lua_pushinteger(L, status);
    lua_replace(L, -3);
    return 2;
}
/*******************************************************************************
* wrlCpssPxDiagDataIntegrityStatusGet
*
* DESCRIPTION:
*       returns the Data Integrity ErrorCountEnable EventMask  ErrorInjection and ProtectionType        .
*
* APPLICABLE DEVICES: PIPE
*
*
* NOT APPLICABLE DEVICES: None
*
* INPUTS:
*       L                     - lua state
*
* OUTPUTS:
*
* RETURNS:
*       status, configuration parameters
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssPxDiagDataIntegrityConfigStatusGet
(
    IN lua_State *L
)
{
    GT_SW_DEV_NUM                                  devNum;
    CPSS_PX_LOGICAL_TABLE_ENT                      table;
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC             memEntry;
    CPSS_PX_DIAG_DATA_INTEGRITY_CONFIG_STATUS_STC  configStatus;

    GT_STATUS   status = GT_OK;

    PARAM_NUMBER(status, devNum, 1, GT_SW_DEV_NUM);
    QUIT_IF_NOT_OK(status);
    PARAM_ENUM(status, table, 2, CPSS_PX_LOGICAL_TABLE_ENT);
    QUIT_IF_NOT_OK(status);

    lua_newtable(L);
    memEntry.type = CPSS_PX_LOCATION_LOGICAL_INDEX_INFO_TYPE;
    memEntry.info.logicalEntryInfo.logicalTableType = table;

    configStatus.table = table;

    status = cpssPxDiagDataIntegrityErrorCountEnableGet(devNum, &memEntry, &configStatus.errorType, &configStatus.errorCountEnable);
    QUIT_IF_NOT_OK(status);

    status = cpssPxDiagDataIntegrityEventMaskGet(devNum, &memEntry, &configStatus.errorType, &configStatus.eventMask);
    QUIT_IF_NOT_OK(status);

    status = cpssPxDiagDataIntegrityErrorInjectionConfigGet(devNum, &memEntry, &configStatus.injectMode, &configStatus.injectEnable);
    QUIT_IF_NOT_OK(status);

    status = cpssPxDiagDataIntegrityProtectionTypeGet(devNum, &memEntry, &configStatus.protectionType);
    QUIT_IF_NOT_OK(status);

    prv_c_to_lua_CPSS_PX_DIAG_DATA_INTEGRITY_CONFIG_STATUS_STC(L,&configStatus);

    lua_pushinteger(L, status);
    lua_replace(L, -3);

    return 2;
}

/*******************************************************************************
* wrlCpssPxDiagDataIntegrityShadowSizeGet
*
* DESCRIPTION:
*       returns the Data Integrity shadow tables size        .
*
* APPLICABLE DEVICES: PIPE
*
*
* NOT APPLICABLE DEVICES: None
*
* INPUTS:
*       L                     - lua state
*
* OUTPUTS:
*
* RETURNS:
*       status,tables size
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssPxDiagDataIntegrityShadowSizeGet
(
    IN lua_State *L
)
{
    GT_SW_DEV_NUM                                  devNum;
    CPSS_PX_LOGICAL_TABLE_ENT                      table;
    CPSS_PX_LOGICAL_SHADOW_TABLES_INFO_STC         tablesInfo;
    CPSS_PX_LOGICAL_TABLE_SHADOW_INFO_STC          logArr[1];
    CPSS_PX_DIAG_DATA_INTEGRITY_SHADOW_SIZE_STC    shadowInfo;

    GT_STATUS   status = GT_OK;

    PARAM_NUMBER(status, devNum, 1, GT_SW_DEV_NUM);
    QUIT_IF_NOT_OK(status);
    PARAM_ENUM(status, table, 2, CPSS_PX_LOGICAL_TABLE_ENT);
    QUIT_IF_NOT_OK(status);

    lua_newtable(L);
    tablesInfo.numOfDataIntegrityElements = 1;
    tablesInfo.logicalTablesArr = logArr;
    tablesInfo.logicalTablesArr[0].logicalTableName = table;
    tablesInfo.logicalTablesArr[0].shadowType       = CPSS_PX_SHADOW_TYPE_CPSS_E;
    status =  cpssPxDiagDataIntegrityShadowTableSizeGet(devNum, &tablesInfo, &shadowInfo.tableSize);
    QUIT_IF_NOT_OK(status);
    shadowInfo.table = table;
    prv_c_to_lua_CPSS_PX_DIAG_DATA_INTEGRITY_SHADOW_SIZE_STC(L,&shadowInfo);

    lua_pushinteger(L, status);
    lua_replace(L, -3);

    return 2;

}
