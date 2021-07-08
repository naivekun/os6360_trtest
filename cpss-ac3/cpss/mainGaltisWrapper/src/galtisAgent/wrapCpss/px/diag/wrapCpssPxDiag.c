/*******************************************************************************
*              (c), Copyright 2006, Marvell International Ltd.                 *
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
* @file wrapCpssPxDiag.c
*
* @brief Wrapper functions for
* cpss/Px/diag/cpssPxDiag.h API's
*
* @version   1
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>

/* Feature specific includes. */
#include <cpss/common/cpssTypes.h>
#include <cpss/px/diag/cpssPxDiag.h>

/**
* @internal wrCpssPxDiagRegsNumGet function
* @endinternal
*
* @brief   Gets the number of registers for the PP.
*         Used to allocate memory for cpssPxDiagRegsDump.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*
*/
static CMD_STATUS wrCpssPxDiagRegsNumGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                          result;
    GT_SW_DEV_NUM                      devNum;
    GT_U32                             regsNumPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    /* call cpss api function */
    result = cpssPxDiagRegsNumGet(devNum, &regsNumPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", regsNumPtr);
    return CMD_OK;
}

/**
* @internal wrCpssPxDiagResetAndInitControllerRegsNumGet function
* @endinternal
*
* @brief   Gets the number of registers for the Reset and Init Controller.
*         Used to allocate memory for cpssPxDiagResetAndInitControllerRegsDump.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*
*/
static CMD_STATUS wrCpssPxDiagResetAndInitControllerRegsNumGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                          result;
    GT_SW_DEV_NUM                      devNum;
    GT_U32                             regsNumPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    /* call cpss api function */
    result = cpssPxDiagResetAndInitControllerRegsNumGet(devNum, &regsNumPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", regsNumPtr);
    return CMD_OK;
}

/**
* @internal wrCpssPxDiagRegWrite function
* @endinternal
*
* @brief   Performs single 32 bit data write to one of the PP PCI configuration or
*         registers.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong input parameter
*
* @note The function may be called before Phase 1 initialization and
*       can NOT be used after cpssHwPpPhase1 to read/write registers
*
*/
static CMD_STATUS wrCpssPxDiagRegWrite
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
    result = cpssPxDiagRegWrite(
        baseAddr, ifChannel, regType, offset, data, doByteSwap);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssPxDiagRegRead function
* @endinternal
*
* @brief   Performs single 32 bit data read from one of the PP PCI configuration or
*         registers.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong input parameter
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note The function may be called before Phase 1 initialization and
*       can NOT be used after cpssHwPpPhase1 to read/write registers
*
*/
static CMD_STATUS wrCpssPxDiagRegRead
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
    result = cpssPxDiagRegRead(
        baseAddr, ifChannel, regType, offset, &dataPtr, doByteSwap);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", dataPtr);
    return CMD_OK;
}

/**
* @internal wrCpssPxDiagRegsDump function
* @endinternal
*
* @brief   Dumps the device register addresses and values according to the given
*         starting offset and number of registers to dump.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*       To dump all the registers the user may call cpssPxDiagRegsNumGet in
*       order to get the number of registers of the PP.
*
*/
static CMD_STATUS wrCpssPxDiagRegsDump
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS           result;
    GT_SW_DEV_NUM       devNum;
    GT_U32              offset;
    GT_U32              regsNum;
    GT_U32              *regAddrPtr;
    GT_U32              *regDataPtr;
    GT_U32              tmpRegsNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM)inArgs[0];
    regsNum = (GT_U32)inArgs[1];
    offset  = (GT_U32)inArgs[2];

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
    tmpRegsNum = regsNum;
    result = cpssPxDiagRegsDump(
        devNum, &tmpRegsNum, offset, regAddrPtr, regDataPtr);

    /* pack output arguments to galtis string */
    galtisOutput(
        outArgs, result, "%d%d%d",
        tmpRegsNum, regAddrPtr[(tmpRegsNum - 1)], regDataPtr[(tmpRegsNum - 1)]);

    cmdOsFree(regAddrPtr);
    cmdOsFree(regDataPtr);
    return CMD_OK;
}

/**
* @internal wrCpssPxDiagResetAndInitControllerRegsDump function
* @endinternal
*
* @brief   Dumps the Reset and Init controller register addresses and values
*         according to the given starting offset and number of registers to dump.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*       To dump all the registers the user may call
*       cpssPxDiagResetAndInitControllerRegsNumGet in order to get the number
*       of registers of the Reset and Init Controller.
*
*/
static CMD_STATUS wrCpssPxDiagResetAndInitControllerRegsDump
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS           result;
    GT_SW_DEV_NUM       devNum;
    GT_U32              offset;
    GT_U32              regsNum;
    GT_U32              *regAddrPtr;
    GT_U32              *regDataPtr;
    GT_U32              tmpRegsNum;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_SW_DEV_NUM)inArgs[0];
    regsNum = (GT_U32)inArgs[1];
    offset  = (GT_U32)inArgs[2];

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
    tmpRegsNum = regsNum;
    result = cpssPxDiagResetAndInitControllerRegsDump(
        devNum, &tmpRegsNum, offset, regAddrPtr, regDataPtr);

    /* pack output arguments to galtis string */
    galtisOutput(
        outArgs, result, "%d%d%d",
        tmpRegsNum, regAddrPtr[(tmpRegsNum - 1)], regDataPtr[(tmpRegsNum - 1)]);

    cmdOsFree(regAddrPtr);
    cmdOsFree(regDataPtr);
    return CMD_OK;
}

/**
* @internal wrCpssPxDiagRegTest function
* @endinternal
*
* @brief   Tests the device read/write ability of a specific register.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*       After the register is tested, the original value prior to the test is
*       restored.
*
*/
static CMD_STATUS wrCpssPxDiagRegTest
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                          result;
    GT_SW_DEV_NUM                      devNum;
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
    devNum  = (GT_SW_DEV_NUM)inArgs[0];
    regAddr = (GT_U32)inArgs[1];
    regMask = (GT_U32)inArgs[2];
    profile = (CPSS_DIAG_TEST_PROFILE_ENT)inArgs[3];

    /* call cpss api function */
    result = cpssPxDiagRegTest(
        devNum, regAddr, regMask, profile,
        &testStatusPtr, &readValPtr, &writeValPtr);

    /* pack output arguments to galtis string */
    galtisOutput(
        outArgs, result, "%d%d%d",
        testStatusPtr, readValPtr, writeValPtr);
    return CMD_OK;
}

/**
* @internal wrCpssPxDiagAllRegTest function
* @endinternal
*
* @brief   Tests the device read/write ability of all the registers.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The test is done by invoking cpssPxDiagRegTest in loop for all the
*       diagnostics registers and for all the patterns.
*       After each register is tested, the original value prior to the test is
*       restored.
*       The function may be called after Phase 1 initialization.
*
*/
static CMD_STATUS wrCpssPxDiagAllRegTest
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                          result;
    GT_SW_DEV_NUM                      devNum;
    GT_BOOL                            testStatusPtr;
    GT_U32                             badRegPtr;
    GT_U32                             readValPtr;
    GT_U32                             writeValPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    /* call cpss api function */
    result = cpssPxDiagAllRegTest(
        devNum, &testStatusPtr, &badRegPtr, &readValPtr, &writeValPtr);

    /* pack output arguments to galtis string */
    galtisOutput(
        outArgs, result, "%d%d%d%d",
        testStatusPtr, badRegPtr, readValPtr, writeValPtr);
    return CMD_OK;
}

/**
* @internal wrCpssPxDiagDeviceTemperatureSensorsSelectSet function
* @endinternal
*
* @brief   Select Temperature Sensors.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum, sensorType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssPxDiagDeviceTemperatureSensorsSelectSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM devNum;
    CPSS_PX_DIAG_TEMPERATURE_SENSOR_ENT  sensorType;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum     = (GT_SW_DEV_NUM)inArgs[0];
    sensorType = (CPSS_PX_DIAG_TEMPERATURE_SENSOR_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssPxDiagDeviceTemperatureSensorsSelectSet(devNum, sensorType);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxDiagDeviceTemperatureSensorsSelectGet function
* @endinternal
*
* @brief   Get Temperature Sensors Select.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssPxDiagDeviceTemperatureSensorsSelectGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM     devNum;
    CPSS_PX_DIAG_TEMPERATURE_SENSOR_ENT  sensorType;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    /* call cpss api function */
    result = cpssPxDiagDeviceTemperatureSensorsSelectGet(
        devNum, &sensorType);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", sensorType);

    return CMD_OK;
}

/**
* @internal wrCpssPxDiagDeviceTemperatureThresholdSet function
* @endinternal
*
* @brief   Set Threshold for Interrupt. If Temperature Sensors is equal or above
*         the threshold, interrupt is activated.
*         APPLICABLE DEVICES:
*         Pipe.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum   - device number
*         thresholdValue - Threshold value in Celsius degrees.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_OUT_OF_RANGE          - on wrong thresholdValue
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssPxDiagDeviceTemperatureThresholdSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_SW_DEV_NUM devNum;
    GT_32  thresholdValue;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum         = (GT_SW_DEV_NUM)inArgs[0];
    thresholdValue = (GT_32)inArgs[1];

    /* call cpss api function */
    result = cpssPxDiagDeviceTemperatureThresholdSet(devNum, thresholdValue);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssPxDiagDeviceTemperatureThresholdGet function
* @endinternal
*
* @brief   Get Threshold for Interrupt. If Temperature Sensors is equal or above
*         the threshold, interrupt is activated.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssPxDiagDeviceTemperatureThresholdGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS         result;
    GT_SW_DEV_NUM     devNum;
    GT_32             thresholdValue;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssPxDiagDeviceTemperatureThresholdGet(
        devNum, &thresholdValue);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", thresholdValue);
    return CMD_OK;
}

/**
* @internal wrCpssPxDiagDeviceTemperatureGet function
* @endinternal
*
* @brief   Gets the PP temperature.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssPxDiagDeviceTemperatureGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS         result;
    GT_SW_DEV_NUM     devNum;
    GT_32             temperature;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    temperature = 0;

    /* call cpss api function */
    result = cpssPxDiagDeviceTemperatureGet(
        devNum, &temperature);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", temperature);
    return CMD_OK;
}

/**
* @internal wrCpssPxDiagDeviceVoltageGet function
* @endinternal
*
* @brief   Gets the PP sensor voltage.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - null pointer
*/
static CMD_STATUS wrCpssPxDiagDeviceVoltageGet
(
        IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
        IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
        IN  GT_32 numFields,
        OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS         result;
    GT_SW_DEV_NUM     devNum;
    IN  GT_U32        sensorNum;
    GT_U32            voltage_milivolts;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum    = (GT_SW_DEV_NUM)inArgs[0];
    sensorNum =  (GT_U32)inArgs[1];
    voltage_milivolts = 0;

    /* call cpss api function */
    result = cpssPxDiagDeviceVoltageGet(devNum, sensorNum, &voltage_milivolts);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", voltage_milivolts);
    return CMD_OK;
}


/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
    /* commandName,                                  funcReference                                       funcArgs funcFields */
    {"cpssPxDiagRegsNumGet",                         &wrCpssPxDiagRegsNumGet,                            1, 0},
    {"cpssPxDiagResetAndInitControllerRegsNumGet",   &wrCpssPxDiagResetAndInitControllerRegsNumGet,      1, 0},
    {"cpssPxDiagRegWrite",                           &wrCpssPxDiagRegWrite,                              6, 0},
    {"cpssPxDiagRegRead",                            &wrCpssPxDiagRegRead,                               5, 0},
    {"cpssPxDiagRegsDump",                           &wrCpssPxDiagRegsDump,                              3, 0},
    {"cpssPxDiagResetAndInitControllerRegsDump",     &wrCpssPxDiagResetAndInitControllerRegsDump,        3, 0},
    {"cpssPxDiagRegTest",                            &wrCpssPxDiagRegTest,                               4, 0},
    {"cpssPxDiagAllRegTest",                         &wrCpssPxDiagAllRegTest,                            1, 0},
    {"cpssPxDiagDeviceTemperatureSensorsSelectSet",  &wrCpssPxDiagDeviceTemperatureSensorsSelectSet,     2, 0},
    {"cpssPxDiagDeviceTemperatureSensorsSelectGet",  &wrCpssPxDiagDeviceTemperatureSensorsSelectGet,     1, 0},
    {"cpssPxDiagDeviceTemperatureThresholdSet",      &wrCpssPxDiagDeviceTemperatureThresholdSet,         2, 0},
    {"cpssPxDiagDeviceTemperatureThresholdGet",      &wrCpssPxDiagDeviceTemperatureThresholdGet,         1, 0},
    {"cpssPxDiagDeviceTemperatureGet",               &wrCpssPxDiagDeviceTemperatureGet,                  1, 0},
    {"cpssPxDiagDeviceVoltageGet",                   &wrCpssPxDiagDeviceVoltageGet,                      2, 0}
};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssPxDiag function
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
GT_STATUS cmdLibInitCpssPxDiag
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}



