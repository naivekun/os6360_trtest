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
* @file prvCpssLog.h
*
* @brief Includes definitions for CPSS log functions.
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssLogh
#define __prvCpssLogh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/extServices/os/gtOs/gtGenTypes.h>
#include <stdarg.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

/**
* @enum PRV_CPSS_LOG_PARAM_DIRECTION_ENT
 *
 * @brief This enum describes API parameter direction.
*/
typedef enum{

    /** input API/function parameter */
    PRV_CPSS_LOG_PARAM_IN_E,

    /** output API/function parameter */
    PRV_CPSS_LOG_PARAM_OUT_E,

    /** input/output API/function parameter */
    PRV_CPSS_LOG_PARAM_INOUT_E

} PRV_CPSS_LOG_PARAM_DIRECTION_ENT;

/* size of key array */
#define PRV_CPSS_LOG_PARAM_KEY_ARR_SIZE_STC 8

/**
* @struct PRV_CPSS_LOG_PARAM_KEY_INFO_STC
 *
 * @brief This structure describes key for parameters parsing
*/
typedef struct{

    GT_UINTPTR paramKeyArr[PRV_CPSS_LOG_PARAM_KEY_ARR_SIZE_STC];

} PRV_CPSS_LOG_PARAM_KEY_INFO_STC;
/**
* @struct PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC
 *
 * @brief This structure describes input or output parameter info which may be
 * used in API parameter logging logic.
*/
typedef struct{

    /** @brief parameter key value which may be referenced in current API parameter logging logic
     *  formatPtr -
     *  (pointer to) parameter output format string
     */
    PRV_CPSS_LOG_PARAM_KEY_INFO_STC paramKey;

    GT_CHAR * formatPtr;

    /** value of parameter. used for parameters those are pointers. */
    GT_VOID_PTR paramValue;

} PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC;

/* unique IDs of Log libs in common directory */
#define CPSS_LOG_LIB_CONFIG_COMMON_E          (CPSS_LOG_LIB_ALL_E)
#define CPSS_LOG_LIB_SYSTEM_RECOVERY_COMMON_E (CPSS_LOG_LIB_ALL_E + 1)
#define CPSS_LOG_LIB_HW_INIT_COMMON_E         (CPSS_LOG_LIB_ALL_E + 2)

/* Last LIB in Log engine */
#define CPSS_LOG_LIB_LAST_CNS (CPSS_LOG_LIB_HW_INIT_COMMON_E + 1)

/**
* @internal prvCpssLogStcLogStart function
* @endinternal
*
* @brief   The function starts log structure
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] stcName                  - structure field name.
*                                       None.
*/
void prvCpssLogStcLogStart
(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 stcName
);

/**
* @internal prvCpssLogStcLogEnd function
* @endinternal
*
* @brief   The function ends log structure
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
*                                       None.
*/
void prvCpssLogStcLogEnd
(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType
);

/**
* @internal prvCpssLogPacket function
* @endinternal
*
* @brief   The function logs packet as array of bytes
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] packetSize               -
*                                      size of packet (in bytes)
* @param[in] packetPtr                -
*                                      a pointer to the first byte of the packet
*                                       None.
*/
void prvCpssLogPacket
(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_U32                      packetSize,
    IN GT_U8                       *packetPtr
);

/**
* @internal prvCpssLogEnum function
* @endinternal
*
* @brief   The function to log simple enum parameter
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] fieldName                - parameter/structure field name.
* @param[in] enumArray[]              - array of emumerator strings.
* @param[in] enumArrayIndex           - log string access index in enum array.
* @param[in] enumArrayEntries         - number of enum array entries.
*                                       None.
*/
void prvCpssLogEnum
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              fieldName,
    IN char                   * enumArray[],
    IN GT_U32                   enumArrayIndex,
    IN GT_U32                   enumArrayEntries
);

/**
* @internal prvCpssLogEnumMap function
* @endinternal
*
* @brief   The function to log mapped enum parameter
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] fieldName                - parameter/structure field name.
* @param[in] keyValue                 - lookup enum value
* @param[in] enumArray                - array of emumerator strings.
* @param[in] enumArrayEntries         - number of enum array entries.
* @param[in] enumArrayEntrySize       - array entry size
*                                       None.
*/
void prvCpssLogEnumMap
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              fieldName,
    IN GT_U32                   keyValue,
    IN void                   * enumArray,
    IN GT_U32                   enumArrayEntries,
    IN GT_U32                   enumArrayEntrySize
);

/**
* @internal prvCpssEnumStringValueGet function
* @endinternal
*
* @brief   The function maps enum parameter to string value
*
* @param[in] fieldName                - parameter/structure field name.
* @param[in] keyValue                 - lookup enum value
* @param[in] enumArray                - array of emumerator strings.
* @param[in] enumArrayEntries         - number of enum array entries.
* @param[in] enumArrayEntrySize       - array entry size
*
* @param[out] keyStringPtr             - (pointer to) key value mapped string
*                                       None.
*/
void prvCpssEnumStringValueGet
(
    IN GT_CHAR_PTR              fieldName,
    IN GT_U32                   keyValue,
    IN void                   * enumArray,
    IN GT_U32                   enumArrayEntries,
    IN GT_U32                   enumArrayEntrySize,
    OUT GT_U8                 * keyStringPtr
);

/**
* @internal prvCpssLogStcUintptr function
* @endinternal
*
* @brief   The function to log GT_UINTPTR parameter
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] fieldName                - structure field name
* @param[in] numValue                 - field value.
*                                       None.
*/
void prvCpssLogStcUintptr
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              fieldName,
    IN GT_UINTPTR               numValue
);

/**
* @internal prvCpssLogStcBool function
* @endinternal
*
* @brief   The function logs GT_BOOL structure field
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] fieldName                - structure field name
* @param[in] bValue                   - field value.
*                                       None.
*/
void prvCpssLogStcBool
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              fieldName,
    IN GT_BOOL                  bValue
);

/**
* @internal prvCpssLogStcNumber function
* @endinternal
*
* @brief   The function to log numeric parameter
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] fieldName                - structure field name
* @param[in] numValue                 - field value.
*                                       None.
*/
void prvCpssLogStcNumber
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              fieldName,
    IN GT_U32                   numValue
);

/**
* @internal prvCpssLogStcByte function
* @endinternal
*
* @brief   The function to log byte parameter
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] fieldName                - structure field name
*                                      numValue - field value.
*                                       None.
*/
void prvCpssLogStcByte
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              fieldName,
    IN GT_U8                    byteValue
);

/**
* @internal prvCpssLogParamFuncStc_GT_FLOAT32_PTR function
* @endinternal
*
* @brief   The function to log GT_FLOAT32 parameter by pointer
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] fieldName                - structure field name
* @param[in] fieldPtr                 - (pointer to) field value
*                                       None.
*/
void prvCpssLogParamFuncStc_GT_FLOAT32_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              fieldName,
    IN GT_VOID                * fieldPtr
);

/**
* @internal prvCpssLogStcFloat function
* @endinternal
*
* @brief   The function to log float parameter
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] fieldName                - structure field name
* @param[in] numValue                 - field value.
*                                       None.
*/
void prvCpssLogStcFloat
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              fieldName,
    IN float                    numValue
);


/**
* @internal prvCpssLogStcPointer function
* @endinternal
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] fieldName                - structure field name
* @param[in] ptrValue                 - (pointer to) field value.
*                                       None.
*/
void prvCpssLogStcPointer
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              fieldName,
    IN void *                   ptrValue
);

/**
* @internal prvCpssLogStcMac function
* @endinternal
*
* @brief   Function to log mac address parameter
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] fieldName                - structure field name
* @param[in] macPtr                   - pointer to ethernet address.
*                                       None.
*/
void prvCpssLogStcMac
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              fieldName,
    IN GT_U8                  * macPtr
);

/**
* @internal prvCpssLogStcIpV4 function
* @endinternal
*
* @brief   Function to log IPV4 address parameter
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] fieldName                - structure field name
* @param[in] ipAddrPtr                - pointer to IPV4 address.
*                                       None.
*/
void prvCpssLogStcIpV4
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              fieldName,
    IN GT_U8                  * ipAddrPtr
);

/**
* @internal prvCpssLogStcIpV6 function
* @endinternal
*
* @brief   Function to log IPV6 address parameter
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] fieldName                - structure field name
* @param[in] ipV6Ptr                  - pointer to IPV6 address.
*                                       None.
*/
void prvCpssLogStcIpV6
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              fieldName,
    IN GT_U8                  * ipV6Ptr
);

/**
* @internal prvCpssLogParamDataCheck function
* @endinternal
*
* @brief   Parameters validation.
*
* @param[in] skipLog                  - skip log flag.
* @param[in] inOutParamInfoPtr        - pointer to parameter log data.
*                                       None.
*/
GT_BOOL prvCpssLogParamDataCheck
(
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC  * inOutParamInfoPtr
);

/* Macros */

/* method to avoid compiler warning about unused variable */
#define PRV_CPSS_LOG_UNUSED_MAC(x) (void)x

#define PRV_CPSS_LOG_FUNC_TYPE_MAC(type_name) \
    prvCpssLogParamFunc_ ## type_name

/* Parameter(pointer) log functons */
#define PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(type_name) \
    prvCpssLogParamFunc_ ## type_name ## _PTR

/* Parameter(pointer to pointer) log functons */
#define PRV_CPSS_LOG_FUNC_TYPE_PTR_PTR_MAC(type_name) \
    prvCpssLogParamFunc_ ## type_name ## _PTR_PTR

/* Parameter(array of pointers) log functons */
#define PRV_CPSS_LOG_FUNC_TYPE_PTR_ARRAY_MAC(type_name) \
    prvCpssLogParamFunc_ ## type_name ## _PTR_ARRAY

/* Parameter(array of primitives) log functons */
#define PRV_CPSS_LOG_FUNC_TYPE_ARRAY_MAC(type_name) \
    prvCpssLogParamFunc_ ## type_name ## _ARRAY

/* Strurcture pointer log functons */
#define PRV_CPSS_LOG_FUNC_STC_TYPE_PTR_MAC(type_name) \
    prvCpssLogParamFuncStc_ ## type_name ## _PTR

/* Log array in structures */
#define PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(val_ptr, field_name, elem_num, field_type) \
    prvCpssLogParamFuncStcTypeArray(contextLib, logType, #field_name, (void *)val_ptr->field_name, elem_num, sizeof(field_type), PRV_CPSS_LOG_FUNC_STC_TYPE_PTR_MAC(field_type), inOutParamInfoPtr)

/* Log array of parameters */
#define PRV_CPSS_LOG_ARRAY_PARAMS_MAC(val_ptr, arr_name, elem_num, elem_type) \
    prvCpssLogParamFuncStcTypeArray(contextLib, logType, arr_name, (void *)val_ptr, elem_num, sizeof(elem_type), PRV_CPSS_LOG_FUNC_STC_TYPE_PTR_MAC(elem_type), inOutParamInfoPtr)

/* Log 2-dimentional array field in structures */
#define PRV_CPSS_LOG_STC_TYPE_ARRAY2_MAC(val_ptr, field_name, elem_num, elem_num2, field_type) \
    prvCpssLogParamFuncStcTypeArray2(contextLib, logType, #field_name, (void *)val_ptr->field_name, elem_num, elem_num2, sizeof(field_type), PRV_CPSS_LOG_FUNC_STC_TYPE_PTR_MAC(field_type), inOutParamInfoPtr)

/* 'Before' parameter function logic */
#define PRV_CPSS_LOG_FUNC_PRE_LOGIC_MAC(type_name) \
    prvCpssLogPreParamFunc_ ## type_name

/* 'After' parameter function logic */
#define PRV_CPSS_LOG_FUNC_POST_LOGIC_MAC(type_name) \
    prvCpssLogPostParamFunc_ ## type_name

/* Set parameter value to be used in parameter log function */
#define PRV_CPSS_LOG_SET_PARAM_VAL_MAC(param_type, paramVal) \
    param_type paramVal;\
    if (argsPtr == NULL) \
    {\
        cpssOsMemCpy(&paramVal, &(inOutParamInfoPtr->paramValue), sizeof(paramVal)); \
    }\
    else {\
        paramVal = va_arg(*argsPtr, param_type); \
        cpssOsMemCpy(&(inOutParamInfoPtr->paramValue), &paramVal, sizeof(inOutParamInfoPtr->paramValue)); \
    }\
    if (GT_FALSE == prvCpssLogParamDataCheck(skipLog, inOutParamInfoPtr)) \
        return

/* Set parameter field value to be used in parameter log function */
#define PRV_CPSS_LOG_SET_FIELD_VAL_PTR_MAC(field_type, valPtr) \
    field_type valPtr = (field_type) fieldPtr; \
    if (valPtr == NULL) {\
        prvCpssLogTabs(contextLib, logType);\
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr); \
        return;\
    }

/* The macro must be called before field log structure */
#define PRV_CPSS_LOG_START_FIELD_STC_MAC(stc_name, valPtr) \
    PRV_CPSS_LOG_SET_FIELD_VAL_PTR_MAC(stc_name, valPtr);  \
    prvCpssLogStcLogStart(contextLib, logType, namePtr)

/* The macro must be called before parameter log structure */
#define PRV_CPSS_LOG_START_PARAM_STC_MAC(stc_name, valPtr) \
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(stc_name, valPtr);  \
    if (valPtr == NULL) {\
        prvCpssLogTabs(contextLib, logType);\
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr); \
        return;\
    }

/* The macro for call the array of enums handler */
#define PRV_CPSS_LOG_ENUM_ARRAY_MAC(contextLib, logType, arr_name, paramVal, enum_type, param_info) \
    prvCpssLogArrayOfEnumsHandle(contextLib, logType, arr_name, paramVal, sizeof(enum_type), prvCpssLogEnum_##enum_type, prvCpssLogEnum_size_##enum_type, param_info)

/* The macro to log simple enum parameter */
#define PRV_CPSS_LOG_ENUM_MAC(fieldName, paramVal, fieldtype) \
    prvCpssLogEnum(contextLib, logType, fieldName, prvCpssLogEnum_##fieldtype, paramVal, prvCpssLogEnum_size_##fieldtype)

/* The macro to log simple enum field structure */
#define PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, field, fieldtype) \
    prvCpssLogEnum(contextLib, logType, #field, prvCpssLogEnum_##fieldtype, valPtr->field, prvCpssLogEnum_size_##fieldtype)

/*  Macro to map enum key value to string */
#define PRV_CPSS_LOG_ENUM_STRING_GET_MAC(param, param_type, param_str) \
    prvCpssEnumStringValueGet(#param, param, (void *)prvCpssLogEnum_map_##param_type, prvCpssLogEnum_size_##param_type, sizeof(prvCpssLogEnum_map_##param_type[0]), param_str);

/*  Macro to log enum parameter using binary search */
#define PRV_CPSS_LOG_VAR_ENUM_MAP_MAC(param, param_type) \
    prvCpssLogEnumMap(contextLib, logType, #param, param, (void *)prvCpssLogEnum_map_##param_type, prvCpssLogEnum_size_##param_type, sizeof(prvCpssLogEnum_map_##param_type[0]));

/*  Macro to log enum parameter using binary search */
#define PRV_CPSS_LOG_ENUM_MAP_MAC(paramName, param, param_type) \
    prvCpssLogEnumMap(contextLib, logType, paramName, param, (void *)prvCpssLogEnum_map_##param_type, prvCpssLogEnum_size_##param_type, sizeof(prvCpssLogEnum_map_##param_type[0]));

/*  Macro to log enum field using binary search */
#define PRV_CPSS_LOG_STC_ENUM_MAP_MAC(valPtr, field, fieldtype) \
    prvCpssLogEnumMap(contextLib, logType, #field, valPtr->field, (void *)prvCpssLogEnum_map_##fieldtype, prvCpssLogEnum_size_##fieldtype, sizeof(prvCpssLogEnum_map_##fieldtype[0]));

#define PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(enum_type) \
extern char * prvCpssLogEnum_##enum_type[]; \
extern GT_U32 prvCpssLogEnum_size_##enum_type

#define PRV_CPSS_LOG_STC_ENUM_MAP_DECLARE_MAC(enum_type) \
extern PRV_CPSS_ENUM_STRING_VALUE_PAIR_STC prvCpssLogEnum_map_##enum_type[]; \
extern GT_U32 prvCpssLogEnum_size_##enum_type

#define PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(enum_type) \
GT_U32 prvCpssLogEnum_size_##enum_type = sizeof(prvCpssLogEnum_##enum_type)/sizeof(prvCpssLogEnum_##enum_type[0])

#define PRV_CPSS_LOG_STC_ENUM_MAP_ARRAY_SIZE_MAC(enum_type) \
GT_U32 prvCpssLogEnum_size_##enum_type = sizeof(prvCpssLogEnum_map_##enum_type)/sizeof(prvCpssLogEnum_map_##enum_type[0])

/* Log pointer field (as address in hex) */
#define PRV_CPSS_LOG_STC_PTR_MAC(valPtr, field)          \
    prvCpssLogStcPointer(contextLib, logType, #field, (void *)(GT_UINTPTR)valPtr->field);

/* Log boolean field */
#define PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, field) \
    prvCpssLogStcBool(contextLib, logType, #field, valPtr->field);

/* Log numeric field */
#define PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, field) \
    prvCpssLogStcNumber(contextLib, logType, #field, valPtr->field);

/* Log 16 bit hexadecimal field */
#define PRV_CPSS_LOG_STC_16_HEX_MAC(valPtr, field) \
    prvCpssLogStc16HexNumber(contextLib, logType, #field, valPtr->field);

/* Log 32 bit hexadecimal field */
#define PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, field) \
    prvCpssLogStc32HexNumber(contextLib, logType, #field, valPtr->field);

/* Log 64 bit hexadecimal field */
#define PRV_CPSS_LOG_STC_64_HEX_MAC(valPtr, field) \
    prvCpssLogU64Number(contextLib, logType, #field, valPtr->field, GT_TRUE);

/* Log UINTPTR field */
#define PRV_CPSS_LOG_STC_UINTPTR_MAC(valPtr, field) \
    prvCpssLogStcUintptr(contextLib, logType, #field, valPtr->field);

/* Log byte field */
#define PRV_CPSS_LOG_STC_BYTE_MAC(valPtr, field) \
    prvCpssLogStcByte(contextLib, logType, #field, valPtr->field);

/* Log structure field */
#define PRV_CPSS_LOG_STC_STC_MAC(valPtr, field, fieldtype) {\
    PRV_CPSS_LOG_STC_FUNC logFunc = PRV_CPSS_LOG_FUNC_STC_TYPE_PTR_MAC(fieldtype); \
    logFunc(contextLib, logType, #field, &valPtr->field, inOutParamInfoPtr);       \
  }

/* Log structure pointer field */
#define PRV_CPSS_LOG_STC_STC_PTR_MAC(valPtr, field, fieldtype) {\
    PRV_CPSS_LOG_STC_FUNC logFunc = PRV_CPSS_LOG_FUNC_STC_TYPE_PTR_MAC(fieldtype); \
    logFunc(contextLib, logType, #field, valPtr->field, inOutParamInfoPtr);        \
  }

/* Structure field value */
#define PRV_CPSS_LOG_STC_FIELD_VAL_MAC(valPtr, param_field) \
    valPtr->param_field

/* Log API parameters and save history in database */
#define PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(log_lib, log_type, ...) \
    prvCpssLogAndHistoryParam(log_lib, log_type, __VA_ARGS__)


/* Set a format string for logging pointer values */
#define PRV_CPSS_LOG_PTR_FORMAT_MAC(formatStr)                                   \
    formatStr = (prvCpssLogPointerFormat == CPSS_LOG_POINTER_FORMAT_PREFIX_E) ? \
            "%s = (addr)%p\n" :  "%s = %p\n";


/* CPSS LOG API output Format */
extern CPSS_LOG_API_FORMAT_ENT prvCpssLogFormat;
/* CPSS LOG TAB Index */
extern GT_U32 prvCpssLogTabIndex;

/* CPSS LOG pointer output Format */
extern CPSS_LOG_POINTER_FORMAT_ENT prvCpssLogPointerFormat;

/* Check zero value log condition */
#define PRV_CPSS_LOG_ZERO_VALUE_LOG_CHECK_MAC(field) \
    if (field == 0 && (prvCpssLogFormat == CPSS_LOG_API_FORMAT_NON_ZERO_PARAMS_E) && (prvCpssLogTabIndex != 0)) \
        return;

/* Check zero array log condition */
#define PRV_CPSS_LOG_ZERO_ARRAY_LOG_CHECK_MAC(arr, size) \
    {                                                    \
        GT_U32 i, val = 0;                               \
        for (i = 0; i < size; i++){                      \
            if (arr[i]) {                                \
                val = 1; break;                          \
            }                                            \
        }                                                \
        PRV_CPSS_LOG_ZERO_VALUE_LOG_CHECK_MAC(val);      \
    }

/* Check zero 2-dimentional array log condition */
#define PRV_CPSS_LOG_ZERO_ARRAY2_LOG_CHECK_MAC(arr, size_i, size_j)   \
    {                                                       \
        GT_U32 i, j, val = 0;                               \
        for (i = 0; i < size_i; i++){                       \
            for (j = 0; j < size_j; j++){                   \
                if (arr[i][j]) {                            \
                    val = 1; break;                         \
                }                                           \
            }                                               \
        }                                                   \
        PRV_CPSS_LOG_ZERO_VALUE_LOG_CHECK_MAC(val);         \
    }

/* String constants */
extern const GT_CHAR *prvCpssLogErrorMsgDeviceNotExist;
extern const GT_CHAR *prvCpssLogErrorMsgFdbIndexOutOfRange;
extern const GT_CHAR *prvCpssLogErrorMsgExactMatchIndexOutOfRange;
extern const GT_CHAR *prvCpssLogErrorMsgPortGroupNotValid;
extern const GT_CHAR *prvCpssLogErrorMsgPortGroupNotActive;
extern const GT_CHAR *prvCpssLogErrorMsgIteratorNotValid;
extern const GT_CHAR *prvCpssLogErrorMsgGlobalIteratorNotValid;
extern const GT_CHAR *prvCpssLogErrorMsgPortGroupBitmapNotValid;
extern const GT_CHAR *prvCpssLogErrorMsgDeviceNotInitialized;

/**
* @internal prvCpssLogTabs function
* @endinternal
*
* @brief   Log tab for recursive structures
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
*                                       None.
*/
void prvCpssLogTabs
(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType
);

/*******************************************************************************
* PRV_CPSS_LOG_PARAM_FUNC
*
* DESCRIPTION:
*       Log API parameter function
*
* INPUTS:
*       contextLib -
*           lib log activity
*       logType -
*           type of the log
*       namePtr -
*           pointer to parameter name
*       argsPtr -
*           pointer to parameter argiment list or NULL
*       fieldPtr -
*           pointer to field in structure or NULL
*       skipLog -
*           skip parameter/field logging
*       inOutParamInfoPtr -
*           pointer to additional parameter data
*
* OUTPUTS:
*       inOutParamInfoPtr -
*           pointer to additional parameter data
*
* RETURNS:
*       None
*
* COMMENTS:
*       NONE
*
*******************************************************************************/
typedef GT_VOID  (*PRV_CPSS_LOG_PARAM_FUNC)
(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);

/*******************************************************************************
* PRV_CPSS_LOG_STC_FUNC
*
* DESCRIPTION:
*       Log structure function
*
* INPUTS:
*       contextLib -
*           lib log activity
*       logType -
*           type of the log
*       namePtr -
*           pointer to structure field name
*       fieldPtr -
*           pointer to field in structure
*       inOutParamInfoPtr -
*           pointer to additional parameter data
*
* OUTPUTS:
*       None
* RETURNS:
*       None
*
* COMMENTS:
*       NONE
*
*******************************************************************************/
typedef GT_VOID  (*PRV_CPSS_LOG_STC_FUNC)
(
  IN    CPSS_LOG_LIB_ENT                        contextLib,
  IN    CPSS_LOG_TYPE_ENT                       logType,
  IN    GT_CHAR_PTR                             namePtr,
  IN    void                                  * fieldPtr,
  INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);

/**
* @struct PRV_CPSS_LOG_FUNC_PARAM_STC
 *
 * @brief This structure describes API parameter info.
*/
typedef struct{

    /** pointer to parameter name */
    GT_CHAR_PTR paramNamePtr;

    /** API parameter direction. */
    PRV_CPSS_LOG_PARAM_DIRECTION_ENT direction;

    /** parameter log function */
    PRV_CPSS_LOG_PARAM_FUNC logFunc;

} PRV_CPSS_LOG_FUNC_PARAM_STC;

/*******************************************************************************
* PRV_CPSS_LOG_PRE_LOG_HANDLER_FUN
*
* DESCRIPTION:
*       Fucntion called before API log.
*
* INPUTS:
*       args -
*           log API variable arguments
*
* OUTPUTS:
*       paramDataPtr -
*           pointer to API parameters common data
*
* RETURNS:
*       None
*
* COMMENTS:
*       NONE
*
*******************************************************************************/
typedef GT_VOID  (*PRV_CPSS_LOG_PRE_LOG_HANDLER_FUN)
(
    IN va_list  args,
    OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr
);

/**
* @struct PRV_CPSS_LOG_FUNC_ENTRY_STC
 *
 * @brief This structure describes database parameter entry.
*/
typedef struct{

    /** pointer to API name */
    GT_CHAR_PTR apiName;

    /** @brief API parameters number
     *  paramsPtrPtr -
     *  (pointer to) pointer to API parameter info.
     */
    GT_U32 numOfParams;

    PRV_CPSS_LOG_FUNC_PARAM_STC **paramsPtrPtr;

    /** function may be called before API log function */
    PRV_CPSS_LOG_PRE_LOG_HANDLER_FUN prvCpssLogPreLogFunc;

} PRV_CPSS_LOG_FUNC_ENTRY_STC;

/*******************************************************************************
* PRV_CPSS_LOG_PARAM_LIB_DB_GET_FUNC
*
* DESCRIPTION:
*       Get API log functions database from specific lib
*
* INPUTS:
*       None
*
* OUTPUTS:
*       logFuncDbPtr -
*           (pointer to) pointer to LIB API log function database
*       logFuncDbSizePtr -
*           pointer to LIB API log function database size
*
* RETURNS:
*       None
*
* COMMENTS:
*       NONE
*
*******************************************************************************/
typedef GT_VOID  (*PRV_CPSS_LOG_PARAM_LIB_DB_GET_FUNC)
(
    OUT PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtr,
    OUT GT_U32 * logFuncDbSizePtr
);

/**
* @struct PRV_CPSS_ENUM_STRING_VALUE_PAIR_STC
 *
 * @brief Enumerator key pair
*/
typedef struct{

    /** enum name */
    GT_CHAR_PTR namePtr;

    /** enumerator value */
    GT_32 enumValue;

} PRV_CPSS_ENUM_STRING_VALUE_PAIR_STC;

/* the status of the CPSS LOG - enable/disable */
extern GT_BOOL prvCpssLogEnabled;
#ifdef  WIN32
#define __FILENAME__ (cpssOsStrRChr(__FILE__, '\\') ? cpssOsStrRChr(__FILE__, '\\') + 1 : __FILE__)
#define __FUNCNAME__ __FUNCTION__
#else
#define __FILENAME__ (cpssOsStrRChr(__FILE__, '/') ? cpssOsStrRChr(__FILE__, '/') + 1 : __FILE__)
#define __FUNCNAME__ __func__
#endif
/* empty log error string */
#define LOG_ERROR_NO_MSG    ""

/* Log numeric format */
extern const GT_CHAR *prvCpssLogFormatNumFormat;
/* format parameter value by its name */
#define LOG_ERROR_ONE_PARAM_FORMAT_MAC(_param)      prvCpssLogFormatNumFormat, #_param, _param

#ifdef CPSS_LOG_ENABLE

/* macro to convert hws log arguments to cpss log */
#define HWS_TO_CPSS_LOG_ERROR_AND_RETURN_MAC(funcName, fileName, lineNum, rc, strMsg) \
    do { \
        if (prvCpssLogEnabled == GT_TRUE) prvCpssLogError(funcName, fileName, lineNum, rc, strMsg); \
        return rc; \
    } while (0)

/* macro to convert hws log arguments to cpss log */
#define HWS_TO_CPSS_LOG_INFORMATION_MAC(funcName, fileName, lineNum, strMsg) \
    if (prvCpssLogEnabled == GT_TRUE) prvCpssLogInformation(funcName, fileName, lineNum, strMsg);

/* macro to log function error by formatted string */
#define CPSS_LOG_ERROR_AND_RETURN_MAC(_rc, ...) \
    do { \
        if (prvCpssLogEnabled == GT_TRUE) prvCpssLogError(__FUNCNAME__, __FILENAME__, __LINE__, _rc, __VA_ARGS__); \
        return _rc; \
    } while (0)
#define CPSS_LOG_ERROR_MAC(...) \
    do { \
        if (prvCpssLogEnabled == GT_TRUE) prvCpssLogError( \
        __FUNCNAME__, __FILENAME__, __LINE__, 0xFFFF/*_rc*/, __VA_ARGS__); \
    } while (0)

/* macro to log function information by formatted string */
#define CPSS_LOG_INFORMATION_MAC(...) \
    if (prvCpssLogEnabled == GT_TRUE) prvCpssLogInformation(__FUNCNAME__, __FILENAME__, __LINE__, __VA_ARGS__);

#else
#ifdef PX_FAMILY
    /*patch for PX devices that need ERROR LOG ... although the 'LOG info' not exists */
    #define CPSS_LOG_ERROR_AND_RETURN_MAC(_rc, ...) \
        do { \
            if (prvCpssLogEnabled == GT_TRUE) prvCpssLogError(__FUNCNAME__, __FILENAME__, __LINE__, _rc, __VA_ARGS__); \
            return _rc; \
        } while (0)
    #define CPSS_LOG_ERROR_MAC(...) \
        do { \
            if (prvCpssLogEnabled == GT_TRUE) prvCpssLogError( \
            __FUNCNAME__, __FILENAME__, __LINE__, 0xFFFF/*_rc*/, __VA_ARGS__); \
        } while (0)
#else /*PX_FAMILY*/
    #define CPSS_LOG_ERROR_AND_RETURN_MAC(_rc, ...) return _rc
    #define CPSS_LOG_ERROR_MAC(...)
#endif /*PX_FAMILY*/


#define CPSS_LOG_INFORMATION_MAC(...)
#define HWS_TO_CPSS_LOG_ERROR_AND_RETURN_MAC(funcName, fileName, lineNum, rc, strMsg) return rc
#define HWS_TO_CPSS_LOG_INFORMATION_MAC(funcName, fileName, lineNum, strMsg)
#endif

/* macro to log info parameter : it's name and value */
#define CPSS_LOG_INFO_PARAM_MAC(param)  CPSS_LOG_INFORMATION_MAC("%s = [0x%x] \n", #param , param)

/* Get the name (string) and value of field */
#define PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(field)   {#field , field}

/* macro to log unknown ENUM parameter , and return GT_BAD_PARAM */
#define CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(enumParam)                     \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ENUM [%s] not supports value [%d]",  \
            #enumParam,                                                      \
            enumParam)


/**
* @internal prvCpssLogApiEnter function
* @endinternal
*
* @brief   Internal API for API/Internal/Driver function call log - enter function.
*
* @param[in] functionId               - function identifier (of the function we just entered into).
*                                      ... - the parameters values of the function we just entered into.
*                                       None.
*/
void prvCpssLogApiEnter
(
    IN GT_U32 functionId,
    IN ...
);

/**
* @internal prvCpssLogApiExit function
* @endinternal
*
* @brief   Internal API for API/Internal function call log - exit function.
*
* @param[in] functionId               - function identifier (of the function we just exit from).
* @param[in] rc                       - the return code of the function.
*                                       None.
*/
void prvCpssLogApiExit
(
    IN GT_U32 functionId,
    IN GT_STATUS rc
);

/**
* @internal prvCpssLogError function
* @endinternal
*
* @brief   Internal API for API/Internal function error log
*
* @param[in] functionName             - name of function that generates error.
* @param[in] fileName                 - name of file that generates error.
* @param[in] line                     -  number in file, may be excluded from log by global configuration.
* @param[in] rc                       - the return code of the function.
* @param[in] formatPtr                - cpssOsLog format starting.
*                                      ...               optional parameters according to formatPtr
*                                       None.
*/
GT_VOID prvCpssLogError
(
    IN const char *functionName,
    IN const char *fileName,
    IN GT_U32 line,
    IN GT_STATUS rc,
    IN const char * formatPtr,
    ...
);

/**
* @internal prvCpssLogInformation function
* @endinternal
*
* @brief   Internal API for API/Internal function information log
*
* @param[in] functionName             - name of function that generates error.
* @param[in] fileName                 - name of file that generates error.
* @param[in] line                     -  number in file, may be excluded from log by global configuration.
* @param[in] formatPtr                - cpssOsLog format starting.
*                                      ...               optional parameters according to formatPtr
*                                       None.
*/
GT_VOID prvCpssLogInformation
(
    IN const char *functionName,
    IN const char *fileName,
    IN GT_U32 line,
    IN const char * formatPtr,
    ...
);

/**
* @internal prvCpssLogAndHistoryParam function
* @endinternal
*
* @brief   Log output and log history of function's parameter
*
* @param[in] lib                      - the function will print the log of the functions in "lib".
* @param[in] type                     - the function will print the logs from "type".
* @param[in] format                   - usual printf  string.
*                                      ...             - additional parameters.
*                                       None.
*/
GT_VOID prvCpssLogAndHistoryParam
(
    IN CPSS_LOG_LIB_ENT         lib,
    IN CPSS_LOG_TYPE_ENT        type,
    IN const char*              format,
    ...
);

/**
* @internal prvCpssLogParamFuncStcTypeArray function
* @endinternal
*
* @brief   Log array in structure
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] namePtr                  -
*                                      array name.
* @param[in] firstElementPtr          -
*                                      pointer to first array element
* @param[in] elementNum               -
*                                      number of array elements
* @param[in] elementSize              -
*                                      size of array element in bytes
* @param[in] logFunc                  -
*                                      pointer to specific type log function
*                                       None.
*/
GT_VOID prvCpssLogParamFuncStcTypeArray
(
    IN CPSS_LOG_LIB_ENT        contextLib,
    IN CPSS_LOG_TYPE_ENT       logType,
    IN GT_CHAR_PTR             namePtr,
    IN GT_VOID               * firstElementPtr,
    IN GT_U32                  elementNum,
    IN GT_U32                  elementSize,
    IN PRV_CPSS_LOG_STC_FUNC   logFunc,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);

/**
* @internal prvCpssLogParamFuncStcTypeArray2 function
* @endinternal
*
* @brief   Log two-dimentional array
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] namePtr                  -
*                                      array name.
* @param[in] firstElementPtr          -
*                                      pointer to first array element
* @param[in] rowNum                   -
*                                      number of array rows
* @param[in] rowNum                   -
*                                      number of array columns
* @param[in] elementSize              -
*                                      size of array element in bytes
* @param[in] logFunc                  -
*                                      pointer to specific type log function
*                                       None.
*/
GT_VOID prvCpssLogParamFuncStcTypeArray2
(
    IN CPSS_LOG_LIB_ENT        contextLib,
    IN CPSS_LOG_TYPE_ENT       logType,
    IN GT_CHAR_PTR             namePtr,
    IN GT_VOID               * firstElementPtr,
    IN GT_U32                  rowNum,
    IN GT_U32                  colNum,
    IN GT_U32                  elementSize,
    IN PRV_CPSS_LOG_STC_FUNC   logFunc,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);

/**
* @internal prvCpssLogArrayOfParamsHandle function
* @endinternal
*
* @brief   Log array of parameters (input and output arrays)
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] namePtr                  -
*                                      array name.
* @param[in] firstElementPtr          -
*                                      pointer to first array element
* @param[in] elementSize              -
*                                      size of array element in bytes
* @param[in] logFunc                  -
*                                      pointer to specific type log function
* @param[in] inOutParamInfoPtr        -
*                                      pointer to information about the array of parameters
* @param[in] inOutParamInfoPtr        - pointer to output array size
* @param[in] inOutParamInfoPtr        - the array max size
* @param[in] inOutParamInfoPtr        - size of an input array
* @param[in] inOutParamInfoPtr        - value of 1 is a sign that the output might be an array of strucutures
*                                       None.
*/
GT_VOID prvCpssLogArrayOfParamsHandle
(
    IN CPSS_LOG_LIB_ENT        contextLib,
    IN CPSS_LOG_TYPE_ENT       logType,
    IN GT_CHAR_PTR             namePtr,
    IN GT_VOID               * firstElementPtr,
    IN GT_U32                  elementSize,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr,
    IN PRV_CPSS_LOG_STC_FUNC   logFunc
);

/**
* @internal prvCpssLogArrayOfEnumsHandle function
* @endinternal
*
* @brief   Log array of parameters (input and output arrays)
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] namePtr                  -
*                                      array name
* @param[in] firstElementPtr          -
*                                      pointer to first enum in the array
* @param[in] enumSize                 -
*                                      size of the enum
* @param[in] enumArray[]              -
*                                      key value array of the enum values
* @param[in] enumArrayEntries         -
*                                      the size of the enum array
* @param[in] inOutParamInfoPtr        -
*                                      pointer to information about the array of parameters
* @param[in] inOutParamInfoPtr        - pointer to output array size
* @param[in] inOutParamInfoPtr        - the array max size
* @param[in] inOutParamInfoPtr        - size of an input array
* @param[in] inOutParamInfoPtr        - value of 1 is a sign that the output might be an array of strucutures
*                                       None.
*/
GT_VOID prvCpssLogArrayOfEnumsHandle
(
    IN CPSS_LOG_LIB_ENT        contextLib,
    IN CPSS_LOG_TYPE_ENT       logType,
    IN GT_CHAR_PTR             namePtr,
    IN GT_VOID               * firstElementPtr,
    IN GT_U32                  enumSize,
    IN char                  * enumArray[],
    IN GT_U32                  enumArrayEntries,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);

/**
* @internal prvCpssLogInit function
* @endinternal
*
* @brief   Init log related info
*/
GT_STATUS prvCpssLogInit
(
    GT_VOID
);

/**
* @internal prvCpssLogStateCheck function
* @endinternal
*
* @brief   Debug function to check log readiness before new API call
*
* @retval GT_OK                    - log is ready to run
* @retval GT_BAD_STATE             - log is state machine in bad state to run log
*/
GT_STATUS prvCpssLogStateCheck
(
    GT_VOID
);

/**
* @internal prvCpssLogParamFunc_GT_U8_PTR_ARRAY function
* @endinternal
*
* @brief   log handler for parsing an array of GT_U8 pointers
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] namePtr                  -
*                                      array name.
* @param[in] argsPtr                  -
*                                      va_list of the function arguments
* @param[in] skipLog                  -
*                                      to log or not to log
* @param[in] inOutParamInfoPtr        -
*                                      pointer to information about the function
*                                       None.
*/
void prvCpssLogParamFunc_GT_U8_PTR_ARRAY(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);

/**
* @internal prvCpssLogParamFunc_GT_U8_ARRAY function
* @endinternal
*
* @brief   log handler for parsing an array of GT_U8 elements
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] namePtr                  -
*                                      array name.
* @param[in] argsPtr                  -
*                                      va_list of the function arguments
* @param[in] skipLog                  -
*                                      to log or not to log
* @param[in] inOutParamInfoPtr        -
*                                      pointer to information about the function
*                                       None.
*/
void prvCpssLogParamFunc_GT_U8_ARRAY(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);

/**
* @internal prvCpssLogParamFunc_GT_U32_ARRAY function
* @endinternal
*
* @brief   log handler for parsing an array of GT_U32 elements
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] namePtr                  -
*                                      array name.
* @param[in] argsPtr                  -
*                                      va_list of the function arguments
* @param[in] skipLog                  -
*                                      to log or not to log
* @param[in] inOutParamInfoPtr        -
*                                      pointer to information about the function
*                                       None.
*/
void prvCpssLogParamFunc_GT_U32_ARRAY(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);

/**
* @internal prvCpssLogParamFunc_PACKET_PTR function
* @endinternal
*
* @brief   log handler for parsing a packet
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] namePtr                  -
*                                      variable name.
* @param[in] argsPtr                  -
*                                      va_list of the function arguments
* @param[in] skipLog                  -
*                                      to log or not to log
* @param[in] inOutParamInfoPtr        -
*                                      pointer to information about the function
*                                       None.
*/
void prvCpssLogParamFunc_PACKET_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);

/**
* @internal prvCpssLogParamFunc_PACKET_PTR_ARRAY function
* @endinternal
*
* @brief   log handler for parsing an array of packets
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] namePtr                  -
*                                      array name.
* @param[in] argsPtr                  -
*                                      va_list of the function arguments
* @param[in] skipLog                  -
*                                      to log or not to log
* @param[in] inOutParamInfoPtr        -
*                                      pointer to information about the function
*                                       None.
*/
void prvCpssLogParamFunc_PACKET_PTR_ARRAY(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);

/**
* @internal prvCpssLogParamFunc_GT_BOOL_ARRAY function
* @endinternal
*
* @brief   logs an array of GT_BOOL elements
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] namePtr                  -
*                                      array name.
* @param[in] argsPtr                  -
*                                      additional args
* @param[in] skipLog                  -
*                                      to skip the log or not
* @param[in] inOutParamInfoPtr        -
*                                      pointer to information about the array of parameters
*                                       None.
*/
void prvCpssLogParamFunc_GT_BOOL_ARRAY(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);


/**
* @internal prvCpssLogParamFunc_DSA_PTR function
* @endinternal
*
* @brief   logs an array of GT_U8 as DSA (array of words)
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] namePtr                  -
*                                      array name.
* @param[in] argsPtr                  -
*                                      additional args
* @param[in] skipLog                  -
*                                      to skip the log or not
* @param[in] inOutParamInfoPtr        -
*                                      pointer to information about the array of parameters
*                                       None.
*/
void prvCpssLogParamFunc_DSA_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);

/**
* @internal prvCpssLogParamFunc_GT_CHAR_ARRAY function
* @endinternal
*
* @brief   logs an array of GT_CHAR as a string
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] namePtr                  -
*                                      array name.
* @param[in] argsPtr                  -
*                                      additional args
* @param[in] skipLog                  -
*                                      to skip the log or not
* @param[in] inOutParamInfoPtr        -
*                                      pointer to information about the array of parameters
*                                       None.
*/
void prvCpssLogParamFunc_GT_CHAR_ARRAY(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);

/**
* @internal prvCpssLogStc32HexNumber function
* @endinternal
*
* @brief   The function to log 32 bit hex parameter
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] fieldName                - structure field name
* @param[in] numValue                 - field value.
*                                       None.
*/
void prvCpssLogStc32HexNumber
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              fieldName,
    IN GT_U32                   numValue
);

/**
* @internal prvCpssLogStc16HexNumber function
* @endinternal
*
* @brief   The function to log 16 bit hex parameter
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] fieldName                - structure field name
* @param[in] numValue                 - field value.
*                                       None.
*/
void prvCpssLogStc16HexNumber
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              fieldName,
    IN GT_U32                   numValue
);
/**
* @internal prvCpssLogParamFuncStc_GT_U32_HEX_PTR function
* @endinternal
*
* @brief   logs a variable from GT_U32 type in hexa format
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] namePtr                  -
*                                      array name.
* @param[in] fieldPtr                 -
*                                      pointer to the field
* @param[in,out] inOutParamInfoPtr        -
*                                      pointer to information about the array of parameters
*                                       None.
*/
void prvCpssLogParamFuncStc_GT_U32_HEX_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);

/**
* @internal prvCpssLogParamFunc_GT_U32_HEX_PTR function
* @endinternal
*
* @brief   logs a variable from GT_U32 type in hexa format
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] namePtr                  -
*                                      array name.
* @param[in] argsPtr                  -
*                                      additional args
* @param[in] skipLog                  -
*                                      to skip the log or not
* @param[in] inOutParamInfoPtr        -
*                                      pointer to information about the parameters
*                                       None.
*/
void prvCpssLogParamFunc_GT_U32_HEX_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);

/**
* @internal prvCpssLogParamFunc_GT_U32_HEX function
* @endinternal
*
* @brief   logs a variable from GT_U32 type in hexa format
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] namePtr                  -
*                                      array name.
* @param[in] argsPtr                  -
*                                      additional args
* @param[in] skipLog                  -
*                                      to skip the log or not
* @param[in] inOutParamInfoPtr        -
*                                      pointer to information about the parameters
*                                       None.
*/
void prvCpssLogParamFunc_GT_U32_HEX(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);

/**
* @internal prvCpssLogParamFunc_GT_U32_HEX_ARRAY function
* @endinternal
*
* @brief   logs an array of GT_U32_HEX elements
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] namePtr                  -
*                                      array name.
* @param[in] argsPtr                  -
*                                      additional args
* @param[in] skipLog                  -
*                                      to skip the log or not
* @param[in] inOutParamInfoPtr        -
*                                      pointer to information about the array of parameters
*                                       None.
*/
void prvCpssLogParamFunc_GT_U32_HEX_ARRAY(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);

/**
* @internal prvCpssLogParamFuncStc_GT_U64_HEX_PTR function
* @endinternal
*
* @brief   logs a variable from GT_U64 type in hexa format
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] namePtr                  -
*                                      array name.
* @param[in] fieldPtr                 -
*                                      pointer to the field
* @param[in,out] inOutParamInfoPtr        -
*                                      pointer to information about the array of parameters
*                                       None.
*/
void prvCpssLogParamFuncStc_GT_U64_HEX_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);

/**
* @internal prvCpssLogParamFunc_GT_U64_HEX function
* @endinternal
*
* @brief   logs a variable from GT_U64 type in hexa format
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] namePtr                  -
*                                      array name.
*                                      fieldPtr -
*                                      pointer to the field
* @param[in] inOutParamInfoPtr        -
*                                      pointer to information about the array of parameters
*                                       None.
*/
void prvCpssLogParamFunc_GT_U64_HEX(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);

/**
* @internal prvCpssLogParamFunc_GT_U64_HEX_PTR function
* @endinternal
*
* @brief   logs a variable from GT_U64 type in hexa format
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] namePtr                  -
*                                      array name.
*                                      fieldPtr -
*                                      pointer to the field
* @param[in] inOutParamInfoPtr        -
*                                      pointer to information about the array of parameters
*                                       None.
*/
void prvCpssLogParamFunc_GT_U64_HEX_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);

/**
* @internal prvCpssLogParamFuncStc_GT_U16_HEX_PTR function
* @endinternal
*
* @brief   logs a variable from GT_U16 type in hexa format
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] namePtr                  -
*                                      array name.
* @param[in] fieldPtr                 -
*                                      pointer to the field
* @param[in,out] inOutParamInfoPtr        -
*                                      pointer to information about the array of parameters
*                                       None.
*/
void prvCpssLogParamFuncStc_GT_U16_HEX_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);

/**
* @internal prvCpssLogParamFunc_GT_U16_HEX_PTR function
* @endinternal
*
* @brief   logs a variable from GT_U16 type in hexa format
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] namePtr                  -
*                                      array name.
* @param[in] argsPtr                  -
*                                      additional args
* @param[in] skipLog                  -
*                                      to skip the log or not
* @param[in] inOutParamInfoPtr        -
*                                      pointer to information about the parameters
*                                       None.
*/
void prvCpssLogParamFunc_GT_U16_HEX_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);

/**
* @internal prvCpssLogParamFunc_GT_U16_HEX function
* @endinternal
*
* @brief   logs a variable from GT_U16 type in hexa format
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] namePtr                  -
*                                      array name.
* @param[in] argsPtr                  -
*                                      additional args
* @param[in] skipLog                  -
*                                      to skip the log or not
* @param[in] inOutParamInfoPtr        -
*                                      pointer to information about the parameters
*                                       None.
*/
void prvCpssLogParamFunc_GT_U16_HEX(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);

/**
* @internal prvCpssLogParamFunc_GT_U16_HEX_ARRAY function
* @endinternal
*
* @brief   logs an array of GT_U16_HEX elements
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] namePtr                  -
*                                      array name.
* @param[in] argsPtr                  -
*                                      additional args
* @param[in] skipLog                  -
*                                      to skip the log or not
* @param[in] inOutParamInfoPtr        -
*                                      pointer to information about the array of parameters
*                                       None.
*/
void prvCpssLogParamFunc_GT_U16_HEX_ARRAY(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);

/**
* @internal prvCpssLogU64Number function
* @endinternal
*
* @brief   The function to log GT_U64 variable as two GT_U32 variables
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] variableName             - variable name
* @param[in] variablePtr              - pointer to the variable.
* @param[in] isHex                    - GT_TRUE for hexadecimal format and GT_FALSE otherwise
*                                       None.
*/
void prvCpssLogU64Number
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              variableName,
    IN GT_U64                   * variablePtr,
    IN GT_BOOL                  isHex
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssLogh */


