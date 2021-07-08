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
* @file prvCpssDxChVirtualTcamSegments.c
*
* @brief The CPSS DXCH High Level Virtual TCAM Manager - segments DB
*
* @version   1
********************************************************************************
*/
#include <cpss/dxCh/dxChxGen/virtualTcam/cpssDxChVirtualTcam.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/private/prvCpssDxChVirtualTcam.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/private/prvCpssDxChVirtualTcamDb.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>

    /* Two bits for Hit num, three bits for Client group. */
#define PRV_CPSS_DXCH_VIRTUAL_TCAM_MAX_HITNUM       4
#define PRV_CPSS_DXCH_VIRTUAL_TCAM_MAX_CLIENTGROUP  8

/**********************************************************************************/
/* segments Table (the alone feature for physical layout)                      */
/**********************************************************************************/

/* Debug trace support */
GT_BOOL prvCpssDxChVirtualTcamDebugTraceEnable = GT_FALSE;

void prvCpssDxChVirtualTcamDebugTraceEnableSet(GT_BOOL enable)
{
    prvCpssDxChVirtualTcamDebugTraceEnable = enable;
}

#define PRV_DBG_TRACE(_x)                                               \
    if (prvCpssDxChVirtualTcamDebugTraceEnable != GT_FALSE)             \
        {cpssOsPrintf("TRACE: "); cpssOsPrintf _x; cpssOsPrintf("\n");}

#define PRV_DUMP_SEG(_s) \
    if (prvCpssDxChVirtualTcamDebugTraceEnable != GT_FALSE)             \
    {                                                                   \
        cpssOsPrintf("SEG:0x%08X(%3.3d),Blocks use, %s  %s \n",         \
        _s->rowsBase, _s->rowsAmount,                                   \
        prvCpssDxChVirtualTcamDbSegmentColumnsEnumNames[                \
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(  \
                _s->segmentColumnsMap,0)],                              \
        prvCpssDxChVirtualTcamDbSegmentColumnsEnumNames[                \
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(  \
                _s->segmentColumnsMap,1)]);                             \
    }

/* exception debug support */
GT_U32 prvCpssDxChVirtualTcamDbSegmentTableExceptionCounter = 0;

void prvCpssDxChVirtualTcamDbSegmentTableException(void)
{
    prvCpssDxChVirtualTcamDbSegmentTableExceptionCounter ++;
    if (prvCpssDxChVirtualTcamDebugTraceEnable != GT_FALSE)
    {
        cpssOsPrintf("***** Segment Table Exception ******\n");
    }
}

void prvCpssDxChVirtualTcamDbSegmentTableExceptionReset(void)
{
    prvCpssDxChVirtualTcamDbSegmentTableExceptionCounter ++;
}

void prvCpssDxChVirtualTcamDbSegmentTableExceptionPrint(void)
{
    cpssOsPrintf(
        "Exceprion counter: %d\n",
         prvCpssDxChVirtualTcamDbSegmentTableExceptionCounter);
}

/* this function only for debugging purposes */

PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_CHAR_PTR
    prvCpssDxChVirtualTcamDbSegmentColumnsEnumNames[
        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LAST_E] =
{
    "NONE    ",
    "COL 5   ",
    "COL 0,1 ",
    "COL 4,5 ",
    "COL 0..3",
    "COL 2..5",
    "COL 0..4",
    "COL ALL ",
    "LEGACY 3   ",
    "LEGACY 0..2",
    "LEGACY ALL ",
    "LEG_TTI ALL"
} ;

void prvCpssDxChVirtualTcamDbSegmentTableDumpTree
(
    IN     PRV_CPSS_AVL_TREE_ID  treeId
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segPtr1; /* segment  */
    GT_BOOL                 seeked;      /* node seeked                   */
    PRV_CPSS_AVL_TREE_PATH  path;        /* tree iterator                 */
    GT_VOID_PTR             dbEntryPtr;  /* pointer to entry in DB        */

    if (prvCpssDxChVirtualTcamDebugTraceEnable == GT_FALSE)
    {
        return;
    }

    cpssOsPrintf("SEG Tree Dump\n");

    seeked = prvCpssAvlPathSeek(
            treeId, PRV_CPSS_AVL_TREE_SEEK_FIRST_E,
            path, &dbEntryPtr /* use dedicated var to avoid warnings */);
    segPtr1 = dbEntryPtr;
    while (seeked != GT_FALSE)
    {
        /* print */
        cpssOsPrintf(
            "0x%08X(%3.3d) columnsMap 0x%08X lookup %04X vTCAM %04X\n",
            segPtr1->rowsBase, segPtr1->rowsAmount,
            segPtr1->segmentColumnsMap,
            segPtr1->lookupId, segPtr1->vTcamId);
        /* next segment */
        seeked = prvCpssAvlPathSeek(
                treeId, PRV_CPSS_AVL_TREE_SEEK_NEXT_E,
                path, &dbEntryPtr /* use dedicated var to avoid warnings */);
        segPtr1 = dbEntryPtr;
    }
}

/* prvCpssDxChVirtualTcamDbSegmentTablePrimaryTreeMergeAndSplitStep trace */

GT_BOOL prvCpssDxChVirtualTcamDbSegmentTablePrimaryTreeMrgSplTraceEnable = GT_FALSE;

GT_VOID prvCpssDxChVirtualTcamDbSegmentTablePrimaryTreeMrgSplTraceEnableSet(GT_BOOL enable)
{
    prvCpssDxChVirtualTcamDbSegmentTablePrimaryTreeMrgSplTraceEnable = enable;
}

#define PRV_PRIMARY_TREE_MRG_SPLIT_TRACE_MAC(x) \
    if (prvCpssDxChVirtualTcamDbSegmentTablePrimaryTreeMrgSplTraceEnable != GT_FALSE) \
        {cpssOsPrintf("\n*** PrimaryTreeMerge&Split ***\n"); cpssOsPrintf x; cpssOsPrintf("\n");}

/* debugging mechanism for prvCpssAvlItemInsert and prvCpssAvlItemRemove */

GT_BOOL prvCpssDxChVirtualTcamSegmentTableAvlItemDebugPrintEnable = GT_FALSE;

GT_VOID prvCpssDxChVirtualTcamDbSegmentTableAvlItemDebugPrintSet(GT_BOOL enable)
{
    prvCpssDxChVirtualTcamSegmentTableAvlItemDebugPrintEnable = enable;
}

GT_STATUS prvCpssDxChVirtualTcamSegmentTableAvlItemInsert
(
    IN  PRV_CPSS_AVL_TREE_ID                            treeId,
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segPtr
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segPtr1;

    if ((segPtr->rowsAmount == 0) || (segPtr->segmentColumnsMap == 0))
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssAvlItemInsert(treeId, segPtr);
    if (rc == GT_OK) return GT_OK;
    prvCpssDxChVirtualTcamDbSegmentTableException();
    if (prvCpssDxChVirtualTcamSegmentTableAvlItemDebugPrintEnable == GT_FALSE)
    {
        return rc;
    }
    /* print */
    cpssOsPrintf("prvCpssAvlItemInsert failed: \n");
    cpssOsPrintf(
        "0x%08X(%3.3d) columnsMap 0x%08X lookup %04X vTCAM %04X\n",
        segPtr->rowsBase, segPtr->rowsAmount,
        segPtr->segmentColumnsMap,
        segPtr->lookupId, segPtr->vTcamId);
    segPtr1 = prvCpssAvlSearch(treeId, segPtr);
    if (segPtr1 == NULL)
    {
        cpssOsPrintf("prvCpssAvlSearch found nothing equal \n");
    }
    else
    {
        cpssOsPrintf("prvCpssAvlSearch found equal: \n");
        cpssOsPrintf(
            "0x%08X(%3.3d) columnsMap 0x%08X lookup %04X vTCAM %04X\n",
            segPtr1->rowsBase, segPtr1->rowsAmount,
            segPtr1->segmentColumnsMap,
            segPtr1->lookupId, segPtr1->vTcamId);
    }
    return rc;
}

GT_VOID* prvCpssDxChVirtualTcamSegmentTableAvlItemRemove
(
    IN  PRV_CPSS_AVL_TREE_ID                            treeId,
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segPtr
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segPtr1;

    segPtr1 = prvCpssAvlItemRemove(treeId, segPtr);
    if (segPtr1 != NULL) return segPtr1;
    prvCpssDxChVirtualTcamDbSegmentTableException();
    if (prvCpssDxChVirtualTcamSegmentTableAvlItemDebugPrintEnable == GT_FALSE)
    {
        return segPtr1;
    }
    /* print */
    cpssOsPrintf("prvCpssAvlItemRemove failed: \n");
    cpssOsPrintf(
        "0x%08X(%3.3d) columnsMap 0x%08X lookup %04X vTCAM %04X\n",
        segPtr->rowsBase, segPtr->rowsAmount,
        segPtr->segmentColumnsMap,
        segPtr->lookupId, segPtr->vTcamId);
    segPtr1 = prvCpssAvlSearch(treeId, segPtr);
    if (segPtr1 == NULL)
    {
        cpssOsPrintf("prvCpssAvlSearch found nothing equal \n");
    }
    else
    {
        cpssOsPrintf("prvCpssAvlSearch found equal: \n");
        cpssOsPrintf(
            "0x%08X(%3.3d) columnsMap 0x%08X lookup %04X vTCAM %04X\n",
            segPtr1->rowsBase, segPtr1->rowsAmount,
            segPtr1->segmentColumnsMap,
            segPtr1->lookupId, segPtr1->vTcamId);
    }
    return NULL;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableColumnsToUnitsAmount function
* @endinternal
*
* @brief   Get amount of units in row columns ENUM element.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] segmentColumns           - segment columns in one horizontal block
*                                       amount of units in segment columns.
*/
GT_U32 prvCpssDxChVirtualTcamDbSegmentTableColumnsToUnitsAmount
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT segmentColumns
)
{
    switch (segmentColumns)
    {
        /* SIP5 devices - ruleSize unit is 10-byte rule */
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E:
            return 6;
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01234_E:
            return 5;
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_0123_E:
            return 4;
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_2345_E:
            return 4;
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_45_E:
            return 2;
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E:
            return 2;
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_5_E:
            return 1;
        /* legacy devices  - ruleSize unit is standard rule */
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_3_E:
            return 1;
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_012_E:
            return 3;
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_ALL_E:
            return 4;
        /* legacy devices TTI  - ruleSize unit is standard rule */
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_TTI_ALL_E:
            return 1;

        /* NONE and not supported */
        default: return 0;
    }
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableSegColComplementGet function
* @endinternal
*
* @brief   returns complement of segment column.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] segmentColumns           - segment columns
*                                       complement of segment column.
*/
PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT prvCpssDxChVirtualTcamDbSegmentTableSegColComplementGet
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT segmentColumns
)
{
    switch (segmentColumns)
    {
        default:
            prvCpssDxChVirtualTcamDbSegmentTableException();
            return PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E;

        /* SIP5 devices */
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_5_E:
            return PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01234_E;

        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E:
            return PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_2345_E;

        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_45_E:
            return PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_0123_E;

        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_0123_E:
            return PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_45_E;

        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_2345_E:
            return PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E;

        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01234_E:
            return PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_5_E;

        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E:
            return PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E;

        /* Legacy devices */
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_3_E:
            return PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_012_E;

        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_012_E:
            return PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_3_E;

        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_ALL_E:
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_TTI_ALL_E:
            return PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E;
    }
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableSegColMapToMinRuleColumnsBitmap function
* @endinternal
*
* @brief   Convert segment columns map to bitmap of minimal rule places.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamWidth                - number of columns in TCAM
* @param[in] horzBlockWidth           - amount of columns in horizontal block
* @param[in] segmentColumnsMap        - segment columns map
*                                       Calcuilated bitmap.
*/
GT_U32 prvCpssDxChVirtualTcamDbSegmentTableSegColMapToMinRuleColumnsBitmap
(
    IN  GT_U32                                            tcamWidth,
    IN  GT_U32                                            horzBlockWidth,
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP segmentColumnsMap
)
{
    GT_U32 i, totalBmp, blockBmp, horzBlocksNum;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT segmentColumns;

    horzBlocksNum = tcamWidth / horzBlockWidth;

    totalBmp = 0;
    for (i = 0; (i < horzBlocksNum); i++)
    {
        segmentColumns = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
            segmentColumnsMap, i);
        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_BITMAP_OF_COLUMNS_MAC(
            segmentColumns, blockBmp);
        totalBmp |= (blockBmp << (horzBlockWidth * i));
    }
    return totalBmp;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableRowCapacity function
* @endinternal
*
* @brief   Calculate one row capacity for horizontal block part of vTcam segment.
*         Not useable and not empty columns are valid, 0 returned.
*         80-byte rules supported for legacy devices only: size = 3.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
*                                       amount of rules that can be placed in segment columns.
*/
GT_U32 prvCpssDxChVirtualTcamDbSegmentTableRowCapacity
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT rowType,
    IN  GT_U32                                        ruleSize
)
{
    if (ruleSize == 0)
    {
        return 0;
    }
    /* PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E used only as a part */
    /* of a rule that passes from previous block, it's capacity is 0      */
    switch (rowType)
    {
        /* SIP5 devices - ruleSize unit is 10-byte rule */
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E:
            return (6 / ruleSize);
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_0123_E:
            return ((ruleSize == 4) ? 1 : 0);
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01234_E:
            return ((ruleSize == 5) ? 1 : 0);
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_2345_E:
            return ((ruleSize <= 3) ? (4 / ruleSize) : 0);
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_45_E:
            return (2 / ruleSize);
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_5_E:
            return (1 / ruleSize);
        /* legacy devices  - ruleSize unit is standard rule */
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_3_E:
            return (1 / ruleSize);
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_012_E:
            return ((ruleSize == 3) ? 1 : 0);
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_ALL_E:
            return (4 / ruleSize);
        /* legacy devices TTI  - ruleSize unit is standard rule */
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_TTI_ALL_E:
            return (1 / ruleSize);

        /* NONE and not supported */
        default: return 0;
    }
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableMaxRulesAmountPerColumnsTypeGet function
* @endinternal
*
* @brief   Calculate one row capacity for horizontal block part of vTcam segment.
*         Not useable and not empty columns cause error tracing.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] segmentColumns           - segment columns in one horizontal block
* @param[in] size                     - rule  measured in minimal rule units
*                                       amount of rules that can be placed in segment columns.
*/
GT_U32 prvCpssDxChVirtualTcamDbSegmentTableMaxRulesAmountPerColumnsTypeGet
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT segmentColumns,
    IN  GT_U32                                                  size
)
{
    GT_U32 result;

    if (segmentColumns == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E)
    {
        /* no exception */
        return 0;
    }
    result = prvCpssDxChVirtualTcamDbSegmentTableRowCapacity(
        segmentColumns, size);
    if (result == 0)
    {
        /* should never occur */
        prvCpssDxChVirtualTcamDbSegmentTableException();
        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_DBG_PRINT(
            (" prvCpssDxChVirtualTcamDbSegmentTableMaxRulesAmountPerColumnsTypeGet size"));
        return 0;
    }
    return result;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableMaxRowRulesAmountGet function
* @endinternal
*
* @brief   Calculate full row capacity of vTcam segment.
*         Not useable and not empty columns skipped without error tracing.
*         Does not support 80-byte rules.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] ptr1                     - (pointer to) segment structure
* @param[in] size                     - rule  measured in 10-byte units
*                                       amount of rules that can be placed in segment row.
*/
static GT_U32 prvCpssDxChVirtualTcamDbSegmentTableMaxRowRulesAmountGet
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC* ptr1,
    IN  GT_U32                                           size
)
{
    GT_U32 i, v;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT segmentColumns;

    if (size > 6)
    {
        /* should never occur */
        prvCpssDxChVirtualTcamDbSegmentTableException();
        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_DBG_PRINT(
            ("prvCpssDxChVirtualTcamDbSegmentTableMaxRowRulesAmountGet  size"));
        return 0;
    }

    v = 0;
    for (i = 0; (i < PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_MAX_SUPPORTED_CNS); i++)
    {
        segmentColumns = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
            ptr1->segmentColumnsMap, i);
        if (segmentColumns == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E) continue;

        v += prvCpssDxChVirtualTcamDbSegmentTableRowCapacity(
            segmentColumns, size);
    }
    return v;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableFullRowColumnsGet function
* @endinternal
*
* @brief   Full rows element of columns enum depends on device type.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
*                                       Full rows element of columns enum.
*/
PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT prvCpssDxChVirtualTcamDbSegmentTableFullRowColumnsGet
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr
)
{
    switch (tcamSegCfgPtr->deviceClass)
    {
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP5_E:
            return PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E;

        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_E:
            return PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_ALL_E;

        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_TTI_E:
            return PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_TTI_ALL_E;

        default:
            prvCpssDxChVirtualTcamDbSegmentTableException();
            return PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E;
    }
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableFullRowColumnsToCrop function
* @endinternal
*
* @brief   Element of columns enum to be cropped from full row for given rule size.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] ruleSizeInUnits          - rule size in units of minimal rule
* @param[in] horzBlockIndex           - horizontal block index
*                                       columns to be cropped from full row for given rule size.
*/
static PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT prvCpssDxChVirtualTcamDbSegmentTableFullRowColumnsToCrop
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr,
    IN     GT_U32                              ruleSizeInUnits,
    IN     GT_U32                              horzBlockIndex
)
{
    switch (tcamSegCfgPtr->deviceClass)
    {
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP5_E:
            switch (ruleSizeInUnits)
            {
                case 1:
                case 2:
                case 3: return PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E;
                case 4: return PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_0123_E;
                case 5: return PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01234_E;
                case 6: return PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E;
                case 8:
                    return (((horzBlockIndex % 2) == 0)
                        ? PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E
                        : PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E);
                default: break;
            }

        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_E:
            switch (ruleSizeInUnits)
            {
                case 1:
                case 2: return PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_ALL_E;
                case 3: return PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_012_E;
                default: break;
            }

        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_TTI_E:
            switch (ruleSizeInUnits)
            {
                case 1: return PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_TTI_ALL_E;
                default: break;
            }

        default: break;
    }
    prvCpssDxChVirtualTcamDbSegmentTableException();
    return PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableSegmentRowCapacity function
* @endinternal
*
* @brief   Calculate full row capacity of vTcam segment.
*         Not useable and not empty columns skipped without error tracing.
*         Supports 80-byte rules.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] segmentColumnsMap        - Segment Columns Map
* @param[in] ruleSize                 - rule size measured in 10-byte units
*                                       amount of rules that can be placed in segment row.
*/
static GT_U32 prvCpssDxChVirtualTcamDbSegmentTableSegmentRowCapacity
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC     *tcamSegCfgPtr,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP segmentColumnsMap,
    IN     GT_U32                                            ruleSize
)
{
    GT_U32                                          rulesPerRow;  /* rules Per Row               */
    GT_U32                                          horzBlocksInRow; /* horzontal Blocks per row */
    GT_U32                                          horzBlockIdx;    /* horzontal Block Index    */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT   segmentColumns;  /* segment columns          */
    GT_U32                                          even80, odd80;   /* 80-bytes capacity        */

    horzBlocksInRow =
        (tcamSegCfgPtr->tcamColumnsAmount
        / tcamSegCfgPtr->lookupColumnAlignment);

    if ((tcamSegCfgPtr->deviceClass !=
         PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP5_E)
        || (ruleSize != 8))
    {
        /* all cases exclude SIP5 80-byte rules only */
        rulesPerRow = 0;
        for (horzBlockIdx = 0; (horzBlockIdx < horzBlocksInRow); horzBlockIdx++)
        {
            segmentColumns = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
                segmentColumnsMap, horzBlockIdx);

            if (segmentColumns
                == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E) continue;

            rulesPerRow += prvCpssDxChVirtualTcamDbSegmentTableRowCapacity(
                segmentColumns, ruleSize);
        }
        return rulesPerRow;
    }

    /* SIP5 80-byte rulesonly  case */
    rulesPerRow = 0;
    for (horzBlockIdx = 0; (horzBlockIdx < horzBlocksInRow); horzBlockIdx += 2)
    {
        even80 = 0;
        segmentColumns = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
            segmentColumnsMap, horzBlockIdx);
        if (segmentColumns == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E) even80 = 1;

        odd80  = 0;
        segmentColumns = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
            segmentColumnsMap, (horzBlockIdx + 1));
        if (segmentColumns == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E) odd80 = 1;
        if (segmentColumns == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E)  odd80 = 1;

        rulesPerRow += (even80 & odd80);
    }
    return rulesPerRow;
}

/**
* @internal prvCpssDxChVirtualTcamSegmentTableRuleSizeToUnits function
* @endinternal
*
* @brief   Convert Rule Size from enum to units.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] ruleSize                 - enum ruleSize
*                                       rule size in the smallest device rule units, 0 at error.
*/
GT_U32 prvCpssDxChVirtualTcamSegmentTableRuleSizeToUnits
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC   *tcamSegCfgPtr,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT        ruleSize
)
{
    switch (tcamSegCfgPtr->deviceClass)
    {
    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP5_E:
        switch (ruleSize)
        {
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_10_B_E:
                return 1;
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_20_B_E:
                return 2;
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E:
                return 3;
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_40_B_E:
                return 4;
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_50_B_E:
                return 5;
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E:
                return 6;
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E:
                return 8;
            default:
                prvCpssDxChVirtualTcamDbSegmentTableException();
                CPSS_LOG_ERROR_AND_RETURN_MAC(0, LOG_ERROR_NO_MSG);
        }
        break;
    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_E:
        switch (ruleSize)
        {
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E:
                return 1;
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E:
                return 2;
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E:
                return 3;
            default:
                prvCpssDxChVirtualTcamDbSegmentTableException();
                CPSS_LOG_ERROR_AND_RETURN_MAC(0, LOG_ERROR_NO_MSG);
        }
        break;
    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_TTI_E:
        switch (ruleSize)
        {
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E:
                return 1;
            default:
                prvCpssDxChVirtualTcamDbSegmentTableException();
                CPSS_LOG_ERROR_AND_RETURN_MAC(0, LOG_ERROR_NO_MSG);
        }
        break;
    }
    prvCpssDxChVirtualTcamDbSegmentTableException();
    CPSS_LOG_ERROR_AND_RETURN_MAC(0, LOG_ERROR_NO_MSG);
}

/**
* @internal prvCpssDxChVirtualTcamSegmentTableRuleSizeToEnum function
* @endinternal
*
* @brief   Convert Rule Size from units to enum.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] ruleSize                 - units measured ruleSize
*                                       rule size in the enum values, 0 at error.
*/
CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT prvCpssDxChVirtualTcamSegmentTableRuleSizeToEnum
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC   *tcamSegCfgPtr,
    IN  GT_U32                                      ruleSize
)
{
    switch (tcamSegCfgPtr->deviceClass)
    {
    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP5_E:
        switch (ruleSize)
        {
            case 1:
                return CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_10_B_E;
            case 2:
                return CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_20_B_E;
            case 3:
                return CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E;
            case 4:
                return CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_40_B_E;
            case 5:
                return CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_50_B_E;
            case 6:
                return CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E;
            case 8:
                return CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E;
            default:
                prvCpssDxChVirtualTcamDbSegmentTableException();
                CPSS_LOG_ERROR_AND_RETURN_MAC(0, LOG_ERROR_NO_MSG);
        }
        break;
    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_E:
        switch (ruleSize)
        {
            case 1:
                return CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E;
            case 2:
                return CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E;
            case 3:
                return CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E;
            default:
                prvCpssDxChVirtualTcamDbSegmentTableException();
                CPSS_LOG_ERROR_AND_RETURN_MAC(0, LOG_ERROR_NO_MSG);
        }
        break;
    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_TTI_E:
        switch (ruleSize)
        {
            case 1:
                return CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E;
            default:
                prvCpssDxChVirtualTcamDbSegmentTableException();
                CPSS_LOG_ERROR_AND_RETURN_MAC(0, LOG_ERROR_NO_MSG);
        }
        break;
    }
    prvCpssDxChVirtualTcamDbSegmentTableException();
    CPSS_LOG_ERROR_AND_RETURN_MAC(0, LOG_ERROR_NO_MSG);
}

/**
* @internal prvCpssDxChVirtualTcamSegmentTableRuleSizeToAlignment function
* @endinternal
*
* @brief   Convert Rule Size from enum to units.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] ruleSize                 - enum ruleSize
*                                       rule alignment in the smallest device rule units, 0 at error.
*/
GT_U32 prvCpssDxChVirtualTcamSegmentTableRuleSizeToAlignment
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC   *tcamSegCfgPtr,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT        ruleSize
)
{
    switch (tcamSegCfgPtr->deviceClass)
    {
    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP5_E:
        switch (ruleSize)
        {
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_10_B_E:
                return 1;
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_20_B_E:
                return 2;
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E:
                return 3;
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_40_B_E:
                return 6;
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_50_B_E:
                return 6;
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E:
                return 6;
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E:
                return 12;
            default:
                prvCpssDxChVirtualTcamDbSegmentTableException();
                CPSS_LOG_ERROR_AND_RETURN_MAC(0, LOG_ERROR_NO_MSG);
        }
        break;
    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_E:
        switch (ruleSize)
        {
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E:
                return 1;
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E:
                return 2;
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E:
                return 4;
            default:
                prvCpssDxChVirtualTcamDbSegmentTableException();
                CPSS_LOG_ERROR_AND_RETURN_MAC(0, LOG_ERROR_NO_MSG);
        }
        break;
    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_TTI_E:
        switch (ruleSize)
        {
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E:
                return 1;
            default:
                prvCpssDxChVirtualTcamDbSegmentTableException();
                CPSS_LOG_ERROR_AND_RETURN_MAC(0, LOG_ERROR_NO_MSG);
        }
        break;
    }
    prvCpssDxChVirtualTcamDbSegmentTableException();
    CPSS_LOG_ERROR_AND_RETURN_MAC(0, LOG_ERROR_NO_MSG);
}

GT_BOOL prvCpssDxChVirtualTcamSegmentTableIsBlockFull
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_BLOCK_INFO_STC *blockInfoPtr
)
{
    GT_U32 i;

    for (i = 0; (i < PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LAST_E); i++)
    {
        if (blockInfoPtr->freeSpace[i] != 0) return GT_FALSE;
    }
    return GT_TRUE;
}

GT_U32 prvCpssDxChVirtualTcamSegmentTableBlockFreeSpaceCapacityGet
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_BLOCK_INFO_STC *blockInfoPtr,
    IN  GT_U32                                                ruleSizeIn10byteUnits
)
{
    GT_U32 i, capacity;

    capacity = 0;
    for (i = 0; (i < PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LAST_E); i++)
    {
        capacity += (blockInfoPtr->freeSpace[i]
             *  prvCpssDxChVirtualTcamDbSegmentTableRowCapacity(i, ruleSizeIn10byteUnits));
    }
    return capacity;
}

/* rule alignment */
static GT_U32 prvCpssDxChVirtualTcamDbSegmentTableRuleAlignmentGet
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC     *tcamSegCfgPtr,
    IN  GT_U32                                        ruleSize
)
{
    switch (tcamSegCfgPtr->deviceClass)
    {
    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP5_E:
        switch (ruleSize)
        {
            case 1:  return 1;
            case 2:  return 2;
            case 3:  return 3;
            case 4:
            case 5:
            case 6:  return 6;
            case 8:  return 12;
            default:
                prvCpssDxChVirtualTcamDbSegmentTableException();
                return 0;
        }
        break;
    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_E:
        switch (ruleSize)
        {
            case 1:  return 1;
            case 2:  return 2;
            case 3:  return 4;
            default:
                prvCpssDxChVirtualTcamDbSegmentTableException();
                return 0;
        }
        break;
    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_TTI_E:
        switch (ruleSize)
        {
            case 1:  return 1;
            default:
                prvCpssDxChVirtualTcamDbSegmentTableException();
                return 0;
        }
        break;
    }
    prvCpssDxChVirtualTcamDbSegmentTableException();
    return 0;
}

/* get Columns type and rule multiplier */
static GT_STATUS prvCpssDxChVirtualTcamDbSegmentTableBestColumnsTypeGet
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC  *tcamSegCfgPtr,
    IN  GT_U32                                         ruleSize,
    OUT PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT  *columnsTypePtr,
    OUT GT_U32                                        *placeMultiplierPtr
)
{
    *placeMultiplierPtr = 0;
    *columnsTypePtr = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E;

    switch (tcamSegCfgPtr->deviceClass)
    {
    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP5_E:
        switch (ruleSize)
        {
            case 1:
            case 2:
            case 3:
            case 6:
                *columnsTypePtr = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E;
                *placeMultiplierPtr = prvCpssDxChVirtualTcamDbSegmentTableRowCapacity(
                    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E,
                    ruleSize);
                break;
            case 4:
                *columnsTypePtr = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_0123_E;
                *placeMultiplierPtr = 1;
                break;
            case 5:
                *columnsTypePtr = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01234_E;
                *placeMultiplierPtr = 1;
                break;
            default:
                prvCpssDxChVirtualTcamDbSegmentTableException();
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        break;
    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_E:
        switch (ruleSize)
        {
            case 1:
            case 2:
                *columnsTypePtr = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_ALL_E;
                *placeMultiplierPtr = prvCpssDxChVirtualTcamDbSegmentTableRowCapacity(
                    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_ALL_E,
                    ruleSize);
                break;
            case 3:
                *columnsTypePtr = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_012_E;
                *placeMultiplierPtr = prvCpssDxChVirtualTcamDbSegmentTableRowCapacity(
                    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_012_E,
                    ruleSize);
                break;
            default:
                prvCpssDxChVirtualTcamDbSegmentTableException();
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        break;
    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_TTI_E:
        switch (ruleSize)
        {
            case 1:
                *columnsTypePtr = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_TTI_ALL_E;
                *placeMultiplierPtr = prvCpssDxChVirtualTcamDbSegmentTableRowCapacity(
                    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_TTI_ALL_E,
                    ruleSize);
                break;
            default:
                prvCpssDxChVirtualTcamDbSegmentTableException();
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        break;
    }
    if (*placeMultiplierPtr == 0)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/* rule segment type for search free space */
static GT_U32 prvCpssDxChVirtualTcamDbSegmentTableStartColumnsTypeGet
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr,
    IN  GT_U32                                        ruleSize
)
{
    switch (tcamSegCfgPtr->deviceClass)
    {
    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP5_E:
        switch (ruleSize)
        {
            case 1:  return PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_5_E;
            case 2:  return PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_45_E;
            case 3:  return PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_2345_E;
            case 4:  return PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_0123_E;
            case 5:  return PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01234_E;
            case 6:  return PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E;
            default:
                prvCpssDxChVirtualTcamDbSegmentTableException();
                return 0;
        }
        break;
    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_E:
        switch (ruleSize)
        {
            case 1:  return PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_3_E;
            case 2:  return PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_ALL_E;
            case 3:  return PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_012_E;
            default:
                prvCpssDxChVirtualTcamDbSegmentTableException();
                return 0;
        }
        break;
    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_TTI_E:
        switch (ruleSize)
        {
            case 1:  return PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_TTI_ALL_E;
            default:
                prvCpssDxChVirtualTcamDbSegmentTableException();
                return 0;
        }
        break;
    }
    prvCpssDxChVirtualTcamDbSegmentTableException();
    return 0;
}


/* vTCAM size granularity - amount of rules in the full row of block */
static GT_U32 prvCpssDxChVirtualTcamDbSegmentTableVTcamSizeGranularity
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC   *tcamSegCfgPtr,
    IN  GT_U32                               ruleSize
)
{
    switch (tcamSegCfgPtr->deviceClass)
    {
    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP5_E:
        switch (ruleSize)
        {
            default:
            case 8:
            case 6:
            case 5:
            case 4: return 1;
            case 3: return 2;
            case 2: return 3;
            case 1: return 6;
        }
        break;
    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_E:
        /* legacy devices */
        switch (ruleSize)
        {
            default:
            case 3: return 1;
            case 2: return 2;
            case 1: return 4;
        }
        break;
    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_TTI_E:
        /* legacy devices */
        return 1;
    }
    return 0;
}

static GT_U32 prvCpssDxChVirtualTcamDbSegmentTableVTcamSegmentColumnsToLookupOrderRank
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT segmentColumns
)
{
    switch (segmentColumns)
    {
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E:
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E:
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_0123_E:
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01234_E:
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_ALL_E:
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_TTI_ALL_E:
            return 0;
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_2345_E:
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_012_E:
            return 2;
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_45_E:
            return 4;
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_5_E:
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_3_E:
            return 5;
        default:
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E:
            return 6;
    }
}

static GT_U32 prvCpssDxChVirtualTcamDbSegmentTableVTcamSegmentColumnsToFreeSpaceRank
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT segmentColumns
)
{
    switch (segmentColumns)
    {
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E:
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_ALL_E:
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_TTI_ALL_E:
            return 4;
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01234_E:
            return 3;
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_0123_E:
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_2345_E:
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_012_E:
            return 2;
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_45_E:
            return 1;
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_5_E:
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_3_E:
            return 0;
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E:
            return 100;
        default: /* should not occur */
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E:
            prvCpssDxChVirtualTcamDbSegmentTableException();
            return 100;
    }
}
/* segments in lookup order */
static GT_COMP_RES prvCpssDxChVirtualTcamDbSegmentTableLookupOrderCompare
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC* ptr1,
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC* ptr2
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP b1, b2;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT c1,c2;
    GT_U32 i, rank1, rank2;

    if (ptr1->rowsBase > ptr2->rowsBase)
        return GT_GREATER;
    if (ptr1->rowsBase < ptr2->rowsBase)
        return GT_SMALLER;

    b1 = ptr1->segmentColumnsMap;
    b2 = ptr2->segmentColumnsMap;
    for (i = 0; (i < PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_MAX_SUPPORTED_CNS); i++)
    {
        c1 = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(b1,i);
        c2 = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(b2,i);
        if (c1 == c2) continue;

        rank1 = prvCpssDxChVirtualTcamDbSegmentTableVTcamSegmentColumnsToLookupOrderRank(c1);
        rank2 = prvCpssDxChVirtualTcamDbSegmentTableVTcamSegmentColumnsToLookupOrderRank(c2);

        if (rank1 > rank2) return GT_GREATER; /* ptr1 is greater */
        if (rank1 < rank2) return GT_SMALLER;
    }

    return GT_EQUAL;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableLookupOrderCompareFunc function
* @endinternal
*
* @brief   AVL Tree compare function for segments in lookup order.
*         Used only internal.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] data_ptr1                - (pointer to) first segment DB entry
* @param[in] data_ptr2                - (pointer to) second segment DB entry
*                                       GT_EQUAL, GT_GREATER or GT_SMALLER.
*/
GT_COMP_RES prvCpssDxChVirtualTcamDbSegmentTableLookupOrderCompareFunc
(
    IN  GT_VOID    *data_ptr1,
    IN  GT_VOID    *data_ptr2
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC* ptr1;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC* ptr2;

    ptr1 = (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC*)data_ptr1;
    ptr2 = (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC*)data_ptr2;

    /* lookup id ignored */
    return prvCpssDxChVirtualTcamDbSegmentTableLookupOrderCompare(
        ptr1, ptr2);
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableLookupOrderCompareFreeSegFunc function
* @endinternal
*
* @brief   AVL Tree compare function for free segments in lookup order.
*         Used only internal.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] data_ptr1                - (pointer to) first segment DB entry
* @param[in] data_ptr2                - (pointer to) second segment DB entry
*                                       GT_EQUAL, GT_GREATER or GT_SMALLER.
*/
GT_COMP_RES prvCpssDxChVirtualTcamDbSegmentTableLookupOrderCompareFreeSegFunc
(
    IN  GT_VOID    *data_ptr1,
    IN  GT_VOID    *data_ptr2
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC* ptr1;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC* ptr2;

    ptr1 = (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC*)data_ptr1;
    ptr2 = (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC*)data_ptr2;

    if (ptr1->lookupId > ptr2->lookupId) return GT_GREATER;
    if (ptr1->lookupId < ptr2->lookupId) return GT_SMALLER;

    /* lookup id ignored */
    return prvCpssDxChVirtualTcamDbSegmentTableLookupOrderCompare(
        ptr1, ptr2);
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableResizeGranularityGet function
* @endinternal
*
* @brief   Get the resize granularity - according to rule size and vTcam Mng. configuration.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] enumRuleSize             - Enum Rule Size
*                                       Resize granularity in rules, default (12) on wrong parameters
*/
GT_U32 prvCpssDxChVirtualTcamDbSegmentTableResizeGranularityGet
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC     *tcamSegCfgPtr,
    IN     CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT    enumRuleSize
)
{
    GT_U32 ruleAmountPerRow;      /* Amount of rules or this size in 1 TCAM row */
    GT_U32 granularity;

    ruleAmountPerRow = prvCpssDxChVirtualTcamDbSegmentTableVTcamRuleAmountGranularity(
                        tcamSegCfgPtr, enumRuleSize);
    if (ruleAmountPerRow == 0)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        granularity = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_DEFAULT_AUTO_RESIZE_GRANULARITY_CNS;
    }
    else
    {
        /* Round up to a whole row */
        granularity = tcamSegCfgPtr->pVTcamMngPtr->autoResizeGranularity;
        granularity += (ruleAmountPerRow - 1);
        granularity -= (granularity % ruleAmountPerRow);
    }

    return granularity;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableVTcamRuleAmountGranularity function
* @endinternal
*
* @brief   Calculate rule amount granularity.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] enumRuleSize             - Enum Rule Size
*                                       Rules amount granularity, 0 on wrong parameters
*/
GT_U32 prvCpssDxChVirtualTcamDbSegmentTableVTcamRuleAmountGranularity
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC     *tcamSegCfgPtr,
    IN     CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT    enumRuleSize
)
{
    GT_U32 ruleSizeInUnits;

    ruleSizeInUnits =
        prvCpssDxChVirtualTcamSegmentTableRuleSizeToUnits(
            tcamSegCfgPtr, enumRuleSize);
    if (ruleSizeInUnits == 0)
    {
        return 0;
    }
    return prvCpssDxChVirtualTcamDbSegmentTableVTcamSizeGranularity(
        tcamSegCfgPtr, ruleSizeInUnits);
}

/* free space segments in order of "the worst preffered" */
static GT_COMP_RES prvCpssDxChVirtualTcamDbSegmentTableFreeCompare
(
    IN  GT_VOID    *data_ptr1,
    IN  GT_VOID    *data_ptr2
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC* ptr1;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC* ptr2;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT segmentColumns1;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT segmentColumns2;
    GT_U32 i;
    GT_U32 s1, s2;

    ptr1 = (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC*)data_ptr1;
    ptr2 = (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC*)data_ptr2;

    if (ptr1->lookupId > ptr2->lookupId)
        return GT_GREATER;
    if (ptr1->lookupId < ptr2->lookupId)
        return GT_SMALLER;

    /* preference rank */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_FIND_FIRST_MAC(
        ptr1->segmentColumnsMap, i, segmentColumns1);
    s1 = prvCpssDxChVirtualTcamDbSegmentTableVTcamSegmentColumnsToFreeSpaceRank(
        segmentColumns1);
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_FIND_FIRST_MAC(
        ptr2->segmentColumnsMap, i, segmentColumns2);
    s2 = prvCpssDxChVirtualTcamDbSegmentTableVTcamSegmentColumnsToFreeSpaceRank(
        segmentColumns2);

    if (s1 > s2)
        return GT_GREATER;
    if (s1 < s2)
        return GT_SMALLER;

    if (ptr1->rowsAmount > ptr2->rowsAmount)
        return GT_GREATER;
    if (ptr1->rowsAmount < ptr2->rowsAmount)
        return GT_SMALLER;

    return prvCpssDxChVirtualTcamDbSegmentTableLookupOrderCompare(
        ptr1, ptr2);
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableMemPoolCreate function
* @endinternal
*
* @brief   Create Segment Table memory pools for TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvCpssDxChVirtualTcamDbSegmentTableMemPoolCreate
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                  *pVTcamMngPtr
)
{
    GT_STATUS     rc;              /* return code                        */
    GT_U32        maxSegmentsInDb;  /* Total segments in one or both classes */

    maxSegmentsInDb = pVTcamMngPtr->tcamSegCfg.maxSegmentsInDb +
                        pVTcamMngPtr->tcamTtiSegCfg.maxSegmentsInDb;
    rc = cpssBmPoolCreate(
        sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC),
        CPSS_BM_POOL_4_BYTE_ALIGNMENT_E,
         maxSegmentsInDb,
        &(pVTcamMngPtr->segmentsPool));
    if (rc != GT_OK)
    {
        prvCpssDxChVirtualTcamDbSegmentTableDelete(pVTcamMngPtr);
        return rc;
    }
    /* pool for global free segments tree ordered by allocation preference */
    rc = prvCpssAvlMemPoolCreate(
        maxSegmentsInDb,
        &(pVTcamMngPtr->segNodesPoolFreePref));
    if (rc != GT_OK)
    {
        prvCpssDxChVirtualTcamDbSegmentTableDelete(pVTcamMngPtr);
        return rc;
    }
    /* pool for global free segments tree ordered by lookup */
    rc = prvCpssAvlMemPoolCreate(
        maxSegmentsInDb,
        &(pVTcamMngPtr->segNodesPoolFreeLu));
    if (rc != GT_OK)
    {
        prvCpssDxChVirtualTcamDbSegmentTableDelete(pVTcamMngPtr);
        return rc;
    }
    /* pool for trees for segments             */
    /*per - vTCAM trees for used segments      */
    /*temporary tree for vTCAM rearrangement   */
    rc = prvCpssAvlMemPoolCreate(
        (maxSegmentsInDb + MAX(pVTcamMngPtr->tcamSegCfg.maxSegmentsInDb,
                                pVTcamMngPtr->tcamTtiSegCfg.maxSegmentsInDb)),
        &(pVTcamMngPtr->segNodesPool));
    if (rc != GT_OK)
    {
        prvCpssDxChVirtualTcamDbSegmentTableDelete(pVTcamMngPtr);
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableFreeTreesCreate function
* @endinternal
*
* @brief   Create Segment Table free AVL trees and block array for TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
* @param[in,out] segTabCfgPtr             - (pointer to) segments table configuration.
* @param[in] fillSegFreeTrees         - skip the filling of segFreePrefTree,segFreeLuTree with buffers
*                                      from segmentsPool
* @param[in,out] segTabCfgPtr             - (pointer to) segments table configuration - updated.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvCpssDxChVirtualTcamDbSegmentTableFreeTreesCreate
(
    IN    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                  *pVTcamMngPtr,
    INOUT PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC       *segTabCfgPtr,
    IN    GT_BOOL                                              fillSegFreeTrees
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_BLOCK_INFO_STC* blockPtr; /* pointer to block info */
    GT_STATUS     rc;              /* return code                        */
    GT_U32        i,j;             /* loop index                         */
    GT_U32        blocksInRow;     /* number of blocks In Row            */
    GT_U32        blocksInColumn;  /* number of blocks In blocksInColumn */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC* segPtr; /* pointer to segment entry */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT        fullColumnsInDb;

    blocksInRow =
        (segTabCfgPtr->tcamColumnsAmount / segTabCfgPtr->lookupColumnAlignment);
    blocksInColumn =
        (segTabCfgPtr->tcamRowsAmount / segTabCfgPtr->lookupRowAlignment);
    segTabCfgPtr->blocksAmount = (blocksInRow * blocksInColumn);
    segTabCfgPtr->blockInfArr =
        (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_BLOCK_INFO_STC*)
        cpssOsMalloc(segTabCfgPtr->blocksAmount *
            sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_BLOCK_INFO_STC));
    if (segTabCfgPtr->blockInfArr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssAvlTreeCreate(
        (GT_INTFUNCPTR)prvCpssDxChVirtualTcamDbSegmentTableFreeCompare,
        pVTcamMngPtr->segNodesPoolFreePref,
        &(segTabCfgPtr->segFreePrefTree));
    if (rc != GT_OK)
    {
        prvCpssDxChVirtualTcamDbSegmentTableDelete(pVTcamMngPtr);
        return rc;
    }
    rc = prvCpssAvlTreeCreate(
        (GT_INTFUNCPTR)prvCpssDxChVirtualTcamDbSegmentTableLookupOrderCompareFreeSegFunc,
        pVTcamMngPtr->segNodesPoolFreeLu,
        &(segTabCfgPtr->segFreeLuTree));
    if (rc != GT_OK)
    {
        prvCpssDxChVirtualTcamDbSegmentTableDelete(pVTcamMngPtr);
        return rc;
    }

    fullColumnsInDb = prvCpssDxChVirtualTcamDbSegmentTableFullRowColumnsGet(segTabCfgPtr);
    for (i = 0; (i < segTabCfgPtr->blocksAmount); i++)
    {
        blockPtr                 = &(segTabCfgPtr->blockInfArr[i]);
        blockPtr->columnsBase    = ((i % blocksInRow) * segTabCfgPtr->lookupColumnAlignment);
        blockPtr->rowsBase       = ((i / blocksInRow) * segTabCfgPtr->lookupRowAlignment);
        blockPtr->columnsAmount  = segTabCfgPtr->lookupColumnAlignment;
        blockPtr->rowsAmount     = segTabCfgPtr->lookupRowAlignment;
        blockPtr->lookupId       =
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_LOOKUP_FREE_BLOCK_CNS;
        for (j = 0; (j < PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LAST_E); j++)
        {
            blockPtr->freeSpace[j] = 0;
        }
        blockPtr->freeSpace[fullColumnsInDb]
                = blockPtr->rowsAmount;

        if (segTabCfgPtr->lookupUnmapFuncPtr != NULL)
        {
            rc = segTabCfgPtr->lookupUnmapFuncPtr(
                segTabCfgPtr->cookiePtr,
                blockPtr->rowsBase,
                blockPtr->columnsBase);
            if (rc != GT_OK)
            {
                PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_DBG_PRINT(
                    ("lookupMapUnuncPtr returned %d", rc));
                prvCpssDxChVirtualTcamDbSegmentTableException();
                prvCpssDxChVirtualTcamDbSegmentTableDelete(pVTcamMngPtr);
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }

        if(fillSegFreeTrees == GT_FALSE)
        {
            /* skip the filling of segFreePrefTree,segFreeLuTree with buffers
                from segmentsPool */
            continue;
        }

        segPtr    = cpssBmPoolBufGet(pVTcamMngPtr->segmentsPool);
        if (segPtr == NULL)
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            prvCpssDxChVirtualTcamDbSegmentTableDelete(pVTcamMngPtr);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
        }
        cpssOsMemSet(segPtr, 0, sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC));
        segPtr->rowsBase          = blockPtr->rowsBase;
        segPtr->rowsAmount        = blockPtr->rowsAmount;
        segPtr->segmentColumnsMap =
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_EMPTY_CNS;
        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
            segPtr->segmentColumnsMap,
            (blockPtr->columnsBase / segTabCfgPtr->lookupColumnAlignment),
            fullColumnsInDb);
        segPtr->lookupId          =
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_LOOKUP_FREE_BLOCK_CNS;
        segPtr->vTcamId           =
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_VTAM_ID_FREE_SPACE_CNS;
        segPtr->rulesAmount       = 0xFFFFFFFF; /* not relevant */
        segPtr->ruleSize          = 0xFF;       /* not relevant */
        segPtr->baseLogicalIndex  = 0xFFFFFFFF; /* not relevant */
        rc = prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(
            segTabCfgPtr->segFreePrefTree, segPtr);
        if (rc != GT_OK)
        {
            prvCpssDxChVirtualTcamDbSegmentTableDelete(pVTcamMngPtr);
            return rc;
        }
        rc = prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(
            segTabCfgPtr->segFreeLuTree, segPtr);
        if (rc != GT_OK)
        {
            prvCpssDxChVirtualTcamDbSegmentTableDelete(pVTcamMngPtr);
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableCreate function
* @endinternal
*
* @brief   Create Segment Table for TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in,out] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
* @param[in] segTabCfgPtr             - (pointer to) segments table configuration.
* @param[in] segTabTtiCfgPtr          - (pointer to) segments table configuration for TTI TCAM on legacy devices.
* @param[in] fillSegFreeTrees         - skip the filling of segFreePrefTree,segFreeLuTree with buffers
*                                      from segmentsPool
* @param[in,out] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
*                                      Keeps created objects handles.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChVirtualTcamDbSegmentTableCreate
(
    INOUT PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                  *pVTcamMngPtr,
    IN    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC       *segTabCfgPtr,
    IN    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC       *segTabTtiCfgPtr,
    IN    GT_BOOL                                              fillSegFreeTrees
)
{
    GT_STATUS     rc;              /* return code                        */

    /* set NULL values to allocated pointers */
    pVTcamMngPtr->segmentsPool          = NULL;
    pVTcamMngPtr->segNodesPoolFreePref  = NULL;
    pVTcamMngPtr->segNodesPoolFreeLu    = NULL;
    pVTcamMngPtr->segNodesPool          = NULL;

    cpssOsMemCpy(
        &(pVTcamMngPtr->tcamSegCfg), segTabCfgPtr,
        sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC));
    pVTcamMngPtr->tcamTtiSegCfg = *segTabTtiCfgPtr;
    pVTcamMngPtr->tcamSegCfg.pVTcamMngPtr          = pVTcamMngPtr;
    pVTcamMngPtr->tcamSegCfg.blockInfArr           = NULL;
    pVTcamMngPtr->tcamSegCfg.segFreePrefTree       = NULL;
    pVTcamMngPtr->tcamSegCfg.segFreeLuTree         = NULL;
    pVTcamMngPtr->tcamTtiSegCfg.pVTcamMngPtr          = pVTcamMngPtr;
    pVTcamMngPtr->tcamTtiSegCfg.blockInfArr           = NULL;
    pVTcamMngPtr->tcamTtiSegCfg.segFreePrefTree       = NULL;
    pVTcamMngPtr->tcamTtiSegCfg.segFreeLuTree         = NULL;

    rc = prvCpssDxChVirtualTcamDbSegmentTableMemPoolCreate(pVTcamMngPtr);
    if (rc != GT_OK)
    {
        prvCpssDxChVirtualTcamDbSegmentTableDelete(pVTcamMngPtr);
        return rc;
    }

    rc = prvCpssDxChVirtualTcamDbSegmentTableFreeTreesCreate(
                pVTcamMngPtr,
                &pVTcamMngPtr->tcamSegCfg,
                fillSegFreeTrees);
    if (rc != GT_OK)
    {
        prvCpssDxChVirtualTcamDbSegmentTableDelete(pVTcamMngPtr);
        return rc;
    }
    if ((segTabTtiCfgPtr->lookupColumnAlignment != 0) &&
        (segTabTtiCfgPtr->lookupRowAlignment != 0))
    {
        rc = prvCpssDxChVirtualTcamDbSegmentTableFreeTreesCreate(
                    pVTcamMngPtr,
                    &pVTcamMngPtr->tcamTtiSegCfg,
                    fillSegFreeTrees);
        if (rc != GT_OK)
        {
            prvCpssDxChVirtualTcamDbSegmentTableDelete(pVTcamMngPtr);
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableDelete function
* @endinternal
*
* @brief   Delete Segment Table for TCAM. (Free used memory)
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in,out] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
* @param[in,out] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
*                                      Deleted objects handles set to NULL.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChVirtualTcamDbSegmentTableDelete
(
    INOUT PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                  *pVTcamMngPtr
)
{
    GT_STATUS     rc, rcFinal;   /* return code                        */

    rcFinal = GT_OK;

    if (pVTcamMngPtr->tcamSegCfg.segFreeLuTree != NULL)
    {
        /* during tree deleting all data will be freed from segmentsPool */
        rc = prvCpssAvlTreeDelete(
            pVTcamMngPtr->tcamSegCfg.segFreeLuTree,
            (GT_VOIDFUNCPTR)cpssBmPoolBufFree,
            (GT_VOID*)pVTcamMngPtr->segmentsPool);
        pVTcamMngPtr->tcamSegCfg.segFreeLuTree = NULL;
        if (rc != GT_OK)
        {
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_DBG_PRINT(
                ("prvCpssAvlTreeDelete returned %d", rc));
            rcFinal = rc;
        }
    }
    if (pVTcamMngPtr->tcamTtiSegCfg.segFreeLuTree != NULL)
    {
        /* during tree deleting all data will be freed from segmentsPool */
        rc = prvCpssAvlTreeDelete(
            pVTcamMngPtr->tcamTtiSegCfg.segFreeLuTree,
            (GT_VOIDFUNCPTR)cpssBmPoolBufFree,
            (GT_VOID*)pVTcamMngPtr->segmentsPool);
        pVTcamMngPtr->tcamTtiSegCfg.segFreeLuTree = NULL;
        if (rc != GT_OK)
        {
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_DBG_PRINT(
                ("prvCpssAvlTreeDelete returned %d", rc));
            rcFinal = rc;
        }
    }
    if (pVTcamMngPtr->segNodesPoolFreeLu != NULL)
    {
        rc = prvCpssAvlMemPoolDelete(pVTcamMngPtr->segNodesPoolFreeLu);
        if (rc != GT_OK)
        {
            cpssBmPoolReCreate(pVTcamMngPtr->segNodesPoolFreeLu);
            prvCpssAvlMemPoolDelete(pVTcamMngPtr->segNodesPoolFreeLu);
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_DBG_PRINT(
                ("prvCpssAvlMemPoolDelete returned %d", rc));
            rcFinal = rc;
        }
        pVTcamMngPtr->segNodesPoolFreeLu    = NULL;
    }

    if (pVTcamMngPtr->tcamSegCfg.segFreePrefTree != NULL)
    {
        /* segments data freed destroying Lookup sorted tree */
        rc = prvCpssAvlTreeDelete(
            pVTcamMngPtr->tcamSegCfg.segFreePrefTree,
            (GT_VOIDFUNCPTR)NULL,
            (GT_VOID*)NULL);
        pVTcamMngPtr->tcamSegCfg.segFreePrefTree = NULL;
        if (rc != GT_OK)
        {
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_DBG_PRINT(
                ("prvCpssAvlTreeDelete returned %d", rc));
            rcFinal = rc;
        }
    }
    if (pVTcamMngPtr->tcamTtiSegCfg.segFreePrefTree != NULL)
    {
        /* segments data freed destroying Lookup sorted tree */
        rc = prvCpssAvlTreeDelete(
            pVTcamMngPtr->tcamTtiSegCfg.segFreePrefTree,
            (GT_VOIDFUNCPTR)NULL,
            (GT_VOID*)NULL);
        pVTcamMngPtr->tcamTtiSegCfg.segFreePrefTree = NULL;
        if (rc != GT_OK)
        {
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_DBG_PRINT(
                ("prvCpssAvlTreeDelete returned %d", rc));
            rcFinal = rc;
        }
    }
    if (pVTcamMngPtr->segNodesPoolFreePref != NULL)
    {
        rc = prvCpssAvlMemPoolDelete(pVTcamMngPtr->segNodesPoolFreePref);
        if (rc != GT_OK)
        {
            cpssBmPoolReCreate(pVTcamMngPtr->segNodesPoolFreePref);
            prvCpssAvlMemPoolDelete(pVTcamMngPtr->segNodesPoolFreePref);
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_DBG_PRINT(
                ("prvCpssAvlMemPoolDelete returned %d", rc));
            rcFinal = rc;
        }
        pVTcamMngPtr->segNodesPoolFreePref    = NULL;
    }
    if (pVTcamMngPtr->segNodesPool != NULL)
    {
        rc = prvCpssAvlMemPoolDelete(pVTcamMngPtr->segNodesPool);
        if (rc != GT_OK)
        {
            cpssBmPoolReCreate(pVTcamMngPtr->segNodesPool);
            prvCpssAvlMemPoolDelete(pVTcamMngPtr->segNodesPool);
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_DBG_PRINT(
                ("prvCpssAvlMemPoolDelete returned %d", rc));
            rcFinal = rc;
        }
        pVTcamMngPtr->segNodesPool    = NULL;
    }
    if (pVTcamMngPtr->segmentsPool != NULL)
    {
        rc = cpssBmPoolDelete(pVTcamMngPtr->segmentsPool);
        if (rc != GT_OK)
        {
            if (prvCpssDxChVirtualTcamDebugTraceEnable != GT_FALSE)
            {
                cpssBmPoolDebugDumpBusyMemory(
                    pVTcamMngPtr->segmentsPool,
                    0 /*skipBufs*/, 0xFFFFFFFF /*dumpBufs*/);
            }
            cpssBmPoolReCreate(pVTcamMngPtr->segmentsPool);
            cpssBmPoolDelete(pVTcamMngPtr->segmentsPool);
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_DBG_PRINT(
                ("cpssBmPoolDelete returned %d", rc));
            rcFinal = rc;
        }
        pVTcamMngPtr->segmentsPool    = NULL;
    }

    if (pVTcamMngPtr->tcamSegCfg.blockInfArr != NULL)
    {
        cpssOsFree(pVTcamMngPtr->tcamSegCfg.blockInfArr);
        pVTcamMngPtr->tcamSegCfg.blockInfArr = NULL;
    }
    if (pVTcamMngPtr->tcamTtiSegCfg.blockInfArr != NULL)
    {
        cpssOsFree(pVTcamMngPtr->tcamTtiSegCfg.blockInfArr);
        pVTcamMngPtr->tcamTtiSegCfg.blockInfArr = NULL;
    }

    return rcFinal;
}

/**
* @internal copyVtcamSegmentsLayout function
* @endinternal
*
* @brief   copy from source vtcam segments in source TCAM manager to
*         target vtcam segments in target TCAM manager
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] srcVtcamInfoPtr          - (pointer to) the source virtual TCAM
* @param[in,out] trgVtcamInfoPtr          - (pointer to) the target virtual TCAM
* @param[in,out] trgVtcamInfoPtr          - (pointer to) the updated target virtual TCAM
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NO_RESOURCE           - not enough resources
*/
static GT_STATUS copyVtcamSegmentsLayout
(
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC               *trgVTcamMngPtr,
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC      *srcVtcamInfoPtr,
    INOUT PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC   *trgVtcamInfoPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *srcSegPtr; /* segment from src TCAM */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *trgBuffPtr;/* buffer in target TCAM */
    PRV_CPSS_AVL_TREE_PATH  path;            /* tree iterator                 */
    GT_VOID_PTR             dbEntryPtr;      /* pointer to entry in DB        */
    GT_BOOL found;

    CPSS_LOG_INFORMATION_MAC("start loop on the tree from the source TCAM \n");
    /* loop on the tree from the source TCAM */
    found = prvCpssAvlPathSeek(
            srcVtcamInfoPtr->segmentsTree, PRV_CPSS_AVL_TREE_SEEK_FIRST_E,
            path, &dbEntryPtr /* use dedicated var to avoid warnings */);
    srcSegPtr = dbEntryPtr;
    while (found != GT_FALSE)
    {
        CPSS_LOG_INFORMATION_MAC("start handle segment from the source vtcam : rowsBase[%d] rowsAmount[%d] \n",
            srcSegPtr->rowsBase,
            srcSegPtr->rowsAmount);

        /* get new buffer from the target TCAM */
        trgBuffPtr = cpssBmPoolBufGet(trgVTcamMngPtr->segmentsPool);
        if (trgBuffPtr == NULL)
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, "failed on cpssBmPoolBufGet");
        }
        /* copy data from the source segment */
        *trgBuffPtr = *srcSegPtr;

        /* add the new buffer to the trees of the target vTCAM */
        rc = prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(
            trgVtcamInfoPtr->segmentsTree, trgBuffPtr);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "failed on prvCpssAvlItemInsert");
        }

        /* jump to next segment in the source vTCAM */
        found = prvCpssAvlPathSeek(
                srcVtcamInfoPtr->segmentsTree, PRV_CPSS_AVL_TREE_SEEK_NEXT_E,
                path, &dbEntryPtr /* use dedicated var to avoid warnings */);
        srcSegPtr = dbEntryPtr;
    }

    CPSS_LOG_INFORMATION_MAC("copy tcamInfo and rulesAmount[%d] from the source TCAM \n",
        srcVtcamInfoPtr->rulesAmount);

    trgVtcamInfoPtr->tcamInfo    = srcVtcamInfoPtr->tcamInfo;
    trgVtcamInfoPtr->rulesAmount = srcVtcamInfoPtr->rulesAmount;

    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableFreeTreesCopy function
* @endinternal
*
* @brief   Copy Segment Table from source TCAM to target TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] srcTcamSegCfgPtr         - (pointer to) source segment configuration information.
* @param[in] trgTcamSegCfgPtr         - (pointer to) target segment configuration information.
* @param[in,out] trgVTcamMngPtr           - (pointer to) target VTCAM Manager structure
* @param[in,out] trgVTcamMngPtr           - (pointer to) updated target VTCAM Manager structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
* @retval GT_NO_RESOURCE           - not enough resources
*/
static GT_STATUS prvCpssDxChVirtualTcamDbSegmentTableFreeTreesCopy
(
    IN    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC       *srcTcamSegCfgPtr,
    IN    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC       *trgTcamSegCfgPtr,
    INOUT PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                  *trgVTcamMngPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *srcSegPtr; /* segment from src TCAM */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *trgBuffPtr;/* buffer in target TCAM */
    PRV_CPSS_AVL_TREE_PATH  path;            /* tree iterator                 */
    GT_VOID_PTR             dbEntryPtr;      /* pointer to entry in DB        */
    GT_BOOL found;

    CPSS_LOG_INFORMATION_MAC("start loop on the tree from the source TCAM \n");
    /* loop on the tree from the source TCAM */
    found = prvCpssAvlPathSeek(
            srcTcamSegCfgPtr->segFreeLuTree, PRV_CPSS_AVL_TREE_SEEK_FIRST_E,
            path, &dbEntryPtr /* use dedicated var to avoid warnings */);
    while (found != GT_FALSE)
    {
        srcSegPtr = dbEntryPtr;
        CPSS_LOG_INFORMATION_MAC("start handle segment from the source TCAM : rowsBase[%d] rowsAmount[%d] columnsUse[0x%08X]\n",
            srcSegPtr->rowsBase,
            srcSegPtr->rowsAmount,
            srcSegPtr->segmentColumnsMap);

        /* get new buffer from the target TCAM */
        trgBuffPtr = cpssBmPoolBufGet(trgVTcamMngPtr->segmentsPool);
        if (trgBuffPtr == NULL)
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
        }
        /* copy data from the source segment */
        *trgBuffPtr = *srcSegPtr;

        /* add the new buffer to the 2 trees of the target TCAM */
        rc = prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(
            trgTcamSegCfgPtr->segFreePrefTree, trgBuffPtr);
        if (rc != GT_OK)
        {
        }

        rc = prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(
            trgTcamSegCfgPtr->segFreeLuTree, trgBuffPtr);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* jump to next segment in the source TCAM */
        found = prvCpssAvlPathSeek(
                srcTcamSegCfgPtr->segFreeLuTree, PRV_CPSS_AVL_TREE_SEEK_NEXT_E,
                path, &dbEntryPtr /* use dedicated var to avoid warnings */);
    }

    /* copy array of blocks including lookup mappings and free space statistics */
    cpssOsMemCpy(
        trgTcamSegCfgPtr->blockInfArr, srcTcamSegCfgPtr->blockInfArr,
        (sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_BLOCK_INFO_STC)
            * srcTcamSegCfgPtr->blocksAmount));

    CPSS_LOG_INFORMATION_MAC("ended loop on the tree from the source TCAM \n");

    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableCopy function
* @endinternal
*
* @brief   Copy Segment Table from source TCAM to target TCAM.
*         also copy ALL source vtcam segments in source TCAM manager to
*         target vtcam segments in target TCAM manager
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] srcVTcamMngPtr           - (pointer to) source VTCAM Manager structure
* @param[in,out] trgVTcamMngPtr           - (pointer to) target VTCAM Manager structure
* @param[in] allocSkeletonOnly        - indication to allocate only skeleton
* @param[in,out] trgVTcamMngPtr           - (pointer to) updated target VTCAM Manager structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
* @retval GT_NO_RESOURCE           - not enough resources
*/
GT_STATUS prvCpssDxChVirtualTcamDbSegmentTableCopy
(
    IN    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                  *srcVTcamMngPtr,
    INOUT PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                  *trgVTcamMngPtr,
    IN    GT_BOOL                                             allocSkeletonOnly
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC* srcVtcamInfoPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC* trgVtcamInfoPtr;
    GT_U32  ii;

    if(allocSkeletonOnly == GT_FALSE)
    {
        rc = prvCpssDxChVirtualTcamDbSegmentTableFreeTreesCopy(
                    &srcVTcamMngPtr->tcamSegCfg,
                    &trgVTcamMngPtr->tcamSegCfg,
                    trgVTcamMngPtr);
        if (rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("fail to allocate memory for vTCAM Mng \n");
            goto cleanExit_lbl;
        }

        if((srcVTcamMngPtr->tcamTtiSegCfg.segFreeLuTree != NULL) &&
            (srcVTcamMngPtr->tcamTtiSegCfg.blockInfArr != NULL))
        {
            rc = prvCpssDxChVirtualTcamDbSegmentTableFreeTreesCopy(
                        &srcVTcamMngPtr->tcamTtiSegCfg,
                        &trgVTcamMngPtr->tcamTtiSegCfg,
                        trgVTcamMngPtr);
            if (rc != GT_OK)
            {
                CPSS_LOG_INFORMATION_MAC("fail to allocate memory for vTCAM Mng \n");
                goto cleanExit_lbl;
            }
        }

        CPSS_LOG_INFORMATION_MAC("start copy to the new TCAM manager the vTCAMs used segments of the orig TCAM manager \n");
    }
    else
    {
        /* do not copy info */
    }

    /* in orig TCAM : loop on all vTCAMs and copy to target TCAM proper vTCAM */
    for(ii = 0 ; ii < srcVTcamMngPtr->vTcamCfgPtrArrSize ; ii++)
    {
        srcVtcamInfoPtr = srcVTcamMngPtr->vTcamCfgPtrArr[ii];
        if(srcVtcamInfoPtr == NULL)
        {
            continue;
        }

        /* allocate vtcam with segments tree support */
        trgVtcamInfoPtr =
            prvCpssDxChVirtualTcamDbAllocWithOnlySegmentTable(trgVTcamMngPtr,
                ii,/*vTcamId*/
                &srcVtcamInfoPtr->tcamInfo,
                allocSkeletonOnly);
        if (trgVtcamInfoPtr == NULL)
        {
            CPSS_LOG_INFORMATION_MAC("fail to allocate memory for vTCAM [%d] \n",
                ii);
            rc = GT_OUT_OF_CPU_MEM;
            goto cleanExit_lbl;
        }

        if(allocSkeletonOnly == GT_TRUE)
        {
            /* do not copy info */
            continue;
        }

        CPSS_LOG_INFORMATION_MAC("copy used segments from vTCAM [%d] to the new TCAM manager \n",
            ii);

        rc = copyVtcamSegmentsLayout(trgVTcamMngPtr,srcVtcamInfoPtr,trgVtcamInfoPtr);
        if(rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("fail to copy used segments from vTCAM [%d] \n",
                ii);
            goto cleanExit_lbl;
        }
    }

    rc = GT_OK;

cleanExit_lbl:
    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("due to error : delete the segment table of the target TCAM \n");

        prvCpssDxChVirtualTcamDbSegmentTableDeleteWithVtcamsSegments(trgVTcamMngPtr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableDeleteWithVtcamsSegments function
* @endinternal
*
* @brief   1. delete TCAM manager that hold only 'segments table' .
*         2. delete all the vTCAMs of this TCAM manager (their 'segments table')
*         NOTE : DO NOT free the CPU memory of vTcamMngPtr ,
*         vTcamMngPtr->vTcamCfgPtrArr
*         vTcamCfgPtrArr[ii]
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] vTcamMngPtr              - (pointer to) the TCAM manager
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChVirtualTcamDbSegmentTableDeleteWithVtcamsSegments
(
    OUT PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC   *vTcamMngPtr
)
{
    GT_STATUS rc,finalRc = GT_OK;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC* vtcamInfoPtr;
    GT_U32  ii;

    /* loop on all vTCAMs and delete their segment tree */
    for(ii = 0 ; ii < vTcamMngPtr->vTcamCfgPtrArrSize ; ii++)
    {
        vtcamInfoPtr = vTcamMngPtr->vTcamCfgPtrArr[ii];
        if(vtcamInfoPtr == NULL)
        {
            continue;
        }

        /* delete the segment tree of the VTCAM */
        rc = prvCpssDxChVirtualTcamDbSegmentTableVTcamFree(vtcamInfoPtr);
        if(rc != GT_OK)
        {
            /* don't stop */
            finalRc = rc;
        }
    }
    /* Delete Segment Table for TCAM. */
    rc = prvCpssDxChVirtualTcamDbSegmentTableDelete(vTcamMngPtr);
    if(rc != GT_OK)
    {
        finalRc = rc;
    }

    return finalRc;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableUpdateBlockFreeSpace function
* @endinternal
*
* @brief   Decrease or increase per block counters for segment removed from free space.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] segPtr                   - (pointer to) segment removed from free space.
* @param[in] decrease                 - GT_TRUE - decrease, GT_FALSE - increase
*                                       None
*/
GT_VOID prvCpssDxChVirtualTcamDbSegmentTableUpdateBlockFreeSpace
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC              *tcamSegCfgPtr,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC            *segPtr,
    IN     GT_BOOL                                                    decrease
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT   segmentColumns;
    GT_U32                                          blocksInRow;
    GT_U32                                          blockHorzIdx ;
    GT_U32                                          blockIdx;

    blocksInRow =
        (tcamSegCfgPtr->tcamColumnsAmount
         / tcamSegCfgPtr->lookupColumnAlignment);
    for (blockHorzIdx = 0; (blockHorzIdx < blocksInRow); blockHorzIdx++)
    {
        segmentColumns =
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
                segPtr->segmentColumnsMap, blockHorzIdx);

        if (segmentColumns ==
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E) continue;

        blockIdx =
            ((blocksInRow *
              (segPtr->rowsBase
               / tcamSegCfgPtr->lookupRowAlignment)) +
            blockHorzIdx);

        if (decrease != GT_FALSE)
        {
            if (tcamSegCfgPtr->blockInfArr[blockIdx].freeSpace[segmentColumns]
                < segPtr->rowsAmount)
            {
                prvCpssDxChVirtualTcamDbSegmentTableException();
                PRV_DBG_TRACE(("Block free space underflow."));
                tcamSegCfgPtr->blockInfArr[blockIdx].freeSpace[segmentColumns] = 0;
                return;
            }
            tcamSegCfgPtr->blockInfArr[blockIdx].freeSpace[segmentColumns] -=
                segPtr->rowsAmount;
        }
        else
        {
            tcamSegCfgPtr->blockInfArr[blockIdx].freeSpace[segmentColumns] +=
                segPtr->rowsAmount;
        }
    }
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableCropSegmentVertical function
* @endinternal
*
* @brief   Split segment to 3 vertical pairs: low, cropped, high.
*         Used to crop part of free segment allocating or dragging vTcam and
*         to free part of vTCAM segment on resize.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] tree1                    - first tree including segments (NULL means tree not used)
* @param[in] tree2                    - second tree including segments (NULL means tree not used)
* @param[in,out] segPtr                   - (pointer to) segment being splitted.
* @param[in] rowsBase                 - rows base of cropped segment.
* @param[in] rowsAmount               - amount of rows in cropped segment
* @param[in] putBack                  - GT_TRUE  - put back to trees
*                                      GT_FALSE - leave removed from trees
* @param[in,out] segPtr                   - (pointer to) updated segment - cropped part.
*
* @param[out] lowSegPtrPtr             - (pointer to) (pointer to) low remainder of segment
*                                      (NULL parameter - caller not needs expects the value)
*                                      (NULL value - no low remainder)
* @param[out] highSegPtrPtr            - (pointer to) (pointer to) high remainder of segment
*                                      (NULL parameter - caller not needs expects the value)
*                                      (NULL value - no high remainder)
*
* @retval GT_OK                    - on success
* @retval other                    - on fail
*/
static GT_STATUS prvCpssDxChVirtualTcamDbSegmentTableCropSegmentVertical
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC             *tcamSegCfgPtr,
    IN     PRV_CPSS_AVL_TREE_ID                                       tree1,
    IN     PRV_CPSS_AVL_TREE_ID                                       tree2,
    INOUT  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC            *segPtr,
    IN     GT_U32                                                     rowsBase,
    IN     GT_U32                                                     rowsAmount,
    IN     GT_BOOL                                                    putBack,
    OUT    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC            **lowSegPtrPtr,
    OUT    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC            **highSegPtrPtr
)
{
    GT_STATUS                                                  rc;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                         *pVTcamMngPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC            *segPtr1;

    /* default values of output parameters */
    if (lowSegPtrPtr != NULL)
    {
        *lowSegPtrPtr = NULL;
    }
    if (highSegPtrPtr != NULL)
    {
        *highSegPtrPtr = NULL;
    }

    if ((rowsBase == segPtr->rowsBase) &&
        ((rowsBase + rowsAmount) == (segPtr->rowsBase + segPtr->rowsAmount)))
    {
        if (putBack == GT_FALSE)
        {
            /* remove updated segment from both trees */
            if (tree1 != NULL)
            {
                prvCpssDxChVirtualTcamSegmentTableAvlItemRemove(
                    tree1, (GT_VOID*)segPtr);
            }
            if (tree2 != NULL)
            {
                prvCpssDxChVirtualTcamSegmentTableAvlItemRemove(
                    tree2, (GT_VOID*)segPtr);
            }
        }
        return GT_OK;
    }

    if ((rowsAmount == 0) ||
        (rowsBase < segPtr->rowsBase) ||
        ((rowsBase + rowsAmount) > (segPtr->rowsBase + segPtr->rowsAmount)))
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "wrong segment cropped bounds");
    }

    /* remove updated segment from both trees */
    if (tree1 != NULL)
    {
        prvCpssDxChVirtualTcamSegmentTableAvlItemRemove(
            tree1, (GT_VOID*)segPtr);
    }
    if (tree2 != NULL)
    {
        prvCpssDxChVirtualTcamSegmentTableAvlItemRemove(
            tree2, (GT_VOID*)segPtr);
    }

    /* split low part */
    pVTcamMngPtr = tcamSegCfgPtr->pVTcamMngPtr;
    if (segPtr->rowsBase < rowsBase)
    {
        /* put back not cropped low rows */
        segPtr1 = cpssBmPoolBufGet(pVTcamMngPtr->segmentsPool);
        if (segPtr1 == NULL)
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, "cpssBmPoolBufGet failed");
        }
        cpssOsMemCpy(segPtr1, segPtr, sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC));
        segPtr1->rowsBase   = segPtr->rowsBase;
        segPtr1->rowsAmount = rowsBase - segPtr->rowsBase;
        /* update resulting segment */
        segPtr->rowsBase    = rowsBase;
        segPtr->rowsAmount -= segPtr1->rowsAmount;
        if (tree1 != NULL)
        {
            rc = prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(tree1, segPtr1);
            if (rc != GT_OK)
            {
                cpssBmPoolBufFree(pVTcamMngPtr->segmentsPool, segPtr1);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, "prvCpssAvlItemInsert failed");
            }
        }
        if (tree2 != NULL)
        {
            rc = prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(tree2, segPtr1);
            if (rc != GT_OK)
            {
                prvCpssDxChVirtualTcamSegmentTableAvlItemRemove(tree1, segPtr1);
                cpssBmPoolBufFree(pVTcamMngPtr->segmentsPool, segPtr1);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, "prvCpssAvlItemInsert failed");
            }
        }
        if (lowSegPtrPtr != NULL)
        {
            *lowSegPtrPtr = segPtr1;
        }
    }

    /* split high part */
    /* assumed segPtr->rowsBase == rowsBase */
    if (segPtr->rowsAmount > rowsAmount)
    {
        /* put back not cropped low rows */
        segPtr1 = cpssBmPoolBufGet(pVTcamMngPtr->segmentsPool);
        if (segPtr1 == NULL)
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, "cpssBmPoolBufGet failed");
        }
        cpssOsMemCpy(segPtr1, segPtr, sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC));
        segPtr1->rowsBase = segPtr->rowsBase + rowsAmount;
        segPtr1->rowsAmount = segPtr->rowsAmount - rowsAmount;
        /* update resulting segment */
        /*segPtr->rowsBase unchanged */
        segPtr->rowsAmount = rowsAmount;
        if (tree1 != NULL)
        {
            rc = prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(tree1, segPtr1);
            if (rc != GT_OK)
            {
                cpssBmPoolBufFree(pVTcamMngPtr->segmentsPool, segPtr1);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, "prvCpssAvlItemInsert failed");
            }
        }
        if (tree2 != NULL)
        {
            rc = prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(tree2, segPtr1);
            if (rc != GT_OK)
            {
                prvCpssDxChVirtualTcamSegmentTableAvlItemRemove(tree1, segPtr1);
                cpssBmPoolBufFree(pVTcamMngPtr->segmentsPool, segPtr1);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, "prvCpssAvlItemInsert failed");
            }
        }
        if (highSegPtrPtr != NULL)
        {
            *highSegPtrPtr = segPtr1;
        }
    }

    /* put back updated segment to both trees */
    if (putBack != GT_FALSE)
    {
        if (tree1 != NULL)
        {
            rc = prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(tree1, segPtr);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, "prvCpssAvlItemInsert failed");
            }
        }
        if (tree2 != NULL)
        {
            rc = prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(tree2, segPtr);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, "prvCpssAvlItemInsert failed");
            }
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableCropSegmentHorizontal function
* @endinternal
*
* @brief   Split segment to 2 horizontal pairs: cropped and remainder.
*         Used to crop part of free segment allocating or dragging vTcam and
*         to free part of vTCAM segment on resize.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] tree1                    - first tree including segments (NULL means tree not used)
* @param[in] tree2                    - second tree including segments (NULL means tree not used)
* @param[in,out] segPtr                   - (pointer to) segment being splitted.
* @param[in] segmentColumnsMap        - columns map of cropped segment.
* @param[in] putBack                  - GT_TRUE  - put back to trees
*                                      GT_FALSE - leave removed from trees
* @param[in,out] segPtr                   - (pointer to) updated segment - cropped part.
*
* @param[out] remainSegPtrPtr          - (pointer to) (pointer to) remainder of segment
*                                      (NULL parameter - caller not needs expects the value)
*                                      (NULL value - no low remainder)
*
* @retval GT_OK                    - on success
* @retval other                    - on fail
*/
static GT_STATUS prvCpssDxChVirtualTcamDbSegmentTableCropSegmentHorizontal
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC             *tcamSegCfgPtr,
    IN     PRV_CPSS_AVL_TREE_ID                                       tree1,
    IN     PRV_CPSS_AVL_TREE_ID                                       tree2,
    INOUT  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC            *segPtr,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP          segmentColumnsMap,
    IN     GT_BOOL                                                    putBack,
    OUT    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC            **remainSegPtrPtr
)
{
    GT_STATUS                                                  rc;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC            *segPtr1;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT              segmentColumns;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT              fullRowColumns;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT              croppedColumns;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT              remainColumns;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP          remainColumnsMap;
    GT_U32                                                     blocksInRow;
    GT_U32                                                     blockHorzIdx ;

    fullRowColumns = prvCpssDxChVirtualTcamDbSegmentTableFullRowColumnsGet(tcamSegCfgPtr);

    /* default values of output parameters */
    if (remainSegPtrPtr != NULL)
    {
        *remainSegPtrPtr = NULL;
    }

    if (segPtr->segmentColumnsMap == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_EMPTY_CNS)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "wrong segment columns found");
    }

    if (segmentColumnsMap == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_EMPTY_CNS)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "nothing to crop required");
    }

    if (segPtr->segmentColumnsMap == segmentColumnsMap)
    {
        if (putBack == GT_FALSE)
        {
            /* remove updated segment from both trees */
            if (tree1 != NULL)
            {
                prvCpssDxChVirtualTcamSegmentTableAvlItemRemove(
                    tree1, (GT_VOID*)segPtr);
            }
            if (tree2 != NULL)
            {
                prvCpssDxChVirtualTcamSegmentTableAvlItemRemove(
                    tree2, (GT_VOID*)segPtr);
            }
        }
        return GT_OK;
    }

    /* calculate remainder segment columns map*/
    blocksInRow =
        (tcamSegCfgPtr->tcamColumnsAmount
         / tcamSegCfgPtr->lookupColumnAlignment);
    remainColumnsMap = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_EMPTY_CNS;
    for (blockHorzIdx = 0; (blockHorzIdx < blocksInRow); blockHorzIdx++)
    {
        segmentColumns =
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
                segPtr->segmentColumnsMap, blockHorzIdx);
        croppedColumns =
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
                segmentColumnsMap, blockHorzIdx);

        /* this block-wide-column in remainder already contain NONE */
        if (segmentColumns == croppedColumns) continue;

        /* nothing cropped from this block - all it has is thye remainder */
        if (croppedColumns ==
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E)
        {
            remainColumns = segmentColumns;
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
                remainColumnsMap, blockHorzIdx, remainColumns);
            continue;
        }

        if (segmentColumns != fullRowColumns)
        {
            /* splitting not full block row not supported */
            prvCpssDxChVirtualTcamDbSegmentTableException();
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "wrong segment cropped columns");
        }
        remainColumns =
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_COMPLEMENT_MAC(
                croppedColumns);
        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
            remainColumnsMap, blockHorzIdx, remainColumns);
    }

    /* remove updated segment from both trees */
    if (tree1 != NULL)
    {
        prvCpssDxChVirtualTcamSegmentTableAvlItemRemove(
            tree1, (GT_VOID*)segPtr);
    }
    if (tree2 != NULL)
    {
        prvCpssDxChVirtualTcamSegmentTableAvlItemRemove(
            tree2, (GT_VOID*)segPtr);
    }

    /* split remainder part */
    /* assumed  remainColumnsMap != PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_EMPTY_CNS */
    /* put back not cropped columns */
    segPtr1 = cpssBmPoolBufGet(tcamSegCfgPtr->pVTcamMngPtr->segmentsPool);
    if (segPtr1 == NULL)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, "cpssBmPoolBufGet failed");
    }
    /* new segment - remainder */
    cpssOsMemCpy(segPtr1, segPtr, sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC));
    segPtr1->segmentColumnsMap = remainColumnsMap;
    /* update old segment to be cropped part */
    segPtr-> segmentColumnsMap = segmentColumnsMap;
    if (tree1 != NULL)
    {
        rc = prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(tree1, segPtr1);
        if (rc != GT_OK)
        {
            cpssBmPoolBufFree(tcamSegCfgPtr->pVTcamMngPtr->segmentsPool, segPtr1);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, "prvCpssAvlItemInsert failed");
        }
    }
    if (tree2 != NULL)
    {
        rc = prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(tree2, segPtr1);
        if (rc != GT_OK)
        {
            prvCpssDxChVirtualTcamSegmentTableAvlItemRemove(tree1, segPtr1);
            cpssBmPoolBufFree(tcamSegCfgPtr->pVTcamMngPtr->segmentsPool, segPtr1);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, "prvCpssAvlItemInsert failed");
        }
    }

    if (remainSegPtrPtr != NULL)
    {
        *remainSegPtrPtr = segPtr1;
    }

    /* put back updated segment to both trees */
    if (putBack != GT_FALSE)
    {
        if (tree1 != NULL)
        {
            rc = prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(tree1, segPtr);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, "prvCpssAvlItemInsert failed");
            }
        }
        if (tree2 != NULL)
        {
            rc = prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(tree2, segPtr);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, "prvCpssAvlItemInsert failed");
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableCropSegmentHorzBlockBitmap function
* @endinternal
*
* @brief   Split segment to 2 horizontal pairs:
*         cropped horizontal block bitmap and remainder.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] tree1                    - first tree including segments (NULL means tree not used)
* @param[in] tree2                    - second tree including segments (NULL means tree not used)
* @param[in,out] segPtr                   - (pointer to) segment being splitted.
*                                      horzBlockIndexPtr - horizontal block bitmasp to include
*                                      segmentColumnsMap - columns map of cropped segment.
* @param[in] putBack                  - GT_TRUE  - put back to trees
*                                      GT_FALSE - leave removed from trees
* @param[in,out] segPtr                   - (pointer to) updated segment - cropped part.
*
* @param[out] remainSegPtrPtr          - (pointer to) (pointer to) remainder of segment
*                                      (NULL parameter - caller not needs expects the value)
*                                      (NULL value - no low remainder)
*
* @retval GT_OK                    - on success
* @retval other                    - on fail
*/
static GT_STATUS prvCpssDxChVirtualTcamDbSegmentTableCropSegmentHorzBlockBitmap
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC             *tcamSegCfgPtr,
    IN     PRV_CPSS_AVL_TREE_ID                                       tree1,
    IN     PRV_CPSS_AVL_TREE_ID                                       tree2,
    INOUT  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC            *segPtr,
    IN     GT_U32                                                     horzBlockBitmap,
    IN     GT_BOOL                                                    putBack,
    OUT    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC            **remainSegPtrPtr
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT              segmentColumns;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP          segmentColumnsMap;
    GT_U32                                                     blocksInRow;
    GT_U32                                                     blockHorzIdx ;

    blocksInRow =
        (tcamSegCfgPtr->tcamColumnsAmount
         / tcamSegCfgPtr->lookupColumnAlignment);
    segmentColumnsMap = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_EMPTY_CNS;
    for (blockHorzIdx = 0; (blockHorzIdx < blocksInRow); blockHorzIdx++)
    {
        if ((horzBlockBitmap & (1 << blockHorzIdx)) == 0) continue;
        segmentColumns =
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
                segPtr->segmentColumnsMap, blockHorzIdx);
        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
            segmentColumnsMap, blockHorzIdx, segmentColumns);
    }

    return prvCpssDxChVirtualTcamDbSegmentTableCropSegmentHorizontal(
        tcamSegCfgPtr, tree1, tree2, segPtr,
        segmentColumnsMap, putBack, remainSegPtrPtr);
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableChainFreeSegment function
* @endinternal
*
* @brief   Chain free segment to trees.
*         Segment data in data pool, but not in both trees - will be added.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] segPtr                   - (pointer to) segment being added to trees.
*
* @retval GT_OK                    - on success
* @retval GT_NO_RESOURCE           - on no TCAM space
*/
GT_STATUS prvCpssDxChVirtualTcamDbSegmentTableChainFreeSegment
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC              *tcamSegCfgPtr,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC            *segPtr
)
{
    GT_STATUS rc, rcTotal; /* return code             */
    GT_BOOL   fullBlock;   /* full Block              */
    GT_U32    blocksInRow; /* number of blocks In Row */
    GT_U32    blockIdx;    /* blocks Index            */
    GT_U32    blockHorzIdx ; /* blocks horizontal Index */
    GT_U32    j;           /* loop Index              */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT  segmentColumns;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT  fullRowColumns;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP  checkColumnsMap;

    fullRowColumns = prvCpssDxChVirtualTcamDbSegmentTableFullRowColumnsGet(tcamSegCfgPtr);

    rcTotal = GT_OK;
    segPtr->vTcamId = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_VTAM_ID_FREE_SPACE_CNS;

    fullBlock = GT_TRUE; /* initial, will be overridden */

    if (segPtr->rowsAmount != tcamSegCfgPtr->lookupRowAlignment)
    {
        fullBlock = GT_FALSE;
    }
    blocksInRow =
        (tcamSegCfgPtr->tcamColumnsAmount
         / tcamSegCfgPtr->lookupColumnAlignment);
    /* find first Horizontal block in columns using map */
    /* assumed, but not checked that segment not covers other blocks */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_FIND_FIRST_MAC(
        segPtr->segmentColumnsMap, blockHorzIdx, segmentColumns);

    /* check that it is one column segment */
    checkColumnsMap = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_EMPTY_CNS;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
        checkColumnsMap, blockHorzIdx, segmentColumns);
    if (segPtr->segmentColumnsMap != checkColumnsMap)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_DBG_PRINT(
            ("ChainFreeSegment: Error, Multi column segment"));
    }

    if (segmentColumns != fullRowColumns)
    {
        fullBlock = GT_FALSE;
    }

    blockIdx =
        ((blocksInRow *
          (segPtr->rowsBase
           / tcamSegCfgPtr->lookupRowAlignment)) +
        blockHorzIdx);

    if (fullBlock != GT_FALSE)
    {
        segPtr->lookupId = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_LOOKUP_FREE_BLOCK_CNS;
        tcamSegCfgPtr->blockInfArr[blockIdx].lookupId =
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_LOOKUP_FREE_BLOCK_CNS;
        for (j = 0; (j < PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LAST_E); j++)
        {
            tcamSegCfgPtr->blockInfArr[blockIdx].freeSpace[j] = 0;
        }
        j = fullRowColumns;
        tcamSegCfgPtr->blockInfArr[blockIdx].freeSpace[j] =
            tcamSegCfgPtr->lookupRowAlignment;
        if (tcamSegCfgPtr->lookupUnmapFuncPtr != NULL)
        {
            rc = tcamSegCfgPtr->lookupUnmapFuncPtr(
                tcamSegCfgPtr->cookiePtr,
                tcamSegCfgPtr->blockInfArr[blockIdx].rowsBase,
                tcamSegCfgPtr->blockInfArr[blockIdx].columnsBase);
            if (rc != GT_OK)
            {
                PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_DBG_PRINT(
                    ("lookupMapUnuncPtr returned %d", rc));
                prvCpssDxChVirtualTcamDbSegmentTableException();
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }
    }
    else
    {
        /* segmentColumns found in segmentColumnsMap */
        tcamSegCfgPtr->blockInfArr[blockIdx].freeSpace[segmentColumns] +=
            segPtr->rowsAmount;
    }

    segPtr->rulesAmount       = 0xFFFFFFFF; /* not relevant */
    segPtr->ruleSize          = 0xFF;       /* not relevant */
    segPtr->baseLogicalIndex  = 0xFFFFFFFF; /* not relevant */
    rc = prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(
        tcamSegCfgPtr->segFreePrefTree, segPtr);
    if (rc != GT_OK)
    {
        /* should never occur */
        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_DBG_PRINT(
            ("prvCpssAvlItemInsert failed"));
        rcTotal = rc;
    }
    rc = prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(
        tcamSegCfgPtr->segFreeLuTree, segPtr);
    if (rc != GT_OK)
    {
        /* should never occur */
        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_DBG_PRINT(
            ("prvCpssAvlItemInsert failed"));
        rcTotal = rc;
    }
    return rcTotal;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableChainFreeSegmentWithSplit function
* @endinternal
*
* @brief   Chain free segment to trees with horizontal splitting.
*         Segment data in data pool, but not in both trees - will be added.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] segPtr                   - (pointer to) segment being added to trees.
*
* @retval GT_OK                    - on success
* @retval GT_NO_RESOURCE           - on no TCAM space
*/
GT_STATUS prvCpssDxChVirtualTcamDbSegmentTableChainFreeSegmentWithSplit
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC              *tcamSegCfgPtr,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC            *segPtr
)
{
    GT_STATUS                                                rc, rcTotal;   /* return code    */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC          *segPtr1;      /* segment        */
    GT_U32                                                   blocksInRow;   /* blocks In Row  */
    GT_U32                                                   blocksIdx;     /* blocks Index   */
    GT_U32                                                   lastBlocksIdx; /* blocks Index   */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT            segmentColumns; /* segment Columns */

    rcTotal = GT_OK;

    blocksInRow =
        (tcamSegCfgPtr->tcamColumnsAmount
         / tcamSegCfgPtr->lookupColumnAlignment);

    lastBlocksIdx = 0;
    for (blocksIdx = 0; (blocksIdx < blocksInRow); blocksIdx++)
    {
        segmentColumns = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
            segPtr->segmentColumnsMap, blocksIdx);
        if (segmentColumns == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E)
        {
            continue;
        }
        lastBlocksIdx = blocksIdx;
    }

    for (blocksIdx = 0; (blocksIdx < blocksInRow); blocksIdx++)
    {
        segmentColumns = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
            segPtr->segmentColumnsMap, blocksIdx);
        if (segmentColumns == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E)
        {
            continue;
        }
        if (lastBlocksIdx != blocksIdx)
        {
            /* not last horizontal block - new segment*/
            segPtr1 = cpssBmPoolBufGet(tcamSegCfgPtr->pVTcamMngPtr->segmentsPool);
            if (segPtr1 == NULL)
            {
                /* should never occur */
                prvCpssDxChVirtualTcamDbSegmentTableException();
                PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_DBG_PRINT(
                    ("cpssBmPoolBufGet failed"));
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
            }
            cpssOsMemCpy(
                segPtr1, segPtr, sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC));
        }
        else
        {
            /* last horizontal block - update segment*/
            segPtr1 = segPtr;
        }
        segPtr1->segmentColumnsMap =
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_EMPTY_CNS;
        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
            segPtr1->segmentColumnsMap, blocksIdx, segmentColumns);
        rc = prvCpssDxChVirtualTcamDbSegmentTableChainFreeSegment(
            tcamSegCfgPtr, segPtr1);
        if (rc != GT_OK)
        {
            rcTotal = rc;
        }
    }

    return rcTotal;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableFreeSpaceMergeIsNeeeded function
* @endinternal
*
* @brief   Check is Merge free space segments needed
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] segPtr1                  - (pointer to) first segment
* @param[in] segPtr2                  - (pointer to) second segment
*
* @retval GT_TRUE                  - merge needed
* @retval GT_FALSE                 - merge not needed
*/
static GT_BOOL prvCpssDxChVirtualTcamDbSegmentTableFreeSpaceMergeIsNeeeded
(
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC   *tcamSegCfgPtr,
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segPtr1,
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segPtr2
)
{
    GT_U32                  nextBlockBase;   /* next Block Base               */
    GT_U32                  w;               /* work data                     */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segPtr;  /* segment  */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT    segmentColumns1;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT    segmentColumns2;
    GT_U32                  blockHorzIdx1, blockHorzIdx2;

    if (segPtr2->rowsBase < segPtr1->rowsBase)
    {
        /* swap arguments */
        segPtr  = segPtr2;
        segPtr2 = segPtr1;
        segPtr1 = segPtr;
    }

    /* assumed segPtr2->rowsBase >= segPtr1->rowsBase */

    if (segPtr2->rowsBase > (segPtr1->rowsBase + segPtr1->rowsAmount))
    {
        /* vertical separated */
        return GT_FALSE;
    }

    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_FIND_FIRST_MAC(
        segPtr1->segmentColumnsMap, blockHorzIdx1, segmentColumns1);
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_FIND_FIRST_MAC(
        segPtr2->segmentColumnsMap, blockHorzIdx2, segmentColumns2);
    if (blockHorzIdx1 != blockHorzIdx2)
    {
        /* different blocks - assumed that segment is a part of one block */
        return GT_FALSE;
    }

    /* check vertical merge */
    if (segPtr2->rowsBase == (segPtr1->rowsBase + segPtr1->rowsAmount))
    {
        if (segmentColumns1 != segmentColumns2)
        {
            /* different columns */
            return GT_FALSE;
        }
        w = segPtr1->rowsBase + tcamSegCfgPtr->lookupRowAlignment;
        nextBlockBase = w - (w % tcamSegCfgPtr->lookupRowAlignment);
        if (segPtr2->rowsBase >= nextBlockBase)
        {
            /* different blocks */
            return GT_FALSE;
        }
        return GT_TRUE;
    }

    /* check horizontal merge */
    /* assumed segPtr2->rowsBase >= segPtr1->rowsBase */
    /* assumed segPtr2->rowsBase < (segPtr1->rowsBase + segPtr1->rowsAmount) */
    if (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_COMPLEMENT_MAC(
        segmentColumns1) == segmentColumns2)
    {
        return GT_TRUE;
    }
    return GT_FALSE;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableFreeSpaceMergeNeighbors function
* @endinternal
*
* @brief   Merge neighbor (horizontal and vertical) free space segments
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
*
* @retval GT_OK                    - on success
* @retval GT_NO_RESOURCE           - on no TCAM space
*/
GT_STATUS prvCpssDxChVirtualTcamDbSegmentTableFreeSpaceMergeNeighbors
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC          *tcamSegCfgPtr
)
{
    GT_STATUS                                       rc, rcTotal;  /* return code  */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segPtr1; /* segment  */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segPtr2; /* segment  */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segPtr;  /* segment  */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC bookmark; /* to find after update */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT   segmentColumns; /* segment Columns */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT   fullRowColumns; /* segment Columns */
    GT_U32                     blockHorzIdx ;   /* blocks horizontal Index       */
    GT_BOOL                    seg1Seeked;      /* bookmark seeked               */
    PRV_CPSS_AVL_TREE_PATH     seg1Path;        /* tree iterator                 */
    PRV_CPSS_AVL_TREE_PATH     seg2Path;        /* tree iterator                 */
    GT_VOID_PTR                dbEntryPtr;      /* pointer to entry in DB        */
    GT_BOOL                    mergeNeeded;     /* merge needed                  */
    GT_U32                     r1,r2,r3,r4;     /* work data                     */
    GT_U32                     bufUsed; /* work data                   */
    GT_U32                     low, high;       /* work data                     */

    rcTotal = GT_OK;

    fullRowColumns = prvCpssDxChVirtualTcamDbSegmentTableFullRowColumnsGet(tcamSegCfgPtr);

    /* first low of pair checked to merging */
    seg1Seeked = prvCpssAvlPathSeek(
            tcamSegCfgPtr->segFreeLuTree, PRV_CPSS_AVL_TREE_SEEK_FIRST_E,
            seg1Path, &dbEntryPtr /* use dedicated var to avoid warnings */);
    /* to prevent compiler warnings only */
    segPtr1 = dbEntryPtr;
    if (seg1Seeked == GT_FALSE)
    {
        return GT_OK;
    }
    cpssOsMemCpy(&bookmark, segPtr1, sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC));
    while (seg1Seeked != GT_FALSE)
    {
        segPtr1 = dbEntryPtr;

        mergeNeeded = GT_FALSE;
        cpssOsMemCpy(seg2Path, seg1Path, sizeof(PRV_CPSS_AVL_TREE_PATH));
        while (GT_FALSE != prvCpssAvlPathSeek(
                tcamSegCfgPtr->segFreeLuTree, PRV_CPSS_AVL_TREE_SEEK_NEXT_E,
                seg2Path, &dbEntryPtr))
        {
            segPtr2 = dbEntryPtr;
            if (segPtr2->rowsBase > (segPtr1->rowsBase + segPtr1->rowsAmount))
            {
                mergeNeeded = GT_FALSE;
                break;
            }

            mergeNeeded = prvCpssDxChVirtualTcamDbSegmentTableFreeSpaceMergeIsNeeeded(
                tcamSegCfgPtr, segPtr1, segPtr2);
            if (mergeNeeded == GT_FALSE)
            {
                continue;
            }

            cpssOsMemCpy(&bookmark, segPtr1, sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC));

            /* remove from trees (but not from pool) */
            prvCpssDxChVirtualTcamSegmentTableAvlItemRemove(
                tcamSegCfgPtr->segFreePrefTree, (GT_VOID*)segPtr1);
            prvCpssDxChVirtualTcamSegmentTableAvlItemRemove(
                tcamSegCfgPtr->segFreeLuTree, (GT_VOID*)segPtr1);
            prvCpssDxChVirtualTcamSegmentTableAvlItemRemove(
                tcamSegCfgPtr->segFreePrefTree, (GT_VOID*)segPtr2);
            prvCpssDxChVirtualTcamSegmentTableAvlItemRemove(
                tcamSegCfgPtr->segFreeLuTree, (GT_VOID*)segPtr2);

            prvCpssDxChVirtualTcamDbSegmentTableUpdateBlockFreeSpace(
                tcamSegCfgPtr, segPtr1, GT_TRUE/*decrease*/);
            prvCpssDxChVirtualTcamDbSegmentTableUpdateBlockFreeSpace(
                tcamSegCfgPtr, segPtr2, GT_TRUE/*decrease*/);

            if (segPtr2->rowsBase == (segPtr1->rowsBase + segPtr1->rowsAmount))
            {
                /* vertical merge */
                segPtr1->rowsAmount += segPtr2->rowsAmount;
                cpssBmPoolBufFree(tcamSegCfgPtr->pVTcamMngPtr->segmentsPool, segPtr2);
                /* insert fixed data */
                rc = prvCpssDxChVirtualTcamDbSegmentTableChainFreeSegment(
                    tcamSegCfgPtr, segPtr1);
                if (rc != GT_OK) rcTotal = rc;
            }
            else
            {
                /* horizontal merge */
                /* vertical bounds */
                r1 = segPtr1->rowsBase;
                r2 = segPtr2->rowsBase;
                r3 = segPtr1->rowsBase + segPtr1->rowsAmount;
                r4 = segPtr2->rowsBase + segPtr2->rowsAmount;
                /* assumed r1 <= r2 */
                /* vertical bounds of parts */
                /* before merged: r1  .. (r2 - 1)    */
                /* merged:        r2  .. (low - 1)   */
                /* after  merged: low .. (high - 1)  */
                if (r3 < r4)
                {
                    low = r3;
                    high = r4;
                }
                else
                {
                    low = r4;
                    high = r3;
                }
                bufUsed = 0;
                if (r1 < r2)
                {
                    /* freed : r1  .. (r2 - 1)    */
                    segPtr1->rowsAmount = r2 - r1;
                    /* insert fixed data */
                    rc = prvCpssDxChVirtualTcamDbSegmentTableChainFreeSegment(
                        tcamSegCfgPtr, segPtr1);
                    if (rc != GT_OK) rcTotal = rc;
                    bufUsed |= 1;
                }
                if (low < high)
                {
                    /* freed : low .. (high - 1)  */
                    if (r3 > r4)
                    {
                        segPtr2->segmentColumnsMap = segPtr1->segmentColumnsMap;
                    }
                    segPtr2->rowsBase = low;
                    segPtr2->rowsAmount = high - low;
                    /* insert fixed data */
                    rc = prvCpssDxChVirtualTcamDbSegmentTableChainFreeSegment(
                        tcamSegCfgPtr, segPtr2);
                    if (rc != GT_OK) rcTotal = rc;
                    bufUsed |= 2;
                }
                if ((bufUsed & 1) == 0)
                {
                    segPtr = segPtr1;
                    bufUsed |= 1;
                }
                else if ((bufUsed & 2) == 0)
                {
                    segPtr = segPtr2;
                    bufUsed |= 2;
                }
                else
                {
                    segPtr = cpssBmPoolBufGet(tcamSegCfgPtr->pVTcamMngPtr->segmentsPool);
                    if (segPtr == NULL)
                    {
                        /* should never occur */
                        prvCpssDxChVirtualTcamDbSegmentTableException();
                        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_DBG_PRINT(
                            ("cpssBmPoolBufGet failed"));
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
                    }
                    cpssOsMemCpy(segPtr, segPtr1, sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC));
                }
                /* merged:        r2  .. (low - 1)   */
                /* update columns in used block to be ALL */
                PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_FIND_FIRST_MAC(
                    segPtr->segmentColumnsMap, blockHorzIdx, segmentColumns);
                PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
                    segPtr->segmentColumnsMap, blockHorzIdx, fullRowColumns);
                segPtr->rowsBase = r2;
                segPtr->rowsAmount = low - r2;
                /* insert fixed data */
                rc = prvCpssDxChVirtualTcamDbSegmentTableChainFreeSegment(
                    tcamSegCfgPtr, segPtr);
                if (rc != GT_OK) rcTotal = rc;
                /* free unused buffers */
                if ((bufUsed & 1) == 0)
                {
                    cpssBmPoolBufFree(tcamSegCfgPtr->pVTcamMngPtr->segmentsPool, segPtr1);
                }
                if ((bufUsed & 2) == 0)
                {
                    cpssBmPoolBufFree(tcamSegCfgPtr->pVTcamMngPtr->segmentsPool, segPtr2);
                }
            }
            break;
        }

        /* next low of pair checked to merging */
        if (mergeNeeded == GT_FALSE)
        {
            seg1Seeked = prvCpssAvlPathSeek(
                    tcamSegCfgPtr->segFreeLuTree, PRV_CPSS_AVL_TREE_SEEK_NEXT_E,
                    seg1Path, &dbEntryPtr /* use dedicated var to avoid warnings */);
        }
        else
        {
            seg1Seeked = prvCpssAvlPathFind(
                tcamSegCfgPtr->segFreeLuTree, PRV_CPSS_AVL_TREE_FIND_MIN_GREATER_OR_EQUAL_E,
                &bookmark, seg1Path, &dbEntryPtr);
        }
    }

    return rcTotal;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableFreeSpaceMerge function
* @endinternal
*
* @brief   Merge free space segments
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
*
* @retval GT_OK                    - on success
* @retval GT_NO_RESOURCE           - on no TCAM space
*/
static GT_VOID prvCpssDxChVirtualTcamDbSegmentTableFreeSpaceMerge
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC     *tcamSegCfgPtr
)
{
    /* NOT HERE !!!                              */
    /* add moving all 10-byte and 20-byte rules  */
    /* after free 40-byte and 50-byte places     */
    /* to whole rows dragging along owner vTCAMs */
    prvCpssDxChVirtualTcamDbSegmentTableFreeSpaceMergeNeighbors(tcamSegCfgPtr);
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableFindFirstCoveringSegment function
* @endinternal
*
* @brief   Find first segment partially covering the given range of 10-byte rules
*         in lookup ordered tree.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] segLuTree                - lookup oredered tree
* @param[in] rowsBase                 - rows base of memory being partially covered
* @param[in] rowsAmount               - amount of rows in memory being partially covered
* @param[in] horzBlockIndex           - horizontal block index
* @param[in] segmentColumns           - columns in given horizontal block
*
* @param[out] outPath                  - path to the found segment.
* @param[out] outSegPtrPtr             - (pointer to)(pointer to) the found segment.
*
* @retval GT_OK                    - on success
* @retval other                    - on fail
*/
static GT_STATUS prvCpssDxChVirtualTcamDbSegmentTableFindFirstCoveringSegment
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC     *tcamSegCfgPtr,
    IN     PRV_CPSS_AVL_TREE_ID                              segLuTree,
    IN     GT_U32                                            rowsBase,
    IN     GT_U32                                            rowsAmount,
    IN     GT_U32                                            horzBlockIndex,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT     segmentColumns,
    OUT    PRV_CPSS_AVL_TREE_PATH                            outPath,
    OUT    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC   **outSegPtrPtr
)
{
    GT_U32                                                     lookupId;
    GT_U32                                                     rowsBound;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC            segPattern;
    GT_BOOL                                                    found;
    PRV_CPSS_AVL_TREE_PATH                                     startPath;
    PRV_CPSS_AVL_TREE_PATH                                     path;
    GT_VOID                                                    *segVoidPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC            *segPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC            *savedSegPtr;
    PRV_CPSS_AVL_TREE_PATH                                     savedPath;
    GT_U32                                                     blocksInRow;
    GT_U32                                                     blockIndex;
    GT_U32                                                     blockRowsBase;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT              foundSegmentColumns;
    GT_U32                                                     col10byteBitmap;
    GT_U32                                                     foundCol10byteBitmap;

    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_BITMAP_OF_COLUMNS_MAC(
        segmentColumns, col10byteBitmap);

    rowsBound = (rowsBase + rowsAmount);
    if (rowsBound > tcamSegCfgPtr->tcamRowsAmount)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    blocksInRow =
        (tcamSegCfgPtr->tcamColumnsAmount
         / tcamSegCfgPtr->lookupColumnAlignment);
    if (horzBlockIndex > blocksInRow)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    blockIndex =
        (((rowsBase / tcamSegCfgPtr->lookupRowAlignment)
         * blocksInRow) + horzBlockIndex);
    lookupId = tcamSegCfgPtr->blockInfArr[blockIndex].lookupId;

    blockRowsBase =
        (rowsBase - (rowsBase % tcamSegCfgPtr->lookupRowAlignment));

    cpssOsMemSet(&segPattern, 0, sizeof(segPattern));
    segPattern.lookupId          = lookupId;
    segPattern.rowsBase          = rowsBase;
    segPattern.rowsAmount        = 1;
    segPattern.segmentColumnsMap =
        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_EMPTY_CNS;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
        segPattern.segmentColumnsMap, horzBlockIndex, segmentColumns);

    /* the first approximation */
    found = prvCpssAvlPathFind(
        segLuTree,
        PRV_CPSS_AVL_TREE_FIND_MAX_LESS_OR_EQUAL_E,
        &segPattern, path, &segVoidPtr);
    if (found == GT_FALSE)
    {
        found = prvCpssAvlPathSeek(
                segLuTree,
                PRV_CPSS_AVL_TREE_SEEK_FIRST_E,
                path, &segVoidPtr);
    }
    if (found == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    /* save found path */
    cpssOsMemCpy(startPath, path, sizeof(PRV_CPSS_AVL_TREE_PATH));

    /* look for first segment that covers the given rowsBase */
    /* search to low direction                               */
    savedSegPtr = NULL;
    do
    {
        segPtr = segVoidPtr;

        if (segPtr->lookupId != lookupId) break;

        /* another horisontal blocks row reached */
        if (segPtr->rowsBase < blockRowsBase) break;

        /* more than all relevant range */
        if (segPtr->rowsBase >= rowsBound) continue;

        foundSegmentColumns =
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
                segPtr->segmentColumnsMap, horzBlockIndex);
        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_BITMAP_OF_COLUMNS_MAC(
            foundSegmentColumns, foundCol10byteBitmap);
        /* another columns in the given horizontal block */
        if ((col10byteBitmap & foundCol10byteBitmap) == 0) continue;

        /* save correct result that possibly is the best */
        /* assumed (segPtr->rowsBase < rowsBound) */
        if ((segPtr->rowsBase + segPtr->rowsAmount) > rowsBase)
        {
            savedSegPtr = segPtr;
            cpssOsMemCpy(savedPath, path, sizeof(PRV_CPSS_AVL_TREE_PATH));
        }

        if (segPtr->rowsBase <= rowsBase)
        {
            if ((segPtr->rowsBase + segPtr->rowsAmount) > rowsBase)
            {
                cpssOsMemCpy(outPath, path, sizeof(PRV_CPSS_AVL_TREE_PATH));
                *outSegPtrPtr = segPtr;
                return GT_OK;
            }
            break;
        }
    } while (GT_FALSE != prvCpssAvlPathSeek(
            segLuTree,
            PRV_CPSS_AVL_TREE_SEEK_PREVIOUS_E,
            path, &segVoidPtr));

    if (savedSegPtr != NULL)
    {
        outPath       = savedPath;
        *outSegPtrPtr = savedSegPtr;
        return GT_OK;
    }

    cpssOsMemCpy(path, startPath, sizeof(PRV_CPSS_AVL_TREE_PATH));

    while (GT_FALSE != prvCpssAvlPathSeek(
            segLuTree,
            PRV_CPSS_AVL_TREE_SEEK_NEXT_E,
            path, &segVoidPtr))
    {
        segPtr = segVoidPtr;

        if (segPtr->lookupId != lookupId) break;

        if (segPtr->rowsBase >= rowsBound) break;

        foundSegmentColumns =
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
                segPtr->segmentColumnsMap, horzBlockIndex);
        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_BITMAP_OF_COLUMNS_MAC(
            foundSegmentColumns, foundCol10byteBitmap);
        /* another columns in the given horizontal block */
        if ((col10byteBitmap & foundCol10byteBitmap) == 0) continue;

        if ((segPtr->rowsBase + segPtr->rowsAmount) > rowsBase)
        {
            cpssOsMemCpy(outPath, path, sizeof(PRV_CPSS_AVL_TREE_PATH));
            *outSegPtrPtr = segPtr;
            return GT_OK;
        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableRemoveMemoryFromFree function
* @endinternal
*
* @brief   Remove memory from free space DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] lookupId                 - lookup Id (needed when marking busy parts of free block)
* @param[in] rowsBase                 - rows base of memory removed from free DB
* @param[in] rowsAmount               - amount of rows of memory removed from free DB
* @param[in] segmentColumnsMap        - columns map used by memory removed from free DB
*
* @retval GT_OK                    - on success
* @retval other                    - on fail
*/
GT_STATUS prvCpssDxChVirtualTcamDbSegmentTableRemoveMemoryFromFree
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC             *tcamSegCfgPtr,
    IN     GT_U32                                                     lookupId,
    IN     GT_U32                                                     rowsBase,
    IN     GT_U32                                                     rowsAmount,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP          segmentColumnsMap
)
{
    GT_STATUS                                                  rc;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT              segmentColumns;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT              fullRowColumns;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT              foundColumns;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP          checkColumnsMap;
    PRV_CPSS_AVL_TREE_PATH                                     path;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC            *segPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC            *segPtr1;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC            segCopy;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_BLOCK_INFO_STC       *blockPtr;
    GT_U32                                                     bitmap;
    GT_U32                                                     blocksInRow;
    GT_U32                                                     blockHorzIdx ;
    GT_U32                                                     blockIdx;
    GT_U32                                                     segBound;
    GT_U32                                                     bound;
    GT_U32                                                     cropRowsBase;
    GT_U32                                                     cropRowsAmount;
    GT_BOOL                                                    putCroppedBack;

    fullRowColumns = prvCpssDxChVirtualTcamDbSegmentTableFullRowColumnsGet(tcamSegCfgPtr);

    /* all checks needed - one of unsigned values may be negative */
    if ((rowsBase >= tcamSegCfgPtr->tcamRowsAmount)
        || (rowsAmount > tcamSegCfgPtr->tcamRowsAmount)
        || ((rowsBase + rowsAmount) > tcamSegCfgPtr->tcamRowsAmount))
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(
            GT_BAD_PARAM, "RemoveMemoryFromFree: wrong rowsBase and rowsAmount");
    }
    if (lookupId ==
        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_LOOKUP_FREE_BLOCK_CNS)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(
            GT_BAD_PARAM, "RemoveMemoryFromFree: wrong lookupId");
    }

    blocksInRow =
        (tcamSegCfgPtr->tcamColumnsAmount
         / tcamSegCfgPtr->lookupColumnAlignment);

    bitmap = 0;
    for (blockHorzIdx = 0; (blockHorzIdx < blocksInRow); blockHorzIdx++)
    {
        segmentColumns =
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
                segmentColumnsMap, blockHorzIdx);
        if (segmentColumns
            != PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E)
        {
            bitmap |= (1 << blockHorzIdx);
        }
    }

    while (bitmap != 0)
    {
        for (blockHorzIdx = 0; (blockHorzIdx < blocksInRow); blockHorzIdx++)
        {
            if ((bitmap & (1 << blockHorzIdx)) == 0) continue;

            segmentColumns =
                PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
                    segmentColumnsMap, blockHorzIdx);
            while (1)
            {
                rc = prvCpssDxChVirtualTcamDbSegmentTableFindFirstCoveringSegment(
                    tcamSegCfgPtr, tcamSegCfgPtr->segFreeLuTree,
                    rowsBase, rowsAmount, blockHorzIdx, segmentColumns,
                    path, &segPtr);
                if (rc != GT_OK)
                {
                    /* no more in this horizontal block */
                    bitmap &= (~ (1 << blockHorzIdx));
                    break;
                }

                if (segPtr->lookupId ==
                    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_LOOKUP_FREE_BLOCK_CNS)
                {
                    blockIdx =
                        ((blocksInRow *
                          (segPtr->rowsBase / tcamSegCfgPtr->lookupRowAlignment)) +
                        blockHorzIdx);
                    if (blockIdx >= tcamSegCfgPtr->blocksAmount)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                    }

                    blockPtr                 = &(tcamSegCfgPtr->blockInfArr[blockIdx]);
                    if (blockPtr->lookupId !=
                        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_LOOKUP_FREE_BLOCK_CNS)
                    {
                        prvCpssDxChVirtualTcamDbSegmentTableException();
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                    }
                    if (tcamSegCfgPtr->lookupMapFuncPtr != NULL)
                    {
                        rc = tcamSegCfgPtr->lookupMapFuncPtr(
                            tcamSegCfgPtr->cookiePtr,
                            blockPtr->rowsBase,
                            blockPtr->columnsBase,
                            lookupId);
                        if (rc != GT_OK)
                        {
                            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_DBG_PRINT(
                                ("lookupMapFuncPtr returned %d", rc));
                            return rc;
                        }
                        blockPtr->lookupId = lookupId;
                    }
                    /* update segment lookup Id */
                    prvCpssDxChVirtualTcamSegmentTableAvlItemRemove(
                        tcamSegCfgPtr->segFreeLuTree, (GT_VOID*)segPtr);
                    prvCpssDxChVirtualTcamSegmentTableAvlItemRemove(
                        tcamSegCfgPtr->segFreePrefTree, (GT_VOID*)segPtr);
                    segPtr->lookupId = lookupId;
                    rc = prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(
                        tcamSegCfgPtr->segFreeLuTree, segPtr);
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "failed on prvCpssAvlItemInsert");
                    }
                    rc = prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(
                        tcamSegCfgPtr->segFreePrefTree, segPtr);
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "failed on prvCpssAvlItemInsert");
                    }
                }

                cropRowsBase =
                    (rowsBase > segPtr->rowsBase) ? rowsBase : segPtr->rowsBase;
                segBound = (segPtr->rowsBase + segPtr->rowsAmount);
                bound    = (rowsBase + rowsAmount);
                cropRowsAmount =
                    (((segBound < bound) ? segBound : bound) - cropRowsBase);

                foundColumns =
                    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
                        segPtr->segmentColumnsMap, blockHorzIdx);
                if ((foundColumns != fullRowColumns) && (foundColumns != segmentColumns))
                {
                    /* not supported or not relevant */
                     prvCpssDxChVirtualTcamDbSegmentTableException();
                     PRV_DBG_TRACE(("RemoveMemoryFromFree - not supported found columns."));
                    bitmap &= (~ (1 << blockHorzIdx));
                    break;
                }

                putCroppedBack = GT_FALSE; /* default */
                /* check that segment has exactly the same columns that should be freed */
                if (segPtr->segmentColumnsMap != segmentColumnsMap)
                {
                    putCroppedBack = GT_TRUE;
                }

                rc = prvCpssDxChVirtualTcamDbSegmentTableCropSegmentVertical(
                    tcamSegCfgPtr, tcamSegCfgPtr->segFreePrefTree, tcamSegCfgPtr->segFreeLuTree,
                    segPtr, cropRowsBase, cropRowsAmount, putCroppedBack,
                    NULL /*lowSegPtrPtr*/, NULL /*highSegPtrPtr*/);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }

                /* need horizontal split after vertical split */
                if (putCroppedBack != GT_FALSE)
                {
                    /* decreased per block statistic source */
                    cpssOsMemCpy(&segCopy, segPtr, sizeof(segCopy));
                    segCopy.segmentColumnsMap =
                        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_EMPTY_CNS;
                    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
                        segCopy.segmentColumnsMap, blockHorzIdx, foundColumns);

                    checkColumnsMap = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_EMPTY_CNS;
                    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
                        checkColumnsMap, blockHorzIdx, segmentColumns);
                    rc = prvCpssDxChVirtualTcamDbSegmentTableCropSegmentHorizontal(
                        tcamSegCfgPtr, tcamSegCfgPtr->segFreePrefTree, tcamSegCfgPtr->segFreeLuTree,
                        segPtr, checkColumnsMap, GT_FALSE /*putBack*/, &segPtr1 /*remainSegPtrPtr*/);
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                    }
                    /* update per block statistics */
                    prvCpssDxChVirtualTcamDbSegmentTableUpdateBlockFreeSpace(
                        tcamSegCfgPtr, &segCopy, GT_TRUE/*decrease*/);
                    if (segPtr1 != NULL)
                    {
                        prvCpssDxChVirtualTcamDbSegmentTableUpdateBlockFreeSpace(
                            tcamSegCfgPtr, segPtr1, GT_FALSE/*decrease*/);
                    }
                }
                else
                {
                    /* update per block statistics */
                    prvCpssDxChVirtualTcamDbSegmentTableUpdateBlockFreeSpace(
                        tcamSegCfgPtr, segPtr, GT_TRUE/*decrease*/);
                }

                cpssBmPoolBufFree(tcamSegCfgPtr->pVTcamMngPtr->segmentsPool, segPtr);
            }
        }
    }

    return GT_OK;
}

/* Per block row bitmap functions prvCpssDxChVirtualTcamDbSegmentTableBlockRowBitmap...*/

/*******************************************************************************
* prvCpssDxChVirtualTcamDbSegmentTableBlockRowBitmapAllocate
*
* DESCRIPTION:
*       Allocate per Block Row bitmap.
*
* APPLICABLE DEVICES:
*        xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* INPUTS:
*       tcamSegCfgPtr    - (pointer to) segment configuration information.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       Address of bitmap or NULL.
*
* COMMENTS:
*
*******************************************************************************/
GT_U32* prvCpssDxChVirtualTcamDbSegmentTableBlockRowBitmapAllocate
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr
)
{
    GT_U32 horzBlocksInRow;
    GT_U32 blockRowsAmount;
    GT_U32 bitmapSize;

    horzBlocksInRow =
        (tcamSegCfgPtr->tcamColumnsAmount
        / tcamSegCfgPtr->lookupColumnAlignment);
    blockRowsAmount = (horzBlocksInRow * tcamSegCfgPtr->tcamRowsAmount);
    bitmapSize = (((blockRowsAmount + 31) / 32) * sizeof(GT_U32));
    return (GT_U32*)cpssOsMalloc(bitmapSize);
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableBlockRowBitmapFree function
* @endinternal
*
* @brief   Free per Block Row bitmap.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] bitmapPtr                - (pointer to) Block Row bitmap
*                                       None.
*/
GT_VOID prvCpssDxChVirtualTcamDbSegmentTableBlockRowBitmapFree
(
    IN     GT_U32                             *bitmapPtr
)
{
   cpssOsFree(bitmapPtr);
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableBlockRowBitmapReset function
* @endinternal
*
* @brief   Reset per Block Row bitmap.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] bitmapPtr                - (pointer to) Block Row bitmap
*                                       None.
*/
GT_VOID prvCpssDxChVirtualTcamDbSegmentTableBlockRowBitmapReset
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr,
    IN     GT_U32                             *bitmapPtr
)
{
    GT_U32 horzBlocksInRow;
    GT_U32 blockRowsAmount;
    GT_U32 bitmapSize;

    horzBlocksInRow =
        (tcamSegCfgPtr->tcamColumnsAmount
        / tcamSegCfgPtr->lookupColumnAlignment);
    blockRowsAmount = (horzBlocksInRow * tcamSegCfgPtr->tcamRowsAmount);
    bitmapSize = (((blockRowsAmount + 31) / 32) * sizeof(GT_U32));
    cpssOsMemSet(bitmapPtr, 0, bitmapSize);
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableBlockRowBitmapSet function
* @endinternal
*
* @brief   Set bit in per Block Row bitmap.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] bitmapPtr                - (pointer to) Block Row bitmap
* @param[in] rowIndex                 - row index
* @param[in] horzBlockIndex           - horzontal block index
* @param[in] value                    - 0 or 1, any other converted to 1
*                                       None.
*/
GT_VOID prvCpssDxChVirtualTcamDbSegmentTableBlockRowBitmapSet
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr,
    IN     GT_U32                             *bitmapPtr,
    IN     GT_U32                             rowIndex,
    IN     GT_U32                             horzBlockIndex,
    IN     GT_U32                             value
)
{
    GT_U32 horzBlocksInRow;
    GT_U32 bitFullIndex;
    GT_U32 bitIndex;
    GT_U32 wordIndex;
    GT_U32 bitMask;

    horzBlocksInRow =
        (tcamSegCfgPtr->tcamColumnsAmount
        / tcamSegCfgPtr->lookupColumnAlignment);
    if (rowIndex >= tcamSegCfgPtr->tcamRowsAmount)
    {
        return;
    }
    if (horzBlockIndex >= horzBlocksInRow)
    {
        return;
    }

    bitFullIndex = ((rowIndex * horzBlocksInRow) + horzBlockIndex);
    wordIndex    = bitFullIndex / 32;
    bitIndex     = bitFullIndex % 32;
    bitMask      = (1 << bitIndex);
    if (value)
    {
        bitmapPtr[wordIndex] |= bitMask;
    }
    else
    {
        bitmapPtr[wordIndex] &= (~ bitMask);
    }
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableBlockRowBitmapGet function
* @endinternal
*
* @brief   Get bit in per Block Row bitmap.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] bitmapPtr                - (pointer to) Block Row bitmap
* @param[in] rowIndex                 - row index
* @param[in] horzBlockIndex           - horzontal block index
*
* @retval value                    - 0 or 1
*/
static GT_U32 prvCpssDxChVirtualTcamDbSegmentTableBlockRowBitmapGet
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr,
    IN     GT_U32                             *bitmapPtr,
    IN     GT_U32                             rowIndex,
    IN     GT_U32                             horzBlockIndex
)
{
    GT_U32 horzBlocksInRow;
    GT_U32 bitFullIndex;
    GT_U32 bitIndex;
    GT_U32 wordIndex;

    horzBlocksInRow =
        (tcamSegCfgPtr->tcamColumnsAmount
        / tcamSegCfgPtr->lookupColumnAlignment);
    if (rowIndex >= tcamSegCfgPtr->tcamRowsAmount)
    {
        return 0;
    }
    if (horzBlockIndex >= horzBlocksInRow)
    {
        return 0;
    }

    bitFullIndex = ((rowIndex * horzBlocksInRow) + horzBlockIndex);
    wordIndex    = bitFullIndex / 32;
    bitIndex     = bitFullIndex % 32;
    return ((bitmapPtr[wordIndex] >> bitIndex) & 1);
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableBlockRowBitmapSetForFreeRows function
* @endinternal
*
* @brief   Set bits of per Block Row bitmap for Free TCAM rows of given type.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] bitmapPtr                - (pointer to) Block Row bitmap
* @param[in] lookupId                 - lookup id
* @param[in] freeSpaceType            - type of ree space
* @param[in] value                    - 0 or 1, any other converted to 1
*                                       None.
*/
static GT_VOID prvCpssDxChVirtualTcamDbSegmentTableBlockRowBitmapSetForFreeRows
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC  *tcamSegCfgPtr,
    IN     GT_U32                                         *bitmapPtr,
    IN     GT_U32                                         lookupId,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT  freeSpaceType,
    IN     GT_U32                                         value
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC  segPattern;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segPtr;
    PRV_CPSS_AVL_TREE_PATH                           path;
    PRV_CPSS_AVL_TREE_FIND_ENT                       findType;
    GT_VOID                                          *dbEntryPtr;
    GT_BOOL                                          seeked;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT    segmentColumns;
    GT_U32                                           horzBlocksInRow;
    GT_U32                                           horzBlockIdx;
    GT_U32                                           rowIdx;
    GT_U32                                           rowHigh;

    horzBlocksInRow =
        (tcamSegCfgPtr->tcamColumnsAmount
        / tcamSegCfgPtr->lookupColumnAlignment);

    cpssOsMemSet(&segPattern, 0, sizeof(segPattern));
    segPattern.lookupId = lookupId;
    segmentColumns = prvCpssDxChVirtualTcamDbSegmentTableFullRowColumnsGet(tcamSegCfgPtr);
    for (horzBlockIdx = 0; (horzBlockIdx < horzBlocksInRow); horzBlockIdx++)
    {
        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
            segPattern.segmentColumnsMap, horzBlockIdx, segmentColumns);
    }
    findType = PRV_CPSS_AVL_TREE_FIND_MIN_GREATER_OR_EQUAL_E;

    for (seeked = prvCpssAvlPathFind(
            tcamSegCfgPtr->segFreeLuTree, findType,
            &segPattern, path, &dbEntryPtr);
        (GT_FALSE != seeked);
        seeked = prvCpssAvlPathSeek(
            tcamSegCfgPtr->segFreeLuTree,
            PRV_CPSS_AVL_TREE_SEEK_NEXT_E, path, &dbEntryPtr))
    {
        segPtr = dbEntryPtr;
        if (segPtr->lookupId != lookupId)
        {
            break;
        }

        for (horzBlockIdx = 0; (horzBlockIdx < horzBlocksInRow); horzBlockIdx++)
        {
            segmentColumns = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
                segPtr->segmentColumnsMap, horzBlockIdx);
            if (segmentColumns == freeSpaceType)
            {
                rowHigh = segPtr->rowsBase + segPtr->rowsAmount;
                for (rowIdx = segPtr->rowsBase; (rowIdx < rowHigh); rowIdx++)
                {
                    prvCpssDxChVirtualTcamDbSegmentTableBlockRowBitmapSet(
                        tcamSegCfgPtr, bitmapPtr, rowIdx, horzBlockIdx, value);
                }
            }
        }
    }
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableCountFreeSpaceInUnits function
* @endinternal
*
* @brief   Count free space for in minimal record units.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] lookupId                 - lookup Id
*                                       used to fast estimate possibility of allocations.
*/
GT_U32 prvCpssDxChVirtualTcamDbSegmentTableCountFreeSpaceInUnits
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr,
    IN     GT_U32                             lookupId
)
{
    GT_U32      freeSpace;
    GT_U32      i, j;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_BLOCK_INFO_STC *blockPtr;

    freeSpace = 0;
    for (i = 0; (i < tcamSegCfgPtr->blocksAmount); i++)
    {
        blockPtr = &(tcamSegCfgPtr->blockInfArr[i]);
        if (blockPtr->lookupId != lookupId)
        {
            continue;
        }
        for (j = 0; (j < PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LAST_E); j++)
        {
            if (blockPtr->freeSpace[j] == 0) continue;
            freeSpace +=
                blockPtr->freeSpace[j] *
                prvCpssDxChVirtualTcamDbSegmentTableColumnsToUnitsAmount(j);
        }
    }

    return freeSpace;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableCountAvailableSpace function
* @endinternal
*
* @brief   Count available free space for allocation.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] lookupId                 - lookup Id
* @param[in] ruleSize                 - rule Size (measured in 10-byte units)
*                                       amount of rules that can be allocated
*                                       without moving rules of other vTCAMs
*/
static GT_U32 prvCpssDxChVirtualTcamDbSegmentTableCountAvailableSpace
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr,
    IN     GT_U32                             lookupId,
    IN     GT_U32                             ruleSize
)
{
    GT_U32      freeSpace[PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LAST_E];
    GT_U32      i,j; /* loop counters */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_BLOCK_INFO_STC *blockPtr;
    GT_U32      availableRulePlaces;
    GT_U32      placeCapacity;

    for (j = 0; (j < PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LAST_E); j++)
    {
        freeSpace[j] = 0;
    }

    for (i = 0; (i < tcamSegCfgPtr->blocksAmount); i++)
    {
        blockPtr = &(tcamSegCfgPtr->blockInfArr[i]);
        if (blockPtr->lookupId != lookupId)
        {
            continue;
        }
        for (j = 0; (j < PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LAST_E); j++)
        {
            freeSpace[j] += blockPtr->freeSpace[j];
        }
    }

    availableRulePlaces = 0;
    for (j = 0; (j < PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LAST_E); j++)
    {
        placeCapacity = prvCpssDxChVirtualTcamDbSegmentTableRowCapacity(
                j, ruleSize);
            availableRulePlaces += (freeSpace[j] * placeCapacity);
    }

    return availableRulePlaces;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableCountAvailableSpaceFor80ByteRules function
* @endinternal
*
* @brief   Count available free space for allocation of 80 byte rules in SIP5 devices.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] lookupId                 - lookup Id
*                                       amount of rules that can be allocated
*                                       without moving rules of other vTCAMs
*/
static GT_U32 prvCpssDxChVirtualTcamDbSegmentTableCountAvailableSpaceFor80ByteRules
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr,
    IN     GT_U32                             lookupId
)
{
    GT_BOOL                                         found0;
    GT_BOOL                                         found1;
    GT_VOID                                         *dataPtr0;
    GT_VOID                                         *dataPtr1;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC segKey0;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC segKey1;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segPtr0;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segPtr1;
    PRV_CPSS_AVL_TREE_PATH                          path0;
    PRV_CPSS_AVL_TREE_PATH                          path1;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT   columns1;
    GT_U32                                            rowsBase;
    GT_U32                                            rowsEnd;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT    segmentColumns;
    GT_U32                                           horzBlocksInRow;
    GT_U32                                           horzBlockIdx;

    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_BLOCK_INFO_STC *blockPtr1;
    GT_U32      availableRulePlaces;

    horzBlocksInRow =
        (tcamSegCfgPtr->tcamColumnsAmount
        / tcamSegCfgPtr->lookupColumnAlignment);

    /* start search of free rows in provided block */
    cpssOsMemSet(&segKey0, 0, sizeof(segKey0));
    segKey0.lookupId = lookupId;
    segmentColumns = prvCpssDxChVirtualTcamDbSegmentTableFullRowColumnsGet(tcamSegCfgPtr);
    for (horzBlockIdx = 0; (horzBlockIdx < horzBlocksInRow); horzBlockIdx++)
    {
        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
            segKey0.segmentColumnsMap, horzBlockIdx, segmentColumns);
    }
    found0 = prvCpssAvlPathFind(
            tcamSegCfgPtr->segFreeLuTree,
            PRV_CPSS_AVL_TREE_FIND_MIN_GREATER_OR_EQUAL_E,
            &segKey0, path0, &dataPtr0);
    segPtr0 = dataPtr0;

    availableRulePlaces = 0;
    while ((found0) && (segPtr0->lookupId == lookupId))
    {

        /* Look for pairs of free space:
         *   the first (0): a full free row in a low block
         *   the second (1): either free row or _01 free row at its right.
         */
        if (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
                                        segPtr0->segmentColumnsMap, 0) !=
                PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E)
        {
            found0 = prvCpssAvlPathSeek(
                    tcamSegCfgPtr->segFreeLuTree,
                    PRV_CPSS_AVL_TREE_SEEK_NEXT_E,
                    path0, &dataPtr0);
            segPtr0 = dataPtr0;
            continue;
        }

        /* Found free space at low block, look at its right */

        blockPtr1 = &tcamSegCfgPtr->blockInfArr[((segPtr0->rowsBase/
                                                    tcamSegCfgPtr->lookupRowAlignment)
                                                        * 2) + 1];
        if (blockPtr1->lookupId == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_LOOKUP_FREE_BLOCK_CNS)
        {
            /* The block at its right is wholly empty */
            availableRulePlaces += segPtr0->rowsAmount;
        }
        else
        {
            cpssOsMemSet(&segKey1, 0, sizeof(segKey1));
            segKey1.lookupId = lookupId;
            segKey1.rowsBase = segPtr0->rowsBase + segPtr0->rowsAmount - 1;
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
                    segKey1.segmentColumnsMap,
                    1,
                    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E);
            found1 = prvCpssAvlPathFind(
                    tcamSegCfgPtr->segFreeLuTree,
                    PRV_CPSS_AVL_TREE_FIND_MAX_LESS_OR_EQUAL_E,
                    &segKey1, path1, &dataPtr1);
            segPtr1 = dataPtr1;

            while (found1 &&
                    (segPtr1->lookupId == lookupId) &&
                    ((segPtr1->rowsBase + segPtr1->rowsAmount) > segPtr0->rowsBase))
            {
                columns1 = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
                                            segPtr1->segmentColumnsMap, 1);
                if ((columns1 == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E) ||
                    (columns1 == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E))
                {
                    /* Found suitable free space just at the right of first free segment. */
                    /* The intersection between both is useable */
                    if (segPtr1->rowsBase < segPtr0->rowsBase)
                    {
                        rowsBase = segPtr0->rowsBase;
                    }
                    else
                    {
                        rowsBase = segPtr1->rowsBase;
                    }
                    if ((segPtr1->rowsBase + segPtr1->rowsAmount) <
                        (segPtr0->rowsBase + segPtr0->rowsAmount))
                    {
                        rowsEnd = (segPtr1->rowsBase + segPtr1->rowsAmount);
                    }
                    else
                    {
                        rowsEnd = (segPtr0->rowsBase + segPtr0->rowsAmount);
                    }
                    availableRulePlaces += rowsEnd - rowsBase;
                }

                found1 = prvCpssAvlPathSeek(
                        tcamSegCfgPtr->segFreeLuTree,
                        PRV_CPSS_AVL_TREE_SEEK_PREVIOUS_E,
                        path1, &dataPtr1);
                segPtr1 = dataPtr1;
            }
        }

        found0 = prvCpssAvlPathSeek(
                tcamSegCfgPtr->segFreeLuTree,
                PRV_CPSS_AVL_TREE_SEEK_NEXT_E,
                path0, &dataPtr0);
        segPtr0 = dataPtr0;
    }

    return availableRulePlaces;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableCountFreeableSpaceByOtherLookups function
* @endinternal
*
* @brief   Count space currently unused by other lookups, rounded down to full blocks.
*         Return also a list of lookups which can be compressed into fewer blocks.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] lookupId                 - lookup Id
* @param[in] ruleSize                 - ruleSize
*
* @param[out] savedBlocksPtr[PRV_CPSS_DXCH_VIRTUAL_TCAM_MAX_HITNUM] - (array of pointers to) representative blocks for each lookup
*                                      on which gain can be achieved, i.e. more than one whole block free.
*                                       amount of rows unused by blocks assigned to all lookups other than lookupId.
*                                       (rounded down to a multiple of a whole block)
*
* @note savedBlocksPtr[][] points into pVTcamMngPtr->blockInfArr, so that block number
*       can be easily computed by subtracting pVTcamMngPtr->blockInfArr.
*
*/
static GT_U32 prvCpssDxChVirtualTcamDbSegmentTableCountFreeableSpaceByOtherLookups
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr,
    IN     GT_U32                             lookupId,
    IN     GT_U32                             ruleSize,
    OUT    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_BLOCK_INFO_STC
                                *savedBlocksPtr[PRV_CPSS_DXCH_VIRTUAL_TCAM_MAX_HITNUM]
                                            [PRV_CPSS_DXCH_VIRTUAL_TCAM_MAX_CLIENTGROUP]
)
{
    GT_U32      freeSpace[PRV_CPSS_DXCH_VIRTUAL_TCAM_MAX_HITNUM]
                         [PRV_CPSS_DXCH_VIRTUAL_TCAM_MAX_CLIENTGROUP];
    GT_U32      hitNum;
    GT_U32      clientGroup;
    GT_U32      i, j;  /* loop counter */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_BLOCK_INFO_STC *blockPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_BLOCK_INFO_STC *representativeBlockPtr;
    GT_U32      freeSpaceInRows;
    GT_U32      freeableRows;
    GT_U32      freeableRules;

    cpssOsMemSet (&freeSpace[0][0], 0, sizeof(freeSpace));

    for (i = 0; (i < tcamSegCfgPtr->blocksAmount); i++)
    {
        blockPtr = &(tcamSegCfgPtr->blockInfArr[i]);
        if ((blockPtr->lookupId != PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_LOOKUP_FREE_BLOCK_CNS) &&
            (blockPtr->lookupId != lookupId))
        {
            hitNum = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_LOOKUP_ID_TO_HIT_NUMBER_MAC(blockPtr->lookupId);
            if (hitNum >= PRV_CPSS_DXCH_VIRTUAL_TCAM_MAX_HITNUM)
            {
                prvCpssDxChVirtualTcamDbSegmentTableException();
                return 0;
            }
            clientGroup = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_LOOKUP_ID_TO_CLIENT_GROUP_MAC(blockPtr->lookupId);
            if (clientGroup >= PRV_CPSS_DXCH_VIRTUAL_TCAM_MAX_CLIENTGROUP)
            {
                prvCpssDxChVirtualTcamDbSegmentTableException();
                return 0;
            }
            savedBlocksPtr[hitNum][clientGroup] = blockPtr;
            /* Compute amount of free rules */
            freeSpace[hitNum][clientGroup] +=
                (1 * blockPtr->freeSpace[PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_5_E]) +
                (2 * blockPtr->freeSpace[PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_45_E]) +
                (4 * blockPtr->freeSpace[PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_0123_E]) +
                (4 * blockPtr->freeSpace[PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_2345_E]) +
                (5 * blockPtr->freeSpace[PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01234_E]) +
                (6 * blockPtr->freeSpace[PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E]);
            freeSpace[hitNum][clientGroup] +=
                (1 * blockPtr->freeSpace[PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_3_E]) +
                (3 * blockPtr->freeSpace[PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_012_E]) +
                (4 * blockPtr->freeSpace[PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_ALL_E]) +
                (1 * blockPtr->freeSpace[PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_TTI_ALL_E]);
        }
    }

    /* Sum up freeable space for all other lookups, after round down to a whole block */
    freeableRows = 0;
    for (i = 0; i < PRV_CPSS_DXCH_VIRTUAL_TCAM_MAX_HITNUM; i++)
    {
        for (j = 0; j < PRV_CPSS_DXCH_VIRTUAL_TCAM_MAX_CLIENTGROUP; j++)
        {
            representativeBlockPtr = savedBlocksPtr[i][j];
            if (representativeBlockPtr != NULL)
            {
                freeSpaceInRows = freeSpace[i][j] / 6;
                if (freeSpaceInRows < representativeBlockPtr->rowsAmount)
                {
                    /* Don't count this lookup since no gain will be achieved */
                    savedBlocksPtr[i][j] = NULL;
                }
                else
                {
                    /* This lookup can be compressed into fewer blocks
                     *  - return some block (don't care which one as long as same lookup)
                     *      and update total */
                    freeableRows += freeSpaceInRows -
                                        (freeSpaceInRows %
                                            representativeBlockPtr->rowsAmount);
                }
            }
        }
    }

    /* Convert rows to rules */
    if (ruleSize == 8)
    {
        freeableRules = freeableRows / 2;
    }
    else
    {
        freeableRules = freeableRows * (6 / ruleSize);
    }

    return freeableRules;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableCountFreeBlocksSpace function
* @endinternal
*
* @brief   Count space in free blocks (not mapped to any lookups).
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] ruleSize                 - rule Size (measured in 10-byte units)
*                                       amount of rules that can be allocated
*                                       without moving rules of other vTCAMs
*/
static GT_U32 prvCpssDxChVirtualTcamDbSegmentTableCountFreeBlocksSpace
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr,
    IN     GT_U32                             ruleSize
)
{
    GT_U32      i; /* loop counter */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_BLOCK_INFO_STC *blockPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT        fullRowColumns;
    GT_U32      blocksAmount;
    GT_U32      availbleRulePlaces;
    GT_U32      placeCapacity;

    fullRowColumns = prvCpssDxChVirtualTcamDbSegmentTableFullRowColumnsGet(tcamSegCfgPtr);

    blocksAmount = 0;
    for (i = 0; (i < tcamSegCfgPtr->blocksAmount); i++)
    {
        blockPtr = &(tcamSegCfgPtr->blockInfArr[i]);
        if (blockPtr->lookupId !=
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_LOOKUP_FREE_BLOCK_CNS)
        {
            continue;
        }
        blocksAmount ++;
    }

    if (ruleSize == 8)
    {
        availbleRulePlaces =
            (blocksAmount * tcamSegCfgPtr->lookupRowAlignment / 2);
    }
    else
    {
        placeCapacity = prvCpssDxChVirtualTcamDbSegmentTableRowCapacity(
                fullRowColumns, ruleSize);
        availbleRulePlaces =
            (blocksAmount * tcamSegCfgPtr->lookupRowAlignment * placeCapacity);
    }

    return availbleRulePlaces;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTablePrimaryTreeMergeAndSplitStep function
* @endinternal
*
* @brief   Splits or merges primary tree segments to be vTCAM space.
*         Parts containing same rows (but different columns) merged to one segment.
*         Parts remaining after merge splitted to the segments.
*         When the last segment more than needed for request it also splitted
*         to two segments.
*         Rule Amounts not updated.
*         SegmentColumnsMap merged only in different horizontal blocks NONE + OTHER.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] treeId                   - primary tree for allocation.
* @param[in] first                    - GT_TRUE - start from  segment
*                                      GT_FALSE - start from bookmark
* @param[in,out] bookmarkPtr              - (pointer to) bookmark, see "first"
* @param[in,out] bookmarkPtr              - (pointer to) bookmark, saved after function call
*
* @retval GT_OK                    - on success
* @retval GT_NO_RESOURCE           - on no TCAM space
* @retval GT_NO_MORE               - not needed split
*/
static GT_STATUS prvCpssDxChVirtualTcamDbSegmentTablePrimaryTreeMergeAndSplitStep
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC             *tcamSegCfgPtr,
    IN     PRV_CPSS_AVL_TREE_ID                                       treeId,
    IN     GT_BOOL                                                    first,
    INOUT  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC            *bookmarkPtr
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segPtr1; /* segment   */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segPtr2; /* segment   */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segPtr ; /* segment   */
    GT_STATUS               rc;          /* return code                    */
    GT_BOOL                 seeked;      /* node seeked                    */
    PRV_CPSS_AVL_TREE_PATH  path;        /* tree iterator                  */
    GT_VOID_PTR             dbEntryPtr;  /* pointer to entry in DB         */
    GT_U32                  horzBlocksInRow; /* horizontal Blocks In Row   */
    GT_U32                  horzBlocksIndex; /* horizontal Blocks Index    */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT segmentColumns1;/*Columns*/
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT segmentColumns2;/*Columns*/
    GT_BOOL                 verticalMerge; /* vertical Merge               */
    GT_U32                  blockBase;     /* block Base                   */

    if (first != GT_FALSE)
    {
        seeked = prvCpssAvlPathSeek(
                treeId, PRV_CPSS_AVL_TREE_SEEK_FIRST_E,
                path, &dbEntryPtr /* use dedicated var to avoid warnings */);
        segPtr1 = dbEntryPtr;
        if (seeked == GT_FALSE)
        {
            return /* do not treat as error */ GT_NO_MORE;
        }
    }
    else
    {
        seeked = prvCpssAvlPathFind(
                treeId, PRV_CPSS_AVL_TREE_FIND_MIN_GREATER_OR_EQUAL_E,
                bookmarkPtr, path, &dbEntryPtr /* use dedicated var to avoid warnings */);
        segPtr1 = dbEntryPtr;
        if (seeked == GT_FALSE)
        {
            return /* do not treat as error */ GT_NO_MORE;
        }
    }
    seeked = prvCpssAvlPathSeek(
            treeId, PRV_CPSS_AVL_TREE_SEEK_NEXT_E,
            path, &dbEntryPtr /* use dedicated var to avoid warnings */);
    segPtr2 = dbEntryPtr;
    if (seeked == GT_FALSE)
    {
        return /* do not treat as error */ GT_NO_MORE;
    }

    verticalMerge = GT_FALSE;
    while (seeked != GT_FALSE)
    {
        if ((segPtr1->rowsBase + segPtr1->rowsAmount) > segPtr2->rowsBase)
        {
            /* segments have common rows */
            break;
        }

        if (((segPtr1->rowsBase + segPtr1->rowsAmount) == segPtr2->rowsBase)
            && (segPtr1->segmentColumnsMap == segPtr2->segmentColumnsMap))
        {
            blockBase = segPtr2->rowsBase -
                (segPtr2->rowsBase % tcamSegCfgPtr->lookupRowAlignment);
            if (segPtr1->rowsBase >= blockBase)
            {
                /* vertical merge */
                verticalMerge = GT_TRUE;
                break;
            }
        }

        segPtr1 = segPtr2;
        seeked = prvCpssAvlPathSeek(
                treeId, PRV_CPSS_AVL_TREE_SEEK_NEXT_E,
                path, &dbEntryPtr /* use dedicated var to avoid warnings */);
        segPtr2 = dbEntryPtr;
    }

    if (seeked == GT_FALSE)
    {
        /* no update needed */
        return /* do not treat as error */ GT_NO_MORE;
    }

    cpssOsMemCpy(bookmarkPtr, segPtr1, sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC));

    if (verticalMerge != GT_FALSE)
    {
        PRV_PRIMARY_TREE_MRG_SPLIT_TRACE_MAC((
            "Vertical before %d(%d, 0x%X)+%d(%d, 0x%X)",
            segPtr1->rowsBase, segPtr1->rowsAmount, segPtr1->segmentColumnsMap,
            segPtr2->rowsBase, segPtr2->rowsAmount, segPtr2->segmentColumnsMap));

        prvCpssDxChVirtualTcamSegmentTableAvlItemRemove(treeId, segPtr1);
        prvCpssDxChVirtualTcamSegmentTableAvlItemRemove(treeId, segPtr2);
        segPtr1->rowsAmount += segPtr2->rowsAmount;
        cpssBmPoolBufFree(tcamSegCfgPtr->pVTcamMngPtr->segmentsPool, segPtr2);
        rc = prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(treeId, segPtr1);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
        }

        PRV_PRIMARY_TREE_MRG_SPLIT_TRACE_MAC((
        "Vertical result %d(%d, 0x%X)",
        segPtr1->rowsBase, segPtr1->rowsAmount, segPtr1->segmentColumnsMap));

        return GT_OK;
    }

    PRV_PRIMARY_TREE_MRG_SPLIT_TRACE_MAC((
        "Horizontal before %d(%d, 0x%X)+%d(%d, 0x%X)",
        segPtr1->rowsBase, segPtr1->rowsAmount, segPtr1->segmentColumnsMap,
        segPtr2->rowsBase, segPtr2->rowsAmount, segPtr2->segmentColumnsMap));

    /* split part before common                       */
    /* assumed segPtr1->rowsBase <= segPtr2->rowsBase */
    if (segPtr1->rowsBase < segPtr2->rowsBase)
    {
        segPtr = segPtr1;
        rc =  prvCpssDxChVirtualTcamDbSegmentTableCropSegmentVertical(
            tcamSegCfgPtr, treeId, NULL /*tree2*/,
            segPtr,
            segPtr->rowsBase,
            (segPtr2->rowsBase - segPtr->rowsBase),
            GT_TRUE /*putBack*/,
            NULL /*lowSegPtrPtr*/,
            &segPtr1 /*highSegPtrPtr*/);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        if (segPtr1 == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        PRV_PRIMARY_TREE_MRG_SPLIT_TRACE_MAC((
            "Horizontal -- vertical1 %d(%d, 0x%X)+%d(%d, 0x%X)",
            segPtr->rowsBase, segPtr->rowsAmount, segPtr->segmentColumnsMap,
            segPtr1->rowsBase, segPtr1->rowsAmount, segPtr1->segmentColumnsMap));
    }

    /* split part after common                        */
    /* assumed segPtr1->rowsBase == segPtr2->rowsBase */
    if (segPtr1->rowsAmount != segPtr2->rowsAmount)
    {
        /* swap pointers if needed to cause segPtr1->rowsAmount be greater */
        if (segPtr1->rowsAmount < segPtr2->rowsAmount)
        {
            segPtr  = segPtr1;
            segPtr1 = segPtr2;
            segPtr2 = segPtr;
        }
        rc =  prvCpssDxChVirtualTcamDbSegmentTableCropSegmentVertical(
            tcamSegCfgPtr, treeId, NULL /*tree2*/,
            segPtr1,
            segPtr1->rowsBase,
            segPtr2->rowsAmount,
            GT_TRUE /*putBack*/,
            NULL /*lowSegPtrPtr*/,
            NULL /*highSegPtrPtr*/);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        PRV_PRIMARY_TREE_MRG_SPLIT_TRACE_MAC((
            "Horizontal -- vertical2 %d(%d, 0x%X)",
            segPtr1->rowsBase, segPtr1->rowsAmount, segPtr1->segmentColumnsMap));
    }

    /* update */
    prvCpssDxChVirtualTcamSegmentTableAvlItemRemove(treeId, segPtr1);
    prvCpssDxChVirtualTcamSegmentTableAvlItemRemove(treeId, segPtr2);

    /* merge segPtr1 and segPtr2 */
    horzBlocksInRow =
        (tcamSegCfgPtr->tcamColumnsAmount
        / tcamSegCfgPtr->lookupColumnAlignment);
    for (horzBlocksIndex = 0; (horzBlocksIndex < horzBlocksInRow); horzBlocksIndex++)
    {
        segmentColumns1 = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
            segPtr1->segmentColumnsMap, horzBlocksIndex);
        segmentColumns2 = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
            segPtr2->segmentColumnsMap, horzBlocksIndex);
        if (segmentColumns1
            == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E)
        {
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
                segPtr1->segmentColumnsMap, horzBlocksIndex, segmentColumns2);
        }
    }

    cpssBmPoolBufFree(tcamSegCfgPtr->pVTcamMngPtr->segmentsPool, segPtr2);
    rc = prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(treeId, segPtr1);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }

    PRV_PRIMARY_TREE_MRG_SPLIT_TRACE_MAC((
        "Horizontal -- result %d(%d, 0x%X)",
        segPtr1->rowsBase, segPtr1->rowsAmount, segPtr1->segmentColumnsMap));
    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableLuOrderedTreeMerge function
* @endinternal
*
* @brief   Splits or merges lookup ordered segments to be vTCAM space.
*         Parts containing same rows (but different columns) merged to one segment.
*         Parts remaining after merge splitted to new segments.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
*                                      allocRequestPtr - (pointer to) allocation request structure
* @param[in] treeId                   - primary tree for allocation.
*
* @retval GT_OK                    - on success
* @retval GT_NO_RESOURCE           - on no TCAM space
*/
GT_STATUS prvCpssDxChVirtualTcamDbSegmentTableLuOrderedTreeMerge
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC             *tcamSegCfgPtr,
    IN     PRV_CPSS_AVL_TREE_ID                                       treeId
)
{
    GT_STATUS               rc;          /* return code                        */
    GT_BOOL                 first;       /* start iteration                    */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC seg;      /* segment       */

    /* split and merge free spaces */
    first = GT_TRUE;
    while (1)
    {
        rc = prvCpssDxChVirtualTcamDbSegmentTablePrimaryTreeMergeAndSplitStep(
            tcamSegCfgPtr, treeId, first, &seg /*bookmark*/);
        first = GT_FALSE;
        if (rc == GT_NO_MORE)
        {
            break; /* normal finish */
        }
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableUpdateRuleAmounts function
* @endinternal
*
* @brief   Update ruleAmount field in all segments of the tree.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] treeId                   - segments tree.
*                                       None
*/
GT_VOID prvCpssDxChVirtualTcamDbSegmentTableUpdateRuleAmounts
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC             *tcamSegCfgPtr,
    IN     PRV_CPSS_AVL_TREE_ID                                       treeId
)
{
    PRV_CPSS_AVL_TREE_SEEK_ENT                      seekType;     /* seek type                   */
    PRV_CPSS_AVL_TREE_PATH                          path;         /* tree path                   */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segPtr;      /* segment pointer             */
    GT_VOID                                         *dbEntryPtr;  /* void segment pointer        */
    GT_U32                                          rulesPerRow;  /* rules Per Row               */

    seekType = PRV_CPSS_AVL_TREE_SEEK_FIRST_E;
    while (GT_FALSE != prvCpssAvlPathSeek(
            treeId, seekType, path, &dbEntryPtr))
    {
        seekType = PRV_CPSS_AVL_TREE_SEEK_NEXT_E;
        segPtr   = dbEntryPtr;

        rulesPerRow = prvCpssDxChVirtualTcamDbSegmentTableSegmentRowCapacity(
            tcamSegCfgPtr, segPtr->segmentColumnsMap, segPtr->ruleSize);
        segPtr->rulesAmount = (segPtr->rowsAmount * rulesPerRow);
    }
}

/**
* @internal segmentTableVTcamBaseLogicalIndexUpdate function
* @endinternal
*
* @brief   update the baseLogicalIndex in the segments of the vtcam
*         and the rulesAmount of the vtcam
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in,out] tcamCfgPtr               - (pointer to)virtual TCAM structure
*                                       None.
*/
void segmentTableVTcamBaseLogicalIndexUpdate
(
    INOUT  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC                *tcamCfgPtr
)
{
    PRV_CPSS_AVL_TREE_SEEK_ENT  seekMode;
    PRV_CPSS_AVL_TREE_PATH  currentPath;
    GT_VOID_PTR             dbEntryPtr;/* pointer to entry in DB */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segEntryPtr;
    GT_U32    totalRules = 0;

    seekMode = PRV_CPSS_AVL_TREE_SEEK_FIRST_E;

    while (GT_TRUE == prvCpssAvlPathSeek(tcamCfgPtr->segmentsTree,seekMode,
                currentPath, &dbEntryPtr))
    {
        seekMode = PRV_CPSS_AVL_TREE_SEEK_NEXT_E;
        segEntryPtr = dbEntryPtr;

        segEntryPtr->baseLogicalIndex  = totalRules;
        totalRules += segEntryPtr->rulesAmount;
    }

    /* update the total rules amount */
    tcamCfgPtr->rulesAmount = totalRules;

    return ;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableLogToPhyIndexTabGenerate function
* @endinternal
*
* @brief   Generate table for conversion logical indexes to physical indexes of rules.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] treeId                   - tree of vTCAM segments.
*
* @param[out] rulePhysicalIndexArr     - buffer to fulfill by physical indexes
*                                       None
*/
GT_VOID prvCpssDxChVirtualTcamDbSegmentTableLogToPhyIndexTabGenerate
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr,
    IN     PRV_CPSS_AVL_TREE_ID                        treeId,
    OUT    GT_U16                                      *rulePhysicalIndexArr
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segPtr;  /* segment       */
    GT_VOID_PTR                                     dbEntryPtr;  /* pointer to entry in DB       */
    GT_BOOL                                         seeked;       /* node seeked                 */
    PRV_CPSS_AVL_TREE_PATH                          path;         /* tree iterator               */
    GT_U32                                          horzBlocksInRow; /* horzontal Blocks per row */
    GT_U32                                          horzBlockIdx;    /* horzontal Block Index    */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT   segmentColumns;  /* segment columns          */
    GT_U32                                          columnsBitmap;   /* columns bitmap           */
    GT_U32                                          oneColumnBitmap; /* one column bitmap        */
    GT_U32                                          ruleColMask; /* rule Columns mask             */
    GT_U32                                          column;      /* TCAM column                   */
    GT_U32                                          w;           /* work variable                 */
    GT_U32                                          row;         /* row  index                    */
    GT_U32                                          rowBound;    /* row  Bound index              */
    GT_U32                                          logIndex;    /* rule log Index                */

    horzBlocksInRow =
        (tcamSegCfgPtr->tcamColumnsAmount
        / tcamSegCfgPtr->lookupColumnAlignment);

    seeked = prvCpssAvlPathSeek(
            treeId, PRV_CPSS_AVL_TREE_SEEK_FIRST_E,
            path, &dbEntryPtr /* use dedicated var to avoid warnings */);
    segPtr = dbEntryPtr;

    while (seeked != GT_FALSE)
    {
        columnsBitmap = 0;
        for (horzBlockIdx = 0; (horzBlockIdx < horzBlocksInRow); horzBlockIdx++)
        {
            segmentColumns = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
                segPtr->segmentColumnsMap, horzBlockIdx);

            if (segmentColumns
                == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E) continue;

            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_BITMAP_OF_COLUMNS_MAC(
                segmentColumns, oneColumnBitmap);

            columnsBitmap |=
                (oneColumnBitmap
                 << (horzBlockIdx * tcamSegCfgPtr->lookupColumnAlignment));
        }
        ruleColMask = ((1 << segPtr->ruleSize) - 1);
        rowBound = segPtr->rowsBase + segPtr->rowsAmount;
        logIndex = segPtr->baseLogicalIndex;
        for (row = segPtr->rowsBase; (row < rowBound); row++)
        {
            for (column = 0;
                  (column < tcamSegCfgPtr->tcamColumnsAmount);
                  column += prvCpssDxChVirtualTcamDbSegmentTableRuleAlignmentGet(
                      tcamSegCfgPtr, segPtr->ruleSize))
            {
                w = ruleColMask << column;
                if ((columnsBitmap & w) == w)
                {
                    rulePhysicalIndexArr[logIndex] =
                        (GT_U16)((row * tcamSegCfgPtr->tcamColumnsAmount) + column);
                    logIndex ++;
                }
            }
        }
        seeked = prvCpssAvlPathSeek(
                treeId, PRV_CPSS_AVL_TREE_SEEK_NEXT_E,
                path, &dbEntryPtr /* use dedicated var to avoid warnings */);
        segPtr = dbEntryPtr;
    }
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableMoveRulesToNewLayout function
* @endinternal
*
* @brief   Generate table for conversion logical indexes to physical indexes of rules.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] rulesAmount              - amount of rules.
* @param[in] ruleSize                 - rule size measured in 10-byte rules
* @param[in] ruleValidBmpPtr          - (pointer to) bitmap of valid rules.
* @param[in] srcRulePhysicalIndexArr  - (pointer to) array of source rules physical indexes
* @param[in] dstRulePhysicalIndexArr  - (pointer to) array of destination rules physical indexes
*
* @retval GT_OK                    - on success
* @retval GT_NO_RESOURCE           - on no TCAM space
*/
GT_STATUS prvCpssDxChVirtualTcamDbSegmentTableMoveRulesToNewLayout
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr,
    IN     GT_U32                                      rulesAmount,
    IN     GT_U32                                      ruleSize,
    IN     GT_U32                                      *ruleValidBmpPtr,
    IN     GT_U16                                      *srcRulePhysicalIndexArr,
    IN     GT_U16                                      *dstRulePhysicalIndexArr
)
{
    GT_STATUS     rc;        /* return code          */
    GT_U32        lowIdx;    /* low index            */
    GT_U32        highBound; /* high bound           */
    GT_U32        idx;       /* index                */
    GT_COMP_RES   compRes;   /* comparison result */

    if (rulesAmount == 0)
    {
        return GT_OK;
    }

    if (ruleValidBmpPtr == NULL)
    {
        /* should not occur, but the meaning is: no rules to move */
        prvCpssDxChVirtualTcamDbSegmentTableException();
        return GT_OK;
    }

    /* to avoid compiler warnings only */
    lowIdx    = 0;
    compRes   = GT_EQUAL;

    highBound = 0;
    while (highBound < rulesAmount)
    {
        /* find move sequence - all source greater than all desinations or all less */
        /* sequence low index  */
        for (idx = highBound; (idx < rulesAmount); idx++)
        {
            if (srcRulePhysicalIndexArr[idx] != dstRulePhysicalIndexArr[idx])
            {
                lowIdx = idx;
                compRes =
                    ((srcRulePhysicalIndexArr[idx] < dstRulePhysicalIndexArr[idx])
                     ? GT_SMALLER : GT_GREATER);
                break;
            }
        }

        /* next sequence not found */
        if (idx >= rulesAmount) break;

        /* sequence high bound         */
        /* default to updated if found */
        highBound = rulesAmount;
        if (compRes == GT_SMALLER)
        {
            for (idx = (lowIdx + 1); (idx < rulesAmount); idx++)
            {
                if (srcRulePhysicalIndexArr[idx] >= dstRulePhysicalIndexArr[idx])
                {
                    highBound = idx;
                    break;
                }
            }
        }
        else /*compRes == GT_GREATER */
        {
            for (idx = (lowIdx + 1); (idx < rulesAmount); idx++)
            {
                if (srcRulePhysicalIndexArr[idx] <= dstRulePhysicalIndexArr[idx])
                {
                    highBound = idx;
                    break;
                }
            }
        }
        /* sequence move  */
        if (compRes == GT_SMALLER)
        {
            /* destiation indexes are greater - begin from the greatest      */
            /* the condition supports descending loop ending at (lowIdx == 0)*/
            for (idx = (highBound - 1); ((idx + 1) >= (lowIdx + 1)); idx--)
            {
                /* bypass invalid rules */
                if ((ruleValidBmpPtr[idx / 32] & (1 << (idx % 32))) == 0) continue;

                /* move rule */
                rc = tcamSegCfgPtr->ruleMoveFuncPtr(
                    (GT_VOID*)tcamSegCfgPtr /*cookiePtr*/,
                    srcRulePhysicalIndexArr[idx],
                    dstRulePhysicalIndexArr[idx],
                    ruleSize);
                if (rc != GT_OK) return rc;
            }
        }
        else /*compRes == GT_GREATER */
        {
            /* destiation indexes are less - begin from the least */
            for (idx = lowIdx; (idx < highBound); idx++)
            {
                /* bypass invalid rules */
                if ((ruleValidBmpPtr[idx / 32] & (1 << (idx % 32))) == 0) continue;

                /* move rule */
                rc = tcamSegCfgPtr->ruleMoveFuncPtr(
                    (GT_VOID*)tcamSegCfgPtr /*cookiePtr*/,
                    srcRulePhysicalIndexArr[idx],
                    dstRulePhysicalIndexArr[idx],
                    ruleSize);
                if (rc != GT_OK) return rc;
            }
        }
    }
    return GT_OK;
}

/* covert TCAM Rule Size to goal segment type */
#define PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_CONVERT_GOAL_MAC(      \
    _rule_size, _goal)                                                   \
    switch (_rule_size)                                                  \
    {                                                                    \
        case 1: case 2: case 3: case 6:                                  \
            _goal = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E;     \
            break;                                                       \
        case 4:                                                          \
            _goal = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_0123_E;    \
            break;                                                       \
        case 5:                                                          \
            _goal = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01234_E;   \
            break;                                                       \
        default: return _error_retVal;                                   \
    }

/* Queues support */

/* maximal amount of segments used in one conversion */
#define PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_QUEUE_SIZE_CNS 6

typedef struct
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_PTR  seg_ptr[
        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_QUEUE_SIZE_CNS];
    GT_U32  used_rows[PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_QUEUE_SIZE_CNS];
    GT_U32 head;  /* index of valid or == tail */
    GT_U32 tail;  /* index of free space or == head */
} PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_QUEUE_STC;

#define PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_QUEUE_RESET_MAC(_que)           \
    {_que.head = 0; _que.tail = 0;}

/* add to queue tail */
#define PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_QUEUE_TAIL_ADD_MAC(_que, _s, _u) \
    {                                                                      \
        GT_U32 __t = ((_que.tail + 1)                                      \
            % PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_QUEUE_SIZE_CNS);           \
        if (__t != _que.head) /* que is not full */                        \
        {                                                                  \
            _que.seg_ptr[_que.tail] = _s;                                  \
            _que.used_rows[_que.tail] = _u;                                \
            _que.tail = __t;                                               \
        }                                                                  \
    }

/* add to queue head */
#define PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_QUEUE_HEAD_ADD_MAC(_que, _s, _u) \
    {                                                                      \
        GT_U32 __h = (                                                     \
        (_que.head + PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_QUEUE_SIZE_CNS - 1) \
            % PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_QUEUE_SIZE_CNS);           \
        if (__h != _que.tail) /* que is not full */                        \
        {                                                                  \
            _que.head = __h;                                               \
            _que.seg_ptr[_que.head] = _s;                                  \
            _que.used_rows[_que.head] = _u;                                \
        }                                                                  \
    }

/* delete from que */
#define PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_QUEUE_DEL_MAC(_que, _s, _u)     \
    {                                                                     \
        if (_que.head != _que.tail) /* que is not empty */                \
        {                                                                 \
            _s = _que.seg_ptr[_que.head];                                 \
            _u = _que.used_rows[_que.head];                               \
            _que.head = ((_que.head + 1)                                  \
                % PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_QUEUE_SIZE_CNS);      \
        }                                                                 \
        else                                                              \
        {                                                                 \
            _s = NULL;                                                    \
            _u = 0;                                                       \
        }                                                                 \
    }

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableVTcamHorzSplitByBitmap function
* @endinternal
*
* @brief   Horizontal split segment by given bitmap.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] vTcamCfgPtr              - (pointer to) VTCAM configuration structure
*                                      bitmapPtr       - (pointer to) bitmap that contains 1
*                                      for any row that contain any amount of free space
*                                      goalFreeColumns - the free space type that is a reordering goal
*                                      (keep such free space)
* @param[in] segPtr                   - (pointer to) segment for splitting
*
* @param[out] splittedPtr              - (pointer to) GT_TRUE  - segment spluitted
*                                      GT_FALSE - DB unchanged
* @param[out] segUseablePtrPtr         - (pointer to) (pointer to) cropped part of segment.
*                                      NULL if nothing to crop.
*
* @retval GT_OK                    - success, other - error.
*
* @note If segment covers more than one horizontal block it will be
*       vertical splitted to allow undependant using the horizontal parts.
*
*/
static GT_STATUS prvCpssDxChVirtualTcamDbSegmentTableVTcamHorzSplitByBitmap
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC   *tcamSegCfgPtr,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC     *vTcamCfgPtr,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segPtr,
    IN     GT_U32                                          horzBlockBmp,
    OUT    GT_BOOL                                         *splittedPtr,
    OUT    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC **segUseablePtrPtr
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segPtr1;
    GT_STATUS                                       rc;           /* return code                 */

    /* default results */
    *splittedPtr      = GT_FALSE;
    *segUseablePtrPtr = NULL;

    rc = prvCpssDxChVirtualTcamDbSegmentTableCropSegmentHorzBlockBitmap(
        tcamSegCfgPtr, vTcamCfgPtr->segmentsTree, NULL /*tree2*/,
        segPtr, horzBlockBmp, GT_TRUE /*putBack*/, &segPtr1);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "failed on CropSegmentHorzBlockBitmap");
    }

    *splittedPtr = (segPtr1 == NULL) ? GT_FALSE : GT_TRUE;
    *segUseablePtrPtr = segPtr;

    return GT_OK;
}

/* internal state maschine data of function                           */
/* prvCpssDxChVirtualTcamDbSegmentTableVTcamRearrangeRetrieveFreeRows */
/* iterator of free rows segments used by rearranging                 */
typedef struct
{
    enum
    {
        PRV_REARRANGE_FREE_ROWS_INIT_E = 0,
        PRV_REARRANGE_FREE_ROWS_TEMP_TREE_E,
        PRV_REARRANGE_FREE_ROWS_GLOBAL_TREE_INIT_E,
        PRV_REARRANGE_FREE_ROWS_GLOBAL_TREE_NEXT_E,
        PRV_REARRANGE_FREE_ROWS_NO_MORE_E
    }                                   stage;
    PRV_CPSS_AVL_TREE_PATH              path;
} PRV_REARRANGE_FREE_ROWS_ITERATOR_STC;

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableDrag function
* @endinternal
*
* @brief   Drag rules from source to destination trees on a given vTCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] vTcamCfgPtr              - (pointer to) VTCAM configuration structure
* @param[in] vTcamId                  - vTcam Id
* @param[in] srcVTcamSegTree          - Source segment tree
* @param[in] dstVTcamSegTree          - Destination segment tree
*
* @retval GT_OK                    - success, other - error.
*
* @note This function is part of the defragmentation process.
*       This function should be called after it was determined which
*       segments from a certain vTcam (i.e. the ones on srcVTcamSegTree) are
*       to be moved to current free space (i.e. at dstVTcamSegTree).
*       This function rearranges both the free segment tree and the vTcam
*       segment tree, then computes a new mapping of logical to Hw indices,
*       and moves the existing rules to their new Hw places.
*       Note: Both srcVTcamSegTree and dstVTcamSegTree are deleted within
*       this function.
*
*/
static GT_STATUS prvCpssDxChVirtualTcamDbSegmentTableDrag
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC   *vTcamCfgPtr,
    IN     GT_U32                                        vTcamId,
    IN     PRV_CPSS_AVL_TREE_ID                          srcVTcamSegTree,
    IN     PRV_CPSS_AVL_TREE_ID                          dstVTcamSegTree
)
{
    GT_STATUS               rc;                   /* return code */
    GT_U32                  lookupId;
    PRV_CPSS_AVL_TREE_SEEK_ENT                      seekType;     /* seek type  */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segPtr;      /* segment pointer             */
    PRV_CPSS_AVL_TREE_PATH                          path;         /* tree iterator               */
    GT_VOID                                         *dbEntryPtr;  /* segment void pointer        */
    GT_U16                                          *oldRulePhysicalIndexArr; /* old table log -> phy*/
    GT_U32                                          vTcamRuleSize; /* vTcam Rule Size            */

    lookupId = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_LOOKUP_ID_MAC(
        vTcamCfgPtr->tcamInfo.clientGroup, vTcamCfgPtr->tcamInfo.hitNumber);
    vTcamRuleSize = prvCpssDxChVirtualTcamSegmentTableRuleSizeToUnits(
        tcamSegCfgPtr, vTcamCfgPtr->tcamInfo.ruleSize);

    /* remove replaced segments from vTCAM tree and insert to free space */
    while (GT_FALSE != prvCpssAvlPathSeek(
            srcVTcamSegTree, PRV_CPSS_AVL_TREE_SEEK_FIRST_E, path, &dbEntryPtr))
    {
        segPtr   = dbEntryPtr;

        prvCpssDxChVirtualTcamSegmentTableAvlItemRemove(
            srcVTcamSegTree, segPtr);

        prvCpssDxChVirtualTcamSegmentTableAvlItemRemove(
            vTcamCfgPtr->segmentsTree, segPtr);

        rc = prvCpssDxChVirtualTcamDbSegmentTableChainFreeSegmentWithSplit(
            tcamSegCfgPtr, segPtr);
        if (rc != GT_OK)
        {
            prvCpssAvlTreeDelete(
                srcVTcamSegTree, (GT_VOIDFUNCPTR)NULL, (GT_VOID*)NULL);
            prvCpssAvlTreeDelete(
                dstVTcamSegTree,
                (GT_VOIDFUNCPTR)cpssBmPoolBufFree,
                (GT_VOID*)tcamSegCfgPtr->pVTcamMngPtr->segmentsPool);
            CPSS_LOG_ERROR_AND_RETURN_MAC(
                rc, "failed on TableChainFreeSegment");
        }
    }

    /* delete replaced segments tree (empty) before removing from free */
    prvCpssAvlTreeDelete(
        srcVTcamSegTree, (GT_VOIDFUNCPTR)NULL, (GT_VOID*)NULL);

    /* merge free space segments */
    rc = prvCpssDxChVirtualTcamDbSegmentTableFreeSpaceMergeNeighbors(tcamSegCfgPtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(
            rc, "failed on FreeSpaceMergeNeighbors");
    }

    /* remove replaced segments from free space */
    seekType = PRV_CPSS_AVL_TREE_SEEK_FIRST_E;
    while (GT_FALSE != prvCpssAvlPathSeek(
            dstVTcamSegTree, seekType, path, &dbEntryPtr))
    {
        seekType = PRV_CPSS_AVL_TREE_SEEK_NEXT_E;
        segPtr   = dbEntryPtr;

        rc = prvCpssDxChVirtualTcamDbSegmentTableRemoveMemoryFromFree(
            tcamSegCfgPtr, lookupId,
            segPtr->rowsBase, segPtr->rowsAmount, segPtr->segmentColumnsMap);
        if (rc != GT_OK)
        {
            prvCpssAvlTreeDelete(
                dstVTcamSegTree,
                (GT_VOIDFUNCPTR)cpssBmPoolBufFree,
                (GT_VOID*)tcamSegCfgPtr->pVTcamMngPtr->segmentsPool);
            CPSS_LOG_ERROR_AND_RETURN_MAC(
                rc, "failed on FreeSpaceMergeNeighbors");
        }

        rc = prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(
            vTcamCfgPtr->segmentsTree, segPtr);
        if (rc != GT_OK)
        {
            prvCpssAvlTreeDelete(
                dstVTcamSegTree, (GT_VOIDFUNCPTR)NULL, (GT_VOID*)NULL);
            CPSS_LOG_ERROR_AND_RETURN_MAC(
                rc, "failed on TcamSegmentTableAvlItemInsert");
        }
    }

    /* delete replacing segments tree */
    prvCpssAvlTreeDelete(
        dstVTcamSegTree, (GT_VOIDFUNCPTR)NULL, (GT_VOID*)NULL);

    /* final merge free space segments */
    rc = prvCpssDxChVirtualTcamDbSegmentTableFreeSpaceMergeNeighbors(tcamSegCfgPtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(
            rc, "failed on FreeSpaceMergeNeighbors");
    }

    /* update vTcamId and ruleSize fields */
    seekType = PRV_CPSS_AVL_TREE_SEEK_FIRST_E;
    while (GT_FALSE != prvCpssAvlPathSeek(
            vTcamCfgPtr->segmentsTree, seekType, path, &dbEntryPtr))
    {
        seekType = PRV_CPSS_AVL_TREE_SEEK_NEXT_E;
        segPtr   = dbEntryPtr;
        segPtr->vTcamId = vTcamId;
        segPtr->ruleSize = vTcamRuleSize;
    }

    /* merge vTCAM new layout segments */
    rc = prvCpssDxChVirtualTcamDbSegmentTableLuOrderedTreeMerge(
        tcamSegCfgPtr, vTcamCfgPtr->segmentsTree);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(
            rc, "failed on FreeSpaceMergeNeighbors");
    }

    /* update rule amounts after splits and merges */
    prvCpssDxChVirtualTcamDbSegmentTableUpdateRuleAmounts(
        tcamSegCfgPtr, vTcamCfgPtr->segmentsTree);

    /* update start segment logical indexes */
    segmentTableVTcamBaseLogicalIndexUpdate(vTcamCfgPtr);

    if (prvCpssDxChVirtualTcamDbSegmentTablePrimaryTreeMrgSplTraceEnable != GT_FALSE)
    {
        cpssOsPrintf("vTcamCfgPtr->segmentsTree after merging\n");
        prvCpssDxChVirtualTcamDbVTcamDumpSegmentsTree(vTcamCfgPtr->segmentsTree);
    }

    /* save old table logical => physical rule indexes */
    oldRulePhysicalIndexArr = vTcamCfgPtr->rulePhysicalIndexArr;

    /* update Locical => physical table and move rules */
    /* bypassed for dummy vTcam Manager used for checking availability */
    if (oldRulePhysicalIndexArr != NULL)
    {
        /* create new table logical => physical rule indexes */
        vTcamCfgPtr->rulePhysicalIndexArr =
            (GT_U16*)cpssOsMalloc(sizeof(GT_U16) * vTcamCfgPtr->rulesAmount);
        if (vTcamCfgPtr->rulePhysicalIndexArr == NULL)
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }
        prvCpssDxChVirtualTcamDbSegmentTableLogToPhyIndexTabGenerate(
            tcamSegCfgPtr,
            vTcamCfgPtr->segmentsTree,
            vTcamCfgPtr->rulePhysicalIndexArr);

        /* move rules from old to new location */
        rc = prvCpssDxChVirtualTcamDbSegmentTableMoveRulesToNewLayout(
            tcamSegCfgPtr, vTcamCfgPtr->rulesAmount, vTcamRuleSize,
            vTcamCfgPtr->usedRulesBitmapArr,
            oldRulePhysicalIndexArr /*srcRulePhysicalIndexArr*/,
            vTcamCfgPtr->rulePhysicalIndexArr /*dstRulePhysicalIndexArr*/);

        /* free old bitmap (saved) */
        cpssOsFree(oldRulePhysicalIndexArr);

        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(
                rc, "failed on TableMoveRulesToNewLoayout");
        }
    }

    return rc;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableVTcamRearrangeRetrieveFreeRowsInABlock function
* @endinternal
*
* @brief   Retrieve next full-rows segment in a block from permanent free space tree.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] blockInfoPtr             - block to which search is associated
* @param[in] ruleSize                 - Only retrieve free segments appropriate for this rule size.
* @param[in,out] freeRowsIteratorPtr      - (pointer to)free Rows Iterator - holding iterator state
*
* @param[out] segPtrPtr                - (pointer to)(pointer to) found full-rows segment
*
* @retval GT_TRUE                  - found next, GT_FALSE - no more.
*/
static GT_BOOL prvCpssDxChVirtualTcamDbSegmentTableVTcamRearrangeRetrieveFreeRowsInABlock
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC           *tcamSegCfgPtr,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_BLOCK_INFO_STC    *blockInfoPtr,
    IN     CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT             ruleSize,
    INOUT  PRV_REARRANGE_FREE_ROWS_ITERATOR_STC             *freeRowsIteratorPtr,
    OUT    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC  **segPtrPtr
)
{
    GT_BOOL                                         found;
    GT_VOID                                         *dataPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC segKey;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segPtr;

    while (1) /* each stage returns result or pass to ther stage */
    {
        switch (freeRowsIteratorPtr->stage)
        {
            case PRV_REARRANGE_FREE_ROWS_INIT_E:
            case PRV_REARRANGE_FREE_ROWS_TEMP_TREE_E:
                freeRowsIteratorPtr->stage = PRV_REARRANGE_FREE_ROWS_GLOBAL_TREE_INIT_E;
                continue;

            case PRV_REARRANGE_FREE_ROWS_GLOBAL_TREE_INIT_E:
                /* start search of free rows in provided block */
                cpssOsMemSet(&segKey, 0, sizeof(segKey));
                segKey.lookupId = blockInfoPtr->lookupId;
                segKey.rowsBase = blockInfoPtr->rowsBase;
                segKey.segmentColumnsMap =
                    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_EMPTY_CNS;
                PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
                    segKey.segmentColumnsMap, (blockInfoPtr->columnsBase / blockInfoPtr->columnsAmount),
                    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E);
                found = prvCpssAvlPathFind(
                        tcamSegCfgPtr->segFreeLuTree,
                        PRV_CPSS_AVL_TREE_FIND_MIN_GREATER_OR_EQUAL_E,
                        &segKey, freeRowsIteratorPtr->path, &dataPtr);
                segPtr = dataPtr;

                if ((found == GT_FALSE)
                    || (segPtr->lookupId != blockInfoPtr->lookupId))
                {
                    freeRowsIteratorPtr->stage = PRV_REARRANGE_FREE_ROWS_NO_MORE_E;
                    return GT_FALSE;
                }
                else
                if (segPtr->rowsBase >= (blockInfoPtr->rowsBase + blockInfoPtr->rowsAmount) )  /* Vert'ly higher */
                {
                    freeRowsIteratorPtr->stage = PRV_REARRANGE_FREE_ROWS_NO_MORE_E;
                    return GT_FALSE;
                }
                else
                if ((segPtr->rowsBase >= blockInfoPtr->rowsBase) &&
                    (segPtr->rowsBase < (blockInfoPtr->rowsBase + blockInfoPtr->rowsAmount)) &&  /* Vert'ly same */
                    (prvCpssDxChVirtualTcamDbSegmentTableRowCapacity(
                            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
                                            segPtr->segmentColumnsMap,
                                            (blockInfoPtr->columnsBase / blockInfoPtr->columnsAmount)),
                            prvCpssDxChVirtualTcamSegmentTableRuleSizeToUnits(tcamSegCfgPtr, ruleSize))
                        == 0))
                {
                    /* This segment's columns type is either NONE or does not support the rule size */
                    freeRowsIteratorPtr->stage = PRV_REARRANGE_FREE_ROWS_GLOBAL_TREE_NEXT_E;
                    continue;
                }

                freeRowsIteratorPtr->stage = PRV_REARRANGE_FREE_ROWS_GLOBAL_TREE_NEXT_E;
                *segPtrPtr = dataPtr;
                return GT_TRUE;

            case PRV_REARRANGE_FREE_ROWS_GLOBAL_TREE_NEXT_E:
                /* continue search of free rows in provided block */
                found = prvCpssAvlPathSeek(
                    tcamSegCfgPtr->segFreeLuTree, PRV_CPSS_AVL_TREE_SEEK_NEXT_E,
                    freeRowsIteratorPtr->path, &dataPtr);
                segPtr = dataPtr;

                if ((found == GT_FALSE)
                    || (segPtr->lookupId != blockInfoPtr->lookupId))
                {
                    freeRowsIteratorPtr->stage = PRV_REARRANGE_FREE_ROWS_NO_MORE_E;
                    return GT_FALSE;
                }
                else
                if (segPtr->rowsBase >= (blockInfoPtr->rowsBase + blockInfoPtr->rowsAmount) )  /* Vert'ly higher */
                {
                    freeRowsIteratorPtr->stage = PRV_REARRANGE_FREE_ROWS_NO_MORE_E;
                    return GT_FALSE;
                }
                else
                if ((segPtr->rowsBase >= blockInfoPtr->rowsBase) &&
                    (segPtr->rowsBase < (blockInfoPtr->rowsBase + blockInfoPtr->rowsAmount)) &&  /* Vert'ly same */
                    (prvCpssDxChVirtualTcamDbSegmentTableRowCapacity(
                            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
                                            segPtr->segmentColumnsMap,
                                            (blockInfoPtr->columnsBase / blockInfoPtr->columnsAmount)),
                            prvCpssDxChVirtualTcamSegmentTableRuleSizeToUnits(tcamSegCfgPtr, ruleSize))
                        == 0))
                {
                    /* This segment's columns type is either NONE or does not support the rule size */
                    freeRowsIteratorPtr->stage = PRV_REARRANGE_FREE_ROWS_GLOBAL_TREE_NEXT_E;
                    continue;
                }

                freeRowsIteratorPtr->stage = PRV_REARRANGE_FREE_ROWS_GLOBAL_TREE_NEXT_E;
                *segPtrPtr = dataPtr;
                return GT_TRUE;

            case PRV_REARRANGE_FREE_ROWS_NO_MORE_E:
            default: return GT_FALSE;
        }
    }
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableVTcamRearrangeAddSegsWithFreeRowsInABlock function
* @endinternal
*
* @brief   Adding segments higher than the block to src tree,
*         adding free segments in a certain block with whole rows to dst tree.
*         The purpose is to prepare to fill free rows (the ones at dst tree) at
*         this block with rows in higher blocks (in src tree), thus compacting
*         vTcams into fewer blocks.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] vTcamCfgPtr              - (pointer to) VTCAM configuration structure
* @param[in] blockInfoPtr             - block to which vTcam rows are to be moved
*                                      (so that other blocks are freed)
* @param[in] srcVTcamSegTree          - tree to add found segments to it
* @param[in] dstVTcamSegTree          - tree to add replacing segments
*
* @retval GT_OK                    - success, other - error.
*/
static GT_STATUS prvCpssDxChVirtualTcamDbSegmentTableVTcamRearrangeAddSegsWithFreeRowsInABlock
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC   *vTcamCfgPtr,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_BLOCK_INFO_STC    *blockInfoPtr,
    IN     PRV_CPSS_AVL_TREE_ID                          srcVTcamSegTree,
    IN     PRV_CPSS_AVL_TREE_ID                          dstVTcamSegTree
)
{
    GT_STATUS                                       rc;           /* return code */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segPtr;      /* segment pointer             */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC segBookmark;  /* Bookmark saved in order to continue iteration*/
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segPtr1;     /* segment pointer             */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segFreePtr;  /* segment pointer             */
    GT_VOID                                         *dbEntryPtr;  /* segment void pointer        */
    GT_BOOL                                         splitted;     /*splitted                     */
    GT_U32                                          horzBlockBmp; /* horisontal blocks with free space*/
    GT_U32                                          horzBlocksInRow; /*horz Blocks In Row        */
    GT_U32                                          horzBlockIdx; /*horz Blocks Index            */
    GT_U32                                          blockIndex;     /*Blocks Index of segment into TCAM's blocks       */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT   segmentColumns;/* source columns type        */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT   dstSegmentColumns;/* change dst columns type        */
    GT_U32                                          rulesPerFreeSegRow; /*free sgment width*/
    GT_U32                                          rulesPerVTcamSegRow; /*vTcam segment width*/
    GT_BOOL                                         found;         /* found                      */
    GT_U32                                          freeRowsRemainder; /*remainder of free rows  */
    GT_U32                                          freeRowsSpent;   /*spent free rows           */
    PRV_CPSS_AVL_TREE_ITERATOR_STC                  treeIterator; /* tree Iterator               */
    GT_U32                                          segHorzBlocks;/* Segment Horizontal blocks   */
    GT_U32                                          segMaxRows;  /* Segment maximal rows amount  */
    GT_U32                                          segFreeMaxRows;  /* Free Segment maximal rows amount  */
    PRV_REARRANGE_FREE_ROWS_ITERATOR_STC            freeRowsIterator; /* full-free-rows Iterator */

    horzBlocksInRow =
        (tcamSegCfgPtr->tcamColumnsAmount
        / tcamSegCfgPtr->lookupColumnAlignment);

    freeRowsIterator.stage = PRV_REARRANGE_FREE_ROWS_INIT_E;
    found = prvCpssDxChVirtualTcamDbSegmentTableVTcamRearrangeRetrieveFreeRowsInABlock(
        tcamSegCfgPtr, blockInfoPtr,
        vTcamCfgPtr->tcamInfo.ruleSize,
        &freeRowsIterator, &segFreePtr);
    if (found == GT_FALSE)
    {
        /* should not occur, free rows found */
        CPSS_LOG_ERROR_AND_RETURN_MAC(
            GT_FAIL, "No segments with free space");
    }

    freeRowsRemainder = 0;
    freeRowsSpent = 0;

    /* loop by vTCAM segments */
    PRV_CPSS_AVL_TREE_ITERATOR_INIT_MAC(
        treeIterator, vTcamCfgPtr->segmentsTree, GT_FALSE);
    while (GT_TRUE == prvCpssAvlIterationDo(&treeIterator, &dbEntryPtr))
    {
        /* bypass already processed segments */
        if (NULL != prvCpssAvlSearch(srcVTcamSegTree, dbEntryPtr)) continue;
        segPtr   = dbEntryPtr;

        /* Only segments located after relevant block are relevant */
        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_FIND_LAST_MAC(
            segPtr->segmentColumnsMap, horzBlockIdx, segmentColumns);
        if((segPtr->rowsBase + segPtr->rowsAmount) <= blockInfoPtr->rowsBase)
        {
            /* Vert'ly lower - no need to continue seeking */
            break;
        }
        else
        if(((segPtr->rowsBase + segPtr->rowsAmount) <= (blockInfoPtr->rowsBase + blockInfoPtr->rowsAmount)) &&
           ((horzBlockIdx * blockInfoPtr->columnsAmount) <= blockInfoPtr->columnsBase))
        {
            /* Vert'ly same, hor'ly same or lower */
            continue;
        }

        blockIndex =
            ((segPtr->rowsBase / tcamSegCfgPtr->lookupRowAlignment)
             * horzBlocksInRow);
        if (((PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
                    segPtr->segmentColumnsMap,
                    0) == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E) ||
             (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_IS_FULL_BLOCK_MAC(
                                    &tcamSegCfgPtr->blockInfArr[blockIndex + 0]))) &&
            ((PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
                    segPtr->segmentColumnsMap,
                    1) == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E) ||
             (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_IS_FULL_BLOCK_MAC(
                                    &tcamSegCfgPtr->blockInfArr[blockIndex + 1]))))
        {
            /* This segment is located in already full blocks. No need to touch full blocks */
            continue;
        }

        segBookmark = *segPtr;

        /* Crop this segment: leave only the part which is not in this block (both hor'ly and vert'ly) and which fits into free segment */
        if ((segPtr->rowsBase < (blockInfoPtr->rowsBase + blockInfoPtr->rowsAmount)) &&
            (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
                                                segPtr->segmentColumnsMap,
                                                (blockInfoPtr->columnsBase / blockInfoPtr->columnsAmount))
                                                    != PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E))
        {
            /* If segment starts in the block: */
            horzBlockBmp = 0;
            segHorzBlocks = 0;
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_FIND_LAST_MAC(
                segPtr->segmentColumnsMap, horzBlockIdx, segmentColumns);
            if (((horzBlockIdx * blockInfoPtr->columnsAmount) > blockInfoPtr->columnsBase) &&
                (segmentColumns != PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E))
            {
                horzBlockBmp |= (1 << horzBlockIdx);
                segHorzBlocks ++;
            }

            if (horzBlockBmp == 0)
            {
                continue;
            }
            rc = prvCpssDxChVirtualTcamDbSegmentTableVTcamHorzSplitByBitmap(
                tcamSegCfgPtr, vTcamCfgPtr, segPtr, horzBlockBmp, &splitted, &segPtr1);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "failed on SegmentTableVTcamHorzSplitByBitmap");
            }
            segPtr = segPtr1;

            if (splitted != GT_FALSE)
            {
                /* update tree iterator */
                segBookmark = *segPtr;
                PRV_CPSS_AVL_TREE_ITERATOR_FIND_MAC(
                    treeIterator, &segBookmark, PRV_CPSS_AVL_TREE_FIND_MAX_LESS_OR_EQUAL_E);
                PRV_CPSS_AVL_TREE_ITERATOR_RESEEK_MAC(treeIterator);
                if (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_IS_FULL_BLOCK_MAC(
                            &tcamSegCfgPtr->blockInfArr[blockIndex + horzBlockIdx]))
                {
                    /* Splitted segment is located in already full block. No need to move it */
                    continue;
                }
            }

        }
        else
        if (segPtr->rowsBase >= (blockInfoPtr->rowsBase + blockInfoPtr->rowsAmount))
        {
            /* If segment starts after the block: */
            horzBlockBmp = 0;
            segHorzBlocks = 0;
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_FIND_LAST_MAC(
                segPtr->segmentColumnsMap, horzBlockIdx, segmentColumns);
            if (segmentColumns != PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E)
            {
                horzBlockBmp |= (1 << horzBlockIdx);
                segHorzBlocks ++;
            }

            if (horzBlockBmp == 0)
            {
                continue;
            }
            rc = prvCpssDxChVirtualTcamDbSegmentTableVTcamHorzSplitByBitmap(
                tcamSegCfgPtr, vTcamCfgPtr, segPtr, horzBlockBmp, &splitted, &segPtr1);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "failed on SegmentTableVTcamHorzSplitByBitmap");
            }
            segPtr = segPtr1;

            if (splitted != GT_FALSE)
            {
                /* update tree iterator */
                segBookmark = *segPtr;
                PRV_CPSS_AVL_TREE_ITERATOR_FIND_MAC(
                    treeIterator, &segBookmark, PRV_CPSS_AVL_TREE_FIND_MAX_LESS_OR_EQUAL_E);
                PRV_CPSS_AVL_TREE_ITERATOR_RESEEK_MAC(treeIterator);
                if (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_IS_FULL_BLOCK_MAC(
                            &tcamSegCfgPtr->blockInfArr[blockIndex + horzBlockIdx]))
                {
                    /* Splitted segment is located in already full block. No need to move it */
                    continue;
                }
            }

        }
        else
        {
            /* Segment is in a neighbor block */
            segPtr1 = segPtr;
            segHorzBlocks = 1;
        }

        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_FIND_FIRST_MAC(
            segFreePtr->segmentColumnsMap, horzBlockIdx, segmentColumns);
        rulesPerFreeSegRow = prvCpssDxChVirtualTcamDbSegmentTableRowCapacity(
                                segmentColumns,
                                segPtr->ruleSize);
        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_FIND_FIRST_MAC(
            segPtr->segmentColumnsMap, horzBlockIdx, segmentColumns);
        rulesPerVTcamSegRow = prvCpssDxChVirtualTcamDbSegmentTableRowCapacity(
                                segmentColumns,
                                segPtr->ruleSize);
        if (rulesPerVTcamSegRow == 0)
        {
            /* Can't be */
            continue;
        }
        dstSegmentColumns = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E;
        segFreeMaxRows = segFreePtr->rowsAmount - freeRowsSpent;
        if ((segFreeMaxRows * rulesPerFreeSegRow) < (segPtr->rowsAmount * rulesPerVTcamSegRow))
        {
            /* Free segment is smaller than vTcam segment - crop vTcam segment */
            segMaxRows = (segFreeMaxRows * rulesPerFreeSegRow) / rulesPerVTcamSegRow;
            if (segMaxRows == 0)
            {
                /* This free segment is not big enough for one row of vTcam segment */
                continue;
            }

            /* split last segment to needed and not needed parts */
            /* new part - low */
            rc = prvCpssDxChVirtualTcamDbSegmentTableCropSegmentVertical(
                tcamSegCfgPtr, vTcamCfgPtr->segmentsTree, NULL /*tree2*/,
                segPtr,
                segPtr->rowsBase + segPtr->rowsAmount - segMaxRows,
                segMaxRows,
                GT_TRUE /*putBack*/,
                NULL /*lowSegPtrPtr*/, NULL /*highSegPtrPtr*/);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
            }

            /* update tree iterator */
            segBookmark = *segPtr;
            PRV_CPSS_AVL_TREE_ITERATOR_FIND_MAC(
                treeIterator, &segBookmark, PRV_CPSS_AVL_TREE_FIND_MAX_LESS_OR_EQUAL_E);
            PRV_CPSS_AVL_TREE_ITERATOR_RESEEK_MAC(treeIterator);
        }
        else
        {
            /* Free segment is bigger than vTcam segment - Check if V-crop is required because of alignment */
            segFreeMaxRows = (segPtr->rowsAmount * rulesPerVTcamSegRow) / rulesPerFreeSegRow;
            if (segFreeMaxRows == 0)
            {
                /* Try to split the free seg in two */
                if ((segFreePtr->segmentColumnsMap == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E) ||
                    (segFreePtr->segmentColumnsMap == (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E << 4)))
                {
                    if (segPtr->ruleSize == 2)
                    {
                        /* Change free seg row into 40-20, use the -20 as destination */
                        dstSegmentColumns = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_45_E;
                        rulesPerFreeSegRow = 1;
                        segFreeMaxRows = (segPtr->rowsAmount * rulesPerVTcamSegRow) / rulesPerFreeSegRow;
                    }
                    else
                    if ((segPtr->ruleSize == 1) &&
                        (((segPtr->rowsAmount * rulesPerVTcamSegRow) % 2) == 0))
                    {
                        /* Change free seg row into 40-10-10, use the -10-10 as destination */
                        dstSegmentColumns = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_45_E;
                        rulesPerFreeSegRow = 2;
                        segFreeMaxRows = (segPtr->rowsAmount * rulesPerVTcamSegRow) / rulesPerFreeSegRow;
                    }
                    else
                    if (segPtr->ruleSize == 1)
                    {
                        /* Change free seg row into 50-10, use the -10 as destination */
                        dstSegmentColumns = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_5_E;
                        rulesPerFreeSegRow = 1;
                        segFreeMaxRows = (segPtr->rowsAmount * rulesPerVTcamSegRow) / rulesPerFreeSegRow;
                    }
                }

                if (segFreeMaxRows == 0)
                {
                    /* This free segment can not be split for one row of vTcam segment */
                    continue;
                }
            }

            segMaxRows = (segFreeMaxRows * rulesPerFreeSegRow) / rulesPerVTcamSegRow;
            if (segMaxRows < segPtr->rowsAmount)
            {
                /* split segment in order to fit into whole free rows */
                /* new part - low */
                rc = prvCpssDxChVirtualTcamDbSegmentTableCropSegmentVertical(
                    tcamSegCfgPtr, vTcamCfgPtr->segmentsTree, NULL /*tree2*/,
                    segPtr,
                    segPtr->rowsBase + segPtr->rowsAmount - segMaxRows,
                    segMaxRows,
                    GT_TRUE /*putBack*/,
                    NULL /*lowSegPtrPtr*/, NULL /*highSegPtrPtr*/);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
                }

                /* update tree iterator */
                segBookmark = *segPtr;
                PRV_CPSS_AVL_TREE_ITERATOR_FIND_MAC(
                    treeIterator, &segBookmark, PRV_CPSS_AVL_TREE_FIND_MAX_LESS_OR_EQUAL_E);
                PRV_CPSS_AVL_TREE_ITERATOR_RESEEK_MAC(treeIterator);
            }
        }

        /* replaced segment */
        rc = prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(
            srcVTcamSegTree, segPtr);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "failed on prvCpssAvlItemInsert");
        }

        freeRowsRemainder += (segPtr->rowsAmount * segHorzBlocks);

        /* if previous free space segment wasted - find the next */
        /* free segments assumed having single horizontal block   */
        while ((freeRowsRemainder * rulesPerVTcamSegRow) >=
                ((segFreePtr->rowsAmount - freeRowsSpent) * rulesPerFreeSegRow))
        {
            /* update columns */
            segPtr1 = cpssBmPoolBufGet(tcamSegCfgPtr->pVTcamMngPtr->segmentsPool);
            if (segPtr1 == NULL)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
            }
            *segPtr1 = *segFreePtr;

            /* add to destination segments */
            segPtr1->rowsBase   += freeRowsSpent;
            segPtr1->rowsAmount -= freeRowsSpent;
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_FIND_FIRST_MAC(
                segPtr1->segmentColumnsMap, horzBlockIdx, segmentColumns);
            if (dstSegmentColumns == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E)
            {
                switch (segPtr->ruleSize)
                {
                    case 4:
                        segmentColumns = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_0123_E;
                        break;
                    case 5:
                        segmentColumns = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01234_E;
                        break;
                    default:
                        break;
                }
            }
            else
            {
                segmentColumns = dstSegmentColumns;
                dstSegmentColumns = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E;
            }
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
                segPtr1->segmentColumnsMap, horzBlockIdx, segmentColumns);
            rc = prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(
                dstVTcamSegTree, segPtr1);
            if (rc != GT_OK)
            {
                cpssBmPoolBufFree(tcamSegCfgPtr->pVTcamMngPtr->segmentsPool, segPtr1);
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "failed on prvCpssAvlItemInsert");
            }
            freeRowsRemainder -= (segPtr1->rowsAmount * rulesPerFreeSegRow) / rulesPerVTcamSegRow;

            /* next full free rows segment */
            found = prvCpssDxChVirtualTcamDbSegmentTableVTcamRearrangeRetrieveFreeRowsInABlock(
                tcamSegCfgPtr, blockInfoPtr,
                vTcamCfgPtr->tcamInfo.ruleSize,
                &freeRowsIterator, &segFreePtr);
            if (found == GT_FALSE)
            {
                /* no free full rows more */
                segFreePtr = NULL;
                break;
            }
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_FIND_FIRST_MAC(
                segFreePtr->segmentColumnsMap, horzBlockIdx, segmentColumns);
            rulesPerFreeSegRow = prvCpssDxChVirtualTcamDbSegmentTableMaxRulesAmountPerColumnsTypeGet(
                                    segmentColumns,
                                    segPtr->ruleSize);
            freeRowsSpent      = 0;
        }

        /* free rows remainder */
        if ((segFreePtr != NULL) &&
            ((freeRowsRemainder * rulesPerVTcamSegRow) >= rulesPerFreeSegRow))
        {
            /* update columns */
            segPtr1 = cpssBmPoolBufGet(tcamSegCfgPtr->pVTcamMngPtr->segmentsPool);
            if (segPtr1 == NULL)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
            }
            *segPtr1 = *segFreePtr;
            segPtr1->rowsBase   += freeRowsSpent;
            segPtr1->rowsAmount -= freeRowsSpent;
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_FIND_FIRST_MAC(
                segPtr1->segmentColumnsMap, horzBlockIdx, segmentColumns);
            if (dstSegmentColumns == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E)
            {
                switch (segPtr->ruleSize)
                {
                    case 4:
                        segmentColumns = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_0123_E;
                        break;
                    case 5:
                        segmentColumns = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01234_E;
                        break;
                    default:
                        break;
                }
            }
            else
            {
                segmentColumns = dstSegmentColumns;
                dstSegmentColumns = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E;
            }
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
                segPtr1->segmentColumnsMap, horzBlockIdx, segmentColumns);
            if ((freeRowsRemainder * rulesPerVTcamSegRow) <
                    (segPtr1->rowsAmount * rulesPerFreeSegRow))
            {
                segPtr1->rowsAmount = (freeRowsRemainder * rulesPerVTcamSegRow) / rulesPerFreeSegRow;
            }

            /* add to destination segments */
            rc = prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(
                dstVTcamSegTree, segPtr1);
            if (rc != GT_OK)
            {
                cpssBmPoolBufFree(tcamSegCfgPtr->pVTcamMngPtr->segmentsPool, segPtr1);
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "failed on prvCpssAvlItemInsert");
            }
            freeRowsRemainder -= (segPtr1->rowsAmount * rulesPerFreeSegRow) / rulesPerVTcamSegRow;
            freeRowsSpent     += segPtr1->rowsAmount;

        }

        if (segFreePtr == NULL) break;

    }

    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableVTcamBlockRearrange function
* @endinternal
*
* @brief   Rearrange given vTCAM to free whole blocks for allocation to other lookup.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] vTcamCfgPtr              - (pointer to) VTCAM configuration structure
* @param[in] vTcamId                  - vTcam Id
* @param[in] blockInfoPtr             - block to which vTcam rows are to be moved
*                                      (so that other blocks are freed)
*
* @retval GT_OK                    - success, other - error.
*/
static GT_STATUS prvCpssDxChVirtualTcamDbSegmentTableVTcamBlockRearrange
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC   *vTcamCfgPtr,
    IN     GT_U32                                        vTcamId,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_BLOCK_INFO_STC    *blockInfoPtr
)
{
    GT_STATUS               rc;                   /* return code */
    PRV_CPSS_AVL_TREE_ID    srcVTcamSegTree;      /*  tree  */
    PRV_CPSS_AVL_TREE_ID    dstVTcamSegTree;      /*  tree  */

    if (vTcamCfgPtr->rulesAmount == 0)
    {
        /* cannot be rearranged - related also to vTCAM currently being allocated */
        return GT_OK;
    }

    srcVTcamSegTree  = NULL;
    dstVTcamSegTree  = NULL;

    rc = prvCpssAvlTreeCreate(
        (GT_INTFUNCPTR)prvCpssDxChVirtualTcamDbSegmentTableLookupOrderCompareFunc,
        tcamSegCfgPtr->pVTcamMngPtr->segNodesPool,
        &(srcVTcamSegTree));
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "failed on prvCpssAvlTreeCreate");
    }
    rc = prvCpssAvlTreeCreate(
        (GT_INTFUNCPTR)prvCpssDxChVirtualTcamDbSegmentTableLookupOrderCompareFunc,
        tcamSegCfgPtr->pVTcamMngPtr->segNodesPool,
        &(dstVTcamSegTree));
    if (rc != GT_OK)
    {
        prvCpssAvlTreeDelete(
            srcVTcamSegTree, (GT_VOIDFUNCPTR)NULL, (GT_VOID*)NULL);
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "failed on prvCpssAvlTreeCreate");
    }

    rc = prvCpssDxChVirtualTcamDbSegmentTableVTcamRearrangeAddSegsWithFreeRowsInABlock(
        tcamSegCfgPtr, vTcamCfgPtr, blockInfoPtr,
        srcVTcamSegTree, dstVTcamSegTree);
    if (rc != GT_OK)
    {
        prvCpssAvlTreeDelete(
            srcVTcamSegTree, (GT_VOIDFUNCPTR)NULL, (GT_VOID*)NULL);
        prvCpssAvlTreeDelete(
            dstVTcamSegTree,
            (GT_VOIDFUNCPTR)cpssBmPoolBufFree,
            (GT_VOID*)tcamSegCfgPtr->pVTcamMngPtr->segmentsPool);
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "failed on RearrangeAddSegsWithFreeRows");
    }

    rc = prvCpssDxChVirtualTcamDbSegmentTableDrag(
                        tcamSegCfgPtr,
                        vTcamCfgPtr,
                        vTcamId,
                        srcVTcamSegTree,
                        dstVTcamSegTree);

    return rc;
}

/**
* @internal prvCpssDxChVirtualTcamDbBlocksMakeSpaceOnOneLookup function
* @endinternal
*
* @brief   Compress vTcams spanning more than one block into free space of existing block.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] lookupId                 - only compress blocks belonging to this lookup
*
* @retval GT_OK                    - on success
* @retval GT_NO_RESOURCE           - on tree/buffers resource errors
* @retval GT_FULL                  - the TCAM space is full for current allocation
*                                       request
*/
static GT_STATUS prvCpssDxChVirtualTcamDbBlocksMakeSpaceOnOneLookup
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC           *tcamSegCfgPtr,
    IN     GT_U32                                                   lookupId
)
{
    GT_U32                                        i/*, j*/;     /* loop counters */
    GT_STATUS                                     rc = GT_OK;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC           *pVTcamMngPtr;
    GT_U32                                        vTcamId;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC   *vTcamCfgPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segPtr;
    PRV_CPSS_AVL_TREE_PATH                           path;
    GT_VOID                                          *dbEntryPtr;
    GT_BOOL                                       seeked;
    GT_BOOL                                       higher;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_BLOCK_INFO_STC    *blockInfoPtr;
    GT_U32                                        blocksInRow;
    GT_U32                                        blockIndex;

    /* For each block in this lookup:
    *        - If it has free space:
    *           - Find vTcams which can be compressed:
    *               - A vTcam of same lookup which has segments higher
    *                       than this block could be compressed by moving
    *                       rows into this block hopefully freeing other blocks.
    *           - Rearrange rows of found vTcams, until:
    *               - Either there is no more need for free space, or:
    *               - There are no more vTcams.
    */

    if ((tcamSegCfgPtr->blockInfArr == NULL) ||
        (tcamSegCfgPtr->blocksAmount == 0))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    pVTcamMngPtr = tcamSegCfgPtr->pVTcamMngPtr;
    blocksInRow =
        (tcamSegCfgPtr->tcamColumnsAmount
         / tcamSegCfgPtr->lookupColumnAlignment);

    for (i = 0; i < tcamSegCfgPtr->blocksAmount - 1; i++)
    {
        blockInfoPtr = &tcamSegCfgPtr->blockInfArr[i];
        if (blockInfoPtr->lookupId != lookupId)
        {
            /* Not same lookup - not interested */
            continue;
        }

        if (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_IS_FULL_BLOCK_MAC(blockInfoPtr))
        {
            /* No free space - not interesting */
            continue;
        }

        /* There is free space - look for a vTcam which could be moved to fill the free space. */
        for (vTcamId = 0; vTcamId < pVTcamMngPtr->vTcamCfgPtrArrSize; vTcamId++)
        {
            vTcamCfgPtr = pVTcamMngPtr->vTcamCfgPtrArr[vTcamId];

            if (vTcamCfgPtr == NULL)
            {
                /* not assigned vTcam Id */
                continue;
            }
            if (vTcamCfgPtr->tcamInfo.hitNumber != PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_LOOKUP_ID_TO_HIT_NUMBER_MAC(lookupId))
            {
                /* Not same lookup - not interested */
                continue;
            }
            if (vTcamCfgPtr->tcamInfo.clientGroup != PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_LOOKUP_ID_TO_CLIENT_GROUP_MAC(lookupId))
            {
                /* Not same lookup - not interested */
                continue;
            }

            if (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_IS_FULL_BLOCK_FOR_RULE_SIZE_MAC(
                    blockInfoPtr,
                    prvCpssDxChVirtualTcamSegmentTableRuleSizeToUnits(
                        tcamSegCfgPtr, vTcamCfgPtr->tcamInfo.ruleSize)))
            {
                /* No suitable free space for this vTcam - not interesting */
                continue;
            }

            /* Go over vTcam segments located higher, see if they should move into this block */
            seeked = prvCpssAvlPathSeek(
                vTcamCfgPtr->segmentsTree,
                PRV_CPSS_AVL_TREE_SEEK_LAST_E,
                path, &dbEntryPtr);
            while (seeked)
            {
                segPtr = dbEntryPtr;

                if((segPtr->rowsBase + segPtr->rowsAmount) <= blockInfoPtr->rowsBase)
                {
                    /* Vert'ly lower - no need to continue seeking */
                    break;
                }

                blockIndex =
                    ((segPtr->rowsBase / tcamSegCfgPtr->lookupRowAlignment)
                     * blocksInRow);
                if (((PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
                            segPtr->segmentColumnsMap,
                            0) == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E) ||
                     (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_IS_FULL_BLOCK_MAC(
                                            &tcamSegCfgPtr->blockInfArr[blockIndex + 0]))) &&
                    ((PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
                            segPtr->segmentColumnsMap,
                            1) == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E) ||
                     (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_IS_FULL_BLOCK_MAC(
                                            &tcamSegCfgPtr->blockInfArr[blockIndex + 1]))))
                {
                    /* This segment is located in already full blocks. No need to touch it */
                    seeked = prvCpssAvlPathSeek(
                        vTcamCfgPtr->segmentsTree,
                        PRV_CPSS_AVL_TREE_SEEK_PREVIOUS_E,
                        path, &dbEntryPtr);
                    continue;
                }

                higher = GT_FALSE;
                if ((segPtr->rowsBase + segPtr->rowsAmount) >
                        (blockInfoPtr->rowsBase + blockInfoPtr->rowsAmount))
                {
                    /* Vertically higher */
                    higher = GT_TRUE;
                }
                else
                if ((segPtr->rowsBase + segPtr->rowsAmount) > blockInfoPtr->rowsBase)
                {
                    /* Vertically same, check horizontal */
                    if ((PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SHIFT_MAC(
                                        segPtr->segmentColumnsMap,
                                        (blockInfoPtr->columnsBase / blockInfoPtr->columnsAmount) + 1)
                                != 0))
                    {
                        /* Vertically same, horizontally higher */
                        higher = GT_TRUE;
                    }
                }
                if (higher)
                {
                    /* Segment ends higher than current block - let's try to pull it in */
                    /* (Since there is free space for sure at this block, segment can't begin before the block begins) */
                    rc = prvCpssDxChVirtualTcamDbSegmentTableVTcamBlockRearrange(
                                tcamSegCfgPtr,
                                vTcamCfgPtr,
                                vTcamId,
                                blockInfoPtr);

                    /* Finished handling this vTcam, look for another one */
                    break;
                }

                seeked = prvCpssAvlPathSeek(
                    vTcamCfgPtr->segmentsTree,
                    PRV_CPSS_AVL_TREE_SEEK_PREVIOUS_E,
                    path, &dbEntryPtr);
            }

            if (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_IS_FULL_BLOCK_MAC(blockInfoPtr))
            {
                /* No free space in this block - skip to next block */
                break;
            }
        }
    }

    /* freeing space compressing other lookups finished */
    return rc;
}

/**
* @internal prvCpssDxChVirtualTcamDbBlocksMakeSpace function
* @endinternal
*
* @brief   Compresses vTcams of other lookups (i.e. different than allocRequestPtr->lookupId)
*         into fewer blocks, in order to free whole blocks so that allocRequestPtr will succeed.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] allocRequestPtr          - (pointer to) allocation request structure
* @param[in] requestRemainder         - requested amount of space (in rules of size allocRequestPtr->ruleSize)
*
* @retval GT_OK                    - on success
* @retval GT_NO_RESOURCE           - on tree/buffers resource errors
* @retval GT_FULL                  - the TCAM space is full for current allocation
*                                       request
*/
GT_STATUS prvCpssDxChVirtualTcamDbBlocksMakeSpace
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC              *tcamSegCfgPtr,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ALLOC_REQUEST_STC    *allocRequestPtr,
    IN     GT_U32                                                   requestRemainder
)
{
    GT_U32                                        availableInOtherLookupBlocks;
    GT_U32                                        i, j;     /* loop counters */
    GT_U32                                        lookupId;
    GT_STATUS                                     rc;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_BLOCK_INFO_STC
                                *savedBlocksPtr[PRV_CPSS_DXCH_VIRTUAL_TCAM_MAX_HITNUM]
                                            [PRV_CPSS_DXCH_VIRTUAL_TCAM_MAX_CLIENTGROUP];


    cpssOsMemSet (&savedBlocksPtr[0][0], 0, sizeof(savedBlocksPtr));
    availableInOtherLookupBlocks = prvCpssDxChVirtualTcamDbSegmentTableCountFreeableSpaceByOtherLookups(
                                    tcamSegCfgPtr,
                                    allocRequestPtr->lookupId, allocRequestPtr->ruleSize,
                                    savedBlocksPtr);
    if (availableInOtherLookupBlocks < requestRemainder)
    {
        /* Can't reach requested space by compressing blocks */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FULL, LOG_ERROR_NO_MSG);
    }

    /* Compress relevant blocks in other lookups */
    /* A list of blocks is returned by the previous function at saveBlockPtr */

    for (i = 0; i < PRV_CPSS_DXCH_VIRTUAL_TCAM_MAX_HITNUM; i++)
    {
        for (j = 0; j < PRV_CPSS_DXCH_VIRTUAL_TCAM_MAX_CLIENTGROUP; j++)
        {
            if (savedBlocksPtr[i][j] != NULL)
            {
                lookupId = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_LOOKUP_ID_MAC(j, i);
                rc = prvCpssDxChVirtualTcamDbBlocksMakeSpaceOnOneLookup(
                        tcamSegCfgPtr,
                        lookupId);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(
                        rc, "failed on prvCpssDxChVirtualTcamDbBlocksMakeSpaceOnOneLookup");
                }
            }
        }
    }

    /* freeing space compressing other lookups didn't succeed */
    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableCountAvailableSpaceGen function
* @endinternal
*
* @brief   Count available free space for allocation (supports 80 bytes also).
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] lookupId                 - lookup Id
* @param[in] ruleSize                 - rule Size (measured in 10-byte units)
*                                       amount of rules that can be allocated
*                                       without moving rules of other vTCAMs
*/
static GT_U32 prvCpssDxChVirtualTcamDbSegmentTableCountAvailableSpaceGen
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr,
    IN     GT_U32                             lookupId,
    IN     GT_U32                             ruleSize
)
{
    if ((tcamSegCfgPtr->deviceClass ==
         PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP5_E)
        && (ruleSize == 8))
    {
        return prvCpssDxChVirtualTcamDbSegmentTableCountAvailableSpaceFor80ByteRules(
            tcamSegCfgPtr, lookupId);
    }
    else
    {
        return prvCpssDxChVirtualTcamDbSegmentTableCountAvailableSpace(
            tcamSegCfgPtr, lookupId, ruleSize);
    }
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableMakeSpace function
* @endinternal
*
* @brief   Drags existing vTCAMs of 10-bytes and 20-bytes rules
*         to provide required space if needed rearranging contents
*         of blocks already mapped to given vTCAM.
*         If trere is no enough memory only - maps additional blocks to lookup.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] allocRequestPtr          - (pointer to) allocation request structure
*
* @retval GT_OK                    - on success
* @retval GT_NO_RESOURCE           - on tree/buffers resource errors
* @retval GT_FULL                  - the TCAM space is full for current allocation
*                                       request
*/
static GT_STATUS prvCpssDxChVirtualTcamDbSegmentTableMakeSpace
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC              *tcamSegCfgPtr,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ALLOC_REQUEST_STC    *allocRequestPtr
)
{
    GT_U32                                                  availableInLookup;
    GT_U32                                                  possibleAvailable;
    GT_U32                                                  availableInFreeBlocks;
    GT_STATUS                                               rc;
    availableInLookup = prvCpssDxChVirtualTcamDbSegmentTableCountAvailableSpaceGen(
        tcamSegCfgPtr, allocRequestPtr->lookupId, allocRequestPtr->ruleSize);
    if (availableInLookup >= allocRequestPtr->rulesAmount)
    {
        return GT_OK;
    }

    availableInFreeBlocks = prvCpssDxChVirtualTcamDbSegmentTableCountFreeBlocksSpace(
        tcamSegCfgPtr, allocRequestPtr->ruleSize);
    if ((availableInFreeBlocks + availableInLookup) >= allocRequestPtr->rulesAmount)
    {
        return GT_OK;
    }

    /* no enough free space in all TCAM */
    rc = prvCpssDxChVirtualTcamDbSegSpaceEstimateFreeSpace(
        tcamSegCfgPtr, allocRequestPtr);
    if (rc != GT_OK) return rc;

    possibleAvailable = 0;
    rc = prvCpssDxChVirtualTcamDbSegSpaceDragVTcamsInLookupId(
        tcamSegCfgPtr, allocRequestPtr,
        GT_TRUE /*checkWithoutUpdate*/, &possibleAvailable);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(
            rc, "failed on check by SpaceDragVTcamsInLookupId");
    }

    if (possibleAvailable < allocRequestPtr->rulesAmount)
    {
        /* Try to make space by compacting blocks assigned to other lookups */
        rc = prvCpssDxChVirtualTcamDbBlocksMakeSpace(
                tcamSegCfgPtr, allocRequestPtr,
                (allocRequestPtr->rulesAmount - possibleAvailable));
        /* GT_FULL says that freed less then required space, but maybe */
        /* the freed space will be enough for dragged vTcams           */
        if ((rc != GT_OK) && (rc != GT_FULL))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(
                rc, "failed on check by DbBlocksMakeSpace");
        }

        /* recalculate free spaces */

        availableInLookup = prvCpssDxChVirtualTcamDbSegmentTableCountAvailableSpaceGen(
            tcamSegCfgPtr, allocRequestPtr->lookupId, allocRequestPtr->ruleSize);
        if (availableInLookup >= allocRequestPtr->rulesAmount)
        {
            return GT_OK;
        }

        /* Check if blocks are free after compacting other lookup blocks */
        availableInFreeBlocks = prvCpssDxChVirtualTcamDbSegmentTableCountFreeBlocksSpace(
            tcamSegCfgPtr, allocRequestPtr->ruleSize);
        if ((availableInFreeBlocks + availableInLookup) >= allocRequestPtr->rulesAmount)
        {
            return GT_OK;
        }

        rc = prvCpssDxChVirtualTcamDbSegSpaceDragVTcamsInLookupId(
            tcamSegCfgPtr, allocRequestPtr,
            GT_TRUE /*checkWithoutUpdate*/, &possibleAvailable);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(
                rc, "failed on check by SpaceDragVTcamsInLookupId");
        }
        if (possibleAvailable < allocRequestPtr->rulesAmount)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FULL, LOG_ERROR_NO_MSG);
        }
    }

    /* freeing blocks rearranging vTCAMs in the same lookup */
    rc = prvCpssDxChVirtualTcamDbSegSpaceDragVTcamsInLookupId(
        tcamSegCfgPtr, allocRequestPtr,
        GT_FALSE /*checkWithoutUpdate*/, &possibleAvailable);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(
            rc, "failed on check by SpaceDragVTcamsInLookupId");
    }
    if (possibleAvailable < allocRequestPtr->rulesAmount)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FULL, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableVTcamAllocate80bytesPrimaryTreeBuild function
* @endinternal
*
* @brief   Allocate primary tree of memory for Virtual TCAM with 80-byte rules in Segment Table for TCAM.
*         The tree contains segments with first 60-byte pars of 80-byte rules.
*         All segments data allocated from buffer pool, not shared with free space DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] allocRequestPtr          - (pointer to) allocation request structure
*
* @param[out] treeIdPtr                - (pointer to) tree of allocated segments
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
* @retval GT_NO_RESOURCE           - on tree/buffers resource errors
* @retval GT_OUT_OF_CPU_MEM        - no CPU memory
* @retval GT_FULL                  - the TCAM space is full for current allocation
*                                       request
*/
static GT_STATUS prvCpssDxChVirtualTcamDbSegmentTableVTcamAllocate80bytesPrimaryTreeBuild
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC              *tcamSegCfgPtr,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ALLOC_REQUEST_STC    *allocRequestPtr,
    OUT    PRV_CPSS_AVL_TREE_ID                                       *treeIdPtr
)
{
    const GT_U32                                         startBlockIndexAlignment = 2;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                   *pVTcamMngPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC      seg;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC      *segPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC      *segPtr1;
    GT_U32                                               startLookupId[2];
    GT_U32                                               startLookupIdIdx;
    GT_STATUS                                            rc;
    GT_U32                                               *freePlacesBitmapPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT        segmentColumns;
    PRV_CPSS_AVL_TREE_ID                                 primaryTree;
    GT_VOID_PTR                                          dbEntryPtr;
    GT_U32                                               horzBlockIdx;
    PRV_CPSS_AVL_TREE_ITERATOR_STC                       treeIterator;
    GT_U32                                               longRulesBase;
    GT_U32                                               longRulesAmount;
    GT_U32                                               ruleCounter;
    GT_U32                                               upperBound;
    GT_U32                                               neededRules;
    GT_U32                                               blockIdx;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_BLOCK_INFO_STC *blockPtr;

    ruleCounter = 0;
    *treeIdPtr = NULL;
    pVTcamMngPtr = tcamSegCfgPtr->pVTcamMngPtr;

    rc = prvCpssAvlTreeCreate(
        (GT_INTFUNCPTR)prvCpssDxChVirtualTcamDbSegmentTableLookupOrderCompareFunc,
        pVTcamMngPtr->segNodesPool, &primaryTree);
    if (rc != GT_OK)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssAvlTreeCreate failed");
    }

    freePlacesBitmapPtr =
        prvCpssDxChVirtualTcamDbSegmentTableBlockRowBitmapAllocate(tcamSegCfgPtr);
    if (freePlacesBitmapPtr == NULL)
    {

        prvCpssDxChVirtualTcamDbSegmentTableException();
        prvCpssAvlTreeDelete(
            primaryTree,
            (GT_VOIDFUNCPTR)cpssBmPoolBufFree,
            (GT_VOID*)pVTcamMngPtr->segmentsPool);
        CPSS_LOG_ERROR_AND_RETURN_MAC(
            GT_OUT_OF_CPU_MEM, "failed on BlockRowBitmapAllocate");
    }

    /* free space bitmap for the second blocks of 80-byte rules */
    prvCpssDxChVirtualTcamDbSegmentTableBlockRowBitmapReset(
        tcamSegCfgPtr, freePlacesBitmapPtr);
    prvCpssDxChVirtualTcamDbSegmentTableBlockRowBitmapSetForFreeRows(
        tcamSegCfgPtr, freePlacesBitmapPtr, allocRequestPtr->lookupId,
        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E,
        1 /*value*/);
    prvCpssDxChVirtualTcamDbSegmentTableBlockRowBitmapSetForFreeRows(
        tcamSegCfgPtr, freePlacesBitmapPtr, allocRequestPtr->lookupId,
        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E,
        1 /*value*/);

    startLookupId[0] = allocRequestPtr->lookupId;
    startLookupId[1] = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_LOOKUP_FREE_BLOCK_CNS;
    for (startLookupIdIdx = 0; (startLookupIdIdx < 2); startLookupIdIdx++)
    {
        /* looking for free space for the first part of 80-rule */
        cpssOsMemSet(&seg, 0, sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC));
        seg.lookupId = startLookupId[startLookupIdIdx];
        seg.rowsAmount = 1;    /* the smallest           */
        seg.segmentColumnsMap =
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_EMPTY_CNS;
        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
            seg.segmentColumnsMap, 0, PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E);

        PRV_CPSS_AVL_TREE_ITERATOR_INIT_MAC(
            treeIterator, tcamSegCfgPtr->segFreePrefTree, GT_TRUE);
        PRV_CPSS_AVL_TREE_ITERATOR_FIND_MAC(
            treeIterator, &seg, PRV_CPSS_AVL_TREE_FIND_MIN_GREATER_OR_EQUAL_E);
        treeIterator.doSeek = GT_FALSE;
        while (GT_FALSE != prvCpssAvlIterationDo(&treeIterator, &dbEntryPtr))
        {
            if (allocRequestPtr->rulesAmount <= ruleCounter) break;
            neededRules = allocRequestPtr->rulesAmount - ruleCounter;
            segPtr = dbEntryPtr;
            /* given lookup finished */
            if (segPtr->lookupId != seg.lookupId) break;
            /* check that free space is 60-bytes in even horizontal block */
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_FIND_FIRST_MAC(
                segPtr->segmentColumnsMap, horzBlockIdx, segmentColumns);
            if (segmentColumns !=
                PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E) continue;
            if ((horzBlockIdx % startBlockIndexAlignment) != 0) continue;

            /* check free space in the next horizontal block */
            /* next horizontal block lookup Id */
            blockIdx = /* index of current block*/
                ((segPtr->rowsBase / tcamSegCfgPtr->lookupRowAlignment)
                 * (tcamSegCfgPtr->tcamColumnsAmount
                    / tcamSegCfgPtr->lookupColumnAlignment))
                + horzBlockIdx;
            blockPtr = &(tcamSegCfgPtr->blockInfArr[blockIdx + 1]); /*next horizontal block*/
            if ((blockPtr->lookupId != allocRequestPtr->lookupId) &&
                (blockPtr->lookupId != PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_LOOKUP_FREE_BLOCK_CNS))
            {
                /* next horizontal block mapped to other lookup */
                continue;
            }

            if (blockPtr->lookupId == allocRequestPtr->lookupId)
            {
                /* next horizontal block free space (block already mapped to needed lookup) */
                upperBound = (segPtr->rowsBase + segPtr->rowsAmount);
                for (longRulesBase = segPtr->rowsBase;
                     (longRulesBase < upperBound); longRulesBase++)
                {
                   /* first free place in neighbor block */
                   if (0 != prvCpssDxChVirtualTcamDbSegmentTableBlockRowBitmapGet(
                       tcamSegCfgPtr, freePlacesBitmapPtr, longRulesBase, (horzBlockIdx + 1))) break;
                }
                if (longRulesBase >= upperBound) continue;
                for (longRulesAmount = 1;
                     ((longRulesBase + longRulesAmount) < upperBound); longRulesAmount++)
                {
                    if (neededRules <= longRulesAmount) break;
                    /* first busy place in neighbor block */
                    if (0 == prvCpssDxChVirtualTcamDbSegmentTableBlockRowBitmapGet(
                       tcamSegCfgPtr, freePlacesBitmapPtr, (longRulesBase + longRulesAmount),
                       (horzBlockIdx + 1))) break;
                }
            }
            else
            {
                /* if all next block is free */
                longRulesBase   = segPtr->rowsBase;
                longRulesAmount = segPtr->rowsAmount;
            }
            /* put back not cropped low rows */
            segPtr1 = cpssBmPoolBufGet(pVTcamMngPtr->segmentsPool);
            if (segPtr1 == NULL)
            {
                prvCpssDxChVirtualTcamDbSegmentTableException();
                prvCpssDxChVirtualTcamDbSegmentTableBlockRowBitmapFree(
                    freePlacesBitmapPtr);
                prvCpssAvlTreeDelete(
                    primaryTree,
                    (GT_VOIDFUNCPTR)cpssBmPoolBufFree,
                    (GT_VOID*)pVTcamMngPtr->segmentsPool);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, "cpssBmPoolBufGet failed");
            }
            cpssOsMemCpy(segPtr1, segPtr, sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC));
            segPtr1->rowsBase   = longRulesBase;
            segPtr1->rowsAmount = longRulesAmount;
            if (segPtr1->rowsAmount > neededRules)
            {
                segPtr1->rowsAmount = neededRules;
            }

            rc = prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(
                primaryTree, segPtr1);
            if (rc != GT_OK)
            {
                prvCpssDxChVirtualTcamDbSegmentTableBlockRowBitmapFree(
                    freePlacesBitmapPtr);
                prvCpssAvlTreeDelete(
                    primaryTree,
                    (GT_VOIDFUNCPTR)cpssBmPoolBufFree,
                    (GT_VOID*)pVTcamMngPtr->segmentsPool);
                CPSS_LOG_ERROR_AND_RETURN_MAC(
                    rc, "failed on SegmentTableAvlItemInsert");
            }
            ruleCounter += segPtr1->rowsAmount;
        }
    }
    prvCpssDxChVirtualTcamDbSegmentTableBlockRowBitmapFree(
        freePlacesBitmapPtr);
    if (allocRequestPtr->rulesAmount <= ruleCounter)
    {
        *treeIdPtr = primaryTree;
        return GT_OK;
    }
    else
    {
        prvCpssAvlTreeDelete(
            primaryTree,
            (GT_VOIDFUNCPTR)cpssBmPoolBufFree,
            (GT_VOID*)pVTcamMngPtr->segmentsPool);
        *treeIdPtr = NULL;
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FULL, LOG_ERROR_NO_MSG);
    }
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableVTcamAllocate80bytesTreeAllocAndMap function
* @endinternal
*
* @brief   Converts primary tree of memory for Virtual TCAM with 80-byte rules in Segment Table for TCAM
*         to final segments tree:
*         - maps free neighbor blocks to given lookup
*         - removes from free space DB all segments from tree and 20-byte continuations
*         in the neighbor blocks.
*         - updates in segments in the tree segmentColumnsMap, ruleSize, ruleAmount,
*         baseLogicalIndex, vTcamId, lookupId
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] allocRequestPtr          - (pointer to) allocation request structure
* @param[in,out] primaryTree              - primary tree of allocated segments - removed
*
* @param[out] treeIdPtr                - (pointer to) finally tree of allocated segments
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
* @retval GT_NO_RESOURCE           - on tree/buffers resource errors
* @retval GT_OUT_OF_CPU_MEM        - no CPU memory
* @retval GT_FULL                  - the TCAM space is full for current allocation
*                                       request
*/
static GT_STATUS prvCpssDxChVirtualTcamDbSegmentTableVTcamAllocate80bytesTreeAllocAndMap
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC              *tcamSegCfgPtr,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ALLOC_REQUEST_STC    *allocRequestPtr,
    INOUT  PRV_CPSS_AVL_TREE_ID                                       primaryTree,
    OUT    PRV_CPSS_AVL_TREE_ID                                       *treeIdPtr
)
{
    GT_STATUS                                            rc;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                  *pVTcamMngPtr;
    PRV_CPSS_AVL_TREE_ID                                 treeId;
    PRV_CPSS_AVL_TREE_PATH                               path;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC      *segPtr;
    GT_VOID_PTR                                          dbEntryPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT        segmentColumns;
    GT_U32                                               horzBlockIdx;
    GT_U32                                               rulesCount;

    *treeIdPtr = NULL;

    pVTcamMngPtr = tcamSegCfgPtr->pVTcamMngPtr;
    rc = prvCpssAvlTreeCreate(
        (GT_INTFUNCPTR)prvCpssDxChVirtualTcamDbSegmentTableLookupOrderCompareFunc,
        pVTcamMngPtr->segNodesPool, &treeId);
    if (rc != GT_OK)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssAvlTreeCreate failed");
    }

    rulesCount = 0;
    while (GT_FALSE != prvCpssAvlPathSeek(
                primaryTree, PRV_CPSS_AVL_TREE_SEEK_FIRST_E, path, &dbEntryPtr))
    {
        segPtr = dbEntryPtr;
        /* remove from free space segment from primary tree  */
        prvCpssDxChVirtualTcamSegmentTableAvlItemRemove(
            primaryTree, segPtr);

        /* add 20-byte extension in neighbor block to 60-bytes in segment */
        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_FIND_FIRST_MAC(
            segPtr->segmentColumnsMap, horzBlockIdx, segmentColumns);
        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
            segPtr->segmentColumnsMap,(horzBlockIdx + 1),
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E);

        /* set space busy and map blocks to lookup if needed */
        rc = prvCpssDxChVirtualTcamDbSegmentTableRemoveMemoryFromFree(
            tcamSegCfgPtr, allocRequestPtr->lookupId,
            segPtr->rowsBase, segPtr->rowsAmount, segPtr->segmentColumnsMap);
        if (rc != GT_OK)
        {
            cpssBmPoolBufFree(pVTcamMngPtr->segmentsPool, segPtr);
            prvCpssAvlTreeDelete(
                primaryTree,
                (GT_VOIDFUNCPTR)cpssBmPoolBufFree,
                (GT_VOID*)pVTcamMngPtr->segmentsPool);
            prvCpssAvlTreeDelete(
                treeId,
                (GT_VOIDFUNCPTR)cpssBmPoolBufFree,
                (GT_VOID*)pVTcamMngPtr->segmentsPool);
            prvCpssDxChVirtualTcamDbSegmentTableFreeSpaceMerge(tcamSegCfgPtr);
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "SegmentTableRemoveMemoryFromFree failed");
        }

        /* update segment fields */
        segPtr->lookupId          = allocRequestPtr->lookupId;
        segPtr->vTcamId           = allocRequestPtr->vTcamId;
        segPtr->ruleSize          = 8;
        segPtr->rulesAmount       = segPtr->rowsAmount;
        segPtr->baseLogicalIndex  = rulesCount;
        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
            segPtr->segmentColumnsMap,(horzBlockIdx + 1),
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E);
        rulesCount += segPtr->rulesAmount;
        /* add updated segment to finally segment queue */
        rc = prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(
            treeId, segPtr);
        if (rc != GT_OK)
        {
            cpssBmPoolBufFree(pVTcamMngPtr->segmentsPool, segPtr);
            prvCpssAvlTreeDelete(
                primaryTree,
                (GT_VOIDFUNCPTR)cpssBmPoolBufFree,
                (GT_VOID*)pVTcamMngPtr->segmentsPool);
            prvCpssAvlTreeDelete(
                treeId,
                (GT_VOIDFUNCPTR)cpssBmPoolBufFree,
                (GT_VOID*)pVTcamMngPtr->segmentsPool);
            prvCpssDxChVirtualTcamDbSegmentTableFreeSpaceMerge(tcamSegCfgPtr);
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "SegmentTableAvlItemInsert failed");
        }
    }
    prvCpssAvlTreeDelete(
        primaryTree,
        (GT_VOIDFUNCPTR)cpssBmPoolBufFree,
        (GT_VOID*)pVTcamMngPtr->segmentsPool);
    *treeIdPtr = treeId;
    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableVTcamAllocate80bytes function
* @endinternal
*
* @brief   Allocate memory for Virtual TCAM with 80-byte rules in Segment Table for TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in,out] tcamCfgPtr               - (pointer to)virtual TCAM structure
* @param[in] allocRequestPtr          - (pointer to) allocation request structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
* @retval GT_NO_RESOURCE           - on tree/buffers resource errors
* @retval GT_OUT_OF_CPU_MEM        - no CPU memory
* @retval GT_FULL                  - the TCAM space is full for current allocation
*                                       request
*/
static GT_STATUS prvCpssDxChVirtualTcamDbSegmentTableVTcamAllocate80bytes
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC              *tcamSegCfgPtr,
    INOUT  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC                *tcamCfgPtr,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ALLOC_REQUEST_STC    *allocRequestPtr
)
{
    GT_STATUS                                            rc;
    PRV_CPSS_AVL_TREE_ID                                 primaryTree;
    PRV_CPSS_AVL_TREE_ID                                 treeId;

    if (allocRequestPtr->ruleSize != 8)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(
            GT_BAD_PARAM, "not 80 bytes rules");
    }

    /* was prvCpssDxChVirtualTcamDbSegmentTableMakeSpace*/
    rc = prvCpssDxChVirtualTcamDbSegSpace80bytesSpaceMake(tcamSegCfgPtr, allocRequestPtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(
            rc, "failed on prvCpssDxChVirtualTcamDbSegmentTableMakeSpace");
    }

    rc = prvCpssDxChVirtualTcamDbSegmentTableVTcamAllocate80bytesPrimaryTreeBuild(
        tcamSegCfgPtr, allocRequestPtr, &primaryTree);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(
            rc, "failed on VTcamAllocate80bytesPrimaryTreeBuild");
    }

    rc = prvCpssDxChVirtualTcamDbSegmentTableVTcamAllocate80bytesTreeAllocAndMap(
        tcamSegCfgPtr, allocRequestPtr, primaryTree, &treeId);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(
            rc, "failed on VTcamAllocate80bytesTreeAllocAndMap");
    }

    tcamCfgPtr->rulesAmount  = allocRequestPtr->rulesAmount;
    tcamCfgPtr->segmentsTree = treeId;

    if (allocRequestPtr->rulesAmount != 0)
    {
        /* logical to physical conversion array */
        tcamCfgPtr->rulePhysicalIndexArr =
            (GT_U16*)cpssOsMalloc(sizeof(GT_U16) * tcamCfgPtr->rulesAmount);
        if (tcamCfgPtr->rulePhysicalIndexArr == NULL)
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
        }
        prvCpssDxChVirtualTcamDbSegmentTableLogToPhyIndexTabGenerate(
            tcamSegCfgPtr,
            tcamCfgPtr->segmentsTree,
            tcamCfgPtr->rulePhysicalIndexArr);
    }

    /*PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_DBG_PRINT(("vTCAM space tree dump"));
    prvCpssDxChVirtualTcamDbSegmentTableDumpTree(tcamCfgPtr->segmentsTree);*/

    prvCpssDxChVirtualTcamDbSegmentTableFreeSpaceMerge(tcamSegCfgPtr);

    if (prvCpssDxChVirtualTcamDbSegmentTablePrimaryTreeMrgSplTraceEnable != GT_FALSE)
    {
        cpssOsPrintf("\n\n\n End of prvCpssDxChVirtualTcamDbSegmentTableVTcamAllocate \n\n\n");
        prvCpssDxChVirtualTcamDbVTcamDumpFullTcamLayoutByPtr(tcamSegCfgPtr);
        cpssOsPrintf("current vTcam %d Segments tree \n", allocRequestPtr->vTcamId);
        prvCpssDxChVirtualTcamDbVTcamDumpByFieldByPtr(
            tcamSegCfgPtr->pVTcamMngPtr, allocRequestPtr->vTcamId, "segmentsTree");
        cpssOsPrintf("allocated vTcamSegments tree \n");
        prvCpssDxChVirtualTcamDbVTcamDumpSegmentsTree(tcamCfgPtr->segmentsTree);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTablePrimaryTreeBuildCycle function
* @endinternal
*
* @brief   Cycle of Build tree of free TCAM segments for allocation to vTCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] lookupId                 - searched lookup Id
* @param[in] ruleSize                 - rule size measured in 10-byte units
* @param[in] requiredRulesNum         - number of required rules
* @param[in] treeId                   - (pointer to) id of tree accumulating segments.
* @param[in] startFreeColumns         - type of row in free segment to start search
* @param[in] stopFreeColumns          - type of row in found segment to stop search
*                                      if specified PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E - no stop
*                                      ruleConuterPtr   - counter of rule places incremented with rule accumulation.
*
* @retval GT_OK                    - on success
* @retval GT_NO_RESOURCE           - on tree/buffers resource errors
* @retval GT_FULL                  - the TCAM space is full for current allocation
*                                       request
*/
static GT_STATUS prvCpssDxChVirtualTcamDbSegmentTablePrimaryTreeBuildCycle
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC             *tcamSegCfgPtr,
    IN     GT_U32                                                     lookupId,
    IN     GT_U32                                                     ruleSize,
    IN     GT_U32                                                     requiredRulesNum,
    IN     PRV_CPSS_AVL_TREE_ID                                       treeId,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT              startFreeColumns,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT              stopFreeColumns,
    INOUT  GT_U32                                                     *ruleCounterPtr
)
{
    GT_STATUS               rc;          /* return code                         */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC seg;     /* search key      */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segPtr; /* found segment   */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT   segmentColumns;/*segment Col*/
    GT_BOOL                 found;       /* node found                          */
    GT_BOOL                 seeked;      /* node seeked                         */
    PRV_CPSS_AVL_TREE_PATH  path;        /* tree iterator                       */
    GT_U32                  rulesPerRow; /* number of rules per row             */
    GT_VOID_PTR             dbEntryPtr;  /* pointer to entry in DB              */
    GT_U32                  horzBlockIdx;/* horizontal block index              */

    /* gather segments in blocks mapped to the same lookup id */
    /* with the best Columns Map                              */
    cpssOsMemSet(&seg, 0, sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC));
    seg.lookupId = lookupId;
    /* the worst candidate preferred */
    seg.rowsAmount = 1;    /* the smallest           */
    seg.segmentColumnsMap =
        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_EMPTY_CNS;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
        seg.segmentColumnsMap, 0, startFreeColumns);

    found = prvCpssAvlPathFind(
        tcamSegCfgPtr->segFreePrefTree, PRV_CPSS_AVL_TREE_FIND_MIN_GREATER_OR_EQUAL_E,
        &seg, path, &dbEntryPtr /* use dedicated var to avoid warnings */);
    segPtr = dbEntryPtr;
    if (found == GT_FALSE)
    {
        return GT_OK;
    }

    while ((segPtr->lookupId == lookupId)
           && ((*ruleCounterPtr) < requiredRulesNum))
    {
        if (stopFreeColumns !=
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E)
        {
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_FIND_FIRST_MAC(
                segPtr->segmentColumnsMap, horzBlockIdx, segmentColumns);
            if (prvCpssDxChVirtualTcamDbSegmentTableVTcamSegmentColumnsToFreeSpaceRank(
                segmentColumns)
                >= prvCpssDxChVirtualTcamDbSegmentTableVTcamSegmentColumnsToFreeSpaceRank(
                    stopFreeColumns))
            {
                return GT_OK;
            }
        }
        rulesPerRow = prvCpssDxChVirtualTcamDbSegmentTableMaxRowRulesAmountGet(
            segPtr, ruleSize);
        if (rulesPerRow != 0)
        {
            *ruleCounterPtr += (rulesPerRow * segPtr->rowsAmount);
            rc = prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(treeId, segPtr);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "failed on prvCpssAvlItemInsert");
            }
        }
        seeked = prvCpssAvlPathSeek(
                tcamSegCfgPtr->segFreePrefTree, PRV_CPSS_AVL_TREE_SEEK_NEXT_E,
                path, &dbEntryPtr /* use dedicated var to avoid warnings */);
        segPtr = dbEntryPtr;
        if (seeked == GT_FALSE)
        {
            break;
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTablePrimaryTreeBuild function
* @endinternal
*
* @brief   Build tree of free TCAM segments for allocation to vTCAM.
*         Does no changes in old DB tree/pool.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] allocRequestPtr          - (pointer to) allocation request structure
*
* @param[out] treeIdPtr                - (pointer to) id of tree containig space to be
*                                      allocated in lookup order. Segments yet not
*                                      united / splitted for segments using the same
*                                      rows. Yet not clientId mapped and not vTcam updated
*
* @retval GT_OK                    - on success
* @retval GT_NO_RESOURCE           - on tree/buffers resource errors
* @retval GT_FULL                  - the TCAM space is full for current allocation
*                                       request
*/
static GT_STATUS prvCpssDxChVirtualTcamDbSegmentTablePrimaryTreeBuild
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC              *tcamSegCfgPtr,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ALLOC_REQUEST_STC    *allocRequestPtr,
    OUT    PRV_CPSS_AVL_TREE_ID                                       *treeIdPtr
)
{
    GT_STATUS               rc;          /* return code                        */
    PRV_CPSS_AVL_TREE_ID    treeId;      /* id of a new tree                   */
    GT_U32                  rulesNum;    /* number of rules in passed segments */
    GT_U32                  placeMultiplier;      /* converts 60-byte rows to 10,20,30 byte rules */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT   goalFreeColumns;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT   startFreeColumns;
    GT_U32                                          goalFreeColumnsRank;
    GT_U32                                          startFreeColumnsRank;

    rc = prvCpssAvlTreeCreate(
        (GT_INTFUNCPTR)prvCpssDxChVirtualTcamDbSegmentTableLookupOrderCompareFunc,
        tcamSegCfgPtr->pVTcamMngPtr->segNodesPool, &treeId);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssAvlTreeCreate failed");
    }

    rc = prvCpssDxChVirtualTcamDbSegmentTableBestColumnsTypeGet(
        tcamSegCfgPtr, allocRequestPtr->ruleSize, &goalFreeColumns, &placeMultiplier);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "failed on ColumnsTypeGet");
    }

    startFreeColumns =
        prvCpssDxChVirtualTcamDbSegmentTableStartColumnsTypeGet(
            tcamSegCfgPtr, allocRequestPtr->ruleSize);

    rulesNum = 0;

    goalFreeColumnsRank =
        prvCpssDxChVirtualTcamDbSegmentTableVTcamSegmentColumnsToFreeSpaceRank(
            goalFreeColumns);
    startFreeColumnsRank =
        prvCpssDxChVirtualTcamDbSegmentTableVTcamSegmentColumnsToFreeSpaceRank(
            startFreeColumns);


    /* gather segments in blocks mapped to the same lookup id */
    /* with the best Columns Map                              */
    rc = prvCpssDxChVirtualTcamDbSegmentTablePrimaryTreeBuildCycle(
        tcamSegCfgPtr, allocRequestPtr->lookupId,
        allocRequestPtr->ruleSize, allocRequestPtr->rulesAmount,
        treeId, goalFreeColumns,
        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E, &rulesNum);
    if (rc != GT_OK)
    {
        prvCpssAvlTreeDelete(
            treeId, (GT_VOIDFUNCPTR)NULL, (GT_VOID*)NULL);
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "failed on PrimaryTreeBuildCycle");
    }

    if (rulesNum >= allocRequestPtr->rulesAmount)
    {
        CPSS_LOG_INFORMATION_MAC("Successfully allocated [%d] rules as requested , and there are at least [%d] free rules suitable for this client \n",
            allocRequestPtr->rulesAmount , rulesNum - allocRequestPtr->rulesAmount);

        *treeIdPtr = treeId;
        return GT_OK;
    }

    if (goalFreeColumnsRank > startFreeColumnsRank)
    {
        /* gather segments in blocks mapped to the same lookup id */
        /* with any matching Columns Map                          */
        rc = prvCpssDxChVirtualTcamDbSegmentTablePrimaryTreeBuildCycle(
            tcamSegCfgPtr, allocRequestPtr->lookupId,
            allocRequestPtr->ruleSize, allocRequestPtr->rulesAmount,
            treeId, startFreeColumns, goalFreeColumns, &rulesNum);
        if (rc != GT_OK)
        {
            prvCpssAvlTreeDelete(
                treeId, (GT_VOIDFUNCPTR)NULL, (GT_VOID*)NULL);
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "failed on PrimaryTreeBuildCycle");
        }

        if (rulesNum >= allocRequestPtr->rulesAmount)
        {
            CPSS_LOG_INFORMATION_MAC("Successfully allocated [%d] rules as requested , and there are at least [%d] free rules suitable for this client \n",
                allocRequestPtr->rulesAmount , rulesNum - allocRequestPtr->rulesAmount);

            *treeIdPtr = treeId;
            return GT_OK;
        }
    }

    /* gather segments that are free blocks */
    rc = prvCpssDxChVirtualTcamDbSegmentTablePrimaryTreeBuildCycle(
        tcamSegCfgPtr, PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_LOOKUP_FREE_BLOCK_CNS,
        allocRequestPtr->ruleSize, allocRequestPtr->rulesAmount,
        treeId, goalFreeColumns,
        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E, &rulesNum);
    if (rc != GT_OK)
    {
        prvCpssAvlTreeDelete(
            treeId, (GT_VOIDFUNCPTR)NULL, (GT_VOID*)NULL);
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "failed on PrimaryTreeBuildCycle");
    }

    if (rulesNum >= allocRequestPtr->rulesAmount)
    {
        CPSS_LOG_INFORMATION_MAC("Successfully allocated [%d] rules as requested , and there are at least [%d] free rules suitable for this client \n",
            allocRequestPtr->rulesAmount , rulesNum - allocRequestPtr->rulesAmount);

        *treeIdPtr = treeId;
        return GT_OK;
    }

    /* no enough free space found */
    prvCpssAvlTreeDelete(
        treeId, (GT_VOIDFUNCPTR)NULL, (GT_VOID*)NULL);

    /* we may get here from resize or auto-resize or 'memory availability check' */
    /* each one of them should decide how to register error or not */

    CPSS_LOG_INFORMATION_MAC("There are only [%d] (out of[%d] requested) free rules suitable for this client \n",
        rulesNum,allocRequestPtr->rulesAmount);

    return /* do not register as error */ GT_FULL;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTablePrimaryTreeAllocAndMap function
* @endinternal
*
* @brief   Remove primary tree from free space and map free blocks to lookup.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] allocRequestPtr          - (pointer to) allocation request structure
* @param[in] treeId                   - primary tree for allocation.
*
* @retval GT_OK                    - on success
* @retval other                    - on fail
*/
static GT_STATUS prvCpssDxChVirtualTcamDbSegmentTablePrimaryTreeAllocAndMap
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC              *tcamSegCfgPtr,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ALLOC_REQUEST_STC    *allocRequestPtr,
    IN     PRV_CPSS_AVL_TREE_ID                                       treeId
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segPtr; /* found segment  */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_BLOCK_INFO_STC* blockPtr; /* pointer to block info */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT segmentColumns;
    GT_STATUS               rc = GT_OK;  /* return code                        */
    GT_U32                  blocksInRow; /* number of blocks In Row            */
    GT_U32                  blockHorzIdx;/* block In Row index                 */
    GT_BOOL                 seeked;      /* node seeked                        */
    PRV_CPSS_AVL_TREE_PATH  path;        /* tree iterator                      */
    GT_U32                  blockIdx;    /* block index                        */
    GT_VOID_PTR             dbEntryPtr;  /* pointer to entry in DB             */
    GT_U32                  savedBlockNum;    /* amount of save block pointers         */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_BLOCK_INFO_STC** savedBlockPtr; /* saved pointers to block info */

    blocksInRow =
        (tcamSegCfgPtr->tcamColumnsAmount
         / tcamSegCfgPtr->lookupColumnAlignment);

    savedBlockNum = 0;
    savedBlockPtr = cpssOsMalloc (tcamSegCfgPtr->blocksAmount * sizeof(*savedBlockPtr));
    if (savedBlockPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    /* block mapping to lookup */
    seeked = prvCpssAvlPathSeek(
            treeId, PRV_CPSS_AVL_TREE_SEEK_FIRST_E,
            path, &dbEntryPtr /* use dedicated var to avoid warnings */);
    segPtr = dbEntryPtr;
    while (seeked != GT_FALSE)
    {
        if (segPtr->lookupId  != allocRequestPtr->lookupId)
        {
            for (blockHorzIdx = 0; (blockHorzIdx < blocksInRow); blockHorzIdx++)
            {
                segmentColumns =
                    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
                        segPtr->segmentColumnsMap, blockHorzIdx);
                if (segmentColumns ==
                    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E) continue;
                blockIdx =
                    ((blocksInRow * (segPtr->rowsBase / tcamSegCfgPtr->lookupRowAlignment)) +
                    blockHorzIdx);
                if (blockIdx >= tcamSegCfgPtr->blocksAmount)
                {
                    rc = GT_FAIL;
                    break;
                }

                blockPtr                 = &(tcamSegCfgPtr->blockInfArr[blockIdx]);
                if (blockPtr->lookupId == allocRequestPtr->lookupId)
                {
                    /* already mapped as needed */
                    continue;
                }
                if (blockPtr->lookupId !=
                    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_LOOKUP_FREE_BLOCK_CNS)
                {
                    rc = GT_FAIL;
                    break;
                }
                if (tcamSegCfgPtr->lookupMapFuncPtr != NULL)
                {
                    rc = tcamSegCfgPtr->lookupMapFuncPtr(
                        tcamSegCfgPtr->cookiePtr,
                        blockPtr->rowsBase,
                        blockPtr->columnsBase,
                        allocRequestPtr->lookupId);
                    if (rc != GT_OK)
                    {
                        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_DBG_PRINT(
                            ("lookupMapFuncPtr returned %d", rc));
                        break;
                    }
                    /* save blockPtr so that block will be changed if everything's OK */
                    if (savedBlockNum < tcamSegCfgPtr->blocksAmount)
                    {
                        savedBlockPtr[savedBlockNum++] = blockPtr;
                    }
                    else
                    {
                        rc = GT_FAIL;
                        break;
                    }
                }
            }

        }

        if (rc != GT_OK)
        {
            break;
        }
        else
        {
            seeked = prvCpssAvlPathSeek(
                    treeId, PRV_CPSS_AVL_TREE_SEEK_NEXT_E,
                    path, &dbEntryPtr /* use dedicated var to avoid warnings */);
            segPtr = dbEntryPtr;
        }
    }

    if (rc == GT_OK)
    {
        /* remove segments from tree of free segments */
        seeked = prvCpssAvlPathSeek(
                treeId, PRV_CPSS_AVL_TREE_SEEK_FIRST_E,
                path, &dbEntryPtr);

        while (seeked != GT_FALSE)
        {
            segPtr = dbEntryPtr;

            /* remove from free segments trees now that everything's OK */
            prvCpssDxChVirtualTcamSegmentTableAvlItemRemove(
                tcamSegCfgPtr->segFreePrefTree, segPtr);
            prvCpssDxChVirtualTcamSegmentTableAvlItemRemove(
                tcamSegCfgPtr->segFreeLuTree, segPtr);
            prvCpssDxChVirtualTcamDbSegmentTableUpdateBlockFreeSpace(
                tcamSegCfgPtr, segPtr, GT_TRUE/*decrease*/);

            /* update saved segPtr now that everything's OK */
            segPtr->lookupId   = allocRequestPtr->lookupId;

            seeked = prvCpssAvlPathSeek(
                    treeId, PRV_CPSS_AVL_TREE_SEEK_NEXT_E,
                    path, &dbEntryPtr);
        }

        /* update saved blockPtr now that everything's OK */
        while (savedBlockNum-- > 0)
        {
            savedBlockPtr[savedBlockNum]->lookupId = allocRequestPtr->lookupId;
        }
    }

    cpssOsFree (savedBlockPtr);

    return rc;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTablePrimaryTreeMergeAndSplit function
* @endinternal
*
* @brief   Splits or merges primary tree segments to be vTCAM space.
*         Parts containing same rows (but different columns) merged to one segment.
*         Parts remaining after merge splitted to new segments.
*         When the last segment more than needed for request it also splitted
*         to two segments.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] allocRequestPtr          - (pointer to) allocation request structure
* @param[in] treeId                   - primary tree for allocation.
*
* @retval GT_OK                    - on success
* @retval GT_NO_RESOURCE           - on no TCAM space
*/
static GT_STATUS prvCpssDxChVirtualTcamDbSegmentTablePrimaryTreeMergeAndSplit
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC              *tcamSegCfgPtr,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ALLOC_REQUEST_STC    *allocRequestPtr,
    IN     PRV_CPSS_AVL_TREE_ID                                       treeId
)
{
    GT_STATUS               rc;          /* return code                        */
    GT_U32                  rulesPerRow; /* rules per row                      */
    GT_U32                  numOfRules;  /* number of rules                    */
    GT_U32                  totalRules;  /* number of rules                    */
    GT_U32                  extraRules;  /* redundant rules                    */
    GT_U32                  extraRows;   /* redundant rows                     */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segPtr1; /* segment       */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segPtr;  /* segment       */
    GT_VOID_PTR             segPtr2;     /* segment                            */
    GT_BOOL                 seeked;      /* node seeked                        */
    GT_BOOL                 found;       /* node found                         */
    PRV_CPSS_AVL_TREE_PATH  path;        /* tree iterator                      */
    GT_VOID_PTR             dbEntryPtr;  /* pointer to entry in DB             */


    /* split and merge free spaces */
    rc = prvCpssDxChVirtualTcamDbSegmentTableLuOrderedTreeMerge(
        tcamSegCfgPtr, treeId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* free redundant space in chain */
    totalRules = 0;

    seeked = prvCpssAvlPathSeek(
            treeId, PRV_CPSS_AVL_TREE_SEEK_FIRST_E,
            path, &dbEntryPtr /* use dedicated var to avoid warnings */);
    segPtr1 = dbEntryPtr;
    while (seeked != GT_FALSE)
    {
        rulesPerRow = prvCpssDxChVirtualTcamDbSegmentTableMaxRowRulesAmountGet(
            segPtr1, allocRequestPtr->ruleSize);
        numOfRules  = rulesPerRow * segPtr1->rowsAmount;
        totalRules  += numOfRules;
        if (totalRules >= allocRequestPtr->rulesAmount)
        {
            break;
        }
        seeked = prvCpssAvlPathSeek(
                treeId, PRV_CPSS_AVL_TREE_SEEK_NEXT_E,
                path, &dbEntryPtr /* use dedicated var to avoid warnings */);
        segPtr1 = dbEntryPtr;
    }

    if (totalRules < allocRequestPtr->rulesAmount)
    {
        /* should not occur */
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    if (seeked == GT_FALSE)
    {
        return GT_OK;
    }

    /* free space tail */
    while (1)
    {
        found = prvCpssAvlPathFind(
            treeId, PRV_CPSS_AVL_TREE_FIND_MIN_GREATER_OR_EQUAL_E,
            segPtr1, path, &segPtr2);
        if (found == GT_FALSE)
        {
            /* should not occur */
            prvCpssDxChVirtualTcamDbSegmentTableException();
            break;
        }
        seeked = prvCpssAvlPathSeek(
                treeId, PRV_CPSS_AVL_TREE_SEEK_NEXT_E,
                path, &segPtr2);
        if (seeked == GT_FALSE)
        {
            break;
        }
        prvCpssDxChVirtualTcamSegmentTableAvlItemRemove(
            treeId, segPtr2);
        rc = prvCpssDxChVirtualTcamDbSegmentTableChainFreeSegmentWithSplit(
            tcamSegCfgPtr, segPtr2);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    if (totalRules == allocRequestPtr->rulesAmount)
    {
        /* no extra rules */
        return GT_OK;
    }

    /* split last segment */
    extraRules = totalRules - allocRequestPtr->rulesAmount;
    rulesPerRow = prvCpssDxChVirtualTcamDbSegmentTableMaxRowRulesAmountGet(
        segPtr1, allocRequestPtr->ruleSize);
    if (extraRules >= rulesPerRow)
    {
        /* split last rows */
        extraRows = extraRules / rulesPerRow;
        segPtr1->rowsAmount -= extraRows;

        /* split horizontal between blocks */
        segPtr = cpssBmPoolBufGet(tcamSegCfgPtr->pVTcamMngPtr->segmentsPool);
        if (segPtr == NULL)
        {
            /* should never occur */
            prvCpssDxChVirtualTcamDbSegmentTableException();
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_DBG_PRINT(
                ("cpssBmPoolBufGet failed"));
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
        }
        cpssOsMemCpy(segPtr, segPtr1, sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC));
        segPtr->rowsBase    = (segPtr1->rowsBase + segPtr1->rowsAmount);
        segPtr->rowsAmount  = extraRows;
        rc = prvCpssDxChVirtualTcamDbSegmentTableChainFreeSegmentWithSplit(
            tcamSegCfgPtr, segPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTablePrimaryTreeSplitUnusedColumns function
* @endinternal
*
* @brief   Horizontal split and free not useable space of segments.
*         Segments retieved from free space tree covered oly one horizontal block,
*         but already merged with horizontal neighbors (if were).
*         The needed split relevant only for 40 and 50 byte rules that
*         got full block row segments.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] allocRequestPtr          - (pointer to) allocation request structure
* @param[in] treeId                   - primary tree for allocation.
*
* @retval GT_OK                    - on success
* @retval GT_NO_RESOURCE           - on no TCAM space
*/
static GT_STATUS prvCpssDxChVirtualTcamDbSegmentTablePrimaryTreeSplitUnusedColumns
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC              *tcamSegCfgPtr,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ALLOC_REQUEST_STC    *allocRequestPtr,
    IN     PRV_CPSS_AVL_TREE_ID                                       treeId
)
{
    GT_STATUS               rc;          /* return code                        */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segPtr;  /* segment       */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segPtr1; /* segment       */
    GT_BOOL                 seeked;      /* node seeked                        */
    PRV_CPSS_AVL_TREE_PATH  path;        /* tree iterator                      */
    GT_VOID_PTR             dbEntryPtr;  /* pointer to entry in DB             */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT     goalColumns;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP goalColumnsMap;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT     segmentColumns;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT     fullRowColumns;
    GT_U32                                            blocksInRow;
    GT_U32                                            blockHorzIdx ;

    blocksInRow =
        (tcamSegCfgPtr->tcamColumnsAmount
         / tcamSegCfgPtr->lookupColumnAlignment);

    fullRowColumns = prvCpssDxChVirtualTcamDbSegmentTableFullRowColumnsGet(tcamSegCfgPtr);

    seeked = prvCpssAvlPathSeek(
            treeId, PRV_CPSS_AVL_TREE_SEEK_FIRST_E,
            path, &dbEntryPtr /* use dedicated var to avoid warnings */);
    segPtr = dbEntryPtr;
    while (seeked != GT_FALSE)
    {
        /* get ColumnsMap of useable part */
        goalColumnsMap = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_EMPTY_CNS;
        for (blockHorzIdx = 0; (blockHorzIdx < blocksInRow); blockHorzIdx++)
        {
            segmentColumns =
                PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
                    segPtr->segmentColumnsMap, blockHorzIdx);
            if (segmentColumns == fullRowColumns)
            {
               goalColumns =
                   prvCpssDxChVirtualTcamDbSegmentTableFullRowColumnsToCrop(
                       tcamSegCfgPtr, allocRequestPtr->ruleSize, blockHorzIdx);
                PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
                    goalColumnsMap, blockHorzIdx, goalColumns);
            }
            else
            {
                PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
                    goalColumnsMap, blockHorzIdx, segmentColumns);
            }
        }

        /* nothing to free */
        if (segPtr->segmentColumnsMap == goalColumnsMap)
        {
            seeked = prvCpssAvlPathSeek(
                    treeId, PRV_CPSS_AVL_TREE_SEEK_NEXT_E,
                    path, &dbEntryPtr /* use dedicated var to avoid warnings */);
            segPtr = dbEntryPtr;
            continue;
        }

        /* crop useable part and get remainder */
        rc = prvCpssDxChVirtualTcamDbSegmentTableCropSegmentHorizontal(
            tcamSegCfgPtr, treeId, NULL /*tree2*/,
            segPtr, goalColumnsMap, GT_TRUE /*putBack*/, &segPtr1);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        if (segPtr1 == NULL)
        {
            /* must never occur */
            prvCpssDxChVirtualTcamDbSegmentTableException();
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        /* free remainder columns */
        prvCpssDxChVirtualTcamSegmentTableAvlItemRemove(
            treeId, segPtr1);

        rc = prvCpssDxChVirtualTcamDbSegmentTableChainFreeSegmentWithSplit(
            tcamSegCfgPtr, segPtr1);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        /* updated seg - must be found */
        seeked = prvCpssAvlPathFind(
                treeId, PRV_CPSS_AVL_TREE_FIND_MIN_GREATER_OR_EQUAL_E,
                segPtr, path, &dbEntryPtr /* use dedicated var to avoid warnings */);
        segPtr = dbEntryPtr;

        seeked = prvCpssAvlPathSeek(
                treeId, PRV_CPSS_AVL_TREE_SEEK_NEXT_E,
                path, &dbEntryPtr /* use dedicated var to avoid warnings */);
        segPtr = dbEntryPtr;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTablePrimaryTreeReduceHalfRow function
* @endinternal
*
* @brief   reduce last half row from the allocation if got too much rules.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] allocRequestPtr          - (pointer to) allocation request structure
* @param[in] treeId                   - primary tree for allocation.
*
* @param[out] didChangePtr             - indication that the allocation tree changed
*
* @retval GT_OK                    - on success
* @retval GT_NO_RESOURCE           - on no TCAM space
*/
static GT_STATUS prvCpssDxChVirtualTcamDbSegmentTablePrimaryTreeReduceHalfRow
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC              *tcamSegCfgPtr,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ALLOC_REQUEST_STC    *allocRequestPtr,
    IN     PRV_CPSS_AVL_TREE_ID                                       treeId,
    IN     GT_U32                                                     totalRules,
    OUT    GT_BOOL                                                    *didChangePtr
)
{
    GT_STATUS               rc;          /* return code                        */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segPtr; /* segment       */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *bestSegPtr = NULL;  /* segment       */
    PRV_CPSS_AVL_TREE_PATH  path;        /* tree iterator                      */
    GT_VOID_PTR             dbEntryPtr;  /* pointer to entry in DB             */
    PRV_CPSS_AVL_TREE_SEEK_ENT seekType;
    GT_U32  moreThanRequested = totalRules - allocRequestPtr->rulesAmount;
    GT_U32  bestValue = 0;
    GT_U32  newValue;
    GT_U32  bestBlockHorzIdx = 0;
    GT_U32  bestRank = 0;
    GT_U32  maxRulesPerFullRow; /* max rules per full row               */
    GT_U32  maxRulesPerBlockRow; /* max rules per block row             */
    GT_U32  blocksInRow;
    GT_U32  blockHorzLoopIdx;
    GT_U32  blockHorzIdx;
    GT_U32  blocksSegmentInRow = 0;
    GT_U32  bestBlocksSegmentInRow = 0;
    GT_U32  rank;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT     segmentColumns;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP segmentColumnsMap;

    *didChangePtr = GT_FALSE;

    if(moreThanRequested == 0)
    {
        return GT_OK;
    }

    if (allocRequestPtr->ruleSize > tcamSegCfgPtr->lookupColumnAlignment)
    {
        /* 80-byte rules */
        return GT_OK;
    }

    /* calculate maximal rule amount per full TCAM row */
    blocksInRow =
        (tcamSegCfgPtr->tcamColumnsAmount
         / tcamSegCfgPtr->lookupColumnAlignment);

    if (blocksInRow <= 1)
    {
        /* Legacy devices */
        return GT_OK;
    }

    maxRulesPerBlockRow =
        prvCpssDxChVirtualTcamDbSegmentTableMaxRulesAmountPerColumnsTypeGet(
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E,
            allocRequestPtr->ruleSize);
    maxRulesPerFullRow = (maxRulesPerBlockRow * blocksInRow);

    if (moreThanRequested >= maxRulesPerFullRow)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
            "DB ERROR : the [%d B] must allocated too much more than requested",
            allocRequestPtr->ruleSize*10);
    }

    /* searching for the best candidate from the end */
    do
    {
        bestValue = 0;
        bestRank = 0;
        bestSegPtr = NULL;
        seekType = PRV_CPSS_AVL_TREE_SEEK_LAST_E;

        /* look for segment from 2 sizes of the floor */
        /* find segment with the most rules in row */
        while(GT_TRUE == prvCpssAvlPathSeek(
                    treeId, seekType,
                    path, &dbEntryPtr /* use dedicated var to avoid warnings */))
        {
            segPtr = dbEntryPtr;
            seekType = PRV_CPSS_AVL_TREE_SEEK_PREVIOUS_E;

            /* amount of blocks intersecting with segment */
            blocksSegmentInRow = 0;
            for (blockHorzIdx = 0; (blockHorzIdx < blocksInRow); blockHorzIdx++)
            {
                segmentColumns =
                    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
                        segPtr->segmentColumnsMap, blockHorzIdx);
                if (segmentColumns ==
                    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E) continue;
                blocksSegmentInRow ++;
            }

            /* rank of segment candidate to split */
            rank = 0;
            if (blocksSegmentInRow == 1) rank ++;
            if (segPtr->rowsAmount == 1) rank ++;

            /* the best segment/block-column to split and free */
            for (blockHorzLoopIdx = 1; (blockHorzLoopIdx <= blocksInRow); blockHorzLoopIdx++)
            {
                /* back order from (blocksInRow - 1) to 0*/
                blockHorzIdx = (blocksInRow - blockHorzLoopIdx);
                segmentColumns =
                    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
                        segPtr->segmentColumnsMap, blockHorzIdx);
                if (segmentColumns ==
                    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E) continue;
                newValue =
                    prvCpssDxChVirtualTcamDbSegmentTableMaxRulesAmountPerColumnsTypeGet(
                        segmentColumns, allocRequestPtr->ruleSize);

                if (moreThanRequested < newValue) continue;

                /* better with priority for one-block and one-row segments */
                if ((newValue > bestValue) ||
                    ((newValue == bestValue) && (rank > bestRank)))
                {
                    bestValue = newValue;
                    bestSegPtr = segPtr;
                    bestBlockHorzIdx = blockHorzIdx;
                    bestRank = rank;
                    bestBlocksSegmentInRow = blocksSegmentInRow;
                }

                /* better will no be found */
                if ((newValue == maxRulesPerBlockRow)
                    && (rank == 2)) break;
            }
        }

        if(bestSegPtr == NULL)
        {
            /* no more (or any) option to reduce the amount */
            break;
        }

        if (bestSegPtr->rowsAmount > 1)
        {
            /* split last row */
            rc = prvCpssDxChVirtualTcamDbSegmentTableCropSegmentVertical(
                tcamSegCfgPtr, treeId, NULL /*tree2*/,
                bestSegPtr, (bestSegPtr->rowsBase + bestSegPtr->rowsAmount - 1), 1,
                GT_TRUE /*putBack*/,
                &segPtr /*lowSegPtrPtr*/, NULL /*highSegPtrPtr*/);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "CropSegmentVertical failed");
            }
            if (segPtr == NULL)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, "NULL low-part pointer");
            }
            segPtr->rulesAmount =
                (prvCpssDxChVirtualTcamDbSegmentTableMaxRowRulesAmountGet(
                    segPtr, allocRequestPtr->ruleSize)
                 * segPtr->rowsAmount);
        }

        if (bestBlocksSegmentInRow > 1)
        {
            segmentColumnsMap = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_EMPTY_CNS;
            segmentColumns =
                PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
                    bestSegPtr->segmentColumnsMap, bestBlockHorzIdx);
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
                segmentColumnsMap, bestBlockHorzIdx, segmentColumns);
            rc = prvCpssDxChVirtualTcamDbSegmentTableCropSegmentHorizontal(
                tcamSegCfgPtr, treeId, NULL /*tree2*/,
                bestSegPtr,  segmentColumnsMap,
                GT_TRUE /*putBack*/, &segPtr /*remainSegPtrPtr*/);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "CropSegmentVertical failed");
            }
            if (segPtr == NULL)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, "NULL low-part pointer");
            }
            segPtr->rulesAmount =
                (prvCpssDxChVirtualTcamDbSegmentTableMaxRowRulesAmountGet(
                    segPtr, allocRequestPtr->ruleSize)
                 * segPtr->rowsAmount);
        }

        /* state that this segment is now released to the free list of the TCAM manager */
        prvCpssDxChVirtualTcamSegmentTableAvlItemRemove(
            treeId, bestSegPtr);
        rc = prvCpssDxChVirtualTcamDbSegmentTableChainFreeSegmentWithSplit(
            tcamSegCfgPtr, bestSegPtr);
        if (rc != GT_OK)
        {
            return rc;
        }

        *didChangePtr = GT_TRUE;

        if(bestValue > moreThanRequested)
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "DB error: bestValue[%d] > totalRules[%d]",
                bestValue,totalRules);
        }
        else
        if (bestValue == moreThanRequested)
        {
            /* reached best effort */
            break;
        }

        moreThanRequested -= bestValue;
    }
    while(1);

    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableVTcamAllocate function
* @endinternal
*
* @brief   Allocate memory for Virtual TCAM in Segment Table for TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in,out] tcamCfgPtr               - (pointer to)virtual TCAM structure
* @param[in] allocRequestPtr          - (pointer to) allocation request structure
* @param[in,out] tcamCfgPtr               - (pointer to)virtual TCAM structure that updated
*                                      None
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
* @retval GT_NO_RESOURCE           - on tree/buffers resource errors
* @retval GT_FULL                  - the TCAM space is full for current allocation
*                                       request
*/
GT_STATUS prvCpssDxChVirtualTcamDbSegmentTableVTcamAllocate
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC              *tcamSegCfgPtr,
    INOUT  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC                *tcamCfgPtr,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ALLOC_REQUEST_STC    *allocRequestPtr
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segPtr;  /* segment       */
    GT_STATUS               rc;          /* return code                        */
    PRV_CPSS_AVL_TREE_ID    treeId = NULL; /* tree Id                          */
    GT_BOOL                 seeked;      /* node seeked                        */
    PRV_CPSS_AVL_TREE_PATH  path;        /* tree iterator                      */
    GT_U32                  rulesPerRow; /* rules per row                      */
    GT_U32                  totalRules;  /* number of rules                    */
    GT_VOID_PTR             dbEntryPtr;  /* pointer to entry in DB             */
    GT_BOOL                 didChange = GT_FALSE;   /* indication that tree changed  */
    GT_U32                  granularity; /* rule amount granularity            */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ALLOC_REQUEST_STC    allocRequestCopy;

    if (allocRequestPtr->ruleSize == 8)
    {
        return prvCpssDxChVirtualTcamDbSegmentTableVTcamAllocate80bytes(
            tcamSegCfgPtr, tcamCfgPtr, allocRequestPtr);
    }

    granularity =
        prvCpssDxChVirtualTcamDbSegmentTableVTcamSizeGranularity(
            tcamSegCfgPtr, allocRequestPtr->ruleSize);
    if (granularity != 1)
    {
        allocRequestCopy = *allocRequestPtr;
        allocRequestPtr  = &allocRequestCopy;
        allocRequestPtr->rulesAmount += (granularity - 1);
        allocRequestPtr->rulesAmount -= (allocRequestPtr->rulesAmount % granularity);
    }

    /*PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_DBG_PRINT(
        ("prvCpssDxChVirtualTcamDbSegmentTableVTcamAllocate")); */

    /*PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_DBG_PRINT(("Free space tree dump"));
    prvCpssDxChVirtualTcamDbSegmentTableDumpTree(pVTcamMngPtr->segFreeLuTree); */

    CPSS_LOG_INFORMATION_MAC("request for[%d] rules for lookupId[%d] vTcamId[%d] ruleAlignment[%d B] ruleSize[%d B] \n",
        allocRequestPtr->rulesAmount,
        allocRequestPtr->lookupId,
        allocRequestPtr->vTcamId,
        allocRequestPtr->ruleAlignment * 10,
        allocRequestPtr->ruleSize/*1..6*/ * 10);

    rc = prvCpssDxChVirtualTcamDbSegmentTableMakeSpace(
        tcamSegCfgPtr, allocRequestPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChVirtualTcamDbSegmentTablePrimaryTreeBuild(
        tcamSegCfgPtr, allocRequestPtr, &treeId);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChVirtualTcamDbSegmentTablePrimaryTreeAllocAndMap(
        tcamSegCfgPtr, allocRequestPtr, treeId);
    if (rc != GT_OK)
    {
        prvCpssAvlTreeDelete(treeId, (GT_VOIDFUNCPTR)NULL, (GT_VOID*)NULL);
        /* should not occur if not a bug */
        prvCpssDxChVirtualTcamDbSegmentTableException();
        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_DBG_PRINT(
            ("prvCpssDxChVirtualTcamDbSegmentTablePrimaryTreeAllocAndMap failed\n"));
        return rc;
    }

    rc = prvCpssDxChVirtualTcamDbSegmentTablePrimaryTreeMergeAndSplit(
        tcamSegCfgPtr, allocRequestPtr, treeId);
    if (rc != GT_OK)
    {
        prvCpssAvlTreeDelete(treeId, (GT_VOIDFUNCPTR)NULL, (GT_VOID*)NULL);
        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_DBG_PRINT(
            ("prvCpssDxChVirtualTcamDbSegmentTablePrimaryTreeMergeAndSplit failed\n"));
        return rc;
    }

    rc = prvCpssDxChVirtualTcamDbSegmentTablePrimaryTreeSplitUnusedColumns(
        tcamSegCfgPtr, allocRequestPtr, treeId);
    if (rc != GT_OK)
    {
        prvCpssAvlTreeDelete(treeId, (GT_VOIDFUNCPTR)NULL, (GT_VOID*)NULL);
        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_DBG_PRINT(
            ("prvCpssDxChVirtualTcamDbSegmentTablePrimaryTreeSplitUnusedColumns failed\n"));
        return rc;
    }

recalcRulesInTheTree_lbl:
    /* update allocated segments */
    totalRules = 0;
    seeked = prvCpssAvlPathSeek(
            treeId, PRV_CPSS_AVL_TREE_SEEK_FIRST_E,
            path, &dbEntryPtr /* use dedicated var to avoid warnings */);
    segPtr = dbEntryPtr;

    while (seeked != GT_FALSE)
    {
        rulesPerRow = prvCpssDxChVirtualTcamDbSegmentTableMaxRowRulesAmountGet(
            segPtr, allocRequestPtr->ruleSize);
        segPtr->vTcamId           = allocRequestPtr->vTcamId;
        segPtr->lookupId          = allocRequestPtr->lookupId;
        segPtr->rulesAmount       = (rulesPerRow * segPtr->rowsAmount);
        segPtr->ruleSize          = allocRequestPtr->ruleSize;
        segPtr->baseLogicalIndex  = totalRules;
        totalRules               += segPtr->rulesAmount;
        seeked = prvCpssAvlPathSeek(
                treeId, PRV_CPSS_AVL_TREE_SEEK_NEXT_E,
                path, &dbEntryPtr /* use dedicated var to avoid warnings */);
        segPtr = dbEntryPtr;
        /* adding to additional trees (for resizing/defrag) should be added here */
    }

    tcamCfgPtr->segmentsTree = treeId;
    tcamCfgPtr->rulesAmount  = totalRules;
    tcamCfgPtr->rulePhysicalIndexArr = NULL;

    if(didChange == GT_FALSE)
    {
        /* reduce last half row from the allocation if got too much rules */
        rc = prvCpssDxChVirtualTcamDbSegmentTablePrimaryTreeReduceHalfRow(
            tcamSegCfgPtr, allocRequestPtr, treeId,totalRules, &didChange);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
        }

        if(didChange == GT_TRUE)
        {
            goto recalcRulesInTheTree_lbl;
        }
    }

    if (tcamCfgPtr->rulesAmount)
    {

        /* logical to physical conversion array */
        tcamCfgPtr->rulePhysicalIndexArr =
            (GT_U16*)cpssOsMalloc(sizeof(GT_U16) * tcamCfgPtr->rulesAmount);
        if (tcamCfgPtr->rulePhysicalIndexArr == NULL)
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
        }
        prvCpssDxChVirtualTcamDbSegmentTableLogToPhyIndexTabGenerate(
            tcamSegCfgPtr,
            tcamCfgPtr->segmentsTree,
            tcamCfgPtr->rulePhysicalIndexArr);
    }

    /*PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_DBG_PRINT(("vTCAM space tree dump"));
    prvCpssDxChVirtualTcamDbSegmentTableDumpTree(tcamCfgPtr->segmentsTree);*/

    if(totalRules > allocRequestPtr->rulesAmount)
    {
        CPSS_LOG_INFORMATION_MAC("Successfully reserve [%d] rules ([%d] more than requested[%d]) for the client \n",
            totalRules,totalRules-allocRequestPtr->rulesAmount,allocRequestPtr->rulesAmount);
    }
    else
    {
        CPSS_LOG_INFORMATION_MAC("Successfully reserve [%d] rules (as requested) for the client \n",
            totalRules);
    }

    /* split and merge free spaces */
    rc = prvCpssDxChVirtualTcamDbSegmentTableLuOrderedTreeMerge(
        tcamSegCfgPtr, tcamCfgPtr->segmentsTree);
    if (rc != GT_OK)
    {
        if (tcamCfgPtr->rulePhysicalIndexArr != NULL)
        {
            /* free logical to physical conversion array */
            cpssOsFree(tcamCfgPtr->rulePhysicalIndexArr);
            tcamCfgPtr->rulePhysicalIndexArr = NULL;
        }
        return rc;
    }

    /* update rule amounts after splits and merges */
    prvCpssDxChVirtualTcamDbSegmentTableUpdateRuleAmounts(
        tcamSegCfgPtr, tcamCfgPtr->segmentsTree);

    prvCpssDxChVirtualTcamDbSegmentTableFreeSpaceMerge(tcamSegCfgPtr);

    if (prvCpssDxChVirtualTcamDbSegmentTablePrimaryTreeMrgSplTraceEnable != GT_FALSE)
    {
        cpssOsPrintf("\n\n\n End of prvCpssDxChVirtualTcamDbSegmentTableVTcamAllocate \n\n\n");
        prvCpssDxChVirtualTcamDbVTcamDumpFullTcamLayoutByPtr(tcamSegCfgPtr);
        cpssOsPrintf("current vTcam %d Segments tree \n", allocRequestPtr->vTcamId);
        prvCpssDxChVirtualTcamDbVTcamDumpByFieldByPtr(
            tcamSegCfgPtr->pVTcamMngPtr, allocRequestPtr->vTcamId, "segmentsTree");
        cpssOsPrintf("allocated vTcamSegments tree \n");
        prvCpssDxChVirtualTcamDbVTcamDumpSegmentsTree(tcamCfgPtr->segmentsTree);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableFreeWithSplit function
* @endinternal
*
* @brief   Put all segments of TCAM memory from the given tree
*         to global list of free segments.
*         Split segments that covers more than one block.
*         (related only to columns bitmap, block never covers rows of different flors)
*         Delete the given tree.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] treeId                   - tree of allocated segments.
*
* @retval GT_OK                    - on success
* @retval GT_NO_RESOURCE           - on no TCAM space
*/
static GT_STATUS prvCpssDxChVirtualTcamDbSegmentTableFreeWithSplit
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC             *tcamSegCfgPtr,
    IN     PRV_CPSS_AVL_TREE_ID                                       treeId
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segPtr ; /* segment  */
    GT_BOOL                 seeked;      /* node seeked                   */
    PRV_CPSS_AVL_TREE_PATH  path;        /* tree iterator                 */
    GT_VOID_PTR             dbEntryPtr;  /* pointer to entry in DB        */

    seeked = prvCpssAvlPathSeek(
            treeId, PRV_CPSS_AVL_TREE_SEEK_FIRST_E,
            path, &dbEntryPtr /* use dedicated var to avoid warnings */);
    segPtr = dbEntryPtr;
    while (seeked != GT_FALSE)
    {
        /* remove */
        prvCpssDxChVirtualTcamSegmentTableAvlItemRemove(
            treeId, (GT_VOID*)segPtr);

        /* insert */
        prvCpssDxChVirtualTcamDbSegmentTableChainFreeSegmentWithSplit(
            tcamSegCfgPtr, segPtr);

        /* next segment */
        seeked = prvCpssAvlPathSeek(
                treeId, PRV_CPSS_AVL_TREE_SEEK_FIRST_E,
                path, &dbEntryPtr /* use dedicated var to avoid warnings */);
        segPtr = dbEntryPtr;
    }

    prvCpssDxChVirtualTcamDbSegmentTableFreeSpaceMerge(tcamSegCfgPtr);

    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableVTcamFree function
* @endinternal
*
* @brief   Free memory of Virtual TCAM in Segment Table for TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in,out] tcamCfgPtr               - (pointer to)virtual TCAM structure
* @param[in,out] tcamCfgPtr               - (pointer to)virtual TCAM structure
*                                      freed objects handlers set to NULL
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       - the given tree has not been initialized.
* @retval GT_BAD_STATE             - In case the tree is not empty.
*/
GT_STATUS prvCpssDxChVirtualTcamDbSegmentTableVTcamFree
(
    INOUT  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC                *tcamCfgPtr
)
{
    GT_STATUS     rc, rcFinal;              /* return code */

    rcFinal = GT_OK;

    /* free logical to physical conversion array */
    if (tcamCfgPtr->rulePhysicalIndexArr != NULL)
    {
        cpssOsFree(tcamCfgPtr->rulePhysicalIndexArr);
        tcamCfgPtr->rulePhysicalIndexArr = NULL;
    }

    if (tcamCfgPtr->segmentsTree != NULL)
    {
        rc = prvCpssDxChVirtualTcamDbSegmentTableFreeWithSplit(
            tcamCfgPtr->tcamSegCfgPtr, tcamCfgPtr->segmentsTree);
        if (rc != GT_OK)
        {
            rcFinal = rc;
        }

        rc = prvCpssAvlTreeDelete(
            tcamCfgPtr->segmentsTree, (GT_VOIDFUNCPTR)NULL, (GT_VOID*)NULL);
        if (rc != GT_OK)
        {
            rcFinal = rc;
        }

        tcamCfgPtr->segmentsTree = NULL;

        /*PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_DBG_PRINT(("Free space tree dump"));
        prvCpssDxChVirtualTcamDbSegmentTableDumpTree(pVTcamMngPtr->segFreeLuTree);*/
    }

    return rcFinal;
}

/**
* @internal segmentTableVTcamFullCombine function
* @endinternal
*
* @brief   Move all segments from secondary Virtual TCAM tree to the primary Virtual TCAM tree.
*         Segments moved as is. All vertical and horizontal merging of segments left
*         for the next processing. THe seconary tree becomes empty.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in,out] primaryTcamCfgPtr        - (pointer to)primary virtual TCAM structure
* @param[in,out] secondaryTcamCfgPtr      - (pointer to)secondary virtual TCAM structure
* @param[in,out] primaryTcamCfgPtr        - (pointer to)updated primary virtual TCAM structure
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       - the given tree has not been initialized.
* @retval GT_BAD_STATE             - In case the tree is not empty.
*/
GT_STATUS segmentTableVTcamFullCombine
(
    INOUT  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC  *primaryTcamCfgPtr,
    INOUT  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC  *secondaryTcamCfgPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_AVL_TREE_ID    avlSecondary;
    PRV_CPSS_AVL_TREE_PATH  pathSecondary;
    GT_VOID_PTR             dbEntryPtr;/* pointer to entry in DB */

    avlSecondary = secondaryTcamCfgPtr->segmentsTree;

    /* get element from secondary */
    while (GT_TRUE == prvCpssAvlPathSeek(avlSecondary,
                PRV_CPSS_AVL_TREE_SEEK_FIRST_E,/* we remove the first so we keep popping first*/
                pathSecondary, &dbEntryPtr))
    {
        /* remove the element from the secondary , before we may modify it's content */
        if(NULL == prvCpssDxChVirtualTcamSegmentTableAvlItemRemove(
            avlSecondary, dbEntryPtr))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        rc = prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(
            primaryTcamCfgPtr->segmentsTree, dbEntryPtr);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableVTcamCombine function
* @endinternal
*
* @brief   Move segmentsTree of secondary Virtual TCAM into primary Virtual TCAM.
*         the function free any secondary resource related to segmentsTree that
*         not needed any more.
*         the function fully align the info in the tree of primaryTcamCfgPtr->segmentsTree
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in,out] primaryTcamCfgPtr        - (pointer to)primary virtual TCAM structure
* @param[in] secondaryTcamCfgPtr      - (pointer to)secondary virtual TCAM structure
* @param[in,out] primaryTcamCfgPtr        - (pointer to)updated primary virtual TCAM structure
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       - the given tree has not been initialized.
* @retval GT_BAD_STATE             - In case the tree is not empty.
*/
GT_STATUS prvCpssDxChVirtualTcamDbSegmentTableVTcamCombine
(
    INOUT  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC  *primaryTcamCfgPtr,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC  *secondaryTcamCfgPtr
)
{
    GT_STATUS   rc;

    /* combine segments from secondary into primary */
    rc = segmentTableVTcamFullCombine(primaryTcamCfgPtr,secondaryTcamCfgPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    if (prvCpssDxChVirtualTcamDbSegmentTablePrimaryTreeMrgSplTraceEnable != GT_FALSE)
    {
        cpssOsPrintf("after segmentTableVTcamFullCombine\n");
        prvCpssDxChVirtualTcamDbVTcamDumpSegmentsTree(primaryTcamCfgPtr->segmentsTree);
    }

    /***************************************************************/
    /* the vtcam now hold in the segments tree the needed segments */
    /* from the secondary tree                                     */
    /* and the secondary tree is no longer valid                   */
    /***************************************************************/

    /* there are no collides any more between the segments (not suppose to be) */
    /* but there may still be 'touching' segments that can be combined */
    /* segments with the same rows in parallel blocks also combined    */
    rc = prvCpssDxChVirtualTcamDbSegmentTableLuOrderedTreeMerge(
        primaryTcamCfgPtr->tcamSegCfgPtr,primaryTcamCfgPtr->segmentsTree);
    if(rc != GT_OK)
    {
        return rc;
    }

    if (prvCpssDxChVirtualTcamDbSegmentTablePrimaryTreeMrgSplTraceEnable != GT_FALSE)
    {
        cpssOsPrintf("after prvCpssDxChVirtualTcamDbSegmentTableLuOrderedTreeMerge\n");
        prvCpssDxChVirtualTcamDbVTcamDumpSegmentsTree(primaryTcamCfgPtr->segmentsTree);
    }

    /* update rule amounts */
    prvCpssDxChVirtualTcamDbSegmentTableUpdateRuleAmounts(
        primaryTcamCfgPtr->tcamSegCfgPtr, primaryTcamCfgPtr->segmentsTree);

    /* update the baseLogicalIndex in the segments of the vtcam */
    /* and primaryTcamCfgPtr->rulesAmount */
    segmentTableVTcamBaseLogicalIndexUpdate(primaryTcamCfgPtr);

    /* realloc logical to physical conversion array */
    if (primaryTcamCfgPtr->rulePhysicalIndexArr != NULL)
    {
        cpssOsFree(primaryTcamCfgPtr->rulePhysicalIndexArr);
    }
    primaryTcamCfgPtr->rulePhysicalIndexArr =
        (GT_U16*)cpssOsMalloc(
            (sizeof(GT_U16) * primaryTcamCfgPtr->rulesAmount));
    if (primaryTcamCfgPtr->rulePhysicalIndexArr == NULL)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }

    /* Re-Generate table for conversion logical indexes to physical indexes of rules. */
    prvCpssDxChVirtualTcamDbSegmentTableLogToPhyIndexTabGenerate(
        primaryTcamCfgPtr->tcamSegCfgPtr,
        primaryTcamCfgPtr->segmentsTree,
        primaryTcamCfgPtr->rulePhysicalIndexArr);

    /* check to compress the 'free' segments */
    prvCpssDxChVirtualTcamDbSegmentTableFreeSpaceMerge(primaryTcamCfgPtr->tcamSegCfgPtr);

    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableDetachSpace function
* @endinternal
*
* @brief   the function detach the last rules from the vtcam and restore them into
*         'free' segments tree
*         the function may remove one or more segments that hold the rules range.
*         the function may split one more segment to reduce number of rules in it.
*         NOTE: updates the vTcamInfoPtr->rulesAmount according to actual number of
*         rules that can be used by the vtcam after the operation.
*         this value may be more than numOfRulesToKeep due to rules granularity
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in,out] vTcamCfgPtr              - (pointer to) virtual TCAM structure
* @param[in] fromEndOrStart           - the type of direction
*                                      GT_TRUE  - hole from end   of range
*                                      GT_FALSE - hole from start of range
* @param[in] numOfRulesToKeep         - number of rules that need to keep , so may release
*                                      what ever is more than that.
*                                      NOTE: there may be 'align' issues that may release
*                                      a little less rules.
* @param[in,out] vTcamCfgPtr              - (pointer to)updated virtual TCAM structure
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       - the given tree has not been initialized.
* @retval GT_BAD_STATE             - In case the tree is not empty.
*/
GT_STATUS prvCpssDxChVirtualTcamDbSegmentTableDetachSpace
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr,
    INOUT  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC  *vTcamCfgPtr,
    IN     GT_BOOL                                      fromEndOrStart,
    IN     GT_U32                                       numOfRulesToKeep
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segEntryPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *newSplitSegPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segEntryToSplitPtr = NULL;
    PRV_CPSS_AVL_TREE_ID    avlTree;
    PRV_CPSS_AVL_TREE_PATH  currentPath;
    GT_VOID_PTR             dbEntryPtr;/* pointer to entry in DB */
    GT_U32  maxRulesToRemove;/*max rules that may be removed due to rules granularity*/
    PRV_CPSS_AVL_TREE_SEEK_ENT                        seekMode;
    GT_U32                                            totalRemoved = 0;
    GT_U32                                            rulesPerRow; /* number of rules per row            */
    GT_U32                                            granularity;
    GT_U32                                            ruleSizeInUnits;
    GT_U32                                            detachRowsNum;
    GT_U32                                            detachRulesNum;
    GT_U32                                            horzBlocksInRow;
    GT_U32                                            horzBlockIdx;
    GT_U32                                            horzBlockLoopIdx;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP segmentColumnsMap;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT     segmentColumns;
    GT_U32                                            mapCapacity;
    GT_U32                                            colCapacity;

    horzBlocksInRow =
        (tcamSegCfgPtr->tcamColumnsAmount
        / tcamSegCfgPtr->lookupColumnAlignment);

    CPSS_LOG_INFORMATION_MAC("start request to keep [%d] out of [%d] rules \n",
        numOfRulesToKeep , vTcamCfgPtr->rulesAmount);

    /* Granularity of detached space */
    ruleSizeInUnits  = prvCpssDxChVirtualTcamSegmentTableRuleSizeToUnits(
        tcamSegCfgPtr, vTcamCfgPtr->tcamInfo.ruleSize);
    granularity =
        prvCpssDxChVirtualTcamDbSegmentTableVTcamSizeGranularity(
            tcamSegCfgPtr, ruleSizeInUnits);
    if (granularity != 1)
    {
        /* around to nearest greater granularity-aligned number */
        numOfRulesToKeep += (granularity - 1);
        numOfRulesToKeep -= (numOfRulesToKeep % granularity);
    }

    if (vTcamCfgPtr->rulesAmount < numOfRulesToKeep)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (vTcamCfgPtr->rulesAmount == numOfRulesToKeep)
    {
        return GT_OK;
    }

    maxRulesToRemove = vTcamCfgPtr->rulesAmount - numOfRulesToKeep;

    /* find first segment in the vtcam that covers the new needed size */

    avlTree = vTcamCfgPtr->segmentsTree;

    if(fromEndOrStart == GT_TRUE)
    {
        seekMode = PRV_CPSS_AVL_TREE_SEEK_LAST_E;
    }
    else
    {
        seekMode = PRV_CPSS_AVL_TREE_SEEK_FIRST_E;
    }

    segEntryToSplitPtr = NULL;
    while (GT_TRUE == prvCpssAvlPathSeek(avlTree,seekMode,
                currentPath, &dbEntryPtr))
    {
        segEntryPtr = dbEntryPtr;

        if((segEntryPtr->rulesAmount + totalRemoved) <= maxRulesToRemove)
        {
            /* this segment can be fully removed from the list of vtcam */
            CPSS_LOG_INFORMATION_MAC("remove segment fully with [%d] rules\n",
                segEntryPtr->rulesAmount);

            /* calculate before freeing segment - it will be updated */
            totalRemoved += segEntryPtr->rulesAmount;

            prvCpssDxChVirtualTcamSegmentTableAvlItemRemove(
                vTcamCfgPtr->segmentsTree, segEntryPtr);
            prvCpssDxChVirtualTcamDbSegmentTableChainFreeSegmentWithSplit(
                tcamSegCfgPtr, segEntryPtr);

            /* not needed to split the next segment */
            if (totalRemoved >= maxRulesToRemove) break;
        }
        else
        {
            segEntryToSplitPtr = segEntryPtr;
            break;
        }
    }

    if (segEntryToSplitPtr)
    {
        /* split first or last block */
        rulesPerRow = prvCpssDxChVirtualTcamDbSegmentTableSegmentRowCapacity(
            tcamSegCfgPtr, segEntryToSplitPtr->segmentColumnsMap, ruleSizeInUnits);

        detachRowsNum = ((maxRulesToRemove - totalRemoved) / rulesPerRow);

        if (segEntryToSplitPtr->rowsAmount <= detachRowsNum)
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        if (detachRowsNum > 0)
        {
            /* newSplitSegPtr - to free space, segEntryToSplitPtr - reminder in VTCAM */
            newSplitSegPtr = segEntryToSplitPtr;
            if (fromEndOrStart == GT_TRUE)
            {
                rc = prvCpssDxChVirtualTcamDbSegmentTableCropSegmentVertical(
                    tcamSegCfgPtr, vTcamCfgPtr->segmentsTree /*tree1*/, NULL /*tree2*/,
                    newSplitSegPtr,
                    (newSplitSegPtr->rowsBase + newSplitSegPtr->rowsAmount - detachRowsNum)/*rowsBase*/,
                    detachRowsNum /*rowsAmount*/,
                    GT_FALSE /*putBack*/,
                    &segEntryToSplitPtr /*lowSegPtrPtr*/, NULL /*highSegPtrPtr*/);
            }
            else
            {
                rc = prvCpssDxChVirtualTcamDbSegmentTableCropSegmentVertical(
                    tcamSegCfgPtr, vTcamCfgPtr->segmentsTree /*tree1*/, NULL /*tree2*/,
                    newSplitSegPtr,
                    newSplitSegPtr->rowsBase/*rowsBase*/, detachRowsNum /*rowsAmount*/,
                    GT_FALSE /*putBack*/,
                    NULL /*lowSegPtrPtr*/, &segEntryToSplitPtr /*highSegPtrPtr*/);
            }
            if (rc != GT_OK)
            {
                return rc;
            }
            prvCpssDxChVirtualTcamDbSegmentTableChainFreeSegmentWithSplit(
                tcamSegCfgPtr, newSplitSegPtr);
            totalRemoved += (detachRowsNum * rulesPerRow);
        }

        detachRulesNum = (maxRulesToRemove - totalRemoved);

        if (detachRulesNum > 0)
        {
            segmentColumnsMap = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_EMPTY_CNS;
            mapCapacity = 0;
            for (horzBlockLoopIdx = 1; (horzBlockLoopIdx <= horzBlocksInRow); horzBlockLoopIdx++)
            {
                if (fromEndOrStart == GT_TRUE)
                {
                    horzBlockIdx = (horzBlocksInRow - horzBlockLoopIdx);
                }
                else
                {
                    horzBlockIdx = (horzBlockLoopIdx - 1);
                }
                segmentColumns =
                    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
                        segEntryToSplitPtr->segmentColumnsMap, horzBlockIdx);
                if (segmentColumns ==
                    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E) continue;
                colCapacity =
                    prvCpssDxChVirtualTcamDbSegmentTableMaxRulesAmountPerColumnsTypeGet(
                        segmentColumns, ruleSizeInUnits);
                if ((mapCapacity + colCapacity) <= detachRulesNum)
                {
                    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
                        segmentColumnsMap, horzBlockIdx, segmentColumns);
                    mapCapacity += colCapacity;
                    if (mapCapacity == detachRulesNum) break;
                }
            }
            if (mapCapacity > 0)
            {
                if (segEntryToSplitPtr->rowsAmount > 1)
                {
                    /* segEntryToSplitPtr - to horizontal split */
                    if (fromEndOrStart == GT_TRUE)
                    {
                        rc = prvCpssDxChVirtualTcamDbSegmentTableCropSegmentVertical(
                            tcamSegCfgPtr, vTcamCfgPtr->segmentsTree /*tree1*/, NULL /*tree2*/,
                            segEntryToSplitPtr,
                            (segEntryToSplitPtr->rowsBase + segEntryToSplitPtr->rowsAmount - 1) /*rowsBase*/,
                            1 /*rowsAmount*/,
                            GT_TRUE /*putBack*/,
                            NULL /*lowSegPtrPtr*/, NULL/*highSegPtrPtr*/);
                    }
                    else
                    {
                        rc = prvCpssDxChVirtualTcamDbSegmentTableCropSegmentVertical(
                            tcamSegCfgPtr, vTcamCfgPtr->segmentsTree /*tree1*/, NULL /*tree2*/,
                            segEntryToSplitPtr,
                            segEntryToSplitPtr->rowsBase /*rowsBase*/, 1 /*rowsAmount*/,
                            GT_TRUE /*putBack*/,
                            NULL /*lowSegPtrPtr*/, NULL/*highSegPtrPtr*/);
                    }
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }
                rc = prvCpssDxChVirtualTcamDbSegmentTableCropSegmentHorizontal(
                    tcamSegCfgPtr, vTcamCfgPtr->segmentsTree /*tree1*/, NULL /*tree2*/,
                    segEntryToSplitPtr, segmentColumnsMap,
                    GT_FALSE /*putBack*/, NULL /*remainSegPtrPtr*/);
                if (rc != GT_OK)
                {
                    return rc;
                }
                prvCpssDxChVirtualTcamDbSegmentTableChainFreeSegmentWithSplit(
                    tcamSegCfgPtr, segEntryToSplitPtr);
                totalRemoved += mapCapacity;
            }
        }
    }

    /* update rule amounts */
    prvCpssDxChVirtualTcamDbSegmentTableUpdateRuleAmounts(
        tcamSegCfgPtr, vTcamCfgPtr->segmentsTree);

    /* update the baseLogicalIndex in the segments of the vtcam */
    /* and primaryTcamCfgPtr->rulesAmount */
    segmentTableVTcamBaseLogicalIndexUpdate(vTcamCfgPtr);

    CPSS_LOG_INFORMATION_MAC("ended request . actual keep [%d] rules (requested [%d]) , after removed [%d]\n",
        vTcamCfgPtr->rulesAmount,numOfRulesToKeep,totalRemoved);

    if(fromEndOrStart == GT_TRUE)
    {
        /* NOTE:
            no need to update vTcamCfgPtr->rulePhysicalIndexArr
            because it only hold 'extra' mapping , but it not harm the operation.
        */
    }
    else
    {
        /* NOTE:
            no need to update vTcamCfgPtr->rulePhysicalIndexArr
            because it was needed to be recalculated by the caller before this stage.
        */
    }

    /* Check availability util uses VTCAM DB without Logical to Physical */
    /* conversion table - bypass it's regeneration                       */
    if (vTcamCfgPtr->rulePhysicalIndexArr)
    {
        /* Re-Generate table for conversion logical indexes to physical indexes of rules. */
        prvCpssDxChVirtualTcamDbSegmentTableLogToPhyIndexTabGenerate(
            tcamSegCfgPtr,
            vTcamCfgPtr->segmentsTree,
            vTcamCfgPtr->rulePhysicalIndexArr);
    }

    /* check to compress the 'free' segments */
    prvCpssDxChVirtualTcamDbSegmentTableFreeSpaceMerge(tcamSegCfgPtr);

    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableAllTcamCapacity function
* @endinternal
*
* @brief   Get Maximal amount of rules that can be placed in all TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] ruleSize                 - rule size enum element.
*                                       amount of rules that can be placed in all TCAM.
*/
GT_U32 prvCpssDxChVirtualTcamDbSegmentTableAllTcamCapacity
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr,
    IN     CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT       ruleSize
)
{
    GT_U32                                        ruleSizeInMinRules;
    GT_U32                                        blockRowsAmount;
    GT_U32                                        blockRowsCapacity;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT blockRowType;
    ruleSizeInMinRules =
        prvCpssDxChVirtualTcamSegmentTableRuleSizeToUnits(
            tcamSegCfgPtr, ruleSize);

    /* error treated in called function */
    if (ruleSizeInMinRules == 0) return 0;

    blockRowsAmount =
        ((tcamSegCfgPtr->tcamRowsAmount
          * tcamSegCfgPtr->tcamColumnsAmount)
          / tcamSegCfgPtr->lookupColumnAlignment);

    switch (tcamSegCfgPtr->deviceClass)
    {
    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP5_E:
        if (ruleSizeInMinRules == 8)
        {
            return (blockRowsAmount / 2);
        }
        blockRowType = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E;
        break;
    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_E:
        blockRowType = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_ALL_E;
        break;
    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_TTI_E:
        blockRowType = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_TTI_ALL_E;
        break;
    default:
        blockRowType = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E;
        break;
    }

    blockRowsCapacity =
        prvCpssDxChVirtualTcamDbSegmentTableRowCapacity(
            blockRowType, ruleSizeInMinRules);
    return (blockRowsAmount * blockRowsCapacity);
}


