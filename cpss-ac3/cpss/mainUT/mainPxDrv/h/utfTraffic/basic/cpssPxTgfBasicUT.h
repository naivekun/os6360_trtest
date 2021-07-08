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
* @file cpssPxTgfBasicUT.h
*
* @brief Pipe : definitions needed for tests with traffic
*
* @version   1
********************************************************************************
*/

#ifndef __cpssPxTgfBasicUT_h
#define __cpssPxTgfBasicUT_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#ifndef UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC
    #define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC
#endif /*UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC*/

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

#include <cpss/px/port/cpssPxPortStat.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/port/cpssPxPortCtrl.h>

#include <extUtils/trafficEngine/prvTgfLog.h>
#include <extUtils/trafficEngine/tgfTrafficGenerator.h>
#include <extUtils/trafficEngine/tgfTrafficTable.h>

extern GT_STATUS prvTgfPxSkipPortWithNoMac(IN GT_SW_DEV_NUM devNum , IN GT_PHYSICAL_PORT_NUM portNum);

/* indication to use ingress info */
#define TGF_AS_INGRESS_CNS      0xffffffff

/* Ingress/egress standard 802_1BR tag operations */
#define EXT_PARAM_DEFAULT_CNS                           (1<<0)

/* Packet ingress at 'UpLink Port' */
/* Egress at 'Extended Port' with removed 8 bytes ETAG and removed 4 bytes VLAN TAG */
#define EXT_PARAM_802_1BR_U2E_EGRESS_UNTAGGED_CNS       (1<<1)
/* Egress at 'Extended Port' with removed 8 bytes ETAG */
#define EXT_PARAM_802_1BR_U2E_EGRESS_TAGGED_CNS         (1<<2)
/* Egress at 'Extended Port' with removed 8 bytes ETAG and removed 4 bytes VLAN TAG  (TPID matched) */
#define EXT_PARAM_802_1BR_U2E_EGRESS_ALL_UNTAGGED_CNS   (1<<3)
/* Egress at Extended Port' with removed 8 bytes ETAG (keep 4 bytes VLAN TAG because not recognized as vlan tag) */
#define EXT_PARAM_802_1BR_U2E_EGRESS_UNTAGGED_TPID_NO_MATCH_CNS       (1<<4)
/* Egress at 'Cascade Port' (as ingress) --> not pop ETAG */
#define EXT_PARAM_802_1BR_U2C_EGRESS_KEEP_ETAG_CNS      (1<<1)
/* Egress at 'Cascade Port' with removed 8 bytes ETAG (target to CPU on next PE) */
#define EXT_PARAM_802_1BR_U2C_EGRESS_POP_ETAG_CNS       (1<<2)
/* Egress at 'Cascade Port' (modified CoS ) --> not pop ETAG */
#define EXT_PARAM_802_1BR_U2C_EGRESS_MODIFIED_COS_ETAG_CNS            (1<<3)
/* Egress at 'Upstream' (untagged packet) --> with VLAN tag */
#define EXT_PARAM_802_1BR_E2U_EGRESS_PVID_CNS            (1<<4)

/* Packet ingress at 'CPU Port' */
/* Egress at 'Uplink Port' (as ingress) --> without ETAG */
#define EXT_PARAM_802_1BR_CPU2U_EGRESS_NO_ETAG_CNS      (1<<1)
/* Egress at 'Uplink Port' (as ingress) --> with ETAG */
#define EXT_PARAM_802_1BR_CPU2U_EGRESS_WITH_ETAG_CNS    (1<<2)

typedef struct{
    GT_PHYSICAL_PORT_NUM    portNum;/* the egress portNum */
    GT_U32                  numBytes;
    GT_U8                  *pktInfo;/* if not NULL checked for match pattern */
}TGF_EXPECTED_EGRESS_INFO_SIMPLE_STC;


typedef struct{
    GT_U32      numOfBytes;
    GT_U8       *partBytesPtr;
}TGF_BYTES_PART_STC;

/* we need parts for :
1. macSa,macDa
2. vlan tag
3. dsa tag
4. qcn tag (mac+dsa+tpid)
5. eTag
6. ip header
7. tcp/udp
8. ptp
9. payload
*/
#define TGF_MAX_PARTS   10
typedef struct{
    GT_U32                  numOfParts;/*must not be more than TGF_MAX_PARTS*/
    TGF_BYTES_PART_STC      partsInfo[TGF_MAX_PARTS];
}TGF_BYTES_INFO_LIST_STC;


typedef struct{
    GT_PHYSICAL_PORT_NUM    portNum;/* the egress portNum */
    TGF_BYTES_INFO_LIST_STC egressFrame;
}TGF_EXPECTED_EGRESS_INFO_SIMPLE_LIST_STC;


/**
* @internal prvTgfPxInjectToIngressPortAndCheckEgressSimple function
* @endinternal
*
* @brief   simple : send SINGLE packet to ingress port and check expected egress ports.
*         function will set :
*         1. link UP
*         2. MAC LOOPBACK
*         3. set 'forwarding' tables to direct the packet from CPU to the port.
*         3. send from cpu packet to egress the port and LOOPBACK will make it
*         ingress the port.
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the ingress port num
* @param[in] framePtr                 - (pointer to) the frame bytes
* @param[in] frameLength              - frame length
* @param[in] numOfEgressPortsInfo     - number of egress ports info in array of egressPortsArr[]
*                                      if 0 --> not checking egress ports
* @param[in] simple_egressPortsArr[]  - array of egress ports to check for counters and packets
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfPxInjectToIngressPortAndCheckEgressSimple
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U8                *framePtr,
    IN GT_U32               frameLength,
    IN GT_U32               numOfEgressPortsInfo,
    IN TGF_EXPECTED_EGRESS_INFO_SIMPLE_STC simple_egressPortsArr[]
);

/**
* @internal prvTgfPxInjectToIngressPortAndCheckEgressSimpleList function
* @endinternal
*
* @brief   simple : send SINGLE packet (build of list of simple parts) to ingress
*         port and check expected egress ports.
*         function will set :
*         1. link UP
*         2. MAC LOOPBACK
*         3. set 'forwarding' tables to direct the packet from CPU to the port.
*         3. send from cpu packet to egress the port and LOOPBACK will make it
*         ingress the port.
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the ingress port num
* @param[in] ingressFramePtr          - (pointer to) the frame parts bytes
* @param[in] numOfEgressPortsInfo     - number of egress ports info in array of egressPortsArr[]
*                                      if 0 --> not checking egress ports
* @param[in] simpleList_egressPortsArr[] - array of egress ports to check for counters and packets
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfPxInjectToIngressPortAndCheckEgressSimpleList
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN TGF_BYTES_INFO_LIST_STC *ingressFramePtr,
    IN GT_U32               numOfEgressPortsInfo,
    IN TGF_EXPECTED_EGRESS_INFO_SIMPLE_LIST_STC simpleList_egressPortsArr[]
);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssPxTgfBasicUT_h */


