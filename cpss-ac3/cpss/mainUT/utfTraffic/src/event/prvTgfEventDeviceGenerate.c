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
* @file prvTgfEventDeviceGenerate.c
*
* @brief Includes code for test that checks event generation from cpssAPI
*
* @version   1
********************************************************************************
*/
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/private/prvUtfHelpers.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

#include <common/tgfCommon.h>
#include <cpss/generic/events/private/prvCpssGenEvReq.h>
#include <event/prvTgfEventDeviceGenerate.h>
#include <appDemo/userExit/userEventHandler.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvExMxEventsBobcat2.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvExMxEventsBobK.h>

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
GT_VOID prvTgfPrintFailedEvents
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
* @internal prvTgfBobcat2InitEvExtDataArray function
* @endinternal
*
* @brief   This function is used to fill evExtData
*         array for unified event.
* @param[in] devNum                   - device number
* @param[in] event                    - a unified event
*
* @param[out] evExtDataArray           - array of evExtData values
* @param[out] evExtDataSize            - size of evExtDataArray
* @param[out] expectedCounter          - (pointer to) expected counter for unified event
*                                       None
*/
static GT_VOID prvTgfBobcat2InitEvExtDataArray
(
    IN  GT_U8   devNum,
    IN  CPSS_UNI_EV_CAUSE_ENT event, /* unified event */
    OUT GT_U32* evExtDataArray,      /* array of evExtData values */
    OUT GT_U32* evExtDataSize,       /* size of evExtDataArray */
    OUT GT_U32* expectedCounter      /* expected counter for unified event */
)
{
    GT_U32 i = 0;
    GT_U32 numOfTxQPorts;
    GT_U32 numOfTxQDqUnits;
    GT_U32 numOfProcessingPipes;
    GT_U32 isPerTile;
    GT_U32 isPerMg,mgNumOfUnits;
    GT_U32 index;
    GT_U32 numPerPipe;
    GT_U32 isPerMac,numMacPorts;

    if(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipes > 1)
    {
        numOfProcessingPipes = PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipes;
    }
    else
    {
        numOfProcessingPipes = 1;
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.numTxqDq)
    {
        numOfTxQDqUnits = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.numTxqDq;
        numOfTxQPorts = numOfTxQDqUnits *
            PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp;
    }
    else
    {
        numOfTxQPorts = 72;
        numOfTxQDqUnits = 1;
    }

    *evExtDataSize = 1;

    switch (event)
    {
        case CPSS_PP_GPP_E:
            PRV_TGF_FILL_EXT_DATA_ARRAY(8);
            break;
        case CPSS_PP_TQ_MISC_E:
            PRV_TGF_FILL_EXT_DATA_ARRAY(5);
            break;
        case CPSS_PP_EB_NA_FIFO_FULL_E:
            PRV_TGF_FILL_EXT_DATA_ARRAY(8);
            break;
        case CPSS_PP_MAC_SFLOW_E:
            if (PRV_CPSS_SIP_5_25_CHECK_MAC(devNum))
            {
                PRV_TGF_FILL_EXT_DATA_ARRAY(128);
            }
            else if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                PRV_TGF_FILL_EXT_DATA_ARRAY(512);
            }
            else {
                PRV_TGF_FILL_EXT_DATA_ARRAY(256);
            }
            break;

        case CPSS_PP_SCT_RATE_LIMITER_E:
            PRV_TGF_FILL_EXT_DATA_ARRAY(256);
            break;
        case CPSS_PP_POLICER_ADDR_OUT_OF_MEMORY_E:
        case CPSS_PP_POLICER_IPFIX_WRAP_AROUND_E:
        case CPSS_PP_POLICER_IPFIX_ALARM_E:
        case CPSS_PP_POLICER_IPFIX_ALARMED_ENTRIES_FIFO_WRAP_AROUND_E:
        case CPSS_PP_PCL_LOOKUP_DATA_ERROR_E:
        case CPSS_PP_PCL_LOOKUP_FIFO_FULL_E:
            PRV_TGF_FILL_EXT_DATA_ARRAY(3);
            break;
        case CPSS_PP_EGRESS_SFLOW_E:
        case CPSS_PP_TQ_PORT_DESC_FULL_E:
            PRV_TGF_FILL_EXT_DATA_ARRAY(numOfTxQPorts);
            break;

        case CPSS_PP_OAM_EXCEPTION_KEEPALIVE_AGING_E:
        case CPSS_PP_OAM_EXCEPTION_EXCESS_KEEPALIVE_E:
        case CPSS_PP_OAM_EXCEPTION_INVALID_KEEPALIVE_E:
        case CPSS_PP_OAM_EXCEPTION_RDI_STATUS_E:
        case CPSS_PP_OAM_EXCEPTION_TX_PERIOD_E:
        case CPSS_PP_OAM_EXCEPTION_MEG_LEVEL_E:
        case CPSS_PP_OAM_EXCEPTION_SOURCE_INTERFACE_E:
        case CPSS_PP_OAM_ILLEGAL_ENTRY_INDEX_E:
        case CPSS_PP_TTI_ACCESS_DATA_ERROR_E:
            PRV_TGF_FILL_EXT_DATA_ARRAY(2);
            break;

        case CPSS_PP_DATA_INTEGRITY_ERROR_E:
            if (IS_BOBK_DEV_MAC(devNum))
            {
                index = 0;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_TTI_SUM_CRITICAL_ECC_ONE_ERROR_CORRECTED_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_TTI_SUM_CRITICAL_ECC_TWO_ERROR_DETECTED_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_HA_SUM_ECC_SINGLE_ERROR_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_HA_SUM_ECC_DOUBLE_ERROR_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_MLL_SUM_FILE_ECC_1_ERROR_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_MLL_SUM_FILE_ECC_2_ERROR_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_EFT_SUM_DESC_ECC_SINGLE_ERROR_DETECTED_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_EFT_SUM_DESC_ECC_DOUBLE_ERROR_DETECTED_INT_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_TXQ_GEN_SUM_TD_CLR_ECC_ONE_ERROR_CORRECTED_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_TXQ_GEN_SUM_TD_CLR_ECC_TWO_ERROR_DETECTED_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_PFC_PARITY_SUM_PFC_PORT_GROUP0_PFC_IND_FIFO_ECC_ERR_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_QCN_SUM_DESC_ECC_SINGLE_ERROR_DETECTED_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_QCN_SUM_DESC_ECC_DOUBLE_ERROR_DETECTED_INT_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_TXQ_DQ_GEN_SUM_DQ_INC_ECC_ONE_ERROR_CORRECTED_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_TXQ_DQ_GEN_SUM_DQ_INC_ECC_TWO_ERROR_DETECTED_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_TXQ_DQ_GEN_SUM_DQ_CLR_ECC_ONE_ERROR_CORRECTED_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_TXQ_DQ_GEN_SUM_DQ_CLR_ECC_TWO_ERROR_DETECTED_E;
#ifndef ASIC_SIMULATION
                if (!IS_BOBK_DEV_CETUS_MAC(devNum))
                {
                    evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_TXDMA_0_GENENAL_SUM_ECC_SINGLE_ERROR_E;
                    evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_TXDMA_0_GENENAL_SUM_ECC_DOUBLE_ERROR_E;
                }
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_TXDMA_1_GENENAL_SUM_ECC_SINGLE_ERROR_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_TXDMA_1_GENENAL_SUM_ECC_DOUBLE_ERROR_E;
                if (!IS_BOBK_DEV_CETUS_MAC(devNum))
                {
                    evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_TX_FIFO_0_GEBERAL1_SUM_ECC_0_SINGLE_ERROR_E;
                    evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_TX_FIFO_0_GEBERAL1_SUM_ECC_1_SINGLE_ERROR_E;
                }
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_TX_FIFO_1_GEBERAL1_SUM_ECC_0_DOUBLE_ERROR_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_TX_FIFO_1_GEBERAL1_SUM_ECC_1_DOUBLE_ERROR_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_MPPM_ECC_SUM_MPPM_ECC_DOUBLE_ERROR_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_MPPM_ECC_SUM_MPPM_ECC_SINGLE_ERROR_E;
                if (!IS_BOBK_DEV_CETUS_MAC(devNum))
                {
                    evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_ETH_TXFIFO_0_GEN_SUM_ECC_SINGLE_ERROR_E;
                    evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_ETH_TXFIFO_0_GEN_SUM_ECC_DOUBLE_ERROR_E;
                }
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_ETH_TXFIFO_1_GEN_SUM_ECC_SINGLE_ERROR_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_ETH_TXFIFO_1_GEN_SUM_ECC_DOUBLE_ERROR_E;
#endif
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_TCAM_LOGIC_SUM_TCAM_ARRAY_PARITY_ERROR_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_PFC_PARITY_SUM_PFC_PORT_GROUP0_COUNTERS_PARITY_ERR_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_QCN_SUM_CN_BUFFER_FIFO_PARITY_ERR_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_TXQ_DQ_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_BM_GEN1_SUM_CORE_0_VALID_TABLE_PARITY_ERROR_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_BM_GEN1_SUM_CORE_1_VALID_TABLE_PARITY_ERROR_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_BM_GEN1_SUM_CORE_2_VALID_TABLE_PARITY_ERROR_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_BM_GEN1_SUM_CORE_3_VALID_TABLE_PARITY_ERROR_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_BM_GEN1_SUM_CORE_4_VALID_TABLE_PARITY_ERROR_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_BM_GEN1_SUM_CORE_5_VALID_TABLE_PARITY_ERROR_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_IPLR0_DATA_ERROR_E;
                evExtDataArray[index++] = (GT_U32)PRV_CPSS_BOBK_IPLR1_SUM_DATA_ERROR_E;

                *evExtDataSize = index;
            }
            else
            {
                evExtDataArray[0] = (GT_U32)PRV_CPSS_BOBCAT2_CRITICAL_ECC_ERROR_CLEAR_DESC_CRITICAL_ECC_ONE_ERROR_E;
                evExtDataArray[1] = (GT_U32)PRV_CPSS_BOBCAT2_CRITICAL_ECC_ERROR_CLEAR_DESC_CRITICAL_ECC_TWO_ERROR_E;
                evExtDataArray[2] = (GT_U32)PRV_CPSS_BOBCAT2_CRITICAL_ECC_ERROR_INCREMENT_DESC_CRITICAL_ECC_ONE_ERROR_E;
                evExtDataArray[3] = (GT_U32)PRV_CPSS_BOBCAT2_CRITICAL_ECC_ERROR_INCREMENT_DESC_CRITICAL_ECC_TWO_ERROR_E;
                evExtDataArray[4] = (GT_U32)PRV_CPSS_BOBCAT2_TTI_SUM_CRITICAL_ECC_ONE_ERROR_CORRECTED_E;
                evExtDataArray[5] = (GT_U32)PRV_CPSS_BOBCAT2_TTI_SUM_CRITICAL_ECC_TWO_ERROR_DETECTED_E;
                evExtDataArray[6] = (GT_U32)PRV_CPSS_BOBCAT2_TCAM_LOGIC_SUM_TCAM_ARRAY_PARITY_ERROR_E;
                evExtDataArray[7] = (GT_U32)PRV_CPSS_BOBCAT2_IPLR0_DATA_ERROR_E;
                evExtDataArray[8] = (GT_U32)PRV_CPSS_BOBCAT2_HA_SUM_ECC_SINGLE_ERROR_E;
                evExtDataArray[9] = (GT_U32)PRV_CPSS_BOBCAT2_HA_SUM_ECC_DOUBLE_ERROR_E;
                evExtDataArray[10] = (GT_U32)PRV_CPSS_BOBCAT2_MLL_SUM__FILE_ECC_1_ERROR_E;
                evExtDataArray[11] = (GT_U32)PRV_CPSS_BOBCAT2_MLL_SUM__FILE_ECC_2_ERROR_E;
                evExtDataArray[12] = (GT_32)PRV_CPSS_BOBCAT2_IPLR1_SUM_DATA_ERROR_E;
                evExtDataArray[13] = (GT_U32)PRV_CPSS_BOBCAT2_EFT_SUM_DESC_ECC_SINGLE_ERROR_DETECTED_E;
                evExtDataArray[14] = (GT_U32)PRV_CPSS_BOBCAT2_EFT_SUM_DESC_ECC_DOUBLE_ERROR_DETECTED_INT_E;
                evExtDataArray[15] = (GT_U32)PRV_CPSS_BOBCAT2_TXQ_GEN_SUM_TD_CLR_ECC_ONE_ERROR_CORRECTED_E;
                evExtDataArray[16] = (GT_U32)PRV_CPSS_BOBCAT2_TXQ_GEN_SUM_TD_CLR_ECC_TWO_ERROR_DETECTED_E;
                evExtDataArray[17] = (GT_U32)PRV_CPSS_BOBCAT2_PFC_PARITY_SUM_PFC_PORT_GROUP1_COUNTERS_PARITY_ERR_E;
                evExtDataArray[18] = (GT_U32)PRV_CPSS_BOBCAT2_PFC_PARITY_SUM_PFC_PORT_GROUP2_COUNTERS_PARITY_ERR_E;
                evExtDataArray[19] = (GT_U32)PRV_CPSS_BOBCAT2_PFC_PARITY_SUM_PFC_PORT_GROUP3_COUNTERS_PARITY_ERR_E;
                evExtDataArray[20] = (GT_U32)PRV_CPSS_BOBCAT2_PFC_PARITY_SUM_PFC_PORT_GROUP4_COUNTERS_PARITY_ERR_E;
                evExtDataArray[21] = (GT_U32)PRV_CPSS_BOBCAT2_PFC_PARITY_SUM_PFC_PORT_GROUP5_COUNTERS_PARITY_ERR_E;
                evExtDataArray[22] = (GT_U32)PRV_CPSS_BOBCAT2_PFC_PARITY_SUM_PFC_PORT_GROUP6_COUNTERS_PARITY_ERR_E;
                evExtDataArray[23] = (GT_U32)PRV_CPSS_BOBCAT2_PFC_PARITY_SUM_PFC_PORT_GROUP7_COUNTERS_PARITY_ERR_E;
                evExtDataArray[24] = (GT_U32)PRV_CPSS_BOBCAT2_PFC_PARITY_SUM_PFC_PORT_GROUP0_PFC_IND_FIFO_ECC_ERR_E;
                evExtDataArray[25] = (GT_U32)PRV_CPSS_BOBCAT2_PFC_PARITY_SUM_PFC_PORT_GROUP1_PFC_IND_FIFO_ECC_ERR_E;
                evExtDataArray[26] = (GT_U32)PRV_CPSS_BOBCAT2_PFC_PARITY_SUM_PFC_PORT_GROUP2_PFC_IND_FIFO_ECC_ERR_E;
                evExtDataArray[27] = (GT_U32)PRV_CPSS_BOBCAT2_PFC_PARITY_SUM_PFC_PORT_GROUP3_PFC_IND_FIFO_ECC_ERR_E;
                evExtDataArray[28] = (GT_U32)PRV_CPSS_BOBCAT2_PFC_PARITY_SUM_PFC_PORT_GROUP4_PFC_IND_FIFO_ECC_ERR_E;
                evExtDataArray[29] = (GT_U32)PRV_CPSS_BOBCAT2_PFC_PARITY_SUM_PFC_PORT_GROUP5_PFC_IND_FIFO_ECC_ERR_E;
                evExtDataArray[30] = (GT_U32)PRV_CPSS_BOBCAT2_PFC_PARITY_SUM_PFC_PORT_GROUP6_PFC_IND_FIFO_ECC_ERR_E;
                evExtDataArray[31] = (GT_U32)PRV_CPSS_BOBCAT2_PFC_PARITY_SUM_PFC_PORT_GROUP7_PFC_IND_FIFO_ECC_ERR_E;
                evExtDataArray[32] = (GT_U32)PRV_CPSS_BOBCAT2_QCN_SUM_CN_BUFFER_FIFO_PARITY_ERR_E;
                evExtDataArray[33] = (GT_U32)PRV_CPSS_BOBCAT2_QCN_SUM_DESC_ECC_SINGLE_ERROR_DETECTED_E;
                evExtDataArray[34] = (GT_U32)PRV_CPSS_BOBCAT2_QCN_SUM_DESC_ECC_DOUBLE_ERROR_DETECTED_INT_E;
                evExtDataArray[35] = (GT_U32)PRV_CPSS_BOBCAT2_BM_GEN1_SUM_CORE_0_VALID_TABLE_PARITY_ERROR_E;
                evExtDataArray[36] = (GT_U32)PRV_CPSS_BOBCAT2_BM_GEN1_SUM_CORE_1_VALID_TABLE_PARITY_ERROR_E;
                evExtDataArray[37] = (GT_U32)PRV_CPSS_BOBCAT2_BM_GEN1_SUM_CORE_2_VALID_TABLE_PARITY_ERROR_E;
                evExtDataArray[38] = (GT_U32)PRV_CPSS_BOBCAT2_BM_GEN1_SUM_CORE_3_VALID_TABLE_PARITY_ERROR_E;
                evExtDataArray[39] = (GT_U32)PRV_CPSS_BOBCAT2_BM_GEN1_SUM_CORE_4_VALID_TABLE_PARITY_ERROR_E;
                evExtDataArray[40] = (GT_U32)PRV_CPSS_BOBCAT2_BM_GEN1_SUM_CORE_5_VALID_TABLE_PARITY_ERROR_E;
                evExtDataArray[41] = (GT_U32)PRV_CPSS_BOBCAT2_TXDMA_GENERAL_SUM_ECC_SINGLE_ERROR_E;
                evExtDataArray[42] = (GT_U32)PRV_CPSS_BOBCAT2_TXDMA_GENERAL_SUM_ECC_DOUBLE_ERROR_E;
                evExtDataArray[43] = (GT_U32)PRV_CPSS_BOBCAT2_TX_FIFO_GENERAL1_SUM_ECC_0_SINGLE_ERROR_E;
                evExtDataArray[44] = (GT_U32)PRV_CPSS_BOBCAT2_TX_FIFO_GENERAL1_SUM_ECC_1_SINGLE_ERROR_E;
                evExtDataArray[45] = (GT_U32)PRV_CPSS_BOBCAT2_TX_FIFO_GENERAL1_SUM_ECC_0_DOUBLE_ERROR_E;
                evExtDataArray[46] = (GT_U32)PRV_CPSS_BOBCAT2_TX_FIFO_GENERAL1_SUM_ECC_1_DOUBLE_ERROR_E;
                evExtDataArray[47] = (GT_U32)PRV_CPSS_BOBCAT2_MPPM_ECC_SUM_MPPM_ECC_DOUBLE_ERROR_E;
                evExtDataArray[48] = (GT_U32)PRV_CPSS_BOBCAT2_MPPM_ECC_SUM_MPPM_ECC_SINGLE_ERROR_E;
                evExtDataArray[49] = (GT_U32)PRV_CPSS_BOBCAT2_ETH_TXFIFO_GEN_SUM_ECC_SINGLE_ERROR_E;
                evExtDataArray[50] = (GT_U32)PRV_CPSS_BOBCAT2_ETH_TXFIFO_GEN_SUM_ECC_DOUBLE_ERROR_E;
                *evExtDataSize = 51;
            }
            break;

        case CPSS_PP_TQ_TXQ2_FLUSH_PORT_E:
            switch (PRV_CPSS_PP_MAC(devNum)->devFamily)
            {
                case CPSS_PP_FAMILY_DXCH_BOBCAT3_E:
                case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
                    *evExtDataSize = numOfTxQPorts;
                    PRV_TGF_FILL_EXT_DATA_ARRAY(*evExtDataSize);
                    break;
                default:
                    break;
            }
            break;

        case CPSS_PP_PTP_TAI_INCOMING_TRIGGER_E:
        case CPSS_PP_PTP_TAI_GENERATION_E:
            switch (PRV_CPSS_PP_MAC(devNum)->devFamily)
            {
                case CPSS_PP_FAMILY_DXCH_BOBCAT3_E:
                case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
                    PRV_TGF_FILL_EXT_DATA_ARRAY(2);
                    break;
                default:
                    PRV_TGF_FILL_EXT_DATA_ARRAY(1);
                    break;
            }
            break;

        default:
            evExtDataArray[0] = CPSS_PARAM_NOT_USED_CNS;
            break;
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum) && GT_FALSE == prvUtfIsGmCompilation())
    {
        /* per MG */
        mgNumOfUnits = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.mg.sip6MgNumOfUnits;
    }
    else
    {
        mgNumOfUnits = 1;
    }

    isPerMg   = 0;
    isPerTile = 1;
    *expectedCounter = 1;
    isPerMac  = 0;
    switch (event)
    {
        case CPSS_PP_PORT_RX_FIFO_OVERRUN_E:
        case CPSS_PP_PORT_TX_FIFO_UNDERRUN_E:
        case CPSS_PP_PORT_TX_FIFO_OVERRUN_E:
        case CPSS_PP_PORT_LINK_STATUS_CHANGED_E:
            if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                /* !!! NOTE: supported ONLY by the WM simulation !!!  */
                /* we use it to check the tree of CPSS and simulation */
                isPerMac  = 1;
                isPerTile = 0;
            }
            break;

        case CPSS_PP_MISC_PEX_ADDR_UNMAPPED_E:
        case CPSS_PP_MISC_GENXS_READ_DMA_DONE_E:
        case CPSS_PP_EB_AUQ_FULL_E:
        case CPSS_PP_EB_AUQ_PENDING_E:
        case CPSS_PP_EB_AUQ_OVER_E:
        case CPSS_PP_EB_AUQ_ALMOST_FULL_E:
        case CPSS_PP_EB_FUQ_FULL_E:
        case CPSS_PP_EB_FUQ_PENDING_E:
        case CPSS_PP_MISC_TWSI_TIME_OUT_E:
        case CPSS_PP_MISC_TWSI_STATUS_E:
        case CPSS_PP_MISC_ILLEGAL_ADDR_E:
            if(mgNumOfUnits > 1)
            {
                /* this event comes from 4 MGs */
                isPerMg   = 1;
                isPerTile = 0;
            }
            break;

        case CPSS_PP_TQ_SNIFF_DESC_DROP_E:
           isPerTile = 0;/*per DQ*/
           *expectedCounter = numOfTxQDqUnits;
           break;
        case CPSS_PP_PCL_LOOKUP_DATA_ERROR_E:
            isPerTile = 0;/*per pipe*/
            *expectedCounter = 4 * numOfProcessingPipes;
            break;

        case CPSS_PP_PHA_E:
            isPerTile = 0;/*per pipe*/

            if(PRV_CPSS_SIP_6_CHECK_MAC(devNum) &&
               GT_FALSE == prvUtfIsGmCompilation())
            {
                numPerPipe = /* from PHA */
                             /*   PHA_INTERNAL_ERROR_BAD_ADDRESS_ERROR_E,            */
                             /*   PHA_INTERNAL_ERROR_TABLE_ACCESS_OVERLAP_ERROR_E,   */
                             /*   PHA_INTERNAL_ERROR_HEADER_SIZE_VIOLATION_E,        */
                             /*   PHA_INTERNAL_ERROR_PPA_CLOCK_DOWN_VIOLATION_E,     */
                             4 +

                             /* from PPA */
                             0 +

                             /* from PPG */
                             /*   _ERROR_BAD_ADDRESS_ERROR_E                          */
                             /*   _ERROR_TABLE_ACCESS_OVERLAP_ERROR_E                 */
                             2 * PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.phaInfo.numOfPpg +

                             /* from PPN */
                             /*  _HOST_UNMAPPED_ACCESS_E                              */
                             /*  _CORE_UNMAPPED_ACCESS_E                              */
                             /*  _NEAR_EDGE_IMEM_ACCESS_E                             */
                             /*  _DOORBELL_INTERRUPT_E                                */
                             4 *(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.phaInfo.numOfPpn *
                                 PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.phaInfo.numOfPpg);

            }
            else
            {
                numPerPipe = 0;
            }

            *expectedCounter = numPerPipe * numOfProcessingPipes;
            break;

        case CPSS_PP_PCL_LOOKUP_FIFO_FULL_E:
        case CPSS_PP_TTI_CPU_ADDRESS_OUT_OF_RANGE_E:
        case CPSS_PP_EB_MG_ADDR_OUT_OF_RANGE_E:
        case CPSS_PP_EB_SECURITY_BREACH_UPDATE_E:
        case CPSS_PP_OAM_EXCEPTION_TX_PERIOD_E:
        case CPSS_PP_PCL_MG_ADDR_OUT_OF_RANGE_E:
        case CPSS_PP_SCT_RATE_LIMITER_E:
        case CPSS_PP_MAC_SFLOW_E:
        case CPSS_PP_POLICER_ADDR_OUT_OF_MEMORY_E:
        case CPSS_PP_POLICER_DATA_ERR_E:
        case CPSS_PP_POLICER_IPFIX_ALARM_E:
        case CPSS_PP_POLICER_IPFIX_ALARMED_ENTRIES_FIFO_WRAP_AROUND_E:
        case CPSS_PP_POLICER_IPFIX_WRAP_AROUND_E:

        case CPSS_PP_OAM_EXCEPTION_KEEPALIVE_AGING_E:
        case CPSS_PP_OAM_EXCEPTION_EXCESS_KEEPALIVE_E:
        case CPSS_PP_OAM_EXCEPTION_INVALID_KEEPALIVE_E:
        case CPSS_PP_OAM_EXCEPTION_RDI_STATUS_E:
        case CPSS_PP_OAM_EXCEPTION_MEG_LEVEL_E:
        case CPSS_PP_OAM_EXCEPTION_SOURCE_INTERFACE_E:
        case CPSS_PP_OAM_ILLEGAL_ENTRY_INDEX_E:
        case CPSS_PP_TTI_ACCESS_DATA_ERROR_E:
            isPerTile = 0;/*per pipe*/
            *expectedCounter  = numOfProcessingPipes;
            break;
        default:
            if(GT_TRUE == prvCpssDrvEventIsCpuSdmaPortPerQueue(devNum,event))
            {
                isPerTile = 0;/* event per queue */
            }
            break;
    }

    if(isPerTile && PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
    {
        *expectedCounter *= PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
    }

    if(isPerMg && (mgNumOfUnits >= 2))
    {
        *expectedCounter *= mgNumOfUnits;
    }

    if(isPerMac)
    {
        numMacPorts = 0;
        /* sum the number of MAC ports */
        for(i = 0 ; i < CPSS_MAX_PORTS_BMP_NUM_CNS ; i++)
        {
            if(PRV_CPSS_PP_MAC(devNum)->existingPorts.ports[i])
            {
                numMacPorts += prvCpssPpConfigBitmapNumBitsGet(PRV_CPSS_PP_MAC(devNum)->existingPorts.ports[i]);
            }
        }

        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
        {
            /* the cpss and simulation not supports yet the 'cpu netwrok ports' */
            numMacPorts = 64 * PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
        }

        *expectedCounter *= numMacPorts;
    }


}

/**
* @internal prvTgfCheckEventCounters function
* @endinternal
*
* @brief   This function checks event counter with
*         the expected value
* @param[in] eventCounter             - event counter
* @param[in] evExpectedCounter        - expected event counter
*
* @retval GT_TRUE                  - the event counter matches the expected value
* @retval GT_FALSE                 - the event counter doesn't match the expected value
*/
static GT_BOOL prvTgfCheckEventCounters
(
    GT_U32 eventCounter,            /* current value of event counter */
    GT_U32 evExpectedCounter         /* expected value of event counter */
)
{
    if (eventCounter == evExpectedCounter)
        return GT_TRUE;
    else
        return GT_FALSE;
}

/* flag to allow the 'MAC' portNumFrom that was not mapped to 'physical port' ,
   but still we want to get indication about it
*/
extern void drvEventExtDataConvert_allowNonMappedMac(IN GT_U32 allow);

/**
* @internal prvTgfGenEvent function
* @endinternal
*
* @brief   The body of prvTgfEventDeviceGenerate test
*/
GT_VOID prvTgfGenEvent(GT_VOID)
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
    static GT_U32 evExtDataArray[576];     /* array of evExtData values */
    GT_U32 evExtDataSize;
    GT_U32 evExpectedCounter;
    GT_U32  iterator,lastValue;

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

    /* enable the support for MACs that are not mapped */
    drvEventExtDataConvert_allowNonMappedMac(1);

    /* Do the generation of all supported events */
    for (eventItr = (GT_U32)CPSS_PP_UNI_EV_MIN_E;
            eventItr < (GT_U32)CPSS_PP_UNI_EV_MAX_E;
            eventItr++)
    {
        if (PRV_CPSS_SIP_5_10_CHECK_MAC(prvTgfDevNum))
        {
            /* BC2 B0 does not support GPP events */
            if (eventItr == (GT_U32)CPSS_PP_GPP_E)
                continue;
        }

        if ((eventItr == (GT_U32)CPSS_PP_PTP_TAI_INCOMING_TRIGGER_E) ||
            (eventItr == (GT_U32)CPSS_PP_PTP_TAI_GENERATION_E))
        {
#ifndef ASIC_SIMULATION
            if (PRV_CPSS_SIP_5_15_CHECK_MAC(prvTgfDevNum) == GT_FALSE)
            {
                /* TAI interrupts supported starting from BobK */
                continue;
            }
#else
            /* Simulation of TAI interrupts is not supported */
            continue;
#endif
        }

        /* Initialize skipEvent flag */
        skipEvent = GT_FALSE;
        /* Skip unsupported events */
        PRV_TGF_SKIP_EVENTS(unsupportedEvents, unsupportedEventsSize);

        event = (CPSS_UNI_EV_CAUSE_ENT)eventItr;

        /* get array of extData for current event */
        prvTgfBobcat2InitEvExtDataArray(prvTgfDevNum, event, evExtDataArray,
                                        &evExtDataSize, &evExpectedCounter);

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

            cpssOsTimerWkAfter(15);

            counter = 0;
            iterator = 0;
            do{
                lastValue = counter;
                /* get events counter */
                rc=utfGenEventCounterGet(prvTgfDevNum,event,GT_FALSE,&counter);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoGenEventCounterGet");

                if(rc1 != GT_OK)
                {
                    /* no sleep needed */
                    break;
                }

                if(counter < evExpectedCounter)
                {
                    cpssOsTimerWkAfter(20);
                }
                else
                {
                    break;
                }

            }
            while(((counter == 0) || (lastValue < counter)) && (iterator++ < evExpectedCounter));/* counter keep on growing */

            /*cpssOsPrintf("Event: %s, counter: %d\n", uniEvName[eventItr], counter);*/

            if (prvTgfCheckEventCounters(counter, evExpectedCounter) == GT_FALSE &&
                    rc1 != GT_NOT_SUPPORTED &&
                    rc1 != GT_NOT_FOUND)
            {
                /*Set Fail Status if event counter is 0*/
                UTF_VERIFY_EQUAL5_STRING_MAC(
                    GT_TRUE,GT_FALSE, "\nFailed to generate event %s[%d] index[%d] ,counter [%d], evExpectedCounter [%d] \n",
                    uniEvName[eventItr], evExtDataArray[evExtDataItr] ,evExtDataItr, counter, evExpectedCounter);

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

    /* restore (disable) the support for MACs that are not mapped */
    drvEventExtDataConvert_allowNonMappedMac(0);

    /* Mask events disabled by default */
    rc = appDemoEventsToTestsHandlerBind(disabledEvents, disabledEventsSize, CPSS_EVENT_MASK_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoEventsToTestsHandlerBind");

    /* Print all failed events */
    prvTgfPrintFailedEvents(failedEvents, failedEventsCnt, uniEvName, overflow);
    return;
}

static GT_U32 prvTgPortEventReceptorLastEvExtData = 0xFFFFFFFF;
static CPSS_OS_SIG_SEM prvTgPortEventReceivedSem = CPSS_OS_SEMB_EMPTY_E;
static CPSS_OS_SIG_SEM prvTgPortEventReceivedAckSem = CPSS_OS_SEMB_FULL_E;

static GT_VOID prvTgPortEventsReceptor
(
        IN GT_U8    devNum,
        IN GT_U32   uniEv,
        IN GT_U32   evExtData
)
{
    if (CPSS_PP_PORT_LINK_STATUS_CHANGED_E == uniEv) {
        PRV_UTF_LOG2_MAC("SIGNAL STATE CHANGE for dev=%d port=%d\n", devNum, evExtData);
        cpssOsSigSemWait(prvTgPortEventReceivedAckSem, 10); /* wait but not a long time */
        prvTgPortEventReceptorLastEvExtData = evExtData;
        cpssOsSigSemSignal(prvTgPortEventReceivedSem); /* wake thread waiting for evt */
    } else {
        /* Do nothing for other events */
    }
}


static GT_STATUS prvTgfPhyToMacPortMappingWaitForEventOnPort(GT_PHYSICAL_PORT_NUM phyPort)
{
    GT_STATUS st;

    while (1)
    {
        st = cpssOsSigSemWait(prvTgPortEventReceivedSem, 5000); /* wait 5 sec */
        UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, st);
        if (GT_OK != st)
        {
            break;
        }
        if (phyPort != prvTgPortEventReceptorLastEvExtData) /* compare received value with expected value*/
        {
            cpssOsSigSemSignal(prvTgPortEventReceivedAckSem); /* Acknowledge reception - not expected value */
            continue; /* wait till correct port number arrives */
        } else
        {
            prvTgPortEventReceptorLastEvExtData = 0xFFFFFFFF; /* reset to known BAD value */
            cpssOsSigSemSignal(prvTgPortEventReceivedAckSem); /* Acknowledge reception - expected value */
            break;
        }
    }

    return st;
}

static GT_STATUS prvTgfPhyToMacPortMappingInit(GT_U8 devNum, int *portCnt, CPSS_DXCH_PORT_MAP_STC **portArray)
{
    GT_STATUS st = GT_FALSE;
    GT_U32    phyPortsMaxNum;
    GT_PHYSICAL_PORT_NUM        currentPortMap;
    GT_U32                      idx = 0;
    GT_BOOL                     isValid;
    GT_BOOL                     isExtendedCascadePort;
    CPSS_DXCH_PORT_MAP_STC      *portMapArray = NULL;
    CPSS_PORT_INTERFACE_MODE_ENT phyPortIfConfig;
    CPSS_PORT_SPEED_ENT         phyPortSpeed;

    st = cpssDxChCfgTableNumEntriesGet(devNum,CPSS_DXCH_CFG_TABLE_PHYSICAL_PORT_E, &phyPortsMaxNum);
    PRV_UTF_LOG1_MAC("total valid ports %d\n",phyPortsMaxNum);
    portMapArray = (CPSS_DXCH_PORT_MAP_STC  *)cpssOsMalloc(phyPortsMaxNum * sizeof(CPSS_DXCH_PORT_MAP_STC));
    if (NULL == portMapArray)
    {
            PRV_UTF_LOG0_DEBUG_MAC("no mem\n");
            st = GT_FAIL;
            goto exit_freemem;
    }

    currentPortMap = 0;
    while (currentPortMap < phyPortsMaxNum)
    {
        st = cpssDxChPortPhysicalPortMapIsValidGet(devNum, currentPortMap, &isValid);
        if((GT_OK != st) || (isValid != GT_TRUE))
        {
            currentPortMap++;
            continue;
        }
        isExtendedCascadePort = PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[currentPortMap].portMap.isExtendedCascadePort;
        if(isExtendedCascadePort != GT_FALSE)
        {
            /* Skip extended cascade ports */
            currentPortMap++;
            continue;
        }
        st = cpssDxChPortPhysicalPortMapGet(devNum, currentPortMap, 1, &portMapArray[idx]);
        UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, st);
        if(GT_OK != st)
        {
            goto exit_freemem;
        }

        if (GT_TRUE == portMapArray[idx].tmEnable  && (GT_32)portMapArray[idx].tmPortInd < -1)
        {
            portMapArray[idx].tmPortInd =  (GT_U32) -1; /* Get often return junk here set to -1 if its < -1 */
        }

        if (CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E !=  portMapArray[idx].mappingType)
        {
            currentPortMap++;
            continue;
        }

        st = cpssDxChPortInterfaceModeGet(devNum, currentPortMap, &phyPortIfConfig);
        UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, st);
        if(GT_OK != st)
        {
            goto exit_freemem;
        }

        st = cpssDxChPortSpeedGet(devNum , currentPortMap, &phyPortSpeed);
        UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, st);
        if(GT_OK != st)
        {
            goto exit_freemem;
        }
        if((CPSS_PORT_INTERFACE_MODE_NA_E == phyPortIfConfig) ||
                (CPSS_PORT_SPEED_NA_E == phyPortSpeed)) {
            currentPortMap++;
            continue; /* SKIP port with not initialized speed and/or mode */
        }

        PRV_UTF_LOG1_MAC("disable PASS for port=%d\n",currentPortMap);
        st = cpssDxChPortForceLinkPassEnableSet(devNum, currentPortMap, GT_FALSE );
        if(GT_OK != st)
        {
            goto exit_freemem;
        }
        PRV_UTF_LOG1_MAC("disable DOWN for port=%d\n",currentPortMap);
        st = cpssDxChPortForceLinkDownEnableSet(devNum, currentPortMap, GT_FALSE );
        if(GT_OK != st)
        {
            goto exit_freemem;
        }

        currentPortMap++;
        idx++;
    }

    /* provide time for events handlers after Force Link change manipulations.*/
    cpssOsTimerWkAfter(1000);

    st = cpssOsSigSemBinCreate("utfPortMapEvSem", CPSS_OS_SEMB_EMPTY_E, &prvTgPortEventReceivedSem);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, st);
    if (GT_OK != st)
    {
        goto exit_freemem;
    }

    st = cpssOsSigSemBinCreate("utfPortMapEvAckSem", CPSS_OS_SEMB_FULL_E, &prvTgPortEventReceivedAckSem);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, st);
    if (GT_OK != st)
    {
        (void)cpssOsSigSemDelete(prvTgPortEventReceivedSem);
        goto exit_freemem;
    }

    *portArray = portMapArray;
    *portCnt = idx;

    notifyEventArrivedFunc = prvTgPortEventsReceptor;   /* set cbk  */

    return st;   /* Normal return */

exit_freemem:
    if (portMapArray)
    {
        cpssOsFree(portMapArray);
    }

    return st;
}

static GT_STATUS prvTgfPhyToMacPortMappingProcess(GT_U8 devNum, int portCnt, CPSS_DXCH_PORT_MAP_STC *portArray)
{
    GT_STATUS                   st = GT_FAIL;
    GT_PHYSICAL_PORT_NUM        currentPort;
    int                         i;
    GT_BOOL                     islinkisUp;

    for (i= 0; i< portCnt; i++)
    {
        currentPort = portArray[i].physicalPortNumber;

        PRV_UTF_LOG0_MAC("----------------------------------------\n");
        st = cpssDxChPortLinkStatusGet(devNum, currentPort, &islinkisUp );
        UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, st);
        if (GT_OK != st)
        {
            PRV_UTF_LOG2_MAC("error at LINE %d %s\n", __LINE__ , __FILE__);
            goto unset_ev_cbk;
        }

        if (GT_FALSE == islinkisUp)
        {
            PRV_UTF_LOG1_MAC("link is DOWN  FORCE port=%d to UP\n", currentPort);
            st = cpssDxChPortForceLinkPassEnableSet(devNum, currentPort, GT_TRUE );
            UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, st);
            if (GT_OK != st)
            {
                PRV_UTF_LOG2_MAC("error at LINE %d %s\n", __LINE__ , __FILE__);
                goto unset_ev_cbk;
            }
        }
        else
        {
            PRV_UTF_LOG1_MAC("link is UP    FORCE port=%d to DOWN\n", currentPort);
            st = cpssDxChPortForceLinkDownEnableSet(devNum, currentPort, GT_TRUE );
            UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, st);
            if (GT_OK != st)
            {
                PRV_UTF_LOG2_MAC("error at LINE %d %s\n", __LINE__ , __FILE__);
                goto unset_ev_cbk;
            }
        }
        /*PRV_UTF_LOG2_MAC("wait evt1 LINE %d %s\n", __LINE__ , __FILE__);*/
        st = prvTgfPhyToMacPortMappingWaitForEventOnPort(currentPort);
        if (GT_OK != st)
        {
            UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, st);
            PRV_UTF_LOG2_MAC("error at LINE %d %s\n", __LINE__ , __FILE__);
            goto unset_ev_cbk;
        }

        /* Now disable link state bypass  and check again that is back */
        if (GT_FALSE == islinkisUp)
        {
            PRV_UTF_LOG1_MAC("link was DOWN  reset back port=%d to DOWN\n", currentPort);
            st = cpssDxChPortForceLinkPassEnableSet(devNum, currentPort, GT_FALSE );
            UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, st);
            if (GT_OK != st)
            {
                PRV_UTF_LOG2_MAC("error at LINE %d %s\n", __LINE__ , __FILE__);
                goto unset_ev_cbk;
            }
        }
        else
        {
            PRV_UTF_LOG1_MAC("link was UP  reset back port=%d to UP\n", currentPort);
            st = cpssDxChPortForceLinkDownEnableSet(devNum, currentPort, GT_FALSE );
            UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, st);
            if (GT_OK != st)
            {
                PRV_UTF_LOG2_MAC("error at LINE %d %s\n", __LINE__ , __FILE__);
                goto unset_ev_cbk;
            }
        }

        st = prvTgfPhyToMacPortMappingWaitForEventOnPort(currentPort);
        if (GT_OK != st)
        {
            UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, st);
            PRV_UTF_LOG2_MAC("error at LINE %d %s\n", __LINE__ , __FILE__);
            goto unset_ev_cbk;
        }

    }

unset_ev_cbk:

    return st;

}

static GT_VOID prvTgfPhyToMacPortMappingCleanup(CPSS_DXCH_PORT_MAP_STC *portArray)
{
    GT_STATUS st;

    notifyEventArrivedFunc = NULL; /* remove cbk after use */

    st = cpssOsSigSemDelete(prvTgPortEventReceivedSem);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
    st = cpssOsSigSemDelete(prvTgPortEventReceivedAckSem);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
    if (portArray)
    {
        cpssOsFree(portArray);
    }
    return;
}

GT_VOID prvTgfEventPortLinkStatusUnbindBind(GT_VOID)
{
    GT_STATUS rc;
    CPSS_UNI_EV_CAUSE_ENT evHndlrCauseAllArr[1] = {CPSS_PP_PORT_LINK_STATUS_CHANGED_E};

    PRV_CPSS_DRV_EVENT_HNDL_STC  *hnd;
    GT_U32      eventCount;
    GT_U32      i;

    /* get CPSS_PP_PORT_LINK_STATUS_CHANGED_E event handler */
    hnd = prvCpssDrvEvReqQUserHndlGet(CPSS_PP_PORT_LINK_STATUS_CHANGED_E);
    if (hnd == NULL)
    {
        /* should not happen */
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, GT_FAIL);
        return;
    }

    /* wait event list is clear */
    i = 2000;
    eventCount = prvCpssDrvEvReqQBitmapGet((GT_UINTPTR)hnd, NULL, 0);
    while ((eventCount > 0) &&
            (--i > 0))
    {
        cpssOsTimerWkAfter(1);
        eventCount = prvCpssDrvEvReqQBitmapGet((GT_UINTPTR)hnd, NULL, 0);
    }
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, eventCount);

    /* unbind CPSS_PP_PORT_LINK_STATUS_CHANGED_E event */
    rc = cpssEventUnBind(evHndlrCauseAllArr, 1);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* clear event list  */
    hnd->evListPtr = NULL;

    /* bind CPSS_PP_PORT_LINK_STATUS_CHANGED_E event to the handler */
    rc = prvCpssDrvEvReqQUserHndlSet(CPSS_PP_PORT_LINK_STATUS_CHANGED_E, hnd);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
}

GT_VOID prvTgfPhyToMacPortMapping(GT_VOID)
{
    GT_STATUS                   st, st1;
    GT_U8                       devNum   = 0;
    CPSS_DXCH_PORT_MAP_STC      *portMapArray = NULL;
    int                         portMapArrayCnt = 0;

    st = prvTgfPhyToMacPortMappingInit(devNum, &portMapArrayCnt, &portMapArray );
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
    PRV_UTF_LOG2_MAC("cnt=%d ptr=%p\n",portMapArrayCnt, portMapArray);

    /* Event handler executes SERDES training procedures on Link UP.
       Need to disable it to avoid failure of link event. */
    appDemoDbEntryAdd("serdesTraining", 0);

    /* make sure that event is opened */
    st1 = cpssEventDeviceMaskSet(devNum, CPSS_PP_PORT_LINK_STATUS_CHANGED_E, CPSS_EVENT_UNMASK_E);
    cpssOsTimerWkAfter(1000);

    st = prvTgfPhyToMacPortMappingProcess(devNum, portMapArrayCnt, portMapArray );

    prvTgfPhyToMacPortMappingCleanup(portMapArray);

    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st1,
        "cpssEventDeviceMaskWithEvExtDataSet");
}

