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
* @file tgfOamGen.h
*
* @brief Generic API for OAM.
*
* @version   8
********************************************************************************
*/
#ifndef __tgfOamGenh
#define __tgfOamGenh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfCommon.h>

/******************************************************************************\
 *                          Private type definitions                          *
\******************************************************************************/

/**
* @enum PRV_TGF_OAM_SOURCE_INTERFACE_CHECK_MODE_ENT
 *
 * @brief Enumeration for OAM Unit Type.
*/
typedef enum{

    /** @brief Verify that the packet’s source interface
     *  does NOT match <LocalDevSRCePort / LocalDevSrcTrunk>.
     */
    PRV_TGF_OAM_SOURCE_INTERFACE_CHECK_MODE_NO_MATCH_E,

    /** @brief Verify that the packet’s source interface
     *  matches <LocalDevSRCePort / LocalDevSrcTrunk>.
     */
    PRV_TGF_OAM_SOURCE_INTERFACE_CHECK_MODE_MATCH_E

} PRV_TGF_OAM_SOURCE_INTERFACE_CHECK_MODE_ENT;

/**
* @enum PRV_TGF_OAM_LM_COUNTING_MODE_ENT
 *
 * @brief Enumeration for OAM Unit Type.
*/
typedef enum{

    /** @brief Retain: Keep the previous decision of whether to enable
     *  Policer counting for this packet or not.
     */
    PRV_TGF_OAM_LM_COUNTING_MODE_RETAIN_E = 0,

    /** Disable Counting */
    PRV_TGF_OAM_LM_COUNTING_MODE_DISABLE_E,

    /** @brief Enable Counting: If <OAM Opcode Parsing Enable> is enabled,
     *  then OAM packets with an opcode that is equal to <OAM Counted Opcode<n>>
     *  are counted by the LM counter. - If <OAM Opcode Parsing Enable> is disabled,
     *  all packets that access the OAM table are counted by the LM counter.
     */
    PRV_TGF_OAM_LM_COUNTING_MODE_ENABLE_E

} PRV_TGF_OAM_LM_COUNTING_MODE_ENT;

/**
* @struct PRV_TGF_OAM_ENTRY_STC
 *
 * @brief Structure of OAM table entry
*/
typedef struct{

    /** @brief When enabled, the packet is treated as
     *  an OAM packet;
     *  GT_FALSE - disable opcode parsing;
     *  GT_TRUE - enable opcode parsing;
     */
    GT_BOOL opcodeParsingEnable;

    /** @brief Determines if the packet’s MEG level
     *  is checked, using the <MEG level> field;
     *  GT_FALSE - disable MEG level check;
     *  GT_TRUE - enable MEG level check;
     */
    GT_BOOL megLevelCheckEnable;

    /** @brief Determines the MEG level of the current MEP;
     *  (APPLICABLE RANGES: 0..7)
     */
    GT_U32 megLevel;

    /** @brief This field determines the profile of the
     *  packet command according to the opcode
     *  to packet command table;
     *  (APPLICABLE RANGES: 0..7)
     */
    GT_U32 packetCommandProfile;

    /** @brief If the packet is trapped/mirrored to the
     *  CPU by the OAM Opcode Packet Command table,
     *  this field is added to the CPU Code;
     *  (APPLICABLE RANGES: 0..3)
     */
    GT_U32 cpuCodeOffset;

    /** @brief Determines whether the packet’s
     *  source interface is checked;
     *  GT_FALSE - disable source interface check;
     *  GT_TRUE - enable source interface check;
     */
    GT_BOOL sourceInterfaceCheckEnable;

    /** Source interface check mode */
    PRV_TGF_OAM_SOURCE_INTERFACE_CHECK_MODE_ENT sourceInterfaceCheckMode;

    /** @brief Determines source local interface: Port/Trunk.
     *  Fields relevant to Loss Measurement
     */
    CPSS_INTERFACE_INFO_STC sourceInterface;

    /** @brief Determines whether the OAM Loss Measurement
     *  counter is captured;
     *  GT_FALSE - disable loss measurement capturing;
     *  GT_TRUE - enable loss measurement capturing;
     */
    GT_BOOL lmCounterCaptureEnable;

    /** @brief When enabled, OAM packets whose opcode
     *  is <Dual-Ended Loss Measurement Opcode>
     *  are treated as Loss Measurement packets;
     *  GT_FALSE - disable dual-ended
     *  loss measurement treatment;
     *  GT_TRUE - enable dual-ended
     *  loss measurement treatment;
     */
    GT_BOOL dualEndedLmEnable;

    /** Determines which packets are counted by the LM counters. */
    PRV_TGF_OAM_LM_COUNTING_MODE_ENT lmCountingMode;

    /** @brief Relevant only for Egress OAM.
     *  An index to the OAM Offset Table,
     *  specifying the offset in the packet where
     *  the Loss Measurement/Delay Measurement counter
     *  is inserted into;
     *  (APPLICABLE RANGES: 0..15)
     *  Fields relevant to Delay Measurement
     *  (APPLICABLE RANGES: 0..15)
     */
    GT_U32 oamPtpOffsetIndex;

    /** @brief When enabled, the current timestamp
     *  is copied into the packet header,
     *  according to the configurable offset;
     *  GT_FALSE - disable OAM timestamping;
     *  GT_TRUE - enable OAM timestamping;
     *  Fields relevant to Keep Alive processing
     */
    GT_BOOL timestampEnable;

    /** @brief Determines whether the current entry is
     *  subject to the aging mechanism;
     *  GT_FALSE - disable OAM keepalive aging;
     *  GT_TRUE - enable OAM keepalive aging;
     */
    GT_BOOL keepaliveAgingEnable;

    /** @brief Determines the age state of the current
     *  entry, i.e., the number of age periods
     *  that have passed since the last
     *  keepalive reception;
     *  (APPLICABLE RANGES: 0..15)
     */
    GT_U32 ageState;

    /** @brief This index selects one of the eight entries
     *  in the Aging Period Table;
     *  (APPLICABLE RANGES: 0..7)
     */
    GT_U32 agingPeriodIndex;

    /** @brief The number of age periods that indicate
     *  loss of continuity;
     *  (APPLICABLE RANGES: 0..15)
     */
    GT_U32 agingThreshold;

    /** @brief If enabled, the correctness of the
     *  packet header is verified by comparing the
     *  packet hash[5:0] with the <Flow Hash> field;
     *  GT_FALSE - disable keepalive hash verification;
     *  GT_TRUE - enable keepalive hash verification;
     */
    GT_BOOL hashVerifyEnable;

    /** @brief When enabled, the OAM engine does not change
     *  the value of the <Flow Hash> field.
     *  When disabled, and this OAN entry is
     *  accessed, the OAM engine updates the
     *  <Flow Hash> field, and sets the
     *  <Lock Hash Value Enable> field;
     *  GT_FALSE - disable hash value updating;
     *  GT_TRUE - enable hash value updating;
     */
    GT_BOOL lockHashValueEnable;

    /** @brief When enabled, the Aging Daemon updates the
     *  <OAM Protection LOC Status<n>> when an
     *  OAM flow<n> aged out;
     *  GT_FALSE - disable OAM Protection LOC Status<n> updating;
     *  GT_TRUE - enable OAM Protection LOC Status<n> updating;
     */
    GT_BOOL protectionLocUpdateEnable;

    /** @brief Determines the hash value of the flow;
     *  (APPLICABLE RANGES: 0..4095)
     */
    GT_U32 flowHash;

    /** @brief If enabled, the current entry is subject
     *  to excess keepalive detection;
     *  GT_FALSE - disable excess keepalive detection;
     *  GT_TRUE - enable excess keepalive detection;
     */
    GT_BOOL excessKeepaliveDetectionEnable;

    /** @brief This counter is incremented by one
     *  each time the daemon updates this entry;
     *  (APPLICABLE RANGES: 0..15)
     */
    GT_U32 excessKeepalivePeriodCounter;

    /** @brief Determines the number of Aging Periods
     *  used as the observation interval for
     *  Excess Keepalive detection;
     *  (APPLICABLE RANGES: 0..15)
     */
    GT_U32 excessKeepalivePeriodThreshold;

    /** @brief This counter is incremented each time a
     *  keepalive message arrives and this OAM entry
     *  is accessed;
     *  (APPLICABLE RANGES: 0..7)
     *  excessKeepaliveMessageThreshold- Determines the minimal number of
     *  keepalive messages during the observation
     *  interval that trigger an Excess event;
     *  (APPLICABLE RANGES: 0..7)
     */
    GT_U32 excessKeepaliveMessageCounter;

    GT_U32 excessKeepaliveMessageThreshold;

    /** @brief When enabled, the packet’s RDI bit is
     *  compared to the <RDI Status<n>>;
     *  GT_FALSE - disable RDI bits comparing;
     *  GT_TRUE - enable RDI bits comparing;
     */
    GT_BOOL rdiCheckEnable;

    /** @brief This bit contains the RDI status of the
     *  current OAM flow;
     *  0 - the peer switch has not detected a failure.
     *  1 - the peer switch has detected a failure.
     */
    GT_U32 rdiStatus;

    /** @brief When enabled, the packet’s Period field is
     *  compared to the <Keepalive TX Period> Field;
     *  GT_FALSE - disable packet's Period field checking;
     *  GT_TRUE - enable packet's Period field checking;
     */
    GT_BOOL periodCheckEnable;

    /** @brief The expected transmission period field;
     *  (APPLICABLE RANGES: 0..7)
     */
    GT_U32 keepaliveTxPeriod;

} PRV_TGF_OAM_ENTRY_STC;

/**
* @enum PRV_TGF_OAM_STAGE_TYPE_ENT
 *
 * @brief Enumeration for OAM Unit Type.
*/
typedef enum{

    /** Ingress OAM stage. */
    PRV_TGF_OAM_STAGE_TYPE_INGRESS_E,

    /** Egress OAM stage. */
    PRV_TGF_OAM_STAGE_TYPE_EGRESS_E,

    /** last enumerator */
    PRV_TGF_OAM_STAGE_TYPE_LAST_E

} PRV_TGF_OAM_STAGE_TYPE_ENT;

/**
* @enum PRV_TGF_OAM_EXCEPTION_TYPE_ENT
 *
 * @brief Enumeratiom of OAM exception types
*/
typedef enum{

    /** MEG level check exception. */
    PRV_TGF_OAM_EXCEPTION_TYPE_MEG_LEVEL_E = 0,

    /** OAM source interface exception. */
    PRV_TGF_OAM_EXCEPTION_TYPE_SOURCE_INTERFACE_E,

    /** Keepalive RDI exception. */
    PRV_TGF_OAM_EXCEPTION_TYPE_RDI_STATUS_CHANGED_E,

    /** Keepalive aging exception. */
    PRV_TGF_OAM_EXCEPTION_TYPE_KEEPALIVE_AGING_E,

    /** Keepalive excess exception. */
    PRV_TGF_OAM_EXCEPTION_TYPE_KEEPALIVE_EXCESS_E,

    /** Invalid keepalive exception. */
    PRV_TGF_OAM_EXCEPTION_TYPE_KEEPALIVE_INVALID_HASH_E,

    /** TX period exception. */
    PRV_TGF_OAM_EXCEPTION_TYPE_TX_PERIOD_E,

    /** OAM summary exception. */
    PRV_TGF_OAM_EXCEPTION_TYPE_SUMMARY_E,

    /** Last OAM exception enumerator. */
    PRV_TGF_OAM_EXCEPTION_TYPE_LAST_E

} PRV_TGF_OAM_EXCEPTION_TYPE_ENT;

/**
* @struct PRV_TGF_OAM_EXCEPTION_CONFIG_STC
 *
 * @brief Structure of OAM exception.
*/
typedef struct{

    /** OAM exception packet command. */
    CPSS_PACKET_CMD_ENT command;

    /** @brief OAM exception packet CPU code.
     *  used for MIRROR_TO_CPU and TRAP packet commands.
     */
    CPSS_NET_RX_CPU_CODE_ENT cpuCode;

    /** @brief OAM exception summary bit.
     *  GT_TRUE - exception status changing is included
     *  in the OAM exception bitmap.
     *  GT_FALSE - exception status changing is NOT included
     *  in the OAM summary exception bitmap.
     */
    GT_BOOL summaryBitEnable;

} PRV_TGF_OAM_EXCEPTION_CONFIG_STC;

/**
* @enum PRV_TGF_OAM_OPCODE_TYPE_ENT
 *
 * @brief Structure of OAM opcode types
*/
typedef enum{

    /** OAM Loss Measurement Counted Opcodes. */
    PRV_TGF_OAM_OPCODE_TYPE_LM_COUNTED_E = 0,

    /** OAM SingleEnded Loss Measurement Opcodes. */
    PRV_TGF_OAM_OPCODE_TYPE_LM_SINGLE_ENDED_E,

    /** OAM DualEnded Loss Measurement Opcodes. */
    PRV_TGF_OAM_OPCODE_TYPE_LM_DUAL_ENDED_E,

    /** OAM Delay Measurement Opcodes. */
    PRV_TGF_OAM_OPCODE_TYPE_DM_E,

    /** OAM Keep Alive Opcodes. */
    PRV_TGF_OAM_OPCODE_TYPE_KEEPALIVE_E,

    /** last enumerator */
    PRV_TGF_OAM_OPCODE_TYPE_LAST_E

} PRV_TGF_OAM_OPCODE_TYPE_ENT;

/* OAM group status size */
#define PRV_TGF_OAM_GROUP_STATUS_SIZE_IN_WORDS_CNS          2

/******************************************************************************\
 *                       CPSS generic API section                             *
\******************************************************************************/

/**
* @internal prvTgfOamEntrySet function
* @endinternal
*
* @brief   Set OAM Entry configuration
*
* @param[in] stage                    - OAM  type.
* @param[in] entryIndex               - OAM entry index.
*                                      Range: see datasheet for specific device.
* @param[in] entryPtr                 - pointer to OAM entry.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum, stage, entryIndex or entry parameter.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_OUT_OF_RANGE          - one of the OAM entry fields are out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamEntrySet
(
    IN  PRV_TGF_OAM_STAGE_TYPE_ENT          stage,
    IN  GT_U32                              entryIndex,
    IN  PRV_TGF_OAM_ENTRY_STC               *entryPtr
);

/**
* @internal prvTgfOamEntryGet function
* @endinternal
*
* @brief   Get OAM Entry
*
* @param[in] stage                    - OAM  type.
* @param[in] entryIndex               - OAM entry index.
*                                      Range: see datasheet for specific device.
*
* @param[out] entryPtr                 - pointer to OAM entry.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum, stage or entryIndex.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamEntryGet
(
    IN  PRV_TGF_OAM_STAGE_TYPE_ENT          stage,
    IN  GT_U32                              entryIndex,
    OUT PRV_TGF_OAM_ENTRY_STC               *entryPtr
);

/**
* @internal prvTgfOamEnableSet function
* @endinternal
*
* @brief   Enable OAM processing
*
* @param[in] stage                    - OAM  type.
* @param[in] enable                   - enable/disable OAM processing.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamEnableSet
(
    IN  PRV_TGF_OAM_STAGE_TYPE_ENT          stage,
    IN  GT_BOOL                             enable
);

/**
* @internal prvTgfOamEnableGet function
* @endinternal
*
* @brief   Get OAM processing status
*
* @param[in] stage                    - OAM  type.
*
* @param[out] enablePtr                - pointer to enable/disable OAM processing.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamEnableGet
(
    IN  PRV_TGF_OAM_STAGE_TYPE_ENT          stage,
    OUT  GT_BOOL                            *enablePtr
);

/**
* @internal prvTgfOamTableBaseFlowIdSet function
* @endinternal
*
* @brief   Set the base first Flow ID index used for OAM.
*
* @param[in] stage                    - OAM  type.
* @param[in] baseFlowId               - base first Flow ID index used for OAM.
*                                      (APPLICABLE RANGES: 0..65534)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or stage.
* @retval GT_OUT_OF_RANGE          - base FlowId is out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note OAM indices are calculated by <Flow ID>-<OAM Table Base Flow ID>.
*
*/
GT_STATUS prvTgfOamTableBaseFlowIdSet
(
    IN  PRV_TGF_OAM_STAGE_TYPE_ENT          stage,
    IN  GT_U32                              baseFlowId
);

/**
* @internal prvTgfOamTableBaseFlowIdGet function
* @endinternal
*
* @brief   Get the base first Flow ID index used for OAM.
*
* @param[in] stage                    - OAM  type
*
* @param[out] baseFlowIdPtr            - pointer to first Flow ID index used for OAM.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or stage.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note OAM indices are calculated by <Flow ID>-<OAM Table Base Flow ID>.
*
*/
GT_STATUS prvTgfOamTableBaseFlowIdGet
(
    IN  PRV_TGF_OAM_STAGE_TYPE_ENT          stage,
    OUT  GT_U32                            *baseFlowIdPtr
);


/**
* @internal prvTgfOamExceptionConfigSet function
* @endinternal
*
* @brief   Set OAM exception configuration
*
* @param[in] stage                    - OAM  type.
* @param[in] exceptionType            - OAM exception type.
* @param[in] exceptionConfigPtr       - OAM exception configuration.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum, stage, exceptionType or
*                                       PRV_TGF_OAM_EXCEPTION_CONFIG_STC fields.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamExceptionConfigSet
(
    IN  PRV_TGF_OAM_STAGE_TYPE_ENT              stage,
    IN  PRV_TGF_OAM_EXCEPTION_TYPE_ENT          exceptionType,
    IN  PRV_TGF_OAM_EXCEPTION_CONFIG_STC        *exceptionConfigPtr
);

/**
* @internal prvTgfOamExceptionConfigGet function
* @endinternal
*
* @brief   Get OAM exception configuration
*
* @param[in] stage                    - OAM  type.
* @param[in] exceptionType            - OAM exception type.
*
* @param[out] exceptionConfigPtr       - pointer to OAM exception configuration.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum, stage or exceptionType.
* @retval GT_BAD_STATE             - on bad value found in HW
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamExceptionConfigGet
(
    IN  PRV_TGF_OAM_STAGE_TYPE_ENT              stage,
    IN  PRV_TGF_OAM_EXCEPTION_TYPE_ENT          exceptionType,
    OUT PRV_TGF_OAM_EXCEPTION_CONFIG_STC        *exceptionConfigPtr
);

/**
* @internal prvTgfOamExceptionCounterGet function
* @endinternal
*
* @brief   Get OAM exception counter value
*
* @param[in] stage                    - OAM  type.
* @param[in] exceptionType            - OAM exception type.
*
* @param[out] counterValuePtr          - pointer to OAM exception counter value.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum, stage or exceptionType.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamExceptionCounterGet
(
    IN  PRV_TGF_OAM_STAGE_TYPE_ENT              stage,
    IN  PRV_TGF_OAM_EXCEPTION_TYPE_ENT          exceptionType,
    OUT GT_U32                                  *counterValuePtr
);

/**
* @internal prvTgfOamExceptionGroupStatusGet function
* @endinternal
*
* @brief   Get OAM exception group status
*
* @param[in] stage                    - OAM  type.
* @param[in] exceptionType            - OAM exception type.
*
* @param[out] groupStatusArr[PRV_TGF_OAM_GROUP_STATUS_SIZE_IN_WORDS_CNS] - array of OAM exception groups status.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum, stage or
*                                       PRV_TGF_OAM_EXCEPTION_TYPE_ENT value.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note Each bit in group status notifies exception in one of 32 OAM table entries
*       according to formula: <<1024n+32m>> - <<1024n+32m+31>>.
*       Where n - word in group status (0 < n < PRV_TGF_OAM_GROUP_STATUS_SIZE_IN_WORDS_CNS),
*       m - bit in group status (0 < m < 32)
*
*/
GT_STATUS prvTgfOamExceptionGroupStatusGet
(
    IN  PRV_TGF_OAM_STAGE_TYPE_ENT              stage,
    IN  PRV_TGF_OAM_EXCEPTION_TYPE_ENT          exceptionType,
    OUT GT_U32                                  groupStatusArr[PRV_TGF_OAM_GROUP_STATUS_SIZE_IN_WORDS_CNS]
);

/**
* @internal prvTgfOamOpcodeSet function
* @endinternal
*
* @brief   Set OAM opcode value
*
* @param[in] stage                    - OAM  type.
* @param[in] opcodeType               - OAM opcode type.
* @param[in] opcodeIndex              - OAM opcode index: (APPLICABLE RANGES: 0..3)
* @param[in] opcodeValue              - OAM opcode value: (APPLICABLE RANGES: 0..255)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum, stage, opcodeType or opcodeIndex.
* @retval GT_OUT_OF_RANGE          - opcode value is out of range.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamOpcodeSet
(
    IN PRV_TGF_OAM_STAGE_TYPE_ENT               stage,
    IN PRV_TGF_OAM_OPCODE_TYPE_ENT              opcodeType,
    IN GT_U32                                   opcodeIndex,
    IN GT_U32                                   opcodeValue
);

/**
* @internal prvTgfOamOpcodeGet function
* @endinternal
*
* @brief   Get OAM opcode value
*
* @param[in] stage                    - OAM  type.
* @param[in] opcodeType               - OAM opcode type.
* @param[in] opcodeIndex              - OAM opcode index: (APPLICABLE RANGES: 0..3).
*
* @param[out] opcodeValuePtr           - pointer to OAM opcode value
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum, stage, opcodeType or opcodeIndex.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamOpcodeGet
(
    IN  PRV_TGF_OAM_STAGE_TYPE_ENT              stage,
    IN  PRV_TGF_OAM_OPCODE_TYPE_ENT             opcodeType,
    IN  GT_U32                                  opcodeIndex,
    OUT GT_U32                                  *opcodeValuePtr
);

/**
* @internal prvTgfOamAgingPeriodEntrySet function
* @endinternal
*
* @brief   Set OAM aging period entry
*
* @param[in] stage                    - OAM  type.
* @param[in] entryIndex               - aging period entry index:
*                                      (APPLICABLE RANGES: 0..7).
* @param[in] agingPeriodValue         - expected time interval between
*                                      two received keepalive
*                                      messages from a given source in nanoseconds.
*                                      The granularity of the parameter is 40 ns.
*                                      Thus, the 34 bit field allows a period of
*                                      up to ~10 minutes
*                                      The  value is rounded down to nearest HW value.
*                                      (APPLICABLE RANGES: 0..0x3FFFFFFFF).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum, stage or entryIndex.
* @retval GT_OUT_OF_RANGE          - aging period value is out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamAgingPeriodEntrySet
(
    IN PRV_TGF_OAM_STAGE_TYPE_ENT       stage,
    IN GT_U32                           entryIndex,
    IN GT_U64                           agingPeriodValue
);

/**
* @internal prvTgfOamAgingPeriodEntryGet function
* @endinternal
*
* @brief   Get OAM aging period entry
*
* @param[in] stage                    - OAM  type.
* @param[in] entryIndex               - aging period entry index
*                                      (APPLICABLE RANGES: 0..7).
*
* @param[out] agingPeriodValuePtr      - pointer to expected time interval between
*                                      two received keepalive messages from a given source.
*                                      The period is measured using a 25 MHz clock,
*                                      i.e., in a granularity of 40 ns.
*                                      Thus, the 34 bit field allows a period of
*                                      up to ~10 minutes
*                                      The  value is rounded down to nearest HW value.
*                                      (APPLICABLE RANGES: 0..0x3FFFFFFFF).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum, stage or entryIndex.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamAgingPeriodEntryGet
(
    IN PRV_TGF_OAM_STAGE_TYPE_ENT           stage,
    IN GT_U32                               entryIndex,
    OUT GT_U64                             *agingPeriodValuePtr
);

/**
* @internal prvTgfOamOpcodeProfilePacketCommandEntrySet function
* @endinternal
*
* @brief   Set OAM Opcode Packet Command Entry
*
* @param[in] stage                    - OAM  type.
* @param[in] opcode                   -  value: (APPLICABLE RANGES: 0..255).
* @param[in] profile                  -  index: (APPLICABLE RANGES: 0..7).
* @param[in] command                  - OAM packet command.
* @param[in] cpuCodeOffset            - two least significant bits of OAM CPU code:
*                                      (APPLICABLE RANGES: 0..3)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum, stage, opcode, profile or command.
* @retval GT_OUT_OF_RANGE          - CPU offset is out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamOpcodeProfilePacketCommandEntrySet
(
    IN  PRV_TGF_OAM_STAGE_TYPE_ENT          stage,
    IN  GT_U32                              opcode,
    IN  GT_U32                              profile,
    IN  CPSS_PACKET_CMD_ENT                 command,
    IN  GT_U32                              cpuCodeOffset
);

/**
* @internal prvTgfOamOpcodeProfilePacketCommandEntryGet function
* @endinternal
*
* @brief   Get OAM Opcode Packet Command Entry
*
* @param[in] stage                    - OAM  type.
* @param[in] opcode                   -  value: (APPLICABLE RANGES: 0..255).
* @param[in] profile                  -  index: (APPLICABLE RANGES: 0..7).
*
* @param[out] commandPtr               - pointer to OAM packet command.
* @param[out] cpuCodeOffsetPtr         - pointer to two least significant bits
*                                      of OAM CPU code.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum, stage, entryIndex or entry parameter.
* @retval GT_BAD_STATE             - on bad value found in HW
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamOpcodeProfilePacketCommandEntryGet
(
    IN  PRV_TGF_OAM_STAGE_TYPE_ENT          stage,
    IN  GT_U32                              opcode,
    IN  GT_U32                              profile,
    OUT CPSS_PACKET_CMD_ENT                 *commandPtr,
    OUT GT_U32                              *cpuCodeOffsetPtr
);

/**
* @internal prvTgfOamTimeStampEtherTypeSet function
* @endinternal
*
* @brief   Set the ethertype which is used to identify Timestamp Tag
*
* @param[in] etherType                - the Ethertype of the Timestamp Tag.
*                                      (APPLICABLE RANGES: 0..0xFFFF).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or etherType.
* @retval GT_OUT_OF_RANGE          - Ethertype is out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamTimeStampEtherTypeSet
(
    IN  GT_U32      etherType
);

/**
* @internal prvTgfOamTimeStampEtherTypeGet function
* @endinternal
*
* @brief   Get the ethertype used to identify Timestamp Tag
*
* @param[out] etherTypePtr             - pointer to Ethertype of the Timestamp Tag.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamTimeStampEtherTypeGet
(
    OUT  GT_U32      *etherTypePtr
);

/**
* @internal prvTgfOamTimeStampParsingEnableSet function
* @endinternal
*
* @brief   Enable timestamp parsing
*
* @param[in] enable                   - enable/disable timestamp parsing.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamTimeStampParsingEnableSet
(
    IN  GT_BOOL      enable
);

/**
* @internal prvTgfOamTimeStampParsingEnableGet function
* @endinternal
*
* @brief   Get timestamp parsing status
*
* @param[out] enablePtr                - pointer to enable/disable timestamp parsing.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamTimeStampParsingEnableGet
(
    OUT  GT_BOOL     *enablePtr
);

/**
* @internal prvTgfOamLmOffsetTableSet function
* @endinternal
*
* @brief   Set Loss Measurement Offset Table Entry
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum, entryIndex or offset.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_OUT_OF_RANGE          - on offset out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamLmOffsetTableSet
(
    IN  GT_U32    entryIndex,
    IN  GT_U32    offset
);

/**
* @internal prvTgfOamLmOffsetTableGet function
* @endinternal
*
* @brief   Get Loss Measurement Offset Table Entry
*
* @param[out] offsetPtr                - pointer to LM offset in bytes.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or entryIndex.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamLmOffsetTableGet
(
    IN  GT_U32                                      entryIndex,
    OUT GT_U32                                     *offsetPtr
);

/**
* @internal prvTgfOamCpuCodeBaseSet function
* @endinternal
*
* @brief   Set CPU code of an OAM packet that is trapped or mirrored to the CPU.
*
* @param[in] stage                    - OAM  type.
* @param[in] baseCpuCode              - CPU code of an OAM packet that is trapped
*                                      or mirrored to the CPU.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum, stage or baseCpuCode.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamCpuCodeBaseSet
(
    IN PRV_TGF_OAM_STAGE_TYPE_ENT                   stage,
    IN CPSS_NET_RX_CPU_CODE_ENT                     baseCpuCode
);

/**
* @internal prvTgfOamCpuCodeBaseGet function
* @endinternal
*
* @brief   Get CPU code of an OAM packet that is trapped or mirrored to the CPU.
*
* @param[in] stage                    - OAM  type.
*
* @param[out] baseCpuCodePtr           - pointer to CPU code of an OAM packet that is trapped
*                                      or mirrored to the CPU.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or stage.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamCpuCodeBaseGet
(
    IN  PRV_TGF_OAM_STAGE_TYPE_ENT                  stage,
    OUT CPSS_NET_RX_CPU_CODE_ENT                    *baseCpuCodePtr
);

/**
* @internal prvTgfOamDualEndedLmPacketCommandSet function
* @endinternal
*
* @brief   Set Packet Command for Dual-Ended Loss Measurement packets.
*
* @param[in] stage                    - OAM  type.
* @param[in] command                  - Packet Command for Dual-Ended Loss Measurement packets.
* @param[in] cpuCodeLsBits            - Two least significant bits of the CPU code
*                                      (APPLICABLE RANGES: 0..3).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum, stage, command.
* @retval GT_OUT_OF_RANGE          - Packet command or two least significant bits
*                                       of CPU code are out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamDualEndedLmPacketCommandSet
(
    IN PRV_TGF_OAM_STAGE_TYPE_ENT                   stage,
    IN CPSS_PACKET_CMD_ENT                          command,
    IN GT_U32                                       cpuCodeLsBits
);

/**
* @internal prvTgfOamDualEndedLmPacketCommandGet function
* @endinternal
*
* @brief   Get Packet Command for Dual-Ended Loss Measurement packets.
*
* @param[in] stage                    - OAM  type.
*
* @param[out] commandPtr               - pointer to Packet Command for
*                                      Dual-Ended Loss Measurement packets.
* @param[out] cpuCodeLsBitsPtr         - pointer to two least significant bits
*                                      of the CPU code.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or stage.
* @retval GT_BAD_STATE             - on bad value found in HW
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamDualEndedLmPacketCommandGet
(
    IN  PRV_TGF_OAM_STAGE_TYPE_ENT                  stage,
    OUT CPSS_PACKET_CMD_ENT                         *commandPtr,
    OUT GT_U32                                      *cpuCodeLsBitsPtr
);

/**
* @internal prvTgfOamExceptionStatusGet function
* @endinternal
*
* @brief   Get exception status entry.
*
* @param[in] stage                    - OAM  type.
* @param[in] exceptionType            - OAM exception type.
* @param[in] entryIndex               - index of OAM exception table entry
*
* @param[out] entryBmpPtr              - pointer to OAM exception table entry
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum, stage, exceptionType or entryIndex.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note Each bit in the entry notifies exception per OAM flow according to formula:
*       <<32m+n>>; where m - table entry index (0 < m < 64). n - exception bit (0 < n < 32)
*
*/
GT_STATUS prvTgfOamExceptionStatusGet
(
    IN  PRV_TGF_OAM_STAGE_TYPE_ENT                  stage,
    IN  PRV_TGF_OAM_EXCEPTION_TYPE_ENT              exceptionType,
    IN  GT_U32                                      entryIndex,
    OUT GT_U32                                      *entryBmpPtr
);

/**
* @internal prvTgfOamAgingDaemonEnableSet function
* @endinternal
*
* @brief   Enable or disable OAM aging daemon.
*
* @param[in] stage                    - OAM  type.
* @param[in] enable                   - enable/disable OAM aging daemon:
*                                      GT_TRUE  - Aging Daemon periodically access
*                                      keepalive table and check the age state.
*                                      GT_FALSE - Aging Daemon doesn't request access
*                                      to keepalive table.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamAgingDaemonEnableSet
(
    IN  PRV_TGF_OAM_STAGE_TYPE_ENT          stage,
    IN  GT_BOOL                             enable
);

/**
* @internal prvTgfOamAgingDaemonEnableGet function
* @endinternal
*
* @brief   Get OAM aging daemon status.
*
* @param[in] stage                    - OAM  type.
*
* @param[out] enablePtr                - pointer to OAM aging daemon status:
*                                      GT_TRUE  - Aging Daemon periodically access
*                                      keepalive table  and check the age state.
*                                      GT_FALSE - Aging Daemon doesn't request access
*                                      to keepalive table.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or stage.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfOamAgingDaemonEnableGet
(
    IN  PRV_TGF_OAM_STAGE_TYPE_ENT          stage,
    OUT GT_BOOL                             *enablePtr
);

/**
* @internal prvTgfOamMplsCwChannelTypeProfileSet function
* @endinternal
*
* @brief   Set an MPLS Control Word Channel Type to Channel Type Profile
*
* @param[in] profileIndex             - channel type profile index (APPLICABLE RANGES: 1..15)
* @param[in] channelTypeId            - channel type id      (APPLICABLE RANGES: 0..65535)
* @param[in] profileEnable            - enable/disable the channel id:
*                                      GT_TRUE: the packet Control Word Channel Type is
*                                      compared with the channel type profile value
*                                      GT_FALSE: the packet Control Word Channel Type is not
*                                      compared with the channel type profile value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_OUT_OF_RANGE          - out-of-range in channelTypeId parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The MPLS packet is assumed to have a Control Word if the first nibble of
*       the MPLS payload is 0x1.
*       The Channel Type ID placed in 16 LSBs of a Control Word.
*
*/
GT_STATUS prvTgfOamMplsCwChannelTypeProfileSet
(
    IN  GT_U32    profileIndex,
    IN  GT_U32    channelTypeId,
    IN  GT_BOOL   profileEnable
);

/**
* @internal prvTgfOamHashBitSelectionSet function
* @endinternal
*
* @brief   Set indexes of the first and last bits of the 32 bits hash.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] stage                    - OAM  type.
* @param[in] hashFirstBit             - hash first index
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] hashLastBit              - hash last index
*                                      (APPLICABLE RANGES: 0..31)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on bad stage, bad first or last index.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_OUT_OF_RANGE          - on hash index is out of range.
*/
GT_STATUS prvTgfOamHashBitSelectionSet
(
    IN  PRV_TGF_OAM_STAGE_TYPE_ENT      stage,
    IN  GT_U32                          hashFirstBit,
    IN  GT_U32                          hashLastBit
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __tgfOamGenh */


