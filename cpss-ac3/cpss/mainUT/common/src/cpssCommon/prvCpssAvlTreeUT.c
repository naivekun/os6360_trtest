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
* @file prvCpssAvlTreeUT.c
*
* @brief Unit tests for AVL Tree libtary.
*
* @version   1
********************************************************************************
*/

/* includes */
#include <cpssCommon/private/prvCpssAvlTree.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/* Automatic tests will be added later                        */
/* Now here placed functions for manual debugging the library */
/* The will also called from  Automatic tests                 */

/* debug print macro - will be redefined as UTF_LOG ... */
#define PRINT(_param_list) cpssOsPrintf _param_list

/* conversions GT_VOID* <===> GT_U32 */
#define GT_U32_TO_GT_VOID_PTR_MAC(_u32) (GT_VOID*)(_u32 + (char*)0)
#define GT_VOID_PTR_TO_GT_U32_MAC(_void_ptr) (GT_U32)((char*)_void_ptr - (char*)0)

static PRV_CPSS_AVL_TREE_PATH    debugTreePath = {0};
static CPSS_BM_POOL_ID           debugPoolId = NULL;
static PRV_CPSS_AVL_TREE_ID      debugTreeId = NULL;

static GT_COMP_RES debugGT_U32CompareFunc
(
    IN  GT_VOID    *data_ptr1,
    IN  GT_VOID    *data_ptr2
)
{
    GT_U32 data1;
    GT_U32 data2;

    data1 = GT_VOID_PTR_TO_GT_U32_MAC(data_ptr1);
    data2 = GT_VOID_PTR_TO_GT_U32_MAC(data_ptr2);

    if(data1 == data2)
        return GT_EQUAL;
    if(data1 < data2)
        return GT_SMALLER;
    if(data1 > data2)
        return GT_GREATER;
    /* never occurs */
    return GT_EQUAL;
}

static GT_VOID debugCleanupTree()
{
    if (debugTreeId != NULL)
    {
        prvCpssAvlTreeDelete(
            debugTreeId,
            (GT_VOIDFUNCPTR)NULL /*dataFreeFunc*/,
            (GT_VOID*)NULL       /*cookiePtr*/);
        debugTreeId = NULL;
    }
    if (debugPoolId != NULL)
    {
        prvCpssAvlMemPoolDelete(debugPoolId);
        debugPoolId = NULL;
    }
}

static void debugCreateTree(GT_U32 maxNodes)
{
    GT_STATUS rc;

    if (debugPoolId == NULL)
    {
        rc = prvCpssAvlMemPoolCreate(
            maxNodes, &debugPoolId);
        if (rc != GT_OK)
        {
            PRINT(("prvCpssAvlMemPoolCreate failed\n"));
            debugCleanupTree();
            return;
        }
    }
    if (debugTreeId == NULL)
    {
        rc = prvCpssAvlTreeCreate(
            (GT_INTFUNCPTR)debugGT_U32CompareFunc,
            debugPoolId, &debugTreeId);
        if (rc != GT_OK)
        {
            PRINT(("prvCpssAvlTreeCreate failed\n"));
            debugCleanupTree();
            return;
        }
    }
}

static void debugTreeAdd(char* option, GT_U32 base, GT_U32 amount, GT_U32 increment)
{
    GT_STATUS rc;
    GT_U32    i, val;
    GT_BOOL toIncrement;

    if (cpssOsStrCmp(option, "inc") == 0)
    {
        toIncrement = GT_TRUE;
    }
    else if (cpssOsStrCmp(option, "dec") == 0)
    {
        toIncrement = GT_FALSE;
    }
    else
    {
        PRINT(("wrong option - <inc> used\n"));
        toIncrement = GT_TRUE;
    }

    for (i = 0; (i < amount); i++)
    {
        if (toIncrement != GT_FALSE)
        {
            val = base + (i * increment);
        }
        else
        {
            val = base - (i * increment);
        }
        rc = prvCpssAvlItemInsert(debugTreeId, GT_U32_TO_GT_VOID_PTR_MAC(val));
        if (rc != GT_OK)
        {
            PRINT(("prvCpssAvlItemInsert failed\n"));
            return;
        }
    }
}

static void debugTreeDel(char* option, GT_U32 base, GT_U32 amount, GT_U32 increment)
{
    GT_VOID*  pResVal;
    GT_U32    i, val;
    GT_BOOL   toIncrement;

    if (cpssOsStrCmp(option, "inc") == 0)
    {
        toIncrement = GT_TRUE;
    }
    else if (cpssOsStrCmp(option, "dec") == 0)
    {
        toIncrement = GT_FALSE;
    }
    else
    {
        PRINT(("wrong option - <inc> used\n"));
        toIncrement = GT_TRUE;
    }

    for (i = 0; (i < amount); i++)
    {
        if (toIncrement != GT_FALSE)
        {
            val = base + (i * increment);
        }
        else
        {
            val = base - (i * increment);
        }
        pResVal = prvCpssAvlItemRemove(debugTreeId, GT_U32_TO_GT_VOID_PTR_MAC(val));
        if (pResVal == NULL)
        {
            PRINT(("prvCpssAvlItemRemove failed\n"));
            return;
        }
    }
}

/* used not public macros copied from prvCpssAvlTree.c */
#define PRV_AVL_TREE_PATH_BYTE_MAC(_path, _deep) _path[(_deep + 6) / 8]
#define PRV_AVL_TREE_PATH_BIT_MAC(_deep) (1 << ((_deep + 6) % 8))

#define AVL_TREE_PATH_DEEP_GET_MAC(_path) (_path[0] & 0x3F)
/* get/set way (left == 0, rigt = 1) at deepness = _deep in the (_deep + 6)th bit */
#define AVL_TREE_PATH_WAY_GET_MAC(_path, _deep) \
    ((PRV_AVL_TREE_PATH_BYTE_MAC(_path, _deep) & PRV_AVL_TREE_PATH_BIT_MAC(_deep)) ? 1 : 0)

static void debugTreePrintPath(PRV_CPSS_AVL_TREE_PATH path, GT_U32 val)
{
    GT_U32 i, depth, way;

    PRINT(("%8.8d ", val));

    depth = AVL_TREE_PATH_DEEP_GET_MAC(path);
    for (i = 0; (i < depth); i++)
    {
        way = AVL_TREE_PATH_WAY_GET_MAC(path, i);
        PRINT(((way ? "R" : "L")));
    }
    PRINT(("\n"));
}

static void debugTreeFindPath(char* option, GT_U32 key)
{
    GT_BOOL                    rc;
    PRV_CPSS_AVL_TREE_FIND_ENT findOption;
    GT_VOID*                   pFoundData;

    if (cpssOsStrCmp(option, "==") == 0)
    {
        findOption = PRV_CPSS_AVL_TREE_FIND_EQUAL_E;
    }
    else if (cpssOsStrCmp(option, "<=") == 0)
    {
        findOption = PRV_CPSS_AVL_TREE_FIND_MAX_LESS_OR_EQUAL_E;
    }
    else if (cpssOsStrCmp(option, ">=") == 0)
    {
        findOption = PRV_CPSS_AVL_TREE_FIND_MIN_GREATER_OR_EQUAL_E;
    }
    else
    {
        PRINT(("wrong option - {==} used\n"));
        findOption = PRV_CPSS_AVL_TREE_FIND_EQUAL_E;
    }
    rc = prvCpssAvlPathFind(
        debugTreeId, findOption, GT_U32_TO_GT_VOID_PTR_MAC(key), debugTreePath, &pFoundData);
    if (rc == GT_FALSE)
    {
        PRINT(("prvCpssAvlPathFind failed\n"));
        return;
    }
    debugTreePrintPath(debugTreePath, GT_VOID_PTR_TO_GT_U32_MAC(pFoundData));
}

static void debugTreeSeekPath(char* option)
{
    GT_BOOL                    rc;
    PRV_CPSS_AVL_TREE_SEEK_ENT seekOption;
    GT_VOID*                   pSeekedData;

    if (cpssOsStrCmp(option, "this") == 0)
    {
        seekOption = PRV_CPSS_AVL_TREE_SEEK_THIS_E;
    }
    else if (cpssOsStrCmp(option, "next") == 0)
    {
        seekOption = PRV_CPSS_AVL_TREE_SEEK_NEXT_E;
    }
    else if (cpssOsStrCmp(option, "prev") == 0)
    {
        seekOption = PRV_CPSS_AVL_TREE_SEEK_PREVIOUS_E;
    }
    else if (cpssOsStrCmp(option, "first") == 0)
    {
        seekOption = PRV_CPSS_AVL_TREE_SEEK_FIRST_E;
    }
    else if (cpssOsStrCmp(option, "last") == 0)
    {
        seekOption = PRV_CPSS_AVL_TREE_SEEK_LAST_E;
    }
    else
    {
        PRINT(("wrong option - <this> used\n"));
        seekOption = PRV_CPSS_AVL_TREE_SEEK_THIS_E;
    }
    rc = prvCpssAvlPathSeek(
        debugTreeId, seekOption, debugTreePath, &pSeekedData);
    if (rc == GT_FALSE)
    {
        PRINT(("prvCpssAvlPathSeek failed\n"));
        return;
    }
    debugTreePrintPath(debugTreePath, GT_VOID_PTR_TO_GT_U32_MAC(pSeekedData));
}

static void debugTreeDumpPath(char* option, GT_U32 baseKey, GT_U32 amount)
{
    GT_BOOL                    rc;
    PRV_CPSS_AVL_TREE_FIND_ENT findOption;
    PRV_CPSS_AVL_TREE_SEEK_ENT seekOption;
    PRV_CPSS_AVL_TREE_SEEK_ENT seekStartOption;
    GT_VOID*                   pFoundData;
    GT_U32                     count;

    if (cpssOsStrCmp(option, "from_ascend") == 0)
    {
        findOption = PRV_CPSS_AVL_TREE_FIND_MAX_LESS_OR_EQUAL_E;
        seekStartOption = PRV_CPSS_AVL_TREE_SEEK_THIS_E; /* no seek */
        seekOption = PRV_CPSS_AVL_TREE_SEEK_NEXT_E;
    }
    else if (cpssOsStrCmp(option, "from_descend") == 0)
    {
        findOption = PRV_CPSS_AVL_TREE_FIND_MIN_GREATER_OR_EQUAL_E;
        seekStartOption = PRV_CPSS_AVL_TREE_SEEK_THIS_E; /* no seek */
        seekOption = PRV_CPSS_AVL_TREE_SEEK_PREVIOUS_E;
    }
    else if (cpssOsStrCmp(option, "all_ascend") == 0)
    {
        findOption = PRV_CPSS_AVL_TREE_FIND_EQUAL_E; /*no find*/
        seekStartOption = PRV_CPSS_AVL_TREE_SEEK_FIRST_E;
        seekOption = PRV_CPSS_AVL_TREE_SEEK_NEXT_E;
    }
    else if (cpssOsStrCmp(option, "all_descend") == 0)
    {
        findOption = PRV_CPSS_AVL_TREE_FIND_EQUAL_E; /*no find*/
        seekStartOption = PRV_CPSS_AVL_TREE_SEEK_LAST_E;
        seekOption = PRV_CPSS_AVL_TREE_SEEK_PREVIOUS_E;
    }
    else
    {
        PRINT(("wrong option - <all_ascend> used\n"));
        findOption = PRV_CPSS_AVL_TREE_FIND_EQUAL_E; /*no find*/
        seekStartOption = PRV_CPSS_AVL_TREE_SEEK_FIRST_E;
        seekOption = PRV_CPSS_AVL_TREE_SEEK_NEXT_E;
    }

    if (findOption != PRV_CPSS_AVL_TREE_FIND_EQUAL_E)
    {
        /* find */
        rc = prvCpssAvlPathFind(
            debugTreeId, findOption, GT_U32_TO_GT_VOID_PTR_MAC(baseKey), debugTreePath, &pFoundData);
        if (rc == GT_FALSE)
        {
            PRINT(("prvCpssAvlPathFind failed\n"));
            return;
        }
    }
    else
    {
        rc = prvCpssAvlPathSeek(
            debugTreeId, seekStartOption, debugTreePath, &pFoundData);
        if (rc == GT_FALSE)
        {
            PRINT(("prvCpssAvlPathSeek failed\n"));
            return;
        }
    }
    debugTreePrintPath(debugTreePath, GT_VOID_PTR_TO_GT_U32_MAC(pFoundData));

    for (count = 1; (1); count++)
    {
        rc = prvCpssAvlPathSeek(
            debugTreeId, seekOption, debugTreePath, &pFoundData);
        if (rc == GT_FALSE)
        {
            PRINT(("prvCpssAvlPathSeek failed\n"));
            return;
        }
        debugTreePrintPath(debugTreePath, GT_VOID_PTR_TO_GT_U32_MAC(pFoundData));
        if (findOption != PRV_CPSS_AVL_TREE_FIND_EQUAL_E)
        {
            /* find case */
            if (count > amount)
            {
                break;
            }
        }
    }
}

void debugCpssAvlCmdLine(
        char* cmd, char* option,
        GT_U32 n0, GT_U32 n1, GT_U32 n2)
{
    if ((cmd == NULL) || (cpssOsStrCmp(cmd, "") == 0))
    {
        PRINT(("debugCpssAvlCmdLine use:\n"));
        PRINT(("create tree <maxNodes>\n"));
        PRINT(("cleanup\n"));
        PRINT(("add {inc | dec } <base> <amount> <increment>\n"));
        PRINT(("del {inc | dec } <base> <amount> <increment>\n"));
        PRINT(("find_path { == | <= | >= } <key>\n"));
        PRINT(("seek_path {this next prev first last}\n"));
        PRINT(("dump {from_ascend | from_descend | all_ascend | all_descend} <base_key> <amount> \n"));
        return;
    }
    else if (cpssOsStrCmp(cmd, "create") == 0)
    {
        debugCreateTree(n0 /*maxNodes*/);
        return;
    }
    else if (cpssOsStrCmp(cmd, "cleanup") == 0)
    {
        debugCleanupTree();
        return;
    }
    else if (cpssOsStrCmp(cmd, "add") == 0)
    {
        debugTreeAdd(option, n0 /*base*/, n1 /*amount*/, n2 /*increment*/);
        return;
    }
    else if (cpssOsStrCmp(cmd, "del") == 0)
    {
        debugTreeDel(option, n0 /*base*/, n1 /*amount*/, n2 /*increment*/);
        return;
    }
    else if (cpssOsStrCmp(cmd, "find_path") == 0)
    {
        debugTreeFindPath(option, n0 /*key*/);
        return;
    }
    else if (cpssOsStrCmp(cmd, "seek_path") == 0)
    {
        debugTreeSeekPath(option);
        return;
    }
    else if (cpssOsStrCmp(cmd, "dump") == 0)
    {
        debugTreeDumpPath(option, n0 /*baseKey*/, n1 /*amount*/);
        return;
    }
    else
    {
        debugCpssAvlCmdLine(NULL, NULL, 0, 0, 0);
        return;
    }
}

/*
Test sequence
@@debugCpssAvlCmdLine  "create","",100
@@debugCpssAvlCmdLine  "add","inc",100,10,1
@@debugCpssAvlCmdLine  "dump","all_ascend"
@@debugCpssAvlCmdLine  "dump","all_descend"
@@debugCpssAvlCmdLine  "del","dec",109,5,2
@@debugCpssAvlCmdLine  "dump","all_ascend"
@@debugCpssAvlCmdLine  "dump","all_descend"
@@debugCpssAvlCmdLine  "find_path","==",106
@@debugCpssAvlCmdLine  "seek_path","prev"
@@debugCpssAvlCmdLine  "find_path","<=",106
@@debugCpssAvlCmdLine  "seek_path","next"
@@debugCpssAvlCmdLine  "find_path","<=",105
@@debugCpssAvlCmdLine  "find_path",">=",104
@@debugCpssAvlCmdLine  "find_path","==",104
@@debugCpssAvlCmdLine  "find_path",">=",103
@@debugCpssAvlCmdLine  "find_path","==",103
@@debugCpssAvlCmdLine  "seek_path","first"
@@debugCpssAvlCmdLine  "seek_path","last"
@@debugCpssAvlCmdLine  "seek_path","this"
@@debugCpssAvlCmdLine  "cleanup"
*/

