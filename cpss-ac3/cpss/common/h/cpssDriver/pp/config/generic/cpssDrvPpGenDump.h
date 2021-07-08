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
* @file cpssDrvPpGenDump.h
*
* @brief PP Registers and PP memory dump function headers
*
* @version   1
********************************************************************************
*/
#ifndef __cpssDrvPpGenDumph
#define __cpssDrvPpGenDumph

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/config/private/prvCpssConfigTypes.h>

/**
* @enum CPSS_MEMORY_DUMP_TYPE_ENT
 *
 * @brief This enumeration defines format of memory dump
*/
typedef enum{

    /** dump string will contain 16 byte elements (8 bits each) */
    CPSS_MEMORY_DUMP_BYTE_E,

    /** dump string will contain 8 short elements (16 bits each) */
    CPSS_MEMORY_DUMP_SHORT_E,

    /** dump string will contain 4 word elements (32 bits each) */
    CPSS_MEMORY_DUMP_WORD_E

} CPSS_MEMORY_DUMP_TYPE_ENT;


/**
* @internal cpssPpDumpRegisters function
* @endinternal
*
* @brief   This routine print dump of PP registers.
*
* @param[in] devNum                   - PP's device number .
*
* @retval GT_OK                    - on success (all Registers dumped successfully)
* @retval GT_FAIL                  - on failure. (at least one register read error)
* @retval GT_BAD_PARAM             - invalid device number
*/
GT_STATUS cpssPpDumpRegisters
(
    IN GT_U8                    devNum
);

/**
* @internal cpssPpDumpPortGroupRegisters function
* @endinternal
*
* @brief   This routine print dump of PP registers.
*
* @param[in] devNum                   - PP's device number .
* @param[in] portGroupId              - The port group Id.
*                                      relevant only to 'multi-port-groups' devices.
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*
* @retval GT_OK                    - on success (all Registers dumped successfully)
* @retval GT_FAIL                  - on failure. (at least one register read error)
* @retval GT_BAD_PARAM             - invalid device number
*/
GT_STATUS cpssPpDumpPortGroupRegisters
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId
);

/**
* @internal cpssPpDumpMemory function
* @endinternal
*
* @brief   This routine print dump of PP memory.
*
* @param[in] devNum                   - PP's device number .
* @param[in] startAddr                - dump start address (will be aligned to 16 bytes)
* @param[in] dumpType                 - dump line format (16 bytes / 8 shorts / or 4 words)
* @param[in] dumpLength               - dump length in bytes
*
* @retval GT_OK                    - on success (memory dumped successfully)
* @retval GT_FAIL                  - on failure. (memory read error)
* @retval GT_BAD_PARAM             - invalid device number
*
* @note 4 lower bits of startAddr will be zeroed (ignored) for alignment
*
*/
GT_STATUS cpssPpDumpMemory
(
    IN GT_U8                        devNum,
    IN GT_U32                       startAddr,
    IN CPSS_MEMORY_DUMP_TYPE_ENT    dumpType,
    IN GT_U32                       dumpLength
);

/**
* @internal cpssPpDumpPortGroupTables function
* @endinternal
*
* @brief   This routine print dump of PP tables.
*
* @param[in] devNum                   - PP's device number .
* @param[in] portGroupId              - The port group Id.
*                                      relevant only to 'multi-port-groups' devices.
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*
* @retval GT_OK                    - on success (all Registers dumped successfully)
* @retval GT_FAIL                  - on failure. (at least one register read error)
* @retval GT_BAD_PARAM             - invalid device number
*/
GT_STATUS cpssPpDumpPortGroupTables
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId
);

/**
* @internal cpssPpDumpCpuMemory function
* @endinternal
*
* @brief   This routine print dump of CPU memory.
*
* @param[in] startAddr                - dump start address (will be aligned to 16 bytes)
* @param[in] dumpLength               - dump length in bytes
* @param[in] dumpType                 - dump line format (16 bytes / 8 shorts / or 4 words)
* @param[in] wordsPerLine             - words Per Line
*
* @retval GT_OK                    - on success (memory dumped successfully)
* @retval GT_FAIL                  - on failure. (memory read error)
* @retval GT_BAD_PARAM             - invalid device number
*
* @note 4 lower bits of startAddr will be zeroed (ignored) for alignment
*
*/
GT_STATUS cpssPpDumpCpuMemory
(
    IN GT_U8*                       startAddr,
    IN GT_U32                       dumpLength,
    IN CPSS_MEMORY_DUMP_TYPE_ENT    dumpType,
    IN GT_U32                       wordsPerLine
);

/**
* @internal cpssPpPortGroupDumpMemory function
* @endinternal
*
* @brief   This routine print dump of PP memory.
*
* @param[in] devNum                   - PP's device number .
* @param[in] portGroupId              - The port group Id.
*                                      relevant only to 'multi-port-groups' devices.
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] startAddr                - dump start address (will be aligned to 16 bytes)
* @param[in] dumpType                 - dump line format (16 bytes / 8 shorts / or 4 words)
* @param[in] dumpLength               - dump length in bytes
*
* @retval GT_OK                    - on success (memory dumped successfully)
* @retval GT_FAIL                  - on failure. (memory read error)
* @retval GT_BAD_PARAM             - invalid device number
*
* @note 4 lower bits of startAddr will be zeroed (ignored) for alignment
*
*/
GT_STATUS cpssPpPortGroupDumpMemory
(
    IN GT_U8                        devNum,
    IN GT_U32                       portGroupId,
    IN GT_U32                       startAddr,
    IN CPSS_MEMORY_DUMP_TYPE_ENT    dumpType,
    IN GT_U32                       dumpLength
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDrvPpGenDumph */



