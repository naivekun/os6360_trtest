/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfCommonList.h
*
* DESCRIPTION:
*       Double-linked list functions.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/
#ifndef __prvTgfCommonListh
#define __prvTgfCommonListh

#include <cpss/extServices/os/gtOs/gtGenTypes.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/* Typedefs */
/*
 * Typedef: struct PRV_TGF_LIST_ITEM_STC
 *
 * Description: -- item of double-linked list.
 *
 * Fields:
 *        nextPtr - pointer to a next item of list. If NULL item is last.
 *        prevPtr - pointer to a next item of list. If NULL item is first.
 * Comments:
 *        You can use struct-"descendants" of PRV_TGF_LIST_ITEM_STCT
 *        as list items. It means the first field of such struct is
 *        PRV_TGF_LIST_ITEM_STC  and some additional fields can be present.
 */
typedef struct PRV_TGF_LIST_ITEM_STCT
{
    struct PRV_TGF_LIST_ITEM_STCT *nextPtr;
    struct PRV_TGF_LIST_ITEM_STCT *prevPtr;
} PRV_TGF_LIST_ITEM_STC, *PRV_TGF_LIST_ITEM_PTR;


/**
* @struct *PRV_TGF_LIST_HEAD_PTR
 *
 * @brief -- list head information.
*/
typedef struct{

    /** pointer to last item of list. If NULL list is empty. */
    PRV_TGF_LIST_ITEM_PTR lastPtr;

    /** pointer to first item of list. If NULL list is empty. */
    PRV_TGF_LIST_ITEM_PTR firstPtr;

    /** @brief size of list item.
     *  itemSize - is not necessary equal sizeof(PRV_TGF_LIST_ITEM_STC).
     *  It can be more (if list item is descendant of
     *  PRV_TGF_LIST_ITEM_STC), but can't be less.
     */
    GT_U32 itemSize;

    /** count of list items. It can't exceed max field value. */
    GT_U32 count;

    /** @brief maximum amount of list items.
     *  Comments:
     */
    GT_U32 max;

} PRV_TGF_LIST_HEAD_STC, *PRV_TGF_LIST_HEAD_PTR;


    
/**
* @internal prvTgfListDelAll function
* @endinternal
*
*/
extern GT_VOID prvTgfListDelAll(INOUT  PRV_TGF_LIST_HEAD_PTR listPtr);


/**
* @internal prvTgfListAddAfter function
* @endinternal
*
*
* @note Function doesn't check pointer "listPtr" on NULL.
*       This should be done by a calling code.
*
*/
extern PRV_TGF_LIST_ITEM_PTR prvTgfListAddAfter
(
    INOUT PRV_TGF_LIST_HEAD_PTR listPtr,
    INOUT PRV_TGF_LIST_ITEM_PTR prevItemPtr
);


/**
* @internal prvTgfListAppend function
* @endinternal
*
*/
extern PRV_TGF_LIST_ITEM_PTR prvTgfListAppend
(
    INOUT PRV_TGF_LIST_HEAD_PTR listPtr
);



/**
* @internal prvTgfListDelItem function
* @endinternal
*
*/
extern GT_VOID prvTgfListDelItem(INOUT    PRV_TGF_LIST_HEAD_PTR listPtr,
                                 IN const PRV_TGF_LIST_ITEM_PTR itemPtr);

    
/**
* @internal prvTgfListInit function
* @endinternal
*
*/
extern GT_STATUS prvTgfListInit(INOUT PRV_TGF_LIST_HEAD_PTR listPtr,
                                IN const GT_U32             itemSize,
                                IN const GT_U32             max);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfCommonListh */

