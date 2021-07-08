/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/

/*Includes*/
#include <gtExtDrv/os/extDrvOs.h>
#include <gtExtDrv/drivers/gtHwIfOp.h>
#include <gtExtDrv/drivers/gtSmiHwCtrl.h>


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
    GT_STATUS   retVal;
    GT_U32      i;
    static GT_SEM   hwOpLock = NULL;
    
    if (NULL == hwOpLock)
    {
        if(osSemBinCreate("hwOpLock", OS_SEMB_FULL, &hwOpLock) != GT_OK)
        {
            hwOpLock = NULL;
            return GT_FAIL;
        }    
    }

    osSemWait(hwOpLock, OS_WAIT_FOREVER);
    
    for (i = 0; i < arrayLength; i++)
    {
        switch (operationArr->coProc[i].opCode)
        {
            case GT_READ_SMI_BUFFER_E:
#ifdef CPU_EMULATED_BUS
                retVal = miiInfSmiWriteAsicRegArr(arrayLength, operationArr, 
                                                  nonBlockCbPtr, 1);
                osSemSignal(hwOpLock);
                return retVal;
#else
                retVal = hwIfSmiTaskReadReg(operationArr->smiOper[i].smiDeviceAddr,
                                        operationArr->smiOper[i].regAddress,
                                        &operationArr->smiOper[i].smiData);
#endif
                break;
                
            case GT_WRITE_SMI_BUFFER_E:
#ifdef CPU_EMULATED_BUS
                retVal = miiInfSmiWriteAsicRegArr(arrayLength, operationArr, 
                                                  nonBlockCbPtr, 1);
                osSemSignal(hwOpLock);
                return retVal;
#else
                retVal = hwIfSmiTaskWriteReg(operationArr->smiOper[i].smiDeviceAddr,
                                         operationArr->smiOper[i].regAddress,
                                         operationArr->smiOper[i].smiData);
#endif
            break;
                                 
            case GT_COPROC_READ_COUNTERS_E:
            case GT_COPROC_READ_MIBS_E:
            case GT_COPROC_CLEAR_SRAM_E:
                retVal = GT_FAIL;
                break;
            
            default:
                osSemSignal(hwOpLock);
                return GT_FAIL;
        }
        
        if (retVal != GT_OK)
            break;
    }
    
    osSemSignal(hwOpLock);
    
    return (retVal == GT_OK) ? GT_OK : GT_FAIL;
}



