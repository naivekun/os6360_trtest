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
* @file prvCpssAvlTree.h
*
* @brief AVL Tree implementation.
*
* @version   1
********************************************************************************
*/


#ifndef __prvCpssAvlTree_h
#define __prvCpssAvlTree_h

#ifdef __cplusplus
extern "C" {
#endif


/************* Includes *******************************************************/

#include <cpss/extServices/os/gtOs/gtGenTypes.h>
#include <cpssCommon/cpssBuffManagerPool.h>


/************ Typedefs ***********************************************/

/*
 * Typedef: PRV_CPSS_AVL_TREE_ID
 *
 * Description: type of tree's ID
 *
 */
typedef GT_VOID * PRV_CPSS_AVL_TREE_ID;

/*
 * Typedef: PRV_CPSS_AVL_TREE_ITERATOR_ID
 *
 * Description: type of tree's iterator ID
 *
 */
typedef GT_VOID *PRV_CPSS_AVL_TREE_ITERATOR_ID;

/************ Public Functions ************************************************/

/**
* @internal prvCpssAvlMemPoolCreate function
* @endinternal
*
* @brief   Create memory pool to be used by the AVL tree.
*
* @param[in] maxNode                  - Max AVL nodes expected to be useed. The function will
*                                      allocate memory to support that number of nodes.
*
* @param[out] pPoolId                  - Return the created pool's ID.
*
* @retval GT_STATUS      GT_OK     - For successful operation.
*                                       Otherwise return the returned status from the pool function.
*/
GT_STATUS prvCpssAvlMemPoolCreate
(
    IN   GT_U32            maxNode,
    OUT  CPSS_BM_POOL_ID*  pPoolId
);

/**
* @internal prvCpssAvlMemPoolDelete function
* @endinternal
*
* @brief   Delete memory pool used by AVL tree.
*
* @retval GT_STATUS      GT_OK     - For successful operation.
*                                       Otherwise return the returned status from the pool function.
*/
GT_STATUS prvCpssAvlMemPoolDelete
(
    IN  CPSS_BM_POOL_ID   poolId
);

/**
* @internal prvCpssAvlTreeCreate function
* @endinternal
*
* @brief   Create AVL tree
*
* @param[in] compareFunc              - Comparison function used to sort the nodes in the tree.
*                                      recieves two node's data and returns:
*                                      GT_EQUAL / GT_GREATER / GT_SMALLER
* @param[in] poolId                   - pool ID of the buffers's pool to be used for memory allocation.
*                                      If In case pool ID is GT_POOL_NULL_ID, memory will be allocated
*                                      by malloc function.
*
* @param[out] pTreeId                  - Return the ID (handler) of the new created tree.
*
* @retval GT_STATUS      GT_OK     - For successful operation.
* @retval GT_NO_RESOURCE           - No memory is available to create that tree.
*/
GT_STATUS prvCpssAvlTreeCreate
(
    IN   GT_INTFUNCPTR                compareFunc,
    IN   CPSS_BM_POOL_ID              poolId,
    OUT  PRV_CPSS_AVL_TREE_ID         *pTreeId
);

/**
* @internal prvCpssAvlTreeDelete function
* @endinternal
*
* @brief   Delete AVL tree
*
* @param[in] treeId                   - ID of the tree to be deleted.
* @param[in] dataFreeFunc             - A pointer to a function to be called for deleteing the
*                                      user data, or NULL if no free operation should be done
*                                      on the user's data. The function gets 2 parameters:
*                                      cookie pointer and data ponter from node.
* @param[in] cookiePtr                - cookie parameter to be passed to dataFreeFunc function.
*
* @retval GT_STATUS      GT_OK     - For successful operation.
* @retval GT_NOT_INITIALIZED       - the given tree has not been initialized.
* @retval GT_BAD_STATE             - In case the tree is not empty.
*/
GT_STATUS prvCpssAvlTreeDelete
(
    IN  PRV_CPSS_AVL_TREE_ID      treeId,
    IN  GT_VOIDFUNCPTR            dataFreeFunc,
    IN  GT_VOID*                  cookiePtr
);

/**
* @internal prvCpssAvlTreeReset function
* @endinternal
*
* @brief   Reset AVL tree - remove all nodes
*
* @param[in] treeId                   - ID of the tree to be reseted.
* @param[in] dataFreeFunc             - A pointer to a function to be called for deleteing the
*                                      user data, or NULL if no free operation should be done
*                                      on the user's data. The function gets 2 parameters:
*                                      cookie pointer and data ponter from node.
* @param[in] cookiePtr                - cookie parameter to be passed to dataFreeFunc function.
*
* @retval GT_STATUS      GT_OK     - For successful operation.
* @retval GT_NOT_INITIALIZED       - the given tree has not been initialized.
* @retval GT_BAD_STATE             - In case the tree is not empty.
*/
GT_STATUS prvCpssAvlTreeReset
(
    IN  PRV_CPSS_AVL_TREE_ID      treeId,
    IN  GT_VOIDFUNCPTR            dataFreeFunc,
    IN  GT_VOID*                  cookiePtr
);

/**
* @internal prvCpssAvlItemInsert function
* @endinternal
*
* @brief   insert item into the AVL tree.
*
* @param[in] treeId                   - ID of the tree.
* @param[in] pData                    - user's data pointer to be inserted into the tree.
*
* @retval GT_STATUS      GT_OK     - For successful operation.
* @retval GT_NOT_INITIALIZED       - The given tree has not been initialized.
* @retval GT_ALREADY_EXIST         - Item already exist in the tree.
* @retval GT_NO_RESOURCE           - There is no memory to add the new item.
* @retval GT_FAIL                  - General error.
*/
GT_STATUS prvCpssAvlItemInsert
(
    IN  PRV_CPSS_AVL_TREE_ID  treeId,
    IN  GT_VOID*              pData
);

/*******************************************************************************
* prvCpssAvlItemRemove
*
* DESCRIPTION:  Remove item from the tree
*
*
* INPUTS:
*       treeId - ID of the tree.
*       pData - user's data pointer to be removed from the tree.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_VOID* - A pointer to the data stored in the deleted node, or
*                  NULL if no such node exists.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_VOID* prvCpssAvlItemRemove
(
    IN  PRV_CPSS_AVL_TREE_ID  treeId,
    IN  GT_VOID*              pData
);

/*******************************************************************************
* prvCpssAvlSearch
*
* DESCRIPTION:
*       This function searches for a given key in the avl tree.
*
* INPUTS:
*       treeId - ID of the tree.
*       pData   - a pointer to the data to be searched.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_VOID* - A pointer to the data stored in the found node, or
*                  NULL if no such node exists.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_VOID* prvCpssAvlSearch
(
    IN  PRV_CPSS_AVL_TREE_ID  treeId,
    IN  GT_VOID*              pData
);

/**
* @internal prvCpssAvlIsEmpty function
* @endinternal
*
* @brief   checks if a the given tree is empty.
*
* @param[in] treeId                   - ID of the tree.
*                                       GT_TRUE if the tree is empty, GT_FALSE otherwise.
*
* @note The funtion returns with GT_TRUE also in case of error.
*
*/
GT_BOOL prvCpssAvlIsEmpty
(
    IN  PRV_CPSS_AVL_TREE_ID  treeId
);

/*******************************************************************************
* prvCpssAvlFirstDataGet
*
* DESCRIPTION:
*       search for a given key in the AVL tree.
*       This function returns a pointer to the data stored in the first node
*       with key >= KEY(pData), OR NULL if there is no such node.
*
* INPUTS:
*       treeId - ID of the tree.
*       pData - user's data pointer to search for.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_VOID* - A pointer to the first node with data >= pData.
*       NULL will be returned if no such node exists or the given tree
*       has not been initialized.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_VOID* prvCpssAvlFirstDataGet
(
    IN  PRV_CPSS_AVL_TREE_ID  treeId,
    IN  GT_VOID*              pData
);

/*******************************************************************************
* prvCpssAvlIteratorForNodeGet
*
* DESCRIPTION:
*       Returns the iteratorId for the the first data which is >= pData.
*       this iterator can be used to pass over the tree from a
*       specific location.
*
* INPUTS:
*       treeId - ID of the tree.
*       pData - A pointer to the data to be searched.
*
* OUTPUTS:
*       iteratorId  - Return the iterator ID of the given data.
*
* RETURNS:
*       The data represented by iteratorId if succeeded, or NULL otherwise.
*
* COMMENTS:
*       The returned iterator represents the first data which is >= pData.
*
*******************************************************************************/
GT_VOID* prvCpssAvlIteratorForNodeGet
(
    IN  PRV_CPSS_AVL_TREE_ID                   treeId,
    IN  GT_VOID*                               pData,
    OUT PRV_CPSS_AVL_TREE_ITERATOR_ID*         pIteratorId
);

/*******************************************************************************
* prvCpssAvlGetNext
*
* DESCRIPTION:
*       This function is used for iterations over the AVL tree. The function return
*       the iterator ID of the next item of the data represented by the given
*       iterator ID.
*
* INPUTS:
*       treeId - ID of the tree.
*       pData - A pointer to the data to be searched.
*       iteratorId - the iterator ID of the data the next item will be searched.
*
* OUTPUTS:
*       iteratorId  - Return the iterator ID of the given data.
*
* RETURNS:
*       A pointer to the next node data if exists, NULL otherwise.
*
* COMMENTS:
*       1.  If (*iteratorId == NULL): the first node in the tree is returned.
*       2.  The new value of iterator Id should be sent in the next call to
*           this function to get the next data.
*
*******************************************************************************/
GT_VOID* prvCpssAvlGetNext
(
    IN    PRV_CPSS_AVL_TREE_ID                 treeId,
    INOUT PRV_CPSS_AVL_TREE_ITERATOR_ID        *pIteratorId
);

/******************************************************************************/
/*                       Fast iteration functions                             */
/******************************************************************************/

/* this array contais 64 bits                    */
/* - 6  first bits for current deepness          */
/* - 58 last bits for path: 0 - left, 1 - right  */
typedef GT_U8 PRV_CPSS_AVL_TREE_PATH[8];

/**
* @enum PRV_CPSS_AVL_TREE_FIND_ENT
 *
 * @brief Find request types for find function.
*/
typedef enum{

    /** find equal */
    PRV_CPSS_AVL_TREE_FIND_EQUAL_E,

    /** find maximal less or equal, */
    PRV_CPSS_AVL_TREE_FIND_MAX_LESS_OR_EQUAL_E,

    /** find minimal greater or equal, */
    PRV_CPSS_AVL_TREE_FIND_MIN_GREATER_OR_EQUAL_E

} PRV_CPSS_AVL_TREE_FIND_ENT;

/**
* @enum PRV_CPSS_AVL_TREE_SEEK_ENT
 *
 * @brief Seek request types for seek function.
*/
typedef enum{

    /** seek node by path exactly, */
    PRV_CPSS_AVL_TREE_SEEK_THIS_E,

    /** seek next node, */
    PRV_CPSS_AVL_TREE_SEEK_NEXT_E,

    /** seek previous node, */
    PRV_CPSS_AVL_TREE_SEEK_PREVIOUS_E,

    /** seek first node, */
    PRV_CPSS_AVL_TREE_SEEK_FIRST_E,

    /** seek last node, */
    PRV_CPSS_AVL_TREE_SEEK_LAST_E

} PRV_CPSS_AVL_TREE_SEEK_ENT;

/**
* @internal prvCpssAvlPathFind function
* @endinternal
*
* @brief   Find node and return path to it.
*
* @param[in] treeId                   - ID of the tree.
* @param[in] findType                 - type of found (see enum).
* @param[in] pData                    - A pointer to the data to be searched.
*
* @param[out] path                     - The  to found node. It is array (i.e. passed as pointer)
*                                      pFoundDataPtr - (pointer to) found value.
*
* @retval GT_TRUE                  - found, GT_FALSE - not found.
*/
GT_BOOL prvCpssAvlPathFind
(
    IN  PRV_CPSS_AVL_TREE_ID       treeId,
    IN  PRV_CPSS_AVL_TREE_FIND_ENT findType,
    IN  GT_VOID*                   pData,
    OUT PRV_CPSS_AVL_TREE_PATH     path,
    OUT GT_VOID**                  pFoundDataPtr
);

/**
* @internal prvCpssAvlPathSeek function
* @endinternal
*
* @brief   Seek node relative from given node.
*
* @param[in] treeId                   - ID of the tree.
* @param[in] seekType                 - type of seek (see enum).
* @param[in,out] path                     - The start  node. It is array (i.e. passed as pointer)
* @param[in,out] path                     - The  to seeked node. It is array (i.e. passed as pointer)
*                                      pSeekedDataPtr - (pointer to) seeked value.
*
* @retval GT_TRUE                  - seeked, GT_FALSE - not seeked.
*/
GT_BOOL prvCpssAvlPathSeek
(
    IN    PRV_CPSS_AVL_TREE_ID       treeId,
    IN    PRV_CPSS_AVL_TREE_SEEK_ENT seekType,
    INOUT PRV_CPSS_AVL_TREE_PATH     path,
    OUT   GT_VOID**                  pSeekedDataPtr
);

/**
* @internal prvCpssAvlTreeIsOrderCorrect function
* @endinternal
*
* @brief   Check whether the tree data poiner by nodes ascending according
*         the given compare function
* @param[in] treeId                   - ID of the tree.
*
* @retval GT_TRUE                  - correct, GT_FALSE - incorrect.
*/
GT_BOOL prvCpssAvlTreeIsOrderCorrect
(
    IN    PRV_CPSS_AVL_TREE_ID       treeId
);

/******************************************************************************/
/*                       Generic iteration function                           */
/******************************************************************************/
/**
* @struct PRV_CPSS_AVL_TREE_ITERATOR_STC
 *
 * @brief This structure AVL Tree generic iterator.
*/
typedef struct{

    /** Tree position path. */
    PRV_CPSS_AVL_TREE_PATH path;

    /** tree Id. */
    PRV_CPSS_AVL_TREE_ID treeId;

    /** @brief default Seek Type.
     *  Default seek done when both doFind and doSeek are GT_FALSE.
     */
    PRV_CPSS_AVL_TREE_SEEK_ENT defaultSeekType;

    /** @brief if GT_TRUE
     *  Cleared by generic iterator function.
     */
    GT_BOOL doFind;

    /** @brief if GT_TRUE
     *  Cleared by generic iterator function.
     *  bookmarkPtr    - (pointer to) find data.
     */
    GT_BOOL doSeek;

    GT_BOOL doDefaultSeek;

    GT_VOID *bookmarkPtr;

    /** find type. */
    PRV_CPSS_AVL_TREE_FIND_ENT findType;

    /** @brief seek type.
     *  Comments:
     */
    PRV_CPSS_AVL_TREE_SEEK_ENT seekType;

} PRV_CPSS_AVL_TREE_ITERATOR_STC;

/* typical iterator reposition after find */
#define PRV_CPSS_AVL_TREE_ITERATOR_RESEEK_MAC(_iter)                     \
    {                                                                    \
        _iter.doDefaultSeek = GT_TRUE;                                   \
    }

/* typical iterator reposition by find */
#define PRV_CPSS_AVL_TREE_ITERATOR_FIND_MAC(_iter, _data, _find_type)    \
    {                                                                    \
        _iter.doFind = GT_TRUE;                                          \
        _iter.bookmarkPtr = _data;                                       \
        _iter.findType = _find_type;                                     \
    }

/* typical iterator initialization */
#define PRV_CPSS_AVL_TREE_ITERATOR_INIT_MAC(_iter, _tree, _ascending)    \
    {                                                                    \
        _iter.treeId = _tree;                                            \
        _iter.doSeek = GT_TRUE;                                          \
        _iter.doFind = GT_FALSE;                                         \
        _iter.doDefaultSeek = GT_FALSE;                                  \
        if (_ascending != GT_FALSE)                                      \
        {                                                                \
            _iter.seekType = PRV_CPSS_AVL_TREE_SEEK_FIRST_E;             \
            _iter.defaultSeekType = PRV_CPSS_AVL_TREE_SEEK_NEXT_E;       \
        }                                                                \
        else                                                             \
        {                                                                \
            _iter.seekType = PRV_CPSS_AVL_TREE_SEEK_LAST_E;              \
            _iter.defaultSeekType = PRV_CPSS_AVL_TREE_SEEK_PREVIOUS_E;   \
        }                                                                \
    }

/**
* @internal prvCpssAvlIterationDo function
* @endinternal
*
* @brief   Generic iretation in AVL Tree passing loop.
*         Serves to avoid additional finds/reseeks in loop body.
* @param[in,out] iteratorPtr              - (pointer to)iterator structure.
* @param[in,out] iteratorPtr              - (pointer to) updated iterator
*                                      dataPtr       - (pointer to) found/seeked value.
*
* @retval GT_TRUE                  - found/seeked, GT_FALSE - end of iteration reached.
*/
GT_BOOL prvCpssAvlIterationDo
(
    INOUT PRV_CPSS_AVL_TREE_ITERATOR_STC*   iteratorPtr,
    OUT   GT_VOID**                         dataPtr
);

#ifdef __cplusplus
}
#endif

#endif /* __prvCpssAvlTree_h */



