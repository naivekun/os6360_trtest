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
* @file prvTgfPclBypass.h
*
* @brief Second Lookup match
*
* @version   2
********************************************************************************
*/
#ifndef __prvTgfPclBypass
#define __prvTgfPclBypass

#include <trafficEngine/tgfTrafficGenerator.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @enum PRV_TGF_PCL_BYPASS_ENT
 *
 * @brief Type of Bypass
*/
typedef enum{

    PRV_TGF_PCL_BYPASS_BRIDGE_E,

    PRV_TGF_PCL_BYPASS_INGRESS_PIPE_E,

    PRV_TGF_PCL_BYPASS_MAX_NUMBER_E

} PRV_TGF_PCL_BYPASS_ENT;



/**
* @internal prvTgfPclBypass function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfPclBypass
(
    IN PRV_TGF_PCL_BYPASS_ENT fieldIndex
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfPclBypass */

