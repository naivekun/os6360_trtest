/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
#include <gtExtDrv/drivers/gtHwIfDrv.h>
#include <gtExtDrv/drivers/gtI2cDrv.h>

/**
* @internal extDrvMgmtReadRegister function
* @endinternal
*
* @brief   Managment read from GT devices register via specified hardware
*         interface.
* @param[in] mgmtIf                   - Managmnet hardware interface to device.
* @param[in] devId                    - Device id defined by interface for accessing device.
*                                      union depends on type of hardware interface.
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
GT_STATUS extDrvMgmtReadRegister
(
    IN GT_MGMT_IF_TYPE      mgmtIf,
    IN GT_DEV_ID            devId,
    IN GT_U32               regAddr,
    OUT GT_U32              *data
)
{
	GT_STATUS rc = GT_OK;

#ifdef IMPL_FA

	if (mgmtIf == GT_MGMT_I2C)
    {
        return gtI2cMgmtReadRegister(devId.i2cBaseAddr, regAddr | 0x80000000,
                                     data);
    }
    else
    {

        return hwFaReadRegister(devId.devNum, regAddr, data);
    }
#endif 
	return rc;

}


/**
* @internal extDrvMgmtWriteRegister function
* @endinternal
*
* @brief   Managment write to GT devices registers via specified hardware
*         interface.
* @param[in] mgmtIf                   - Managmnet hardware interface to device.
* @param[in] devId                    - Device id defined by interface for accessing device.
*                                      union depends on type of hardware interface.
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
GT_STATUS extDrvMgmtWriteRegister
(
    IN GT_MGMT_IF_TYPE      mgmtIf,
    IN GT_DEV_ID            devId,
    IN GT_U32               regAddr,
    IN GT_U32               data
)
{
	GT_STATUS rc = GT_OK;

#ifdef IMPL_FA

	if (mgmtIf == GT_MGMT_I2C)
    {
        return gtI2cMgmtWriteRegister(devId.i2cBaseAddr,
                                       (regAddr&0x7FFFFFFF), data);
      }
    else
    {

        return hwFaWriteRegister(devId.devNum, regAddr, data);
    }
#endif 
	return rc;
}


/**
* @internal extDrvMgmtIsrReadRegister function
* @endinternal
*
* @brief   Managment read from GT devices register, guaranteed to be non blocked
*         access via specified hardware interface.
* @param[in] mgmtIf                   - Managmnet hardware interface to device.
* @param[in] devId                    - Device id defined by interface for accessing device.
*                                      union depends on type of hardware interface.
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
GT_STATUS extDrvMgmtIsrReadRegister
(
    IN GT_MGMT_IF_TYPE      mgmtIf,
    IN GT_DEV_ID            devId,
    IN GT_U32               regAddr,
    OUT GT_U32              *data
)
{
	GT_STATUS rc = GT_OK;

#ifdef IMPL_FA

	if (mgmtIf == GT_MGMT_I2C)
    {
        return gtI2cMgmtReadRegister(devId.i2cBaseAddr, regAddr | 0xC0000000,
                                     data);
    }
    else
    {

        return hwFaIsrRead(devId.devNum, regAddr, data);
    }
#endif 
	return rc;

}


/**
* @internal extDrvMgmtIsrWriteRegister function
* @endinternal
*
* @brief   Managment write to GT devices register, guaranteed to be non blocked
*         access via specified hardware interface.
* @param[in] mgmtIf                   - Managmnet hardware interface to device.
* @param[in] devId                    - Device id defined by interface for accessing device.
*                                      union depends on type of hardware interface.
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
GT_STATUS extDrvMgmtIsrWriteRegister
(
    IN GT_MGMT_IF_TYPE      mgmtIf,
    IN GT_DEV_ID            devId,
    IN GT_U32               regAddr,
    IN GT_U32               data
)
{
	GT_STATUS rc = GT_OK;

#ifdef IMPL_FA

	if (mgmtIf == GT_MGMT_I2C)
    {
        return gtI2cMgmtWriteRegister(devId.i2cBaseAddr,
                                       (regAddr&0x0FFFFFFF), data);
    }
    else
    {

        return hwFaIsrWrite(devId.devNum, regAddr, data);
    }
#endif 
	return rc;
}





