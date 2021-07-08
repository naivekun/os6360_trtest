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
* wrapDiagCpssDxCh.c
*
* DESCRIPTION:
*       Wrapper functions for Diag cpss.dxCh functions
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 27 $
*
*******************************************************************************/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>


/* Feature specific includes */
#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/diag/cpssDxChDiag.h>
#include <cpss/dxCh/dxChxGen/diag/cpssDxChDiagPacketGenerator.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

/* support for multi port groups */

/**
* @internal diagMultiPortGroupsBmpGet function
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
static void diagMultiPortGroupsBmpGet
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
static GT_STATUS pg_wrap_cpssDxChDiagMemWrite
(
    IN GT_U8                          devNum,
    IN CPSS_DIAG_PP_MEM_TYPE_ENT      memType,
    IN GT_U32                         offset,
    IN GT_U32                         data
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    diagMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChDiagMemWrite(devNum, memType, offset, data);
    }
    else
    {
        return cpssDxChDiagPortGroupMemWrite(devNum, pgBmp, memType, offset,
                                             data);
    }
}


/* Port Group aqnd Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChDiagMemRead
(
    IN  GT_U8                          devNum,
    IN  CPSS_DIAG_PP_MEM_TYPE_ENT      memType,
    IN  GT_U32                         offset,
    OUT GT_U32                         *dataPtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    diagMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChDiagMemRead(devNum, memType, offset, dataPtr);
    }
    else
    {
        return cpssDxChDiagPortGroupMemRead(devNum, pgBmp, memType, offset,
                                            dataPtr);
    }
}


/* Port Group aqnd Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChDiagRegsNumGet
(
    IN  GT_U8     devNum,
    OUT GT_U32    *regsNumPtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    diagMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChDiagRegsNumGet(devNum, regsNumPtr);
    }
    else
    {
        return cpssDxChDiagPortGroupRegsNumGet(devNum, pgBmp, regsNumPtr);
    }
}


/* Port Group aqnd Regular version wrapper     */
/* description see in original function header */
static GT_STATUS pg_wrap_cpssDxChDiagRegsDump
(
    IN    GT_U8     devNum,
    INOUT GT_U32    *regsNumPtr,
    IN    GT_U32    offset,
    OUT   GT_U32    *regAddrPtr,
    OUT   GT_U32    *regDataPtr
)
{
    GT_BOOL             pgEnable; /* multi port group  enable */
    GT_PORT_GROUPS_BMP  pgBmp;    /* port group BMP           */

    diagMultiPortGroupsBmpGet(devNum, &pgEnable, &pgBmp);

    if (pgEnable == GT_FALSE)
    {
        return cpssDxChDiagRegsDump(devNum, regsNumPtr, offset, regAddrPtr,
                                    regDataPtr);
    }
    else
    {
        return cpssDxChDiagPortGroupRegsDump(devNum, pgBmp, regsNumPtr,
                                             offset, regAddrPtr, regDataPtr);
    }
}

/**
* @internal wrCpssDxChDiagMemTest function
* @endinternal
*
* @brief   Performs memory test on a specified memory location and size for a
*         specified memory type.
*
* @note   APPLICABLE DEVICES:      All DX CH devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong memory type
* @retval GT_FAIL                  - on error
*
* @note The test is done by writing and reading a test pattern.
*       The function may be called after Phase 2 initialization.
*       The test is destructive and leaves the memory corrupted.
*       Supported memories:
*       - Buffer DRAM
*       - MAC table memory
*       - VLAN table memory
*       For buffer DRAM:
*       startOffset must be aligned to 64 Bytes and size must be in 64 bytes
*       resolution.
*       For MAC table:
*       startOffset must be aligned to 16 Bytes and size must be in 16 bytes
*       resolution.
*       For VLAN table:
*       DX CH devices: startOffset must be aligned to 12 Bytes and size must
*       be in 12 bytes resolution.
*       DX CH2 devices: startOffset must be aligned to 16 Bytes and size must
*       be in 16 bytes resolution.
*
*/
static CMD_STATUS wrCpssDxChDiagMemTest

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                      result;

    GT_U8                          devNum;
    CPSS_DIAG_PP_MEM_TYPE_ENT      memType;
    GT_U32                         startOffset;
    GT_U32                         size;
    CPSS_DIAG_TEST_PROFILE_ENT     profile;
    GT_BOOL                        testStatusPtr;
    GT_U32                         addrPtr;
    GT_U32                         readValPtr;
    GT_U32                         writeValPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    memType = (CPSS_DIAG_PP_MEM_TYPE_ENT)inArgs[1];
    startOffset = (GT_U32)inArgs[2];
    size = (GT_U32)inArgs[3];
    profile = (CPSS_DIAG_TEST_PROFILE_ENT)inArgs[4];

    /* call cpss api function */
    result = cpssDxChDiagMemTest(devNum, memType, startOffset, size, profile,
                         &testStatusPtr, &addrPtr, &readValPtr, &writeValPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d", testStatusPtr, addrPtr,
                                             readValPtr, writeValPtr);
    return CMD_OK;
}


/**
* @internal wrCpssDxChDiagAllMemTest function
* @endinternal
*
* @brief   Performs memory test for all the internal and external memories.
*         Tested memories:
*         - Buffer DRAM
*         - MAC table memory
*         - VLAN table memory
*
* @note   APPLICABLE DEVICES:      All DX CH devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note The test is done by invoking cpssDxChDiagMemTest in loop for all the
*       memory types and for AA-55, random and incremental patterns.
*       The function may be called after Phase 2 initialization.
*       The test is destructive and leaves the memory corrupted.
*
*/
static CMD_STATUS wrCpssDxChDiagAllMemTest

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                      result;

    GT_U8                          devNum;
    GT_BOOL                        testStatusPtr;
    GT_U32                         addrPtr;
    GT_U32                         readValPtr;
    GT_U32                         writeValPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* to avoid warning */
    testStatusPtr = GT_FALSE;

    /* call cpss api function */
    result = cpssDxChDiagAllMemTest(devNum, &testStatusPtr, &addrPtr,
                                           &readValPtr, &writeValPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d", testStatusPtr, addrPtr,
                                             readValPtr, writeValPtr);
    return CMD_OK;
}


/**
* @internal wrCpssDxChDiagMemWrite function
* @endinternal
*
* @brief   performs a single 32 bit data write to one of the PP memory spaces.
*
* @note   APPLICABLE DEVICES:      All DX CH devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note offset must be aligned to 4 Bytes.
*       The function may be called after Phase 2 initialization.
*
*/
static CMD_STATUS wrCpssDxChDiagMemWrite

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                      result;

    GT_U8                          devNum;
    CPSS_DIAG_PP_MEM_TYPE_ENT      memType;
    GT_U32                         offset;
    GT_U32                         data;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    memType = (CPSS_DIAG_PP_MEM_TYPE_ENT)inArgs[1];
    offset = (GT_U32)inArgs[2];
    data = (GT_U32)inArgs[3];

    /* call cpss api function */
    result = pg_wrap_cpssDxChDiagMemWrite(devNum, memType, offset, data);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChDiagMemRead function
* @endinternal
*
* @brief   performs a single 32 bit data read from one of the PP memory spaces.
*
* @note   APPLICABLE DEVICES:      All DX CH devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note offset must be aligned to 4 Bytes.
*       The function may be called after Phase 2 initialization.
*
*/
static CMD_STATUS wrCpssDxChDiagMemRead

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                      result;

    GT_U8                          devNum;
    CPSS_DIAG_PP_MEM_TYPE_ENT      memType;
    GT_U32                         offset;
    GT_U32                         dataPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    memType = (CPSS_DIAG_PP_MEM_TYPE_ENT)inArgs[1];
    offset = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = pg_wrap_cpssDxChDiagMemRead(devNum, memType, offset, &dataPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", dataPtr);
    return CMD_OK;
}


/**
* @internal wrCpssDxChDiagRegWrite function
* @endinternal
*
* @brief   Performs single 32 bit data write to one of the PP PCI configuration or
*         registers.
*
* @note   APPLICABLE DEVICES:      All DX CH devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note The function may be called before Phase 1 initialization.
*
*/
static CMD_STATUS wrCpssDxChDiagRegWrite

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                          result;

    GT_U32                             baseAddr;
    CPSS_PP_INTERFACE_CHANNEL_ENT      ifChannel;
    CPSS_DIAG_PP_REG_TYPE_ENT          regType;
    GT_U32                             offset;
    GT_U32                             data;
    GT_BOOL                            doByteSwap;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    baseAddr = (GT_U32)inArgs[0];
    ifChannel = (CPSS_PP_INTERFACE_CHANNEL_ENT)inArgs[1];
    regType = (CPSS_DIAG_PP_REG_TYPE_ENT)inArgs[2];
    offset = (GT_U32)inArgs[3];
    data = (GT_U32)inArgs[4];
    doByteSwap = (GT_BOOL)inArgs[5];

    /* call cpss api function */
    result = cpssDxChDiagRegWrite(baseAddr, ifChannel, regType,
                                      offset, data, doByteSwap);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChDiagRegRead function
* @endinternal
*
* @brief   Performs single 32 bit data read from one of the PP PCI configuration or
*         registers.
*
* @note   APPLICABLE DEVICES:      All DX CH devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note The function may be called before Phase 1 initialization.
*
*/
static CMD_STATUS wrCpssDxChDiagRegRead

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                          result;

    GT_U32                             baseAddr;
    CPSS_PP_INTERFACE_CHANNEL_ENT      ifChannel;
    CPSS_DIAG_PP_REG_TYPE_ENT          regType;
    GT_U32                             offset;
    GT_U32                             dataPtr;
    GT_BOOL                            doByteSwap;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    baseAddr = (GT_U32)inArgs[0];
    ifChannel = (CPSS_PP_INTERFACE_CHANNEL_ENT)inArgs[1];
    regType = (CPSS_DIAG_PP_REG_TYPE_ENT)inArgs[2];
    offset = (GT_U32)inArgs[3];
    doByteSwap = (GT_BOOL)inArgs[4];

    /* call cpss api function */
    result = cpssDxChDiagRegRead(baseAddr, ifChannel, regType,
                                      offset, &dataPtr, doByteSwap);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", dataPtr);
    return CMD_OK;
}


/**
* @internal wrCpssDxChDiagPhyRegWrite function
* @endinternal
*
* @brief   Performs single 32 bit data write to one of the PHY registers.
*
* @note   APPLICABLE DEVICES:      All DX CH devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note The function may be called before Phase 1 initialization.
*
*/
static CMD_STATUS wrCpssDxChDiagPhyRegWrite

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                          result;

    GT_U32                             baseAddr;
    CPSS_PP_INTERFACE_CHANNEL_ENT      ifChannel;
    GT_U32                             smiRegOffset;
    GT_U32                             phyAddr;
    GT_U32                             offset;
    GT_U32                             data;
    GT_BOOL                            doByteSwap;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    baseAddr = (GT_U32)inArgs[0];
    ifChannel = (CPSS_PP_INTERFACE_CHANNEL_ENT)inArgs[1];
    smiRegOffset = (GT_U32)inArgs[2];
    phyAddr = (GT_U32)inArgs[3];
    offset = (GT_U32)inArgs[4];
    data = (GT_U32)inArgs[5];
    doByteSwap = (GT_BOOL)inArgs[6];

    /* call cpss api function */
    result = cpssDxChDiagPhyRegWrite(baseAddr, ifChannel, smiRegOffset,
                                     phyAddr, offset, data, doByteSwap);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChDiagPhyRegRead function
* @endinternal
*
* @brief   Performs single 32 bit data read from one of the PHY registers.
*
* @note   APPLICABLE DEVICES:      All DX CH devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note The function may be called before Phase 1 initialization.
*
*/
static CMD_STATUS wrCpssDxChDiagPhyRegRead

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                          result;

    GT_U32                             baseAddr;
    CPSS_PP_INTERFACE_CHANNEL_ENT      ifChannel;
    GT_U32                             smiRegOffset;
    GT_U32                             phyAddr;
    GT_U32                             offset;
    GT_U32                             dataPtr;
    GT_BOOL                            doByteSwap;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    baseAddr = (GT_U32)inArgs[0];
    ifChannel = (CPSS_PP_INTERFACE_CHANNEL_ENT)inArgs[1];
    smiRegOffset = (GT_U32)inArgs[2];
    phyAddr = (GT_U32)inArgs[3];
    offset = (GT_U32)inArgs[4];
    doByteSwap = (GT_BOOL)inArgs[5];

    /* call cpss api function */
    result = cpssDxChDiagPhyRegRead(baseAddr, ifChannel, smiRegOffset,
                                phyAddr, offset, &dataPtr, doByteSwap);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", dataPtr);
    return CMD_OK;
}


/**
* @internal wrCpssDxChDiagRegsNumGet function
* @endinternal
*
* @brief   Gets the number of registers for the PP.
*         Used to allocate memory for cpssDxChDiagRegsDump.
*
* @note   APPLICABLE DEVICES:      All DX CH devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note The function may be called after Phase 1 initialization.
*
*/
static CMD_STATUS wrCpssDxChDiagRegsNumGet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                          result;

    GT_U8                              devNum;
    GT_U32                             regsNumPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = pg_wrap_cpssDxChDiagRegsNumGet(devNum, &regsNumPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", regsNumPtr);
    return CMD_OK;
}


/**
* @internal wrCpssDxChDiagRegsDump function
* @endinternal
*
* @brief   Dumps the device register addresses and values.
*
* @note   APPLICABLE DEVICES:      All DX CH devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note The function may be called after Phase 1 initialization.
*       To dump all the registers the user may call cpssDxChDiagRegsNumGet in
*       order to get the number of registers of the PP.
*
*/
static CMD_STATUS wrCpssDxChDiagRegsDump

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_U32      offset;
    GT_U32      regsNum;
    GT_U32      *regAddrPtr;
    GT_U32      *regDataPtr;
    GT_U32      tmpRegsNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    regsNum = tmpRegsNum= (GT_U32)inArgs[1];
    offset = (GT_U32)inArgs[2];

    regAddrPtr = cmdOsMalloc(regsNum * sizeof(GT_U32));
    if (regAddrPtr == NULL)
    {
        return CMD_AGENT_ERROR;
    }

    regDataPtr = cmdOsMalloc(regsNum * sizeof(GT_U32));
    if (regDataPtr == NULL)
    {
        cmdOsFree(regAddrPtr);
        return CMD_AGENT_ERROR;
    }

    /* call cpss api function */
    result = pg_wrap_cpssDxChDiagRegsDump(devNum, &tmpRegsNum, offset,
                                          regAddrPtr, regDataPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d", regsNum, regAddrPtr[(regsNum - 1)],
                 regDataPtr[(regsNum - 1)]);

    cmdOsFree(regAddrPtr);
    cmdOsFree(regDataPtr);
    return CMD_OK;
}


GT_STATUS wrCpssDxChDiagRegsPrint(int devNum)
{
    GT_U32 *regAddrPtr, *regDataPtr; /* pointers to arrays of reg addresses and data*/
    GT_U32 regNum, i; /* number of regs */
    GT_STATUS       result;

    /* call cpss api function */
    result = cpssDxChDiagRegsNumGet((GT_U8)devNum, &regNum);
    if(result != GT_OK)
    {
        return (int)result;
    }
    cmdOsPrintf("registers number %d\n\n", regNum);
    regAddrPtr = cmdOsMalloc(regNum * sizeof(GT_U32));
    regDataPtr = cmdOsMalloc(regNum * sizeof(GT_U32));


    result = cpssDxChDiagRegsDump((GT_U8)devNum, &regNum, 0, regAddrPtr, regDataPtr);


    if(result != GT_OK)
    {
        return result;
    }

    cmdOsPrintf("registers number %d\n\n", regNum);
    for(i = 0; i < regNum; i++)
    {
        cmdOsPrintf("register addr 0x%08X value 0x%08X\n", regAddrPtr[i], regDataPtr[i]);
    }

    cmdOsFree(regAddrPtr);
    cmdOsFree(regDataPtr);

    return GT_OK;
}

GT_STATUS wrCpssDxChDiagResetAndInitControllerRegsPrint(int devNum)
{
    GT_U32 *regAddrPtr, *regDataPtr; /* pointers to arrays of reg addresses and data*/
    GT_U32 regNum, i; /* number of regs */
    GT_STATUS       result;

    /* call cpss api function */
    result = cpssDxChDiagResetAndInitControllerRegsNumGet((GT_U8)devNum, &regNum);
    if(result != GT_OK)
    {
        return result;
    }
    cmdOsPrintf("registers number %d\n\n", regNum);
    regAddrPtr = cmdOsMalloc(regNum * sizeof(GT_U32));
    regDataPtr = cmdOsMalloc(regNum * sizeof(GT_U32));


    result = cpssDxChDiagResetAndInitControllerRegsDump((GT_U8)devNum, &regNum, 0, regAddrPtr, regDataPtr);


    if(result != GT_OK)
    {
        return result;
    }

    cmdOsPrintf("registers number %d\n\n", regNum);
    for(i = 0; i < regNum; i++)
    {
        cmdOsPrintf("register addr 0x%08X value 0x%08X\n", regAddrPtr[i], regDataPtr[i]);
    }

    cmdOsFree(regAddrPtr);
    cmdOsFree(regDataPtr);

    return GT_OK;
}


/**
* @internal wrCpssDxChDiagRegTest function
* @endinternal
*
* @brief   Tests the device read/write ability of a specific register.
*
* @note   APPLICABLE DEVICES:      All DX CH devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note The function may be called after Phase 1 initialization.
*       After the register is tested, the original value prior to the test is
*       restored.
*
*/
static CMD_STATUS wrCpssDxChDiagRegTest

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                          result;

    GT_U8                              devNum;
    GT_U32                             regAddr;
    GT_U32                             regMask;
    CPSS_DIAG_TEST_PROFILE_ENT         profile;
    GT_BOOL                            testStatusPtr;
    GT_U32                             readValPtr;
    GT_U32                             writeValPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    regAddr = (GT_U32)inArgs[1];
    regMask = (GT_U32)inArgs[2];
    profile = (CPSS_DIAG_TEST_PROFILE_ENT)inArgs[3];

    /* call cpss api function */
    result = cpssDxChDiagRegTest(devNum, regAddr, regMask, profile,
                         &testStatusPtr, &readValPtr, &writeValPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d", testStatusPtr, readValPtr,
                                                          writeValPtr);
    return CMD_OK;
}


/**
* @internal wrCpssDxChDiagAllRegTest function
* @endinternal
*
* @brief   Tests the device read/write ability of all the registers.
*
* @note   APPLICABLE DEVICES:      All DX CH devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note The test is done by invoking cpssDxChDiagRegTest in loop for all the
*       diagnostics registers and for all the patterns.
*       After each register is tested, the original value prior to the test is
*       restored.
*       The function may be called after Phase 1 initialization.
*
*/
static CMD_STATUS wrCpssDxChDiagAllRegTest

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                          result;

    GT_U8                              devNum;
    GT_BOOL                            testStatusPtr;
    GT_U32                             badRegPtr;
    GT_U32                             readValPtr;
    GT_U32                             writeValPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChDiagAllRegTest(devNum, &testStatusPtr, &badRegPtr,
                                             &readValPtr, &writeValPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d", testStatusPtr, badRegPtr,
                                               readValPtr, writeValPtr);
    return CMD_OK;
}
/**
* @internal wrCpssDxChDiagPrbsPortTransmitModeSet function
* @endinternal
*
* @brief   Set transmit mode for specified port.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for Tri-speed ports of all DxCh devices
*       and XAUI/HGS ports of DxCh3 and above devices.
*       CPU port doesn't support the transmit mode.
*
*/
static CMD_STATUS wrCpssDxChDiagPrbsPortTransmitModeSet
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
    GT_U32 laneNum;
    CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT mode;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    laneNum = (GT_U32)inArgs[2];
    mode = (CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT)inArgs[3];

    /* call cpss api function */
    result = cpssDxChDiagPrbsPortTransmitModeSet(devNum, portNum, laneNum, mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChDiagPrbsPortTransmitModeGet function
* @endinternal
*
* @brief   Get transmit mode for specified port.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for Tri-speed ports of all DxCh devices
*       and XAUI/HGS ports of DxCh3 and above devices.
*       CPU port doesn't support the transmit mode.
*
*/
static CMD_STATUS wrCpssDxChDiagPrbsPortTransmitModeGet
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
    GT_U32 laneNum;
    CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT mode;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    laneNum = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChDiagPrbsPortTransmitModeGet(devNum, portNum, laneNum, &mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mode);

    return CMD_OK;
}

/**
* @internal wrCpssDxChDiagPrbsPortGenerateEnableSet function
* @endinternal
*
* @brief   Enable/Disable PRBS (Pseudo Random Bit Generator) pattern generation
*         per Port and per lane.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for Tri-speed ports of all DxCh devices
*       and XAUI/HGS ports of DxCh3 and above devices.
*       When operating, enable the PRBS checker before the generator.
*
*/
static CMD_STATUS wrCpssDxChDiagPrbsPortGenerateEnableSet
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
    GT_U32 laneNum;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    laneNum = (GT_U32)inArgs[2];
    enable = (GT_BOOL)inArgs[3];

    /* call cpss api function */
    result = cpssDxChDiagPrbsPortGenerateEnableSet(devNum, portNum, laneNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChDiagPrbsPortGenerateEnableGet function
* @endinternal
*
* @brief   Get the status of PRBS (Pseudo Random Bit Generator) pattern generation
*         per port and per lane.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for Tri-speed ports of all DxCh devices
*       and XAUI/HGS ports of DxCh3 and above devices.
*
*/
static CMD_STATUS wrCpssDxChDiagPrbsPortGenerateEnableGet
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
    GT_U32 laneNum;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    laneNum = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChDiagPrbsPortGenerateEnableGet(devNum, portNum, laneNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChDiagPrbsPortCheckEnableSet function
* @endinternal
*
* @brief   Enable/Disable PRBS (Pseudo Random Bit Generator) checker per port and
*         per lane.
*         When the checker is enabled, it seeks to lock onto the incoming bit
*         stream, and once this is achieved the PRBS checker starts counting the
*         number of bit errors. Tne number of errors can be retrieved by
*         cpssDxChDiagPrbsGigPortStatusGet API.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for Tri-speed ports of all DxCh devices
*       and XAUI/HGS ports of DxCh3 and above devices.
*       When operating, enable the PRBS checker before the generator.
*
*/
static CMD_STATUS wrCpssDxChDiagPrbsPortCheckEnableSet
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
    GT_U32 laneNum;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    laneNum = (GT_U32)inArgs[2];
    enable = (GT_BOOL)inArgs[3];

    /* call cpss api function */
    result = cpssDxChDiagPrbsPortCheckEnableSet(devNum, portNum, laneNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChDiagPrbsPortCheckEnableGet function
* @endinternal
*
* @brief   Get the status (enabled or disabled) of (Pseudo Random Bit Generator)
*         checker per port and per lane.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for Tri-speed ports of all DxCh devices
*       and XAUI/HGS ports of DxCh3 and above devices.
*
*/
static CMD_STATUS wrCpssDxChDiagPrbsPortCheckEnableGet
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
    GT_U32 laneNum;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    laneNum = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChDiagPrbsPortCheckEnableGet(devNum, portNum, laneNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChDiagPrbsPortCheckReadyGet function
* @endinternal
*
* @brief   Get the PRBS checker ready status.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for the Tri-speed ports.
*       The Check ready status indicates that the PRBS checker has completed
*       the initialization phase. The PRBS generator at the transmit side may
*       be enabled.
*
*/
static CMD_STATUS wrCpssDxChDiagPrbsPortCheckReadyGet
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
    GT_BOOL isReady;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);


    /* call cpss api function */
    result = cpssDxChDiagPrbsPortCheckReadyGet(devNum, portNum, &isReady);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", isReady);

    return CMD_OK;
}

/**
* @internal wrCpssDxChDiagPrbsPortStatusGet function
* @endinternal
*
* @brief   Get PRBS (Pseudo Random Bit Generator) Error Counter and Checker Locked
*         status per port and per lane.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note PRBS Error counter is cleared on read.
*       Applicable only for Tri-speed ports of all DxCh devices
*       and XAUI/HGS ports of DxCh3 and above devices.
*
*/
static CMD_STATUS wrCpssDxChDiagPrbsPortStatusGet
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
    GT_U32 laneNum;
    GT_BOOL checkerLocked;
    GT_U32 errorCntr;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    laneNum = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChDiagPrbsPortStatusGet(devNum, portNum, laneNum, &checkerLocked, &errorCntr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", checkerLocked, errorCntr);

    return CMD_OK;
}

/**
* @internal wrCpssDxChDiagPrbsCyclicDataSet function
* @endinternal
*
* @brief   Set cylic data for transmition. See cpssDxChDiagPrbsPortTransmitModeSet.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for the XAUI/HGS ports.
*
*/
static CMD_STATUS wrCpssDxChDiagPrbsCyclicDataSet
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
    GT_U32 laneNumber;
    GT_U32 cyclicDataArr[4];


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    laneNumber = (GT_U32)inArgs[2];
    cyclicDataArr[0]=(GT_U32)inArgs[3];
    cyclicDataArr[1]=(GT_U32)inArgs[4];
    cyclicDataArr[2]=(GT_U32)inArgs[5];
    cyclicDataArr[3]=(GT_U32)inArgs[6];

    /* call cpss api function */
    result = cpssDxChDiagPrbsCyclicDataSet(devNum, portNum, laneNumber, cyclicDataArr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChDiagPrbsCyclicDataGet function
* @endinternal
*
* @brief   Get cylic data for transmition. See cpssDxChDiagPrbsPortTransmitModeSet.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for the XAUI/HGS ports.
*
*/
static CMD_STATUS wrCpssDxChDiagPrbsCyclicDataGet
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
    GT_U32 laneNumber;
    GT_U32 cyclicDataArr[4];


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    laneNumber = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChDiagPrbsCyclicDataGet(devNum, portNum, laneNumber, cyclicDataArr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d",cyclicDataArr[0],cyclicDataArr[1],cyclicDataArr[2],
                                             cyclicDataArr[3]);

    return CMD_OK;
}
/**
* @internal wrCpssDxChDiagPrbsSerdesTestEnableSet function
* @endinternal
*
* @brief   Enable/Disable SERDES PRBS (Pseudo Random Bit Generator) test mode.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number, device or lane number
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Transmit mode should be set before enabling test mode.
*       See test cpssDxChDiagPrbsSerdesTransmitModeSet.
*
*/
static CMD_STATUS wrCpssDxChDiagPrbsSerdesTestEnableSet
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
    GT_U32 laneNum;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    laneNum = (GT_U32)inArgs[2];
    enable = (GT_BOOL)inArgs[3];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChDiagPrbsSerdesTestEnableSet(devNum, portNum, laneNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");



    return CMD_OK;
}

/**
* @internal wrCpssDxChDiagPrbsSerdesTestEnableGet function
* @endinternal
*
* @brief   Get the status of PRBS (Pseudo Random Bit Generator) test mode.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number, device or lane number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChDiagPrbsSerdesTestEnableGet
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
    GT_U32 laneNum;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    laneNum = (GT_U32)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChDiagPrbsSerdesTestEnableGet(devNum, portNum, laneNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);



    return CMD_OK;
}

/**
* @internal wrCpssDxChDiagPrbsSerdesTransmitModeSet function
* @endinternal
*
* @brief   Set transmit mode for SERDES PRBS on specified port/lane.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device or lane number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type
*                                       or unsupported transmit mode.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChDiagPrbsSerdesTransmitModeSet
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
    GT_U32 laneNum;
    CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT mode;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    laneNum = (GT_U32)inArgs[2];
    mode = (CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT)inArgs[3];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChDiagPrbsSerdesTransmitModeSet(devNum, portNum, laneNum, mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");



    return CMD_OK;
}

/**
* @internal wrCpssDxChDiagPrbsSerdesTransmitModeGet function
* @endinternal
*
* @brief   Get transmit mode for SERDES PRBS on specified port/lane.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device or lane number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on unkonown transmit mode
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChDiagPrbsSerdesTransmitModeGet
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
    GT_U32 laneNum;
    CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT mode;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    laneNum = (GT_U32)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChDiagPrbsSerdesTransmitModeGet(devNum, portNum, laneNum, &mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mode);



    return CMD_OK;
}

/**
* @internal wrCpssDxChDiagPrbsSerdesStatusGet function
* @endinternal
*
* @brief   Get SERDES PRBS (Pseudo Random Bit Generator) pattern detector state,
*         error counter and pattern counter.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number, device or lane number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChDiagPrbsSerdesStatusGet
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
    GT_U32 laneNum;
    GT_BOOL locked;
    GT_U32 errorCntr;
    GT_U64 patternCntr;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    laneNum = (GT_U32)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChDiagPrbsSerdesStatusGet(devNum, portNum, laneNum, &locked, &errorCntr, &patternCntr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    galtisOutput(outArgs, result, "%d%d%d%d", locked, errorCntr,patternCntr.l[0],patternCntr.l[1]);



    return CMD_OK;
}

/**
* @internal wrCpssDxChDiagMemoryBistBlockStatusGet function
* @endinternal
*
* @brief   Gets redundancy block status.
*
* @note   APPLICABLE DEVICES:      DxCh3 only
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*
* @note Each redundancy block contains 256 rows. Block 0: rows 0 to 255,
*       block 1: raws 256 to 511 ...
*       One raw replacement is possible though the full redundancy block
*       of 256 rows.
*
*/
static CMD_STATUS wrCpssDxChDiagMemoryBistBlockStatusGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DIAG_PP_MEM_BIST_TYPE_ENT memBistType;
    GT_U32 blockIndex;
    GT_BOOL blockFixed;
    GT_U32 replacedIndex;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    memBistType = (CPSS_DIAG_PP_MEM_BIST_TYPE_ENT)inArgs[1];
    blockIndex = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChDiagMemoryBistBlockStatusGet(devNum, memBistType,
                                blockIndex, &blockFixed, &replacedIndex);

    if(result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", blockFixed, replacedIndex);

    return CMD_OK;
}

/**
* @internal wrCpssDxChDiagMemoryBistsRun function
* @endinternal
*
* @brief   Runs BIST (Built-in self-test) on specified memory.
*
* @note   APPLICABLE DEVICES:      DxCh3 only
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_UNFIXABLE_BIST_ERROR  - on unfixable problem in the TCAM
*
* @note 1. This function should not be called under traffic.
*       2. Bist destroys the content of the memory. If clearMemoryAfterTest set
*       function will clear the content of the memory after the test.
*       Application responsible to restore the content after the function
*       completion.
*
*/
static CMD_STATUS wrCpssDxChDiagMemoryBistsRun
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DIAG_PP_MEM_BIST_TYPE_ENT memBistType;
    GT_U32 timeOut;
    GT_BOOL clearMemoryAfterTest;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    memBistType = (CPSS_DIAG_PP_MEM_BIST_TYPE_ENT)inArgs[1];
    timeOut = (GT_U32)inArgs[2];
    clearMemoryAfterTest = (GT_BOOL)inArgs[3];

    /* call cpss api function */
    result = cpssDxChDiagMemoryBistsRun(devNum, memBistType, CPSS_DIAG_PP_MEM_BIST_PURE_MEMORY_TEST_E,
                                        timeOut, clearMemoryAfterTest, NULL, NULL);

    if(result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChDiagMemoryBistsRun1 function
* @endinternal
*
* @brief   Runs BIST (Built-in self-test) on specified memory.
*
* @note   APPLICABLE DEVICES:      Lion
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_UNFIXABLE_BIST_ERROR  - on unfixable problem in the TCAM
*
* @note 1. This function should not be called under traffic.
*       2. Bist destroys the content of the memory. If clearMemoryAfterTest set
*       function will clear the content of the memory after the test.
*       Application responsible to restore the content after the function
*       completion.
*
*/
static CMD_STATUS wrCpssDxChDiagMemoryBistsRun1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DIAG_PP_MEM_BIST_TYPE_ENT memBistType;
    CPSS_DIAG_PP_MEM_BIST_TEST_TYPE_ENT bistTestType;
    GT_U32 timeOut;
    GT_BOOL clearMemoryAfterTest;
    GT_U32 testsToRunBmp[1];
    GT_U32 testsResultBmp[1];

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_U8)inArgs[0];
    memBistType     = (CPSS_DIAG_PP_MEM_BIST_TYPE_ENT)inArgs[1];
    bistTestType    = (CPSS_DIAG_PP_MEM_BIST_TEST_TYPE_ENT)inArgs[2];
    timeOut         = (GT_U32)inArgs[3];
    clearMemoryAfterTest = (GT_BOOL)inArgs[4];
    testsToRunBmp[0]   = (GT_BOOL)inArgs[5];

    /* call cpss api function */
    result = cpssDxChDiagMemoryBistsRun(devNum, memBistType, bistTestType,
                                        timeOut, clearMemoryAfterTest,
                                        testsToRunBmp, testsResultBmp);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "0x%x", testsResultBmp[0]);

    return CMD_OK;
}

/**
* @internal wrCpssDxChDiagDeviceTemperatureSensorsSelectSet function
* @endinternal
*
* @brief   Select Temperature Sensors.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum, sensorType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChDiagDeviceTemperatureSensorsSelectSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_ENT  sensorType;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    sensorType = (CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChDiagDeviceTemperatureSensorsSelectSet(devNum, sensorType);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**
* @internal wrCpssDxChDiagDeviceTemperatureSensorsSelectGet function
* @endinternal
*
* @brief   Get Temperature Sensors Select.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChDiagDeviceTemperatureSensorsSelectGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_ENT  sensorType;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChDiagDeviceTemperatureSensorsSelectGet(
        devNum, &sensorType);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", sensorType);

    return CMD_OK;
}

/**
* @internal wrCpssDxChDiagDeviceTemperatureThresholdSet function
* @endinternal
*
* @brief   Set Threshold for Interrupt. If Temperature Sensors is equal or above
*         the threshold, interrupt is activated.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_OUT_OF_RANGE          - on wrong thresholdValue
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChDiagDeviceTemperatureThresholdSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_32  thresholdValue;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    thresholdValue = (GT_32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChDiagDeviceTemperatureThresholdSet(devNum, thresholdValue);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");



    return CMD_OK;
}


/**
* @internal wrCpssDxChDiagDeviceTemperatureThresholdGet function
* @endinternal
*
* @brief   Get Threshold for Interrupt. If Temperature Sensors is equal or above
*         the threshold, interrupt is activated.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChDiagDeviceTemperatureThresholdGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_32    thresholdValue;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChDiagDeviceTemperatureThresholdGet(
        devNum, &thresholdValue);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", thresholdValue);

    return CMD_OK;
}


/**
* @internal wrCpssDxChDiagDeviceVoltageGet function
* @endinternal
*
* @brief   Gets the PP sensor voltage.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - null pointer
*/
static CMD_STATUS wrCpssDxChDiagDeviceVoltageGet
(
        IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
        IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
        IN  GT_32 numFields,
        OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    IN  GT_U32    sensorNum;
    GT_U32 voltage_milivolts;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    sensorNum =  (GT_U32)inArgs[1];
    voltage_milivolts = 0;

    /* call cpss api function */
    result = cpssDxChDiagDeviceVoltageGet(devNum, sensorNum, &voltage_milivolts);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", voltage_milivolts);

    return CMD_OK;
}

/**
* @internal wrCpssDxChDiagDeviceTemperatureGet function
* @endinternal
*
* @brief   Gets the PP temperature.
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChDiagDeviceTemperatureGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_32     temperature;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    temperature = 0;

    /* call cpss api function */
    result = cpssDxChDiagDeviceTemperatureGet(
        devNum, &temperature);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", temperature);

    return CMD_OK;
}


/**
* @internal wrCpssDxChDiagDeviceTemperatureKelvinGet function
* @endinternal
*
* @brief   Gets the PP temperature.
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChDiagDeviceTemperatureKelvinGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_32     temperature;
    GT_U32    temperatureKelvin;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    temperature = 0;

    /* call cpss api function */
    result = cpssDxChDiagDeviceTemperatureGet(
        devNum, &temperature);

    temperatureKelvin = temperature + 273;

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", temperatureKelvin);

    return CMD_OK;
}

/*******************************************************************************
* cpssDxChDiagSerdesTuningRxTune Get
*
* DESCRIPTION:
*       Gets serdes RX tuning values
*
* APPLICABLE DEVICES:  Xcat and above
*
* INPUTS:
*       devNum  - device number
*       portNum - global port number
*       laneBmp - bitmap of serdes lanes of portNum, where to generate PRBS signal
*       mode    - type of PRBS signal (not relevant if enable == GT_FALSE) [5-8]
*                 Now supported:
*                               CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS7_E
*                               CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS15_E
*                               CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS23_E
*                               CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS31_E
*       prbsTime - wait time till PRBS test finish (set 0 for CPSS default)
*       optMode  - optimization algorithm mode
*       dbgPrintEn  -   GT_TRUE - enable report print during run,
*                       GT_FALSE - disable report print.
*
* OUTPUTS:
*       optResultArrPtr - array of tuning result values
*
* RETURNS:
*       GT_OK           - on success
*       GT_HW_ERROR - on hardware error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
/* for now maximum number of lanes in Lion XLG */
#define DXCH_MAX_SERDES_LANES_PER_PORT 8

static GT_U32      portLaneArrLength;  /* length of (port,laneNum) array */
static CPSS_DXCH_DIAG_SERDES_TUNE_PORT_LANE_STC portLaneArr
                            [DXCH_MAX_SERDES_LANES_PER_PORT];
        /* array of (port,laneNum) pairs, where to run the test */
static GT_U32      laneNumber;          /* iterator */
static CPSS_DXCH_PORT_SERDES_TUNE_STC optResultArr
                    [DXCH_MAX_SERDES_LANES_PER_PORT];
                                    /* array of tuning result values */

static CMD_STATUS wrCpssDxChDiagSerdesTuningRxTuneGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8                            devNum;
    GT_PHYSICAL_PORT_NUM             portNum;
    GT_U32                           laneBmp;
    CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT mode;
    GT_U32                           prbsTime;
    GT_BOOL                          dbgPrintEn;
    CPSS_DXCH_DIAG_SERDES_OPTIMIZE_MODE_ENT     optMode;

    GT_STATUS   rc;         /* return code */
    GT_U32      i,j;          /* iterator */

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum      = (GT_U8)inArgs[0];
    portNum     = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    laneBmp     = (GT_U32)inArgs[2];
    mode        = (CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT)inArgs[3];
    prbsTime    = (GT_U32)inArgs[4];
    dbgPrintEn  = (GT_BOOL)inArgs[5];
    optMode     = (CPSS_DXCH_DIAG_SERDES_OPTIMIZE_MODE_ENT)inArgs[6];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    rc = cpssDxChDiagSerdesTuningTracePrintEnable(devNum,dbgPrintEn);
    if (rc != GT_OK)
    {
        galtisOutput(outArgs, rc, "cpssDxChDiagSerdesTuningTracePrintEnable");
        return CMD_OK;
    }

    rc = cpssDxChDiagSerdesTuningSystemInit(devNum);
    if (rc != GT_OK)
    {
        galtisOutput(outArgs, rc, "cpssDxChDiagSerdesTuningSystemInit");
        return CMD_OK;
    }

    for(i = 0, portLaneArrLength=0; i < 32; i++)
    {
        if(laneBmp & (1<<i))
        {
            portLaneArrLength++;
        }
    }

    for(i = 0,j=0; i < 32; i++)
    {
        if(laneBmp & (1<<i))
        {
            portLaneArr[j].portNum = portNum;
            portLaneArr[j++].laneNum = i;
        }
    }

    rc = cpssDxChDiagSerdesTuningRxTune(devNum, portLaneArr, portLaneArrLength,
                                        mode, prbsTime, optMode, optResultArr);
    if (rc != GT_OK)
    {
        galtisOutput(outArgs, rc, "cpssDxChDiagSerdesTuningRxTune");
        return CMD_OK;
    }

    inFields[0] = portLaneArr[0].laneNum;
    inFields[1] = optResultArr[0].dfe;
    inFields[2] = optResultArr[0].ffeC;
    inFields[3] = optResultArr[0].ffeR;
    inFields[4] = optResultArr[0].sampler;
    inFields[5] = optResultArr[0].sqlch;
    laneNumber = 1;

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d", inFields[0], inFields[1], inFields[2],
                    inFields[3], inFields[4], inFields[5]);
    galtisOutput(outArgs, rc, "%f");

    rc = cpssDxChDiagSerdesTuningSystemClose(devNum);
    if (rc != GT_OK)
    {
        galtisOutput(outArgs, rc, "cpssDxChDiagSerdesTuningSystemClose");
        return CMD_OK;
    }

    return CMD_OK;
}

/**
* @internal wrCpssDxChDiagSerdesTuningRxTuneGetNext function
* @endinternal
*
* @brief   This function is called for ending table printing.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
*/
static CMD_STATUS wrCpssDxChDiagSerdesTuningRxTuneGetNext

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    if(laneNumber >= portLaneArrLength)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
    }
    else
    {
        inFields[0] = portLaneArr[laneNumber].laneNum;
        inFields[1] = optResultArr[laneNumber].dfe;
        inFields[2] = optResultArr[laneNumber].ffeC;
        inFields[3] = optResultArr[laneNumber].ffeR;
        inFields[4] = optResultArr[laneNumber].sampler;
        inFields[5] = optResultArr[laneNumber].sqlch;
        laneNumber++;

        /* pack and output table fields */
        fieldOutput("%d%d%d%d%d%d", inFields[0], inFields[1], inFields[2],
                        inFields[3], inFields[4], inFields[5]);
        galtisOutput(outArgs, GT_OK, "%f");
    }

    return CMD_OK;
}

static GT_U32   wrDxChPortNum = 0;

/**
* @internal wrCpssDxChDiagPacketGeneratorConnectGetNext function
* @endinternal
*
* @brief   Get the connect status of specified port.
*         Get packet generator's configurations.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChDiagPacketGeneratorConnectGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8                                 devNum;
    GT_BOOL                               connect;
    CPSS_DXCH_DIAG_PG_CONFIGURATIONS_STC  config;
    GT_STATUS  result = GT_OK;
    GT_BOOL showOnlyConnected;
    GT_PHYSICAL_PORT_NUM                  tempPortNum;

    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cpssOsMemSet(&config, 0, sizeof(CPSS_DXCH_DIAG_PG_CONFIGURATIONS_STC));

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    showOnlyConnected = (GT_BOOL)inArgs[1];

    /* add port convertion*/
    while(1)
    {
        if(wrDxChPortNum == CPSS_MAX_PORTS_NUM_CNS)
        {
            /* no ports */
            galtisOutput(outArgs, GT_OK, "%d", -1);
            return CMD_OK;
        }

        if (!PRV_CPSS_IS_DEV_EXISTS_MAC(devNum) || PRV_CPSS_PHY_PORT_IS_EXIST_MAC(devNum, wrDxChPortNum))
        {
            tempPortNum = (GT_PHYSICAL_PORT_NUM)wrDxChPortNum;

            /* Override Device and Port */
            CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, tempPortNum);

            /* get port's attributes */
            result = cpssDxChDiagPacketGeneratorConnectGet(devNum,tempPortNum,&connect,&config);
            if ((result == GT_NOT_INITIALIZED) || ((showOnlyConnected == GT_TRUE) && (connect == GT_FALSE)))
            {
                wrDxChPortNum++;
                continue;
            }
            break;
        }
        else
        {
            wrDxChPortNum++;
        }
    }

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    inFields[0] = wrDxChPortNum;
    inFields[1] = connect;
    inFields[3] = config.macDaIncrementEnable;
    inFields[5] = config.vlanTagEnable;
    inFields[6] = config.vpt;
    inFields[7] = config.cfi;
    inFields[8] = config.vid;
    inFields[9] = config.etherType;
    inFields[10] = config.payloadType;
    inFields[12] = config.packetLengthType;
    inFields[13] = config.packetLength;
    inFields[14] = config.undersizeEnable;
    inFields[15] = config.transmitMode;
    inFields[16] = config.packetCount;
    inFields[17] = config.packetCountMultiplier;
    inFields[18] = config.ipg;

    /* pack and output table fields */
    fieldOutput("%d%d%6b%d%6b%d%d%d%d%d%d%8b%d%d%d%d%d%d%d",
                          inFields[0],  inFields[1],  config.macDa.arEther,  inFields[3],
                          config.macSa.arEther, inFields[5], inFields[6],
                          inFields[7],  inFields[8],  inFields[9],
                          inFields[10], config.cyclicPatternArr, inFields[12],
                          inFields[13], inFields[14], inFields[15],
                          inFields[16], inFields[17], inFields[18]);
    galtisOutput(outArgs, GT_OK, "%f");

    /* move to the next portNum */
    wrDxChPortNum++;
    return CMD_OK;

}

static CMD_STATUS wrCpssDxChDiagPacketGeneratorConnectGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    wrDxChPortNum = 0;
    return wrCpssDxChDiagPacketGeneratorConnectGetNext(inArgs,inFields,numFields,outArgs);

}



static CMD_STATUS wrCpssDxChDiagPacketGeneratorConnect_1GetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8                                 devNum;
    GT_BOOL                               connect;
    CPSS_DXCH_DIAG_PG_CONFIGURATIONS_STC  config;
    GT_STATUS  result = GT_OK;
    GT_BOOL showOnlyConnected;
    GT_PHYSICAL_PORT_NUM                  tempPortNum;

    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cpssOsMemSet(&config, 0, sizeof(CPSS_DXCH_DIAG_PG_CONFIGURATIONS_STC));

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    showOnlyConnected = (GT_BOOL)inArgs[1];

    /* add port convertion*/
    while(1)
    {
        if(wrDxChPortNum == CPSS_MAX_PORTS_NUM_CNS)
        {
            /* no ports */
            galtisOutput(outArgs, GT_OK, "%d", -1);
            return CMD_OK;
        }

        if (!PRV_CPSS_IS_DEV_EXISTS_MAC(devNum) || PRV_CPSS_PHY_PORT_IS_EXIST_MAC(devNum, wrDxChPortNum))
        {
            tempPortNum = (GT_PHYSICAL_PORT_NUM)wrDxChPortNum;

            /* Override Device and Port */
            CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, tempPortNum);

            /* get port's attributes */
            result = cpssDxChDiagPacketGeneratorConnectGet(devNum,tempPortNum,&connect,&config);
            if ((result == GT_NOT_INITIALIZED) || ((showOnlyConnected == GT_TRUE) && (connect == GT_FALSE)))
            {
                wrDxChPortNum++;
                continue;
            }
            break;
        }
        else
        {
            wrDxChPortNum++;
        }
    }

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    inFields[ 0] = wrDxChPortNum;
    inFields[ 1] = connect;
    inFields[ 3] = config.macDaIncrementEnable;
    inFields[ 4] = config.macDaIncrementLimit;
    inFields[ 6] = config.vlanTagEnable;
    inFields[ 7] = config.vpt;
    inFields[ 8] = config.cfi;
    inFields[ 9] = config.vid;
    inFields[10] = config.etherType;
    inFields[11] = config.packetLengthType;
    inFields[12] = config.packetLength;
    inFields[13] = config.undersizeEnable;
    inFields[14] = config.transmitMode;
    inFields[15] = config.packetCount;
    inFields[16] = config.packetCountMultiplier;
    inFields[17] = config.ipg;
    inFields[18] = config.interfaceSize;
    inFields[19] = config.payloadType;
    inFields[20] = 8;

    /* pack and output table fields */
               /* 1 2  3 4 5 6 7 8 91011121314151617181920 */
    fieldOutput("%d%d%6b%d%d%6b%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%8b",
                          inFields[0],  inFields[1],  
                          config.macDa.arEther,  inFields[3],inFields[4],       /* mac DA, increment enable, increment  limit */
                          config.macSa.arEther, 
                          inFields[6], inFields[7],   inFields[8],  inFields[9], /* vlan tag */
                          inFields[10],                                          /* ethernet type */
                          inFields[11], inFields[12], inFields[13],               /* packet length type, packet length, undersize enable */
                          inFields[14], inFields[15], inFields[16],               /* transmit mode, packet count, packet count multiplier */
                          inFields[17],                                           /* ipg */
                          inFields[18],                                           /* interface size */
                          inFields[19], inFields[20],config.cyclicPatternArr);

    galtisOutput(outArgs, GT_OK, "%f");

    /* move to the next portNum */
    wrDxChPortNum++;
    return CMD_OK;

}

static CMD_STATUS wrCpssDxChDiagPacketGeneratorConnect_1GetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    wrDxChPortNum = 0;
    return wrCpssDxChDiagPacketGeneratorConnect_1GetNext(inArgs,inFields,numFields,outArgs);

}


/**
* @internal wrCpssDxChDiagPacketGeneratorConnectSet function
* @endinternal
*
* @brief   Connect/Disconnect port to packet generator.
*         Set packet generator's configurations.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. In case packet generator connected to other port the function perfroms
*       the following:
*       - stops traffic.
*       - connects packet generator to new port
*       - overrides packet generator's configurations
*
*/
static CMD_STATUS wrCpssDxChDiagPacketGeneratorConnectSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                    result;

    GT_U8                                 devNum;
    GT_PHYSICAL_PORT_NUM                  portNum;
    GT_BOOL                               connect;
    CPSS_DXCH_DIAG_PG_CONFIGURATIONS_STC  config;
    GT_BYTE_ARRY cyclicBArr;

    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inFields[0];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    connect = (GT_BOOL)inFields[1];
    galtisMacAddr(&config.macDa, (GT_U8*)inFields[2]);
    config.macDaIncrementEnable = (GT_BOOL)inFields[3];
    config.macDaIncrementLimit  = 0;
    galtisMacAddr(&config.macSa, (GT_U8*)inFields[4]);
    config.vlanTagEnable = (GT_BOOL)inFields[5];
    config.vpt = (GT_U8)inFields[6];
    config.cfi = (GT_U8)inFields[7];
    config.vid = (GT_U16)inFields[8];
    config.etherType = (GT_U16)inFields[9];
    config.payloadType = (CPSS_DIAG_PG_PACKET_PAYLOAD_TYPE_ENT)inFields[10];
    galtisBArray(&cyclicBArr,(GT_U8*)inFields[11]);
    cmdOsMemCpy(config.cyclicPatternArr, cyclicBArr.data, 8);
    config.packetLengthType = (CPSS_DIAG_PG_PACKET_LENGTH_TYPE_ENT)inFields[12];
    config.packetLength = (GT_U32)inFields[13];
    config.undersizeEnable = (GT_BOOL)inFields[14];
    config.transmitMode = (CPSS_DIAG_PG_TRANSMIT_MODE_ENT)inFields[15];
    config.packetCount = (GT_U32)inFields[16];
    config.packetCountMultiplier = (CPSS_DIAG_PG_PACKET_COUNT_MULTIPLIER_ENT)inFields[17];
    config.ipg = (GT_U32)inFields[18];
    config.interfaceSize = CPSS_DIAG_PG_IF_SIZE_DEFAULT_E;

    /* call cpss api function */
    result = cpssDxChDiagPacketGeneratorConnectSet(devNum,portNum,connect,&config);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;

}


static CMD_STATUS wrCpssDxChDiagPacketGeneratorConnect_1Set
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                    result;

    GT_U8                                 devNum;
    GT_PHYSICAL_PORT_NUM                  portNum;
    GT_BOOL                               connect;
    CPSS_DXCH_DIAG_PG_CONFIGURATIONS_STC  config;
    GT_BYTE_ARRY cyclicBArr;

    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inFields[0];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    connect                      = (GT_BOOL)inFields[1];
    galtisMacAddr(&config.macDa,   (GT_U8*) inFields[2]);
    config.macDaIncrementEnable  = (GT_BOOL)inFields[3];
    config.macDaIncrementLimit   = (GT_U32) inFields[4];
    galtisMacAddr(&config.macSa,   (GT_U8*) inFields[5]);
    config.vlanTagEnable         = (GT_BOOL)inFields[6];
    config.vpt                   = (GT_U8)  inFields[7];
    config.cfi                   = (GT_U8)  inFields[8];
    config.vid                   = (GT_U16) inFields[9];
    config.etherType             = (GT_U16) inFields[10];
    config.packetLengthType      = (CPSS_DIAG_PG_PACKET_LENGTH_TYPE_ENT)inFields[11];
    config.packetLength          = (GT_U32) inFields[12];
    config.undersizeEnable       = (GT_BOOL)inFields[13];
    config.transmitMode          = (CPSS_DIAG_PG_TRANSMIT_MODE_ENT)inFields[14];
    config.packetCount           = (GT_U32) inFields[15];
    config.packetCountMultiplier = (CPSS_DIAG_PG_PACKET_COUNT_MULTIPLIER_ENT)inFields[16];
    config.ipg                   = (GT_U32) inFields[17];
    config.interfaceSize         = (CPSS_DIAG_PG_IF_SIZE_ENT)inFields[18]; 
    config.payloadType           = (CPSS_DIAG_PG_PACKET_PAYLOAD_TYPE_ENT)inFields[19];
    galtisBArray(&cyclicBArr,      (GT_U8*)inFields[21]);
    cmdOsMemCpy(config.cyclicPatternArr, cyclicBArr.data, 8);

    /* call cpss api function */
    result = cpssDxChDiagPacketGeneratorConnectSet(devNum,portNum,connect,&config);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;

}


/**
* @internal wrCpssDxChDiagPacketGeneratorTransmitEnable function
* @endinternal
*
* @brief   Enable/Disable(Start/Stop) transmission on specified port.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_STATE             - on port not connected to packet generator
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChDiagPacketGeneratorTransmitEnable
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_PHYSICAL_PORT_NUM     portNum;
    GT_BOOL   enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    enable = (GT_BOOL)inArgs[2];

    /* call cpss api function */
    result = cpssDxChDiagPacketGeneratorTransmitEnable(devNum, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChDiagPacketGeneratorBurstTransmitStatusGet function
* @endinternal
*
* @brief   Get burst transmission status on specified port.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on port not connected to packet generator
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Burst transmission status is clear on read.
*
*/
static CMD_STATUS wrCpssDxChDiagPacketGeneratorBurstTransmitStatusGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_PHYSICAL_PORT_NUM     portNum;
    GT_BOOL   burstTransmitDone;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChDiagPacketGeneratorBurstTransmitStatusGet(devNum, portNum, &burstTransmitDone);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", burstTransmitDone);

    return CMD_OK;
}

/**
* @internal wrCpssDxChDiagBistTriggerAllSet function
* @endinternal
*
* @brief   Trigger the starting of BIST on device physical RAMs.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Triggering the BIST will cause memory content corruption!!!
*
*/
static CMD_STATUS wrCpssDxChDiagBistTriggerAllSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;
    GT_U8                                       devNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChDiagBistTriggerAllSet(devNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

#define CPSS_DXCH_DIAG_BIST_RAMS_NUM_CNS 1667
CPSS_DXCH_DIAG_BIST_RESULT_STC              bistResultsArr[CPSS_DXCH_DIAG_BIST_RAMS_NUM_CNS];
GT_U32                                      bistResultsNum = 0;
GT_U32                                      currentBistResult;

/**
* @internal wrCpssDxChDiagBistStatusGet function
* @endinternal
*
* @brief   Retrieve the BIST results.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChDiagBistStatusGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;
    GT_U8                                       devNum;
    CPSS_DXCH_DIAG_BIST_STATUS_ENT              bistStatus;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    bistResultsNum = CPSS_DXCH_DIAG_BIST_RAMS_NUM_CNS;

    /* call cpss api function */
    result = cpssDxChDiagBistResultsGet(devNum, &bistStatus, bistResultsArr, &bistResultsNum);

    if( CPSS_DXCH_DIAG_BIST_STATUS_FAIL_E != bistStatus )
    {
        bistResultsNum = 0;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", bistStatus, bistResultsNum);

    return CMD_OK;
}

static CMD_STATUS wrCpssDxChDiagBistResultsGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    currentBistResult = 0;

    if( 0 < bistResultsNum )
    {
        inFields[0] = bistResultsArr[currentBistResult].memType;
        inFields[1] = bistResultsArr[currentBistResult].location.dfxPipeId;
        inFields[2] = bistResultsArr[currentBistResult].location.dfxClientId;
        inFields[3] = bistResultsArr[currentBistResult].location.dfxMemoryId;

        /* pack and output table fields */
        fieldOutput("%d%d%d%d", inFields[0], inFields[1], inFields[2], inFields[3]);
        galtisOutput(outArgs, GT_OK, "%f");
    }
    else
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
    }

    return CMD_OK;
}

static CMD_STATUS wrCpssDxChDiagBistResultsGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    currentBistResult++;

    if( currentBistResult < bistResultsNum )
    {
        inFields[0] = bistResultsArr[currentBistResult].memType;
        inFields[1] = bistResultsArr[currentBistResult].location.dfxPipeId;
        inFields[2] = bistResultsArr[currentBistResult].location.dfxClientId;
        inFields[3] = bistResultsArr[currentBistResult].location.dfxMemoryId;

        /* pack and output table fields */
        fieldOutput("%d%d%d%d", inFields[0], inFields[1], inFields[2], inFields[3]);
        galtisOutput(outArgs, GT_OK, "%f");
    }
    else
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
    }

    return CMD_OK;
}

/**
* @internal wrCpssDxChDiagExternalMemoriesBistRun function
* @endinternal
*
* @brief   This function runs BIST in given set of DDR units.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This function should be called when External DRAM was already initialized.
*       The cpssDxChTmGlueDramInit is used for External DRAM initialization.
*
*/
static CMD_STATUS wrCpssDxChDiagExternalMemoriesBistRun
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                    result;
    GT_U8                                        devNum;
    GT_U32                                       extMemoBitmap;
    GT_BOOL                                      testWholeMemory;
    GT_U32                                       testedAreaOffset;
    GT_U32                                       testedAreaLength;
    CPSS_DXCH_DIAG_EXT_MEMORY_BIST_PATTERN_ENT   pattern;
    GT_BOOL                                      testStatus;
    CPSS_DXCH_DIAG_EXT_MEMORY_INF_BIST_ERROR_STC errorInfoArr[5];

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum            = (GT_U8)inArgs[0];
    extMemoBitmap     = (GT_U32)inArgs[1];
    testWholeMemory   = (GT_BOOL)inArgs[2];
    testedAreaOffset  = (GT_U32)inArgs[3];
    testedAreaLength  = (GT_U32)inArgs[4];
    pattern           = (CPSS_DXCH_DIAG_EXT_MEMORY_BIST_PATTERN_ENT)inArgs[5];

    cpssOsMemSet(errorInfoArr, 0, sizeof(errorInfoArr));

    /* call cpss api function */
    result = cpssDxChDiagExternalMemoriesBistRun(
    	devNum, extMemoBitmap, testWholeMemory,
        testedAreaOffset, testedAreaLength,
        pattern, &testStatus, errorInfoArr);

    /* pack output arguments to galtis string */
    galtisOutput(
        outArgs, result, "%d%d%d%d%d%d%d%d%d%d%d", testStatus,
        errorInfoArr[0].errCounter, errorInfoArr[0].lastFailedAddr,
        errorInfoArr[1].errCounter, errorInfoArr[1].lastFailedAddr,
        errorInfoArr[2].errCounter, errorInfoArr[2].lastFailedAddr,
        errorInfoArr[3].errCounter, errorInfoArr[3].lastFailedAddr,
        errorInfoArr[4].errCounter, errorInfoArr[4].lastFailedAddr);

    return CMD_OK;
}

/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
    {"cpssDxChDiagMemTest",
        &wrCpssDxChDiagMemTest,
        5, 0},

    {"cpssDxChDiagAllMemTest",
        &wrCpssDxChDiagAllMemTest,
        1, 0},

    {"cpssDxChDiagMemWrite",
        &wrCpssDxChDiagMemWrite,
        4, 0},

    {"cpssDxChDiagMemRead",
        &wrCpssDxChDiagMemRead,
        3, 0},

    {"cpssDxChDiagRegWrite",
        &wrCpssDxChDiagRegWrite,
        6, 0},

    {"cpssDxChDiagRegRead",
        &wrCpssDxChDiagRegRead,
        5, 0},

    {"cpssDxChDiagPhyRegWrite",
        &wrCpssDxChDiagPhyRegWrite,
        7, 0},

    {"cpssDxChDiagPhyRegRead",
        &wrCpssDxChDiagPhyRegRead,
        6, 0},

    {"cpssDxChDiagRegsNumGet",
        &wrCpssDxChDiagRegsNumGet,
        1, 0},

    {"cpssDxChDiagRegsDump",
        &wrCpssDxChDiagRegsDump,
        3, 0},

    {"cpssDxChDiagRegTest",
        &wrCpssDxChDiagRegTest,
        4, 0},

    {"cpssDxChDiagAllRegTest",
        &wrCpssDxChDiagAllRegTest,
        1, 0},
    {"cpssDxChDiagPrbsPortTransmitModeSet",
        &wrCpssDxChDiagPrbsPortTransmitModeSet,
        4, 0},
    {"cpssDxChDiagPrbsPortTransmitModeGet",
        &wrCpssDxChDiagPrbsPortTransmitModeGet,
        3, 0},
    {"cpssDxChDiagPrbsPortGenerateEnableSet",
        &wrCpssDxChDiagPrbsPortGenerateEnableSet,
        4, 0},
    {"cpssDxChDiagPrbsPortGenerateEnableGet",
        &wrCpssDxChDiagPrbsPortGenerateEnableGet,
        3, 0},
    {"cpssDxChDiagPrbsPortCheckEnableSet",
        &wrCpssDxChDiagPrbsPortCheckEnableSet,
        4, 0},
    {"cpssDxChDiagPrbsPortCheckEnableGet",
        &wrCpssDxChDiagPrbsPortCheckEnableGet,
        3, 0},
    {"cpssDxChDiagPrbsPortCheckReadyGet",
        &wrCpssDxChDiagPrbsPortCheckReadyGet,
        2, 0},
    {"cpssDxChDiagPrbsPortStatusGet",
        &wrCpssDxChDiagPrbsPortStatusGet,
        3, 0},
    {"cpssDxChDiagPrbsCyclicDataSet",
        &wrCpssDxChDiagPrbsCyclicDataSet,
        7, 0},
    {"cpssDxChDiagPrbsCyclicDataGet",
        &wrCpssDxChDiagPrbsCyclicDataGet,
        3, 0},
    {"cpssDxChDiagPrbsSerdesTestEnableSet",
        &wrCpssDxChDiagPrbsSerdesTestEnableSet,
        4, 0},
    {"cpssDxChDiagPrbsSerdesTestEnableGet",
        &wrCpssDxChDiagPrbsSerdesTestEnableGet,
        3, 0},
    {"cpssDxChDiagPrbsSerdesTransmitModeSet",
        &wrCpssDxChDiagPrbsSerdesTransmitModeSet,
        4, 0},
    {"cpssDxChDiagPrbsSerdesTransmitModeSet1",
        &wrCpssDxChDiagPrbsSerdesTransmitModeSet,
        4, 0},
    {"cpssDxChDiagPrbsSerdesTransmitModeGet",
        &wrCpssDxChDiagPrbsSerdesTransmitModeGet,
        3, 0},
    {"cpssDxChDiagPrbsSerdesTransmitModeGet1",
        &wrCpssDxChDiagPrbsSerdesTransmitModeGet,
        3, 0},
    {"cpssDxChDiagPrbsSerdesStatusGet",
        &wrCpssDxChDiagPrbsSerdesStatusGet,
        3, 0},
    {"cpssDxChDiagMemoryBistBlockStatusGet",
        &wrCpssDxChDiagMemoryBistBlockStatusGet,
        3, 0},
    {"cpssDxChDiagMemoryBistsRun",
        &wrCpssDxChDiagMemoryBistsRun,
        4, 0},
    {"cpssDxChDiagMemoryBistsRun1",
         &wrCpssDxChDiagMemoryBistsRun1,
         6, 0},
    {"cpssDxChDiagDeviceTemperatureGet",
        &wrCpssDxChDiagDeviceTemperatureGet,
        1, 0},
    {"cpssDxChDiagDeviceVoltageGet",
        &wrCpssDxChDiagDeviceVoltageGet,
        2, 0},
    {"cpssDxChDiagDeviceTemperatureKelvinGet",
        &wrCpssDxChDiagDeviceTemperatureKelvinGet,
        1, 0},
    {"cpssDxChDiagDeviceTemperatureSensorsSelectSet",
        &wrCpssDxChDiagDeviceTemperatureSensorsSelectSet,
        2, 0},
    {"cpssDxChDiagDeviceTemperatureSensorsSelectGet",
        &wrCpssDxChDiagDeviceTemperatureSensorsSelectGet,
        1, 0},
    {"cpssDxChDiagDeviceTemperatureThresholdSet",
        &wrCpssDxChDiagDeviceTemperatureThresholdSet,
        2, 0},
    {"cpssDxChDiagDeviceTemperatureThresholdGet",
        &wrCpssDxChDiagDeviceTemperatureThresholdGet,
        1, 0},
    {"cpssDxChDiagSerdesTuningRxTuneGetFirst",
        &wrCpssDxChDiagSerdesTuningRxTuneGet,
        7, 0},
    {"cpssDxChDiagSerdesTuningRxTuneGetNext",
        &wrCpssDxChDiagSerdesTuningRxTuneGetNext,
        7, 0},
    {"cpssDxChDiagPacketGeneratorConnectGetFirst",
        &wrCpssDxChDiagPacketGeneratorConnectGetFirst,
        2, 0},
    {"cpssDxChDiagPacketGeneratorConnectGetNext",
        &wrCpssDxChDiagPacketGeneratorConnectGetNext,
        2, 0},

    {"cpssDxChDiagPacketGeneratorConnectSet",
        &wrCpssDxChDiagPacketGeneratorConnectSet,
        2, 18},

    {"cpssDxChDiagPacketGeneratorConnect_1GetFirst",
        &wrCpssDxChDiagPacketGeneratorConnect_1GetFirst,
        2, 0},
    {"cpssDxChDiagPacketGeneratorConnect_1GetNext",
        &wrCpssDxChDiagPacketGeneratorConnect_1GetNext,
        2, 0},

    {"cpssDxChDiagPacketGeneratorConnect_1Set",
        &wrCpssDxChDiagPacketGeneratorConnect_1Set,
        2, 20},

    {"cpssDxChDiagPacketGeneratorTransmitEnable",
        &wrCpssDxChDiagPacketGeneratorTransmitEnable,
        3, 0},
    {"cpssDxChDiagPacketGeneratorBurstTransmitStatusGet",
        &wrCpssDxChDiagPacketGeneratorBurstTransmitStatusGet,
        2, 0},

    {"cpssDxChDiagBistTriggerAllSet",
        &wrCpssDxChDiagBistTriggerAllSet,
        1, 0},

    {"cpssDxChDiagBistStatusGet",
        &wrCpssDxChDiagBistStatusGet,
        1, 0},

    {"cpssDxChDiagBistResultsGetFirst",
        &wrCpssDxChDiagBistResultsGetFirst,
        1,0},

    {"cpssDxChDiagBistResultsGetNext",
        &wrCpssDxChDiagBistResultsGetNext,
        1,0},

    {"cpssDxChDiagExternalMemoriesBistRun",
        &wrCpssDxChDiagExternalMemoriesBistRun,
        5,0}
};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssDxChDiag function
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
GT_STATUS cmdLibInitCpssDxChDiag
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}

#define PRV_DIAG_DEBUG_MAC 1

#if PRV_DIAG_DEBUG_MAC

/* gebug purposed functions */

GT_STATUS    prvDebugCpssDxChDiagExternalMemoriesBistRun
(
	IN  GT_U8	                                     devNum,
    IN  GT_U32                                       extMemoBitmap,
    IN  GT_BOOL                                      testWholeMemory,
    IN  GT_U32                                       testedAreaOffset,
    IN  GT_U32                                       testedAreaLength,
    IN  CPSS_DXCH_DIAG_EXT_MEMORY_BIST_PATTERN_ENT   pattern
)
{
    GT_STATUS                                    result;
    GT_BOOL                                      testStatus;
    CPSS_DXCH_DIAG_EXT_MEMORY_INF_BIST_ERROR_STC errorInfoArr[5];
    GT_U32                                       i;

    /* call cpss api function */
    result = cpssDxChDiagExternalMemoriesBistRun(
        devNum, extMemoBitmap, testWholeMemory,
        testedAreaOffset, testedAreaLength,
        pattern, &testStatus, errorInfoArr);
    if (result != GT_OK)
    {
        cpssOsPrintf(
            "cpssDxChDiagExternalMemoriesBistRun failed, rc = %d\n",
            result);
        return result;
    }

    /* pack output arguments to galtis string */
    cpssOsPrintf(
        "cpssDxChDiagExternalMemoriesBistRun testStatus = %d\n",
        testStatus);
    if (testStatus == GT_FALSE)
    {
        for (i = 0; (i < 5); i++)
        {
            if (((extMemoBitmap >> i) & 1) == 0)
            {
                continue;
            }
            cpssOsPrintf(
                "#%d errCounter: 0x%8.8X, lastFailedAddr: 0x%8.8X\n",
                i, errorInfoArr[i].errCounter, errorInfoArr[i].lastFailedAddr);
        }
    }

    return CMD_OK;
}

#include <mvDdr3TrainingIp.h>
#include <mvDdr3TrainingIpDb.h>
#include <mvDdr3TrainingIpDef.h>
#include <mvDdr3TrainingIpBist.h>
#include <mvDdr3TrainingIpFlow.h>
#include <mvDdr3TrainingIpPrvIf.h>
#include <mvHwsDdr3Bc2.h>

GT_STATUS ddr3BapSetAlgoFlow
(
	GT_U8       devNum,
	GT_U32		algoFlowMask
);

/*******************************************************************************
* prvCpssDxChDiagExternalMemoriesBistInit
*
* DESCRIPTION:
*       This function initializes DDR units for BIST.
*
* APPLICABLE DEVICES:
*        Bobcat2; Caelum.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
*
* INPUTS:
*       devNum           - device number
*       boardId          - boardId: 0 and 1 supported
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - one of the parameters value is wrong
*       GT_OUT_OF_CPU_MEM        - no CPU memory allocation fail
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       This function should be called when External DRAM was not initialized.
*       by cpssDxChTmGlueDramInit.
*
*******************************************************************************/
extern MV_HWS_TOPOLOGY_MAP bc2TopologyMap[];
GT_STATUS prvCpssDxChDiagExternalMemoriesBistInit
(
	IN  GT_U8 	                                     devNum,
	IN  GT_U32	                                     boardId
)
{
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(
        devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
        | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
        CPSS_XCAT2_E | CPSS_LION_E | CPSS_LION2_E );

    if (boardId >= 2)
    {
        return GT_BAD_PARAM;
    }

    if (PRV_CPSS_DXCH_PP_MAC(devNum)->extMemory.mvHwsDevTopologyMapPtr == NULL)
    {
        PRV_CPSS_DXCH_PP_MAC(devNum)->extMemory.mvHwsDevTopologyMapPtr =
            (MV_HWS_TOPOLOGY_MAP*)cpssOsMalloc(sizeof(MV_HWS_TOPOLOGY_MAP));
        if (PRV_CPSS_DXCH_PP_MAC(devNum)->extMemory.mvHwsDevTopologyMapPtr == NULL)
        {
            return GT_OUT_OF_CPU_MEM;
        }
    }

    cpssOsMemCpy(
        PRV_CPSS_DXCH_PP_MAC(devNum)->extMemory.mvHwsDevTopologyMapPtr,
        &(bc2TopologyMap[boardId]),
        sizeof(MV_HWS_TOPOLOGY_MAP));

    rc = mvHwsDdr3TipLoadTopologyMap(
        devNum, PRV_CPSS_DXCH_PP_MAC(devNum)->extMemory.mvHwsDevTopologyMapPtr);
    if( GT_OK != rc )
    {
        return rc;
    }

    rc = mvHwsDdr3TipSelectDdrController(devNum, GT_TRUE);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = ddr3BapSetAlgoFlow(devNum, 0x668F1);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = mvHwsDdr3TipRunAlg(devNum, ALGO_TYPE_DYNAMIC);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

GT_STATUS    prvDebugDdrIfRegWrite
(
    GT_U32                devNum,
    MV_HWS_ACCESS_TYPE    interfaceAccess,
    GT_U32                interfaceId,
    GT_U32                regAddr,
    GT_U32                dataValue
)
{
    return mvHwsDdr3TipIFWrite(
        devNum, interfaceAccess, interfaceId,
        regAddr, dataValue, 0xFFFFFFFF /*mask*/);
}

GT_STATUS    prvDebugDdrIfRegRead
(
    GT_U32                devNum,
    GT_U32                interfaceId,
    GT_U32                regAddr
)
{
    GT_STATUS rc;
    GT_U32 readData[MAX_INTERFACE_NUM];

    rc = mvHwsDdr3TipIFRead(
        devNum, ACCESS_TYPE_UNICAST, interfaceId,
        regAddr, readData, 0xFFFFFFFF /*mask*/);
    if (rc != GT_OK)
    {
        return rc;
    }
    cpssOsPrintf("Read value 0x%8.8X\n", readData[interfaceId]);
    return GT_OK;
}

GT_STATUS prvDebugDdrBistSingle(GT_U8 devNum, MV_HWS_PATTERN  pattern)
{
    GT_STATUS rc;
    GT_U32 res[MAX_INTERFACE_NUM];
    GT_U32 i;

    for(i=0; i<MAX_INTERFACE_NUM; i++)
    {
        res[i] = 0;
    }

    rc = mvHwsDdr3RunBist(0, pattern, &(res[0]), 0);
    if (rc != GT_OK)
    {
        cpssOsPrintf("mvHwsDdr3RunBist rc: 0x%X\n", rc);
        return rc;
    }

    for(i=0; i<MAX_INTERFACE_NUM; i++)
    {
        cpssOsPrintf("result interface %d 0x%X\n", i, res[i]);
    }

    return GT_OK;
}

#endif /*PRV_DIAG_DEBUG_MAC*/



