/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* wrapAsicSim.h
*
* DESCRIPTION:
*       This file implements wrapper functions for the 'Hardware' library
*
* FILE REVISION NUMBER:
*       $Revision: 7 $
*
*******************************************************************************/
#ifndef __WrapAsicSimh
#define __WrapAsicSimh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cmdShell/common/cmdCommon.h>


/**
* @internal cmdLibInitAsicSim function
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
GT_STATUS cmdLibInitAsicSim();

/**
* @internal wrScibWriteRegister function
* @endinternal
*
* @brief   Write data to register in the simulation
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
*
* @note none
*
*/
CMD_STATUS wrScibWriteRegister
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
);

/**
* @internal wrScibReadRegister function
* @endinternal
*
* @brief   Reads the unmasked bits of a register.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
*
* @note none
*
*/
CMD_STATUS wrScibReadRegister
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
);

/**
* @internal wrScibWriteMemory function
* @endinternal
*
* @brief   Write data to memory in the simulation. Up to 8 words may be written
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
*
* @note none
*
*/
CMD_STATUS wrScibWriteMemory
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
);

/**
* @internal wrScibReadMemory function
* @endinternal
*
* @brief   Read data from memory in the simulation. Up to 8 words may be read.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
*
* @note none
*
*/
CMD_STATUS wrScibReadMemory
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
);


CMD_STATUS wrIndirectTblMemoryTest
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
);

CMD_STATUS wrSearchMemoryTest
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
);

/**
* @internal wrSkernelPolicerConformanceLevelForce function
* @endinternal
*
* @brief   force the conformance level for the packets entering the policer
*         (traffic cond)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*
* @note none
*
*/
CMD_STATUS wrSkernelPolicerConformanceLevelForce
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
);

/**
* @internal wrCpssAsicSimRtosOnSimRemoteConnectionParametersSet function
* @endinternal
*
* @brief   set RTOS on simulation parameters , this emulate the INI file of simulation
*
* @param[in] inArgs[CMD_MAX_ARGS]     - simulation IP - the "server side" (Note: currently the PP side is
*                                      the "server side" , it will be changed ... , and then this
*                                      parameter will be meaningless)
*                                      format is "10.6.150.92"
* @param[in] inArgs[CMD_MAX_ARGS]     - tcp_port_synchronic - the TCP port of the "synchronic connection"
* @param[in] inArgs[CMD_MAX_ARGS]     - tcp_port_asynchronous - the TCP port of the "asynchronous connection"
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*
* @note none
*
*/
CMD_STATUS wrCpssAsicSimRtosOnSimRemoteConnectionParametersSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
);

/**
* @internal wrCpssAsicSimRtosOnSimIniFileEmulateFill function
* @endinternal
*
* @brief   set RTOS on simulation parameters , this emulate the INI file of simulation
*
* @param[in] inArgs[CMD_MAX_ARGS]     - chapter name (string)
* @param[in] inArgs[CMD_MAX_ARGS]     - value name (string)
* @param[in] inArgs[CMD_MAX_ARGS]     - value (string)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*
* @note none
*
*/
CMD_STATUS wrCpssAsicSimRtosOnSimIniFileEmulateFill
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __WrapAsicSimh */



