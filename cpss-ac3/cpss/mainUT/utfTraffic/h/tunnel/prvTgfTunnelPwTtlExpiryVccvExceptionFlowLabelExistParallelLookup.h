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
* @file prvTgfTunnelPwTtlExpiryVccvExceptionFlowLabelExistParallelLookup.h
*
* @brief Tunnel: Test VCCV exception on PW TTL and FLOW LABEL Exist
*
* @version   1
********************************************************************************
*/

/**
* @internal prvTgfTunnelPwTtlExpiryVccvExceptionFlowLabelExistParallelLookup function
* @endinternal
*
* @brief   Test main func
*
* @param[in] ttlExpiryVccvEnable[]    - array of ttlExpiryVccv enable values for each lookup
* @param[in] pwe3FlowLabelExist[]     - array of pwe3FlowLabelExist values for each lookup
* @param[in] restoreConfig            - GT_TRUE:restore
*                                      GT_FALSE: do not restore
*                                       None
*/
GT_VOID prvTgfTunnelPwTtlExpiryVccvExceptionFlowLabelExistParallelLookup
(
    IN GT_BOOL ttlExpiryVccvEnable[],
    IN GT_BOOL pwe3FlowLabelExist[],    
    IN GT_BOOL restoreConfig
);

