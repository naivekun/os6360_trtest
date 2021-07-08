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
* @file gtAppDemoSmiConfig.c
*
* @brief Prestera Devices smi initialization & detection module.
*
* @version   8
********************************************************************************
*/
#include <appDemo/sysHwConfig/gtAppDemoSmiConfig.h>
#include <gtExtDrv/drivers/gtSmiHwCtrl.h>

#include <cpss/common/cpssTypes.h>


#ifndef PRESTERA_NO_HW
#include <appDemo/sysHwConfig/gtAppDemoPciConfig.h>
#endif

/*******************************************************************************
* internal definitions
*******************************************************************************/
#ifdef PRESTERA_DEBUG
#define APP_INIT_DEBUG
#endif

#ifdef APP_INIT_DEBUG
#define DBG_INFO(x)     osPrintf x
#else
#define DBG_INFO(x)
#endif


#define DEV_ID(gtDevice) ((gtDevice)>>16)
/*******************************************************************************
* Internal usage environment parameters
*******************************************************************************/
static GT_SMI_DEV_VENDOR_ID device[] =
{
    /* lion2 Hooper devices */
    {GT_SMI_VENDOR_ID,DEV_ID(CPSS_98CX8121_CNS)},
    {GT_SMI_VENDOR_ID,DEV_ID(CPSS_98CX8123_CNS)},
    {GT_SMI_VENDOR_ID,DEV_ID(CPSS_98CX8124_CNS)},
    {GT_SMI_VENDOR_ID,DEV_ID(CPSS_98CX8129_CNS)},

    {0,0}
};

#ifndef PRESTERA_NO_HW

/*******************************************************************************
* gtPresteraGetSmiDev
*
* DESCRIPTION:
*       This routine searches for Prestera Devices Over the SMI.
*
* INPUTS:
*       first - whether to bring the first device, if GT_FALSE return the next
*               device.
*
* OUTPUTS:
*       smiInfo <- the next device SMI info.
*
* RETURNS:
*       GT_OK      - on success.
*       GT_FAIL    - otherwise.
*       GT_NO_MORE - no more prestera devices.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS gtPresteraGetSmiDev
(
    IN  GT_BOOL     first,
    OUT GT_SMI_INFO *smiInfo
)
{
    static GT_U32   deviceIdx = 0;      /* device id index      */
    GT_U32          i;                  /* device instance      */
    GT_U16          vendorId;
    GT_U16          devId;
    GT_U16          devNum;

    /* check parameters */
    if(smiInfo == NULL)
    {
        return GT_BAD_PARAM;
    }
    /* check whether first call */
    if(first == GT_TRUE)
    {
                /* Hooper devices connected to 0-3 */
        deviceIdx = 0x0;
    }
    /* call the BSP SMI facility to get the next Prestera device */
    for (i = deviceIdx; i < 32; i++)
    {
        if(extDrvSmiDevVendorIdGet(&vendorId,
                                   &devId,
                                   i) != GT_OK)
        {
            continue;
        }

        /* a device has been found, search in the device list */
        /* if it exists add it to the device list             */
        for (devNum = 0; device[devNum].vendorId != 0 ; devNum++)
        {
            if (device[devNum].vendorId == vendorId &&
                device[devNum].devId == devId)
            {
                DBG_INFO(("found the SMI device 0x%04x\n", devId));

                smiInfo->smiDevVendorId.devId       = devId;
                smiInfo->smiDevVendorId.vendorId    = vendorId;
                smiInfo->smiIdSel                   = i;

                deviceIdx = i + 1;

                return GT_OK;
            }
        }
    }
    return GT_NO_MORE;
}

#else


/**
* @internal gtPresteraGetSmiDev function
* @endinternal
*
* @brief   This routine search for Prestera Devices Over the SMI.
*
* @param[in] first                    - whether to bring the  device, if GT_FALSE return the next
*                                      device.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - otherwise.
* @retval GT_NO_MORE               - no more prestera devices.
*/
GT_STATUS gtPresteraGetSmiDev
(
    IN  GT_BOOL     first,
    OUT GT_SMI_INFO *smiInfo
)
{
    static GT_U32 instance;

    /* check parameters */
    if(smiInfo == NULL)
    {
        return GT_BAD_PARAM;
    }
    if(first == GT_TRUE)
    {
        instance = 0;
    }
    if(i >= 2)
    {
        return GT_NO_MORE;
    }
    i++;
    smiInfo->smiDevVendorId.devId = device[0].devId;
    smiInfo->smiDevVendorId.vendorId = device[0].vendorId;

    return GT_OK;
}

#endif



