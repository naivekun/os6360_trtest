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
* @file prvTgfPolicerMirroring.h
*
* @brief Policer Packet cmd Mirroring tests
*
* @version   10
********************************************************************************
*/
#ifndef __prvTgfPolicerMirroringh
#define __prvTgfPolicerMirroringh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfPolicerGen.h>

GT_VOID prvTgfPolicerMirroringCfgSet
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT    stage
);

GT_VOID prvTgfPolicerMirroringTrafficGenerate
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT    stage
);

GT_VOID prvTgfPolicerMirroringCfgRestore
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT    stage
);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
