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
* @file prvCpssDxChTxqPdq.h
*
* @brief CPSS tunnel termination declarations.
*
* @version   54
********************************************************************************
*/

#ifndef __prvCpssDxChTxqPdq
#define __prvCpssDxChTxqPdq

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



#define CPSS_PDQ_SCHED_SHAPING_INFINITE_PROFILE_INDEX_CNS  0

#define CPSS_PDQ_SCHED_MIN_NODE_QUANTUM_CNS  ((GT_U32)16 )/*MTU = 4K . Min quantum (MTU)/256 = 16*/

#define CPSS_PDQ_SCHED_MIN_PORT_QUANTUM_CNS  ((GT_U32)32 )/*MTU = 4K . Min quantum (MTU)/128 = 32*/


#define CPSS_PDQ_SCHED_NODE_QUANTUM_UNIT_CNS  256

/************************* 32 bit Invalid data indicator **********************/
#define CPSS_PDQ_SCHED_INVAL_DATA_CNS   0xFFFFFFFF


/**
* @struct PRV_CPSS_PDQ_SCHED_LEVEL_PERIODIC_PARAMS_STC
 *
 * @brief Periodic Update Rate configuration params per level.
*/
typedef struct{

    /** Periodic shaper update Enable/Disable */
    GT_BOOL periodicState;

    /** Shaper Decoupling Enable/Disable */
    GT_BOOL shaperDecoupling;

} PRV_CPSS_PDQ_SCHED_LEVEL_PERIODIC_PARAMS_STC;

/**
* @enum CPSS_PDQ_SCHED_LEVEL_ENT
 *
 * @brief TM levels enumeration
*/
typedef enum{

    CPSS_PDQ_SCHED_LEVEL_Q_E = 0,

    CPSS_PDQ_SCHED_LEVEL_A_E,

    CPSS_PDQ_SCHED_LEVEL_B_E,

    CPSS_PDQ_SCHED_LEVEL_C_E,

    CPSS_PDQ_SCHED_LEVEL_P_E

} CPSS_PDQ_SCHED_LEVEL_ENT;

/**
* @struct CPSS_PDQ_SCHED_SHAPING_PROFILE_PARAMS_STC
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
    GT_U16 cbs;

    /** @brief EIR BW in KBit/Sec
     *  (APPLICABLE RANGES: 0..maxBw)
     */
    GT_U32 eirBw;

    /** @brief EIR Burst Size in Kbytes
     *  (APPLICABLE RANGES: 0..4K)
     */
    GT_U16 ebs;

} CPSS_PDQ_SCHED_SHAPING_PROFILE_PARAMS_STC;



/**
* @enum CPSS_PDQ_SCHD_MODE_ENT
 *
 * @brief Scheduler mode.
*/
typedef enum{

    CPSS_PDQ_SCHD_MODE_RR_E,

    CPSS_PDQ_SCHD_MODE_DWRR_E

} CPSS_PDQ_SCHD_MODE_ENT;



/**
* @enum CPSS_PDQ_SCHED_ELIG_FUNC_NODE_ENT
 *
 * @brief Node eligable function Ids
*/
typedef enum{

    CPSS_PDQ_SCHED_ELIG_N_PRIO1_E = 0,

    CPSS_PDQ_SCHED_ELIG_N_PRIO5_E = 1,

    CPSS_PDQ_SCHED_ELIG_N_SHP_E   = 2,

    CPSS_PDQ_SCHED_ELIG_N_MIN_SHP_E = 3,

    CPSS_PDQ_SCHED_ELIG_N_PPA_E     = 4,

    CPSS_PDQ_SCHED_ELIG_N_PPA_SP_E  = 5,

    CPSS_PDQ_SCHED_ELIG_N_PPA_SHP_E = 6,

    CPSS_PDQ_SCHED_ELIG_N_PPA_SP_MIN_SHP_E = 7,

    CPSS_PDQ_SCHED_ELIG_N_PPA_SHP_IGN_E    = 8,

    CPSS_PDQ_SCHED_ELIG_N_PPA_MIN_SHP_SP_IGN_E = 9,

    CPSS_PDQ_SCHED_ELIG_N_FP0_E = 10,

    CPSS_PDQ_SCHED_ELIG_N_FP1_E = 11,

    CPSS_PDQ_SCHED_ELIG_N_FP2_E = 12,

    CPSS_PDQ_SCHED_ELIG_N_FP3_E = 13,

    CPSS_PDQ_SCHED_ELIG_N_FP4_E = 14,

    CPSS_PDQ_SCHED_ELIG_N_FP5_E = 15,

    CPSS_PDQ_SCHED_ELIG_N_FP6_E = 16,

    CPSS_PDQ_SCHED_ELIG_N_FP7_E = 17,

    CPSS_PDQ_SCHED_ELIG_N_MIN_SHP_FP0_E = 18,

    CPSS_PDQ_SCHED_ELIG_N_MIN_SHP_FP1_E = 19,

    CPSS_PDQ_SCHED_ELIG_N_MIN_SHP_FP2_E = 20,

    CPSS_PDQ_SCHED_ELIG_N_MIN_SHP_FP3_E = 21,

    CPSS_PDQ_SCHED_ELIG_N_MIN_SHP_FP4_E = 22,

    CPSS_PDQ_SCHED_ELIG_N_MIN_SHP_FP5_E = 23,

    CPSS_PDQ_SCHED_ELIG_N_MIN_SHP_FP6_E = 24,

    CPSS_PDQ_SCHED_ELIG_N_MIN_SHP_FP7_E = 25,

    CPSS_PDQ_SCHED_ELIG_N_MAX_INC_MIN_SHP_E = 26,

    CPSS_PDQ_SCHED_ELIG_N_PP_E              = 27,

    CPSS_PDQ_SCHED_ELIG_N_MIN_SHP_PP_E      = 28,

    CPSS_PDQ_SCHED_ELIG_N_PP_SHP_E          = 29,

    CPSS_PDQ_SCHED_ELIG_N_SHP_4P_MIN_4P_MAX_E = 30,

    CPSS_PDQ_SCHED_ELIG_N_SHP_PP_TB_E       = 31,

    CPSS_PDQ_SCHED_ELIG_N_SHP_PP_MAX_TB_0   = 32,

    CPSS_PDQ_SCHED_ELIG_N_MAX_LIM_SHP_FP   = 33,

    CPSS_PDQ_SCHED_ELIG_N_DEQ_DIS_E         = 63

} CPSS_PDQ_SCHED_ELIG_FUNC_NODE_ENT;


/**
* @enum PRV_CPSS_PDQ_SCHED_ELIG_FUNC_QUEUE_ENT
 *
 * @brief Node eligable function Ids
*/
typedef enum{

    CPSS_PDQ_SCHED_ELIG_Q_PRIO0_E = 0,

    CPSS_PDQ_SCHED_ELIG_Q_PRIO1_E = 1,

    CPSS_PDQ_SCHED_ELIG_Q_PRIO2_E = 2,

    CPSS_PDQ_SCHED_ELIG_Q_PRIO3_E = 3,

    CPSS_PDQ_SCHED_ELIG_Q_MIN_SHP_PRIO0_E = 4,

    CPSS_PDQ_SCHED_ELIG_Q_MIN_SHP_PRIO1_E = 5,

    CPSS_PDQ_SCHED_ELIG_Q_MIN_SHP_PRIO2_E = 6,

    CPSS_PDQ_SCHED_ELIG_Q_MIN_SHP_PRIO3_E = 7,

    CPSS_PDQ_SCHED_ELIG_Q_PRIO4_E = 8,

    CPSS_PDQ_SCHED_ELIG_Q_PRIO5_E = 9,

    CPSS_PDQ_SCHED_ELIG_Q_PRIO6_E = 10,

    CPSS_PDQ_SCHED_ELIG_Q_PRIO7_E = 11,

    CPSS_PDQ_SCHED_ELIG_Q_MIN_SHP_PRIO4_E = 12,

    CPSS_PDQ_SCHED_ELIG_Q_MIN_SHP_PRIO5_E = 13,

    CPSS_PDQ_SCHED_ELIG_Q_MIN_SHP_PRIO6_E = 14,

    CPSS_PDQ_SCHED_ELIG_Q_MIN_SHP_PRIO7_E = 15,

    CPSS_PDQ_SCHED_ELIG_Q_SHP_E = 16,

    CPSS_PDQ_SCHED_ELIG_Q_MAX_INC_MIN_SHP_E = 17,

    CPSS_PDQ_SCHED_ELIG_Q_SHP_SCHED00_PROP00 = 18,

    CPSS_PDQ_SCHED_ELIG_Q_SHP_SCHED10_PROP10 = 19,

    CPSS_PDQ_SCHED_ELIG_Q_SHP_SCHED20_PROP20 = 20,

    CPSS_PDQ_SCHED_ELIG_Q_SHP_SCHED30_PROP30 = 21,

    CPSS_PDQ_SCHED_ELIG_Q_SHP_SCHED40_PROP40 = 22,

    CPSS_PDQ_SCHED_ELIG_Q_SHP_SCHED50_PROP50 = 23,

    CPSS_PDQ_SCHED_ELIG_Q_SHP_SCHED60_PROP60 = 24,

    CPSS_PDQ_SCHED_ELIG_Q_SHP_SCHED70_PROP70 = 25,

    CPSS_PDQ_SCHED_ELIG_Q_DEQ_DIS_E = 63

} PRV_CPSS_PDQ_SCHED_ELIG_FUNC_QUEUE_ENT;

/**
* @struct PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE_CONNECTED_QUEUES
 *
 * @brief Describe queue group connected to A node
*/
typedef struct{

    /** index that represent this range at Queue Group table at PDX unit [0..1023] */
    GT_U32 queueGroupIndex;

    /** the number of the first queue in this range[0..2K] */
    GT_U32 pdqQueueFirst;

    /** the number of the last queue in this range[0..2K] */
    GT_U32 pdqQueueLast;

    /** @brief queue number in sdq
     *  NOTE :currently assumed that only one queue group is connected to A node
     */
    GT_U32 queueBase;

    /**dp number */
   GT_U32 dp;

} PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE_CONNECTED_QUEUES;


/**
* @struct PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE
 *
 * @brief Describe list of A node connected to local port
*/
typedef struct{

    /** @brief Index of A node in TM tree
     *  queueGroup      - Information regarding queues connected to this A node
     */
    GT_U32 aNodeIndex;

    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE_CONNECTED_QUEUES queuesData;

    /** Physical port number that is represented by A node */
    GT_U32 physicalPort;

      /**Mapping type of physical port */
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT mappingType;

} PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE;


/**
* @struct PRV_CPSS_PDQ_SCHD_PORT_PARAMS_STC
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
     *  dropProfileInd      - Index of Port-Level Drop profile
     *  (APPLICABLE RANGES: 0..191,
     *  CPSS_TM_NO_DROP_PROFILE_CNS).
     */
    GT_U32 ebs;

    GT_U32 quantumArr[8];

    CPSS_PDQ_SCHD_MODE_ENT schdModeArr[8];

    /** @brief Eligible Priority Function pointer
     *  (APPLICABLE RANGES: (CPSS_TM_ELIG_FUNC_NODE_ENT) 0..32, 63).
     */
    CPSS_PDQ_SCHED_ELIG_FUNC_NODE_ENT eligiblePrioFuncId;

    /** @brief Number of children nodes
     *  (APPLICABLE RANGES: 1..TM_MAX_C_NODES)
     */
    GT_U32 numOfChildren;

} PRV_CPSS_PDQ_SCHD_PORT_PARAMS_STC;


/************************* Nodes Parameters Data Structures *******************/

/**
* @struct PRV_CPSS_PDQ_SCHD_QUEUE_PARAMS_STC
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
     *  dropProfileInd      - Index of Queue Drop profile
     *  (APPLICABLE RANGES: 1..2047,
     *  CPSS_TM_NO_DROP_PROFILE_CNS)
     */
    GT_U32 quantum;

    /** @brief Eligible Priority Function pointer
     *  (APPLICABLE RANGES: (CPSS_TM_ELIG_FUNC_QUEUE_ENT) 0..17, 18..25, 63)
     */
    CPSS_TM_ELIG_FUNC_QUEUE_ENT eligiblePrioFuncId;

} PRV_CPSS_PDQ_SCHD_QUEUE_PARAMS_STC;

/**
* @struct PRV_CPSS_PDQ_SCHD_A_NODE_PARAMS_STC
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
     *  dropProfileInd      - Index of A-Level Drop profile
     *  (APPLICABLE RANGES: 1..255,
     *  CPSS_TM_NO_DROP_PROFILE_CNS)
     */
    GT_U32 quantum;

    /** @brief Eligible Priority Function pointer
     *  (APPLICABLE RANGES: (CPSS_TM_ELIG_FUNC_NODE_ENT) 0..32, 63).
     *  schdModeArr[8]      - RR/DWRR Priority for A-Node Scheduling
     *  (APPLICABLE RANGES: (CPSS_TM_SCHD_MODE_ENT) 0..1).
     */
    CPSS_PDQ_SCHED_ELIG_FUNC_NODE_ENT eligiblePrioFuncId;

    CPSS_TM_SCHD_MODE_ENT schdModeArr[8];

    /** @brief Number of children nodes
     *  (APPLICABLE RANGES: 1..TM_MAX_QUEUES).
     */
    GT_U32 numOfChildren;

} PRV_CPSS_PDQ_SCHD_A_NODE_PARAMS_STC;

/**
* @struct PRV_CPSS_PDQ_SCHD_B_NODE_PARAMS_STC
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
     *  dropProfileInd      - Index of B-Level Drop profile
     *  (APPLICABLE RANGES: 1..63,
     *  CPSS_TM_NO_DROP_PROFILE_CNS).
     */
    GT_U32 quantum;

    /** @brief Eligible Priority Function pointer
     *  (APPLICABLE RANGES: (CPSS_TM_ELIG_FUNC_NODE_ENT) 0..32, 63).
     *  schdModeArr[8]      - RR/DWRR Priority for B-Node Scheduling
     *  (APPLICABLE RANGES: (CPSS_TM_SCHD_MODE_ENT) 0..1).
     */
    CPSS_PDQ_SCHED_ELIG_FUNC_NODE_ENT eligiblePrioFuncId;

    CPSS_TM_SCHD_MODE_ENT schdModeArr[8];

    /** @brief Number of children nodes
     *  (APPLICABLE RANGES: 1..TM_MAX_A_NODES).
     */
    GT_U32 numOfChildren;

} PRV_CPSS_PDQ_SCHD_B_NODE_PARAMS_STC;

/**
* @struct PRV_CPSS_PDQ_SCHD_C_NODE_PARAMS_STC
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
     *  dropCosMap        - C-node CoS bit map for Drop profiles
     *  (APPLICABLE RANGES: 1..255).
     *  dropProfileIndArr[8]   - Index of C-Level Drop profile per CoS
     *  (APPLICABLE RANGES: 0..63,
     *  CPSS_TM_NO_DROP_PROFILE_CNS).
     */
    GT_U32 quantum;

    /** @brief Eligible Priority Function pointer
     *  (APPLICABLE RANGES: (CPSS_TM_ELIG_FUNC_NODE_ENT) 0..32, 63).
     *  schdModeArr[8]      - RR/DWRR Priority for C-Node Scheduling
     *  (APPLICABLE RANGES: (CPSS_TM_SCHD_MODE_ENT) 0..1).
     */
    CPSS_PDQ_SCHED_ELIG_FUNC_NODE_ENT eligiblePrioFuncId;

    CPSS_TM_SCHD_MODE_ENT schdModeArr[8];

    /** @brief Number of children nodes
     *  (APPLICABLE RANGES: 1..TM_MAX_B_NODES).
     */
    GT_U32 numOfChildren;

} PRV_CPSS_PDQ_SCHD_C_NODE_PARAMS_STC;

/**
* @struct PRV_CPSS_DXCH_TXQ_SIP_6_SCHED_PROFILE
 *
 * @brief Sip 6 scheduling profile.
*/
typedef struct{
    /** @brief Binded ports bitmap .Each bit represent port (if bit is set then port is binded to profile).
   */

    GT_U32 bindedPortsBmp[32];

    /** @brief If bit is set than this traffic class using WRR ,else this traffic class using SP
   */

    GT_U32 wrrEnableBmp;

     /** @brief If bit is set than this traffic class using WRR group 1  ,else this traffic class using  WRR group 0.
     *    This bitmap is relevant if coresponding bit is set at wrrEnableBmp .
   */
    GT_U32 wrrGroupBmp;

    /** @brief Weight used for DWRR. Note that this weigts are multiplied by MSU(Maximum Selected data Unit),
   */
    GT_U32 weights[16];

    GT_U32 lowLatencyQueue;

} PRV_CPSS_DXCH_TXQ_SIP_6_SCHED_PROFILE;


/**
* @internal prvCpssFalconTxqPdqInit function
* @endinternal
*
* @brief   Initialize the PDQ scheduler configuration library.
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - Device number.
* @param[in] pdqNum                   - tile number.(APPLICABLE RANGES:0..3)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvCpssFalconTxqPdqInit
(
    IN GT_U8 devNum,
    IN  GT_U32 pdqNum,
    IN GT_U32 mtu
);
/**
* @internal prvCpssFalconTxqPdqPeriodicSchemeConfig function
* @endinternal
*
* @brief   Configure Periodic Scheme.
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - Device number.
* @param[in] pdqNum                   - tile number.(APPLICABLE RANGES:0..3)
* @param[in] paramsPtr                - (pointer to) scheduling parameters structure.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1. The API may be invoked once in a system lifetime.
*       2. To indicate the shaping is disabled for a level the
*       periodicState field must be set to GT_FALSE.
*       3 paramsPtr is handled as 5 elements array including also a
*       port level scheduling configuration.
*
*/
GT_STATUS prvCpssFalconTxqPdqPeriodicSchemeConfig
(
    IN GT_U8                                                devNum,
    IN GT_U32                                               pdqNum,
    IN PRV_CPSS_PDQ_SCHED_LEVEL_PERIODIC_PARAMS_STC        *paramsPtr
);
/**
* @internal prvCpssFalconTxqPdqAnodeToPortCreate function
* @endinternal
*
* @brief   Create path from A-node to Port.
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - Device number.
* @param[in] pdqNum                   - tile number.(APPLICABLE RANGES:0..3)
* @param[in] portInd                  - Port index.
* @param[in] aParamsPtr               - A-Node parameters structure pointer.
* @param[in] explicitQueueStart       - if GT_TRUE then queue number is scpecified by the caller,otherwise first free queue number is given
* @param[in] queueStart               - relevant if explicitQueueStart is equal GT_TRUE.Queue number requested by the caller
* @param[in] fixedPortToBnodeMapping  - if equal GT_TRUE then P node,C node and P node will have same index.
* @param[in] bParamsPtr               - B-Node parameters structure pointer.
* @param[in] cParamsPtr               - C-Node parameters structure pointer.
* @param[in] addToExisting            - if equal GT_TRUE then A node is added to already existing P->C->B branch.
*
* @param[out] aNodeIndPtr             - (pointer to) The created A-Node index (APPLICABLE RANGES: 0..16383).
* @param[out] bNodeIndPtr             - (pointer to) The created B-Node index (APPLICABLE RANGES: 0..4095).
* @param[out] cNodeIndPtr             - (pointer to) The created C-Node index (APPLICABLE RANGES: 0..511).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_BAD_STATE             - on portInd not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note To indicate that no shaping is needed to the queue/node, set
*       shapingProfilePtr to CPSS_TM_INVAL_CNS.
*
*/

GT_STATUS prvCpssFalconTxqPdqAnodeToPortCreate
(
    IN  GT_U8                           devNum,
    IN GT_U32                           pdqNum,
    IN  GT_U32                          portInd,
    IN  PRV_CPSS_PDQ_SCHD_A_NODE_PARAMS_STC       *aParamsPtr,
    IN  GT_BOOL explicitQueueStart,
    IN  GT_U32  queueStart,
    IN GT_BOOL fixedPortToBnodeMapping,
    IN  PRV_CPSS_PDQ_SCHD_B_NODE_PARAMS_STC       *bParamsPtr,
    IN  PRV_CPSS_PDQ_SCHD_C_NODE_PARAMS_STC       *cParamsPtr,
    IN GT_BOOL                           addToExisting,
    OUT GT_U32                          *aNodeIndPtr,
    OUT GT_U32                          *bNodeIndPtr,
    OUT GT_U32                          *cNodeIndPtr
);

/**
* @internal prvCpssFalconTxqPdqAsymPortCreate function
* @endinternal
*
* @brief   Create Port with assymetric sub-tree and download its parameters to HW.
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - Device number.
* @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
* @param[in] portInd                  - Port index.
* @param[in] paramsPtr                - Port parameters structure pointer.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_BAD_STATE             - on portInd not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note To indicate that no shaping is needed for port, set
*       cirBw to CPSS_PDQ_SCHED_INVAL_DATA_CNS, in this case other shaping
*       parameters will not be considered.
*
* @param[in] fixedMappingToCNode - if equal GT_TRUE then P[i] is mapped to C[i]
*/


GT_STATUS prvCpssFalconTxqPdqAsymPortCreate
(
    IN GT_U8                        devNum,
    IN GT_U32                       pdqNum,
    IN GT_U32                       portInd,
    IN  GT_BOOL                  fixedMappingToCNode,
    IN PRV_CPSS_PDQ_SCHD_PORT_PARAMS_STC      *paramsPtr
);
/**
* @internal prvCpssFalconTxqPdqShapingProfileCreate function
* @endinternal
*
* @brief   Create a Shaping Profile.
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - Device number.
* @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
* @param[in] level                    - Level to configure the profile for.
* @param[in] profileStrPtr            - Shaping profile configuration struct pointer.
*
* @param[out] profileIndPtr            - (pointer to) The created Shaping profile index.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_BAD_STATE             - on portInd not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/

GT_STATUS prvCpssFalconTxqPdqShapingProfileCreate
(
    IN  GT_U8                                   devNum,
    IN GT_U32                                   pdqNum,
    IN  CPSS_PDQ_SCHED_LEVEL_ENT                       level,
    IN  CPSS_PDQ_SCHED_SHAPING_PROFILE_PARAMS_STC      *profileStrPtr,
    OUT GT_U32                                  *profileIndPtr
);
/**
* @internal prvCpssFalconTxqPdqQueueToAnodeCreate function
* @endinternal
*
* @brief   Create path from Queue to A-node.
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - Device number.
* @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
* @param[in] aNodeInd                 - A-Node index.
* @param[in] qParamsPtr               - Queue parameters structure pointer.
*
* @param[out] queueIndPtr              - (pointer to) The created Queue index (APPLICABLE RANGES: 0..65535).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_BAD_STATE             - on portInd not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvCpssFalconTxqPdqQueueToAnodeCreate
(
    IN  GT_U8                           devNum,
    IN GT_U32                           pdqNum,
    IN  GT_U32                          aNodeInd,
    IN  PRV_CPSS_PDQ_SCHD_QUEUE_PARAMS_STC        *qParamsPtr,
    IN  GT_BOOL                     explicitNumber,
    OUT GT_U32                          *queueIndPtr
);

/**
* @internal prvCpssFalconTxqPdqDumpPortSw function
* @endinternal
*
* @brief   print the configure tree under a specific port (SW data base ,no read from HW)
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - Device number.
* @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
* @param[in] portIndex                - Port index.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/

GT_STATUS prvCpssFalconTxqPdqDumpPortSw
(
    IN GT_U8                   devNum,
    IN GT_U32                  pdqNum,
    IN GT_U32                  portIndex
);

/**
* @internal prvCpssFalconTxqPdqDumpPortHw function
* @endinternal
*
* @brief   print the configure tree under a specific port ( read from HW)
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - Device number.
* @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
* @param[in] portIndex                - Port index.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/

GT_STATUS prvCpssFalconTxqPdqDumpPortHw
(
    IN GT_U8                   devNum,
    IN GT_U32                  pdqNum,
    IN GT_U32                  portIndex
);

/**
* @internal prvCpssFalconTxqPdqSetShapingOnNode function
* @endinternal
*
* @brief   Write shaping parameters to specific node
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - Device number.
* @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
* @param[in] nodeInd                  - Node index.
* @param[in] isQueueNode              - if equal GT_TRUE then it is node at Q level ,if  if equal GT_TRUE then it is node at A level
* @param[in] burstSize                - burst size in units of 4K bytes
*                                      (max value is 4K which results in 16K burst size)
* @param[in,out] maxRatePtr               - (pointer to)Requested Rate in Kbps
* @param[in,out] maxRatePtr               -(pointer to) the actual Rate value in Kbps.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/

GT_STATUS prvCpssFalconTxqPdqSetShapingOnNode
(
    IN      GT_U8                        devNum,
    IN      GT_U32                       pdqNum,
    IN      GT_U32                       nodeInd,
    IN      GT_BOOL                       isQueueNode,
    IN      GT_U16    burstSize,
    INOUT GT_U32    *maxRatePtr

);
/**
* @internal prvCpssFalconTxqPdqGetShapingOnNode function
* @endinternal
*
* @brief   Read shaping parameters of specific node( read from SW)
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - Device number.
* @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
* @param[in] nodeInd                  - Node index.
* @param[in] isQueueNode              - if equal GT_TRUE then it is node at Q level ,if  if equal GT_TRUE then it is node at A level
*
* @param[out] burstSizePtr             - burst size in units of 4K bytes
*                                      (max value is 4K which results in 16K burst size)
* @param[out] maxRatePtr               - Requested Rate in Kbps
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/

GT_STATUS prvCpssFalconTxqPdqGetShapingOnNode
(
    IN      GT_U8                        devNum,
    IN      GT_U32                       pdqNum,
    IN      GT_U32                       nodeInd,
    IN      GT_BOOL                       isQueueNode,
    OUT  GT_U16    *burstSizePtr,
    OUT  GT_U32    *maxRatePtr

);

/**
* @internal prvCpssFalconTxqPdqSetShapingEnable function
* @endinternal
*
* @brief   Enable/Disable Token Bucket rate shaping on specified port or queue of  specified device.
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - Device number.
* @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
* @param[in] nodeInd                  - Node index.
* @param[in] isQueueNode              - if equal GT_TRUE then it is node at Q level ,if  if equal GT_TRUE then it is node at A level
* @param[in] enable                   - GT_TRUE,  Shaping
*                                      GT_FALSE, disable Shaping
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/
GT_STATUS prvCpssFalconTxqPdqSetShapingEnable
(
    IN      GT_U8                        devNum,
    IN      GT_U32                       pdqNum,
    IN      GT_U32                       nodeInd,
    IN      GT_BOOL                       isQueueNode,
    IN      GT_BOOL                       enable
);
/**
* @internal prvCpssFalconTxqPdqGetShapingEnable function
* @endinternal
*
* @brief   Get Enable/Disable Token Bucket rate shaping on specified port or queue of  specified device.
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - Device number.
* @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
* @param[in] nodeInd                  - Node index.
* @param[in] isQueueNode              - if equal GT_TRUE then it is node at Q level ,if  if equal GT_TRUE then it is node at A level
*
* @param[out] enablePtr                - GT_TRUE, enable Shaping
*                                      GT_FALSE, disable Shaping
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/

GT_STATUS prvCpssFalconTxqPdqGetShapingEnable
(
    IN      GT_U8                         devNum,
    IN      GT_U32                       pdqNum,
    IN      GT_U32                       nodeInd,
    IN      GT_BOOL                   isQueueNode,
    OUT      GT_BOOL                   *enablePtr
);

/**
* @internal prvCpssFalconTxqPdqUpdateTypeANodeSchedulingAttributes function
* @endinternal
*
* @brief   Update scheduler HW with shadow scheduling profile attributes
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -                   Device number.
* @param[in] pdqNum                                     - Tile number.(APPLICABLE RANGES:0..3)
* @param[in] schedAtrributesPtr                  - (pointer to)Scheduling attributes
* @param[in] lowLatencyQueueIsPresent     if equal GT_TRUE then it mean that there is at least one queue in SP group
* @param[in] firstPnodeQueue                -      First queue in corresponding Pnode
* @param[in] lastPnodeQueue                -      First queue in corresponding Pnode
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/
GT_STATUS prvCpssFalconTxqPdqUpdateTypeANodeSchedulingAttributes
(
    IN      GT_U8                                   devNum,
    IN      GT_U32                                  pdqNum,
    IN      PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE          *aNodePtr,
    IN      PRV_CPSS_DXCH_TXQ_SIP_6_SCHED_PROFILE   *schedAtrributesPtr,
    IN      GT_BOOL                                 lowLatencyQueueIsPresent,
    IN      GT_U32                                  firstPnodeQueue,
    IN      GT_U32                                  lastPnodeQueue
);

/**
* @internal prvCpssFalconTxqGlobalTxEnableSet function
* @endinternal
 *
* @brief   Enable/Disable transmission of specified tile on specific device
*
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
* @param[in] devNum                   - physical device number
* @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
* @param[in] enable           -
*                                      GT_TRUE, enable transmission
*                                      GT_FALSE, disable transmission
*/
GT_STATUS   prvCpssFalconTxqPdqGlobalTxEnableSet
(
    IN GT_U8    devNum,
    IN GT_U32  pdqNum,
    IN GT_BOOL  enable
);
/**
* @internal prvCpssFalconTxqGlobalTxEnableGet function
* @endinternal
*
* @brief   Get the status of transmission of specified tile on specific device (Enable/Disable).
*                Note this function check the status in all availeble tiles (The status should be the SAME)
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2;Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - physical device number
* @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE, enable transmission
*                                      GT_FALSE, disable transmission
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS   prvCpssFalconTxqPdqGlobalTxEnableGet
(
    IN GT_U8    devNum,
    IN GT_U32  pdqNum,
    IN GT_BOOL  * enablePtr
);

/**
* @internal prvCpssFalconTxqPdqGetErrorStatus function
* @endinternal
*
* @brief   Get the status of scheduler errors
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2;Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - physical device number
* @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
*
* @param[out] errorCounterPtr                - (pointer to) error counter
* @param[out] exceptionCounterPtr                - (pointer to) exception counter
* @param[out] errorStatusPtr                - (pointer to) error status
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS   prvCpssFalconTxqPdqGetErrorStatus
(
    IN GT_U8    devNum,
    IN GT_U32  pdqNum,
    IN GT_U32   * errorCounterPtr,
    IN GT_U32   * exceptionCounterPtr,
    IN GT_U32   * errorStatusPtr
);

/**
* @internal prvCpssFalconTxqPdqGetQueueStatus function
* @endinternal
*
* @brief   Read queue token bucket level and deficit
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2;Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - physical device number
* @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
* @param[in] queueNum                   - queue number .(APPLICABLE RANGES:0..2K-1)

* @param[out] levelPtr                - (pointer to)  token bucket level
* @param[out] deficitPtr                - (pointer to) deficit
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS   prvCpssFalconTxqPdqGetQueueStatus
(
    IN GT_U8    devNum,
    IN GT_U32  pdqNum,
    IN GT_U32   queueNum,
    OUT GT_32   *  levelPtr,
    OUT GT_U32   * dificitPtr
);
/**
* @internal prvCpssFalconTxqPdqGetAlevelStatus function
* @endinternal
*
* @brief   Read A level token bucket level and deficit
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2;Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - physical device number
* @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
* @param[in] alevelNum                   - A level number .(APPLICABLE RANGES:0..511)

* @param[out] levelPtr                - (pointer to)  token bucket level
* @param[out] deficitPtr                - (pointer to) deficit
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS   prvCpssFalconTxqPdqGetAlevelStatus
(
    IN GT_U8    devNum,
    IN GT_U32  pdqNum,
    IN GT_U32   alevelNum,
    OUT GT_32   *  levelPtr,
    OUT GT_U32   * dificitPtr
);

/**
* @internal prvCpssFalconTxqPdqGetQueueShapingParameters function
* @endinternal
*
* @brief   Read Q level shaping configurations
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2;Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - physical device number
* @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
* @param[in] queueNum                   - Q level number .(APPLICABLE RANGES:0..2047)

* @param[out] divExpPtr                - (pointer to)  divider exponent
* @param[out] tokenPtr                - (pointer to) number of tokens
* @param[out] resPtr                - (pointer to)  resolution exponent
* @param[out] burstPtr                - (pointer to) burst size
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS   prvCpssFalconTxqPdqGetQueueShapingParameters
(
    IN GT_U8    devNum,
    IN GT_U32  pdqNum,
    IN GT_U32   queueNum,
    OUT GT_U32   *  divExpPtr,
    OUT GT_U32   * tokenPtr,
    OUT GT_U32   * resPtr,
    OUT GT_U32   * burstPtr
);
/**
* @internal prvCpssFalconTxqPdqGetPerLevelShapingParameters function
* @endinternal
*
* @brief   Read per  level shaping configurations
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2;Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - physical device number
* @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
* @param[in] queueLevel                   - equal GT_TRUE if request is for Q level ,else A level

* @param[out] shaperDecPtr                - (pointer to) shaper decopling
* @param[out] perInterPtr                - (pointer to) periodic interval
* @param[out] perEnPtr                - (pointer to)  per level shaping enabled
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS   prvCpssFalconTxqPdqGetPerLevelShapingParameters
(
    IN GT_U8    devNum,
    IN GT_U32  pdqNum,
    IN GT_BOOL  queueLevel,
    OUT GT_U32   *  shaperDecPtr,
    OUT GT_U32   * perInterPtr,
    OUT GT_U32   * perEnPtr
);
/**
* @internal prvCpssFalconTxqPdqGetQueueShapingParameters function
* @endinternal
*
* @brief   Read A level shaping configurations
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2;Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - physical device number
* @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
* @param[in] aNodeNum                   - A level number .(APPLICABLE RANGES:0..511)

* @param[out] divExpPtr                - (pointer to)  divider exponent
* @param[out] tokenPtr                - (pointer to) number of tokens
* @param[out] resPtr                - (pointer to)  resolution exponent
* @param[out] burstPtr                - (pointer to) burst size
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS   prvCpssFalconTxqPdqGetAnodeShapingParameters
(
    IN GT_U8    devNum,
    IN GT_U32  pdqNum,
    IN GT_U32  aNodeNum,
    OUT GT_U32   *  divExpPtr,
    OUT GT_U32   * tokenPtr,
    OUT GT_U32   * resPtr,
    OUT GT_U32   * burstPtr
);

GT_STATUS prvCpssFalconTxqPdqTreePlevelDwrrEnableSet
(
    IN GT_U8               devNum,
    IN GT_U32              pdqNum,
    IN GT_BOOL            enable
);


GT_STATUS prvCpssFalconTxqPdqConvertSpeedToQuantum
(
        IN CPSS_PORT_SPEED_ENT speed ,
        IN GT_BOOL isRemotePort,
        OUT GT_U32 * quantum
);

GT_STATUS   prvCpssFalconTxqPdqSetPortQuantum
(
    IN GT_U8    devNum,
    IN GT_U32  pdqNum,
    IN GT_U32   portIndex,
    IN GT_U32   quantum
);

GT_STATUS   prvCpssFalconTxqPdqSetAlevelQuantum
(
    IN GT_U8    devNum,
    IN GT_U32  pdqNum,
    IN GT_U32   aNodeIndex,
    IN GT_U32   quantum
);

/**
* @internal prvCpssFalconTxqPdqClose function
* @endinternal
*
* @brief  Release dynamic memory allocation for tile specific TxQ scheduler
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum             -physical device number
* @param[in] pdqNum             -number of PDQ[0..3]
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqPdqClose
(
    IN GT_U8   devNum,
    IN GT_U32  pdqNum
);




#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChTxqPdq */

