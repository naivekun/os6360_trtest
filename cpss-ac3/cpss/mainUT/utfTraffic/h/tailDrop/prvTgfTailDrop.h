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
* @file prvTgfTailDrop.h
*
* @brief Enhanced UTs for CPSS Tail Drop
*
* @version   3
********************************************************************************
*/
#ifndef __prvTgfTailDroph
#define __prvTgfTailDroph

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @enum PRV_TGF_DROP_CONFIG_SET_ENT
 *
 * @brief There are several Drop Config Sets of parameters for the test.
 * This enumeration specifies which Drop Config Set should be
 * given for the test.
*/
typedef enum{

    PRV_TGF_DROP_CONFIG_SET_0_E = 0,

    PRV_TGF_DROP_CONFIG_SET_1_E,

    PRV_TGF_DROP_CONFIG_SET_2_E,

    PRV_TGF_DROP_CONFIG_SET_LAST_E

} PRV_TGF_DROP_CONFIG_SET_ENT;

/**
* @internal prvTgfTailDropConfigurationSet function
* @endinternal
*
* @brief   Set common configuration
*
* @param[in] isVariousPorts           - GT_TRUE  creates FDB entries with different MAC DA
*                                      GT_FALSE creates FDB entries with  the same MAC DA
* @param[in] numEntries               - number of QoS, FDB and PCL entries to create.
* @param[in] dropPrecedence           - drop precedence for cos profile.
*                                       None
*/
GT_VOID prvTgfTailDropConfigurationSet
(
    IN GT_BOOL           isVariousPorts,
    IN GT_U32            numEntries,
    IN CPSS_DP_LEVEL_ENT dropPrecedence
);

/**
* @internal prvTgfTailDropProfileConfigurationSet function
* @endinternal
*
* @brief   Set Drop Profile configuration
*
* @param[in] isVariousPorts           - GT_TRUE  creates FDB entries with different MAC DA
*                                      GT_FALSE creates FDB entries with  the same MAC DA
*                                       None
*/
GT_VOID prvTgfTailDropProfileConfigurationSet
(
    IN GT_BOOL isVariousPorts
);

/**
* @internal prvTgfTailDropWrtdConfigurationSet function
* @endinternal
*
* @brief   Set Drop Profile configuration
*
* @param[in] configSetIdx             - index of set of parameters for using in the test
*                                       None
*/
GT_VOID prvTgfTailDropWrtdConfigurationSet
(
    IN PRV_TGF_DROP_CONFIG_SET_ENT configSetIdx
);

/**
* @internal prvTgfTailDropSharingConfigurationSet function
* @endinternal
*
* @brief   Set Drop Profile configuration
*
* @param[in] enableMode               - Drop Precedence (DP) enable mode to use the shared pool
*                                       None
*/
GT_VOID prvTgfTailDropSharingConfigurationSet
(
    IN PRV_TGF_PORT_TX_SHARED_DP_MODE_ENT enableMode
);

/**
* @internal prvTgfTailDropSharedPoolsConfigurationSet function
* @endinternal
*
* @brief   Set Drop Profile configuration
*/
GT_VOID prvTgfTailDropSharedPoolsConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfTailDropVariousPortsTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] burstCount               - number of packets in one burst to transmit
*                                       None
*/
GT_VOID prvTgfTailDropVariousPortsTrafficGenerate
(
    IN GT_U32 burstCount
);

/**
* @internal prvTgfTailDropOnePortTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] fdbIdxMin                - minimum for an iterator fdbIdx
*                                      FDB index fdbIdx must be decreased from
*                                      PRV_TGF_DST_COUNT_CNS down to fdbIdxMin because different
*                                      tests must check different quantity of destinations
* @param[in] burstCount               - number of packets in one burst to transmit
*                                       None
*/
GT_VOID prvTgfTailDropOnePortTrafficGenerate
(
    IN GT_U32 fdbIdxMin,
    IN GT_U32 burstCount
);

/**
* @internal prvTgfTailDropSharedPoolsTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] burstCount               - number of packets in one burst to transmit
*                                       None
*/
GT_VOID prvTgfTailDropSharedPoolsTrafficGenerate
(
    IN GT_U32 burstCount
);

/**
* @internal prvTgfTailDropWrtdTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] isWrtdEnabled            - GT_TRUE for enable WRTD Threshold
* @param[in] burstCount               - number of packets in one burst to transmit
* @param[in] numExpect                - number of expected packets on receive port
* @param[in] configSetIdx             - index of set of parameters for using in the test
*                                       None
*/
GT_VOID prvTgfTailDropWrtdTrafficGenerate
(
    IN GT_BOOL                     isWrtdEnabled,
    IN GT_U32                      burstCount,
    IN GT_U32                      numExpect,
    IN PRV_TGF_DROP_CONFIG_SET_ENT configSetIdx
);

/**
* @internal prvTgfTailDropMcConfigurationSet function
* @endinternal
*
* @brief   Set configuration for Mcast
*/
GT_VOID prvTgfTailDropMcConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfTailDropMcTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] burstCount1              - number of packets in first burst to transmit
* @param[in] burstCount2              - number of packets in second burst to transmit
*                                      numExpectMc     - number of expected Mc buffers
* @param[in] numExpectGlobal          - number of expected global buffers
*                                       None
*/
GT_VOID prvTgfTailDropMcTrafficGenerate
(
    IN GT_U32 burstCount1,
    IN GT_U32 burstCount2,
    IN GT_U32 numExpectMcast,
    IN GT_U32 numExpectGlobal
);

/**
* @internal prvTgfTailDropConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @param[in] numEntries               - number of QoS, FDB and PCL entries to restore.
*                                       None
*/
GT_VOID prvTgfTailDropConfigurationRestore
(
    IN GT_U32 numEntries
);


/**
* @internal tgfTailDropSeparateUcMcCountPerQueueConfigurationSet function
* @endinternal
*
* @brief   Set configuration for Mcast
*/
GT_VOID tgfTailDropSeparateUcMcCountPerQueueConfigurationSet
(
    IN  GT_BOOL isFirstCall,
    IN  GT_BOOL sharedDisable
);

/**
* @internal tgfTailDropSeparateUcMcCountPerQueueTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] burstCount1              - number of packets in first burst to transmit
* @param[in] burstCount2              - number of packets in second burst to transmit
* @param[in] numExpectPackets         - number of expected buffers
*                                       None
*/
GT_VOID tgfTailDropSeparateUcMcCountPerQueueTrafficGenerate
(
    IN GT_U32 burstCount1,
    IN GT_U32 burstCount2,
    IN GT_U32 numExpectPackets
);


/**
* @internal tgfTailDropSeparateUcMcCountPerQueueConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID tgfTailDropSeparateUcMcCountPerQueueConfigurationRestore
(
    GT_VOID
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfTailDroph */


