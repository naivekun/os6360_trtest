/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* wrapCpssDxChL2mll.c
*
* DESCRIPTION:
*       Wrapper functions for Cpss Dx L2MLL
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 50 $
*
*******************************************************************************/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>


/* Feature specific includes. */
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/l2mll/cpssDxChL2Mll.h>
#include <cpss/dxCh/dxChxGen/cpssDxChTypes.h>

/**
* @internal l2MllMultiPortGroupsBmpGet function
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
static void l2MllMultiPortGroupsBmpGet
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

/**
* @internal wrCpssDxChL2MllLookupForAllEvidxEnableSet function
* @endinternal
*
* @brief   Enable or disable MLL lookup for all multi-target packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChL2MllLookupForAllEvidxEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;

    GT_U8           devNum;
    GT_BOOL         enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    enable = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChL2MllLookupForAllEvidxEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChL2MllLookupForAllEvidxEnableGet function
* @endinternal
*
* @brief   Get enabling status of MLL lookup for all multi-target packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChL2MllLookupForAllEvidxEnableGet
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
    result = cpssDxChL2MllLookupForAllEvidxEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}


/**
* @internal wrCpssDxChL2MllLookupMaxVidxIndexSet function
* @endinternal
*
* @brief   Set the maximal VIDX value that refers to a port distribution list(VIDX).
*         By default the value is 4K-1
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_OUT_OF_RANGE          - parameter is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant only when cpssDxChL2MllLookupForAllEvidxEnableSet== FALSE
*
*/
static CMD_STATUS wrCpssDxChL2MllLookupMaxVidxIndexSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;

    GT_U8           devNum;
    GT_U32          maxVidxIndex;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    maxVidxIndex = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChL2MllLookupMaxVidxIndexSet(devNum, maxVidxIndex);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChL2MllLookupMaxVidxIndexGet function
* @endinternal
*
* @brief   Get the maximal VIDX value that refers to a port distribution list(VIDX).
*         By default the value is 4K-1
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant only when cpssDxChL2MllLookupForAllEvidxEnableGet == FALSE
*
*/
static CMD_STATUS wrCpssDxChL2MllLookupMaxVidxIndexGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;

    GT_U8       devNum;
    GT_U32      maxVidxIndex;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChL2MllLookupMaxVidxIndexGet(devNum, &maxVidxIndex);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", maxVidxIndex);
    return CMD_OK;
}
/**
* @internal wrCpssDxChL2MllTtlExceptionConfigurationSet function
* @endinternal
*
* @brief   Set configuration for L2 MLL TTL Exceptions.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChL2MllTtlExceptionConfigurationSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_U8                       devNum;
    GT_BOOL                     trapEnable;
    CPSS_NET_RX_CPU_CODE_ENT    cpuCode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    trapEnable = (GT_U32)inArgs[1];
    cpuCode = (CPSS_NET_RX_CPU_CODE_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssDxChL2MllTtlExceptionConfigurationSet(devNum, trapEnable, cpuCode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChL2MllTtlExceptionConfigurationGet function
* @endinternal
*
* @brief   Get configuration of L2 MLL TTL Exceptions.
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
static CMD_STATUS wrCpssDxChL2MllTtlExceptionConfigurationGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_U8                       devNum;
    GT_BOOL                     trapEnable;
    CPSS_NET_RX_CPU_CODE_ENT    cpuCode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChL2MllTtlExceptionConfigurationGet(devNum, &trapEnable, &cpuCode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", trapEnable,cpuCode);
    return CMD_OK;
}
/**
* @internal wrCpssDxChL2MllExceptionCountersGet function
* @endinternal
*
* @brief   Get L2 MLL exception counters.
*         Port Group and Regular version wrapper.
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
*
* @note 1. for non multi-port groups device this parameter is IGNORED.
*       2. for multi-port groups device :
*       bitmap must be set with at least one bit representing
*       valid port group(s). If a bit of non valid port group
*       is set then function returns GT_BAD_PARAM.
*       value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
*/
static CMD_STATUS wrCpssDxChL2MllExceptionCountersGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS           result;

    GT_U8               devNum;
    CPSS_DXCH_L2_MLL_EXCEPTION_COUNTERS_STC  counters;
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    l2MllMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        /* call cpss api function */
        result = cpssDxChL2MllExceptionCountersGet(devNum, &counters);
    }
    else
    {
        /* call cpss api function */
        result = cpssDxChL2MllPortGroupExceptionCountersGet(devNum, pgBmp, &counters);
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", counters);
    return CMD_OK;
}

/**
* @internal wrCpssDxChL2MllCounterGet function
* @endinternal
*
* @brief   Get L2 MLL counter.
*         Port Group and Regular version wrapper
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
*
* @note 1. for non multi-port groups device this parameter is IGNORED.
*       2. for multi-port groups device :
*       bitmap must be set with at least one bit representing
*       valid port group(s). If a bit of non valid port group
*       is set then function returns GT_BAD_PARAM.
*       value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
*/
static CMD_STATUS wrCpssDxChL2MllCounterGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_U32      index;
    GT_U32      counter;
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    index=(GT_U32)inArgs[1];


    l2MllMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        /* call cpss api function */
        result = cpssDxChL2MllCounterGet(devNum, index, &counter);
    }
    else
    {
        /* call cpss api function */
        result = cpssDxChL2MllPortGroupCounterGet(devNum, pgBmp, index, &counter);

    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", counter);
    return CMD_OK;
}

/**
* @internal wrCpssDxChL2MllCountersGet function
* @endinternal
*
* @brief   Get L2 MLL counter.
*         Port Group and Regular version wrapper
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
*
* @note 1. for non multi-port groups device this parameter is IGNORED.
*       2. for multi-port groups device :
*       bitmap must be set with at least one bit representing
*       valid port group(s). If a bit of non valid port group
*       is set then function returns GT_BAD_PARAM.
*       value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
*/
static CMD_STATUS wrCpssDxChL2MllCountersGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_U32      counter;
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    l2MllMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        /* call cpss api function */
        result = cpssDxChL2MllCounterGet(devNum, 0, &counter);
    }
    else
    {
        /* call cpss api function */
        result = cpssDxChL2MllPortGroupCounterGet(devNum, pgBmp, 0, &counter);
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", counter);
    return CMD_OK;
}

/**
* @internal wrCpssDxChL2MllSetCntInterfaceCfg function
* @endinternal
*
* @brief   Sets a L2 mll counter set's bounded inteface.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_OUT_OF_RANGE          - parameter is out of range
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChL2MllSetCntInterfaceCfg

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
    CPSS_DXCH_L2MLL_COUNTER_SET_INTERFACE_CFG_STC interfaceCfgPtr;

    GT_HW_DEV_NUM                                 tmpHwDevNum;
    GT_PORT_NUM                                   tmpPortNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cpssOsMemSet(&interfaceCfgPtr,0,sizeof(CPSS_DXCH_L2MLL_COUNTER_SET_INTERFACE_CFG_STC));

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    mllCntSet = (GT_U32)inArgs[1];

    interfaceCfgPtr.portTrunkCntMode =
                             (CPSS_DXCH_L2MLL_PORT_TRUNK_CNT_MODE_ENT)inArgs[2];
    interfaceCfgPtr.ipMode = (CPSS_IP_PROTOCOL_STACK_ENT)inArgs[3];
    interfaceCfgPtr.vlanMode = (CPSS_DXCH_L2MLL_VLAN_CNT_MODE_ENT)inArgs[4];

    switch(inArgs[2])
    {
    case 1:
        /* check if need to update CPSS_DXCH_L2MLL_COUNTER_SET_INTERFACE_CFG_STC to
           support intergace of type: GT_HW_DEV_NUM, GT_PORT_NUM
           and fix the code accordingly to support more devs and ports*/
        CPSS_TBD_BOOKMARK_EARCH

        tmpHwDevNum = (GT_HW_DEV_NUM)inArgs[5];
        tmpPortNum  = (GT_PORT_NUM)inArgs[6];
        CONVERT_DEV_PORT_DATA_MAC(tmpHwDevNum,tmpPortNum);

        interfaceCfgPtr.hwDevNum = tmpHwDevNum;
        interfaceCfgPtr.portTrunk.port = tmpPortNum;

        break;

    case 2:
        interfaceCfgPtr.portTrunk.trunk = (GT_TRUNK_ID)inArgs[7];
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(interfaceCfgPtr.portTrunk.trunk);
        break;

    default:
        break;
    }

    if(inArgs[4])
        interfaceCfgPtr.vlanId = (GT_U16)inArgs[8];

    /* call cpss api function */
    result = cpssDxChL2MllSetCntInterfaceCfg(devNum, mllCntSet, &interfaceCfgPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChL2MllGetCntInterfaceCfg function
* @endinternal
*
* @brief   Gets a L2 mll counter set's bounded inteface.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_OUT_OF_RANGE          - parameter is out of range
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChL2MllGetCntInterfaceCfg
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
    CPSS_DXCH_L2MLL_COUNTER_SET_INTERFACE_CFG_STC interfaceCfgPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cpssOsMemSet(&interfaceCfgPtr,0,sizeof(CPSS_DXCH_L2MLL_COUNTER_SET_INTERFACE_CFG_STC));

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    mllCntSet = (GT_U32)inArgs[1];

     /* call cpss api function */
    result = cpssDxChL2MllGetCntInterfaceCfg(devNum, mllCntSet, &interfaceCfgPtr);

    interfaceCfgPtr.portTrunkCntMode =
                             (CPSS_DXCH_L2MLL_PORT_TRUNK_CNT_MODE_ENT)inArgs[2];
    interfaceCfgPtr.ipMode = (CPSS_IP_PROTOCOL_STACK_ENT)inArgs[3];
    interfaceCfgPtr.vlanMode = (CPSS_DXCH_L2MLL_VLAN_CNT_MODE_ENT)inArgs[4];

    switch(interfaceCfgPtr.portTrunkCntMode)
    {
    case CPSS_DXCH_L2MLL_PORT_CNT_MODE_E:
        CONVERT_BACK_DEV_PORT_DATA_MAC(interfaceCfgPtr.hwDevNum,interfaceCfgPtr.portTrunk.port);
        break;

    case CPSS_DXCH_L2MLL_TRUNK_CNT_MODE_E:
        CONVERT_TRUNK_ID_CPSS_TO_TEST_MAC(interfaceCfgPtr.portTrunk.trunk);
        break;

    default:
        break;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d%d%d%d",
                 interfaceCfgPtr.portTrunkCntMode,
                 interfaceCfgPtr.ipMode,
                 interfaceCfgPtr.vlanMode,
                 interfaceCfgPtr.hwDevNum,
                 interfaceCfgPtr.portTrunk.port,
                 interfaceCfgPtr.portTrunk.trunk,
                 interfaceCfgPtr.vlanId);
    return CMD_OK;
}


/**
* @internal wrCpssDxChL2MllMcCntGet function
* @endinternal
*
* @brief   Get the L2MLL MC counter
*         Port Group and Regular version wrapper
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. for non multi-port groups device this parameter is IGNORED.
*       2. for multi-port groups device :
*       bitmap must be set with at least one bit representing
*       valid port group(s). If a bit of non valid port group
*       is set then function returns GT_BAD_PARAM.
*       value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
*/
static CMD_STATUS wrCpssDxChL2MllMcCntGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_U32      mllCntSet;
    GT_U32      mllOutMCPkts;
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    mllCntSet=(GT_U32)inArgs[1];

    l2MllMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        /* call cpss api function */
        result = cpssDxChL2MllMcCntGet(devNum, mllCntSet, &mllOutMCPkts);
    }
    else
    {
        /* call cpss api function */
        result = cpssDxChL2MllPortGroupMcCntGet(devNum, pgBmp, mllCntSet, &mllOutMCPkts);

    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mllOutMCPkts);
    return CMD_OK;
}


/**
* @internal wrCpssDxChL2MllSilentDropCntGet function
* @endinternal
*
* @brief   Get the silent drops in the L2 MLL replication block.
*         A drop occurs for a packet that was:
*         - Replicated in the TTI/IP MLL
*         AND
*         - All the elements of the linked list are filtered
*         Port Group and Regular version wrapper
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. for non multi-port groups device this parameter is IGNORED.
*       2. for multi-port groups device :
*       bitmap must be set with at least one bit representing
*       valid port group(s). If a bit of non valid port group
*       is set then function returns GT_BAD_PARAM.
*       value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
*/
static CMD_STATUS wrCpssDxChL2MllSilentDropCntGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_U32      silentDropPkts;
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    l2MllMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        /* call cpss api function */
        result = cpssDxChL2MllSilentDropCntGet(devNum, &silentDropPkts);
    }
    else
    {
        /* call cpss api function */
        result = cpssDxChL2MllPortGroupSilentDropCntGet(devNum, pgBmp, &silentDropPkts);

    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", silentDropPkts);
    return CMD_OK;
}

/**************Table: cpssDxChIpMLLPair****************/
#define MAX_MLL_ENTRIES 16384
GT_U32  gMllPairEntryIndex;
GT_U32  gMllEntryIndex;
GT_U32  tmpIndex;
CPSS_DXCH_L2_MLL_ENTRY_STC  mllPairEntry;

/**
* @internal wrCpssDxChL2MllLttEntrySet function
* @endinternal
*
* @brief   Set L2 MLL Lookup Translation Table (LTT) entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - LTT entry's parameter is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChL2MllLttEntrySet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   status;
    GT_U8                            devNum;
    GT_U32                           lttIndex;
    CPSS_DXCH_L2_MLL_LTT_ENTRY_STC   lttEntry;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum = (GT_U8)inArgs[0];
    lttIndex = (GT_U32)inFields[0];
    lttEntry.mllPointer = (GT_U32)inFields[1];
    lttEntry.entrySelector = CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;
    lttEntry.mllMaskProfileEnable = (GT_BOOL)inFields[2];
    lttEntry.mllMaskProfile = (GT_U32)inFields[3];

    /* call cpss api function */
    status =  cpssDxChL2MllLttEntrySet(devNum, lttIndex,&lttEntry);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChL2MllLttEntryGet function
* @endinternal
*
* @brief   Read L2 LTT MLL Lookup Translation Table (LTT) entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - LTT entry's parameter is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChL2MllLttEntryGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    GT_STATUS   status;
    GT_U8                            devNum;
    CPSS_DXCH_L2_MLL_LTT_ENTRY_STC   lttEntry;
    GT_U32                           rangeEnd;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    rangeEnd = (GT_U32)inArgs[2];

    if(gMllEntryIndex > rangeEnd)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* call cpss api function */
    cmdOsMemSet(&lttEntry, 0, sizeof(lttEntry));

    /* call cpss api function */
    status = cpssDxChL2MllLttEntryGet(devNum, gMllEntryIndex, &lttEntry);

    if (status != GT_OK)
    {
        galtisOutput(outArgs, status, "");
        return CMD_OK;
    }

    inFields[0] = gMllEntryIndex;
    inFields[1] = lttEntry.mllPointer;
    inFields[2] = lttEntry.mllMaskProfileEnable;
    inFields[3] = lttEntry.mllMaskProfile;

    /* pack and output table fields */
    fieldOutput("%d%d%d%d"
        ,inFields[0], inFields[1], inFields[2], inFields[3]);

    galtisOutput(outArgs, status, "%f");

    gMllEntryIndex++;

    return CMD_OK;
}

/**
* @internal wrCpssDxChL2MllLttEntryGetFirst function
* @endinternal
*
* @brief   Read L2 LTT MLL Lookup Translation Table (LTT) entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - LTT entry's parameter is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChL2MllLttEntryGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    GT_STATUS   status;
    GT_U8                            devNum;
    CPSS_DXCH_L2_MLL_LTT_ENTRY_STC   lttEntry;
    GT_U32                           rangeStart;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    rangeStart = (GT_U32)inArgs[1];

    gMllEntryIndex = rangeStart;

     /* call cpss api function */
     cmdOsMemSet(&lttEntry, 0, sizeof(lttEntry));

    /* call cpss api function */
    status = cpssDxChL2MllLttEntryGet(devNum, gMllEntryIndex, &lttEntry);

    if (status != GT_OK)
    {
        galtisOutput(outArgs, status, "");
        return CMD_OK;
    }

    return wrCpssDxChL2MllLttEntryGet(inArgs,inFields,numFields,outArgs);
}

/**
* @internal wrCpssDxChL2MllLttEntry_1Set function
* @endinternal
*
* @brief   Set L2 MLL Lookup Translation Table (LTT) entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - LTT entry's parameter is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChL2MllLttEntry_1Set
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   status;
    GT_U8                            devNum;
    GT_U32                           lttIndex;
    CPSS_DXCH_L2_MLL_LTT_ENTRY_STC   lttEntry;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum = (GT_U8)inArgs[0];
    lttIndex = (GT_U32)inFields[0];
    lttEntry.mllPointer = (GT_U32)inFields[1];
    lttEntry.entrySelector = (CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_ENT)inFields[2];
    lttEntry.mllMaskProfileEnable = (GT_BOOL)inFields[3];
    lttEntry.mllMaskProfile = (GT_U32)inFields[4];

    /* call cpss api function */
    status =  cpssDxChL2MllLttEntrySet(devNum, lttIndex,&lttEntry);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChL2MllLttEntry_1Get function
* @endinternal
*
* @brief   Read L2 LTT MLL Lookup Translation Table (LTT) entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - LTT entry's parameter is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChL2MllLttEntry_1Get
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    GT_STATUS   status;
    GT_U8                            devNum;
    CPSS_DXCH_L2_MLL_LTT_ENTRY_STC   lttEntry;
    GT_U32                           rangeEnd;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    rangeEnd = (GT_U32)inArgs[2];

    if(gMllEntryIndex > rangeEnd)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* call cpss api function */
    cmdOsMemSet(&lttEntry, 0, sizeof(lttEntry));

    /* call cpss api function */
    status = cpssDxChL2MllLttEntryGet(devNum, gMllEntryIndex, &lttEntry);

    if (status != GT_OK)
    {
        galtisOutput(outArgs, status, "");
        return CMD_OK;
    }

    inFields[0] = gMllEntryIndex;
    inFields[1] = lttEntry.mllPointer;
    inFields[2] = lttEntry.entrySelector;
    inFields[3] = lttEntry.mllMaskProfileEnable;
    inFields[4] = lttEntry.mllMaskProfile;

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d"
        ,inFields[0], inFields[1], inFields[2], inFields[3], inFields[4]);

    galtisOutput(outArgs, status, "%f");

    gMllEntryIndex++;

    return CMD_OK;
}

/**
* @internal wrCpssDxChL2MllLttEntry_1GetFirst function
* @endinternal
*
* @brief   Read L2 LTT MLL Lookup Translation Table (LTT) entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - LTT entry's parameter is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChL2MllLttEntry_1GetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    GT_STATUS   status;
    GT_U8                            devNum;
    CPSS_DXCH_L2_MLL_LTT_ENTRY_STC   lttEntry;
    GT_U32                           rangeStart;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    rangeStart = (GT_U32)inArgs[1];

    gMllEntryIndex = rangeStart;

     /* call cpss api function */
     cmdOsMemSet(&lttEntry, 0, sizeof(lttEntry));

    /* call cpss api function */
    status = cpssDxChL2MllLttEntryGet(devNum, gMllEntryIndex, &lttEntry);

    if (status != GT_OK)
    {
        galtisOutput(outArgs, status, "");
        return CMD_OK;
    }

    return wrCpssDxChL2MllLttEntry_1Get(inArgs,inFields,numFields,outArgs);
}

/**
* @internal wrCpssDxChL2MllMultiTargetPortEnableSet function
* @endinternal
*
* @brief   Enable multi-target port mapping
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
* @note The function is useful when unicast traffic is forwarded over multiple
*       interfaces, for example, in 1+1 protection schemes where traffic is
*       forwarded to a logical unicast target that is represented by the target
*       eport but is physically sent over 2 paths: the working path and the
*       protection path
*
*/
static CMD_STATUS wrCpssDxChL2MllMultiTargetPortEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_U8           devNum;
    GT_BOOL         enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    enable = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChL2MllMultiTargetPortEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChL2MllMultiTargetPortEnableGet function
* @endinternal
*
* @brief   Get the multi-target port mapping enabling status
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function is useful when unicast traffic is forwarded over multiple
*       interfaces, for example, in 1+1 protection schemes where traffic is
*       forwarded to a logical unicast target that is represented by the target
*       eport but is physically sent over 2 paths: the working path and the
*       protection path
*
*/
static CMD_STATUS wrCpssDxChL2MllMultiTargetPortEnableGet
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
    result = cpssDxChL2MllMultiTargetPortEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChL2MllMultiTargetPortSet function
* @endinternal
*
* @brief   Set the multi-target port value and mask
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out of range value or mask
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function is useful when unicast traffic is forwarded over multiple
*       interfaces, for example, in 1+1 protection schemes where traffic is
*       forwarded to a logical unicast target that is represented by the target
*       eport but is physically sent over 2 paths: the working path and the
*       protection path
*
*/
static CMD_STATUS wrCpssDxChL2MllMultiTargetPortSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_U8           devNum;
    GT_U32          value;
    GT_U32          mask;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    value = (GT_U32)inArgs[1];
    mask = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChL2MllMultiTargetPortSet(devNum, value, mask);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChL2MllMultiTargetPortGet function
* @endinternal
*
* @brief   Get the multi-target port value and mask
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function is useful when unicast traffic is forwarded over multiple
*       interfaces, for example, in 1+1 protection schemes where traffic is
*       forwarded to a logical unicast target that is represented by the target
*       eport but is physically sent over 2 paths: the working path and the
*       protection path
*
*/
static CMD_STATUS wrCpssDxChL2MllMultiTargetPortGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_U32      value;
    GT_U32      mask;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChL2MllMultiTargetPortGet(devNum, &value, &mask);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", value, mask);
    return CMD_OK;
}

/**
* @internal wrCpssDxChL2MllMultiTargetPortBaseSet function
* @endinternal
*
* @brief   Set the multi-target port base
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out of range port base
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function is useful when unicast traffic is forwarded over multiple
*       interfaces, for example, in 1+1 protection schemes where traffic is
*       forwarded to a logical unicast target that is represented by the target
*       eport but is physically sent over 2 paths: the working path and the
*       protection path
*
*/
static CMD_STATUS wrCpssDxChL2MllMultiTargetPortBaseSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_U8           devNum;
    GT_U32          portBase;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portBase = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChL2MllMultiTargetPortBaseSet(devNum, portBase);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChL2MllMultiTargetPortBaseGet function
* @endinternal
*
* @brief   Get the multi-target port base
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function is useful when unicast traffic is forwarded over multiple
*       interfaces, for example, in 1+1 protection schemes where traffic is
*       forwarded to a logical unicast target that is represented by the target
*       eport but is physically sent over 2 paths: the working path and the
*       protection path
*
*/
static CMD_STATUS wrCpssDxChL2MllMultiTargetPortBaseGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_U32      portBase;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChL2MllMultiTargetPortBaseGet(devNum, &portBase);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", portBase);
    return CMD_OK;
}

/**
* @internal wrCpssDxChL2MllPortToVidxBaseSet function
* @endinternal
*
* @brief   Set the port to VIDX base
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out of range vidx
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function is useful when unicast traffic is forwarded over multiple
*       interfaces, for example, in 1+1 protection schemes where traffic is
*       forwarded to a logical unicast target that is represented by the target
*       eport but is physically sent over 2 paths: the working path and the
*       protection path
*
*/
static CMD_STATUS wrCpssDxChL2MllPortToVidxBaseSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_U8           devNum;
    GT_U32          vidxBase;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vidxBase = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChL2MllPortToVidxBaseSet(devNum, vidxBase);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChL2MllPortToVidxBaseGet function
* @endinternal
*
* @brief   Get the port to VIDX base
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function is useful when unicast traffic is forwarded over multiple
*       interfaces, for example, in 1+1 protection schemes where traffic is
*       forwarded to a logical unicast target that is represented by the target
*       eport but is physically sent over 2 paths: the working path and the
*       protection path
*
*/
static CMD_STATUS wrCpssDxChL2MllPortToVidxBaseGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_U32      vidxBase;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChL2MllPortToVidxBaseGet(devNum, &vidxBase);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", vidxBase);
    return CMD_OK;
}
/**
* @internal wrCpssDxChL2MllSourceBasedFilteringConfigurationSet function
* @endinternal
*
* @brief   Set configuration for Source Based L2 MLL Filtering.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChL2MllSourceBasedFilteringConfigurationSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_U8           devNum;
    GT_BOOL         enable;
    GT_U32          meshIdOffset;
    GT_U32          meshIdSize;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];
    meshIdOffset = (GT_U32)inArgs[2];
    meshIdSize = (GT_U32)inArgs[3];

    /* call cpss api function */
    result = cpssDxChL2MllSourceBasedFilteringConfigurationSet(devNum, enable, meshIdOffset, meshIdSize);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChL2MllSourceBasedFilteringConfigurationGet function
* @endinternal
*
* @brief   Get configuration for Source Based L2 MLL Filtering.
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
static CMD_STATUS wrCpssDxChL2MllSourceBasedFilteringConfigurationGet
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
    GT_U32                  meshIdOffset;
    GT_U32                  meshIdSize;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChL2MllSourceBasedFilteringConfigurationGet(devNum, &enable, &meshIdOffset, &meshIdSize);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d", enable,meshIdOffset,meshIdSize);
    return CMD_OK;
}

/**
* @internal wrCpssDxChL2MllPairWrite function
* @endinternal
*
* @brief   Write L2 MLL pair entry to hw.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - MLL entry's parameter is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChL2MllPairWrite
(
 IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
 IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
 IN  GT_32 numFields,
 OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   status;
    GT_U8                                       devNum;
    GT_U32                                      mllPairEntryIndex;
    CPSS_DXCH_L2_MLL_PAIR_STC                   mllPairEntry;
    CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT          mllPairWriteForm;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum = (GT_U8)inArgs[0];

    mllPairEntryIndex = (GT_U32)inFields[0];
    mllPairWriteForm = (CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT)inFields[1];
    mllPairEntry.nextPointer = (GT_U16)inFields[2];
    mllPairEntry.entrySelector = (CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_ENT)inFields[3];

    if (mllPairWriteForm == CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E ||
        mllPairWriteForm == CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E)
    {
        mllPairEntry.firstMllNode.unknownUcFilterEnable = (GT_BOOL)inFields[4];
        mllPairEntry.firstMllNode.unregMcFilterEnable = (GT_BOOL)inFields[5];
        mllPairEntry.firstMllNode.bcFilterEnable = (GT_BOOL)inFields[6];
        mllPairEntry.firstMllNode.mcLocalSwitchingEnable = (GT_BOOL)inFields[7];
        mllPairEntry.firstMllNode.maxHopCountEnable = (GT_BOOL)inFields[8];
        mllPairEntry.firstMllNode.maxOutgoingHopCount = (GT_U32)inFields[9];
        mllPairEntry.firstMllNode.egressInterface.type = (CPSS_INTERFACE_TYPE_ENT)inFields[10];
        mllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum = (GT_HW_DEV_NUM)inFields[11];
        mllPairEntry.firstMllNode.egressInterface.devPort.portNum = (GT_PORT_NUM)inFields[12];
        CONVERT_DEV_PORT_DATA_MAC(mllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum,
                                  mllPairEntry.firstMllNode.egressInterface.devPort.portNum);
        mllPairEntry.firstMllNode.egressInterface.trunkId = (GT_TRUNK_ID)inFields[13];
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(mllPairEntry.firstMllNode.egressInterface.trunkId);
        mllPairEntry.firstMllNode.egressInterface.vidx = (GT_U16)inFields[14];
        mllPairEntry.firstMllNode.egressInterface.vlanId = (GT_U16)inFields[15];
        mllPairEntry.firstMllNode.maskBitmap = (GT_U32)inFields[16];
        mllPairEntry.firstMllNode.ttlThreshold = (GT_U32)inFields[17];
        mllPairEntry.firstMllNode.bindToMllCounterEnable = (GT_BOOL)inFields[18];
        mllPairEntry.firstMllNode.mllCounterIndex = (GT_U32)inFields[19];
        mllPairEntry.firstMllNode.onePlusOneFilteringEnable = (GT_BOOL)inFields[20];
        mllPairEntry.firstMllNode.meshId = (GT_U32)inFields[21];
        mllPairEntry.firstMllNode.last = (GT_BOOL)inFields[22];
    }

    if (mllPairWriteForm == CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E ||
        mllPairWriteForm == CPSS_DXCH_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E)
    {
        mllPairEntry.secondMllNode.unknownUcFilterEnable = (GT_BOOL)inFields[23];
        mllPairEntry.secondMllNode.unregMcFilterEnable = (GT_BOOL)inFields[24];
        mllPairEntry.secondMllNode.bcFilterEnable = (GT_BOOL)inFields[25];
        mllPairEntry.secondMllNode.mcLocalSwitchingEnable = (GT_BOOL)inFields[26];
        mllPairEntry.secondMllNode.maxHopCountEnable = (GT_BOOL)inFields[27];
        mllPairEntry.secondMllNode.maxOutgoingHopCount = (GT_U32)inFields[28];
        mllPairEntry.secondMllNode.egressInterface.type = (CPSS_INTERFACE_TYPE_ENT)inFields[29];
        mllPairEntry.secondMllNode.egressInterface.devPort.hwDevNum = (GT_HW_DEV_NUM)inFields[30];
        mllPairEntry.secondMllNode.egressInterface.devPort.portNum = (GT_PORT_NUM)inFields[31];
        CONVERT_DEV_PORT_DATA_MAC(mllPairEntry.secondMllNode.egressInterface.devPort.hwDevNum,
                                  mllPairEntry.secondMllNode.egressInterface.devPort.portNum);
        mllPairEntry.secondMllNode.egressInterface.trunkId = (GT_TRUNK_ID)inFields[32];
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(mllPairEntry.secondMllNode.egressInterface.trunkId);
        mllPairEntry.secondMllNode.egressInterface.vidx = (GT_U16)inFields[33];
        mllPairEntry.secondMllNode.egressInterface.vlanId = (GT_U16)inFields[34];
        mllPairEntry.secondMllNode.maskBitmap = (GT_U32)inFields[35];
        mllPairEntry.secondMllNode.ttlThreshold = (GT_U32)inFields[36];
        mllPairEntry.secondMllNode.bindToMllCounterEnable = (GT_BOOL)inFields[37];
        mllPairEntry.secondMllNode.mllCounterIndex = (GT_U32)inFields[38];
        mllPairEntry.secondMllNode.onePlusOneFilteringEnable = (GT_BOOL)inFields[39];
        mllPairEntry.secondMllNode.meshId = (GT_U32)inFields[40];
        mllPairEntry.secondMllNode.last = (GT_BOOL)inFields[41];
    }

    /* call cpss api function */
    status =  cpssDxChL2MllPairWrite(devNum, mllPairEntryIndex, mllPairWriteForm, &mllPairEntry);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}
/**
* @internal wrCpssDxChL2MllPairReadFirst function
* @endinternal
*
* @brief   Read L2 MLL pair entry to hw.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChL2MllPairReadFirst
(
     IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
     IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
     IN  GT_32 numFields,
     OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;
    GT_U8                                       devNum;
    GT_U32                                      rangeStart;
    GT_U32                                      rangeEnd;
    CPSS_DXCH_L2_MLL_PAIR_STC                   mllPairEntry;
    CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT          mllPairReadForm;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    rangeStart = (GT_U32)inArgs[1];
    rangeEnd = (GT_U32)inArgs[2];
    if((rangeStart!=0)&&(rangeStart==rangeEnd))
    {
        mllPairReadForm = (CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT)inArgs[3];
    }
    else
    {
        mllPairReadForm = CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E;
    }

    gMllPairEntryIndex = rangeStart;

    cmdOsMemSet(&mllPairEntry, 0, sizeof(mllPairEntry));

    result = cpssDxChL2MllPairRead(devNum, gMllPairEntryIndex,
                                   mllPairReadForm, &mllPairEntry);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    inFields[0] = gMllPairEntryIndex;
    inFields[1] = mllPairReadForm;
    inFields[2] = mllPairEntry.nextPointer;
    inFields[3] = mllPairEntry.entrySelector;

    /* First entry of the pair */
    inFields[4] = mllPairEntry.firstMllNode.unknownUcFilterEnable;
    inFields[5] = mllPairEntry.firstMllNode.unregMcFilterEnable;
    inFields[6] = mllPairEntry.firstMllNode.bcFilterEnable;
    inFields[7] = mllPairEntry.firstMllNode.mcLocalSwitchingEnable;
    inFields[8] = mllPairEntry.firstMllNode.maxHopCountEnable;
    inFields[9] = mllPairEntry.firstMllNode.maxOutgoingHopCount;
    inFields[10] = mllPairEntry.firstMllNode.egressInterface.type;
    CONVERT_BACK_DEV_PORT_DATA_MAC(mllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum,
        mllPairEntry.firstMllNode.egressInterface.devPort.portNum);
    inFields[11] = mllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum;
    inFields[12] = mllPairEntry.firstMllNode.egressInterface.devPort.portNum;
    CONVERT_TRUNK_ID_CPSS_TO_TEST_MAC(mllPairEntry.firstMllNode.egressInterface.trunkId);
    inFields[13] = mllPairEntry.firstMllNode.egressInterface.trunkId;
    inFields[14] = mllPairEntry.firstMllNode.egressInterface.vidx;
    inFields[15] = mllPairEntry.firstMllNode.egressInterface.vlanId;
    inFields[16] = mllPairEntry.firstMllNode.maskBitmap;
    inFields[17] = mllPairEntry.firstMllNode.ttlThreshold;
    inFields[18] = mllPairEntry.firstMllNode.bindToMllCounterEnable;
    inFields[19] = mllPairEntry.firstMllNode.mllCounterIndex;
    inFields[20] = mllPairEntry.firstMllNode.onePlusOneFilteringEnable;
    inFields[21] = mllPairEntry.firstMllNode.meshId;
    inFields[22] = mllPairEntry.firstMllNode.last;

    /* Second entry of the pair */
    inFields[23] = mllPairEntry.secondMllNode.unknownUcFilterEnable;
    inFields[24] = mllPairEntry.secondMllNode.unregMcFilterEnable;
    inFields[25] = mllPairEntry.secondMllNode.bcFilterEnable;
    inFields[26] = mllPairEntry.secondMllNode.mcLocalSwitchingEnable;
    inFields[27] = mllPairEntry.secondMllNode.maxHopCountEnable;
    inFields[28] = mllPairEntry.secondMllNode.maxOutgoingHopCount;
    inFields[29] = mllPairEntry.secondMllNode.egressInterface.type;
    CONVERT_BACK_DEV_PORT_DATA_MAC(mllPairEntry.secondMllNode.egressInterface.devPort.hwDevNum,
        mllPairEntry.secondMllNode.egressInterface.devPort.portNum);
    inFields[30] = mllPairEntry.secondMllNode.egressInterface.devPort.hwDevNum;
    inFields[31] = mllPairEntry.secondMllNode.egressInterface.devPort.portNum;
    CONVERT_TRUNK_ID_CPSS_TO_TEST_MAC(mllPairEntry.secondMllNode.egressInterface.trunkId);
    inFields[32] = mllPairEntry.secondMllNode.egressInterface.trunkId;
    inFields[33] = mllPairEntry.secondMllNode.egressInterface.vidx;
    inFields[34] = mllPairEntry.secondMllNode.egressInterface.vlanId;
    inFields[35] = mllPairEntry.secondMllNode.maskBitmap;
    inFields[36] = mllPairEntry.secondMllNode.ttlThreshold;
    inFields[37] = mllPairEntry.secondMllNode.bindToMllCounterEnable;
    inFields[38] = mllPairEntry.secondMllNode.mllCounterIndex;
    inFields[39] = mllPairEntry.secondMllNode.onePlusOneFilteringEnable;
    inFields[40] = mllPairEntry.secondMllNode.meshId;
    inFields[41] = mllPairEntry.secondMllNode.last;

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
        inFields[0], inFields[1], inFields[2], inFields[3],
        inFields[4], inFields[5], inFields[6], inFields[7],
        inFields[8], inFields[9], inFields[10], inFields[11],
        inFields[12], inFields[13], inFields[14], inFields[15],
        inFields[16], inFields[17], inFields[18], inFields[19],
        inFields[20], inFields[21], inFields[22], inFields[23],
        inFields[24], inFields[25], inFields[26], inFields[27],
        inFields[28], inFields[29], inFields[30], inFields[31],
        inFields[32], inFields[33], inFields[34], inFields[35],
        inFields[36], inFields[37], inFields[38], inFields[39],
        inFields[40], inFields[41]);

    galtisOutput(outArgs, result, "%f");

    gMllPairEntryIndex++;

    return CMD_OK;
}

/**
* @internal wrCpssDxChL2MllPairReadNext function
* @endinternal
*
* @brief   Read L2 MLL pair entry to hw.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChL2MllPairReadNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;
    GT_U8                                       devNum;
    GT_U32                                      rangeStart;
    GT_U32                                      rangeEnd;
    CPSS_DXCH_L2_MLL_PAIR_STC                   mllPairEntry;
    CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT          mllPairReadForm;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    rangeStart = (GT_U32)inArgs[1];
    rangeEnd = (GT_U32)inArgs[2];

    /*if the user doesn't enter a range show the whole range*/
    if ((rangeStart == 0) && (rangeEnd == 0))
    {
        rangeEnd = MAX_MLL_ENTRIES - 1;
    }

    if (gMllPairEntryIndex > rangeEnd)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    if (gMllPairEntryIndex == rangeEnd)
    {
        mllPairReadForm = (CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT)inArgs[3];
    }
    else
    {
        mllPairReadForm = CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E;
    }

    /* call cpss api function */
    cmdOsMemSet(&mllPairEntry, 0, sizeof(mllPairEntry));

    /* call cpss api function */
    result = cpssDxChL2MllPairRead(devNum, gMllPairEntryIndex,
                                   mllPairReadForm, &mllPairEntry);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    inFields[0] = gMllPairEntryIndex;
    inFields[1] = mllPairReadForm;
    inFields[2] = mllPairEntry.nextPointer;
    inFields[3] = mllPairEntry.entrySelector;

    /* First entry of the pair */
    inFields[4] = mllPairEntry.firstMllNode.unknownUcFilterEnable;
    inFields[5] = mllPairEntry.firstMllNode.unregMcFilterEnable;
    inFields[6] = mllPairEntry.firstMllNode.bcFilterEnable;
    inFields[7] = mllPairEntry.firstMllNode.mcLocalSwitchingEnable;
    inFields[8] = mllPairEntry.firstMllNode.maxHopCountEnable;
    inFields[9] = mllPairEntry.firstMllNode.maxOutgoingHopCount;
    inFields[10] = mllPairEntry.firstMllNode.egressInterface.type;
    CONVERT_BACK_DEV_PORT_DATA_MAC(mllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum,
        mllPairEntry.firstMllNode.egressInterface.devPort.portNum);
    inFields[11] = mllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum;
    inFields[12] = mllPairEntry.firstMllNode.egressInterface.devPort.portNum;
    CONVERT_TRUNK_ID_CPSS_TO_TEST_MAC(mllPairEntry.firstMllNode.egressInterface.trunkId);
    inFields[13] = mllPairEntry.firstMllNode.egressInterface.trunkId;
    inFields[14] = mllPairEntry.firstMllNode.egressInterface.vidx;
    inFields[15] = mllPairEntry.firstMllNode.egressInterface.vlanId;
    inFields[16] = mllPairEntry.firstMllNode.maskBitmap;
    inFields[17] = mllPairEntry.firstMllNode.ttlThreshold;
    inFields[18] = mllPairEntry.firstMllNode.bindToMllCounterEnable;
    inFields[19] = mllPairEntry.firstMllNode.mllCounterIndex;
    inFields[20] = mllPairEntry.firstMllNode.onePlusOneFilteringEnable;
    inFields[21] = mllPairEntry.firstMllNode.meshId;
    inFields[22] = mllPairEntry.firstMllNode.last;

    /* Second entry of the pair */
    inFields[23] = mllPairEntry.secondMllNode.unknownUcFilterEnable;
    inFields[24] = mllPairEntry.secondMllNode.unregMcFilterEnable;
    inFields[25] = mllPairEntry.secondMllNode.bcFilterEnable;
    inFields[26] = mllPairEntry.secondMllNode.mcLocalSwitchingEnable;
    inFields[27] = mllPairEntry.secondMllNode.maxHopCountEnable;
    inFields[28] = mllPairEntry.secondMllNode.maxOutgoingHopCount;
    inFields[29] = mllPairEntry.secondMllNode.egressInterface.type;
    CONVERT_BACK_DEV_PORT_DATA_MAC(mllPairEntry.secondMllNode.egressInterface.devPort.hwDevNum,
        mllPairEntry.secondMllNode.egressInterface.devPort.portNum);
    inFields[30] = mllPairEntry.secondMllNode.egressInterface.devPort.hwDevNum;
    inFields[31] = mllPairEntry.secondMllNode.egressInterface.devPort.portNum;
    CONVERT_TRUNK_ID_CPSS_TO_TEST_MAC(mllPairEntry.secondMllNode.egressInterface.trunkId);
    inFields[32] = mllPairEntry.secondMllNode.egressInterface.trunkId;
    inFields[33] = mllPairEntry.secondMllNode.egressInterface.vidx;
    inFields[34] = mllPairEntry.secondMllNode.egressInterface.vlanId;
    inFields[35] = mllPairEntry.secondMllNode.maskBitmap;
    inFields[36] = mllPairEntry.secondMllNode.ttlThreshold;
    inFields[37] = mllPairEntry.secondMllNode.bindToMllCounterEnable;
    inFields[38] = mllPairEntry.secondMllNode.mllCounterIndex;
    inFields[39] = mllPairEntry.secondMllNode.onePlusOneFilteringEnable;
    inFields[40] = mllPairEntry.secondMllNode.meshId;
    inFields[41] = mllPairEntry.secondMllNode.last;

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
        inFields[0], inFields[1], inFields[2], inFields[3],
        inFields[4], inFields[5], inFields[6], inFields[7],
        inFields[8], inFields[9], inFields[10], inFields[11],
        inFields[12], inFields[13], inFields[14], inFields[15],
        inFields[16], inFields[17], inFields[18], inFields[19],
        inFields[20], inFields[21], inFields[22], inFields[23],
        inFields[24], inFields[25], inFields[26], inFields[27],
        inFields[28], inFields[29], inFields[30], inFields[31],
        inFields[32], inFields[33], inFields[34], inFields[35],
        inFields[36], inFields[37], inFields[38], inFields[39],
        inFields[40], inFields[41]);

    galtisOutput(outArgs, result, "%f");

    gMllPairEntryIndex++;

    return CMD_OK;
}

/**
* @internal wrCpssDxChL2MllVirtualPortToMllMappingTableEntrySet function
* @endinternal
*
* @brief   Set virtual port mapping table entry. This entry maps
*         virtual target port to L2MLL entry.
*
* @note   APPLICABLE DEVICES:      xCat.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_HW_ERROR              - on Hardware error.
*
* @note xCat device: applicable starting from revision C0
*       If the target supports APS 1+1, there are 2 L2MLL entries in this list, one
*       with a target logical port for the working path and one with a target
*       logical port for the protection path. It is required that these 2 target
*       logical ports be consecutive and differ only in the least significant bit of
*       the port number.
*
*/
static CMD_STATUS wrCpssDxChL2MllVirtualPortToMllMappingTableEntrySet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U8     targetDevNum;
    GT_U8     targetPortNum;
    GT_U32    mllPointer;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    targetDevNum = (GT_U8)inArgs[1];
    targetPortNum = (GT_U8)inArgs[2];
    mllPointer = (GT_U32)inArgs[3];

    /* call cpss api function */
    result = cpssDxChL2MLLVirtualPortToMllMappingTableEntrySet(devNum, targetDevNum, targetPortNum, mllPointer);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChL2MllVirtualPortToMllMappingTableEntryGet function
* @endinternal
*
* @brief   Get virtual port mapping table entry. This entry maps
*         virtual target port to L2MLL entry.
*
* @note   APPLICABLE DEVICES:      xCat.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_HW_ERROR              - on Hardware error.
*
* @note xCat device: applicable starting from revision C0
*
*/
static CMD_STATUS wrCpssDxChL2MllVirtualPortToMllMappingTableEntryGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U8     targetDevNum;
    GT_U8     targetPortNum;
    GT_U32    mllPointer;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    targetDevNum = (GT_U8)inArgs[1];
    targetPortNum = (GT_U8)inArgs[2];

    /* call cpss api function */
    result = cpssDxChL2MLLVirtualPortToMllMappingTableEntryGet(devNum, targetDevNum, targetPortNum, &mllPointer);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mllPointer);

    return CMD_OK;
}

/**
* @internal wrCpssDxChL2MllVidxEnableSet function
* @endinternal
*
* @brief   Enable/disable MLL lookup for given vidx on the specified device.
*         When enabled the VIDX value is used as the L2MLL index.
*
* @note   APPLICABLE DEVICES:      xCat.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note xCat device: applicable starting from revision C0
*
*/
static CMD_STATUS wrCpssDxChL2MllVidxEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U16  vidx;
    GT_BOOL enable;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    vidx   = (GT_U16)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* call cpss api function */
    result = cpssDxChL2MllVidxEnableSet(devNum, vidx, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChL2MllVidxEnableGet function
* @endinternal
*
* @brief   Get enabling status of MLL lookup for given vidx on the specified device.
*         When enabled the VIDX value is used as the L2MLL index.
*
* @note   APPLICABLE DEVICES:      xCat.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note xCat device: applicable starting from revision C0
*
*/
static CMD_STATUS wrCpssDxChL2MllVidxEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U16    vidx;
    GT_BOOL   enablePtr;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum    = (GT_U8)inArgs[0];
    vidx      = (GT_U16)inArgs[1];

    /* call cpss api function */
    result = cpssDxChL2MllVidxEnableGet(devNum, vidx, &enablePtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enablePtr);

    return CMD_OK;
}

/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{

    {"cpssDxChL2MllLookupForAllEvidxEnableSet",
        &wrCpssDxChL2MllLookupForAllEvidxEnableSet,
        2, 0},
    {"cpssDxChL2MllLookupForAllEvidxEnableGet",
        &wrCpssDxChL2MllLookupForAllEvidxEnableGet,
        1, 0},
    {"cpssDxChL2MllLookupMaxVidxIndexSet",
        &wrCpssDxChL2MllLookupMaxVidxIndexSet,
        2, 0},
    {"cpssDxChL2MllLookupMaxVidxIndexGet",
        &wrCpssDxChL2MllLookupMaxVidxIndexGet,
        1, 0},
    {"cpssDxChL2MllTtlExceptionConfigurationSet",
        &wrCpssDxChL2MllTtlExceptionConfigurationSet,
        3, 0},
    {"cpssDxChL2MllTtlExceptionConfigurationGet",
        &wrCpssDxChL2MllTtlExceptionConfigurationGet,
        1, 0},
    {"cpssDxChL2MllExceptionCountersGet",
        &wrCpssDxChL2MllExceptionCountersGet,
        1, 0},
    {"cpssDxChL2MllCounterGet",
        &wrCpssDxChL2MllCounterGet,
        2, 0},
    {"cpssDxChL2MllCountersGet",
        &wrCpssDxChL2MllCountersGet,
        1, 0},
    {"cpssDxChL2MllCntInterfaceCfgSet",
        &wrCpssDxChL2MllSetCntInterfaceCfg,
        9, 0},
    {"cpssDxChL2MllCntInterfaceCfgGet",
        &wrCpssDxChL2MllGetCntInterfaceCfg,
        2, 0},
    {"cpssDxChL2MllMcCntGet",
        &wrCpssDxChL2MllMcCntGet,
        2, 0},
    {"cpssDxChL2MllSilentDropCntGet",
        &wrCpssDxChL2MllSilentDropCntGet,
        1, 0},

    {"cpssDxChL2MllLttEntrySet",
        &wrCpssDxChL2MllLttEntrySet,
        3,4},
    {"cpssDxChL2MllLttEntryGetFirst",
        &wrCpssDxChL2MllLttEntryGetFirst,
        3,0},
    {"cpssDxChL2MllLttEntryGetNext",
        &wrCpssDxChL2MllLttEntryGet,
        3,0},
    {"cpssDxChL2MllLttEntry_1Set",
        &wrCpssDxChL2MllLttEntry_1Set,
        3,5},
    {"cpssDxChL2MllLttEntry_1GetFirst",
        &wrCpssDxChL2MllLttEntry_1GetFirst,
        3,0},
    {"cpssDxChL2MllLttEntry_1GetNext",
        &wrCpssDxChL2MllLttEntry_1Get,
        3,0},
    {"cpssDxChL2MllMultiTargetPortEnableSet",
        &wrCpssDxChL2MllMultiTargetPortEnableSet,
        2, 0},
    {"cpssDxChL2MllMultiTargetPortEnableGet",
        &wrCpssDxChL2MllMultiTargetPortEnableGet,
        1, 0},
    {"cpssDxChL2MllMultiTargetPortSet",
        &wrCpssDxChL2MllMultiTargetPortSet,
        3, 0},
    {"cpssDxChL2MllMultiTargetPortGet",
        &wrCpssDxChL2MllMultiTargetPortGet,
        1, 0},
    {"cpssDxChL2MllMultiTargetPortBaseSet",
        &wrCpssDxChL2MllMultiTargetPortBaseSet,
        2, 0},
    {"cpssDxChL2MllMultiTargetPortBaseGet",
        &wrCpssDxChL2MllMultiTargetPortBaseGet,
        1, 0},
    {"cpssDxChL2MllPortToVidxBaseSet",
        &wrCpssDxChL2MllPortToVidxBaseSet,
        2, 0},
    {"cpssDxChL2MllPortToVidxBaseGet",
        &wrCpssDxChL2MllPortToVidxBaseGet,
        1, 0},
    {"cpssDxChL2MllSourceBasedFilteringConfigurationSet",
        &wrCpssDxChL2MllSourceBasedFilteringConfigurationSet,
        4, 0},
    {"cpssDxChL2MllSourceBasedFilteringConfigurationGet",
        &wrCpssDxChL2MllSourceBasedFilteringConfigurationGet,
        1, 0},
    {"cpssDxChL2MllPairSet",
        &wrCpssDxChL2MllPairWrite,
        4, 42},
    {"cpssDxChL2MllPairGetFirst",
        &wrCpssDxChL2MllPairReadFirst,
        4, 0},
    {"cpssDxChL2MllPairGetNext",
        &wrCpssDxChL2MllPairReadNext,
        4, 0},
    {"cpssDxChL2MLLVirtualPortToMllMappingTableEntrySet",
        &wrCpssDxChL2MllVirtualPortToMllMappingTableEntrySet,
        4, 0},
    {"cpssDxChL2MLLVirtualPortToMllMappingTableEntryGet",
        &wrCpssDxChL2MllVirtualPortToMllMappingTableEntryGet,
        3, 0},
    {"cpssDxChL2MllVidxEnableSet",
        &wrCpssDxChL2MllVidxEnableSet,
        3, 0},
    {"cpssDxChL2MllVidxEnableGet",
        &wrCpssDxChL2MllVidxEnableGet,
        2, 0}
};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssDxChL2Mll function
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
GT_STATUS cmdLibInitCpssDxChL2Mll
(
GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}

