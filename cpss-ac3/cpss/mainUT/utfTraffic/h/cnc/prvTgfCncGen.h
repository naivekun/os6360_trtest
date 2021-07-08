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
* @file prvTgfCncGen.h
*
* @brief Centralized Counters (Cnc)
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfCncGen_h
#define __prvTgfCncGen_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfPclCncSizeGet function
* @endinternal
*
* @brief   Get CNC counters number info
*
* @param[out] cncBlockCntrsPtr         - pointer to number of CNC counters in a block
* @param[out] cncBlocksPtr             - pointer to number of CNC blocks
*                                       None
*/
GT_VOID prvTgfPclCncSizeGet
(
    OUT GT_U32          *cncBlockCntrsPtr,
    OUT GT_U32          *cncBlocksPtr
);

/**
* @internal prvTgfPclCncClientGet function
* @endinternal
*
* @brief   Get CNC client for a block
*
* @param[in] cncBlocks                - number of CNC blocks
* @param[in] blockIdx                 - CNC block's index
*                                       None
*/
PRV_TGF_CNC_CLIENT_ENT prvTgfPclCncClientGet
(
    IN GT_U32          cncBlocks,
    IN GT_U32          blockIdx
);

/**
* @internal prvTgfCncGenEgrVlanTrafficGenerateAndCheck function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet:
*         Check counters.
* @param[in] droppedPacket            - GT_TRUE   - dropped Packet
*                                      GT_FALSE  - passed Packet
*                                       None
*/
GT_VOID prvTgfCncGenEgrVlanTrafficGenerateAndCheck
(
    GT_BOOL droppedPacket
);

/**
* @internal prvTgfCncGenEgrVlanCfgRestore function
* @endinternal
*
* @brief   Restore Configuration.
*/
GT_VOID prvTgfCncGenEgrVlanCfgRestore
(
    GT_VOID
);

/**
* @internal prvTgfCncGenTxqTrafficGenerateAndCheck function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet:
*         Check counters.
* @param[in] droppedPacket            - GT_TRUE   - dropped Packet
*                                      GT_FALSE  - passed Packet
* @param[in] cnMode                   - GT_TRUE   - CN messages count mode
*                                      GT_FALSE  - Tail Drop mode
*                                       None
*/
GT_VOID prvTgfCncGenTxqTrafficGenerateAndCheck
(
    GT_BOOL droppedPacket,
    GT_BOOL cnMode
);

/**
* @internal prvTgfCncGenTxqCfgRestore function
* @endinternal
*
* @brief   Restore Configuration.
*
* @param[in] droppedPacket            - GT_TRUE   - dropped Packet
*                                      GT_FALSE  - passed Packet
* @param[in] cnMode                   - GT_TRUE   - CN messages count mode
*                                      GT_FALSE  - Tail Drop mode
*                                       None
*/
GT_VOID prvTgfCncGenTxqCfgRestore
(
    GT_BOOL droppedPacket,
    GT_BOOL cnMode
);

/**
* @internal prvTgfCncGenIPclTrafficGenerateAndCheck function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet:
*         Check counters.
* @param[in] useLookup1               - GT_FALSE = lookup0, GT_TRUE - lookup1
*                                       None
*/
GT_VOID prvTgfCncGenIPclTrafficGenerateAndCheck
(
    GT_BOOL useLookup1
);

/**
* @internal prvTgfCncGenIPclCfgRestore function
* @endinternal
*
* @brief   Restore Configuration.
*
* @param[in] useLookup1               - GT_FALSE = lookup0, GT_TRUE - lookup1
*                                       None
*/
GT_VOID prvTgfCncGenIPclCfgRestore
(
    GT_BOOL useLookup1
);

/**
* @internal prvTgfCncGenEPclTrafficGenerateAndCheck function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet:
*         Check counters.
*/
GT_VOID prvTgfCncGenEPclTrafficGenerateAndCheck
(
    GT_VOID
);

/**
* @internal prvTgfCncGenEPclCfgRestore function
* @endinternal
*
* @brief   Restore Configuration.
*/
GT_VOID prvTgfCncGenEPclCfgRestore
(
    GT_VOID
);

/**
* @internal prvTgfCncGenTtiTrafficGenerateAndCheck function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet:
*         Check counters.
*/
GT_VOID prvTgfCncGenTtiTrafficGenerateAndCheck
(
    GT_VOID
);

/**
* @internal prvTgfCncGenTtiCfgRestore function
* @endinternal
*
* @brief   Restore Configuration.
*/
GT_VOID prvTgfCncGenTtiCfgRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfCncGen_h */


