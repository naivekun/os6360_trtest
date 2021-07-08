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
* @file prvCpssGenEvReq.h
*
* @brief Includes unified CPSS private event routine.
* this file is used to define bind between the cpssDriver of PP to
* unify events , and for FA and for XBAR
*
* @version   20
********************************************************************************
*/
#ifndef __prvCpssGenEvReqh
#define __prvCpssGenEvReqh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/extServices/os/gtOs/gtGenTypes.h>
#include <cpss/generic/events/cpssGenEventUnifyTypes.h>
#include <cpss/generic/events/cpssGenEventCtrl.h>
#include <cpss/generic/events/cpssGenEventRequests.h>

/**
* @enum PRV_CPSS_DEVICE_TYPE_ENT
 *
 * @brief Enumeration of types of devices PP/FA/XBAR/DRAGONITE
 * PRV_CPSS_DEVICE_TYPE_PP_E - PP device
 * PRV_CPSS_DEVICE_TYPE_FA_E - FA device
 * PRV_CPSS_DEVICE_TYPE_XBAR_E - XBAR device
 * PRV_CPSS_DEVICE_TYPE_DRAGONITE_E - DRAGONITE device
*/
typedef enum{

    PRV_CPSS_DEVICE_TYPE_PP_E,

    PRV_CPSS_DEVICE_TYPE_FA_E,

    PRV_CPSS_DEVICE_TYPE_XBAR_E,

    PRV_CPSS_DEVICE_TYPE_DRAGONITE_E,

    PRV_CPSS_DEVICE_TYPE_LAST_E

} PRV_CPSS_DEVICE_TYPE_ENT;

/**
* @enum PRV_CPSS_EV_CONVERT_DIRECTION_ENT
 *
 * @brief Enumeration of convert the 'extData' according to direction:
 *      PRV_CPSS_EV_CONVERT_DIRECTION_UNI_EVENT_TO_HW_INTERRUPT_E -
 *              direction 'uniEvent to HW interrupt'
 *      PRV_CPSS_EV_CONVERT_DIRECTION_HW_INTERRUPT_TO_UNI_EVENT_E
 *              direction 'HW interrupt to uniEvent'
 *
*/
typedef enum{

    PRV_CPSS_EV_CONVERT_DIRECTION_UNI_EVENT_TO_HW_INTERRUPT_E,
    PRV_CPSS_EV_CONVERT_DIRECTION_HW_INTERRUPT_TO_UNI_EVENT_E,

    PRV_CPSS_EV_CONVERT_TYPE_LAST

} PRV_CPSS_EV_CONVERT_DIRECTION_ENT;

#define STR(strname)    \
    #strname

#define UNI_EV_NAME                                                             \
    /* PCI */                                                                   \
STR(CPSS_PP_SLV_WRITE_ERR_E),  /* 0  */                                         \
STR(CPSS_PP_MAS_WRITE_ERR_E),  /* 1  */                                         \
STR(CPSS_PP_ADDR_ERR_E),  /* 2  */                                              \
STR(CPSS_PP_MAS_ABORT_E),  /* 3  */                                             \
STR(CPSS_PP_TARGET_ABORT_E),  /* 4  */                                          \
STR(CPSS_PP_SLV_READ_ERR_E),  /* 5  */                                          \
STR(CPSS_PP_MAS_READ_ERR_E),  /* 6  */                                          \
STR(CPSS_PP_RETRY_CNTR_E),  /* 7  */                                            \
                                                                                \
    /* Misc */                                                                  \
STR(CPSS_PP_MISC_TWSI_TIME_OUT_E),  /* 8  */                                    \
STR(CPSS_PP_MISC_TWSI_STATUS_E),  /* 9  */                                      \
STR(CPSS_PP_MISC_ILLEGAL_ADDR_E),  /* 10 */                                     \
STR(CPSS_PP_MISC_CPU_PORT_RX_OVERRUN_E),  /* 11 */                              \
STR(CPSS_PP_MISC_CPU_PORT_TX_OVERRUN_E),  /* 12 */                              \
STR(CPSS_PP_MISC_TX_CRC_PORT_E),  /* 13 */                                      \
STR(CPSS_PP_MISC_C2C_W_FAR_END_UP_E),  /* 14 */                                 \
STR(CPSS_PP_MISC_C2C_N_FAR_END_UP_E),  /* 15 */                                 \
STR(CPSS_PP_MISC_C2C_DATA_ERR_E),  /* 16 */                                     \
STR(CPSS_PP_MISC_MSG_TIME_OUT_E),  /* 17 */                                     \
STR(CPSS_PP_MISC_UPDATED_STAT_E),  /* 18 */                                     \
STR(CPSS_PP_GPP_E),  /* 19 */                                                   \
STR(CPSS_PP_MISC_UPLINK_W_ECC_ON_DATA_E),  /* 20 */                             \
STR(CPSS_PP_MISC_UPLINK_W_ECC_ON_HEADER_E),  /* 21 */                           \
STR(CPSS_PP_MISC_UPLINK_N_ECC_ON_DATA_E),  /* 22 */                             \
STR(CPSS_PP_MISC_UPLINK_N_ECC_ON_HEADER_E),  /* 23 */                           \
STR(CPSS_PP_MISC_PEX_ADDR_UNMAPPED_E),  /* 24 */                                \
STR(CPSS_PP_MISC_GENXS_READ_DMA_DONE_E),  /* 25 */                              \
                                                                                \
    /* Ingress Buffer Manager */                                                \
STR(CPSS_PP_BM_EMPTY_CLEAR_E),  /* 26 */                                        \
STR(CPSS_PP_BM_EMPTY_INC_E),  /* 27 */                                          \
STR(CPSS_PP_BM_AGED_PACKET_E),  /* 28 */                                        \
STR(CPSS_PP_BM_MAX_BUFF_REACHED_E),  /* 29 */                                   \
STR(CPSS_PP_BM_PORT_RX_BUFFERS_CNT_UNDERRUN_E),  /* 30 */                       \
STR(CPSS_PP_BM_PORT_RX_BUFFERS_CNT_OVERRUN_E),  /* 31 */                        \
STR(CPSS_PP_BM_INVALID_ADDRESS_E),  /* 32 */                                    \
STR(CPSS_PP_BM_WRONG_SRC_PORT_E),  /* 33 */                                     \
STR(CPSS_PP_BM_MC_INC_OVERFLOW_E),  /* 34 */                                    \
STR(CPSS_PP_BM_MC_INC_UNDERRUN_E),  /* 35 */                                    \
STR(CPSS_PP_BM_TQ_PARITY_ERROR_PORT_E),  /* 36 */                               \
STR(CPSS_PP_BM_RX_MEM_READ_ECC_ERROR_E),  /* 37 */                              \
STR(CPSS_PP_BM_VLT_ECC_ERR_E),  /* 38 */                                        \
STR(CPSS_PP_BM_MISC_E),  /* 39 */                                               \
STR(CPSS_PP_BM_TRIGGER_AGING_DONE_E),  /* 40 */                                 \
STR(CPSS_PP_BM_PORT_RX_FULL_E),  /* 41 */                                       \
STR(CPSS_PP_BM_ALL_GIGA_PORTS_BUFF_LIMIT_REACHED_E),  /* 42 */                  \
STR(CPSS_PP_BM_ALL_HG_STACK_PORTS_BUFF_LIMIT_REACHED_E),  /* 43 */              \
                                                                                \
    /* Port */                                                                  \
STR(CPSS_PP_PORT_LINK_STATUS_CHANGED_E),  /* 44 */                              \
STR(CPSS_PP_PORT_AN_COMPLETED_E),  /* 45 */                                     \
STR(CPSS_PP_PORT_RX_FIFO_OVERRUN_E),  /* 46 */                                  \
STR(CPSS_PP_PORT_TX_FIFO_UNDERRUN_E),  /* 47 */                                 \
STR(CPSS_PP_PORT_TX_FIFO_OVERRUN_E),  /* 48 */                                  \
STR(CPSS_PP_PORT_TX_UNDERRUN_E),  /* 49 */                                      \
STR(CPSS_PP_PORT_ADDRESS_OUT_OF_RANGE_E),  /* 50 */                             \
STR(CPSS_PP_PORT_PRBS_ERROR_E),  /* 51 */                                       \
STR(CPSS_PP_PORT_SYNC_STATUS_CHANGED_E),  /* 52 */                              \
STR(CPSS_PP_PORT_TX_CRC_ERROR_E),  /* 53 */                                     \
STR(CPSS_PP_PORT_ILLEGAL_SEQUENCE_E),  /* 54 */                                 \
STR(CPSS_PP_PORT_IPG_TOO_SMALL_E),  /* 55 */                                    \
STR(CPSS_PP_PORT_FAULT_TYPE_CHANGE_E),  /* 56 */                                \
STR(CPSS_PP_PORT_FC_STATUS_CHANGED_E),  /* 57 */                                \
STR(CPSS_PP_PORT_CONSECUTIVE_TERM_CODE_E),  /* 58 */                            \
                                                                                \
STR(CPSS_PP_INTERNAL_PHY_E),  /* 59 */                                          \
STR(CPSS_PP_PORT_NO_BUFF_PACKET_DROP_E),  /* 60 */                              \
STR(CPSS_PP_PORT_XAUI_PHY_E),  /* 61*/                                          \
STR(CPSS_PP_PORT_COUNT_COPY_DONE_E),  /* 62*/                                   \
STR(CPSS_PP_PORT_COUNT_EXPIRED_E),  /* 63 */                                    \
                                                                                \
    /* Group Of Ports */                                                        \
STR(CPSS_PP_GOP_ADDRESS_OUT_OF_RANGE_E),  /* 64 */                              \
STR(CPSS_PP_GOP_COUNT_EXPIRED_E),  /* 65 */                                     \
STR(CPSS_PP_GOP_COUNT_COPY_DONE_E),  /* 66 */                                   \
STR(CPSS_PP_GOP_GIG_BAD_FC_PACKET_GOOD_CRC_E),  /* 67 */                        \
STR(CPSS_PP_GOP_XG_BAD_FC_PACKET_GOOD_CRC_E),  /* 68 */                         \
STR(CPSS_PP_XSMI_WRITE_DONE_E),  /* 69 */                                       \
                                                                                \
    /* Tx Queue */                                                              \
STR(CPSS_PP_TQ_WATCHDOG_EX_PORT_E),  /* 70 */                                   \
STR(CPSS_PP_TQ_TXQ2_FLUSH_PORT_E),  /* 71 */                                    \
STR(CPSS_PP_TQ_TXQ2_MG_FLUSH_E),  /* 72 */                                      \
STR(CPSS_PP_TQ_ONE_ECC_ERROR_E),  /* 73 */                                      \
STR(CPSS_PP_TQ_TWO_ECC_ERROR_E),  /* 74 */                                      \
STR(CPSS_PP_TQ_MG_READ_ERR_E),  /* 75 */                                        \
STR(CPSS_PP_TQ_HOL_REACHED_PORT_E),  /* 76 */                                   \
STR(CPSS_PP_TQ_RED_REACHED_PORT_E),  /* 77 */                                   \
STR(CPSS_PP_TQ_TOTAL_DESC_UNDERFLOW_E),  /* 78 */                               \
STR(CPSS_PP_TQ_TOTAL_DESC_OVERFLOW_E),  /* 79 */                                \
STR(CPSS_PP_TQ_TOTAL_BUFF_UNDERFLOW_E),  /* 80 */                               \
STR(CPSS_PP_TQ_SNIFF_DESC_DROP_E),  /* 81 */                                    \
STR(CPSS_PP_TQ_MLL_PARITY_ERR_E),  /* 82 */                                     \
STR(CPSS_PP_TQ_MC_FIFO_OVERRUN_E),  /* 83 */                                    \
STR(CPSS_PP_TQ_MISC_E),  /* 84 */                                               \
STR(CPSS_PP_TQ_PORT_DESC_FULL_E),  /* 85 */                                     \
STR(CPSS_PP_TQ_MC_DESC_FULL_E),  /* 86 */                                       \
STR(CPSS_PP_TQ_GIGA_FIFO_FULL_E),  /* 87 */                                     \
STR(CPSS_PP_TQ_XG_MC_FIFO_FULL_E),  /* 88 */                                    \
STR(CPSS_PP_TQ_PORT_FULL_XG_E),  /* 89 */                                       \
                                                                                \
    /* Ethernet Bridge */                                                       \
STR(CPSS_PP_EB_AUQ_FULL_E),  /*  90 */                                          \
STR(CPSS_PP_EB_AUQ_PENDING_E),  /*  91 */                                       \
STR(CPSS_PP_EB_AUQ_OVER_E),  /*  92 */                                          \
STR(CPSS_PP_EB_AUQ_ALMOST_FULL_E),  /*  93 */                                   \
STR(CPSS_PP_EB_FUQ_FULL_E),  /*  94 */                                          \
STR(CPSS_PP_EB_FUQ_PENDING_E),  /*  95 */                                       \
STR(CPSS_PP_EB_NA_FIFO_FULL_E),  /*  96 */                                      \
STR(CPSS_PP_EB_MG_ADDR_OUT_OF_RANGE_E),  /*  97 */                              \
STR(CPSS_PP_EB_VLAN_TBL_OP_DONE_E),  /*  98 */                                  \
STR(CPSS_PP_EB_SECURITY_BREACH_UPDATE_E),  /*  99 */                            \
STR(CPSS_PP_EB_VLAN_SECURITY_BREACH_E),  /* 100 */                              \
STR(CPSS_PP_EB_NA_NOT_LEARNED_SECURITY_BREACH_E),  /* 101 */                    \
STR(CPSS_PP_EB_SA_MSG_DISCARDED_E),  /* 102 */                                  \
STR(CPSS_PP_EB_QA_MSG_DISCARDED_E),  /* 103 */                                  \
STR(CPSS_PP_EB_SA_DROP_SECURITY_BREACH_E),  /* 104 */                           \
STR(CPSS_PP_EB_DA_DROP_SECURITY_BREACH_E),  /* 105 */                           \
STR(CPSS_PP_EB_DA_SA_DROP_SECURITY_BREACH_E),  /* 106 */                        \
STR(CPSS_PP_EB_NA_ON_LOCKED_DROP_SECURITY_BREACH_E),  /* 107 */                 \
STR(CPSS_PP_EB_MAC_RANGE_DROP_SECURITY_BREACH_E),  /* 108 */                    \
STR(CPSS_PP_EB_INVALID_SA_DROP_SECURITY_BREACH_E),  /* 109 */                   \
STR(CPSS_PP_EB_VLAN_NOT_VALID_DROP_SECURITY_BREACH_E),  /* 110 */               \
STR(CPSS_PP_EB_VLAN_NOT_MEMBER_DROP_DROP_SECURITY_BREACH_E),  /* 111 */         \
STR(CPSS_PP_EB_VLAN_RANGE_DROP_SECURITY_BREACH_E),  /* 112 */                   \
STR(CPSS_PP_EB_INGRESS_FILTER_PCKT_E),  /* 113 */                               \
STR(CPSS_PP_MAC_TBL_READ_ECC_ERR_E),  /* 114 */                                 \
STR(CPSS_PP_MAC_SFLOW_E),  /* 115 */                                            \
STR(CPSS_PP_MAC_NUM_OF_HOP_EXP_E),  /* 116 */                                   \
STR(CPSS_PP_MAC_NA_LEARNED_E),  /* 117 */                                       \
STR(CPSS_PP_MAC_NA_NOT_LEARNED_E),  /* 118 */                                   \
STR(CPSS_PP_MAC_AGE_VIA_TRIGGER_ENDED_E),  /* 119 */                            \
STR(CPSS_PP_MAC_MG_ADDR_OUT_OF_RANGE_E),  /* 120 */                             \
STR(CPSS_PP_MAC_UPDATE_FROM_CPU_DONE_E),  /* 121 */                             \
STR(CPSS_PP_MAC_MESSAGE_TO_CPU_READY_E),  /* 122 */                             \
STR(CPSS_PP_MAC_NA_SELF_LEARNED_E),  /* 123 */                                  \
STR(CPSS_PP_MAC_NA_FROM_CPU_LEARNED_E),  /* 124 */                              \
STR(CPSS_PP_MAC_NA_FROM_CPU_DROPPED_E),  /* 125 */                              \
STR(CPSS_PP_MAC_AGED_OUT_E),  /* 126 */                                         \
STR(CPSS_PP_MAC_FIFO_2_CPU_EXCEEDED_E),  /* 127 */                              \
STR(CPSS_PP_MAC_1ECC_ERRORS_E),  /* 128 */                                      \
STR(CPSS_PP_MAC_2ECC_ERRORS_E),  /* 129 */                                      \
STR(CPSS_PP_MAC_TBL_OP_DONE_E),  /* 130 */                                      \
STR(CPSS_PP_MAC_PACKET_RSSI_LESS_THAN_THRESH_E),  /* 131 */                     \
STR(CPSS_PP_MAC_UPD_AVG_RSSI_LESS_THAN_THRESH_E),  /* 132 */                    \
STR(CPSS_PP_MAC_STG_TBL_DATA_ERROR_E),  /* 133 */                               \
STR(CPSS_PP_MAC_VIDX_TBL_DATA_ERROR_E),  /* 134*/                               \
STR(CPSS_PP_LX_LB_ERR_E),  /* 135 */                                            \
STR(CPSS_PP_LX_CTRL_MEM_2_RF_ERR_E),  /* 136 */                                 \
STR(CPSS_PP_LX_TCB_CNTR_E),  /* 137 */                                          \
STR(CPSS_PP_LX_IPV4_MC_ERR_E),  /* 138 */                                       \
STR(CPSS_PP_LX_IPV4_LPM_ERR_E),  /* 139 */                                      \
STR(CPSS_PP_LX_IPV4_ROUTE_ERR_E),  /* 140 */                                    \
STR(CPSS_PP_LX_IPV4_CNTR_E),  /* 141 */                                         \
STR(CPSS_PP_LX_L3_L7_ERR_ADDR_E),  /* 142 */                                    \
STR(CPSS_PP_LX_TRUNK_ADDR_OUT_OF_RANGE_E),  /* 143 */                           \
STR(CPSS_PP_LX_IPV4_REFRESH_AGE_OVERRUN_E),  /* 144 */                          \
STR(CPSS_PP_LX_PCE_PAR_ERR_E),  /* 145 */                                       \
STR(CPSS_PP_LX_TC_2_RF_CNTR_ALRM_E),  /* 146 */                                 \
STR(CPSS_PP_LX_TC_2_RF_PLC_ALRM_E),  /* 147 */                                  \
STR(CPSS_PP_LX_TC_2_RF_TBL_ERR_E),  /* 148 */                                   \
STR(CPSS_PP_LX_CLASSIFIER_HASH_PAR_ERR_E),  /* 149 */                           \
STR(CPSS_PP_LX_FLOW_LKUP_PAR_ERR_E),  /* 150 */                                 \
STR(CPSS_PP_LX_FLOW_KEY_TBL_PAR_ERR_E),  /* 151 */                              \
STR(CPSS_PP_LX_MPLS_ILM_TBL_PAR_ERR_E),  /* 152 */                              \
STR(CPSS_PP_LX_MPLS_CNTR_E),  /* 153 */                                         \
STR(CPSS_PP_EB_TCC_E),  /* 154 */                                               \
                                                                                \
    /* Network Interface */                                                     \
STR(CPSS_PP_RX_BUFFER_QUEUE0_E),  /* 155 */                                     \
STR(CPSS_PP_RX_BUFFER_QUEUE1_E),  /* 156 */                                     \
STR(CPSS_PP_RX_BUFFER_QUEUE2_E),  /* 157 */                                     \
STR(CPSS_PP_RX_BUFFER_QUEUE3_E),  /* 158 */                                     \
STR(CPSS_PP_RX_BUFFER_QUEUE4_E),  /* 159 */                                     \
STR(CPSS_PP_RX_BUFFER_QUEUE5_E),  /* 160 */                                     \
STR(CPSS_PP_RX_BUFFER_QUEUE6_E),  /* 161 */                                     \
STR(CPSS_PP_RX_BUFFER_QUEUE7_E),  /* 162 */                                     \
STR(CPSS_PP_RX_ERR_QUEUE0_E),  /* 163 */                                        \
STR(CPSS_PP_RX_ERR_QUEUE1_E),  /* 164 */                                        \
STR(CPSS_PP_RX_ERR_QUEUE2_E),  /* 165 */                                        \
STR(CPSS_PP_RX_ERR_QUEUE3_E),  /* 166 */                                        \
STR(CPSS_PP_RX_ERR_QUEUE4_E),  /* 167 */                                        \
STR(CPSS_PP_RX_ERR_QUEUE5_E),  /* 168 */                                        \
STR(CPSS_PP_RX_ERR_QUEUE6_E),  /* 169 */                                        \
STR(CPSS_PP_RX_ERR_QUEUE7_E),  /* 170 */                                        \
STR(CPSS_PP_RX_CNTR_OVERFLOW_E),  /* 171 */                                     \
STR(CPSS_PP_TX_BUFFER_QUEUE_E),  /* 172 */                                      \
STR(CPSS_PP_TX_ERR_QUEUE_E),  /* 173 */                                         \
STR(CPSS_PP_TX_END_E),  /* 174 */                                               \
                                                                                \
    /* Per Port Per lane events */                                              \
STR(CPSS_PP_PORT_LANE_PRBS_ERROR_E),  /* 175 */                                 \
STR(CPSS_PP_PORT_LANE_DISPARITY_ERROR_E),  /* 176 */                            \
STR(CPSS_PP_PORT_LANE_SYMBOL_ERROR_E),  /* 177 */                               \
STR(CPSS_PP_PORT_LANE_CJR_PAT_ERROR_E),  /* 178 */                              \
STR(CPSS_PP_PORT_LANE_SIGNAL_DETECT_CHANGED_E),  /* 179 */                      \
STR(CPSS_PP_PORT_LANE_SYNC_STATUS_CHANGED_E),  /* 180 */                        \
STR(CPSS_PP_PORT_LANE_DETECTED_IIAII_E),  /* 181 */                             \
                                                                                \
    /* Per Port PCS events */                                                   \
STR(CPSS_PP_PORT_PCS_LINK_STATUS_CHANGED_E),  /* 182 */                         \
STR(CPSS_PP_PORT_PCS_DESKEW_TIMEOUT_E),  /* 183 */                              \
STR(CPSS_PP_PORT_PCS_DETECTED_COLUMN_IIAII_E),  /* 184 */                       \
STR(CPSS_PP_PORT_PCS_DESKEW_ERROR_E),  /* 185 */                                \
STR(CPSS_PP_PORT_PCS_PPM_FIFO_UNDERRUN_E),  /* 186 */                           \
STR(CPSS_PP_PORT_PCS_PPM_FIFO_OVERRUN_E),  /* 187 */                            \
STR(CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E),  /* 188 */                             \
                                                                                \
    /* Egress Buffer Manager */                                                 \
STR(CPSS_PP_BM_EGRESS_EMPTY_CLEAR_E),  /* 189 */                                \
STR(CPSS_PP_BM_EGRESS_EMPTY_INC_E),  /* 190 */                                  \
STR(CPSS_PP_BM_EGRESS_AGED_PACKET_E),  /* 191 */                                \
STR(CPSS_PP_BM_EGRESS_MAX_BUFF_REACHED_E),  /* 192 */                           \
STR(CPSS_PP_BM_EGRESS_PORT_RX_BUFFERS_CNT_UNDERRUN_E),  /* 193 */               \
STR(CPSS_PP_BM_EGRESS_PORT_RX_BUFFERS_CNT_OVERRUN_E),  /* 194 */                \
STR(CPSS_PP_BM_EGRESS_INVALID_ADDRESS_E),  /* 195 */                            \
STR(CPSS_PP_BM_EGRESS_WRONG_SRC_PORT_E),  /* 196 */                             \
STR(CPSS_PP_BM_EGRESS_MC_INC_OVERFLOW_E),  /* 197 */                            \
STR(CPSS_PP_BM_EGRESS_MC_INC_UNDERRUN_E),  /* 198 */                            \
STR(CPSS_PP_BM_EGRESS_TQ_PARITY_ERROR_PORT_E),  /* 199 */                       \
STR(CPSS_PP_BM_EGRESS_RX_MEM_READ_ECC_ERROR_E),  /* 200 */                      \
STR(CPSS_PP_BM_EGRESS_VLT_ECC_ERR_E),  /* 201 */                                \
STR(CPSS_PP_BM_EGRESS_MISC_E),  /* 202 */                                       \
                                                                                \
    /************************** PEX events *********************************/   \
STR(CPSS_PP_PEX_DL_DOWN_TX_ACC_ERR_E),  /* 203 */                               \
STR(CPSS_PP_PEX_MASTER_DISABLED_E),  /* 204 */                                  \
STR(CPSS_PP_PEX_ERROR_WR_TO_REG_E),  /* 205 */                                  \
STR(CPSS_PP_PEX_HIT_DEFAULT_WIN_ERR_E),  /* 206 */                              \
STR(CPSS_PP_PEX_COR_ERROR_DET_E),  /* 207 */                                    \
STR(CPSS_PP_PEX_NON_FATAL_ERROR_DET_E),  /* 208 */                              \
STR(CPSS_PP_PEX_FATAL_ERROR_DET_E),  /* 209 */                                  \
STR(CPSS_PP_PEX_DSTATE_CHANGED_E),  /* 210 */                                   \
STR(CPSS_PP_PEX_BIST_E),  /* 211 */                                             \
STR(CPSS_PP_PEX_RCV_ERROR_FATAL_E),  /* 212 */                                  \
STR(CPSS_PP_PEX_RCV_ERROR_NON_FATAL_E),  /* 213 */                              \
STR(CPSS_PP_PEX_RCV_ERROR_COR_E),  /* 214 */                                    \
STR(CPSS_PP_PEX_RCV_CRS_E),  /* 215 */                                          \
STR(CPSS_PP_PEX_PEX_SLAVE_HOT_RESET_E),  /* 216 */                              \
STR(CPSS_PP_PEX_PEX_SLAVE_DISABLE_LINK_E),  /* 217 */                           \
STR(CPSS_PP_PEX_PEX_SLAVE_LOOPBACK_E),  /* 218 */                               \
STR(CPSS_PP_PEX_PEX_LINK_FAIL_E),  /* 219 */                                    \
STR(CPSS_PP_PEX_RCV_A_E),  /* 220 */                                            \
STR(CPSS_PP_PEX_RCV_B_E),  /* 221 */                                            \
STR(CPSS_PP_PEX_RCV_C_E),  /* 222 */                                            \
STR(CPSS_PP_PEX_RCV_D_E),  /* 223 */                                            \
                                                                                \
    /******************** Buffer Memory events ****************************/    \
STR(CPSS_PP_BUF_MEM_MAC_ERROR_E),  /* 224 */                                    \
STR(CPSS_PP_BUF_MEM_ONE_ECC_ERROR_E),  /* 225 */                                \
STR(CPSS_PP_BUF_MEM_TWO_OR_MORE_ECC_ERRORS_E),  /* 226 */                       \
                                                                                \
                                                                                \
    /************************** Policer events *****************************/   \
STR(CPSS_PP_POLICER_ADDR_OUT_OF_MEMORY_E),  /* 227 */                           \
STR(CPSS_PP_POLICER_DATA_ERR_E),  /* 228 */                                     \
STR(CPSS_PP_POLICER_IPFIX_WRAP_AROUND_E),  /* 229 */                            \
STR(CPSS_PP_POLICER_IPFIX_ALARM_E),  /* 230 */                                  \
STR(CPSS_PP_POLICER_IPFIX_ALARMED_ENTRIES_FIFO_WRAP_AROUND_E),  /* 231 */       \
                                                                                \
    /************************** Policy Engine events ***********************/   \
STR(CPSS_PP_PCL_TCC_ECC_ERR_E),  /* 232 */                                      \
STR(CPSS_PP_PCL_MG_ADDR_OUT_OF_RANGE_E),  /* 233 */                             \
STR(CPSS_PP_PCL_LOOKUP_DATA_ERROR_E),  /* 234 */                                \
STR(CPSS_PP_PCL_ACTION_ERROR_DETECTED_E),  /* 235 */                            \
STR(CPSS_PP_PCL_MG_LOOKUP_RESULTS_READY_E),  /* 236 */                          \
STR(CPSS_PP_PCL_INLIF_TABLE_DATA_ERROR_E),  /* 237 */                           \
STR(CPSS_PP_PCL_CONFIG_TABLE_DATA_ERROR_E),  /* 238 */                          \
STR(CPSS_PP_PCL_LOOKUP_FIFO_FULL_E),  /* 239 */                                 \
                                                                                \
    /**************** Centralized Counters (CNC) events *********************/  \
STR(CPSS_PP_CNC_WRAPAROUND_BLOCK_E),  /* 240 */                                 \
STR(CPSS_PP_CNC_DUMP_FINISHED_E),  /* 241 */                                    \
                                                                                \
    /************************** SCT *****************************************/  \
STR(CPSS_PP_SCT_RATE_LIMITER_E),  /* 242 */                                     \
                                                                                \
    /************************** Egress and Ingress SFLOW (STC)***************/  \
STR(CPSS_PP_EGRESS_SFLOW_E),  /* 243 */                                         \
STR(CPSS_PP_INGRESS_SFLOW_SAMPLED_CNTR_E),  /* 244 */                           \
STR(CPSS_PP_EGRESS_SFLOW_SAMPLED_CNTR_E),  /* 245 */                            \
                                                                                \
    /************************** TTI events *****************************/       \
STR(CPSS_PP_TTI_CPU_ADDRESS_OUT_OF_RANGE_E),  /* 246 */                         \
STR(CPSS_PP_TTI_ACCESS_DATA_ERROR_E),  /* 247 */                                \
                                                                                \
    /************************** DIT events *****************************/       \
STR(CPSS_PP_DIT_MC_DIT_TTL_TRHOLD_EXCEED_E),  /* 248 */                         \
STR(CPSS_PP_DIT_UC_DIT_TTL_TRHOLD_EXCEED_E),  /* 249 */                         \
STR(CPSS_PP_DIT_DATA_ERROR_E),  /* 250 */                                       \
STR(CPSS_PP_DIT_MC0_PARITY_ERROR_E),  /* 251 */                                 \
STR(CPSS_PP_DIT_INT_TABLE_ECC_ERROR_CNTR_E),  /* 252 */                         \
STR(CPSS_PP_DIT_MC0_IP_TV_PARITY_ERROR_CNTR_E),  /* 253 */                      \
                                                                                \
    /************************** External TCAM events ***********************/   \
STR(CPSS_PP_EXTERNAL_TCAM_INT_SUM_E),  /* 254 */                                \
STR(CPSS_PP_EXTERNAL_TCAM_IO_ERR_E),  /* 255 */                                 \
STR(CPSS_PP_EXTERNAL_TCAM_RX_PARITY_ERR_E),  /* 256 */                          \
STR(CPSS_PP_EXTERNAL_TCAM_MG_CMD_EXE_DONE_E),  /* 257 */                        \
STR(CPSS_PP_EXTERNAL_TCAM_NEW_DATA_IN_MAILBOX_E),  /* 258 */                    \
STR(CPSS_PP_EXTERNAL_TCAM_MG_WRONG_EXP_RES_LEN_E),  /* 259 */                   \
STR(CPSS_PP_EXTERNAL_TCAM_RSP_IDQ_EMPTY_E),  /* 260 */                          \
STR(CPSS_PP_EXTERNAL_TCAM_PHASE_ERROR_E),  /* 261 */                            \
STR(CPSS_PP_EXTERNAL_TCAM_TRAINING_SEQUENCE_DONE_E),  /* 262 */                 \
                                                                                \
    /******************** External Memory events ******************** */        \
STR(CPSS_PP_EXTERNAL_MEMORY_PARITY_ERROR_E),  /* 263 */                         \
STR(CPSS_PP_EXTERNAL_MEMORY_ECC_ERROR_E),  /* 264 */                            \
STR(CPSS_PP_EXTERNAL_MEMORY_RLDRAM_UNIT_PARITY_ERR_LOW_E),  /* 265 */           \
STR(CPSS_PP_EXTERNAL_MEMORY_RLDRAM_UNIT_PARITY_ERR_HIGH_E),  /* 266 */          \
STR(CPSS_PP_EXTERNAL_MEMORY_RLDRAM_UNIT_MG_CMD_DONE_E),  /* 267 */              \
                                                                                \
                                                                                \
    /******************** PP VOQ events ****************************/           \
STR(CPSS_PP_VOQ_DROP_DEV_EN_E),  /* 268 */                                      \
STR(CPSS_PP_VOQ_GLOBAL_DESC_FULL_E),  /* 269 */                                 \
STR(CPSS_PP_VOQ_ADDR_OUT_OF_RANGE_E),  /* 270 */                                \
STR(CPSS_PP_VOQ_ECC_ERR_E),  /* 271 */                                          \
STR(CPSS_PP_VOQ_GLOBAL_BUF_FULL_E),  /* 272 */                                  \
                                                                                \
    /************************** PP XBAR events ******************************/  \
STR(CPSS_PP_XBAR_INVALID_TRG_DROP_CNTR_RL_E),  /* 273 */                        \
STR(CPSS_PP_XBAR_SRC_FILTER_DROP_CNTR_RL_E),  /* 274 */                         \
STR(CPSS_PP_XBAR_RX_CELL_CNTR_RL_E),  /* 275 */                                 \
STR(CPSS_PP_XBAR_PRIORITY_0_3_TX_DROP_CNTR_RL_E),  /* 276 */                    \
STR(CPSS_PP_XBAR_SHP_TX_DROP_CNTR_RL_E),  /* 277 */                             \
STR(CPSS_PP_XBAR_RX_FIFO_OVERRUN_CNTR_RL_E),  /* 278 */                         \
STR(CPSS_PP_XBAR_RX_FIFO_OVERRUN_CNTR_INC_E),  /* 279 */                        \
STR(CPSS_PP_XBAR_INVALID_TRG_DROP_CNTR_INC_E),  /* 280 */                       \
STR(CPSS_PP_XBAR_SRC_FILTER_DROP_CNTR_INC_E),  /* 281 */                        \
STR(CPSS_PP_XBAR_RX_CELL_CNTR_INC_E),  /* 282 */                                \
STR(CPSS_PP_XBAR_PRIORITY_0_3_TX_DROP_CNTR_INC_E),  /* 283 */                   \
STR(CPSS_PP_XBAR_SHP_TX_DROP_CNTR_INC_E),  /* 284 */                            \
STR(CPSS_PP_XBAR_PRIORITY_0_3_TX_FC_CHANGED_TO_XOFF_E),  /* 285 */              \
STR(CPSS_PP_XBAR_PRIORITY_0_3_TX_FC_CHANGED_TO_XON_E),  /* 286 */               \
STR(CPSS_PP_XBAR_INVALID_ADDR_E),  /* 287 */                                    \
STR(CPSS_PP_XBAR_TX_CELL_CNTR_RL_E),  /* 288 */                                 \
STR(CPSS_PP_XBAR_TX_CELL_CNTR_INC_E),  /* 289 */                                \
                                                                                \
    /******************** PP Fabric Reassembly engine events ****************/  \
STR(CPSS_PP_CRX_MAIL_ARRIVED_E),  /* 290 */                                     \
STR(CPSS_PP_CRX_READ_EMPTY_CPU_E),  /* 291 */                                   \
STR(CPSS_PP_CRX_DATA_DROP_E),  /* 292 */                                        \
STR(CPSS_PP_CRX_CPU_DROP_E),  /* 293 */                                         \
STR(CPSS_PP_CRX_AGED_OUT_CONTEXT_E),  /* 294 */                                 \
STR(CPSS_PP_CRX_CRC_DROP_E),  /* 295 */                                         \
STR(CPSS_PP_CRX_PACKET_LENGTH_DROP_E),  /* 296 */                               \
STR(CPSS_PP_CRX_MAX_BUFFERS_DROP_E),  /* 297 */                                 \
STR(CPSS_PP_CRX_BAD_CONTEXT_DROP_E),  /* 298 */                                 \
STR(CPSS_PP_CRX_NO_BUFFERS_DROP_E),  /* 299 */                                  \
STR(CPSS_PP_CRX_MULTICAST_CONGESTION_DROP_E),  /* 300 */                        \
STR(CPSS_PP_CRX_DATA_FIFO_UNDERRUN_E),  /* 301 */                               \
STR(CPSS_PP_CRX_DATA_FIFO_OVERRUN_E),  /* 302 */                                \
STR(CPSS_PP_CRX_CPU_UNDERRUN_E),  /* 303 */                                     \
STR(CPSS_PP_CRX_CPU_OVERRUN_E),  /* 304 */                                      \
STR(CPSS_PP_CRX_E2E_FIFO_UNDERRUN_E),  /* 305 */                                \
STR(CPSS_PP_CRX_E2E_FIFO_OVERRUN_E),  /* 306 */                                 \
STR(CPSS_PP_CRX_UNKNOWN_CELL_TYPE_E),  /* 307 */                                \
STR(CPSS_PP_CRX_COUNTER_BAD_ADDR_E),  /* 308 */                                 \
STR(CPSS_PP_CRX_PSM_BAD_ADDR_E),  /* 309 */                                     \
                                                                                \
    /*************** PP Fabric Segmentation engine events *******************/  \
STR(CPSS_PP_TXD_SEG_FIFO_PARITY_ERR_E),  /* 310 */                              \
STR(CPSS_PP_TXD_CPU_MAIL_SENT_E),  /* 311 */                                    \
STR(CPSS_PP_TXD_SEG_FIFO_OVERRUN_E),  /* 312 */                                 \
STR(CPSS_PP_TXD_WRONG_CH_TO_FABRIC_DIST_E),  /* 313 */                          \
                                                                                \
    /************************** IPVX events ********************************/   \
STR(CPSS_PP_IPVX_DATA_ERROR_E),  /* 314 */                                      \
STR(CPSS_PP_IPVX_ADDRESS_OUT_OF_MEMORY_E),  /* 315 */                           \
STR(CPSS_PP_IPVX_LPM_DATA_ERROR_E),  /* 316 */                                  \
                                                                                \
    /************************** Statistic Interface *************************/  \
STR(CPSS_PP_STAT_INF_TX_SYNC_FIFO_FULL_E),  /* 317 */                           \
STR(CPSS_PP_STAT_INF_TX_SYNC_FIFO_OVERRUN_E),  /* 318 */                        \
STR(CPSS_PP_STAT_INF_TX_SYNC_FIFO_UNDERRUN_E),  /* 319 */                       \
STR(CPSS_PP_STAT_INF_RX_SYNC_FIFO_FULL_E),  /* 320 */                           \
STR(CPSS_PP_STAT_INF_RX_SYNC_FIFO_OVERRUN_E),  /* 321 */                        \
STR(CPSS_PP_STAT_INF_RX_SYNC_FIFO_UNDERRUN_E),  /* 322 */                       \
                                                                                \
    /******************** PP HyperG Link ports events ***********************/  \
STR(CPSS_PP_HGLINK_PING_RECEIVED_E),  /* 323 */                                 \
STR(CPSS_PP_HGLINK_PING_SENT_E),  /* 324 */                                     \
STR(CPSS_PP_HGLINK_MAC_TX_OVERRUN_E),  /* 325 */                                \
STR(CPSS_PP_HGLINK_MAC_TX_UNDERRUN_E),  /* 326 */                               \
STR(CPSS_PP_HGLINK_RX_ERR_CNTR_RL_E),  /* 327 */                                \
STR(CPSS_PP_HGLINK_BAD_PCS_TO_MAC_REFORMAT_RX_E),  /* 328 */                    \
STR(CPSS_PP_HGLINK_BAD_HEAD_CELL_RX_E),  /* 329 */                              \
STR(CPSS_PP_HGLINK_BAD_LENGTH_CELL_RX_E),  /* 330 */                            \
STR(CPSS_PP_HGLINK_FC_CELL_RX_INT_E),  /* 331 */                                \
STR(CPSS_PP_HGLINK_FC_CELL_TX_INT_E),  /* 332 */                                \
STR(CPSS_PP_HGLINK_FC_XOFF_DEAD_LOCK_TC0_E),  /* 333 */                         \
STR(CPSS_PP_HGLINK_FC_XOFF_DEAD_LOCK_TC1_E),  /* 334 */                         \
STR(CPSS_PP_HGLINK_FC_XOFF_DEAD_LOCK_TC2_E),  /* 335 */                         \
STR(CPSS_PP_HGLINK_FC_XOFF_DEAD_LOCK_TC3_E),  /* 336 */                         \
STR(CPSS_PP_HGLINK_ADDR_OUT_OF_RANGE_E),  /* 337 */                             \
                                                                                \
    /**************************** TCC-TCAM *********************************/   \
STR(CPSS_PP_TCC_TCAM_ERROR_DETECTED_E),  /* 338 */                              \
STR(CPSS_PP_TCC_TCAM_BIST_FAILED_E),  /* 339 */                                 \
    /**************************** BCN **************************************/   \
STR(CPSS_PP_BCN_COUNTER_WRAP_AROUND_ERR_E),  /* 340 */                          \
                                                                                \
    /**************************** GTS (ingress/egress time stamp) **********/   \
STR(CPSS_PP_GTS_GLOBAL_FIFO_FULL_E),  /* 341 */                                 \
STR(CPSS_PP_GTS_VALID_TIME_SAMPLE_MESSAGE_E),  /* 342 */                        \
                                                                                \
    /**************************** Critical HW Error ***************************/\
STR(CPSS_PP_CRITICAL_HW_ERROR_E_E),  /* 343 */                                  \
                                                                                \
    /**************************** Port related addition ***********************/\
STR(CPSS_PP_PORT_PRBS_ERROR_QSGMII_E),  /* 344 */                               \
STR(CPSS_PP_PORT_802_3_AP_E),           /* 345 */                               \
STR(CPSS_PP_PORT_802_3_AP_CORE_1_E),           /* 346 */                        \
STR(CPSS_PP_PORT_802_3_AP_CORE_2_E),           /* 347 */                        \
STR(CPSS_PP_PORT_802_3_AP_CORE_3_E),           /* 348 */                        \
STR(CPSS_PP_PORT_802_3_AP_CORE_4_E),           /* 349 */                        \
STR(CPSS_PP_PORT_802_3_AP_CORE_5_E),           /* 350 */                        \
STR(CPSS_PP_PORT_802_3_AP_CORE_6_E),           /* 351 */                        \
STR(CPSS_PP_PORT_802_3_AP_CORE_7_E),           /* 352 */                        \
STR(CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E), /* 353 */                       \
                                                                                \
    /**************************** DFX Server related events *******************/\
STR(CPSS_PP_DATA_INTEGRITY_ERROR_E),           /* 354 */                        \
    /**************************** MAC table (FDB) addition ********************/\
STR(CPSS_PP_MAC_BANK_LEARN_COUNTERS_OVERFLOW_E),  /* 355 */                     \
                                                                                \
STR(CPSS_PP_PORT_EEE_E),                          /* 356 */                     \
                                                                                \
STR(CPSS_PP_OAM_EXCEPTION_KEEPALIVE_AGING_E),     /* 357 */                     \
STR(CPSS_PP_OAM_EXCEPTION_EXCESS_KEEPALIVE_E),    /* 358 */                     \
STR(CPSS_PP_OAM_EXCEPTION_INVALID_KEEPALIVE_E),   /* 359 */                     \
STR(CPSS_PP_OAM_EXCEPTION_RDI_STATUS_E),          /* 360 */                     \
STR(CPSS_PP_OAM_EXCEPTION_TX_PERIOD_E),           /* 361 */                     \
STR(CPSS_PP_OAM_EXCEPTION_MEG_LEVEL_E),           /* 362 */                     \
STR(CPSS_PP_OAM_EXCEPTION_SOURCE_INTERFACE_E),/* 363 */                         \
STR(CPSS_PP_OAM_ILLEGAL_ENTRY_INDEX_E),       /* 364 */                         \
/**************************** Network Interface addition ********************/  \
STR(CPSS_PP_RX_BUFFER_QUEUE8_E), /* 365 */                                      \
STR(CPSS_PP_RX_BUFFER_QUEUE9_E), /* 366 */                                      \
STR(CPSS_PP_RX_BUFFER_QUEUE10_E),/* 367 */                                      \
STR(CPSS_PP_RX_BUFFER_QUEUE11_E),/* 368 */                                      \
STR(CPSS_PP_RX_BUFFER_QUEUE12_E),/* 369 */                                      \
STR(CPSS_PP_RX_BUFFER_QUEUE13_E),/* 370 */                                      \
STR(CPSS_PP_RX_BUFFER_QUEUE14_E),/* 371 */                                      \
STR(CPSS_PP_RX_BUFFER_QUEUE15_E),/* 372 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE8_E),    /* 373 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE9_E),    /* 374 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE10_E),   /* 375 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE11_E),   /* 376 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE12_E),   /* 377 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE13_E),   /* 378 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE14_E),   /* 379 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE15_E),   /* 380 */                                      \
/* third CPU SDMA - queues 16..23 */                                            \
STR(CPSS_PP_RX_BUFFER_QUEUE16_E),/* 381 */                                      \
STR(CPSS_PP_RX_BUFFER_QUEUE17_E),/* 382 */                                      \
STR(CPSS_PP_RX_BUFFER_QUEUE18_E),/* 383 */                                      \
STR(CPSS_PP_RX_BUFFER_QUEUE19_E),/* 384 */                                      \
STR(CPSS_PP_RX_BUFFER_QUEUE20_E),/* 385 */                                      \
STR(CPSS_PP_RX_BUFFER_QUEUE21_E),/* 386 */                                      \
STR(CPSS_PP_RX_BUFFER_QUEUE22_E),/* 387 */                                      \
STR(CPSS_PP_RX_BUFFER_QUEUE23_E),/* 388 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE16_E),   /* 389 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE17_E),   /* 390 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE18_E),   /* 391 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE19_E),   /* 392 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE20_E),   /* 393 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE21_E),   /* 394 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE22_E),   /* 395 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE23_E),   /* 396 */                                      \
/* forth CPU SDMA - queues 24..31 */                                            \
STR(CPSS_PP_RX_BUFFER_QUEUE24_E),/* 397 */                                      \
STR(CPSS_PP_RX_BUFFER_QUEUE25_E),/* 398 */                                      \
STR(CPSS_PP_RX_BUFFER_QUEUE26_E),/* 399 */                                      \
STR(CPSS_PP_RX_BUFFER_QUEUE27_E),/* 400 */                                      \
STR(CPSS_PP_RX_BUFFER_QUEUE28_E),/* 401 */                                      \
STR(CPSS_PP_RX_BUFFER_QUEUE29_E),/* 402 */                                      \
STR(CPSS_PP_RX_BUFFER_QUEUE30_E),/* 403 */                                      \
STR(CPSS_PP_RX_BUFFER_QUEUE31_E),/* 404 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE24_E),   /* 405 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE25_E),   /* 406 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE26_E),   /* 407 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE27_E),   /* 408 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE28_E),   /* 409 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE29_E),   /* 410 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE30_E),   /* 411 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE31_E),   /* 412 */                                      \
                                                                                \
/**************************** Pipe events *********************************/    \
STR(CPSS_PP_PIPE_PCP_E), /* 413 */                                              \
STR(CPSS_PP_PHA_E     ), /* 414 */                                             \
                                                                                \
STR(CPSS_PP_PORT_PCS_GB_LOCK_SYNC_CHANGE_E),  /* 415 */                         \
                                                                                \
/**************************** Network Interface addition ********************/  \
/* CPU SDMA [4..15] - queues 32..127 : 'RX_BUFFER' */                           \
STR(CPSS_PP_RX_BUFFER_QUEUE32_E),  /* 416 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE33_E),  /* 417 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE34_E),  /* 418 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE35_E),  /* 419 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE36_E),  /* 420 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE37_E),  /* 421 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE38_E),  /* 422 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE39_E),  /* 423 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE40_E),  /* 424 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE41_E),  /* 425 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE42_E),  /* 426 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE43_E),  /* 427 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE44_E),  /* 428 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE45_E),  /* 429 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE46_E),  /* 430 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE47_E),  /* 431 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE48_E),  /* 432 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE49_E),  /* 433 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE50_E),  /* 434 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE51_E),  /* 435 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE52_E),  /* 436 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE53_E),  /* 437 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE54_E),  /* 438 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE55_E),  /* 439 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE56_E),  /* 440 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE57_E),  /* 441 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE58_E),  /* 442 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE59_E),  /* 443 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE60_E),  /* 444 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE61_E),  /* 445 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE62_E),  /* 446 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE63_E),  /* 447 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE64_E),  /* 448 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE65_E),  /* 449 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE66_E),  /* 450 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE67_E),  /* 451 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE68_E),  /* 452 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE69_E),  /* 453 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE70_E),  /* 454 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE71_E),  /* 455 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE72_E),  /* 456 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE73_E),  /* 457 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE74_E),  /* 458 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE75_E),  /* 459 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE76_E),  /* 460 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE77_E),  /* 461 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE78_E),  /* 462 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE79_E),  /* 463 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE80_E),  /* 464 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE81_E),  /* 465 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE82_E),  /* 466 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE83_E),  /* 467 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE84_E),  /* 468 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE85_E),  /* 469 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE86_E),  /* 470 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE87_E),  /* 471 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE88_E),  /* 472 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE89_E),  /* 473 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE90_E),  /* 474 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE91_E),  /* 475 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE92_E),  /* 476 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE93_E),  /* 477 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE94_E),  /* 478 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE95_E),  /* 479 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE96_E),  /* 480 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE97_E),  /* 481 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE98_E),  /* 482 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE99_E),  /* 483 */                                    \
STR(CPSS_PP_RX_BUFFER_QUEUE100_E),  /* 484 */                                   \
STR(CPSS_PP_RX_BUFFER_QUEUE101_E),  /* 485 */                                   \
STR(CPSS_PP_RX_BUFFER_QUEUE102_E),  /* 486 */                                   \
STR(CPSS_PP_RX_BUFFER_QUEUE103_E),  /* 487 */                                   \
STR(CPSS_PP_RX_BUFFER_QUEUE104_E),  /* 488 */                                   \
STR(CPSS_PP_RX_BUFFER_QUEUE105_E),  /* 489 */                                   \
STR(CPSS_PP_RX_BUFFER_QUEUE106_E),  /* 490 */                                   \
STR(CPSS_PP_RX_BUFFER_QUEUE107_E),  /* 491 */                                   \
STR(CPSS_PP_RX_BUFFER_QUEUE108_E),  /* 492 */                                   \
STR(CPSS_PP_RX_BUFFER_QUEUE109_E),  /* 493 */                                   \
STR(CPSS_PP_RX_BUFFER_QUEUE110_E),  /* 494 */                                   \
STR(CPSS_PP_RX_BUFFER_QUEUE111_E),  /* 495 */                                   \
STR(CPSS_PP_RX_BUFFER_QUEUE112_E),  /* 496 */                                   \
STR(CPSS_PP_RX_BUFFER_QUEUE113_E),  /* 497 */                                   \
STR(CPSS_PP_RX_BUFFER_QUEUE114_E),  /* 498 */                                   \
STR(CPSS_PP_RX_BUFFER_QUEUE115_E),  /* 499 */                                   \
STR(CPSS_PP_RX_BUFFER_QUEUE116_E),  /* 500 */                                   \
STR(CPSS_PP_RX_BUFFER_QUEUE117_E),  /* 501 */                                   \
STR(CPSS_PP_RX_BUFFER_QUEUE118_E),  /* 502 */                                   \
STR(CPSS_PP_RX_BUFFER_QUEUE119_E),  /* 503 */                                   \
STR(CPSS_PP_RX_BUFFER_QUEUE120_E),  /* 504 */                                   \
STR(CPSS_PP_RX_BUFFER_QUEUE121_E),  /* 505 */                                   \
STR(CPSS_PP_RX_BUFFER_QUEUE122_E),  /* 506 */                                   \
STR(CPSS_PP_RX_BUFFER_QUEUE123_E),  /* 507 */                                   \
STR(CPSS_PP_RX_BUFFER_QUEUE124_E),  /* 508 */                                   \
STR(CPSS_PP_RX_BUFFER_QUEUE125_E),  /* 509 */                                   \
STR(CPSS_PP_RX_BUFFER_QUEUE126_E),  /* 510 */                                   \
STR(CPSS_PP_RX_BUFFER_QUEUE127_E),  /* 511 */                                   \
/* CPU SDMA [4..15] - queues 32..127 : 'RX_ERR' */                              \
STR(CPSS_PP_RX_ERR_QUEUE32_E),  /* 512 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE33_E),  /* 513 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE34_E),  /* 514 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE35_E),  /* 515 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE36_E),  /* 516 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE37_E),  /* 517 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE38_E),  /* 518 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE39_E),  /* 519 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE40_E),  /* 520 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE41_E),  /* 521 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE42_E),  /* 522 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE43_E),  /* 523 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE44_E),  /* 524 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE45_E),  /* 525 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE46_E),  /* 526 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE47_E),  /* 527 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE48_E),  /* 528 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE49_E),  /* 529 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE50_E),  /* 530 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE51_E),  /* 531 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE52_E),  /* 532 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE53_E),  /* 533 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE54_E),  /* 534 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE55_E),  /* 535 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE56_E),  /* 536 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE57_E),  /* 537 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE58_E),  /* 538 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE59_E),  /* 539 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE60_E),  /* 540 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE61_E),  /* 541 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE62_E),  /* 542 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE63_E),  /* 543 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE64_E),  /* 544 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE65_E),  /* 545 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE66_E),  /* 546 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE67_E),  /* 547 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE68_E),  /* 548 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE69_E),  /* 549 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE70_E),  /* 550 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE71_E),  /* 551 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE72_E),  /* 552 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE73_E),  /* 553 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE74_E),  /* 554 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE75_E),  /* 555 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE76_E),  /* 556 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE77_E),  /* 557 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE78_E),  /* 558 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE79_E),  /* 559 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE80_E),  /* 560 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE81_E),  /* 561 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE82_E),  /* 562 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE83_E),  /* 563 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE84_E),  /* 564 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE85_E),  /* 565 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE86_E),  /* 566 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE87_E),  /* 567 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE88_E),  /* 568 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE89_E),  /* 569 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE90_E),  /* 570 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE91_E),  /* 571 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE92_E),  /* 572 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE93_E),  /* 573 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE94_E),  /* 574 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE95_E),  /* 575 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE96_E),  /* 576 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE97_E),  /* 577 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE98_E),  /* 578 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE99_E),  /* 579 */                                       \
STR(CPSS_PP_RX_ERR_QUEUE100_E),  /* 580 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE101_E),  /* 581 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE102_E),  /* 582 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE103_E),  /* 583 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE104_E),  /* 584 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE105_E),  /* 585 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE106_E),  /* 586 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE107_E),  /* 587 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE108_E),  /* 588 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE109_E),  /* 589 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE110_E),  /* 590 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE111_E),  /* 591 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE112_E),  /* 592 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE113_E),  /* 593 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE114_E),  /* 594 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE115_E),  /* 595 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE116_E),  /* 596 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE117_E),  /* 597 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE118_E),  /* 598 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE119_E),  /* 599 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE120_E),  /* 600 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE121_E),  /* 601 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE122_E),  /* 602 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE123_E),  /* 603 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE124_E),  /* 604 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE125_E),  /* 605 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE126_E),  /* 606 */                                      \
STR(CPSS_PP_RX_ERR_QUEUE127_E),  /* 607 */                                      \
                                                                                \
STR(CPSS_PP_PTP_TAI_INCOMING_TRIGGER_E),    /* 416 */                           \
STR(CPSS_PP_PTP_TAI_GENERATION_E),          /* 417 */                           \
/**************************** SRVCPU EVENTS *******************************/    \
STR(CPSS_SRVCPU_IPC_E),   /* 608 */                                             \
STR(CPSS_SRVCPU_PORT_LINK_STATUS_CHANGED_E),  /* 609 */                         \
STR(CPSS_SRVCPU_PORT_802_3_AP_E),   /* 610 */                                   \
                                                                                \
/************************** Xbar events ***********************************/    \
                                                                                \
STR(CPSS_XBAR_HG_LANE_SYNCH_CHANGE_E),/* 611 */                                 \
STR(CPSS_XBAR_HG_LANE_COMMA_ALIGN_CHANGE_E),  /* 612 */                         \
STR(CPSS_XBAR_GPORT_INVALID_TRG_DROP_CNT_INC_E),  /* 613 */                     \
STR(CPSS_XBAR_GPORT_DIAG_CELL_RX_CNTR_INC_E), /* 614 */                         \
STR(CPSS_XBAR_MISC_E),/* 615 */                                                 \
STR(CPSS_XBAR_HG_PING_RECEIVED_E),/* 616 */                                     \
STR(CPSS_XBAR_HOST_RX_FIFO_PEND_E),   /* 617 */                                 \
STR(CPSS_XBAR_HOST_SEND_CELL_E),  /* 618 */                                     \
STR(CPSS_XBAR_HG_ALIGNMENT_LOCK_CHANGE_E),/* 619 */                             \
STR(CPSS_XBAR_HG_PING_E), /* 620 */                                             \
STR(CPSS_XBAR_SD_INV_ADDR_E), /* 621 */                                         \
STR(CPSS_XBAR_SXBAR_INV_ADDR_E),  /* 622 */                                     \
STR(CPSS_XBAR_S_INV_ADDR_E),  /* 623 */                                         \
STR(CPSS_XBAR_G_INV_ADDR_E),  /* 624 */                                         \
STR(CPSS_XBAR_MC_INV_ADDR_E), /* 625 */                                         \
                                                                                \
/************************** Fa events *************************************/    \
                                                                                \
STR(CPSS_FA_UC_PRIO_BUF_FULL_E), /* 626 */                                      \
STR(CPSS_FA_MC_PRIO_BUF_FULL_E), /* 627 */                                      \
STR(CPSS_FA_UC_RX_BUF_FULL_E),   /* 628 */                                      \
STR(CPSS_FA_MC_RX_BUF_FULL_E),   /* 629 */                                      \
STR(CPSS_FA_WD_FPORT_GRP_E), /* 630 */                                          \
STR(CPSS_FA_MC_WD_EXP_E),/* 631 */                                              \
STR(CPSS_FA_GLBL_DESC_FULL_E),   /* 632 */                                      \
STR(CPSS_FA_MAC_UPD_THRS_E), /* 633 */                                          \
STR(CPSS_FA_DROP_DEV_EN_E),  /* 634 */                                          \
STR(CPSS_FA_MISC_I2C_TIMEOUT_E), /* 635 */                                      \
STR(CPSS_FA_MISC_STATUS_E),  /* 636 */                                          \
                                                                                \
/************************** DRAGONITE events *********************************/ \
                                                                                \
STR(CPSS_DRAGONITE_PORT_ON_E),/* 637 */                                         \
STR(CPSS_DRAGONITE_PORT_OFF_E),  /* 638 */                                      \
STR(CPSS_DRAGONITE_DETECT_FAIL_E),   /* 639 */                                  \
STR(CPSS_DRAGONITE_PORT_FAULT_E),  /* 640 */                                    \
STR(CPSS_DRAGONITE_PORT_UDL_E),  /* 641 */                                      \
STR(CPSS_DRAGONITE_PORT_FAULT_ON_STARTUP_E), /* 642 */                          \
STR(CPSS_DRAGONITE_PORT_PM_E),  /* 643 */                                       \
STR(CPSS_DRAGONITE_POWER_DENIED_E),  /* 644 */                                  \
STR(CPSS_DRAGONITE_OVER_TEMP_E), /* 645 */                                      \
STR(CPSS_DRAGONITE_TEMP_ALARM_E),  /* 646 */                                    \
STR(CPSS_DRAGONITE_DEVICE_FAULT_E),  /* 647 */                                  \
STR(CPSS_DRAGONITE_OVER_CONSUM_E),   /* 648 */                                  \
STR(CPSS_DRAGONITE_VMAIN_LOW_AF_E),  /* 649 */                                  \
STR(CPSS_DRAGONITE_VMAIN_LOW_AT_E),  /* 650 */                                  \
STR(CPSS_DRAGONITE_VMAIN_HIGH_E),  /* 651 */                                    \
STR(CPSS_DRAGONITE_READ_EVENT_E),  /* 652 */                                    \
STR(CPSS_DRAGONITE_WRITE_EVENT_E),   /* 653 */                                  \
STR(CPSS_DRAGONITE_ERROR_E)  /* 654 */

/*******************************************************************************
* PRV_CPSS_EVENT_MASK_DEVICE_SET_FUNC
*
* DESCRIPTION:
*       This routine mask/unmasks an unified event on specific device.
*
* INPUTS:
*       devNum - device number - PP/FA/Xbar device number -
*                depend on the uniEvent
*                if the uniEvent is in range of PP events , then devNum relate
*                to PP
*                if the uniEvent is in range of FA events , then devNum relate
*                to FA
*                if the uniEvent is in range of XBAR events , then devNum relate
*                to XBAR
*       uniEvent   - The unified event.
*       evExtData - The additional data (port num / priority
*                     queue number / other ) the event was received upon.
*                   may use value PRV_CPSS_DRV_EV_REQ_UNI_EV_EXTRA_DATA_ANY_CNS
*                   to indicate 'ALL interrupts' that relate to this unified
*                   event
*       operation  - the operation : mask / unmask
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL - on failure.
*       GT_BAD_PARAM - bad unify event value or bad device number
*       GT_NOT_FOUND - the unified event or the evExtData within the unified
*                      event are not found in this device interrupts
*       GT_HW_ERROR - on hardware error
*
* COMMENTS:
*
*       This function called when OS interrupts and the ExtDrv interrupts are
*       locked !
*
*******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_EVENT_MASK_DEVICE_SET_FUNC)
(
    IN GT_U8                    devNum,
    IN CPSS_UNI_EV_CAUSE_ENT    uniEvent,
    IN GT_U32                   evExtData,
    IN CPSS_EVENT_MASK_SET_ENT  operation
);

/*******************************************************************************
* PRV_CPSS_EVENT_MASK_DEVICE_GET_FUNC
*
* DESCRIPTION:
*       This routine reads mask status of given unified event, for specific port
*       or other constrain defined by extended data.
*
* INPUTS:
*       devNum - device number - PP/FA/Xbar device number -
*                depend on the uniEvent
*                if the uniEvent is in range of PP events , then devNum relate
*                to PP
*                if the uniEvent is in range of FA events , then devNum relate
*                to FA
*                if the uniEvent is in range of XBAR events , then devNum relate
*                to XBAR
*       uniEvent   - The unified event.
*       evExtData - The additional data (port num / priority
*                     queue number / other ) the event was received upon.
*                   may use value PRV_CPSS_DRV_EV_REQ_UNI_EV_EXTRA_DATA_ANY_CNS
*                   to indicate 'ALL interrupts' that relate to this unified
*                   event
*
* OUTPUTS:
*       maskedPtr  - is interrupt masked (GT_TRUE) or unmasked (GT_FALSE)
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL - on failure.
*       GT_BAD_PARAM - bad unify event value or bad device number
*       GT_NOT_FOUND - the unified event or the evExtData within the unified
*                      event are not found in this device interrupts
*       GT_HW_ERROR - on hardware error
*       GT_BAD_PTR  - bad pointer
*
* COMMENTS:
*
*       This function called when OS interrupts and the ExtDrv interrupts are
*       locked !
*
*******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_EVENT_MASK_DEVICE_GET_FUNC)
(
    IN  GT_U8                    devNum,
    IN  CPSS_UNI_EV_CAUSE_ENT    uniEvent,
    IN  GT_U32                   evExtData,
    OUT CPSS_EVENT_MASK_SET_ENT *maskedPtr
);


/*******************************************************************************
*  PRV_CPSS_EVENT_EXT_DATA_CONVERT_FUNC
*
*  DESCRIPTION:
*       This routine converts extended data for unified event on specific device.
*
*  APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon.
*
*  INPUTS:
*       devNum      - PP's device number.
*       uniEvent    - the unified event.
*       evConvertType - convert type for specific event.
*       evExtDataPtr - (pointer to) additional data (port num / lane number / other)
*                       the event was received upon.
*
*
*  OUTPUTS:
*       evExtDataPtr - (pointer to) converted/unchanged additional data (port num / lane number / other )
*                       the event was received upon.
*
*  RETURNS:
*       GT_OK   - on success
*       GT_BAD_PARAM - bad device number, or event convert type
*       GT_BAD_PTR  - evExtDataPtr is NULL pointer
*       GT_HW_ERROR - on hardware error
*       GT_NOT_SUPPORTED - not supported for current device family
*
*  COMMENTS:
*       This function called in scope of cpssEventRecv and converts extended data if need.
*       For example - MAC number or local RX/TX DMA number per-port events
*       will be converted to physical port numbers defined by cpssDxChPortPhysicalPortMapSet.
*
*******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_EVENT_EXT_DATA_CONVERT_FUNC)
(
    IN GT_U8                    devNum,
    IN CPSS_UNI_EV_CAUSE_ENT    uniEvent,
    IN PRV_CPSS_EV_CONVERT_DIRECTION_ENT evConvertType,
    INOUT GT_U32                *evExtDataPtr
);


/*******************************************************************************
*  PRV_CPSS_EVENT_MAC_EXT_DATA_CONVERT_FUNC
*
*  DESCRIPTION:
*       This routine converts extended data for unified event on specific device.
*
*  APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon.
*
*  INPUTS:
*       devNum      - PP's device number.
*       uniEvent    - the unified event.
*       evConvertType - convert type for specific event.
*       evExtDataPtr - (pointer to) additional data (port num / lane number / other)
*                       the event was received upon.
*
*
*  OUTPUTS:
*       evExtDataPtr - (pointer to) converted/unchanged additional data (port num / lane number / other )
*                       the event was received upon.
*
*  RETURNS:
*       GT_OK   - on success
*       GT_BAD_PARAM - bad device number, or event convert type
*       GT_BAD_PTR  - evExtDataPtr is NULL pointer
*       GT_HW_ERROR - on hardware error
*       GT_NOT_SUPPORTED - not supported for current device family
*
*  COMMENTS:
*       This function called in scope of cpssEventRecv and converts extended data if need.
*       For example - MAC number or local RX/TX DMA number per-port events
*       will be converted to physical port numbers defined by cpssDxChPortPhysicalPortMapSet.
*
*******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_EVENT_MAC_EXT_DATA_CONVERT_FUNC)
(
    IN  GT_U8       devNum,
    OUT GT_U32      *portMacNum
);


/**
* @internal prvCpssGenEventExtDataConvertBind function
* @endinternal
*
* @brief   This routine binds function that converts extended data for unified event
*         on specific device.
*
* @note   APPLICABLE DEVICES:      All devices (PP / FA / XBAR) with GPP
* @param[in] funcPtr                  - pointer to function that will be called
*                                      to convert extended data for unified event on specific device.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - funcPtr is NULL pointer
*/
GT_STATUS prvCpssGenEventExtDataConvertBind
(
    IN PRV_CPSS_EVENT_EXT_DATA_CONVERT_FUNC funcPtr
);

/**
* @internal prvCpssMacEventExtDataConvertBind function
* @endinternal
*
* @brief   This routine converts extended data for Mac related unified events on specific device.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @param[in] funcPtr                  - pointer to function that will be called
*                                      to convert extended data for Mac unified event on specific device.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - funcPtr is NULL pointer
*/
GT_STATUS prvCpssMacEventExtDataConvertBind
(
    IN PRV_CPSS_EVENT_MAC_EXT_DATA_CONVERT_FUNC funcPtr
);

/**
* @internal prvCpssGenEventMaskDeviceBind function
* @endinternal
*
* @brief   This routine mask/unmasks an unified event on specific device.
*
* @note   APPLICABLE DEVICES:      All devices (PP / FA / XBAR)
* @param[in] deviceType               - device Type - PP/FA/Xbar device
* @param[in] funcPtr                  - pointer to function that will be called when the
*                                      Application wants to mask/unmask events relate to the
* @param[in] deviceType
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - bad deviceType value or bad device number
* @retval GT_BAD_PTR               - funcPtr is NULL pointer
*/
GT_STATUS prvCpssGenEventMaskDeviceBind
(
    IN PRV_CPSS_DEVICE_TYPE_ENT    deviceType,
    IN PRV_CPSS_EVENT_MASK_DEVICE_SET_FUNC funcPtr
);

/**
* @internal prvCpssGenEventMaskGetDeviceBind function
* @endinternal
*
* @brief   This routine prepare getting data about masking/unmasking an unified event on specific device.
*
* @note   APPLICABLE DEVICES:      All devices (PP / FA / XBAR)
* @param[in] deviceType               - device Type - PP/FA/Xbar device
* @param[in] funcPtr                  - pointer to function that will be called when the
*                                      Application wants to get data is events relate to the
* @param[in] deviceType               masked or unmusked
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - bad deviceType value or bad device number
* @retval GT_BAD_PTR               - funcPtr is NULL pointer
*/
GT_STATUS prvCpssGenEventMaskGetDeviceBind
(
    IN PRV_CPSS_DEVICE_TYPE_ENT    deviceType,
    IN PRV_CPSS_EVENT_MASK_DEVICE_GET_FUNC funcPtr
);


/*******************************************************************************
* PRV_CPSS_EVENT_GENERATE_DEVICE_SET_FUNC
*
* DESCRIPTION:
*       This debug routine configures device to generate unified event for
*       specific element in the event associated with extra data.
*
* INPUTS:
*       devNum - device number - PP/FA/Xbar device number -
*                depend on the uniEvent
*                if the uniEvent is in range of PP events , then devNum relate
*                to PP
*                if the uniEvent is in range of FA events , then devNum relate
*                to FA
*                if the uniEvent is in range of XBAR events , then devNum relate
*                to XBAR
*       uniEvent   - The unified event.
*       evExtData - The additional data (port num / priority
*                   queue number / other ) the event was received upon.
*                   Use CPSS_PARAM_NOT_USED_CNS to generate events for all supported evExtData.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL - on failure.
*       GT_BAD_PARAM - bad unify event value or bad device number
*       GT_NOT_FOUND - the unified event has no interrupts associated with it in
*                      the device Or the 'extra data' has value that not relate
*                      to the uniEvent
*       GT_NOT_INITIALIZED - the CPSS was not initialized properly to handle
*                   this type of event
*       GT_HW_ERROR - on hardware error
*       GT_NOT_SUPPORTED - on not supported unify event
*
* COMMENTS:
*
*******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_EVENT_GENERATE_DEVICE_SET_FUNC)
(
    IN GT_U8                    devNum,
    IN CPSS_UNI_EV_CAUSE_ENT    uniEvent,
    IN GT_U32                   evExtData
);

/**
* @internal prvCpssGenEventGenerateDeviceBind function
* @endinternal
*
* @brief   This routine binds function that configure device to generate unified event for
*         specific element in the event associated with extra data.
*
* @note   APPLICABLE DEVICES:      All devices (PP / FA / XBAR)
* @param[in] deviceType               - device Type - PP/FA/Xbar device
* @param[in] funcPtr                  - pointer to function that will be called when the
*                                      Application wants to generate events relate to the
* @param[in] deviceType
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - bad deviceType value or bad device number
* @retval GT_BAD_PTR               - funcPtr is NULL pointer
*/
GT_STATUS prvCpssGenEventGenerateDeviceBind
(
    IN PRV_CPSS_DEVICE_TYPE_ENT                deviceType,
    IN PRV_CPSS_EVENT_GENERATE_DEVICE_SET_FUNC funcPtr
);

/*******************************************************************************
* PRV_CPSS_EVENT_GPP_ISR_CONNECT_FUNC
*
* DESCRIPTION:
*       This function connects an Isr for a given Gpp interrupt.
*
* APPLICABLE DEVICES:  All devices (PP / FA / XBAR) with GPP
*
* INPUTS:
*       devNum      - The Pp device number at which the Gpp device is connected.
*       gppId       - The Gpp Id to be connected.
*       isrFuncPtr  - A pointer to the function to be called on Gpp interrupt
*                     reception.
*       cookie      - A cookie to be passed to the isrFuncPtr when its called.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
* COMMENTS:
*       To disconnect a Gpp Isr, call this function with a NULL parameter in
*       the isrFuncPtr param.
*
*******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_EVENT_GPP_ISR_CONNECT_FUNC)
(
    IN  GT_U8                   devNum,
    IN  CPSS_EVENT_GPP_ID_ENT   gppId,
    IN  CPSS_EVENT_ISR_FUNC     isrFuncPtr,
    IN  void                    *cookie
);

/**
* @internal prvCpssGenEventGppIsrConnectBind function
* @endinternal
*
* @brief   This function binds function that connects an Isr for a given Gpp
*         interrupt.
*
* @note   APPLICABLE DEVICES:      All devices (PP / FA / XBAR) with GPP
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_BAD_PTR               - funcPtr is NULL pointer
*
* @note To disconnect a Gpp Isr, call this function with a NULL parameter in
*       the isrFuncPtr param.
*
*/
GT_STATUS prvCpssGenEventGppIsrConnectBind
(
    IN PRV_CPSS_EVENT_GPP_ISR_CONNECT_FUNC funcPtr
);

/**
* @internal prvCpssGenEventRequestsDbRelease function
* @endinternal
*
* @brief   private (internal) function to release the DB of the event related.
*         NOTE: function 'free' the allocated memory.
*         and restore DB to 'pre-init' state
*
* @note   APPLICABLE DEVICES:      ALL
*
* @retval GT_OK                    - on success.
*/
GT_STATUS prvCpssGenEventRequestsDbRelease
(
    void
);

/* check if event is CPU SDMA port 'per queue' event */
GT_BOOL prvCpssDrvEventIsCpuSdmaPortPerQueue(
    IN GT_U8                    devNum,
    IN CPSS_UNI_EV_CAUSE_ENT    uniEvent
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssGenEvReqh */


