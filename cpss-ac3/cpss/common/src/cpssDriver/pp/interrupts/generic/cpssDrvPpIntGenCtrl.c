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
* @file cpssDrvPpIntGenCtrl.c
*
* @brief Includes driver interrupts control functions.
*
* @version   2
********************************************************************************
*/

#include <cpssDriver/pp/interrupts/generic/prvCpssDrvInterrupts.h>
#include <cpssDriver/pp/config/prvCpssDrvPpCfgInit.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpssDriver/pp/hardware/prvCpssDrvHwCntl.h>
#include <cpssDriver/pp/hardware/prvCpssDrvObj.h>

/**
* @internal prvCpssDrvEventsMask function
* @endinternal
*
* @brief   mask/unmask a given event. A masked event doesn't reach a CPU.
*
* @param[in] devNum                   - The PP's device number to mask / unmask the interrupt for.
* @param[in] intIndex                 - The interrupt cause to enable/disable.
* @param[in] maskEn                   - GT_TRUE mask, GT_FALSE unmask.
*                                       GT_OK on success, or
*                                       GT_FAIL on failure.
*
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
*/
GT_STATUS prvCpssDrvEventsMask
(
    IN  GT_U8   devNum,
    IN  GT_U32  intIndex,
    IN  GT_BOOL maskEn
)
{
    GT_STATUS retVal;

    PRV_CPSS_DRV_CHECK_GEN_FUNC_OBJ_MAC(devNum);

    PRV_CPSS_INT_SCAN_LOCK();
    /* Call appropriate implementation by virtual function */
    retVal = prvCpssDrvPpObjConfig[devNum]->genPtr->drvHwPpEventsMask(devNum,
                                                                      intIndex, maskEn);
    PRV_CPSS_INT_SCAN_UNLOCK();

    return retVal;
}

/**
* @internal prvCpssDrvEventsMaskGet function
* @endinternal
*
* @brief   Gets enable/disable status of a given event reaching the CPU.
*
* @param[in] devNum                   - The PP's device number to mask / unmask the interrupt for.
* @param[in] intIndex                 - The interrupt cause to get enable/disable status.
*
* @param[out] enablePtr                - (pointer to)GT_TRUE enable, GT_FALSE disable.
*/
GT_VOID prvCpssDrvEventsMaskGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  intIndex,
    OUT  GT_BOOL *enablePtr
)
{

    /* Call appropriate implementation by virtual function */
    PRV_CPSS_INT_SCAN_LOCK();
    prvCpssDrvPpObjConfig[devNum]->genPtr->drvHwPpEventsMaskGet(
        devNum, intIndex, enablePtr);
    PRV_CPSS_INT_SCAN_UNLOCK();
}
