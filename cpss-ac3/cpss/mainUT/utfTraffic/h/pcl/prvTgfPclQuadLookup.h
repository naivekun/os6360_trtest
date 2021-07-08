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
* @file prvTgfPclQuadLookup.h
*
* @brief PCL Quad lookup test header file
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfPclQuadLookup
#define __prvTgfPclQuadLookup

#include <trafficEngine/tgfTrafficGenerator.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfPclIngressQuadLookupTest function
* @endinternal
*
* @brief   IPCL Quad lookup test
*/
GT_VOID prvTgfPclIngressQuadLookupTest
(
    GT_VOID
);

/**
* @internal prvTgfPclIngressQuadLookupTest_virtTcam function
* @endinternal
*
* @brief   IPCL Quad lookup test with Virtual TCAM
*/
GT_VOID prvTgfPclIngressQuadLookupTest_virtTcam
(
    GT_VOID
);

/**
* @internal prvTgfPclIngressQuadLookupTest_virtTcamPriority function
* @endinternal
*
* @brief   IPCL Quad lookup test with Virtual TCAM add rule by prioirity
*/
GT_VOID prvTgfPclIngressQuadLookupTest_virtTcamPriority
(
    GT_VOID
);


/**
* @internal prvTgfPclIngressQuadLookupTestWithInvalid function
* @endinternal
*
* @brief   IPCL Quad lookup test with invalid rules
*/
GT_VOID prvTgfPclIngressQuadLookupTestWithInvalid
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfPclQuadLookup */

