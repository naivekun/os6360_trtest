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
* @file wrapCpssPxPortMapping.c
*
* @brief Wrapper functions for
* cpss/px/port/cpssPxPortMapping.h API's
*
* @version   1
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>
#include <cpss/px/port/cpssPxPortMapping.h>

static CPSS_PX_PORT_MAP_STC     portMapArray[17];
static GT_U32                   portMapNum;
static GT_U32                   currPortMap;
static GT_U32                   numEntriesReturned = 0;/* number of entries that the 'refresh' already returned to galtis terminal */

/* Feature specific includes. */
#include <cpss/px/port/cpssPxPortMapping.h>

/**
* @internal internalPortPhysicalPortMapSet function
* @endinternal
*
* @brief   configure ports mapping
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. All usable ports shall be configured at ONCE. Any port, that was mapped previously and is
*       not included into current mapping, losses its mapping.
*       2. Changing mapping under traffic may cause to traffic interruption.
*       3. The function checks whether all mapping are consistent.
*       In case of contradiction between mappings
*       (e,g. single TxQ port is mapped to several Tx DMA ports) the function returns with BAD_PARAM.
*       5. Other CPSS APIs that receive GT_PHYSICAL_PORT_NUM as input parameter will use this mapping
*       to convert the physical port to mac/dma/txq to configure them
*       6. Usage example
*       CPSS_PX_PORT_MAP_STC exampleMap[] =
*       (
*       -- the list for 'network ports' : 10G each
*       ( 0, _INTRFACE_TYPE_ETHERNET_MAC_E,   0, 0),
*       ( 1, _INTRFACE_TYPE_ETHERNET_MAC_E,   1, 1),
*       ...
*       ( 11, _INTRFACE_TYPE_ETHERNET_MAC_E,   11, 11),
*       -- the 'uplink port' : 100G
*       ( 12, _INTRFACE_TYPE_ETHERNET_MAC_E,   12, 12),
*       -- the SDMA CPU
*       ( 16, _INTRFACE_TYPE_CPU_SDMA_E,   GT_NA, 16),
*       );
*       rc = cpssPxCfgPhysicalPortMapSet(devNum, sizeof(exampleMap)/sizeof(exampleMap[0]), &exampleMap[0]);
*       if(rc != GT_OK)
*       (
*       fatal error.
*       )
*
*/
static CMD_STATUS internalPortPhysicalPortMapSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER],
    IN GT_U32   opCode
)
{
    GT_STATUS                               result;
    GT_SW_DEV_NUM                           devNum;
    GT_PHYSICAL_PORT_NUM                    port;
    GT_U32                                  portMapArraySize;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if(opCode == 0)/*start registration*/
    {
        portMapNum = 0;
    }
    else if(opCode == 0xFFFFFFFF)/*end registration*/
    {
        /* map input arguments to locals */
        devNum = (GT_SW_DEV_NUM)inArgs[0];

        /* Dummy port - for device override */
        port = portMapArray[0].physicalPortNumber;

        /* Override Device and Port */
        CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, port);

        portMapArraySize = portMapNum;

        result = cpssPxPortPhysicalPortMapSet(devNum, portMapArraySize, portMapArray);
        if(result != GT_OK)
        {
            galtisOutput(outArgs, result, "%d",-1);/* Error ! */
            return CMD_OK;
        }
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, GT_OK, "");
        return CMD_OK;
    }

    if (portMapNum > 17)
    {
        galtisOutput(outArgs, GT_FULL, "%d",-1);/* Error ! */
        return CMD_OK;
    }

    port = (GT_PHYSICAL_PORT_NUM)inFields[0];
    devNum = (GT_SW_DEV_NUM)inArgs[0];

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, port);

    portMapArray[portMapNum].physicalPortNumber = port;
    portMapArray[portMapNum].mappingType = (CPSS_PX_PORT_MAPPING_TYPE_ENT)inFields[1];
    portMapArray[portMapNum].interfaceNum = (GT_U32)inFields[2];
    portMapNum++;

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "");

    return CMD_OK;
}

static CMD_STATUS wrCpssPxPortPhysicalPortMapSetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return internalPortPhysicalPortMapSet(inArgs,inFields,numFields,outArgs,0);
}

static CMD_STATUS wrCpssPxPortPhysicalPortMapSetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return internalPortPhysicalPortMapSet(inArgs,inFields,numFields,outArgs,1);
}

static CMD_STATUS wrCpssPxPortPhysicalPortMapEndSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return internalPortPhysicalPortMapSet(inArgs,inFields,numFields,outArgs,0xFFFFFFFF);
}

/**
* @internal wrCpssPxPortPhysicalPortMapGetNext function
* @endinternal
*
* @brief   get ports mapping
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_FAIL                  - when cannot deliver requested number of items
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortPhysicalPortMapGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32          portMapArraySize;
    GT_STATUS       result;
    GT_STATUS       result1;
    GT_SW_DEV_NUM   devNum;
    GT_BOOL         isValid;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum = (GT_SW_DEV_NUM)inArgs[0];
    portMapArraySize = (GT_U32)inArgs[2];

    while(1)
    {
        if((numEntriesReturned >= portMapArraySize)/* we returned the max needed */
            ||
           (currPortMap >= 17))                   /* we checked all 256 possible ports */
        {
            galtisOutput(outArgs, GT_OK, "%d", -1);
            return CMD_OK;
        }

        result = cpssPxPortPhysicalPortMapGet(devNum, currPortMap, 1, portMapArray);
        result1 = cpssPxPortPhysicalPortMapIsValidGet(devNum, currPortMap, &isValid);

        currPortMap++;

        if(isValid != GT_TRUE)
        {
            continue;
        }

        if((result != GT_OK) || (result1 != GT_OK))
        {
            continue;
        }
        else
        {
            break;
        }
    }

    numEntriesReturned++;/* we returned another entry */

    inFields[0] = portMapArray[0].physicalPortNumber;
    inFields[1] = portMapArray[0].mappingType;
    inFields[2] = portMapArray[0].interfaceNum;

    /* pack and output table fields */
    fieldOutput("%d%d%d", inFields[0], inFields[1], inFields[2]);

    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}

/**
* @internal wrCpssPxPortPhysicalPortMapGetFirst function
* @endinternal
*
* @brief   get ports mapping
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_FAIL                  - when cannot deliver requested number of items
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssPxPortPhysicalPortMapGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_SW_DEV_NUM   devNum;
    GT_U32          firstPhysicalPortNumber;
    GT_U32          portMapArraySize;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    numEntriesReturned = 0;/* state that no entry was returned */

    /* map input arguments to locals */
    devNum = (GT_SW_DEV_NUM)inArgs[0];
    firstPhysicalPortNumber = (GT_U32)inArgs[1];
    portMapArraySize = (GT_U32)inArgs[2];

    if(portMapArraySize == 0)
    {
        galtisOutput(outArgs, GT_BAD_PARAM, "%s", "portMapArraySize==0");
        return CMD_OK;
    }
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, firstPhysicalPortNumber);

    currPortMap = firstPhysicalPortNumber;

    return wrCpssPxPortPhysicalPortMapGetNext(inArgs,inFields,numFields,outArgs);
}

/**
* @internal wrCpssPxPortPhysicalPortMapUpdateOrAdd function
* @endinternal
*
* @brief   The function add new physicalPortNumber mapping or update existing physicalPortNumber mapping.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssPxPortPhysicalPortMapUpdateOrAdd
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_STATUS       result1;
    GT_SW_DEV_NUM   devNum;
    GT_BOOL         isValid;
    CPSS_PX_PORT_MAP_STC  newInfo;
    GT_U32      index;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    index = 0;

    devNum                      = (GT_SW_DEV_NUM)inArgs[index++];  /*0*/
    newInfo.physicalPortNumber  = (GT_PHYSICAL_PORT_NUM)inArgs[index++];  /*1*/
    newInfo.mappingType         = (CPSS_PX_PORT_MAPPING_TYPE_ENT) inArgs[index++];  /*2*/
    newInfo.interfaceNum        = (GT_U32)inArgs[index++];  /*3*/

    /* Override Device and Port */
    CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, newInfo.physicalPortNumber);

    currPortMap = 0;
    index = 0;

    while((currPortMap < 17) && (index < 17))
    {
        result = cpssPxPortPhysicalPortMapGet(devNum, currPortMap, 1, &portMapArray[index]);
        result1 = cpssPxPortPhysicalPortMapIsValidGet(devNum, currPortMap, &isValid);

        currPortMap++;

        if(isValid != GT_TRUE)
        {
            continue;
        }
        else
        if((result != GT_OK) || (result1 != GT_OK))
        {
            continue;
        }

        if((currPortMap - 1) == newInfo.physicalPortNumber)
        {
            /* ignore the entry as we going to override it ...but we will put it
                as last entry in the array ... to unify code with 'new port' */
        }
        else
        {
            index++;/* valid entry */
        }
    }

    if(index >= 17)
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, GT_FULL, "");
        return CMD_OK;
    }

    /* To delete port from mapping use CPSS_DXCH_PORT_MAPPING_TYPE_MAX_E */
    if (newInfo.mappingType != CPSS_PX_PORT_MAPPING_TYPE_MAX_E)
    {
        /* add new port / update new port as the 'last entry' */
        portMapArray[index] = newInfo;
        index ++;
    }

    result = cpssPxPortPhysicalPortMapSet(devNum, index, portMapArray);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;

}

/**** database initialization **************************************/
static CMD_COMMAND dbCommands[] =
{
    /* commandName                                   funcReference                  funcArgs  funcFields */
    {"cpssPxPortPhysicalPortMapSetFirst",
        &wrCpssPxPortPhysicalPortMapSetFirst,
        3, 3},

    {"cpssPxPortPhysicalPortMapSetNext",
        &wrCpssPxPortPhysicalPortMapSetNext,
        3, 3},

    {"cpssPxPortPhysicalPortMapEndSet",
        &wrCpssPxPortPhysicalPortMapEndSet,
        3, 0},

    {"cpssPxPortPhysicalPortMapGetFirst",
        &wrCpssPxPortPhysicalPortMapGetFirst,
        3, 0},

    {"cpssPxPortPhysicalPortMapGetNext",
        &wrCpssPxPortPhysicalPortMapGetNext,
        3, 0},

    {"cpssPxPortPhysicalPortMapUpdateOrAdd",
        &wrCpssPxPortPhysicalPortMapUpdateOrAdd,
        4, 0}
};


#define  numCommands     (sizeof(dbCommands) / sizeof(CMD_COMMAND))


/**
* @internal cmdLibInitCpssPxPortMapping function
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
GT_STATUS cmdLibInitCpssPxPortMapping
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}

