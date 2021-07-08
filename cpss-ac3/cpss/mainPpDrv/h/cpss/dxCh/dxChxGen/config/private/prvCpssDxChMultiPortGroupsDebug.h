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
* @file prvCpssDxChMultiPortGroupsDebug.h
*
* @brief This file includes the declaration of the "debug"
* for the multi-port-groups device.
*
* @version   3
********************************************************************************
*/
#ifndef __prvCpssDxChMultiPortGroupsDebugh
#define __prvCpssDxChMultiPortGroupsDebugh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>


/**
* @struct PRV_CPSS_DXCH_PP_PORT_GROUPS_DEBUG_PORT_GROUP_PORT_INFO_STC
 *
 * @brief A structure to hold debug info for
 * the 'multi-port-groups' device.
 * define 'port-group port' info as
 * portGroupId, local port number
 * (APPLICABLE DEVICES: Lion; Lion2)
*/
typedef struct{

    /** @brief the port
     *  when CPSS_PORT_GROUP_UNAWARE_MODE_CNS -->
     *  this port-group port info 'Not valid'
     */
    GT_U32 portGroupId;

    /** the local port number in this port */
    GT_U32 localPort;

} PRV_CPSS_DXCH_PP_PORT_GROUPS_DEBUG_PORT_GROUP_PORT_INFO_STC;

/**
* @struct PRV_CPSS_DXCH_PP_PORT_GROUPS_DEBUG_UPLINK_INFO_STC
 *
 * @brief A structure to hold debug info for the
 * 'multi-port-groups' device.
 * define 'Uplink' info with it's 'Internal connections'
 * (APPLICABLE DEVICES: Lion; Lion2)
*/
typedef struct{

    /** Id of the port group that 'start the ring' for this uplink */
    GT_U32 startPortGroupId;

    /** the 'uplink' port */
    PRV_CPSS_DXCH_PP_PORT_GROUPS_DEBUG_PORT_GROUP_PORT_INFO_STC uplinkPortGroupPortInfo;

    /** @brief number of 'Inter connections' , see internalConnectionsArr.
     *  internalConnectionsArr - array of 'Internal connections' from the port-group of
     *  the 'uplink' to the other port-groups.
     *  (dynamic allocated array)
     *  NOTE: those ports should be on active port-groups that reside
     *  on other port-groups then the 'uplink' port-group port
     */
    GT_U32 numberOfInternalConnections;

    PRV_CPSS_DXCH_PP_PORT_GROUPS_DEBUG_PORT_GROUP_PORT_INFO_STC *internalConnectionsArr;

} PRV_CPSS_DXCH_PP_PORT_GROUPS_DEBUG_UPLINK_INFO_STC;


/**
* @struct PRV_CPSS_DXCH_PP_PORT_GROUPS_DEBUG_INFO_STC
 *
 * @brief A structure to hold debug info for the 'multi-port-groups' device.
 * this info used for internal behavior tests (debug) and
 * configuration.
 * (APPLICABLE DEVICES: Lion; Lion2)
*/
typedef struct{

    /** is the debug info valid ? */
    GT_BOOL debugInfoValid;

    /** @brief number of 'uplinks'. see uplinkInfoArr.
     *  uplinkInfoArr - array of 'uplinks' . this array allocated dynamically ,
     *  according to caller.
     *  unknownUnregDesignatedDeviceArray - array of the device number(s) to use in the 'Device map table'
     *  for traffic used as 'Trap to CPU' for 'Vlan unknown Uc, Unreg Mc'
     *  access to this array is 'port group id' of the port group
     *  from which the traffic 'jump' to the next port group
     *  NOTE: the size is (CPSS_MAX_PORT_GROUPS_CNS)
     */
    GT_U32 numOfUplinkPorts;

    PRV_CPSS_DXCH_PP_PORT_GROUPS_DEBUG_UPLINK_INFO_STC *uplinkInfoArr;

    GT_U8 *unknownUnregDesignatedDeviceArray;

    /** @brief the index that used in the 'Cpu code table'
     *  for the entry of 'Vlan unknown Uc, Unreg Mc'
     *  internalTrunksPtr - array with the trunk id's to use for internal trunks ,
     *  according to their port group
     *  access to this array is 'port group id' of the port group
     *  from which the traffic 'jump' to the next port group
     *  NOTE: 1. the size is (CPSS_MAX_PORT_GROUPS_CNS)
     *  2. used when numOfUplinkPorts > 1 the size is CPSS_MAX_PORT_GROUPS_CNS
     */
    GT_U32 unknownUnregDesignatedDeviceIndex;

    GT_TRUNK_ID *internalTrunksPtr;

    /** @brief number of cpu codes , see unknownUnregCpuCodesArray[]
     *  unknownUnregCpuCodesPtr - (pointer to) array of the CPU codes that we need to force to
     *  go to the next port group (unless last port group)
     */
    GT_U32 numOfCpuCodes;

    CPSS_NET_RX_CPU_CODE_ENT *unknownUnregCpuCodesPtr;

    /** @brief the bitmap of port groups that represent the 'last' port
     *  'ends' their interconnection link.
     *  for example (when all 4 port-groups are active):
     *  for single link mode : ( ->0->1->2->3->)
     *  when the uplink is port-group 0 --> lastPortGroupId is 3
     *  when the uplink is port-group 1 --> lastPortGroupId is 0
     *  when the uplink is port-group 2 --> lastPortGroupId is 1
     *  when the uplink is port-group 3 --> lastPortGroupId is 2
     *  for dual link mode : (0<->1 , 2<->3  or 0<->2 , 1<->3 or 0<->3 , 1<->2)
     *  when the uplink is port-group 0 --> lastPortGroupId is 1
     *  when the uplink is port-group 1 --> lastPortGroupId is 0
     *  when the uplink is port-group 2 --> lastPortGroupId is 3
     *  when the uplink is port-group 3 --> lastPortGroupId is 2
     */
    GT_PORT_GROUPS_BMP lastPortGroupBmp;

    /** @brief the srcId filtering needed for :
     *  because packets sent across the ring are not DSA-tagged, the original source
     *  info is not passed to next port group, so that would prevent the use of
     *  source filtering of flooded packets. So we need to use SrcID filtering in
     *  this case, where traffic received on ring ports are filtered on uplink ports.
     */
    GT_U32 srcIdFilterFloodToUplinks;

} PRV_CPSS_DXCH_PP_PORT_GROUPS_DEBUG_INFO_STC;

/* macro to PRV_CPSS_DXCH_PP_PORT_GROUPS_DEBUG_INFO_PTR_MAC */
#define PRV_CPSS_DXCH_PP_PORT_GROUPS_DEBUG_INFO_PTR_MAC(devNum)   \
    (&PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.debugInfo)


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChMultiPortGroupsDebugh */


