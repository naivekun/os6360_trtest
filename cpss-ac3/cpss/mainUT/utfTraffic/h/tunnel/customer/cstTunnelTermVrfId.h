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
* @file cstTunnelTermVrfId.h
*
* @brief CST: Tunnel Term: L3 Interface based on (port, vlan)
*
* @version   1
********************************************************************************
*/
#ifdef CHX_FAMILY



/**
* @internal cstTunnelTermVrfIdConfigSet function
* @endinternal
*
* @brief   Set configuration
*/
GT_STATUS cstTunnelTermVrfIdConfigSet(GT_VOID);


/**
* @internal cstTunnelTermVrfIdConfigRestore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_STATUS cstTunnelTermVrfIdConfigRestore(GT_VOID);


#endif /* CHX_FAMILY */



