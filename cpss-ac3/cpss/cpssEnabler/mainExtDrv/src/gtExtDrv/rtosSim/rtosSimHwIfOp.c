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
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS extDrvHwIfOperExec
(
    IN   GT_U32                         arrayLength,
    IN   GT_HW_IF_UNT                   operationArr[],
    IN   GT_HW_IF_NON_BLOCK_CB_STC      *nonBlockCbPtr
)
{
    GT_U32      i;
    GT_STATUS   retVal;
    
    for (i = 0; i < arrayLength; i++)
    {
        switch (operationArr->coProc[i].opCode)
        {
            case GT_READ_SMI_BUFFER_E:
                retVal = hwIfSmiReadReg(operationArr->smiOper[i].smiDeviceAddr,
                                        operationArr->smiOper[i].regAddress,
                                        &operationArr->smiOper[i].smiData);
                break;
                    
            case GT_WRITE_SMI_BUFFER_E:
                retVal = hwIfSmiWriteReg(operationArr->smiOper[i].smiDeviceAddr,
                                         operationArr->smiOper[i].regAddress,
                                         operationArr->smiOper[i].smiData);
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



