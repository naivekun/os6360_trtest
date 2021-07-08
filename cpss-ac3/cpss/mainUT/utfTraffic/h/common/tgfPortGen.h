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
* @file tgfPortGen.h
*
* @brief Generic API for Port configuration.
*
* @version   26
********************************************************************************
*/
#ifndef __tgfPortGenh
#define __tgfPortGenh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/port/cpssPortTx.h>
#include <cpss/generic/port/cpssPortCtrl.h>
#include <utf/private/prvUtfExtras.h>

/**
* @enum PRV_TGF_PORT_HOL_FC_ENT
 *
 * @brief Enumeration of system modes HOL and Flow Control.
*/
typedef enum{

    PRV_TGF_PORT_PORT_FC_E,

    PRV_TGF_PORT_PORT_HOL_E

} PRV_TGF_PORT_HOL_FC_ENT;

/**
* @enum PRV_TGF_PORT_TX_SHARED_DP_MODE_ENT
 *
 * @brief Drop Precedence (DP) enable mode to use the shared pool.
*/
typedef enum{

    /** @brief None of the packets (with any DP) can use the shared pool.
     *  Compatible with DxCh3 and DxChXcat Disable mode.
     */
    PRV_TGF_PORT_TX_SHARED_DP_MODE_DISABLE_E = GT_FALSE,

    /** @brief All packets (with any DP) can use the shared pool.
     *  Compatible with DxCh3 and DxChXcat Enable mode.
     */
    PRV_TGF_PORT_TX_SHARED_DP_MODE_ALL_E = GT_TRUE,

    /** Only packets with DP0 can use the shared pool. */
    PRV_TGF_PORT_TX_SHARED_DP_MODE_DP0_E,

    /** Packets with DP0 and DP1 can use the shared pool. */
    PRV_TGF_PORT_TX_SHARED_DP_MODE_DP0_DP1_E

} PRV_TGF_PORT_TX_SHARED_DP_MODE_ENT;

/**
* @struct PRV_TGF_PORT_TX_TAIL_DROP_WRTD_ENABLERS_STC
 *
 * @brief Tail Drop limits enabling for Weigthed Random Tail Drop.
*/
typedef struct{

    /** @brief enable\disable WRTD for (Queue,DP) descriptor/buffer
     *  limits.
     */
    GT_BOOL tcDpLimit;

    /** enable\disable Port descriptor/buffer limits. */
    GT_BOOL portLimit;

    /** enable\disable Queue descriptor/buffer limits. */
    GT_BOOL tcLimit;

    /** enable\disable Shared Pool descriptor/buffer limits. */
    GT_BOOL sharedPoolLimit;

} PRV_TGF_PORT_TX_TAIL_DROP_WRTD_ENABLERS_STC;

/**
* @struct PRV_TGF_PORT_TX_WRTD_MASK_LSB
 *
 * @brief The Tail Drop WRTD masked lsbs' for each limit type.
 * All masks are up to 9 bits.
*/
typedef struct{

    /** The number of lsbs' masked for WRTD for (Queue,DP) limits. */
    GT_U32 tcDp;

    /** The number of lsbs' masked for WRTD for Port limits. */
    GT_U32 port;

    /** The number of lsbs' masked for WRTD for Queue limits. */
    GT_U32 tc;

    /** The number of lsbs' masked for WRTD for Shared Pool limits. */
    GT_U32 pool;

} PRV_TGF_PORT_TX_WRTD_MASK_LSB;

/**
* @struct PRV_TGF_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS
 *
 * @brief The Tail Drop profile Traffic Class parameters
*/
typedef struct{

    /** @brief the number of buffers that can be allocated to all of the
     *  port's TC Tx queues, for a port that uses this profile.
     *  for GREEN packets
     *  Puma 3: If UC and MC share counters this is the UC limit.
     */
    GT_U32 dp0MaxBuffNum;

    /** @brief the number of buffers that can be allocated to all of the
     *  port's TC Tx queues, for a port that uses this profile.
     *  for YELLOW packets
     *  Puma 3: If UC and MC share counters this is the UC limit.
     */
    GT_U32 dp1MaxBuffNum;

    /** @brief the number of buffers that can be allocated to all of the
     *  port's TC Tx queues, for a port that uses this profile.
     *  for RED packets
     *  Puma 3: If UC and MC share counters this is the UC limit.
     */
    GT_U32 dp2MaxBuffNum;

    /** @brief the number of descriptors that can be allocated to all of
     *  the port's TC Tx queues, for a port that uses this
     *  profile. for GREEN packets
     */
    GT_U32 dp0MaxDescrNum;

    /** @brief the number of descriptors that can be allocated to all of
     *  the port's TC Tx queues, for a port that uses this
     *  profile. for Yellow packets
     */
    GT_U32 dp1MaxDescrNum;

    /** @brief the number of descriptors that can be allocated to all of
     *  the port's TC Tx queues, for a port that uses this
     *  profile. for RED packets
     */
    GT_U32 dp2MaxDescrNum;

    /** @brief the number of buffers that can be allocated to all of the
     *  port's TC Tx queues, for a port that uses this profile.
     *  (APPLICABLE DEVICES: Puma3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 tcMaxBuffNum;

    /** @brief the number of descriptors that can be allocated to all of
     *  the port's TC Tx queues, for a port that uses this
     *  (APPLICABLE DEVICES: Puma3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 tcMaxDescrNum;

    /** @brief the number of multicast buffers that can be allocated to all of the
     *  port's TC Tx queues, for a port that uses this profile.
     *  for GREEN packets
     *  (APPLICABLE DEVICES: Puma3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 dp0MaxMCBuffNum;

    /** @brief the number of multicast buffers that can be allocated to all of the
     *  port's TC Tx queues, for a port that uses this profile.
     *  for YELLOW packets
     *  (APPLICABLE DEVICES: Puma3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 dp1MaxMCBuffNum;

    /** @brief the number of multicast buffers that can be allocated to all of the
     *  port's TC Tx queues, for a port that uses this profile.
     *  for RED packets
     *  (APPLICABLE DEVICES: Puma3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_U32 dp2MaxMCBuffNum;

    /** @brief disable unicast and multicast traffic counting in the same buffer counters.
     *  (APPLICABLE DEVICES: Puma3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL sharedUcAndMcCountersDisable;
    /** @brief the ratio of the free buffers that can be used for the queue
     *  threshold. For GREEN packets
     */
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT dp0QueueAlpha;

    /** @brief the ratio of the free buffers that can be used for the queue
     *  threshold. For YELLOW packets
     */
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT dp1QueueAlpha;

    /** @brief the ratio of the free buffers that can be used for the queue
     *  threshold. For RED packets
     */
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT dp2QueueAlpha;
} PRV_TGF_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS;

/**
* @internal prvTgfPortDeviceNumByPortGet function
* @endinternal
*
* @brief   Gets device number for specified port number.
*
* @param[in] portNum                  - port number
*
* @param[out] devNumPtr                - (pointer to)device number
*                                      None.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on port number not found in port list
*/
GT_STATUS prvTgfPortDeviceNumByPortGet
(
    IN  GT_U32                          portNum,
    OUT GT_U8                          *devNumPtr
);

/******************************************************************************\
 *                       CPSS generic API section                             *
\******************************************************************************/

/**
* @internal prvTgfPortForceLinkDownEnableSet function
* @endinternal
*
* @brief   Enable/disable Force Link Down on specified port on specified device.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfPortForceLinkDownEnableSet
(
    IN GT_U32                          portNum,
    IN GT_BOOL                        enable
);

/**
* @internal prvTgfPortEgressCntrModeSet function
* @endinternal
*
* @brief   Configure a set of egress counters that work in specified bridge egress
*         counters mode on specified device.
* @param[in] cntrSetNum               - counter set number.
* @param[in] setModeBmp               - counter mode bitmap. For example:
*                                      GT_EGRESS_CNT_PORT | GT_EGRESS_CNT_VLAN | GT_EGRESS_CNT_DP.
* @param[in] portNum                  - physical port number, if corresponding bit in setModeBmp is 1.
* @param[in] vlanId                   - VLAN Id, if corresponding bit in setModeBmp is 1.
* @param[in] tc                       - traffic class queue, if corresponding bit in setModeBmp is 1.
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
GT_STATUS prvTgfPortEgressCntrModeSet
(
    IN  GT_U8                           cntrSetNum,
    IN  CPSS_PORT_EGRESS_CNT_MODE_ENT   setModeBmp,
    IN  GT_U32                          portNum,
    IN  GT_U16                          vlanId,
    IN  GT_U8                           tc,
    IN  CPSS_DP_LEVEL_ENT               dpLevel
);

/**
* @internal prvTgfPortEgressCntrModeGet function
* @endinternal
*
* @brief   Get configuration of a bridge egress counters set.
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
GT_STATUS prvTgfPortEgressCntrModeGet
(
    IN   GT_U8                           devNum,
    IN   GT_U8                           cntrSetNum,
    OUT  CPSS_PORT_EGRESS_CNT_MODE_ENT   *setModeBmpPtr,
    OUT  GT_U8                           *portNumPtr,
    OUT  GT_U16                          *vlanIdPtr,
    OUT  GT_U8                           *tcPtr,
    OUT  CPSS_DP_LEVEL_ENT               *dpLevelPtr
);

/**
* @internal prvTgfPortEgressCntrsGet function
* @endinternal
*
* @brief   Gets a egress counters from specific counter-set.
*
* @param[in] cntrSetNum               - counter set number : 0, 1
*
* @param[out] egrCntrPtr               - (pointer to) structure of egress counters current values.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortEgressCntrsGet
(
    IN  GT_U8                       cntrSetNum,
    OUT CPSS_PORT_EGRESS_CNTR_STC   *egrCntrPtr
);

/**
* @internal prvTgfPortFCPacketsCntrsGet function
* @endinternal
*
* @brief   Get the number of received and dropped Flow Control packets
*
* @param[out] receivedCntPtr           - the number of received packets
* @param[out] droppedCntPtr            - the number of dropped packets
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortFCPacketsCntrsGet
(
    OUT GT_U32      *receivedCntPtr,
    OUT GT_U32      *droppedCntPtr
);

/**
* @internal prvTgfPortPFCAllPortsReceptionEnableSet function
* @endinternal
*
* @brief   Enable/disable PFC packets reception on all ports
*
* @param[in] enable                   -  or disbale reception.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortPFCAllPortsReceptionEnableSet
(
   IN GT_BOOL enable
);

/**
* @internal prvTgfPortMacCountersCaptureOnPortGet function
* @endinternal
*
* @brief   Gets captured Ethernet MAC counter for a particular Port.
*
* @param[in] portNum                  - physical port number
*
* @param[out] portMacCounterSetArrayPtr - (pointer to) array of current counter values.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - previous capture operation isn't finished.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note Overiding of previous capture by another port in a group is occurs,
*       when the capture triggered port
*       (see cpssDxChPortMacCountersCaptureTriggerSet) and previous
*       capture triggered port are in the same group.
*       (This comment dosn't connected to HyperG.Stack ports.)
*       Not supported counters: CPSS_BAD_PKTS_RCV_E, CPSS_UNRECOG_MAC_CNTR_RCV_E,
*       CPSS_BadFC_RCV_E, CPSS_GOOD_PKTS_RCV_E, CPSS_GOOD_PKTS_SENT_E.
*
*/
GT_STATUS prvTgfPortMacCountersCaptureOnPortGet
(
    IN  GT_U32                           portNum,
    OUT CPSS_PORT_MAC_COUNTER_SET_STC   *portMacCounterSetArrayPtr
);

/**
* @internal prvTgfPortMacCountersOnPortGet function
* @endinternal
*
* @brief   Gets Ethernet MAC counter for a particular Port.
*
* @param[in] portNum                  - physical port number
*
* @param[out] portMacCounterSetArrayPtr - (pointer to) array of current counter values.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - previous capture operation isn't finished.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note Not supported counters: CPSS_BAD_PKTS_RCV_E, CPSS_UNRECOG_MAC_CNTR_RCV_E,
*       CPSS_BadFC_RCV_E, CPSS_GOOD_PKTS_RCV_E, CPSS_GOOD_PKTS_SENT_E.
*
*/
GT_STATUS prvTgfPortMacCountersOnPortGet
(
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PORT_MAC_COUNTER_SET_STC   *portMacCounterSetArrayPtr
);

/**
* @internal prvTgfPortMacCountersRxHistogramEnable function
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
* @param[in] portNum                  - physical port number
* @param[in] enable                   -  updating of the counters for received packets
*                                      GT_FALSE - Counters are updated.
*                                      GT_TRUE - Counters are not updated.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - previous capture operation isn't finished.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortMacCountersRxHistogramEnable
(
    IN GT_U32    portNum,
    IN GT_BOOL  enable
);

/**
* @internal prvTgfPortMacCountersTxHistogramEnable function
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
* @param[in] portNum                  - physical port number
* @param[in] enable                   -  updating of the counters for transmitted packets
*                                      GT_FALSE - Counters are updated.
*                                      GT_TRUE - Counters are not updated.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - previous capture operation isn't finished.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortMacCountersTxHistogramEnable
(
    IN GT_U32    portNum,
    IN GT_BOOL  enable
);

/**
* @internal prvTgfPortMacCountersClearOnReadSet function
* @endinternal
*
* @brief   Enable or disable MAC Counters Clear on read per group of ports
*         for Tri-Speed Ports or per port for HyperG.Stack ports.
*         For Tri-Speed Ports groups are:
*         - Ports 0 through 5
*         - Ports 6 through 11
*         - Ports 12 through 17
*         - Ports 18 through 23
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
GT_STATUS prvTgfPortMacCountersClearOnReadSet
(
    IN GT_U32    portNum,
    IN GT_BOOL  enable
);

/**
* @internal prvTgfPortMacCountersClearOnReadGet function
* @endinternal
*
* @brief   Get status (Enable or disable) of MAC Counters Clear on read per group of ports
*         for Tri-Speed Ports or per port for HyperG.Stack ports.
*         For Tri-Speed Ports groups are:
*         - Ports 0 through 5
*         - Ports 6 through 11
*         - Ports 12 through 17
*         - Ports 18 through 23
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
GT_STATUS prvTgfPortMacCountersClearOnReadGet
(
    IN GT_U8    portNum,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal prvTgfPortMruSet function
* @endinternal
*
* @brief   Sets the Maximal Receive Packet size for specified port
*         on specified device.
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] mru                      - max receive packet size in bytes. (0..16382)
*                                      value must be even
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortMruSet
(
    IN GT_U32    portNum,
    IN GT_U32   mru
);

/**
* @internal prvTgfPortMruGet function
* @endinternal
*
* @brief   Sets the Maximal Receive Packet size for specified port
*         on specified device.
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] mruPtr                   - max receive packet size in bytes.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvTgfPortMruGet
(
    IN  GT_U32    portNum,
    OUT GT_U32   *mruPtr
);

/**
* @internal prvTgfPortFcHolSysModeSet function
* @endinternal
*
* @brief   Set Flow Control or HOL system mode on the specified device.
*
* @param[in] devNum                   - physical device number
* @param[in] modeFcHol                - Flow Control or HOL system mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortFcHolSysModeSet
(
    IN  GT_U8                   devNum,
    IN  PRV_TGF_PORT_HOL_FC_ENT modeFcHol
);

/**
* @internal prvTgfPortTxTailDropProfileSet function
* @endinternal
*
* @brief Configures dynamic limit alpha and sets maximal port's limits of buffers
*        and descriptors.
*
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated
* @param[in] portAlpha                - ratio of the free buffers used for the port
*                                       thresholds.
* @param[in] portMaxBuffLimit         - maximal number of buffers for a port.
* @param[in] portMaxDescrLimit        - maximal number of descriptors for a port.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number or profile set
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxTailDropProfileSet
(
    IN  GT_U8                                devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT    profileSet,
    IN  CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT portAlpha,
    IN  GT_U32                               portMaxBuffLimit,
    IN  GT_U32                               portMaxDescrLimit
);

/**
* @internal prvTgfPortTxTailDropProfileWithoutAlphaSet function
* @endinternal
*
* @brief Set maximal port's limits of buffers and descriptors. Does not configure
*        dynamic limit alpha.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number or profile set
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated
* @param[in] portMaxBuffLimit         - maximal number of buffers for a port.
* @param[in] portMaxDescrLimit        - maximal number of descriptors for a port.
*/
GT_STATUS prvTgfPortTxTailDropProfileWithoutAlphaSet
(
    IN  GT_U8                                devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT    profileSet,
    IN  GT_U32                               portMaxBuffLimit,
    IN  GT_U32                               portMaxDescrLimit
);

/**
* @internal prvTgfPortTxTailDropProfileGet function
* @endinternal
*
* @brief Get dynamic limit alpha of an egress port and get maximal port's limits
*        of buffers and descriptors.
*
*
* @param[in] devNum                    - physical device number
* @param[in] profileSet                - the Profile Set in which the Traffic
*                                        Class Drop Parameters is associated
*
* @param[out] portAlphaPtr             - Pointer to ratio of the free buffers used for the
*                                        port thresholds.
* @param[out] portMaxBuffLimitPtr      - Pointer to maximal number of buffers for a port
* @param[out] portMaxDescrLimitPtr     - Pointer to maximal number of descriptors for a port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number or profile set
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxTailDropProfileGet
(
    IN  GT_U8                                devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT    profileSet,
    OUT CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT *portAlphaPtr,
    OUT GT_U32                               *portMaxBuffLimitPtr,
    OUT GT_U32                               *portMaxDescrLimitPtr
);

/**
* @internal prvTgfPortTxTailDropProfileWithoutAlphaGet function
* @endinternal
*
* @brief Get maximal port's limits of buffers and descriptors.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number or profile set
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @param[in] devNum                    - physical device number
* @param[in] profileSet                - the Profile Set in which the Traffic
*                                        Class Drop Parameters is associated
* @param[out] portMaxBuffLimitPtr      - Pointer to maximal number of buffers for a port
* @param[out] portMaxDescrLimitPtr     - Pointer to maximal number of descriptors for a port
*/
GT_STATUS prvTgfPortTxTailDropProfileWithoutAlphaGet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT           profileSet,
    OUT GT_U32                                      *portMaxBuffLimitPtr,
    OUT GT_U32                                      *portMaxDescrLimitPtr
);

/**
* @internal prvTgfPortTx4TcTailDropProfileSet function
* @endinternal
*
* @brief   Set tail drop profiles limits for particular TC.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated.
* @param[in] trafficClass             - the Traffic Class associated with this set of
*                                      Drop Parameters.
* @param[in] tailDropProfileParamsPtr -
*                                      the Drop Profile Parameters to associate
*                                      with the Traffic Class in this Profile set.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTx4TcTailDropProfileSet
(
    IN    GT_U8                                   devNum,
    IN    CPSS_PORT_TX_DROP_PROFILE_SET_ENT       profileSet,
    IN    GT_U8                                   trafficClass,
    IN    PRV_TGF_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS *tailDropProfileParamsPtr
);

/**
* @internal prvTgfPortTx4TcTailDropProfileGet function
* @endinternal
*
* @brief   Get tail drop profiles limits for particular TC.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated.
* @param[in] trafficClass             - the Traffic Class associated with this set of
*                                      Drop Parameters.
*
* @param[out] tailDropProfileParamsPtr - Pointer to
*                                      the Drop Profile Parameters to associate
*                                      with the Traffic Class in this Profile.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTx4TcTailDropProfileGet
(
    IN    GT_U8                                   devNum,
    IN    CPSS_PORT_TX_DROP_PROFILE_SET_ENT       profileSet,
    IN    GT_U8                                   trafficClass,
    OUT   PRV_TGF_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS *tailDropProfileParamsPtr
);

/**
* @internal prvTgfPortTxBindPortToDpSet function
* @endinternal
*
* @brief   Bind a Physical Port to a specific Drop Profile Set.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] profileSet               - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfPortTxBindPortToDpSet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                               portNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet
);

/**
* @internal prvTgfPortTxBindPortToDpGet function
* @endinternal
*
* @brief   Get Drop Profile Set according to a Physical Port.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] profileSetPtr            - Pointer to the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxBindPortToDpGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                               portNum,
    OUT CPSS_PORT_TX_DROP_PROFILE_SET_ENT   *profileSetPtr
);

/**
* @internal prvTgfPortTxQueueTxEnableSet function
* @endinternal
*
* @brief   Enable/Disable transmission from a Traffic Class queue
*         on the specified port of specified device.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] tcQueue                  - traffic class queue on this Physical Port
* @param[in] enable                   - GT_TRUE,  transmission from the queue
*                                      GT_FALSE, disable transmission from the queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxQueueTxEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_U32     portNum,
    IN  GT_U8     tcQueue,
    IN  GT_BOOL   enable
);

/**
* @internal prvTgfPortTxQueueTxEnableGet function
* @endinternal
*
* @brief   Get the status of transmission from a Traffic Class queue
*         on the specified port of specified device (Enable/Disable).
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] tcQueue                  - traffic class queue on this Physical Port (0..7)
*
* @param[out] enablePtr                - GT_TRUE, enable transmission from the queue
*                                      GT_FALSE, disable transmission from the queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxQueueTxEnableGet
(
    IN  GT_U8     devNum,
    IN  GT_U32     portNum,
    IN  GT_U8     tcQueue,
    OUT GT_BOOL   *enablePtr
);

/**
* @internal prvTgfPortTxResourceHistogramThresholdSet function
* @endinternal
*
* @brief   Sets threshold for Histogram counter increment.
*
* @param[in] devNum                   - physical device number
* @param[in] cntrNum                  - Histogram Counter number. Range 0..3.
* @param[in] threshold                - If the Global Descriptors Counter exceeds this Threshold,
*                                      the Histogram Counter is incremented by 1.
*                                      Range 0..0x3FFF.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, cntrNum
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxResourceHistogramThresholdSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  cntrNum,
    IN  GT_U32  threshold
);

/**
* @internal prvTgfPortTxResourceHistogramCounterGet function
* @endinternal
*
* @brief   Gets Histogram Counter.
*
* @param[in] devNum                   - physical device number
* @param[in] cntrNum                  - Histogram Counter number. Range 0..3.
*
* @param[out] cntrPtr                  - (pointer to) Histogram Counter value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, cntrNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Histogram Counter is cleared on read.
*
*/
GT_STATUS prvTgfPortTxResourceHistogramCounterGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  cntrNum,
    OUT GT_U32  *cntrPtr
);

/**
* @internal prvTgfPortTxGlobalBufNumberGet function
* @endinternal
*
* @brief   Gets total number of virtual buffers enqueued.
*
* @param[in] devNum                   - physical device number
*
* @param[out] numberPtr                - (pointer to) number of buffers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The buffers associated with enqueued multi-target descriptors are
*       counted multiple times, once for each multi-target descriptor instance.
*
*/
GT_STATUS prvTgfPortTxGlobalBufNumberGet
(
    IN  GT_U8  devNum,
    OUT GT_U32 *numberPtr
);

/**
* @internal prvTgfPortTxGlobalDescNumberGet function
* @endinternal
*
* @brief   Gets total number of descriptors allocated.
*
* @param[in] devNum                   - physical device number
*
* @param[out] numberPtr                - (pointer to) number of descriptors
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxGlobalDescNumberGet
(
    IN  GT_U8  devNum,
    OUT GT_U32 *numberPtr
);

/**
* @internal prvTgfPortTxSharedResourceDescNumberGet function
* @endinternal
*
* @brief   Gets total number of descriptors allocated from the shared descriptors
*         pool.
* @param[in] devNum                   - physical device number
* @param[in] poolNum                  - shared pool number, range 0..7
*
* @param[out] numberPtr                - (pointer to) number of descriptors
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong poolNum or devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxSharedResourceDescNumberGet
(
    IN  GT_U8  devNum,
    IN  GT_U32 poolNum,
    OUT GT_U32 *numberPtr
);

/**
* @internal prvTgfPortTxSharedResourceBufNumberGet function
* @endinternal
*
* @brief   Gets the number of virtual buffers enqueued from the shared buffer pool.
*
* @param[in] devNum                   - physical device number
* @param[in] poolNum                  - shared pool number, range 0..7
*
* @param[out] numberPtr                - (pointer to) number of buffers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong poolNum or devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxSharedResourceBufNumberGet
(
    IN  GT_U8  devNum,
    IN  GT_U32 poolNum,
    OUT GT_U32 *numberPtr
);
/**
* @internal prvTgfPortTxSniffedBuffersLimitSet function
* @endinternal
*
* @brief   Set maximal buffers limits for mirrored packets.
*
* @param[in] devNum                   - physical device number
* @param[in] rxSniffMaxBufNum         - The number of buffers allocated for packets
*                                      forwarded to the ingress analyzer port due to
*                                      mirroring. Range 0..0xFFFF
* @param[in] txSniffMaxBufNum         - The number of buffers allocated for packets
*                                      forwarded to the egress analyzer port due to
*                                      mirroring. Range 0..0xFFFF
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range value
*/
GT_STATUS prvTgfPortTxSniffedBuffersLimitSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  rxSniffMaxBufNum,
    IN  GT_U32  txSniffMaxBufNum
);

/**
* @internal prvTgfPortTxSniffedPcktDescrLimitSet function
* @endinternal
*
* @brief   Set maximal descriptors limits for mirrored packets.
*
* @param[in] devNum                   - physical device number
* @param[in] rxSniffMaxDescNum        - The number of descriptors allocated for packets
*                                      forwarded to the ingress analyzer port due to
*                                      mirroring.
*                                      For DxCh1,DxCh2: range 0..0xFFF
*                                      For DxCh3,xCat,xCat2: range 0..0x3FFF
*                                      For Lion: range 0..0xFFFF
* @param[in] txSniffMaxDescNum        - The number of descriptors allocated for packets
*                                      forwarded to the egress analyzer port due to
*                                      mirroring.
*                                      For DxCh,DxCh2: range 0..0xFFF
*                                      For DxCh3,xCat,xCat2: range 0..0x3FFF
*                                      For Lion: range 0..0xFFFF
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range value
*/
GT_STATUS prvTgfPortTxSniffedPcktDescrLimitSet
(
    IN    GT_U8                  devNum,
    IN    GT_U32                 rxSniffMaxDescNum,
    IN    GT_U32                 txSniffMaxDescNum
);

/**
* @internal prvTgfPortTxSniffedPcktDescrLimitGet function
* @endinternal
*
* @brief   Get maximal descriptors limits for mirrored packets.
*
* @param[in] devNum                   - physical device number
*
* @param[out] rxSniffMaxDescNumPtr     - (pointer to) The number of descriptors allocated
*                                      for packets forwarded to the ingress analyzer
*                                      port due to mirroring.
* @param[out] txSniffMaxDescNumPtr     - (pointer to) The number of descriptors allocated
*                                      for packets forwarded to the egress analyzer
*                                      port due to mirroring.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxSniffedPcktDescrLimitGet
(
    IN    GT_U8                  devNum,
    OUT   GT_U32                *rxSniffMaxDescNumPtr,
    OUT   GT_U32                *txSniffMaxDescNumPtr
);

/**
* @internal prvTgfPortTxSniffedDescNumberGet function
* @endinternal
*
* @brief   Gets total number of mirrored descriptors allocated.
*
* @param[in] devNum                   - physical device number
*
* @param[out] rxNumberPtr              - (pointer to) number of ingress mirrored descriptors.
* @param[out] txNumberPtr              - (pointer to) number of egress mirrored descriptors.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxSniffedDescNumberGet
(
    IN  GT_U8  devNum,
    OUT GT_U32 *rxNumberPtr,
    OUT GT_U32 *txNumberPtr
);

/**
* @internal prvTgfPortTxSniffedBufNumberGet function
* @endinternal
*
* @brief   Gets total number of mirrored buffers allocated.
*
* @param[in] devNum                   - physical device number
*
* @param[out] rxNumberPtr              - (pointer to) number of ingress mirrored buffers.
* @param[out] txNumberPtr              - (pointer to) number of egress mirrored buffers.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxSniffedBufNumberGet
(
    IN  GT_U8  devNum,
    OUT GT_U32 *rxNumberPtr,
    OUT GT_U32 *txNumberPtr
);

/**
* @internal prvTgfPortTxMcastDescNumberGet function
* @endinternal
*
* @brief   Gets total number of multi-target descriptors allocated.
*
* @param[in] devNum                   - physical device number
*
* @param[out] numberPtr                - (pointer to) number of descriptors
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Multi-target descriptors are descriptors that are duplicated to
*       multiple egress targets, e.g. Multicast, Broadcast, or unknown Unicast
*       flooded packets.
*
*/
GT_STATUS prvTgfPortTxMcastDescNumberGet
(
    IN  GT_U8  devNum,
    OUT GT_U32 *numberPtr
);

/**
* @internal prvTgfPortTxMcastBufNumberGet function
* @endinternal
*
* @brief   Gets total number of multi-target buffers allocated (virtual buffers).
*
* @param[in] devNum                   - physical device number
*
* @param[out] numberPtr                - (pointer to) number of buffers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The buffers associated with enqueued multi-target descriptors are
*       counted multiple times, once for each multi-target descriptor instance.
*       Multi-target descriptors are descriptors that are duplicated to
*       multiple egress targets, e.g. Multicast, Broadcast, or unknown Unicast
*       flooded packets.
*
*/
GT_STATUS prvTgfPortTxMcastBufNumberGet
(
    IN  GT_U8  devNum,
    OUT GT_U32 *numberPtr
);

/**
* @internal prvTgfPortTxTcProfileSharedPoolSet function
* @endinternal
*
* @brief   Sets the shared pool associated for traffic class and Profile.
*
* @param[in] devNum                   - physical device number
* @param[in] pfSet                    - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated
* @param[in] tc                       - the Traffic Class, range 0..7.
* @param[in] poolNum                  - shared pool associated, range 0..7.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, pfSet
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxTcProfileSharedPoolSet
(
    IN  GT_U8                             devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT pfSet,
    IN  GT_U8                             tc,
    IN  GT_U32                            poolNum
);

/**
* @internal prvTgfPortTxTcProfileSharedPoolGet function
* @endinternal
*
* @brief   Gets the shared pool associated for traffic class and Profile.
*
* @param[in] devNum                   - physical device number
* @param[in] pfSet                    - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated
* @param[in] tc                       - the Traffic Class, range 0..7.
*
* @param[out] poolNumPtr               - (pointer to) shared pool associated.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, pfSet
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxTcProfileSharedPoolGet
(
    IN  GT_U8                             devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT pfSet,
    IN  GT_U8                             tc,
    OUT GT_U32                            *poolNumPtr
);

/**
* @internal prvTgfPortTxSharedPoolLimitsSet function
* @endinternal
*
* @brief   Set maximal descriptors and buffers limits for shared pool.
*
* @param[in] devNum                   - physical device number
* @param[in] poolNum                  - Shared pool number. Range 0..7
* @param[in] maxBufNum                - The number of buffers allocated for a shared pool.
*                                      Range 0..0x3FFF.
* @param[in] maxDescNum               - The number of descriptors allocated for a shared pool.
*                                      Range 0..0x3FFF.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range value
*/
GT_STATUS prvTgfPortTxSharedPoolLimitsSet
(
    IN  GT_U8  devNum,
    IN  GT_U32 poolNum,
    IN  GT_U32 maxBufNum,
    IN  GT_U32 maxDescNum
);

/**
* @internal prvTgfPortTxSharedPoolLimitsGet function
* @endinternal
*
* @brief   Get maximal descriptors and buffers limits for shared pool.
*
* @param[in] devNum                   - physical device number
* @param[in] poolNum                  - Shared pool number. Range 0..7
*
* @param[out] maxBufNumPtr             - (pointer to) The number of buffers allocated for a
*                                      shared pool.
* @param[out] maxDescNumPtr            - (pointer to) The number of descriptors allocated
*                                      for a shared pool.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxSharedPoolLimitsGet
(
    IN  GT_U8  devNum,
    IN  GT_U32 poolNum,
    OUT GT_U32 *maxBufNumPtr,
    OUT GT_U32 *maxDescNumPtr
);

/**
* @internal prvTgfPortTxTcSharedProfileEnableSet function
* @endinternal
*
* @brief   Enable/Disable usage of the shared descriptors / buffer pool for
*         packets with the traffic class (tc) that are transmited via a
*         port that is associated with the Profile (pfSet).
* @param[in] devNum                   - physical device number
* @param[in] pfSet                    - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated
* @param[in] tc                       - the Traffic Class, range 0..7.
* @param[in] enableMode               - Drop Precedence (DPs) enabled mode for sharing.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, pfSet
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxTcSharedProfileEnableSet
(
    IN  GT_U8                              devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT  pfSet,
    IN  GT_U8                              tc,
    IN  PRV_TGF_PORT_TX_SHARED_DP_MODE_ENT enableMode
);

/**
* @internal prvTgfPortTxTcSharedProfileEnableGet function
* @endinternal
*
* @brief   Gets usage of the shared descriptors / buffer pool status for
*         packets with the traffic class (tc) that are transmited via a
*         port that is associated with the Profile (pfSet).
* @param[in] devNum                   - physical device number
* @param[in] pfSet                    - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated
* @param[in] tc                       - the Traffic Class, range 0..7.
*
* @param[out] enableModePtr            - (pointer to) Drop Precedence (DPs) enabled mode for
*                                      sharing.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, pfSet
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxTcSharedProfileEnableGet
(
    IN  GT_U8                              devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT  pfSet,
    IN  GT_U8                              tc,
    OUT PRV_TGF_PORT_TX_SHARED_DP_MODE_ENT *enableModePtr
);

/**
* @internal prvTgfPortTxProfileWeightedRandomTailDropEnableSet function
* @endinternal
*
* @brief   Enable/disable Weighted Random Tail Drop Threshold to overcome
*         synchronization.
* @param[in] devNum                   - device number.
* @param[in] pfSet                    - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated.
* @param[in] dp                       - Drop Precedence
*                                      (APPLICABLE RANGES: Lion 0..2; xCat2 0..1)
* @param[in] tc                       - the Traffic Class, range 0..7.
*                                      (APPLICABLE DEVICES xCat2)
* @param[in] enablersPtr              - (pointer to) Tail Drop limits enabling
*                                      for Weigthed Random Tail Drop
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on invalid input paramters value
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxProfileWeightedRandomTailDropEnableSet
(
    IN GT_U8                                       devNum,
    IN CPSS_PORT_TX_DROP_PROFILE_SET_ENT           pfSet,
    IN GT_U32                                      dp,
    IN GT_U8                                       tc,
    IN PRV_TGF_PORT_TX_TAIL_DROP_WRTD_ENABLERS_STC *enablersPtr
);

/**
* @internal prvTgfPortTxTailDropWrtdMasksSet function
* @endinternal
*
* @brief   Sets Weighted Random Tail Drop (WRTD) masks.
*
* @param[in] devNum                   - physical device number
* @param[in] maskLsbPtr               - WRTD masked least significant bits.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxTailDropWrtdMasksSet
(
    IN GT_U8                         devNum,
    IN PRV_TGF_PORT_TX_WRTD_MASK_LSB *maskLsbPtr
);

/**
* @internal prvTgfPortTxMcastBuffersLimitSet function
* @endinternal
*
* @brief   Set maximal buffers limits for multicast packets.
*
* @param[in] devNum                   - physical device number
* @param[in] mcastMaxBufNum           - The number of buffers allocated for multicast
*                                      packets. Range 0..0xFFFF
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range mcastMaxBufNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxMcastBuffersLimitSet
(
    IN  GT_U8  devNum,
    IN  GT_U32 mcastMaxBufNum
);

/**
* @internal prvTgfPortTxMcastBuffersLimitGet function
* @endinternal
*
* @brief   Get maximal buffers limits for multicast packets.
*
* @param[in] devNum                   - physical device number
*
* @param[out] mcastMaxBufNumPtr        - (pointer to) the number of buffers allocated
*                                      for multicast packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxMcastBuffersLimitGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *mcastMaxBufNumPtr
);

/**
* @internal prvTgfPortTxMcastPcktDescrLimitSet function
* @endinternal
*
* @brief   Set maximal descriptors limits for multicast packets.
*
* @param[in] devNum                   - physical device number
* @param[in] mcastMaxDescNum          - The number of descriptors allocated for multicast
*                                      packets.
*                                      For all devices except Lion and above: in units of 128
*                                      descriptors, the actual number descriptors
*                                      allocated will be 128  mcastMaxDescNum.
*                                      For Lion and above: actual descriptors number (granularity
*                                      of 1).
*                                      For DxCh,DxCh2,DxCh3,DxChXcat: range 0..0xF80
*                                      For Lion and above: range 0..0xFFFF
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxMcastPcktDescrLimitSet
(
    IN    GT_U8  devNum,
    IN    GT_U32 mcastMaxDescNum
);

/**
* @internal prvTgfPortTxMcastPcktDescrLimitGet function
* @endinternal
*
* @brief   Get maximal descriptors limits for multicast packets.
*
* @param[in] devNum                   - physical device number
*
* @param[out] mcastMaxDescNumPtr       - (pointer to) the number of descriptors allocated
*                                      for multicast packets.
*                                      For all devices except Lion and above: in units of 128
*                                      descriptors, the actual number descriptors
*                                      allocated will be 128  mcastMaxDescNum.
*                                      For Lion and above: actual descriptors number (granularity
*                                      of 1).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxMcastPcktDescrLimitGet
(
    IN  GT_U8  devNum,
    OUT GT_U32 *mcastMaxDescNumPtr
);

/**
* @internal prvTgfPortTxQueueingEnableSet function
* @endinternal
*
* @brief   Enable/Disable enqueuing to a Traffic Class queue
*         on the specified port of specified device.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] tcQueue                  - traffic class queue on this device (0..7)
* @param[in] enable                   - GT_TRUE,  enqueuing to the queue
*                                      GT_FALSE, disable enqueuing to the queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxQueueingEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_U32     portNum,
    IN  GT_U8     tcQueue,
    IN  GT_BOOL   enable
);

/**
* @internal prvTgfPortTxQueueingEnableGet function
* @endinternal
*
* @brief   Get the status of enqueuing to a Traffic Class queue
*         on the specified port of specified device (Enable/Disable).
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] tcQueue                  - traffic class queue on this device (0..7)
*
* @param[out] enablePtr                - GT_TRUE, enable enqueuing to the queue
*                                      GT_FALSE, disable enqueuing to the queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxQueueingEnableGet
(
    IN  GT_U8     devNum,
    IN  GT_U8     portNum,
    IN  GT_U8     tcQueue,
    OUT GT_BOOL   *enablePtr
);

/**
* @internal prvTgfPortSpeedGet function
* @endinternal
*
* @brief   Gets speed for specified port on specified device.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] speedPtr                 - pointer to actual port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - on no initialized SERDES per port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1.This API also checks if at least one serdes per port was initialized.
*       In case there was no initialized SERDES per port GT_NOT_INITIALIZED is
*       returned.
*       2.Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
GT_STATUS prvTgfPortSpeedGet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT CPSS_PORT_SPEED_ENT   *speedPtr
);

/**
* @internal prvTgfPortFlowControlEnableSet function
* @endinternal
*
* @brief   Enable/disable receiving and transmission of 802.3x Flow Control frames
*         in full duplex on specified port on specified device.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] state                    - Flow Control state: Both disabled,
*                                      Both enabled, Only Rx or Only Tx enabled.
*                                      Note: only XG ports can be configured in all 4 options,
*                                      Tri-Speed and FE ports may use only first two.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, state or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortFlowControlEnableSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                       portNum,
    IN  CPSS_PORT_FLOW_CONTROL_ENT  state
);

/**
* @internal prvTgfPortTxSharingGlobalResourceEnableSet function
* @endinternal
*
* @brief   Enable/Disable sharing of resources for enqueuing of packets.
*
* @param[in] devNum                   - physical device number
* @param[in] enable                   - GT_TRUE   -  TX queue resourses sharing
*                                      GT_FALSE - disable TX queue resources sharing
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxSharingGlobalResourceEnableSet
(
    IN  GT_U8   devNum,
    IN  GT_BOOL enable
);

/**
* @internal prvTgfPortTxSchedulerArbGroupSet function
* @endinternal
*
* @brief   Set Traffic Class Queue scheduling arbitration group on
*         specificed profile of specified device.
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Tx Queue scheduler Profile Set in which the arbGroup
*                                      parameter is associated.
* @param[in] tcQueue                  - traffic class queue
* @param[in] arbGroup                 - scheduling arbitration group:
*                                      1) Strict Priority
*                                      2) WRR Group 1
*                                      3) WRR Group 0
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong profile number, device or arbGroup
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - previous updated operation not yet completed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxSchedulerArbGroupSet
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  profileSet,
    IN  GT_U32                                  tcQueue,
    IN  CPSS_PORT_TX_Q_ARB_GROUP_ENT            arbGroup
);

/**
* @internal prvTgfPortTxSchedulerArbGroupGet function
* @endinternal
*
* @brief   Get Traffic Class Queue scheduling arbitration group on
*         specificed profile of specified device.
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Tx Queue scheduler Profile Set in which the arbGroup
*                                      parameter is associated.
* @param[in] tcQueue                  - traffic class queue
*
* @param[out] arbGroupPtr              - Pointer to scheduling arbitration group:
*                                      1) Strict Priority
*                                      2) WRR Group 1
*                                      3) WRR Group 0
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong profile number, device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxSchedulerArbGroupGet
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  profileSet,
    IN  GT_U32                                  tcQueue,
    OUT CPSS_PORT_TX_Q_ARB_GROUP_ENT            *arbGroupPtr
);

/**
* @internal prvTgfPortTxSchedulerProfileIndexSet function
* @endinternal
*
* @brief   Bind a port to scheduler profile set.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number,
*                                      or the Queue Group number in case of Aggregate Mode
*                                      in ExMxPm XG device.
* @param[in] index                    - The scheduler profile index. range 0..7.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on illegal parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxSchedulerProfileIndexSet
(
    IN  GT_U8   devNum,
    IN  GT_U8   portNum,
    IN  GT_U32  index
);

/**
* @internal prvTgfPortTxSchedulerProfileIndexGet function
* @endinternal
*
* @brief   Get scheduler profile set that is binded to the port.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number,
*                                      or the Queue Group number in case of Aggregate Mode
*                                      in ExMxPm XG device.
*
* @param[out] indexPtr                 - (pointer to) The scheduler profile index. range 0..7.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on illegal parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxSchedulerProfileIndexGet
(
    IN  GT_U8   devNum,
    IN  GT_U32   portNum,
    OUT GT_U32  *indexPtr
);

/**
* @internal prvTgfPortMaxSpeedForce function
* @endinternal
*
* @brief   Force or restore port speed, duplex and other configurations for Full
*         Wire Speed testing.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] force                    - GT_TRUE -  maximal settings and  link UP
*                                      GT_FALSE - disable force link UP
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, state or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortMaxSpeedForce
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_BOOL                     force
);

/**
* @internal prvTgfPortSpeedForce function
* @endinternal
*
* @brief   Force port speed, configure interface mode if need.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] speed                    - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, state or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortSpeedForce
(
    IN  GT_U8                       devNum,
    IN  GT_U32                       portNum,
    IN  CPSS_PORT_SPEED_ENT         speed
);

/**
* @internal prvTgfPortWsRateCheck function
* @endinternal
*
* @brief   Check rate by WS rate of port
*
* @param[in] portIdx                  - index of egress port
* @param[in] rate                     -  to check in packets per second
* @param[in] packetSize               - packet size in bytes including CRC
*
* @retval 0                        - input rate is WS
* @retval other                    - difference between input rate and WS one
*/
GT_U32 prvTgfPortWsRateCheck
(
    IN  GT_U32 portIdx,
    IN  GT_U32 rate,
    IN  GT_U32 packetSize
);

/**
* @internal prvTgfPortProfileMruSizeSet function
* @endinternal
*
* @brief   Set MRU size for MRU profile.
*         cpssDxChPortProfileMruSizeSet set MRU size for a specific MRU profile.
* @param[in] mruIndex                 - MRU profile index [0..7]
* @param[in] mruValue                 - MRU size in bytes [0..0x3FFF]
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortProfileMruSizeSet
(
    IN GT_U32    mruIndex,
    IN GT_U32    mruValue
);

/**
* @internal prvTgfPortProfileMruSizeGet function
* @endinternal
*
* @brief   Get MRU size for MRU profile.
*         cpssDxChPortProfileMruSizeSet set MRU size for a specific MRU profile.
* @param[in] mruIndex                 - MRU profile index [0..7]
*                                      mruValue - MRU size in bytes [0..0x3FFF]
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortProfileMruSizeGet
(
    IN GT_U32    mruIndex,
    OUT GT_U32    *mruValuePtr
);

/**
* @internal prvTgfPortMruExceptionCommandSet function
* @endinternal
*
* @brief   Set the command assigned to frames that exceed the default ePort MRU size.
*
* @param[in] command                  - the  valid values:
*                                      CPSS_PACKET_CMD_FORWARD_E
*                                      CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E
*                                      CPSS_PACKET_CMD_DROP_HARD_E
*                                      CPSS_PACKET_CMD_DROP_SOFT_E
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortMruExceptionCommandSet
(
    IN CPSS_PACKET_CMD_ENT      command
);

/**
* @internal prvTgfPortMruExceptionCommandGet function
* @endinternal
*
* @brief   Get the command assigned to frames that exceed the default ePort MRU size.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortMruExceptionCommandGet
(
    OUT CPSS_PACKET_CMD_ENT      *commandPtr
);

/**
* @internal prvTgfPortMruExceptionCPUCodeSet function
* @endinternal
*
* @brief   Set the CPU/drop code assigned to a frame which fails the MRU check.
*
* @param[in] cpuCode                  - the CPU/drop code
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortMruExceptionCPUCodeSet
(
    IN CPSS_NET_RX_CPU_CODE_ENT     cpuCode
);

/**
* @internal prvTgfPortMruExceptionCPUCodeGet function
* @endinternal
*
* @brief   Get the CPU/drop code assigned to a frame which fails the MRU check.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortMruExceptionCPUCodeGet
(
    OUT CPSS_NET_RX_CPU_CODE_ENT     *cpuCodePtr
);

/**
* @internal prvTgfPortMruProfileSet function
* @endinternal
*
* @brief   Set an MRU profile for port
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number, CPU port
* @param[in] profileId                - the profile index (APPLICABLE RANGES: 0..7)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortMruProfileSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_U32           profileId
);

/**
* @internal prvTgfPortMruProfileGet function
* @endinternal
*
* @brief   Get an MRU profile for port
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number, CPU port
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortMruProfileGet
(
    IN  GT_U8          devNum,
    IN  GT_PORT_NUM    portNum,
    OUT GT_U32        *profileIdPtr
);

/**
* @internal prvTgfPortEcnMarkingEnableSet function
* @endinternal
*
* @brief   Enable/Disable Explicit Congestion Notification (ECN) extension in IPv4/IPv6 header
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or control packet command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortEcnMarkingEnableSet
(
    IN CPSS_IP_PROTOCOL_STACK_ENT        cmd,
    IN GT_BOOL                           enable
);

/**
* @internal prvTgfPortEcnMarkingEnableGet function
* @endinternal
*
* @brief   Get status of Explicit Congestion Notification (ECN) extension from IPv4/IPv6 packet header
*
* @param[out] enablePtr                - GT_TRUE: ECN marking enable
*                                      GT_FALSE: ECN marking disable
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or control packet command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - wrong hardware value.
*/
GT_STATUS prvTgfPortEcnMarkingEnableGet
(
    IN CPSS_IP_PROTOCOL_STACK_ENT        cmd,
    OUT GT_BOOL                          *enablePtr
);


/**
* @internal prvTgfPortTxTailDropUcEnableSet function
* @endinternal
*
* @brief   Enable/Disable tail-dropping for all packets based on the profile limits.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -  device number.
* @param[in] enable                   -  GT_TRUE  - Tail Drop is enabled.
*                                      GT_FALSE - The Tail Drop limits for all packets are
*                                      ignored and packet is dropped only
*                                      when the Tx Queue has reached its global
*                                      descriptors limit.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxTailDropUcEnableSet
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
);

/**
* @internal prvTgfPortTxTailDropUcEnableGet function
* @endinternal
*
* @brief   Get enable/disable tail-dropping for all packets based on the profile limits.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -  device number.
*
* @param[out] enablePtr                -  pointer to tail drop status:
*                                      GT_TRUE  - Tail Drop is enabled.
*                                      GT_FALSE - The Tail Drop limits for all packets are
*                                      ignored and packet is dropped only
*                                      when the Tx Queue has reached its global
*                                      descriptors limit.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortTxTailDropUcEnableGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
);

/**
* @internal prvTgfPortEnableSet function
* @endinternal
*
* @brief   Enable/disable a specified port on specified device.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number, CPU port number
* @param[in] enable                   - GT_TRUE:   port,
*                                      GT_FALSE: disable port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Added callback bind option used to run PHYMAC configuration functions
*       Callbacks can be run before switch MAC configuration and after
*       First callback can set doPpMacConfig TRUE or FALSE. If the return value
*       is TRUE the switch MAC will be configured
*
*/
GT_STATUS prvTgfPortEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL   enable
);

/**
* @internal prvTgfPortRemotePhysicalMapAdd function
* @endinternal
*
* @brief   Add physical port map for remote port
*
* @param[in] devNum                   - physical device number
* @param[in] localPortNumForRemote    - local physical port number for remote one
* @param[in] cscdPortNum              - cascade physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_ALREADY_EXIST         - on localPortNumForRemote already in map
*/
GT_STATUS prvTgfPortRemotePhysicalMapAdd
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     localPortNumForRemote,
    IN  GT_PHYSICAL_PORT_NUM     cscdPortNum
);

/**
* @internal prvTgfPortPhysicalMapAdd function
* @endinternal
*
* @brief   Add physical port map entry
*
* @param[in] devNum                   - physical device number
* @param[in] newInfoPtr               - pointer to new MAP entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortPhysicalMapAdd
(
    IN  GT_U8                   devNum,
    IN  CPSS_DXCH_PORT_MAP_STC *newInfoPtr
);

/**
* @internal prvTgfPortPhysicalMapDelete function
* @endinternal
*
* @brief   Delete physical port map entry
*
* @param[in] devNum                   - physical device number
* @param[in] portNumToDelete          - physical port number to delete from map
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfPortPhysicalMapDelete
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNumToDelete
);

/**
* @internal prvTgfPortTxDbaEnableSet function
* @endinternal
*
* @brief Enabling DBA feature.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @param[in] devNum - physical device number.
* @param[in] enable - GT_TRUE  - enable DBA feature
*                     GT_FALSE - no DBA feature support
*/
GT_STATUS prvTgfPortTxDbaEnableSet
(
    IN    GT_U8         devNum,
    IN    GT_BOOL       enable
);

/**
* @internal prvTgfPortTxDbaEnableGet function
* @endinternal
*
* @brief Indicates if DBA feature is enabled.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - enabledPtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @param[in] devNum - physical device number.
*
* @param[out] enable - (pointer to) GT_TRUE  - enable DBA feature
*                                   GT_FALSE - no DBA feature support
*/
GT_STATUS prvTgfPortTxDbaEnableGet
(
    IN    GT_U8         devNum,
    IN    GT_BOOL       *enablePtr
);

/**
* @internal prvTgfPortTxDbaAvailBuffSet function
* @endinternal
*
* @brief Set maximal available buffers for allocation.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NO_SUCH               - on unrecognized message op code
* @retval GT_BAD_VALUE             - on wrong message parameters
* @retval GT_BAD_SIZE              - on wrong message size
* @retval GT_OUT_OF_RANGE          - on out of range number of buffers
*
* @param[in] devNum - physical device number.
* @param[in] maxBuff - Maximal available buffers for allocation.
*/
GT_STATUS prvTgfPortTxDbaAvailBuffSet
(
    IN    GT_U8         devNum,
    IN    GT_U32        maxBuff
);

/**
* @internal prvTgfPortTxDbaAvailBuffGet function
* @endinternal
*
* @brief Get maximal available buffers for allocation.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NO_SUCH               - on unrecognized message op code
* @retval GT_BAD_VALUE             - on wrong message parameters
* @retval GT_BAD_SIZE              - on wrong message @retval
*
* @param[in] devNum - physical device number.
*
* @param[out] maxBuffPtr - (pointer to) Maximal available buffers for allocation.
*/
GT_STATUS prvTgfPortTxDbaAvailBuffGet
(
    IN    GT_U8         devNum,
    IN    GT_U32        *maxBuffPtr
);

/**
 * @internal prvTgfPortTxBufferStatisticsEnableSet function
 * @endinternal
 *
 * @brief  Enable/disable port buffer statistics.
 *
 * @param[in] devNum                   - device number
 * @param[in] enPortStat               - enable port statistics
 *                                       GT_FALSE - disable port buffer statistics.
 *                                       GT_TRUE  - enable port buffer statistics.
 * @param[in] enQueueStat              - enable port queue statistics
 *                                       GT_FALSE - disable port queue buffer statistics.
 *                                       GT_TRUE  - enable port queue buffer statistics.
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 */
GT_STATUS prvTgfPortTxBufferStatisticsEnableSet
(
    IN  GT_U8                devNum,
    IN  GT_BOOL              enPortStat,
    IN  GT_BOOL              enQueueStat
);

/**
 * @internal prvTgfPortTxBufferStatisticsEnableGet function
 * @endinternal
 *
 * @brief  Get enable status of port buffer statistics.
 *
 * @param[in]  devNum                   - device number
 *
 * @param[out] enPortStatPtr            - (pointer to) enabled status of
 *                                        port buffer statistics
 * @param[out] enQueueStatPtr           - (pointer to) enabled status of
 *                                        queue buffer statistics
 *
 * @retval GT_OK                        - on success
 * @retval GT_BAD_PTR                   - on NULL pointer
 * @retval GT_BAD_PARAM                 - wrong value in any of the parameters
 * @retval GT_HW_ERROR                  - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE     - on not applicable device
 */
GT_STATUS prvTgfPortTxBufferStatisticsEnableGet
(
    IN  GT_U8                devNum,
    OUT GT_BOOL              *enPortStatPtr,
    OUT GT_BOOL              *enQueueStatPtr
);

/**
 * @internal prvTgfPortTxMaxBufferFillLevelSet function
 * @endinternal
 *
 * @brief  Set max port buffer fill level per port.
 *
 * @note   APPLICABLE DEVICES:      Aldrin2; Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X.
 *
 * @param[in] devNum                   - device number
 * @param[in] portNum                  - port number
 * @param[in] maxBuffFillLvl           - max buffer fill level (APPLICABLE RANGES: 0..0xFFFFF)
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 */
GT_STATUS prvTgfPortTxMaxBufferFillLevelSet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U32               maxBuffFillLvl
);

/**
 * @internal prvTgfPortTxMaxBufferFillLevelGet function
 * @endinternal
 *
 * @brief  Get max port buffer fill level per port.
 *
 * @param[in] devNum                   - device number
 * @param[in] portNum                  - port number
 *
 * @param[out] maxBuffFillLvlPtr       - (pointer to)max buffer fill level
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 */
GT_STATUS prvTgfPortTxMaxBufferFillLevelGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U32               *maxBuffFillLvlPtr
);

/**
 * @internal prvTgfPortTxQueueMaxBufferFillLevelSet function
 * @endinternal
 *
 * @brief  Set max port queue buffer fill level per port.
 *
 * @param[in] devNum                   - device number
 * @param[in] portNum                  - port number
 * @param[in] tc                       - traffic class queue (0..7)
 * @param[in] maxBuffFillLvl           - max buffer fill level (APPLICABLE RANGES: 0..0xFFFFF)
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 */
GT_STATUS prvTgfPortTxQueueMaxBufferFillLevelSet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U8                tc,
    IN  GT_U32               maxBuffFillLvl
);

/**
 * @internal prvTgfPortTxQueueMaxBufferFillLevelGet function
 * @endinternal
 *
 * @brief  Get max port queue buffer fill level per port.
 *
 * @param[in] devNum                   - device number
 * @param[in] portNum                  - port number
 * @param[in] tc                       - traffic class queue (0..7)
 *
 * @param[out] maxBuffFillLvlPtr       - (pointer to)max buffer fill level
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 */
GT_STATUS prvTgfPortTxQueueMaxBufferFillLevelGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U8                tc,
    OUT GT_U32               *maxBuffFillLvlPtr
);

/**
* @internal prvTgfPortModeSpeedSet function
* @endinternal
*
* @brief   Configure Interface mode and speed on a specified port and execute
*          on port's serdeses power down and power up sequence.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] ifMode                   - interface mode
* @param[in] speed                    - port data
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvTgfPortModeSpeedSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
);

/**
* @internal prvTgfPortModeSpeedSet function
* @endinternal
*
* @brief   Configure Interface mode and speed on a specified port and execute
*          on port's serdeses power down and power up sequence.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[out] ifModePtr               - (pointer to) interface mode
* @param[out] speedPtr                - (pointer to) port data
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR                  - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvTgfPortModeSpeedGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PORT_INTERFACE_MODE_ENT    *ifModePtr,
    OUT CPSS_PORT_SPEED_ENT             *speedPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __tgfPortGenh */


