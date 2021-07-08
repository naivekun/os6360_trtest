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
* @file cpssDxChNetIfTypes.h
* @version   22
********************************************************************************
*/

#ifndef __cpssDxChGenNetIfTypesh
#define __cpssDxChGenNetIfTypesh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/************ Includes ********************************************************/
#include <cpss/common/cpssTypes.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>
#include <cpss/generic/cos/cpssCosTypes.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgVlan.h>

/**
* @struct CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC
 *
 * @brief parameter of the "cpu code table" entry
*/
typedef struct{

    /** @brief The Traffic Class assigned to packets with this
     *  CPU code (APPLICABLE RANGES: 0..7).
     */
    GT_U8 tc;

    /** @brief The Drop Precedence assigned to packets with this
     *  CPU code.
     */
    CPSS_DP_LEVEL_ENT dp;

    /** @brief Enables/disable the truncation of packets assigned
     *  with this CPU code to up to 128 bytes.
     *  GT_FALSE - don't truncate packets assigned with
     *  this CPU code.
     *  GT_TRUE - truncate packets assigned with this
     *  CPU code to up to 128 bytes.
     */
    GT_BOOL truncate;

    /** @brief Rate Limiting mode
     *  CPSS_NET_CPU_CODE_RATE_LIMIT_LOCAL_E =
     *  Rate Limiting is done on packets forwarded to the
     *  CPU by the local device ONLY.
     *  CPSS_NET_CPU_CODE_RATE_LIMIT_AGGREGATE_E =
     *  Rate Limiting is done on all packets forwarded to
     *  the CPU.
     */
    CPSS_NET_CPU_CODE_RATE_LIMIT_MODE_ENT cpuRateLimitMode;

    /** @brief CPU Code Rate Limiter index
     *  The index to one of the 31 or 255 rate limiters
     *  0 = This CPU Code is not bound to any Rate
     *  Limiter, thus the rate of packets with this CPU
     *  code is not limited.
     *  1-31 or 1-255 = CPU Code is bound to a Rate limiter
     *  according to this index
     *  (APPLICABLE RANGES: DxCh1, DxCh1_Diamond, DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2 1..31;
     *  Bobcat2, Caelum, Bobcat3; Aldrin2, Aldrin, AC3X 1..255)
     *  see API :
     *  cpssDxChNetIfCpuCodeRateLimiterTableSet(...)
     */
    GT_U32 cpuCodeRateLimiterIndex;

    /** @brief Index of one of the 32 or 256 statistical rate
     *  limiters for this CPU Code .
     *  0-31 or 0-255 = CPU Code is bound to a statistical Rate
     *  limiter according to this index
     *  (APPLICABLE RANGES: DxCh1, DxCh1_Diamond, DxCh2, DxCh3, xCat, xCat3, Lion, xCat2, Lion2 0..31;
     *  Bobcat2, Caelum, Bobcat3; Aldrin2, Aldrin, AC3X 0..255)
     *  see API :
     *  cpssDxChNetIfCpuCodeStatisticalRateLimitsTableSet(...)
     */
    GT_U32 cpuCodeStatRateLimitIndex;

    /** @brief The target device index for packets with this
     *  CPU code.
     *  0 = Packets with this CPU code are forwarded to
     *  the CPU attached to the local device.
     *  1-7 = Packets with this CPU code are forwarded
     *  to CPU attached to the device indexed by
     *  this field
     *  see API :
     *  cpssDxChNetIfCpuCodeDesignatedDeviceTableSet(...)
     */
    GT_U32 designatedDevNumIndex;

} CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC;

/**
* @struct CPSS_DXCH_NET_SDMA_RX_COUNTERS_STC
 *
 * @brief information about Rx counters -- per Queue (traffic class)
*/
typedef struct{

    /** Packets received on this queue since last read */
    GT_U32 rxInPkts;

    /** Octets received on this queue since last read */
    GT_U32 rxInOctets;

} CPSS_DXCH_NET_SDMA_RX_COUNTERS_STC;

/* deprecated legacy names that should not be used ,
    and replaced by CPSS_CSCD_PORT_DSA_MODE_1_WORD_E ,
    and CPSS_CSCD_PORT_DSA_MODE_2_WORDS_E*/
enum{
    CPSS_DXCH_NET_DSA_TYPE_REGULAR_E = 0,
    CPSS_DXCH_NET_DSA_TYPE_EXTENDED_E  = 1
};

/**
* @enum CPSS_DXCH_NET_DSA_TYPE_ENT
 *
 * @brief Enumeration of DSA tag types
*/
typedef enum{

    /** regular DSA tag - single word - 4 bytes */
    CPSS_DXCH_NET_DSA_1_WORD_TYPE_ENT = CPSS_DXCH_NET_DSA_TYPE_REGULAR_E,

    /** extended DSA tag- two words - 8 bytes */
    CPSS_DXCH_NET_DSA_2_WORD_TYPE_ENT = CPSS_DXCH_NET_DSA_TYPE_EXTENDED_E,

    /** extended DSA tag- 3 words - 12 bytes */
    CPSS_DXCH_NET_DSA_3_WORD_TYPE_ENT,

    /** extended DSA tag- 4 words - 16 bytes */
    CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT

} CPSS_DXCH_NET_DSA_TYPE_ENT;

/**
* @enum CPSS_DXCH_NET_DSA_CMD_ENT
 *
 * @brief Enumeration of DSA tag commands
*/
typedef enum{

    /** DSA command is "To CPU" */
    CPSS_DXCH_NET_DSA_CMD_TO_CPU_E = 0 ,

    /** DSA command is "FROM CPU" */
    CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E    ,

    /** DSA command is "TO Analyzer" */
    CPSS_DXCH_NET_DSA_CMD_TO_ANALYZER_E ,

    /** DSA command is "FORWARD" */
    CPSS_DXCH_NET_DSA_CMD_FORWARD_E

} CPSS_DXCH_NET_DSA_CMD_ENT;

/**
* @struct CPSS_DXCH_NET_DSA_COMMON_STC
 *
 * @brief structure of common parameters in all DSA tag formats
*/
typedef struct{

    /** DSA type regular/extended */
    CPSS_DXCH_NET_DSA_TYPE_ENT dsaTagType;

    /** the value vlan priority tag (APPLICABLE RANGES: 0..7) */
    GT_U8 vpt;

    /** CFI bit of the vlan tag (APPLICABLE RANGES: 0..1) */
    GT_U8 cfiBit;

    /** @brief the value of vlan id
     *  (APPLICABLE RANGES: DxCh1, DxCh2, DxCh3, xCat, xCat3, Lion, Lion2 0..4095)
     *  (APPLICABLE RANGES: Bobcat2, Caelum, Bobcat3; Aldrin2; Falcon, Aldrin, AC3X 0..0xFFFF)
     */
    GT_U16 vid;

    /** @brief Drop packet on its source device indicator,
     *  for Fast Stack Fail over Recovery Support.
     *  GT_TRUE: If packet is looped (packetIsLooped = 1)and
     *  SrcDev = OwnDev then packet is dropped
     *  GT_FALSE: in other cases
     *  (relevant only for CH3 devices)
     */
    GT_BOOL dropOnSource;

    /** @brief Packet is looped indicator, for Fast Stack Fail
     *  over Recovery Support.
     *  GT_FALSE: Packet is not looped and is being routed via its
     *  designated route in the Stack.
     *  GT_TRUE: Packet is looped and its being routed on an
     *  alternative route in the Stack.
     *  (relevant only for CH3 devices).
     */
    GT_BOOL packetIsLooped;

} CPSS_DXCH_NET_DSA_COMMON_STC;

/**
* @struct CPSS_DXCH_NET_DSA_FROM_CPU_STC
 *
 * @brief structure of "from CPU" DSA tag parameters
*/
typedef struct{
    CPSS_INTERFACE_INFO_STC         dstInterface; /* vid/vidx/port */
    GT_U8                           tc;
    CPSS_DP_LEVEL_ENT               dp;
    GT_BOOL                         egrFilterEn;
    GT_BOOL                         cascadeControl;
    GT_BOOL                         egrFilterRegistered;

    GT_U32                          srcId;
    GT_HW_DEV_NUM                   srcHwDev;

    union{
        struct{
            GT_BOOL                         excludeInterface;
            CPSS_INTERFACE_INFO_STC         excludedInterface; /* port/trunk */
            GT_BOOL                         mirrorToAllCPUs;

            /* supported in extended, 4 words,  DSA tag */
            GT_BOOL                         excludedIsPhyPort;
            CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT   srcIsTagged;

        }multiDest;

        struct{
            GT_BOOL                     dstIsTagged;
            GT_BOOL                     mailBoxToNeighborCPU;
        }devPort;
    }extDestInfo;

   /* supported in extended, 4 words,  DSA tag */
    GT_BOOL                         isTrgPhyPortValid;
    GT_PORT_NUM                     dstEport; /* for port but not for vid/vidx */
    GT_U32                          tag0TpidIndex;

}CPSS_DXCH_NET_DSA_FROM_CPU_STC;

/**
* @struct CPSS_DXCH_NET_DSA_FORWARD_STC
 *
 * @brief structure of "forward" DSA tag parameters
*/
typedef struct{
    CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT srcIsTagged;
    GT_HW_DEV_NUM                   srcHwDev;
    GT_BOOL                         srcIsTrunk;
    union
    {
        GT_TRUNK_ID                 trunkId;
        GT_PORT_NUM                 portNum;
    }source;

    GT_U32                          srcId;

    GT_BOOL                         egrFilterRegistered;
    GT_BOOL                         wasRouted;
    GT_U32                          qosProfileIndex;

    CPSS_INTERFACE_INFO_STC         dstInterface; /* vid/vidx/port */

   /* supported in extended, 4 words,  DSA tag */
    GT_BOOL                         isTrgPhyPortValid;
    GT_PORT_NUM                     dstEport; /* for port but not for vid/vidx */
    GT_U32                          tag0TpidIndex;
    GT_BOOL                         origSrcPhyIsTrunk;
    union
    {
        GT_TRUNK_ID                 trunkId;
        GT_PHYSICAL_PORT_NUM        portNum;
    }origSrcPhy;
    GT_BOOL                         phySrcMcFilterEnable;
    GT_U32                          hash;
    /** @brief Skip automatic Source MAC Address learning by receiving device
     *  GT_FALSE: Do learning. GT_TRUE: Skip learning.
     *  (APPLICABLE DEVICES: Falcon)
     */
    GT_BOOL                         skipFdbSaLookup;
}CPSS_DXCH_NET_DSA_FORWARD_STC;

/**
* @struct CPSS_DXCH_NET_DSA_TO_ANALYZER_STC
 *
 * @brief structure of "to analyzer" DSA tag parameters
*/
typedef struct{
    GT_BOOL                         rxSniffer;

    GT_BOOL                         isTagged;
    struct
    {
        GT_HW_DEV_NUM               hwDevNum;
        GT_PHYSICAL_PORT_NUM        portNum;

        /* supported in extended, 4 words,  DSA tag */
        GT_PORT_NUM                 ePort;
    }devPort;

    /* supported in extended, 4 words,  DSA tag */
    CPSS_INTERFACE_TYPE_ENT         analyzerTrgType;

    union{
        struct{
            GT_U16                          analyzerEvidx;
        }multiDest;

        struct{
            GT_BOOL                         analyzerIsTrgPortValid;
            GT_HW_DEV_NUM                   analyzerHwTrgDev;
            GT_PHYSICAL_PORT_NUM            analyzerTrgPort;
            GT_PORT_NUM                     analyzerTrgEport;
        }devPort;
    }extDestInfo;

    GT_U32                          tag0TpidIndex;

}CPSS_DXCH_NET_DSA_TO_ANALYZER_STC;

/**
* @struct CPSS_DXCH_NET_DSA_TO_CPU_STC
 *
 * @brief structure of "To CPU" DSA tag parameters
*/
typedef struct{
   GT_BOOL                          isEgressPipe;

   GT_BOOL                          isTagged;
   GT_HW_DEV_NUM                    hwDevNum;
   GT_BOOL                          srcIsTrunk;
   struct
   {
       GT_TRUNK_ID                  srcTrunkId;
       GT_PHYSICAL_PORT_NUM         portNum;
      /* supported in extended, 4 words,  DSA tag */
       GT_PORT_NUM                  ePort;
   }interface;

   CPSS_NET_RX_CPU_CODE_ENT         cpuCode;
   GT_BOOL                          wasTruncated;
   GT_U32                           originByteCount;
   GT_U32                           timestamp;

   /* supported in extended, 4 words,  DSA tag */
   GT_BOOL                          packetIsTT;
   union
   {
       GT_U32                           flowId;
       GT_U32                           ttOffset;
   }flowIdTtOffset;

   GT_U32                           tag0TpidIndex;

}CPSS_DXCH_NET_DSA_TO_CPU_STC;

/**
* @struct CPSS_DXCH_NET_SDMA_TX_PARAMS_STC
 *
 * @brief structure of Tx parameters , that should be set to PP when using
 * the SDMA channel
*/
typedef struct{

    /** @brief GT_TRUE
     *  GT_FALSE - leave packet unchanged.
     *  NOTE : The DXCH device always add 4 bytes of CRC when need
     *  to recalcCrc = GT_TRUE
     */
    GT_BOOL recalcCrc;

    /** the queue that packet should be sent to CPU port. (APPLICABLE RANGES: 0..7) */
    GT_U8 txQueue;

    /** @brief The application handle got from cpssEventBind for
     *  CPSS_PP_TX_BUFFER_QUEUE_E events.
     */
    GT_UINTPTR evReqHndl;

    /** @brief invoke Tx buffer Queue event.
     *  when the SDMA copies the buffers of the packet from the CPU ,
     *  the PP may invoke the event of CPSS_PP_TX_BUFFER_QUEUE_E.
     *  this event notify the CPU that the Tx buffers of the packet can
     *  be reused by CPU for other packets. (this in not event of
     *  CPSS_PP_TX_END_E that notify that packet left the SDMA)
     *  The invokeTxBufferQueueEvent parameter control the invoking of
     *  that event for this Packet.
     *  GT_TRUE - PP will invoke the event when buffers are copied (for
     *  this packet's buffers).
     *  GT_FALSE - PP will NOT invoke the event when buffers are copied.
     *  (for this packet's buffers).
     *  NOTE :
     *  when the sent is done as "Tx synchronic" this parameter
     *  IGNORED (the behavior will be as GT_FALSE)
     */
    GT_BOOL invokeTxBufferQueueEvent;

} CPSS_DXCH_NET_SDMA_TX_PARAMS_STC;

/**
* @struct CPSS_DXCH_NET_DSA_PARAMS_STC
 *
 * @brief structure of DSA parameters , that contain full information on
 * DSA for RX,Tx packets to/from CPU from/to PP.
*/
typedef struct{
    CPSS_DXCH_NET_DSA_COMMON_STC  commonParams;

    CPSS_DXCH_NET_DSA_CMD_ENT           dsaType;
    union{
        CPSS_DXCH_NET_DSA_TO_CPU_STC         toCpu;
        CPSS_DXCH_NET_DSA_FROM_CPU_STC       fromCpu;
        CPSS_DXCH_NET_DSA_TO_ANALYZER_STC    toAnalyzer;
        CPSS_DXCH_NET_DSA_FORWARD_STC        forward;
    }dsaInfo;
}CPSS_DXCH_NET_DSA_PARAMS_STC;



/**
* @struct CPSS_DXCH_NET_TX_PARAMS_STC
 *
 * @brief structure of Tx parameters , that contain full information on
 * how to send TX packet from CPU to PP.
*/
typedef struct{

    /** @brief the packet (buffers) contain the vlan TAG inside.
     *  GT_TRUE - the packet contain the vlan TAG inside.
     *  GT_FALSE - the packet NOT contain the vlan TAG inside.
     */
    GT_BOOL packetIsTagged;

    /** @brief The user's data to be returned to the Application when in
     *  the Application's buffers can be re-used .
     */
    GT_PTR cookie;

    /** the info needed when send packets using the SDMA. */
    CPSS_DXCH_NET_SDMA_TX_PARAMS_STC sdmaInfo;

    /** the DSA parameters */
    CPSS_DXCH_NET_DSA_PARAMS_STC dsaParam;

} CPSS_DXCH_NET_TX_PARAMS_STC;

/**
* @struct CPSS_DXCH_NET_RX_PARAMS_STC
 *
 * @brief structure of Rx parameters , that contain full information on
 * how packet was received from PP to CPU port.
*/
typedef struct{

    /** the DSA parameters */
    CPSS_DXCH_NET_DSA_PARAMS_STC dsaParam;

} CPSS_DXCH_NET_RX_PARAMS_STC;


/**
* @enum CPSS_DXCH_NET_PCKT_ENCAP_ENT
 *
 * @brief Defines the different transmitted packet encapsulations.
*/
typedef enum{

    /** Non - control packet. */
    CPSS_DXCH_NET_REGULAR_PCKT_E  = 0,

    /** Control packet. */
    CPSS_DXCH_NET_CONTROL_PCKT_E  = 7

} CPSS_DXCH_NET_PCKT_ENCAP_ENT;


/**
* @enum CPSS_DXCH_NET_MAC_TYPE_ENT
 *
 * @brief Defines the different Mac-Da types of a transmitted packet.
*/
typedef enum{

    /** MAC_DA[0] = 1'b0 */
    CPSS_DXCH_NET_UNICAST_MAC_E,

    /** MAC_DA[0] = 1'b1 */
    CPSS_DXCH_NET_MULTICAST_MAC_E,

    /** MAC_DA = 0xFFFFFFFF */
    CPSS_DXCH_NET_BROADCAST_MAC_E

} CPSS_DXCH_NET_MAC_TYPE_ENT;

/**
* @enum CPSS_DXCH_NET_RESOURCE_ERROR_MODE_ENT
 *
 * @brief Defines the behavior in case of Rx SDMA resource error
*/
typedef enum{

    /** the packet remains scheduled for transmission. */
    CPSS_DXCH_NET_RESOURCE_ERROR_MODE_RETRY_E,

    /** the packet is dropped. */
    CPSS_DXCH_NET_RESOURCE_ERROR_MODE_ABORT_E

} CPSS_DXCH_NET_RESOURCE_ERROR_MODE_ENT;

/**
* @enum CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_ENT
 *
 * @brief Defines the rate mode for Tx SDMA working as packet Generator
*/
typedef enum{

    /** rate is measured as packets per second. */
    CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_GLOBAL_THROUGHPUT_E,

    /** interval between successive transmissions of the same packet. */
    CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_PER_FLOW_INTERVAL_E,

    /** no interval between successive transmissions. */
    CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_NO_GAP_E

} CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_ENT;

/**
* @enum CPSS_DXCH_NET_TX_GENERATOR_BURST_STATUS_ENT
 *
 * @brief Defines Tx SDMA Generator packet burst counter status.
*/
typedef enum{

    /** packet counter is idle. */
    CPSS_DXCH_NET_TX_GENERATOR_BURST_STATUS_IDLE_E,

    /** packet counter is running and has not yet reached its configurable limit. */
    CPSS_DXCH_NET_TX_GENERATOR_BURST_STATUS_RUN_E,

    /** packet counter has reached its configurable limit. */
    CPSS_DXCH_NET_TX_GENERATOR_BURST_STATUS_LIMIT_E,

    /** packet counter has been terminated by the CPU. */
    CPSS_DXCH_NET_TX_GENERATOR_BURST_STATUS_TERMINATE_E

} CPSS_DXCH_NET_TX_GENERATOR_BURST_STATUS_ENT;


/**
* @enum CPSS_DXCH_NET_TO_CPU_FLOW_ID_OR_TT_OFFSET_MODE_ENT
 *
 * @brief Defines TO_CPU dsa tag field <Flow-ID/TT Offset> to hold 'flowid'
 * or 'ttOffset'.
 * related to CPSS_DXCH_NET_DSA_TO_CPU_STC::flowIdTtOffset
*/
typedef enum{

    /** use 'flowid' in <Flow-ID/TT Offset> */
    CPSS_DXCH_NET_TO_CPU_FLOW_ID_MODE_E,

    /** @brief use 'TT Offset' in <Flow-ID/TT Offset>
     *  NOTE: In this mode if 'non TT' will hold 'flowid'.
     */
    CPSS_DXCH_NET_TO_CPU_TT_OFFSET_MODE_E

} CPSS_DXCH_NET_TO_CPU_FLOW_ID_OR_TT_OFFSET_MODE_ENT;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif   /* __cpssDxChGenNetIfTypesh */

