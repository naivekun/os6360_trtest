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
* @file prvTgfPortTxTailDropDba.h
*
* @brief "DBA Queue Resources Allocation" enhanced UT for Tx Tail Drop APIs
*
* @version   1
********************************************************************************
*/

#ifndef __prvTgfPortTxTailDropDbah
#define __prvTgfPortTxTailDropDbah

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>

/**
* @internal tgfPortTxTailDropDbaQueueResourcesConfigure function
* @endinternal
*
* @brief Prepare or restore tgfPortTxTailDropDbaQueueResources test configuration
*
* @param[in] configSet    - store/restore configuration
*                           GT_TRUE  -- configure
*                           GT_FALSE -- restore
*/
GT_VOID tgfPortTxTailDropDbaQueueResourcesConfigure
(
    GT_BOOL configSet
);

/**
* @internal tgfPortTxTailDropDbaQueueResourcesTrafficGenerate function
* @endinternal
*
* @brief Generate traffic fot tgfPortTxTailDropDbaQueueResources test.
*
*/
GT_VOID tgfPortTxTailDropDbaQueueResourcesTrafficGenerate
(
    GT_VOID
);

/**
* @internal tgfPortTxTailDropDbaPortResourcesConfigure function
* @endinternal
*
* @brief Prepare or restore tgfPortTxTailDropDbaPortResources test configuration
*
* @param[in] configSet    - store/restore configuration
*                           GT_TRUE  -- configure
*                           GT_FALSE -- restore
*/
GT_VOID tgfPortTxTailDropDbaPortResourcesConfigure
(
    GT_BOOL configSet
);

/**
* @internal tgfPortTxTailDropDbaPortResourcesTrafficGenerate function
* @endinternal
*
* @brief Generate traffic fot tgfPortTxTailDropDbaPortResources test.
*
*/
GT_VOID tgfPortTxTailDropDbaPortResourcesTrafficGenerate
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfPortTxTailDropDbah */