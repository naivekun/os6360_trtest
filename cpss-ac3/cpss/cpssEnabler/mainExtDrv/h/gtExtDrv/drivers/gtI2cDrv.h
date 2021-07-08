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
* @file gtI2cDrv.h
*
* @brief Function to control the I2C unit.
*
*/


#ifndef __gtI2cDrvh
#define __gtI2cDrvh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/********* Include files ******************************************************/
#include <gtExtDrv/os/extDrvOs.h>

#include <cpssDriver/pp/hardware/private/prvCpssDriverCreate.h>

/********* Typedefs ***********************************************************/

/**
* @internal gtI2cMgmtMasterInit function
* @endinternal
*
*/
GT_STATUS gtI2cMgmtMasterInit
(
    GT_VOID
);

/**
* @internal gtI2cMgmtSlaveInit function
* @endinternal
*
*/
GT_VOID gtI2cMgmtSlaveInit
(
    GT_VOID
);

/**
* @internal gtI2cMgmtReadRegister function
* @endinternal
*
* @brief   Managment read from GT devices register via I2c interface.
*
* @param[in] i2cBaseAddr              - Base address to which GT device is mapped in the I2c
*                                      interface.
* @param[in] regAddr                  - The device register address to read.
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
    IN  GT_U16  i2cBaseAddr,
    IN  GT_U32  regAddr,
    OUT GT_U32  *data
);


/**
* @internal gtI2cMgmtWriteRegister function
* @endinternal
*
* @brief   Managment write to GT devices registers via specified hardware
*         interface.
* @param[in] i2cBaseAddr              - Base address to which GT device is mapped in the I2c
*                                      interface.
* @param[in] regAddr                  - The device register address to write.
* @param[in] data                     - Data word to write to device.
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
    IN  GT_U16  i2cBaseAddr,
    IN  GT_U32  regAddr,
    IN GT_U32   data
);


/* I2C offset type definitions */
typedef enum {
    EXT_DRV_I2C_OFFSET_TYPE_NONE_E,
    EXT_DRV_I2C_OFFSET_TYPE_8_E,
    EXT_DRV_I2C_OFFSET_TYPE_16_E
} EXT_DRV_I2C_OFFSET_TYPE_ENT;

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
);

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
);

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
);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif



