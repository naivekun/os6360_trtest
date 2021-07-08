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
* @file tgfTrafficGenerator.h
*
* @brief - emulate traffic generator capabilities
* reset counters
* read counters
* send traffic
* capture received traffic
* set trigger on received traffic
* ...
*
* @version   31
********************************************************************************
*/
#ifndef __tgfTrafficGeneratorh
#define __tgfTrafficGeneratorh

#include <cpss/common/port/cpssPortStat.h>
#include <extUtils/trafficEngine/tgfTrafficEngine.h>
#ifdef CHX_FAMILY
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>
#include <cpss/generic/cscd/cpssGenCscd.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>
#else
 #ifndef CPSS_CSCD_PORT_TYPE_ENT_DEFINED
  #define CPSS_CSCD_PORT_TYPE_ENT_DEFINED
typedef enum { CPSS_CSCD_PORT_TYPE_ENT_E } CPSS_CSCD_PORT_TYPE_ENT;
 #endif
typedef enum { CPSS_ETHER_MODE_ENT_E } CPSS_ETHER_MODE_ENT;
#endif /*CHX_FAMILY*/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* default device number */
extern __THREAD_LOCAL GT_U8    prvTgfDevNum; /* = 0; */
/* do we sent traffic to the CPU ? */
extern GT_U32  tgfTrafficGeneratorRxCaptureNum;
/* indication that a test set traffic to be sent to the CPU , but not relate to 'capture' */
extern GT_BOOL tgfTrafficGeneratorExpectTraficToCpuEnabled;
/* number of packets that returned 'not GT_OK' on send */
extern GT_U32  prvTgfTrafficGeneratorNumberPacketsSendFailed;
/* number of packets that returned 'GT_OK' on send */
extern GT_U32  prvTgfTrafficGeneratorNumberPacketsSendOk;
/* device that is used to send packets from CPU */
extern GT_U8   prvTgfCpuDevNum;
/* flag to use contineouse FWS */
extern GT_U32 prvTgfFwsUse;

/* max ports for API : prvTgfCommonPortsCntrRateGet_byDevPort() */
#define GET_RATE_MAX_PORTS_CNS   128

/******************************************************************************\
 *                          Enumerations definitions                          *
\******************************************************************************/

/**
* @enum PRV_TGF_LOOPBACK_UNIT_ENT
 *
 * @brief In which unit define loopback on port
*/
typedef enum{

    /**  make loopback in MAC unit */
    PRV_TGF_LOOPBACK_UNIT_MAC_E,

    /**  make loopback in SerDes unit */
    PRV_TGF_LOOPBACK_UNIT_SERDES_E,

    /**  for validity checks */
    PRV_TGF_LOOPBACK_UNIT_MAX_E

} PRV_TGF_LOOPBACK_UNIT_ENT;

extern PRV_TGF_LOOPBACK_UNIT_ENT prvTgfLoopbackUnit;

/**
* @enum TGF_PACKET_TYPE_ENT
 *
 * @brief Enumeration of packet types that receive in the CPU table
*/
typedef enum{

    /** @brief the packet received in the CPU ,
     *  was trapped/mirrored/forwarded due test configuration.
     */
    TGF_PACKET_TYPE_REGULAR_E,

    /** @brief the packet received in the CPU ,
     *  was trapped by the 'Cpture' emulation , that we supply in order to
     *  capture traffic that egress a port.
     */
    TGF_PACKET_TYPE_CAPTURE_E,

    /** any kind regular and capture */
    TGF_PACKET_TYPE_ANY_E

} TGF_PACKET_TYPE_ENT;


/**
* @enum TGF_CAPTURE_MODE_ENT
 *
 * @brief Enumeration of packet capture mode
*/
typedef enum{

    /** @brief packet is captured by (ingress) PCL
     *  (the egress port is defined 'loopback' and
     *  after pack looped it is send to CPU by IPCL)
     */
    TGF_CAPTURE_MODE_PCL_E,

    /** @brief packet is captured by (ingress) mirroring to cpu port
     *  (the egress port is defined 'loopback' and
     *  after pack looped it is ingress mirrored to the CPU)
     */
    TGF_CAPTURE_MODE_MIRRORING_E

} TGF_CAPTURE_MODE_ENT;

/**
* @enum PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT
 *
 * @brief Type of access to Ingress / Egress PCL configuration Table
 * Enumerators:
 * PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E - by PORT access mode
 * PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_VLAN_E - by VLAN access mode
 * PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_EPORT_E - by ePORT access mode (12 LSBits)
*/
typedef enum{

    PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E,

    PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_VLAN_E,

    PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_EPORT_E

} PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT;

/**
* @enum PRV_TGF_TX_CONT_MODE_UNIT_ENT
 *
 * @brief The Tx continuouse mode units type.
*/
typedef enum{

    /** wirespeed traffic with port's line rate */
    PRV_TGF_TX_CONT_MODE_UNIT_WS_E,

    /** kilo bits per second rate units */
    PRV_TGF_TX_CONT_MODE_UNIT_KBPS_SEC_E,

    /** percent of line rate units */
    PRV_TGF_TX_CONT_MODE_UNIT_PERCENT_E

} PRV_TGF_TX_CONT_MODE_UNIT_ENT;

/*
    port interface type used by :
        tgfTrafficGeneratorPortTxEthTriggerCountersGet
        tgfTrafficGeneratorPortTxEthCaptureCompare
        tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet

    PRV_TGF_EXPECTED_INTERFACE_TYPE_PHYSICAL_PORT_NUM_E - physical port number
                                                        used when 'eport' not defined
    PRV_TGF_EXPECTED_INTERFACE_TYPE_EPORT_NUM_E         - eport number
                                                        used when 'eport' defined or
                                                        IMPLICITLY when 'TO_CPU' dsa tag
                                                        not hold enough bits for 'srcPhyPort' so need to use the 'srcEport'
    PRV_TGF_EXPECTED_INTERFACE_TYPE_FORCE_PHYSICAL_PORT_NUM_E -
                                                        force use of physical port number , regardless to 'eport' logic !
*/
typedef enum{
    PRV_TGF_EXPECTED_INTERFACE_TYPE_PHYSICAL_PORT_NUM_E,
    PRV_TGF_EXPECTED_INTERFACE_TYPE_EPORT_NUM_E,

    PRV_TGF_EXPECTED_INTERFACE_TYPE_FORCE_PHYSICAL_PORT_NUM_E

}PRV_TGF_EXPECTED_INTERFACE_TYPE_ENT;




/**
* @struct PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC
 *
 * @brief used to describe Analyzer interface.
*/
typedef struct{

    /** @brief Analyzer interface.
     *  Only CPSS_INTERFACE_PORT_E interface type is supported.
     */
    CPSS_INTERFACE_INFO_STC interface;

} PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC;

/**
* @struct PRV_TGF_TX_DEV_PORT_STC
 *
 * @brief continuous mode DB entry.
*/
typedef struct{

    /** device number of Tx port */
    GT_U8 devNum;

    /** port number of Tx port */
    GT_PHYSICAL_PORT_NUM portNum;

    /** Tx units type */
    PRV_TGF_TX_CONT_MODE_UNIT_ENT unitsType;

    /** Tx units value */
    GT_U32 units;

    /** @brief analyzer interface to restore after stop traffic
     *  Comment:
     */
    PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC analyzerInf;

} PRV_TGF_TX_DEV_PORT_STC;


/**
* @enum PRV_TGF_TX_MODE_ENT
 *
 * @brief The Tx mode of packets engine.
*/
typedef enum{

    /** @brief packet engine sends single burst of
     *  packets. The prvTgfSetTxSetupEth configures
     *  burst size.
     */
    PRV_TGF_TX_MODE_SINGLE_BURST_E,

    /** @brief packet engine send packets continuously.
     *  The prvTgfStopTransmitingEth stops transmission.
     */
    PRV_TGF_TX_MODE_CONTINUOUS_E,

    /** @brief packet engine send packets continuously.
     *  The prvTgfStopTransmitingEth stops transmission.
     */
    PRV_TGF_TX_MODE_CONTINUOUS_FWS_E

} PRV_TGF_TX_MODE_ENT;

/* number of packets to get WS traffic. */
extern GT_U32 prvTgfWsBurst;

/* API to configure prvTgfWsBurst */
extern GT_U32 prvTgfWsBurstSet(GT_U32 newVal);

/* Tx ports maximal amount for continuous mode.
  The limitation is 7 Analyzer interfaces in source based mode. */
#define PRV_TGF_TX_PORTS_MAX_NUM_CNS 7

/******************************************************************************\
 *                               Struct definitions                           *
\******************************************************************************/
#define PRV_TGF_MAX_PORTS_NUM_CNS               20

/* ports array */
extern GT_U32    prvTgfPortsArray[PRV_TGF_MAX_PORTS_NUM_CNS];/* = {0, 8, 18, 23}; */

/*
    structure: PRV_TGF_DEV_PORT_STC
    for dev,port pair
*/
typedef struct{
    GT_U8           devNum;
    GT_PORT_NUM             portNum;   /* in some case the port is 'physical' on others 'ePort' */
}PRV_TGF_DEV_PORT_STC;

/*
    structure: PRV_TGF_MEMBER_FORCE_INFO_STC

    support ring ports that need to be in all vlan/mc groups.
        NOTE: in lion A those port must NOT be in vidx !
    support cascade ports that need to be in all vlan/mc groups.

    fields:
    member - {dev,port} that we refer to.
    forceToVlan - do want to force the member to vlan.
    forceToVidx - do we want to force the member to vidx.
    vlanInfo - info relate only to vlans.
        .tagged - the member tagged or untagged.
            relevant to devices without tr101 support.
        .tagCmd - tag command. the member's tag command.
            relevant to devices with tr101 support.
            hold value of one of : PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT

*/
typedef struct{
    PRV_TGF_DEV_PORT_STC    member;
    GT_BOOL     forceToVlan;
    GT_BOOL     forceToVidx;
    struct{
        GT_BOOL                                        tagged;
        GT_U32 /*PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT*/   tagCmd;
    }vlanInfo;
}PRV_TGF_MEMBER_FORCE_INFO_STC;

/* max number of ports in devices that must be added to all vlans and MC (vidx) groups */
#define PRV_TGF_FORCE_MEMBERS_COUNT_CNS     128

/* list of ports in devices that must be added to all vlans and MC (vidx) groups */
extern PRV_TGF_MEMBER_FORCE_INFO_STC  prvTgfDevPortForceArray[/*PRV_TGF_FORCE_MEMBERS_COUNT_CNS*/];
/* number of used ports/devices in arrays of :prvTgfDevPortForceArray[] */
extern GT_U32    prvTgfDevPortForceNum;



/*
 * typedef: structure TGF_NET_SDMA_TX_PARAMS_STC
 *
 * Description: structure of Tx parameters, that should be set to PP when using
 *              the SDMA channel
 *
 * fields:
 *      recalcCrc - GT_TRUE  - the PP should add CRC to the transmitted packet,
 *                  GT_FALSE - leave packet unchanged.
 *                  NOTE : The DXCH device always add 4 bytes of CRC when need
 *                         to recalcCrc = GT_TRUE
 *
 *
 *      txQueue - the queue that packet should be sent to CPU port. (0..7)
 *
 *      evReqHndl - The application handle got from cpssEventBind for
 *                  CPSS_PP_TX_BUFFER_QUEUE_E events.
 *
 *     invokeTxBufferQueueEvent - invoke Tx buffer Queue event.
 *              when the SDMA copies the buffers of the packet from the CPU ,
 *              the PP may invoke the event of CPSS_PP_TX_BUFFER_QUEUE_E.
 *              this event notify the CPU that the Tx buffers of the packet can
 *              be reused by CPU for other packets. (this in not event of
 *              CPSS_PP_TX_END_E that notify that packet left the SDMA)
 *              The invokeTxBufferQueueEvent parameter control the invoking of
 *              that event for this Packet.
 *              GT_TRUE - PP will invoke the event when buffers are copied (for
 *                        this packet's buffers).
 *              GT_FALSE - PP will NOT invoke the event when buffers are copied.
 *                        (for this packet's buffers).
 *              NOTE :
 *                  when the sent is done as "Tx synchronic" this parameter
 *                  IGNORED (the behavior will be as GT_FALSE)
 */
typedef struct
{
    GT_BOOL                         recalcCrc;
    GT_U8                           txQueue;
    GT_UINTPTR                      evReqHndl;
    GT_BOOL                         invokeTxBufferQueueEvent;
} TGF_NET_SDMA_TX_PARAMS_STC;

/*
 * typedef: structure TGF_NET_DSA_STC
 *
 * Description: structure of specific DSA tag parameters
 *
 * fields:
 *      dsaCmdIsToCpu - indicates that the DSA command is 'to CPU'
 *      cpuCode       - CPU codes that indicate the reason for sending
 *                      a packet to the CPU.
 *      srcIsTrunk    - is source Trunk
 *      hwDevNum      - the source HW device number
 *      portNum       - the source port number
 *      originByteCount - The packet's original byte count (TO_CPU packets).
 *      trunkId       - the trunkId , valid when srcIsTrunk = GT_TRUE
 *                      (supported by eDSA info)
 *      ePortNum      -  the ePort number , valid when srcIsTrunk = GT_FALSE
 *                      (supported by eDSA info)
 */
typedef struct
{
    GT_BOOL                     dsaCmdIsToCpu;
    CPSS_NET_RX_CPU_CODE_ENT    cpuCode;
    GT_BOOL                     srcIsTrunk;
    GT_HW_DEV_NUM               hwDevNum;
    GT_PHYSICAL_PORT_NUM        portNum;
    GT_U32                      originByteCount;
    GT_TRUNK_ID                 trunkId;
    GT_PORT_NUM                 ePortNum;
} TGF_NET_DSA_STC;


/******************************************************************************\
 *                            Public API section                              *
\******************************************************************************/

/**
* @internal tgfTrafficGeneratorPacketChecksumUpdate function
* @endinternal
*
* @brief   Update checksums in packet built by tgfTrafficEnginePacketBuild
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - parser caused bad state that should not happen
*
* @note Input parameters are arrays returened by tgfTrafficEnginePacketBuild.
*
*/
GT_STATUS tgfTrafficGeneratorPacketChecksumUpdate
(
    OUT TGF_AUTO_CHECKSUM_FULL_INFO_STC *checksumInfoPtr,
    OUT TGF_AUTO_CHECKSUM_EXTRA_INFO_STC *checksumExtraInfoPtr
);

/**
* @internal tgfTrafficGeneratorCpuDeviceSet function
* @endinternal
*
* @brief   Set the CPU device though it we send traffic to the 'Loop back port'
*
* @param[in] cpuDevice                - the CPU device number
*
* @retval GT_OK                    - on success
*/
GT_STATUS tgfTrafficGeneratorCpuDeviceSet
(
    IN GT_U8    cpuDevice
);

/**
* @internal tgfTrafficGeneratorPortLoopbackModeEnableSet function
* @endinternal
*
* @brief   Set port in 'loopback' mode
*
* @param[in] portInterfacePtr         - (pointer to) port interface
* @param[in] enable                   -  / disable (loopback/no loopback)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS tgfTrafficGeneratorPortLoopbackModeEnableSet
(
    IN CPSS_INTERFACE_INFO_STC  *portInterfacePtr,
    IN GT_BOOL                   enable
);

/**
* @internal tgfTrafficGeneratorPortForceLinkWa function
* @endinternal
*
* @brief   WA for force link UP in CG ports.
*         Function manages PCS Loopback and RX enable/disable to get CG MAC working.
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   -  / disable force link UP
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong interface
*/
GT_STATUS tgfTrafficGeneratorPortForceLinkWa
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
);

/**
* @internal tgfTrafficGeneratorPortForceLinkCheckWa function
* @endinternal
*
* @brief   WA to check force link UP state for CG/MTI ports.
*         Function checks Loopback and RX and TX enable/disable to get CG/MTI MAC force link status.
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[out] enablePtr               -  (pointer to) port force link state
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad parameteres
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS tgfTrafficGeneratorPortForceLinkCheckWa
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                *enablePtr
);

/**
* @internal tgfTrafficGeneratorPortForceLinkUpEnableSet function
* @endinternal
*
* @brief   Set port in 'force linkup' mode
*
* @param[in] portInterfacePtr         - (pointer to) port interface
* @param[in] enable                   -  / disable (force/not force)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS tgfTrafficGeneratorPortForceLinkUpEnableSet
(
    IN CPSS_INTERFACE_INFO_STC  *portInterfacePtr,
    IN GT_BOOL                   enable
);

/**
* @internal tgfTrafficGeneratorPortCountersEthReset function
* @endinternal
*
* @brief   Reset the traffic generator counters on the port
*
* @param[in] portInterfacePtr         - (pointer to) port interface
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS tgfTrafficGeneratorPortCountersEthReset
(
    IN CPSS_INTERFACE_INFO_STC  *portInterfacePtr
);

/**
* @internal tgfTrafficGeneratorPortCountersEthRead function
* @endinternal
*
* @brief   Read the traffic generator counters on the port
*
* @param[in] portInterfacePtr         - (pointer to) port interface
*
* @param[out] countersPtr              - (pointer to) the counters of port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS tgfTrafficGeneratorPortCountersEthRead
(
    IN  CPSS_INTERFACE_INFO_STC        *portInterfacePtr,
    OUT CPSS_PORT_MAC_COUNTER_SET_STC  *countersPtr
);


/**
* @internal tgfStateTrgEPortInsteadPhyPort function
* @endinternal
*
* @brief   Option to state that the cpu will send packets to trgEport that is not
*         the 'physical port' (portNum) that used by prvLuaTgfTransmitPackets(..portNum..)
*         prvLuaTgfTransmitPacketsWithCapture(..inPortNum..)
*         use enable = GT_TRUE to state to start using trgEPort
*         use enable = GT_FALSE to state to start NOT using
*         trgEPort (trgEPort is ignored). use the phyPort
* @param[in] trgEPort                 - egress device number
* @param[in] enable                   - (GT_BOOL) do we use 
*                                       eport or the phy port
*                                       return_code
*/
GT_STATUS tgfStateTrgEPortInsteadPhyPort(
    IN GT_U32                trgEPort,
    IN GT_BOOL               enable
);

/**
* @internal tgfTrafficGeneratorCheckProcessignDone function
* @endinternal
*
* @brief   Check that all packets were processed by device.
*         The function checks Idle state of device - works on simulation only.
* @param[in] sleepAfterCheck          - GT_TRUE - sleep some time after Idle check
*                                      GT_FALSE - do not sleep after Idle check
* @param[in] loopCount                - number of 1 milisecond sleeps done before call of
*                                      the function. Used only if sleepAfterCheck == GT_TRUE
*
* @retval 0                        - pooling detected that system is still busy
* @retval other value              - number of pooling loops till detection of busy.
*/
GT_U32 tgfTrafficGeneratorCheckProcessignDone
(
    IN GT_BOOL  sleepAfterCheck,
    IN GT_U32   loopCount
);

/**
* @internal tgfTrafficGeneratorStormingExpected function
* @endinternal
*
* @brief   state that the test doing storming on the ingress port (ingress traffic
*         will be send back to ingress port that is in 'loopback mode').
*         this indication needed for WM to know to break the loop regardless to
*         indications from skernelStatusGet()
* @param[in] stormingExpected         - GT_TRUE - storming is expected
*                                      GT_FALSE - storming is NOT expected
*                                       nothing
*/
GT_VOID tgfTrafficGeneratorStormingExpected(IN GT_BOOL stormingExpected);

/**
* @internal tgfTrafficGeneratorPortTxEthTransmit function
* @endinternal
*
* @brief   Transmit the traffic to the port
*         NOTE:
*         1. allow to transmit with chunks of burst:
*         after x sent frames --> do sleep of y millisecond
*         see parameters
* @param[in] portInterfacePtr         - (pointer to) port interface
* @param[in] packetInfoPtr            - (pointer to) the packet info
* @param[in] burstCount               - number of frames (non-zero value)
* @param[in] numVfd                   - number of VFDs
* @param[in] vfdArray[]               - pointer to array of VFDs (may be NULL if numVfd = 0)
* @param[in] sleepAfterXCount         - do 'sleep' after X packets sent
*                                      when = 0 , meaning NO SLEEP needed during the burst
*                                      of 'burstCount'
* @param[in] sleepTime                - sleep time (in milliseconds) after X packets sent , see
*                                      parameter sleepAfterXCount
* @param[in] traceBurstCount          - number of packets in burst count that will be printed
* @param[in] loopbackEnabled          - Check RCV counters when TRUE
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note The packet will ingress to the PP after 'loopback' and will act as
*       it was received from a traffic generator connected to the port
*
*/
GT_STATUS tgfTrafficGeneratorPortTxEthTransmit
(
    IN CPSS_INTERFACE_INFO_STC         *portInterfacePtr,
    IN TGF_PACKET_STC                  *packetInfoPtr,
    IN GT_U32                           burstCount,
    IN GT_U32                           numVfd,
    IN TGF_VFD_INFO_STC                 vfdArray[],
    IN GT_U32                           sleepAfterXCount,
    IN GT_U32                           sleepTime,
    IN GT_U32                           traceBurstCount,
    IN GT_BOOL                          loopbackEnabled
);

/**
* @internal tgfTrafficGeneratorRxInCpuGet function
* @endinternal
*
* @brief   Get entry from rxNetworkIf table
*
* @param[in] packetType               - the packet type to get
* @param[in] getFirst                 - get first/next entry
* @param[in] trace                    - enable\disable packet tracing
* @param[in,out] packetBufLenPtr          - the length of the user space for the packet
*
* @param[out] packetBufPtr             - packet's buffer (pre allocated by the user)
* @param[in,out] packetBufLenPtr          - length of the copied packet to gtBuf
* @param[out] packetLenPtr             - Rx packet original length
* @param[out] devNumPtr                - packet's device number
* @param[out] queuePtr                 - Rx queue in which the packet was received.
* @param[out] rxParamsPtr              - specific device Rx info format.
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - on more entries
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note For 'captured' see API tgfTrafficGeneratorPortTxEthCaptureSet
*
*/
GT_STATUS tgfTrafficGeneratorRxInCpuGet
(
    IN    TGF_PACKET_TYPE_ENT   packetType,
    IN    GT_BOOL               getFirst,
    IN    GT_BOOL               trace,
    OUT   GT_U8                *packetBufPtr,
    INOUT GT_U32               *packetBufLenPtr,
    OUT   GT_U32               *packetLenPtr,
    OUT   GT_U8                *devNumPtr,
    OUT   GT_U8                *queuePtr,
    OUT   TGF_NET_DSA_STC      *rxParamsPtr
);

/**
* @internal tgfTrafficGeneratorPortTxEthTriggerCountersGet function
* @endinternal
*
* @brief   Get number of triggers on port/trunk
*         when 'trunk' implementation actually loops on all enabled members.
* @param[in] portInterfacePtr         - (pointer to) port/trunk interface
* @param[in] numVfd                   - number of VFDs
* @param[in] vfdArray[]               - pointer to array of VFDs
*
* @param[out] numTriggersBmpPtr        - (pointer to) bitmap of triggers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note The 'captured' (emulation of capture) must be set on this port/trunk
*       see tgfTrafficGeneratorPortTxEthCaptureSet
*       Triggers bitmap has the following structure:
*       N - number of captured packets on this port
*       K - number of VFDs (equal to numVfd)
*       |   Packet0   |   Packet1   |   |    PacketN  |
*       +----+----+---+----+----+----+---+----+ ... +----+----+---+----+
*       |Bit0|Bit1|...|BitK|Bit0|Bit1|...|BitK|   |Bit0|Bit1|...|BitK|
*       +----+----+---+----+----+----+---+----+   +----+----+---+----+
*       Bit[i] for Packet[j] set to 1 means that VFD[i] is matched for captured
*       packet number j.
*
*/
GT_STATUS tgfTrafficGeneratorPortTxEthTriggerCountersGet
(
    IN  CPSS_INTERFACE_INFO_STC         *portInterfacePtr,
    IN  GT_U32                           numVfd,
    IN  TGF_VFD_INFO_STC                 vfdArray[],
    OUT GT_U32                          *numTriggersBmpPtr
);

/**
* @internal tgfTrafficGeneratorTxEthTriggerCheck function
* @endinternal
*
* @brief   Check if the buffer triggered by the trigger parameters
*
* @param[in] bufferPtr                - (pointer to) the buffer
* @param[in] bufferLength             - length of the buffer
* @param[in] numVfd                   - number of VFDs
* @param[in] vfdArray[]               - array of VFDs
*
* @param[out] triggeredPtr             - (pointer to) triggered / not triggered
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note The 'captured' (emulation of capture) must be set on this port see
*       tgfTrafficGeneratorPortTxEthCaptureSet, tgfTrafficGeneratorRxInCpuGet
*
*/
GT_STATUS tgfTrafficGeneratorTxEthTriggerCheck
(
    IN  GT_U8                           *bufferPtr,
    IN  GT_U32                           bufferLength,
    IN  GT_U32                           numVfd,
    IN  TGF_VFD_INFO_STC                 vfdArray[],
    OUT GT_BOOL                         *triggeredPtr
);

/**
* @internal tgfTrafficGeneratorPortTxEthCaptureSet function
* @endinternal
*
* @brief   Start/Stop capture the traffic that egress the port/trunk
*         when 'trunk' implementation actually loops on all enabled members.
* @param[in] portInterfacePtr         - (pointer to) port/trunk interface
* @param[in] mode                     - packet capture mode
* @param[in] start                    - start/stop capture on this port/trunk
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note The Start capture is emulated by setting next for the port:
*       1. loopback on the port (so all traffic that egress the port will
*       ingress the same way)
*       2. ingress PCL rule for ALL traffic from this port to TRAP to the CPU
*       we will assign it with 'Application specific cpu code' to
*       distinguish from other traffic go to the CPU
*       The Stop capture is emulated by setting next for the port:
*       1. disable loopback on the port
*       2. remove the ingress PCL rule that function
*       tgfTrafficGeneratorPortTxEthCaptureStart added
*
*/
GT_STATUS tgfTrafficGeneratorPortTxEthCaptureSet
(
    IN CPSS_INTERFACE_INFO_STC  *portInterfacePtr,
    IN TGF_CAPTURE_MODE_ENT      mode,
    IN GT_BOOL                   start
);

/**
* @internal tgfTrafficGeneratorCaptureLoopTimeSet function
* @endinternal
*
* @brief   Set sleeping time before disabling packet's capturing
*
* @param[in] timeInMSec               - time in miliseconds (0 for restore defaults)
*
* @retval GT_OK                    - on success
*
* @note Needed for debugging purposes
*
*/
GT_STATUS tgfTrafficGeneratorCaptureLoopTimeSet
(
    IN GT_U32   timeInMSec
);

/**
* @internal tgfTrafficGeneratorEtherTypeForVlanTagSet function
* @endinternal
*
* @brief   set etherType to recognize 'Tagged packes' that send from the traffic
*         generator to the PP
* @param[in] etherType                - new etherType
*                                      (was initialized as 0x8100)
*                                       None
*/
GT_VOID tgfTrafficGeneratorEtherTypeForVlanTagSet
(
    IN GT_U16   etherType
);

/**
* @internal tgfTrafficGeneratorLoopTimeSet function
* @endinternal
*
* @brief   Set sleeping time after sending burst of packets from the CPU
*
* @param[in] timeInSec                - time in seconds
*
* @retval GT_OK                    - on success
*
* @note Needed for debugging purposes
*
*/
GT_STATUS tgfTrafficGeneratorLoopTimeSet
(
    IN GT_U32   timeInSec
);

/**
* @internal tgfTrafficGeneratorGmTimeFactorSet function
* @endinternal
*
* @brief   Set time factor for GM devices , due to very slow processing of
*         packets (2 per second)
* @param[in] timeFactor               - factor for the time
*
* @retval GT_OK                    - on success
*
* @note Needed for debugging purposes
*
*/
GT_STATUS tgfTrafficGeneratorGmTimeFactorSet
(
    IN GT_U32   timeFactor
);

/**
* @internal tgfTrafficGeneratorPortTxEthCapturePclExcludeVidSet function
* @endinternal
*
* @brief   do we need to capture by PCL all packets , exclude those of specific vlan ?
*         this is relevant only when 'PCL capture' used (TGF_CAPTURE_MODE_PCL_E)
* @param[in] excludeEnable            - GT_TRUE - exclude
*                                      GT_FALSE - no excluding.
* @param[in] excludedVid              - the vid to exclude .
*                                      relevant only when  excludeEnable = GT_TRUE
*
* @retval GT_OK                    - on success
*/
GT_STATUS tgfTrafficGeneratorPortTxEthCapturePclExcludeVidSet
(
    IN GT_BOOL  excludeEnable,
    IN GT_U16   excludedVid
);

/**
* @internal tgfTrafficGeneratorRxInCpuNumWait function
* @endinternal
*
* @brief   wait for ALL packets that expected in the CPU, to received.
*
* @param[in] numOfExpectedPackets     - number of packets
* @param[in] maxTimeToWait            - MAX time (in milliseconds) to wait for the packets.
*                                      The functions will return as soon as all packets
*                                      received (but not more then this max time)
*
* @param[out] timeActuallyWaitPtr      - time in milliseconds actually waited for the packets.
*                                      NOTE: this parameter can be NULL.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - not get all packets on time
* @retval GT_BAD_PARAM             - wrong numOfExpectedPackets
*/
GT_STATUS tgfTrafficGeneratorRxInCpuNumWait
(
    IN GT_U32   numOfExpectedPackets,
    IN GT_U32   maxTimeToWait,
    OUT GT_U32  *timeActuallyWaitPtr
);

/**
* @internal tgfTrafficGeneratorExpectTraficToCpu function
* @endinternal
*
* @brief   state that current test expect traffic to the CPU
*         NOTE: this function should not be called for cases that test uses
*         'capture' to the CPU.
* @param[in] enable                   - GT_TRUE    - test expect traffic to the CPU.
*                                      GT_FALSE   - test NOT expect traffic to the CPU.
*
* @retval GT_OK                    - on success
*/
GT_STATUS tgfTrafficGeneratorExpectTraficToCpu
(
    IN GT_BOOL  enable
);

/**
* @internal tgfTrafficGeneratorIngressCscdPortEnableSet function
* @endinternal
*
* @brief   enable/disable the port to assume that ingress traffic hold DSA tag
*         info.
*         NOTE: relevant only to devices that support detach of ingress and
*         egress cascade configurations.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - The port to be configured as cascade enable/disabled
* @param[in] enable                   - enable/disable the ingress DSA tag processing
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note this API needed to support testing of DSA tag processing by the device
*       (DSA that not processed on the ingress of the CPU_PORT)
*
*/
GT_STATUS tgfTrafficGeneratorIngressCscdPortEnableSet
(
    IN GT_U8                        devNum,
    IN GT_PHYSICAL_PORT_NUM         portNum,
    IN GT_BOOL                      enable
);

/**
* @internal tgfTrafficGeneratorEgressCscdPortEnableSet function
* @endinternal
*
* @brief   enable/disable the port to assume that egress traffic hold DSA tag
*         info.
*         NOTE: relevant only to devices that support detach of ingress and
*         egress cascade configurations.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - The port to be configured as cascade enable/disabled
* @param[in] enable                   - enable/disable the ingress DSA tag processing
* @param[in] cscdPortType             - type of cascade port (APPLICABLE VALUES:
*                                      CPSS_CSCD_PORT_DSA_MODE_2_WORDS_E,
*                                      CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E,
*                                      CPSS_CSCD_PORT_NETWORK_E).
*                                      Relevant only when enable is GT_TRUE.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note this API needed to support testing of DSA tag processing by the device
*       (DSA that not processed on the ingress of the CPU_PORT)
*
*/
GT_STATUS tgfTrafficGeneratorEgressCscdPortEnableSet
(
    IN GT_U8                        devNum,
    IN GT_PHYSICAL_PORT_NUM         portNum,
    IN GT_BOOL                      enable,
    IN CPSS_CSCD_PORT_TYPE_ENT      cscdPortType
);

/**
* @internal prvTgfTrafficGeneratorPacketDsaTagGetOrigSrcPhy function
* @endinternal
*
* @brief   Get original src port (it's different than src port on remote ports).
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number (either local or remote)
*                                       Original physical port number to be used at DSA tag.
*/
GT_PHYSICAL_PORT_NUM prvTgfTrafficGeneratorPacketDsaTagGetOrigSrcPhy
(
    IN  GT_U8                          devNum,
    IN  GT_PORT_NUM                    portNum
);

/**
* @internal tgfTrafficGeneratorPortTxEthCaptureCompare function
* @endinternal
*
* @brief   compare captured packets with the input packet format (+VFD array)
*         NOTE: the function not care about reason why packet reached the CPU ,
*         is it due to 'capture' or other reason.
* @param[in] portInterfacePtr         - (pointer to) port/trunk interface
*                                      NOTE: can be NULL --> meaning 'any interface'
* @param[in] packetInfoPtr            - the packet format to compare the captured packet with.
* @param[in] numOfPackets             - number of packets expected to be captured on the interface (can be 0)
* @param[in] numVfd                   - number of VFDs (can be 0)
* @param[in] vfdArray[]               - pointer to array of VFDs (can be NULL when numVfd == 0)
* @param[in] byteNumMaskList[]        - pointer to array of bytes for which the comparison
*                                      is prohibited (can be NULL when byteNumMaskListLen == 0)
*                                      supports both single number elements and
*                                      range elements in format (low | (high << 16))
* @param[in] byteNumMaskListLen       - number of members in byteNumMaskList(can be 0)
*
* @param[out] actualCapturedNumOfPacketsPtr - (pointer to) actual number of packet captured from
*                                      the interface
* @param[out] onFirstPacketNumTriggersBmpPtr - (pointer to) bitmap of triggers - that
*                                      relate only to first packet  (can be NULL when numVfd == 0)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note Triggers bitmap has the following structure:
*       K - number of VFDs (equal to numVfd)
*       |   Packet0   |
*       +----+----+---+----+
*       |Bit0|Bit1|...|BitK|
*       +----+----+---+----+
*       Bit[i] to 1 means that VFD[i] is matched for first captured packet .
*
*/
GT_STATUS tgfTrafficGeneratorPortTxEthCaptureCompare
(
    IN  CPSS_INTERFACE_INFO_STC     *portInterfacePtr,
    IN  TGF_PACKET_STC      *packetInfoPtr,
    IN  GT_U32              numOfPackets,
    IN  GT_U32              numVfd,
    IN  TGF_VFD_INFO_STC    vfdArray[],
    IN  GT_U32              byteNumMaskList[],
    IN  GT_U32              byteNumMaskListLen,
    OUT GT_U32              *actualCapturedNumOfPacketsPtr,
    OUT GT_U32              *onFirstPacketNumTriggersBmpPtr
);

/**
* @internal tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet function
* @endinternal
*
* @brief   Get entry from rxNetworkIf table , for expected interface only.
*         similar to tgfTrafficGeneratorRxInCpuGet , but filtering the packets
*         from non relevant interfaces.
* @param[in] portInterfacePtr         - (pointer to) port/trunk interface
*                                      NOTE: can be NULL --> meaning 'any interface'
* @param[in] packetType               - the packet type to get
* @param[in] getFirst                 - get first/next entry
* @param[in] trace                    - enable\disable packet tracing
* @param[in,out] packetBufLenPtr          - the length of the user space for the packet
*
* @param[out] packetBufPtr             - packet's buffer (pre allocated by the user)
* @param[in,out] packetBufLenPtr          - length of the copied packet to gtBuf
* @param[out] packetLenPtr             - Rx packet original length
* @param[out] devNumPtr                - packet's device number
* @param[out] queuePtr                 - Rx queue in which the packet was received.
* @param[out] rxParamsPtr              - specific device Rx info format.
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - on more entries
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note For 'captured' see API tgfTrafficGeneratorPortTxEthCaptureSet
*
*/
GT_STATUS tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet
(
    IN CPSS_INTERFACE_INFO_STC  *portInterfacePtr,
    IN    TGF_PACKET_TYPE_ENT   packetType,
    IN    GT_BOOL               getFirst,
    IN    GT_BOOL               trace,
    OUT   GT_U8                *packetBufPtr,
    INOUT GT_U32               *packetBufLenPtr,
    OUT   GT_U32               *packetLenPtr,
    OUT   GT_U8                *devNumPtr,
    OUT   GT_U8                *queuePtr,
    OUT   TGF_NET_DSA_STC      *rxParamsPtr
);

/**
* @internal tgfTrafficGeneratorExpectedInterfaceEport function
* @endinternal
*
* @brief   set the expected mode of 'port' to be : ePort / phyPort
*         because the CPSS_INTERFACE_INFO_STC not hold difference between eport and
*         physical port , this setting will give the functions of 'expected' packets
*         to know if expected from the eport or from the physical port.
*         NOTEs:
*         1. by defaults : physical port
*         2. after compares done , the default value restored automatically by the engine
*
* @retval GT_TRUE                  - packet came from the expected interface
* @retval GT_FALSE                 - packet came from unexpected interface
*/
GT_STATUS tgfTrafficGeneratorExpectedInterfaceEport
(
    IN PRV_TGF_EXPECTED_INTERFACE_TYPE_ENT portInterfaceType
);

/**
* @internal prvTgfTrafficPrintPacketTxEnableSet function
* @endinternal
*
* @brief   Enable or Disable printing of TX Packet event
*
* @param[in] printEnable              - GT_FALSE - disable
*                                      GT_TRUE  - enable
*                                       previous state
*/
GT_BOOL prvTgfTrafficPrintPacketTxEnableSet
(
    GT_BOOL printEnable
);

/**
* @internal prvTgfTrafficPrintPacketTxEnableGet function
* @endinternal
*
* @brief   Returns Enable or Disable printing of TX Packet event
*
* @retval GT_FALSE                 - disable
* @retval GT_TRUE                  - enable
*/
GT_BOOL prvTgfTrafficPrintPacketTxEnableGet
(
    GT_VOID
);


/**
* @internal prvTgfTrafficGeneratorNetIfSyncTxBurstSend function
* @endinternal
*
* @brief   Transmit the traffic to the port
*
* @param[in] portInterfacePtr         - (pointer to) port interface
* @param[in] bufferPtr                - (pointer to) the buffer to send
* @param[in] bufferLength             - buffer length (include CRC bytes)
* @param[in] packetsNumber            - number of packets to send
* @param[in] doForceDsa               - GT_FALSE - choose DSA tag automatically
*                                      GT_TRUE - use DSA tag defined by forcedDsa
* @param[in] forcedDsa                - DSA tag type, used when doForceDsa == GT_TRUE
* @param[in] useMii                   - use MII
*
* @param[out] timeMilliPtr             - pointer to duration of TX operation in milliseconds.
* @param[out] sendFailPtr              - number of failed TX operations
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfTrafficGeneratorNetIfSyncTxBurstSend
(
    IN CPSS_INTERFACE_INFO_STC          *portInterfacePtr,
    IN GT_U8                            *bufferPtr,
    IN GT_U32                            bufferLength,
    IN GT_U32                            packetsNumber,
    IN GT_BOOL                           doForceDsa,
    IN TGF_DSA_TYPE_ENT                  forcedDsa,
    IN GT_BOOL                           useMii,
    OUT double                          *timeMilliPtr,
    OUT GT_U32                          *sendFailPtr
);

/**
* @internal prvTgfTrafficGeneratorRxToTxInfoSet function
* @endinternal
*
* @brief   Fill in structure for Rx To Tx send packet info
*
* @param[in] portInterfacePtr         - (pointer to) port interface
* @param[in] vpt                      - VPT of the packet
* @param[in] cfiBit                   - CFI bit
* @param[in] vid                      - VID of the packet
* @param[in] packetIsTagged           - packet is tagged flag
* @param[in] doForceDsa               - GT_FALSE - choose DSA tag automatically
*                                      GT_TRUE - use DSA tag defined by forcedDsa
* @param[in] forcedDsa                - DSA tag type, used when doForceDsa == GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong interface
*/
GT_STATUS prvTgfTrafficGeneratorRxToTxInfoSet
(
    IN CPSS_INTERFACE_INFO_STC            *portInterfacePtr,
    IN GT_U32                              vpt,
    IN GT_U32                              cfiBit,
    IN GT_U32                              vid,
    IN GT_BOOL                             packetIsTagged,
    IN GT_BOOL                             doForceDsa,
    IN TGF_DSA_TYPE_ENT                    forcedDsa
);

/**
* @internal prvTgfTrafficGeneratorRxToTxSend function
* @endinternal
*
* @brief   Transmit the traffic to predefined port by prvTgfTrafficGeneratorRxToTxInfoSet
*
* @param[in] devNum                   - device number
* @param[in] numOfBufs                - number of buffers for packet
* @param[in] buffList[]               - list of buffers
* @param[in] buffLenList[]            - buffer length list (include CRC bytes)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfTrafficGeneratorRxToTxSend
(
    IN GT_U8            devNum,
    IN GT_U32           numOfBufs,
    IN GT_U8            *buffList[],
    IN GT_U32           buffLenList[]
);

/**
* @internal tgfTrafficGeneratorPortTxEthBurstTransmit function
* @endinternal
*
* @brief   Transmit the traffic burst to the port.
*         Used for TX SDMA performance measurement.
* @param[in] portInterfacePtr         - (pointer to) port interface
* @param[in] packetInfoPtr            - (pointer to) the packet info
* @param[in] burstCount               - number of frames (non-zero value)
* @param[in] doForceDsa               - GT_FALSE - choose DSA tag automatically
*                                      GT_TRUE - use DSA tag defined by forcedDsa
* @param[in] forcedDsa                - DSA tag type, used when doForceDsa == GT_TRUE
*
* @param[out] timeMilliPtr             - pointer to duration of TX operation in milliseconds.
* @param[out] sendFailPtr              - number of failed TX operations
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS tgfTrafficGeneratorPortTxEthBurstTransmit
(
    IN CPSS_INTERFACE_INFO_STC         *portInterfacePtr,
    IN TGF_PACKET_STC                  *packetInfoPtr,
    IN GT_U32                           burstCount,
    IN GT_BOOL                          doForceDsa,
    IN TGF_DSA_TYPE_ENT                 forcedDsa,
    OUT double                         *timeMilliPtr,
    OUT GT_U32                         *sendFailPtr
);

/**
* @internal prvTgfCommonMemberForceInfoSet function
* @endinternal
*
* @brief   add member info to DB of 'members to force to vlan/mc groups'
*         in case that the member already exists (dev,port) the other parameters
*         are 'updated' according to last setting.
* @param[in] memberInfoPtr            - (pointer to) member info
*
* @retval GT_OK                    - on success
* @retval GT_FULL                  - the DB is full and no more members can be added -->
*                                       need to make DB larger.
*/
GT_STATUS prvTgfCommonMemberForceInfoSet(
    IN PRV_TGF_MEMBER_FORCE_INFO_STC    *memberInfoPtr
);

/**
* @internal prvTgfCommonIsDeviceForce function
* @endinternal
*
* @brief   check if the device has ports that used with force configuration
*
* @retval GT_TRUE                  - the device force configuration
* @retval GT_FALSE                 - the device NOT force configuration
*/
GT_BOOL prvTgfCommonIsDeviceForce(
    IN GT_U8    devNum
);

/**
* @internal prvTgfPortMacCounterGet function
* @endinternal
*
* @brief   Gets Ethernet MAC counter for a specified port on specified device.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*                                      CPU port counters are valid only when using "Ethernet CPU
*                                      port" (not using the SDMA interface).
*                                      when using the SDMA interface refer to the API
*                                      cpssDxChNetIfSdmaRxCountersGet(...)
* @param[in] cntrName                 - specific counter name
*
* @param[out] cntrValuePtr             - (pointer to) current counter value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number, device or counter name
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The 10G MAC MIB counters are 64-bit wide.
*       Not supported counters: CPSS_BAD_PKTS_RCV_E, CPSS_UNRECOG_MAC_CNTR_RCV_E,
*       CPSS_BadFC_RCV_E, CPSS_GOOD_PKTS_RCV_E, CPSS_GOOD_PKTS_SENT_E.
*
*/
GT_STATUS prvTgfPortMacCounterGet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      portNum,
    IN  CPSS_PORT_MAC_COUNTERS_ENT  cntrName,
    OUT GT_U64                      *cntrValuePtr
);

/**
* @internal prvTgfDevPortMacCountersClearOnReadSet function
* @endinternal
*
* @brief   Enable or disable MAC Counters Clear on read per group of ports
*         for Tri-Speed Ports or per port for HyperG.Stack ports.
*         For Tri-Speed Ports groups are:
*         - Ports 0 through 5
*         - Ports 6 through 11
*         - Ports 12 through 17
*         - Ports 18 through 23
* @param[in] devNum
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] enable                   -  clear on read for MAC counters
*                                      GT_FALSE - Counters are not cleared.
*                                      GT_TRUE - Counters are cleared.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfDevPortMacCountersClearOnReadSet
(
    IN GT_U8    devNum,
    IN GT_U32    portNum,
    IN GT_BOOL  enable
);

/**
* @internal prvTgfDevPortMacCountersClearOnReadGet function
* @endinternal
*
* @brief   Get status (Enable or disable) of MAC Counters Clear on read per group of ports
*         for Tri-Speed Ports or per port for HyperG.Stack ports.
*         For Tri-Speed Ports groups are:
*         - Ports 0 through 5
*         - Ports 6 through 11
*         - Ports 12 through 17
*         - Ports 18 through 23
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] enablePtr                - pointer to enable clear on read for MAC counters
*                                      GT_FALSE - Counters are not cleared.
*                                      GT_TRUE - Counters are cleared.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfDevPortMacCountersClearOnReadGet
(
    IN GT_U8    devNum,
    IN GT_U32    portNum,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal prvTgfBrgVlanPortIngressTpidSet function
* @endinternal
*
* @brief   Function sets bitmap of TPID (Tag Protocol ID) table indexes per
*         ingress port.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] ethMode                  - TAG0/TAG1 selector
* @param[in] tpidBmp                  - bitmap represent entries in the TPID Table
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, ethMode
* @retval GT_OUT_OF_RANGE          - tpidBmp initialized with more then 8 bits
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanPortIngressTpidSet
(
    IN  GT_U8                devNum,
    IN  GT_U32                portNum,
    IN  CPSS_ETHER_MODE_ENT  ethMode,
    IN  GT_U32               tpidBmp
);

/**
* @internal prvTgfBrgVlanPortIngressTpidGet function
* @endinternal
*
* @brief   Function gets bitmap of TPID (Tag Protocol ID) table indexes per
*         ingress port.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] ethMode                  - TAG0/TAG1 selector
*
* @param[out] tpidBmpPtr               - (pointer to) bitmap represent 8 entries at the TPID Table
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, ethMode
* @retval GT_OUT_OF_RANGE          - tpidBmp initialized with more then 8 bits
* @retval GT_BAD_PTR               - tpidBmpPtr is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgVlanPortIngressTpidGet
(
    IN  GT_U8                devNum,
    IN  GT_U32                portNum,
    IN  CPSS_ETHER_MODE_ENT  ethMode,
    OUT GT_U32              *tpidBmpPtr
);

/**
* @internal tgfTrafficGeneratorSystemReset function
* @endinternal
*
* @brief   Preparation for system reset
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_VOID tgfTrafficGeneratorSystemReset
(
    GT_VOID
);

/**
* @internal tgfTrafficGeneratorPortTxEthCaptureDoLoopbackSet function
* @endinternal
*
* @brief   indication for function tgfTrafficGeneratorPortTxEthCaptureSet(...)
*         to allow loopback on the 'captured port' or not.
*/
GT_VOID tgfTrafficGeneratorPortTxEthCaptureDoLoopbackSet
(
    IN GT_BOOL      doLoopback
);

/**
* @internal tgfTrafficGeneratorPortTxEthCaptureWaitAfterDisableLoopbackSet function
* @endinternal
*
* @brief   indication for function tgfTrafficGeneratorPortTxEthCaptureSet(...)
*         to wait some time after loopback disable. This provides time to handle
*         all packets those already captured by device.
* @param[in] waitAfterDisableLoopback - enable/disable wait
*                                       None.
*/
GT_VOID tgfTrafficGeneratorPortTxEthCaptureWaitAfterDisableLoopbackSet
(
    IN GT_BOOL      waitAfterDisableLoopback
);

/**
* @internal prvTgfTxContModeEntryGet function
* @endinternal
*
* @brief   Get entry from transmit continuous mode DB
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] modePtr                  - (pointer to) mode entry
* @param[out] idxPtr                   - (pointer to) entry index
* @param[out] freeIdxPtr               - (pointer to) free entry index
*
* @retval GT_OK                    - on success.
* @retval GT_NO_SUCH               - on entry not exist.
*/
GT_STATUS prvTgfTxContModeEntryGet
(
    IN GT_U8                            devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    OUT PRV_TGF_TX_DEV_PORT_STC         **modePtr,
    OUT GT_U32                          *idxPtr,
    OUT GT_U32                          *freeIdxPtr
);

/**
* @internal tgfTrafficGeneratorWsModeOnPortSet function
* @endinternal
*
* @brief   set WS mode on port , by setting rxMirror and setting proper tx mode
*
* @param[in] devNum                   - device number
*                                      portNum - port number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
*/
GT_STATUS  tgfTrafficGeneratorWsModeOnPortSet(IN GT_U8    devNum, IN GT_U32  portNum);

/**
* @internal tgfTrafficGeneratorStopWsOnPort function
* @endinternal
*
* @brief   stop WS on port , by disable rxMirror and setting proper tx mode
*
* @param[in] devNum                   - device number
*                                      portNum - port number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
*/
GT_STATUS  tgfTrafficGeneratorStopWsOnPort(IN GT_U8    devNum, IN GT_U32  portNum);

/**
* @internal prvTgfCommonDevicePortWsRateGet function
* @endinternal
*
* @brief   Get wire speed rate of port in packets per second
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] packetSize               - packet size in bytes including CRC
*                                       wire speed rate of port in packets per second.
*
* @retval 0xFFFFFFFF               - on error
*/
GT_U32 prvTgfCommonDevicePortWsRateGet
(
    IN  GT_U8  devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_U32 packetSize
);
/**
* @internal prvTgfCommonDiffInPercentCalc function
* @endinternal
*
* @brief   Calculate difference between input values in percent
*
* @param[in] value1                   -  to compare
* @param[in] value2                   -  to compare
*                                       difference between input values in percent
*/
GT_U32 prvTgfCommonDiffInPercentCalc
(
    IN  GT_U32 value1,
    IN  GT_U32 value2
);

/**
* @internal prvTgfCommonPortsCntrRateGet_byDevPort function
* @endinternal
*
* @brief   Get rate of specific ports MAC MIB for up to 2 counters.
*
* @param[in] devNumArr[]              - array of devNum
* @param[in] portNumArr[]             - array of portNum
* @param[in] numOfPorts               - number of ports
* @param[in] cntrName1                - name of first counter
* @param[in] timeOut                  - timeout in milli seconds to calculate rate
* @param[in] cntrName2                - name of second counter
*
* @param[out] outRateArr1[]            - (pointer to) array of rate of ports for cntrName1.
* @param[out] outRateArr2[]            - (pointer to) array of rate of ports for cntrName2.
*                                      if NULL then this rate ignored.
*                                       none
*/
GT_STATUS prvTgfCommonPortsCntrRateGet_byDevPort
(
    IN  GT_U8  devNumArr[],
    IN  GT_U32 portNumArr[],
    IN  GT_U32 numOfPorts,
    IN  GT_U32 timeOut,
    IN  CPSS_PORT_MAC_COUNTERS_ENT  cntrName1,
    OUT GT_U32 outRateArr1[],
    IN  CPSS_PORT_MAC_COUNTERS_ENT  cntrName2,
    OUT GT_U32 outRateArr2[]
);

/**
* @internal prvTgfCaptureEnableUnmatchedTpidProfile function
* @endinternal
*
* @brief   Configure (or restore) specified TPID profile to not match with
*         any TPID in packets. This profile will be used by TGF to treat incoming
*         packets on captured port as untagged. This saves us from moving of
*         vlan tag infromation into TO_CPU DSA tag so a packet's original
*         etherType will not be lost.
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE  - configure TPID profile tpidIndex to not match
*                                      with any TPID.
*                                      GT_FALSE - restore state of TPID profile as it was before
*                                      configuring.
* @param[in] tpidIndex                - the TPID profile index
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong profile
*/
GT_STATUS prvTgfCaptureEnableUnmatchedTpidProfile
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable,
    IN GT_U32   tpidIndex
);

/**
* @internal tgfTrafficGeneratorEtherTypeIgnoreSet function
* @endinternal
*
* @brief   indication to IGNORE setting of 'etherTypeForVlanTag' that is set by
*         tgfTrafficGeneratorEtherTypeForVlanTagSet(...) and default is '0x8100'
* @param[in] ignore                   - GT_TRUE  - to      etherTypeForVlanTag.
*                                      GT_FALSE - to NOT ignore etherTypeForVlanTag .
*                                       None
*/
extern GT_VOID tgfTrafficGeneratorEtherTypeIgnoreSet
(
    IN GT_BOOL ignore
);

/**
* @internal tgfTrafficGeneratorEtherTypeIgnoreGet function
* @endinternal
*
* @brief   get state of flag to IGNORE setting of 'etherTypeForVlanTag' that is set
*         by tgfTrafficGeneratorEtherTypeForVlanTagSet(...) and default is '0x8100'
*
* @retval GT_TRUE                  - ignore etherTypeForVlanTag.
* @retval GT_FALSE                 - not ignore etherTypeForVlanTag .
*/
extern GT_BOOL tgfTrafficGeneratorEtherTypeIgnoreGet
(
    IN GT_VOID
);
/**
* @internal tgfDefaultTxSdmaQueueSet function
* @endinternal
*
* @brief   Debug function to set the tx SDMA queue used in tgf when sending a packet
*
* @param[in] txQNum                   - the queue number (0-31)
*
* @retval GT_OK                    - Queue was set as requested
*/
GT_STATUS tgfDefaultTxSdmaQueueSet
(
    IN  GT_U8 txQNum
);

/**
* @internal tgfTrafficGeneratorPxIgnoreCounterCheckSet function
* @endinternal
*
* @brief   Ignore counter check in tgfTrafficGeneratorPxTransmitPacketAndCheckResult
*
* @param[in] ignore                   - GT_TRUE - ignore
*                                      GT_FALSE - not ignore
*                                       None.
*/
GT_VOID tgfTrafficGeneratorPxIgnoreCounterCheckSet
(
    GT_BOOL ignore
);

/**
* @internal tgfTrafficGeneratorPxTransmitPacketAndCheckResult function
* @endinternal
*
* @brief   Send packet to ingress port and check expected egress ports.
*         function will set :
*         1. link UP
*         2. MAC LOOPBACK
*         3. set 'forwarding' tables to direct the packet from CPU to the port.
*         3. send from cpu packet to egress the port and LOOPBACK will make it
*         ingress the port.
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the ingress port num
* @param[in] packetInfoPtr            - (pointer to) the packet info
* @param[in] burstCount               - number of frames
* @param[in] numOfEgressPortsInfo     - number of egress ports info in array of egressPortsArr[]
*                                      if 0 --> not checking egress ports
* @param[in] egressPortsArr[]         - array of egress ports to check for counters and packets
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS tgfTrafficGeneratorPxTransmitPacketAndCheckResult
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN TGF_PACKET_STC      *packetInfoPtr,
    IN GT_U32               burstCount,
    IN GT_U32               numOfEgressPortsInfo,
    IN TGF_EXPECTED_EGRESS_INFO_STC     egressPortsArr[]
);

/**
* @internal tgfTrafficGeneratorPxTransmitPacket function
* @endinternal
*
* @brief   Send packet to ingress port.
*         function will set :
*         1. link UP
*         2. MAC LOOPBACK
*         3. set 'forwarding' tables to direct the packet from CPU to the port.
*         3. send from cpu packet to egress the port and LOOPBACK will make it
*         ingress the port.
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the ingress port num
* @param[in] packetInfoPtr            - (pointer to) the packet info
* @param[in] burstCount               - number of frames
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS tgfTrafficGeneratorPxTransmitPacket
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN TGF_PACKET_STC      *packetInfoPtr,
    IN GT_U32               burstCount
);

/**
* @internal prvTgfEgressPortForceLinkPassEnable function
* @endinternal
*
* @brief  Manage force link pass on egress port.
*         In case of running on emulator (MAC MIB counter issue),
*         loopback state and MRU should be changed depending on
*         force link pass and loopback state on egress port.
*
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the egress port num
* @param[in] enableForceLinkUp        - enable/disable force link pass on egress port
* @param[in] doWa                     - enable/disable MAC MIB counters WA on egress port
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
*/
GT_STATUS prvTgfEgressPortForceLinkPassEnable
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_BOOL              enableForceLinkUp,
    IN GT_BOOL              doWa

);

/**
* @internal tgfTrafficGeneratorIsSpecialPortNumAfterReductionInDsa function
* @endinternal
*
* @brief   check if the port number is special (59-63) when need to be in the DSA tag
*           as the DSA hold limited number of bits for the portNum.
*
* @param[in] devNum  - the device number
* @param[in] portNum - the port number
* @param[in] dsaType - the DSA type (1/2/4 words)
*
* @retval GT_TRUE  - the port is     special only because the DSA hold less bits
*                    than actually set in the portNum
*         GT_FALSE - the port is not special or not reduced in the DSA
*/
GT_BOOL tgfTrafficGeneratorIsSpecialPortNumAfterReductionInDsa(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN TGF_DSA_TYPE_ENT     dsaType
);


/**
* @internal tgfTrafficGeneratorIsSpecialPortNumAfterReductionInBits function
* @endinternal
*
* @brief   check if the port number is special (59-63) when need to be with limited
*           number of bits for the portNum.
*
* @param[in] devNum  - the device number
* @param[in] portNum - the port number
* @param[in] numBits - the number of bits
*
* @retval GT_TRUE  - the port is     special only because the 'numBits' less bits
*                    than actually set in the portNum
*         GT_FALSE - the port is not special or not reduced in the 'numBits'
*/
GT_BOOL tgfTrafficGeneratorIsSpecialPortNumAfterReductionInBits(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_U32               numBits
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __tgfTrafficGeneratorh */

