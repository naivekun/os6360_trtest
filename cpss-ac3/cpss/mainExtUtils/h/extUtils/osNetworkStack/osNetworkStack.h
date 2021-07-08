/*******************************************************************************
*                Copyright 2014, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
*/
/**
********************************************************************************
* @file osNetworkStack.h
*
* @brief CPSS <=> OS TCP/IP stack
*
* @version   1
********************************************************************************
*/
#ifndef __osNetworkStack_h__
#define __osNetworkStack_h__

#include <cpss/common/cpssTypes.h>

/**
* @internal osNetworkStackInit function
* @endinternal
*
* @brief   Initialize library
*/
GT_STATUS osNetworkStackInit(void);

/**
* @internal osNetworkStackConnect function
* @endinternal
*
* @brief   Connect device/vlan to OS network stack (unicast and broadcast)
*         Only one OS network interface can be confugured
* @param[in] devId                    - device ID
* @param[in] vlanId                   - Vlan ID (already configured)
* @param[in] etherAddr                - Mac address of interface
*                                       GT_STATUS
*/
GT_STATUS osNetworkStackConnect(
  IN  GT_U8         devId,
  IN  GT_U16        vlanId,
  IN  GT_ETHERADDR  *etherAddr
);

/**
* @internal osNetworkStackDisconnect function
* @endinternal
*
* @brief   Disconnect PP from OS network stack
*/
GT_STATUS osNetworkStackDisconnect(void);

/**
* @internal osNetworkStackMacSet function
* @endinternal
*
* @brief   Change MAC address of OS network interface, update FDB
*/
GT_STATUS osNetworkStackMacSet(
  IN  GT_ETHERADDR *etherPtr
);

/**
* @internal osNetworkStackMacGet function
* @endinternal
*
* @brief   Query for MAC address of OS network interface
*/
GT_STATUS osNetworkStackMacGet(
  OUT GT_ETHERADDR *etherPtr
);

/**
* @internal osNetworkStackIfconfig function
* @endinternal
*
* @brief   Configure OS network interface
*
* @param[in] config                   - Configuration string
*                                       GT_STATUS
*
* @note Example:
*       osNetworkStackIfconfig("up inet 10.1.2.3 netmask 255.255.255.0")
*
*/
GT_STATUS osNetworkStackIfconfig(
  IN const char *config
);

#endif /* __osNetworkStack_h__ */

