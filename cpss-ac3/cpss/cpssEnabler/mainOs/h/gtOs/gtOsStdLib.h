/**
********************************************************************************
* @file gtOsStdLib.h
*
* @brief Operating System wrapper. stdlib functions
*
* @version   1
********************************************************************************
*/
/*******************************************************************************
* gtOsStdLib.h
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

#ifndef __gtOsStdLibh
#define __gtOsStdLibh

#ifdef __cplusplus
extern "C" {
#endif

/************* Includes *******************************************************/

#include <gtOs/gtGenTypes.h>

/************* Typedefs *******************************************************/
/*
 * Typedef: COMPARE_ITEMS_FUNC_PTR
 *
 * Description: Comparison function type
 *
 * Inputs:
 *      a - pointer to one item
 *      b - pointer to another item
 *
 * Outputs:
 *      None
 *
 * Returns:
 *      Less than zero if a < b
 *      Zero if a == b
 *      greather than zero if a > b
 *
 */
typedef int(*COMPARE_ITEMS_FUNC_PTR)
(
    const void *a,
    const void *b
);

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
    IN  void *                  array,
    IN  GT_U32                  nItems,
    IN  GT_U32                  itemSize,
    IN  COMPARE_ITEMS_FUNC_PTR  comparFunc
);

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
);


#ifdef __cplusplus
}
#endif

#endif  /* __gtOsStdLibh */

