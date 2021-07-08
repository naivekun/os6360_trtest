/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* wrapPhyCpssDxCh.c
*
* DESCRIPTION:
*       Wrapper functions for Phy cpss.dxCh functions
*
* FILE REVISION NUMBER:
*       $Revision: 103 $
*
*******************************************************************************/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>

/* Feature specific includes. */
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortManager.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortAp.h>

/* port used in status get tables */
static GT_PHYSICAL_PORT_NUM portForMacCountersGet = 0xFF;

static GT_STATUS  util_wrCpssDxChPortManagerStatusGet
(
    IN     GT_U8        devNum,
    INOUT  GT_PHYSICAL_PORT_NUM        *portNumPtr,
    IN     GT_BOOL      insertPortNumBeforeTab,
    IN     GT_BOOL      invokedForAllPorts,
    OUT    GT_UINTPTR   inFields[CMD_MAX_FIELDS],
    OUT    GT_8         outArgs[CMD_MAX_BUFFER]
);

extern GT_U32  disableAutoPortMgr;
extern GT_BOOL portMgr;

GT_STATUS wrPrvCpssDxChPortManagerGetPortMacNumber(GT_U8 devNum, GT_PHYSICAL_PORT_NUM portNum, GT_U32 *portMacNum)
{
    GT_U32 tmpPortMacNum;

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, tmpPortMacNum);
    *portMacNum = tmpPortMacNum;

    return GT_OK;
}

/**
* @internal wrCpssDxChPortManagerEvetSet function
* @endinternal
*
* @brief   Set the port according to the given event. This API performs actions based on
*         the given event and will transfer the port to a new state. For example, when port
*         is in PORT_MANAGER_STATE_RESET_E state and event PORT_MANAGER_EVENT_CREATE_E is called,
*         this API will perform port creation operations and upon success, port state will be
*         changed to PORT_MANAGER_STATE_LINK_DOWN_E state.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; AC3X.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssDxChPortManagerEvetSet
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
    CPSS_PORT_MANAGER_STC portEventStc;
    CPSS_PORT_MANAGER_EVENT_ENT portEvent;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if (!portMgr) /* Port Manager was not enabled */
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum          = (GT_U8)inArgs[0];
    portNum         = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    portEvent       = (CPSS_PORT_MANAGER_EVENT_ENT)inArgs[2];

    portEventStc.portEvent = portEvent;

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPortManagerEventSet(devNum, portNum, &portEventStc );

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPortManagerElectricalParamsSet function
* @endinternal
*
* @brief   Set the port TX parameters, use this API to override
*          the defaults values.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; AC3X.
*
* * @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                   - cpss interface mode
*  @param[in] speed                   - cpss speed mode
* @param[in] lanesNum                 - number of lanes
* @param[in] txAmp                    - txAmp
* @param[in] emph0                    - emph0
* @param[in] emph1                    - emph1
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssDxChPortManagerElectricalParamsSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;
    CPSS_PORT_SPEED_ENT             speed;
    GT_U8                           lanesNum;
    GT_U32                          txAmp;
    GT_U32                          emph0;
    GT_U32                          emph1;

    CPSS_PM_PORT_PARAMS_STC portParams;
    GT_STATUS result;
    GT_U32 serdesIndex;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if (!portMgr) /* Port Manager was not enabled */
    {
        return CMD_AGENT_ERROR;
    }

    result = cpssDxChPortManagerPortParamsStructInit(CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E, &portParams);
    if(result != GT_OK)
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum       = (GT_U8)inArgs[0];
    portNum      = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    ifMode       = (CPSS_PORT_INTERFACE_MODE_ENT)inArgs[2];
    speed        = (CPSS_PORT_SPEED_ENT)inArgs[3];
    lanesNum     = (GT_U8)inArgs[4];
    txAmp        = (GT_U32)inArgs[5];
    emph0        = (GT_U32)inArgs[6];
    emph1        = (GT_U32)inArgs[7];

    /* standard */
    portParams.portParamsType.regPort.ifMode = ifMode;
    portParams.portParamsType.regPort.speed = speed;

    for (serdesIndex= 0; serdesIndex< lanesNum; serdesIndex++)
    {
        CPSS_PM_SET_VALID_LANE_PARAM(&portParams, serdesIndex, CPSS_PM_LANE_PARAM_TX_E);

        portParams.portParamsType.regPort.laneParams[serdesIndex].txParams.txAmp = txAmp;
        portParams.portParamsType.regPort.laneParams[serdesIndex].txParams.emph0 = emph0;
        portParams.portParamsType.regPort.laneParams[serdesIndex].txParams.emph1 = emph1;
    }

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    result = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPortManagerMultiPortParamsSet function
* @endinternal
*
* @brief   Set the port manager paraneters for multi-ports,
*          based on a given string. call with overrideEnable
*          only after cpssDxChPortManagerPolarityParamsSet
*          or cpssDxChPortManagerElectricalParamsSet, else call
*          with overrideEnable = GT_FALSE
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] ifMode                   - cpss interface mode
*  @param[in] speed                   - cpss speed mode
* @param[in] adaptiveMode             - wheater to run adaptive
*       or not
* @param[in] trainMode                - training mode
* @param[in] overrideElectricalParams - set to true if  Tx
*       values were overriden
* @param[in] overridePolarityParams                    - set to
*       true if polarity was overriden
* @param[in]                          - ports string
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssDxChPortManagerMultiPortParamsSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8                           devNum;
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;
    CPSS_PORT_SPEED_ENT             speed;
    GT_BOOL                         adaptiveMode;
    CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT  trainMode;
    GT_BOOL                              overrideElectricalParams;
    GT_BOOL                              overridePolarityParams;
    char*                           portString;
    GT_U8                           minLF;
    GT_U8                           maxLF;
    CPSS_PORT_FEC_MODE_ENT          fecMode;

    GT_U32 ii,jj;
    GT_PHYSICAL_PORT_NUM  portNum;
    GT_U32 temp = 0;
    GT_STATUS result;
    char *ports = (char*)cpssOsMalloc(CPSS_MAX_PORTS_NUM_CNS*(sizeof(char)));
    cpssOsMemSet(&ports,0,sizeof(ports));

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if (!portMgr) /* Port Manager was not enabled */
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum          = (GT_U8)inArgs[0];
    ifMode         = (CPSS_PORT_INTERFACE_MODE_ENT)inArgs[1];
    speed       = (CPSS_PORT_SPEED_ENT)inArgs[2];
    adaptiveMode  = (GT_BOOL)inArgs[3];
    trainMode  =(CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT)inArgs[4];
    overrideElectricalParams  = (GT_BOOL)inArgs[5];
    overridePolarityParams  = (GT_BOOL)inArgs[6];
    portString  = (char*)inArgs[7];
    minLF       = (GT_U8)inArgs[8];
    maxLF       = (GT_U8)inArgs[9];
    fecMode     = (CPSS_PORT_FEC_MODE_ENT)inArgs[10];


    ports = portString;

    /* start loop on all relevant ports */
    osPrintf("\n ports %s ", ports);

    for (ii = 0; ports[ii] != '\0'; ii++)
    {
        if (ports[ii] == ',')
        {
            continue;
        }
        /* if portNum is 1 digit */
        else if ((ports[ii+1] == ',') || (ports[ii+1] == '\0'))
        {
            portNum = ports[ii] - '0';
        }
        /* if portNum is above 1 digit */
        else
        {
            for (jj = ii; jj < ii+2; jj++)
            {
                temp = temp * 10 + (ports[jj] - '0');
            }
            portNum = temp;
            temp = 0;
            ii++;
        }
        /* Override Device and Port */
        CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

        osPrintf("\n setting Port-Manager parameters for port %d ", portNum);

        result = prvCpssDxChPortManagerParametersSetExt(devNum,portNum,ifMode,speed,adaptiveMode,trainMode, overrideElectricalParams, overridePolarityParams, minLF, maxLF, fecMode);

        /* pack output arguments to galtis string */
        galtisOutput(outArgs, result, "");
    }
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortManagerMultiPortsEventSet function
* @endinternal
*
* @brief   set the port event on multi ports, based on a given
*          string for example "0,16,36,40"
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portEvent                - port manager event type
*  @param[in] speed                   - cpss speed mode
* @param[in] portString               - ports number string
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

CMD_STATUS wrCpssDxChPortManagerMultiPortsEventSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8                   devNum;
    CPSS_PORT_MANAGER_EVENT_ENT portEvent;
    char*                   portString;
    GT_U32 ii,jj;
    GT_PHYSICAL_PORT_NUM  portNum;
    GT_U32 temp = 0;
    GT_STATUS result;
    CPSS_PORT_MANAGER_STC portEventStc;
    char *ports = (char*)cpssOsMalloc(CPSS_MAX_PORTS_NUM_CNS*(sizeof(char)));
    cpssOsMemSet(&ports,0,sizeof(ports));


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if (!portMgr) /* Port Manager was not enabled */
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum          = (GT_U8)inArgs[0];
    portEvent       = (CPSS_PORT_MANAGER_EVENT_ENT)inArgs[1];
    portString      = (char*)inArgs[2];

    portEventStc.portEvent = portEvent;
    ports = portString;

    /* start loop on all relevant ports */
    osPrintf("\n ports %s ", ports);

    for (ii = 0; ports[ii] != '\0'; ii++)
    {
        if (ports[ii] == ',')
        {
            continue;
        }
        /* if portNum is 1 digit */
        else if ((ports[ii+1] == ',') || (ports[ii+1] == '\0'))
        {
            portNum = ports[ii] - '0';
        }
        /* if portNum is above 1 digit */
        else
        {
            for (jj = ii; jj < ii+2; jj++)
            {
                temp = temp * 10 + (ports[jj] - '0');
            }
            portNum = temp;
            temp = 0;
            ii++;
        }
        /* Override Device and Port */
        CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
        osPrintf("\n setEvent on Port-Manager, portNum %d ", portNum);

        result = cpssDxChPortManagerEventSet(devNum,portNum,&portEventStc);
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, result, "");
    }
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortManagerStatusGetFirst function
* @endinternal
*
* @brief   Get the status of the port. This API will return (1) State of the port
*         within the port manager state machine (2) Whether or not port have been
*         disabled (using PORT_MANAGER_EVENT_DISABLE_E event) (3) Failure status.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; AC3X.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssDxChPortManagerStatusGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8                           devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    CPSS_PORT_MANAGER_STATUS_STC portStageStatus;
    GT_STATUS result;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if (!portMgr) /* Port Manager was not enabled */
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_U8)inArgs[1];

    portForMacCountersGet = portNum;

    result = cpssDxChPortManagerStatusGet(devNum,portNum,&portStageStatus);

    inFields[0] = GT_TRUE;/*portStageStatus.isPortUnderManagement;*/
    inFields[1] = portStageStatus.portState;
    inFields[2] = portStageStatus.portUnderOperDisable;
    inFields[3] = portStageStatus.failure;

        /* pack and output table fields */
    fieldOutput("%d%d%d%d",
            inFields[0],  inFields[1],
            inFields[2],  inFields[3]);

    galtisOutput(outArgs, result, "%f");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPortManagerStatusGetNext function
* @endinternal
*
* @brief   Get the status of the port. This API will return (1) State of the port
*         within the port manager state machine (2) Whether or not port have been
*         disabled (using PORT_MANAGER_EVENT_DISABLE_E event) (3) Failure status.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; AC3X.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssDxChPortManagerStatusGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8                           devNum;
    GT_PHYSICAL_PORT_NUM            portNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if (!portMgr) /* Port Manager was not enabled */
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* print for specified port only done at "GetFirst" call */
    galtisOutput(outArgs, GT_OK, "%d", -1);
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortManagerPortParametersSet function
* @endinternal
*
* @brief   Set the given attributes to a port. Those configurations will take place
*         during the port management. This API is applicable only when port is in
*         reset (PORT_MANAGER_STATE_RESET_E state).
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; AC3X.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssDxChPortManagerPortParametersSet
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
    CPSS_PM_PORT_PARAMS_STC portParams;
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;
    CPSS_PORT_SPEED_ENT             speed;
    CPSS_PORT_FEC_MODE_ENT          fecMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if (!portMgr) /* Port Manager was not enabled */
    {
        return CMD_AGENT_ERROR;
    }

    result = cpssDxChPortManagerPortParamsStructInit(CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E, &portParams);
    if(result != GT_OK)
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum          = (GT_U8)inArgs[0];
    portNum         = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    ifMode          = (CPSS_PORT_INTERFACE_MODE_ENT)inArgs[2];
    speed           = (CPSS_PORT_SPEED_ENT)inArgs[3];
    fecMode         = (CPSS_PORT_FEC_MODE_ENT)inArgs[4];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    portParams.portParamsType.regPort.ifMode = ifMode;
    portParams.portParamsType.regPort.speed  = speed;
    portParams.portParamsType.regPort.portAttributes.fecMode  = fecMode;
    CPSS_PM_SET_VALID_ATTR(&portParams, CPSS_PM_ATTR_FEC_MODE_E);

    /* call cpss api function */
    result = cpssDxChPortManagerPortParamsSet(devNum,portNum,&portParams);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPortManagerPortParametersGetFirst function
* @endinternal
*
* @brief   Get the attributes of a port. In case port was created, this API will return
*         all the current attributes of the port. In case the port was not created yet
*         this API will return all the attributes the port will be configured with upon
*         creation. This API is not applicable if port is in
*         CPSS_PORT_MANAGER_STATE_RESET_E state and previous call to
*         cpssDxChPortManagerPortParamsSet have not been made.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; AC3X.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssDxChPortManagerPortParametersGetFirst
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
    CPSS_PM_PORT_PARAMS_STC portParams;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if (!portMgr) /* Port Manager was not enabled */
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum          = (GT_U8)inArgs[0];
    portNum         = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    osMemSet(&portParams,0,sizeof(portParams));

    /* call cpss api function */
    result = cpssDxChPortManagerPortParamsGet(devNum,portNum,&portParams);

    inFields[0] = portParams.portParamsType.regPort.ifMode;
    inFields[1] = portParams.portParamsType.regPort.speed;

    /*osPrintf("\n GALTIS WRAPPER %d %d \n",inFields[0],inFields[1]);*/
    /*osPrintf("\n GALTIS WRAPPER #2 %d %d \n",portParams.portParams.port.portModeParams.modeMandatoryParams.ifMode,portParams.portParams.port.portModeParams.modeMandatoryParams.speed);*/
    fieldOutput("%d%d",
                inFields[0], inFields[1]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPortManagerPortParametersGetNext function
* @endinternal
*
* @brief   Get the attributes of a port. In case port was created, this API will return
*         all the current attributes of the port. In case the port was not created yet
*         this API will return all the attributes the port will be configured with upon
*         creation. This API is not applicable if port is in
*         CPSS_PORT_MANAGER_STATE_RESET_E state and previous call to
*         cpssDxChPortManagerPortParamsSet have not been made.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; AC3X.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssDxChPortManagerPortParametersGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8                           devNum;
    GT_PHYSICAL_PORT_NUM            portNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if (!portMgr) /* Port Manager was not enabled */
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* print for specified port only done at "GetFirst" call */
    galtisOutput(outArgs, GT_OK, "%d", -1);
    return CMD_OK;
}

/**
* @internal util_wrCpssDxChPortManagerStatusGet function
* @endinternal
*
* @brief   Get the status of all desired ports. This API will return (1) State of the port
*         within the port manager state machine (2) Whether or not port have been
*         disabled (using PORT_MANAGER_EVENT_DISABLE_E event) (3) Failure status.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; AC3X.
*
* @param[in] devNum                   - physical device number
*                                      portNum   - physical port number
*                                      portStatusPtr - (pointer to) a structure that contains current port status
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS  util_wrCpssDxChPortManagerStatusGet
(
    IN     GT_U8        devNum,
    INOUT  GT_PHYSICAL_PORT_NUM        *portNumPtr,
    IN     GT_BOOL      insertPortNumBeforeTab,
    IN     GT_BOOL      invokedForAllPorts,
    OUT    GT_UINTPTR   inFields[CMD_MAX_FIELDS],
    OUT    GT_8         outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_PHYSICAL_PORT_NUM            portNum;
    CPSS_PORT_MANAGER_STATUS_STC    portStageStatus;

    while (1)
    {
        portNum = *portNumPtr;

        if (portNum >= PRV_CPSS_MAX_PP_PORTS_NUM_CNS)
        {
            /* all ports alredy treated - end of table */
            galtisOutput(outArgs, GT_OK, "%d", -1);
            return GT_OK;
        }

        /* call cpss api function */
        result = cpssDxChPortManagerStatusGet(devNum,portNum,&portStageStatus);
        if (result == GT_OK)
        {
            /* success, increment port number for the next calls */
            (*portNumPtr) ++;
            break;
        }

        if (invokedForAllPorts == GT_FALSE)
        {
            /* error when invoked for particular port */
            galtisOutput(outArgs, result, "%d", -1);
            return result;
        }

        /* this port failed, try the next port */
        (*portNumPtr) ++;
    }

    inFields[0] = portNum;
    inFields[1] = GT_TRUE;/*portStageStatus.isPortUnderManagement;*/
    inFields[2] = portStageStatus.portState;
    inFields[3] = portStageStatus.portUnderOperDisable;
    inFields[4] = portStageStatus.failure;

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d",
            inFields[0],  inFields[1],
            inFields[2],  inFields[3], inFields[4]);

    galtisOutput(outArgs, result, "%f");

    return GT_OK;
}

/**
* @internal wrCpssDxChPortManagerStatusesTableGetFirst function
* @endinternal
*
* @brief   Get the status of the first port in system. This API will return (1) State of the port
*         within the port manager state machine (2) Whether or not port have been
*         disabled (using PORT_MANAGER_EVENT_DISABLE_E event) (3) Failure status.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; AC3X.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssDxChPortManagerStatusesTableGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8                           devNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if (!portMgr) /* Port Manager was not enabled */
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portForMacCountersGet = 0;

    util_wrCpssDxChPortManagerStatusGet(
        devNum, &portForMacCountersGet,
        GT_FALSE, GT_TRUE,
        inFields, outArgs);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPortManagerStatusesTableGetNext function
* @endinternal
*
* @brief   Get the status of the next port in iteration. This API will return (1) State of the port
*         within the port manager state machine (2) Whether or not port have been
*         disabled (using PORT_MANAGER_EVENT_DISABLE_E event) (3) Failure status.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; AC3X.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssDxChPortManagerStatusesTableGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8    devNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if (!portMgr) /* Port Manager was not enabled */
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    util_wrCpssDxChPortManagerStatusGet(
        devNum, &portForMacCountersGet,
        GT_FALSE, GT_TRUE,
        inFields, outArgs);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPortManagerInitPorts function
* @endinternal
*
* @brief   Init all ports under INIT_IN_PROGRESS_ state.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; AC3X.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssDxChPortManagerInitPorts
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_U32                   timeOut;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if (!portMgr) /* Port Manager was not enabled */
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum          = (GT_U8)inArgs[0];
    timeOut         = (GT_U8)inArgs[1];

    /* call cpss api function */
    result = prvCpssPortManagerDbgInitPorts(devNum, timeOut);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**
* @internal util_wrCpssDxChPortManagerConfigEntryGet function
* @endinternal
*
* @brief   Get the parameters of all desired ports. This API
*         will return (1) State of the port within the port
*         manager state machine (2) Whether or not port have
*         been disabled (using PORT_MANAGER_EVENT_DISABLE_E
*         event) (3) Failure status.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; AC3X.
*
* @param[in] devNum                   - physical device number
*                                      portNum   - physical port number
*                                      portStatusPtr - (pointer to) a structure that contains current port status
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS  util_wrCpssDxChPortManagerConfigEntryGet
(
    IN     GT_U8        devNum,
    INOUT  GT_PHYSICAL_PORT_NUM        *portNumPtr,
    IN     GT_BOOL      insertPortNumBeforeTab,
    IN     GT_BOOL      invokedForAllPorts,
    OUT    GT_UINTPTR   inFields[CMD_MAX_FIELDS],
    OUT    GT_8         outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_PHYSICAL_PORT_NUM            portNum;
    CPSS_PM_PORT_PARAMS_STC         portParams;
    GT_U32                          i, j; /* loop iterators */
    GT_U32                          laneNumOffset;

    while (1)
    {
        portNum = *portNumPtr;

        if (portNum >= PRV_CPSS_MAX_PP_PORTS_NUM_CNS)
        {
            /* all ports alredy treated - end of table */
            galtisOutput(outArgs, GT_OK, "%d", -1);
            return GT_OK;
        }

        /* call cpss api function */
        result = cpssDxChPortManagerPortParamsGet(devNum,portNum,&portParams);
        if (result == GT_OK)
        {
            /* success, increment port number for the next calls */
            (*portNumPtr) ++;
            break;
        }

        if (invokedForAllPorts == GT_FALSE)
        {
            /* error when invoked for particular port */
            galtisOutput(outArgs, result, "%d", -1);
            return result;
        }

        /* this port failed, try the next port */
        (*portNumPtr) ++;
    }

    inFields[0] = portNum;
    inFields[1] = GT_TRUE;/* enable*/
    inFields[2] = (portParams.portType == CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E ? GT_TRUE : GT_FALSE);

    inFields[3] = GT_TRUE; /* use CPSS defaults*/

    inFields[4] = portParams.portParamsType.apPort.apAttrs.fcAsmDir;
    inFields[5] = portParams.portParamsType.apPort.apAttrs.fcPause;
    inFields[6] = portParams.portParamsType.apPort.modesArr[0].fecRequested;
    inFields[7] = portParams.portParamsType.apPort.modesArr[0].fecSupported;

    laneNumOffset = 8;
    inFields[laneNumOffset] = portParams.portParamsType.apPort.apAttrs.negotiationLaneNum;
    inFields[laneNumOffset+1] = portParams.portParamsType.apPort.apAttrs.nonceDisable ;

     for(i = laneNumOffset+2, j = 0; i < (laneNumOffset+2)+2*CPSS_DXCH_PORT_AP_IF_ARRAY_SIZE_CNS; i+=4, j++)
    {
       inFields[i] =  portParams.portParamsType.apPort.modesArr[j].ifMode;

        if(CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E == portParams.portParamsType.apPort.modesArr[j].ifMode)
        {
            break;
        }
        inFields[i+1] =  portParams.portParamsType.apPort.modesArr[j].speed;
        inFields[i+2] =  portParams.portParamsType.apPort.modesArr[j].fecSupported ;
        inFields[i+3] =  portParams.portParamsType.apPort.modesArr[j].fecRequested;
    }

     /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
            inFields[0],  inFields[1],
            inFields[2],  inFields[3], inFields[4],
            inFields[5],  inFields[6], inFields[7],
            inFields[8],  inFields[9], inFields[10],
            inFields[11],  inFields[12], inFields[13],
            inFields[14],  inFields[15], inFields[16],
            inFields[17],  inFields[18], inFields[19],inFields[20],
            inFields[21],  inFields[22], inFields[23],
            inFields[24],  inFields[25], inFields[26],
            inFields[27],  inFields[28], inFields[29]
                );

    galtisOutput(outArgs, result, "%f");

    return GT_OK;
}


/**
* @internal wrCpssDxChPortManagerConfigEntryGetNext function
* @endinternal
*
* @brief   Get the parameters of the next port in iteration.
*         This API will return (1) State of the port within the
*         port manager state machine (2) Whether or not port
*         have been disabled (using PORT_MANAGER_EVENT_DISABLE_E
*         event) (3) Failure status.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; AC3X.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssDxChPortManagerConfigEntryGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8    devNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if (!portMgr) /* Port Manager was not enabled */
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    util_wrCpssDxChPortManagerConfigEntryGet(
        devNum, &portForMacCountersGet,
        GT_FALSE, GT_TRUE,
        inFields, outArgs);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPortManagerConfigEntryGetFirst function
* @endinternal
*
* @brief   Get the attributes of a port. In case port was created, this API will return
*         all the current attributes of the port. In case the port was not created yet
*         this API will return all the attributes the port will be configured with upon
*         creation. This API is not applicable if port is in
*         CPSS_PORT_MANAGER_STATE_RESET_E state and previous call to
*         cpssDxChPortManagerPortParamsSet have not been made.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; AC3X.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssDxChPortManagerConfigEntryGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8                   devNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if (!portMgr) /* Port Manager was not enabled */
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portForMacCountersGet = 0;

    util_wrCpssDxChPortManagerConfigEntryGet(
        devNum, &portForMacCountersGet,
        GT_FALSE, GT_TRUE,
        inFields, outArgs);

    return CMD_OK;

}


/**
* @internal wrCpssDxChPortManagerConfigEntrySet function
* @endinternal
*
* @brief   Enable/disable port manager on a port
*
* @note   APPLICABLE DEVICES:      Caelum; xCat3; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortManagerConfigEntrySet
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
    GT_BOOL                         enable;
    GT_BOOL                         useCpssDefault;
    CPSS_DXCH_PORT_AP_PARAMS_STC    apParams;
    GT_U32                          i, j; /* loop iterators */
    GT_U32                          laneNumOffset;
    CPSS_PORT_MANAGER_STC           portEventStc;
    CPSS_PM_PORT_PARAMS_STC         portParams;
    GT_BOOL                         apPort;
    GT_U32                          ii;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if (!portMgr) /* Port Manager was not enabled */
    {
        return CMD_AGENT_ERROR;
    }

    result = cpssDxChPortManagerPortParamsStructInit(CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E, &portParams);
    if(result != GT_OK)
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum      = (GT_U8)inArgs[0];
    portNum     = (GT_PHYSICAL_PORT_NUM)inFields[0];

    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    enable = (GT_BOOL)inFields[1];
    apPort = (GT_BOOL)inFields[2];

    if(GT_FALSE == enable)
    {
        /* to avoid compiler warnings */
        useCpssDefault = GT_FALSE;
        apParams.fcAsmDir = CPSS_DXCH_PORT_AP_FLOW_CONTROL_SYMMETRIC_E;
        apParams.fcPause = GT_FALSE;
        apParams.fecRequired = GT_FALSE;
        apParams.fecSupported = GT_FALSE;
        apParams.laneNum = 0;
        apParams.noneceDisable = GT_FALSE;
        goto config;
    }

    useCpssDefault = (GT_BOOL)inFields[3];

    apParams.fcAsmDir       = (GT_BOOL)inFields[4];
    apParams.fcPause        = (GT_BOOL)inFields[5];
    apParams.fecRequired    = (GT_BOOL)inFields[6];
    apParams.fecSupported   = (GT_BOOL)inFields[7];

    /*laneNumOffset = (29 == numFields) ? 7 : 9;*/
    laneNumOffset = 8;
    apParams.laneNum        = (GT_U32)inFields[laneNumOffset];
    apParams.noneceDisable  = (GT_U32)inFields[laneNumOffset+1];

    for(i = laneNumOffset+2, j = 0; i < (laneNumOffset+2)+2*CPSS_DXCH_PORT_AP_IF_ARRAY_SIZE_CNS; i+=4, j++)
    {
        apParams.modesAdvertiseArr[j].ifMode =
            (CPSS_PORT_INTERFACE_MODE_ENT)inFields[i];
        if(CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E == apParams.modesAdvertiseArr[j].ifMode)
        {
            break;
        }
        apParams.modesAdvertiseArr[j].speed = (CPSS_PORT_SPEED_ENT)inFields[i+1];
        apParams.fecAbilityArr[j] = (CPSS_PORT_FEC_MODE_ENT)inFields[i+2];
        apParams.fecRequestedArr[j] = (CPSS_PORT_FEC_MODE_ENT)inFields[i+3];
        /*if(numFields > 29)
        {
             actually we have no HW ability to define specific FEC advertisement for every interface
            apParams.fecAbilityArr[j] = (CPSS_DXCH_PORT_FEC_MODE_ENT)inFields[7];
            apParams.fecRequestedArr[j] = (CPSS_DXCH_PORT_FEC_MODE_ENT)inFields[8];
        }*/
    }

config:

    /*if(GT_TRUE == enable)
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
    }*/

    /* minimal non AP port parameters filling */
    if (apPort == GT_FALSE && enable==GT_TRUE)
    {
        portParams.portType = CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E;
        portParams.portParamsType.regPort.ifMode = apParams.modesAdvertiseArr[0].ifMode;
        portParams.portParamsType.regPort.speed = apParams.modesAdvertiseArr[0].speed;

        /* port params set */
        result = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);
        if(result != GT_OK)
        {
            cpssOsPrintf("cpssDxChPortManagerPortParamsSet(portNum=%d):rc=%d\n",
                            portNum, result);
            return result;
        }
    }
    else if (enable==GT_TRUE)
    {
        /* adv mode */
        for (ii=0;ii<CPSS_DXCH_PORT_AP_IF_ARRAY_SIZE_CNS;ii++)
        {
            portParams.portParamsType.apPort.modesArr[ii].ifMode = apParams.modesAdvertiseArr[ii].ifMode;
            if(CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E == apParams.modesAdvertiseArr[ii].ifMode)
            {
                break;
            }
            portParams.portParamsType.apPort.modesArr[ii].speed = apParams.modesAdvertiseArr[ii].speed;
            portParams.portParamsType.apPort.modesArr[ii].fecSupported = apParams.fecAbilityArr[ii];
            portParams.portParamsType.apPort.modesArr[ii].fecRequested = apParams.fecRequestedArr[ii];
        }

        if (GT_FALSE == useCpssDefault)
        {
          CPSS_PM_SET_VALID_AP_ATTR (&portParams, CPSS_PM_AP_PORT_ATTR_NONCE_E);
          portParams.portParamsType.apPort.apAttrs.nonceDisable = apParams.noneceDisable;

          CPSS_PM_SET_VALID_AP_ATTR (&portParams, CPSS_PM_AP_PORT_ATTR_FC_PAUSE_E);
          portParams.portParamsType.apPort.apAttrs.fcPause = apParams.fcPause;

          CPSS_PM_SET_VALID_AP_ATTR (&portParams, CPSS_PM_AP_PORT_ATTR_FC_ASM_DIR_E);
          portParams.portParamsType.apPort.apAttrs.fcAsmDir = apParams.fcAsmDir;

          CPSS_PM_SET_VALID_AP_ATTR (&portParams, CPSS_PM_AP_PORT_ATTR_LANE_NUM_E);
          portParams.portParamsType.apPort.apAttrs.negotiationLaneNum = apParams.laneNum;

          /*
           CPSS_PM_SET_VALID_AP_ATTR (&portParams, CPSS_PM_AP_PORT_ATTR_LANE_OVR_PARAMS_E);
           TODO: Q
           */
        }

        portParams.portType = CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E;
        portParams.portParamsType.apPort.numOfModes = ii;

        /* port params set */
        result = cpssDxChPortManagerPortParamsSet(devNum,portNum,&portParams);
        if(result != GT_OK)
        {
            cpssOsPrintf("cpssDxChPortManagerPortParamsSet(portNum=%d):rc=%d\n",
                            portNum, result);
            return result;
        }
    }

    /* creating the port */
    if (enable==GT_TRUE)
    {
        portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_CREATE_E;
    }
    else
    {
        portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_DELETE_E;
    }
    /* call cpss api function */
    result = cpssDxChPortManagerEventSet(devNum, portNum, &portEventStc);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortMangerEventEnaDisAllPorts function
* @endinternal
*
* @brief   Enable/disable all ports in port manager
*
* @note   APPLICABLE DEVICES:      Caelum; xCat3; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortMangerEventEnaDisAllPorts
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
    CPSS_PORT_MANAGER_STC       portEventStc;
    CPSS_PORT_MANAGER_EVENT_ENT portEventEnableDisable;
    GT_U32 portMacNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if (!portMgr) /* Port Manager was not enabled */
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum          = (GT_U8)inArgs[0];
    portEventEnableDisable       = (CPSS_PORT_MANAGER_EVENT_ENT)inArgs[1];

    result = GT_OK;

    if (portEventEnableDisable == 0)
    {
        portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_ENABLE_E;
    }
    else
    {
        portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_DISABLE_E;
    }

    for (portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
    {
        if ( wrPrvCpssDxChPortManagerGetPortMacNumber(devNum,portNum,&portMacNum) != GT_OK)
        {
            continue;
        }

        portMacNum = portMacNum;

        /* call cpss api function */
        result = cpssDxChPortManagerEventSet(devNum, portNum, &portEventStc );
        if (result!=GT_OK)
        {
            cpssOsPrintf("cpssDxChPortManagerEventSet(portNum=%d):rc=%d\n",
                            portNum, result);
        }
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPortManagerSetAllPorts function
* @endinternal
*
* @brief   Create all port in the system using port manager.
*
* @note   APPLICABLE DEVICES:      Caelum; xCat3; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortManagerSetAllPorts
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
    CPSS_PORT_MANAGER_STC   portEventStc;
    CPSS_PM_PORT_PARAMS_STC portParams;
    GT_BOOL                 automaticMode;
    CPSS_PORT_INTERFACE_MODE_ENT ifMode;
    CPSS_PORT_SPEED_ENT speed;
    GT_U32 portMacNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if (!portMgr) /* Port Manager was not enabled */
    {
        return CMD_AGENT_ERROR;
    }

    result = cpssDxChPortManagerPortParamsStructInit(CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E, &portParams);
    if(result != GT_OK)
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    ifMode  = (CPSS_PORT_INTERFACE_MODE_ENT)inArgs[1];
    speed   = (CPSS_PORT_SPEED_ENT)inArgs[2];
    automaticMode = (GT_BOOL)inArgs[3];

    result = GT_OK;

    /* First step, set port parameters on all ports */
    for (portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
    {
        if ( wrPrvCpssDxChPortManagerGetPortMacNumber(devNum,portNum,&portMacNum) != GT_OK)
        {
            continue;
        }

        portMacNum = portMacNum;

        portParams.portParamsType.regPort.ifMode = ifMode;
        portParams.portParamsType.regPort.speed  = speed;

        /* call cpss api function */
        result = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);
        if(result != GT_OK)
        {
            cpssOsPrintf("cpssDxChPortManagerParametersSet(portNum=%d):rc=%d\n",
                            portNum, result);
            /*return result;*/
        }
    }

    if (automaticMode==GT_FALSE)
    {
        /* disable event driven port manager */
        disableAutoPortMgr = GT_TRUE;
    }

    /* Next step, create all ports */
    portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_CREATE_E;

    for (portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
    {
        if ( wrPrvCpssDxChPortManagerGetPortMacNumber(devNum,portNum,&portMacNum) != GT_OK)
        {
            continue;
        }

        portMacNum = portMacNum;

        /* call cpss api function */
        result = cpssDxChPortManagerEventSet(devNum, portNum, &portEventStc );
        if(result != GT_OK)
        {
            cpssOsPrintf("cpssDxChPortManagerEventSet(portNum=%d):rc=%d\n",
                            portNum, result);
            /*return result;*/
        }
    }

    if (automaticMode==GT_FALSE)
    {
        /* Next step, perform notify low level events on all ports */
        portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_LOW_LEVEL_STATUS_CHANGED_E;

        for (portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
        {
            if ( wrPrvCpssDxChPortManagerGetPortMacNumber(devNum,portNum,&portMacNum) != GT_OK)
            {
                continue;
            }

            portMacNum = portMacNum;

            /* call cpss api function */
            result = cpssDxChPortManagerEventSet(devNum, portNum, &portEventStc );
            if(result != GT_OK)
            {
                cpssOsPrintf("cpssDxChPortManagerEventSet(portNum=%d):rc=%d\n",
                            portNum, result);
                /*return result;*/
            }
        }
    }

    /* Next step, perform init on all ports  */
    if (automaticMode==GT_FALSE)
    {
        result = prvCpssPortManagerDbgInitPorts(devNum,0);
    }

    result = GT_OK;

    /* enable event driven again */
    disableAutoPortMgr = GT_FALSE;

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPortManagerOverrideGlobalParameters
*           function
* @endinternal
*
* @brief   Function for override default global parameters.
*
* @note   APPLICABLE DEVICES:      Caelum; xCat3; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortManagerOverrideGlobalParameters
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_BOOL                 enableSDChangeOverride;

    /* struct that hold the global parameters to ovrride */
    /* for now there is only sd change to override, still want to make it dynamic */
    CPSS_PORT_MANAGER_GLOBAL_PARAMS_STC globalParamsStc;
    cpssOsMemSet(&globalParamsStc,0,sizeof(globalParamsStc));

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if (!portMgr) /* Port Manager was not enabled */
    {
        return CMD_AGENT_ERROR;
    }

    devNum  = (GT_U8)inArgs[0];
    enableSDChangeOverride = (GT_BOOL)inArgs[1];

    if (enableSDChangeOverride) /* sd change override enable */
    {
        globalParamsStc.globalParamsBitmapType = CPSS_PORT_MANAGER_GLOBAL_PARAMS_SIGNAL_DETECT_E;
        globalParamsStc.signalDetectDbCfg.sdChangeInterval = (GT_U32)inArgs[2];
        globalParamsStc.signalDetectDbCfg.sdChangeMinWindowSize = (GT_U32)inArgs[3];
        globalParamsStc.signalDetectDbCfg.sdCheckTimeExpired = (GT_U32)inArgs[4];
    }

    result = cpssDxChPortManagerGlobalParamsOverride(devNum, &globalParamsStc);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

static CMD_COMMAND dbCommands[] =
{
    /* Port manager set event on port */
    {"cpssDxChPortManagerEventSet",
         &wrCpssDxChPortManagerEvetSet,
         3, 0},

    {"cpssDxChPortManagerElectricalParamsSet",
        &wrCpssDxChPortManagerElectricalParamsSet,
        8,0},

    {"cpssDxChPortManagerMultiPortParamsSet",
        &wrCpssDxChPortManagerMultiPortParamsSet,
        11,0},

    {"cpssDxChPortManagerMultiPortsEventSet",
        &wrCpssDxChPortManagerMultiPortsEventSet,
        3,0},

    /* Port manager Get First port status */
    {"cpssDxChPortManagerStatusGetFirst",
         &wrCpssDxChPortManagerStatusGetFirst,
         2, 0},

    /* Port manager Get Next (dummy) status */
    {"cpssDxChPortManagerStatusGetNext",
         &wrCpssDxChPortManagerStatusGetNext,
         2, 0},

     /* Port manager set port attributes */
    {"cpssDxChPortManagerPortParametersSet",
         &wrCpssDxChPortManagerPortParametersSet,
         5, 0},

    /* Port manager Get First port attributes */
    {"cpssDxChPortManagerPortParametersGetFirst",
         &wrCpssDxChPortManagerPortParametersGetFirst,
         2, 0},

    /* Port manager Get Next (dummy) port attributes */
    {"cpssDxChPortManagerPortParametersGetNext",
         &wrCpssDxChPortManagerPortParametersGetNext,
         2, 0},

    /* Port manager Get all ports statuses [Table First] */
    {"cpssDxChPortManagerStatusesGetFirst",
         &wrCpssDxChPortManagerStatusesTableGetFirst,
         1, 0},

    /* Port manager Get all ports statuses [Table Next] */
    {"cpssDxChPortManagerStatusesGetNext",
         &wrCpssDxChPortManagerStatusesTableGetNext,
         1, 0},

    /* Port manager init port */
    {"cpssDxChPortManagerInitPorts",
         &wrCpssDxChPortManagerInitPorts,
         2, 0},

    /* config port manager ports by table */
    {"cpssDxChPortManagerConfigEntrySet",
        &wrCpssDxChPortManagerConfigEntrySet,
        1, 29},

    /* config port manager ports by table */
    {"cpssDxChPortManagerConfigEntryGetFirst",
        &wrCpssDxChPortManagerConfigEntryGetFirst,
        1, 0},

     /* config port manager ports by table */
    {"cpssDxChPortManagerConfigEntryGetNext",
        &wrCpssDxChPortManagerConfigEntryGetNext,
        1, 0},

    {"cpssDxChPortManagerSetAllPorts",
        &wrCpssDxChPortManagerSetAllPorts,
        4, 0},

    {"cpssDxChPortManagerEventEnaDisAllPorts",
        &wrCpssDxChPortMangerEventEnaDisAllPorts,
        2, 0},

    {"cpssDxChPortManagerOverrideGlobalParameters",
        &wrCpssDxChPortManagerOverrideGlobalParameters,
        5, 0},
};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssDxChPortManager function
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
GT_STATUS cmdLibInitCpssDxChPortManager
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}

