/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cpssFalconTxqPdxRegFile.h
*
* DESCRIPTION:
*      Describe TXQ PDX registers (Automaticly generated from CIDER)
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 3 $
*
*******************************************************************************/
#ifndef _TXQ_PDX_REGFILE_H_
#define _TXQ_PDX_REGFILE_H_

/*PDX - IDX*/

/* start of register pdx_metal_fix*/

#define      TXQ_PDX_ITX_PDX_METAL_FIX_PDX_METAL_FIX_FIELD_OFFSET           0


#define      TXQ_PDX_ITX_PDX_METAL_FIX_PDX_METAL_FIX_FIELD_SIZE         32

/* end of pdx_metal_fix*/


/* start of register DB_Read_Control*/

#define      TXQ_PDX_ITX_DB_READ_CONTROL_DB_READ_CONTROL_FIELD_OFFSET           0


#define      TXQ_PDX_ITX_DB_READ_CONTROL_DB_READ_CONTROL_FIELD_SIZE         9

/* end of DB_Read_Control*/


/* start of register QCX_Read_qu_db*/

#define      TXQ_PDX_ITX_QCX_READ_QU_DB_QCX_READ_QU_DB_FIELD_OFFSET         0


#define      TXQ_PDX_ITX_QCX_READ_QU_DB_QCX_READ_QU_DB_FIELD_SIZE           20

/* end of QCX_Read_qu_db*/


/* start of register QCX_Read_Port_DB*/

#define      TXQ_PDX_ITX_QCX_READ_PORT_DB_QCX_READ_PORT_DB_FIELD_OFFSET         0


#define      TXQ_PDX_ITX_QCX_READ_PORT_DB_QCX_READ_PORT_DB_FIELD_SIZE           20

/* end of QCX_Read_Port_DB*/


/* start of register QCX_Read_Reorder_Fifos*/

#define      TXQ_PDX_ITX_QCX_READ_REORDER_FIFOS_QCX_READ_REORDER_FIFOS_FIELD_OFFSET         0


#define      TXQ_PDX_ITX_QCX_READ_REORDER_FIFOS_QCX_READ_REORDER_FIFOS_FIELD_SIZE           30

/* end of QCX_Read_Reorder_Fifos*/


/* start of register pdx2pdx_interface_mapping*/

#define      TXQ_PDX_ITX_PDX2PDX_INTERFACE_MAPPING_PDX_INTERFACE_ID_0_FIELD_OFFSET          0
#define      TXQ_PDX_ITX_PDX2PDX_INTERFACE_MAPPING_PDX_INTERFACE_ID_1_FIELD_OFFSET          2
#define      TXQ_PDX_ITX_PDX2PDX_INTERFACE_MAPPING_PDX_INTERFACE_ID_2_FIELD_OFFSET          4
#define      TXQ_PDX_ITX_PDX2PDX_INTERFACE_MAPPING_PDX_INTERFACE_ID_3_FIELD_OFFSET          6


#define      TXQ_PDX_ITX_PDX2PDX_INTERFACE_MAPPING_PDX_INTERFACE_ID_0_FIELD_SIZE            2
#define      TXQ_PDX_ITX_PDX2PDX_INTERFACE_MAPPING_PDX_INTERFACE_ID_1_FIELD_SIZE            2
#define      TXQ_PDX_ITX_PDX2PDX_INTERFACE_MAPPING_PDX_INTERFACE_ID_2_FIELD_SIZE            2
#define      TXQ_PDX_ITX_PDX2PDX_INTERFACE_MAPPING_PDX_INTERFACE_ID_3_FIELD_SIZE            2

/* end of pdx2pdx_interface_mapping*/


/* start of table entry CP_RXs_outstanding*/

#define      TXQ_PDX_ITX_CP_RXS_OUTSTANDING_RX0_OUTSTANDING_FIELD_OFFSET            0
#define      TXQ_PDX_ITX_CP_RXS_OUTSTANDING_RX1_OUTSTANDING_FIELD_OFFSET            8
#define      TXQ_PDX_ITX_CP_RXS_OUTSTANDING_RX2_OUTSTANDING_FIELD_OFFSET            16
#define      TXQ_PDX_ITX_CP_RXS_OUTSTANDING_RX3_OUTSTANDING_FIELD_OFFSET            24


#define      TXQ_PDX_ITX_CP_RXS_OUTSTANDING_RX0_OUTSTANDING_FIELD_SIZE          7
#define      TXQ_PDX_ITX_CP_RXS_OUTSTANDING_RX1_OUTSTANDING_FIELD_SIZE          7
#define      TXQ_PDX_ITX_CP_RXS_OUTSTANDING_RX2_OUTSTANDING_FIELD_SIZE          7
#define      TXQ_PDX_ITX_CP_RXS_OUTSTANDING_RX3_OUTSTANDING_FIELD_SIZE          7

/* end of CP_RXs_outstanding*/


/* start of register Interrupt_Cause*/

#define      TXQ_PDX_ITX_INTERRUPT_CAUSE_PDX_INTSUM_FIELD_OFFSET            0
#define      TXQ_PDX_ITX_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_FIELD_OFFSET            1
#define      TXQ_PDX_ITX_INTERRUPT_CAUSE_WRONGLY_MAPPED_QGRP_INT_FIELD_OFFSET           2
#define      TXQ_PDX_ITX_INTERRUPT_CAUSE_BURST_FIFO_DROP_INT_FIELD_OFFSET           3


#define      TXQ_PDX_ITX_INTERRUPT_CAUSE_PDX_INTSUM_FIELD_SIZE          1
#define      TXQ_PDX_ITX_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_FIELD_SIZE          1
#define      TXQ_PDX_ITX_INTERRUPT_CAUSE_WRONGLY_MAPPED_QGRP_INT_FIELD_SIZE         1
#define      TXQ_PDX_ITX_INTERRUPT_CAUSE_BURST_FIFO_DROP_INT_FIELD_SIZE         1

/* end of Interrupt_Cause*/


/* start of register Interrupt_Mask*/

#define      TXQ_PDX_ITX_INTERRUPT_MASK_PDX_INTERRUPT_CAUSE_MASK1_FIELD_OFFSET          1
#define      TXQ_PDX_ITX_INTERRUPT_MASK_DROPS_INTERRUPT_CAUSE_MASK_FIELD_OFFSET         2


#define      TXQ_PDX_ITX_INTERRUPT_MASK_PDX_INTERRUPT_CAUSE_MASK1_FIELD_SIZE            1
#define      TXQ_PDX_ITX_INTERRUPT_MASK_DROPS_INTERRUPT_CAUSE_MASK_FIELD_SIZE           2

/* end of Interrupt_Mask*/


/* start of register Address_Violation*/

#define      TXQ_PDX_ITX_ADDRESS_VIOLATION_ADDRESS_VIOLATION_FIELD_OFFSET           0


#define      TXQ_PDX_ITX_ADDRESS_VIOLATION_ADDRESS_VIOLATION_FIELD_SIZE         32

/* end of Address_Violation*/


/* start of register pfcc_burst_fifo_thr*/

#define      TXQ_PDX_ITX_PFCC_BURST_FIFO_THR_PFCC_BURST_FIFO_THR_FIELD_OFFSET			0


#define      TXQ_PDX_ITX_PFCC_BURST_FIFO_THR_PFCC_BURST_FIFO_THR_FIELD_SIZE			9

/* end of pfcc_burst_fifo_thr*/


/* start of register global_burst_fifo_thr*/

#define      TXQ_PDX_ITX_GLOBAL_BURST_FIFO_THR_GLOBAL_BURST_FIFO_THR_FIELD_OFFSET			0


#define      TXQ_PDX_ITX_GLOBAL_BURST_FIFO_THR_GLOBAL_BURST_FIFO_THR_FIELD_SIZE			9

/* end of global_burst_fifo_thr*/


/* start of register micro_Memory_Traffic_Counter*/

#define      TXQ_PDX_ITX_MICRO_MEMORY_TRAFFIC_COUNTER_MICRO_MEMORY_TRAFFIC_COUNTER_FIELD_OFFSET         0


#define      TXQ_PDX_ITX_MICRO_MEMORY_TRAFFIC_COUNTER_MICRO_MEMORY_TRAFFIC_COUNTER_FIELD_SIZE           32

/* end of micro_Memory_Traffic_Counter*/


/* start of table entry Micro_memory_bank_fill_level*/

#define      TXQ_PDX_ITX_MICRO_MEMORY_BANK_FILL_LEVEL_MICRO_MEMORY_BANK_FILL_LEVEL_FIELD_OFFSET         0


#define      TXQ_PDX_ITX_MICRO_MEMORY_BANK_FILL_LEVEL_MICRO_MEMORY_BANK_FILL_LEVEL_FIELD_SIZE           9

/* end of Micro_memory_bank_fill_level*/


/* start of register global_burst_fifo_available_entries*/

#define      TXQ_PDX_ITX_GLOBAL_BURST_FIFO_AVAILABLE_ENTRIES_GLOBAL_BURST_FIFO_AVAILABLE_ENTRIES_FIELD_OFFSET           0


#define      TXQ_PDX_ITX_GLOBAL_BURST_FIFO_AVAILABLE_ENTRIES_GLOBAL_BURST_FIFO_AVAILABLE_ENTRIES_FIELD_SIZE         16

/* end of global_burst_fifo_available_entries*/


/* start of register max_bank_fill_levels*/

#define      TXQ_PDX_ITX_MAX_BANK_FILL_LEVELS_MAX_BANK_FILL_LEVELS_FIELD_OFFSET         0


#define      TXQ_PDX_ITX_MAX_BANK_FILL_LEVELS_MAX_BANK_FILL_LEVELS_FIELD_SIZE           9

/* end of max_bank_fill_levels*/


/* start of register drop_Counter*/

#define      TXQ_PDX_ITX_DROP_COUNTER_DROP_COUNTER_FIELD_OFFSET         0


#define      TXQ_PDX_ITX_DROP_COUNTER_DROP_COUNTER_FIELD_SIZE           32

/* end of drop_Counter*/


/* start of register Drop_Counter_Mask*/

#define      TXQ_PDX_ITX_DROP_COUNTER_MASK_DROP_COUNTER_MASK_FIELD_OFFSET           0


#define      TXQ_PDX_ITX_DROP_COUNTER_MASK_DROP_COUNTER_MASK_FIELD_SIZE         8

/* end of Drop_Counter_Mask*/


/* start of register MIN_Global_Burst_Fifo_Available_Entries*/

#define      TXQ_PDX_ITX_MIN_GLOBAL_BURST_FIFO_AVAILABLE_ENTRIES_MIN_GLOBAL_BURST_FIFO_AVAILABLE_ENTRIES_FIELD_OFFSET           0


#define      TXQ_PDX_ITX_MIN_GLOBAL_BURST_FIFO_AVAILABLE_ENTRIES_MIN_GLOBAL_BURST_FIFO_AVAILABLE_ENTRIES_FIELD_SIZE         16

/* end of MIN_Global_Burst_Fifo_Available_Entries*/


/* start of table entry dx_qgrpmap*/

#define      TXQ_PDX_ITX_DX_QGRPMAP_QUEUE_BASE_FIELD_OFFSET         0
#define      TXQ_PDX_ITX_DX_QGRPMAP_DP_CORE_LOCAL_TRG_PORT_FIELD_OFFSET         9
#define      TXQ_PDX_ITX_DX_QGRPMAP_QUEUE_PDS_INDEX_FIELD_OFFSET            13
#define      TXQ_PDX_ITX_DX_QGRPMAP_QUEUE_PDX_INDEX_FIELD_OFFSET            16


#define      TXQ_PDX_ITX_DX_QGRPMAP_QUEUE_BASE_FIELD_SIZE           9
#define      TXQ_PDX_ITX_DX_QGRPMAP_DP_CORE_LOCAL_TRG_PORT_FIELD_SIZE           4
#define      TXQ_PDX_ITX_DX_QGRPMAP_QUEUE_PDS_INDEX_FIELD_SIZE          3
#define      TXQ_PDX_ITX_DX_QGRPMAP_QUEUE_PDX_INDEX_FIELD_SIZE          2

/* end of dx_qgrpmap*/


/* start of table entry qcx_qgrpmap*/

#define      TXQ_PDX_ITX_QCX_QGRPMAP_QUEUE_BASE_FIELD_OFFSET            0
#define      TXQ_PDX_ITX_QCX_QGRPMAP_DP_CORE_LOCAL_TRG_PORT_FIELD_OFFSET            9
#define      TXQ_PDX_ITX_QCX_QGRPMAP_QUEUE_PDS_INDEX_FIELD_OFFSET           13
#define      TXQ_PDX_ITX_QCX_QGRPMAP_QUEUE_PDX_INDEX_FIELD_OFFSET           16


#define      TXQ_PDX_ITX_QCX_QGRPMAP_QUEUE_BASE_FIELD_SIZE          9
#define      TXQ_PDX_ITX_QCX_QGRPMAP_DP_CORE_LOCAL_TRG_PORT_FIELD_SIZE          4
#define      TXQ_PDX_ITX_QCX_QGRPMAP_QUEUE_PDS_INDEX_FIELD_SIZE         3
#define      TXQ_PDX_ITX_QCX_QGRPMAP_QUEUE_PDX_INDEX_FIELD_SIZE         2

/* end of qcx_qgrpmap*/


/* start of table entry dx_qgrpmap_1*/

#define      TXQ_PDX_ITX_DX_QGRPMAP_1_QUEUE_BASE_FIELD_OFFSET           0
#define      TXQ_PDX_ITX_DX_QGRPMAP_1_DP_CORE_LOCAL_TRG_PORT_FIELD_OFFSET           9
#define      TXQ_PDX_ITX_DX_QGRPMAP_1_QUEUE_PDS_INDEX_FIELD_OFFSET          13
#define      TXQ_PDX_ITX_DX_QGRPMAP_1_QUEUE_PDX_INDEX_FIELD_OFFSET          16


#define      TXQ_PDX_ITX_DX_QGRPMAP_1_QUEUE_BASE_FIELD_SIZE         9
#define      TXQ_PDX_ITX_DX_QGRPMAP_1_DP_CORE_LOCAL_TRG_PORT_FIELD_SIZE         4
#define      TXQ_PDX_ITX_DX_QGRPMAP_1_QUEUE_PDS_INDEX_FIELD_SIZE            3
#define      TXQ_PDX_ITX_DX_QGRPMAP_1_QUEUE_PDX_INDEX_FIELD_SIZE            2

/* end of dx_qgrpmap_1*/


/* start of table entry qcx_qgrpmap_1*/

#define      TXQ_PDX_ITX_QCX_QGRPMAP_1_QUEUE_BASE_FIELD_OFFSET          0
#define      TXQ_PDX_ITX_QCX_QGRPMAP_1_DP_CORE_LOCAL_TRG_PORT_FIELD_OFFSET          9
#define      TXQ_PDX_ITX_QCX_QGRPMAP_1_QUEUE_PDS_INDEX_FIELD_OFFSET         13
#define      TXQ_PDX_ITX_QCX_QGRPMAP_1_QUEUE_PDX_INDEX_FIELD_OFFSET         16


#define      TXQ_PDX_ITX_QCX_QGRPMAP_1_QUEUE_BASE_FIELD_SIZE            9
#define      TXQ_PDX_ITX_QCX_QGRPMAP_1_DP_CORE_LOCAL_TRG_PORT_FIELD_SIZE            4
#define      TXQ_PDX_ITX_QCX_QGRPMAP_1_QUEUE_PDS_INDEX_FIELD_SIZE           3
#define      TXQ_PDX_ITX_QCX_QGRPMAP_1_QUEUE_PDX_INDEX_FIELD_SIZE           2

/* end of qcx_qgrpmap_1*/

/*PDX - PAC*/

/* start of register pdx_pac_metal_fix*/

#define      TXQ_PDX_PAC_PDX_PAC_METAL_FIX_PDX_PAC_METAL_FIX_FIELD_OFFSET           0


#define      TXQ_PDX_PAC_PDX_PAC_METAL_FIX_PDX_PAC_METAL_FIX_FIELD_SIZE         32

/* end of pdx_pac_metal_fix*/


/* start of table entry port_desc_counter*/

#define      TXQ_PDX_PAC_PORT_DESC_COUNTER_PORT_DESC_COUNTER_FIELD_OFFSET           0


#define      TXQ_PDX_PAC_PORT_DESC_COUNTER_PORT_DESC_COUNTER_FIELD_SIZE         16

/* end of port_desc_counter*/


/* start of table entry PDS_Port_Mem_status*/

#define      TXQ_PDX_PAC_PDS_PORT_MEM_STATUS_PDS_0_PORT_MEM_STATUS_FIELD_OFFSET         0
#define      TXQ_PDX_PAC_PDS_PORT_MEM_STATUS_PDS_PORT_1_MEM_STATUS_FIELD_OFFSET         3
#define      TXQ_PDX_PAC_PDS_PORT_MEM_STATUS_PDS_PORT_2_MEM_STATUS_FIELD_OFFSET         6
#define      TXQ_PDX_PAC_PDS_PORT_MEM_STATUS_PDS_PORT_3_MEM_STATUS_FIELD_OFFSET         9
#define      TXQ_PDX_PAC_PDS_PORT_MEM_STATUS_PDS_PORT_4_MEM_STATUS_FIELD_OFFSET         12
#define      TXQ_PDX_PAC_PDS_PORT_MEM_STATUS_PDS_PORT_5_MEM_STATUS_FIELD_OFFSET         15
#define      TXQ_PDX_PAC_PDS_PORT_MEM_STATUS_PDS_PORT_6_MEM_STATUS_FIELD_OFFSET         18
#define      TXQ_PDX_PAC_PDS_PORT_MEM_STATUS_PDS_PORT_7_MEM_STATUS_FIELD_OFFSET         21
#define      TXQ_PDX_PAC_PDS_PORT_MEM_STATUS_PDS_PORT_8_MEM_STATUS_FIELD_OFFSET         24


#define      TXQ_PDX_PAC_PDS_PORT_MEM_STATUS_PDS_0_PORT_MEM_STATUS_FIELD_SIZE           3
#define      TXQ_PDX_PAC_PDS_PORT_MEM_STATUS_PDS_PORT_1_MEM_STATUS_FIELD_SIZE           3
#define      TXQ_PDX_PAC_PDS_PORT_MEM_STATUS_PDS_PORT_2_MEM_STATUS_FIELD_SIZE           3
#define      TXQ_PDX_PAC_PDS_PORT_MEM_STATUS_PDS_PORT_3_MEM_STATUS_FIELD_SIZE           3
#define      TXQ_PDX_PAC_PDS_PORT_MEM_STATUS_PDS_PORT_4_MEM_STATUS_FIELD_SIZE           3
#define      TXQ_PDX_PAC_PDS_PORT_MEM_STATUS_PDS_PORT_5_MEM_STATUS_FIELD_SIZE           3
#define      TXQ_PDX_PAC_PDS_PORT_MEM_STATUS_PDS_PORT_6_MEM_STATUS_FIELD_SIZE           3
#define      TXQ_PDX_PAC_PDS_PORT_MEM_STATUS_PDS_PORT_7_MEM_STATUS_FIELD_SIZE           3
#define      TXQ_PDX_PAC_PDS_PORT_MEM_STATUS_PDS_PORT_8_MEM_STATUS_FIELD_SIZE           3

/* end of PDS_Port_Mem_status*/


/* start of table entry PDS_traffic_count_mask*/

#define      TXQ_PDX_PAC_PDS_TRAFFIC_COUNT_MASK_PDS_TRAFFIC_COUNT_MASK_FIELD_OFFSET         0


#define      TXQ_PDX_PAC_PDS_TRAFFIC_COUNT_MASK_PDS_TRAFFIC_COUNT_MASK_FIELD_SIZE           9

/* end of PDS_traffic_count_mask*/


/* start of table entry PDS_traffic_count*/

#define      TXQ_PDX_PAC_PDS_TRAFFIC_COUNT_PDS_TRAFFIC_COUNT_FIELD_OFFSET           0


#define      TXQ_PDX_PAC_PDS_TRAFFIC_COUNT_PDS_TRAFFIC_COUNT_FIELD_SIZE         32

/* end of PDS_traffic_count*/


/* start of register Interrupt_Cause*/

#define      TXQ_PDX_PAC_INTERRUPT_CAUSE_PDX_PAC_INTSUM_FIELD_OFFSET            0
#define      TXQ_PDX_PAC_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_FIELD_OFFSET            1


#define      TXQ_PDX_PAC_INTERRUPT_CAUSE_PDX_PAC_INTSUM_FIELD_SIZE          1
#define      TXQ_PDX_PAC_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_FIELD_SIZE          1

/* end of Interrupt_Cause*/


/* start of register Interrupt_Mask*/

#define      TXQ_PDX_PAC_INTERRUPT_MASK_PDX_PAC_INTERRUPT_CAUSE_MASK1_FIELD_OFFSET          1


#define      TXQ_PDX_PAC_INTERRUPT_MASK_PDX_PAC_INTERRUPT_CAUSE_MASK1_FIELD_SIZE            1

/* end of Interrupt_Mask*/


/* start of register Address_Violation*/

#define      TXQ_PDX_PAC_ADDRESS_VIOLATION_ADDRESS_VIOLATION_FIELD_OFFSET           0


#define      TXQ_PDX_PAC_ADDRESS_VIOLATION_ADDRESS_VIOLATION_FIELD_SIZE         32

/* end of Address_Violation*/


/* start of table entry pizza_arbiter_configuration_register_pds_0*/

#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_0_PDS_SLOT_MAP_0_FIELD_OFFSET         0
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_0_PDS_SLOT_VALID_0_FIELD_OFFSET           31


#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_0_PDS_SLOT_MAP_0_FIELD_SIZE           4
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_0_PDS_SLOT_VALID_0_FIELD_SIZE         1

/* end of pizza_arbiter_configuration_register_pds_0*/


/* start of table entry pizza_arbiter_configuration_register_pds_1*/

#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_1_PDS_1_SLOT_MAP_0_FIELD_OFFSET           0
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_1_PDS_1_SLOT_VALID_0_FIELD_OFFSET         31


#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_1_PDS_1_SLOT_MAP_0_FIELD_SIZE         4
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_1_PDS_1_SLOT_VALID_0_FIELD_SIZE           1

/* end of pizza_arbiter_configuration_register_pds_1*/


/* start of table entry pizza_arbiter_configuration_register_pds_2*/

#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_2_PDS_2_SLOT_MAP_0_FIELD_OFFSET           0
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_2_PDS_2_SLOT_VALID_0_FIELD_OFFSET         31


#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_2_PDS_2_SLOT_MAP_0_FIELD_SIZE         4
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_2_PDS_2_SLOT_VALID_0_FIELD_SIZE           1

/* end of pizza_arbiter_configuration_register_pds_2*/


/* start of table entry pizza_arbiter_configuration_register_pds_3*/

#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_3_PDS_3_SLOT_MAP_0_FIELD_OFFSET           0
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_3_PDS_3_SLOT_VALID_0_FIELD_OFFSET         31


#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_3_PDS_3_SLOT_MAP_0_FIELD_SIZE         4
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_3_PDS_3_SLOT_VALID_0_FIELD_SIZE           1

/* end of pizza_arbiter_configuration_register_pds_3*/


/* start of table entry pizza_arbiter_configuration_register_pds_10*/

#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_10_PDS_SLOT_MAP_10_FIELD_OFFSET           0
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_10_PDS_SLOT_VALID_10_FIELD_OFFSET         31


#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_10_PDS_SLOT_MAP_10_FIELD_SIZE         4
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_10_PDS_SLOT_VALID_10_FIELD_SIZE           1

/* end of pizza_arbiter_configuration_register_pds_10*/


/* start of table entry pizza_arbiter_configuration_register_pds_11*/

#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_11_PDS_SLOT_MAP_11_FIELD_OFFSET           0
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_11_PDS_SLOT_VALID_11_FIELD_OFFSET         31


#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_11_PDS_SLOT_MAP_11_FIELD_SIZE         4
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_11_PDS_SLOT_VALID_11_FIELD_SIZE           1

/* end of pizza_arbiter_configuration_register_pds_11*/


/* start of table entry pizza_arbiter_configuration_register_pds_12*/

#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_12_PDS_SLOT_MAP_12_FIELD_OFFSET           0
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_12_PDS_SLOT_VALID_12_FIELD_OFFSET         31


#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_12_PDS_SLOT_MAP_12_FIELD_SIZE         4
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_12_PDS_SLOT_VALID_12_FIELD_SIZE           1

/* end of pizza_arbiter_configuration_register_pds_12*/


/* start of table entry pizza_arbiter_configuration_register_pds_13*/

#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_13_PDS_SLOT_MAP_13_FIELD_OFFSET           0
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_13_PDS_SLOT_VALID_13_FIELD_OFFSET         31


#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_13_PDS_SLOT_MAP_13_FIELD_SIZE         4
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_13_PDS_SLOT_VALID_13_FIELD_SIZE           1

/* end of pizza_arbiter_configuration_register_pds_13*/


/* start of table entry pizza_arbiter_configuration_register_pds_14*/

#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_14_PDS_SLOT_MAP_14_FIELD_OFFSET           0
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_14_PDS_SLOT_VALID_14_FIELD_OFFSET         31


#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_14_PDS_SLOT_MAP_14_FIELD_SIZE         4
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_14_PDS_SLOT_VALID_14_FIELD_SIZE           1

/* end of pizza_arbiter_configuration_register_pds_14*/


/* start of table entry pizza_arbiter_configuration_register_pds_15*/

#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_15_PDS_SLOT_MAP_15_FIELD_OFFSET           0
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_15_PDS_SLOT_VALID_15_FIELD_OFFSET         31


#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_15_PDS_SLOT_MAP_15_FIELD_SIZE         4
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_15_PDS_SLOT_VALID_15_FIELD_SIZE           1

/* end of pizza_arbiter_configuration_register_pds_15*/


/* start of table entry pizza_arbiter_configuration_register_pds_16*/

#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_16_PDS_SLOT_MAP_16_FIELD_OFFSET           0
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_16_PDS_SLOT_VALID_16_FIELD_OFFSET         31


#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_16_PDS_SLOT_MAP_16_FIELD_SIZE         4
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_16_PDS_SLOT_VALID_16_FIELD_SIZE           1

/* end of pizza_arbiter_configuration_register_pds_16*/


/* start of table entry pizza_arbiter_configuration_register_pds_17*/

#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_17_PDS_SLOT_MAP_17_FIELD_OFFSET           0
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_17_PDS_SLOT_VALID_17_FIELD_OFFSET         31


#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_17_PDS_SLOT_MAP_17_FIELD_SIZE         4
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_17_PDS_SLOT_VALID_17_FIELD_SIZE           1

/* end of pizza_arbiter_configuration_register_pds_17*/


/* start of table entry pizza_arbiter_configuration_register_pds_18*/

#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_18_PDS_SLOT_MAP_18_FIELD_OFFSET           0
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_18_PDS_SLOT_VALID_18_FIELD_OFFSET         31


#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_18_PDS_SLOT_MAP_18_FIELD_SIZE         4
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_18_PDS_SLOT_VALID_18_FIELD_SIZE           1

/* end of pizza_arbiter_configuration_register_pds_18*/


/* start of table entry pizza_arbiter_configuration_register_pds_19*/

#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_19_PDS_SLOT_MAP_19_FIELD_OFFSET           0
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_19_PDS_SLOT_VALID_19_FIELD_OFFSET         31


#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_19_PDS_SLOT_MAP_19_FIELD_SIZE         4
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_19_PDS_SLOT_VALID_19_FIELD_SIZE           1

/* end of pizza_arbiter_configuration_register_pds_19*/


/* start of table entry pizza_arbiter_configuration_register_pds_20*/

#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_20_PDS_SLOT_MAP_20_FIELD_OFFSET           0
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_20_PDS_SLOT_VALID_20_FIELD_OFFSET         31


#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_20_PDS_SLOT_MAP_20_FIELD_SIZE         4
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_20_PDS_SLOT_VALID_20_FIELD_SIZE           1

/* end of pizza_arbiter_configuration_register_pds_20*/


/* start of table entry pizza_arbiter_configuration_register_pds_21*/

#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_21_PDS_SLOT_MAP_21_FIELD_OFFSET           0
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_21_PDS_SLOT_VALID_21_FIELD_OFFSET         31


#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_21_PDS_SLOT_MAP_21_FIELD_SIZE         4
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_21_PDS_SLOT_VALID_21_FIELD_SIZE           1

/* end of pizza_arbiter_configuration_register_pds_21*/


/* start of table entry pizza_arbiter_configuration_register_pds_22*/

#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_22_PDS_SLOT_MAP_22_FIELD_OFFSET           0
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_22_PDS_SLOT_VALID_22_FIELD_OFFSET         31


#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_22_PDS_SLOT_MAP_22_FIELD_SIZE         4
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_22_PDS_SLOT_VALID_22_FIELD_SIZE           1

/* end of pizza_arbiter_configuration_register_pds_22*/


/* start of table entry pizza_arbiter_configuration_register_pds_23*/

#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_23_PDS_SLOT_MAP_23_FIELD_OFFSET           0
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_23_PDS_SLOT_VALID_23_FIELD_OFFSET         31


#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_23_PDS_SLOT_MAP_23_FIELD_SIZE         4
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_23_PDS_SLOT_VALID_23_FIELD_SIZE           1

/* end of pizza_arbiter_configuration_register_pds_23*/


/* start of table entry pizza_arbiter_configuration_register_pds_24*/

#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_24_PDS_SLOT_MAP_24_FIELD_OFFSET           0
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_24_PDS_SLOT_VALID_24_FIELD_OFFSET         31


#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_24_PDS_SLOT_MAP_24_FIELD_SIZE         4
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_24_PDS_SLOT_VALID_24_FIELD_SIZE           1

/* end of pizza_arbiter_configuration_register_pds_24*/


/* start of table entry pizza_arbiter_configuration_register_pds_25*/

#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_25_PDS_SLOT_MAP_25_FIELD_OFFSET           0
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_25_PDS_SLOT_VALID_25_FIELD_OFFSET         31


#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_25_PDS_SLOT_MAP_25_FIELD_SIZE         4
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_25_PDS_SLOT_VALID_25_FIELD_SIZE           1

/* end of pizza_arbiter_configuration_register_pds_25*/


/* start of table entry pizza_arbiter_configuration_register_pds_26*/

#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_26_PDS_SLOT_MAP_26_FIELD_OFFSET           0
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_26_PDS_SLOT_VALID_26_FIELD_OFFSET         31


#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_26_PDS_SLOT_MAP_26_FIELD_SIZE         4
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_26_PDS_SLOT_VALID_26_FIELD_SIZE           1

/* end of pizza_arbiter_configuration_register_pds_26*/


/* start of table entry pizza_arbiter_configuration_register_pds_27*/

#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_27_PDS_SLOT_MAP_27_FIELD_OFFSET           0
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_27_PDS_SLOT_VALID_27_FIELD_OFFSET         31


#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_27_PDS_SLOT_MAP_27_FIELD_SIZE         4
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_27_PDS_SLOT_VALID_27_FIELD_SIZE           1

/* end of pizza_arbiter_configuration_register_pds_27*/


/* start of table entry pizza_arbiter_configuration_register_pds_28*/

#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_28_PDS_SLOT_MAP_28_FIELD_OFFSET           0
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_28_PDS_SLOT_VALID_28_FIELD_OFFSET         31


#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_28_PDS_SLOT_MAP_28_FIELD_SIZE         4
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_28_PDS_SLOT_VALID_28_FIELD_SIZE           1

/* end of pizza_arbiter_configuration_register_pds_28*/


/* start of table entry pizza_arbiter_configuration_register_pds_29*/

#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_29_PDS_SLOT_MAP_29_FIELD_OFFSET           0
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_29_PDS_SLOT_VALID_29_FIELD_OFFSET         31


#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_29_PDS_SLOT_MAP_29_FIELD_SIZE         4
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_29_PDS_SLOT_VALID_29_FIELD_SIZE           1

/* end of pizza_arbiter_configuration_register_pds_29*/


/* start of table entry pizza_arbiter_configuration_register_pds_30*/

#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_30_PDS_SLOT_MAP_30_FIELD_OFFSET           0
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_30_PDS_SLOT_VALID_30_FIELD_OFFSET         31


#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_30_PDS_SLOT_MAP_30_FIELD_SIZE         4
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_30_PDS_SLOT_VALID_30_FIELD_SIZE           1

/* end of pizza_arbiter_configuration_register_pds_30*/


/* start of table entry pizza_arbiter_configuration_register_pds_31*/

#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_31_PDS_SLOT_MAP_31_FIELD_OFFSET           0
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_31_PDS_SLOT_VALID_31_FIELD_OFFSET         31


#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_31_PDS_SLOT_MAP_31_FIELD_SIZE         4
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_31_PDS_SLOT_VALID_31_FIELD_SIZE           1

/* end of pizza_arbiter_configuration_register_pds_31*/


/* start of table entry pizza_arbiter_configuration_register_pds_32*/

#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_32_PDS_SLOT_MAP_32_FIELD_OFFSET           0
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_32_PDS_SLOT_VALID_32_FIELD_OFFSET         31


#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_32_PDS_SLOT_MAP_32_FIELD_SIZE         4
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONFIGURATION_REGISTER_PDS_32_PDS_SLOT_VALID_32_FIELD_SIZE           1

/* end of pizza_arbiter_configuration_register_pds_32*/

/* start of table entry Pizza_Arbiter_Control_Register_pds*/

#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONTROL_REGISTER_PDS_MAX_SLICE_FIELD_OFFSET          0
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONTROL_REGISTER_PDS_LOAD_NEW_PIZZA_FIELD_OFFSET         31


#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONTROL_REGISTER_PDS_MAX_SLICE_FIELD_SIZE            7
#define      TXQ_PDX_PAC_PIZZA_ARBITER_CONTROL_REGISTER_PDS_LOAD_NEW_PIZZA_FIELD_SIZE           1

/* end of Pizza_Arbiter_Control_Register_pds*/



#endif 
