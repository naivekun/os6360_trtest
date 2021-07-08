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
* @file gtAppDemoTwsiConfig.h
*
* @brief Prestera Devices TWSI initialization & detection module.
*
* @version   1.1.2.1
********************************************************************************
*/
#ifndef __gtAppDemoTwsiConfigh
#define __gtAppDemoTwsiConfigh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <gtExtDrv/drivers/gtTwsiHwCtrl.h>
#include <appDemo/os/appOs.h>


#define GT_TWSI_VENDOR_ID    0x11AB

/**
* @struct GT_TWSI_DEV_VENDOR_ID
 *
 * @brief SMI device and vendor ID struct
*/
typedef struct{

    /** The Prestera TWSI vendor Id. */
    GT_U16 vendorId;

    /** The different Prestera TWSI device Id. */
    GT_U16 devId;

} GT_TWSI_DEV_VENDOR_ID;


/**
* @struct GT_TWSI_INFO
 *
 * @brief TWSI device information
*/
typedef struct{

    GT_TWSI_DEV_VENDOR_ID twsiDevVendorId;

    GT_U32 twsiIdSel;

} GT_TWSI_INFO;



/**
* @internal gtPresteraGetTwsiDev function
* @endinternal
*
* @brief   This routine search for Prestera Devices Over the TWSI.
*
* @param[in] first                    - whether to bring the  device, if GT_FALSE return the next
*                                      device.
*
* @param[out] twsiInfo                 - the next device TWSI info.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - otherwise.
* @retval GT_NO_MORE               - no more prestera devices.
*
* @note 
*       Galtis:
*       None.
*       Toolkit:
*
*/
GT_STATUS gtPresteraGetTwsiDev
(
    IN  GT_BOOL      first,
    OUT GT_SMI_INFO *twsiInfo
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __gtAppDemoTwsiConfigh */


