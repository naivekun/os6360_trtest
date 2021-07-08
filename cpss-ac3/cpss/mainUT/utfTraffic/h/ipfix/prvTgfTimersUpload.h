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
* @file prvTgfTimersUpload.h
*
* @brief Timers upload tests for IPFIX declarations
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfTimersUploadh
#define __prvTgfTimersUploadh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfIpfixTimersUploadAbsoluteTest function
* @endinternal
*
*/
GT_VOID prvTgfIpfixTimersUploadAbsoluteTest
(
    GT_VOID
);

/**
* @internal prvTgfIpfixTimersUploadIncrementalTest function
* @endinternal
*
*/
GT_VOID prvTgfIpfixTimersUploadIncrementalTest
(
    GT_VOID
);

/**
* @internal prvTgfIpfixTimersUploadRestoreByClear function
* @endinternal
*
*/
GT_VOID prvTgfIpfixTimersUploadRestoreByClear
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfTimersUploadh */

