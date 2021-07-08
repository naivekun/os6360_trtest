/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfDitFcoe.h
*
* DESCRIPTION:
*       FCOE tests
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/
#ifndef __prvTgfDitFcoeh
#define __prvTgfDitFcoeh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfFcoeTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 FCOE packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:11,
*         FCOE EtherType = 0x8906
*         FCOE D_ID = 0x112233
*         FCOE S_ID = 0x445566
*         FCOE OX_ID = 0x8888
*         FCOE RX_ID = 0x9999
*         Success Criteria:
*         Packet is captured on port 23 and mirrored to CPU
*         Packet DA is changed according to ARP entry = 00:00:00:00:34:02
*         Packet SA is changed according to vlan value = 00:00:00:00:00:02
*/
GT_VOID prvTgfFcoeTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfFcoeBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfFcoeBaseConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfFcoeBaseConfigurationRestore function
* @endinternal
*
* @brief   Base Configuration Restore
*/
GT_VOID prvTgfFcoeBaseConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfFcoePclConfigurationSet function
* @endinternal
*
* @brief   Set IPCL Configuration
*/
GT_VOID prvTgfFcoePclConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfFcoePclConfigurationRestore function
* @endinternal
*
* @brief   Restore IPCL Configuration
*/
GT_VOID prvTgfFcoePclConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfTtiFcoeForwardingConfigurationSet function
* @endinternal
*
* @brief   Set FCOE Global Configuration
*/
GT_VOID prvTgfTtiFcoeForwardingConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfTtiFcoeForwardingConfigurationRestore function
* @endinternal
*
* @brief   Restore FCOE Global Configuration
*/
GT_VOID prvTgfTtiFcoeForwardingConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfBasicFcoeLpmForwardingConfigurationSet function
* @endinternal
*
* @brief   Set FCOE LPM Configuration
*/
GT_VOID prvTgfBasicFcoeLpmForwardingConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfBasicFcoeLpmForwardingConfigurationRestore function
* @endinternal
*
* @brief   Restore FCOE LPM Configuration
*/
GT_VOID prvTgfBasicFcoeLpmForwardingConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfVlanFcoeForwardingConfigurationSet function
* @endinternal
*
* @brief   Set FCOE Configuration
*/
GT_VOID prvTgfVlanFcoeForwardingConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfVlanFcoeForwardingConfigurationRestore function
* @endinternal
*
* @brief   Restore FCOE Configuration
*/
GT_VOID prvTgfVlanFcoeForwardingConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfIpPortFcoeForwardingConfigurationSet function
* @endinternal
*
* @brief   Set FCOE Configuration
*/
GT_VOID prvTgfIpPortFcoeForwardingConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfIpPortFcoeForwardingConfigurationRestore function
* @endinternal
*
* @brief   Restore FCOE Configuration
*/
GT_VOID prvTgfIpPortFcoeForwardingConfigurationRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfDitFcoeh */

