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
* @file prvTgfCscdDsaFrwdSrcId.h
*
* @brief Test a source-ID in the eDsa tag (FORWARD command). The packet is sent:
* - from a network port to a cascade port
* - from a cascade port to a cascade port
* - from a cascade port to a network port
* Tx port is checked: it should or shouldn't filter the packet
* depending on the port is added to appropriate source-id group.
*
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfCscdDsaFrwdSrcId
#define __prvTgfCscdDsaFrwdSrcId

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfCscdDsaFrwdSrcIdTest function
* @endinternal
*
* @brief   Test a source-ID value in the eDsa tag with command FORWARD
*/
GT_VOID prvTgfCscdDsaFrwdSrcIdTest
(
    GT_VOID
);

/**
* @internal prvTgfCscdDsaFrwdSrcIdTest_xCat3x_Extra function
* @endinternal
*
* @brief   Test a source-ID value in the eDsa tag with command FORWARD
*         on remote physical ports
*/
GT_VOID prvTgfCscdDsaFrwdSrcIdTest_xCat3x_Extra
(
    GT_VOID
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfCscdDsaFrwdSrcId */




