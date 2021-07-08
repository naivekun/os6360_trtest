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
* @file mv_hws_memmove.c
*
* @brief memmove() implementation
*
*
* @version   1
********************************************************************************
*/

typedef unsigned long GT_UINTPTR;

void *hws_memmove(void *dest, void *src, unsigned len)
{
    char *dstp = (char*) dest;
    const char *srcp = (const char*) src;

    if (len == 0 || dest == src)  /* nothing to do */
        return dest;

    if ((GT_UINTPTR)dest < (GT_UINTPTR)src)
    {
        while (len--)
            *dstp++ = *srcp++;
    }
    else
    {
        /* backward copy */
        srcp += len;
        dstp += len;
        while (len--)
            *(--dstp) = *(--srcp);
    }

    return dest;
}

