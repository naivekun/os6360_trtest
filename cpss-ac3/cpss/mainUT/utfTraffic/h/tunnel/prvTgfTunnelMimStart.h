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
* @file prvTgfTunnelMimStart.h
*
* @brief Tunnel Start: Mac In Mac Functionality
*
* @version   5
********************************************************************************
*/
#ifndef __prvTgfTunnelMimStarth
#define __prvTgfTunnelMimStarth

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @enum PRV_TGF_TUNNEL_MIM_START_TYPE_ENT
 *
 * @brief Type of MAC In MAC Tunnel Start Test
*/
typedef enum{

    PRV_TGF_TUNNEL_MIM_START_FROM_TTI_E,

    PRV_TGF_TUNNEL_MIM_START_FROM_PCL_E,

    PRV_TGF_TUNNEL_MIM_START_TYPE_MAX_E

} PRV_TGF_TUNNEL_MIM_START_TYPE_ENT;


/**
* @internal prvTgfTunnelMimStart function
* @endinternal
*
* @brief   MIM Tunnel Start
*
* @param[in] startType                - Type of MAC in MAC Test
*                                      OUTPUTS:
*                                      None
*                                       None
*/
GT_VOID prvTgfTunnelMimStart
(
    IN PRV_TGF_TUNNEL_MIM_START_TYPE_ENT startType
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfTunnelMimStarth */


