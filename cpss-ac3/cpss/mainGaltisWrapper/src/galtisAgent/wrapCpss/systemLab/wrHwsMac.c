/******************************************************************************
*              Copyright (c) Marvell International Ltd. and its affiliates
*
* This software file (the \"File\") is owned and distributed by Marvell
* International Ltd. and/or its affiliates (\"Marvell\") under the following
* alternative licensing terms.
* If you received this File from Marvell, you may opt to use, redistribute
* and/or modify this File under the following licensing terms.
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*  -   Redistributions of source code must retain the above copyright notice,
*       this list of conditions and the following disclaimer.
*  -   Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*  -    Neither the name of Marvell nor the names of its contributors may be
*       used to endorse or promote products derived from this software without
*       specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************
*/
/**
********************************************************************************
* @file wrHwsMac.c
*
* @brief Implementation of the Galtis Wrapper functions
*
* @version   1
********************************************************************************
*/

/* Common for all HWS Galtis wrappers */
#include <galtisAgent/wrapCpss/systemLab/wrHwsCommon.h>

/* General H Files */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>

/* Feature specific includes*/
#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>
#include <cpss/common/labServices/port/gop/port/mac/geMac/mvHwsGeMacDb.h>
#include <cpss/common/labServices/port/gop/port/mac/hglMac/mvHwsHglMacDb.h>
#include <cpss/common/labServices/port/gop/port/mac/xgMac/mvHwsXgMacDb.h>
#include <cpss/common/labServices/port/gop/port/mac/xlgMac/mvHwsXlgMacDb.h>
#include <cpss/common/labServices/port/gop/port/mac/cgMac/mvHwsCgMacDb.h>
#include <cpss/common/labServices/port/gop/port/mac/ilknMac/mvHwsIlknMacDb.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>


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
static CMD_STATUS wrHwsMacModeCfgSetEntry
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum;
    GT_U32      macNum;
    MV_HWS_PORT_STANDARD    portMode;

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }
    devNum = (GT_U8)inArgs[0];
    macNum = (GT_U32)inFields[0];
    portMode = (MV_HWS_PORT_STANDARD)inFields[1];

    rc = mvHwsMacModeCfg(globalDevNum , devNum, macNum, portMode);

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
static CMD_STATUS wrHwsMacResetCfgSetEntry
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum;
    GT_U32      macNum;
    MV_HWS_PORT_MAC_TYPE    macType;
    MV_HWS_RESET  action;
    MV_HWS_PORT_STANDARD    portMode;

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }
    devNum = (GT_U8)inArgs[0];
    macNum = (GT_U32)inFields[0];

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

    portMode = (MV_HWS_PORT_STANDARD)inFields[3];
    rc = mvHwsMacReset(globalDevNum , devNum, macNum, portMode, macType, action);

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
static CMD_STATUS wrHwsMacSeqCfgGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    MV_CFG_ELEMENT cfgEl;
    GT_U32          baseAddr, unitIndex;

    seqCurLine = 0;
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
      mvUnitInfoGet(globalDevNum, (MV_HWS_UNITS_ID)cfgEl.params.unitId, &baseAddr, &unitIndex);
      inFields[2] = baseAddr;
      inFields[3] = cfgEl.params.regOffset;
      inFields[4] = cfgEl.params.operData;
      inFields[5] = cfgEl.params.mask;
      inFields[6] = 0;
      inFields[7] = 0;
      break;
    case DELAY_OP:
      inFields[2] = 0;
      inFields[3] = 0;
      inFields[4] = 0;
      inFields[5] = 0;
      inFields[6] = cfgEl.params.operData;
      inFields[7] = 0;
      break;
    case POLLING_OP:
      mvUnitInfoGet(globalDevNum, (MV_HWS_UNITS_ID)cfgEl.params.unitId, &baseAddr, &unitIndex);
      inFields[2] = baseAddr;
      inFields[3] = cfgEl.params.regOffset;
      inFields[4] = cfgEl.params.operData;
      inFields[5] = cfgEl.params.mask;
      inFields[6] = 0;
      inFields[7] = 0;
      break;
    }

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d",inFields[0],inFields[1],inFields[2],inFields[3],inFields[4],inFields[5],inFields[6],inFields[7]);
    galtisOutput(outArgs, rc, "%f");
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
static CMD_STATUS wrHwsMacSeqCfgGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    MV_CFG_ELEMENT cfgEl;
    GT_U32          baseAddr, unitIndex;

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }
    inFields[0] = ++seqCurLine;

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
    inFields[1] = (GT_U32)(cfgEl.op);
    switch(inFields[1])
    {
    case WRITE_OP:
      mvUnitInfoGet(globalDevNum, (MV_HWS_UNITS_ID)cfgEl.params.unitId, &baseAddr, &unitIndex);
      inFields[2] = baseAddr;
      inFields[3] = cfgEl.params.regOffset;
      inFields[4] = cfgEl.params.operData;
      inFields[5] = cfgEl.params.mask;
      inFields[6] = 0;
      inFields[7] = 0;
      break;
    case DELAY_OP:
      inFields[2] = 0;
      inFields[3] = 0;
      inFields[4] = 0;
      inFields[5] = 0;
      inFields[6] = cfgEl.params.operData;
      inFields[7] = 0;
      break;
    case POLLING_OP:
      mvUnitInfoGet(globalDevNum, (MV_HWS_UNITS_ID)cfgEl.params.unitId, &baseAddr, &unitIndex);
      inFields[2] = baseAddr;
      inFields[3] = cfgEl.params.regOffset;
      inFields[4] = cfgEl.params.operData;
      inFields[5] = cfgEl.params.mask;
      inFields[6] = 0;
      inFields[7] = 0;
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
static CMD_STATUS wrHwsMacSeqCfgSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
#if 0
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
    devNum = (GT_U8)inArgs[0];
    switch(inFields[1])
    {
    case WRITE_OP:
      mvUnitInfoGetByAddr(devNum, (GT_U32)inFields[2], &unitId);
      wrParam.indexOffset = unitId;
      wrParam.regOffset = (GT_U32)inFields[3];
      wrParam.data = (GT_U32)inFields[4];
      wrParam.mask = (GT_U32)inFields[5];
      cfgEl.params = &wrParam;
      cfgEl.op = WRITE_OP;
      break;
    case DELAY_OP:
      delOp.delay = (GT_U16)inFields[6];
      cfgEl.params = &delOp;
      cfgEl.op = DELAY_OP;
      break;
    case POLLING_OP:
      mvUnitInfoGetByAddr(devNum, (GT_U32)inFields[2], &unitId);
      polParam.indexOffset = unitId;
      polParam.regOffset = (GT_U32)inFields[3];
      polParam.data = (GT_U32)inFields[4];
      polParam.mask = (GT_U32)inFields[5];
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
        (MV_HWS_GE_MAC_SUB_SEQ)inArgs[1], &cfgEl, (GT_U32)inFields[0]);
      break;
    case 7:
      /* HGL MAC */
      rc = hwsHglMacSeqSet(((inFields[0] == 0) ? GT_TRUE : GT_FALSE),
        (MV_HWS_HGL_MAC_SUB_SEQ)(inArgs[1]-7), &cfgEl, (GT_U32)inFields[0]);
      break;
    case 8:
      /* XG MAC */
      rc = hwsXgMacSeqSet(((inFields[0] == 0) ? GT_TRUE : GT_FALSE),
        (MV_HWS_XG_MAC_SUB_SEQ)(inArgs[1]-8), &cfgEl, (GT_U32)inFields[0]);
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
        (MV_HWS_XLG_MAC_SUB_SEQ)(inArgs[1]-9), &cfgEl, (GT_U32)inFields[0]);
      break;
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
      /* CG MAC */
      rc = hwsCgMacSeqSet(((inFields[0] == 0) ? GT_TRUE : GT_FALSE),
        (MV_HWS_CG_MAC_SUB_SEQ)(inArgs[1]-17), &cfgEl, (GT_U32)inFields[0]);
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
        (MV_HWS_INTKLN_MAC_SUB_SEQ)(inArgs[1]-22), &cfgEl, (GT_U32)inFields[0]);
      break;
    default:
        galtisOutput(outArgs, GT_BAD_PARAM, "");
        return CMD_OK;
    }

    galtisOutput(outArgs, rc, "");
#endif
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

