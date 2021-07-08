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
* @file gtAppDemoPciConfig.c
*
* @brief Prestera Devices pci initialization & detection module.
*
* @version   28
********************************************************************************
*/

#include <appDemo/sysHwConfig/gtAppDemoPciConfig.h>
#include <appDemo/sysHwConfig/gtAppDemoSmiConfig.h>
#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>

#include <gtExtDrv/drivers/gtSmiHwCtrl.h>

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

/*******************************************************************************
* Internal usage environment parameters
*******************************************************************************/

#ifdef ASIC_SIMULATION
    #include <asicSimulation/SCIB/scib.h>
#endif /* ASIC_SIMULATION */

#define MAX_DEV_IDS_CNS  300

static GT_PCI_DEV_VENDOR_ID device[MAX_DEV_IDS_CNS] =
{
    {0,0}/* --> must be last 'Non CPSS' device */


    /************************************************************/
    /* entries here will be filled in runtime from the CPSS DB  */
    /* see function initDevArray()                              */
    /* NOTE : this function will arange the Dx devices as last  */
    /* devices to support VB with ExMx/ExMxPm device with the Dx*/
    /* device(s)                                                */
    /***********************************************************/

};

static GT_STATUS initDevArray(void);

#define END_OF_TABLE    0xFFFFFFFF
/* DB to hold the device types that the CPSS support */
extern const struct {
    CPSS_PP_FAMILY_TYPE_ENT     devFamily;
    GT_U32                      numOfPorts;
    const CPSS_PP_DEVICE_TYPE   *devTypeArray;
    const CPSS_PORTS_BMP_STC    *defaultPortsBmpPtr;
    const /*CORES_INFO_STC*/void *coresInfoPtr;
    const GT_U32                *activeCoresBmpPtr;
}cpssSupportedTypes[];

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
)
{
    int ii;
    int hdr_size = (sizeof(p->pciHeaderInfo) / sizeof(p->pciHeaderInfo[0]));
    GT_U32* h = &(p->pciHeaderInfo[0]);
    osPrintf("\n PCI INFO: \n");
    osPrintf("pciDevVendorId = 0x%X\n", p->pciDevVendorId);
    osPrintf("pciBusNum=0x%x pciIdSel = 0x%X, funcNo = 0x%X\n",
        p->pciBusNum, p->pciIdSel, p->funcNo);
    osPrintf("pciHeaderInfo: \n");
    for (ii = 0; (ii < hdr_size); ii++)
    {
        if ((ii > 0) && ((ii % 4) == 0))
        {
            osPrintf("\n");
        }
        osPrintf("0x%08X ", h[ii]);
    }
    osPrintf("\n");
}

/**
* @internal gtPresteraGetPciDev function
* @endinternal
*
* @brief   This routine search for Prestera Devices Over the PCI.
*
* @param[in] first                 - whether to bring the first device, if GT_FALSE return the next
*                                    device.
*
* @param[out] pciInfo              - the next device PCI info.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - otherwise.
* @retval GT_NO_MORE               - no more prestera devices.
*/
GT_STATUS gtPresteraGetPciDev
(
    IN  GT_BOOL     first,
    OUT GT_PCI_INFO *pciInfo
)
{
    GT_U32  busNo;                  /* pci bus number */
    GT_U32  deviceNo;               /* PCI device number */
    GT_U32  funcNo;                 /* PCI function number */
    static GT_U32 deviceIdx = 0;    /* device id index */
    static GT_U32 instance = 0;     /* device instance */
    static GT_U32 numOfDevices = 0;/* total number of devices in the system */
    GT_U8   i;
    GT_STATUS ret;                  /* function return value */

    /* check parameters */
    if(pciInfo == NULL)
    {
        return GT_BAD_PARAM;
    }
    /* check whether it is the first call */
    if(first == GT_TRUE)
    {
        deviceIdx = 0;
        instance = 0;
        numOfDevices = 0;

        ret = initDevArray();
        CPSS_ENABLER_DBG_TRACE_RC_MAC("initDevArray", ret);
        if(ret != GT_OK)
        {
            return ret;
        }
    }

#ifdef ASIC_SIMULATION
    /* optimize the number of times calling the device to get PCI/PEX info */
    {
        GT_U32  regData;
        for(/*continue*/;instance < 256; instance++)
        {
            scibPciRegRead(instance, 0, 1, &regData);
            if(regData == 0xFFFFFFFF)
            {
                /* the 'instance' is not exists on the PCI/PEX bus */
                continue;
            }

            for(deviceIdx = 0; device[deviceIdx].vendorId ; deviceIdx++)
            {
                /* check if the device is 'known device' */
                if(regData == (device[deviceIdx].vendorId  |  (GT_U32)(device[deviceIdx].devId << 16)))
                {
                    busNo = 0;/* not used by asic simulation */
                    deviceNo = instance;
                    funcNo = 0;/* not used by asic simulation */

                    /* found device , update instance for next iteration*/
                    instance++;

                    goto foundInstanceOnPex_lbl;
                }
            }
        }
        /* get here only when not doing 'goto foundInstanceOnPex_lbl' */
        return GT_NO_MORE;

    }
#endif /*ASIC_SIMULATION*/


    /*call the BSP PCI facility to get all Prestera devices */
    for(;device[deviceIdx].vendorId != 0;)
    {
        DBG_INFO(("search the PCI devices 0x%04x\n",device[deviceIdx].devId));
        if(extDrvPciFindDev(device[deviceIdx].vendorId, device[deviceIdx].devId,
                            instance++, &busNo, &deviceNo, &funcNo) != GT_OK)
        {
            DBG_INFO(("move to the next device\n"));
            instance = 0; /* first instance for that device type */
            deviceIdx++;
            continue;
        }

#ifdef ASIC_SIMULATION
    foundInstanceOnPex_lbl:
#endif /*ASIC_SIMULATION*/

        numOfDevices++;
        DBG_INFO(("found Prestera device\n"));

        pciInfo->pciDevVendorId = device[deviceIdx];
        pciInfo->pciIdSel   = deviceNo;
        pciInfo->pciBusNum  = busNo;
        pciInfo->funcNo     = funcNo;

        /* Get the Pci header info  */
        for(i = 0; i < 64; i += 4)
        {
            ret = extDrvPciConfigReadReg(busNo,deviceNo,funcNo,i,
                                         &(pciInfo->pciHeaderInfo[i / 4]));
            if(ret != GT_OK)
            {
                DBG_INFO(("Failed In PCI configuration\n"));

                return GT_FAIL;
            }
        }

        return GT_OK;
    }
    return GT_NO_MORE;
}


#define IS_DEVICE_CAN_BE_VB_MUX_DEVICE_MAC(devId)    \
    ((((devId) >> 12) == 0xD) ? 1 :    /*dxCh devices*/ \
     (((devId) >> 8)  == 0x0D) ? 1 :   /*dxSal devices*/\
     0)

#define IS_XCAT_DEV_CPU_ENABLED_MAC(devId)    \
        ((((devId) & 0x2) == 0) ? GT_TRUE : GT_FALSE)

/* init the device[] array                                                    */
/* NOTES :                                                                    */
/* 1.This function will arange the Dx devices as last                         */
/* devices to support VB with ExMx/ExMxPm device with the Dx                  */
/* device(s)                                                                  */
/* 2.The XCAT devices with CPU enabled will be last.                          */
/* This is done to support existing tests for B2B XCAT boards                 */
static GT_STATUS initDevArray(void)
{
    GT_U32 ii,jj,kk;
    GT_U32  devId;
    static GT_BOOL  initWasDone = GT_FALSE;
    static GT_STATUS    firstStatus = GT_OK;

    if(initWasDone == GT_FALSE)
    {
        initWasDone = GT_TRUE;
    }
    else
    {
        return firstStatus;
    }

    /* loop on devices that are 'Non CPSS' devices , and we need to look for the end of them */
    for(ii = 0 ; ii < MAX_DEV_IDS_CNS;ii++)
    {
        if(device[ii].vendorId == 0)
        {
            break;
        }
    }

    if(ii == MAX_DEV_IDS_CNS)
    {
        firstStatus = GT_FULL;
        return firstStatus;
    }

    /* now ii is the index to start the adding to device[] of the cpss devices */


    /***********************************************************************/
    /*on first loop add only devices that can't be use as MUX device in VB */
    /***********************************************************************/

    jj=0;
    while(cpssSupportedTypes[jj].devFamily != END_OF_TABLE)
    {
        kk = 0;
        while(cpssSupportedTypes[jj].devTypeArray[kk] != END_OF_TABLE)
        {
            devId = (cpssSupportedTypes[jj].devTypeArray[kk] >> 16);
            if(IS_DEVICE_CAN_BE_VB_MUX_DEVICE_MAC(devId))
            {
                /* the device can be mux so we skip it on the first loop of devices */
                /* we will add those devices on the 'Second loop' */
                /* XCAT devices can be mux */
                kk++;

                /* don't increment ii here */

                continue;
            }

            device[ii].vendorId = (GT_U16)(cpssSupportedTypes[jj].devTypeArray[kk] & 0xFFFF);
            device[ii].devId    = (GT_U16) devId;

            ii++;
            if(ii == MAX_DEV_IDS_CNS)
            {
                firstStatus = GT_FULL;
                return firstStatus;
            }

            kk++;
        }
        jj++;
    }

    /**********************************************************************/
    /*on the second loop add only devices that CAN be use as MUX device in VB */
    /**********************************************************************/
    jj=0;
    while(cpssSupportedTypes[jj].devFamily != END_OF_TABLE)
    {
        kk = 0;
        while(cpssSupportedTypes[jj].devTypeArray[kk] != END_OF_TABLE)
        {
            devId = (cpssSupportedTypes[jj].devTypeArray[kk] >> 16);
            if(!IS_DEVICE_CAN_BE_VB_MUX_DEVICE_MAC(devId) ||
               (cpssSupportedTypes[jj].devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E &&
                IS_XCAT_DEV_CPU_ENABLED_MAC(devId)))

            {
                /* the device CAN'T be mux or XCAT dev with CPU enabled,*/
                /* so we skip it on the second loop of devices */
                kk++;

                /* don't increment ii here */

                continue;
            }
            device[ii].vendorId = (GT_U16)(cpssSupportedTypes[jj].devTypeArray[kk] & 0xFFFF);
            device[ii].devId    = (GT_U16) devId;

            ii++;
            if(ii == MAX_DEV_IDS_CNS)
            {
                firstStatus = GT_FULL;
                return firstStatus;
            }

            kk++;
        }
        jj++;
    }

    /**********************************************************************/
    /*on the third loop add XCAT devices with CPU enabled                     */
    /**********************************************************************/
    jj=0;
    while(cpssSupportedTypes[jj].devFamily != END_OF_TABLE)
    {
        kk = 0;
        if (cpssSupportedTypes[jj].devFamily != CPSS_PP_FAMILY_DXCH_XCAT_E)
        {
            jj++;
            continue;
        }
        while(cpssSupportedTypes[jj].devTypeArray[kk] != END_OF_TABLE)
        {
            devId = (cpssSupportedTypes[jj].devTypeArray[kk] >> 16);
            if (!IS_XCAT_DEV_CPU_ENABLED_MAC(devId))
            {
                kk++;
                continue;
            }
            device[ii].vendorId = (GT_U16)(cpssSupportedTypes[jj].devTypeArray[kk] & 0xFFFF);
            device[ii].devId    = (GT_U16) devId;

            ii++;
            if(ii == MAX_DEV_IDS_CNS)
            {
                firstStatus = GT_FULL;
                return firstStatus;
            }

            kk++;
        }
        jj++;
    }

    /* put here ONLY devices that are 'unknown' to the CPSS */
    device[ii].vendorId =  GT_PCI_VENDOR_ID;
    device[ii++].devId    = 0xF950;

    if(ii == MAX_DEV_IDS_CNS)
    {
        firstStatus = GT_FULL;
        return firstStatus;
    }

    device[ii].vendorId = 0;/* set new 'end of array' */

    firstStatus = GT_OK;
    return firstStatus;
}



