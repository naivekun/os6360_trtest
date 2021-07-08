/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file gtMemoryUsageTrace.c
*
* @brief Tool for tracing memory usage.
* Based on osMemGetHeapBytesAllocated function;
*
* @version   1
********************************************************************************
*/

/********* include ************************************************************/

#include <gtUtil/gtMemoryUsageTrace.h>

/* needed for fast skipping of tracing */
static GT_BOOL gtMemoryUsageTraceEnable = GT_FALSE;

#define MAX_ACHOR_KEYS_CNS  32
#define MAX_ACHOR_KEYS_MEMO_CNS  2048

/* anchor keys - ancor enabled if one of keys is it's name substring substing */
typedef char* CHAR_PTR;
static char anchorKeyMemoArr[MAX_ACHOR_KEYS_MEMO_CNS];
static GT_U32 anchorKeyMemoArrUsed = 0;

/**
* @internal gtMemoryUsageTraceEnableSet function
* @endinternal
*
* @brief   Enables/Disales Memory Usage Tracing
*
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*                                       None.
*/
GT_VOID  gtMemoryUsageTraceEnableSet
(
    IN      GT_BOOL enable
)
{
    gtMemoryUsageTraceEnable = enable;
}

/**
* @internal gtMemoryUsageTraceAnchorKeysReset function
* @endinternal
*
* @brief   Resets anchor keys;
*/
GT_VOID  gtMemoryUsageTraceAnchorKeysReset
(
    IN   GT_VOID
)
{
    anchorKeyMemoArrUsed = 0;
}

/**
* @internal gtMemoryUsageTraceAnchorKeyAdd function
* @endinternal
*
* @brief   Add anchor keys.
*
* @param[in] key                      - anchor  to be addeed.
*                                       None.
*/
GT_VOID  gtMemoryUsageTraceAnchorKeyAdd
(
    IN      char* key
)
{
    GT_U32 keySize;

    keySize = cpssOsStrlen(key) + 1;

    if ((anchorKeyMemoArrUsed + keySize) >= MAX_ACHOR_KEYS_MEMO_CNS) return;

    cpssOsStrCpy(&(anchorKeyMemoArr[anchorKeyMemoArrUsed]), key);
    anchorKeyMemoArrUsed += keySize;
}

/**
* @internal gtMemoryUsageTraceAnchorReset function
* @endinternal
*
* @brief   Reset Anchor.
*
* @param[in] anchorPtr                - (pointer to) anchor.
*                                       None.
*/
GT_VOID  gtMemoryUsageTraceAnchorReset
(
    IN      GT_MEMORY_USAGE_TRACE_ANCHOR_STC *anchorPtr
)
{
    anchorPtr->allocatedBytes = osMemGetHeapBytesAllocated();
}

/**
* @internal gtMemoryUsageTraceNode function
* @endinternal
*
* @brief   Trace node.
*
* @param[in] anchorPtr                - (pointer to) anchor.
* @param[in] nodeName                 - name of checked node.
*                                       None.
*/
GT_VOID  gtMemoryUsageTraceNode
(
    IN      GT_MEMORY_USAGE_TRACE_ANCHOR_STC *anchorPtr,
    IN      const char*                      nodeName
)
{
    GT_U32  allocatedBytes;
    char*  keyPtr;
    GT_BOOL enable;

    if (gtMemoryUsageTraceEnable == GT_FALSE) return;

    /* check if anchor is enabled by key */
    enable = GT_FALSE;
    for (keyPtr = &(anchorKeyMemoArr[0]);
          ((GT_U32)(keyPtr - &(anchorKeyMemoArr[0])) < anchorKeyMemoArrUsed);
          keyPtr += (cpssOsStrlen(keyPtr) + 1))
    {
        if (cpssOsStrStr(anchorPtr->anchorName, keyPtr) != NULL)
        {
            /* key found as substring in anchor name */
            enable = GT_TRUE;
            break;
        }
    }
    if (enable == GT_FALSE) return;

    allocatedBytes = osMemGetHeapBytesAllocated();

    if (anchorPtr->allocatedBytes == 0)
    {
        /* first anchor using - reset */
        anchorPtr->allocatedBytes = allocatedBytes;
        return;
    }

    if (anchorPtr->allocatedBytes == allocatedBytes)
    {
        /* no changes */
        return;
    }

    cpssOsPrintf(
        "total allocated bytes: old %d new %d at %s\n",
        anchorPtr->allocatedBytes, allocatedBytes, nodeName);
    if (anchorPtr->allocatedBytes < allocatedBytes)
    {
        cpssOsPrintf(
            "added allocated bytes  %d \n",
            (allocatedBytes - anchorPtr->allocatedBytes));
    }
    else
    {
        cpssOsPrintf(
            "freed bytes  %d \n",
            (anchorPtr->allocatedBytes - allocatedBytes));
    }

    /* update anchor */
    anchorPtr->allocatedBytes = allocatedBytes;
}


