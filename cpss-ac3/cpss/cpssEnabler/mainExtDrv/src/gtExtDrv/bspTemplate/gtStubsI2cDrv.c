/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/

/*includes*/
#include <gtExtDrv/os/extDrvOs.h>
#include <gtExtDrv/drivers/gtI2cDrv.h>

#ifdef PRESTERA_DEBUG
#define I2C_DEBUG
#endif

#ifdef I2C_DEBUG
#define DBG_INFO(x)     osPrintf x
#else
#define DBG_INFO(x)
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

    return (GT_OK);
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
    return (GT_OK);
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
	return (GT_OK);
}





