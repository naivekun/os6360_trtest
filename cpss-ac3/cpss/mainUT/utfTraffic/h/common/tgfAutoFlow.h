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
* @file tgfAutoFlow.h
*
* @brief Generic API for enhanced UT AutoFlow.
*
* @version   2
********************************************************************************
*/

#ifndef __tgfAutoFlowh
#define __tgfAutoFlowh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* get public types */
#include <cpss/common/cpssTypes.h>

/**
* @enum PRV_TGF_AF_TRAFFIC_TYPE_ENT
 *
 * @brief types of packets to send.
*/
typedef enum{

    /** Layer 2 packets */
    PRV_TGF_AF_TRAFFIC_TYPE_L2_E,

    /** IPv4 packets */
    PRV_TGF_AF_TRAFFIC_TYPE_IP4_E,

    /** IPv6 packets */
    PRV_TGF_AF_TRAFFIC_TYPE_IP6_E

} PRV_TGF_AF_TRAFFIC_TYPE_ENT;
/**
* @enum PRV_TGF_AF_VLAN_TAG_TYPE_ENT
 *
 * @brief packets tagging.
*/
typedef enum{

    /** untagged packets */
    PRV_TGF_AF_VLAN_TAG_TYPE_UNTAGGED_E,

    /** tagged packets (TAG 0) */
    PRV_TGF_AF_VLAN_TAG_TYPE_TAGGED_E,

    /** double tagged packets (TAG 0 out TAG 1 in) */
    PRV_TGF_AF_VLAN_TAG_TYPE_DOUBLE_TAGGED_E

} PRV_TGF_AF_VLAN_TAG_TYPE_ENT;

/**
* @enum PRV_TGF_AF_ADDRESS_TYPE_ENT
 *
 * @brief unicast or multicast.
*/
typedef enum{

    /** unicast */
    PRV_TGF_AF_ADDRESS_TYPE_UC_E,

    /** multicast */
    PRV_TGF_AF_ADDRESS_TYPE_MC_E,

    /** broadcast */
    PRV_TGF_AF_ADDRESS_TYPE_BC_E

} PRV_TGF_AF_ADDRESS_TYPE_ENT;

/**
* @enum PRV_TGF_AF_TRAFFIC_PARAM_ENT
 *
 * @brief traffic parameters.
*/
typedef enum{

    PRV_TGF_AF_TRAFFIC_PARAM_TRAFFIC_TYPE_E,

    PRV_TGF_AF_TRAFFIC_PARAM_BURST_COUNT_E,

    PRV_TGF_AF_TRAFFIC_PARAM_MAC_DA_E,

    PRV_TGF_AF_TRAFFIC_PARAM_MAC_SA_E,

    /** @brief PRV_TGF_AF_TRAFFIC_PARAM_VLAN_TAG_TYPE_E,
     *  PRV_TGF_AF_TRAFFIC_PARAM_VLAN_TAG_0_E,
     *  PRV_TGF_AF_TRAFFIC_PARAM_VLAN_TAG_1_E,
     *  PRV_TGF_AF_TRAFFIC_PARAM_ETHER_TYPE_E,
     *  PRV_TGF_AF_TRAFFIC_PARAM_IPV4_HEADER_E,
     *  PRV_TGF_AF_TRAFFIC_PARAM_IPV6_HEADER_E,
     */
    PRV_TGF_AF_TRAFFIC_PARAM_ADDRESS_TYPE_E,

    PRV_TGF_AF_TRAFFIC_PARAM_VLAN_TAG_TYPE_E,

    PRV_TGF_AF_TRAFFIC_PARAM_VLAN_TAG_0_E,

    PRV_TGF_AF_TRAFFIC_PARAM_VLAN_TAG_1_E,

    PRV_TGF_AF_TRAFFIC_PARAM_ETHER_TYPE_E,

    PRV_TGF_AF_TRAFFIC_PARAM_IPV4_HEADER_E,

    PRV_TGF_AF_TRAFFIC_PARAM_IPV6_HEADER_E,

    PRV_TGF_AF_TRAFFIC_PARAM_PACKET_SIZE_E,

    PRV_TGF_AF_TRAFFIC_PARAM_DATA_STC_E

} PRV_TGF_AF_TRAFFIC_PARAM_ENT;

/**
* @enum PRV_TGF_AF_BRIDGE_PARAM_ENT
 *
 * @brief bridge parameters.
*/
typedef enum{

    PRV_TGF_AF_BRIDGE_PARAM_INGRESS_VLAN_E,

    PRV_TGF_AF_BRIDGE_PARAM_INGRESS_PORT_IDX_E,

    PRV_TGF_AF_BRIDGE_PARAM_INGRESS_TAG_0_PID_E,

    PRV_TGF_AF_BRIDGE_PARAM_INGRESS_TAG_1_PID_E,

    PRV_TGF_AF_BRIDGE_PARAM_EGRESS_TAG_0_PID_E,

    PRV_TGF_AF_BRIDGE_PARAM_EGRESS_TAG_1_PID_E,

    PRV_TGF_AF_BRIDGE_PARAM_EGRESS_TAGGING_CMD_E

} PRV_TGF_AF_BRIDGE_PARAM_ENT;

/**
* @struct PRV_TGF_AF_DATA_STC
 *
 * @brief packets data structure.
*/
typedef struct{

    GT_U8 *dataPtr;

    /** @brief length of data array
     *  Comment:
     */
    GT_U32 dataLen;

} PRV_TGF_AF_DATA_STC;


/**
* @union PRV_TGF_AF_TRAFFIC_UNT
 *
 * @brief The traffic fields of the auto flow.
 *
*/

typedef union{
    /** L2/IPv4/IPv6 */
    PRV_TGF_AF_TRAFFIC_TYPE_ENT trafficType;

    /** number of packets to send */
    GT_U32 burstCount;

    /** MAC DA */
    TGF_MAC_ADDR daMac;

    /** MAC SA */
    TGF_MAC_ADDR saMac;

    /** UC or MC */
    PRV_TGF_AF_ADDRESS_TYPE_ENT addressType;

    /** untagged/tagged/double tagged */
    PRV_TGF_AF_VLAN_TAG_TYPE_ENT vlanTagType;

    /** vlan tag 0 struct */
    TGF_PACKET_VLAN_TAG_STC vlanTag0;

    /** vlan tag 1 struct */
    TGF_PACKET_VLAN_TAG_STC vlanTag1;

    /** packets EtherType struct */
    TGF_PACKET_ETHERTYPE_STC etherTypeStc;

    /** IPv4 header struct */
    TGF_PACKET_IPV4_STC ipv4Header;

    /** IPv6 header struct */
    TGF_PACKET_IPV6_STC ipv6Header;

    /** size of packetin bytes */
    GT_U32 packetSize;

    /** @brief data structure
     *  Comment:
     */
    PRV_TGF_AF_DATA_STC dataStc;

} PRV_TGF_AF_TRAFFIC_UNT;



/**
* @union PRV_TGF_AF_BRIDGE_UNT
 *
 * @brief The bridge identifiers of the auto flow.
 *
*/

typedef union{
    /** the ingress vlan id */
    GT_U16 ingressVlan;

    /** @brief the ingress port index
     *  Comment:
     */
    GT_U32 ingressPortIdx;

    TGF_ETHER_TYPE ingTag0Pid;

    TGF_ETHER_TYPE ingTag1Pid;

    TGF_ETHER_TYPE egTag0Pid;

    TGF_ETHER_TYPE egTag1Pid;

    PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT egTagCmd;

} PRV_TGF_AF_BRIDGE_UNT;



/*packet size range*/
#define PRV_TGF_AF_PACKET_MIN_SIZE_CNS 64
#define PRV_TGF_AF_PACKET_MAX_SIZE_CNS 1500

/*max vlans*/
#define PRV_TGF_AF_MAX_VLANS_CNS 4094

/*max burst of packets*/
#define PRV_TGF_AF_MAX_BURST_COUNT_CNS 5

/*MAC address length in bytes*/
#define PRV_TGF_AF_MAC_ADDR_LEN_CNS 6

/*IPv4 address length*/
#define PRV_TGF_AF_IPV4_ADDR_LEN_CNS 4

/*IPv6 address length in 2_bytes*/
#define PRV_TGF_AF_IPV6_ADDR_LEN_CNS 8

/*max ethertype value*/
#define PRV_TGF_AF_MAX_GTU16_CNS 0xffff



/**
* @internal tgfAutoFlowTrafficPreset function
* @endinternal
*
* @brief   The function presets basic traffic settings for the auto test creation.
*
* @param[in] trafficParam             - parameter for setting
* @param[in] valuePtr                 - (pointer to) value for the parameter
*                                       None.
*/
GT_VOID tgfAutoFlowTrafficPreset
(
    IN  PRV_TGF_AF_TRAFFIC_PARAM_ENT                    trafficParam,
    IN  PRV_TGF_AF_TRAFFIC_UNT                         *valuePtr
);

/**
* @internal tgfAutoFlowTrafficGet function
* @endinternal
*
* @brief   The function returns basic traffic settings for the auto test creation.
*         If the parameter has not been set earlier,
*         the recieved value will not necessarily be the one used in the test
*         - randomization might occur while creating the packet.
*         for more information - look in the description of the parameters.
* @param[in] trafficParam             - parameter to get
*
* @param[out] valuePtr                 - (pointer to) value of the parameter
*                                       None.
*/
GT_VOID tgfAutoFlowTrafficGet
(
    IN  PRV_TGF_AF_TRAFFIC_PARAM_ENT                    trafficParam,
    OUT PRV_TGF_AF_TRAFFIC_UNT                          *valuePtr
);

/**
* @internal tgfAutoFlowBridgePreset function
* @endinternal
*
* @brief   The function presets basic bridge settings for the auto test creation.
*
* @param[in] bridgeParam              - parameter for setting
* @param[in] value                    -  for the parameter
*                                       None.
*/
GT_VOID tgfAutoFlowBridgePreset
(
    IN  PRV_TGF_AF_BRIDGE_PARAM_ENT                    bridgeParam,
    IN  PRV_TGF_AF_BRIDGE_UNT                          value
);
    
/**
* @internal tgfAutoFlowBridgeGet function
* @endinternal
*
* @brief   The function returns basic bridge settings for the auto test creation.
*         If the parameter has not been set earlier,
*         the recieved value will not necessarily be the one used in the test
*         - randomization might occur while creating the test.
*         for more information - look in the description of the parameters.
* @param[in] bridgeParam              - parameter to get
*
* @param[out] valuePtr                 - (pointer to) value of the parameter
*                                       None.
*/
GT_VOID tgfAutoFlowBridgeGet
(
    IN  PRV_TGF_AF_BRIDGE_PARAM_ENT                    bridgeParam,
    OUT  PRV_TGF_AF_BRIDGE_UNT                        *valuePtr
);
    
/**
* @internal tgfAutoFlowRandSeed function
* @endinternal
*
* @brief   The function presets the seed,
*         to be used in the rand process.
*/
GT_VOID tgfAutoFlowRandSeed
(
    IN GT_U32   seed
);

/**
* @internal tgfAutoFlowCreate function
* @endinternal
*
* @brief   The function configures the switch & creates the packets to be sent,
*         according to preset, and randomization of undetermined parameters.
*/
GT_VOID tgfAutoFlowCreate
(
    IN GT_VOID
);

/**
* @internal tgfAutoFlowTxSetup function
* @endinternal
*
* @brief   The function sets the transmission of a single flow.
*/
GT_VOID tgfAutoFlowTxSetup
(
    IN GT_VOID
);

/**
* @internal tgfAutoFlowTxStart function
* @endinternal
*
* @brief   The function starts the transmission of a single flow.
*/
GT_VOID tgfAutoFlowTxStart
(
    IN GT_VOID
);

/**
* @internal tgfAutoFlowDel function
* @endinternal
*
* @brief   The function resets all the configurations.
*/
GT_VOID tgfAutoFlowDel
(
    IN GT_VOID
);

/**
* @internal tgfAutoFlowLibInit function
* @endinternal
*
* @brief   The function initializes the auto flow library,
*         and allocates the dynamic data base.
*/
GT_VOID tgfAutoFlowLibInit
(
    IN GT_VOID
);

/**
* @internal tgfAutoFlowLibClose function
* @endinternal
*
* @brief   The function closes the auto flow library,
*         and frees the allocated memory for the data base.
*/
GT_VOID tgfAutoFlowLibClose
(
    IN GT_VOID
);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __tgfAutoFlowh */




