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
* @file wrapCpssDxChPortAp.c
*
* @brief Wrapper functions for CPSS DXCH Port AP API
*
* @version   10
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>


/* Feature specific includes. */
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortAp.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>

#ifndef ASIC_SIMULATION_NO_HW
extern GT_STATUS appDemoApLog
(
    GT_BOOL enable,
    GT_U8 devNum,
    GT_PHYSICAL_PORT_NUM   portNum
);

extern GT_STATUS appDemoDxChPortApLinkUpDebug
(
    GT_U8 devNum,
    GT_PHYSICAL_PORT_NUM portNum0,
    GT_PHYSICAL_PORT_NUM portNum1,
    CPSS_PORT_SPEED_ENT speed,
    CPSS_PORT_INTERFACE_MODE_ENT ifMode,
    GT_BOOL noneceDisableValue,
    GT_U32 timesLoop,
    GT_U32 linkCheckDelay,
    GT_U32 enDelay,
    GT_U32 disDelay
);
#endif

extern GT_STATUS appDemoConfigApPorts
(
    GT_U8 devNum,
    GT_BOOL apEnable,
    GT_U32 numOfPortsToConfig,
    CPSS_PORTS_BMP_STC *apPrtBitmapPtr,
    CPSS_DXCH_PORT_AP_PARAMS_STC *apPortsArrPtr,
    GT_BOOL *useCpssDefultsArrPtr
);

/* counter for "get" functions of cpssDxChPortApPortConfig table */
static GT_PHYSICAL_PORT_NUM   portConfigEntryCount;

/* counter for cpssDxChPortApPortStatus table */
static GT_PHYSICAL_PORT_NUM   portStatusEntryCount;

/* counter for cpssDxChPortApPortStats table */
static GT_PHYSICAL_PORT_NUM   portStatsEntryCount;

/**
* @internal interatorIncrement function
* @endinternal
*
* @brief   Calculate next port number
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; xCat3; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] iteratorPtr              - (pointer to) current pysical port number
*
* @param[out] iteratorPtr              - (pointer to) pysical port number to treat next
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS interatorIncrement
(
    GT_U8                   devNum,
    GT_PHYSICAL_PORT_NUM    *iteratorPtr
)
{
    GT_STATUS result;
    GT_PHYSICAL_PORT_NUM physPortNum;
    static CPSS_DXCH_DETAILED_PORT_MAP_STC portMapShadow;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr = &portMapShadow;
    GT_U32 macNum;

    physPortNum = *iteratorPtr;

    CONVERT_BACK_DEV_PHYSICAL_PORT_MAC(devNum, physPortNum);

    /* skip CPU and not existing ports */
    for(result = GT_OK; physPortNum < CPSS_MAX_PORTS_NUM_CNS; physPortNum++)
    {
        macNum = physPortNum;
        if (PRV_CPSS_PHY_PORT_IS_EXIST_MAC(devNum, physPortNum) ||
            (PRV_CPSS_DXCH_PP_HW_INFO_PORT_MAP_CHECK_SUPPORTED_MAC(devNum) == GT_TRUE))
        {
            if(PRV_CPSS_DXCH_PP_HW_INFO_PORT_MAP_CHECK_SUPPORTED_MAC(devNum) == GT_TRUE)
            {
                result = cpssDxChPortPhysicalPortDetailedMapGet(devNum,
                                    physPortNum, /*OUT*/portMapShadowPtr);
                if (result != GT_OK)
                {
                    return result;
                }
                if((portMapShadowPtr->valid != GT_TRUE) || (portMapShadowPtr->portMap.mappingType !=
                                                            CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E))
                {
                    continue;
                }
                macNum = portMapShadowPtr->portMap.macNum;
            }

            if (PRV_CPSS_GE_PORT_GE_ONLY_E ==
                PRV_CPSS_DXCH_PORT_TYPE_OPTIONS_MAC(devNum, macNum))
            {
                continue;
            }

            if(CPSS_PP_FAMILY_DXCH_BOBCAT3_E == PRV_CPSS_PP_MAC(devNum)->devFamily ||
               CPSS_PP_FAMILY_DXCH_ALDRIN2_E == PRV_CPSS_PP_MAC(devNum)->devFamily )
            {
                if((72 == portMapShadowPtr->portMap.macNum) || (73 == portMapShadowPtr->portMap.macNum))
                {/* In BC3 AP not supported for these MAC's as they supposed to serve as CPU ports */
                    continue;
                }
            }

            break;
        }
        else
            continue;
    }

    *iteratorPtr = physPortNum;

    return result;
}

/**
* @internal wrCpssDxChPortApEnableSet function
* @endinternal
*
* @brief   Enable/disable AP engine (loads AP code into shared memory and starts AP
*         engine).
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; xCat3; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortApEnableSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8    devNum;
    GT_PORT_GROUPS_BMP  portGroupsBmp;
    GT_BOOL             enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portGroupsBmp = (1<<(GT_U32)inArgs[1]);
    enable  = (GT_BOOL)inArgs[2];

    /* call cpss api function */
    result = cpssDxChPortApEnableSet(devNum, portGroupsBmp, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortApEnableGet function
* @endinternal
*
* @brief   Get AP engine enabled and functional on port group (local core) status.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; xCat3; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port group or device
* @retval GT_BAD_PTR               - enabledPtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortApEnableGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;
    GT_U8      devNum;
    GT_U32     portGroupNum;
    GT_BOOL    enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portGroupNum = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPortApEnableGet(devNum, portGroupNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortApPortConfigEntryFecExtSet function
* @endinternal
*
* @brief   Enable/disable AP process on port.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; xCat3; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortApPortConfigEntryFecExtSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_U8                           devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    GT_BOOL                         apEnable;
    GT_BOOL                         useCpssDefault;
    CPSS_DXCH_PORT_AP_PARAMS_STC    apParams, *apParamsPtr = NULL;
    GT_U32                          i, j; /* loop iterators */
    GT_U32                          laneNum,
                                    lanesCount;
    GT_BOOL                         supported;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum      = (GT_U8)inArgs[0];
    portNum     = (GT_PHYSICAL_PORT_NUM)inFields[0];

    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    apEnable    = (GT_BOOL)inFields[1];
    if(GT_FALSE == apEnable)
    {
        goto config;
    }

    useCpssDefault = (GT_BOOL)inFields[2];
    if(GT_TRUE == useCpssDefault)
    {
        goto config;
    }
    else
    {
        apParamsPtr = &apParams;
    }

    apParams.fcAsmDir       = (GT_BOOL)inFields[3];
    apParams.fcPause        = (GT_BOOL)inFields[4];
    apParams.fecRequired    = (GT_BOOL)inFields[5];
    apParams.fecSupported   = (GT_BOOL)inFields[6];
    apParams.laneNum        = (GT_U32)inFields[7];
    apParams.noneceDisable  = (GT_U32)inFields[8];

    for(i = 9, j = 0; i < 9+4*CPSS_DXCH_PORT_AP_IF_ARRAY_SIZE_CNS; i+=4, j++)
    {
        apParams.modesAdvertiseArr[j].ifMode = (CPSS_PORT_INTERFACE_MODE_ENT)inFields[i];
        if(CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E == apParams.modesAdvertiseArr[j].ifMode)
        {
            break;
        }
        apParams.modesAdvertiseArr[j].speed = (CPSS_PORT_SPEED_ENT)inFields[i+1];
        /* to let configure different FEC advertisement for IEEE and consortium modes */
        apParams.fecAbilityArr[j] = (CPSS_DXCH_PORT_FEC_MODE_ENT)inFields[i+2];
        apParams.fecRequestedArr[j] = (CPSS_DXCH_PORT_FEC_MODE_ENT)inFields[i+3];
    }

config:

    if(GT_TRUE == apEnable)
    {
        result = cpssDxChPortInterfaceSpeedSupportGet(devNum, portNum,
                                                  CPSS_PORT_INTERFACE_MODE_XGMII_E,
                                                  CPSS_PORT_SPEED_10000_E,
                                                  &supported);
        if(result != GT_OK)
        {
            cpssOsPrintf("cpssDxChPortInterfaceSpeedSupportGet(portNum=%d,XGMII):rc=%d\n",
                            portNum, result);
            return result;
        }

        if(supported)
        {
            lanesCount = 6;
            for(laneNum = 0; laneNum < lanesCount; laneNum++)
            {
                (GT_VOID)cpssEventDeviceMaskWithEvExtDataSet(devNum,
                                                            CPSS_PP_PORT_LANE_SIGNAL_DETECT_CHANGED_E,
                                                            ((portNum<<8)|laneNum),
                                                            CPSS_EVENT_MASK_E);
            }
        }
        else
        {
            result = cpssDxChPortInterfaceSpeedSupportGet(devNum, portNum,
                                                      CPSS_PORT_INTERFACE_MODE_RXAUI_E,
                                                      CPSS_PORT_SPEED_10000_E,
                                                      &supported);
            if(result != GT_OK)
            {
                cpssOsPrintf("cpssDxChPortInterfaceSpeedSupportGet(portNum=%d,RXAUI):rc=%d\n",
                                portNum, result);
                return result;
            }
            if(supported)
            {
                lanesCount = 2;
                for(laneNum = 0; laneNum < lanesCount; laneNum++)
                {
                    (GT_VOID)cpssEventDeviceMaskWithEvExtDataSet(devNum,
                                                                CPSS_PP_PORT_LANE_SIGNAL_DETECT_CHANGED_E,
                                                                ((portNum<<8)|laneNum),
                                                                CPSS_EVENT_MASK_E);
                }
            }
        }

        (GT_VOID)cpssEventDeviceMaskWithEvExtDataSet(devNum,
                                                    CPSS_PP_PORT_SYNC_STATUS_CHANGED_E,
                                                    portNum,
                                                    CPSS_EVENT_MASK_E);

        (GT_VOID)cpssEventDeviceMaskWithEvExtDataSet(devNum,
                                                    CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E,
                                                    portNum,
                                                    CPSS_EVENT_MASK_E);
    }

    /* call cpss api function */
    result = cpssDxChPortApPortConfigSet(devNum, portNum, apEnable, apParamsPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortApPortConfigEntrySet function
* @endinternal
*
* @brief   Enable/disable AP process on port.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; xCat3; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortApPortConfigEntrySet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_U8                           devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    GT_BOOL                         apEnable;
    GT_BOOL                         useCpssDefault;
    CPSS_DXCH_PORT_AP_PARAMS_STC    apParams, *apParamsPtr = NULL;
    GT_U32                          i, j; /* loop iterators */
    GT_U32                          laneNum,
                                    lanesCount;
    GT_BOOL                         supported;
    GT_U32                          laneNumOffset;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum      = (GT_U8)inArgs[0];
    portNum     = (GT_PHYSICAL_PORT_NUM)inFields[0];

    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    apEnable    = (GT_BOOL)inFields[1];
    if(GT_FALSE == apEnable)
    {
        goto config;
    }

    useCpssDefault = (GT_BOOL)inFields[2];
    if(GT_TRUE == useCpssDefault)
    {
        goto config;
    }
    else
    {
        apParamsPtr = &apParams;
    }
    cpssOsMemSet(&apParams, 0, sizeof(apParams));

    apParams.fcAsmDir       = (GT_BOOL)inFields[3];
    apParams.fcPause        = (GT_BOOL)inFields[4];
    apParams.fecRequired    = (GT_BOOL)inFields[5];
    apParams.fecSupported   = (GT_BOOL)inFields[6];

    laneNumOffset = (29 == numFields) ? 7 : 9;
    apParams.laneNum        = (GT_U32)inFields[laneNumOffset];
    apParams.noneceDisable  = (GT_U32)inFields[laneNumOffset+1];

    for(i = laneNumOffset+2, j = 0; i < (laneNumOffset+2)+2*CPSS_DXCH_PORT_AP_IF_ARRAY_SIZE_CNS; i+=2, j++)
    {
        apParams.modesAdvertiseArr[j].ifMode =
            (CPSS_PORT_INTERFACE_MODE_ENT)inFields[i];
        if(CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E == apParams.modesAdvertiseArr[j].ifMode)
        {
            break;
        }
        apParams.modesAdvertiseArr[j].speed = (CPSS_PORT_SPEED_ENT)inFields[i+1];
        if(numFields > 29)
        {
            /* actually we have no HW ability to define specific FEC advertisement for every interface */
            apParams.fecAbilityArr[j] = (CPSS_DXCH_PORT_FEC_MODE_ENT)inFields[7];
            apParams.fecRequestedArr[j] = (CPSS_DXCH_PORT_FEC_MODE_ENT)inFields[8];
        }
    }

config:

    if(GT_TRUE == apEnable)
    {
        result = cpssDxChPortInterfaceSpeedSupportGet(devNum, portNum,
                                                  CPSS_PORT_INTERFACE_MODE_XGMII_E,
                                                  CPSS_PORT_SPEED_10000_E,
                                                  &supported);
        if(result != GT_OK)
        {
            cpssOsPrintf("cpssDxChPortInterfaceSpeedSupportGet(portNum=%d,XGMII):rc=%d\n",
                            portNum, result);
            return result;
        }

        if(supported)
        {
            lanesCount = 6;
            for(laneNum = 0; laneNum < lanesCount; laneNum++)
            {
                (GT_VOID)cpssEventDeviceMaskWithEvExtDataSet(devNum,
                                                            CPSS_PP_PORT_LANE_SIGNAL_DETECT_CHANGED_E,
                                                            ((portNum<<8)|laneNum),
                                                            CPSS_EVENT_MASK_E);
            }
        }
        else
        {
            result = cpssDxChPortInterfaceSpeedSupportGet(devNum, portNum,
                                                      CPSS_PORT_INTERFACE_MODE_RXAUI_E,
                                                      CPSS_PORT_SPEED_10000_E,
                                                      &supported);
            if(result != GT_OK)
            {
                cpssOsPrintf("cpssDxChPortInterfaceSpeedSupportGet(portNum=%d,RXAUI):rc=%d\n",
                                portNum, result);
                return result;
            }
            if(supported)
            {
                lanesCount = 2;
                for(laneNum = 0; laneNum < lanesCount; laneNum++)
                {
                    (GT_VOID)cpssEventDeviceMaskWithEvExtDataSet(devNum,
                                                                CPSS_PP_PORT_LANE_SIGNAL_DETECT_CHANGED_E,
                                                                ((portNum<<8)|laneNum),
                                                                CPSS_EVENT_MASK_E);
                }
            }
        }

        (GT_VOID)cpssEventDeviceMaskWithEvExtDataSet(devNum,
                                                    CPSS_PP_PORT_SYNC_STATUS_CHANGED_E,
                                                    portNum,
                                                    CPSS_EVENT_MASK_E);

        (GT_VOID)cpssEventDeviceMaskWithEvExtDataSet(devNum,
                                                    CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E,
                                                    portNum,
                                                    CPSS_EVENT_MASK_E);

        if(CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
        {
            (GT_VOID)cpssEventDeviceMaskWithEvExtDataSet(devNum,
                                                        CPSS_PP_PORT_LINK_STATUS_CHANGED_E,
                                                        portNum,
                                                        CPSS_EVENT_MASK_E);
        }
    }

    /* call cpss api function */
    result = cpssDxChPortApPortConfigSet(devNum, portNum, apEnable, apParamsPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal prvWrCpssDxChPortApPortConfigGet function
* @endinternal
*
* @brief   Get AP configuration of port.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; xCat3; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - apEnablePtr or apParamsPtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS prvWrCpssDxChPortApPortConfigGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    IN  GT_BOOL     enhanced, /* engaged for newer table with enhanced FEC fields */
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_U8                           devNum;
    GT_BOOL                         apEnable;
    CPSS_DXCH_PORT_AP_PARAMS_STC    apParams;
    GT_U32                          i, j; /* loop iterators */
    GT_U32                          portGroup;
    GT_BOOL                         apEnabledOnCore;
    GT_U32                          laneNumOffset;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    result = interatorIncrement(devNum, &portConfigEntryCount);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    if(portConfigEntryCount >= CPSS_MAX_PORTS_NUM_CNS)
    {
        /* no ports */
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                          portConfigEntryCount);
    result = cpssDxChPortApEnableGet(devNum,portGroup,&apEnabledOnCore);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    if(apEnabledOnCore)
    {
        cpssOsMemSet(&apParams, 0, sizeof(CPSS_DXCH_PORT_AP_PARAMS_STC));
        /* call cpss api function */
        result = cpssDxChPortApPortConfigGet(devNum,portConfigEntryCount,&apEnable,
                                             &apParams);
        if (result != GT_OK)
        {
            galtisOutput(outArgs, result, "%d", -1);
            return CMD_OK;
        }

        inFields[1] = apEnable;
        inFields[3] = apParams.fcAsmDir;
        inFields[4] = apParams.fcPause;
        inFields[5] = apParams.fecRequired;
        inFields[6] = apParams.fecSupported;
        /* actually we have no HW ability to define specific FEC advertisement for every interface */
        inFields[7] = apParams.fecAbilityArr[0];
        inFields[8] = apParams.fecRequestedArr[0];
        laneNumOffset = (enhanced) ? 9 : 7;
        inFields[laneNumOffset] = apParams.laneNum;
        inFields[laneNumOffset+1] = apParams.noneceDisable;

        for(i = laneNumOffset+2, j = 0; i < (laneNumOffset+2)+2*CPSS_DXCH_PORT_AP_IF_ARRAY_SIZE_CNS; i+=2, j++)
        {
            inFields[i] = apParams.modesAdvertiseArr[j].ifMode;
            inFields[i+1] = apParams.modesAdvertiseArr[j].speed;
        }
    }
    else
    {
        GT_U32 maxFields;
        maxFields = (enhanced) ? 30 : 28;
        inFields[1] = GT_FALSE; /* apEnable */
        for(i = 3; i<maxFields; i++)
        {
            inFields[i] = 0;
        }
        inFields[7] = CPSS_DXCH_PORT_FEC_MODE_DISABLED_E;
        inFields[8] = CPSS_DXCH_PORT_FEC_MODE_DISABLED_E;
    }
    inFields[0] = portConfigEntryCount;
    inFields[2] = GT_FALSE; /* useCpssDefault */

    /* pack and output table fields */
    if(enhanced)
    {
        fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
                    inFields[0],  inFields[1],  inFields[2],
                    inFields[3],  inFields[4],  inFields[5],
                    inFields[6],  inFields[7],  inFields[8],
                    inFields[9],  inFields[10], inFields[11],
                    inFields[12], inFields[13], inFields[14],
                    inFields[15], inFields[16], inFields[17],
                    inFields[18], inFields[19], inFields[20],
                    inFields[21], inFields[22], inFields[23],
                    inFields[24], inFields[25], inFields[26],
                    inFields[27], inFields[28], inFields[29]);
    }
    else
    {
        fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
                    inFields[0],  inFields[1],  inFields[2],
                    inFields[3],  inFields[4],  inFields[5],
                    inFields[6],  inFields[7],  inFields[8],
                    inFields[9],  inFields[10], inFields[11],
                    inFields[12], inFields[13], inFields[14],
                    inFields[15], inFields[16], inFields[17],
                    inFields[18], inFields[19], inFields[20],
                    inFields[21], inFields[22], inFields[23],
                    inFields[24], inFields[25], inFields[26],
                    inFields[27]);
    }


    galtisOutput(outArgs, GT_OK, "%f");

    portConfigEntryCount++;

    return CMD_OK;
}

/**
* @internal prvWrCpssDxChPortApPortConfigFecExtGet function
* @endinternal
*
* @brief   Get AP configuration of port.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; xCat3; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - apEnablePtr or apParamsPtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS prvWrCpssDxChPortApPortConfigFecExtGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_U8                           devNum;
    GT_BOOL                         apEnable;
    CPSS_DXCH_PORT_AP_PARAMS_STC    apParams;
    GT_U32                          i, j; /* loop iterators */
    GT_U32                          portGroup;
    GT_BOOL                         apEnabledOnCore;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    result = interatorIncrement(devNum, &portConfigEntryCount);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    if(portConfigEntryCount >= CPSS_MAX_PORTS_NUM_CNS)
    {
        /* no ports */
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                          portConfigEntryCount);
    result = cpssDxChPortApEnableGet(devNum,portGroup,&apEnabledOnCore);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    if(apEnabledOnCore)
    {
        cpssOsMemSet(&apParams, 0, sizeof(CPSS_DXCH_PORT_AP_PARAMS_STC));
        /* call cpss api function */
        result = cpssDxChPortApPortConfigGet(devNum,portConfigEntryCount,&apEnable,
                                             &apParams);
        if (result != GT_OK)
        {
            galtisOutput(outArgs, result, "%d", -1);
            return CMD_OK;
        }

        inFields[1] = apEnable;
        inFields[3] = apParams.fcAsmDir;
        inFields[4] = apParams.fcPause;
        inFields[5] = apParams.fecRequired;
        inFields[6] = apParams.fecSupported;
        inFields[7] = apParams.laneNum;
        inFields[8] = apParams.noneceDisable;

        for(i = 9, j = 0; i < 9+4*CPSS_DXCH_PORT_AP_IF_ARRAY_SIZE_CNS; i+=4, j++)
        {
            inFields[i] = apParams.modesAdvertiseArr[j].ifMode;
            inFields[i+1] = apParams.modesAdvertiseArr[j].speed;
            inFields[i+2] = (0 == apParams.modesAdvertiseArr[j].ifMode) ?
                                CPSS_DXCH_PORT_FEC_MODE_DISABLED_E : apParams.fecAbilityArr[j];
            inFields[i+3] = (0 == apParams.modesAdvertiseArr[j].ifMode) ?
                                CPSS_DXCH_PORT_FEC_MODE_DISABLED_E : apParams.fecRequestedArr[j];
        }
    }
    else
    {
        inFields[1] = GT_FALSE; /* apEnable */
        for(i = 3; i<9; i++)
        {
            inFields[i] = 0;
        }
        for(i = 9; i < 9+4*CPSS_DXCH_PORT_AP_IF_ARRAY_SIZE_CNS; i+=4)
        {
            inFields[i] = 0;
            inFields[i+1] = 0;
            inFields[i+2] = CPSS_DXCH_PORT_FEC_MODE_DISABLED_E;
            inFields[i+3] = CPSS_DXCH_PORT_FEC_MODE_DISABLED_E;
        }
    }
    inFields[0] = portConfigEntryCount;
    inFields[2] = GT_FALSE; /* useCpssDefault */

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
                inFields[0],  inFields[1],  inFields[2],
                inFields[3],  inFields[4],  inFields[5],
                inFields[6],  inFields[7],  inFields[8],
                inFields[9],  inFields[10], inFields[11],
                inFields[12], inFields[13], inFields[14],
                inFields[15], inFields[16], inFields[17],
                inFields[18], inFields[19], inFields[20],
                inFields[21], inFields[22], inFields[23],
                inFields[24], inFields[25], inFields[26],
                inFields[27], inFields[28], inFields[29],
                inFields[30], inFields[31], inFields[32],
                inFields[33], inFields[34], inFields[35],
                inFields[36], inFields[37], inFields[38],
                inFields[39], inFields[40], inFields[41],
                inFields[42], inFields[43], inFields[44],
                inFields[45], inFields[46], inFields[47],
                inFields[48]);

    galtisOutput(outArgs, GT_OK, "%f");

    portConfigEntryCount++;

    return CMD_OK;
}

static CMD_STATUS wrCpssDxChPortApPortConfigEntryFecExtGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    portConfigEntryCount = 0;

    return prvWrCpssDxChPortApPortConfigFecExtGet(inArgs,inFields,numFields,outArgs);
}

static CMD_STATUS wrCpssDxChPortApPortConfigEntryFecExtGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return prvWrCpssDxChPortApPortConfigFecExtGet(inArgs,inFields,numFields,outArgs);
}

static CMD_STATUS wrCpssDxChPortApPortConfigEntryEnhGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    portConfigEntryCount = 0;

    return prvWrCpssDxChPortApPortConfigGet(inArgs,inFields,numFields,GT_TRUE,outArgs);
}

static CMD_STATUS wrCpssDxChPortApPortConfigEntryEnhGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return prvWrCpssDxChPortApPortConfigGet(inArgs,inFields,numFields,GT_TRUE,outArgs);
}

static CMD_STATUS wrCpssDxChPortApPortConfigEntryGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    portConfigEntryCount = 0;

    return prvWrCpssDxChPortApPortConfigGet(inArgs,inFields,numFields,GT_FALSE,outArgs);
}

static CMD_STATUS wrCpssDxChPortApPortConfigEntryGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return prvWrCpssDxChPortApPortConfigGet(inArgs,inFields,numFields,GT_FALSE,outArgs);
}

/**
* @internal prvWrCpssDxChPortApPortStatusEntryGet function
* @endinternal
*
* @brief   Get status of AP on port.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; xCat3; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - apStatusPtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - AP engine not run
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS prvWrCpssDxChPortApPortStatusEntryGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    IN  GT_BOOL     invokedForAllPorts,
    IN  GT_BOOL     enhanced, /* engaged for newer table with enhanced FEC fields */
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_U8                           devNum;
    CPSS_DXCH_PORT_AP_STATUS_STC    apStatus;
    GT_U32                          portGroup;
    GT_BOOL                         apEnabled;
    CPSS_DXCH_PORT_FEC_MODE_ENT     mode;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;
    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if(invokedForAllPorts)
    {
        result = interatorIncrement(devNum, &portStatusEntryCount);
        if (result != GT_OK)
        {
            galtisOutput(outArgs, result, "%d", -1);
            return CMD_OK;
        }

        if(portStatusEntryCount >= CPSS_MAX_PORTS_NUM_CNS)
        {
            /* no ports */
            galtisOutput(outArgs, GT_OK, "%d", -1);
            return CMD_OK;
        }
    }
    else
    {
        portStatusEntryCount = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    }

    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                      portStatusEntryCount);
    result = cpssDxChPortApEnableGet(devNum,portGroup,&apEnabled);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    if(apEnabled)
    {
        cpssOsMemSet(&apStatus, 0, sizeof(CPSS_DXCH_PORT_AP_STATUS_STC));
        /* call cpss api function */
        result = cpssDxChPortApPortStatusGet(devNum, portStatusEntryCount,
                                             &apStatus);
        if (result != GT_OK)
        {
            galtisOutput(outArgs, result, "%d", -1);
            return CMD_OK;
        }

        inFields[1] = apStatus.postApPortNum;
        inFields[2] = apStatus.hcdFound;

        if (CPSS_PORT_SPEED_100G_E == apStatus.portMode.speed)
        {
            result = cpssDxChPortFecModeGet(devNum, portStatusEntryCount, &mode);
            if (result != GT_OK)
            {
                galtisOutput(outArgs, result, "%d", -1);
                return CMD_OK;
            }
        }

        /* under 100G_KR4 RS FEC always enabled not subject for negotiation */
        if (enhanced)
        {
            if (CPSS_PORT_SPEED_100G_E == apStatus.portMode.speed)
            {
                inFields[4] = mode;
            }
            else
            {
                if (apStatus.fecType != CPSS_DXCH_PORT_FEC_MODE_DISABLED_E)
                {
                    inFields[4] = apStatus.fecType;
                }
                else
                {
                    inFields[4] = (GT_TRUE == apStatus.fecEnabled) ?
                                                CPSS_DXCH_PORT_FEC_MODE_ENABLED_E :
                                                    CPSS_DXCH_PORT_FEC_MODE_DISABLED_E;
                }
            }
        }
        else
        {
            if (CPSS_PORT_SPEED_100G_E == apStatus.portMode.speed)
            {
                inFields[4] = (CPSS_DXCH_PORT_RS_FEC_MODE_ENABLED_E == mode);
            }
            else
            {
                inFields[4] = ((apStatus.fecType != CPSS_DXCH_PORT_FEC_MODE_DISABLED_E)
                               || apStatus.fecEnabled) ? GT_TRUE : GT_FALSE;
            }
        }

        inFields[5] = apStatus.portMode.ifMode;
        inFields[6] = apStatus.portMode.speed;
        inFields[7] = apStatus.fcRxPauseEn;
        inFields[8] = apStatus.fcTxPauseEn;
    }
    else
    {
        inFields[1] = 0;
        inFields[2] = 0;
        inFields[4] = 0;
        inFields[5] = CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E;
        inFields[6] = CPSS_PORT_SPEED_NA_E;
        inFields[7] = GT_FALSE;
        inFields[8] = GT_FALSE;
    }

    inFields[0] = portStatusEntryCount;
    inFields[3] = 0; /* was portInit */

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d%d",
                inFields[0],  inFields[1],  inFields[2],
                inFields[3],  inFields[4],  inFields[5],
                inFields[6], inFields[7], inFields[8]);

    galtisOutput(outArgs, GT_OK, "%f");

    portStatusEntryCount++;

    return CMD_OK;
}

static CMD_STATUS  wrCpssDxChPortApPortStatusEntryGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    portStatusEntryCount = 0;

    return prvWrCpssDxChPortApPortStatusEntryGet(inArgs, inFields, numFields,
                                                 GT_TRUE, GT_FALSE, outArgs);
}

static CMD_STATUS  wrCpssDxChPortApPortStatusEntryGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return prvWrCpssDxChPortApPortStatusEntryGet(inArgs, inFields, numFields,
                                                 GT_TRUE, GT_FALSE, outArgs);
}

static CMD_STATUS  wrCpssDxChPortApPortStatusEnhEntryGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    portStatusEntryCount = 0;

    return prvWrCpssDxChPortApPortStatusEntryGet(inArgs, inFields, numFields,
                                                 GT_TRUE, GT_TRUE, outArgs);
}

static CMD_STATUS  wrCpssDxChPortApPortStatusEnhEntryGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return prvWrCpssDxChPortApPortStatusEntryGet(inArgs, inFields, numFields,
                                                 GT_TRUE, GT_TRUE, outArgs);
}

static CMD_STATUS  wrCpssDxChPortApPortStatusOnPortGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return prvWrCpssDxChPortApPortStatusEntryGet(inArgs, inFields, numFields,
                                                 GT_FALSE, GT_FALSE, outArgs);
}

/**
* @internal wrCpssDxChPortApGetNext function
* @endinternal
*
* @brief   This function is called for ending table printing.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
*/
static CMD_STATUS wrCpssDxChPortApGetNext

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)

{
    galtisOutput(outArgs, GT_OK, "%d", -1);
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortApResolvedPortsBmpGet function
* @endinternal
*
* @brief   Get bitmap of ports on port group (local core) where AP process finished
*         with agreed for both sides resolution
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port group or device
* @retval GT_BAD_PTR               - apResolvedPortsBmpPtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortApResolvedPortsBmpGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;
    GT_U8      devNum;
    GT_U32     portGroupNum;
    GT_U32     apResolvedPortsBmp;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portGroupNum = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPortApResolvedPortsBmpGet(devNum, portGroupNum,
                                               &apResolvedPortsBmp);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "0x%x", apResolvedPortsBmp);
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortApSetActiveMode function
* @endinternal
*
* @brief   Update port's AP active lanes according to new interface.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortApSetActiveMode
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_U8                           devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;
    CPSS_PORT_SPEED_ENT             speed;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    ifMode  = (CPSS_PORT_INTERFACE_MODE_ENT)inArgs[2];
    speed   = (CPSS_PORT_SPEED_ENT)inArgs[3];

    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum,portNum);

    /* call cpss api function */
    result = cpssDxChPortApSetActiveMode(devNum, portNum, ifMode, speed);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortApStatsReset function
* @endinternal
*
* @brief   Reset AP port statistics information
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; xCat3; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success - port not in use by AP processor
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_INITIALIZED       - AP engine or library not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS  wrCpssDxChPortApStatsReset
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;

    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum  = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    result = cpssDxChPortApStatsReset(devNum, portNum);

    galtisOutput(outArgs, result, "");

    return CMD_OK;

}

/**
* @internal wrCpssDxChPortApIntropSet function
* @endinternal
*
* @brief   Set AP port introp information
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; xCat3; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - apStatusPtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - AP engine not run
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS  wrCpssDxChPortApIntropSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_U8                           devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    CPSS_DXCH_PORT_AP_INTROP_STC    apIntropParams;

    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum                                  = (GT_U8)inArgs[0];
    portNum                                 = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    apIntropParams.attrBitMask              = (GT_U16)inArgs[2];
    apIntropParams.txDisDuration            = (GT_U16)inArgs[3];
    apIntropParams.abilityDuration          = (GT_U16)inArgs[4];
    apIntropParams.abilityMaxInterval       = (GT_U16)inArgs[5];
    apIntropParams.abilityFailMaxInterval   = (GT_U16)inArgs[6];
    apIntropParams.apLinkDuration           = (GT_U16)inArgs[7];
    apIntropParams.apLinkMaxInterval        = (GT_U16)inArgs[8];
    apIntropParams.pdLinkDuration           = (GT_U16)inArgs[9];
    apIntropParams.pdLinkMaxInterval        = (GT_U16)inArgs[10];

    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    result = cpssDxChPortApIntropSet(devNum, portNum, &apIntropParams);

    galtisOutput(outArgs, result, "");

    return CMD_OK;

}

/**
* @internal wrCpssDxChPortApIntropGet function
* @endinternal
*
* @brief   Get AP port introp information
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; xCat3; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - apStatusPtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - AP engine not run
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS  wrCpssDxChPortApIntropGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_U8                           devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    CPSS_DXCH_PORT_AP_INTROP_STC    apIntropParams;

    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum      = (GT_U8)inArgs[0];
    portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    result = cpssDxChPortApIntropGet(devNum, portNum, &apIntropParams);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
    }
    else
    {
        inFields[0] = apIntropParams.attrBitMask;
        inFields[1] = apIntropParams.txDisDuration;
        inFields[2] = apIntropParams.abilityDuration;
        inFields[3] = apIntropParams.abilityMaxInterval;
        inFields[4] = apIntropParams.abilityFailMaxInterval;
        inFields[5] = apIntropParams.apLinkDuration;
        inFields[6] = apIntropParams.apLinkMaxInterval;
        inFields[7] = apIntropParams.pdLinkDuration;
        inFields[8] = apIntropParams.pdLinkMaxInterval;

        galtisOutput(outArgs, result, "%d%d%d%d%d%d%d%d%d", apIntropParams.attrBitMask, apIntropParams.txDisDuration,
                     apIntropParams.abilityDuration, apIntropParams.abilityMaxInterval, apIntropParams.abilityFailMaxInterval,
                     apIntropParams.apLinkDuration, apIntropParams.apLinkMaxInterval, apIntropParams.pdLinkDuration,
                     apIntropParams.pdLinkMaxInterval);
    }

    return CMD_OK;
}

/**
* @internal prvWrCpssDxChPortApPortsStatsGet function
* @endinternal
*
* @brief   Get AP port statistics information
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; xCat3; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - apStatusPtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - AP engine not run
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS prvWrCpssDxChPortApPortsStatsGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    IN  GT_BOOL     invokedForAllPorts,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_U8                           devNum;
    CPSS_DXCH_PORT_AP_STATS_STC     apStats;
    GT_U32                          portGroup;
    GT_BOOL                         apEnabled;
    GT_U16                          intropAbilityMaxInterval;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if(invokedForAllPorts)
    {
        result = interatorIncrement(devNum, &portStatsEntryCount);
        if (result != GT_OK)
        {
            galtisOutput(outArgs, result, "%d", -1);
            return CMD_OK;
        }

        if(portStatsEntryCount >= CPSS_MAX_PORTS_NUM_CNS)
        {
            /* no ports */
            galtisOutput(outArgs, GT_OK, "%d", -1);
            return CMD_OK;
        }
    }
    else
    {
        portStatsEntryCount = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    }

    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portStatsEntryCount);
    result = cpssDxChPortApEnableGet(devNum,portGroup,&apEnabled);

    if (result != GT_OK || apEnabled == GT_FALSE)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    /* Initializing apStats structure */
    cpssOsMemSet(&apStats, 0, sizeof(CPSS_DXCH_PORT_AP_STATS_STC));

    /* call cpss api function */
    result = cpssDxChPortApStatsGet(devNum, portStatsEntryCount, &apStats, &intropAbilityMaxInterval);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    if (!invokedForAllPorts)
    {
        inFields[0] = apStats.txDisCnt;
        /* Avoiding arithmetic exception */
        if (intropAbilityMaxInterval==0)
        {
            inFields[1] = 0;
            inFields[2] = 0;
        }
        else
        {
            inFields[1] = apStats.abilityCnt % intropAbilityMaxInterval;
            inFields[2] = apStats.abilityCnt / intropAbilityMaxInterval;
        }
        inFields[3] = apStats.abilitySuccessCnt;
        inFields[4] = apStats.linkFailCnt;
        inFields[5] = apStats.linkSuccessCnt;
        inFields[6] = apStats.hcdResoultionTime;
        inFields[7] = apStats.linkUpTime;

        fieldOutput("%d%d%d%d%d%d%d%d", inFields[0], inFields[1], inFields[2],
                    inFields[3], inFields[4], inFields[5], inFields[6], inFields[7]);
    }
    else
    {
        inFields[0] = portStatsEntryCount;
        inFields[1] = apStats.txDisCnt;
        /* Avoiding arithmetic exception */
        if (intropAbilityMaxInterval==0)
        {
            inFields[2] = 0;
            inFields[3] = 0;
        }
        else
        {
            inFields[2] = apStats.abilityCnt % intropAbilityMaxInterval;
            inFields[3] = apStats.abilityCnt / intropAbilityMaxInterval;
        }
        inFields[4] = apStats.abilitySuccessCnt;
        inFields[5] = apStats.linkFailCnt;
        inFields[6] = apStats.linkSuccessCnt;
        inFields[7] = apStats.hcdResoultionTime;
        inFields[8] = apStats.linkUpTime;

        portStatsEntryCount++;

        fieldOutput("%d%d%d%d%d%d%d%d%d", inFields[0], inFields[1], inFields[2],
                    inFields[3], inFields[4], inFields[5], inFields[6], inFields[7], inFields[8]);

    }

    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}

static CMD_STATUS  wrCpssDxChPortApPortsStatsGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    portStatsEntryCount = 0;

    return prvWrCpssDxChPortApPortsStatsGet(inArgs, inFields, numFields,
                                                 GT_TRUE, outArgs);
}

static CMD_STATUS  wrCpssDxChPortApPortsStatsGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return prvWrCpssDxChPortApPortsStatsGet(inArgs, inFields, numFields,
                                                 GT_TRUE, outArgs);
}

static CMD_STATUS  wrCpssDxChPortApPortStatsGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return prvWrCpssDxChPortApPortsStatsGet(inArgs, inFields, numFields,
                                                 GT_FALSE, outArgs);
}

/**
* @internal wrCpssDxChPortApPortStatsGetNext function
* @endinternal
*
* @brief   This function is called for ending table printing.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
*/
static CMD_STATUS wrCpssDxChPortApPortStatsGetNext

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)

{
    galtisOutput(outArgs, GT_OK, "%d", -1);
    return CMD_OK;
}

static CPSS_DXCH_PORT_AP_PARAMS_STC         apPortsArray[CPSS_MAX_PORTS_NUM_CNS];
static GT_BOOL                              useCpssDefaultsArray[CPSS_MAX_PORTS_NUM_CNS];
static GT_U32                               numOfApPortsToConfig;
static CPSS_PORTS_BMP_STC                   apPortBitmap;

/**
* @internal internalAPPortsSet function
* @endinternal
*
* @brief   configure AP ports
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS internalAPPortsSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER],
    IN GT_U32   opCode
)
{
    GT_STATUS                               result;
    GT_U8                                   devNum;
    GT_PHYSICAL_PORT_NUM                    port;
    GT_BOOL                                 apEnable;
    GT_U32                                  i,j;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if(opCode == 0)/*start registration*/
    {
        numOfApPortsToConfig = 0;
        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&apPortBitmap);
    }
    else if(opCode == 0xFFFFFFFF)/*end registration*/
    {
        devNum = (GT_U8)inArgs[0];
        apEnable = (GT_BOOL)inArgs[1];

        result = appDemoConfigApPorts(devNum, apEnable, numOfApPortsToConfig, &apPortBitmap, apPortsArray, useCpssDefaultsArray);
        if(result != GT_OK)
        {
            galtisOutput(outArgs, result, "%d",-1);/* Error ! */
            return CMD_OK;
        }
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, GT_OK, "");
        return CMD_OK;
    }

    if (numOfApPortsToConfig > 255)
    {
        galtisOutput(outArgs, GT_FULL, "%d",-1);/* Error ! */
        return CMD_OK;
    }

    port = (GT_PHYSICAL_PORT_NUM)inFields[0];
    devNum = (GT_U8)inArgs[0];
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, port);
    CPSS_PORTS_BMP_PORT_SET_MAC(&apPortBitmap, port);

    useCpssDefaultsArray[numOfApPortsToConfig] = (GT_BOOL)inFields[1];
    if(useCpssDefaultsArray[numOfApPortsToConfig] == GT_FALSE)
    {
        GT_BOOL fecExtTbl; /* is it table with FEC ability/request per interface */
        GT_U32  step;   /* step to skip in loop */

        fecExtTbl = (numFields > 27) ? GT_TRUE : GT_FALSE;
        step = fecExtTbl ? 4 : 2;

        apPortsArray[numOfApPortsToConfig].fcAsmDir = (GT_BOOL)inFields[2];
        apPortsArray[numOfApPortsToConfig].fcPause = (CPSS_DXCH_PORT_AP_FLOW_CONTROL_ENT)inFields[3];
        apPortsArray[numOfApPortsToConfig].fecRequired = (GT_BOOL)inFields[4];
        apPortsArray[numOfApPortsToConfig].fecSupported = (GT_BOOL)inFields[5];
        apPortsArray[numOfApPortsToConfig].laneNum = (GT_BOOL)inFields[6];
        apPortsArray[numOfApPortsToConfig].noneceDisable = (GT_U32)inFields[7];

        for(i = 8, j = 0; j < CPSS_DXCH_PORT_AP_IF_ARRAY_SIZE_CNS; i+=step, j++)
        {
            apPortsArray[numOfApPortsToConfig].modesAdvertiseArr[j].ifMode = (CPSS_PORT_INTERFACE_MODE_ENT)inFields[i];
            if(CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E == apPortsArray[numOfApPortsToConfig].modesAdvertiseArr[j].ifMode)
            {
                break;
            }
            apPortsArray[numOfApPortsToConfig].modesAdvertiseArr[j].speed = (CPSS_PORT_SPEED_ENT)inFields[i+1];
            if(fecExtTbl)
            {
                apPortsArray[numOfApPortsToConfig].fecAbilityArr[j] = (CPSS_DXCH_PORT_FEC_MODE_ENT)inFields[i+2];
                apPortsArray[numOfApPortsToConfig].fecRequestedArr[j] = (CPSS_DXCH_PORT_FEC_MODE_ENT)inFields[i+3];
            }
        }
    }
    numOfApPortsToConfig++;

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "");

    return CMD_OK;
}

static CMD_STATUS wrCpssDxChPortApPortsSetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return internalAPPortsSet(inArgs,inFields,numFields,outArgs,0);
}

static CMD_STATUS wrCpssDxChPortApPortsSetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return internalAPPortsSet(inArgs,inFields,numFields,outArgs,1);
}

static CMD_STATUS wrCpssDxChPortApPortsEndSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return internalAPPortsSet(inArgs,inFields,numFields,outArgs,0xFFFFFFFF);
}

/**
* @internal wrCpssDxChPortApDebugInfoEnable function
* @endinternal
*
* @brief   Enable/disable application thread for AP ports real time logging.
*
* @note   APPLICABLE DEVICES:      BC2; BOBK.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortApDebugInfoEnable

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
#ifndef ASIC_SIMULATION_NO_HW
    GT_STATUS                       result;
    GT_U8                           devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    GT_BOOL                         enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    enable  = (GT_BOOL)inArgs[2];

    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum,portNum);

    result = appDemoApLog(enable, devNum, portNum);

    galtisOutput(outArgs, result, "");
#endif

    return CMD_OK;
}

/**
* @internal wrCpssDxChPortApLinkUpDebug function
* @endinternal
*
* @brief   debug function to check AP linkUp stabilitiy
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong port number or device
*/

static CMD_STATUS wrCpssDxChPortApLinkUpDebug

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    #ifndef ASIC_SIMULATION_NO_HW
    GT_STATUS rc;
    GT_U8 devNum;
    GT_PHYSICAL_PORT_NUM portNum0;
    GT_PHYSICAL_PORT_NUM portNum1;
    CPSS_PORT_SPEED_ENT speed;
    CPSS_PORT_INTERFACE_MODE_ENT ifMode;
    GT_BOOL noneceDisableValue;
    GT_U32 timesLoop;
    GT_U32 linkCheckDelay;
    GT_U32 enDelay;
    GT_U32 disDelay;

        /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

        /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    portNum0 = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    portNum1 = (GT_PHYSICAL_PORT_NUM)inArgs[2];
    speed = (CPSS_PORT_SPEED_ENT)inArgs[3];
    ifMode = (CPSS_PORT_INTERFACE_MODE_ENT)inArgs[4];
    noneceDisableValue = (GT_BOOL)inArgs[5];
    timesLoop = (GT_U32)inArgs[6];
    linkCheckDelay = (GT_U32)inArgs[7];
    enDelay = (GT_U32)inArgs[8];
    disDelay = (GT_U32)inArgs[9];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum0);
    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum1);

    rc = appDemoDxChPortApLinkUpDebug(devNum,portNum0,portNum1,speed,
                ifMode,noneceDisableValue,timesLoop, linkCheckDelay, enDelay, disDelay);
    if (rc != GT_OK)
    {
        galtisOutput(outArgs, rc, "appDemoDxChPortApLinkUpDebug");
        return CMD_OK;
    }

    galtisOutput(outArgs, rc, "%f");

    #endif

    return CMD_OK;
}



/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
    {"cpssDxChPortApEnableSet",
        &wrCpssDxChPortApEnableSet,
        3, 0},

    {"cpssDxChPortApEnableGet",
        &wrCpssDxChPortApEnableGet,
        2, 0},


    {"cpssDxChPortApPortConfigEntrySet",
        &wrCpssDxChPortApPortConfigEntrySet,
        1, 29},
    {"cpssDxChPortApPortConfigEntryGetFirst",
        &wrCpssDxChPortApPortConfigEntryGetFirst,
        1, 0},
    {"cpssDxChPortApPortConfigEntryGetNext",
        &wrCpssDxChPortApPortConfigEntryGetNext,
        1, 0},

    {"cpssDxChPortApPortConfigEntryEnhFecSet",
        &wrCpssDxChPortApPortConfigEntrySet,
        1, 31},
    {"cpssDxChPortApPortConfigEntryEnhFecGetFirst",
        &wrCpssDxChPortApPortConfigEntryEnhGetFirst,
        1, 0},
    {"cpssDxChPortApPortConfigEntryEnhFecGetNext",
        &wrCpssDxChPortApPortConfigEntryEnhGetNext,
        1, 0},

    {"cpssDxChPortApPortConfigEntryFecExtSet",
        &wrCpssDxChPortApPortConfigEntryFecExtSet,
        1, 49},
    {"cpssDxChPortApPortConfigEntryFecExtGetFirst",
        &wrCpssDxChPortApPortConfigEntryFecExtGetFirst,
        1, 0},
    {"cpssDxChPortApPortConfigEntryFecExtGetNext",
        &wrCpssDxChPortApPortConfigEntryFecExtGetNext,
        1, 0},

    {"cpssDxChPortApPortConfigEntriesFecExtSetFirst",
        &wrCpssDxChPortApPortsSetFirst,
        2, 48},
    {"cpssDxChPortApPortConfigEntriesFecExtSetNext",
        &wrCpssDxChPortApPortsSetNext,
        2, 48},
    {"cpssDxChPortApPortConfigEntriesFecExtEndSet",
        &wrCpssDxChPortApPortsEndSet,
        2, 0},

    {"cpssDxChPortApPortStatusEntryExtGetFirst",
        &wrCpssDxChPortApPortStatusEntryGetFirst,
        1, 0},
    {"cpssDxChPortApPortStatusEntryExtGetNext",
        &wrCpssDxChPortApPortStatusEntryGetNext,
        1, 0},

    {"cpssDxChPortApPortStatusEntryEnhFecGetFirst",
        &wrCpssDxChPortApPortStatusEnhEntryGetFirst,
        1, 0},
    {"cpssDxChPortApPortStatusEntryEnhFecGetNext",
        &wrCpssDxChPortApPortStatusEnhEntryGetNext,
        1, 0},

    {"cpssDxChPortApPortStatusOnPortGetFirst",
        &wrCpssDxChPortApPortStatusOnPortGetFirst,
        2, 0},
    {"cpssDxChPortApPortStatusOnPortGetNext",
        &wrCpssDxChPortApGetNext,
        2, 0},

    {"cpssDxChPortApResolvedPortsBmpGet",
        &wrCpssDxChPortApResolvedPortsBmpGet,
        2, 0},

    {"cpssDxChPortApSetActiveMode",
        &wrCpssDxChPortApSetActiveMode,
        4, 0},

    {"cpssDxChPortApStatsGetFirst",
        &wrCpssDxChPortApPortsStatsGetFirst,
        1, 0},
    {"cpssDxChPortApStatsGetNext",
        &wrCpssDxChPortApPortsStatsGetNext,
        1, 0},
    {"cpssDxChPortApStatsOnPortGetFirst",
        &wrCpssDxChPortApPortStatsGetFirst,
        2, 0},
    {"cpssDxChPortApStatsOnPortGetNext",
        &wrCpssDxChPortApPortStatsGetNext,
        2, 0},
    {"cpssDxChPortApStatsReset",
        &wrCpssDxChPortApStatsReset,
        2, 0},
    {"cpssDxChPortApIntropSet",
        &wrCpssDxChPortApIntropSet,
        11, 0},
    {"cpssDxChPortApIntropGet",
        &wrCpssDxChPortApIntropGet,
        2, 0},
    {"cpssDxChPortApPortConfigEntriesSetFirst",
        &wrCpssDxChPortApPortsSetFirst,
        2, 27},
    {"cpssDxChPortApPortConfigEntriesSetNext",
        &wrCpssDxChPortApPortsSetNext,
        2, 27},
    {"cpssDxChPortApPortConfigEntriesEndSet",
        &wrCpssDxChPortApPortsEndSet,
        2, 0},
    {"cpssDxChPortApDebugInfoEnable",
        &wrCpssDxChPortApDebugInfoEnable,
        3, 0},
    {"cpssDxChPortApLinkUpDebugSet",
        &wrCpssDxChPortApLinkUpDebug,
        10, 0},

};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssDxChPortAp function
* @endinternal
*
* @brief   Library database initialization function.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*
* @note none
*
*/
GT_STATUS cmdLibInitCpssDxChPortAp
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}


