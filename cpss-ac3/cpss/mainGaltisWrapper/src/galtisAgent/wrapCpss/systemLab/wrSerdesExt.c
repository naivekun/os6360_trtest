/******************************************************************************
*              Copyright (c) Marvell International Ltd. and its affiliates
*
* This software file (the "File") is owned and distributed by Marvell
* International Ltd. and/or its affiliates ("Marvell") under the following
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
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
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
* @file wrSerdesExt.c
*
* @brief Implementation of the Galtis Wrapper functions
*
* @version   3.
********************************************************************************
*/

/* Common for all HWS Galtis wrappers */
#include <galtisAgent/wrapCpss/systemLab/wrHwsCommon.h>

/* General H Files */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>
#include <stdlib.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesIf.h>

/**
* @internal wrHwsSerdesRegAccessGetFirst function
* @endinternal
*
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
static CMD_STATUS wrHwsSerdesRegAccessGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32  data;
    GT_STATUS  res;

     /* Parameters List */
    GT_U8   DevNum;
    GT_U32  SerdesNum;
    GT_U32  RegAddress;
    GT_U32  Mask;

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    DevNum        = (GT_U8)inArgs[0];
    SerdesNum     = (GT_U32)inArgs[1];
    RegAddress    = (GT_U32)inArgs[2];
    Mask          = (GT_U32)inArgs[3];

    /* Warnings */
    SerdesNum = SerdesNum;

    if (Mask == 0)
    {
        Mask = 0xFFFFFFFF;
    }

    res = genUnitRegisterGet(globalDevNum, DevNum, SERDES_UNIT, SerdesNum, RegAddress, &data, Mask);

    if (res != GT_OK)
    {
        galtisOutput(outArgs, res, "");
        return CMD_OK;
    }

    /* Fields List */
    inFields[0] = data;

    /* pack and output table fields */
    fieldOutput("%d", inFields[0]);

    galtisOutput(outArgs, GT_OK, "%f");
    return CMD_OK;
}

/**
* @internal wrHwsSerdesRegAccessGetNext function
* @endinternal
*
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
static CMD_STATUS wrHwsSerdesRegAccessGetNext
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
* @internal wrHwsSerdesRegAccessSet function
* @endinternal
*
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
static CMD_STATUS wrHwsSerdesRegAccessSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  res;

    /* Parameters List */
    GT_U8   DevNum;
    GT_U32  SerdesNum;
    GT_U32  RegAddress;
    GT_U32  Mask;
    GT_U32  Data;

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    /* Parameters List */
    DevNum        = (GT_U8)inArgs[0];
    SerdesNum     = (GT_U32)inArgs[1];
    RegAddress    = (GT_U32)inArgs[2];
    Mask          = (GT_U32)inArgs[3];
    Data          = (GT_U32)inFields[0];

    /* Warnings */
    SerdesNum = SerdesNum;

    if (Mask == 0)
    {
        Mask = 0xFFFFFFFF;
    }

    res = genUnitRegisterSet(globalDevNum, DevNum, SERDES_UNIT, SerdesNum, RegAddress, Data, Mask);

    galtisOutput(outArgs, res, "");
    return CMD_OK;
}

/**
* @internal wrHwsSerdesPolarityCfg function
* @endinternal
*
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
static CMD_STATUS wrHwsSerdesPolarityCfg
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  res = GT_OK;

    /* Parameters List */
    GT_U32  DevNum     = (GT_U32)inArgs[0];
    GT_U32  SerdesNum  = (GT_U32)inArgs[1];
    GT_U32  TxInvert   = (GT_U32)inArgs[2];
    GT_U32  RxInvert   = (GT_U32)inArgs[3];

    /* Warnings */
    DevNum = DevNum;
    SerdesNum = SerdesNum;
    TxInvert = TxInvert;
    RxInvert = RxInvert;

    res = mvHwsSerdesPolarityConfig(globalDevNum, DevNum, SerdesNum, (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(globalDevNum),
                                    TxInvert, RxInvert);
    if (res != GT_OK)
    {
        galtisOutput(outArgs, res, "");
        return CMD_OK;
    }

    galtisOutput(outArgs, GT_OK, "");
    return CMD_OK;
}


/*******************************************************************************
*
* dbCommands - Galtis database galtis commands
*
*******************************************************************************/
static CMD_COMMAND dbCommands[] = {

{"HwsSerdesRegAccessGetFirst", &wrHwsSerdesRegAccessGetFirst, 4,  0},
{"HwsSerdesRegAccessGetNext", &wrHwsSerdesRegAccessGetNext, 4,  0},
{"HwsSerdesRegAccessSet", &wrHwsSerdesRegAccessSet, 4,  1},
{"HwsSerdesPolarityCfg", &wrHwsSerdesPolarityCfg, 4,  0},};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))


/**
* @internal cmdLibInitSerdesExt function
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
GT_STATUS cmdLibInitSerdesExt()
{
    return cmdInitLibrary(dbCommands, numCommands);
}

