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
* @file wrHwsPcs.cpp
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

extern "C" {
GT_STATUS cmdLibInitHwsPcsIf
(
    GT_VOID
);
}

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
CMD_STATUS wrHwsPcsModeCfgSetEntry
(
    IN  GT_32 inArgs[CMD_MAX_ARGS],
    IN  GT_32 inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum = inArgs[0];
    GT_U32      pcsNum = inFields[0];
    MV_HWS_PORT_PCS_TYPE    pcsType;
    MV_HWS_PORT_FEC_MODE    fecCtrl;
    GT_U32                  numOfLanes;

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }
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
    switch(inFields[2])
    {
    case 0:
      numOfLanes = 1;
      break;
    case 1:
      numOfLanes = 2;
      break;
    case 2:
      numOfLanes = 4;
      break;
    case 3:
      numOfLanes = 6;
      break;
    }
    if (inFields[3] == 1)
    {
      fecCtrl = FC_FEC;
    }
    else
    {
      fecCtrl = FEC_OFF;
    }

    rc = mvHwsPcsModeCfg(devNum, devNum, pcsNum, pcsType, fecCtrl,numOfLanes);

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
CMD_STATUS wrHwsPcsResetCfgSetEntry
(
    IN  GT_32 inArgs[CMD_MAX_ARGS],
    IN  GT_32 inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum = inArgs[0];
    GT_U32      pcsNum = inFields[0];
    MV_HWS_PORT_PCS_TYPE    pcsType;
    MV_HWS_RESET  action;

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

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
    rc = mvHwsPcsReset(devNum, devNum, pcsNum, pcsType, action);

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
CMD_STATUS wrHwsPcsTestGeneratorCfgSetEntry
(
    IN  GT_32 inArgs[CMD_MAX_ARGS],
    IN  GT_32 inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum = inArgs[0];
    GT_U32      pcsNum = inFields[0];
    MV_HWS_PORT_TEST_GEN_PATTERN pattern;

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

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
      rc = mvHwsGPcsTestGenCfg(devNum,devNum,pcsNum,pattern);
      break;
    case 2:
      rc = mvHwsXPcsTestGenCfg(devNum,devNum,pcsNum,pattern);
      break;
    case 3:
      rc = mvHwsMMPcsTestGenCfg(devNum,devNum,pcsNum,pattern);
      break;
    case 4:
      rc = mvHwsXPcsTestGenCfg(devNum,devNum,pcsNum,pattern);
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
CMD_STATUS wrHwsPcsTestGeneratorErrorsGetFirstEntry
(
    IN  GT_32 inArgs[CMD_MAX_ARGS],
    IN  GT_32 inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum = inArgs[0];
    GT_U32      pcsNum = inArgs[2];
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
      rc = mvHwsGPcsTestGenStatus(devNum, devNum, pcsNum, pattern, &status);
      break;
    case 2:
      rc = mvHwsXPcsTestGenStatus(devNum, devNum, pcsNum, pattern, &status);
      break;
    case 3:
      rc = mvHwsMMPcsTestGenStatus(devNum, devNum, pcsNum, pattern, &status);
      break;
    case 4:
      rc = mvHwsHglPcsTestGenStatus(devNum, devNum, pcsNum, pattern, &status);
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
    galtisOutput(outArgs, GT_OK, "%f");
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
CMD_STATUS wrHwsPcsTestGeneratorErrorsGetNextEntry
(
    IN  GT_32 inArgs[CMD_MAX_ARGS],
    IN  GT_32 inFields[CMD_MAX_FIELDS],
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
CMD_STATUS wrHwsPcsSeqCfgGetFirst
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

    devNum = inArgs[0];
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
CMD_STATUS wrHwsPcsSeqCfgGetNext
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
CMD_STATUS wrHwsPcsSeqCfgSet
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
    GT_U8           devNum;
    MV_HWS_UNITS_ID unitId;

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
        /* CG PCS */
      rc = hwsCgPcsSeqSet(((inFields[0] == 0) ? GT_TRUE : GT_FALSE),
          (MV_HWS_CGPCS_SUB_SEQ)(inArgs[1]), &cfgEl, inFields[0]);
      break;
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
      /* GE PCS */
      rc = hwsGPcsSeqSet(((inFields[0] == 0) ? GT_TRUE : GT_FALSE),
        (MV_HWS_GPCS_SUB_SEQ)(inArgs[1]-7), &cfgEl, inFields[0]);
      break;
    case 13:
    case 14:
    case 15:
      /* HGL PCS */
      rc = hwsHglPcsSeqSet(((inFields[0] == 0) ? GT_TRUE : GT_FALSE),
        (MV_HWS_HGLPCS_SUB_SEQ)(inArgs[1]-13), &cfgEl, inFields[0]);
      break;
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
      /* ILKN PCS */
      rc = hwsIlknPcsSeqSet(((inFields[0] == 0) ? GT_TRUE : GT_FALSE),
        (MV_HWS_ILKNPCS_SUB_SEQ)(inArgs[1]-16), &cfgEl, inFields[0]);
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
        (MV_HWS_MM_PCS_SUB_SEQ)(inArgs[1]-22), &cfgEl, inFields[0]);
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
        (MV_HWS_XPCS_SUB_SEQ)(inArgs[1]-32), &cfgEl, inFields[0]);
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
                                    (MV_HWS_MM_PCS_SUB_SEQ)(inArgs[1]- 46 + 13), &cfgEl, inFields[0]);
        }
        else
        {
            /* MMPCS 28nm sequences */
            rc = hwsMMPcs28nmSeqSet(((inFields[0] == 0) ? GT_TRUE : GT_FALSE),
                                    (MV_HWS_MM_PCS_SUB_SEQ)(inArgs[1]- 46 + 13), &cfgEl, inFields[0]);
        }
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
{"HwsPcsModeCfgSet", &wrHwsPcsModeCfgSetEntry, 1,  4},
{"HwsPcsResetCfgSet", &wrHwsPcsResetCfgSetEntry, 1,  3},
{"HwsPcsTestGeneratorCfgSet", &wrHwsPcsTestGeneratorCfgSetEntry, 1,  3},
{"HwsPcsTestGeneratorErrorsGetFirst", &wrHwsPcsTestGeneratorErrorsGetFirstEntry, 4,  0},
{"HwsPcsTestGeneratorErrorsGetNext", &wrHwsPcsTestGeneratorErrorsGetNextEntry, 4,  0},
{"HwsPcsSeqCfgGetFirst", &wrHwsPcsSeqCfgGetFirst, 2,  0},
{"HwsPcsSeqCfgGetNext", &wrHwsPcsSeqCfgGetNext, 2,  0},
{"HwsPcsSeqCfgSet", &wrHwsPcsSeqCfgSet, 2,  8},

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


