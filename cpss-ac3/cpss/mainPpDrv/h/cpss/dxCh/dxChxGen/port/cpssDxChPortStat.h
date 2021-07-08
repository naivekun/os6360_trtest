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
* @file cpssDxChPortStat.h
*
* @brief CPSS implementation for core port statistics and
* egress counters facility.
*
* @version   16
********************************************************************************
*/

#ifndef __cpssDxChPortStath
#define __cpssDxChPortStath

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/common/port/cpssPortStat.h>
#include <cpss/generic/cos/cpssCosTypes.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h>


/******* info about Ethernet CPU port ******************************************
    support only next counters:

    1. CPSS_GOOD_PKTS_SENT_E:      32 bits counter
    2. CPSS_MAC_TRANSMIT_ERR_E:    16 bits counter
    3. CPSS_GOOD_OCTETS_SENT_E:    32 bits counter
    4. CPSS_DROP_EVENTS_E:         16 bits counter
    5. CPSS_GOOD_PKTS_RCV_E:       16 bits counter
    6. CPSS_BAD_PKTS_RCV_E:        16 bits counter
    7. CPSS_GOOD_OCTETS_RCV_E:     32 bits counter
    8. CPSS_BAD_OCTETS_RCV_E:      16 bits counter

*******************************************************************************/

/**
* @enum CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT
 *
 * @brief Enumeration of oversized packets counter mode.
*/
typedef enum{

    /** @brief CPSS_DEFERRED_PKTS_SENT_E counts deferred packets sent.
     *  CPSS_PKTS_1024TOMAX_OCTETS_E counts total bytes sent/received from
     *  packets which have a size of 1024 or more.
     */
    CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_NORMAL_E,

    /** @brief CPSS_DEFERRED_PKTS_SENT_E counts total bytes sent/received from
     *  packets which have a size of 1024 to 1518.
     *  CPSS_PKTS_1024TOMAX_OCTETS_E counts total bytes sent/received from
     *  packets which have a size of 1519 or more.
     */
    CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_HISTOGRAM_1518_E,

    /** @brief CPSS_DEFERRED_PKTS_SENT_E counts total bytes sent/received from
     *  packets which have a size of 1024 to 1522.
     *  CPSS_PKTS_1024TOMAX_OCTETS_E counts total bytes sent/received from
     *  packets which have a size of 1523 or more.
     */
    CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_HISTOGRAM_1522_E

} CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT;


/**
* @struct CPSS_DXCH_PORT_STAT_TX_DROP_COUNTERS_STC
 *
 * @brief Structure of TXQ drop counters.
*/
typedef struct{

    GT_U32 egrMirrorDropCntr;

    /** egress STC dropped counter value */
    GT_U32 egrStcDropCntr;

    /** egress QCN dropped counter value */
    GT_U32 egrQcnDropCntr;

    /** dropped PFC triggered events counter value */
    GT_U32 dropPfcEventsCntr;

    /** @brief total number of discarded descriptors
     *  (clear events to the Buffer Manager)
     */
    GT_U32 clearPacketsDroppedCounter;

} CPSS_DXCH_PORT_STAT_TX_DROP_COUNTERS_STC;

/**
* @internal cpssDxChPortStatInit function
* @endinternal
*
* @brief   Init port statistics counter set CPSS facility.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortStatInit
(
    IN  GT_U8       devNum
);

/**
* @internal cpssDxChMacCounterGet function
* @endinternal
*
* @brief   Gets Ethernet MAC counter for a specified port on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number or CPU port
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
* @note 1. The following counters are not supported:
*       Tri-Speed Ports:
*       CPSS_BAD_PKTS_RCV_E, CPSS_UNRECOG_MAC_CNTR_RCV_E, CPSS_BadFC_RCV_E,
*       CPSS_GOOD_PKTS_RCV_E and CPSS_GOOD_PKTS_SENT_E.
*       XG / HyperG.Stack / XLG Ports:
*       CPSS_BAD_PKTS_RCV_E, CPSS_UNRECOG_MAC_CNTR_RCV_E, CPSS_BadFC_RCV_E,
*       CPSS_GOOD_PKTS_RCV_E, CPSS_GOOD_PKTS_SENT_E,
*       CPSS_EXCESSIVE_COLLISIONS_E, CPSS_COLLISIONS_E,
*       CPSS_LATE_COLLISIONS_E and CPSS_DEFERRED_PKTS_SENT_E.
*       2. CPU port counters are valid only when using "Ethernet CPU port", i.e.
*       not using SDMA interface.
*       When using SDMA interface the following APIs are relevant:
*       cpssDxChNetIfSdmaRxCountersGet, cpssDxChNetIfSdmaRxErrorCountGet.
*       3. The following counters are supported for CPU port:
*       CPSS_GOOD_PKTS_SENT_E, CPSS_MAC_TRANSMIT_ERR_E, CPSS_DROP_EVENTS_E,
*       CPSS_GOOD_OCTETS_SENT_E, CPSS_GOOD_PKTS_RCV_E, CPSS_BAD_PKTS_RCV_E,
*       CPSS_GOOD_OCTETS_RCV_E, CPSS_BAD_OCTETS_RCV_E.
*
*/
GT_STATUS cpssDxChMacCounterGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_MAC_COUNTERS_ENT  cntrName,
    OUT GT_U64                      *cntrValuePtr
);

/**
* @internal cpssDxChPortMacCountersOnPortGet function
* @endinternal
*
* @brief   Gets Ethernet MAC counter for a particular Port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number or CPU port
*
* @param[out] portMacCounterSetArrayPtr - (pointer to) array of current counter values.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. The following counters are not supported:
*       Tri-Speed Ports:
*       badPktsRcv, unrecogMacCntrRcv, badFcRcv, goodPktsRcv, goodPktsSent.
*       XG / HyperG.Stack / XLG Potrs:
*       badPktsRcv, unrecogMacCntrRcv, badFcRcv, goodPktsRcv, goodPktsSent,
*       excessiveCollisions, collisions, lateCollisions, deferredPktsSent.
*       2. CPU port counters are valid only when using "Ethernet CPU port", i.e.
*       not using SDMA interface.
*       When using SDMA interface the following APIs are relevant:
*       cpssDxChNetIfSdmaRxCountersGet, cpssDxChNetIfSdmaRxErrorCountGet.
*       3. The following counters are supported for CPU port:
*       goodPktsSent, macTransmitErr, goodOctetsSent, dropEvents,
*       goodPktsRcv, badPktsRcv, goodOctetsRcv, badOctetsRcv.
*
*/
GT_STATUS   cpssDxChPortMacCountersOnPortGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PORT_MAC_COUNTER_SET_STC   *portMacCounterSetArrayPtr
);

/**
* @internal cpssDxChPortMacCounterCaptureGet function
* @endinternal
*
* @brief   Gets the captured Ethernet MAC counter for a specified port
*         on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] cntrName                 - specific counter name
*
* @param[out] cntrValuePtr             - (pointer to) current counter value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number, device or counter name
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. For Tri-speed ports, each MAC counter capture performed on ports of
*       the same group overrides previous capture made on port of that group.
*       Ports 0-5 belong to group 0.
*       Ports 6-11 belong to group 1.
*       Ports 12-17 belong to group 2.
*       Ports 18-23 belong to group 3.
*       2. The following counters are not supported:
*       Tri-Speed Ports:
*       CPSS_BAD_PKTS_RCV_E, CPSS_UNRECOG_MAC_CNTR_RCV_E, CPSS_BadFC_RCV_E,
*       CPSS_GOOD_PKTS_RCV_E and CPSS_GOOD_PKTS_SENT_E.
*       XG / HyperG.Stack / XLG Potrs:
*       CPSS_BAD_PKTS_RCV_E, CPSS_UNRECOG_MAC_CNTR_RCV_E, CPSS_BadFC_RCV_E,
*       CPSS_GOOD_PKTS_RCV_E, CPSS_GOOD_PKTS_SENT_E,
*       CPSS_EXCESSIVE_COLLISIONS_E, CPSS_COLLISIONS_E,
*       CPSS_LATE_COLLISIONS_E and CPSS_DEFERRED_PKTS_SENT_E.
*
*/
GT_STATUS cpssDxChPortMacCounterCaptureGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_MAC_COUNTERS_ENT  cntrName,
    OUT GT_U64                      *cntrValuePtr
);

/**
* @internal cpssDxChPortMacCountersCaptureOnPortGet function
* @endinternal
*
* @brief   Gets captured Ethernet MAC counter for a particular Port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] portMacCounterSetArrayPtr - (pointer to) array of current counter values.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. For Tri-speed ports, each MAC counter capture performed on ports of
*       the same group overrides previous capture made on port of that group.
*       Ports 0-5 belong to group 0.
*       Ports 6-11 belong to group 1.
*       Ports 12-17 belong to group 2.
*       Ports 18-23 belong to group 3.
*       2. The following counters are not supported:
*       Tri-Speed Ports:
*       badPktsRcv, unrecogMacCntrRcv, badFcRcv, goodPktsRcv, goodPktsSent.
*       XG / HyperG.Stack / XLG Potrs:
*       badPktsRcv, unrecogMacCntrRcv, badFcRcv, goodPktsRcv, goodPktsSent,
*       excessiveCollisions, collisions, lateCollisions, deferredPktsSent.
*       3. On AC3X remote ports, when "useMibCounterReadByPacket" option was set
*       at Bind operation, MIB counters are sent to CPU on a "TO_CPU" packet
*       with configured CPU code.
*       It is applications responsibility to provide the packet to CPSS for parsing, using
*       function cpssDxChPortRemotePortsMacCountersByPacketParse()
*
*/
GT_STATUS   cpssDxChPortMacCountersCaptureOnPortGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PORT_MAC_COUNTER_SET_STC   *portMacCounterSetArrayPtr
);

/**
* @internal cpssDxChPortMacCountersCaptureTriggerSet function
* @endinternal
*
* @brief   The function triggers a capture of MIB counters for specific port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number, whose counters are to be captured.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - previous capture operation isn't finished.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. For Tri-speed ports, each MAC counter capture performed on ports of
*       the same group overrides previous capture made on port of that group.
*       Ports 0-5 belong to group 0.
*       Ports 6-11 belong to group 1.
*       Ports 12-17 belong to group 2.
*       Ports 18-23 belong to group 3.
*       2. To get the counters, see
*       cpssDxChPortMacCountersCaptureOnPortGet,
*       cpssDxChPortMacCounterCaptureGet.
*       3. On AC3X remote ports, when "useMibCounterReadByPacket" option was set
*       at Bind operation, MIB counters are sent to CPU on a "TO_CPU" packet
*       with configured CPU code.
*       It is applications responsibility to provide the packet to CPSS for parsing, using
*       function cpssDxChPortRemotePortsMacCountersByPacketParse()
*
*/
GT_STATUS cpssDxChPortMacCountersCaptureTriggerSet
(
    IN  GT_U8  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum
);

/**
* @internal cpssDxChPortMacCountersCaptureTriggerGet function
* @endinternal
*
* @brief   The function gets status of a capture of MIB counters for specific port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number.
*
* @param[out] captureIsDonePtr         - pointer to status of Capture counter Trigger
*                                      - GT_TRUE  - capture is done.
*                                      - GT_FALSE - capture action is in proccess.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. For Tri-speed ports, each MAC counter capture performed on ports of
*       the same group overrides previous capture made on port of that group.
*       Ports 0-5 belong to group 0.
*       Ports 6-11 belong to group 1.
*       Ports 12-17 belong to group 2.
*       Ports 18-23 belong to group 3.
*       2. To get the counters, see
*       cpssDxChPortMacCountersCaptureOnPortGet,
*       cpssDxChPortMacCounterCaptureGet.
*       3. On AC3X remote ports, when "useMibCounterReadByPacket" option was set
*       at Bind operation, MIB counters are sent to CPU on a "TO_CPU" packet
*       with configured CPU code.
*       It is applications responsibility to provide the packet to CPSS for parsing, using
*       function cpssDxChPortRemotePortsMacCountersByPacketParse()
*
*/
GT_STATUS cpssDxChPortMacCountersCaptureTriggerGet
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL  *captureIsDonePtr
);

/**
* @internal cpssDxChPortEgressCntrModeSet function
* @endinternal
*
* @brief   Configure a set of egress counters that work in specified bridge egress
*         counters mode on specified device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] cntrSetNum               - counter set number.
* @param[in] setModeBmp               - counter mode bitmap. For example:
*                                      GT_EGRESS_CNT_PORT | GT_EGRESS_CNT_VLAN | GT_EGRESS_CNT_DP.
* @param[in] portNum                  - physical port number, if corresponding bit in setModeBmp is 1.
* @param[in] vlanId                   - VLAN Id, if corresponding bit in setModeBmp is 1.
* @param[in] tc                       - traffic class queue (0..7),
*                                      if corresponding bit in setModeBmp is 1.
* @param[in] dpLevel                  - drop precedence level, if corresponding bit in setModeBmp is 1.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Supported DP levels: CPSS_DP_GREEN_E, CPSS_DP_RED_E.
*
*/
GT_STATUS cpssDxChPortEgressCntrModeSet
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           cntrSetNum,
    IN  CPSS_PORT_EGRESS_CNT_MODE_ENT   setModeBmp,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U16                          vlanId,
    IN  GT_U8                           tc,
    IN  CPSS_DP_LEVEL_ENT               dpLevel
);

/**
* @internal cpssDxChPortEgressCntrModeGet function
* @endinternal
*
* @brief   Get configuration of a bridge egress counters set.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] cntrSetNum               - counter set number.
*
* @param[out] setModeBmpPtr            - Pointer to counter mode bitmap. For example:
*                                      GT_EGRESS_CNT_PORT | GT_EGRESS_CNT_VLAN | GT_EGRESS_CNT_DP.
* @param[out] portNumPtr               - Pointer to the egress port of the packets counted by
*                                      the set of counters
* @param[out] vlanIdPtr                - Pointer to the vlan id of the packets counted by the
*                                      set of counters.
* @param[out] tcPtr                    - Pointer to the tc of the packets counted by the set
*                                      of counters.
* @param[out] dpLevelPtr               - Pointer to the dp of the packets counted by the set
*                                      of counters.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Supported DP levels: CPSS_DP_GREEN_E, CPSS_DP_RED_E.
*
*/
GT_STATUS cpssDxChPortEgressCntrModeGet
(
    IN   GT_U8                           devNum,
    IN   GT_U8                           cntrSetNum,
    OUT  CPSS_PORT_EGRESS_CNT_MODE_ENT   *setModeBmpPtr,
    OUT  GT_PHYSICAL_PORT_NUM            *portNumPtr,
    OUT  GT_U16                          *vlanIdPtr,
    OUT  GT_U8                           *tcPtr,
    OUT  CPSS_DP_LEVEL_ENT               *dpLevelPtr
);

/**
* @internal cpssDxChPortEgressCntrsGet function
* @endinternal
*
* @brief   Gets a egress counters from specific counter-set.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] cntrSetNum               - counter set number : 0, 1
*
* @param[out] egrCntrPtr               - (pointer to) structure of egress counters current values.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortEgressCntrsGet
(
    IN  GT_U8                       devNum,
    IN  GT_U8                       cntrSetNum,
    OUT CPSS_PORT_EGRESS_CNTR_STC   *egrCntrPtr
);

/**
* @internal cpssDxChPortMacCountersEnable function
* @endinternal
*
* @brief   Enable or disable MAC Counters update for this port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] enable                   -   update of MAC counters
*                                      GT_FALSE = MAC counters update for this port is disabled.
*                                      GT_TRUE = MAC counters update for this port is enabled.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortMacCountersEnable(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM        portNum,
    IN GT_BOOL enable
);


/**
* @internal cpssDxChPortMacCountersClearOnReadSet function
* @endinternal
*
* @brief   Enable or disable MAC Counters Clear on read per group of ports
*         for Tri-Speed Ports or per port for HyperG.Stack ports.
*         For Tri-Speed Ports groups are:
*         - Ports 0 through 5
*         - Ports 6 through 11
*         - Ports 12 through 17
*         - Ports 18 through 23
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
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
GT_STATUS cpssDxChPortMacCountersClearOnReadSet(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM        portNum,
    IN GT_BOOL  enable
);

/**
* @internal cpssDxChPortMacCountersClearOnReadGet function
* @endinternal
*
* @brief   Get "Clear on read" status of MAC Counters per group of ports
*         for Tri-Speed Ports or per port for HyperG.Stack ports.
*         For Tri-Speed Ports groups are:
*         - Ports 0 through 5
*         - Ports 6 through 11
*         - Ports 12 through 17
*         - Ports 18 through 23
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] enablePtr                - (pointer to) "Clear on read" status for MAC counters
*                                      GT_FALSE - Counters are not cleared.
*                                      GT_TRUE - Counters are cleared.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortMacCountersClearOnReadGet(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal cpssDxChPortMacCountersRxHistogramEnable function
* @endinternal
*
* @brief   Enable/disable updating of the RMON Etherstat histogram
*         counters for received packets per group of ports
*         for Tri-Speed Ports or per port for HyperG.Stack ports.
*         For Tri-Speed Ports groups are:
*         - Ports 0 through 5
*         - Ports 6 through 11
*         - Ports 12 through 17
*         - Ports 18 through 23
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number
* @param[in] enable                   -  updating of the counters for received packets
*                                      GT_TRUE - Counters are updated.
*                                      GT_FALSE - Counters are not updated.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortMacCountersRxHistogramEnable(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM        portNum,
    IN GT_BOOL  enable
);

/**
* @internal cpssDxChPortMacCountersTxHistogramEnable function
* @endinternal
*
* @brief   Enable/disable updating of the RMON Etherstat histogram
*         counters for transmitted packets per group of ports
*         for Tri-Speed Ports or per port for HyperG.Stack ports.
*         For Tri-Speed Ports groups are:
*         - Ports 0 through 5
*         - Ports 6 through 11
*         - Ports 12 through 17
*         - Ports 18 through 23
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number
* @param[in] enable                   -  updating of the counters for transmitted packets
*                                      GT_TRUE - Counters are updated.
*                                      GT_FALSE - Counters are not updated.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortMacCountersTxHistogramEnable(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM        portNum,
    IN GT_BOOL  enable
);

/**
* @internal cpssDxChPortMacOversizedPacketsCounterModeSet function
* @endinternal
*
* @brief   Sets MIB counters behavior for oversized packets on given port.
*
* @note   APPLICABLE DEVICES:      xCat2, xCat3; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] counterMode              - oversized packets counter mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number, device or counter mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The oversized packets counter mode determines the behavior of the
*       CPSS_PKTS_1024TOMAX_OCTETS_E and CPSS_DEFERRED_PKTS_SENT_E counters.
*
*/
GT_STATUS cpssDxChPortMacOversizedPacketsCounterModeSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT  counterMode
);

/**
* @internal cpssDxChPortMacOversizedPacketsCounterModeGet function
* @endinternal
*
* @brief   Gets MIB counters behavior for oversized packets on given port.
*
* @note   APPLICABLE DEVICES:      xCat2, xCat3, Bobcat2, Caelum, Bobcat3; Aldrin2; Falcon, Aldrin, AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] counterModePtr           - (pointer to) oversized packets counter mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The oversized packets counter mode determines the behavior of the
*       CPSS_PKTS_1024TOMAX_OCTETS_E and CPSS_DEFERRED_PKTS_SENT_E counters.
*
*/
GT_STATUS cpssDxChPortMacOversizedPacketsCounterModeGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT  *counterModePtr
);


/**
* @internal cpssDxChPortStatTxDebugCountersGet function
* @endinternal
*
* @brief   Gets egress TxQ drop counters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
*
* @param[out] dropCntrStcPtr           - (pointer to) drop counter structure
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortStatTxDebugCountersGet
(
    IN  GT_U8   devNum,
    OUT CPSS_DXCH_PORT_STAT_TX_DROP_COUNTERS_STC  *dropCntrStcPtr
);


/**
* @internal cpssDxChPortMacCountersRxHistogramEnableGet function
* @endinternal
*
* @brief   Get state (enabled/disabled) of the RMON Etherstat histogram
*         counters for received packets.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number
*
* @param[out] enablePtr                - state of the counters for received packets
*                                      GT_TRUE - Counters are enabled.
*                                      GT_FALSE - Counters are disabled.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - the register's address is not found
*/
GT_STATUS cpssDxChPortMacCountersRxHistogramEnableGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
);

/**
* @internal cpssDxChPortMacCountersTxHistogramEnableGet function
* @endinternal
*
* @brief   Get state (enabled/disabled) of the RMON Etherstat histogram
*         counters for transmitted packets.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number
*
* @param[out] enablePtr                - state of the counters for transmitted packets
*                                      GT_TRUE - Counters are enabled.
*                                      GT_FALSE - Counters are disabled.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - the register's address is not found
*/
GT_STATUS cpssDxChPortMacCountersTxHistogramEnableGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
);

/**
* @internal cpssDxChPortMacCountersEnableGet function
* @endinternal
*
* @brief   Get state (enabled/disable) of MAC Counters update
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] enablePtr                - state of the MAC counters
*                                      GT_FALSE = MAC counters update for this port is disabled.
*                                      GT_TRUE  = MAC counters update for this port is enabled.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - the register's address is not found
*/
GT_STATUS cpssDxChPortMacCountersEnableGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
);

/**
* @internal cpssDxChPortMacCountersOnCgPortGet function
* @endinternal
*
* @brief   Gets the CG mac mib counters as is without any conversion.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   -  physical device number
* @param[in] portNum                  -  port number
*
* @param[out] cgMibStcPtr              - (pointer to) CG mac mib structure
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number, device or counter name
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported port
*/
GT_STATUS cpssDxChPortMacCountersOnCgPortGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT CPSS_PORT_MAC_CG_COUNTER_SET_STC   *cgMibStcPtr
);

/**
* @internal cpssDxChPortMacCountersOnMtiPortGet function
* @endinternal
*
* @brief   Gets the MTI mac mib counters as is without any conversion.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  physical device number
* @param[in] portNum                  -  port number
*
* @param[out] mtiMibStcPtr            - (pointer to) MTI mac mib structure
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number, device or counter name
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported port
*/
GT_STATUS cpssDxChPortMacCountersOnMtiPortGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT CPSS_PORT_MAC_MTI_COUNTER_SET_STC   *mtiMibStcPtr
);

/**
* @internal cpssDxChPortRemotePortsMacCountersByPacketParse function
* @endinternal
*
* @brief   Parses packet containing Ethernet MAC counters.
*         If the packet has the CPU code as configured during remote port Bind,
*         The result is stored at shadow buffers, and it can be retrieved
*         by function cpssDxChPortMacCountersCaptureOnPortGet()
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] numOfBuff                - Num of used buffs in packetBuffsArrPtr.
* @param[in] packetBuffsArrPtr[]      - (pointer to)The received packet buffers list.
* @param[in] buffLenArr[]             - List of buffer lengths for packetBuffsArrPtr.
* @param[in] rxParamsPtr              - (pointer to)information parameters of received packets
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The input parameters are obtained by function cpssDxChNetIfSdmaRxPacketGet()
*
*/
GT_STATUS cpssDxChPortRemotePortsMacCountersByPacketParse
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          numOfBuff,
    IN  GT_U8                          *packetBuffsArrPtr[],
    IN  GT_U32                          buffLenArr[],
    IN  CPSS_DXCH_NET_RX_PARAMS_STC    *rxParamsPtr
);

/**
* @internal cpssDxChMacCounterOnPhySideGet function
* @endinternal
*
* @brief   Gets Ethernet MAC counter on PHY side of connecting link to remote ports.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - connecting physical port number connected to remote ports
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
* @note Applicable only to remote ports on AC3X.
*
*/
GT_STATUS cpssDxChMacCounterOnPhySideGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_MAC_COUNTERS_ENT  cntrName,
    OUT GT_U64                      *cntrValuePtr
);

/**
* @internal cpssDxChPortMacCountersOnPhySidePortGet function
* @endinternal
*
* @brief   Gets all port Ethernet MAC counters on PHY side of connecting link to remote Port.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - connecting physical port number connected to remote ports
*
* @param[out] portMacCounterSetArrayPtr - (pointer to) array of current counter values.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only to remote ports on AC3X.
*
*/
GT_STATUS   cpssDxChPortMacCountersOnPhySidePortGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PORT_MAC_COUNTER_SET_STC   *portMacCounterSetArrayPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChPortStath */

