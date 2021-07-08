/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssCommonCpssHwInitLog.c
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/cpssHwInit/cpssCommonLedCtrl.h>
#include <cpss/common/cpssHwInit/cpssHwInit.h>
#include <cpss/common/cpssHwInit/private/prvCpssCommonCpssHwInitLog.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>


/********* enums *********/

char * prvCpssLogEnum_CPSS_HW_PP_INIT_STAGE_ENT[]  =
{
    "CPSS_HW_PP_INIT_STAGE_INIT_DURING_RESET_E",
    "CPSS_HW_PP_INIT_STAGE_EEPROM_DONE_INT_MEM_DONE_E",
    "CPSS_HW_PP_INIT_STAGE_EEPROM_NOT_DONE_INT_MEM_DONE_E",
    "CPSS_HW_PP_INIT_STAGE_FULLY_FUNC_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_HW_PP_INIT_STAGE_ENT);
char * prvCpssLogEnum_CPSS_HW_PP_RESET_SKIP_TYPE_ENT[]  =
{
    "CPSS_HW_PP_RESET_SKIP_TYPE_REGISTER_E",
    "CPSS_HW_PP_RESET_SKIP_TYPE_TABLE_E",
    "CPSS_HW_PP_RESET_SKIP_TYPE_EEPROM_E",
    "CPSS_HW_PP_RESET_SKIP_TYPE_PEX_E",
    "CPSS_HW_PP_RESET_SKIP_TYPE_LINK_LOSS_E",
    "CPSS_HW_PP_RESET_SKIP_TYPE_CHIPLETS_E",
    "CPSS_HW_PP_RESET_SKIP_TYPE_ALL_E",
    "CPSS_HW_PP_RESET_SKIP_TYPE_ALL_EXCLUDE_PEX_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_HW_PP_RESET_SKIP_TYPE_ENT);
char * prvCpssLogEnum_CPSS_PP_INTERFACE_CHANNEL_ENT[]  =
{
    "CPSS_CHANNEL_PCI_E",
    "CPSS_CHANNEL_SMI_E",
    "CPSS_CHANNEL_TWSI_E",
    "CPSS_CHANNEL_PEX_E",
    "CPSS_CHANNEL_PEX_MBUS_E",
    "CPSS_CHANNEL_PEX_EAGLE_E",
    "CPSS_CHANNEL_PEX_KERNEL_E",
    "CPSS_CHANNEL_LAST_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PP_INTERFACE_CHANNEL_ENT);
char * prvCpssLogEnum_CPSS_LED_BLINK_DURATION_ENT[]  =
{
    "CPSS_LED_BLINK_DURATION_0_E",
    "CPSS_LED_BLINK_DURATION_1_E",
    "CPSS_LED_BLINK_DURATION_2_E",
    "CPSS_LED_BLINK_DURATION_3_E",
    "CPSS_LED_BLINK_DURATION_4_E",
    "CPSS_LED_BLINK_DURATION_5_E",
    "CPSS_LED_BLINK_DURATION_6_E",
    "CPSS_LED_BLINK_DURATION_7_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_LED_BLINK_DURATION_ENT);
char * prvCpssLogEnum_CPSS_LED_BLINK_DUTY_CYCLE_ENT[]  =
{
    "CPSS_LED_BLINK_DUTY_CYCLE_0_E",
    "CPSS_LED_BLINK_DUTY_CYCLE_1_E",
    "CPSS_LED_BLINK_DUTY_CYCLE_2_E",
    "CPSS_LED_BLINK_DUTY_CYCLE_3_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_LED_BLINK_DUTY_CYCLE_ENT);
char * prvCpssLogEnum_CPSS_LED_BLINK_SELECT_ENT[]  =
{
    "CPSS_LED_BLINK_SELECT_0_E",
    "CPSS_LED_BLINK_SELECT_1_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_LED_BLINK_SELECT_ENT);
char * prvCpssLogEnum_CPSS_LED_CLOCK_OUT_FREQUENCY_ENT[]  =
{
    "CPSS_LED_CLOCK_OUT_FREQUENCY_500_E",
    "CPSS_LED_CLOCK_OUT_FREQUENCY_1000_E",
    "CPSS_LED_CLOCK_OUT_FREQUENCY_2000_E",
    "CPSS_LED_CLOCK_OUT_FREQUENCY_3000_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_LED_CLOCK_OUT_FREQUENCY_ENT);
char * prvCpssLogEnum_CPSS_LED_ORDER_MODE_ENT[]  =
{
    "CPSS_LED_ORDER_MODE_BY_PORT_E",
    "CPSS_LED_ORDER_MODE_BY_CLASS_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_LED_ORDER_MODE_ENT);
char * prvCpssLogEnum_CPSS_LED_PULSE_STRETCH_ENT[]  =
{
    "CPSS_LED_PULSE_STRETCH_0_NO_E",
    "CPSS_LED_PULSE_STRETCH_1_E",
    "CPSS_LED_PULSE_STRETCH_2_E",
    "CPSS_LED_PULSE_STRETCH_3_E",
    "CPSS_LED_PULSE_STRETCH_4_E",
    "CPSS_LED_PULSE_STRETCH_5_E",
    "CPSS_LED_PULSE_STRETCH_6_E",
    "CPSS_LED_PULSE_STRETCH_7_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_LED_PULSE_STRETCH_ENT);
char * prvCpssLogEnum_CPSS_RX_BUFF_ALLOC_METHOD_ENT[]  =
{
    "CPSS_RX_BUFF_DYNAMIC_ALLOC_E",
    "CPSS_RX_BUFF_STATIC_ALLOC_E",
    "CPSS_RX_BUFF_NO_ALLOC_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_RX_BUFF_ALLOC_METHOD_ENT);

#ifdef CHX_FAMILY
char * prvCpssLogEnum_CPSS_SYS_HA_MODE_ENT[]  =
{
    "CPSS_SYS_HA_MODE_ACTIVE_E",
    "CPSS_SYS_HA_MODE_STANDBY_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_SYS_HA_MODE_ENT);
char * prvCpssLogEnum_CPSS_TX_BUFF_ALLOC_METHOD_ENT[]  =
{
    "CPSS_TX_BUFF_DYNAMIC_ALLOC_E",
    "CPSS_TX_BUFF_STATIC_ALLOC_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_TX_BUFF_ALLOC_METHOD_ENT);
char * prvCpssLogEnum_CPSS_TX_SDMA_QUEUE_MODE_ENT[]  =
{
    "CPSS_TX_SDMA_QUEUE_MODE_NORMAL_E",
    "CPSS_TX_SDMA_QUEUE_MODE_PACKET_GENERATOR_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_TX_SDMA_QUEUE_MODE_ENT);
#endif


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_LED_GROUP_CONF_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_LED_GROUP_CONF_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, classA);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, classB);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, classC);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, classD);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_NET_IF_CFG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_NET_IF_CFG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, txDescBlock);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, txDescBlockSize);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, rxDescBlock);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, rxDescBlockSize);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, rxBufInfo, CPSS_RX_BUF_INFO_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_RX_BUF_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_RX_BUF_INFO_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, allocMethod, CPSS_RX_BUFF_ALLOC_METHOD_ENT);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, bufferPercentage, CPSS_MAX_RX_QUEUE_CNS, GT_U32);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, rxBufSize);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, headerOffset);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, buffersInCachedMem);
    prvCpssLogStcLogStart(contextLib,  logType, "buffData");
    prvCpssLogStcLogStart(contextLib,  logType, "staticAlloc");
    PRV_CPSS_LOG_STC_PTR_MAC((&(&valPtr->buffData)->staticAlloc), rxBufBlockPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC((&(&valPtr->buffData)->staticAlloc), rxBufBlockSize);
    prvCpssLogStcLogEnd(contextLib, logType);
    prvCpssLogStcLogStart(contextLib,  logType, "dynamicAlloc");
    PRV_CPSS_LOG_STC_PTR_MAC((&(&valPtr->buffData)->dynamicAlloc), mallocFunc);
    PRV_CPSS_LOG_STC_NUMBER_MAC((&(&valPtr->buffData)->dynamicAlloc), numOfRxBuffers);
    prvCpssLogStcLogEnd(contextLib, logType);
    prvCpssLogStcLogEnd(contextLib, logType);
    prvCpssLogStcLogEnd(contextLib, logType);
}

#ifdef CHX_FAMILY
void prvCpssLogParamFuncStc_CPSS_AUQ_CFG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_AUQ_CFG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, auDescBlock);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, auDescBlockSize);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_MULTI_NET_IF_CFG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_MULTI_NET_IF_CFG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_TYPE_ARRAY2_MAC(valPtr, rxSdmaQueuesConfig, CPSS_MAX_SDMA_CPU_PORTS_CNS, CPSS_MAX_RX_QUEUE_CNS,  CPSS_MULTI_NET_IF_RX_SDMA_QUEUE_STC);
    PRV_CPSS_LOG_STC_TYPE_ARRAY2_MAC(valPtr, txSdmaQueuesConfig, CPSS_MAX_SDMA_CPU_PORTS_CNS, CPSS_MAX_TX_QUEUE_CNS,  CPSS_MULTI_NET_IF_TX_SDMA_QUEUE_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_MULTI_NET_IF_RX_SDMA_QUEUE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_MULTI_NET_IF_RX_SDMA_QUEUE_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, buffAllocMethod, CPSS_RX_BUFF_ALLOC_METHOD_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, numOfRxDesc);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, numOfRxBuff);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, buffSize);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, buffHeaderOffset);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, buffersInCachedMem);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, descMemPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, descMemSize);
    prvCpssLogStcLogStart(contextLib,  logType, "memData");
    prvCpssLogStcLogStart(contextLib,  logType, "staticAlloc");
    PRV_CPSS_LOG_STC_PTR_MAC((&(&valPtr->memData)->staticAlloc), buffMemPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC((&(&valPtr->memData)->staticAlloc), buffMemSize);
    prvCpssLogStcLogEnd(contextLib, logType);
    prvCpssLogStcLogStart(contextLib,  logType, "dynamicAlloc");
    PRV_CPSS_LOG_STC_PTR_MAC((&(&valPtr->memData)->dynamicAlloc), buffMallocFunc);
    prvCpssLogStcLogEnd(contextLib, logType);
    prvCpssLogStcLogEnd(contextLib, logType);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_MULTI_NET_IF_TX_SDMA_QUEUE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_MULTI_NET_IF_TX_SDMA_QUEUE_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, queueMode, CPSS_TX_SDMA_QUEUE_MODE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, buffAndDescAllocMethod, CPSS_TX_BUFF_ALLOC_METHOD_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, numOfTxDesc);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, numOfTxBuff);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, buffSize);
    prvCpssLogStcLogStart(contextLib,  logType, "memData");
    prvCpssLogStcLogStart(contextLib,  logType, "staticAlloc");
    PRV_CPSS_LOG_STC_PTR_MAC((&(&valPtr->memData)->staticAlloc), buffAndDescMemPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC((&(&valPtr->memData)->staticAlloc), buffAndDescMemSize);
    prvCpssLogStcLogEnd(contextLib, logType);
    prvCpssLogStcLogStart(contextLib,  logType, "dynamicAlloc");
    PRV_CPSS_LOG_STC_PTR_MAC((&(&valPtr->memData)->dynamicAlloc), buffAndDescMallocFunc);
    prvCpssLogStcLogEnd(contextLib, logType);
    prvCpssLogStcLogEnd(contextLib, logType);
    prvCpssLogStcLogEnd(contextLib, logType);
}

#endif

#ifdef PX_FAMILY
void prvCpssLogParamFuncStc_CPSS_DMA_QUEUE_CFG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DMA_QUEUE_CFG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, dmaDescBlock);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, dmaDescBlockSize);
    prvCpssLogStcLogEnd(contextLib, logType);
}

#endif


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_HW_PP_INIT_STAGE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_HW_PP_INIT_STAGE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_HW_PP_INIT_STAGE_ENT);
}
void prvCpssLogParamFunc_CPSS_HW_PP_RESET_SKIP_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_HW_PP_RESET_SKIP_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_HW_PP_RESET_SKIP_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_LED_GROUP_CONF_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_LED_GROUP_CONF_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_LED_GROUP_CONF_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_PP_INTERFACE_CHANNEL_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PP_INTERFACE_CHANNEL_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PP_INTERFACE_CHANNEL_ENT);
}

#ifdef CHX_FAMILY
void prvCpssLogParamFunc_CPSS_AUQ_CFG_STC_PTR_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_AUQ_CFG_STC**, paramVal);
    prvCpssLogParamFuncStc_CPSS_AUQ_CFG_STC_PTR(contextLib, logType, namePtr, *paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_MULTI_NET_IF_CFG_STC_PTR_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_MULTI_NET_IF_CFG_STC**, paramVal);
    prvCpssLogParamFuncStc_CPSS_MULTI_NET_IF_CFG_STC_PTR(contextLib, logType, namePtr, *paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_NET_IF_CFG_STC_PTR_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_NET_IF_CFG_STC**, paramVal);
    prvCpssLogParamFuncStc_CPSS_NET_IF_CFG_STC_PTR(contextLib, logType, namePtr, *paramVal, inOutParamInfoPtr);
}

#endif

#ifdef PX_FAMILY
void prvCpssLogParamFunc_CPSS_DMA_QUEUE_CFG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DMA_QUEUE_CFG_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DMA_QUEUE_CFG_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_NET_IF_CFG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_NET_IF_CFG_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_NET_IF_CFG_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}

#endif


/********* API fields DB *********/

PRV_CPSS_LOG_FUNC_PARAM_STC INOUT_GT_U32_PTR_configDevDataBufferSizePtr = {
     "configDevDataBufferSizePtr", PRV_CPSS_LOG_PARAM_INOUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC INOUT_void_PTR_configDevDataBufferPtr = {
     "configDevDataBufferPtr", PRV_CPSS_LOG_PARAM_INOUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(void)
};
PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_configDevDataBufferSize = {
     "configDevDataBufferSize", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
PRV_CPSS_LOG_FUNC_PARAM_STC IN_void_PTR_configDevDataBufferPtr = {
     "configDevDataBufferPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(void)
};


/********* API prototypes DB *********/

PRV_CPSS_LOG_FUNC_PARAM_STC * cpssPpInterruptsDisable_PARAMS[] =  {
    &IN_GT_U8_devNum
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssPpConfigDevDataImport_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_void_PTR_configDevDataBufferPtr,
    &IN_GT_U32_configDevDataBufferSize
};
PRV_CPSS_LOG_FUNC_PARAM_STC * cpssPpConfigDevDataExport_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &INOUT_void_PTR_configDevDataBufferPtr,
    &INOUT_GT_U32_PTR_configDevDataBufferSizePtr
};


/********* lib API DB *********/


#ifdef CHX_FAMILY
static PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssCommonDXCpssHwInit_commonLogLibDb[] = {
    {"cpssPpInterruptsDisable", 1, cpssPpInterruptsDisable_PARAMS, NULL},
    {"cpssPpConfigDevDataImport", 3, cpssPpConfigDevDataImport_PARAMS, NULL},
    {"cpssPpConfigDevDataExport", 3, cpssPpConfigDevDataExport_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_HW_INIT_COMMON(
    OUT PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssCommonDXCpssHwInit_commonLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssCommonDXCpssHwInit_commonLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}
#endif

#ifdef PX_FAMILY
static PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssCommonPXCpssHwInit_commonLogLibDb[] = {
    {"cpssPpInterruptsDisable", 1, cpssPpInterruptsDisable_PARAMS, NULL},
    {"cpssPpConfigDevDataImport", 3, cpssPpConfigDevDataImport_PARAMS, NULL},
    {"cpssPpConfigDevDataExport", 3, cpssPpConfigDevDataExport_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_PX_CPSS_LOG_LIB_HW_INIT_COMMON(
    OUT PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssCommonPXCpssHwInit_commonLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssCommonPXCpssHwInit_commonLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}
#endif

