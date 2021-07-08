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
* @file gtHwIfDrv.h
*
* @brief Enable managment hw access to GT devices via selected hardware
* interfaces.
*
*/


#ifndef __gtHwIfDrvh
#define __gtHwIfDrvh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <gtExtDrv/os/extDrvOs.h>

/**
* @enum GT_MGMT_IF_TYPE
 *
 * @brief Managmnet hardware interface of GT device.
*/
typedef enum{

    /** @brief GT device is managed through the packet processor
     *  uplink interface (i.e. packet processor PCI).
     */
    GT_MGMT_VIA_PP = 0,

    /** @brief GT device is managed through I2C managment
     *  interface.
     */
    GT_MGMT_I2C

} GT_MGMT_IF_TYPE;


/**
* @union GT_DEV_ID
 *
 * @brief Device id defined by interface for accessing device.
 * union depends on type of hardware interface.
 *
*/

typedef union{
    /** @brief Device number of connected packet processor through
     *  uplink bus, for PCI interface access.
     */
    GT_U8 devNum;

    /** @brief Base address to which GT device is mapped in the I2c
     *  interface.
     *  Comments:
     */
    GT_U16 i2cBaseAddr;

} GT_DEV_ID;



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
);

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
);

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
);

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
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif



