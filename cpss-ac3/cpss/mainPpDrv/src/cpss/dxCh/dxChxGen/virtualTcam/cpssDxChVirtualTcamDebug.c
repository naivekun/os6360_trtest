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
* @file cpssDxChVirtualTcamDebug.c
*
* @brief The CPSS DXCH Debug/verification tools for Virtual TCAM Manager
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/cpssDxChVirtualTcam.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/private/prvCpssDxChVirtualTcam.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/private/prvCpssDxChVirtualTcamDb.h>
#include <cpss/dxCh/dxChxGen/tcam/private/prvCpssDxChTcam.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwTables.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/private/prvCpssDxChVirtualTcamLog.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpssCommon/private/prvCpssSkipList.h>

/* condition to check that the last index in pritority range used by rule */
static GT_BOOL prvCpssDxChVirtualTcamDbVTcamRulesLayoutVerify_priorityRangeLastUsed = GT_TRUE;

/**
* @internal prvCpssDxChVirtualTcamDbVTcamRulesLayoutVerify function
* @endinternal
*
* @brief   Verify DB info about Virtual TCAM Rules layout.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] stopAfterFirstError      - GT_TRUE  - return after first error
*                                      - GT_FALSE - continue checking
* @param[in] printErrorMessage        - GT_TRUE  - print error messages
*                                      GT_FALSE - count errors only
*
* @retval GT_OK                    - no errors found,
* @retval other                    - the code of the last error.
*/
GT_STATUS prvCpssDxChVirtualTcamDbVTcamRulesLayoutVerify
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_U32                                      vTcamId,
    IN  GT_BOOL                                     stopAfterFirstError,
    IN  GT_BOOL                                     printErrorMessage
)
{
    GT_STATUS  totalRc;      /* last not OK returned code */
    PRV_CPSS_AVL_TREE_PATH      avlTreePath;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC*     vtcamInfoPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC ruleIdEntry, * ruleIdEntryPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC    *foundPriorityEntryPtr;
    GT_VOID_PTR                 dbEntryPtr;  /* pointer to entry in DB             */
    PRV_CPSS_AVL_TREE_SEEK_ENT  seekMode;
    GT_U32  ii;
    GT_U32  actualUsed = 0;
    GT_U32  lastIndexFromPriority = 0xFFFFFFFF;
    GT_U32  savedIndexAfterPriority;
    GT_U32  emptyPriorityRangeLow;
    GT_U32  emptyPriorityRangeHigh;
    GT_U32  prevPrio = 0;
    GT_U32  errorCounter;

    totalRc = GT_OK;
    errorCounter = 0;

    /* get vtcam DB */
    vtcamInfoPtr = prvCpssDxChVirtualTcamDbVTcamGet(vTcamMngId,vTcamId);
    if(vtcamInfoPtr == NULL)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        if (printErrorMessage != GT_FALSE)
        {
            cpssOsPrintf(
                "prvCpssDxChVirtualTcamDbVTcamRulesLayoutVerify vTcamMngId [%d] vTcamId [%d] BAD PARAM\n",
                vTcamMngId ,vTcamId);
        }
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    ruleIdEntry.ruleId = 0;
    ruleIdEntry.applicationDataPtr = NULL;

#define CHECK_TREE_ORDER_IS_CORRECT_MAC(treeId)                           \
    if(vtcamInfoPtr->treeId)                                              \
    {                                                                     \
        GT_BOOL treeIsOk = prvCpssAvlTreeIsOrderCorrect(vtcamInfoPtr->treeId); \
        if(treeIsOk == GT_FALSE)                                          \
        {                                                                 \
            prvCpssDxChVirtualTcamDbSegmentTableException();              \
            errorCounter ++;                                              \
            totalRc = GT_BAD_STATE;                                       \
            if (printErrorMessage != GT_FALSE)                            \
            {                                                             \
                cpssOsPrintf(                                             \
                    "prvCpssDxChVirtualTcamDbVTcamRulesLayoutVerify vTcamMngId [%d] vTcamId [%d] [%s] corrupted\n", \
                    vTcamMngId ,vTcamId,                                  \
                    #treeId);                                             \
            }                                                             \
        }                                                                 \
    }

    /**************************************************
        check trees
    ***************************************************/
    CHECK_TREE_ORDER_IS_CORRECT_MAC(segmentsTree);
    CHECK_TREE_ORDER_IS_CORRECT_MAC(ruleIdIdTree);
    CHECK_TREE_ORDER_IS_CORRECT_MAC(ruleIdIndexTree);
    CHECK_TREE_ORDER_IS_CORRECT_MAC(priorityPriTree);
    CHECK_TREE_ORDER_IS_CORRECT_MAC(priorityIndexTree);

    if(totalRc != GT_OK)
    {
        if (stopAfterFirstError != GT_FALSE)
        {
            return totalRc;
        }
    }

    /**************************************************
        check that logical index and ruleId coherent in the DB.
    ***************************************************/
    for (ii = 0; ii < vtcamInfoPtr->rulesAmount; ii++)
    {
        if (vtcamInfoPtr->usedRulesBitmapArr[ii>>5] & (1<<(ii & 0x1f)))
        {
            actualUsed++;

            if(vtcamInfoPtr->tcamInfo.ruleAdditionMethod == CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E &&
               vtcamInfoPtr->ruleIdIndexTree )
            {
                /**************************************************
                    check that valid logical index also valid in the tree : ruleIdIndexTree
                ***************************************************/

                ruleIdEntry.logicalIndex = ii;
                ruleIdEntryPtr = prvCpssAvlSearch(vtcamInfoPtr->ruleIdIndexTree,&ruleIdEntry);
                if(ruleIdEntryPtr == NULL)
                {
                    prvCpssDxChVirtualTcamDbSegmentTableException();
                    errorCounter ++;
                    totalRc = GT_NOT_FOUND;
                    if (printErrorMessage != GT_FALSE)
                    {
                        cpssOsPrintf(
                            "VTcamRulesLayoutVerify: logical index [%d] missing in vtcamInfoPtr->ruleIdIndexTree\n", ii);
                    }
                    if (stopAfterFirstError != GT_FALSE)
                    {
                        return totalRc;
                    }
                }
                else
                {
                    /**************************************************
                        check that ruleId that was derived from tree ruleIdIndexTree
                        also exists in tree : ruleIdIdTree
                    ***************************************************/
                    ruleIdEntry.ruleId = ruleIdEntryPtr->ruleId;
                    ruleIdEntryPtr = prvCpssAvlSearch(vtcamInfoPtr->ruleIdIdTree,&ruleIdEntry);
                    if(ruleIdEntryPtr == NULL)
                    {
                        prvCpssDxChVirtualTcamDbSegmentTableException();
                        errorCounter ++;
                        totalRc = GT_NOT_FOUND;
                        if (printErrorMessage != GT_FALSE)
                        {
                            cpssOsPrintf(
                                "VTcamRulesLayoutVerify: RuleId [%d] missing in vtcamInfoPtr->ruleIdIdTree\n",
                                ruleIdEntry.ruleId);
                        }
                        if (stopAfterFirstError != GT_FALSE)
                        {
                            return totalRc;
                        }
                    }
                }
            }
        }
        else
        {
            /**************************************************
                check that non valid logical index also not exist in the tree : ruleIdIndexTree
            ***************************************************/
            if(vtcamInfoPtr->tcamInfo.ruleAdditionMethod == CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E)
            {
                ruleIdEntry.logicalIndex = ii;
                ruleIdEntryPtr = prvCpssAvlSearch(vtcamInfoPtr->ruleIdIndexTree,&ruleIdEntry);
                if(ruleIdEntryPtr != NULL)
                {
                    prvCpssDxChVirtualTcamDbSegmentTableException();
                    errorCounter ++;
                    totalRc = GT_NOT_FOUND;
                    if (printErrorMessage != GT_FALSE)
                    {
                        cpssOsPrintf(
                            "VTcamRulesLayoutVerify: logical index [%d] found (with ruleId[%d])in vtcamInfoPtr->ruleIdIndexTree (but not in usedRulesBitmapArr[])\n",
                            ii,ruleIdEntryPtr->ruleId);
                    }
                    if (stopAfterFirstError != GT_FALSE)
                    {
                        return totalRc;
                    }
                }
            }
        }
    }

    /**************************************************
        check that actual number of logical indexes set in the usedRulesBitmapArr[]
        match the number of usedRulesAmount
    ***************************************************/
    if (vtcamInfoPtr->usedRulesAmount != actualUsed)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        errorCounter ++;
        totalRc = GT_BAD_STATE;
        if (printErrorMessage != GT_FALSE)
        {
            cpssOsPrintf(
                "VTcamRulesLayoutVerify: Different vtcamInfoPtr->usedRulesAmount [%d] and actualUsed [%d] from vtcamInfoPtr->usedRulesBitmapArr \n",
                vtcamInfoPtr->usedRulesAmount, actualUsed);
        }
        if (stopAfterFirstError != GT_FALSE)
        {
            return totalRc;
        }
    }

    /**************************************************
        check priority table
    ***************************************************/
    if(vtcamInfoPtr->tcamInfo.ruleAdditionMethod == CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E &&
        vtcamInfoPtr->priorityPriTree)
    {
        savedIndexAfterPriority = 0;

        seekMode = PRV_CPSS_AVL_TREE_SEEK_FIRST_E;
        while(GT_TRUE == prvCpssAvlPathSeek(vtcamInfoPtr->priorityPriTree,
                seekMode,
                avlTreePath,
                &dbEntryPtr /* use dedicated var to avoid warnings */))
        {
            seekMode = PRV_CPSS_AVL_TREE_SEEK_NEXT_E;
            foundPriorityEntryPtr = dbEntryPtr;

            /**************************************************
                check that the tree priorityPriTree not hold ZERO size ranges.
            ***************************************************/
            if (foundPriorityEntryPtr->rangeSize == 0)
            {
                prvCpssDxChVirtualTcamDbSegmentTableException();
                errorCounter ++;
                totalRc = GT_ALREADY_EXIST;
                if (printErrorMessage != GT_FALSE)
                {
                    cpssOsPrintf(
                        "VTcamRulesLayoutVerify: priority [%d] rangeSize == 0 (DB should not hold those) \n",
                        foundPriorityEntryPtr->priority);
                }
                if (stopAfterFirstError != GT_FALSE)
                {
                    return totalRc;
                }
            }

            /**************************************************
                check that priority info not violate rulesAmount.
            ***************************************************/
            if ((foundPriorityEntryPtr->baseLogIndex + foundPriorityEntryPtr->rangeSize) >
                vtcamInfoPtr->rulesAmount)
            {
                errorCounter ++;
                totalRc = GT_ALREADY_EXIST;
                if (printErrorMessage != GT_FALSE)
                {
                    prvCpssDxChVirtualTcamDbSegmentTableException();
                    cpssOsPrintf(
                        "VTcamRulesLayoutVerify: priority [%d] baseLogIndex [%d] + rangeSize[%d] violate rulesAmount[%d] \n",
                        foundPriorityEntryPtr->baseLogIndex,
                        foundPriorityEntryPtr->rangeSize,
                        vtcamInfoPtr->rulesAmount);
                }
                if (stopAfterFirstError != GT_FALSE)
                {
                    return totalRc;
                }
            }

            if(lastIndexFromPriority == 0xFFFFFFFF)
            {
                /* first time nothing to compare */
            }
            else
            {
                /**************************************************
                    check that previous range not violate into current priority
                    range.
                ***************************************************/
                if(lastIndexFromPriority >= foundPriorityEntryPtr->baseLogIndex)
                {
                    prvCpssDxChVirtualTcamDbSegmentTableException();
                    errorCounter ++;
                    totalRc = GT_ALREADY_EXIST;
                    if (printErrorMessage != GT_FALSE)
                    {
                        cpssOsPrintf(
                            "VTcamRulesLayoutVerify: priority [%d] starts at[%d] collide with end of previous priority [%] that ends at[%d]\n",
                            foundPriorityEntryPtr->priority,
                            foundPriorityEntryPtr->baseLogIndex,
                            prevPrio,
                            lastIndexFromPriority);
                    }
                    if (stopAfterFirstError != GT_FALSE)
                    {
                        return totalRc;
                    }
                }
            }
            /* save the end of range */
            lastIndexFromPriority = foundPriorityEntryPtr->baseLogIndex +
                                    foundPriorityEntryPtr->rangeSize - 1;


            prevPrio = foundPriorityEntryPtr->priority;

            /**************************************************
                check that no logical index is set between current priority range
               and the end of the previous range
            ***************************************************/
            emptyPriorityRangeLow = savedIndexAfterPriority;
            emptyPriorityRangeHigh = foundPriorityEntryPtr->baseLogIndex;
            if (emptyPriorityRangeLow < emptyPriorityRangeHigh)
            {
                for (ii = emptyPriorityRangeLow; (ii < emptyPriorityRangeHigh); ii++)
                {
                    if (vtcamInfoPtr->usedRulesBitmapArr[ii>>5] & (1<<(ii & 0x1f)))
                    {
                        prvCpssDxChVirtualTcamDbSegmentTableException();
                        errorCounter ++;
                        totalRc = GT_BAD_STATE;
                        if (printErrorMessage != GT_FALSE)
                        {
                            cpssOsPrintf(
                                "VTcamRulesLayoutVerify: logical index [%d] out of priority ranges\n", ii);
                        }
                        if (stopAfterFirstError != GT_FALSE)
                        {
                            return totalRc;
                        }
                    }
                }
            }
            savedIndexAfterPriority = foundPriorityEntryPtr->baseLogIndex
                + foundPriorityEntryPtr->rangeSize;

            if (prvCpssDxChVirtualTcamDbVTcamRulesLayoutVerify_priorityRangeLastUsed == GT_TRUE)
            {
                /**************************************************
                    check that the last index in the range of priority is set
                    (the 'delete' rule API should make sure of it)
                ***************************************************/
                ii = foundPriorityEntryPtr->baseLogIndex
                    + foundPriorityEntryPtr->rangeSize - 1;
                if ((vtcamInfoPtr->usedRulesBitmapArr[ii>>5] & (1<<(ii & 0x1f))) == 0)
                {
                    prvCpssDxChVirtualTcamDbSegmentTableException();
                    errorCounter ++;
                    totalRc = GT_ALREADY_EXIST;
                    if (printErrorMessage != GT_FALSE)
                    {
                        cpssOsPrintf(
                            "VTcamRulesLayoutVerify: priority [%d] rangeSize [%d] last index unused (but expected to be)\n",
                            foundPriorityEntryPtr->priority, foundPriorityEntryPtr->rangeSize);
                    }
                    if (stopAfterFirstError != GT_FALSE)
                    {
                        return totalRc;
                    }
                }
            }
        }

        /**************************************************
            check that no logical index is set after last priority range
        ***************************************************/
        if(vtcamInfoPtr->rulesAmount)
        {
            emptyPriorityRangeLow = savedIndexAfterPriority;
            emptyPriorityRangeHigh = vtcamInfoPtr->rulesAmount - 1;
            if (emptyPriorityRangeLow < emptyPriorityRangeHigh)
            {
                for (ii = emptyPriorityRangeLow; (ii < emptyPriorityRangeHigh); ii++)
                {
                    if (vtcamInfoPtr->usedRulesBitmapArr[ii>>5] & (1<<(ii & 0x1f)))
                    {
                        prvCpssDxChVirtualTcamDbSegmentTableException();
                        errorCounter ++;
                        totalRc = GT_BAD_STATE;
                        if (printErrorMessage != GT_FALSE)
                        {
                            cpssOsPrintf(
                                "VTcamRulesLayoutVerify: logical index [%d] out of priority ranges (after last priority range) \n", ii);
                        }
                        if (stopAfterFirstError != GT_FALSE)
                        {
                            return totalRc;
                        }
                    }
                }
            }
        }
    }

    /**************************************************
        check that the HW indexes for the logical indexes are in ascending order
    ***************************************************/
    if (vtcamInfoPtr->rulesAmount >= 2)
    {
        for (ii = 0; (ii < (vtcamInfoPtr->rulesAmount - 1)); ii++)
        {
            if (vtcamInfoPtr->rulePhysicalIndexArr[ii]
                >= vtcamInfoPtr->rulePhysicalIndexArr[ii + 1])
            {
                prvCpssDxChVirtualTcamDbSegmentTableException();
                errorCounter ++;
                totalRc = GT_BAD_STATE;
                if (printErrorMessage != GT_FALSE)
                {
                    cpssOsPrintf(
                        "VTcamRulesLayoutVerify: logical index [%d] hold HW index [%d] that is not less than the HW index [%d] of next logical index \n",
                            ii,
                            vtcamInfoPtr->rulePhysicalIndexArr[ii],
                            vtcamInfoPtr->rulePhysicalIndexArr[ii+1]);
                }
                if (stopAfterFirstError != GT_FALSE)
                {
                    return totalRc;
                }
            }
        }
    }

    if ((printErrorMessage != GT_FALSE) && (errorCounter != 0))
    {
        cpssOsPrintf(
            "prvCpssDxChVirtualTcamDbVTcamRulesLayoutVerify vTcamMngId [%d] vTcamId [%d] total errors [%d] \n",
            vTcamMngId ,vTcamId, errorCounter);
    }
    return totalRc;
}

/**
* @internal prvCpssDxChVirtualTcamDbVerifySegment function
* @endinternal
*
* @brief   Verify TCAM segment layout.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] segmentPtr               - (pointer to) segment DB entry
* @param[in] rowsAmount               - amount of rows in TCAM
* @param[in] blockColumnsNum          - amount of columns in block
* @param[in] tcamColumnsNum           - amount of  columns in TCAM
* @param[in,out] columnBmpArr[]           - array of column bitmaps per row (checked)
* @param[in] stopAfterFirstError      - GT_TRUE  - return after first error
*                                      - GT_FALSE - continue checking
* @param[in] printErrorMessage        - GT_TRUE  - print error messages
*                                      GT_FALSE - count errors only
* @param[in,out] errorCounterPtr          - (pointer to) error counter
* @param[in,out] columnBmpArr[]           - array of column bitmaps per row (updated)
* @param[in,out] errorCounterPtr          - (pointer to) error counter
*
* @retval GT_OK                    - no errors found,
* @retval other                    - the code of the last error.
*/
static GT_STATUS prvCpssDxChVirtualTcamDbVerifySegment
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segmentPtr,
    IN     GT_U32                                          rowsAmount,
    IN     GT_U32                                          tcamColumnsNum,
    IN     GT_U32                                          blockColumnsNum,
    INOUT  GT_U16                                          columnBmpArr[],
    IN     GT_BOOL                                         stopAfterFirstError,
    IN     GT_BOOL                                         printErrorMessage,
    INOUT  GT_U32                                          *errorCounterPtr
)
{
    GT_STATUS totalRc; /* last not GT_OK returned code       */
    GT_U32    row;     /* TCAM row                           */
    GT_U32    maxRow;  /* TCAM row upper bound of segment    */
    GT_U32    segColumnsBitmap; /* bitmap of 10-byte columns */

    segColumnsBitmap =
        prvCpssDxChVirtualTcamDbSegmentTableSegColMapToMinRuleColumnsBitmap(
            tcamColumnsNum, blockColumnsNum, segmentPtr->segmentColumnsMap);

    totalRc = GT_OK;
    maxRow  = (segmentPtr->rowsBase + segmentPtr->rowsAmount);
    if (maxRow > rowsAmount)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        (*errorCounterPtr) ++;
        totalRc = GT_BAD_STATE;
        if (printErrorMessage != GT_FALSE)
        {
            cpssOsPrintf(
                "VirtualTcamDbVerifySegment: segment out of TCAM rowsBase [%d] rowsAmount [%d]\n",
                segmentPtr->rowsBase, segmentPtr->rowsAmount);
        }
        if (stopAfterFirstError != GT_FALSE)
        {
            return totalRc;
        }
    }

    for (row = segmentPtr->rowsBase; (row < maxRow); row++)
    {
        if ((segColumnsBitmap & columnBmpArr[row]) != 0)
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            (*errorCounterPtr) ++;
            totalRc = GT_BAD_STATE;
            if (printErrorMessage != GT_FALSE)
            {
                cpssOsPrintf(
                    "VirtualTcamDbVerifySegment: overlap: row [%d] columns [0x%X] vTcamId [%d]\n",
                    row, (segColumnsBitmap & columnBmpArr[row]), segmentPtr->vTcamId);
            }
            if (stopAfterFirstError != GT_FALSE)
            {
                return totalRc;
            }
        }
        columnBmpArr[row] |= (GT_U16)segColumnsBitmap;
    }

    return totalRc;
}

/*******************************************************************************
* prvCpssDxChVirtualTcamDbVerifySegmentTree
*
* APPLICABLE DEVICES:
*        xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* INPUTS:
*       segmentsPtr         - (pointer to) segment DB entry
*       rowsAmount          - amount of rows in TCAM
*       tcamColumnsNum      - amount of columns in TCAM
*       blockColumnsNum     - amount of columns in block
*       columnBmpArr        - array of column bitmaps per row (checked)
*       stopAfterFirstError - GT_TRUE  - return after first error
*                           - GT_FALSE - continue checking
*       printErrorMessage   - GT_TRUE  - print error messages
*                             GT_FALSE - count errors only
*       errorCounterPtr     - (pointer to) error counter
*
* OUTPUTS:
*       columnBmpArr        - array of column bitmaps per row (updated)
*       errorCounterPtr     - (pointer to) error counter
*
* RETURNS:
*       GT_OK               - no errors found,
*       other               - the code of the last error.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static GT_STATUS prvCpssDxChVirtualTcamDbVerifySegmentTree
(
    IN     PRV_CPSS_AVL_TREE_ID                            segmentsTree,
    IN     GT_U32                                          rowsAmount,
    IN     GT_U32                                          tcamColumnsNum,
    IN     GT_U32                                          blockColumnsNum,
    INOUT  GT_U16                                          columnBmpArr[],
    IN     GT_BOOL                                         stopAfterFirstError,
    IN     GT_BOOL                                         printErrorMessage,
    INOUT  GT_U32                                          *errorCounterPtr
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segmentPtr; /* pointer to segnent DB entry   */
    GT_STATUS                                       totalRc;     /* last not GT_OK returned code  */
    GT_STATUS                                       rc;          /* returned code                 */
    PRV_CPSS_AVL_TREE_SEEK_ENT                      seekType;    /* seek type                     */
    GT_VOID                                         *dbEntryPtr; /* DB Entry pointer              */
    PRV_CPSS_AVL_TREE_PATH                          path;        /* tree iterator                 */

    totalRc = GT_OK;

    /* pass segments to segments of the tree */
    for (seekType = PRV_CPSS_AVL_TREE_SEEK_FIRST_E;
         (GT_FALSE != prvCpssAvlPathSeek(
          segmentsTree, seekType, path, &dbEntryPtr));
         seekType = PRV_CPSS_AVL_TREE_SEEK_NEXT_E)
    {
        segmentPtr = (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC*)dbEntryPtr;
        rc = prvCpssDxChVirtualTcamDbVerifySegment(
            segmentPtr, rowsAmount, tcamColumnsNum, blockColumnsNum, columnBmpArr,
            stopAfterFirstError, printErrorMessage, errorCounterPtr);
        if (rc != GT_OK)
        {
            totalRc = rc;
            if (stopAfterFirstError != GT_FALSE)
            {
                return totalRc;
            }
        }
    }
    return totalRc;
}

/**
* @internal prvCpssDxChVirtualTcamDbVerifyFullTcamLayout function
* @endinternal
*
* @brief   Verify DB info about full TCAM layout.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
* @param[in] stopAfterFirstError      - GT_TRUE  - return after first error
*                                      - GT_FALSE - continue checking
* @param[in] printErrorMessage        - GT_TRUE  - print error messages
*                                      GT_FALSE - count errors only
*
* @retval GT_OK                    - no errors found,
* @retval other                    - the code of the last error.
*/
GT_STATUS prvCpssDxChVirtualTcamDbVerifyFullTcamLayout
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_BOOL                                     stopAfterFirstError,
    IN  GT_BOOL                                     printErrorMessage
)
{
    GT_STATUS                                     rc;           /* return code               */
    GT_STATUS                                     totalRc;      /* last not OK returned code */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC*           vTcamMngPtr;  /* pointer to vTcam Manager  */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC*  vTcamCfgPtr;  /* pointer to vTcam          */
    GT_U32                                        vTcamId;      /* vTcamId                   */
    GT_U32                                        errorCounter; /* error counter             */
    GT_U32                                        rowsAmount;   /* rows amount               */
    GT_U16                                        *columnBmpArr; /* column Bitmap Array      */
    GT_U16                                        *columnTtiBmpArr; /* column Bitmap Array      */
    GT_U16                                        fullColBmp;   /* full columns bitmap       */
    GT_U32                                        row;          /* rows index                */

    vTcamMngPtr = prvCpssDxChVirtualTcamDbVTcamMngGet(vTcamMngId);
    if (vTcamMngPtr == NULL)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        if (printErrorMessage != GT_FALSE)
        {
            cpssOsPrintf(
                "VirtualTcamDbVerifyFullTcamLayout vTcamMngId [%d] BAD PARAM\n",
                vTcamMngId);
        }
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (vTcamMngPtr->vTcamCfgPtrArr == NULL)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        if (printErrorMessage != GT_FALSE)
        {
            cpssOsPrintf(
                "VirtualTcamDbVerifyFullTcamLayout vTcamMngId [%d] vTcamCfgPtrArr == NULL\n",
                vTcamMngId);
        }
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    if ((vTcamMngPtr->tcamSegCfg.tcamColumnsAmount > 16) ||
        (vTcamMngPtr->tcamTtiSegCfg.tcamColumnsAmount > 16))
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        if (printErrorMessage != GT_FALSE)
        {
            cpssOsPrintf(
                "VirtualTcamDbVerifyFullTcamLayout vTcamMngId [%d] tcamColumnsAmount > 16 not supported\n",
                vTcamMngId);
        }
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    totalRc = GT_OK;
    errorCounter = 0;
    rowsAmount = vTcamMngPtr->tcamSegCfg.tcamRowsAmount +
                    vTcamMngPtr->tcamTtiSegCfg.tcamRowsAmount;
    columnBmpArr = (GT_U16*)cpssOsMalloc(rowsAmount * sizeof(GT_U16));
    if (columnBmpArr == NULL)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        if (printErrorMessage != GT_FALSE)
        {
            cpssOsPrintf(
                "VirtualTcamDbVerifyFullTcamLayout no memory fo work data\n");
        }
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }
    cpssOsMemSet(columnBmpArr, 0, (rowsAmount * sizeof(GT_U16)));
    columnTtiBmpArr = &columnBmpArr[vTcamMngPtr->tcamSegCfg.tcamRowsAmount];

    rc = prvCpssDxChVirtualTcamDbVerifySegmentTree(
        vTcamMngPtr->tcamSegCfg.segFreeLuTree,
        vTcamMngPtr->tcamSegCfg.tcamRowsAmount,
        vTcamMngPtr->tcamSegCfg.tcamColumnsAmount,
        vTcamMngPtr->tcamSegCfg.lookupColumnAlignment,
        columnBmpArr,
        stopAfterFirstError, printErrorMessage, &errorCounter);
    if (rc != GT_OK)
    {
        totalRc = rc;
        if (stopAfterFirstError != GT_FALSE)
        {
            cpssOsFree(columnBmpArr);
            prvCpssDxChVirtualTcamDbVTcamDumpFullTcamLayoutByPtr(&vTcamMngPtr->tcamSegCfg);
            return totalRc;
        }
    }
    rc = prvCpssDxChVirtualTcamDbVerifySegmentTree(
        vTcamMngPtr->tcamTtiSegCfg.segFreeLuTree,
        vTcamMngPtr->tcamTtiSegCfg.tcamRowsAmount,
        vTcamMngPtr->tcamTtiSegCfg.tcamColumnsAmount,
        vTcamMngPtr->tcamTtiSegCfg.lookupColumnAlignment,
        columnTtiBmpArr,
        stopAfterFirstError, printErrorMessage, &errorCounter);
    if (rc != GT_OK)
    {
        totalRc = rc;
        if (stopAfterFirstError != GT_FALSE)
        {
            cpssOsFree(columnBmpArr);
            prvCpssDxChVirtualTcamDbVTcamDumpFullTcamLayoutByPtr(&vTcamMngPtr->tcamTtiSegCfg);
            return totalRc;
        }
    }

    for (vTcamId = 0; (vTcamId < vTcamMngPtr->vTcamCfgPtrArrSize); vTcamId++)
    {
        vTcamCfgPtr = vTcamMngPtr->vTcamCfgPtrArr[vTcamId];
        if (vTcamCfgPtr == NULL) continue;

        rc = prvCpssDxChVirtualTcamDbVerifySegmentTree(
            vTcamCfgPtr->segmentsTree, rowsAmount,
            vTcamCfgPtr->tcamSegCfgPtr->tcamColumnsAmount,
            vTcamCfgPtr->tcamSegCfgPtr->lookupColumnAlignment,
            (vTcamCfgPtr->tcamSegCfgPtr == &vTcamMngPtr->tcamSegCfg) ?
                columnBmpArr :
                columnTtiBmpArr,
            stopAfterFirstError, printErrorMessage, &errorCounter);
        if (rc != GT_OK)
        {
            totalRc = rc;
            if (stopAfterFirstError != GT_FALSE)
            {
                cpssOsFree(columnBmpArr);
                prvCpssDxChVirtualTcamDbVTcamDumpFullTcamLayoutByPtr(vTcamCfgPtr->tcamSegCfgPtr);
                return totalRc;
            }
        }
    }
    /* Check for lost segments */

    fullColBmp = (GT_U16)((1 << vTcamMngPtr->tcamSegCfg.tcamColumnsAmount) - 1);
    for (row = 0; (row < vTcamMngPtr->tcamSegCfg.tcamRowsAmount); row++)
    {
        if (fullColBmp != columnBmpArr[row])
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            totalRc = GT_BAD_STATE;
            if (printErrorMessage != GT_FALSE)
            {
                cpssOsPrintf(
                    "VirtualTcamDbVerifyFullTcamLayout found lost memory in row [%d]\n", row);
            }
            if (stopAfterFirstError != GT_FALSE)
            {
                cpssOsFree(columnBmpArr);
                prvCpssDxChVirtualTcamDbVTcamDumpFullTcamLayoutByPtr(&vTcamMngPtr->tcamSegCfg);
                return totalRc;
            }
        }
    }

    fullColBmp = (GT_U16)((1 << vTcamMngPtr->tcamTtiSegCfg.tcamColumnsAmount) - 1);
    for (; (row < rowsAmount); row++)
    {
        if (fullColBmp != columnBmpArr[row])
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            totalRc = GT_BAD_STATE;
            if (printErrorMessage != GT_FALSE)
            {
                cpssOsPrintf(
                    "VirtualTcamDbVerifyFullTcamLayout found lost memory in row [%d]\n", row);
            }
            if (stopAfterFirstError != GT_FALSE)
            {
                cpssOsFree(columnBmpArr);
                prvCpssDxChVirtualTcamDbVTcamDumpFullTcamLayoutByPtr(&vTcamMngPtr->tcamTtiSegCfg);
                return totalRc;
            }
        }
    }

    /* cleanup */
    cpssOsFree(columnBmpArr);
    if ((printErrorMessage != GT_FALSE) && (errorCounter != 0))
    {
        cpssOsPrintf(
            "VirtualTcamDbVerifyFullTcamLayout vTcamMngId [%d] total errors [%d] \n",
            vTcamMngId , errorCounter);
    }
    if (totalRc != GT_OK)
    {
        prvCpssDxChVirtualTcamDbVTcamDumpFullTcamLayoutByPtr(&vTcamMngPtr->tcamSegCfg);
        prvCpssDxChVirtualTcamDbVTcamDumpFullTcamLayoutByPtr(&vTcamMngPtr->tcamTtiSegCfg);
    }
    return totalRc;
}

/**
* @internal prvCpssDxChVirtualTcamDbVerifyAllTrees function
* @endinternal
*
* @brief   Verify trees in DB - matching data to compaire functions.
*         Should detect corruption caused by data updated without removing and inserting
*         related tree items.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
* @param[in] stopAfterFirstError      - GT_TRUE  - return after first error
*                                      - GT_FALSE - continue checking
* @param[in] printErrorMessage        - GT_TRUE  - print error messages
*                                      GT_FALSE - count errors only
*
* @retval GT_OK                    - no errors found,
* @retval other                    - the code of the last error.
*/
GT_STATUS prvCpssDxChVirtualTcamDbVerifyAllTrees
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_BOOL                                     stopAfterFirstError,
    IN  GT_BOOL                                     printErrorMessage
)
{
    GT_BOOL                                       rc;           /* return code               */
    GT_STATUS                                     totalRc;      /* last not OK returned code */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC*           vTcamMngPtr;  /* pointer to vTcam Manager  */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC*  vTcamCfgPtr;  /* pointer to vTcam          */
    GT_U32                                        vTcamId;      /* vTcamId                   */
    GT_U32                                        errorCounter; /* error counter             */

    vTcamMngPtr = prvCpssDxChVirtualTcamDbVTcamMngGet(vTcamMngId);
    if (vTcamMngPtr == NULL)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        if (printErrorMessage != GT_FALSE)
        {
            cpssOsPrintf(
                "prvCpssDxChVirtualTcamDbVerifyAllTrees vTcamMngId [%d] BAD PARAM\n",
                vTcamMngId);
        }
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (vTcamMngPtr->vTcamCfgPtrArr == NULL)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        if (printErrorMessage != GT_FALSE)
        {
            cpssOsPrintf(
                "prvCpssDxChVirtualTcamDbVerifyAllTrees vTcamMngId [%d] vTcamCfgPtrArr == NULL\n",
                vTcamMngId);
        }
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    totalRc = GT_OK;
    errorCounter = 0;
    rc = prvCpssAvlTreeIsOrderCorrect(vTcamMngPtr->tcamSegCfg.segFreeLuTree);
    if (rc == GT_FALSE)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        errorCounter ++;
        totalRc = GT_BAD_STATE;
        if (printErrorMessage != GT_FALSE)
        {
            cpssOsPrintf(
                "prvCpssDxChVirtualTcamDbVerifyAllTrees vTcamMngId [%d] segFreeLuTree corrupted\n",
                vTcamMngId);
        }
        if (stopAfterFirstError != GT_FALSE)
        {
            return totalRc;
        }
    }
    rc = prvCpssAvlTreeIsOrderCorrect(vTcamMngPtr->tcamTtiSegCfg.segFreeLuTree);
    if (rc == GT_FALSE)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        errorCounter ++;
        totalRc = GT_BAD_STATE;
        if (printErrorMessage != GT_FALSE)
        {
            cpssOsPrintf(
                "prvCpssDxChVirtualTcamDbVerifyAllTrees vTcamMngId [%d] segFreeLuTree corrupted\n",
                vTcamMngId);
        }
        if (stopAfterFirstError != GT_FALSE)
        {
            return totalRc;
        }
    }

    rc = prvCpssAvlTreeIsOrderCorrect(vTcamMngPtr->tcamSegCfg.segFreePrefTree);
    if (rc == GT_FALSE)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        errorCounter ++;
        totalRc = GT_BAD_STATE;
        if (printErrorMessage != GT_FALSE)
        {
            cpssOsPrintf(
                "prvCpssDxChVirtualTcamDbVerifyAllTrees vTcamMngId [%d] segFreePrefTree corrupted\n",
                vTcamMngId);
        }
        if (stopAfterFirstError != GT_FALSE)
        {
            return totalRc;
        }
    }

    rc = prvCpssAvlTreeIsOrderCorrect(vTcamMngPtr->tcamTtiSegCfg.segFreePrefTree);
    if (rc == GT_FALSE)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        errorCounter ++;
        totalRc = GT_BAD_STATE;
        if (printErrorMessage != GT_FALSE)
        {
            cpssOsPrintf(
                "prvCpssDxChVirtualTcamDbVerifyAllTrees vTcamMngId [%d] segFreePrefTree corrupted\n",
                vTcamMngId);
        }
        if (stopAfterFirstError != GT_FALSE)
        {
            return totalRc;
        }
    }

    for (vTcamId = 0; (vTcamId < vTcamMngPtr->vTcamCfgPtrArrSize); vTcamId++)
    {
        vTcamCfgPtr = vTcamMngPtr->vTcamCfgPtrArr[vTcamId];
        if (vTcamCfgPtr == NULL) continue;

        rc = prvCpssAvlTreeIsOrderCorrect(vTcamCfgPtr->priorityIndexTree);
        if (rc == GT_FALSE)
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            errorCounter ++;
            totalRc = GT_BAD_STATE;
            if (printErrorMessage != GT_FALSE)
            {
                cpssOsPrintf(
                    "VirtualTcamDbVerifyAllTrees vTcamMngId [%d] vTcamId  [%d] priorityIndexTree corrupted\n",
                    vTcamMngId, vTcamId);
            }
            if (stopAfterFirstError != GT_FALSE)
            {
                return totalRc;
            }
        }

        rc = prvCpssAvlTreeIsOrderCorrect(vTcamCfgPtr->priorityPriTree);
        if (rc == GT_FALSE)
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            errorCounter ++;
            totalRc = GT_BAD_STATE;
            if (printErrorMessage != GT_FALSE)
            {
                cpssOsPrintf(
                    "VirtualTcamDbVerifyAllTrees vTcamMngId [%d] vTcamId  [%d] priorityPriTree corrupted\n",
                    vTcamMngId, vTcamId);
            }
            if (stopAfterFirstError != GT_FALSE)
            {
                return totalRc;
            }
        }

        rc = prvCpssAvlTreeIsOrderCorrect(vTcamCfgPtr->ruleIdIdTree);
        if (rc == GT_FALSE)
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            errorCounter ++;
            totalRc = GT_BAD_STATE;
            if (printErrorMessage != GT_FALSE)
            {
                cpssOsPrintf(
                    "VirtualTcamDbVerifyAllTrees vTcamMngId [%d] vTcamId  [%d] ruleIdIdTree corrupted\n",
                    vTcamMngId, vTcamId);
            }
            if (stopAfterFirstError != GT_FALSE)
            {
                return totalRc;
            }
        }

        rc = prvCpssAvlTreeIsOrderCorrect(vTcamCfgPtr->ruleIdIndexTree);
        if (rc == GT_FALSE)
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            errorCounter ++;
            totalRc = GT_BAD_STATE;
            if (printErrorMessage != GT_FALSE)
            {
                cpssOsPrintf(
                    "VirtualTcamDbVerifyAllTrees vTcamMngId [%d] vTcamId ruleIdIndexTree [%d]  corrupted\n",
                    vTcamMngId, vTcamId);
            }
            if (stopAfterFirstError != GT_FALSE)
            {
                return totalRc;
            }
        }

        rc = prvCpssAvlTreeIsOrderCorrect(vTcamCfgPtr->segmentsTree);
        if (rc == GT_FALSE)
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            errorCounter ++;
            totalRc = GT_BAD_STATE;
            if (printErrorMessage != GT_FALSE)
            {
                cpssOsPrintf(
                    "VirtualTcamDbVerifyAllTrees vTcamMngId [%d] vTcamId  [%d] segmentsTree corrupted\n",
                    vTcamMngId, vTcamId);
            }
            if (stopAfterFirstError != GT_FALSE)
            {
                return totalRc;
            }
        }
    }

    if ((printErrorMessage != GT_FALSE) && (errorCounter != 0))
    {
        cpssOsPrintf(
            "VirtualTcamDbVerifyAllTrees vTcamMngId [%d] total errors [%d] \n",
            vTcamMngId , errorCounter);
    }
    return totalRc;
}

/**
* @internal prvCpssDxChVirtualTcamDbManagerDump function
* @endinternal
*
* @brief   Dump tcam info without ‘vtcam info’
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
*                                       None.
*/
GT_VOID prvCpssDxChVirtualTcamDbManagerDump
(
    IN  GT_U32                                      vTcamMngId
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC *vTcamMngDBPtr; /* poinetr to vTcam manager */
    GT_U32 *devsBitmap; /* pointer to the devices bitmap */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr; /* pointer to DB segments table configuration */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_BLOCK_INFO_STC *blockInfArr; /* array of DB segments block state */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_PTR *vTcamCfgPtrArr; /* array of vTCAMs */
    GT_BOOL inRange; /* helper variable for printing range */
    GT_U32 i, beginOfRange, arrSize; /* index, helper variable for printing range, array size */

    cpssOsPrintf("\n"); /* make sure the printings will start in new line */

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        cpssOsPrintf("vTcamMngId out of range\n");
        return;
    }

    vTcamMngDBPtr = prvCpssDxChVirtualTcamDbVTcamMngGet(vTcamMngId);
    if(vTcamMngDBPtr == NULL)
    {
        /* vTcam manager is NULL */
        cpssOsPrintf("vTcam manager is NULL\n");
        return;
    }

    devsBitmap = vTcamMngDBPtr->devsBitmap; /* get's devsBitmap */
    cpssOsPrintf("devsBitmap: ");
    if (devsBitmap == NULL)
    {
        cpssOsPrintf("NULL\n");
    }
    else
    {
        inRange = GT_FALSE;
        beginOfRange = 0;
        arrSize = vTcamMngDBPtr->numOfDevices; /* get's devsBitmap size */
        /* iterating over the bitmap */
        for (i = 0; i < arrSize; i++)
        {
            if (0 == (devsBitmap[i>>5] & (1<<(i & 0x1f))))
            {
                /* end of range */
                if (inRange == GT_TRUE)
                {
                    /* printing the range that just ended*/
                    if ( i != 0 && i-1 != beginOfRange)
                    {
                        cpssOsPrintf("..%d", i - 1);
                    }
                    if (i != arrSize-1)
                    {
                        cpssOsPrintf(", ");
                    }
                    inRange = GT_FALSE;
                }
            }
            /* devsBitmap[i] is on */
            else
            {
                /* beginning of range */
                if (inRange == GT_FALSE)
                {
                    cpssOsPrintf("%d",i);
                    inRange = GT_TRUE;
                    beginOfRange = i;
                }
            }
        }
        /* daling with the last range */
        if (inRange == GT_TRUE)
        {
            /* printing the range that just ended*/
            if (i != 0 && i-1 != beginOfRange)
            {
                cpssOsPrintf("..%d", i - 1);
            }
        }
    }
    cpssOsPrintf("\n");

    arrSize = vTcamMngDBPtr->numOfDevices;
    cpssOsPrintf("numOfDevices = %d\n", arrSize); /* printing num of devices*/

    tcamSegCfgPtr = &vTcamMngDBPtr->tcamSegCfg;
    cpssOsPrintf("tcamSegCfg{\n");/* printing tcamSegCfg structure*/
    cpssOsPrintf("\t");
    cpssOsPrintf("tcamColumnsAmount = %d\n", tcamSegCfgPtr->tcamColumnsAmount);
    cpssOsPrintf("\t");
    cpssOsPrintf("tcamRowsAmount = %d\n", tcamSegCfgPtr->tcamRowsAmount);
    cpssOsPrintf("\t");
    cpssOsPrintf("lookupColumnAlignment = %d\n", tcamSegCfgPtr->lookupColumnAlignment);
    cpssOsPrintf("\t");
    cpssOsPrintf("lookupRowAlignment = %d\n", tcamSegCfgPtr->lookupRowAlignment);
    cpssOsPrintf("\t");
    cpssOsPrintf("maxSegmentsInDb = %d\n", tcamSegCfgPtr->maxSegmentsInDb);

    arrSize = tcamSegCfgPtr->blocksAmount;
    cpssOsPrintf("blocksAmount = %d\n", arrSize); /* printing blocks amount*/

    blockInfArr = tcamSegCfgPtr->blockInfArr;
    if (blockInfArr == NULL)
    {
        cpssOsPrintf("blockInfArr is NULL\n");
    }
    else
    {
        cpssOsPrintf("blockInfArr table:\n");
        cpssOsPrintf("index\t");
        cpssOsPrintf("columnsBase\t");
        cpssOsPrintf("rowsBase\t");
        cpssOsPrintf("columnsAmount\t");
        cpssOsPrintf("rowsAmount\t");
        cpssOsPrintf("lookupId\n");
        /* iterating over blockInfArr elements */
        for (i = 0; i < arrSize; i++)
        {
            /* printing blockInfArr element*/
            cpssOsPrintf("%d\t    ",i);
            cpssOsPrintf("%d\t\t    ", blockInfArr[i].columnsBase);
            cpssOsPrintf("%d\t\t    ", blockInfArr[i].rowsBase);
            cpssOsPrintf("%d\t\t    ", blockInfArr[i].columnsAmount);
            cpssOsPrintf("%d\t\t    ", blockInfArr[i].rowsAmount);
            cpssOsPrintf("%d\n",       blockInfArr[i].lookupId);
        }
    }
    cpssOsPrintf("}\n");

    tcamSegCfgPtr = &vTcamMngDBPtr->tcamTtiSegCfg;
    cpssOsPrintf("tcamTtiSegCfg{\n");/* printing tcamTtiSegCfg structure*/
    cpssOsPrintf("\t");
    cpssOsPrintf("tcamColumnsAmount = %d\n", tcamSegCfgPtr->tcamColumnsAmount);
    cpssOsPrintf("\t");
    cpssOsPrintf("tcamRowsAmount = %d\n", tcamSegCfgPtr->tcamRowsAmount);
    cpssOsPrintf("\t");
    cpssOsPrintf("lookupColumnAlignment = %d\n", tcamSegCfgPtr->lookupColumnAlignment);
    cpssOsPrintf("\t");
    cpssOsPrintf("lookupRowAlignment = %d\n", tcamSegCfgPtr->lookupRowAlignment);
    cpssOsPrintf("\t");
    cpssOsPrintf("maxSegmentsInDb = %d\n", tcamSegCfgPtr->maxSegmentsInDb);

    arrSize = tcamSegCfgPtr->blocksAmount;
    cpssOsPrintf("blocksAmount = %d\n", arrSize); /* printing blocks amount*/

    blockInfArr = tcamSegCfgPtr->blockInfArr;
    if (blockInfArr == NULL)
    {
        cpssOsPrintf("blockInfArr is NULL\n");
    }
    else
    {
        cpssOsPrintf("blockInfArr table:\n");
        cpssOsPrintf("index\t");
        cpssOsPrintf("columnsBase\t");
        cpssOsPrintf("rowsBase\t");
        cpssOsPrintf("columnsAmount\t");
        cpssOsPrintf("rowsAmount\t");
        cpssOsPrintf("lookupId\n");
        /* iterating over blockInfArr elements */
        for (i = 0; i < arrSize; i++)
        {
            /* printing blockInfArr element*/
            cpssOsPrintf("%d\t    ",i);
            cpssOsPrintf("%d\t\t    ", blockInfArr[i].columnsBase);
            cpssOsPrintf("%d\t\t    ", blockInfArr[i].rowsBase);
            cpssOsPrintf("%d\t\t    ", blockInfArr[i].columnsAmount);
            cpssOsPrintf("%d\t\t    ", blockInfArr[i].rowsAmount);
            cpssOsPrintf("%d\n",       blockInfArr[i].lookupId);
        }
    }
    cpssOsPrintf("}\n");

    arrSize = vTcamMngDBPtr->vTcamCfgPtrArrSize;
    vTcamCfgPtrArr = vTcamMngDBPtr->vTcamCfgPtrArr;
    if (vTcamCfgPtrArr == NULL)
    {
        cpssOsPrintf("vTcamCfgPtrArr is NULL\n");
    }
    else
    {
        cpssOsPrintf("vTcamCfgPtrArr: ");
        beginOfRange = GT_FALSE;
        /* iterating over vTcamCfgPtrArr elements */
        for (i = 0; i < arrSize; i++)
        {
            /* printing all indexes of elements which are not NULL */
            if (vTcamCfgPtrArr[i] != NULL)
            {
                if (beginOfRange == GT_FALSE)
                {
                    cpssOsPrintf("%d",i);
                    beginOfRange = GT_TRUE;
                }
                else
                {
                    cpssOsPrintf(", %d",i);
                }
            }
        }
        cpssOsPrintf("\n");
    }
}

/**
* @internal printTcamInfo function
* @endinternal
*
* @brief   print tcam info about a vTcam in vTcam manager.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamInfo                 - structure of tcamInfo
*                                       None
*/
static GT_VOID printTcamInfo
(
    IN CPSS_DXCH_VIRTUAL_TCAM_INFO_STC tcamInfo
)
{
    /* the vTcam Information */
    cpssOsPrintf("tcamInfo{\n");
    cpssOsPrintf("\t");
    cpssOsPrintf("clientGroup = %d\n", tcamInfo.clientGroup);
    cpssOsPrintf("\t");
    cpssOsPrintf("hitNumber = %d\n", tcamInfo.hitNumber);
    cpssOsPrintf("\t");
    cpssOsPrintf("ruleSize = %d\n", tcamInfo.ruleSize);
    cpssOsPrintf("\t");
    cpssOsPrintf("autoResize = %d\n", tcamInfo.autoResize);
    cpssOsPrintf("\t");
    cpssOsPrintf("guaranteedNumOfRules = %d\n", tcamInfo.guaranteedNumOfRules);
    cpssOsPrintf("\t");
    cpssOsPrintf("ruleAdditionMethod = %d\n", tcamInfo.ruleAdditionMethod);
    cpssOsPrintf("}\n");
}


/**
* @internal printUsedRulesBitmapArr function
* @endinternal
*
* @brief   print a used rules bitmap array of a vTcam in vTcam manager.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] usedRulesBitmapArr       - bitmap of used rules
* @param[in] rulesAmount              - amount of rules
* @param[in] expected_usedRulesAmount - expected amount of used rules
*                                       None
*/
static GT_VOID printUsedRulesBitmapArr
(
    IN GT_U32   *usedRulesBitmapArr,
    IN GT_U32   rulesAmount,
    IN GT_U32   expected_usedRulesAmount
)
{
    if (usedRulesBitmapArr == NULL)
    {
        cpssOsPrintf("usedRulesBitmapArr:not-exist \n");
        return;
    }
    cpssOsPrintf("usedRulesBitmapArr: ");
    printBitmapArr("usedRulesBitmapArr",
                    usedRulesBitmapArr,
                    rulesAmount,
                    expected_usedRulesAmount);
    cpssOsPrintf("\n");
}

/**
* @internal print_segmentColumnsMap function
* @endinternal
*
* @brief   print a used rules bitmap array of a vTcam in vTcam manager.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
*                                       None
*/
static GT_VOID print_segmentColumnsMap
(
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP segmentColumnsMap
)
{
    GT_U32 i;
    char* name;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT segmentColumns;

    for (i = 0; (i < 8); i++)
    {
        segmentColumns = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
            segmentColumnsMap, i);
        if (segmentColumns ==
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E) continue;
        name =
            (segmentColumns < PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LAST_E)
                ? prvCpssDxChVirtualTcamDbSegmentColumnsEnumNames[segmentColumns]
                : "WRONG";
        cpssOsPrintf("%d/%s ", i, name);
    }
    cpssOsPrintf("\t");
}

/**
* @internal printRulePhysicalIndexArr function
* @endinternal
*
* @brief   print a rule physical index array of a vTcam in vTcam manager.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] rulesAmount              - amount of rules
* @param[in] rulePhysicalIndexArr     - array of physical indexes
*                                       None
*/
static GT_VOID printRulePhysicalIndexArr
(
    IN GT_U16   *rulePhysicalIndexArr,
    IN GT_U32   rulesAmount
)
{
    GT_U32 i,j,rowNum; /* indexes */

    if (rulePhysicalIndexArr == NULL)
    {
        cpssOsPrintf("rulePhysicalIndexArr is NULL\n");
    }
    else
    {
        cpssOsPrintf("Logical index to HW index mapping:\n\n");
        cpssOsPrintf("Row/column\t");
        for (i = 0; i < 10 ; i++)
        {
            cpssOsPrintf("%d\t", i);
        }
        cpssOsPrintf("\n");
        rowNum = 0;
        /* iterating over rulePhysicalIndexArr elements and printing*/
        for (i = 0; i < rulesAmount; i++)
        {
            cpssOsPrintf("%d\t\t",rowNum);
            /* printing a row */
            for (j = 0; j < 10 && i < rulesAmount; j++, i++ )
            {
                cpssOsPrintf("%d\t",rulePhysicalIndexArr[i]);
            }
            cpssOsPrintf("\n");
            rowNum++;
            i--;
        }
        cpssOsPrintf("\n");
    }
}

/**
* @internal printSegmentsTree function
* @endinternal
*
* @brief   print the avl segments tree of a vTcam in vTcam manager.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] treeIdPtr                - a pointer to the id of the segments tree
*                                       None
*/
static GT_VOID printSegmentsTree
(
    IN PRV_CPSS_AVL_TREE_ID treeIdPtr
)
{
    GT_U32 i; /* index */
    PRV_CPSS_AVL_TREE_ITERATOR_ID iterPtr; /* avl tree iterator */
    GT_VOID *dataPtr; /* pointer to avl node data */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC  *segmentsNodePtr; /* pointer to segmentsTree node data */

    /* checks if the tree itslef is not NULL */
    if (treeIdPtr == NULL)
    {
        cpssOsPrintf("segmentsTree is NULL\n");
    }
    else
    {
        i = 0;
        /* get's the first avl tree node */
        iterPtr = NULL;
        dataPtr = prvCpssAvlGetNext(treeIdPtr, &iterPtr);
        if (dataPtr == NULL)
        {
            cpssOsPrintf("segmentsTree is empty\n");
        }
        else
        {
            cpssOsPrintf("segmentsTree table:\n\n");
            cpssOsPrintf("index\t");
            cpssOsPrintf("rowsBase    ");
            cpssOsPrintf("rowsAmount\t");
            cpssOsPrintf("segmentColumnsMap\t");
            cpssOsPrintf("vTcamId\t   ");
            cpssOsPrintf("lookupId\t");
            cpssOsPrintf("rulesAmount\t");
            cpssOsPrintf("ruleSize    ");
            cpssOsPrintf("baseLogicalIndex\n");
            /* iterating over all tree elements */
            while (dataPtr != NULL)
            {
                segmentsNodePtr = (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *)dataPtr;
                cpssOsPrintf("%d\t ",i);
                cpssOsPrintf("%3.3d\t      ", segmentsNodePtr->rowsBase);
                cpssOsPrintf("%3.3d\t    ", segmentsNodePtr->rowsAmount);
                print_segmentColumnsMap(segmentsNodePtr->segmentColumnsMap);
                cpssOsPrintf("%4.4d\t      ", segmentsNodePtr->vTcamId);
                cpssOsPrintf("%3.3d\t   ", segmentsNodePtr->lookupId);
                cpssOsPrintf("%4.4d\t\t    ", segmentsNodePtr->rulesAmount);
                cpssOsPrintf("%d\t\t  ", segmentsNodePtr->ruleSize);
                cpssOsPrintf("%d\n", segmentsNodePtr->baseLogicalIndex);
                dataPtr = prvCpssAvlGetNext(treeIdPtr, &iterPtr);
                i++;
            }
            cpssOsPrintf("\n");
        }
    }
}

/**
* @internal printRuleIdTree function
* @endinternal
*
* @brief   print the avl rule ID type tree of a vTcam in vTcam manager.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] treeIdPtr                - a pointer to the id of the rule ID type tree
* @param[in] namePtr                  - the name of the tree
*                                      can be "ruleIdIdTree" or "ruleIdIndexTree"
*                                       None
*/
static GT_VOID printRuleIdTree
(
    IN PRV_CPSS_AVL_TREE_ID        treeIdPtr,
    IN GT_CHAR_PTR                 namePtr
)
{
    GT_U32 i; /* index */
    PRV_CPSS_AVL_TREE_ITERATOR_ID iterPtr; /* avl tree iterator */
    GT_VOID *dataPtr; /* pointer to avl node data */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC  *ruleIdIdNodePtr; /* pointer to ruleIdTree node data */

    /* checks if the tree itslef is not NULL */
    if (treeIdPtr == NULL)
    {
        cpssOsPrintf("%s is NULL\n",namePtr);
    }
    else
    {
        i = 0;
        /* get's the first avl tree node */
        iterPtr = NULL;
        dataPtr = prvCpssAvlGetNext(treeIdPtr, &iterPtr);
        if (dataPtr == NULL)
        {
            cpssOsPrintf("%s is empty\n",namePtr);
        }
        else
        {
            cpssOsPrintf("%s table:\n\n",namePtr);
            cpssOsPrintf("index\t");
            cpssOsPrintf("ruleId\t");
            cpssOsPrintf("logicalIndex\n");
            /* iterating over all tree elements */
            while (dataPtr != NULL)
            {
                ruleIdIdNodePtr = (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC *)dataPtr;
                cpssOsPrintf("%d\t ",i);
                cpssOsPrintf("%d\t", ruleIdIdNodePtr->ruleId);
                cpssOsPrintf("%d\n", ruleIdIdNodePtr->logicalIndex);
                dataPtr = prvCpssAvlGetNext(treeIdPtr, &iterPtr);
                i++;
            }
            cpssOsPrintf("\n");
        }
    }
}

/**
* @internal printPriorityTree function
* @endinternal
*
* @brief   print the avl priority type tree of a vTcam in vTcam manager.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] treeIdPtr                - a pointer to the id of the priority type tree
* @param[in] namePtr                  - the name of the tree
*                                      can be "priorityPriTree" or "priorityIndexTree"
*                                       None
*/
static GT_VOID printPriorityTree
(
    IN PRV_CPSS_AVL_TREE_ID        treeIdPtr,
    IN GT_CHAR_PTR                 namePtr
)
{
    GT_U32 i; /* index */
    PRV_CPSS_AVL_TREE_ITERATOR_ID iterPtr; /* avl tree iterator */
    GT_VOID *dataPtr; /* pointer to avl node data */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC *priorityPriNodePtr; /* pointer to priorityTree node data */

    if (treeIdPtr == NULL)
    {
        cpssOsPrintf("%s is NULL\n", namePtr);
    }
    else
    {
        i = 0;
        /* get's the first avl tree node */
        iterPtr = NULL;
        dataPtr = prvCpssAvlGetNext(treeIdPtr, &iterPtr);
        if (dataPtr == NULL)
        {
            cpssOsPrintf("%s is empty\n", namePtr);
        }
        else
        {
            cpssOsPrintf("%s table:\n\n",namePtr);
            cpssOsPrintf("index\t");
            cpssOsPrintf("priority\t");
            cpssOsPrintf("baseLogIndex\t");
            cpssOsPrintf("rangeSize\n");
            /* iterating over all tree elements */
            while (dataPtr != NULL)
            {
                priorityPriNodePtr = (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC *)dataPtr;
                cpssOsPrintf("%d\t ",i);
                cpssOsPrintf("%d\t", priorityPriNodePtr->priority);
                cpssOsPrintf("%d\t", priorityPriNodePtr->baseLogIndex);
                cpssOsPrintf("%d\n", priorityPriNodePtr->rangeSize);
                dataPtr = prvCpssAvlGetNext(treeIdPtr, &iterPtr);
                i++;
            }
            cpssOsPrintf("\n");
        }
    }
}

/**
* @enum HW_INDEX_SEGMENT_TYPE_ENT
 *
 * @brief enumeration for typed of segments for HW indexes.
*/
typedef enum{

    /** @brief indication that the current HW index is in 'existing' segment
     *  'old' = valid index , old = valid index
     */
    HW_INDEX_SEGMENT_TYPE_EXISTING_E,

    /** @brief indication that the current HW index is in 'added' segment
     *  'old' = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_HW_INDEX_NOT_USED_CNS , new = valid index
     */
    HW_INDEX_SEGMENT_TYPE_ADDED_E,

    /** @brief indication that the current HW index is in 'removed' segment
     *  'new' = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_HW_INDEX_NOT_USED_CNS , old = valid index
     */
    HW_INDEX_SEGMENT_TYPE_REMOVED_E

} HW_INDEX_SEGMENT_TYPE_ENT;


/* indication to not valid diff of GT_32 variable */
#define NOT_VALID_DIFF_CNS  ((GT_32)(BIT_30))


/**
* @internal pringSegmentTypeRangeInfo function
* @endinternal
*
* @brief   helper function to print range in prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[]
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] segmentType              - the type of segment
* @param[in] startRangeLogicalIndex   - the logical index that starts the range
* @param[in] lastInRangeLogicalIndex  - the logical index that is last in the range (inclusive)
* @param[in] diffToOldLogicalIndex    - the diff between new logical indexes to old logical indexes
*                                      (zero or negative or positive)
*                                       None
*/
static void pringSegmentTypeRangeInfo(
    IN HW_INDEX_SEGMENT_TYPE_ENT segmentType,
    IN GT_U32                    startRangeLogicalIndex,
    IN GT_U32                    lastInRangeLogicalIndex,
    IN GT_32                     diffToOldLogicalIndex
)
{
    GT_U32  rangeSize = (lastInRangeLogicalIndex + 1) - startRangeLogicalIndex;
    if(segmentType == HW_INDEX_SEGMENT_TYPE_EXISTING_E)
    {
        /* the segment hold different values for old and new with specific offset between them */
        cpssOsPrintf("existing segment: new logical index[%4.4d..%4.4d] with diff[%d] (to old logical index) ([%d] indexes)\n",
            startRangeLogicalIndex,
            lastInRangeLogicalIndex,
            diffToOldLogicalIndex,
            rangeSize);
    }
    else
    if(segmentType == HW_INDEX_SEGMENT_TYPE_ADDED_E)
    {
        /* the segment was 'added' (no 'old') */
        cpssOsPrintf("added segment:    new logical index[%4.4d..%4.4d] (no old logical index)([%d] indexes)\n",
            startRangeLogicalIndex,
            lastInRangeLogicalIndex,
            rangeSize);
    }
    else  /*HW_INDEX_SEGMENT_TYPE_REMOVED_E*/
    {
        /* the segment was 'removed' (no 'new') */
        cpssOsPrintf("removed segment:  old logical index[%4.4d..%4.4d] (no new logical index)([%d] indexes)\n",
            startRangeLogicalIndex,
            lastInRangeLogicalIndex,
            rangeSize);
    }
}

/**
* @internal print_resizeHwIndexDbArr function
* @endinternal
*
* @brief   print the prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[] for specific vtcam
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] ruleSize                 - the rule size of the entries in the vtcam.
*                                       None
*/
void print_resizeHwIndexDbArr(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC* tcamSegCfgPtr,
    IN CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT         ruleSize
)
{
    GT_U32  ii;
    GT_32 currDiffOldAndNew = NOT_VALID_DIFF_CNS;/* current diff between old and new */
    GT_32 prevDiffOldAndNew = NOT_VALID_DIFF_CNS;/* previous diff between old and new */
    HW_INDEX_SEGMENT_TYPE_ENT currSegmentType;
    HW_INDEX_SEGMENT_TYPE_ENT prevSegmentType = HW_INDEX_SEGMENT_TYPE_EXISTING_E;
    GT_U32  stepSize;
    GT_U32  startRangeLogicalIndex=0;
    GT_U32  inRangeLogicalIndex=0;
    GT_BOOL startRange = GT_TRUE;
    GT_U32  indexInRange;
    GT_BOOL currentInfoPrinted = GT_TRUE;

    stepSize = prvCpssDxChVirtualTcamSegmentTableRuleSizeToAlignment(
        tcamSegCfgPtr, ruleSize);
    if (stepSize == 0)
    {
        cpssOsPrintf("prvCpssDxChVirtualTcamDbResizeHwIndexDbArr wrong rule size \n");
        return;
    }

    if(prvCpssDxChVirtualTcamDbResizeHwIndexDbArr == NULL)
    {
        cpssOsPrintf("prvCpssDxChVirtualTcamDbResizeHwIndexDbArr not exists \n");
        return;
    }

    cpssOsPrintf("prvCpssDxChVirtualTcamDbResizeHwIndexDbArr: \n");
    for(ii = 0 ; ii < prvCpssDxChVirtualTcamDbNumEntries_resizeHwIndexDbArr ; ii += stepSize)
    {
        if((prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[ii].oldLogicalIndex == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_HW_INDEX_NOT_USED_CNS) &&
           (prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[ii].newLogicalIndex == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_HW_INDEX_NOT_USED_CNS))
        {
            /* this is not indication that the range (of logical indexes) ended !
              (because we probably got HW indexes in non full floor  */
            continue;
        }

        if((prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[ii].oldLogicalIndex != PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_HW_INDEX_NOT_USED_CNS) &&
           (prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[ii].newLogicalIndex != PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_HW_INDEX_NOT_USED_CNS))
        {
            currSegmentType = HW_INDEX_SEGMENT_TYPE_EXISTING_E;

            currDiffOldAndNew = prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[ii].newLogicalIndex -
                                prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[ii].oldLogicalIndex;

            indexInRange = prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[ii].newLogicalIndex;
        }
        else
        if(prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[ii].oldLogicalIndex != PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_HW_INDEX_NOT_USED_CNS)
        {
            currSegmentType = HW_INDEX_SEGMENT_TYPE_REMOVED_E;
            currDiffOldAndNew = 0;
            indexInRange = prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[ii].oldLogicalIndex;
        }
        else  /* newLogicalIndex != PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_HW_INDEX_NOT_USED_CNS */
        {
            currSegmentType = HW_INDEX_SEGMENT_TYPE_ADDED_E;
            currDiffOldAndNew = 0;
            indexInRange = prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[ii].newLogicalIndex;
        }

        if(startRange == GT_TRUE)
        {
            /* save the start of range info */
            startRange = GT_FALSE;

            if(currentInfoPrinted == GT_FALSE)
            {
                /* save the start of range info */
                startRangeLogicalIndex = indexInRange;
            }
            else
            {
                currentInfoPrinted = GT_FALSE;
            }
        }
        else
        if(prevSegmentType   != currSegmentType ||
           prevDiffOldAndNew != currDiffOldAndNew )
        {
            /* end of range */
            /* print the range info */
            pringSegmentTypeRangeInfo(prevSegmentType,
                startRangeLogicalIndex,
                inRangeLogicalIndex,
                prevDiffOldAndNew);
            /* indicate that the range ended */
            startRange = GT_TRUE;
            currentInfoPrinted = GT_TRUE;
            /* save the start of range info */
            startRangeLogicalIndex = indexInRange;
        }

        /* update 'last' index in the range */
        inRangeLogicalIndex = indexInRange;

        prevDiffOldAndNew = currDiffOldAndNew;
        prevSegmentType = currSegmentType;
    }

    if(currentInfoPrinted == GT_FALSE)
    {
        /* print the last range info */
        pringSegmentTypeRangeInfo(prevSegmentType,
            startRangeLogicalIndex,
            inRangeLogicalIndex,
            prevDiffOldAndNew);
    }

    cpssOsPrintf("prvCpssDxChVirtualTcamDbResizeHwIndexDbArr: ended \n");
}

/**
* @internal prvCpssDxChVirtualTcamDbVTcamDump function
* @endinternal
*
* @brief   Dump DB info about a vTcam in vTcam manager.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
*                                       None.
*/
GT_VOID prvCpssDxChVirtualTcamDbVTcamDump
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_U32                                      vTcamId
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC*  vTcamCfgPtr; /* pointer to vTcam */

    cpssOsPrintf("\n"); /* make sure the printings will start in new line */

    /* get's a pointer to the DB of the vTcam */
    vTcamCfgPtr = prvCpssDxChVirtualTcamDbVTcamGet(vTcamMngId,vTcamId);
    if (vTcamCfgPtr == NULL)
    {
        cpssOsPrintf("vTcamCfgPtr is NULL");
        return;
    }
    printTcamInfo(vTcamCfgPtr->tcamInfo); /* printing tcamInfo structure */

    cpssOsPrintf("rulesAmount = %d\n", vTcamCfgPtr->rulesAmount); /* printing rules amount */

    cpssOsPrintf("usedRulesAmount = %d\n", vTcamCfgPtr->usedRulesAmount); /* printing used rules amount */

    printUsedRulesBitmapArr(vTcamCfgPtr->usedRulesBitmapArr,vTcamCfgPtr->rulesAmount,vTcamCfgPtr->usedRulesAmount); /* printing used usedRulesBitmapArr */

    printRulePhysicalIndexArr(vTcamCfgPtr->rulePhysicalIndexArr,vTcamCfgPtr->rulesAmount); /* printing rulePhysicalIndexArr */

    printSegmentsTree(vTcamCfgPtr->segmentsTree); /* printing segmentsTree */

    printRuleIdTree(vTcamCfgPtr->ruleIdIdTree,"ruleIdIdTree"); /* printing ruleIdIdTree */

    printRuleIdTree(vTcamCfgPtr->ruleIdIndexTree,"ruleIdIndexTree"); /* printing ruleIdIndexTree */

    printPriorityTree(vTcamCfgPtr->priorityPriTree,"priorityPriTree"); /* printing priorityPriTree */

    printPriorityTree(vTcamCfgPtr->priorityIndexTree,"priorityIndexTree"); /* printing priorityIndexTree */
}

/**
* @internal prvCpssDxChVirtualTcamDbVTcamDumpBlockFreeSpaceByPtr function
* @endinternal
*
* @brief   Dump block free space.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
*                                      (usefull for dummy vTCAM Manager that has no Id)
* @param[in] perLookupOnly            - GT_TRUE  - amounts per lookup only
*                                      - GT_FALSE - amounts per block and per lookup
*                                       None.
*/
GT_VOID prvCpssDxChVirtualTcamDbVTcamDumpBlockFreeSpaceByPtr
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC* tcamSegCfgPtr,
    IN  GT_BOOL                                 perLookupOnly
)
{
    GT_U32                                        blockIdx;
    GT_U32                                        *blockFreeSpaceArrPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT columnsType;
    GT_U32                                        sumArr[
        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LAST_E];
    GT_U32                                        startBlockIdx;
    GT_U32                                        lookupId;

    cpssOsPrintf("\n"); /* make sure the printings will start in new line */

    if (tcamSegCfgPtr == NULL)
    {
        cpssOsPrintf("vTcamMngPtr == NULL\n");
        return;
    }

    if (perLookupOnly == GT_FALSE)
    {
        cpssOsPrintf("block ");
        for (columnsType = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E;
              (columnsType < PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LAST_E);
              columnsType++)
        {
            cpssOsPrintf("\t %s", prvCpssDxChVirtualTcamDbSegmentColumnsEnumNames[columnsType]);
        }

        for (blockIdx = 0; (blockIdx < tcamSegCfgPtr->blocksAmount); blockIdx++)
        {
            blockFreeSpaceArrPtr = &(tcamSegCfgPtr->blockInfArr[blockIdx].freeSpace[0]);
            cpssOsPrintf("\n");
            cpssOsPrintf("%d ", blockIdx);
            for (columnsType = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E;
                  (columnsType < PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LAST_E);
                  columnsType++)
            {
                cpssOsPrintf("\t\t %d", blockFreeSpaceArrPtr[columnsType]);
            }
        }
        cpssOsPrintf("\n");
    }

    cpssOsPrintf("sums per lookup \n");
    cpssOsPrintf("lookupId ");
    for (columnsType = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E;
          (columnsType < PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LAST_E);
          columnsType++)
    {
        cpssOsPrintf("\t %s", prvCpssDxChVirtualTcamDbSegmentColumnsEnumNames[columnsType]);
    }

    for (startBlockIdx = 0; (startBlockIdx < tcamSegCfgPtr->blocksAmount); startBlockIdx++)
    {
        lookupId = tcamSegCfgPtr->blockInfArr[startBlockIdx].lookupId;

        /* bypass all not first from the same lookupId value owners */
        for (blockIdx = 0; (blockIdx < startBlockIdx); blockIdx++)
        {
            if (tcamSegCfgPtr->blockInfArr[blockIdx].lookupId == lookupId) break;
        }
        /* found block with the less index and the same lookup iD */
        if (blockIdx < startBlockIdx) continue;

        /* clear sums array */
        cpssOsMemSet(sumArr, 0, sizeof(sumArr));
        /* summing over all blocks with the same lookup */
        for (blockIdx = startBlockIdx; (blockIdx < tcamSegCfgPtr->blocksAmount); blockIdx++)
        {
            if (tcamSegCfgPtr->blockInfArr[blockIdx].lookupId != lookupId) continue;

            blockFreeSpaceArrPtr = &(tcamSegCfgPtr->blockInfArr[blockIdx].freeSpace[0]);

            for (columnsType = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E;
                  (columnsType < PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LAST_E);
                  columnsType++)
            {
                sumArr[columnsType] += blockFreeSpaceArrPtr[columnsType];
            }
        }
        cpssOsPrintf("\n");
        cpssOsPrintf("%d ", lookupId);
        for (columnsType = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E;
              (columnsType < PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LAST_E);
              columnsType++)
        {
            cpssOsPrintf("\t\t %d", sumArr[columnsType]);
        }
    }
    cpssOsPrintf("\n");
}

/**
* @internal prvCpssDxChVirtualTcamDbVTcamDumpBlockFreeSpace function
* @endinternal
*
* @brief   Dump block free space.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] perLookupOnly            - GT_TRUE  - amounts per lookup only
*                                      - GT_FALSE - amounts per block and per lookup
*                                       None.
*/
GT_VOID prvCpssDxChVirtualTcamDbVTcamDumpBlockFreeSpace
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_BOOL                                     perLookupOnly
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC*     vTcamMngPtr;

    vTcamMngPtr = prvCpssDxChVirtualTcamDbVTcamMngGet(vTcamMngId);
    if (vTcamMngPtr == NULL)
    {
        cpssOsPrintf("vTcamMngPtr is NULL, vTcamMngId is %d\n", vTcamMngId);
        return;
    }

    prvCpssDxChVirtualTcamDbVTcamDumpBlockFreeSpaceByPtr(
        &vTcamMngPtr->tcamSegCfg, perLookupOnly);

    prvCpssDxChVirtualTcamDbVTcamDumpBlockFreeSpaceByPtr(
        &vTcamMngPtr->tcamTtiSegCfg, perLookupOnly);
}

/**
* @internal prvCpssDxChVirtualTcamDbVTcamDumpByFieldByPtr function
* @endinternal
*
* @brief   Dump DB info about a vTcam in vTcam manager of specific field.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] vTcamMngPtr              - (pointer to)Virtual TCAM Manager structure
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] fieldNamePtr             - name of the required field
*                                       None.
*/
GT_VOID prvCpssDxChVirtualTcamDbVTcamDumpByFieldByPtr
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC*         vTcamMngPtr,
    IN  GT_U32                                      vTcamId,
    IN  GT_CHAR_PTR                                 fieldNamePtr
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC*  vTcamCfgPtr; /* pointer to vTcam         */

    cpssOsPrintf("\n"); /* make sure the printings will start in new line */

    /* dump of data per vTCAM Manager */

    if (cpssOsStrCmp(fieldNamePtr,"segFreePrefTree") == 0)
    {
        printSegmentsTree(vTcamMngPtr->tcamSegCfg.segFreePrefTree); /* printing segFreePrefTree */
        printSegmentsTree(vTcamMngPtr->tcamTtiSegCfg.segFreePrefTree); /* printing segFreePrefTree */
        return;
    }
    if (cpssOsStrCmp(fieldNamePtr,"segFreeLuTree") == 0)
    {
        printSegmentsTree(vTcamMngPtr->tcamSegCfg.segFreeLuTree); /* printing segFreeLuTree */
        printSegmentsTree(vTcamMngPtr->tcamTtiSegCfg.segFreeLuTree); /* printing segFreeLuTree */
        return;
    }
    if (cpssOsStrCmp(fieldNamePtr,"segmentsPoolUsage") == 0)
    {
        cpssBmPoolStatsPrint(vTcamMngPtr->segmentsPool);
        return;
    }

    /* dump of data per vTCAM */

    /* get's a pointer to the DB of the vTcam */
    if (vTcamMngPtr->vTcamCfgPtrArr == NULL)
    {
        cpssOsPrintf("vTcamCfgPtrArr is NULL (all array not allocated)\n");
        return;
    }
    if(vTcamId >= vTcamMngPtr->vTcamCfgPtrArrSize)
    {
        cpssOsPrintf("vTcamId out of range\n");
        return;
    }
    vTcamCfgPtr = vTcamMngPtr->vTcamCfgPtrArr[vTcamId];
    if (vTcamCfgPtr == NULL)
    {
        cpssOsPrintf("vTcamCfgPtr is NULL\n");
        return;
    }

    if (cpssOsStrCmp(fieldNamePtr,"tcamInfo") == 0)
    {
        printTcamInfo(vTcamCfgPtr->tcamInfo); /* printing tcamInfo structure */
        return;
    }

    if (cpssOsStrCmp(fieldNamePtr,"rulesAmount") == 0)
    {
        cpssOsPrintf("rulesAmount = %d\n", vTcamCfgPtr->rulesAmount); /* printing rules amount */
        return;
    }

    if (cpssOsStrCmp(fieldNamePtr,"usedRulesAmount") == 0)
    {
        cpssOsPrintf("usedRulesAmount = %d\n", vTcamCfgPtr->usedRulesAmount); /* printing used rules amount */
        return;
    }

    if (cpssOsStrCmp(fieldNamePtr,"usedRulesBitmapArr") == 0) {
        printUsedRulesBitmapArr(vTcamCfgPtr->usedRulesBitmapArr,vTcamCfgPtr->rulesAmount,vTcamCfgPtr->usedRulesAmount); /* printing used usedRulesBitmapArr */
        return;
    }

    if (cpssOsStrCmp(fieldNamePtr,"rulePhysicalIndexArr") == 0) {
        printRulePhysicalIndexArr(vTcamCfgPtr->rulePhysicalIndexArr,vTcamCfgPtr->rulesAmount); /* printing rulePhysicalIndexArr */
        return;
    }

    if (cpssOsStrCmp(fieldNamePtr,"segmentsTree") == 0)
    {
        printSegmentsTree(vTcamCfgPtr->segmentsTree); /* printing segmentsTree */
        return;
    }

    if (cpssOsStrCmp(fieldNamePtr,"ruleIdIdTree") == 0)
    {
        printRuleIdTree(vTcamCfgPtr->ruleIdIdTree, "ruleIdIdTree"); /* printing ruleIdIdTree */
        return;
    }

    if (cpssOsStrCmp(fieldNamePtr,"ruleIdIndexTree") == 0)
    {
        printRuleIdTree(vTcamCfgPtr->ruleIdIndexTree, "ruleIdIndexTree"); /* printing ruleIdIndexTree */
        return;
    }

    if (cpssOsStrCmp(fieldNamePtr,"priorityPriTree") == 0)
    {
        printPriorityTree(vTcamCfgPtr->priorityPriTree, "priorityPriTree"); /* printing priorityPriTree */
        return;
    }

    if (cpssOsStrCmp(fieldNamePtr,"priorityIndexTree") == 0)
    {
        printPriorityTree(vTcamCfgPtr->priorityIndexTree, "priorityIndexTree"); /* printing priorityIndexTree */
        return;
    }

    if(cpssOsStrCmp(fieldNamePtr,"prvCpssDxChVirtualTcamDbResizeHwIndexDbArr") == 0)
    {

        print_resizeHwIndexDbArr(vTcamCfgPtr->tcamSegCfgPtr, vTcamCfgPtr->tcamInfo.ruleSize);
        return;
    }
}

/**
* @internal prvCpssDxChVirtualTcamDbVTcamDumpByField function
* @endinternal
*
* @brief   Dump DB info about a vTcam in vTcam manager of specific field.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] fieldNamePtr             - name of the required field
*                                       None.
*/
GT_VOID prvCpssDxChVirtualTcamDbVTcamDumpByField
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_U32                                      vTcamId,
    IN  GT_CHAR_PTR                                 fieldNamePtr
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC*           vTcamMngPtr; /* pointer to vTcam Manager */

    vTcamMngPtr = prvCpssDxChVirtualTcamDbVTcamMngGet(vTcamMngId);
    if (vTcamMngPtr == NULL)
    {
        cpssOsPrintf("vTcamMngPtr is NULL, vTcamMngId is %d\n", vTcamMngId);
        return;
    }
    prvCpssDxChVirtualTcamDbVTcamDumpByFieldByPtr(vTcamMngPtr, vTcamId, fieldNamePtr);
}

/**
* @internal prvCpssDxChVirtualTcamDbVTcamDumpFullTcamLayoutByPtr function
* @endinternal
*
* @brief   Dump DB info about full TCAM layout.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
*                                       None.
*/
GT_VOID prvCpssDxChVirtualTcamDbVTcamDumpFullTcamLayoutByPtr
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC*   tcamSegCfgPtr
)
{
    GT_STATUS                                     rc;          /* return code              */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC*           vTcamMngPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC*  vTcamCfgPtr; /* pointer to vTcam         */
    GT_U32                                        vTcamId;     /* vTcamId                  */
    CPSS_BM_POOL_ID                               nodesPool;   /* pool for tree nodes      */
    PRV_CPSS_AVL_TREE_ID                          segLuTree;   /* tree of all segments     */
    GT_BOOL                                       seeked;      /* seeked                   */
    GT_VOID                                       *dbEntryPtr; /* DB Entry pointer         */
    PRV_CPSS_AVL_TREE_PATH                        path;        /* tree iterator            */
    GT_U32                                        space;       /* space                    */
    GT_U32                                        totalSpace;  /* total space              */

    cpssOsPrintf("\n"); /* make sure the printings will start in new line */

    if (tcamSegCfgPtr == NULL)
    {
        cpssOsPrintf("tcamSegCfgPtr == NULL\n");
        return;
    }

    /* pool for tree for all busy and free TCAM segments */
    rc = prvCpssAvlMemPoolCreate(
        tcamSegCfgPtr->maxSegmentsInDb,
        &nodesPool);
    if (rc != GT_OK)
    {
        cpssOsPrintf("Temporary tree nodes pool allocation failed\n");
        return;
    }
    rc = prvCpssAvlTreeCreate(
        (GT_INTFUNCPTR)prvCpssDxChVirtualTcamDbSegmentTableLookupOrderCompareFunc,
        nodesPool,&segLuTree);
    if (rc != GT_OK)
    {
        cpssOsPrintf("Temporary tree creation failed\n");
        cpssBmPoolReCreate(nodesPool);
        prvCpssAvlMemPoolDelete(nodesPool);
        return;
    }

    cpssOsPrintf("Id lookup rule_size r_total r_used space\n");

    vTcamMngPtr = tcamSegCfgPtr->pVTcamMngPtr;
    totalSpace = 0;
    for (vTcamId = 0; (vTcamId < vTcamMngPtr->vTcamCfgPtrArrSize); vTcamId++)
    {
        vTcamCfgPtr = vTcamMngPtr->vTcamCfgPtrArr[vTcamId];
        if (vTcamCfgPtr == NULL) continue;
        if (vTcamCfgPtr->tcamSegCfgPtr != tcamSegCfgPtr) continue;
        space = (vTcamCfgPtr->rulesAmount * (vTcamCfgPtr->tcamInfo.ruleSize + 1));
        totalSpace += space;
        cpssOsPrintf(
            "%4.4d  %4.4d  %4.4d  %4.4d  %4.4d  %4.4d\n",
            vTcamId,
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_LOOKUP_ID_MAC(
                vTcamCfgPtr->tcamInfo.clientGroup, vTcamCfgPtr->tcamInfo.hitNumber),
            (vTcamCfgPtr->tcamInfo.ruleSize + 1),
            vTcamCfgPtr->rulesAmount, vTcamCfgPtr->usedRulesAmount,
            space);
        /* add all vTcam used segments to segments tree */
        seeked = prvCpssAvlPathSeek(
                vTcamCfgPtr->segmentsTree, PRV_CPSS_AVL_TREE_SEEK_FIRST_E,
                path, &dbEntryPtr);
        while (seeked != GT_FALSE)
        {
            rc = prvCpssAvlItemInsert(segLuTree, dbEntryPtr);
            if (rc != GT_OK)
            {
                cpssOsPrintf("Segment adding to temporary tree failed - will not be printed\n");
            }
            seeked = prvCpssAvlPathSeek(
                    vTcamCfgPtr->segmentsTree, PRV_CPSS_AVL_TREE_SEEK_NEXT_E,
                    path, &dbEntryPtr);
        }
    }

    /* add all free segments to segments tree */
    seeked = prvCpssAvlPathSeek(
            tcamSegCfgPtr->segFreeLuTree, PRV_CPSS_AVL_TREE_SEEK_FIRST_E,
            path, &dbEntryPtr);
    while (seeked != GT_FALSE)
    {
        rc = prvCpssAvlItemInsert(segLuTree, dbEntryPtr);
        if (rc != GT_OK)
        {
            cpssOsPrintf("Segment adding to temporary tree failed - will not be printed\n");
        }
        seeked = prvCpssAvlPathSeek(
                tcamSegCfgPtr->segFreeLuTree, PRV_CPSS_AVL_TREE_SEEK_NEXT_E,
                path, &dbEntryPtr);
    }

    cpssOsPrintf("Total Space used by vTcams %d\n", totalSpace);

    /* dump temporary table with all segments */
    cpssOsPrintf("All TCAM Segments (busy and free)\n");
    printSegmentsTree(segLuTree);

    /* cleanup */
    prvCpssAvlTreeDelete(
        segLuTree, (GT_VOIDFUNCPTR)NULL, (GT_VOID*)NULL);
    cpssBmPoolReCreate(nodesPool);
    prvCpssAvlMemPoolDelete(nodesPool);
}

/**
* @internal prvCpssDxChVirtualTcamDbVTcamDumpFullTcamLayout function
* @endinternal
*
* @brief   Dump DB info about full TCAM layout.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                       None.
*/
GT_VOID prvCpssDxChVirtualTcamDbVTcamDumpFullTcamLayout
(
    IN  GT_U32                                      vTcamMngId
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC*           vTcamMngPtr; /* pointer to vTcam Manager */

    vTcamMngPtr = prvCpssDxChVirtualTcamDbVTcamMngGet(vTcamMngId);
    if (vTcamMngPtr == NULL)
    {
        cpssOsPrintf("vTcamMngPtr is NULL, vTcamMngId is %d\n", vTcamMngId);
        return;
    }

    prvCpssDxChVirtualTcamDbVTcamDumpFullTcamLayoutByPtr(&vTcamMngPtr->tcamSegCfg);
    prvCpssDxChVirtualTcamDbVTcamDumpFullTcamLayoutByPtr(&vTcamMngPtr->tcamTtiSegCfg);
}

/**
* @internal prvCpssDxChVirtualTcamDbVTcamDumpSegmentsTree function
* @endinternal
*
* @brief   print the avl segments tree..
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] treeIdPtr                - a pointer to the id of the segments tree
*                                       None
*/
GT_VOID prvCpssDxChVirtualTcamDbVTcamDumpSegmentsTree
(
    IN PRV_CPSS_AVL_TREE_ID treeIdPtr
)
{
    printSegmentsTree(treeIdPtr);
}

