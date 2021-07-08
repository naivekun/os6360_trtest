/*******************************************************************************
*              (C), Copyright 2001, Marvell International Ltd.                 *
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
* @file prvTgfCscdDsaEgrToCpuTrgFlds.h
*
* @brief Test checks target info for TO_CPU by Egress Pipe.
* The packet is sent:
* 1.1 from a network port to a local CPU
* 1.2 from a network port to a remote CPU
* 2  from a cascade port to a remote CPU
* 3  from a cascade port to a local CPU
* Following eDSA tag fields checked:
* Target Tagged / Untagged
* Target Device
* Target physical port
* Target ePort
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfCscdDsaEgrToCpuTrgFldsTest
#define __prvTgfCscdDsaEgrToCpuTrgFldsTest

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfCscdDsaEgrToCpuTrgFldsTest function
* @endinternal
*
* @brief   Test checks target info for TO_CPU by Egress Pipe.
*         Following eDSA tag fields checked:
*         Target Tagged / Untagged
*         Target Device
*         Target physical port
*         Target ePort
*/
GT_VOID prvTgfCscdDsaEgrToCpuTrgFldsTest
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfCscdDsaEgrToCpuTrgFldsTest */

