/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssGenCpssHwInitLog.c
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/

/* disable deprecation warnings (if one) */
#ifdef __GNUC__
#if  (__GNUC__*100+__GNUC_MINOR__) >= 406
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#endif

#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/cpssHwInit/private/prvCpssCommonCpssHwInitLog.h>
#include <cpss/generic/cpssHwInit/cpssHwInit.h>
#include <cpss/generic/cpssHwInit/cpssLedCtrl.h>
#include <cpss/generic/cpssHwInit/private/prvCpssGenCpssHwInitLog.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>


/********* enums *********/

char * prvCpssLogEnum_CPSS_MAC_SA_LSB_MODE_ENT[]  =
{
    "CPSS_SA_LSB_PER_PORT_E",
    "CPSS_SA_LSB_PER_PKT_VID_E",
    "CPSS_SA_LSB_PER_VLAN_E",
    "CPSS_SA_LSB_FULL_48_BIT_GLOBAL"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_MAC_SA_LSB_MODE_ENT);
char * prvCpssLogEnum_CPSS_AU_MESSAGE_LENGTH_ENT[]  =
{
    "CPSS_AU_MESSAGE_LENGTH_4_WORDS_E",
    "CPSS_AU_MESSAGE_LENGTH_8_WORDS_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_AU_MESSAGE_LENGTH_ENT);
char * prvCpssLogEnum_CPSS_LED_CLASS_13_SELECT_ENT[]  =
{
    "CPSS_LED_CLASS_13_SELECT_LINK_DOWN_E",
    "CPSS_LED_CLASS_13_SELECT_COPPER_LINK_UP_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_LED_CLASS_13_SELECT_ENT);
char * prvCpssLogEnum_CPSS_LED_CLASS_5_SELECT_ENT[]  =
{
    "CPSS_LED_CLASS_5_SELECT_HALF_DUPLEX_E",
    "CPSS_LED_CLASS_5_SELECT_FIBER_LINK_UP_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_LED_CLASS_5_SELECT_ENT);
char * prvCpssLogEnum_CPSS_DRAM_BUS_WIDTH_ENT[]  =
{
    "CPSS_DRAM_BUS_WIDTH_4_E",
    "CPSS_DRAM_BUS_WIDTH_8_E",
    "CPSS_DRAM_BUS_WIDTH_16_E",
    "CPSS_DRAM_BUS_WIDTH_32_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DRAM_BUS_WIDTH_ENT);
PRV_CPSS_ENUM_STRING_VALUE_PAIR_STC prvCpssLogEnum_map_CPSS_DRAM_FREQUENCY_ENT[]  =
{PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DRAM_FREQ_100_MHZ_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DRAM_FREQ_125_MHZ_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DRAM_FREQ_166_MHZ_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DRAM_FREQ_200_MHZ_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DRAM_FREQ_250_MHZ_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DRAM_FREQ_300_MHZ_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DRAM_FREQ_667_MHZ_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DRAM_FREQ_800_MHZ_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DRAM_FREQ_933_MHZ_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DRAM_FREQ_1066_MHZ_E)
};
PRV_CPSS_LOG_STC_ENUM_MAP_ARRAY_SIZE_MAC(CPSS_DRAM_FREQUENCY_ENT);
PRV_CPSS_ENUM_STRING_VALUE_PAIR_STC prvCpssLogEnum_map_CPSS_DRAM_SIZE_ENT[]  =
{PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DRAM_NOT_USED_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DRAM_64KB_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DRAM_128KB_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DRAM_8MB_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DRAM_16MB_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DRAM_32MB_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DRAM_64MB_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DRAM_128MB_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DRAM_256MB_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DRAM_512MB_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DRAM_1GB_E)
};
PRV_CPSS_LOG_STC_ENUM_MAP_ARRAY_SIZE_MAC(CPSS_DRAM_SIZE_ENT);
char * prvCpssLogEnum_CPSS_DRAM_SPEED_BIN_ENT[]  =
{
    "CPSS_DRAM_SPEED_BIN_DDR3_800D_E",
    "CPSS_DRAM_SPEED_BIN_DDR3_800E_E",
    "CPSS_DRAM_SPEED_BIN_DDR3_1066E_E",
    "CPSS_DRAM_SPEED_BIN_DDR3_1066F_E",
    "CPSS_DRAM_SPEED_BIN_DDR3_1066G_E",
    "CPSS_DRAM_SPEED_BIN_DDR3_1333F_E",
    "CPSS_DRAM_SPEED_BIN_DDR3_1333G_E",
    "CPSS_DRAM_SPEED_BIN_DDR3_1333H_E",
    "CPSS_DRAM_SPEED_BIN_DDR3_1333J_E",
    "CPSS_DRAM_SPEED_BIN_DDR3_1600G_E",
    "CPSS_DRAM_SPEED_BIN_DDR3_1600H_E",
    "CPSS_DRAM_SPEED_BIN_DDR3_1600J_E",
    "CPSS_DRAM_SPEED_BIN_DDR3_1600K_E",
    "CPSS_DRAM_SPEED_BIN_DDR3_1866J_E",
    "CPSS_DRAM_SPEED_BIN_DDR3_1866K_E",
    "CPSS_DRAM_SPEED_BIN_DDR3_1866L_E",
    "CPSS_DRAM_SPEED_BIN_DDR3_1866M_E",
    "CPSS_DRAM_SPEED_BIN_DDR3_2133K_E",
    "CPSS_DRAM_SPEED_BIN_DDR3_2133L_E",
    "CPSS_DRAM_SPEED_BIN_DDR3_2133M_E",
    "CPSS_DRAM_SPEED_BIN_DDR3_2133N_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DRAM_SPEED_BIN_ENT);
char * prvCpssLogEnum_CPSS_DRAM_TEMPERATURE_ENT[]  =
{
    "CPSS_DRAM_TEMPERATURE_NORMAL_E",
    "CPSS_DRAM_TEMPERATURE_HIGH_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DRAM_TEMPERATURE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_LED_CLASS_MANIPULATION_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_LED_CLASS_MANIPULATION_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, invertEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, blinkEnable);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, blinkSelect, CPSS_LED_BLINK_SELECT_ENT);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, forceEnable);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, forceData);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, pulseStretchEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, disableOnLinkDown);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_LED_CONF_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_LED_CONF_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, ledOrganize, CPSS_LED_ORDER_MODE_ENT);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, disableOnLinkDown);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, blink0DutyCycle, CPSS_LED_BLINK_DUTY_CYCLE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, blink0Duration, CPSS_LED_BLINK_DURATION_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, blink1DutyCycle, CPSS_LED_BLINK_DUTY_CYCLE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, blink1Duration, CPSS_LED_BLINK_DURATION_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, pulseStretch, CPSS_LED_PULSE_STRETCH_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ledStart);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ledEnd);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, clkInvert);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, class5select, CPSS_LED_CLASS_5_SELECT_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, class13select, CPSS_LED_CLASS_13_SELECT_ENT);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, invertEnable);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, ledClockFrequency, CPSS_LED_CLOCK_OUT_FREQUENCY_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_REG_VALUE_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_REG_VALUE_INFO_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, addrType);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, regMask);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, regVal);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, repeatCount);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_LED_CLASS_MANIPULATION_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_LED_CLASS_MANIPULATION_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_LED_CLASS_MANIPULATION_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_LED_CONF_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_LED_CONF_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_LED_CONF_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_MAC_SA_LSB_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_MAC_SA_LSB_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_MAC_SA_LSB_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_MAC_SA_LSB_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_MAC_SA_LSB_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_MAC_SA_LSB_MODE_ENT);
}

