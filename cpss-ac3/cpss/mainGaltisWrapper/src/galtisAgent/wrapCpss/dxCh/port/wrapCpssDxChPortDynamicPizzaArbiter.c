/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* wrapCpssDxChDiagPortSlicesDirectCtrl.c
*
* DESCRIPTION:
*       Wrapper functions for CPSS DXCH Port AP API
*
* FILE REVISION NUMBER:
*       $Revision: 3 $
*
*******************************************************************************/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>


/* Feature specific includes. */
#include <cpss/dxCh/dxChxGen/port/SliceManipulation/cpssDxChPortSlicesDirectCtrlDbg.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>    
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h> 
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/cpssDxChPortDynamicPAUnitBW.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/cpssDxChPortPizzaArbiter.h>

extern GT_STATUS gtBobcat2PortPizzaArbiterIfStateDump
(
    IN  GT_U8  devNum,
    IN  GT_U32 portGroupId
);

extern GT_STATUS gtBobcat2PortMappingDump
(
    IN  GT_U8  dev
);


extern GT_STATUS gtBobcat2TxQHighSpeedPortPrint
(
    IN  GT_U8  dev
);

extern GT_STATUS gtBobcat2TxQHighSpeed_DQ_LL_Dump
(
    IN  GT_U8  dev
);



static CMD_STATUS wrGtBobcat2PortPizzaArbiterIfStateDump
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U32    portGroupNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_U8) inArgs[0];
    portGroupNum    = (GT_U32)inArgs[1];

    result = gtBobcat2PortPizzaArbiterIfStateDump(devNum, portGroupNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result,"");
    return CMD_OK;
}

static CMD_STATUS wrGtBobcat2PortMappingDump
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_U8) inArgs[0];

    result = gtBobcat2PortMappingDump(devNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result,"");
    return CMD_OK;
}


static CMD_STATUS wrGtBobcat2TxQHighSpeedPortDump
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_U8) inArgs[0];

    result = gtBobcat2TxQHighSpeedPortPrint(devNum);
    result = gtBobcat2TxQHighSpeed_DQ_LL_Dump(devNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result,"");
    return CMD_OK;
}


static CMD_STATUS wrCpssDxChPipeBWMinPortSpeedResolutionSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U32                   pipeBandwithInGbps;
    CPSS_DXCH_MIN_SPEED_ENT  minimalPortSpeedResolutionInMBps;
    /* map input arguments to locals */
    devNum                           = (GT_U8) inArgs[0];
    pipeBandwithInGbps               = (GT_U32)inArgs[1];
    minimalPortSpeedResolutionInMBps = (CPSS_DXCH_MIN_SPEED_ENT)inArgs[2];

    result = cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionSet(devNum,pipeBandwithInGbps,minimalPortSpeedResolutionInMBps);
    galtisOutput(outArgs, result,"");
    return CMD_OK;
}

static CMD_STATUS wrCpssDxChPipeBWMinPortSpeedResolutionGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U32                   pipeBandwithInGbps;
    CPSS_DXCH_MIN_SPEED_ENT  minimalPortSpeedResolutionInMBps;
    GT_U32 i;

    typedef struct SpeedResStr
    {
        CPSS_DXCH_MIN_SPEED_ENT  resEnm;
        GT_CHAR                 *resStr;
    }SpeedResStr;

    SpeedResStr SpeedResStrArr[] = 
    {
	     { CPSS_DXCH_MIN_SPEED_500_Mbps_E  ,  "500 Mbps" }
	    ,{ CPSS_DXCH_MIN_SPEED_1000_Mbps_E ,  "1000 Mbps" }
	    ,{ CPSS_DXCH_MIN_SPEED_2000_Mbps_E ,  "2000 Mbps" }
	    ,{ CPSS_DXCH_MIN_SPEED_5000_Mbps_E ,  "5000 Mbps" }
        ,{ CPSS_DXCH_MIN_SPEED_10000_Mbps_E,  "10000 Mbps" }
    };
    GT_CHAR *str;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_U8) inArgs[0];

    result = cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionGet(devNum,&pipeBandwithInGbps,&minimalPortSpeedResolutionInMBps);
    if (result == GT_OK)
    {
        str = (GT_CHAR *)NULL;
        for (i = 0 ; i < sizeof(SpeedResStrArr)/sizeof(SpeedResStrArr[0]); i++)
        {
            if (SpeedResStrArr[i].resEnm == minimalPortSpeedResolutionInMBps)
            {
                str = SpeedResStrArr[i].resStr;
                break;
            }
        }
        if (str == NULL)
        {
            str = "Unknown";
        }

        cpssOsPrintf("\n PA pipe BW = %d Gbps",pipeBandwithInGbps); 
        cpssOsPrintf("\n Speed Res  = %s",str);                     
    }
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result,"");
    return CMD_OK;

}

/**
* @internal wrCpssDxChPortPizzaArbiterIfConfigSet function
* @endinternal
*
* @brief   Configure Pizza Arbiter accoringly to new port's i/f mode and speed
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
static CMD_STATUS wrCpssDxChPortPizzaArbiterIfConfigSet
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
    CPSS_PORT_SPEED_ENT portSpeed;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    portSpeed = (CPSS_PORT_SPEED_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPortPizzaArbiterIfConfigSet(devNum, portNum, portSpeed);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPortResourcesConfigSet function
* @endinternal
*
* @brief   Caelum port resource configuration
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device, ifMode, speed
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortResourcesConfigSet
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
    CPSS_PORT_INTERFACE_MODE_ENT ifMode;
    CPSS_PORT_SPEED_ENT portSpeed;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    ifMode = (CPSS_PORT_INTERFACE_MODE_ENT)inArgs[2];
    portSpeed = (CPSS_PORT_SPEED_ENT)inArgs[3];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPortResourcesConfigSet(devNum, portNum, ifMode, portSpeed);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}




/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
      /* commandName;                             funcReference;                           funcArgs;  funcFields */
      {"gtBobcat2PortPizzaArbiterIfStateDump",    wrGtBobcat2PortPizzaArbiterIfStateDump,     2,             0}
     ,{"gtBobcat2PortMappingDump",                wrGtBobcat2PortMappingDump,                 1,             0}
     ,{"gtBobcat2TxQHighSpeedPortDump",           wrGtBobcat2TxQHighSpeedPortDump,            1,             0}
     ,{"cpssDxChPipeBWMinPortSpeedResolutionSet", wrCpssDxChPipeBWMinPortSpeedResolutionSet,  3,             0}
     ,{"cpssDxChPipeBWMinPortSpeedResolutionGet", wrCpssDxChPipeBWMinPortSpeedResolutionGet,  1,             0}
     ,{"cpssDxChPortPizzaArbiterIfConfigSet",     wrCpssDxChPortPizzaArbiterIfConfigSet,      3,             0}
     ,{"cpssDxChPortResourcesConfigSet",          wrCpssDxChPortResourcesConfigSet,           4,             0}
};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssDxChPortDynamicPizzaArbiter function
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
GT_STATUS cmdLibInitCpssDxChPortDynamicPizzaArbiter
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}


