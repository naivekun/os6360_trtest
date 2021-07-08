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
* @file prvTgfExactMatchTtiPclFullPath.h
*
* @brief Test Exact Match Expanded Action functionality with TTI and PCL
*        Configuration
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfExactMatchTtiPclFullPathh
#define __prvTgfExactMatchTtiPclFullPathh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfExactMatchGen.h>


/**
* @internal prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigSet function
* @endinternal
*
* @brief   Set TTI Generic configuration not related to Exact Match
*/
GT_VOID prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigSet
(
    GT_VOID
);
/**
* @internal prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigRestore function
* @endinternal
*
* @brief   Restore TTI Generic configuration not related to Exact Match
*/
GT_VOID prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigRestore
(
    GT_VOID
);
/**
* @internal prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigSet function
* @endinternal
*
* @brief   Set PCL Generic configuration not related to Exact Match
*/
GT_VOID prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigSet
(
    GT_VOID
);
/**
* @internal prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigRestore function
* @endinternal
*
* @brief   Restore PCL Generic configuration not related to Exact Match
*/
GT_VOID prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigRestore
(
    GT_VOID
);
/**
* @internal prvTgfExactMatchTtiPclFullPathInvalidateRule function
* @endinternal
*
* @brief   Invalidate PCL Rule
*/
GT_VOID prvTgfExactMatchTtiPclFullPathInvalidateRule
(
    GT_VOID
);
/**
* @internal GT_VOID prvTgfExactMatchTtiPclFullPathConfigSet function
* @endinternal
*
* @brief   Set PCL test configuration related to Exact Match Expanded Action
*
* @param[in] pktCmd   - packet command to assign in Exact Match Action
*
*/
GT_VOID prvTgfExactMatchTtiPclFullPathConfigSet
(
     CPSS_PACKET_CMD_ENT pktCmd
);

/**
* @internal GT_VOID prvTgfExactMatchTtiPclFullPathConfigSet function
* @endinternal
*
* @brief   Set PCL test configuration related to Exact Match Expanded Action
*          and Reduced Action
*
* @param[in] pktCmd   - packet command to assign in Exact Match Action
*
*/
GT_VOID prvTgfExactMatchTtiPclFullPathReducedConfigSet
(
     CPSS_PACKET_CMD_ENT pktCmd
);
/**
* @internal prvTgfExactMatchTtiPclFullPathNewKeySizeConfigSet function
* @endinternal
*
* @brief   Set Exact Match keyParams.keySize to be != 0
*
*/
GT_VOID prvTgfExactMatchTtiPclFullPathNewKeySizeConfigSet
(
     GT_VOID
);
/**
* @internal prvTgfExactMatchTtiPclFullPathNewKeyMaskConfigSet function
* @endinternal
*
* @brief   Set Exact Match keyParams.keyMask to be != 0xFF
*
*/
GT_VOID prvTgfExactMatchTtiPclFullPathNewKeyMaskConfigSet
(
     GT_VOID
);
/**
* @internal prvTgfExactMatchTtiPclFullPathUdb47ConfigSet function
* @endinternal
*
* @brief   Set Exact Match keyParams.keySize to be PRV_TGF_EXACT_MATCH_KEY_SIZE_47B_E
*
*/
GT_VOID prvTgfExactMatchTtiPclFullPathUdb47ConfigSet
(
     GT_VOID
);
/**
* @internal prvTgfExactMatchTtiPclFullPathActivitySet function
* @endinternal
*
* @brief   Set Expected Activity
*
*/
GT_VOID prvTgfExactMatchTtiPclFullPathActivitySet
(
     GT_BOOL    expectedActivityHit
);
/**
* @internal GT_VOID prvTgfExactMatchTtiPclFullPathConfigRestore function
* @endinternal
*
* @brief   Restore PCL test configuration related to Exact Match Expanded Action
*/
GT_VOID prvTgfExactMatchTtiPclFullPathConfigRestore
(
    GT_VOID
);
/**
* @internal prvTgfExactMatchTtiPclFullPathInvalidateEmEntry function
* @endinternal
*
* @brief   Invalidate Exact Match Entry
*/
GT_VOID prvTgfExactMatchTtiPclFullPathInvalidateEmEntry
(
    GT_VOID
);
/**
* @internal prvTgfExactMatchTtiPclFullPathInvalidateSecondEmEntry function
* @endinternal
*
* @brief   Invalidate Second Exact Match Entry
*/
GT_VOID prvTgfExactMatchTtiPclFullPathInvalidateSecondEmEntry
(
    GT_VOID
);
/*
* @internal prvTgfExactMatchTtiPclFullPathTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic, expect traffic to be trapped/dropped/forwarded
*
* @param[in] expectNoTraffic   - GT_TRUE:  No traffic
*                                GT_FALSE: Expect Traffic
* @param[in] expectFdbForwarding - GT_TRUE:  FBB forwarding
*                                GT_FALSE: NO FBB forwarding
*/
GT_VOID prvTgfExactMatchTtiPclFullPathTrafficGenerate
(
    GT_BOOL     expectNoTraffic,
    GT_BOOL     expectFdbForwarding
);
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
