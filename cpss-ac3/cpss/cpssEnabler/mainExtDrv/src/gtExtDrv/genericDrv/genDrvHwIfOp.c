/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file genDrvHwIfOp.c
*
* @brief API implementation for Hw interface operations.
*
*/

/*Includes*/
#include <gtExtDrv/os/extDrvOs.h>
#include <gtExtDrv/drivers/gtHwIfOp.h>
#include <gtExtDrv/drivers/gtSmiHwCtrl.h>
#include <intLib.h>

extern GT_STATUS miiInfSmiWriteAsicRegArr
(
    IN   GT_U32                         arrayLength,
    IN   GT_HW_IF_UNT                   operationArr[],
    IN   GT_HW_IF_NON_BLOCK_CB_STC      *nonBlockCbPtr,
    IN   GT_U32                         mode
);

/**
* @internal extDrvHwIfOperExec function
* @endinternal
*
* @brief   This routine performs co-processor operations according to data in the
*         operationLength entries. The routine can be blocking or non-blocking.
* @param[in] arrayLength              - Array length.
* @param[in] operationArr[]           - Array of operations to be perform by the co-processor.
* @param[in] nonBlockCbPtr            - A struct containing the data needed for non blocking
*                                      operation (callback routine and parameter). If NULL,
*                                      the routine call is blocking.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS extDrvHwIfOperExec
(
    IN   GT_U32                         arrayLength,
    IN   GT_HW_IF_UNT                   operationArr[],
    IN   GT_HW_IF_NON_BLOCK_CB_STC      *nonBlockCbPtr
)
{
    GT_STATUS                   retVal;
    GT_U32                      i;
#ifndef CPU_EMULATED_BUS
    GT_BOOL                     inIntContext = (GT_BOOL)intContext();
#endif
    
    for (i = 0; i < arrayLength; i++)
    {
        switch (operationArr->coProc[i].opCode)
        {
            case GT_READ_SMI_BUFFER_E:
#ifdef CPU_EMULATED_BUS
                return miiInfSmiWriteAsicRegArr(arrayLength, operationArr, 
                                                nonBlockCbPtr, 1);
#else
                if (GT_TRUE == inIntContext)
                {
                    retVal = hwIfSmiInterruptReadReg(
                                        operationArr->smiOper[i].smiDeviceAddr,
                                        operationArr->smiOper[i].regAddress,
                                        &operationArr->smiOper[i].smiData);
                }
                else
                {
                    retVal = hwIfSmiTaskReadReg(
                                        operationArr->smiOper[i].smiDeviceAddr,
                                        operationArr->smiOper[i].regAddress,
                                        &operationArr->smiOper[i].smiData);
                }
#endif
                break;
                
            case GT_WRITE_SMI_BUFFER_E:
#ifdef CPU_EMULATED_BUS
                return miiInfSmiWriteAsicRegArr(arrayLength, operationArr, 
                                                nonBlockCbPtr, 1);
#else
                if (GT_TRUE == inIntContext)
                {
                    retVal = hwIfSmiInterruptWriteReg(
                                        operationArr->smiOper[i].smiDeviceAddr,
                                        operationArr->smiOper[i].regAddress,
                                        operationArr->smiOper[i].smiData);
                }
                else
                {
                    retVal = hwIfSmiTaskWriteReg(
                                        operationArr->smiOper[i].smiDeviceAddr,
                                        operationArr->smiOper[i].regAddress,
                                        operationArr->smiOper[i].smiData);
                }
#endif
            break;
                                 
            case GT_COPROC_READ_COUNTERS_E:
            case GT_COPROC_READ_MIBS_E:
            case GT_COPROC_CLEAR_SRAM_E:
                retVal = GT_FAIL;
                break;
            
            default:
                return GT_FAIL;
        }
        
        if (retVal != GT_OK)
        {
            return GT_FAIL;
        }
    }
    
    return GT_OK;
}



