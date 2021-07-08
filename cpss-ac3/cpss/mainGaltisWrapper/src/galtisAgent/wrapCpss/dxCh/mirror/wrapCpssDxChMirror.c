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
* @file wrapCpssDxChMirror.c
*
* @brief Wrapper functions for Mirror cpss.dx Chita2 functions
*
* @version   20
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>


/* Feature specific includes. */
#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/mirror/cpssDxChMirror.h>


/**
* @internal wrCpssDxChMirrorAnalyzerVlanTagEnable function
* @endinternal
*
* @brief   Enable/Disable Analyzer port adding additional VLAN Tag to mirrored
*         packets. The content of this VLAN tag is configured
*         by gmirrSetAnalyzerVlanTagConfig.
*
* @note   APPLICABLE DEVICES:      All DxCh2 devices
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_SUPPORTED         - not supported device type.
* @retval GT_BAD_PARAM             - wrong device or port number.
* @retval GT_HW_ERROR              - on writing to HW error.
*/
static CMD_STATUS wrCpssDxChMirrorAnalyzerVlanTagEnable
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8       devNum;
    GT_PORT_NUM       portNum;
    GT_BOOL     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum =  (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];
    enable =  (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChMirrorAnalyzerVlanTagEnable(devNum, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrorAnalyzerVlanTagEnableGet function
* @endinternal
*
* @brief   Get Analyzer port VLAN Tag to mirrored packets mode.
*
* @note   APPLICABLE DEVICES:      All DxCh2 devices
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_SUPPORTED         - not supported device type.
* @retval GT_BAD_PARAM             - wrong device or port number.
* @retval GT_HW_ERROR              - on writing to HW error.
*/
static CMD_STATUS wrCpssDxChMirrorAnalyzerVlanTagEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8       devNum;
    GT_PORT_NUM       portNum;
    GT_BOOL     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum =  (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChMirrorAnalyzerVlanTagEnableGet(devNum, portNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrorTxAnalyzerVlanTagConfig function
* @endinternal
*
* @brief   Set global Tx mirroring VLAN tag details: EtherType, VID, VPT, CFI
*
* @note   APPLICABLE DEVICES:      All DxCh2 devices
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_SUPPORTED         - not supported device type.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
*
* @note Supported for 98DX2x5 devices only
*
*/
static CMD_STATUS wrCpssDxChMirrorTxAnalyzerVlanTagConfig
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8                                       devNum;
    CPSS_DXCH_MIRROR_ANALYZER_VLAN_TAG_CFG_STC  analyzerVlanTagConfig;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum =  (GT_U8)inArgs[0];
    analyzerVlanTagConfig.etherType = (GT_U16)inArgs[1];
    analyzerVlanTagConfig.vpt = (GT_U8)inArgs[2];
    analyzerVlanTagConfig.cfi = (GT_U8)inArgs[3];
    analyzerVlanTagConfig.vid = (GT_U16)inArgs[4];

    /* call cpss api function */
    result = cpssDxChMirrorTxAnalyzerVlanTagConfig(devNum,
                                                   &analyzerVlanTagConfig);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrorTxAnalyzerVlanTagConfigGet function
* @endinternal
*
* @brief   Get global Tx mirroring VLAN tag details: EtherType, VID, VPT, CFI
*
* @note   APPLICABLE DEVICES:      All DxCh2 devices
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_SUPPORTED         - not supported device type.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note Supported for 98DX2x5 devices only
*
*/
static CMD_STATUS wrCpssDxChMirrorTxAnalyzerVlanTagConfigGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8                                       devNum;
    CPSS_DXCH_MIRROR_ANALYZER_VLAN_TAG_CFG_STC  analyzerVlanTagConfig;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum =  (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChMirrorTxAnalyzerVlanTagConfigGet(devNum,
                                                   &analyzerVlanTagConfig);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d", analyzerVlanTagConfig.etherType,
                                      analyzerVlanTagConfig.vpt,
                                      analyzerVlanTagConfig.cfi,
                                      analyzerVlanTagConfig.vid);
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrorRxAnalyzerVlanTagConfig function
* @endinternal
*
* @brief   Set global Rx mirroring VLAN tag details: EtherType, VID, VPT, CFI
*
* @note   APPLICABLE DEVICES:      All DxCh2 devices
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_SUPPORTED         - not supported device type.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
*
* @note Supported for 98DX2x5 devices only
*
*/
static CMD_STATUS wrCpssDxChMirrorRxAnalyzerVlanTagConfig
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8                                       devNum;
    CPSS_DXCH_MIRROR_ANALYZER_VLAN_TAG_CFG_STC  analyzerVlanTagConfig;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum =  (GT_U8)inArgs[0];
    analyzerVlanTagConfig.etherType = (GT_U16)inArgs[1];
    analyzerVlanTagConfig.vpt = (GT_U8)inArgs[2];
    analyzerVlanTagConfig.cfi = (GT_U8)inArgs[3];
    analyzerVlanTagConfig.vid = (GT_U16)inArgs[4];

    /* call cpss api function */
    result = cpssDxChMirrorRxAnalyzerVlanTagConfig(devNum,
                                                   &analyzerVlanTagConfig);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrorRxAnalyzerVlanTagConfigGet function
* @endinternal
*
* @brief   Get global Rx mirroring VLAN tag details: EtherType, VID, VPT, CFI
*
* @note   APPLICABLE DEVICES:      All DxCh2 devices
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_SUPPORTED         - not supported device type.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note Supported for 98DX2x5 devices only
*
*/
static CMD_STATUS wrCpssDxChMirrorRxAnalyzerVlanTagConfigGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8                                       devNum;
    CPSS_DXCH_MIRROR_ANALYZER_VLAN_TAG_CFG_STC  analyzerVlanTagConfig;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum =  (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChMirrorRxAnalyzerVlanTagConfigGet(devNum,
                                                   &analyzerVlanTagConfig);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d", analyzerVlanTagConfig.etherType,
                                      analyzerVlanTagConfig.vpt,
                                      analyzerVlanTagConfig.cfi,
                                      analyzerVlanTagConfig.vid);
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrorTxAnalyzerPortSet function
* @endinternal
*
* @brief   Sets a port to be an analyzer port of tx mirrored
*         ports on all system pp's.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
*/
static CMD_STATUS wrCpssDxChMirrorTxAnalyzerPortSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8   dev;
    GT_PORT_NUM     analyzerPort;
    GT_HW_DEV_NUM   analyzerHwDev;
    CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC  interface;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev =  (GT_U8)inArgs[0];
    analyzerPort = (GT_PORT_NUM)inArgs[1];
    analyzerHwDev =  (GT_HW_DEV_NUM)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_DATA_MAC(analyzerHwDev, analyzerPort);

    if(PRV_CPSS_PP_MAC(dev)->devFamily < CPSS_PP_FAMILY_DXCH_XCAT_E)
    {
        /* call cpss api function */
        result = cpssDxChMirrorTxAnalyzerPortSet(dev, analyzerPort, analyzerHwDev);
    }
    else
    {
        interface.interface.type = CPSS_INTERFACE_PORT_E;
        interface.interface.devPort.hwDevNum = analyzerHwDev;
        interface.interface.devPort.portNum = analyzerPort;

        /* call cpss api function */
        result = cpssDxChMirrorAnalyzerInterfaceSet(dev, 1, &interface);
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrorTxAnalyzerPortGet function
* @endinternal
*
* @brief   Gets the analyzer port of tx mirrored ports on all system pp's.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChMirrorTxAnalyzerPortGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8   dev;

    GT_PORT_NUM     analyzerPort;
    GT_HW_DEV_NUM   analyzerHwDev;

    CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC  interface;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev =  (GT_U8)inArgs[0];

    if(PRV_CPSS_PP_MAC(dev)->devFamily < CPSS_PP_FAMILY_DXCH_XCAT_E)
    {
        /* call cpss api function */
        result = cpssDxChMirrorTxAnalyzerPortGet(dev, &analyzerPort, &analyzerHwDev);
    }
    else
    {
        /* call cpss api function */
        result = cpssDxChMirrorAnalyzerInterfaceGet(dev, 1, &interface);
        analyzerHwDev = interface.interface.devPort.hwDevNum;
        analyzerPort = interface.interface.devPort.portNum;
    }

    /* Override Back Device and Port */
    CONVERT_BACK_DEV_PORT_DATA_MAC(analyzerHwDev, analyzerPort);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", analyzerPort, analyzerHwDev);
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrorRxAnalyzerPortSet function
* @endinternal
*
* @brief   Sets a specific port to be an analyzer port of Rx mirrored
*         ports.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
*/
static CMD_STATUS wrCpssDxChMirrorRxAnalyzerPortSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8   dev;
    GT_PORT_NUM     analyzerPort;
    GT_HW_DEV_NUM   analyzerHwDev;
    CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC   interface;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev =  (GT_U8)inArgs[0];
    analyzerPort =  (GT_PORT_NUM)inArgs[1];
    analyzerHwDev =  (GT_HW_DEV_NUM)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_DATA_MAC(analyzerHwDev, analyzerPort);

   if(PRV_CPSS_PP_MAC(dev)->devFamily < CPSS_PP_FAMILY_DXCH_XCAT_E)
   {
       /* call cpss api function */
       result = cpssDxChMirrorRxAnalyzerPortSet(dev, analyzerPort, analyzerHwDev);
   }
   else
   {
       interface.interface.type = CPSS_INTERFACE_PORT_E;
       interface.interface.devPort.hwDevNum = analyzerHwDev;
       interface.interface.devPort.portNum = analyzerPort;
       /* call cpss api function */
       result = cpssDxChMirrorAnalyzerInterfaceSet(dev, 0, &interface);
   }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrorRxAnalyzerPortGet function
* @endinternal
*
* @brief   Gets the analyzer port of Rx mirrored ports.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChMirrorRxAnalyzerPortGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8   dev;
    GT_PORT_NUM     analyzerPort;
    GT_HW_DEV_NUM   analyzerHwDev;

    CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC  interface;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev =  (GT_U8)inArgs[0];

    if(PRV_CPSS_PP_MAC(dev)->devFamily < CPSS_PP_FAMILY_DXCH_XCAT_E)
    {
        /* call cpss api function */
        result = cpssDxChMirrorRxAnalyzerPortGet(dev, &analyzerPort, &analyzerHwDev);
    }
    else
    {
        /* call cpss api function */
        result = cpssDxChMirrorAnalyzerInterfaceGet(dev, 0, &interface);
        analyzerHwDev = interface.interface.devPort.hwDevNum;
        analyzerPort = interface.interface.devPort.portNum;
    }

    /* Override Back Device and Port */
    CONVERT_BACK_DEV_PORT_DATA_MAC(analyzerHwDev, analyzerPort);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", analyzerPort, analyzerHwDev);
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrorTxCascadeMonitorEnable function
* @endinternal
*
* @brief   One global bit that is set to 1 when performing egress mirroring or
*         egress STC of any of the cascading ports.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
*/
static CMD_STATUS wrCpssDxChMirrorTxCascadeMonitorEnable
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8       dev;
    GT_BOOL     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev =  (GT_U8)inArgs[0];
    enable =  (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChMirrorTxCascadeMonitorEnable(dev, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrorTxCascadeMonitorEnableGet function
* @endinternal
*
* @brief   Get One global bit that indicate performing egress mirroring or
*         egress STC of any of the cascading ports.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChMirrorTxCascadeMonitorEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8       dev;
    GT_BOOL     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev =  (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChMirrorTxCascadeMonitorEnableGet(dev, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrorRxPortSet function
* @endinternal
*
* @brief   Sets a specific port or group of ports in pp's devices to be Rx
*         mirrored ports
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, mirrPort.
* @retval GT_HW_ERROR              - on writing to HW error.
*
* @note CPU port can not be Rx mirrored port.
*       the Salsa device support CPU port as Rx mirror port .
*
*/
static CMD_STATUS wrCpssDxChMirrorRxPortSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8       dev;
    GT_PORT_NUM       mirrPort;
    GT_BOOL     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev =  (GT_U8)inArgs[0];
    mirrPort =  (GT_PORT_NUM)inArgs[1];
    enable =  (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, mirrPort);

    /* call cpss api function */
    result = cpssDxChMirrorRxPortSet(dev, mirrPort, GT_TRUE, enable, 0);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrorRxPortGet function
* @endinternal
*
* @brief   Gets a specific port or group of ports in pp's devices to be Rx
*         mirrored ports
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, mirrPort.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note CPU port can not be Rx mirrored port.
*       the Salsa device support CPU port as Rx mirror port .
*
*/
static CMD_STATUS wrCpssDxChMirrorRxPortGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8       dev;
    GT_PORT_NUM       mirrPort;
    GT_BOOL     enable;
    GT_U32      index;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev =  (GT_U8)inArgs[0];
    mirrPort =  (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, mirrPort);

    /* call cpss api function */
    result = cpssDxChMirrorRxPortGet(dev, mirrPort, GT_TRUE, &enable, &index);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrorRxPortExtSet function
* @endinternal
*
* @brief   Sets a specific port to be Rx mirrored port.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, mirrPort.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChMirrorRxPortExtSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8       dev;
    GT_PORT_NUM mirrPort;
    GT_BOOL     enable;
    GT_U32      index;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev =  (GT_U8)inArgs[0];
    mirrPort =  (GT_PORT_NUM)inArgs[1];
    enable =  (GT_BOOL)inArgs[2];
    index = (GT_U32)inArgs[3];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, mirrPort);

    /* call cpss api function */
    result = cpssDxChMirrorRxPortSet(dev, mirrPort, GT_TRUE, enable, index);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrorRxPortExtGet function
* @endinternal
*
* @brief   Gets status of Rx mirroring (enabled or disabled) of specific port
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, mirrPort.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChMirrorRxPortExtGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8       dev;
    GT_PORT_NUM mirrPort;
    GT_BOOL     enable;
    GT_U32      index = 0;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev =  (GT_U8)inArgs[0];
    mirrPort =  (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, mirrPort);

    /* call cpss api function */
    result = cpssDxChMirrorRxPortGet(dev, mirrPort, GT_TRUE, &enable, &index);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", enable, index);
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrorRxPortSet_1 function
* @endinternal
*
* @brief   Sets a specific port to be Rx mirrored port.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, mirrPort.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChMirrorRxPortSet_1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8       dev;
    GT_PORT_NUM mirrPort;
    GT_BOOL     enable;
    GT_U32      index;
    GT_BOOL     isPhysicalPort;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev =  (GT_U8)inArgs[0];
    mirrPort =  (GT_PORT_NUM)inArgs[1];
    isPhysicalPort = (GT_BOOL)inArgs[2];
    enable =  (GT_BOOL)inArgs[3];
    index = (GT_U32)inArgs[4];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, mirrPort);

    /* call cpss api function */
    result = cpssDxChMirrorRxPortSet(dev, mirrPort, isPhysicalPort, enable, index);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrorRxPortGet_1 function
* @endinternal
*
* @brief   Gets status of Rx mirroring (enabled or disabled) of specific port
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, mirrPort.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChMirrorRxPortGet_1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8       dev;
    GT_PORT_NUM mirrPort;
    GT_BOOL     enable;
    GT_U32      index = 0;
    GT_BOOL     isPhysicalPort;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev =  (GT_U8)inArgs[0];
    mirrPort =  (GT_PORT_NUM)inArgs[1];
    isPhysicalPort = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, mirrPort);

    /* call cpss api function */
    result = cpssDxChMirrorRxPortGet(dev, mirrPort, isPhysicalPort, &enable, &index);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", enable, index);
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrorTxPortSet function
* @endinternal
*
* @brief   Adds ports to be Tx mirrored ports .
*         Up to 8 ports in all system can be tx mirrored ports.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or mirrPort.
* @retval GT_HW_ERROR              - on writing to HW error.
*
* @note There can be up to 8 Tx mirrored ports per Prestera chipset.
*       CPU port can be Tx mirrored port.
*
*/
static CMD_STATUS wrCpssDxChMirrorTxPortSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8       dev;
    GT_PORT_NUM       mirrPort;
    GT_BOOL     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev =  (GT_U8)inArgs[0];
    mirrPort =  (GT_PORT_NUM)inArgs[1];
    enable =  (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, mirrPort);

    /* call cpss api function */
    result = cpssDxChMirrorTxPortSet(dev, mirrPort, GT_TRUE, enable, 0);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrorTxPortExtSet function
* @endinternal
*
* @brief   Enable or disable Tx mirroring per port.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or mirrPort.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChMirrorTxPortExtSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8       dev;
    GT_PORT_NUM mirrPort;
    GT_BOOL     enable;
    GT_U32      index;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev =  (GT_U8)inArgs[0];
    mirrPort =  (GT_PORT_NUM)inArgs[1];
    enable =  (GT_BOOL)inArgs[2];
    index =  (GT_U32)inArgs[3];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, mirrPort);

    /* call cpss api function */
    result = cpssDxChMirrorTxPortSet(dev, mirrPort, GT_TRUE, enable, index);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrorTxPortGet function
* @endinternal
*
* @brief   Adds ports to be Tx mirrored ports .
*         Up to 8 ports in all system can be tx mirrored ports.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or mirrPort.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note There can be up to 8 Tx mirrored ports per Prestera chipset.
*       CPU port can be Tx mirrored port.
*
*/
static CMD_STATUS wrCpssDxChMirrorTxPortGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8       dev;
    GT_PORT_NUM       mirrPort;
    GT_BOOL     enable;
    GT_U32      index;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev =  (GT_U8)inArgs[0];
    mirrPort =  (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, mirrPort);

    /* call cpss api function */
    result = cpssDxChMirrorTxPortGet(dev, mirrPort, GT_TRUE, &enable, &index);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrorTxPortExtGet function
* @endinternal
*
* @brief   Get status (enabled/disabled) of Tx mirroring per port .
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or mirrPort.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChMirrorTxPortExtGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8       dev;
    GT_PORT_NUM mirrPort;
    GT_BOOL     enable;
    GT_U32      index = 0;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev =  (GT_U8)inArgs[0];
    mirrPort =  (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, mirrPort);

    /* call cpss api function */
    result = cpssDxChMirrorTxPortGet(dev, mirrPort, GT_TRUE, &enable, &index);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", enable, index);
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrorTxPortSet_1 function
* @endinternal
*
* @brief   Enable or disable Tx mirroring per port.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or mirrPort.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChMirrorTxPortSet_1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8       dev;
    GT_PORT_NUM mirrPort;
    GT_BOOL     enable;
    GT_U32      index;
    GT_BOOL     isPhysicalPort;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev =  (GT_U8)inArgs[0];
    mirrPort =  (GT_PORT_NUM)inArgs[1];
    isPhysicalPort = (GT_BOOL)inArgs[2];
    enable =  (GT_BOOL)inArgs[3];
    index =  (GT_U32)inArgs[4];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, mirrPort);

    /* call cpss api function */
    result = cpssDxChMirrorTxPortSet(dev, mirrPort, isPhysicalPort, enable, index);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrorTxPortGet_1 function
* @endinternal
*
* @brief   Get status (enabled/disabled) of Tx mirroring per port .
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or mirrPort.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChMirrorTxPortGet_1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8       dev;
    GT_PORT_NUM mirrPort;
    GT_BOOL     enable;
    GT_U32      index = 0;
    GT_BOOL     isPhysicalPort;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev =  (GT_U8)inArgs[0];
    mirrPort =  (GT_PORT_NUM)inArgs[1];
    isPhysicalPort = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, mirrPort);

    /* call cpss api function */
    result = cpssDxChMirrorTxPortGet(dev, mirrPort, isPhysicalPort, &enable, &index);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", enable, index);
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrorTxPortModeSet function
* @endinternal
*
* @brief   Set Egress Mirroring mode to given port
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or mirrPort.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChMirrorTxPortModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8       dev;
    GT_PORT_NUM mirrPort;
    GT_BOOL     isPhysicalPort;
    CPSS_DXCH_MIRROR_EGRESS_MODE_ENT egressMirroringMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev =  (GT_U8)inArgs[0];
    mirrPort =  (GT_PORT_NUM)inArgs[1];
    isPhysicalPort = (GT_BOOL)inArgs[2];
    egressMirroringMode =  (CPSS_DXCH_MIRROR_EGRESS_MODE_ENT)inArgs[3];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, mirrPort);

    /* call cpss api function */
    result = cpssDxChMirrorTxPortModeSet(dev, mirrPort, isPhysicalPort, egressMirroringMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrorTxPortModeGet function
* @endinternal
*
* @brief   Get Egress Mirroring mode to given port
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or mirrPort.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChMirrorTxPortModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8       dev;
    GT_PORT_NUM mirrPort;
    GT_BOOL     isPhysicalPort;
    CPSS_DXCH_MIRROR_EGRESS_MODE_ENT egressMirroringMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev =  (GT_U8)inArgs[0];
    mirrPort =  (GT_PORT_NUM)inArgs[1];
    isPhysicalPort = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, mirrPort);

    /* call cpss api function */
    result = cpssDxChMirrorTxPortModeGet(dev, mirrPort, isPhysicalPort, &egressMirroringMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", egressMirroringMode);
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexSet function
* @endinternal
*
* @brief   Set analyzer interface index, used for
*          egress mirroring for tail dropped/congestion frames.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, index or egressMirroringMode is not tail drop or congestion.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8       dev;
    CPSS_DXCH_MIRROR_EGRESS_MODE_ENT egressMirroringMode;
    GT_BOOL     enable;
    GT_U32      index;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev =  (GT_U8)inArgs[0];
    egressMirroringMode = (CPSS_DXCH_MIRROR_EGRESS_MODE_ENT)inArgs[1];
    enable = (GT_BOOL)inArgs[2];
    index =  (GT_U32)inArgs[3];

    /* call cpss api function */
    result = cpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexSet(dev, egressMirroringMode, enable, index);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexGet function
* @endinternal
*
* @brief   Get analyzer interface index, used for
*          egress mirroring for tail dropped/congestion frames.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or egressMirroringMode is not tail drop or congestion.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8       dev;
    CPSS_DXCH_MIRROR_EGRESS_MODE_ENT egressMirroringMode;
    GT_BOOL     enable;
    GT_U32      index;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev =  (GT_U8)inArgs[0];
    egressMirroringMode = (CPSS_DXCH_MIRROR_EGRESS_MODE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexGet(dev, egressMirroringMode, &enable, &index);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", enable, index);
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet function
* @endinternal
*
* @brief   Set Ingress statistical Mirroring
*          to the Ingress Analyzer Port including ratio per analyzer index.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or index.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8       dev;
    GT_BOOL     enable;
    GT_U32      index;
    GT_U32      ratio;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev =  (GT_U8)inArgs[0];
    index =  (GT_U32)inArgs[1];
    enable = (GT_BOOL)inArgs[2];
    ratio =  (GT_U32)inArgs[3];

    /* call cpss api function */
    result = cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet(dev, index, enable, ratio);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioGet function
* @endinternal
*
* @brief   Get Ingress statistical Mirroring
*          to the Ingress Analyzer Port including ratio per analyzer index.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or ratio.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8       dev;
    GT_U32      index;
    GT_BOOL     enable;
    GT_U32      ratio;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev =  (GT_U8)inArgs[0];
    index =  (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioGet(dev, index, &enable, &ratio);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", enable, ratio);
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrorToAnalyzerTruncateSet function
* @endinternal
*
* @brief   Set if to truncate TO_ANALYZER packets per analyzer index.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or index.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChMirrorToAnalyzerTruncateSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8       dev;
    GT_U32      index;
    GT_BOOL     truncate;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev =  (GT_U8)inArgs[0];
    index =  (GT_U32)inArgs[1];
    truncate = (GT_BOOL)inArgs[2];

    /* call cpss api function */
    result = cpssDxChMirrorToAnalyzerTruncateSet(dev, index, truncate);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrorToAnalyzerTruncateGet function
* @endinternal
*
* @brief   Get truncate field per analyzer index.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or index.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChMirrorToAnalyzerTruncateGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8       dev;
    GT_U32      index;
    GT_BOOL     truncate;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev =  (GT_U8)inArgs[0];
    index =  (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChMirrorToAnalyzerTruncateGet(dev, index, &truncate);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", truncate);
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrRxStatMirroringToAnalyzerRatioSet function
* @endinternal
*
* @brief   Set the statistical mirroring rate in the Rx Analyzer port
*         Set Statistic mirroring to analyzer port statistical ratio configuration.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or ratio.
* @retval GT_HW_ERROR              - on writing to HW error.
*/
static CMD_STATUS wrCpssDxChMirrRxStatMirroringToAnalyzerRatioSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8       dev;
    GT_U32      ratio, current_ratio;
    GT_U32      analyzerIndex;
    GT_BOOL     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev =  (GT_U8)inArgs[0];
    ratio =  (GT_U32)inArgs[1];

    /* call cpss api function */
    if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
    {
        for(analyzerIndex = 0 ; analyzerIndex <= CPSS_DXCH_MIRROR_ANALYZER_MAX_INDEX_CNS ; analyzerIndex ++)
        {
            result = cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioGet(dev,analyzerIndex,&enable,&current_ratio);
            if(result != GT_OK)
            {
                break;
            }
            result = cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet(dev,analyzerIndex,enable,ratio);
            if(result != GT_OK)
            {
                break;
            }
        }
    }
    else
    {
        result = cpssDxChMirrRxStatMirroringToAnalyzerRatioSet(dev, ratio);
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrRxStatMirroringToAnalyzerRatioGet function
* @endinternal
*
* @brief   Get the statistical mirroring rate in the Rx Analyzer port
*         Get Statistic mirroring to analyzer port statistical ratio configuration.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
*/
static CMD_STATUS wrCpssDxChMirrRxStatMirroringToAnalyzerRatioGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8       dev;
    GT_U32      ratio;
    GT_BOOL     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev =  (GT_U8)inArgs[0];

    /* call cpss api function */
    if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
    {
        result = cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioGet(dev,0,&enable,&ratio);
    }
    else
    {
        result = cpssDxChMirrRxStatMirroringToAnalyzerRatioGet(dev, &ratio);
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", ratio);
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrRxStatMirrorToAnalyzerEnable function
* @endinternal
*
* @brief   Enable Ingress Statistical Mirroring to the Egress Analyzer Port
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
*/
static CMD_STATUS wrCpssDxChMirrRxStatMirrorToAnalyzerEnable
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8       dev;
    GT_BOOL     enable, current_enable;
    GT_U32      analyzerIndex, ratio;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev =  (GT_U8)inArgs[0];
    enable =  (GT_BOOL)inArgs[1];

    /* call cpss api function */
    if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
    {
        for(analyzerIndex = 0 ; analyzerIndex <= CPSS_DXCH_MIRROR_ANALYZER_MAX_INDEX_CNS ; analyzerIndex ++)
        {
            result = cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioGet(dev,analyzerIndex,&current_enable,&ratio);
            if(result != GT_OK)
            {
                break;
            }
            result = cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet(dev,analyzerIndex,enable,ratio);
            if(result != GT_OK)
            {
                break;
            }
        }
    }
    else
    {
        result = cpssDxChMirrRxStatMirrorToAnalyzerEnable(dev, enable);
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrRxStatMirrorToAnalyzerEnableGet function
* @endinternal
*
* @brief   Get Ingress Statistical Mirroring to the Ingress Analyzer Port Mode
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
*/
static CMD_STATUS wrCpssDxChMirrRxStatMirrorToAnalyzerEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8       dev;
    GT_BOOL     enable;
    GT_U32      ratio;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev =  (GT_U8)inArgs[0];

    /* call cpss api function */
    if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
    {
        result = cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioGet(dev,0,&enable,&ratio);
    }
    else
    {
        result = cpssDxChMirrRxStatMirrorToAnalyzerEnableGet(dev, &enable);
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrRxAnalyzerDpTcSet function
* @endinternal
*
* @brief   The TC/DP assigned to the packet forwarded to the ingress analyzer port due
*         to ingress mirroring to the analyzer port.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_HW_ERROR              - on writing to HW error.
*/
static CMD_STATUS wrCpssDxChMirrRxAnalyzerDpTcSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8             dev;
    CPSS_DP_LEVEL_ENT analyzerDp;
    GT_U8             analyzerTc;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev =  (GT_U8)inArgs[0];
    analyzerDp =  (CPSS_DP_LEVEL_ENT)inArgs[1];
    analyzerTc =  (GT_U8)inArgs[2];

    /* call cpss api function */
    result = cpssDxChMirrRxAnalyzerDpTcSet(dev, analyzerDp, analyzerTc);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrRxAnalyzerDpTcGet function
* @endinternal
*
* @brief   Get TC/DP assigned to the packet forwarded to the ingress analyzer port due
*         to ingress mirroring to the analyzer port cofiguration.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_BAD_PTR               - wrong pointer.
* @retval GT_HW_ERROR              - on writing to HW error.
*/
static CMD_STATUS wrCpssDxChMirrRxAnalyzerDpTcGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8             dev;
    CPSS_DP_LEVEL_ENT analyzerDp;
    GT_U8             analyzerTc;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev =  (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChMirrRxAnalyzerDpTcGet(dev, &analyzerDp, &analyzerTc);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", analyzerDp, analyzerTc);
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrTxAnalyzerDpTcSet function
* @endinternal
*
* @brief   The TC/DP assigned to the packet forwarded to the egress analyzer port due
*         to ingress mirroring to the analyzer port.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on writing to HW error.
*/
static CMD_STATUS wrCpssDxChMirrTxAnalyzerDpTcSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8             dev;
    CPSS_DP_LEVEL_ENT analyzerDp;
    GT_U8             analyzerTc;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev =  (GT_U8)inArgs[0];
    analyzerDp =  (CPSS_DP_LEVEL_ENT)inArgs[1];
    analyzerTc =  (GT_U8)inArgs[2];

    /* call cpss api function */
    result = cpssDxChMirrTxAnalyzerDpTcSet(dev, analyzerDp, analyzerTc);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrTxAnalyzerDpTcGet function
* @endinternal
*
* @brief   Get TC/DP assigned to the packet forwarded to the egress analyzer port due
*         to ingress mirroring to the analyzer port cofiguration.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device number.
* @retval GT_BAD_PTR               - wrong pointer.
* @retval GT_HW_ERROR              - on writing to HW error.
*/
static CMD_STATUS wrCpssDxChMirrTxAnalyzerDpTcGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8             dev;
    CPSS_DP_LEVEL_ENT analyzerDp;
    GT_U8             analyzerTc;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev =  (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChMirrTxAnalyzerDpTcGet(dev, &analyzerDp, &analyzerTc);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", analyzerDp, analyzerTc);
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrTxStatMirroringToAnalyzerRatioSet function
* @endinternal
*
* @brief   Set the statistical mirroring rate in the Tx Analyzer port
*         Set Statistic mirroring to analyzer port statistical ratio configuration.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or ratio.
* @retval GT_HW_ERROR              - on writing to HW error.
*/
static CMD_STATUS wrCpssDxChMirrTxStatMirroringToAnalyzerRatioSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8       dev;
    GT_U32      ratio;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev =  (GT_U8)inArgs[0];
    ratio =  (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChMirrTxStatMirroringToAnalyzerRatioSet(dev, ratio);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrTxStatMirroringToAnalyzerRatioGet function
* @endinternal
*
* @brief   Get the statistical mirroring rate in the Tx Analyzer port
*         Get Statistic mirroring to analyzer port statistical ratio configuration.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
*/
static CMD_STATUS wrCpssDxChMirrTxStatMirroringToAnalyzerRatioGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8       dev;
    GT_U32      ratio;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev =  (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChMirrTxStatMirroringToAnalyzerRatioGet(dev, &ratio);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", ratio);
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrTxStatMirrorToAnalyzerEnable function
* @endinternal
*
* @brief   Enable Egress Statistical Mirroring to the Egress Analyzer Port
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_HW_ERROR              - on writing to HW error.
*/
static CMD_STATUS wrCpssDxChMirrTxStatMirrorToAnalyzerEnable
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8       dev;
    GT_BOOL     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev =  (GT_U8)inArgs[0];
    enable =  (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChMirrTxStatMirrorToAnalyzerEnable(dev, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrTxStatMirrorToAnalyzerEnableGet function
* @endinternal
*
* @brief   Get mode of Egress Statistical Mirroring to the Egress Analyzer Port
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
*/
static CMD_STATUS wrCpssDxChMirrTxStatMirrorToAnalyzerEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8       dev;
    GT_BOOL     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev =  (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChMirrTxStatMirrorToAnalyzerEnableGet(dev, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}


/**
* @internal wrCpssDxChMirrorToAnalyzerForwardingModeSet function
* @endinternal
*
* @brief   Set Forwarding mode to Analyzer for egress/ingress mirroring.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, mode.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To change Forwarding mode, applicaton should
*       disable Rx/Tx mirrorred ports.
*
*/
static CMD_STATUS wrCpssDxChMirrorToAnalyzerForwardingModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8       devNum;
    CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT   mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum =  (GT_U8)inArgs[0];
    mode =  (CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChMirrorToAnalyzerForwardingModeSet(devNum, mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrorToAnalyzerForwardingModeGet function
* @endinternal
*
* @brief   Get Forwarding mode to Analyzer for egress/ingress mirroring.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChMirrorToAnalyzerForwardingModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8     devNum;
    CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT   mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum =  (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChMirrorToAnalyzerForwardingModeGet(devNum, &mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mode);
    return CMD_OK;

}

/**
* @internal wrCpssDxChMirrorAnalyzerInterfaceSet function
* @endinternal
*
* @brief   This function sets analyzer interface.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, index, interface type.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - on wrong port or device number in interfacePtr.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChMirrorAnalyzerInterfaceSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8     devNum;
    GT_U32    index;
    CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC   interface;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum =  (GT_U8)inArgs[0];
    index = (GT_U32)inArgs[1];
    interface.interface.type = (CPSS_INTERFACE_TYPE_ENT)inArgs[2];
    interface.interface.devPort.hwDevNum = (GT_HW_DEV_NUM)inArgs[3];
    interface.interface.devPort.portNum = (GT_PORT_NUM)inArgs[4];

    /*dev,port conversion*/
    CONVERT_DEV_PORT_DATA_MAC(interface.interface.devPort.hwDevNum ,
                              interface.interface.devPort.portNum);

    /* call cpss api function */
    result = cpssDxChMirrorAnalyzerInterfaceSet(devNum, index, &interface);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrorAnalyzerInterfaceGet function
* @endinternal
*
* @brief   This function gets analyzer interface.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, index.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChMirrorAnalyzerInterfaceGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U32    index;
    CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC   interface;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum =  (GT_U8)inArgs[0];
    index =  (GT_U8)inArgs[1];

    cpssOsMemSet(&interface, 0, sizeof(CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC));

    /* call cpss api function */
    result = cpssDxChMirrorAnalyzerInterfaceGet(devNum, index, &interface);

    /* Override Back Device and Port */
    CONVERT_BACK_DEV_PORT_DATA_MAC(interface.interface.devPort.hwDevNum,
                                   interface.interface.devPort.portNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d%d%d%d%d%d", interface.interface.type,
                 interface.interface.devPort.hwDevNum,
                 interface.interface.devPort.portNum,
                 interface.interface.trunkId, interface.interface.vidx,
                 interface.interface.vlanId, interface.interface.hwDevNum,
                 interface.interface.fabricVidx, interface.interface.index);

    return CMD_OK;

}

/**
* @internal wrCpssDxChMirrorRxGlobalAnalyzerInterfaceIndexSet function
* @endinternal
*
* @brief   This function sets analyzer interface index, used for ingress
*         mirroring from all engines except
*         port-mirroring source-based-forwarding mode.
*         (Port-Based hop-by-hop mode, Policy-Based, VLAN-Based,
*         FDB-Based, Router-Based).
*         If a packet is mirrored by both the port-based ingress mirroring,
*         and one of the other ingress mirroring, the selected analyzer is
*         the one with the higher index.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChMirrorRxGlobalAnalyzerInterfaceIndexSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8     devNum;
    GT_U32    index;
    GT_BOOL   enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum =  (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];
    index = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexSet(devNum, enable, index);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrorRxGlobalAnalyzerInterfaceIndexGet function
* @endinternal
*
* @brief   This function gets analyzer interface index, used for ingress mirroring
*         from all engines except port-mirroring source-based-forwarding mode.
*         (Port-Based hop-by-hop mode, Policy-Based, VLAN-Based,
*         FDB-Based, Router-Based).
*         If a packet is mirrored by both the port-based ingress mirroring,
*         and one of the other ingress mirroring, the selected analyzer is
*         the one with the higher index.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChMirrorRxGlobalAnalyzerInterfaceIndexGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U32    index = 0;
    GT_BOOL   enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum =  (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexGet(devNum, &enable,
                                                             &index);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", enable, index);

    return CMD_OK;

}

/**
* @internal wrCpssDxChMirrorTxGlobalAnalyzerInterfaceIndexSet function
* @endinternal
*
* @brief   This function sets analyzer interface index, used for egress
*         mirroring for Port-Based hop-by-hop mode.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_OUT_OF_RANGE          - index is out of range.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChMirrorTxGlobalAnalyzerInterfaceIndexSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8     devNum;
    GT_U32    index;
    GT_BOOL   enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum =  (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];
    index = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexSet(devNum, enable, index);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrorTxGlobalAnalyzerInterfaceIndexGet function
* @endinternal
*
* @brief   This function gets analyzer interface index, used for egress
*         mirroring for Port-Based hop-by-hop mode.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChMirrorTxGlobalAnalyzerInterfaceIndexGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U32    index = 0;
    GT_BOOL   enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum =  (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexGet(devNum, &enable,
                                                             &index);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", enable, index);
    return CMD_OK;

}

/**
* @internal wrCpssDxChMirrorAnalyzerMirrorOnDropEnableSet function
* @endinternal
*
* @brief   Enable / Disable mirroring of dropped packets.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, index.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChMirrorAnalyzerMirrorOnDropEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8     devNum;
    GT_U32    index;
    GT_BOOL   enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum =  (GT_U8)inArgs[0];
    index = (GT_U32)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* call cpss api function */
    result = cpssDxChMirrorAnalyzerMirrorOnDropEnableSet(devNum, index, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrorAnalyzerMirrorOnDropEnableGet function
* @endinternal
*
* @brief   Get mirroring status of dropped packets.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, index.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChMirrorAnalyzerMirrorOnDropEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U32    index;
    GT_BOOL   enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum =  (GT_U8)inArgs[0];
    index = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChMirrorAnalyzerMirrorOnDropEnableGet(devNum, index,
                                                         &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;

}

/**
* @internal wrCpssDxChMirrorAnalyzerVlanTagRemoveEnableSet function
* @endinternal
*
* @brief   Enable/Disable VLAN tag removal of mirrored traffic.
*         When VLAN tag removal is enabled for a specific analyzer port, all
*         packets that are mirrored to this port are sent without any VLAN tags.
*
* @note   APPLICABLE DEVICES:      Lion and above
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChMirrorAnalyzerVlanTagRemoveEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8     devNum;
    GT_PORT_NUM     portNum;
    GT_BOOL   enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum =  (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /*dev,port conversion*/
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChMirrorAnalyzerVlanTagRemoveEnableSet(devNum, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrorAnalyzerVlanTagRemoveEnableGet function
* @endinternal
*
* @brief   Get status of enabling/disabling VLAN tag removal of mirrored traffic.
*         When VLAN tag removal is enabled for a specific analyzer port, all
*         packets that are mirrored to this port are sent without any VLAN tags.
*
* @note   APPLICABLE DEVICES:      Lion and above
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChMirrorAnalyzerVlanTagRemoveEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_PORT_NUM     portNum;
    GT_BOOL   enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum =  (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];

    /*dev,port conversion*/
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChMirrorAnalyzerVlanTagRemoveEnableGet(devNum, portNum,
                                                          &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;

}

/**
* @internal wrCpssDxChMirrorEnhancedMirroringPriorityModeSet function
* @endinternal
*
* @brief   Setting Enhanced Mirroring Priority mode.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or mode.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChMirrorEnhancedMirroringPriorityModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8       devNum;
    CPSS_DXCH_MIRROR_ENHANCED_PRIORITY_MODE_ENT   mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum =  (GT_U8)inArgs[0];
    mode =  (CPSS_DXCH_MIRROR_ENHANCED_PRIORITY_MODE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChMirrorEnhancedMirroringPriorityModeSet(devNum, mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrorEnhancedMirroringPriorityModeGet function
* @endinternal
*
* @brief   Getting Enhanced Mirroring Priority mode.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChMirrorEnhancedMirroringPriorityModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8     devNum;
    CPSS_DXCH_MIRROR_ENHANCED_PRIORITY_MODE_ENT   mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum =  (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChMirrorEnhancedMirroringPriorityModeGet(devNum, &mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mode);
    return CMD_OK;

}
/**
* @internal wrCpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeSet function
* @endinternal
*
* @brief   Set the drop code to be used for Hop-By-Hop mirroring mode, when a packet
*         is duplicated to a target analyzer, and the global Ingress Analyzer Index
*         or Egress Analyzer Index is DISABLED.
*         In this case the packet is hard dropped with this drop code.
*         NOTE: not relevant to the device that generates the duplication but only
*         to device that will get 'TO_ANALYZER' DSA tag (in HOP-BY_HOP mode)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device , bad dropCode.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8       devNum;
    CPSS_NET_RX_CPU_CODE_ENT   dropCode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum =  (GT_U8)inArgs[0];
    dropCode =  (CPSS_NET_RX_CPU_CODE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeSet(devNum, dropCode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeGet function
* @endinternal
*
* @brief   Get the drop code to be used for Hop-By-Hop mirroring mode, when a packet
*         is duplicated to a target analyzer, and the global Ingress Analyzer Index
*         or Egress Analyzer Index is DISABLED.
*         In this case the packet is hard dropped with this drop code.
*         NOTE: not relevant to the device that generates the duplication but only
*         to device that will get 'TO_ANALYZER' DSA tag (in HOP-BY_HOP mode)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8       devNum;
    CPSS_NET_RX_CPU_CODE_ENT   dropCode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum =  (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeGet(devNum, &dropCode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", dropCode);
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrorTxPortVlanEnableSet function
* @endinternal
*
* @brief   Enable or disable Tx vlan mirroring from specific physical port.
*         (define if the physical port allow/deny egress vlan mirroring)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device , bad portNum.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChMirrorTxPortVlanEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8       devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_BOOL                 enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum =  (GT_U8)inArgs[0];
    portNum =  (GT_PHYSICAL_PORT_NUM)inArgs[1];
    enable =  (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChMirrorTxPortVlanEnableSet(devNum, portNum , enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChMirrorTxPortVlanEnableGet function
* @endinternal
*
* @brief   Get is the Tx vlan mirroring from specific physical port enabled/disabled.
*         (define if the physical port allow/deny egress vlan mirroring)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChMirrorTxPortVlanEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8       devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_BOOL                 enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum =  (GT_U8)inArgs[0];
    portNum =  (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChMirrorTxPortVlanEnableGet(devNum, portNum ,&enable );

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}


/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
    {"cpssDxChMirrorAnalyzerVlanTagEnable",
        &wrCpssDxChMirrorAnalyzerVlanTagEnable,
        3, 0},

    {"cpssDxChMirrorAnalyzerVlanTagEnableGet",
        &wrCpssDxChMirrorAnalyzerVlanTagEnableGet,
        2, 0},

    {"cpssDxChMirrorTxAnalyzerVlanTagConfig",
        &wrCpssDxChMirrorTxAnalyzerVlanTagConfig,
        5, 0},

    {"cpssDxChMirrorTxAnalyzerVlanTagConfigGet",
        &wrCpssDxChMirrorTxAnalyzerVlanTagConfigGet,
        1, 0},

    {"cpssDxChMirrorRxAnalyzerVlanTagConfig",
        &wrCpssDxChMirrorRxAnalyzerVlanTagConfig,
        5, 0},

    {"cpssDxChMirrorRxAnalyzerVlanTagConfigGet",
        &wrCpssDxChMirrorRxAnalyzerVlanTagConfigGet,
        1, 0},

    {"cpssDxChMirrorTxAnalyzerPortSet",
        &wrCpssDxChMirrorTxAnalyzerPortSet,
        3, 0},

    {"cpssDxChMirrorTxAnalyzerPortGet",
        &wrCpssDxChMirrorTxAnalyzerPortGet,
        1, 0},

    {"cpssDxChMirrorRxAnalyzerPortSet",
        &wrCpssDxChMirrorRxAnalyzerPortSet,
        3, 0},

    {"cpssDxChMirrorRxAnalyzerPortGet",
        &wrCpssDxChMirrorRxAnalyzerPortGet,
        1, 0},

    {"cpssDxChMirrorTxCascadeMonitorEnable",
        &wrCpssDxChMirrorTxCascadeMonitorEnable,
        2, 0},

    {"cpssDxChMirrorTxCascadeMonitorEnableGet",
        &wrCpssDxChMirrorTxCascadeMonitorEnableGet,
        1, 0},

    {"cpssDxChMirrorRxPortSet",
        &wrCpssDxChMirrorRxPortSet,
        3, 0},

    {"cpssDxChMirrorRxPortGet",
        &wrCpssDxChMirrorRxPortGet,
        2, 0},

    {"cpssDxChMirrorRxPortExtSet",
        &wrCpssDxChMirrorRxPortExtSet,
        4, 0},

    {"cpssDxChMirrorRxPortExtGet",
        &wrCpssDxChMirrorRxPortExtGet,
        2, 0},

    {"cpssDxChMirrorRxPortSet_1",
        &wrCpssDxChMirrorRxPortSet_1,
        5, 0},

    {"cpssDxChMirrorRxPortGet_1",
        &wrCpssDxChMirrorRxPortGet_1,
        3, 0},

    {"cpssDxChMirrorTxPortSet",
        &wrCpssDxChMirrorTxPortSet,
        3, 0},

    {"cpssDxChMirrorTxPortExtSet",
        &wrCpssDxChMirrorTxPortExtSet,
        4, 0},

    {"cpssDxChMirrorTxPortGet",
        &wrCpssDxChMirrorTxPortGet,
        2, 0},

    {"cpssDxChMirrorTxPortExtGet",
        &wrCpssDxChMirrorTxPortExtGet,
        2, 0},

    {"cpssDxChMirrorTxPortSet_1",
        &wrCpssDxChMirrorTxPortSet_1,
        5, 0},

    {"cpssDxChMirrorTxPortGet_1",
        &wrCpssDxChMirrorTxPortGet_1,
        3, 0},

    {"cpssDxChMirrRxStatMirroringToAnalyzerRatioSet",
        &wrCpssDxChMirrRxStatMirroringToAnalyzerRatioSet,
        2, 0},

    {"cpssDxChMirrRxStatMirroringToAnalyzerRatioGet",
        &wrCpssDxChMirrRxStatMirroringToAnalyzerRatioGet,
        1, 0},

    {"cpssDxChMirrRxStatMirrorToAnalyzerEnable",
        &wrCpssDxChMirrRxStatMirrorToAnalyzerEnable,
        2, 0},

    {"cpssDxChMirrRxStatMirrorToAnalyzerEnableGet",
        &wrCpssDxChMirrRxStatMirrorToAnalyzerEnableGet,
        1, 0},

    {"cpssDxChMirrRxAnalyzerDpTcSet",
        &wrCpssDxChMirrRxAnalyzerDpTcSet,
        3, 0},

    {"cpssDxChMirrRxAnalyzerDpTcGet",
        &wrCpssDxChMirrRxAnalyzerDpTcGet,
        1, 0},

    {"cpssDxChMirrTxAnalyzerDpTcSet",
        &wrCpssDxChMirrTxAnalyzerDpTcSet,
        3, 0},

    {"cpssDxChMirrTxAnalyzerDpTcGet",
        &wrCpssDxChMirrTxAnalyzerDpTcGet,
        1, 0},

    {"cpssDxChMirrTxStatMirroringToAnalyzerRatioSet",
        &wrCpssDxChMirrTxStatMirroringToAnalyzerRatioSet,
        2, 0},

    {"cpssDxChMirrTxStatMirroringToAnalyzerRatioGet",
        &wrCpssDxChMirrTxStatMirroringToAnalyzerRatioGet,
        1, 0},

    {"cpssDxChMirrTxStatMirrorToAnalyzerEnable",
        &wrCpssDxChMirrTxStatMirrorToAnalyzerEnable,
        2, 0},

    {"cpssDxChMirrTxStatMirrorToAnalyzerEnableGet",
        &wrCpssDxChMirrTxStatMirrorToAnalyzerEnableGet,
        1, 0},

    {"cpssDxChMirrorToAnalyzerForwardingModeSet",
        &wrCpssDxChMirrorToAnalyzerForwardingModeSet,
        2, 0},

    {"cpssDxChMirrorToAnalyzerForwardingModeGet",
        &wrCpssDxChMirrorToAnalyzerForwardingModeGet,
        1, 0},

    {"cpssDxChMirrorAnalyzerInterfaceSet",
        &wrCpssDxChMirrorAnalyzerInterfaceSet,
        5, 0},

    {"cpssDxChMirrorAnalyzerInterfaceGet",
        &wrCpssDxChMirrorAnalyzerInterfaceGet,
        2, 0},

    {"cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexSet",
        &wrCpssDxChMirrorRxGlobalAnalyzerInterfaceIndexSet,
        3, 0},

    {"cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexGet",
        &wrCpssDxChMirrorRxGlobalAnalyzerInterfaceIndexGet,
        1, 0},

    {"cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexSet",
        &wrCpssDxChMirrorTxGlobalAnalyzerInterfaceIndexSet,
        3, 0},

    {"cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexGet",
        &wrCpssDxChMirrorTxGlobalAnalyzerInterfaceIndexGet,
        1, 0},

    {"cpssDxChMirrorAnalyzerMirrorOnDropEnableSet",
        &wrCpssDxChMirrorAnalyzerMirrorOnDropEnableSet,
        3, 0},

    {"cpssDxChMirrorAnalyzerMirrorOnDropEnableGet",
        &wrCpssDxChMirrorAnalyzerMirrorOnDropEnableGet,
        2, 0},

    {"cpssDxChMirrorAnalyzerVlanTagRemoveEnableSet",
        &wrCpssDxChMirrorAnalyzerVlanTagRemoveEnableSet,
        3, 0},

    {"cpssDxChMirrorAnalyzerVlanTagRemoveEnableGet",
        &wrCpssDxChMirrorAnalyzerVlanTagRemoveEnableGet,
        2, 0},

    {"cpssDxChMirrorEnhancedMirroringPriorityModeSet",
        &wrCpssDxChMirrorEnhancedMirroringPriorityModeSet,
        2, 0},

    {"cpssDxChMirrorEnhancedMirroringPriorityModeGet",
        &wrCpssDxChMirrorEnhancedMirroringPriorityModeGet,
        1, 0},

    {"cpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeSet",
        &wrCpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeSet,
        2, 0},

    {"cpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeGet",
        &wrCpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeGet,
        1, 0},

    {"cpssDxChMirrorTxPortVlanEnableSet",
        &wrCpssDxChMirrorTxPortVlanEnableSet,
        3, 0},

    {"cpssDxChMirrorTxPortVlanEnableGet",
        &wrCpssDxChMirrorTxPortVlanEnableGet,
        2, 0},

    {"cpssDxChMirrorTxPortModeSet",
        &wrCpssDxChMirrorTxPortModeSet,
        4, 0},

    {"cpssDxChMirrorTxPortModeGet",
        &wrCpssDxChMirrorTxPortModeGet,
        3, 0},

    {"cpssDxChMirrorTxModeGlobalAnalyzInterfaceIndxSet",
        &wrCpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexSet,
        4, 0},

    {"cpssDxChMirrorTxModeGlobalAnalyzInterfaceIndxGet",
        &wrCpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexGet,
        2, 0},

    {"cpssDxChMirrRxStatMirrToAnalyzInterfaceRatioSet",
        &wrCpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet,
        4, 0},

    {"cpssDxChMirrRxStatMirrToAnalyzInterfaceRatioGet",
        &wrCpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioGet,
        2, 0},

    {"cpssDxChMirrorToAnalyzerTruncateSet",
        &wrCpssDxChMirrorToAnalyzerTruncateSet,
        3, 0},

    {"cpssDxChMirrorToAnalyzerTruncateGet",
        &wrCpssDxChMirrorToAnalyzerTruncateGet,
        2, 0}

};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssDxChMirror function
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
GT_STATUS cmdLibInitCpssDxChMirror
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}



