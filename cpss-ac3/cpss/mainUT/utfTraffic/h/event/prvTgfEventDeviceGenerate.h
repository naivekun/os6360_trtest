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
* @file prvTgfEventDeviceGenerate.h
* @version   1
********************************************************************************
*/

#ifndef __prvTgfEventDeviceGenerate
#define __prvTgfEventDeviceGenerate

#include <utf/utfMain.h>

/* Size of array of failed events */
#define PRV_TGF_FAILED_EVENTS_ARRAY_SIZE 100
/* List of events disabled by default */
#define PRV_TGF_DISABLED_BY_DEFAULT_EVENTS {                              \
                                      CPSS_PP_MISC_TWSI_TIME_OUT_E        \
                                     ,CPSS_PP_MISC_TWSI_STATUS_E          \
                                     ,CPSS_PP_TQ_PORT_DESC_FULL_E         \
                                     ,CPSS_PP_MAC_UPDATE_FROM_CPU_DONE_E  \
                                     ,CPSS_PP_MAC_NA_SELF_LEARNED_E       \
                                     ,CPSS_PP_MAC_NA_FROM_CPU_DROPPED_E   \
                                     ,CPSS_PP_MAC_AGED_OUT_E              \
                                     ,CPSS_PP_XSMI_WRITE_DONE_E           \
                                            }

/* List of unsupported events */
#define PRV_TGF_UNSUPPORTED_EVENTS {  CPSS_PP_PORT_802_3_AP_E             \
                                     ,CPSS_PP_CRITICAL_HW_ERROR_E         \
                                     ,CPSS_PP_RX_BUFFER_QUEUE0_E          \
                                     ,CPSS_PP_RX_BUFFER_QUEUE1_E          \
                                     ,CPSS_PP_RX_BUFFER_QUEUE2_E          \
                                     ,CPSS_PP_RX_BUFFER_QUEUE3_E          \
                                     ,CPSS_PP_RX_BUFFER_QUEUE4_E          \
                                     ,CPSS_PP_RX_BUFFER_QUEUE5_E          \
                                     ,CPSS_PP_RX_BUFFER_QUEUE6_E          \
                                     ,CPSS_PP_RX_BUFFER_QUEUE7_E          \
                                     ,CPSS_PP_RX_ERR_QUEUE0_E             \
                                     ,CPSS_PP_RX_ERR_QUEUE1_E             \
                                     ,CPSS_PP_RX_ERR_QUEUE2_E             \
                                     ,CPSS_PP_RX_ERR_QUEUE3_E             \
                                     ,CPSS_PP_RX_ERR_QUEUE4_E             \
                                     ,CPSS_PP_RX_ERR_QUEUE5_E             \
                                     ,CPSS_PP_RX_ERR_QUEUE6_E             \
                                     ,CPSS_PP_RX_ERR_QUEUE7_E             \
                                     ,CPSS_PP_RX_CNTR_OVERFLOW_E          \
                                     ,CPSS_PP_TX_BUFFER_QUEUE_E           \
                                     ,CPSS_PP_TX_ERR_QUEUE_E              \
                                     ,CPSS_PP_TX_END_E                    \
                                     ,CPSS_PP_BM_INVALID_ADDRESS_E        \
                                    }

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

/* PRV_PRINT_EVENT_CTX types is used for printing failed events*/
typedef struct
{
    CPSS_UNI_EV_CAUSE_ENT event; /* event to print */
    GT_STATUS rc;                /* return code from
                                 cpssEventDeviceGenerate function*/
    GT_U32 counter;              /* event counter value */
    GT_U32 evExtData;
} PRV_PRINT_EVENT_CTX;

GT_VOID prvTgfGenEvent(GT_VOID);
GT_VOID prvTgfPhyToMacPortMapping(GT_VOID);
GT_VOID prvTgfEventPortLinkStatusUnbindBind(GT_VOID);

#endif /* __prvTgfEventDeviceGenerate */

