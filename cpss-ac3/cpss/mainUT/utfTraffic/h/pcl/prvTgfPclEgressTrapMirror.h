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
* @file prvTgfPclEgressTrapMirror.h
*
* @brief Pcl Egress Packet cmd Trap/Mirror tests
*
* @version   10
********************************************************************************
*/
#ifndef __prvTgfPclEgressTrapMirrorh
#define __prvTgfPclEgressTrapMirrorh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfPclGen.h>

#define PRV_TGF_VLANID_CNS 5

#define PRV_TGF_PORT_COUNT_CNS 4

#define PRV_TGF_CPU_PORT_IDX_CNS 3

#define PRV_TGF_EPCL_RULE_INDEX_CNS 0

#define PRV_TGF_EPCL_SEND_PORT_INDEX_CNS 0

#define PRV_TGF_EPCL_RECEIVE_PORT_INDEX_CNS 1

GT_STATUS prvTgfFdbDefCfgSet
(
    GT_VOID
);

GT_STATUS prvTgfFdbDefCfgRestore
(
    GT_VOID
);

GT_VOID prvTgfPclEgressTrapMirrorCfgSet
(
    GT_VOID
);

GT_VOID prvTgfPclEgressTrapMirrorTrafficGenerate
(
    GT_VOID
);

GT_VOID prvTgfPclEgressTrapMirrorCfgRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
