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
* @file prvCpssDxChVirtualTcamRuleIds.c
*
* @brief The CPSS DXCH High Level Virtual TCAM Manager - Rule Ids and priorities DB
*
* @version   1
********************************************************************************
*/
#include <cpss/dxCh/dxChxGen/virtualTcam/cpssDxChVirtualTcam.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/private/prvCpssDxChVirtualTcam.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/private/prvCpssDxChVirtualTcamDb.h>

/**
* @internal prvCpssDxChVirtualTcamDbRuleIdTableCreate function
* @endinternal
*
* @brief   Create Rule Id Table for TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in,out] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
* @param[in] maxEntriesInDb           - maximal amount entries in DB.
* @param[in,out] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
*                                      allocated objects handlers updated
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChVirtualTcamDbRuleIdTableCreate
(
    INOUT  PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC    *pVTcamMngPtr,
    IN     GT_U32                                maxEntriesInDb
)
{
    GT_STATUS     rc;              /* return code                        */

    /* set NULL values to allocated pointers */
    pVTcamMngPtr->ruleIdPool            = CPSS_BM_POOL_NULL_ID;
    pVTcamMngPtr->ruleIdIdNodesPool     = CPSS_BM_POOL_NULL_ID;
    pVTcamMngPtr->ruleIdIndexNodesPool  = CPSS_BM_POOL_NULL_ID;

    rc = cpssBmPoolCreate(
        sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC),
        CPSS_BM_POOL_4_BYTE_ALIGNMENT_E,
        maxEntriesInDb,
        &(pVTcamMngPtr->ruleIdPool));
    if (rc != GT_OK)
    {
        prvCpssDxChVirtualTcamDbRuleIdTableDelete(pVTcamMngPtr);
        return rc;
    }
    rc = prvCpssAvlMemPoolCreate(
        maxEntriesInDb,
        &(pVTcamMngPtr->ruleIdIdNodesPool));
    if (rc != GT_OK)
    {
        prvCpssDxChVirtualTcamDbRuleIdTableDelete(pVTcamMngPtr);
        return rc;
    }
    rc = prvCpssAvlMemPoolCreate(
        maxEntriesInDb,
        &(pVTcamMngPtr->ruleIdIndexNodesPool));
    if (rc != GT_OK)
    {
        prvCpssDxChVirtualTcamDbRuleIdTableDelete(pVTcamMngPtr);
        return rc;
    }
    return GT_OK;
}


/**
* @internal prvCpssDxChVirtualTcamDbRuleIdTableDelete function
* @endinternal
*
* @brief   Delete Rule Id Table for TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in,out] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
* @param[in,out] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
*                                      freed objects handlers set to NULL
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChVirtualTcamDbRuleIdTableDelete
(
    INOUT  PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC    *pVTcamMngPtr
)
{
    GT_STATUS     rc, rcFinal;              /* return code */

    rcFinal = GT_OK;

    if (pVTcamMngPtr->ruleIdIndexNodesPool != CPSS_BM_POOL_NULL_ID)
    {
        rc = prvCpssAvlMemPoolDelete(pVTcamMngPtr->ruleIdIndexNodesPool);
        if (rc != GT_OK)
        {
            cpssBmPoolReCreate(pVTcamMngPtr->ruleIdIndexNodesPool);
            prvCpssAvlMemPoolDelete(pVTcamMngPtr->ruleIdIndexNodesPool);
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_DBG_PRINT(
                ("prvCpssAvlMemPoolDelete returned %d", rc));
            rcFinal = rc;
        }
        pVTcamMngPtr->ruleIdIndexNodesPool    = CPSS_BM_POOL_NULL_ID;
    }
    if (pVTcamMngPtr->ruleIdIdNodesPool != CPSS_BM_POOL_NULL_ID)
    {
        rc = prvCpssAvlMemPoolDelete(pVTcamMngPtr->ruleIdIdNodesPool);
        if (rc != GT_OK)
        {
            cpssBmPoolReCreate(pVTcamMngPtr->ruleIdIdNodesPool);
            prvCpssAvlMemPoolDelete(pVTcamMngPtr->ruleIdIdNodesPool);
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_DBG_PRINT(
                ("prvCpssAvlMemPoolDelete returned %d", rc));
            rcFinal = rc;
        }
        pVTcamMngPtr->ruleIdIdNodesPool    = CPSS_BM_POOL_NULL_ID;
    }
    if (pVTcamMngPtr->ruleIdPool != CPSS_BM_POOL_NULL_ID)
    {
        rc = cpssBmPoolDelete(pVTcamMngPtr->ruleIdPool);
        if (rc != GT_OK)
        {
            cpssBmPoolReCreate(pVTcamMngPtr->ruleIdPool);
            cpssBmPoolDelete(pVTcamMngPtr->ruleIdPool);
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_DBG_PRINT(
                ("cpssBmPoolDelete returned %d", rc));
            rcFinal = rc;
        }
        pVTcamMngPtr->ruleIdPool    = CPSS_BM_POOL_NULL_ID;
    }
    return rcFinal;
}

/* trivial compare functions for Rule Id table entries */

static GT_COMP_RES prvCpssDxChVirtualTcamDbRuleIdTableRuleIdCompare
(
    IN  GT_VOID    *data_ptr1,
    IN  GT_VOID    *data_ptr2
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC* ptr1;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC* ptr2;

    ptr1 = (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC*)data_ptr1;
    ptr2 = (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC*)data_ptr2;

    if (ptr1->ruleId > ptr2->ruleId)
        return GT_GREATER;
    if (ptr1->ruleId < ptr2->ruleId)
        return GT_SMALLER;
    return GT_EQUAL;
}

static GT_COMP_RES prvCpssDxChVirtualTcamDbRuleIdTableLogicalIndexCompare
(
    IN  GT_VOID    *data_ptr1,
    IN  GT_VOID    *data_ptr2
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC* ptr1;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC* ptr2;

    ptr1 = (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC*)data_ptr1;
    ptr2 = (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC*)data_ptr2;

    if (ptr1->logicalIndex > ptr2->logicalIndex)
        return GT_GREATER;
    if (ptr1->logicalIndex < ptr2->logicalIndex)
        return GT_SMALLER;
    return GT_EQUAL;
}

/**
* @internal prvCpssDxChVirtualTcamDbRuleIdTableVTcamTreesCreate function
* @endinternal
*
* @brief   Create Local per-vTcam Access Trees for Rule Id Table.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
* @param[in,out] tcamCfgPtr               - (pointer to)virtual TCAM structure
* @param[in,out] tcamCfgPtr               - (pointer to)virtual TCAM structure
*                                      allocated objects handlers updated
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChVirtualTcamDbRuleIdTableVTcamTreesCreate
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC          *pVTcamMngPtr,
    INOUT  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC *tcamCfgPtr
)
{
    GT_STATUS     rc;              /* return code                        */

    /* set NULL values to allocated pointers */
    tcamCfgPtr->ruleIdIdTree     = NULL;
    tcamCfgPtr->ruleIdIndexTree  = NULL;

    rc = prvCpssAvlTreeCreate(
        (GT_INTFUNCPTR)prvCpssDxChVirtualTcamDbRuleIdTableRuleIdCompare,
        pVTcamMngPtr->ruleIdIdNodesPool,
        &(tcamCfgPtr->ruleIdIdTree));
    if (rc != GT_OK)
    {
        prvCpssDxChVirtualTcamDbRuleIdTableVTcamTreesDelete(pVTcamMngPtr, tcamCfgPtr);
        return rc;
    }
    rc = prvCpssAvlTreeCreate(
        (GT_INTFUNCPTR)prvCpssDxChVirtualTcamDbRuleIdTableLogicalIndexCompare,
        pVTcamMngPtr->ruleIdIndexNodesPool,
        &(tcamCfgPtr->ruleIdIndexTree));
    if (rc != GT_OK)
    {
        prvCpssDxChVirtualTcamDbRuleIdTableVTcamTreesDelete(pVTcamMngPtr, tcamCfgPtr);
        return rc;
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbRuleIdTableVTcamTreesDelete function
* @endinternal
*
* @brief   Delete Local per-vTcam Access Trees for Rule Id Table.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
* @param[in,out] tcamCfgPtr               - (pointer to)virtual TCAM structure
* @param[in,out] tcamCfgPtr               - (pointer to)virtual TCAM structure
*                                      freed objects handlers updated
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChVirtualTcamDbRuleIdTableVTcamTreesDelete
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC          *pVTcamMngPtr,
    INOUT  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC *tcamCfgPtr
)
{
    GT_STATUS     rc, rcFinal;              /* return code */

    /* compiler warning prevent */
    pVTcamMngPtr = pVTcamMngPtr;

    rcFinal = GT_OK;

    if (tcamCfgPtr->ruleIdIdTree != NULL)
    {
        rc = prvCpssAvlTreeDelete(
            tcamCfgPtr->ruleIdIdTree, (GT_VOIDFUNCPTR)NULL, (GT_VOID*)NULL);
        if (rc != GT_OK)
        {
            rcFinal = rc;
        }
        tcamCfgPtr->ruleIdIdTree     = NULL;
    }
    if (tcamCfgPtr->ruleIdIndexTree != NULL)
    {
        rc = prvCpssAvlTreeDelete(
            tcamCfgPtr->ruleIdIndexTree, (GT_VOIDFUNCPTR)NULL, (GT_VOID*)NULL);
        if (rc != GT_OK)
        {
            rcFinal = rc;
        }
        tcamCfgPtr->ruleIdIndexTree  = NULL;
    }
    return rcFinal;
}

/**
* @internal prvCpssDxChVirtualTcamDbPriorityTableCreate function
* @endinternal
*
* @brief   Create Priority Table for TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in,out] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
* @param[in] maxEntriesInDb           - maximal amount entries in DB.
* @param[in,out] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
*                                      Handle to allocted table updated
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChVirtualTcamDbPriorityTableCreate
(
    INOUT   PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC  *pVTcamMngPtr,
    IN      GT_U32                              maxEntriesInDb
)
{
    GT_STATUS     rc;              /* return code                        */

    /* set NULL values to allocated pointers */
    pVTcamMngPtr->priorityPool            = CPSS_BM_POOL_NULL_ID;
    pVTcamMngPtr->priorityPriNodesPool    = CPSS_BM_POOL_NULL_ID;
    pVTcamMngPtr->priorityIndexNodesPool  = CPSS_BM_POOL_NULL_ID;

    rc = cpssBmPoolCreate(
        sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC),
        CPSS_BM_POOL_4_BYTE_ALIGNMENT_E,
         maxEntriesInDb,
        &(pVTcamMngPtr->priorityPool));
    if (rc != GT_OK)
    {
        prvCpssDxChVirtualTcamDbPriorityTableDelete(pVTcamMngPtr);
        return rc;
    }
    rc = prvCpssAvlMemPoolCreate(
        maxEntriesInDb,
        &(pVTcamMngPtr->priorityPriNodesPool));
    if (rc != GT_OK)
    {
        prvCpssDxChVirtualTcamDbPriorityTableDelete(pVTcamMngPtr);
        return rc;
    }
    rc = prvCpssAvlMemPoolCreate(
        maxEntriesInDb,
        &(pVTcamMngPtr->priorityIndexNodesPool));
    if (rc != GT_OK)
    {
        prvCpssDxChVirtualTcamDbPriorityTableDelete(pVTcamMngPtr);
        return rc;
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbPriorityTableDelete function
* @endinternal
*
* @brief   Delete Priority Table for TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in,out] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
* @param[in,out] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
*                                      Handle to freed table set to NULL
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChVirtualTcamDbPriorityTableDelete
(
    INOUT   PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC  *pVTcamMngPtr
)
{
    GT_STATUS     rc, rcFinal;              /* return code */

    rcFinal = GT_OK;

    if (pVTcamMngPtr->priorityIndexNodesPool != CPSS_BM_POOL_NULL_ID)
    {
        rc = prvCpssAvlMemPoolDelete(pVTcamMngPtr->priorityIndexNodesPool);
        if (rc != GT_OK)
        {
            cpssBmPoolReCreate(pVTcamMngPtr->priorityIndexNodesPool);
            prvCpssAvlMemPoolDelete(pVTcamMngPtr->priorityIndexNodesPool);
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_DBG_PRINT(
                ("prvCpssAvlMemPoolDelete returned %d", rc));
            rcFinal = rc;
        }
        pVTcamMngPtr->priorityIndexNodesPool    = CPSS_BM_POOL_NULL_ID;
    }
    if (pVTcamMngPtr->priorityPriNodesPool != CPSS_BM_POOL_NULL_ID)
    {
        rc = prvCpssAvlMemPoolDelete(pVTcamMngPtr->priorityPriNodesPool);
        if (rc != GT_OK)
        {
            cpssBmPoolReCreate(pVTcamMngPtr->priorityPriNodesPool);
            prvCpssAvlMemPoolDelete(pVTcamMngPtr->priorityPriNodesPool);
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_DBG_PRINT(
                ("prvCpssAvlMemPoolDelete returned %d", rc));
            rcFinal = rc;
        }
        pVTcamMngPtr->priorityPriNodesPool    = CPSS_BM_POOL_NULL_ID;
    }
    if (pVTcamMngPtr->priorityPool != CPSS_BM_POOL_NULL_ID)
    {
        rc = cpssBmPoolDelete(pVTcamMngPtr->priorityPool);
        if (rc != GT_OK)
        {
            cpssBmPoolReCreate(pVTcamMngPtr->priorityPool);
            cpssBmPoolDelete(pVTcamMngPtr->priorityPool);
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_DBG_PRINT(
                ("cpssBmPoolDelete returned %d", rc));
            rcFinal = rc;
        }
        pVTcamMngPtr->priorityPool    = CPSS_BM_POOL_NULL_ID;
    }
    return rcFinal;
}

/* trivial compare functions for Priority table entries */

static GT_COMP_RES prvCpssDxChVirtualTcamDbPriorityTablePriorityCompare
(
    IN  GT_VOID    *data_ptr1,
    IN  GT_VOID    *data_ptr2
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC* ptr1;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC* ptr2;

    ptr1 = (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC*)data_ptr1;
    ptr2 = (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC*)data_ptr2;

    if (ptr1->priority > ptr2->priority)
        return GT_GREATER;
    if (ptr1->priority < ptr2->priority)
        return GT_SMALLER;
    return GT_EQUAL;
}

static GT_COMP_RES prvCpssDxChVirtualTcamDbPriorityTableLogicalIndexCompare
(
    IN  GT_VOID    *data_ptr1,
    IN  GT_VOID    *data_ptr2
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC* ptr1;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC* ptr2;

    ptr1 = (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC*)data_ptr1;
    ptr2 = (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC*)data_ptr2;

    if (ptr1->baseLogIndex > ptr2->baseLogIndex)
        return GT_GREATER;
    if (ptr1->baseLogIndex < ptr2->baseLogIndex)
        return GT_SMALLER;
    return GT_EQUAL;
}

/**
* @internal prvCpssDxChVirtualTcamDbPriorityTableVTcamTreesCreate function
* @endinternal
*
* @brief   Create Local per-vTcam Access Trees for Priority Table.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
* @param[in,out] tcamCfgPtr               - (pointer to)virtual TCAM structure
* @param[in,out] tcamCfgPtr               - (pointer to)virtual TCAM structure
*                                      allocated objects handlers updated
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChVirtualTcamDbPriorityTableVTcamTreesCreate
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC          *pVTcamMngPtr,
    INOUT  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC *tcamCfgPtr
)
{
    GT_STATUS     rc;              /* return code                        */

    /* set NULL values to allocated pointers */
    tcamCfgPtr->priorityPriTree     = NULL;
    tcamCfgPtr->priorityIndexTree  = NULL;

    rc = prvCpssAvlTreeCreate(
        (GT_INTFUNCPTR)prvCpssDxChVirtualTcamDbPriorityTablePriorityCompare,
        pVTcamMngPtr->priorityPriNodesPool,
        &(tcamCfgPtr->priorityPriTree));
    if (rc != GT_OK)
    {
        prvCpssDxChVirtualTcamDbPriorityTableVTcamTreesDelete(pVTcamMngPtr, tcamCfgPtr);
        return rc;
    }
    rc = prvCpssAvlTreeCreate(
        (GT_INTFUNCPTR)prvCpssDxChVirtualTcamDbPriorityTableLogicalIndexCompare,
        pVTcamMngPtr->priorityIndexNodesPool,
        &(tcamCfgPtr->priorityIndexTree));
    if (rc != GT_OK)
    {
        prvCpssDxChVirtualTcamDbPriorityTableVTcamTreesDelete(pVTcamMngPtr, tcamCfgPtr);
        return rc;
    }
    return GT_OK;
}


/**
* @internal prvCpssDxChVirtualTcamDbPriorityTableVTcamTreesDelete function
* @endinternal
*
* @brief   Delete Local per-vTcam Access Trees for Priority Table.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
* @param[in,out] tcamCfgPtr               - (pointer to)virtual TCAM structure
* @param[in,out] tcamCfgPtr               - (pointer to)virtual TCAM structure
*                                      freed objects handlers updated
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChVirtualTcamDbPriorityTableVTcamTreesDelete
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC          *pVTcamMngPtr,
    INOUT  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC *tcamCfgPtr
)
{
    GT_STATUS     rc, rcFinal;              /* return code */

    /* compiler warning prevent */
    pVTcamMngPtr = pVTcamMngPtr;

    rcFinal = GT_OK;

    if (tcamCfgPtr->priorityPriTree != NULL)
    {
        rc = prvCpssAvlTreeDelete(
            tcamCfgPtr->priorityPriTree, (GT_VOIDFUNCPTR)NULL, (GT_VOID*)NULL);
        if (rc != GT_OK)
        {
            rcFinal = rc;
        }
        tcamCfgPtr->priorityPriTree     = NULL;
    }
    if (tcamCfgPtr->priorityIndexTree != NULL)
    {
        rc = prvCpssAvlTreeDelete(
            tcamCfgPtr->priorityIndexTree, (GT_VOIDFUNCPTR)NULL, (GT_VOID*)NULL);
        if (rc != GT_OK)
        {
            rcFinal = rc;
        }
        tcamCfgPtr->priorityIndexTree  = NULL;
    }
    return rcFinal;
}




