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
* @file prvTgfProtection.h
*
* @brief Protection switching tests functions
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfProtectionh
#define __prvTgfProtectionh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfProtectionGen.h>

/************************************************************************************************************************************************************
*                                                                                                                                                           *
*   TX test                                                                                     RX test                                                     *
*   -------                                                                                     -------                                                     *
*                                                                                                                                                           *
*   *********************************************                                                            *********************************************  *
*   *                                           *                                                    port 18 *                                           *  *
*   *                                           *                                           <----------------------                                      *  *
*   *                                           *                                                            *     |                                     *  *
*   *  *******              *********           * ePort 1000 (physical port 8)                       port 0  *     |                                     *  *
*   *  *     *              *       * ------------------>------------ working path -------->-----------------------                                      *  *
*   *  *     *              *       *           *                                                            *                                           *  *
*   *  *     *              *       *           *                                                ePort 1002  *                                           *  *
*   *  * FDB * -----------> * eVidx *           *                                                            *                                           *  *
*   *  *     *    global    *  98   * ------------------>---------- protection path ------->-----------------------                                      *  *
*   *  *     *     ePort    *       *           * ePort 1001 (physical port 18)                      port 8  *     |                                     *  *
*   *  *     *     1018     *       *           *                                                            *     |                                     *  *
*   *  *******              *********           *                                           <----------------------                                      *  *
*   *    /|\                                    *                                                    port 23 *                                           *  *
*   *     |                                     *                                                            *                                           *  *
*   *     |                                     *                                                            *                                           *  *
*   *     |                                     * port 0                                                     *                                           *  *
*   *     |-------------------------------------------------                                                 *                                           *  *
*   *                                           *                                                            *                                           *  *
*   *********************************************                                                            *********************************************  *
*                                                                                                                                                           *
************************************************************************************************************************************************************/

/**
* @internal prvTgfProtectionTxBridgeConfigurationSet function
* @endinternal
*
* @brief   TX protection switching bridge configurations:
*         - Set VLAN entry
*         - Add ports to VLAN
*         - Set MAC entry
*         - Set E2Phy entries
* @param[in] oneToOne                 - GT_TRUE:  1:1 configuration test
*                                      GT_FALSE: 1+1 configuration test
*                                       None
*/
GT_VOID prvTgfProtectionTxBridgeConfigurationSet
(
    GT_BOOL     oneToOne
);

/**
* @internal prvTgfProtectionRxBridgeConfigurationSet function
* @endinternal
*
* @brief   RX protection switching bridge configurations
*/
GT_VOID prvTgfProtectionRxBridgeConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfProtectionEnableConfigSet function
* @endinternal
*
* @brief   Enable or disable protection switching
*
* @param[in] enable                   - enable/disable protection switching
* @param[in] store                    -  the original enabling status
*                                       None
*/
GT_VOID prvTgfProtectionEnableConfigSet
(
    GT_BOOL         enable,
    GT_BOOL         store
);

/**
* @internal prvTgfProtectionLocStatusConfigSet function
* @endinternal
*
* @brief   Set the LOC table status bits
*
* @param[in] direction                - CPSS_DIRECTION_INGRESS_E: for RX protection switching
*                                      CPSS_DIRECTION_EGRESS_E:  for TX protection switching
* @param[in] workingPathStatus        - LOC status on the working port
* @param[in] store                    -  the original enabling status
*                                       None
*/
GT_VOID prvTgfProtectionLocStatusConfigSet
(
    CPSS_DIRECTION_ENT                      direction,
    PRV_TGF_PROTECTION_LOC_STATUS_ENT       workingPathStatus,
    GT_BOOL                                 store
);

/**
* @internal prvTgfProtectionRxExceptionConfigSet function
* @endinternal
*
* @brief   Set protection RX exception packet command and CPU/drop code
*
* @param[in] command                  - the protection RX exception command
* @param[in] cpuCode                  - the CPU/drop code. relevant if command is not
*                                      CPSS_PACKET_CMD_FORWARD_E
*                                       None
*/
GT_VOID prvTgfProtectionRxExceptionConfigSet
(
    CPSS_PACKET_CMD_ENT                     command,
    CPSS_NET_RX_CPU_CODE_ENT                cpuCode
);

/**
* @internal prvTgfProtectionOnePlusOneTxConfigurationSet function
* @endinternal
*
* @brief   Protection switching 1+1 TX configurations:
*         - Set global ePort configuration
*         - Enable multi-target port mapping
*         - Set the multi-target port value and mask
*         - Set the multi-target port base
*         - Set the port to VIDX base
*         - Configure MLL LTT entry
*         - Configure L2 MLL entry with 1+1 protection enabled
*/
GT_VOID prvTgfProtectionOnePlusOneTxConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfProtectionOneToOneTxConfigurationSet function
* @endinternal
*
* @brief   Protection switching 1:1 TX configurations:
*         - Enable TX protection switching for the target eport
*         - Map between the target eport to the LOC table
*/
GT_VOID prvTgfProtectionOneToOneTxConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfProtectionRxConfigurationSet function
* @endinternal
*
* @brief   Protection switching 1+1 RX configurations:
*         - Enable TTI lookup for TTI_KEY_IPV4 on the RX ports
*         - Set MAC mode for IPv4 key
*         - Configure 2 TTI actions - one for working path and one for protection
*         path
*         - Set packet command for RX protection to hard drop
*         - Map between source ePort in the TTI action to Protection LOC table
*/
GT_VOID prvTgfProtectionRxConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfProtectionTxOneToOneTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] workingPath              - GT_TRUE: the traffic is sent over the working path
*                                      - GT_FALSE: the traffic is sent over the protection path
*                                       None
*/
GT_VOID prvTgfProtectionTxOneToOneTrafficGenerate
(
    GT_BOOL workingPath
);

/**
* @internal prvTgfProtectionTxOnePlusOneTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfProtectionTxOnePlusOneTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfProtectionRxTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] workingCmd               - command on the working path
* @param[in] protectionCmd            - command on the protection path
*                                       None
*/
GT_VOID prvTgfProtectionRxTrafficGenerate
(
    CPSS_PACKET_CMD_ENT     workingCmd,
    CPSS_PACKET_CMD_ENT     protectionCmd
);

/**
* @internal prvTgfProtectionTxOnePlusOneRestoreConfiguration function
* @endinternal
*
* @brief   Restore configuration for the 1+1 test
*/
GT_VOID prvTgfProtectionTxOnePlusOneRestoreConfiguration
(
    GT_VOID
);

/**
* @internal prvTgfProtectionTxOneToOneRestoreConfiguration function
* @endinternal
*
* @brief   Restore configuration for the 1:1 test
*/
GT_VOID prvTgfProtectionTxOneToOneRestoreConfiguration
(
    GT_VOID
);

/**
* @internal prvTgfProtectionRxRestoreConfiguration function
* @endinternal
*
* @brief   Restore configuration for the RX test
*/
GT_VOID prvTgfProtectionRxRestoreConfiguration
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfProtectionh */


