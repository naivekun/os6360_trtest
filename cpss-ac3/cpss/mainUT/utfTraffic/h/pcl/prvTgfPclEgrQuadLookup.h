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
* @file prvTgfPclEgrQuadLookup.h
*
* @brief PCL Quad lookup test header file
*
* @version   2
********************************************************************************
*/
#ifndef __prvTgfPclEgrQuadLookup
#define __prvTgfPclEgrQuadLookup

#include <trafficEngine/tgfTrafficGenerator.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfPclEgressQuadLookupTest function
* @endinternal
*
* @brief   EPCL Quad lookup test
*/
GT_VOID prvTgfPclEgressQuadLookupTest
(
    GT_VOID
);

/**
* @internal prvTgfPclEgressQuadLookupTestWithInvalid function
* @endinternal
*
* @brief   EPCL Quad lookup test with invalid rules
*/
GT_VOID prvTgfPclEgressQuadLookupTestWithInvalid
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfPclEgrQuadLookup */

