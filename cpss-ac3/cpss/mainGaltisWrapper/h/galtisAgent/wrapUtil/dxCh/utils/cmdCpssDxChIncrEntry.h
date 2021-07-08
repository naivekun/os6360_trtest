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
* @file cmdCpssDxChIncrEntry.h
*
* @brief CPSS Utils for create incremented Entries sequences
*
* @version   11
********************************************************************************
*/
#ifndef __cmdCpssDxChIncrEntryh
#define __cmdCpssDxChIncrEntryh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>

/* Feature specific includes. */
#include <cpss/dxCh/dxChxGen/oam/cpssDxChOam.h>
#include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>

/**
* @internal cmdCpssDxChGenIntrfaceInfoIncrement function
* @endinternal
*
* @brief   Inrement the given structure.
*
* @param[in,out] basePtr                  - (pointer to)the structure with base values.
* @param[in] incPtr                   - (pointer to)the structure with increment values.
* @param[in,out] basePtr                  - (pointer to)the structure with result values.
*
* @retval GT_OK                    - on sucess, GT_FAIL otherwise.
*/
GT_STATUS cmdCpssDxChGenIntrfaceInfoIncrement
(
    INOUT CPSS_INTERFACE_INFO_STC *basePtr,
    IN    CPSS_INTERFACE_INFO_STC *incPtr
);

/**
* @internal cmdCpssDxChOamEntryIncrement function
* @endinternal
*
* @brief   Inrement the given structure.
*
* @param[in,out] basePtr                  - (pointer to)the structure with base values.
* @param[in] incPtr                   - (pointer to)the structure with increment values.
* @param[in,out] basePtr                  - (pointer to)the structure with result values.
*
* @retval GT_OK                    - on sucess, GT_FAIL otherwise.
*/
GT_STATUS cmdCpssDxChOamEntryIncrement
(
    INOUT CPSS_DXCH_OAM_ENTRY_STC *basePtr,
    IN    CPSS_DXCH_OAM_ENTRY_STC *incPtr
);

/**
* @internal cmdCpssDxChOamEntrySequenceWrite function
* @endinternal
*
* @brief   Write sequence of incremented OAM Entries.
*
* @param[in] devNum                   - device number
* @param[in] stage                    - OAM  type.
* @param[in] baseEntryIndex           - index of entry to be retrieved and used as base values.
*                                      this entry also will be start entry of the sequence
* @param[in] incrEntryIndex           - index of entry to be retrieved and used as increment values.
*                                      retieved at the beginnig and can be overriden by sequence.
* @param[in] amountToWrite            - amount of enries to write
* @param[in] writeIndexIncr           - signed increment of sequence entry index
*
* @retval GT_OK                    - on sucess, GT_FAIL otherwise.
*/
GT_STATUS cmdCpssDxChOamEntrySequenceWrite
(
    IN   GT_U8                              devNum,
    IN   CPSS_DXCH_OAM_STAGE_TYPE_ENT       stage,
    IN   GT_U32                             baseEntryIndex,
    IN   GT_U32                             incrEntryIndex,
    IN   GT_U32                             amountToWrite,
    IN   GT_32                              writeIndexIncr
);

/**
* @internal cmdCpssDxChPclRuleIncrement function
* @endinternal
*
* @brief   Increment all supported fields in the given structure.
*
* @param[in] ruleFormat               - Rule Format
* @param[in,out] basePtr                  - (pointer to)the structure with base values.
* @param[in] incrPtr                  - (pointer to)the structure with increment values.
* @param[in,out] basePtr                  - (pointer to)the structure with result values.
*
* @retval GT_OK                    - on sucess, GT_FAIL otherwise.
*/
GT_STATUS cmdCpssDxChPclRuleIncrement
(
    IN    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT        ruleFormat,
    INOUT CPSS_DXCH_PCL_RULE_FORMAT_UNT             *basePtr,
    IN    CPSS_DXCH_PCL_RULE_FORMAT_UNT             *incrPtr
);

/**
* @internal cmdCpssDxChPclActionIncrement function
* @endinternal
*
* @brief   Increment all supported fields in the given structure.
*
* @param[in,out] basePtr                  - (pointer to)the structure with result values.
*
* @retval GT_OK                    - on sucess, GT_FAIL otherwise.
*/
GT_STATUS cmdCpssDxChPclActionIncrement
(
    INOUT CPSS_DXCH_PCL_ACTION_STC             *basePtr,
    IN    CPSS_DXCH_PCL_ACTION_STC             *incrPtr
);

/**
* @internal cmdCpssDxChPclRuleSequenceWrite function
* @endinternal
*
* @brief   Write sequence of incremented PclRules.
*         The rule mask copied unchanged, pattern and action incremented.
* @param[in] devNum                   - device number
* @param[in] ruleFormat               - format of all related rules.
* @param[in] baseRuleIndex            - index of rule to be retrieved and used as base values.
*                                      this entry also will be start rule of the sequence
* @param[in] incrRuleIndex            - index of rule to be retrieved and used as increment values.
*                                      retieved at the beginnig and can be overriden by sequence.
* @param[in] amountToWrite            - amount of rules to write
* @param[in] writeIndexIncr           - signed increment of sequence rule index
*
* @retval GT_OK                    - on sucess, other on error.
*/
GT_STATUS cmdCpssDxChPclRuleSequenceWrite
(
    IN   GT_U8                              devNum,
    IN   CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN   GT_U32                             baseRuleIndex,
    IN   GT_U32                             incrRuleIndex,
    IN   GT_U32                             amountToWrite,
    IN   GT_32                              writeIndexIncr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cmdCpssDxChIncrEntryh */



