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
* @file wrHwsMac.cpp
*
* @brief Implementation of the Galtis Wrapper functions
*
* @version   1
********************************************************************************
*/

/* General H Files */
#include <common/os/gtOs.h>
#include <common/system/system.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdCommon.h>
#include <cmdShell/common/cmdWrapUtils.h>
#include <common/macros/gtCommonFuncs.h>
#include <galtisAgent/wrapUtil/galtisValues.h>
#include <galtisAgent/wrapUtil/cmdUtils.h>
#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>
#include <cpss/common/labServices/port/gop/port/mac/geMac/mvHwsGeMacDb.h>
#include <cpss/common/labServices/port/gop/port/mac/hglMac/mvHwsHglMacDb.h>
#include <cpss/common/labServices/port/gop/port/mac/xgMac/mvHwsXgMacDb.h>
#include <cpss/common/labServices/port/gop/port/mac/xlgMac/mvHwsXlgMacDb.h>
#include <cpss/common/labServices/port/gop/port/mac/cgMac/mvHwsCgMacDb.h>
#include <cpss/common/labServices/port/gop/port/mac/ilknMac/mvHwsIlknMacDb.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>

extern "C" {
GT_STATUS cmdLibInitHwsMacIf
(
    GT_VOID
);
}

/**
* @internal wrHwsMacModeCfgSetEntry function
* @endinternal
*
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
CMD_STATUS wrHwsMacModeCfgSetEntry
(
    IN  GT_32 inArgs[CMD_MAX_ARGS],
    IN  GT_32 inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum = inArgs[0];
    GT_U32      macNum = inFields[0];
    MV_HWS_PORT_MAC_TYPE    macType;

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }
    switch(inFields[1])
    {
    case 0:
      macType = GEMAC_X;
      break;
    case 1:
      macType = GEMAC_SG;
      break;
    case 2:
      macType = XLGMAC;
      break;
    case 3:
      macType = XGMAC;
      break;
    case 4:
      macType = HGLMAC;
      break;
    case 5:
      macType = CGMAC;
      break;
    case 6:
      macType = INTLKN_MAC;
      break;
    case 7:
      macType = TCAM_MAC;
      break;
    default:
        galtisOutput(outArgs, CMD_SYNTAX_ERROR, "");
        return CMD_OK;
    }

    rc = mvHwsMacModeCfg(devNum, devNum, macNum, macType, (GT_U32)inFields[2]);

    galtisOutput(outArgs, rc, "");
    return CMD_OK;
}

/**
* @internal wrHwsMacResetCfgSetEntry function
* @endinternal
*
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
CMD_STATUS wrHwsMacResetCfgSetEntry
(
    IN  GT_32 inArgs[CMD_MAX_ARGS],
    IN  GT_32 inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum = inArgs[0];
    GT_U32      macNum = inFields[0];
    MV_HWS_PORT_MAC_TYPE    macType;
    MV_HWS_RESET  action;

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    switch(inFields[1])
    {
    case 0:
      macType = GEMAC_X;
      break;
    case 1:
      macType = GEMAC_SG;
      break;
    case 2:
      macType = XLGMAC;
      break;
    case 3:
      macType = XGMAC;
      break;
    case 4:
      macType = HGLMAC;
      break;
    case 5:
      macType = CGMAC;
      break;
    case 6:
      macType = INTLKN_MAC;
      break;
    case 7:
      macType = TCAM_MAC;
      break;
    default:
        galtisOutput(outArgs, CMD_SYNTAX_ERROR, "");
        return CMD_OK;
    }

    if (inFields[2] == 0)
    {
      action = UNRESET;
    }
    else
    {
      action = RESET;
    }
    rc = mvHwsMacReset(devNum, devNum, macNum, macType, action);

    galtisOutput(outArgs, rc, "");
    return CMD_OK;
}

static GT_U32 seqCurLine;
/**
* @internal wrHwsMacSeqCfgGetFirst function
* @endinternal
*
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
CMD_STATUS wrHwsMacSeqCfgGetFirst
(
    IN  GT_32 inArgs[CMD_MAX_ARGS],
    IN  GT_32 inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    MV_POLLING_OP_PARAMS *polParam;
    MV_WRITE_OP_PARAMS   *wrParam;
    MV_DELAY_OP_PARAMS   *delOp;
    MV_CFG_ELEMENT cfgEl;
    GT_U8           devNum;
    GT_U32          baseAddr, unitIndex;

    seqCurLine = 0;
    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }
    devNum = inArgs[0];
    switch(inArgs[1])
    {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
      /* GE MAC */
      rc = hwsGeMacSeqGet((MV_HWS_GE_MAC_SUB_SEQ)inArgs[1], &cfgEl, 0);
      break;
    case 7:
      /* HGL MAC */
      rc = hwsHglMacSeqGet((MV_HWS_HGL_MAC_SUB_SEQ)(inArgs[1]-7), &cfgEl, 0);
      break;
    case 8:
      /* XG MAC */
      rc = hwsXgMacSeqGet((MV_HWS_XG_MAC_SUB_SEQ)(inArgs[1]-8), &cfgEl, 0);
      break;
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
      /* XLG MAC */
      rc = hwsXlgMacSeqGet((MV_HWS_XLG_MAC_SUB_SEQ)(inArgs[1]-9), &cfgEl, 0);
      break;
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
      /* CG MAC */
      rc = hwsCgMacSeqGet((MV_HWS_CG_MAC_SUB_SEQ)(inArgs[1]-17), &cfgEl, 0);
      break;
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
      /* INLK MAC */
      rc = hwsIlknMacSeqGet((MV_HWS_INTKLN_MAC_SUB_SEQ)(inArgs[1]-22), &cfgEl, 0);
      break;
    default:
        galtisOutput(outArgs, GT_BAD_PARAM, "");
        return CMD_OK;
    }

    inFields[0] = 0;
    inFields[1] = cfgEl.op;
    switch(inFields[1])
    {
    case WRITE_OP:
      wrParam = (MV_WRITE_OP_PARAMS *)cfgEl.params;
      mvUnitInfoGet(devNum, (MV_HWS_UNITS_ID)wrParam->indexOffset, &baseAddr, &unitIndex);
      inFields[2] = baseAddr;
      inFields[3] = wrParam->regOffset;
      inFields[4] = wrParam->data;
      inFields[5] = wrParam->mask;
      inFields[6] = 0;
      inFields[7] = 0;
      break;
    case DELAY_OP:
      delOp = (MV_DELAY_OP_PARAMS *)cfgEl.params;
      inFields[2] = 0;
      inFields[3] = 0;
      inFields[4] = 0;
      inFields[5] = 0;
      inFields[6] = delOp->delay;
      inFields[7] = 0;
      break;
    case POLLING_OP:
      polParam = (MV_POLLING_OP_PARAMS *)cfgEl.params;
      mvUnitInfoGet(devNum, (MV_HWS_UNITS_ID)polParam->indexOffset, &baseAddr, &unitIndex);
      inFields[2] = baseAddr;
      inFields[3] = polParam->regOffset;
      inFields[4] = polParam->data;
      inFields[5] = polParam->mask;
      inFields[6] = polParam->waitTime;
      inFields[7] = polParam->numOfLoops;
      break;
    }

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d",inFields[0],inFields[1],inFields[2],inFields[3],inFields[4],inFields[5],inFields[6],inFields[7]);
    galtisOutput(outArgs, GT_OK, "%f");
    return CMD_OK;
}

/**
* @internal wrHwsMacSeqCfgGetNext function
* @endinternal
*
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
CMD_STATUS wrHwsMacSeqCfgGetNext
(
    IN  GT_32 inArgs[CMD_MAX_ARGS],
    IN  GT_32 inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    MV_POLLING_OP_PARAMS *polParam;
    MV_WRITE_OP_PARAMS   *wrParam;
    MV_DELAY_OP_PARAMS   *delOp;
    MV_CFG_ELEMENT cfgEl;
    GT_U8           devNum;
    GT_U32          baseAddr, unitIndex;

    inFields[0] = ++seqCurLine;
    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }
    switch(inArgs[1])
    {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
      /* GE MAC */
      rc = hwsGeMacSeqGet((MV_HWS_GE_MAC_SUB_SEQ)inArgs[1], &cfgEl, seqCurLine);
      break;
    case 7:
      /* HGL MAC */
      rc = hwsHglMacSeqGet((MV_HWS_HGL_MAC_SUB_SEQ)(inArgs[1]-7), &cfgEl, seqCurLine);
      break;
    case 8:
      /* XG MAC */
      rc = hwsXgMacSeqGet((MV_HWS_XG_MAC_SUB_SEQ)(inArgs[1]-8), &cfgEl, seqCurLine);
      break;
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
      /* XLG MAC */
      rc = hwsXlgMacSeqGet((MV_HWS_XLG_MAC_SUB_SEQ)(inArgs[1]-9), &cfgEl, seqCurLine);
      break;
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
      /* CG MAC */
      rc = hwsCgMacSeqGet((MV_HWS_CG_MAC_SUB_SEQ)(inArgs[1]-17), &cfgEl, seqCurLine);
      break;
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
      /* INLK MAC */
      rc = hwsIlknMacSeqGet((MV_HWS_INTKLN_MAC_SUB_SEQ)(inArgs[1]-22), &cfgEl, seqCurLine);
      break;
    default:
        galtisOutput(outArgs, GT_BAD_PARAM, "");
        return CMD_OK;
    }

    if (rc == GT_NO_MORE)
    {
      galtisOutput(outArgs, GT_OK, "%d", -1);
      return CMD_OK;
    }
    devNum = inArgs[0];
    inFields[1] = cfgEl.op;
    switch(inFields[1])
    {
    case WRITE_OP:
      wrParam = (MV_WRITE_OP_PARAMS *)cfgEl.params;
      mvUnitInfoGet(devNum, (MV_HWS_UNITS_ID)wrParam->indexOffset, &baseAddr, &unitIndex);
      inFields[2] = baseAddr;
      inFields[3] = wrParam->regOffset;
      inFields[4] = wrParam->data;
      inFields[5] = wrParam->mask;
      inFields[6] = 0;
      inFields[7] = 0;
      break;
    case DELAY_OP:
      delOp = (MV_DELAY_OP_PARAMS *)cfgEl.params;
      inFields[2] = 0;
      inFields[3] = 0;
      inFields[4] = 0;
      inFields[5] = 0;
      inFields[6] = delOp->delay;
      inFields[7] = 0;
      break;
    case POLLING_OP:
      polParam = (MV_POLLING_OP_PARAMS *)cfgEl.params;
      mvUnitInfoGet(devNum, (MV_HWS_UNITS_ID)polParam->indexOffset, &baseAddr, &unitIndex);
      inFields[2] = baseAddr;
      inFields[3] = polParam->regOffset;
      inFields[4] = polParam->data;
      inFields[5] = polParam->mask;
      inFields[6] = polParam->waitTime;
      inFields[7] = polParam->numOfLoops;
      break;
    }

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d",inFields[0],inFields[1],inFields[2],inFields[3],inFields[4],inFields[5],inFields[6],inFields[7]);
    galtisOutput(outArgs, GT_OK, "%f");
    return CMD_OK;

}

/**
* @internal wrHwsMacSeqCfgSet function
* @endinternal
*
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
CMD_STATUS wrHwsMacSeqCfgSet
(
    IN  GT_32 inArgs[CMD_MAX_ARGS],
    IN  GT_32 inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    MV_POLLING_OP_PARAMS polParam;
    MV_WRITE_OP_PARAMS   wrParam;
    MV_DELAY_OP_PARAMS   delOp;
    MV_CFG_ELEMENT       cfgEl;
    GT_U8                devNum;
    MV_HWS_UNITS_ID      unitId;

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }
    devNum = inArgs[0];
    switch(inFields[1])
    {
    case WRITE_OP:
      mvUnitInfoGetByAddr(devNum, (GT_U32)inFields[2], &unitId);
      wrParam.indexOffset = unitId;
      wrParam.regOffset = inFields[3];
      wrParam.data = inFields[4];
      wrParam.mask = inFields[5];
      cfgEl.params = &wrParam;
      cfgEl.op = WRITE_OP;
      break;
    case DELAY_OP:
      delOp.delay = inFields[6];
      cfgEl.params = &delOp;
      cfgEl.op = DELAY_OP;
      break;
    case POLLING_OP:
      mvUnitInfoGetByAddr(devNum, (GT_U32)inFields[2], &unitId);
      polParam.indexOffset = unitId;
      polParam.regOffset = inFields[3];
      polParam.data = inFields[4];
      polParam.mask = inFields[5];
      polParam.waitTime = inFields[6];
      polParam.numOfLoops = inFields[7];
      cfgEl.params = &polParam;
      cfgEl.op = POLLING_OP;
      break;
    }

    switch(inArgs[1])
    {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
      /* GE MAC */
      rc = hwsGeMacSeqSet(((inFields[0] == 0) ? GT_TRUE : GT_FALSE),
        (MV_HWS_GE_MAC_SUB_SEQ)inArgs[1], &cfgEl, inFields[0]);
      break;
    case 7:
      /* HGL MAC */
      rc = hwsHglMacSeqSet(((inFields[0] == 0) ? GT_TRUE : GT_FALSE),
        (MV_HWS_HGL_MAC_SUB_SEQ)(inArgs[1]-7), &cfgEl, inFields[0]);
      break;
    case 8:
      /* XG MAC */
      rc = hwsXgMacSeqSet(((inFields[0] == 0) ? GT_TRUE : GT_FALSE),
        (MV_HWS_XG_MAC_SUB_SEQ)(inArgs[1]-8), &cfgEl, inFields[0]);
      break;
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
      /* XLG MAC */
      rc = hwsXlgMacSeqSet(((inFields[0] == 0) ? GT_TRUE : GT_FALSE),
        (MV_HWS_XLG_MAC_SUB_SEQ)(inArgs[1]-9), &cfgEl, inFields[0]);
      break;
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
      /* CG MAC */
      rc = hwsCgMacSeqSet(((inFields[0] == 0) ? GT_TRUE : GT_FALSE),
        (MV_HWS_CG_MAC_SUB_SEQ)(inArgs[1]-17), &cfgEl, inFields[0]);
      break;
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
      /* INLK MAC */
      rc = hwsIlknMacSeqSet(((inFields[0] == 0) ? GT_TRUE : GT_FALSE),
        (MV_HWS_INTKLN_MAC_SUB_SEQ)(inArgs[1]-22), &cfgEl, inFields[0]);
      break;
    default:
        galtisOutput(outArgs, GT_BAD_PARAM, "");
        return CMD_OK;
    }

    galtisOutput(outArgs, rc, "");
    return CMD_OK;

}

/*******************************************************************************
*
* dbCommands - Galtis database galtis commands
*
*******************************************************************************/
static CMD_COMMAND dbCommands[] = {
{"HwsMacModeCfgSet", &wrHwsMacModeCfgSetEntry, 1,  3},
{"HwsMacResetCfgSet", &wrHwsMacResetCfgSetEntry, 1,  3},
{"HwsMacSeqCfgGetFirst", &wrHwsMacSeqCfgGetFirst, 2,  0},
{"HwsMacSeqCfgGetNext", &wrHwsMacSeqCfgGetNext, 2,  0},
{"HwsMacSeqCfgSet", &wrHwsMacSeqCfgSet, 2,  8},
};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitHwsMacIf function
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
GT_STATUS cmdLibInitHwsMacIf()
{
    return cmdInitLibrary(dbCommands, numCommands);
}


