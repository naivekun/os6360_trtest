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
* @file cpssPxHalBpe.h
*
* @brief CPSS PX implementation for Bridge Port Extenders (BPE).
*
* @version   1
********************************************************************************
*/

#ifndef __cpssPxHalBpeh
#define __cpssPxHalBpeh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/common/cos/cpssCosTypes.h>
#include <cpss/px/cnc/cpssPxCnc.h>

/**************************** Constants definitions *****************************/

#define CPSS_PCP_RANGE_CNS  8 /* 3 bits - class of service */
#define CPSS_DEI_RANGE_CNS  2 /* 1 bit - Drop eligible indicator */
#define CPSS_PX_HAL_BPE_CTRL_DATA_RULE_MAX_CNS 21 /* max index for Data Control Traffic QoS rules */

/****************************** Enum definitions ********************************/
/**
* @enum CPSS_PX_HAL_BPE_PORT_MODE_ENT
 *
 * @brief Enumeration of bridge port extender port types.
*/
typedef enum{

    CPSS_PX_HAL_BPE_PORT_MODE_IDLE_E  =  0,

    CPSS_PX_HAL_BPE_PORT_MODE_TRUSTED_E,

    CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E,

    CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E,

    CPSS_PX_HAL_BPE_PORT_MODE_CASCADE_E,

    CPSS_PX_HAL_BPE_PORT_MODE_INTERNAL_E

} CPSS_PX_HAL_BPE_PORT_MODE_ENT;


/**
* @enum CPSS_PX_HAL_BPE_INTERFACE_MODE_ENT
 *
 * @brief Enumeration of bridge port extender interface types.
*/
typedef enum{

    CPSS_PX_HAL_BPE_INTERFACE_MODE_IDLE_E  =  0,

    CPSS_PX_HAL_BPE_INTERFACE_MODE_TRUSTED_E,

    CPSS_PX_HAL_BPE_INTERFACE_MODE_UPSTREAM_E,

    CPSS_PX_HAL_BPE_INTERFACE_MODE_EXTENDED_E,

    CPSS_PX_HAL_BPE_INTERFACE_MODE_CASCADE_E,

    CPSS_PX_HAL_BPE_INTERFACE_MODE_INTERNAL_E

} CPSS_PX_HAL_BPE_INTERFACE_MODE_ENT;

/**
* @enum CPSS_PX_HAL_BPE_INTERFACE_TYPE_ENT
 *
 * @brief enumerator for interface type
*/
typedef enum{

    /** @brief the interface is of port type (dev,port)
     *  CPSS_PX_HAL_BPE_INTERFACE_TRUNK_E       the interface is of trunk type (trunkId)
     *  CPSS_PX_HAL_BPE_INTERFACE_NONE_E    -   no interface
     */
    CPSS_PX_HAL_BPE_INTERFACE_PORT_E    =   CPSS_INTERFACE_PORT_E,

    CPSS_PX_HAL_BPE_INTERFACE_TRUNK_E   =   CPSS_INTERFACE_TRUNK_E,

    CPSS_PX_HAL_BPE_INTERFACE_NONE_E    =   0xFF

} CPSS_PX_HAL_BPE_INTERFACE_TYPE_ENT;

/**
* @enum CPSS_PX_HAL_BPE_TRUNK_TYPE_ENT
 *
 * @brief enumerator for trunk type
*/
typedef enum{

    /** the trunk consists of upstream ports */
    CPSS_PX_HAL_BPE_TRUNK_UPSTREAM_E,

    /** the trunk consists of extended ports */
    CPSS_PX_HAL_BPE_TRUNK_EXTENDED_E,

    /** the trunk consists of cascade ports */
    CPSS_PX_HAL_BPE_TRUNK_CASCADE_E,

    /** the trunk consists of internal ports */
    CPSS_PX_HAL_BPE_TRUNK_INTERNAL_E

} CPSS_PX_HAL_BPE_TRUNK_TYPE_ENT;

/**
* @enum CPSS_PX_HAL_BPE_TRUNK_LOAD_BALANCE_MODE_ENT
 *
 * @brief enumerator for trunk load balance mode
*/
typedef enum{

    /** trunk load balance according to MAC,IP,TCP */
    CPSS_PX_HAL_BPE_TRUNK_LOAD_BALANCE_MODE_MAC_IP_TCP_E = 0,

    /** trunk load balance according to port */
    CPSS_PX_HAL_BPE_TRUNK_LOAD_BALANCE_MODE_PORT_BASE_E,

    /** trunk load balance according to MAC */
    CPSS_PX_HAL_BPE_TRUNK_LOAD_BALANCE_MODE_MAC_E,

    /** trunk load balance according to IP */
    CPSS_PX_HAL_BPE_TRUNK_LOAD_BALANCE_MODE_IP_E

} CPSS_PX_HAL_BPE_TRUNK_LOAD_BALANCE_MODE_ENT;

/********************************* Structures definitions ***********************/
/**
* @struct CPSS_PX_HAL_BPE_INTERFACE_INFO_STC
 *
 * @brief Defines the interface info
*/

typedef struct{
    CPSS_PX_HAL_BPE_INTERFACE_TYPE_ENT  type;
    struct{
        GT_SW_DEV_NUM devNum;
        GT_PORT_NUM   portNum;
    }devPort;
    GT_TRUNK_ID       trunkId;
}CPSS_PX_HAL_BPE_INTERFACE_INFO_STC;

/**
* @struct CPSS_PX_HAL_BPE_QOSMAP_STC
 *
 * @brief Defines the remap E-PCP and E-DEI
*/
typedef struct{

    GT_U32 newPcp;

    GT_U32 newDei;

} CPSS_PX_HAL_BPE_QOSMAP_STC;


/**
* @struct CPSS_PX_HAL_BPE_QOSMAP_DATA_STC
 *
 * @brief Defines the remap E-PCP and E-DEI and assign queue
*/
typedef struct{

    GT_U32 newPcp;

    GT_U32 newDei;

    /** egress tx traffic class queue (APPLICABLE RANGES: 0..7). */
    GT_U8 queue;

} CPSS_PX_HAL_BPE_QOSMAP_DATA_STC;

/**
* @struct CPSS_PX_HAL_BPE_PORT_QOS
 *
 * @brief Defines qos mapping per port
*/
typedef struct{

    CPSS_PX_HAL_BPE_QOSMAP_STC defaultQos;

    CPSS_PX_HAL_BPE_QOSMAP_STC qosMap[CPSS_PCP_RANGE_CNS][CPSS_DEI_RANGE_CNS];

} CPSS_PX_HAL_BPE_PORT_QOS;


/**
* @enum CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_TYPE_ENT
 *
 * @brief enumerator for encapsulation type
*/
typedef enum{

    /** encapsulation type any */
    CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_ANY_E,

    /** encapsulation type ipv4 */
    CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_IPV4_E,

    /** encapsulation type ipv6 */
    CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_IPV6_E,

    /** encapsulation type ethernet or LLC SNAP */
    CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_ETHERNET_OR_LLC_SNAP_E,

    /** encapsulation type LLC not SNAP */
    CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_LLC_NOT_SNAP_E

} CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_TYPE_ENT;

/**
* @struct CPSS_PX_HAL_BPE_CTRL_DATA_CLASSIFIER_STC
 *
 * @brief Defines the classifier
*/
typedef struct{

    /** @brief Ethernet Type or DSAP+SSAP.
     *  Relevant for CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_ETHERNET_OR_LLC_SNAP_E
     *  CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_LLC_NOT_SNAP_E
     */
    GT_U16 etherType;

    /** Destination MAC Address */
    GT_ETHERADDR macDa;

    /** @brief The DSCP field of the IPv4/6 header.
     *  Relevant for CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_IPV4_E
     *  CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_IPV6_E
     */
    GT_U32 dscp;

    /** @brief IPv4 destination IP address field.
     *  Relevant for CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_IPV4_E
     */
    GT_IPADDR dip;

    /** @brief IP protocol/Next Header type.
     *  Relevant for CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_IPV4_E
     */
    GT_U32 ipProtocol;

    /** @brief IPv6 destination IP address field.
     *  Relevant for CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_IPV6_E
     */
    GT_IPV6ADDR dipv6;

} CPSS_PX_HAL_BPE_CTRL_DATA_CLASSIFIER_STC;

/**
* @internal cpssPxHalBpeInit function
* @endinternal
*
* @brief   Initialize the BPE package, called only after CPSS initialize is completed.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or infoType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxHalBpeInit
(
    IN  GT_SW_DEV_NUM                    devNum
);


/**
* @internal cpssPxHalBpeNumOfChannelsGet function
* @endinternal
*
* @brief   Get the number of E-channels supported by the BPE (unicast+multicast channels).
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] numOfChannelsPtr         - (pointer to) number of E-channels
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssPxHalBpeNumOfChannelsGet
(
    IN  GT_SW_DEV_NUM                    devNum,
    OUT GT_U32                           *numOfChannelsPtr
);


/**
* @internal cpssPxHalBpeNumOfChannelsSet function
* @endinternal
*
* @brief   Set the maximum number of unicast E-channels and multicast E-channels.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] numOfUnicastChannels     - number of unicast E-channels (APPLICABLE RANGE: 0..4k-20)
* @param[in] numOfMulticastChannels   - number of multicast E-channels (APPLICABLE RANGE: 0..8K-20)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
*
* @note This function must be called after the cpssPxHalBpeInit
*
*/
GT_STATUS cpssPxHalBpeNumOfChannelsSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_U32                          numOfUnicastChannels,
    IN  GT_U32                          numOfMulticastChannels
);

/**
* @internal cpssPxHalBpeDebugCncBlocksSet function
* @endinternal
*
* @brief   Set the bitmap of CNC Blocks that can be used by 802.1BR refHal for debug.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cncBitmap                - bitmap of CNC Blocks (APPLICABLE RANGE: 0..3).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
*
* @note Default Configuration:
*       1 CNC block: Enable ingress-processing client
*       2 CNC blocks: Enable ingress-processing and egress-queue_pass_drop clients
*
*/
GT_STATUS cpssPxHalBpeDebugCncBlocksSet
(
    IN  GT_SW_DEV_NUM                    devNum,
    IN  GT_U32                           cncBitmap
);


/**
* @internal cpssPxHalBpeCncConfigClient function
* @endinternal
*
* @brief   Configure CNC for client.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -       device number
* @param[in] client                   -       CNC client; valid values:
*                                      CPSS_PX_CNC_CLIENT_INGRESS_PROCESSING_E
*                                      CPSS_PX_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_QCN_E
* @param[in] blockNum                 -      CNC block number (APPLICABLE RANGE: 0..1).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - cpssPxCncUploadInit not called
* @retval GT_BAD_STATE             - if the previous CNC upload in process.
* @retval GT_NOT_SUPPORTED         - block upload operation not supported.
* @retval GT_NO_MORE               - the action succeeded and there are no more waiting
*                                       CNC counter value
* @retval GT_NOT_READY             - Upload started after upload trigger or
*                                       continued after queue rewind but yet
*                                       not paused due to queue full and yet not finished.
*                                       The part of queue memory does not contain uploaded counters yet.
*                                       No counters retrieved.
*/
GT_STATUS cpssPxHalBpeCncConfigClient
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  CPSS_PX_CNC_CLIENT_ENT      client,
    IN  GT_U32                      blockNum
);

/**
* @internal cpssPxHalBpeReset function
* @endinternal
*
* @brief   Clean all BPE configurations.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note set all ports to mode idle.
*       delete all E-channels Unicast and Multicast.
*
*/
GT_STATUS cpssPxHalBpeReset
(
    IN  GT_SW_DEV_NUM   devNum
);

/**
* @internal cpssPxHalBpeInterfaceTypeSet function
* @endinternal
*
* @brief   Set BPE interface mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] interfacePtr             - (pointer to) interface number
* @param[in] mode                     - interface mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Changing mode: CASCADE->EXTENDED or EXTENDED->CASCADE:
*       it is the application responsiablitiy to check validity of E-channels.
*
*/
GT_STATUS cpssPxHalBpeInterfaceTypeSet
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  CPSS_PX_HAL_BPE_INTERFACE_INFO_STC  *interfacePtr,
    IN  CPSS_PX_HAL_BPE_INTERFACE_MODE_ENT  mode
);

/**
* @internal cpssPxHalBpeUpstreamQosMapSet function
* @endinternal
*
* @brief   Set CoS mapping between E-PCP to QUEUE for Upstream port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] pcp                      - class of service (APPLICABLE RANGE: 0..7).
* @param[in] queue                    - egress  (APPLICABLE RANGE: 0..7).
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxHalBpeUpstreamQosMapSet
(
    IN  GT_SW_DEV_NUM       devNum,
    IN  GT_U32              pcp,
    IN  GT_U32              queue
);

/**
* @internal cpssPxHalBpeExtendedQosMapSet function
* @endinternal
*
* @brief   Set CoS mapping between E-PCP to QUEUE for Extended/Cascade port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] pcp                      - class of service (APPLICABLE RANGE: 0..7).
* @param[in] queue                    - egress  (APPLICABLE RANGE: 0..7).
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This function must be called after cpssPxHalBpePortExtendedQosMapSet,
*       where egress PCP is updated.
*
*/
GT_STATUS cpssPxHalBpeExtendedQosMapSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_PORT_NUM     portNum,
    IN  GT_U32          pcp,
    IN  GT_U32          queue
);

/**
* @internal cpssPxHalBpeInterfaceExtendedPcidSet function
* @endinternal
*
* @brief   Set Extended/Cascade interface Pcid.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] interfacePtr             - (pointer to) interface number
* @param[in] pcid                     - E-channel Id to assign to ingress traffic on interface
*                                      (APPLICABLE RANGE: 0..maxBpeUnicastEChannels-1)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxHalBpeInterfaceExtendedPcidSet
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  CPSS_PX_HAL_BPE_INTERFACE_INFO_STC  *interfacePtr,
    IN  GT_U32                              pcid
);

/**
* @internal cpssPxHalBpePortExtendedUpstreamSet function
* @endinternal
*
* @brief   Set Extended/Cascade port Upstream interface.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] upstreamPtr              - (pointer to) upstream interface
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxHalBpePortExtendedUpstreamSet
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  CPSS_PX_HAL_BPE_INTERFACE_INFO_STC  *upstreamPtr
);

/**
* @internal cpssPxHalBpePortExtendedQosDefaultPcpDeiSet function
* @endinternal
*
* @brief   Set Extended/Cascade port default E-PCP and E-DEI.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] pcp                      - class of service (APPLICABLE RANGE: 0..7).
* @param[in] dei                      -   drop eligibility indication (APPLICABLE RANGE: 0..1).
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxHalBpePortExtendedQosDefaultPcpDeiSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_PORT_NUM     portNum,
    IN  GT_U32          pcp,
    IN  GT_U32          dei
);

/**
* @internal cpssPxHalBpePortExtendedQosMapSet function
* @endinternal
*
* @brief   Set CoS mapping between C-UP,C-CFI to E-PCP,E-DEI for Extended port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] qosMapArr[CPSS_PCP_RANGE_CNS] - qos mapping array
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxHalBpePortExtendedQosMapSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PORT_NUM                 portNum,
    IN  CPSS_PX_HAL_BPE_QOSMAP_STC  qosMapArr[CPSS_PCP_RANGE_CNS][CPSS_DEI_RANGE_CNS]
);

/**
* @internal cpssPxHalBpeUnicastEChannelCreate function
* @endinternal
*
* @brief   Create E-channel.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cid                      - E-channel Id (APPLICABLE RANGE: 0..maxBpeUnicastEChannels-1)
* @param[in] downStreamInterfacePtr   - (pointer to) downstream Interface
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_ALREADY_EXIST         - the E-Channel already was already created
*/
GT_STATUS cpssPxHalBpeUnicastEChannelCreate
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  GT_U32                              cid,
    IN  CPSS_PX_HAL_BPE_INTERFACE_INFO_STC  *downStreamInterfacePtr
);

/**
* @internal cpssPxHalBpeUnicastEChannelDelete function
* @endinternal
*
* @brief   Delete E-channel.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cid                      - E-channel Id (APPLICABLE RANGE: 0..maxBpeUnicastEChannels-1)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxHalBpeUnicastEChannelDelete
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          cid
);

/**
* @internal cpssPxHalBpePortExtendedUntaggedVlanAdd function
* @endinternal
*
* @brief   Configure the Extended interface to egress packets untagged
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] interfacePtr             - (pointer to) interface number
* @param[in] vid                      - vlanId to egress packet untagged.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxHalBpePortExtendedUntaggedVlanAdd
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  CPSS_PX_HAL_BPE_INTERFACE_INFO_STC  *interfacePtr,
    IN  GT_U32                              vid
);


/**
* @internal cpssPxHalBpePortExtendedUntaggedVlanDel function
* @endinternal
*
* @brief   Configure the Extended interface to egress packets tagged
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] interfacePtr             - (pointer to) interface number
* @param[in] vid                      -   vlanId to egress packet tagged
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxHalBpePortExtendedUntaggedVlanDel
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  CPSS_PX_HAL_BPE_INTERFACE_INFO_STC  *interfacePtr,
    IN  GT_U32                              vid
);

/**
* @internal cpssPxHalBpeTrunkCreate function
* @endinternal
*
* @brief   Create a TRUNK.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - trunk ID
* @param[in] trunkType                - type of trunk ports
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_ALREADY_EXIST         - the E-Channel already was already created
*
* @note TRUNK contains ports from same TRUNK type
*
*/
GT_STATUS cpssPxHalBpeTrunkCreate
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_TRUNK_ID                     trunkId,
    IN  CPSS_PX_HAL_BPE_TRUNK_TYPE_ENT  trunkType
);

/**
* @internal cpssPxHalBpeTrunkRemove function
* @endinternal
*
* @brief   Remove a TRUNK.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - trunk ID
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on trunk doesn't exists or doesn't have cid defined
*/
GT_STATUS cpssPxHalBpeTrunkRemove
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_TRUNK_ID     trunkId
);

/**
* @internal cpssPxHalBpeTrunkLoadBalanceModeSet function
* @endinternal
*
* @brief   Set the Load Balance Algorithm of the TRUNK.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] loadBalanceMode          - load balance mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on offset in udbpArr is out of range
*/
GT_STATUS cpssPxHalBpeTrunkLoadBalanceModeSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_HAL_BPE_TRUNK_LOAD_BALANCE_MODE_ENT loadBalanceMode
);

/**
* @internal cpssPxHalBpeTrunkPortsAdd function
* @endinternal
*
* @brief   Add ports to a TRUNK.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - trunk ID
* @param[in] portListPtr              -  (pointer to) list of ports to add to trunkId
* @param[in] portListLen              -  port list length
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note TRUNK contains ports from same TRUNK type
*
*/
GT_STATUS cpssPxHalBpeTrunkPortsAdd
(
    IN  GT_SW_DEV_NUM                         devNum,
    IN  GT_TRUNK_ID                           trunkId,
    IN  CPSS_PX_HAL_BPE_INTERFACE_INFO_STC    *portListPtr,
    IN  GT_U8                                 portListLen
);

/**
* @internal cpssPxHalBpeTrunkPortsDelete function
* @endinternal
*
* @brief   Delete ports from a TRUNK.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - trunk ID
* @param[in] portListPtr              -  (pointer to) list of ports to delete from trunkId
* @param[in] portListLen              -  port list length
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxHalBpeTrunkPortsDelete
(
    IN  GT_SW_DEV_NUM                         devNum,
    IN  GT_TRUNK_ID                           trunkId,
    IN  CPSS_PX_HAL_BPE_INTERFACE_INFO_STC    *portListPtr,
    IN  GT_U8                                 portListLen
);


/**
* @internal cpssPxHalBpeMulticastEChannelCreate function
* @endinternal
*
* @brief   Create Multicast E-Channel and add interface.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cid                      - Multicast E-Channel Id (APPLICABLE RANGE: maxBpeUnicastEChannels..maxBpeUnicastEChannels+maxBpeMulticastEChannels-1)
* @param[in] interfaceListPtr         - (pointer to) interface list
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note cid of MC: <GRP[1:0],CID[11:0]>; GRP>0
*
*/
GT_STATUS cpssPxHalBpeMulticastEChannelCreate
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  GT_U32                              cid,
    IN  CPSS_PX_HAL_BPE_INTERFACE_INFO_STC  *interfaceListPtr
);

/**
* @internal cpssPxHalBpeMulticastEChannelUpdate function
* @endinternal
*
* @brief   Update Multicast E-Channel and add interface.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cid                      - Multicast E-Channel Id (APPLICABLE RANGE: maxBpeUnicastEChannels..maxBpeUnicastEChannels+maxBpeMulticastEChannels-1)
* @param[in] interfaceListPtr         - (pointer to) interface list to replace
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note cid of MC: <GRP[1:0],CID[11:0]>; GRP>0
*
*/
GT_STATUS cpssPxHalBpeMulticastEChannelUpdate
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  GT_U32                              cid,
    IN  CPSS_PX_HAL_BPE_INTERFACE_INFO_STC  *interfaceListPtr
);

/**
* @internal cpssPxHalBpeMulticastEChannelDelete function
* @endinternal
*
* @brief   Delete multicast E-Channel.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cid                      - Multicast E-Channel Id (APPLICABLE RANGE: maxBpeUnicastEChannels..maxBpeUnicastEChannels+maxBpeMulticastEChannels-1)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note cid of MC: <GRP[1:0],CID[11:0]>; GRP>0
*
*/
GT_STATUS cpssPxHalBpeMulticastEChannelDelete
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          cid
);

/**
* @internal cpssPxHalBpeDataControlQosRuleAdd function
* @endinternal
*
* @brief   Add or replace the existing rule (of any type) with the given index for Extended port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -   device number
* @param[in] ruleIndex                -   index of the rule (0 .. CPSS_PX_HAL_BPE_CTRL_DATA_RULE_MAX_CNS)
* @param[in] qosMapPtr                -   qos map
* @param[in] encapsulationType        -   encapsulation type
* @param[in] maskPtr                  -   mask
* @param[in] patternPtr               -   pattern
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxHalBpeDataControlQosRuleAdd
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  GT_U32                                      ruleIndex,
    IN  CPSS_PX_HAL_BPE_QOSMAP_DATA_STC            *qosMapPtr,
    IN  CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_TYPE_ENT   encapsulationType,
    IN  CPSS_PX_HAL_BPE_CTRL_DATA_CLASSIFIER_STC   *maskPtr,
    IN  CPSS_PX_HAL_BPE_CTRL_DATA_CLASSIFIER_STC   *patternPtr
);

/**
* @internal cpssPxHalBpeDataControlQosRuleDelete function
* @endinternal
*
* @brief   Delete the rule with the given index.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -   device number
* @param[in] ruleIndex                -   index of the rule (0 .. CPSS_PX_HAL_BPE_CTRL_DATA_RULE_MAX_CNS)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxHalBpeDataControlQosRuleDelete
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_U32                      ruleIndex
);

/**
* @internal cpssPxHalBpeSendPacketToInterface function
* @endinternal
*
* @brief   Send a packet to egress interface
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
*                                      pcktParamsPtr- The internal packet params to be set into the packet
*                                      descriptors.
* @param[in] buffList[]               - The packet data buffers list.
* @param[in] buffLenList[]            - A list of the buffers len in buffList.
* @param[in] numOfBufs                - Length of buffList.
*
* @retval GT_OK                    - on success, or
* @retval GT_NO_RESOURCE           - if there is not enough free elements in the fifo
*                                       associated with the Event Request Handle.
* @retval GT_EMPTY                 - if there are not enough descriptors to do the sending.
* @retval GT_HW_ERROR              - when after transmission last descriptor own bit wasn't
*                                       changed for long time.
* @retval GT_BAD_PARAM             - the data buffer is longer than allowed.
*                                       Buffer data can occupied up to the maximum number of descriptors defined.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                       GT_FAIL otherwise.
*/
GT_STATUS cpssPxHalBpeSendPacketToInterface
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32           txQueue,
    IN  CPSS_PX_HAL_BPE_INTERFACE_INFO_STC *egressInterface,
    IN  GT_U8            *buffList[],
    IN  GT_U32           buffLenList[],
    IN  GT_U32           numOfBufs
);

/**
* @internal cpssPxHalBpeRxPacketGet function
* @endinternal
*
* @brief   This function returns packets from PP destined to the CPU port
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] rxQueue                  - The queue from which this packet was received (APPLICABLE RANGES: 0..7).
* @param[in,out] numOfBuffPtr             Num of buffs in packetBuffsArrPtr.
*
* @param[out] ingressInterface         - the interface packet was recieved
* @param[in,out] numOfBuffPtr             - Num of used buffs in packetBuffsArrPtr.
* @param[out] packetBuffsArrPtr[]      - (pointer to)The received packet buffers list.
* @param[out] buffLenArr[]             - List of buffer lengths for packetBuffsArrPtr.
*
* @retval GT_NO_MORE               - no more packets on the device/queue
* @retval GT_OK                    - packet got with no error.
* @retval GT_FAIL                  - failed to get the packet
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - wrong devNum,portNum,rxQueue.
* @retval GT_NOT_INITIALIZED       - the library was not initialized
* @retval GT_DSA_PARSING_ERROR     - DSA tag parsing error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxHalBpeRxPacketGet
(
    IN  GT_SW_DEV_NUM                        devNum,
    IN  GT_U32                               rxQueue,
    OUT  CPSS_PX_HAL_BPE_INTERFACE_INFO_STC  *ingressInterface,
    INOUT GT_U32                             *numOfBuffPtr,
    OUT GT_U8                                *packetBuffsArrPtr[],
    OUT GT_U32                               buffLenArr[]
);

/**
* @internal cpssPxHalBpeRxBufFree function
* @endinternal
*
* @brief   Frees a list of buffers, that where previously passed to the upper layer
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number through which these buffers where
*                                      received.
* @param[in] rxQueue                  - The Rx queue number through which these buffers where
*                                      received (APPLICABLE RANGES: 0..7).
* @param[in] rxBuffList[]             - List of Rx buffers to be freed.
* @param[in] buffListLen              - Length of rxBufList.
*                                       GT_OK on success, or
*                                       GT_FAIL otherwise.
*
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxHalBpeRxBufFree
(
    IN GT_SW_DEV_NUM    devNum,
    IN GT_U32           rxQueue,
    IN GT_U8            *rxBuffList[],
    IN GT_U32           buffListLen
);

/**
* @internal cpssPxHalBpeCncDump function
* @endinternal
*
* @brief   Display CNC information for all clients.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -   device number
* @param[in] displayAll               -   GT_TRUE:  display all entries
*                                      GT_FALSE: display only valid entries
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - cpssPxCncUploadInit not called
* @retval GT_BAD_STATE             - if the previous CNC upload in process.
* @retval GT_NOT_SUPPORTED         - block upload operation not supported.
* @retval GT_NO_MORE               - the action succeeded and there are no more waiting
*                                       CNC counter value
* @retval GT_NOT_READY             - Upload started after upload trigger or
*                                       continued after queue rewind but yet
*                                       not paused due to queue full and yet not finished.
*                                       The part of queue memory does not contain uploaded counters yet.
*                                       No counters retrieved.
*/
GT_STATUS cpssPxHalBpeCncDump
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         displayAll
);

/**
* @internal cpssPxHalBpeIngressTablesDump function
* @endinternal
*
* @brief   Display Ingress tables information.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -   device number
* @param[in] displayAll               -   GT_TRUE:  display all entries
*                                      GT_FALSE: display only valid entries
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxHalBpeIngressTablesDump
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         displayAll
);

/**
* @internal cpssPxHalBpeIngressErrorsDump function
* @endinternal
*
* @brief   Display Ingress Errors information.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -   device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxHalBpeIngressErrorsDump
(
    IN  GT_SW_DEV_NUM   devNum
);

/**
* @internal cpssPxHalBpeEgressTablesDump function
* @endinternal
*
* @brief   Display Egress tables information.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -   device number
* @param[in] displayAll               -   GT_TRUE:  display all entries
*                                      GT_FALSE: display only valid entries
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the port was not initialized with valid info
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxHalBpeEgressTablesDump
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         displayAll
);

/**
* @internal cpssPxHalBpeCosTablesDump function
* @endinternal
*
* @brief   Display COS tables information.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -   device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the port was not initialized with valid info
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
*/
GT_STATUS cpssPxHalBpeCosTablesDump
(
    IN  GT_SW_DEV_NUM   devNum
);

/**
* @internal cpssPxHalBpeIngressHashTablesDump function
* @endinternal
*
* @brief   Display Ingress Hash tables information.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -   device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the port was not initialized with valid info
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
*/
GT_STATUS cpssPxHalBpeIngressHashTablesDump
(
    IN  GT_SW_DEV_NUM   devNum
);

/**
* @internal cpssPxHalBpeInternalDbDump function
* @endinternal
*
* @brief   Display Internal Database information.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -   device number
* @param[in] displayAll               -   GT_TRUE:  display all entries
*                                      GT_FALSE: display only valid entries
*
* @retval GT_OK                    - on success
*/
GT_STATUS cpssPxHalBpeInternalDbDump
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         displayAll
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssPxHalBpeh */







