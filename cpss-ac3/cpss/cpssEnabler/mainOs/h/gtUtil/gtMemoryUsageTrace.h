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
* @file gtMemoryUsageTrace.h
*
* @brief Tool for tracing memory usage.
* Based on osMemGetHeapBytesAllocated function;
*
* @version   1
********************************************************************************
*/

#ifndef __gtMemoryUsageTraceh
#define __gtMemoryUsageTraceh

#ifdef __cplusplus
extern "C" {
#endif

/************* Includes *******************************************************/

#include <gtOs/gtGenTypes.h>
#include <gtOs/gtOsMem.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

/**
* @struct GT_MEMORY_USAGE_TRACE_ANCHOR_STC
 *
 * @brief This structure describe memory usage trace anchor.
*/
typedef struct{

    const char*  anchorName;

    /** @brief total amount of bytes that was allocated
     *  before last node processing.
     *  Comments:
     */
    GT_U32 allocatedBytes;

} GT_MEMORY_USAGE_TRACE_ANCHOR_STC;

/* Trace Anchor declaration */
#define GT_MEMORY_USAGE_TRACE_ANCHOR_DEFINE_GEN_MAC(_var, _name) \
    GT_MEMORY_USAGE_TRACE_ANCHOR_STC _var = {_name, 0};

/* Trace Anchor declaration with short variable name */
#define GT_MEMORY_USAGE_TRACE_ANCHOR_DEFINE_MAC(_var, _name) \
    GT_MEMORY_USAGE_TRACE_ANCHOR_DEFINE_GEN_MAC(GT_MEMORY_USAGE_TRACE_ANCHOR##_var, _name) \

/* trace node */
#define GT_MEMORY_USAGE_TRACE_NODE_GEN_MAC(_anchor_var, _node_name) \
    gtMemoryUsageTraceNode(&_anchor_var, _node_name)

/* trace node with short variable name */
#define GT_MEMORY_USAGE_TRACE_NODE_MAC(_anchor_var, _node_name) \
    GT_MEMORY_USAGE_TRACE_NODE_GEN_MAC(GT_MEMORY_USAGE_TRACE_ANCHOR##_anchor_var, _node_name)

/* isolated node with it's own anchore to compare memory allocation */
/* at different executions of the same code lines                   */
#define GT_MEMORY_USAGE_TRACE_ISOLATED_NODE_MAC(_anchor_name, _node_name)           \
    {                                                                               \
        static GT_MEMORY_USAGE_TRACE_ANCHOR_DEFINE_GEN_MAC(__anchor, _anchor_name); \
        GT_MEMORY_USAGE_TRACE_NODE_GEN_MAC(__anchor, _node_name);                   \
    }

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
);

/**
* @internal gtMemoryUsageTraceAnchorKeysReset function
* @endinternal
*
* @brief   Resets anchor keys;
*/
GT_VOID  gtMemoryUsageTraceAnchorKeysReset
(
    IN    GT_VOID
);

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
);

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
);

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
);

#ifdef __cplusplus
}
#endif

#endif  /* __gtMemoryUsageTraceh */
/* Do Not Add Anything Below This Line */



