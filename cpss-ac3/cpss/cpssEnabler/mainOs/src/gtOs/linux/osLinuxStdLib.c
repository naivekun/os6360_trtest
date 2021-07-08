/**
********************************************************************************
* @file osLinuxStdLib.c
*
* @brief Operating System wrapper. stdlib functions
*
* @version   1
********************************************************************************
*/
/*******************************************************************************
* osLinuxStdLib.c
*
* DESCRIPTION:
*       Operating System wrapper. stdlib functions
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/

/************* Includes *******************************************************/

#include <gtOs/gtOsStdLib.h>

#include <stdlib.h>

/************* Functions ******************************************************/

/**
* @internal osQsort function
* @endinternal
*
* @brief   Sort an array
*
* @param[in] array                    - Address of first element in array
* @param[in] nItems                   - Number of items in array
* @param[in] itemSize                 - Size of one memner
* @param[in] comparFunc               - Function which compares two items
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osQsort
(
    IN  GT_VOID_PTR             array,
    IN  GT_U32                  nItems,
    IN  GT_U32                  itemSize,
    IN  COMPARE_ITEMS_FUNC_PTR  comparFunc
)
{
    qsort(array, nItems, itemSize, comparFunc);
    return GT_OK;
}

/**
* @internal osBsearch function
* @endinternal
*
* @brief   binary search of a sorted array
*
* @param[in] key                      -  to search
* @param[in] array                    - Address of first element in array
* @param[in] nItems                   - Number of items in array
* @param[in] itemSize                 - Size of one memner
* @param[in] comparFunc               - Function which compares two items
*
* @param[out] result                   - pointer to item found or NULL
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osBsearch
(
    IN  const void *            key,
    IN  const void *            array,
    IN  GT_U32                  nItems,
    IN  GT_U32                  itemSize,
    IN  COMPARE_ITEMS_FUNC_PTR  comparFunc,
    OUT GT_VOID_PTR             *result
)
{
    void *ret;

    ret = bsearch(key, array, nItems, itemSize, comparFunc);

    if (result != NULL)
        *result = ret;

    return (ret == NULL) ? GT_FAIL : GT_OK;
}

