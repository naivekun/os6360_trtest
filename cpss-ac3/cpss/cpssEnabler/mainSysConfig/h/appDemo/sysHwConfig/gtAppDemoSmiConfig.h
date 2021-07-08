/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
#ifndef __gtAppDemoSmiConfigh
#define __gtAppDemoSmiConfigh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <gtExtDrv/drivers/gtSmiDrv.h>
#include <appDemo/os/appOs.h>


#define GT_SMI_VENDOR_ID    0x11AB

/**
* @struct GT_SMI_DEV_VENDOR_ID
 *
 * @brief SMI device and vendor ID struct
*/
typedef struct{

    /** The Prestera SMI vendor Id. */
    GT_U16 vendorId;

    /** The different Prestera SMI device Id. */
    GT_U16 devId;

} GT_SMI_DEV_VENDOR_ID;


/**
* @struct GT_SMI_INFO
 *
 * @brief SMI device information
*/
typedef struct{

    /** The Prestera SMI vendor Id. */
    GT_SMI_DEV_VENDOR_ID smiDevVendorId;

    GT_U32 smiIdSel;

} GT_SMI_INFO;



/**
* @internal gtPresteraGetSmiDev function
* @endinternal
*
* @brief   This routine search for Prestera Devices Over the SMI.
*
* @param[in] first                    - whether to bring the  device, if GT_FALSE return the next
*                                      device.
*
* @param[out] smiInfo                  - the next device SMI info.
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
GT_STATUS gtPresteraGetSmiDev
(
    IN  GT_BOOL     first,
    OUT GT_SMI_INFO *smiInfo
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __gtAppDemoSmiConfigh */


