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
* @file cpssDxChVirtualTcamUT.h
*
* @brief The mainUt of CPSS DXCH High Level Virtual TCAM Manager
*
* @version   1
********************************************************************************
*/

#ifndef __cpssDxChVirtualTcamUTh
#define __cpssDxChVirtualTcamUTh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* get CPSS Common Driver facility types definitions */
#include <cpss/common/cpssTypes.h>

/* get CPSS definitions for port configurations */
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/cpssDxChVirtualTcam.h>

typedef enum
{
    VT_CREATE,
    VT_REMOVE,
    VT_COMPLETE,
    VT_INCREASE,
    VT_DECREASE,
    VT_NO_MORE,
    VT_STAMP_FILL,
    VT_STAMP_CHECK,
    VT_STAMP_RANGE_CHECK,
    VT_TITLE
} VT_OPERATION;

typedef struct
{
    VT_OPERATION operation;
    GT_U32    vTcamId;
    enum      {CG_PCL = 0, CG_TTI = 1, CG_NONE = 0xFF} clientGroup;
    enum
    {
        RS_10 = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_10_B_E,
        RS_20 = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_20_B_E,
        RS_30 = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E,
        RS_40 = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_40_B_E,
        RS_50 = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_50_B_E,
        RS_60 = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E,
        RS_80 = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E,
        RS_NONE = 0xFF
    } ruleSize;
    GT_U32    rulesAmount;
    GT_STATUS rc;
    char*     title; /* relevant only to VT_TITLE */
} VT_SEQUENCE;

typedef struct
{
    VT_OPERATION operation;     /* Either VT_INCREASE or VT_DECREASE */
    GT_U32    vTcamId;
    GT_U32    vTcamIdIdx;       /* Index of corresponding entry (for same vTcamId) at VT_SEQUENCE */
    GT_U32    rulesAmount;      /* Amount to increase or decrease */
    GT_U32    rulesPlace;       /* Where to place the newly increased rules / where to start removing the dereased rules */
    GT_STATUS rc;               /* Expected return code */
} VT_INCREASE_DECREASE_SEQUENCE;

GT_STATUS vtcamSequenceEntryProcess(
    INOUT VT_SEQUENCE *sequencePtr,
    IN GT_U32 idx,
    IN GT_U32 vTcamMngId,
    INOUT CPSS_DXCH_VIRTUAL_TCAM_INFO_STC  *vTcamInfoPtr
);

/* indication for deleteConsecutiveRules()  to delete entries from the end */
#define DELETE_RULE_ID_FROM_END_OF_TABLE_CNS   (CPSS_DXCH_VIRTUAL_TCAM_START_RULE_ID_CNS - 1)
/* indication for deleteConsecutiveRules()  to delete entries from the middle */
#define DELETE_RULE_ID_FROM_MIDDLE_OF_TABLE_CNS   (CPSS_DXCH_VIRTUAL_TCAM_START_RULE_ID_CNS - 2)
#define DELETE_ALL_CNS  0xFFFFFFFF
/* function to delete Consecutive Rules from specific ruleId */
void vTcamDeleteConsecutiveRules(
    IN GT_U32   vTcamMngId,
    IN GT_U32   vTcamId,
    IN GT_U32   startRuleId,
    IN GT_U32   numRulesToRemove
);

/* Generic function to determine TCAM size in a device */
void internal_ut_cpssDxChVirtualTcamTotalRulesAllowedPerDevGet(
    IN  GT_U32                               devNum,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT ruleSize,
    OUT GT_U32                               *ptrTotalRulesAllowed
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChVirtualTcamUTh */

