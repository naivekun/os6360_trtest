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
*      Describe TXQ PSI registers (Automaticly generated from CIDER)
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 2 $
*
*******************************************************************************/
#ifndef _TXQ_PSI_REGFILE_H_
#define _TXQ_PSI_REGFILE_H_

/* start of table entry Port_Queue_Offset*/

#define      TXQ_PSI_PORT_QUEUE_OFFSET_PORT_Q_OFFSET_0_FIELD_OFFSET			0


#define      TXQ_PSI_PORT_QUEUE_OFFSET_PORT_Q_OFFSET_0_FIELD_SIZE			32

/* end of Port_Queue_Offset*/


/* start of register Credit_Value*/

#define      TXQ_PSI_CREDIT_VALUE_CREDIT_VALUE_FIELD_OFFSET			0


#define      TXQ_PSI_CREDIT_VALUE_CREDIT_VALUE_FIELD_SIZE			14

/* end of Credit_Value*/


/* start of register PSI_Interrupt_Cause*/

#define      TXQ_PSI_PSI_INTERRUPT_CAUSE_PSI_INTSUM_FIELD_OFFSET			0
#define      TXQ_PSI_PSI_INTERRUPT_CAUSE_SCHEDULER_INTERRUPT_FIELD_OFFSET			1
#define      TXQ_PSI_PSI_INTERRUPT_CAUSE_MG2SNAKE_INTERRUPT_FIELD_OFFSET			2
#define      TXQ_PSI_PSI_INTERRUPT_CAUSE_PSI_BAD_ADDRESS_INT_FIELD_OFFSET			3


#define      TXQ_PSI_PSI_INTERRUPT_CAUSE_PSI_INTSUM_FIELD_SIZE			1
#define      TXQ_PSI_PSI_INTERRUPT_CAUSE_SCHEDULER_INTERRUPT_FIELD_SIZE			1
#define      TXQ_PSI_PSI_INTERRUPT_CAUSE_MG2SNAKE_INTERRUPT_FIELD_SIZE			1
#define      TXQ_PSI_PSI_INTERRUPT_CAUSE_PSI_BAD_ADDRESS_INT_FIELD_SIZE			1

/* end of PSI_Interrupt_Cause*/


/* start of register PSI_Interrupt_Mask*/

#define      TXQ_PSI_PSI_INTERRUPT_MASK_SCHEDULER_INTERRUPT_MASK_FIELD_OFFSET			1
#define      TXQ_PSI_PSI_INTERRUPT_MASK_MG2SNAKE_INTERRUPT_MASK_FIELD_OFFSET			2
#define      TXQ_PSI_PSI_INTERRUPT_MASK_BAD_ADDR_INTERRUPT_MASK_FIELD_OFFSET			3


#define      TXQ_PSI_PSI_INTERRUPT_MASK_SCHEDULER_INTERRUPT_MASK_FIELD_SIZE			1
#define      TXQ_PSI_PSI_INTERRUPT_MASK_MG2SNAKE_INTERRUPT_MASK_FIELD_SIZE			1
#define      TXQ_PSI_PSI_INTERRUPT_MASK_BAD_ADDR_INTERRUPT_MASK_FIELD_SIZE			1

/* end of PSI_Interrupt_Mask*/


/* start of register PSI_Last_Address_Violation*/

#define      TXQ_PSI_PSI_LAST_ADDRESS_VIOLATION_LAST_ADDRESS_VIOLATION_FIELD_OFFSET			0


#define      TXQ_PSI_PSI_LAST_ADDRESS_VIOLATION_LAST_ADDRESS_VIOLATION_FIELD_SIZE			32

/* end of PSI_Last_Address_Violation*/


/* start of register PSI_Metal_Fix_Register*/

#define      TXQ_PSI_PSI_METAL_FIX_REGISTER_PSI_METAL_FIX_FIELD_OFFSET			0


#define      TXQ_PSI_PSI_METAL_FIX_REGISTER_PSI_METAL_FIX_FIELD_SIZE			32

/* end of PSI_Metal_Fix_Register*/


/* start of register QmrSchPLenBursts*/

#define      TXQ_PSI_REGS_QMRSCHPLENBURSTS_QMRSCHPLENBURSTS_FIELD_OFFSET			0


#define      TXQ_PSI_REGS_QMRSCHPLENBURSTS_QMRSCHPLENBURSTS_FIELD_SIZE			10

/* end of QmrSchPLenBursts*/


/* start of register QmrSchBurstsSent*/

#define      TXQ_PSI_REGS_QMRSCHBURSTSSENT_QMRSCHBURSTSSENT_FIELD_OFFSET			0


#define      TXQ_PSI_REGS_QMRSCHBURSTSSENT_QMRSCHBURSTSSENT_FIELD_SIZE			6

/* end of QmrSchBurstsSent*/


/* start of register MSG_FIFO_Max_Peak*/

#define      TXQ_PSI_REGS_MSG_FIFO_MAX_PEAK_MSG_FIFO_MAX_PEAK_FIELD_OFFSET			0


#define      TXQ_PSI_REGS_MSG_FIFO_MAX_PEAK_MSG_FIFO_MAX_PEAK_FIELD_SIZE			12

/* end of MSG_FIFO_Max_Peak*/


/* start of register MSG_FIFO_Min_Peak*/

#define      TXQ_PSI_REGS_MSG_FIFO_MIN_PEAK_MSG_FIFO_MIN_PEAK_FIELD_OFFSET			0


#define      TXQ_PSI_REGS_MSG_FIFO_MIN_PEAK_MSG_FIFO_MIN_PEAK_FIELD_SIZE			12

/* end of MSG_FIFO_Min_Peak*/


/* start of register MSG_FIFO_Fill_Level*/

#define      TXQ_PSI_REGS_MSG_FIFO_FILL_LEVEL_MSG_FIFO_FILL_LEVEL_FIELD_OFFSET			0


#define      TXQ_PSI_REGS_MSG_FIFO_FILL_LEVEL_MSG_FIFO_FILL_LEVEL_FIELD_SIZE			12

/* end of MSG_FIFO_Fill_Level*/


/* start of register Queue_Status_Read_Request*/

#define      TXQ_PSI_REGS_QUEUE_STATUS_READ_REQUEST_QUEUE_STATUS_READ_REQUEST_FIELD_OFFSET			0


#define      TXQ_PSI_REGS_QUEUE_STATUS_READ_REQUEST_QUEUE_STATUS_READ_REQUEST_FIELD_SIZE			11

/* end of Queue_Status_Read_Request*/


/* start of register Queue_Status_Read_Reply*/

#define      TXQ_PSI_REGS_QUEUE_STATUS_READ_REPLY_QUEUE_STATUS_FIELD_OFFSET			0


#define      TXQ_PSI_REGS_QUEUE_STATUS_READ_REPLY_QUEUE_STATUS_FIELD_SIZE			1

/* end of Queue_Status_Read_Reply*/


/* start of register Debug_CFG_Register*/

#define      TXQ_PSI_REGS_DEBUG_CFG_REGISTER_DISABLE_DEQUEUE_OF_MSG_FIFO_FIELD_OFFSET			0


#define      TXQ_PSI_REGS_DEBUG_CFG_REGISTER_DISABLE_DEQUEUE_OF_MSG_FIFO_FIELD_SIZE			1

/* end of Debug_CFG_Register*/
#endif 
