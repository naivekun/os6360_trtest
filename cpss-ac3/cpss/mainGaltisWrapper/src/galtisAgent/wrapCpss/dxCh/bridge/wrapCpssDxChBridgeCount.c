


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
* wrapBridgeCountCpssDxCh.c
*
* DESCRIPTION:
*       Wrapper functions for BridgeCount cpss.dxCh functions
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 8 $
*
*******************************************************************************/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>


/* Feature specific includes. */
#include <cpss/common/cpssTypes.h>
#include <cpss/generic/bridge/cpssGenBrgGen.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgCount.h>

/**
* @internal bridgeCountMultiPortGroupsBmpGet function
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
static void bridgeCountMultiPortGroupsBmpGet
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
static GT_STATUS pg_wrap_cpssDxChBrgCntDropCntrGet
(
    IN   GT_U8               devNum,
    OUT  GT_U32              *dropCntPtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    bridgeCountMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChBrgCntDropCntrGet(devNum, dropCntPtr);
    }
    else
    {
        return cpssDxChBrgCntPortGroupDropCntrGet(devNum, pgBmp, dropCntPtr);
    }
}

/* Port Group aqnd Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChBrgCntDropCntrSet
(
    IN   GT_U8               devNum,
    IN  GT_U32               dropCnt
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    bridgeCountMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChBrgCntDropCntrSet(devNum, dropCnt);
    }
    else
    {
        return cpssDxChBrgCntPortGroupDropCntrSet(devNum, pgBmp, dropCnt);
    }
}


/* Port Group aqnd Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChBrgCntHostGroupCntrsGet
(
    IN  GT_U8                               devNum,
    OUT  CPSS_DXCH_BRIDGE_HOST_CNTR_STC     *hostGroupCntPtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    bridgeCountMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChBrgCntHostGroupCntrsGet(devNum, hostGroupCntPtr);
    }
    else
    {
        return cpssDxChBrgCntPortGroupHostGroupCntrsGet(devNum, pgBmp,
                                                        hostGroupCntPtr);
    }
}

/* Port Group aqnd Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChBrgCntMatrixGroupCntrsGet
(
    IN  GT_U8       devNum,
    OUT  GT_U32     *matrixCntSaDaPktsPtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    bridgeCountMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChBrgCntMatrixGroupCntrsGet(devNum, matrixCntSaDaPktsPtr);
    }
    else
    {
        return cpssDxChBrgCntPortGroupMatrixGroupCntrsGet(devNum, pgBmp,
                                                        matrixCntSaDaPktsPtr);
    }
}


/* Port Group aqnd Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChBrgCntBridgeIngressCntrsGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_BRIDGE_CNTR_SET_ID_ENT    cntrSetId,
    OUT CPSS_BRIDGE_INGRESS_CNTR_STC        *ingressCntrPtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    bridgeCountMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChBrgCntBridgeIngressCntrsGet(devNum, cntrSetId,
                                                   ingressCntrPtr);
    }
    else
    {
        return cpssDxChBrgCntPortGroupBridgeIngressCntrsGet(devNum, pgBmp,
                                                            cntrSetId, 
                                                            ingressCntrPtr);
    }
}

/* Port Group aqnd Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChBrgCntLearnedEntryDiscGet
(
    IN  GT_U8         devNum,
    OUT GT_U32        *countValuePtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    bridgeCountMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChBrgCntLearnedEntryDiscGet(devNum, countValuePtr);
    }
    else
    {
        return cpssDxChBrgCntPortGroupLearnedEntryDiscGet(devNum, pgBmp,
                                                          countValuePtr);
    }
}

/**
* @internal wrCpssDxChBrgCntDropCntrModeSet function
* @endinternal
*
* @brief   Sets Drop Counter Mode (configures a Bridge Drop Counter "reason").
*
* @note   APPLICABLE DEVICES:      ALL DxCh Devices.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum or dropMode
* @retval GT_NOT_SUPPORTED         - on non-supported dropMode
*/
static CMD_STATUS wrCpssDxChBrgCntDropCntrModeSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                               result;

    GT_U8                                   devNum;
    CPSS_DXCH_BRIDGE_DROP_CNTR_MODE_ENT     dropMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    dropMode = (CPSS_DXCH_BRIDGE_DROP_CNTR_MODE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgCntDropCntrModeSet(devNum, dropMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgCntDropCntrModeGet function
* @endinternal
*
* @brief   Gets the Drop Counter Mode (Bridge Drop Counter "reason").
*
* @note   APPLICABLE DEVICES:      ALL DxCh Devices.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_SUPPORTED         - on non-supported dropMode
*/
static CMD_STATUS wrCpssDxChBrgCntDropCntrModeGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                               result;

    GT_U8                                   devNum;
    CPSS_DXCH_BRIDGE_DROP_CNTR_MODE_ENT     dropModePtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgCntDropCntrModeGet(devNum, &dropModePtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", dropModePtr);
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgCntMacDaSaSet function
* @endinternal
*
* @brief   Sets a specific MAC DA and SA to be monitored by Host
*         and Matrix counter groups on specified device.
*
* @note   APPLICABLE DEVICES:      ALL DxCh Devices.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static CMD_STATUS wrCpssDxChBrgCntMacDaSaSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS        result;

    GT_U8            devNum;
    GT_ETHERADDR     saAddrPtr;
    GT_ETHERADDR     daAddrPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    galtisMacAddr(&saAddrPtr, (GT_U8*)inArgs[1]);
    galtisMacAddr(&daAddrPtr, (GT_U8*)inArgs[2]);

    /* call cpss api function */
    result = cpssDxChBrgCntMacDaSaSet(devNum, &saAddrPtr, &daAddrPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgCntMacDaSaGet function
* @endinternal
*
* @brief   Gets a MAC DA and SA are monitored by Host
*         and Matrix counter groups on specified device.
*
* @note   APPLICABLE DEVICES:      ALL DxCh Devices.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static CMD_STATUS wrCpssDxChBrgCntMacDaSaGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS        result;

    GT_U8            devNum;
    GT_ETHERADDR     saAddrPtr;
    GT_ETHERADDR     daAddrPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgCntMacDaSaGet(devNum, &saAddrPtr, &daAddrPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%6b%6b", saAddrPtr.arEther, daAddrPtr.arEther);
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgCntBridgeIngressCntrModeSet function
* @endinternal
*
* @brief   Configures a specified Set of Bridge Ingress
*         counters to work in requested mode.
*
* @note   APPLICABLE DEVICES:      ALL DxCh Devices.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum or setMode
* @retval GT_OUT_OF_RANGE          - on port number bigger then 63 or counter set number
* @retval that is out of range of [0 -1].
*/
static CMD_STATUS wrCpssDxChBrgCntBridgeIngressCntrModeSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;

    GT_U8                               devNum;
    CPSS_DXCH_BRIDGE_CNTR_SET_ID_ENT    cntrSetId;
    CPSS_BRIDGE_INGR_CNTR_MODES_ENT     setMode;
    GT_PORT_NUM                               port;
    GT_U16                              vlan;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    cntrSetId = (CPSS_DXCH_BRIDGE_CNTR_SET_ID_ENT)inArgs[1];
    setMode = (CPSS_BRIDGE_INGR_CNTR_MODES_ENT)inArgs[2];
    port = (GT_PORT_NUM)inArgs[3];
    vlan = (GT_U16)inArgs[4];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, port);

    /* call cpss api function */
    result = cpssDxChBrgCntBridgeIngressCntrModeSet(devNum, cntrSetId,
                                                  setMode, port, vlan);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgCntBridgeIngressCntrModeGet function
* @endinternal
*
* @brief   Gets the mode (port number and VLAN Id as well) of specified
*         Bridge Ingress counters Set.
*
* @note   APPLICABLE DEVICES:      ALL DxCh Devices.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum or setMode
* @retval GT_OUT_OF_RANGE          - on port number bigger then 63 or counter set number
* @retval that is out of range of [0 -1].
*/
static CMD_STATUS wrCpssDxChBrgCntBridgeIngressCntrModeGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;

    GT_U8                               devNum;
    CPSS_DXCH_BRIDGE_CNTR_SET_ID_ENT    cntrSetId;
    CPSS_BRIDGE_INGR_CNTR_MODES_ENT     setMode;
    GT_PORT_NUM                         portNum;
    GT_U16                              vlan;
    GT_HW_DEV_NUM                       tmpHwDevNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    cntrSetId = (CPSS_DXCH_BRIDGE_CNTR_SET_ID_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgCntBridgeIngressCntrModeGet(devNum, cntrSetId,
                                                          &setMode,
                                                   &portNum, &vlan);

    tmpHwDevNum = devNum;
    CONVERT_BACK_DEV_PORT_DATA_MAC(tmpHwDevNum, portNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d", setMode, portNum, vlan);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgCntDropCntrGet function
* @endinternal
*
* @brief   Gets the Bridge Ingress Drop Counter of specified device.
*
* @note   APPLICABLE DEVICES:      ALL DxCh Devices.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static CMD_STATUS wrCpssDxChBrgCntDropCntrGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;

    GT_U8                               devNum;
    GT_U32                              dropCntPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = pg_wrap_cpssDxChBrgCntDropCntrGet(devNum, &dropCntPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", dropCntPtr);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgCntDropCntrSet function
* @endinternal
*
* @brief   Sets the Bridge Ingress Drop Counter of specified device.
*
* @note   APPLICABLE DEVICES:      ALL DxCh Devices.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum or invalid Drop Counter Value.
*/
static CMD_STATUS wrCpssDxChBrgCntDropCntrSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;

    GT_U8                               devNum;
    GT_U32                              dropCnt;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    dropCnt = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = pg_wrap_cpssDxChBrgCntDropCntrSet(devNum, dropCnt);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgCntHostGroupCntrsGet function
* @endinternal
*
* @brief   Gets Bridge Host group counters value of specified device.
*
* @note   APPLICABLE DEVICES:      ALL DxCh Devices.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note The Bridge Host group of counters are clear-on-read.
*
*/
static CMD_STATUS wrCpssDxChBrgCntHostGroupCntrsGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                result;

    GT_U8                                    devNum;
    CPSS_DXCH_BRIDGE_HOST_CNTR_STC           hostGroupCntPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = pg_wrap_cpssDxChBrgCntHostGroupCntrsGet(devNum, &hostGroupCntPtr);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    inFields[0] = hostGroupCntPtr.gtHostInPkts;
    inFields[1] = hostGroupCntPtr.gtHostOutPkts;
    inFields[2] = hostGroupCntPtr.gtHostOutBroadcastPkts;
    inFields[3] = hostGroupCntPtr.gtHostOutMulticastPkts;

    /* pack and output table fields */
    fieldOutput("%d%d%d%d", inFields[0], inFields[1],
                            inFields[2], inFields[3]);

    galtisOutput(outArgs, GT_OK, "%f");
    return CMD_OK;
}

/******************************************************************************/
static CMD_STATUS wrCpssDxChBrgCntHostGroupCntrsEndGet

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
* @internal wrCpssDxChBrgCntMatrixGroupCntrsGet function
* @endinternal
*
* @brief   Gets Bridge Matrix counter value of specified device.
*
* @note   APPLICABLE DEVICES:      ALL DxCh Devices.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note The Bridge Matrix counter is clear-on-read.
*
*/
static CMD_STATUS wrCpssDxChBrgCntMatrixGroupCntrsGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;

    GT_U8                               devNum;
    GT_U32                              matrixCntSaDaPktsPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = pg_wrap_cpssDxChBrgCntMatrixGroupCntrsGet(devNum, &matrixCntSaDaPktsPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", matrixCntSaDaPktsPtr);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgCntBridgeIngressCntrsGet function
* @endinternal
*
* @brief   Gets a Bridge ingress Port/VLAN/Device counters from
*         specified counter set.
*
* @note   APPLICABLE DEVICES:      ALL DxCh Devices.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - on counter set number that is out of range of [0-1]
*
* @note The Bridge Ingress group of counters are clear-on-read.
*
*/
static CMD_STATUS wrCpssDxChBrgCntBridgeIngressCntrsGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                result;

    GT_U8                                    devNum;
    CPSS_DXCH_BRIDGE_CNTR_SET_ID_ENT         cntrSetId;
    CPSS_BRIDGE_INGRESS_CNTR_STC             ingressCntrPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    cntrSetId = (CPSS_DXCH_BRIDGE_CNTR_SET_ID_ENT)inArgs[1];

    /* call cpss api function */
    result = pg_wrap_cpssDxChBrgCntBridgeIngressCntrsGet(devNum, cntrSetId,
                                                   &ingressCntrPtr);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    inFields[0] = ingressCntrPtr.gtBrgInFrames;
    inFields[1] = ingressCntrPtr.gtBrgVlanIngFilterDisc;
    inFields[2] = ingressCntrPtr.gtBrgSecFilterDisc;
    inFields[3] = ingressCntrPtr.gtBrgLocalPropDisc;

    /* pack and output table fields */
    fieldOutput("%d%d%d%d", inFields[0], inFields[1],
                            inFields[2], inFields[3]);

    galtisOutput(outArgs, GT_OK, "%f");
    return CMD_OK;
}

/******************************************************************************/
static CMD_STATUS wrCpssDxChBrgCntBridgeIngressCntrsEndGet

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
* @internal wrCpssDxChBrgCntLearnedEntryDiscGet function
* @endinternal
*
* @brief   Gets the total number of source addresses the were
*         not learned due to bridge internal congestion.
*
* @note   APPLICABLE DEVICES:      ALL DxCh Devices.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hw error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note Learned Entry Discards Counter is clear-on-read.
*
*/
static CMD_STATUS wrCpssDxChBrgCntLearnedEntryDiscGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;

    GT_U8                               devNum;
    GT_U32                              countValuePtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = pg_wrap_cpssDxChBrgCntLearnedEntryDiscGet(devNum, &countValuePtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", countValuePtr);
    return CMD_OK;
}

/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{

    {"cpssDxChBrgCntDropCntrModeSet",
        &wrCpssDxChBrgCntDropCntrModeSet,
        2, 0},

    {"cpssDxChBrgCntDropCntrModeGet",
        &wrCpssDxChBrgCntDropCntrModeGet,
        1, 0},

    {"cpssDxChBrgCntMacDaSaSet",
        &wrCpssDxChBrgCntMacDaSaSet,
        3, 0},

    {"cpssDxChBrgCntMacDaSaGet",
        &wrCpssDxChBrgCntMacDaSaGet,
        1, 0},

    {"cpssDxChBrgCntBridgeIngressCntrModeSet",
        &wrCpssDxChBrgCntBridgeIngressCntrModeSet,
        5, 0},

    {"cpssDxChBrgCntBridgeIngressCntrModeGet",
        &wrCpssDxChBrgCntBridgeIngressCntrModeGet,
        2, 0},

    {"cpssDxChBrgCntDropCntrGet",
        &wrCpssDxChBrgCntDropCntrGet,
        1, 0},

    {"cpssDxChBrgCntDropCntrSet",
        &wrCpssDxChBrgCntDropCntrSet,
        2, 0},

    {"cpssDxChBrgCntHostGroupCntrsGetFirst",
        &wrCpssDxChBrgCntHostGroupCntrsGet,
        1, 0},

    {"cpssDxChBrgCntHostGroupCntrsGetNext",
        &wrCpssDxChBrgCntHostGroupCntrsEndGet,
        1, 0},

    {"cpssDxChBrgCntMatrixGroupCntrsGet",
        &wrCpssDxChBrgCntMatrixGroupCntrsGet,
        1, 0},

    {"cpssDxChBrgCntBridgeIngressCntrsGetFirst",
        &wrCpssDxChBrgCntBridgeIngressCntrsGet,
        2, 0},

    {"cpssDxChBrgCntBridgeIngressCntrsGetNext",
        &wrCpssDxChBrgCntBridgeIngressCntrsEndGet,
        2, 0},

    {"cpssDxChBrgCntLearnedEntryDiscGet",
        &wrCpssDxChBrgCntLearnedEntryDiscGet,
        1, 0}

};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssDxChBridgeCount function
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
GT_STATUS cmdLibInitCpssDxChBridgeCount
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}



