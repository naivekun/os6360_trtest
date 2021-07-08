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
* @file prvTgfPclXCat.h
*
* @brief Second Lookup match
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfPclXCath
#define __prvTgfPclXCath

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#ifdef CHX_FAMILY


/**
* @internal prvTgfPclXCatStdUdbTest function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfPclXCatStdUdbTest
(
    GT_VOID
);

/**
* @internal prvTgfPclXCatDummy function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfPclXCatDummy
(
    GT_VOID
);

#endif /* CHX_FAMILY */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfPclXCath */


