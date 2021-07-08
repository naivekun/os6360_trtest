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
* @file wrapCpssDriverPpHw.c
*
* @brief Wrapper functions for Cpss Hardware
*
* @version   20
********************************************************************************
*/

/* Common galtis includes */
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>


/* Feature specific includes. */
#include <cpss/common/cpssTypes.h>
#include <cpssDriver/pp/hardware/cpssDriverPpHw.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpssDriver/pp/config/generic/cpssDrvPpGenDump.h>

extern GT_STATUS prvCpssDrvHwPpMemoryAccessTraceEnableSet
(
    IN GT_BOOL enable
);

/**
* @internal wrCpssDrvPpHwRegBitMaskRead function
* @endinternal
*
* @brief   Reads the unmasked bits of a register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note The bits in value to be read are the masked bit of 'mask'.
*
*/
static CMD_STATUS wrCpssDrvPpHwRegBitMaskRead
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    GT_STATUS   status;

    GT_U8    devNum;
    GT_U32   regAddr;
    GT_U32   mask;
    GT_U32   data;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum = (GT_U8)inArgs[0];
    regAddr = (GT_U32)inArgs[1];
    mask = (GT_U32)inArgs[2];

    if (mask == 0)
    {
        mask = 0xFFFFFFFF;
    }

    /* call cpss api function */
    status =  cpssDrvPpHwRegBitMaskRead(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,regAddr, mask, &data);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d", data);

    return CMD_OK;
}

/**
* @internal wrCpssDrvPpHwRegBitMaskWrite function
* @endinternal
*
* @brief   Writes the unmasked bits of a register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note The bits in value to be written are the masked bit of 'mask'.
*
*/
static CMD_STATUS wrCpssDrvPpHwRegBitMaskWrite
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    GT_STATUS   status;

    GT_U8    devNum;
    GT_U32   regAddr;
    GT_U32   mask;
    GT_U32   data;
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum = (GT_U8)inArgs[0];
    regAddr = (GT_U32)inArgs[1];
    mask = (GT_U32)inArgs[2];
    data = (GT_U32)inArgs[3];

    if (mask == 0)
    {
        mask = 0xFFFFFFFF;
    }

    /* call cpss api function */
    status =  cpssDrvPpHwRegBitMaskWrite(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,regAddr, mask, data);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssDrvPpHwPortGroupRegBitMaskRead function
* @endinternal
*
* @brief   Reads the unmasked bits of a register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note The bits in value to be read are the masked bit of 'mask'.
*
*/
static CMD_STATUS wrCpssDrvPpHwPortGroupRegBitMaskRead
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    GT_STATUS   status;

    GT_U8    devNum;
    GT_U32   portGroupId;
    GT_U32   regAddr;
    GT_U32   mask;
    GT_U32   data;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum = (GT_U8)inArgs[0];
    portGroupId = (GT_U32)inArgs[1];
    regAddr = (GT_U32)inArgs[2];
    mask = (GT_U32)inArgs[3];

    if (mask == 0)
    {
        mask = 0xFFFFFFFF;
    }

    /* call cpss api function */
    status =  cpssDrvPpHwRegBitMaskRead(
        devNum, portGroupId,regAddr, mask, &data);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d", data);

    return CMD_OK;
}

/**
* @internal wrCpssDrvPpHwPortGroupRegBitMaskWrite function
* @endinternal
*
* @brief   Writes the unmasked bits of a register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note The bits in value to be written are the masked bit of 'mask'.
*
*/
static CMD_STATUS wrCpssDrvPpHwPortGroupRegBitMaskWrite
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    GT_STATUS   status;

    GT_U8    devNum;
    GT_U32   portGroupId;
    GT_U32   regAddr;
    GT_U32   mask;
    GT_U32   data;
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum = (GT_U8)inArgs[0];
    portGroupId =  (GT_U32)inArgs[1];
    regAddr = (GT_U32)inArgs[2];
    mask = (GT_U32)inArgs[3];
    data = (GT_U32)inArgs[4];

    if (mask == 0)
    {
        mask = 0xFFFFFFFF;
    }

    /* call cpss api function */
    status =  cpssDrvPpHwRegBitMaskWrite(devNum, portGroupId,regAddr, mask, data);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssDrvPpHwPortGroupRegBitMaskRangeWrite function
* @endinternal
*
* @brief   Writes the unmasked bits of a register to a certain range and offset given
*         by the user.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note The bits in value to be written are the masked bit of 'mask'.
*
*/
static CMD_STATUS wrCpssDrvPpHwPortGroupRegBitMaskRangeWrite
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    GT_STATUS   status = GT_OK;

    GT_U8    devNum;
    GT_U32   portGroupId;
    GT_U32   regAddr;
    GT_U32   mask;
    GT_U32   data;
    GT_U32   formulaOffset;
    GT_U32   rangeStart;
    GT_U32   rangeEnd;
    GT_U32   address;
    GT_U32   i=0;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum = (GT_U8)inArgs[0];
    portGroupId =  (GT_U32)inArgs[1];
    regAddr = (GT_U32)inArgs[2];
    mask = (GT_U32)inArgs[3];
    data = (GT_U32)inArgs[4];
    formulaOffset = (GT_U32)inArgs[5];
    rangeStart = (GT_U32)inArgs[6];
    rangeEnd = (GT_U32)inArgs[7];

    if (rangeStart > rangeEnd)
    {
        return CMD_AGENT_ERROR;
    }

    if (mask == 0)
    {
        mask = 0xFFFFFFFF;
    }

    /* call cpss api function */
    for (i = rangeStart;i <= rangeEnd;i++)
    {
        address = regAddr + (i*formulaOffset);
        status =  cpssDrvPpHwRegBitMaskWrite(devNum, portGroupId,address, mask, data);
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssDrvPpHwPortGroupRegBitMaskRangeRead function
* @endinternal
*
* @brief   Reads the unmasked bits of a register to a certain range and offset given
*         by the user.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note The bits in value to be read are the masked bit of 'mask'.
*
*/
static CMD_STATUS wrCpssDrvPpHwPortGroupRegBitMaskRangeRead
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    GT_STATUS   status = GT_OK;

    GT_U8    devNum;
    GT_U32   portGroupId;
    GT_U32   regAddr;
    GT_U32   mask;
    GT_U32   data;
    GT_U32   formulaOffset;
    GT_U32   rangeStart;
    GT_U32   rangeEnd;
    GT_U32   address;
    GT_U32   i=0;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum = (GT_U8)inArgs[0];
    portGroupId =  (GT_U32)inArgs[1];
    regAddr = (GT_U32)inArgs[2];
    mask = (GT_U32)inArgs[3];
    formulaOffset = (GT_U32)inArgs[4];
    rangeStart = (GT_U32)inArgs[5];
    rangeEnd = (GT_U32)inArgs[6];

    if (rangeStart > rangeEnd)
    {
        return CMD_AGENT_ERROR;
    }

    if (mask == 0)
    {
        mask = 0xFFFFFFFF;
    }

    /* call cpss api function */
    for (i = rangeStart;i <= rangeEnd;i++)
    {
        address = regAddr + (i*formulaOffset);
        status =  cpssDrvPpHwRegBitMaskRead(devNum, portGroupId,address, mask, &data);
        /* pack output arguments to galtis string */
        cmdOsPrintf("port group: %d, address: 0x%08X, data: 0x%08X\n", portGroupId, address , data);
    }

    galtisOutput(outArgs, status, "%d", GT_OK);

    return CMD_OK;
}

/**
* @internal wrCpssDrvHwPpResetAndInitControllerReadReg function
* @endinternal
*
* @brief   Read a register value from the Reset and Init Controller.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDrvHwPpResetAndInitControllerReadReg
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    GT_STATUS   status;

    GT_U8    devNum;
    GT_U32   regAddr;
    GT_U32   data;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum = (GT_U8)inArgs[0];
    regAddr = (GT_U32)inArgs[1];

    /* call cpss api function */
    status = cpssDrvHwPpResetAndInitControllerReadReg(devNum, regAddr, &data);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d", data);

    return CMD_OK;
}

/**
* @internal wrCpssDrvHwPpResetAndInitControllerWriteReg function
* @endinternal
*
* @brief   Write to the Reset and Init Controller given register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDrvHwPpResetAndInitControllerWriteReg
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    GT_STATUS   status;

    GT_U8    devNum;
    GT_U32   regAddr;
    GT_U32   data;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum = (GT_U8)inArgs[0];
    regAddr = (GT_U32)inArgs[1];
    data = (GT_U32)inArgs[2];

    /* call cpss api function */
    status = cpssDrvHwPpResetAndInitControllerWriteReg(devNum, regAddr, data);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssDrvPpHwInternalPciRegRead function
* @endinternal
*
* @brief   This function reads from an internal pci register, it's used by the
*         initialization process and the interrupt service routine.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDrvPpHwInternalPciRegRead
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   status;

    GT_U8    devNum;
    GT_U32   regAddr;
    GT_U32   data;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum = (GT_U8)inArgs[0];
    regAddr = (GT_U32)inArgs[1];

    /* call cpss api function */
    status = cpssDrvPpHwInternalPciRegRead(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, regAddr, &data);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d", data);

    return CMD_OK;
}

/**
* @internal wrCpssDrvPpHwInternalPciRegWrite function
* @endinternal
*
* @brief   This function reads from an internal pci register, it's used by the
*         initialization process and the interrupt service routine.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDrvPpHwInternalPciRegWrite
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    GT_STATUS   status;

    GT_U8    devNum;
    GT_U32   regAddr;
    GT_U32   data;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum = (GT_U8)inArgs[0];
    regAddr = (GT_U32)inArgs[1];
    data = (GT_U32)inArgs[2];

    /* call cpss api function */
    status = cpssDrvPpHwInternalPciRegWrite(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, regAddr, data);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}


/* Dump functions */
GT_STATUS cpssDrvDumPciReg
(
    IN GT_U8   devNum,
    IN GT_U32  regAddr
)
{
    GT_U32 regData;
    GT_STATUS rc;

    rc = cpssDrvPpHwInternalPciRegRead(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,regAddr,&regData);


    cmdOsPrintf("Device %d PCI/PEX register address 0x%08X value = 0x%08X\n",
             devNum,regAddr,regData);

    return rc;
}

/**
* @internal wrCpssPpDumpRegisters function
* @endinternal
*
* @brief   This routine print dump of PP registers.
*
* @retval GT_OK                    - on success (all Registers dumped successfully)
* @retval GT_FAIL                  - on failure. (at least one register read error)
* @retval GT_BAD_PARAM             - invalid device number
*/
static CMD_STATUS wrCpssPpDumpRegisters
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   status;

    IN GT_U8    devNum;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    status =  cpssPpDumpRegisters(devNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;

}

/**
* @internal wrCpssPpDumpMemory function
* @endinternal
*
* @brief   This routine print dump of PP memory.
*
* @retval GT_OK                    - on success (memory dumped successfully)
* @retval GT_FAIL                  - on failure. (memory read error)
* @retval GT_BAD_PARAM             - invalid device number
*
* @note 4 lower bits of startAddr will be zeroed (ignored) for alignment
*
*/
static CMD_STATUS wrCpssPpDumpMemory
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   status;

    IN GT_U8                        devNum;
    IN GT_U32                       startAddr;
    IN CPSS_MEMORY_DUMP_TYPE_ENT    dumpType;
    IN GT_U32                       dumpLength;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum     = (GT_U8)inArgs[0];
    startAddr  = (GT_U32)inArgs[1];
    dumpType   = (CPSS_MEMORY_DUMP_TYPE_ENT)inArgs[2];
    dumpLength = (GT_U32)inArgs[3];

    /* call cpss api function */
    status =  cpssPpDumpMemory(devNum,startAddr,dumpType,dumpLength);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;

}

/**
* @internal wrCpssHwPortGroupSetMemDumpTblEntry function
* @endinternal
*
* @brief   set data to memory
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
static CMD_STATUS wrCpssHwPortGroupSetMemDumpTblEntry
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8 devNum;
    GT_U32 portGroupId;
    GT_U32 regAddr;
    GT_STATUS status;
    GT_U32 data[4];

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if(numFields < 5)
        return CMD_FIELD_UNDERFLOW;
    if(numFields > 5)
        return CMD_FIELD_OVERFLOW;

    devNum  = (GT_U8)inArgs[0];
    portGroupId = (GT_U32)inArgs[1];
    regAddr = (GT_U32)inFields[0];

    data[0] = (GT_U32)inFields[1];
    data[1] = (GT_U32)inFields[2];
    data[2] = (GT_U32)inFields[3];
    data[3] = (GT_U32)inFields[4];

    status = cpssDrvPpHwRamWrite(devNum,portGroupId,regAddr,4,data);
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/********************************************************************/

static GT_U32 currentPortGroupAddress;
static GT_U32 endPortGroupAddress;

/**
* @internal wrCpssHwPortGroupGetMemDumpTblNextEntry function
* @endinternal
*
* @brief   Get next data from memory
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
static CMD_STATUS wrCpssHwPortGroupGetMemDumpTblNextEntry
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS status;
    GT_U8 devNum;
    GT_U32 portGroupId;
    GT_U32 data[4];

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if (currentPortGroupAddress > endPortGroupAddress)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    devNum = (GT_U8)inArgs[0];
    portGroupId = (GT_U32)inArgs[1];


    status = cpssDrvPpHwRamRead(devNum,portGroupId,currentPortGroupAddress,4,data);
    if(status != GT_OK)
    {
        galtisOutput(outArgs, status, "");
        return CMD_OK;
    }

    inFields[0] = currentPortGroupAddress;
    inFields[1] = data[0];
    inFields[2] = data[1];
    inFields[3] = data[2];
    inFields[4] = data[3];
    currentPortGroupAddress += 16;

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d", inFields[0], inFields[1], inFields[2],
                inFields[3], inFields[4]);
    galtisOutput(outArgs, status, "%f");

    return CMD_OK;
}

/**
* @internal wrCpssHwPortGroupGetMemDumpTblFirstEntry function
* @endinternal
*
* @brief   Get first data from memory
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
static CMD_STATUS wrCpssHwPortGroupGetMemDumpTblFirstEntry
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32 numOfWords;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    currentPortGroupAddress = (GT_U32)inArgs[2];
    numOfWords         = (GT_U32)inArgs[3];
    endPortGroupAddress     = currentPortGroupAddress+4*numOfWords-4;

    return wrCpssHwPortGroupGetMemDumpTblNextEntry(
        inArgs,inFields,numFields,outArgs);
}

/**
* @internal wrCpssHwSetMemDumpTblEntry function
* @endinternal
*
* @brief   set data to memory
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
static CMD_STATUS wrCpssHwSetMemDumpTblEntry
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

    inArgs[3] = inArgs[2];
    inArgs[2] = inArgs[1];
    inArgs[1] = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    return wrCpssHwPortGroupSetMemDumpTblEntry(
        inArgs,inFields,numFields,outArgs);
}

/**
* @internal wrCpssHwGetMemDumpTblFirstEntry function
* @endinternal
*
* @brief   Get first data from memory
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
static CMD_STATUS wrCpssHwGetMemDumpTblFirstEntry
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
    inArgs[3] = inArgs[2];
    inArgs[2] = inArgs[1];
    inArgs[1] = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    return wrCpssHwPortGroupGetMemDumpTblFirstEntry(inArgs,inFields,numFields,outArgs);

}
/**
* @internal wrCpssHwGetMemDumpTblNextEntry function
* @endinternal
*
* @brief   Get next data from memory
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
static CMD_STATUS wrCpssHwGetMemDumpTblNextEntry
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

    inArgs[3] = inArgs[2];
    inArgs[2] = inArgs[1];
    inArgs[1] = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    return wrCpssHwPortGroupGetMemDumpTblNextEntry(
        inArgs,inFields,numFields,outArgs);
}

/**
* @internal wrCpssDrvHwPpMemoryAccessTraceOnOff function
* @endinternal
*
* @brief   On/Off memory access trace
*
* @retval CMD_OK                   - on success.
*/
static CMD_STATUS wrCpssDrvHwPpMemoryAccessTraceOnOff
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   status;
    GT_U8 enable;
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;
    if ((GT_U8)inArgs[0] == 0)
    {
        enable = GT_FALSE;
    }
    else
    {
        enable = GT_TRUE;
    }
    /* call cpss api function */
    status =  prvCpssDrvHwPpMemoryAccessTraceEnableSet(enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssHwPortGroupInternalPciSetMemDumpTblEntry function
* @endinternal
*
* @brief   set data to PCI memory
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
static CMD_STATUS wrCpssHwPortGroupInternalPciSetMemDumpTblEntry
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8 devNum;
    GT_U32 portGroupId;
    GT_U32 regAddr;
    GT_STATUS status = GT_OK;
    GT_U32 ii;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if(numFields < 5)
        return CMD_FIELD_UNDERFLOW;
    if(numFields > 5)
        return CMD_FIELD_OVERFLOW;

    devNum  = (GT_U8)inArgs[0];
    portGroupId = (GT_U32)inArgs[1];
    regAddr = (GT_U32)inFields[0];

    for ( ii = 0 ; ii < 4 ; ii++ )
    {
        status = cpssDrvPpHwInternalPciRegWrite(devNum,
                                                portGroupId,
                                                regAddr + ii*4,
                                                (GT_U32)inFields[ii+1]);
        if( GT_OK != status )
        {
            break;
        }
    }

    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/********************************************************************/

/**
* @internal wrCpssHwPortGroupInternalPciGetMemDumpTblNextEntry function
* @endinternal
*
* @brief   Get next data from PCI memory
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
static CMD_STATUS wrCpssHwPortGroupInternalPciGetMemDumpTblNextEntry
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS status = GT_OK;
    GT_U8 devNum;
    GT_U32 portGroupId;
    GT_U32 data[4];
    GT_U32 ii;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if (currentPortGroupAddress > endPortGroupAddress)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    devNum = (GT_U8)inArgs[0];
    portGroupId = (GT_U32)inArgs[1];

    for( ii = 0 ; ii < 4 ; ii++ )
    {
        status = cpssDrvPpHwInternalPciRegRead(devNum,
                                               portGroupId,
                                               currentPortGroupAddress + ii*4,
                                               &data[ii]);
        if( GT_OK != status )
        {
            galtisOutput(outArgs, status, "");
            return CMD_OK;
        }
    }

    inFields[0] = currentPortGroupAddress;
    inFields[1] = data[0];
    inFields[2] = data[1];
    inFields[3] = data[2];
    inFields[4] = data[3];
    currentPortGroupAddress += 16;

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d", inFields[0], inFields[1], inFields[2],
                inFields[3], inFields[4]);
    galtisOutput(outArgs, status, "%f");

    return CMD_OK;
}

/**
* @internal wrCpssHwPortGroupInternalPciGetMemDumpTblFirstEntry function
* @endinternal
*
* @brief   Get first data from PCI memory
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
static CMD_STATUS wrCpssHwPortGroupInternalPciGetMemDumpTblFirstEntry
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32 numOfWords;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    currentPortGroupAddress = (GT_U32)inArgs[2];
    numOfWords         = (GT_U32)inArgs[3];
    endPortGroupAddress     = currentPortGroupAddress+4*numOfWords-4;

    return wrCpssHwPortGroupInternalPciGetMemDumpTblNextEntry(
        inArgs,inFields,numFields,outArgs);
}

/**
* @internal wrCpssDrvPpHwResourceReadRegister function
* @endinternal
*
* @brief   Read a register value from a resource.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDrvPpHwResourceReadRegister
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    GT_STATUS   status;

    GT_U8    devNum;
    GT_U32   regAddr;
    GT_U32   data;
    GT_U32   portGroupId;
    CPSS_DRV_HW_RESOURCE_TYPE_ENT resource;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum = (GT_U8)inArgs[0];
    portGroupId = (GT_U32)inArgs[1];
    resource    = (CPSS_DRV_HW_RESOURCE_TYPE_ENT)(GT_U32)inArgs[2];
    regAddr = (GT_U32)inArgs[3];

    /* call cpss api function */
    status = cpssDrvPpHwResourceReadRegister(devNum, portGroupId, resource, regAddr, &data);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d", data);

    return CMD_OK;
}

/**
* @internal wrCpssDrvPpHwResourceWriteRegister function
* @endinternal
*
* @brief   Write to a resource given register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDrvPpHwResourceWriteRegister
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    GT_STATUS   status;

    GT_U8    devNum;
    GT_U32   regAddr;
    GT_U32   data;
    GT_U32   portGroupId;
    CPSS_DRV_HW_RESOURCE_TYPE_ENT resource;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum = (GT_U8)inArgs[0];
    portGroupId = (GT_U32)inArgs[1];
    resource    = (CPSS_DRV_HW_RESOURCE_TYPE_ENT)(GT_U32)inArgs[2];
    regAddr = (GT_U32)inArgs[3];
    data = (GT_U32)inArgs[4];

    /* call cpss api function */
    status = cpssDrvPpHwResourceWriteRegister(devNum, portGroupId, resource, regAddr, data);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
    {"cpssDrvPpHwRegBitMaskRead",
        &wrCpssDrvPpHwRegBitMaskRead,
        3, 0},

    {"cpssDrvPpHwRegBitMaskWrite",
        &wrCpssDrvPpHwRegBitMaskWrite,
        4, 0},

    {"cpssDrvPpHwPortGroupRegBitMaskRead",
        &wrCpssDrvPpHwPortGroupRegBitMaskRead,
        4, 0},

    {"cpssDrvPpHwPortGroupRegBitMaskWrite",
        &wrCpssDrvPpHwPortGroupRegBitMaskWrite,
        5, 0},

    {"cpssDrvHwPpResetAndInitControllerReadReg",
        &wrCpssDrvHwPpResetAndInitControllerReadReg,
        2, 0},

    {"cpssDrvHwPpResetAndInitControllerWriteReg",
        &wrCpssDrvHwPpResetAndInitControllerWriteReg,
        3, 0},

    {"cpssDrvPpHwInternalPciRegRead",
        &wrCpssDrvPpHwInternalPciRegRead,
        2, 0},

    {"cpssDrvPpHwInternalPciRegWrite",
        &wrCpssDrvPpHwInternalPciRegWrite,
        3, 0},

    {"cpssPpPortGroupInternalPciDumpMemorySet",
        &wrCpssHwPortGroupInternalPciSetMemDumpTblEntry,
        2, 5},
    {"cpssPpPortGroupInternalPciDumpMemoryGetFirst",
        &wrCpssHwPortGroupInternalPciGetMemDumpTblFirstEntry,
        4, 0},
    {"cpssPpPortGroupInternalPciDumpMemoryGetNext",
        &wrCpssHwPortGroupInternalPciGetMemDumpTblNextEntry,
        4, 0},

    {"cpssPpDumpRegisters ",
        &wrCpssPpDumpRegisters,
        4, 0},

    {"cpssPpDumpMemory",
        &wrCpssPpDumpMemory,
        1, 0},

    {"cpssPpPortGroupRamDumpMemorySet",
        &wrCpssHwPortGroupSetMemDumpTblEntry,
        2, 5},
    {"cpssPpPortGroupRamDumpMemoryGetFirst",
        &wrCpssHwPortGroupGetMemDumpTblFirstEntry,
        4, 0},
    {"cpssPpPortGroupRamDumpMemoryGetNext",
        &wrCpssHwPortGroupGetMemDumpTblNextEntry,
        4, 0},

    {"cpssPpRamDumpMemorySet",
        &wrCpssHwSetMemDumpTblEntry,
        1, 5},
    {"cpssPpRamDumpMemoryGetFirst",
        &wrCpssHwGetMemDumpTblFirstEntry,
        3, 0},
    {"cpssPpRamDumpMemoryGetNext",
        &wrCpssHwGetMemDumpTblNextEntry,
        3, 0},
    {"cpssDrvHwPpMemoryAccessTraceOnOff",
        &wrCpssDrvHwPpMemoryAccessTraceOnOff,
        1, 0},
    {"cpssDrvPpHwPortGroupRegBitMaskRangeWrite",
        &wrCpssDrvPpHwPortGroupRegBitMaskRangeWrite,
        8, 0},
    {"cpssDrvPpHwPortGroupRegBitMaskRangeRead",
        &wrCpssDrvPpHwPortGroupRegBitMaskRangeRead,
        7, 0},
    {"cpssDrvPpHwResourceReadRegister",
        &wrCpssDrvPpHwResourceReadRegister,
        4, 0},
    {"cpssDrvPpHwResourceWriteRegister",
        &wrCpssDrvPpHwResourceWriteRegister,
        5, 0},
};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssDriverPpHw function
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
GT_STATUS cmdLibInitCpssDriverPpHw
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}




