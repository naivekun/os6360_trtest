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
* @file prvTgfCscdDsaToCpuTrgFlds.h
*
* @brief Test checks target info for TO CPU by Ingress Pipe for Original is Trunk.
* The packet is sent:
* - from a network port to a cascade port
* - from a cascade port to a cascade port
* - from a cascade port to a CPU port
* Following eDSA tag fields checked:
* Source / Target packet data
* Source Tagged / Untagged
* Source Device
* Source physical port
* Orig is Trunk - should be 1
* Source ePort
* Source Trunk
*
* @version   2
********************************************************************************
*/
#ifndef __prvTgfCscdDsaToCpuTrgFlds
#define __prvTgfCscdDsaToCpuTrgFlds

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfCscdDsaToCpuNetworkToCascade function
* @endinternal
*
* @brief   Test for to CPU eDsa tag (network to cascade)
*/
GT_VOID prvTgfCscdDsaToCpuNetworkToCascade
(
    GT_VOID
);

/**
* @internal prvTgfCscdDsaToCpuCascadeToCascade function
* @endinternal
*
* @brief   Test for to CPU eDsa tag (cascade to cascade)
*/
GT_VOID prvTgfCscdDsaToCpuCascadeToCascade
(
    GT_VOID
);

/**
* @internal prvTgfCscdDsaToCpuCascadeToCpu function
* @endinternal
*
* @brief   Test for to CPU eDsa tag (cascade to CPU port)
*/
GT_VOID prvTgfCscdDsaToCpuCascadeToCpu
(
    GT_VOID
);


/**
* @internal prvTgfCscdDsaForwardCascadeToCpu function
* @endinternal
*
* @brief   Test for to FORWARD dsa that trap to CPU eDsa tag (cascade to CPU port)
*         but with trunkId on the 'remap on tti'
*/
GT_VOID prvTgfCscdDsaForwardCascadeToCpu
(
    GT_VOID
);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfCscdDsaToCpuTrgFlds */

