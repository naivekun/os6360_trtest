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
* @file genDrvI2cDrv.c
*
* @brief Functions to control the I2C.
*
* @version   1
********************************************************************************
*/
#include <gtExtDrv/os/extDrvOs.h>
#include <gtExtDrv/drivers/gtI2cDrv.h>
#include <gtExtDrv/drivers/gtIntDrv.h>
#include "i2cDrv/i2cDrv.h"

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
    gtI2cMasterInit(I2C_FREQ_100KHZ,100000000);

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
    GT_STATUS       rc;   /* Return code */
    GT_32           intKey = 0; /* Unlocking.                           */

    /* This must be done. With out this the driver does not work */
    osTimerWkAfter(1);

    /* Close interrupts */
    extDrvSetIntLockUnlock(INTR_MODE_LOCK, &intKey);

    /* Read the register value */
    rc = gtI2cWriteRegister(deviceAddress,regAddr,data);

    /* Open interruts */
    extDrvSetIntLockUnlock(INTR_MODE_UNLOCK, &intKey);

    return (rc);
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
    GT_STATUS rc;
    GT_32           intKey = 0; /* Unlocking.                           */

    /* This must be done. With out this the driver does not work */
    osTimerWkAfter(1);

    /* Close interrupts */
    extDrvSetIntLockUnlock(INTR_MODE_LOCK, &intKey);

    /* Read the value of the register */
    rc = gtI2cReadRegister(deviceAddress,regAddr,data);

    /* Open interruts */
    extDrvSetIntLockUnlock(INTR_MODE_UNLOCK, &intKey);

    return (rc);
}

/**
* @internal extDrvI2cRead function
* @endinternal
*
* @brief   This routine receive I2C data with optional offset.
*
* @param[in] drv           - driver pointer.
* @param[in] bus_id        - the I2C bus id.
* @param[in] slave_address - the target device slave address.
* @param[in] offset_type   - offset mode.
* @param[in] offset        - internal offset in slave to read
*       from.
* @param[in] buffer_size   - buffer length.
*
* @param[out] buffer     - received buffer with the read data.
*
* @retval GT_OK                - on success,
* @retval GT_NOT_INITIALIZED   - driver not initialized,
* @retval GT_NO_RESOURCE       - fail to open bus (file) driver,
* @retval GT_FAIL              - othersise.
*/
GT_STATUS extDrvI2cRead(
    IN  CPSS_HW_DRIVER_STC *drv,
    IN  GT_U8     bus_id,
    IN  GT_U8     slave_address,
    IN  EXT_DRV_I2C_OFFSET_TYPE_ENT   offset_type,
    IN  GT_U32    offset,
    IN  GT_U32    buffer_size,
    OUT GT_U8    *buffer
)
{
    (void)drv;
    (void)bus_id;
    (void)slave_address;
    (void)offset_type;
    (void)offset;
    (void)buffer_size;
    (void)buffer;

    return GT_NOT_IMPLEMENTED;
}

/**
* @internal extDrvI2cWrite function
* @endinternal
*
* @brief   This routine transmit I2C data with optional offset.
*
* @param[in] drv           - driver pointer.
* @param[in] bus_id        - the I2C bus id.
* @param[in] slave_address - the target device slave address.
* @param[in] offset_type   - offset mode.
* @param[in] offset        - internal offset in slave to write
*       to.
* @param[in] buffer_size   - buffer length.
* @param[in] buffer        - buffer with the data to write.
*
* @retval GT_OK                - on success,
* @retval GT_NOT_INITIALIZED   - driver not initialized,
* @retval GT_NO_RESOURCE       - fail to open bus (file) driver,
* @retval GT_FAIL              - othersise.
*/
GT_STATUS extDrvI2cWrite(
    IN CPSS_HW_DRIVER_STC *drv,
    IN GT_U8     bus_id,
    IN GT_U8     slave_address,
    IN EXT_DRV_I2C_OFFSET_TYPE_ENT   offset_type,
    IN GT_U32    offset,
    IN GT_U32    buffer_size,
    IN GT_U8    *buffer
)
{
    (void)drv;
    (void)bus_id;
    (void)slave_address;
    (void)offset_type;
    (void)offset;
    (void)buffer_size;
    (void)buffer;

    return GT_NOT_IMPLEMENTED;
}

/**
* @internal extCpssHwDriverI2cCreateDrv function
* @endinternal
*
* @param[in] parent  - parent driver
*
* @brief Create I2C driver
*
* @retval CPSS_HW_DRIVER_STC* or NULL
*/
CPSS_HW_DRIVER_STC *extDrvI2cCreateDrv(
    IN CPSS_HW_DRIVER_STC *parent
)
{
    (void)parent;

    return NULL;
}
