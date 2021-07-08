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
* @file prvTgfTunnelTermExceptionCpuCode.h
* @version   1
********************************************************************************
*/
#ifndef __prvTgfTunnelTermExceptionCpuCodeh
#define __prvTgfTunnelTermExceptionCpuCodeh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfTunnelGen.h>
#include <common/tgfIpGen.h>

/**
* @internal prvTgfTunnelTermExceptionCpuCodeBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*
* @param[in] vrfId                    - virtual router index
*                                       None
*/
GT_VOID prvTgfTunnelTermExceptionCpuCodeBaseConfigurationSet
(
    GT_U32     vrfId
);

/**
* @internal prvTgfTunnelTermExceptionCpuCodeRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*/
GT_VOID prvTgfTunnelTermExceptionCpuCodeRouteConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermExceptionCpuCodeTtiConfigurationSet function
* @endinternal
*
* @brief   Set TTI Configuration
*/
GT_VOID prvTgfTunnelTermExceptionCpuCodeTtiConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermExceptionCpuCodeMaskHeaderErrorType function
* @endinternal
*
* @brief   Mask/unmask IPv4 UC header error type
*
* @param[in] errorType                - type of IPv4 UC error
* @param[in] mask                     - GT_TRUE:  the error
*                                      GT_FALSE: unmask the error
*                                       None
*/
GT_VOID prvTgfTunnelTermExceptionCpuCodeMaskHeaderErrorType
(
    PRV_TGF_IP_HEADER_ERROR_ENT     errorType,
    GT_BOOL                         mask
);

/**
* @internal prvTgfTunnelTermExceptionCpuCodeTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] test                     - 0: send IPv4 packet with SIP address error
*                                      1: send IPv4 packet with header error - expect trapping to CPU
*                                      2: send IPv4 packet with header error - expect forwarding
*                                       None
*/
GT_VOID prvTgfTunnelTermExceptionCpuCodeTrafficGenerate
(
    IN GT_U8    test
);

/**
* @internal prvTgfTunnelTermExceptionCpuCodeConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 3. Restore TTI Configuration
*       2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfTunnelTermExceptionCpuCodeConfigurationRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfTunnelTermExceptionCpuCodeh */


