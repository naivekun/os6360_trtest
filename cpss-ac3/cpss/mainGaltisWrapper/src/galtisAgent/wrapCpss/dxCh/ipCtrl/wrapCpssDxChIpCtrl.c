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
* @file wrapCpssDxChIpCtrl.c
*
* @brief Wrapper functions for IpCtrl cpss.dxCh functions
*
* @version   40
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>


/* Feature specific includes. */
#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIpCtrl.h>
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIpTypes.h>


/**
* @internal ipCtrlMultiPortGroupsBmpGet function
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
static void ipCtrlMultiPortGroupsBmpGet
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


/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChIpPortGroupCntSet
(
    IN  GT_U8                        devNum,
    IN CPSS_IP_CNT_SET_ENT           cntSet,
    IN CPSS_DXCH_IP_COUNTER_SET_STC  *countersPtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    ipCtrlMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChIpCntSet(devNum, cntSet, countersPtr);
    }
    else
    {
        return cpssDxChIpPortGroupCntSet(devNum, pgBmp, cntSet, countersPtr);
    }
}

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChIpPortGroupCntGet
(
    IN  GT_U8                         devNum,
    IN  CPSS_IP_CNT_SET_ENT           cntSet,
    OUT CPSS_DXCH_IP_COUNTER_SET_STC  *countersPtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    ipCtrlMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChIpCntGet(devNum, cntSet, countersPtr);
    }
    else
    {
        return cpssDxChIpPortGroupCntGet(devNum, pgBmp, cntSet, countersPtr);
    }
}

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChIpDropCntGet
(
    IN  GT_U8     devNum,
    OUT GT_U32    *dropPktsPtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    ipCtrlMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChIpDropCntGet(devNum, dropPktsPtr);
    }
    else
    {
        return cpssDxChIpPortGroupDropCntGet(devNum, pgBmp, dropPktsPtr);
    }
}

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChIpDropCntSet
(
    IN  GT_U8     devNum,
    IN GT_U32     dropPkts
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    ipCtrlMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChIpDropCntSet(devNum, dropPkts);
    }
    else
    {
        return cpssDxChIpPortGroupDropCntSet(devNum, pgBmp, dropPkts);
    }
}


/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChIpRouterBridgedPacketsExceptionCntGet
(
    IN  GT_U8     devNum,
    OUT GT_U32    *routerBridgedExceptionPktsPtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    ipCtrlMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChIpRouterBridgedPacketsExceptionCntGet(devNum,
                                                             routerBridgedExceptionPktsPtr);
    }
    else
    {
        return cpssDxChIpPortGroupRouterBridgedPacketsExceptionCntGet(devNum,
                                                                      pgBmp,
                                                                      routerBridgedExceptionPktsPtr);
    }
}

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChIpRouterBridgedPacketsExceptionCntSet
(
    IN  GT_U8     devNum,
    IN GT_U32     routerBridgedExceptionPkts
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    ipCtrlMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChIpRouterBridgedPacketsExceptionCntSet(devNum,
                                                             routerBridgedExceptionPkts);
    }
    else
    {
        return cpssDxChIpPortGroupRouterBridgedPacketsExceptionCntSet(devNum,
                                                                      pgBmp,
                                                                      routerBridgedExceptionPkts);
    }
}

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChIpPortGroupMultiTargetQueueFullDropCntGet
(
    IN  GT_U8     devNum,
    OUT GT_U32    *dropPktsPtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    ipCtrlMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChIpMultiTargetQueueFullDropCntGet(devNum, dropPktsPtr);
    }
    else
    {
        return cpssDxChIpPortGroupMultiTargetQueueFullDropCntGet(devNum, pgBmp,
                                                                 dropPktsPtr);
    }
}

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChIpMultiTargetQueueFullDropCntSet
(
    IN  GT_U8    devNum,
    IN GT_U32    dropPkts
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    ipCtrlMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChIpMultiTargetQueueFullDropCntSet(devNum, dropPkts);
    }
    else
    {
        return cpssDxChIpPortGroupMultiTargetQueueFullDropCntSet(devNum, pgBmp,
                                                                 dropPkts);
    }
}

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChIpMllSkippedEntriesCountersGet
(
    IN  GT_U8     devNum,
    OUT GT_U32    *skipCounterPtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    ipCtrlMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChIpMllSkippedEntriesCountersGet(devNum, skipCounterPtr);
    }
    else
    {
        return cpssDxChIpMllPortGroupSkippedEntriesCountersGet(devNum, pgBmp, skipCounterPtr);
    }
}


/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChIpMllCntGet
(
    IN  GT_U8     devNum,
    IN GT_U32     mllCntSet,
    OUT GT_U32    *mllOutMCPktsPtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    ipCtrlMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChIpMllCntGet(devNum, mllCntSet, mllOutMCPktsPtr);
    }
    else
    {
        return cpssDxChIpPortGroupMllCntGet(devNum, pgBmp, mllCntSet,
                                            mllOutMCPktsPtr);
    }
}

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChIpPortGroupMllCntSet
(
    IN  GT_U8     devNum,
    IN GT_U32     mllCntSet,
    IN GT_U32     mllOutMCPkts
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    ipCtrlMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChIpMllCntSet(devNum, mllCntSet, mllOutMCPkts);
    }
    else
    {
        return cpssDxChIpPortGroupMllCntSet(devNum, pgBmp, mllCntSet,
                                            mllOutMCPkts);
    }
}

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChIpMllSilentDropCntGet
(
    IN  GT_U8     devNum,
    IN  GT_U32    *silentDropPktsPtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    ipCtrlMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChIpMllSilentDropCntGet(devNum, silentDropPktsPtr);
    }
    else
    {
        return cpssDxChIpMllPortGroupSilentDropCntGet(devNum, pgBmp, silentDropPktsPtr);
    }
}


/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChIpMultiTargetRateShaperSet
(
    IN   GT_U8      devNum,
    IN   GT_BOOL    multiTargetRateShaperEnable,
    IN   GT_U32     windowSize
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    ipCtrlMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChIpMultiTargetRateShaperSet(devNum,
                                                  multiTargetRateShaperEnable,
                                                  windowSize);
    }
    else
    {
        return cpssDxChIpPortGroupMultiTargetRateShaperSet(devNum, pgBmp,
                                                    multiTargetRateShaperEnable,
                                                    windowSize);
    }
}

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChIpMultiTargetRateShaperGet
(
    IN    GT_U8      devNum,
    OUT   GT_BOOL    *multiTargetRateShaperEnablePtr,
    OUT   GT_U32     *windowSizePtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    ipCtrlMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChIpMultiTargetRateShaperGet(devNum,
                                                multiTargetRateShaperEnablePtr,
                                                windowSizePtr);
    }
    else
    {
        return cpssDxChIpPortGroupMultiTargetRateShaperGet(devNum, pgBmp,
                                                multiTargetRateShaperEnablePtr,
                                                windowSizePtr);
    }
}

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChIpPortGroupRouterSourceIdSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_IP_UNICAST_MULTICAST_ENT   ucMcSet,
    IN  GT_U32                          sourceId,
    IN  GT_U32                          sourceIdMask
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    ipCtrlMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChIpRouterSourceIdSet(devNum, ucMcSet, sourceId, sourceIdMask);
    }
    else
    {
        return cpssDxChIpPortGroupRouterSourceIdSet(devNum, pgBmp,
                                                    ucMcSet, sourceId, sourceIdMask);
    }
}

/* Port Group and Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChIpPortGroupRouterSourceIdGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_IP_UNICAST_MULTICAST_ENT   ucMcSet,
    OUT GT_U32                          *sourceIdPtr,
    OUT GT_U32                          *sourceIdMaskPtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    ipCtrlMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChIpRouterSourceIdGet(devNum, ucMcSet, sourceIdPtr, sourceIdMaskPtr);
    }
    else
    {
        return cpssDxChIpPortGroupRouterSourceIdGet(devNum, pgBmp,
                                                    ucMcSet, sourceIdPtr, sourceIdMaskPtr);
    }
}

/**
* @internal wrCpssDxChIpSpecialRouterTriggerEnable function
* @endinternal
*
* @brief   Sets the special router trigger enable modes for packets with bridge
*         command other then FORWARD or MIRROR.
*
* @note   APPLICABLE DEVICES:      All DxCh2 devices.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_FAIL                  - on error
*
* @note none.
*
*/
static CMD_STATUS wrCpssDxChIpSpecialRouterTriggerEnable

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                        result;

    GT_U8                            devNum;
    CPSS_DXCH_IP_BRG_EXCP_CMD_ENT    bridgeExceptionCmd;
    GT_BOOL                          enableRouterTrigger;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    bridgeExceptionCmd = (CPSS_DXCH_IP_BRG_EXCP_CMD_ENT)inArgs[1];
    enableRouterTrigger = (GT_BOOL)inArgs[2];

    /* call cpss api function */
    result = cpssDxChIpSpecialRouterTriggerEnable(devNum, bridgeExceptionCmd,
                                                         enableRouterTrigger);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChIpSpecialRouterTriggerEnableGet function
* @endinternal
*
* @brief   Gets the special router trigger enable modes for packets with bridge
*         command other then FORWARD or MIRROR.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; xCat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or bridgeExceptionCmd
* @retval GT_BAD_PTR               - on NULL ptr
*/
static CMD_STATUS wrCpssDxChIpSpecialRouterTriggerEnableGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                        result;

    GT_U8                            devNum;
    CPSS_DXCH_IP_BRG_EXCP_CMD_ENT    bridgeExceptionCmd;
    GT_BOOL                          enableRouterTrigger;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    bridgeExceptionCmd = (CPSS_DXCH_IP_BRG_EXCP_CMD_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChIpSpecialRouterTriggerEnableGet(devNum, bridgeExceptionCmd,
                                                         &enableRouterTrigger);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enableRouterTrigger);
    return CMD_OK;
}

/**
* @internal wrCpssDxChIpExceptionCommandSet function
* @endinternal
*
* @brief   set a specific excption commnad.
*
* @note   APPLICABLE DEVICES:      All DxCh2 devices.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_FAIL                  - on error
*
* @note none.
*       GalTis:
*
*/
static CMD_STATUS wrCpssDxChIpExceptionCommandSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                        result;

    GT_U8                            devNum;
    CPSS_DXCH_IP_EXCEPTION_TYPE_ENT  exceptionType;
    CPSS_IP_PROTOCOL_STACK_ENT       protocolStack;
    CPSS_PACKET_CMD_ENT              command;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    exceptionType = (CPSS_DXCH_IP_EXCEPTION_TYPE_ENT)inArgs[1];
    protocolStack = (CPSS_IP_PROTOCOL_STACK_ENT)inArgs[2];
    command = (CPSS_PACKET_CMD_ENT)inArgs[3];

    /* call cpss api function */
    result = cpssDxChIpExceptionCommandSet(devNum, exceptionType,
                                          protocolStack, command);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}
/**
* @internal wrCpssDxChIpExceptionCommandGetExt function
* @endinternal
*
* @brief   set a specific exception command.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - on invalid hardware value read
*
* @note
*       GalTis:
*
*/
static CMD_STATUS wrCpssDxChIpExceptionCommandGetExt

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                        result;
    GT_U8                            devNum;
    CPSS_DXCH_IP_EXCEPTION_TYPE_ENT  exceptionType;
    CPSS_IP_PROTOCOL_STACK_ENT       protocolStack;
    CPSS_PACKET_CMD_ENT              commandPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    exceptionType = (CPSS_DXCH_IP_EXCEPTION_TYPE_ENT)inArgs[1];
    protocolStack = (CPSS_IP_PROTOCOL_STACK_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssDxChIpExceptionCommandGet(devNum, exceptionType,
                                          protocolStack, &commandPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", commandPtr);
    return CMD_OK;
}

/**
* @internal wrCpssDxChIpUcRouteAgingModeSet function
* @endinternal
*
* @brief   Sets the global route aging modes.
*
* @note   APPLICABLE DEVICES:      All DxCh2 devices.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static CMD_STATUS wrCpssDxChIpUcRouteAgingModeSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS            result;

    GT_U8                devNum;
    GT_BOOL              refreshEnable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    refreshEnable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChIpUcRouteAgingModeSet(devNum, refreshEnable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChIpUcRouteAgingModeGet function
* @endinternal
*
* @brief   Gets the global route aging modes.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; xCat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
static CMD_STATUS wrCpssDxChIpUcRouteAgingModeGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS            result;

    GT_U8                devNum;
    GT_BOOL              refreshEnable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChIpUcRouteAgingModeGet(devNum, &refreshEnable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", refreshEnable);
    return CMD_OK;
}









/**
* @internal wrCpssDxChIpRouterSourceIdSet function
* @endinternal
*
* @brief   set the router source id assignmnet.
*
* @note   APPLICABLE DEVICES:      All DxCh2 devices.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static CMD_STATUS wrCpssDxChIpRouterSourceIdSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;

    GT_U8                           devNum;
    CPSS_IP_UNICAST_MULTICAST_ENT   ucMcSet;
    GT_U32                          sourceId;
    GT_U32                          sourceIdMask = 0xFFF;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    ucMcSet = (CPSS_IP_UNICAST_MULTICAST_ENT)inArgs[1];
    sourceId = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = pg_wrap_cpssDxChIpPortGroupRouterSourceIdSet(devNum, ucMcSet, sourceId, sourceIdMask);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}








/**
* @internal wrCpssDxChIpRouterSourceIdGet function
* @endinternal
*
* @brief   get the router source id.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChIpRouterSourceIdGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;

    GT_U8                           devNum;
    CPSS_IP_UNICAST_MULTICAST_ENT   ucMcSet;
    GT_U32                          sourceId;
    GT_U32                          sourceIdMask;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    ucMcSet = (CPSS_IP_UNICAST_MULTICAST_ENT)inArgs[1];

    /* call cpss api function */
    result = pg_wrap_cpssDxChIpPortGroupRouterSourceIdGet(devNum, ucMcSet, &sourceId, &sourceIdMask);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", sourceId);
    return CMD_OK;
}



/**
* @internal wrCpssDxChIpRouterSourceId_1Set function
* @endinternal
*
* @brief   set the router source id assignmnet.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChIpRouterSourceId_1Set

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;

    GT_U8                           devNum;
    CPSS_IP_UNICAST_MULTICAST_ENT   ucMcSet;
    GT_U32                          sourceIdMask = 0xFFF;
    GT_U32                          sourceId;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    ucMcSet = (CPSS_IP_UNICAST_MULTICAST_ENT)inArgs[1];
    sourceId = (GT_U32)inArgs[2];
    sourceIdMask = (GT_U32)inArgs[3];

    /* call cpss api function */
    result = pg_wrap_cpssDxChIpPortGroupRouterSourceIdSet(devNum, ucMcSet, sourceId, sourceIdMask);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}









/**
* @internal wrCpssDxChIpRouterSourceId_1Get function
* @endinternal
*
* @brief   get the router source id.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChIpRouterSourceId_1Get

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;

    GT_U8                           devNum;
    CPSS_IP_UNICAST_MULTICAST_ENT   ucMcSet;
    GT_U32                          sourceId;
    GT_U32                          sourceIdMask;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    ucMcSet = (CPSS_IP_UNICAST_MULTICAST_ENT)inArgs[1];

    /* call cpss api function */
    result = pg_wrap_cpssDxChIpPortGroupRouterSourceIdGet(devNum, ucMcSet, &sourceId, &sourceIdMask);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d, %d", sourceId, sourceIdMask);
    return CMD_OK;
}













/**
* @internal wrCpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueSet function
* @endinternal
*
* @brief   Sets the multi-target TC queue assigned to multi-target Control
*         packets and to RPF Fail packets where the RPF Fail Command is assigned
*         from the MLL entry.
*
* @note   APPLICABLE DEVICES:      All DxCh2 devices.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note A packet is considered as a multi-target control if it is a FROM_CPU DSA
*       Tagged with DSA<Use_Vidx> = 1 or a multi-target packet that is also to
*       be mirrored to the CPU
*
*/
static CMD_STATUS wrCpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;

    GT_U8                           devNum;
    GT_U32                          ctrlMultiTargetTCQueue;
    GT_U32                          failRpfMultiTargetTCQueue;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    ctrlMultiTargetTCQueue = (GT_U32)inArgs[1];
    failRpfMultiTargetTCQueue = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueSet(devNum,
                                               ctrlMultiTargetTCQueue,
                                            failRpfMultiTargetTCQueue);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueGet function
* @endinternal
*
* @brief   Gets the multi-target TC queue assigned to multi-target Control
*         packets and to RPF Fail packets where the RPF Fail Command is assigned
*         from the MLL entry.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; xCat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL Ptr
*
* @note A packet is considered as a multi-target control if it is a FROM_CPU DSA
*       Tagged with DSA<Use_Vidx> = 1 or a multi-target packet that is also to
*       be mirrored to the CPU
*
*/
static CMD_STATUS wrCpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;

    GT_U8                           devNum;
    GT_U32                          ctrlMultiTargetTCQueue;
    GT_U32                          failRpfMultiTargetTCQueue;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueGet(devNum,
                                               &ctrlMultiTargetTCQueue,
                                            &failRpfMultiTargetTCQueue);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, " %d, %d", ctrlMultiTargetTCQueue,
                                             failRpfMultiTargetTCQueue);
    return CMD_OK;
}

/**
* @internal wrCpssDxChIpQosProfileToMultiTargetTCQueueMapSet function
* @endinternal
*
* @brief   Sets the Qos Profile to multi-target TC queue mapping.
*
* @note   APPLICABLE DEVICES:      All DxCh2 devices.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static CMD_STATUS wrCpssDxChIpQosProfileToMultiTargetTCQueueMapSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;

    GT_U8                           devNum;
    GT_U32                          qosProfile;
    GT_U32                          multiTargeTCQueue;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    qosProfile = (GT_U32)inArgs[1];
    multiTargeTCQueue = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChIpQosProfileToMultiTargetTCQueueMapSet(devNum, qosProfile,
                                                             multiTargeTCQueue);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChIpQosProfileToMultiTargetTCQueueMapGet function
* @endinternal
*
* @brief   Sets the Qos Profile to multi-target TC queue mapping.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; xCat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChIpQosProfileToMultiTargetTCQueueMapGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;

    GT_U8                           devNum;
    GT_U32                          qosProfile;
    GT_U32                          multiTargeTCQueue;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    qosProfile = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChIpQosProfileToMultiTargetTCQueueMapGet(devNum, qosProfile,
                                                             &multiTargeTCQueue);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", multiTargeTCQueue);
    return CMD_OK;
}






/**
* @internal wrCpssDxChIpTcDpToMultiTargetTcQueueMapSet function
* @endinternal
*
* @brief   Sets the packet's (TC,DP) to multi-target TC queue mapping.
*         (It is used for packets received on cascade ports configured to work in extended QoS mode)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChIpTcDpToMultiTargetTcQueueMapSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;

    GT_U8                           devNum;
    GT_U32                          tc;
    CPSS_DP_LEVEL_ENT               dp;
    GT_U32                          multiTargeTCQueue;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum            = (GT_U8)inArgs[0];
    tc                = (GT_U32)inArgs[1];
    dp                = (CPSS_DP_LEVEL_ENT)inArgs[2];
    multiTargeTCQueue = (GT_U32)inArgs[3];

    /* call cpss api function */
    result = cpssDxChIpTcDpToMultiTargetTcQueueMapSet(devNum, tc, dp, multiTargeTCQueue);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChIpTcDpToMultiTargetTCQueueMapGet function
* @endinternal
*
* @brief   Gets the packet's (TC,DP) to multi-target TC queue mapping.
*         (It is used for packets received on cascade ports configured to work in extended QoS mode)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChIpTcDpToMultiTargetTCQueueMapGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_U8                           devNum;
    GT_U32                          tc;
    CPSS_DP_LEVEL_ENT               dp;
    GT_U32                          multiTargeTCQueue;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    tc     = (GT_U32)inArgs[1];
    dp     = (CPSS_DP_LEVEL_ENT)inArgs[2];
    /* call cpss api function */
    result = cpssDxChIpTcDpToMultiTargetTcQueueMapGet(devNum, tc, dp, &multiTargeTCQueue);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", multiTargeTCQueue);
    return CMD_OK;
}


/**
* @internal wrCpssDxChIpMultiTargetQueueFullDropCntGet function
* @endinternal
*
* @brief   Get the multi target queue full drop packet counter.
*
* @note   APPLICABLE DEVICES:      All DxCh2 devices.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static CMD_STATUS wrCpssDxChIpMultiTargetQueueFullDropCntGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;

    GT_U8                           devNum;
    GT_U32                          dropPktsPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = pg_wrap_cpssDxChIpPortGroupMultiTargetQueueFullDropCntGet(devNum,
                                                                  &dropPktsPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", dropPktsPtr);
    return CMD_OK;
}

/**
* @internal wrCpssDxChIpMultiTargetQueueFullDropCntSet function
* @endinternal
*
* @brief   set the multi target queue full drop packet counter.
*
* @note   APPLICABLE DEVICES:      All DxCh2 devices.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static CMD_STATUS wrCpssDxChIpMultiTargetQueueFullDropCntSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;

    GT_U8                           devNum;
    GT_U32                          dropPkts;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    dropPkts = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = pg_wrap_cpssDxChIpMultiTargetQueueFullDropCntSet(devNum, dropPkts);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChIpMultiTargetTCQueueSchedModeSet function
* @endinternal
*
* @brief   sets the multi-target TC queue scheduling mode.
*
* @note   APPLICABLE DEVICES:      All DxCh2 devices.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note none.
*
*/
static CMD_STATUS wrCpssDxChIpMultiTargetTCQueueSchedModeSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;

    GT_U8                                       devNum;
    GT_U32                                      multiTargetTcQueue;
    CPSS_DXCH_IP_MT_TC_QUEUE_SCHED_MODE_ENT     schedulingMode;
    GT_U32                                      queueWeight;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    multiTargetTcQueue = (GT_U32)inArgs[1];
    schedulingMode = (CPSS_DXCH_IP_MT_TC_QUEUE_SCHED_MODE_ENT)inArgs[2];
    queueWeight = (GT_U32)inArgs[3];

    /* call cpss api function */
    result = cpssDxChIpMultiTargetTCQueueSchedModeSet(devNum,
                                          multiTargetTcQueue,
                                              schedulingMode,
                                                 queueWeight);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChIpMultiTargetTCQueueSchedModeGet function
* @endinternal
*
* @brief   gets the multi-target TC queue scheduling mode.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; xCat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChIpMultiTargetTCQueueSchedModeGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;

    GT_U8                                       devNum;
    GT_U32                                      multiTargetTcQueue;
    CPSS_DXCH_IP_MT_TC_QUEUE_SCHED_MODE_ENT     schedulingMode;
    GT_U32                                      queueWeight;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    multiTargetTcQueue = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChIpMultiTargetTCQueueSchedModeGet(devNum,
                                          multiTargetTcQueue,
                                              &schedulingMode,
                                                 &queueWeight);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d, %d",schedulingMode, queueWeight);
    return CMD_OK;
}

/**
* @internal wrCpssDxChIpBridgeServiceEnable function
* @endinternal
*
* @brief   enable/disable a router bridge service.
*
* @note   APPLICABLE DEVICES:      All DxCh2 devices.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note none.
*
*/
static CMD_STATUS wrCpssDxChIpBridgeServiceEnable

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                               result;

    GT_U8                                                   devNum;
    CPSS_DXCH_IP_BRG_SERVICE_ENT                            bridgeService;
    CPSS_DXCH_IP_BRG_SERVICE_ENABLE_DISABLE_MODE_ENT        enableDisableMode;
    GT_BOOL                                                 enableService;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    bridgeService = (CPSS_DXCH_IP_BRG_SERVICE_ENT)inArgs[1];
    enableDisableMode =
                    (CPSS_DXCH_IP_BRG_SERVICE_ENABLE_DISABLE_MODE_ENT)inArgs[2];
    enableService = (GT_BOOL)inArgs[3];

    /* call cpss api function */
    result = cpssDxChIpBridgeServiceEnable(devNum, bridgeService,
                                enableDisableMode, enableService);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChIpBridgeServiceEnableGet function
* @endinternal
*
* @brief   enable/disable a router bridge service.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; xCat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
static CMD_STATUS wrCpssDxChIpBridgeServiceEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8                                               devNum;
    CPSS_DXCH_IP_BRG_SERVICE_ENT                        bridgeService;
    CPSS_DXCH_IP_BRG_SERVICE_ENABLE_DISABLE_MODE_ENT    enableDisableMode;
    GT_BOOL                                             enableService;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    bridgeService = (CPSS_DXCH_IP_BRG_SERVICE_ENT)inArgs[1];
    enableDisableMode =
                    (CPSS_DXCH_IP_BRG_SERVICE_ENABLE_DISABLE_MODE_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssDxChIpBridgeServiceEnableGet(devNum, bridgeService,
                                            enableDisableMode, &enableService);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enableService);
    return CMD_OK;
}
/**
* @internal wrCpssDxChIpMllBridgeEnable function
* @endinternal
*
* @brief   enable/disable MLL based bridging.
*
* @note   APPLICABLE DEVICES:      All DxCh2 devices.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note none.
*
*/
static CMD_STATUS wrCpssDxChIpMllBridgeEnable

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                result;

    GT_U8                    devNum;
    GT_BOOL                  mllBridgeEnable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    mllBridgeEnable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChIpMllBridgeEnable(devNum, mllBridgeEnable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChIpMllBridgeEnableGet function
* @endinternal
*
* @brief   get state of MLL based bridging.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; xCat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL prt
*/
static CMD_STATUS wrCpssDxChIpMllBridgeEnableGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                result;

    GT_U8                    devNum;
    GT_BOOL                  mllBridgeEnable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChIpMllBridgeEnableGet(devNum, &mllBridgeEnable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mllBridgeEnable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChIpMultiTargetRateShaperSet function
* @endinternal
*
* @brief   set the Multi target Rate shaper params.
*
* @note   APPLICABLE DEVICES:      All DxCh2 devices.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static CMD_STATUS wrCpssDxChIpMultiTargetRateShaperSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;

    GT_U8                                       devNum;
    GT_BOOL                                     multiTargetRateShaperEnable;
    GT_U32                                      windowSize;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    multiTargetRateShaperEnable = (GT_BOOL)inArgs[1];
    windowSize = (GT_U32)inArgs[2];

    /* call port group api function */
    result = pg_wrap_cpssDxChIpMultiTargetRateShaperSet(devNum,
                                                    multiTargetRateShaperEnable,
                                                    windowSize);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChIpMultiTargetRateShaperGet function
* @endinternal
*
* @brief   set the Multi target Rate shaper params.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; xCat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChIpMultiTargetRateShaperGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;

    GT_U8                                       devNum;
    GT_BOOL                                     multiTargetRateShaperEnable;
    GT_U32                                      windowSize;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];


    /* call port group api function */
    result = pg_wrap_cpssDxChIpMultiTargetRateShaperGet(devNum,
                                                    &multiTargetRateShaperEnable,
                                                    &windowSize);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d, %d",multiTargetRateShaperEnable, windowSize);
    return CMD_OK;
}

/**
* @internal wrCpssDxChIpMultiTargetUcSchedModeSet function
* @endinternal
*
* @brief   set the Multi target/unicast sheduler mode.
*
* @note   APPLICABLE DEVICES:      All DxCh2 devices.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static CMD_STATUS wrCpssDxChIpMultiTargetUcSchedModeSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;

    GT_U8                                       devNum;
    GT_BOOL                                     ucSPEnable;
    GT_U32                                      ucWeight;
    GT_U32                                      mcWeight;
    CPSS_DXCH_IP_MT_UC_SCHED_MTU_ENT            schedMtu;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    ucSPEnable = (GT_BOOL)inArgs[1];
    ucWeight = (GT_U32)inArgs[2];
    mcWeight = (GT_U32)inArgs[3];
    schedMtu = (CPSS_DXCH_IP_MT_UC_SCHED_MTU_ENT)inArgs[4];

    /* call cpss api function */
    result = cpssDxChIpMultiTargetUcSchedModeSet(devNum, ucSPEnable, ucWeight,
                                                           mcWeight, schedMtu);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChIpMultiTargetUcSchedModeGet function
* @endinternal
*
* @brief   get the Multi target/unicast sheduler mode.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; xCat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
static CMD_STATUS wrCpssDxChIpMultiTargetUcSchedModeGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;

    GT_U8                                       devNum;
    GT_BOOL                                     ucSPEnable;
    GT_U32                                      ucWeight;
    GT_U32                                      mcWeight;
    CPSS_DXCH_IP_MT_UC_SCHED_MTU_ENT            schedMtu;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChIpMultiTargetUcSchedModeGet(devNum, &ucSPEnable, &ucWeight,
                                                          &mcWeight, &schedMtu);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d, %d, %d, %d", ucSPEnable, ucWeight,
                                                            mcWeight, schedMtu);
    return CMD_OK;
}

/**
* @internal wrCpssDxChIpArpBcModeSet function
* @endinternal
*
* @brief   set a arp broadcast mode.
*
* @note   APPLICABLE DEVICES:      All DxCh2 devices.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_FAIL                  - on error
*
* @note none.
*       GalTis:
*
*/
static CMD_STATUS wrCpssDxChIpArpBcModeSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;

    GT_U8                                       devNum;
    CPSS_PACKET_CMD_ENT                         arpBcMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    arpBcMode = (CPSS_PACKET_CMD_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChIpArpBcModeSet(devNum, arpBcMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChIpArpBcModeGet function
* @endinternal
*
* @brief   get a arp broadcast mode.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; xCat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note
*       GalTis:
*
*/
static CMD_STATUS wrCpssDxChIpArpBcModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8                       devNum;
    CPSS_PACKET_CMD_ENT         arpBcMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChIpArpBcModeGet(devNum, &arpBcMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", arpBcMode);
    return CMD_OK;
}

/**
* @internal wrCpssDxChIpPortRoutingEnable function
* @endinternal
*
* @brief   Enable multicast/unicast IPv4/v6 routing on a port.
*
* @note   APPLICABLE DEVICES:      All DxCh2 devices.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note none
*
*/
static CMD_STATUS wrCpssDxChIpPortRoutingEnable

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                       result;

    GT_U8                                           devNum;
    GT_PORT_NUM                                           portNum;
    CPSS_IP_UNICAST_MULTICAST_ENT                   ucMcEnable;
    CPSS_IP_PROTOCOL_STACK_ENT                      protocolStack;
    GT_BOOL                                         enableRouting;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];
    ucMcEnable = (CPSS_IP_UNICAST_MULTICAST_ENT)inArgs[2];
    protocolStack = (CPSS_IP_PROTOCOL_STACK_ENT)inArgs[3];
    enableRouting = (GT_BOOL)inArgs[4];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChIpPortRoutingEnable(devNum, portNum, ucMcEnable,
                                        protocolStack, enableRouting);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChIpPortRoutingEnableGet function
* @endinternal
*
* @brief   Get status of multicast/unicast IPv4/v6 routing on a port.
*
* @note   APPLICABLE DEVICES:      DxCh2 and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - wrong protocolStack
*/
static CMD_STATUS wrCpssDxChIpPortRoutingEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                       result;

    GT_U8                                           devNum;
    GT_PORT_NUM                                           portNum;
    CPSS_IP_UNICAST_MULTICAST_ENT                   ucMcEnable;
    CPSS_IP_PROTOCOL_STACK_ENT                      protocolStack;
    GT_BOOL                                         enableRouting;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];
    ucMcEnable = (CPSS_IP_UNICAST_MULTICAST_ENT)inArgs[2];
    protocolStack = (CPSS_IP_PROTOCOL_STACK_ENT)inArgs[3];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChIpPortRoutingEnableGet(devNum, portNum, ucMcEnable,
                                        protocolStack, &enableRouting);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enableRouting);
    return CMD_OK;
}

/**
* @internal wrCpssDxChIpPortFcoeForwardingEnable function
* @endinternal
*
* @brief   Enable FCoE Forwarding on a port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChIpPortFcoeForwardingEnable

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS      result;

    GT_U8           devNum;
    GT_PORT_NUM     portNum;
    GT_BOOL         enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChIpPortFcoeForwardingEnableSet(devNum, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChIpPortFcoeForwardingEnableGet function
* @endinternal
*
* @brief   Get status of FCoE Forwarding on a port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - wrong protocolStack
* @retval GT_BAD_PTR               - on NULL pointer
*/
static CMD_STATUS wrCpssDxChIpPortFcoeForwardingEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;

    GT_U8           devNum;
    GT_PORT_NUM     portNum;
    GT_BOOL         enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChIpPortFcoeForwardingEnableGet(devNum, portNum, &enable);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChIpQosProfileToRouteEntryMapSet function
* @endinternal
*
* @brief   Sets the QoS profile to route entry offset mapping table.
*
* @note   APPLICABLE DEVICES:      All DxCh2 devices.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_FAIL                  - on error
*
* @note none
*
*/
static CMD_STATUS wrCpssDxChIpQosProfileToRouteEntryMapSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;

    GT_U8                                       devNum;
    GT_U32                                      qosProfile;
    GT_U32                                      routeEntryOffset;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    qosProfile = (GT_U32)inArgs[1];
    routeEntryOffset = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChIpQosProfileToRouteEntryMapSet(devNum, qosProfile,
                                                      routeEntryOffset);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChIpQosProfileToRouteEntryMapGet function
* @endinternal
*
* @brief   gets the QoS profile to route entry offset mapping table.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; xCat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*
* @note In QoS-based routing, the Route table entry is selected according to the
*       following index calculation:
*       1. in DxCh2, DxCh3, xCat: <Route Entry Index> +
*       (QoSProfile-to-Route-Block-Offset(QoSProfile) %
*       (<Number of Paths> + 1)
*       2. in Lion : <Route Entry Index> +
*       Floor(QoSProfile-to-Route-Block-Offset(QoSProfile)
*       (<Number of Paths> + 1) / 8)
*
*/
static CMD_STATUS wrCpssDxChIpQosProfileToRouteEntryMapGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;

    GT_U8                                       devNum;
    GT_U32                                      qosProfile;
    GT_U32                                      routeEntryOffset;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    qosProfile = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChIpQosProfileToRouteEntryMapGet(devNum, qosProfile,
                                                      &routeEntryOffset);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", routeEntryOffset);
    return CMD_OK;
}

/**
* @internal wrCpssDxChIpRoutingEnable function
* @endinternal
*
* @brief   globally enable/disable routing.
*
* @note   APPLICABLE DEVICES:      DxCh2 and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - wrong devNum
*
* @note the ASIC defualt is routing enabled.
*
*/
static CMD_STATUS wrCpssDxChIpRoutingEnable

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;

    GT_U8                   devNum;
    GT_BOOL                 enableRouting;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    enableRouting = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChIpRoutingEnable(devNum, enableRouting);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChIpRoutingEnableGet function
* @endinternal
*
* @brief   Get global routing status
*
* @note   APPLICABLE DEVICES:      DxCh2 and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static CMD_STATUS wrCpssDxChIpRoutingEnableGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;

    GT_U8                   devNum;
    GT_BOOL                 enableRouting;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChIpRoutingEnableGet(devNum, &enableRouting);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enableRouting);
    return CMD_OK;
}


/*  table cpssDxChIpCnt global variable   */

static CPSS_IP_CNT_SET_ENT    cntSetCnt;

/**
* @internal wrCpssDxChIpCntGetFirst function
* @endinternal
*
* @brief   Return the IP counter set requested.
*
* @note   APPLICABLE DEVICES:      All DxCh2 devices.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
*
* @note This function doesn't handle counters overflow.
*
*/
static CMD_STATUS wrCpssDxChIpCntGetFirst

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                      result;

    GT_U8                          devNum;
    CPSS_DXCH_IP_COUNTER_SET_STC   counters;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    cntSetCnt = 0;

    /* call cpss api function */
    result = pg_wrap_cpssDxChIpPortGroupCntGet(devNum, cntSetCnt, &counters);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    inFields[0] = cntSetCnt;
    inFields[1] = counters.inUcPkts;
    inFields[2] = counters.inMcPkts;
    inFields[3] = counters.inUcNonRoutedExcpPkts;
    inFields[4] = counters.inUcNonRoutedNonExcpPkts;
    inFields[5] = counters.inMcNonRoutedExcpPkts;
    inFields[6] = counters.inMcNonRoutedNonExcpPkts;
    inFields[7] = counters.inUcTrappedMirrorPkts;
    inFields[8] = counters.inMcTrappedMirrorPkts;
    inFields[9] = counters.mcRfpFailPkts;
    inFields[10] = counters.outUcRoutedPkts;

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d%d%d%d", inFields[0],  inFields[1],
                  inFields[2], inFields[3], inFields[4],  inFields[5],
                  inFields[6], inFields[7], inFields[8],  inFields[9],
                  inFields[10]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");

    cntSetCnt++;

    return CMD_OK;
}

/******************************************************************************/
static CMD_STATUS wrCpssDxChIpCntGetNext

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                      result;

    GT_U8                          devNum;
    CPSS_DXCH_IP_COUNTER_SET_STC   counters;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if (cntSetCnt > 3)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = pg_wrap_cpssDxChIpPortGroupCntGet(devNum, cntSetCnt, &counters);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    inFields[0] = cntSetCnt;
    inFields[1] = counters.inUcPkts;
    inFields[2] = counters.inMcPkts;
    inFields[3] = counters.inUcNonRoutedExcpPkts;
    inFields[4] = counters.inUcNonRoutedNonExcpPkts;
    inFields[5] = counters.inMcNonRoutedExcpPkts;
    inFields[6] = counters.inMcNonRoutedNonExcpPkts;
    inFields[7] = counters.inUcTrappedMirrorPkts;
    inFields[8] = counters.inMcTrappedMirrorPkts;
    inFields[9] = counters.mcRfpFailPkts;
    inFields[10] = counters.outUcRoutedPkts;

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d%d%d%d", inFields[0],  inFields[1],
                  inFields[2], inFields[3], inFields[4],  inFields[5],
                  inFields[6], inFields[7], inFields[8],  inFields[9],
                  inFields[10]);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%f");

    cntSetCnt++;

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpCntSetModeSet function
* @endinternal
*
* @brief   Sets a counter set's bounded inteface and interface mode.
*
* @note   APPLICABLE DEVICES:      All DxCh2 devices.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static CMD_STATUS wrCpssDxChIpCntSetModeSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                     result;

    GT_U8                                         devNum;
    CPSS_IP_CNT_SET_ENT                           cntSet;
    CPSS_DXCH_IP_CNT_SET_MODE_ENT                 cntSetMode;
    CPSS_DXCH_IP_COUNTER_SET_INTERFACE_CFG_STC    interfaceModeCfgPtr = {0};
    GT_HW_DEV_NUM                                 tmpHwDevNum;
    GT_PORT_NUM                                   tmpPortNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    cntSet = (CPSS_INTERFACE_TYPE_ENT)inFields[0];
    cntSetMode = (CPSS_INTERFACE_TYPE_ENT)inFields[1];

    interfaceModeCfgPtr.portTrunkCntMode =
                             (CPSS_DXCH_IP_PORT_TRUNK_CNT_MODE_ENT)inFields[2];
    interfaceModeCfgPtr.ipMode = (CPSS_IP_PROTOCOL_STACK_ENT)inFields[3];
    interfaceModeCfgPtr.vlanMode = (CPSS_DXCH_IP_VLAN_CNT_MODE_ENT)inFields[4];

    switch(inFields[2])
    {
    case 1:
        tmpHwDevNum = (GT_HW_DEV_NUM)inFields[5];
        tmpPortNum  = (GT_PORT_NUM)inFields[6];
        CONVERT_DEV_PORT_DATA_MAC(tmpHwDevNum,tmpPortNum);
        interfaceModeCfgPtr.hwDevNum = tmpHwDevNum;
        interfaceModeCfgPtr.portTrunk.port = tmpPortNum;
        break;

    case 2:
        interfaceModeCfgPtr.portTrunk.trunk = (GT_TRUNK_ID)inFields[7];
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(interfaceModeCfgPtr.portTrunk.trunk);
        break;

    default:
        break;
    }

    if(inFields[4])
        interfaceModeCfgPtr.vlanId = (GT_U16)inFields[8];

    /* call cpss api function */
    result = cpssDxChIpCntSetModeSet(devNum, cntSet, cntSetMode,
                                                      &interfaceModeCfgPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChIpCntSetModeGet function
* @endinternal
*
* @brief   Gets a counter set's bounded inteface and interface mode.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; xCat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
static CMD_STATUS wrCpssDxChIpCntSetModeGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                     result;

    GT_U8                                         devNum;
      CPSS_DXCH_IP_CNT_SET_MODE_ENT                 cntSetMode =
                                          CPSS_DXCH_IP_CNT_SET_INTERFACE_MODE_E;
    CPSS_DXCH_IP_COUNTER_SET_INTERFACE_CFG_STC    interfaceModeCfgPtr = {0};
    GT_HW_DEV_NUM                                 tmpHwDevNum;
    GT_PORT_NUM                                   tmpPortNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if (cntSetCnt ==  CPSS_IP_CNT_NO_SET_E)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    inFields[0] = cntSetCnt ;


    /* call cpss api function */
    result = cpssDxChIpCntSetModeGet(devNum, cntSetCnt, &cntSetMode,
                                                      &interfaceModeCfgPtr);

    inFields[1] = cntSetMode;
    inFields[2] = interfaceModeCfgPtr.portTrunkCntMode;
    inFields[3] = interfaceModeCfgPtr.ipMode;
    inFields[4] = interfaceModeCfgPtr.vlanMode;

    switch(inFields[2])
    {
    case 1:
        tmpHwDevNum = interfaceModeCfgPtr.hwDevNum;
        tmpPortNum = interfaceModeCfgPtr.portTrunk.port;
        CONVERT_BACK_DEV_PORT_DATA_MAC(tmpHwDevNum,tmpPortNum);
        inFields[5] = tmpHwDevNum;
        inFields[6] = tmpPortNum;
        break;

    case 2:
        CONVERT_TRUNK_ID_CPSS_TO_TEST_MAC(interfaceModeCfgPtr.portTrunk.trunk);
        inFields[7] = interfaceModeCfgPtr.portTrunk.trunk;
        break;

    default:
        break;
    }

    if(inFields[4])
        inFields[8] = interfaceModeCfgPtr.vlanId;


    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d%d", inFields[0], inFields[1], inFields[2],
        inFields[3], inFields[4], inFields[5], inFields[6], inFields[7], inFields[8]);

    cntSetCnt++;

    galtisOutput(outArgs, result, "%f");
    return CMD_OK;
}


/**
* @internal wrCpssDxChIpCntSetModeGetFirst function
* @endinternal
*
* @brief   Gets a counter set's bounded inteface and interface mode.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; xCat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
static CMD_STATUS wrCpssDxChIpCntSetModeGetFirst

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
            cntSetCnt = 0;
            return wrCpssDxChIpCntSetModeGet(inArgs,inFields,numFields,outArgs);
}

/**
* @internal wrCpssDxChIpCntSetModeGetNext function
* @endinternal
*
* @brief   Gets a counter set's bounded inteface and interface mode.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; xCat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
static CMD_STATUS wrCpssDxChIpCntSetModeGetNext

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
           return  wrCpssDxChIpCntSetModeGet(inArgs,inFields,numFields,outArgs);
}

/**
* @internal wrCpssDxChIpCntSet function
* @endinternal
*
* @brief   set the requested IP counter set.
*
* @note   APPLICABLE DEVICES:      All DxCh2 devices.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static CMD_STATUS wrCpssDxChIpCntSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;

    GT_U8                               devNum;
    CPSS_IP_CNT_SET_ENT                 cntSet;
    CPSS_DXCH_IP_COUNTER_SET_STC        counters;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    cntSet = (CPSS_IP_CNT_SET_ENT)inFields[0];

    counters.inUcPkts = (GT_U32)inFields[1];
    counters.inMcPkts = (GT_U32)inFields[2];
    counters.inUcNonRoutedExcpPkts = (GT_U32)inFields[3];
    counters.inUcNonRoutedNonExcpPkts = (GT_U32)inFields[4];
    counters.inMcNonRoutedExcpPkts = (GT_U32)inFields[5];
    counters.inMcNonRoutedNonExcpPkts = (GT_U32)inFields[6];
    counters.inUcTrappedMirrorPkts = (GT_U32)inFields[7];
    counters.inMcTrappedMirrorPkts = (GT_U32)inFields[8];
    counters.mcRfpFailPkts = (GT_U32)inFields[9];
    counters.outUcRoutedPkts = (GT_U32)inFields[10];

    /* call cpss api function */
    result = pg_wrap_cpssDxChIpPortGroupCntSet(devNum, cntSet, &counters);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChIpMllSkippedEntriesCountersGet function
* @endinternal
*
* @brief   Get MLL entries skipped counter.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChIpMllSkippedEntriesCountersGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;

    GT_U8                                       devNum;
    GT_U32                                      skipCounter;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = pg_wrap_cpssDxChIpMllSkippedEntriesCountersGet(devNum, &skipCounter);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", skipCounter);
    return CMD_OK;
}

/**
* @internal wrCpssDxChIpSetMllCntInterface function
* @endinternal
*
* @brief   Sets a mll counter set's bounded inteface.
*
* @note   APPLICABLE DEVICES:      All DxCh2 devices.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static CMD_STATUS wrCpssDxChIpSetMllCntInterface

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                     result;

    GT_U8                                         devNum;
    GT_U32                                        mllCntSet;
    CPSS_DXCH_IP_COUNTER_SET_INTERFACE_CFG_STC    interfaceCfgPtr;

    GT_HW_DEV_NUM                                 tmpHwDevNum;
    GT_PORT_NUM                                   tmpPortNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cpssOsMemSet(&interfaceCfgPtr,0,sizeof(CPSS_DXCH_IP_COUNTER_SET_INTERFACE_CFG_STC));

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    mllCntSet = (GT_U32)inFields[0];

    interfaceCfgPtr.portTrunkCntMode =
                             (CPSS_DXCH_IP_PORT_TRUNK_CNT_MODE_ENT)inFields[1];
    interfaceCfgPtr.ipMode = (CPSS_IP_PROTOCOL_STACK_ENT)inFields[2];
    interfaceCfgPtr.vlanMode = (CPSS_DXCH_IP_VLAN_CNT_MODE_ENT)inFields[3];

    switch(inFields[1])
    {
    case 1:
        /* check if need to update CPSS_DXCH_IP_COUNTER_SET_INTERFACE_CFG_STC to
           support intergace of type: GT_HW_DEV_NUM, GT_PORT_NUM
           and fix the code accordingly to support more devs and ports*/
        CPSS_TBD_BOOKMARK_EARCH

        tmpHwDevNum = (GT_HW_DEV_NUM)inFields[4];
        tmpPortNum  = (GT_PORT_NUM)inFields[5];
        CONVERT_DEV_PORT_DATA_MAC(tmpHwDevNum,tmpPortNum);

        interfaceCfgPtr.hwDevNum = tmpHwDevNum;
        interfaceCfgPtr.portTrunk.port = tmpPortNum;

        break;

    case 2:
        interfaceCfgPtr.portTrunk.trunk = (GT_TRUNK_ID)inFields[6];
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(interfaceCfgPtr.portTrunk.trunk);
        break;

    default:
        break;
    }

    if(inFields[3])
        interfaceCfgPtr.vlanId = (GT_U16)inFields[7];

    /* call cpss api function */
    result = cpssDxChIpSetMllCntInterface(devNum, mllCntSet,
                                                  &interfaceCfgPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChIpMllCntGet function
* @endinternal
*
* @brief   Get the mll counter.
*
* @note   APPLICABLE DEVICES:      All DxCh2 devices.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static CMD_STATUS wrCpssDxChIpMllCntGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;

    GT_U8                                       devNum;
    GT_U32                                      mllCntSet;
    GT_U32                                      mllOutMCPktsPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    mllCntSet = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = pg_wrap_cpssDxChIpMllCntGet(devNum, mllCntSet, &mllOutMCPktsPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mllOutMCPktsPtr);
    return CMD_OK;
}

/**
* @internal wrCpssDxChIpMllCntSet function
* @endinternal
*
* @brief   set an mll counter.
*
* @note   APPLICABLE DEVICES:      All DxCh2 devices.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static CMD_STATUS wrCpssDxChIpMllCntSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;

    GT_U8                                       devNum;
    GT_U32                                      mllCntSet;
    GT_U32                                      mllOutMCPkts;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    mllCntSet = (GT_U32)inArgs[1];
    mllOutMCPkts = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = pg_wrap_cpssDxChIpPortGroupMllCntSet(devNum, mllCntSet,
                                                  mllOutMCPkts);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChIpMllSilentDropCntGet function
* @endinternal
*
* @brief   Get the silent drops in the MLL priority queues.
*         A silent drop is a drop that is applied to a replica of the packet that
*         was previously replicated in the TTI.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChIpMllSilentDropCntGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;

    GT_U8                                       devNum;
    GT_U32                                      silentDropPkts;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = pg_wrap_cpssDxChIpMllSilentDropCntGet(devNum, &silentDropPkts);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", silentDropPkts);
    return CMD_OK;
}

/**
* @internal wrCpssDxChIpDropCntSet function
* @endinternal
*
* @brief   set the drop counter.
*
* @note   APPLICABLE DEVICES:      All DxCh2 devices.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static CMD_STATUS wrCpssDxChIpDropCntSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;

    GT_U8                                       devNum;
    GT_U32                                      dropPkts;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    dropPkts = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = pg_wrap_cpssDxChIpDropCntSet(devNum, dropPkts);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChIpRouterBridgedPacketsExceptionCntGet function
* @endinternal
*
* @brief   Get exception counter for Special Services for Bridged Traffic.
*         it counts the number of Bridged packets failing any of the following checks:
*         - SIP Filter check for bridged IPv4/6 packets
*         - IP Header Check for bridged IPv4/6 packets
*         - Unicast RPF check for bridged IPv4/6 and ARP packets
*         - Unicast SIP/SA check for bridged IPv4/6 and ARP packets
*
* @note   APPLICABLE DEVICES:      DxCh2 and above.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on illegal pointer value
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChIpRouterBridgedPacketsExceptionCntGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;

    GT_U8                                       devNum;
    GT_U32                                      routerBridgedExceptionPkts;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = pg_wrap_cpssDxChIpRouterBridgedPacketsExceptionCntGet(devNum,
                                                                   &routerBridgedExceptionPkts);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", routerBridgedExceptionPkts);
    return CMD_OK;
}


/**
* @internal wrCpssDxChIpRouterBridgedPacketsExceptionCntSet function
* @endinternal
*
* @brief   Set exception counter for Special Services for Bridged Traffic.
*         it counts the number of Bridged packets failing any of the following checks:
*         - SIP Filter check for bridged IPv4/6 packets
*         - IP Header Check for bridged IPv4/6 packets
*         - Unicast RPF check for bridged IPv4/6 and ARP packets
*         - Unicast SIP/SA check for bridged IPv4/6 and ARP packets
*
* @note   APPLICABLE DEVICES:      DxCh2 and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChIpRouterBridgedPacketsExceptionCntSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;

    GT_U8                                       devNum;
    GT_U32                                      routerBridgedExceptionPkts;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    routerBridgedExceptionPkts = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = pg_wrap_cpssDxChIpRouterBridgedPacketsExceptionCntSet(devNum,
                                                                   routerBridgedExceptionPkts);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChIpSetDropCntMode function
* @endinternal
*
* @brief   Sets the drop counter count mode.
*
* @note   APPLICABLE DEVICES:      All DxCh2 devices.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static CMD_STATUS wrCpssDxChIpSetDropCntMode

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;

    GT_U8                                       devNum;
    CPSS_DXCH_IP_DROP_CNT_MODE_ENT              dropCntMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    dropCntMode = (CPSS_DXCH_IP_DROP_CNT_MODE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChIpSetDropCntMode(devNum, dropCntMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChIpDropCntGet function
* @endinternal
*
* @brief   Get the drop counter.
*
* @note   APPLICABLE DEVICES:      All DxCh2 devices.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static CMD_STATUS wrCpssDxChIpDropCntGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;

    GT_U8                                       devNum;
    GT_U32                                      dropPktsPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = pg_wrap_cpssDxChIpDropCntGet(devNum, &dropPktsPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", dropPktsPtr);
    return CMD_OK;
}

/**
* @internal wrCpssDxChIpMtuProfileSet function
* @endinternal
*
* @brief   Sets the next hop interface MTU profile limit value.
*
* @note   APPLICABLE DEVICES:      All DxCh2 devices.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/

static CMD_STATUS wrCpssDxChIpMtuProfileSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;

    GT_U8                                       devNum;
    GT_U32                                      mtuProfileIndex;
    GT_U32                                      mtu;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    mtuProfileIndex = (GT_U32)inArgs[1];
    mtu = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChIpMtuProfileSet(devNum, mtuProfileIndex, mtu);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}
/**
* @internal wrCpssDxChIpMtuProfileGet function
* @endinternal
*
* @brief   Gets the next hop interface MTU profile limit value.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; xCat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
static CMD_STATUS wrCpssDxChIpMtuProfileGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;

    GT_U8                                       devNum;
    GT_U32                                      mtuProfileIndex;
    GT_U32                                      mtuPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    mtuProfileIndex = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChIpMtuProfileGet(devNum, mtuProfileIndex, &mtuPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mtuPtr);
    return CMD_OK;
}

/**
* @internal wrCpssDxChIpv6AddrPrefixScopeSet function
* @endinternal
*
* @brief   Defines a prefix of a scope type.
*
* @note   APPLICABLE DEVICES:      All DxCh2 devices.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - case that the prefix length is out of range.
* @retval GT_NOT_SUPPORTED         - if working with a PP that doesn't support IPv6
*
* @note Configures an entry in the prefix look up table
*
*/
static CMD_STATUS wrCpssDxChIpv6AddrPrefixScopeSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;

    GT_U8                                       devNum;
    GT_IPV6ADDR                                 prefix;
    GT_U32                                      prefixLen;
    CPSS_IPV6_PREFIX_SCOPE_ENT                  addressScope;
    GT_U32                                      prefixScopeIndex;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    galtisIpv6Addr(&prefix, (GT_U8*)inArgs[1]);
    prefixLen = (GT_U32)inArgs[2];
    addressScope = (CPSS_IPV6_PREFIX_SCOPE_ENT)inArgs[3];
    prefixScopeIndex = (GT_U32)inArgs[4];

    /* call cpss api function */
    result = cpssDxChIpv6AddrPrefixScopeSet(devNum, &prefix, prefixLen,
                                       addressScope, prefixScopeIndex);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/*******************************************************************************
* cpssDxChIpv6AddrPrefixScopeGet
*
* DESCRIPTION:
*       Get a prefix of a scope type.
*
* APPLICABLE DEVICES:
*        DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; xCat2.
*
* INPUTS:
*       devNum           - the device number
*       prefixScopeIndex - index of the new prefix scope entry (APPLICABLE RANGES: 0..4)
*
* OUTPUTS:
*       prefixPtr        - an IPv6 address prefix
*       prefixLenPtr     - length of the prefix
*       addressScopePtr  - type of the address scope spanned by the prefix
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_FAIL                  - on error.
*       GT_BAD_PARAM             - case that the prefix length is out of range.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static  GT_U32 index;

static CMD_STATUS ipv6AddrPrefixScopeGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_IPV6ADDR prefix;
    GT_U32 prefixLen;
    CPSS_IPV6_PREFIX_SCOPE_ENT addressScope;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChIpv6AddrPrefixScopeGet(devNum, &prefix, &prefixLen, &addressScope, index);


    if(result != CMD_OK)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    inFields[1] = prefixLen;
    inFields[2] = addressScope;
    inFields[3] = index;

    /* pack output arguments to galtis string */
    fieldOutput("%d%16B%d%d", prefix.arIP, inFields[1],
                                inFields[2],  inFields[3]);

    galtisOutput(outArgs, result, "%f");
    return CMD_OK;
}

/******************************************************************************/
static CMD_STATUS wrCpssDxChIpv6AddrPrefixScopeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_U8                       devNum;
    GT_U32                      index;
    GT_IPV6ADDR                 prefix;
    GT_U32                      prefixLen;
    CPSS_IPV6_PREFIX_SCOPE_ENT  addressScope;

    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    index = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChIpv6AddrPrefixScopeGet(devNum, &prefix, &prefixLen,
                                            &addressScope, index);

    galtisOutput(outArgs, result, "%16B%d%d%d",
                 prefix.arIP, prefixLen, addressScope, index);

    return CMD_OK;
}

/******************************************************************************/
static CMD_STATUS wrCpssDxChIpv6AddrPrefixScopeGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    CMD_STATUS res;

    index = 0;
    res = ipv6AddrPrefixScopeGet(inArgs,inFields,numFields,outArgs);
    if (res == CMD_OK)
        index++;
    return res;
}
/******************************************************************************/
static CMD_STATUS wrCpssDxChIpv6AddrPrefixScopeGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    CMD_STATUS res;

    if(index > 3)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }
    res = ipv6AddrPrefixScopeGet(inArgs,inFields,numFields,outArgs);
    if (res == CMD_OK)
        index++;
    return res;
}

/**
* @internal wrCpssDxChIpv6UcScopeCommandSet function
* @endinternal
*
* @brief   sets the ipv6 Unicast scope commands.
*
* @note   APPLICABLE DEVICES:      All DxCh2 devices.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_NO_RESOURCE           - if failed to allocate CPU memory
* @retval GT_NOT_SUPPORTED         - if working with a PP that doesn't support IPv6
*/
static CMD_STATUS wrCpssDxChIpv6UcScopeCommandSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;

    GT_U8                                       devNum;
    CPSS_IPV6_PREFIX_SCOPE_ENT                  addressScopeSrc;
    CPSS_IPV6_PREFIX_SCOPE_ENT                  addressScopeDest;
    GT_BOOL                                     borderCrossed;
    CPSS_PACKET_CMD_ENT                         scopeCommand;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    addressScopeSrc = (CPSS_IPV6_PREFIX_SCOPE_ENT)inArgs[1];
    addressScopeDest = (CPSS_IPV6_PREFIX_SCOPE_ENT)inArgs[2];
    borderCrossed = (GT_BOOL)inArgs[3];
    scopeCommand = (CPSS_PACKET_CMD_ENT)inArgs[4];

    /* call cpss api function */
    result = cpssDxChIpv6UcScopeCommandSet(devNum, addressScopeSrc,
                     addressScopeDest, borderCrossed, scopeCommand);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChIpv6UcScopeCommandGet function
* @endinternal
*
* @brief   gets the ipv6 Unicast scope commands.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; xCat2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_NO_RESOURCE           - if failed to allocate CPU memory
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
static CMD_STATUS wrCpssDxChIpv6UcScopeCommandGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;

    GT_U8                                       devNum;
    CPSS_IPV6_PREFIX_SCOPE_ENT                  addressScopeSrc;
    CPSS_IPV6_PREFIX_SCOPE_ENT                  addressScopeDest;
    GT_BOOL                                     borderCrossed;
    CPSS_PACKET_CMD_ENT                         scopeCommand;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    addressScopeSrc = (CPSS_IPV6_PREFIX_SCOPE_ENT)inArgs[1];
    addressScopeDest = (CPSS_IPV6_PREFIX_SCOPE_ENT)inArgs[2];
    borderCrossed = (GT_BOOL)inArgs[3];

    /* call cpss api function */
    result = cpssDxChIpv6UcScopeCommandGet(devNum, addressScopeSrc,
                     addressScopeDest, borderCrossed, &scopeCommand);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", scopeCommand);
    return CMD_OK;
}

/**
* @internal wrCpssDxChIpv6McScopeCommandSet function
* @endinternal
*
* @brief   sets the ipv6 Multicast scope commands.
*
* @note   APPLICABLE DEVICES:      All DxCh2 devices.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_NO_RESOURCE           - if failed to allocate CPU memory
* @retval GT_NOT_SUPPORTED         - if working with a PP that doesn't support IPv6
*/
static CMD_STATUS wrCpssDxChIpv6McScopeCommandSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;

    GT_U8                                       devNum;
    CPSS_IPV6_PREFIX_SCOPE_ENT                  addressScopeSrc;
    CPSS_IPV6_PREFIX_SCOPE_ENT                  addressScopeDest;
    GT_BOOL                                     borderCrossed;
    CPSS_PACKET_CMD_ENT                         scopeCommand;
    CPSS_IPV6_MLL_SELECTION_RULE_ENT            mllSelectionRule;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    addressScopeSrc = (CPSS_IPV6_PREFIX_SCOPE_ENT)inArgs[1];
    addressScopeDest = (CPSS_IPV6_PREFIX_SCOPE_ENT)inArgs[2];
    borderCrossed = (GT_BOOL)inArgs[3];
    scopeCommand = (CPSS_PACKET_CMD_ENT)inArgs[4];
    mllSelectionRule = (CPSS_IPV6_MLL_SELECTION_RULE_ENT)inArgs[5];

    /* call cpss api function */
    result = cpssDxChIpv6McScopeCommandSet(devNum, addressScopeSrc,
                                   addressScopeDest, borderCrossed,
                                    scopeCommand, mllSelectionRule);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChIpv6McScopeCommandGet function
* @endinternal
*
* @brief   gets the ipv6 Multicast scope commands.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; xCat2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_NO_RESOURCE           - if failed to allocate CPU memory
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
static CMD_STATUS wrCpssDxChIpv6McScopeCommandGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;

    GT_U8                                       devNum;
    CPSS_IPV6_PREFIX_SCOPE_ENT                  addressScopeSrc;
    CPSS_IPV6_PREFIX_SCOPE_ENT                  addressScopeDest;
    GT_BOOL                                     borderCrossed;
    CPSS_PACKET_CMD_ENT                         scopeCommand;
    CPSS_IPV6_MLL_SELECTION_RULE_ENT            mllSelectionRule;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    addressScopeSrc = (CPSS_IPV6_PREFIX_SCOPE_ENT)inArgs[1];
    addressScopeDest = (CPSS_IPV6_PREFIX_SCOPE_ENT)inArgs[2];
    borderCrossed = (GT_BOOL)inArgs[3];

    /* call cpss api function */
    result = cpssDxChIpv6McScopeCommandGet(devNum, addressScopeSrc,
                                   addressScopeDest, borderCrossed,
                                    &scopeCommand, &mllSelectionRule);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d, %d",scopeCommand, mllSelectionRule);
    return CMD_OK;
}

/**
* @internal wrCpssDxChIpRouterMacSaBaseSet function
* @endinternal
*
* @brief   Sets 8 MSBs of Router MAC SA Base address on specified device.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChIpRouterMacSaBaseSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8         devNum;
    GT_ETHERADDR  macPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    galtisMacAddr(&macPtr, (GT_U8*)inArgs[1]);

    /* call cpss api function */
    result = cpssDxChIpRouterMacSaBaseSet(devNum, &macPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChIpRouterMacSaBaseGet function
* @endinternal
*
* @brief   Get 8 MSBs of Router MAC SA Base address on specified device.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChIpRouterMacSaBaseGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8         devNum;
    GT_ETHERADDR  macPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChIpRouterMacSaBaseGet(devNum, &macPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%6b", macPtr.arEther);
    return CMD_OK;
}

/**
* @internal wrCpssDxChIpRouterMacSaModifyEnable function
* @endinternal
*
* @brief   Per Egress port bit Enable Routed packets MAC SA Modification
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_NOT_SUPPORTED         - if the feature does not supported by specified device
* @retval GT_BAD_PARAM             - wrong devNum/portNum.
*/
static CMD_STATUS wrCpssDxChIpRouterMacSaModifyEnable
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8      devNum;
    GT_PHYSICAL_PORT_NUM      portNum;
    GT_BOOL    enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChIpRouterMacSaModifyEnable(devNum, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChIpRouterMacSaModifyEnableGet function
* @endinternal
*
* @brief   Per Egress port bit Get Routed packets MAC SA Modification State
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_NOT_SUPPORTED         - if the feature does not supported by specified device
* @retval GT_BAD_PARAM             - wrong devNum/portNum.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChIpRouterMacSaModifyEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_U8      devNum;
    GT_PHYSICAL_PORT_NUM      portNum;
    GT_BOOL    enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

   /* call cpss api function */
    result = cpssDxChIpRouterMacSaModifyEnableGet(devNum, portNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}
/**
* @internal wrCpssDxChIpPortRouterMacSaLsbModeSet function
* @endinternal
*
* @brief   Sets the mode, per port, in which the device sets the packet's MAC SA
*         least significant bytes.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_SUPPORTED         - if the feature does not supported by specified device
* @retval GT_BAD_PARAM             - wrong devNum/saLsbMode.
*
* @note The device 5 most significant bytes are set by cpssDxChIpRouterMacSaBaseSet().
*       If saLsbMode is Port Mode, the Port library action
*       cpssDxChPortMacSaLsbSet() sets the per port least significant byte.
*       CPSS_MAC_SA_LSB_MODE_ENT -
*       If saLsbMode is Vlan Mode, the Vlan library action
*       cpssDxChBrgVlanMacPerVlanSet() sets the per vlan least significant byte.
*
*/
static CMD_STATUS wrCpssDxChIpPortRouterMacSaLsbModeSet
(
   IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
   IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
   IN  GT_32 numFields,
   OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    CPSS_MAC_SA_LSB_MODE_ENT saLsbMode;


    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    saLsbMode = (CPSS_MAC_SA_LSB_MODE_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChIpPortRouterMacSaLsbModeSet(devNum, portNum, saLsbMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}
/**
* @internal wrCpssDxChIpPortRouterMacSaLsbModeGet function
* @endinternal
*
* @brief   Gets the mode, per port, in which the device sets the packet's MAC SA least
*         significant bytes.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_SUPPORTED         - if the feature does not supported by specified device
* @retval GT_BAD_PARAM             - wrong devNum/saLsbMode.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChIpPortRouterMacSaLsbModeGet
(
   IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
   IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
   IN  GT_32 numFields,
   OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    CPSS_MAC_SA_LSB_MODE_ENT saLsbMode;


    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChIpPortRouterMacSaLsbModeGet(devNum, portNum, &saLsbMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", saLsbMode);

    return CMD_OK;
}





/**
* @internal wrCpssDxChIpRouterGlobalMacSaSet function
* @endinternal
*
* @brief   Sets full 48-bit Router MAC SA in Global MAC SA table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or routerMacSaIndex.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note This API is applicable when per-egress-physical-port MAC SA assignment
*       mode is configured to global :CPSS_SA_LSB_FULL_48_BIT_GLOBAL by API
*       cpssDxChIpPortRouterMacSaLsbModeSet. The routerMacSaIndex is configured
*       by cpssDxChIpRouterGlobalMacSaIndexSet.
*
*/
static CMD_STATUS wrCpssDxChIpRouterGlobalMacSaSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    GT_STATUS   status;

    GT_U8               dev;
    GT_U32              routerMacSaIndex;
    GT_ETHERADDR        macSaAddr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    dev = (GT_U8)inArgs[0];
    routerMacSaIndex = (GT_U32)inArgs[1];
    galtisMacAddr(&macSaAddr, (GT_U8*)inArgs[2]);

    /* call cpss api function */
    status =  cpssDxChIpRouterGlobalMacSaSet(dev,routerMacSaIndex,&macSaAddr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}




/**
* @internal wrCpssDxChIpRouterGlobalMacSaGet function
* @endinternal
*
* @brief   Gets full 48-bit Router MAC SA from Global MAC SA table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or routerMacSaIndex.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note This API is applicable when per-egress-physical-port MAC SA assignment
*       mode is configured to global :CPSS_SA_LSB_FULL_48_BIT_GLOBAL by API
*       cpssDxChIpPortRouterMacSaLsbModeSet. The routerMacSaIndex is configured
*       by cpssDxChIpRouterGlobalMacSaIndexSet.
*
*/
static CMD_STATUS wrCpssDxChIpRouterGlobalMacSaGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    GT_STATUS   status;

    GT_U8               dev;
    GT_U32              routerMacSaIndex;
    GT_ETHERADDR        macSaAddr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    dev = (GT_U8)inArgs[0];
    routerMacSaIndex = (GT_U32)inArgs[1];

    /* call cpss api function */
    status =  cpssDxChIpRouterGlobalMacSaGet(dev,routerMacSaIndex,&macSaAddr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%6b", macSaAddr.arEther);

    return CMD_OK;
}


/**
* @internal wrCpssDxChIpRouterGlobalMacSaIndexSet function
* @endinternal
*
* @brief   Set router mac sa index refered to global MAC SA table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static CMD_STATUS wrCpssDxChIpRouterGlobalMacSaIndexSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                          result;
    GT_U8                              devNum;
    GT_PORT_NUM                        portNum;
    GT_U32                             routerMacSaIndex;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];
    routerMacSaIndex = (GT_U32)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChIpRouterPortGlobalMacSaIndexSet(devNum, portNum, routerMacSaIndex);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpRouterGlobalMacSaIndexGet function
* @endinternal
*
* @brief   Get router mac sa index refered to global MAC SA table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static CMD_STATUS wrCpssDxChIpRouterGlobalMacSaIndexGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                          result;
    GT_U8                              devNum;
    GT_PORT_NUM                        portNum;
    GT_U32                             routerMacSaIndex;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChIpRouterPortGlobalMacSaIndexGet(devNum, portNum, &routerMacSaIndex);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", routerMacSaIndex);
    return CMD_OK;
}

/**
* @internal wrCpssDxChIpRouterPortMacSaLsbSet function
* @endinternal
*
* @brief   Sets the 8 LSB Router MAC SA for this EGGRESS PORT.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_SUPPORTED         - if the feature does not supported by specified device
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
*/
static CMD_STATUS wrCpssDxChIpRouterPortMacSaLsbSet
(
   IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
   IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
   IN  GT_32 numFields,
   OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_U8 saMac;


    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    saMac = (GT_U8)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChIpRouterPortMacSaLsbSet(devNum, portNum, saMac);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpRouterPortMacSaLsbGet function
* @endinternal
*
* @brief   Gets the 8 LSB Router MAC SA for this EGGRESS PORT.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_SUPPORTED         - if the feature does not supported by specified device
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChIpRouterPortMacSaLsbGet
(
   IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
   IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
   IN  GT_32 numFields,
   OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_U8 saMac;


    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChIpRouterPortMacSaLsbGet(devNum, portNum, &saMac);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", saMac);

    return CMD_OK;
}
/**
* @internal wrCpssDxChIpRouterVlanMacSaLsbSet function
* @endinternal
*
* @brief   Sets the 8 LSB Router MAC SA for this VLAN.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_SUPPORTED         - if the feature does not supported by specified device
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vlanId
*/
static CMD_STATUS wrCpssDxChIpRouterVlanMacSaLsbSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U16 vlan;
    GT_U32 saMac;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    vlan = (GT_U16)inArgs[1];
    saMac = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChIpRouterVlanMacSaLsbSet(devNum, vlan, saMac);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpRouterVlanMacSaLsbGet function
* @endinternal
*
* @brief   Gets the 8 LSB Router MAC SA for this VLAN.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_SUPPORTED         - if the feature does not supported by specified device
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vlanId
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChIpRouterVlanMacSaLsbGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U16 vlan;
    GT_U32 saMac;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    vlan = (GT_U16)inArgs[1];

    /* call cpss api function */
    result = cpssDxChIpRouterVlanMacSaLsbGet(devNum, vlan, &saMac);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", saMac);

    return CMD_OK;
}
/**
* @internal wrCpssDxChIpEcmpUcRpfCheckEnableSet function
* @endinternal
*
* @brief   Globally enables/disables ECMP/QoS unicast RPF check.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_FAIL                  - on error
*/
static CMD_STATUS wrCpssDxChIpEcmpUcRpfCheckEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChIpEcmpUcRpfCheckEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpEcmpUcRpfCheckEnableGet function
* @endinternal
*
* @brief   Gets globally enables/disables ECMP/QoS unicast RPF check state.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_FAIL                  - on error
*/
static CMD_STATUS wrCpssDxChIpEcmpUcRpfCheckEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChIpEcmpUcRpfCheckEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d",enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpUcRpfModeSet function
* @endinternal
*
* @brief   Defines the uRPF check mode for a given VID.
*
* @note   APPLICABLE DEVICES:      Lion and above
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device, vid or uRpfMode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note If ucRPFCheckEnable field configured in cpssDxChIpLttWrite is enabled
*       then VLAN-based uRPF check is performed, regardless of this configuration.
*       Otherwise, uRPF check is performed for this VID according to this
*       configuration.
*       Port-based uRPF mode is not supported if the address is associated
*       with an ECMP/QoS block of nexthop entries.
*
*/
static CMD_STATUS wrCpssDxChIpUcRpfModeSet
(
 IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
 IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
 IN  GT_32 numFields,
 OUT GT_8  outArgs[CMD_MAX_BUFFER]
 )
{

    GT_STATUS   status;

    GT_U8                           devNum;
    GT_U16                          vid;
    CPSS_DXCH_IP_URPF_MODE_ENT      uRpfMode;

   /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum      = (GT_U8)inArgs[0];
    vid         = (GT_U16)inArgs[1];
    uRpfMode    = (CPSS_DXCH_IP_URPF_MODE_ENT)inArgs[2];

    /* call cpss api function */
    status =  cpssDxChIpUcRpfModeSet(devNum,vid, uRpfMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}


/**
* @internal wrCpssDxChIpUcRpfModeGet function
* @endinternal
*
* @brief   Read uRPF check mode for a given VID.
*
* @note   APPLICABLE DEVICES:      Lion and above
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note If ucRPFCheckEnable field configured in cpssDxChIpLttWrite is enabled
*       then VLAN-based uRPF check is performed, regardless of this configuration.
*       Otherwise, uRPF check is performed for this VID according to this
*       configuration.
*       Port-based uRPF mode is not supported if the address is associated
*       with an ECMP/QoS block of nexthop entries.
*
*/
static CMD_STATUS wrCpssDxChIpUcRpfModeGet
(
 IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
 IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
 IN  GT_32 numFields,
 OUT GT_8  outArgs[CMD_MAX_BUFFER]
 )
{

    GT_STATUS   status;

    GT_U8                           devNum;
    GT_U16                          vid;
    CPSS_DXCH_IP_URPF_MODE_ENT      uRpfMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum      = (GT_U8)inArgs[0];
    vid         = (GT_U16)inArgs[1];

    /* call cpss api function */
    status =  cpssDxChIpUcRpfModeGet(devNum,vid, &uRpfMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d", uRpfMode);

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpUrpfLooseModeTypeSet function
* @endinternal
*
* @brief   This function set type of Urpf loose mode
*
* @note   APPLICABLE DEVICES:      Aldrin2; Falcon
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; xCat3; xCat3x ; Lion2; Bobcat3; Aldrin2; Falcon
*
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_FAIL                  - otherwise.
*
* @note none.
*
*/
static CMD_STATUS wrCpssDxChIpUrpfLooseModeTypeSet
(
 IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
 IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
 IN  GT_32 numFields,
 OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    GT_STATUS   status;

    GT_U8                               devNum;
    CPSS_DXCH_URPF_LOOSE_MODE_TYPE_ENT  looseModeType;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum          = (GT_U8)inArgs[0];
    looseModeType   = (CPSS_DXCH_URPF_LOOSE_MODE_TYPE_ENT)inArgs[1];

    /* call cpss api function */
    status =  cpssDxChIpUrpfLooseModeTypeSet(devNum, looseModeType);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpUrpfLooseModeTypeGet function
* @endinternal
*
* @brief   This function get type of Urpf loose mode
*
* @note   APPLICABLE DEVICES:      Aldrin2; Falcon
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; xCat3; xCat3x ; Lion2; Bobcat3; Aldrin2; Falcon
*
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_FAIL                  - otherwise.
*
* @note none.
*
*/
static CMD_STATUS wrCpssDxChIpUrpfLooseModeTypeGet
(
 IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
 IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
 IN  GT_32 numFields,
 OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    GT_STATUS   status;

    GT_U8                               devNum;
    CPSS_DXCH_URPF_LOOSE_MODE_TYPE_ENT  looseModeType;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    status =  cpssDxChIpUrpfLooseModeTypeGet(devNum, &looseModeType);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d", looseModeType);

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpPortSipSaEnableSet function
* @endinternal
*
* @brief   Enable SIP/SA check for packets received from the given port.
*
* @note   APPLICABLE DEVICES:      Lion and above
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note SIP/SA check is triggered only if both this flag and
*       the sipSaCheckMismatchEnable field configured in cpssDxChIpLttWrite
*       are enabled.
*
*/
static CMD_STATUS wrCpssDxChIpPortSipSaEnableSet
(
 IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
 IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
 IN  GT_32 numFields,
 OUT GT_8  outArgs[CMD_MAX_BUFFER]
 )
{

    GT_STATUS   status;

    GT_U8                        devNum;
    GT_PORT_NUM                  portNum;
    GT_BOOL                      enable;

   /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum      = (GT_U8)inArgs[0];
    portNum     = (GT_PORT_NUM)inArgs[1];
    enable      = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    status =  cpssDxChIpPortSipSaEnableSet(devNum,portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpPortSipSaEnableGet function
* @endinternal
*
* @brief   Return the SIP/SA check status for packets received from the given port.
*
* @note   APPLICABLE DEVICES:      Lion and above
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note SIP/SA check is triggered only if both this flag and
*       the sipSaCheckMismatchEnable field configured in cpssDxChIpLttWrite
*       are enabled.
*
*/
static CMD_STATUS wrCpssDxChIpPortSipSaEnableGet
(
 IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
 IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
 IN  GT_32 numFields,
 OUT GT_8  outArgs[CMD_MAX_BUFFER]
 )
{

    GT_STATUS   status;

    GT_U8                        devNum;
    GT_PORT_NUM                        portNum;
    GT_BOOL                      enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum      = (GT_U8)inArgs[0];
    portNum     = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    status =  cpssDxChIpPortSipSaEnableGet(devNum,portNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpRouterSourceIdOverrideEnableSet function
* @endinternal
*
* @brief   Enable/Disable overriding of source id by routing engine.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Lion2 device: applicable starting from revision B1
*
*/
static CMD_STATUS wrCpssDxChIpRouterSourceIdOverrideEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChIpRouterSourceIdOverrideEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpRouterSourceIdOverrideEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable status of overriding of source id
*         by routing engine.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Lion2 device: applicable starting from revision B1
*
*/
static CMD_STATUS wrCpssDxChIpRouterSourceIdOverrideEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    enable = GT_FALSE;

    /* call cpss api function */
    result = cpssDxChIpRouterSourceIdOverrideEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpHeaderErrorMaskSet function
* @endinternal
*
* @brief   Mask or unmask an IP header error.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note If the error is masked, then an IPv4/6 UC/MC header exception will not
*       be triggered for the relevant header error.
*
*/
static CMD_STATUS wrCpssDxChIpHeaderErrorMaskSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH_IP_HEADER_ERROR_ENT        ipHeaderErrorType;
    CPSS_IP_PROTOCOL_STACK_ENT           protocolStack;
    CPSS_UNICAST_MULTICAST_ENT           prefixType;
    GT_BOOL                              mask;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    ipHeaderErrorType = (CPSS_DXCH_IP_HEADER_ERROR_ENT)inArgs[1];
    protocolStack = (CPSS_IP_PROTOCOL_STACK_ENT)inArgs[2];
    prefixType = (CPSS_UNICAST_MULTICAST_ENT)inArgs[3];
    mask = (GT_BOOL)inArgs[4];

    /* call cpss api function */
    result = cpssDxChIpHeaderErrorMaskSet(devNum, ipHeaderErrorType, protocolStack, prefixType, mask);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpHeaderErrorMaskGet function
* @endinternal
*
* @brief   Get the masking status of an IP header error.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note If the error is masked, then an IPv4/6 UC/MC header exception will not
*       be triggered for the relevant header error.
*
*/
static CMD_STATUS wrCpssDxChIpHeaderErrorMaskGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH_IP_HEADER_ERROR_ENT        ipHeaderErrorType;
    CPSS_IP_PROTOCOL_STACK_ENT           protocolStack;
    CPSS_UNICAST_MULTICAST_ENT           prefixType;
    GT_BOOL                              mask;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    ipHeaderErrorType = (CPSS_DXCH_IP_HEADER_ERROR_ENT)inArgs[1];
    protocolStack = (CPSS_IP_PROTOCOL_STACK_ENT)inArgs[2];
    prefixType = (CPSS_UNICAST_MULTICAST_ENT)inArgs[3];

    /* call cpss api function */
    result = cpssDxChIpHeaderErrorMaskGet(devNum, ipHeaderErrorType, protocolStack, prefixType, &mask);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mask);

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpUcRoutingVid1AssignEnableSet function
* @endinternal
*
* @brief   Enable/disable VID1 assignment by the unicast routing
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; xCat2; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
*
* @note When VID1 assignment is enabled QoS attributes can't be set by the
*       router and the following fields in the unicast nexthop entries are not
*       applicable:
*       qosProfileMarkingEnable, qosProfileIndex, qosPrecedence, modifyUp,
*       modifyDscp.
*       When VID1 assignment is disabled QoS attributes are applicable and
*       nextHopVlanId1 field in the unicast nexthop entries is not applicable.
*       It's recommended to use this API before configuring the nexthop entries.
*       Using this API when nexthops are already configured can cause unexpected
*       results.
*
*/
static CMD_STATUS wrCpssDxChIpUcRoutingVid1AssignEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChIpUcRoutingVid1AssignEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpUcRoutingVid1AssignEnableGet function
* @endinternal
*
* @brief   Get the enabling status of VID1 assignment by the unicast routing
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; xCat2; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
*
* @note When VID1 assignment is enabled QoS attributes can't be set by the
*       router and the following fields in the unicast nexthop entries are not
*       applicable:
*       qosProfileMarkingEnable, qosProfileIndex, qosPrecedence, modifyUp,
*       modifyDscp.
*       When VID1 assignment is disabled QoS attributes are applicable and
*       nextHopVlanId1 field in the unicast nexthop entries is not applicable.
*
*/
static CMD_STATUS wrCpssDxChIpUcRoutingVid1AssignEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChIpUcRoutingVid1AssignEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}


/**
* @internal wrCpssDxChIpMllMultiTargetShaperBaselineSet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Set Token Bucket Baseline.
*         The Token Bucket Baseline is the "zero" level of the token bucket.
*         When the token bucket fill level < Baseline,
*         the respective packet is not served.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range baseline
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Token Bucket Baseline must be configured as follows:
*       1. At least MTU (the maximum expected packet size in the system).
*       2. When packet based shaping is enabled, the following used as
*       shaper's MTU:
*       cpssDxChIpMllMultiTargetShaperMtuSet.
*
*/
static CMD_STATUS wrCpssDxChIpMllMultiTargetShaperBaselineSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32 baseline;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    baseline = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChIpMllMultiTargetShaperBaselineSet(devNum, baseline);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpMllMultiTargetShaperBaselineGet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Get Token Bucket Baseline.
*         The Token Bucket Baseline is the "zero" level of the token bucket.
*         When the token bucket fill level < Baseline, the respective packet
*         is not served.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChIpMllMultiTargetShaperBaselineGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32 baseline;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChIpMllMultiTargetShaperBaselineGet(devNum, &baseline);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", baseline);

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpMllMultiTargetShaperMtuSet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Set the packet length in bytes for updating the shaper token bucket.
*         Valid when <Token Bucket Mode>=Packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, mtu
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note <Token Bucket Mode> is configured in cpssDxChIpMllMultiTargetShaperConfigurationSet
*
*/
static CMD_STATUS wrCpssDxChIpMllMultiTargetShaperMtuSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32 mtu;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    mtu = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChIpMllMultiTargetShaperMtuSet(devNum, mtu);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**
* @internal wrCpssDxChIpMllMultiTargetShaperMtuGet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Get the packet length in bytes for updating the shaper token bucket.
*         Valid when <Token Bucket Mode>=Packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note <Token Bucket Mode> is configured in cpssDxChIpMllMultiTargetShaperConfigurationSet
*
*/
static CMD_STATUS wrCpssDxChIpMllMultiTargetShaperMtuGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32 baseline;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChIpMllMultiTargetShaperMtuGet(devNum, &baseline);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", baseline);

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpMllMultiTargetShaperTokenBucketModeSet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Set Token Bucket Mode Mll shaper.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChIpMllMultiTargetShaperTokenBucketModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_PORT_TX_DROP_SHAPER_MODE_ENT            tokenBucketMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    tokenBucketMode = (CPSS_PORT_TX_DROP_SHAPER_MODE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChIpMllMultiTargetShaperTokenBucketModeSet(devNum, tokenBucketMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**
* @internal wrCpssDxChIpMllMultiTargetShaperTokenBucketModeGet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Get Token Bucket Mode Mll shaper.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
static CMD_STATUS wrCpssDxChIpMllMultiTargetShaperTokenBucketModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_PORT_TX_DROP_SHAPER_MODE_ENT   tokenBucketMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChIpMllMultiTargetShaperTokenBucketModeGet(devNum, &tokenBucketMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", tokenBucketMode);

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpMllMultiTargetShaperEnableSet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Enable/Disable Token Bucket rate shaping.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChIpMllMultiTargetShaperEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChIpMllMultiTargetShaperEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**
* @internal wrCpssDxChIpMllMultiTargetShaperEnableGet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Get Enable/Disable Token Bucket rate shaping status
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChIpMllMultiTargetShaperEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChIpMllMultiTargetShaperEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpMllMultiTargetShaperConfigurationSet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Set configuration for Mll shaper.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note When packet based shaping is enabled, the following used as shaper's MTU:
*       see:
*       cpssDxChIpMllMultiTargetShaperMtuSet
*       cpssDxChIpMllMultiTargetShaperBaselineSet.
*
*/
static CMD_STATUS wrCpssDxChIpMllMultiTargetShaperConfigurationSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                            result;
    GT_U8                                devNum;
    GT_U32                               maxBucketSize;
    GT_U32                               maxRate;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    maxBucketSize   = (GT_U32)inArgs[1];
    maxRate = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChIpMllMultiTargetShaperConfigurationSet(devNum,
                                                            maxBucketSize,
                                                            &maxRate);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpMllMultiTargetShaperConfigurationGet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Get configuration for shaper.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
static CMD_STATUS wrCpssDxChIpMllMultiTargetShaperConfigurationGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    GT_U32                              maxBucketSize;
    GT_U32                              maxRate;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChIpMllMultiTargetShaperConfigurationGet(devNum, &maxBucketSize,&maxRate);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d",
                 maxBucketSize,
                 maxRate);

    return CMD_OK;
}


/**
* @internal wrCpssDxChIpMllMultiTargetShaperIntervalConfigurationSet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Set configuration for Mll shaper Interval.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChIpMllMultiTargetShaperIntervalConfigurationSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                            result;
    GT_U8                                devNum;
    GT_U32                                          tokenBucketIntervalSlowUpdateRatio;
    GT_U32                                          tokenBucketUpdateInterval;
    CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_ENT   tokenBucketIntervalUpdateRatio;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    tokenBucketIntervalSlowUpdateRatio   = (GT_U32)inArgs[1];
    tokenBucketUpdateInterval = (GT_U32)inArgs[2];
    tokenBucketIntervalUpdateRatio = (GT_U32)inArgs[3];

    /* call cpss api function */
    result = cpssDxChIpMllMultiTargetShaperIntervalConfigurationSet(devNum,
                                                            tokenBucketIntervalSlowUpdateRatio,
                                                            tokenBucketUpdateInterval,
                                                            tokenBucketIntervalUpdateRatio);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}
/**
* @internal wrCpssDxChIpMllMultiTargetShaperIntervalConfigurationGet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Get configuration for shaper Interval.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
static CMD_STATUS wrCpssDxChIpMllMultiTargetShaperIntervalConfigurationGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    GT_U32                                          tokenBucketIntervalSlowUpdateRatio;
    GT_U32                                          tokenBucketUpdateInterval;
    CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_ENT   tokenBucketIntervalUpdateRatio;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChIpMllMultiTargetShaperIntervalConfigurationGet(devNum,
                                                            &tokenBucketIntervalSlowUpdateRatio,
                                                            &tokenBucketUpdateInterval,
                                                            &tokenBucketIntervalUpdateRatio);
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d",
                 tokenBucketIntervalSlowUpdateRatio,
                 tokenBucketUpdateInterval,
                 tokenBucketIntervalUpdateRatio);

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpPbrBypassRouterTriggerRequirementsEnableSet function
* @endinternal
*
* @brief   Enable/disable bypassing the router triggering requirements for policy
*         based routing packets
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChIpPbrBypassRouterTriggerRequirementsEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_BOOL   enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChIpPbrBypassRouterTriggerRequirementsEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpPbrBypassRouterTriggerRequirementsEnableGet function
* @endinternal
*
* @brief   Get the enabling status of bypassing the router triggering requirements
*         for policy based routing packets
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChIpPbrBypassRouterTriggerRequirementsEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_BOOL   enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChIpPbrBypassRouterTriggerRequirementsEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpVlanMrstBitmapSet function
* @endinternal
*
* @brief   Set the next-hop MRST state bitmap per eVLAN.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChIpVlanMrstBitmapSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U16    vlanId;
    GT_U64    mrstBmp;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];
    mrstBmp.l[0] = (GT_U32)inArgs[2];
    mrstBmp.l[1] = (GT_U32)inArgs[3];

    /* call cpss api function */
    result = cpssDxChIpVlanMrstBitmapSet(devNum, vlanId , &mrstBmp);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpVlanMrstBitmapGet function
* @endinternal
*
* @brief   Get the next-hop MRST state bitmap per eVLAN.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChIpVlanMrstBitmapGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U16    vlanId;
    GT_U64    mrstBmp;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];

    /* call cpss api function */
    result = cpssDxChIpVlanMrstBitmapGet(devNum, vlanId , &mrstBmp);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", mrstBmp.l[0], mrstBmp.l[1]);

    return CMD_OK;
}
/**
* @internal wrCpssDxChIpFcoeExceptionPacketCommandSet function
* @endinternal
*
* @brief   Set packet command of invalid FCoE packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The commands are:
*       CPSS_PACKET_CMD_FORWARD_E
*       CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*       CPSS_PACKET_CMD_TRAP_TO_CPU_E
*       CPSS_PACKET_CMD_DROP_HARD_E
*       CPSS_PACKET_CMD_DROP_SOFT_E
*
*/
static CMD_STATUS wrCpssDxChIpFcoeExceptionPacketCommandSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS           result;
    GT_U8               devNum;
    CPSS_PACKET_CMD_ENT command;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    command = (CPSS_PACKET_CMD_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChIpFcoeExceptionPacketCommandSet(devNum, command);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpFcoeExceptionPacketCommandGet function
* @endinternal
*
* @brief   Get packet command of invalid FCoE packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The commands are:
*       CPSS_PACKET_CMD_FORWARD_E
*       CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*       CPSS_PACKET_CMD_TRAP_TO_CPU_E
*       CPSS_PACKET_CMD_DROP_HARD_E
*       CPSS_PACKET_CMD_DROP_SOFT_E
*
*/
static CMD_STATUS wrCpssDxChIpFcoeExceptionPacketCommandGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS           result;
    GT_U8               devNum;
    CPSS_PACKET_CMD_ENT command;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChIpFcoeExceptionPacketCommandGet(devNum, &command);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", command);

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpFcoeExceptionCpuCodeSet function
* @endinternal
*
* @brief   Set FCoE exception CPU code.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChIpFcoeExceptionCpuCodeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_U8                           devNum;
    CPSS_NET_RX_CPU_CODE_ENT        cpuCode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    cpuCode = (CPSS_NET_RX_CPU_CODE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChIpFcoeExceptionCpuCodeSet(devNum, cpuCode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}
/**
* @internal wrCpssDxChIpFcoeExceptionCpuCodeGet function
* @endinternal
*
* @brief   Get FCoE exception CPU code.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChIpFcoeExceptionCpuCodeGet
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
    result = cpssDxChIpFcoeExceptionCpuCodeGet(devNum, &cpuCode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", cpuCode);

    return CMD_OK;
}
/**
* @internal wrCpssDxChIpFcoeSoftDropRouterEnableSet function
* @endinternal
*
* @brief   When enabled, FCoE packets with a SOFT_DROP command can trigger FCoE Forwarding.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChIpFcoeSoftDropRouterEnableSet
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
    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChIpFcoeSoftDropRouterEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}
/**
* @internal wrCpssDxChIpFcoeSoftDropRouterEnableGet function
* @endinternal
*
* @brief   Return if FCoE packets with a SOFT_DROP command can trigger FCoE Forwarding.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChIpFcoeSoftDropRouterEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_BOOL     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChIpFcoeSoftDropRouterEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}
/**
* @internal wrCpssDxChIpFcoeTrapRouterEnableSet function
* @endinternal
*
* @brief   When enabled, FCoE packets with a TRAP command can trigger FCoE Forwarding.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChIpFcoeTrapRouterEnableSet
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
    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChIpFcoeTrapRouterEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}
/**
* @internal wrCpssDxChIpFcoeTrapRouterEnableGet function
* @endinternal
*
* @brief   Return if FCoE packets with a TRAP command can trigger FCoE Forwarding.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChIpFcoeTrapRouterEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_BOOL     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChIpFcoeTrapRouterEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpFcoeBridgedUrpfCheckEnableSet function
* @endinternal
*
* @brief   When enabled, Unicast RPF check is performed for Bridged FCoE traffic.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChIpFcoeBridgedUrpfCheckEnableSet
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
    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChIpFcoeBridgedUrpfCheckEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpFcoeBridgedUrpfCheckEnableGet function
* @endinternal
*
* @brief   Return if Unicast RPF check is performed for Bridged FCoE traffic.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChIpFcoeBridgedUrpfCheckEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_BOOL     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChIpFcoeBridgedUrpfCheckEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpFcoeBridgedUrpfCheckCommandSet function
* @endinternal
*
* @brief   Set packet command assigned to FCoE traffic that fails the UC RPF check.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The commands are:
*       CPSS_PACKET_CMD_FORWARD_E
*       CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*       CPSS_PACKET_CMD_TRAP_TO_CPU_E
*       CPSS_PACKET_CMD_DROP_HARD_E
*       CPSS_PACKET_CMD_DROP_SOFT_E
*
*/
static CMD_STATUS wrCpssDxChIpFcoeBridgedUrpfCheckCommandSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    CPSS_PACKET_CMD_ENT     command;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    command = (CPSS_PACKET_CMD_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChIpFcoeBridgedUrpfCheckCommandSet(devNum, command);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpFcoeBridgedUrpfCheckCommandGet function
* @endinternal
*
* @brief   Get packet command assigned to FCoE traffic that fails the UC RPF check.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The commands are:
*       CPSS_PACKET_CMD_FORWARD_E
*       CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*       CPSS_PACKET_CMD_TRAP_TO_CPU_E
*       CPSS_PACKET_CMD_DROP_HARD_E
*       CPSS_PACKET_CMD_DROP_SOFT_E
*
*/
static CMD_STATUS wrCpssDxChIpFcoeBridgedUrpfCheckCommandGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    CPSS_PACKET_CMD_ENT     command;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChIpFcoeBridgedUrpfCheckCommandGet(devNum, &command);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", command);

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpFcoeUcRpfAccessLevelSet function
* @endinternal
*
* @brief   Set the SIP Access Level for FCoE Unicast packets where SIP is associated
*         with ECMP block, and Unicast RPF with ECMP is enabled.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChIpFcoeUcRpfAccessLevelSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_U32                  accessLevel;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    accessLevel = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChIpFcoeUcRpfAccessLevelSet(devNum, accessLevel);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpFcoeUcRpfAccessLevelGet function
* @endinternal
*
* @brief   Get the SIP Access Level for FCoE Unicast packets where SIP is associated
*         with ECMP block, and Unicast RPF with ECMP is enabled.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChIpFcoeUcRpfAccessLevelGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_U32                  accessLevel;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChIpFcoeUcRpfAccessLevelGet(devNum, &accessLevel);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", accessLevel);

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpFcoeBridgedSidSaMismatchCheckEnableSet function
* @endinternal
*
* @brief   When enabled, S_ID / SA mismatch check is performed for Bridged FCoE traffic.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChIpFcoeBridgedSidSaMismatchCheckEnableSet
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
    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChIpFcoeBridgedSidSaMismatchCheckEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpFcoeBridgedSidSaMismatchCheckEnableGet function
* @endinternal
*
* @brief   Return if S_ID / SA mismatch check is performed for Bridged FCoE traffic.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChIpFcoeBridgedSidSaMismatchCheckEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_BOOL     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChIpFcoeBridgedSidSaMismatchCheckEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpFcoeBridgedSidSaMismatchCommandSet function
* @endinternal
*
* @brief   Set packet command for S_ID/SA exceptions.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The commands are:
*       CPSS_PACKET_CMD_FORWARD_E
*       CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*       CPSS_PACKET_CMD_TRAP_TO_CPU_E
*       CPSS_PACKET_CMD_DROP_HARD_E
*       CPSS_PACKET_CMD_DROP_SOFT_E
*
*/
static CMD_STATUS wrCpssDxChIpFcoeBridgedSidSaMismatchCommandSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    CPSS_PACKET_CMD_ENT     command;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    command = (CPSS_PACKET_CMD_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChIpFcoeBridgedSidSaMismatchCommandSet(devNum, command);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}
/**
* @internal wrCpssDxChIpFcoeBridgedSidSaMismatchCommandGet function
* @endinternal
*
* @brief   Get packet command for S_ID/SA exceptions.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The commands are:
*       CPSS_PACKET_CMD_FORWARD_E
*       CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*       CPSS_PACKET_CMD_TRAP_TO_CPU_E
*       CPSS_PACKET_CMD_DROP_HARD_E
*       CPSS_PACKET_CMD_DROP_SOFT_E
*
*/
static CMD_STATUS wrCpssDxChIpFcoeBridgedSidSaMismatchCommandGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    CPSS_PACKET_CMD_ENT     command;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChIpFcoeBridgedSidSaMismatchCommandGet(devNum, &command);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", command);

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpFcoeBridgedSidFilterEnableSet function
* @endinternal
*
* @brief   When enabled, S_ID filtering is performed for Bridged FCoE traffic.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChIpFcoeBridgedSidFilterEnableSet
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
    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChIpFcoeBridgedSidFilterEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpFcoeBridgedSidFilterEnableGet function
* @endinternal
*
* @brief   Return if S_ID filtering is performed for Bridged FCoE traffic.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChIpFcoeBridgedSidFilterEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_BOOL                 enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChIpFcoeBridgedSidFilterEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpFcoeBridgedHeaderErrorCheckEnableSet function
* @endinternal
*
* @brief   When enabled, FCoE header error checking is performed
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChIpFcoeBridgedHeaderErrorCheckEnableSet
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
    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChIpFcoeBridgedHeaderErrorCheckEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}
/**
* @internal wrCpssDxChIpFcoeBridgedHeaderErrorCheckEnableGet function
* @endinternal
*
* @brief   Return if FCoE header error checking is performed,
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChIpFcoeBridgedHeaderErrorCheckEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_BOOL                 enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChIpFcoeBridgedHeaderErrorCheckEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpFdbRoutePrefixLenSet function
* @endinternal
*
* @brief   set the IPv4/6 prefix length when accessing the FDB
*          table for IPv4/6 Route lookup
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*          Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] protocolStack            - the type of protocol
* @param[in] prefixLen                - FDB lookup prefix length.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChIpFdbRoutePrefixLenSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_U8                       devNum;
    CPSS_IP_PROTOCOL_STACK_ENT  protocolStack;
    GT_U32                      prefixLen;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    protocolStack = (CPSS_IP_PROTOCOL_STACK_ENT)inArgs[1];
    prefixLen = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChIpFdbRoutePrefixLenSet(devNum, protocolStack, prefixLen);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpFdbRoutePrefixLenGet function
* @endinternal
*
* @brief   Get the IPv4/6 prefix length when accessing the FDB
*          table for IPv4/6 Route lookup
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*          Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] protocolStack            - the type of protocol
*
* @param[out] prefixLenPtr            - (pointer to)FDB lookup prefix length.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_BAD_VALUE             - on bad output value
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChIpFdbRoutePrefixLenGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_U8                       devNum;
    CPSS_IP_PROTOCOL_STACK_ENT  protocolStack;
    GT_U32                      prefixLen;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    protocolStack = (CPSS_IP_PROTOCOL_STACK_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChIpFdbRoutePrefixLenGet(devNum, protocolStack, &prefixLen);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", prefixLen);

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpFdbUnicastRouteForPbrEnableSet function
* @endinternal
*
* @brief   Enable/Disable FDB Unicast routing for PBR (Policy Basedssss
*          Routed) packets
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*          Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - enable/disable FDB
*                                       routing for PBR packets
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This is useful for overriding the PBR forwarding
*    decision by a matching FDB Route entry.
*/
static CMD_STATUS wrCpssDxChIpFdbUnicastRouteForPbrEnableSet
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
    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChIpFdbUnicastRouteForPbrEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChIpFdbUnicastRouteForPbrEnableGet function
* @endinternal
*
* @brief   Return if FDB Unicast routing for PBR value (Policy Based
*          Routed) packets is enabled
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*          Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr               - (pointer to) enabling status
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This is useful for overriding the PBR forwarding
*    decision by a matching FDB Route entry.
*/
static CMD_STATUS wrCpssDxChIpFdbUnicastRouteForPbrEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_BOOL                 enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChIpFdbUnicastRouteForPbrEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
    {"cpssDxChIpSpecialRouterTriggerEnable",
        &wrCpssDxChIpSpecialRouterTriggerEnable,
        3, 0},

    {"cpssDxChIpSpecialRouterTriggerEnableGet",
        &wrCpssDxChIpSpecialRouterTriggerEnableGet,
        2, 0},

    {"cpssDxChIpExceptionCommandSet",
        &wrCpssDxChIpExceptionCommandSet,
        4, 0},

    {"cpssDxChIpExceptionCommandGetExt",
        &wrCpssDxChIpExceptionCommandGetExt,
        3, 0},

    {"cpssDxChIpUcRouteAgingModeSet",
        &wrCpssDxChIpUcRouteAgingModeSet,
        2, 0},

    {"cpssDxChIpUcRouteAgingModeGet",
        &wrCpssDxChIpUcRouteAgingModeGet,
        1, 0},

    {"cpssDxChIpRouterSourceIdSet",
        &wrCpssDxChIpRouterSourceIdSet,
        3, 0},

    {"cpssDxChIpRouterSourceIdGet",
        &wrCpssDxChIpRouterSourceIdGet,
        2, 0},

    {"cpssDxChIpRouterSourceId_1Set",
        &wrCpssDxChIpRouterSourceId_1Set,
        4, 0},

    {"cpssDxChIpRouterSourceId_1Get",
        &wrCpssDxChIpRouterSourceId_1Get,
        2, 0},

    {"cpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueSet",
        &wrCpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueSet,
        3, 0},

    {"cpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueGet",
        &wrCpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueGet,
        1, 0},

    {"cpssDxChIpQosProfileToMultiTargetTCQueueMapSet",
        &wrCpssDxChIpQosProfileToMultiTargetTCQueueMapSet,
        3, 0},

    {"cpssDxChIpQosProfileToMultiTargetTCQueueMapGet",
        &wrCpssDxChIpQosProfileToMultiTargetTCQueueMapGet,
        2, 0},

    {"cpssDxChIpTcDpToMultiTargetTcQueueMapSet",
        &wrCpssDxChIpTcDpToMultiTargetTcQueueMapSet,
        4, 0},

    {"cpssDxChIpTcDpToMultiTargetTcQueueMapGet",
        &wrCpssDxChIpTcDpToMultiTargetTCQueueMapGet,
        3, 0},

    {"cpssDxChIpMultiTargetQueueFullDropCntGet",
        &wrCpssDxChIpMultiTargetQueueFullDropCntGet,
        1, 0},

    {"cpssDxChIpMultiTargetQueueFullDropCntSet",
        &wrCpssDxChIpMultiTargetQueueFullDropCntSet,
        2, 0},

    {"cpssDxChIpMultiTargetTCQueueSchedModeSet",
        &wrCpssDxChIpMultiTargetTCQueueSchedModeSet,
        4, 0},

    {"cpssDxChIpMultiTargetTCQueueSchedModeGet",
        &wrCpssDxChIpMultiTargetTCQueueSchedModeGet,
        2, 0},

    {"cpssDxChIpBridgeServiceEnable",
        &wrCpssDxChIpBridgeServiceEnable,
        4, 0},

    {"cpssDxChIpBridgeServiceEnableGet",
        &wrCpssDxChIpBridgeServiceEnableGet,
        3, 0},

    {"cpssDxChIpMllBridgeEnable",
        &wrCpssDxChIpMllBridgeEnable,
        2, 0},

    {"cpssDxChIpMllBridgeEnableGet",
        &wrCpssDxChIpMllBridgeEnableGet,
        1, 0},

    {"cpssDxChIpMultiTargetRateShaperSet",
        &wrCpssDxChIpMultiTargetRateShaperSet,
        3, 0},

    {"cpssDxChIpMultiTargetRateShaperGet",
        &wrCpssDxChIpMultiTargetRateShaperGet,
        1, 0},

    {"cpssDxChIpMultiTargetUcSchedModeSet",
        &wrCpssDxChIpMultiTargetUcSchedModeSet,
        5, 0},

    {"cpssDxChIpMultiTargetUcSchedModeGet",
        &wrCpssDxChIpMultiTargetUcSchedModeGet,
        1, 0},

    {"cpssDxChIpArpBcModeSet",
        &wrCpssDxChIpArpBcModeSet,
        2, 0},

    {"cpssDxChIpArpBcModeGet",
        &wrCpssDxChIpArpBcModeGet,
        1, 0},

    {"cpssDxChIpPortRoutingEnable",
        &wrCpssDxChIpPortRoutingEnable,
        5, 0},

    {"cpssDxChIpPortRoutingEnableGet",
        &wrCpssDxChIpPortRoutingEnableGet,
        4, 0},

    {"cpssDxChIpPortFcoeForwardingEnableSet",
        &wrCpssDxChIpPortFcoeForwardingEnable,
        3, 0},

    {"cpssDxChIpPortFcoeForwardingEnableGet",
        &wrCpssDxChIpPortFcoeForwardingEnableGet,
        2, 0},

    {"cpssDxChIpQosProfileToRouteEntryMapSet",
        &wrCpssDxChIpQosProfileToRouteEntryMapSet,
        3, 0},

    {"cpssDxChIpQosProfileToRouteEntryMapGet",
        &wrCpssDxChIpQosProfileToRouteEntryMapGet,
        2, 0},

    {"cpssDxChIpRoutingEnable",
        &wrCpssDxChIpRoutingEnable,
        2, 0},

    {"cpssDxChIpRoutingEnableGet",
        &wrCpssDxChIpRoutingEnableGet,
        1, 0},

    {"cpssDxChIpCntGetFirst",
        &wrCpssDxChIpCntGetFirst,
        1, 0},

    {"cpssDxChIpCntGetNext",
        &wrCpssDxChIpCntGetNext,
        1, 0},

    {"cpssDxChIpCntSetModeSet",
        &wrCpssDxChIpCntSetModeSet,
        1, 9},

    {"cpssDxChIpCntSetModeGetFirst",
        &wrCpssDxChIpCntSetModeGetFirst,
        1, 0},
      {"cpssDxChIpCntSetModeGetNext",
        &wrCpssDxChIpCntSetModeGetNext,
        1, 0},

    {"cpssDxChIpCntSet",
        &wrCpssDxChIpCntSet,
        1, 11},

    {"cpssDxChIpMllSkippedEntriesCountersGet",
        &wrCpssDxChIpMllSkippedEntriesCountersGet,
        1, 0},

    {"cpssDxChIpMllCntInterfaceSet",
        &wrCpssDxChIpSetMllCntInterface,
        1, 8},

    {"cpssDxChIpMllCntGet",
        &wrCpssDxChIpMllCntGet,
        2, 0},

    {"cpssDxChIpMllCntSet",
        &wrCpssDxChIpMllCntSet,
        3, 0},

    {"cpssDxChIpMllSilentDropCntGet",
        &wrCpssDxChIpMllSilentDropCntGet,
        1, 0},

    {"cpssDxChIpDropCntSet",
        &wrCpssDxChIpDropCntSet,
        2, 0},

    {"cpssDxChIpSetDropCntMode",
        &wrCpssDxChIpSetDropCntMode,
        2, 0},

    {"cpssDxChIpDropCntGet",
        &wrCpssDxChIpDropCntGet,
        1, 0},

    {"cpssDxChIpMtuProfileSet",
        &wrCpssDxChIpMtuProfileSet,
        3, 0},

    {"cpssDxChIpMtuProfileGet",
        &wrCpssDxChIpMtuProfileGet,
        2, 0},

    {"cpssDxChIpv6AddrPrefixScopeSet",
        &wrCpssDxChIpv6AddrPrefixScopeSet,
        5, 0},

    {"cpssDxChIpv6AddrPrefixScopeGet",
        &wrCpssDxChIpv6AddrPrefixScopeGet,
        2, 0},

    {"cpssDxChIpv6AddrPrefixScopeGetFirst",
        &wrCpssDxChIpv6AddrPrefixScopeGetFirst,
        1, 0},

    {"cpssExMxPmIpv6AddrPrefixScopeGetNext",
        &wrCpssDxChIpv6AddrPrefixScopeGetNext,
        1, 0},

    {"cpssDxChIpv6UcScopeCommandSet",
        &wrCpssDxChIpv6UcScopeCommandSet,
        5, 0},

    {"cpssDxChIpv6UcScopeCommandGet",
        &wrCpssDxChIpv6UcScopeCommandGet,
        4, 0},

    {"cpssDxChIpv6McScopeCommandSet",
        &wrCpssDxChIpv6McScopeCommandSet,
        6, 0},

    {"cpssDxChIpv6McScopeCommandGet",
        &wrCpssDxChIpv6McScopeCommandGet,
        4, 0},

    {"cpssDxChIpRouterMacSaBaseSet",
        &wrCpssDxChIpRouterMacSaBaseSet,
        2, 0},

    {"cpssDxChIpRouterMacSaBaseGet",
        &wrCpssDxChIpRouterMacSaBaseGet,
        1, 0},

    {"cpssDxChIpRouterMacSaModifyEnable",
        &wrCpssDxChIpRouterMacSaModifyEnable,
        3, 0},

    {"cpssDxChIpRouterMacSaModifyEnableGet",
        &wrCpssDxChIpRouterMacSaModifyEnableGet,
        2, 0},

   {"cpssDxChIpPortRouterMacSaLsbModeSet",
        &wrCpssDxChIpPortRouterMacSaLsbModeSet,
        3, 0},
    {"cpssDxChIpPortRouterMacSaLsbModeGet",
        &wrCpssDxChIpPortRouterMacSaLsbModeGet,
        2, 0},
    {"cpssDxChIpRouterGlobalMacSaSet",
        &wrCpssDxChIpRouterGlobalMacSaSet,
        3, 0},
    {"cpssDxChIpRouterGlobalMacSaGet",
        &wrCpssDxChIpRouterGlobalMacSaGet,
        2, 0},
    {"cpssDxChIpRouterGlobalMacSaIndexSet",
        &wrCpssDxChIpRouterGlobalMacSaIndexSet,
        3, 0},
    {"cpssDxChIpRouterGlobalMacSaIndexGet",
        &wrCpssDxChIpRouterGlobalMacSaIndexGet,
        2, 0},

    {"cpssDxChIpRouterPortMacSaLsbSet",
        &wrCpssDxChIpRouterPortMacSaLsbSet,
        3, 0},
    {"cpssDxChIpRouterPortMacSaLsbGet",
        &wrCpssDxChIpRouterPortMacSaLsbGet,
        2, 0},

    {"cpssDxChIpRouterVlanMacSaLsbSet",
         &wrCpssDxChIpRouterVlanMacSaLsbSet,
         3, 0},
    {"cpssDxChIpRouterVlanMacSaLsbGet",
         &wrCpssDxChIpRouterVlanMacSaLsbGet,
         2, 0},
    {"cpssDxChIpEcmpUcRpfCheckEnableSet",
         &wrCpssDxChIpEcmpUcRpfCheckEnableSet,
         2, 0},
    {"cpssDxChIpEcmpUcRpfCheckEnableGet",
         &wrCpssDxChIpEcmpUcRpfCheckEnableGet,
         1, 0},
    {"cpssDxChIpExceptionCommandSetExt",
        &wrCpssDxChIpExceptionCommandSet,
        4, 0},

    {"cpssDxChIpUcRpfVlanModeSet",
    &wrCpssDxChIpUcRpfModeSet,
    3, 0},

    {"cpssDxChIpUcRpfVlanModeGet",
    &wrCpssDxChIpUcRpfModeGet,
    2, 0},

    {"cpssDxChIpUcRpfVlanModeSet_C001",
    &wrCpssDxChIpUcRpfModeSet,
    3, 0},

    {"cpssDxChIpUcRpfVlanModeGet_C001",
    &wrCpssDxChIpUcRpfModeGet,
    2, 0},

    {"cpssDxChIpUrpfLooseModeTypeSet",
    &wrCpssDxChIpUrpfLooseModeTypeSet,
    2, 0},

    {"cpssDxChIpUrpfLooseModeTypeGet",
    &wrCpssDxChIpUrpfLooseModeTypeGet,
    1, 0},

    {"cpssDxChIpPortSipSaEnableSet",
    &wrCpssDxChIpPortSipSaEnableSet,
    3, 0},

    {"cpssDxChIpPortSipSaEnableGet",
    &wrCpssDxChIpPortSipSaEnableGet,
    2, 0},
    {"cpssDxChIpRouterBridgedPacketsExceptionCntSet",
        &wrCpssDxChIpRouterBridgedPacketsExceptionCntSet,
        2, 0},
    {"cpssDxChIpRouterBridgedPacketsExceptionCntGet",
        &wrCpssDxChIpRouterBridgedPacketsExceptionCntGet,
        1, 0},
    {"cpssDxChIpRouterSourceIdOverrideEnableSet",
         &wrCpssDxChIpRouterSourceIdOverrideEnableSet,
         2, 0},
    {"cpssDxChIpRouterSourceIdOverrideEnableGet",
         &wrCpssDxChIpRouterSourceIdOverrideEnableGet,
         1, 0},

    {"cpssDxChIpHeaderErrorMaskSet",
        &wrCpssDxChIpHeaderErrorMaskSet,
        5, 0},

    {"cpssDxChIpHeaderErrorMaskGet",
        &wrCpssDxChIpHeaderErrorMaskGet,
        4, 0},

    {"cpssDxChIpUcRoutingVid1AssignEnableSet",
        &wrCpssDxChIpUcRoutingVid1AssignEnableSet,
        2, 0},

    {"cpssDxChIpUcRoutingVid1AssignEnableGet",
        &wrCpssDxChIpUcRoutingVid1AssignEnableGet,
        1, 0},

    {"cpssDxChIpMllMultiTargetShaperBaselineSet",
        &wrCpssDxChIpMllMultiTargetShaperBaselineSet,
        2, 0},
    {"cpssDxChIpMllMultiTargetShaperBaselineGet",
        &wrCpssDxChIpMllMultiTargetShaperBaselineGet,
        1, 0},
    {"cpssDxChIpMllMultiTargetShaperMtuSet",
        &wrCpssDxChIpMllMultiTargetShaperMtuSet,
        2, 0},
    {"cpssDxChIpMllMultiTargetShaperMtuGet",
        &wrCpssDxChIpMllMultiTargetShaperMtuGet,
        1, 0},

    {"cpssDxChIpMllMultiTargetShaperTokenBucketModeSet",
        &wrCpssDxChIpMllMultiTargetShaperTokenBucketModeSet,
        2, 0},
    {"cpssDxChIpMllMultiTargetShaperTokenBucketModeGet",
        &wrCpssDxChIpMllMultiTargetShaperTokenBucketModeGet,
        1, 0},

    {"cpssDxChIpMllMultiTargetShaperEnableSet",
        &wrCpssDxChIpMllMultiTargetShaperEnableSet,
        2, 0},
    {"cpssDxChIpMllMultiTargetShaperEnableGet",
        &wrCpssDxChIpMllMultiTargetShaperEnableGet,
        1, 0},
    {"cpssDxChIpMllMultiTargetShaperConfigurationSet",
        &wrCpssDxChIpMllMultiTargetShaperConfigurationSet,
        3, 0},
    {"cpssDxChIpMllMultiTargetShaperConfigurationGet",
        &wrCpssDxChIpMllMultiTargetShaperConfigurationGet,
        1, 0},
    {"cpssDxChIpMllMultiTargetShaperIntervalConfigSet",
        &wrCpssDxChIpMllMultiTargetShaperIntervalConfigurationSet,
        4, 0},
    {"cpssDxChIpMllMultiTargetShaperIntervalConfigGet",
        &wrCpssDxChIpMllMultiTargetShaperIntervalConfigurationGet,
        1, 0},

    {"cpssDxChIpPbrBypassRouterTriggerEnableSet",
        &wrCpssDxChIpPbrBypassRouterTriggerRequirementsEnableSet,
        2, 0},
    {"cpssDxChIpPbrBypassRouterTriggerEnableGet",
        &wrCpssDxChIpPbrBypassRouterTriggerRequirementsEnableGet,
        1, 0},

    {"cpssDxChIpVlanMrstBitmapSet",
        &wrCpssDxChIpVlanMrstBitmapSet,
        4, 0},
    {"cpssDxChIpVlanMrstBitmapGet",
        &wrCpssDxChIpVlanMrstBitmapGet,
        2, 0},

    {"cpssDxChIpFcoeExceptionPacketCommandSet",
        &wrCpssDxChIpFcoeExceptionPacketCommandSet,
        2, 0},
    {"cpssDxChIpFcoeExceptionPacketCommandGet",
        &wrCpssDxChIpFcoeExceptionPacketCommandGet,
        1, 0},
    {"cpssDxChIpFcoeExceptionCpuCodeSet",
        &wrCpssDxChIpFcoeExceptionCpuCodeSet,
        2, 0},
    {"cpssDxChIpFcoeExceptionCpuCodeGet",
        &wrCpssDxChIpFcoeExceptionCpuCodeGet,
        1, 0},
    {"cpssDxChIpFcoeSoftDropRouterEnableSet",
        &wrCpssDxChIpFcoeSoftDropRouterEnableSet,
        2, 0},
    {"cpssDxChIpFcoeSoftDropRouterEnableGet",
        &wrCpssDxChIpFcoeSoftDropRouterEnableGet,
        1, 0},
    {"cpssDxChIpFcoeTrapRouterEnableSet",
        &wrCpssDxChIpFcoeTrapRouterEnableSet,
        2, 0},
    {"cpssDxChIpFcoeTrapRouterEnableGet",
        &wrCpssDxChIpFcoeTrapRouterEnableGet,
        1, 0},
    {"cpssDxChIpFcoeBridgedUrpfCheckEnableSet",
        &wrCpssDxChIpFcoeBridgedUrpfCheckEnableSet,
        2, 0},
    {"cpssDxChIpFcoeBridgedUrpfCheckEnableGet",
        &wrCpssDxChIpFcoeBridgedUrpfCheckEnableGet,
        1, 0},
    {"cpssDxChIpFcoeBridgedUrpfCheckCommandSet",
        &wrCpssDxChIpFcoeBridgedUrpfCheckCommandSet,
        2, 0},
    {"cpssDxChIpFcoeBridgedUrpfCheckCommandGet",
        &wrCpssDxChIpFcoeBridgedUrpfCheckCommandGet,
        1, 0},
    {"cpssDxChIpFcoeUcRpfAccessLevelSet",
        &wrCpssDxChIpFcoeUcRpfAccessLevelSet,
        2, 0},
    {"cpssDxChIpFcoeUcRpfAccessLevelGet",
        &wrCpssDxChIpFcoeUcRpfAccessLevelGet,
        1, 0},
    {"cpssDxChIpFcoeBridgedSidSaMismatchCheckEnableSet",
        &wrCpssDxChIpFcoeBridgedSidSaMismatchCheckEnableSet,
        2, 0},
    {"cpssDxChIpFcoeBridgedSidSaMismatchCheckEnableGet",
        &wrCpssDxChIpFcoeBridgedSidSaMismatchCheckEnableGet,
        1, 0},
    {"cpssDxChIpFcoeBridgedSidSaMismatchCheckEnableSet",
        &wrCpssDxChIpFcoeBridgedSidSaMismatchCheckEnableSet,
        2, 0},
    {"cpssDxChIpFcoeBridgedSidSaMismatchCheckEnableGet",
        &wrCpssDxChIpFcoeBridgedSidSaMismatchCheckEnableGet,
        1, 0},
    {"cpssDxChIpFcoeBridgedSidSaMismatchCommandSet",
        &wrCpssDxChIpFcoeBridgedSidSaMismatchCommandSet,
        2, 0},
    {"cpssDxChIpFcoeBridgedSidSaMismatchCommandGet",
        &wrCpssDxChIpFcoeBridgedSidSaMismatchCommandGet,
        1, 0},
    {"cpssDxChIpFcoeBridgedSidFilterEnableSet",
        &wrCpssDxChIpFcoeBridgedSidFilterEnableSet,
        2, 0},
    {"cpssDxChIpFcoeBridgedSidFilterEnableGet",
        &wrCpssDxChIpFcoeBridgedSidFilterEnableGet,
        1, 0},
    {"cpssDxChIpFcoeBridgedHeaderErrorCheckEnableSet",
        &wrCpssDxChIpFcoeBridgedHeaderErrorCheckEnableSet,
        2, 0},
    {"cpssDxChIpFcoeBridgedHeaderErrorCheckEnableGet",
        &wrCpssDxChIpFcoeBridgedHeaderErrorCheckEnableGet,
        1, 0},
    {"cpssDxChIpFdbRoutePrefixLenSet",
        &wrCpssDxChIpFdbRoutePrefixLenSet,
        3, 0},
    {"cpssDxChIpFdbRoutePrefixLenGet",
        &wrCpssDxChIpFdbRoutePrefixLenGet,
        1, 0},
    {"cpssDxChIpFdbUnicastRouteForPbrEnableSet",
        &wrCpssDxChIpFdbUnicastRouteForPbrEnableSet,
        2, 0},
    {"cpssDxChIpFdbUnicastRouteForPbrEnableGet",
        &wrCpssDxChIpFdbUnicastRouteForPbrEnableGet,
        1, 0}
};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibResetCpssDxChIpCtrl function
* @endinternal
*
* @brief   Library database reset function.
*
* @note none
*
*/
static GT_VOID cmdLibResetCpssDxChIpCtrl
(
    GT_VOID
)
{
    cntSetCnt = CPSS_IP_CNT_SET0_E;
    index = 0;
}


/**
* @internal cmdLibInitCpssDxChIpCtrl function
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
GT_STATUS cmdLibInitCpssDxChIpCtrl
(
    GT_VOID
)
{
    GT_STATUS rc;

    rc = wrCpssRegisterResetCb(cmdLibResetCpssDxChIpCtrl);
    if(rc != GT_OK)
    {
        return rc;
    }

    return cmdInitLibrary(dbCommands, numCommands);
}




