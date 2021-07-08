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
* @file gtTimeMeasure.h
*
* @brief Tool for measuring exact times of part of code or
* function calls execution.
*
* @version   1
********************************************************************************
*/

#ifndef __gtTimeMeasureh
#define __gtTimeMeasureh

#ifdef __cplusplus
extern "C" {
#endif

/************* Includes *******************************************************/

#include <gtOs/gtGenTypes.h>

#define GT_DEFINE_TIME_EXACT_NODE(_name)                 \
    {                                                    \
        static GT_TIME_EXACT_NODE __node =               \
            {GT_FALSE, _name, NULL, {0,0}, {0,0}, 0};    \
        gtTimeExactProcessNode(&__node);                 \
    }

/**
* @struct GT_TIME_EXACT
 *
 * @brief This structure defines the exact time.
*/
typedef struct{

    /** the whole seconds part of time. */
    GT_U32 seconds;

    /** @brief the nano seconds part of time, less then 1000000000.
     *  Comments:
     *  The maximal value for such time representation more
     *  then 8 years.
     *  The osTimeRT function used for such time retrieving.
     */
    GT_U32 nanoSeconds;

} GT_TIME_EXACT;


/*
 * Typedef: struct GT_TIME_EXACT_NODE
 *
 * Description:
 *     This structure defines the exact time node.
 *     Dedicated to accomulate the time passed to the next time node.
 *
 *
 * Fields:
 *     isInitialized   - GT_FALSE - not initialized, GT_TRUE - initialized
 *     nodeName        - the name of node for printing
 *     nextNodePtr     - pointer to the next node for printing
 *                       not used for summing.
 *     lastMeasure     - the last mesured time.
 *     summaryTime     - the summary time up to the next time node.
 *     eventCount      - event count.
 *
 *  Comments:
 */
typedef struct _pss_GT_TIME_EXACT_NODE
{
    GT_BOOL                        isInitialized;
    char                           *nodeName;
    struct _pss_GT_TIME_EXACT_NODE *nextNodePtr;
    GT_TIME_EXACT                  lastMeasure;
    GT_TIME_EXACT                  summaryTime;
    GT_U32                         eventCount;
} GT_TIME_EXACT_NODE;

/**
* @internal gtTimeExactAdd function
* @endinternal
*
* @brief   Used to add exact times
*
* @param[in] time1Ptr                 - pointer to the time1 for summing
* @param[in] time2Ptr                 - pointer to the time2 for summing
*
* @param[out] sumPtr                   - pointer to the time sum
*                                       None.
*
* @note The sum can be the same than one (or both) of added times
*
*/
GT_VOID  gtTimeExactAdd
(
    IN      GT_TIME_EXACT *time1Ptr,
    IN      GT_TIME_EXACT *time2Ptr,
    OUT     GT_TIME_EXACT *sumPtr
);

/**
* @internal gtTimeExactSub function
* @endinternal
*
* @brief   Used to subtract exact times
*
* @param[in] time1Ptr                 - pointer to the time1 to subtract from
* @param[in] time2Ptr                 - pointer to the time2 to subtract it
*
* @param[out] subPtr                   - pointer to the time sum
*                                       None.
*
* @note The sum can be the same than one (or both) of subtracted times
*
*/
GT_VOID  gtTimeExactSub
(
    IN      GT_TIME_EXACT *time1Ptr,
    IN      GT_TIME_EXACT *time2Ptr,
    OUT     GT_TIME_EXACT *subPtr
);

/**
* @internal gtTimeExactProcessNode function
* @endinternal
*
* @brief   Used to process the node
*
* @param[in,out] nodePtr                  - pointer to processed node
* @param[in,out] nodePtr                  - pointer to processed node
*                                       None.
*/
GT_VOID  gtTimeExactProcessNode
(
    INOUT   GT_TIME_EXACT_NODE *nodePtr
);

/**
* @internal gtTimeExactDumpNodes function
* @endinternal
*
* @brief   Used to dump all the nodes
*/
GT_VOID  gtTimeExactDumpNodes
(
    GT_VOID
);

/**
* @internal gtTimeExactResetNodes function
* @endinternal
*
* @brief   Used to Reset all the nodes
*/
GT_VOID  gtTimeExactResetNodes
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif

#endif  /* __gtTimeMeasureh */
/* Do Not Add Anything Below This Line */



