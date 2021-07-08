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
* @file cpssDrvInterrupts.h
*
* @brief Includes general definitions for the interrupts handling unit.
*
* @version   1
********************************************************************************
*/
#ifndef __cpssDrvInterruptsh
#define __cpssDrvInterruptsh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/events/cpssGenEventCtrl.h>

/**
* @internal cpssDrvInterruptsTreeGet function
* @endinternal
*
* @brief   function return :
*         1. the root to the interrupts tree info of the specific device
*         2. the interrupt registers that can't be accesses before 'Start Init'
*
* @note   APPLICABLE DEVICES:      All devices --> Packet Processors (not Fa/Xbar)
* @param[in] devNum                   - the device number
*
* @param[out] numOfElementsPtr         - (pointer to) number of elements in the tree.
* @param[out] treeRootPtrPtr           - (pointer to) pointer to root of the interrupts tree info.
* @param[out] numOfInterruptRegistersNotAccessibleBeforeStartInitPtr - (pointer to)
*                                      number of interrupt registers that can't be accessed
*                                      before 'Start init'
* @param[out] notAccessibleBeforeStartInitPtrPtr (pointer to)pointer to the interrupt
*                                      registers that can't be accessed before 'Start init'
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - the driver was not initialized for the device
*/
GT_STATUS   cpssDrvInterruptsTreeGet
(
    IN GT_U8    devNum,
    OUT GT_U32  *numOfElementsPtr,
    OUT const CPSS_INTERRUPT_SCAN_STC        **treeRootPtrPtr,
    OUT GT_U32  *numOfInterruptRegistersNotAccessibleBeforeStartInitPtr,
    OUT GT_U32  **notAccessibleBeforeStartInitPtrPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDrvInterruptsh */



