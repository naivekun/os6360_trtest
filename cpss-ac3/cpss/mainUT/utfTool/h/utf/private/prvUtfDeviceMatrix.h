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
* @file prvUtfDeviceMatrix.h
*
* @brief Internal header with definitions for Device Matrix tests.
*
* @version   2
********************************************************************************
*/
#ifndef __prvUtfDeviceMatrixh
#define __prvUtfDeviceMatrixh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <utf/private/prvUtfHelpers.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>

/* not applicable (NA) table */
#define NA_TABLE_CNS    0

/**
* @struct PRV_UTF_DEVICE_MATRIX_DATA1_STC
 *
 * @brief Holds extra Pp related data .
*/
typedef struct{

    /** indication to do check on <existingPorts> */
    GT_BOOL check_existingPorts;

    /** bitmap of actual exiting ports not including CPU one. */
    CPSS_PORTS_BMP_STC existingPorts;

} PRV_UTF_DEVICE_MATRIX_DATA1_STC;
/**
* @struct PRV_UTF_DEVICE_MATRIX_DATA_STC
 *
 * @brief Holds Pp related data derived from the device matrix.
*/
typedef struct{

    /** HW device ID */
    GT_U32 deviceId;

    /** @brief buffer memory size
     *  transmitDescriptors - number of transmit descriptors
     */
    GT_U32 bufferMemory;

    GT_U32 transmitDescr;

    /** FDB table size */
    GT_U32 fdb;

    /** number of transmit queues */
    GT_U32 txQueues;

    /** number of multicast groups\VIDX */
    GT_U32 mcGroups;

    /** number of trunks */
    GT_U32 trunks;

    /** number of MSTP groups */
    GT_U32 mstp;

    /** number of ingress ACLs */
    GT_U32 ingressAcls;

    /** number of ingress policers */
    GT_U32 ingressPolicer;

    /** number of egress policers */
    GT_U32 egressPolicer;

    /** number of ARP entries */
    GT_U32 arp;

    /** number of TS entries */
    GT_U32 tunnelStart;

    /** number of TTI entries */
    GT_U32 tunnelTermination;

    /** router TCAM size */
    GT_U32 lpm;

    /** number of router next hop entries */
    GT_U32 routerNextHop;

    /** number of IP MLL (multi */
    GT_U32 ipMll;

    /** number of centralized counters */
    GT_U32 cnc;

    /** @brief VLAN translation feature support:
     *  GT_TRUE - supported
     *  GT_FALSE - not supported
     */
    GT_BOOL vlanTranslationSupport;

    /** @brief TR101 translation feature support:
     *  GT_TRUE - supported
     *  GT_FALSE - not supported
     */
    GT_BOOL tr101Support;

    /** number of network ports */
    GT_U32 networkPorts;

    /** network ports speed type */
    PRV_CPSS_PORT_TYPE_ENT networkPortsType;

    /** number of stacking ports */
    GT_U32 stackPorts;

    PRV_UTF_DEVICE_MATRIX_DATA1_STC *data1InfoPtr;

} PRV_UTF_DEVICE_MATRIX_DATA_STC;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvUtfDeviceMatrixh */


