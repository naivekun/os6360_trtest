/*******************************************************************************
*              (c), Copyright 2017, Marvell International Ltd.                 *
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
* @file wrapCpssPxDiagPacketGenerator.c
*
* @brief Wrapper functions for
* cpss/px/diag/cpssPxDiagPacketGenerator.h API's
*
* @version   1
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>

/* Feature specific includes. */
#include <cpss/px/diag/cpssPxDiagPacketGenerator.h>


/**
* @internal wrCpssPxDiagPacketGeneratorConnectSet function
* @endinternal
*
* @brief   Connect/Disconnect port to packet generator. Set packet generator's
*         configurations.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported port
*
* @note 1. In case packet generator connected to other port the function perfroms
*       the following:
*       - stops traffic.
*       - connects packet generator to new port
*       - overrides packet generator's configurations
*       2. Before enabling the packet generator, port must be initialized,
*       i.e. port interface and speed must be set.
*
*/
CMD_STATUS wrCpssPxDiagPacketGeneratorConnectSet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_SW_DEV_NUM                       devNum;
    GT_PHYSICAL_PORT_NUM                portNum;
    GT_BOOL                             connect;
    CPSS_PX_DIAG_PG_CONFIGURATIONS_STC  config;
    GT_BYTE_ARRY                        cyclicBArr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inFields[0];

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
    cmdOsMemCpy(config.cyclicPatternArr, cyclicBArr.data, 64);
    config.packetLengthType = (CPSS_DIAG_PG_PACKET_LENGTH_TYPE_ENT)inFields[12];
    config.packetLength = (GT_U32)inFields[13];
    config.undersizeEnable = (GT_BOOL)inFields[14];
    config.transmitMode = (CPSS_DIAG_PG_TRANSMIT_MODE_ENT)inFields[15];
    config.packetCount = (GT_U32)inFields[16];
    config.packetCountMultiplier = (CPSS_DIAG_PG_PACKET_COUNT_MULTIPLIER_ENT)inFields[17];
    config.ifg = (GT_U32)inFields[18];
    config.interfaceSize = CPSS_DIAG_PG_IF_SIZE_DEFAULT_E;

    /* override device and port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxDiagPacketGeneratorConnectSet(devNum, portNum, connect, &config);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

static GT_U32   wrPxPortNum = 0;
/**
* @internal wrCpssPxDiagPacketGeneratorConnectGetNext function
* @endinternal
*
* @brief   Get the connect status of specified port. Get packet generator's
*         configurations.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxDiagPacketGeneratorConnectGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM                         devNum;
    GT_BOOL                               connect;
    CPSS_PX_DIAG_PG_CONFIGURATIONS_STC    config;
    GT_STATUS                             result = GT_OK;
    GT_BOOL                               showOnlyConnected;
    GT_PHYSICAL_PORT_NUM                  tempPortNum;

    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cpssOsMemSet(&config, 0, sizeof(CPSS_PX_DIAG_PG_CONFIGURATIONS_STC));

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    showOnlyConnected = (GT_BOOL)inArgs[1];

    /* add port conversion*/
    while(1)
    {
        if(wrPxPortNum == PRV_CPSS_PX_PORTS_NUM_CNS)
        {
            /* no ports */
            galtisOutput(outArgs, GT_OK, "%d", -1);
            return CMD_OK;
        }

        if (!PRV_CPSS_IS_DEV_EXISTS_MAC(devNum) || 
            (PRV_CPSS_PHY_PORT_IS_EXIST_MAC(devNum, wrPxPortNum) &&
             PRV_CPSS_PX_PP_MAC(devNum)->port.portsMapInfoShadowArr[wrPxPortNum].valid == GT_TRUE))
        {
            tempPortNum = (GT_PHYSICAL_PORT_NUM)wrPxPortNum;

            /* Override device and port */
            CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, tempPortNum);


            /* get port's attributes */
            result = cpssPxDiagPacketGeneratorConnectGet(devNum,tempPortNum,&connect,&config);
            if ((result == GT_NOT_INITIALIZED) || ((showOnlyConnected == GT_TRUE) && (connect == GT_FALSE)))
            {
                wrPxPortNum++;
                continue;
            }
            break;
        }
        else
        {
            wrPxPortNum++;
        }
    }

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    inFields[0] = wrPxPortNum;
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
    inFields[18] = config.ifg;

    /* pack and output table fields */
    fieldOutput("%d%d%6b%d%6b%d%d%d%d%d%d%64b%d%d%d%d%d%d%d",
                          inFields[0],  inFields[1],  config.macDa.arEther,  inFields[3],
                          config.macSa.arEther, inFields[5], inFields[6],
                          inFields[7],  inFields[8],  inFields[9],
                          inFields[10], config.cyclicPatternArr, inFields[12],
                          inFields[13], inFields[14], inFields[15],
                          inFields[16], inFields[17], inFields[18]);
    galtisOutput(outArgs, GT_OK, "%f");

    /* move to the next portNum */
    wrPxPortNum++;
    return CMD_OK;

}

static CMD_STATUS wrCpssPxDiagPacketGeneratorConnectGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    wrPxPortNum = 0;
    return wrCpssPxDiagPacketGeneratorConnectGetNext(inArgs,inFields,numFields,outArgs);

}

/**
* @internal wrCpssPxDiagPacketGeneratorTransmitEnable function
* @endinternal
*
* @brief   Enable/Disable(Start/Stop) transmission on specified port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_STATE             - on port not connected to packet generator
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported port
*
* @note 1. Before calling this function the port must be connected to packet
*       generator (cpssPxDiagPacketGeneratorConnectSet),
*       otherwise GT_BAD_STATE is returned.
*
*/
CMD_STATUS wrCpssPxDiagPacketGeneratorTransmitEnable
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_SW_DEV_NUM           devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_BOOL                 enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum   = (GT_SW_DEV_NUM) inArgs[0];
    portNum  = (GT_PHYSICAL_PORT_NUM) inArgs[1];
    enable   = (GT_BOOL) inArgs[2];

    /* override device and port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxDiagPacketGeneratorTransmitEnable(devNum, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssPxDiagPacketGeneratorBurstTransmitStatusGet function
* @endinternal
*
* @brief   Get burst transmission status on specified port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
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
* @note 1. Burst transmission status is clear on read.
*       2. Before calling this function the port must be connected to packet
*       generator (cpssPxDiagPacketGeneratorConnectSet),
*       otherwise GT_BAD_STATE is returned.
*
*/
CMD_STATUS wrCpssPxDiagPacketGeneratorBurstTransmitStatusGet
(
    IN  GT_UINTPTR  inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR  inFields[CMD_MAX_FIELDS],
    IN  GT_32       numFields,
    OUT GT_8        outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_SW_DEV_NUM           devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_BOOL                 burstTransmitDone;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum              = (GT_SW_DEV_NUM) inArgs[0];
    portNum             = (GT_PHYSICAL_PORT_NUM) inArgs[1];

    /* override device and port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssPxDiagPacketGeneratorBurstTransmitStatusGet(devNum, portNum, &burstTransmitDone);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", burstTransmitDone);

    return CMD_OK;
}


/**** database initialization **************************************/
static CMD_COMMAND dbCommands[] =
{
    /*
        commandName,
        funcReference,
        funcArgs, funcFields
    */
    {"cpssPxDiagPacketGeneratorConnectSet",
        &wrCpssPxDiagPacketGeneratorConnectSet,
        2, 19},
    {"cpssPxDiagPacketGeneratorConnectGetFirst",
        &wrCpssPxDiagPacketGeneratorConnectGetFirst,
        2, 0},
    {"cpssPxDiagPacketGeneratorConnectGetNext",
        &wrCpssPxDiagPacketGeneratorConnectGetNext,
        2, 0},
    {"cpssPxDiagPacketGeneratorTransmitEnable",
        &wrCpssPxDiagPacketGeneratorTransmitEnable,
        3, 0},
    {"cpssPxDiagPacketGeneratorBurstTransmitStatusGet",
        &wrCpssPxDiagPacketGeneratorBurstTransmitStatusGet,
        2, 0}
};


#define  numCommands     (sizeof(dbCommands) / sizeof(CMD_COMMAND))


/**
* @internal cmdLibInitCpssPxDiagPacketGenerator function
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
GT_STATUS cmdLibInitCpssPxDiagPacketGenerator
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}

