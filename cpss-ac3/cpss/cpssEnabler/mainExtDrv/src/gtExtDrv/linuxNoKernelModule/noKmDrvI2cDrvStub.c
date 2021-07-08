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
* @file noKmDrvI2cDrvStub.c
*
* @brief Functions to control the I2C.
*
* @version   1
********************************************************************************
*/
#include <gtExtDrv/os/extDrvOs.h>
#include <gtExtDrv/drivers/gtI2cDrv.h>
#include <gtExtDrv/drivers/gtIntDrv.h>
#if 0
#include "i2cDrv/i2cDrv.h"
#endif



/********* Internal functions *************************************************/



/********* Variables **********************************************************/

/**
* @internal gtI2cMgmtSlaveInit function
* @endinternal
*
*/
GT_VOID gtI2cMgmtSlaveInit
(
    GT_VOID
)
{

}

/**
* @internal gtI2cMgmtMasterInit function
* @endinternal
*
*/
GT_STATUS gtI2cMgmtMasterInit
(
    GT_VOID
)
{
    return GT_FAIL;
}


/**
* @internal gtI2cMgmtWriteRegister function
* @endinternal
*
* @brief   This function utilizes managment write registers to GT devices
*         via I2c interface.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
*
* @note Galtis:
*       None.
*
*/
GT_STATUS gtI2cMgmtWriteRegister
(
    IN GT_U16   deviceAddress,
    IN GT_U32   regAddr,
    IN GT_U32   data
)
{
    (void)deviceAddress;
    (void)regAddr;
    (void)data;
    return GT_FAIL;
}


/**
* @internal gtI2cMgmtReadRegister function
* @endinternal
*
* @brief   This function utilizes managment register read from GT devices
*         via I2c interface.
*
* @param[out] data                     - Data word read from device, is valid only
*                                      if status == GT_OK.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
*
* @note Galtis:
*       None.
*
*/
GT_STATUS gtI2cMgmtReadRegister
(
    IN GT_U16   deviceAddress,
    IN GT_U32   regAddr,
    OUT GT_U32  *data
)
{
    (void)deviceAddress;
    (void)regAddr;
    (void)data;
    return GT_FAIL;
}


