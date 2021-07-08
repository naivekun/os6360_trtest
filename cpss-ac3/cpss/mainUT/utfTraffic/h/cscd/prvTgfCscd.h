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
* @file prvTgfCscd.h
*
* @brief CPSS Mirror
*
* @version   2.
********************************************************************************
*/
#ifndef prvTgfCscd
#define prvTgfCscd

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfCscdSingleTargetDestinationConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set FDB entry with device number 31, port 27, MAC 00:00:00:00: 00:56.
*         -    Set lookup mode to Port for accessing the Device Map table.
*         -    Set the cascade map table
*         - Target device 0, target port 0, link type port, link number 23.
*/
GT_VOID prvTgfCscdSingleTargetDestinationConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfCscdSingleTargetDestinationTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 8 packet:
*         macDa = 00:00:00:00:00:56,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         1 packet is captured on ports 23.
*/
GT_VOID prvTgfCscdSingleTargetDestinationTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfCscdSingleTargetDestinationConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfCscdSingleTargetDestinationConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfCscdSingleTargetDestinationLocalDeviceConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set FDB entry with device number 0, port 1, MAC 00:00:00:00: 00:56.
*         -    Set lookup mode to Port for accessing the Device Map table.
*         -    Set the cascade map table
*         - Target device 0, target port 1, link type port, link number 23.
*/
GT_VOID prvTgfCscdSingleTargetDestinationLocalDeviceConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfCscdSingleTargetDestinationLocalDeviceTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 8 packet:
*         macDa = 00:00:00:00:00:56,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         No packets are captured.
*         Set FDB entry with device number 13, port 27, MAC 00:00:00:00: 00:56.
*         Set the cascade map table
*         - Target device 13, target port 27, link type port, link number 23.
*         Send to device's port 8 packet:
*         macDa = 00:00:00:00:00:56,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         1 packet is captured on ports 23.
*         Set FDB entry with device number 31, port 58, MAC 00:00:00:00: 00:56.
*         Set the cascade map table
*         - Target device 31, target port 58, link type port, link number 0.
*         Send to device's port 8 packet:
*         macDa = 00:00:00:00:00:56,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         1 packet is captured on ports 0.
*/
GT_VOID prvTgfCscdSingleTargetDestinationLocalDeviceTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfCscdSingleTargetDestinationLocalDeviceConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfCscdSingleTargetDestinationLocalDeviceConfigurationRestore
(
    GT_VOID
);


/**
* @internal localEPortMapToRemotePhyPortAndDsaTagCheck function
* @endinternal
*
* @brief   test relevant only to E_ARCH devices
*         1. set fdb entry on eport (TARGET_EPORT_NUM_CNS) on local device
*         2. map by the e2Phy this eport to remote (dev,port) physical interface.
*         a. remote dev = targetDevNumArr[2] , remote port = targetPortNumArr[2]
*         3. Set the egress port as 'egress cascade port' with DSA of 2 words
*         (but as ingress 'not cascade port')
*         4. send the traffic
*         5. trap the traffic that egress the port
*         6. make sure that trgPort in the DSA is targetPortNumArr[2] (and not eport)
*/
GT_VOID localEPortMapToRemotePhyPortAndDsaTagCheck(GT_VOID);





#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfCscd */


