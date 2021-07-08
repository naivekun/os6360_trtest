/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file mvHwsPortModeElements.c
*/

#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>

#if defined (CM3) && (defined (BOBK_DEV_SUPPORT) || defined (ALDRIN_DEV_SUPPORT))
char *devTypeNamesArray[LAST_SIL_TYPE] =
{
    "BobK",
    "Aldrin",
};
#else
char *devTypeNamesArray[LAST_SIL_TYPE] =
{
    "Lion2A0",
    "Lion2B0",
    "Puma3A0",
    "HooperA0",
    "Puma3B0",
    "NP5A0",
    "BobcatA0",
    "Alleycat3A0",
    "NP5B0",
    "BobK",
    "Aldrin",
    "Bobcat3",
    "BearValley", /* to be sure it's not forgotten, because this array indexed by MV_HWS_DEV_TYPE */
    "Pipe",
    "Aldrin2"
};
#endif

/* Array of Devices - each device is an array of ports */
MV_HWS_DEVICE_PORTS_ELEMENTS hwsDevicesPortsElementsArray[HWS_MAX_DEVICE_NUM] = { 0 };

MV_HWS_PORT_INIT_PARAMS hwsSingleEntryInitParams = {
    /* port mode,   MAC Type    Mac Num    PCS Type    PCS Num    FEC       Speed      FirstLane LanesList LanesNum  Media         10Bit      FecSupported */
     NON_SUP_MODE,  MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,   FEC_OFF,   SPEED_NA,  NA_NUM,   {0,0,0,0},   0,    XAUI_MEDIA,   _10BIT_ON,  FEC_NA};

/**
* @internal hwsInitPortsModesParam function
* @endinternal
*
* @brief   Registers silicon ports modes parameters in the system.
*         Calls ones per active silicon type.
* @param[in] devType                  - silicon type.
* @param[in] portModes                - pointer to array of ports modes parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsInitPortsModesParam
(
    GT_U8                   devNum,
    MV_HWS_PORT_INIT_PARAMS *portModes[]
)
{
    MV_HWS_DEV_TYPE                 devType = HWS_DEV_SILICON_TYPE(devNum);
    MV_HWS_DEVICE_PORTS_ELEMENTS    curDevicePorts;
    GT_U32                          numOfPorts = hwsDeviceSpecInfo[devNum].portsNum;
    GT_U32                          lastSupMode;
    GT_U32                          portNum;

    if (devType == Lion2B0 || devType == Lion2A0 || devType == HooperA0)
    {
        numOfPorts = 14;
    }
    /* Allocate ports array and save it to the general devices array*/
    curDevicePorts = (MV_HWS_DEVICE_PORTS_ELEMENTS)hwsOsMallocFuncPtr(numOfPorts*sizeof(MV_HWS_SINGLE_PORT_STC));
    if (NULL == curDevicePorts)
    {
      return GT_NO_RESOURCE;
    }
    hwsDevicesPortsElementsArray[devNum] = curDevicePorts;

    for (portNum = 0; portNum < numOfPorts ; portNum++)
    {
        curDevicePorts[portNum].perModeFecList = NULL;
        curDevicePorts[portNum].supModesCatalog = NULL;
        hwsOsMemCopyFuncPtr(&curDevicePorts[portNum].curPortParams, &hwsSingleEntryInitParams, sizeof(MV_HWS_PORT_INIT_PARAMS));
    }

    for (portNum = 0; (NULL != portModes) && (portNum < numOfPorts) ; portNum++)
    { /* copy defualts portParams if configured in the **portModes array */
        if (portModes[portNum] != NULL)
        {
            lastSupMode = hwsDeviceSpecInfo[devNum].lastSupPortMode;
            curDevicePorts[portNum].supModesCatalog = (MV_HWS_PORT_INIT_PARAMS*)hwsOsMallocFuncPtr((1+lastSupMode)*sizeof(MV_HWS_PORT_INIT_PARAMS));
            if (NULL == curDevicePorts[portNum].supModesCatalog)
            {
              return GT_NO_RESOURCE;
            }
            hwsOsMemCopyFuncPtr(curDevicePorts[portNum].supModesCatalog, portModes[portNum], (1+lastSupMode)*sizeof(MV_HWS_PORT_INIT_PARAMS));
        }
    }
    return GT_OK;
}

/**
* @internal mvHwsBuildActiveLaneList function
* @endinternal
*
* @brief   Get SD vector.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsBuildActiveLaneList
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_PORT_INIT_PARAMS *curPortParams;

    curPortParams = hwsPortModeParamsGet(devNum, portGroup, portNum, portMode);
    if (curPortParams == NULL || curPortParams->numOfActLanes == 0)
    {
      return GT_NOT_SUPPORTED;
    }
    return hwsPortParamsGetLanes(devNum,portGroup,portNum,curPortParams);
}

/**
* @internal mvHwsRebuildActiveLaneList function
* @endinternal
*
* @brief   Get SD vector and rebuild active lane array. Can be called during CreatePort
*         only.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsRebuildActiveLaneList
(
    GT_U8                 devNum,
    GT_U32                portGroup,
    GT_U32                portNum,
    MV_HWS_PORT_STANDARD  portMode,
    GT_U32                *laneList
)
{
    GT_STATUS rc = GT_OK;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 sdVec, tmp, startSerdes;
    GT_U32 laneCtnt;

    rc = hwsPortModeParamsGetToBuffer(devNum, portGroup, portNum, portMode, &curPortParams);
    if (GT_OK != rc)
    {
        return rc;
    }

    if (curPortParams.numOfActLanes != 0)
    {
        CHECK_STATUS(mvHwsRedundancyVectorGet(devNum, portGroup, &sdVec));

        startSerdes = curPortParams.activeLanesList[0];
        tmp = (sdVec >> startSerdes) & 0xF;
        for (laneCtnt = 0; (laneCtnt < curPortParams.numOfActLanes); laneCtnt++)
        {
            laneList[laneCtnt] = curPortParams.activeLanesList[laneCtnt];
            if (tmp & 1)
            {
                laneList[laneCtnt]++;
            }
            tmp = tmp >> 1;
        }
    }

    return GT_OK;
}

/**
* @internal mvHwsDestroyActiveLaneList function
* @endinternal
*
* @brief   Free lane list allocated memory.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsDestroyActiveLaneList
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_PORT_INIT_PARAMS *curPortParams;
    GT_U8 i;

    curPortParams = hwsPortModeParamsGet(devNum, portGroup, portNum, portMode);
    if ((curPortParams != NULL) && (curPortParams->activeLanesList != NULL))
    {
      for (i = 0; i < 4; i++)
        curPortParams->activeLanesList[i] = 0;
    }

    return GT_OK;
}

/**
* @internal mvHwsBuildDeviceLaneList function
* @endinternal
*
* @brief   Build the device lane list per port and per mort mode.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsBuildDeviceLaneList
(
   GT_U8    devNum,
   GT_U32   portGroup
)
{
    GT_U32    portNum;
#ifndef MV_HWS_REDUCED_BUILD
    GT_U32    portMode;
    GT_STATUS res;

    /* init active lines in ports DB */
    for (portNum = 0; portNum < HWS_CORE_PORTS_NUM(devNum); portNum++)
    {
        for (portMode = 0; portMode < HWS_DEV_PORT_MODES(devNum); portMode++)
        {
            res = mvHwsBuildActiveLaneList(devNum, portGroup, portNum, (MV_HWS_PORT_STANDARD)portMode);
            if((res != GT_OK) && (res != GT_NOT_SUPPORTED))
            {
                return res;
            }
        }
    }
#else
    /* Internal CPU receives the port parameters from the Host so no need to hold table per port mode */
    for (portNum = 0; portNum < HWS_CORE_PORTS_NUM(devNum); portNum++)
    {
        CHECK_STATUS(mvHwsBuildActiveLaneList(devNum, portGroup, portNum, (MV_HWS_PORT_STANDARD)0 /*portMode*/));
    }
#endif

    return GT_OK;
}

/**
* @internal mvHwsDestroyDeviceLaneList function
* @endinternal
*
* @brief   Free the device lane list allocated memory.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsDestroyDeviceLaneList
(
   GT_U8    devNum,
   GT_U32   portGroup
)
{
    GT_U32  portNum;
    GT_U32  portMode;

    /* free active lines allocated memory in ports DB */
    for (portNum = 0; portNum < HWS_CORE_PORTS_NUM(devNum); portNum++)
    {
        for (portMode = 0; portMode < HWS_DEV_PORT_MODES(devNum); portMode++)
        {
            CHECK_STATUS(mvHwsDestroyActiveLaneList(devNum, portGroup, portNum, (MV_HWS_PORT_STANDARD)portMode));
        }
    }

    return GT_OK;
}

MV_HWS_PORT_INIT_PARAMS *hwsPortsParamsArrayGet
(
   GT_U8    devNum,
   GT_U32   portGroup,
   GT_U32   portNum
)
{
    MV_HWS_DEVICE_PORTS_ELEMENTS tmp = NULL;
    MV_HWS_DEV_TYPE  devType = HWS_DEV_SILICON_TYPE(devNum);
#ifndef CM3
    GT_BOOL extMode;
#endif /* CM3 */

    if (devNum >= HWS_MAX_DEVICE_NUM)
        return NULL;

#ifndef CM3
    /* In Alleycat3: check if Ports #25 and #27 are in Extended mode by checking the
       EXTERNAL_CONTROL register value. If the port is in Extended mode, the related
       PortsParamsArray for Extended port will be #28/#29 */
    if (devType == Alleycat3A0)
    {
        if (mvHwsPortExtendedModeCfgGet(devNum, portGroup, portNum, _10GBase_KR, &extMode) != GT_OK)
        {
            return NULL;
        }

        if ((extMode == GT_TRUE) && (portNum == 25))
        {
            portNum = 28;
        }
        else if ((extMode == GT_TRUE) && (portNum == 27))
        {
            portNum = 29;
        }
    }
    /* In Lion2A0, Lion2B0,HooperA0: check if Ports #9 and #11 are in Extended mode by checking the
       EXTERNAL_CONTROL register value. If the port is in Extended mode, the related
       PortsParamsArray for Extended port will be #12/#13 */
    else if (HWS_DEV_GOP_REV(devNum) == GOP_40NM_REV1)
    {
        if (mvHwsPortExtendedModeCfgGet(devNum, portGroup, portNum, _10GBase_KR /* dummy */, &extMode) != GT_OK)
        {
            return NULL;
        }

        if ((extMode == GT_TRUE) && (portNum == 9))
        {
            portNum = 12; /* port 12 entry contains Port 9 externded mode parameters */
        }
        else if ((extMode == GT_TRUE) && (portNum == 11))
        {
            portNum = 13; /* port 13 entry contains Port 11 externded mode parameters */
        }
    }
#endif /* CM3 */

    tmp = (MV_HWS_DEVICE_PORTS_ELEMENTS)hwsDevicesPortsElementsArray[devNum];

    if (tmp == NULL)
    {
        return NULL;
    }
    /* Avoid warning */
    devType = devType;
    return tmp[portNum].supModesCatalog;
}

GT_U32 hwsPortModeParamsIndexGet
(
   GT_U8                    devNum,
   GT_U32                   portGroup,
   GT_U32                   portNum,
   MV_HWS_PORT_STANDARD     portMode
)
{
#ifndef MV_HWS_REDUCED_BUILD
    MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;

    /* get func DB */
    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);

    if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portParamsIndexGetFunc != 0)
    {
        return hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portParamsIndexGetFunc(devNum, portGroup, portNum, portMode);
    }
    else
    {
        return portMode;
    }
#else
    return 0;
#endif
}

GT_STATUS hwsPortModeParamsSetMode
(
   GT_U8                    devNum,
   GT_U32                   portGroup,
   GT_U32                   portNum,
   MV_HWS_PORT_STANDARD     portMode
)
{
#ifndef MV_HWS_REDUCED_BUILD
    MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;

    /* get func DB */
    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);
    if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portParamsSetFunc != 0)
    {
        return hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portParamsSetFunc(devNum, portGroup, portNum, portMode);
    }
    else
    {
        MV_HWS_DEVICE_PORTS_ELEMENTS    curDevicePorts = hwsDevicesPortsElementsArray[devNum];
        MV_HWS_PORT_INIT_PARAMS         *curPortParams;
        MV_HWS_PORT_INIT_PARAMS         *newPortParams;
        GT_U32                          portModeIndex;

        /* avoid warnings */
        portGroup = portGroup;

        if (portNum > hwsDeviceSpecInfo[devNum].portsNum)
        {
            return GT_OUT_OF_RANGE;
        }

        curPortParams = &curDevicePorts[portNum].curPortParams;
        if (NON_SUP_MODE == portMode) /* Reset port */
        {
            /* initialize entry with default port params */
            hwsOsMemCopyFuncPtr(curPortParams, &hwsSingleEntryInitParams, sizeof(MV_HWS_PORT_INIT_PARAMS));
            curPortParams->portMacNumber = (GT_U16)portNum;
            curPortParams->portPcsNumber = (GT_U16)portNum;
            curPortParams->firstLaneNum  = (GT_U16)portNum;
            return GT_OK;
        }

        newPortParams = hwsPortsElementsFindMode(curDevicePorts[portNum].supModesCatalog,portMode,&portModeIndex);
        if ( (NULL == newPortParams) || (newPortParams->numOfActLanes == 0)
           ||  ((_100GBase_MLG != portMode) && (portNum % newPortParams->numOfActLanes) != 0))
        {
            return GT_NOT_SUPPORTED;
        }
        /* Copy the right port mode line from the port's specific supported modes list*/
        hwsOsMemCopyFuncPtr(curPortParams, newPortParams, sizeof(MV_HWS_PORT_INIT_PARAMS));
        curPortParams->portMacNumber = (GT_U16)portNum;
        curPortParams->portPcsNumber = (GT_U16)portNum;
        curPortParams->firstLaneNum  = (GT_U16)portNum;
        curPortParams->portFecMode   = curDevicePorts[portNum].perModeFecList[portModeIndex];

        /* QSGMII is single lane port mode with special Serdes mapping */
        if((curPortParams->numOfActLanes == 1) && (portMode == QSGMII))
        {
            curPortParams->firstLaneNum = (GT_U16)(portNum & 0xFFFFFFFC);
            if(portNum % 4 != 0)
            {
                curPortParams->numOfActLanes = 0;
                return GT_OK;
            }
        }

        /* _100GBase_MLG is 4 lanes port mode with special Serdes mapping: 4 lanes are bound to all
           four ports in the GOP, but only first port number in the GOP has 4 active lanes.
           For other free ports in GOP, the numOfActLanes should change from NA_NUM to 0 */
        if((curPortParams->numOfActLanes == 4) && (portMode == _100GBase_MLG))
        {
            curPortParams->firstLaneNum = (GT_U16)(portNum & 0xFFFFFFFC);
            if(portNum % 4 != 0)
            {
                curPortParams->numOfActLanes = 0;
                return GT_OK;
            }
        }

        /* build active lane list */
        CHECK_STATUS(hwsPortParamsGetLanes(devNum,portGroup,portNum,curPortParams));
        return GT_OK;
    }
#else
    return GT_OK;
#endif
}

GT_STATUS hwsPortModeParamsSetFec
(
   GT_U8                    devNum,
   GT_U32                   portGroup,
   GT_U32                   portNum,
   MV_HWS_PORT_STANDARD     portMode,
   MV_HWS_PORT_FEC_MODE     fecMode
)
{
#ifndef MV_HWS_REDUCED_BUILD
    GT_U32                  portModeIndex;
    MV_HWS_SINGLE_PORT_STC  *curPortStruct;
    portGroup = portGroup;

    if (portNum >= hwsDeviceSpecInfo[devNum].portsNum || devNum >= HWS_MAX_DEVICE_NUM ||
         NULL == hwsDevicesPortsElementsArray[devNum])
    {
        return GT_BAD_PARAM;
    }
    curPortStruct =  &((hwsDevicesPortsElementsArray[devNum])[portNum]);
    if(NULL == hwsPortsElementsFindMode(curPortStruct->supModesCatalog,
                                       portMode,
                                       &portModeIndex)) /* Get the port mode index */
    {
        return GT_NOT_SUPPORTED;
    }

    curPortStruct->perModeFecList[portModeIndex] = fecMode;
#else
    hwsPortModeParamsGet(devNum,portGroup,portNum,portMode)->portFecMode = fecMode;
#endif
    return GT_OK;
}

MV_HWS_PORT_INIT_PARAMS *hwsPortModeParamsGet
(
   GT_U8                    devNum,
   GT_U32                   portGroup,
   GT_U32                   portNum,
   MV_HWS_PORT_STANDARD     portMode
)
{
    MV_HWS_DEVICE_PORTS_ELEMENTS curDevicePorts = HWS_DEV_PORTS_ARRAY(devNum);

#ifndef MV_HWS_REDUCED_BUILD
    MV_HWS_DEV_TYPE             devType = HWS_DEV_SILICON_TYPE(devNum);
    MV_HWS_PORT_INIT_PARAMS     *retPortParams = NULL;
    GT_U32                      portModeIndex = portMode;

/* SIP_5_15 and above - From BobK and on, each port has one line entry of its own current params (single MV_HWS_PORT_INIT_PARAMS) */
    if (devType == BobK     || devType == Aldrin    || devType == Bobcat3  || devType == Pipe ||
        devType == Aldrin2  || devType == Raven     || devType == Falcon)
    {
        if (curDevicePorts[portNum].curPortParams.portStandard == portMode || portMode == NON_SUP_MODE)
        {
            retPortParams = &curDevicePorts[portNum].curPortParams; /* CURRENT PORT'S VALUES! (not the portMode's defualts) */
        }
        else
        { /* if hwsPortModeParamsGet is used to retrieve portMode's values other than the current port's values */
            if (NULL != hwsDeviceSpecInfo[devNum].portsParamsSupModesMap) /* if this is a valid port */
            {   /* find defualt values of the requested portMode */
                retPortParams = hwsPortsElementsFindMode(
                        curDevicePorts[portNum].supModesCatalog,
                        portMode,&portModeIndex); /* NULL if mode not supported */
                if((retPortParams != NULL) && (_100GBase_MLG != portMode) && (portNum % retPortParams->numOfActLanes) != 0)
                {
                    retPortParams = NULL;
                }
            }
        }
    }
/* Before SIP_5_15 - each port has its own copy of the entire list of all the port modes (array of MV_HWS_PORT_INIT_PARAMS's) */
    else
    {
        retPortParams = hwsPortsParamsArrayGet(devNum, portGroup, portNum);
        if (retPortParams != NULL && retPortParams[portMode].portMacNumber != NA_NUM)
        {
            retPortParams = &retPortParams[portMode];
        }
        else
        {
            retPortParams = NULL;
        }
    }
    return retPortParams;
#else
    return &curDevicePorts[portNum].curPortParams;
#endif
}

GT_STATUS hwsPortModeParamsGetToBuffer
(
   GT_U8                    devNum,
   GT_U32                   portGroup,
   GT_U32                   portNum,
   MV_HWS_PORT_STANDARD     portMode,
   MV_HWS_PORT_INIT_PARAMS  *portParamsBuffer
)
{
#ifndef MV_HWS_REDUCED_BUILD
    MV_HWS_DEVICE_PORTS_ELEMENTS    curDevicePorts = HWS_DEV_PORTS_ARRAY(devNum);
    MV_HWS_DEV_TYPE                 devType = HWS_DEV_SILICON_TYPE(devNum);
    GT_U32                          portModeIndex = portMode;
#endif
    MV_HWS_PORT_INIT_PARAMS         *srcPortParams = hwsPortModeParamsGet(devNum, portGroup, portNum, portMode);
    if (NULL == srcPortParams || NULL == portParamsBuffer)
    {
        return GT_FAIL;
    }

    /* copy the port parameters into the buffer*/
    hwsOsMemCopyFuncPtr(portParamsBuffer, srcPortParams, sizeof(MV_HWS_PORT_INIT_PARAMS));

#ifndef MV_HWS_REDUCED_BUILD
    /* for SIP_5_15 - fix the active lanes list (needed if hwsPortModeParamsGet returned the defualts port mode params) */
    if (devType == BobK     || devType == Aldrin    || devType == Bobcat3  || devType == Pipe ||
        devType == Aldrin2  || devType == Raven     || devType == Falcon)
    {
        if (NULL != hwsPortsElementsFindMode(curDevicePorts[portNum].supModesCatalog,
                                             portMode,&portModeIndex)) /* the requested portMode is supported by this port */
        {
            portParamsBuffer->portMacNumber =  (GT_U16)portNum;
            portParamsBuffer->portPcsNumber =  (GT_U16)portNum;
            portParamsBuffer->firstLaneNum  =  (GT_U16)portNum;
            portParamsBuffer->portFecMode   =  curDevicePorts[portNum].perModeFecList[portModeIndex];
            hwsPortParamsGetLanes(devNum,portGroup,portNum,portParamsBuffer);
        }
    }
#endif
    return GT_OK;
}

GT_STATUS hwsPortModeParamsSet
(
   GT_U8                    devNum,
   GT_U32                   portGroup,
   GT_U32                   portNum,
   MV_HWS_PORT_INIT_PARAMS  *portParams
)
{
    MV_HWS_PORT_INIT_PARAMS *portModesCurr;

    if(portParams->portStandard >= (MV_HWS_PORT_STANDARD)HWS_DEV_PORT_MODES(devNum))
    {
        return GT_BAD_PARAM;
    }

    portModesCurr = hwsPortModeParamsGet(devNum, portGroup, portNum, portParams->portStandard);
    if(portModesCurr == NULL)
    {
        return GT_NOT_INITIALIZED;
    }

    /* copy new port parameters */
    hwsOsMemCopyFuncPtr(portModesCurr, portParams, sizeof(MV_HWS_PORT_INIT_PARAMS));

    return GT_OK;
}

GT_STATUS hwsPortsParamsCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32                  macNum,
    GT_U32                  pcsNum,
    GT_U8                   sdVecSize,
    GT_U8                   *sdVector
)
{
    MV_HWS_PORT_INIT_PARAMS *curPortParams;
    GT_U8 i;

    curPortParams = hwsPortModeParamsGet(devNum, portGroup, portNum, portMode);
    if (curPortParams == NULL)
    {
        return GT_NOT_SUPPORTED;
    }

    curPortParams->portMacNumber = (GT_U8)macNum;
    curPortParams->portPcsNumber = (GT_U8)pcsNum;

    for (i = 0; i < sdVecSize; i++)
    {
        curPortParams->activeLanesList[i] = sdVector[i];
    }
    curPortParams->numOfActLanes = sdVecSize;

    return GT_OK;
}

GT_STATUS hwsPortsElementsClose
(
    GT_U8              devNum
)
{
#ifndef MV_HWS_REDUCED_BUILD
    MV_HWS_DEVICE_PORTS_ELEMENTS    curDevicePorts;
    GT_U32                          portNum; /* iterator */
    GT_U32                          numOfSupModes;
    GT_U32                          numOfPorts;
#endif
    if (NULL == hwsDevicesPortsElementsArray[devNum])
    {
      return GT_OK;
    }
#ifndef MV_HWS_REDUCED_BUILD
    curDevicePorts = hwsDevicesPortsElementsArray[devNum];
    numOfPorts = hwsDeviceSpecInfo[devNum].portsNum;

    for (portNum = 0 ; portNum < numOfPorts ; portNum++)
    {
        if(curDevicePorts[portNum].supModesCatalog  !=  NULL)
        {
           hwsPortsElementsFindMode(curDevicePorts[portNum].supModesCatalog,NON_SUP_MODE,&numOfSupModes);
           if (NULL != curDevicePorts[portNum].perModeFecList)
           {
               hwsOsFreeFuncPtr(curDevicePorts[portNum].perModeFecList);
           }
        }
    }
#endif
    hwsOsFreeFuncPtr(hwsDevicesPortsElementsArray[devNum]);
    hwsDevicesPortsElementsArray[devNum] = NULL;
    return GT_OK;
}

GT_BOOL hwsIsPortModeSupported
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
#ifndef MV_HWS_REDUCED_BUILD
    MV_HWS_PORT_INIT_PARAMS *curPortParams = NULL;
    portGroup = portGroup;

    if (portMode == NON_SUP_MODE)
    {
        return GT_FALSE;
    }
    curPortParams = hwsPortModeParamsGet(devNum, portGroup, portNum, portMode);
    if (NULL == curPortParams || NA_NUM == curPortParams->portMacNumber)
    {
        return GT_FALSE;
    }
    return GT_TRUE;
#else
    return GT_TRUE;
#endif
}

GT_STATUS hwsPortParamsGetLanes
(
    GT_U8                       devNum,
    GT_U32                      portGroup,
    GT_U32                      portNum,
    MV_HWS_PORT_INIT_PARAMS*    portParams
)
{
#ifndef MV_HWS_REDUCED_BUILD
    MV_HWS_DEV_FUNC_PTRS            *hwsDevFunc = NULL;
#endif
    GT_U16                          i = 0;
#if defined(ALDRIN2_DEV_SUPPORT) || !defined(MV_HWS_REDUCED_BUILD)
    MV_HWS_PORT_SERDES_TO_MAC_MUX   hwsMacToSerdesMuxStc;
    GT_U16                          j;
#endif
    if (NULL == portParams)
    {
        return GT_BAD_PARAM;
    }

    if (portParams->portStandard == NON_SUP_MODE)
    {
        return GT_NOT_SUPPORTED;
    }
#ifndef MV_HWS_REDUCED_BUILD
    /* get func DB */
    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);
    /* For devices with special lanes mapping */
    if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portParamsGetLanesFunc != NULL)
    {
        return hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portParamsGetLanesFunc(devNum, portGroup, portNum, portParams);
    }
    else /* Build default active lane list */
    {
#endif
#if defined(ALDRIN2_DEV_SUPPORT) || !defined(MV_HWS_REDUCED_BUILD)
        mvHwsPortLaneMacToSerdesMuxGet(devNum, (GT_U8)portGroup, portNum, &hwsMacToSerdesMuxStc);
#endif
        for (i = 0 ; i < portParams->numOfActLanes ; i++)
        {
#if defined(ALDRIN2_DEV_SUPPORT) || !defined(MV_HWS_REDUCED_BUILD)
            if(hwsMacToSerdesMuxStc.enableSerdesMuxing)
            {
                j = i + (portNum % 4); /* start from the right offset in the ports quad */
                portParams->activeLanesList[i] = (GT_U16)(portNum & 0xFFFFFFFC) + (GT_U16)(hwsMacToSerdesMuxStc.serdesLanes[j % 4]/* modulo to avoid warnings */);
            }
            else
#endif
            {
                portParams->activeLanesList[i] = (GT_U16)(portParams->firstLaneNum) + i;
            }
        }
        portParams->firstLaneNum = portParams->activeLanesList[0];

#ifndef MV_HWS_REDUCED_BUILD
    }
#endif
    return GT_OK;
}

GT_BOOL hwsIsFecModeSupported
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_FEC_MODE    fecMode
)
{
    GT_U8                   fecSupportedModesBmp;
    MV_HWS_DEV_TYPE         devType = HWS_DEV_SILICON_TYPE(devNum);
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    portGroup = portGroup;

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portNum, portMode, &curPortParams))
    {
        return GT_FALSE;
    }
    fecSupportedModesBmp = curPortParams.fecSupportedModesBmp;

    /* Exceptions */
    if ((devType == Bobcat3  || devType == Aldrin2)
        &&  ((portMode == _50GBase_KR2) || (portMode == _50GBase_KR2_C) || (portMode == _40GBase_KR2) ||
             (portMode == _50GBase_CR2) || (portMode == _50GBase_CR2_C) || (portMode == _50GBase_SR2))
        && ((portNum % 2 == 0) && (portNum % 4 != 0)))
    {
        fecSupportedModesBmp = FEC_OFF|RS_FEC;
    }

    return (GT_BOOL)((fecSupportedModesBmp) & (fecMode));
}

GT_BOOL hwsIsQuadCgMacSupported
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portNum
)
{
    if ((HWS_DEV_SILICON_TYPE(devNum) == Raven) || (HWS_DEV_SILICON_TYPE(devNum) == Falcon))
    {
        return GT_FALSE;
    }
    /* check if given port num has support for _100GBase_KR4 which uses CG-MAC  */
    return (NULL != hwsPortModeParamsGet(devNum, portGroup, (portNum  & HWS_2_LSB_MASK_CNS), _100GBase_KR4)) ? GT_TRUE : GT_FALSE;
}

/*============================== NOT DEFINED: MV_HWS_REDUCED_BUILD ==============================*/
#ifndef MV_HWS_REDUCED_BUILD

/**
* @internal hwsDeviceElementsCfg function
* @endinternal
*
* @brief   Init the ports' struct and initialize
*          portModesElements.
*/
GT_STATUS hwsDevicePortsElementsCfg
(
    GT_U8       devNum
)
{
    GT_U32                          portNum, portMode; /* iterators */
    GT_U32                          numOfSupModes;
    /* Get device's specific data from hwsDeviceSpecInfo Db*/
    GT_U32                          numOfPorts = hwsDeviceSpecInfo[devNum].portsNum;
    MV_HWS_PORT_INIT_PARAMS         **hwsPortsSupModes = hwsDeviceSpecInfo[devNum].portsParamsSupModesMap;
    MV_HWS_DEVICE_PORTS_ELEMENTS    curDevicePorts = hwsDevicesPortsElementsArray[devNum];

    for (portNum = 0; portNum < numOfPorts ; portNum++)
    {
    /* Initialize the MV_HWS_SINGLE_PORT_STC fields for each port: */
        /* (1) Initialize MV_HWS_SINGLE_PORT_STC.supModesCatalog */
        if (NULL != hwsPortsSupModes)
        {
            curDevicePorts[portNum].supModesCatalog = hwsPortsSupModes[portNum];
        }

        if(curDevicePorts[portNum].supModesCatalog  !=  NULL)
        { /* if this port is useable (in some devices,not every port number is been used) */

        /* (2) Initialize MV_HWS_SINGLE_PORT_STC.perModeFecList */
           hwsPortsElementsFindMode(curDevicePorts[portNum].supModesCatalog,NON_SUP_MODE,&numOfSupModes);
           curDevicePorts[portNum].perModeFecList = (MV_HWS_PORT_FEC_MODE*)hwsOsMallocFuncPtr(numOfSupModes*sizeof(MV_HWS_PORT_FEC_MODE));
           if (NULL == curDevicePorts[portNum].perModeFecList)
           {
               return GT_NO_RESOURCE;
           }
           for (portMode = 0; portMode < numOfSupModes; portMode++)
           { /* for each portMode, copy the defualt FEC value from the right SupModesCatalog */
               curDevicePorts[portNum].perModeFecList[portMode]=
                   curDevicePorts[portNum].supModesCatalog[portMode].portFecMode;
           }

       /* (3) Initialize MV_HWS_SINGLE_PORT_STC.curPortParams */
           hwsPortModeParamsSetMode(devNum,0,portNum,NON_SUP_MODE);
        }
    }
    return GT_OK;
}

MV_HWS_PORT_INIT_PARAMS *hwsPortsElementsFindMode
(
    MV_HWS_PORT_INIT_PARAMS hwsPortSupModes[],
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32                  *portModeIndex
)
{
    GT_U32 i = 0;
    if(NULL == hwsPortSupModes)
    {
        *portModeIndex = NA_NUM;
        return NULL;
    }
    while (NON_SUP_MODE != hwsPortSupModes[i].portStandard &&
            i < LAST_PORT_MODE) { /* Last line of each SupMode array MUST be NON_SUP_MODE */
        if (hwsPortSupModes[i].portStandard == portMode)
        {
            if(NULL != portModeIndex) *portModeIndex = i;
            return &hwsPortSupModes[i];
        }
        i++;
    }
    if(NULL != portModeIndex)
    {
        (NON_SUP_MODE == portMode) ? (*portModeIndex = i) /* return num of supported modes */
                                   : (*portModeIndex = NA_NUM);
    }
    return NULL;
}

GT_STATUS debugPrvHwsPrintElementsDb
(
    GT_U8                   devNum,
    GT_U32                  portNum
)
{
    MV_HWS_PORT_INIT_PARAMS *portParamPtr;
    if(PRINT_ALL_DEVS == devNum) /* print all ports of all devices */
    {
        devNum = 0;
        while (devNum<HWS_MAX_DEVICE_NUM)
        {
            if (NULL == hwsDevicesPortsElementsArray[devNum])
            {
                devNum++;
                continue;
            }
            hwsOsPrintf("+----------+-----------+---------+-----+---------+-----+-----+-------+-------+---------------+--------+-------+-------+\n");
            hwsOsPrintf("| dev/port |    Port   |   MAC   | MAC |   PCS   | PCS | FEC | Speed | First | Active Lanes  | Num Of | Media | 10Bit |\n");
            hwsOsPrintf("|          |    Mode   |   Type  | Num |   Type  | Num |     |       | Lane  |     List      | Lanes  |       | 10Bit |\n");
            hwsOsPrintf("+----------+-----------+---------+-----+---------+-----+-----+-------+-------+---------------+--------+-------+-------+\n");
            for (portNum = 0; portNum < hwsDeviceSpecInfo[devNum].portsNum; portNum++)
            {
                portParamPtr = hwsPortModeParamsGet(devNum, 0, portNum, NON_SUP_MODE);
                if(portParamPtr == NULL)
                {
                    continue;
                }
                hwsOsPrintf( "|%3d/%-6d|0x%-9x|0x%-7x|%-5d|0x%-7x|%-5d|0x%-3x|0x%-5x|%-7d|%-3d,%-3d,%-3d,%-3d|%-8d|0x%-5x|0x%-5x|\n",
                             devNum, portNum,
                             portParamPtr->portStandard,
                             portParamPtr->portMacType, portParamPtr->portMacNumber,
                             portParamPtr->portPcsType, portParamPtr->portPcsNumber,
                             portParamPtr->portFecMode, portParamPtr->serdesSpeed,
                             portParamPtr->firstLaneNum,
                             portParamPtr->activeLanesList[0], portParamPtr->activeLanesList[1], portParamPtr->activeLanesList[2], portParamPtr->activeLanesList[3],
                             portParamPtr->numOfActLanes, portParamPtr->serdesMediaType,
                             portParamPtr->serdes10BitStatus);
            }
            hwsOsPrintf("+----------+-----------+---------+-----+---------+-----+-----+-------+-------+---------------+--------+-------+-------+\n");
            devNum++;
         }
    }
    else /* print specific port */
    {
        if (devNum >= HWS_MAX_DEVICE_NUM)
        {
            return GT_OUT_OF_RANGE;
        }
        if (NULL == hwsDevicesPortsElementsArray[devNum])
        {
            return GT_NOT_INITIALIZED;
        }
        if (portNum >= hwsDeviceSpecInfo[devNum].portsNum)
        {
            return GT_OUT_OF_RANGE;
        }
        portParamPtr = hwsPortModeParamsGet(devNum, 0, portNum, NON_SUP_MODE);
        if(portParamPtr == NULL)
        {
            return GT_NOT_INITIALIZED;;
        }

        hwsOsPrintf("+----------+-----------+---------+-----+---------+-----+-----+-------+-------+---------------+--------+-------+-------+\n");
        hwsOsPrintf("| dev/port |    Port   |   MAC   | MAC |   PCS   | PCS | FEC | Speed | First | Active Lanes  | Num Of | Media | 10Bit |\n");
        hwsOsPrintf("|          |    Mode   |   Type  | Num |   Type  | Num |     |       | Lane  |     List      | Lanes  |       | 10Bit |\n");
        hwsOsPrintf("+----------+-----------+---------+-----+---------+-----+-----+-------+-------+---------------+--------+-------+-------+\n");
        hwsOsPrintf( "|%3d/%-6d|0x%-9x|0x%-7x|%-5d|0x%-7x|%-5d|0x%-3x|0x%-5x|%-7d|%-3d,%-3d,%-3d,%-3d|%-8d|0x%-5x|0x%-5x|\n",
                     devNum, portNum,
                     portParamPtr->portStandard,
                     portParamPtr->portMacType, portParamPtr->portMacNumber,
                     portParamPtr->portPcsType, portParamPtr->portPcsNumber,
                     portParamPtr->portFecMode, portParamPtr->serdesSpeed,
                     portParamPtr->firstLaneNum,
                     portParamPtr->activeLanesList[0], portParamPtr->activeLanesList[1], portParamPtr->activeLanesList[2], portParamPtr->activeLanesList[3],
                     portParamPtr->numOfActLanes, portParamPtr->serdesMediaType,
                     portParamPtr->serdes10BitStatus);
        hwsOsPrintf("+----------+-----------+---------+-----+---------+-----+-----+-------+-------+---------------+--------+-------+-------+\n");
    }
    return GT_OK;
}

#endif
