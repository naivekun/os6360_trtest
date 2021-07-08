/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfCommonList.c
*
* DESCRIPTION:
*       Implementation of double-linked list functions.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <extUtils/tgf/tgfCommonList.h>



/**
* @internal prvTgfListDelItem function
* @endinternal
*
*/
GT_VOID prvTgfListDelItem
(
    INOUT PRV_TGF_LIST_HEAD_PTR       listPtr,
    IN    const PRV_TGF_LIST_ITEM_PTR itemPtr
)
{
    if (NULL == itemPtr)
    {
        return;
    };

    (NULL != itemPtr->prevPtr)
        ? (itemPtr->prevPtr->nextPtr = itemPtr->nextPtr)
        : (listPtr->firstPtr      = itemPtr->nextPtr);

    (NULL != itemPtr->nextPtr)
        ? (itemPtr->nextPtr->prevPtr = itemPtr->prevPtr)
        : (listPtr->lastPtr       = itemPtr->prevPtr);

    cpssOsFree(itemPtr);
    listPtr->count--;
};


/**
* @internal prvTgfListDelAll function
* @endinternal
*
*/
GT_VOID prvTgfListDelAll
(
    INOUT PRV_TGF_LIST_HEAD_PTR listPtr
)
{
    if (NULL != listPtr)
    {
        while (NULL != listPtr->firstPtr)
        {
            prvTgfListDelItem(listPtr, listPtr->firstPtr);
        };
    };
};


/**
* @internal prvTgfListAddAfter function
* @endinternal
*
*
* @note Function doesn't check pointer "listPtr" on NULL.
*
*/
PRV_TGF_LIST_ITEM_PTR prvTgfListAddAfter
(
  INOUT PRV_TGF_LIST_HEAD_PTR   listPtr,
  INOUT PRV_TGF_LIST_ITEM_PTR   prevItemPtr
)
{
    PRV_TGF_LIST_ITEM_PTR itemPtr;

    if (listPtr->count >= listPtr->max)
    {
        return NULL;
    };
    itemPtr = (PRV_TGF_LIST_ITEM_PTR) cpssOsMalloc(listPtr->itemSize);
    if (NULL == itemPtr) /*cpssOsMalloc error*/
    {
        return NULL;
    };
    itemPtr->prevPtr = prevItemPtr;

    if (NULL == prevItemPtr)   /* add item at the begin of the list*/
    {
        itemPtr->nextPtr = listPtr->firstPtr;
        if ( NULL != listPtr->firstPtr)
        {
            listPtr->firstPtr->prevPtr = itemPtr;
        }
        listPtr->firstPtr = itemPtr;
    }
    else
    {
        itemPtr->nextPtr = prevItemPtr->nextPtr;
        prevItemPtr->nextPtr = itemPtr;
    };

    if (prevItemPtr == listPtr->lastPtr) /*item was added to the end of the list*/
    {
        listPtr->lastPtr = itemPtr;
    };

    listPtr->count++;
    return itemPtr;
};


/**
* @internal prvTgfListAppend function
* @endinternal
*
*/
PRV_TGF_LIST_ITEM_PTR prvTgfListAppend
(
  INOUT PRV_TGF_LIST_HEAD_PTR listPtr
)
{
    return prvTgfListAddAfter(listPtr, listPtr->lastPtr);
};

