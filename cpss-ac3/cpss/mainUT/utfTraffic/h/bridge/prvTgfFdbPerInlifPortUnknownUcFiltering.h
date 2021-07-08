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
* @file prvTgfFdbPerInlifPortUnknownUcFiltering.h
*
* @brief Verify UC packets filtering thru InLIF port commandset.
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfFdbPerInlifPortUnknownUcFilteringh
#define __prvTgfFdbPerInlifPortUnknownUcFilteringh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfFdbPerInlifPortUnknownUcFilteringConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfFdbPerInlifPortUnknownUcFilteringConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfFdbPerInlifPortUnknownUcFilteringTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfFdbPerInlifPortUnknownUcFilteringTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfFdbPerInlifPortUnknownUcFilteringConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbPerInlifPortUnknownUcFilteringConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfFdbPerInlifPortUnknownUcFilteringh */


