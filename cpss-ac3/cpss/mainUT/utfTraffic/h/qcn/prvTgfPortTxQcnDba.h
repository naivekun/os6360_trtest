/*******************************************************************************
*              (c), Copyright 2018, Marvell International Ltd.                 *
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
* @file prvTgfPortTxQcnDba.h
*
* @brief "QCN DBA" enhanced UT for QCN Trigger APIs
*
* @version   1
********************************************************************************
*/

#ifndef __prvTgfPortTxQcnDbah
#define __prvTgfPortTxQcnDbah

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>

/**
* @internal tgfPortTxQcnQueueResourcesConfigure function
* @endinternal
*
* @brief Prepare or restore  test configuration
*
* @param[in] configSet    - store/restore configuration
*                           GT_TRUE  -- configure
*                           GT_FALSE -- restore
* @param[in] ingressPktHdr - inlude/exclude IngressPktHdr in QCN
*                            GT_TRUE  -- include
*                            GT_FALSE -- exclude
*/
GT_VOID tgfPortTxQcnQueueResourcesConfigure
(
    GT_BOOL configSet,
    GT_BOOL ingressPktHdr
);

/**
* @internal tgfPortTxQcnDbaQueueResourcesConfigure function
* @endinternal
*
* @brief Prepare or restore tgfPortTxQcnDbaQueueResourcesConfigure test configuration
*
* @param[in] configSet    - store/restore configuration
*                           GT_TRUE  -- configure
*                           GT_FALSE -- restore
*/
GT_VOID tgfPortTxQcnDbaQueueResourcesConfigure
(
    GT_BOOL configSet
);

/**
* @internal tgfPortTxQcnPortResourcesTrafficGenerate function
* @endinternal
*
* @brief Generate traffic fot tgfPortTxQcnPortResourcesTrafficGenerate test.
*
* @param[in] ingressPktHdr - inlude/exclude IngressPktHdr in QCN
*                            GT_TRUE  -- include
*                            GT_FALSE -- exclude
*/
GT_VOID tgfPortTxQcnPortResourcesTrafficGenerate
(
    GT_BOOL ingressPktHdr
);

/**
* @internal tgfPortTxQcnDbaPortResourcesTrafficGenerate function
* @endinternal
*
* @brief Generate traffic fot tgfPortTxQcnDbaPortResourcesTrafficGenerate test.
*
* @param[in] ingressPktHdr - inlude/exclude IngressPktHdr in QCN
*                            GT_TRUE  -- include
*                            GT_FALSE -- exclude
*/
GT_VOID tgfPortTxQcnDbaPortResourcesTrafficGenerate
(
    GT_BOOL ingressPktHdr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfPortTxQcnDbah */

