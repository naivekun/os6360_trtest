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
* @file prvUtfSuitsStart.h
*
* @brief Internal header which is used for suits registration.
*
* @version   1.7
********************************************************************************
*/
#ifndef __prvUtfSuitsStarth
#define __prvUtfSuitsStarth

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef UTF_Suits_Forward_Declaration
#undef UTF_Suits_Forward_Declaration
#undef UTF_MAIN_BEGIN_SUITS_MAC
#undef UTF_MAIN_DECLARE_SUIT_MAC
#undef UTF_MAIN_END_SUITS_MAC
#undef UTF_MAIN_DECLARE_TEST_TYPE_MAC
#define UTF_Suits_Implementation
#else
#define UTF_Suits_Forward_Declaration
#endif

/* <add description here> */
#ifdef UTF_Suits_Implementation
#define UTF_MAIN_BEGIN_SUITS_MAC() GT_STATUS utfAllSuitsDeclare(GT_VOID) { GT_STATUS st = GT_OK;
#else
#define UTF_MAIN_BEGIN_SUITS_MAC()
#endif

/* <add description here> */
#ifdef UTF_Suits_Forward_Declaration
#define UTF_MAIN_DECLARE_SUIT_MAC(x) GT_STATUS utfSuit_##x(GT_VOID);
#else
#define UTF_MAIN_DECLARE_SUIT_MAC(x) st = utfSuit_##x(); if (GT_OK != st) return st;
#endif

/* <add description here> */
#ifdef UTF_Suits_Forward_Declaration
#define UTF_MAIN_DECLARE_TEST_TYPE_MAC(x) GT_VOID utfDeclareTestType(IN UTF_TEST_TYPE_ENT testType);
#else
#define UTF_MAIN_DECLARE_TEST_TYPE_MAC(x) utfDeclareTestType(x);
#endif

/* <add description here> */
#ifdef UTF_Suits_Implementation
#define UTF_MAIN_END_SUITS_MAC() return st; }
#else
#define UTF_MAIN_END_SUITS_MAC()
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvUtfSuitsStarth */

