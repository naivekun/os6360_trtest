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
* @file prvTgfTunnelTermEtherOverMplsPwLabelParallelLookup.h
*
* @brief Tunnel term Ethernet over MPLS with PW label (Parallel Lookup)
*
* @version   1
********************************************************************************
*/

/**
* @internal tgfTunnelTermEtherOverMplsPwLabelParallelLookupBridgeConfigSet function
* @endinternal
*
* @brief   Set Bridge Configuration
*/
GT_VOID tgfTunnelTermEtherOverMplsPwLabelParallelLookupBridgeConfigSet
(
    GT_VOID
);

/**
* @internal tgfTunnelTermEtherOverMplsPwLabelParallelLookupTtiConfigSet function
* @endinternal
*
* @brief   build TTI Basic rule
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_VOID tgfTunnelTermEtherOverMplsPwLabelParallelLookupTtiConfigSet
(
    GT_VOID
);

/**
* @internal tgfTunnelTermEtherOverMplsPwLabelParallelLookupExceptionCommandSet function
* @endinternal
*
* @brief   Set a PW CW exception command
*
* @param[in] exceptionType            - exception type
* @param[in] command                  - the packet command
*                                       None
*/
GT_VOID tgfTunnelTermEtherOverMplsPwLabelParallelLookupExceptionCommandSet
(
    IN PRV_TGF_TTI_PW_CW_EXCEPTION_ENT  exceptionType,
    IN CPSS_PACKET_CMD_ENT              command
);

/**
* @internal tgfTunnelTermEtherOverMplsPwLabelParallelLookupCpuCodeBaseSet function
* @endinternal
*
* @brief   Set the base CPU code value for PWE3
*
* @param[in] cpuCodeBase              - the base CPU code value for PWE3
*                                      (APPLICABLE RANGES: 192..255)
*                                       None
*/
GT_VOID tgfTunnelTermEtherOverMplsPwLabelParallelLookupCpuCodeBaseSet
(
    IN GT_U32   cpuCodeBase
);

/**
* @internal tgfTunnelTermEtherOverMplsPwLabelParallelLookupConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 1. Restore TTI Configuration
*       2. Restore Base Configuration
*
*/
GT_VOID tgfTunnelTermEtherOverMplsPwLabelParallelLookupConfigurationRestore
(
    GT_VOID
);

/**
* @internal tgfTunnelTermEtherOverMplsPwLabelParallelLookupTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] packetType               -    0: send legal packet
*                                      1: send a packet with illegal control word
* @param[in] expectTraffic            - GT_TRUE: expect the traffic to be forwarded
*                                      GT_FALSE: expect the traffic to be dropped
* @param[in] trapToCpu                -     whether to trap traffic to the CPU
*                                       None
*/
GT_VOID tgfTunnelTermEtherOverMplsPwLabelParallelLookupTrafficGenerate
(
    IN GT_U32   packetType,
    IN GT_BOOL  expectTraffic,
    IN GT_BOOL  trapToCpu
);

/**
* @internal tgfTunnelTermEtherOverMplsPwLabelParallelLookup function
* @endinternal
*
* @brief   Test tunnel term Ethernet over MPLS with PW label Parallel Lookup (main func)
*
* @param[in] cwBasedPwArray[]         - array of cwBasedPw enable values for each lookup
* @param[in] applyNonDataCwCommandArray[] - array of applyNonDataCwCommand enable values for each lookup
* @param[in] restoreConfig            - GT_TRUE:restore
*                                      GT_FALSE: do not restore
*                                       None
*/
GT_VOID tgfTunnelTermEtherOverMplsPwLabelParallelLookup
(
    GT_BOOL cwBasedPwArray[],
    GT_BOOL applyNonDataCwCommandArray[],    
    GT_BOOL restoreConfig
);

