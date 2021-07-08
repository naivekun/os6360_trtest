/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfTablesFillingLtt.h
*
* DESCRIPTION:
*       High Level IP table filling test
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/
#ifndef __prvTgfTablesFillingLpmh
#define __prvTgfTablesFillingLpmh

#include <common/tgfIpGen.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfTablesFillingLpmConfigurationSet function
* @endinternal
*
* @brief   Set common configuration
*/
GT_VOID prvTgfTablesFillingLpmConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfTablesFillingLpmTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] protocol                 - type of IP stack used
*                                      incValue - value for increment ip addresses
*                                      if 0 the test generate random prefixes
* @param[in] numLoops                 - number of loops of the test to discover
*                                      a memory leak and so on
*                                       None
*/
GT_VOID prvTgfTablesFillingLpmTrafficGenerate
(
    IN CPSS_IP_PROTOCOL_STACK_ENT protocol,
    IN GT_U32                     incVulue,
    IN GT_U32                     numLoops
);

/**
* @internal prvTgfTablesFillingLpmConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfTablesFillingLpmConfigurationRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfTablesFillingLpmh */


