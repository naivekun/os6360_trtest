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
* @file cpssTmPublicDefs.h
*
* @brief TM Configuration Library Public Definitions.
*
* @version   2
********************************************************************************
*/

#ifndef __cpssTmPublicDefsh
#define __cpssTmPublicDefsh

#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/common/cpssTypes.h>

/************************* Constants Definitions ******************************/

/************************* Node Quantum Unit (in 256 bytes units) *************/
#define CPSS_TM_NODE_QUANTUM_UNIT_CNS  256

/************************* Port Quantum Unit (in 64 bytes units) **************/
#define CPSS_TM_PORT_QUANTUM_UNIT_CNS  64

/************************* Infinite Profile ***********************************/
#define CPSS_TM_SHAPING_INFINITE_PROFILE_INDEX_CNS  0

/************************* No Drop ********************************************/
#define CPSS_TM_NO_DROP_PROFILE_CNS  0

/************************* 32 bit Invalid data indicator **********************/
#define CPSS_TM_INVAL_CNS   0xFFFFFFFF

/************************* Maximum Bandwidth (in Kbits/sec) ********************/
#define CPSS_TM_SHAPING_INFINITE_BW_CNS  0xFFFFFFFF

#define CPSS_TM_SHAPING_MAX_BURST_CNS (128*1024) /* 128 KB */

/*********************** WRED curve probability array size ********************/
#define CPSS_TM_DROP_PROB_ARR_SIZE_CNS 32

/************************* Enumerated Types ***********************************/

/**
* @enum CPSS_TM_LEVEL_ENT
 *
 * @brief TM levels enumeration
*/
typedef enum{

    /** Q-nodes level */
    CPSS_TM_LEVEL_Q_E = 0,

    /** A-nodes level */
    CPSS_TM_LEVEL_A_E,

    /** B-nodes level */
    CPSS_TM_LEVEL_B_E,

    /** C-nodes level */
    CPSS_TM_LEVEL_C_E,

    /** Ports level */
    CPSS_TM_LEVEL_P_E

} CPSS_TM_LEVEL_ENT;


/**
* @enum CPSS_TM_ELIG_FUNC_NODE_ENT
 *
 * @brief Node eligable function Ids
*/
typedef enum{

    /** Eligible function priority 1 */
    CPSS_TM_ELIG_N_PRIO1_E = 0,

    /** Eligible function priority 5 */
    CPSS_TM_ELIG_N_PRIO5_E = 1,

    /** Eligible function shaping */
    CPSS_TM_ELIG_N_SHP_E   = 2,

    /** Eligible function min shaping */
    CPSS_TM_ELIG_N_MIN_SHP_E = 3,

    /** Eligible function propagated priority */
    CPSS_TM_ELIG_N_PPA_E     = 4,

    /** @brief Eligible function propagated priority with
     *  strict priority (priority = 5)
     */
    CPSS_TM_ELIG_N_PPA_SP_E  = 5,

    /** Eligible function propagated with shaping */
    CPSS_TM_ELIG_N_PPA_SHP_E = 6,

    /** @brief Eligible function propagated priority with
     *  strict priority (priority = 5) with min shaper
     */
    CPSS_TM_ELIG_N_PPA_SP_MIN_SHP_E = 7,

    /** @brief Eligible function propagated priority and shaping with
     *  shaper ignore for propagated priority = 3
     */
    CPSS_TM_ELIG_N_PPA_SHP_IGN_E    = 8,

    /** @brief Eligible function propagated priority with strict priority
     *  (priority = 5) with min shaper with shaper ignore for
     *  propagated priority = 3
     */
    CPSS_TM_ELIG_N_PPA_MIN_SHP_SP_IGN_E = 9,

    /** Node is always eligible in priority 0 */
    CPSS_TM_ELIG_N_FP0_E = 10,

    /** Node is always eligible in priority 1 */
    CPSS_TM_ELIG_N_FP1_E = 11,

    /** Node is always eligible in priority 2 */
    CPSS_TM_ELIG_N_FP2_E = 12,

    /** Node is always eligible in priority 3 */
    CPSS_TM_ELIG_N_FP3_E = 13,

    /** Node is always eligible in priority 4 */
    CPSS_TM_ELIG_N_FP4_E = 14,

    /** Node is always eligible in priority 5 */
    CPSS_TM_ELIG_N_FP5_E = 15,

    /** Node is always eligible in priority 6 */
    CPSS_TM_ELIG_N_FP6_E = 16,

    /** Node is always eligible in priority 7 */
    CPSS_TM_ELIG_N_FP7_E = 17,

    /** Node is eligible in priority 0 when minTB is positive */
    CPSS_TM_ELIG_N_MIN_SHP_FP0_E = 18,

    /** Node is eligible in priority 1 when minTB is positive */
    CPSS_TM_ELIG_N_MIN_SHP_FP1_E = 19,

    /** Node is eligible in priority 2 when minTB is positive */
    CPSS_TM_ELIG_N_MIN_SHP_FP2_E = 20,

    /** Node is eligible in priority 3 when minTB is positive */
    CPSS_TM_ELIG_N_MIN_SHP_FP3_E = 21,

    /** Node is eligible in priority 4 when minTB is positive */
    CPSS_TM_ELIG_N_MIN_SHP_FP4_E = 22,

    /** Node is eligible in priority 5 when minTB is positive */
    CPSS_TM_ELIG_N_MIN_SHP_FP5_E = 23,

    /** Node is eligible in priority 6 when minTB is positive */
    CPSS_TM_ELIG_N_MIN_SHP_FP6_E = 24,

    /** Node is eligible in priority 7 when minTB is positive */
    CPSS_TM_ELIG_N_MIN_SHP_FP7_E = 25,

    /** Eligible function propagated with MaxTB include MinTB shaping */
    CPSS_TM_ELIG_N_MAX_INC_MIN_SHP_E = 26,

    /** Node is always eligible in propagated priority */
    CPSS_TM_ELIG_N_PP_E              = 27,

    /** Node is eligible when MinTB is positive */
    CPSS_TM_ELIG_N_MIN_SHP_PP_E      = 28,

    /** Eligible function with propagated priority and shaping */
    CPSS_TM_ELIG_N_PP_SHP_E          = 29,

    /** Eligible function with min max shaping */
    CPSS_TM_ELIG_N_SHP_4P_MIN_4P_MAX_E = 30,

    /** @brief Eligible function with min max shaping, the pp influences
     *  which shaper will be used
     */
    CPSS_TM_ELIG_N_SHP_PP_TB_E       = 31,

    /** @brief Eligible function with min max shaping,SchedPrio0 = Prop0(Min)+Prop0..7(Max)
     *  SchedPrio1..7 = SchedPrio1..7(Min), PropOut = Prop
     */
    CPSS_TM_ELIG_N_SHP_PP_MAX_TB_0   = 32,

    /** @brief Eligible function for dequeue disable (the node is not eligible)
     *  PPA Propagated priority according to RevA .
     *  i.e. - only propagated priority 0...3 are supported.
     *  When node gets propagated priority 4...7 it is not eligible
     */
    CPSS_TM_ELIG_N_DEQ_DIS_E         = 63

} CPSS_TM_ELIG_FUNC_NODE_ENT;


/**
* @enum CPSS_TM_ELIG_FUNC_QUEUE_ENT
 *
 * @brief Node eligable function Ids
*/
typedef enum{

    /** Eligible function priority 0 */
    CPSS_TM_ELIG_Q_PRIO0_E = 0,

    /** Eligible function priority 1 */
    CPSS_TM_ELIG_Q_PRIO1_E = 1,

    /** Eligible function priority 2 */
    CPSS_TM_ELIG_Q_PRIO2_E = 2,

    /** Eligible function priority 3 */
    CPSS_TM_ELIG_Q_PRIO3_E = 3,

    /** Eligible function min shaping priority 0 */
    CPSS_TM_ELIG_Q_MIN_SHP_PRIO0_E = 4,

    /** Eligible function min shaping priority 1 */
    CPSS_TM_ELIG_Q_MIN_SHP_PRIO1_E = 5,

    /** Eligible function min shaping priority 2 */
    CPSS_TM_ELIG_Q_MIN_SHP_PRIO2_E = 6,

    /** Eligible function min shaping priority 3 */
    CPSS_TM_ELIG_Q_MIN_SHP_PRIO3_E = 7,

    /** Eligible function priority 4 */
    CPSS_TM_ELIG_Q_PRIO4_E = 8,

    /** Eligible function priority 5 */
    CPSS_TM_ELIG_Q_PRIO5_E = 9,

    /** Eligible function priority 6 */
    CPSS_TM_ELIG_Q_PRIO6_E = 10,

    /** Eligible function priority 7 */
    CPSS_TM_ELIG_Q_PRIO7_E = 11,

    /** Eligible function min shaping priority 4 */
    CPSS_TM_ELIG_Q_MIN_SHP_PRIO4_E = 12,

    /** Eligible function min shaping priority 5 */
    CPSS_TM_ELIG_Q_MIN_SHP_PRIO5_E = 13,

    /** Eligible function min shaping priority 6 */
    CPSS_TM_ELIG_Q_MIN_SHP_PRIO6_E = 14,

    /** Eligible function min shaping priority 7 */
    CPSS_TM_ELIG_Q_MIN_SHP_PRIO7_E = 15,

    /** Eligible function with shaping */
    CPSS_TM_ELIG_Q_SHP_E = 16,

    /** Eligible function propagated with MaxTB include MinTB shaping */
    CPSS_TM_ELIG_Q_MAX_INC_MIN_SHP_E = 17,

    /** Eligible function min max shaping, scheduling priority 0/0, propagated priority 0/0 */
    CPSS_TM_ELIG_Q_SHP_SCHED00_PROP00 = 18,

    /** Eligible function min max shaping, scheduling priority 1/0, propagated priority 1/0 */
    CPSS_TM_ELIG_Q_SHP_SCHED10_PROP10 = 19,

    /** Eligible function min max shaping, scheduling priority 2/0, propagated priority 2/0 */
    CPSS_TM_ELIG_Q_SHP_SCHED20_PROP20 = 20,

    /** Eligible function min max shaping, scheduling priority 3/0, propagated priority 3/0 */
    CPSS_TM_ELIG_Q_SHP_SCHED30_PROP30 = 21,

    /** Eligible function min max shaping, scheduling priority 4/0, propagated priority 4/0 */
    CPSS_TM_ELIG_Q_SHP_SCHED40_PROP40 = 22,

    /** Eligible function min max shaping, scheduling priority 5/0, propagated priority 5/0 */
    CPSS_TM_ELIG_Q_SHP_SCHED50_PROP50 = 23,

    /** Eligible function min max shaping, scheduling priority 6/0, propagated priority 6/0 */
    CPSS_TM_ELIG_Q_SHP_SCHED60_PROP60 = 24,

    /** Eligible function min max shaping, scheduling priority 7/0, propagated priority 7/0 */
    CPSS_TM_ELIG_Q_SHP_SCHED70_PROP70 = 25,

    /** Eligible function for dequeue disable (the queue is not eligible) */
    CPSS_TM_ELIG_Q_DEQ_DIS_E = 63

} CPSS_TM_ELIG_FUNC_QUEUE_ENT;

/**
* @enum CPSS_TM_TOKEN_BUCKET_ENT
 *
 * @brief Token bucket usage
*/
typedef enum{

    /** Use Min token bucket */
    CPSS_TM_TOKEN_BUCKET_MIN_E = 0,

    /** Use Max token bucket */
    CPSS_TM_TOKEN_BUCKET_MAX_E

} CPSS_TM_TOKEN_BUCKET_ENT;

/**
* @enum CPSS_TM_DROP_MODE_ENT
 *
 * @brief Drop WRED/CATD mode
*/
typedef enum{

    CPSS_TM_DROP_MODE_CB_TD_CA_WRED_E = 0,

    CPSS_TM_DROP_MODE_CB_TD_CA_TD_E,

    CPSS_TM_DROP_MODE_CB_TD_ONLY_E

} CPSS_TM_DROP_MODE_ENT;

/**
* @enum CPSS_TM2TM_CHANNEL_ENT
 *
 * @brief TM2TM channel
*/
typedef enum{

    /** TM2TM Node channel */
    CPSS_TM2TM_CHANNEL_NODE_E = 0,

    /** TM2TM Port channel */
    CPSS_TM2TM_CHANNEL_PORT_E

} CPSS_TM2TM_CHANNEL_ENT;


/**
* @enum CPSS_TM_COLOR_NUM_ENT
 *
 * @brief Number of colors
*/
typedef enum{

    /** One color per level */
    CPSS_TM_COLOR_NUM_1_E = 0,

    /** Two colors per level */
    CPSS_TM_COLOR_NUM_2_E,

    /** Three colors per level */
    CPSS_TM_COLOR_NUM_3_E

} CPSS_TM_COLOR_NUM_ENT;

/**
* @enum CPSS_TM_DP_SOURCE_ENT
 *
 * @brief DP source.
*/
typedef enum{

    /** Average Queue Length is used to calculate DP source. */
    CPSS_TM_DP_SOURCE_AQL_E = 0,

    /** Queue Length is used to calculate DP source. */
    CPSS_TM_DP_SOURCE_QL_E

} CPSS_TM_DP_SOURCE_ENT;


/**
* @enum CPSS_TM_SCHD_MODE_ENT
 *
 * @brief Scheduler mode.
*/
typedef enum{

    /** RR mode. */
    CPSS_TM_SCHD_MODE_RR_E,

    /** DWRR mode. */
    CPSS_TM_SCHD_MODE_DWRR_E

} CPSS_TM_SCHD_MODE_ENT;


/************************* Global Paramteres Data structures ******************/

/**
* @struct CPSS_TM_LIB_INIT_PARAMS_STC
 *
 * @brief Customize TM lib initialization parameters.
*/
typedef struct{

    /** @brief if 0 then MTU is TM_MTU
     *  (APPLICABLE RANGES: 0..16K)
     */
    GT_U32 tmMtu;

} CPSS_TM_LIB_INIT_PARAMS_STC;

/**
* @struct CPSS_TM_LEVEL_PERIODIC_PARAMS_STC
 *
 * @brief Periodic Update Rate configuration params per level.
*/
typedef struct{

    /** Periodic shaper update Enable/Disable */
    GT_BOOL periodicState;

    /** Shaper Decoupling Enable/Disable */
    GT_BOOL shaperDecoupling;

} CPSS_TM_LEVEL_PERIODIC_PARAMS_STC;


/* Maximum DROP threshold in bytes */
#define CPSS_TM_MAX_DROP_THRESHOLD_CNS	0xFFFFFFFF

/**
* @struct CPSS_TM_DROP_PROFILE_CA_TD_PARAMS_STC
 *
 * @brief Color Aware Drop configuration profile.
*/
typedef struct{

    GT_U32 caTdThreshold[3];

} CPSS_TM_DROP_PROFILE_CA_TD_PARAMS_STC;


/**
* @struct CPSS_TM_DROP_PROFILE_CA_WRED_PARAMS_STC
 *
 * @brief Drop configuration profile.
*/
typedef struct{

    /** @brief Exponent value to determine new average queue length
     *  as function of previous average and the current size
     *  of the queue
     *  (APPLICABLE RANGES: 0..15)
     *  curveIndex[3]      - RED curve index per color
     *  (APPLICABLE RANGES: 0..7)
     *  dpCurveScale[3]     - Drop Probabiliry Curve scaling:
     *  0 - no scaling, DP[i] = DPCurve[i].
     *  2 colors - scale down from DPCurve[i] 1/64 to DPCurve[i] 63/64
     *  3 colors - scale down from DPCurve[i] 1/16 to DPCurve[i] 15/16
     *  (APPLICABLE RANGES: 2 colors: 0..64, 3 colors: 0..15)
     *  caWredTdMinThreshold[3] - Color Aware Wred Min TD in Bytes Per Color
     *  (APPLICABLE RANGES: 0..caWredTdMaxThreshold[i])
     *  caWredTdMaxThreshold[3] - Color Aware Max Wred TD in Bytes Per Color
     *  (APPLICABLE RANGES: caWredTdMinThreshold[i]..CPSS_TM_MAX_DROP_THRESHOLD_CNS)
     */
    GT_U8 aqlExponent;

    GT_U8 curveIndex[3];

    GT_U8 dpCurveScale[3];

    GT_U32 caWredTdMinThreshold[3];

    GT_U32 caWredTdMaxThreshold[3];

} CPSS_TM_DROP_PROFILE_CA_WRED_PARAMS_STC;


/**
* @struct CPSS_TM_DROP_PROFILE_PARAMS_STC
 *
 * @brief Structure which groups the 3 drop modes.
*/
typedef struct{

    /** Drop Mode */
    CPSS_TM_DROP_MODE_ENT dropMode;

    /** @brief Color Blind TD in Bytes
     *  (APPLICABLE RANGES: 0..CPSS_TM_MAX_DROP_THRESHOLD_CNS)
     */
    GT_U32 cbTdThresholdBytes;

    /** Color Aware Tail Drop configuration profile */
    CPSS_TM_DROP_PROFILE_CA_TD_PARAMS_STC caTdDp;

    /** Color Aware WRED Drop configuration profile */
    CPSS_TM_DROP_PROFILE_CA_WRED_PARAMS_STC caWredDp;

} CPSS_TM_DROP_PROFILE_PARAMS_STC;


/**
* @struct CPSS_TM_SHAPING_PROFILE_PARAMS_STC
 *
 * @brief Shared Shaper Profile configuration structure.
*/
typedef struct{

    /** @brief CIR BW in KBit/Sec
     *  (APPLICABLE RANGES: 0..maxBw)
     */
    GT_U32 cirBw;

    /** @brief CIR Burst Size in Kbytes
     *  (APPLICABLE RANGES: 0..4K)
     */
    GT_U32 cbs;

    /** @brief EIR BW in KBit/Sec
     *  (APPLICABLE RANGES: 0..maxBw)
     */
    GT_U32 eirBw;

    /** @brief EIR Burst Size in Kbytes
     *  (APPLICABLE RANGES: 0..4K)
     */
    GT_U32 ebs;

} CPSS_TM_SHAPING_PROFILE_PARAMS_STC;


/************************* Nodes Parameters Data Structures *******************/

/**
* @struct CPSS_TM_QUEUE_PARAMS_STC
 *
 * @brief Queue Parameters Data Structure.
*/
typedef struct{

    /** @brief Index of Shaping profile
     *  (APPLICABLE RANGES: 1..total tree Q,A,B,C levels nodes (84479 for BC2),
     *  CPSS_TM_SHAPING_INFINITE_PROFILE_INDEX_CNS)
     *  number of tree levels nodes can be retrived with cpssTmTreeParamsGet.
     */
    GT_U32 shapingProfilePtr;

    /** @brief DWRR Quantum in resolution of CPSS_TM_NODE_QUANTUM_UNIT_CNS (256)
     *  (APPLICABLE RANGES: 0x40..0x3FFF).
     *  Quantum Limits depend on system MTU and can be retrieved with
     *  cpssTmNodeQuantumLimitsGet API.
     */
    GT_U32 quantum;

    /** @brief Index of Queue Drop profile
     *  (APPLICABLE RANGES: 1..2047,
     *  CPSS_TM_NO_DROP_PROFILE_CNS)
     */
    GT_U32 dropProfileInd;

    /** @brief Eligible Priority Function pointer
     *  (APPLICABLE RANGES: (CPSS_TM_ELIG_FUNC_QUEUE_ENT) 0..17, 18..25, 63)
     */
    CPSS_TM_ELIG_FUNC_QUEUE_ENT eligiblePrioFuncId;

} CPSS_TM_QUEUE_PARAMS_STC;

/**
* @struct CPSS_TM_A_NODE_PARAMS_STC
 *
 * @brief A-Node Parameters Data Structure.
*/
typedef struct{

    /** @brief Index of Shaping profile
     *  (APPLICABLE RANGES: 1..total tree Q,A,B,C levels nodes (84479 for BC2),
     *  CPSS_TM_SHAPING_INFINITE_PROFILE_INDEX_CNS)
     *  number of tree levels nodes can be retrived with cpssTmTreeParamsGet.
     */
    GT_U32 shapingProfilePtr;

    /** @brief DWRR Quantum in resolution of CPSS_TM_NODE_QUANTUM_UNIT_CNS (256) bytes
     *  (APPLICABLE RANGES: 0x40..0x3FFF).
     *  Quantum Limits depend on system MTU and can be retrieved with
     *  cpssTmNodeQuantumLimitsGet API.
     */
    GT_U32 quantum;

    /** @brief Index of A
     *  (APPLICABLE RANGES: 1..255,
     *  CPSS_TM_NO_DROP_PROFILE_CNS)
     */
    GT_U32 dropProfileInd;

    /** @brief Eligible Priority Function pointer
     *  (APPLICABLE RANGES: (CPSS_TM_ELIG_FUNC_NODE_ENT) 0..32, 63).
     *  schdModeArr[8]      - RR/DWRR Priority for A-Node Scheduling
     *  (APPLICABLE RANGES: (CPSS_TM_SCHD_MODE_ENT) 0..1).
     */
    CPSS_TM_ELIG_FUNC_NODE_ENT eligiblePrioFuncId;

    CPSS_TM_SCHD_MODE_ENT schdModeArr[8];

    /** @brief Number of children nodes
     *  (APPLICABLE RANGES: 1..TM_MAX_QUEUES).
     */
    GT_U32 numOfChildren;

} CPSS_TM_A_NODE_PARAMS_STC;

/**
* @struct CPSS_TM_B_NODE_PARAMS_STC
 *
 * @brief B-Node Parameters Data Structure.
*/
typedef struct{

    /** @brief Index of Shaping profile
     *  (APPLICABLE RANGES: 1..total tree Q,A,B,C levels nodes (84479 for BC2),
     *  CPSS_TM_SHAPING_INFINITE_PROFILE_INDEX_CNS)
     *  number of tree levels nodes can be retrived with cpssTmTreeParamsGet.
     */
    GT_U32 shapingProfilePtr;

    /** @brief DWRR Quantum in resolution of CPSS_TM_NODE_QUANTUM_UNIT_CNS (256) bytes
     *  (APPLICABLE RANGES: 0x40..0x3FFF).
     *  Quantum Limits depend on system MTU and can be retrieved with
     *  cpssTmNodeQuantumLimitsGet API.
     */
    GT_U32 quantum;

    /** @brief Index of B
     *  (APPLICABLE RANGES: 1..63,
     *  CPSS_TM_NO_DROP_PROFILE_CNS).
     */
    GT_U32 dropProfileInd;

    /** @brief Eligible Priority Function pointer
     *  (APPLICABLE RANGES: (CPSS_TM_ELIG_FUNC_NODE_ENT) 0..32, 63).
     *  schdModeArr[8]      - RR/DWRR Priority for B-Node Scheduling
     *  (APPLICABLE RANGES: (CPSS_TM_SCHD_MODE_ENT) 0..1).
     */
    CPSS_TM_ELIG_FUNC_NODE_ENT eligiblePrioFuncId;

    CPSS_TM_SCHD_MODE_ENT schdModeArr[8];

    /** @brief Number of children nodes
     *  (APPLICABLE RANGES: 1..TM_MAX_A_NODES).
     */
    GT_U32 numOfChildren;

} CPSS_TM_B_NODE_PARAMS_STC;

/**
* @struct CPSS_TM_C_NODE_PARAMS_STC
 *
 * @brief C-Node Parameters Data Structure.
*/
typedef struct{

    /** @brief Index of Shaping profile
     *  (APPLICABLE RANGES: 1..total tree Q,A,B,C levels nodes (84479 for BC2),
     *  CPSS_TM_SHAPING_INFINITE_PROFILE_INDEX_CNS)
     *  number of tree levels nodes can be retrived with cpssTmTreeParamsGet.
     */
    GT_U32 shapingProfilePtr;

    /** @brief DWRR Quantum in resolution of CPSS_TM_NODE_QUANTUM_UNIT_CNS (256) bytes
     *  (APPLICABLE RANGES: 0x40..0x3FFF).
     *  Quantum Limits depend on system MTU and can be retrieved with
     *  cpssTmNodeQuantumLimitsGet API.
     */
    GT_U32 quantum;

    /** @brief C
     *  (APPLICABLE RANGES: 1..255).
     *  dropProfileIndArr[8]   - Index of C-Level Drop profile per CoS
     *  (APPLICABLE RANGES: 0..63,
     *  CPSS_TM_NO_DROP_PROFILE_CNS).
     */
    GT_U32 dropCosMap;

    GT_U32 dropProfileIndArr[8];

    /** @brief Eligible Priority Function pointer
     *  (APPLICABLE RANGES: (CPSS_TM_ELIG_FUNC_NODE_ENT) 0..32, 63).
     *  schdModeArr[8]      - RR/DWRR Priority for C-Node Scheduling
     *  (APPLICABLE RANGES: (CPSS_TM_SCHD_MODE_ENT) 0..1).
     */
    CPSS_TM_ELIG_FUNC_NODE_ENT eligiblePrioFuncId;

    CPSS_TM_SCHD_MODE_ENT schdModeArr[8];

    /** @brief Number of children nodes
     *  (APPLICABLE RANGES: 1..TM_MAX_B_NODES).
     */
    GT_U32 numOfChildren;

} CPSS_TM_C_NODE_PARAMS_STC;

/**
* @struct CPSS_TM_PORT_PARAMS_STC
 *
 * @brief Port Parameters Data Structure.
*/
typedef struct{

    /** @brief CIR BW in KBit/Sec
     *  (APPLICABLE RANGES: 0..100G,
     *  CPSS_TM_SHAPING_INFINITE_BW_CNS)
     */
    GT_U32 cirBw;

    /** @brief EIR BW in KBit/Sec
     *  (APPLICABLE RANGES: 0..100G,
     *  CPSS_TM_SHAPING_INFINITE_BW_CNS)
     */
    GT_U32 eirBw;

    /** CBS in Kbytes */
    GT_U32 cbs;

    /** @brief EBS in Kbytes
     *  quantumArr[8]       - DWRR Quantum Per Cos in resolution of CPSS_TM_PORT_QUANTUM_UNIT_CNS (64) bytes
     *  (APPLICABLE RANGES: 0x10..0x1FF).
     *  Quantum Limits depend on system MTU and can be retrieved with
     *  cpssTmPortQuantumLimitsGet API.
     *  schdModeArr[8]      - RR/DWRR Priority for Port Scheduling
     *  (APPLICABLE RANGES: 0..1)
     */
    GT_U32 ebs;

    GT_U32 quantumArr[8];

    CPSS_TM_SCHD_MODE_ENT schdModeArr[8];

    /** @brief Index of Port
     *  (APPLICABLE RANGES: 0..191,
     *  CPSS_TM_NO_DROP_PROFILE_CNS).
     */
    GT_U32 dropProfileInd;

    /** @brief Eligible Priority Function pointer
     *  (APPLICABLE RANGES: (CPSS_TM_ELIG_FUNC_NODE_ENT) 0..32, 63).
     */
    CPSS_TM_ELIG_FUNC_NODE_ENT eligiblePrioFuncId;

    /** @brief Number of children nodes
     *  (APPLICABLE RANGES: 1..TM_MAX_C_NODES)
     */
    GT_U32 numOfChildren;

} CPSS_TM_PORT_PARAMS_STC;


/**
* @struct CPSS_TM_PORT_DROP_PER_COS_STC
 *
 * @brief Port Drop per Cos Parameters Data Structure.
*/
typedef struct{

    /** @brief port CoS bit map for Drop profiles
     *  (APPLICABLE RANGES: 1..255)
     *  dropProfileIndArr[8]    - Index of Drop profile per CoS
     *  (APPLICABLE RANGES: 1..255,
     *  CPSS_TM_NO_DROP_PROFILE_CNS)
     */
    GT_U32 dropCosMap;

    GT_U32 dropProfileIndArr[8];

} CPSS_TM_PORT_DROP_PER_COS_STC;


/**
* @struct CPSS_TM_PORT_STATUS_STC
 *
 * @brief Port status data structure.
*/
typedef struct{

    /** Maximal Shaper Bucket level */
    GT_U32 maxBucketLevel;

    /** @brief Minimal Shaper Bucket level
     *  deficitArr[8]    - DWRR Deficit per instance
     */
    GT_U32 minBucketLevel;

    GT_U32 deficitArr[8];

} CPSS_TM_PORT_STATUS_STC;


/**
* @struct CPSS_TM_NODE_STATUS_STC
 *
 * @brief Node status data structure.
*/
typedef struct{

    /** Maximal Shaper Bucket level */
    GT_U32 maxBucketLevel;

    /** Minimal Shaper Bucket level */
    GT_U32 minBucketLevel;

    /** DWRR Deficit */
    GT_U32 deficit;

} CPSS_TM_NODE_STATUS_STC;


/**
* @struct CPSS_TM_QMR_PKT_STATISTICS_STC
 *
 * @brief QMR Packet Statistics data structure.
*/
typedef struct
{
#if 0 /* TBD */
    GT_U64  numPktsFromSms;
    GT_U64  numErrsFromSmsDropped;
    GT_U64  numErrsFromSmsPassed;
#endif
    GT_U64  numPktsToUnInstallQueue;
} CPSS_TM_QMR_PKT_STATISTICS_STC;


/**
* @struct CPSS_TM_RCB_PKT_STATISTICS_STC
 *
 * @brief RCB Packet Statistics data structure.
*/
typedef struct{

    /** Non */
    GT_U64 numPktsToSms;

    /** Packets with CRC error */
    GT_U64 numCrcErrPktsToSms;

    /** Packtes with error from SMS that has been written to DRAM */
    GT_U64 numErrsFromSmsToDram;

    /** Dropped Port Flush Packets counter */
    GT_U64 numPortFlushDrpDataPkts;

} CPSS_TM_RCB_PKT_STATISTICS_STC;


/**
* @struct CPSS_TM_ERROR_INFO_STC
 *
 * @brief TM Blocks Error Information.
*/
typedef struct{

    /** TM Block Error counter */
    GT_U32 errorCounter;

    /** TM Block Exception Counter */
    GT_U32 exceptionCounter;

} CPSS_TM_ERROR_INFO_STC;


/**
* @struct CPSS_TM2TM_EXTERNAL_HDR_STC
 *
 * @brief TM2TM External Headers.
*/
typedef struct{

    /** @brief Header size
     *  (APPLICABLE RANGES: 3, 7, 11, 15) - fixed values
     *  contentArr[4] - The header content
     */
    GT_U32 size;

    GT_U64 contentArr[4];

} CPSS_TM2TM_EXTERNAL_HDR_STC;


/**
* @struct CPSS_TM2TM_CNTRL_PKT_STRUCT_STC
 *
 * @brief TM2TM Control Packet Structure.
*/
typedef struct{

    /** Number of ports updates in TM to TM packet */
    GT_U32 portsNum;

    /** Number of nodes updates in TM to TM packet */
    GT_U32 nodesNum;

} CPSS_TM2TM_CNTRL_PKT_STRUCT_STC;


/**
* @struct CPSS_TM2TM_DELTA_RANGE_STC
 *
 * @brief TM2TM Delta Range Mapping to Priority.
*/
typedef struct{

    /** Priority 0 upper range */
    GT_U32 upperRange0;

    /** Priority 1 upper range */
    GT_U32 upperRange1;

    /** Priority 2 upper range */
    GT_U32 upperRange2;

} CPSS_TM2TM_DELTA_RANGE_STC;


/******************************************************************************
* Typedef: struct CPSS_TM_TREE_CHANGE_STC
*
* Description: Reshuffling index/range change structure.
*
* Fields:
*
*      type     - Type of change: index - TM_ENABLE, range - TM_DISABLE
*      index    - Index of changed parent node
*      oldIndex - Old index/range
*      newIndex - New index/range
*      nextPtr  - Pointer to the next change
******************************************************************************/
typedef struct CPSS_TM_TREE_CHANGE_STCT
{
	GT_U32   type;
	GT_U32   index;
	GT_U32   oldIndex;
    GT_U32   newIndex;
    struct CPSS_TM_TREE_CHANGE_STCT *nextPtr;
} CPSS_TM_TREE_CHANGE_STC;


/**
* @struct CPSS_TM_ELIG_PRIO_FUNC_OUT_STC
 *
 * @brief Eligible Priority Function Data structure.
*/
typedef struct{

    /** Use Max Token Bucket */
    GT_U32 maxTb;

    /** Use Min Token Bucket */
    GT_U32 minTb;

    /** Propagated priority */
    GT_U32 propPrio;

    /** Scheduling priority */
    GT_U32 schedPrio;

    /** Eligibility */
    GT_U32 elig;

} CPSS_TM_ELIG_PRIO_FUNC_OUT_STC;


typedef union
{
	CPSS_TM_ELIG_PRIO_FUNC_OUT_STC queueEligPrioFunc[4];
    CPSS_TM_ELIG_PRIO_FUNC_OUT_STC nodeEligPrioFunc[8][4];
} CPSS_TM_ELIG_PRIO_FUNC_STC;


/**
* @struct CPSS_TM_QUANTUM_LIMITS_STC
 *
 * @brief quantum Limits for port/node. Used for read these parameters from TM instance
*/
typedef struct{

    /** minimum possible quantum value for port/node in bytes. */
    GT_U32 minQuantum;

    /** maximum possible quantum value for port/node in bytes. */
    GT_U32 maxQuantum;

    /** quantum value granularioty in bytes. */
    GT_U32 resolution;

} CPSS_TM_QUANTUM_LIMITS_STC;


/**
* @struct CPSS_TM_TREE_PARAMS_STC
 *
 * @brief TM Tree HW properties and Limits.
*/
typedef struct{

    /** max number of Queue Nodes. */
    GT_U32 maxQueues;

    /** max number of A Nodes. */
    GT_U32 maxAnodes;

    /** max number of B Nodes. */
    GT_U32 maxBnodes;

    /** max number of C Nodes. */
    GT_U32 maxCnodes;

    /** max number of TM Ports. */
    GT_U32 maxPorts;

} CPSS_TM_TREE_PARAMS_STC;

/**
* @struct CPSS_TM_UNITS_ERROR_STATUS_STC
 *
 * @brief TM Units Global Error Status
*/
typedef struct{

    /** Queue Manager Unit Status. */
    GT_U32 qmngrStatus;

    /** Drop Unit Status */
    GT_U32 dropStatus;

    /** Scheduler Unit Status. */
    GT_U32 schedStatus;

    /** RCB (Packets Reordering) Unit Status. */
    GT_U32 rcbStatus;

} CPSS_TM_UNITS_ERROR_STATUS_STC;

/**
* @struct CPSS_TM_BAP_UNIT_ERROR_STATUS_STC
 *
 * @brief TM BAP Unit Global Error Status
*/
typedef struct{

    /** BAP (Dram Access) Unit Status. */
    GT_U32 bapStatus;

    /** BAP device number. */
    GT_U8 bapNum;

} CPSS_TM_BAP_UNIT_ERROR_STATUS_STC;

#endif 	    /* __cpssTmPublicDefsh */


