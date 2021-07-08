/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cpssDxChPortPacketBuffer.h
*
* DESCRIPTION:
*       CPSS DxCh Packet Buffer functions.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef __cpssDxChPortPacketBuffer_h
#define __cpssDxChPortPacketBuffer_h

#include <cpss/common/cpssTypes.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @enum CPSS_DXCH_PORT_PACKET_BUFFER_COUNTER_MEMORY_ENT
*
* @brief enum of Packet Buffer Counters Memory.
*/
typedef enum
{
    /** counted Cells allocated in Logical Memories of Shared Memory Buffers */
    CPSS_DXCH_PORT_PACKET_BUFFER_COUNTER_MEMORY_SMB_CELLS_E,

    /** counted Next Pointer Memory Entries allocated In Logical Memories of  Next Pointer Memory */
    CPSS_DXCH_PORT_PACKET_BUFFER_COUNTER_MEMORY_NPM_ENTRIES_E

} CPSS_DXCH_PORT_PACKET_BUFFER_COUNTER_MEMORY_ENT;

/**
* @enum CPSS_DXCH_PORT_PACKET_BUFFER_COUNTER_SCOPE_ENT
*
* @brief enum of Packet Buffer Scope.
*      Related to Set of logical memories and time of measuring values.
*/
typedef enum
{
    /** Snapshot value related to logical memories on all Tiles */
    CPSS_DXCH_PORT_PACKET_BUFFER_COUNTER_SCOPE_GLOBAL_E,

    /** Snapshot value related to logical memories on local Tile */
    CPSS_DXCH_PORT_PACKET_BUFFER_COUNTER_SCOPE_LOCAL_E,

    /** maximal value from the previous counter read
           related to logical memories on all Tiles (i.e. global) */
    CPSS_DXCH_PORT_PACKET_BUFFER_COUNTER_SCOPE_OVERTIME_MAX_E,

    /** minimal value from the previous counter read
          related to logical memories on all Tiles (i.e. global) */
    CPSS_DXCH_PORT_PACKET_BUFFER_COUNTER_SCOPE_OVERTIME_MIN_E

} CPSS_DXCH_PORT_PACKET_BUFFER_COUNTER_SCOPE_ENT;

/**
* @enum CPSS_DXCH_PORT_PACKET_BUFFER_COUNTER_VALUE_ENT
*
* @brief enum of Packet Buffer Counters Values.
*/
typedef enum
{
    /** maximal amount of any allocated memory units over all logical memories in scope.
           Related only to GLOBAL and LOCAL scopes. */
    CPSS_DXCH_PORT_PACKET_BUFFER_COUNTER_VALUE_MAX_E,

    /** minimal amount of any allocated memory units over all logical memories in scope.
           Related only to GLOBAL and LOCAL scopes. */
    CPSS_DXCH_PORT_PACKET_BUFFER_COUNTER_VALUE_MIN_E,

    /** difference between maximal and minimal amounts
           of any allocated memory units over all logical memories in scope
           Related only to OVERTIME scopes.*/
    CPSS_DXCH_PORT_PACKET_BUFFER_COUNTER_VALUE_DIFF_MAX_MIN_E,

    /** summary amount of any allocated memory units over all logical memories in scope.*/
    CPSS_DXCH_PORT_PACKET_BUFFER_COUNTER_VALUE_SUM_ALL_E,

    /** summary amount of multicast allocated memory units over all logical memories in scope. */
    CPSS_DXCH_PORT_PACKET_BUFFER_COUNTER_VALUE_SUM_MC_E,

    /** summary amount of unicast allocated memory units over all logical memories in scope. */
    CPSS_DXCH_PORT_PACKET_BUFFER_COUNTER_VALUE_SUM_UC_E

} CPSS_DXCH_PORT_PACKET_BUFFER_COUNTER_VALUE_ENT;

/**
* @internal cpssDxChPortPacketBufferFillCounterGet function
* @endinternal
*
* @brief Get memory fill-counter value.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in]  devNum         - device number
* @param[in]  tileIndex      - index of tile
*                              (APPLICABLE RANGES: 0..3)
* @param[in]  counterMemory  - (enum value) - SMB cells or NPM entries
* @param[in]  counterScope   - (enum value) - snapshot global or local or overtime-global
* @param[in]  counterValue   - (enum value) - MIN, MAX, SUM and others
*
* @param[out] receivedValuePtr   - (pointer to)counter value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS cpssDxChPortPacketBufferFillCounterGet
(
    IN   GT_U8                                           devNum,
    IN   GT_U32                                          tileIndex,
    IN   CPSS_DXCH_PORT_PACKET_BUFFER_COUNTER_MEMORY_ENT counterMemory,
    IN   CPSS_DXCH_PORT_PACKET_BUFFER_COUNTER_SCOPE_ENT  counterScope,
    IN   CPSS_DXCH_PORT_PACKET_BUFFER_COUNTER_VALUE_ENT  counterValue,
    OUT  GT_U32                                          *receivedValuePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChPortPacketBuffer_h */
