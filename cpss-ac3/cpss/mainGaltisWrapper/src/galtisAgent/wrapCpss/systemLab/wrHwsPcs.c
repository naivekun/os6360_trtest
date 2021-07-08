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
* @file wrHwsPcs.c
*
* @brief Implementation of the Galtis Wrapper functions
*
* @version   2
********************************************************************************
*/

/* Common for all HWS Galtis wrappers */
#include <galtisAgent/wrapCpss/systemLab/wrHwsCommon.h>

/* General H Files */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>

/* Feature specific includes */
#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/gPcs/mvHwsGPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/hglPcs/mvHwsHglPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mmPcs/mvHwsMMPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/xPcs/mvHwsXPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/gPcs/mvHwsGPcsDb.h>
#include <cpss/common/labServices/port/gop/port/pcs/hglPcs/mvHwsHglPcsDb.h>
#include <cpss/common/labServices/port/gop/port/pcs/cgPcs/mvHwsCgPcsDb.h>
#include <cpss/common/labServices/port/gop/port/pcs/ilknPcs/mvHwsIlknPcsDb.h>
#include <cpss/common/labServices/port/gop/port/pcs/mmPcs/mvHwsMMPcsDb.h>
#include <cpss/common/labServices/port/gop/port/pcs/mmPcs/mvHwsMMPcsDbRev3.h>
#include <cpss/common/labServices/port/gop/port/pcs/mmPcs/mvHwsMMPcs28nmDb.h>
#include <cpss/common/labServices/port/gop/port/pcs/xPcs/mvHwsXPcsDb.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/d2dPcs/mvHwsD2dPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mmPcs/mvHwsMMPcs28nmIf.h>
/**
* @internal wrHwsPcsModeCfgSetEntry function
* @endinternal
*
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
static CMD_STATUS wrHwsPcsModeCfgSetEntry
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum;
    GT_U32      pcsNum;
    MV_HWS_PORT_STANDARD    portMode;

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }
    devNum = (GT_U8)inArgs[0];
    pcsNum = (GT_U32)inFields[0];
    portMode = (MV_HWS_PORT_STANDARD)inFields[1];

    rc = mvHwsPcsModeCfg(globalDevNum, devNum, pcsNum, portMode);

    galtisOutput(outArgs, rc, "");
    return CMD_OK;
}

/**
* @internal wrHwsPcsResetCfgSetEntry function
* @endinternal
*
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
static CMD_STATUS wrHwsPcsResetCfgSetEntry
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum;
    GT_U32      pcsNum;
    MV_HWS_PORT_PCS_TYPE    pcsType;
    MV_HWS_RESET  action;
    MV_HWS_PORT_STANDARD portMode;

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }
    devNum = (GT_U8)inArgs[0];
    pcsNum = (GT_U32)inFields[0];
    portMode = (MV_HWS_PORT_STANDARD)inFields[3];

    switch(inFields[1])
    {
    case 1:
      pcsType = GPCS;
      break;
    case 2:
      pcsType = XPCS;
      break;
    case 3:
      pcsType = MMPCS;
      break;
    case 4:
      pcsType = HGLPCS;
      break;
    default:
          galtisOutput(outArgs, GT_NOT_SUPPORTED, "");
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

    /*osPrintf("mvHwsPcsReset num %d type %d action %d\n", pcsNum, pcsType, action);*/
    rc = mvHwsPcsReset(globalDevNum, devNum, pcsNum, portMode, pcsType, action);

    galtisOutput(outArgs, rc, "");
    return CMD_OK;
}

/**
* @internal wrHwsPcsTestGeneratorCfgSetEntry function
* @endinternal
*
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
static CMD_STATUS wrHwsPcsTestGeneratorCfgSetEntry
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum;
    GT_U32      pcsNum;
    MV_HWS_PORT_TEST_GEN_PATTERN pattern;

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }
    devNum = (GT_U8)inArgs[0];
    pcsNum = (GT_U32)inFields[0];

    switch(inFields[2])
    {
    case 0:
      pattern = TEST_GEN_PRBS7;
      break;
    case 1:
      pattern = TEST_GEN_PRBS23;
      break;
    case 2:
      pattern = TEST_GEN_CJPAT;
      break;
    case 3:
      pattern = TEST_GEN_CRPAT;
      break;
    case 4:
      pattern = TEST_GEN_KRPAT;
      break;
    case 5:
      pattern = TEST_GEN_Normal;
      break;
    default:
          galtisOutput(outArgs, GT_NOT_SUPPORTED, "");
          return CMD_OK;
    }

    switch(inFields[1])
    {
    case 1:
      rc = mvHwsGPcsTestGenCfg(globalDevNum, devNum,pcsNum,pattern);
      break;
    case 2:
      rc = mvHwsXPcsTestGenCfg(globalDevNum, devNum,pcsNum,pattern);
      break;
    case 3:
      rc = mvHwsMMPcsTestGenCfg(globalDevNum, devNum,pcsNum,pattern);
      break;
    case 4:
      rc = mvHwsXPcsTestGenCfg(globalDevNum, devNum,pcsNum,pattern);
      break;
    default:
          galtisOutput(outArgs, GT_NOT_SUPPORTED, "");
          return CMD_OK;
    }

    galtisOutput(outArgs, rc, "");
    return CMD_OK;
}


/**
* @internal wrHwsPcsTestGeneratorErrorsGetFirstEntry function
* @endinternal
*
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
static CMD_STATUS wrHwsPcsTestGeneratorErrorsGetFirstEntry
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum = (GT_U8)inArgs[0];
    GT_U32      pcsNum = (GT_U32)inArgs[2];
    MV_HWS_PORT_TEST_GEN_PATTERN pattern;
    MV_HWS_TEST_GEN_STATUS status;

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }
    switch(inArgs[3])
    {
    case 0:
      pattern = TEST_GEN_PRBS7;
      break;
    case 1:
      pattern = TEST_GEN_PRBS23;
      break;
    case 2:
      pattern = TEST_GEN_CJPAT;
      break;
    case 3:
      pattern = TEST_GEN_CRPAT;
      break;
    case 4:
      pattern = TEST_GEN_KRPAT;
      break;
    case 5:
      pattern = TEST_GEN_Normal;
      break;
    default:
          galtisOutput(outArgs, GT_NOT_SUPPORTED, "");
          return CMD_OK;
    }

    switch(inArgs[1])
    {
    case 1:
      rc = mvHwsGPcsTestGenStatus(globalDevNum, devNum, pcsNum, pattern, &status);
      break;
    case 2:
      rc = mvHwsXPcsTestGenStatus(globalDevNum, devNum, pcsNum, pattern, &status);
      break;
    case 3:
      rc = mvHwsMMPcsTestGenStatus(globalDevNum, devNum, pcsNum, pattern, &status);
      break;
    case 4:
      rc = mvHwsHglPcsTestGenStatus(globalDevNum, devNum, pcsNum, pattern, &status);
      break;
    default:
      galtisOutput(outArgs, GT_NOT_SUPPORTED, "");
      return CMD_OK;
    }
    inFields[1] = (GT_32)status.checkerLock;
    inFields[2] = (GT_32)status.goodFrames;
    inFields[3] = (GT_32)status.totalErrors;

    /* pack and output table fields */
    fieldOutput("%d%d%d%d", (GT_32)pcsNum, inFields[1],inFields[2],inFields[3]);
    galtisOutput(outArgs, rc, "%f");
    return CMD_OK;
}

/**
* @internal wrHwsPcsTestGeneratorErrorsGetNextEntry function
* @endinternal
*
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
static CMD_STATUS wrHwsPcsTestGeneratorErrorsGetNextEntry
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

static GT_U32 seqCurLine;
/**
* @internal wrHwsPcsSeqCfgGetFirst function
* @endinternal
*
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
static CMD_STATUS wrHwsPcsSeqCfgGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    MV_CFG_ELEMENT cfgEl;
    GT_U8           devNum;
    GT_U32          baseAddr, unitIndex;

    seqCurLine = 0;
    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    devNum = (GT_U8)inArgs[0];

    switch(inArgs[1])
    {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
        /* CG PCS */
      rc = hwsCgPcsSeqGet((MV_HWS_CGPCS_SUB_SEQ)(inArgs[1]), &cfgEl, 0);
      break;
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
      /* GE PCS */
      rc = hwsGPcsSeqGet((MV_HWS_GPCS_SUB_SEQ)(inArgs[1]-7), &cfgEl, 0);
      break;
    case 13:
    case 14:
    case 15:
      /* HGL PCS */
      rc = hwsHglPcsSeqGet((MV_HWS_HGLPCS_SUB_SEQ)(inArgs[1]-13), &cfgEl, 0);
      break;
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
      /* ILKN PCS */
      rc = hwsIlknPcsSeqGet((MV_HWS_ILKNPCS_SUB_SEQ)(inArgs[1]-16), &cfgEl, 0);
      break;
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
    case 30:
    case 31:
        /* MMPCS */
      rc = hwsMMPcsSeqGet((MV_HWS_MM_PCS_SUB_SEQ)(inArgs[1]-22), &cfgEl, 0);
      break;
    case 32:
    case 33:
    case 34:
    case 35:
    case 36:
    case 37:
    case 38:
    case 39:
    case 40:
    case 41:
    case 42:
    case 43:
    case 44:
    case 45:
      /* XG PCS */
      rc = hwsXPcsSeqGet((MV_HWS_XPCS_SUB_SEQ)(inArgs[1]-32), &cfgEl, 0);
      break;
    case 46:
    case 47:
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
        if(HWS_DEV_SILICON_TYPE(devNum) == Puma3B0)
        {
            /* MMPCS Rev3 sequences */
            rc = hwsMMPcsRev3SeqGet((MV_HWS_MM_PCS_SUB_SEQ)(inArgs[1]- 46 + 13), &cfgEl, 0);
        }
        else
        {
            /* MMPCS 28nm sequences */
            rc = hwsMMPcs28nmSeqGet((MV_HWS_MM_PCS_SUB_SEQ)(inArgs[1]- 46 + 13), &cfgEl, 0);
        }
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
* @internal wrHwsPcsSeqCfgGetNext function
* @endinternal
*
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
static CMD_STATUS wrHwsPcsSeqCfgGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    MV_CFG_ELEMENT cfgEl;
    GT_U8           devNum;
    GT_U32          baseAddr, unitIndex;

    inFields[0] = ++seqCurLine;
    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    devNum = (GT_U8)inArgs[0];

    switch(inArgs[1])
    {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
        /* CG PCS */
      rc = hwsCgPcsSeqGet((MV_HWS_CGPCS_SUB_SEQ)(inArgs[1]), &cfgEl, seqCurLine);
      break;
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
      /* GE PCS */
      rc = hwsGPcsSeqGet((MV_HWS_GPCS_SUB_SEQ)(inArgs[1]-7), &cfgEl, seqCurLine);
      break;
    case 13:
    case 14:
    case 15:
      /* HGL PCS */
      rc = hwsHglPcsSeqGet((MV_HWS_HGLPCS_SUB_SEQ)(inArgs[1]-13), &cfgEl, seqCurLine);
      break;
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
      /* ILKN PCS */
      rc = hwsIlknPcsSeqGet((MV_HWS_ILKNPCS_SUB_SEQ)(inArgs[1]-16), &cfgEl, seqCurLine);
      break;
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
    case 30:
    case 31:
        /* MMPCS */
        rc = hwsMMPcsSeqGet((MV_HWS_MM_PCS_SUB_SEQ)(inArgs[1]-22), &cfgEl, seqCurLine);
        break;
    case 32:
    case 33:
    case 34:
    case 35:
    case 36:
    case 37:
    case 38:
    case 39:
    case 40:
    case 41:
    case 42:
    case 43:
    case 44:
    case 45:
      /* XG PCS */
      rc = hwsXPcsSeqGet((MV_HWS_XPCS_SUB_SEQ)(inArgs[1]-32), &cfgEl, seqCurLine);
      break;
    case 46:
    case 47:
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
        if(HWS_DEV_SILICON_TYPE(devNum) == Puma3B0)
        {
            /* MMPCS Rev3 sequences */
            rc = hwsMMPcsRev3SeqGet((MV_HWS_MM_PCS_SUB_SEQ)(inArgs[1]- 46 + 13), &cfgEl, seqCurLine);
        }
        else
        {
            /* MMPCS 28nm sequences */
            rc = hwsMMPcs28nmSeqGet((MV_HWS_MM_PCS_SUB_SEQ)(inArgs[1]- 46 + 13), &cfgEl, seqCurLine);
        }
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
    galtisOutput(outArgs, GT_OK, "%f");
    return CMD_OK;

}

/**
* @internal wrHwsPcsSeqCfgSet function
* @endinternal
*
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
static CMD_STATUS wrHwsPcsSeqCfgSet
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
    GT_U8           devNum;
    MV_HWS_UNITS_ID unitId;

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
        /* CG PCS */
      rc = hwsCgPcsSeqSet(((inFields[0] == 0) ? GT_TRUE : GT_FALSE),
          (MV_HWS_CGPCS_SUB_SEQ)(inArgs[1]), &cfgEl, (GT_U32)inFields[0]);
      break;
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
      /* GE PCS */
      rc = hwsGPcsSeqSet(((inFields[0] == 0) ? GT_TRUE : GT_FALSE),
        (MV_HWS_GPCS_SUB_SEQ)(inArgs[1]-7), &cfgEl, (GT_U32)inFields[0]);
      break;
    case 13:
    case 14:
    case 15:
      /* HGL PCS */
      rc = hwsHglPcsSeqSet(((inFields[0] == 0) ? GT_TRUE : GT_FALSE),
        (MV_HWS_HGLPCS_SUB_SEQ)(inArgs[1]-13), &cfgEl, (GT_U32)inFields[0]);
      break;
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
      /* ILKN PCS */
      rc = hwsIlknPcsSeqSet(((inFields[0] == 0) ? GT_TRUE : GT_FALSE),
        (MV_HWS_ILKNPCS_SUB_SEQ)(inArgs[1]-16), &cfgEl, (GT_U32)inFields[0]);
      break;
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
    case 30:
    case 31:
        /* MMPCS */
      rc = hwsMMPcsSeqSet(((inFields[0] == 0) ? GT_TRUE : GT_FALSE),
        (MV_HWS_MM_PCS_SUB_SEQ)(inArgs[1]-22), &cfgEl, (GT_U32)inFields[0]);
        break;
    case 32:
    case 33:
    case 34:
    case 35:
    case 36:
    case 37:
    case 38:
    case 39:
    case 40:
    case 41:
    case 42:
    case 43:
    case 44:
    case 45:
      /* XG PCS */
      rc = hwsXPcsSeqSet(((inFields[0] == 0) ? GT_TRUE : GT_FALSE),
        (MV_HWS_XPCS_SUB_SEQ)(inArgs[1]-32), &cfgEl, (GT_U32)inFields[0]);
      break;
    case 46:
    case 47:
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
        if(HWS_DEV_SILICON_TYPE(devNum) == Puma3B0)
        {
            /* MMPCS Rev3 sequences */
            rc = hwsMMPcsRev3SeqSet(((inFields[0] == 0) ? GT_TRUE : GT_FALSE),
                                    (MV_HWS_MM_PCS_SUB_SEQ)(inArgs[1]- 46 + 13), &cfgEl, (GT_U32)inFields[0]);
        }
        else
        {
            /* MMPCS 28nm sequences */
            rc = hwsMMPcs28nmSeqSet(((inFields[0] == 0) ? GT_TRUE : GT_FALSE),
                                    (MV_HWS_MM_PCS_SUB_SEQ)(inArgs[1]- 46 + 13), &cfgEl, (GT_U32)inFields[0]);
        }
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
* mvHwsD2dPcsTestGen
*
* DESCRIPTION:
*       D2D PCS PRBS configuration start/stop.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
static CMD_STATUS wrHwsD2dPcsTestGen
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       rc;             /* Return code  */
    GT_U8                           devNum;
    GT_U32                          port;
    MV_HWS_D2D_PRBS_CONFIG_STC      config;

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    devNum = (GT_U8)inArgs[0];
    port = (GT_U32)inArgs[1];
    config.direction = (MV_HWS_D2D_DIRECTION_MODE_ENT)inArgs[2];
    config.portPattern = (MV_HWS_PORT_TEST_GEN_PATTERN)inArgs[3];

    rc = mvHwsD2dPcsTestGen(devNum, 0, port, &config);

    galtisOutput(outArgs, rc, "");
    return CMD_OK;
}

/*******************************************************************************
* mvHwsD2dPcsLoopbackSet
*
* DESCRIPTION:
*       D2D PCS loopback configuration.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
static CMD_STATUS wrHwsD2dPcsLoopbackSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       rc;             /* Return code  */
    GT_U8                           devNum;
    GT_U32                          port;
    MV_HWS_PORT_LB_TYPE             lbType;

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    devNum = (GT_U8)inArgs[0];
    port = (GT_U32)inArgs[1];
    lbType = (MV_HWS_PORT_LB_TYPE)inArgs[2];

    rc = mvHwsD2dPcsLoopbackSet(devNum, 0,port, NON_SUP_MODE, lbType);

    galtisOutput(outArgs, rc, "");
    return CMD_OK;
}

/*******************************************************************************
* mvHwsD2dPcsErrorInjectionSet
*
* DESCRIPTION:
*       D2D PCS error injection configuration.
*       This configuration is SC and will be cleared after err is inserted
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
static CMD_STATUS wrHwsD2dPcsErrorInjectionSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               rc;         /* return code */
    GT_U8                   devNum;     /* device number */
    GT_U32                  port;       /* port number */
    GT_U32                  serdesIdx;  /* lane 0..3 */
    GT_U32                  errorNum;   /* 1..0x7FF, 0x7FF enables continuously injection */

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    devNum = (GT_U8)inArgs[0];
    port = (GT_U32)inArgs[1];
    serdesIdx = (GT_U32)inArgs[2];
    errorNum = (GT_U32)inArgs[3];

    rc = mvHwsD2dPcsErrorInjectionSet(devNum, 0, port, serdesIdx, errorNum);

    galtisOutput(outArgs, rc, "");
    return CMD_OK;
}

/*******************************************************************************
* mvHwsD2dPcsErrorInjectionSet
*
* DESCRIPTION:
*       D2D PCS error injection configuration.
*       This configuration is SC and will be cleared after err is inserted
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
static CMD_STATUS wrHwsD2dPcsErrorCounterGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* return code */
    GT_U8       devNum;     /* device number */
    GT_U32      port;       /* port number 1024.. */
    GT_U32      serdesIdx;  /* lane 0..3 */
    GT_U32      errorCnt;   /* error counter value */

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    devNum = (GT_U8)inArgs[0];
    port = (GT_U32)inArgs[1];
    serdesIdx = (GT_U32)inArgs[2];

    rc = mvHwsD2dPcsErrorCounterGet(devNum, 0, port, serdesIdx, &errorCnt);

    galtisOutput(outArgs, rc, "%d", errorCnt);
    return CMD_OK;
}

/*******************************************************************************
* mvHwsRsFecCorrectedError
*
* DESCRIPTION:
*       Return number of codewords corrected by RS-FEC.
*
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
static CMD_STATUS wrHwsRsFecCorrectedError
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* return code */
    GT_U8       devNum;     /* device number */
    GT_U32      portGroup;
    GT_U32      portNum;
    GT_U32      errorCnt;   /* error corrected counter value */

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    devNum = (GT_U8)inArgs[0];
    portGroup = (GT_U32)inArgs[1];
    portNum = (GT_U32)inArgs[2];

    rc = mvHwsRsFecCorrectedError(devNum, portGroup, portNum, &errorCnt);

    galtisOutput(outArgs, rc, "%d", errorCnt);
    return CMD_OK;
}

/*******************************************************************************
* mvHwsFcFecCorrectedError
*
* DESCRIPTION:
*       Return number of bitss corrected by FC-FEC.
*
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
static CMD_STATUS wrHwsFcFecCorrectedError
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* return code */
    GT_U8       devNum;     /* device number */
    GT_U32      portGroup;
    GT_U32      portNum;
    GT_U32      errorCnt;   /* error corrected counter value */

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    devNum = (GT_U8)inArgs[0];
    portGroup = (GT_U32)inArgs[1];
    portNum = (GT_U32)inArgs[2];

    rc = mvHwsFcFecCorrectedError(devNum, portGroup, portNum, &errorCnt);

    galtisOutput(outArgs, rc, "%d", errorCnt);
    return CMD_OK;
}

/*******************************************************************************
*
* dbCommands - Galtis database galtis commands
*
*******************************************************************************/
static CMD_COMMAND dbCommands[] = {
{"HwsPcsModeCfgSet", &wrHwsPcsModeCfgSetEntry, 1,  4},
{"HwsPcsResetCfgSet", &wrHwsPcsResetCfgSetEntry, 1,  4},
{"HwsPcsTestGeneratorCfgSet", &wrHwsPcsTestGeneratorCfgSetEntry, 1,  3},
{"HwsPcsTestGeneratorErrorsGetFirst", &wrHwsPcsTestGeneratorErrorsGetFirstEntry, 4,  0},
{"HwsPcsTestGeneratorErrorsGetNext", &wrHwsPcsTestGeneratorErrorsGetNextEntry, 4,  0},
{"HwsPcsSeqCfgGetFirst", &wrHwsPcsSeqCfgGetFirst, 2,  0},
{"HwsPcsSeqCfgGetNext", &wrHwsPcsSeqCfgGetNext, 2,  0},
{"HwsPcsSeqCfgSet", &wrHwsPcsSeqCfgSet, 2,  8},
{"HwsD2dPcsTestGen", &wrHwsD2dPcsTestGen, 4, 0},
{"HwsD2dPcsLoopbackSet", &wrHwsD2dPcsLoopbackSet, 3, 0},
{"HwsD2dPcsErrorInjectionSet", &wrHwsD2dPcsErrorInjectionSet, 4, 0},
{"HwsD2dPcsErrorCounterGet", &wrHwsD2dPcsErrorCounterGet, 3, 0},
{"HwsRsFecCorrectedError", &wrHwsRsFecCorrectedError, 3, 0},
{"HwsFcFecCorrectedError", &wrHwsFcFecCorrectedError, 3, 0}

};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitHwsPcsIf function
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
GT_STATUS cmdLibInitHwsPcsIf()
{
    return cmdInitLibrary(dbCommands, numCommands);
}


