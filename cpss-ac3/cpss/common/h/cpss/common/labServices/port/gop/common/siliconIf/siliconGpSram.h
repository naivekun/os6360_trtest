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
* @file siliconGpSram.h
*
* @brief Structure of General purpose SRAM used for inter CPU communication
*
* @version   1
********************************************************************************
*/

#ifndef __SILICON_GP_SRAM_h
#define __SILICON_GP_SRAM_h

/* serdes sequences array offset in scratch memory (hwsSerdes28nmSeqDb[MV_SERDES_LAST_SEQ]) */
#define CO_CPU_SERDES_SEQ_PTR 0x100
void coCpuSeqReadOperation(GT_U8 devNum, GT_U16 operAddr, MV_EL_DB_OPERATION *oper, MV_OP_PARAMS *params);

#endif /* __SILICON_GP_SRAM_h*/

