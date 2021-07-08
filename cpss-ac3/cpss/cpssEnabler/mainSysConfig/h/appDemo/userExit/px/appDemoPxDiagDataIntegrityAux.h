/*******************************************************************************
*                Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
*/
/**
********************************************************************************
* @file appDemoPxDiagDataIntegrityAux.h
*
* @brief This file definests PX data integrity event increment callback function
*
* @version   1
********************************************************************************
*/

#ifndef __appDemoPxDiagDataIntegrityAux_h__
#define __appDemoPxDiagDataIntegrityAux_h__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/px/diag/cpssPxDiagDataIntegrity.h>



#define TEST_PX_DIAG_DATA_INTEGRITY_BANK_NUM_CNS 2
#define TEST_PX_DIAG_DATA_INTEGRITY_MPPM_NUM_CNS 2
#define TEST_PX_DIAG_DATA_INTEGRITY_MPPM_DB_SIZE_CNS (TEST_PX_DIAG_DATA_INTEGRITY_BANK_NUM_CNS*TEST_PX_DIAG_DATA_INTEGRITY_MPPM_NUM_CNS*8*2/*Single and Multiple*/)


typedef struct
{
    GT_U16 eventCounterArr[BIT_17];
    GT_U16 memTypeArr[BIT_17];
} TEST_PX_DIAG_DATA_INTEGRITY_COUNTER_DB_STC;


typedef struct
{
    GT_U16 eventCounterArr[TEST_PX_DIAG_DATA_INTEGRITY_MPPM_DB_SIZE_CNS];
} TEST_PX_DIAG_DATA_INTEGRITY_MPPM_COUNTER_DB_STC;

typedef GT_STATUS PX_DATA_INTEGRITY_EVENT_COUNTER_INCREMENT_FUNC
(
    IN  GT_U8                                       devNum,
    IN  CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_STC     *eventPtr
);

GT_STATUS appDemoPxDiagDataIntegrityCountersCallBackGet
(

    PX_DATA_INTEGRITY_EVENT_COUNTER_INCREMENT_FUNC **dataIntegrityEventCounterBlockGetCB
);


/**
* @internal cpssDxChDiagDataIntegrityEventTableClear function
* @endinternal
*
* @brief   clear all entries in event counter entry block.
*
* @retval GT_OK
* @note none
*
*/
GT_VOID appDemoPxDiagDataIntegrityEventTableClear(GT_VOID);


/**
* @internal cpssPxDiagDataIntegrityMppmEventTableClear function
* @endinternal
*
* @brief   clear all entries in event counter entry block.
*
*
*/
GT_VOID appDemoPxDiagDataIntegrityMppmEventTableClear(GT_VOID);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*__appDemoPxDiagDataIntegrityAux_h__*/



