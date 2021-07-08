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
* @file gtAppDemoGenSysConfig.c
*
* @brief General System configuration and initialization control.
*
*/
#include <appDemo/os/appOs.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>
#include <appDemo/boardConfig/appDemoBoardConfig.h>

#include <cpss/common/config/private/prvCpssConfigTypes.h>
/* flag to indicate wthether this is normal or hir app */
GT_BOOL isHirApp;


/*******************************************************************************
 * Global variables
 ******************************************************************************/

/* becomes TRUE after first initialization of the system, stays TRUE after
 the shutdown */
GT_BOOL systemInitialized = GT_FALSE;

/* appDemoPpConfigList - Holds the Pp's configuration parameters.
   allocate additional entries for multi portgroup devices. Because the PCI/PEX
   scan fill in DB starting from first SW devNum and for all port groups */
APP_DEMO_PP_CONFIG appDemoPpConfigList[APP_DEMO_PP_CONFIG_SIZE_CNS];
/* hold the "system" generic info */
APP_DEMO_SYS_CONFIG_STC appDemoSysConfig = {
    GT_FALSE  /*cpuEtherPortUsed*/
};

/* offset used during HW device ID calculation formula */
GT_U8 appDemoHwDevNumOffset = 0x10;

/*****************************************************************************
* Local variables
******************************************************************************/
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


/* PEX devices */
PRV_PCI_DEVICE_QUIRKS_ARRAY_MAC


/* Forward declaration of ExMx configuration functions */
#if (defined EX_FAMILY) || (defined MX_FAMILY)
    extern GT_STATUS appDemoExMxLibrariesInit();
    extern GT_STATUS appDemoExMxPpLogicalInit();
    extern GT_STATUS appDemoExMxTrafficEnable();
    extern GT_STATUS appDemoExMxHwPpPhase1Init();
    extern GT_STATUS appDemoExMxHwPpStartInit();
    extern GT_STATUS appDemoExMxHwPpPhase2Init();
    extern GT_STATUS appDemoExMxPpGeneralInit();
#endif /*(defined EX_FAMILY) || (defined MX_FAMILY)*/

/* Forward declaration of ExMxPm configuration functions */
#if (defined EXMXPM_FAMILY)
    extern GT_STATUS appDemoExMxPmLibrariesInit();
    extern GT_STATUS appDemoExMxPmPpLogicalInit();
    extern GT_STATUS appDemoExMxPmTrafficEnable();
    extern GT_STATUS appDemoExMxPmHwPpPhase1Init();
    extern GT_STATUS appDemoExMxPmHwPpStartInit();
    extern GT_STATUS appDemoExMxPmHwPpPhase2Init();
    extern GT_STATUS appDemoExMxPmPpGeneralInit();
#endif /*(defined EX_FAMILY) || (defined MX_FAMILY)*/

#if (defined CHX_FAMILY) || (defined SAL_FAMILY)
/* Forward declaration of Dx configuration functions */
extern GT_STATUS appDemoDxLibrariesInit();
extern GT_STATUS appDemoDxPpLogicalInit();
extern GT_STATUS appDemoDxTrafficEnable();
extern GT_STATUS appDemoDxHwPpPhase1Init();
extern GT_STATUS appDemoDxHwPpStartInit();
extern GT_STATUS appDemoDxHwPpPhase2Init();
extern GT_STATUS appDemoDxPpGeneralInit();
#endif /*(defined CHX_FAMILY) || (defined SAL_FAMILY)*/
/* macro to force casting between 2 functions prototypes */
#define FORCE_FUNC_CAST (void*)


/*****************************************************************************
* Public API implementation
******************************************************************************/

/**
* @internal getDevFamily function
* @endinternal
*
* @brief   Gets the device family from vendor Id and device Id (read from PCI bus)
*
* @param[in] pciDevVendorIdPtr        - pointer to PCI/PEX device identification data.
*
* @param[out] devFamilyPtr             - pointer to CPSS PP device family.
* @param[out] isPexPtr                 - pointer to is PEX flag:
*                                      GT_TRUE - device has PEX interface
*                                      GT_FALSE - device has not PEX interface
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS getDevFamily(
    IN GT_PCI_DEV_VENDOR_ID    *pciDevVendorIdPtr,
    OUT CPSS_PP_FAMILY_TYPE_ENT *devFamilyPtr,
    OUT GT_BOOL     *isPexPtr
)
{
    CPSS_PP_DEVICE_TYPE deviceType = pciDevVendorIdPtr->devId << 16 | pciDevVendorIdPtr->vendorId;
    GT_U32                  ii;
    GT_U32                  jj;
    GT_BOOL                 found = GT_FALSE;

    /* get the info about our device */
    ii = 0;
    while (cpssSupportedTypes[ii].devFamily != END_OF_TABLE)
    {
        jj = 0;
        while (cpssSupportedTypes[ii].devTypeArray[jj] != END_OF_TABLE)
        {
            if (deviceType == cpssSupportedTypes[ii].devTypeArray[jj])
            {
                found = GT_TRUE;
                break;
            }
            jj++;
        }

        if (GT_TRUE == found)
        {
            break;
        }
        ii++;
    }

    /* check if deviceType was found */
    if (GT_TRUE == found)
    {
        /* get family type from CPSS DB */
        *devFamilyPtr = cpssSupportedTypes[ii].devFamily;
    }
    else  /* device not managed by CPSS , so check the devId */
    {
        return GT_NOT_SUPPORTED;
    }

    *isPexPtr = GT_FALSE;

    /* loop on the PEX devices */
    for (ii = 0; prvPciDeviceQuirks[ii].pciId != 0xffffffff; ii++)
    {
        if (prvPciDeviceQuirks[ii].pciId == deviceType)
        {
            *isPexPtr = GT_TRUE;
            break;
        }
    }

    return GT_OK;
}

/**
* @internal appDemoSysGetPciInfo function
* @endinternal
*
* @brief   Gets the Pci info for the mini-application usage.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoSysGetPciInfo
(
    GT_VOID
)
{
    GT_PCI_INFO pciInfo;
    GT_U8       devIdx;
    GT_BOOL     isFirst;
    GT_STATUS   status;
    GT_BOOL     isPex;/* is it PEX device */


    devIdx = SYSTEM_DEV_NUM_MAC(0);
    isFirst = GT_TRUE;

    status = gtPresteraGetPciDev(isFirst, &pciInfo);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("gtPresteraGetPciDev", status);
    if (GT_OK != status)
    {
        return status;
    }

    while (GT_OK == status)
    {
        appDemoPpConfigList[devIdx].channel = CPSS_CHANNEL_PCI_E;
        appDemoPpConfigList[devIdx].devNum  = devIdx;

        osMemCpy(&(appDemoPpConfigList[devIdx].pciInfo),
                 &pciInfo,
                 sizeof(GT_PCI_INFO));

        /* get family */
        status = getDevFamily(&appDemoPpConfigList[devIdx].pciInfo.pciDevVendorId,
                              &appDemoPpConfigList[devIdx].devFamily,
                              &isPex);
        if (GT_OK != status)
        {
            return GT_FAIL;
        }
        appDemoPpConfigList[devIdx].valid = GT_TRUE;

        devIdx++;

        isFirst = GT_FALSE;

        status = gtPresteraGetPciDev(isFirst, &pciInfo);
    }
    return GT_OK;
}

/**
* @internal appDemoHotInsRmvSysGetPciInfo function
* @endinternal
*
* @brief  Gets the Pci info for the mini-application usage.
* IN devIdx                        -   device number
* IN pciBus                        -   PCI Bus number
* IN pciDev                        -   PCI device number
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoHotInsRmvSysGetPciInfo
(
    IN GT_U8 devIdx,
    IN GT_U8 pciBus,
    IN GT_U8 pciDev
)
{
    GT_PCI_INFO pciInfo;
    GT_STATUS   rc;
    GT_U32      DevId;
    GT_BOOL     isPex;/* is it PEX device */
    GT_BOOL     isFirst = GT_TRUE;
    GT_BOOL     isFound = GT_FALSE;

    while((rc = gtPresteraGetPciDev(isFirst, &pciInfo)) == GT_OK)
    {
        isFirst = GT_FALSE;

        if(pciInfo.pciBusNum == pciBus && pciInfo.pciIdSel == pciDev)
        {
            isFound = GT_TRUE;
            break;
        }

    }
#ifndef ASIC_SIMULATION
    if(isFound == GT_TRUE)
    {
        rc = extDrvPciConfigDev(pciBus, pciDev, 0);
        if(rc != GT_OK)
            return rc;
    }
#endif
    if(isFound == GT_TRUE && appDemoPpConfigList[devIdx].valid == GT_FALSE) 
    {
        appDemoPpConfigList[devIdx].channel = CPSS_CHANNEL_PCI_E;
        appDemoPpConfigList[devIdx].devNum  = devIdx;

        osMemCpy(&(appDemoPpConfigList[devIdx].pciInfo),
                 &pciInfo,
                 sizeof(GT_PCI_INFO));

        /* get family */
        rc = getDevFamily(&appDemoPpConfigList[devIdx].pciInfo.pciDevVendorId,
                              &appDemoPpConfigList[devIdx].devFamily,
                              &isPex);
        if (GT_OK != rc)
        {
            return GT_FAIL;
        }
        appDemoPpConfigList[devIdx].valid = GT_TRUE;

        DevId = (pciInfo.pciDevVendorId.devId << 16) |
                   (pciInfo.pciDevVendorId.vendorId/*0x11ab*/);

        cpssOsPrintf("Recognized[0x%8.8x]Bus [0x%8.8x] device on the PCI/PEX\n",pciInfo.pciBusNum, DevId);
    }

    return rc;
}
/**
* @internal appDemoSysGetSmiInfo function
* @endinternal
*
* @brief   Gets the SMI info for the mini-application usage.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoSysGetSmiInfo
(
    GT_VOID
)
{
    GT_STATUS   status;
    GT_SMI_INFO smiInfo;
    GT_U8       devIdx;
    GT_BOOL     isFirst;
    GT_PCI_DEV_VENDOR_ID devVendorId;
    GT_BOOL     isPex;/* is it PEX device */

    isFirst = GT_TRUE;

    /* Find first not valid device */
    for (devIdx = SYSTEM_DEV_NUM_MAC(0); devIdx < PRV_CPSS_MAX_PP_DEVICES_CNS; devIdx++)
    {
        if (GT_FALSE == appDemoPpConfigList[devIdx].valid)
        {
            break;
        }
    }

    if (devIdx == PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        return GT_FAIL;
    }

    while (GT_OK == gtPresteraGetSmiDev(isFirst, &smiInfo))
    {
        appDemoPpConfigList[devIdx].channel = CPSS_CHANNEL_SMI_E;
        appDemoPpConfigList[devIdx].devNum  = devIdx;

        osMemCpy(&(appDemoPpConfigList[devIdx].smiInfo),
                 &smiInfo,
                 sizeof(GT_SMI_INFO));

        devVendorId.vendorId = smiInfo.smiDevVendorId.vendorId;
        devVendorId.devId = smiInfo.smiDevVendorId.devId;

        /* get family */
        status = getDevFamily(&devVendorId,
                    &appDemoPpConfigList[devIdx].devFamily,
                    &isPex);/* dummy parameter for SMI devices */
        if (GT_OK != status)
        {
            return GT_FAIL;
        }

        appDemoPpConfigList[devIdx].valid = GT_TRUE;
        devIdx++;
        isFirst = GT_FALSE;
    }
    return GT_OK;
}


/**
* @internal appDemoSysGetTwsiInfo function
* @endinternal
*
* @brief   Gets the SMI info for the mini-application usage.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoSysGetTwsiInfo
(
    GT_VOID
)
{
    return GT_NOT_IMPLEMENTED;
}

/**
* @internal appDemoSysConfigFuncsGet function
* @endinternal
*
* @brief   Gets the system configuration functions accordingly to given device type.
*
* @param[in] deviceType               - device type.
*
* @param[out] sysConfigFuncsPtr        - system configuration functions.
* @param[out] apiSupportedBmpPtr       - (pointer to)bmp of supported type of API to call
*
* @retval GT_OK                    -  on success
* @retval GT_BAD_PTR               -  bad pointer
* @retval GT_NOT_FOUND             -  device type is unknown
* @retval GT_FAIL                  -  on failure
*/
GT_STATUS appDemoSysConfigFuncsGet
(
    IN  GT_U32                      deviceType,
    OUT APP_DEMO_SYS_CONFIG_FUNCS   *sysConfigFuncsPtr,
    OUT GT_U32                      *apiSupportedBmpPtr
)
{
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    GT_U32                  ii;
    GT_U32                  jj;
    GT_BOOL                 found = GT_FALSE;


    if (NULL == sysConfigFuncsPtr || apiSupportedBmpPtr == NULL)
    {
        return GT_BAD_PTR;
    }

    /* get the info about our device */
    ii = 0;
    while (cpssSupportedTypes[ii].devFamily != END_OF_TABLE)
    {
        jj = 0;
        while (cpssSupportedTypes[ii].devTypeArray[jj] != END_OF_TABLE)
        {
            if (deviceType == cpssSupportedTypes[ii].devTypeArray[jj])
            {
                found = GT_TRUE;
                break;
            }
            jj++;
        }

        if (GT_TRUE == found)
        {
            break;
        }
        ii++;
    }

    /* check if deviceType was found */
    if (GT_FALSE == found)
    {
        return GT_NOT_FOUND;
    }

    /* get family type */
    devFamily = cpssSupportedTypes[ii].devFamily;
    *apiSupportedBmpPtr = 0;

    /* fill API accordingly to family type */
    if(CPSS_IS_EXMX_FAMILY_MAC(devFamily))
    {
#if (defined EX_FAMILY) || (defined MX_FAMILY)
        /* ExMx API should be used */
        *apiSupportedBmpPtr = APP_DEMO_EXMX_FUNCTIONS_SUPPORT_CNS;
        if(devFamily == CPSS_PP_FAMILY_TIGER_E)
        {
            *apiSupportedBmpPtr |= APP_DEMO_EXMXTG_FUNCTIONS_SUPPORT_CNS;
        }

        sysConfigFuncsPtr->cpssHwPpPhase1Init   = FORCE_FUNC_CAST appDemoExMxHwPpPhase1Init;
        sysConfigFuncsPtr->cpssHwPpStartInit    = FORCE_FUNC_CAST appDemoExMxHwPpStartInit;
        sysConfigFuncsPtr->cpssHwPpPhase2Init   = FORCE_FUNC_CAST appDemoExMxHwPpPhase2Init;
        sysConfigFuncsPtr->cpssLibrariesInit    = FORCE_FUNC_CAST appDemoExMxLibrariesInit;
        sysConfigFuncsPtr->cpssPpLogicalInit    = FORCE_FUNC_CAST appDemoExMxPpLogicalInit;
        sysConfigFuncsPtr->cpssTrafficEnable    = FORCE_FUNC_CAST appDemoExMxTrafficEnable;
        sysConfigFuncsPtr->cpssPpGeneralInit    = FORCE_FUNC_CAST appDemoExMxPpGeneralInit;
#endif /* (defined EX_FAMILY) || (defined MX_FAMILY) */
    }
    else if(CPSS_IS_EXMXPM_FAMILY_MAC(devFamily))
    {
#if (defined EXMXPM_FAMILY)
        /* ExMx API should be used */
        *apiSupportedBmpPtr = APP_DEMO_EXMXPM_FUNCTIONS_SUPPORT_CNS;

        sysConfigFuncsPtr->cpssHwPpPhase1Init   = FORCE_FUNC_CAST appDemoExMxPmHwPpPhase1Init;
        sysConfigFuncsPtr->cpssHwPpStartInit    = FORCE_FUNC_CAST appDemoExMxPmHwPpStartInit;
        sysConfigFuncsPtr->cpssHwPpPhase2Init   = FORCE_FUNC_CAST appDemoExMxPmHwPpPhase2Init;
        sysConfigFuncsPtr->cpssLibrariesInit    = FORCE_FUNC_CAST appDemoExMxPmLibrariesInit;
        sysConfigFuncsPtr->cpssPpLogicalInit    = FORCE_FUNC_CAST appDemoExMxPmPpLogicalInit;
        sysConfigFuncsPtr->cpssTrafficEnable    = FORCE_FUNC_CAST appDemoExMxPmTrafficEnable;
        sysConfigFuncsPtr->cpssPpGeneralInit    = FORCE_FUNC_CAST appDemoExMxPmPpGeneralInit;
#endif /* (defined EXMXPM_FAMILY) */
    }
    else if(CPSS_IS_DXSAL_FAMILY_MAC(devFamily))
    {
#if (defined SAL_FAMILY)
        *apiSupportedBmpPtr = APP_DEMO_DXSAL_FUNCTIONS_SUPPORT_CNS;

        sysConfigFuncsPtr->cpssHwPpPhase1Init   = FORCE_FUNC_CAST appDemoDxHwPpPhase1Init;
        sysConfigFuncsPtr->cpssHwPpStartInit    = FORCE_FUNC_CAST appDemoDxHwPpStartInit;
        sysConfigFuncsPtr->cpssHwPpPhase2Init   = FORCE_FUNC_CAST appDemoDxHwPpPhase2Init;
        sysConfigFuncsPtr->cpssLibrariesInit    = FORCE_FUNC_CAST appDemoDxLibrariesInit;
        sysConfigFuncsPtr->cpssPpLogicalInit    = FORCE_FUNC_CAST appDemoDxPpLogicalInit;
        sysConfigFuncsPtr->cpssTrafficEnable    = FORCE_FUNC_CAST appDemoDxTrafficEnable;
        sysConfigFuncsPtr->cpssPpGeneralInit    = FORCE_FUNC_CAST appDemoDxPpGeneralInit;
#endif /* (defined SAL_FAMILY) */
    }
    else if(CPSS_IS_DXCH_FAMILY_MAC(devFamily))
    {
#if (defined CHX_FAMILY)
        *apiSupportedBmpPtr = APP_DEMO_DXCH_FUNCTIONS_SUPPORT_CNS;

        if(devFamily >= CPSS_PP_FAMILY_CHEETAH2_E)
        {
            /*ch2,3 ... */
            *apiSupportedBmpPtr |= APP_DEMO_DXCH2_FUNCTIONS_SUPPORT_CNS;
        }

        if(devFamily >= CPSS_PP_FAMILY_CHEETAH3_E)
        {
            /*ch3 ...*/
            *apiSupportedBmpPtr |= APP_DEMO_DXCH3_FUNCTIONS_SUPPORT_CNS;
        }

        if(devFamily >= CPSS_PP_FAMILY_DXCH_XCAT_E)
        {
            /*xCat ...*/
            *apiSupportedBmpPtr |= APP_DEMO_DXCH_XCAT_FUNCTIONS_SUPPORT_CNS;
        }

        if(devFamily >= CPSS_PP_FAMILY_DXCH_LION_E)
        {
            /*Lion ...*/
            *apiSupportedBmpPtr |= APP_DEMO_DXCH_LION_FUNCTIONS_SUPPORT_CNS;
        }

        sysConfigFuncsPtr->cpssHwPpPhase1Init   = FORCE_FUNC_CAST appDemoDxHwPpPhase1Init;
        sysConfigFuncsPtr->cpssHwPpStartInit    = FORCE_FUNC_CAST appDemoDxHwPpStartInit;
        sysConfigFuncsPtr->cpssHwPpPhase2Init   = FORCE_FUNC_CAST appDemoDxHwPpPhase2Init;
        sysConfigFuncsPtr->cpssLibrariesInit    = FORCE_FUNC_CAST appDemoDxLibrariesInit;
        sysConfigFuncsPtr->cpssPpLogicalInit    = FORCE_FUNC_CAST appDemoDxPpLogicalInit;
        sysConfigFuncsPtr->cpssTrafficEnable    = FORCE_FUNC_CAST appDemoDxTrafficEnable;
        sysConfigFuncsPtr->cpssPpGeneralInit    = FORCE_FUNC_CAST appDemoDxPpGeneralInit;
#endif /* (defined CHX_FAMILY) */
    }

    if((*apiSupportedBmpPtr) == 0)
    {
        /* Unsupported family type */
        return GT_NOT_IMPLEMENTED;
    }

    if(appDemoOnDistributedSimAsicSide != 0)
    {
        sysConfigFuncsPtr->cpssHwPpStartInit = NULL;
        sysConfigFuncsPtr->cpssTrafficEnable = NULL;
        sysConfigFuncsPtr->cpssPpGeneralInit = NULL;
    }

    return GT_OK;
}

/**
* @internal appDemoHwDevNumOffsetSet function
* @endinternal
*
* @brief   Sets value of appDemoHwDevNumOffset used during device HW ID calculation
*         based on device SW ID..
* @param[in] hwDevNumOffset           - value to save into appDemoHwDevNumOffset.
*                                       GT_OK
*/
GT_STATUS appDemoHwDevNumOffsetSet
(
    IN GT_U8 hwDevNumOffset
)
{
    appDemoHwDevNumOffset = hwDevNumOffset;

    return GT_OK;
}

/*******************************************************************************
* appDemoDbByDevNum
*
* DESCRIPTION:
*       get the AppDemo DB entry for the 'devNum'.
*
* INPUTS:
*       devNum - the 'cpss devNum'
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       pointer to the appDemoDb
*
* COMMENTS:
*       None.
*
*******************************************************************************/
APP_DEMO_PP_CONFIG* appDemoDbByDevNum(IN GT_U8 devNum)
{
    static APP_DEMO_PP_CONFIG dummyDbEntry;
    GT_U32  ii;

    APP_DEMO_PP_CONFIG* currentEntryInAppDemoDbPtr = &appDemoPpConfigList[0];
    for(ii = 0 ; ii < APP_DEMO_PP_CONFIG_SIZE_CNS ; ii++,currentEntryInAppDemoDbPtr++)
    {
        if(currentEntryInAppDemoDbPtr->valid == GT_TRUE &&
           currentEntryInAppDemoDbPtr->devNum == devNum)
        {
            return currentEntryInAppDemoDbPtr;
        }
    }

    cpssOsPrintf("appDemoDbByDevNum: Error not found device [%d] \n",devNum);
    /* should not happen */
    cpssOsMemSet(&dummyDbEntry,0,sizeof(dummyDbEntry));
    return &dummyDbEntry;
}



