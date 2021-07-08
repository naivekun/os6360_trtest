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
* @file wrapCpssPxPortManager.c
*
* @brief Wrapper functions for port manager cpss.Px functions
*
* @version   103
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>

/* Feature specific includes. */
#include <cpss/px/port/cpssPxPortManager.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/port/cpssPxPortAp.h>

/* port used in status get tables */
static GT_PHYSICAL_PORT_NUM portForMacCountersGet = 0xFF;

extern GT_U32  disableAutoPortMgr;
extern GT_BOOL portMgr;

GT_STATUS wrPrvCpssPxPortManagerGetPortMacNumber(GT_U8 devNum, GT_PHYSICAL_PORT_NUM portNum, GT_U32 *portMacNum)
{
    GT_U32 tmpPortMacNum;

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, tmpPortMacNum);
    *portMacNum = tmpPortMacNum;

    return GT_OK;
}

/**
* @internal wrCpssPxPortManagerEvetSet function
* @endinternal
*
* @brief   Set the port according to the given event. This API performs actions based on
*         the given event and will transfer the port to a new state. For example, when port
*         is in PORT_MANAGER_STATE_RESET_E state and event PORT_MANAGER_EVENT_CREATE_E is called,
*         this API will perform port creation operations and upon success, port state will be
*         changed to PORT_MANAGER_STATE_LINK_DOWN_E state.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortManagerEvetSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_SW_DEV_NUM           devNum;
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
    devNum          = (GT_SW_DEV_NUM)inArgs[0];
    portNum         = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    portEvent       = (CPSS_PORT_MANAGER_EVENT_ENT)inArgs[2];

    portEventStc.portEvent = portEvent;

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxPortManagerEventSet(devNum, portNum, &portEventStc );

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortManagerStatusGetFirst function
* @endinternal
*
* @brief   Get the status of the port. This API will return (1) State of the port
*         within the port manager state machine (2) Whether or not port have been
*         disabled (using PORT_MANAGER_EVENT_DISABLE_E event) (3) Failure status.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortManagerStatusGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM                   devNum;
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
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_U8)inArgs[1];
    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    portForMacCountersGet = portNum;

    result = cpssPxPortManagerStatusGet(devNum, portNum, &portStageStatus);

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
* @internal wrCpssPxPortManagerStatusGetNext function
* @endinternal
*
* @brief   Get the status of the port. This API will return (1) State of the port
*         within the port manager state machine (2) Whether or not port have been
*         disabled (using PORT_MANAGER_EVENT_DISABLE_E event) (3) Failure status.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortManagerStatusGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM                   devNum;
    GT_PHYSICAL_PORT_NUM            portNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if (!portMgr) /* Port Manager was not enabled */
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* print for specified port only done at "GetFirst" call */
    galtisOutput(outArgs, GT_OK, "%d", -1);
    return CMD_OK;
}

/**
* @internal wrCpssPxPortManagerPortParametersSet function
* @endinternal
*
* @brief   Set the given attributes to a port. Those configurations will take place
*         during the port management. This API is applicable only when port is in
*         reset (PORT_MANAGER_STATE_RESET_E state).
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortManagerPortParametersSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_SW_DEV_NUM           devNum;
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

    /* map input arguments to locals */
    devNum          = (GT_SW_DEV_NUM)inArgs[0];
    portNum         = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    ifMode          = (CPSS_PORT_INTERFACE_MODE_ENT)inArgs[2];
    speed           = (CPSS_PORT_SPEED_ENT)inArgs[3];
    fecMode         = (CPSS_PORT_FEC_MODE_ENT)inArgs[4];

    result = cpssPxPortManagerPortParamsStructInit(CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E, &portParams);
    if(result != GT_OK)
    {
        return CMD_AGENT_ERROR;
    }

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    portParams.portParamsType.regPort.ifMode = ifMode;
    portParams.portParamsType.regPort.speed = speed;
    portParams.portParamsType.regPort.portAttributes.fecMode = fecMode;
    CPSS_PM_SET_VALID_ATTR (&portParams,CPSS_PM_ATTR_FEC_MODE_E);

    /* call cpss api function */
    result = cpssPxPortManagerPortParamsSet(devNum, portNum, &portParams);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**
* @internal wrCpssPxPortManagerPortParametersGetFirst function
* @endinternal
*
* @brief   Get the attributes of a port. In case port was created, this API will return
*         all the current attributes of the port. In case the port was not created yet
*         this API will return all the attributes the port will be configured with upon
*         creation. This API is not applicable if port is in
*         CPSS_PORT_MANAGER_STATE_RESET_E state and previous call to
*         cpssPxPortManagerPortParamsSet have not been made.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortManagerPortParametersGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_SW_DEV_NUM           devNum;
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
    devNum          = (GT_SW_DEV_NUM)inArgs[0];
    portNum         = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    osMemSet(&portParams,0,sizeof(portParams));

    /* call cpss api function */
    result = cpssPxPortManagerPortParamsGet(devNum, portNum, &portParams);

    inFields[0] = portParams.portParamsType.regPort.ifMode;
    inFields[1] = portParams.portParamsType.regPort.speed;

    /*osPrintf("\n GALTIS WRAPPER %d %d \n",inFields[0],inFields[1]);*/
    /*osPrintf("\n GALTIS WRAPPER #2 %d %d \n",portParams.portParamsType.regPort.ifMode,portParams.portParamsType.regPort.speed);*/
    fieldOutput("%d%d",
                inFields[0], inFields[1]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortManagerPortParametersGetNext function
* @endinternal
*
* @brief   Get the attributes of a port. In case port was created, this API will return
*         all the current attributes of the port. In case the port was not created yet
*         this API will return all the attributes the port will be configured with upon
*         creation. This API is not applicable if port is in
*         CPSS_PORT_MANAGER_STATE_RESET_E state and previous call to
*         cpssPxPortManagerPortParamsSet have not been made.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortManagerPortParametersGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM                   devNum;
    GT_PHYSICAL_PORT_NUM            portNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if (!portMgr) /* Port Manager was not enabled */
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* print for specified port only done at "GetFirst" call */
    galtisOutput(outArgs, GT_OK, "%d", -1);
    return CMD_OK;
}

/**
* @internal util_wrCpssPxPortManagerStatusGet function
* @endinternal
*
* @brief   Get the status of all desired ports. This API will return (1) State of the port
*         within the port manager state machine (2) Whether or not port have been
*         disabled (using PORT_MANAGER_EVENT_DISABLE_E event) (3) Failure status.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
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
static GT_STATUS  util_wrCpssPxPortManagerStatusGet
(
    IN     GT_SW_DEV_NUM          devNum,
    INOUT  GT_PHYSICAL_PORT_NUM   *portNumPtr,
    IN     GT_BOOL      insertPortNumBeforeTab,
    IN     GT_BOOL      invokedForAllPorts,
    OUT    GT_UINTPTR   inFields[CMD_MAX_FIELDS],
    OUT    GT_8         outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_PHYSICAL_PORT_NUM            portNum;
    CPSS_PORT_MANAGER_STATUS_STC    portStageStatus;

    if (!portMgr) /* Port Manager was not enabled */
    {
        return CMD_AGENT_ERROR;
    }

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
        result = cpssPxPortManagerStatusGet(devNum, portNum, &portStageStatus);
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
* @internal wrCpssPxPortManagerStatusesTableGetFirst function
* @endinternal
*
* @brief   Get the status of the first port in system. This API will return (1) State of the port
*         within the port manager state machine (2) Whether or not port have been
*         disabled (using PORT_MANAGER_EVENT_DISABLE_E event) (3) Failure status.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortManagerStatusesTableGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM            devNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if (!portMgr) /* Port Manager was not enabled */
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portForMacCountersGet = 0;

    util_wrCpssPxPortManagerStatusGet(
        devNum, &portForMacCountersGet,
        GT_FALSE, GT_TRUE,
        inFields, outArgs);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortManagerStatusesTableGetNext function
* @endinternal
*
* @brief   Get the status of the next port in iteration. This API will return (1) State of the port
*         within the port manager state machine (2) Whether or not port have been
*         disabled (using PORT_MANAGER_EVENT_DISABLE_E event) (3) Failure status.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortManagerStatusesTableGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM    devNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if (!portMgr) /* Port Manager was not enabled */
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    util_wrCpssPxPortManagerStatusGet(
        devNum, &portForMacCountersGet,
        GT_FALSE, GT_TRUE,
        inFields, outArgs);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortManagerInitPorts function
* @endinternal
*
* @brief   Init all ports under INIT_IN_PROGRESS_ state.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortManagerInitPorts
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_SW_DEV_NUM           devNum;
    GT_U32                   timeOut;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if (!portMgr) /* Port Manager was not enabled */
    {
        return CMD_AGENT_ERROR;
    }

    /* map input arguments to locals */
    devNum          = (GT_SW_DEV_NUM)inArgs[0];
    timeOut         = (GT_U8)inArgs[1];

    /* call cpss api function */
    result = prvCpssPortManagerDbgInitPorts(devNum, timeOut);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}
/* TBD - PX AP */

/**
* @internal util_wrCpssPxPortManagerConfigEntryGet function
* @endinternal
*
* @brief  Get the parameters of all desired ports. This API will
*         return (1) State of the port within the port manager
*         state machine (2) Whether or not port have been
*         disabled (using PORT_MANAGER_EVENT_DISABLE_E event)
*         (3) Failure status.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
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
static GT_STATUS  util_wrCpssPxPortManagerConfigEntryGet
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
        result = cpssPxPortManagerPortParamsGet(devNum,portNum,&portParams);
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

     for(i = laneNumOffset+2, j = 0; i < (laneNumOffset+2)+2*10; i+=4, j++)
    {
       inFields[i] =  portParams.portParamsType.apPort.modesArr[j].ifMode;
       if(CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E == portParams.portParamsType.apPort.modesArr[j].ifMode)
       {
           break;
       }
       inFields[i+1] =  portParams.portParamsType.apPort.modesArr[j].speed;
       inFields[i+2] =  portParams.portParamsType.apPort.modesArr[j].fecSupported;
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
* @internal wrCpssPxPortManagerConfigEntryGetNext function
* @endinternal
*
* @brief   Get the parameters of the next port in iteration. 
*         This API will return (1) State of the port within the
*         port manager state machine (2) Whether or not port
*         have been disabled (using PORT_MANAGER_EVENT_DISABLE_E
*         event) (3) Failure status.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortManagerConfigEntryGetNext
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

    util_wrCpssPxPortManagerConfigEntryGet(
        devNum, &portForMacCountersGet,
        GT_FALSE, GT_TRUE,
        inFields, outArgs);

    return CMD_OK;
}

/**
* @internal wrCpssPxPortManagerConfigEntryGetFirst function
* @endinternal
*
* @brief   Get the attributes of a port. In case port was created, this API will return
*         all the current attributes of the port. In case the port was not created yet
*         this API will return all the attributes the port will be configured with upon
*         creation. This API is not applicable if port is in
*         CPSS_PORT_MANAGER_STATE_RESET_E state and previous call to
*         cpssDxChPortManagerPortParamsSet have not been made.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortManagerConfigEntryGetFirst
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

    util_wrCpssPxPortManagerConfigEntryGet(
        devNum, &portForMacCountersGet,
        GT_FALSE, GT_TRUE,
        inFields, outArgs);

    return CMD_OK;

}

/**
* @internal wrCpssPxPortManagerConfigEntrySet function
* @endinternal
*
* @brief   Enable/disable port manager on a port
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortManagerConfigEntrySet
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
    CPSS_PX_PORT_AP_PARAMS_STC      apParams;
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

    result = cpssPxPortManagerPortParamsStructInit(CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E, &portParams);
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
        apParams.fcAsmDir = CPSS_PX_PORT_AP_FLOW_CONTROL_SYMMETRIC_E;
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

    for(i = laneNumOffset+2, j = 0; i < (laneNumOffset+2)+2*CPSS_PX_PORT_AP_IF_ARRAY_SIZE_CNS; i+=4, j++)
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
            apParams.fecAbilityArr[j] = (CPSS_PORT_FEC_MODE_ENT)inFields[7];
            apParams.fecRequestedArr[j] = (CPSS_PORT_FEC_MODE_ENT)inFields[8];
        }*/
    }

config:

    /*if(GT_TRUE == enable)
    {
        result = cpssPxPortInterfaceSpeedSupportGet(devNum, portNum,
                                                  CPSS_PORT_INTERFACE_MODE_XGMII_E,
                                                  CPSS_PORT_SPEED_10000_E,
                                                  &supported);
        if(result != GT_OK)
        {
            cpssOsPrintf("cpssPxPortInterfaceSpeedSupportGet(portNum=%d,XGMII):rc=%d\n",
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
        result = cpssPxPortManagerPortParamsSet(devNum,portNum,&portParams);
        if(result != GT_OK)
        {
            cpssOsPrintf("cpssPxPortManagerPortParamsSet(portNum=%d):rc=%d\n",
                            portNum, result);
            return result;
        }
    }
    else if (enable==GT_TRUE)
    {
        /* adv mode */
        for (ii=0;ii<CPSS_PX_PORT_AP_IF_ARRAY_SIZE_CNS;ii++)
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
        portParams.portParamsType.apPort.numOfModes = ii;

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

        /* port params set */
        result = cpssPxPortManagerPortParamsSet(devNum, portNum, &portParams);
        if(result != GT_OK)
        {
            cpssOsPrintf("cpssPxPortManagerPortParamsSet(portNum=%d):rc=%d\n",
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
    result = cpssPxPortManagerEventSet(devNum, portNum, &portEventStc);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssPxPortMangerEventEnaDisAllPorts function
* @endinternal
*
* @brief   Enable/disable all ports in port manager
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortMangerEventEnaDisAllPorts
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
        if ( wrPrvCpssPxPortManagerGetPortMacNumber(devNum,portNum,&portMacNum) != GT_OK)
        {
            continue;
        }

        portMacNum = portMacNum;

        /* call cpss api function */
        result = cpssPxPortManagerEventSet(devNum, portNum, &portEventStc );
        if (result!=GT_OK)
        {
            cpssOsPrintf("cpssPxPortManagerEventSet(portNum=%d):rc=%d\n",
                            portNum, result);
        }
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortManagerSetAllPorts function
* @endinternal
*
* @brief   Create all port in the system using port manager.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortManagerSetAllPorts
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

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    ifMode  = (CPSS_PORT_INTERFACE_MODE_ENT)inArgs[1];
    speed   = (CPSS_PORT_SPEED_ENT)inArgs[2];
    automaticMode = (GT_BOOL)inArgs[3];

    result = cpssPxPortManagerPortParamsStructInit(CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E, &portParams);
    if(result != GT_OK)
    {
        return CMD_AGENT_ERROR;
    }

    result = GT_OK;

    /* First step, set port parameters on all ports */
    for (portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
    {
        if ( wrPrvCpssPxPortManagerGetPortMacNumber(devNum,portNum,&portMacNum) != GT_OK)
        {
            continue;
        }

        portMacNum = portMacNum;

        portParams.portParamsType.regPort.ifMode = ifMode;
        portParams.portParamsType.regPort.speed = speed;

        /* call cpss api function */
        result = cpssPxPortManagerPortParamsSet(devNum, portNum, &portParams );
        if(result != GT_OK)
        {
            cpssOsPrintf("cpssPxPortManagerParametersSet(portNum=%d):rc=%d\n",
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
        if ( wrPrvCpssPxPortManagerGetPortMacNumber(devNum,portNum,&portMacNum) != GT_OK)
        {
            continue;
        }

        portMacNum = portMacNum;

        /* call cpss api function */
        result = cpssPxPortManagerEventSet(devNum, portNum, &portEventStc );
        if(result != GT_OK)
        {
            cpssOsPrintf("cpssPxPortManagerEventSet(portNum=%d):rc=%d\n",
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
            if ( wrPrvCpssPxPortManagerGetPortMacNumber(devNum,portNum,&portMacNum) != GT_OK)
            {
                continue;
            }

            portMacNum = portMacNum;

            /* call cpss api function */
            result = cpssPxPortManagerEventSet(devNum, portNum, &portEventStc );
            if(result != GT_OK)
            {
                cpssOsPrintf("cpssPxPortManagerEventSet(portNum=%d):rc=%d\n",
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
* @internal wrCpssPxPortManagerOverrideGlobalParameters
*           function
* @endinternal
*
* @brief   Function for override default global parameters.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortManagerOverrideGlobalParameters
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

    result = cpssPxPortManagerGlobalParamsOverride(devNum, &globalParamsStc);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

static CMD_COMMAND dbCommands[] =
{
    /* Port manager set event on port */
    {"cpssPxPortManagerEventSet",
         &wrCpssPxPortManagerEvetSet,
         3, 0},

    /* Port manager Get First port status */
    {"cpssPxPortManagerStatusGetFirst",
         &wrCpssPxPortManagerStatusGetFirst,
         2, 0},

    /* Port manager Get Next (dummy) status */
    {"cpssPxPortManagerStatusGetNext",
         &wrCpssPxPortManagerStatusGetNext,
         2, 0},

     /* Port manager set port attributes */
    {"cpssPxPortManagerPortParametersSet",
         &wrCpssPxPortManagerPortParametersSet,
         5, 0},

    /* Port manager Get First port attributes */
    {"cpssPxPortManagerPortParametersGetFirst",
         &wrCpssPxPortManagerPortParametersGetFirst,
         2, 0},

    /* Port manager Get Next (dummy) port attributes */
    {"cpssPxPortManagerPortParametersGetNext",
         &wrCpssPxPortManagerPortParametersGetNext,
         2, 0},

    /* Port manager Get all ports statuses [Table First] */
    {"cpssPxPortManagerStatusesGetFirst",
         &wrCpssPxPortManagerStatusesTableGetFirst,
         1, 0},

    /* Port manager Get all ports statuses [Table Next] */
    {"cpssPxPortManagerStatusesGetNext",
         &wrCpssPxPortManagerStatusesTableGetNext,
         1, 0},

    /* Port manager init port */
    {"cpssPxPortManagerInitPorts",
         &wrCpssPxPortManagerInitPorts,
         2, 0},
    /* config port manager ports by table */
    {"cpssPxPortManagerConfigEntrySet",
        &wrCpssPxPortManagerConfigEntrySet,
        1, 29},

    /* config port manager ports by table */
    {"cpssPxPortManagerConfigEntryGetFirst",
        &wrCpssPxPortManagerConfigEntryGetFirst,
        1, 0},

     /* config port manager ports by table */
    {"cpssPxPortManagerConfigEntryGetNext",
        &wrCpssPxPortManagerConfigEntryGetNext,
        1, 0},

    {"cpssPxPortManagerSetAllPorts",
        &wrCpssPxPortManagerSetAllPorts,
        4, 0},

    {"cpssPxPortManagerEventEnaDisAllPorts",
        &wrCpssPxPortMangerEventEnaDisAllPorts,
        2, 0},

    {"cpssPxPortManagerOverrideGlobalParameters",
        &wrCpssPxPortManagerOverrideGlobalParameters,
        5, 0},
};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssPxPortManager function
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
GT_STATUS cmdLibInitCpssPxPortManager
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}

