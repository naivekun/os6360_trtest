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
* @file gtAppDemoPciConfig.h
*
* @brief Prestera Devices pci initialization & detection module.
*
* @version   3
********************************************************************************
*/
#ifndef __gtAppDemoPciConfigh
#define __gtAppDemoPciConfigh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <gtExtDrv/drivers/gtPciDrv.h>
#include <appDemo/os/appOs.h>


#define GT_PCI_VENDOR_ID    0x11AB


/**
* @struct GT_PCI_DEV_VENDOR_ID
 *
 * @brief PCI device and vendor ID struct
*/
typedef struct{

    /** The Prestera PCI vendor Id. */
    GT_U16 vendorId;

    /** The different Prestera PCI device Id. */
    GT_U16 devId;

} GT_PCI_DEV_VENDOR_ID;


/**
* @struct GT_PCI_INFO
 *
 * @brief PCI device information
*/
typedef struct{

    /** The Prestera PCI vendor Id. */
    GT_PCI_DEV_VENDOR_ID pciDevVendorId;

    /** The device PCI ID_SEL. */
    GT_U32 pciIdSel;

    /** The PCI bus number. */
    GT_U32 pciBusNum;

    /** The PCI device function number */
    GT_U32 funcNo;

    GT_U32 pciHeaderInfo[16];

} GT_PCI_INFO;


/**
* @internal prvPresteraPciInfoDump function
* @endinternal
*
* @brief   Dump PCI info
*
* @param[in] p                        - the device PCI info.
*                                       None.
*
* @note 
*       Galtis:
*       None.
*       Toolkit:
*
*/
GT_VOID prvPresteraPciInfoDump
(
    IN GT_PCI_INFO   *p
);

/**
* @internal gtPresteraGetPciDev function
* @endinternal
*
* @brief   This routine search for Prestera Devices Over the PCI.
*
* @param[in] first                    - whether to bring the  device, if GT_FALSE return the next
*                                      device.
*
* @param[out] pciInfo                  - the next device PCI info.
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
GT_STATUS gtPresteraGetPciDev
(
    IN  GT_BOOL     first,
    OUT GT_PCI_INFO *pciInfo
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __gtAppDemoPciConfigh */


