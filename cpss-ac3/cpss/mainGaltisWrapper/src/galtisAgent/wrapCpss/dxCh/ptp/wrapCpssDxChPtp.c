/*******************************************************************************
*              Copyright 2001, GALILEO TECHNOLOGY, LTD.
*
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL. NO RIGHTS ARE GRANTED
* HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT OF MARVELL OR ANY THIRD
* PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE DISCRETION TO REQUEST THAT THIS
* CODE BE IMMEDIATELY RETURNED TO MARVELL. THIS CODE IS PROVIDED "AS IS".
* MARVELL MAKES NO WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS
* ACCURACY, COMPLETENESS OR PERFORMANCE. MARVELL COMPRISES MARVELL TECHNOLOGY
* GROUP LTD. (MTGL) AND ITS SUBSIDIARIES, MARVELL INTERNATIONAL LTD. (MIL),
* MARVELL TECHNOLOGY, INC. (MTI), MARVELL SEMICONDUCTOR, INC. (MSI), MARVELL
* ASIA PTE LTD. (MAPL), MARVELL JAPAN K.K. (MJKK), GALILEO TECHNOLOGY LTD. (GTL)
* AND GALILEO TECHNOLOGY, INC. (GTI).
********************************************************************************
*/
/**
********************************************************************************
* @file wrapCpssDxChPtp.c
*
* @brief Wrapper functions for PTP cpss.dxCh functions
*
* @version   18
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>


/* Feature specific includes. */
#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/ptp/cpssDxChPtp.h>
#include <cpss/dxCh/dxChxGen/ptp/private/prvCpssDxChPtp.h>

/* support for multi port groups */

/**
* @internal ptpMultiPortGroupsBmpGet function
* @endinternal
*
* @brief   Get the portGroupsBmp for multi port groups device.
*         when 'enabled' --> wrappers will use the APIs
*         with portGroupsBmp parameter
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)enable / disable the use of APIs with portGroupsBmp parameter.
* @param[out] portGroupsBmpPtr         - (pointer to)port groups bmp , relevant only when enable = GT_TRUE
*                                       NONE
*/
static void ptpMultiPortGroupsBmpGet
(
    IN   GT_U8               devNum,
    OUT  GT_BOOL             *enablePtr,
    OUT  GT_PORT_GROUPS_BMP  *portGroupsBmpPtr
)
{
    /* default */
    *enablePtr  = GT_FALSE;
    *portGroupsBmpPtr = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    if (0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        return;
    }

    utilMultiPortGroupsBmpGet(devNum, enablePtr, portGroupsBmpPtr);
}


/* Port Group aqnd Regular version wrapper     */
/* description see in original function header */

static GT_STATUS pg_wrap_cpssDxChPtpTodCounterFunctionSet
(
    IN GT_U8                                    devNum,
    IN CPSS_PORT_DIRECTION_ENT                  direction,
    IN CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT       function
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */
    CPSS_DXCH_PTP_TAI_ID_STC   taiId =
        {CPSS_DXCH_PTP_TAI_INSTANCE_GLOBAL_E, CPSS_DXCH_PTP_TAI_NUMBER_0_E, 0};

    ptpMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return  cpssDxChPtpTodCounterFunctionSet(devNum, direction, &taiId, function);
    }
    else
    {
        return  cpssDxChPtpPortGroupTodCounterFunctionSet(devNum, pgBmp,
                                                          direction, function);
    }
}


static GT_STATUS pg_wrap_cpssDxChPtpTodCounterFunctionGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_PORT_DIRECTION_ENT                 direction,
    OUT CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT      *functionPtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */
    CPSS_DXCH_PTP_TAI_ID_STC   taiId =
        {CPSS_DXCH_PTP_TAI_INSTANCE_GLOBAL_E, CPSS_DXCH_PTP_TAI_NUMBER_0_E, 0};

    ptpMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return  cpssDxChPtpTodCounterFunctionGet(devNum, direction, &taiId, functionPtr);
    }
    else
    {
        return  cpssDxChPtpPortGroupTodCounterFunctionGet(devNum, pgBmp,
                                                          direction, functionPtr);
    }
}


static GT_STATUS pg_wrap_cpssDxChPtpTodCounterFunctionTriggerSet
(
    IN GT_U8                    devNum,
    IN CPSS_PORT_DIRECTION_ENT  direction
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */
    CPSS_DXCH_PTP_TAI_ID_STC   taiId =
        {CPSS_DXCH_PTP_TAI_INSTANCE_GLOBAL_E, CPSS_DXCH_PTP_TAI_NUMBER_0_E, 0};

    ptpMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return  cpssDxChPtpTodCounterFunctionTriggerSet(devNum, direction, &taiId);
    }
    else
    {
        return  cpssDxChPtpPortGroupTodCounterFunctionTriggerSet(devNum, pgBmp,
                                                                 direction);
    }
}


static GT_STATUS pg_wrap_cpssDxChPtpTodCounterFunctionTriggerGet
(
    IN GT_U8                    devNum,
    IN CPSS_PORT_DIRECTION_ENT  direction,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */
    CPSS_DXCH_PTP_TAI_ID_STC   taiId =
        {CPSS_DXCH_PTP_TAI_INSTANCE_GLOBAL_E, CPSS_DXCH_PTP_TAI_NUMBER_0_E, 0};

    ptpMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return  cpssDxChPtpTodCounterFunctionTriggerGet(devNum, direction,
                                                        &taiId, enablePtr);
    }
    else
    {
        return  cpssDxChPtpPortGroupTodCounterFunctionTriggerGet(devNum, pgBmp,
                                                                 direction,
                                                                 enablePtr);
    }
}


static GT_STATUS pg_wrap_cpssDxChPtpTodCounterShadowSet
(
    IN GT_U8                        devNum,
    IN CPSS_PORT_DIRECTION_ENT      direction,
    IN CPSS_DXCH_PTP_TOD_COUNT_STC  *todCounterPtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    ptpMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return  cpssDxChPtpTodCounterShadowSet(devNum, direction, todCounterPtr);
    }
    else
    {
        return  cpssDxChPtpPortGroupTodCounterShadowSet(devNum, pgBmp, direction,
                                                        todCounterPtr);
    }
}


static GT_STATUS pg_wrap_cpssDxChPtpTodCounterShadowGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_PORT_DIRECTION_ENT         direction,
    OUT CPSS_DXCH_PTP_TOD_COUNT_STC     *todCounterPtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    ptpMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return  cpssDxChPtpTodCounterShadowGet(devNum, direction, todCounterPtr);
    }
    else
    {
        return  cpssDxChPtpPortGroupTodCounterShadowGet(devNum, pgBmp, direction,
                                                        todCounterPtr);
    }
}

static GT_STATUS pg_wrap_cpssDxChPtpTodCounterGet
(
    IN GT_U8                            devNum,
    IN CPSS_PORT_DIRECTION_ENT          direction,
    OUT CPSS_DXCH_PTP_TOD_COUNT_STC     *todCounterPtr
)
{

    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    ptpMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return  cpssDxChPtpTodCounterGet(devNum, direction, todCounterPtr);
    }
    else
    {
        return  cpssDxChPtpPortGroupTodCounterGet(devNum, pgBmp, direction,
                                                  todCounterPtr);
    }
}

/**
* @internal wrprvCpssDxChPtpTaiDebugInstanceSet function
* @endinternal
*
* @brief  Sets the TAI debug instance.
*
* @note   APPLICABLE DEVICES:      Falcon.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or etherTypeIndex
* @retval GT_OUT_OF_RANGE          - on wrong etherType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrprvCpssDxChPtpTaiDebugInstanceSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_U32      tileId;
    GT_U32      instanceId;
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_TYPE_ENT taiInstanceType;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_U8)inArgs[0];
    tileId          = (GT_U32)inArgs[1];
    taiInstanceType = (PRV_CPSS_DXCH_PTP_TAI_INSTANCE_TYPE_ENT)inArgs[2];
    instanceId      = (GT_U32)inArgs[3];

    /* call cpss api function */
    result = prvCpssDxChPtpTaiDebugInstanceSet(devNum, tileId, taiInstanceType, instanceId);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpEtherTypeSet function
* @endinternal
*
* @brief   Configure ethertype0/ethertype1 of PTP over Ethernet packets.
*         A packet is identified as PTP over Ethernet if its EtherType matches
*         one of the configured values.
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or etherTypeIndex
* @retval GT_OUT_OF_RANGE          - on wrong etherType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpEtherTypeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_U32      etherTypeIndex;
    GT_U32      etherType;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    etherTypeIndex = (GT_U32)inArgs[1];
    etherType = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChPtpEtherTypeSet(devNum, etherTypeIndex, etherType);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpEtherTypeGet function
* @endinternal
*
* @brief   Get ethertypes of PTP over Ethernet packets.
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or etherTypeIndex
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpEtherTypeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_U32      etherTypeIndex;
    GT_U32      etherType;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    etherTypeIndex = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPtpEtherTypeGet(devNum, etherTypeIndex, &etherType);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", etherType);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpUdpDestPortsSet function
* @endinternal
*
* @brief   Configure UDP destination port0/port1 of PTP over UDP packets.
*         A packet is identified as PTP over UDP if it is a UDP packet, whose
*         destination port matches one of the configured ports.
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or udpPortIndex
* @retval GT_OUT_OF_RANGE          - on wrong udpPortNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpUdpDestPortsSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_U32      udpPortIndex;
    GT_U32      udpPortNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    udpPortIndex = (GT_U32)inArgs[1];
    udpPortNum = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChPtpUdpDestPortsSet(devNum, udpPortIndex, udpPortNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpUdpDestPortsGet function
* @endinternal
*
* @brief   Get UDP destination port0/port1 of PTP over UDP packets.
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or udpPortIndex
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpUdpDestPortsGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_U32      udpPortIndex;
    GT_U32      udpPortNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    udpPortIndex = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPtpUdpDestPortsGet(devNum, udpPortIndex, &udpPortNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", udpPortNum);

    return CMD_OK;
}


/**
* @internal wrCpssDxChPtpMessageTypeCmdSet function
* @endinternal
*
* @brief   Configure packet command per PTP message type.
*         The message type is extracted from the PTP header.
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum, messageType or command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpMessageTypeCmdSet
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
    GT_U32                  messageType;
    CPSS_PACKET_CMD_ENT     command;
    GT_U32                  domainNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    domainNum = (GT_U32)inArgs[2];
    messageType = (GT_U32)inArgs[3];
    command = (CPSS_PACKET_CMD_ENT)inArgs[4];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPtpMessageTypeCmdSet(
        devNum, portNum, domainNum, messageType, command);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpMessageTypeCmdGet function
* @endinternal
*
* @brief   Get packet command per PTP message type.
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or messageType
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on bad value found in HW
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpMessageTypeCmdGet
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
    GT_U32                  messageType;
    CPSS_PACKET_CMD_ENT     command;
    GT_U32                  domainNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    domainNum = (GT_U32)inArgs[2];
    messageType = (GT_U32)inArgs[3];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPtpMessageTypeCmdGet(
        devNum, portNum, domainNum, messageType, &command);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", command);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpCpuCodeBaseSet function
* @endinternal
*
* @brief   Set CPU code base assigned to trapped and mirrored PTP packets.
*         A PTP packet's CPU code is computed as base CPU code + messageType
*         (where message type is taken from PTP header).
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or cpuCode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpCpuCodeBaseSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_U8                       devNum;
    CPSS_NET_RX_CPU_CODE_ENT    cpuCode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    cpuCode = (CPSS_NET_RX_CPU_CODE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPtpCpuCodeBaseSet(devNum, cpuCode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpCpuCodeBaseGet function
* @endinternal
*
* @brief   Get CPU code base assigned to trapped and mirrored PTP packets.
*         A PTP packet's CPU code is computed as base CPU code + messageType
*         (where message type is taken from PTP header).
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpCpuCodeBaseGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_U8                       devNum;
    CPSS_NET_RX_CPU_CODE_ENT    cpuCode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPtpCpuCodeBaseGet(devNum, &cpuCode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", cpuCode);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTodCounterFunctionSet function
* @endinternal
*
* @brief   The function configures type of TOD counter action that will be
*         performed once triggered by cpssDxChPtpTodCounterFunctionTriggerSet()
*         API or by pulse from an external interface.
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or direction
*                                       or function
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTodCounterFunctionSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_PORT_DIRECTION_ENT             direction;
    CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT  function;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    direction = (CPSS_PORT_DIRECTION_ENT)inArgs[1];
    function = (CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT)inArgs[2];

    /* call cpss api function */
    result = pg_wrap_cpssDxChPtpTodCounterFunctionSet(devNum, direction,
                                                      function);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTodCounterFunctionGet function
* @endinternal
*
* @brief   Get type of TOD counter action that will be performed once triggered by
*         cpssDxChPtpTodCounterFunctionTriggerSet() API or by pulse from an
*         external interface.
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or direction
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on bad value found in HW
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTodCounterFunctionGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_PORT_DIRECTION_ENT             direction;
    CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT  function;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    direction = (CPSS_PORT_DIRECTION_ENT)inArgs[1];

    /* call cpss api function */
    result = pg_wrap_cpssDxChPtpTodCounterFunctionGet(devNum, direction, &function);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", function);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTodCounterFunctionTriggerSet function
* @endinternal
*
* @brief   Trigger TOD (Time of Day) counter function accorging to the function set
*         by cpssDxChPtpTodCounterFunctionSet().
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - when previous TOD triggered action is not finished yet
* @retval GT_BAD_PARAM             - on wrong devNum or direction
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTodCounterFunctionTriggerSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_U8                       devNum;
    CPSS_PORT_DIRECTION_ENT     direction;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    direction = (CPSS_PORT_DIRECTION_ENT)inArgs[1];

    /* call cpss api function */
    result = pg_wrap_cpssDxChPtpTodCounterFunctionTriggerSet(devNum, direction);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTodCounterFunctionTriggerGet function
* @endinternal
*
* @brief   Get status of trigger TOD (Time of Day) counter function.
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or direction
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTodCounterFunctionTriggerGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_U8                       devNum;
    CPSS_PORT_DIRECTION_ENT     direction;
    GT_BOOL                     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    direction = (CPSS_PORT_DIRECTION_ENT)inArgs[1];

    /* call cpss api function */
    result = pg_wrap_cpssDxChPtpTodCounterFunctionTriggerGet(devNum, direction,
                                                             &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTodCounterShadowSet function
* @endinternal
*
* @brief   Configure TOD timestamping shadow counter values.
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or direction
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTodCounterShadowSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_U8                       devNum;
    CPSS_PORT_DIRECTION_ENT     direction;
    CPSS_DXCH_PTP_TOD_COUNT_STC todCounter;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    direction = (CPSS_PORT_DIRECTION_ENT)inArgs[1];
    todCounter.nanoSeconds = (GT_U32)inArgs[2];
    todCounter.seconds.l[0] = (GT_U32)inArgs[3];
    todCounter.seconds.l[1] = (GT_U32)inArgs[4];

    /* call cpss api function */
    result = pg_wrap_cpssDxChPtpTodCounterShadowSet(devNum, direction,
                                                    &todCounter);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTodCounterShadowGet function
* @endinternal
*
* @brief   Get TOD timestamping shadow counter values.
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or direction
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTodCounterShadowGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_U8                       devNum;
    CPSS_PORT_DIRECTION_ENT     direction;
    CPSS_DXCH_PTP_TOD_COUNT_STC todCounter;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    direction = (CPSS_PORT_DIRECTION_ENT)inArgs[1];

    /* call cpss api function */
    result = pg_wrap_cpssDxChPtpTodCounterShadowGet(devNum, direction,
                                                    &todCounter);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%X%X%X", todCounter.nanoSeconds,
                 todCounter.seconds.l[0], todCounter.seconds.l[1]);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTodCounterGet function
* @endinternal
*
* @brief   Get TOD timestamping counter value.
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or direction
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTodCounterGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_U8                       devNum;
    CPSS_PORT_DIRECTION_ENT     direction;
    CPSS_DXCH_PTP_TOD_COUNT_STC todCounter;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    direction = (CPSS_PORT_DIRECTION_ENT)inArgs[1];

    /* call cpss api function */
    result = pg_wrap_cpssDxChPtpTodCounterGet(devNum, direction, &todCounter);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%X%X%X", todCounter.nanoSeconds,
                 todCounter.seconds.l[0], todCounter.seconds.l[1]);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpPortTimeStampEnableSet function
* @endinternal
*
* @brief   Enable/Disable port for PTP timestamping.
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum, direction or portNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpPortTimeStampEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_U8                       devNum;
    CPSS_PORT_DIRECTION_ENT     direction;
    GT_PHYSICAL_PORT_NUM                       portNum;
    GT_BOOL                     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    direction = (CPSS_PORT_DIRECTION_ENT)inArgs[1];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[2];
    enable = (GT_BOOL)inArgs[3];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPtpPortTimeStampEnableSet(devNum, direction, portNum,
                                               enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpPortTimeStampEnableGet function
* @endinternal
*
* @brief   Get port PTP timestamping status (enabled/disabled).
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum, direction or portNum.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpPortTimeStampEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_U8                       devNum;
    CPSS_PORT_DIRECTION_ENT     direction;
    GT_PHYSICAL_PORT_NUM                       portNum;
    GT_BOOL                     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    direction = (CPSS_PORT_DIRECTION_ENT)inArgs[1];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPtpPortTimeStampEnableGet(devNum, direction, portNum,
                                               &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}


/**
* @internal wrCpssDxChPtpEthernetTimeStampEnableSet function
* @endinternal
*
* @brief   Enable/Disable PTP Timestamping over Ethernet packets.
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or direction
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpEthernetTimeStampEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_U8                       devNum;
    CPSS_PORT_DIRECTION_ENT     direction;
    GT_BOOL                     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    direction = (CPSS_PORT_DIRECTION_ENT)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* call cpss api function */
    result = cpssDxChPtpEthernetTimeStampEnableSet(devNum, direction, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**
* @internal wrCpssDxChPtpEthernetTimeStampEnableGet function
* @endinternal
*
* @brief   Get PTP Timestamping status over Ethernet packets (enable/disabled).
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or direction
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpEthernetTimeStampEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_U8                       devNum;
    CPSS_PORT_DIRECTION_ENT     direction;
    GT_BOOL                     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    direction = (CPSS_PORT_DIRECTION_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPtpEthernetTimeStampEnableGet(devNum, direction, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpUdpTimeStampEnableSet function
* @endinternal
*
* @brief   Enable/Disable PTP Timestamping over UDP packets.
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum, protocolStack or direction
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpUdpTimeStampEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_U8                       devNum;
    CPSS_PORT_DIRECTION_ENT     direction;
    CPSS_IP_PROTOCOL_STACK_ENT  protocolStack;
    GT_BOOL                     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    direction = (CPSS_PORT_DIRECTION_ENT)inArgs[1];
    protocolStack = (CPSS_IP_PROTOCOL_STACK_ENT)inArgs[2];
    enable = (GT_BOOL)inArgs[3];

    /* call cpss api function */
    result = cpssDxChPtpUdpTimeStampEnableSet(devNum, direction, protocolStack,
                                              enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**
* @internal wrCpssDxChPtpUdpTimeStampEnableGet function
* @endinternal
*
* @brief   Get PTP Timestamping status over UDP packets (enable/disabled).
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum, protocolStack or direction
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpUdpTimeStampEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_U8                       devNum;
    CPSS_PORT_DIRECTION_ENT     direction;
    CPSS_IP_PROTOCOL_STACK_ENT  protocolStack;
    GT_BOOL                     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    direction = (CPSS_PORT_DIRECTION_ENT)inArgs[1];
    protocolStack = (CPSS_IP_PROTOCOL_STACK_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssDxChPtpUdpTimeStampEnableGet(devNum, direction, protocolStack,
                                              &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpMessageTypeTimeStampEnableSet function
* @endinternal
*
* @brief   Enable/Disable PTP Timestamping for specific message type of the packet.
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum, messageType or direction
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpMessageTypeTimeStampEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_U8                       devNum;
    CPSS_PORT_DIRECTION_ENT     direction;
    GT_U32                      messageType;
    GT_BOOL                     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    direction = (CPSS_PORT_DIRECTION_ENT)inArgs[1];
    messageType = (GT_U32)inArgs[2];
    enable = (GT_BOOL)inArgs[3];

    /* call cpss api function */
    result = cpssDxChPtpMessageTypeTimeStampEnableSet(devNum, direction,
                                                      messageType, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpMessageTypeTimeStampEnableGet function
* @endinternal
*
* @brief   Get PTP Timestamping status for specific message type of the packet
*         (enable/disabled).
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum, messageType or direction
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpMessageTypeTimeStampEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_U8                       devNum;
    CPSS_PORT_DIRECTION_ENT     direction;
    GT_U32                      messageType;
    GT_BOOL                     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    direction = (CPSS_PORT_DIRECTION_ENT)inArgs[1];
    messageType = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChPtpMessageTypeTimeStampEnableGet(devNum, direction,
                                                      messageType, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTransportSpecificCheckEnableSet function
* @endinternal
*
* @brief   Enable/Disable PTP Timestamping check of the packet's transport specific value.
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or direction
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTransportSpecificCheckEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_U8                       devNum;
    CPSS_PORT_DIRECTION_ENT     direction;
    GT_BOOL                     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    direction = (CPSS_PORT_DIRECTION_ENT)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* call cpss api function */
    result = cpssDxChPtpTransportSpecificCheckEnableSet(devNum, direction,
                                                        enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTransportSpecificCheckEnableGet function
* @endinternal
*
* @brief   Get status of PTP Timestamping check of the packet's transport specific
*         value (the check is enabled/disabled).
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or direction
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTransportSpecificCheckEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_U8                       devNum;
    CPSS_PORT_DIRECTION_ENT     direction;
    GT_BOOL                     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    direction = (CPSS_PORT_DIRECTION_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPtpTransportSpecificCheckEnableGet(devNum, direction,
                                                        &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTransportSpecificTimeStampEnableSet function
* @endinternal
*
* @brief   Enable PTP Timestamping for transport specific value of the packet.
*         Note: Relevant only if global configuration determines whether the
*         transport specific field should be checked by
*         cpssDxChPtpTransportSpecificCheckEnableSet().
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum, transportSpecificVal or
*                                       direction
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTransportSpecificTimeStampEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_U8                       devNum;
    CPSS_PORT_DIRECTION_ENT     direction;
    GT_U32                      transportSpecificVal;
    GT_BOOL                     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    direction = (CPSS_PORT_DIRECTION_ENT)inArgs[1];
    transportSpecificVal = (GT_U32)inArgs[2];
    enable = (GT_BOOL)inArgs[3];

    /* call cpss api function */
    result = cpssDxChPtpTransportSpecificTimeStampEnableSet(devNum, direction,
                                                            transportSpecificVal,
                                                            enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTransportSpecificTimeStampEnableGet function
* @endinternal
*
* @brief   Get PTP Timestamping status for specific message type of the packet
*         (enable/disabled).
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum, transportSpecificVal or
*                                       direction
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTransportSpecificTimeStampEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_U8                       devNum;
    CPSS_PORT_DIRECTION_ENT     direction;
    GT_U32                      transportSpecificVal;
    GT_BOOL                     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    direction = (CPSS_PORT_DIRECTION_ENT)inArgs[1];
    transportSpecificVal = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChPtpTransportSpecificTimeStampEnableGet(devNum, direction,
                                                            transportSpecificVal,
                                                            &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;

}

/**
* @internal wrCpssDxChPtpTimestampEntryGet function
* @endinternal
*
* @brief   Read current Timestamp entry from the timestamp FIFO queue.
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - the action succeeded and there are no more waiting
*                                       Timestamp entries
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or direction
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTimestampEntryGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_PORT_DIRECTION_ENT             direction;
    CPSS_DXCH_PTP_TIMESTAMP_ENTRY_STC   entry;
    GT_HW_DEV_NUM               __HwDev;  /* Dummy for converting. */
    GT_PORT_NUM                 __Port; /* Dummy for converting. */

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    direction = (CPSS_PORT_DIRECTION_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPtpTimestampEntryGet(devNum, direction, &entry);

    /* need to remove casting after changing the devNum and portNum type*/
    CPSS_TBD_BOOKMARK_EARCH

    __HwDev = (GT_U32)devNum;
    __Port = (GT_U32)entry.portNum;
    /* convert port number */
    CONVERT_BACK_DEV_PORT_DATA_MAC(__HwDev,__Port);
    devNum = (GT_U8)__HwDev;
    entry.portNum = (GT_U8)__Port;

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d %d %d %d", entry.portNum, entry.messageType,
                 entry.sequenceId, entry.timeStampVal);

    return CMD_OK;
}


/*============== SIP5 new API =============*/
/**
* @internal wrCpssDxChPtpTodCounterFunctionSet_Tai function
* @endinternal
*
* @brief   The function configures type of TOD counter action that will be
*         performed once triggered by cpssDxChPtpTodCounterFunctionTriggerSet()
*         API or by pulse from an external interface.
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or direction
*                                       or function
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTodCounterFunctionSet_Tai
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_PORT_DIRECTION_ENT             direction;
    CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT  function;
    CPSS_DXCH_PTP_TAI_ID_STC            taiId;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    direction = (CPSS_PORT_DIRECTION_ENT)inArgs[1];
    taiId.taiInstance = (CPSS_DXCH_PTP_TAI_INSTANCE_ENT)inArgs[2];
    taiId.taiNumber   = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[3];
    taiId.portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[4];
    function = (CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT)inArgs[5];

    /* call cpss api function */
    result = cpssDxChPtpTodCounterFunctionSet(
        devNum, direction, &taiId, function);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTodCounterFunctionGet_Tai function
* @endinternal
*
* @brief   Get type of TOD counter action that will be performed once triggered by
*         cpssDxChPtpTodCounterFunctionTriggerSet() API or by pulse from an
*         external interface.
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or direction
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on bad value found in HW
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTodCounterFunctionGet_Tai
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_PORT_DIRECTION_ENT             direction;
    CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT  function;
    CPSS_DXCH_PTP_TAI_ID_STC            taiId;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    direction = (CPSS_PORT_DIRECTION_ENT)inArgs[1];
    taiId.taiInstance = (CPSS_DXCH_PTP_TAI_INSTANCE_ENT)inArgs[2];
    taiId.taiNumber   = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[3];
    taiId.portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[4];

    /* call cpss api function */
    result = cpssDxChPtpTodCounterFunctionGet(
        devNum, direction, &taiId, &function);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", function);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTodCounterFunctionTriggerSet_Tai function
* @endinternal
*
* @brief   Trigger TOD (Time of Day) counter function accorging to the function set
*         by cpssDxChPtpTodCounterFunctionSet().
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - when previous TOD triggered action is not finished yet
* @retval GT_BAD_PARAM             - on wrong devNum or direction
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTodCounterFunctionTriggerSet_Tai
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_U8                       devNum;
    CPSS_PORT_DIRECTION_ENT     direction;
    CPSS_DXCH_PTP_TAI_ID_STC    taiId;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    direction = (CPSS_PORT_DIRECTION_ENT)inArgs[1];
    taiId.taiInstance = (CPSS_DXCH_PTP_TAI_INSTANCE_ENT)inArgs[2];
    taiId.taiNumber   = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[3];
    taiId.portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[4];

    /* call cpss api function */
    result = cpssDxChPtpTodCounterFunctionTriggerSet(
        devNum, direction, &taiId);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTodCounterFunctionTriggerGet_Tai function
* @endinternal
*
* @brief   Get status of trigger TOD (Time of Day) counter function.
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or direction
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTodCounterFunctionTriggerGet_Tai
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_U8                       devNum;
    CPSS_PORT_DIRECTION_ENT     direction;
    GT_BOOL                     enable;
    CPSS_DXCH_PTP_TAI_ID_STC    taiId;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    direction = (CPSS_PORT_DIRECTION_ENT)inArgs[1];
    taiId.taiInstance = (CPSS_DXCH_PTP_TAI_INSTANCE_ENT)inArgs[2];
    taiId.taiNumber   = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[3];
    taiId.portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[4];

    /* call cpss api function */
    result = cpssDxChPtpTodCounterFunctionTriggerGet(
        devNum, direction, &taiId, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTaiClockModeSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Clock Mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTaiClockModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_PTP_TAI_ID_STC            taiId;
    CPSS_DXCH_PTP_TAI_CLOCK_MODE_ENT    clockMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    taiId.taiInstance = (CPSS_DXCH_PTP_TAI_INSTANCE_ENT)inArgs[1];
    taiId.taiNumber   = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[2];
    taiId.portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[3];
    clockMode         = (CPSS_DXCH_PTP_TAI_CLOCK_MODE_ENT)inArgs[4];

    /* call cpss api function */
    result = cpssDxChPtpTaiClockModeSet(
        devNum, &taiId, clockMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTaiClockModeGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Clock Mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTaiClockModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_PTP_TAI_ID_STC            taiId;
    CPSS_DXCH_PTP_TAI_CLOCK_MODE_ENT    clockMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    taiId.taiInstance = (CPSS_DXCH_PTP_TAI_INSTANCE_ENT)inArgs[1];
    taiId.taiNumber   = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[2];
    taiId.portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[3];

    /* call cpss api function */
    result = cpssDxChPtpTaiClockModeGet(
        devNum, &taiId, &clockMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", clockMode);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTaiInternalClockGenerateEnableSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Internal Clock Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTaiInternalClockGenerateEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_PTP_TAI_ID_STC            taiId;
    GT_BOOL                             enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    taiId.taiInstance = (CPSS_DXCH_PTP_TAI_INSTANCE_ENT)inArgs[1];
    taiId.taiNumber   = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[2];
    taiId.portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[3];
    enable            = (GT_BOOL)inArgs[4];

    /* call cpss api function */
    result = cpssDxChPtpTaiInternalClockGenerateEnableSet(
        devNum, &taiId, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTaiInternalClockGenerateEnableGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Internal Clock Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTaiInternalClockGenerateEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_PTP_TAI_ID_STC            taiId;
    GT_BOOL                             enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    taiId.taiInstance = (CPSS_DXCH_PTP_TAI_INSTANCE_ENT)inArgs[1];
    taiId.taiNumber   = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[2];
    taiId.portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[3];

    /* call cpss api function */
    result = cpssDxChPtpTaiInternalClockGenerateEnableGet(
        devNum, &taiId, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTaiPtpPClockDriftAdjustEnableSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Ptp Clock Adjust Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTaiPtpPClockDriftAdjustEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_PTP_TAI_ID_STC            taiId;
    GT_BOOL                             enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    taiId.taiInstance = (CPSS_DXCH_PTP_TAI_INSTANCE_ENT)inArgs[1];
    taiId.taiNumber   = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[2];
    taiId.portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[3];
    enable            = (GT_BOOL)inArgs[4];

    /* call cpss api function */
    result = cpssDxChPtpTaiPtpPClockDriftAdjustEnableSet(
        devNum, &taiId, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTaiPtpPClockDriftAdjustEnableGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Ptp Clock Adjust Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTaiPtpPClockDriftAdjustEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_PTP_TAI_ID_STC            taiId;
    GT_BOOL                             enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    taiId.taiInstance = (CPSS_DXCH_PTP_TAI_INSTANCE_ENT)inArgs[1];
    taiId.taiNumber   = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[2];
    taiId.portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[3];

    /* call cpss api function */
    result = cpssDxChPtpTaiPtpPClockDriftAdjustEnableGet(
        devNum, &taiId, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTaiCaptureOverrideEnableSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Capture Override Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTaiCaptureOverrideEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_PTP_TAI_ID_STC            taiId;
    GT_BOOL                             enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    taiId.taiInstance = (CPSS_DXCH_PTP_TAI_INSTANCE_ENT)inArgs[1];
    taiId.taiNumber   = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[2];
    taiId.portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[3];
    enable            = (GT_BOOL)inArgs[4];

    /* call cpss api function */
    result = cpssDxChPtpTaiCaptureOverrideEnableSet(
        devNum, &taiId, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTaiCaptureOverrideEnableGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Capture Override Enable
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTaiCaptureOverrideEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_PTP_TAI_ID_STC            taiId;
    GT_BOOL                             enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    taiId.taiInstance = (CPSS_DXCH_PTP_TAI_INSTANCE_ENT)inArgs[1];
    taiId.taiNumber   = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[2];
    taiId.portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[3];

    /* call cpss api function */
    result = cpssDxChPtpTaiCaptureOverrideEnableGet(
        devNum, &taiId, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTaiInputTriggersCountEnableSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Count Input Triggers Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTaiInputTriggersCountEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_PTP_TAI_ID_STC            taiId;
    GT_BOOL                             enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    taiId.taiInstance = (CPSS_DXCH_PTP_TAI_INSTANCE_ENT)inArgs[1];
    taiId.taiNumber   = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[2];
    taiId.portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[3];
    enable            = (GT_BOOL)inArgs[4];

    /* call cpss api function */
    result = cpssDxChPtpTaiInputTriggersCountEnableSet(
        devNum, &taiId, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTaiInputTriggersCountEnableGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Count Input Triggers Enable
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTaiInputTriggersCountEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_PTP_TAI_ID_STC            taiId;
    GT_BOOL                             enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    taiId.taiInstance = (CPSS_DXCH_PTP_TAI_INSTANCE_ENT)inArgs[1];
    taiId.taiNumber   = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[2];
    taiId.portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[3];

    /* call cpss api function */
    result = cpssDxChPtpTaiInputTriggersCountEnableGet(
        devNum, &taiId, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTaiExternalPulseWidthSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) External Pulse Width.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on bad extPulseWidth
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTaiExternalPulseWidthSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_PTP_TAI_ID_STC            taiId;
    GT_U32                              extPulseWidth;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    taiId.taiInstance = (CPSS_DXCH_PTP_TAI_INSTANCE_ENT)inArgs[1];
    taiId.taiNumber   = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[2];
    taiId.portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[3];
    extPulseWidth     = (GT_U32)inArgs[4];

    /* call cpss api function */
    result = cpssDxChPtpTaiExternalPulseWidthSet(
        devNum, &taiId, extPulseWidth);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTaiExternalPulseWidthGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) External Pulse Width
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTaiExternalPulseWidthGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_PTP_TAI_ID_STC            taiId;
    GT_U32                              extPulseWidth;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    taiId.taiInstance = (CPSS_DXCH_PTP_TAI_INSTANCE_ENT)inArgs[1];
    taiId.taiNumber   = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[2];
    taiId.portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[3];

    /* call cpss api function */
    result = cpssDxChPtpTaiExternalPulseWidthGet(
        devNum, &taiId, &extPulseWidth);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", extPulseWidth);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTaiTodSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) TOD values.
*         The TOD will be updated by triggering an appropriate function.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API does not activate any triggers, other APIs does it
*
*/
static CMD_STATUS wrCpssDxChPtpTaiTodSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_PTP_TAI_ID_STC            taiId;
    CPSS_DXCH_PTP_TAI_TOD_TYPE_ENT      todValueType;
    CPSS_DXCH_PTP_TOD_COUNT_STC         todValue;
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                    = (GT_U8)inArgs[0];
    taiId.taiInstance         = (CPSS_DXCH_PTP_TAI_INSTANCE_ENT)inArgs[1];
    taiId.taiNumber           = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[2];
    taiId.portNum             = (GT_PHYSICAL_PORT_NUM)inArgs[3];
    todValueType              = (CPSS_DXCH_PTP_TAI_TOD_TYPE_ENT)inArgs[4];
    todValue.seconds.l[0]     = (GT_U32)inArgs[5];
    todValue.seconds.l[1]     = (GT_U32)inArgs[6];
    todValue.nanoSeconds      = (GT_U32)inArgs[7];
    todValue.fracNanoSeconds  = (GT_U32)inArgs[8];

    /* call cpss api function */
    result = cpssDxChPtpTaiTodSet(
        devNum, &taiId, todValueType, &todValue);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTaiTodGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) TOD values.
*         The TOD was captured by triggering an appropriate function.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTaiTodGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_PTP_TAI_ID_STC            taiId;
    CPSS_DXCH_PTP_TAI_TOD_TYPE_ENT      todValueType;
    CPSS_DXCH_PTP_TOD_COUNT_STC         todValue;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum            = (GT_U8)inArgs[0];
    taiId.taiInstance = (CPSS_DXCH_PTP_TAI_INSTANCE_ENT)inArgs[1];
    taiId.taiNumber   = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[2];
    taiId.portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[3];
    todValueType      = (CPSS_DXCH_PTP_TAI_TOD_TYPE_ENT)inArgs[4];

    /* call cpss api function */
    result = cpssDxChPtpTaiTodGet(
        devNum, &taiId, todValueType, &todValue);

    /* pack output arguments to galtis string */
    galtisOutput(
        outArgs, result, "%d%d%d%d",
        todValue.seconds.l[0], todValue.seconds.l[1],
        todValue.nanoSeconds, todValue.fracNanoSeconds);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTaiOutputTriggerEnableSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Output Trigger Generation Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note use cpssDxChPtpTaiTodSet with
*       CPSS_DXCH_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_E and
*       the time to trigger generation.
*
*/
static CMD_STATUS wrCpssDxChPtpTaiOutputTriggerEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_PTP_TAI_ID_STC            taiId;
    GT_BOOL                             enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    taiId.taiInstance = (CPSS_DXCH_PTP_TAI_INSTANCE_ENT)inArgs[1];
    taiId.taiNumber   = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[2];
    taiId.portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[3];
    enable            = (GT_BOOL)inArgs[4];

    /* call cpss api function */
    result = cpssDxChPtpTaiOutputTriggerEnableSet(
        devNum, &taiId, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTaiOutputTriggerEnableGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Output Trigger Generation Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTaiOutputTriggerEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_PTP_TAI_ID_STC            taiId;
    GT_BOOL                             enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    taiId.taiInstance = (CPSS_DXCH_PTP_TAI_INSTANCE_ENT)inArgs[1];
    taiId.taiNumber   = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[2];
    taiId.portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[3];

    /* call cpss api function */
    result = cpssDxChPtpTaiOutputTriggerEnableGet(
        devNum, &taiId, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTaiTodStepSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) TOD Step.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTaiTodStepSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_PTP_TAI_ID_STC            taiId;
    CPSS_DXCH_PTP_TAI_TOD_STEP_STC      todStep;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                    = (GT_U8)inArgs[0];
    taiId.taiInstance         = (CPSS_DXCH_PTP_TAI_INSTANCE_ENT)inArgs[1];
    taiId.taiNumber           = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[2];
    taiId.portNum             = (GT_PHYSICAL_PORT_NUM)inArgs[3];
    todStep.nanoSeconds       = (GT_U32)inArgs[4];
    todStep.fracNanoSeconds   = (GT_U32)inArgs[5];

    /* call cpss api function */
    result = cpssDxChPtpTaiTodStepSet(
        devNum, &taiId, &todStep);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTaiTodStepGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) TOD Step.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTaiTodStepGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_PTP_TAI_ID_STC            taiId;
    CPSS_DXCH_PTP_TAI_TOD_STEP_STC      todStep;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum            = (GT_U8)inArgs[0];
    taiId.taiInstance = (CPSS_DXCH_PTP_TAI_INSTANCE_ENT)inArgs[1];
    taiId.taiNumber   = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[2];
    taiId.portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[3];

    /* call cpss api function */
    result = cpssDxChPtpTaiTodStepGet(
        devNum, &taiId, &todStep);

    /* pack output arguments to galtis string */
    galtisOutput(
        outArgs, result, "%d%d",
        todStep.nanoSeconds, todStep.fracNanoSeconds);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTaiPulseDelaySet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) External Trigger Propagation Delay.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTaiPulseDelaySet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_PTP_TAI_ID_STC            taiId;
    GT_U32                              pulseDelay;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    taiId.taiInstance = (CPSS_DXCH_PTP_TAI_INSTANCE_ENT)inArgs[1];
    taiId.taiNumber   = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[2];
    taiId.portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[3];
    pulseDelay        = (GT_U32)inArgs[4];

    /* call cpss api function */
    result = cpssDxChPtpTaiPulseDelaySet(
        devNum, &taiId, pulseDelay);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTaiPulseDelayGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) External Trigger Propagation Delay.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTaiPulseDelayGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_PTP_TAI_ID_STC            taiId;
    GT_U32                              pulseDelay;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    taiId.taiInstance = (CPSS_DXCH_PTP_TAI_INSTANCE_ENT)inArgs[1];
    taiId.taiNumber   = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[2];
    taiId.portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[3];

    /* call cpss api function */
    result = cpssDxChPtpTaiPulseDelayGet(
        devNum, &taiId, &pulseDelay);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", pulseDelay);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTaiClockDelaySet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) External Clock Propagation Delay.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTaiClockDelaySet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_PTP_TAI_ID_STC            taiId;
    GT_U32                              clockDelay;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    taiId.taiInstance = (CPSS_DXCH_PTP_TAI_INSTANCE_ENT)inArgs[1];
    taiId.taiNumber   = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[2];
    taiId.portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[3];
    clockDelay        = (GT_U32)inArgs[4];

    /* call cpss api function */
    result = cpssDxChPtpTaiClockDelaySet(
        devNum, &taiId, clockDelay);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTaiClockDelayGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) External Clock Propagation Delay.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTaiClockDelayGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_PTP_TAI_ID_STC            taiId;
    GT_U32                              clockDelay;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    taiId.taiInstance = (CPSS_DXCH_PTP_TAI_INSTANCE_ENT)inArgs[1];
    taiId.taiNumber   = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[2];
    taiId.portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[3];

    /* call cpss api function */
    result = cpssDxChPtpTaiClockDelayGet(
        devNum, &taiId, &clockDelay);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", clockDelay);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTaiFractionalNanosecondDriftSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Fractional Nanosecond Drift.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTaiFractionalNanosecondDriftSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_PTP_TAI_ID_STC            taiId;
    GT_32                               fracNanoDrift;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    taiId.taiInstance = (CPSS_DXCH_PTP_TAI_INSTANCE_ENT)inArgs[1];
    taiId.taiNumber   = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[2];
    taiId.portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[3];
    fracNanoDrift     = (GT_32)inArgs[4];

    /* call cpss api function */
    result = cpssDxChPtpTaiFractionalNanosecondDriftSet(
        devNum, &taiId, fracNanoDrift);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTaiFractionalNanosecondDriftGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Fractional Nanosecond Drift.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTaiFractionalNanosecondDriftGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_PTP_TAI_ID_STC            taiId;
    GT_32                               fracNanoDrift;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    taiId.taiInstance = (CPSS_DXCH_PTP_TAI_INSTANCE_ENT)inArgs[1];
    taiId.taiNumber   = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[2];
    taiId.portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[3];

    /* call cpss api function */
    result = cpssDxChPtpTaiFractionalNanosecondDriftGet(
        devNum, &taiId, &fracNanoDrift);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", fracNanoDrift);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTaiPClockCycleSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) PClock Cycle.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTaiPClockCycleSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_PTP_TAI_ID_STC            taiId;
    GT_U32                              pClockCycle;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    taiId.taiInstance = (CPSS_DXCH_PTP_TAI_INSTANCE_ENT)inArgs[1];
    taiId.taiNumber   = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[2];
    taiId.portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[3];
    pClockCycle       = (GT_U32)inArgs[4];

    /* call cpss api function */
    result = cpssDxChPtpTaiPClockCycleSet(
        devNum, &taiId, pClockCycle);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTaiPClockCycleGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) PClock Cycle.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTaiPClockCycleGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_PTP_TAI_ID_STC            taiId;
    GT_U32                              pClockCycle;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    taiId.taiInstance = (CPSS_DXCH_PTP_TAI_INSTANCE_ENT)inArgs[1];
    taiId.taiNumber   = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[2];
    taiId.portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[3];

    /* call cpss api function */
    result = cpssDxChPtpTaiPClockCycleGet(
        devNum, &taiId, &pClockCycle);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", pClockCycle);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTaiClockCycleSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Clock Cycle.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTaiClockCycleSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_PTP_TAI_ID_STC            taiId;
    GT_U32                              clockCycleSec;
    GT_U32                              clockCycleNano;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    taiId.taiInstance = (CPSS_DXCH_PTP_TAI_INSTANCE_ENT)inArgs[1];
    taiId.taiNumber   = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[2];
    taiId.portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[3];
    clockCycleSec     = (GT_U32)inArgs[4];
    clockCycleNano    = (GT_U32)inArgs[5];

    /* call cpss api function */
    result = cpssDxChPtpTaiClockCycleSet(
        devNum, &taiId, clockCycleSec, clockCycleNano);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTaiClockCycleGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Clock Cycle.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTaiClockCycleGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_PTP_TAI_ID_STC            taiId;
    GT_U32                              clockCycleSec;
    GT_U32                              clockCycleNano;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    taiId.taiInstance = (CPSS_DXCH_PTP_TAI_INSTANCE_ENT)inArgs[1];
    taiId.taiNumber   = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[2];
    taiId.portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[3];

    /* call cpss api function */
    result = cpssDxChPtpTaiClockCycleGet(
        devNum, &taiId, &clockCycleSec, &clockCycleNano);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", clockCycleSec, clockCycleNano);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTaiTodCaptureStatusSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) TOD Capture Status.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTaiTodCaptureStatusSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_PTP_TAI_ID_STC            taiId;
    GT_U32                              captureIndex;
    GT_BOOL                             valid;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    taiId.taiInstance = (CPSS_DXCH_PTP_TAI_INSTANCE_ENT)inArgs[1];
    taiId.taiNumber   = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[2];
    taiId.portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[3];
    captureIndex      = (GT_U32)inArgs[4];
    valid             = (GT_BOOL)inArgs[5];

    /* call cpss api function */
    result = cpssDxChPtpTaiTodCaptureStatusSet(
        devNum, &taiId, captureIndex, valid);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTaiTodCaptureStatusGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) TOD Capture Status.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTaiTodCaptureStatusGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_PTP_TAI_ID_STC            taiId;
    GT_U32                              captureIndex;
    GT_BOOL                             valid;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    taiId.taiInstance = (CPSS_DXCH_PTP_TAI_INSTANCE_ENT)inArgs[1];
    taiId.taiNumber   = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[2];
    taiId.portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[3];
    captureIndex      = (GT_U32)inArgs[4];

    /* call cpss api function */
    result = cpssDxChPtpTaiTodCaptureStatusGet(
        devNum, &taiId, captureIndex, &valid);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", valid);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTaiTodUpdateCounterGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) TOD Update Counter.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Counter is Clear On Read. Value incremented at each
*       TOD Update/Increment/Decrement function.
*
*/
static CMD_STATUS wrCpssDxChPtpTaiTodUpdateCounterGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_PTP_TAI_ID_STC            taiId;
    GT_U32                              todUpdCnt;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    taiId.taiInstance = (CPSS_DXCH_PTP_TAI_INSTANCE_ENT)inArgs[1];
    taiId.taiNumber   = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[2];
    taiId.portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[3];

    /* call cpss api function */
    result = cpssDxChPtpTaiTodUpdateCounterGet(
        devNum, &taiId, &todUpdCnt);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", todUpdCnt);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTaiIncomingTriggerCounterSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Incoming Trigger Counter.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTaiIncomingTriggerCounterSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_PTP_TAI_ID_STC            taiId;
    GT_U32                              inTrigCnt;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    taiId.taiInstance = (CPSS_DXCH_PTP_TAI_INSTANCE_ENT)inArgs[1];
    taiId.taiNumber   = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[2];
    taiId.portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[3];
    inTrigCnt         = (GT_U32)inArgs[4];

    /* call cpss api function */
    result = cpssDxChPtpTaiIncomingTriggerCounterSet(
        devNum, &taiId, inTrigCnt);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTaiIncomingTriggerCounterGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Incoming Trigger Counter.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTaiIncomingTriggerCounterGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_PTP_TAI_ID_STC            taiId;
    GT_U32                              inTrigCnt;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    taiId.taiInstance = (CPSS_DXCH_PTP_TAI_INSTANCE_ENT)inArgs[1];
    taiId.taiNumber   = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[2];
    taiId.portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[3];

    /* call cpss api function */
    result = cpssDxChPtpTaiIncomingTriggerCounterGet(
        devNum, &taiId, &inTrigCnt);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", inTrigCnt);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTaiIncomingClockCounterSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Incoming Clock Counter.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTaiIncomingClockCounterSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_PTP_TAI_ID_STC            taiId;
    GT_BOOL                             enable;
    GT_U32                              inClockCnt;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    taiId.taiInstance = (CPSS_DXCH_PTP_TAI_INSTANCE_ENT)inArgs[1];
    taiId.taiNumber   = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[2];
    taiId.portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[3];
    enable            = (GT_BOOL)inArgs[4];
    inClockCnt        = (GT_U32)inArgs[5];

    /* call cpss api function */
    result = cpssDxChPtpTaiIncomingClockCounterSet(
        devNum, &taiId, enable, inClockCnt);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTaiIncomingClockCounterGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Incoming Clock Counter.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTaiIncomingClockCounterGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_PTP_TAI_ID_STC            taiId;
    GT_BOOL                             enable;
    GT_U32                              inClockCnt;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    taiId.taiInstance = (CPSS_DXCH_PTP_TAI_INSTANCE_ENT)inArgs[1];
    taiId.taiNumber   = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[2];
    taiId.portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[3];

    /* call cpss api function */
    result = cpssDxChPtpTaiIncomingClockCounterGet(
        devNum, &taiId, &enable, &inClockCnt);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", enable, inClockCnt);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTaiFrequencyDriftThesholdsSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Frequency Drift Thesholds.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTaiFrequencyDriftThesholdsSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_PTP_TAI_ID_STC            taiId;
    GT_U32                              interruptThreshold;
    GT_U32                              adjustThreshold;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    taiId.taiInstance  = (CPSS_DXCH_PTP_TAI_INSTANCE_ENT)inArgs[1];
    taiId.taiNumber    = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[2];
    taiId.portNum      = (GT_PHYSICAL_PORT_NUM)inArgs[3];
    interruptThreshold = (GT_U32)inArgs[4];
    adjustThreshold    = (GT_U32)inArgs[5];

    /* call cpss api function */
    result = cpssDxChPtpTaiFrequencyDriftThesholdsSet(
        devNum, &taiId, interruptThreshold, adjustThreshold);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTaiFrequencyDriftThesholdsGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Frequency Drift Thesholds.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTaiFrequencyDriftThesholdsGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_PTP_TAI_ID_STC            taiId;
    GT_U32                              interruptThreshold;
    GT_U32                              adjustThreshold;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    taiId.taiInstance = (CPSS_DXCH_PTP_TAI_INSTANCE_ENT)inArgs[1];
    taiId.taiNumber   = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[2];
    taiId.portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[3];

    /* call cpss api function */
    result = cpssDxChPtpTaiFrequencyDriftThesholdsGet(
        devNum, &taiId, &interruptThreshold, &adjustThreshold);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", interruptThreshold, adjustThreshold);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTaiGracefulStepSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Graceful Step.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on wrong gracefulStep
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTaiGracefulStepSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_PTP_TAI_ID_STC            taiId;
    GT_U32                              gracefulStep;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    taiId.taiInstance  = (CPSS_DXCH_PTP_TAI_INSTANCE_ENT)inArgs[1];
    taiId.taiNumber    = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[2];
    taiId.portNum      = (GT_PHYSICAL_PORT_NUM)inArgs[3];
    gracefulStep       = (GT_U32)inArgs[4];

    /* call cpss api function */
    result = cpssDxChPtpTaiGracefulStepSet(devNum, &taiId, gracefulStep);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTaiGracefulStepGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Graceful Step.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTaiGracefulStepGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_PTP_TAI_ID_STC            taiId;
    GT_U32                              gracefulStep;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    taiId.taiInstance = (CPSS_DXCH_PTP_TAI_INSTANCE_ENT)inArgs[1];
    taiId.taiNumber   = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[2];
    taiId.portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[3];

    /* call cpss api function */
    result = cpssDxChPtpTaiGracefulStepGet(devNum, &taiId, &gracefulStep);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", gracefulStep);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTsuControlTableSet function
* @endinternal
*
* @brief   Set TSU (Time Stamp Unit) Control.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTsuControlTableSet
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
    CPSS_DXCH_PTP_TSU_CONTROL_STC   control;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* to support the not seted/geted members of structures */
    cpssOsMemSet(&control, 0, sizeof(control));

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    /* startPort and numOfPorts arguments ignored for Set */

    portNum                     = (GT_PHYSICAL_PORT_NUM)inFields[0];
    control.unitEnable          = (GT_BOOL)inFields[1];
    control.rxTaiSelect         = (GT_U32)inFields[2];
    control.tsQueOverrideEnable = (GT_BOOL)inFields[3];

    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPtpTsuControlSet(
        devNum, portNum, &control);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**
* @internal wrCpssDxChPtpTsuControlTableGet_util function
* @endinternal
*
* @brief   Get TSU (Time Stamp Unit) Control.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTsuControlTableGet_util
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32      numFields,
    OUT GT_8       outArgs[CMD_MAX_BUFFER],
    IN  GT_BOOL   isGetFirst
)
{
    GT_STATUS rc;
    GT_U8     devNum;
    GT_U8     devNum1;
    /* variables initialised only to prevent compiler warnings */
    static GT_PHYSICAL_PORT_NUM     portNum = 0;
    static GT_U32     numOfPorts = 0;
    CPSS_DXCH_PTP_TSU_CONTROL_STC   control;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum = (GT_U8)inArgs[0];
    if (isGetFirst == GT_TRUE)
    {
        devNum1     = devNum;
        portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[1];
        numOfPorts  = inArgs[2];
        CONVERT_DEV_PHYSICAL_PORT_MAC(devNum1, portNum);
        if (numOfPorts == 0)
        {
            /* no ports more */
            galtisOutput(outArgs, GT_OK, "%d", -1);
            return CMD_OK;
        }
        numOfPorts --;
    }
    else
    {
        if (numOfPorts == 0)
        {
            /* no ports more */
            galtisOutput(outArgs, GT_OK, "%d", -1);
            return CMD_OK;
        }
        numOfPorts --;

        /* find next existing port */
        for (portNum++; (1); portNum++)
        {
            if (portNum >= PRV_CPSS_PP_MAC(devNum)->numOfPorts)
            {
                /* no ports more */
                galtisOutput(outArgs, GT_OK, "%d", -1);
                return CMD_OK;
            }
            if (CPSS_PORTS_BMP_IS_PORT_SET_MAC(
                &(PRV_CPSS_PP_MAC(devNum)->existingPorts), portNum))
            {
                break;
            }
        }
    }


    rc = cpssDxChPtpTsuControlGet(
        devNum, portNum, &control);
    if (rc != GT_OK)
    {
        galtisOutput(outArgs, rc, "cpssDxChPtpTsuControlGet");
        return CMD_OK;
    }

    /* pack and output table fields */
    fieldOutput(
        "%d%d%d%d", portNum, control.unitEnable,
        control.rxTaiSelect, control.tsQueOverrideEnable);
    galtisOutput(outArgs, rc, "%f");

    return CMD_OK;
}

static CMD_STATUS wrCpssDxChPtpTsuControlTableGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssDxChPtpTsuControlTableGet_util(
        inArgs, inFields, numFields, outArgs, GT_TRUE/*isGetFirst*/);
}

static CMD_STATUS wrCpssDxChPtpTsuControlTableGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssDxChPtpTsuControlTableGet_util(
        inArgs, inFields, numFields, outArgs, GT_FALSE/*isGetFirst*/);
}

/* configurable limit of amount of reading of PtpTsuTxTimestampQueue */
/* needed to avoid endless loop due to HW bug                        */
/* default - unlimited                                               */
static GT_U32 ptpTsuTxTimestampQueueTableReadLimit = 0xFFFFFFFF;
/* current amount of reading of PtpTsuTxTimestampQueue               */
static GT_U32 ptpTsuTxTimestampQueueTableReadAmount = 0;


static CMD_STATUS wrUtilPtpTsuTxTimestampQueueTableReadLimitSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    ptpTsuTxTimestampQueueTableReadLimit = (GT_U32)inArgs[0];

    galtisOutput(outArgs, GT_OK, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChPtpTsuTxTimestampQueueTableGet function
* @endinternal
*
* @brief   Read TSU (Time Stamp Unit) TX Timestamp Queue Entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Entry deleted from the Queue by reading it.
*
*/
static CMD_STATUS wrCpssDxChPtpTsuTxTimestampQueueTableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                          rc;
    GT_U8                                              devNum;
    GT_PHYSICAL_PORT_NUM                               portNum;
    GT_U32                                             queueNum;
    CPSS_DXCH_PTP_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC     queuEntry;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum      = (GT_U8)inArgs[0];
    portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    queueNum    = (GT_U32)inArgs[2];
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    if (ptpTsuTxTimestampQueueTableReadAmount
        >= ptpTsuTxTimestampQueueTableReadLimit)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    ptpTsuTxTimestampQueueTableReadAmount ++;

    rc = cpssDxChPtpTsuTxTimestampQueueRead(
        devNum, portNum, queueNum, &queuEntry);
    if (rc != GT_OK)
    {
        galtisOutput(outArgs, rc, "cpssDxChPtpTsuTxTimestampQueueRead");
        return CMD_OK;
    }

    if (queuEntry.entryValid == GT_FALSE)
    {
        /* no entries more */
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* pack and output table fields */
    fieldOutput(
        "%d%d%d%d", queuEntry.entryId, queuEntry.taiSelect,
        queuEntry.todUpdateFlag, queuEntry.timestamp);
    galtisOutput(outArgs, rc, "%f");

    return CMD_OK;
}

static CMD_STATUS wrCpssDxChPtpTsuTxTimestampQueueTableGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    ptpTsuTxTimestampQueueTableReadAmount = 0;

    return wrCpssDxChPtpTsuTxTimestampQueueTableGet(
        inArgs, inFields, numFields, outArgs);
}

/**
* @internal wrCpssDxChPtpTsuCountersClear function
* @endinternal
*
* @brief   Clear All TSU (Time Stamp Unit) Packet counters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTsuCountersClear
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_PHYSICAL_PORT_NUM       portNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPtpTsuCountersClear(
        devNum, portNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTsuPacketCouterGet function
* @endinternal
*
* @brief   Get TSU (Time Stamp Unit) packet counter.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTsuPacketCouterGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                 result;
    GT_U8                                     devNum;
    GT_PHYSICAL_PORT_NUM                                     portNum;
    CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_ENT counterType;
    GT_U32                                    conterVal;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum     = (GT_U8)inArgs[0];
    portNum    = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    counterType  = (CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_ENT)inArgs[2];
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);


    /* call cpss api function */
    result = cpssDxChPtpTsuPacketCouterGet(
        devNum, portNum, counterType, &conterVal);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", conterVal);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTsuNtpTimeOffsetSet function
* @endinternal
*
* @brief   Set TSU (Time Stamp Unit) NTP Time Offset.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTsuNtpTimeOffsetSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_PHYSICAL_PORT_NUM       portNum;
    GT_U32      ntpTimeOffset;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum         = (GT_U8)inArgs[0];
    portNum        = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    ntpTimeOffset  = (GT_U8)inArgs[2];
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPtpTsuNtpTimeOffsetSet(
        devNum, portNum, ntpTimeOffset);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTsuNtpTimeOffsetGet function
* @endinternal
*
* @brief   Get TSU (Time Stamp Unit) NTP Time Offset.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTsuNtpTimeOffsetGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_PHYSICAL_PORT_NUM       portNum;
    GT_U32      ntpTimeOffset;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum         = (GT_U8)inArgs[0];
    portNum        = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPtpTsuNtpTimeOffsetGet(
        devNum, portNum, &ntpTimeOffset);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", ntpTimeOffset);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTsTagGlobalCfgSet function
* @endinternal
*
* @brief   Set Timestamp Global Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTsTagGlobalCfgSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_PTP_TS_TAG_GLOBAL_CFG_STC tsTagGlobalCfg;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cpssOsMemSet(&tsTagGlobalCfg, 0, sizeof(tsTagGlobalCfg));

    /* map input arguments to locals */
    devNum                                 = (GT_U8)inArgs[0];
    tsTagGlobalCfg.tsTagParseEnable        = (GT_BOOL)inArgs[1];
    tsTagGlobalCfg.hybridTsTagParseEnable  = (GT_BOOL)inArgs[2];
    tsTagGlobalCfg.tsTagEtherType          = (GT_U32)inArgs[3];
    tsTagGlobalCfg.hybridTsTagEtherType    = (GT_U32)inArgs[4];

    /* call cpss api function */
    result = cpssDxChPtpTsTagGlobalCfgSet(
        devNum, &tsTagGlobalCfg);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTsTagGlobalCfgGet function
* @endinternal
*
* @brief   Get Timestamp Global Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTsTagGlobalCfgGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_PTP_TS_TAG_GLOBAL_CFG_STC tsTagGlobalCfg;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cpssOsMemSet(&tsTagGlobalCfg, 0, sizeof(tsTagGlobalCfg));

    /* map input arguments to locals */
    devNum                                 = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPtpTsTagGlobalCfgGet(
        devNum, &tsTagGlobalCfg);

    /* pack output arguments to galtis string */
    galtisOutput(
        outArgs, result, "%d%d%d%d",
        tsTagGlobalCfg.tsTagParseEnable,
        tsTagGlobalCfg.hybridTsTagParseEnable,
        tsTagGlobalCfg.tsTagEtherType,
        tsTagGlobalCfg.hybridTsTagEtherType);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTsTagPortCfgTableSet function
* @endinternal
*
* @brief   Set Timestamp Port Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTsTagPortCfgTableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
        GT_PHYSICAL_PORT_NUM            portNum;
    CPSS_DXCH_PTP_TS_TAG_PORT_CFG_STC   tsTagPortCfg;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* to support the not seted/geted members of structures */
    cpssOsMemSet(&tsTagPortCfg, 0, sizeof(tsTagPortCfg));

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    /* startPort and numOfPorts arguments ignored for Set */

    portNum                        = (GT_PHYSICAL_PORT_NUM)inFields[0];
    tsTagPortCfg.tsReceptionEnable = (GT_BOOL)inFields[1];
    tsTagPortCfg.tsPiggyBackEnable = (GT_BOOL)inFields[2];
    tsTagPortCfg.tsTagMode         = (CPSS_DXCH_PTP_TS_TAG_MODE_ENT)inFields[3];

    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPtpTsTagPortCfgSet(
        devNum, portNum, &tsTagPortCfg);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTsTagPortCfgTableGet_util function
* @endinternal
*
* @brief   Get Timestamp Port Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTsTagPortCfgTableGet_util
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32      numFields,
    OUT GT_8       outArgs[CMD_MAX_BUFFER],
    IN  GT_BOOL    isGetFirst
)
{
    GT_STATUS rc;
    GT_U8     devNum;
    GT_U8     devNum1;
    /* variables initialised only to prevent compiler warnings */
    static GT_PHYSICAL_PORT_NUM         portNum = 0;
    static GT_U32                       numOfPorts = 0;
    CPSS_DXCH_PTP_TS_TAG_PORT_CFG_STC   tsTagPortCfg;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum = (GT_U8)inArgs[0];
    if (isGetFirst == GT_TRUE)
    {
        devNum1     = devNum;
        portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[1];
        numOfPorts  = inArgs[2];
        CONVERT_DEV_PHYSICAL_PORT_MAC(devNum1, portNum);
        if (numOfPorts == 0)
        {
            /* no ports more */
            galtisOutput(outArgs, GT_OK, "%d", -1);
            return CMD_OK;
        }
        numOfPorts --;
    }
    else
    {
        if (numOfPorts == 0)
        {
            /* no ports more */
            galtisOutput(outArgs, GT_OK, "%d", -1);
            return CMD_OK;
        }
        numOfPorts --;

        /* find next existing port */
        for (portNum++; (1); portNum++)
        {
            if (portNum >= PRV_CPSS_PP_MAC(devNum)->numOfPorts)
            {
                /* no ports more */
                galtisOutput(outArgs, GT_OK, "%d", -1);
                return CMD_OK;
            }
            if (CPSS_PORTS_BMP_IS_PORT_SET_MAC(
                &(PRV_CPSS_PP_MAC(devNum)->existingPorts), portNum))
            {
                break;
            }
        }
    }


    rc = cpssDxChPtpTsTagPortCfgGet(
        devNum, portNum, &tsTagPortCfg);
    if (rc != GT_OK)
    {
        galtisOutput(outArgs, rc, "cpssDxChPtpTsTagPortCfgGet");
        return CMD_OK;
    }

    /* pack and output table fields */
    fieldOutput(
        "%d%d%d%d", portNum, tsTagPortCfg.tsReceptionEnable,
        tsTagPortCfg.tsPiggyBackEnable, tsTagPortCfg.tsTagMode);
    galtisOutput(outArgs, rc, "%f");

    return CMD_OK;
}

static CMD_STATUS wrCpssDxChPtpTsTagPortCfgTableGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssDxChPtpTsTagPortCfgTableGet_util(
        inArgs, inFields, numFields, outArgs, GT_TRUE/*isGetFirst*/);
}

static CMD_STATUS wrCpssDxChPtpTsTagPortCfgTableGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssDxChPtpTsTagPortCfgTableGet_util(
        inArgs, inFields, numFields, outArgs, GT_FALSE/*isGetFirst*/);
}

/**
* @internal wrCpssDxChPtpTsDelayIngressPortDelaySet function
* @endinternal
*
* @brief   Set Ingress Port Delay Correction.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTsDelayIngressPortDelaySet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_PHYSICAL_PORT_NUM       portNum;
    GT_32       ingrPortDelayCorr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum             = (GT_U8)inArgs[0];
    portNum            = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    ingrPortDelayCorr  = (GT_32)inArgs[2];
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPtpTsDelayIngressPortDelaySet(
        devNum, portNum, ingrPortDelayCorr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTsDelayIngressPortDelayGet function
* @endinternal
*
* @brief   Get Port Delay Correction Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTsDelayIngressPortDelayGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_PHYSICAL_PORT_NUM       portNum;
    GT_32       ingrPortDelayCorr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum            = (GT_U8)inArgs[0];
    portNum           = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPtpTsDelayIngressPortDelayGet(
        devNum, portNum, &ingrPortDelayCorr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", ingrPortDelayCorr);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTsDelayIngressLinkDelaySet function
* @endinternal
*
* @brief   Set Ingress Link Delay.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTsDelayIngressLinkDelaySet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_PHYSICAL_PORT_NUM       portNum;
    GT_U32      domainProfile;
    GT_32       ingrPortDelay;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum             = (GT_U8)inArgs[0];
    portNum            = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    domainProfile      = (GT_U32)inArgs[2];
    ingrPortDelay      = (GT_32)inArgs[3];
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPtpTsDelayIngressLinkDelaySet(
        devNum, portNum, domainProfile, ingrPortDelay);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTsDelayIngressLinkDelayGet function
* @endinternal
*
* @brief   Get Ingress Link Delay.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTsDelayIngressLinkDelayGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_PHYSICAL_PORT_NUM       portNum;
    GT_U32      domainProfile;
    GT_32       ingrPortDelay;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum            = (GT_U8)inArgs[0];
    portNum           = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    domainProfile     = (GT_U32)inArgs[2];
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPtpTsDelayIngressLinkDelayGet(
        devNum, portNum, domainProfile, &ingrPortDelay);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", ingrPortDelay);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTsDelayEgressPipeDelaySet function
* @endinternal
*
* @brief   Set Egress Pipe Delay Correction.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTsDelayEgressPipeDelaySet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_PHYSICAL_PORT_NUM       portNum;
    GT_32       egrPipeDelayCorr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum             = (GT_U8)inArgs[0];
    portNum            = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    egrPipeDelayCorr   = (GT_32)inArgs[2];
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPtpTsDelayEgressPipeDelaySet(
        devNum, portNum, egrPipeDelayCorr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTsDelayEgressPipeDelayGet function
* @endinternal
*
* @brief   Get Egress Pipe Delay Correction.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTsDelayEgressPipeDelayGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_PHYSICAL_PORT_NUM       portNum;
    GT_32       egrPipeDelayCorr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum            = (GT_U8)inArgs[0];
    portNum           = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPtpTsDelayEgressPipeDelayGet(
        devNum, portNum, &egrPipeDelayCorr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", egrPipeDelayCorr);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTsDelayEgressAsymmetryCorrectionSet function
* @endinternal
*
* @brief   Set Egress Asymmetry Correction.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTsDelayEgressAsymmetryCorrectionSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_PHYSICAL_PORT_NUM       portNum;
    GT_U32      domainNum;
    GT_32       egrAsymmetryCorr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum             = (GT_U8)inArgs[0];
    portNum            = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    domainNum          = (GT_U32)inArgs[2];
    egrAsymmetryCorr   = (GT_32)inArgs[3];
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPtpTsDelayEgressAsymmetryCorrectionSet(
        devNum, portNum, domainNum, egrAsymmetryCorr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTsDelayEgressAsymmetryCorrectionGet function
* @endinternal
*
* @brief   Get Egress Asymmetry Correction.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTsDelayEgressAsymmetryCorrectionGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_PHYSICAL_PORT_NUM       portNum;
    GT_U32      domainNum;
    GT_32       egrAsymmetryCorr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum            = (GT_U8)inArgs[0];
    portNum           = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    domainNum          = (GT_U32)inArgs[2];
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPtpTsDelayEgressAsymmetryCorrectionGet(
        devNum, portNum, domainNum, &egrAsymmetryCorr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", egrAsymmetryCorr);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTsDelayIngressCorrFieldPiggybackEnableSet function
* @endinternal
*
* @brief   Set Ingress Correction Field Piggyback Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTsDelayIngressCorrFieldPiggybackEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_PHYSICAL_PORT_NUM       portNum;
    GT_BOOL     ingrCorrFldPBEnable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum              = (GT_U8)inArgs[0];
    portNum             = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    ingrCorrFldPBEnable = (GT_BOOL)inArgs[2];
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPtpTsDelayIngressCorrFieldPiggybackEnableSet(
        devNum, portNum, ingrCorrFldPBEnable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTsDelayIngressCorrFieldPiggybackEnableGet function
* @endinternal
*
* @brief   Get Ingress Correction Field Piggyback Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTsDelayIngressCorrFieldPiggybackEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_PHYSICAL_PORT_NUM       portNum;
    GT_BOOL     ingrCorrFldPBEnable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum              = (GT_U8)inArgs[0];
    portNum             = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPtpTsDelayIngressCorrFieldPiggybackEnableGet(
        devNum, portNum, &ingrCorrFldPBEnable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", ingrCorrFldPBEnable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTsDelayEgressCorrFieldPiggybackEnableSet function
* @endinternal
*
* @brief   Set Egress Correction Field Piggyback Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTsDelayEgressCorrFieldPiggybackEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_PHYSICAL_PORT_NUM       portNum;
    GT_BOOL     egrCorrFldPBEnable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum              = (GT_U8)inArgs[0];
    portNum             = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    egrCorrFldPBEnable  = (GT_BOOL)inArgs[2];
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPtpTsDelayEgressCorrFieldPiggybackEnableSet(
        devNum, portNum, egrCorrFldPBEnable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTsDelayEgressCorrFieldPiggybackEnableGet function
* @endinternal
*
* @brief   Get Egress Correction Field Piggyback Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTsDelayEgressCorrFieldPiggybackEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_PHYSICAL_PORT_NUM       portNum;
    GT_BOOL     egrCorrFldPBEnable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum              = (GT_U8)inArgs[0];
    portNum             = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPtpTsDelayEgressCorrFieldPiggybackEnableGet(
        devNum, portNum, &egrCorrFldPBEnable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", egrCorrFldPBEnable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTsDelayEgressTimeCorrTaiSelectModeSet function
* @endinternal
*
* @brief   Set Egress Time Correction TAI Select Mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTsDelayEgressTimeCorrTaiSelectModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                 result;
    GT_U8                                     devNum;
    GT_PHYSICAL_PORT_NUM                                     portNum;
    CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_ENT   egrTimeCorrTaiSelMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                 = (GT_U8)inArgs[0];
    portNum                = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    egrTimeCorrTaiSelMode  = (CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_ENT)inArgs[2];
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPtpTsDelayEgressTimeCorrTaiSelectModeSet(
        devNum, portNum, egrTimeCorrTaiSelMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTsDelayEgressTimeCorrTaiSelectModeGet function
* @endinternal
*
* @brief   Get Egress Time Correction TAI Select Mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTsDelayEgressTimeCorrTaiSelectModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                 result;
    GT_U8                                     devNum;
    GT_PHYSICAL_PORT_NUM                                     portNum;
    CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_ENT   egrTimeCorrTaiSelMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum              = (GT_U8)inArgs[0];
    portNum             = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPtpTsDelayEgressTimeCorrTaiSelectModeGet(
        devNum, portNum, &egrTimeCorrTaiSelMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", egrTimeCorrTaiSelMode);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTsDelayEgressTimestampTaiSelectSet function
* @endinternal
*
* @brief   Set Egress Timestamp TAI Select.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTsDelayEgressTimestampTaiSelectSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS    result;
    GT_U8        devNum;
    GT_PHYSICAL_PORT_NUM        portNum;
    GT_U32       egrTsTaiNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum       = (GT_U8)inArgs[0];
    portNum      = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    egrTsTaiNum  = (GT_U32)inArgs[2];
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPtpTsDelayEgressTimestampTaiSelectSet(
        devNum, portNum, egrTsTaiNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTsDelayEgressTimestampTaiSelectGet function
* @endinternal
*
* @brief   Get Egress Timestamp TAI Select.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTsDelayEgressTimestampTaiSelectGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS    result;
    GT_U8        devNum;
    GT_PHYSICAL_PORT_NUM        portNum;
    GT_U32       egrTsTaiNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum     = (GT_U8)inArgs[0];
    portNum    = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPtpTsDelayEgressTimestampTaiSelectGet(
        devNum, portNum, &egrTsTaiNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", egrTsTaiNum);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTsUdpChecksumUpdateModeSet function
* @endinternal
*
* @brief   Set Timestamp UDP Checksum Update Mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTsUdpChecksumUpdateModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                     result;
    GT_U8                                         devNum;
    CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_STC modes;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum             = (GT_U8)inArgs[0];
    modes.ptpIpv4Mode  = (CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT)inArgs[1];
    modes.ptpIpv6Mode  = (CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT)inArgs[2];
    modes.ntpIpv4Mode  = (CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT)inArgs[3];
    modes.ntpIpv6Mode  = (CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT)inArgs[4];
    modes.wampIpv4Mode = (CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT)inArgs[5];
    modes.wampIpv6Mode = (CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT)inArgs[6];

    /* call cpss api function */
    result = cpssDxChPtpTsUdpChecksumUpdateModeSet(
        devNum, &modes);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTsUdpChecksumUpdateModeGet function
* @endinternal
*
* @brief   Get Timestamp UDP Checksum Update Mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTsUdpChecksumUpdateModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                     result;
    GT_U8                                         devNum;
    CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_STC modes;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum     = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPtpTsUdpChecksumUpdateModeGet(
        devNum, &modes);

    /* pack output arguments to galtis string */
    galtisOutput(
        outArgs, result, "%d%d%d%d%d%d%d",
        modes.ptpIpv4Mode, modes.ptpIpv6Mode,
        modes.ntpIpv4Mode, modes.ntpIpv6Mode,
        modes.wampIpv4Mode, modes.wampIpv6Mode);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpOverEthernetEnableSet function
* @endinternal
*
* @brief   Set enable PTP over Ethernet packet.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpOverEthernetEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS    result;
    GT_U8        devNum;
    GT_BOOL      enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    enable  = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPtpOverEthernetEnableSet(
        devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpOverEthernetEnableGet function
* @endinternal
*
* @brief   Get enable PTP over Ethernet packet.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpOverEthernetEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS    result;
    GT_U8        devNum;
    GT_BOOL      enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum     = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPtpOverEthernetEnableGet(
        devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(
        outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpOverUdpEnableSet function
* @endinternal
*
* @brief   Set enable PTP over UDP packet.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpOverUdpEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS    result;
    GT_U8        devNum;
    GT_BOOL      enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    enable  = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPtpOverUdpEnableSet(
        devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpOverUdpEnableGet function
* @endinternal
*
* @brief   Get enable PTP over UDP packet.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpOverUdpEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS    result;
    GT_U8        devNum;
    GT_BOOL      enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum     = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPtpOverUdpEnableGet(
        devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(
        outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpIngressExceptionCfgSet function
* @endinternal
*
* @brief   Set PTP packet Ingress Exception Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpIngressExceptionCfgSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                               result;
    GT_U8                                   devNum;
    CPSS_DXCH_PTP_INGRESS_EXCEPTION_CFG_STC cfg;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                         = (GT_U8)inArgs[0];
    cfg.ptpExceptionCommandEnable  = (GT_BOOL)inArgs[1];
    cfg.ptpExceptionCommand        = (CPSS_PACKET_CMD_ENT)inArgs[2];
    cfg.ptpExceptionCpuCode        = (CPSS_NET_RX_CPU_CODE_ENT)inArgs[3];
    cfg.ptpVersionCheckEnable      = (GT_BOOL)inArgs[4];

    /* call cpss api function */
    result = cpssDxChPtpIngressExceptionCfgSet(
        devNum, &cfg);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpIngressExceptionCfgGet function
* @endinternal
*
* @brief   Get PTP packet Ingress Exception Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpIngressExceptionCfgGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                               result;
    GT_U8                                   devNum;
    CPSS_DXCH_PTP_INGRESS_EXCEPTION_CFG_STC cfg;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum     = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPtpIngressExceptionCfgGet(
        devNum, &cfg);

    /* pack output arguments to galtis string */
    galtisOutput(
        outArgs, result, "%d%d%d%d",
        cfg.ptpExceptionCommandEnable, cfg.ptpExceptionCommand,
        cfg.ptpExceptionCpuCode, cfg.ptpVersionCheckEnable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpEgressExceptionCfgSet function
* @endinternal
*
* @brief   Set PTP packet Egress Exception Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpEgressExceptionCfgSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                               result;
    GT_U8                                   devNum;
    CPSS_DXCH_PTP_EGRESS_EXCEPTION_CFG_STC  cfg;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* init with zeros */
    cmdOsMemSet(&cfg, 0, sizeof(cfg));

    /* map input arguments to locals */
    devNum                         = (GT_U8)inArgs[0];

    result = cpssDxChPtpEgressExceptionCfgGet(
        devNum, &cfg);
    if (result != GT_OK)
    {
        goto end;
    }

    cfg.invalidPtpPktCmd           = (CPSS_PACKET_CMD_ENT)inArgs[1];
    cfg.invalidPtpCpuCode          = CPSS_NET_FIRST_USER_DEFINED_E;
    cfg.invalidOutPiggybackPktCmd  = (CPSS_PACKET_CMD_ENT)inArgs[2];
    cfg.invalidOutPiggybackCpuCode = CPSS_NET_FIRST_USER_DEFINED_E;
    cfg.invalidInPiggybackPktCmd   = (CPSS_PACKET_CMD_ENT)inArgs[3];
    cfg.invalidInPiggybackCpuCode  = CPSS_NET_FIRST_USER_DEFINED_E;
    cfg.invalidTsPktCmd            = (CPSS_PACKET_CMD_ENT)inArgs[4];
    cfg.invalidTsCpuCode           = CPSS_NET_FIRST_USER_DEFINED_E;

    /* call cpss api function */
    result = cpssDxChPtpEgressExceptionCfgSet(
        devNum, &cfg);
end:
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

static CMD_STATUS wrCpssDxChPtpEgressExceptionCfgSet_1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                               result;
    GT_U8                                   devNum;
    CPSS_DXCH_PTP_EGRESS_EXCEPTION_CFG_STC  cfg;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* init with zeros */
    cmdOsMemSet(&cfg, 0, sizeof(cfg));

    /* map input arguments to locals */
    devNum                         = (GT_U8)inArgs[0];

    cfg.invalidPtpPktCmd           = (CPSS_PACKET_CMD_ENT)inArgs[1];
    cfg.invalidPtpCpuCode          = (CPSS_NET_RX_CPU_CODE_ENT)inArgs[2];
    cfg.invalidOutPiggybackPktCmd  = (CPSS_PACKET_CMD_ENT)inArgs[3];
    cfg.invalidOutPiggybackCpuCode = (CPSS_NET_RX_CPU_CODE_ENT)inArgs[4];
    cfg.invalidInPiggybackPktCmd   = (CPSS_PACKET_CMD_ENT)inArgs[5];
    cfg.invalidInPiggybackCpuCode  = (CPSS_NET_RX_CPU_CODE_ENT)inArgs[6];
    cfg.invalidTsPktCmd            = (CPSS_PACKET_CMD_ENT)inArgs[7];
    cfg.invalidTsCpuCode           = (CPSS_NET_RX_CPU_CODE_ENT)inArgs[8];

    /* call cpss api function */
    result = cpssDxChPtpEgressExceptionCfgSet(
        devNum, &cfg);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpEgressExceptionCfgGet function
* @endinternal
*
* @brief   Get PTP packet Egress Exception Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpEgressExceptionCfgGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                               result;
    GT_U8                                   devNum;
    CPSS_DXCH_PTP_EGRESS_EXCEPTION_CFG_STC  cfg;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum     = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPtpEgressExceptionCfgGet(
        devNum, &cfg);

    /* pack output arguments to galtis string */
    galtisOutput(
        outArgs, result, "%d%d%d%d",
        cfg.invalidPtpPktCmd, cfg.invalidOutPiggybackPktCmd,
        cfg.invalidInPiggybackPktCmd, cfg.invalidTsPktCmd);

    return CMD_OK;
}

static CMD_STATUS wrCpssDxChPtpEgressExceptionCfgGet_1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                               result;
    GT_U8                                   devNum;
    CPSS_DXCH_PTP_EGRESS_EXCEPTION_CFG_STC  cfg;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum     = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPtpEgressExceptionCfgGet(
        devNum, &cfg);

    /* pack output arguments to galtis string */
    galtisOutput(
        outArgs, result, "%d%d%d%d%d%d%d%d",
        cfg.invalidPtpPktCmd, cfg.invalidPtpCpuCode,
        cfg.invalidOutPiggybackPktCmd, cfg.invalidOutPiggybackCpuCode,
        cfg.invalidInPiggybackPktCmd, cfg.invalidInPiggybackCpuCode,
        cfg.invalidTsPktCmd, cfg.invalidTsCpuCode);

    return CMD_OK;
}
/**
* @internal wrCpssDxChPtpIngressExceptionCounterGet function
* @endinternal
*
* @brief   Get PTP Ingress Exception packet Counter.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The counter is Clear On Read.
*
*/
static CMD_STATUS wrCpssDxChPtpIngressExceptionCounterGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;
    GT_U8      devNum;
    GT_U32     counter;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum     = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPtpIngressExceptionCounterGet(
        devNum, &counter);

    /* pack output arguments to galtis string */
    galtisOutput(
        outArgs, result, "%d", counter);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpEgressExceptionCountersGet function
* @endinternal
*
* @brief   Get PTP packet Egress Exception Counters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The counters are Clear On Read.
*
*/
static CMD_STATUS wrCpssDxChPtpEgressExceptionCountersGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;
    GT_U8                                       devNum;
    CPSS_DXCH_PTP_EGRESS_EXCEPTION_COUNTERS_STC counter;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum     = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPtpEgressExceptionCountersGet(
        devNum, &counter);

    /* pack output arguments to galtis string */
    galtisOutput(
        outArgs, result, "%d%d%d%d",
        counter.invalidPtpPktCnt, counter.invalidOutPiggybackPktCnt,
        counter.invalidInPiggybackPktCnt, counter.invalidTsPktCnt);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTsMessageTypeToQueueIdMapSet function
* @endinternal
*
* @brief   Set PTP Message Type To Queue Id Map.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTsMessageTypeToQueueIdMapSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS    result;
    GT_U8        devNum;
    GT_U32       idMapBmp;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum    = (GT_U8)inArgs[0];
    idMapBmp  = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPtpTsMessageTypeToQueueIdMapSet(
        devNum, idMapBmp);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTsMessageTypeToQueueIdMapGet function
* @endinternal
*
* @brief   Get PTP Message Type To Queue Id Map.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTsMessageTypeToQueueIdMapGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS    result;
    GT_U8        devNum;
    GT_U32       idMapBmp;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum     = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPtpTsMessageTypeToQueueIdMapGet(
        devNum, &idMapBmp);

    /* pack output arguments to galtis string */
    galtisOutput(
        outArgs, result, "%d", idMapBmp);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTsQueuesEntryOverrideEnableSet function
* @endinternal
*
* @brief   Set Timestamp Queues Override Enable.
*         Related to all 2 ingress and 2 egress Timestamp Queues.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note cpssDxChPtpTsuControlSet should configure the same value per port
*
*/
static CMD_STATUS wrCpssDxChPtpTsQueuesEntryOverrideEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS    result;
    GT_U8        devNum;
    GT_BOOL      enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    enable  = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPtpTsQueuesEntryOverrideEnableSet(
        devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTsQueuesEntryOverrideEnableGet function
* @endinternal
*
* @brief   Get Timestamp Queues Override Enable.
*         Related to all 2 ingress and 2 egress Timestamp Queues.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTsQueuesEntryOverrideEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS    result;
    GT_U8        devNum;
    GT_BOOL      enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum     = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPtpTsQueuesEntryOverrideEnableGet(
        devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(
        outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTsQueuesSizeSet function
* @endinternal
*
* @brief   Set Timestamp Queues Size.
*         Related to all 2 ingress and 2 egress Timestamp Queues.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTsQueuesSizeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS    result;
    GT_U8        devNum;
    GT_U32       queueSize;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum    = (GT_U8)inArgs[0];
    queueSize = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPtpTsQueuesSizeSet(
        devNum, queueSize);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTsQueuesSizeGet function
* @endinternal
*
* @brief   Get Timestamp Queues Size.
*         Related to all 2 ingress and 2 egress Timestamp Queues.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTsQueuesSizeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS    result;
    GT_U8        devNum;
    GT_U32       queueSize;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum     = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPtpTsQueuesSizeGet(
        devNum, &queueSize);

    /* pack output arguments to galtis string */
    galtisOutput(
        outArgs, result, "%d", queueSize);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTsDebugQueuesEntryIdsClear function
* @endinternal
*
* @brief   Set Timestamp Queues Current Entry Ids Clear.
*         For Debug only.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTsDebugQueuesEntryIdsClear
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS    result;
    GT_U8        devNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum    = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPtpTsDebugQueuesEntryIdsClear(
        devNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTsDebugQueuesEntryIdsGet function
* @endinternal
*
* @brief   Get Timestamp Queues Current Entry IDs.
*         For Debug only.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTsDebugQueuesEntryIdsGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS    result;
    GT_U8        devNum;
    GT_U32       queueEntryId0;
    GT_U32       queueEntryId1;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum     = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPtpTsDebugQueuesEntryIdsGet(
        devNum, &queueEntryId0, &queueEntryId1);

    /* pack output arguments to galtis string */
    galtisOutput(
        outArgs, result, "%d%d", queueEntryId0, queueEntryId1);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTsNtpTimeOffsetSet function
* @endinternal
*
* @brief   Set TS NTP Time Offset.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The same value should be configured per port.
*       See cpssDxChPtpTsuNtpTimeOffsetSet.
*
*/
static CMD_STATUS wrCpssDxChPtpTsNtpTimeOffsetSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS    result;
    GT_U8        devNum;
    GT_U32       ntpTimeOffset;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum        = (GT_U8)inArgs[0];
    ntpTimeOffset = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPtpTsNtpTimeOffsetSet(
        devNum, ntpTimeOffset);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTsNtpTimeOffsetGet function
* @endinternal
*
* @brief   Get TS NTP Time Offset.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTsNtpTimeOffsetGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS    result;
    GT_U8        devNum;
    GT_U32       ntpTimeOffset;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum     = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPtpTsNtpTimeOffsetGet(
        devNum, &ntpTimeOffset);

    /* pack output arguments to galtis string */
    galtisOutput(
        outArgs, result, "%d", ntpTimeOffset);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpMessageTypeCmdTableSet function
* @endinternal
*
* @brief   Configure packet command per PTP message type.
*         The message type is extracted from the PTP header.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum, messageType or command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpMessageTypeCmdTableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                rc;
    GT_U8                    devNum;
    GT_PHYSICAL_PORT_NUM     portNum;
    GT_U32                   domainNum;
    GT_U32                   messageType;
    CPSS_PACKET_CMD_ENT      command;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum      = (GT_U8)inArgs[0];
    portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    domainNum   = (GT_U32)inArgs[2];
    messageType = (GT_U32)inFields[0];
    command     = (CPSS_PACKET_CMD_ENT)inFields[1];

    /* call cpss api function */
    rc = cpssDxChPtpMessageTypeCmdSet(
        devNum, portNum, domainNum, messageType, command);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, rc, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpMessageTypeCmdTableGet_util function
* @endinternal
*
* @brief   Get packet command per PTP message type.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or messageType
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on bad value found in HW
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpMessageTypeCmdTableGet_util
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32      numFields,
    OUT GT_8       outArgs[CMD_MAX_BUFFER],
    IN  GT_BOOL    isGetFirst
)
{
    GT_STATUS                rc;
    GT_U8                    devNum;
    GT_PHYSICAL_PORT_NUM     portNum;
    GT_U32                   domainNum;
    /* variables initialised only to prevent compiler warnings */
    static GT_U32            messageType = 0;
    CPSS_PACKET_CMD_ENT      command;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum      = (GT_U8)inArgs[0];
    portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    domainNum   = (GT_U32)inArgs[2];

    if (isGetFirst == GT_TRUE)
    {
        messageType = 0;
    }
    else
    {
        messageType ++;
        if (messageType > 15)
        {
            /* no entries more */
            galtisOutput(outArgs, GT_OK, "%d", -1);
            return CMD_OK;
        }
    }

    rc = cpssDxChPtpMessageTypeCmdGet(
        devNum, portNum, domainNum, messageType, &command);
    if (rc != GT_OK)
    {
        galtisOutput(outArgs, rc, "cpssDxChPtpMessageTypeCmdGet");
        return CMD_OK;
    }

    /* pack and output table fields */
    fieldOutput(
        "%d%d", messageType, command);
    galtisOutput(outArgs, rc, "%f");

    return CMD_OK;
}

static CMD_STATUS wrCpssDxChPtpMessageTypeCmdTableGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssDxChPtpMessageTypeCmdTableGet_util(
        inArgs, inFields, numFields, outArgs, GT_TRUE/*isGetFirst*/);
}

static CMD_STATUS wrCpssDxChPtpMessageTypeCmdTableGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssDxChPtpMessageTypeCmdTableGet_util(
        inArgs, inFields, numFields, outArgs, GT_FALSE/*isGetFirst*/);
}

/**
* @internal wrCpssDxChPtpTsCfgTableSet function
* @endinternal
*
* @brief   Set Timestamp Configuration Table Entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTsCfgTableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                      rc;
    GT_U8                          devNum;
    GT_U32                         entryIndex;
    CPSS_DXCH_PTP_TS_CFG_ENTRY_STC entry;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                            = (GT_U8)inArgs[0];
    entryIndex                        = (GT_U32)inFields[0];
    entry.tsMode                      = (CPSS_DXCH_PTP_TS_TIMESTAMPING_MODE_ENT)inFields[1];
    entry.offsetProfile               = (GT_U32                                )inFields[2];
    entry.OE                          = (GT_BOOL                               )inFields[3];
    entry.tsAction                    = (CPSS_DXCH_PTP_TS_ACTION_ENT           )inFields[4];
    entry.packetFormat                = (CPSS_DXCH_PTP_TS_PACKET_TYPE_ENT      )inFields[5];
    entry.ptpTransport                = (CPSS_DXCH_PTP_TRANSPORT_TYPE_ENT      )inFields[6];
    entry.offset                      = (GT_U32                                )inFields[7];
    entry.ptpMessageType              = (GT_U32                                )inFields[8];
    entry.domain                      = (GT_U32                                )inFields[9];
    entry.ingrLinkDelayEnable         = (GT_BOOL                               )inFields[10];
    entry.packetDispatchingEnable     = (GT_BOOL                               )inFields[11];
    entry.offset2                     = (GT_U32                                )inFields[12];

    /* call cpss api function */
    rc = cpssDxChPtpTsCfgTableSet(
        devNum, entryIndex, &entry);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, rc, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTsCfgTableGet_util function
* @endinternal
*
* @brief   Get Timestamp Configuration Table Entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTsCfgTableGet_util
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32      numFields,
    OUT GT_8       outArgs[CMD_MAX_BUFFER],
    IN  GT_BOOL    isGetFirst
)
{
    GT_STATUS                      rc;
    GT_U8                          devNum;
    /* variables initialised only to prevent compiler warnings */
    static GT_U32                  entryIndex = 0;
    static GT_U32                  entriesAmount = 0;
    CPSS_DXCH_PTP_TS_CFG_ENTRY_STC entry;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum      = (GT_U8)inArgs[0];

    if (isGetFirst == GT_TRUE)
    {
        entryIndex      = (GT_U8)inArgs[1];
        entriesAmount   = (GT_U32)inArgs[2];
    }
    else
    {
        entryIndex      ++;
        entriesAmount   --;
    }

    if (entriesAmount == 0)
    {
        /* no entries more */
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    rc = cpssDxChPtpTsCfgTableGet(
        devNum, entryIndex, &entry);
    if (rc != GT_OK)
    {
        galtisOutput(outArgs, rc, "cpssDxChPtpTsCfgTableGet");
        return CMD_OK;
    }

    /* pack and output table fields */
    fieldOutput(
        "%d%d%d%d%d%d%d%d%d%d%d%d%d",
        entryIndex,
        entry.tsMode,
        entry.offsetProfile,
        entry.OE,
        entry.tsAction,
        entry.packetFormat,
        entry.ptpTransport,
        entry.offset,
        entry.ptpMessageType,
        entry.domain,
        entry.ingrLinkDelayEnable,
        entry.packetDispatchingEnable,
        entry.offset2);
    galtisOutput(outArgs, rc, "%f");

    return CMD_OK;
}

static CMD_STATUS wrCpssDxChPtpTsCfgTableGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssDxChPtpTsCfgTableGet_util(
        inArgs, inFields, numFields, outArgs, GT_TRUE/*isGetFirst*/);
}

static CMD_STATUS wrCpssDxChPtpTsCfgTableGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssDxChPtpTsCfgTableGet_util(
        inArgs, inFields, numFields, outArgs, GT_FALSE/*isGetFirst*/);
}

/**
* @internal wrCpssDxChPtpTsLocalActionTableSet function
* @endinternal
*
* @brief   Set Timestamp Local Action Table Entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTsLocalActionTableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                               rc;
    GT_U8                                   devNum;
    GT_PHYSICAL_PORT_NUM                    portNum;
    GT_U32                                  domainNum;
    GT_U32                                  messageType;
    CPSS_DXCH_PTP_TS_LOCAL_ACTION_ENTRY_STC entry;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                            = (GT_U8)inArgs[0];
    portNum                           = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    domainNum                         = (GT_U8)inArgs[2];
    messageType                       = (GT_U32)inFields[0];
    entry.tsAction                    = (CPSS_DXCH_PTP_TS_ACTION_ENT)inFields[1];
    entry.ingrLinkDelayEnable         = (GT_BOOL)inFields[2];
    entry.packetDispatchingEnable     = (GT_BOOL)inFields[3];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    rc = cpssDxChPtpTsLocalActionTableSet(
        devNum, portNum, domainNum, messageType, &entry);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, rc, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTsLocalActionTableGet_util function
* @endinternal
*
* @brief   Get Timestamp Local Action Table Entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTsLocalActionTableGet_util
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32      numFields,
    OUT GT_8       outArgs[CMD_MAX_BUFFER],
    IN  GT_BOOL    isGetFirst
)
{
    GT_STATUS                               rc;
    GT_U8                                   devNum;
    GT_PHYSICAL_PORT_NUM                    portNum;
    GT_U32                                  domainNum;
    /* variables initialised only to prevent compiler warnings */
    static GT_U32                           messageType = 0;
    CPSS_DXCH_PTP_TS_LOCAL_ACTION_ENTRY_STC entry;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                            = (GT_U8)inArgs[0];
    portNum                           = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    domainNum                         = (GT_U8)inArgs[2];

    if (isGetFirst == GT_TRUE)
    {
        messageType = 0;
    }
    else
    {
        messageType ++;
    }

    if (messageType > 15)
    {
        /* no entries more */
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    rc = cpssDxChPtpTsLocalActionTableGet(
        devNum, portNum, domainNum, messageType, &entry);
    if (rc != GT_OK)
    {
        galtisOutput(outArgs, rc, "cpssDxChPtpTsLocalActionTableGet");
        return CMD_OK;
    }

    /* pack and output table fields */
    fieldOutput(
        "%d%d%d%d",
        messageType,
        entry.tsAction,
        entry.ingrLinkDelayEnable,
        entry.packetDispatchingEnable);
    galtisOutput(outArgs, rc, "%f");

    return CMD_OK;
}

static CMD_STATUS wrCpssDxChPtpTsLocalActionTableGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssDxChPtpTsLocalActionTableGet_util(
        inArgs, inFields, numFields, outArgs, GT_TRUE/*isGetFirst*/);
}

static CMD_STATUS wrCpssDxChPtpTsLocalActionTableGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssDxChPtpTsLocalActionTableGet_util(
        inArgs, inFields, numFields, outArgs, GT_FALSE/*isGetFirst*/);
}

/**
* @internal wrCpssDxChPtpDomainModeTableSet function
* @endinternal
*
* @brief   Set PTP domain mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpDomainModeTableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                               rc;
    GT_U8                                   devNum;
    GT_U32                                  domainNum;
    CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_ENT   domainMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum      = (GT_U8)inArgs[0];
    domainNum   = (GT_U32)inFields[0];
    domainMode  = (CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_ENT)inFields[1];

    /* call cpss api function */
    rc = cpssDxChPtpDomainModeSet(
        devNum, domainNum, domainMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, rc, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpDomainModeTableGet_util function
* @endinternal
*
* @brief   Get PTP domain mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpDomainModeTableGet_util
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32      numFields,
    OUT GT_8       outArgs[CMD_MAX_BUFFER],
    IN  GT_BOOL    isGetFirst
)
{
    GT_STATUS                               rc;
    GT_U8                                   devNum;
    /* variables initialised only to prevent compiler warnings */
    static GT_U32                           domainNum = 0;
    CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_ENT   domainMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                            = (GT_U8)inArgs[0];

    if (isGetFirst == GT_TRUE)
    {
        domainNum = 0;
    }
    else
    {
        domainNum ++;
    }

    if (domainNum > 3)
    {
        /* no entries more */
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    rc = cpssDxChPtpDomainModeGet(
        devNum, domainNum, &domainMode);
    if (rc != GT_OK)
    {
        galtisOutput(outArgs, rc, "cpssDxChPtpDomainModeGet");
        return CMD_OK;
    }

    /* pack and output table fields */
    fieldOutput(
        "%d%d", domainNum, domainMode);
    galtisOutput(outArgs, rc, "%f");

    return CMD_OK;
}

static CMD_STATUS wrCpssDxChPtpDomainModeTableGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssDxChPtpDomainModeTableGet_util(
        inArgs, inFields, numFields, outArgs, GT_TRUE/*isGetFirst*/);
}

static CMD_STATUS wrCpssDxChPtpDomainModeTableGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssDxChPtpDomainModeTableGet_util(
        inArgs, inFields, numFields, outArgs, GT_FALSE/*isGetFirst*/);
}

/**
* @internal wrCpssDxChPtpDomainV1IdTableSet function
* @endinternal
*
* @brief   Set PTP V1 domain Id.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note PTP V1 Header contains 128-bit domain Id.
*       4 domain Id values mapped to domainNum 0-3
*       all other domain Id values mapped to default domain
*
*/
static CMD_STATUS wrCpssDxChPtpDomainV1IdTableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS    rc;
    GT_U8        devNum;
    GT_U32       domainNum;
    GT_U32       domainIdArr[4];

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_U8)inArgs[0];
    domainNum       = (GT_U32)inFields[0];
    domainIdArr[0]  = (GT_U32)inFields[1];
    domainIdArr[1]  = (GT_U32)inFields[2];
    domainIdArr[2]  = (GT_U32)inFields[3];
    domainIdArr[3]  = (GT_U32)inFields[4];

    /* call cpss api function */
    rc = cpssDxChPtpDomainV1IdSet(
        devNum, domainNum, domainIdArr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, rc, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpDomainV1IdTableGet_util function
* @endinternal
*
* @brief   Get PTP V1 domain Id.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note PTP V1 Header contains 128-bit domain Id.
*       4 domain Id values mapped to domainNum 0-3
*       all other domain Id values mapped to default domain
*
*/
static CMD_STATUS wrCpssDxChPtpDomainV1IdTableGet_util
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32      numFields,
    OUT GT_8       outArgs[CMD_MAX_BUFFER],
    IN  GT_BOOL    isGetFirst
)
{
    GT_STATUS      rc;
    GT_U8          devNum;
    /* variables initialised only to prevent compiler warnings */
    static GT_U32  domainNum = 0;
    GT_U32         domainIdArr[4];

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                            = (GT_U8)inArgs[0];

    if (isGetFirst == GT_TRUE)
    {
        domainNum = 0;
    }
    else
    {
        domainNum ++;
    }

    if (domainNum > 3)
    {
        /* no entries more */
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    rc = cpssDxChPtpDomainV1IdGet(
        devNum, domainNum, domainIdArr);
    if (rc != GT_OK)
    {
        galtisOutput(outArgs, rc, "cpssDxChPtpDomainV1IdGet");
        return CMD_OK;
    }

    /* pack and output table fields */
    fieldOutput(
        "%d%d%d%d%d", domainNum,
        domainIdArr[0], domainIdArr[1], domainIdArr[2], domainIdArr[3]);
    galtisOutput(outArgs, rc, "%f");

    return CMD_OK;
}

static CMD_STATUS wrCpssDxChPtpDomainV1IdTableGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssDxChPtpDomainV1IdTableGet_util(
        inArgs, inFields, numFields, outArgs, GT_TRUE/*isGetFirst*/);
}

static CMD_STATUS wrCpssDxChPtpDomainV1IdTableGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssDxChPtpDomainV1IdTableGet_util(
        inArgs, inFields, numFields, outArgs, GT_FALSE/*isGetFirst*/);
}

/**
* @internal wrCpssDxChPtpDomainV2IdTableSet function
* @endinternal
*
* @brief   Set PTP V2 domain Id.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note PTP V2 Header contains 8-bit domain Id.
*       4 domain Id values mapped to domainNum 0-3
*       all other domain Id values mapped to default domain
*
*/
static CMD_STATUS wrCpssDxChPtpDomainV2IdTableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS    rc;
    GT_U8        devNum;
    GT_U32       domainNum;
    GT_U32       domainId;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_U8)inArgs[0];
    domainNum       = (GT_U32)inFields[0];
    domainId        = (GT_U32)inFields[1];

    /* call cpss api function */
    rc = cpssDxChPtpDomainV2IdSet(
        devNum, domainNum, domainId);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, rc, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpDomainV2IdTableGet_util function
* @endinternal
*
* @brief   Get PTP V2 domain Id.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note PTP V2 Header contains 8-bit domain Id.
*       4 domain Id values mapped to domainNum 0-3
*       all other domain Id values mapped to default domain
*
*/
static CMD_STATUS wrCpssDxChPtpDomainV2IdTableGet_util
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32      numFields,
    OUT GT_8       outArgs[CMD_MAX_BUFFER],
    IN  GT_BOOL    isGetFirst
)
{
    GT_STATUS      rc;
    GT_U8          devNum;
    /* variables initialised only to prevent compiler warnings */
    static GT_U32  domainNum = 0;
    GT_U32         domainId;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                            = (GT_U8)inArgs[0];

    if (isGetFirst == GT_TRUE)
    {
        domainNum = 0;
    }
    else
    {
        domainNum ++;
    }

    if (domainNum > 3)
    {
        /* no entries more */
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    rc = cpssDxChPtpDomainV2IdGet(
        devNum, domainNum, &domainId);
    if (rc != GT_OK)
    {
        galtisOutput(outArgs, rc, "cpssDxChPtpDomainV2IdGet");
        return CMD_OK;
    }

    /* pack and output table fields */
    fieldOutput(
        "%d%d", domainNum, domainId);
    galtisOutput(outArgs, rc, "%f");

    return CMD_OK;
}

static CMD_STATUS wrCpssDxChPtpDomainV2IdTableGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssDxChPtpDomainV2IdTableGet_util(
        inArgs, inFields, numFields, outArgs, GT_TRUE/*isGetFirst*/);
}

static CMD_STATUS wrCpssDxChPtpDomainV2IdTableGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssDxChPtpDomainV2IdTableGet_util(
        inArgs, inFields, numFields, outArgs, GT_FALSE/*isGetFirst*/);
}

/**
* @internal wrCpssDxChPtpEgressDomainTableSet function
* @endinternal
*
* @brief   Set Egress Domain Table Entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpEgressDomainTableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                              rc;
    GT_U8                                  devNum;
    GT_PHYSICAL_PORT_NUM                   portNum;
    GT_U32                                 domainNum;
    CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC  entry;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                               = (GT_U8)inArgs[0];
    portNum                              = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    domainNum                            = (GT_U32)inFields[0];
    entry.ptpOverEhernetTsEnable         = (GT_BOOL)inFields[1];
    entry.ptpOverUdpIpv4TsEnable         = (GT_BOOL)inFields[2];
    entry.ptpOverUdpIpv6TsEnable         = (GT_BOOL)inFields[3];
    entry.messageTypeTsEnableBmp         = (GT_U32)inFields[4];
    entry.transportSpecificTsEnableBmp   = (GT_U32)inFields[5];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    rc = cpssDxChPtpEgressDomainTableSet(
        devNum, portNum, domainNum, &entry);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, rc, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpEgressDomainTableGet_util function
* @endinternal
*
* @brief   Get Egress Domain Table Entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpEgressDomainTableGet_util
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32      numFields,
    OUT GT_8       outArgs[CMD_MAX_BUFFER],
    IN  GT_BOOL    isGetFirst
)
{
    GT_STATUS                              rc;
    GT_U8                                  devNum;
    GT_PHYSICAL_PORT_NUM                   portNum;
    /* variables initialised only to prevent compiler warnings */
    static GT_U32                          domainNum = 0;
    CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC  entry;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum      = (GT_U8)inArgs[0];
    portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    if (isGetFirst == GT_TRUE)
    {
        domainNum = 0;
    }
    else
    {
        domainNum ++;
    }

    if (domainNum > 4)
    {
        /* no entries more */
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    rc = cpssDxChPtpEgressDomainTableGet(
        devNum, portNum, domainNum, &entry);
    if (rc != GT_OK)
    {
        galtisOutput(outArgs, rc, "cpssDxChPtpEgressDomainTableGet");
        return CMD_OK;
    }

    /* pack and output table fields */
    fieldOutput(
        "%d%d%d%d%d%d", domainNum,
        entry.ptpOverEhernetTsEnable,
        entry.ptpOverUdpIpv4TsEnable,
        entry.ptpOverUdpIpv6TsEnable,
        entry.messageTypeTsEnableBmp,
        entry.transportSpecificTsEnableBmp);
    galtisOutput(outArgs, rc, "%f");

    return CMD_OK;
}

static CMD_STATUS wrCpssDxChPtpEgressDomainTableGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssDxChPtpEgressDomainTableGet_util(
        inArgs, inFields, numFields, outArgs, GT_TRUE/*isGetFirst*/);
}

static CMD_STATUS wrCpssDxChPtpEgressDomainTableGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssDxChPtpEgressDomainTableGet_util(
        inArgs, inFields, numFields, outArgs, GT_FALSE/*isGetFirst*/);
}

/**
* @internal wrCpssDxChPtpIngressPacketCheckingModeTableSet function
* @endinternal
*
* @brief   Set PTP packet Ingress Checking Mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum, messageType or command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpIngressPacketCheckingModeTableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                rc;
    GT_U8                                    devNum;
    GT_U32                                   domainNum;
    GT_U32                                   messageType;
    CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_ENT  checkingMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum        = (GT_U8)inArgs[0];
    domainNum     = (GT_U32)inArgs[1];
    messageType   = (GT_U32)inFields[0];
    checkingMode  = (CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_ENT)inFields[1];

    /* call cpss api function */
    rc = cpssDxChPtpIngressPacketCheckingModeSet(
        devNum, domainNum, messageType, checkingMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, rc, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpIngressPacketCheckingModeTableGet_util function
* @endinternal
*
* @brief   Get PTP packet Ingress Checking Mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or messageType
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on bad value found in HW
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpIngressPacketCheckingModeTableGet_util
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32      numFields,
    OUT GT_8       outArgs[CMD_MAX_BUFFER],
    IN  GT_BOOL    isGetFirst
)
{
    GT_STATUS                                rc;
    GT_U8                                    devNum;
    GT_U32                                   domainNum;
    /* variables initialised only to prevent compiler warnings */
    static GT_U32                            messageType = 0;
    CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_ENT  checkingMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum      = (GT_U8)inArgs[0];
    domainNum   = (GT_U32)inArgs[1];

    if (isGetFirst == GT_TRUE)
    {
        messageType = 0;
    }
    else
    {
        messageType ++;
    }

    if (messageType > 15)
    {
        /* no entries more */
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    rc = cpssDxChPtpIngressPacketCheckingModeGet(
        devNum, domainNum, messageType, &checkingMode);
    if (rc != GT_OK)
    {
        galtisOutput(outArgs, rc, "cpssDxChPtpIngressPacketCheckingModeGet");
        return CMD_OK;
    }

    /* pack and output table fields */
    fieldOutput(
        "%d%d", messageType, checkingMode);
    galtisOutput(outArgs, rc, "%f");

    return CMD_OK;
}

static CMD_STATUS wrCpssDxChPtpIngressPacketCheckingModeTableGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssDxChPtpIngressPacketCheckingModeTableGet_util(
        inArgs, inFields, numFields, outArgs, GT_TRUE/*isGetFirst*/);
}

static CMD_STATUS wrCpssDxChPtpIngressPacketCheckingModeTableGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssDxChPtpIngressPacketCheckingModeTableGet_util(
        inArgs, inFields, numFields, outArgs, GT_FALSE/*isGetFirst*/);
}

/**
* @internal wrCpssDxChPtpTsIngressTimestampQueueTableGet function
* @endinternal
*
* @brief   Read Ingress Timestamp Queue Entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Queue read causes deleting the entry from the queue.
*       The next read will retrieve the next entry.
*
*/
static CMD_STATUS wrCpssDxChPtpTsIngressTimestampQueueTableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32      numFields,
    OUT GT_8       outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                            rc;
    GT_U8                                                devNum;
    GT_U32                                               queueNum;
    CPSS_DXCH_PTP_TS_INGRESS_TIMESTAMP_QUEUE_ENTRY_STC   tsQueueEntry;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum     = (GT_U8)inArgs[0];
    queueNum   = (GT_U32)inArgs[1];

    rc = cpssDxChPtpTsIngressTimestampQueueEntryRead(
        devNum, queueNum, &tsQueueEntry);
    if (rc != GT_OK)
    {
        galtisOutput(outArgs, rc, "cpssDxChPtpTsIngressTimestampQueueEntryRead");
        return CMD_OK;
    }

    if (tsQueueEntry.entryValid == GT_FALSE)
    {
        /* no entries more */
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* pack and output table fields */
    fieldOutput(
        "%d%d%d%d%d%d%d%d%d",
        tsQueueEntry.isPtpExeption,
        tsQueueEntry.packetFormat,
        tsQueueEntry.taiSelect,
        tsQueueEntry.todUpdateFlag,
        tsQueueEntry.messageType,
        tsQueueEntry.domainNum,
        tsQueueEntry.sequenceId,
        tsQueueEntry.timestamp,
        tsQueueEntry.portNum);
    galtisOutput(outArgs, rc, "%f");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTsEgressTimestampQueueTableGet function
* @endinternal
*
* @brief   Read Egress Timestamp Queue Entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Queue read causes deleting the entry from the queue.
*       The next read will retrieve the next entry.
*
*/
static CMD_STATUS wrCpssDxChPtpTsEgressTimestampQueueTableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32      numFields,
    OUT GT_8       outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                            rc;
    GT_U8                                                devNum;
    GT_U32                                               queueNum;
    CPSS_DXCH_PTP_TS_EGRESS_TIMESTAMP_QUEUE_ENTRY_STC    tsQueueEntry;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum     = (GT_U8)inArgs[0];
    queueNum   = (GT_U32)inArgs[1];

    rc = cpssDxChPtpTsEgressTimestampQueueEntryRead(
        devNum, queueNum, &tsQueueEntry);
    if (rc != GT_OK)
    {
        galtisOutput(outArgs, rc, "cpssDxChPtpTsEgressTimestampQueueEntryRead");
        return CMD_OK;
    }

    if (tsQueueEntry.entryValid == GT_FALSE)
    {
        /* no entries more */
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* pack and output table fields */
    fieldOutput(
        "%d%d%d%d%d%d%d",
        tsQueueEntry.isPtpExeption,
        tsQueueEntry.packetFormat,
        tsQueueEntry.messageType,
        tsQueueEntry.domainNum,
        tsQueueEntry.sequenceId,
        tsQueueEntry.queueEntryId,
        tsQueueEntry.portNum);
    galtisOutput(outArgs, rc, "%f");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTaiPtpPulseIterfaceSet function
* @endinternal
*
* @brief   Set TAI PTP pulse interface.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note PTP pulse interface used for generating/receiving discrete pulses
*       that trigger a time-related operation such as Update or Capture.
*
*/
static CMD_STATUS wrCpssDxChPtpTaiPtpPulseIterfaceSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DIRECTION_ENT                  pulseInterfaceDirection;
    CPSS_DXCH_PTP_TAI_NUMBER_ENT        taiNumber;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    pulseInterfaceDirection = (CPSS_DIRECTION_ENT)inArgs[1];
    taiNumber = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssDxChPtpTaiPtpPulseIterfaceSet(devNum,
                                               pulseInterfaceDirection,
                                               taiNumber);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTaiPtpPulseIterfaceGet function
* @endinternal
*
* @brief   Get TAI PTP pulse interface.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note PTP pulse interface used for generating/receiving discrete pulses
*       that trigger a time-related operation such as Update or Capture.
*
*/
static CMD_STATUS wrCpssDxChPtpTaiPtpPulseIterfaceGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DIRECTION_ENT                  pulseInterfaceDirection;
    CPSS_DXCH_PTP_TAI_NUMBER_ENT        taiNumber;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPtpTaiPtpPulseIterfaceGet(devNum,
                                               &pulseInterfaceDirection,
                                               &taiNumber);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", pulseInterfaceDirection, taiNumber);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTaiPtpClockInterfaceSet function
* @endinternal
*
* @brief   Set TAI PTP clock interface.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note PTP clock interface used for generating/receiving a periodic pulse
*       such as 1 PPS signal.
*
*/
static CMD_STATUS wrCpssDxChPtpTaiPtpClockInterfaceSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_PTP_TAI_NUMBER_ENT        taiNumber;
    CPSS_DIRECTION_ENT                  clockInterfaceDirection;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    taiNumber = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[1];
    clockInterfaceDirection = (CPSS_DIRECTION_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssDxChPtpTaiPtpClockInterfaceSet(devNum,
                                                taiNumber,
                                                clockInterfaceDirection);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTaiPtpClockInterfaceGet function
* @endinternal
*
* @brief   Get TAI PTP clock interface.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note PTP clock interface used for generating/receiving a periodic pulse
*       such as 1 PPS signal.
*
*/
static CMD_STATUS wrCpssDxChPtpTaiPtpClockInterfaceGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_PTP_TAI_NUMBER_ENT        taiNumber;
    CPSS_DIRECTION_ENT                  clockInterfaceDirection;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    taiNumber = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPtpTaiPtpClockInterfaceGet(devNum,
                                                taiNumber,
                                                &clockInterfaceDirection);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", clockInterfaceDirection);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTaiPClkOutputInterfaceSet function
* @endinternal
*
* @brief   Set TAI PTP PClk or Sync-E recovered clock output interface.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTaiPClkOutputInterfaceSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;
    GT_U8                                       devNum;
    CPSS_DXCH_PTP_TAI_PCLK_RCVR_CLK_MODE_ENT    pclkRcvrClkMode;
    CPSS_DXCH_PTP_TAI_NUMBER_ENT                taiNumber;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    pclkRcvrClkMode = (CPSS_DXCH_PTP_TAI_PCLK_RCVR_CLK_MODE_ENT)inArgs[1];
    taiNumber = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssDxChPtpTaiPClkOutputInterfaceSet(devNum, pclkRcvrClkMode, taiNumber);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTaiPClkOutputInterfaceGet function
* @endinternal
*
* @brief   Get TAI PTP Pclock or Sync-E recovered clock output interface.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTaiPClkOutputInterfaceGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;
    GT_U8                                       devNum;
    CPSS_DXCH_PTP_TAI_PCLK_RCVR_CLK_MODE_ENT    pclkRcvrClkMode;
    CPSS_DXCH_PTP_TAI_NUMBER_ENT                taiNumber;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPtpTaiPClkOutputInterfaceGet(devNum,
                                                  &pclkRcvrClkMode,
                                                  &taiNumber);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", pclkRcvrClkMode, taiNumber);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTaiInputClockSelectSet function
* @endinternal
*
* @brief   Configures input TAI clock’s selection.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTaiInputClockSelectSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;
    GT_U8                                       devNum;
    CPSS_DXCH_PTP_TAI_NUMBER_ENT                taiNumber;
    CPSS_DXCH_PTP_TAI_CLOCK_SELECT_ENT          clockSelect;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    taiNumber = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[1];
    clockSelect = (CPSS_DXCH_PTP_TAI_CLOCK_SELECT_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssDxChPtpTaiInputClockSelectSet(devNum, taiNumber, clockSelect);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTaiInputClockSelectGet function
* @endinternal
*
* @brief   Get input TAI clock’s selection.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTaiInputClockSelectGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;
    GT_U8                                       devNum;
    CPSS_DXCH_PTP_TAI_NUMBER_ENT                taiNumber;
    CPSS_DXCH_PTP_TAI_CLOCK_SELECT_ENT          clockSelect;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    taiNumber = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPtpTaiInputClockSelectGet(devNum, taiNumber, &clockSelect);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", clockSelect);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpTaiTodCounterFunctionAllTriggerSet function
* @endinternal
*
* @brief   Triggeres TOD update operation for all TAI units.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpTaiTodCounterFunctionAllTriggerSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;
    GT_U8                                       devNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChPtpTaiTodCounterFunctionAllTriggerSet(devNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpPortUnitResetSet function
* @endinternal
*
* @brief   Reset/unreset Port PTP unit.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpPortUnitResetSet
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
    GT_BOOL                         resetTxUnit;
    GT_BOOL                         resetRxUnit;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum       = (GT_U8)inArgs[0];
    portNum      = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    resetTxUnit  = (GT_BOOL)inArgs[2];
    resetRxUnit  = (GT_BOOL)inArgs[3];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPtpPortUnitResetSet(
        devNum, portNum, resetTxUnit, resetRxUnit);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpPortUnitResetGet function
* @endinternal
*
* @brief   Get Reset/unreset Port PTP unit state.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpPortUnitResetGet
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
    GT_BOOL                         resetTxUnit;
    GT_BOOL                         resetRxUnit;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum       = (GT_U8)inArgs[0];
    portNum      = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPtpPortUnitResetGet(
        devNum, portNum, &resetTxUnit, &resetRxUnit);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", resetTxUnit, resetRxUnit);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpPortTxPipeStatusDelaySet function
* @endinternal
*
* @brief   Set PTP Tx Pipe Status Delay.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_OUT_OF_RANGE          - on out of range parameter value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpPortTxPipeStatusDelaySet
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
    GT_U32                          delay;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum       = (GT_U8)inArgs[0];
    portNum      = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    delay        = (GT_U32)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPtpPortTxPipeStatusDelaySet(
        devNum, portNum, delay);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPtpPortTxPipeStatusDelayGet function
* @endinternal
*
* @brief   Get PTP Tx Pipe Status Delay.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPtpPortTxPipeStatusDelayGet
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
    GT_U32                          delay;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPtpPortTxPipeStatusDelayGet(
        devNum, portNum, &delay);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", delay);

    return CMD_OK;
}

/**
* @internal wrcpssDxChPtpTaiPulseInModeSet function
* @endinternal
*
* @brief  Set PTP PulseIn Mode.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrcpssDxChPtpTaiPulseInModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_PTP_TAI_NUMBER_ENT        taiNumber;
    CPSS_DXCH_PTP_TAI_PULSE_IN_MODE_ENT pulseMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    taiNumber = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[1];
    pulseMode = (CPSS_DXCH_PTP_TAI_PULSE_IN_MODE_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssDxChPtpTaiPulseInModeSet(
        devNum, taiNumber, pulseMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrcpssDxChPtpTaiPulseInModeGet function
* @endinternal
*
* @brief  Get PTP PulseIn Mode.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrcpssDxChPtpTaiPulseInModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_DXCH_PTP_TAI_NUMBER_ENT        taiNumber;
    CPSS_DXCH_PTP_TAI_PULSE_IN_MODE_ENT pulseMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    taiNumber = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPtpTaiPulseInModeGet(
        devNum, taiNumber, &pulseMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", pulseMode);

    return CMD_OK;
}


/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
    {"cpssDxChPtpEtherTypeSet",
        &wrCpssDxChPtpEtherTypeSet,
        3, 0},

    {"cpssDxChPtpEtherTypeGet",
        &wrCpssDxChPtpEtherTypeGet,
        2, 0},

    {"cpssDxChPtpUdpDestPortsSet",
        &wrCpssDxChPtpUdpDestPortsSet,
        3, 0},

    {"cpssDxChPtpUdpDestPortsGet",
        &wrCpssDxChPtpUdpDestPortsGet,
        2, 0},

    {"cpssDxChPtpMessageTypeCmdSet",
        &wrCpssDxChPtpMessageTypeCmdSet,
        5, 0},

    {"cpssDxChPtpMessageTypeCmdGet",
        &wrCpssDxChPtpMessageTypeCmdGet,
        4, 0},

    {"cpssDxChPtpCpuCodeBaseSet",
        &wrCpssDxChPtpCpuCodeBaseSet,
        2, 0},

    {"cpssDxChPtpCpuCodeBaseGet",
        &wrCpssDxChPtpCpuCodeBaseGet,
        1, 0},

    {"cpssDxChPtpTodCounterFunctionSet",
        &wrCpssDxChPtpTodCounterFunctionSet,
        3, 0},

    {"cpssDxChPtpTodCounterFunctionGet",
        &wrCpssDxChPtpTodCounterFunctionGet,
        2, 0},

    {"cpssDxChPtpTodCounterFunctionTriggerSet",
        &wrCpssDxChPtpTodCounterFunctionTriggerSet,
        2, 0},

    {"cpssDxChPtpTodCounterFunctionTriggerGet",
        &wrCpssDxChPtpTodCounterFunctionTriggerGet,
        2, 0},

    {"cpssDxChPtpTodCounterShadowSet",
        &wrCpssDxChPtpTodCounterShadowSet,
        5, 0},

    {"cpssDxChPtpTodCounterShadowGet",
        &wrCpssDxChPtpTodCounterShadowGet,
        2, 0},

    {"cpssDxChPtpTodCounterGet",
        &wrCpssDxChPtpTodCounterGet,
        2, 0},

    {"cpssDxChPtpPortTimeStampEnableSet",
        &wrCpssDxChPtpPortTimeStampEnableSet,
        4, 0},

    {"cpssDxChPtpPortTimeStampEnableGet",
        &wrCpssDxChPtpPortTimeStampEnableGet,
        3, 0},

    {"cpssDxChPtpEthernetTimeStampEnableSet",
        &wrCpssDxChPtpEthernetTimeStampEnableSet,
        3, 0},

    {"cpssDxChPtpEthernetTimeStampEnableGet",
        &wrCpssDxChPtpEthernetTimeStampEnableGet,
        2, 0},

    {"cpssDxChPtpUdpTimeStampEnableSet",
        &wrCpssDxChPtpUdpTimeStampEnableSet,
        4, 0},

    {"cpssDxChPtpUdpTimeStampEnableGet",
        &wrCpssDxChPtpUdpTimeStampEnableGet,
        3, 0},

    {"cpssDxChPtpMessageTypeTimeStampEnableSet",
        &wrCpssDxChPtpMessageTypeTimeStampEnableSet,
        4, 0},

    {"cpssDxChPtpMessageTypeTimeStampEnableGet",
        &wrCpssDxChPtpMessageTypeTimeStampEnableGet,
        3, 0},

    {"cpssDxChPtpTransportSpecificCheckEnableSet",
        &wrCpssDxChPtpTransportSpecificCheckEnableSet,
        3, 0},

    {"cpssDxChPtpTransportSpecificCheckEnableGet",
        &wrCpssDxChPtpTransportSpecificCheckEnableGet,
        2, 0},

    {"cpssDxChPtpTransportSpecificTimeStampEnableSet",
        &wrCpssDxChPtpTransportSpecificTimeStampEnableSet,
        4, 0},

    {"cpssDxChPtpTransportSpecificTimeStampEnableGet",
        &wrCpssDxChPtpTransportSpecificTimeStampEnableGet,
        3, 0},

    {"cpssDxChPtpTimestampEntryGet",
        &wrCpssDxChPtpTimestampEntryGet,
        2, 0},

    /* new SIP5 commands */

    {"cpssDxChPtpTodCounterFunctionSet_Tai",
        &wrCpssDxChPtpTodCounterFunctionSet_Tai,
        6, 0},

    {"cpssDxChPtpTodCounterFunctionGet_Tai",
        &wrCpssDxChPtpTodCounterFunctionGet_Tai,
        5, 0},

    {"cpssDxChPtpTodCounterFunctionTriggerSet_Tai",
        &wrCpssDxChPtpTodCounterFunctionTriggerSet_Tai,
        5, 0},

    {"cpssDxChPtpTodCounterFunctionTriggerGet_Tai",
        &wrCpssDxChPtpTodCounterFunctionTriggerGet_Tai,
        5, 0},

    {"cpssDxChPtpTaiClockModeSet",
        &wrCpssDxChPtpTaiClockModeSet,
        5, 0},

    {"cpssDxChPtpTaiClockModeGet",
        &wrCpssDxChPtpTaiClockModeGet,
        4, 0},

    {"cpssDxChPtpTaiInternalClockGenerateEnableSet",
        &wrCpssDxChPtpTaiInternalClockGenerateEnableSet,
        5, 0},

    {"cpssDxChPtpTaiInternalClockGenerateEnableGet",
        &wrCpssDxChPtpTaiInternalClockGenerateEnableGet,
        4, 0},

    {"cpssDxChPtpTaiPtpPClockDriftAdjustEnableSet",
        &wrCpssDxChPtpTaiPtpPClockDriftAdjustEnableSet,
        5, 0},

    {"cpssDxChPtpTaiPtpPClockDriftAdjustEnableGet",
        &wrCpssDxChPtpTaiPtpPClockDriftAdjustEnableGet,
        4, 0},

    {"cpssDxChPtpTaiCaptureOverrideEnableSet",
        &wrCpssDxChPtpTaiCaptureOverrideEnableSet,
        5, 0},

    {"cpssDxChPtpTaiCaptureOverrideEnableGet",
        &wrCpssDxChPtpTaiCaptureOverrideEnableGet,
        4, 0},

    {"cpssDxChPtpTaiInputTriggersCountEnableSet",
        &wrCpssDxChPtpTaiInputTriggersCountEnableSet,
        5, 0},

    {"cpssDxChPtpTaiInputTriggersCountEnableGet",
        &wrCpssDxChPtpTaiInputTriggersCountEnableGet,
        4, 0},

    {"cpssDxChPtpTaiExternalPulseWidthSet",
        &wrCpssDxChPtpTaiExternalPulseWidthSet,
        5, 0},

    {"cpssDxChPtpTaiExternalPulseWidthGet",
        &wrCpssDxChPtpTaiExternalPulseWidthGet,
        4, 0},

    {"cpssDxChPtpTaiTodSet",
        &wrCpssDxChPtpTaiTodSet,
        9, 0},

    {"cpssDxChPtpTaiTodGet",
        &wrCpssDxChPtpTaiTodGet,
        5, 0},

    {"cpssDxChPtpTaiOutputTriggerEnableSet",
        &wrCpssDxChPtpTaiOutputTriggerEnableSet,
        5, 0},

    {"cpssDxChPtpTaiOutputTriggerEnableGet",
        &wrCpssDxChPtpTaiOutputTriggerEnableGet,
        4, 0},

    {"cpssDxChPtpTaiTodStepSet",
        &wrCpssDxChPtpTaiTodStepSet,
        6, 0},

    {"cpssDxChPtpTaiTodStepGet",
        &wrCpssDxChPtpTaiTodStepGet,
        4, 0},

    {"cpssDxChPtpTaiPulseDelaySet",
        &wrCpssDxChPtpTaiPulseDelaySet,
        5, 0},

    {"cpssDxChPtpTaiPulseDelayGet",
        &wrCpssDxChPtpTaiPulseDelayGet,
        4, 0},

    {"cpssDxChPtpTaiClockDelaySet",
        &wrCpssDxChPtpTaiClockDelaySet,
        5, 0},

    {"cpssDxChPtpTaiClockDelayGet",
        &wrCpssDxChPtpTaiClockDelayGet,
        4, 0},

    {"cpssDxChPtpTaiFractionalNanosecondDriftSet",
        &wrCpssDxChPtpTaiFractionalNanosecondDriftSet,
        5, 0},

    {"cpssDxChPtpTaiFractionalNanosecondDriftGet",
        &wrCpssDxChPtpTaiFractionalNanosecondDriftGet,
        4, 0},

    {"cpssDxChPtpTaiPClockCycleSet",
        &wrCpssDxChPtpTaiPClockCycleSet,
        5, 0},

    {"cpssDxChPtpTaiPClockCycleGet",
        &wrCpssDxChPtpTaiPClockCycleGet,
        4, 0},

    {"cpssDxChPtpTaiClockCycleSet",
        &wrCpssDxChPtpTaiClockCycleSet,
        6, 0},

    {"cpssDxChPtpTaiClockCycleGet",
        &wrCpssDxChPtpTaiClockCycleGet,
        4, 0},

    {"cpssDxChPtpTaiTodCaptureStatusSet",
        &wrCpssDxChPtpTaiTodCaptureStatusSet,
        6, 0},

    {"cpssDxChPtpTaiTodCaptureStatusGet",
        &wrCpssDxChPtpTaiTodCaptureStatusGet,
        5, 0},

    {"cpssDxChPtpTaiTodUpdateCounterGet",
        &wrCpssDxChPtpTaiTodUpdateCounterGet,
        4, 0},

    {"cpssDxChPtpTaiIncomingTriggerCounterSet",
        &wrCpssDxChPtpTaiIncomingTriggerCounterSet,
        5, 0},

    {"cpssDxChPtpTaiIncomingTriggerCounterGet",
        &wrCpssDxChPtpTaiIncomingTriggerCounterGet,
        4, 0},

    {"cpssDxChPtpTaiIncomingClockCounterSet",
        &wrCpssDxChPtpTaiIncomingClockCounterSet,
        6, 0},

    {"cpssDxChPtpTaiIncomingClockCounterGet",
        &wrCpssDxChPtpTaiIncomingClockCounterGet,
        4, 0},

    {"cpssDxChPtpTaiFrequencyDriftThesholdsSet",
        &wrCpssDxChPtpTaiFrequencyDriftThesholdsSet,
        6, 0},

    {"cpssDxChPtpTaiFrequencyDriftThesholdsGet",
        &wrCpssDxChPtpTaiFrequencyDriftThesholdsGet,
        4, 0},

    {"cpssDxChPtpTaiGracefulStepSet",
        &wrCpssDxChPtpTaiGracefulStepSet,
        5, 0},

    {"cpssDxChPtpTaiGracefulStepGet",
        &wrCpssDxChPtpTaiGracefulStepGet,
        4, 0},

    {"cpssDxChPtpTsuControlTableSet",
        &wrCpssDxChPtpTsuControlTableSet,
        3, 4},

    {"cpssDxChPtpTsuControlTableGetFirst",
        &wrCpssDxChPtpTsuControlTableGetFirst,
        3, 0},

    {"cpssDxChPtpTsuControlTableGetNext",
        &wrCpssDxChPtpTsuControlTableGetNext,
        3, 0},

    {"cpssDxChPtpTsuTxTimestampQueueTableGetFirst",
        &wrCpssDxChPtpTsuTxTimestampQueueTableGetFirst,
        3, 0},

    {"cpssDxChPtpTsuTxTimestampQueueTableGetNext",
        &wrCpssDxChPtpTsuTxTimestampQueueTableGet,
        3, 0},

    {"utilPtpTsuTxTimestampQueueTableReadLimitSet",
        &wrUtilPtpTsuTxTimestampQueueTableReadLimitSet,
        1, 0},

    {"cpssDxChPtpTsuCountersClear",
        &wrCpssDxChPtpTsuCountersClear,
        2, 0},

    {"cpssDxChPtpTsuPacketCouterGet",
        &wrCpssDxChPtpTsuPacketCouterGet,
        3, 0},

    {"cpssDxChPtpTsuNtpTimeOffsetSet",
        &wrCpssDxChPtpTsuNtpTimeOffsetSet,
        3, 0},

    {"cpssDxChPtpTsuNtpTimeOffsetGet",
        &wrCpssDxChPtpTsuNtpTimeOffsetGet,
        2, 0},

    {"cpssDxChPtpTsTagGlobalCfgSet",
        &wrCpssDxChPtpTsTagGlobalCfgSet,
        5, 0},

    {"cpssDxChPtpTsTagGlobalCfgGet",
        &wrCpssDxChPtpTsTagGlobalCfgGet,
        1, 0},

    {"cpssDxChPtpTsTagPortCfgTableSet",
        &wrCpssDxChPtpTsTagPortCfgTableSet,
        3, 4},

    {"cpssDxChPtpTsTagPortCfgTableGetFirst",
        &wrCpssDxChPtpTsTagPortCfgTableGetFirst,
        3, 0},

    {"cpssDxChPtpTsTagPortCfgTableGetNext",
        &wrCpssDxChPtpTsTagPortCfgTableGetNext,
        3, 0},

    {"cpssDxChPtpTsDelayIngressPortDelaySet",
        &wrCpssDxChPtpTsDelayIngressPortDelaySet,
        3, 0},

    {"cpssDxChPtpTsDelayIngressPortDelayGet",
        &wrCpssDxChPtpTsDelayIngressPortDelayGet,
        2, 0},

    {"cpssDxChPtpTsDelayIngressLinkDelaySet",
        &wrCpssDxChPtpTsDelayIngressLinkDelaySet,
        4, 0},

    {"cpssDxChPtpTsDelayIngressLinkDelayGet",
        &wrCpssDxChPtpTsDelayIngressLinkDelayGet,
        3, 0},

    {"cpssDxChPtpTsDelayEgressPipeDelaySet",
        &wrCpssDxChPtpTsDelayEgressPipeDelaySet,
        3, 0},

    {"cpssDxChPtpTsDelayEgressPipeDelayGet",
        &wrCpssDxChPtpTsDelayEgressPipeDelayGet,
        2, 0},

    {"cpssDxChPtpTsDelayEgressAsymmetryCorrectionSet",
        &wrCpssDxChPtpTsDelayEgressAsymmetryCorrectionSet,
        4, 0},

    {"cpssDxChPtpTsDelayEgressAsymmetryCorrectionGet",
        &wrCpssDxChPtpTsDelayEgressAsymmetryCorrectionGet,
        3, 0},

    {"cpssDxChPtpTsDelayIngressCorrFieldPiggybackEnSet",
        &wrCpssDxChPtpTsDelayIngressCorrFieldPiggybackEnableSet,
        3, 0},

    {"cpssDxChPtpTsDelayIngressCorrFieldPiggybackEnGet",
        &wrCpssDxChPtpTsDelayIngressCorrFieldPiggybackEnableGet,
        2, 0},

    {"cpssDxChPtpTsDelayEgressCorrFieldPiggybackEnSet",
        &wrCpssDxChPtpTsDelayEgressCorrFieldPiggybackEnableSet,
        3, 0},

    {"cpssDxChPtpTsDelayEgressCorrFieldPiggybackEnGet",
        &wrCpssDxChPtpTsDelayEgressCorrFieldPiggybackEnableGet,
        2, 0},

    {"cpssDxChPtpTsDelayEgressTimeCorrTaiSelectModeSet",
        &wrCpssDxChPtpTsDelayEgressTimeCorrTaiSelectModeSet,
        3, 0},

    {"cpssDxChPtpTsDelayEgressTimeCorrTaiSelectModeGet",
        &wrCpssDxChPtpTsDelayEgressTimeCorrTaiSelectModeGet,
        2, 0},

    {"cpssDxChPtpTsDelayEgressTimestampTaiSelectSet",
        &wrCpssDxChPtpTsDelayEgressTimestampTaiSelectSet,
        3, 0},

    {"cpssDxChPtpTsDelayEgressTimestampTaiSelectGet",
        &wrCpssDxChPtpTsDelayEgressTimestampTaiSelectGet,
        2, 0},

    {"cpssDxChPtpTsUdpChecksumUpdateModeSet",
        &wrCpssDxChPtpTsUdpChecksumUpdateModeSet,
        7, 0},

    {"cpssDxChPtpTsUdpChecksumUpdateModeGet",
        &wrCpssDxChPtpTsUdpChecksumUpdateModeGet,
        1, 0},

    {"cpssDxChPtpOverEthernetEnableSet",
        &wrCpssDxChPtpOverEthernetEnableSet,
        2, 0},

    {"cpssDxChPtpOverEthernetEnableGet",
        &wrCpssDxChPtpOverEthernetEnableGet,
        1, 0},

    {"cpssDxChPtpOverUdpEnableSet",
        &wrCpssDxChPtpOverUdpEnableSet,
        2, 0},

    {"cpssDxChPtpOverUdpEnableGet",
        &wrCpssDxChPtpOverUdpEnableGet,
        1, 0},

    {"cpssDxChPtpIngressExceptionCfgSet",
        &wrCpssDxChPtpIngressExceptionCfgSet,
        5, 0},

    {"cpssDxChPtpIngressExceptionCfgGet",
        &wrCpssDxChPtpIngressExceptionCfgGet,
        1, 0},

    {"cpssDxChPtpEgressExceptionCfgSet",
        &wrCpssDxChPtpEgressExceptionCfgSet,
        5, 0},

    {"cpssDxChPtpEgressExceptionCfgSet_1",
        &wrCpssDxChPtpEgressExceptionCfgSet_1,
        9, 0},

    {"cpssDxChPtpEgressExceptionCfgGet",
        &wrCpssDxChPtpEgressExceptionCfgGet,
        1, 0},

    {"cpssDxChPtpEgressExceptionCfgGet_1",
        &wrCpssDxChPtpEgressExceptionCfgGet_1,
        1, 0},

    {"cpssDxChPtpIngressExceptionCounterGet",
        &wrCpssDxChPtpIngressExceptionCounterGet,
        1, 0},

    {"cpssDxChPtpEgressExceptionCountersGet",
        &wrCpssDxChPtpEgressExceptionCountersGet,
        1, 0},

    {"cpssDxChPtpTsMessageTypeToQueueIdMapSet",
        &wrCpssDxChPtpTsMessageTypeToQueueIdMapSet,
        2, 0},

    {"cpssDxChPtpTsMessageTypeToQueueIdMapGet",
        &wrCpssDxChPtpTsMessageTypeToQueueIdMapGet,
        1, 0},

    {"cpssDxChPtpTsQueuesEntryOverrideEnableSet",
        &wrCpssDxChPtpTsQueuesEntryOverrideEnableSet,
        2, 0},

    {"cpssDxChPtpTsQueuesEntryOverrideEnableGet",
        &wrCpssDxChPtpTsQueuesEntryOverrideEnableGet,
        1, 0},

    {"cpssDxChPtpTsQueuesSizeSet",
        &wrCpssDxChPtpTsQueuesSizeSet,
        2, 0},

    {"cpssDxChPtpTsQueuesSizeGet",
        &wrCpssDxChPtpTsQueuesSizeGet,
        1, 0},

    {"cpssDxChPtpTsDebugQueuesEntryIdsClear",
        &wrCpssDxChPtpTsDebugQueuesEntryIdsClear,
        1, 0},

    {"cpssDxChPtpTsDebugQueuesEntryIdsGet",
        &wrCpssDxChPtpTsDebugQueuesEntryIdsGet,
        1, 0},

    {"cpssDxChPtpTsNtpTimeOffsetSet",
        &wrCpssDxChPtpTsNtpTimeOffsetSet,
        2, 0},

    {"cpssDxChPtpTsNtpTimeOffsetGet",
        &wrCpssDxChPtpTsNtpTimeOffsetGet,
        1, 0},

    {"cpssDxChPtpMessageTypeCmdTableSet",
        &wrCpssDxChPtpMessageTypeCmdTableSet,
        3, 2},

    {"cpssDxChPtpMessageTypeCmdTableGetFirst",
        &wrCpssDxChPtpMessageTypeCmdTableGetFirst,
        3, 0},

    {"cpssDxChPtpMessageTypeCmdTableGetNext",
        &wrCpssDxChPtpMessageTypeCmdTableGetNext,
        3, 0},

    {"cpssDxChPtpTsCfgTableSet",
        &wrCpssDxChPtpTsCfgTableSet,
        3, 12},

    {"cpssDxChPtpTsCfgTableGetFirst",
        &wrCpssDxChPtpTsCfgTableGetFirst,
        3, 0},

    {"cpssDxChPtpTsCfgTableGetNext",
        &wrCpssDxChPtpTsCfgTableGetNext,
        3, 0},

    {"cpssDxChPtpTsLocalActionTableSet",
        &wrCpssDxChPtpTsLocalActionTableSet,
        3, 4},

    {"cpssDxChPtpTsLocalActionTableGetFirst",
        &wrCpssDxChPtpTsLocalActionTableGetFirst,
        3, 0},

    {"cpssDxChPtpTsLocalActionTableGetNext",
        &wrCpssDxChPtpTsLocalActionTableGetNext,
        3, 0},

    {"cpssDxChPtpDomainModeTableSet",
        &wrCpssDxChPtpDomainModeTableSet,
        1, 2},

    {"cpssDxChPtpDomainModeTableGetFirst",
        &wrCpssDxChPtpDomainModeTableGetFirst,
        1, 0},

    {"cpssDxChPtpDomainModeTableGetNext",
        &wrCpssDxChPtpDomainModeTableGetNext,
        1, 0},

    {"cpssDxChPtpDomainV1IdTableSet",
        &wrCpssDxChPtpDomainV1IdTableSet,
        1, 5},

    {"cpssDxChPtpDomainV1IdTableGetFirst",
        &wrCpssDxChPtpDomainV1IdTableGetFirst,
        1, 0},

    {"cpssDxChPtpDomainV1IdTableGetNext",
        &wrCpssDxChPtpDomainV1IdTableGetNext,
        1, 0},

    {"cpssDxChPtpDomainV2IdTableSet",
        &wrCpssDxChPtpDomainV2IdTableSet,
        1, 2},

    {"cpssDxChPtpDomainV2IdTableGetFirst",
        &wrCpssDxChPtpDomainV2IdTableGetFirst,
        1, 0},

    {"cpssDxChPtpDomainV2IdTableGetNext",
        &wrCpssDxChPtpDomainV2IdTableGetNext,
        1, 0},

    {"cpssDxChPtpEgressDomainTableSet",
        &wrCpssDxChPtpEgressDomainTableSet,
        2, 6},

    {"cpssDxChPtpEgressDomainTableGetFirst",
        &wrCpssDxChPtpEgressDomainTableGetFirst,
        2, 0},

    {"cpssDxChPtpEgressDomainTableGetNext",
        &wrCpssDxChPtpEgressDomainTableGetNext,
        2, 0},

    {"cpssDxChPtpIngressPacketCheckingModeTblSet",
        &wrCpssDxChPtpIngressPacketCheckingModeTableSet,
        2, 2},

    {"cpssDxChPtpIngressPacketCheckingModeTblGetFirst",
        &wrCpssDxChPtpIngressPacketCheckingModeTableGetFirst,
        2, 0},

    {"cpssDxChPtpIngressPacketCheckingModeTblGetNext",
        &wrCpssDxChPtpIngressPacketCheckingModeTableGetNext,
        2, 0},

    {"cpssDxChPtpTsIngressTimestampQueueTableGetFirst",
        &wrCpssDxChPtpTsIngressTimestampQueueTableGet,
        2, 0},

    {"cpssDxChPtpTsIngressTimestampQueueTableGetNext",
        &wrCpssDxChPtpTsIngressTimestampQueueTableGet,
        2, 0},

    {"cpssDxChPtpTsEgressTimestampQueueTableGetFirst",
        &wrCpssDxChPtpTsEgressTimestampQueueTableGet,
        2, 0},

    {"cpssDxChPtpTsEgressTimestampQueueTableGetNext",
        &wrCpssDxChPtpTsEgressTimestampQueueTableGet,
        2, 0},

    {"cpssDxChPtpTaiPtpPulseIterfaceSet",
        &wrCpssDxChPtpTaiPtpPulseIterfaceSet,
        3, 0},

    {"cpssDxChPtpTaiPtpPulseIterfaceGet",
        &wrCpssDxChPtpTaiPtpPulseIterfaceGet,
        1, 0},

    {"cpssDxChPtpTaiPtpClockInterfaceSet",
        &wrCpssDxChPtpTaiPtpClockInterfaceSet,
        3, 0},

    {"cpssDxChPtpTaiPtpClockInterfaceGet",
        &wrCpssDxChPtpTaiPtpClockInterfaceGet,
        2, 0},

    {"cpssDxChPtpTaiPClkOutputInterfaceSet",
        &wrCpssDxChPtpTaiPClkOutputInterfaceSet,
        3, 0},

    {"cpssDxChPtpTaiPClkOutputInterfaceGet",
        &wrCpssDxChPtpTaiPClkOutputInterfaceGet,
        1, 0},

    {"cpssDxChPtpTaiInputClockSelectSet",
        &wrCpssDxChPtpTaiInputClockSelectSet,
        3, 0},

    {"cpssDxChPtpTaiInputClockSelectGet",
        &wrCpssDxChPtpTaiInputClockSelectGet,
        2, 0},

    {"cpssDxChPtpTaiTodCounterFunctionAllTriggerSet",
        &wrCpssDxChPtpTaiTodCounterFunctionAllTriggerSet,
        1, 0},

    {"cpssDxChPtpPortUnitResetSet",
        &wrCpssDxChPtpPortUnitResetSet,
        4, 0},

    {"cpssDxChPtpPortUnitResetGet",
        &wrCpssDxChPtpPortUnitResetGet,
        2, 0},

    {"cpssDxChPtpPortTxPipeStatusDelaySet",
        &wrCpssDxChPtpPortTxPipeStatusDelaySet,
        3, 0},

    {"cpssDxChPtpPortTxPipeStatusDelayGet",
        &wrCpssDxChPtpPortTxPipeStatusDelayGet,
        2, 0},

    {"cpssDxChPtpTaiPulseInModeSet",
        &wrcpssDxChPtpTaiPulseInModeSet,
        3, 0},

    {"cpssDxChPtpTaiPulseInModeGet",
        &wrcpssDxChPtpTaiPulseInModeGet,
        2, 0},

    {"prvCpssDxChPtpTaiDebugInstanceSet",
        &wrprvCpssDxChPtpTaiDebugInstanceSet,
        4, 0}
};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssDxChPtp function
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
CMD_STATUS cmdLibInitCpssDxChPtp
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}



