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
* @file wrapCpssPhyMcd.c
* @version   1
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>

/* Feature specific includes. */
#include <cpss/common/cpssTypes.h>
#include <cpss/common/cpssHwInit/cpssHwInit.h>
#include <mcd/src/mcdApiTypes.h>
#include <mcd/src/mcdInitialization.h>
#include <mcd/samples/sample.h>
#include <mcd/src/mcdHwCntl.h>
#include <mcd/src/mcdAPI.h>
#include <mcd/src/mcdRsFEC.h>
#include <mcd/src/mcdDiagnostics.h>
#include <mcd/src/mcdHwSerdesCntl.h>
#include <mcd/src/mcdInternalCtrlApInitIf.h>
#include <mcd/src/mcdAPIInternal.h>

extern MCD_STATUS mcdSerdesTestGen
(
   MCD_DEV_PTR              pDev,
   MCD_U16                  serdesNum,
   MCD_SERDES_TX_PATTERN    txPattern,
   MCD_SERDES_TEST_GEN_MODE mode
);

extern MCD_STATUS mcdSerdesAccessValidate
(
    MCD_SER_DEV_PTR pSerdesDev,
    MCD_U32 sbus_addr
);

extern MCD_STATUS mcdSerdesAutoTuneStatusShort
(
    MCD_DEV_PTR                     pDev,
    MCD_U8                          serdesNum,
    MCD_AUTO_TUNE_STATUS            *rxStatus,
    MCD_AUTO_TUNE_STATUS            *txStatus
);

extern MCD_STATUS mcdSerdesAutoTuneStartExt
(
    MCD_DEV_PTR                     pDev,
    MCD_U8                          serdesNum,
    MCD_RX_TRAINING_MODES           rxTraining,
    MCD_TX_TRAINING_MODES           txTraining
);

extern MCD_STATUS osTimerWkAfter
(
    IN MCD_U32 mils
);
extern MCD_SERDES_CONFIG_DATA serdesElectricalParamsMv[16];
extern MCD_AP_SERDES_CONFIG_DATA serdesApParameters[4];
extern MCD_PRE_DEFINED_CTLE_DATA serdesCtleParamsMv[16];
extern MCD_LANE_STEERING_OVERRIDE_CFG laneSteeringOverrideMv[MCD_MAX_PORT_NUM];

/**
* @internal wrCpssPhyMcdSampleDrvInit function
* @endinternal
*
* @brief   PHY Driver Initialization Routine.
*
* @note   APPLICABLE DEVICES:      ???
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number or configDevDataBufferSize
*/
static CMD_STATUS wrCpssPhyMcdSampleDrvInit
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_PVOID pHostContext;   /* optional host context */
    MCD_BOOL  loadImage;      /* 1:load image; 0:do not load */
    MCD_U16   mdioFirstPort;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pHostContext = (MCD_PVOID)inArgs[1];
    loadImage = (MCD_BOOL)inArgs[2];
    mdioFirstPort = (MCD_U16)inArgs[3];


    /* call cpss api function */
    status = mcdSampleInitDrv(phyId, pHostContext, loadImage, mdioFirstPort);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/*******************************************************************************
 mcdSampleInitDrvLoadAllDevices

 Inputs:
   phyIdBmp         - bitmap of logical Ids
                      logical Id range is 0..15
                      device with logical Id == logId has  firstMDIOPort == (logId * 2)
                      pHostContext wil be NULL for each device
   imagesAmount     - what FW images to load:
                      0 - nothing
                      1 - serdes only
                      2 - serdes and master
                      3 - serdes, master and swap
                      4 - (yet not supported) serdes, master, swap and CM3

 Outputs:
       none

 Returns:
       MCD_OK               - on success
       MCD_FAIL             - on error

 Description:
   Marvell X5123 and EC808 Driver Initialization Routine.

 Side effects:
   None

 Notes/Warnings:

*******************************************************************************/

static CMD_STATUS wrCpssPhyMcdSampleInitDrvLoadAllDevices
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyIdBmp;
    MCD_U32   imagesAmount;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyIdBmp     = (MCD_U32)inArgs[0];
    imagesAmount = (MCD_U32)inArgs[1];


    /* call cpss api function */
    status = mcdSampleInitDrvLoadAllDevices(phyIdBmp, imagesAmount);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdSampleDrvUnload function
* @endinternal
*
* @brief   PHY Driver Unload Routine.
*
* @note   APPLICABLE DEVICES:      ???
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number or configDevDataBufferSize
*/
static CMD_STATUS wrCpssPhyMcdSampleDrvUnload
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];

    /* call cpss api function */
    status = mcdSampleUnloadDrv(phyId);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdHwXmdioWrite function
* @endinternal
*
* @brief   Writes a 16-bit word to the MDIO.
*
* @note   APPLICABLE DEVICES:      ???
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number or configDevDataBufferSize
*
* @note Address is in format X.Y.Z, where X selects the MDIO port (0-31), Y selects
*       the MMD/Device (0-31), and Z selects the register.
*
*/
static CMD_STATUS wrCpssPhyMcdHwXmdioWrite
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 mdioPort;
    MCD_U16 dev;
    MCD_U16 reg;
    MCD_U16 val;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }
    mdioPort = (MCD_U16)inArgs[1];
    dev = (MCD_U16)inArgs[2];
    reg = (MCD_U16)inArgs[3];
    val = (MCD_U16)inArgs[4];

    /* call cpss api function */
    status = mcdHwXmdioWrite(pDev,mdioPort,dev,reg,val);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdHwXmdioRead function
* @endinternal
*
* @brief   Reads a 16-bit word from the MDIO
*
* @note   APPLICABLE DEVICES:      ???
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number or configDevDataBufferSize
*
* @note Address is in format X.Y.Z, where X selects the MDIO port (0-31), Y selects
*       the MMD/Device (0-31), and Z selects the register.
*
*/
static CMD_STATUS wrCpssPhyMcdHwXmdioRead
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 mdioPort;
    MCD_U16 dev;
    MCD_U16 reg;
    MCD_U16 val;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort = (MCD_U16)inArgs[1];
    dev = (MCD_U16)inArgs[2];
    reg = (MCD_U16)inArgs[3];

    /* call cpss api function */
    status = mcdHwXmdioRead(pDev,mdioPort,dev,reg,&val);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d", val);

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdHwPhyRegMask32bitSet function
* @endinternal
*
* @brief   Set a bits of the 32-bit register from the MDIO given by mask
*
* @note   APPLICABLE DEVICES:      ???
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number or configDevDataBufferSize
*
* @note Address is 32 bit value from Functional Specification.
*
*/
static CMD_STATUS wrCpssPhyMcdHwPhyRegMask32bitSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U32   mdioPort;
    MCD_U32   reg;
    MCD_U32   mask;
    MCD_U32   val;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort = (MCD_U32)inArgs[1];
    reg = (MCD_U32)inArgs[2];
    mask = (MCD_U32)inArgs[3];
    val = (MCD_U32)inArgs[4];

    if (mask == 0) mask = 0xFFFFFFFF;

    /* call cpss api function */
    status = mcdHwSetPhyRegMask32bit(pDev,mdioPort,reg,mask,val);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdHwPhyRegMask32bitGet function
* @endinternal
*
* @brief   Get a bits of the 32-bit register from the MDIO given by mask
*
* @note   APPLICABLE DEVICES:      ???
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number or configDevDataBufferSize
*
* @note Address is 32 bit value from Functional Specification.
*
*/
static CMD_STATUS wrCpssPhyMcdHwPhyRegMask32bitGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U32   mdioPort;
    MCD_U32   reg;
    MCD_U32   mask;
    MCD_U32   val;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort = (MCD_U32)inArgs[1];
    reg = (MCD_U32)inArgs[2];
    mask = (MCD_U32)inArgs[3];

    if (mask == 0) mask = 0xFFFFFFFF;

    /* call cpss api function */
    status = mcdHwGetPhyRegMask32bit(pDev,mdioPort,reg,mask,&val);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d", val);

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdHwXmdioWrite32bitSet function
* @endinternal
*
* @brief   Set data to memory
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
static CMD_STATUS wrCpssPhyMcdHwXmdioWrite32bitSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U32   mdioPort;
    MCD_U32   reg;
    MCD_U32   data[4];

    GT_U32  wordIdx;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if(numFields < 5)
        return CMD_FIELD_UNDERFLOW;
    if(numFields > 5)
        return CMD_FIELD_OVERFLOW;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort = (MCD_U32)inArgs[1];

    reg = (GT_U32)inFields[0];
    data[0] = (GT_U32)inFields[1];
    data[1] = (GT_U32)inFields[2];
    data[2] = (GT_U32)inFields[3];
    data[3] = (GT_U32)inFields[4];

    for (wordIdx = 0; wordIdx < 4; wordIdx++)
    {
        status = mcdHwXmdioWrite32bit(pDev, mdioPort, reg + (wordIdx * 0x4), data[wordIdx]);
        if(status != MCD_OK)
        {
            galtisOutput(outArgs, status, "");
            return CMD_OK;
        }
    }

    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

static GT_U32 currentAddress;
static GT_U32 endAddress;

/**
* @internal wrCpssPhyMcdHwXmdioRead32bitCurrentEntryGet function
* @endinternal
*
* @brief   Get current data from memory
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
static CMD_STATUS wrCpssPhyMcdHwXmdioRead32bitCurrentEntryGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U32   mdioPort;
    MCD_U32   data[4];

    GT_U32  wordIdx;


    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort = (MCD_U32)inArgs[1];


    for (wordIdx = 0; wordIdx < 4; wordIdx++)
    {
        status = mcdHwXmdioRead32bit(pDev, mdioPort, currentAddress + (wordIdx * 0x4), &data[wordIdx]);
        if(status != MCD_OK)
        {
            galtisOutput(outArgs, status, "");
            return CMD_OK;
        }
    }

    inFields[0] = currentAddress;
    inFields[1] = data[0];
    inFields[2] = data[1];
    inFields[3] = data[2];
    inFields[4] = data[3];
    currentAddress += 16;

    /* pack and output table fields */
    fieldOutput("%d %d %d %d %d", inFields[0], inFields[1], inFields[2],inFields[3], inFields[4]);

    galtisOutput(outArgs, status, "%f");

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdHwXmdioRead32bitFirstEntry function
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
static CMD_STATUS wrCpssPhyMcdHwXmdioRead32bitFirstEntry
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32  numOfWords;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* Init current and end register adresses for the first time */
    currentAddress = (MCD_U32)inArgs[2];
    numOfWords     = (MCD_U32)inArgs[3];
    endAddress     = currentAddress + (4 * numOfWords) - 4;

    return wrCpssPhyMcdHwXmdioRead32bitCurrentEntryGet(inArgs,inFields,numFields,outArgs);
}

/**
* @internal wrCpssPhyMcdHwXmdioRead32bitNextEntry function
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
static CMD_STATUS wrCpssPhyMcdHwXmdioRead32bitNextEntry
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

    if (currentAddress > endAddress)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    return wrCpssPhyMcdHwXmdioRead32bitCurrentEntryGet(inArgs,inFields,numFields,outArgs);
}

/**
* @internal wrCpssPhyMcdAutoNegEnable function
* @endinternal
*
* @brief   This function Re-enables auto-negotiation..
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK or MCD_FAIL, if action was successful or not
*
* @note Restart autonegation will not take effect if AN is disabled.
*
*/
static CMD_STATUS wrCpssPhyMcdAutoNegEnable
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 mdioPort;
    MCD_U16 host_or_line;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort = (MCD_U16)inArgs[1];
    host_or_line = (MCD_U16)inArgs[2];

    /* call cpss api function */
    status = mcdAutoNegEnable(pDev,mdioPort,host_or_line);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdAutoNegDisable function
* @endinternal
*
* @brief   Turns off the enable auto-negotiation bit disabling auto-negotiation.
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK or MCD_FAIL, if action was successful or not
*
* @note Restart autonegation will not take effect if AN is disabled.
*
*/
static CMD_STATUS wrCpssPhyMcdAutoNegDisable
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 mdioPort;
    MCD_U16 host_or_line;
    MCD_BOOL swReset;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort = (MCD_U16)inArgs[1];
    host_or_line = (MCD_U16)inArgs[2];
    swReset = (MCD_BOOL)inArgs[3];

    /* call cpss api function */
    status = mcdAutoNegDisable(pDev, mdioPort, host_or_line, swReset);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdAutoNegCheckCompleteExt function
* @endinternal
*
* @brief   This function returns the auto-negotiated speed upon completion.
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK or MCD_FAIL, if action was successful or not
*
* @note If the training/AN is not finished, it returns MTD_NEG_NONE for the speed.
*
*/
static CMD_STATUS wrCpssPhyMcdAutoNegCheckCompleteExt
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 mdioPort;
    MCD_U16 host_or_line;
    MCD_BOOL set_speed;
    MCD_BOOL setElecticalParams;
    MCD_U32         counter;
    MCD_STATUS      rc;
    MCD_U32 signalCode = 0;
    MCD_U16 speed = 0;
    MCD_MODE_CONFIG_PARAM config;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    memset(&config, 0, sizeof(MCD_MODE_CONFIG_PARAM));

    mdioPort = (MCD_U16)inArgs[1];
    host_or_line = (MCD_U16)inArgs[2];
    set_speed = (MCD_BOOL)inArgs[3];
    setElecticalParams = (MCD_BOOL)inArgs[4];

    config.ctleBiasParams.host = MCD_CTLE_BIAS_NORMAL;
    config.ctleBiasParams.line = MCD_CTLE_BIAS_NORMAL;

    config.ctleParamsPtr = NULL;

    if (setElecticalParams)
    {
        config.electricalParamsPtr = (MCD_PORT_PER_SERDES_CONFIG_DATA*)&serdesElectricalParamsMv;
    }
    else
    {
        config.electricalParamsPtr = NULL;
    }

    /* call cpss api function */
    counter = 0;
    while (((signalCode & (1 << mdioPort)) == 0) && (counter < 100))
    {
        mcdGetCm3ToHostSignal(pDev, &signalCode);
        osTimerWkAfter(10);
        counter ++;
    }
    if (counter >= 100)
    {
        rc = MCD_FAIL;
    }
    else
    {
        rc = mcdAutoNegCheckCompleteExt(pDev, mdioPort, host_or_line, set_speed, &config, &speed);
    }
    galtisOutput(outArgs, rc, "%d %d %d ", speed, signalCode, counter);
    return CMD_OK;
}

/* Lines Amount (to output) for mcdCheckLinkStatus */
static GT_U32 mcdCheckLinkStatusLanes = 0;
static GT_U32 mcdCheckLinkStatusCurrentLane = 0;
static GT_U32 mcdCheckLinkStatusMcdMode;
static DETAILED_STATUS_TYPE mcdCheckLinkStatusStatusDetail;
static MCD_U16 mcdCheckLinkStatusStatusCurrentStatus;
static MCD_U16 mcdCheckLinkStatusStatusLatchedStatus;


/*********************************************************************************/
static CMD_STATUS  wrCpssPhyMcdCheckLinkStatusReadParseFields
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U32   hostCurrent;

    for (; (mcdCheckLinkStatusCurrentLane < mcdCheckLinkStatusLanes); mcdCheckLinkStatusCurrentLane++)
    {
        switch (mcdCheckLinkStatusMcdMode)
        {
            case 0:
                hostCurrent = mcdCheckLinkStatusStatusDetail.P100_40_status.hostCurrent;
                break;
            case 1:
                hostCurrent =
                    mcdCheckLinkStatusStatusDetail.R25_10_status.hostCurrent[mcdCheckLinkStatusCurrentLane];
                break;
            case 2:
                /* let output all lines - includes pairs 0,4; 1,5; 2,6; 3,7; */
                hostCurrent = 0;
                break;
            default:
                galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong MCD mode \n");
                galtisOutput(outArgs, GT_OK, "%d", -1);
                return CMD_OK;
        }
        /* break on relevant value, for not relevant serdes of retimer modes used -1 */
        if ((hostCurrent == 0) || (hostCurrent == 1)) break;
    }

    if (mcdCheckLinkStatusCurrentLane >= mcdCheckLinkStatusLanes)
    {
        /* end of table */
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    switch (mcdCheckLinkStatusMcdMode)
    {
        case 0:
            fieldOutput(
                "%d %d %d %d %d %d",
                mcdCheckLinkStatusStatusCurrentStatus, mcdCheckLinkStatusStatusLatchedStatus,
                mcdCheckLinkStatusStatusDetail.P100_40_status.hostCurrent,
                mcdCheckLinkStatusStatusDetail.P100_40_status.hostLatched,
                mcdCheckLinkStatusStatusDetail.P100_40_status.lineCurrent,
                mcdCheckLinkStatusStatusDetail.P100_40_status.lineLatched);
            break;
        case 1:
            fieldOutput(
                "%d %d %d %d",
                mcdCheckLinkStatusStatusCurrentStatus, mcdCheckLinkStatusCurrentLane,
                mcdCheckLinkStatusStatusDetail.R25_10_status.hostCurrent[mcdCheckLinkStatusCurrentLane],
                mcdCheckLinkStatusStatusDetail.R25_10_status.lineCurrent[mcdCheckLinkStatusCurrentLane]);
            break;
        case 2:
            fieldOutput(
                "%d %d %d %d %d %d %d %d",
                mcdCheckLinkStatusStatusCurrentStatus, mcdCheckLinkStatusCurrentLane,
                mcdCheckLinkStatusStatusDetail.R100_40_status.host_P0[mcdCheckLinkStatusCurrentLane],
                mcdCheckLinkStatusStatusDetail.R100_40_status.line_P0[mcdCheckLinkStatusCurrentLane],
                mcdCheckLinkStatusStatusDetail.R100_40_status.hostCurrent[mcdCheckLinkStatusCurrentLane],
                mcdCheckLinkStatusStatusDetail.R100_40_status.lineCurrent[mcdCheckLinkStatusCurrentLane],
                mcdCheckLinkStatusStatusDetail.R100_40_status.host_P1[mcdCheckLinkStatusCurrentLane],
                mcdCheckLinkStatusStatusDetail.R100_40_status.line_P1[mcdCheckLinkStatusCurrentLane],
                mcdCheckLinkStatusStatusDetail.R100_40_status.hostCurrent[4+mcdCheckLinkStatusCurrentLane],
                mcdCheckLinkStatusStatusDetail.R100_40_status.lineCurrent[4+mcdCheckLinkStatusCurrentLane]);
            break;
        default:
            galtisOutput(outArgs, (GT_STATUS)GT_ERROR, "\nERROR : Wrong MCD mode \n");
    }

    galtisOutput(outArgs, GT_OK, "%d%f", mcdCheckLinkStatusMcdMode);

    /* Increment current lane */
    mcdCheckLinkStatusCurrentLane++;

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdCheckLinkStatusFirstEntry function
* @endinternal
*
* @brief   This function returns an overall indication if the link is up or down currently in"currentStatus".
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK or MCD_FAIL, if action was successful or not
*
* @note Depending on the mode of operation of this mdioPort, looks at all
*       components of the link on both host and line side and returns an
*       overall indication if the link is up or down currently in
*       "currentStatus". Returns an indication if the link dropped since
*       the last time mcdCheckLinkStatus() was called for those
*       modes that have a latched status (PCS modes and MLG modes).
*       Also returns more details in "statusDetail" in case host or
*       line side details for specific lanes are desired. See
*       DETAILED_STATUS_TYPE for the different structures that will be
*       returned for different modes of operation.
*
*/
static CMD_STATUS wrCpssPhyMcdCheckLinkStatusFirstEntry
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    MCD_U32     phyId;
    MCD_DEV_PTR pDev;
    MCD_U16     mdioPort;
    MCD_STATUS  status;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort = (MCD_U16)inArgs[1];

    switch(pDev->portConfig[mdioPort].portMode)
    {
        /* new modes for X5123 */
        case MCD_MODE_P50R2L:
        case MCD_MODE_P50R2S:
        case MCD_MODE_P40R2L:
        case MCD_MODE_P40R2S:
        case MCD_MODE_G21L:
        case MCD_MODE_G21SK:
        case MCD_MODE_P25L:
        case MCD_MODE_P25S:
        case MCD_MODE_P10L:
        case MCD_MODE_P10S:
        case MCD_MODE_P2_5:
        case MCD_MODE_P2_5S:
        /* legacy modes */
        case MCD_MODE_P100_40_C:
        case MCD_MODE_P100_40_K:
        case MCD_MODE_P100L:
        case MCD_MODE_P100S:
        case MCD_MODE_P100C:
        case MCD_MODE_P100K:
        case MCD_MODE_P40L:
        case MCD_MODE_P40S:
        case MCD_MODE_P40C:
        case MCD_MODE_P40K:
        /* new modes for X5123 - low speed */
        case MCD_MODE_P1:
        case MCD_MODE_P1_SGMII:
        case MCD_MODE_P1_BaseX_SGMII:
            mcdCheckLinkStatusLanes = 1;
            mcdCheckLinkStatusMcdMode = 0;
            break;
        case MCD_MODE_R1L:
        case MCD_MODE_R1C:
        case MCD_MODE_R25L:
        case MCD_MODE_R10L:
        case MCD_MODE_R25C:
        case MCD_MODE_R10K:
            mcdCheckLinkStatusLanes = 8;
            mcdCheckLinkStatusMcdMode = 1;
            break;
        /* legacy repeater modes */
        case MCD_MODE_R100L:
        case MCD_MODE_R40L:
        case MCD_MODE_R100C:
        case MCD_MODE_R40C:
            mcdCheckLinkStatusMcdMode = 2;
            mcdCheckLinkStatusLanes = 4;
            break;
        default:
            galtisOutput(outArgs, MCD_FAIL, "");
            return CMD_OK;
    }

    /* call cpss api function */
    status = mcdCheckLinkStatus(
        pDev, mdioPort, &mcdCheckLinkStatusStatusCurrentStatus,
        &mcdCheckLinkStatusStatusLatchedStatus, &mcdCheckLinkStatusStatusDetail);
    if (status != MCD_OK)
    {
        galtisOutput(outArgs, status, "%f");
        return CMD_OK;
    }

    mcdCheckLinkStatusCurrentLane = 0;

    return wrCpssPhyMcdCheckLinkStatusReadParseFields(inArgs,inFields,outArgs);
}

/*********************************************************************************/
static CMD_STATUS wrCpssPhyMcdCheckLinkStatusNextEntry
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

    return wrCpssPhyMcdCheckLinkStatusReadParseFields(inArgs,inFields,outArgs);
}

/**
* @internal wrCpssPhyMcdGetPcsFaultStatus function
* @endinternal
*
* @brief   This function is to be called in any of the 40G or 100G PCS modes
*         to check the Tx/Rx fault bits.
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK or MCD_FAIL, if action was successful or not
*
* @note Based on the current operating mode, it reads either the 40G
*       or 100G Host/Line PCS Status 2 register twice and returns the value
*       of the Tx/Rx fault bits. It returns the first read in the
*       "latched" version and the second read in the "current" version.
*       For 100G Host Side:
*       4.8.11 Tx Fault
*       4.8.10 Rx Fault
*       For 100G Line Side:
*       3.8.11 Tx Fault
*       3.8.10 Rx Fault
*       For 40G Host Side:
*       4.1008.11 Tx Fault
*       4.1008.10 Rx Fault
*       For 40G Line Side:
*       3.1008.11 Tx Fault
*       4.1008.10 Rx Fault
*       Clears the latch status of those registers being read.
*       Speed must be resolved if using AN and/or port must be configured using
*       40G or 100G PCS mode, otherwise an error is returned.
*
*/
static CMD_STATUS wrCpssPhyMcdGetPcsFaultStatus
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 mdioPort;
    MCD_U16 host_or_line;
    MCD_U16 currentTxFaultStatus;
    MCD_U16 currentRxFaultStatus;
    MCD_U16 latchedTxFaultStatus;
    MCD_U16 latchedRxFaultStatus;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort = (MCD_U16)inArgs[1];
    host_or_line = (MCD_U16)inArgs[2];

    /* call cpss api function */
    status = mcdGetPcsFaultStatus(pDev, mdioPort, host_or_line ,
                                  &currentTxFaultStatus,
                                  &currentRxFaultStatus,
                                  &latchedTxFaultStatus,
                                  &latchedRxFaultStatus);


    /* pack and output table fields */

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d %d %d %d", currentTxFaultStatus, currentRxFaultStatus, latchedTxFaultStatus, latchedRxFaultStatus);

    return CMD_OK;
}

/*******************************************************************************/
static CMD_STATUS wrCpssPhyMcdSetSerdesElectricalParams
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_U8    serdesNum;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    serdesNum = (MCD_U32)inArgs[0];

    serdesElectricalParamsMv[serdesNum].rxPolarity  = (MCD_U8)inArgs[1];;
    serdesElectricalParamsMv[serdesNum].txPolarity  = (MCD_U8)inArgs[2]; ;
    serdesElectricalParamsMv[serdesNum].preCursor = (MCD_U16)inArgs[3];;
    serdesElectricalParamsMv[serdesNum].attenuation   = (MCD_U16)inArgs[4];;
    serdesElectricalParamsMv[serdesNum].postCursor  = (MCD_U16)inArgs[5];;

    return CMD_OK;
}


/******************************************************************************
* mcdSetModeSelectionExt
*
* INPUTS:
*       phyId         - logical number of PHY device, 0..15
*       mdioPort      - MDIO port address, 0-31
*       portMode - operational mode
*       autoNegAdvEnable - Enable Auto Neg
*       fecCorrect - FEC correct for 100G, N/A for other modes
*       configPtr  - the structure contains fields to configure different parameters:
*        noPpmMode should be set to TRUE if there is a need to propagate the line side Rx
*                   clock through the device toward the host side, and use it as Tx clock–
*                   this mode is used in synchronous ethernet applications in which the recovered clock selection
*                   is done on the host side  and not directly from the device.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       MCD_OK or MCD_FAIL, if action was successful or not
*
* COMMENTS:
*
*******************************************************************************/
static CMD_STATUS wrCpssPhyMcdMcdSetModeSelectionExt
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 mdioPort;
    MCD_OP_MODE portMode;
    MCD_BOOL autoNegAdvEnable,setElecticalParams;
    MCD_FEC_TYPE fecCorrect;
    MCD_NO_PPM_MODE noPpmMode;
    MCD_MODE_CONFIG_PARAM config;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }
    memset(&config, 0, sizeof(MCD_MODE_CONFIG_PARAM));

    mdioPort = (MCD_U16)inArgs[1];
    portMode = (MCD_OP_MODE)inArgs[2];
    autoNegAdvEnable = (MCD_BOOL)inArgs[3];
    fecCorrect = (MCD_FEC_TYPE)inArgs[4];
    noPpmMode = (MCD_NO_PPM_MODE)inArgs[5];
    setElecticalParams = (MCD_BOOL)inArgs[6];

    config.noPpmMode = noPpmMode;
    config.refClk.lsRefClkSel = MCD_PRIMARY_CLK;
    config.refClk.hsRefClkSel = MCD_PRIMARY_CLK;
    config.refClk.lsRefClkDiv = MCD_REF_CLK_NO_DIV;
    config.refClk.hsRefClkDiv = MCD_REF_CLK_NO_DIV;

    config.ctleParamsPtr = NULL;

    if (setElecticalParams)
    {
        config.electricalParamsPtr = (MCD_PORT_PER_SERDES_CONFIG_DATA*)&serdesElectricalParamsMv;
    }
    else
        config.electricalParamsPtr = NULL;
    /* call cpss api function */
    status = mcdSetModeSelectionExt(pDev, mdioPort, portMode, autoNegAdvEnable, fecCorrect, &config);
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/******************************************************************************
MCD_STATUS wrCpssPhyMcdSetLaneSteering

 Description:
    Set port with laneSteering/Remap mode, support AP and none AP port.
 Inputs:
    pDev - pointer to MCD_DEV initialized by mcdInitDriver() call
    mdioPort - MDIO port address 0...31 of mdioPort to be configured
    portMode - operational mode
    masterSlice - active slice
    laneSteeringMode - MCD_P100G_STEERING,
                       MCD_P40G_STEERING,
                       MCD_R4_P25G_STEERING,
                       MCD_R4_P10G_STEERING,
                       MCD_P100G_P40G_P10G_STEERING_AP,
                       MCD_R4_P25G_STEERING_AP,
                       MCD_R4_P10G_STEERING_AP,
                       MCD_G21L_NONE_STEERING,
                       MCD_G21L_P10G_NONE_STEERING_AP
    fecCorrect - FEC correct
    reducedRxTraining - bypass HF/BW values



 Outputs:
    None

 Returns:
    MCD_OK or MCD_FAIL, if action was successful or not

 Notes/Warnings:

******************************************************************************/
MCD_STATUS wrCpssPhyMcdSetLaneSteering
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS status;
    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 mdioPort;
    MCD_MASTER_SLICE masterSlice;
    MCD_LANE_STEERING_MODE laneSteeringMode;
    MCD_FEC_TYPE fecCorrect;
    MCD_U32 modeVector;
    MCD_BOOL autoNegAdvEnable;
    MCD_BOOL reducedRxTraining;
    MCD_BOOL setElectricalParams, setCtleParams;

    MCD_MODE_CONFIG_PARAM   config;
    MCD_CONFIG_AP_PARAM     configAp;
    MCD_CONFIG_SERDES_AP_PARAM configApSerdes;
    MCD_U32 i, laneNum;
    MCD_U32 polarityVector = 0;
    MCD_U32 overrideRemapConf;

        /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    memset(&config, 0, sizeof(MCD_MODE_CONFIG_PARAM));
    memset(&configAp, 0, sizeof(MCD_CONFIG_AP_PARAM));
    memset(&configApSerdes, 0, sizeof(MCD_CONFIG_SERDES_AP_PARAM));

    mdioPort = (MCD_U16)inArgs[1];
    masterSlice = (MCD_MASTER_SLICE)inArgs[2];
    laneSteeringMode = (MCD_LANE_STEERING_MODE)inArgs[3];
    fecCorrect = (MCD_FEC_TYPE)inArgs[4];
    modeVector = (MCD_U32)inArgs[5];
    autoNegAdvEnable = (MCD_BOOL)inArgs[6];
    reducedRxTraining = (MCD_BOOL)inArgs[7];
    setElectricalParams = (MCD_BOOL)inArgs[8];
    setCtleParams = (MCD_BOOL)inArgs[9];
    laneNum       = (MCD_U32)inArgs[10];
    overrideRemapConf = (MCD_BOOL)inArgs[11];


    if (autoNegAdvEnable)
    {
        config.configApPtr = (MCD_CONFIG_AP_PARAM *)&configAp;
        config.configApPtr->laneNum = laneNum;
        configApSerdes.params = (MCD_AP_SERDES_CONFIG_DATA* )&serdesApParameters;
        configApSerdes.numOfLanes = 4;
        for(i = 0; i < 4; i++)
        {
            serdesApParameters[i].txPolarity = (polarityVector  >> (2*i+1)) & 1;
            serdesApParameters[i].rxPolarity = (polarityVector  >> (2*i)) & 1;
        }
        config.configApPtr->configApSerdesPtr = (MCD_CONFIG_SERDES_AP_PARAM*)&configApSerdes;
        if ((laneSteeringMode == MCD_4P_P1G_1000BaseX_STEERING) ||
            (laneSteeringMode == MCD_4P_P1G_SGMII_STEERING))
        {
            config.configApPtr->apEnable = MCD_TRUE;
        }
        if ((autoNegAdvEnable == MCD_TRUE) && (modeVector != 0))
        {
            config.configApPtr->apEnable = MCD_TRUE;
            config.configApPtr->fcAsmDir = MCD_FALSE;
            config.configApPtr->fcPause = MCD_FALSE;
            config.configApPtr->fecReq = fecCorrect;
            config.configApPtr->fecSup = fecCorrect;
            config.configApPtr->fecReqConsortium = fecCorrect;
            config.configApPtr->fecSupConsortium = fecCorrect;
            config.configApPtr->modeVector = modeVector;
            config.configApPtr->nonceDis = MCD_TRUE;
            config.configApPtr->retimerEnable = MCD_FALSE;
            config.configApPtr->ctleBiasVal = MCD_CTLE_BIAS_NORMAL;
            if (laneSteeringMode == MCD_G21L_P10G_NONE_STEERING_AP)
            {
                config.configApPtr->g21Mode = 1;
            }
        }
    }

    config.ctleParamsPtr = NULL;
    config.electricalParamsPtr = NULL;
    config.noPpmMode = MCD_NO_PPM_OFF_MODE;
    config.ctleBiasParams.host = MCD_CTLE_BIAS_NORMAL;
    config.ctleBiasParams.line = MCD_CTLE_BIAS_NORMAL;
    config.refClk.lsRefClkSel = MCD_PRIMARY_CLK;
    config.refClk.hsRefClkSel = MCD_PRIMARY_CLK;
    config.refClk.lsRefClkDiv = MCD_REF_CLK_NO_DIV;
    config.refClk.hsRefClkDiv = MCD_REF_CLK_NO_DIV;

    if (setElectricalParams)
    {
        config.electricalParamsPtr = (MCD_PORT_PER_SERDES_CONFIG_DATA*)&serdesElectricalParamsMv;
    }
    else
    {
        config.electricalParamsPtr = NULL;
    }

    if (setCtleParams)
    {
        config.ctleParamsPtr = (MCD_PORT_PER_SERDES_CTLE_CONFIG_DATA*)&serdesCtleParamsMv;
    }
    else
    {
        config.ctleParamsPtr = NULL;
    }

    if (overrideRemapConf)
    {
        config.laneSteerCfgPtr = (MCD_LANE_STEERING_OVERRIDE_CFG*)&laneSteeringOverrideMv;
    }
    else
    {
        config.laneSteerCfgPtr = NULL;
    }

    status = mcdSetLaneSteering(pDev, mdioPort, masterSlice,
                               laneSteeringMode, fecCorrect, reducedRxTraining, &config);
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdSetAutoNegAdvertisingSpeeds function
* @endinternal
*
* @brief   This function sets the auto-negotiation local advertising speeds only.
*         It does not start the auto negotiation. To start auto-negotiation, function
*         mcdAutoNegStart() needs to be called.
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK or MCD_FAIL, if action was successful or not
*/
static CMD_STATUS wrCpssPhyMcdSetAutoNegAdvertisingSpeeds
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 mdioPort;
    MCD_U16 host_or_line;
    MCD_U16 speed_bits;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort = (MCD_U16)inArgs[1];
    host_or_line = (MCD_U16 )inArgs[2];
    speed_bits = (MCD_U16 )inArgs[3];

    /* call cpss api function */
    status = mcdSetAutoNegAdvertisingSpeeds(pDev, mdioPort, host_or_line, speed_bits);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdGetAutoNegAdvertisingSpeeds function
* @endinternal
*
* @brief   This function returns the speeds that have been advertised.
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK or MCD_FAIL, if action was successful or not
*
* @note Speeds advertised don't take effect until auto-negotiation is restarted.
*
*/
static CMD_STATUS wrCpssPhyMcdGetAutoNegAdvertisingSpeeds
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 mdioPort;
    MCD_U16 host_or_line;
    MCD_U16 speed_bits = 0;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort = (MCD_U16)inArgs[1];
    host_or_line = (MCD_U16 )inArgs[2];

    /* call cpss api function */
    status = mcdGetAutoNegAdvertisingSpeeds(pDev, mdioPort, host_or_line, &speed_bits);

    inFields[0] = speed_bits;

    /* pack and output table fields */

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d", speed_bits);

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdAutoNegStart function
* @endinternal
*
* @brief   Start or Restarts auto-negotiation. The bit is self-clearing.
*         If the link is up,the link will drop and auto-negotiation will start again.
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK or MCD_FAIL, if action was successful or not
*
* @note Restarting auto-negotiation will enable the auto-negotiation if auto-negotiation is
*       disabled.
*       This function is important as it is necessary to start or restart auto-negotiation
*       after changing many auto-negotiation settings before the changes will take
*       effect.
*
*/
static CMD_STATUS wrCpssPhyMcdAutoNegStart
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 mdioPort;
    MCD_U16 host_or_line;
    MCD_BOOL swReset;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort = (MCD_U16)inArgs[1];
    host_or_line = (MCD_U16 )inArgs[2];
    swReset = (MCD_BOOL)inArgs[3];

    /* call cpss api function */
    status = mcdAutoNegStart(pDev, mdioPort, host_or_line, swReset);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdSetRsFecControl function
* @endinternal
*
* @brief   Enable or disable the RS-FEC bypass indication and bypass correction
*         Register name: RS-FEC control register, Device 1 Register 0x00C8
*
* @note   APPLICABLE DEVICES:      ???
*
* @retval MCD_OK if RS             -FEC configuration is successful, MCD_FAIL if not
*/
static CMD_STATUS wrCpssPhyMcdSetRsFecControl
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 mdioPort;
    MCD_U16 bypassIndicationEnable;
    MCD_U16 bypassCorrectionEnable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort = (MCD_U16)inArgs[1];
    bypassIndicationEnable = (MCD_U16)inArgs[2];
    bypassCorrectionEnable = (MCD_U16)inArgs[3];

    /* call cpss api function */
    status = mcdSetRsFecControl(pDev, mdioPort, bypassIndicationEnable, bypassCorrectionEnable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdGetRsFecStatus function
* @endinternal
*
* @brief   Reads and returns the value of the RS-FEC status register
*         Register name: RS-FEC status register, Device 1 Register 0x00C9
*
* @note   APPLICABLE DEVICES:      ???
*
* @retval MCD_OK if RS             -FEC status is successful, MCD_FAIL if not
*/
static CMD_STATUS wrCpssPhyMcdGetRsFecStatus
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 mdioPort;
    MCD_U16 pcsLaneAlignment;
    MCD_U16 fecLaneAlignment;
    MCD_U16 latchedRsFecHighErr;
    MCD_U16 currRsFecHighErr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort = (MCD_U16)inArgs[1];

    /* call cpss api function */
    status = mcdGetRsFecStatus(pDev, mdioPort, &pcsLaneAlignment, &fecLaneAlignment, &latchedRsFecHighErr, &currRsFecHighErr);

    /* pack and output table fields */

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d %d %d %d", pcsLaneAlignment, fecLaneAlignment, latchedRsFecHighErr, currRsFecHighErr);

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdGetRsFecPCSAlignmentStatus function
* @endinternal
*
* @brief   Get RS-FEC PCS Alignment status per lane.
*         Register name: RS-FEC PCS Alignment status 1 - 3,
*         Device 1, Register 0x0118, 0x0119, 0x011A, 0x011B
*
* @note   APPLICABLE DEVICES:      ???
*
* @retval MCD_OK if read RS        -FEC PCS Aligment status is successful, MCD_FAIL if not
*/
static CMD_STATUS wrCpssPhyMcdGetRsFecPCSAlignmentStatus
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 mdioPort;
    MCD_U16 pcs_lane;
    MCD_U16 blockLocked = 0;
    MCD_U16 laneAligned = 0;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort = (MCD_U16)inArgs[1];
    pcs_lane = (MCD_U16)inArgs[2];

    /* call cpss api function */
    status = mcdGetRsFecPCSAlignmentStatus(pDev, mdioPort, pcs_lane, &blockLocked, &laneAligned);

    /* pack and output table fields */

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d %d", blockLocked, laneAligned);

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdGetRsFecPMALaneMapping function
* @endinternal
*
* @brief   Get RS-FEC lane to PMA lane mapping
*         Register name: RS-FEC lane mapping register, Device 1, Register 0x00CE
*
* @note   APPLICABLE DEVICES:      ???
*
* @retval MCD_OK if read RS        -FEC PMA lane mapping is successful, MCD_FAIL if not
*/
static CMD_STATUS wrCpssPhyMcdGetRsFecPMALaneMapping
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 mdioPort;
    MCD_U16 mapping[MCD_NUM_LANES];

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort = (MCD_U16)inArgs[1];

    /* call cpss api function */
    status = mcdGetRsFecPMALaneMapping(pDev, mdioPort, mapping);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d %d %d %d", mapping[0], mapping[1], mapping[2], mapping[3]);

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdGetRsFecPCSLaneMapping function
* @endinternal
*
* @brief   Get RS-FEC PCS lane mapping received in service interface lane #
*         Register name: RS-FEC PCS lane # mapping register
*         Device 1, Register 0x00FA - 0x010D
*
* @note   APPLICABLE DEVICES:      ???
*
* @retval MCD_OK if read RS        -FEC PMA lane mapping is successful, MCD_FAIL if not
*/
static CMD_STATUS wrCpssPhyMcdGetRsFecPCSLaneMapping
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 mdioPort;
    MCD_U16 pcs_lane;
    MCD_U16 interface_lane = 0;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort = (MCD_U16)inArgs[1];
    pcs_lane = (MCD_U16)inArgs[2];

    /* call cpss api function */
    status = mcdGetRsFecPCSLaneMapping(pDev, mdioPort, pcs_lane, &interface_lane);

    /* pack and output table fields */

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d", interface_lane);

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdGetRsFecCorrectedCwCntr function
* @endinternal
*
* @brief   Get RS-FEC corrected codewords counter
*         Register name: RS-FEC corrected codewords counter lower
*         RS-FEC corrected codewords counter upper
*         Device 1, Register 0x00CA, 0x00CB
*
* @note   APPLICABLE DEVICES:      ???
*
* @retval MCD_OK if read RS        -FEC PMA lane mapping is successful, MCD_FAIL if not
*/
static CMD_STATUS wrCpssPhyMcdGetRsFecCorrectedCwCntr
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 mdioPort;
    MCD_U32 codeWordCounter = 0;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort = (MCD_U16)inArgs[1];

    /* call cpss api function */
    status = mcdGetFecCorrectedCwCntr(pDev, mdioPort, MCD_LINE_SIDE, MCD_RS_FEC, &codeWordCounter);

    /* pack and output table fields */

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d", codeWordCounter);

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdGetRsFecUnCorrectedCwCntr function
* @endinternal
*
* @brief   Get RS-FEC uncorrected codewords counter
*         Register name: RS-FEC Uncorrected codewords counter lower
*         RS-FEC Uncorrected codewords counter upper
*         Device 1, Register 0x00CC, 0x00CD
*
* @note   APPLICABLE DEVICES:      ???
*
* @retval MCD_OK if read RS        -FEC PMA lane mapping is successful, MCD_FAIL if not
*/
static CMD_STATUS wrCpssPhyMcdGetRsFecUnCorrectedCwCntr
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 mdioPort;
    MCD_U32 codeWordCounter = 0;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort = (MCD_U16)inArgs[1];

    /* call cpss api function */
    status = mcdGetFecUnCorrectedCwCntr(pDev, mdioPort, MCD_LINE_SIDE, MCD_RS_FEC, &codeWordCounter);

    /* pack and output table fields */

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d", codeWordCounter);

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdGetRsFecSymbolErrorCntr function
* @endinternal
*
* @brief   Get RS-FEC symbol error counter per lane
*         Register name: RS-FEC symbol error counter lower lane #
*         RS-FEC symbol error counter upper lane #
*         Device 1, Register 0x00D2, 0x00D3 - 0x00D8 0x00D9
*
* @note   APPLICABLE DEVICES:      ???
*
* @retval MCD_OK if read RS        -FEC PMA lane mapping is successful, MCD_FAIL if not
*/
static CMD_STATUS wrCpssPhyMcdGetRsFecSymbolErrorCntr
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 mdioPort;
    MCD_U16 lane;
    MCD_U32 errorCounter = 0;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort = (MCD_U16)inArgs[1];
    lane = (MCD_U16)inArgs[2];

    /* call cpss api function */
    status = mcdGetFecSymbolErrorCntr(pDev, mdioPort, lane, MCD_LINE_SIDE, MCD_RS_FEC, &errorCounter);

    /* pack and output table fields */
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d", errorCounter);

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdCheckChipCapablities function
* @endinternal
*
* @brief   Checks the chip capablities with the provided port mode selection. If the
*         mode selection is not supported, it will return a MCD_FAIL status. Otherwise,
*         a MCD_OK status will be returned.
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK if portMode is supported, otherwise, returns MCD_FAIL
*/
static CMD_STATUS wrCpssPhyMcdCheckChipCapablities
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_OP_MODE portMode;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    portMode = (MCD_OP_MODE)inArgs[1];

    /* call cpss api function */
    status = mcdCheckChipCapablities(pDev, portMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdGetChipRevision function
* @endinternal
*
* @brief   Determines the PHY revision and returns the value in phyRev.
*         See definition of MCD_DEVICE_ID for a list of available
*         devices and capabilities.
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK if query was successful, MCD_FAIL if not
*/
static CMD_STATUS wrCpssPhyMcdGetChipRevision
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 mdioPort;
    MCD_DEVICE_ID deviceId;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort = (MCD_U16)inArgs[1];

    /* call cpss api function */
    status = mcdGetChipRevision(pDev, mdioPort, &deviceId);

    /* pack and output table fields */

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d", deviceId);

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdGetChipFWRevision function
* @endinternal
*
* @brief   Retrieves the chip firmware revision number. The revision is in
*         the <major>.<minor> format.
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK if query was successful, MCD_FAIL if not
*/
static CMD_STATUS wrCpssPhyMcdGetChipFWRevision
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 major = 0;
    MCD_U16 minor = 0;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    /* call cpss api function */
    status = mcdGetChipFWRevision(pDev, &major, &minor);


    /* pack and output table fields */

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d %d", major, minor);

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdGetSerdesSignalDetectAndDspLock function
* @endinternal
*
* @brief   Reads the corresponding lane signal detect bit and returns it in
*         signalDetect. Also reads the corresponding dspLock and returns it
*         in dspLock.
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK if query was successful, MCD_FAIL if not
*
* @note Signal detect is "Serdes Line Signal OK" field.
*       Dsp lock is "Channel 0 Link is Up" field.
*       See "UMAC/Slice %m/umac3gbsx4/glbl/livelnkstat0"
*
*/
static CMD_STATUS wrCpssPhyMcdGetSerdesSignalDetectAndDspLock
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 mdioPort;
    MCD_U16 host_or_line;
    MCD_U16 laneOffset;

    MCD_U16 signalDetect;
    MCD_U16 dspLock;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort = (MCD_U16)inArgs[1];
    host_or_line = (MCD_U16)inArgs[2];
    laneOffset = (MCD_U16)inArgs[3];

    /* call cpss api function */
    status = mcdGetSerdesSignalDetectAndDspLock(pDev, mdioPort, host_or_line, laneOffset, &signalDetect, &dspLock);


    /* pack and output table fields */

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d %d", signalDetect, dspLock);

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdSetLineLoopback function
* @endinternal
*
* @brief   This function is used to Enable/Disable line side loopbacks
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK if query was successful, MCD_FAIL if not
*/
static CMD_STATUS wrCpssPhyMcdSetLineLoopback
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 mdioPort;
    MCD_U16 loopback_type;
    MCD_U16 enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort = (MCD_U16)inArgs[1];
    loopback_type = (MCD_U16)inArgs[2];
    enable = (MCD_U16)inArgs[3];

    /* call cpss api function */
    status = mcdSetLineLoopback(pDev, mdioPort, loopback_type, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdSetHostLoopback function
* @endinternal
*
* @brief   This function is used to Enable/Disable host side loopbacks
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK if query was successful, MCD_FAIL if not
*/
static CMD_STATUS wrCpssPhyMcdSetHostLoopback
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 mdioPort;
    MCD_U16 loopback_type;
    MCD_U16 enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort = (MCD_U16)inArgs[1];
    loopback_type = (MCD_U16)inArgs[2];
    enable = (MCD_U16)inArgs[3];

    /* call cpss api function */
    status = mcdSetHostLoopback(pDev, mdioPort, loopback_type, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdConfigurePktGeneratorChecker function
* @endinternal
*
* @brief   This function is used to configure the packet generator/checker.
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK if query was successful, MCD_FAIL if not
*
* @note Call mcdEnablePktGeneratorChecker() to enable/start the generator/checker.
*
*/
static CMD_STATUS wrCpssPhyMcdConfigurePktGeneratorChecker
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16  mdioPort;
    MCD_U16  host_or_line;
    MCD_BOOL readToClear;
    MCD_BOOL dontuseSFDinChecker;
    MCD_U16  pktPatternControl;
    MCD_BOOL generateCRCoff;
    MCD_U32  initialPayload;
    MCD_U16  frameLengthControl;
    MCD_U16  numPktsToSend;
    MCD_BOOL randomIPG;
    MCD_U16  ipgDuration;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort = (MCD_U16)inArgs[1];
    host_or_line = (MCD_U16)inArgs[2];
    readToClear = (MCD_BOOL)inArgs[3];
    dontuseSFDinChecker = (MCD_BOOL)inArgs[4];
    pktPatternControl = (MCD_U16)inArgs[5];
    generateCRCoff = (MCD_BOOL)inArgs[6];
    initialPayload = (MCD_U32)inArgs[7];
    frameLengthControl = (MCD_U16)inArgs[8];
    numPktsToSend = (MCD_U16)inArgs[9];
    randomIPG = (MCD_BOOL)inArgs[10];
    ipgDuration = (MCD_U16)inArgs[11];

    /* call cpss api function */
    status = mcdConfigurePktGeneratorChecker(pDev, mdioPort, host_or_line,
                                             readToClear, dontuseSFDinChecker,
                                             pktPatternControl, generateCRCoff,
                                             initialPayload, frameLengthControl,
                                             numPktsToSend, randomIPG, ipgDuration);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdEnablePktGeneratorChecker function
* @endinternal
*
* @brief   This function enables/disables either the packet generator or packet checker (or both).
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK if query was successful, MCD_FAIL if not
*/
static CMD_STATUS wrCpssPhyMcdEnablePktGeneratorChecker
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16  mdioPort;
    MCD_U16  host_or_line;
    MCD_BOOL enableGenerator;
    MCD_BOOL enableChecker;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort = (MCD_U16)inArgs[1];
    host_or_line = (MCD_U16)inArgs[2];
    enableGenerator = (MCD_BOOL)inArgs[3];
    enableChecker = (MCD_BOOL)inArgs[4];

    /* call cpss api function */
    status = mcdEnablePktGeneratorChecker(pDev, mdioPort, host_or_line,
                                          enableGenerator, enableChecker);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdPktGeneratorCounterReset function
* @endinternal
*
* @brief   This function explicitly clears all the counters when the packet
*         generator/checker has been setup to be cleared by writing a
*         bit to the control register instead of being cleared when
*         the counter(s) are read.
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK if query was successful, MCD_FAIL if not
*
* @note This function operates on the counters.
*       When this function is called (and 3.F010.15/4.F010.15 is 0)
*       the transmit, receive, error and link drop counters will be cleared.
*       This function assumes the generator/checker has been configured to
*       be cleared by bit 3.F010.6/4.F010.6 by previously passing
*       MCD_FALSE for parameter readToClear in function
*       mcdConfigurePktGeneratorChecker() (thus setting bit F010.15 <= 0).
*
*/
static CMD_STATUS wrCpssPhyMcdPktGeneratorCounterReset
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16  mdioPort;
    MCD_U16  host_or_line;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort = (MCD_U16)inArgs[1];
    host_or_line = (MCD_U16)inArgs[2];

    /* call cpss api function */
    status = mcdPktGeneratorCounterReset(pDev, mdioPort, host_or_line);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdPktGeneratorGetCounter function
* @endinternal
*
* @brief   This function is used to read the transmit/receive/error counter for the
*         packet generator/checker.
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK if query was successful, MCD_FAIL if not
*
* @note TbyteCount is always 0 for MCD_PKT_GET_ERR, since the error counter
*       only counts packets.
*       If packet generator/checker was configured to clear counter(s) on read,
*       this function will clear the counter being read.
*
*/
static CMD_STATUS wrCpssPhyMcdPktGeneratorGetCounter
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16  mdioPort;
    MCD_U16  host_or_line;
    MCD_U16  whichCounter;
    MCD_U64  packetCount = 0;
    MCD_U64  byteCount = 0;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort = (MCD_U16)inArgs[1];
    host_or_line = (MCD_U16)inArgs[2];
    whichCounter = (MCD_U16)inArgs[3];

    /* call cpss api function */
    status = mcdPktGeneratorGetCounter(pDev, mdioPort, host_or_line, whichCounter, &packetCount, &byteCount);


    /* pack and output table fields */
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d %d %d %d", (MCD_U32)packetCount, packetCount >> 32, (MCD_U32)byteCount, byteCount >> 32);

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdSetLineSidePRBSPattern function
* @endinternal
*
* @brief   This function is used to select the type of PRBS pattern desired
*         for the line side PRBS in registers 3.F1X0.3:0.
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK if query was successful, MCD_FAIL if not
*/
static CMD_STATUS wrCpssPhyMcdSetLineSidePRBSPattern
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16  mdioPort;
    MCD_U16  laneOffset;
    MCD_PRBS_LINE_SELECTOR_TYPE pattSel;
    MCD_PATTERN_LINE_AB_SELECTOR_TYPE pattSubSel;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort = (MCD_U16)inArgs[1];
    laneOffset = (MCD_U16)inArgs[2];
    pattSel = (MCD_PRBS_LINE_SELECTOR_TYPE)inArgs[3];
    pattSubSel = (MCD_PATTERN_LINE_AB_SELECTOR_TYPE)inArgs[4];

    /* call cpss api function */
    status = mcdSetLineSidePRBSPattern(pDev, mdioPort, laneOffset, pattSel, pattSubSel);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdSetPRBSEnableTxRx function
* @endinternal
*
* @brief   This function is used to start or stop the PRBS transmit and/or
*         receiver.
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK if query was successful, MCD_FAIL if not
*
* @note The channel line rate must have previously been setup by setting the
*       portMode before starting the transmitter.
*
*/
static CMD_STATUS wrCpssPhyMcdSetPRBSEnableTxRx
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16  mdioPort;
    MCD_U16  host_or_line;
    MCD_U16  laneOffset;
    MCD_U16  txEnable;
    MCD_U16  rxEnable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort = (MCD_U16)inArgs[1];
    host_or_line = (MCD_U16)inArgs[2];
    laneOffset = (MCD_U16)inArgs[3];
    txEnable = (MCD_U16)inArgs[4];
    rxEnable = (MCD_U16)inArgs[5];

    /* call cpss api function */
    status = mcdSetPRBSEnableTxRx(pDev, mdioPort, host_or_line, laneOffset, txEnable, rxEnable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdPRBSCounterReset function
* @endinternal
*
* @brief   This function is used to reset the counters when the PRBS has been
*         setup for manual clearing instead of clear-on-read. Default
*         is to use manual clearing. Call mcdSetPRBSEnableClearOnRead() to
*         enable clearing the counters when reading the registers.
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK if query was successful, MCD_FAIL if not
*
* @note Assumes the PRBS has not been setup for clear-on-read.
*
*/
static CMD_STATUS wrCpssPhyMcdPRBSCounterReset
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16  mdioPort;
    MCD_U16  host_or_line;
    MCD_U16  laneOffset;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort = (MCD_U16)inArgs[1];
    host_or_line = (MCD_U16)inArgs[2];
    laneOffset = (MCD_U16)inArgs[3];

    /* call cpss api function */
    status = mcdPRBSCounterReset(pDev, mdioPort, host_or_line, laneOffset);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdSetPRBSWaitForLock function
* @endinternal
*
* @brief   TConfigures PRBS to wait for locking before counting, or to wait for
*         locking before counting.
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK if query was successful, MCD_FAIL if not
*
* @note Should be called before starting the receiver.
*
*/
static CMD_STATUS wrCpssPhyMcdSetPRBSWaitForLock
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16  mdioPort;
    MCD_U16  host_or_line;
    MCD_U16  laneOffset;
    MCD_U16  disableWaitforLock;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort = (MCD_U16)inArgs[1];
    host_or_line = (MCD_U16)inArgs[2];
    laneOffset = (MCD_U16)inArgs[3];
    disableWaitforLock = (MCD_U16)inArgs[4];
    /* call cpss api function */
    status = mcdSetPRBSWaitForLock(pDev, mdioPort, host_or_line, laneOffset, disableWaitforLock);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdSetPRBSClearOnRead function
* @endinternal
*
* @brief   The default for the PRBS counters is to be cleared manually by
*         calling mcdPRBSCounterReset(). This configures either to read-clear,or
*         manual clear (by setting a register bit).
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK if query was successful, MCD_FAIL if not
*
* @note Should be called before starting the receiver.
*
*/
static CMD_STATUS wrCpssPhyMcdSetPRBSClearOnRead
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16  mdioPort;
    MCD_U16  host_or_line;
    MCD_U16  laneOffset;
    MCD_U16  enableReadClear;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort = (MCD_U16)inArgs[1];
    host_or_line = (MCD_U16)inArgs[2];
    laneOffset = (MCD_U16)inArgs[3];
    enableReadClear = (MCD_U16)inArgs[4];

    /* call cpss api function */
    status = mcdSetPRBSClearOnRead(pDev, mdioPort, host_or_line, laneOffset, enableReadClear);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdGetPRBSClearOnRead function
* @endinternal
*
* @brief   Checks whether the PRBS is configured to clear-on-read (1) or manual
*         cleared (0).
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK if query was successful, MCD_FAIL if not
*/
static CMD_STATUS wrCpssPhyMcdGetPRBSClearOnRead
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16  mdioPort;
    MCD_U16  host_or_line;
    MCD_U16  laneOffset;
    MCD_U16  enableReadClear = 0;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort = (MCD_U16)inArgs[1];
    host_or_line = (MCD_U16)inArgs[2];
    laneOffset = (MCD_U16)inArgs[3];

    /* call cpss api function */
    status = mcdGetPRBSClearOnRead(pDev, mdioPort, host_or_line, laneOffset, &enableReadClear);


    /* pack and output table fields */
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d", enableReadClear);

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdGetPRBSLocked function
* @endinternal
*
* @brief   Returns the indicator if the PRBS receiver is locked or not.
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK if query was successful, MCD_FAIL if not
*
* @note Should be called after starting the receiver.
*
*/
static CMD_STATUS wrCpssPhyMcdGetPRBSLocked
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16  mdioPort;
    MCD_U16  host_or_line;
    MCD_U16  laneOffset;
    MCD_BOOL prbsLocked = MCD_FALSE;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort = (MCD_U16)inArgs[1];
    host_or_line = (MCD_U16)inArgs[2];
    laneOffset = (MCD_U16)inArgs[3];

    /* call cpss api function */
    status = mcdGetPRBSLocked(pDev, mdioPort, host_or_line, laneOffset, &prbsLocked);


    /* pack and output table fields */
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d", prbsLocked);

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdGetPRBSCounts function
* @endinternal
*
* @brief   Returns the 48-bit results in the output parameters above. If the PRBS
*         control has been set to clear-on-read, the registers will clear. If
*         not, they must be cleared manually by calling mcdPRBSCounterReset()
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK if query was successful, MCD_FAIL if not
*/
static CMD_STATUS wrCpssPhyMcdGetPRBSCounts
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16  mdioPort;
    MCD_U16  host_or_line;
    MCD_U16  laneOffset;
    MCD_U64  txBitCount = 0;
    MCD_U64  rxBitCount = 0;
    MCD_U64  rxBitErrorCount = 0;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort = (MCD_U16)inArgs[1];
    host_or_line = (MCD_U16)inArgs[2];
    laneOffset = (MCD_U16)inArgs[3];

    /* call cpss api function */
    status = mcdGetPRBSCounts(pDev, mdioPort, host_or_line, laneOffset, &txBitCount,&rxBitCount, &rxBitErrorCount);


    /* pack and output table fields */
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d %d %d %d %d %d",
                (MCD_U32)txBitCount, (MCD_U32)(txBitCount >> 32),
                (MCD_U32)rxBitCount, (MCD_U32)(rxBitCount >> 32),
                (MCD_U32)rxBitErrorCount, (MCD_U32)(rxBitErrorCount >> 32));

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdSetTxPolarity function
* @endinternal
*
* @brief   This function set the Tx polarity with the given input parameters. To invert
*         the polarity on a given lane, set the polarity to 1 and issued a software
*         host or line reset(swReset:1). The reset is issued to the entire host or line
*         on a MDIO port.
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK if query was successful, MCD_FAIL if not
*
* @note The link will be down if the Tx and Rx polarities are mismatched. The set
*       polarity will take effects after a software reset is issued.
*
*/
static CMD_STATUS wrCpssPhyMcdSetTxPolarity
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16  mdioPort;
    MCD_U16  host_or_line;
    MCD_U16  laneOffset;
    MCD_U16 polarity;
    MCD_U16 swReset;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort = (MCD_U16)inArgs[1];
    host_or_line = (MCD_U16)inArgs[2];
    laneOffset = (MCD_U16)inArgs[3];
    polarity = (MCD_U16)inArgs[4];
    swReset = (MCD_U16)inArgs[5];

    /* call cpss api function */
    status = mcdSetTxPolarity(pDev, mdioPort, host_or_line, laneOffset, polarity, swReset);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdGetTxPolarity function
* @endinternal
*
* @brief   This function reads the Tx polarity with the given input parameters.
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK if query was successful, MCD_FAIL if not
*
* @note The link will be down if the Tx and Rx polarities are mismatched.
*
*/
static CMD_STATUS wrCpssPhyMcdGetTxPolarity
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16  mdioPort;
    MCD_U16  host_or_line;
    MCD_U16  laneOffset;
    MCD_U16 polarity = 0;



    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort = (MCD_U16)inArgs[1];
    host_or_line = (MCD_U16)inArgs[2];
    laneOffset = (MCD_U16)inArgs[3];

    /* call cpss api function */
    status = mcdGetTxPolarity(pDev, mdioPort, host_or_line, laneOffset, &polarity);


    /* pack and output table fields */
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d", polarity);

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdSetRxPolarity function
* @endinternal
*
* @brief   This function set the Rx polarity with the given input parameters. To invert
*         the polarity on a given lane, set the polarity to 1 and issued a software
*         host or line reset(swReset:1). The reset is issued to the entire host or line
*         on a MDIO port.
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK if query was successful, MCD_FAIL if not
*
* @note The link will be down if the Tx and Rx polarities are mismatched. The set
*       polarity will take effects after a software reset is issued.
*
*/
static CMD_STATUS wrCpssPhyMcdSetRxPolarity
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16  mdioPort;
    MCD_U16  host_or_line;
    MCD_U16  laneOffset;
    MCD_U16 polarity;
    MCD_U16 swReset;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort = (MCD_U16)inArgs[1];
    host_or_line = (MCD_U16)inArgs[2];
    laneOffset = (MCD_U16)inArgs[3];
    polarity = (MCD_U16)inArgs[4];
    swReset = (MCD_U16)inArgs[5];

    /* call cpss api function */
    status = mcdSetRxPolarity(pDev, mdioPort, host_or_line, laneOffset, polarity, swReset);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdGetRxPolarity function
* @endinternal
*
* @brief   This function reads the Rx polarity with the given input parameters.
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK if query was successful, MCD_FAIL if not
*
* @note The link will be down if the Tx and Rx polarities are mismatched.
*
*/
static CMD_STATUS wrCpssPhyMcdGetRxPolarity
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16  mdioPort;
    MCD_U16  host_or_line;
    MCD_U16  laneOffset;
    MCD_U16 polarity = 0;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort = (MCD_U16)inArgs[1];
    host_or_line = (MCD_U16)inArgs[2];
    laneOffset = (MCD_U16)inArgs[3];

    /* call cpss api function */
    status = mcdGetRxPolarity(pDev, mdioPort, host_or_line, laneOffset, &polarity);


    /* pack and output table fields */
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d", polarity);

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdSetTxFFE function
* @endinternal
*
* @brief   This function set the Tx FFE with the given input parameters. To change
*         the Tx FFE on a given lane, provide the preCursor, attenuation and postCursor.
*         with the reset enabled bit set(swReset:1). The reset is issued to the entire
*         host or line on a MDIO port.
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK if query was successful, MCD_FAIL if not
*
* @note The link will be down if the Tx and Rx polarities are mismatched.
*
*/
static CMD_STATUS wrCpssPhyMcdSetTxFFE
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 mdioPort;
    MCD_U16 host_or_line;
    MCD_U16 laneOffset;
    MCD_16  preCursor;
    MCD_16  attenuation;
    MCD_16  postCursor;
    MCD_U16 swReset;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort = (MCD_U16)inArgs[1];
    host_or_line = (MCD_U16)inArgs[2];
    laneOffset = (MCD_U16)inArgs[3];
    preCursor = (MCD_16)inArgs[4];
    attenuation = (MCD_16)inArgs[5];
    postCursor = (MCD_16)inArgs[6];
    swReset = (MCD_U16)inArgs[7];

    /* call cpss api function */
    status = mcdSetTxFFE(pDev, mdioPort, host_or_line, laneOffset, preCursor, attenuation, postCursor, swReset);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdGetTxFFE function
* @endinternal
*
* @brief   This function reads the Tx FFE with the given input parameters.
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK if query was successful, MCD_FAIL if not
*
* @note The link will be down if the Tx and Rx polarities are mismatched.
*
*/
static CMD_STATUS wrCpssPhyMcdGetTxFFE
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 mdioPort;
    MCD_U16 host_or_line;
    MCD_U16 laneOffset;
    MCD_16  preCursor;
    MCD_16  attenuation;
    MCD_16  postCursor;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort = (MCD_U16)inArgs[1];
    host_or_line = (MCD_U16)inArgs[2];
    laneOffset = (MCD_U16)inArgs[3];

    /* call cpss api function */
    status = mcdGetTxFFE(pDev, mdioPort, host_or_line, laneOffset, &preCursor, &attenuation, &postCursor);

    /* pack output arguments to galtis string */
    galtisOutput(
        outArgs, status, "%d%d%d", preCursor, attenuation, postCursor);

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdSerdesSpicoInterrupt function
* @endinternal
*
* @brief   Issue the interrupt to the Spico processor.
*         The return value is the interrupt number.
*
* @note   APPLICABLE DEVICES:      ???
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static CMD_STATUS wrCpssPhyMcdSerdesSpicoInterrupt
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_UINT  serdesNum;
    MCD_UINT  interruptCode;
    MCD_UINT  interruptData;
    MCD_INT   result = 0;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    serdesNum = (MCD_UINT)inArgs[1];
    interruptCode = (MCD_UINT)inArgs[2];
    interruptData = (MCD_UINT)inArgs[3];

    /* call cpss api function */
    status = mcdSerdesSpicoInterrupt(pDev, serdesNum, interruptCode, interruptData, &result);

    /* pack and output table fields */
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d", result);

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdSerdesManualTxConfig function
* @endinternal
*
* @brief   Per SERDES configure the TX parameters: amplitude, 3 TAP Tx FIR.
*         Can be run after create port.
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK if read was successful, MCD_FAIL otherwise
*/
static CMD_STATUS wrCpssPhyMcdSerdesManualTxConfig
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U8   serdesNum;
    MCD_16   txAmp;
    MCD_16   emph0;
    MCD_16   emph1;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    serdesNum = (MCD_U8)inArgs[1];
    txAmp = (MCD_16)inArgs[2];
    emph0 = (MCD_16)inArgs[4];
    emph1 = (MCD_16)inArgs[5];

    /* call cpss api function */
    status = mcdSerdesManualTxConfig(pDev, serdesNum, txAmp, emph0, emph1);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdSerdesPowerCtrl function
* @endinternal
*
* @brief   Power up SERDES
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK if read was successful, MCD_FAIL otherwise
*/
static CMD_STATUS wrCpssPhyMcdSerdesPowerCtrl
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U32  serdesNum;
    MCD_U8   powerUp;
    MCD_SERDES_SPEED baudRate;
    MCD_BOOL retimerMode;
    MCD_REF_CLK_SEL refClkSel = MCD_PRIMARY_CLK;
    MCD_REF_CLK_DIV refClkDiv = MCD_REF_CLK_NO_DIV;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    serdesNum = (MCD_U32)inArgs[1];
    powerUp = (MCD_U8)inArgs[2];
    baudRate = (MCD_SERDES_SPEED)inArgs[3];
    retimerMode = (MCD_BOOL) inArgs[4];
    /* call cpss api function */
    status = mcdSerdesPowerCtrl(pDev, serdesNum, powerUp, baudRate, retimerMode, (MCD_U32)refClkSel, (MCD_U32)refClkDiv);

    /* pack output arguments to galtis string */

    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdSerdesGetRevision function
* @endinternal
*
* @brief   This call returns the SerDes and SBus master revisions
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK if read was successful, MCD_FAIL otherwise
*
* @note Calling this function requires blocking all other caller from issuing
*       interrupt read/write calls to the SerDes and SBus master.
*
*/
static CMD_STATUS wrCpssPhyMcdSerdesGetRevision
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 serdesRevision = 0;
    MCD_U16 sbmRevision = 0;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }


    /* call cpss api function */
    status = mcdSerdesGetRevision(pDev, &serdesRevision, &sbmRevision);

    /* pack and output table fields */
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d %d", serdesRevision, sbmRevision);

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdSerdesGetVoltage function
* @endinternal
*
* @brief   The call returns the output voltages of the core, DVDD and AVDD.
*         The values return in the outputs are in millivolt(mV).
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK if read was successful, MCD_FAIL otherwise
*
* @note Calling this function requires blocking all other caller from issuing
*       interrupt read/write calls to the SerDes and SBus master.
*
*/
static CMD_STATUS wrCpssPhyMcdSerdesGetVoltage
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 coreVoltage;
    MCD_U16 DVDDVoltage;
    MCD_U16 AVDDVoltage;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    coreVoltage = (MCD_U16)inArgs[1];
    DVDDVoltage = (MCD_U16)inArgs[2];
    AVDDVoltage = (MCD_U16)inArgs[3];

    /* call cpss api function */
    status = mcdSerdesGetVoltage(pDev, &coreVoltage, &DVDDVoltage, &AVDDVoltage);

    /* pack and output table fields */
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d %d %d", coreVoltage, DVDDVoltage, AVDDVoltage);

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdSerdesGetTemperature function
* @endinternal
*
* @brief   The call returns the output voltages of the core, DVDD and AVDD.
*         The values return in the outputs are in millivolt(mV).
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK if read was successful, MCD_FAIL otherwise
*
* @note Calling this function requires blocking all other caller from issuing
*       interrupt read/write calls to the SerDes and SBus master.
*
*/
static CMD_STATUS wrCpssPhyMcdSerdesGetTemperature
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_32 coreTemperature = 0;
    MCD_32 serdesTemperature = 0;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }


    /* call cpss api function */
    status = mcdSerdesGetTemperature(pDev, &coreTemperature, &serdesTemperature);
    cpssOsPrintf("\n coreTemperature = %d \n serdesTemperature = %d \n",coreTemperature, serdesTemperature);

    /* pack and output table fields */
    /* pack output to galtis string */
    galtisOutput(outArgs, status,"");

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdSerdesGetTxFFE function
* @endinternal
*
* @brief   This function call returns the TX equalization values of an individual
*         SerDes lane.
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK if read was successful, MCD_FAIL otherwise
*
* @note Calling this function requires blocking all other caller from issuing
*       interrupt read/write calls to the SerDes and SBus master.
*
*/
static CMD_STATUS wrCpssPhyMcdSerdesGetTxFFE
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 sAddr;
    MCD_16 preCursor = 0;
    MCD_16 attenuation = 0;
    MCD_16 postCursor = 0;
    MCD_16 slew = 0;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    sAddr = (MCD_U16)inArgs[1];

    /* call cpss api function */
    status = mcdSerdesGetTxFFE(pDev, sAddr, &preCursor, &attenuation, &postCursor, &slew);


    /* pack and output table fields */
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d %d %d %d", preCursor, attenuation, postCursor, slew);

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdSerdesSetTxFFE function
* @endinternal
*
* @brief   This function sets the TX equalization preCursor, attenuation,
*         postCursor and slew values on an individual SerDes lane.
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK if read was successful, MCD_FAIL otherwise
*
* @note Calling this function requires blocking all other caller from issuing
*       interrupt read/write calls to the SerDes and SBus master.
*
*/
static CMD_STATUS wrCpssPhyMcdSerdesSetTxFFE
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 sAddr;
    MCD_16 preCursor;
    MCD_16 attenuation;
    MCD_16 postCursor;
    MCD_16 slew;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    sAddr = (MCD_U16)inArgs[1];
    preCursor = (MCD_16)inArgs[2];
    attenuation = (MCD_16)inArgs[3];
    postCursor = (MCD_16)inArgs[4];
    slew = (MCD_16)inArgs[5];

    /* call cpss api function */
    status = mcdSerdesSetTxFFE(pDev, sAddr, preCursor, attenuation, postCursor, slew);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdSerdesGetTxPolarity function
* @endinternal
*
* @brief   This function returns the TX inverter polarity for an individual SerDes lane.
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK if read was successful, MCD_FAIL otherwise
*
* @note Calling this function requires blocking all other caller from issuing
*       interrupt read/write calls to the SerDes and SBus master.
*
*/
static CMD_STATUS wrCpssPhyMcdSerdesGetTxPolarity
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 sAddr;
    MCD_U16 polarity = 0;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    sAddr = (MCD_U16)inArgs[1];

    /* call cpss api function */
    status = mcdSerdesGetTxPolarity(pDev, sAddr, &polarity);

    /* pack and output table fields */
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d", polarity);

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdSerdesSetTxPolarity function
* @endinternal
*
* @brief   This function returns the TX inverter polarity for an individual SerDes lane.
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK if read was successful, MCD_FAIL otherwise
*
* @note Calling this function requires blocking all other caller from issuing
*       interrupt read/write calls to the SerDes and SBus master.
*
*/
static CMD_STATUS wrCpssPhyMcdSerdesSetTxPolarity
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 sAddr;
    MCD_U16 polarity;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    sAddr = (MCD_U16)inArgs[1];
    polarity = (MCD_U16)inArgs[2];

    /* call cpss api function */
    status = mcdSerdesSetTxPolarity(pDev, sAddr, polarity);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdSerdesGetRxPolarity function
* @endinternal
*
* @brief   This function returns the RX inverter polarity for an individual SerDes lane.
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK if read was successful, MCD_FAIL otherwise
*
* @note Calling this function requires blocking all other caller from issuing
*       interrupt read/write calls to the SerDes and SBus master.
*
*/
static CMD_STATUS wrCpssPhyMcdSerdesGetRxPolarity
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 sAddr;
    MCD_U16 polarity = 0;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    sAddr = (MCD_U16)inArgs[1];

    /* call cpss api function */
    status = mcdSerdesGetRxPolarity(pDev, sAddr, &polarity);

    /* pack and output table fields */
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d", polarity);

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdSerdesSetRxPolarity function
* @endinternal
*
* @brief   This function returns the RX inverter polarity for an individual SerDes lane.
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK if read was successful, MCD_FAIL otherwise
*
* @note Calling this function requires blocking all other caller from issuing
*       interrupt read/write calls to the SerDes and SBus master.
*
*/
static CMD_STATUS wrCpssPhyMcdSerdesSetRxPolarity
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 sAddr;
    MCD_U16 polarity;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    sAddr = (MCD_U16)inArgs[1];
    polarity = (MCD_U16)inArgs[2];

    /* call cpss api function */
    status = mcdSerdesSetRxPolarity(pDev, sAddr, polarity);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdSerdesGetCDRLock function
* @endinternal
*
* @brief   This function retrieves the Clock Data Recovery(CDR) lock state for an
*         individual SerDes lane.
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK if read was successful, MCD_FAIL otherwise
*
* @note Calling this function requires blocking all other caller from issuing
*       interrupt read/write calls to the SerDes and SBus master.
*
*/
static CMD_STATUS wrCpssPhyMcdSerdesGetCDRLock
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 sAddr;
    MCD_U16 CDRLockState = 0;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    sAddr = (MCD_U16)inArgs[1];

    /* call cpss api function */
    status = mcdSerdesGetCDRLock(pDev, sAddr, &CDRLockState);

    /* pack and output table fields */
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d", CDRLockState);

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdSerdesTxInjectError function
* @endinternal
*
* @brief   This function injects errors into the TX data on an individual SerDes lane.
*         TX error injection affects the transmitted data stream. Therefore, errors
*         are only detected when comparing with a known pattern.
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK if read was successful, MCD_FAIL otherwise
*
* @note Calling this function requires blocking all other caller from issuing
*       interrupt read/write calls to the SerDes and SBus master.
*
*/
static CMD_STATUS wrCpssPhyMcdSerdesTxInjectError
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 sAddr;
    MCD_U16 numErrBit;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    sAddr = (MCD_U16)inArgs[1];
    numErrBit = (MCD_U16)inArgs[2];

    /* call cpss api function */
    status = mcdSerdesTxInjectError(pDev, sAddr, numErrBit);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdSerdesRxInjectError function
* @endinternal
*
* @brief   This function injects errors into the RX data on an individual SerDes lane.
*         RX error injection affects only the main data channel. Therefore, errors
*         are only detected if the MAIN channel is one of the selected compare inputs.
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK if read was successful, MCD_FAIL otherwise
*
* @note Calling this function requires blocking all other caller from issuing
*       interrupt read/write calls to the SerDes and SBus master.
*
*/
static CMD_STATUS wrCpssPhyMcdSerdesRxInjectError
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 sAddr;
    MCD_U16 numErrBit;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    sAddr = (MCD_U16)inArgs[1];
    numErrBit = (MCD_U16)inArgs[2];

    /* call cpss api function */
    status = mcdSerdesRxInjectError(pDev, sAddr, numErrBit);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdSerdesSetTxUserData function
* @endinternal
*
* @brief   This function loads the 80-bit value into the TX user data register and
*         select it as the TX input. The bit transmit order is from the least
*         significant bit [0] of user_data[0] through bit [19] of user_data[3].
*         Each word holds 20 significant bits.
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK if read was successful, MCD_FAIL otherwise
*
* @note Calling this function requires blocking all other caller from issuing
*       interrupt read/write calls to the SerDes and SBus master.
*
*/
static CMD_STATUS wrCpssPhyMcdSerdesSetTxUserData
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 sAddr;
    MCD_32 userData[4];

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    sAddr = (MCD_U16)inArgs[1];
    userData[0] = (MCD_32)inArgs[2];
    userData[1] = (MCD_32)inArgs[3];
    userData[2] = (MCD_32)inArgs[4];
    userData[3] = (MCD_32)inArgs[5];

    /* call cpss api function */
    status = mcdSerdesSetTxUserData(pDev, sAddr, userData);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdSerdesGetTxUserData function
* @endinternal
*
* @brief   This function retrieves the 80-bit TX user data register.
*         Only the first 20 bits of the pattern are returned as that is all
*         that is readily available from the SerDes.
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK if read was successful, MCD_FAIL otherwise
*
* @note Calling this function requires blocking all other caller from issuing
*       interrupt read/write calls to the SerDes and SBus master.
*       Only the first 20 bits of the pattern are returned as that is all
*       that is readily available from the SerDes. See description for details.
*
*/
static CMD_STATUS wrCpssPhyMcdSerdesGetTxUserData
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 sAddr;
    MCD_32 userData[4] = {0};

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    sAddr = (MCD_U16)inArgs[1];

    /* call cpss api function */
    status = mcdSerdesGetTxUserData(pDev, sAddr, userData);


    /* pack and output table fields */
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d %d %d %d", userData[0], userData[1], userData[2], userData[3]);

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdSerdesGetErrCount function
* @endinternal
*
* @brief   This function retrieves the error count for the earlier operations.
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK if read was successful, MCD_FAIL otherwise
*
* @note Calling this function requires blocking all other caller from issuing
*       interrupt read/write calls to the SerDes and SBus master.
*       Only the first 20 bits of the pattern are returned as that is all
*       that is readily available from the SerDes. See description for details.
*
*/
static CMD_STATUS wrCpssPhyMcdSerdesGetErrCount
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 sAddr;
    MCD_BOOL resetCounter;
    MCD_U32 errCount = 0;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    sAddr = (MCD_U16)inArgs[1];
    resetCounter = (MCD_BOOL)inArgs[2];
    /* call cpss api function */
    status = mcdSerdesGetErrCount(pDev, sAddr, resetCounter, &errCount);

    /* pack and output table fields */
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d", errCount);

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdSerdesSetLoopback function
* @endinternal
*
* @brief   This function sets an individual serdes lane to input loopback mode.
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK if read was successful, MCD_FAIL otherwise
*
* @note Calling this function requires blocking all other caller from issuing
*       interrupt read/write calls to the SerDes and SBus master.
*
*/
static CMD_STATUS wrCpssPhyMcdSerdesSetLoopback
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 sAddr;
    MCD_U16 loopbackMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    sAddr = (MCD_U16)inArgs[1];
    loopbackMode = (MCD_U16)inArgs[2];
    /* call cpss api function */
    status = mcdSerdesSetLoopback(pDev, sAddr, loopbackMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdSerdesReset function
* @endinternal
*
* @brief   This function resets a SerDes lane on a given SerDes lane ID.
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK if read was successful, MCD_FAIL otherwise
*
* @note Calling this function requires blocking all other caller from issuing
*       interrupt read/write calls to the SerDes and SBus master.
*
*/
static CMD_STATUS wrCpssPhyMcdSerdesReset
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 sAddr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    sAddr = (MCD_U16)inArgs[1];
    /* call cpss api function */
    status = mcdSerdesReset(pDev, sAddr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdSerdesAccessLock function
* @endinternal
*
* @brief   Serdes access lock
*
* @note   APPLICABLE DEVICES:      ???
*                                       None
*/
static CMD_STATUS wrCpssPhyMcdSerdesAccessLock
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_U32   phyId;
    MCD_DEV_PTR pDev;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    /* call cpss api function */
    mcdSerdesAccessLock(pDev);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, MCD_OK, "");

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdSerdesAccessUnlock function
* @endinternal
*
* @brief   Serdes access unlock
*
* @note   APPLICABLE DEVICES:      ???
*                                       None
*/
static CMD_STATUS wrCpssPhyMcdSerdesAccessUnlock
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_U32   phyId;
    MCD_DEV_PTR pDev;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    /* call cpss api function */
    mcdSerdesAccessUnlock(pDev);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, MCD_OK, "");

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdSerdesResetImpl function
* @endinternal
*
* @brief   Per SERDES Clear the serdes registers (back to defaults.
*
* @note   APPLICABLE DEVICES:      ???
*                                       None
*/
static CMD_STATUS wrCpssPhyMcdSerdesResetImpl
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_UINT serdesNum;
    MCD_UINT analogReset;
    MCD_UINT digitalReset;
    MCD_UINT syncEReset;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    serdesNum = (MCD_UINT)inArgs[1];
    analogReset = (MCD_UINT)inArgs[2];
    digitalReset = (MCD_UINT)inArgs[3];
    syncEReset = (MCD_UINT)inArgs[4];

    /* call cpss api function */
    status = mcdSerdesResetImpl(pDev, serdesNum, analogReset, digitalReset, syncEReset);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdSetHostSidePRBSPattern function
* @endinternal
*
* @brief   This function is used to select the type of PRBS pattern desired in
*         for the host side PRBS in registers 4.AX30.3:0 (X depends on Port/Lane).
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK or MCD_FAIL if io error or bad parameter passed in (out of range)
*/
static CMD_STATUS wrCpssPhyMcdSetHostSidePRBSPattern
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16  mdioPort;
    MCD_U16  laneOffset;
    MCD_PRBS_HOST_SELECTOR_TYPE pattSel;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort = (MCD_U16)inArgs[1];
    laneOffset = (MCD_U16)inArgs[2];
    pattSel = (MCD_PRBS_HOST_SELECTOR_TYPE)inArgs[3];

    /* call cpss api function */
    status = mcdSetHostSidePRBSPattern(pDev, mdioPort, laneOffset, pattSel);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdGetPRBSWaitForLock function
* @endinternal
*
* @brief   Returns configuration for PRBS whether it is set to wait for locking
*         or not before counting.
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK or MCD_FAIL if io error or bad parameter passed in (out of range)
*/
static CMD_STATUS wrCpssPhyMcdGetPRBSWaitForLock
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16  mdioPort;
    MCD_U16  host_or_line;
    MCD_U16  laneOffset;
    MCD_U16  disableWaitforLock = 0;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort = (MCD_U16)inArgs[1];
    host_or_line = (MCD_U16)inArgs[2];
    laneOffset = (MCD_U16)inArgs[3];

    /* call cpss api function */
    status = mcdGetPRBSWaitForLock(pDev, mdioPort, host_or_line, laneOffset, &disableWaitforLock);

    /* pack and output table fields */
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d", disableWaitforLock);

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdGetRsFecBipErrorCntr function
* @endinternal
*
* @brief   Get RS-FEC BIP error counter per PCS lane
*         Register name: RS-FEC BIP error counter lane #
*         Device 1, Register 0x00E6 - 0x00F9
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK or MCD_FAIL if io error or bad parameter passed in (out of range)
*/
static CMD_STATUS wrCpssPhyMcdGetRsFecBipErrorCntr
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 mdioPort;
    MCD_U16 pcs_lane;
    MCD_U16 errorCounter = 0;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort = (MCD_U16)inArgs[1];
    pcs_lane = (MCD_U16)inArgs[2];

    /* call cpss api function */
    status = mcdGetRsFecBipErrorCntr(pDev, mdioPort, pcs_lane, &errorCounter);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdSerdesPMDTrainingLog function
* @endinternal
*
* @brief   This function retrieves the link training log for an individual SerDes lane.
*         The output results are stored in the pTrainInfo. The caller must pass in
*         a pre-allocated memory MCD_TRAINING_INFO structure for the pTrainInfo
*         to store the training log results.
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK or MCD_FAIL if io error or bad parameter passed in (out of range)
*
* @note Calling this function requires blocking all other caller from issuing
*       interrupt read/write calls to the SerDes and SBus master.
*
*/
static CMD_STATUS wrCpssPhyMcdSerdesPMDTrainingLog
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 sAddr;
    MCD_TRAINING_INFO pTrainInfo;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    sAddr = (MCD_U16)inArgs[1];

    cpssOsMemSet(&pTrainInfo, 0, sizeof(pTrainInfo));

    /* call cpss api function */
    status = mcdSerdesPMDTrainingLog(pDev, sAddr, &pTrainInfo);

    /* pack and output table fields */
    galtisOutput(outArgs, status,"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %dd %d %d %d %d %d %d",
        pTrainInfo.reset,
        pTrainInfo.rx_metric,
        pTrainInfo.local.preset,
        pTrainInfo.local.initialize,
        pTrainInfo.local.tap[0].inc,
        pTrainInfo.local.tap[0].dec,
        pTrainInfo.local.tap[0].max,
        pTrainInfo.local.tap[0].min,
        pTrainInfo.local.tap[1].inc,
        pTrainInfo.local.tap[1].dec,
        pTrainInfo.local.tap[1].max,
        pTrainInfo.local.tap[1].min,
        pTrainInfo.local.tap[2].inc,
        pTrainInfo.local.tap[2].dec,
        pTrainInfo.local.tap[2].max,
        pTrainInfo.local.tap[2].min,
        pTrainInfo.remote.preset,
        pTrainInfo.remote.initialize,
        pTrainInfo.remote.tap[0].inc,
        pTrainInfo.remote.tap[0].dec,
        pTrainInfo.remote.tap[0].max,
        pTrainInfo.remote.tap[0].min,
        pTrainInfo.remote.tap[1].inc,
        pTrainInfo.remote.tap[1].dec,
        pTrainInfo.remote.tap[1].max,
        pTrainInfo.remote.tap[1].min,
        pTrainInfo.remote.tap[2].inc,
        pTrainInfo.remote.tap[2].dec,
        pTrainInfo.remote.tap[2].max,
        pTrainInfo.remote.tap[2].min,
        pTrainInfo.last_remote_request[0],
        pTrainInfo.last_remote_request[1],
        pTrainInfo.last_remote_request[2],
        pTrainInfo.last_remote_request[3],
        pTrainInfo.last_remote_request[4],
        pTrainInfo.last_remote_request[5],
        pTrainInfo.last_remote_request[6],
        pTrainInfo.last_remote_request[7],
        pTrainInfo.last_local_request
    );

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdSerdesSetTxPRBS function
* @endinternal
*
* @brief   This function sets the Tx PRBS data source on an individual SerDes lane.
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK or MCD_FAIL if io error or bad parameter passed in (out of range)
*
* @note Calling this function requires blocking all other caller from issuing
*       interrupt read/write calls to the SerDes and SBus master.
*
*/
static CMD_STATUS wrCpssPhyMcdSerdesSetTxPRBS
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 sAddr;
    MCD_SERDES_TX_DATA_TYPE txDataType;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    sAddr = (MCD_U16)inArgs[1];
    txDataType = (MCD_SERDES_TX_DATA_TYPE)inArgs[2];

    /* call cpss api function */
    status = mcdSerdesSetTxPRBS(pDev, sAddr, txDataType);
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdSerdesGetTxPRBS function
* @endinternal
*
* @brief   This function gets the Tx PRBS data source on an individual SerDes lane.
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK or MCD_FAIL if io error or bad parameter passed in (out of range)
*
* @note Calling this function requires blocking all other caller from issuing
*       interrupt read/write calls to the SerDes and SBus master.
*
*/
static CMD_STATUS wrCpssPhyMcdSerdesGetTxPRBS
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 sAddr;
    MCD_SERDES_TX_DATA_TYPE txDataType = MCD_SERDES_TX_DATA_SEL_PRBS7;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    sAddr = (MCD_U16)inArgs[1];

    /* call cpss api function */
    status = mcdSerdesGetTxPRBS(pDev, sAddr, &txDataType);

    /* pack and output table fields */
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d", txDataType);

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdSerdesSetRxCmpData function
* @endinternal
*
* @brief   This function sets the RX compare data configuration. It sets the PRBS pattern
*         to compare incoming data against. If MCD_SERDES_RX_CMP_DATA_SELF_SEED is
*         selected, the comparator automatically synchronizes to an incoming 80 bit
*         pattern. The error counter then reports deviations from that pattern.
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK or MCD_FAIL if io error or bad parameter passed in (out of range)
*
* @note Calling this function requires blocking all other caller from issuing
*       interrupt read/write calls to the SerDes and SBus master.
*
*/
static CMD_STATUS wrCpssPhyMcdSerdesSetRxCmpData
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 sAddr;
    MCD_SERDES_RX_CMP_TYPE RxCmpType;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    sAddr = (MCD_U16)inArgs[1];
    RxCmpType = (MCD_SERDES_RX_CMP_TYPE)inArgs[2];

    /* call cpss api function */
    status = mcdSerdesSetRxCmpData(pDev, sAddr, RxCmpType);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdSerdesGetRxCmpData function
* @endinternal
*
* @brief   This function retrieves the Rx compare data configuration that is set by
*         the mcdSerdesSetRxCmpData function.
*
* @note   APPLICABLE DEVICES:      ???
*                                       MCD_OK or MCD_FAIL if io error or bad parameter passed in (out of range)
*
* @note Calling this function requires blocking all other caller from issuing
*       interrupt read/write calls to the SerDes and SBus master.
*
*/
static CMD_STATUS wrCpssPhyMcdSerdesGetRxCmpData
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 sAddr;
    MCD_SERDES_RX_CMP_TYPE RxCmpType = MCD_SERDES_RX_CMP_DATA_PRBS7;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    sAddr = (MCD_U16)inArgs[1];

    /* call cpss api function */
    status = mcdSerdesGetRxCmpData(pDev, sAddr, &RxCmpType);

    /* pack and output table fields */
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d", RxCmpType);

    return CMD_OK;
}

/******************************************************************************
MCD_STATUS mcdSetInterruptEnableEx
(
    IN  MCD_DEV_PTR pDev,
    IN  MCD_U16 mdioPort,
    IN  MCD_U16 host_or_line,
    IN  MCD_U16 interruptRegisterNum,
    IN  MCD_U16 intrEnableFlags
);

 Inputs:
    pDev - pointer to MCD_DEV initialized by mcdLoadDriver() call
    mdioPort - MDIO port address, 0-7
    host_or_line - which interface is being read:
        MCD_HOST_SIDE
        MCD_LINE_SIDE
    interruptRegisterNum - number of interrupt related register 0..14
        described in Functional Specification.
    intrEnableFlags - For each flag a 1 enables the interrupt and a
        0 disables the interrupt. Flags described in Functional Specification.

 Outputs:
    None

 Returns:
    MCD_OK if change was successful, MCD_FAIL if not.

 Description:
        This function can be called after the port has been initialized.

 Side effects:
    None.

 Notes/Warnings:
    The bit flags can be OR together to enable multiple interrupts.

******************************************************************************/
static CMD_STATUS wrCpssPhyMcdSetInterruptEnableEx
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 mdioPort;
    MCD_U16 host_or_line;
    MCD_U16 interruptRegisterNum; /*0..14*/
    MCD_U16 intrEnableFlags;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId                = (MCD_U32)inArgs[0];
    pDev                 = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort             = (MCD_U16)inArgs[1];
    host_or_line         = (MCD_U16)inArgs[2];
    interruptRegisterNum = (MCD_U16)inArgs[3];
    intrEnableFlags      = (MCD_U16)inArgs[4];

    /* call cpss api function */
    status = mcdSetInterruptEnableEx(
        pDev, mdioPort, host_or_line, interruptRegisterNum, intrEnableFlags);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/******************************************************************************
MCD_STATUS mcdGetInterruptEnableEx
(
    IN  MCD_DEV_PTR pDev,
    IN  MCD_U16 mdioPort,
    IN  MCD_U16 host_or_line,
    IN  MCD_U16 interruptRegisterNum,
    OUT MCD_U16 *intrEnableFlags
);

 Inputs:
    pDev - pointer to MCD_DEV initialized by mcdLoadDriver() call
    mdioPort - MDIO port address, 0-7
    host_or_line - which interface is being read:
        MCD_HOST_SIDE
        MCD_LINE_SIDE
    interruptRegisterNum - number of interrupt related register 0..14
        described in Functional Specification.

 Outputs:
        intrEnableFlags - For each flag a 1 enables the interrupt and a
            0 disables the interrupt. Flags described in Functional Specification.

 Returns:
    MCD_OK if change was successful, MCD_FAIL if not.

 Description:
        This function can be called after the port has been initialized.

 Side effects:
    None.

 Notes/Warnings:
    None

******************************************************************************/
static CMD_STATUS wrCpssPhyMcdGetInterruptEnableEx
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 mdioPort;
    MCD_U16 host_or_line;
    MCD_U16 interruptRegisterNum; /*0..14*/
    MCD_U16 intrEnableFlags;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId                = (MCD_U32)inArgs[0];
    pDev                 = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort             = (MCD_U16)inArgs[1];
    host_or_line         = (MCD_U16)inArgs[2];
    interruptRegisterNum = (MCD_U16)inArgs[3];

    /* call cpss api function */
    status = mcdGetInterruptEnableEx(
        pDev, mdioPort, host_or_line, interruptRegisterNum, &intrEnableFlags);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d", intrEnableFlags);

    return CMD_OK;
}

/******************************************************************************
MCD_STATUS mcdGetInterruptStatusEx
(
    IN  MCD_DEV_PTR pDev,
    IN  MCD_U16 mdioPort,
    IN  MCD_U16 host_or_line,
    IN  MCD_BOOL toClearStatus,
    IN  MCD_U16 interruptRegisterNum,
    OUT MCD_U16 *interruptStatusFlags
);

 Inputs:
    pDev - pointer to MCD_DEV initialized by mcdLoadDriver() call
    mdioPort - MDIO port address, 0-7
    host_or_line - which interface is being read:
        MCD_HOST_SIDE
        MCD_LINE_SIDE
    toClearStatus - MCD_TRUE - clear interrupt status register after reading,
                    MCD_FALSE - leave interrupt status register as was before reading.
    interruptRegisterNum - number of interrupt related register 0..14
        described in Functional Specification.

 Outputs:
        interruptStatusFlags - For each flag a 1 means interrupt occurred and a
            0 means the interrupt not occurred. Flags described in Functional Specification.

 Returns:
    MCD_OK if change was successful, MCD_FAIL if not.

 Description:
        This function can be called after the port has been initialized.

 Side effects:
    None.

 Notes/Warnings:
    None

******************************************************************************/
static CMD_STATUS wrCpssPhyMcdGetInterruptStatusEx
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 mdioPort;
    MCD_U16 host_or_line;
    MCD_BOOL toClearStatus;
    MCD_U16 interruptRegisterNum; /*0..14*/
    MCD_U16 interruptStatusFlags;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId                = (MCD_U32)inArgs[0];
    pDev                 = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort             = (MCD_U16)inArgs[1];
    host_or_line         = (MCD_U16)inArgs[2];
    toClearStatus        = (MCD_BOOL)inArgs[3];
    interruptRegisterNum = (MCD_U16)inArgs[4];

    /* call cpss api function */
    status = mcdGetInterruptStatusEx(
        pDev, mdioPort, host_or_line, toClearStatus,
        interruptRegisterNum, &interruptStatusFlags);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d", interruptStatusFlags);

    return CMD_OK;
}

/******************************************************************************
MCD_STATUS mcdGetRealtimeStatusEx
(
    IN  MCD_DEV_PTR pDev,
    IN  MCD_U16 mdioPort,
    IN  MCD_U16 host_or_line,
    IN  MCD_U16 interruptRegisterNum,
    OUT MCD_U16 *intrRtStatusFlags
);

 Inputs:
    pDev - pointer to MCD_DEV initialized by mcdLoadDriver() call
    mdioPort - MDIO port address, 0-7
    host_or_line - which interface is being read:
        MCD_HOST_SIDE
        MCD_LINE_SIDE
    interruptRegisterNum - number of interrupt related register 0..14
        described in Functional Specification.

 Outputs:
        intrRtStatusFlags - For each flag a 1 means interrupt occurred and a
            0 means the interrupt not occurred. Flags described in Functional Specification.

 Returns:
    MCD_OK if change was successful, MCD_FAIL if not.

 Description:
        This function can be called after the port has been initialized.

 Side effects:
    None.

 Notes/Warnings:
    None

******************************************************************************/
static CMD_STATUS wrCpssPhyMcdGetRealtimeStatusEx
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 mdioPort;
    MCD_U16 host_or_line;
    MCD_U16 interruptRegisterNum; /*0..14*/
    MCD_U16 intrRtStatusFlags;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId                = (MCD_U32)inArgs[0];
    pDev                 = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort             = (MCD_U16)inArgs[1];
    host_or_line         = (MCD_U16)inArgs[2];
    interruptRegisterNum = (MCD_U16)inArgs[3];

    /* call cpss api function */
    status = mcdGetRealtimeStatusEx(
        pDev, mdioPort, host_or_line, MCD_TRUE, interruptRegisterNum, 0, &intrRtStatusFlags);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d", intrRtStatusFlags);

    return CMD_OK;
}

#if 0
/**
* @internal wrCpssPhyMcdSerdesTestGenGet function
* @endinternal
*
* @brief   Get configuration of the Serdes test generator/checker.
*         Can be run after create port.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_SUPPORTED         - unexpected pattern
* @retval GT_FAIL                  - HW error
*/
static CMD_STATUS wrCpssPhyMcdSerdesTestGenGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32     phyId;
    MCD_DEV_PTR pDev;
    MCD_U16     serdesNum;
    MCD_SERDES_TX_PATTERN    txPattern = MCD_1T;
    MCD_SERDES_TEST_GEN_MODE mode = MCD_SERDES_NORMAL;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }
    serdesNum = (MCD_U16)inArgs[1];

    /* call cpss api function */
    status = mcdSerdesTestGenGet(pDev, serdesNum, &txPattern, &mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d %d", txPattern, mode);

    return CMD_OK;
}

#endif
/**
* @internal wrCpssPhyMcdSerdesAccessValidate function
* @endinternal
*
* @brief   Validate access to Avago device
*/
static CMD_STATUS wrCpssPhyMcdSerdesAccessValidate
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32     phyId;
    MCD_DEV_PTR pDev;
    MCD_SER_DEV_PTR pSerdesDev;
    MCD_U32     sbus_addr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }
    pSerdesDev = (MCD_SER_DEV_PTR)pDev->serdesDev;
    sbus_addr = (MCD_BOOL)inArgs[1];

    /* call cpss api function */
    status = mcdSerdesAccessValidate(pSerdesDev, sbus_addr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}
static GT_U32 SerdesAutoTuneResultsSerdesNum;
/**
* @internal wrCpssPhyMcdSerdesAutoTuneResultsGetFirst function
* @endinternal
*
* @brief   Per SERDES return the adapted tuning results
*         Can be run after create port.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static CMD_STATUS wrCpssPhyMcdSerdesAutoTuneResultsGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_U32     phyId,i;
    MCD_DEV_PTR pDev;
    MCD_U8      startSerdesNum,endSerdesNum;
    MCD_AUTO_TUNE_RESULTS results;

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }
    SerdesAutoTuneResultsSerdesNum = (MCD_U32)inArgs[1];
    startSerdesNum = (MCD_U8)inArgs[1];
    endSerdesNum = (MCD_U8)inArgs[2];
    if (endSerdesNum <  startSerdesNum)
    {
        endSerdesNum = startSerdesNum;
    }

    mcdSerdesAutoTuneResult(pDev,(MCD_U8)SerdesAutoTuneResultsSerdesNum,&results);
    for (i = 0; i < 13; i++)
    {
        inFields[9 + i] = (results.DFE[i] < 0) ? (0x80000000 - results.DFE[i]) : results.DFE[i]; /* for negative value */
    }

    inFields[0] = (MCD_32)results.txAmp;
    inFields[1] = (MCD_32)results.txEmph0;
    inFields[2] = (MCD_32)results.txEmph1;
    inFields[3] = (MCD_32)results.DC;
    inFields[4] = (MCD_32)results.LF;
    inFields[5] = (MCD_32)results.HF;
    inFields[6] = (MCD_32)results.BW;
    inFields[7] = (MCD_32)results.LB;
    inFields[8] = (MCD_32)results.EO;

    for (i = 0; i < 13; i++)
    {
        cpssOsPrintf("\nDFE%d is %d", i, results.DFE[i]);
    }
    cpssOsPrintf("\nHF is %d", results.HF);
    cpssOsPrintf("\nLF is %d", results.LF);
    cpssOsPrintf("\nDC is %d", results.DC);
    cpssOsPrintf("\nBW is %d", results.BW);
    cpssOsPrintf("\nLB is %d", results.LB);
    cpssOsPrintf("\nEO is %d", results.EO);
    cpssOsPrintf("\nTX Amp is %d", results.txAmp);
    cpssOsPrintf("\nTX Emph0 is %d", results.txEmph0);
    cpssOsPrintf("\nTX Emph1 is %d\n", results.txEmph1);

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",inFields[0],inFields[1], inFields[2], inFields[3], inFields[4], inFields[5], inFields[6], inFields[7], inFields[8], inFields[9], inFields[10], inFields[11], inFields[12], inFields[13], inFields[14], inFields[15], inFields[16], inFields[17], inFields[18], inFields[19], inFields[20],inFields[21]);

    galtisOutput(outArgs, GT_OK, "%f");
    return CMD_OK;
}

static CMD_STATUS wrCpssPhyMcdSerdesAutoTuneResultsGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS   rc;
    MCD_U32     phyId,i;
    MCD_DEV_PTR pDev;
    MCD_U8      endSerdesNum;
    MCD_U8      serdesNum;
    MCD_AUTO_TUNE_RESULTS results;

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    endSerdesNum = (MCD_U8)inArgs[2];
    serdesNum = (MCD_U8)SerdesAutoTuneResultsSerdesNum;
    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    /* get next index */
    serdesNum++;
    if (serdesNum > endSerdesNum)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    inFields[0] = serdesNum;
    SerdesAutoTuneResultsSerdesNum = serdesNum;

    rc = mcdSerdesAutoTuneResult(pDev,(MCD_U8)SerdesAutoTuneResultsSerdesNum,&results);
    if (rc != MCD_OK)
    {
        cpssOsPrintf("\nexecute mvHwsSerdesAutoTuneResult fail\n");
        galtisOutput(outArgs, rc, "");
        return CMD_OK;
    }

    for (i = 0; i < 13; i++)
    {
        inFields[10 + i] = (results.DFE[i] < 0) ? (0x80000000 - results.DFE[i]) : results.DFE[i]; /* for negative value */
    }
    inFields[0] = (MCD_32)results.txAmp;
    inFields[1] = (MCD_32)results.txEmph0;
    inFields[2] = (MCD_32)results.txEmph1;
    inFields[3] = (MCD_32)results.DC;
    inFields[4] = (MCD_32)results.LF;
    inFields[5] = (MCD_32)results.HF;
    inFields[6] = (MCD_32)results.BW;
    inFields[7] = (MCD_32)results.LB;
    inFields[8] = (MCD_32)results.EO;


    for (i = 0; i < 13; i++)
    {
        cpssOsPrintf("\nDFE%d is %d", i, results.DFE[i]);
    }
    cpssOsPrintf("\nHF is %d", results.HF);
    cpssOsPrintf("\nLF is %d", results.LF);
    cpssOsPrintf("\nDC is %d", results.DC);
    cpssOsPrintf("\nBW is %d", results.BW);
    cpssOsPrintf("\nLB is %d", results.LB);
    cpssOsPrintf("\nEO is %d", results.EO);
    cpssOsPrintf("\nTX Amp is %d", results.txAmp);
    cpssOsPrintf("\nTX Emph0 is %d", results.txEmph0);
    cpssOsPrintf("\nTX Emph1 is %d\n", results.txEmph1);

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d", inFields[0], inFields[1], inFields[2], inFields[3], inFields[4], inFields[5], inFields[6], inFields[7], inFields[8], inFields[9], inFields[10], inFields[11], inFields[12], inFields[13], inFields[14], inFields[15], inFields[16], inFields[17], inFields[18], inFields[19], inFields[20], inFields[21]);
    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdSerdesAutoTuneStatusShort function
* @endinternal
*
* @brief   Check the Serdes Rx or Tx training status
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static CMD_STATUS wrCpssPhyMcdSerdesAutoTuneStatusShort
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32     phyId;
    MCD_DEV_PTR pDev;
    MCD_U8      serdesNum;
    MCD_AUTO_TUNE_STATUS rxStatus = MCD_TUNE_PASS;
    MCD_AUTO_TUNE_STATUS txStatus = MCD_TUNE_PASS;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }
    serdesNum = (MCD_U8)inArgs[1];

    /* call cpss api function */
    status = mcdSerdesAutoTuneStatusShort(pDev, serdesNum, &rxStatus, &txStatus);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d %d", rxStatus, txStatus);

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdSerdesAutoTuneStartExt function
* @endinternal
*
* @brief   Per SERDES control the TX training & Rx Training starting
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static CMD_STATUS wrCpssPhyMcdSerdesAutoTuneStartExt
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32     phyId;
    MCD_DEV_PTR pDev;
    MCD_U8      serdesNum;
    MCD_AUTO_TUNE_STATUS rxStatus;
    MCD_AUTO_TUNE_STATUS txStatus;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }
    serdesNum = (MCD_U8)inArgs[1];
    rxStatus = (MCD_AUTO_TUNE_STATUS)inArgs[2];
    txStatus = (MCD_AUTO_TUNE_STATUS )inArgs[3];

    /* call cpss api function */
    status = mcdSerdesAutoTuneStartExt(pDev, serdesNum, rxStatus, txStatus);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdSerdesAutoTuneStart function
* @endinternal
*
* @brief   Per SERDES control the TX training & Rx Training starting.
*         This function calls to the mcdSerdesAutoTuneStartExt, which includes
*         all the functional options.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static CMD_STATUS wrCpssPhyMcdSerdesAutoTuneStart
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32     phyId;
    MCD_DEV_PTR pDev;
    MCD_U8      serdesNum;
    MCD_BOOL    rxTraining;
    MCD_BOOL    txTraining;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }
    serdesNum = (MCD_U8)inArgs[1];
    rxTraining = (MCD_BOOL)inArgs[2];
    txTraining = (MCD_BOOL)inArgs[3];

    /* call cpss api function */
    status = mcdSerdesAutoTuneStart(pDev, serdesNum, rxTraining, txTraining);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/*******************************************************************
MCD_STATUS mcdPortReset
(
    IN MCD_DEV_PTR pDev,
    IN MCD_U16 mdioPort,
    IN MCD_U16 host_or_line,
    IN MCD_RESET_TYPE resetType,
    IN MCD_U16 timeoutMs
);

 Inputs:
   pDev  - pointer to holds device information to be used for each API call.
   mdioPort - MDIO port address, 0-31
   host_or_line - which interface is being modified:
        MCD_HOST_SIDE
        MCD_LINE_SIDE
        MCD_BOTH_SIDE
   resetType - option for Hard or Soft port reset
        MCD_SOFT_RESET
        MCD_HARD_RESET
   timeoutMs - reset timeout in ms

 Outputs:
   None

 Returns:
   MCD_OK               - on success
   MCD_FAIL             - on error

 Description:
   This function performs a port level hardware or software reset.

 Side effects:
   None

 Notes/Warnings:
*******************************************************************/
static CMD_STATUS wrCpssPhyMcdPortReset
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32     phyId;
    MCD_DEV_PTR pDev;
    MCD_U16     mdioPort;
    MCD_U16 host_or_line;
    MCD_RESET_TYPE resetType;
    MCD_U16 timeoutMs;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }
    mdioPort = (MCD_U16)inArgs[1];
    host_or_line = (MCD_U16)inArgs[2];
    resetType = (MCD_RESET_TYPE)inArgs[3];
    timeoutMs = (MCD_U16)inArgs[4];

    /* call cpss api function */
    status = mcdPortReset(pDev,mdioPort,host_or_line,resetType,timeoutMs);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/*******************************************************************
MCD_STATUS mcdChipAndZ80ResetControl
(
    IN MCD_DEV_PTR pDev,
    IN MCD_U16 resetType,
    IN MCD_BOOL bRestore
);

 Inputs:
   pDev  - pointer to holds device information to be used for each API call.
   resetType - From the following list
    MCD_CHIP_SW_RESET
         or
    MCD_CHIP_HW_RESET (optionally | with MCD_Z80_RESET to reset both)
    MCD_Z80_RESET (optionally | with MCD_CHIP_HW_RESET to reset both)
         or
    MCD_Z80_RESET_RELEASE (following MCD_Z80_RESET to take the Z80 out of reset)

   bRestore - restore saved registers on power-on default

 Outputs:
   None

 Returns:
   MCD_OK               - on success
   MCD_FAIL             - on error

 Description:
   If the option is MCD_CHIP_SW_RESET, does a chip-level software reset
   (this bit self clears).

   If the option is MCD_CHIP_HW_RESET, does a chip-level hardware reset
   (this bit self clears)

   MCD_CHIP_HW_RESET | MCD_Z80_RESET, will do a chip-level and also
   reset the Z80 leaving the Z80 in reset when this function exits. Call
   with MCD_Z80_RESET_RELEASE to take the Z80 out of reset.

   MCD_Z80_RESET, does a Z80 reset, leaving the Z80 in reset. Call
   with MCD_Z80_RESET_RELEASE to take the Z80 out of reset.

 Side effects:
   None

 Notes/Warnings:

*******************************************************************/
static CMD_STATUS wrCpssPhyMcdChipAndZ80ResetControl
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32     phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 resetType;
    MCD_BOOL bRestore;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }
    resetType = (MCD_U16)inArgs[1];
    bRestore = (MCD_BOOL)inArgs[2];

    /* call cpss api function */
    status = mcdChipAndZ80ResetControl(pDev,resetType,bRestore);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdSerdesManualCtleCfg function
* @endinternal
*
* @brief   Set the Serdes Manual CTLE config for DFE
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
CMD_STATUS wrCpssPhyMcdSerdesManualCtleCfg
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    MCD_STATUS  res;
    MCD_U32     phyId;
    MCD_DEV_PTR pDev;
    MCD_U8      serdesNum;

    MCD_U8  HF;
    MCD_U8  LF;
    MCD_U8  DC;
    MCD_U8  BW;
    MCD_U8  LB;
    MCD_U16 squelch;
 /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }
    serdesNum = (MCD_U8)inArgs[1];
    /* Fields List */
   DC = (GT_U8)inArgs[2];
   LF = (GT_U8)inArgs[3];
   HF = (GT_U8)inArgs[4];
   BW = (GT_U8)inArgs[5];
   LB = (GT_U8)inArgs[6];
   squelch = (GT_U16)inArgs[7];

   res = mcdSerdesManualCtleConfig(pDev, serdesNum, DC, LF, HF, BW, LB, squelch);

   if (res != MCD_OK)
   {
       galtisOutput(outArgs, res, "");
       return CMD_OK;
   }

   galtisOutput(outArgs, GT_OK, "");
   return CMD_OK;
}

/**
* @internal wrCpssPhyMcdSerdesTestGen function
* @endinternal
*
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
static CMD_STATUS wrCpssPhyMcdSerdesTestGen
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    rc;
    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16  serdesNum;
    MCD_SERDES_TX_PATTERN txP;
    MCD_SERDES_TEST_GEN_MODE mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

   serdesNum = (MCD_U16)inArgs[1];
   switch(inArgs[2])
    {
    case 0:
      txP = MCD_1T;
      break;
    case 1:
      txP = MCD_2T;
      break;
    case 2:
      txP = MCD_5T;
      break;
    case 3:
      txP = MCD_10T;
      break;
    case 4:
      txP = MCD_PRBS7;
      break;
    case 5:
      txP = MCD_PRBS9;
      break;
    case 6:
      txP = MCD_PRBS15;
      break;
    case 7:
      txP = MCD_PRBS23;
      break;
    case 8:
      txP = MCD_PRBS31;
      break;
    case 9:
      txP = MCD_DFETraining;
      break;
    default:
        galtisOutput(outArgs, GT_BAD_PARAM, "");
        return CMD_OK;
    }
    switch(inArgs[3])
    {
    case 0:
      mode = MCD_SERDES_NORMAL;
      break;
    case 1:
      mode = MCD_SERDES_TEST;
      break;
    default:
        galtisOutput(outArgs, GT_BAD_PARAM, "");
        return CMD_OK;
    }
    rc = mcdSerdesTestGen(pDev, serdesNum, txP, mode);

    galtisOutput(outArgs, rc, "");
    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdSerdesTestGenStatusGetFirst function
* @endinternal
*
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
static CMD_STATUS wrCpssPhyMcdSerdesTestGenStatusGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc; /* Return code  */

    MCD_U32     phyId;
    MCD_DEV_PTR pDev;
    MCD_U16      serdesNum;
    MCD_BOOL     counterAccumulateMode;
    MCD_SERDES_TX_PATTERN txP;
    MCD_SERDES_TEST_GEN_STATUS status;

   /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    /* get counter mode (clear on read or accumulate mode */
    counterAccumulateMode = (MCD_BOOL)inArgs[3];

    /* get serdes number */
    serdesNum = (MCD_U16)inArgs[1];

    switch(inArgs[2])
    {
    case 0:
      txP = MCD_1T;
      break;
    case 1:
      txP = MCD_2T;
      break;
    case 2:
      txP = MCD_5T;
      break;
    case 3:
      txP = MCD_10T;
      break;
    case 4:
      txP = MCD_PRBS7;
      break;
    case 5:
      txP = MCD_PRBS9;
      break;
    case 6:
      txP = MCD_PRBS15;
      break;
    case 7:
      txP = MCD_PRBS23;
      break;
    case 8:
      txP = MCD_PRBS31;
      break;
    case 9:
      txP = MCD_DFETraining;
      break;
    default:
        galtisOutput(outArgs, GT_BAD_PARAM, "");
        return CMD_OK;
    }
    rc = mcdSerdesTestGenStatus(pDev, serdesNum, txP,counterAccumulateMode,&status);

    inFields[0] = status.errorsCntr;
    inFields[1] = status.txFramesCntr.l[0];
    inFields[2] = status.txFramesCntr.l[1];
    inFields[3] = status.lockStatus;

    cpssOsPrintf("\nerrorsCntr is %d", status.errorsCntr);
    /* pack and output table fields */
    fieldOutput("%d%d%d%d",inFields[0],inFields[1],inFields[2],inFields[3]);
    galtisOutput(outArgs, rc, "%f");
    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdSerdesTestGenStatusGetNext function
* @endinternal
*
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
static CMD_STATUS wrCpssPhyMcdSerdesTestGenStatusGetNext
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
* @internal wrCpssPhyMcdSerdesEnhanceTuneSet function
* @endinternal
*
* @brief   Set the ICAL with shifted sampling point to find best sampling point
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
CMD_STATUS wrCpssPhyMcdSerdesEnhanceTuneSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS  res;

    MCD_U32     phyId;
    MCD_DEV_PTR pDev;
    MCD_U8      serdesNum;

    MCD_U8  minLF, maxLF;
    /*check for valid arguments */
     if(!inArgs || !outArgs || !inFields)
     {
         return CMD_AGENT_ERROR;
     }

     phyId = (MCD_U32)inArgs[0];
     pDev = mcdSampleGetDrvDev(phyId);
     if(pDev == NULL)
     {
         galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
         return CMD_OK;
     }
     /* get serdes number */
     serdesNum = (MCD_U8)inArgs[1];
     cpssOsPrintf("\nmcdSerdesEnhanceTune %d ", serdesNum);

     /* Fields List */
     minLF = (MCD_U8)inFields[0];
     maxLF = (MCD_U8)inFields[1];

     res = mcdSerdesEnhanceTune(pDev, &serdesNum, 1, minLF, maxLF);
     if (res != MCD_OK)
     {
        galtisOutput(outArgs, res, "");
        return CMD_OK;
     }

     galtisOutput(outArgs, GT_OK, "");
     return CMD_OK;
}
/**
* @internal wrCpssPhyMcdLedControl function
* @endinternal
*
*/
static CMD_STATUS wrCpssPhyMcdLedControl
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32   phyId;
    MCD_DEV_PTR pDev;
    MCD_U16                      ledNum;
    MCD_U16                      portNum;
    MCD_U16                      host_or_line;
    MCD_LED_PULSE_STRETCH_TYPE   pulseStretch;
    MCD_LED_BEHAVIOR_TYPE        ledMode;
    MCD_LED_FUNC_TYPE            ledDisplay;
    MCD_LED_BLINK_RATE_TYPE      blinkRate;
    MCD_LED_POLARITY_TYPE        ledPolarity;
    MCD_LED_MIX_PERCENTAGE_TYPE  ledMixPercentage;
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    ledNum = (MCD_U16)inArgs[1];
    portNum = (MCD_U16)inArgs[2];
    host_or_line= (MCD_U16)inArgs[3];
    pulseStretch= (MCD_LED_PULSE_STRETCH_TYPE)inArgs[4];
    ledMode= (MCD_LED_BEHAVIOR_TYPE)inArgs[5];
    ledDisplay= (MCD_LED_FUNC_TYPE)inArgs[6];
    blinkRate= (MCD_LED_BLINK_RATE_TYPE)inArgs[7];
    ledPolarity= (MCD_LED_POLARITY_TYPE)inArgs[8];
    ledMixPercentage = (MCD_LED_MIX_PERCENTAGE_TYPE)inArgs[9];

    status = mcdLedControl(pDev, ledNum, portNum, host_or_line, pulseStretch, ledMode, ledDisplay, blinkRate, ledPolarity, ledMixPercentage);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}
static CMD_STATUS wrCpssPhyMcdApPortInitSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;
    MCD_U32   phyId;

    MCD_DEV_PTR pDev;
    MCD_U16  portNum;
    MCD_U32  serdesNum;
    MCD_U32  modesVector;
    MCD_FEC_TYPE fecSup;
    MCD_FEC_TYPE fecReq;
    MCD_BOOL fcPause;
    MCD_BOOL fcAsmDir;
    MCD_BOOL  nonceDis;
/*    GT_BOOL  RxPolarityInvert = (GT_BOOL)inFields[8];
    GT_BOOL  TxPolarityInvert = (GT_BOOL)inFields[9];*/
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }
    portNum          = (MCD_U16)inArgs[1];
    serdesNum        = (MCD_U32)inArgs[2];
    modesVector      = (MCD_U32)inArgs[3];
    fecSup           = (MCD_FEC_TYPE)inArgs[4];
    fecReq           = (MCD_FEC_TYPE)inArgs[5];
    fcPause          = (MCD_BOOL)inArgs[6];
    fcAsmDir         = (MCD_BOOL)inArgs[7];
    nonceDis         = (MCD_BOOL)inArgs[8];

    cpssOsPrintf("Parameters:\n");
    cpssOsPrintf("-----------\n\n");
    cpssOsPrintf("\tPort Num:      %d\n", portNum);
    cpssOsPrintf("\tLane Num:      %d\n", serdesNum);
    cpssOsPrintf("\tMode Mask:     0x%x\n", modesVector);
    cpssOsPrintf("\tLoopback:      %s\n", (nonceDis) ? "Enabled" : "Disabled");
    cpssOsPrintf("\tFECSup:        %d\n", fecSup);
    cpssOsPrintf("\tFECReq:        %s\n", (fecReq) ? "Enabled" : "Disabled");
    cpssOsPrintf("\tFC:            %s\n", (fcPause) ? "Enabled" : "Disabled");
    cpssOsPrintf("\tFC Asymmetric: %s\n", (fcAsmDir) ? "Enabled" : "Disabled");

    status = mcdSetAPModeSelection( pDev, portNum, serdesNum, modesVector, fcPause, fcAsmDir, fecSup, fecReq, nonceDis, NULL, MCD_TRUE);

    galtisOutput(outArgs,status, "");
    return CMD_OK;
}

/*******************************************************************************
* wrCpssPhyMcdApPortInitWithPolaritySet
*
*******************************************************************************/
static CMD_STATUS wrCpssPhyMcdApPortInitWithPolaritySet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;
    MCD_U32   phyId;

    MCD_DEV_PTR pDev;
    MCD_U16  portNum;
    MCD_U32  serdesNum;
    MCD_U32  modesVector,polarityVector,i;
    MCD_FEC_TYPE fecSup;
    MCD_FEC_TYPE fecReq;
    MCD_BOOL  fcPause;
    MCD_BOOL  fcAsmDir;
    MCD_BOOL  nonceDis;
    MCD_CONFIG_SERDES_AP_PARAM config;
/*    GT_BOOL  RxPolarityInvert = (GT_BOOL)inFields[8];
    GT_BOOL  TxPolarityInvert = (GT_BOOL)inFields[9];*/
    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }
    portNum          = (MCD_U16)inArgs[1];
    serdesNum        = (MCD_U32)inArgs[2];
    modesVector      = (MCD_U32)inArgs[3];
    fecSup           = (MCD_FEC_TYPE)inArgs[4];
    fecReq           = (MCD_FEC_TYPE)inArgs[5];
    fcPause          = (MCD_BOOL)inArgs[6];
    fcAsmDir         = (MCD_BOOL)inArgs[7];
    nonceDis         = (MCD_BOOL)inArgs[8];
    polarityVector   = (MCD_U32)inArgs[9];

    cpssOsPrintf("Parameters:\n");
    cpssOsPrintf("-----------\n\n");
    cpssOsPrintf("\tPort Num:      %d\n", portNum);
    cpssOsPrintf("\tLane Num:      %d\n", serdesNum);
    cpssOsPrintf("\tMode Mask:     0x%x\n", modesVector);
    cpssOsPrintf("\tpolarity:      0x%x\n", polarityVector);
    cpssOsPrintf("\tLoopback:      %s\n", (nonceDis) ? "Enabled" : "Disabled");
    cpssOsPrintf("\tFECSup:        %d\n", fecSup);
    cpssOsPrintf("\tFECReq:        %s\n", (fecReq) ? "Enabled" : "Disabled");
    cpssOsPrintf("\tFC:            %s\n", (fcPause) ? "Enabled" : "Disabled");
    cpssOsPrintf("\tFC Asymmetric: %s\n", (fcAsmDir) ? "Enabled" : "Disabled");

    config.numOfLanes = 4;
    for(i = 0; i < 4; i++)
    {
        serdesApParameters[i].txPolarity = (polarityVector  >> (2*i+1)) & 1;
        serdesApParameters[i].rxPolarity = (polarityVector  >> (2*i)) & 1;
        cpssOsPrintf("txPolarity: %d\n", serdesApParameters[i].txPolarity);
        cpssOsPrintf("rxPolarity: %d\n", serdesApParameters[i].rxPolarity);
    }
    config.params = (MCD_AP_SERDES_CONFIG_DATA *)serdesApParameters;

    status = mcdSetAPModeSelection( pDev, portNum, serdesNum, modesVector, fcPause, fcAsmDir, fecSup, fecReq, nonceDis, &config, MCD_TRUE);

    galtisOutput(outArgs,status, "");
    return CMD_OK;
}
/* counter for cpssDxChPortApPortStats table */
static GT_PHYSICAL_PORT_NUM   portStatsEntryCount;

/**
* @internal wrCpssPhyMcdPortApPortsStatsGet function
* @endinternal
*
* @brief   Get AP port statistics information
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; xCat3; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - apStatusPtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - AP engine not run
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPhyMcdPortApPortsStatsGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    IN  GT_BOOL     invokedForAllPorts,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result = GT_OK;
    MCD_AP_PORT_STATS               apStats;
    MCD_U32   phyId;
    MCD_DEV_PTR pDev;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    phyId = (GT_U8)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);

    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    if(invokedForAllPorts)
    {

        if(portStatsEntryCount >= 8)
        {
            /* no ports */
            galtisOutput(outArgs, GT_OK, "%d", -1);
            return CMD_OK;
        }
    }
    else
    {
        portStatsEntryCount = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    }

    /* Initializing apStats structure */
    cpssOsMemSet(&apStats, 0, sizeof(MCD_AP_PORT_STATS));

    /* call cpss api function */
    if (pDev->apCfg[portStatsEntryCount].apLaneNum  != 0xFF)
    {
        result =  mcdApPortCtrlStatsGet(pDev, portStatsEntryCount, &apStats);
    }

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    if (!invokedForAllPorts)
    {
        inFields[0] = apStats.txDisCnt;
        inFields[1] = apStats.abilityCnt;
        inFields[2] = apStats.abilitySuccessCnt;
        inFields[3] = apStats.linkFailCnt;
        inFields[4] = apStats.linkSuccessCnt;
        inFields[5] = apStats.hcdResoultionTime;
        inFields[6] = apStats.linkUpTime;

        fieldOutput("%d%d%d%d%d%d%d", inFields[0], inFields[1], inFields[2],
                    inFields[3], inFields[4], inFields[5], inFields[6]);
    }
    else
    {
        inFields[0] = portStatsEntryCount;
        inFields[1] = apStats.txDisCnt;
        inFields[2] = apStats.abilityCnt ;
        inFields[3] = apStats.abilitySuccessCnt;
        inFields[4] = apStats.linkFailCnt;
        inFields[5] = apStats.linkSuccessCnt;
        inFields[6] = apStats.hcdResoultionTime;
        inFields[7] = apStats.linkUpTime;

        portStatsEntryCount++;

        fieldOutput("%d%d%d%d%d%d%d%d", inFields[0], inFields[1], inFields[2],
                    inFields[3], inFields[4], inFields[5], inFields[6], inFields[7]);

    }

    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}

static CMD_STATUS  wrCpssPhyMcdPortApPortsStatsGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    portStatsEntryCount = 0;

    return wrCpssPhyMcdPortApPortsStatsGet(inArgs, inFields, numFields,
                                                 GT_TRUE, outArgs);
}

static CMD_STATUS  wrCpssPhyMcdPortApPortsStatsGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssPhyMcdPortApPortsStatsGet(inArgs, inFields, numFields,
                                                 GT_TRUE, outArgs);
}

static CMD_STATUS  wrCpssPhyMcdPortApPortStatsGetFirst /* one port*/
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return wrCpssPhyMcdPortApPortsStatsGet(inArgs, inFields, numFields,
                                                 GT_FALSE, outArgs);
}


/**
* @internal wrCpssPhyMcdPortApPortStatsGetNext function
* @endinternal
*
* @brief   This function is called for ending table printing.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
*/
static CMD_STATUS wrCpssPhyMcdPortApPortStatsGetNext

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
* @internal wrCpssPhyMcdPollingStatValueGet function
* @endinternal
*
* @brief   Print FW Hws real-time polling statistic information stored in system
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static CMD_STATUS wrCpssPhyMcdPollingStatValueGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    MCD_U32                             phyId;
    MCD_DEV_PTR                         pDev;
    MCD_U32                             pollingStatVal;
    MCD_U32                             portOrSerdes;
    MCD_U32                             lineSide;
    MCD_POLLING_PORT_STATISTIC_VAL_ENT  valueType;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    phyId         = (MCD_U8)inArgs[0];
    portOrSerdes  = (MCD_U32)inArgs[1];
    lineSide      = (MCD_U32)inArgs[2];
    valueType     = (MCD_POLLING_PORT_STATISTIC_VAL_ENT)inArgs[3];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    result = mcdPollingStatValueGet(
        pDev, portOrSerdes, lineSide, valueType, &pollingStatVal);
    galtisOutput(outArgs, result, "%d", pollingStatVal);

    return CMD_OK;
}

/******************************************************************************
 MCD_STATUS mcdSyncEClkConfig

  Description:
    The function defines which pin should output the recovered clock and if squelch
    is enable, call the API mcdSquelchRxClockSet.

 Inputs:
    pDev - pointer to MCD_DEV initialized by mcdLoadDriver() call
    mdioPort - MDIO port address, 0-7.
    laneNum - SerDes lane number, 0-3.
    divider - selects the division value of the recovered clock.
    squelchEnable - MCD_TRUE - enable Rx squelch,
                    MCD_FALSE - ignore Rx squelch.

 Outputs:
    outPin - configures the pin to output the selected divided recvored
             clock.

 Returns:
    MCD_OK if successful, MCD_FAIL if not. Will return
    MCD_FAIL if the parameters are not valid or if link on line side is not UP

 Side effects:
    None.

 Notes/Warnings:
 This function can be called after the port has been initialized and link is UP

******************************************************************************/
static CMD_STATUS wrCpssPhyMcdSyncEClkConfig
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MCD_STATUS    status;

    MCD_U32 phyId;
    MCD_DEV_PTR pDev;
    MCD_U16 mdioPort;
    MCD_U32 laneNum;
    MCD_RCLK_DIV_VAL divider;
    MCD_BOOL squelchEnable;
    MCD_RCLK_OUT_PIN outPin;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    phyId = (MCD_U32)inArgs[0];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    mdioPort = (MCD_U16)inArgs[1];
    laneNum = (MCD_U32)inArgs[2];
    divider = (MCD_RCLK_DIV_VAL)inArgs[3];
    squelchEnable = (MCD_FEC_TYPE)inArgs[4];
    outPin = (MCD_RCLK_OUT_PIN)inArgs[5];

    /* call cpss api function */
    status = mcdSyncEClkConfig(pDev,mdioPort,laneNum,divider,squelchEnable,outPin);
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssPhyMcdPollingStatRetimerPerPortValuesClear function
* @endinternal
*
* @brief   Clear FW real-time polling statistic related to retimer port side.
*
* @retval MCD_OK                   - on success
* @retval MCD_FAIL                 - on wrong parameters
*/
static CMD_STATUS wrCpssPhyMcdPollingStatRetimerPerPortValuesClear
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    MCD_U32                             phyId;
    MCD_DEV_PTR                         pDev;
    MCD_U32                             portNum;
    MCD_U32                             lineSide;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    phyId         = (MCD_U8)inArgs[0];
    portNum       = (MCD_U32)inArgs[1];
    lineSide      = (MCD_U32)inArgs[2];
    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        galtisOutput(outArgs, MCD_STATUS_NOT_INITIALIZED, "");
        return CMD_OK;
    }

    result = mcdPollingStatRetimerPerPortValuesClear(
        pDev, portNum, lineSide);
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**** database initialization **************************************/
static CMD_COMMAND dbCommands[] =
{
        {"mcdSampleInitDrv",
            &wrCpssPhyMcdSampleDrvInit,
            4, 0},
        {"mcdSampleInitDrvLoadAllDevices",
            &wrCpssPhyMcdSampleInitDrvLoadAllDevices,
            2, 0},
        {"mcdUnloadDriver",
            &wrCpssPhyMcdSampleDrvUnload,
            1, 0},
        {"mcdHwXmdioWrite",
            &wrCpssPhyMcdHwXmdioWrite,
            5, 0},
        {"mcdHwXmdioRead",
            &wrCpssPhyMcdHwXmdioRead,
            4, 0},
        {"mcdHwSetPhyRegMask32bit",
            &wrCpssPhyMcdHwPhyRegMask32bitSet,
            5, 0},
        {"mcdHwGetPhyRegMask32bit",
            &wrCpssPhyMcdHwPhyRegMask32bitGet,
            4, 0},
        {"mcdHwXmdioReadWrite32bitSet",
            &wrCpssPhyMcdHwXmdioWrite32bitSet,
            4, 5},
        {"mcdHwXmdioReadWrite32bitGetFirst",
            &wrCpssPhyMcdHwXmdioRead32bitFirstEntry,
            4, 0},
        {"mcdHwXmdioReadWrite32bitGetNext",
            &wrCpssPhyMcdHwXmdioRead32bitNextEntry,
            4, 0},
        {"mcdAutoNegStart",
            &wrCpssPhyMcdAutoNegStart,
            4, 0},
        {"mcdAutoNegEnable",
            &wrCpssPhyMcdAutoNegEnable,
            3, 0},
        {"mcdAutoNegDisable",
            &wrCpssPhyMcdAutoNegDisable,
            4, 0},
        {"mcdAutoNegCheckCompleteExt",
            &wrCpssPhyMcdAutoNegCheckCompleteExt,
            4, 0},
        {"mcdCheckLinkStatusGetFirst",
            &wrCpssPhyMcdCheckLinkStatusFirstEntry,
            2, 0},
        {"mcdCheckLinkStatusGetNext",
            &wrCpssPhyMcdCheckLinkStatusNextEntry,
            2, 0},
        {"mcdGetPcsFaultStatus",
            &wrCpssPhyMcdGetPcsFaultStatus,
            3, 0},
        {"mcdSetSerdesElectricalParams",
            &wrCpssPhyMcdSetSerdesElectricalParams,
            6, 0},
        {"mcdSetModeSelectionExt",
            &wrCpssPhyMcdMcdSetModeSelectionExt,
            7, 0},
        {"mcdSetLaneSteering",
            &wrCpssPhyMcdSetLaneSteering,
            12,0},
        {"mcdSetAutoNegAdvertisingSpeeds",
            &wrCpssPhyMcdSetAutoNegAdvertisingSpeeds,
            4, 0},
        {"mcdGetAutoNegAdvertisingSpeeds",
            &wrCpssPhyMcdGetAutoNegAdvertisingSpeeds,
            3, 0},
        {"mcdSetRsFecControl",
            &wrCpssPhyMcdSetRsFecControl,
            4, 0},
        {"mcdGetRsFecStatus",
            &wrCpssPhyMcdGetRsFecStatus,
            2, 0},
        {"mcdGetRsFecPCSAlignmentStatus",
            &wrCpssPhyMcdGetRsFecPCSAlignmentStatus,
            3, 0},
        {"mcdGetRsFecPMALaneMapping",
            &wrCpssPhyMcdGetRsFecPMALaneMapping,
            2, 0},
        {"mcdGetRsFecPCSLaneMapping",
            &wrCpssPhyMcdGetRsFecPCSLaneMapping,
            3, 0},
        {"mcdGetFecCorrectedCwCntr",
            &wrCpssPhyMcdGetRsFecCorrectedCwCntr,
            2, 0},
        {"mcdGetFecUnCorrectedCwCntr",
            &wrCpssPhyMcdGetRsFecUnCorrectedCwCntr,
            2, 0},
        {"mcdGetFecSymbolErrorCntr",
            &wrCpssPhyMcdGetRsFecSymbolErrorCntr,
            3, 0},
        {"mcdCheckChipCapablities",
            &wrCpssPhyMcdCheckChipCapablities,
            2, 0},
        {"mcdGetChipRevision",
            &wrCpssPhyMcdGetChipRevision,
            2, 0},
        {"mcdGetChipFWRevision",
            &wrCpssPhyMcdGetChipFWRevision,
            1, 0},
        {"mcdGetSerdesSignalDetectAndDspLock",
            &wrCpssPhyMcdGetSerdesSignalDetectAndDspLock,
            4, 0},
        {"mcdSetLineLoopback",
            &wrCpssPhyMcdSetLineLoopback,
            4, 0},
        {"mcdSetHostLoopback",
            &wrCpssPhyMcdSetHostLoopback,
            4, 0},
        {"mcdConfigurePktGeneratorChecker",
            &wrCpssPhyMcdConfigurePktGeneratorChecker,
            12, 0},
        {"mcdEnablePktGeneratorChecker",
            &wrCpssPhyMcdEnablePktGeneratorChecker,
            5, 0},
        {"mcdPktGeneratorCounterReset",
            &wrCpssPhyMcdPktGeneratorCounterReset,
            3, 0},
        {"mcdPktGeneratorGetCounter",
            &wrCpssPhyMcdPktGeneratorGetCounter,
            4, 0},
        {"mcdSetLineSidePRBSPattern",
            &wrCpssPhyMcdSetLineSidePRBSPattern,
            5, 0},
        {"mcdSetPRBSEnableTxRx",
            &wrCpssPhyMcdSetPRBSEnableTxRx,
            6, 0},
        {"mcdPRBSCounterReset",
            &wrCpssPhyMcdPRBSCounterReset,
            4, 0},
        {"mcdSetPRBSWaitForLock",
            &wrCpssPhyMcdSetPRBSWaitForLock,
            5, 0},
        {"mcdSetPRBSClearOnRead",
            &wrCpssPhyMcdSetPRBSClearOnRead,
            5, 0},
        {"mcdGetPRBSClearOnRead",
            &wrCpssPhyMcdGetPRBSClearOnRead,
            4, 0},
        {"mcdGetPRBSLocked",
            &wrCpssPhyMcdGetPRBSLocked,
            4, 0},
        {"mcdGetPRBSCounts",
            &wrCpssPhyMcdGetPRBSCounts,
            4, 0},
        {"mcdSetTxPolarity",
            &wrCpssPhyMcdSetTxPolarity,
            6, 0},
        {"mcdGetTxPolarity",
            &wrCpssPhyMcdGetTxPolarity,
            4, 0},
        {"mcdSetRxPolarity",
            &wrCpssPhyMcdSetRxPolarity,
            6, 0},
        {"mcdGetRxPolarity",
            &wrCpssPhyMcdGetRxPolarity,
            4, 0},
        {"mcdSetTxFFE",
            &wrCpssPhyMcdSetTxFFE,
            8, 0},
        {"mcdGetTxFFE",
            &wrCpssPhyMcdGetTxFFE,
            4, 0},
        {"mcdSerdesSpicoInterrupt",
            &wrCpssPhyMcdSerdesSpicoInterrupt,
            4, 0},
        {"mcdSerdesManualTxConfig",
            &wrCpssPhyMcdSerdesManualTxConfig,
            7, 0},
        {"mcdSerdesPowerCtrl",
            &wrCpssPhyMcdSerdesPowerCtrl,
            4, 0},
        {"mcdSerdesGetRevision",
            &wrCpssPhyMcdSerdesGetRevision,
            1, 0},
        {"mcdSerdesGetVoltage",
            &wrCpssPhyMcdSerdesGetVoltage,
            1, 0},
        {"mcdSerdesGetTemperature",
            &wrCpssPhyMcdSerdesGetTemperature,
            1, 0},
        {"mcdSerdesGetTxFFE",
            &wrCpssPhyMcdSerdesGetTxFFE,
            2, 0},
        {"mcdSerdesSetTxFFE",
            &wrCpssPhyMcdSerdesSetTxFFE,
            6, 0},
        {"mcdSerdesGetTxPolarity",
            &wrCpssPhyMcdSerdesGetTxPolarity,
            2, 0},
        {"mcdSerdesSetTxPolarity",
            &wrCpssPhyMcdSerdesSetTxPolarity,
            3, 0},
        {"mcdSerdesGetRxPolarity",
            &wrCpssPhyMcdSerdesGetRxPolarity,
            2, 0},
        {"mcdSerdesSetRxPolarity",
            &wrCpssPhyMcdSerdesSetRxPolarity,
            3, 0},
        {"mcdSerdesGetCDRLock",
            &wrCpssPhyMcdSerdesGetCDRLock,
            2, 0},
        {"mcdSerdesTxInjectError",
            &wrCpssPhyMcdSerdesTxInjectError,
            3, 0},
        {"mcdSerdesRxInjectError",
            &wrCpssPhyMcdSerdesRxInjectError,
            3, 0},
        {"mcdSerdesSetTxUserData",
            &wrCpssPhyMcdSerdesSetTxUserData,
            6, 0},
        {"mcdSerdesGetTxUserData",
            &wrCpssPhyMcdSerdesGetTxUserData,
            2, 0},
        {"mcdSerdesGetErrCount",
            &wrCpssPhyMcdSerdesGetErrCount,
            3, 0},
        {"mcdSerdesSetLoopback",
            &wrCpssPhyMcdSerdesSetLoopback,
            3, 0},
        {"mcdSerdesReset",
            &wrCpssPhyMcdSerdesReset,
            2, 0},
        {"mcdSerdesAccessLock",
            &wrCpssPhyMcdSerdesAccessLock,
            1, 0},
        {"mcdSerdesAccessUnlock",
            &wrCpssPhyMcdSerdesAccessUnlock,
            1, 0},
        {"mcdSerdesResetImpl",
            &wrCpssPhyMcdSerdesResetImpl,
            5, 0},
        {"mcdSetHostSidePRBSPattern",
            &wrCpssPhyMcdSetHostSidePRBSPattern,
            4, 0},
        {"mcdGetPRBSWaitForLock",
            &wrCpssPhyMcdGetPRBSWaitForLock,
            4, 0},
        {"mcdGetRsFecBipErrorCntr",
            &wrCpssPhyMcdGetRsFecBipErrorCntr,
            3, 0},
        {"mcdSerdesPMDTrainingLog",
            &wrCpssPhyMcdSerdesPMDTrainingLog,
            2, 0},
        {"mcdSerdesSetTxPRBS",
            &wrCpssPhyMcdSerdesSetTxPRBS,
            3, 0},
        {"mcdSerdesGetTxPRBS",
            &wrCpssPhyMcdSerdesGetTxPRBS,
            2, 0},
        {"mcdSerdesSetRxCmpData",
            &wrCpssPhyMcdSerdesSetRxCmpData,
            3, 0},
        {"mcdSerdesGetRxCmpData",
            &wrCpssPhyMcdSerdesGetRxCmpData,
            2, 0},
        {"mcdSetInterruptEnableEx",
            &wrCpssPhyMcdSetInterruptEnableEx,
            5, 0},
        {"mcdGetInterruptEnableEx",
            &wrCpssPhyMcdGetInterruptEnableEx,
            4, 0},
        {"mcdGetInterruptStatusEx",
            &wrCpssPhyMcdGetInterruptStatusEx,
            5, 0},
        {"mcdGetRealtimeStatusEx",
            &wrCpssPhyMcdGetRealtimeStatusEx,
            4, 0},
        {"mcdSerdesTestGen",
            &wrCpssPhyMcdSerdesTestGen,
            4, 0},
        {"mcdSerdesTestGenStatusGetFirst",
            &wrCpssPhyMcdSerdesTestGenStatusGetFirst,
            4,  0},
        {"mcdSerdesTestGenStatusGetNext",
            &wrCpssPhyMcdSerdesTestGenStatusGetNext,
            4,  0},
        {"mcdSerdesAccessValidate",
            &wrCpssPhyMcdSerdesAccessValidate,
            2, 0},
        {"mcdSerdesAutoTuneResultGetFirst",
            &wrCpssPhyMcdSerdesAutoTuneResultsGetFirst,
            3,  0},
        {"mcdSerdesAutoTuneResultGetNext",
            &wrCpssPhyMcdSerdesAutoTuneResultsGetNext,
            3,  0},
        {"mcdSerdesAutoTuneStatusShort",
            &wrCpssPhyMcdSerdesAutoTuneStatusShort,
            2, 0},
        {"mcdSerdesAutoTuneStartExt",
            &wrCpssPhyMcdSerdesAutoTuneStartExt,
            4, 0},
        {"mcdSerdesAutoTuneStart",
            &wrCpssPhyMcdSerdesAutoTuneStart,
            4, 0},
        {"mcdSerdesManualCtleConfig",
            &wrCpssPhyMcdSerdesManualCtleCfg,
            8, 0},
        {"mcdSerdesEnhanceTuneSet",
            &wrCpssPhyMcdSerdesEnhanceTuneSet,
            2, 2},
        {"mcdPortReset",
            &wrCpssPhyMcdPortReset,
            5, 0},
        {"mcdLedControl",
            &wrCpssPhyMcdLedControl,
            10, 0},
        {"mcdPortApStatsGetFirst",
            &wrCpssPhyMcdPortApPortsStatsGetFirst,
            1, 0},
        {"mcdPortApStatsGetNext",
            &wrCpssPhyMcdPortApPortsStatsGetNext,
            1, 0},
        {"mcdPortApStatsOnPortGetFirst",
            &wrCpssPhyMcdPortApPortStatsGetFirst,
            2, 0},
        {"mcdPortApStatsOnPortGetNext",
            &wrCpssPhyMcdPortApPortStatsGetNext,
            2, 0},
        {"mcdApPortInitSet",
            &wrCpssPhyMcdApPortInitSet,
            9, 0},
        {"mcdApWithPolarityPortInitSet",
            &wrCpssPhyMcdApPortInitWithPolaritySet,
            10, 0},
        {"mcdChipAndZ80ResetControl",
            &wrCpssPhyMcdChipAndZ80ResetControl,
            3, 0},
        {"mcdPollingStatValueGet",
            &wrCpssPhyMcdPollingStatValueGet,
            4, 0},
         {"mcdSyncEClkConfig",
            &wrCpssPhyMcdSyncEClkConfig,
            6, 0},
        {"mcdPollingStatRetimerPerPortValuesClear",
            &wrCpssPhyMcdPollingStatRetimerPerPortValuesClear,
            3, 0}

};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssPhyMcd function
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
GT_STATUS cmdLibInitCpssPhyMcd
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}

/*****************************************************************************/
/* debug purpose functions */

/* use mcdSerdesSetRetimerRequireSignalOk to allow Selection Retimer modes without link */
/* call mcdSamplePortStop  before not first mcdOneSideSetMode */

MCD_STATUS wr_mcd_serdes_debug_PowerCtrl
(
    unsigned int        phyIndex,
    unsigned int        serdesNum,
    unsigned char       powerUp,
    MCD_SERDES_SPEED    baudRate,
    MCD_BOOL            retimerMode
)
{
#ifndef ASIC_SIMULATION
    MCD_DEV_PTR                pDev;
    int                        rc;

    pDev = mcdSampleGetDrvDev(phyIndex);
    if (pDev == NULL)
    {
        cpssOsPrintf("no device with given ID %d\n", phyIndex);
        return MCD_OK;
    }

    rc = mcdSerdesPowerCtrl(
        pDev, serdesNum, powerUp, baudRate, retimerMode, 0/*primary*/, 1/* no division*/);
    cpssOsPrintf("RC: %d\n", rc);
#endif
    return MCD_OK;
}

#ifndef ASIC_SIMULATION
static struct
{
    MCD_U8 divider;
    MCD_U8 width;
} mcdBaudRate2DividerAndWidth[]=
{
    {0,    0},         /*  SPEED_NA   */
    {8,   10},         /*  _1_25G     */
    {20,  10},         /*  _3_125G    */
    {32,  40},         /*  _5G        */
    {40,  40},         /*  _6_25G     */
    {66,  20},         /*  _10_3125G  */
    {70,  40},         /*  _10_9375G  */
    {78,  20},         /*  _12_1875G  */
    {80,  40},         /*  _12_5G     */
    {132, 40},         /*  _20_625G   */
    {165, 40},         /*  _25_78125G */
    {180, 40}          /*  _28_05G    */
};
#endif

MCD_STATUS wr_mcd_serdes_debug_avago_serdes_init
(
    unsigned int        phyIndex,
    unsigned int        serdesNum,
    unsigned int        speed
)
{
#ifndef ASIC_SIMULATION
    MCD_DEV_PTR                pDev;
    int                        rc;
    MCD_SER_DEV_PTR            pSerdesDev;
    Avago_serdes_init_config_t config;

    pDev = mcdSampleGetDrvDev(phyIndex);
    if (pDev == NULL)
    {
        cpssOsPrintf("no device with given ID %d\n", phyIndex);
        return MCD_OK;
    }
    pSerdesDev = (MCD_SER_DEV_PTR)pDev->serdesDev;
    pSerdesDev->return_code = 0;
    avago_serdes_init_config(&config);
    config.tx_divider = mcdBaudRate2DividerAndWidth[speed].divider;
    config.rx_divider = mcdBaudRate2DividerAndWidth[speed].divider;
    /* initializes the Avago_serdes_init_config_t struct */
    config.sbus_reset = FALSE;
    config.signal_ok_threshold = 2;
    config.tx_width = mcdBaudRate2DividerAndWidth[speed].width;
    config.rx_width = mcdBaudRate2DividerAndWidth[speed].width;

    rc = avago_serdes_init(pSerdesDev, serdesNum, &config);
    cpssOsPrintf("RC: %d return_code: %d\n", rc, pSerdesDev->return_code);
#endif
    return MCD_OK;
}

MCD_STATUS wr_mcd_serdes_debug_default_ManualCtleConfig
(
    unsigned int        phyIndex,
    unsigned int        serdesNum
)
{
#ifndef ASIC_SIMULATION
    MCD_DEV_PTR                pDev;
    int                        rc;

    pDev = mcdSampleGetDrvDev(phyIndex);
    if (pDev == NULL)
    {
        cpssOsPrintf("no device with given ID %d\n", phyIndex);
        return MCD_OK;
    }

    rc = mcdSerdesManualCtleConfig (
        pDev, serdesNum,
        MCD_SERDES_DEFAILT_DC_GAIN, MCD_SERDES_DEFAILT_LOW_FREQUENCY,
        MCD_SERDES_DEFAILT_HIGH_FREQUENCY,MCD_SERDES_DEFAILT_BANDWIDTH,
        MCD_SERDES_DEFAILT_LOOP_BANDWIDTH,MCD_SERDES_DEFAILT_SQUELCH);
    cpssOsPrintf("RC: %d\n", rc);
#endif
    return MCD_OK;
}

MCD_STATUS wr_mcd_serdes_debug_set_tx_output_enable
(
    unsigned int        phyIndex,
    unsigned int        serdesNum,
    unsigned int        enable
)
{
#ifndef ASIC_SIMULATION
    MCD_DEV_PTR                pDev;
    int                        rc;
    MCD_SER_DEV_PTR            pSerdesDev;

    pDev = mcdSampleGetDrvDev(phyIndex);
    if (pDev == NULL)
    {
        cpssOsPrintf("no device with given ID %d\n", phyIndex);
        return MCD_OK;
    }
    pSerdesDev = (MCD_SER_DEV_PTR)pDev->serdesDev;
    pSerdesDev->return_code = 0;

    rc = avago_serdes_set_tx_output_enable(pSerdesDev, serdesNum, (MCD_BOOL)enable);
    cpssOsPrintf("RC: %d return_code: %d\n", rc, pSerdesDev->return_code);
#endif
    return MCD_OK;
}

MCD_STATUS wr_mcd_serdes_debug_initialize_signal_ok
(
    unsigned int        phyIndex,
    unsigned int        serdesNum,
    unsigned int        threshold
)
{
#ifndef ASIC_SIMULATION
    MCD_DEV_PTR         pDev;
    MCD_BOOL            rc;
    MCD_SER_DEV_PTR     pSerdesDev;

    pDev = mcdSampleGetDrvDev(phyIndex);
    if (pDev == NULL)
    {
        cpssOsPrintf("no device with given ID %d\n", phyIndex);
        return MCD_OK;
    }
    pSerdesDev = (MCD_SER_DEV_PTR)pDev->serdesDev;
    pSerdesDev->return_code = 0;

    rc = avago_serdes_initialize_signal_ok(pSerdesDev, serdesNum, (int)threshold);
    cpssOsPrintf(
        "Signal OK enabled RC: %d return_code: %d\n",
        rc, pSerdesDev->return_code);
#endif
    return MCD_OK;
}

MCD_STATUS wr_mcd_serdes_debug_get_signal_ok_live
(
    unsigned int        phyIndex,
    unsigned int        serdesNum
)
{
#ifndef ASIC_SIMULATION
    MCD_DEV_PTR         pDev;
    MCD_BOOL            sig_ok;
    MCD_SER_DEV_PTR     pSerdesDev;

    pDev = mcdSampleGetDrvDev(phyIndex);
    if (pDev == NULL)
    {
        cpssOsPrintf("no device with given ID %d\n", phyIndex);
        return MCD_OK;
    }
    pSerdesDev = (MCD_SER_DEV_PTR)pDev->serdesDev;
    pSerdesDev->return_code = 0;

    sig_ok = avago_serdes_get_signal_ok_live(pSerdesDev, serdesNum);
    cpssOsPrintf(
        "Signal live: %d return_code: %d\n",
        sig_ok, pSerdesDev->return_code);
#endif
    return MCD_OK;
}

MCD_STATUS wr_mcd_serdes_debug_get_signal_ok
(
    unsigned int        phyIndex,
    unsigned int        sAddr,
    unsigned int        reset
)
{
#ifndef ASIC_SIMULATION
    MCD_DEV_PTR         pDev;
    MCD_BOOL            sig_ok;
    MCD_SER_DEV_PTR     pSerdesDev;

    pDev = mcdSampleGetDrvDev(phyIndex);
    if (pDev == NULL)
    {
        cpssOsPrintf("no device with given ID %d\n", phyIndex);
        return MCD_OK;
    }
    pSerdesDev = (MCD_SER_DEV_PTR)pDev->serdesDev;
    pSerdesDev->return_code = 0;

    sig_ok = avago_serdes_get_signal_ok(pSerdesDev, sAddr, reset);
    cpssOsPrintf(
        "Signal OK: %d return_code: %d\n",
        sig_ok, pSerdesDev->return_code);
#endif
    return MCD_OK;
}

MCD_STATUS wr_mcd_serdes_debug_get_signal_ok_enable
(
    unsigned int        phyIndex,
    unsigned int        serdesNum
)
{
#ifndef ASIC_SIMULATION
    MCD_DEV_PTR         pDev;
    MCD_BOOL            sig_ok;
    MCD_SER_DEV_PTR     pSerdesDev;

    pDev = mcdSampleGetDrvDev(phyIndex);
    if (pDev == NULL)
    {
        cpssOsPrintf("no device with given ID %d\n", phyIndex);
        return MCD_OK;
    }
    pSerdesDev = (MCD_SER_DEV_PTR)pDev->serdesDev;
    pSerdesDev->return_code = 0;

    sig_ok = avago_serdes_get_signal_ok_enable(pSerdesDev, serdesNum);
    cpssOsPrintf(
        "Signal OK enable: %d return_code: %d\n",
        sig_ok, pSerdesDev->return_code);
#endif
    return MCD_OK;
}

MCD_STATUS wr_mcd_serdes_debug_get_signal_ok_threshold
(
    unsigned int        phyIndex,
    unsigned int        serdesNum
)
{
#ifndef ASIC_SIMULATION
    MCD_DEV_PTR         pDev;
    MCD_BOOL            sig_ok;
    MCD_SER_DEV_PTR     pSerdesDev;

    pDev = mcdSampleGetDrvDev(phyIndex);
    if (pDev == NULL)
    {
        cpssOsPrintf("no device with given ID %d\n", phyIndex);
        return MCD_OK;
    }
    pSerdesDev = (MCD_SER_DEV_PTR)pDev->serdesDev;
    pSerdesDev->return_code = 0;

    sig_ok = avago_serdes_get_signal_ok_threshold(pSerdesDev, serdesNum);
    cpssOsPrintf(
        "Signal OK threshold: %d return_code: %d\n",
        sig_ok, pSerdesDev->return_code);
#endif
    return MCD_OK;
}

/* similfied copy of avago_serdes_get_tx_rx_ready */
/* the function not present in reduced build      */
void util_mcd_avago_serdes_get_tx_rx_ready(
    Aapl_t *aapl,            /**< [in] Pointer to Aapl_t structure. */
    uint addr,               /**< [in] Device address number. */
    unsigned int *tx_ready,  /**< [out] Where to store TX state. */
    unsigned int *rx_ready)  /**< [out] Where to store RX state. */
{
#ifndef ASIC_SIMULATION
    int bits = avago_serdes_mem_rd(aapl, addr, AVAGO_LSB, 0x026);
    *tx_ready = (bits & 0x01);
    *rx_ready = (bits & 0x02);
#endif
}

MCD_STATUS wr_mcd_serdes_debug_get_tx_rx_ready
(
    unsigned int        phyIndex,
    unsigned int        serdesNum
)
{
#ifndef ASIC_SIMULATION
    MCD_DEV_PTR         pDev;
    MCD_SER_DEV_PTR     pSerdesDev;
    unsigned int        tx_ready;
    unsigned int        rx_ready;

    pDev = mcdSampleGetDrvDev(phyIndex);
    if (pDev == NULL)
    {
        cpssOsPrintf("no device with given ID %d\n", phyIndex);
        return MCD_OK;
    }
    pSerdesDev = (MCD_SER_DEV_PTR)pDev->serdesDev;
    pSerdesDev->return_code = 0;

    util_mcd_avago_serdes_get_tx_rx_ready(
        pSerdesDev, serdesNum, &tx_ready, &rx_ready);
    cpssOsPrintf(
        "tx_ready: %d rx_ready: %d return_code: %d\n",
        tx_ready, rx_ready, pSerdesDev->return_code);
#endif
    return MCD_OK;
}

MCD_STATUS wr_mcd_mcdCheckSerdesSignalStability
(
    unsigned int        phyIndex,
    unsigned int        serdesNum
)
{
#ifndef ASIC_SIMULATION
    MCD_DEV_PTR         pDev;
    MCD_STATUS          rc;
    MCD_U32             host_or_line;
    MCD_U32             slice;
    MCD_U32             channel;
    MCD_U32             status;
    MCD_U32             channels_on_side;
    MCD_U32             side_chan_num;

    channels_on_side = (MCD_MAX_SLICE_NUM * MCD_MAX_CHANNEL_NUM_PER_SLICE);
    host_or_line     = ((serdesNum >= channels_on_side) ? MCD_HOST_SIDE : MCD_LINE_SIDE);
    side_chan_num    = (serdesNum % channels_on_side);
    slice            = (side_chan_num / MCD_MAX_CHANNEL_NUM_PER_SLICE);
    channel          = (side_chan_num % MCD_MAX_CHANNEL_NUM_PER_SLICE);

    pDev = mcdSampleGetDrvDev(phyIndex);
    if (pDev == NULL)
    {
        cpssOsPrintf("no device with given ID %d\n", phyIndex);
        return MCD_OK;
    }

    rc =  mcdCheckSerdesSignalStability(
        pDev, host_or_line, slice, channel,
        10 /*testTimeMs*/, &status);
    cpssOsPrintf("rc: %d status: %d\n", rc, status);
#endif
    return MCD_OK;
}

MCD_STATUS wr_mcd_lock_hw_semaphore
(
    unsigned int        phyIndex,
    unsigned int        semaphore_id,
    unsigned int        event_id
)
{
#ifndef ASIC_SIMULATION
    MCD_DEV_PTR         pDev;
    MCD_STATUS          rc;

    pDev = mcdSampleGetDrvDev(phyIndex);
    if (pDev == NULL)
    {
        cpssOsPrintf("no device with given ID %d\n", phyIndex);
        return MCD_OK;
    }

    rc =  mcdLockHwSemaphore(
        pDev, semaphore_id, event_id);
    cpssOsPrintf("rc: %d \n", rc);
#endif
    return MCD_OK;
}

MCD_STATUS wr_mcd_unlock_hw_semaphore
(
    unsigned int        phyIndex,
    unsigned int        semaphore_id
)
{
#ifndef ASIC_SIMULATION
    MCD_DEV_PTR         pDev;
    MCD_STATUS          rc;

    pDev = mcdSampleGetDrvDev(phyIndex);
    if (pDev == NULL)
    {
        cpssOsPrintf("no device with given ID %d\n", phyIndex);
        return MCD_OK;
    }

    rc =  mcdUnlockHwSemaphore(
        pDev, semaphore_id);
    cpssOsPrintf("rc: %d \n", rc);
#endif
    return MCD_OK;
}



