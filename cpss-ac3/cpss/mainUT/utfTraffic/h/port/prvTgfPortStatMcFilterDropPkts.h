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
* @file prvTgfPortStatMcFilterDropPkts.h
*
* @brief Check filtered multicast drop counters
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfPortStatMcFilterDropPktsh
#define __prvTgfPortStatMcFilterDropPktsh

#include <cpssCommon/cpssPresteraDefs.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfPortStatMcFilterDropPktsConfigure function
* @endinternal
*
* @brief   Configure or restore test configuration
*
* @param[in] config - GT_TRUE  - configure
*                     GT_FALSE - restore
*
*/
GT_VOID prvTgfPortStatMcFilterDropPktsConfigure
(
    GT_BOOL config
);

/**
* @internal prvTgfPortStatMcFilterDropPktsGenerate function
* @endinternal
*
* @brief   Send test traffic and validate counters
*
*
*/
GT_VOID prvTgfPortStatMcFilterDropPktsGenerate
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfPortStatMcFilterDropPktsh */