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
* @file prvTgfPclEgressMirroring.h
*
* @brief Pcl Egress Packet cmd Mirroring tests
*
* @version   10
********************************************************************************
*/
#ifndef __prvTgfPclEgressMirroringh
#define __prvTgfPclEgressMirroringh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfPclGen.h>

GT_VOID prvTgfPclEgressMirroringCfgSet
(
    GT_VOID
);

GT_VOID prvTgfPclEgressMirroringTrafficGenerate
(
    GT_VOID
);

GT_VOID prvTgfPclEgressMirroringCfgRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
